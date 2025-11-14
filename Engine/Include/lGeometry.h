//==============================================================================================================================================================================
/// \file
/// \brief     Geometry interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lComponent.h"
#include "lMesh.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Geometry);
    CLASS_WEAK_PTR_DEF(Geometry);

    /// Geometry class
    class Geometry : public Component
    {
        CLASS_NO_DEFAULT_CTOR(Geometry);
        CLASS_NO_COPY_MOVE(Geometry);
        COMPONENT_TYPEINFO;

    public:
        /// serialize
        void Serialize(Serialized::Type &out, bool packed) const override;

        /// deserialize
        void Deserialize(const Serialized::Type &in, bool packed) override;

        /// get mesh
        [[nodiscard]] const MeshPtr GetMesh() const;

        /// set mesh
        void SetMesh(const MeshPtr &mesh);

    private:
        /// constructs a geometry with a mesh
        explicit Geometry(const EntityWeakPtr &entity);

        /// creates a smart pointer version of the geometry component
        static ComponentPtr MakePtr(const EngineWeakPtr &engine, const EntityWeakPtr &entity);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
