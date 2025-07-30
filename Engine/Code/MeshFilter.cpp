//==============================================================================================================================================================================
/// \file
/// \brief     MeshFilter
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lDefs.h"
#include "lMeshFilter.h"

using namespace Lumen;

/// MeshFilter::Impl class
class MeshFilter::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class MeshFilter;

public:
    /// constructs a mesh filter
    explicit Impl(MeshPtr mesh) :mMesh(mesh) {}

    /// get mesh
    [[nodiscard]] const MeshPtr GetMesh() const { return mMesh;  }

    /// set mesh
    void SetMesh(const MeshPtr &mesh) { mMesh = mesh; }

private:
    /// run component
    void Run() {}

    /// mesh
    MeshPtr mMesh;
};

//==============================================================================================================================================================================

DEFINE_COMPONENT_TYPEINFO(MeshFilter);

/// constructs a mesh filter with name and background color
MeshFilter::MeshFilter(const GameObjectWeakPtr &gameObject, MeshPtr mesh) :
    Behavior(Type(), Name(), gameObject), mImpl(MeshFilter::Impl::MakeUniquePtr(mesh)) {}

/// creates a smart pointer version of the mesh filter component
ComponentPtr MeshFilter::MakePtr(const GameObjectWeakPtr &gameObject, const Object &params)
{
    if (params.Type() == MeshFilter::Params::Type())
    {
        const auto &createParams = static_cast<const Params &>(params);
        return ComponentPtr(new MeshFilter(gameObject, createParams.mMesh));
    }
#ifdef TYPEINFO
    DebugLog::Error("Create component, unknown parameter type: {}", params.Type().mName);
#else
    DebugLog::Error("Create component, unknown parameter hash type: 0x{:08X}", params.Type());
#endif
    return {};
}

/// get mesh
const MeshPtr MeshFilter::GetMesh() const { return mImpl->GetMesh(); }

/// set mesh
void MeshFilter::SetMesh(const MeshPtr &mesh) { mImpl->SetMesh(mesh); }

/// run component
void MeshFilter::Run() { mImpl->Run(); }
