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

//
// SecureRandomClass - Generate random values
//

#pragma warning(disable : 4514)	// unreferenced inline function removed....

#include "srandom.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "sha.h"

// Static class variables
unsigned char	SecureRandomClass::Seeds[SecureRandomClass::SeedLength];
bool				SecureRandomClass::Initialized=false;
unsigned int	SecureRandomClass::RandomCache[SecureRandomClass::SHADigestBytes / sizeof(unsigned int)];
int				SecureRandomClass::RandomCacheEntries=0;
unsigned int	SecureRandomClass::Counter=0;
Random3Class	SecureRandomClass::RandomHelper;

SecureRandomClass::SecureRandomClass()
{
	if (Initialized == false)
	{
		Generate_Seed();
		Initialized=true;
	}
}

SecureRandomClass::~SecureRandomClass()
{
}


//
// Add seed values to our pool of randomness
//
void SecureRandomClass::Add_Seeds(unsigned char *values, int length)
{
	for (int i=0; i<length; i++)
	{
		Seeds[0]^=values[i];

		// Rotate the seeds to the left
		unsigned char uctemp=Seeds[SeedLength-1];
		for (int j=SeedLength-1; j>=1; j--)
			Seeds[j]=Seeds[j-1];
		Seeds[0]=uctemp;
	}

	// We have a better seed pool now so trigger new random values
	RandomCacheEntries=0;
}

//
// Get a 32bit random value
//
unsigned long SecureRandomClass::Randval(void)
{
	if (RandomCacheEntries == 0)
	{
		SHAEngine sha;
		char digest[SHADigestBytes];	// SHA produces a 20 byte hash

		sha.Hash(Seeds, SeedLength);
		sha.Result(digest);

		memcpy(RandomCache, digest, SHADigestBytes);
		RandomCacheEntries=(SHADigestBytes / sizeof(unsigned int));

		unsigned int *int_seeds=(unsigned int *)Seeds;
		int_seeds[0]^=Counter;			// remove the last counter (double xor)
		int_seeds[0]^=(Counter+1);		// put the new counter in place

		int_seeds[(SeedLength/sizeof(int))-1]^=Counter;		// remove the last counter (double xor)
		int_seeds[(SeedLength/sizeof(int))-1]^=(Counter+1);	// put the new counter in place

		Counter++;				// increment counter
	}

	unsigned long retval=RandomCache[--RandomCacheEntries];

	// SHA doesn't have the best distribution properties in the world
	//   We'll XOR the result with the output of another random number
	unsigned long helperval=RandomHelper();
	retval^=helperval;

	return(retval);
}
