#include "Platform/Synchronization.h"
#include <windows.h>
#include <new>
#include <system_error>

namespace {
void Check(BOOL result)
{
    if (!result) throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
}
}

void* Platform::CreateMutexHandle(const char* name)
{
    HANDLE mutex = CreateMutexA(nullptr, FALSE, name);
    Check(mutex != nullptr);
    return mutex;
}
void Platform::DestroyMutex(void* mutex) { Check(CloseHandle(mutex)); }
bool Platform::LockMutex(void* mutex, int milliseconds)
{
    const DWORD result = WaitForSingleObject(mutex, milliseconds == -1 ? INFINITE : static_cast<DWORD>(milliseconds));
    if (result == WAIT_FAILED) Check(FALSE);
    return result == WAIT_OBJECT_0;
}
void Platform::UnlockMutex(void* mutex) { Check(ReleaseMutex(mutex)); }
void* Platform::CreateCriticalSection()
{
    auto* section = new CRITICAL_SECTION;
    if (!InitializeCriticalSectionEx(section, 0, 0)) {
        const int error = static_cast<int>(GetLastError());
        delete section;
        throw std::system_error(error, std::system_category());
    }
    return section;
}
void Platform::DestroyCriticalSection(void* section)
{
    auto* native = static_cast<CRITICAL_SECTION*>(section);
    DeleteCriticalSection(native);
    delete native;
}
void Platform::LockCriticalSection(void* section) { EnterCriticalSection(static_cast<CRITICAL_SECTION*>(section)); }
void Platform::UnlockCriticalSection(void* section) { LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(section)); }
void* Platform::MemoryLogCriticalSection()
{
    alignas(CRITICAL_SECTION) static unsigned char storage[sizeof(CRITICAL_SECTION)];
    static auto* section = [] {
        auto* native = new (storage) CRITICAL_SECTION;
        Check(InitializeCriticalSectionEx(native, 0, 0));
        return native;
    }();
    return section;
}
