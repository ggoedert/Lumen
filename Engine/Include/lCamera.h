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
#include "lProperty.h"

/// Lumen namespace
namespace Lumen
{
    /// Camera class
    class Camera : public Behavior
    {
        CLASS_NO_DEFAULT_CTOR(Camera);
        CLASS_NO_COPY_MOVE(Camera);
        COMPONENT_TRAITS(Camera);
        friend void SceneManager::Initialize();

    public:
        /// camera creation parameters
        struct Params { Math::Vector backgroundColor; };

        /// background color property
        PROPERTY(Lumen::Math::Vector, BackgroundColor);

    private:
        /// constructs a camera with name and background color
        explicit Camera(const GameObjectWeakPtr &gameObject, Math::Vector backgroundColor);

        /// creates a smart pointer version of the camera component
        static ComponentPtr MakePtr(const GameObjectWeakPtr &gameObject, const std::any &params);

        /// run component
        void Run() override;

        /// get the camera's background color
        [[nodiscard]] const Math::Vector &GetBackgroundColor() const;

        /// set the camera's background color
        const Math::Vector &SetBackgroundColor(const Math::Vector &backgroundColor);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
