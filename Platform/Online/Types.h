#pragma once
#ifdef _WIN32
#include "Platform/Windows/TextTypes.h"
#else
#include <wsl/winadapter.h>
#include <cstdint>
inline constexpr unsigned SEVERITY_SUCCESS = 0;
inline constexpr unsigned SEVERITY_ERROR = 1;
inline constexpr unsigned FACILITY_ITF = 4;
#ifndef STDMETHODIMP
#define STDMETHODIMP HRESULT STDMETHODCALLTYPE
#endif
#ifndef MAKE_HRESULT
#define MAKE_HRESULT(severity, facility, code) static_cast<HRESULT>((static_cast<std::uint32_t>(severity) << 31) | (static_cast<std::uint32_t>(facility) << 16) | static_cast<std::uint32_t>(code))
#endif
#endif

namespace Platform {
inline constexpr GUID IID_OnlineInterface = {0, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46}};
struct OnlineInterface {
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID interfaceId, void** object) = 0;
    virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
    virtual ULONG STDMETHODCALLTYPE Release() = 0;
protected:
    ~OnlineInterface() = default;
};
}
