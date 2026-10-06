#include "cpudetect.h"
#include "Platform/SystemInfo.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <limits>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <unistd.h>
#include <cerrno>
#include <vector>
#include <fstream>
#include <sstream>

std::string Platform::ProcessStartTimeString()
{
    std::ifstream process("/proc/self/stat");
    std::string record;
    if (!std::getline(process, record)) return {};
    const auto nameEnd = record.rfind(')');
    if (nameEnd == std::string::npos) return {};
    std::istringstream fields(record.substr(nameEnd + 1));
    std::string field;
    for (int number = 3; number < 22; ++number) if (!(fields >> field)) return {};
    std::uint64_t startTicks;
    if (!(fields >> startTicks)) return {};
    const auto frequency = sysconf(_SC_CLK_TCK);
    if (frequency <= 0) return {};
    std::ifstream system("/proc/stat");
    std::uint64_t bootSeconds = 0;
    bool found = false;
    while (std::getline(system, record)) {
        std::istringstream entry(record);
        if ((entry >> field) && field == "btime") {
            found = static_cast<bool>(entry >> bootSeconds);
            break;
        }
    }
    if (!found || bootSeconds > static_cast<std::uint64_t>(std::numeric_limits<std::time_t>::max()) ||
        startTicks / frequency > static_cast<std::uint64_t>(std::numeric_limits<std::time_t>::max()) - bootSeconds) return {};
    const auto epoch = static_cast<std::time_t>(bootSeconds + startTicks / frequency);
    std::tm time{};
    if (!localtime_r(&epoch, &time)) return {};
    char result[256]{};
    return std::strftime(result, sizeof(result), "%x - %H:%M:%S", &time) ? std::string(result) : std::string();
}

std::string Platform::ExecutablePath()
{
    std::vector<char> path(512);
    for (;;) {
        const auto size = readlink("/proc/self/exe", path.data(), path.size());
        if (size < 0) return {};
        if (static_cast<std::size_t>(size) < path.size()) return std::string(path.data(), size);
        path.resize(path.size() * 2);
    }
}

std::string Platform::ComputerName()
{
    struct utsname info{};
    return uname(&info) == 0 ? std::string(info.nodename) : std::string();
}

std::string Platform::UserName()
{
    const auto size = sysconf(_SC_GETPW_R_SIZE_MAX);
    std::vector<char> buffer(size > 0 ? static_cast<std::size_t>(size) : 1024);
    struct passwd record{};
    struct passwd* result = nullptr;
    int error;
    while ((error = getpwuid_r(geteuid(), &record, buffer.data(), buffer.size(), &result)) == ERANGE)
        buffer.resize(buffer.size() * 2);
    return error == 0 && result ? std::string(result->pw_name) : std::string();
}

const char* Platform::OperatingSystemName(unsigned) { return "Linux"; }

void CPUDetectClass::Init_Memory()
{
    struct sysinfo memory{};
    if (sysinfo(&memory) != 0) return;
    const auto bytes = [unit = memory.mem_unit](std::uint64_t count) {
        return static_cast<unsigned>(std::min<std::uint64_t>(count * unit, std::numeric_limits<unsigned>::max()));
    };
    TotalPhysicalMemory = bytes(memory.totalram);
    AvailablePhysicalMemory = bytes(memory.freeram + memory.bufferram);
    TotalPageMemory = bytes(memory.totalram + memory.totalswap);
    AvailablePageMemory = bytes(memory.freeram + memory.bufferram + memory.freeswap);
    struct rlimit limit{};
    const auto maximum = std::numeric_limits<unsigned>::max();
    TotalVirtualMemory = getrlimit(RLIMIT_AS, &limit) == 0 && limit.rlim_cur != RLIM_INFINITY
        ? static_cast<unsigned>(std::min<std::uint64_t>(limit.rlim_cur, maximum)) : maximum;
    AvailableVirtualMemory = TotalVirtualMemory;
}

void CPUDetectClass::Init_OS()
{
    struct utsname info{};
    if (uname(&info) != 0) return;
    std::sscanf(info.release, "%u.%u.%u", &OSVersionNumberMajor, &OSVersionNumberMinor, &OSVersionBuildNumber);
    OSVersionExtraInfo = info.release;
}

void CPUDetectClass::Init_Compact_Log()
{
    tzset();
    CompactLog.Format("%ld\tLinux\t%u\t%s\t%d\t%u\t%x\t%x\t", timezone / 60,
        OSVersionBuildNumber, Get_Processor_Manufacturer_Name(), Get_Processor_Speed(),
        Get_Total_Physical_Memory() / (1024 * 1024) + 1, Get_Feature_Bits(), Get_Extended_Feature_Bits());
}
