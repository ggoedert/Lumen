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
        Camera(const GameObjectWeakPtr &gameObject, Math::Vector backgroundColor);

        /// get background color
        [[nodiscard]] Math::Vector GetBackgroundColor() const;

        /// set background color
        void SetBackgroundColor(Math::Vector &backgroundColor);

    private:
        /// run component
        void Run() override;

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };

    /// creator helper
    inline ComponentWeakPtr AddCamera(const GameObjectWeakPtr &gameObject, const Math::Vector &backgroundColor)
    {
        auto lockedGameObject = gameObject.lock();
        if (lockedGameObject)
        {
            auto component = ComponentPtr(new Camera(gameObject, backgroundColor));
            lockedGameObject->AddComponent(component);
            return component;
        }
        return {};
    }
}
