#include "Platform/Online/Provider.h"

HRESULT Platform::CreateOnlineProvider(REFCLSID, REFIID, void** object)
{
    if (!object) return E_POINTER;
    *object = nullptr;
    return static_cast<HRESULT>(0x80040111u);
}
