//==============================================================================================================================================================================
/// \file
/// \brief     Assets
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lAssets.h"
#include "lStringMap.h"

using namespace Lumen;

/// Assets::Impl class
class Assets::Impl
{
    CLASS_NO_COPY_MOVE(Assets::Impl);
    CLASS_PTR_UNIQUEMAKER(Assets::Impl);
    friend class Assets;

public:
    /// constructs an assets
    explicit Impl() {}

    /// destroys assets
    ~Impl() = default;

    /// register an asset factory
    void RegisterFactory(const AssetFactoryPtr &assetFactory, std::string_view extension, float priority = 0.5f);

    /// get the asset factory for the given path
    std::optional<AssetFactoryPtr> GetAssetFactory(std::filesystem::path path);

    /// list asset info for the given path
    std::vector<Lumen::AssetInfoPtr> ListAssetInfo(std::filesystem::path path);

    /// import asset
    Expected<ObjectPtr> Import(std::filesystem::path path, const HashType type, std::string_view name);

private:
    Lumen::StringMap<std::multimap<float, AssetFactoryPtr>> mAssetFactories;
};

/// register an asset factory
void Assets::Impl::RegisterFactory(const AssetFactoryPtr &assetFactory, std::string_view extension, float priority)
{
    // insert asset factory with the given extension and priority
    std::string key(extension);
    std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::tolower(c); });
    mAssetFactories[key].emplace(priority, assetFactory);
}

/// get the asset factory for the given path
std::optional<AssetFactoryPtr> Assets::Impl::GetAssetFactory(std::filesystem::path path)
{
    // construct key from the file extension
    std::string key = path.extension().string();
    if (!key.empty())
    {
        // remove the dot
        key = key.substr(1);
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::tolower(c); });
    }

    // find the asset factory for the given extension
    auto assetFactoriesIt = mAssetFactories.find(key);
    if (assetFactoriesIt != mAssetFactories.end())
    {
        // get the first factory with the highest priority
        auto &extensionFactories = assetFactoriesIt->second;
        for (auto &priorityFactory : extensionFactories)
        {
            auto &factoryPtr = priorityFactory.second;
            if (factoryPtr->Accepts(path))
            {
                // return the first factory that accepts the path
                return factoryPtr;
            }
        }
    }

    // if no asset factory is found, return nullopt
    return std::nullopt;
}

/// list asset info for the given path
std::vector<Lumen::AssetInfoPtr> Assets::Impl::ListAssetInfo(std::filesystem::path path)
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

/// import asset
Expected<ObjectPtr> Assets::Impl::Import(std::filesystem::path path, const HashType type, std::string_view name)
{
    for (Lumen::AssetInfoPtr& assetInfo : ListAssetInfo(path))
    {
        if (assetInfo->Type() == type && (!name.empty() || assetInfo->Name() == name))
        {
            return assetInfo->Import();
        }
    }

    // none found, return empty
    return Expected<ObjectPtr>::Unexpected("Asset not found");
}

//==============================================================================================================================================================================

/*/// factory options
struct FactoryOptions
{
    std::string_view mBaseDir {};
    std::string_view mFileName {};
    bool mRecursive = true;
};*/

/// constructor
Assets::Assets() : mImpl(Assets::Impl::MakeUniquePtr()) {}

/// destructor
Assets::~Assets() {}

/// register an asset factory
void Assets::RegisterFactory(const AssetFactoryPtr &assetFactory, std::string_view extension, float priority)
{
    mImpl->RegisterFactory(assetFactory, extension, priority);
}

/// import asset
Expected<ObjectPtr> Assets::Import(std::filesystem::path path, const HashType type, std::string_view name)
{
    // normalize the path and import the asset
    //std::string normalizedPath = std::filesystem::path(path).lexically_normal().generic_string();
    return mImpl->Import(path, type, name);
}
