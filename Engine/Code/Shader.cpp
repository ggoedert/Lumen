//==============================================================================================================================================================================
/// \file
/// \brief     Shader
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lShader.h"

using namespace Lumen;

/// Shader::Impl class
class Shader::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Shader;

public:
    /// constructs a shader
    explicit Impl() {}
};

//==============================================================================================================================================================================

/// constructs a shader
Shader::Shader(const EngineWeakPtr &engine) : Object(Type()), mEngine(engine), mShaderId(Engine::InvalidId), mImpl(Shader::Impl::MakeUniquePtr()) {}

/// destroys shader
Shader::~Shader() noexcept
{
    Release();
}

/// custom smart pointer maker
Expected<ShaderPtr> Shader::MakePtr(std::string_view shaderName)
{
    // load simple diffuse shader
    Lumen::Expected<Lumen::ObjectPtr> shaderExp = Lumen::Assets::Import(
        std::nullopt,
        Lumen::Shader::Type(),
        "Simple/Diffuse"
    );
    if (!shaderExp.HasValue())
    {
        return Expected<ShaderPtr>::Unexpected(std::format("Unable to load {} shader resource, {}", shaderName, shaderExp.Error()));
    }
    return static_pointer_cast<Lumen::Shader>(shaderExp.Value());
}

/// release a shader
void Shader::Release()
{
#ifdef WIP
    if (Engine::InvalidId != mShaderId)
    {
        Engine::IdType shaderId = mShaderId;
        mShaderId = Engine::InvalidId;
        if (auto engineLock = mEngine.lock())
        {
            engineLock->ReleaseShader(shaderId);
        }
    }
#endif
}

/// set int property
void Shader::SetProperty(std::string_view name, const int property)
{
    Lumen::DebugLog::Info("Shader::SetProperty {} to {}", name, property);
}

/// set float property
void Shader::SetProperty(std::string_view name, const float property)
{
    Lumen::DebugLog::Info("Shader::SetProperty {} to {}", name, property);
}

/// set TexturePtr property
void Shader::SetProperty(std::string_view name, const Lumen::TexturePtr &property)
{
    Lumen::DebugLog::Info("Shader::SetProperty {} from an TexturePtr", name);
}
