#pragma once

#include <cstdint>

namespace Platform {
bool FormatGraphicsError(std::int32_t error, char* output, std::uint32_t length);
}
