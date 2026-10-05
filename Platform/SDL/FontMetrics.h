#pragma once
#include <vector>
namespace Platform {
struct FontPixelMetrics { int pixels, ascent, descent; };
struct FontMetrics {
    int units = 0;
    int ascent = 0;
    int descent = 0;
    std::vector<FontPixelMetrics> sizes;
};
FontMetrics ReadFontMetrics(const char* path);
}
