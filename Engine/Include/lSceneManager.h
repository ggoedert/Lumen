//==============================================================================================================================================================================
/// \file
/// \brief     scene manager interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lComponent.h"
#include "lGameObject.h"
#include "lScene.h"

/// Lumen namespace
namespace Lumen
{
    /// SceneManager namespace
    namespace SceneManager
    {
        /// initialize scene manager namespace
        void Initialize();

        /// shutdown scene manager namespace
        void Shutdown();

        /// load scene
        bool Load(const ScenePtr &scene);

        /// unload current scene
        void Unload();

        /// register game object in the current scene
        [[nodiscard]] GameObjectWeakPtr RegisterGameObject(const GameObjectPtr &gameObject);

        /// unregister game object from the current scene
        bool UnregisterGameObject(const GameObjectWeakPtr &gameObject);

        /// register component
        [[nodiscard]] ComponentWeakPtr RegisterComponent(const ComponentPtr &component);

        /// unregister component
        bool UnregisterComponent(const ComponentWeakPtr &component);

        /// get all components of type
        [[nodiscard]] Components GetComponents(Type componentType);

        /// run application
        void Run();
    };
}
