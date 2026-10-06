#pragma once
#include <bit>
#include <cstddef>
#include <cstdint>

inline std::uint32_t Compute_Packet_CRC(const unsigned char* data, std::size_t length)
{
    std::uint32_t crc = 0;
    for (std::size_t offset = 0; offset < length;) {
        std::uint32_t word = 0;
        for (unsigned shift = 0; shift < 32 && offset < length; shift += 8) {
            word |= static_cast<std::uint32_t>(data[offset++]) << shift;
        }
        crc = std::rotl(crc, 1) + word;
    }
    return crc;
}
