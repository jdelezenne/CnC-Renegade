#include "Platform/Platform.h"
#include <SDL3/SDL.h>

namespace {
SDL_Window* Window = nullptr;
Platform::EventHandler ApplicationEvents = nullptr;
Platform::EventHandler InputEvents = nullptr;
bool Initialized = false;
bool Pumping = false;
}

bool Platform::Initialize()
{
    if (Initialized) return true;
    Initialized = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    if (Initialized) SDL_DisableScreenSaver();
    return Initialized;
}

void Platform::Shutdown()
{
    InputEvents = ApplicationEvents = nullptr;
    if (Window) SDL_DestroyWindow(Window);
    Window = nullptr;
    SDL_Quit();
    Initialized = false;
}

bool Platform::CreateGameWindow(const char* title, int width, int height)
{
    Window = SDL_CreateWindow(title, width, height, SDL_WINDOW_HIDDEN);
    return Window != nullptr;
}

SDL_Window* Platform::GetWindow() { return Window; }
void Platform::ShowWindow() { if (Window) SDL_ShowWindow(Window); }
void Platform::MinimizeWindow() { if (Window) SDL_MinimizeWindow(Window); }

void Platform::ConfigureRendererWindow(int width, int height, bool windowed)
{
    if (!Window) return;
    // Direct3D 8 still owns the exclusive display mode. SDL owns the window.
    SDL_SetWindowBordered(Window, windowed);
    SDL_SetWindowAlwaysOnTop(Window, !windowed);
    SDL_SetWindowSize(Window, width, height);
    if (windowed) {
        const int center = SDL_WINDOWPOS_CENTERED_DISPLAY(SDL_GetDisplayForWindow(Window));
        SDL_SetWindowPosition(Window, center, center);
    } else {
        SDL_SetWindowPosition(Window, 0, 0);
    }
}

void Platform::SetEventHandler(EventHandler handler) { ApplicationEvents = handler; }
void Platform::SetInputEventHandler(EventHandler handler) { InputEvents = handler; }

void Platform::PumpEvents()
{
    if (!Initialized || Pumping) return;
    Pumping = true;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (InputEvents) InputEvents(event);
        if (ApplicationEvents) ApplicationEvents(event);
    }
    Pumping = false;
}

std::uint64_t Platform::Ticks() { return SDL_GetTicks(); }
void Platform::Sleep(std::uint32_t milliseconds) { SDL_Delay(milliseconds); }
