//==============================================================================================================================================================================
/// \file
/// \brief     Mesh interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lId.h"
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
        static AssetPtr MakePtr(EngineWeakPtr &engine, std::string_view name, const std::filesystem::path &path);

        /// release a mesh
        void Release() override;

        /// get mesh id
        Id::Type GetMeshId();

        /// get mesh data
        void GetMeshData(byte *data);

    private:
        /// constructs a mesh
        explicit Mesh(const EngineWeakPtr &engine, std::string_view name, const std::filesystem::path &path);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
