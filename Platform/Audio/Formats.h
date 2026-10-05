#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct WAVEFORMAT {
    std::uint16_t wFormatTag;
    std::uint16_t nChannels;
    std::uint32_t nSamplesPerSec;
    std::uint32_t nAvgBytesPerSec;
    std::uint16_t nBlockAlign;
};
#pragma pack(pop)

using LPWAVEFORMAT = WAVEFORMAT*;
using LPHWAVEOUT = void**;
