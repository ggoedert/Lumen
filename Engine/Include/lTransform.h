//==============================================================================================================================================================================
/// \file
/// \brief     Transform interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lObject.h"
#include "lMath.h"

/// Lumen namespace
namespace Lumen
{
    class GameObject;
    CLASS_PTR_DEFS(Transform);

    /// Transform class
    class Transform : public Object
    {
        CLASS_NO_DEFAULT_CTOR(Transform);
        CLASS_NO_COPY_MOVE(Transform);

    public:
        /// constructor
        Transform(GameObject *parent) : Object(parent) {}

        /// position
        Math::Vector3 mPosition;
    };
}
