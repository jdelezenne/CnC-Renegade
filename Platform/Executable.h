#pragma once
#include <cstdint>
#ifdef _WIN32
#include "Platform/Windows/TextTypes.h"
#endif

namespace Platform {
#ifdef _WIN32
using ExecutableFileHeader = IMAGE_FILE_HEADER;
#else
struct ExecutableFileHeader {
    std::uint16_t Machine;
    std::uint16_t NumberOfSections;
    std::uint32_t TimeDateStamp;
    std::uint32_t PointerToSymbolTable;
    std::uint32_t NumberOfSymbols;
    std::uint16_t SizeOfOptionalHeader;
    std::uint16_t Characteristics;
};
#endif
}

bool Get_Image_File_Header(const char* filename, Platform::ExecutableFileHeader* header);
