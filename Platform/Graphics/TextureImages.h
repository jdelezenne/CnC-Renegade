#pragma once

#include <cstdint>

namespace DirectX {
class ScratchImage;
struct TexMetadata;
}

namespace Platform {
std::int32_t LoadTextureImage(const char* filename, DirectX::ScratchImage& image,
    DirectX::TexMetadata& metadata, std::uint32_t& fileFormat);
}
