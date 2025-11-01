//==============================================================================================================================================================================
/// \file
/// \brief     MeshRenderer
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lMeshRenderer.h"
#include "lMaterial.h"
#include "lMeshFilter.h"
#include "lShader.h"
#include "lTransform.h"
#include "lStringMap.h"
#include "lSceneManager.h"

using namespace Lumen;

/// MeshRenderer::Impl class
class MeshRenderer::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class MeshRenderer;

public:
    /// constructs a mesh renderer
    explicit Impl(const EngineWeakPtr &engine) : mEngine(engine) {}

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        const std::string &materialTypeToken = packed ? Serialized::cMaterialTypeTokenPacked : Serialized::cMaterialTypeToken;
        const std::string &propertiesToken = packed ? Serialized::cPropertiesTokenPacked : Serialized::cPropertiesToken;
        const std::string &textureTypeToken = packed ? Serialized::cTextureTypeTokenPacked : Serialized::cTextureTypeToken;

        Serialized::Type materialObj = Serialized::Type::object();
        materialObj[Serialized::cPathToken] = mMaterial->Path();
        materialObj[Serialized::cNameToken] = mMaterial->Name();
        out[materialTypeToken] = materialObj;

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
                Serialized::Type textureValue = Serialized::Type::object();
                auto tex = std::get<Lumen::TexturePtr>(value);
                if (tex)
                {
                    textureValue[Serialized::cPathToken] = tex->Path();
                    textureValue[Serialized::cNameToken] = tex->Name();
                }
                Serialized::Type textureObj = Serialized::Type::object();
                textureObj[textureTypeToken] = textureValue;
                propertiesObj[key] = textureObj;
            }
        }
        out[propertiesToken] = propertiesObj;
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        // tokens
        const std::string &materialTypeToken = packed ? Serialized::cMaterialTypeTokenPacked : Serialized::cMaterialTypeToken;
        const std::string &propertiesToken = packed ? Serialized::cPropertiesTokenPacked : Serialized::cPropertiesToken;
        const std::string &textureTypeToken = packed ? Serialized::cTextureTypeTokenPacked : Serialized::cTextureTypeToken;

        mMaterial.reset();
        mProperties.clear();

        for (auto &inItem : in.items())
        {
            if (inItem.key() == materialTypeToken)
            {
                auto &obj = inItem.value();

                std::string path, name;
                if (obj.contains(Serialized::cPathToken))
                {
                    path = obj[Serialized::cPathToken].get<std::string>();
                }
                if (obj.contains(Serialized::cNameToken))
                {
                    name = obj[Serialized::cNameToken].get<std::string>();
                }
                if (!path.empty() && !name.empty())
                {
                    // load material material
                    Expected<AssetPtr> materialExp = AssetManager::Import(path, Material::Type(), name);
                    if (!materialExp.HasValue())
                    {
                        throw std::runtime_error(std::format("Unable to load material resource, {}", materialExp.Error()));
                    }
                    mMaterial = static_pointer_cast<Material>(materialExp.Value());
                }
            }

            if (inItem.key() == propertiesToken) //@REVIEW@ FIXME move this inside the material
            {
                for (auto &inProperty : inItem.value().items())
                {
                    auto &propertyValue = inProperty.value();
                    if (propertyValue.contains(textureTypeToken))
                    {
                        auto propertyData = propertyValue[textureTypeToken];
                        std::string path, name;
                        if (propertyData.contains(Serialized::cPathToken))
                        {
                            path = propertyData[Serialized::cPathToken].get<std::string>();
                        }
                        if (propertyData.contains(Serialized::cNameToken))
                        {
                            name = propertyData[Serialized::cNameToken].get<std::string>();
                        }
                        if (!path.empty() && !name.empty())
                        {
                            // load texture
                            Expected<AssetPtr> textureExp = AssetManager::Import(path, Texture::Type(), name);
                            if (!textureExp.HasValue())
                            {
                                throw std::runtime_error(std::format("Unable to load default checker gray texture resource, {}", textureExp.Error()));
                            }
                            const TexturePtr texture = static_pointer_cast<Texture>(textureExp.Value());

                            // set property
                            SetProperty(inProperty.key(), texture);
                        }
                    }
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
        //DebugLog::Info("MeshRenderer::SetProperty {} to {}", name, property);
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

    /// render with a mesh filter
    void Render()
    {
        if (auto engineLock = mEngine.lock())
        {
            if (auto meshRenderer = mOwner.lock())
            {
                if (auto gameObject = meshRenderer->GameObject().lock())
                {
                    if (auto meshFilter = std::static_pointer_cast<MeshFilter>(gameObject->Component(MeshFilter::Type()).lock()))
                    {
                        ShaderPtr shader = mMaterial->GetShader();
                        MeshPtr mesh = meshFilter->GetMesh();
                        TexturePtr texture;
                        auto textureProperty = GetProperty("_MainTex");
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
                            gameObject->Transform().lock()->GetWorldMatrix(drawPrimitive.world);
                            engineLock->PushRenderCommand(Engine::RenderCommand(drawPrimitive));
                        }

                    }
                }
            }
        }
    }

    /// owner
    MeshRendererWeakPtr mOwner;

    /// engine pointer
    EngineWeakPtr mEngine;

    /// material
    MaterialPtr mMaterial;

    /// map of properties
    StringMap<PropertyValue> mProperties;
};

//==============================================================================================================================================================================

DEFINE_COMPONENT_TYPEINFO(MeshRenderer);

/// constructs a mesh renderer with an material
MeshRenderer::MeshRenderer(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject) :
    Component(Type(), Name(), gameObject), mImpl(MeshRenderer::Impl::MakeUniquePtr(engine)) {}

/// creates a smart pointer version of the mesh renderer component
ComponentPtr MeshRenderer::MakePtr(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject)
{
    auto pMeshRenderer = new MeshRenderer(engine, gameObject);
    auto meshRendererPtr = ComponentPtr(pMeshRenderer);
    pMeshRenderer->mImpl->mOwner = static_pointer_cast<MeshRenderer>(meshRendererPtr);
    return meshRendererPtr;
}

/// serialize
void MeshRenderer::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void MeshRenderer::Deserialize(const Serialized::Type &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// set property
void MeshRenderer::SetProperty(std::string_view name, const PropertyValue &property) { mImpl->SetProperty(name, property); }

/// get property
[[nodiscard]] Expected<MeshRenderer::PropertyValue> MeshRenderer::GetProperty(std::string_view name) const { return mImpl->GetProperty(name); }

/// render with a mesh filter
void MeshRenderer::Render()
{
    mImpl->Render();
}
