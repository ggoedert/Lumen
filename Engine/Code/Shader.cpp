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
Shader::Shader(const EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name) : Asset(Type(), path, name), mImpl(Shader::Impl::MakeUniquePtr(engine)) {}

/// destroys shader
Shader::~Shader()
{
    Release();
}

/// creates a smart pointer version of the shader asset
Expected<AssetPtr> Shader::MakePtr(EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name)
{
    Expected<AssetPtr> shaderExp = AssetPtr(new Shader(engine, path, name));
    if (!shaderExp.HasValue())
    {
        return Expected<AssetPtr>::Unexpected(std::format("Unable to load ({} - {}) shader resource, {}", path.string(), name, shaderExp.Error()));
    }
    ShaderPtr ptr = static_pointer_cast<Shader>(shaderExp.Value());
    if (auto engineLock = engine.lock())
    {
        ptr->mImpl->mShaderId = engineLock->CreateShader(ptr/*, Engine::SimpleDiffuseShader*/); //@REVIEW@ FIXME: hardcoded shader type
        L_ASSERT_MSG(ptr->mImpl->mShaderId != Id::Invalid, "Failed to create shader");
    }
    return shaderExp.Value();
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
