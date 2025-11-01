//==============================================================================================================================================================================
/// \file
/// \brief     MeshFilter
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lMeshFilter.h"
#include "lTexture.h"
#include "lSceneManager.h"

using namespace Lumen;

/// MeshFilter::Impl class
class MeshFilter::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class MeshFilter;

public:
    /// constructs a mesh filter
    explicit Impl() = default;

    /// serialize
    void Serialize(Serialized::Type &out, bool packed) const
    {
        // token
        const std::string &meshTypeToken = packed ? Serialized::cMeshTypeTokenPacked : Serialized::cMeshTypeToken;

        Serialized::Type meshObj = Serialized::Type::object();
        meshObj[Serialized::cPathToken] = mMesh->Path();
        meshObj[Serialized::cNameToken] = mMesh->Name();
        out[meshTypeToken] = meshObj;
    }

    /// deserialize
    void Deserialize(const Serialized::Type &in, bool packed)
    {
        // token
        const std::string &meshTypeToken = packed ? Serialized::cMeshTypeTokenPacked : Serialized::cMeshTypeToken;

        mMesh.reset();

        if (in.contains(meshTypeToken))
        {
            auto obj = in[meshTypeToken];

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
                // load sphere mesh
                Expected<AssetPtr> meshExp = AssetManager::Import(path, Mesh::Type(), name);
                if (!meshExp.HasValue())
                {
                    throw std::runtime_error(std::format("Unable to load default sphere mesh resource, {}", meshExp.Error()));
                }
                mMesh = static_pointer_cast<Mesh>(meshExp.Value());
            }
        }
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

DEFINE_COMPONENT_TYPEINFO(MeshFilter);

/// constructs a mesh filter with a mesh
MeshFilter::MeshFilter(const GameObjectWeakPtr &gameObject) :
    Component(Type(), Name(), gameObject), mImpl(MeshFilter::Impl::MakeUniquePtr()) {}

/// creates a smart pointer version of the mesh filter component
ComponentPtr MeshFilter::MakePtr(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject)
{
    return ComponentPtr(new MeshFilter(gameObject));
}

/// serialize
void MeshFilter::Serialize(Serialized::Type &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void MeshFilter::Deserialize(const Serialized::Type &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// get mesh
const MeshPtr MeshFilter::GetMesh() const { return mImpl->GetMesh(); }

/// set mesh
void MeshFilter::SetMesh(const MeshPtr &mesh) { mImpl->SetMesh(mesh); }
