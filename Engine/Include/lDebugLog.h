//==============================================================================================================================================================================
/// \file
/// \brief     debug log interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include <string>

/// Lumen namespace
namespace Lumen
{
    /// DebugLog namespace
    namespace DebugLog
    {
        /// log error
        void Error(std::string_view error);

        /// log warning
        void Warning(std::string_view warning);

        /// log info
        void Info(std::string_view info);
    };
}
