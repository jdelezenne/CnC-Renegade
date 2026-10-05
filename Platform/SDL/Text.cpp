#include "Platform/Text.h"
#include <SDL3/SDL_stdinc.h>

bool Platform::NarrowFromWide(const wchar_t* source, std::string& result, bool& unmapped)
{
    char* converted = SDL_iconv_wchar_utf8(source);
    if (!converted) return false;
    result.assign(converted);
    SDL_free(converted);
    unmapped = false;
    return true;
}

bool Platform::WideFromNarrow(const char* source, std::wstring& result)
{
    char* converted = SDL_iconv_string("WCHAR_T", "UTF-8", source, std::strlen(source) + 1);
    if (!converted) return false;
    result.assign(reinterpret_cast<const wchar_t*>(converted));
    SDL_free(converted);
    return true;
}
