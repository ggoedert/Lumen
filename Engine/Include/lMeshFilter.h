//==============================================================================================================================================================================
/// \file
/// \brief     MeshFilter interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lComponent.h"
#include "lMesh.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(MeshFilter);
    CLASS_WEAK_PTR_DEF(MeshFilter);

    /// MeshFilter class
    class MeshFilter : public Component
    {
        CLASS_NO_DEFAULT_CTOR(MeshFilter);
        CLASS_NO_COPY_MOVE(MeshFilter);
        COMPONENT_TYPEINFO;

    public:
        /// serialize
        void Serialize(json &out) const override;

        /// deserialize
        void Deserialize(const json &in) override;

        /// get mesh
        [[nodiscard]] const MeshPtr GetMesh() const;

        /// set mesh
        void SetMesh(const MeshPtr &mesh);

    private:
        /// constructs a mesh filter with a mesh
        explicit MeshFilter(const GameObjectWeakPtr &gameObject);

        /// creates a smart pointer version of the mesh filter component
        static ComponentPtr MakePtr(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
