//_________________________________________________________________________________________________________________________nFO
// "cd_osx.m" - MacOS X audio CD driver.
//
// Written by:	Axel 'awe' Wefers			[mailto:awe@fruitz-of-dojo.de].
//				�2001-2006 Fruitz Of Dojo 	[http://www.fruitz-of-dojo.de].
//
// Quake� is copyrighted by id software		[http://www.idsoftware.com].
//
// Version History:
// v1.0.9: Rewritten. Uses now QuickTime for playback. Added support for MP3 and MP4 [AAC] playback.
// v1.0.3: Fixed an issue with requesting a track number greater than the max number.
// v1.0.1: Added "cdda" as extension for detection of audio-tracks [required by MacOS X v10.1 or later]
// v1.0.0: Initial release.
//____________________________________________________________________________________________________________________iNCLUDES

#pragma mark =Includes=

#import <Cocoa/Cocoa.h>
#import <CoreAudio/AudioHardware.h>
#import <QuickTime/QuickTime.h>
#import <sys/mount.h>
#import <pthread.h>

#import "quakedef.h"
#import "Quake.h"
#import "cd_osx.h"
#import "sys_osx.h"

#pragma mark -

//___________________________________________________________________________________________________________________vARIABLES

#pragma mark =Variables=

extern cvar_t				mp3_volume;

static UInt16				gCDTrackCount;
static UInt16				gCDCurTrack;
static NSMutableArray *		gCDTrackList;
static char					gCDDevice[MAX_OSPATH];
static BOOL					gCDLoop;
static BOOL					gCDNextTrack;
static Movie				gCDController = NULL;

#pragma mark -

//_________________________________________________________________________________________________________fUNCTION_pROTOTYPES

#pragma mark =Function Prototypes=

static	void		MP3Audio_Error (cderror_t theErrorNumber);
static	SInt32		MP3Audio_StripVideoTracks (Movie theMovie);
static	void		MP3Audio_SafePath (const char *thePath);
static	void		MP3Audio_AddTracks2List (NSString *theMountPath, NSArray *theExtensions);
static 	void 		CD_f (void);

#pragma mark -

//_____________________________________________________________________________________________________________MP3Audio_Error()

void	MP3Audio_Error (cderror_t theErrorNumber)
{
    if ([[NSApp delegate] mediaFolder] == NULL)
    {
        Con_Print ("Audio-CD driver: ");
    }
    else
    {
        Con_Print ("MP3/MP4 driver: ");
    }
    
    switch (theErrorNumber)
    {
        case CDERR_ALLOC_TRACK:
            Con_Print ("Failed to allocate track!\n");
            break;
        case CDERR_MOVIE_DATA:
            Con_Print ("Failed to retrieve track data!\n");
            break;
        case CDERR_AUDIO_DATA:
            Con_Print ("File without audio track!\n");
            break;
        case CDERR_QUICKTIME_ERROR:
            Con_Print ("QuickTime error!\n");
            break;
        case CDERR_THREAD_ERROR:
            Con_Print ("Failed to initialize thread!\n");
            break;
        case CDERR_NO_MEDIA_FOUND:
            Con_Print ("No Audio-CD found.\n");
            break;
        case CDERR_MEDIA_TRACK:
            Con_Print ("Failed to retrieve media track!\n");
            break;
        case CDERR_MEDIA_TRACK_CONTROLLER:
            Con_Print ("Failed to retrieve track controller!\n");
            break;
        case CDERR_EJECT:
            Con_Print ("Can\'t eject Audio-CD!\n");
            break;
        case CDERR_NO_FILES_FOUND:
            if ([[NSApp delegate] mediaFolder] == NULL)
            {
                Con_Print ("No audio tracks found.\n");
            }
            else
            {
                Con_Print ("No files found with the extension \'.mp3\', \'.mp4\' or \'.m4a\'!\n");
            }
            break;
    }
}

//__________________________________________________________________________________________________MP3Audio_StripVideoTracks()

SInt32	MP3Audio_StripVideoTracks (Movie theMovie)
{
    SInt64 	myTrackCount, i;
    Track 	myCurTrack;
    OSType 	myMediaType;
	
    myTrackCount = GetMovieTrackCount (theMovie);

    for (i = myTrackCount; i >= 1; i--)
    {
        myCurTrack = GetMovieIndTrack (theMovie, i);
        GetMediaHandlerDescription (GetTrackMedia (myCurTrack), &myMediaType, NULL, NULL);
        if (myMediaType != SoundMediaType && myMediaType != MusicMediaType)
        {
            DisposeMovieTrack (myCurTrack);
        }
    }

    return (GetMovieTrackCount (theMovie));
}

//____________________________________________________________________________________________________MP3Audio_AddTracks2List()

void	MP3Audio_AddTracks2List (NSString *theMountPath, NSArray *theExtensions)
{
    NSFileManager		*myFileManager = [NSFileManager defaultManager];
    
    if (myFileManager != NULL)
    {
        NSDirectoryEnumerator	*myDirEnum = [myFileManager enumeratorAtPath: theMountPath];

        if (myDirEnum != NULL)
        {
            NSString	*myFilePath;
            SInt32	myIndex, myExtensionCount;
            
            myExtensionCount = [theExtensions count];
            
            // get all audio tracks:
            while ((myFilePath = [myDirEnum nextObject]))
            {
                if ([[NSApp delegate] abortMediaScan] == YES)
                {
                    break;
                }

                for (myIndex = 0; myIndex < myExtensionCount; myIndex++)
                {
                    if ([[myFilePath pathExtension] isEqualToString: [theExtensions objectAtIndex: myIndex]])
                    {
                        NSString	*myFullPath = [theMountPath stringByAppendingPathComponent: myFilePath];
                        NSURL		*myMoviePath = [NSURL fileURLWithPath: myFullPath];
                        NSMovie		*myMovie = NULL;
                        
                        myMovie = [[NSMovie alloc] initWithURL: myMoviePath byReference: YES];
                        if (myMovie != NULL)
                        {
                            Movie	myQTMovie = [myMovie QTMovie];
                            
                            if (myQTMovie != NULL)
                            {
                                // add only movies with audiotacks and use only the audio track:
                                if (MP3Audio_StripVideoTracks (myQTMovie) > 0)
                                {
                                    [gCDTrackList addObject: myMovie];
                                }
                                else
                                {
                                    MP3Audio_Error (CDERR_AUDIO_DATA);
                                }
                            }
                            else
                            {
                                MP3Audio_Error (CDERR_MOVIE_DATA);
                            }
                        }
                        else
                        {
                            MP3Audio_Error (CDERR_ALLOC_TRACK);
                        }
                    }
                }
            }
        }
    }
    gCDTrackCount = [gCDTrackList count];
}

//__________________________________________________________________________________________________________MP3Audio_SafePath()

void	MP3Audio_SafePath (const char *thePath)
{
    SInt32	myStrLength = 0;

    if (thePath != NULL)
    {
        SInt32		i;
        
        myStrLength = strlen (thePath);
        if (myStrLength > MAX_OSPATH - 1)
        {
            myStrLength = MAX_OSPATH - 1;
        }
        for (i = 0; i < myStrLength; i++)
        {
            gCDDevice[i] = thePath[i];
        }
    }
    gCDDevice[myStrLength] = 0x00;
}

//______________________________________________________________________________________________________MP3Audio_GetTrackList()

BOOL	MP3Audio_GetTrackList (void)
{
    NSAutoreleasePool 		*myPool;
    
    // release previously allocated memory:
    MP3Audio_Shutdown ();
    
    // get memory for the new tracklisting:
    gCDTrackList = [[NSMutableArray alloc] init];
    myPool = [[NSAutoreleasePool alloc] init];
    gCDTrackCount = 0;
    
    // Get the current MP3 listing or retrieve the TOC of the AudioCD:
    if ([[NSApp delegate] mediaFolder] != NULL)
    {
        NSString	*myMediaFolder = [[NSApp delegate] mediaFolder];

        MP3Audio_SafePath ([myMediaFolder fileSystemRepresentation]);
        Con_Printf ("Scanning for audio tracks. Be patient!\n");
        MP3Audio_AddTracks2List (myMediaFolder, [NSArray arrayWithObjects: @"mp3", @"mp4", @"m4a", NULL]);
    }
    else
    {
        NSString		*myMountPath;
        struct statfs  		*myMountList;
        UInt32			myMountCount;

        // get number of mounted devices:
        myMountCount = getmntinfo (&myMountList, MNT_NOWAIT);
        
        // zero devices? return.
        if (myMountCount <= 0)
        {
            [gCDTrackList release];
            gCDTrackList = NULL;
            gCDTrackCount = 0;
            MP3Audio_Error (CDERR_NO_MEDIA_FOUND);
            return (0);
        }
        
        while (myMountCount--)
        {
            // is the device read only?
            if ((myMountList[myMountCount].f_flags & MNT_RDONLY) != MNT_RDONLY) continue;
            
            // is the device local?
            if ((myMountList[myMountCount].f_flags & MNT_LOCAL) != MNT_LOCAL) continue;
            
            // is the device "cdda"?
            if (strcmp (myMountList[myMountCount].f_fstypename, "cddafs")) continue;
            
            // is the device a directory?
            if (strrchr (myMountList[myMountCount].f_mntonname, '/') == NULL) continue;
            
            // we have found a Audio-CD!
            Con_Printf ("Found Audio-CD at mount entry: \"%s\".\n", myMountList[myMountCount].f_mntonname);
            
            // preserve the device name:
            MP3Audio_SafePath (myMountList[myMountCount].f_mntonname);
            myMountPath = [NSString stringWithCString: myMountList[myMountCount].f_mntonname];
    
            Con_Print ("Scanning for audio tracks. Be patient!\n");
            MP3Audio_AddTracks2List (myMountPath, [NSArray arrayWithObjects: @"aiff", @"cdda", NULL]);
            
            break;
        }
    }
    
    // release the pool:
    [myPool release];
    
    // just security:
    if (![gCDTrackList count])
    {
        [gCDTrackList release];
        gCDTrackList = NULL;
        gCDTrackCount = 0;
        MP3Audio_Error (CDERR_NO_FILES_FOUND);
        return (0);
    }
    
    return (1);
}

//______________________________________________________________________________________________________________MP3Audio_Play()

void	MP3Audio_Play (byte theTrack, qbool theLoop)
{
    gCDNextTrack = NO;
    
    if (gCDTrackList != NULL && gCDTrackCount != 0)
    {
        NSMovie	*	myMovie;
        
        // check for mismatching CD track number:
        if (theTrack > gCDTrackCount || theTrack <= 0)
        {
            theTrack = 1;
        }
        gCDCurTrack = 0;
        
        if (gCDController != NULL && IsMovieDone (gCDController) == NO)
        {
            StopMovie(gCDController);
            gCDController = NULL;
        }
        
        myMovie = [gCDTrackList objectAtIndex: theTrack - 1];
        
        if (myMovie != NULL)
        {
            gCDController = [myMovie QTMovie];
            
            if (gCDController != NULL)
            {
                gCDCurTrack	= theTrack;
                gCDLoop		= theLoop;
				
                GoToBeginningOfMovie (gCDController);
                SetMovieActive (gCDController, YES);
                StartMovie (gCDController);
				
				if (GetMoviesError () != noErr)
				{
                    MP3Audio_Error (CDERR_QUICKTIME_ERROR);
				}
            }
            else
            {
                MP3Audio_Error (CDERR_MEDIA_TRACK);
            }
        }
        else
        {
            MP3Audio_Error (CDERR_MEDIA_TRACK_CONTROLLER);
        }
    }
}

//______________________________________________________________________________________________________________MP3Audio_Stop()

void	MP3Audio_Stop (void)
{
    // just stop the audio IO:
    if (gCDController != NULL && IsMovieDone (gCDController) == NO)
    {
        StopMovie (gCDController);
        GoToBeginningOfMovie (gCDController);
        SetMovieActive (gCDController, NO);
    }
}

//_____________________________________________________________________________________________________________MP3Audio_Pause()

void	MP3Audio_Pause (void)
{
    if (gCDController != NULL && GetMovieActive (gCDController) == YES && IsMovieDone (gCDController) == NO)
    {
        StopMovie (gCDController);
        SetMovieActive (gCDController, NO);
    }
}

//____________________________________________________________________________________________________________MP3Audio_Resume()

void	MP3Audio_Resume (void)
{
    if (gCDController != NULL && GetMovieActive (gCDController) == NO && IsMovieDone (gCDController) == NO)
    {
        SetMovieActive (gCDController, YES);
        StartMovie (gCDController);
    }
}

//____________________________________________________________________________________________________________MP3Audio_Update()

void	MP3Audio_Update (void)
{
    // update volume settings:
    if (gCDController != NULL)
    {
        SetMovieVolume (gCDController, kFullVolume * mp3_volume.floater);

        if (GetMovieActive (gCDController) == YES)
        {
            if (IsMovieDone (gCDController) == NO)
            {
                MoviesTask (gCDController, 0);
            }
            else
            {
                if (gCDLoop == YES)
                {
                    GoToBeginningOfMovie (gCDController);
                    StartMovie (gCDController);
                }
                else
                {
                    gCDCurTrack++;
                    MP3Audio_Play (gCDCurTrack, NO);
                }
            }
        }
    }
}

//____________________________________________________________________________________________________________MP3Audio_Enable()


void	MP3Audio_Enable (BOOL theState)
{
    static BOOL	myCDIsEnabled = YES;
    
    if (myCDIsEnabled != theState)
    {
        static BOOL	myCDWasPlaying = NO;
        
        if (theState == NO)
        {
            if (gCDController != NULL && GetMovieActive (gCDController) == YES && IsMovieDone (gCDController) == NO)
            {
                MP3Audio_Pause ();
                myCDWasPlaying = YES;
            }
            else
            {
                myCDWasPlaying = NO;
            }
        }
        else
        {
            if (myCDWasPlaying == YES)
            {
                MP3Audio_Resume ();
            }
        }
		
        myCDIsEnabled = theState;
    }
}
//______________________________________________________________________________________________________________MP3Audio_Init()

int	MP3Audio_Init (void)
{
    // add "cd" and "mp3" console command:
    if ([[NSApp delegate] mediaFolder] != NULL)
    {
        Cmd_AddCommand ("mp3", CD_f);
        Cmd_AddCommand ("mp4", CD_f);
    }
    Cmd_AddCommand ("cd", CD_f);
    
    gCDCurTrack = 0;
    
    if (gCDTrackList != NULL)
    {
        if ([[NSApp delegate] mediaFolder] == NULL)
        {
            Con_Print ("QuickTime CD driver initialized...\n");
        }
        else
        {
            Con_Print ("QuickTime MP3/MP4 driver initialized...\n");
        }

        return (1);
    }
    
    // failure. return 0.
    if ([[NSApp delegate] mediaFolder] == NULL)
    {
        Con_Print ("QuickTime CD driver failed.\n");
    }
    else
    {
        Con_Print ("QuickTime MP3/MP4 driver failed.\n");
    }
    
    return (0);
}

//__________________________________________________________________________________________________________MP3Audio_Shutdown()

void	MP3Audio_Shutdown (void)
{
    // shutdown the audio IO:
    MP3Audio_Stop ();

    gCDController	= NULL;
    gCDDevice[0]	= 0x00;    
    gCDCurTrack		= 0;

    if (gCDTrackList != NULL)
    {
       while ([gCDTrackList count])
        {
            NSMovie 	*myMovie = [gCDTrackList objectAtIndex: 0];
            
            [gCDTrackList removeObjectAtIndex: 0];
            [myMovie release];
        }
        [gCDTrackList release];
        gCDTrackList = NULL;
        gCDTrackCount = 0;
    }
}

//______________________________________________________________________________________________________________________CD_f()

void	CD_f (void)
{
    char	*myCommandOption;

    // this command requires options!
    if (Cmd_Argc () < 2)
    {
        return;
    }

    // get the option:
    myCommandOption = Cmd_Argv (1);
    
    // turn CD playback on:
    if (strcasecmp (myCommandOption, "on") == 0)
    {
        if (gCDTrackList == NULL)
        {
            MP3Audio_GetTrackList();
        }
        MP3Audio_Play(1, 0);
        
		return;
    }
    
    // turn CD playback off:
    if (strcasecmp (myCommandOption, "off") == 0)
    {
        MP3Audio_Shutdown ();
        
		return;
    }

    // just for compatibility:
    if (strcasecmp (myCommandOption, "remap") == 0)
    {
        return;
    }

    // reset the current CD:
    if (strcasecmp (myCommandOption, "reset") == 0)
    {
        MP3Audio_Stop ();
        if (MP3Audio_GetTrackList ())
        {
            if ([[NSApp delegate] mediaFolder] == NULL)
            {
                Con_Print ("CD");
            }
            else
            {
                Con_Print ("MP3/MP4 files");
            }
            Con_Printf (" found. %d tracks (\"%s\").\n", gCDTrackCount, gCDDevice);
	}
        else
        {
            MP3Audio_Error (CDERR_NO_FILES_FOUND);
        }
        
		return;
    }
    
    // the following commands require a valid track array, so build it, if not present:
    if (gCDTrackCount == 0)
    {
        MP3Audio_GetTrackList ();
        if (gCDTrackCount == 0)
        {
            MP3Audio_Error (CDERR_NO_FILES_FOUND);
            return;
        }
    }
    
    // play the selected track:
    if (strcasecmp (myCommandOption, "play") == 0)
    {
        MP3Audio_Play (atoi (Cmd_Argv (2)), 0);
        
		return;
    }
    
    // loop the selected track:
    if (strcasecmp (myCommandOption, "loop") == 0)
    {
        MP3Audio_Play (atoi (Cmd_Argv (2)), 1);
        
		return;
    }
    
    // stop the current track:
    if (strcasecmp (myCommandOption, "stop") == 0)
    {
        MP3Audio_Stop ();
        
		return;
    }
    
    // pause the current track:
    if (strcasecmp (myCommandOption, "pause") == 0)
    {
        MP3Audio_Pause ();
        
		return;
    }
    
    // resume the current track:
    if (strcasecmp (myCommandOption, "resume") == 0)
    {
        MP3Audio_Resume ();
        
		return;
    }
    
    // eject the CD:
    if ([[NSApp delegate] mediaFolder] == NULL && strcasecmp (myCommandOption, "eject") == 0)
    {
        // eject the CD:
        if (gCDDevice[0] != 0x00)
        {
            NSString	*myDevicePath = [NSString stringWithCString: gCDDevice];
            
            if (myDevicePath != NULL)
            {
                MP3Audio_Shutdown ();
                
                if (![[NSWorkspace sharedWorkspace] unmountAndEjectDeviceAtPath: myDevicePath])
                {
                    MP3Audio_Error (CDERR_EJECT);
                }
            }
            else
            {
                MP3Audio_Error (CDERR_EJECT);
            }
        }
        else
        {
            MP3Audio_Error (CDERR_NO_MEDIA_FOUND);
        }
        
		return;
    }
    
    // output CD info:
    if (strcasecmp(myCommandOption, "info") == 0)
    {
        if (gCDTrackCount == 0)
        {
            MP3Audio_Error (CDERR_NO_FILES_FOUND);
        }
        else
        {
            if (gCDController != NULL && GetMovieActive (gCDController) == YES)
            {
                Con_Printf ("Playing track %d of %d (\"%s\").\n", gCDCurTrack, gCDTrackCount, gCDDevice);
            }
            else
            {
                Con_Printf ("Not playing. Tracks: %d (\"%s\").\n", gCDTrackCount, gCDDevice);
            }
            Con_Printf ("Volume is: %.2f.\n", mp3_volume.floater);
        }
        
		return;
    }
}

//_________________________________________________________________________________________________________________________eOF
