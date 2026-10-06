#include "datasafe.h"
#include <cstring>

namespace {
std::uint32_t Read_Word(const unsigned char* data)
{
    std::uint32_t word;
    std::memcpy(&word, data, sizeof(word));
    return word;
}
void Write_Word(unsigned char* data, std::uint32_t word)
{
    std::memcpy(data, &word, sizeof(word));
}
}

void GenericDataSafeClass::Mem_Copy_Encrypt(void* dest, void* src, int size, bool do_checksum)
{
    ds_assert((size % 4) == 0);
    const auto* source = static_cast<const unsigned char*>(src);
    auto* destination = static_cast<unsigned char*>(dest);
    for (int i = 0; i < size / 4; ++i) {
        const auto word = Read_Word(source + i * 4) ^ SimpleKey;
        if (do_checksum) Checksum ^= word;
        Write_Word(destination + i * 4, word);
    }
}

void GenericDataSafeClass::Mem_Copy_Decrypt(void* dest, void* src, int size, bool do_checksum)
{
    ds_assert((size % 4) == 0);
    const auto* source = static_cast<const unsigned char*>(src);
    auto* destination = static_cast<unsigned char*>(dest);
    for (int i = 0; i < size / 4; ++i) {
        const auto word = Read_Word(source + i * 4);
        if (do_checksum) Checksum ^= word;
        Write_Word(destination + i * 4, word ^ SimpleKey);
    }
}

void GenericDataSafeClass::Encrypt(void* data, int size, std::uint32_t key, bool do_checksum)
{
    ds_assert((size % 4) == 0);
    auto* bytes = static_cast<unsigned char*>(data);
    for (int i = 0; i < size / 4; ++i) {
        const auto word = Read_Word(bytes + i * 4) ^ key;
        if (do_checksum) Checksum ^= word;
        Write_Word(bytes + i * 4, word);
    }
}

void GenericDataSafeClass::Decrypt(void* data, int size, std::uint32_t key, bool do_checksum)
{
    ds_assert((size % 4) == 0);
    auto* bytes = static_cast<unsigned char*>(data);
    for (int i = 0; i < size / 4; ++i) {
        const auto word = Read_Word(bytes + i * 4);
        if (do_checksum) Checksum ^= word;
        Write_Word(bytes + i * 4, word ^ key);
    }
}
