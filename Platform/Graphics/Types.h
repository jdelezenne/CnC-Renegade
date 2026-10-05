#pragma once

#ifdef _WIN32
#include "Platform/Windows/GraphicsTypes.h"
#else
#include <wsl/winadapter.h>
#include <cstdint>

namespace Platform {
using GraphicsWindowHandle = void*;
struct GraphicsPaletteEntry {
    std::uint8_t peRed;
    std::uint8_t peGreen;
    std::uint8_t peBlue;
    std::uint8_t peFlags;
};
}

using HMONITOR = void*;
using HMODULE = void*;
using HFONT = void*;
using LPBYTE = BYTE*;
using PBYTE = BYTE*;
using LPRECT = RECT*;
using LPDWORD = DWORD*;
using LPGUID = GUID*;
struct RGNDATA;
struct LOGFONT;
struct IStream;
struct GLYPHMETRICSFLOAT;
using LPGLYPHMETRICSFLOAT = GLYPHMETRICSFLOAT*;

#define HMONITOR_DECLARED
#define MAKE_HRESULT(severity, facility, code) static_cast<HRESULT>((static_cast<std::uint32_t>(severity) << 31) | (static_cast<std::uint32_t>(facility) << 16) | static_cast<std::uint32_t>(code))
#endif
