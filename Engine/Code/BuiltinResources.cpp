//==============================================================================================================================================================================
/// \file
/// \brief     BuiltinResources
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lBuiltinResources.h"
#include "lAssetManager.h"
#include "lMesh.h"
#include "lTexture.h"
#include "lShader.h"
#include "lMaterial.h"
#include "lEngine.h"

using namespace Lumen;

CLASS_PTR_DEF(BuiltinInfo);

/// BuiltinInfo class, the asset info of a builtin asset (path, type and static UUID) inlined in code, and how to make the asset
/// the editor generates the asset procedurally and stores it in the Library, a game build has no generation code and loads it from the Library
class BuiltinInfo
{
public:
    /// virtual destructor
    virtual ~BuiltinInfo() = default;

    /// get name
    [[nodiscard]] virtual std::string_view Name() const = 0;

    /// get type
    [[nodiscard]] virtual HashType Type() const = 0;

    /// get path
    [[nodiscard]] virtual std::string_view Path() const = 0;

    /// get the static UUID, it identifies the asset (and its Library entry) across sessions and builds, never change it
    [[nodiscard]] virtual Lumen::UUID GetUUID() const = 0;

    /// create the empty asset, its state comes from Generate (editor) or from its Library entry (game)
    [[nodiscard]] virtual Expected<AssetPtr> Create(EngineWeakPtr &engine) const = 0;

#ifdef EDITOR
    /// generate the procedural state of the asset
    [[nodiscard]] virtual Expected<AssetPtr> Generate(const AssetPtr &asset) const { return asset; }
#endif
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
    [[nodiscard]] std::string_view Name() const override { return "Sphere"; }

    /// get type
    [[nodiscard]] HashType Type() const override { return Mesh::Type(); }

    /// get path
    [[nodiscard]] std::string_view Path() const override { return "|Procedural|Sphere"; }

    /// get the static UUID
    [[nodiscard]] Lumen::UUID GetUUID() const override { return 0xc724e92af7e52617ull; }

    /// create the sphere mesh, the platform builds its geometry
    [[nodiscard]] Expected<AssetPtr> Create(EngineWeakPtr &engine) const override
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
    [[nodiscard]] std::string_view Name() const override { return "Checker-Gray"; }

    /// get type
    [[nodiscard]] HashType Type() const override { return Texture::Type(); }

    /// get path
    [[nodiscard]] std::string_view Path() const override { return "|Procedural|Checker-Gray"; }

    /// get the static UUID
    [[nodiscard]] Lumen::UUID GetUUID() const override { return 0xcc657c9ea8acf128ull; }

    /// create the checker gray texture
    [[nodiscard]] Expected<AssetPtr> Create(EngineWeakPtr &engine) const override
    {
        return Texture::MakePtr(engine, Path());
    }

#ifdef EDITOR
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
#endif

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
    [[nodiscard]] std::string_view Name() const override { return "Simple/Diffuse"; }

    /// get type
    [[nodiscard]] HashType Type() const override { return Shader::Type(); }

    /// get path
    [[nodiscard]] std::string_view Path() const override { return "|Procedural|Simple-Diffuse"; }

    /// get the static UUID
    [[nodiscard]] Lumen::UUID GetUUID() const override { return 0x7eb35e0be2b83a73ull; }

    /// create the simple diffuse shader, the platform builds it
    [[nodiscard]] Expected<AssetPtr> Create(EngineWeakPtr &engine) const override
    {
        return Shader::MakePtr(engine, Path(), Name());
    }

private:
    /// constructs a simple diffuse shader info
    explicit SimpleDiffuseShaderInfo() = default;
};

/// DefaultMaterialInfo class
class DefaultMaterialInfo : public BuiltinInfo
{
public:
    /// creates a smart pointer version of the default material info
    static BuiltinInfoPtr MakePtr()
    {
        auto ptr = new DefaultMaterialInfo();
        Material::Register(ptr->Name(), ptr->Path());
        return BuiltinInfoPtr(ptr);
    }

    /// get name
    [[nodiscard]] std::string_view Name() const override { return "Default-Material"; }

    /// get type
    [[nodiscard]] HashType Type() const override { return Material::Type(); }

    /// get path
    [[nodiscard]] std::string_view Path() const override { return "|Procedural|Default-Material"; }

    /// get the static UUID
    [[nodiscard]] Lumen::UUID GetUUID() const override { return 0xab48d509ae7efc93ull; }

    /// create the default material
    [[nodiscard]] Expected<AssetPtr> Create(EngineWeakPtr &engine) const override
    {
        return Material::MakePtr(Path());
    }

#ifdef EDITOR
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
#endif

private:
    /// constructs a default material info
    explicit DefaultMaterialInfo() = default;
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

        // the asset manager tracks the builtin assets like the ones in the file systems, with the asset info inlined above
        for (const auto &builtinInfo : mBuiltinInfos)
        {
            AssetManager::RegisterBuiltin(builtinInfo->Path(), builtinInfo->Type(), builtinInfo->GetUUID());
        }
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
                auto assetExpected = builtinInfo->Create(engine);
                if (!assetExpected)
                {
                    Lumen::DebugLog::Error("BuiltinResources load, {}", assetExpected.Error());
                    break;
                }
                AssetPtr asset = assetExpected.Value();

#ifdef EDITOR
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
#else
                // a game build has no procedural generation, the state of the asset is the Library entry the editor wrote
                if (!AssetManager::LoadFromLibrary(*asset))
                {
                    return Expected<AssetPtr>::Unexpected(
                        std::format("No valid Library entry for builtin resource {}, run the editor to generate it", path.generic_string()));
                }
#endif
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
