#pragma once
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cwchar>
#ifdef _WIN32
#include "Platform/Windows/TextTypes.h"
#else
#include <cstring>
#include <strings.h>
#include "Platform/POSIX/CRT.h"
using TCHAR = char;
using WCHAR = wchar_t;
#define _tcslen std::strlen
#define _tcsclen std::strlen
#define _tcscmp std::strcmp
#define _tcsicmp strcasecmp
#define _tcscpy std::strcpy
#define _wcsicmp wcscasecmp
#endif
namespace Platform {
bool NarrowFromWide(const wchar_t* source, std::string& result, bool& unmapped);
bool WideFromNarrow(const char* source, std::wstring& result);
}
