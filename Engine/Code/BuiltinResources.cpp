//==============================================================================================================================================================================
/// \file
/// \brief     BuiltinResources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lBuiltinResources.h"
#include "lMesh.h"
#include "lTexture.h"
#include "lShader.h"

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

CLASS_PTR_DEF(CheckerGrayTexture);

/// CheckerGrayTexture class
class CheckerGrayTexture : public Lumen::Texture
{
    CLASS_NO_COPY_MOVE(CheckerGrayTexture);

public:
    /// creates a smart pointer version of the checker gray texture info
    static ObjectPtr MakePtr() { return CheckerGrayTexturePtr(new CheckerGrayTexture()); }

    /// constructs a checker gray texture
    explicit CheckerGrayTexture() {}

    /// destroys checker gray texture
    ~CheckerGrayTexture() = default;
};

/// CheckerGrayTextureInfo class
class CheckerGrayTextureInfo : public Lumen::AssetInfo
{
public:
    /// creates a smart pointer version of the checker gray texture info
    static AssetInfoPtr MakePtr()
    {
        return AssetInfoPtr(new CheckerGrayTextureInfo());
    }

    /// get type
    [[nodiscard]] HashType Type() const
    {
        return Lumen::Texture::Type();
    }

    /// get name
    [[nodiscard]] std::string_view Name() const
    {
        return "Default-Checker-Gray";
    }

    /// import the checker gray texture
    [[nodiscard]] ObjectPtr Import()
    {
        return CheckerGrayTexture::MakePtr();
    }
};

CLASS_PTR_DEF(SimpleDiffuseShader);

/// SimpleDiffuseShader class
class SimpleDiffuseShader : public Lumen::Shader
{
    CLASS_NO_COPY_MOVE(SimpleDiffuseShader);

public:
    /// creates a smart pointer version of the simple diffuse shader info
    static ObjectPtr MakePtr() { return SimpleDiffuseShaderPtr(new SimpleDiffuseShader()); }

    /// constructs a simple diffuse shader
    explicit SimpleDiffuseShader() {}

    /// destroys simple diffuse shader
    ~SimpleDiffuseShader() = default;
};

/// SimpleDiffuseShaderInfo class
class SimpleDiffuseShaderInfo : public Lumen::AssetInfo
{
public:
    /// creates a smart pointer version of the simple diffuse shader info
    static AssetInfoPtr MakePtr()
    {
        return AssetInfoPtr(new SimpleDiffuseShaderInfo());
    }

    /// get type
    [[nodiscard]] HashType Type() const
    {
        return Lumen::Shader::Type();
    }

    /// get name
    [[nodiscard]] std::string_view Name() const
    {
        return "Simple/Diffuse";
    }

    /// import the checker gray texture
    [[nodiscard]] ObjectPtr Import()
    {
        return SimpleDiffuseShader::MakePtr();
    }
};

/// creates a smart pointer version of the builtin extra
AssetFactoryPtr BuiltinExtra::MakePtr()
{
    return AssetFactoryPtr(new BuiltinExtra());
}

/// accepts a path
bool BuiltinExtra::Accepts(std::filesystem::path path) const
{
    return path == "Resources/lumen_builtin_extra";
}

/// get asset infos
std::vector<Lumen::AssetInfoPtr> BuiltinExtra::GetAssetInfos()
{
    std::vector<Lumen::AssetInfoPtr> assetInfos;
    assetInfos.push_back(CheckerGrayTextureInfo::MakePtr());
    assetInfos.push_back(SimpleDiffuseShaderInfo::MakePtr());
    return assetInfos;
}
