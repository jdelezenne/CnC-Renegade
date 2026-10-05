#pragma once
#include <cstdint>
#include <vector>
namespace Platform {
struct Font;
struct Glyph {
    int width = 0;
    int height = 0;
    std::vector<std::uint16_t> pixels;
};
bool RegisterFontFile(const char* path);
void ClearFontFiles();
Font* OpenFont(const char* family, int pointSize, bool bold);
void CloseFont(Font* font);
int FontHeight(const Font* font);
Glyph RasterizeGlyph(Font* font, std::uint32_t codepoint);
}
