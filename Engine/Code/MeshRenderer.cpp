//==============================================================================================================================================================================
/// \file
/// \brief     MeshRenderer
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lDefs.h"
#include "lMeshRenderer.h"
#include "lMeshFilter.h"
#include "lShader.h"
#include "lTransform.h"
#include "lStringMap.h"

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
    explicit Impl(const EngineWeakPtr &engine, MaterialPtr material) : mEngine(engine), mMaterial(material) {}

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
MeshRenderer::MeshRenderer(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject, MaterialPtr material) :
    Component(Type(), Name(), gameObject), mImpl(MeshRenderer::Impl::MakeUniquePtr(engine, material)) {}

/// creates a smart pointer version of the mesh renderer component
ComponentPtr MeshRenderer::MakePtr(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject, const Object &params)
{
    if (params.Type() == MeshRenderer::Params::Type())
    {
        const auto &createParams = static_cast<const Params &>(params);
        auto pMeshRenderer = new MeshRenderer(engine, gameObject, createParams.mMaterial);
        auto meshRendererPtr = ComponentPtr(pMeshRenderer);
        pMeshRenderer->mImpl->mOwner = static_pointer_cast<MeshRenderer>(meshRendererPtr);
        return meshRendererPtr;
    }
#ifdef TYPEINFO
    DebugLog::Error("Create component, unknown parameter type: {}", params.Type().mName);
#else
    DebugLog::Error("Create component, unknown parameter hash type: 0x{:08X}", params.Type());
#endif
    return {};
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
