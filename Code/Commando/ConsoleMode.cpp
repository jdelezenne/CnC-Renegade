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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/ConsoleMode.cpp                     $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/21/03 11:09a                                              $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "Platform/Paths.h"
#include "Platform/Files.h"
#include "Platform/Directory.h"
#include "Platform/Calendar.h"
#include "Platform/Terminal.h"
#include <SDL3/SDL_time.h>
#include <limits>
#include "consolemode.h"
#include "consolefunction.h"
#include "wwdebug.h"

#include "slavemaster.h"
#include <stdio.h>
#include "systimer.h"
#include "widestring.h"
#include "vector3.h"
#include "cnetwork.h"
#include "textdisplay.h"
#include "console.h"
#include "crc.h"
#include "buildnum.h"
#include "init.h"
#include "gamesideservercontrol.h"
#include "specialbuilds.h"
#include "serversettings.h"

/*
** Single instance of console.
*/
ConsoleModeClass ConsoleBox;

/*
** Console title bar text
*/
#define MASTER_TITLE_BASE "Renegade Master Server"
#define SLAVE_TITLE_BASE "Renegade Slave Server"

#define MASTER_COLORS (Platform::TerminalGreen | Platform::TerminalBlue | Platform::TerminalBright | Platform::TerminalBackgroundBlue)
#define SLAVE_COLORS	 (Platform::TerminalBackgroundGreen | Platform::TerminalBackgroundRed | Platform::TerminalBackgroundBlue)

/***********************************************************************************************
 * ConsoleModeClass::ConsoleModeClass -- Class constructor                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:00PM ST : Created                                                            *
 *=============================================================================================*/
ConsoleModeClass::ConsoleModeClass(void)
{
	TerminalOpen = false;
	LastKeypressTime = 0;
	Pos = 1;
	IsExclusive = false;
	LastProfileCRC = 0;
	LastProfilePrint = 0;
	ProfileMode = false;
}


/***********************************************************************************************
 * ConsoleModeClass::~ConsoleModeClass -- Class destructor                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:01PM ST : Created                                                            *
 *=============================================================================================*/
ConsoleModeClass::~ConsoleModeClass(void)
{
	if (TerminalOpen) Platform::CloseTerminal();
}


/***********************************************************************************************
 * ConsoleModeClass::Init -- Enable console mode - start up a console                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:01PM ST : Created                                                            *
 *=============================================================================================*/
void ConsoleModeClass::Init(void)
{
	if (TerminalOpen) return;
	TerminalOpen = Platform::OpenTerminal(SlaveMaster.Am_I_Slave() ? SLAVE_COLORS : MASTER_COLORS);
	if (!TerminalOpen) return;
	Set_Title(NULL, NULL);
	Platform::RaiseTerminal();
	unsigned long version_major = 1, version_minor = 0;
	Get_Version_Number(&version_major, &version_minor);
#ifdef FREEDEDICATEDSERVER
	Print("Renegade Free Dedicated Server ");
#else
	Print("Renegade ");
#endif
	Print("v%lu.%.3lu %s-%s %s\n", version_major >> 16, version_major & 0xFFFF, BuildInfoClass::Get_Builder_Initials(), BuildInfoClass::Get_Build_Number_String(), BuildInfoClass::Get_Build_Date_String());
	Print("Console mode active\n");
	LastKeypressTime = 0;
}





/***********************************************************************************************
 * ConsoleModeClass::Get_Slave_Window_By_Title -- Look for a slave window                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Login name of slave                                                               *
 *           Settings file name of slave                                                       *
 *                                                                                             *
 * OUTPUT:   HWND of slave window                                                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/4/2002 1:21PM ST : Created                                                              *
 *=============================================================================================*/
void* ConsoleModeClass::Get_Slave_Window_By_Title(char *name, char *settings)
{
	StringClass title = Compose_Window_Title(name, settings, true);
	const auto window = Platform::FindTerminalWindow(title.Peek_Buffer());
	return(window);
}


/***********************************************************************************************
 * ConsoleModeClass::Compose_Window_Title -- Build a window title string from name and settings*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Login name                                                                        *
 *           Settings file name                                                                *
 *                                                                                             *
 * OUTPUT:   StringClass containing full title bar string                                      *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/4/2002 1:19PM ST : Created                                                              *
 *=============================================================================================*/
StringClass ConsoleModeClass::Compose_Window_Title(char *name, char *settings, bool slave)
{
	char title[256];

	if (!slave) {
		strcpy(title, MASTER_TITLE_BASE);
	} else {
		strcpy(title, SLAVE_TITLE_BASE);
	}
	if (name) {
		strcat(title, " - ");
		strcat(title, name);
	}

	if (settings) {
		strcat(title, " - ");
		strcat(title, settings);
	}
	return(StringClass(title));
}



/***********************************************************************************************
 * ConsoleModeClass::Set_Title -- Sets the text in the console title bar                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Base text                                                                         *
 *           Name of settings file                                                             *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:03PM ST : Created                                                            *
 *=============================================================================================*/
void ConsoleModeClass::Set_Title(char *name, char *settings)
{
	if (TerminalOpen) {
		StringClass title = Compose_Window_Title(name, settings, SlaveMaster.Am_I_Slave());
		strcpy(Title, title.Peek_Buffer());
		Platform::SetTerminalTitle(Title);
	}
}




/***********************************************************************************************
 * ConsoleModeClass::Print -- Formatted print to console box                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    string                                                                            *
 *           format specifiers                                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:04PM ST : Created                                                            *
 *=============================================================================================*/
void ConsoleModeClass::Print(char const * string, ...)
{
	if (TerminalOpen) {
		char buffer[8192];

		va_list va;

		va_start(va, string);
		vsprintf(&buffer[0], string, va);
		va_end(va);

		/*
		** Have to use '%s' here or we end up doing the formatting twice.
		*/
		cprintf("%s", buffer);
		Log_To_Disk(buffer);
		//WWDEBUG_SAY((buffer));
		Apply_Attributes();
	}
}


/***********************************************************************************************
 * ConsoleModeClass::Print_Maybe -- Formatted print to console box if not busy                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    string                                                                            *
 *           format specifiers                                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:04PM ST : Created                                                            *
 *=============================================================================================*/
void ConsoleModeClass::Print_Maybe(char const * string, ...)
{
	if (string && !ProfileMode) {

		char buffer[8192];
		va_list va;
		va_start(va, string);
		vsprintf(&buffer[0], string, va);
		va_end(va);
		Log_To_Disk(buffer);

		if (Pos == 1 && (TIMEGETTIME() - LastKeypressTime > 5 * 1000) && TerminalOpen) {

			/*
			** Have to use '%s' here or we end up doing the formatting twice.
			*/
			cprintf("%s", buffer);
			//WWDEBUG_SAY((buffer));
			Apply_Attributes();
		}
	}
}


/***********************************************************************************************
 * ConsoleModeClass::Static_Print_Maybe -- Static version of Print_Maybe                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    String                                                                            *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/14/2002 11:59AM ST : Created                                                            *
 *=============================================================================================*/
void ConsoleModeClass::Static_Print_Maybe(char const * string, ...)
{
	ConsoleBox.Print_Maybe(string);
}			  



/***********************************************************************************************
 * ConsoleModeClass::cprintf -- let's get all the prints going through one place again         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    string                                                                            *
 *           format specifiers                                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:04PM ST : Created                                                            *
 *=============================================================================================*/
void ConsoleModeClass::cprintf(char const * string, ...)
{
	if (string)	{
		char buffer[8192];

		va_list va;

		buffer[sizeof(buffer)-1] = 0;
		va_start(va, string);
		_vsnprintf(&buffer[0], sizeof(buffer)-1, string, va);
		va_end(va);

		/*
		** Have to use '%s' here or we end up doing the formatting twice.
		*/
		Platform::WriteTerminal(buffer);
		GameSideServerControlClass::Print("%s", buffer);
	}
}



/***********************************************************************************************
 * ConsoleModeClass::Get_Log_File_Nmae -- Get name of log file                                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/8/2002 1:07PM ST : Created                                                              *
 *=============================================================================================*/
const char *ConsoleModeClass::Get_Log_File_Name(void)
{
	static std::string log_file_name;
	static int last_day = -1;
	Platform::CalendarTime time{};
	Platform::LocalCalendarTime(time);
	char name[64];
	snprintf(name, sizeof(name), "renlog_%d-%d-%02d.txt", time.wMonth, time.wDay, time.wYear);
	log_file_name = Platform::UserPath(name);
	const int days = ServerSettingsClass::Get_Disk_Log_Size();
	if (last_day != time.wDay && days >= 0) {
		last_day = time.wDay;
		SDL_Time now;
		constexpr SDL_Time day = 86400LL * 1'000'000'000;
		if (SDL_GetCurrentTime(&now) && days <= std::numeric_limits<SDL_Time>::max() / day &&
			now >= std::numeric_limits<SDL_Time>::min() + days * day) {
			const auto cutoff = now - days * day;
			for (const auto& file : Platform::ListFiles(Platform::UserPath("renlog_*.txt").c_str()))
				if (file.ModifiedTime < cutoff) Platform::RemoveRawFile(file.Path.c_str());
		}
	}
	return log_file_name.c_str();
}



/***********************************************************************************************
 * ConsoleModeClass::Log_To_Disk -- Log console output to disk                                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    String to log                                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/8/2002 12:47PM ST : Created                                                             *
 *=============================================================================================*/
void ConsoleModeClass::Log_To_Disk(const char *string)
{
	if (TerminalOpen) {
		if (ServerSettingsClass::Get_Disk_Log_Size() > 0) {
		FILE *log_file = Platform::OpenStream(Get_Log_File_Name(), "at");
   		if (log_file != NULL) {
				char timestr[256] = "?";
				Platform::CalendarTime time{};
				Platform::LocalCalendarTime(time);
				snprintf(timestr, sizeof(timestr), "[%02d:%02d:%02d] ", time.wHour, time.wMinute, time.wSecond);
			   fwrite(timestr, 1, strlen(timestr), log_file);
			   fwrite(string, 1, strlen(string), log_file);
			   fclose(log_file);
   		}
		}
	}
}




/***********************************************************************************************
 * ConsoleModeClass::Think -- Handles input to the console box                                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:04PM ST : Created                                                            *
 *=============================================================================================*/
void ConsoleModeClass::Think(void)
{
	static char string[256] = ">";
	int key = 0;
	static char suggestion[256] = "";
	static char last_suggestion[256] = "";
	static char help[256] = "";
	static char suggestion_stub[256];
	static unsigned long last_info_time = 0;
	static int num_players = -1;	//eh?

	static int delay = 100;

	if (TerminalOpen) {

		/*
		** See if there is a key waiting in the queue.
		*/
		if (Platform::TerminalKeyAvailable()) {

			/*
			** Get the key from the queue.
			*/
			key = Platform::ReadTerminalKey(true);
			if (key < 0) return;

			if (key == 0 || key == 0xE0) {
				key = Platform::ReadTerminalKey(true);
			}

			/*
			** Note the time of the last keypress.
			*/
			LastKeypressTime = TIMEGETTIME();

			switch (key) {

				/*
				** TAB key used to get command line suggestions.
				*/
				case 9:
				{
					string[Pos] = 0;
					if (!strlen(last_suggestion)) {
						strcpy(suggestion_stub, string+1);
					}
					ConsoleFunctionManager::Get_Command_Suggestion(suggestion_stub, last_suggestion, suggestion, help, 256);
					strcpy(last_suggestion, suggestion);

					/*
					** Save the cursor position.
					*/
					const auto cursor = Platform::SaveTerminalCursor();
					bool ok = cursor.Valid;

					/*
					** Clear out the two lines below.
					*/
					cprintf("\r\n                                                                               ");
					cprintf("\r\n                                                                               ");

					/*
					** Move the cursor back up one line to the current command prompt line.
					*/
					if (ok) {
						Platform::RestoreTerminalCursor(cursor, true);
					}

					/*
					** Print the suggestion on the line below.
					*/
					cprintf("\r\n%s\r", help);

					/*
					** Move the cursor back up one line to the current command prompt line.
					*/
					if (ok) {
						Platform::RestoreTerminalCursor(cursor, true);
					}

					/*
					** Clear out the command line and print the suggestion.
					*/
					cprintf("\r                                                                              \r>%s", suggestion);
					strcpy(string+1, suggestion);
					Pos = strlen(string);
					break;
				}

				/*
				** Handle backspace.
				*/
				case 8:
					if (Pos > 1) {
						Pos--;
						last_suggestion[0] = 0;
						cprintf(" \b");
					} else {
						/*
						** Compensate for backspace going too far.
						*/
						if (Pos == 1) {
							cprintf(">");
						}
					}
					break;

				/*
				** Handle escape.
				*/
				case 27:
					if (ProfileMode) {
						StatisticsDisplayManager::Set_Display("off");
						ProfileMode = false;
						Print("\n\n\n\n");
						Print("\n\n\n\n");
						Print("\n\n\n\n");
						Print("\n\n\n\n");
					}
					cprintf("\r                                                                              \r>");
					Pos = 1;
					string[0] = '>';
					string[1] = 0;
					break;


				/*
				** Anything else gets put into the command buffer.
				*/
				default:
					if (ProfileMode) {
						Handle_Profile_Key(key);
					} else {
						if (key == 32 || isgraph(key)) {
							string[Pos++] = key;
							last_suggestion[0] = 0;
						}
					}
					break;
			}

			/*
			** Handle user hitting enter (13).
			*/
			if (Pos > 1 && key == 13 || Pos > 200) {
				string[Pos] = 0;
				cprintf("\r\n                                                                              \r>");

				/*
				** Pass the command string to the console parser.
				*/
				ConsoleFunctionManager::Parse_Input(string+1);
				cprintf("\r\n>");
				Pos = 1;
				string[0] = '>';
				Apply_Attributes();
			}
		}

		/*
		** Print up game info if console is idle.
		*/
		delay--;
		if (delay <= 0) {
			delay = 100;
			unsigned long time = TIMEGETTIME();

			/*
			** Handle timer reset.
			*/
			if (time < last_info_time) {
				last_info_time = time;
			}

			/*
			** Print if enough time has gone by.
			*/
			if (Pos == 1 && time - LastKeypressTime > 30 * 1000) {
				if (time - last_info_time > 60 * 1000) {
					if (cNetwork::PServerConnection && cNetwork::PServerConnection->Get_Num_RHosts() != num_players) {
						num_players = cNetwork::PServerConnection->Get_Num_RHosts();
						ConsoleFunctionManager::Parse_Input("game_info");
						cprintf(">");
						last_info_time = time;
						Apply_Attributes();
					}
				}
			}
		}
	}
}




/***********************************************************************************************
 * ConsoleModeClass::Add_Message -- Print colored text to the console                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Formatted wide string                                                             *
 *           Color to print in                                                                 *
 *           Forced - set to true to print even if the user is currently typing at the console *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:06PM ST : Created                                                            *
 *=============================================================================================*/
void ConsoleModeClass::Add_Message(WideStringClass *formatted_text, Vector3 *text_color, bool forced)
{

	if (!ProfileMode && formatted_text && text_color && (forced || (Pos == 1 && TIMEGETTIME() - LastKeypressTime > 3 * 1000))) {

		unsigned short color = 0;

		/*
		** Convert the Vector3 RGB to text attribute colors.
		*/
		if (text_color->X != 0.0f) {
			color |= Platform::TerminalRed;
			if (text_color->X > 0.4f) {
				color |= Platform::TerminalBright;
			}
		}

		if (text_color->Y != 0.0f) {
			color |= Platform::TerminalGreen;
			if (text_color->Y > 0.4f) {
				color |= Platform::TerminalBright;
			}
		}

		if (text_color->Z != 0.0f) {
			color |= Platform::TerminalBlue;
			if (text_color->Z > 0.4f) {
				color |= Platform::TerminalBright;
			}
		}

		if (!SlaveMaster.Am_I_Slave()) {
			Platform::SetTerminalColors( color | Platform::TerminalBackgroundBlue);
		} else {
			Platform::SetTerminalColors( color | Platform::TerminalBackgroundGreen | Platform::TerminalBackgroundRed | Platform::TerminalBackgroundBlue);
		}

		StringClass string(128, true);
		formatted_text->Convert_To(string);
		cprintf("%s", string.Peek_Buffer());
		Log_To_Disk(string.Peek_Buffer());

		if (!SlaveMaster.Am_I_Slave()) {
			Platform::SetTerminalColors( MASTER_COLORS);
		} else {
			Platform::SetTerminalColors( SLAVE_COLORS);
		}

		/*
		** Reprint the prompt.
		*/
		//cprintf(">");

		Apply_Attributes();
	}
}




/***********************************************************************************************
 * ConsoleModeClass::Apply_Attributes -- Reapply the console colors.                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/2001 1:26PM ST : Created                                                            *
 *=============================================================================================*/
void ConsoleModeClass::Apply_Attributes(void)
{
	Platform::ApplyTerminalColors(5 * 80, SlaveMaster.Am_I_Slave() ? SLAVE_COLORS : MASTER_COLORS);
}



// unrecognized character escape sequence
#pragma warning(disable : 4129)


/***********************************************************************************************
 * ConsoleModeClass::Update_Profile -- Print the profile text                                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Profile text                                                                      *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/13/2002 12:56PM ST : Created                                                            *
 *=============================================================================================*/
void ConsoleModeClass::Update_Profile(StringClass profile_string)
{

	if (profile_string.Get_Length() && ProfileMode && TIMEGETTIME() - LastProfilePrint > 1500) {

		LastProfilePrint = TIMEGETTIME();

		/*
		** Get a checksum of the profile string.
		*/
		unsigned long crc = CRC::Memory((unsigned char*)profile_string.Peek_Buffer(), profile_string.Get_Length());
		if (crc != LastProfileCRC) {


			/*
			** Create a copy of the string and scan it for '%'.
			*/
			int len = profile_string.Get_Length();
			char *str = (char*) alloca(len * 2);
			char *src = profile_string.Peek_Buffer();
			char *dst = str;
			char c;

			for (int i=0 ; i<len ; i++) {
				c = *src++;
				/*
				** % = 37. Double up % sign so it prints literally.
				*/
				if (c == 37) {
					*dst++ = 37;
				}
				*dst++ = c;
			}
			*dst = 0;

			/*
			** Save the cursor position.
			*/
			const auto cursor = Platform::SaveTerminalCursor();
			Platform::ClearTerminalFromCursor(cursor, 206 * 80);
			if (LastProfileCRC == 0)
				Platform::FillTerminalColors(cursor, 20 * 80, SlaveMaster.Am_I_Slave() ? SLAVE_COLORS : MASTER_COLORS);
			Print(str);
			Platform::RestoreTerminalCursor(cursor);

			LastProfileCRC = crc;
		}
	}
}


/***********************************************************************************************
 * ConsoleModeClass::Handle_Profile_Key -- Handle keyboard input in profile mode               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Key pressed                                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/13/2002 1:09PM ST : Created                                                             *
 *=============================================================================================*/
void ConsoleModeClass::Handle_Profile_Key(int key)
{
	if (Get_Console()) {
		if (key >= '0' && key <= '9') {
			char profile_text[4];
			profile_text[0] = (char)key;
			profile_text[1] = 0;
			Get_Console()->Profile_Command(profile_text);
			cprintf("\r");
			LastProfilePrint = 0;
		} else {
			if (key >= 'a' && key <= 'z') {
				char profile_text[4];
				profile_text[0] = '1';
				profile_text[1] = (char)(key - 'a') + '0';
				profile_text[2] = 0;
				Get_Console()->Profile_Command(profile_text);
				cprintf("\r");
				LastProfilePrint = 0;
			} else {
				if (key == '.') {
					Get_Console()->Profile_Command("up");
					cprintf("\r");
					LastProfilePrint = 0;
				}
			}
		}
	}
}




/***********************************************************************************************
 * ConsoleModeClass::Wait_For_Keypress -- Wait for user to press a key                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/4/2002 11:10AM ST : Created                                                             *
 *=============================================================================================*/
void ConsoleModeClass::Wait_For_Keypress(void)
{
	if (Get_Console()) {
		if (TerminalOpen) {
			Print("** Press any key to continue **\n");
		}
		Platform::ReadTerminalKey(false);
	}
}