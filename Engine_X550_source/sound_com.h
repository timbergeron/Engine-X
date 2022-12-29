
#ifndef __SOUND_COM_H__
#define __SOUND_COM_H__


typedef struct
{
	qbool		initialized;				// Sound system initialized?
	qbool		sound_started;				// The sound system can be initialized only for it to fail later
} sound_com_t;

#endif // __SOUND_COM_H__
