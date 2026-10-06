#include "Platform/Application.h"
#include "Platform/Platform.h"
#include "Platform/GameState.h"
#include "WebBrowser.h"
#include "input.h"
#include "useroptions.h"
#include "devoptions.h"
#include "renegadedialogmgr.h"
#include "directinput.h"
#include "wwmemlog.h"
#include "gamemode.h"
#include "init.h"
#include "mainloop.h"
#include "buildnum.h"
#include "consolemode.h"
#include "specialbuilds.h"
#include "slavemaster.h"
#include "serversettings.h"
#include "singletoninstancekeeper.h"
#include "packetmgr.h"
#include "ww3d.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdlib>
#include <string>

void On_Focus_Loss()
{
    DirectInput::Unacquire();
}

void On_Focus_Restore()
{
    DirectInput::Acquire();
    GameModeManager::Hide_Render_Frames(1);
}

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


namespace {
bool DataFileExists()
{
    int count = 0;
    char** directories = SDL_GlobDirectory(".", "data", SDL_GLOB_CASEINSENSITIVE, &count);
    bool found = false;
    for (int i = 0; directories && i < count && !found; ++i) {
        int files = 0;
        char** names = SDL_GlobDirectory(directories[i], "always.dat", SDL_GLOB_CASEINSENSITIVE, &files);
        for (int j = 0; names && j < files && !found; ++j) {
            const std::string path = std::string(directories[i]) + "/" + names[j];
            SDL_PathInfo info{};
            found = SDL_GetPathInfo(path.c_str(), &info) && info.type == SDL_PATHTYPE_FILE;
        }
        SDL_free(names);
    }
    SDL_free(directories);
    return found;
}

struct RuntimeLifetime {
    ~RuntimeLifetime() { Platform::Shutdown(); }
};
}

int main(int argc, char** argv)
{
    WWMemoryLogClass::Init();
    bool trackMemory = false;
#ifdef WWDEBUG
    trackMemory = cDevOptions::CrtDbgEnabled.Is_True();
    if (cDevOptions::PacketOptimizationsEnabled.Is_False()) PacketManager.Disable_Optimizations();
#endif
    Platform::PrepareApplication(trackMemory);
#ifdef _DEBUG
    if (!WebBrowser::InstallPrerequisites()) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Renegade Warning!",
            "Embedded browser prerequisites missing", nullptr);
    }
#endif
    if (!cUserOptions::Parse_Command_Line(argc, argv)) return EXIT_SUCCESS;
#ifdef FREEDEDICATEDSERVER
    if (!SlaveMaster.Am_I_Slave() && !ServerSettingsClass::Is_Server_Settings_File_Set()) {
        char server[] = "STARTSERVER=server.ini";
        cUserOptions::Set_Server_INI_File(server);
    }
    ConsoleBox.Set_Exclusive(true);
#endif
    SingletonInstanceKeeperClass instance;
    if (!instance.Verify_Safe_To_Execute()) return EXIT_SUCCESS;
    if (!DataFileExists()) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invalid working folder",
            "Set working folder and try again...", nullptr);
        return EXIT_SUCCESS;
    }
    WWMEMLOG(MEM_GAMEINIT);
    RuntimeLifetime runtime;
    if (!Platform::Initialize()) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL initialization failed", SDL_GetError(), nullptr);
        return EXIT_SUCCESS;
    }
    if (!ConsoleBox.Is_Exclusive()) {
        if (!Platform::CreateGameWindow("Renegade", 800, 600)) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Window creation failed", SDL_GetError(), nullptr);
            return EXIT_SUCCESS;
        }
        Platform::AttachApplicationWindow();
    }
    Platform::SetEventHandler(Application_Event);
    bool handleExceptions = true;
#ifdef WWDEBUG
    handleExceptions = cDevOptions::EnableExceptionHandler.Is_True();
#endif
    WWDEBUG_SAY(("Game_Main_Loop\n"));
    return Platform::RunApplicationLoop(Game_Main_Loop, handleExceptions,
        Application_Exception_Callback, BuildInfoClass::Composite_Build_Info);
}

void Prog_End() {}
