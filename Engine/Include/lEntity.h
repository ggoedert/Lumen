//==============================================================================================================================================================================
/// \file
/// \brief     entity interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lSerializedData.h"
#include "lObject.h"
#include "lApplication.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_WEAK_PTR_DEF(Engine);
    CLASS_PTR_DEF(Entity);
    CLASS_WEAK_PTR_DEF(Entity);
    CLASS_WEAK_PTR_DEF(Transform);
    CLASS_WEAK_PTR_DEF(Component);
    namespace SceneManager { void Run(); }

    /// Entity class
    class Entity : public Object, public std::enable_shared_from_this<Entity>
    {
        CLASS_NO_COPY_MOVE(Entity);
        OBJECT_TYPEINFO;
        friend void SceneManager::Run();

    public:
        /// destroys entity
        ~Entity() override;

        /// custom smart pointer maker, self registers into scene manager
        static EntityWeakPtr MakePtr(Lumen::Application &application, std::string_view name);

        /// serialize
        void Serialize(Serialized::Type &out, bool packed) const;

        /// deserialize
        void Deserialize(const Serialized::Type &in, bool packed);

        /// get application
        [[nodiscard]] Application &GetApplication();

        /// get name
        [[nodiscard]] std::string_view Name() const;

        /// get transform
        [[nodiscard]] TransformWeakPtr Transform() const;

        /// get component
        [[nodiscard]] ComponentWeakPtr Component(Hash type) const;

        /// add a component
        [[maybe_unused]] ComponentWeakPtr AddComponent(Hash type);

    protected:
        /// run entity
        void Run();

    private:
        /// constructs a entity
        explicit Entity();

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };

    /// alias for collection of entities
    using Entities = std::vector<EntityWeakPtr>;
}
