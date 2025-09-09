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

/// set property
void Shader::SetProperty(const std::string name, const Lumen::ObjectPtr &property) {}
