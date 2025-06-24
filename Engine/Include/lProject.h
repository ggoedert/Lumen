//==============================================================================================================================================================================
/// \file
/// \brief     project interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lFileSource.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Project);

    /// Project class
    class Project
    {
        CLASS_NO_COPY_MOVE(Project);

    public:
        /// constructor
        explicit Project() = default;

        /// virtual destructor
        ~Project() noexcept = default;

        /// add file source
        bool AddFileSource(FileSourcePtr fileSource);

        /// import file
        void ImportFile();
    };
}
