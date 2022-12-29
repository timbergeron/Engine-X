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

#ifndef __MODEL_H__
#define __MODEL_H__



#include "model_brush.h"
#include "model_alias.h"
#include "model_sprite.h"
#include "model_md3.h"

/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/


//===================================================================

// Whole model

typedef enum {
	mod_brush,
	mod_sprite,
	mod_alias,
	mod_md3,
	mod_spr32
} modtype_t;

// some models are special
typedef enum {
	MOD_NORMAL, // 0
	MOD_PLAYER, // 1
	MOD_EYES,   // 2
	MOD_FLAME,  // 3
	MOD_THUNDERBOLT,  // 4
	MOD_WEAPON,       // 5
	MOD_LAVABALL,     // 6
	MOD_SPIKE,        // 7
	MOD_SHAMBLER,     // 8
	MOD_SPR,          // 9
	MOD_SPR32         // 10
} modhint_t;

#define	EF_ROCKET				1			// leave a trail
#define	EF_GRENADE				2			// leave a trail
#define	EF_GIB					4			// leave a trail
#define	EF_ROTATE				8			// rotate (bonus items)
#define	EF_TRACER				16			// green split trail
#define	EF_ZOMGIB				32			// small blood trail
#define	EF_TRACER2				64			// orange split trail + rotate
#define	EF_TRACER3				128			// purple trail
#define	EF_Q3TRANS				256			// Q3 model containing transparent surface(s)

#if FITZQUAKE_PROTOCOL
//johnfitz -- extra flags for rendering
#define MOD_FORCEDRENDERFENCE   512			// Some mods can end up on the forced listed, changing a listing cvar cannot change this flag
#define	MOD_NOLERP				1024		//don't lerp when animating
#define	MOD_NOSHADOW			2048		//don't cast a shadow
#define	MOD_FBRIGHTHACK			4096		//when fullbrights are disabled, use a hack to render this model brighter

#define	MOD_RENDERADDITIVE		8192		//when fullbrights are disabled, use a hack to render this model brighter
#define	MOD_RENDERFILTER		16384		//when fullbrights are disabled, use a hack to render this model brighter
#define	MOD_RENDERFENCE			32768		// 255 is transparent color, use GL_ALPHA_TEST



//johnfitz
#endif

typedef struct model_s
{
	char			name[MAX_QPATH];
	qbool			needload;	// bmodels and sprites don't cache normally

	modhint_t		modhint;
	modtype_t		modelformat;
	int				numframes;
	synctype_t		synctype;

	int				modelflags;

// volume occupied by the model graphics

	vec3_t			mins, maxs;
#ifdef XFITZQUAKE_PROTOCOL
	vec3_t			ymins, ymaxs; //johnfitz -- bounds for entities with nonzero yaw
	vec3_t			rmins, rmaxs; //johnfitz -- bounds for entities with nonzero pitch or roll
	//johnfitz -- removed float radius;
#else
	float			radius;
#endif

// brush model
	int				firstmodelsurface, nummodelsurfaces;

	int				numsubmodels;
	dmodel_t		*submodels;

	int				numplanes;
	mplane_t		*planes;

	int				numleafs;	// number of visible leafs, not counting 0
	mleaf_t			*leafs;

	int				numvertexes;
	mvertex_t		*vertexes;

	int				numedges;
	medge_t			*edges;

	int				numnodes;
	mnode_t			*nodes;

	int				numtexinfo;
	mtexinfo_t		*texinfo;

	int				numsurfaces;
	msurface_t		*surfaces;

	int				numsurfedges;
	int				*surfedges;

	int				numclipnodes;
#if FITZQUAKE_PROTOCOL
	mclipnode_t		*clipnodes; //johnfitz -- was dclipnode_t
#else
	dclipnode_t		*clipnodes;
#endif

	int				nummarksurfaces;
	msurface_t		**marksurfaces;

	hull_t			hulls[MAX_MAP_HULLS];

	int				numtextures;
	texture_t		**textures;

	byte			*visdata;
	byte			*lightdata;
	char			*entities;

	int				bspversion;
	qbool			isworldmodel;

// additional model data
	cache_user_t	cache;		// only access through Mod_Extradata

	qfs_loadinfo_t	loadinfo;
} model_t;

//============================================================================




void Mod_Init (void);
void Mod_ClearAll (const int clear_level);
model_t *Mod_ForName (const char *name, qbool crash);
void *Mod_Extradata (model_t *mod);	// handles caching
void Mod_TouchModel (const char *name);



// Loading globals ... model.c
extern	model_t	*loadmodel;
extern	char loadname[32];	// for hunk tags


#include "model_brush_funcs.h" // Not related to loading, has to be included AFTER model_t is defined.



// texture_funcs.c
#include "texture_funcs.h"


// light funcs

#define NUMVERTEXNORMALS	162



// Baker: Is hack .. sniff.
#define	ISTELETEX(name)	 ((name)[0] == '*' && (name)[1] == 't' && (name)[2] == 'e' && (name)[3] == 'l' && (name)[4] == 'e')

// RQuake teleporter texture
#define	ISTELETEX2(name) ((name)[0] == '*' && (name)[1] == 'g' && (name)[2] == 'i' && (name)[3] == 'l' && (name)[4] == 't' && (name)[5] == 'e' && (name)[6] == 'l'  && (name)[7] == 'e')

#define IS_LIGHTMAP_ONLY				r_lightmap.integer == 1
#define IS_FULL_LIGHT					(r_lightmap.integer == 2 || !cl.worldmodel->lightdata)
#define IS_FULL_LIGHT_MODELS			(r_lightmap.integer == 3 || IS_FULL_LIGHT)


#endif // __MODEL_H__


