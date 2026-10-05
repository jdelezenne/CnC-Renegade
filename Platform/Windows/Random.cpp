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


#include "srandom.h"
#include "sha.h"
#include "win.h"
#include <process.h>
#include <ctime>
#include <cstring>

void SecureRandomClass::Generate_Seed(void)
{
	int i;

	// Start with some garbage values
	memset(Seeds, 0xAA, SeedLength);

	unsigned int *int_seeds=(unsigned int *)Seeds;
	int int_seed_length=SeedLength/sizeof(unsigned int);


	//
	// Get free drive space
	//
	DWORD spc, bps, nfc, tnc;	// various drive attributes (we don't care what they mean)
	GetDiskFreeSpace(NULL, &spc, &bps, &nfc, &tnc);
	int_seeds[0]^=spc;
	int_seeds[1 % int_seed_length]^=bps;
	int_seeds[2 % int_seed_length]^=nfc;
	int_seeds[3 % int_seed_length]^=tnc;

	//
	// Get computer & user name
	//
	char	comp_name[128];
	char	user_name[128];
	DWORD	comp_len=128;
	DWORD	name_len=128;

	GetComputerName(comp_name, &comp_len);
	GetUserName(user_name, &name_len);
	for (i=0; i<128; i++)
	{
		// Offset in case user_name == comp_name
		Seeds[(i+0) % SeedLength]^=comp_name[i];
		Seeds[(i+2) % SeedLength]^=user_name[i];
	}


	for (i=0; i<int_seed_length; i++)
	{
		if ((i % 4) == 0)
			int_seeds[i]^=time(NULL);
		else if ((i % 4) == 1)
			int_seeds[i]^=getpid();
		else if ((i % 4) == 2)
			int_seeds[i]^=GetTickCount();
		else if ((i % 4) == 3)
			int_seeds[i]^=i;
	}
}