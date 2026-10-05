#pragma once
#include <locale>
#include <string>
#include <stdexcept>
namespace Platform {
inline int CompareUIStrings(const wchar_t* first, const wchar_t* second) {
    static const auto locale = [] {
        try { return std::locale(""); }
        catch (const std::runtime_error&) { return std::locale::classic(); }
    }();
    std::wstring left(first ? first : L""), right(second ? second : L"");
    const auto& characters = std::use_facet<std::ctype<wchar_t>>(locale);
    if (!left.empty()) characters.tolower(left.data(), left.data() + left.size());
    if (!right.empty()) characters.tolower(right.data(), right.data() + right.size());
    return std::use_facet<std::collate<wchar_t>>(locale).compare(left.data(), left.data() + left.size(), right.data(), right.data() + right.size());
}
}
