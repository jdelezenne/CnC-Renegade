#include <fstream>
#include <memory>
#include <vector>
#include <cstring>
#include <DirectXTex.h>
#include <d3dx8.h>
#include <SDL3_image/SDL_image.h>
#include "Platform/Graphics/TextureImages.h"
#include "Platform/Paths.h"

std::int32_t Platform::LoadTextureImage(const char* filename, DirectX::ScratchImage& image,
    DirectX::TexMetadata& metadata, std::uint32_t& fileFormat)
{
    std::ifstream input(ReadPath(filename), std::ios::binary | std::ios::ate);
    if (!input) return E_FAIL;
    const auto length = input.tellg();
    if (length <= 0) return E_FAIL;
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
    input.seekg(0);
    if (!input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()))) return E_FAIL;
    HRESULT result = DirectX::LoadFromDDSMemory(bytes.data(), bytes.size(), DirectX::DDS_FLAGS_NONE, &metadata, image);
    fileFormat = D3DXIFF_DDS;
    if (FAILED(result)) {
        result = DirectX::LoadFromTGAMemory(bytes.data(), bytes.size(), DirectX::TGA_FLAGS_NONE, &metadata, image);
        fileFormat = D3DXIFF_TGA;
    }
    if (SUCCEEDED(result)) return result;
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> decoded(
        IMG_Load_IO(SDL_IOFromConstMem(bytes.data(), bytes.size()), true), SDL_DestroySurface);
    if (!decoded) return E_FAIL;
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(
        SDL_ConvertSurface(decoded.get(), SDL_PIXELFORMAT_BGRA32), SDL_DestroySurface);
    if (!surface) return E_FAIL;
    result = image.Initialize2D(DXGI_FORMAT_B8G8R8A8_UNORM, surface->w, surface->h, 1, 1);
    if (FAILED(result)) return result;
    const auto* destination = image.GetImage(0, 0, 0);
    for (int row = 0; row < surface->h; ++row)
        std::memcpy(destination->pixels + row * destination->rowPitch,
            static_cast<const std::uint8_t*>(surface->pixels) + row * surface->pitch,
            static_cast<std::size_t>(surface->w) * 4);
    metadata = image.GetMetadata();
    fileFormat = D3DXIFF_BMP;
    return S_OK;
}
