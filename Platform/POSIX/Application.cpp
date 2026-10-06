#include "Platform/Application.h"
#include "Platform/Debug.h"
#include <atomic>
#include <cstdlib>
#include <exception>
#include <unistd.h>

namespace {
std::atomic<bool> ExitOnException{false};
std::atomic<bool> ExitingAfterException{false};
}

void Platform::PrepareApplication(bool) {}
void Platform::AttachApplicationWindow() {}
std::uint32_t Platform::ProcessId() { return static_cast<std::uint32_t>(getpid()); }
bool Platform::IsExitingAfterException() { return ExitingAfterException; }
void Platform::SetExitOnException(bool enabled) { ExitOnException = enabled; }

int Platform::RunApplicationLoop(int (*loop)(), bool handleExceptions, void (*callback)(), char* (*version)())
{
    if (!handleExceptions) return loop();
    const auto handle = [&] {
        if (version) DebuggerOutput(version());
        if (callback) callback();
        if (!ExitOnException) throw;
        ExitingAfterException = true;
        return EXIT_FAILURE;
    };
    try { return loop(); }
    catch (const std::exception& error) { DebuggerOutput(error.what()); return handle(); }
    catch (...) { DebuggerOutput("Unhandled application exception"); return handle(); }
}
