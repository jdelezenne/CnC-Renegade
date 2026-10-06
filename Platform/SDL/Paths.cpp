#include "Platform/Paths.h"
#include <SDL3/SDL.h>
#include <filesystem>
#include <cstring>
#include <stdexcept>
#include <map>
#include <mutex>
#include <unordered_map>

namespace fs = std::filesystem;

namespace {
fs::path FilePath(const char* name)
{
    std::string normalized(name);
    for (char& c : normalized) if (c == '\\') c = '/';
    return fs::path(normalized);
}

struct CaseInsensitiveLess {
    bool operator()(const std::string& left, const std::string& right) const
    {
        return SDL_strcasecmp(left.c_str(), right.c_str()) < 0;
    }
};
struct FileNameEntry { fs::path Name; bool Ambiguous = false; };
struct DirectoryIndex {
    fs::file_time_type Modified{};
    bool Ready = false;
    std::map<std::string, FileNameEntry, CaseInsensitiveLess> Names;
};
struct DirectoryCache {
    std::mutex Mutex;
    std::unordered_map<fs::path, DirectoryIndex> Directories;
};
DirectoryCache& PathCache()
{
    static DirectoryCache cache;
    return cache;
}

std::string Utf8Name(const fs::path& path)
{
    const auto name = path.u8string();
    return {reinterpret_cast<const char*>(name.data()), name.size()};
}

fs::path ResolvePath(const fs::path& path)
{
    if (path.empty()) return path;
    std::error_code error;
    if (fs::exists(path, error)) return path;
    const auto parent = path.parent_path();
    if (parent.empty()) return ResolvePath(fs::absolute(path));
    if (parent == path) return path;
    const auto directory = ResolvePath(parent);
    const auto requested = directory / path.filename();
    if (!fs::is_directory(directory, error)) return requested;
    const auto modified = fs::last_write_time(directory, error);
    if (error) return requested;
    auto& cache = PathCache();
    std::lock_guard lock(cache.Mutex);
    auto& index = cache.Directories[directory];
    if (!index.Ready || index.Modified != modified) {
        decltype(index.Names) names;
        for (fs::directory_iterator entry(directory, error), end; !error && entry != end; entry.increment(error)) {
            const auto name = entry->path().filename();
            auto [item, inserted] = names.try_emplace(Utf8Name(name), FileNameEntry{name});
            if (!inserted) item->second.Ambiguous = true;
        }
        if (error) return requested;
        index.Names = std::move(names);
        index.Modified = modified;
        index.Ready = true;
    }
    const auto item = index.Names.find(Utf8Name(path.filename()));
    return item == index.Names.end() || item->second.Ambiguous ? requested : directory / item->second.Name;
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
    const fs::path path = ResolvePath(fs::path(PreferenceDirectory()) / relative);
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
    if (relative.is_absolute()) return ResolvePath(relative).string();
    return UserPath(relative.string().c_str());
}

std::string Platform::ReadPath(const char* path)
{
    const fs::path relative = UserRelativePath(path);
    if (!relative.is_absolute() && (relative.empty() || *relative.begin() != "..")) {
        const fs::path user = ResolvePath(fs::path(PreferenceDirectory()) / relative);
        if (fs::is_regular_file(user)) return user.string();
    }
    return ResolvePath(FilePath(path)).string();
}

bool Platform::HasRootPath(const char* path)
{
    return path && *path && FilePath(path).has_root_path();
}

std::FILE* Platform::OpenStream(const char* path, const char* mode)
{
    const bool write = std::strpbrk(mode, "wa+") != nullptr;
    const std::string resolved = write ? WritePath(path) : ReadPath(path);
    return std::fopen(resolved.c_str(), mode);
}

bool Platform::CreateUserDirectory(const char* path)
{
    const auto resolved = WritePath(path);
    std::error_code error;
    if (fs::create_directory(resolved, error)) return true;
    return !error && fs::is_directory(resolved, error);
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
