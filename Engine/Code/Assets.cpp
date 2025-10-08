//==============================================================================================================================================================================
/// \file
/// \brief     Assets
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lAssets.h"
#include "lEngine.h"

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
    void RegisterFactory(const AssetFactoryPtr &assetFactory, float priority = 0.5f);

    /// get the asset factory for the given path
    std::optional<AssetFactoryPtr> GetAssetFactory(std::filesystem::path path);

    /// list asset info for the given path
    std::vector<Lumen::AssetInfoPtr> ListAssetInfo(std::filesystem::path path);

    /// find asset info for the given type and name
    Expected<Lumen::AssetInfoPtr> FindAssetInfo(const HashType type, std::string_view name);

    /// import asset
    Expected<ObjectPtr> Import(std::optional<std::filesystem::path> path, const HashType type, std::string_view name);

private:
    /// engine pointer
    EngineWeakPtr mEngine;

    /// asset factories
    std::multimap<float, AssetFactoryPtr> mAssetFactories;
};

/// register an asset factory
void AssetsImpl::RegisterFactory(const AssetFactoryPtr &assetFactory, float priority)
{
    // insert asset factory with the given extension and priority
    mAssetFactories.emplace(priority, assetFactory);
}

/// get the asset factory for the given path
std::optional<AssetFactoryPtr> AssetsImpl::GetAssetFactory(std::filesystem::path path)
{
    // construct key from the file extension
    std::string key = path.extension().string();
    if (!key.empty())
    {
        // remove the dot
        key = key.substr(1);
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::tolower(c); });
    }

    // get the first factory with the highest priority
    for (auto &priorityFactory : mAssetFactories)
    {
        auto &factoryPtr = priorityFactory.second;
        if (factoryPtr->Accepts(path))
        {
            // return the first factory that accepts the path
            return factoryPtr;
        }
    }

    // if no asset factory is found, return nullopt
    return std::nullopt;
}

/// list asset info for the given path
std::vector<Lumen::AssetInfoPtr> AssetsImpl::ListAssetInfo(std::filesystem::path path)
{
    // get the asset factory for the given path
    std::optional<AssetFactoryPtr> assetFactory = GetAssetFactory(path);
    if (!assetFactory.has_value())
    {
        // if no asset factory is found, return empty
        return {};
    }

    // use the asset factory to get assets
    return assetFactory.value()->GetAssetInfos();
}

/// find asset info for the given type and name
Expected<Lumen::AssetInfoPtr> AssetsImpl::FindAssetInfo(const HashType type, std::string_view name)
{
    // iterate over all asset factories
    for (const auto &priorityFactoryPair : mAssetFactories)
    {
        // iterate over asset infos in the factory
        const auto &factoryPtr = priorityFactoryPair.second;
        for (const auto &assetInfoPtr : factoryPtr->GetAssetInfos())
        {
            if (assetInfoPtr->Type() == type && assetInfoPtr->Name() == name)
            {
                // return the first asset infos that matches the type and name
                return assetInfoPtr;
            }
        }
    }

    // none found, return empty
    return Expected<Lumen::AssetInfoPtr>::Unexpected("Asset Information not found");
}

/// import asset
Expected<ObjectPtr> AssetsImpl::Import(std::optional<std::filesystem::path> path, const HashType type, std::string_view name)
{
    if (path.has_value())
    {
        // normalize the path
        //std::string normalizedPath = std::filesystem::path(path).lexically_normal().generic_string();
        for (Lumen::AssetInfoPtr &assetInfo : ListAssetInfo(path.value()))
        {
            if (assetInfo->Type() == type && (!name.empty() || assetInfo->Name() == name))
            {
                return assetInfo->Import(mEngine);
            }
        }

        // none found, return empty
        return Expected<ObjectPtr>::Unexpected("Asset Information not found");
    }
    else
    {
        // search all asset infos for the given type and name
        Expected<Lumen::AssetInfoPtr> assetInfoExp = FindAssetInfo(type, name);
        if (assetInfoExp.HasValue())
        {
            return assetInfoExp.Value()->Import(mEngine);
        }

        // not found, return empty
        return Expected<ObjectPtr>::Unexpected(assetInfoExp.Error());
    }
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
void Assets::RegisterFactory(const AssetFactoryPtr &assetFactory, float priority)
{
    Hidden::gAssetsImpl->RegisterFactory(assetFactory, priority);
}

/// import asset
Expected<ObjectPtr> Assets::Import(std::optional<std::filesystem::path> path, const HashType type, std::string_view name)
{
    return Hidden::gAssetsImpl->Import(path, type, name);
}
