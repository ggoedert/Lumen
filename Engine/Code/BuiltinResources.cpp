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
        return "lumen default resources/Assets/Mesh/Sphere.fbx";
    }

    /// import the sphere mesh
    [[nodiscard]] Expected<AssetPtr> Import(EngineWeakPtr &engine, const std::filesystem::path &path)
    {
        return Mesh::MakePtr(engine, path);
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
        mAssetInfos.push_back(SphereMeshInfo::MakePtr());
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

    /// import the checker gray texture
    [[nodiscard]] Expected<AssetPtr> Import(EngineWeakPtr &engine, const std::filesystem::path &path)
    {
        return Texture::MakePtr(engine, path, Texture::Info { 64, 64 });
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

    /// get name
    [[nodiscard]] std::string_view Name() const
    {
        return "Simple/Diffuse";
    }

    /// get path
    [[nodiscard]] std::string_view Path() const override
    {
        return "DefaultResourcesExtra/Mobile/Simple-Diffuse.shader";
    }

    /// import the simple diffuse shader
    [[nodiscard]] Expected<AssetPtr> Import(EngineWeakPtr &engine, const std::filesystem::path &path)
    {
        return Shader::MakePtr(engine, path, Name());
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
