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
    [[nodiscard]] const ShaderPtr GetShader() const { return mShader; }

    /// set shader
    void SetShader(const ShaderPtr &shader) { mShader = shader; }

private:
    /// run component
    void Run() {}

    /// shader
    ShaderPtr mShader;
};

//==============================================================================================================================================================================

DEFINE_COMPONENT_TYPEINFO(Material);

/// constructs a material with an shader
Material::Material(const GameObjectWeakPtr &gameObject, ShaderPtr shader) :
    Component(Type(), Name(), gameObject), mImpl(Material::Impl::MakeUniquePtr(shader))
{}

/// creates a smart pointer version of the material component
ComponentPtr Material::MakePtr(const GameObjectWeakPtr &gameObject, const Object &params)
{
    if (params.Type() != Material::Params::Type())
    {
#ifdef TYPEINFO
        DebugLog::Error("Create component, unknown parameter type: {}", params.Type().mName);
#else
        DebugLog::Error("Create component, unknown parameter hash type: 0x{:08X}", params.Type());
#endif
        return {};
    }

    const auto &createParams = static_cast<const Params &>(params);
    Expected<ShaderPtr> shaderExp = Shader::MakePtr(createParams.mShaderName);
    if (!shaderExp.HasValue())
    {
        Lumen::DebugLog::Error("Unable to create shader {}: {}", createParams.mShaderName, shaderExp.Error());
        return {};
    }

    return ComponentPtr(new Material(gameObject, shaderExp.Value()));
}

/// run component
void Material::Run() { mImpl->Run(); }
