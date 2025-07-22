//==============================================================================================================================================================================
/// \file
/// \brief     BuiltinResources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lBuiltinResources.h"
#include "lMesh.h"

using namespace Lumen;

CLASS_PTR_DEF(SphereMesh);

/// SphereMesh class
class SphereMesh : public Lumen::Mesh
{
    CLASS_NO_COPY_MOVE(SphereMesh);

public:
    /// creates a smart pointer version of the sphere mesh info
    static ObjectPtr MakePtr() { return SphereMeshPtr(new SphereMesh()); }

    /// constructs a sphere mesh
    explicit SphereMesh() {}

    /// destroys sphere mesh
    ~SphereMesh() = default;
};

/// SphereMeshInfo class
class SphereMeshInfo : public Lumen::AssetInfo
{
public:
    /// creates a smart pointer version of the sphere mesh info
    static AssetInfoPtr MakePtr()
    {
        return AssetInfoPtr(new SphereMeshInfo());
    }

    /// get type
    [[nodiscard]] HashType Type() const
    {
        return Lumen::Mesh::Type();
    }

    /// get name
    [[nodiscard]] std::string_view Name() const
    {
        return "Sphere";
    }

    /// import the sphere mesh
    [[nodiscard]] ObjectPtr Import()
    {
        return SphereMesh::MakePtr();
    }
};

/// creates a smart pointer version of the default resources
AssetFactoryPtr DefaultResources::MakePtr()
{
    return AssetFactoryPtr(new DefaultResources());
}

/// accepts a path
bool DefaultResources::Accepts(std::filesystem::path path) const
{
    return path == "Library/lumen default resources";
}

/// get asset infos
std::vector<Lumen::AssetInfoPtr> DefaultResources::GetAssetInfos()
{
    std::vector<Lumen::AssetInfoPtr> assetInfos;
    assetInfos.push_back(SphereMeshInfo::MakePtr());
    return assetInfos;
}
