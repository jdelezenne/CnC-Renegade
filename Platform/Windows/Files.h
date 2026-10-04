#pragma once
#include <winsock2.h>
#include <windows.h>

namespace Platform {
HANDLE OpenFile(const char* path, DWORD access, DWORD sharing,
    LPSECURITY_ATTRIBUTES security, DWORD creation, DWORD attributes, HANDLE templateFile);
BOOL MakeDirectory(const char* path, LPSECURITY_ATTRIBUTES security);
BOOL RemoveFile(const char* path);
BOOL RenameFile(const char* source, const char* destination);
BOOL DuplicateFile(const char* source, const char* destination, BOOL failIfExists);
}
