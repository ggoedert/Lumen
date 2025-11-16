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

/// SphereMeshInfo class
class SphereMeshInfo : public AssetInfo
{
public:
    /// creates a smart pointer version of the sphere mesh info
    static AssetInfoPtr MakePtr()
    {
        return AssetInfoPtr(new SphereMeshInfo());
    }

    /// get type
    [[nodiscard]] HashType Type() const override
    {
        return Mesh::Type();
    }

    /// get path
    [[nodiscard]] std::string_view Path() const override
    {
        return "|Procedural|Sphere";
    }

    /// import the sphere mesh
    [[nodiscard]] Expected<AssetPtr> Import(EngineWeakPtr &engine)
    {
        return Mesh::MakePtr(engine, Path());
    }

private:
    /// constructs a sphere mesh info
    explicit SphereMeshInfo() {}
};

/// CheckerGrayTextureInfo class
class CheckerGrayTextureInfo : public AssetInfo
{
public:
    /// creates a smart pointer version of the checker gray texture info
    static AssetInfoPtr MakePtr()
    {
        return AssetInfoPtr(new CheckerGrayTextureInfo());
    }

    /// get type
    [[nodiscard]] HashType Type() const override
    {
        return Texture::Type();
    }

    /// get path
    [[nodiscard]] std::string_view Path() const override
    {
        return "|Procedural|Checker-Gray";
    }

    /// import the checker gray texture
    [[nodiscard]] Expected<AssetPtr> Import(EngineWeakPtr &engine)
    {
        Texture::Info textureInfo { 64, 64 };
        TexturePtr texture = static_pointer_cast<Texture>(Texture::MakePtr(engine, Path(), textureInfo));

        UniqueByteArray textureData(textureInfo.mWidth * textureInfo.mHeight * 4);
        int textureDataPitch = textureInfo.mWidth * 4;
        for (int y = 0; y < textureInfo.mHeight; y++)
        {
            for (int x = 0; x < textureInfo.mWidth; x++)
            {
                if ((x < (textureInfo.mWidth >> 1)) == (y < (textureInfo.mHeight >> 1)))
                {
                    textureData[y * textureDataPitch + 4 * x + 0] = 198;
                    textureData[y * textureDataPitch + 4 * x + 1] = 197;
                    textureData[y * textureDataPitch + 4 * x + 2] = 198;
                }
                else
                {
                    textureData[y * textureDataPitch + 4 * x + 0] = 156;
                    textureData[y * textureDataPitch + 4 * x + 1] = 158;
                    textureData[y * textureDataPitch + 4 * x + 2] = 156;
                }
                textureData[y * textureDataPitch + 4 * x + 3] = 255;
            }
        }
        texture->PushTextureData(std::move(textureData));

        return texture;
    }

private:
    /// constructs a checker gray texture info
    explicit CheckerGrayTextureInfo() {}
};

/// SimpleDiffuseShaderInfo class
class SimpleDiffuseShaderInfo : public AssetInfo
{
public:
    /// creates a smart pointer version of the simple diffuse shader info
    static AssetInfoPtr MakePtr()
    {
        return AssetInfoPtr(new SimpleDiffuseShaderInfo());
    }

    /// get type
    [[nodiscard]] HashType Type() const override
    {
        return Shader::Type();
    }

    /// get path
    [[nodiscard]] std::string_view Path() const override
    {
        return "|Procedural|Simple-Diffuse";
    }

    /// import the simple diffuse shader
    [[nodiscard]] Expected<AssetPtr> Import(EngineWeakPtr &engine)
    {
        return Shader::MakePtr(engine, Path(), "Simple/Diffuse");
    }

private:
    /// constructs a simple diffuse shader info
    explicit SimpleDiffuseShaderInfo() {}
};

//==============================================================================================================================================================================

/// BuiltinResources::Impl class
class BuiltinResources::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructor
    explicit Impl(float priority)
    {
        mAssetInfos.push_back(SphereMeshInfo::MakePtr());
        mAssetInfos.push_back(CheckerGrayTextureInfo::MakePtr());
        mAssetInfos.push_back(SimpleDiffuseShaderInfo::MakePtr());
    }

    /// destructor
    ~Impl() = default;

    /// get asset infos
    [[nodiscard]] std::vector<AssetInfoPtr> GetAssetInfos(const std::filesystem::path &path) const
    {
        std::vector<AssetInfoPtr> result;
        for (auto assetInfo : mAssetInfos)
        {
            if (assetInfo->Path() == path)
            {
                result.push_back(assetInfo);
            }
        }
        return result;
    }

private:
    /// asset infos
    std::vector<AssetInfoPtr> mAssetInfos;
};

//==============================================================================================================================================================================

/// constructs default resources
BuiltinResources::BuiltinResources(float priority) : AssetFactory(priority), mImpl(BuiltinResources::Impl::MakeUniquePtr(priority)) {}

/// creates a smart pointer version of the default resources
AssetFactoryPtr BuiltinResources::MakePtr(float priority)
{
    return AssetFactoryPtr(new BuiltinResources(priority));
}

/// get asset infos
std::vector<AssetInfoPtr> BuiltinResources::GetAssetInfos(const std::filesystem::path &path) const
{
    return mImpl->GetAssetInfos(path);
}
