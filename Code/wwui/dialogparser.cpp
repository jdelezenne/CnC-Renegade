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

#include "dialogparser.h"
#include "translatedb.h"
#include "Platform/UI/Dialogs.h"
#include <cwchar>
#include <string>
namespace {
WideStringClass TranslateTitle(const wchar_t* title, bool preservePrefix) {
    if (!title) return WideStringClass();
    const auto* identifier = std::wcsstr(title, L"IDS_");
    if (!identifier) return WideStringClass(title);
    WideStringClass description(identifier);
    StringClass ascii;
    description.Convert_To(ascii);
    const auto* translated = TRANSLATE_BY_DESC(ascii);
    if (!preservePrefix) return WideStringClass(translated);
    std::wstring result(title, identifier);
    result += translated;
    return WideStringClass(result.c_str());
}
}
void DialogParserClass::Parse_Template(int res_id, int* dlg_width, int* dlg_height, WideStringClass* dlg_title, DynamicVectorClass<ControlDefinitionStruct>* control_list) {
    const auto* dialog = Platform::FindDialogDefinition(res_id);
    if (!dialog) return;
    *dlg_width = dialog->width;
    *dlg_height = dialog->height;
    *dlg_title = TranslateTitle(dialog->title, false);
    for (const auto& control : dialog->controls) {
        ControlDefinitionStruct definition;
        definition.id = control.id;
        definition.type = static_cast<CONTROL_TYPE>(control.type);
        definition.style = control.style;
        definition.x = control.x;
        definition.y = control.y;
        definition.cx = control.width;
        definition.cy = control.height;
        definition.title = TranslateTitle(control.title, true);
        control_list->Add(definition);
    }
}

