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
// Filename:     devoptions.h
// Author:       Tom Spencer-Smith
// Date:         Dec 1999
// Description:
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef DEVOPTIONS_H
#define DEVOPTIONS_H

#include "SettingsBool.h"
#include "SettingsInt.h"
#include "boolean.h"

//-----------------------------------------------------------------------------
//
// Various options used for developing and testing
//
class cDevOptions
{
	public:

#ifdef WWDEBUG
   static cSettingsBool ShowGodStatus;
   static cSettingsBool ShowSoldierData;
   static cSettingsBool ShowVehicleData;
   static cSettingsBool ShowDoorData;
   static cSettingsBool ShowElevatorData;
   static cSettingsBool ShowDSAPOData;
   static cSettingsBool ShowPowerupData;
   static cSettingsBool ShowBuildingData;
   static cSettingsBool ShowSpawnerData;
   static cSettingsBool ShowImportStates;
   static cSettingsBool ShowImportStatesSV;
   static cSettingsBool ShowServerRhostData;
   static cSettingsBool ShowClientRhostData;

	//static cSettingsBool ShowPacketGraphs;

	static cSettingsBool PacketsSentServer;
	static cSettingsBool PacketsSentClient;
	static cSettingsBool PacketsRecdServer;
	static cSettingsBool PacketsRecdClient;
	static cSettingsBool AvgSizePacketsSentServer;
	static cSettingsBool AvgSizePacketsSentClient;
	static cSettingsBool AvgSizePacketsRecdServer;
	static cSettingsBool AvgSizePacketsRecdClient;
	static cSettingsBool BytesSentServer;
	static cSettingsBool BytesSentClient;
	static cSettingsBool BytesRecdServer;
	static cSettingsBool BytesRecdClient;

	static cSettingsBool WwnetPacketsSentServer;
	static cSettingsBool WwnetPacketsSentClient;
	static cSettingsBool WwnetPacketsRecdServer;
	static cSettingsBool WwnetPacketsRecdClient;
	static cSettingsBool WwnetAvgSizePacketsSentServer;
	static cSettingsBool WwnetAvgSizePacketsSentClient;
	static cSettingsBool WwnetAvgSizePacketsRecdServer;
	static cSettingsBool WwnetAvgSizePacketsRecdClient;
	static cSettingsBool WwnetBytesSentServer;
	static cSettingsBool WwnetBytesSentClient;
	static cSettingsBool WwnetBytesRecdServer;
	static cSettingsBool WwnetBytesRecdClient;

   static cSettingsBool ShowPriorities;
   static cSettingsBool ShowBandwidth;
   static cSettingsBool ShowLatency;
   static cSettingsBool ShowLastContact;
   static cSettingsBool ShowListSizes;
   static cSettingsBool ShowListTimes;
   static cSettingsBool ShowListPacketSizes;
   //static cSettingsBool ShowBandwidthBudgetOut;
   static cSettingsBool ShowWatchList;
   static cSettingsBool ShowWolLocation;
	static cSettingsBool ShowDiagnostics;
	static cSettingsBool ShowMenuStack;
	static cSettingsBool ShowIpAddresses;
	static cSettingsBool ShowClientFps;
	static cSettingsBool ShowId;
	static cSettingsBool ShowPing;
	static cSettingsBool ShowObjectTally;
	static cSettingsBool ShowInactivePlayers;
	static cSettingsBool SoundEffectOnAssert;
	static cSettingsBool DisplayLogfileOnAssert;
	static cSettingsBool BreakToDebuggerOnAssert;
	static cSettingsBool ShutdownInputOnAssert;
	static cSettingsBool PreloadAssets;
	static cSettingsBool FilterLevelFiles;
	static cSettingsBool IBelieveInGod;
	static cSettingsBool LogDataSafe;
	static cSettingsBool EnableExceptionHandler;
	static cSettingsBool ShowThumbnailPreInitDialog;
	static cSettingsBool CrtDbgEnabled;
	static cSettingsBool PacketOptimizationsEnabled;
	static cSettingsBool ShowMoney;
	static cSettingsBool ExtraNetDebug;
	static cSettingsBool ExtraModemBandwidthThrottling;
	static cSettingsBool ShowGameSpyAuthState;

   //
   // These are development conveniences for starting a client or server,
   // either from a main menu keypress or from a command line param.
	//

	static cSettingsInt DesiredFrameSleepMs;
	static cSettingsInt SimulatedPacketLossPc;
	static cSettingsInt SimulatedPacketDuplicationPc;
	static cSettingsInt SimulatedLatencyRangeMsLower;
	static cSettingsInt SimulatedLatencyRangeMsUpper;
	static cSettingsInt SpamCount;

	//
	// GoToMainMenu uses QuickFullExit but stops at the main menu.
	//
	static cBoolean GoToMainMenu;

#endif WWDEBUG

   //
   // QuickFullExit is a quick but hopefully clean way to leave combat (via
   // a keypress), and stop execution.
   // If running as a server, clients will be instructed to exit also (debug only).
   //
   static cBoolean QuickFullExit;

	static cSettingsBool ExitThreadOnAssert;
	static cSettingsBool CompareExeVersionOnNetwork;
   static cSettingsBool ShowFps;

	// TEMP. ST - 12/10/2001 3:39PM
	static cSettingsBool UseNewTCADO;

   private:

};

//-----------------------------------------------------------------------------
#endif // DEVOPTIONS_H












	//static cSettingsBool DoThumbnailPreInit;