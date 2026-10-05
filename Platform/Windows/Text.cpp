#include "Platform/Text.h"

bool Platform::NarrowFromWide(const wchar_t* source, std::string& result, bool& unmapped)
{
    BOOL substituted = FALSE;
    const int length = WideCharToMultiByte(CP_ACP, 0, source, -1, nullptr, 0, nullptr, &substituted);
    if (length <= 0) return false;
    result.resize(length);
    if (!WideCharToMultiByte(CP_ACP, 0, source, -1, result.data(), length, nullptr, &substituted)) return false;
    result.resize(length - 1);
    unmapped = substituted != FALSE;
    return true;
}

bool Platform::WideFromNarrow(const char* source, std::wstring& result)
{
    const int length = MultiByteToWideChar(CP_ACP, 0, source, -1, nullptr, 0);
    if (length <= 0) return false;
    result.resize(length);
    if (!MultiByteToWideChar(CP_ACP, 0, source, -1, result.data(), length)) return false;
    result.resize(length - 1);
    return true;
}
