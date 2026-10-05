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
 *                     $Archive:: /Commando/Code/Commando/WINMAIN.CPP                         $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/17/02 11:08a                                              $*
 *                                                                                             *
 *                    $Revision:: 85                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   WinMain -- Win32 Program Entry Point!                                                     *
 *   WIN_resize -- Surrender-required function which resizes the main window                   *
 *   WIN_set_fullscreen -- Surrender-required function for toggling full-screen mode           *
 *   Main_Window_Proc -- Windows Proc for the main game window                                 *
 *   Create_Main_Window -- Creates the main game window                                        *
 *   On_Focus_Loss -- this function is called when the application loses focus                 *
 *   On_Focus_Restore -- This function is called when the application gets focus               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "winmain.h"
#include "Platform/Platform.h"
#include "Platform/Windows/WindowsMessages.h"
#include <SDL3/SDL.h>
#include <cstring>
#define _WIN32_WINDOWS 0x0401
#include "win.h"
#include "resource.h"
#include "WW3D.H"

#include "miscutil.h"
#include "input.h"
#include "useroptions.h"

#include "WWAudio.H"
#include "FFactory.H"

#include "debug.h"
#include "verchk.h"
#include "devoptions.h"
#include "dialogmgr.h"
#include "renegadedialogmgr.h"
#include "except.h"

#include "DirectInput.h"
#include "WebBrowser.h"
#include "wwmemlog.h"

#include "datasafe.h"

#include "combatgmode.h"
#include "Settings.h"
#include "init.h"
#include "mainloop.h"
#include "_globals.h"
#include "buildnum.h"
#include "dx8wrapper.h"
#include "formconv.h"
#include "autostart.h"
#include "consolemode.h"
#include "specialbuilds.h"
#include "slavemaster.h"

#include "serversettings.h"

#ifdef _DEBUG
#include	<crtdbg.h>
#endif //_DEBUG

#if (_MSC_VER >= 1200)
#pragma warning(push,1)
#endif

#include <iostream>
#include "singletoninstancekeeper.h"
#include "packetmgr.h"


#if (_MSC_VER >= 1200)
#pragma warning(pop)
#endif

//----------------------------------------------------------------------------
//	Globals
//----------------------------------------------------------------------------
extern "C"
{
	HWND		hWndMain;
	bool		WIN_fullscreen = true;
}


//----------------------------------------------------------------------------
//	Local functions
//----------------------------------------------------------------------------
static BOOL Create_Main_Window(HANDLE hInstance, int nCmdShow);
static void Application_Event(const SDL_Event& event);
void On_Focus_Loss(void);
void On_Focus_Restore(void);
void Split_Command_Line_Args(HINSTANCE instance, char *path_to_exe, char *command_line);
int Start_Application( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow );



#ifndef _MSC_VER
long Top_Level_Exception_Filter(EXCEPTION_POINTERS *e_info)
{
	return(Exception_Handler(e_info->ExceptionRecord->ExceptionCode, e_info));
}
#endif //_MSC_VER




/***********************************************************************************************
 * WinMain -- Win32 Program Entry Point!                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *  																														  *
 * 	Standard WinMain inputs :-)																				  *
 * 																														  *
 * OUTPUT:																												  *
 *  																														  *
 * 	Standard WinMain output																						  *
 * 																														  *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	WWMemoryLogClass::Init();	// This switches memlog from static to dynamic allocations mode

#ifdef _DEBUG
	/*
	**	Setup to track memory heap integrity and check for
	**	memory leaks. Output is dumped to the debug string console.
	*/
	if (cDevOptions::CrtDbgEnabled.Is_True()) {

		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	}

	/*
	** Leak test
	*/
	//char *flibble = new char [1024];
	//unsigned long *flibble2 = new unsigned long;
	//void *flibble3 = malloc(128);

	//flibble = flibble;
	//flibble2 = flibble2;
	//flibble3 = flibble3;
#endif //_DEBUG

#ifdef WWDEBUG
	//
	// If necessary, disable packet optimizations
	//
	if (cDevOptions::PacketOptimizationsEnabled.Is_False()) {
		PacketManager.Disable_Optimizations();
	}
#endif //WWDEBUG

#ifdef _DEBUG // Denzil - DO NOT COMPILE INTO FINAL BUILD
	bool webInstalled = WebBrowser::InstallPrerequisites();

	if (!webInstalled) {
		::MessageBox(NULL, "Embedded browser prerequisites missing\n\nBrowser functionality questionable.\n\n(Please call Denzil @ 27272)",
				"Renegade Warning!", MB_ICONWARNING|MB_OK);
	}
#endif // WWDEBUG

   if (!cUserOptions::Parse_Command_Line(lpCmdLine)) {
		return(0);
	}

	//
	// There's a couple of options we need for the FDS
	//
#ifdef FREEDEDICATEDSERVER
	if (!SlaveMaster.Am_I_Slave() && !ServerSettingsClass::Is_Server_Settings_File_Set()) {
		cUserOptions::Set_Server_INI_File("STARTSERVER=server.ini");
	}
	ConsoleBox.Set_Exclusive(true);
#endif //FREEDEDICATEDSERVER

	//
	//	Verify that we can execute (i.e. make sure there are no other instances running)
	//
	SingletonInstanceKeeperClass instance_keeper;
	if (instance_keeper.Verify_Safe_To_Execute () == false) {
		return 0;
	}

	//
	//	Start the game!
	//
	int retval = Start_Application( hInstance, hPrevInstance, lpCmdLine, nCmdShow );
	return retval;
}

/***********************************************************************************************
 * Start_Application -- Handles WinMain execution.															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *  																														  *
 * 	Standard WinMain inputs :-)																				  *
 * 																														  *
 * OUTPUT:																												  *
 *  																														  *
 * 	Standard WinMain output																						  *
 * 																														  *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/2001 PDS  : Created.                                                                *
 *=============================================================================================*/
// Keep SEH in a function without C++ objects that require stack unwinding.
static int Run_Game_Main_Loop_With_Exception_Handler()
{
	int exitCode = EXIT_SUCCESS;
#ifdef _MSC_VER
	__try {
		exitCode = Game_Main_Loop();
	} __except(Exception_Handler(GetExceptionCode(), GetExceptionInformation())) {};
#else
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER) &Top_Level_Exception_Filter);
	exitCode = Game_Main_Loop();
#endif
	return exitCode;
}

int Start_Application( HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow )
{
	{
		WWMEMLOG(MEM_GAMEINIT);
		//LPSTR	command	= lpCmdLine;

		//
		// TEMP Dev code - check the working folder is correct!
		//
		char tmp_buffer[256];
		char* tmp_ptr;
		if (!SearchPath(
			"data",
			"always.dat",
			NULL,
			sizeof(tmp_buffer),
			tmp_buffer,
			&tmp_ptr)) {
			MessageBox(NULL,"Set working folder and try again...","Invalid working folder",MB_OK);
			return 0;
		}

		/*
		** Only do these checks if this isn't an auto-restart. If we are restarting then we must have run once OK already. Right?
		*/

		//Debug_Say(("Started logging at time %s", cMiscUtil::Get_Text_Time()));

		if (!Create_Main_Window(hInstance, nCmdShow)) return 0;

		Register_Thread_ID(GetCurrentThreadId(), "Main Thread", true);
	}

	int exitCode = EXIT_SUCCESS;

#ifdef WWDEBUG
	if (cDevOptions::EnableExceptionHandler.Is_False()) {
		exitCode = Game_Main_Loop();
	} else {
#endif //WWDEBUG

		Register_Application_Exception_Callback(&Application_Exception_Callback);
		Register_Application_Version_Callback(&BuildInfoClass::Composite_Build_Info);

		WWDEBUG_SAY(("Game_Main_Loop\n"));
		exitCode = Run_Game_Main_Loop_With_Exception_Handler();
#ifdef WWDEBUG
	}
#endif //WWDEBUG

	Unregister_Thread_ID(GetCurrentThreadId(), "Main Thread");
	Platform::Shutdown();

   //Debug_Say(("Finished logging at time %s", cMiscUtil::Get_Text_Time()));

	return exitCode;
}

/***********************************************************************************************
 * Main_Window_Proc -- Windows Proc for the main game window                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 * 																														  *
 * 	Standard Windows Proc inputs																				  *
 * 																														  *
 * OUTPUT:																												  *
 * 																														  *
 * 	Standard Windows Proc output																				  *
 * 																														  *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
static void Application_Event(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        Stop_Main_Loop(EXIT_SUCCESS);
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        if (!GameInFocus) { GameInFocus = true; On_Focus_Restore(); }
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        if (GameInFocus) { GameInFocus = false; On_Focus_Loss(); }
        break;
    case SDL_EVENT_TEXT_INPUT:
    case SDL_EVENT_TEXT_EDITING:
        if (_TheWWUIInput && !Input::Is_Console_Enabled()) _TheWWUIInput->ProcessSDLEvent(event);
        else if (event.type == SDL_EVENT_TEXT_INPUT && Input::Is_Console_Enabled()) {
            const char* text = event.text.text;
            while (text && *text) Input::Console_Add_Key(static_cast<int>(SDL_StepUTF8(&text, nullptr)));
        }
        break;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        if (Input::Is_Console_Enabled() && event.key.down) {
            if (event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER) Input::Console_Add_Key(13);
            else if (event.key.key == SDLK_BACKSPACE) Input::Console_Add_Key(8);
            else if (event.key.key == SDLK_ESCAPE) Input::Console_Add_Key(27);
            else if (event.key.key == SDLK_TAB) Input::Console_Add_Key(9);
        }
        if (_TheWWUIInput && !Input::Is_Console_Enabled()) _TheWWUIInput->ProcessSDLEvent(event);
        if (event.key.down && !event.key.repeat && event.key.scancode == SDL_SCANCODE_RETURN &&
            (event.key.mod & SDL_KMOD_ALT) && WW3D::Is_Initted()) WW3D::Toggle_Windowed();
        break;
    }
}

/***********************************************************************************************
 * Create_Main_Window -- Creates the main game window                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *  																														  *
 * 	hInstance -- Instance handle of the application														  *
 * 	nCmdShow -- how the window is to be shown																  *
 * 																														  *
 * OUTPUT:																												  *
 * 																														  *
 * 	TRUE = success, FALSE = failure																			  *
 * 																														  *
 * WARNINGS:																											  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
static BOOL Create_Main_Window(HANDLE hInstance, int /*nCmdShow*/)
{
    ProgramInstance = (HINSTANCE)hInstance;
    if (!Platform::Initialize()) {
        MessageBox(NULL, SDL_GetError(), "SDL initialization failed", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    if (!ConsoleBox.Is_Exclusive()) {
        if (!Platform::CreateGameWindow("Renegade", 800, 600)) {
            MessageBox(NULL, SDL_GetError(), "Window creation failed", MB_OK | MB_ICONERROR);
            Platform::Shutdown();
            return FALSE;
        }
        MainWindow = static_cast<HWND>(Platform::NativeWindowHandle());
    }
    Platform::SetEventHandler(Application_Event);
    return TRUE;
}

/***********************************************************************************************
 * On_Focus_Loss -- this function is called when the application loses focus                   *
 *                                                                                             *
 * INPUT:		Nothing																								  *
 * 																														  *
 * OUTPUT:		Nothing																								  *
 * 																														  *
 * WARNINGS:	None																									  *
 * 																														  *
 * HISTORY:                                                                                    *
 *   07/18/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void On_Focus_Loss(void)
{
	DirectInput::Unacquire();
}


/***********************************************************************************************
 * On_Focus_Restore -- This function is called when the application gets focus                 *
 *                                                                                             *
 * INPUT:		Nothing																								  *
 * 																														  *
 * OUTPUT:		Nothing																								  *
 * 																														  *
 * WARNINGS:	None																									  *
 * 																														  *
 * HISTORY:                                                                                    *
 *   07/18/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void On_Focus_Restore(void)
{
	if (WebBrowser::IsWebPageDisplayed() == false) {
		DirectInput::Acquire();
	}

	GameModeManager::Hide_Render_Frames(1);	// Hide the first rendered frame
}


void Prog_End(void)
{
}
