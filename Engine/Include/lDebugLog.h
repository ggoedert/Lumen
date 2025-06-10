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
        void Error(const std::string &error);

        /// log warning
        void Warning(const std::string &warning);

        /// log info
        void Info(const std::string &info);
    };
}
