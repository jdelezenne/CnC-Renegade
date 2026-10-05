#pragma once
#include <intrin.h>
namespace Platform {
inline std::uint64_t ProcessorTicks() { return __rdtsc(); }
inline void ProcessorRegisters(int (&registers)[4], unsigned leaf) { __cpuidex(registers, static_cast<int>(leaf), 0); }
}
