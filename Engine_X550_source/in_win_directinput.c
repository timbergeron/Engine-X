/*
Copyright (C) 1996-1997 Id Software, Inc.

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
// in_win.c -- windows 95 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.

#include "quakedef.h"
#if SUPPORTS_DIRECTINPUT
#include "winquake.h"




#define DIRECTINPUT_VERSION	0x0700

#ifdef __GNUC__
#include "dxsdk\sdk\inc\dinput.h"
#pragma comment (lib, "dxsdk/sdk/lib/dxguid.lib")
#else
#include "dxsdk\sdk8\include\dinput.h"
#pragma comment (lib, "dxsdk/sdk8/lib/dxguid.lib")
#endif // End GNUC

qbool in_dinput_acquired;	// Not static so menu can know state

static LPDIRECTINPUT		g_pdi;
static LPDIRECTINPUTDEVICE	g_pMouse;

static unsigned int	mstate_di;



void IN_Mouse_DInput_Acquire (void)
{
	if (!g_pMouse) return;		// No mouse process
	if (in_dinput_acquired) return;	// DirectInput already acquired

	IDirectInputDevice_Acquire (g_pMouse);
	in_dinput_acquired = true;

	Con_DevPrintf (DEV_MOUSE, "DirectInput acquired.\n");
}

void IN_Mouse_DInput_Unacquire (void)
{
	if (!g_pMouse) return;	// No mouse process
	if (!in_dinput_acquired) return;	// Already unacquired

	IDirectInputDevice_Unacquire (g_pMouse);
	in_dinput_acquired = false;

	Con_DevPrintf (DEV_MOUSE, "DirectInput unacquired.\n");
}

#define INPUT_CASE_DIMOFS_BUTTON(NUM)			\
	case (DIMOFS_BUTTON0 + NUM):			\
		if (od.dwData &	0x80)			\
			mstate_di |= (1	<< NUM);	\
		else					\
			mstate_di &= ~(1 << NUM);	\
		break;

#define INPUT_CASE_DINPUT_MOUSE_BUTTONS			\
		INPUT_CASE_DIMOFS_BUTTON(0);		\
		INPUT_CASE_DIMOFS_BUTTON(1);		\
		INPUT_CASE_DIMOFS_BUTTON(2);		\
		INPUT_CASE_DIMOFS_BUTTON(3);	\
		INPUT_CASE_DIMOFS_BUTTON(4);	\

extern const int		mouse_buttons;
extern int				mouse_oldbuttonstate;

void IN_Mouse_DInput_Move (int *accumx, int *accumy)
{
	DIDEVICEOBJECTDATA	od;
	DWORD			dwElements;
	HRESULT			hr;
	int 			i, mx = 0, my = 0;

	while (1)
	{
		dwElements = 1;

		hr = IDirectInputDevice_GetDeviceData (g_pMouse, sizeof(DIDEVICEOBJECTDATA), &od, &dwElements, 0);

		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED)) { in_dinput_acquired = false; IN_Mouse_DInput_Acquire (); break; }
		if (FAILED(hr) || dwElements == 0) 						  { break; }  // Unable to read data or no data available

		// Look at the element to see what happened

		switch (od.dwOfs)
		{
			case DIMOFS_X:
				mx += od.dwData;
				break;

			case DIMOFS_Y:
				my += od.dwData;
				break;

			case DIMOFS_Z:		// Mousewheel


				if (od.dwData & 0x80)
				{
					Key_Event (K_MWHEELDOWN, 0, true);
					Key_Event (K_MWHEELDOWN, 0, false);

				}
				else
				{
					Key_Event (K_MWHEELUP, 0, true);
					Key_Event (K_MWHEELUP, 0, false);
				}
				break;

			INPUT_CASE_DINPUT_MOUSE_BUTTONS

		}
	}

// perform button actions

	for (i=0 ; i<mouse_buttons ; i++)
	{
		if ( (mstate_di & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)) ) Key_Event (K_MOUSE1 + i, 0, true);
		if (!(mstate_di & (1<<i)) && (mouse_oldbuttonstate & (1<<i)) ) Key_Event (K_MOUSE1 + i, 0, false);
	}

	mouse_oldbuttonstate = mstate_di;

	*accumx = mx;
	*accumy = my;

}






static	HINSTANCE	hInstDI;

qbool IN_Mouse_DInput_Shutdown (void)
{
	if (g_pMouse) 	{ IDirectInputDevice_Release (g_pMouse); g_pMouse = NULL; }
	if (g_pdi) 		{ IDirectInput_Release (g_pdi); g_pdi = NULL; }

//	if (hInstDI)	{ FreeLibrary(hInstDI); hInstDI = NULL; }
// Baker: In theory this is a good idea.  In practice, let us re-use it.

	return false;
}

#define iDirectInputCreate(a,b,c,d) pDirectInputCreate(a,b,c,d)
HRESULT (WINAPI *pDirectInputCreate)(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUT * lplpDirectInput, LPUNKNOWN punkOuter);

typedef struct MYDATA
{
	LONG  lX;                   // X axis goes here
	LONG  lY;                   // Y axis goes here
	LONG  lZ;                   // Z axis goes here
	BYTE  bButtonA;             // One button goes here
	BYTE  bButtonB;             // Another button goes here
	BYTE  bButtonC;             // Another button goes here
	BYTE  bButtonD;             // Another button goes here
	BYTE  bButtonE;             // Baker: DINPUT fix for 8 buttons?
	BYTE  bButtonF;             // Baker: DINPUT fix for 8 buttons?
	BYTE  bButtonG;             // Baker: DINPUT fix for 8 buttons?
	BYTE  bButtonH;             // Baker: DINPUT fix for 8 buttons?
} MYDATA;

static DIOBJECTDATAFORMAT rgodf[] = {
  { &GUID_XAxis,    FIELD_OFFSET(MYDATA, lX),       DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { &GUID_YAxis,    FIELD_OFFSET(MYDATA, lY),       DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { &GUID_ZAxis,    FIELD_OFFSET(MYDATA, lZ),       0x80000000 | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonA), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonB), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonC), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonD), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
	{0, FIELD_OFFSET(MYDATA, bButtonE), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
	{0, FIELD_OFFSET(MYDATA, bButtonF), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
	{0, FIELD_OFFSET(MYDATA, bButtonG), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
	{0, FIELD_OFFSET(MYDATA, bButtonH), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
};

#define NUM_OBJECTS (sizeof(rgodf) / sizeof(rgodf[0]))

static DIDATAFORMAT	df = {
	sizeof(DIDATAFORMAT),       // this structure
	sizeof(DIOBJECTDATAFORMAT), // size of object data format
	DIDF_RELAXIS,               // absolute axis coordinates
	sizeof(MYDATA),             // device data size
	NUM_OBJECTS,                // number of objects
	rgodf,                      // and here they are
};

qbool IN_Mouse_DInput_Startup (void)
{
	#define DINPUT_BUFFERSIZE	16
	HRESULT		hr;
	DIPROPDWORD	dipdw = {
		{
			sizeof(DIPROPDWORD),        // diph.dwSize
			sizeof(DIPROPHEADER),       // diph.dwHeaderSize
			0,                          // diph.dwObj
			DIPH_DEVICE,                // diph.dwHow
		},
		DINPUT_BUFFERSIZE,              // dwData
	};

	if (!hInstDI)
	{
		hInstDI = LoadLibrary ("dinput.dll");
		if (hInstDI == NULL)
		{
			Con_Warning ("Couldn't load dinput.dll\n");
			return false;
		}
	}

	if (!pDirectInputCreate)
	{
		pDirectInputCreate = (void *)GetProcAddress(hInstDI,"DirectInputCreateA");

		if (!pDirectInputCreate)
		{
			Con_Warning ("Couldn't get DI proc addr\n");
			return false;
		}
	}

// register with DirectInput and get an IDirectInput to play with.
	hr = iDirectInputCreate(global_hInstance, DIRECTINPUT_VERSION, &g_pdi, NULL);

	if (FAILED(hr))
	{
		Con_Warning ("iDirectInputCreate failed\n");
		return false;
	}

// obtain an interface to the system mouse device.
	hr = IDirectInput_CreateDevice(g_pdi, &GUID_SysMouse, &g_pMouse, NULL);

	if (FAILED(hr))
	{
		Con_Warning ("Couldn't open DI mouse device\n");
		return false;
	}

// set the data format to "mouse format".
	hr = IDirectInputDevice_SetDataFormat(g_pMouse, &df);

	if (FAILED(hr))
	{
		Con_Warning ("Couldn't set DI mouse format\n");
		return false;
	}

// set the cooperativity level.
	hr = IDirectInputDevice_SetCooperativeLevel(g_pMouse, mainwindow, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

	if (FAILED(hr))
	{
		Con_Warning ("Couldn't set DI coop level\n");
		return false;
	}

// set the buffer size to DINPUT_BUFFERSIZE elements.
// the buffer size is a DWORD property associated with the device
	hr = IDirectInputDevice_SetProperty(g_pMouse, DIPROP_BUFFERSIZE, &dipdw.diph);

	if (FAILED(hr))
	{
		Con_Warning ("Couldn't set DI buffersize\n");
		return false;
	}

	return true;
}


#if WINDOWS_SCROLLWHEEL_PEEK
// Baker: this is ONLY used to capture mouse input for console scrolling
//        and ONLY if directinput is enabled
void IN_Mouse_DInput_MouseWheel (void)
{
	DIDEVICEOBJECTDATA	od;
	DWORD				dwElements;
	HRESULT				hr;

	if (!in_dinput_acquired)	return; // No mouse ... pointless

	while (1)
	{
		dwElements = 1;
		hr = IDirectInputDevice_GetDeviceData (g_pMouse, sizeof(DIDEVICEOBJECTDATA), &od, &dwElements, 0);

		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			in_dinput_acquired = false;
			IN_Mouse_DInput_Acquire ();
			break;
		}

		/* Unable to read data or no data available */
		if (FAILED(hr) || !dwElements)
			break;

		if (od.dwOfs == DIMOFS_Z)
		{
			if (od.dwData & 0x80)
			{
				Key_Event(K_MWHEELDOWN, 0, true);
				Key_Event(K_MWHEELDOWN, 0, false);
			}
			else
			{
				Key_Event(K_MWHEELUP, 0, true);
				Key_Event(K_MWHEELUP, 0, false);
			}
			break;
		}
	}
}
#endif // SCROLLWHEEL PEEK



#endif // DIRECTINPUT

