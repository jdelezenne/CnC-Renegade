/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Platform/Application.h"
#include "Platform/Platform.h"
#include "win.h"
#include "except.h"
#include <SDL3/SDL.h>
#include <cstdlib>
#ifdef _DEBUG
#include <crtdbg.h>
#endif

extern "C" {
HWND hWndMain;
bool WIN_fullscreen = true;
}

void Platform::PrepareApplication(bool trackMemory)
{
    ProgramInstance = GetModuleHandle(nullptr);
#ifdef _DEBUG
    if (trackMemory) {
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    }
#endif
}

void Platform::AttachApplicationWindow()
{
    MainWindow = static_cast<HWND>(NativeWindowHandle());
}

std::uint32_t Platform::ProcessId() { return GetCurrentProcessId(); }
bool Platform::IsExitingAfterException() { return Is_Trying_To_Exit(); }
void Platform::SetExitOnException(bool enabled) { Set_Exit_On_Exception(enabled); }

int Platform::RunApplicationLoop(int (*loop)(), bool handleExceptions,
    void (*exceptionCallback)(), char* (*versionCallback)())
{
    const auto identifier = GetCurrentThreadId();
    Register_Thread_ID(identifier, "Main Thread", true);
    int exitCode = EXIT_SUCCESS;
    if (handleExceptions) {
        Register_Application_Exception_Callback(exceptionCallback);
        Register_Application_Version_Callback(versionCallback);
        __try { exitCode = loop(); }
        __except(Exception_Handler(GetExceptionCode(), GetExceptionInformation())) {}
    } else {
        exitCode = loop();
    }
    Unregister_Thread_ID(identifier, "Main Thread");
    return exitCode;
}
