//==============================================================================================================================================================================
/// \file
/// \brief     Material
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lDefs.h"
#include "lMaterial.h"
#include "lStringMap.h"

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

    /// set property
    void SetProperty(std::string_view name, const PropertyValue &property)
    {
        //Lumen::DebugLog::Info("Material::SetProperty {} to {}", name, property);
        mProperties.insert_or_assign(std::string(name), property);
    }

    /// get property
    [[nodiscard]] Expected<PropertyValue> GetProperty(std::string_view name) const
    {
        auto it = mProperties.find(name);
        if (it != mProperties.end())
        {
            return it->second;
        }
        return Expected<PropertyValue>::Unexpected(std::format("Property '{}' not found", name));
    }

private:
    /// run component
    void Run() {}

    /// shader
    ShaderPtr mShader;

    /// map of properties
    StringMap<PropertyValue> mProperties;
};

//==============================================================================================================================================================================

DEFINE_COMPONENT_TYPEINFO(Material);

/// constructs a material with an shader
Material::Material(const GameObjectWeakPtr &gameObject, ShaderPtr shader) :
    Component(Type(), Name(), gameObject), mImpl(Material::Impl::MakeUniquePtr(shader)) {}

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

/// set property
void Material::SetProperty(std::string_view name, const PropertyValue &property) { mImpl->SetProperty(name, property); }

/// get property
[[nodiscard]] Expected<Material::PropertyValue> Material::GetProperty(std::string_view name) const { return mImpl->GetProperty(name); }

/// run component
void Material::Run() { mImpl->Run(); }
