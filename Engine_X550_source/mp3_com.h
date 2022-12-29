#ifndef __MP3_H__
#define __MP3_H__

typedef struct
{
	qbool	initialized;		// It is functional
	qbool	enabled;			// It is on
	qbool	playing;
	qbool	looping;
	qbool	paused;
	byte	CurrentTrack;
} mp3_com_t;


#endif // __MP3_H__

