//==============================================================================================================================================================================
/// \file
/// \brief     Camera interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lMath.h"
#include "lBehavior.h"

/// Lumen namespace
namespace Lumen
{
    class GameObject;
    CLASS_PTR_DEFS(Camera);

    /// Camera class
    class Camera : public Behavior
    {
        CLASS_NO_DEFAULT_CTOR(Camera);
        CLASS_NO_COPY_MOVE(Camera);
        COMPONENT_UTILS(Camera);

    public:
        /// constructs a camera with name and background color
        Camera(GameObjectPtr parent, Math::Vector backgroundColor) : Behavior(ComponentType(), ComponentName(), parent), mBackgroundColor(backgroundColor) {}

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
    inline Component *AddCamera(GameObjectPtr parent, Math::Vector backgroundColor) { return new Camera(parent, backgroundColor); }
}
