#include "Platform/Debug.h"
#include "Platform/Platform.h"
#include <SDL3/SDL.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <system_error>

void Platform::FormatSystemError(int error, char* buffer, int length)
{
    if (!buffer || length <= 0) return;
    const auto message = std::system_category().message(error);
    std::snprintf(buffer, static_cast<std::size_t>(length), "%s", message.c_str());
}
int Platform::LastSystemError() { return errno; }
void Platform::DebugOutput(const char* message) { std::fputs(message, stderr); }
void Platform::BreakDebugger() { SDL_TriggerBreakpoint(); }
void Platform::HandleAssertion(const char* message)
{
    const SDL_MessageBoxButtonData buttons[] = {
        {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Abort"},
        {0, 2, "Retry"},
        {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 3, "Ignore"}
    };
    const SDL_MessageBoxData dialog{SDL_MESSAGEBOX_ERROR, GetWindow(), "WWDebug_Assert_Fail",
        message, 3, buttons, nullptr};
    int selected = 1;
    if (!SDL_ShowMessageBox(&dialog, &selected)) {
        std::fprintf(stderr, "%s\n", message);
        std::abort();
    }
    if (selected == 1) std::abort();
    if (selected == 2) BreakDebugger();
}

void Platform::DebuggerOutput(const char* message) { DebugOutput(message); }
void Platform::ShowErrorDialog(const char* title, const char* message) { SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, GetWindow()); }
