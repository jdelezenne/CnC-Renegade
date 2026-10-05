#include "Platform/SDL/FontMetrics.h"
#include <SDL3/SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include <cstdint>
#include <limits>
Platform::FontMetrics Platform::ReadFontMetrics(const char* path) {
    FontMetrics result;
    std::size_t size = 0;
    auto* bytes = static_cast<FT_Byte*>(SDL_LoadFile(path, &size));
    if (!bytes) return result;
    FT_Library library = nullptr;
    FT_Face face = nullptr;
    if (size > static_cast<std::size_t>(std::numeric_limits<FT_Long>::max()) || FT_Init_FreeType(&library) || FT_New_Memory_Face(library, bytes, static_cast<FT_Long>(size), 0, &face)) {
        if (library) FT_Done_FreeType(library);
        SDL_free(bytes);
        return result;
    }
    if (const auto* metrics = static_cast<const TT_OS2*>(FT_Get_Sfnt_Table(face, ft_sfnt_os2))) {
        if (metrics->version != 0xffff && face->units_per_EM && metrics->usWinAscent) {
            result.units = face->units_per_EM;
            result.ascent = metrics->usWinAscent;
            result.descent = metrics->usWinDescent;
        }
    }
    FT_ULong length = 0;
    const auto tag = FT_MAKE_TAG('V', 'D', 'M', 'X');
    if (!FT_Load_Sfnt_Table(face, tag, 0, nullptr, &length) && length >= 6 && length <= 65536) {
        std::vector<FT_Byte> table(length);
        if (!FT_Load_Sfnt_Table(face, tag, 0, table.data(), &length)) {
            const auto word = [&](std::size_t offset) { return (static_cast<unsigned>(table[offset]) << 8) | table[offset + 1]; };
            const auto ratios = word(4);
            if (6 + static_cast<std::size_t>(ratios) * 6 <= table.size()) {
                for (unsigned index = 0; index < ratios; ++index) {
                    const auto ratio = 6 + index * 4;
                    const unsigned x = table[ratio + 1], start = table[ratio + 2], end = table[ratio + 3];
                    if (!((x == 0 && start == 0 && end == 0) || (x >= start && x <= end))) continue;
                    const auto group = word(6 + ratios * 4 + index * 2);
                    if (group + 4 > table.size()) break;
                    const auto count = word(group);
                    if (group + 4 + static_cast<std::size_t>(count) * 6 > table.size()) break;
                    for (unsigned entry = 0; entry < count; ++entry) {
                        const auto offset = group + 4 + entry * 6;
                        const int ascent = static_cast<std::int16_t>(word(offset + 2));
                        const int descent = -static_cast<std::int16_t>(word(offset + 4));
                        if (ascent > 0 && descent >= 0) result.sizes.push_back({static_cast<int>(word(offset)), ascent, descent});
                    }
                    break;
                }
            }
        }
    }
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    SDL_free(bytes);
    return result;
}
