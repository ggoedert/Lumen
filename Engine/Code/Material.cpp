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

using namespace Lumen;

/// Material::Impl class
class Material::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Material;

public:
    /// constructs a material
    explicit Impl(Material &owner) : mOwner(owner) {}

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
                    Serialized::SerializeValue(textureValue, packed, Serialized::cTextureTypeToken, Serialized::cTextureTypeTokenPacked, tex->Path().string());
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
        Expected<AssetPtr> shaderExp = AssetManager::Import(Shader::Type(), shaderPathExp.Value());
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
                    Expected<AssetPtr> textureExp = AssetManager::Import(Texture::Type(), path);
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
    bool Save() const { return true; }

    /// load material
    bool Load()
    {
        const std::filesystem::path &path = mOwner.Path();
        Lumen::DebugLog::Info("Material::Impl::Load {}", path.string());

        // read the material
        auto [materialData, packed] = FileSystem::ReadSerializedData(path);
        if (materialData.empty())
        {
            Lumen::DebugLog::Error("Unable to read the material");
            return false;
        }

        // parse the material
        try
        {
            const Serialized::Type in = Serialized::Type::parse(materialData);
            Deserialize(in, packed);
        }
        catch (const std::exception &e)
        {
            Lumen::DebugLog::Error("{}", e.what());
            return false;
        }
        return true;
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
