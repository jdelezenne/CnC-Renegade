#include "Platform/Terminal.h"
#include <windows.h>
#include <conio.h>

namespace {
HANDLE Output = INVALID_HANDLE_VALUE;
bool Owned = false;
}
bool Platform::OpenTerminal(std::uint16_t colors)
{
    Owned = AllocConsole() != FALSE;
    Output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!Output || Output == INVALID_HANDLE_VALUE) {
        CloseTerminal();
        return false;
    }
    SetConsoleScreenBufferSize(Output, {80, 4192});
    DWORD written = 0;
    FillConsoleOutputAttribute(Output, colors, 4192 * 50, {0, 0}, &written);
    SetTerminalColors(colors);
    return true;
}
void Platform::CloseTerminal()
{
    if (Owned) FreeConsole();
    Owned = false;
    Output = INVALID_HANDLE_VALUE;
}
void Platform::SetTerminalTitle(const char* title) { SetConsoleTitleA(title); }
void Platform::RaiseTerminal() { SetForegroundWindow(GetConsoleWindow()); }
void* Platform::FindTerminalWindow(const char* title) { return FindWindowA("ConsoleWindowClass", title); }
void Platform::WriteTerminal(const char* text) { _cprintf("%s", text); }
bool Platform::TerminalKeyAvailable() { return _kbhit() != 0; }
int Platform::ReadTerminalKey(bool echo) { return echo ? _getche() : _getch(); }
void Platform::SetTerminalColors(std::uint16_t colors) { SetConsoleTextAttribute(Output, colors); }
Platform::TerminalCursor Platform::SaveTerminalCursor()
{
    CONSOLE_SCREEN_BUFFER_INFO info{};
    const bool valid = GetConsoleScreenBufferInfo(Output, &info) != FALSE;
    return {info.dwCursorPosition.X, info.dwCursorPosition.Y, valid};
}
void Platform::RestoreTerminalCursor(TerminalCursor cursor, bool lineStart)
{
    if (cursor.Valid) SetConsoleCursorPosition(Output, {static_cast<SHORT>(lineStart ? 0 : cursor.X), static_cast<SHORT>(cursor.Y)});
}
void Platform::ClearTerminalFromCursor(TerminalCursor cursor, int count)
{
    if (!cursor.Valid || count <= 0) return;
    DWORD written = 0;
    FillConsoleOutputCharacterA(Output, ' ', count, {static_cast<SHORT>(cursor.X), static_cast<SHORT>(cursor.Y)}, &written);
}
void Platform::FillTerminalColors(TerminalCursor cursor, int count, std::uint16_t colors)
{
    if (!cursor.Valid || count <= 0) return;
    DWORD written = 0;
    FillConsoleOutputAttribute(Output, colors, count, {static_cast<SHORT>(cursor.X), static_cast<SHORT>(cursor.Y)}, &written);
}
void Platform::ApplyTerminalColors(int count, std::uint16_t colors) { FillTerminalColors(SaveTerminalCursor(), count, colors); }
