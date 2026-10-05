#include "Platform/Executable.h"
#include "ffactory.h"
#include "wwfile.h"
#include <array>
#include <bit>
#include <cstring>
#include <limits>

bool Get_Image_File_Header(const char* filename, Platform::ExecutableFileHeader* header)
{
    if (!filename || !header || !_TheFileFactory) return false;
    file_auto_ptr file(_TheFileFactory, filename);
    if (!file.get() || !file->Open()) return false;
    std::array<unsigned char, 64> dos{};
    if (file->Read(dos.data(), dos.size()) != dos.size() || dos[0] != 'M' || dos[1] != 'Z') return false;
    std::uint32_t offset = 0;
    std::memcpy(&offset, dos.data() + 60, sizeof(offset));
    if constexpr (std::endian::native == std::endian::big) offset = std::byteswap(offset);
    constexpr int required = 4 + 20;
    const int size = file->Size();
    if (offset < dos.size() || size < required || offset > static_cast<std::uint32_t>(size - required)
        || offset > static_cast<std::uint32_t>(std::numeric_limits<int>::max())
        || file->Seek(static_cast<int>(offset), SEEK_SET) != offset) return false;
    std::array<unsigned char, required> data{};
    if (file->Read(data.data(), data.size()) != data.size()
        || data[0] != 'P' || data[1] != 'E' || data[2] || data[3]) return false;
    Platform::ExecutableFileHeader value{};
    static_assert(sizeof(value) == 20);
    std::memcpy(&value, data.data() + 4, sizeof(value));
    if constexpr (std::endian::native == std::endian::big) {
        value.Machine = std::byteswap(value.Machine);
        value.NumberOfSections = std::byteswap(value.NumberOfSections);
        value.TimeDateStamp = std::byteswap(value.TimeDateStamp);
        value.PointerToSymbolTable = std::byteswap(value.PointerToSymbolTable);
        value.NumberOfSymbols = std::byteswap(value.NumberOfSymbols);
        value.SizeOfOptionalHeader = std::byteswap(value.SizeOfOptionalHeader);
        value.Characteristics = std::byteswap(value.Characteristics);
    }
    *header = value;
    return true;
}
