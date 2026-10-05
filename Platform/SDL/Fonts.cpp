#include "Platform/Fonts.h"
#include "Platform/SDL/FontMetrics.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
namespace {
struct FontFile { std::string path; std::string family; bool bold; Platform::FontMetrics metrics; };
std::vector<FontFile> Files;
struct FontInitialization {
    bool initialized = TTF_Init();
    ~FontInitialization() { if (initialized) TTF_Quit(); }
};
bool Equal(const std::string& first, const char* second) { return second && SDL_strcasecmp(first.c_str(), second) == 0; }
void DiscoverFonts() {
    std::error_code error;
    for (std::filesystem::directory_iterator entry(std::filesystem::current_path(error), error), end; !error && entry != end; entry.increment(error)) {
        const auto extension = entry->path().extension().string();
        if (Equal(extension, ".ttf") || Equal(extension, ".otf") || Equal(extension, ".ttc")) Platform::RegisterFontFile(entry->path().string().c_str());
    }
}
}
struct Platform::Font { TTF_Font* handle; int height; int ascent; bool syntheticBold; bool arial; };
bool Platform::RegisterFontFile(const char* path) {
    if (!path) return false;
    FontInitialization session;
    if (!session.initialized) return false;
    for (const auto& file : Files) if (file.path == path) return true;
    TTF_Font* font = TTF_OpenFont(path, 12);
    if (!font) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot load font %s: %s", path, SDL_GetError()); return false; }
    const char* family = TTF_GetFontFamilyName(font);
    const char* style = TTF_GetFontStyleName(font);
    Files.push_back({path, family ? family : "", style && (SDL_strcasestr(style, "bold") != nullptr), ReadFontMetrics(path)});
    TTF_CloseFont(font);
    return true;
}
void Platform::ClearFontFiles() { Files.clear(); }
Platform::Font* Platform::OpenFont(const char* family, int pointSize, bool bold) {
    if (!family || pointSize <= 0) return nullptr;
    FontInitialization session;
    if (!session.initialized) return nullptr;
    DiscoverFonts();
    const FontFile* selected = nullptr;
    for (const auto& file : Files) {
        if (Equal(file.family, family)) {
            if (!selected || file.bold == bold) selected = &file;
            if (file.bold == bold) break;
        }
    }
    if (!selected) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Font family unavailable: %s", family);
        return nullptr;
    }
    const int pixels = (pointSize * 96 + 36) / 72;
    TTF_Font* handle = TTF_OpenFont(selected->path.c_str(), static_cast<float>(pixels));
    if (!handle) return nullptr;
    TTF_SetFontHinting(handle, TTF_HINTING_MONO);
    int height = TTF_GetFontHeight(handle), ascent = TTF_GetFontAscent(handle);
    const auto& metrics = selected->metrics;
    if (metrics.units) {
        ascent = (metrics.ascent * pixels + metrics.units / 2) / metrics.units;
        height = ascent + (metrics.descent * pixels + metrics.units / 2) / metrics.units;
    }
    for (const auto& size : metrics.sizes) if (size.pixels == pixels) { ascent = size.ascent; height = ascent + size.descent; break; }
    auto* font = new Font{handle, height, ascent, bold && !selected->bold, Equal(selected->family, "Arial MT")};
    session.initialized = false;
    return font;
}
void Platform::CloseFont(Font* font) { if (font) { TTF_CloseFont(font->handle); delete font; TTF_Quit(); } }
int Platform::FontHeight(const Font* font) { return font ? font->height : 0; }
Platform::Glyph Platform::RasterizeGlyph(Font* font, std::uint32_t codepoint) {
    Glyph glyph;
    if (!font) return glyph;
    int minx = 0, maxx = 0, miny = 0, maxy = 0, advance = 0;
    if (!TTF_GetGlyphMetrics(font->handle, codepoint, &minx, &maxx, &miny, &maxy, &advance)) {
        codepoint = '?';
        if (!TTF_GetGlyphMetrics(font->handle, codepoint, &minx, &maxx, &miny, &maxy, &advance)) return glyph;
    }
    const bool padding = font->arial && (codepoint == 'W' || codepoint == 'V');
    glyph.width = std::max(0, advance) + (font->syntheticBold && advance > 0 ? 1 : 0) + (padding ? 1 : 0);
    glyph.height = font->height;
    glyph.pixels.resize(static_cast<std::size_t>(glyph.width) * glyph.height);
    if (!glyph.width || codepoint == ' ' || codepoint == '\t') return glyph;
    SDL_Surface* surface = TTF_RenderGlyph_Blended(font->handle, codepoint, {255,255,255,255});
    if (!surface) return glyph;
    SDL_Surface* rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);
    if (!rgba) return glyph;
    const int offset = font->ascent - TTF_GetFontAscent(font->handle);
    for (int y = 0; y < rgba->h; ++y) {
        if (y + offset < 0 || y + offset >= glyph.height) continue;
        const auto* row = static_cast<const std::uint8_t*>(rgba->pixels) + y * rgba->pitch;
        for (int x = 0; x < glyph.width; ++x) {
            auto alpha = x < rgba->w ? row[x * 4 + 3] : std::uint8_t{0};
            if (font->syntheticBold && x > 0 && x - 1 < rgba->w) alpha = std::max(alpha, row[(x - 1) * 4 + 3]);
            glyph.pixels[static_cast<std::size_t>(y + offset) * glyph.width + x] = alpha ? static_cast<std::uint16_t>(0x0fff | ((alpha >> 4) << 12)) : 0;
        }
    }
    SDL_DestroySurface(rgba);
    return glyph;
}
