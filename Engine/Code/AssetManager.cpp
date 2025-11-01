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

    /// register an asset info
    void RegisterAssetInfo(const std::filesystem::path &path, HashType type, std::string_view name, AssetInfoPtr &assetInfoPtr, float priority);

    /// find asset path from name
    Expected<std::string_view> FindPath(HashType type, std::string_view name);

    /// import asset
    Expected<AssetPtr> Import(const std::filesystem::path &path, HashType type, std::string_view name);

private:
    /// engine pointer
    EngineWeakPtr mEngine;

    /// registered asset info map
    std::map<HashType, StringMap<std::tuple<float, AssetInfoPtr>>> mRegisteredAssetInfo;

    /// asset factories
    std::multimap<float, AssetFactoryPtr, std::greater<float>> mAssetFactories;
};

/// register an asset factory
void AssetManagerImpl::RegisterFactory(const AssetFactoryPtr &assetFactory)
{
    // insert asset factory with the given priority
    mAssetFactories.emplace(assetFactory->Priority(), assetFactory);
}

/// register an asset info
void AssetManagerImpl::RegisterAssetInfo(const std::filesystem::path &path, HashType type, std::string_view name, AssetInfoPtr &assetInfoPtr, float priority)
{
    bool doInsert = true;
    auto typeIt = mRegisteredAssetInfo.find(type);
    if (typeIt != mRegisteredAssetInfo.end())
    {
        StringMap<std::tuple<float, AssetInfoPtr>>::iterator nameIt = typeIt->second.find(name);
        if (nameIt != typeIt->second.end())
        {
            doInsert = priority > std::get<float>(nameIt->second);
        }
    }
    if (doInsert)
    {
        mRegisteredAssetInfo[type].insert_or_assign(std::string(name), std::tuple<float, AssetInfoPtr>(priority, assetInfoPtr));
    }
}

/// find asset path from name
Expected<std::string_view> AssetManagerImpl::FindPath(HashType type, std::string_view name)
{
    auto typeIt = mRegisteredAssetInfo.find(type);
    if (typeIt != mRegisteredAssetInfo.end())
    {
        StringMap<std::tuple<float, AssetInfoPtr>>::iterator nameIt = typeIt->second.find(name);
        if (nameIt != typeIt->second.end())
        {
            auto AssetInfo = std::get<AssetInfoPtr>(nameIt->second);
            return AssetInfo->Path();
        }
    }

    // none found
    return Expected<std::string_view>::Unexpected(std::format("Asset Information path for '{}' not found", name));
}

/// import asset
Expected<AssetPtr> AssetManagerImpl::Import(const std::filesystem::path &path, HashType type, std::string_view name)
{
    // normalize the path
    std::string normalizedPath = std::filesystem::path(path).lexically_normal().generic_string();

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
            return assetInfo->Import(mEngine, path, name);
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

/// register an asset info
void AssetManager::RegisterAssetInfo(const std::filesystem::path &path, HashType type, std::string_view name, AssetInfoPtr &assetInfoPtr, float priority)
{
    Hidden::gAssetManagerImpl->RegisterAssetInfo(path, type, name, assetInfoPtr, priority);
}

/// find asset path from name
Expected<std::string_view> AssetManager::FindPath(HashType type, std::string_view name)
{
    return Hidden::gAssetManagerImpl->FindPath(type, name);
}

/// import asset
Expected<AssetPtr> AssetManager::Import(const std::filesystem::path &path, HashType type, std::string_view name)
{
    return Hidden::gAssetManagerImpl->Import(path, type, name);
}
