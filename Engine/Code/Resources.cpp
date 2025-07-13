//==============================================================================================================================================================================
/// \file
/// \brief     Resources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lResources.h"
#include "lStringMap.h"
#include "lMesh.h"

#include <filesystem>

using namespace Lumen;
namespace fs = std::filesystem;

class ResourceInfo
{
public:
    /// type of resource
    ResourceInfo(HashType type, std::string_view name) : mType(type), mName(name) {}

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
    std::vector<ResourceInfo> GetResources()
    {
        //TEST REMOVE THIS!!!
        std::vector<ResourceInfo> resourceInfos;
        resourceInfos.emplace_back(Lumen::Mesh::Type(), "Sphere");
        return resourceInfos;
        //TEST REMOVE THIS!!!
    }

/*private:
    std::string prefix;
    std::string sufix;
*/
};

/// Resources::Impl class
class Resources::Impl
{
    CLASS_NO_COPY_MOVE(Resources::Impl);
    CLASS_PTR_UNIQUEMAKER(Resources::Impl);
    friend class Resources;

public:
    /// constructs a resources
    explicit Impl()
    {
        //TEST REMOVE THIS!!!
        mContainerHandlers.emplace(std::string(), ContainerHandler());
        //TEST REMOVE THIS!!!
    }

    /// destroys behavior
    ~Impl() = default;

    std::optional<ContainerHandler> GetContainerHandler(std::string_view path);

    std::vector<ResourceInfo> ListResourceInfo(std::string_view path);

    /// import resource
    ObjectPtr Import(std::string_view path, const HashType type, std::string_view name);

private:
    Lumen::StringMap<ContainerHandler> mContainerHandlers;
};

std::optional<ContainerHandler> Resources::Impl::GetContainerHandler(std::string_view path)
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

std::vector<ResourceInfo> Resources::Impl::ListResourceInfo(std::string_view path)
{
    // get the container handler for the given path
    std::optional<ContainerHandler> containerHandlerOpt = GetContainerHandler(path);
    if (!containerHandlerOpt.has_value())
    {
        // if no container handler is found, return empty
        return {};
    }

    // use the container handler to get resources
    return containerHandlerOpt.value().GetResources();
}

/// import resource
ObjectPtr Resources::Impl::Import(std::string_view path, const HashType type, std::string_view name)
{
    for (ResourceInfo resourceInfo : ListResourceInfo(path))
    {
        if (resourceInfo.Type() == type && (!name.empty() || resourceInfo.Name() == name))
        {
            return resourceInfo.Import();
        }
    }

    // none found, return empty
    return {};
}

//==============================================================================================================================================================================

/// constructor
Resources::Resources() : mImpl(Resources::Impl::MakeUniquePtr()) {}

/// destructor
Resources::~Resources() {}

ObjectPtr Resources::Import(std::string_view path, const HashType type, std::string_view name)
{
    // normalize the path and import the resource
    std::string normalizedPath = fs::path(path).lexically_normal().generic_string();
    return mImpl->Import(normalizedPath, type, name);
}
