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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/mpsettingsmgr.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/06/02 2:58p                                               $*
 *                                                                                             *
 *                    $Revision:: 23                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "mpsettingsmgr.h"
#include "Settings.h"
#include "bittype.h"
#include "_globals.h"
#include "wwonline\\wolsession.h"
#include <WWOnline\WOLLangCodes.h>
#include "time.h"


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
StringClass								MPSettingsMgrClass::SelectedLogin;
StringClass								MPSettingsMgrClass::LastLogin;
StringClass								MPSettingsMgrClass::AutoLogin;
StringClass								MPSettingsMgrClass::AutoPassword;
bool										MPSettingsMgrClass::HasMOTDBeenViewed				= false;
bool										MPSettingsMgrClass::DisplaySidebarHelp				= true;
bool										MPSettingsMgrClass::IsAutoLoginPromptEnabled		= true;
bool										MPSettingsMgrClass::AreSkinsUnlocked				= false;

#ifdef OBSOLETE
MPSettingsMgrClass::GameModeMap	MPSettingsMgrClass::_mModePrefs;
#endif

int										MPSettingsMgrClass::OptionFlags	= OPTION_DEFAULTS;


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
static const int MAX_PERSONA_LEN			= 64;


////////////////////////////////////////////////////////////////////////////////////////////////
//	Settings key names
////////////////////////////////////////////////////////////////////////////////////////////////
//static const char *QUICKMATCH_SUB_KEY	= APPLICATION_SETTINGS_SECTION_WOLSETTINGS "\\QuickMatch";



////////////////////////////////////////////////////////////////////////////////////////////////
//	Settings value names
////////////////////////////////////////////////////////////////////////////////////////////////
static const char *SETTING_LAST_LOGIN			= "LastLogin";
static const char *SETTING_AUTOLOGIN				= "AutoLogin";
static const char *SETTING_AUTOPASSWORD			= "AutoPassword";
static const char *SETTING_SIDEBAR_HELP			= "SidebarHelp";
static const char *SETTING_AUTOLOGIN_PROMPT	= "AutoLoginPrompt";
static const char *SETTING_OPTIONS				= "Options";
static const char *SETTING_ARE_SKINS_UNLOCKED	= "PrimeSocket";


////////////////////////////////////////////////////////////////
//
//	Load_Settings
//
////////////////////////////////////////////////////////////////
void
MPSettingsMgrClass::Load_Settings (void)
{
	//
	//	Attempt to open the MP settings sub-key
	//
	SettingsClass settings (APPLICATION_SETTINGS_SECTION_WOLSETTINGS, false);
	if (settings.Is_Valid ()) {

		//
		//	Read the simple data from the settings
		//
		settings.Get_String(SETTING_LAST_LOGIN, LastLogin.Get_Buffer(MAX_PERSONA_LEN), MAX_PERSONA_LEN, "");
		settings.Get_String(SETTING_AUTOLOGIN, AutoLogin.Get_Buffer(MAX_PERSONA_LEN), MAX_PERSONA_LEN, "");
		settings.Get_String(SETTING_AUTOPASSWORD, AutoPassword.Get_Buffer(MAX_PERSONA_LEN), MAX_PERSONA_LEN, "");

		DisplaySidebarHelp			= settings.Get_Bool (SETTING_SIDEBAR_HELP, true);
		IsAutoLoginPromptEnabled	= settings.Get_Bool (SETTING_AUTOLOGIN_PROMPT, true);
		AreSkinsUnlocked				= settings.Get_Bool (SETTING_ARE_SKINS_UNLOCKED, false);

		// The default options are language specific
		int defaultOptions = OPTION_DEFAULTS;

		SettingsClass skuReg(APPLICATION_SETTINGS_SECTION, false);

		if (skuReg.Is_Valid()) {
			unsigned long sku = skuReg.Get_Int("SKU", RENEGADE_BASE_SKU);
			unsigned long lang = (sku & 0xFF);

			// If this is not an Asian language region then use the Western defaults
			if ((WWOnline::LANGCODE_JAPANESE != lang) && (WWOnline::LANGCODE_KOREAN != lang)
					&& (WWOnline::LANGCODE_CHINESE != lang)) {

				defaultOptions = OPTION_DEFAULTS_LATIN;
			}
		}

		OptionFlags = settings.Get_Int (SETTING_OPTIONS, defaultOptions);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Last_Logon
//
////////////////////////////////////////////////////////////////
void
MPSettingsMgrClass::Set_Last_Login(const char *name)
{
	LastLogin = name;

	SettingsClass settings(APPLICATION_SETTINGS_SECTION_WOLSETTINGS);

	if (settings.Is_Valid()) {
		settings.Set_String(SETTING_LAST_LOGIN, LastLogin);
	}
}


////////////////////////////////////////////////////////////////
//
//	Get_Last_Logon
//
////////////////////////////////////////////////////////////////
const char*
MPSettingsMgrClass::Get_Last_Login(void)
{
	return (const char*)LastLogin;
}


////////////////////////////////////////////////////////////////
//
//	Set_Auto_Login
//
////////////////////////////////////////////////////////////////
void
MPSettingsMgrClass::Set_Auto_Login(const char* login)
{
	AutoLogin = login;

	SettingsClass settings(APPLICATION_SETTINGS_SECTION_WOLSETTINGS);

	if (settings.Is_Valid()) {
		settings.Set_String(SETTING_AUTOLOGIN, AutoLogin);
	}
}


////////////////////////////////////////////////////////////////
//
//	Get_Auto_Login
//
////////////////////////////////////////////////////////////////
const char*
MPSettingsMgrClass::Get_Auto_Login(void)
{
	return (const char*)AutoLogin;
}



////////////////////////////////////////////////////////////////
//
//	Set_Auto_Password
//
////////////////////////////////////////////////////////////////
void
MPSettingsMgrClass::Set_Auto_Password(const char* pass)
{
	AutoPassword = pass;

	SettingsClass settings(APPLICATION_SETTINGS_SECTION_WOLSETTINGS);

	if (settings.Is_Valid()) {
		settings.Set_String(SETTING_AUTOPASSWORD, AutoPassword);
	}
}

////////////////////////////////////////////////////////////////
//
//	Get_Auto_Password
//
////////////////////////////////////////////////////////////////
const char*
MPSettingsMgrClass::Get_Auto_Password(void)
{
	return (const char*)AutoPassword;
}



#ifdef OBSOLETE
////////////////////////////////////////////////////////////////
//
//	Get_QuickMatch_Mode_Preference
//
////////////////////////////////////////////////////////////////
int
MPSettingsMgrClass::Get_QuickMatch_Mode_Preference (const char *mode)
{
	GameModeMap::iterator modePref = _mModePrefs.find(mode);

	if (modePref == _mModePrefs.end()) {
		SettingsClass settings(APPLICATION_SETTINGS_SECTION_QUICKMATCH);

		if (settings.Is_Valid()) {
			int pref = settings.Get_Int(mode, 10);
			_mModePrefs[mode] = pref;
			return pref;
		}
	}

	return (*modePref).second;
}


////////////////////////////////////////////////////////////////
//
//	Set_QuickMatch_Mode_Preference
//
////////////////////////////////////////////////////////////////
void
MPSettingsMgrClass::Set_QuickMatch_Mode_Preference (const char *mode, int preference)
{
	_mModePrefs[mode] = preference;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Save_QuickMatch_Mode_Preferences
//
////////////////////////////////////////////////////////////////
void
MPSettingsMgrClass::Save_QuickMatch_Mode_Preferences (void)
{
	SettingsClass settings(APPLICATION_SETTINGS_SECTION_QUICKMATCH);

	if (settings.Is_Valid()) {
		GameModeMap::iterator iter = _mModePrefs.begin();

		while (iter != _mModePrefs.end()) {
			settings.Set_Int((*iter).first, (*iter).second);
			iter++;
		}
	}
}
#endif // OBSOLETE

////////////////////////////////////////////////////////////////
//
//	Set_Is_Sidebar_Help_Displayed
//
////////////////////////////////////////////////////////////////
void
MPSettingsMgrClass::Set_Is_Sidebar_Help_Displayed (bool onoff)
{
	DisplaySidebarHelp = onoff;

	//
	//	Attempt to open the MP settings sub-key
	//
	SettingsClass settings (APPLICATION_SETTINGS_SECTION_WOLSETTINGS);
	if (settings.Is_Valid ()) {
		settings.Set_Bool (SETTING_SIDEBAR_HELP, DisplaySidebarHelp);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Enable_Auto_Login_Prompt
//
////////////////////////////////////////////////////////////////
void
MPSettingsMgrClass::Enable_Auto_Login_Prompt (bool onoff)
{
	IsAutoLoginPromptEnabled = onoff;

	//
	//	Attempt to open the MP settings sub-key
	//
	SettingsClass settings (APPLICATION_SETTINGS_SECTION_WOLSETTINGS);
	if (settings.Is_Valid ()) {
		settings.Set_Bool (SETTING_AUTOLOGIN_PROMPT, IsAutoLoginPromptEnabled);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Option_Flag
//
////////////////////////////////////////////////////////////////
void
MPSettingsMgrClass::Set_Option_Flag (OPTION flag, bool onoff)
{
	if (onoff) {
		OptionFlags |= flag;
	} else {
		OptionFlags &= (~flag);
	}

	//
	//	Save this setting in the settings
	//
	SettingsClass settings (APPLICATION_SETTINGS_SECTION_WOLSETTINGS);
	if (settings.Is_Valid ()) {
		settings.Set_Int (SETTING_OPTIONS, OptionFlags);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Are_Alternate_Skins_Unlocked
//
////////////////////////////////////////////////////////////////
bool
MPSettingsMgrClass::Are_Alternate_Skins_Unlocked (void)
{
	if (AreSkinsUnlocked == false) {
		
		//
		//	Do we have a valid WOL session to query?
		//
		RefPtr<WWOnline::Session> wol_session = WWOnline::Session::GetInstance ();
		if (wol_session.IsValid ()) {

			//
			//	First check the insider status of the current user.  If that faild, then
			// check the server time to see if its past the waiting period.
			//
			AreSkinsUnlocked = wol_session->IsCurrUserInsider ();
			if (AreSkinsUnlocked == false) {
				
				//
				//	First, build a time structure representing when the skins are unlocked for everybody
				//
				struct tm expiration_time_struct = { 0 };
				expiration_time_struct.tm_year	= 102;
				expiration_time_struct.tm_mon		= 2;
				expiration_time_struct.tm_mday	= 15;
				time_t expire_time = ::mktime (&expiration_time_struct);

				//
				//	Now, do a simple check to see if time has expired
				//
				AreSkinsUnlocked = (wol_session->GetServerTime () >= expire_time);
			}

			//
			//	If we've found out from WOL that the skins are unlocked, then cache
			// this information in the settings.
			//
			if (AreSkinsUnlocked) {
				
				//
				//	Save this setting in the settings
				//
				SettingsClass settings (APPLICATION_SETTINGS_SECTION_WOLSETTINGS);
				if (settings.Is_Valid ()) {
					settings.Set_Bool (SETTING_ARE_SKINS_UNLOCKED, AreSkinsUnlocked);
				}
			}

		} else {

			//
			//	First, build a time structure representing when the skins are unlocked for everybody
			//
			struct tm expiration_time_struct = { 0 };
			expiration_time_struct.tm_year	= 102;
			expiration_time_struct.tm_mon		= 2;
			expiration_time_struct.tm_mday	= 15;
			time_t expire_time = ::mktime (&expiration_time_struct);

			//
			//	Now, simply check to see if time has expired
			//
			time_t curr_time;
			AreSkinsUnlocked = (::time (&curr_time) >= expire_time);
		}
	}	

	return AreSkinsUnlocked;
}
