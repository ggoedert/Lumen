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
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Shader;

public:
    /// constructs a shader
    explicit Impl(const EngineWeakPtr &engine) : mEngine(engine), mShaderId(Id::Invalid) {}

    // release a shader
    void Release()
    {
        if (Id::Invalid != mShaderId)
        {
            Id::Type shaderId = mShaderId;
            mShaderId = Id::Invalid;
            if (auto engineLock = mEngine.lock())
            {
                engineLock->ReleaseShader(shaderId);
            }
        }
    }

    /// get shader id
    Id::Type GetShaderId()
    {
        return mShaderId;
    }

    /// set shader id
    void SetShaderId(Id::Type shaderId)
    {
        mShaderId = shaderId;
    }

    /// engine pointer
    EngineWeakPtr mEngine;

    /// engine shader id
    Id::Type mShaderId;

};

//==============================================================================================================================================================================

/// constructs a shader
Shader::Shader(const EngineWeakPtr &engine) : Object(Type()), mImpl(Shader::Impl::MakeUniquePtr(engine)) {}

/// destroys shader
Shader::~Shader()
{
    Release();
}

/// custom smart pointer maker
Expected<ShaderPtr> Shader::MakePtr(std::string_view shaderName)
{
    // load simple diffuse shader
    Expected<ObjectPtr> shaderExp = Assets::GlobalImport(
        Shader::Type(),
        shaderName
    );
    if (!shaderExp.HasValue())
    {
        return Expected<ShaderPtr>::Unexpected(std::format("Unable to load {} shader resource, {}", shaderName, shaderExp.Error()));
    }
    return static_pointer_cast<Shader>(shaderExp.Value());
}

/// release a shader
void Shader::Release()
{
    mImpl->Release();
}

/// get shader id
Id::Type Shader::GetShaderId()
{
    return mImpl->GetShaderId();
}

/// set shader id
void Shader::SetShaderId(Id::Type shaderId)
{
    mImpl->SetShaderId(shaderId);
}
