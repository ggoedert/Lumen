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
class SphereMesh : public Mesh
{
    CLASS_NO_COPY_MOVE(SphereMesh);

public:
    /// destroys sphere mesh
    ~SphereMesh() override = default;

    /// creates a smart pointer version of the sphere mesh info
    static ObjectPtr MakePtr(EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name)
    {
        auto ptr = SphereMeshPtr(new SphereMesh(engine, path, name));

        if (auto engineLock = engine.lock())
        {
            Id::Type meshId = engineLock->CreateMesh(ptr);
            L_ASSERT_MSG(meshId != Id::Invalid, "Failed to create sphere mesh");
            ptr->SetMeshId(meshId);
        }

        return ptr;
    }

    /// get mesh data
    void GetMeshData(byte *data) override {}

private:
    /// constructs a sphere mesh
    explicit SphereMesh(EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name) : Mesh(engine, path, name) {}
};

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
        return "lumen default resources/Assets/Mesh/Sphere.fbx";
    }

    /// get name
    [[nodiscard]] std::string_view Name() const override
    {
        return "Sphere";
    }

    /// import the sphere mesh
    [[nodiscard]] Expected<ObjectPtr> Import(EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name)
    {
        return SphereMesh::MakePtr(engine, path, name);
    }

private:
    /// constructs a sphere mesh info
    explicit SphereMeshInfo() {}
};

//==============================================================================================================================================================================

/// DefaultResources::Impl class
class DefaultResources::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructor
    explicit Impl(float priority)
    {
        auto sphereMeshInfo = SphereMeshInfo::MakePtr();
        AssetManager::RegisterAssetInfo(sphereMeshInfo->Path(), sphereMeshInfo->Type(), sphereMeshInfo->Name(), sphereMeshInfo, priority);
        mAssetInfos.push_back(sphereMeshInfo);
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
DefaultResources::DefaultResources(float priority) : AssetFactory(priority), mImpl(DefaultResources::Impl::MakeUniquePtr(priority)) {}

/// creates a smart pointer version of the default resources
AssetFactoryPtr DefaultResources::MakePtr(float priority)
{
    return AssetFactoryPtr(new DefaultResources(priority));
}

/// get asset infos
std::vector<AssetInfoPtr> DefaultResources::GetAssetInfos(const std::filesystem::path &path) const
{
    return mImpl->GetAssetInfos(path);
}

CLASS_PTR_DEF(CheckerGrayTexture);

/// CheckerGrayTexture class
class CheckerGrayTexture : public Texture
{
    CLASS_NO_COPY_MOVE(CheckerGrayTexture);

public:
    /// destroys checker gray texture
    ~CheckerGrayTexture() override = default;

    /// creates a smart pointer version of the checker gray texture info
    static ObjectPtr MakePtr(EngineWeakPtr &engine)
    {
        auto ptr = CheckerGrayTexturePtr(new CheckerGrayTexture(engine));

        if (auto engineLock = engine.lock())
        {
            Id::Type texId = engineLock->CreateTexture(ptr, 64, 64);
            L_ASSERT_MSG(texId != Id::Invalid, "Failed to create checker gray texture");
            ptr->SetTextureId(texId);
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

private:
    /// constructs a checker gray texture
    explicit CheckerGrayTexture(EngineWeakPtr &engine) : Texture(engine) {}
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
        return "lumen_builtin_extra/Assets/Texture2D/Default-Checker-Gray.png";
    }

    /// get name
    [[nodiscard]] std::string_view Name() const override
    {
        return "Default-Checker-Gray";
    }

    /// import the checker gray texture
    [[nodiscard]] Expected<ObjectPtr> Import(EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name)
    {
        return CheckerGrayTexture::MakePtr(engine);
    }

private:
    /// constructs a checker gray texture info
    explicit CheckerGrayTextureInfo() {}
};

CLASS_PTR_DEF(SimpleDiffuseShader);

/// SimpleDiffuseShader class
class SimpleDiffuseShader : public Shader
{
    CLASS_NO_COPY_MOVE(SimpleDiffuseShader);

public:
    /// destroys simple diffuse shader
    ~SimpleDiffuseShader() override = default;

    /// creates a smart pointer version of the simple diffuse shader info
    static ObjectPtr MakePtr(EngineWeakPtr &engine)
    {
        auto ptr = SimpleDiffuseShaderPtr(new SimpleDiffuseShader(engine));

        if (auto engineLock = engine.lock())
        {
            Id::Type shaderId = engineLock->CreateShader(ptr/*, Engine::SimpleDiffuseShader*/);
            L_ASSERT_MSG(shaderId != Id::Invalid, "Failed to create simple diffuse shader");
            ptr->SetShaderId(shaderId);
        }

        return ptr;
    }

private:
    /// constructs a simple diffuse shader
    explicit SimpleDiffuseShader(EngineWeakPtr &engine) : Shader(engine) {}
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
        return "DefaultResourcesExtra/Mobile/Simple-Diffuse.shader";
    }

    /// get name
    [[nodiscard]] std::string_view Name() const override
    {
        return "Simple/Diffuse";
    }

    /// import the simple diffuse shader
    [[nodiscard]] Expected<ObjectPtr> Import(EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name)
    {
        return SimpleDiffuseShader::MakePtr(engine);
    }

private:
    /// constructs a simple diffuse shader info
    explicit SimpleDiffuseShaderInfo() {}
};

//==============================================================================================================================================================================

/// BuiltinExtra::Impl class
class BuiltinExtra::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructor
    explicit Impl(float priority)
    {
        auto checkerGrayTextureInfo = CheckerGrayTextureInfo::MakePtr();
        AssetManager::RegisterAssetInfo(checkerGrayTextureInfo->Path(), checkerGrayTextureInfo->Type(), checkerGrayTextureInfo->Name(), checkerGrayTextureInfo, priority);
        mAssetInfos.push_back(checkerGrayTextureInfo);
        auto simpleDiffuseShaderInfo = SimpleDiffuseShaderInfo::MakePtr();
        AssetManager::RegisterAssetInfo(simpleDiffuseShaderInfo->Path(), simpleDiffuseShaderInfo->Type(), simpleDiffuseShaderInfo->Name(), simpleDiffuseShaderInfo, priority);
        mAssetInfos.push_back(simpleDiffuseShaderInfo);
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

/// constructs builtin extra
BuiltinExtra::BuiltinExtra(float priority) : AssetFactory(priority), mImpl(BuiltinExtra::Impl::MakeUniquePtr(priority)) {}

/// creates a smart pointer version of the builtin extra
AssetFactoryPtr BuiltinExtra::MakePtr(float priority)
{
    return AssetFactoryPtr(new BuiltinExtra(priority));
}

/// get asset infos
std::vector<AssetInfoPtr> BuiltinExtra::GetAssetInfos(const std::filesystem::path &path) const
{
    return mImpl->GetAssetInfos(path);
}
