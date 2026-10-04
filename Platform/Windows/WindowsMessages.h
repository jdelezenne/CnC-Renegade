#pragma once
#include <windows.h>

// Retained native dialogs, browser and IME run inside SDL's message pump.
void Install_Windows_Message_Hook(bool (*game_handler)(MSG&));
