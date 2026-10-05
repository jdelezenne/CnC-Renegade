#pragma once
#include <strings.h>
#include <cstdio>
#include <cwchar>
#include <cstdarg>
#include <alloca.h>
#include <cctype>
namespace Platform {
inline char* PosixStrlwr(char* text) {
    for (char* current = text; *current; ++current) *current = static_cast<char>(std::tolower(static_cast<unsigned char>(*current)));
    return text;
}
inline char* PosixStrupr(char* text) {
    for (char* current = text; *current; ++current) *current = static_cast<char>(std::toupper(static_cast<unsigned char>(*current)));
    return text;
}
inline int PosixVsnprintf(char* buffer, std::size_t count, const char* format, va_list arguments) {
    const int result = std::vsnprintf(buffer, count, format, arguments);
    return result >= 0 && static_cast<std::size_t>(result) >= count ? -1 : result;
}
}
#define _vsnprintf Platform::PosixVsnprintf
#define _alloca alloca
#define strcmpi strcasecmp
#define _vsnwprintf std::vswprintf
#ifndef _stdcall
#define _stdcall
#endif
#ifndef _cdecl
#define _cdecl
#endif
#define wcsicmp wcscasecmp
#define _wcsicmp wcscasecmp
#define wcsnicmp wcsncasecmp
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define _strlwr Platform::PosixStrlwr
#define strlwr Platform::PosixStrlwr
#define _strupr Platform::PosixStrupr
#define strupr Platform::PosixStrupr
