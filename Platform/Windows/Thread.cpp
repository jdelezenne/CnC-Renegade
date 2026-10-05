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

#include "Platform/Threads.h"
#include "Except.h"
#include <windows.h>

void Platform::RunThreadFunction(void (*function)(void*), void* context, ThreadExceptionHandler handler,
    const char* name, std::uint32_t identifier)
{
    Register_Thread_ID(identifier, name);
    if (handler) {
        __try { function(context); }
        __except(handler(GetExceptionCode(), GetExceptionInformation())) {}
    } else {
        function(context);
    }
    Unregister_Thread_ID(identifier, name);
}
