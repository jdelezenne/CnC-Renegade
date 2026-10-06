#include "Platform/Process.h"
#include <SDL3/SDL_process.h>
#include <SDL3/SDL_properties.h>

namespace {
class ChildProcess final : public Platform::Process {
    SDL_Process* Handle;
    std::uint32_t Identifier;
    bool Exited = false;
public:
    explicit ChildProcess(SDL_Process* process) : Handle(process), Identifier(static_cast<std::uint32_t>(
        SDL_GetNumberProperty(SDL_GetProcessProperties(process), SDL_PROP_PROCESS_PID_NUMBER, 0))) {}
    ~ChildProcess() override { SDL_DestroyProcess(Handle); }
    std::uint32_t Id() const override { return Identifier; }
    bool Running() override {
        int code;
        if (!Exited) Exited = SDL_WaitProcess(Handle, false, &code);
        return !Exited;
    }
    bool Terminate() override { return !Running() || SDL_KillProcess(Handle, true); }
};
}
Platform::ProcessPointer Platform::StartProcess(const std::vector<std::string>& arguments)
{
    if (arguments.empty() || arguments.front().empty()) return {};
    std::vector<const char*> argv;
    for (const auto& argument : arguments) argv.push_back(argument.c_str());
    argv.push_back(nullptr);
    SDL_Process* process = SDL_CreateProcess(argv.data(), false);
    if (!process) return {};
    try { return std::make_shared<ChildProcess>(process); }
    catch (...) { SDL_KillProcess(process, true); SDL_WaitProcess(process, true, nullptr); SDL_DestroyProcess(process); throw; }
}
