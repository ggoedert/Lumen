//==============================================================================================================================================================================
/// \file
/// \brief     MeshFilter interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lComponent.h"
#include "lMesh.h"
#include "lSceneManager.h"

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
        /// mesh filter creation parameters
        struct Params : Object
        {
            OBJECT_TYPEINFO;
            explicit Params(MeshPtr mesh) : Object(Type()), mMesh(mesh) {}
            MeshPtr mMesh;
        };

        /// get mesh
        [[nodiscard]] const MeshPtr GetMesh() const;

        /// set mesh
        void SetMesh(const MeshPtr &mesh);

    private:
        /// constructs a mesh filter with a mesh
        explicit MeshFilter(const GameObjectWeakPtr &gameObject, const MeshPtr &mesh);

        /// creates a smart pointer version of the mesh filter component
        static ComponentPtr MakePtr(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject, const Object &params);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
