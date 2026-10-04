#include "directinput.h"
#include "Platform/Platform.h"
#include "Platform/SDL/KeyMapping.h"
#include "timemgr.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cstring>

char DirectInput::DIKeyboardButtons[NUM_KEYBOARD_BUTTONS];
char DirectInput::DIMouseButtons[NUM_MOUSE_BUTTONS];
long DirectInput::DIMouseAxis[NUM_MOUSE_AXIS];
char DirectInput::DIJoystickButtons[NUM_MOUSE_BUTTONS];
float DirectInput::ButtonLastHitTime[NUM_KEYBOARD_BUTTONS];
Vector3 DirectInput::CursorPos(0, 0, 0);
bool DirectInput::EatMouseHeld = false;
bool DirectInput::Captured = false;
int DirectInput::LastKeyPressed = 0;

namespace {
char Held[DirectInput::BUTTON_MAX]{};
char Pending[DirectInput::BUTTON_MAX]{};
float Motion[3]{};
SDL_Joystick* Joystick = nullptr;
bool Ready = false;

void Button(int id, bool down)
{
    if (down && !Held[id]) Pending[id] |= DirectInput::DI_BUTTON_HIT;
    if (!down && Held[id]) Pending[id] |= DirectInput::DI_BUTTON_RELEASED;
    Held[id] = down ? DirectInput::DI_BUTTON_HELD : 0;
}

void Open_Joystick()
{
    if (Joystick) return;
    int count = 0;
    SDL_JoystickID* ids = SDL_GetJoysticks(&count);
    if (count) Joystick = SDL_OpenJoystick(ids[0]);
    SDL_free(ids);
}
}

void DirectInput::Init()
{
    Ready = true;
    Flush();
    std::fill_n(ButtonLastHitTime, NUM_KEYBOARD_BUTTONS, 1.0f);
    Platform::SetInputEventHandler(Process_Event);
    Open_Joystick();
    Acquire();
}

void DirectInput::Shutdown()
{
    Unacquire();
    Platform::SetInputEventHandler(nullptr);
    if (Joystick) SDL_CloseJoystick(Joystick);
    Joystick = nullptr;
    Ready = false;
}

void DirectInput::Flush()
{
    std::memset(Held, 0, sizeof(Held));
    std::memset(Pending, 0, sizeof(Pending));
    std::memset(Motion, 0, sizeof(Motion));
    std::memset(DIKeyboardButtons, 0, sizeof(DIKeyboardButtons));
    std::memset(DIMouseButtons, 0, sizeof(DIMouseButtons));
    std::memset(DIMouseAxis, 0, sizeof(DIMouseAxis));
    std::memset(DIJoystickButtons, 0, sizeof(DIJoystickButtons));
    EatMouseHeld = false;
    LastKeyPressed = 0;
}

void DirectInput::Acquire()
{
    if (!Ready || Captured || !Platform::GetWindow()) return;
    Flush();
    SDL_GetMouseState(&CursorPos.X, &CursorPos.Y);
    SDL_SetWindowRelativeMouseMode(Platform::GetWindow(), true);
    Captured = true;
}

void DirectInput::Unacquire()
{
    if (Captured && Platform::GetWindow()) {
        SDL_SetWindowRelativeMouseMode(Platform::GetWindow(), false);
        SDL_WarpMouseInWindow(Platform::GetWindow(), CursorPos.X, CursorPos.Y);
    }
    Captured = false;
    Flush();
}

void DirectInput::Process_Event(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_JOYSTICK_ADDED) Open_Joystick();
    if (event.type == SDL_EVENT_JOYSTICK_REMOVED && Joystick &&
        event.jdevice.which == SDL_GetJoystickID(Joystick)) {
        SDL_CloseJoystick(Joystick);
        Joystick = nullptr;
        for (int i = 0; i < NUM_JOYSTICK_BUTTONS; ++i) Button(BUTTON_JOYSTICK_FIRST + i, false);
        Open_Joystick();
    }
    if (!Captured) return;
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        int key = Legacy_Key_ID(event.key.scancode);
        if (!key || event.key.repeat) break;
        Button(key, event.key.down);
        if (event.key.down) LastKeyPressed = key;
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        int id = -1;
        if (event.button.button == SDL_BUTTON_LEFT) id = BUTTON_MOUSE_LEFT;
        if (event.button.button == SDL_BUTTON_RIGHT) id = BUTTON_MOUSE_RIGHT;
        if (event.button.button == SDL_BUTTON_MIDDLE) id = BUTTON_MOUSE_CENTER;
        if (id >= 0) Button(id, event.button.down);
        break;
    }
    case SDL_EVENT_MOUSE_MOTION:
        Motion[0] += event.motion.xrel;
        Motion[1] += event.motion.yrel;
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        Motion[2] += event.wheel.y * 120.0f *
            (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1);
        break;
    case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
    case SDL_EVENT_JOYSTICK_BUTTON_UP:
        if (Joystick && event.jbutton.which == SDL_GetJoystickID(Joystick) &&
            event.jbutton.button < NUM_JOYSTICK_BUTTONS)
            Button(BUTTON_JOYSTICK_FIRST + event.jbutton.button, event.jbutton.down);
        break;
    }
}

void DirectInput::Read()
{
    Platform::PumpEvents();
    if (!Captured) return;
    for (int i = 0; i < BUTTON_MAX; ++i) {
        char state = Held[i] | Pending[i];
        if (i < BUTTON_MOUSE_FIRST) DIKeyboardButtons[i] = state;
        else if (i < BUTTON_JOYSTICK_FIRST) DIMouseButtons[i - BUTTON_MOUSE_FIRST] = state;
        else DIJoystickButtons[i - BUTTON_JOYSTICK_FIRST] = state;
        Pending[i] = 0;
    }
    DIKeyboardButtons[DIK_SHIFT] = DIKeyboardButtons[DIK_LSHIFT] | DIKeyboardButtons[DIK_RSHIFT];
    DIKeyboardButtons[DIK_CONTROL] = DIKeyboardButtons[DIK_LCONTROL] | DIKeyboardButtons[DIK_RCONTROL];
    DIKeyboardButtons[DIK_ALT] = DIKeyboardButtons[DIK_LMENU] | DIKeyboardButtons[DIK_RMENU];
    DIKeyboardButtons[DIK_WIN] = DIKeyboardButtons[DIK_LWIN] | DIKeyboardButtons[DIK_RWIN];
    for (int i = 0; i < NUM_MOUSE_AXIS; ++i) {
        DIMouseAxis[i] = static_cast<long>(Motion[i]);
        Motion[i] -= DIMouseAxis[i];
        CursorPos[i] += DIMouseAxis[i] * 2;
    }
    if (!(DIMouseButtons[0] & DI_BUTTON_HELD)) EatMouseHeld = false;
    if (EatMouseHeld) DIMouseButtons[0] &= ~DI_BUTTON_HELD;
    Update_Double_Clicks();
}

void DirectInput::Eat_Mouse_Held_States()
{
    if (DIMouseButtons[0] & (DI_BUTTON_HELD | DI_BUTTON_HIT)) EatMouseHeld = true;
}

long DirectInput::Get_Joystick_Axis_State(JoystickAxis axis)
{
    if (!Captured || !Joystick) return 0;
    long value = static_cast<long>(SDL_GetJoystickAxis(Joystick, axis)) * 1000 / 32767;
    return value > -20 && value < 20 ? 0 : std::clamp(value, -1000L, 1000L);
}

void DirectInput::Update_Double_Clicks()
{
    for (int i = 0; i < NUM_KEYBOARD_BUTTONS; ++i) {
        ButtonLastHitTime[i] += TimeManager::Get_Frame_Real_Seconds();
        if (DIKeyboardButtons[i] & DI_BUTTON_HIT) {
            if (ButtonLastHitTime[i] <= 0.25f) DIKeyboardButtons[i] |= 8;
            ButtonLastHitTime[i] = 0;
        }
    }
}
