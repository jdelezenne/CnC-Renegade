#include "Platform/Desktop.h"
#include "Platform/Paths.h"
#include <SDL3/SDL_misc.h>
#include <SDL3/SDL_process.h>
#include <SDL3/SDL_properties.h>
#include <filesystem>
#include <string_view>

bool Platform::OpenExternal(const char* target)
{
    if (!target || !*target) return false;
    const std::string_view name(target);
    if (name.find("://") != name.npos || name.starts_with("mailto:")) return SDL_OpenURL(target);
    std::error_code error;
    const auto path = std::filesystem::absolute(ReadPath(target), error);
    if (error) return false;
    if (std::filesystem::exists(path, error)) {
        const auto utf8 = path.generic_u8string();
        std::string uri = utf8.starts_with(u8"//") ? "file:" : utf8.starts_with(u8"/") ? "file://" : "file:///";
        constexpr char digits[] = "0123456789ABCDEF";
        for (const unsigned char byte : utf8) {
            if ((byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z') ||
                (byte >= '0' && byte <= '9') || byte == '-' || byte == '_' || byte == '.' ||
                byte == '~' || byte == '/' || byte == ':') uri += byte;
            else { uri += '%'; uri += digits[byte >> 4]; uri += digits[byte & 15]; }
        }
        return SDL_OpenURL(uri.c_str());
    }
    if (error) return false;
    const char* args[] = {target, nullptr};
    const auto properties = SDL_CreateProperties();
    if (!properties) return false;
    if (!SDL_SetPointerProperty(properties, SDL_PROP_PROCESS_CREATE_ARGS_POINTER, args) ||
        !SDL_SetBooleanProperty(properties, SDL_PROP_PROCESS_CREATE_BACKGROUND_BOOLEAN, true)) {
        SDL_DestroyProperties(properties);
        return false;
    }
    auto* process = SDL_CreateProcessWithProperties(properties);
    SDL_DestroyProperties(properties);
    if (!process) return false;
    SDL_DestroyProcess(process);
    return true;
}
