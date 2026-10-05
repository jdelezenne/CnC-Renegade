#include "Platform/Threads.h"
void Platform::RunThreadFunction(void (*function)(void*), void* context, ThreadExceptionHandler,
    const char*, std::uint32_t)
{
    function(context);
}
