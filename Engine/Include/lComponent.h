//==============================================================================================================================================================================
/// \file
/// \brief     Component interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lObject.h"

#include <string>

/// Lumen namespace
namespace Lumen
{
    /// Component class
    class Component : public Object
    {
        friend class SceneManager;
        friend class GameObject;
        CLASS_NO_DEFAULT_CTOR(Component);
        CLASS_NO_COPY_MOVE(Component);

    public:
        /// get component type
        [[nodiscard]] Type ComponentType() const noexcept { return mComponentType; }

        /// get component name
        [[nodiscard]] const std::string &ComponentName() const noexcept { return mComponentName; }

    protected:
        /// constructs a component with type, name, and parent. called by derived classes
        Component(Type componentType, const std::string &componentName, GameObjectPtr parent);

        /// virtual destructor
        virtual ~Component() noexcept override = default;

    private:
        /// start component
        virtual void Start() {}

        /// run component
        virtual void Run() = 0;

        /// component type
        const Type mComponentType;

        /// component name
        const std::string &mComponentName;
    };
}
