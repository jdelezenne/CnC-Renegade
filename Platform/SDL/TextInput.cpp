#include "Platform/UI/Input.h"
#include "Platform/Platform.h"
#include <SDL3/SDL.h>
namespace {
bool UIActive = false;
bool Numeric = false;
bool Password = false;
bool ConsoleActive = false;
bool Apply() {
    auto* window = Platform::GetWindow();
    if (!window) return false;
    if (!ConsoleActive && !UIActive) return SDL_StopTextInput(window);
    const auto properties = SDL_CreateProperties();
    if (!properties) return false;
    const auto type = ConsoleActive ? SDL_TEXTINPUT_TYPE_TEXT : Numeric ? (Password ? SDL_TEXTINPUT_TYPE_NUMBER_PASSWORD_HIDDEN : SDL_TEXTINPUT_TYPE_NUMBER) : (Password ? SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_HIDDEN : SDL_TEXTINPUT_TYPE_TEXT);
    SDL_SetNumberProperty(properties, SDL_PROP_TEXTINPUT_TYPE_NUMBER, type);
    SDL_SetNumberProperty(properties, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_NONE);
    SDL_SetBooleanProperty(properties, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN, false);
    SDL_SetBooleanProperty(properties, SDL_PROP_TEXTINPUT_MULTILINE_BOOLEAN, false);
    const bool started = SDL_StartTextInputWithProperties(window, properties);
    SDL_DestroyProperties(properties);
    return started;
}
}
bool Platform::SetUITextInput(bool active, bool numeric, bool password) {
    UIActive = active;
    Numeric = numeric;
    Password = password;
    return Apply();
}
void Platform::SetConsoleTextInput(bool active) { ConsoleActive = active; if (!Apply() && Platform::GetWindow()) SDL_LogError(SDL_LOG_CATEGORY_INPUT, "%s", SDL_GetError()); }
