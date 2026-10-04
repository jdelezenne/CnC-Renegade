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
// Filename:     devoptions.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1999
// Description:
//

#include "devoptions.h" // I WANNA BE FIRST!

#include "_globals.h"
#include "wwdebug.h"
#include "player.h"
#include "Settings.h"

//
// Class statics
//
#ifdef WWDEBUG
   cSettingsBool cDevOptions::ShowGodStatus(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowGodStatus",				true);
   cSettingsBool cDevOptions::ShowSoldierData(        APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowSoldierData",          false);
   cSettingsBool cDevOptions::ShowVehicleData(        APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowVehicleData",          false);
   cSettingsBool cDevOptions::ShowDoorData(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowDoorData",					false);
   cSettingsBool cDevOptions::ShowElevatorData(       APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowElevatorData",         false);
   cSettingsBool cDevOptions::ShowDSAPOData(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowDSAPOData",				false);
   cSettingsBool cDevOptions::ShowPowerupData(        APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowPowerupData",          false);
   cSettingsBool cDevOptions::ShowBuildingData(       APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowBuildingData",         false);
   cSettingsBool cDevOptions::ShowSpawnerData(        APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowSpawnerData",          false);
   cSettingsBool cDevOptions::ShowImportStates(       APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowImportStates",         false);
   cSettingsBool cDevOptions::ShowImportStatesSV(		APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowImportStatesSV",			false);
   cSettingsBool cDevOptions::ShowServerRhostData(    APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowServerRhostData",      false);
   cSettingsBool cDevOptions::ShowClientRhostData(    APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowClientRhostData",      false);
   //cSettingsBool cDevOptions::ShowPacketGraphs(       APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowPacketGraphs",         false);

	cSettingsBool cDevOptions::PacketsSentServer(					APPLICATION_SETTINGS_SECTION_NETDEBUG, "PacketsSentServer",					false);
	cSettingsBool cDevOptions::PacketsSentClient(					APPLICATION_SETTINGS_SECTION_NETDEBUG, "PacketsSentClient",					false);
	cSettingsBool cDevOptions::PacketsRecdServer(					APPLICATION_SETTINGS_SECTION_NETDEBUG, "PacketsRecdServer",					false);
	cSettingsBool cDevOptions::PacketsRecdClient(					APPLICATION_SETTINGS_SECTION_NETDEBUG, "PacketsRecdClient",					false);
	cSettingsBool cDevOptions::AvgSizePacketsSentServer(			APPLICATION_SETTINGS_SECTION_NETDEBUG, "AvgSizePacketsSentServer",       false);
	cSettingsBool cDevOptions::AvgSizePacketsSentClient(			APPLICATION_SETTINGS_SECTION_NETDEBUG, "AvgSizePacketsSentClient",       false);
	cSettingsBool cDevOptions::AvgSizePacketsRecdServer(			APPLICATION_SETTINGS_SECTION_NETDEBUG, "AvgSizePacketsRecdServer",       false);
	cSettingsBool cDevOptions::AvgSizePacketsRecdClient(			APPLICATION_SETTINGS_SECTION_NETDEBUG, "AvgSizePacketsRecdClient",       false);
	cSettingsBool cDevOptions::BytesSentServer(						APPLICATION_SETTINGS_SECTION_NETDEBUG, "BytesSentServer",						false);
	cSettingsBool cDevOptions::BytesSentClient(						APPLICATION_SETTINGS_SECTION_NETDEBUG, "BytesSentClient",						false);
	cSettingsBool cDevOptions::BytesRecdServer(						APPLICATION_SETTINGS_SECTION_NETDEBUG, "BytesRecdServer",						false);
	cSettingsBool cDevOptions::BytesRecdClient(						APPLICATION_SETTINGS_SECTION_NETDEBUG, "BytesRecdClient",						false);

	cSettingsBool cDevOptions::WwnetPacketsSentServer(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetPacketsSentServer",         false);
	cSettingsBool cDevOptions::WwnetPacketsSentClient(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetPacketsSentClient",         false);
	cSettingsBool cDevOptions::WwnetPacketsRecdServer(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetPacketsRecdServer",         false);
	cSettingsBool cDevOptions::WwnetPacketsRecdClient(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetPacketsRecdClient",         false);
	cSettingsBool cDevOptions::WwnetAvgSizePacketsSentServer(	APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetAvgSizePacketsSentServer",	false);
	cSettingsBool cDevOptions::WwnetAvgSizePacketsSentClient(   APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetAvgSizePacketsSentClient",  false);
	cSettingsBool cDevOptions::WwnetAvgSizePacketsRecdServer(   APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetAvgSizePacketsRecdServer",  false);
	cSettingsBool cDevOptions::WwnetAvgSizePacketsRecdClient(   APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetAvgSizePacketsRecdClient",  false);
	cSettingsBool cDevOptions::WwnetBytesSentServer(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetBytesSentServer",				 false);
	cSettingsBool cDevOptions::WwnetBytesSentClient(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetBytesSentClient",				false);
	cSettingsBool cDevOptions::WwnetBytesRecdServer(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetBytesRecdServer",				false);
	cSettingsBool cDevOptions::WwnetBytesRecdClient(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "WwnetBytesRecdClient",				false);

   cSettingsBool cDevOptions::ShowPriorities(			APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowPriorities",				false);
   cSettingsBool cDevOptions::ShowBandwidth(          APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowBandwidth",            false);
   cSettingsBool cDevOptions::ShowLatency(            APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowLatency",              false);
   cSettingsBool cDevOptions::ShowLastContact(        APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowLastContact",          false);
   cSettingsBool cDevOptions::ShowListSizes(          APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowListSizes",            false);
   cSettingsBool cDevOptions::ShowListTimes(          APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowListTimes",            false);
   cSettingsBool cDevOptions::ShowListPacketSizes(    APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowListPacketSizes",      false);
   //cSettingsBool cDevOptions::ShowBandwidthBudgetOut( APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowBandwidthBudgetOut",   false);
   cSettingsBool cDevOptions::ShowWatchList(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowWatchList",				false);
   cSettingsBool cDevOptions::ShowWolLocation(        APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowWolLocation",          false);
   cSettingsBool cDevOptions::ShowDiagnostics(			APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowDiagnostics",				false);
   cSettingsBool cDevOptions::ShowMenuStack(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowMenuStack",				false);
   cSettingsBool cDevOptions::ShowIpAddresses(			APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowIpAddresses",				false);
   cSettingsBool cDevOptions::ShowClientFps(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowClientFps",				false);
   cSettingsBool cDevOptions::ShowId(						APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowId",							false);
	cSettingsBool cDevOptions::ShowPing(					APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowPing",                 false);
   cSettingsBool cDevOptions::ShowObjectTally(			APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowObjectTally",				false);
   cSettingsBool cDevOptions::ShowInactivePlayers(		APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowInactivePlayers",		false);
	cSettingsBool cDevOptions::ShowGameSpyAuthState(	APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowGameSpyAuthState",		false);

	cSettingsInt cDevOptions::DesiredFrameSleepMs(				APPLICATION_SETTINGS_SECTION_NETDEBUG, "DesiredFrameSleepMs",				0);
	cSettingsInt cDevOptions::SimulatedPacketLossPc(			APPLICATION_SETTINGS_SECTION_NETDEBUG, "SimulatedPacketLossPc",				0);
	cSettingsInt cDevOptions::SimulatedPacketDuplicationPc(	APPLICATION_SETTINGS_SECTION_NETDEBUG, "SimulatedPacketDuplicationPc",	0);
	cSettingsInt cDevOptions::SimulatedLatencyRangeMsLower(	APPLICATION_SETTINGS_SECTION_NETDEBUG, "SimulatedLatencyRangeMsLower",	0);
	cSettingsInt cDevOptions::SimulatedLatencyRangeMsUpper(	APPLICATION_SETTINGS_SECTION_NETDEBUG, "SimulatedLatencyRangeMsUpper",	0);
	cSettingsInt cDevOptions::SpamCount(							APPLICATION_SETTINGS_SECTION_NETDEBUG, "SpamCount",								0);

	cSettingsBool cDevOptions::SoundEffectOnAssert(				APPLICATION_SETTINGS_SECTION_DEBUG,	"SoundEffectOnAssert",				false);
	cSettingsBool cDevOptions::DisplayLogfileOnAssert(			APPLICATION_SETTINGS_SECTION_DEBUG,	"DisplayLogfileOnAssert",			false);
	cSettingsBool cDevOptions::BreakToDebuggerOnAssert(		APPLICATION_SETTINGS_SECTION_DEBUG,	"BreakToDebuggerOnAssert",			true);
	cSettingsBool cDevOptions::ShutdownInputOnAssert(			APPLICATION_SETTINGS_SECTION_DEBUG,	"ShutdownInputOnAssert",			true);
	cSettingsBool cDevOptions::PreloadAssets(						APPLICATION_SETTINGS_SECTION_DEBUG,	"PreloadAssets",						true);
	cSettingsBool cDevOptions::FilterLevelFiles(					APPLICATION_SETTINGS_SECTION_DEBUG,	"FilterLevelFiles",					true);
	cSettingsBool cDevOptions::IBelieveInGod(						APPLICATION_SETTINGS_SECTION_DEBUG,	"IBelieveInGod",						false);
	cSettingsBool cDevOptions::LogDataSafe(						APPLICATION_SETTINGS_SECTION_DEBUG,	"LogDataSafe",							true);
	cSettingsBool cDevOptions::EnableExceptionHandler(			APPLICATION_SETTINGS_SECTION_DEBUG,	"ExceptionHandler",					true);
	cSettingsBool cDevOptions::ShowThumbnailPreInitDialog(	APPLICATION_SETTINGS_SECTION_DEBUG,	"ShowThumbnailPreInitDialog",		true);
	cSettingsBool cDevOptions::CrtDbgEnabled(						APPLICATION_SETTINGS_SECTION_DEBUG,	"CrtDbgEnabled",						true);
	cSettingsBool cDevOptions::PacketOptimizationsEnabled(	APPLICATION_SETTINGS_SECTION_DEBUG,	"PacketOptimizationsEnabled",		true);
	cSettingsBool cDevOptions::ShowMoney(							APPLICATION_SETTINGS_SECTION_DEBUG,	"ShowMoney",							false);
	cSettingsBool cDevOptions::ExtraNetDebug(						APPLICATION_SETTINGS_SECTION_DEBUG,	"NetDebugLog",							false);
	cSettingsBool cDevOptions::ExtraModemBandwidthThrottling(APPLICATION_SETTINGS_SECTION_DEBUG,	"ExtraModemBWThrottling",			true);

   cBoolean cDevOptions::GoToMainMenu(false);

#endif // WWDEBUG

   cBoolean cDevOptions::QuickFullExit(false);

   cSettingsBool cDevOptions::ExitThreadOnAssert(				APPLICATION_SETTINGS_SECTION_DEBUG,	"ExitThreadOnAssert",				true);
   cSettingsBool cDevOptions::CompareExeVersionOnNetwork(	APPLICATION_SETTINGS_SECTION_DEBUG,	"CompareExeVersionOnNetwork",		true);

	cSettingsBool cDevOptions::UseNewTCADO(						APPLICATION_SETTINGS_SECTION_DEBUG,	"NewTCADO",								true);
   cSettingsBool cDevOptions::ShowFps(								APPLICATION_SETTINGS_SECTION_NETDEBUG, "ShowFps",							false);








	//cSettingsBool cDevOptions::DoThumbnailPreInit(				APPLICATION_SETTINGS_SECTION_DEBUG,	"DoThumbnailPreInit",				true);