/*
Copyright (C) 2002, Anton Gavrilov

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
// version.h

#ifndef __VERSION_H__
#define __VERSION_H__

// Messages:
// MSVC: #pragma message ( "text" )
// GCC version: #warning "hello"
// #error to terminate compilation

// Create our own define for Mac OS X
#if defined(__APPLE__) && defined(__MACH__)
# define MACOSX
#endif

// Define Operating System Names

#ifdef _WIN32
# define OS_NAME "Windows"
//#error no touch
#elif defined(MACOSX)
# define OS_NAME "Mac OSX"
#else // Everything else gets to be Linux for now
# define OS_NAME "Linux"
# define LINUX // Everything else gets to be Linux for now ;)
#endif

#define VARIABLE_EDICTS_AND_ENTITY_SIZE			1	// This dynamically allocates client entities
#define IMAGEWORK_ON_HUNK						0	// This puts all the image work on the hunk, less flexible and requires a greater hunk allocation
#define HIGH_MEMORY_SYSTEM						1	// Normal.  Use extended limits
// Super major features can be explicitly declared
#define FITZQUAKE_PROTOCOL						1
#define SUPPORTS_MISSING_MODELS					1
#define SUPPORTS_GL_DELETETEXTURES				1	// Delete textures on start of new map
#define SUPPORTS_QCEXEC							1	// Execute QuakeC function from command line
#define SUPPORTS_MULTIMAP_DEMOS					1	// Plays demos record with 1 or more maps
#define SUPPORTS_EXTERNAL_ENTS					1
#define SUPPORTS_EXTERNAL_VIS					1
#define SUPPORTS_BRUSHMODEL_ROTATION			1	// Non-standard feature not in original Quake

// Library and system API availability limited features..
#define SUPPORTS_CHEATFREE_MODE					0	// Not in current form
#define SUPPORTS_AVI_CAPTURE					1	// AVI capture
#define SUPPORTS_OPEN_FOLDER					1	// Can it open a folder.  I don't know (if) this is possible on OSX
#define HTTP_DOWNLOAD							1	// Some operating systems, it may not be easy to implement
#define SUPPORTS_LIBPNG							0	// PNG library.  I have had trouble with GCC compiler on Windows
#define SUPPORTS_LIBJPEG						1	// JPEG library.  I think I have gotten this to work on GCC + Windows
#define SUPPORTS_DIRECTINPUT					1	// Windows feature
#define SUPPORTS_DZIP_DEMOS						1	// Play .dz demos.  Well requires dzip executable.
#define SUPPORTS_MP3_TRACKS						1	// Plays MP3 from <gamedir>\music
#define SUPPORTS_FTESTAINS						1	// FTE Stain maps

// Calculation intense features.  Platforms that hate floating point might be too slow or awkward
#define SUPPORTS_CHASECAM_FIX					1	// The R00k version of the chase cam fix

// Renderer oriented features.  May be hard or impossible or slow on some platforms.
#define SUPPORTS_HLBSP							1	// Half-Life map support.  Current implemention would be hard to do in software
#define SUPPORTS_HLBSP_EXTWADS					0	// I don't like external WAD usage.  Sometimes I need it for testing, though.
#define SUPPORTS_FOG							1	// D3DQuake can't do the fog thing
#define SUPPORTS_XFLIP							1	// Might be tough to support on some renderers
#define SUPPORTS_CONSOLE_SIZING					0	// GL can size the console; WinQuake can't do that yet (BAKER: THIS NOT IMPLEMENTED)
#define SUPPORTS_AUTOID							0	// We aren't supporting this quite yet
#define SUPPORTS_TEXTURE_POINTER				1	// Our texture pointer
#define SUPPORTS_OVERBRIGHT_SWITCH				1   // Baker: Realtime modification of lightmode

// Platform oriented features.  Sometimes Windows only, sometimes tough or even impossible on other platforms (fixed resolution platforms)
// Video ...
#define SUPPORTS_GLVIDEO_MODESWITCH  			1	// I might have trouble doing this on OS X
#define SUPPORTS_VSYNC 							1	// Some renderers and operating systems may not support this
#define SUPPORTS_DUAL_MONITOR_HACK				1	// -monitor2 command line param
// Input
#define RELEASE_MOUSE_FULLSCREEN				1	// Some operating systems do not have mouse (Baker: Note disabling this is broke)
#define SUPPORTS_INTERNATIONAL_KEYBOARD			0	// Currently only know how to do this on windows (BAKER: THIS NOT IMPLEMENTED)
#define SUPPORTS_CLIPBOARD_COPY					1	// Not all operating systems have a clipboard
#define WINDOWS_SCROLLWHEEL_PEEK				1	// CAPTURES MOUSEWHEEL WHEN keydest != game
//
#define SUPPORTS_DEMO_AUTOPLAY					1	// Uses file association.  Not sure how to do on OS X
#define SUPPORTS_SYSSLEEP						1	// Flash actually doesn't support "Sleep"

// These mostly server to mark code more than anything else
#define SUPPORTS_NEHAHRA_PROTOCOL_AS_CLIENT		1	// We don't want to support as server.  Fitzy protocol can do that.

#define GL_QUAKE_SKIN_METHOD						// GLQuake uses a different method for skinning
#define SUPPORTS_NEHAHRA							// Nehahra is kind of "non-standard".  Considering removal
#define SUPPORTS_COLORED_LIGHTS						// Markers for colored lights.  Software engines don't support easily.
#define SUPPORTS_CENTERED_MENUS						// Centered menus.  And scale stuff too.
#define SUPPORTS_EXTENDED_MENUS						// Extra menus.
#define SUPPORTS_QMB								// The "QMB" particle system
#define KEYS_NOVEAU									// Home and End and other keys do special things

#define PROQUAKE_HELP_SCREEN					0	// Extra ProQuake help screen
//#define SUPPORTS_SMOOTH_STAIRS					1	// Turn into cvar instead. I hate this.  Maybe make into cvar.
#define SUPPORTS_STUFFCMDS						1	// Marking this code out

#define INTRO_ANIMATION							1

// Features limitations I haven't been able to work around with MinGW + GCC compiler on Windows
#if defined( _WIN32) && defined(__GNUC__) // Baker: MinGW issues

#define OPENGL_ONLY								// Direct3D Wrapper isn't working with MinGW + GCC
												// DirectInput isn't working with GCC either.  Hmmmm.
#undef SUPPORTS_AVI_CAPTURE
#undef SUPPORTS_MP3_TRACKS
#undef SUPPORTS_DUAL_MONITOR_HACK
#undef SUPPORTS_LIBJPEG

#define SUPPORTS_AVI_CAPTURE					0					// Hopelessly Windows locked
#define SUPPORTS_MP3_TRACKS						0
#define SUPPORTS_DUAL_MONITOR_HACK				0
#define SUPPORTS_LIBJPEG                        0
#endif


// Dual monitor hack can only be supported with MSVC6 with an SDK that isn't available
// for download from Microsoft any longer.  Enabled for Microsoft Visual C++ Express 2008 only.
#if _MSC_VER <= 1200
#undef SUPPORTS_DUAL_MONITOR_HACK
#define SUPPORTS_DUAL_MONITOR_HACK				0
#endif

#ifdef _DEBUG
//#define OPENGL_ONLY //DIRECT3D_ONLY		// Baker: I test the builds in Direct3D instead of OpenGL with debug mode
//#define DIRECT3D_ONLY
							//        since the Direct3D wrapper is more fragile than the OpenGL it
							//		  is emulating
#endif

#ifdef MACOSX
#define OPENGL_ONLY

#undef HTTP_DOWNLOAD
#undef SUPPORTS_DZIP_DEMOS
#undef SUPPORTS_LIBJPEG
#undef SUPPORTS_AVI_CAPTURE
#undef SUPPORTS_GLVIDEO_MODESWITCH
#undef WINDOWS_SCROLLWHEEL_PEEK
#undef SUPPORTS_DIRECTINPUT
#undef SUPPORTS_OPEN_FOLDER
#undef SUPPORTS_CLIPBOARD_COPY

#define HTTP_DOWNLOAD							0	// Some operating systems, it may not be easy to implement
#define SUPPORTS_DZIP_DEMOS						0 	// There is no tool for this for OSX at the moment
#define SUPPORTS_LIBJPEG                        0
#define SUPPORTS_AVI_CAPTURE					0	// movie_avi.c Hopelessly Windows locked
#define SUPPORTS_GLVIDEO_MODESWITCH 			0
#define SUPPORTS_DIRECTINPUT 					0
#define WINDOWS_SCROLLWHEEL_PEEK 				0 
#define SUPPORTS_OPEN_FOLDER					0
#define SUPPORTS_CLIPBOARD_COPY					0	// Don't know how to write to the clipboard on OSX
#endif	// MACOSX
//#define DO_WGL	// Of all things ... this is ruining the alpha 2d gfx in ONLY the OpenGL *Release* build.
//                     Not the debug build.

#endif // __VERSION_H__
