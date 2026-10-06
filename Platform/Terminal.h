#pragma once
#include <cstdint>

namespace Platform {
inline constexpr std::uint16_t TerminalBlue = 1;
inline constexpr std::uint16_t TerminalGreen = 2;
inline constexpr std::uint16_t TerminalRed = 4;
inline constexpr std::uint16_t TerminalBright = 8;
inline constexpr std::uint16_t TerminalBackgroundBlue = 16;
inline constexpr std::uint16_t TerminalBackgroundGreen = 32;
inline constexpr std::uint16_t TerminalBackgroundRed = 64;
struct TerminalCursor { int X = 0; int Y = 0; bool Valid = false; };
bool OpenTerminal(std::uint16_t colors);
void CloseTerminal();
void SetTerminalTitle(const char* title);
void RaiseTerminal();
void* FindTerminalWindow(const char* title);
void WriteTerminal(const char* text);
bool TerminalKeyAvailable();
int ReadTerminalKey(bool echo);
void SetTerminalColors(std::uint16_t colors);
TerminalCursor SaveTerminalCursor();
void RestoreTerminalCursor(TerminalCursor cursor, bool lineStart = false);
void ClearTerminalFromCursor(TerminalCursor cursor, int count);
void FillTerminalColors(TerminalCursor cursor, int count, std::uint16_t colors);
void ApplyTerminalColors(int count, std::uint16_t colors);
}
