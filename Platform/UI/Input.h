#pragma once
namespace Platform {
bool SetUITextInput(bool active, bool numeric = false, bool password = false);
void SetConsoleTextInput(bool active);
float MouseWheelScrollLines();
}
