//==============================================================================================================================================================================
/// \file
/// \brief     debug log
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lDebugLog.h"
#include "lEngine.h"

#include <deque>

using namespace Lumen;

/// Lumen Hidden namespace
namespace Lumen::Hidden
{
    /// maximum callback log size
    constexpr size_t MaxCallbackLogSize = 256;

    /// log callback
    DebugLog::LogCallback gLogCallback = nullptr;

    /// keep recent log messages waiting for callback to be set
    std::deque<std::pair<DebugLog::LogLevel, std::string>> gCallbackLog;
}

/// set log callback
void DebugLog::SetCallback(LogCallback callback)
{
    Hidden::gLogCallback = callback;

    // sent stored callback log messages to callback
    for (auto &[level, message] : Hidden::gCallbackLog)
    {
        Hidden::gLogCallback(level, message);
    }
    Hidden::gCallbackLog.clear();
}

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

    std::string fullMessage(prefix);
    fullMessage += std::vformat(format, args);
    Engine::DebugOutput(fullMessage);

    // either callback log message, or store to callback later
    std::string_view message(fullMessage);
    message.remove_prefix(prefix.size());
    if (Hidden::gLogCallback)
    {
        Hidden::gLogCallback(level, message);
    }
    else
    {
        if (Hidden::gCallbackLog.size() >= Hidden::MaxCallbackLogSize)
        {
            Hidden::gCallbackLog.pop_front();
        }
        Hidden::gCallbackLog.emplace_back(level, message);
    }
}
