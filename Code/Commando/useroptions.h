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
// Filename:     useroptions.h
// Author:       Tom Spencer-Smith
// Date:         Dec 1999
// Description:
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef USEROPTIONS_H
#define USEROPTIONS_H

#include "SettingsBool.h"
#include "SettingsInt.h"
#include "SettingsFloat.h"
#include "boolean.h"
#include "SettingsString.h"
#include "bandwidth.h"


//-----------------------------------------------------------------------------
//
// Various options that the player chooses
//
class cUserOptions
{
	public:

		static bool Parse_Command_Line(int argc, char** argv);

		static void Set_Server_INI_File(char *cmd_line_entry);

		static void Set_Bandwidth_Type(BANDWIDTH_TYPE_ENUM bandwidth_type);
		static BANDWIDTH_TYPE_ENUM Get_Bandwidth_Type(void);
		static void Set_Bandwidth_Bps(int bandwidth_bbs);

		static void Reread(void);

		static cSettingsBool ShowNamesOnSoldier;
		static cSettingsBool SkipQuitConfirmDialog;
		static cSettingsBool SkipIngameQuitConfirmDialog;
		static cSettingsBool CameraLockedToTurret;
		static cSettingsBool PermitDiagLogging;

		static cSettingsInt Sku;

		static cSettingsInt BandwidthBps;
		static cSettingsInt BandwidthType;

		static cSettingsInt		GameSpyBandwidthType;
		static cSettingsInt		PreferredGameSpyNic;
		static cSettingsString	GameSpyNickname;
		static cSettingsInt		GameSpyGamePort;
		static cSettingsInt		GameSpyQueryPort;
		static cSettingsInt		SplashCount;
		static cSettingsBool	DoneClientBandwidthTest;

		static cSettingsInt PreferredLanNic;

		static cSettingsInt NetUpdateRate;
		static cSettingsFloat ClientHintFactor;
		static cSettingsFloat MaxFacingPenalty;
		static cSettingsFloat IrrelevancePenalty;

		static cSettingsInt ResultsLogNumber;

	private:
};

//-----------------------------------------------------------------------------
#endif // USEROPTIONS_H










/*
		static cSettingsInt	GameListFilterMaxPing;
		static cSettingsInt	GameListFilterMinPlayersPresent;
		static cSettingsInt	GameListFilterMaxPlayersPresent;
		static cSettingsInt	GameListFilterMaxPlayersPermitted;
		static cSettingsBool	GameListFilterShowPrivateGames;
		static cSettingsBool	GameListFilterShowOnlyDedicatedGames;
		static cSettingsBool	GameListFilterShowOnlyGamesIRankFor;
*/