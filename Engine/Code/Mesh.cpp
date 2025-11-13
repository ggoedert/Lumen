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

    /// save a mesh
    bool Save() const { return true; }

    /// load a mesh
    bool Load() { return true; }

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

    /// get mesh data
    void GetMeshData(byte *data)
    {
        const std::filesystem::path &path = mOwner.lock()->Path();
        // if ("|Procedural|Sphere" == path.string()) do something?
    }

    /// owner
    MeshWeakPtr mOwner;

    /// engine pointer
    EngineWeakPtr mEngine;

    /// engine mesh id
    Id::Type mMeshId;
};

//==============================================================================================================================================================================

/// constructs a mesh
Mesh::Mesh(const EngineWeakPtr &engine, const std::filesystem::path &path) : Asset(Type(), path), mImpl(Mesh::Impl::MakeUniquePtr(engine)) {}

/// destroys mesh
Mesh::~Mesh()
{
    Release();
}

/// creates a smart pointer version of the mesh asset
AssetPtr Mesh::MakePtr(EngineWeakPtr &engine, const std::filesystem::path &path)
{
    auto ptr = MeshPtr(new Mesh(engine, path));
    ptr->mImpl->mOwner = ptr;
    if (auto engineLock = engine.lock())
    {
        ptr->mImpl->mMeshId = engineLock->CreateMesh(ptr);
        L_ASSERT_MSG(ptr->mImpl->mMeshId != Id::Invalid, "Failed to create mesh");
    }
    return ptr;
}

/// save a mesh
bool Mesh::Save() const
{
    return mImpl->Save();
}

/// load a mesh
bool Mesh::Load()
{
    return mImpl->Load();
}

/// release a mesh
void Mesh::Release()
{
    mImpl->Release();
}

/// get mesh id
Id::Type Mesh::GetMeshId()
{
    return mImpl->mMeshId;
}

/// get mesh data
void Mesh::GetMeshData(byte *data)
{
    mImpl->GetMeshData(data);
}
