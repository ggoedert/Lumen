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
Shader::Shader() : Object(Type()), mImpl(Shader::Impl::MakeUniquePtr()) {}

/// destroys shader
Shader::~Shader() noexcept {}

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
