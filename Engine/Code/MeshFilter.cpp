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
    void Serialize(json &out) const
    {
    }

    /// deserialize
    void Deserialize(const json &in)
    {
        mMesh.reset();

        std::string_view meshTypeName = Lumen::Mesh::Type().mName;
        if (in.contains(meshTypeName))
        {
            auto obj = in[meshTypeName];

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
                // load sphere mesh
                Lumen::Expected<Lumen::ObjectPtr> meshExp = Lumen::Assets::Import(path, Lumen::Mesh::Type(), name);
                if (!meshExp.HasValue())
                {
                    throw std::runtime_error(std::format("Unable to load default sphere mesh resource, {}", meshExp.Error()));
                }
                mMesh = static_pointer_cast<Lumen::Mesh>(meshExp.Value());
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
void MeshFilter::Serialize(json &out) const
{
    mImpl->Serialize(out);
}

/// deserialize
void MeshFilter::Deserialize(const json &in)
{
    mImpl->Deserialize(in);
}

/// get mesh
const MeshPtr MeshFilter::GetMesh() const { return mImpl->GetMesh(); }

/// set mesh
void MeshFilter::SetMesh(const MeshPtr &mesh) { mImpl->SetMesh(mesh); }
