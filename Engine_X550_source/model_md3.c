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

// model_md3.c -- brush model loading

#include "quakedef.h"



/*
===============================================================================

								Q3 MODELS

===============================================================================
*/

animdata_t	anims[NUM_ANIMTYPES];

static vec3_t		md3bboxmins, md3bboxmaxs;

static void Mod_GetQ3AnimData (char *buf, const char *animtype, animdata_t *adata)
{
	int		i, j, data[4];
	char	*token, num[4];

	if ((token = strstr(buf, animtype)))
	{
		while (*token != '\n')
			token--;
		token++;	// so we jump back to the first char
		for (i = 0 ; i < 4 ; i++)
		{
			memset (num, 0, sizeof(num));
			for (j = 0 ; *token != '\t' ; j++)
				num[j] = *token++;
			data[i] = atoi(num);
			token++;
		}
		adata->offset = data[0];
		adata->num_frames = data[1];
		adata->loop_frames = data[2];
		adata->interval = 1.0 / (float)data[3];
	}
}

static void Mod_LoadQ3Animation (void)
{
	int			ofs_legs;
	char		*animdata;
	animdata_t	tmp1, tmp2;

	if (!(animdata = (char *)QFS_LoadFile("progs/player/animation.cfg", 0, loadmodel->loadinfo.searchpath)))
	{
		Con_Printf ("ERROR: Couldn't open animation file\n");
		return;
	}

	memset (anims, 0, sizeof(anims));

	Mod_GetQ3AnimData (animdata, "BOTH_DEATH1", &anims[both_death1]);
	Mod_GetQ3AnimData (animdata, "BOTH_DEATH2", &anims[both_death2]);
	Mod_GetQ3AnimData (animdata, "BOTH_DEATH3", &anims[both_death3]);
	Mod_GetQ3AnimData (animdata, "BOTH_DEAD1", &anims[both_dead1]);
	Mod_GetQ3AnimData (animdata, "BOTH_DEAD2", &anims[both_dead2]);
	Mod_GetQ3AnimData (animdata, "BOTH_DEAD3", &anims[both_dead3]);

	Mod_GetQ3AnimData (animdata, "TORSO_ATTACK", &anims[torso_attack]);
	Mod_GetQ3AnimData (animdata, "TORSO_ATTACK2", &anims[torso_attack2]);
	Mod_GetQ3AnimData (animdata, "TORSO_STAND", &anims[torso_stand]);
	Mod_GetQ3AnimData (animdata, "TORSO_STAND2", &anims[torso_stand2]);

	Mod_GetQ3AnimData (animdata, "TORSO_GESTURE", &tmp1);
	Mod_GetQ3AnimData (animdata, "LEGS_WALKCR", &tmp2);
// we need to subtract the torso-only frames to get the correct indices
	ofs_legs = tmp2.offset - tmp1.offset;

	Mod_GetQ3AnimData (animdata, "LEGS_RUN", &anims[legs_run]);
	Mod_GetQ3AnimData (animdata, "LEGS_IDLE", &anims[legs_idle]);
	anims[legs_run].offset -= ofs_legs;
	anims[legs_idle].offset -= ofs_legs;

	Z_Free (animdata);
}

/*
=================
Mod_LoadQ3ModelTexture
=================
*/
static void Mod_LoadQ3ModelTexture (const char *identifier, const int flags, int *gl_texnum, int *fb_texnum)
{
	char	loadpath[64];

	snprintf (loadpath, sizeof(loadpath), "textures/q3models/%s", identifier); // Baker: Texture Rogue
	*gl_texnum = GL_LoadExternalTextureImage (loadpath, identifier, flags, loadmodel->loadinfo.searchpath);
	if (*gl_texnum)
		*fb_texnum = GL_LoadExternalTextureImage (va("%s_glow", loadpath), va("@fb_%s", identifier), flags | TEX_LUMA, loadmodel->loadinfo.searchpath);

	if (!*gl_texnum)
	{
		snprintf (loadpath, sizeof(loadpath), "textures/%s", identifier);
		*gl_texnum = GL_LoadExternalTextureImage (loadpath, identifier, flags, loadmodel->loadinfo.searchpath);
		if (*gl_texnum)
			*fb_texnum = GL_LoadExternalTextureImage (va("%s_luma", loadpath), va("@fb_%s", identifier), flags | TEX_LUMA, loadmodel->loadinfo.searchpath);
	}

	if (!*gl_texnum)
		*gl_texnum = *gl_texnum;
}

/*
=================
Mod_LoadQ3Model
=================
*/
int GameHacks_MissingFlags (const char *modelname);


void Mod_LoadQ3Model (model_t *mod, void *buffer)
{
	
	int				i, j, size, base, texture_flag, version, gl_texnum, fb_texnum, numskinsfound;
	char			basename[MAX_QPATH], pathname[MAX_QPATH], **skinsfound;
	float			radiusmax;
	md3header_t		*header;
	md3frame_t		*frame;
	md3tag_t		*tag;
	md3surface_t	*surf;
	md3shader_t		*shader;
	md3triangle_t	*tris;
	md3tc_t			*tc;
	md3vert_t		*vert;
	searchpath_t	*search;

	mod->modelformat = mod_md3;

	mod->modelflags |= GameHacks_MissingFlags (mod->name);


// some models are special
	if (!strncmp(mod->name, "progs/player", 12))
		mod->modhint = MOD_PLAYER;
#if 0
	else if (COM_StringMatch (mod->name, "progs/flame.md3"))
		mod->modhint = MOD_FLAME;
	else if (COM_StringMatch (mod->name, "progs/v_shot.md3")	||
		 COM_StringMatch (mod->name, "progs/v_shot2.md3") ||
		 COM_StringMatch (mod->name, "progs/v_nail.md3")	||
		 COM_StringMatch (mod->name, "progs/v_nail2.md3") ||
		 COM_StringMatch (mod->name, "progs/v_rock.md3")	||
		 COM_StringMatch (mod->name, "progs/v_rock2.md3"))
		mod->modhint = MOD_WEAPON;
	else if (COM_StringMatch (mod->name, "progs/lavaball.md3"))
		mod->modhint = MOD_LAVABALL;
	else if (COM_StringMatch (mod->name, "progs/spike.md3") ||
		 COM_StringMatch (mod->name, "progs/s_spike.md3"))
		mod->modhint = MOD_SPIKE;
//	else if (COM_StringMatch (mod->name, "progs/bullet.md3"))
//		mod->modhint = MOD_Q3GUNSHOT;
//	else if (COM_StringMatch (mod->name, "progs/telep.md3"))
//		mod->modhint = MOD_Q3TELEPORT;
	else
#endif
	mod->modhint = MOD_NORMAL;

	header = (md3header_t *)buffer;

	version = LittleLong (header->version);
	if (version != MD3_VERSION)
		Sys_Error ("Mod_LoadQ3Model: %s has wrong version number (%i should be %i)", mod->name, version, MD3_VERSION);

// endian-adjust all data
	header->numframes = LittleLong (header->numframes);
	if (header->numframes < 1)
		Sys_Error ("Mod_LoadQ3Model: model %s has no frames", mod->name);
	else if (header->numframes > MAXMD3FRAMES)
		Sys_Error ("Mod_LoadQ3Model: model %s has too many frames", mod->name);

	header->numtags = LittleLong (header->numtags);
	if (header->numtags > MAXMD3TAGS)
		Sys_Error ("Mod_LoadQ3Model: model %s has too many tags", mod->name);

	header->numsurfs = LittleLong (header->numsurfs);
	if (header->numsurfs < 1)
		Sys_Error ("Mod_LoadQ3Model: model %s has no surfaces", mod->name);
	else if (header->numsurfs > MAXMD3SURFS)
		Sys_Error ("Mod_LoadQ3Model: model %s has too many surfaces", mod->name);

	header->numskins = LittleLong (header->numskins);
	header->ofsframes = LittleLong (header->ofsframes);
	header->ofstags = LittleLong (header->ofstags);
	header->ofssurfs = LittleLong (header->ofssurfs);
	header->ofsend = LittleLong (header->ofsend);

	// swap all the frames
	frame = (md3frame_t *)((byte *)header + header->ofsframes);
	for (i = 0  ; i < header->numframes ; i++)
	{
		frame[i].radius = LittleFloat (frame->radius);
		for (j = 0 ; j < 3 ; j++)
		{
			frame[i].mins[j] = LittleFloat (frame[i].mins[j]);
			frame[i].maxs[j] = LittleFloat (frame[i].maxs[j]);
			frame[i].pos[j] = LittleFloat (frame[i].pos[j]);
		}
	}

	// swap all the tags
	tag = (md3tag_t *)((byte *)header + header->ofstags);
	for (i = 0 ; i < header->numtags ; i++)
	{
		for (j = 0 ; j < 3 ; j++)
		{
			tag[i].pos[j] = LittleFloat (tag[i].pos[j]);
			tag[i].rot[0][j] = LittleFloat (tag[i].rot[0][j]);
			tag[i].rot[1][j] = LittleFloat (tag[i].rot[1][j]);
			tag[i].rot[2][j] = LittleFloat (tag[i].rot[2][j]);
		}
	}

	// swap all the surfaces
	surf = (md3surface_t *)((byte *)header + header->ofssurfs);
	for (i = 0 ; i < header->numsurfs ; i++)
	{
		surf->ident = LittleLong (surf->ident);
		surf->flags = LittleLong (surf->flags);
		surf->numframes = LittleLong (surf->numframes);
		if (surf->numframes != header->numframes)
			Sys_Error ("Mod_LoadQ3Model: number of frames don't match in %s", mod->name);

		surf->numshaders = LittleLong (surf->numshaders);
		if (surf->numshaders > MAXMD3SHADERS)
			Sys_Error ("Mod_LoadQ3Model: model %s has too many shaders", mod->name);

		surf->numverts = LittleLong (surf->numverts);
		if (surf->numverts <= 0)
			Sys_Error ("Mod_LoadQ3Model: model %s has no vertices", mod->name);
		else if (surf->numverts > MAXMD3VERTS)
			Sys_Error ("Mod_LoadQ3Model: model %s has too many vertices", mod->name);

		surf->numtris = LittleLong (surf->numtris);
		if (surf->numtris <= 0)
			Sys_Error ("Mod_LoadQ3Model: model %s has no triangles", mod->name);
		else if (surf->numtris > MAXMD3TRIS)
			Sys_Error ("Mod_LoadQ3Model: model %s has too many triangles", mod->name);

		surf->ofstris = LittleLong (surf->ofstris);
		surf->ofsshaders = LittleLong (surf->ofsshaders);
		surf->ofstc = LittleLong (surf->ofstc);
		surf->ofsverts = LittleLong (surf->ofsverts);
		surf->ofsend = LittleLong (surf->ofsend);

		// swap all the shaders
		shader = (md3shader_t *)((byte *)surf + surf->ofsshaders);
		for (j = 0 ; j < surf->numshaders ; j++)
			shader[j].index = LittleLong (shader[j].index);

		// swap all the triangles
		tris = (md3triangle_t *)((byte *)surf + surf->ofstris);
		for (j = 0 ; j < surf->numtris ; j++)
		{
			tris[j].indexes[0] = LittleLong (tris[j].indexes[0]);
			tris[j].indexes[1] = LittleLong (tris[j].indexes[1]);
			tris[j].indexes[2] = LittleLong (tris[j].indexes[2]);
		}

		// swap all the texture coords
		tc = (md3tc_t *)((byte *)surf + surf->ofstc);
		for (j = 0 ; j < surf->numverts ; j++)
		{
			tc[j].s = LittleFloat (tc[j].s);
			tc[j].t = LittleFloat (tc[j].t);
		}

		// swap all the vertices
		vert = (md3vert_t *)((byte *)surf + surf->ofsverts);
		for (j = 0 ; j < surf->numverts * surf->numframes ; j++)
		{
			vert[j].vec[0] = LittleShort (vert[j].vec[0]);
			vert[j].vec[1] = LittleShort (vert[j].vec[1]);
			vert[j].vec[2] = LittleShort (vert[j].vec[2]);
			vert[j].normal = LittleShort (vert[j].normal);
		}

		// find the next surface
		surf = (md3surface_t *)((byte *)surf + surf->ofsend);
	}

// load the skins
	i = strrchr (mod->name, '/') - mod->name;
//	Q_strncpyz (pathname, mod->name, i+1);
//	strncpy (pathname, mod->name, i);
//	pathname[i+1]=0;
	strlcpy (pathname, mod->name, i+1);

#if 1 // Baker: What is going on here?  This isn't path limited.
{
	void EraseDirEntries (void);

	EraseDirEntries ();
	for (search = com_searchpaths ; search ; search = search->next)
	{
		if (!search->pack)
		{
			RDFlags |= RD_NOERASE;
			ReadDir (va("%s/%s", search->filename, pathname), va("%s*.skin", loadname));
		}
	}
	QFS_FindFilesInPak (va("%s/%s*.skin", pathname, loadname), 0, loadmodel->loadinfo.searchpath);
}
#endif //


	numskinsfound = num_files;
	skinsfound = (char **)Q_malloc (numskinsfound * sizeof(char *), "Q3");
	for (i = 0 ; i < numskinsfound ; i++)
	{
		skinsfound[i] = Q_malloc (MAX_QPATH, "Q3");
		snprintf (skinsfound[i], MAX_QPATH, "%s/%s", pathname, filelist[i].name);
	}

// allocate extra size for structures different in memory
	surf = (md3surface_t *)((byte *)header + header->ofssurfs);
	for (size = 0, i = 0 ; i < header->numsurfs ; i++)
	{
		if (numskinsfound)
			surf->numshaders = numskinsfound;
		size += surf->numshaders * sizeof(md3shader_mem_t);			// shader containing texnum
		size += surf->numverts * surf->numframes * sizeof(md3vert_mem_t);	// floating point vertices
		surf = (md3surface_t *)((byte *)surf + surf->ofsend);
	}

//	header = Cache_Alloc (&mod->cache, com_filesize + size, loadname);
	header = Cache_Alloc (&mod->cache, qfs_lastload.filelen + size, loadname);
	if (!mod->cache.data)
		return;

//	memcpy (header, buffer, com_filesize);
//	base = com_filesize;

	memcpy (header, buffer, qfs_lastload.filelen);
	base = qfs_lastload.filelen;

	mod->numframes = header->numframes;

	md3bboxmins[0] = md3bboxmins[1] = md3bboxmins[2] = 99999;
	md3bboxmaxs[0] = md3bboxmaxs[1] = md3bboxmaxs[2] = -99999;
	radiusmax = 0;

	frame = (md3frame_t *)((byte *)header + header->ofsframes);
	for (i = 0 ; i < header->numframes ; i++)
	{
		for (j = 0 ; j < 3 ; j++)
		{
			md3bboxmins[j] = min(md3bboxmins[j], frame[i].mins[j]);
			md3bboxmaxs[j] = max(md3bboxmaxs[j], frame[i].maxs[j]);
		}
		radiusmax = max(radiusmax, frame[i].radius);
	}
	VectorCopy (md3bboxmins, mod->mins);
	VectorCopy (md3bboxmaxs, mod->maxs);
	mod->radius = radiusmax;

// load the animation frames if loading the player model
	if (COM_StringMatch (mod->name, "progs/player/lower.md3"))
		Mod_LoadQ3Animation ();

	texture_flag = TEX_MIPMAP;

	surf = (md3surface_t *)((byte *)header + header->ofssurfs);
	for (i = 0 ; i < header->numsurfs; i++)
	{
#if 0
		if (strstr(surf->name, "energy") ||
			strstr(surf->name, "f_") ||
			strstr(surf->name, "flare") ||
			strstr(surf->name, "flash") ||
			strstr(surf->name, "Sphere") ||
			strstr(surf->name, "telep"))
		{
			mod->modelflags |= EF_Q3TRANS;
		}
#endif
		shader = (md3shader_t *)((byte *)surf + surf->ofsshaders);
		surf->ofsshaders = base;
		size = !numskinsfound ? surf->numshaders : numskinsfound;
		for (j = 0 ; j < size ; j++)
		{
			md3shader_mem_t	*memshader = (md3shader_mem_t *)((byte *)header + surf->ofsshaders);

			memset (memshader[j].name, 0, sizeof(memshader[j].name));
			if (!numskinsfound)
			{
//				Q_strncpyz (memshader[j].name, shader->name, sizeof(memshader[j].name));
				strlcpy (memshader[j].name, shader->name, sizeof(memshader[j].name));
				memshader[j].index = shader->index;
				shader++;
			}
			else
			{
				int		pos;
				char	*surfname, *skinname, *skindata;

				skindata = (char *)QFS_LoadFile (skinsfound[j], 0, loadmodel->loadinfo.searchpath);

				pos = 0;
//				while (pos < com_filesize)
				while (pos < qfs_lastload.filelen)
				{
					surfname = &skindata[pos];
					while (skindata[pos] != ',' && skindata[pos])
						pos++;
					skindata[pos++] = '\0';

					skinname = &skindata[pos];
					while (skindata[pos] != '\n' && skindata[pos])
						pos++;
					skindata[pos++-1] = '\0';	// becoz of \r\n

					if (strcmp(surf->name, surfname))
						continue;

					if (skinname[0])
//						Q_strncpyz (memshader[j].name, skinname, sizeof(memshader[j].name));
						strlcpy (memshader[j].name, skinname, sizeof(memshader[j].name));
				}

				Z_Free (skindata);
			}

			// the teleport model doesn't have any shaders, so we need to add
//			if (mod->modhint == MOD_Q3TELEPORT)
//				Q_strncpyz (basename, "teleportEffect2", sizeof(basename));
//			else
				COM_Copy_StripExtension (StringTemp_SkipPath(memshader[j].name), basename);

			gl_texnum = fb_texnum = 0;
			ImageWork_Start ("Q3", basename);
			Mod_LoadQ3ModelTexture (basename, texture_flag, &gl_texnum, &fb_texnum);

			memshader[j].gl_texnum = gl_texnum;
			memshader[j].fb_texnum = fb_texnum;
			ImageWork_Finish ();
		}
		base += size * sizeof(md3shader_mem_t);

		vert = (md3vert_t *)((byte *)surf + surf->ofsverts);
		surf->ofsverts = base;
		size = surf->numverts * surf->numframes;
		for (j = 0 ; j < size ; j++)
		{
			float		lat, lng;
			vec3_t		ang;
			md3vert_mem_t *vertexes = (md3vert_mem_t *)((byte *)header + surf->ofsverts);

			vertexes[j].oldnormal = vert->normal;

			vertexes[j].vec[0] = (float)vert->vec[0] * MD3_XYZ_SCALE;
			vertexes[j].vec[1] = (float)vert->vec[1] * MD3_XYZ_SCALE;
			vertexes[j].vec[2] = (float)vert->vec[2] * MD3_XYZ_SCALE;

			lat = ((vert->normal >> 8) & 0xff) * M_PI / 128.0f;
			lng = (vert->normal & 0xff) * M_PI / 128.0f;
			vertexes[j].normal[0] = cos(lat) * sin(lng);
			vertexes[j].normal[1] = sin(lat) * sin(lng);
			vertexes[j].normal[2] = cos(lng);

			vectoangles (vertexes[j].normal, ang);
			vertexes[j].anorm_pitch = ang[0] * 256 / 360;
			vertexes[j].anorm_yaw = ang[1] * 256 / 360;

			vert++;
		}
		base += size * sizeof(md3vert_mem_t);

		surf = (md3surface_t *)((byte *)surf + surf->ofsend);
	}

	for (i = 0 ; i < numskinsfound ; i++)
		free (skinsfound[i]);
	free (skinsfound);
}

