//==============================================================================================================================================================================
/// \file
/// \brief     project interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lProjectSource.h"

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

        /// add project source
        bool AddSource(ProjectSourcePtr projectSource);

        /// open file
        int OpenFile(int fileIndex);
    };
}
