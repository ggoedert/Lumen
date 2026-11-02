//==============================================================================================================================================================================
/// \file
/// \brief     AssetManager
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lAssetManager.h"
#include "lAssetInfo.h"
#include "lEngine.h"

/// \cond
#include <unordered_set>
#include <charconv>
/// \endcond

using namespace Lumen;

constexpr milliseconds InfoDelayTime { 500 };

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    CLASS_UNIQUE_PTR_DEF(AssetManagerImpl);

    /// library entry file layout, all values little endian:
    ///   4 bytes magic, u32 format version, u32 asset type, u64 source size, i64 source last write time, then the CBOR body until the end of the file
    /// the source size and last write time are compared for equality (not ordering) so a source that was restored from an older version is detected too
    constexpr std::array<byte, 4> cLibraryMagic { 'L', 'L', 'I', 'B' };
    constexpr dword cLibraryVersion = 1;
    constexpr size_t cLibraryHeaderSize = 4 + 4 + 4 + 8 + 8;

    /// library entry header
    struct LibraryHeader
    {
        /// format version
        dword mVersion = 0;

        /// asset type
        Hash mType = 0;

        /// size of the source when the entry was written
        qword mSourceSize = 0;

        /// last write time of the source when the entry was written
        int64_t mSourceTime = 0;
    };

    /// normalizes a path to the key used by every cache of the asset manager: lexically normalized, forward slashes
    inline std::string Normalize(const std::filesystem::path &path)
    {
        return FileSystem::NormalizeFilePath(path).generic_string();
    }

    /// whether a file name is an asset info file
    inline bool IsInfoName(const std::string &name)
    {
        return (name.size() > 5) && (ToLower(name.substr(name.size() - 5)) == ".info");
    }

    /// append a little endian integer
    inline void PutLE(std::vector<uint8_t> &out, uint64_t value, int bytes)
    {
        for (int i = 0; i < bytes; ++i)
        {
            out.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xff));
        }
    }

    /// read a little endian integer
    inline uint64_t GetLE(const uint8_t *in, int bytes)
    {
        uint64_t value = 0;
        for (int i = bytes - 1; i >= 0; --i)
        {
            value = (value << 8) | in[i];
        }
        return value;
    }

    /// source stamp, identifies the exact state of a source file
    struct SourceStamp
    {
        /// whether the source file exists
        bool mExists = false;

        /// size
        qword mSize = 0;

        /// last write time
        int64_t mTime = 0;
    };

    /// gets the stamp of a source file
    inline SourceStamp GetSourceStamp(const std::string &path)
    {
        const FileSystem::FileAttributes attributes = FileSystem::GetAttributes(path);
        SourceStamp stamp;
        stamp.mExists = attributes.mFlags.Has(FileSystem::Flag::File);
        stamp.mSize = attributes.mSize;
        stamp.mTime = static_cast<int64_t>(attributes.mLastWrite.time_since_epoch().count());
        return stamp;
    }

    /// reads a whole file
    inline bool ReadFileBytes(const std::filesystem::path &path, std::vector<uint8_t> &bytes)
    {
        // check first, a cache miss is normal and must stay silent
        if (!FileSystem::Exists(path))
        {
            return false;
        }
        Id::Type file = FileSystem::Open(path, false, true);
        if (file == Id::Invalid)
        {
            return false;
        }
        const size_t size = FileSystem::Size(file);
        bool result = (size != static_cast<size_t>(-1));
        if (result)
        {
            bytes.resize(size);
            result = (FileSystem::ReadBytes(file, bytes.data(), size) == size);
        }
        FileSystem::Close(file);
        return result;
    }

    /// parses the header of a library entry, bodyOffset is where the CBOR body starts
    inline bool ParseLibraryHeader(const std::vector<uint8_t> &bytes, LibraryHeader &header, size_t &bodyOffset)
    {
        if ((bytes.size() < cLibraryHeaderSize) || !std::equal(cLibraryMagic.begin(), cLibraryMagic.end(), bytes.begin()))
        {
            return false;
        }
        const uint8_t *p = bytes.data() + cLibraryMagic.size();
        header.mVersion = static_cast<dword>(GetLE(p, 4));
        header.mType = static_cast<Hash>(GetLE(p + 4, 4));
        header.mSourceSize = static_cast<qword>(GetLE(p + 8, 8));
        header.mSourceTime = static_cast<int64_t>(GetLE(p + 16, 8));
        bodyOffset = cLibraryHeaderSize;
        return true;
    }

    /// whether a library entry file name is a 16 digit hex UUID, temp files (name.tmp) are reported through isTemp
    inline bool ParseLibraryName(const std::string &name, UUID &uuid, bool &isTemp)
    {
        isTemp = name.size() == 20 && name.ends_with(".tmp");
        const std::string digits = isTemp ? name.substr(0, 16) : name;
        if (digits.size() != 16)
        {
            return false;
        }
        const auto result = std::from_chars(digits.data(), digits.data() + digits.size(), uuid, 16);
        return (result.ec == std::errc()) && (result.ptr == digits.data() + digits.size());
    }

    /// writes a library entry, atomically: the data is written to a temp file next to the entry which then replaces it
    inline bool WriteLibraryEntry(UUID uuid, Hash type, const std::string &sourcePath, const Serialized::Type &data)
    {
        // the stamp must describe the source as it is right now, callers write the source before the library entry
        const SourceStamp stamp = GetSourceStamp(sourcePath);
        if (!stamp.mExists)
        {
            Lumen::DebugLog::Detail("Library entry not written, source {} does not exist", sourcePath);
            return false;
        }

        std::vector<uint8_t> bytes(cLibraryMagic.begin(), cLibraryMagic.end());
        PutLE(bytes, cLibraryVersion, 4);
        PutLE(bytes, type, 4);
        PutLE(bytes, stamp.mSize, 8);
        PutLE(bytes, static_cast<uint64_t>(stamp.mTime), 8);
        const std::vector<uint8_t> body = Serialized::Type::to_cbor(data);
        bytes.insert(bytes.end(), body.begin(), body.end());

        const std::filesystem::path entryPath = AssetManager::LibraryPath(uuid);
        const std::filesystem::path tempPath = std::filesystem::path(entryPath).concat(".tmp");
        Id::Type file = FileSystem::Open(tempPath, true, true);
        if (file == Id::Invalid)
        {
            Lumen::DebugLog::Warning("Unable to open Library entry for writing, {}", tempPath.generic_string());
            return false;
        }
        bool result = FileSystem::WriteBytes(file, bytes.data(), bytes.size());
        FileSystem::Close(file);
        if (result)
        {
            result = FileSystem::Rename(tempPath, entryPath);
        }
        if (!result)
        {
            Lumen::DebugLog::Warning("Unable to write Library entry {}", entryPath.generic_string());
            FileSystem::Delete(tempPath);
        }
        return result;
    }

    /// reads a library entry, false when it is missing, damaged, of another version or type, or stale compared to its source
    inline bool ReadLibraryEntry(UUID uuid, Hash type, const std::string &sourcePath, Serialized::Type &data)
    {
        std::vector<uint8_t> bytes;
        if (!ReadFileBytes(AssetManager::LibraryPath(uuid), bytes))
        {
            return false;
        }

        LibraryHeader header;
        size_t bodyOffset = 0;
        if (!ParseLibraryHeader(bytes, header, bodyOffset))
        {
            Lumen::DebugLog::Warning("Library entry of {} is damaged, ignoring it", sourcePath);
            return false;
        }
        if ((header.mVersion != cLibraryVersion) || (header.mType != type))
        {
            Lumen::DebugLog::Detail("Library entry of {} has another version or type, ignoring it", sourcePath);
            return false;
        }

        const SourceStamp stamp = GetSourceStamp(sourcePath);
        if (stamp.mExists)
        {
            if ((stamp.mSize != header.mSourceSize) || (stamp.mTime != header.mSourceTime))
            {
                Lumen::DebugLog::Detail("Library entry of {} is stale", sourcePath);
                return false;
            }
        }
#ifdef EDITOR
        else
        {
            // in the editor the source is the truth, no source means no asset
            return false;
        }
#endif
        // a game build may ship the Library without its sources, trust the entry

        try
        {
            data = Serialized::Type::from_cbor(bytes.begin() + bodyOffset, bytes.end());
        }
        catch (const std::exception &e)
        {
            Lumen::DebugLog::Warning("Library entry of {} is damaged, {}", sourcePath, e.what());
            return false;
        }
        return true;
    }

    /// AssetManagerImpl class
    class AssetManagerImpl
    {
        CLASS_NO_DEFAULT_CTOR(AssetManagerImpl);
        CLASS_NO_COPY_MOVE(AssetManagerImpl);
        CLASS_PTR_UNIQUEMAKER(AssetManagerImpl);

    public:
        /// constructs an assets implementation with engine
        explicit AssetManagerImpl(const EngineWeakPtr &engine) : mEngine(engine) {}

        /// destroys assets
        ~AssetManagerImpl() = default;

        /// scan asset info files
        void Scan(const std::filesystem::path &path)
        {
            // one listing per folder, the sibling .info files are looked up in memory
            const std::vector<FileSystem::FileEntry> entries = FileSystem::ListFiles(path);
            std::unordered_set<std::string> names;
            names.reserve(entries.size());
            for (const auto &entry : entries)
            {
                names.insert(entry.mName);
            }

            for (const auto &entry : entries)
            {
                const std::string entryPath = Normalize(path / entry.mName);
                if (!IsInfoName(entry.mName) && names.contains(entry.mName + ".info"))
                {
                    Register(entryPath);
                }
                if (entry.mAttributes.mFlags.Has(FileSystem::Flag::Directory))
                {
                    Scan(entryPath);
                }
            }
        }

        /// register an asset factory
        void RegisterFactory(const AssetFactoryPtr &assetFactory)
        {
            // insert asset factory with the given priority
            mAssetFactories.emplace(assetFactory->Priority(), assetFactory);
        }

        /// register the asset type of an extension
        void RegisterType(std::string_view extension, HashType type)
        {
            mAssetTypes.insert_or_assign(ToLower(std::string(extension)), type);
        }

        /// generates a UUID that is not used by any known asset
        UUID NewUUID() const
        {
            UUID uuid;
            do
            {
                uuid = UUIDGenerate();
            }
            while (mAssetPaths.contains(uuid));
            return uuid;
        }

        /// register asset info, reads the UUID from the .info file and maps it to the asset
        void Register(const std::string &path)
        {
            const std::string key = Normalize(path);
            if (mAssetUUIDs.contains(key))
            {
                return;
            }

            const std::filesystem::path infoFile = std::filesystem::path(key).concat(".info");
            Lumen::UUID uuid = UUID_INVALID;
            Serialized::Type data;
            if (!FileSystem::ReadAssetInfo(infoFile, uuid, &data) || (uuid == UUID_INVALID))
            {
                Lumen::DebugLog::Warning("Unable to register asset info {}", infoFile.generic_string());
                return;
            }

            // a UUID must identify one asset, copying an asset together with its .info file creates a duplicate
            auto owner = mAssetPaths.find(uuid);
            if ((owner != mAssetPaths.end()) && (owner->second != key) && FileSystem::Exists(owner->second))
            {
#ifdef EDITOR
                const Lumen::UUID fresh = NewUUID();
                if (!FileSystem::WriteAssetInfo(infoFile, fresh, data))
                {
                    Lumen::DebugLog::Error("Duplicate UUID of {} and {}, unable to assign a new UUID to {}", owner->second, key, key);
                    return;
                }
                Lumen::DebugLog::Warning("Duplicate UUID of {} and {}, {} got a new UUID", owner->second, key, key);
                uuid = fresh;
#else
                Lumen::DebugLog::Error("Duplicate UUID of {} and {}, ignoring {}", owner->second, key, key);
                return;
#endif
            }

            mAssetPaths[uuid] = key;
            mAssetUUIDs[key] = uuid;
        }

        /// drops what is cached for a path that changed on disk: the loaded instance and the asset info
        void Invalidate(const std::string &path)
        {
            const std::string key = Normalize(path);
            mLoaded.erase(key);
            mAssetInfos.erase(key);
        }

        /// rename asset info
        void Rename(const std::string &oldPath, const std::string &newPath)
        {
            const std::string oldKey = Normalize(oldPath);
            const std::string newKey = Normalize(newPath);

            // instances and infos are keyed by path (and an asset knows its path), they are recreated on demand
            Invalidate(oldKey);
            Invalidate(newKey);

            // the UUID stays, so does the Library entry
            auto assetUUIDIt = mAssetUUIDs.find(oldKey);
            if (assetUUIDIt != mAssetUUIDs.end())
            {
                const Lumen::UUID uuid = assetUUIDIt->second;
                mAssetUUIDs.erase(assetUUIDIt);
                mAssetUUIDs[newKey] = uuid;
                mAssetPaths[uuid] = newKey;
            }
        }

        /// remove asset info
        void Remove(const std::string &path)
        {
            const std::string key = Normalize(path);
            Invalidate(key);

            auto assetUUIDIt = mAssetUUIDs.find(key);
            if (assetUUIDIt != mAssetUUIDs.end())
            {
                const Lumen::UUID uuid = assetUUIDIt->second;
                mAssetPaths.erase(uuid);
                mAssetUUIDs.erase(assetUUIDIt);
#ifdef EDITOR
                // the cached data of an asset that is gone (or is about to be given a new UUID) is garbage
                FileSystem::Delete(AssetManager::LibraryPath(uuid));
#endif
            }
        }

        /// create or get assetinfo
        Expected<AssetInfoPtr> GetInfo(const std::filesystem::path &path)
        {
            const std::string key = Normalize(path);
            auto assetInfoIt = mAssetInfos.find(key);
            if (assetInfoIt != mAssetInfos.end())
            {
                if (auto ptr = assetInfoIt->second.lock())
                {
                    return ptr;
                }
                mAssetInfos.erase(assetInfoIt);
            }

            // custom deleter, cleans up the cache when the last AssetInfoPtr is destroyed
            auto deleter = [this, key](AssetInfo *ptr)
            {
                delete ptr;
                mAssetInfos.erase(key);
            };
            AssetInfoPtr assetInfoPtr = AssetInfo::MakePtr(key, deleter);

            const std::filesystem::path infoFile = std::filesystem::path(key).concat(".info");
            if (!FileSystem::Exists(infoFile))
            {
#ifdef EDITOR
                // register assetinfo
                do
                {
                    assetInfoPtr->Initialize();
                }
                while (mAssetPaths.contains(assetInfoPtr->UUID()));
                if (!assetInfoPtr->Save())
                {
                    return Expected<AssetInfoPtr>::Unexpected("Failed to save asset info");
                }

                // register assetinfo in caches
                mAssetPaths[assetInfoPtr->UUID()] = key;
                mAssetUUIDs[key] = assetInfoPtr->UUID();

                DebugLog::Info("Created asset info file: {}", infoFile.generic_string());
#else
                // a game build never writes into the assets
                return Expected<AssetInfoPtr>::Unexpected(std::format("Missing asset info file, {}", infoFile.generic_string()));
#endif
            }
            else
            {
                // resolves a duplicate UUID (which rewrites the .info file) before the info is read
                Register(key);
                if (!assetInfoPtr->Load())
                {
                    return Expected<AssetInfoPtr>::Unexpected("Failed to load asset info");
                }
            }

            mAssetInfos[key] = assetInfoPtr;
            return assetInfoPtr;
        }

        /// update asset info
        bool UpdateAssetInfo(const std::string &path)
        {
            auto assetInfoExpected = GetInfo(path);
            if (!assetInfoExpected.HasValue())
            {
                DebugLog::Error("Unable to acquire asset info for file: {}, error: {}", path, assetInfoExpected.Error());
                return false;
            }
            return true;
        }

        /// load asset
        Expected<AssetPtr> Load(HashType type, const std::filesystem::path &path)
        {
            const std::string key = Normalize(path);

            // shared instance still alive?
            auto loadedIt = mLoaded.find(key);
            if (loadedIt != mLoaded.end())
            {
                if (AssetPtr loaded = loadedIt->second.lock())
                {
                    if (static_cast<Hash>(loaded->Type()) == static_cast<Hash>(type))
                    {
                        return loaded;
                    }
                }
                else
                {
                    mLoaded.erase(loadedIt);
                }
            }

            // ask the factories in priority order, keep the reason of the last failure
            std::string lastError;
            for (auto &priorityFactory : mAssetFactories)
            {
                auto &assetFactory = priorityFactory.second;
                if (!assetFactory->Exists(key))
                {
                    continue;
                }
                auto assetExpected = assetFactory->Load(mEngine, type, key);
                if (!assetExpected)
                {
                    lastError = assetExpected.Error();
                    continue;
                }

                AssetPtr asset = assetExpected.Value();
                auto uuidIt = mAssetUUIDs.find(key);
                if (uuidIt != mAssetUUIDs.end())
                {
                    asset->SetUUID(uuidIt->second);
                }
                if (asset->Shareable())
                {
                    PurgeLoaded();
                    mLoaded[key] = asset;
                }
                return asset;
            }

            return Expected<AssetPtr>::Unexpected(lastError.empty() ? std::format("Asset not found, {}", key) : lastError);
        }

        /// load asset from asset info
        Expected<AssetPtr> Load(const AssetInfoPtr &assetInfo)
        {
            if (!assetInfo)
            {
                return Expected<AssetPtr>::Unexpected("Invalid asset info");
            }
            const std::string extension = ToLower(assetInfo->Path().extension().generic_string());
            auto typeIt = mAssetTypes.find(extension);
            if (typeIt == mAssetTypes.end())
            {
                return Expected<AssetPtr>::Unexpected(std::format("No asset type registered for '{}', {}", extension, assetInfo->Path().generic_string()));
            }
            return Load(typeIt->second, assetInfo->Path());
        }

        /// asset info of an asset that lives in a file system, null for procedural assets (and assets that do not exist)
        AssetInfoPtr InfoOf(const Asset &asset)
        {
            const std::string key = Normalize(asset.Path());
            if (!FileSystem::Exists(key))
            {
#ifdef EDITOR
                // no source, no asset (and no stray .info file for a path that was never an asset)
                return nullptr;
#else
                // a game build may ship the Library without the sources, the asset info alone identifies the entry
                if (!FileSystem::Exists(std::filesystem::path(key).concat(".info")))
                {
                    return nullptr;
                }
#endif
            }
            auto assetInfoExpected = GetInfo(key);
            if (!assetInfoExpected.HasValue())
            {
                DebugLog::Warning("Unable to acquire asset info for file: {}, error: {}", key, assetInfoExpected.Error());
                return nullptr;
            }
            return assetInfoExpected.Value();
        }

        /// load the state of an asset from its library entry
        bool LoadFromLibrary(Asset &asset)
        {
            if (!asset.SupportsLibrary())
            {
                return false;
            }
            AssetInfoPtr assetInfo = InfoOf(asset);
            if (!assetInfo)
            {
                return false;
            }

            Serialized::Type data;
            if (!ReadLibraryEntry(assetInfo->UUID(), static_cast<Hash>(asset.Type()), assetInfo->Path().generic_string(), data))
            {
                return false;
            }
            try
            {
                asset.Deserialize(data, true);
            }
            catch (const std::exception &e)
            {
                // fall back to the source, which then replaces the entry
                Lumen::DebugLog::Warning("Library entry of {} is unusable, {}", assetInfo->Path().generic_string(), e.what());
                return false;
            }
            asset.SetUUID(assetInfo->UUID());
            Lumen::DebugLog::Detail("Loaded {} from the Library", assetInfo->Path().generic_string());
            return true;
        }

        /// write the library entry of an asset
        bool UpdateLibrary(const Asset &asset)
        {
            if (!asset.SupportsLibrary())
            {
                return false;
            }
            AssetInfoPtr assetInfo = InfoOf(asset);
            if (!assetInfo)
            {
                return false;
            }

            Serialized::Type data;
            try
            {
                asset.Serialize(data, true);
            }
            catch (const std::exception &e)
            {
                // an asset can be unable to serialize at a given moment (a texture whose pixels were already uploaded)
                Lumen::DebugLog::Detail("Library entry of {} not written, {}", assetInfo->Path().generic_string(), e.what());
                return false;
            }
            return WriteLibraryEntry(assetInfo->UUID(), static_cast<Hash>(asset.Type()), assetInfo->Path().generic_string(), data);
        }

        /// load the state of an asset, library first
        bool LoadSerialized(Asset &asset)
        {
            if (LoadFromLibrary(asset))
            {
                return true;
            }

            // read and deserialize the source
            const std::filesystem::path &path = asset.Path();
            Serialized::Type data;
            if (!FileSystem::ReadSerializedData(path, data))
            {
                return false;
            }
            try
            {
                asset.Deserialize(data, FileSystem::IsPacked(path));
            }
            catch (const std::exception &e)
            {
                Lumen::DebugLog::Error("Unable to deserialize {}, {}", path.generic_string(), e.what());
                return false;
            }

            // refresh the library
            if (asset.SupportsLibrary())
            {
                UpdateLibrary(asset);
                auto uuidIt = mAssetUUIDs.find(Normalize(path));
                if (uuidIt != mAssetUUIDs.end())
                {
                    asset.SetUUID(uuidIt->second);
                }
            }
            return true;
        }

#ifdef EDITOR
        /// register an importer
        void RegisterImporter(std::string_view extension, AssetManager::Importer importer)
        {
            mImporters.insert_or_assign(ToLower(std::string(extension)), std::move(importer));
        }

        /// import asset
        bool Import(const AssetInfoPtr &assetInfo)
        {
            if (!assetInfo)
            {
                return false;
            }
            auto importerIt = mImporters.find(ToLower(assetInfo->Path().extension().generic_string()));
            if (importerIt != mImporters.end())
            {
                return importerIt->second(assetInfo);
            }

            // no importer, the library is validated when an entry is used, here a stale entry is dropped so it does not linger
            const std::filesystem::path entryPath = AssetManager::LibraryPath(assetInfo->UUID());
            std::vector<uint8_t> bytes;
            if (ReadFileBytes(entryPath, bytes))
            {
                LibraryHeader header;
                size_t bodyOffset = 0;
                const SourceStamp stamp = GetSourceStamp(assetInfo->Path().generic_string());
                if (!ParseLibraryHeader(bytes, header, bodyOffset) || (header.mVersion != cLibraryVersion) ||
                    (stamp.mExists && ((stamp.mSize != header.mSourceSize) || (stamp.mTime != header.mSourceTime))))
                {
                    FileSystem::Delete(entryPath);
                }
            }
            return true;
        }

        /// import the asset of a file change
        void ImportAsset(const std::string &name, const FileSystem::FileAttributes &attributes)
        {
            if (!attributes.mFlags.Has(FileSystem::Flag::File))
            {
                return;
            }
            auto assetInfoExpected = GetInfo(name);
            if (assetInfoExpected.HasValue())
            {
                Import(assetInfoExpected.Value());
            }
        }

        /// removes Library entries that no asset owns (assets deleted while the editor was not running, interrupted writes)
        void CollectLibraryGarbage()
        {
            // nothing registered usually means the assets could not be read at all, never wipe the cache in that case
            if (mAssetPaths.empty())
            {
                return;
            }

            size_t removed = 0;
            for (const auto &folder : FileSystem::ListFiles("Library/"))
            {
                // only touch what this manager creates: folders of two hex digits
                if (!folder.mAttributes.mFlags.Has(FileSystem::Flag::Directory) || (folder.mName.size() != 2))
                {
                    continue;
                }
                const std::string folderPath = "Library/" + folder.mName;
                for (const auto &file : FileSystem::ListFiles(folderPath))
                {
                    Lumen::UUID uuid = UUID_INVALID;
                    bool isTemp = false;
                    if (!ParseLibraryName(file.mName, uuid, isTemp))
                    {
                        continue;
                    }
                    if (isTemp || !mAssetPaths.contains(uuid))
                    {
                        removed += FileSystem::Delete(folderPath + "/" + file.mName) ? 1 : 0;
                    }
                }

                // succeeds only when the folder is empty
                FileSystem::Delete(folderPath);
            }
            if (removed > 0)
            {
                Lumen::DebugLog::Info("Removed {} unused Library entries", removed);
            }
        }

        /// process file changes
        void ProcessFileChanges(std::vector<FileSystem::FileChange> &&fileBatch)
        {
            auto nowTime = steady_clock::now();
            std::vector<AssetManager::AssetChange> assetBatch;
            std::unordered_set<std::string> newSkipChanges;

            for (auto &fileChange : fileBatch)
            {
                if (mSkipChanges.contains(fileChange.mName))
                {
                    mSkipChanges.erase(fileChange.mName);
                    continue;
                }

                // determine base and info file existence
                auto ext = std::filesystem::path(fileChange.mName).extension().generic_string();
                bool isBase = (ext.size() != 5) ||
                    !std::equal(ext.begin() + 1, ext.end(), "info", [](unsigned char a, unsigned char b) { return std::tolower(a) == b; });
                bool hasBase = false;
                bool hasInfo = false;
                if (isBase)
                {
                    hasBase = FileSystem::Exists(std::filesystem::path(fileChange.mName));
                    hasInfo = FileSystem::Exists(std::filesystem::path(fileChange.mName + ".info"));
                }
                else
                {
                    hasBase = FileSystem::Exists(std::filesystem::path(fileChange.mName.substr(0, fileChange.mName.size() - 5)));
                    hasInfo = FileSystem::Exists(std::filesystem::path(fileChange.mName));
                }

                // pre-process file change
                AssetManager::AssetChange assetChange;
                switch (fileChange.mChange)
                {
                case FileSystem::Change::Added:
                    if (isBase)
                    {
                        assetChange = { FileSystem::Change::Added, fileChange.mAttributes, fileChange.mName, "" };
                        if (hasInfo)
                        {
                            assetBatch.push_back(assetChange);
                            Register(assetChange.mName);
                        }
                        else
                        {
                            mAssetChangeMap.emplace(nowTime + InfoDelayTime, assetChange);
                        }
                    }
                    else
                    {
                        if (hasBase)
                        {
                            std::string name = fileChange.mName.substr(0, fileChange.mName.size() - 5);

                            // update last write time to the latest of the base and info file
                            FileSystem::FileAttributes attributes = FileSystem::GetAttributes(name);
                            attributes.mLastWrite =
                                attributes.mLastWrite > fileChange.mAttributes.mLastWrite ? attributes.mLastWrite : fileChange.mAttributes.mLastWrite;

                            assetBatch.push_back({ FileSystem::Change::Modified, attributes, name, "" });
                            Register(name);
                            ImportAsset(name, attributes);
                        }
                    }
                    break;
                case FileSystem::Change::Modified:
                    if (isBase)
                    {
                        if (hasInfo)
                        {
                            // the source changed, loaded instances and cached infos are out of date and the Library entry is stale
                            Invalidate(fileChange.mName);
                            ImportAsset(fileChange.mName, fileChange.mAttributes);
                            assetBatch.push_back({ FileSystem::Change::Modified, fileChange.mAttributes, fileChange.mName, "" });
                        }
                        else
                        {
                            mAssetChangeMap.emplace(nowTime + InfoDelayTime, AssetManager::AssetChange
                                { FileSystem::Change::Added, fileChange.mAttributes, fileChange.mName, "" });
                        }
                    }
                    else
                    {
                        if (hasBase)
                        {
                            std::string name = fileChange.mName.substr(0, fileChange.mName.size() - 5);
                            Invalidate(name);
                            assetBatch.push_back({ FileSystem::Change::Modified, fileChange.mAttributes, name, "" });
                        }
                    }
                    break;
                case FileSystem::Change::Renamed:
                    if (isBase)
                    {
                        if (hasInfo)
                        {
                            assetBatch.push_back({ FileSystem::Change::Renamed, fileChange.mAttributes, fileChange.mName, fileChange.mOldName });
                        }
                        else
                        {
                            std::string name = fileChange.mName + ".info";
                            std::string oldName = fileChange.mOldName + ".info";
                            if (FileSystem::Exists(oldName))
                            {
                                FileSystem::Rename(oldName, name);
                                assetBatch.push_back({ FileSystem::Change::Renamed, fileChange.mAttributes, fileChange.mName, fileChange.mOldName });
                                newSkipChanges.insert(name);
                            }
                            else
                            {
                                assetChange = { FileSystem::Change::Added, fileChange.mAttributes, fileChange.mName, "" };
                                mAssetChangeMap.emplace(nowTime + InfoDelayTime, assetChange);
                            }
                        }
                        Rename(fileChange.mOldName, fileChange.mName);
                    }
                    else
                    {
                        std::string name = fileChange.mName.substr(0, fileChange.mName.size() - 5);
                        std::string oldName = fileChange.mOldName.substr(0, fileChange.mOldName.size() - 5);
                        if (hasBase)
                        {
                            assetBatch.push_back({ FileSystem::Change::Renamed, fileChange.mAttributes, name, oldName });
                        }
                        else
                        {
                            if (FileSystem::Exists(oldName))
                            {
                                FileSystem::Rename(oldName, name);
                                assetBatch.push_back({ FileSystem::Change::Renamed, fileChange.mAttributes, name, oldName });
                                newSkipChanges.insert(name);
                            }
                            else
                            {
                                assetChange = { FileSystem::Change::Added, fileChange.mAttributes, name, "" };
                                mAssetChangeMap.emplace(nowTime + InfoDelayTime, assetChange);
                            }
                        }
                        Rename(oldName, name);
                    }
                    break;
                case FileSystem::Change::Removed:
                    if (isBase)
                    {
                        assetBatch.push_back({ FileSystem::Change::Removed, fileChange.mAttributes, fileChange.mName, "" });
                        Remove(fileChange.mName);
                    }
                    else
                    {
                        std::string name = fileChange.mName.substr(0, fileChange.mName.size() - 5);
                        if (hasBase)
                        {
                            assetBatch.push_back({ FileSystem::Change::Removed, fileChange.mAttributes, name, "" });
                            AssetManager::AssetChange assetChange = { FileSystem::Change::Added, fileChange.mAttributes, name, "" };
                            if (hasInfo)
                            {
                                Lumen::DebugLog::Detail("Added file with infofile, {}, infofile already exists", assetChange.mName);
                                assetBatch.push_back(assetChange);
                            }
                            else
                            {
                                mAssetChangeMap.emplace(nowTime + InfoDelayTime, assetChange);
                            }
                        }
                        Remove(name);
                    }
                    break;
                }
            }

            // process delayed asset changes
            for (auto it = mAssetChangeMap.begin(); it != mAssetChangeMap.end(); )
            {
                auto &[processTime, assetChange] = *it;

                if (FileSystem::Exists(assetChange.mName + ".info"))
                {
                    Lumen::DebugLog::Detail("Added file with infofile, {}, infofile late arrival", assetChange.mName);
                    assetBatch.push_back(assetChange);
                    it = mAssetChangeMap.erase(it);
                }
                else if (processTime <= nowTime)
                {
                    Lumen::DebugLog::Detail("Added file without infofile, {}, created infofile", assetChange.mName);
                    if (UpdateAssetInfo(assetChange.mName))
                    {
                        assetBatch.push_back(assetChange);
                    }
                    it = mAssetChangeMap.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            // final processing of asset changes
            if (!assetBatch.empty())
            {
                if (auto engineLock = mEngine.lock())
                {
                    engineLock->ProcessAssetChanges(std::move(assetBatch));
                }
            }

            // update skip changes list
            mSkipChanges = newSkipChanges;
        }
#endif

    private:
        /// drop expired instances when the cache has grown a lot, keeps the cost of the cache bounded without scanning on every load
        void PurgeLoaded()
        {
            if (mLoaded.size() < mPurgeThreshold)
            {
                return;
            }
            std::erase_if(mLoaded, [](const auto &entry) { return entry.second.expired(); });
            mPurgeThreshold = std::max<size_t>(64, mLoaded.size() * 2);
        }

        /// engine pointer
        EngineWeakPtr mEngine;

        /// asset factories
        std::multimap<float, AssetFactoryPtr, std::greater<float>> mAssetFactories;

        /// asset types by (lower case) extension
        std::unordered_map<std::string, HashType> mAssetTypes;

        /// asset infos
        std::unordered_map<std::string, AssetInfoWeakPtr> mAssetInfos;

        /// shared assets that are still alive by normalized path
        std::unordered_map<std::string, AssetWeakPtr> mLoaded;

        /// size of mLoaded that triggers the next purge of expired entries
        size_t mPurgeThreshold = 64;

        /// asset info paths by UUID (normalized paths)
        std::unordered_map<Lumen::UUID, std::string> mAssetPaths;

        /// asset info uuids by normalized path
        std::unordered_map<std::string, Lumen::UUID> mAssetUUIDs;

#ifdef EDITOR
        /// importers by (lower case) extension
        std::unordered_map<std::string, AssetManager::Importer> mImporters;

        /// temporary skip changes list, used to avoid processing the same file change multiple times during renames or similar
        std::unordered_set<std::string> mSkipChanges;

        /// file change mapFileChange
        std::multimap<steady_clock::time_point, AssetManager::AssetChange> mAssetChangeMap;
#endif
    };

    /// global asset manager implementation pointer
    static AssetManagerImplUniquePtr gAssetManagerImpl;
}

//==============================================================================================================================================================================

/// initialize assets namespace
void Lumen::AssetManager::Initialize(const EngineWeakPtr &engine)
{
    L_ASSERT(!Hidden::gAssetManagerImpl);
    Hidden::gAssetManagerImpl = Hidden::AssetManagerImpl::MakeUniquePtr(engine);
}

/// shutdown assets namespace
void AssetManager::Shutdown()
{
    L_ASSERT(Hidden::gAssetManagerImpl);
    Hidden::gAssetManagerImpl.reset();
}

/// register an asset factory
void AssetManager::RegisterFactory(const AssetFactoryPtr &assetFactory)
{
    Hidden::gAssetManagerImpl->RegisterFactory(assetFactory);
}

/// register the asset type stored in files with the given extension
void AssetManager::RegisterType(std::string_view extension, HashType type)
{
    Hidden::gAssetManagerImpl->RegisterType(extension, type);
}

/// scan assets
void AssetManager::Scan()
{
    Hidden::gAssetManagerImpl->Scan("Assets/");
    Hidden::gAssetManagerImpl->Scan("BuiltIn/");
#ifdef EDITOR
    Hidden::gAssetManagerImpl->CollectLibraryGarbage();
#endif
}

/// create or get cached assetinfo
Expected<AssetInfoPtr> AssetManager::GetInfo(const std::filesystem::path &path)
{
    return Hidden::gAssetManagerImpl->GetInfo(path);
}

/// load asset
Expected<AssetPtr> AssetManager::Load(HashType type, const std::filesystem::path &path)
{
    return Hidden::gAssetManagerImpl->Load(type, path);
}

/// load asset from asset info
Expected<AssetPtr> AssetManager::Load(const AssetInfoPtr &assetInfo)
{
    return Hidden::gAssetManagerImpl->Load(assetInfo);
}

/// path of the Library entry of an asset UUID
std::filesystem::path AssetManager::LibraryPath(UUID uuid)
{
    // hex only (no path separators, no case collisions), the first byte fans the entries out over 256 folders
    return std::format("Library/{:02x}/{:016x}", static_cast<unsigned>(uuid >> 56), uuid);
}

/// load the state of an asset from its Library entry
bool AssetManager::LoadFromLibrary(Asset &asset)
{
    return Hidden::gAssetManagerImpl->LoadFromLibrary(asset);
}

/// load the state of an asset, library first, then the source
bool AssetManager::LoadSerialized(Asset &asset)
{
    return Hidden::gAssetManagerImpl->LoadSerialized(asset);
}

/// write the Library entry of an asset
bool AssetManager::UpdateLibrary(const Asset &asset)
{
    return Hidden::gAssetManagerImpl->UpdateLibrary(asset);
}

#ifdef EDITOR
/// register an importer for a specific asset extension
void AssetManager::RegisterImporter(std::string_view extension, Importer importer)
{
    Hidden::gAssetManagerImpl->RegisterImporter(extension, std::move(importer));
}

/// import asset
bool AssetManager::Import(const AssetInfoPtr &assetInfo)
{
    return Hidden::gAssetManagerImpl->Import(assetInfo);
}

/// process file changes
void AssetManager::ProcessFileChanges(std::vector<FileSystem::FileChange> &&fileBatch)
{
    L_ASSERT(Hidden::gAssetManagerImpl);
    Hidden::gAssetManagerImpl->ProcessFileChanges(std::move(fileBatch));
}
#endif
