//==============================================================================================================================================================================
/// \file
/// \brief     Transform interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lMath.h"
#include "lObject.h"
#include "lProperty.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Transform);
    CLASS_WEAK_PTR_DEF(Transform);
    CLASS_WEAK_PTR_DEF(GameObject);

    /// Transform class
    class Transform : public Object
    {
        CLASS_NO_DEFAULT_CTOR(Transform);

    public:
        /// creates a smart pointer version of the transform
        static TransformPtr MakePtr(const GameObjectWeakPtr &gameObject);

        /// destructor
        ~Transform() noexcept override;

        /// get owning game object
        [[nodiscard]] GameObjectWeakPtr GameObject() const;

        /// parent property
        PROPERTY(TransformWeakPtr, Parent);

        /// position property
        PROPERTY(Math::Vector3, Position);

    private:
        /// constructor
        explicit Transform(const GameObjectWeakPtr &gameObject);

        /// get parent
        [[nodiscard]] const TransformWeakPtr &GetParent() const;

        /// set parent
        const TransformWeakPtr SetParent(const TransformWeakPtr &parent);

        /// get position
        [[nodiscard]] const Math::Vector3 &GetPosition() const;

        /// set position
        const Math::Vector3 &SetPosition(const Math::Vector3 &position);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
