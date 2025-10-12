//==============================================================================================================================================================================
/// \file
/// \brief     Material
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lDefs.h"
#include "lMaterial.h"

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
    explicit Impl(ShaderPtr shader) :mShader(shader) {}

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
Material::Material(ShaderPtr shader) : Object(Type()), mImpl(Material::Impl::MakeUniquePtr(shader)) {}

/// custom smart pointer maker
Expected<MaterialPtr> Material::MakePtr(std::string_view shaderName)
{
    // load simple diffuse shader
    Expected<ShaderPtr> shaderExp = Shader::MakePtr(shaderName);
    if (!shaderExp.HasValue())
    {
        return Expected<MaterialPtr>::Unexpected(shaderExp.Error());
    }

    return MaterialPtr(new Material(static_pointer_cast<Shader>(shaderExp.Value())));
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
