//==============================================================================================================================================================================
/// \file
/// \brief     AssetManager
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lStringMap.h"
#include "lAsset.h"
#include "lAssetManager.h"

using namespace Lumen;

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

    /// set engine
    void SetEngine(const EngineWeakPtr &engine);

    /// register an asset factory
    void RegisterFactory(const AssetFactoryPtr &assetFactory);

    /// import asset
    Expected<AssetPtr> Import(HashType type, const std::filesystem::path &path);

private:
    /// engine pointer
    EngineWeakPtr mEngine;

    /// asset factories
    std::multimap<float, AssetFactoryPtr, std::greater<float>> mAssetFactories;
};

/// register an asset factory
void AssetManagerImpl::RegisterFactory(const AssetFactoryPtr &assetFactory)
{
    // insert asset factory with the given priority
    mAssetFactories.emplace(assetFactory->Priority(), assetFactory);
}

/// import asset
Expected<AssetPtr> AssetManagerImpl::Import(HashType type, const std::filesystem::path &path)
{
    // normalize the path
    std::string normalizedPath = FileSystem::NormalizeFilePath(path).string();

    // combined asset infos from all factories
    std::vector<AssetInfoPtr> combinedAssetInfos;

    // get asset infos that are on the path from all factories
    for (auto &priorityFactory : mAssetFactories)
    {
        auto &assetFactory = priorityFactory.second;
        std::vector<AssetInfoPtr> assetInfos = assetFactory->GetAssetInfos(normalizedPath);
        combinedAssetInfos.insert(combinedAssetInfos.end(), assetInfos.begin(), assetInfos.end());
    }

    for (AssetInfoPtr &assetInfo : combinedAssetInfos)
    {
        if (assetInfo->Type() == type)
        {
            return assetInfo->Import(mEngine);
        }
    }

    // none found, return empty
    return Expected<AssetPtr>::Unexpected("Asset Information not found");
}

//==============================================================================================================================================================================

/*/// factory options
struct FactoryOptions
{
    std::string_view mBaseDir {};
    std::string_view mFileName {};
    bool mRecursive = true;
};*/

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    static AssetManagerImplUniquePtr gAssetManagerImpl;
}

/// initialize assets namespace
void AssetManager::Initialize(const EngineWeakPtr &engine)
{
    L_ASSERT(!Hidden::gAssetManagerImpl);
    Hidden::gAssetManagerImpl = AssetManagerImpl::MakeUniquePtr(engine);
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

/// process asset changes
void AssetManager::ProcessAssetChanges(std::list<std::vector<AssetChange>> &&batchQueue)
{
    for (auto batch : batchQueue)
    {
        for (auto item: batch)
        {
            switch (item.change)
            {
            case AssetChange::Type::Added:
                Lumen::DebugLog::Info("Added: {}", item.name);
                break;
            case AssetChange::Type::Modified:
                Lumen::DebugLog::Info("Modified: {}", item.name);
                break;
            case AssetChange::Type::Renamed:
                Lumen::DebugLog::Info("Renamed: {} -> {}", item.oldName, item.name);
                break;
            case AssetChange::Type::Removed:
                Lumen::DebugLog::Info("Removed: {}", item.name);
                break;
            }
        }
    }
}

/// import asset
Expected<AssetPtr> AssetManager::Import(HashType type, const std::filesystem::path &path)
{
    return Hidden::gAssetManagerImpl->Import(type, path);
}
