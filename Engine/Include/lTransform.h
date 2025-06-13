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
        Transform(const GameObjectWeakPtr &gameObject) : Object(), mGameObject(gameObject) {}

        /// get owning game object
        [[nodiscard]] GameObjectWeakPtr GetGameObject() const { return mGameObject; }

        /// set parent
        void SetParent(Transform *parent)
        {
            LUMEN_ASSERT(parent != this);
            mParent = parent;
        }

        /// get parent
        [[nodiscard]] Transform *GetParent() const { return mParent; }

        /// set position
        void SetPosition(const Math::Vector3 &position) { mPosition = position; }

        /// get position
        [[nodiscard]] Math::Vector3 GetPosition() const { return mPosition; }

    private:
        /// owning game object
        GameObjectWeakPtr mGameObject;

        /// parent transform
        Transform *mParent = nullptr;

        /// position
        Math::Vector3 mPosition;
    };
}
