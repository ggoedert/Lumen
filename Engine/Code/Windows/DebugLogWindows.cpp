//==============================================================================================================================================================================
/// \file
/// \brief     debug log
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lDebugLog.h"
#include "lFramework.h"

using namespace Lumen;

void DebugLog::LogImpl(LogLevel level, std::string_view format, std::format_args args)
{
    std::string_view prefix = "[Log] ";
    switch (level)
    {
    case LogLevel::Error:
        prefix = "[Error] ";
        break;
    case LogLevel::Warning:
        prefix = "[Warning] ";
        break;
    case LogLevel::Info:
        prefix = "[Info] ";
        break;
    case LogLevel::Detail:
        prefix = "[Detail] ";
        break;
    }

    std::string message;
    message.reserve(256);
    message += prefix;
    message += std::vformat(format, args);
    message += '\n';

    OutputDebugStringA(message.c_str());
}
