//==============================================================================================================================================================================
/// \file
/// \brief     game object interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lObject.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(GameObject);
    CLASS_WEAK_PTR_DEF(GameObject);
    CLASS_WEAK_PTR_DEF(Transform);
    CLASS_WEAK_PTR_DEF(Component);
    namespace SceneManager { void Run(); }

    /// GameObject class
    class GameObject : public Object
    {
        CLASS_NO_COPY_MOVE(GameObject);
        friend void SceneManager::Run();

    public:
        /// custom smart pointer maker, self registers into scene manager
        static GameObjectWeakPtr MakePtr();

        /// destroys game object
        ~GameObject();

        /// get transform
        [[nodiscard]] const Transform &GetTransform() const;

        /// get component
        [[nodiscard]] ComponentWeakPtr GetComponent(Type componentType) const;

        /// add a component
        void AddComponent(const ComponentWeakPtr &component);

    protected:
        /// constructs a game object
        GameObject();

        /// run game object
        void Run();

    private:
        CLASS_PIMPL_DEF(Impl);
    };
}
