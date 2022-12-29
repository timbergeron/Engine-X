#ifndef __ENGINE_H__
#define __ENGINE_H__

//#include "host_.h"		// hostparms
//#include "sys_.h"		// platform

//#include "video_.h"		// video
//#include "client_.h"	// client
//#include "screen_.h"	// screen
//#include "input_.h"		// mouse
//#include "renderer.h"	// renderer
//#include "gl_defs.h"	// opengl

//#include "console_.h"
//#include "cl_demo_.h"	// demoplay
//#include "mp3_.h"
//#include "server_.h"	// server


#include "sound_com.h"		// sound component
#include "mp3_com.h"		// mp3 component
#include "renderer_com.h"	// Renderer component
#include "hwgamma_com.h"	// Hardware gamma control

typedef struct
{
	sound_com_t		*sound;		// Sound sub-system
	mp3_com_t		*mp3;		// MP3 component
	renderer_com_t	*Renderer;	// Direct3D versus OpenGL.  If none, NULL ... dedicated server eventually will do NULL.
	hwgamma_com_t	*HWGamma;	// Hardware gamma control
} engine_globals_def_t;


#define RENDERER_NAME (engine.Renderer ? engine.Renderer->RendererText : "Dedicated")

extern engine_globals_def_t	engine;

#endif  // __ENGINE_H__


/*
typedef struct
{
	host_parms_t	*hostparms;					// The command line, directory and memory

	osdef_t			*platform;					// Handles operating system bullshit
	appdef_t		application;
	renderer_def_t	*renderer;
	opengl_def_t	opengl;
	videodef_t		video;
	consoledef_t	console;

	client_def_t	client;
	screendef_t		screen;
	input_t			input;						// input states
	mousedef_t		mouse;
	sounddef_t		sound;
	mp3def_t		mp3player;

	demo_t			demoplay;

	server_def_t	server;

} engine_globals_def_t;
*/
