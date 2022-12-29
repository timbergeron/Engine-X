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

#ifndef __MODEL_ALIAS_H__
#define __MODEL_ALIAS_H__

#include "model_alias_gen.h"


/*
==============================================================================

ALIAS MODELS

Alias models are position independent, so the cache manager can move them.
==============================================================================
*/

typedef struct
{
	int			firstpose;
	int			numposes;
	float		interval;
	trivertx_t	bboxmin;
	trivertx_t	bboxmax;
	int			frame;
	char		name[16];
} maliasframedesc_t;

typedef struct
{
	trivertx_t	bboxmin;
	trivertx_t	bboxmax;
	int			frame;
} maliasgroupframedesc_t;

typedef struct
{
	int			numframes;
	int			intervals;
	maliasgroupframedesc_t	frames[1];
} maliasgroup_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct mtriangle_s {
	int			facesfront;
	int			vertindex[3];
} mtriangle_t;


#define	MAX_SKINS	32
typedef struct {
	int			ident;
	int			version;
	vec3_t		scale;
	vec3_t		scale_origin;
	float		boundingradius;
	vec3_t		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numvertsperframe;
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;

	int			numposes;
	int			poseverts;
	int			posedata;						// numposes*poseverts trivert_t
	int			commands;						// gl command list with embedded s/t
	int			gl_texturenum[MAX_SKINS][4];
	int			fb_texturenum[MAX_SKINS][4];
	int			isluma[MAX_SKINS][4];
	int			texels[MAX_SKINS];				// only for player skins
	maliasframedesc_t	frames[1];				// variable sized


} aliashdr_t;

#define	MAXALIASVERTS	2048
#define	MAXALIASFRAMES	256
#define	MAXALIASTRIS	4096 // SDQuake

//extern	aliashdr_t	*pheader;


#define ALIAS_BASE_SIZE_RATIO	(1.0 / 11.0) // normalizing factor so player model works out to about 1 pixel per triangle
#define	MAX_LBM_HEIGHT			480	// Max skin height

void R_DrawModel_Alias (entity_t *ent, const qbool shadowpass);

// gl_mesh.c
void GL_MakeAliasModelDisplayLists (/*model_t *m,*/ aliashdr_t *hdr);

//void GL_DrawAliasBlendedShadow (aliashdr_t *paliashdr, int pose1, int pose2, entity_t *e, vec3_t myShadeVector);
//void GL_DrawAliasShadow (entity_t *that_ent, aliashdr_t *paliashdr, int posenum, vec3_t myShadeVector);



// model_alias_vertex_lighting.c

//float VertexLights_GetVertexLightValue (int index, float my_apitch, float my_ayaw);
float VertexLights_GetVertexLightValue (const int index, const float my_apitch, const float my_ayaw);
//float VertexLights_LerpVertexLight (int index1, int index2, float ilerp, float my_apitch, float my_ayaw);
float VertexLights_LerpVertexLight (const int index1, const int index2, const float ilerp, const float my_apitch, const float my_ayaw);
void VertexLights_CalcPitchYaw (const vec3_t origin, const vec3_t angles, const vec3_t my_vertexlight, const float radiusmax, float *set_pitch, float *set_yaw);
void VertexLights_Init (void);




#endif // __MODEL_ALIAS_H__

