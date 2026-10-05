#pragma once
#ifdef _WIN32
#include <atlbase.h>
#else
#include "Platform/Online/Types.h"
#include <cassert>
#include <memory>
#include <utility>

struct IEnumConnectionPoints;
struct IEnumConnections;
struct IConnectionPoint;
struct IConnectionPointContainer : IUnknown {
    virtual HRESULT EnumConnectionPoints(IEnumConnectionPoints**) = 0;
    virtual HRESULT FindConnectionPoint(REFIID, IConnectionPoint**) = 0;
};
struct IConnectionPoint : IUnknown {
    virtual HRESULT GetConnectionInterface(IID*) = 0;
    virtual HRESULT GetConnectionPointContainer(IConnectionPointContainer**) = 0;
    virtual HRESULT Advise(IUnknown*, DWORD*) = 0;
    virtual HRESULT Unadvise(DWORD) = 0;
    virtual HRESULT EnumConnections(IEnumConnections**) = 0;
};
inline constexpr GUID IID_IConnectionPointContainer = {0xB196B284, 0xBAB4, 0x101A, {0xB6, 0x9C, 0x00, 0xAA, 0x00, 0x34, 0x1D, 0x07}};

inline HRESULT AtlAdvise(IUnknown* source, IUnknown* sink, REFIID iid, unsigned long* cookie)
{
    if (!cookie) return E_POINTER;
    *cookie = 0;
    if (!source || !sink) return E_POINTER;
    IConnectionPointContainer* container = nullptr;
    HRESULT result = source->QueryInterface(IID_IConnectionPointContainer, reinterpret_cast<void**>(&container));
    if (FAILED(result)) return result;
    if (!container) return E_NOINTERFACE;
    IConnectionPoint* point = nullptr;
    result = container->FindConnectionPoint(iid, &point);
    container->Release();
    if (FAILED(result)) return result;
    if (!point) return E_NOINTERFACE;
    DWORD value = 0;
    result = point->Advise(sink, &value);
    point->Release();
    if (SUCCEEDED(result)) *cookie = value;
    return result;
}

inline HRESULT AtlUnadvise(IUnknown* source, REFIID iid, unsigned long cookie)
{
    if (!source) return E_POINTER;
    IConnectionPointContainer* container = nullptr;
    HRESULT result = source->QueryInterface(IID_IConnectionPointContainer, reinterpret_cast<void**>(&container));
    if (FAILED(result)) return result;
    if (!container) return E_NOINTERFACE;
    IConnectionPoint* point = nullptr;
    result = container->FindConnectionPoint(iid, &point);
    container->Release();
    if (FAILED(result)) return result;
    if (!point) return E_NOINTERFACE;
    result = point->Unadvise(static_cast<DWORD>(cookie));
    point->Release();
    return result;
}

template<class T> class CComPtr {
    T* pointer = nullptr;
public:
    CComPtr() = default;
    CComPtr(T* value) : pointer(value) { if (pointer) pointer->AddRef(); }
    CComPtr(const CComPtr& other) : CComPtr(other.pointer) {}
    CComPtr(CComPtr&& other) noexcept : pointer(std::exchange(other.pointer, nullptr)) {}
    ~CComPtr() { Release(); }
    CComPtr& operator=(T* value) {
        if (value) value->AddRef();
        Release();
        pointer = value;
        return *this;
    }
    CComPtr& operator=(const CComPtr& other) { return operator=(other.pointer); }
    CComPtr& operator=(CComPtr&& other) noexcept {
        if (this != std::addressof(other)) {
            Release();
            pointer = std::exchange(other.pointer, nullptr);
        }
        return *this;
    }
    operator T*() const { return pointer; }
    T* operator->() const { assert(pointer); return pointer; }
    T** operator&() { assert(!pointer); return &pointer; }
    void Release() { if (auto* value = std::exchange(pointer, nullptr)) value->Release(); }
    void Attach(T* value) { Release(); pointer = value; }
    T* Detach() { return std::exchange(pointer, nullptr); }
    HRESULT Advise(IUnknown* sink, REFIID iid, unsigned long* cookie) const { return AtlAdvise(pointer, sink, iid, cookie); }
};
#endif
