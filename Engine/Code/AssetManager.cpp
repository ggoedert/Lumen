//==============================================================================================================================================================================
/// \file
/// \brief     AssetManager
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lAssetManager.h"
#include "lFileInfo.h"
#include "lLibrary.h"
#include "lEngine.h"

/// \cond
#include <unordered_set>
/// \endcond

using namespace Lumen;

constexpr milliseconds InfoDelayTime { 500 };

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    CLASS_UNIQUE_PTR_DEF(AssetManagerImpl);

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

        /// scan all asset folders
        void ScanAll()
        {
#ifndef EDITOR
            // a game build has no builtin infos, and may ship assets without their sources, the Library entries hold their asset infos
            ScanLibrary();
#endif
            Scan("Assets/");
#ifdef EDITOR
            // once per session, right after the first full scan
            if (!mGarbageCollected)
            {
                mGarbageCollected = true;
                CollectLibraryGarbage();
            }
#endif
        }

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

#ifdef EDITOR
        /// register the asset info of a builtin asset, defined in code, its UUID is in the reserved builtin range
        void RegisterBuiltin(const AssetInfoPtr &assetInfo)
        {
            const std::string key = Normalize(assetInfo->Path());
            const Lumen::UUID uuid = assetInfo->UUID();
            if (!assetInfo->Builtin())
            {
                Lumen::DebugLog::Error("Builtin asset {} has the UUID {}, outside of the reserved builtin range", key, UUIDToString(uuid));
                return;
            }
            if (mAssetUUIDs.contains(key))
            {
                Lumen::DebugLog::Error("Builtin asset {} is already registered", key);
                return;
            }
            auto owner = mAssetPaths.find(uuid);
            if (owner != mAssetPaths.end())
            {
                // static UUIDs are written in the source code, a clash is a programming error
                Lumen::DebugLog::Error("Builtin asset {} has the UUID {} of {}", key, UUIDToString(uuid), owner->second);
                return;
            }
            PinInfo(key, assetInfo);
        }
#else
        /// registers the asset infos stored in the Library entries, a FileInfo for each, including the ones of the builtin assets
        void ScanLibrary()
        {
            for (Lumen::UUID uuid : Library::Entries())
            {
                Serialized::Type entry;
                Serialized::Type infoData;
                if (!Library::Read(uuid, entry) ||
                    !Serialized::DeserializeValue(entry, true, Serialized::cAssetInfoToken, Serialized::cAssetInfoTokenPacked, infoData))
                {
                    Lumen::DebugLog::Warning("Library entry {} has no asset info, skipped", UUIDToString(uuid));
                    continue;
                }
                auto fileInfoExpected = FileInfo::Deserialize(infoData, true, [](FileInfo *ptr) { delete ptr; });
                if (!fileInfoExpected || (fileInfoExpected.Value()->UUID() != uuid))
                {
                    Lumen::DebugLog::Warning("Library entry {} has an unusable asset info, skipped", UUIDToString(uuid));
                    continue;
                }
                const std::string key = Normalize(fileInfoExpected.Value()->Path());
                if (mAssetUUIDs.contains(key) || mAssetPaths.contains(uuid))
                {
                    Lumen::DebugLog::Warning("Library entry {} duplicates the asset {}, skipped", UUIDToString(uuid), key);
                    continue;
                }
                PinInfo(key, fileInfoExpected.Value());
            }
        }
#endif

        /// generates a UUID that is not used by any known asset
        UUID NewUUID() const
        {
            UUID uuid;
            do
            {
                uuid = UUIDGenerate();
            }
            while ((uuid == UUID_INVALID) || mAssetPaths.contains(uuid));
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

            // a UUID must identify one asset, copying an asset together with its .info file creates a duplicate, the builtin range is reserved
            auto owner = mAssetPaths.find(uuid);
            const bool reserved = UUIDIsBuiltin(uuid);
            if (reserved || ((owner != mAssetPaths.end()) && (owner->second != key) && FileSystem::Exists(owner->second)))
            {
                const std::string other = reserved ? std::string("the builtin assets") : owner->second;
#ifdef EDITOR
                // the path registered later gets a new UUID, right for a copy made while the editor runs, for a copy made while it was
                // closed the scan order decides, telling the original apart needs a record of the previous owners (the future index)
                const Lumen::UUID fresh = NewUUID();
                if (!FileSystem::WriteAssetInfo(infoFile, fresh, data))
                {
                    Lumen::DebugLog::Error("Duplicate UUID of {} and {}, unable to assign a new UUID to {}", other, key, key);
                    return;
                }
                Lumen::DebugLog::Warning("Duplicate UUID of {} and {}, {} got a new UUID", other, key, key);
                uuid = fresh;
#else
                Lumen::DebugLog::Error("Duplicate UUID of {} and {}, ignoring {}", other, key, key);
                return;
#endif
            }

            // the previous owner of the UUID is gone (the asset was moved while nothing was watching, or a Library entry still has its old
            // path), its path no longer names the asset
            auto previous = mAssetPaths.find(uuid);
            if ((previous != mAssetPaths.end()) && (previous->second != key))
            {
                mAssetUUIDs.erase(previous->second);
                mAssetInfos.erase(previous->second);
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
                Library::Remove(uuid);
#endif
            }
        }

        /// create or get assetinfo
        Expected<AssetInfoPtr> GetAssetInfo(const std::filesystem::path &path)
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
            // it goes through the global pointer (not this) so an info that outlives the asset manager is still safe to destroy,
            // and only drops an expired entry, the key may already map to a newer info (after an Invalidate)
            auto deleter = [key](FileInfo *ptr)
            {
                delete ptr;
                ForgetAssetInfo(key);
            };

            // the asset type comes from the extension of the source
            auto typeIt = mAssetTypes.find(ToLower(std::filesystem::path(key).extension().generic_string()));
            HashType hashType = (typeIt != mAssetTypes.end()) ? typeIt->second : GetHashType(HASH_INVALID);
            FileInfoPtr assetInfoPtr = FileInfo::MakePtr(key, hashType, deleter);

            const std::filesystem::path infoFile = std::filesystem::path(key).concat(".info");
            if (!FileSystem::Exists(infoFile))
            {
#ifdef EDITOR
                // only an existing source gets an asset info file
                if (!FileSystem::Exists(key))
                {
                    return Expected<AssetInfoPtr>::Unexpected(std::format("Asset not found, {}", key));
                }

                // register assetinfo
                do
                {
                    assetInfoPtr->Initialize();
                }
                while ((assetInfoPtr->UUID() == UUID_INVALID) || mAssetPaths.contains(assetInfoPtr->UUID()));
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
            auto assetInfoExpected = GetAssetInfo(path);
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
                AssignUUID(*asset);
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
            if (static_cast<Hash>(assetInfo->Type()) == HASH_INVALID)
            {
                return Expected<AssetPtr>::Unexpected(std::format("No asset type registered for {}", assetInfo->Path().generic_string()));
            }
            return Load(assetInfo->Type(), assetInfo->Path());
        }

        /// load the state of an asset from its library entry
        bool LoadFromLibrary(Asset &asset)
        {
            if (!asset.SupportsLibrary())
            {
                return false;
            }
            AssetInfoPtr assetInfo = AssetInfoOf(asset);
            if (!assetInfo)
            {
                return false;
            }

            // Library entries are always packed, they hold the asset info and the asset
            Serialized::Type entry;
            if (!Library::Read(assetInfo->UUID(), assetInfo->Path(), entry))
            {
                return false;
            }
            Serialized::Type infoData;
            Serialized::Type data;
            if (!Serialized::DeserializeValue(entry, true, Serialized::cAssetInfoToken, Serialized::cAssetInfoTokenPacked, infoData) ||
                !Serialized::DeserializeValue(entry, true, Serialized::cAssetToken, Serialized::cAssetTokenPacked, data))
            {
                Lumen::DebugLog::Warning("Library entry of {} is broken", assetInfo->Path().generic_string());
                return false;
            }

            // the stored asset info must be the one of the asset, an entry written before a rename still has the old path
            Lumen::UUID entryUUID = UUID_INVALID;
            std::filesystem::path entryPath;
            Hash entryType = HASH_INVALID;
            if (!AssetInfo::Deserialize(infoData, true, entryUUID, entryPath, entryType))
            {
                Lumen::DebugLog::Warning("Library entry of {} has an incomplete asset info, it is rebuilt", assetInfo->Path().generic_string());
                return false;
            }
            if ((entryUUID != assetInfo->UUID()) || (Normalize(entryPath) != Normalize(assetInfo->Path())) ||
                (entryType != static_cast<Hash>(assetInfo->Type())))
            {
                Lumen::DebugLog::Detail("Library entry of {} does not match its asset info", assetInfo->Path().generic_string());
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

        /// write the library entry of an asset, call after its source was written
        bool UpdateLibrary(const Asset &asset)
        {
            if (!asset.SupportsLibrary())
            {
                return false;
            }
            AssetInfoPtr assetInfo = AssetInfoOf(asset);
            if (!assetInfo)
            {
                return false;
            }

            Serialized::Type data;
            try
            {
                // Library entries are always packed
                asset.Serialize(data, true);
            }
            catch (const std::exception &e)
            {
                // an asset can be unable to serialize at a given moment (a texture whose pixels were already uploaded)
                Lumen::DebugLog::Detail("Library entry of {} not written, {}", assetInfo->Path().generic_string(), e.what());
                return false;
            }

            // the asset info goes with the asset, a game build recreates the asset info from the entry alone
            Serialized::Type infoData;
            assetInfo->Serialize(infoData, true);
            Serialized::Type entry = Serialized::Type::array();
            Serialized::SerializeValue(entry, true, Serialized::cAssetInfoToken, Serialized::cAssetInfoTokenPacked, infoData);
            Serialized::SerializeValue(entry, true, Serialized::cAssetToken, Serialized::cAssetTokenPacked, data);
            return Library::Write(assetInfo->UUID(), entry);
        }

        /// load the state of an asset, library first
        bool LoadSerialized(Asset &asset)
        {
            if (LoadFromLibrary(asset))
            {
                return true;
            }

            // no source to fall back to, a builtin asset (or one a game build only has in the Library)
            const std::filesystem::path &path = asset.Path();
            if (!FileSystem::Exists(path))
            {
                return false;
            }

            // note the source time before it is read, see below
            const auto sourceTime = FileSystem::GetAttributes(path).mLastWrite;

            // read and deserialize the source
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
            AssignUUID(asset);
            if (UpdateLibrary(asset) && (FileSystem::GetAttributes(path).mLastWrite != sourceTime))
            {
                // the source changed while it was read, the new entry is newer than the source but holds the old data
                Library::Remove(asset.GetUUID());
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

            // no importer, nothing to do: a stale Library entry is detected (and replaced) the next time the asset is loaded
            return true;
        }

        /// import the asset of a file change
        void ImportAsset(const std::string &name, const FileSystem::FileAttributes &attributes)
        {
            if (!attributes.mFlags.Has(FileSystem::Flag::File))
            {
                return;
            }
            auto assetInfoExpected = GetAssetInfo(name);
            if (assetInfoExpected.HasValue())
            {
                Import(assetInfoExpected.Value());
            }
        }

        /// removes Library entries that no asset owns (assets deleted while the editor was not running, interrupted writes)
        void CollectLibraryGarbage()
        {
            // no source asset registered usually means the assets could not be read at all, never touch the cache in that case
            if (mAssetPaths.size() <= mPinnedInfos.size())
            {
                return;
            }
            Library::CollectGarbage([this](UUID uuid) { return mAssetPaths.contains(uuid); });
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
                bool isBase = !IsInfoName(fileChange.mName);
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
                            // the source changed, loaded instances and cached infos are out of date (the Library entry is detected as stale when used)
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
                            assetChange = { FileSystem::Change::Added, fileChange.mAttributes, name, "" };
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
        /// drops the cache entry of an asset info that was destroyed, unless the key already maps to a newer info
        static void ForgetAssetInfo(const std::string &key);

        /// keeps an asset info alive for the lifetime of the asset manager, the builtin infos (editor) or the Library infos (game build)
        void PinInfo(const std::string &key, const AssetInfoPtr &assetInfo)
        {
            mAssetPaths[assetInfo->UUID()] = key;
            mAssetUUIDs[key] = assetInfo->UUID();
            mAssetInfos[key] = assetInfo;
            mPinnedInfos.push_back(assetInfo);
        }

        /// asset info of an asset, null for assets that do not exist
        AssetInfoPtr AssetInfoOf(const Asset &asset)
        {
            const std::string key = Normalize(asset.Path());

            // a pinned info has no source, a builtin asset (editor) or one a game build only has in the Library
            auto assetInfoIt = mAssetInfos.find(key);
            if (assetInfoIt != mAssetInfos.end())
            {
                if (auto ptr = assetInfoIt->second.lock())
                {
                    return ptr;
                }
            }

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
            auto assetInfoExpected = GetAssetInfo(key);
            if (!assetInfoExpected.HasValue())
            {
                DebugLog::Warning("Unable to acquire asset info for file: {}, error: {}", key, assetInfoExpected.Error());
                return nullptr;
            }
            return assetInfoExpected.Value();
        }

        /// gives an asset the UUID registered for its path, the only place (besides a Library load) that assigns UUIDs
        void AssignUUID(Asset &asset) const
        {
            auto uuidIt = mAssetUUIDs.find(Normalize(asset.Path()));
            if (uuidIt != mAssetUUIDs.end())
            {
                asset.SetUUID(uuidIt->second);
            }
        }

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

        /// asset infos kept alive for the lifetime of the asset manager, the builtin infos (editor) or the Library infos (game build)
        std::vector<AssetInfoPtr> mPinnedInfos;

#ifdef EDITOR
        /// importers by (lower case) extension
        std::unordered_map<std::string, AssetManager::Importer> mImporters;

        /// temporary skip changes list, used to avoid processing the same file change multiple times during renames or similar
        std::unordered_set<std::string> mSkipChanges;

        /// file change mapFileChange
        std::multimap<steady_clock::time_point, AssetManager::AssetChange> mAssetChangeMap;

        /// whether the Library garbage was collected this session
        bool mGarbageCollected = false;
#endif
    };

    /// global asset manager implementation pointer
    static AssetManagerImplUniquePtr gAssetManagerImpl;

    /// drops the cache entry of an asset info that was destroyed
    void AssetManagerImpl::ForgetAssetInfo(const std::string &key)
    {
        // the asset manager may already be shut down
        if (!gAssetManagerImpl)
        {
            return;
        }
        auto &assetInfos = gAssetManagerImpl->mAssetInfos;
        auto assetInfoIt = assetInfos.find(key);
        if ((assetInfoIt != assetInfos.end()) && assetInfoIt->second.expired())
        {
            assetInfos.erase(assetInfoIt);
        }
    }
}

//==============================================================================================================================================================================

/// initialize asset manager namespace, also initializes the Library namespace
void Lumen::AssetManager::Initialize(const EngineWeakPtr &engine)
{
    L_ASSERT(!Hidden::gAssetManagerImpl);
    Library::Initialize("Library/");
    Hidden::gAssetManagerImpl = Hidden::AssetManagerImpl::MakeUniquePtr(engine);
}

/// shutdown asset manager namespace, also shuts down the Library namespace
void AssetManager::Shutdown()
{
    L_ASSERT(Hidden::gAssetManagerImpl);
    Hidden::gAssetManagerImpl.reset();
    Library::Shutdown();
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

#ifdef EDITOR
/// register the asset info of a builtin asset
void AssetManager::RegisterBuiltin(const AssetInfoPtr &assetInfo)
{
    Hidden::gAssetManagerImpl->RegisterBuiltin(assetInfo);
}
#endif

/// scan assets
void AssetManager::Scan()
{
    Hidden::gAssetManagerImpl->ScanAll();
}

/// create or get cached assetinfo
Expected<AssetInfoPtr> AssetManager::GetAssetInfo(const std::filesystem::path &path)
{
    return Hidden::gAssetManagerImpl->GetAssetInfo(path);
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

/// load asset from its Library when there is no valid entry
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
