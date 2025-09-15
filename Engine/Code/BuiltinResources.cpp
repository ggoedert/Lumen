//==============================================================================================================================================================================
/// \file
/// \brief     BuiltinResources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lBuiltinResources.h"
#include "lMesh.h"
#include "lTexture.h"
#include "lShader.h"
#include "lEngine.h"

using namespace Lumen;

CLASS_PTR_DEF(SphereMesh);

/// SphereMesh class
class SphereMesh : public Lumen::Mesh
{
    CLASS_NO_COPY_MOVE(SphereMesh);

public:
    /// creates a smart pointer version of the sphere mesh info
    static ObjectPtr MakePtr(EngineWeakPtr &engine) { return SphereMeshPtr(new SphereMesh(engine)); }

    /// constructs a sphere mesh
    explicit SphereMesh(EngineWeakPtr &engine) {}

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
    [[nodiscard]] ObjectPtr Import(EngineWeakPtr &engine)
    {
        return SphereMesh::MakePtr(engine);
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
const std::vector<Lumen::AssetInfoPtr> &DefaultResources::GetAssetInfos() const
{
    static const std::vector<Lumen::AssetInfoPtr> assetInfos = []
    {
        std::vector<Lumen::AssetInfoPtr> infos;
        infos.push_back(SphereMeshInfo::MakePtr());
        return infos;
    }();

    return assetInfos;
}

CLASS_PTR_DEF(CheckerGrayTexture);

/// CheckerGrayTexture class
class CheckerGrayTexture : public Lumen::Texture
{
    CLASS_NO_COPY_MOVE(CheckerGrayTexture);

public:
    /// creates a smart pointer version of the checker gray texture info
    static ObjectPtr MakePtr(EngineWeakPtr &engine) {
        auto ptr = CheckerGrayTexturePtr(new CheckerGrayTexture(engine));

        if (auto engineLock = engine.lock())
        {
            L_ASSERT_MSG(
                (ptr->mTexId = engineLock->RegisterTexture(ptr, 64, 64)) != Engine::InvalidTextureID,
                "Failed to register checker gray texture");
        }

        return ptr;
    }

    /// get texture data
    void GetTextureData(byte *data, int pitch) override
    {
        for (int y = 0; y < 64; y++)
        {
            for (int x = 0; x < 64; x++)
            {
                if ((x < (64 >> 1)) == (y < (64 >> 1)))
                {
                    data[y * pitch + 4 * x + 0] = 198;
                    data[y * pitch + 4 * x + 1] = 197;
                    data[y * pitch + 4 * x + 2] = 198;
                }
                else
                {
                    data[y * pitch + 4 * x + 0] = 156;
                    data[y * pitch + 4 * x + 1] = 158;
                    data[y * pitch + 4 * x + 2] = 156;
                }
                data[y * pitch + 4 * x + 3] = 255;
            }
        }
    }

    /// destroys checker gray texture
    ~CheckerGrayTexture() noexcept override = default;

private:
    /// constructs a checker gray texture
    explicit CheckerGrayTexture(EngineWeakPtr &engine) : Texture(engine)
    {
    }
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
    [[nodiscard]] ObjectPtr Import(EngineWeakPtr &engine)
    {
        return CheckerGrayTexture::MakePtr(engine);
    }
};

CLASS_PTR_DEF(SimpleDiffuseShader);

/// SimpleDiffuseShader class
class SimpleDiffuseShader : public Lumen::Shader
{
    CLASS_NO_COPY_MOVE(SimpleDiffuseShader);

public:
    /// creates a smart pointer version of the simple diffuse shader info
    static ObjectPtr MakePtr(EngineWeakPtr &engine) { return SimpleDiffuseShaderPtr(new SimpleDiffuseShader(engine)); }

    /// constructs a simple diffuse shader
    explicit SimpleDiffuseShader(EngineWeakPtr &engine) {}

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
    [[nodiscard]] ObjectPtr Import(EngineWeakPtr &engine)
    {
        return SimpleDiffuseShader::MakePtr(engine);
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
const std::vector<Lumen::AssetInfoPtr> &BuiltinExtra::GetAssetInfos() const
{
    static const std::vector<Lumen::AssetInfoPtr> assetInfos = []
    {
        std::vector<Lumen::AssetInfoPtr> infos;
        infos.push_back(CheckerGrayTextureInfo::MakePtr());
        infos.push_back(SimpleDiffuseShaderInfo::MakePtr());
        return infos;
    }();

    return assetInfos;
}
