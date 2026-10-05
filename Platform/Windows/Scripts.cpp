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


#include <windows.h>
#include "scriptregistrar.h"
#include "dprint.h"

__declspec(dllexport)
BOOL APIENTRY DllMain(HINSTANCE hinst, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH) {
//		DebugPrint("\n========== Script.dll loaded ==========\n");
		DebugPrint("Total registered scripts: %d\n", ScriptRegistrar::Count());

	} else if (reason == DLL_PROCESS_DETACH) {
//		DebugPrint("\n========== Script.dll Unloaded ==========\n");
	}

	return TRUE;
}


