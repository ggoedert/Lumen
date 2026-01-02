//==============================================================================================================================================================================
/// \file
/// \brief     debug log interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include <functional>
#include <format>

/// Lumen DebugLog namespace
namespace Lumen::DebugLog
{
    /// log types
    enum class LogLevel { None, Detail, Info, Warning, Error, Count };
    using LogCallback = std::function<void(LogLevel level, std::string_view message)>;

    /// set log callback
    void SetCallback(LogCallback callback);

    /// log implementation
    void LogImpl(LogLevel level, std::string_view format, std::format_args args);

    /// log error
    template <typename... Args>
    void Error(std::string_view format, Args&&...args) { LogImpl(LogLevel::Error, format, std::make_format_args<std::format_context>(args...)); }

    /// log warning
    template <typename... Args>
    void Warning(std::string_view format, Args&&...args) { LogImpl(LogLevel::Warning, format, std::make_format_args<std::format_context>(args...)); }

    /// log info
    template <typename... Args>
    void Info(std::string_view format, Args&&...args) { LogImpl(LogLevel::Info, format, std::make_format_args<std::format_context>(args...)); }

    /// log detail
    template <typename... Args>
    void Detail(std::string_view format, Args&&...args) { LogImpl(LogLevel::Detail, format, std::make_format_args<std::format_context>(args...)); }
}
