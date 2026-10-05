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

#include "IMEManager.h"
#include "Platform/Platform.h"
#include "Platform/UI/Input.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <string>
namespace {
std::wstring Decode(const char* text) {
    std::wstring result;
    if (!text) return result;
    while (*text) {
        const auto character = SDL_StepUTF8(&text, nullptr);
        if constexpr (sizeof(wchar_t) == 2) {
            if (character > 0xffff) {
                result += static_cast<wchar_t>(0xd800 + ((character - 0x10000) >> 10));
                result += static_cast<wchar_t>(0xdc00 + ((character - 0x10000) & 0x3ff));
                continue;
            }
        }
        result += static_cast<wchar_t>(character);
    }
    return result;
}
std::size_t WideIndex(const std::wstring& text, int characters) {
    std::size_t position = 0;
    while (position < text.size() && characters-- > 0) {
        if constexpr (sizeof(wchar_t) == 2) {
            if (text[position] >= 0xd800 && text[position] <= 0xdbff && position + 1 < text.size() && text[position + 1] >= 0xdc00 && text[position + 1] <= 0xdfff) ++position;
        }
        ++position;
    }
    return position;
}
}
IME::IMEManager* IME::IMEManager::Create() { return new IMEManager; }
IME::IMEManager::~IMEManager() { Deactivate(); }
void IME::IMEManager::Activate(bool numeric, bool password) {
    SDL_Window* window = Platform::GetWindow();
    if (!window) return;
    mActive = Platform::SetUITextInput(true, numeric, password);
}
void IME::IMEManager::Deactivate() {
    if (mActive) {
        if (auto* window = Platform::GetWindow()) { SDL_ClearComposition(window); Platform::SetUITextInput(false); }
        EndComposition();
        mActive = false;
    }
}
void IME::IMEManager::EndComposition() {
    if (!mInComposition) return;
    mCompositionString.clear();
    mCompositionCursorPos = mSelectionEnd = 0;
    mInComposition = false;
    CompositionEvent event(COMPOSITION_END, this);
    Notifier<CompositionEvent>::NotifyObservers(event);
}
void IME::IMEManager::GetTargetClause(unsigned long& start, unsigned long& end) {
    start = static_cast<unsigned long>(mCompositionCursorPos);
    end = static_cast<unsigned long>(mSelectionEnd);
}
void IME::IMEManager::SetTextInputArea(int x, int y, int width, int height, int cursor) {
    auto* window = Platform::GetWindow();
    if (!window || !mActive) return;
    int logicalWidth = 0, logicalHeight = 0, pixelWidth = 0, pixelHeight = 0;
    if (!SDL_GetWindowSize(window, &logicalWidth, &logicalHeight) || !SDL_GetWindowSizeInPixels(window, &pixelWidth, &pixelHeight) || !pixelWidth || !pixelHeight) return;
    SDL_Rect area{x * logicalWidth / pixelWidth, y * logicalHeight / pixelHeight, std::max(1, width * logicalWidth / pixelWidth), std::max(1, height * logicalHeight / pixelHeight)};
    SDL_SetTextInputArea(window, &area, cursor * logicalWidth / pixelWidth);
}
bool IME::IMEManager::ProcessEvent(const SDL_Event& event) {
    if (!mActive) return false;
    Add_Ref();
    struct Reference { IMEManager* manager; ~Reference() { manager->Release_Ref(); } } reference{this};
    if (event.type == SDL_EVENT_TEXT_EDITING) {
        if (event.edit.windowID != SDL_GetWindowID(Platform::GetWindow())) return false;
        auto text = Decode(event.edit.text);
        if (text.empty()) { EndComposition(); return true; }
        if (!mInComposition) {
            mInComposition = true;
            CompositionEvent start(COMPOSITION_START, this);
            Notifier<CompositionEvent>::NotifyObservers(start);
            if (!mActive) return true;
        }
        mCompositionString = std::move(text);
        mCompositionCursorPos = WideIndex(mCompositionString, std::max(0, event.edit.start));
        mSelectionEnd = WideIndex(mCompositionString, std::max(0, event.edit.start) + std::max(0, event.edit.length));
        CompositionEvent change(COMPOSITION_CHANGE, this);
        Notifier<CompositionEvent>::NotifyObservers(change);
        return true;
    }
    if (event.type == SDL_EVENT_TEXT_INPUT) {
        if (event.text.windowID != SDL_GetWindowID(Platform::GetWindow())) return false;
        const auto text = Decode(event.text.text);
        EndComposition();
        for (wchar_t character : text) {
            UnicodeChar unicode(character);
            Notifier<UnicodeChar>::NotifyObservers(unicode);
        }
        return true;
    }
    return false;
}
