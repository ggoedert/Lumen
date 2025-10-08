//==============================================================================================================================================================================
/// \file
/// \brief     Id namespace
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen Id namespace
namespace Lumen::Id
{
    /// id type
    using Type = size_t;

    /// invalid id
    static constexpr Type Invalid = static_cast<Type>(-1);

    /// id generator class
    class Generator
    {
        CLASS_NO_COPY_MOVE(Generator);

    public:
        /// constructor
        explicit inline Generator() noexcept : mCurrent(static_cast<Type>(-1)) {}

        /// get next id and advance
        [[nodiscard]] inline Type Next() noexcept { return ++mCurrent; }

    private:
        /// current id
        Type mCurrent;
    };
}
