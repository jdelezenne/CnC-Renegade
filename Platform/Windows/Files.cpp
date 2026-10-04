#include "Platform/Windows/Files.h"
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
