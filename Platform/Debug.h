#pragma once

namespace Platform {
void FormatSystemError(int error, char* buffer, int length);
int LastSystemError();
void HandleAssertion(const char* message);
void DebugOutput(const char* message);
void BreakDebugger();
}

namespace Platform {
void DebuggerOutput(const char* message);
void ShowErrorDialog(const char* title, const char* message);
}
