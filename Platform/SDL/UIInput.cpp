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

#include "wwuiinput.h"
#include "Platform/SDL/KeyMapping.h"
#include "Platform/SDL/UIKeys.h"
#include "dialogmgr.h"
#include <SDL3/SDL.h>
WWUIInputClass::WWUIInputClass() : mIMEManager(nullptr) {}
WWUIInputClass::~WWUIInputClass() { if (mIMEManager) mIMEManager->Release_Ref(); }
void WWUIInputClass::InitIME() {
    if (!mIMEManager) {
        mIMEManager = IME::IMEManager::Create();
        Observer<IME::UnicodeChar>::NotifyMe(*mIMEManager);
    }
}
IME::IMEManager* WWUIInputClass::GetIME() const {
    if (mIMEManager) mIMEManager->Add_Ref();
    return mIMEManager;
}
bool WWUIInputClass::ProcessSDLEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) return ProcessSDLKeyEvent(event.key);
    return mIMEManager && mIMEManager->ProcessEvent(event);
}
bool WWUIInputClass::ProcessSDLKeyEvent(const SDL_KeyboardEvent& event) {
    const unsigned key = Platform::UIKey(event.scancode);
    if (!key) return false;
    auto* state = DialogMgrClass::Get_Keyboard_State();
    state[VK_SHIFT] = (event.mod & SDL_KMOD_SHIFT) ? 0x80 : 0;
    state[VK_CONTROL] = (event.mod & SDL_KMOD_CTRL) ? 0x80 : 0;
    state[VK_MENU] = (event.mod & SDL_KMOD_ALT) ? 0x80 : 0;
    state[VK_CAPITAL] = (event.mod & SDL_KMOD_CAPS) ? 1 : 0;
    state[VK_NUMLOCK] = (event.mod & SDL_KMOD_NUM) ? 1 : 0;
    state[VK_SCROLL] = (event.mod & SDL_KMOD_SCROLL) ? 1 : 0;
    if (!event.down) return DialogMgrClass::On_Key_Up(key);
    const unsigned scan = Legacy_Key_ID(event.scancode);
    unsigned data = 1 | ((scan & 0x7f) << 16);
    if (scan & 0x80) data |= 1u << 24;
    if (event.mod & SDL_KMOD_ALT) data |= 1u << 29;
    if (event.repeat) data |= 1u << 30;
    return DialogMgrClass::On_Key_Down(key, data);
}
void WWUIInputClass::HandleNotification(IME::UnicodeChar& unicode) { DialogMgrClass::On_Unicode_Char(unicode.Subject()); }
