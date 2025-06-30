//==============================================================================================================================================================================
/// \file
/// \brief     debug log
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lDebugLog.h"
#include "lFramework.h"

using namespace Lumen;

void DebugLog::Error(std::string_view error)
{
    OutputDebugStringA((std::string("[Error] ") + std::string(error) + "\n").c_str());
}

void DebugLog::Warning(std::string_view warning)
{
    OutputDebugStringA((std::string("[Warning] ") + std::string(warning) + "\n").c_str());
}

void DebugLog::Info(std::string_view info)
{
    OutputDebugStringA((std::string("[Info] ") + std::string(info) + "\n").c_str());
}
