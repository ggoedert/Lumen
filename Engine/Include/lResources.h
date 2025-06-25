//==============================================================================================================================================================================
/// \file
/// \brief     resources interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lFileSystem.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Resources);

    /// Resources class
    class Resources
    {
        CLASS_NO_COPY_MOVE(Resources);

    public:
        /// constructor
        explicit Resources() = default;

        /// virtual destructor
        ~Resources() noexcept = default;

        /// import file
        void ImportFile();
    };
}
