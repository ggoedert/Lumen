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

    /// register an asset info (for built-in assets)
    void RegisterAssetInfo(HashType hashType, std::string_view name, AssetInfoPtr &assetInfoPtr);

    /// register an asset factory
    void RegisterFactory(const AssetFactoryPtr &assetFactory, float priority = 0.5f);

    /// list asset info for the given path
    std::vector<AssetInfoPtr> ListAssetInfo(const std::filesystem::path &path);

    /// import asset
    Expected<ObjectPtr> Import(std::optional<std::filesystem::path> path, const HashType type, std::string_view name);

private:
    /// engine pointer
    EngineWeakPtr mEngine;

    /// registered asset info map
    std::map<HashType, StringMap<AssetInfoPtr>> mRegisteredAssetInfo;

    /// asset factories
    std::multimap<float, AssetFactoryPtr> mAssetFactories;
};

/// register an asset info (for built-in assets)
void AssetsImpl::RegisterAssetInfo(HashType hashType, std::string_view name, AssetInfoPtr &assetInfoPtr)
{
    mRegisteredAssetInfo[hashType].insert_or_assign(std::string(name), assetInfoPtr);
}

/// register an asset factory
void AssetsImpl::RegisterFactory(const AssetFactoryPtr &assetFactory, float priority)
{
    // insert asset factory with the given extension and priority
    mAssetFactories.emplace(priority, assetFactory);
}

/// list asset info for the given path
std::vector<AssetInfoPtr> AssetsImpl::ListAssetInfo(const std::filesystem::path &path)
{
    // combined asset infos from all factories
    std::vector<AssetInfoPtr> combinedAssetInfos;

    // get asset infos that are on the path from all factories
    for (auto &priorityFactory : mAssetFactories)
    {
        auto &assetFactory = priorityFactory.second;
        std::span<const AssetInfoPtr> assetInfos = assetFactory->GetAssetInfos(path);
        combinedAssetInfos.insert(combinedAssetInfos.end(), assetInfos.begin(), assetInfos.end());
    }

    // return combined asset infos
    return combinedAssetInfos;
}

/// import asset
Expected<ObjectPtr> AssetsImpl::Import(std::optional<std::filesystem::path> path, const HashType type, std::string_view name)
{
    if (path.has_value())
    {
        // normalize the path
        std::string normalizedPath = std::filesystem::path(path.value()).lexically_normal().generic_string();

        for (AssetInfoPtr &assetInfo : ListAssetInfo(normalizedPath))
        {
            if (assetInfo->Type() == type && (!name.empty() || assetInfo->Name() == name))
            {
                return assetInfo->Import(mEngine);
            }
        }
    }
    else
    {
        auto typeIt = mRegisteredAssetInfo.find(type);
        if (typeIt != mRegisteredAssetInfo.end())
        {
            StringMap<AssetInfoPtr>::iterator nameIt = typeIt->second.find(name);
            if (nameIt != typeIt->second.end())
            {
                return nameIt->second->Import(mEngine);
            }
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

/// register an asset info (for built-in assets)
void Assets::RegisterAssetInfo(HashType hashType, std::string_view name, AssetInfoPtr &assetInfoPtr)
{
    Hidden::gAssetsImpl->RegisterAssetInfo(hashType, name, assetInfoPtr);
}

/// register an asset factory
void Assets::RegisterFactory(const AssetFactoryPtr &assetFactory, float priority)
{
    Hidden::gAssetsImpl->RegisterFactory(assetFactory, priority);
}

/// import asset
Expected<ObjectPtr> Assets::Import(std::optional<std::filesystem::path> path, const HashType type, std::string_view name)
{
    return Hidden::gAssetsImpl->Import(path, type, name);
}
