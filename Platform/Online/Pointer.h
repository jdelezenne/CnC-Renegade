#pragma once
#include "Platform/Online/Types.h"
#include <cassert>
#include <memory>
#include <utility>

namespace Platform {
struct IEnumConnectionPoints;
struct IEnumConnections;
struct OnlineConnectionPoint;
struct OnlineConnectionPointContainer : OnlineInterface {
    virtual HRESULT STDMETHODCALLTYPE EnumConnectionPoints(IEnumConnectionPoints**) = 0;
    virtual HRESULT STDMETHODCALLTYPE FindConnectionPoint(REFIID, OnlineConnectionPoint**) = 0;
};
struct OnlineConnectionPoint : OnlineInterface {
    virtual HRESULT STDMETHODCALLTYPE GetConnectionInterface(IID*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConnectionPointContainer(OnlineConnectionPointContainer**) = 0;
    virtual HRESULT STDMETHODCALLTYPE Advise(OnlineInterface*, DWORD*) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unadvise(DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE EnumConnections(IEnumConnections**) = 0;
};
inline constexpr GUID IID_OnlineConnectionPointContainer = {0xB196B284, 0xBAB4, 0x101A, {0xB6, 0x9C, 0x00, 0xAA, 0x00, 0x34, 0x1D, 0x07}};

inline HRESULT AdviseOnlineEvents(OnlineInterface* source, OnlineInterface* sink, REFIID iid, unsigned long* cookie)
{
    if (!cookie) return E_POINTER;
    *cookie = 0;
    if (!source || !sink) return E_POINTER;
    OnlineConnectionPointContainer* container = nullptr;
    HRESULT result = source->QueryInterface(IID_OnlineConnectionPointContainer, reinterpret_cast<void**>(&container));
    if (FAILED(result)) return result;
    if (!container) return E_NOINTERFACE;
    OnlineConnectionPoint* point = nullptr;
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

inline HRESULT UnadviseOnlineEvents(OnlineInterface* source, REFIID iid, unsigned long cookie)
{
    if (!source) return E_POINTER;
    OnlineConnectionPointContainer* container = nullptr;
    HRESULT result = source->QueryInterface(IID_OnlineConnectionPointContainer, reinterpret_cast<void**>(&container));
    if (FAILED(result)) return result;
    if (!container) return E_NOINTERFACE;
    OnlineConnectionPoint* point = nullptr;
    result = container->FindConnectionPoint(iid, &point);
    container->Release();
    if (FAILED(result)) return result;
    if (!point) return E_NOINTERFACE;
    result = point->Unadvise(static_cast<DWORD>(cookie));
    point->Release();
    return result;
}

template<class T> class OnlinePointer {
    T* pointer = nullptr;
public:
    OnlinePointer() = default;
    OnlinePointer(T* value) : pointer(value) { if (pointer) pointer->AddRef(); }
    OnlinePointer(const OnlinePointer& other) : OnlinePointer(other.pointer) {}
    OnlinePointer(OnlinePointer&& other) noexcept : pointer(std::exchange(other.pointer, nullptr)) {}
    ~OnlinePointer() { Release(); }
    OnlinePointer& operator=(T* value) {
        if (value) value->AddRef();
        Release();
        pointer = value;
        return *this;
    }
    OnlinePointer& operator=(const OnlinePointer& other) { return operator=(other.pointer); }
    OnlinePointer& operator=(OnlinePointer&& other) noexcept {
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
    HRESULT Advise(OnlineInterface* sink, REFIID iid, unsigned long* cookie) const { return AdviseOnlineEvents(pointer, sink, iid, cookie); }
};
}
