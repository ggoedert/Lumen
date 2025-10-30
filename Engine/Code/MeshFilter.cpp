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
    void Serialize(SerializedData &out, bool packed) const
    {
    }

    /// deserialize
    void Deserialize(const SerializedData &in, bool packed)
    {
        // tokens
        const std::string &pathToken = packed ? mPackedPathToken : "Path";
        const std::string &nameToken = packed ? mPackedNameToken : "Name";
        const std::string &meshTypeToken = "Lumen::Mesh";

        mMesh.reset();

        std::string meshTypeKey;
        if (packed)
        {
            meshTypeKey = Base64Encode(Mesh::Type());
        }
        else
        {
            meshTypeKey = meshTypeToken;
        }
        if (in.contains(meshTypeKey))
        {
            auto obj = in[meshTypeKey];

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
                // load sphere mesh
                Expected<ObjectPtr> meshExp = AssetManager::Import(path, Mesh::Type(), name);
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

    /// packed path token
    static const std::string mPackedPathToken; //@REVIEW@ FIXME move this to some common place

    /// packed name token
    static const std::string mPackedNameToken; //@REVIEW@ FIXME move this to some common place
};

const std::string MeshFilter::Impl::mPackedPathToken = Base64Encode(HashString("Path"));

const std::string MeshFilter::Impl::mPackedNameToken = Base64Encode(HashString("Name"));

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
void MeshFilter::Serialize(SerializedData &out, bool packed) const
{
    mImpl->Serialize(out, packed);
}

/// deserialize
void MeshFilter::Deserialize(const SerializedData &in, bool packed)
{
    mImpl->Deserialize(in, packed);
}

/// get mesh
const MeshPtr MeshFilter::GetMesh() const { return mImpl->GetMesh(); }

/// set mesh
void MeshFilter::SetMesh(const MeshPtr &mesh) { mImpl->SetMesh(mesh); }
