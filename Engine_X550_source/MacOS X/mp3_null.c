#include "quakedef.h"

void MP3Audio_Stop (void) {}
void MP3Audio_Play (byte track, qbool looping)
 {}
void MP3Audio_Pause (void) {}
void MP3Audio_Resume (void) {}
void MP3Audio_Update (void) {}


void MP3Audio_Shutdown (void) {}
int MP3Audio_IsEnabled (void) {return -1;}
void MP3Audio_MessageHandler (void) {}

mp3_com_t *MP3_Init (void)
{
// Baker: this isn't necessary.  Dedicated server doesn't init this anyway.
//
//	if (cls.state == ca_dedicated)
//	{
//		MP3Com.enabled = false;
//		return -1;
//	}
	Cvar_RegisterVariable (&mp3_enabled, NULL); // Baker 3.99e: we want this available even if -nocdaudio is used (for menu)
	Cvar_RegisterVariable (&mp3_volume, NULL);

	Cvar_GetExternal (&mp3_enabled);
	Cvar_GetExternal (&mp3_volume);

	return NULL;
}