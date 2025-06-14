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
        [[nodiscard]] Type ComponentType() const noexcept;

        /// get component name
        [[nodiscard]] std::string ComponentName() const noexcept;

        /// get owning game object
        [[nodiscard]] GameObjectWeakPtr GetGameObject() const;

    protected:
        /// constructs a component with type, name, and parent. called by derived classes
        Component(Type componentType, const std::string &componentName, const GameObjectWeakPtr &gameObject);

        /// virtual destructor
        virtual ~Component() noexcept override;

    private:
        /// start component
        virtual void Start();

        /// run component
        virtual void Run() = 0;

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };

    /// alias for collection of components
    using Components = std::vector<ComponentWeakPtr>;
}
