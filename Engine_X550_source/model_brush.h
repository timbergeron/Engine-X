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

#ifndef __MODEL_BRUSH_H__
#define __MODEL_BRUSH_H__

#include "quakedef.h"

/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


// in memory representation
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vec3_t					position;
} mvertex_t;

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2


// plane_t structure
// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct mplane_s
{
	vec3_t					normal;
	float					dist;
	byte					type;						// for texture axis selection and fast side tests
	byte					signbits;					// signx + signy<<1 + signz<<1
	byte					pad[2];
} mplane_t;

typedef struct texture_s
{
	char					name[16];
	unsigned				width, height;
	int						gl_texturenum;
	int						fb_texturenum;				// index of fullbright mask or 0
	int						isLumaTexture;

	struct msurface_s		*texturechain[2];
	struct msurface_s		**texturechain_tail[2];

	int						anim_total;					// total tenths in sequence (0 = no)
	int						anim_min, anim_max;			// time for this frame min <=time< max
	struct texture_s		*anim_next;					// in the animation sequence
	struct texture_s		*alternate_anims;			// bmodels in frame 1 use these

	unsigned				offsets[MIPLEVELS];			// four mip maps stored
} texture_t;


#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWLIQUID		0x10
#define SURF_DRAWTILED		0x20
#define SURF_DRAWBACKGROUND	0x40
#define SURF_UNDERWATER		0x80
#define SURF_DRAWFENCE		0x100
#define SURF_DRAWGLASSY     0x200
#define SURF_DRAWREFLECTIVE 0x400

#if SUPPORTS_TEXTURE_POINTER
#define SURF_SELECTED		0x800
#endif

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	unsigned short	v[2];
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct
{
	float		vecs[2][4];
	texture_t	*texture;
	int			flags;
} mtexinfo_t;

#define	VERTEXSIZE 9

typedef struct glpoly_s
{
	struct glpoly_s			*next;
	struct glpoly_s			*chain;
	struct glpoly_s			*fb_chain;
	struct glpoly_s			*luma_chain;				// next luma poly in chain
	struct glpoly_s			*caustics_chain;			// next caustic poly in chain
	struct glpoly_s			*detail_chain;				// next detail poly in chain
#if SUPPORTS_TEXTURE_POINTER
	struct glpoly_s 		*selection_chain;
#endif
	int						numverts;
	float					verts[4][VERTEXSIZE];		// variable sized (xyz s1t1 s2t2)
} glpoly_t;

typedef struct msurface_s
{
	int						visframe;					// should be drawn when node is crossed

	mplane_t				*plane;
	int						flags;

	int						firstedge;					// look up in model->surfedges[], negative numbers
	int						numedges;					// are backwards edges

	short					texturemins[2];
	short					extents[2];

	int						light_s, light_t;			// gl lightmap coordinates

	glpoly_t				*polys;						// multiple if warped
	struct	msurface_s		*texturechain;

	mtexinfo_t				*texinfo;

// lighting info
	int						dlightframe;
	int						dlightbits;

	int						lightmaptexturenum;
	byte					styles[MAXLIGHTMAPS];
	int						cached_light[MAXLIGHTMAPS];	// values currently used in lightmap
	qbool					cached_dlight;				// true if dynamic light in cache
	byte					*samples;					// [numstyles*surfsize]

#if SUPPORTS_OVERBRIGHT_SWITCH
	int						overbright_mode;
#endif

	int						smax;
	int						tmax;
#if SUPPORTS_FTESTAINS
	qbool					stained;
	int						stainnum;
#endif

	// Baker	
	qbool					hasorigin;
	vec3_t					brushmodel_origin;


} msurface_t;

typedef struct mnode_s
{
// common with leaf
	int						contents;					// 0, to differentiate from leafs
	int						visframe;					// node needs to be traversed if current

	float					minmaxs[6];					// for bounding box culling

	struct mnode_s			*parent;

// node specific
	mplane_t				*plane;
	struct mnode_s			*children[2];

	unsigned short			firstsurface;
	unsigned short			numsurfaces;
} mnode_t;

typedef struct mleaf_s
{
// common with node
	int						contents;					// wil be a negative contents number
	int						visframe;					// node needs to be traversed if current

	float					minmaxs[6];					// for bounding box culling

	struct mnode_s			*parent;

// leaf specific
	byte					*compressed_vis;
	efrag_t					*efrags;

	msurface_t				**firstmarksurface;
	int						nummarksurfaces;
#if FITZQUAKE_PROTOCOL
// Baker: find out what this is for in FitzQuake
	int						key;						// BSP sequence number for leaf's contents
#endif
	byte					ambient_sound_level[NUM_AMBIENTS];
} mleaf_t;

#if FITZQUAKE_PROTOCOL
//johnfitz -- for clipnodes>32k
typedef struct mclipnode_s
{
	int						planenum;
	int						children[2];				// negative numbers are contents
} mclipnode_t;
//johnfitz
#endif

// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct
{
#if FITZQUAKE_PROTOCOL
	mclipnode_t				*clipnodes;					//johnfitz -- was dclipnode_t
#else
	dclipnode_t				*clipnodes;
#endif
	mplane_t				*planes;
	int						firstclipnode;
	int						lastclipnode;
	vec3_t					clip_mins;
	vec3_t					clip_maxs;
	int						available;
} hull_t;

#define ISTURBTEX(name)		((loadmodel->bspversion == Q1_BSPVERSION && (name)[0] == '*') ||	\
							 (loadmodel->bspversion == HL_BSPVERSION && (name)[0] == '!'))

#define ISSKYTEX(name)		((name)[0] == 's' && (name)[1] == 'k' && (name)[2] == 'y')
#define ISALPHATEX(name)	(loadmodel->bspversion == HL_BSPVERSION && (name)[0] == '{')
#define ISREFLECTTEX(name)	((name)[0] == 'e' && (name)[1] == 'n' && (name)[2] == 'v')
#define ISGLASSYTEX(name)	((name)[0] == 'g' && (name)[1] == 'l' && (name)[2] == 'a' && (name)[3] == 's' && (name)[4] == 's')



// Rendering

void R_DrawModel_Brush (entity_t *ent);
void R_Build_WorldChains (void);
// gl_rsurf.c
void R_DrawWorld_Solid_TextureChains (const struct model_s *model, const int contents, const float unused1, const int frame_brush);
void R_DrawFenceChain (void);
void R_DrawGlassyChain (void);

// Moved so I can use R00k's caustics on brush models
#define TruePointContents(p) SV_HullPointContents(&cl.worldmodel->hulls[0], 0, p)
#define ISUNDERWATER(x) ((x) == CONTENTS_WATER || (x) == CONTENTS_SLIME || (x) == CONTENTS_LAVA)


// model_brush_render_lighting.c
#define	LIGHTMAP_BLOCK_WIDTH		128
#define	LIGHTMAP_BLOCK_HEIGHT		128
#define LIGHTMAP_MAX_BYTES			3



typedef struct glRect_s {
	unsigned char	l, t, w, h;
} glRect_t;

typedef unsigned char stmap;
typedef struct
{
	qbool		modified;																	////qbool	lightmap_modified[MAX_LIGHTMAPS];
	glRect_t	rectchange;																	//static	glRect_t	lightmap_rectchange[MAX_LIGHTMAPS];
	int			allocated[LIGHTMAP_BLOCK_WIDTH];											//static	int		allocated[MAX_LIGHTMAPS][LIGHTMAP_BLOCK_WIDTH];
	byte		lightmaps[LIGHTMAP_MAX_BYTES*LIGHTMAP_BLOCK_WIDTH*LIGHTMAP_BLOCK_HEIGHT];	//static byte		lightmaps[LIGHTMAP_MAX_BYTES*MAX_LIGHTMAPS*LIGHTMAP_BLOCK_WIDTH*LIGHTMAP_BLOCK_HEIGHT];
#if SUPPORTS_FTESTAINS
	byte		stainmaps[LIGHTMAP_MAX_BYTES*LIGHTMAP_BLOCK_WIDTH*LIGHTMAP_BLOCK_HEIGHT];
#endif
	glpoly_t	*polys;

} lightmapinfo_t;

extern lightmapinfo_t lightmap[MAX_LIGHTMAPS];

void Light_BuildAllLightmaps_NewMap (void);

// gl_refrag.c
#if FITZQUAKE_PROTOCOL
void Efrags_CheckEfrags (void); //johnfitz
#endif

void Efrags_StoreEfrags (efrag_t **ppefrag);
void Efrags_AddEfrags (entity_t *ent);
void Efrags_RemoveEfrags (entity_t *ent);
void Efrags_NewMap (void);


// model_brush_render_lighting.c

void Light_Init (void);											// Once per session
void Light_NewMap (void);										// Once per map
void Light_FrameSetup (void);									// Once per frame

void Light_Render_FlashBlend_Dlights (void);					// Render the flash blend bubble
void Light_PushDlights_DrawModel_Brush_Ent (const struct model_s	*clmodel);	// For every brush entity

// Called when drawing the world surface chains
void Light_GenerateDynamicLightmaps_For_Surface (msurface_t *fa);
void Light_UploadLightmaps_Modified (void);

// Light sampling
extern	vec3_t	lightpoint_lightspot, lightpoint_lightcolor;
int Light_LightPoint (const vec3_t startpoint);								// Tells how much light, the spot and the color on a entity


// Dlights

void Light_NewDlight (const int key, const vec3_t origin, const float radius, const float time, const int type);
dlight_t *Light_AllocDlight (const int key);
void Light_DecayLights (void);
#ifdef SUPPORTS_COLORED_LIGHTS
dlighttype_t Light_SetDlightColor (const float f, const dlighttype_t def, const qbool random);
#endif


void Stain_AddStain(const vec3_t origin, const float tint, /*float red, float green, float blue,*/ const float radius);




// model_brush_surface_pointer.c

#if SUPPORTS_TEXTURE_POINTER
//qbool R_RecursiveWallPoint (mnode_t *node, vec3_t start, vec3_t end);
int TexturePointer_SurfacePoint (void);
void TexturePointer_NewMap (void);
#endif


extern	float	bubblecolor[NUM_DLIGHTTYPES][4];

//extern	float	vlight_pitch, vlight_yaw;


// model_brush_render_sky.c

extern qbool	r_skyboxloaded;

void Sky_OldSky_DrawChain (void);
void Sky_OldSky_Load_NewMap (const byte *qpal_src);		// called at level load
void Sky_SkyBox_NewMap (void);
void Sky_SkyBox_Draw (void);



// model_brush_render_liquids.c
void CalcCausticTexCoords (const float *verts, float *s, float *t);
void CalcReflectiveTexCoords (const float *verts, float *s, float *t);

void EmitWaterPolys (const msurface_t *surf);
void EmitGlassyPolys (const msurface_t *surf);
void EmitFencePolys (const msurface_t *surf);
void SubdividePolygon (const int numverts, float *verts);

void R_DrawWaterSurfaces (void);

#endif // __MODEL_BRUSH_H__

