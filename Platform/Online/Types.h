#pragma once
#ifdef _WIN32
#include "Platform/Windows/TextTypes.h"
#include <unknwn.h>
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
