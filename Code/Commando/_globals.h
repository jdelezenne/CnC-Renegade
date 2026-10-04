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
 *                     $Archive:: /Commando/Code/Commando/_globals.h                           $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/06/02 5:36p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "specialbuilds.h"

#if	defined(FREEDEDICATEDSERVER)
#define APP_SETTINGS_SECTION "RenegadeFDS"
#elif defined(MULTIPLAYERDEMO)
#define APP_SETTINGS_SECTION "RenegadeMPDemo"
#elif defined(BETACLIENT)
#define APP_SETTINGS_SECTION "RenegadeBeta"
#elif defined(BETASERVER)
#define APP_SETTINGS_SECTION "RenegadeBeta"
#else
#define APP_SETTINGS_SECTION "Renegade"
#endif


extern char *Build_Settings_Section(const char *base, const char *modifier, const char *sub);

#define	APPLICATION_SETTINGS_SECTION							Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "")

#define	APPLICATION_SETTINGS_SECTION_RENDER					Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Render")
#define	APPLICATION_SETTINGS_SECTION_OPTIONS					Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Options")
#define	APPLICATION_SETTINGS_SECTION_DEBUG					Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Debug")
#define	APPLICATION_SETTINGS_SECTION_SYSTEM_SETTINGS		Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "System Settings")
#define	APPLICATION_SETTINGS_SECTION_CONTROLS				Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Controls")
#define	APPLICATION_SETTINGS_SECTION_SOUND					Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Sound")
#define	APPLICATION_SETTINGS_SECTION_MOVIES					Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Movies")
#define	APPLICATION_SETTINGS_SECTION_WOLSETTINGS			Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "WOLSettings")
#define	APPLICATION_SETTINGS_SECTION_MISSION_RANKS			Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Ranks")
#define	APPLICATION_SETTINGS_SECTION_INPUT					Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Input")
#define	APPLICATION_SETTINGS_SECTION_GAMESPY					Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "GameSpy")
#define	APPLICATION_SETTINGS_SECTION_WOLSETTINGS			Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "WOLSettings")
#define	APPLICATION_SETTINGS_SECTION_URL						Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "WOLSettings\\URL")
#define	APPLICATION_SETTINGS_SECTION_LOGINS					Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "WOLSettings\\Logins")
#define	APPLICATION_SETTINGS_SECTION_QUICKMATCH				Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "WOLSettings\\QuickMatch")
#define	APPLICATION_SETTINGS_SECTION_IGNORE_LIST			Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "WOLSettings\\Ignore List")
#define	APPLICATION_SETTINGS_SECTION_SERVER_LIST			Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "WOLSettings\\Servers")
#define	APPLICATION_SETTINGS_SECTION_SKIN_LIST				Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "MP Settings\\Skins")

#define	APPLICATION_SETTINGS_SECTION_NETOPTIONS				Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Networking\\Options")
#define	APPLICATION_SETTINGS_SECTION_NETDEBUG				Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Networking\\Debug")
#define	APPLICATION_SETTINGS_SECTION_NET_FIREWALL			Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Networking\\Firewall")
#define	APPLICATION_SETTINGS_SECTION_NET_SLAVE				Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Networking\\Slave")
#define	APPLICATION_SETTINGS_SECTION_NET_SERVER_CONTROL	Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Networking\\ServerControl")

#define	COMBAT_SETTINGS_SECTION_DEBUG							Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Debug")

#define	APPLICATION_SETTINGS_SECTION_BANDTEST				Build_Settings_Section(APP_SETTINGS_SECTION, NULL, "Bandtest")

#define  RENEGADE_BASE_SKU										3072
#define	RENEGADE_FDS_SKU										12288
#define	RENEGADE_DEMO_SKU										13056


#endif