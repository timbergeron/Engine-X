/*

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// sys_win_version.c -- Win32 system versioning code

#include "quakedef.h"
#include <windows.h>

static int	System_Memory_MB = 0;
static int  System_Processor_Speed_MHz = 0;
static char System_Processor_Description[40];

static char		*WindowsVersion_Description[7] =
{
	"Windows 95/98/ME",
	"Windows 2000",
	"Windows XP",
	"Windows 2003",
	"Windows Vista",
	"Windows 7",
	"Windows 7 or later"
};

int				WindowsVersion=0;

void Sys_InfoPrint_f(void)
{
	Con_Printf ("Operating System: %s\n"
				"Memory          : %i MB\n"
		        "Processor Speed : %i Mhz\n"
				"Processor:      : %s\n", WindowsVersion_Description[WindowsVersion], System_Memory_MB, System_Processor_Speed_MHz, System_Processor_Description);
}


qbool Sys_GetWindowsVersion(void)
{
	MEMORYSTATUS    memstat;
	LONG            ret;
	HKEY            hKey;
	OSVERSIONINFO	vinfo;
	qbool		WinNT;

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);


	if (!GetVersionEx(&vinfo))
		Sys_Error ("Couldn't get OS info");

	if (vinfo.dwMajorVersion < 4 || vinfo.dwPlatformId == VER_PLATFORM_WIN32s)
		return false;	// Version cannot run Quake

	WinNT = (vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT) ? true : false;

	// Get The Version

	if (WinNT && vinfo.dwMajorVersion == 5 && vinfo.dwMinorVersion == 0)
		WindowsVersion = 1; // "Windows 2000"
	else if (WinNT && vinfo.dwMajorVersion == 5 && vinfo.dwMinorVersion == 1)
		WindowsVersion = 2; // "Windows XP"
	else if (WinNT && vinfo.dwMajorVersion == 5 && vinfo.dwMinorVersion == 2)
		WindowsVersion = 3; // "Windows 2003"
	else if (WinNT && vinfo.dwMajorVersion == 6 && vinfo.dwMinorVersion == 0)
		WindowsVersion = 4; // "Windows Vista"
	else if (WinNT && vinfo.dwMajorVersion == 6 && vinfo.dwMinorVersion == 1)
		WindowsVersion = 5; // "Windows 7"
	else if (WinNT && vinfo.dwMajorVersion >= 6 && vinfo.dwMinorVersion > 1)
		WindowsVersion = 6; // "Windows 7 or later"
	else
		WindowsVersion = 0; // "Windows 95/98/ME"


	// Print the version to the console
	Con_Printf("Operating System: %s\n", WindowsVersion_Description[WindowsVersion]);

	GlobalMemoryStatus(&memstat);
	System_Memory_MB = (memstat.dwTotalPhys / (1024 * 1024));


	// Get Processor description and speed
	if ((ret = RegOpenKey(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey)) == ERROR_SUCCESS)
	{
		byte  data[1024];
		DWORD datasize=1024, type;

		if ((ret = RegQueryValueEx(hKey, "~MHz", NULL, &type, data, &datasize))== ERROR_SUCCESS  &&  datasize > 0  &&  type == REG_DWORD)
			System_Processor_Speed_MHz = *((DWORD *)data);

		datasize = 1024;
		if ((ret = RegQueryValueEx(hKey, "ProcessorNameString", NULL, &type, data, &datasize)) == ERROR_SUCCESS  &&  datasize > 0  && type == REG_SZ)
			snprintf (System_Processor_Description, sizeof(System_Processor_Description), (char *) data);

		RegCloseKey(hKey);
	}

	return true;	// Windows version capable of running Quake
	}


