//==============================================================================================================================================================================
/// \file
/// \brief     Shader
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lShader.h"
#include "lStringMap.h"

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
    explicit Impl(const EngineWeakPtr &engine, std::string_view name) : mEngine(engine), mName(name), mShaderId(Id::Invalid) {}

    /// destructor
    ~Impl() { Release(); }

    /// register shader name / path
    static void Register(std::string_view name, std::string_view path)
    {
        // register asset
        mAssetPaths.insert_or_assign(std::string(name), path);
    }

    /// find shader path from name
    static Expected<std::string_view> Find(std::string_view name)
    {
        // find asset
        auto it = mAssetPaths.find(name);
        if (it != mAssetPaths.end())
        {
            return it->second;
        }

        // none found
        return Expected<std::string_view>::Unexpected(std::format("Shader path for '{}' not found", name));
    }

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

    /// serialize, the shader is described by its name, the platform builds it
    void Serialize(Serialized::Type &out, bool packed) const
    {
        out = packed ? Serialized::Type::array() : Serialized::Type::object();
        Serialized::SerializeValue(out, packed, Serialized::cNameToken, Serialized::cNameTokenPacked, mName);
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        Serialized::Type name = {};
        if (!Serialized::DeserializeValue(in, packed, Serialized::cNameToken, Serialized::cNameTokenPacked, name) || !name.is_string())
        {
            throw std::runtime_error("shader data is incomplete");
        }
        mName = name.get<std::string>();
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

    /// shader name
    std::string mName;

    /// engine shader id
    Id::Type mShaderId;

    /// static map of asset names to paths
    static StringMap<std::string> mAssetPaths;
};

StringMap<std::string> Shader::Impl::mAssetPaths;

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
    AssetPtr shaderPtr = AssetPtr(new Shader(engine, path, name));
    if (!shaderPtr)
    {
        return Expected<AssetPtr>::Unexpected(std::format("Unable to load ({} - {}) shader resource", path.generic_string(), name));
    }
    ShaderPtr ptr = static_pointer_cast<Shader>(shaderPtr);
    if (auto engineLock = engine.lock())
    {
        ptr->mImpl->mShaderId = engineLock->CreateShader(ptr);
        L_ASSERT_MSG(ptr->mImpl->mShaderId != Id::Invalid, "Failed to create shader");
    }
    return shaderPtr;
}

/// register shader name / path
void Shader::Register(std::string_view name, std::string_view path)
{
    Shader::Impl::Register(name, path);
}

/// find shader path from name
Expected<std::string_view> Shader::Find(std::string_view name)
{
    return Shader::Impl::Find(name);
}

/// shaders are shared between all users of the same path
bool Shader::Shareable() const
{
    return true;
}

/// shaders are cached in the Library
bool Shader::SupportsLibrary() const
{
    return true;
}

/// serialize
void Shader::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void Shader::Deserialize(const Serialized::Type &in, bool packed)
{
    mImpl->Deserialize(in, packed);
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
