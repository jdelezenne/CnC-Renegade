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

//
// Filename:     SettingsString.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:
//

#include "SettingsString.h" // I WANNA BE FIRST!

#include "string.h"
#include "Settings.h"
#include "wwdebug.h"

//
// Class statics
//

//-----------------------------------------------------------------------------
cSettingsString::cSettingsString(LPCSTR settings_section, LPCSTR key_name,
	LPCSTR default_value)
{
   WWASSERT(default_value != NULL);

   if (settings_section == NULL) {
      strcpy(SettingsSection, "");
      strcpy(KeyName, "");
      Set(default_value);
   } else {
      WWASSERT(strlen(settings_section) < sizeof(SettingsSection));
      WWASSERT(key_name != NULL);
      WWASSERT(strlen(key_name) < sizeof(KeyName));
      strcpy(SettingsSection, settings_section);
      strcpy(KeyName, key_name);

	   SettingsClass * settings = new SettingsClass(SettingsSection);
	   WWASSERT(settings != NULL && settings->Is_Valid());
		settings->Get_String(KeyName, Value, sizeof(Value), default_value);
	delete settings;

      Set(Value);
   }
}

//-----------------------------------------------------------------------------
void cSettingsString::Set(LPCSTR value)
{
   WWASSERT(value != NULL);
   WWASSERT(strlen(value) < sizeof(Value));

   strcpy(Value, value);

   if (strcmp(SettingsSection, "")) {
	   SettingsClass * settings = new SettingsClass(SettingsSection);
	   WWASSERT(settings != NULL && settings->Is_Valid());
		settings->Set_String(KeyName, Value);
	delete settings;
   }
}
