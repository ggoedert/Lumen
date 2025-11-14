//==============================================================================================================================================================================
/// \file
/// \brief     Geometry
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lGeometry.h"
#include "lTexture.h"
#include "lSceneManager.h"

using namespace Lumen;

/// Geometry::Impl class
class Geometry::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Geometry;

public:
    /// constructs a geometry
    explicit Impl() = default;

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        Serialized::SerializeValue(out, packed, Serialized::cMeshTypeToken, Serialized::cMeshTypeTokenPacked, mMesh->Path().string());
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        mMesh.reset();

        // load mesh
        Serialized::Type path = {};
        Serialized::DeserializeValue(in, packed, Serialized::cMeshTypeToken, Serialized::cMeshTypeTokenPacked, path);
        if (path.empty())
        {
            throw std::runtime_error(std::format("Unable to load mesh resource, no path in mesh asset"));
        }
        Expected<AssetPtr> meshExp = AssetManager::Import(Mesh::Type(), path);
        if (!meshExp.HasValue())
        {
            throw std::runtime_error(std::format("Unable to load mesh resource, {}", meshExp.Error()));
        }
        mMesh = static_pointer_cast<Mesh>(meshExp.Value());
    }

    /// get mesh
    [[nodiscard]] const MeshPtr GetMesh() const { return mMesh;  }

    /// set mesh
    void SetMesh(const MeshPtr &mesh) { mMesh = mesh; }

private:
    /// mesh
    MeshPtr mMesh;
};

//==============================================================================================================================================================================

DEFINE_COMPONENT_TYPEINFO(Geometry);

/// constructs a geometry with a mesh
Geometry::Geometry(const EntityWeakPtr &entity) :
    Component(Type(), Name(), entity), mImpl(Geometry::Impl::MakeUniquePtr()) {}

/// creates a smart pointer version of the geometry component
ComponentPtr Geometry::MakePtr(const EngineWeakPtr &engine, const EntityWeakPtr &entity)
{
    return ComponentPtr(new Geometry(entity));
}

/// serialize
void Geometry::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void Geometry::Deserialize(const Serialized::Type &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// get mesh
const MeshPtr Geometry::GetMesh() const { return mImpl->GetMesh(); }

/// set mesh
void Geometry::SetMesh(const MeshPtr &mesh) { mImpl->SetMesh(mesh); }
