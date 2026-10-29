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

        /// generates a UUID that is not used by any known asset
        UUID NewUUID() const
        {
            UUID uuid;
            do
            {
                uuid = UUIDGenerate();
            } while ((uuid == UUID_INVALID) || mAssetPaths.contains(uuid));
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

        /// rename asset info
        void Rename(const std::string &oldPath, const std::string &newPath)
        {
            const std::string oldKey = Normalize(oldPath);
            const std::string newKey = Normalize(newPath);

            // the UUID stays
            auto assetUUIDIt = mAssetUUIDs.find(oldKey);
            if (assetUUIDIt != mAssetUUIDs.end())
            {
                // save mapped uuid and erase old path from cache
                Lumen::UUID uuid = assetUUIDIt->second;
                mAssetUUIDs.erase(assetUUIDIt);

                // find uuid in cache
                auto assetPathIt = mAssetPaths.find(uuid);
                if (assetPathIt != mAssetPaths.end())
                {
                    // update mapped path and insert uuid to cache
                    assetPathIt->second = newKey;
                    mAssetUUIDs[newKey] = uuid;
                }
            }
        }

        /// remove asset info
        void Remove(const std::string &path)
        {
            const std::string key = Normalize(path);

            auto assetUUIDIt = mAssetUUIDs.find(key);
            if (assetUUIDIt != mAssetUUIDs.end())
            {
                const Lumen::UUID uuid = assetUUIDIt->second;
                mAssetPaths.erase(uuid);
                mAssetUUIDs.erase(assetUUIDIt);
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
                } while (mAssetPaths.contains(assetInfoPtr->UUID()));
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

        /// register an asset factory
        void RegisterFactory(const AssetFactoryPtr &assetFactory)
        {
            // insert asset factory with the given priority
            mAssetFactories.emplace(assetFactory->Priority(), assetFactory);
        }

        /// load asset
        Expected<AssetPtr> Load(HashType type, const std::filesystem::path &path)
        {
            const std::string key = Normalize(path);

            //@@REVIEW@@
            //cache asset in some map? make some mechanism that purges or keeps assets around depending on whats happening?

            // ask the factories in priority order, keep the reason of the last failure
            std::string lastError;
            for (auto &priorityFactory : mAssetFactories)
            {
                auto &assetFactory = priorityFactory.second;
                if (assetFactory->Exists(key))
                {
                    auto assetExpected = assetFactory->Load(mEngine, type, key);
                    if (assetExpected)
                    {
                        return assetExpected.Value();
                    }
                    lastError = assetExpected.Error();
                }
            }

            return Expected<AssetPtr>::Unexpected(lastError.empty() ? std::format("Asset not found, {}", key) : lastError);
        }

#ifdef EDITOR
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
                            if (attributes.mFlags.Has(FileSystem::Flag::File))
                            {
                                auto assetInfoExpected = GetInfo(name);
                                if (assetInfoExpected.HasValue())
                                {
                                    AssetManager::Import(assetInfoExpected.Value());
                                }
                            }
                        }
                    }
                    break;
                case FileSystem::Change::Modified:
                    if (isBase)
                    {
                        if (hasInfo)
                        {
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
        /// engine pointer
        EngineWeakPtr mEngine;

        /// asset factories
        std::multimap<float, AssetFactoryPtr, std::greater<float>> mAssetFactories;

        /// asset infos
        std::unordered_map<std::string, AssetInfoWeakPtr> mAssetInfos;

        /// asset info paths by UUID (normalized paths)
        std::unordered_map<Lumen::UUID, std::string> mAssetPaths;

        /// asset info uuids by normalized path
        std::unordered_map<std::string, Lumen::UUID> mAssetUUIDs;

#ifdef EDITOR
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

/// scan assets
void AssetManager::Scan()
{
    Hidden::gAssetManagerImpl->Scan("Assets/");
    Hidden::gAssetManagerImpl->Scan("BuiltIn/");
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

#ifdef EDITOR
/// register an importer for a specific asset type
void AssetManager::RegisterImporter(std::string &extension, Importer importer)
{
}

/// import asset
bool AssetManager::Import(const AssetInfoPtr &assetInfo)
{
    return false;
}

/// process file changes
void AssetManager::ProcessFileChanges(std::vector<FileSystem::FileChange> &&fileBatch)
{
    L_ASSERT(Hidden::gAssetManagerImpl);
    Hidden::gAssetManagerImpl->ProcessFileChanges(std::move(fileBatch));
}
#endif
