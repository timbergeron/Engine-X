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
// cl_parse_update.c -- parses an entity update from the server

#include "quakedef.h"


extern qbool warn_about_nehahra_protocol;
entity_t *CL_EntityNum (int num);

// Baker: We might be good.  You need to test monster movement to know.
/*
==================
CL_ParseUpdate

Parse an entity update message from the server
If an entities model or origin changes from frame to frame, it must be
relinked. Other attributes can change without relinking.
==================
*/
static int	bitcounts[16];

void CL_ParseUpdate (int bits)
{
	int			i, num;
	model_t		*model;
//#if FITZQUAKE_PROTOCOL
	int			modnum;
//#endif
	qbool	forcelink;
	entity_t	*ent;
#ifdef GL_QUAKE_SKIN_METHOD
	int		skin;
#endif

	if (cls.signon == SIGNONS - 1)
	{	// first update is the final signon stage
		cls.signon = SIGNONS;
		CL_SignonReply ();
	}

	if (bits & U_MOREBITS)
#if FITZQUAKE_PROTOCOL
	{
		i = MSG_ReadByte ();
		bits |= (i<<8);
	}
#else
		bits |= (MSG_ReadByte() << 8);
#endif

#if FITZQUAKE_PROTOCOL
	//johnfitz -- PROTOCOL_FITZQUAKE
	if (cl.protocol == PROTOCOL_FITZQUAKE)
	{
		if (bits & U_EXTEND1)
			bits |= MSG_ReadByte() << 16;
		if (bits & U_EXTEND2)
			bits |= MSG_ReadByte() << 24;
	}
	//johnfitz
#endif

	if (bits & U_LONGENTITY)
		num = MSG_ReadShort ();
	else
		num = MSG_ReadByte ();

	ent = CL_EntityNum (num);

	for (i=0 ; i<16 ; i++)
		if (bits & (1 << i))
			bitcounts[i]++;

	if (ent->msgtime != cl.mtime[1])
		forcelink = true;	// no previous frame to lerp from
	else
		forcelink = false;

#if FITZQUAKE_PROTOCOL
	//johnfitz -- lerping
	if (ent->msgtime + 0.2 < cl.mtime[0]) //more than 0.2 seconds since the last message (most entities think every 0.1 sec)
	{
		// DATAEVENT_LAG so reset animation
		ent->lerpflags |= LERP_RESETANIM; //if we missed a think, we'd be lerping from the wrong frame
// Baker: FitzQuake interpolation LERP reset
	}
	//johnfitz
#endif

	ent->msgtime = cl.mtime[0];

	// Parse the model index
	if (bits & U_MODEL)
	{
		ent->modelindex = modnum = MSG_ReadByte ();
		if (ent->modelindex >= MAX_MODELS)
			Host_Error ("CL_ParseUpdate: bad modelindex");
	}
	else
	{
		ent->modelindex = modnum = ent->baseline.modelindex;
	}


	// Get the frame
	if (bits & U_FRAME)
		ent->frame = MSG_ReadByte ();
	else
		ent->frame = ent->baseline.frame;

	if (bits & U_COLORMAP)
		i = MSG_ReadByte();
	else
		i = ent->baseline.colormap;

	if (!i)
		ent->colormap = vid.colormap;
	else
	{
		if (i > cl.maxclients)
			Sys_Error ("i >= cl.maxclients");
		ent->colormap = cl.scores[i-1].translations;
	}

	// Parse the skin
#ifdef GL_QUAKE_SKIN_METHOD
	if (bits & U_SKIN)
		skin = MSG_ReadByte();
	else
		skin = ent->baseline.skin;
	if (skin != ent->skinnum)	// it changed
	{
		ent->skinnum = skin;
		if (num > 0 && num <= cl.maxclients) // if it is one of the first 16 entities, color map it
			R_TranslatePlayerSkin (num - 1);
	}
#endif

	// Read the effects
	if (bits & U_EFFECTS)
		ent->effects = MSG_ReadByte();
	else
		ent->effects = ent->baseline.effects;

// shift the known values for interpolation
	VectorCopy (ent->msg_origins[0], ent->msg_origins[1]);
	VectorCopy (ent->msg_angles[0], ent->msg_angles[1]);

	if (bits & U_ORIGIN1)
		ent->msg_origins[0][0] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][0] = ent->baseline.origin[0];

	if (bits & U_ANGLE1)
		ent->msg_angles[0][0] = MSG_ReadAngle();
	else
		ent->msg_angles[0][0] = ent->baseline.angles[0];

	if (bits & U_ORIGIN2)
		ent->msg_origins[0][1] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][1] = ent->baseline.origin[1];

	if (bits & U_ANGLE2)
		ent->msg_angles[0][1] = MSG_ReadAngle();
	else
		ent->msg_angles[0][1] = ent->baseline.angles[1];

	if (bits & U_ORIGIN3)
		ent->msg_origins[0][2] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][2] = ent->baseline.origin[2];

	if (bits & U_ANGLE3)
		ent->msg_angles[0][2] = MSG_ReadAngle();
	else
		ent->msg_angles[0][2] = ent->baseline.angles[2];

#if 1 // FITZQUAKE_PROTOCOL
#if 0 // OLD iNTERPOLATE
	if (cl.protocol == PROTOCOL_FITZQUAKE)
	{
#endif 
		//johnfitz -- lerping for movetype_step entities
		if (bits & U_STEP)
		{
			ent->lerpflags |= LERP_MOVESTEP;
			ent->forcelink = true;
		}
		else
			ent->lerpflags &= ~LERP_MOVESTEP;
	//johnfitz
#endif
#if 0 // OLD INTERPOLATE
	}
// Baker: since we aren't supporting the LERP flags client-side yet
//	else
//#endif
	{
		// Baker: NETQUAKE: handle this awkwardness
		if (bits & U_STEP)
		{
			if (!cl_gameplayhack_monster_lerp.integer)
				ent->forcelink = true;
			else
				ent->forcelink = (sv.active == true); // Baker: single player behavior varies from client-only behavior
		}
	}
#endif

#if FITZQUAKE_PROTOCOL
	//johnfitz -- PROTOCOL_FITZQUAKE and PROTOCOL_NEHAHRA
	if (cl.protocol == PROTOCOL_FITZQUAKE)
	{
		if (bits & U_ALPHA)
			ent->alpha = MSG_ReadByte();
		else
			ent->alpha = ent->baseline.alpha;
		if (bits & U_FRAME2)
			ent->frame = (ent->frame & 0x00FF) | (MSG_ReadByte() << 8);
		if (bits & U_MODEL2)
			ent->modelindex = modnum = (modnum & 0x00FF) | (MSG_ReadByte() << 8);
		if (bits & U_LERPFINISH)
		{
			ent->lerpfinish = ent->msgtime + ((float)(MSG_ReadByte()) / 255);
			ent->lerpflags |= LERP_FINISH;
		}
		else
			ent->lerpflags &= ~LERP_FINISH;
	}
	else if (cl.protocol == PROTOCOL_NETQUAKE)
	{
#if FITZQUAKE_PROTOCOL && SUPPORTS_NEHAHRA_PROTOCOL_AS_CLIENT	//	Baker: back2forwards
//HACK: if this bit is set, assume this is PROTOCOL_NEHAHRA
		if (bits & U_TRANS)
		{
			float a,b;

			if (!cl.already_warned_about_nehahra_protocol)
			{
				Con_Warning ("nonstandard update bit, assuming Nehahra protocol\n");
				cl.already_warned_about_nehahra_protocol = true;
			}

			a = MSG_ReadFloat();
			b = MSG_ReadFloat(); //alpha
			if (a == 2)
				MSG_ReadFloat(); //fullbright (not using this yet)
			ent->alpha = ENTALPHA_ENCODE(b);
		}
		else
#endif
			ent->alpha = ent->baseline.alpha;
	}
	//johnfitz
#endif

	//johnfitz -- moved here from above
	model = cl.model_precache[ent->modelindex /*modnum*/];
	if (model != ent->model)
	{
		ent->model = model;
	// automatic animation (torches, etc) can be either all together
	// or randomized
		if (model)
		{
			if (model->synctype == ST_RAND)
				ent->syncbase = (float)(rand()&0x7fff) / 0x7fff;
			else
				ent->syncbase = 0.0;
		}
		else
			forcelink = true;	// hack to make null model players work
#ifdef GL_QUAKE_SKIN_METHOD
		if (num > 0 && num <= cl.maxclients)
			R_TranslatePlayerSkin (num - 1);
#endif
#if FITZQUAKE_PROTOCOL
		// DATAEVENT_NEW_MODEL
		ent->lerpflags |= LERP_RESETANIM; //johnfitz -- don't lerp animation across model changes
		// Baker: FitzQuake interpolation LERP reset
#endif
	}
	//johnfitz

	if (forcelink)
	{	// didn't have an update last message
		VectorCopy (ent->msg_origins[0], ent->msg_origins[1]);
		VectorCopy (ent->msg_origins[0], ent->origin);
		VectorCopy (ent->msg_angles[0], ent->msg_angles[1]);
		VectorCopy (ent->msg_angles[0], ent->angles);
		ent->forcelink = true;
	}
}
