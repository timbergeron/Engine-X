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

#ifndef __MODEL_MD3_H__
#define __MODEL_MD3_H__

/*
==============================================================================

				Q3 MODELS

==============================================================================
*/

typedef struct
{
	int			ident;
	int			version;
	char		name[MAX_QPATH];
	int			flags;
	int			numframes;
	int			numtags;
	int			numsurfs;
	int			numskins;
	int			ofsframes;
	int			ofstags;
	int			ofssurfs;
	int			ofsend;
} md3header_t;

typedef struct
{
	vec3_t		mins, maxs;
	vec3_t		pos;
	float		radius;
	char		name[16];
} md3frame_t;

typedef struct
{
	char		name[MAX_QPATH];
	vec3_t		pos;
	vec3_t		rot[3];
} md3tag_t;

typedef struct
{
	int			ident;
	char		name[MAX_QPATH];
	int			flags;
	int			numframes;
	int			numshaders;
	int			numverts;
	int			numtris;
	int			ofstris;
	int			ofsshaders;
	int			ofstc;
	int			ofsverts;
	int			ofsend;
} md3surface_t;

typedef struct
{
	char		name[MAX_QPATH];
	int			index;
} md3shader_t;

typedef struct
{
	char		name[MAX_QPATH];
	int			index;
	int			gl_texnum, fb_texnum;
} md3shader_mem_t;

typedef struct
{
	int			indexes[3];
} md3triangle_t;

typedef struct
{
	float		s, t;
} md3tc_t;

typedef struct
{
	short		vec[3];
	unsigned short normal;
} md3vert_t;

typedef struct
{
	vec3_t		vec;
	vec3_t		normal;
	byte		anorm_pitch, anorm_yaw;
	unsigned short oldnormal;	// needed for normal lighting
} md3vert_mem_t;

#define	MD3_XYZ_SCALE	(1.0 / 64)

#define	MAXMD3FRAMES	1024
#define	MAXMD3TAGS	16
#define	MAXMD3SURFS	32
#define	MAXMD3SHADERS	256
#define	MAXMD3VERTS	4096
#define	MAXMD3TRIS	8192

typedef struct animdata_s
{
	int			offset;
	int			num_frames;
	int			loop_frames;
	float		interval;
} animdata_t;

typedef enum animtype_s
{
	both_death1, both_death2, both_death3, both_dead1, both_dead2, both_dead3,
	torso_attack, torso_attack2, torso_stand, torso_stand2,
	legs_run, legs_idle,
	NUM_ANIMTYPES
} animtype_t;

extern	animdata_t	anims[NUM_ANIMTYPES];

#define IDMD3HEADER	(('3'<<24) + ('P'<<16) + ('D'<<8) + 'I')	// little-endian "IDP3"
#define	MD3_VERSION	15

// md3 related
typedef struct tagentity_s
{
	entity_t	ent;

	float		tag_translate_start_time;
	vec3_t		tag_pos1, tag_pos2;

	float		tag_rotate_start_time[3];
	vec3_t		tag_rot1[3], tag_rot2[3];
} tagentity_t;

void R_DrawQ3Model (entity_t *ent);


#endif