//==============================================================================================================================================================================
/// \file
/// \brief     Object interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEFS(GameObject);

    /// Object class
    class Object
    {
        CLASS_NO_COPY_MOVE(Object);

    public:
        /// virtual destructor
        virtual ~Object() noexcept = default;

        /// get parent game object
        [[nodiscard]] GameObject *GetGameObject() { return mParent; }

    protected:
        /// constructs a object without parent
        Object() noexcept = default;

        /// constructs a object with parent
        Object(GameObject *parent) : mParent(parent) {}

        /// set parent game object
        [[nodiscard]] void SetGameObject(GameObject *parent) { mParent = parent; }

    private:
        /// parent game object
        GameObject *mParent = nullptr;
    };
}
