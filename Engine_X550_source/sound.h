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
// sound.h -- client sound i/o functions

#ifndef __SOUND_H__
#define __SOUND_H__

#define DEFAULT_SOUND_PACKET_VOLUME 255
#define DEFAULT_SOUND_PACKET_ATTENUATION 1.0

// !!! if this is changed, it much be changed in asm_i386.h too !!!
typedef struct
{
	int 			left;
	int 			right;
} portable_samplepair_t;

typedef struct sfx_s
{
	char 			name[MAX_QPATH];
	cache_user_t	cache;
} sfx_t;

// !!! if this is changed, it much be changed in asm_i386.h too !!!
typedef struct
{
	int 			length;
	int 			loopstart;
	int 			speed;
	int 			width;
	int 			stereo;
	byte			data[1];		// variable sized
} sfxcache_t;

typedef struct
{
	qbool			gamealive;
	qbool			soundalive;
	qbool			splitbuffer;
	int				channels;
	int				samples;				// mono samples in buffer
	int				submission_chunk;		// don't mix less than this #
	int				samplepos;				// in mono samples
	int				samplebits;
	int				speed;
	unsigned char	*buffer;
} dma_t;

// !!! if this is changed, it much be changed in asm_i386.h too !!!
typedef struct
{
	sfx_t			*sfx;			// sfx number
	int				leftvol;		// 0-255 volume
	int				rightvol;		// 0-255 volume
	int				end;			// end time in global paintsamples
	int 			pos;			// sample position in sfx
	int				looping;		// where to loop, -1 = no looping
	int				entnum;			// to allow overriding a specific sound
	int				entchannel;		//
	vec3_t			origin;			// origin of sound effect
	vec_t			dist_mult;		// distance multiplier (attenuation/clipK)
	int				master_vol;		// 0-255 master volume
} channel_t;

typedef struct
{
	int				rate;
	int				width;
	int				channels;
	int				loopstart;
	int				samples;
	int				dataofs;		// chunk starts this many bytes from file start
} wavinfo_t;

sound_com_t *Sound_Init		(void);


void S_InitAmbients			(void);

void S_Startup				(void);
void S_Shutdown				(void);
void S_StartSound			(const int entnum, const int entchannel, sfx_t *sfx, const vec3_t origin, const float fvol,  const float attenuation);
void S_StaticSound			(sfx_t *sfx, const vec3_t origin, const float vol, const float attenuation);
void S_StopSound			(const int entnum, const int entchannel);
void S_StopAllSounds		(const qbool clear);
void S_ClearBuffer			(void);
void S_Update				(const vec3_t origin, const vec3_t v_forward, const vec3_t v_right, const vec3_t v_up);
void S_ExtraUpdate			(void);

sfx_t *S_PrecacheSound		(const char *sample);
void S_TouchSound			(const char *sample);
void S_ClearPrecache		(void);
void S_BeginPrecaching		(void);
void S_EndPrecaching		(void);
void S_PaintChannels		(const int endtime);
void S_InitPaintChannels	(void);


//channel_t *SND_PickChannel	(const int entnum, const int entchannel);		// picks a channel based on priorities, empty slots, number of channels
//void SND_Spatialize			(channel_t *ch);								// spatializes a channel
int SNDDMA_Init				(void);											// initializes cycling through a DMA buffer and returns information on it
int SNDDMA_GetDMAPos		(void);											// gets the current DMA position
void SNDDMA_Shutdown		(void);											// shutdown the DMA xfer.

// ====================================================================
// User-setable variables
// ====================================================================

#define	MAX_CHANNELS			512
#define	MAX_DYNAMIC_CHANNELS	128

#define	MAX_SFX		1024

extern	channel_t   channels[MAX_CHANNELS];
// 0 to MAX_DYNAMIC_CHANNELS-1	= normal entity sounds
// MAX_DYNAMIC_CHANNELS to MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS -1 = water, etc
// MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS to total_channels = static sounds

extern	int			total_channels;

//
// Fake dma is a synchronous faking of the DMA progress used for
// isolating performance in the renderer.  The fakedma_updates is
// number of times S_Update() is called per second.
//
//extern qbool fakedma;
//extern int fakedma_updates;

//extern vec3_t listener_origin;
//extern vec3_t listener_forward;
//extern vec3_t listener_right;
//extern vec3_t listener_up;
//extern vec_t sound_nominal_clip_dist;

extern int		paintedtime;
extern volatile dma_t *shm;
extern volatile dma_t sn;


extern qbool	snd_initialized;
extern int		snd_blocked;

void S_LocalSound			(const char *s);
sfxcache_t *S_LoadSound		(sfx_t *s);

wavinfo_t GetWavinfo		(const char *name, const byte *wav, const int wavlength);

void SND_InitScaletable 	(void);
void SNDDMA_Submit			(void);

void S_AmbientOff 			(void);
void S_AmbientOn 			(void);



#endif // __SOUND_H__
