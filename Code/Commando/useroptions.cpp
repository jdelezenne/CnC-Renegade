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
// Filename:     useroptions.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1999
// Description:
//

#include "useroptions.h"
#include "Platform/Application.h"
#include "Platform/Debug.h"

#include "_globals.h"
#include "wwdebug.h"
#include "player.h"
#include "cnetwork.h"
#include "Settings.h"
#include "player.h"
#include "playertype.h"
#include "bandwidth.h"
#include "bandwidthcheck.h"
#include <stdio.h>
#include <string>
#include "trim.h"
#include "singletoninstancekeeper.h"
#include "slavemaster.h"
#include "debug.h"
#include "rawfile.h"
#include "serversettings.h"
#include "autostart.h"
#include "consolemode.h"
#include "GameSpy_QnR.h"
#include "gamespyadmin.h"
#include "specialbuilds.h"
#include "useroptions.h"

extern char DefaultSettingsModifier[1024];

//
// Class statics
//
cSettingsBool cUserOptions::ShowNamesOnSoldier(					APPLICATION_SETTINGS_SECTION_NETOPTIONS, "ShowNamesOnSoldier",           true);
cSettingsBool cUserOptions::SkipQuitConfirmDialog(				APPLICATION_SETTINGS_SECTION_OPTIONS,	"SkipQuitConfirmDialog",			false);
cSettingsBool cUserOptions::SkipIngameQuitConfirmDialog(		APPLICATION_SETTINGS_SECTION_OPTIONS,	"SkipIngameQuitConfirmDialog",	false);
cSettingsBool cUserOptions::CameraLockedToTurret(				APPLICATION_SETTINGS_SECTION_OPTIONS,	"CameraLockedToTurret",				false);
cSettingsBool cUserOptions::PermitDiagLogging(					APPLICATION_SETTINGS_SECTION_OPTIONS,	"PermitDiagLogging",					true);

cSettingsInt cUserOptions::Sku(										APPLICATION_SETTINGS_SECTION,				 "SKU",									RENEGADE_BASE_SKU);

cSettingsInt cUserOptions::BandwidthType(							APPLICATION_SETTINGS_SECTION_NETOPTIONS, "BandwidthType",						BANDWIDTH_AUTO);
cSettingsInt cUserOptions::BandwidthBps(							APPLICATION_SETTINGS_SECTION_NETOPTIONS, "BandwidthBps",						33600);

cSettingsInt		cUserOptions::GameSpyBandwidthType(			APPLICATION_SETTINGS_SECTION_GAMESPY,	 "GameSpyBandwidthType",			BANDWIDTH_AUTO);
cSettingsInt		cUserOptions::PreferredGameSpyNic(			APPLICATION_SETTINGS_SECTION_GAMESPY,    "PreferredGameSpyNic",				0);
cSettingsString	cUserOptions::GameSpyNickname(				APPLICATION_SETTINGS_SECTION_GAMESPY,	 "GameSpyNickname",					"");
cSettingsInt		cUserOptions::GameSpyQueryPort(			APPLICATION_SETTINGS_SECTION_GAMESPY,    "GameSpyQueryPort",				25300);
cSettingsInt		cUserOptions::GameSpyGamePort(			APPLICATION_SETTINGS_SECTION_GAMESPY,    "GameSpyGamePort",				4848);
cSettingsInt		cUserOptions::SplashCount(			APPLICATION_SETTINGS_SECTION_GAMESPY,    "SplashCount",				0);
cSettingsBool		cUserOptions::DoneClientBandwidthTest(			APPLICATION_SETTINGS_SECTION_GAMESPY,    "DoneClientBandwidthTest",				false);


cSettingsInt cUserOptions::PreferredLanNic(						APPLICATION_SETTINGS_SECTION_NETOPTIONS, "PreferredLanNic",					0);
cSettingsInt cUserOptions::NetUpdateRate(							APPLICATION_SETTINGS_SECTION_NETOPTIONS, "NetUpdateRate",						10);
cSettingsFloat cUserOptions::ClientHintFactor(					APPLICATION_SETTINGS_SECTION_NETOPTIONS, "ClientHintFactor",					10.0f);
cSettingsFloat cUserOptions::MaxFacingPenalty(					APPLICATION_SETTINGS_SECTION_NETOPTIONS, "MaxFacingPenalty",					0.3f);
cSettingsFloat cUserOptions::IrrelevancePenalty(				APPLICATION_SETTINGS_SECTION_NETOPTIONS, "IrrelevancePenalty",				0.2f);

cSettingsInt cUserOptions::ResultsLogNumber(						APPLICATION_SETTINGS_SECTION_NETOPTIONS, "ResultsLogNumber",					1);

//-----------------------------------------------------------------------------
bool cUserOptions::Parse_Command_Line(int argc, char** argv)
{
    WWASSERT(argv != NULL);
    bool retcode = true;

	//
	// Loop through all the command line arguments.
	//

	char *cmd;
	for (int i=1 ; i<argc ; i++) {
        std::string option(argv[i]);
        cmd = strupr(option.data());

		// Look for ip override.
		if (strstr(cmd, "IP=")) {
			extern ULONG g_ip_override;
			g_ip_override = ::inet_addr(strstr(cmd, "IP=") + 3);
			continue;
		}

		// See if multiple progrram instances are allowed.
		if (strstr(cmd, "MULTI")) {
			SingletonInstanceKeeperClass::Allow_Multiple_Instances(true);
			continue;
		}

		if (strstr(cmd, "REGMOD=")) {
			strcpy(DefaultSettingsModifier, strstr(cmd, "REGMOD=") + 7);
			#ifdef WWDEBUG
			Platform::DebuggerOutput("Settings modifier on command line\n");
			#endif //WWDEBUG
			Reread();
			continue;
		}

		if (strstr(cmd, "SLAVE")) {
			SlaveMaster.Set_Slave_Mode(true);
			DebugManager::Set_Is_Slave(true);

			// Save out process ID so our master server can find us.
			char tempmod[512];
			strcpy(tempmod, DefaultSettingsModifier);
			strcpy(DefaultSettingsModifier, "");
			SettingsClass reg(APPLICATION_SETTINGS_SECTION);
			if (reg.Is_Valid()) {
				reg.Set_Int("ProcessId", Platform::ProcessId());
			}
			strcpy(DefaultSettingsModifier, tempmod);

			SettingsClass::Set_Read_Only(true);
			continue;
		}

		if (strstr(cmd, "STARTSERVER=")) {
			Set_Server_INI_File(cmd);
			continue;
		}

		if (strstr(cmd, "GAMESPYSERVER=")) {
			char server_config_file[MAX_PATH];
			strcpy(server_config_file, strstr(cmd, "GAMESPYSERVER=") + 14);
			WWDEBUG_SAY(("Set to load gamespy server settings from config file %s\n", server_config_file));
			RawFileClass file(server_config_file);
			if (file.Is_Available()) {
				ServerSettingsClass::Set_Settings_File_Name(server_config_file);

				SettingsClass settings (APPLICATION_SETTINGS_SECTION_WOLSETTINGS);
				if (settings.Is_Valid ()) {
					settings.Set_Int(AutoRestartClass::SETTING_AUTO_RESTART_FLAG, 1);
					settings.Set_Int(AutoRestartClass::SETTING_AUTO_RESTART_TYPE, 0);
				}
				cGameSpyAdmin::Set_Is_Server_Gamespy_Listed(true);
				GameSpyQnR.Enable_Reporting(true);
			}
			continue;
		}

		if (strstr(cmd, "NODX")) {
			ConsoleBox.Set_Exclusive(true);
			continue;
		}
	}

#ifndef BETACLIENT
    auto parameter = [argc, argv](const char* name) -> const char* {
        for (int i = 1; i + 1 < argc; ++i) {
            if (stricmp(argv[i], name) == 0) return argv[i + 1];
        }
        return NULL;
    };
    if (const char* address = parameter("+CONNECT")) {
        std::string ipaddr(address);
        USHORT port = 4848;
        const auto separator = ipaddr.find(':');
        if (separator != std::string::npos) {
            const int requested = atoi(ipaddr.c_str() + separator + 1);
            if (requested != 0) port = requested;
            ipaddr.resize(separator);
        }
        cGameSpyAdmin::Set_Game_Host_Ip(::inet_addr(ipaddr.c_str()));
        cGameSpyAdmin::Set_Game_Host_Port(port);
        cGameSpyAdmin::Set_Is_Launch_From_Gamespy_Requested(true);
    }
    if (const char* nickname = parameter("+NETPLAYERNAME")) {
        cUserOptions::GameSpyNickname.Set(nickname);
        cGameSpyAdmin::Set_Is_Launch_From_Gamespy_Requested(true);
    }
    const char* password = parameter("+PASSWORD");
    if (!password) password = parameter("+PASS");
    if (password) {
        WideStringClass wide_password;
        wide_password.Convert_From(password);
        cGameSpyAdmin::Set_Password_Attempt(wide_password);
    }

#endif // !BETACLIENT


	// Return true if command line options scanned OK.
	return(retcode);
}



//-----------------------------------------------------------------------------
void cUserOptions::Set_Server_INI_File(char *cmd_line_entry)
{
	char server_config_file[MAX_PATH];
	strcpy(server_config_file, strstr(cmd_line_entry, "STARTSERVER=") + 12);
	WWDEBUG_SAY(("Set to load server settings from config file %s\n", server_config_file));
	RawFileClass file(server_config_file);
	if (file.Is_Available()) {
		ServerSettingsClass::Set_Settings_File_Name(server_config_file);

		SettingsClass settings (APPLICATION_SETTINGS_SECTION_WOLSETTINGS);
		if (settings.Is_Valid ()) {
			settings.Set_Int(AutoRestartClass::SETTING_AUTO_RESTART_FLAG, 1);
			settings.Set_Int(AutoRestartClass::SETTING_AUTO_RESTART_TYPE, 1);
		}
	}
}

//-----------------------------------------------------------------------------
void cUserOptions::Set_Bandwidth_Type(BANDWIDTH_TYPE_ENUM bandwidth_type)
{
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		GameSpyBandwidthType.Set(bandwidth_type);
	} else {
		BandwidthType.Set(bandwidth_type);
	}

	if (bandwidth_type != BANDWIDTH_CUSTOM) {
		if (bandwidth_type == BANDWIDTH_AUTO && BandwidthCheckerClass::Got_Bandwidth()) {
			ULONG bps = BandwidthCheckerClass::Get_Upstream_Bandwidth();
			WWASSERT(bps > 0);
			BandwidthBps.Set(bps);
		} else {
			ULONG bps = cBandwidth::Get_Bandwidth_Bps_From_Type(bandwidth_type);
			WWASSERT(bps > 0);
			BandwidthBps.Set(bps);
		}
	}
}

//-----------------------------------------------------------------------------
BANDWIDTH_TYPE_ENUM cUserOptions::Get_Bandwidth_Type(void)
{
	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		return (BANDWIDTH_TYPE_ENUM) GameSpyBandwidthType.Get();
	} else {
		return (BANDWIDTH_TYPE_ENUM) BandwidthType.Get();
	}
}

//-----------------------------------------------------------------------------
void cUserOptions::Set_Bandwidth_Bps(int bandwidth_bps)
{
	WWASSERT(bandwidth_bps > 0);

	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		GameSpyBandwidthType.Set(BANDWIDTH_CUSTOM);
	} else {
		BandwidthType.Set(BANDWIDTH_CUSTOM);
	}

	BandwidthBps.Set(bandwidth_bps);
}




//-----------------------------------------------------------------------------
void cUserOptions::Reread(void)
{
	Sku.Set(SettingsClass(APPLICATION_SETTINGS_SECTION).Get_Int("SKU", Sku.Get()));
	BandwidthType.Set(SettingsClass(APPLICATION_SETTINGS_SECTION_NETOPTIONS).Get_Int("BandwidthType", BandwidthType.Get()));
	BandwidthBps.Set(SettingsClass(APPLICATION_SETTINGS_SECTION_NETOPTIONS).Get_Int("BandwidthBps", BandwidthBps.Get()));
	GameSpyBandwidthType.Set(SettingsClass(APPLICATION_SETTINGS_SECTION_GAMESPY).Get_Int("GameSpyBandwidthType", GameSpyBandwidthType.Get()));
}













/*
cSettingsInt cUserOptions::GameListFilterMaxPing(				APPLICATION_SETTINGS_SECTION_NETOPTIONS, "GameListFilterMaxPing",								9999);
cSettingsInt cUserOptions::GameListFilterMinPlayersPresent(	APPLICATION_SETTINGS_SECTION_NETOPTIONS, "GameListFilterMinPlayersPresent",					0);
cSettingsInt cUserOptions::GameListFilterMaxPlayersPresent(	APPLICATION_SETTINGS_SECTION_NETOPTIONS, "GameListFilterMaxPlayersPresent",					99);
cSettingsInt cUserOptions::GameListFilterMaxPlayersPermitted( APPLICATION_SETTINGS_SECTION_NETOPTIONS, "GameListFilterMaxPlayersPermitted",			99);
cSettingsBool cUserOptions::GameListFilterShowPrivateGames(	APPLICATION_SETTINGS_SECTION_NETOPTIONS, "GameListFilterShowPrivateGames",					true);
cSettingsBool cUserOptions::GameListFilterShowOnlyDedicatedGames(	APPLICATION_SETTINGS_SECTION_NETOPTIONS, "GameListFilterShowOnlyDedicatedGames",	false);
cSettingsBool cUserOptions::GameListFilterShowOnlyGamesIRankFor(	APPLICATION_SETTINGS_SECTION_NETOPTIONS, "GameListFilterShowOnlyGamesIRankFor",	false);
*/

		/*
		//
		// Gamespy client launch params.
		// All 3 must be specified.
		// Example: Renegade.exe GAMESPY_IPADDR=192.168.10.100 GAMESPY_PORT=3333 GAMESPY_NICKNAME="Bob 1234"
		//
		LPCSTR param = NULL;
		char * value = NULL;

		param = "GAMESPY_IPADDR=";
		value = ::strstr(cmd, param);
		if (value != NULL) {
			value += ::strlen(param);
			ULONG ip = ::inet_addr(value);
			cGameSpyAdmin::Set_Game_Host_Ip(ip);
			cGameSpyAdmin::Set_Is_Launch_From_Gamespy_Requested(true);
			continue;
		}

		param = "GAMESPY_PORT=";
		value = ::strstr(cmd, param);
		if (value != NULL) {
			value += ::strlen(param);
			USHORT port = (USHORT)::atol(value);
			cGameSpyAdmin::Set_Game_Host_Port(port);
			cGameSpyAdmin::Set_Is_Launch_From_Gamespy_Requested(true);
			continue;
		}

		param = "GAMESPY_NICKNAME=";
		value = ::strstr(cmd, param);
		if (value != NULL) {
			value += ::strlen(param);
			WideStringClass nickname;
			char temp[200] = "";
			char seps[]   = "\"";
			char * start_token = ::strtok(value, seps);
			if (start_token != NULL) {
				start_token++;
			}
			char * end_token = ::strtok(NULL, seps);
			if (end_token != NULL && end_token > start_token) {
				::strncpy(temp, start_token, end_token - start_token);
				temp[end_token - start_token] = 0;
			}

			nickname.Convert_From(temp);
			cGameSpyAdmin::Set_Player_Nickname(nickname);
			cGameSpyAdmin::Set_Is_Launch_From_Gamespy_Requested(true);
			continue;
		}
		*/

		/*
		char nickname[300] = "";
		::sscanf(nickname_param, "%s", nickname);
		nickname[::strlen(nickname) - 1] = ' ';
		nickname[0] = ' ';

		char nickname2[300] = "";
		::sscanf(nickname, "%s", nickname2);
		*/

		/*
		char seps[]   = "\"";
		char * start_token = ::strtok(nickname_param, seps);
		if (start_token != NULL) {
			start_token++;
		}
		char * end_token = ::strtok(NULL, seps);
		char nickname2[300] = "";
		if (end_token != NULL && end_token > start_token) {
			char nickname2[300] = "";
			::strncpy(nickname2, start_token, end_token - start_token);
			nickname2[end_token - start_token] = 0;
		}
		*/

		/*
		WideStringClass wide_nickname;
		wide_nickname.Convert_From(nickname2);
		cGameSpyAdmin::Set_Player_Nickname(wide_nickname);
		*/
