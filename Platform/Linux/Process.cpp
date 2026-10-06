#include "Platform/Process.h"
#include "Platform/SystemInfo.h"
#include <cerrno>
#include <filesystem>
#include <limits>
#include <poll.h>
#include <signal.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace {
class GameProcess final : public Platform::Process {
    int Handle;
    std::uint32_t Identifier;
public:
    GameProcess(int handle, std::uint32_t id) : Handle(handle), Identifier(id) {}
    ~GameProcess() override { close(Handle); }
    std::uint32_t Id() const override { return Identifier; }
    bool Running() override {
        pollfd state{Handle, POLLIN, 0};
        int result;
        do { result = poll(&state, 1, 0); } while (result < 0 && errno == EINTR);
        return result == 0;
    }
    bool Terminate() override { return !Running() || syscall(SYS_pidfd_send_signal, Handle, SIGKILL, nullptr, 0) == 0; }
};
}
Platform::ProcessPointer Platform::AcquireGameProcess(std::uint32_t id)
{
    if (!id || id == static_cast<std::uint32_t>(getpid()) || id > static_cast<std::uint32_t>(std::numeric_limits<pid_t>::max())) return {};
    const int handle = static_cast<int>(syscall(SYS_pidfd_open, static_cast<pid_t>(id), 0));
    if (handle < 0) return {};
    std::error_code error;
    const bool valid = std::filesystem::equivalent("/proc/" + std::to_string(id) + "/exe", ExecutablePath(), error);
    pollfd state{handle, POLLIN, 0};
    if (!valid || error || poll(&state, 1, 0) != 0) { close(handle); return {}; }
    try { return std::make_shared<GameProcess>(handle, id); }
    catch (...) { close(handle); throw; }
}
std::uint32_t Platform::WindowProcessId(void*) { return 0; }
