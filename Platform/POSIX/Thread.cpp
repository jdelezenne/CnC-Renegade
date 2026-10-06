#include "Platform/Threads.h"
Platform::ThreadExceptionHandler Platform::DefaultThreadExceptionHandler() { return nullptr; }
void Platform::RunThreadFunction(void (*function)(void*), void* context, ThreadExceptionHandler,
    const char*, std::uint32_t)
{
    function(context);
}
