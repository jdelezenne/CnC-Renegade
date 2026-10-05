#include "Platform/Directory.h"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>
#include <algorithm>
#include <filesystem>
#include <memory>

std::vector<Platform::DirectoryEntry> Platform::ListFiles(const char* search)
{
    std::string normalized(search);
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    const std::filesystem::path mask(normalized);
    const auto directory = mask.has_parent_path() ? mask.parent_path() : std::filesystem::path(".");
    const auto directory_name = directory.string();
    const auto pattern = mask.filename().string();
    int count = 0;
    std::unique_ptr<char*, decltype(&SDL_free)> names(
        SDL_GlobDirectory(directory_name.c_str(), pattern.c_str(), SDL_GLOB_CASEINSENSITIVE, &count), SDL_free);
    std::vector<DirectoryEntry> result;
    for (int i = 0; names && i < count; ++i) {
        const auto path = directory / names.get()[i];
        const auto full_path = path.string();
        SDL_PathInfo info{};
        if (!SDL_GetPathInfo(full_path.c_str(), &info) || info.type != SDL_PATHTYPE_FILE) continue;
        const SDL_Time ticks = info.modify_time;
        SDL_DateTime local{};
        if (!SDL_TimeToDateTime(ticks, &local, true)) continue;
        result.push_back({names.get()[i], path.string(), ticks, local});
    }
    return result;
}
