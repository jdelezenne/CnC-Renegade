#include <algorithm>
#include <bit>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>
#include <d3dx8.h>
#include "Platform/Graphics/Types.h"
#include "Platform/Graphics/TextureImages.h"
#include <DirectXTex.h>
#ifdef _WIN32
#include "Platform/Windows/GraphicsErrors.h"
#endif

namespace {
DXGI_FORMAT Format(D3DFORMAT format)
{
    switch (format) {
    case D3DFMT_A8R8G8B8: return DXGI_FORMAT_B8G8R8A8_UNORM;
    case D3DFMT_X8R8G8B8: return DXGI_FORMAT_B8G8R8X8_UNORM;
    case D3DFMT_R5G6B5: return DXGI_FORMAT_B5G6R5_UNORM;
    case D3DFMT_A1R5G5B5: case D3DFMT_X1R5G5B5: return DXGI_FORMAT_B5G5R5A1_UNORM;
    case D3DFMT_A4R4G4B4: return DXGI_FORMAT_B4G4R4A4_UNORM;
    case D3DFMT_A8: return DXGI_FORMAT_A8_UNORM;
    case D3DFMT_L8: return DXGI_FORMAT_R8_UNORM;
    case D3DFMT_A8L8: return DXGI_FORMAT_R8G8_UNORM;
    case D3DFMT_V8U8: return DXGI_FORMAT_R8G8_SNORM;
    case D3DFMT_V16U16: return DXGI_FORMAT_R16G16_SNORM;
    case D3DFMT_Q8W8V8U8: return DXGI_FORMAT_R8G8B8A8_SNORM;
    case D3DFMT_DXT1: return DXGI_FORMAT_BC1_UNORM;
    case D3DFMT_DXT2: case D3DFMT_DXT3: return DXGI_FORMAT_BC2_UNORM;
    case D3DFMT_DXT4: case D3DFMT_DXT5: return DXGI_FORMAT_BC3_UNORM;
    default: return DXGI_FORMAT_UNKNOWN;
    }
}
DirectX::TEX_FILTER_FLAGS Filter(DWORD filter)
{
    unsigned mode;
    switch (filter & 15) {
    case D3DX_FILTER_NONE: case D3DX_FILTER_POINT: mode = DirectX::TEX_FILTER_POINT; break;
    case D3DX_FILTER_LINEAR: mode = DirectX::TEX_FILTER_LINEAR; break;
    case D3DX_FILTER_TRIANGLE: mode = DirectX::TEX_FILTER_TRIANGLE; break;
    default: mode = DirectX::TEX_FILTER_BOX; break;
    }
    return static_cast<DirectX::TEX_FILTER_FLAGS>(mode | DirectX::TEX_FILTER_FORCE_NON_WIC | DirectX::TEX_FILTER_SEPARATE_ALPHA);
}
HRESULT Read(IDirect3DSurface8* surface, const RECT* rect, const Platform::GraphicsPaletteEntry* palette, DirectX::ScratchImage& image)
{
    D3DSURFACE_DESC desc{};
    HRESULT result = surface->GetDesc(&desc);
    if (FAILED(result)) return result;
    RECT area = rect ? *rect : RECT{0, 0, static_cast<LONG>(desc.Width), static_cast<LONG>(desc.Height)};
    if (area.left < 0 || area.top < 0 || area.right <= area.left || area.bottom <= area.top ||
        area.right > static_cast<LONG>(desc.Width) || area.bottom > static_cast<LONG>(desc.Height)) return D3DERR_INVALIDCALL;
    const bool expanded = desc.Format == D3DFMT_R8G8B8 || desc.Format == D3DFMT_P8 ||
        desc.Format == D3DFMT_A8L8;
    DXGI_FORMAT format = expanded ? DXGI_FORMAT_B8G8R8A8_UNORM : Format(desc.Format);
    if (format == DXGI_FORMAT_UNKNOWN || (desc.Format == D3DFMT_P8 && !palette)) return D3DERR_NOTAVAILABLE;
    result = image.Initialize2D(format, area.right - area.left, area.bottom - area.top, 1, 1);
    if (FAILED(result)) return result;
    D3DLOCKED_RECT locked{};
    result = surface->LockRect(&locked, &area, D3DLOCK_READONLY);
    if (FAILED(result)) return result;
    const auto* output = image.GetImage(0, 0, 0);
    const size_t rows = DirectX::IsCompressed(format) ? (output->height + 3) / 4 : output->height;
    for (size_t y = 0; y < rows; ++y) {
        const auto* source = static_cast<const unsigned char*>(locked.pBits) + y * locked.Pitch;
        auto* destination = output->pixels + y * output->rowPitch;
        if (!expanded) {
            std::memcpy(destination, source, output->rowPitch);
            if (desc.Format == D3DFMT_X1R5G5B5)
                for (size_t x = 0; x < output->width; ++x) destination[x*2+1] |= 128;
        }
        else for (size_t x = 0; x < output->width; ++x) {
            if (desc.Format == D3DFMT_P8) {
                const auto& color = palette[source[x]];
                destination[x*4] = color.peBlue; destination[x*4+1] = color.peGreen;
                destination[x*4+2] = color.peRed; destination[x*4+3] = color.peFlags;
            } else if (desc.Format == D3DFMT_A8L8) {
                destination[x*4] = destination[x*4+1] = destination[x*4+2] = source[x*2];
                destination[x*4+3] = source[x*2+1];
            } else { std::memcpy(destination + x*4, source + x*3, 3); destination[x*4+3] = 255; }
        }
    }
    return surface->UnlockRect();
}
HRESULT Write(IDirect3DSurface8* surface, const RECT* rect, const DirectX::Image& source, DWORD filter, D3DCOLOR key)
{
    D3DSURFACE_DESC desc{};
    HRESULT result = surface->GetDesc(&desc);
    if (FAILED(result)) return result;
    RECT area = rect ? *rect : RECT{0, 0, static_cast<LONG>(desc.Width), static_cast<LONG>(desc.Height)};
    if (area.left < 0 || area.top < 0 || area.right <= area.left || area.bottom <= area.top ||
        area.right > static_cast<LONG>(desc.Width) || area.bottom > static_cast<LONG>(desc.Height)) return D3DERR_INVALIDCALL;
    DXGI_FORMAT format = desc.Format == D3DFMT_R8G8B8 ? DXGI_FORMAT_B8G8R8A8_UNORM : Format(desc.Format);
    if (format == DXGI_FORMAT_UNKNOWN) return D3DERR_NOTAVAILABLE;
    const DirectX::Image* pixels = &source;
    DirectX::ScratchImage decoded, keyed, working, resized, converted;
    if (DirectX::IsCompressed(pixels->format) && (pixels->format != format || key ||
        pixels->width != static_cast<size_t>(area.right-area.left) || pixels->height != static_cast<size_t>(area.bottom-area.top))) {
        result = DirectX::Decompress(*pixels, DXGI_FORMAT_R8G8B8A8_UNORM, decoded);
        if (FAILED(result)) return result;
        pixels = decoded.GetImage(0, 0, 0);
    }
    if (key) {
        result = DirectX::Convert(*pixels, DXGI_FORMAT_B8G8R8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, 0.5f, keyed);
        if (FAILED(result)) return result;
        pixels = keyed.GetImage(0, 0, 0);
        for (size_t y = 0; y < pixels->height; ++y) for (size_t x = 0; x < pixels->width; ++x) {
            DWORD color; auto* pixel = pixels->pixels + y*pixels->rowPitch + x*4;
            std::memcpy(&color, pixel, 4); if (color == key) pixel[3] = 0;
        }
    }
    if (pixels->width != static_cast<size_t>(area.right-area.left) || pixels->height != static_cast<size_t>(area.bottom-area.top)) {
        if (pixels->format != DXGI_FORMAT_R32G32B32A32_FLOAT) {
            result = DirectX::Convert(*pixels, DXGI_FORMAT_R32G32B32A32_FLOAT, DirectX::TEX_FILTER_DEFAULT, 0.5f, working);
            if (FAILED(result)) return result;
            pixels = working.GetImage(0, 0, 0);
        }
        result = DirectX::Resize(*pixels, area.right-area.left, area.bottom-area.top, Filter(filter), resized);
        if (FAILED(result)) return result;
        pixels = resized.GetImage(0, 0, 0);
    }
    if (pixels->format != format) {
        if (DirectX::IsCompressed(format)) result = DirectX::Compress(*pixels, format, DirectX::TEX_COMPRESS_DEFAULT, 0.5f, converted);
        else result = DirectX::Convert(*pixels, format, Filter(filter), 0.5f, converted);
        if (FAILED(result)) return result;
        pixels = converted.GetImage(0, 0, 0);
    }
    D3DLOCKED_RECT locked{};
    result = surface->LockRect(&locked, &area, 0);
    if (FAILED(result)) return result;
    const size_t rows = DirectX::IsCompressed(format) ? (pixels->height+3)/4 : pixels->height;
    for (size_t y = 0; y < rows; ++y) {
        auto* destination = static_cast<unsigned char*>(locked.pBits) + y*locked.Pitch;
        const auto* input = pixels->pixels + y*pixels->rowPitch;
        if (desc.Format == D3DFMT_R8G8B8) for (size_t x = 0; x < pixels->width; ++x) std::memcpy(destination+x*3, input+x*4, 3);
        else std::memcpy(destination, input, pixels->rowPitch);
    }
    return surface->UnlockRect();
}
}

extern "C" UINT WINAPI D3DXGetFVFVertexSize(DWORD fvf)
{
    unsigned position = fvf & D3DFVF_POSITION_MASK;
    unsigned size = position == D3DFVF_XYZRHW ? 16 : 12;
    if (position >= D3DFVF_XYZB1 && position <= D3DFVF_XYZB5) size += 4*((position-D3DFVF_XYZB1)/2+1);
    if (fvf&D3DFVF_NORMAL) size += 12;
    if (fvf&D3DFVF_PSIZE) size += 4;
    if (fvf&D3DFVF_DIFFUSE) size += 4;
    if (fvf&D3DFVF_SPECULAR) size += 4;
    constexpr unsigned dimensions[]{2, 3, 4, 1};
    unsigned count = (fvf&D3DFVF_TEXCOUNT_MASK)>>D3DFVF_TEXCOUNT_SHIFT;
    for (unsigned i = 0; i < count; ++i) size += 4*dimensions[(fvf>>(16+i*2))&3];
    return size;
}
extern "C" D3DXMATRIX* WINAPI D3DXMatrixTranspose(D3DXMATRIX* output, const D3DXMATRIX* input)
{
    if (!output || !input) return nullptr;
    D3DXMATRIX result;
    for (unsigned y = 0; y < 4; ++y) for (unsigned x = 0; x < 4; ++x) result.m[y][x] = input->m[x][y];
    *output = result; return output;
}
extern "C" D3DXVECTOR4* WINAPI D3DXVec3Transform(D3DXVECTOR4* output, const D3DXVECTOR3* input, const D3DXMATRIX* matrix)
{
    if (!output || !input || !matrix) return nullptr;
    D3DXVECTOR4 result;
    for (unsigned i = 0; i < 4; ++i) (&result.x)[i] = input->x*matrix->m[0][i] + input->y*matrix->m[1][i] + input->z*matrix->m[2][i] + matrix->m[3][i];
    *output = result; return output;
}
extern "C" HRESULT WINAPI D3DXGetErrorStringA(HRESULT error, char* output, UINT length)
{
    if (!output || !length) return E_INVALIDARG;
#ifdef _WIN32
    if (Platform::FormatGraphicsError(error, output, length)) return S_OK;
#endif
    std::snprintf(output, length, "Direct3D error 0x%08x", static_cast<std::uint32_t>(error));
    return S_OK;
}
extern "C" HRESULT WINAPI D3DXCreateTexture(IDirect3DDevice8* device, UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture8** output)
{
    if (!device || !output) return D3DERR_INVALIDCALL;
    *output = nullptr;
    if (!width || !height || width == D3DX_DEFAULT || height == D3DX_DEFAULT) return D3DERR_INVALIDCALL;
    if (format == D3DFMT_UNKNOWN) format = D3DFMT_A8R8G8B8;
    if (levels == D3DX_DEFAULT) levels = 0;
    return device->CreateTexture(width, height, levels, usage, format, pool, output);
}
extern "C" HRESULT WINAPI D3DXLoadSurfaceFromSurface(IDirect3DSurface8* destination, const Platform::GraphicsPaletteEntry*, const RECT* destinationRect, IDirect3DSurface8* source, const Platform::GraphicsPaletteEntry* sourcePalette, const RECT* sourceRect, DWORD filter, D3DCOLOR key)
{
    if (!source || !destination) return D3DERR_INVALIDCALL;
    DirectX::ScratchImage pixels;
    HRESULT result = Read(source, sourceRect, sourcePalette, pixels);
    return FAILED(result) ? result : Write(destination, destinationRect, *pixels.GetImage(0, 0, 0), filter, key);
}
extern "C" HRESULT WINAPI D3DXFilterTexture(IDirect3DBaseTexture8* base, const Platform::GraphicsPaletteEntry* palette, UINT sourceLevel, DWORD filter)
{
    if (!base || base->GetType() != D3DRTYPE_TEXTURE || sourceLevel >= base->GetLevelCount()) return D3DERR_INVALIDCALL;
    auto* texture = static_cast<IDirect3DTexture8*>(base);
    for (UINT level = sourceLevel+1; level < texture->GetLevelCount(); ++level) {
        IDirect3DSurface8 *source = nullptr, *destination = nullptr;
        HRESULT result = texture->GetSurfaceLevel(level-1, &source);
        if (SUCCEEDED(result)) result = texture->GetSurfaceLevel(level, &destination);
        if (SUCCEEDED(result)) result = D3DXLoadSurfaceFromSurface(destination, palette, nullptr, source, palette, nullptr, filter, 0);
        if (source) source->Release(); if (destination) destination->Release();
        if (FAILED(result)) return result;
    }
    return S_OK;
}
extern "C" HRESULT WINAPI D3DXCreateTextureFromFileExA(IDirect3DDevice8* device, const char* filename, UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, DWORD filter, DWORD mipFilter, D3DCOLOR key, D3DXIMAGE_INFO* info, Platform::GraphicsPaletteEntry*, IDirect3DTexture8** output)
{
    if (!device || !filename || !output) return D3DERR_INVALIDCALL;
    *output = nullptr;
    DirectX::ScratchImage image; DirectX::TexMetadata metadata;
    std::uint32_t fileFormat = D3DXIFF_DDS;
    HRESULT result = Platform::LoadTextureImage(filename, image, metadata, fileFormat);
    if (FAILED(result)) return result;
    if (!width || width == D3DX_DEFAULT) width = static_cast<UINT>(metadata.width);
    if (!height || height == D3DX_DEFAULT) height = static_cast<UINT>(metadata.height);
    result = D3DXCreateTexture(device, width, height, levels, usage, format, pool, output);
    if (FAILED(result)) return result;
    IDirect3DSurface8* surface = nullptr;
    result = (*output)->GetSurfaceLevel(0, &surface);
    if (SUCCEEDED(result)) result = Write(surface, nullptr, *image.GetImage(0, 0, 0), filter, key);
    if (surface) surface->Release();
    if (SUCCEEDED(result)) result = D3DXFilterTexture(*output, nullptr, 0, mipFilter);
    if (FAILED(result)) { (*output)->Release(); *output = nullptr; return result; }
    if (info) *info = {static_cast<UINT>(metadata.width), static_cast<UINT>(metadata.height), static_cast<UINT>(metadata.depth), static_cast<UINT>(metadata.mipLevels), format, D3DRTYPE_TEXTURE, static_cast<D3DXIMAGE_FILEFORMAT>(fileFormat)};
    return S_OK;
}

extern "C" D3DXMATRIX* WINAPI D3DXMatrixMultiply(D3DXMATRIX* output, const D3DXMATRIX* left, const D3DXMATRIX* right)
{
    if (!output || !left || !right) return nullptr;
    D3DXMATRIX result;
    for (unsigned row = 0; row < 4; ++row)
        for (unsigned column = 0; column < 4; ++column)
            result.m[row][column] = left->m[row][0] * right->m[0][column] + left->m[row][1] * right->m[1][column] + left->m[row][2] * right->m[2][column] + left->m[row][3] * right->m[3][column];
    *output = result;
    return output;
}
