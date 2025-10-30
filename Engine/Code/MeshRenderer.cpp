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
    void Serialize(SerializedData &out, bool packed) const
    {
    }

    /// deserialize
    void Deserialize(const SerializedData &in, bool packed)
    {
        // tokens
        const std::string &pathToken = packed ? mPackedPathToken : "Path";
        const std::string &nameToken = packed ? mPackedNameToken : "Name";
        const std::string &textureTypeToken = "Lumen::Texture";

        mMaterial.reset();
        mProperties.clear();

        for (auto &inItem : in.items())
        {
            if (inItem.key() == Material::Type().mName)
            {
                auto &obj = inItem.value();

                std::string path, name;
                if (obj.contains(pathToken))
                {
                    path = obj[pathToken].get<std::string>();
                }
                if (obj.contains(nameToken))
                {
                    name = obj[nameToken].get<std::string>();
                }
                if (!path.empty() && !name.empty())
                {
                    // load material material
                    Expected<ObjectPtr> materialExp = AssetManager::Import(path, Material::Type(), name);
                    if (!materialExp.HasValue())
                    {
                        throw std::runtime_error(std::format("Unable to load material resource, {}", materialExp.Error()));
                    }
                    mMaterial = static_pointer_cast<Material>(materialExp.Value());
                }
            }

            if (inItem.key() == "Properties")
            {
                for (auto &inProperty : inItem.value().items())
                {
                    auto &propertyValue = inProperty.value();
                    std::string textureTypeKey;
                    if (packed)
                    {
                        textureTypeKey = Base64Encode(Texture::Type());
                    }
                    else
                    {
                        textureTypeKey = textureTypeToken;
                    }
                    if (propertyValue.contains(textureTypeKey))
                    {
                        auto propertyData = propertyValue[textureTypeKey];
                        std::string path, name;
                        if (propertyData.contains(pathToken))
                        {
                            path = propertyData[pathToken].get<std::string>();
                        }
                        if (propertyData.contains(nameToken))
                        {
                            name = propertyData[nameToken].get<std::string>();
                        }
                        if (!path.empty() && !name.empty())
                        {
                            // load texture
                            Expected<ObjectPtr> textureExp = AssetManager::Import(path, Texture::Type(), name);
                            if (!textureExp.HasValue())
                            {
                                throw std::runtime_error(std::format("Unable to load default checker gray texture resource, {}", textureExp.Error()));
                            }
                            const TexturePtr texture = static_pointer_cast<Texture>(textureExp.Value());

                            // set property
                            SetProperty("_MainTex", texture);
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

    /// packed path token
    static const std::string mPackedPathToken; //@REVIEW@ FIXME move this to some common place

    /// packed name token
    static const std::string mPackedNameToken; //@REVIEW@ FIXME move this to some common place
};

const std::string MeshRenderer::Impl::mPackedPathToken = Base64Encode(HashString("Path"));

const std::string MeshRenderer::Impl::mPackedNameToken = Base64Encode(HashString("Name"));

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
void MeshRenderer::Serialize(SerializedData &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void MeshRenderer::Deserialize(const SerializedData &in, bool packed)
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
