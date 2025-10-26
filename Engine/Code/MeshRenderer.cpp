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
    void Serialize(json &out) const
    {
    }

    /// deserialize
    void Deserialize(const json &in)
    {
        mMaterial.reset();
        mProperties.clear();

        for (auto inItem : in.items())
        {
            if (inItem.key() == Lumen::Material::Type().mName)
            {
                auto obj = inItem.value();

                std::string path, name;
                if (obj.contains("Path"))
                {
                    path = obj["Path"].get<std::string>();
                }
                if (obj.contains("Name"))
                {
                    name = obj["Name"].get<std::string>();
                }
                if (!path.empty() && !name.empty())
                {
                    // load material material
                    Lumen::Expected<Lumen::ObjectPtr> materialExp = Lumen::Assets::Import(path, Lumen::Material::Type(), name);
                    if (!materialExp.HasValue())
                    {
                        throw std::runtime_error(std::format("Unable to load material resource, {}", materialExp.Error()));
                    }
                    mMaterial = static_pointer_cast<Lumen::Material>(materialExp.Value());
                }
            }

            if (inItem.key() == "Properties")
            {
                for (auto inProperty : inItem.value().items())
                {
                    auto propertyValue = inProperty.value();
                    if (propertyValue.contains("Lumen::Texture"))
                    {
                        auto propertyData = propertyValue["Lumen::Texture"];
                        std::string path, name;
                        if (propertyData.contains("Path"))
                        {
                            path = propertyData["Path"].get<std::string>();
                        }
                        if (propertyData.contains("Name"))
                        {
                            name = propertyData["Name"].get<std::string>();
                        }
                        if (!path.empty() && !name.empty())
                        {
                            // load texture
                            Lumen::Expected<Lumen::ObjectPtr> textureExp = Lumen::Assets::Import(path, Lumen::Texture::Type(), name);
                            if (!textureExp.HasValue())
                            {
                                throw std::runtime_error(std::format("Unable to load default checker gray texture resource, {}", textureExp.Error()));
                            }
                            const Lumen::TexturePtr texture = static_pointer_cast<Lumen::Texture>(textureExp.Value());

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
                            if (auto pTexture = std::get_if<Lumen::TexturePtr>(&textureProperty.Value()))
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
void MeshRenderer::Serialize(json &out) const
{
    mImpl->Serialize(out);
}

/// deserialize
void MeshRenderer::Deserialize(const json &in)
{
    mImpl->Deserialize(in);
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
