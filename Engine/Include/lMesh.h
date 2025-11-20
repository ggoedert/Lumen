//==============================================================================================================================================================================
/// \file
/// \brief     Mesh interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lId.h"
#include "lExpected.h"
#include "lAsset.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(Mesh);
    CLASS_WEAK_PTR_DEF(Mesh);

    /// Mesh class
    class Mesh : public Asset
    {
        CLASS_NO_DEFAULT_CTOR(Mesh);
        CLASS_NO_COPY_MOVE(Mesh);
        OBJECT_TYPEINFO;

    public:
        /// destroys mesh
        ~Mesh() override;

        /// creates a smart pointer version of the mesh asset
        static AssetPtr MakePtr(EngineWeakPtr &engine, const std::filesystem::path &path);

        /// register mesh name / path
        static void Register(std::string_view name, std::string_view path);

        /// find mesh path from name
        static Expected<std::string_view> Find(std::string_view name);

        /// save a mesh
        bool Save() const override;

        /// load a mesh
        bool Load() override;

        /// release a mesh
        void Release() override;

        /// get mesh id
        Id::Type GetMeshId();

        /// get mesh data
        void GetMeshData(byte *data);

    private:
        /// constructs a mesh
        explicit Mesh(const EngineWeakPtr &engine, const std::filesystem::path &path);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
