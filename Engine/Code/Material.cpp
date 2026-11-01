//==============================================================================================================================================================================
/// \file
/// \brief     Material
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lDefs.h"
#include "lStringMap.h"
#include "lMaterial.h"
#include "lTexture.h"
#include "lRenderer.h"
#include "lShader.h"
#include "lAssetManager.h"

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
    explicit Impl(Material &owner) : mOwner(owner) {}

    /// destructor
    ~Impl() { Release(); }

    /// register material name / path
    static void Register(std::string_view name, std::string_view path)
    {
        // register asset
        mAssetPaths.insert_or_assign(std::string(name), path);
    }

    /// find material path from name
    static Expected<std::string_view> Find(std::string_view name)
    {
        // find asset
        auto it = mAssetPaths.find(name);
        if (it != mAssetPaths.end())
        {
            return it->second;
        }

        // none found
        return Expected<std::string_view>::Unexpected(std::format("Material path for '{}' not found", name));
    }

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        Serialized::SerializeValue(out, packed, Serialized::cShaderTypeToken, Serialized::cShaderTypeTokenPacked, mShader->Name());

        Serialized::Type propertiesObj = Serialized::Type::object();
        for (const auto &[key, value] : mProperties)
        {
            if (std::holds_alternative<int>(value))
            {
                propertiesObj[key] = std::get<int>(value);
            }
            else if (std::holds_alternative<float>(value))
            {
                propertiesObj[key] = std::get<float>(value);
            }
            else if (std::holds_alternative<Lumen::TexturePtr>(value))
            {
                Serialized::Type textureValue = {};
                auto tex = std::get<Lumen::TexturePtr>(value);
                if (tex)
                {
                    Serialized::SerializeValue(textureValue, packed, Serialized::cTextureTypeToken, Serialized::cTextureTypeTokenPacked, tex->Path().generic_string());
                }
                propertiesObj[key] = textureValue;
            }
        }
        Serialized::SerializeValue(out, packed, Serialized::cPropertiesToken, Serialized::cPropertiesTokenPacked, propertiesObj);
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        mShader.reset();
        mProperties.clear();

        // load shader
        Serialized::Type shaderName = {};
        Serialized::DeserializeValue(in, packed, Serialized::cShaderTypeToken, Serialized::cShaderTypeTokenPacked, shaderName);
        if (shaderName.empty())
        {
            throw std::runtime_error(std::format("Unable to load material resource, no shader name in material asset"));
        }
        Expected<std::string_view> shaderPathExp = Shader::Find(shaderName);
        if (!shaderPathExp.HasValue())
        {
            throw std::runtime_error(std::format("Unable to load {} shader resource, {}", shaderName.get<std::string_view>(), shaderPathExp.Error()));
        }
        Expected<AssetPtr> shaderExp = AssetManager::Load(Shader::Type(), shaderPathExp.Value());
        if (!shaderExp.HasValue())
        {
            throw std::runtime_error(shaderExp.Error());
        }
        mShader = static_pointer_cast<Shader>(shaderExp.Value());

        // load properties
        Serialized::Type propertiesObj = Serialized::Type::object();
        if (Serialized::DeserializeValue(in, packed, Serialized::cPropertiesToken, Serialized::cPropertiesTokenPacked, propertiesObj))
        {
            Serialized::Type path = {};
            for (auto &inProperty : propertiesObj.items())
            {
                if (inProperty.key() == "diffuseTex")
                {
                    Serialized::DeserializeValue(inProperty.value(), packed, Serialized::cTextureTypeToken, Serialized::cTextureTypeTokenPacked, path);

                    // load texture
                    Expected<AssetPtr> textureExp = AssetManager::Load(Texture::Type(), path);
                    if (!textureExp.HasValue())
                    {
                        throw std::runtime_error(std::format("Unable to load texture resource {}, {}", path.get<std::string>(), textureExp.Error()));
                    }
                    const TexturePtr texture = static_pointer_cast<Texture>(textureExp.Value());

                    // set property
                    SetProperty("diffuseTex", texture);
                }
            }
        }
    }

    /// set property
    void SetProperty(std::string_view name, const PropertyValue &property)
    {
        //DebugLog::Info("Renderer::SetProperty {} to {}", name, property);
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

    /// save material
    bool Save() const
    {
        const std::filesystem::path &path = mOwner.Path();
        Lumen::DebugLog::Info("Material::Impl::Save {}", path.generic_string());
        if (!mShader)
        {
            Lumen::DebugLog::Error("Unable to save material {}, it has no shader", path.generic_string());
            return false;
        }

        // write the source, then refresh the Library (best effort)
        Serialized::Type data;
        Serialize(data, FileSystem::IsPacked(path));
        if (!FileSystem::WriteSerializedData(path, data))
        {
            return false;
        }
        if (!AssetManager::UpdateLibrary(mOwner))
        {
            Lumen::DebugLog::Warning("Unable to update the Library entry of {}", path.generic_string());
        }
        return true;
    }

    /// load material
    bool Load()
    {
        Lumen::DebugLog::Info("Material::Impl::Load {}", mOwner.Path().generic_string());

        // Library first, then the source (which refreshes the Library)
        return AssetManager::LoadSerialized(mOwner);
    }

    /// release material
    void Release() {}

    /// get shader
    [[nodiscard]] ShaderPtr GetShader() const { return mShader; }

    /// set shader
    void SetShader(const ShaderPtr &shader) { mShader = shader; }

private:
    /// owner
    Material &mOwner;

    /// shader
    ShaderPtr mShader;

    /// map of properties
    StringMap<PropertyValue> mProperties;

    /// static map of asset names to paths
    static StringMap<std::string> mAssetPaths;
};

StringMap<std::string> Material::Impl::mAssetPaths;

//==============================================================================================================================================================================

/// constructs a material
Material::Material(const std::filesystem::path &path) : Asset(Type(), path), mImpl(Material::Impl::MakeUniquePtr(*this)) {}

/// custom smart pointer maker
Expected<AssetPtr> Material::MakePtr(const std::filesystem::path &path)
{
    return MaterialPtr(new Material(path));
}

/// register material name / path
void Material::Register(std::string_view name, std::string_view path)
{
    Material::Impl::Register(name, path);
}

/// find material path from name
Expected<std::string_view> Material::Find(std::string_view name)
{
    return Material::Impl::Find(name);
}

/// materials are cached in the Library
bool Material::SupportsLibrary() const
{
    return true;
}

/// serialize
void Material::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void Material::Deserialize(const Serialized::Type &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// save material
bool Material::Save() const
{
    return mImpl->Save();
}

/// load material
bool Material::Load()
{
    return mImpl->Load();
}

/// release material
void Material::Release()
{
    mImpl->Release();
}

/// set property
void Material::SetProperty(std::string_view name, const PropertyValue &property) { mImpl->SetProperty(name, property); }

/// get property
[[nodiscard]] Expected<Material::PropertyValue> Material::GetProperty(std::string_view name) const { return mImpl->GetProperty(name); }

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
