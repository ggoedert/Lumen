//==============================================================================================================================================================================
/// \file
/// \brief     MeshFilter interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lBehavior.h"
#include "lSceneManager.h"

/// Lumen namespace
namespace Lumen
{
    /// MeshFilter class
    class MeshFilter : public Behavior
    {
        CLASS_NO_DEFAULT_CTOR(MeshFilter);
        CLASS_NO_COPY_MOVE(MeshFilter);
        COMPONENT_TRAITS(MeshFilter);
        friend void SceneManager::Initialize();

    public:
        /// mesh filter creation parameters
        struct Params {};

    private:
        /// constructs a mesh filter with name and background color
        explicit MeshFilter(const GameObjectWeakPtr &gameObject/*, Math::Vector backgroundColor*/);

        /// creates a smart pointer version of the mesh filter component
        static ComponentPtr MakePtr(const GameObjectWeakPtr &gameObject, const std::any &params);

        /// run component
        void Run() override;

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
