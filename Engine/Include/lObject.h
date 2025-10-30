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
    CLASS_PTR_DEF(Object);

    /// Object class
    class Object
    {
        CLASS_NO_DEFAULT_CTOR(Object);
        CLASS_NO_COPY_MOVE(Object);

        /// get type
        [[nodiscard]] HashType Type() const noexcept;

    protected:
        /// constructs an object with type. called by derived classes
        explicit Object(HashType type);

        /// virtual destructor
        virtual ~Object();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
