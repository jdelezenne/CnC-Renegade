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

#include "thread.h"
#include "Platform/Threads.h"
#include "Platform/Platform.h"
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_error.h>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <cstdlib>

namespace {
thread_local ThreadClass* CurrentWorker = nullptr;
thread_local int AppliedPriority = 0;
thread_local int ExitCode = 0;
struct ThreadExit { int Code; };
void ApplyPriority(int priority)
{
    if (priority == AppliedPriority) return;
    const auto level = priority < 0 ? SDL_THREAD_PRIORITY_LOW
        : priority > 0 ? SDL_THREAD_PRIORITY_HIGH : SDL_THREAD_PRIORITY_NORMAL;
    if (!SDL_SetCurrentThreadPriority(level))
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to set thread priority: %s", SDL_GetError());
    AppliedPriority = priority;
}
}

ThreadClass::ThreadClass(const char* name, ExceptionHandlerType handler)
    : running(false), ThreadID(0), ExceptionHandler(handler), handle(nullptr), thread_priority(0)
{
    std::snprintf(ThreadName, sizeof(ThreadName), "%s", name ? name : "No name");
}

ThreadClass::~ThreadClass() { Stop(); }

void ThreadClass::Invoke_Thread_Function(void* context)
{
    try {
        static_cast<ThreadClass*>(context)->Thread_Function();
    } catch (const ThreadExit& exit) {
        ExitCode = exit.Code;
    }
}

int ThreadClass::Internal_Thread_Function(void* context)
{
    auto* thread = static_cast<ThreadClass*>(context);
    CurrentWorker = thread;
    ExitCode = 0;
    thread->ThreadID = Platform::CurrentThreadId();
    ApplyPriority(thread->thread_priority.load());
    Platform::RunThreadFunction(&Invoke_Thread_Function, thread, thread->ExceptionHandler,
        thread->ThreadName, thread->ThreadID.load());
    thread->running = false;
    thread->ThreadID = 0;
    CurrentWorker = nullptr;
    return ExitCode;
}

void ThreadClass::Exit_Current_Thread(int status)
{
    if (CurrentWorker) throw ThreadExit{status};
    std::_Exit(status);
}

void ThreadClass::Execute()
{
    if (handle) {
        if (Is_Running()) throw std::logic_error("Thread is already running");
        SDL_WaitThread(handle, nullptr);
        handle = nullptr;
    }
    running = true;
    handle = SDL_CreateThread(&Internal_Thread_Function, ThreadName, this);
    if (!handle) {
        running = false;
        throw std::runtime_error(std::string("Unable to start thread: ") + SDL_GetError());
    }
}

void ThreadClass::Set_Priority(int priority)
{
    thread_priority = priority;
    if (CurrentWorker == this) ApplyPriority(priority);
}

void ThreadClass::Stop(unsigned milliseconds)
{
    running = false;
    if (CurrentWorker == this) throw std::logic_error("A thread cannot join itself");
    if (!handle) return;
    const auto start = Platform::Ticks();
    while (SDL_GetThreadState(handle) != SDL_THREAD_COMPLETE && Platform::Ticks() - start < milliseconds)
        Platform::Sleep(1);
    if (SDL_GetThreadState(handle) != SDL_THREAD_COMPLETE)
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Waiting for thread '%s' to finish after %u ms", ThreadName, milliseconds);
    SDL_WaitThread(handle, nullptr);
    handle = nullptr;
}

void ThreadClass::Sleep_Ms(unsigned milliseconds)
{
    if (CurrentWorker) ApplyPriority(CurrentWorker->thread_priority.load());
    Platform::Sleep(milliseconds);
}

void ThreadClass::Switch_Thread() { Sleep_Ms(1); }
unsigned ThreadClass::_Get_Current_Thread_ID() { return Platform::CurrentThreadId(); }
bool ThreadClass::Is_Running() { return handle && SDL_GetThreadState(handle) != SDL_THREAD_COMPLETE; }
