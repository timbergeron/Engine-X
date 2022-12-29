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
// cl_parse.c -- parse a message received from the server

#include "quakedef.h"

static char *svc_strings[] =
{
	"svc_bad",
	"svc_nop",
	"svc_disconnect",
	"svc_updatestat",
	"svc_version",			// [long] server version
	"svc_setview",			// [short] entity number
	"svc_sound",			// <see code>
	"svc_time",			// [float] server time
	"svc_print",			// [string] null terminated string
	"svc_stufftext",		// [string] stuffed into client's console buffer
					// the string should be \n terminated
	"svc_setangle",			// [vec3] set the view angle to this absolute value

	"svc_serverinfo",		// [long] version
					// [string] signon string
					// [string]..[0]model cache [string]...[0]sounds cache
					// [string]..[0]item cache
	"svc_lightstyle",		// [byte] [string]
	"svc_updatename",		// [byte] [string]
	"svc_updatefrags",		// [byte] [short]
	"svc_clientdata",		// <shortbits + data>
	"svc_stopsound",		// <see code>
	"svc_updatecolors",		// [byte] [byte]
	"svc_particle",			// [vec3] <variable>
	"svc_damage",			// [byte] impact [byte] blood [vec3] from

	"svc_spawnstatic",
	"OBSOLETE svc_spawnbinary",
	"svc_spawnbaseline",

	"svc_temp_entity",		// <variable>
	"svc_setpause",
	"svc_signonnum",
	"svc_centerprint",
	"svc_killedmonster",
	"svc_foundsecret",
	"svc_spawnstaticsound",
	"svc_intermission",
	"svc_finale",			// [string] music [string] text
	"svc_cdtrack",			// [byte] track [byte] looptrack
	"svc_sellscreen",
	"svc_cutscene",
// nehahra support
	"svc_showlmp",			// [string] iconlabel [string] lmpfile [byte] x [byte] y
	"svc_hidelmp",			// [string] iconlabel
	"svc_skybox",			// 37	[string] skyname
	"?",			// 38
	"?",			// 39
	"svc_bf", 				// 40 no data
	"svc_fog", 				// 41 [byte] density [byte] red [byte] green [byte] blue [float] time
	"svc_spawnbaseline2", 	// 42 support for large modelindex, large framenum, alpha, using flags
	"svc_spawnstatic2", 	// 43 support for large modelindex, large framenum, alpha, using flags
	"svc_spawnstaticsound2",// 44 [coord3] [short] samp [byte] vol [byte] aten
	"?",	// 45
	"?",	// 46
	"?",	// 47
	"?",	// 48
	"?", 	// 49
	"?", 	// 50
	"svc_fog"				// [byte] enable <optional past this point, only included if enable is true> [float] density [byte] red [byte] green [byte] blue
};



//=============================================================================



/*
===============
CL_ParseDamage
===============
*/
void CL_ParseDamage (void)
{
	float		severity;
	void Viewblends_CalculateDamage_ColorBucket_f (const float count, const int armor, const int blood);


	// Put sbar face into pain frame
	cl.faceanimtime = cl.time + 0.2;

	{	// Read in the damage and calculate the color
		int armor = MSG_ReadByte ();
		int blood = MSG_ReadByte ();

		severity = blood*0.5 + armor*0.5;
		if (severity < 10) severity = 10;

		Viewblends_CalculateDamage_ColorBucket_f (severity, armor, blood);
	}

	{	// Read in the damage coordinates to generate a kick
		entity_t	*ent = &cl_entities[cl.player_point_of_view_entity];
		int 		i;
		vec3_t		from;

		for (i=0 ; i<3 ; i++)
			from[i] = MSG_ReadCoord ();

		VectorSubtract (from, ent->origin, from);
		VectorNormalize (from);

		{
			float		rollside, pitchside;
			vec3_t		forward, right, up;

			// calculate view angle kicks

			AngleVectors (ent->angles, forward, right, up);

			rollside  = DotProduct (from, right);
			pitchside = DotProduct (from, forward);

			View_CalculateDamageRollPitchTime (severity, rollside, pitchside);

		}
	}

}


/*
===============
CL_EntityNum

This error checks and tracks the total number of entities
===============
*/
entity_t *CL_EntityNum (int num)
{
#if FITZQUAKE_PROTOCOL
	//johnfitz -- check minimum number too
	if (num < 0)
		Host_Error ("CL_EntityNum: %i is an invalid number",num);
	//john
#endif

	if (num >= cl.num_entities)
	{
		if (num >= cl.max_entities) //johnfitz -- no more MAX_EDICTS
			Host_Error ("CL_EntityNum: %i is an invalid number", num);

		while (cl.num_entities <= num)
		{
			cl_entities[cl.num_entities].colormap = vid.colormap;
#if FITZQUAKE_PROTOCOL
			// DATAEVENT_BRAND_NEW_ENTITY
			cl_entities[cl.num_entities].lerpflags |= LERP_RESETMOVE|LERP_RESETANIM; //johnfitz
#endif
// Baker: FitzQuake interpolation LERP reset
			cl.num_entities++;
		}
	}

	return &cl_entities[num];
}

/*
==================
CL_ParseStartSoundPacket
==================
*/
static void CL_ParseStartSoundPacket (void)
{
	vec3_t	pos;
	int	i, channel, ent, sound_num, volume, field_mask;
	float	attenuation;

	field_mask = MSG_ReadByte ();

	if (field_mask & SND_VOLUME)
		volume = MSG_ReadByte();
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;

    if (field_mask & SND_ATTENUATION)
		attenuation = MSG_ReadByte () / 64.0;
	else
		attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;

#if FITZQUAKE_PROTOCOL
	//johnfitz -- PROTOCOL_FITZQUAKE
	if (field_mask & SND_LARGEENTITY)
	{
		ent = (unsigned short) MSG_ReadShort ();
		channel = MSG_ReadByte ();
	}
	else
#endif
	{
		channel = (unsigned short) MSG_ReadShort ();
		ent = channel >> 3;
		channel &= 7;
	}

#if FITZQUAKE_PROTOCOL
	if (field_mask & SND_LARGESOUND)
		sound_num = (unsigned short) MSG_ReadShort ();
	else
#endif
		sound_num = MSG_ReadByte ();

#if FITZQUAKE_PROTOCOL
	//johnfitz -- check soundnum
	if (sound_num >= MAX_SOUNDS)
		Host_Error ("CL_ParseStartSoundPacket: %i > MAX_SOUNDS", sound_num);
	//johnfitz
#endif

	if (ent > cl.max_entities)
		Host_Error ("CL_ParseStartSoundPacket: ent = %i", ent);

	for (i=0 ; i<3 ; i++)
		pos[i] = MSG_ReadCoord ();

	S_StartSound (ent, channel, cl.sound_precache[sound_num], pos, volume/255.0, attenuation);
}



/*
=====================
CL_NewTranslation
=====================
*/
void CL_NewTranslation (int slot)
{
	int		i, j, top, bottom;
	byte	*dest, *source;

	if (slot > cl.maxclients)
		Sys_Error ("CL_NewTranslation: slot > cl.maxclients");
	dest = cl.scores[slot].translations;
	source = vid.colormap;
	memcpy (dest, vid.colormap, sizeof(cl.scores[slot].translations));
	top = cl.scores[slot].colors & 0xf0;
	bottom = (cl.scores[slot].colors & 15) << 4;

#ifdef GL_QUAKE_SKIN_METHOD
	R_TranslatePlayerSkin (slot);
#endif

	for (i = 0 ; i < VID_GRADES ; i++, dest += 256, source += 256)
	{
		if (top < 128)	// the artists made some backwards ranges.  sigh.
			memcpy (dest + TOP_RANGE, source + top, 16);
		else
			for (j=0 ; j<16 ; j++)
				dest[TOP_RANGE+j] = source[top+15-j];

		if (bottom < 128)
			memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
		else
			for (j=0 ; j<16 ; j++)
				dest[BOTTOM_RANGE+j] = source[bottom+15-j];
	}
}

/*
=====================
CL_ParseStatic
=====================
*/

void CL_ParseBaseline (entity_t *ent, int version); //johnfitz -- added argument

// Baker: LERP ... we're good
#if FITZQUAKE_PROTOCOL
static void CL_ParseStatic (int version) //johnfitz -- added a parameter
#else
static void CL_ParseStatic (void)
#endif
{
	entity_t	*ent;
	int		i;

	i = cl.num_statics;
	if (i >= MAX_STATIC_ENTITIES)
		Host_Error ("Too many static entities.  Limit is %i", MAX_STATIC_ENTITIES);

	ent = &cl_static_entities[i];
	cl.num_statics++;
#if FITZQUAKE_PROTOCOL
	CL_ParseBaseline (ent, version); //johnfitz -- added second parameter
#else
	CL_ParseBaseline (ent);
#endif

// copy it to the current state
	ent->model = cl.model_precache[ent->baseline.modelindex];
#if FITZQUAKE_PROTOCOL
	// DATAEVENT_MODEL_CHANGED due to baseline
	ent->lerpflags |= LERP_RESETANIM | LERP_RESETMOVE; //johnfitz -- lerping  Baker: Added LERP_RESETMOVE to list
#endif
	ent->frame = ent->baseline.frame;
	ent->colormap = vid.colormap;
	ent->skinnum = ent->baseline.skin;
	ent->effects = ent->baseline.effects;
#if FITZQUAKE_PROTOCOL
	ent->alpha = ent->baseline.alpha; //johnfitz -- alpha
#endif

	VectorCopy (ent->baseline.origin, ent->origin);
	VectorCopy (ent->baseline.angles, ent->angles);
	Efrags_AddEfrags (ent);
}

/*
===================
CL_ParseStaticSound
===================
*/
#if FITZQUAKE_PROTOCOL
void CL_ParseStaticSound (int version) //johnfitz -- added argument
#else
void CL_ParseStaticSound (void)
#endif
{
	int	i, sound_num, vol, atten;
	vec3_t	org;

	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();

#if FITZQUAKE_PROTOCOL
	//johnfitz -- PROTOCOL_FITZQUAKE
	if (version == 2)
		sound_num = MSG_ReadShort ();
	else
#endif
		sound_num = MSG_ReadByte ();

	//johnfitz


	vol = MSG_ReadByte ();
	atten = MSG_ReadByte ();

	S_StaticSound (cl.sound_precache[sound_num], org, vol, atten);
}

// JPG - added this
static int MSG_ReadBytePQ (void)
{
	return MSG_ReadByte() * 16 + MSG_ReadByte() - 272;
}

// JPG - added this
static int MSG_ReadShortPQ (void)
{
	return MSG_ReadBytePQ() * 256 + MSG_ReadBytePQ();
}

/* JPG - added this function for ProQuake messages
=======================
CL_ParseProQuakeMessage
=======================
*/
void CL_ParseProQuakeMessage (void)
{
	int cmd, i;
	int team, frags, shirt, ping;

	MSG_ReadByte();
	cmd = MSG_ReadByte();

	switch (cmd)
	{
	case pqc_new_team:
		Sbar_Changed ();
		team = MSG_ReadByte() - 16;
		if (team < 0 || team > 13)	//color
			Host_Error ("CL_ParseProQuakeMessage: pqc_new_team invalid team");
		shirt = MSG_ReadByte() - 16;
		cl.teamgame = true;
		// cl.teamscores[team].frags = 0;	// JPG 3.20 - removed this
		cl.teamscores[team].colors = 16 * shirt + team;
		//Con_Printf("pqc_new_team %d %d\n", team, shirt);
		break;

	case pqc_erase_team:
		Sbar_Changed ();
		team = MSG_ReadByte() - 16;
		if (team < 0 || team > 13)	// color
			Host_Error ("CL_ParseProQuakeMessage: pqc_erase_team invalid team");
		cl.teamscores[team].colors = 0;
		cl.teamscores[team].frags = 0;		// JPG 3.20 - added this
		//Con_Printf("pqc_erase_team %d\n", team);
		break;

	case pqc_team_frags:
		Sbar_Changed ();
		team = MSG_ReadByte() - 16;
		if (team < 0 || team > 13)	// color
			Host_Error ("CL_ParseProQuakeMessage: pqc_team_frags invalid team");
		frags = MSG_ReadShortPQ();
		if (frags & 32768)
			frags = frags - 65536;
		cl.teamscores[team].frags = frags;
		//Con_Printf("pqc_team_frags %d %d\n", team, frags);
		break;

	case pqc_match_time:
		Sbar_Changed ();
		cl.minutes = MSG_ReadBytePQ();
		cl.seconds = MSG_ReadBytePQ();
		cl.last_match_time = cl.time;
		//Con_Printf("pqc_match_time %d %d\n", cl.minutes, cl.seconds);
		break;

	case pqc_match_reset:
		Sbar_Changed ();
		for (i = 0 ; i < 14 ; i++) //color
		{
			cl.teamscores[i].colors = 0;
			cl.teamscores[i].frags = 0;		// JPG 3.20 - added this
		}
		//Con_Printf("pqc_match_reset\n");
		break;

	case pqc_ping_times:
		while (ping = MSG_ReadShortPQ())
		{
			if ((ping / 4096) >= cl.maxclients)
				Host_Error ("CL_ParseProQuakeMessage: pqc_ping_times > MAX_SCOREBOARD");
			cl.scores[ping / 4096].ping = ping & 4095;
		}
		cl.last_ping_time = cl.time;
		/*
		Con_Printf("pqc_ping_times ");
		for (i = 0 ; i < cl.maxclients ; i++)
			Con_Printf("%4d ", cl.scores[i].ping);
		Con_Printf("\n");
		*/
		break;
	}
}

//  Q_VERSION



 void Q_Version(char *s)
{
	static float q_version_reply_time = -20.0; // Baker: so it can be instantly used
	char *t;
	int l = 0, n = 0;

	// Baker: do not allow spamming of it, 20 second interval max
	if (realtime - q_version_reply_time < 20)
		return;

	t = s;
	t += 1;  // Baker: lazy, to avoid name "q_version" triggering this; later do it "right"
	l = strlen(t);

	while (n < l)
	{
		if (!strncmp(t, ": q_version", 9))
		{
				Cbuf_AddText (va("say %s version %s\n", ENGINE_NAME, VersionString()));
				Cbuf_Execute ();
				q_version_reply_time = realtime;
				break; // Baker: only do once per string
		}
		n += 1;t += 1;
	}
}

/* JPG - on a svc_print, check to see if the string contains useful information
======================
CL_ParseProQuakeString
======================
*/
void CL_ParseProQuakeString (char *string)
{
	static int checkping = -1;
	int ping, i;
	char *s, *s2, *s3;
	static int checkip = -1;	// player whose IP address we're expecting

	// JPG 1.05 - for ip logging
	static int remove_status = 0;
	static int begin_status = 0;
	static int playercount = 0;

	// JPG 3.02 - made this more robust.. try to eliminate screwups due to "unconnected" and '\n'
	s = string;
	if (COM_StringMatch (string, "Client ping times:\n") && scr_hud_scoreboard_pings.integer)
	{
		cl.last_ping_time = cl.time;
		checkping = 0;
		if (!cl.console_ping)
			*string = 0;
	}
	else if (checkping >= 0)
	{
		while (*s == ' ')
			s++;
		ping = 0;
		if (*s >= '0' && *s <= '9')
		{
			while (*s >= '0' && *s <= '9')
				ping = 10 * ping + *s++ - '0';
			if ((*s++ == ' ') && *s && (s2 = strchr(s, '\n')))
			{
				s3 = cl.scores[checkping].name;
				while ((s3 = strchr(s3, '\n')) && s2)
				{
					s3++;
					s2 = strchr(s2+1, '\n');
				}
				if (s2)
				{
					*s2 = 0;
					if (!strncmp(cl.scores[checkping].name, s, 15))
					{
						cl.scores[checkping].ping = ping > 9999 ? 9999 : ping;
						for (checkping++ ; !*cl.scores[checkping].name && checkping < cl.maxclients ; checkping++);
					}
					*s2 = '\n';
				}
				if (!cl.console_ping)
					*string = 0;
				if (checkping == cl.maxclients)
					checkping = -1;
			}
			else
				checkping = -1;
		}
		else
			checkping = -1;
		cl.console_ping = cl.console_ping && (checkping >= 0);	// JPG 1.05 cl.sbar_ping -> cl.console_ping
	}

	// check for match time
	if (!strncmp(string, "Match ends in ", 14))
	{
		s = string + 14;
		if ((*s != 'T') && strchr(s, 'm'))
		{
			sscanf(s, "%d", &cl.minutes);
			cl.seconds = 0;
			cl.last_match_time = cl.time;
		}
	}
	else if (COM_StringMatch (string, "Match paused\n"))
		cl.match_pause_time = cl.time;
	else if (COM_StringMatch (string, "Match unpaused\n"))
	{
		cl.last_match_time += (cl.time - cl.match_pause_time);
		cl.match_pause_time = 0;
	}
	else if (COM_StringMatch (string, "The match is over\n") || !strncmp(string, "Match begins in", 15))
		cl.minutes = 255;
	else if (checkping < 0)
	{
		s = string;
		i = 0;
		while (*s >= '0' && *s <= '9')
			i = 10 * i + *s++ - '0';
		if (COM_StringMatch (s, " minutes remaining\n"))
		{
			cl.minutes = i;
			cl.seconds = 0;
			cl.last_match_time = cl.time;
		}
	}

	// JPG 1.05 check for IP information
	if (iplog_size)
	{
		if (!strncmp(string, "host:    ", 9))
		{
			begin_status = 1;
			if (!cl.console_status)
				remove_status = 1;
		}
		if (begin_status && !strncmp(string, "players: ", 9))
		{
			begin_status = 0;
			remove_status = 0;
			if (sscanf(string + 9, "%d", &playercount))
			{
				if (!cl.console_status)
					*string = 0;
			}
			else
				playercount = 0;
		}
		else if (playercount && string[0] == '#')
		{
			if (!sscanf(string, "#%d", &checkip) || --checkip < 0 || checkip >= cl.maxclients)
				checkip = -1;
			if (!cl.console_status)
				*string = 0;
			remove_status = 0;
		}
		else if (checkip != -1)
		{
			int a, b, c;
			if (sscanf(string, "   %d.%d.%d", &a, &b, &c) == 3)
			{
				cl.scores[checkip].addr = (a << 16) | (b << 8) | c;
				IPLog_Add(cl.scores[checkip].addr, cl.scores[checkip].name);
			}
			checkip = -1;
			if (!cl.console_status)
				*string = 0;
			remove_status = 0;

			if (!--playercount)
				cl.console_status = 0;
		}
		else
		{
			playercount = 0;
			if (remove_status)
				*string = 0;
		}
	}
	Q_Version(string);//R00k: look for "q_version" requests
}

#define SHOWNET(x) if(cl_print_shownet.integer==2)Con_Printf ("%3i:%s\n", msg_readcount-1, x);

#define CHECKDRAWSTATS						\
	if (scr_show_stats.integer == 3 || scr_show_stats.integer == 4)	\
		(drawstats_limit = cl.time + scr_show_stats_length.integer)

/*
=====================
CL_ParseServerMessage
=====================
*/
void Con_LogCenterPrint (char *str);
void CL_ParseClientdata (void);
void CL_ParseUpdate (int bits);

void CL_ParseServerMessage (void)
{
	int		cmd, i;
	extern	float	drawstats_limit;
	char	*str;
#if FITZQUAKE_PROTOCOL
	int lastcmd;
#endif
#if 0 //FITZQUAKE_PROTOCOL
//	int			total, j, lastcmd; //johnfitz
#endif


// if recording demos, copy the message out
	if (cl_print_shownet.integer == 1)
		Con_Printf ("%i ", net_message.cursize);
	else if (cl_print_shownet.integer == 2)
		Con_Printf ("------------------\n");

	cl.onground = false;	// unless the server says otherwise

// parse the message
	MSG_BeginReading ();

	while (1)
	{
		if (msg_badread)
			Host_Error ("CL_ParseServerMessage: Bad server message");

		cmd = MSG_ReadByte ();

		if (cmd == -1)
		{
			SHOWNET("END OF MESSAGE");
			return;		// end of message
		}

	// if the high bit of the command byte is set, it is a fast update
		if (cmd & U_SIGNAL) //johnfitz -- was 128, changed for clarity
		{
			SHOWNET("fast update");
			CL_ParseUpdate (cmd & 127);
			continue;
		}

		SHOWNET(svc_strings[cmd]);

	// other commands
		switch (cmd)
		{
		default:
#if FITZQUAKE_PROTOCOL
			Host_Error ("CL_ParseServerMessage: Illegible server message, previous was %s\n", svc_strings[lastcmd]); //johnfitz -- added svc_strings[lastcmd]
#else
			Host_Error ("CL_ParseServerMessage: Illegible server message\n");
#endif
			break;

		case svc_nop:
//			Con_Printf ("svc_nop\n");
			break;

		case svc_time:
			cl.mtime[1] = cl.mtime[0];
			cl.mtime[0] = MSG_ReadFloat ();
			break;

		case svc_clientdata:
#if FITZQUAKE_PROTOCOL
			CL_ParseClientdata (); //johnfitz -- removed bits parameter, we will read this inside CL_ParseClientdata()
#else
			i = MSG_ReadShort ();
			CL_ParseClientdata (i);
#endif
			break;

		case svc_version:
			i = MSG_ReadLong ();
#if FITZQUAKE_PROTOCOL
			//johnfitz -- support multiple protocols
			if (i != PROTOCOL_NETQUAKE && i != PROTOCOL_FITZQUAKE)
				Host_Error ("Server returned version %i, not %i or %i\n", i, PROTOCOL_NETQUAKE, PROTOCOL_FITZQUAKE);
			cl.protocol = i;
			//johnfitz
#else
			if (i != PROTOCOL_NETQUAKE)
				Host_Error ("CL_ParseServerMessage: Server is protocol %i instead of %i\n", i, PROTOCOL_NETQUAKE);
#endif
			break;

		case svc_disconnect:
			Host_EndGame ("Server disconnected\n");

		case svc_print:
			// JPG - check to see if the message contains useful information
			str = MSG_ReadString();
			CL_ParseProQuakeString(str);
			Con_Printf ("%s", str);

			CHECKDRAWSTATS;

			break;

		case svc_centerprint:
			str = MSG_ReadString ();
//			Con_Printf("Size of string is %i\n", strlen(str));
			if (!scr_centerprint_nodraw.integer)
			{
				SCR_CenterPrint (str);
			}
			Con_LogCenterPrint (str);//johnfitz -- log centerprints to console
			break;

		case svc_stufftext:

			// JPG - check for ProQuake message
			if (MSG_PeekByte() == MOD_PROQUAKE)
				CL_ParseProQuakeMessage();
			// Still want to add text, even on ProQuake messages.  This guarantees compatibility;
			// unrecognized messages will essentially be ignored but there will be no parse errors

			if (developer_show_stufftext.integer)
			{
				// Baker: Unfortunately, this developer cvar interferes with the normal operation of Quake.  Sniff.
				// Might be the Con_Printf.
				char	*command_string;
				int		command_string_end;
				char	lastchar;
				command_string = MSG_ReadString ();

				// To avoid \n in the console print, substitute it ...
				command_string_end = strlen (command_string) - 1;

				if ((lastchar = command_string[command_string_end]) == '\n')
					command_string[command_string_end] = 0;

				Con_Printf ("Stufftext: '%s'\n", command_string);

				// ... then restore it
				if ((lastchar = command_string[command_string_end]) == '\n')
					command_string[command_string_end] = lastchar;

				Cbuf_AddText (command_string);
			}
			else
				Cbuf_AddText (MSG_ReadString ());
			break;

		case svc_damage:
			CL_ParseDamage ();
			CHECKDRAWSTATS;
			break;

		case svc_serverinfo:
			CL_ParseServerInfo ();
			vid.recalc_refdef = true;	// leave intermission full screen
			break;

		case svc_setangle: // JPG - added mviewangles for smooth chasecam, set last_angle_time
			for (i=0 ; i<3 ; i++)
				cl.viewangles[i] = MSG_ReadAngle ();

			if (!cls.demoplayback)
			{
				VectorCopy (cl.mviewangles[0], cl.mviewangles[1]);

				// JPG - hack with last_angle_time to autodetect continuous svc_setangles
				if (cl.last_angle_time > host_time - 0.3)
					cl.last_angle_time = host_time + 0.3;
				else if (cl.last_angle_time > host_time - 0.6)
					cl.last_angle_time = host_time;
				else
					cl.last_angle_time = host_time - 0.3;

				for (i=0 ; i<3 ; i++)
					cl.mviewangles[0][i] = cl.viewangles[i];
			}
			break;

		case svc_setview:
			cl.player_point_of_view_entity = MSG_ReadShort ();
			break;

		case svc_lightstyle:
			i = MSG_ReadByte ();
			if (i >= MAX_LIGHTSTYLES)
				Sys_Error ("svc_lightstyle > MAX_LIGHTSTYLES");
			strcpy (cl_lightstyle[i].map, MSG_ReadString());
			cl_lightstyle[i].length = strlen(cl_lightstyle[i].map);
			break;

		case svc_sound:
			CL_ParseStartSoundPacket ();
			break;

		case svc_stopsound:
			i = MSG_ReadShort ();
			S_StopSound (i >> 3, i & 7);  // Channel is bottom 3 bits
			break;

		case svc_updatename:
			Sbar_Changed ();
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatename > MAX_SCOREBOARD");
			StringLCopy (cl.scores[i].name, MSG_ReadString());
			break;

		case svc_updatefrags:
			Sbar_Changed ();
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatefrags > MAX_SCOREBOARD");
			cl.scores[i].frags = MSG_ReadShort ();
			break;

		case svc_updatecolors:
			Sbar_Changed ();
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatecolors > MAX_SCOREBOARD");
			cl.scores[i].colors = MSG_ReadByte ();
			CL_NewTranslation (i);
			break;

		case svc_particle:
			R_ParseParticleEffect ();
			break;

		case svc_spawnbaseline:
			i = MSG_ReadShort ();
			// must use CL_EntityNum() to force cl.num_entities up
#if FITZQUAKE_PROTOCOL
			CL_ParseBaseline (CL_EntityNum(i), 1); // johnfitz -- added second parameter
#else
			CL_ParseBaseline (CL_EntityNum(i));
#endif
			break;

		case svc_spawnstatic:
#if FITZQUAKE_PROTOCOL
			CL_ParseStatic (1); //johnfitz -- added parameter
#else
			CL_ParseStatic ();
#endif
			break;

		case svc_temp_entity:
			CL_ParseTEnt ();
			break;

		case svc_setpause:
			if ((cl.paused = MSG_ReadByte()))
				MP3Audio_Pause ();
			else
				MP3Audio_Resume ();
			break;

		case svc_signonnum:
			i = MSG_ReadByte ();
			if (i <= cls.signon)
				Host_Error ("Received signon %i when at %i", i, cls.signon);
			cls.signon = i;
#if FITZQUAKE_PROTOCOL
			//johnfitz -- if signonnum==2, signon packet has been fully parsed, so check for excessive static ents and efrags
			if (i == 2)
			{
//				if (cl.num_statics > 128)
//					Con_Warning ("%i static entities exceeds standard limit of 128.\n", cl.num_statics);
				if (cl.num_statics > MAX_WINQUAKE_STATIC_ENTITIES)
					Con_Warning ("%i static entities exceeds standard limit of %i.\n", cl.num_statics, MAX_WINQUAKE_STATIC_ENTITIES);

				Efrags_CheckEfrags ();
			}
			//johnfitz
#endif
			CL_SignonReply ();
			break;

		case svc_killedmonster:
			if (cls.demoplayback && demorewind.integer)
				cl.stats[STAT_MONSTERS]--;
			else
				cl.stats[STAT_MONSTERS]++;
			CHECKDRAWSTATS;
			break;

		case svc_foundsecret:
			if (cls.demoplayback && demorewind.integer)
				cl.stats[STAT_SECRETS]--;
			else
				cl.stats[STAT_SECRETS]++;
			CHECKDRAWSTATS;
			break;

		case svc_updatestat:
			i = MSG_ReadByte ();
			if (i < 0 || i >= MAX_CL_STATS)
				Sys_Error ("svc_updatestat: %i is invalid", i);
			cl.stats[i] = MSG_ReadLong ();
			break;

		case svc_spawnstaticsound:
#if FITZQUAKE_PROTOCOL
			CL_ParseStaticSound (1); //johnfitz -- added parameter
#else
			CL_ParseStaticSound ();
#endif
			break;

		case svc_cdtrack:
			cl.cdtrack = MSG_ReadByte ();
			cl.looptrack = MSG_ReadByte ();
			if ((cls.demoplayback || cls.demorecording) && (cls.forcetrack != -1))
				MP3Audio_Play ((byte)cls.forcetrack, true);
			else
				MP3Audio_Play ((byte)cl.cdtrack, true);
			break;

		case svc_intermission:
			cl.intermission = 1;
//			cl.completed_time = cl.time;
			// intermission bugfix -- by joe
			cl.completed_time = cl.mtime[0];
			vid.recalc_refdef = true;	// go to full screen
			break;

		case svc_finale:
			cl.intermission = 2;
			cl.completed_time = cl.time;
			vid.recalc_refdef = true;	// go to full screen
			//johnfitz -- log centerprints to console
			str = MSG_ReadString ();
			SCR_CenterPrint (str);
			Con_LogCenterPrint (str);
			//johnfitz
			break;

		case svc_cutscene:
			cl.intermission = 3;
			cl.completed_time = cl.time;
			vid.recalc_refdef = true;	// go to full screen
			//johnfitz -- log centerprints to console
			str = MSG_ReadString ();
			SCR_CenterPrint (str);
			Con_LogCenterPrint (str);
			//johnfitz
			break;

		case svc_sellscreen:
			Cmd_ExecuteString ("help", src_command);
			break;

#ifdef SUPPORTS_NEHAHRA
		// nehahra support
        case svc_hidelmp:
			SHOWLMP_decodehide ();
			break;

        case svc_showlmp:
			SHOWLMP_decodeshow ();
			break;
#endif
       	case svc_skybox:
			Cvar_SetStringByRef (&r_skybox, MSG_ReadString());
			break;

		case svc_bf:
			Cmd_ExecuteString ("bf", src_command);
			break;

		case svc_fog:
			Fog_ParseServerMessage ();
			break;

#if FITZQUAKE_PROTOCOL
		case svc_spawnbaseline2: //PROTOCOL_FITZQUAKE
			i = MSG_ReadShort ();
			// must use CL_EntityNum() to force cl.num_entities up
			CL_ParseBaseline (CL_EntityNum(i), 2);
			break;

		case svc_spawnstatic2: //PROTOCOL_FITZQUAKE
			CL_ParseStatic (2);
			break;

		case svc_spawnstaticsound2: //PROTOCOL_FITZQUAKE
			CL_ParseStaticSound (2);
			break;
		//johnfitz
#endif
		}
#if FITZQUAKE_PROTOCOL
		lastcmd = cmd; //johnfitz
#endif
	}
}
