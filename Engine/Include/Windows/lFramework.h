// include file for standard system include files,
// or project specific include files
//

#pragma once

#include <winsdkver.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <sdkddkver.h>

#if (_WIN32_WINNT >= _WIN32_WINNT_WINTHRESHOLD)
// Use the C++ standard templated min/max
#define NOMINMAX
#endif

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <wrl/client.h>
#if (_WIN32_WINNT >= _WIN32_WINNT_WINTHRESHOLD)
#include <wrl/event.h>
#endif
