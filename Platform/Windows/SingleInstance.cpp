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

#include "Platform/Application.h"
#include "wwdebug.h"
#include <SDL3/SDL_error.h>
#include <windows.h>

namespace {
const char* AUTOPLAY_GUID = "01AF9993-3492-11d3-8F6F-0060089C05B1";
struct Instance {
    HANDLE AppMutex = nullptr;
    HANDLE AutoPlayMutex = nullptr;
};
}

void* Platform::AcquireApplicationInstance(const char* identifier, bool allowMultiple)
{
    auto* instance = new Instance;

	bool retval = false;

	instance->AppMutex = ::CreateMutex (NULL, FALSE, identifier);
    if (!instance->AppMutex) {
        SDL_SetError("Could not create application mutex: %lu", GetLastError());
        delete instance;
        return nullptr;
    }

	if (::GetLastError () == ERROR_ALREADY_EXISTS) {

		if (allowMultiple) {
			WWDEBUG_SAY (("Renegade is already running but AllowMultipleInstances is true\n"));
			retval = true;
		} else {
			HWND main_wnd = ::FindWindow ("SDL_app", "Renegade");
			if (main_wnd != NULL) {
				::SetForegroundWindow (main_wnd);
				::ShowWindow (main_wnd, SW_RESTORE);
			}

			WWDEBUG_SAY (("Renegade is already running...Bail!\n"));
		}
	} else {

		WWDEBUG_SAY (("Create AppMutex okay.\n"));

		do
		{
			instance->AutoPlayMutex = ::OpenMutex (MUTEX_ALL_ACCESS, FALSE, AUTOPLAY_GUID);
			if (instance->AutoPlayMutex != NULL) {
				WWDEBUG_SAY (("Waiting for Autoplay to quit!\n"));

				if (::WaitForSingleObject (instance->AutoPlayMutex, 30000) == WAIT_FAILED) {
					WWDEBUG_SAY (("Failed waiting for AutoPlayMutex\n"));
					::CloseHandle (instance->AutoPlayMutex);
					instance->AutoPlayMutex = NULL;
				}
			}

			if (instance->AutoPlayMutex == NULL) {
				instance->AutoPlayMutex = ::CreateMutex (NULL, FALSE, AUTOPLAY_GUID);

				if (::GetLastError () == ERROR_ALREADY_EXISTS) {
					::CloseHandle (instance->AutoPlayMutex);
					instance->AutoPlayMutex = NULL;
					::Sleep (2500);
				} else {
					WWDEBUG_SAY (("Create AutoPlayMutex.\n"));
				}
			}

		} while (instance->AutoPlayMutex == NULL);

		WWDEBUG_SAY (("Got AutoPlayMutex okay.\n"));
		retval = true;
	}

	if (!retval) {
        ReleaseApplicationInstance(instance);
        return nullptr;
    }
    return instance;

}

void Platform::ReleaseApplicationInstance(void* handle)
{
    auto* instance = static_cast<Instance*>(handle);
    if (!instance) return;
    if (instance->AppMutex) CloseHandle(instance->AppMutex);
    if (instance->AutoPlayMutex) CloseHandle(instance->AutoPlayMutex);
    delete instance;
}
