#pragma once

#include <cstdint>

namespace Platform {
void PrepareApplication(bool trackMemory);
std::uint32_t ProcessId();
void AttachApplicationWindow();
int RunApplicationLoop(int (*loop)(), bool handleExceptions,
    void (*exceptionCallback)(), char* (*versionCallback)());
void* AcquireApplicationInstance(const char* identifier, bool allowMultiple);
void ReleaseApplicationInstance(void* instance);
bool IsExitingAfterException();
void SetExitOnException(bool enabled);
}
