#include "Platform/Windows/GraphicsErrors.h"
#include <windows.h>

bool Platform::FormatGraphicsError(std::int32_t error, char* output, std::uint32_t length)
{
    return FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, static_cast<DWORD>(error), 0, output, length, nullptr) != 0;
}
