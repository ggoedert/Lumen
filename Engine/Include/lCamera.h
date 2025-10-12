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
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(Behavior);
    CLASS_WEAK_PTR_DEF(Behavior);

    /// Camera class
    class Camera : public Behavior
    {
        CLASS_NO_DEFAULT_CTOR(Camera);
        CLASS_NO_COPY_MOVE(Camera);
        COMPONENT_TYPEINFO;
        friend void SceneManager::Initialize();

    public:
        /// camera creation parameters
        struct Params : Object
        {
            OBJECT_TYPEINFO;
            explicit Params(const Math::Vector &backgroundColor) : Object(Type()), mBackgroundColor(backgroundColor) {}
            Math::Vector mBackgroundColor;
        };

        /// get the camera's background color
        [[nodiscard]] const Math::Vector &GetBackgroundColor() const;

        /// set the camera's background color
        void SetBackgroundColor(const Math::Vector &backgroundColor);


    private:
        /// constructs a camera with a background color
        explicit Camera(const GameObjectWeakPtr &gameObject, Math::Vector backgroundColor);

        /// creates a smart pointer version of the camera component
        static ComponentPtr MakePtr(const EngineWeakPtr &engine, const GameObjectWeakPtr &gameObject, const Object &params);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
