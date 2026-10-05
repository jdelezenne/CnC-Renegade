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

#include "cpudetect.h"
#include "Platform/SystemInfo.h"
#include <windows.h>

struct OSInfoStruct {
	const char* Code;
	const char* SubCode;
	const char* VersionString;
	unsigned char VersionMajor;
	unsigned char VersionMinor;
	unsigned short VersionSub;
	unsigned char BuildMajor;
	unsigned char BuildMinor;
	unsigned short BuildSub;
/*	OSInfoStruct() {}
	OSInfoStruct(
		const char* code,
		const char* subcode,
		const char* versionstring,
		unsigned char versionmajor,
		unsigned char versionminor,
		unsigned short versionsub,
		unsigned char buildmajor,
		unsigned char buildminor,
		unsigned short buildsub) :
		Code(code),
		SubCode(subcode),
		VersionString(versionstring),
		VersionMajor(versionmajor),
		VersionMinor(versionminor),
		VersionSub(versionsub),
		BuildMajor(buildmajor),
		BuildMinor(buildminor),
		BuildSub(buildsub)
	{
	}
*/
};

static void Get_OS_Info(
	OSInfoStruct& os_info,
	unsigned OSVersionPlatformId,
	unsigned OSVersionNumberMajor,
	unsigned OSVersionNumberMinor,
	unsigned OSVersionBuildNumber);


void CPUDetectClass::Init_Memory()
{
	MEMORYSTATUS mem;
	GlobalMemoryStatus(&mem);
	TotalPhysicalMemory=mem.dwTotalPhys;
	AvailablePhysicalMemory=mem.dwAvailPhys;
	TotalPageMemory=mem.dwTotalPageFile;
	AvailablePageMemory=mem.dwAvailPageFile;
	TotalVirtualMemory=mem.dwTotalVirtual;
	AvailableVirtualMemory=mem.dwAvailVirtual;
}

void CPUDetectClass::Init_OS()
{
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize=sizeof(os);
	GetVersionEx(&os);

	OSVersionNumberMajor=os.dwMajorVersion;
	OSVersionNumberMinor=os.dwMinorVersion;
	OSVersionBuildNumber=os.dwBuildNumber;
	OSVersionPlatformId=os.dwPlatformId;
	OSVersionExtraInfo=os.szCSDVersion;
}

#define COMPACTLOG(n) work.Format n ; CPUDetectClass::CompactLog+=work;

void CPUDetectClass::Init_Compact_Log()
{
	StringClass work(0,true);

	TIME_ZONE_INFORMATION time_zone;
	GetTimeZoneInformation(&time_zone);
	COMPACTLOG(("%d\t",time_zone.Bias));

	OSInfoStruct os_info;
	Get_OS_Info(os_info,OSVersionPlatformId,OSVersionNumberMajor,OSVersionNumberMinor,OSVersionBuildNumber);
	COMPACTLOG(("%s\t",os_info.Code));

	if (!stricmp(os_info.SubCode,"UNKNOWN")) {
		COMPACTLOG(("%d\t",OSVersionBuildNumber&0xffff));
	}
	else {
		COMPACTLOG(("%s\t",os_info.SubCode));
	}

	COMPACTLOG(("%s\t%d\t",Get_Processor_Manufacturer_Name(),Get_Processor_Speed()));

	COMPACTLOG(("%d\t",Get_Total_Physical_Memory()/(1024*1024)+1));

	COMPACTLOG(("%x\t%x\t",Get_Feature_Bits(),Get_Extended_Feature_Bits()));
}

OSInfoStruct Windows9xVersionTable[]={
	{"WIN95",	"FINAL",		"Windows 95",								4,0,950,			4,0,950		},
	{"WIN95",	"A",			"Windows 95a OSR1 final Update",		4,0,950,			4,0,951		},
	{"WIN95",	"B20OEM",	"Windows 95B OSR 2.0 final OEM",		4,0,950,			4,0,1111		},
	{"WIN95",	"B20UPD",	"Windows 95B OSR 2.1 final Update",	4,0,950,			4,3,1212		},
	{"WIN95",	"B21OEM",	"Windows 95B OSR 2.1 final OEM",		4,1,971,			4,1,971		},
	{"WIN95",	"C25OEM",	"Windows 95C OSR 2.5 final OEM",		4,0,950,			4,3,1214		},
	{"WIN98",	"BETAPRD",	"Windows 98 Beta pre-DR",				4,10,1351,		4,10,1351 	},
	{"WIN98",	"BETADR",	"Windows 98 Beta DR",					4,10,1358,		4,10,1358 	},
	{"WIN98",	"BETAE",		"Windows 98 early Beta",				4,10,1378,		4,10,1378 	},
	{"WIN98",	"BETAE",		"Windows 98 early Beta",				4,10,1410,		4,10,1410 	},
	{"WIN98",	"BETAE",		"Windows 98 early Beta",				4,10,1423,		4,10,1423 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1500,		4,10,1500 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1508,		4,10,1508	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1511,		4,10,1511 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1525,		4,10,1525 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1535,		4,10,1535 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1538,		4,10,1538 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1543,		4,10,1543 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1544,		4,10,1544 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1546,		4,10,1546 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1550,		4,10,1550 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1559,		4,10,1559 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1564,		4,10,1564 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1569,		4,10,1569 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1577,		4,10,1577 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1581,		4,10,1581 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1593,		4,10,1593 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1599,		4,10,1599 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1602,		4,10,1602 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1605,		4,10,1605 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1614,		4,10,1614 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1619,		4,10,1619 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1624,		4,10,1624 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1629,		4,10,1629 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1633,		4,10,1633 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1650,		4,10,1650 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1650,/*,3*/4,10,1650/*,3*/	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1650,/*,8*/4,10,1650/*,8*/	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1666,		4,10,1666 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1671,		4,10,1671 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1677,		4,10,1677 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1681,		4,10,1681 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1687,		4,10,1687 	},
	{"WIN98",	"RC0",		"Windows 98 RC0",							4,10,1691,		4,10,1691 	},
	{"WIN98",	"RC0",		"Windows 98 RC0",							4,10,1702,		4,10,1702 	},
	{"WIN98",	"RC0",		"Windows 98 RC0",							4,10,1708,		4,10,1708 	},
	{"WIN98",	"RC0",		"Windows 98 RC0",							4,10,1713,		4,10,1713 	},
	{"WIN98",	"RC1",		"Windows 98 RC1",							4,10,1721,/*,3*/4,10,1721/*,3*/	},
	{"WIN98",	"RC2",		"Windows 98 RC2",							4,10,1723,/*,4*/4,10,1723/*,4*/	},
	{"WIN98",	"RC2",		"Windows 98 RC2",							4,10,1726,		4,10,1726	},
	{"WIN98",	"RC3",		"Windows 98 RC3",							4,10,1900,/*,5*/4,10,1900/*,5*/	},
	{"WIN98",	"RC4",		"Windows 98 RC4",							4,10,1900,/*,8*/4,10,1900/*,8*/	},
	{"WIN98",	"RC5",		"Windows 98 RC5",							4,10,1998,		4,10,1998	},
	{"WIN98",	"FINAL",		"Windows 98",								4,10,1998,/*,6*/4,10,1998/*,6*/	},
	{"WIN98",	"SP1B1",		"Windows 98 SP1 Beta 1",				4,10,2088,		4,10,2088	},
	{"WIN98",	"OSR1B1",	"Windows 98 OSR1 Beta 1",				4,10,2106,		4,10,2106 	},
	{"WIN98",	"OSR1B1",	"Windows 98 OSR1 Beta 1",				4,10,2120,		4,10,2120 	},
	{"WIN98",	"OSR1B1",	"Windows 98 OSR1 Beta 1",				4,10,2126,		4,10,2126 	},
	{"WIN98",	"OSR1B1",	"Windows 98 OSR1 Beta 1",				4,10,2131,		4,10,2131 	},
	{"WIN98",	"SP1B2",		"Windows 98 SP1 Beta 2",				4,10,2150,/*,0*/4,10,2150/*,0*/	},
	{"WIN98",	"SP1B2",		"Windows 98 SP1 Beta 2",				4,10,2150,/*,4*/4,10,2150/*,4*/	},
	{"WIN98",	"SP1",		"Windows 98 SP1 final Update",		4,10,2000,		4,10,2000 	},
	{"WIN98",	"OSR1B2",	"Windows 98 OSR1 Beta 2",				4,10,2174,		4,10,2174 	},
	{"WIN98",	"SERC1",		"Windows 98 SE RC1",						4,10,2183,		4,10,2183 	},
	{"WIN98",	"SERC2",		"Windows 98 SE RC2",						4,10,2185,		4,10,2185 	},
	{"WIN98",	"SE",			"Windows 98 SE",							4,10,2222,		4,10,2222/*,3*/	},
	{"WINME",	"MEBDR1",	"Windows ME Beta DR1",					4,90,2332,		4,90,2332 	},
	{"WINME",	"MEBDR2",	"Windows ME Beta DR2",					4,90,2348,		4,90,2348 	},
	{"WINME",	"MEBDR3",	"Windows ME Beta DR3",					4,90,2358,		4,90,2358 	},
	{"WINME",	"MEBDR4",	"Windows ME Beta DR4",					4,90,2363,		4,90,2363 	},
	{"WINME",	"MEEB",		"Windows ME early Beta",				4,90,2368,		4,90,2368 	},
	{"WINME",	"MEEB",		"Windows ME early Beta",				4,90,2374,		4,90,2374 	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2380,		4,90,2380 	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2394,		4,90,2394 	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2399,		4,90,2399 	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2404,		4,90,2404 	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2410,		4,90,2410	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2416,		4,90,2416	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2419,/*,4*/4,90,2419/*,4*/	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2429,		4,90,2429 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2434,		4,90,2434 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2443,		4,90,2443 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2447,		4,90,2447 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2455,		4,90,2455 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2460,		4,90,2460 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2465,		4,90,2465 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2470,		4,90,2470 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2474,		4,90,2474 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2481,		4,90,2481 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2487,		4,90,2487 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2491,		4,90,2491 	},
	{"WINME",	"MEB3",		"Windows ME Beta 3",						4,90,2499,		4,90,2499 	},
	{"WINME",	"MEB3",		"Windows ME Beta 3",						4,90,2499,/*,3*/4,90,2499/*,3*/	},
	{"WINME",	"MEB3",		"Windows ME Beta 3",						4,90,2509,		4,90,2509 	},
	{"WINME",	"MEB3",		"Windows ME Beta 3",						4,90,2513,		4,90,2513 	},
	{"WINME",	"MEB3",		"Windows ME Beta 3",						4,90,2516,		4,90,2516 	},
	{"WINME",	"RC0",		"Windows ME RC0",							4,90,2525,		4,90,2525 	},
	{"WINME",	"RC1",		"Windows ME RC1",							4,90,2525,/*,6*/4,90,2525/*,6*/	},
	{"WINME",	"RC2",		"Windows ME RC2",							4,90,2535,		4,90,2535	},
	{"WINME",	"FINAL",		"Windows ME",								4,90,3000,/*,2*/4,90,3000/*,2*/	},
};

void Get_OS_Info(
	OSInfoStruct& os_info,
	unsigned OSVersionPlatformId,
	unsigned OSVersionNumberMajor,
	unsigned OSVersionNumberMinor,
	unsigned OSVersionBuildNumber)
{
	unsigned build_major=(OSVersionBuildNumber&0xff000000)>>24;
	unsigned build_minor=(OSVersionBuildNumber&0xff0000)>>16;
	unsigned build_sub=(OSVersionBuildNumber&0xffff);

	// Keep all strings valid when the OS is newer than the version table.
	os_info = {"UNKNOWN", "UNKNOWN", "UNKNOWN", 0, 0, 0, 0, 0, 0};

	switch (OSVersionPlatformId) {
	default:
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		{
			for(int i=0;i<sizeof(Windows9xVersionTable)/sizeof(os_info);++i) {
				if (
					Windows9xVersionTable[i].VersionMajor==OSVersionNumberMajor &&
					Windows9xVersionTable[i].VersionMinor==OSVersionNumberMinor &&
					Windows9xVersionTable[i].BuildMajor==build_major &&
					Windows9xVersionTable[i].BuildMinor==build_minor &&
					Windows9xVersionTable[i].BuildSub==build_sub) {
					os_info=Windows9xVersionTable[i];
					return;
				}
			}

			os_info.BuildMajor=build_major;
			os_info.BuildMinor=build_minor;
			os_info.BuildSub=build_sub;
			if (OSVersionNumberMajor==4) {
//				os_info.SubCode.Format("%d",build_sub);
				os_info.SubCode="UNKNOWN";
				if (OSVersionNumberMinor==0) {
					os_info.Code="WIN95";
					return;
				}
				if (OSVersionNumberMinor==10) {
					os_info.Code="WIN98";
					return;
				}
				if (OSVersionNumberMinor==90) {
					os_info.Code="WINME";
					return;
				}
				os_info.Code="WIN9X";
				return;
			}
		}
		break;
	case VER_PLATFORM_WIN32_NT:
//		os_info.SubCode.Format("%d",build_sub);
		os_info.SubCode="UNKNOWN";
		if (OSVersionNumberMajor==4) {
			os_info.Code="WINNT";
			return;
		}
		if (OSVersionNumberMajor==5) {
			if (OSVersionNumberMinor==0) {
				os_info.Code="WIN2K";
				return;
			}
			if (OSVersionNumberMinor==1) {
				os_info.Code="WINXP";
				return;
			}
			os_info.Code="WINXX";
			return;
		}
	}
}
const char* Platform::OperatingSystemName(unsigned platform)
{
    switch (platform) {
    case VER_PLATFORM_WIN32s: return "Windows 3.1";
    case VER_PLATFORM_WIN32_WINDOWS: return "Windows 9x";
    case VER_PLATFORM_WIN32_NT: return "Windows NT";
    default: return "";
    }
}
