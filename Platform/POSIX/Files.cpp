#include "Platform/Files.h"
#include "Platform/Paths.h"
#include <cerrno>
#include <climits>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
std::FILE* Stream(void* handle) { return static_cast<std::FILE*>(handle); }
bool ValidFile(void* handle)
{
    if (!handle || handle == Platform::InvalidFileHandle()) { errno = EBADF; return false; }
    return true;
}
}

void* Platform::OpenRawFile(const char* path, FileMode mode)
{
    if (mode != FileMode::Read && mode != FileMode::Write && mode != FileMode::ReadWrite && mode != FileMode::CreateNew) { errno = EINVAL; return InvalidFileHandle(); }
    const bool write = mode != FileMode::Read;
    const std::string resolved = write ? WritePath(path) : ReadPath(path);
    const int flags = mode == FileMode::Read ? O_RDONLY : mode == FileMode::Write ? O_WRONLY | O_CREAT | O_TRUNC
        : mode == FileMode::CreateNew ? O_WRONLY | O_CREAT | O_EXCL : O_RDWR | O_CREAT;
    const int descriptor = open(resolved.c_str(), flags | O_CLOEXEC, 0666);
    if (descriptor == -1) return InvalidFileHandle();
    struct stat info{};
    if (fstat(descriptor, &info) != 0 || S_ISDIR(info.st_mode)) {
        const int error = S_ISDIR(info.st_mode) ? EISDIR : errno;
        close(descriptor);
        errno = error;
        return InvalidFileHandle();
    }
    std::FILE* file = fdopen(descriptor, mode == FileMode::Read ? "rb" : mode == FileMode::ReadWrite ? "r+b" : "wb");
    if (!file) {
        const int error = errno;
        close(descriptor);
        errno = error;
        return InvalidFileHandle();
    }
    return file;
}
bool Platform::CloseRawFile(void* handle) { return ValidFile(handle) && std::fclose(Stream(handle)) == 0; }
bool Platform::ReadRawFile(void* handle, void* buffer, int size, std::uint32_t& count)
{
    count = 0;
    if (!ValidFile(handle)) return false;
    if (size < 0) { errno = EINVAL; return false; }
    count = static_cast<std::uint32_t>(std::fread(buffer, 1, size, Stream(handle)));
    return std::ferror(Stream(handle)) == 0;
}
bool Platform::WriteRawFile(void* handle, const void* buffer, int size, std::uint32_t& count)
{
    count = 0;
    if (!ValidFile(handle)) return false;
    if (size < 0) { errno = EINVAL; return false; }
    count = static_cast<std::uint32_t>(std::fwrite(buffer, 1, size, Stream(handle)));
    return count == static_cast<std::uint32_t>(size);
}
int Platform::SeekRawFile(void* handle, int offset, int origin)
{
    if (!ValidFile(handle)) return -1;
    if (std::fseek(Stream(handle), offset, origin) != 0) return -1;
    const long position = std::ftell(Stream(handle));
    if (position > INT_MAX) { errno = EOVERFLOW; return -1; }
    return static_cast<int>(position);
}
int Platform::RawFileSize(void* handle)
{
    if (!ValidFile(handle)) return -1;
    if (std::fflush(Stream(handle)) != 0) return -1;
    struct stat info;
    if (fstat(fileno(Stream(handle)), &info) != 0) return -1;
    if (info.st_size > INT_MAX) { errno = EOVERFLOW; return -1; }
    return static_cast<int>(info.st_size);
}
std::uint32_t Platform::RawFileDateTime(void* handle)
{
    if (!ValidFile(handle)) return 0;
    if (std::fflush(Stream(handle)) != 0) return 0;
    struct stat info;
    std::tm time;
    if (fstat(fileno(Stream(handle)), &info) != 0 || !gmtime_r(&info.st_mtime, &time)) return 0;
    if (time.tm_year < 80 || time.tm_year > 207) { errno = EOVERFLOW; return 0; }
    const std::uint32_t date = ((time.tm_year - 80) << 9) | ((time.tm_mon + 1) << 5) | time.tm_mday;
    const std::uint32_t clock = (time.tm_hour << 11) | (time.tm_min << 5) | (time.tm_sec / 2);
    return (date << 16) | clock;
}
bool Platform::SetRawFileDateTime(void* handle, std::uint32_t dateTime)
{
    if (!ValidFile(handle)) return false;
    if (std::fflush(Stream(handle)) != 0) return false;
    std::tm time{};
    time.tm_year = static_cast<int>(dateTime >> 25) + 80;
    time.tm_mon = static_cast<int>((dateTime >> 21) & 15) - 1;
    time.tm_mday = (dateTime >> 16) & 31;
    time.tm_hour = (dateTime >> 11) & 31;
    time.tm_min = (dateTime >> 5) & 63;
    time.tm_sec = (dateTime & 31) * 2;
    if (time.tm_mon < 0 || time.tm_mon > 11 || time.tm_mday < 1 || time.tm_mday > 31 ||
        time.tm_hour > 23 || time.tm_min > 59 || time.tm_sec > 59) { errno = EINVAL; return false; }
    const std::tm requested = time;
    const std::time_t seconds = timegm(&time);
    std::tm normalized;
    if (!gmtime_r(&seconds, &normalized) || normalized.tm_year != requested.tm_year || normalized.tm_mon != requested.tm_mon ||
        normalized.tm_mday != requested.tm_mday || normalized.tm_hour != requested.tm_hour || normalized.tm_min != requested.tm_min ||
        normalized.tm_sec != requested.tm_sec) { errno = EINVAL; return false; }
    const timespec times[] = {{seconds, 0}, {seconds, 0}};
    return futimens(fileno(Stream(handle)), times) == 0;
}
bool Platform::RemoveRawFile(const char* path) { return std::remove(WritePath(path).c_str()) == 0; }
int Platform::LastFileError() { return errno; }

bool Platform::RenameRawFile(const char* source, const char* destination)
{
    const auto from = WritePath(source);
    const auto to = WritePath(destination);
    return renameat2(AT_FDCWD, from.c_str(), AT_FDCWD, to.c_str(), RENAME_NOREPLACE) == 0;
}

bool Platform::IsReadOnlyFile(const char* path)
{
    struct stat info{};
    return stat(ReadPath(path).c_str(), &info) == 0 && !(info.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH));
}
