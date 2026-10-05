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
