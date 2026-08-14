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
            for (const auto &entry : FileSystem::ListFiles(path))
            {
                auto entryPath = FileSystem::NormalizeFilePath(path / entry.mName);
                if (entryPath.extension() != ".info" && FileSystem::Exists(std::filesystem::path(entryPath).concat(".info")))
                {
                    Register(entryPath);
                }
                if (entry.mFlags.Has(FileSystem::Flag::Directory))
                {
                    Scan(entryPath);
                }
            }
        }

        /// register asset info
        void Register(const std::filesystem::path &path)
        {
            auto assetUUIDIt = mAssetUUIDs.find(path);
            if (assetUUIDIt == mAssetUUIDs.end())
            {
                Lumen::UUID uuid;
                if (FileSystem::ReadAssetInfo(std::filesystem::path(path).concat(".info"), uuid, nullptr))
                {
                    mAssetPaths[uuid] = path;
                    mAssetUUIDs[path] = uuid;
                }
            }
        }

        /// rename asset info
        void Rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath)
        {
            // find old path
            auto assetUUIDIt = mAssetUUIDs.find(oldPath);
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
                    assetPathIt->second = newPath;
                    mAssetUUIDs[newPath] = uuid;
                }
            }
        }

        /// remove asset info
        void Remove(const std::filesystem::path &path)
        {
            // find path
            auto assetUUIDIt = mAssetUUIDs.find(path);
            if (assetUUIDIt != mAssetUUIDs.end())
            {
                // erase uuid and path from cache
                mAssetPaths.erase(assetUUIDIt->second);
                mAssetUUIDs.erase(assetUUIDIt);
            }
        }

        /// create or get assetinfo
        Expected<AssetInfoPtr> GetInfo(const std::filesystem::path &path)
        {
            auto assetInfoIt = mAssetInfos.find(path);
            if (assetInfoIt != mAssetInfos.end())
            {
                if (auto ptr = assetInfoIt->second.lock())
                {
                    return ptr;
                }
                mAssetInfos.erase(assetInfoIt);
            }

            // custom deleter, cleans up the cache when the last AssetInfoPtr is destroyed
            auto deleter = [this, path](AssetInfo *ptr)
            {
                delete ptr;
                mAssetInfos.erase(path);
            };
            auto assetInfoPtr = AssetInfo::MakePtr(path, deleter);

            std::filesystem::path infoFile = std::filesystem::path(path).concat(".info");
            if (!FileSystem::Exists(infoFile))
            {
                // register assetinfo
                assetInfoPtr->Initialize();
                if (!assetInfoPtr->Save())
                {
                    return Expected<AssetInfoPtr>::Unexpected("Failed to save asset info");
                }

                // register assetinfo in caches
                mAssetPaths[assetInfoPtr->UUID()] = assetInfoPtr->Path();
                mAssetUUIDs[assetInfoPtr->Path()] = assetInfoPtr->UUID();

                DebugLog::Info("Created asset info file in Assets folder: {}", infoFile.string());
            }
            else
            {
                if (!assetInfoPtr->Load())
                {
                    return Expected<AssetInfoPtr>::Unexpected("Failed to load asset info");
                }
            }

            mAssetInfos[path] = assetInfoPtr;
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

        /// import asset
        Expected<AssetPtr> Import(HashType type, const std::filesystem::path &path)
        {
            // normalize the path
            std::string normalizedPath = FileSystem::NormalizeFilePath(path).string();

            //@@REVIEW@@
            //cache asset in some map? make some mechanism that purges or keeps assets around depending on whats happening?

            // get asset infos that are on the path from all factories
            for (auto &priorityFactory : mAssetFactories)
            {
                auto &assetFactory = priorityFactory.second;
                if (assetFactory->Exists(normalizedPath))
                {
                    auto assetExpected = assetFactory->Import(mEngine, type, normalizedPath);
                    if (assetExpected)
                    {
                        return assetExpected.Value();
                    }
                }
            }

            // not found
            return Expected<AssetPtr>::Unexpected("Asset not found");
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
                auto ext = std::filesystem::path(fileChange.mName).extension().string();
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
                        assetChange = { FileSystem::Change::Added, fileChange.mFlags, fileChange.mLastWrite, fileChange.mName, "" };
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
                            assetBatch.push_back({ FileSystem::Change::Modified, fileChange.mFlags, fileChange.mLastWrite, name, "" });
                            Register(name);
                        }
                    }
                    break;
                case FileSystem::Change::Modified:
                    if (isBase)
                    {
                        if (hasInfo)
                        {
                            assetBatch.push_back({ FileSystem::Change::Modified, fileChange.mFlags, fileChange.mLastWrite, fileChange.mName, "" });
                        }
                        else
                        {
                            mAssetChangeMap.emplace(nowTime + InfoDelayTime,
                                AssetManager::AssetChange { FileSystem::Change::Added, fileChange.mFlags, fileChange.mLastWrite, fileChange.mName, "" });
                        }
                    }
                    else
                    {
                        if (hasBase)
                        {
                            std::string name = fileChange.mName.substr(0, fileChange.mName.size() - 5);
                            assetBatch.push_back({ FileSystem::Change::Modified, fileChange.mFlags, fileChange.mLastWrite, name, "" });
                        }
                    }
                    break;
                case FileSystem::Change::Renamed:
                    if (isBase)
                    {
                        if (hasInfo)
                        {
                            assetBatch.push_back({ FileSystem::Change::Renamed, fileChange.mFlags, fileChange.mLastWrite, fileChange.mName, fileChange.mOldName });
                        }
                        else
                        {
                            std::string name = fileChange.mName + ".info";
                            std::string oldName = fileChange.mOldName + ".info";
                            if (FileSystem::Exists(std::filesystem::path(oldName)))
                            {
                                FileSystem::Rename(std::filesystem::path(oldName), std::filesystem::path(name));
                                assetBatch.push_back({ FileSystem::Change::Renamed, fileChange.mFlags, fileChange.mLastWrite, fileChange.mName, fileChange.mOldName });
                                newSkipChanges.insert(name);
                            }
                            else
                            {
                                assetChange = { FileSystem::Change::Added, fileChange.mFlags, fileChange.mLastWrite, fileChange.mName, "" };
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
                            assetBatch.push_back({ FileSystem::Change::Renamed, fileChange.mFlags, fileChange.mLastWrite, name, oldName });
                        }
                        else
                        {
                            if (FileSystem::Exists(std::filesystem::path(oldName)))
                            {
                                FileSystem::Rename(std::filesystem::path(oldName), std::filesystem::path(name));
                                assetBatch.push_back({ FileSystem::Change::Renamed, fileChange.mFlags, fileChange.mLastWrite, name, oldName });
                                newSkipChanges.insert(name);
                            }
                            else
                            {
                                assetChange = { FileSystem::Change::Added, fileChange.mFlags, fileChange.mLastWrite, name, "" };
                                mAssetChangeMap.emplace(nowTime + InfoDelayTime, assetChange);
                            }
                        }
                        Rename(oldName, name);
                    }
                    break;
                case FileSystem::Change::Removed:
                    if (isBase)
                    {
                        assetBatch.push_back({ FileSystem::Change::Removed, fileChange.mFlags, fileChange.mLastWrite, fileChange.mName, "" });
                        Remove(fileChange.mName);
                    }
                    else
                    {
                        std::string name = fileChange.mName.substr(0, fileChange.mName.size() - 5);
                        if (hasBase)
                        {
                            assetBatch.push_back({ FileSystem::Change::Removed, fileChange.mFlags, fileChange.mLastWrite, name, "" });
                            AssetManager::AssetChange assetChange = { FileSystem::Change::Added, fileChange.mFlags, fileChange.mLastWrite, name, "" };
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

                if (FileSystem::Exists(std::filesystem::path(assetChange.mName + ".info")))
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
        std::unordered_map<std::filesystem::path, AssetInfoWeakPtr> mAssetInfos;

        /// asset info paths
        std::unordered_map<Lumen::UUID, std::filesystem::path> mAssetPaths;

        /// asset info uuids
        std::unordered_map<std::filesystem::path, Lumen::UUID> mAssetUUIDs;

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
}

/// create or get cached assetinfo
Expected<AssetInfoPtr> AssetManager::GetInfo(const std::filesystem::path &path)
{
    return Hidden::gAssetManagerImpl->GetInfo(path);
}

/// import asset
Expected<AssetPtr> AssetManager::Import(HashType type, const std::filesystem::path &path)
{
    return Hidden::gAssetManagerImpl->Import(type, path);
}

#ifdef EDITOR
/// process file changes
void AssetManager::ProcessFileChanges(std::vector<FileSystem::FileChange> &&fileBatch)
{
    L_ASSERT(Hidden::gAssetManagerImpl);
    Hidden::gAssetManagerImpl->ProcessFileChanges(std::move(fileBatch));
}
#endif
