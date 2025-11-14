//==============================================================================================================================================================================
/// \file
/// \brief     Renderer
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lRenderer.h"
#include "lMaterial.h"
#include "lGeometry.h"
#include "lShader.h"
#include "lTransform.h"
#include "lStringMap.h"
#include "lSceneManager.h"

using namespace Lumen;

/// Renderer::Impl class
class Renderer::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Renderer;

public:
    /// constructs a renderer
    explicit Impl(const EngineWeakPtr &engine) : mEngine(engine) {}

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        Serialized::SerializeValue(out, packed, Serialized::cMaterialTypeToken, Serialized::cMaterialTypeTokenPacked, mMaterial->Path().string());

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
        mMaterial.reset();
        mProperties.clear();

        // load material
        Serialized::Type path = {};
        Serialized::DeserializeValue(in, packed, Serialized::cMaterialTypeToken, Serialized::cMaterialTypeTokenPacked, path);
        if (path.empty())
        {
            throw std::runtime_error(std::format("Unable to load material resource, no path in material asset"));
        }
        Expected<AssetPtr> materialExp = AssetManager::Import(Material::Type(), path);
        if (!materialExp.HasValue())
        {
            throw std::runtime_error(std::format("Unable to load material resource, {}", materialExp.Error()));
        }
        mMaterial = static_pointer_cast<Material>(materialExp.Value());

        Serialized::Type propertiesObj = Serialized::Type::object();
        if (Serialized::DeserializeValue(in, packed, Serialized::cPropertiesToken, Serialized::cPropertiesTokenPacked, propertiesObj))
        {
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

    /// get material
    [[nodiscard]] MaterialPtr GetMaterial() const { return mMaterial; }

    /// set material
    void SetMaterial(const MaterialPtr &material) { mMaterial = material; }

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

    /// render with a geometry
    void Render()
    {
        if (auto engineLock = mEngine.lock())
        {
            if (auto renderer = mOwner.lock())
            {
                if (auto entity = renderer->Entity().lock())
                {
                    if (auto geometry = std::static_pointer_cast<Geometry>(entity->Component(Geometry::Type()).lock()))
                    {
                        MeshPtr mesh = geometry->GetMesh();
                        ShaderPtr shader = mMaterial->GetShader();
                        TexturePtr texture;
                        auto textureProperty = GetProperty("diffuseTex");
                        if (textureProperty.HasValue())
                        {
                            if (auto pTexture = std::get_if<TexturePtr>(&textureProperty.Value()))
                            {
                                texture = *pTexture;
                            }
                        }
                        if (mesh && shader && texture)
                        {
                            Engine::DrawPrimitive drawPrimitive;
                            drawPrimitive.meshId = mesh->GetMeshId();
                            drawPrimitive.shaderId = shader->GetShaderId();
                            drawPrimitive.texId = texture->GetTextureId();
                            entity->Transform().lock()->GetWorldMatrix(drawPrimitive.world);
                            engineLock->PushRenderCommand(Engine::RenderCommand(drawPrimitive));
                        }

                    }
                }
            }
        }
    }

    /// owner
    RendererWeakPtr mOwner;

    /// engine pointer
    EngineWeakPtr mEngine;

    /// material
    MaterialPtr mMaterial;

    /// map of properties
    StringMap<PropertyValue> mProperties;
};

//==============================================================================================================================================================================

DEFINE_COMPONENT_TYPEINFO(Renderer);

/// constructs a renderer with an material
Renderer::Renderer(const EngineWeakPtr &engine, const EntityWeakPtr &entity) :
    Component(Type(), Name(), entity), mImpl(Renderer::Impl::MakeUniquePtr(engine)) {}

/// creates a smart pointer version of the renderer component
ComponentPtr Renderer::MakePtr(const EngineWeakPtr &engine, const EntityWeakPtr &entity)
{
    auto ptr = RendererPtr(new Renderer(engine, entity));
    ptr->mImpl->mOwner = ptr;
    return ptr;
}

/// serialize
void Renderer::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void Renderer::Deserialize(const Serialized::Type &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// set property
void Renderer::SetProperty(std::string_view name, const PropertyValue &property) { mImpl->SetProperty(name, property); }

/// get property
[[nodiscard]] Expected<Renderer::PropertyValue> Renderer::GetProperty(std::string_view name) const { return mImpl->GetProperty(name); }

/// render with a geometry
void Renderer::Render()
{
    mImpl->Render();
}
