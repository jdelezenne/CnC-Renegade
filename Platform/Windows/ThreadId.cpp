#include "Platform/Threads.h"
#include <windows.h>

std::uint32_t Platform::CurrentThreadId() { return GetCurrentThreadId(); }
