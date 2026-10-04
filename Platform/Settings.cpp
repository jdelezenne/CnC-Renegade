/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "Settings.h"
#include "ini.h"
#include "inisup.h"
#include "rawfile.h"
#include "Platform/Paths.h"
#include <cstring>
#include <cstdio>
#include <SDL3/SDL.h>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

namespace {
struct SettingsStore {
    INIClass Ini;
    std::recursive_mutex Mutex;
    bool Dirty = false;
    SettingsStore() {
        RawFileClass file(Platform::UserPath("Settings.ini").c_str());
        if (file.Is_Available()) Ini.Load(file);
    }
    void Save() {
        if (!Dirty) return;
        const std::string path = Platform::UserPath("Settings.ini");
        RawFileClass file((path + ".tmp").c_str());
        if (!file.Open(FileClass::WRITE)) return;
        if (Ini.Save(file) >= 0) {
            file.Close();
            std::error_code error;
            std::filesystem::rename(path + ".tmp", path, error);
            if (!error) Dirty = false;
        }
    }
};
SettingsStore& Store() { static SettingsStore store; return store; }
std::string NormalizeSection(const char* section) {
    std::string result(section);
    for (char& value : result) if (value == '\\') value = '/';
    return result;
}
std::string EncodeString(const char* value) {
    std::string result = "\"";
    for (const unsigned char* item = reinterpret_cast<const unsigned char*>(value ? value : ""); *item; ++item) {
        switch (*item) {
            case '\\': result += "\\\\"; break;
            case '"': result += "\\\""; break;
            case ';': result += "\\x3b"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += static_cast<char>(*item); break;
        }
    }
    return result + '"';
}
std::string DecodeString(const char* value) {
    const std::string source(value);
    if (source.size() < 2 || source.front() != '"' || source.back() != '"') return source;
    std::string result;
    for (std::size_t index = 1; index + 1 < source.size(); ++index) {
        if (source[index] == '\\' && index + 2 < source.size()) {
            switch (source[++index]) {
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'x':
                    if (index + 3 < source.size() && source.substr(index, 3) == "x3b") { result += ';'; index += 2; }
                    else { result += '\\'; result += source[index]; }
                    break;
                case '\\': case '"': result += source[index]; break;
                default: result += '\\'; result += source[index]; break;
            }
        } else result += source[index];
    }
    return result;
}
bool InSection(const char* section, const char* root) {
    const std::size_t size = std::strlen(root);
    return _strnicmp(section, root, size) == 0 &&
        (section[size] == '\0' || section[size] == '/' || section[size] == '\\');
}
}

bool SettingsClass::IsLocked = false;
SettingsClass::SettingsClass(const char* section, bool create) :
    Section(NormalizeSection(section).c_str()), IsValid(create || Exists(section)) {}
SettingsClass::~SettingsClass() {
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    store.Save();
}
bool SettingsClass::Exists(const char* section) {
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    return store.Ini.Is_Present(NormalizeSection(section).c_str());
}
int SettingsClass::Get_Int(const char* name, int value) {
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    return store.Ini.Get_Int(Section, name, value);
}
void SettingsClass::Set_Int(const char* name, int value) {
    if (IsLocked) return;
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    store.Dirty |= store.Ini.Put_Int(Section, name, value);
}
bool SettingsClass::Get_Bool(const char* name, bool value) { return Get_Int(name, value) != 0; }
void SettingsClass::Set_Bool(const char* name, bool value) { Set_Int(name, value); }
float SettingsClass::Get_Float(const char* name, float value) {
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    return store.Ini.Get_Float(Section, name, value);
}
void SettingsClass::Set_Float(const char* name, float value) {
    if (IsLocked) return;
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    store.Dirty |= store.Ini.Put_Float(Section, name, value);
}
char* SettingsClass::Get_String(const char* name, char* value, int size, const char* fallback) {
    if (size <= 0) return value;
    StringClass text;
    Get_String(name, text, fallback);
    std::strncpy(value, text.Peek_Buffer(), size - 1);
    value[size - 1] = '\0';
    return value;
}
void SettingsClass::Get_String(const char* name, StringClass& value, const char* fallback) {
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    if (!store.Ini.Is_Present(Section, name)) { value = fallback ? fallback : ""; return; }
    StringClass text;
    store.Ini.Get_String(text, Section, name);
    value = DecodeString(text.Peek_Buffer()).c_str();
}
void SettingsClass::Set_String(const char* name, const char* value) {
    if (IsLocked) return;
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    store.Dirty |= store.Ini.Put_String(Section, name, EncodeString(value).c_str());
}
void SettingsClass::Get_String(const WCHAR* name, WideStringClass& value, const WCHAR* fallback) {
    char* key = SDL_iconv_wchar_utf8(name);
    char* defaultValue = SDL_iconv_wchar_utf8(fallback ? fallback : L"");
    StringClass text;
    Get_String(key, text, defaultValue);
    wchar_t* wide = reinterpret_cast<wchar_t*>(SDL_iconv_string("WCHAR_T", "UTF-8", text.Peek_Buffer(), std::strlen(text.Peek_Buffer()) + 1));
    value = wide ? wide : L"";
    SDL_free(wide);
    SDL_free(defaultValue);
    SDL_free(key);
}
void SettingsClass::Set_String(const WCHAR* name, const WCHAR* value) {
    char* key = SDL_iconv_wchar_utf8(name);
    char* text = SDL_iconv_wchar_utf8(value);
    Set_String(key, text);
    SDL_free(text);
    SDL_free(key);
}
int SettingsClass::Get_Bin_Size(const char* name) {
    StringClass value;
    Get_String(name, value);
    return value.Get_Length() / 2;
}
void SettingsClass::Get_Bin(const char* name, void* buffer, int size) {
    StringClass text;
    Get_String(name, text);
    const char* value = text;
    auto* bytes = static_cast<unsigned char*>(buffer);
    for (int index = 0; index < size && index * 2 + 1 < text.Get_Length(); ++index) {
        unsigned byte = 0;
        if (std::sscanf(value + index * 2, "%2x", &byte) != 1) break;
        bytes[index] = static_cast<unsigned char>(byte);
    }
}
void SettingsClass::Set_Bin(const char* name, const void* buffer, int size) {
    const auto* bytes = static_cast<const unsigned char*>(buffer);
    constexpr char hex[] = "0123456789abcdef";
    std::string text;
    for (int index = 0; index < size; ++index) {
        text += hex[bytes[index] >> 4];
        text += hex[bytes[index] & 15];
    }
    Set_String(name, text.c_str());
}
void SettingsClass::Get_Value_List(DynamicVectorClass<StringClass>& list) {
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    for (int index = 0; index < store.Ini.Entry_Count(Section); ++index)
        list.Add(store.Ini.Get_Entry(Section, index));
}
void SettingsClass::Delete_Value(const char* name) {
    if (IsLocked) return;
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    store.Dirty |= store.Ini.Clear(Section, name);
}
void SettingsClass::Delete_All_Values() {
    if (IsLocked) return;
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    store.Dirty |= store.Ini.Clear(Section);
}
void SettingsClass::Delete_Settings_Tree(char* root) {
    if (IsLocked) return;
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    std::vector<std::string> sections;
    for (INISection* section = store.Ini.Get_Section_List().First(); section; section = section->Next_Valid())
        if (InSection(section->Section, root)) sections.emplace_back(section->Section);
    for (const auto& section : sections) store.Dirty |= store.Ini.Clear(section.c_str());
    store.Save();
}
void SettingsClass::Save_Settings(const char* filename, char* root) {
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    INIClass output;
    for (INISection* section = store.Ini.Get_Section_List().First(); section; section = section->Next_Valid()) {
        if (!InSection(section->Section, root)) continue;
        for (int index = 0; index < store.Ini.Entry_Count(section->Section); ++index) {
            const char* entry = store.Ini.Get_Entry(section->Section, index);
            StringClass value;
            store.Ini.Get_String(value, section->Section, entry);
            output.Put_String(section->Section, entry, value);
        }
    }
    RawFileClass file(Platform::WritePath(filename).c_str());
    output.Save(file);
}
void SettingsClass::Load_Settings(const char* filename, char* oldRoot, char* newRoot) {
    if (IsLocked) return;
    INIClass input;
    RawFileClass file(Platform::ReadPath(filename).c_str());
    if (!file.Is_Available() || !input.Load(file)) return;
    auto& store = Store();
    std::lock_guard lock(store.Mutex);
    for (INISection* section = input.Get_Section_List().First(); section; section = section->Next_Valid()) {
        if (!InSection(section->Section, oldRoot)) continue;
        const std::string destination = std::string(newRoot) + (section->Section + std::strlen(oldRoot));
        for (int index = 0; index < input.Entry_Count(section->Section); ++index) {
            const char* entry = input.Get_Entry(section->Section, index);
            StringClass value;
            input.Get_String(value, section->Section, entry);
            store.Dirty |= store.Ini.Put_String(destination.c_str(), entry, value);
        }
    }
    store.Save();
}
