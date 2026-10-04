#include "Platform/Platform.h"
#include <SDL3/SDL.h>

void* Platform::NativeWindowHandle()
{
    SDL_Window* window = GetWindow();
    return window ? SDL_GetPointerProperty(SDL_GetWindowProperties(window),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr) : nullptr;
}
