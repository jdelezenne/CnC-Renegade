#pragma once

#include <cstdint>

namespace Platform {
enum class FileMode { Read, Write, ReadWrite };

inline void* InvalidFileHandle() { return reinterpret_cast<void*>(static_cast<std::intptr_t>(-1)); }
void* OpenRawFile(const char* path, FileMode mode);
bool CloseRawFile(void* handle);
bool ReadRawFile(void* handle, void* buffer, int size, std::uint32_t& count);
bool WriteRawFile(void* handle, const void* buffer, int size, std::uint32_t& count);
int SeekRawFile(void* handle, int offset, int origin);
int RawFileSize(void* handle);
std::uint32_t RawFileDateTime(void* handle);
bool SetRawFileDateTime(void* handle, std::uint32_t dateTime);
bool RemoveRawFile(const char* path);
bool RenameRawFile(const char* source, const char* destination);
bool IsReadOnlyFile(const char* path);
int LastFileError();
}
