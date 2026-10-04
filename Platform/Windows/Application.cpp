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
#include <filesystem>
#include <cstring>
#include <string>
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
#include "registry.h"
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
static Uint32 NativeTextEvent = 0;
static void Application_Event(const SDL_Event& event);
static bool Native_Game_Message(MSG& message);
void On_Focus_Loss(void);
void On_Focus_Restore(void);
void Split_Command_Line_Args(HINSTANCE instance, char *path_to_exe, char *command_line);
void Set_Working_Directory(char *old_path, char *new_path);
int Start_Application( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow );
void Set_Working_Directory(HINSTANCE hInstance);



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

static bool Graphics_Settings_Trouble_Shooting()
{
	RegistryClass registry( APPLICATION_SUB_KEY_NAME_DEBUG );
	if (!registry.Is_Valid()) return true;

	int progress=registry.Get_Int( VALUE_NAME_GAME_INITIALIZATION_IN_PROGRESS, 0 );
	if (progress) {
		StringClass options="wwconfig.exe ";
		char* opts=options.Peek_Buffer();
		STARTUPINFO startup_info;
		ZeroMemory(&startup_info,sizeof(STARTUPINFO));
		startup_info.cb=sizeof(STARTUPINFO);
		PROCESS_INFORMATION process_info;
		CreateProcess(
			NULL,
			opts,
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&startup_info,
			&process_info);

		unsigned long exit_code=STILL_ACTIVE;
		unsigned res=0;
		do {
			res=GetExitCodeProcess(process_info.hProcess,&exit_code);
			if (!res) {
				return true;
			}
			Sleep(100);
		}
		while (exit_code==STILL_ACTIVE);
		return !exit_code;
	}
	return true;
}

typedef IDirect3D8* (WINAPI *Direct3DCreate8Type) (UINT SDKVersion);
static Direct3DCreate8Type	Direct3DCreate8Ptr = NULL;
static HINSTANCE D3D8Lib = NULL;

static bool Video_Card_Driver_Check()
{
	RegistryClass render_registry(APPLICATION_SUB_KEY_NAME_RENDER);
	if (!render_registry.Is_Valid()) return true;

	int disabled=render_registry.Get_Int( "DriverVersionCheckDisabled" );
	if (disabled>=87) return true;

	IDirect3D8* d3d=NULL;
	D3DCAPS8 tmp_caps;
	const D3DCAPS8* d3dcaps=NULL;
	D3DADAPTER_IDENTIFIER8 adapter_id;

	// Init D3D
	Init_D3D_To_WW3_Conversion();

	D3D8Lib = LoadLibrary("D3D8.DLL");

	if (D3D8Lib != NULL) {
		Direct3DCreate8Ptr = (Direct3DCreate8Type) GetProcAddress(D3D8Lib, "Direct3DCreate8");
		if (Direct3DCreate8Ptr) {
			d3d=Direct3DCreate8Ptr(D3D_SDK_VERSION);		// TODO: handle failure cases...
			if (!d3d) {
				FreeLibrary(D3D8Lib);
				return true;
			}
		} else {
			FreeLibrary(D3D8Lib);
			return(true);
		}
	} else {
		return(true);
	}

	// Select device. If there is already a device selected in the registry, use it.

	int current_adapter_index=D3DADAPTER_DEFAULT;

	//
	//	Load the render device settings from the registry
	//
	char device_name[256] = { 0 };
	render_registry.Get_String( VALUE_NAME_RENDER_DEVICE_NAME, device_name, sizeof(device_name));

	int adapter_count = d3d->GetAdapterCount();
	for (int adapter_index=0; adapter_index<adapter_count; adapter_index++) {
		D3DADAPTER_IDENTIFIER8 id;
		::ZeroMemory(&id, sizeof(D3DADAPTER_IDENTIFIER8));
		HRESULT res = d3d->GetAdapterIdentifier(adapter_index,D3DENUM_NO_WHQL_LEVEL,&id);
		// If device ok, check if it matches the currently set adapter name
		if (res == D3D_OK) {
			StringClass name(id.Description,true);
			if (name==device_name) {
				current_adapter_index=adapter_index;
				break;
			}
		}
	}

	if (FAILED(d3d->GetDeviceCaps(
		current_adapter_index,
		D3DDEVTYPE_HAL,
		&tmp_caps))) {
		d3d->Release();
		FreeLibrary(D3D8Lib);
		return true;
	}

	::ZeroMemory(&adapter_id, sizeof(D3DADAPTER_IDENTIFIER8));
	if (FAILED( d3d->GetAdapterIdentifier(
		current_adapter_index,
		D3DENUM_NO_WHQL_LEVEL,
		&adapter_id))) {
		d3d->Release();
		FreeLibrary(D3D8Lib);
		return true;
	}

	d3dcaps=&tmp_caps;
	DX8Caps caps(d3d,*d3dcaps,WW3D_FORMAT_UNKNOWN,adapter_id);

	DX8Caps::DriverVersionStatusType status=caps.Get_Driver_Version_Status();

	d3d->Release();
	FreeLibrary(D3D8Lib);

	switch (status) {
	default:
	case DX8Caps::DRIVER_STATUS_GOOD:
	case DX8Caps::DRIVER_STATUS_OK:
	case DX8Caps::DRIVER_STATUS_UNKNOWN:
		render_registry.Set_Int( "DriverVersionCheckDisabled",87 );
		return true;
		break;
	case DX8Caps::DRIVER_STATUS_BAD:
		break;
	}

	StringClass options="wwconfig.exe -driverversion";
	char* opts=options.Peek_Buffer();
	STARTUPINFO startup_info;
	ZeroMemory(&startup_info,sizeof(STARTUPINFO));
	startup_info.cb=sizeof(STARTUPINFO);
	PROCESS_INFORMATION process_info;
	if (!CreateProcess(
		NULL,
		opts,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&startup_info,
		&process_info)) {
		return true;
	}

	unsigned long exit_code=STILL_ACTIVE;
	unsigned res=0;
	do {
		res=GetExitCodeProcess(process_info.hProcess,&exit_code);
		if (!res) {
			return true;
		}
		Sleep(100);
	}
	while (exit_code==STILL_ACTIVE);
	return !exit_code;
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

		/*
		** Set the working directory.
		*/
		Set_Working_Directory(hInstance);

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
		if (AutoRestart.Get_Restart_Flag() == false && !ConsoleBox.Is_Exclusive()) {
			if (!Video_Card_Driver_Check()) return 0;
			if (!Graphics_Settings_Trouble_Shooting()) return 0;
		}

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
static bool Native_Game_Message(MSG& msg)
{
    if (msg.hwnd != MainWindow) return false;
    // Put retained IME text into SDL's queue so text follows its key event.
    if (msg.message == WM_CHAR) {
        SDL_Event event{};
        event.type = NativeTextEvent;
        event.user.code = static_cast<int>(msg.wParam);
        event.user.data1 = reinterpret_cast<void*>(msg.lParam);
        return SDL_PushEvent(&event);
    }
    bool handled = false;
    if (_TheWWUIInput && !Input::Is_Console_Enabled()) {
        LRESULT result = 0;
        handled = _TheWWUIInput->ProcessMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam, result);
    }
    return handled;
}

static void Application_Event(const SDL_Event& event)
{
    if (event.type == NativeTextEvent) {
        LRESULT result = 0;
        if (!_TheWWUIInput || Input::Is_Console_Enabled() ||
            !_TheWWUIInput->ProcessMessage(MainWindow, WM_CHAR, event.user.code,
                reinterpret_cast<LPARAM>(event.user.data1), result)) {
            Input::Console_Add_Key(event.user.code);
        }
        return;
    }
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
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        if (_TheWWUIInput && !Input::Is_Console_Enabled()) _TheWWUIInput->ProcessSDLKeyEvent(event.key);
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
    NativeTextEvent = SDL_RegisterEvents(1);
    if (!NativeTextEvent) { Platform::Shutdown(); return FALSE; }
    Platform::SetEventHandler(Application_Event);
    Install_Windows_Message_Hook(Native_Game_Message);
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


/***********************************************************************************************
 * Set_Working_Directory -- Sets current directory to be the same as the .exe                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Program instance                                                                  *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/6/2001 12:11PM ST : Created                                                            *
 *=============================================================================================*/
void Set_Working_Directory(HINSTANCE /*instance*/)
{
    if (const char* path = SDL_GetBasePath()) {
        const std::u8string utf8_path(path, path + std::strlen(path));
        std::filesystem::current_path(std::filesystem::path(utf8_path));
    }
}
