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
        friend void SceneManager::Initialize();

    public:
        /// camera creation parameters
        struct Params { const GameObjectWeakPtr &gameObject; Math::Vector backgroundColor; };

        /// get background color
        [[nodiscard]] Math::Vector GetBackgroundColor() const;

        /// set background color
        void SetBackgroundColor(Math::Vector &backgroundColor);


    private:

        /// run component
        void Run() override;

        /// constructs a camera with name and background color
        Camera(const GameObjectWeakPtr &gameObject, Math::Vector backgroundColor);

        /// creates a camera
        static ComponentPtr Create(const std::any &params);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
