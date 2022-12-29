
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
// insertions_3d.c -- Adds stuff to 3D world

#include "quakedef.h"

//qbool SV_InvisibleToClient(edict_t *viewer, edict_t *seen);
//void R_EmitBox (const vec_rgba_t myColor, const vec3_t centerpoint, const vec3_t xyz_mins, const vec3_t xyz_maxs, const qbool bTopMost, const qbool bLines, const float Expandsize);

qbool SV_InvisibleToClient(edict_t *viewer, edict_t *seen);

void ShowBBoxes_Draw (void)
{
	extern		edict_t *sv_player;
	edict_t		*ed;
	int			i;
	entity_t	*clent;

	// Cycle through the server entities

	for (i=1, ed=NEXT_EDICT(sv.edicts) ; i<sv.num_edicts ; i++, ed=NEXT_EDICT(ed))
	{
		char		*my_classname = PR_GETSTRING(ed->v.classname);
		vec_rgba_t	BoxColor;

		if (ed->free)
			continue;	// Free edict, just skip

		clent = NULL;
		if (i<cl.num_entities)		// Baker: Not all server ents get communicated to client.
			clent = cl_entities + i;


		if (ed == sv_player && !chase_active.integer)
			continue;	// Don't draw in this scenario

		// Obtain Quake specific super-special color
		do
		{
			int special_entity = 0;

			if      (COM_StringMatchCaseless (my_classname, "info_player_start"))		special_entity = 1;
			else if (COM_StringMatchCaseless (my_classname, "info_player_coop"))			special_entity = 2;
			else if (COM_StringMatchCaseless (my_classname, "info_player_deathmatch"))	special_entity = 3;

			switch (special_entity)
			{
			case 1:		VectorSetColor4f (BoxColor, 1, 0, 0, 0.40);	break;
			case 2:		VectorSetColor4f (BoxColor, 1, 1, 0, 0.30);	break;
			case 3:		VectorSetColor4f (BoxColor, 1, 0, 0, 0.40); break;
			}

			// If we already determined a color, exit stage left ...
			if (special_entity)
				continue;

			// normal color selection
			switch ((int)ed->v.solid)
			{
			case SOLID_NOT:      VectorSetColor4f (BoxColor, 1, 1, 1, 0.05);break;
			case SOLID_TRIGGER:  VectorSetColor4f (BoxColor, 1, 0, 1, 0.10);break;
			case SOLID_BBOX:     VectorSetColor4f (BoxColor, 0, 1, 0, 0.10);break;
			case SOLID_SLIDEBOX: VectorSetColor4f (BoxColor, 1, 0, 0, 0.10);break;
			case SOLID_BSP:      VectorSetColor4f (BoxColor, 0, 0, 1, 0.05);break;
			default:             VectorSetColor4f (BoxColor, 0, 0, 0, 0.50);break;
			}

		} while (0);

		// Render box phase

		do
		{
			qbool isPointEntity  = VectorCompare(ed->v.mins, ed->v.maxs); // point entities have 0 size

			if (isPointEntity)
			{
				qbool bRenderTopMost = false;
				qbool bRenderLines   = false;
				float size_adjust = 4.0f; // Give it a little bit of size

				R_EmitBox (BoxColor, ed->v.origin, NULL, NULL, bRenderTopMost, bRenderLines, size_adjust);
			}
			else
			{
				qbool bRenderTopMost = false;
				qbool bRenderLines   = false;
				float size_adjust	  = -0.10f; // Pull size in just a little

				R_EmitBox (BoxColor, ed->v.origin, ed->v.mins, ed->v.maxs, bRenderTopMost, bRenderLines, size_adjust);
			}
		} while (0);

		// Render Caption phase

		do
		{
			extern vec3_t lastbox_mins, lastbox_maxs;	// Last box rendering result
			vec3_t caption_location;

			if (SV_InvisibleToClient (sv_player, ed))
				continue; //don't draw if entity cannot be seen

			// Averaging the vectors gives the center
			LerpVector (lastbox_mins, lastbox_maxs, 0.5, caption_location);

			// Except we want the caption to be at the top
			caption_location[2] = lastbox_maxs[2];

			// Add a couple of height units to make it overhead
			caption_location[2] = caption_location[2] + 20;

			// Depending on cvar value, have caption show different things

			switch (tool_showbboxes.integer)
			{
			default:
			case 1:		R_EmitCaption  (caption_location, my_classname);								break;
			case 2:		R_EmitCaption  (caption_location, ed->v.model ? PR_GETSTRING(ed->v.model) : "No model");					break;
			case 3:		R_EmitCaption  (caption_location, va("ent #%i", i));							break;
			case 4:		R_EmitCaption  (caption_location, va("modelindex %i", (int)ed->v.modelindex));	break;

						// Baker: First ... this isn't how you do it with sprites.
			case 5:		// Client mirror
						R_EmitCaption (caption_location,
							va("\bServer model:\b\n"
							   "'%s'\n"
							   "\bClient model:\b\n"
								"'%s'", ed->v.model ? PR_GETSTRING(ed->v.model) : "No model", clent ? clent->model->name : "No entity"));



						break;


			}
		} while (0);

		// End of this entity
	}  // For Loop closing brace

	// Now do static entities

	for (i=0; i < cl.num_statics; i++)
	{
		vec_rgba_t	BoxColor;

		clent = &cl_static_entities[i];
		if (!(clent->visframe == r_framecount))
			continue; // Not visible this frame

		VectorSetColor4f (BoxColor, 1, 0.5, 0, 0.40);
		// Render box phase

		do
		{
			qbool isPointEntity  = VectorCompare(clent->model->mins, clent->model->maxs); // point entities have 0 size

			if (isPointEntity)
			{
				qbool bRenderTopMost = false;
				qbool bRenderLines   = false;
				float size_adjust = 4.0f; // Give it a little bit of size

				R_EmitBox (BoxColor, clent->currentorigin, NULL, NULL, bRenderTopMost, bRenderLines, size_adjust);
			}
			else
			{
				qbool bRenderTopMost = false;
				qbool bRenderLines   = false;
				float size_adjust	  = -0.10f; // Pull size in just a little

				R_EmitBox (BoxColor, clent->currentorigin, clent->model->mins, clent->model->maxs, bRenderTopMost, bRenderLines, size_adjust);
			}
		} while (0);

		// Render Caption phase

		do
		{
			extern vec3_t lastbox_mins, lastbox_maxs;	// Last box rendering result
			vec3_t caption_location;

			// Averaging the vectors gives the center
			LerpVector (lastbox_mins, lastbox_maxs, 0.5, caption_location);

			// Except we want the caption to be at the top
			caption_location[2] = lastbox_maxs[2];

			// Add a couple of height units to make it overhead
			caption_location[2] = caption_location[2] + 20;

			// Depending on cvar value, have caption show different things

			R_EmitCaption (caption_location, va("\bStatic ent\b %i '%s'\n", clent- cl_static_entities, clent->model->name));

		} while (0);

		// End of this entity
	}  // For Loop closing brace


}


void ShowSpawns_Draw (void)
{
	extern		edict_t *sv_player;
	edict_t		*ed;
	int			i;

	// Cycle through the server entities

	for (i=0, ed=NEXT_EDICT(sv.edicts) ; i<sv.num_edicts ; i++, ed=NEXT_EDICT(ed))
	{


		if (!ed->v.classname)
			continue;				// No classname, can't very well be a spawnpoint ;)

		// Obtain Quake specific super-special color
		do
		{
			vec_rgba_t	BoxColor;
			char		*my_classname = PR_GETSTRING(ed->v.classname);
			float random_color = ((float)((int)(cl.ctime * 10)%10))/10;

			int spawnpoint_type = 0;

			if      (COM_StringMatchCaseless (my_classname, "info_player_start"))		spawnpoint_type = 1;
			else if (COM_StringMatchCaseless (my_classname, "info_player_coop"))			spawnpoint_type = 2;
			else if (COM_StringMatchCaseless (my_classname, "info_player_deathmatch"))	spawnpoint_type = 3;

			if (!spawnpoint_type)	// Not a spawn point
				break;

			// Baker: We could do some stuff here to eliminate spawn points that don't apply

			switch (spawnpoint_type)
			{
			case 1:		VectorSetColor4f (BoxColor, 1-random_color, 0, 0, 0.40);						break;
			case 2:		VectorSetColor4f (BoxColor, 1-random_color, 1-random_color*.5, 0, 0.30);		break;
			case 3:		VectorSetColor4f (BoxColor, 1-random_color, random_color, random_color, 0.40);	break;
			}

			R_EmitBox	   (BoxColor, ed->v.origin, NULL, NULL, true, false, 28);
			R_EmitCaption  (ed->v.origin, my_classname);
		} while (0);
	}
}

// Baker: Insertions aren't part of the natural world
void Insertions_Draw (void)
{
	extern vec3_t	texturepointer_spot;

	// Client only stuff

	if (texturepointer_spot[0])
		R_EmitWirePoint (color_white_f, texturepointer_spot);

	// Client and server dependent stuffs ...

	if (!sv.active)					// Isn't possible
		return;

	if (!tool_showbboxes.integer && !tool_showspawns.integer)		// Isn't on
		return;

	if (tool_showbboxes.integer)
		ShowBBoxes_Draw ();
	else if (tool_showspawns.integer)
		ShowSpawns_Draw ();

	return;
}

