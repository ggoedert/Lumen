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
    /// Object class
    class Object
    {
    public:
        CLASS_NO_COPY_MOVE(Object);

        /// default constructor
        explicit Object() = default;

        /// pure virtual destructor
        virtual ~Object() noexcept = 0;
    };
}
