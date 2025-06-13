//==============================================================================================================================================================================
/// \file
/// \brief     Object interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

/// Lumen namespace
namespace Lumen
{
    /// Object class
    class Object
    {
    public:
        CLASS_NO_COPY_MOVE(Object);

        /// default constructor
        Object() = default;

        /// virtual destructor
        virtual ~Object() noexcept = default;
    };
}
