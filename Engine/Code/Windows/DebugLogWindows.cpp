//==============================================================================================================================================================================
/// \file
/// \brief     debug log
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lDebugLog.h"

#include "Framework.h"

using namespace Lumen;

void DebugLog::Error(const std::string &error)
{
    OutputDebugStringA((std::string("[Error] ") + error + "\n").c_str());
}

void DebugLog::Warning(const std::string &warning)
{
    OutputDebugStringA((std::string("[Warning] ") + warning + "\n").c_str());
}

void DebugLog::Info(const std::string &info)
{
    OutputDebugStringA((std::string("[Info] ") + info + "\n").c_str());
}
