//==============================================================================================================================================================================
/// \file
/// \brief     BuiltinResources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#ifdef EDITOR

#include "lBuiltinResources.h"
#include "lAssetManager.h"
#include "lMesh.h"
#include "lTexture.h"
#include "lShader.h"
#include "lMaterial.h"
#include "lEngine.h"

using namespace Lumen;

CLASS_PTR_DEF(BuiltinInfo);

/// BuiltinInfo class, the asset info of a builtin asset inlined in code (its UUID is in the reserved builtin range), and how to make the asset
class BuiltinInfo : public AssetInfo
{
    CLASS_NO_DEFAULT_CTOR(BuiltinInfo);
    CLASS_NO_COPY_MOVE(BuiltinInfo);

public:
    /// get name
    [[nodiscard]] std::string_view Name() const { return mName; }

    /// get path
    [[nodiscard]] const std::filesystem::path &Path() const override { return mPath; }

    /// get the static UUID, it identifies the asset (and its Library entry) across sessions and builds, never change it
    [[nodiscard]] Lumen::UUID UUID() const override { return mUUID; }

    /// get the asset type
    [[nodiscard]] HashType Type() const override { return mType; }

    /// create the empty asset
    [[nodiscard]] virtual Expected<AssetPtr> Create(EngineWeakPtr &engine) const = 0;

    /// generate the procedural state of the asset
    [[nodiscard]] virtual Expected<AssetPtr> Generate(const AssetPtr &asset) const { return asset; }

protected:
    /// constructs a builtin info, called by derived classes
    explicit BuiltinInfo(std::string_view name, std::string_view path, HashType type, Lumen::UUID uuid) :
        mName(name), mPath(path), mType(type), mUUID(uuid) {}

private:
    /// name
    const std::string mName;

    /// path
    const std::filesystem::path mPath;

    /// asset type
    const HashType mType;

    /// static UUID
    const Lumen::UUID mUUID;
};

/// SphereMeshInfo class
class SphereMeshInfo : public BuiltinInfo
{
public:
    /// creates a smart pointer version of the sphere mesh info
    static BuiltinInfoPtr MakePtr()
    {
        auto ptr = new SphereMeshInfo();
        Mesh::Register(ptr->Name(), ptr->Path().generic_string());
        return BuiltinInfoPtr(ptr);
    }

    /// create the sphere mesh, the platform builds its geometry
    [[nodiscard]] Expected<AssetPtr> Create(EngineWeakPtr &engine) const override
    {
        return Mesh::MakePtr(engine, Path());
    }

private:
    /// constructs a sphere mesh info
    explicit SphereMeshInfo() : BuiltinInfo("Sphere", "|Procedural|Sphere", Mesh::Type(), 0x0000000000000001ull) {}
};

/// CheckerGrayTextureInfo class
class CheckerGrayTextureInfo : public BuiltinInfo
{
public:
    /// creates a smart pointer version of the checker gray texture info
    static BuiltinInfoPtr MakePtr()
    {
        auto ptr = new CheckerGrayTextureInfo();
        Texture::Register(ptr->Name(), ptr->Path().generic_string());
        return BuiltinInfoPtr(ptr);
    }

    /// create the checker gray texture
    [[nodiscard]] Expected<AssetPtr> Create(EngineWeakPtr &engine) const override
    {
        return Texture::MakePtr(engine, Path());
    }

    /// generate the checker gray pixels
    [[nodiscard]] Expected<AssetPtr> Generate(const AssetPtr &asset) const override
    {
        TexturePtr texture = static_pointer_cast<Texture>(asset);

        Texture::Info textureInfo { 64, 64 };
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
        texture->PushTextureData(textureInfo, std::move(textureData));

        return texture;
    }

private:
    /// constructs a checker gray texture info
    explicit CheckerGrayTextureInfo() : BuiltinInfo("Checker-Gray", "|Procedural|Checker-Gray", Texture::Type(), 0x0000000000000002ull) {}
};

/// SimpleDiffuseShaderInfo class
class SimpleDiffuseShaderInfo : public BuiltinInfo
{
public:
    /// creates a smart pointer version of the simple diffuse shader info
    static BuiltinInfoPtr MakePtr()
    {
        auto ptr = new SimpleDiffuseShaderInfo();
        Shader::Register(ptr->Name(), ptr->Path().generic_string());
        return BuiltinInfoPtr(ptr);
    }

    /// create the simple diffuse shader, the platform builds it
    [[nodiscard]] Expected<AssetPtr> Create(EngineWeakPtr &engine) const override
    {
        return Shader::MakePtr(engine, Path(), Name());
    }

private:
    /// constructs a simple diffuse shader info
    explicit SimpleDiffuseShaderInfo() : BuiltinInfo("Simple/Diffuse", "|Procedural|Simple-Diffuse", Shader::Type(), 0x0000000000000003ull) {}
};

/// DefaultMaterialInfo class
class DefaultMaterialInfo : public BuiltinInfo
{
public:
    /// creates a smart pointer version of the default material info
    static BuiltinInfoPtr MakePtr()
    {
        auto ptr = new DefaultMaterialInfo();
        Material::Register(ptr->Name(), ptr->Path().generic_string());
        return BuiltinInfoPtr(ptr);
    }

    /// create the default material
    [[nodiscard]] Expected<AssetPtr> Create(EngineWeakPtr &engine) const override
    {
        return Material::MakePtr(Path());
    }

    /// generate the default material, the simple diffuse shader with the checker gray texture
    [[nodiscard]] Expected<AssetPtr> Generate(const AssetPtr &asset) const override
    {
        MaterialPtr material = static_pointer_cast<Material>(asset);

        // set shader
        Expected<AssetPtr> shaderExp = AssetManager::Load(Shader::Type(), "|Procedural|Simple-Diffuse");
        if (!shaderExp.HasValue())
        {
            return shaderExp;
        }
        material->SetShader(static_pointer_cast<Shader>(shaderExp.Value()));

        // set texture
        Expected<AssetPtr> textureExp = AssetManager::Load(Texture::Type(), "|Procedural|Checker-Gray");
        if (!textureExp.HasValue())
        {
            return textureExp;
        }
        material->SetProperty("diffuseTex", static_pointer_cast<Texture>(textureExp.Value()));

        return material;
    }

private:
    /// constructs a default material info
    explicit DefaultMaterialInfo() : BuiltinInfo("Default-Material", "|Procedural|Default-Material", Material::Type(), 0x0000000000000004ull) {}
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
        mBuiltinInfos.push_back(DefaultMaterialInfo::MakePtr());

        // the asset manager tracks the builtin assets like the ones in the file systems, through their BuiltinInfo
        for (const auto &builtinInfo : mBuiltinInfos)
        {
            AssetManager::RegisterBuiltin(builtinInfo);
        }
    }

    /// destructor
    ~Impl() = default;

    /// check if asset exists
    [[nodiscard]] bool Exists(const std::filesystem::path &path) const
    {
        for (const auto &builtinInfo : mBuiltinInfos)
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
        for (const auto &builtinInfo : mBuiltinInfos)
        {
            if ((builtinInfo->Type() == type) && (builtinInfo->Path() == path))
            {
                auto assetExpected = builtinInfo->Create(engine);
                if (!assetExpected)
                {
                    Lumen::DebugLog::Error("BuiltinResources load, {}", assetExpected.Error());
                    break;
                }
                AssetPtr asset = assetExpected.Value();

                // the procedural generation is the truth, refresh the Library entry so game builds can load the asset without it,
                // this must happen before the renderer consumes the texture pixels
                auto generatedExpected = builtinInfo->Generate(asset);
                if (!generatedExpected)
                {
                    Lumen::DebugLog::Error("BuiltinResources load, {}", generatedExpected.Error());
                    break;
                }
                if (!AssetManager::UpdateLibrary(*asset))
                {
                    Lumen::DebugLog::Warning("Unable to update the Library entry of {}", path.generic_string());
                }
                return asset;
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
#endif
