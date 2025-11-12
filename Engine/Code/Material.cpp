//==============================================================================================================================================================================
/// \file
/// \brief     Material
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lDefs.h"
#include "lMaterial.h"
#include "lShader.h"

using namespace Lumen;

/// Material::Impl class
class Material::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Material;

public:
    /// constructs a material
    explicit Impl(ShaderPtr shader) : mShader(shader) {}

    /// save material
    bool Save() const { return true; }

    /// load material
    bool Load() { return true; }

    /// release material
    void Release() {}

    /// get shader
    [[nodiscard]] ShaderPtr GetShader() const { return mShader; }

    /// set shader
    void SetShader(const ShaderPtr &shader) { mShader = shader; }

private:
    /// shader
    ShaderPtr mShader;
};

//==============================================================================================================================================================================

/// constructs a material with an shader
Material::Material(ShaderPtr shader, const std::filesystem::path &path) : Asset(Type(), path), mImpl(Material::Impl::MakeUniquePtr(shader)) {}

/// custom smart pointer maker
Expected<AssetPtr> Material::MakePtr(const std::filesystem::path &path)
{
    //@REVIEW@ FIXME we should really open the material and get the shaderName from it
    std::string shaderName = "Simple/Diffuse";

    // get shader information
    Expected<std::string_view> shaderPathExp = Shader::FindPath(shaderName);
    if (!shaderPathExp.HasValue())
    {
        return Expected<AssetPtr>::Unexpected(std::format("Unable to load {} shader resource, {}", shaderName, shaderPathExp.Error()));
    }

    // load shader
    Expected<AssetPtr> shaderExp = AssetManager::Import(Shader::Type(), shaderPathExp.Value());
    if (!shaderExp.HasValue())
    {
        return Expected<AssetPtr>::Unexpected(shaderExp.Error());
    }

    return AssetPtr(new Material(static_pointer_cast<Shader>(shaderExp.Value()), path));
}

/// save material
bool Material::Save() const
{
    return mImpl->Save();
}

/// load material
bool Material::Load()
{
    return mImpl->Load();
}

/// release material
void Material::Release()
{
    mImpl->Release();
}

/// get shader
ShaderPtr Material::GetShader() const
{
    return mImpl->GetShader();
}

/// set shader
void Material::SetShader(const ShaderPtr &shader)
{
    mImpl->SetShader(shader);
}
