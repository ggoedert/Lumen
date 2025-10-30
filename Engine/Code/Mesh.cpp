//==============================================================================================================================================================================
/// \file
/// \brief     Mesh
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lMesh.h"
#include "lEngine.h"

using namespace Lumen;

/// Mesh::Impl class
class Mesh::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);
    friend class Mesh;

public:
    /// constructs a mesh
    explicit Impl(const EngineWeakPtr &engine) : mEngine(engine), mMeshId(Id::Invalid) {}

    /// release a mesh
    void Release()
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

    /// get mesh id
    Id::Type GetMeshId()
    {
        return mMeshId;
    }

    /// set mesh id
    void SetMeshId(Id::Type meshId)
    {
        mMeshId = meshId;
    }

    /// engine pointer
    EngineWeakPtr mEngine;

    /// engine mesh id
    Id::Type mMeshId;
};

//==============================================================================================================================================================================

/// constructs a mesh
Mesh::Mesh(const EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name) : Object(Type()), mImpl(Mesh::Impl::MakeUniquePtr(engine)) {}

/// destroys mesh
Mesh::~Mesh()
{
    Release();
}

/// release a mesh
void Mesh::Release()
{
    mImpl->Release();
}

/// get mesh id
Id::Type Mesh::GetMeshId()
{
    return mImpl->GetMeshId();
}

/// set mesh id
void Mesh::SetMeshId(Id::Type meshId)
{
    mImpl->SetMeshId(meshId);
}
