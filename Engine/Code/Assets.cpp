//==============================================================================================================================================================================
/// \file
/// \brief     Assets
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lStringMap.h"
#include "lAssets.h"

using namespace Lumen;

CLASS_UNIQUE_PTR_DEF(AssetsImpl);

/// AssetsImpl class
class AssetsImpl
{
    CLASS_NO_DEFAULT_CTOR(AssetsImpl);
    CLASS_NO_COPY_MOVE(AssetsImpl);
    CLASS_PTR_UNIQUEMAKER(AssetsImpl);

public:
    /// constructs an assets implementation with engine
    explicit AssetsImpl(const EngineWeakPtr &engine) : mEngine(engine) {}

    /// destroys assets
    ~AssetsImpl() = default;

    /// set engine
    void SetEngine(const EngineWeakPtr &engine);

    /// register an asset factory
    void RegisterFactory(const AssetFactoryPtr &assetFactory);

    /// register an asset info
    void RegisterAssetInfo(const HashType type, const std::string_view name, AssetInfoPtr &assetInfoPtr, float priority);

    /// import asset
    Expected<ObjectPtr> Import(const std::filesystem::path path, const HashType type, std::string_view name);

    /// import asset from name
    Expected<ObjectPtr> GlobalImport(const HashType type, const std::string_view name);

private:
    /// engine pointer
    EngineWeakPtr mEngine;

    /// registered asset info map
    std::map<HashType, StringMap<std::tuple<float, AssetInfoPtr>>> mRegisteredAssetInfo;

    /// asset factories
    std::multimap<float, AssetFactoryPtr, std::greater<float>> mAssetFactories;
};

/// register an asset factory
void AssetsImpl::RegisterFactory(const AssetFactoryPtr &assetFactory)
{
    // insert asset factory with the given extension and priority
    mAssetFactories.emplace(assetFactory->Priority(), assetFactory);
}

/// register an asset info
void AssetsImpl::RegisterAssetInfo(const HashType type, const std::string_view name, AssetInfoPtr &assetInfoPtr, float priority)
{
    bool doInsert = true;
    auto typeIt = mRegisteredAssetInfo.find(type);
    if (typeIt != mRegisteredAssetInfo.end())
    {
        StringMap<std::tuple<float, AssetInfoPtr>>::iterator nameIt = typeIt->second.find(name);
        if (nameIt != typeIt->second.end())
        {
            doInsert = priority > std::get<0>(nameIt->second);
        }
    }
    if (doInsert)
    {
        mRegisteredAssetInfo[type].insert_or_assign(std::string(name), std::tuple<float, AssetInfoPtr>(priority, assetInfoPtr));
    }
}

/// import asset
Expected<ObjectPtr> AssetsImpl::Import(const std::filesystem::path path, const HashType type, std::string_view name)
{
    // normalize the path
    std::string normalizedPath = std::filesystem::path(path).lexically_normal().generic_string();

    // combined asset infos from all factories
    std::vector<AssetInfoPtr> combinedAssetInfos;

    // get asset infos that are on the path from all factories
    for (auto &priorityFactory : mAssetFactories)
    {
        auto &assetFactory = priorityFactory.second;
        std::span<const AssetInfoPtr> assetInfos = assetFactory->GetAssetInfos(normalizedPath);
        combinedAssetInfos.insert(combinedAssetInfos.end(), assetInfos.begin(), assetInfos.end());
    }

    for (AssetInfoPtr &assetInfo : combinedAssetInfos)
    {
        if (assetInfo->Type() == type && (!name.empty() || assetInfo->Name() == name))
        {
            return assetInfo->Import(mEngine);
        }
    }

    // none found, return empty
    return Expected<ObjectPtr>::Unexpected("Asset Information not found");
}

/// import asset from name
Expected<ObjectPtr> AssetsImpl::GlobalImport(const HashType type, const std::string_view name)
{
    auto typeIt = mRegisteredAssetInfo.find(type);
    if (typeIt != mRegisteredAssetInfo.end())
    {
        StringMap<std::tuple<float, AssetInfoPtr>>::iterator nameIt = typeIt->second.find(name);
        if (nameIt != typeIt->second.end())
        {
            return std::get<1>(nameIt->second)->Import(mEngine);
        }
    }

    // none found, return empty
    return Expected<ObjectPtr>::Unexpected("Asset Information not found");
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
    static AssetsImplUniquePtr gAssetsImpl;
}

/// initialize assets namespace
void Assets::Initialize(const EngineWeakPtr &engine)
{
    L_ASSERT(!Hidden::gAssetsImpl);
    Hidden::gAssetsImpl = AssetsImpl::MakeUniquePtr(engine);
}

/// shutdown assets namespace
void Assets::Shutdown()
{
    L_ASSERT(Hidden::gAssetsImpl);
    Hidden::gAssetsImpl.reset();
}

/// register an asset factory
void Assets::RegisterFactory(const AssetFactoryPtr &assetFactory)
{
    Hidden::gAssetsImpl->RegisterFactory(assetFactory);
}

/// register an asset info
void Assets::RegisterAssetInfo(const HashType type, const std::string_view name, AssetInfoPtr &assetInfoPtr, float priority)
{
    Hidden::gAssetsImpl->RegisterAssetInfo(type, name, assetInfoPtr, priority);
}

/// import asset
Expected<ObjectPtr> Assets::Import(const std::filesystem::path path, const HashType type, std::string_view name)
{
    return Hidden::gAssetsImpl->Import(path, type, name);
}

/// import asset from name
Expected<ObjectPtr> Assets::GlobalImport(const HashType type, const std::string_view name)
{
    return Hidden::gAssetsImpl->GlobalImport(type, name);
}
