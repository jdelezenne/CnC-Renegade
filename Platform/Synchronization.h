#pragma once

namespace Platform {
void* CreateMutexHandle(const char* name);
void DestroyMutex(void* mutex);
bool LockMutex(void* mutex, int milliseconds);
void UnlockMutex(void* mutex);
void* CreateCriticalSection();
void DestroyCriticalSection(void* section);
void LockCriticalSection(void* section);
void UnlockCriticalSection(void* section);
void* MemoryLogCriticalSection();
}
