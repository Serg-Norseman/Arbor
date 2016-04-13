#pragma once
#include <sdkddkver.h>

#ifndef WINVER
#define WINVER 0x0A00
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WINTHRESHOLD
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0A00
#endif

#ifndef _WIN32_IE
#define _WIN32_IE _WIN32_IE_WINTHRESHOLD
#endif
