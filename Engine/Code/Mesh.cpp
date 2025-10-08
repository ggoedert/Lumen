//==============================================================================================================================================================================
/// \file
/// \brief     Mesh
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lMesh.h"

using namespace Lumen;

/// Mesh::Impl class
class Mesh::Impl
{
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Mesh;

public:
    /// constructs a mesh
    explicit Impl() {}
};

//==============================================================================================================================================================================

/// constructs a mesh
Mesh::Mesh(const EngineWeakPtr &engine) : Object(Type()), mEngine(engine), mMeshId(Id::Invalid), mImpl(Mesh::Impl::MakeUniquePtr()) {}

/// destroys mesh
Mesh::~Mesh()
{
    Release();
}

/// release a mesh
void Mesh::Release()
{
    if (Id::Invalid != mMeshId)
    {
        Id::Type meshId = mMeshId;
        mMeshId = Id::Invalid;
        if (auto engineLock = mEngine.lock())
        {
            engineLock->ReleaseMesh(meshId);
        }
    }
}
