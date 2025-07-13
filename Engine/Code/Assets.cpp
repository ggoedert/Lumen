//==============================================================================================================================================================================
/// \file
/// \brief     Assets
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lAssets.h"
#include "lStringMap.h"
#include "lMesh.h"

#include <filesystem>

using namespace Lumen;
namespace fs = std::filesystem;

class AssetInfo
{
public:
    /// type of asset
    AssetInfo(HashType type, std::string_view name) : mType(type), mName(name) {}

    /// get type
    [[nodiscard]] HashType Type() const { return mType; }

    /// get name
    [[nodiscard]] std::string_view Name() const { return mName; }

    ObjectPtr Import()
    {
        //TEST REMOVE THIS!!!
        return Mesh::MakePtr();
        //TEST REMOVE THIS!!!
    }

private:
    /// type
    const HashType mType;

    /// name
    const std::string mName;

};

class ContainerHandler
{
public:
    std::vector<AssetInfo> GetAssets()
    {
        //TEST REMOVE THIS!!!
        std::vector<AssetInfo> assetInfos;
        assetInfos.emplace_back(Lumen::Mesh::Type(), "Sphere");
        return assetInfos;
        //TEST REMOVE THIS!!!
    }

/*private:
    std::string prefix;
    std::string sufix;
*/
};

/// Assets::Impl class
class Assets::Impl
{
    CLASS_NO_COPY_MOVE(Assets::Impl);
    CLASS_PTR_UNIQUEMAKER(Assets::Impl);
    friend class Assets;

public:
    /// constructs a assets
    explicit Impl()
    {
        //TEST REMOVE THIS!!!
        mContainerHandlers.emplace(std::string(), ContainerHandler());
        //TEST REMOVE THIS!!!
    }

    /// destroys behavior
    ~Impl() = default;

    std::optional<ContainerHandler> GetContainerHandler(std::string_view path);

    std::vector<AssetInfo> ListAssetInfo(std::string_view path);

    /// import asset
    ObjectPtr Import(std::string_view path, const HashType type, std::string_view name);

private:
    Lumen::StringMap<ContainerHandler> mContainerHandlers;
};

std::optional<ContainerHandler> Assets::Impl::GetContainerHandler(std::string_view path)
{
    // find handler and return it
    auto it = mContainerHandlers.find(path);
    if (it != mContainerHandlers.end())
    {
        return it->second;
    }

    // if no handler found, return empty
    return {};
}

std::vector<AssetInfo> Assets::Impl::ListAssetInfo(std::string_view path)
{
    // get the container handler for the given path
    std::optional<ContainerHandler> containerHandlerOpt = GetContainerHandler(path);
    if (!containerHandlerOpt.has_value())
    {
        // if no container handler is found, return empty
        return {};
    }

    // use the container handler to get assets
    return containerHandlerOpt.value().GetAssets();
}

/// import asset
ObjectPtr Assets::Impl::Import(std::string_view path, const HashType type, std::string_view name)
{
    for (AssetInfo assetInfo : ListAssetInfo(path))
    {
        if (assetInfo.Type() == type && (!name.empty() || assetInfo.Name() == name))
        {
            return assetInfo.Import();
        }
    }

    // none found, return empty
    return {};
}

//==============================================================================================================================================================================

/// constructor
Assets::Assets() : mImpl(Assets::Impl::MakeUniquePtr()) {}

/// destructor
Assets::~Assets() {}

ObjectPtr Assets::Import(std::string_view path, const HashType type, std::string_view name)
{
    // normalize the path and import the asset
    std::string normalizedPath = fs::path(path).lexically_normal().generic_string();
    return mImpl->Import(normalizedPath, type, name);
}
