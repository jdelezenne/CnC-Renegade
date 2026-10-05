#include "Platform/Synchronization.h"
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <new>

void* Platform::CreateMutexHandle(const char* name)
{
    if (name && *name) throw std::invalid_argument("Named interprocess mutexes require a platform implementation");
    return new std::recursive_timed_mutex;
}
void Platform::DestroyMutex(void* mutex) { delete static_cast<std::recursive_timed_mutex*>(mutex); }
bool Platform::LockMutex(void* mutex, int milliseconds)
{
    auto* native = static_cast<std::recursive_timed_mutex*>(mutex);
    if (milliseconds == -1) { native->lock(); return true; }
    if (milliseconds == 0) return native->try_lock();
    if (milliseconds < -1) throw std::invalid_argument("Invalid mutex timeout");
    return native->try_lock_for(std::chrono::milliseconds(milliseconds));
}
void Platform::UnlockMutex(void* mutex) { static_cast<std::recursive_timed_mutex*>(mutex)->unlock(); }
void* Platform::CreateCriticalSection() { return new std::recursive_mutex; }
void Platform::DestroyCriticalSection(void* section) { delete static_cast<std::recursive_mutex*>(section); }
void Platform::LockCriticalSection(void* section) { static_cast<std::recursive_mutex*>(section)->lock(); }
void Platform::UnlockCriticalSection(void* section) { static_cast<std::recursive_mutex*>(section)->unlock(); }
void* Platform::MemoryLogCriticalSection()
{
    alignas(std::recursive_mutex) static unsigned char storage[sizeof(std::recursive_mutex)];
    static auto* section = new (storage) std::recursive_mutex;
    return section;
}
