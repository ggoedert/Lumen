//==============================================================================================================================================================================
/// \file
/// \brief     project source interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(ProjectSource);

    /// ProjectSource class
    class ProjectSource
    {
        CLASS_NO_COPY_MOVE(ProjectSource);

    public:
        /// constructor
        explicit ProjectSource() = default;

        /// virtual destructor
        virtual ~ProjectSource() noexcept = 0;

        /// open file
        int OpenFile(int fileIndex);
    };
}
