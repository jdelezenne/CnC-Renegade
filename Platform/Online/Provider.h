#pragma once
#include "Platform/Online/Types.h"

namespace Platform {
HRESULT CreateOnlineProvider(REFCLSID provider, REFIID interfaceId, void** object);
}
