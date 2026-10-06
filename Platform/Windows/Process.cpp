#include "Platform/Process.h"
#include <windows.h>
#include <filesystem>
#include <vector>

namespace {
class GameProcess final : public Platform::Process {
    HANDLE Handle;
    std::uint32_t Identifier;
public:
    GameProcess(HANDLE handle, std::uint32_t id) : Handle(handle), Identifier(id) {}
    ~GameProcess() override { CloseHandle(Handle); }
    std::uint32_t Id() const override { return Identifier; }
    bool Running() override { return WaitForSingleObject(Handle, 0) == WAIT_TIMEOUT; }
    bool Terminate() override { return !Running() || TerminateProcess(Handle, 0) != FALSE; }
};
}
Platform::ProcessPointer Platform::AcquireGameProcess(std::uint32_t id)
{
    if (!id || id == GetCurrentProcessId()) return {};
    HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE | PROCESS_TERMINATE, FALSE, id);
    if (!handle) return {};
    std::vector<wchar_t> name(32768);
    std::vector<wchar_t> currentName(32768);
    DWORD length = static_cast<DWORD>(name.size());
    const auto currentLength = GetModuleFileNameW(nullptr, currentName.data(), static_cast<DWORD>(currentName.size()));
    std::error_code error;
    const bool valid = currentLength && currentLength < currentName.size() && QueryFullProcessImageNameW(handle, 0, name.data(), &length) &&
        std::filesystem::equivalent(std::filesystem::path(std::wstring(name.data(), length)),
            std::filesystem::path(std::wstring(currentName.data(), currentLength)), error) && !error && WaitForSingleObject(handle, 0) == WAIT_TIMEOUT;
    if (!valid) { CloseHandle(handle); return {}; }
    try { return std::make_shared<GameProcess>(handle, id); }
    catch (...) { CloseHandle(handle); throw; }
}
std::uint32_t Platform::WindowProcessId(void* window)
{
    DWORD id = 0;
    if (window) GetWindowThreadProcessId(static_cast<HWND>(window), &id);
    return id;
}
