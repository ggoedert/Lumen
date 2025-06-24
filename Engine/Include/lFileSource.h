//==============================================================================================================================================================================
/// \file
/// \brief     file source interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(FileSource);

    /// FileSource class
    class FileSource
    {
        CLASS_NO_COPY_MOVE(FileSource);

    public:
        /// constructor
        explicit FileSource() = default;

        /// virtual destructor
        virtual ~FileSource() noexcept = 0;

        /// import file
        virtual void ImportFile() = 0;
    };
}
