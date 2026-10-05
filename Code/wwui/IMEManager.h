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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/wwui/IMEManager.h $
*
* DESCRIPTION
*     Input Method Editor Manager for input of far east characters.
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 3 $
*     $Modtime: 1/08/02 8:38p $
*
******************************************************************************/

#pragma once
#include "refcount.h"
#include "Notify.h"
#include <string>
union SDL_Event;
namespace IME {
class IMEManager;
enum CompositionAction { COMPOSITION_INVALID = 0, COMPOSITION_TYPING, COMPOSITION_START, COMPOSITION_CHANGE, COMPOSITION_FULL, COMPOSITION_END, COMPOSITION_CANCEL, COMPOSITION_RESULT };
using CompositionEvent = TypedActionPtr<CompositionAction, IMEManager>;
class UnicodeType;
using UnicodeChar = TypedEvent<UnicodeType, wchar_t>;
class IMEManager : public RefCountClass, public Notifier<UnicodeChar>, public Notifier<CompositionEvent> {
public:
    static IMEManager* Create();
    void Activate(bool numeric = false, bool password = false);
    void Deactivate();
    bool IsActive() const { return mActive; }
    const wchar_t* GetCompositionString() const { return mCompositionString.c_str(); }
    const wchar_t* GetResultString() const { return mResultString.c_str(); }
    long GetCompositionCursorPos() const { return static_cast<long>(mCompositionCursorPos); }
    void GetTargetClause(unsigned long& start, unsigned long& end);
    void SetTextInputArea(int x, int y, int width, int height, int cursor);
    bool ProcessEvent(const SDL_Event& event);
protected:
    IMEManager() = default;
    ~IMEManager() override;
    DECLARE_NOTIFIER(UnicodeChar)
    DECLARE_NOTIFIER(CompositionEvent)
private:
    void EndComposition();
    bool mActive = false;
    bool mInComposition = false;
    std::wstring mCompositionString;
    std::wstring mResultString;
    std::size_t mCompositionCursorPos = 0;
    std::size_t mSelectionEnd = 0;
};
}
