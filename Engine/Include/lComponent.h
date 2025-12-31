//==============================================================================================================================================================================
/// \file
/// \brief     Component interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lEntity.h"

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
        friend class Entity;
        friend void SceneManager::Run();

    public:
        /// serialize
        virtual void Serialize(Serialized::Type &out, bool packed) const = 0;

        /// deserialize
        virtual void Deserialize(const Serialized::Type &in, bool packed) = 0;

        /// initialize component
        virtual void Initialize() {}

        /// get name
        [[nodiscard]] std::string_view Name() const;

        /// get owning entity
        [[nodiscard]] EntityWeakPtr Entity() const;

    protected:
        /// constructs a component with type, name, and parent. called by derived classes
        explicit Component(HashType type, std::string_view name, const EntityWeakPtr &entity);

        /// destroys component
        ~Component() override;

    private:
        /// start component
        virtual void Start() {};

        /// run component
        virtual void Run() {};

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };

    /// alias for collection of components
    using Components = std::vector<ComponentWeakPtr>;
}
