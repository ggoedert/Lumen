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

CLASS_PTR_DEF(BuiltinInfo);

/// BuiltinInfo class
class BuiltinInfo
{
public:

    /// get type
    [[nodiscard]] virtual HashType Type() const = 0;

    /// get path
    [[nodiscard]] virtual std::string_view Path() const = 0;

    /// load the asset
    [[nodiscard]] virtual Expected<AssetPtr> Load(EngineWeakPtr &engine) = 0;
};

/// SphereMeshInfo class
class SphereMeshInfo : public BuiltinInfo
{
public:
    /// creates a smart pointer version of the sphere mesh info
    static BuiltinInfoPtr MakePtr()
    {
        auto ptr = new SphereMeshInfo();
        Mesh::Register(ptr->Name(), ptr->Path());
        return BuiltinInfoPtr(ptr);
    }

    /// get name
    [[nodiscard]] std::string_view Name() const
    {
        return "Sphere";
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

    /// load the sphere mesh
    [[nodiscard]] Expected<AssetPtr> Load(EngineWeakPtr &engine) override
    {
        return Mesh::MakePtr(engine, Path());
    }

private:
    /// constructs a sphere mesh info
    explicit SphereMeshInfo() = default;
};

/// CheckerGrayTextureInfo class
class CheckerGrayTextureInfo : public BuiltinInfo
{
public:
    /// creates a smart pointer version of the checker gray texture info
    static BuiltinInfoPtr MakePtr()
    {
        auto ptr = new CheckerGrayTextureInfo();
        Texture::Register(ptr->Name(), ptr->Path());
        return BuiltinInfoPtr(ptr);
    }

    /// get name
    [[nodiscard]] std::string_view Name() const
    {
        return "Checker-Gray";
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

    /// load the checker gray texture
    [[nodiscard]] Expected<AssetPtr> Load(EngineWeakPtr &engine) override
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
    explicit CheckerGrayTextureInfo() = default;
};

/// SimpleDiffuseShaderInfo class
class SimpleDiffuseShaderInfo : public BuiltinInfo
{
public:
    /// creates a smart pointer version of the simple diffuse shader info
    static BuiltinInfoPtr MakePtr()
    {
        auto ptr = new SimpleDiffuseShaderInfo();
        Shader::Register(ptr->Name(), ptr->Path());
        return BuiltinInfoPtr(ptr);
    }

    /// get name
    [[nodiscard]] std::string_view Name() const
    {
        return "Simple/Diffuse";
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

    /// load the simple diffuse shader
    [[nodiscard]] Expected<AssetPtr> Load(EngineWeakPtr &engine) override
    {
        return Shader::MakePtr(engine, Path(), Name());
    }

private:
    /// constructs a simple diffuse shader info
    explicit SimpleDiffuseShaderInfo() = default;
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
        mBuiltinInfos.push_back(SphereMeshInfo::MakePtr());
        mBuiltinInfos.push_back(CheckerGrayTextureInfo::MakePtr());
        mBuiltinInfos.push_back(SimpleDiffuseShaderInfo::MakePtr());
    }

    /// destructor
    ~Impl() = default;

    /// check if asset exists
    [[nodiscard]] bool Exists(const std::filesystem::path &path) const
    {
        for (auto builtinInfo : mBuiltinInfos)
        {
            if (builtinInfo->Path() == path)
            {
                return true;
            }
        }
        return false;
    }

    /// load asset
    [[nodiscard]] Expected<AssetPtr> Load(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const
    {
        for (auto builtinInfo : mBuiltinInfos)
        {
            if ((builtinInfo->Type() == type) && (builtinInfo->Path() == path))
            {
                auto assetExpected = builtinInfo->Load(engine);
                if (assetExpected)
                {
                    return assetExpected.Value();
                }
                else
                {
                    Lumen::DebugLog::Error("BuiltinResources load, {}", assetExpected.Error());
                }
            }
        }
        return Expected<AssetPtr>::Unexpected(std::format("Unable to load builtin resource, {}", path.generic_string()));
    }

private:
    /// builtin infos
    std::vector<BuiltinInfoPtr> mBuiltinInfos;
};

//==============================================================================================================================================================================

/// constructs default resources
BuiltinResources::BuiltinResources(float priority) : AssetFactory(priority), mImpl(BuiltinResources::Impl::MakeUniquePtr(priority)) {}

/// creates a smart pointer version of the default resources
AssetFactoryPtr BuiltinResources::MakePtr(float priority)
{
    return AssetFactoryPtr(new BuiltinResources(priority));
}

/// check if asset exists
[[nodiscard]] bool BuiltinResources::Exists(const std::filesystem::path &path) const
{
    return mImpl->Exists(path);
}

/// load asset
Expected<AssetPtr> BuiltinResources::Load(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const
{
    return mImpl->Load(engine, type, path);
}
