#include "Platform/Paths.h"
#include <SDL3/SDL.h>
#include <filesystem>
#include <cstring>
#include <stdexcept>

namespace fs = std::filesystem;

namespace {
fs::path FilePath(const char* name)
{
#ifdef _WIN32
    return fs::path(name);
#else
    std::string normalized(name);
    for (char& c : normalized) if (c == '\\') c = '/';
    return fs::path(normalized);
#endif
}

bool SameComponent(const fs::path& left, const fs::path& right)
{
#ifdef _WIN32
    return _wcsicmp(left.c_str(), right.c_str()) == 0;
#else
    return left == right;
#endif
}
}

const std::string& Platform::PreferenceDirectory()
{
    static const std::string directory = [] {
        char* path = SDL_GetPrefPath("Electronic Arts", "Renegade");
        if (!path) throw std::runtime_error(SDL_GetError());
        std::string result = fs::path(std::u8string(path, path + std::strlen(path))).string();
        SDL_free(path);
        return result;
    }();
    return directory;
}

std::string Platform::UserPath(const char* relativePath)
{
    const fs::path relative = FilePath(relativePath).lexically_normal();
    if (relative.has_root_path() || (!relative.empty() && *relative.begin() == ".."))
        throw std::invalid_argument("User paths must stay inside the preferences folder");
    const fs::path path = fs::path(PreferenceDirectory()) / relative;
    fs::create_directories(path.parent_path());
    return path.string();
}

namespace {
fs::path UserRelativePath(const char* name)
{
    const fs::path path = FilePath(name);
    if (!path.is_absolute()) return path.lexically_normal();
    const fs::path absolute = path.lexically_normal();
    const fs::path base = fs::current_path().lexically_normal();
    auto item = absolute.begin();
    for (auto root = base.begin(); root != base.end(); ++root, ++item) {
        if (item == absolute.end() || !SameComponent(*item, *root)) return path;
    }
    fs::path relative;
    for (; item != absolute.end(); ++item) relative /= *item;
    if (!relative.empty() && *relative.begin() != "..") return relative;
    return path;
}
}

std::string Platform::WritePath(const char* path)
{
    const fs::path relative = UserRelativePath(path);
    if (relative.is_absolute()) return relative.string();
    return UserPath(relative.string().c_str());
}

std::string Platform::ReadPath(const char* path)
{
    const fs::path relative = UserRelativePath(path);
    if (!relative.is_absolute() && (relative.empty() || *relative.begin() != "..")) {
        const fs::path user = fs::path(PreferenceDirectory()) / relative;
        if (fs::is_regular_file(user)) return user.string();
    }
#ifdef _WIN32
    return path;
#else
    return FilePath(path).string();
#endif
}

std::FILE* Platform::OpenStream(const char* path, const char* mode)
{
    const bool write = std::strpbrk(mode, "wa+") != nullptr;
    const std::string resolved = write ? WritePath(path) : ReadPath(path);
    return std::fopen(resolved.c_str(), mode);
}

std::string Platform::FileName(const char* path)
{
    const std::string name(path);
    const auto separator = name.find_last_of("/\\");
    if (separator != std::string::npos) return name.substr(separator + 1);
    return name.size() >= 2 && name[1] == ':' ? name.substr(2) : name;
}

std::string Platform::FileStem(const char* path)
{
    const std::string name = FileName(path);
    return name.substr(0, name.find_last_of('.'));
}

std::string Platform::FileExtension(const char* path)
{
    const std::string name = FileName(path);
    const auto extension = name.find_last_of('.');
    return extension == std::string::npos ? std::string() : name.substr(extension);
}
