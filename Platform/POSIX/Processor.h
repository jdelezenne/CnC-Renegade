#pragma once
#include <x86intrin.h>
#include <cpuid.h>
namespace Platform {
inline std::uint64_t ProcessorTicks() { return __rdtsc(); }
inline void ProcessorRegisters(int (&registers)[4], unsigned leaf) {
    unsigned a, b, c, d;
    __cpuid_count(leaf, 0, a, b, c, d);
    registers[0] = static_cast<int>(a);
    registers[1] = static_cast<int>(b);
    registers[2] = static_cast<int>(c);
    registers[3] = static_cast<int>(d);
}
}
