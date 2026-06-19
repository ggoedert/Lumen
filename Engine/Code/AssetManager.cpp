//==============================================================================================================================================================================
/// \file
/// \brief     AssetManager
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lAssetManager.h"
#include "lFileSystem.h"

using namespace Lumen;

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    CLASS_UNIQUE_PTR_DEF(AssetManagerImpl);

    /// AssetManagerImpl class
    class AssetManagerImpl
    {
        //CLASS_NO_DEFAULT_CTOR(AssetManagerImpl);
        CLASS_NO_COPY_MOVE(AssetManagerImpl);
        CLASS_PTR_UNIQUEMAKER(AssetManagerImpl);

    public:
        /// constructs an assets implementation with engine
        explicit AssetManagerImpl(/*const EngineWeakPtr &engine) : mEngine(engine*/) {}

        /// destroys assets
        ~AssetManagerImpl() = default;

    private:
        /// engine pointer
        //EngineWeakPtr mEngine;
    };

    /// global asset manager implementation pointer
    static AssetManagerImplUniquePtr gAssetManagerImpl;
}

//==============================================================================================================================================================================

/// initialize assets namespace
void Lumen::AssetManager::Initialize(/*const EngineWeakPtr &engine*/)
{
    L_ASSERT(!Hidden::gAssetManagerImpl);
    Hidden::gAssetManagerImpl = Hidden::AssetManagerImpl::MakeUniquePtr(/*engine*/);
}

/// shutdown assets namespace
void AssetManager::Shutdown()
{
    L_ASSERT(Hidden::gAssetManagerImpl);
    Hidden::gAssetManagerImpl.reset();
}


/// get asset infos from a path
bool AssetManager::GetAssetInfos(const std::string &path, std::vector<AssetInfoPtr> &assetInfos)
{
    DebugLog::Info("CHANGE AssetManager::GetAssetInfos to monitor path: {}", path);

    if (!path.size() || !FileSystem::Exists("Assets" + path))
    {
        return false;
    }
    std::vector<FileSystem::FileEntry> files = FileSystem::ListFiles("Assets" + path);
    for(const FileSystem::FileEntry &entry : files)
    {
        bool isInfo = false;
        const std::string::size_type pointPos = entry.mName.find_last_of('.');
        if (pointPos != std::string::npos)
        {
            isInfo = std::ranges::equal(std::string_view(entry.mName).substr(pointPos + 1), std::string_view("info"), [](char a, char b) { return std::tolower(a) == b; });
        }
        if (!isInfo)
        {
            std::string infoFile = "Assets" + entry.mName + std::string(".info");
            auto assetInfoExpected = AssetInfo::MakePtr("Assets" + entry.mName);
            if (!assetInfoExpected.HasValue())
            {
                throw std::runtime_error(std::format("Unable to make asset info for file: {}, error: {}", infoFile, assetInfoExpected.Error()));
            }
            auto &assetInfo = assetInfoExpected.Value();
            if (!FileSystem::Exists(infoFile))
            {
                assetInfo->Initialize();
                assetInfo->Save();
                DebugLog::Info("Create asset file in Assets folder: {}", infoFile);
            }
            else
            {
                assetInfo->Load();
            }
            assetInfos.push_back(assetInfo);
        }
    }
    return true;
}

// DEPRECATED, this is the old asset manager interface, we should move to the new one as soon as possible, but for now we keep it around for testing and reference

CLASS_UNIQUE_PTR_DEF(AssetManagerOldImpl);

/// AssetManagerOldImpl class
class AssetManagerOldImpl
{
    CLASS_NO_DEFAULT_CTOR(AssetManagerOldImpl);
    CLASS_NO_COPY_MOVE(AssetManagerOldImpl);
    CLASS_PTR_UNIQUEMAKER(AssetManagerOldImpl);

public:
    /// constructs an assets implementation with engine
    explicit AssetManagerOldImpl(const EngineWeakPtr &engine) : mEngine(engine) {}

    /// destroys assets
    ~AssetManagerOldImpl() = default;

    /// set engine
    void SetEngine(const EngineWeakPtr &engine);

    /// register an asset factory
    void RegisterFactory(const AssetFactoryOldPtr &assetFactory);

    /// import asset
    Expected<AssetPtr> Import(HashType type, const std::filesystem::path &path);

private:
    /// engine pointer
    EngineWeakPtr mEngine;

    /// asset factories
    std::multimap<float, AssetFactoryOldPtr, std::greater<float>> mAssetFactories;
};

/// register an asset factory
void AssetManagerOldImpl::RegisterFactory(const AssetFactoryOldPtr &assetFactory)
{
    // insert asset factory with the given priority
    mAssetFactories.emplace(assetFactory->Priority(), assetFactory);
}

/// import asset
Expected<AssetPtr> AssetManagerOldImpl::Import(HashType type, const std::filesystem::path &path)
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

//==============================================================================================================================================================================

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    static AssetManagerOldImplUniquePtr gAssetManagerOldImpl;
}

/// initialize assets namespace
void AssetManagerOld::Initialize(const EngineWeakPtr &engine)
{
    L_ASSERT(!Hidden::gAssetManagerOldImpl);
    Hidden::gAssetManagerOldImpl = AssetManagerOldImpl::MakeUniquePtr(engine);
}

/// shutdown assets namespace
void AssetManagerOld::Shutdown()
{
    L_ASSERT(Hidden::gAssetManagerOldImpl);
    Hidden::gAssetManagerOldImpl.reset();
}

/// register an asset factory
void AssetManagerOld::RegisterFactory(const AssetFactoryOldPtr &assetFactory)
{
    Hidden::gAssetManagerOldImpl->RegisterFactory(assetFactory);
}

/// import asset
Expected<AssetPtr> AssetManagerOld::Import(HashType type, const std::filesystem::path &path)
{
    return Hidden::gAssetManagerOldImpl->Import(type, path);
}
