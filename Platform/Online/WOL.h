#pragma once
#include "Platform/Online/Types.h"
#include <cstdint>
#ifdef _WIN32
#include <atlbase.h>
#endif

namespace WOL {
#ifdef _WIN32
#include "Code/wolapi/WOLAPI.h"
#else
#include "Platform/Online/Interfaces.h"
#endif
}
