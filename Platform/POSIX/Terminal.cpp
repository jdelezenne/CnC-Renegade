#include "Platform/Terminal.h"
#include <cerrno>
#include <cstdio>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

namespace {
termios Original{};
bool Changed = false;
bool Interactive = false;
}
bool Platform::OpenTerminal(std::uint16_t colors)
{
    Interactive = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
    if (Interactive) {
        if (tcgetattr(STDIN_FILENO, &Original) != 0) return false;
        auto mode = Original;
        mode.c_lflag &= ~(ICANON | ECHO);
        mode.c_cc[VMIN] = 1;
        mode.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &mode) != 0) return false;
        Changed = true;
    }
    SetTerminalColors(colors);
    return true;
}
void Platform::CloseTerminal()
{
    if (Changed) tcsetattr(STDIN_FILENO, TCSANOW, &Original);
    if (Interactive) WriteTerminal("\x1b[0m");
    Changed = Interactive = false;
}
void Platform::SetTerminalTitle(const char* title)
{
    if (!Interactive || !title) return;
    WriteTerminal("\x1b]0;");
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(title); *p; ++p)
        if (*p >= 32 && *p != 127) std::fputc(*p, stdout);
    WriteTerminal("\x07");
}
void Platform::RaiseTerminal() {}
void* Platform::FindTerminalWindow(const char*) { return nullptr; }
void Platform::WriteTerminal(const char* text) { if (text) { std::fputs(text, stdout); std::fflush(stdout); } }
bool Platform::TerminalKeyAvailable()
{
    pollfd input{STDIN_FILENO, POLLIN, 0};
    return poll(&input, 1, 0) > 0 && (input.revents & POLLIN);
}
int Platform::ReadTerminalKey(bool echo)
{
    unsigned char key = 0;
    ssize_t result;
    do { result = read(STDIN_FILENO, &key, 1); } while (result < 0 && errno == EINTR);
    if (result != 1) return -1;
    if (key == '\n') key = '\r';
    if (key == 127) key = '\b';
    if (echo && Interactive) { std::fputc(key, stdout); std::fflush(stdout); }
    return key;
}
void Platform::SetTerminalColors(std::uint16_t colors)
{
    if (!Interactive) return;
    const int foreground = ((colors & TerminalRed) ? 1 : 0) | ((colors & TerminalGreen) ? 2 : 0) | ((colors & TerminalBlue) ? 4 : 0);
    const int background = ((colors & TerminalBackgroundRed) ? 1 : 0) | ((colors & TerminalBackgroundGreen) ? 2 : 0) | ((colors & TerminalBackgroundBlue) ? 4 : 0);
    std::fprintf(stdout, "\x1b[%d;%dm", ((colors & TerminalBright) ? 90 : 30) + foreground, 40 + background);
    std::fflush(stdout);
}
Platform::TerminalCursor Platform::SaveTerminalCursor()
{
    if (Interactive) WriteTerminal("\x1b" "7");
    return {0, 0, Interactive};
}
void Platform::RestoreTerminalCursor(TerminalCursor cursor, bool lineStart)
{
    if (!cursor.Valid) return;
    WriteTerminal("\x1b" "8");
    if (lineStart) WriteTerminal("\r");
}
void Platform::ClearTerminalFromCursor(TerminalCursor cursor, int count)
{
    if (cursor.Valid && count > 0) WriteTerminal("\x1b[0J");
}
void Platform::FillTerminalColors(TerminalCursor cursor, int, std::uint16_t colors)
{
    if (cursor.Valid) SetTerminalColors(colors);
}
void Platform::ApplyTerminalColors(int, std::uint16_t colors) { SetTerminalColors(colors); }
