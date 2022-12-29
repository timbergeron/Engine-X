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
// cl_parse_serverinfo.c -- parse a message received from the server
// Baker: Validated 6-27-2011

#include "quakedef.h"

/*
==================
CL_KeepaliveMessage

When the client is taking a long time to load stuff, send keepalive messages
so the server doesn't disconnect.
==================
*/
static void CL_KeepaliveMessage (void)
{
	float		time;
	static	float	lastmsg;
	int		ret;
	sizebuf_t	old;
	byte		olddata[8192];

	if (sv.active)
		return;		// no need if server is local
	if (cls.demoplayback)
		return;

// read messages from server, should just be nops
	old = net_message;
	memcpy (olddata, net_message.data, net_message.cursize);

	do
	{
		ret = CL_GetMessage ();
		switch (ret)
		{
		default:
			Host_Error ("CL_KeepaliveMessage: CL_GetMessage failed");

		case 0:
			break;	// nothing waiting

		case 1:
			Host_Error ("CL_KeepaliveMessage: received a message");
			break;

		case 2:
			if (MSG_ReadByte() != svc_nop)
				Host_Error ("CL_KeepaliveMessage: datagram wasn't a nop");
			break;
		}
	} while (ret);

	net_message = old;
	memcpy (net_message.data, olddata, net_message.cursize);

// check time
	time = Sys_DoubleTime ();
	if (time - lastmsg < 5)
		return;
	lastmsg = time;

// write out a nop
	Con_Printf ("--> client to server keepalive\n");

	MSG_WriteByte (&cls.message, clc_nop);
	NET_SendMessage (cls.netcon, &cls.message);
	SZ_Clear (&cls.message);
}


#if HTTP_DOWNLOAD
/*
   =====================
   CL_WebDownloadProgress
   Callback function for webdownloads.
   Since Web_Get only returns once it's done, we have to do various things here:
   Update download percent, handle input, redraw UI and send net packets.
   =====================
*/
static int CL_WebDownloadProgress( double percent )
{
	static double time, oldtime, newtime;
	// This is more or less a mirror of WinMain loop in sys_main.c that allows engine to keep running as expected during
	// a download

	cls.download.percent = percent;

	CL_KeepaliveMessage();

	newtime = Sys_DoubleTime ();
	time = newtime - oldtime;

	Host_Frame (time);

	oldtime = newtime;

	return cls.download.disconnect; // abort if disconnect received
}
#endif





void CL_NewMap (void)
{


	// Start of new map begins here

	// Baker: This stuff is for the world model.  I guess the map could be reloaded -- which happens if the player dies for instance
	//        so this stuff has to be reset.
	Frustum_NewMap ();
	Particles_NewMap ();
	Efrags_NewMap ();
	// Clear this texturechain out
	// This is done in clear texture chain for model before rendering.  Shouldn't be necessary.
//	{
//		int	i, waterline;
//		for (i=0 ; i<cl.worldmodel->numtextures ; i++)
//		{
//			if (!cl.worldmodel->textures[i])
//				continue;
//			for (waterline=0 ; waterline<2 ; waterline++)
//			{
//	 			cl.worldmodel->textures[i]->texturechain[waterline] = NULL;
//				cl.worldmodel->textures[i]->texturechain_tail[waterline] = &cl.worldmodel->textures[i]->texturechain[waterline];
//			}
//		}
//	}

#if SUPPORTS_TEXTURE_POINTER
	TexturePointer_NewMap ();
#endif
	Light_NewMap ();

	Viewblends_NewMap ();

/*
	D3D_FlushTextures ();
	R_SetLeafContents ();

	D3DSky_ParseWorldSpawn ();
	D3D_ModelSurfsBeginMap ();
	D3DAlpha_NewMap ();
*/
	Fog_NewMap (); //johnfitz -- global fog in worldspawn
//	D3DAlias_CreateBuffers ();
	Sky_SkyBox_NewMap (); //johnfitz -- skybox in worldspawn
	// Baker: I guess we could look for clearcolor here too
	// as sounds are now cleared between maps these sounds also need to be
	// reloaded otherwise the known_sfx will go out of sequence for them
	// (this isn't the case any more but it does no harm)
#pragma message ("Quality assurance: We should do these on a new map.  If gamedir changed, the models may have changed in that gamedir.")
	CL_InitTEnts ();
	S_InitAmbients ();
	LOC_LoadLocations();
//	D3DTexture_RegisterChains ();
/*
	// load nehahra hacky crap
	if (nehahra) R_ParseForNehahra ();
*/


	{
		extern qbool r_loadq3player;
		extern	tagentity_t	q3player_body, q3player_head;
		void CL_CopyPlayerInfo (entity_t *ent, entity_t *player);


	// Q3 Model hack newmap
	// HACK HACK HACK - create two extra entities if drawing the player's multimodel
		if (r_loadq3player)
		{
			memset (&q3player_body, 0, sizeof(tagentity_t));
			CL_CopyPlayerInfo (&q3player_body.ent, &cl_entities[cl.player_point_of_view_entity]);
			memset (&q3player_head, 0, sizeof(tagentity_t));
			CL_CopyPlayerInfo (&q3player_head.ent, &cl_entities[cl.player_point_of_view_entity]);
		}
	}



}


extern qbool gbGameDirChanged ;
/*
==================
CL_ParseServerInfo
==================
*/
void CL_ParseServerInfo (void)
{
	char	*str, tempname[MAX_QPATH];
	int		i, nummodels, numsounds;
	char	model_precache[MAX_MODELS][MAX_QPATH];
	char	sound_precache[MAX_SOUNDS][MAX_QPATH];
	extern void R_PreMapLoad (char *);
#if HTTP_DOWNLOAD
	extern int Web_Get( const char *url, const char *referer, const char *name, int resume, int max_downloading_time, int timeout, int ( *_progress )(double) );
#endif
#ifdef GLQUAKE
	extern qbool r_loadq3player;
#endif

	Con_DevPrintf (DEV_PROTOCOL, "Serverinfo packet received.\n");

// wipe the client_state_t struct
	CL_ClearState ();	// Hunk is cleared here!


#if FITZQUAKE_PROTOCOL
	//johnfitz -- support multiple protocols
	do 
	{
		// parse protocol version number
		i = MSG_ReadLong ();

		if (i != PROTOCOL_NETQUAKE && i != PROTOCOL_FITZQUAKE)
		{
			Con_Printf ("\n"); //becuase there's no newline after serverinfo print
			Host_Error ("Server returned version %i, not %i or %i\n", i, PROTOCOL_NETQUAKE, PROTOCOL_FITZQUAKE);
			return;
		}

		if (!ALLOW_PROTOCOL_666 && i == PROTOCOL_FITZQUAKE)
		{
			Con_Printf ("\n"); //becuase there's no newline after serverinfo print
			Host_Error ("FitzQuake protocol 666 support is disabled in this build\n");
			return;
		}	
		
		cl.protocol = i;

	} while (0);
	//johnfitz
#else
	if (i != PROTOCOL_NETQUAKE)
	{
		Con_Printf ("Server returned version %i, not %i", i, PROTOCOL_NETQUAKE);
		return;
	}
#endif

// parse maxclients
	cl.maxclients = MSG_ReadByte ();
	if (cl.maxclients < 1 || cl.maxclients > MAX_SCOREBOARD)
	{
		Con_Printf ("Bad maxclients (%u) from server\n", cl.maxclients);
		return;
	}
	cl.scores = Hunk_AllocName (cl.maxclients, sizeof(*cl.scores), "scores"); // color!
	cl.teamscores = Hunk_AllocName (14, sizeof(*cl.teamscores), "teamscor"); // JPG - for teamscore status bar.  Baker: note that this makes color 14, 15 use with ProQuake incompatible

// parse gametype
	cl.gametype = MSG_ReadByte ();

// parse signon message
	str = MSG_ReadString ();
	StringLCopy (cl.levelname, str); // StringLCopy should not affect performance here, was an strncpy -1

// separate the printfs so the server message can have a color
	Con_Printf ("\n%s\n", Con_Quakebar(37)); //johnfitz
	Con_Printf ("%c%s\n", 2, str);

#if FITZQUAKE_PROTOCOL
//johnfitz -- tell user which protocol this is
	Con_Printf ("Using protocol %i\n", i);
#endif

// first we go through and touch all of the precache data that still
// happens to be in the cache, so precaching something else doesn't
// needlessly purge it

// precache models
	for (i=0 ; i<NUM_MODELINDEX ; i++)
		cl_modelindex[i] = -1;

	memset (cl.model_precache, 0, sizeof(cl.model_precache));
	for (nummodels=1 ; ; nummodels++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
//		DMSG (va("Model name %i is %s", nummodels, str));
		if (nummodels == MAX_MODELS)
		{
			Con_Printf ("Server sent too many model precaches\n");
			return;
		}

#ifdef GLQUAKE // If the FOR loop right here
		if (gl_loadq3models.floater)
		{
			if (COM_StringMatch (str, cl_modelnames[mi_player]) &&
			    QFS_FindFile_Pak_And_Path("progs/player/head.md3", NULL) &&
			    QFS_FindFile_Pak_And_Path("progs/player/upper.md3", NULL) &&
			    QFS_FindFile_Pak_And_Path("progs/player/lower.md3", NULL))
			{
				strlcpy (tempname, "progs/player/lower.md3", sizeof(tempname));
				str = tempname;
				cl_modelindex[mi_player] = nummodels;
				r_loadq3player = true;
			}
			else
			{
				COM_Copy_StripExtension (str, tempname);
				strcat (tempname, ".md3");

				if (QFS_FindFile_Pak_And_Path(tempname, NULL))
					str = tempname;
			}
		}
#endif
		strlcpy (model_precache[nummodels], str, sizeof(model_precache[nummodels]));

		for (i=0 ; i<NUM_MODELINDEX ; i++)
		{
			if (COM_StringMatch (cl_modelnames[i], model_precache[nummodels]))
			{
				cl_modelindex[i] = nummodels;
				break;
			}
		}

//		if (!gbGameDirChanged)
			Mod_TouchModel (str); // Baker: why is this not here?
	} // End for models
//	DMSG (va("Total models is %i", nummodels));

#if FITZQUAKE_PROTOCOL
	//johnfitz -- check for excessive models
//	if (nummodels >= 256)
//		Con_Warning ("%i models exceeds standard limit of 256.\n", nummodels);

	if (nummodels >= MAX_WINQUAKE_MODELS)
		Con_Warning ("%i models exceeds standard limit of %i.\n", nummodels, MAX_WINQUAKE_MODELS);

	//johnfitz
#endif

#ifdef GLQUAKE // After the for loop
// load the extra "no-flamed-torch" model
// NOTE: this is an ugly hack
	if (nummodels == MAX_MODELS)
	{
		Con_Printf ("Server sent too many model precaches -> replacing flame0.mdl with flame.mdl\n");
		cl_modelindex[mi_flame0] = cl_modelindex[mi_flame1];
	}
	else
	{
		StringLCopy (model_precache[nummodels], cl_modelnames[mi_flame0]);
		cl_modelindex[mi_flame0] = nummodels++;
	}
// load the rest of the q3 player model if possible
	if (r_loadq3player)
	{
		if (nummodels + 1 >= MAX_MODELS)
		{
			Con_Printf ("Server sent too many model precaches -> can't load Q3 player model\n");
			strlcpy (model_precache[cl_modelindex[mi_player]], cl_modelnames[mi_player], sizeof(model_precache[cl_modelindex[mi_player]]));
		}
		else
		{
			strlcpy (model_precache[nummodels], "progs/player/upper.md3", sizeof(model_precache[nummodels]));
			cl_modelindex[mi_q3torso] = nummodels++;
			strlcpy (model_precache[nummodels], "progs/player/head.md3", sizeof(model_precache[nummodels]));
			cl_modelindex[mi_q3head] = nummodels++;
		}
	}
#endif


// precache sounds
	memset (cl.sound_precache, 0, sizeof(cl.sound_precache));
	for (numsounds=1 ; ; numsounds++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		if (numsounds == MAX_SOUNDS)
		{
			Con_Printf ("Server sent too many sound precaches\n");
			return;
		}

		StringLCopy (sound_precache[numsounds], str);
//		if (!gbGameDirChanged)
			S_TouchSound (str);
	}

#if FITZQUAKE_PROTOCOL
	//johnfitz -- check for excessive sounds
//	if (numsounds >= 256)
//		Con_Warning ("%i sounds exceeds standard limit of 256.\n", numsounds);

	if (numsounds >= MAX_WINQUAKE_SOUNDS)
		Con_Warning ("%i sounds exceeds standard limit of %i.\n", numsounds, MAX_WINQUAKE_SOUNDS);

	//johnfitz
#endif

	// We may need to set the host name here.
	if (!sv.active)
		StringLCopy (host_worldname, StringTemp_SkipPathAndExten(model_precache[1]));
	cl.worldname = host_worldname;	// Either we just set this now, or the server set it.

	
	// now we try to load everything else until a cache allocation fails
	for (i=1 ; i<nummodels ; i++)	// model 1 is the bsp
	{
		cl.model_precache[i] = Mod_ForName (model_precache[i], false);
		if (cl.model_precache[i] == NULL)
		{
#if HTTP_DOWNLOAD
			if (!cls.demoplayback && cl_web_download.integer && cl_web_download_url.string)
			{
				char url[1024];
				qbool success = false;
				char 			download_tempname[MAX_OSPATH],
								download_finalname[MAX_OSPATH];
				extern char server_name[MAX_QPATH];
				extern int net_hostport;
				extern	char	com_gamedirfull[MAX_OSPATH];

				//Create the FULL path where the file should be written
				snprintf (download_tempname, sizeof(download_tempname), "%s/%s.tmp", com_gamedirfull, model_precache[i]);
				COM_CreatePath (download_tempname); // COM_CreatePath drills down.  We don't need to waste time purifying the filename to just a path.
				Con_Printf( "Web downloading: %s from %s%s\n", model_precache[i], cl_web_download_url.string, model_precache[i]);

				//assign the url + path + file + extension we want
				snprintf( url, sizeof( url ), "%s%s", cl_web_download_url.string, model_precache[i]);

				cls.download.web = true;
				cls.download.disconnect = false;
				cls.download.percent = 0.0;

				Con_DevPrintf (DEV_PROTOCOL, "Web download is Ending loading plaque\n");
				SCR_EndLoadingPlaque ();

				//let libCURL do it's magic!!
				success = Web_Get(url, NULL, download_tempname, false, 600, 30, CL_WebDownloadProgress);

				cls.download.web = false;


				if (success)
				{
					Con_Printf("Web download successful: %s\n", download_tempname);
					//Rename the .tmp file to the final precache filename
					snprintf (download_finalname, sizeof(download_finalname), "%s/%s", com_gamedirfull, model_precache[i]);
					rename (download_tempname, download_finalname);


					Cbuf_AddText (va("connect %s:%u\n",server_name,net_hostport));//reconnect after each success
					return;
				}
				else
				{
					remove (download_tempname);
					Con_Printf( "Web download of %s failed\n", download_tempname );
					return;
				}


				if( cls.download.disconnect )//if the user type disconnect in the middle of the download
				{
					cls.download.disconnect = false;
					CL_Disconnect_f();
					return;
				}
			}
			else
#endif
			{
				Con_Printf("Model %s not found\n", model_precache[i]);
				//don't disconnect, let them sit in console and ask for help.
				//Host_EndGame ("Server disconnected\n");
	//			if (i==1)
	//				Con_Printf("Client download map!\n");
				return;
			}
		}
		CL_KeepaliveMessage ();
	}

	S_BeginPrecaching ();
	for (i=1 ; i<numsounds ; i++)
	{
		cl.sound_precache[i] = S_PrecacheSound (sound_precache[i]);
		CL_KeepaliveMessage ();
	}
	S_EndPrecaching ();

// local state
	cl_entities[0].model = cl.worldmodel = cl.model_precache[1];

	CL_NewMap ();


// Fitz clears last center string stuff here
// Quakespasm clears centerprint
// Check what all should be cleared on a new map

	//johnfitz -- clear out string; we don't consider identical
	//messages to be duplicates if the map has changed in between
	//johnfitz

	Hunk_Check ();			// make sure nothing is hurt



// Fitz has dev stats stuffs here
//johnfitz -- reset developer stats
//	memset(&dev_stats, 0, sizeof(dev_stats));
//	memset(&dev_peakstats, 0, sizeof(dev_peakstats));
//	memset(&dev_overflows, 0, sizeof(dev_overflows));

//	if (gbGameDirChanged)
//		gbGameDirChanged = false;
}
