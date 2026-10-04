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
// Filename:     SettingsInt.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:
//

#include "SettingsInt.h" // I WANNA BE FIRST!

#include "string.h"
#include "Settings.h"
#include "wwdebug.h"
#include "wwmemlog.h"

//
// Class statics
//

//-----------------------------------------------------------------------------
cSettingsInt::cSettingsInt(LPCSTR settings_section, LPCSTR key_name, int default_value)
{
   WWMEMLOG(MEM_GAMEDATA);
	if (settings_section == NULL) {
      strcpy(SettingsSection, "");
      strcpy(KeyName, "");
      Set(default_value);
   } else {
      WWASSERT(key_name != NULL);
      WWASSERT(strlen(settings_section) < sizeof(SettingsSection));
      WWASSERT(strlen(key_name) < sizeof(KeyName));
      strcpy(SettingsSection, settings_section);
      strcpy(KeyName, key_name);

	   SettingsClass * settings = new SettingsClass(SettingsSection);
	   WWASSERT(settings != NULL && settings->Is_Valid());
      Value = settings->Get_Int(KeyName, default_value);
	delete settings;

      Set(Value);
   }
}

//-----------------------------------------------------------------------------
void cSettingsInt::Set(int value)
{
   Value = value;

   if (strcmp(SettingsSection, "")) {
	   SettingsClass * settings = new SettingsClass(SettingsSection);
	   WWASSERT(settings != NULL && settings->Is_Valid());
      settings->Set_Int(KeyName, Value);
	delete settings;
   }
}
