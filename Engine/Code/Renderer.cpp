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
#include "lAssetManager.h"
#include "lSceneManager.h"
#include "lDrawPrimitive.h"

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
    explicit Impl(Renderer &owner, const EngineWeakPtr &engine) : mOwner(owner), mEngine(engine) {}

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        Serialized::SerializeValue(out, packed, Serialized::cMaterialTypeToken, Serialized::cMaterialTypeTokenPacked, mMaterial->Path().string());
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        mMaterial.reset();

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
    }

    /// get material
    [[nodiscard]] MaterialPtr GetMaterial() const { return mMaterial; }

    /// set material
    void SetMaterial(const MaterialPtr &material) { mMaterial = material; }

    /// render with a geometry
    void Render()
    {
        if (auto engineLock = mEngine.lock())
        {
            if (auto entity = mOwner.Entity().lock())
            {
                if (auto geometry = std::static_pointer_cast<Geometry>(entity->Component(Geometry::Type()).lock()))
                {
                    MeshPtr mesh = geometry->GetMesh();
                    ShaderPtr shader = mMaterial->GetShader();
                    TexturePtr texture;
                    auto textureProperty = mMaterial->GetProperty("diffuseTex");
                    if (textureProperty.HasValue())
                    {
                        if (auto pTexture = std::get_if<TexturePtr>(&textureProperty.Value()))
                        {
                            texture = *pTexture;
                        }
                    }
                    if (mesh && shader && texture)
                    {
                        Math::Matrix44 world;
                        entity->Transform().lock()->GetWorldMatrix(world);
                        engineLock->PostRenderCommand(DrawPrimitive::MakeUniquePtr(mesh->GetMeshId(), shader->GetShaderId(), texture->GetTextureId(), world));
                    }
                }
            }
        }
    }

    /// owner
    Renderer &mOwner;

    /// engine pointer
    EngineWeakPtr mEngine;

    /// material
    MaterialPtr mMaterial;
};

//==============================================================================================================================================================================

DEFINE_COMPONENT_TYPEINFO(Renderer);

/// constructs a renderer with an material
Renderer::Renderer(const EngineWeakPtr &engine, const EntityWeakPtr &entity) :
    Component(Type(), Name(), entity), mImpl(Renderer::Impl::MakeUniquePtr(*this, engine)) {}

/// creates a smart pointer version of the renderer component
ComponentPtr Renderer::MakePtr(const EngineWeakPtr &engine, const EntityWeakPtr &entity)
{
    return RendererPtr(new Renderer(engine, entity));
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

/// render with a geometry
void Renderer::Render()
{
    mImpl->Render();
}
