//==============================================================================================================================================================================
/// \file
/// \brief     game object interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

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
    class GameObject : public Object, public std::enable_shared_from_this<GameObject>
    {
        CLASS_NO_COPY_MOVE(GameObject);
        OBJECT_TRAITS;
        friend void SceneManager::Run();

    public:
        /// custom smart pointer maker, self registers into scene manager
        static GameObjectWeakPtr MakePtr();

        /// destroys game object
        ~GameObject() noexcept override;

        /// get transform
        [[nodiscard]] TransformWeakPtr Transform() const;

        /// get component
        [[nodiscard]] ComponentWeakPtr Component(const HashType type) const;

        /// add a component
        [[nodiscard]] ComponentWeakPtr AddComponent(const HashType type, const std::any &params);

    protected:
        /// run game object
        void Run();

    private:
        /// constructs a game object
        explicit GameObject();

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
