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
 *                     $Archive:: /Commando/Code/Commando/slavemaster.cpp                     $*
 *                                                                                             *
 *                       Author:: Steve Tall                                                   *
 *                                                                                             *
 *                     $Modtime:: 2/15/02 12:44p                                              $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "always.h"
#include "Platform/Platform.h"
#include "Platform/SystemInfo.h"
#include "Platform/Process.h"
#include <SDL3/SDL_error.h>
#include "slavemaster.h"
#include "wwdebug.h"
#include "Settings.h"
#include "_globals.h"
#include "autostart.h"
#include "ini.h"
#include "rawfile.h"
#include "inisup.h"
#include "natter.h"
#include "gamesideservercontrol.h"

#include "gamedata.h"
#include "serversettings.h"
#include "bandwidth.h"
#include "consolemode.h"
#include "specialbuilds.h"
#include "useroptions.h"


#include <string.h>
#include <stdio.h>

#define KEY_NUM_SLAVES				"Count"
#define KEY_SLAVE_NAME				"Name"
#define KEY_SLAVE_SERIAL			"Serial"
#define KEY_SLAVE_ENABLE			"Enable"
#define KEY_SLAVE_PORT				"Port"
#define KEY_SLAVE_RUNNING_ID		"RunningID"
#define KEY_SLAVE_SETTINGS			"Settings"
#define KEY_SLAVE_BANDWIDTH		"Bandwidth"
#define KEY_SLAVE_PASSWORD			"Password"

const char *SettingsFileName = "slave.ini";

SlaveMasterClass SlaveMaster;


extern char DefaultSettingsModifier[1024];


/***********************************************************************************************
 * SlaveServerClass::SlaveServerClass -- SlaveServerClass constuctor                           *
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
 *   11/21/2001 3:51PM ST : Created                                                            *
 *=============================================================================================*/
SlaveServerClass::SlaveServerClass(void)
{
	Enable = false;
	IsRunning = false;
	NickName[0] = 0;
	Serial[0] = 0;
	Port = 0;
	Bandwidth = 0;
	Password[0] = 0;
}


/***********************************************************************************************
 * SlaveServerClass::~SlaveServerClass -- SlaveServerClass desturctor                          *
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
 *   11/21/2001 3:51PM ST : Created                                                            *
 *=============================================================================================*/
SlaveServerClass::~SlaveServerClass(void)
{
}



/***********************************************************************************************
 * SlaveServerClass::Set -- Set info about this slave                                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Is slave enabled?                                                                 *
 *           Nickname to use with this slave                                                   *
 *           Serial number to use with this slave                                              *
 *           Port to use with this slave                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:50PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveServerClass::Set(bool enable, const char *nick, const char *serial, unsigned short port, const char *settings_file, int bandwidth, const char *password)
{
	Enable = enable;
	Port = port;
	Bandwidth = bandwidth;

	if (nick) {
		strncpy(NickName, nick, sizeof(NickName));
	}

	if (serial) {
		strncpy(Serial, serial, sizeof(Serial));
	}

	if (password) {
		strncpy(Password, password, sizeof(Password));
	}

	if (settings_file) {
		SettingsFileName = settings_file;
	}

}



/***********************************************************************************************
 * SlaveServerClass::Get -- Get info about this slave                                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Is slave enabled?                                                                 *
 *           Nickname to use with this slave                                                   *
 *           Serial number to use with this slave                                              *
 *           Port to use with this slave                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:49PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveServerClass::Get(bool &enable, char *nick, char *serial, unsigned short &port, StringClass& settings_file, int &bandwidth, char *password)
{
	enable = Enable;
	port = Port;
	bandwidth = Bandwidth;

	if (nick) {
		strcpy(nick, NickName);
	}

	if (serial) {
		strcpy(serial, Serial);
	}

	if (password) {
		strcpy(password, Password);
	}

	settings_file = SettingsFileName;
}




/***********************************************************************************************
 * SlaveMasterClass::SlaveMasterClass -- SlaveMasterClass constructor                          *
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
 *   11/21/2001 3:49PM ST : Created                                                            *
 *=============================================================================================*/
SlaveMasterClass::SlaveMasterClass(void)
{
	NumSlaveServers = 0;
	SlaveMode = false;
}


/***********************************************************************************************
 * SlaveMasterClass::~SlaveMasterClass -- SlaveMasterClass destructor                          *
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
 *   11/21/2001 3:49PM ST : Created                                                            *
 *=============================================================================================*/
SlaveMasterClass::~SlaveMasterClass(void)
{
	/*
	** Make sure all slaves are gone before we quit.
	*/
	Wait_For_Slave_Shutdown();
}





/***********************************************************************************************
 * SlaveMasterClass::Wait_For_Slave_Shutdown -- Wait for slaves to exit                        *
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
 *   1/7/2002 2:58PM ST : Created                                                              *
 *=============================================================================================*/
void SlaveMasterClass::Wait_For_Slave_Shutdown(void)
{
	if (SlaveMode) return;
	const auto start = Platform::Ticks();
	int last_num_running = 0;
	bool forced = false;
	while (Platform::Ticks() - start < 40000) {
		int num_running = 0;
		for (int i = 0; i < NumSlaveServers; ++i) {
			auto& slave = SlaveServers[i];
			if (slave.IsRunning && slave.Process && slave.Process->Running()) ++num_running;
			else { slave.IsRunning = false; slave.Process.reset(); }
		}
		if (!num_running) break;
		if (num_running != last_num_running) {
			ConsoleBox.Print("Waiting for %d slave(s) to shut down\n", num_running);
			WWDEBUG_SAY(("Waiting for %d slave(s) to shut down\n", num_running));
			last_num_running = num_running;
		}
		if (!forced && Platform::Ticks() - start > 27000) {
			forced = true;
			for (int i = 0; i < NumSlaveServers; ++i) {
				auto& process = SlaveServers[i].Process;
				if (process && process->Running()) {
					WWDEBUG_SAY(("Terminating process %u due to timeout\n", process->Id()));
					if (!process->Terminate()) WWDEBUG_SAY(("Failed to terminate slave process %u\n", process->Id()));
				}
			}
		}
		Platform::Sleep(10);
	}
}



/***********************************************************************************************
 * SlaveMasterClass::Get_Slave -- Get slave server entry by index                              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Index                                                                             *
 *                                                                                             *
 * OUTPUT:   Ptr to slave server                                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:48PM ST : Created                                                            *
 *=============================================================================================*/
SlaveServerClass *SlaveMasterClass::Get_Slave(int index)
{
	WWASSERT(index < NumSlaveServers);
	WWASSERT(index >= 0);
	if (index >= 0 && index <NumSlaveServers) {
		return(&SlaveServers[index]);
	}
	return(NULL);
}



/***********************************************************************************************
 * SlaveMasterClass::Save -- Save slave server info to settings                                *
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
 *   11/21/2001 3:48PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Save(void)
{
	SettingsClass reg(APPLICATION_SETTINGS_SECTION_NET_SLAVE);
	if (reg.Is_Valid()) {
		reg.Set_Int(KEY_NUM_SLAVES, NumSlaveServers);
	}

	for (int i=0 ; i<NumSlaveServers ; i++) {
		char entry[256];
		sprintf(entry, "%s%d", KEY_SLAVE_NAME, i);
		reg.Set_String(entry, SlaveServers[i].NickName);

		sprintf(entry, "%s%d", KEY_SLAVE_PASSWORD, i);
		reg.Set_String(entry, SlaveServers[i].Password);

		sprintf(entry, "%s%d", KEY_SLAVE_SETTINGS, i);
		reg.Set_String(entry, SlaveServers[i].SettingsFileName);

		sprintf(entry, "%s%d", KEY_SLAVE_ENABLE, i);
		reg.Set_Bool(entry, SlaveServers[i].Enable);

		sprintf(entry, "%s%d", KEY_SLAVE_PORT, i);
		reg.Set_Int(entry, SlaveServers[i].Port);

		sprintf(entry, "%s%d", KEY_SLAVE_BANDWIDTH, i);
		reg.Set_Int(entry, SlaveServers[i].Bandwidth);

		sprintf(entry, "%s%d", KEY_SLAVE_SERIAL, i);
		StringClass serial(SlaveServers[i].Serial, true);
		StringClass encrypted_serial = serial;
		if (serial.Get_Length()) {
			ServerSettingsClass::Encrypt_Serial(serial, encrypted_serial);
		}
		reg.Set_String(entry, encrypted_serial.Peek_Buffer());
	}
}



/***********************************************************************************************
 * SlaveMasterClass::Load -- Fetch slave server info from settings                             *
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
 *   11/21/2001 3:47PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Load(void)
{
	SettingsClass reg(APPLICATION_SETTINGS_SECTION_NET_SLAVE);
	if (reg.Is_Valid()) {
		NumSlaveServers = reg.Get_Int(KEY_NUM_SLAVES, 0);
	}

	char entry[256];
	for (int i=0 ; i<NumSlaveServers ; i++) {
		sprintf(entry, "%s%d", KEY_SLAVE_NAME, i);
		reg.Get_String(entry, SlaveServers[i].NickName, sizeof(SlaveServers[i].NickName), "");

		sprintf(entry, "%s%d", KEY_SLAVE_PASSWORD, i);
		reg.Get_String(entry, SlaveServers[i].Password, sizeof(SlaveServers[i].Password), "");

		sprintf(entry, "%s%d", KEY_SLAVE_ENABLE, i);
		SlaveServers[i].Enable = reg.Get_Bool(entry, false);

		sprintf(entry, "%s%d", KEY_SLAVE_PORT, i);
		SlaveServers[i].Port = reg.Get_Int(entry, false);

		sprintf(entry, "%s%d", KEY_SLAVE_BANDWIDTH, i);
		SlaveServers[i].Bandwidth = reg.Get_Int(entry, false);

		sprintf(entry, "%s%d", KEY_SLAVE_SETTINGS, i);
		reg.Get_String(entry, SlaveServers[i].SettingsFileName, "");

		sprintf(entry, "%s%d", KEY_SLAVE_SERIAL, i);
		reg.Get_String(entry, SlaveServers[i].Serial, sizeof(SlaveServers[i].Serial), "");
		if (strlen(SlaveServers[i].Serial)) {
			StringClass serial(SlaveServers[i].Serial, true);
			StringClass decrypted_serial = serial;
			if (serial.Get_Length()) {
				ServerSettingsClass::Decrypt_Serial(serial, decrypted_serial);
			}
			strcpy(SlaveServers[i].Serial, decrypted_serial.Peek_Buffer());
		}

		StringClass filename;
		filename.Format("data\\%s", SlaveServers[i].SettingsFileName.Peek_Buffer());
		RawFileClass file(filename);
		if (!file.Is_Available()) {
			SlaveServers[i].SettingsFileName = "svrcfg_cnc.ini";
		}
	}
}


/***********************************************************************************************
 * SlaveMasterClass::Reset -- Clear out slave server list                                      *
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
 *   11/21/2001 3:47PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Reset(void)
{
	for (int i=0 ; i<NumSlaveServers ; i++) {
		SlaveServers[i].Set(false, "", "", 0, "", 0, "");
	}
	NumSlaveServers = 0;
}




/***********************************************************************************************
 * SlaveMasterClass::Add_Slave -- Add slave to list                                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Is slave enabled?                                                                 *
 *           Nickname for this slave to use                                                    *
 *           Serial number for this slave to use                                               *
 *           Port number for this slave to use                                                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:46PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Add_Slave(bool enable, const char *nick, const char *serial, unsigned short port, const char *settings_file, int bandwidth, const char *password)
{
	WWASSERT(NumSlaveServers < MAX_SLAVES);
	SlaveServers[NumSlaveServers++].Set(enable, nick, serial, port, settings_file, bandwidth, password);
}




/***********************************************************************************************
 * SlaveMasterClass::Aquire_Slave -- Find a running slave by it's process ID                   *
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
 *   12/16/2001 11:34PM ST : Created                                                           *
 *=============================================================================================*/
bool SlaveMasterClass::Aquire_Slave(int index)
{
	int proc_id = 0;
	char slave_name[64];
	sprintf(slave_name, "slave_%d", index);
	strcpy(DefaultSettingsModifier, slave_name);
	SettingsClass slave_reg(APPLICATION_SETTINGS_SECTION);
	DefaultSettingsModifier[0] = 0;
	if (slave_reg.Is_Valid()) proc_id = slave_reg.Get_Int("ProcessId", 0);
	if (proc_id == 0) {
		SettingsClass reg(APPLICATION_SETTINGS_SECTION_NET_SLAVE);
		if (reg.Is_Valid()) {
			char entry[128];
			sprintf(entry, "%s%d", KEY_SLAVE_RUNNING_ID, index);
			proc_id = reg.Get_Int(entry, 0);
		}
	}
	if (ConsoleBox.Is_Exclusive()) {
		const auto window = ConsoleBox.Get_Slave_Window_By_Title(SlaveServers[index].NickName, SlaveServers[index].SettingsFileName.Peek_Buffer());
		const auto id = Platform::WindowProcessId(window);
		if (id) proc_id = static_cast<int>(id);
	}
	auto& slave = SlaveServers[index];
	if (proc_id > 0 && slave.Process && slave.Process->Id() == static_cast<unsigned>(proc_id) && slave.Process->Running()) return true;
	auto process = Platform::AcquireGameProcess(static_cast<unsigned>(proc_id));
	if (!process) return false;
	slave.Process = std::move(process);
	SettingsClass reg(APPLICATION_SETTINGS_SECTION_NET_SLAVE);
	if (reg.Is_Valid()) {
		char entry[128];
		sprintf(entry, "%s%d", KEY_SLAVE_RUNNING_ID, index);
		reg.Set_Int(entry, static_cast<int>(slave.Process->Id()));
	}
	WWDEBUG_SAY(("Slave found with process ID %u\n", slave.Process->Id()));
	return true;
}




/***********************************************************************************************
 * SlaveMasterClass::Startup_Slaves -- Create extra slave server processes                     *
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
 *   11/21/2001 3:46PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Startup_Slaves(void)
{
	if (!SlaveMode) {

		/*
		** Make sure we are a dedicated server.
		*/
		if (!The_Game() || The_Game()->IsDedicated.Is_True()) {

			/*
			** Slaves only available in windowed or console mode.
			*/
			if (WW3D::Is_Windowed() || ConsoleBox.Is_Exclusive()) {

				/*
				** Slaves only available in internet mode.
				*/
				GameModeClass *game_mode = GameModeManager::Find("WOL");
				if (game_mode && game_mode->Is_Active()) {

					Wait_For_Slave_Shutdown();

					Load();
					Delete_Settings_Copies();
					Create_Settings_Copies();

					/*
					** Spawn the servers.
					*/
					for (int i=0 ; i<NumSlaveServers ; i++) {
						if (SlaveServers[i].Enable) {

							bool slave_running = false;

							/*
							** Get an access point into the slaves settings base.
							*/
							char slave_name[64];
							sprintf(slave_name, "/slave_%d", i);
							strcpy(DefaultSettingsModifier, slave_name+1);
							SettingsClass slave_reg(APPLICATION_SETTINGS_SECTION);
							DefaultSettingsModifier[0] = 0;

							/*
							** If we are autostarting then take inventory of which slaves are running already.
							** An autostart after a crash should still see the slaves running. An autostart after a reboot will see no slaves.
							*/
							if (AutoRestart.Is_Active()) {
								slave_running = Aquire_Slave(i);
							}

							/*
							** Figure out the name of the .exe to run.
							*/
							std::vector<std::string> arguments{Platform::ExecutablePath(), "/MULTI", "/SLAVE", "/REGMOD=slave_" + std::to_string(i)};
							if (ConsoleBox.Is_Exclusive()) arguments.emplace_back("/NODX");

							int result = 1;
							if (!slave_running) {

								if (slave_reg.Is_Valid()) {
									slave_reg.Set_Int("ProcessId", 0);
								}
								SlaveServers[i].Process = Platform::StartProcess(arguments);
								result = SlaveServers[i].Process != nullptr;
							}
							if (result) {
								SlaveServers[i].IsRunning = true;


								if (!slave_running) {
									unsigned long time = TIMEGETTIME();
									while (TIMEGETTIME() - time < 10000) {
										if (!slave_reg.Is_Valid()) {
											break;
										}

										/*
										** Break out once we read the slaves process ID from the settings indicating that the slave
										** has already parsed its command line.
										*/
										int process_id = slave_reg.Get_Int("ProcessId", 0);
										if (process_id != 0) {
											break;
										}
										Platform::Sleep(250);
									}
								}

								/*
								** Set a settings flag to say this server is active. We need to know this if the master server (us)
								** crashes and restarts.
								*/
								SettingsClass reg(APPLICATION_SETTINGS_SECTION_NET_SLAVE);
								if (reg.Is_Valid()) {
									char entry[128];
									sprintf(entry, "%s%d", KEY_SLAVE_RUNNING_ID, i);
									reg.Set_Int(entry, SlaveServers[i].Process->Id());
								}

							} else {
								WWDEBUG_SAY(("Failed to start slave process - error: %s\n", SDL_GetError()));
								SlaveServers[i].IsRunning = false;
							}
						}
					}
				}
			}
		}
	}
	GameSideServerControlClass::Set_Welcome_Message();
}





/***********************************************************************************************
 * SlaveMasterClass::Shutdown_Slaves -- Send quit message to all slaves                        *
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
 *   11/21/2001 3:45PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Shutdown_Slaves(void)
{
	if (!SlaveMode) {
		char password[64] = DEFAULT_SERVER_CONTROL_PASSWORD;
		SettingsClass reg(APPLICATION_SETTINGS_SECTION_NET_SERVER_CONTROL);
		reg.Get_String(SERVER_CONTROL_PASSWORD_KEY, password, sizeof(password), password);

		for (int i=0 ; i<NumSlaveServers ; i++) {
			if (SlaveServers[i].IsRunning) {

				/*
				** In case the slave was restarted - we won't know it's new process ID.
				*/
				Aquire_Slave(i);

				/*
				** Set the slaves auto-restart flag to false or it will just start right up again.
				*/
				char slave_name[64];
				sprintf(slave_name, "/slave_%d", i);
				strcpy(DefaultSettingsModifier, slave_name+1);
				SettingsClass slave_reg(APPLICATION_SETTINGS_SECTION_WOLSETTINGS);
				DefaultSettingsModifier[0] = 0;
				slave_reg.Set_Int(AutoRestartClass::SETTING_AUTO_RESTART_FLAG, 0);

				/*
				** Send the password to the slave to authenticate the connection.
				*/
				GameSideServerControlClass::Send_Message(password, ntohl(INADDR_LOOPBACK), SlaveServers[i].ControlPort);
				Platform::Sleep(10);
				GameSideServerControlClass::Send_Message("quit", ntohl(INADDR_LOOPBACK), SlaveServers[i].ControlPort);

				/*
				** Remember that we shut this guy down.
				*/
				SettingsClass reg(APPLICATION_SETTINGS_SECTION_NET_SLAVE);
				if (reg.Is_Valid()) {
					char entry[128];
					sprintf(entry, "%s%d", KEY_SLAVE_RUNNING_ID, i);
					reg.Set_Int(entry, 0);
				}
			}
		}
	}
}






/***********************************************************************************************
 * SlaveMasterClass::Shutdown_Slaves -- Send quit message to all slaves                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Slave login name                                                                  *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:45PM ST : Created                                                            *
 *=============================================================================================*/
bool SlaveMasterClass::Shutdown_Slave(char *slave_login)
{
	if (!SlaveMode && slave_login) {
		char password[64] = DEFAULT_SERVER_CONTROL_PASSWORD;
		SettingsClass reg(APPLICATION_SETTINGS_SECTION_NET_SERVER_CONTROL);
		reg.Get_String(SERVER_CONTROL_PASSWORD_KEY, password, sizeof(password), password);

		for (int i=0 ; i<NumSlaveServers ; i++) {
			if (SlaveServers[i].IsRunning && stricmp(slave_login, SlaveServers[i].NickName) == 0) {

				/*
				** In case the slave was restarted - we won't know it's new process ID.
				*/
				Aquire_Slave(i);

				/*
				** Set the slaves auto-restart flag to false or it will just start right up again.
				*/
				char slave_name[64];
				sprintf(slave_name, "/slave_%d", i);
				strcpy(DefaultSettingsModifier, slave_name+1);
				SettingsClass slave_reg(APPLICATION_SETTINGS_SECTION_WOLSETTINGS);
				DefaultSettingsModifier[0] = 0;
				slave_reg.Set_Int(AutoRestartClass::SETTING_AUTO_RESTART_FLAG, 0);

				/*
				** Send the password to the slave to authenticate the connection.
				*/
				GameSideServerControlClass::Send_Message(password, ntohl(INADDR_LOOPBACK), SlaveServers[i].ControlPort);
				Platform::Sleep(10);
				GameSideServerControlClass::Send_Message("quit", ntohl(INADDR_LOOPBACK), SlaveServers[i].ControlPort);

				/*
				** Remember that we shut this guy down.
				*/
				SettingsClass reg(APPLICATION_SETTINGS_SECTION_NET_SLAVE);
				if (reg.Is_Valid()) {
					char entry[128];
					sprintf(entry, "%s%d", KEY_SLAVE_RUNNING_ID, i);
					reg.Set_Int(entry, 0);
				}
				SlaveServers[i].IsRunning = false;
				return(true);
			}
		}
	}
	return(false);
}




/***********************************************************************************************
 * SlaveMasterClass::Get_Slave_Info -- Get text slave info                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to text buffer                                                                *
 *           buffer size                                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:45PM ST : Created                                                            *
 *=============================================================================================*/
char *SlaveMasterClass::Get_Slave_Info(char *buffer, int buflen)
{
	bool any = false;
	if (buffer) {
		assert(buflen >= 500);
		*buffer = 0;

		for (int i=0 ; i<NumSlaveServers ; i++) {
			if (SlaveServers[i].IsRunning) {
				any = true;
				char temp[64];
				sprintf(temp, " Slave %d on port %d\n", i+1, SlaveServers[i].ControlPort);
				if (strlen(temp) + strlen(buffer) < (unsigned)buflen) {
					strcat(buffer, temp);
				}
			}
		}
		if (!any) {
			strcpy(buffer, "No slave servers active\n");
		}
	}
	return(buffer);
}




/***********************************************************************************************
 * SlaveMasterClass::Create_Settings_Copies -- Create 'shadow' settings copies for slaves      *
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
 *   11/21/2001 3:44PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Create_Settings_Copies(void)
{
	WWASSERT(!SlaveMode);

	/*
	** Make sure the Process ID isn't set in our base settings. It's shouldn't be unless I ran with the /slave command during dev.
	*/
	SettingsClass reg(APPLICATION_SETTINGS_SECTION);
	if (reg.Is_Valid()) {
		reg.Delete_Value("ProcessId");
	}

	SettingsClass::Save_Settings(SettingsFileName, APPLICATION_SETTINGS_SECTION);

	char new_path[1024];
	char slave_name[64];

	for (int i=0 ; i<NumSlaveServers ; i++) {
		if (SlaveServers[i].Enable) {
			strcpy(new_path, APPLICATION_SETTINGS_SECTION);
			sprintf(slave_name, "/slave_%d", i);
			strcat(new_path, slave_name);
			SettingsClass::Load_Settings(SettingsFileName, APPLICATION_SETTINGS_SECTION, new_path);

			/*
			** Store the slave settings into the settings.
			*/

			/*
			** Port numbers.
			*/
			{
				strcpy(DefaultSettingsModifier, slave_name+1);
				SettingsClass reg(APPLICATION_SETTINGS_SECTION_NET_FIREWALL);
				DefaultSettingsModifier[0] = 0;
				SettingsClass my_reg(APPLICATION_SETTINGS_SECTION_NET_FIREWALL);

				if (SlaveServers[i].Port != 0) {
					reg.Set_Int("ForcePort", SlaveServers[i].Port);
				} else {
					reg.Set_Int("ForcePort", 0);

					int port = my_reg.Get_Int("PortBase", PORT_BASE_MIN);
					port = port + ((i+1) * 256);
					if (port >= PORT_BASE_MAX-1) {
						port -= (PORT_BASE_MAX - PORT_BASE_MIN);
					}
					reg.Set_Int("PortBase", port);

					port = my_reg.Get_Int("PortPool", PORT_BASE_MIN);
					port = port + ((i+1) * 1024);
					if (port >= PORT_POOL_MAX-1) {
						port -= (PORT_POOL_MAX - PORT_POOL_MIN);
					}
					reg.Set_Int("PortPool", port);
				}
			}


			/*
			** Server control info.
			*/
			{
				strcpy(DefaultSettingsModifier, slave_name+1);
				SettingsClass reg(APPLICATION_SETTINGS_SECTION_NET_SERVER_CONTROL);
				DefaultSettingsModifier[0] = 0;
				SettingsClass my_reg(APPLICATION_SETTINGS_SECTION_NET_SERVER_CONTROL);

				/*
				** The password will be the same for all slaves but they each need a port to listen on.
				*/
				int my_sc_port = my_reg.Get_Int(SERVER_CONTROL_PORT_KEY, DEFAULT_SERVER_CONTROL_PORT);
				int slave_port = my_sc_port;
				if (my_sc_port == 0) {
					/*
					** If server control isn't enabled for me then we need to make up some port.
					*/
					slave_port = DEFAULT_SERVER_CONTROL_PORT;
				}
				slave_port += i;
				slave_port++;
				SlaveServers[i].ControlPort = slave_port;
				reg.Set_Int(SERVER_CONTROL_PORT_KEY, slave_port);

				/*
				** Inherit this from the master now.
				*/
				//if (my_sc_port == 0) {
				//	reg.Set_Int(SERVER_CONTROL_LOOPBACK_KEY, 1);
				//} else {
				//	reg.Set_Int(SERVER_CONTROL_LOOPBACK_KEY, 0);
				//}
			}

			/*
			** Login name.
			*/
			{
				strcpy(DefaultSettingsModifier, slave_name+1);
				SettingsClass reg(APPLICATION_SETTINGS_SECTION_WOLSETTINGS);
				DefaultSettingsModifier[0] = 0;

				reg.Set_String("AutoLogin", SlaveServers[i].NickName);
				reg.Set_String("LastLogin", SlaveServers[i].NickName);
			}

			/*
			** Password name.
			*/
			{
				strcpy(DefaultSettingsModifier, slave_name+1);
				SettingsClass reg(APPLICATION_SETTINGS_SECTION_WOLSETTINGS);
				DefaultSettingsModifier[0] = 0;

				reg.Set_String("AutoPassword", SlaveServers[i].Password);
			}


			/*
			** Serial number.
			*/
			{
				strcpy(DefaultSettingsModifier, slave_name+1);
				SettingsClass reg(APPLICATION_SETTINGS_SECTION);
				DefaultSettingsModifier[0] = 0;

				StringClass serial(SlaveServers[i].Serial, true);
				StringClass encrypted_serial = serial;
				if (serial.Get_Length()) {
					ServerSettingsClass::Encrypt_Serial(serial, encrypted_serial);
				}
				reg.Set_String(KEY_SLAVE_SERIAL, encrypted_serial.Peek_Buffer());
			}

			/*
			** Make it autostart.
			*/
			{
				strcpy(DefaultSettingsModifier, slave_name+1);
				SettingsClass reg(APPLICATION_SETTINGS_SECTION_WOLSETTINGS);
				DefaultSettingsModifier[0] = 0;

				if (reg.Is_Valid()) {
					reg.Set_Int(AutoRestartClass::SETTING_AUTO_RESTART_FLAG, 1);

					int game_type = 0;
					GameModeClass *game_mode = GameModeManager::Find("WOL");
					if (game_mode && game_mode->Is_Active()) {
						game_type = 1;
					}
					reg.Set_Int(AutoRestartClass::SETTING_AUTO_RESTART_TYPE, game_type);
				}
			}

			/*
			** Tell it which multiplayer settings to use.
			*/
			{
				strcpy(DefaultSettingsModifier, slave_name+1);
				SettingsClass reg(APPLICATION_SETTINGS_SECTION_OPTIONS);
				DefaultSettingsModifier[0] = 0;
				reg.Set_String("MultiplayerSettings", SlaveServers[i].SettingsFileName);
			}

			/*
			** Set the SKU number to be the FDS SKU. Do this whether the Master is a FDS or not.
			*/
			{
				strcpy(DefaultSettingsModifier, slave_name+1);
				SettingsClass reg(APPLICATION_SETTINGS_SECTION);
				DefaultSettingsModifier[0] = 0;
				reg.Set_Int("SKU", RENEGADE_FDS_SKU);
			}

			/*
			** Set the bandwidth information.
			** A value of 0 means auto. A value of 0xffffffff means not specified (i.e. use master settings).
			*/
			{
				int bw = SlaveServers[i].Bandwidth;
				if (bw != -1) {
					strcpy(DefaultSettingsModifier, slave_name+1);
					SettingsClass reg_netopt(APPLICATION_SETTINGS_SECTION_NETOPTIONS);
					SettingsClass reg_bw(APPLICATION_SETTINGS_SECTION_BANDTEST);
					DefaultSettingsModifier[0] = 0;
					SettingsClass my_reg_netopt(APPLICATION_SETTINGS_SECTION_NETOPTIONS);
					SettingsClass my_reg_bw(APPLICATION_SETTINGS_SECTION_BANDTEST);

					//reg_netopt.Set_Int("BandwidthType", BANDWIDTH_AUTO);
					cUserOptions::Set_Bandwidth_Type(BANDWIDTH_AUTO);
					int slave_bw = bw;

					/*
					** If bandwidth is set to auto then divide it by the number of servers on this box.
					*/
					if (slave_bw == 0) {
						slave_bw = my_reg_bw.Get_Int("Up", 0);
						int num = Get_Num_Enabled_Slaves();
						if (num) {
							slave_bw = slave_bw / (num+1);
						}
					}
					reg_bw.Set_Int("Up", slave_bw);
					reg_bw.Set_Int("Down", slave_bw);
				}
			}


#if (0)
			/*
			** Give the window a different position so we are not completely overlapping.
			*/
			{
				strcpy(DefaultSettingsModifier, slave_name+1);
				SettingsClass reg(APPLICATION_SETTINGS_SECTION_OPTIONS);
				DefaultSettingsModifier[0] = 0;
				reg.Set_Int("WindowX", (i * 32) + 32);
				reg.Set_Int("WindowY", (i * 32) + 32);
			}
#endif //(0)
		}
	}
}





/***********************************************************************************************
 * SlaveMasterClass::Delete_Settings_Copies -- Delete old slave settings copies                *
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
 *   11/21/2001 3:44PM ST : Created                                                            *
 *=============================================================================================*/
void SlaveMasterClass::Delete_Settings_Copies(void)
{
	{
		int index = 0;
		char new_path[1024];
		char slave_name[64];

		while (index < MAX_SLAVES) {
			strcpy(new_path, APPLICATION_SETTINGS_SECTION);
			sprintf(slave_name, "/slave_%d", index);
			strcat(new_path, slave_name);
			SettingsClass::Delete_Settings_Tree(new_path);
			index++;
		}
	}
}



/***********************************************************************************************
 * SlaveMasterClass::Get_Num_Enabled_Slaves -- How many slaves are enabled?                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Number of enabled slaves                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/5/2002 11:47PM ST : Created                                                             *
 *=============================================================================================*/
int SlaveMasterClass::Get_Num_Enabled_Slaves(void)
{
	int enabled = 0;

	for (int i=0 ; i<NumSlaveServers ; i++) {
		if (SlaveServers[i].Enable) {
			enabled++;
		}
	}
	return(enabled);
}
