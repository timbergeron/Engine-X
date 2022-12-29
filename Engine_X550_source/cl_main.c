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
// cl_main.c -- client main loop
// Baker: Validated 6-27-2011

#include "quakedef.h"

// we need to declare some mouse variables here, because the menu system
// references them even when on a unix system.

client_static_t		cls;
client_state_t		cl;
// FIXME: put these on hunk?
efrag_t			cl_efrags[MAX_EFRAGS];
entity_t		cl_static_entities[MAX_STATIC_ENTITIES];
lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
dlight_t		cl_dlights[MAX_DLIGHTS];

#if VARIABLE_EDICTS_AND_ENTITY_SIZE
entity_t		*cl_entities; //johnfitz -- was a static array, now on hunk
//int				cl_max_edicts; //johnfitz -- only changes when new map loads
#else
entity_t		cl_entities[DEFAULT_MAX_EDICTSS];
#endif

int				cl_numvisedicts;
entity_t		*cl_visedicts[MAX_VISEDICTS];

modelindex_t	cl_modelindex[NUM_MODELINDEX];
char			*cl_modelnames[NUM_MODELINDEX];


unsigned CheckModel (char *mdl);

tagentity_t		q3player_body, q3player_head;

/*
=====================
CL_ClearState
=====================
*/
void CL_ClearState (void)
{
	int	i;

	if (!sv.active)
		Host_ClearMemory ();

	CL_ClearTEnts ();

// wipe the entire cl structure
//	memset (&cl, 0, sizeof(cl));  // Technically redundant as Host_ClearMemory already did this.

	SZ_Clear (&cls.message);

// clear other arrays
	memset (cl_beams, 0, sizeof(cl_beams));
	memset (cl_dlights, 0, sizeof(cl_dlights));
	memset (cl_efrags, 0, sizeof(cl_efrags));
	memset (cl_lightstyle, 0, sizeof(cl_lightstyle));
	memset (cl_temp_entities, 0, sizeof(cl_temp_entities));

#if VARIABLE_EDICTS_AND_ENTITY_SIZE
	//johnfitz -- cl_entities is now dynamically allocated
//	cl_max_edicts = CLAMP (256,(int)max_edicts.value, 8192);
	cl.max_entities = sv.active ? sv.max_edicts : (host_maxedicts.integer ? host_maxedicts.integer : DEFAULT_MAX_EDICTS);
	cl.max_entities = CLAMP (MIN_EDICTS_FLOOR, cl.max_entities, MAX_EDICTS_CAP);
	cl_entities = Hunk_AllocName (cl.max_entities, sizeof(entity_t), "cl_entities");
	//johnfitz

#else
	cl.max_entities = DEFAULT_MAX_EDICTS;
	memset (cl_entities, 0, sizeof(cl_entities));

#endif
// allocate the efrags and chain together into a free list
	cl.free_efrags = cl_efrags;
	for (i=0 ; i<MAX_EFRAGS-1 ; i++)
		cl.free_efrags[i].entnext = &cl.free_efrags[i+1];
	cl.free_efrags[i].entnext = NULL;

#ifdef SUPPORTS_NEHAHRA
	if (nehahra)
		SHOWLMP_clear ();
#endif

	{
		extern	float	scr_centertime_off;
		extern	char	con_lastcenterstring[1024];
		scr_centertime_off = 0;			// This ugly guy goes here
		con_lastcenterstring[0] = 0;	// Baker: this is probably more appropriate for R_NewMap
	}
}



/*
=====================
CL_Disconnect

Sends a disconnect message to the server
This is also called on Host_Error, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect (void)
{
// stop sounds (especially looping!)
	S_StopAllSounds (true);

	MP3Audio_Stop();

#ifdef SUPPORTS_NEHAHRA
	if (nehahra)
		FMOD_Stop_f ();
#endif

// bring the console down and fade the colors back to normal
//	SCR_BringDownConsole ();

	// This makes sure ambient sounds remain silent
	cl.worldmodel = NULL;

#if HTTP_DOWNLOAD
	// We have to shut down webdownloading first
	if( cls.download.web )
	{
		cls.download.disconnect = true;
		return;
	}

#endif

// if running a local server, shut it down
	if (cls.demoplayback)
	{

		CL_StopPlayback ();

		{	// Baker: clear the demos queue
			int i;
			// LordHavoc: clear the remaining slots
			for (i=0;i < MAX_DEMOS;i++)
				cls.demos[i][0] = 0;

			cls.demonum = -1;
		}
	}
	else if (cls.state == ca_connected)
	{
		if (cls.demorecording)
			CL_Stop_f ();

		Con_DevPrintf (DEV_PROTOCOL, "Sending clc_disconnect\n");
		SZ_Clear (&cls.message);
		MSG_WriteByte (&cls.message, clc_disconnect);
		NET_SendUnreliableMessage (cls.netcon, &cls.message);
		SZ_Clear (&cls.message);
		NET_Close (cls.netcon);

		cls.state = ca_disconnected;
		if (sv.active)
			Host_ShutdownServer (false);
	}

	cls.demoplayback = cls.timedemo = false;
	cls.signon = 0;

#ifdef SUPPORTS_NEHAHRA
	if (nehahra)
	        Neh_ResetSFX ();
#endif

	Con_DevPrintf (DEV_PROTOCOL, "CL_Disconnect is Ending loading plaque\n");
	SCR_EndLoadingPlaque (); // Baker: any disconnect state should end the loading plague, right?

}

void CL_Disconnect_f (void)
{
#if HTTP_DOWNLOAD
	// We have to shut down webdownloading first
	if( cls.download.web )
	{
		cls.download.disconnect = true;
		return;
	}

#endif
	CL_Disconnect ();
	if (sv.active)
		Host_ShutdownServer (false);
}

/*
=====================
CL_EstablishConnection

Host should be either "local" or a net address to be passed on
=====================
*/
void CL_EstablishConnection (char *host)
{
	if (cls.state == ca_dedicated)
		return;

#ifdef SUPPORTS_NEHAHRA
	if (nehahra)
	        num_sfxorig = num_sfx;
#endif

	if (cls.demoplayback)
		return;

	CL_Disconnect ();

	cls.netcon = NET_Connect (host);
	if (!cls.netcon) // Baker 3.60 - Rook's Qrack port 26000 notification on failure
	{
		Con_Printf ("\nsyntax: connect server:port (port is optional)\n");//r00k added
		if (net_hostport != 26000)
			Con_Printf ("\nTry using port 26000\n");//r00k added
		Host_Error ("connect failed");
	}

	Con_DevPrintf (DEV_PROTOCOL, "CL_EstablishConnection: connected to %s\n", host);

	// JPG - proquake message
	if (cls.netcon->mod == MOD_PROQUAKE)
	{
		if (pq_cheatfree)
			Con_Printf("%c%cConnected to Cheat-Free server%c\n", 1, 29, 31);
		else
			Con_Printf("%c%cConnected to ProQuake server%c\n", 1, 29, 31);
	}
	cls.demonum = -1;			// not in the demo loop now
	cls.state = ca_connected;
	cls.signon = 0;				// need all the signon messages before playing

	MSG_WriteByte (&cls.message, clc_nop);	// JPG 3.40 - fix for NAT
}

static unsigned source_data[1056] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

byte *COM_LoadFile (char *path, int usehunk);	// JPG 3.00 - model checking
static unsigned source_key1 = 0x36117cbd;
static unsigned source_key2 = 0x2e26857c;

/*
=====================
CL_SignonReply

An svc_signonnum has been received, perform a client side setup
=====================
*/
void CL_SignonReply (void)
{
	char 	str[8192];
	int i;	// JPG 3.00

	Con_DevPrintf (DEV_PROTOCOL, "CL_SignonReply: %i\n", cls.signon);

	switch (cls.signon)
	{
	case 1:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "prespawn");

		// JPG 3.50
		if (cls.netcon && !cls.netcon->encrypt)
			cls.netcon->encrypt = 3;
		break;

	case 2:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("name \"%s\"\n", cl_net_name.string));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("color %i %i\n", ((int)cl_net_color.integer) >> 4, ((int)cl_net_color.integer) & 15));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		snprintf (str, sizeof(str), "spawn %s", cls.spawnparms);
		MSG_WriteString (&cls.message, str);

		// JPG 3.20 - model and .exe checking
		if (pq_cheatfree)
		{
			FILE *f;
			unsigned crc;
			char path[MAX_OSPATH];
#if SUPPORTS_CHEATFREE_MODE

			strcpy(path, argv[0]);
#endif // ^^ MACOSX can't support this code but Windows/Linux do

/*#ifdef _WIN32
			if (!strstr(path, ".exe") && !strstr(path, ".EXE"))
				strlcat (path, ".exe", sizeof(path));
#endif // ^^ This is Windows operating system specific; Linux does not need
*/

			Sys_AppendBinaryExtension (path);

			f = FS_fopen_read(path, "rb");
			if (!f)
				Host_Error("Could not open %s", path);
			fclose(f);
			crc = Security_CRC_File(path);
			MSG_WriteLong(&cls.message, crc);
			MSG_WriteLong(&cls.message, source_key1);

			if (!cl.model_precache[1])
				MSG_WriteLong(&cls.message, 0);
			for (i = 1 ; cl.model_precache[i] ; i++)
			{
				if (cl.model_precache[i]->name[0] != '*')
				{
					byte *data;
					int len;

					data = QFS_LoadFile(cl.model_precache[i]->name, 2, NULL);			// 2 = temp alloc on hunk
					if (data)
					{
						len = (*(int *)(data - 12)) - 16;							// header before data contains size
						MSG_WriteLong(&cls.message, Security_CRC(data, len));
					}
					else
						Host_Error("Could not load %s", cl.model_precache[i]->name);
				}
			}
		}

		break;

	case 3:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "begin");
		Cache_Report ();		// print remaining memory

		// JPG 3.50
		if (cls.netcon)
			cls.netcon->encrypt = 1;
		break;

	case 4:
		Con_DevPrintf (DEV_PROTOCOL, "CL_SignonReply is Ending loading plaque\n");
		SCR_EndLoadingPlaque ();	// allow normal screen updates
#if HIDE_DAMN_CONSOLE
		{
			extern qbool scr_drawloading;
			extern qbool scr_disabled_for_newmap;
			scr_drawloading = false; // Baker:  bad, bad, bad ...
			scr_disabled_for_newmap = false;
		}
#endif
		break;
	}
}

/*
=====================
CL_NextDemo

Called to play the next demo in the demo loop
=====================
*/

//Baker 4.58 note:
// Only difference is that it returns success or failure

int CL_NextDemo (void)
{
	char	str[128];

	if (cls.demonum == -1)
		return 0;		// don't play demos

	Con_DevPrintf (DEV_PROTOCOL, "CL_NextDemo: Begin loading plaque\n");
	SCR_BeginLoadingPlaque ();

	if (!cls.demos[cls.demonum][0] || cls.demonum == MAX_DEMOS)
	{
		cls.demonum = 0;
		if (!cls.demos[cls.demonum][0])
		{
				Con_DevPrintf (DEV_DEMOS, "No demos listed with startdemos\n");

			CL_Disconnect();	// JPG 1.05 - patch by CSR to fix crash
			cls.demonum = -1;
			return 0;	// didn't start a demo
		}
	}

	snprintf (str, sizeof(str), "playdemo %s\n", cls.demos[cls.demonum]);
	Cbuf_InsertText (str);
	cls.demonum++;

	return 1;	// playing a demo
}

/*
==============
CL_PrintEntities_f
==============
*/
static void CL_PrintEntities_f (void)
{
	entity_t	*ent;
	int		i;

	for (i = 0, ent = cl_entities ; i < cl.num_entities ; i++, ent++)
	{
		Con_Printf ("%3i:", i);
		if (!ent->model)
		{
			Con_Printf ("EMPTY\n");
			continue;
		}
		Con_Printf ("%s:%2i  (%5.1f,%5.1f,%5.1f) [%5.1f %5.1f %5.1f]\n", ent->model->name, ent->frame, ent->origin[0], ent->origin[1], ent->origin[2], ent->angles[0], ent->angles[1], ent->angles[2]);
	}
}

/*
===============
CL_LerpPoint

Determines the fraction between the last two messages that the objects
should be put at.
===============
*/
static float CL_LerpPoint (void)
{
	float	f, frac;

	f = cl.mtime[0] - cl.mtime[1];

	if (!f || cl_ent_nolerp.integer || cls.timedemo || sv.active)
	{
		// Baker 3.75 demo rewind
		cl.time = cl.ctime = cl.mtime[0];
		return 1;
	}

	if (f > 0.1)
	{	// dropped packet, or start of demo
		cl.mtime[1] = cl.mtime[0] - 0.1;
		f = 0.1;
	}
	frac = (cl.ctime - cl.mtime[1]) / f;

	if (frac < 0)
	{
		if (frac < -0.01)
			cl.time = cl.ctime = cl.mtime[1];
		frac = 0;
	}
	else if (frac > 1)
	{
		if (frac > 1.01)
			cl.time = cl.ctime = cl.mtime[0];
		frac = 1;
	}

	return frac;
}

vec3_t	player_origin[MAX_SCOREBOARD];
int	numplayers;
//Baker: All lerpy stuff taken into account
/*
===============
CL_RelinkEntities
===============
*/
// Baker: This is where the visible edicts list is built
//        and "final" determination of entity properties for the frame
//        are made.
// This is sort of world pre-setup

static void CL_RelinkEntities (void)
{
	entity_t	*ent;
	int		i, j;
	float		frac, f, d, bobjrotate;
	vec3_t		delta, oldorg;
	dlight_t	*dl;
	model_t		*model;
	qbool Monster_isDead (int modelindex, int frame);
	qbool Model_isHead (int modelindex);

	// determine partial update time
	frac = CL_LerpPoint ();

	// JPG - check to see if we need to update the status bar
	if (scr_hud_timer.integer && ((int)cl.time != (int)cl.oldtime))
	Sbar_Changed();

	cl_numvisedicts = 0;

	// interpolate player info
	for (i=0 ; i<3 ; i++)
		cl.velocity[i] = cl.mvelocity[1][i] + frac * (cl.mvelocity[0][i] - cl.mvelocity[1][i]);
	//PROQUAKE ADDITION --START
	if (cls.demoplayback || (cl.last_angle_time > host_time && !(in_attack.state & 3)) && v_pq_smoothcam.integer) // JPG - check for last_angle_time for smooth chasecam!
	{
		// interpolate the angles
		for (j=0 ; j<3 ; j++)
		{
			d = cl.mviewangles[0][j] - cl.mviewangles[1][j];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;

			// JPG - I can't set cl.viewangles anymore since that messes up the demorecording.  So instead,
			// I'll set lerpangles (new variable), and view.c will use that instead.
			cl.lerpangles[j] = cl.mviewangles[1][j] + frac*d;
		}
	}
	else
	VectorCopy(cl.viewangles, cl.lerpangles);
	//PROQUAKE ADDITION --END

	bobjrotate = anglemod (100 * cl.time);
	numplayers = 0;

	// start on the entity after the world
	for (i = 1, ent = cl_entities + 1 ; i < cl.num_entities ; i++, ent++)
	{
		if (!ent->model)
		{	// empty slot
			if (ent->forcelink)
				Efrags_RemoveEfrags (ent);	// just became empty
			continue;
		}

		// if the object wasn't included in the last packet, remove it
		if (ent->msgtime != cl.mtime[0])
		{
			ent->model = NULL;
#if FITZQUAKE_PROTOCOL
			// DATAEVENT_MODEL_CHANGED because model is null
			ent->lerpflags |= LERP_RESETMOVE|LERP_RESETANIM; //johnfitz -- next time this entity slot is reused, the lerp will need to be reset
#endif
#if 0 // OLD_INTERPOLATE
			// fenix@io.com: model transform interpolation
			ent->frame_start_time = ent->translate_start_time = ent->rotate_start_time = 0;
#endif
			continue;
		}

		VectorCopy (ent->origin, oldorg);

		if (ent->forcelink)
		{	// the entity was not updated in the last message so move to the final spot
			VectorCopy (ent->msg_origins[0], ent->origin);
			VectorCopy (ent->msg_angles[0], ent->angles);
		}
		else
		{	// if the delta is large, assume a teleport and don't lerp
			f = frac;
			for (j=0 ; j<3 ; j++)
			{
				delta[j] = ent->msg_origins[0][j] - ent->msg_origins[1][j];
				if (delta[j] > 100 || delta[j] < -100)
				{
					f = 1;		// assume a teleportation, not a motion
#if FITZQUAKE_PROTOCOL
					// DATAEVENT_TELEPORT_DO_NOT_LERP_MOVE
					ent->lerpflags |= LERP_RESETMOVE; //johnfitz -- don't lerp teleports
#endif
				}
			}
#if 1  // NEW_INTERPOLATE // Baker: what is this doing?
			//johnfitz -- don't cl_lerp entities that will be r_lerped

			// DATAEVENT_NO_MONSTER_LERP
			if (scene_lerpmove.integer && (ent->lerpflags & LERP_MOVESTEP))
				f = 1;
			//johnfitz
#endif
#if 0 // OLD INTERPOLATE
			if (f >= 1)
			{
// Baker: FitzQuake interpolation LERP reset
				ent->translate_start_time = 0;
				ent->rotate_start_time = 0;
			}
#endif

		// interpolate the origin and angles
			for (j=0 ; j<3 ; j++)
			{
				ent->origin[j] = ent->msg_origins[1][j] + f*delta[j];

				d = ent->msg_angles[0][j] - ent->msg_angles[1][j];
				if (d > 180)
					d -= 360;
				else if (d < -180)
					d += 360;
				ent->angles[j] = ent->msg_angles[1][j] + f*d;
			}
		}

		{
			int			client_no		= ent - cl_entities;
			qbool		isPlayer		= (client_no >= 1 && client_no<=cl.maxclients);

			if (isPlayer)	// gamehack
			{
				VectorCopy (ent->origin, player_origin[numplayers]);
				numplayers++;
			}
		}

		if ((cl_ent_disable_blood.integer || cl_ent_deadbodyfilter.integer) && ent->model->modelformat == mod_alias &&
		    Monster_isDead(ent->modelindex, ent->frame))
			continue;

		if ((cl_ent_disable_blood.integer || cl_ent_gibfilter.integer) && ent->model->modelformat == mod_alias &&
		    (ent->modelindex == cl_modelindex[mi_gib1] || ent->modelindex == cl_modelindex[mi_gib2] ||
		     ent->modelindex == cl_modelindex[mi_gib3] || Model_isHead(ent->modelindex)))
			continue;

		if (ent->modelindex == cl_modelindex[mi_explo1] || ent->modelindex == cl_modelindex[mi_explo2])
		{
			// software removal of sprites
			if (particle_explosiontype.integer == 2 || particle_explosiontype.integer == 3
			    || qmb_explosions.integer
			    )
				continue;
		}

		if (!(model = cl.model_precache[ent->modelindex]))
			Host_Error ("CL_RelinkEntities: bad modelindex");

//		if (ent->modelindex == cl_modelindex[mi_rocket] &&
//		    cl_rocket2grenade.integer && cl_modelindex[mi_grenade] != -1)
//			ent->model = cl.model_precache[cl_modelindex[mi_grenade]];

		// rotate binary objects locally
		if (ent->model->modelflags & EF_ROTATE)
		{
			ent->angles[1] = bobjrotate;
			if (cl_ent_rotate_items_bob.integer)
				ent->origin[2] += sinf(bobjrotate / 90 * M_PI) * 5 + 5;
		}

		// EF_BRIGHTFIELD is not used by original progs
		if (ent->effects & EF_BRIGHTFIELD)
			R_EntityParticles (ent);

		if ((ent->effects & EF_MUZZLEFLASH) && light_muzzleflash.integer)
		{
			vec3_t	fv;

			dl = Light_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			dl->origin[2] += 16;
			AngleVectors (ent->angles, fv, NULL, NULL);
			VectorMultiplyAdd (dl->origin, 18, fv, dl->origin);
			dl->radius = 200 + (rand() & 31);
			dl->minlight = 32;
			dl->die = cl.time + 0.1;
#if 0 // Baker: don't use this yet
			//johnfitz -- assume muzzle flash accompanied by muzzle flare, which looks bad when lerped
			if (scene_lerpmodels.integer != 2)
			{
				if (ent == &cl_entities[cl.player_point_of_view_entity])
					cl.viewent.lerpflags |= LERP_RESETANIM|LERP_RESETANIM2; //no lerping for two frames
				else
					ent->lerpflags |= LERP_RESETANIM|LERP_RESETANIM2; //no lerping for two frames
			}
			//johnfitz
#endif

#ifdef GLQUAKE
			if (ent->modelindex == cl_modelindex[mi_shambler] && qmb_initialized && qmb_lightning.integer)
				dl->color_type = lt_blue;
			else
				dl->color_type = lt_muzzleflash;
#endif
		}

		// This JoeQuake code is SOOOO WRONG
		if (ent->modelindex != cl_modelindex[mi_eyes] &&
		    ((ent->modelindex != cl_modelindex[mi_player] && ent->model->modhint != MOD_PLAYER &&
		      ent->modelindex != cl_modelindex[mi_h_player]) || (light_powerups.integer && cl.stats[STAT_HEALTH] > 0)))
		{
			if ((ent->effects & (EF_BLUE | EF_RED)) == (EF_BLUE | EF_RED))
				Light_NewDlight (i, ent->origin, 200 + (rand() & 31), 0.1, lt_redblue);
			else if (ent->effects & EF_BLUE)
				Light_NewDlight (i, ent->origin, 200 + (rand() & 31), 0.1, lt_blue);
			else if (ent->effects & EF_RED)
				Light_NewDlight (i, ent->origin, 200 + (rand() & 31), 0.1, lt_red);
		// EF_BRIGHTLIGHT is not used by original progs
			else if (ent->effects & EF_BRIGHTLIGHT)
			{
				vec3_t	tmp;

				VectorCopy (ent->origin, tmp);
				tmp[2] += 16;
				Light_NewDlight (i, tmp, 400 + (rand() & 31), 0.1, lt_default);
			}
		// EF_DIMLIGHT is for powerup glows and enforcer's laser
			// Baker: Quad/Pent glow hack  ... Baker:  You silly.  This ancient hack is WRONG.
			else if (ent->effects & EF_DIMLIGHT)
			{
				if (ent->model->modhint == MOD_PLAYER && (cl.items & (IT_QUAD | IT_INVULNERABILITY)))
				{
					if ((cl.items & (IT_QUAD | IT_INVULNERABILITY)) == (IT_QUAD | IT_INVULNERABILITY))
						Light_NewDlight (i, ent->origin, 300 + (rand() & 31), 0.3, lt_redblue);
					else if (cl.items & IT_QUAD)
						Light_NewDlight (i, ent->origin, 300 + (rand() & 31), 0.3, lt_blue);
					else
						Light_NewDlight (i, ent->origin, 300 + (rand() & 31), 0.3, lt_red);
				}
				else
				{
					Light_NewDlight (i, ent->origin, 200 + (rand() & 31), 0.1, lt_default);
				}
			}
		}

		if (model->modelflags)
		{
			if (!ent->traildrawn || !VectorL2Compare(ent->trail_origin, ent->origin, 140))
			{
				VectorCopy (ent->origin, oldorg);	//not present last frame or too far away
				ent->traildrawn = true;
			}
			else
				VectorCopy (ent->trail_origin, oldorg);

			if (model->modelflags & EF_GIB)
				R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, BLOOD_TRAIL);
			else if (model->modelflags & EF_ZOMGIB)
				R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, SLIGHT_BLOOD_TRAIL);
			else if (model->modelflags & EF_TRACER)
				R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, TRACER1_TRAIL);
			else if (model->modelflags & EF_TRACER2)
				R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, TRACER2_TRAIL);
			else if (model->modelflags & EF_ROCKET)
			{
				if (model->modhint == MOD_LAVABALL)
				{
					R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, LAVA_TRAIL);

					dl = Light_AllocDlight (i);
					VectorCopy (ent->origin, dl->origin);
					dl->radius = 100 * (1 + CLAMP (0, light_rockets.floater, 1));
					dl->die = cl.time + 0.1;
			#ifdef GLQUAKE
					dl->color_type = lt_rocket;
			#endif
				}
				else
				{
					if (particle_rockettrail.integer)
						R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, ROCKET_TRAIL);

					if (light_rockets.floater)
					{
						dl = Light_AllocDlight (i);
						VectorCopy (ent->origin, dl->origin);
						dl->radius = 100 * (1 + CLAMP (0, light_rockets.floater, 1));
						dl->die = cl.time + 0.1;
			#ifdef GLQUAKE
						dl->color_type = Light_SetDlightColor (light_rockets_color.integer, lt_rocket, false);
			#endif
					}

			#ifdef GLQUAKE
					if (qmb_initialized && qmb_trails.integer)
					{
						vec3_t	back;
						float	scale;

						VectorSubtract (oldorg, ent->origin, back);
						scale = 8.0 / VectorLength(back);
						VectorMultiplyAdd (ent->origin, scale, back, back);
						QMB_MissileFire (back, oldorg, ent->origin);
					}
			#endif
				}
			}
			else if ((model->modelflags & EF_GRENADE) && particle_grenadetrail.integer)
			{
				// Nehahra dem compatibility
#ifdef SUPPORTS_ENTITY_ALPHA_OLD // back2forwards
				if (ent->transparency == -1 && cl.time >= ent->smokepuff_time)
				{
					R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, NEHAHRA_SMOKE);
					ent->smokepuff_time = cl.time + 0.14;
				}
				else
#endif
					R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, GRENADE_TRAIL);
			}
			else if (model->modelflags & EF_TRACER3)
				R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, VOOR_TRAIL);
		}

		ent->forcelink = false;

		{
			extern qbool chase_nodraw;

			// chasecam test
			if (i == cl.player_point_of_view_entity && !chase_active.integer) continue;
			if (i == cl.player_point_of_view_entity && chase_nodraw) continue;
		}


//		if (i == cl.player_point_of_view_entity && !chase_active.integer)
//			continue;

		// nehahra support
		if (ent->effects & EF_NODRAW)
			continue;

#ifdef GLQUAKE
		if (qmb_initialized)
		{
			if (ent->modelindex == cl_modelindex[mi_bubble])
			{
				if (!cl.paused && cl.oldtime != cl.time)
					QMB_StaticBubble (ent);
				continue;
			}
			else if (qmb_lightning.integer && ent->modelindex == cl_modelindex[mi_shambler] &&
				 ent->frame >= 65 && ent->frame <= 68)
			{
				vec3_t	liteorg;

				VectorCopy (ent->origin, liteorg);
				liteorg[2] += 32;
				QMB_ShamblerCharge (liteorg);
			}
			else if (qmb_spiketrails.integer && ent->model->modhint == MOD_SPIKE)
			{
				QMB_RocketTrail (oldorg, ent->origin, &ent->trail_origin, BUBBLE_TRAIL);
			}
		}
#endif


		if (cl_numvisedicts < MAX_VISEDICTS)
		{
			cl_visedicts[cl_numvisedicts] = ent;
			cl_numvisedicts++;
		}
	}
}

//#if SUPPORTS_SMOOTH_STAIRS
/*
===============
CL_CalcCrouch

Smooth out stair step ups
===============
*/
void CL_CalcCrouch (void)
{
	qbool	teleported;
	entity_t	*ent;
	static	vec3_t	oldorigin = {0, 0, 0};
	static	float	oldz = 0, extracrouch = 0, crouchspeed = 100;

	ent = &cl_entities[cl.player_point_of_view_entity];

	teleported = !VectorL2Compare(ent->origin, oldorigin, 48);
	VectorCopy (ent->origin, oldorigin);

	if (teleported)
	{
		// possibly teleported or respawned
		oldz = ent->origin[2];
		extracrouch = 0;
		crouchspeed = 100;
		cl.crouch = 0;
		return;
	}

	if (cl.onground && ent->origin[2] - oldz > 0)
	{
		if (ent->origin[2] - oldz > 20)
		{
			// if on steep stairs, increase speed
			if (crouchspeed < 160)
			{
				extracrouch = ent->origin[2] - oldz - host_frametime * 200 - 15;
				extracrouch = min(extracrouch, 5);
			}
			crouchspeed = 160;
		}

		oldz += host_frametime * crouchspeed;
		if (oldz > ent->origin[2])
			oldz = ent->origin[2];

		if (ent->origin[2] - oldz > 15 + extracrouch)
			oldz = ent->origin[2] - 15 - extracrouch;
		extracrouch -= host_frametime * 200;
		extracrouch = max(extracrouch, 0);

		cl.crouch = oldz - ent->origin[2];
	}
	else
	{
		// in air or moving down
		oldz = ent->origin[2];
		cl.crouch += host_frametime * 150;
		if (cl.crouch > 0)
			cl.crouch = 0;
		crouchspeed = 100;
		extracrouch = 0;
	}
}
//#endif

qbool	r_loadq3player = false;
#ifdef GLQUAKE
void CL_CopyPlayerInfo (entity_t *ent, entity_t *player)
{
	memcpy (&ent->baseline, &player->baseline, sizeof(entity_state_t));

	ent->msgtime = player->msgtime;
	memcpy (ent->msg_origins, player->msg_origins, sizeof(ent->msg_origins));
	VectorCopy (player->origin, ent->origin);
	memcpy (ent->msg_angles, player->msg_angles, sizeof(ent->msg_angles));
	VectorCopy (player->angles, ent->angles);

	ent->model = (ent == &q3player_body.ent) ? cl.model_precache[cl_modelindex[mi_q3torso]] : cl.model_precache[cl_modelindex[mi_q3head]];
	ent->efrag = player->efrag;

	ent->frame = player->frame;
	ent->syncbase = player->syncbase;
	ent->colormap = player->colormap;
	ent->effects = player->effects;
	ent->skinnum = player->skinnum;
	ent->visframe = player->visframe;
	ent->dlightframe = player->dlightframe;
	ent->dlightbits = player->dlightbits;

	ent->trivial_accept = player->trivial_accept;
	ent->topnode = player->topnode;

	ent->modelindex = (ent == &q3player_body.ent) ? cl_modelindex[mi_q3torso] : cl_modelindex[mi_q3head];
	VectorCopy (player->trail_origin, ent->trail_origin);
	ent->traildrawn = player->traildrawn;
//	ent->noshadow = player->noshadow;

	ent->alpha			= player->alpha;
//	ent->istransparent = player->istransparent;
//	ent->transparency = player->transparency;
	ent->smokepuff_time = player->smokepuff_time;
}
#endif

/*
===============
CL_ReadFromServer

Read all incoming data from the server
===============
*/
int CL_ReadFromServer (void)
{
	int	ret;
#if 0 // Baker: FitzQuake dev stats stuff
	extern int	num_temp_entities; //johnfitz
	int			num_beams = 0; //johnfitz
	int			num_dlights = 0; //johnfitz
	beam_t		*b; //johnfitz
	dlight_t	*l; //johnfitz
	int			i; //johnfitz
#endif
	/*cl.oldtime = cl.time;
	cl.time += host_frametime;  //Baker 3.75 old way */

	// Baker 3.75 - demo rewind
	cl.oldtime = cl.ctime;
	cl.time += host_frametime;
	if (!demorewind.integer || !cls.demoplayback)	// by joe
		cl.ctime += host_frametime;
	else
		cl.ctime -= host_frametime;
	// Baker 3.75 - end demo fast rewind

	do
	{
		ret = CL_GetMessage ();
		if (ret == -1)
			Host_Error ("CL_ReadFromServer: lost server connection");
		if (!ret)
			break;

		cl.last_received_message = realtime;
		CL_ParseServerMessage ();
	} while (ret && cls.state == ca_connected);

	if (cl_print_shownet.integer)
		Con_Printf ("\n");

	CL_RelinkEntities ();

//#if SUPPORTS_SMOOTH_STAIRS
	if (v_smoothstairs.integer)
		CL_CalcCrouch ();
//#endif


	CL_UpdateTEnts ();


#pragma message ("Quality assurance:  Enable devstats on these items below")
#if 0 // Baker: Fitz dev stats
//johnfitz -- devstats

	//visedicts
//	if (cl_numvisedicts > 256 && dev_peakstats.visedicts <= 256)
//		Con_Warning ("%i visedicts exceeds standard limit of 256.\n", cl_numvisedicts);
	if (cl_numvisedicts > MAX_WINQUAKE_VISEDICTS && dev_peakstats.visedicts <= MAX_WINQUAKE_VISEDICTS)
		Con_Warning ("%i visedicts exceeds standard limit of %i.\n", cl_numvisedicts, MAX_WINQUAKE_VISEDICTS);


	dev_stats.visedicts = cl_numvisedicts;
	dev_peakstats.visedicts = max(cl_numvisedicts, dev_peakstats.visedicts);

	//temp entities
//	if (num_temp_entities > 64 && dev_peakstats.tempents <= 64)
//		Con_Warning ("%i tempentities exceeds standard limit of 64.\n", num_temp_entities);
	if (num_temp_entities > MAX_WINQUAKE_TEMP_ENTITIES && dev_peakstats.tempents <= MAX_WINQUAKE_TEMP_ENTITIES)
		Con_Warning ("%i tempentities exceeds standard limit of %i.\n", num_temp_entities, MAX_WINQUAKE_TEMP_ENTITIES);
	dev_stats.tempents = num_temp_entities;
	dev_peakstats.tempents = max(num_temp_entities, dev_peakstats.tempents);

	//beams
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->model && b->endtime >= cl.time)
			num_beams++;
//	if (num_beams > 24 && dev_peakstats.beams <= 24)
//		Con_Warning ("%i beams exceeded standard limit of 24.\n", num_beams);
	if (num_beams > MAX_WINQUAKE_BEAMS && dev_peakstats.beams <= MAX_WINQUAKE_BEAMS)
		Con_Warning ("%i beams exceeded standard limit of %i.\n", num_beams, MAX_WINQUAKE_BEAMS);
	dev_stats.beams = num_beams;
	dev_peakstats.beams = max(num_beams, dev_peakstats.beams);

	//dlights
	for (i=0, l=cl_dlights ; i<MAX_DLIGHTS ; i++, l++)
		if (l->die >= cl.time && l->radius)
			num_dlights++;
//	if (num_dlights > 32 && dev_peakstats.dlights <= 32)
//		Con_Warning ("%i dlights exceeded standard limit of 32.\n", num_dlights);
	if (num_dlights > MAX_WINQUAKE_DLIGHTS && dev_peakstats.dlights <= MAX_WINQUAKE_DLIGHTS)
		Con_Warning ("%i dlights exceeded standard limit of %i.\n", num_dlights, MAX_WINQUAKE_DLIGHTS);
	dev_stats.dlights = num_dlights;
	dev_peakstats.dlights = max(num_dlights, dev_peakstats.dlights);

//johnfitz
#endif
// bring the links up to date
	return 0;
}

/*
=================
CL_SendCmd
=================
*/
void CL_SendCmd (void)
{
	usercmd_t	cmd;

	if (cls.state != ca_connected)
		return;

	if (cls.signon == SIGNONS)
	{
	// get basic movement from keyboard
		CL_BaseMove (&cmd);

	// allow mice or other external controllers to add to the move
		IN_Move (&cmd);

	// send the unreliable message
		CL_SendMove (&cmd);

	}

	if (cls.demoplayback)
	{
		SZ_Clear (&cls.message);
		return;
	}

// send the reliable message
	if (!cls.message.cursize)
		return;		// no message at all

	if (!NET_CanSendMessage(cls.netcon))
	{
		Con_DevPrintf (DEV_PROTOCOL, "CL_WriteToServer: can't send\n");
		return;
	}

	if (NET_SendMessage(cls.netcon, &cls.message) == -1)
		Host_Error ("CL_WriteToServer: lost server connection");

	SZ_Clear (&cls.message);
}





/*
=================
CL_Init
=================
*/
void CL_Init (void)
{
	SZ_Alloc (&cls.message, 1024);

	CL_InitInput ();
	CL_InitModelnames ();
//	CL_InitTEnts ();

// register our commands

	Cvar_Registration_Client_Main ();

		Cvar_KickOnChange (&user_effectslevel);  // Will this do something weird?

	//	Cvar_RegisterVariable (&cl_advancedcompletion, NULL);

	Cmd_AddCommand ("entities", CL_PrintEntities_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("stop", CL_Stop_f);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f);

//	Cvar_Registration_Client_Main ();

#if SUPPORTS_FOG
	Fog_Init (); //Cmd_AddCommand ("fog",CL_Fog_f);//R00k   Baker says: Next time!
#endif
}
