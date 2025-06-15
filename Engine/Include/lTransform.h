//==============================================================================================================================================================================
/// \file
/// \brief     Transform interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lMath.h"
#include "lObject.h"

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
        CLASS_PTR_MAKER(Transform);

    public:
        /// constructor
        explicit Transform(const GameObjectWeakPtr &gameObject);

        /// destructor
        ~Transform() noexcept override;

        /// get owning game object
        [[nodiscard]] GameObjectWeakPtr GameObject() const;

        /// get parent
        [[nodiscard]] TransformWeakPtr GetParent() const;

        /// set parent
        void SetParent(TransformWeakPtr parent);

        /// get position
        [[nodiscard]] Math::Vector3 GetPosition() const;

        /// set position
        void SetPosition(const Math::Vector3 &position);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
