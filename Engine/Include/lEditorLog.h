//==============================================================================================================================================================================
/// \file
/// \brief     Editor interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once
#ifdef EDITOR

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(EditorLog);

    /// EditorLog class
    class EditorLog
    {
        CLASS_NO_COPY_MOVE(EditorLog);

    public:
        /// destructor
        ~EditorLog();

        /// creates a smart pointer version of the editor log   
        static EditorLogPtr MakePtr();

        /// add message to editor log
        void AddMessage(DebugLog::LogLevel level, std::string_view message);

        /// run editor log
        void Run(const char *title);

    protected:
        /// constructs editor log
        explicit EditorLog();

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
#endif
