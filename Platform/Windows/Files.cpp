#include "Platform/Windows/Files.h"
#include "Platform/Files.h"
#include <cstdio>
#include "Platform/Paths.h"

HANDLE Platform::OpenFile(const char* path, DWORD access, DWORD sharing,
    LPSECURITY_ATTRIBUTES security, DWORD creation, DWORD attributes, HANDLE templateFile)
{
    const bool write = (access & (GENERIC_WRITE | GENERIC_ALL | DELETE)) || creation != OPEN_EXISTING;
    const std::string resolved = write ? WritePath(path) : ReadPath(path);
    return CreateFileA(resolved.c_str(), access, sharing, security, creation, attributes, templateFile);
}

BOOL Platform::MakeDirectory(const char* path, LPSECURITY_ATTRIBUTES security)
{
    return CreateDirectoryA(WritePath(path).c_str(), security);
}

BOOL Platform::RemoveFile(const char* path)
{
    return DeleteFileA(WritePath(path).c_str());
}

BOOL Platform::RenameFile(const char* source, const char* destination)
{
    return MoveFileA(WritePath(source).c_str(), WritePath(destination).c_str());
}

BOOL Platform::DuplicateFile(const char* source, const char* destination, BOOL failIfExists)
{
    return CopyFileA(ReadPath(source).c_str(), WritePath(destination).c_str(), failIfExists);
}

namespace {
bool ValidRawFile(void* handle)
{
    if (!handle || handle == Platform::InvalidFileHandle()) { SetLastError(ERROR_INVALID_HANDLE); return false; }
    return true;
}
}

void* Platform::OpenRawFile(const char* path, FileMode mode)
{
    switch (mode) {
    case FileMode::Read:
        return OpenFile(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    case FileMode::Write:
        return OpenFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    case FileMode::ReadWrite:
        return OpenFile(path, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    }
    SetLastError(ERROR_INVALID_PARAMETER);
    return InvalidFileHandle();
}

bool Platform::CloseRawFile(void* handle) { return ValidRawFile(handle) && CloseHandle(handle) != 0; }
bool Platform::ReadRawFile(void* handle, void* buffer, int size, std::uint32_t& count)
{
    count = 0;
    if (!ValidRawFile(handle)) return false;
    if (size < 0) { SetLastError(ERROR_INVALID_PARAMETER); return false; }
    DWORD transferred = 0;
    const bool result = ReadFile(handle, buffer, size, &transferred, nullptr) != 0;
    count = transferred;
    return result;
}
bool Platform::WriteRawFile(void* handle, const void* buffer, int size, std::uint32_t& count)
{
    count = 0;
    if (!ValidRawFile(handle)) return false;
    if (size < 0) { SetLastError(ERROR_INVALID_PARAMETER); return false; }
    DWORD transferred = 0;
    const bool result = WriteFile(handle, buffer, size, &transferred, nullptr) != 0;
    count = transferred;
    return result;
}
int Platform::SeekRawFile(void* handle, int offset, int origin)
{
    if (!ValidRawFile(handle)) return -1;
    DWORD method;
    switch (origin) {
    case SEEK_SET: method = FILE_BEGIN; break;
    case SEEK_CUR: method = FILE_CURRENT; break;
    case SEEK_END: method = FILE_END; break;
    default: SetLastError(ERROR_INVALID_PARAMETER); return -1;
    }
    return static_cast<int>(SetFilePointer(handle, offset, nullptr, method));
}
int Platform::RawFileSize(void* handle) { return ValidRawFile(handle) ? static_cast<int>(GetFileSize(handle, nullptr)) : -1; }
std::uint32_t Platform::RawFileDateTime(void* handle)
{
    if (!ValidRawFile(handle)) return 0;
    BY_HANDLE_FILE_INFORMATION info;
    WORD date, time;
    if (!GetFileInformationByHandle(handle, &info) || !FileTimeToDosDateTime(&info.ftLastWriteTime, &date, &time)) return 0;
    return (static_cast<std::uint32_t>(date) << 16) | time;
}
bool Platform::SetRawFileDateTime(void* handle, std::uint32_t dateTime)
{
    if (!ValidRawFile(handle)) return false;
    BY_HANDLE_FILE_INFORMATION info;
    FILETIME time;
    return GetFileInformationByHandle(handle, &info) &&
        DosDateTimeToFileTime(static_cast<WORD>(dateTime >> 16), static_cast<WORD>(dateTime), &time) &&
        SetFileTime(handle, &info.ftCreationTime, &time, &time);
}
bool Platform::RemoveRawFile(const char* path) { return RemoveFile(path) != 0; }
int Platform::LastFileError() { return static_cast<int>(GetLastError()); }

bool Platform::RenameRawFile(const char* source, const char* destination) { return RenameFile(source, destination) != 0; }

bool Platform::IsReadOnlyFile(const char* path)
{
    const auto attributes = GetFileAttributesA(ReadPath(path).c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_READONLY);
}
