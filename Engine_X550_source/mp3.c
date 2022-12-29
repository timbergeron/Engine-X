/*
Ugh, C++

If anybody thinks I'm gonna write a class here though...

This is a drop-in replacement for the existing CD playing stuff.  It's built on DirectShow using the DirectX
8.1 SDK - I don't know if it'll work on older versions of DirectX, but I assume as most people using this
will be gamers anyway, they'll already have upgraded beyond 8.1 a long time ago.  If you want to recompile
you may need the SDK...

Please don't compile Realm using the Direct X 9 SDK as you'll lose an awful lot of speed if you do (I
gained 50 FPS just by going back to 8.1)

This MP3 interface uses DirectShow for streaming the MP3 from the hard disk.  Better performance may be had
by buffering the entire file in memory and playing from that instead...  I'm kinda new to Direct X in general
so I don't know how to yet :(

I'm not a C++ head by any means, so if anything looks stupid in here, it probably is.

This could probably be very easily modified to enable Quake to stream audio off the web!!!
*/

// Baker code audit 2011-Sep-27:	Static to every variable possible		_
//									Static to every function possible		_	
//									Const to variables when possible		_
//
//									Replace common functions (strcmp, etc.) 
//									with more readable non-performance
//									impairing macros						_
//
//									Replace common functions with slight			// Baker:	Things that don't run every frame or in a huge loop
//									performance hit in non-critical					//			StrlCopy is slightly slower than an unsafe string copy, for instance
//									situations (error messages, loadup)				//			This doesn't matter in most situations but can sure matter
//									such as StringLCopy						_		//			if it is a loop that runs 10000 times during a game frame.
//
//									Limit usage of "return"					_		// Baker:	Early returns in a function are sloppy to the "workflow"; make cleanup harder

#if _MSC_VER > 1200 // MSVC6 does not need
#define POINTER_64 __ptr64 // Vile Baker hack ... explanation ...

/*
Visual Studio C++ Express 2008 includes "Additional Include Paths" at higher priority than the standard include paths.  Some sort
of issue with one of the includes DirectX8 includes causes POINTER_64 to not get defined.  The accepted remedies
all suck.  I'll try to remember to list the link MSDN discussion on this here.

But what I did, was made the "Additional Include Paths" specific to this single file instead of the project on
a whole.
*/
#endif


#include "quakedef.h"
#include "version.h"

extern mp3_com_t MP3Com;

#include <windows.h>
#include <stdio.h>
#include <objbase.h>

#define COBJMACROS
#include <dshow.h>

// Add "strmidds.lib" in without kludging up project settings
#pragma comment (lib,    "dxsdk/sdk8/lib/strmiids.lib")

extern HWND mainwindow;


// this needs to be defined in gl_vidnt.c as well
#define WM_GRAPHNOTIFY  WM_USER + 13

static IGraphBuilder	*pGraph		= NULL;
static IMediaControl	*pControl	= NULL;
static IMediaEventEx	*pEvent		= NULL;
static IBasicAudio		*pAudio		= NULL;
static IMediaSeeking	*pSeek		= NULL;

static BOOL COMSTUFFOK				= FALSE;

static void WaitForFilterState (const OAFilterState DesiredState)
{
	OAFilterState MP3FS;
	HRESULT hr;

	while (1)
	{
		hr = IMediaControl_GetState(pControl, 1, &MP3FS);

		if (FAILED (hr))			continue;
		if (MP3FS == DesiredState)	break;
	}
}

char InitMP3DShow (void)
{
	// COM is beautiful, intuitive and really easy in VB.  This is just clunky and awful.
	HRESULT hr = CoInitialize (NULL);

	if (FAILED (hr))
	{
		Con_Printf ("ERROR - Could not initialize COM library");
		return 0;
	}

	if (FAILED (hr))
	{
		Con_Printf ("ERROR - Could not create the Filter Graph Manager.");
		CoUninitialize ();  // kill off COM
		return 0;
	}
	COMSTUFFOK = TRUE;
	return 1;
}

void KillMP3DShow (void)
{
	if (!COMSTUFFOK) return;

	// stop anything that's playing
	if (MP3Com.playing)
	{
		IMediaEventEx_SetNotifyWindow	(pEvent, (OAHWND) NULL, 0, 0);
		IMediaControl_Stop				(pControl);
		IMediaControl_Release			(pControl);
		IMediaEventEx_Release			(pEvent);
		IBasicAudio_Release				(pAudio);
		IMediaEventEx_Release			(pSeek);
		IGraphBuilder_Release			(pGraph);
	}
	CoUninitialize ();
}

void StopMP3DShow (void)
{
	if (!COMSTUFFOK)		return;	// don't try anything if we couldn't start COM
	if (!MP3Com.playing)	return;	// don;t try to stop if we're not even playing!!!

	// kill it straight away
	IMediaEventEx_SetNotifyWindow		(pEvent, (OAHWND) NULL, 0, 0);
	IMediaControl_Stop					(pControl);
	WaitForFilterState					(State_Stopped);
	IMediaControl_Release				(pControl);
	IMediaEventEx_Release				(pEvent);
	IBasicAudio_Release					(pAudio);
	IMediaSeeking_Release				(pSeek);
	IGraphBuilder_Release				(pGraph);

	// nothing playing now
	MP3Com.playing = false;
}

void PawsMP3DShow (const int Paused)
{
	if (!COMSTUFFOK)		return;	// don't try anything if we couldn't start COM
	if (!MP3Com.playing)	return;	// don;t try to pause if we're not even playing!!!

	if (Paused) // don't wait for the filter states here
		IMediaControl_Run(pControl);
	else
		IMediaControl_Pause(pControl);
}

void VolmMP3DShow (const int Level)
{
	if (!COMSTUFFOK)		return;  // don't try anything if we couldn't start COM
	if (!MP3Com.playing)	return;  // don;t try to change volume if we're not even playing!!!

	// put_Volume uses an exponential decibel-based scale going from -10000 (no sound) to 0 (full volume)
	// each 100 represents 1 db.  i could do the maths, but this is faster and more maintainable.
	switch (Level)
	{  // half snd_volume = -6.02 db  i got these figures from GoldWave 5's volume changer...
	case 0:		IBasicAudio_put_Volume	(pAudio, -10000);	break;
	case 1:		IBasicAudio_put_Volume	(pAudio,  -2000); 	break;
	case 2:		IBasicAudio_put_Volume	(pAudio,  -1400); 	break;
	case 3:		IBasicAudio_put_Volume	(pAudio,  -1040);	break;
	case 4:		IBasicAudio_put_Volume	(pAudio,   -800);	break;
	case 5:		IBasicAudio_put_Volume	(pAudio,   -602); 	break;
	case 6:		IBasicAudio_put_Volume	(pAudio,   -440);   break;
	case 7:		IBasicAudio_put_Volume	(pAudio,   -310); 	break;
	case 8:		IBasicAudio_put_Volume	(pAudio,   -190);	break;
	case 9:		IBasicAudio_put_Volume	(pAudio,    -90);	break;
	case 10:	IBasicAudio_put_Volume	(pAudio,      0);	break;
	}
}

/*
===================
MesgMP3DShow

MP3 Message handler.  The only one we're interested in is a stop message.
Everything else is handled within the engine code.
===================
*/
void MesgMP3DShow (const int Looping)
{
	LONG		evCode;
	LONG_PTR	evParam1, evParam2;
    HRESULT		hr = S_OK;

	if (!COMSTUFFOK)		return;	// don't try anything if we couldn't start COM
	if (!MP3Com.playing)	return;	// don't try anything if we're not even playing!!!

    // Process all queued events
	while (MP3Com.playing && /* <--- Baker: Will fix this? */ SUCCEEDED (IMediaEventEx_GetEvent (pEvent, &evCode, &evParam1, &evParam2, 0)))
    {
        // Free memory associated with callback, since we're not using it
        hr = IMediaEventEx_FreeEventParams(pEvent, evCode, evParam1, evParam2);

        // If this is the end of the clip, reset to beginning
        if (evCode == EC_COMPLETE && Looping)
        {
            LONGLONG pos = 0;

            // Reset to first frame of movie
			hr = IMediaSeeking_SetPositions(pSeek, &pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
        }
		else if (evCode == EC_COMPLETE)
		{
			// have to explicitly stop it when it completes otherwise the interfaces will remain open
			// when the next MP3 is played, and both will play simultaneously...!
			StopMP3DShow ();
		}
    }
    return;
}

/*
==================
TouchMP3

quickly confirm that a file exists without having to route it through labyrinthine COM interfaces
this isn't limited to MP3's only, by the way... specify an extension in your "cd play" or "mp3 play"
command and it'll play the file if you have a codec that works with direct show
==================
*/
static char *TouchMP3 (const char *in_mp3filename)
{
	char	MP3File[MAX_OSPATH] = "";
	char	*absolute_path_of_found_file;

	snprintf (MP3File, sizeof(MP3File), "%s/%s", "music", in_mp3filename);				// Music subfolder
	COMD_DefaultExtension (MP3File, ".mp3");											// slap on a ".mp3" extension if it doesn't have one

	absolute_path_of_found_file = FS_Open_DirTree_GetName (MP3File);					// This alters MP3Name to searchpath where found

	if (absolute_path_of_found_file)
		Con_DevPrintf (DEV_SOUND, "Found: playing %s\n", absolute_path_of_found_file);	// Since found, we can specify the name of the opened file
	else
		Con_DevPrintf (DEV_SOUND, "Not found: couldn't find %s\n", in_mp3filename);		// We use "in" because if not found, we have no MP3File name buffer populated

	return absolute_path_of_found_file;		// This is NULL if FS_OpenDirTree failed
}

/*
==================
StringToLPCWSTR

fucking stupid MS data types
==================
*/
static WCHAR *StringToLPCWSTR (const char *instr)
{
	static WCHAR outstr[1024];
	int	i;

	if (!instr)
		return '\0';

	for (i = 0; ; i++)
	{
		outstr[i] = instr[i];
		if (instr[i] == '\0') break;
	}
	return outstr;
}

/*
=====================
SetupMP3DShow

Initialize COM interfaces and begin playing the MP3
=====================
*/
static void SetupMP3DShow (const WCHAR *MP3File)
{
	if (!MP3File)		return;
	if (!COMSTUFFOK)	return;

	// Create the filter graph manager and query for interfaces.

	CoCreateInstance				(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, &IID_IGraphBuilder, (void **) &pGraph);
	IGraphBuilder_QueryInterface	(pGraph, &IID_IMediaControl, (void **) &pControl);
	IGraphBuilder_QueryInterface	(pGraph, &IID_IMediaEventEx, (void **) &pEvent);
	IGraphBuilder_QueryInterface	(pGraph, &IID_IBasicAudio,   (void **) &pAudio);
	IGraphBuilder_QueryInterface	(pGraph, &IID_IMediaSeeking, (void **) &pSeek);
	IGraphBuilder_RenderFile		(pGraph, MP3File, NULL);

	// send events through the standard window event handler
	IMediaEventEx_SetNotifyWindow(pEvent, (OAHWND) mainwindow, WM_GRAPHNOTIFY, 0);

	// Run the graph.
	IMediaControl_Run(pControl);

	// tell us globally that we can play OK
	MP3Com.playing = true;

	// wait until it reports playing
	WaitForFilterState (State_Running);

	// examples in the SDK will wait for the event to complete here, but this is totally inappropriate
	// for a game engine.
}

void UserMP3DShow (const char *mp3name)
{
	MP3Com.playing = false;
	{
		WCHAR *MP3File = StringToLPCWSTR (TouchMP3 (mp3name));
		SetupMP3DShow (MP3File);
	}

	// Baker: Need to assign volume upon starting, otherwise it goes full blast
	VolmMP3DShow ((int) (CLAMP (0, mp3_volume.floater, 1) * 10));
}

void PlayMP3DShow (const int mp3num)
{
	UserMP3DShow (va ("track%02i", mp3num)); // Baker: using -1 to denote we SAY what we are playing, but say nothing if no mp3 track is found
}

#pragma message ("Quality assurance: Does a changelevel with cdtrack, does our music get stopped?  It needs to!")
#pragma message ("Quality assurance: Let alone, we might need some restore QC function that when you load a save game sets to CD track and any other touchups like fog and so forth")