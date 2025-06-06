//==============================================================================================================================================================================
/// \file
/// \brief     game object interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lTransform.h"

/// Lumen namespace
namespace Lumen
{
    class Component;
    class Application;
    CLASS_PTR_DEFS(GameObject);

    /// GameObject class
    class GameObject : public Object
    {
        friend class SceneManager;
        CLASS_NO_DEFAULT_CTOR(GameObject);
        CLASS_NO_COPY_MOVE(GameObject);
        CLASS_PTR_MAKERS(GameObject);

    public:
        /// constructs a game object with a scene manager
        GameObject(SceneManager &sceneManager);

        /// destroys game object
        ~GameObject();

        /// get transform
        [[nodiscard]] Transform &GetTransform() { return mTransform; }

        /// get component
        Component *GetComponent(Type componentType);

        /// add a component
        void AddComponent(Component *component);

        /// get application
        [[nodiscard]] Application &GetApplication();

    protected:
        /// run game object
        void Run();

    private:
        // Scene manager reference
        SceneManager &mSceneManager;

        /// transform
        Transform mTransform;

        /// components
        std::vector<Component *> mComponents;
    };
}
