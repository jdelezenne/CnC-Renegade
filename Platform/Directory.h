#pragma once

#include <SDL3/SDL_time.h>
#include <string>
#include <vector>

namespace Platform {
struct DirectoryEntry {
    std::string Name;
    std::string Path;
    SDL_Time ModifiedTime;
    SDL_DateTime LocalTime;
};
std::vector<DirectoryEntry> ListFiles(const char* search);
}
