#pragma once
#include <cstdint>
#include <span>
namespace Platform {
struct DialogControlDefinition {
    int id;
    int type;
    std::uint32_t style;
    int x, y, width, height;
    const wchar_t* title;
};
struct DialogDefinition {
    int id;
    int width, height;
    const wchar_t* title;
    std::span<const DialogControlDefinition> controls;
};
const DialogDefinition* FindDialogDefinition(int id);
}
