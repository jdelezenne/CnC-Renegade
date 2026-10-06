#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Platform {
class Process {
public:
    virtual ~Process() = default;
    virtual std::uint32_t Id() const = 0;
    virtual bool Running() = 0;
    virtual bool Terminate() = 0;
};
using ProcessPointer = std::shared_ptr<Process>;
ProcessPointer StartProcess(const std::vector<std::string>& arguments);
ProcessPointer AcquireGameProcess(std::uint32_t id);
std::uint32_t WindowProcessId(void* window);
}
