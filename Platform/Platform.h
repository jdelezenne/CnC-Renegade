#pragma once

#include <cstdint>

struct SDL_Window;
union SDL_Event;

namespace Platform {
using EventHandler = void (*)(const SDL_Event&);

bool Initialize();
void Shutdown();
bool CreateGameWindow(const char* title, int width, int height);
SDL_Window* GetWindow();
void ShowWindow();
void MinimizeWindow();
void ConfigureRendererWindow(int width, int height, bool windowed);
void SetEventHandler(EventHandler handler);
void SetInputEventHandler(EventHandler handler);
void PumpEvents();
std::uint64_t Ticks();
void Sleep(std::uint32_t milliseconds);


// Native renderer, browser, and audio backends use this opaque handle.
void* NativeWindowHandle();
}
