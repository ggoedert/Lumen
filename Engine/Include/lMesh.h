//==============================================================================================================================================================================
/// \file
/// \brief     Mesh interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lId.h"
#include "lObject.h"

#include <filesystem>

/// Lumen namespace
namespace Lumen
{
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(Mesh);

    /// Mesh class
    class Mesh : public Object
    {
        CLASS_NO_COPY_MOVE(Mesh);
        OBJECT_TYPEINFO;

    public:
        /// destroys mesh
        ~Mesh() override;

        /// release a mesh
        void Release();

        /// get mesh data
        virtual void GetMeshData(byte *data) = 0;

        /// get mesh id
        Id::Type GetMeshId();

        /// set mesh id
        void SetMeshId(Id::Type meshId);

    protected:
        /// constructs a mesh
        explicit Mesh(const EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
