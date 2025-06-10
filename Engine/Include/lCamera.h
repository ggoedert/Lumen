//==============================================================================================================================================================================
/// \file
/// \brief     Camera interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lMath.h"
#include "lBehavior.h"
#include "lSceneManager.h"
#include "lGameObject.h"

/// Lumen namespace
namespace Lumen
{
    /// Camera class
    class Camera : public Behavior
    {
        CLASS_NO_DEFAULT_CTOR(Camera);
        CLASS_NO_COPY_MOVE(Camera);
        COMPONENT_TRAITS(Camera);

    public:
        /// constructs a camera with name and background color
        Camera(const GameObjectWeakPtr &gameObject, Math::Vector backgroundColor) : Behavior(ComponentType(), ComponentName(), gameObject), mBackgroundColor(backgroundColor) {}

        /// set background color
        [[nodiscard]] void BackgroundColor(Math::Vector &backgroundColor) { mBackgroundColor = backgroundColor; }

        /// return background color
        [[nodiscard]] Math::Vector &BackgroundColor() { return mBackgroundColor; }

    private:
        /// run component
        void Run() override {}

        /// background color
        Math::Vector mBackgroundColor;
    };

    /// creator helper
    inline ComponentWeakPtr AddCamera(const GameObjectWeakPtr &gameObject, const Math::Vector &backgroundColor)
    {
        auto lockedGameObject = gameObject.lock();
        if (lockedGameObject)
        {
            auto component = SceneManager::RegisterComponent(ComponentPtr(new Camera(gameObject, backgroundColor)));
            lockedGameObject.get()->AddComponent(component);
            return component;
        }
        return {};
    }
}
