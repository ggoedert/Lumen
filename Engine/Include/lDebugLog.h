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
    /// DebugLog class
    class DebugLog
    {
    public:
        /// log error
        static void Error(const std::string &error);

        /// log warning
        static void Warning(const std::string &warning);

        /// log info
        static void Info(const std::string &info);
    };
}
