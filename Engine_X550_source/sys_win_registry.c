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
// sys_win_registry.c -- Win32 system registry stuffs

#include "quakedef.h"
#include <windows.h>


qbool CreateSetKeyExtension(const char *MyApplicationName, const char *DotMyFileExtension) // Like .dem
{

	long IsNew = 0;
	DWORD Reserved = 0;
	DWORD dwOptions = 0;
	HKEY hregkey;

	long res = RegCreateKeyEx(HKEY_CLASSES_ROOT, DotMyFileExtension, Reserved, NULL, dwOptions, KEY_WRITE, NULL, &hregkey, &IsNew);

	if (res == ERROR_SUCCESS) // If we created it successfully
	{
		// Storing the string
		char str[256];
		StringLCopy (str, MyApplicationName);
		res = RegSetValueEx(hregkey, "", 0, REG_SZ, (const unsigned char *)str, strlen(str));
		RegCloseKey(hregkey);

		if (res == ERROR_ACCESS_DENIED)
		{
			// Baker: To do this on Vista and later, probably need digital signature or administration elevation
			Con_DevPrintf (DEV_SYSTEM, "CreateSetKeyExtension: Registry access failed. ERROR_ACCESS_DENIED.\n");
			return false;
		}

		Con_DevPrintf (DEV_SYSTEM, "CreateSetKeyExtension: Registry access succeeded\n");
		return true;
	}

	return false;	// Failed
}

qbool CreateSetKeyDescription(const char *MyApplicationName, const char *MyFileTypeDecription)
{
	long IsNew = 0;
	HKEY hregkey;
	DWORD Reserved = 0;
	DWORD dwOptions = 0;

	long res = RegCreateKeyEx(HKEY_CLASSES_ROOT, MyApplicationName, Reserved, NULL, dwOptions, KEY_WRITE, NULL, &hregkey, &IsNew);

	if (res == ERROR_SUCCESS) // If we created it successfully
	{
		// Storing the string
		char	str[256];

		StringLCopy (str, MyFileTypeDecription);
		//RegSetValueEx(hregkey, "String", 0, REG_SZ, (const unsigned char *)str, strlen(str));
		res = RegSetValueEx(hregkey, "", 0, REG_SZ, (const unsigned char *)str, strlen(str));
		RegCloseKey(hregkey);

		if (res == ERROR_ACCESS_DENIED)
		{
			// Baker: To do this on Vista and later, probably need digital signature or administration elevation
			Con_DevPrintf (DEV_SYSTEM, "CreateSetKeyDescription: Registry access failed. ERROR_ACCESS_DENIED.\n");
			return false;
		}

		Con_DevPrintf (DEV_SYSTEM, "CreateSetKeyDescription: Registry access succeeded\n");
		return true;
	}

	return false;
}

qbool CreateSetKeyCommandLine(const char *MyApplicationName, const char *reg_cmdline_string)
{
	// Must send something like c:\quake\quake.exe %1
	long	IsNew = 0, res = 0;
	HKEY	hregkey;
	DWORD	Reserved = 0;
	DWORD	dwOptions = 0;
	char	str[256];

	snprintf (str, sizeof(str), "%s\\shell\\open\\command", MyApplicationName);

	res = RegCreateKeyEx(HKEY_CLASSES_ROOT, (const unsigned char *)str, Reserved, NULL, dwOptions, KEY_WRITE, NULL, &hregkey, &IsNew);

	if (res == ERROR_SUCCESS) // If we created it successfully
	{
		// Format the string
		// Baker: We should detect failure here ideally.
//		char registry_cmdline[256];
//		snprintf (registry_cmdline,  sizeof(registry_cmdline), "%s \"%%1\"", executable_absolutepath);

		res = RegSetValueEx(hregkey, "", 0, REG_SZ, (const unsigned char *)reg_cmdline_string, strlen(reg_cmdline_string));
		RegCloseKey(hregkey);

		if (res == ERROR_ACCESS_DENIED)
		{
			// Baker: To do this on Vista and later, probably need digital signature or administration elevation
			Con_DevPrintf (DEV_SYSTEM, "CreateSetKeyCommandLine: Registry access failed. ERROR_ACCESS_DENIED.\n");
			return false;
		}

		Con_DevPrintf (DEV_SYSTEM, "CreateSetKeyCommandLine: Registry access succeeded\n");
		return true;
	}

	return false; // Failed
}

void BuildRegistryEntries (const char *media_description, const char *media_file_extension, const char *application_description, const char *executable_fullpath)
{
	char	registry_cmdline[MAX_OSPATH];
	qbool 	success = false;

	snprintf(registry_cmdline,  sizeof(registry_cmdline), "%s \"%%1\"", executable_fullpath);

	// Generate registry key and build registry entries
	if ((success = CreateSetKeyExtension				(application_description, media_file_extension)))
		if ((success = CreateSetKeyDescription			(application_description, media_description)))
			if ((success = CreateSetKeyCommandLine		(application_description, registry_cmdline)));

	if (!success)
	{
		Con_Warning ("File association failed for '%s'.\nYou may need to run this engine with administrator priviledges.\n", media_file_extension);
		return;
	}

	Con_Printf ("%s successfully associated with %s (%s) files\n", application_description, media_description, media_file_extension);
}