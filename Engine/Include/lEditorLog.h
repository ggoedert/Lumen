//==============================================================================================================================================================================
/// \file
/// \brief     Editor Log interface
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
        static EditorLogPtr MakePtr(const std::string &logFilename);

        /// run editor log
        void Run(const char *title);

        /// add message to editor log
        void AddMessage(DebugLog::LogLevel level, std::string_view message);

        /// print status
        void PrintStatus();

        /// return editor log window visibility
        bool Visible();

        /// set editor log window visibility
        void Show(bool visible);

    protected:
        /// constructs editor log
        explicit EditorLog(const std::string &logFilename);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
#endif
