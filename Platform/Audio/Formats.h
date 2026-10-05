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
struct PCMWAVEFORMAT {
    WAVEFORMAT wf;
    std::uint16_t wBitsPerSample;
};
#pragma pack(pop)

inline constexpr std::uint16_t WAVE_FORMAT_PCM = 1;
inline constexpr std::uint16_t WAVE_FORMAT_IMA_ADPCM = 0x11;

using LPWAVEFORMAT = WAVEFORMAT*;
using LPHWAVEOUT = void**;
