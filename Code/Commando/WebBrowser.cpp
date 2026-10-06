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

#include "WebBrowser.h"
#include "_globals.h"
#include "Settings.h"
#include "wwstring.h"
#include <SDL3/SDL.h>

#ifdef _DEBUG
bool WebBrowser::InstallPrerequisites(void)
{
    SettingsClass settings(APPLICATION_SETTINGS_SECTION_URL);
		struct URLEntry
			{
			const char* Name;
			const char* Data;
			};

		static URLEntry urls[] =
			{
			{"BattleClans", "http://renchat2.westwood.com/cgi-bin/cgiclient?ren_clan_manager&request=expand_template&Template=index.html&SKU=3072&LANGCODE=0&embedded=1"},
			{"BattleClansX", "http://renchat2.westwood.com/cgi-bin/cgiclient?ren_clan_manager&request=expand_template&Template=index.html&SKU=3072&LANGCODE=0"},
			{"Ladder", "http://renchat2.westwood.com/renegade_embedded/index.html"},
			{"LadderX", "http://renchat2.westwood.com/renegade/index.html"},
			{"NetStatus", "http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_netstatus&LANGCODE=0&SKU=3072&embedded=1"},
			{"NetStatusX", "http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_netstatus&LANGCODE=0&SKU=3072"},
			{"News", "http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_news&LANGCODE=0&SKU=3072&embedded=1"},
			{"NewsX", "http://battleclans.westwood.com/cgi-bin/cgiclient?rosetta&request=do_news&LANGCODE=0&SKU=3072"},
			{"Signup", "http://games2.westwood.com/cgi-bin/cgiclient?ren_reg2&request=expand_template&Template=newreg_menu.html&LANGCODE=0&embedded=1&SKU=3072"},
			{"SignupX", "http://games2.westwood.com/cgi-bin/cgiclient?ren_reg2&request=expand_template&Template=newreg_menu.html&LANGCODE=0"},
			{NULL, NULL}
			};

    for (int index = 0; urls[index].Name; ++index) {
        StringClass value;
        settings.Get_String(urls[index].Name, value, urls[index].Data);
        settings.Set_String(urls[index].Name, value);
    }
    return true;
}
#endif // _DEBUG

bool WebBrowser::ShowWebPage(const char* page)
{
    if (!page || !*page) return false;
    SettingsClass settings(APPLICATION_SETTINGS_SECTION_URL);
    StringClass key(page);
    key += "X";
    StringClass url;
    settings.Get_String(key, url);
    return !url.Is_Empty() && SDL_OpenURL(url);
}
