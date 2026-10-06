#pragma once
#include <cstdio>
#include <string>

namespace Platform {
const std::string& PreferenceDirectory();
std::string UserPath(const char* relativePath);
std::string ReadPath(const char* path);
std::string WritePath(const char* path);
bool HasRootPath(const char* path);
std::string FileName(const char* path);
std::string FileStem(const char* path);
std::string FileExtension(const char* path);
std::FILE* OpenStream(const char* path, const char* mode);
bool CreateUserDirectory(const char* path);

}
