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
    /// find shader path from name
    static Expected<std::string_view> FindPath(std::string_view name)
    {
        // @REVIEW@ TEMP code
        if (name == "Simple/Diffuse")
        {
            return "DefaultResourcesExtra/Mobile/Simple-Diffuse.shader";
        }

        // none found
        return Expected<std::string_view>::Unexpected(std::format("Shader path for '{}' not found", name));
    }

    /// constructs a shader
    explicit Impl(const EngineWeakPtr &engine, std::string_view name) : mEngine(engine), mName(name), mShaderId(Id::Invalid) {}

    /// save a shader
    bool Save() const
    {
        return true;
    }

    /// load a shader
    bool Load()
    {
        return true;
    }

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

    /// get shader name
    const std::string &Name()
    {
        return mName;
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

    /// get shader name
    const std::string mName;

    /// engine shader id
    Id::Type mShaderId;

};

//==============================================================================================================================================================================

/// constructs a shader
Shader::Shader(const EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name) : Asset(Type(), path), mImpl(Shader::Impl::MakeUniquePtr(engine, name)) {}

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
        ptr->mImpl->mShaderId = engineLock->CreateShader(ptr);
        L_ASSERT_MSG(ptr->mImpl->mShaderId != Id::Invalid, "Failed to create shader");
    }
    return shaderExp.Value();
}

/// find shader path from name
Expected<std::string_view> Shader::FindPath(std::string_view name)
{
    return Shader::Impl::FindPath(name);
}

/// save a shader
bool Shader::Save() const
{
    return mImpl->Save();
}

/// load a shader
bool Shader::Load()
{
    return mImpl->Load();
}

/// release a shader
void Shader::Release()
{
    mImpl->Release();
}

/// get shader name
const std::string &Shader::Name()
{
    return mImpl->Name();
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
