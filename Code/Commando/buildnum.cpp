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
 *                     $Archive:: /Commando/Code/Commando/buildnum.cpp                        $*
 *                                                                                             *
 *                       Author:: Steve Tall                                                   *
 *                                                                                             *
 *                     $Modtime:: 10/29/01 10:06p                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "buildnum.h"
#include "wwdebug.h"
#include <stdio.h>
#include <chrono>
#include <cstdint>
#include <cstring>

/*
**
** This is just a placeholder for the build number.
** A post build step will stamp the build number into here.
**
*/
char BuildInfoClass::BuildNumber [64] = {"Insert1Build2Number3Here4   xxxx                               "};
char BuildInfoClass::BuildDate   [64] = {"Insert1Build2Date3Here4     xxxx                               "};

/*
** Unreachable code warning.
*/
#pragma warning(disable : 4702)

/***********************************************************************************************
 * BuildInfoClass::Get_Build_Number -- Gets the 32 bit build number.                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Build number                                                                      *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 4:54PM ST : Created                                                            *
 *=============================================================================================*/
unsigned long BuildInfoClass::Get_Build_Number(void)
{
	std::uint32_t number;
	std::memcpy(&number, &BuildNumber[28], sizeof(number));
	return number;
}




/***********************************************************************************************
 * BuildInfoClass::Get_Build_Number_String -- Gets the build number as a human readable string.*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to build number string                                                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 4:55PM ST : Created                                                            *
 *=============================================================================================*/
char *BuildInfoClass::Get_Build_Number_String(void)
{
	static char _buffer[16];
	snprintf(_buffer, sizeof(_buffer), "%lu", Get_Build_Number());
	return (_buffer);
}




/***********************************************************************************************
 * BuildInfoClass::Get_Builder_Name -- Gets the name of the person who built this executable.  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to builder name string                                                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 5:08PM ST : Created                                                            *
 *=============================================================================================*/
char *BuildInfoClass::Get_Builder_Name(void)
{
	return(&BuildNumber[32]);
}



/***********************************************************************************************
 * BuildInfoClass::Get_Build_Date_String -- Gets the date this executable was built on.        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to build date string                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 5:10PM ST : Created                                                            *
 *=============================================================================================*/
char *BuildInfoClass::Get_Build_Date_String(void)
{
	static char _buffer[64];
	std::uint64_t ticks;
	std::memcpy(&ticks, &BuildDate[28], sizeof(ticks));
	_buffer[0] = 0;
	if (ticks < (std::uint64_t{1} << 63)) {
		using namespace std::chrono;
		const auto time = sys_days{year{1601}/January/1} + seconds{static_cast<std::int64_t>(ticks / 10000000)};
		const auto day = floor<days>(time);
		const year_month_day date{day};
		const hh_mm_ss clock{time - day};
		snprintf(_buffer, sizeof(_buffer), "%02u/%02u/%04d - %02lld:%02lld:%02lld",
			static_cast<unsigned>(date.month()), static_cast<unsigned>(date.day()), static_cast<int>(date.year()),
			static_cast<long long>(clock.hours().count()), static_cast<long long>(clock.minutes().count()),
			static_cast<long long>(clock.seconds().count()));
	}

	return(_buffer);
}



/***********************************************************************************************
 * BuildInfoClass::Get_Builder_Initials -- Gets the initials of the builder                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to string containing initials                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 5:12PM ST : Created                                                            *
 *=============================================================================================*/
char *BuildInfoClass::Get_Builder_Initials(void)
{
	static char _buffer[4];

	_buffer[0] = BuildNumber[32];
	_buffer[1] = 0;
	_buffer[2] = 0;

	char *lastname = strchr(&BuildNumber[32], '_');
	if (lastname) {
		_buffer[1] = lastname[1];
	}
	return (_buffer);
}

/***********************************************************************************************
 * BuildInfoClass::Get_Build_Version_String -- Get composite version as string                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Version string                                                                    *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 7:30PM ST : Created                                                            *
 *=============================================================================================*/
char *BuildInfoClass::Get_Build_Version_String(void)
{
	static char _buffer[128];
	sprintf(_buffer, "%s-%s", Get_Builder_Initials(), Get_Build_Number_String());
	return(_buffer);
}




/***********************************************************************************************
 * BuildInfoClass::Get_Build_Type -- Get the type of build this is                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Build type                                                                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 7:40PM ST : Created                                                            *
 *=============================================================================================*/
BuildInfoClass::BuildType BuildInfoClass::Get_Build_Type(void)
{
	#ifndef _DEBUG
		#ifdef WWDEBUG
			return(BUILD_PROFILE);
		#else //WWDEBUG
			return(BUILD_RELEASE);
		#endif //WWDEBUG
	#else //_DEBUG
		return(BUILD_DEBUG);
	#endif //_DEBUG
}



/***********************************************************************************************
 * BuildInfoClass::Get_Build_Type_String -- Get the build type as a string.                    *
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
 *   10/29/2001 7:42PM ST : Created                                                            *
 *=============================================================================================*/
const char *BuildInfoClass::Get_Build_Type_String(void)
{
	switch (Get_Build_Type()) {
		case BUILD_DEBUG:
			return("DEBUG");

		case BUILD_RELEASE:
			return("RELEASE");

		case BUILD_PROFILE:
			return("PROFILE");

		default:
			WWASSERT(false);
			return("UNKNOWN");
	}
}





/***********************************************************************************************
 * BuildInfoClass::Composite_Build_Info -- Get lots of build info                              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to info                                                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/2001 7:49PM ST : Created                                                            *
 *=============================================================================================*/
char *BuildInfoClass::Composite_Build_Info(void)
{
	static char _buffer[256];
	snprintf(_buffer, sizeof(_buffer), "%s Build %lu by %s - Build time %s", Get_Build_Type_String(), Get_Build_Number(), Get_Builder_Name(), Get_Build_Date_String());
	return(_buffer);
}



/***********************************************************************************************
 * BuildInfoClass::Log_Build_Info -- Dump build info to the logfile.                           *
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
 *   10/29/2001 7:49PM ST : Created                                                            *
 *=============================================================================================*/
void BuildInfoClass::Log_Build_Info(void)
{
	WWDEBUG_SAY((Composite_Build_Info()));
	WWDEBUG_SAY(("\n"));
}




