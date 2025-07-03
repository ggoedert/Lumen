//==============================================================================================================================================================================
/// \file
/// \brief     MeshFilter interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lBehavior.h"
#include "lMesh.h"
#include "lSceneManager.h"
#include "lProperty.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(MeshFilter);

    /// MeshFilter class
    class MeshFilter : public Behavior
    {
        CLASS_NO_DEFAULT_CTOR(MeshFilter);
        CLASS_NO_COPY_MOVE(MeshFilter);
        COMPONENT_TYPEINFO;
        friend void SceneManager::Initialize();

    public:
        /// mesh filter creation parameters
        struct Params : Object
        {
            OBJECT_TYPEINFO;
            explicit Params(MeshPtr mesh) : Object(Type()), mesh(mesh) {}
            MeshPtr mesh;
        };

        /// mesh property
        PROPERTY(MeshPtr, Mesh, GetMesh, SetMesh);

    private:
        /// constructs a mesh filter with name and background color
        explicit MeshFilter(const GameObjectWeakPtr &gameObject, MeshPtr mesh);

        /// creates a smart pointer version of the mesh filter component
        static ComponentPtr MakePtr(const GameObjectWeakPtr &gameObject, const Object &params);

        /// get mesh
        [[nodiscard]] const MeshPtr GetMesh() const;

        /// set mesh
        void SetMesh(const MeshPtr &mesh);

        /// run component
        void Run() override;

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
