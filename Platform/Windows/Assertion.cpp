#include "Platform/Debug.h"
#include "Code/wwlib/Except.h"
#include <windows.h>
#include <process.h>
#include <csignal>

void Platform::HandleAssertion(const char* message)
{
    if (Is_Trying_To_Exit()) ExitProcess(0);
    const int code = MessageBoxA(nullptr, message, "WWDebug_Assert_Fail",
        MB_ABORTRETRYIGNORE | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);
    if (code == IDABORT) { raise(SIGABRT); _exit(3); }
    if (code == IDRETRY) BreakDebugger();
}
