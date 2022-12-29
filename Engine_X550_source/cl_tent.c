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
// cl_tent.c -- client side temporary entities

#include "quakedef.h"

int		num_temp_entities;
entity_t	cl_temp_entities[MAX_TEMP_ENTITIES];
beam_t		cl_beams[MAX_BEAMS];

static	vec3_t	playerbeam_end;
float				ExploColor[3];		// joe: for color mapped explosions
static model_t		*cl_bolt1_mod, *cl_bolt2_mod, *cl_bolt3_mod;

static sfx_t		*cl_sfx_wizhit;
static sfx_t		*cl_sfx_knighthit;
static sfx_t		*cl_sfx_tink1;
static sfx_t		*cl_sfx_ric1;
static sfx_t		*cl_sfx_ric2;
static sfx_t		*cl_sfx_ric3;
static sfx_t		*cl_sfx_r_exp3;



/*
=================
CL_InitTEnts
=================
*/
void CL_InitTEnts (void)
{
	cl_sfx_wizhit = S_PrecacheSound ("wizard/hit.wav");
	cl_sfx_knighthit = S_PrecacheSound ("hknight/hit.wav");
	cl_sfx_tink1 = S_PrecacheSound ("weapons/tink1.wav");
	cl_sfx_ric1 = S_PrecacheSound ("weapons/ric1.wav");
	cl_sfx_ric2 = S_PrecacheSound ("weapons/ric2.wav");
	cl_sfx_ric3 = S_PrecacheSound ("weapons/ric3.wav");
	cl_sfx_r_exp3 = S_PrecacheSound ("weapons/r_exp3.wav");
}

/*
=================
CL_ClearTEnts
=================
*/
void CL_ClearTEnts (void)
{
	/*cl_bolt1_mod = cl_bolt2_mod = cl_bolt3_mod = cl_beam_mod = NULL;

	memset (&cl_beams, 0, sizeof(cl_beams)); Baker says: Next time! */
}

/*
=================
CL_ParseBeam
=================
*/
static void CL_ParseBeam (model_t *m)
{
	int	i, ent;
	vec3_t	start, end;
	beam_t	*b;

	ent = MSG_ReadShort ();

	start[0] = MSG_ReadCoord ();
	start[1] = MSG_ReadCoord ();
	start[2] = MSG_ReadCoord ();

	end[0] = MSG_ReadCoord ();
	end[1] = MSG_ReadCoord ();
	end[2] = MSG_ReadCoord ();
#if SUPPORTS_FTESTAINS
	Stain_AddStain(end, 13, 22); //qbism ftestain
#endif

	if (ent == cl.player_point_of_view_entity)
		VectorCopy (end, playerbeam_end);	// for cl_truelightning

// override any beam with the same entity
	for (i = 0, b = cl_beams ; i < MAX_BEAMS ; i++, b++)
	{
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}
	}

// find a free beam
	for (i = 0, b = cl_beams ; i < MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}
	}

	Con_Printf ("beam list overflow!\n");
}

#define	SetCommonExploStuff						\
	dl = Light_AllocDlight (0);					\
	VectorCopy (pos, dl->origin);					\
	dl->radius = 150 + 200 * CLAMP (0, light_explosions.floater, 1);	\
	dl->die = cl.time + 0.5;					\
	dl->decay = 300

/*
=================
CL_ParseTEnt
=================
*/
void CL_ParseTEnt (void)
{
	int		type;
	vec3_t		pos;
	dlight_t	*dl;
	int		rnd;
	int		colorStart, colorLength;
#ifdef GLQUAKE
	byte		*colorByte;
#endif

	type = MSG_ReadByte ();
	switch (type)
	{
	case TE_WIZSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 20, 30);
#if SUPPORTS_FTESTAINS
		Stain_AddStain(pos, 4, 21); //qbism ftestain
#endif

		S_StartSound (-1, 0, cl_sfx_wizhit, pos, 1, 1);
		break;

	case TE_KNIGHTSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 226, 20);
#if SUPPORTS_FTESTAINS
		Stain_AddStain(pos, 4, 25); //qbism ftestain
#endif

		S_StartSound (-1, 0, cl_sfx_knighthit, pos, 1, 1);
		break;

	case TE_SPIKE:				// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
#if SUPPORTS_FTESTAINS
		Stain_AddStain(pos, 5, 20); //qbism ftestain
#endif

	// joe: they put the ventillator's wind effect to "10" in Nehahra. sigh.
#ifdef SUPPORTS_NEHAHRA
		if (nehahra)
			R_RunParticleEffect (pos, vec3_origin, 0, 9);
		else
#endif
			R_RunParticleEffect (pos, vec3_origin, 0, 10);

		if (rand() % 5)
		{
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		}
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;

	case TE_SUPERSPIKE:			// super spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 0, 20);
#if SUPPORTS_FTESTAINS
		Stain_AddStain(pos, 6, 20); //qbism ftestain
#endif


		if (rand() % 5)
		{
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		}
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;

	case TE_GUNSHOT:			// bullet hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
#if SUPPORTS_FTESTAINS
		Stain_AddStain(pos, 8, 22); //qbism ftestain
#endif

		if(!game_kurok.integer)
			R_RunParticleEffect (pos, vec3_origin, 0, 21);
		break;

	case TE_EXPLOSION:			// rocket explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
#if SUPPORTS_FTESTAINS
		Stain_AddStain(pos, 8, 45); //qbism ftestain
#endif

//		if (game_kurok.integer)
//		{
  //      	ExploColor[0] = MSG_ReadCoord ();
//			ExploColor[1] = MSG_ReadCoord ();
//			ExploColor[2] = MSG_ReadCoord ();
//		}

		if (particle_explosiontype.integer == 3)
			R_RunParticleEffect (pos, vec3_origin, 225, 50);
		else
			R_ParticleExplosion (pos);

		if (light_explosions.floater)
		{
			SetCommonExploStuff;
#ifdef GLQUAKE
			if (game_kurok.integer)
				dl->color_type = lt_explosion3;
			else
				dl->color_type = Light_SetDlightColor (light_explosions_color.integer, lt_explosion, true);
#endif
		}

		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;


	case TE_TAREXPLOSION:			// tarbaby explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
#if SUPPORTS_FTESTAINS
		Stain_AddStain(pos, 8, 60); //qbism ftestain
#endif

//		if (game_kurok.integer)
//		{
  //      	ExploColor[0] = MSG_ReadCoord ();
//			ExploColor[1] = MSG_ReadCoord ();
//			ExploColor[2] = MSG_ReadCoord ();
//		}

		R_BlobExplosion (pos);
// Kurok 2.3
		if (game_kurok.integer && light_explosions.floater)
		{
			SetCommonExploStuff;

			if (game_kurok.integer)
				dl->color_type = lt_explosion3;
		}
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

	case TE_LIGHTNING1:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt.mdl", true));
		break;

	case TE_LIGHTNING2:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt2.mdl", true));
		break;

	case TE_LIGHTNING3:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt3.mdl", true));
		break;

#ifdef SUPPORTS_NEHAHRA
	// nehahra support
        case TE_LIGHTNING4:                             // lightning bolts
                CL_ParseBeam (Mod_ForName(MSG_ReadString(), true));
		break;
#endif

// PGM 01/21/97
	case TE_BEAM:				// grappling hook beam
		CL_ParseBeam (Mod_ForName("progs/beam.mdl", true));
		break;
// PGM 01/21/97

	case TE_LAVASPLASH:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_LavaSplash (pos);
		break;

	case TE_TELEPORT:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_TeleportSplash (pos);
		break;

	case TE_EXPLOSION2:			// color mapped explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		colorStart = MSG_ReadByte ();
		colorLength = MSG_ReadByte ();

		if (particle_explosiontype.integer == 3)
			R_RunParticleEffect (pos, vec3_origin, 225, 50);
		else
			R_ColorMappedExplosion (pos, colorStart, colorLength);

		if (light_explosions.floater)
		{
			SetCommonExploStuff;
#ifdef GLQUAKE
			colorByte = (byte *)&d_8to24table[colorStart];
			ExploColor[0] = ((float)colorByte[0]) / (2.0 * 255.0);
			ExploColor[1] = ((float)colorByte[1]) / (2.0 * 255.0);
			ExploColor[2] = ((float)colorByte[2]) / (2.0 * 255.0);
			dl->color_type = lt_explosion2;
#endif
		}
#if SUPPORTS_FTESTAINS
		Stain_AddStain(pos, 8, 50); //qbism ftestain
#endif

		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

#ifdef SUPPORTS_NEHAHRA
	// nehahra support
	case TE_EXPLOSION3:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		ExploColor[0] = MSG_ReadCoord () / 2.0;
		ExploColor[1] = MSG_ReadCoord () / 2.0;
		ExploColor[2] = MSG_ReadCoord () / 2.0;

		if (particle_explosiontype.integer == 3)
			R_RunParticleEffect (pos, vec3_origin, 225, 50);
		else
			R_ParticleExplosion (pos);
#if SUPPORTS_FTESTAINS
		Stain_AddStain(pos, 8, 50); //qbism ftestain
#endif

		if (light_explosions.floater)
		{
			SetCommonExploStuff;
#ifdef GLQUAKE
			dl->color_type = lt_explosion3;
#endif
		}

		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;
#endif

	default:
		Sys_Error ("CL_ParseTEnt: bad type");
	}
}

/*
=================
CL_NewTempEntity
=================
*/
static entity_t *CL_NewTempEntity (void)
{
	entity_t	*ent;

	if (cl_numvisedicts == MAX_VISEDICTS)
		return NULL;

	if (num_temp_entities == MAX_TEMP_ENTITIES)
		return NULL;

	ent = &cl_temp_entities[num_temp_entities];
	memset (ent, 0, sizeof(*ent));
	num_temp_entities++;
	cl_visedicts[cl_numvisedicts] = ent;
	cl_numvisedicts++;

	ent->colormap = vid.colormap;
	return ent;
}

/*
=================
CL_UpdateTEnts
=================
*/
void CL_UpdateTEnts (void)
{
	int		i;
	beam_t		*b;
	vec3_t		dist, org;
	float		d;
	entity_t	*ent;
	float		yaw, pitch;
	float		forward;
#ifdef SUPPORTS_QMB
	int		j;
	vec3_t		beamstart, beamend;
	qbool	sparks = false;
	extern	void QMB_Lightning_Splash (vec3_t org);
#endif

	num_temp_entities = 0;

	srand ((int) (cl.time * 1000)); //johnfitz -- freeze beams when paused

	// update lightning
	for (i = 0, b = cl_beams ; i < MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
			continue;

		// if coming from the player, update the start position
		if (b->entity == cl.player_point_of_view_entity)
		{
			VectorCopy (cl_entities[cl.player_point_of_view_entity].origin, b->start);
			// joe: using koval's [sons]Quake code
			b->start[2] += cl.crouch;
			if (cl_ent_truelightning.integer)
			{
				vec3_t	forward, v, org, ang;
				float	f, delta;
				trace_t	trace;

				f = max(0, min(1, cl_ent_truelightning.integer));

				VectorSubtract (playerbeam_end, cl_entities[cl.player_point_of_view_entity].origin, v);
				v[2] -= 22;		// adjust for view height
				vectoangles (v, ang);

				// lerp pitch
				ang[0] = -ang[0];
				if (ang[0] < -180)
					ang[0] += 360;
				ang[0] += (cl.viewangles[0] - ang[0]) * f;

				// lerp yaw
				delta = cl.viewangles[1] - ang[1];
				if (delta > 180)
					delta -= 360;
				if (delta < -180)
					delta += 360;
				ang[1] += delta * f;
				ang[2] = 0;

				AngleVectors (ang, forward, NULL, NULL);
				VectorScale (forward, 600, forward);
				VectorCopy (cl_entities[cl.player_point_of_view_entity].origin, org);
				org[2] += 16;
				VectorAdd(org, forward, b->end);

				memset (&trace, 0, sizeof(trace_t));
				if (!SV_RecursiveHullCheck(cl.worldmodel->hulls, 0, 0, 1, org, b->end, &trace))
					VectorCopy (trace.endpos, b->end);
			}
		}

		// calculate pitch and yaw
		VectorSubtract (b->end, b->start, dist);

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = atan2f (dist[1], dist[0]) * 180 / M_PI;
			if (yaw < 0)
				yaw += 360;

			forward = sqrtf (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = atan2f (dist[2], forward) * 180 / M_PI;
			if (pitch < 0)
				pitch += 360;
		}

		// add new entities for the lightning
		VectorCopy (b->start, org);
#ifdef SUPPORTS_QMB
		VectorCopy (b->start, beamstart);
#endif
		d = VectorNormalize (dist);
		VectorScale (dist, 30, dist);

		for ( ; d > 0 ; d -= 30)
		{
#ifdef SUPPORTS_QMB
			if (qmb_initialized && qmb_lightning.integer)
			{
				VectorAdd(org, dist, beamend);
				for (j=0 ; j<3 ; j++)
					beamend[j] += (b->entity != cl.player_point_of_view_entity) ? (rand() % 40) - 20 : (rand() % 16) - 8;
				QMB_LightningBeam (beamstart, beamend);
				VectorCopy (beamend, beamstart);
			}
			else
#endif
			{
				ent = CL_NewTempEntity();
				if (!ent)
					return;
				VectorCopy (org, ent->origin);
				ent->model = b->model;
				ent->angles[0] = pitch;
				ent->angles[1] = yaw;
				ent->angles[2] = rand() % 360;
			}

//#ifdef GLQUAKE
#if 0
			if (qmb_initialized && qmb_lightning.integer && !sparks)
			{
				trace_t	trace;

				memset (&trace, 0, sizeof(trace_t));
				if (!SV_RecursiveHullCheck(cl.worldmodel->hulls, 0, 0, 1, org, beamend, &trace))
				{
					byte	col[3] = {60, 100, 240};

					QMB_GenSparks (trace.endpos, col, 3, 300, 0.25);
				//	QMB_Lightning_Splash (trace.endpos);
					sparks = true;
				}
			}
#endif
			VectorAdd(org, dist, org);
		}
	}
}
