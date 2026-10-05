#pragma once

#include <cstdint>

namespace Platform {
std::uint32_t CurrentThreadId();
}

struct _EXCEPTION_POINTERS;
namespace Platform {
using ThreadExceptionHandler = int (*)(int, _EXCEPTION_POINTERS*);
void RunThreadFunction(void (*function)(void*), void* context, ThreadExceptionHandler handler,
    const char* name, std::uint32_t identifier);
}
