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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Commando/GameResSend.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 25 $
*     $Modtime: 1/14/02 10:40a $
*
******************************************************************************/

#include "GameResSend.h"
#include "GameData.h"
#include "Player.h"
#include "consolemode.h"
#include <Combat/playertype.h>
#include <WWOnline/GameResPacket.h>
#include <WWOnline/WOLSession.h>
#include <WWOnline/WOLProduct.h>
#include <WWOnline/WOLUser.h>
#include <wwlib/cpudetect.h>
#include "GameVersion.h"
#include "Platform/SystemInfo.h"
#include "Platform/Calendar.h"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_time.h>
#include <cstdint>
#include <wwlib/cpudetect.h>
#include <wwlib/global.h>
#include <wwlib/md5.h>
#include <ww3d2/dx8wrapper.h>

using namespace WWOnline;

static void AddPlayerStats(GameResPacket& stats, cPlayer* player, WOL::Locale locale,
		int winnerID, bool teamGame);

/******************************************************************************
*
* NAME
*     SendGameResults
*
* DESCRIPTION
*     Send game results for ladder ranking.
*
* INPUTS
*     TheGame - The game played.
*     Players - List of players.
*
* RESULT
*     NONE
*
******************************************************************************/

void SendGameResults(unsigned long gameID, cGameData* theGame, SList<cPlayer>* playerList)
	{
		int index;
	RefPtr<WWOnline::Session> session = WWOnline::Session::GetInstance(false);

	if (!session.IsValid())
		{
		assert(session.IsValid() && "SendGameResults() WOLSession not instantiated.");
		WWDEBUG_SAY(("ERROR: SendGameResults() WOLSession not instantiated.\n"));
		return;
		}

	//---------------------------------------------------------------------------
	// Gather game information
	//---------------------------------------------------------------------------
	GameResPacket stats;

	// Unique identifier for this game session.
	stats.Add_Field("IDNO", gameID);

	// Product SKU
	RefPtr<Product> product = Product::Current();
	unsigned long gameSKU = product->GetSKU();
	stats.Add_Field("GSKU", gameSKU);

	// Version of executable.
	stats.Add_Field("VERS", static_cast<unsigned long>((REN_FILE_VERSION_MAJOR << 16) | REN_FILE_VERSION_MINOR));

	// Executable build date
	SDL_PathInfo executableInfo{};
	SDL_DateTime time{};
	const auto filename = Platform::ExecutablePath();
	if (SDL_GetPathInfo(filename.c_str(), &executableInfo)) SDL_TimeToDateTime(executableInfo.modify_time, &time, false);

	char buildDate[20];
	sprintf(buildDate, "%02d/%02d/%04d %02d:%02d:%02d",
		time.month, time.day, time.year, time.hour, time.minute, time.second);
	stats.Add_Field("DATE", buildDate);

	// Proocessor information
	stats.Add_Field("PROC", (char*)CPUDetectClass::Get_Processor_String());
	stats.Add_Field("PSPD", (unsigned long)CPUDetectClass::Get_Processor_Speed());

	// Amount of system memory on server
	stats.Add_Field("SMEM", static_cast<unsigned long>(CPUDetectClass::Get_Total_Physical_Memory()));

	// Video card information
	std::uint32_t cardInfo[4]{};
	if (ConsoleBox.Is_Exclusive()) {
		strcpy((char*)&cardInfo[0], "ConsoleMode");
	} else {
		const D3DADAPTER_IDENTIFIER8& adapter = DX8Wrapper::Get_Current_Adapter_Identifier();
		cardInfo[0] = adapter.VendorId;
		cardInfo[1] = adapter.DeviceId;
		cardInfo[2] = adapter.SubSysId;
		cardInfo[3] = adapter.Revision;
	}
	stats.Add_Field("SVID", (void*)cardInfo, sizeof(cardInfo));

	// WOL Name of server
	const WideStringClass& owner = theGame->Get_Owner();
	StringClass serverName(true);
	owner.Convert_To(serverName);
	stats.Add_Field("SNAM", (char*)serverName.Peek_Buffer());

	// Type of game played (Deathmatch, Capture the flag, CNC...)
//	char* gameMode = (char*)theGame->Get_Game_Type_Name();
	stats.Add_Field("MODE", "CNC");

	// Name of the map used.
	char *map_name = (char*)(theGame->Get_Map_Name().Peek_Buffer());
	stats.Add_Field("GMAP", map_name);

	// Was this a dedicated server?
	bool dedicatedServer = theGame->IsDedicated.Get();
	stats.Add_Field("DSVR", (char)dedicatedServer);

	// Time game started
	const Platform::CalendarTime* gameTime = theGame->Get_Game_Start_Time();
	char startTime[20];
	sprintf(startTime, "%02d/%02d/%04d %02d:%02d:%02d",
		gameTime->wMonth, gameTime->wDay, gameTime->wYear, gameTime->wHour, gameTime->wMinute, gameTime->wSecond);
	stats.Add_Field("TIME", startTime);

	// Duration of game
	unsigned long duration = theGame->Get_Duration_Seconds();
	stats.Add_Field("DURA", duration);

	// Average FPS
	unsigned long fps = theGame->Get_Frame_Count();

	if (duration > 1)
		{
		fps = (fps / duration);
		}

	stats.Add_Field("AFPS", fps);

	// Type of tournament
	/*
	char gameType = 'I';

	if (theGame->Is_Team_Game())
		{
		gameType = 'T';
		}
	*/
	char gameType = 'T';

	if (theGame->IsClanGame.Is_True())
		{
		gameType = 'C';
		}

	char ranked = (theGame->IsLaddered.Is_True() == true) ? 'Y' : 'N';

	char tournament[5];
	sprintf(tournament, "%c%c  ", gameType, ranked);
	stats.Add_Field("TRNY", tournament);

	// Include clan information
	if (theGame->IsClanGame.Is_True())
		{
		unsigned long winningClan = 0;
		unsigned long losingClan = 0;

		int winner = theGame->Get_Winner_ID();

		SLNode<cPlayer>* playerNode = playerList->Head();

		for (index = 0; index < playerList->Get_Count(); index++)
			{
			cPlayer* player = playerNode->Data();

			if (player->Is_Human())
				{
				RefPtr<UserData> user = session->FindUser(player->Get_Name());

				if (user.IsValid() && (user->GetSquadID() != 0))
					{
					int playerType = player->Get_Player_Type();

					if ((playerType == winner) && (winningClan == 0))
						{
						winningClan = user->GetSquadID();
						}
					else if (losingClan == 0)
						{
						losingClan = user->GetSquadID();
						}
					else
						{
						break;
						}
					}
				}
			}

		stats.Add_Field("CLN1", winningClan);
		stats.Add_Field("CLN2", losingClan);
		}

	//---------------------------------------------------------------------------
	// Player information
	//---------------------------------------------------------------------------

	// Determine the number of players in the game
	unsigned long numPlayers = 0;
	SLNode<cPlayer>* playerNode = playerList->Head();

	for (index = 0; index < playerList->Get_Count(); index++)
		{
		cPlayer* player = playerNode->Data();

		if (player->Is_Human())
			{
			numPlayers++;
			}
		}

	stats.Add_Field("PLRS", numPlayers);

	// Gather per-player information
	int winnerID = theGame->Get_Winner_ID();
	playerNode = playerList->Head();

	for (index = 0; index < playerList->Get_Count(); index++)
		{
		cPlayer* player = playerNode->Data();

		if (player)
			{
			const WideStringClass& playerName = player->Get_Name();

			// Player Location
			WOL::Locale locale = WOL::LOC_UNKNOWN;

			RefPtr<UserData> user = session->FindUser((const WCHAR*)playerName);

			if (user.IsValid())
				{
				locale = user->GetLocale();
				}

			AddPlayerStats(stats, player, locale, winnerID, true);
			}

		playerNode = playerNode->Next();
		}

	unsigned long packetSize = 0;
	unsigned long sig_offset = 0;
	unsigned char* packet = stats.Create_Comms_Packet(packetSize, NULL, sig_offset);

	WWDEBUG_SAY(("Sending game results packet. Size = %lu\n", packetSize));

	session->SendGameResults(packet, packetSize);
	}


/******************************************************************************
*
* NAME
*     AddPlayerStats
*
* DESCRIPTION
*
* INPUTS
*     Stats  - Game result packet to add player stats to.
*     Player - Player to add stats for.
*
* RESULT
*     NONE
*
******************************************************************************/

void AddPlayerStats(GameResPacket& stats, cPlayer* player, WOL::Locale locale,
		int winnerID, bool isTeamed)
	{
	if (player->Is_Human())
		{
		// Players WOL name
		const WideStringClass& playerName = player->Get_Name();
		char name[10];
		int len = wcstombs(name, (const WCHAR*)playerName, 10);
		name[len] = 0;

		stats.Add_Field("PNAM", name);
		stats.Add_Field("PLOC", (unsigned long)locale);

		int playerType = player->Get_Player_Type();

		// Team (bit 31:win/lose, bits 7-0:team (0 = none, 1 = GDI, 2= NOD)
		unsigned long team = 0;

		if (isTeamed)
			{
			if (playerType == PLAYERTYPE_GDI)
				{
				team = 1;
				}
			else if (playerType == PLAYERTYPE_NOD)
				{
				team = 2;
				}
			}

		if ((isTeamed && playerType == winnerID) || (!isTeamed && player->Get_Id() == winnerID))
			{
			team |= 0x80000000;
			}

		stats.Add_Field("TEAM", team);

		// Score and other information
		unsigned long score = (unsigned long)max<int>(player->Get_Score(), 0);
		stats.Add_Field("PSCR", score);

		stats.Add_Field("PPTS",	(long)player->Get_Ladder_Points());
		stats.Add_Field("PTIM", (unsigned long)player->Get_Game_Time());
		stats.Add_Field("PHLT", (unsigned long)player->Get_Final_Health());
		stats.Add_Field("PKIL", (unsigned long)player->Get_Deaths());
		stats.Add_Field("EKIL", (unsigned long)player->Get_Enemies_Killed());
		stats.Add_Field("AKIL", (unsigned long)player->Get_Allies_Killed());
		stats.Add_Field("SHOT", (unsigned long)player->Get_Shots_Fired());
		stats.Add_Field("HEDF", (unsigned long)player->Get_Head_Shots());
		stats.Add_Field("TORF", (unsigned long)player->Get_Torso_Shots());
		stats.Add_Field("ARMF", (unsigned long)player->Get_Arm_Shots());
		stats.Add_Field("LEGF", (unsigned long)player->Get_Leg_Shots());
		stats.Add_Field("CRTF", (unsigned long)player->Get_Crotch_Shots());
		stats.Add_Field("PUPS", (unsigned long)player->Get_Powerups_Collected());
		stats.Add_Field("VKIL", (unsigned long)player->Get_Vehiclies_Destroyed());
		stats.Add_Field("VTIM", (unsigned long)player->Get_Vehicle_Time());
		stats.Add_Field("NKFV", (unsigned long)player->Get_Kills_From_Vehicle());
		stats.Add_Field("SQUI", (unsigned long)player->Get_Squishes());
		stats.Add_Field("PCRD", (unsigned long)player->Get_Credit_Grant());
		stats.Add_Field("BKIL", (unsigned long)player->Get_Building_Destroyed());
		stats.Add_Field("HEDR", (unsigned long)player->Get_Head_Hit());
		stats.Add_Field("TORR", (unsigned long)player->Get_Torso_Hit());
		stats.Add_Field("ARMR", (unsigned long)player->Get_Arm_Hit());
		stats.Add_Field("LEGR", (unsigned long)player->Get_Leg_Hit());
		stats.Add_Field("CRTR", (unsigned long)player->Get_Crotch_Hit());
		stats.Add_Field("FLGC", (unsigned long)0);//no more CTF! (unsigned long)player->Get_Flag_Caps());

		// Weapon usage
		int numWeapons = min<int>(255, player->Get_Weapon_Fired_Count());

		for (int wepIndex = 0; wepIndex < numWeapons; wepIndex++)
			{
			unsigned long weapon = 0, fired = 0;
			player->Get_Weapon_Fired(wepIndex, weapon, fired);
			std::uint32_t weaponInfo[2] = {static_cast<std::uint32_t>(weapon), static_cast<std::uint32_t>(fired)};

			char token[5];
			sprintf(token, "WP%02X", wepIndex);
			stats.Add_Field(token, (void*)weaponInfo, sizeof(weaponInfo));
			}
		}
	}
