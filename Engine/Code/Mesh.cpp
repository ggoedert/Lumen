//==============================================================================================================================================================================
/// \file
/// \brief     Mesh
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lMesh.h"
#include "lStringMap.h"
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
    explicit Impl(Mesh &owner, const EngineWeakPtr &engine) : mOwner(owner), mEngine(engine), mMeshId(Id::Invalid) {}

    /// register mesh name / path
    static void Register(std::string_view name, std::string_view path)
    {
        // register asset
        mAssetPaths.insert_or_assign(std::string(name), path);
    }

    /// find mesh path from name
    static Expected<std::string_view> Find(std::string_view name)
    {
        // find asset
        auto it = mAssetPaths.find(name);
        if (it != mAssetPaths.end())
        {
            return it->second;
        }

        // none found
        return Expected<std::string_view>::Unexpected(std::format("Mesh path for '{}' not found", name));
    }

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
        const std::filesystem::path &path = mOwner.Path();
        // if ("|Procedural|Sphere" == path.string()) do something?
    }

    /// owner
    Mesh &mOwner;

    /// engine pointer
    EngineWeakPtr mEngine;

    /// engine mesh id
    Id::Type mMeshId;

    /// static map of asset names to paths
    static StringMap<std::string> mAssetPaths;
};

StringMap<std::string> Mesh::Impl::mAssetPaths;

//==============================================================================================================================================================================

/// constructs a mesh
Mesh::Mesh(const EngineWeakPtr &engine, const std::filesystem::path &path) : Asset(Type(), path), mImpl(Mesh::Impl::MakeUniquePtr(*this, engine)) {}

/// destroys mesh
Mesh::~Mesh()
{
    Release();
}

/// creates a smart pointer version of the mesh asset
AssetPtr Mesh::MakePtr(EngineWeakPtr &engine, const std::filesystem::path &path)
{
    auto ptr = MeshPtr(new Mesh(engine, path));
    if (auto engineLock = engine.lock())
    {
        ptr->mImpl->mMeshId = engineLock->CreateMesh(ptr);
        L_ASSERT_MSG(ptr->mImpl->mMeshId != Id::Invalid, "Failed to create mesh");
    }
    return ptr;
}

/// register mesh name / path
void Mesh::Register(std::string_view name, std::string_view path)
{
    Mesh::Impl::Register(name, path);
}

/// find mesh path from name
Expected<std::string_view> Mesh::Find(std::string_view name)
{
    return Mesh::Impl::Find(name);
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
