/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2005 John Fitzgibbons and others

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
// cvar.h

#ifndef __CVAR_H__
#define __CVAR_H__


/*

cvar_t variables are used to hold scalar or string variables that can be changed or displayed at the console or prog code as well as accessed directly
in C code.

it is sufficient to initialize a cvar_t with just the first two fields, or
you can add a ,true flag for variables that you want saved to the configuration
file when the game is quit:

cvar_t	r_draworder = {"r_draworder","1"};
cvar_t	scr_screensize = {"screensize","1",true};

Cvars must be registered before use, or they will have a 0 value instead of the
float interpretation of the string. Generally, all cvar_t declarations should be
registered in the apropriate init function before any console commands are executed:
Cvar_RegisterVariable (&host_framerate, NULL);

C code usually just references a cvar in place:
if ( r_draworder.value )

It could optionally ask for the value to be looked up for a string name:
if (Cvar_GetFloatByName ("r_draworder"))

Interpreted prog code can access cvars with the cvar(name) or
cvar_set (name, value) internal functions:
teamplay = cvar("teamplay");
cvar_set ("registered", "1");

The user can access cvars from the console in two ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.
*/

#define CVAR_NONE			0
#define CVAR_ARCHIVE		1		// set to true to cause it to be saved to config.cfg
#define CVAR_SERVER			2		// notifies players when changed

#define CVAR_EXTERNAL		4		// Cvar not written to config, but separate file.  Doesn't honor quake.rc startup config settings.  
									// Doesn't hard reset.  Doesn't regular reset.  Saves to and is read from its own file.
									// The philosophy is that these are user global settings and that no one except the user
									// Should be modifying them.  

// Read-Only CVAR Types

#define CVAR_ROM			8		// General read-only status.  Always Con_Printf if something tried to change this.
#define CVAR_CMDLINE		(16+8)	// Do not modify.  It was set by a command line switch.  It might be the command line.

// Marking purposes only for future alterations

#define CVAR_LEGACY 		(32+8)	// Mark cvars that don't serve a purpose these days. Considering elimination.
#define CVAR_PARITY			(64+8)	// Cvars that involve game fairness.


// Read-Only CVAR Types



#define CVAR_COMMAND		128		// OnChange function takes place post-processing as a pure function call
									// Baker: (EXPLAIN WHY)  I forget but it was imporant
									// M_DirectInput gets this
									// NOTE: CVAR_COMMAND does not allow for cancelling a change
									//       IN FACT, it fires AFTER change takes effect



//#define	CVAR_NORESET	  65536	// A couple of things like FOG and skybox support use these cvars for developer and
									// and user convenience, but are actually "owned" by the map.  Resetting them
									// in the middle of a map is just wrong.
									// Then again, changing the defaults is better because a user might tweak them
									// in the middle of a map and the reset gets it back.  Still, reset to defaults
									// Reset to session start.  Reset to defaults.

// Gamedir change.
// Disconnect.
// Save config.  Save any other files like iplog.
// Unload all textures and models and sounds.
// Clear hunk.
// Reset all cvars.
// Clear all aliases.
// Unbind all keys.

// However in reality, the above is overkill and probably too slow.  We'll see.

// load palette.  X-Men
// Conback, charset, crosshair, etc.
// quake.rc




typedef struct cvar_s
{
	const char		*cvarname;
	const char		*engine_defaultstring;			// The engine default value.  If changing a gamedir, we use this to reset for new game.
	int				flags;
	qbool			(*OnChange)(struct cvar_s *var, const char *value);
	float			floater;
	int				integer;
	
	char			*string;
												// Engine default is only stored on cvar registration
	char			*default_string;			// The game default value.    If resetting, we use this to reset for existing game.
	struct cvar_s	*next;
} cvar_t;

// Gamedir in future:  Will Unload all charset models and textures and pics and wad sbar and sounds and HUD pics and palette?
//                     Reset to engine defaults.  Externals are not reset.
//					   Exec quake.rc in whatever directory.  Default.cfg -> config.cfg -> Autoexec.cfg

extern cvar_t	*cvar_vars;

//void Cvar_RegisterVariable (cvar_t *variable, void *function); //johnfitz -- cvar callback

qbool Cvar_SetStringByRef (cvar_t *var, const char *value);		// BOSS: All changes flow through me. (except shit that does .value hardcoded in engine)
	void Cvar_SetDefaultStringByRef (cvar_t *var, const char *new_default);
	void Cvar_SetFloatByRef (cvar_t *var, float value);
	void Cvar_SetDefaultFloatByRef (cvar_t *var, float value);
	void Cvar_ForceSetStringByRef (cvar_t *var, const char *string); // force a set even if the cvar is read only
	void Cvar_ResetVarByRef (cvar_t *var, const qbool bHardReset);

void Cvar_SetFlagByRef (cvar_t *var, int newflag);	// Baker: Add a particular flag

qbool Cvar_GetExternal (cvar_t *var);

// For command line param checks like -dinput, etc.  Returns true if command line param and cvar are changed
// The return value allows us the flexibility to check for other things or not
qbool Cvar_CmdLineCheckForceFloatByRef_Maybe (int DevLevel, const char *cmdline_param, cvar_t *var, float value, const char *msg);

int Cvar_GetFlagsByRef (const cvar_t *var);
cvar_t *Cvar_FindVarByName (const char *var_name);
float	Cvar_GetFloatByName (const char *var_name);// returns 0 if not defined or non numeric
char	*Cvar_GetStringByName (const char *var_name); // returns an empty string if not defined
//qbool	Cvar_DeleteByName (const char *name);

int Cvar_CompleteCountPossible (const char *partial);
const char 	*Cvar_CompleteVariable (const char *partial);		// attempts to match a partial variable name for command line completion  returns NULL if nothing fits

qbool	Cvar_Command (void);		// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known command.  Returns true if the command was a variable reference that was handled. (print or change)

void Cvar_Init (void);
void 	Cvar_WriteVariables (FILE *f, const int SaveType);		// Writes lines containing "set variable value" for all variables with the archive flag set to true.

//void Cvar_SetCurrentGroup(char *name);
//void Cvar_ResetCurrentGroup(void);

void Cvar_CvarList_f (void);
void Cvar_Toggle_f (void);
void Cvar_Cycle_f (void);
void Cvar_Inc_f (void);

//void Cvar_Set_f (void);
void Cvar_CvarLock_f (void);

void Cvar_KickOnChange(cvar_t *var);

extern qbool accepts_default_values;	// Baker: our psuedo command injected after default.cfg tells us when to stop updating default values

// Baker: This is "wrong" but I'm sick of the cvar registry.h file being separate file
#include "cvar_registry.h"
#endif // __CVAR_H__