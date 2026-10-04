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
*     $Archive: /Commando/Code/wwlib/LaunchWeb.cpp $
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 2 $
*     $Modtime: 6/22/01 4:39p $
*
******************************************************************************/

#include "Platform/Windows/Files.h"
#include "LaunchWeb.h"
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <assert.h>

/******************************************************************************
*
* NAME
*     LaunchWebBrowser
*
* DESCRIPTION
*     Launch the default browser to view the specified URL
*
* INPUTS
*     URL      - Website address
*     Wait     - Wait for user to close browser (default = false)
*     Callback - User callback to invoke during wait (default = NULL callback)
*
* RESULT
*     Success - True if successful; otherwise false
*
******************************************************************************/

bool LaunchWebBrowser(const char* url)
{
    return url && *url && reinterpret_cast<INT_PTR>(ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL)) > 32;
}
