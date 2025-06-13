//==============================================================================================================================================================================
/// \file
/// \brief     Component interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lGameObject.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Component);
    CLASS_WEAK_PTR_DEF(Component);
    namespace SceneManager { void Run(); }

    /// Component class
    class Component : public Object
    {
        CLASS_NO_DEFAULT_CTOR(Component);
        CLASS_NO_COPY_MOVE(Component);
        friend class GameObject;
        friend void SceneManager::Run();

    public:
        /// get component type
        [[nodiscard]] Type ComponentType() const noexcept { return mComponentType; }

        /// get component name
        [[nodiscard]] std::string ComponentName() const noexcept { return mComponentName; }

        /// get owning game object
        [[nodiscard]] GameObjectWeakPtr GetGameObject() const { return mGameObject; }

    protected:
        /// constructs a component with type, name, and parent. called by derived classes
        Component(Type componentType, const std::string &componentName, const GameObjectWeakPtr &gameObject);

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

        /// owning game object
        GameObjectWeakPtr mGameObject;
    };

    /// alias for collection of components
    using Components = std::vector<ComponentWeakPtr>;
}
