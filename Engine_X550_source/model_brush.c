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

// model_brush.c -- brush model loading

#include "quakedef.h"


/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/


static byte	*mod_base;


/*
=================
Mod_LoadBrushModelTexture
=================
*/
static int Mod_LoadTexturesExternal_Brush (texture_t *tx, int flags)
{
	char	*name;

	if (!external_textures.integer)
		return 0;

	if (loadmodel->bspversion == HL_BSPVERSION)
		return 0;

//	if ((loadmodel->isworldmodel && !external_textures.integer) || !gl_externaltextures_bmodels.integer)
//		return 0;


	name = tx->name;
//	mapname = CL_MapName ();

	// Baker: Worldmodel --- look in textures\<map name>\ folder
	if (loadmodel->isworldmodel)
	{
		if ((tx->gl_texturenum = GL_LoadExternalTextureImage (va("textures/%s/%s", host_worldname, name), name, flags, NULL /*PATH LIMIT ME*/)))
		{
			if (!ISTURBTEX(name))
				tx->fb_texturenum = GL_LoadExternalTextureImage (va("textures/%s/%s_glow", host_worldname, name), va("@fb_%s", name), flags | TEX_LUMA, NULL /*PATH LIMIT ME*/);
		}
	}
	// Baker: Worldmodel plus original map --- look in textures\exmy if external texture not loaded
	if (!tx->gl_texturenum && loadmodel->isworldmodel && cl.original_map_for_exmy_folder_textures)
	{
		if ((tx->gl_texturenum = GL_LoadExternalTextureImage (va("textures/exmy/%s", name), name, flags, NULL /*PATH LIMIT ME*/)))
		{
			if (!ISTURBTEX(name))
				tx->fb_texturenum = GL_LoadExternalTextureImage (va("textures/exmy/%s_glow", name), va("@fb_%s", name), flags | TEX_LUMA, NULL /*PATH LIMIT ME*/);
		}
	}

	// Baker: All above failed ... check textures folder
	if (!tx->gl_texturenum)
	{
		if ((tx->gl_texturenum = GL_LoadExternalTextureImage (va("textures/%s", name), name, flags, NULL /*PATH LIMIT ME*/)))
		{
			if (!ISTURBTEX(name))
				tx->fb_texturenum = GL_LoadExternalTextureImage  (va("textures/%s_glow", name), va("@fb_%s", name), flags | TEX_LUMA, NULL /*PATH LIMIT ME*/);
		}
	}
	if (tx->fb_texturenum)
		tx->isLumaTexture = true;

	return tx->gl_texturenum;
}

//converts paletted to rgba
static byte *ConvertWad3ToRGBA(texture_t *tex)
{
	byte *in, *data, *pal;
	int i, p, image_size;

	if (!tex->offsets[0])
		Sys_Error("ConvertWad3ToRGBA: tex->offsets[0] == 0");

	image_size = tex->width * tex->height;
	in = (byte *) ((byte *) tex + tex->offsets[0]);
	data = ImageWork_malloc(image_size * 4, "WAD 3"); // Baker

	pal = in + ((image_size * 85) >> 6) + 2;
	for (i = 0; i < image_size; i++)
	{
		p = *in++;
		if (tex->name[0] == '{' && p == 255)
		{
			((int *) data)[i] = 0;
		}
		else
		{
			p *= 3;
			data[i * 4 + 0] = pal[p];
			data[i * 4 + 1] = pal[p + 1];
			data[i * 4 + 2] = pal[p + 2];
			data[i * 4 + 3] = 255;
		}
	}
	return data;
}

/*
=================
Mod_LoadTextures
=================
*/
static void Mod_LoadTextures_Brush (lump_t *l)
{
	int				i;
	dmiptexlump_t	*m;

	int				texture_flag = loadmodel->isworldmodel ? TEX_WORLD: 0;

	if (tex_picmip_allmodels.integer || loadmodel->isworldmodel) texture_flag |= TEX_MIPMAP;

	// Baker: we have to "load" the textures even in dedicated, just do not upload them.
	// No textures.  Leave.  FitzQuake would allocate slots for the textures and continue.  Maybe later.
	if (!l->filelen)	{ loadmodel->textures = NULL; return; }

	m = (dmiptexlump_t *)(mod_base + l->fileofs);
	loadmodel->numtextures = m->nummiptex = LittleLong (m->nummiptex);
	loadmodel->textures = Hunk_AllocName (loadmodel->numtextures, sizeof(*loadmodel->textures), loadname);

	for (i = 0 ; i < loadmodel->numtextures ; i++)
	{	miptex_t		*mt;
		qbool			notexture=false;

		if ((m->dataofs[i] = LittleLong (m->dataofs[i])) == -1)	continue;	// Baker: Protect against something foobared?

		{	// Populate the mip texture
			mt = (miptex_t *)((byte *)m + m->dataofs[i]);

			mt->width = LittleLong (mt->width);					// Endian correct
			mt->height = LittleLong (mt->height);				// Endian correct
			mt->offsets[0] = LittleLong (mt->offsets[0]);		// Endian correct (offsets 1,2,3 = on disk mipmaps = aren't used GLQuake)

			if ((mt->width & 15) || (mt->height & 15)) Host_Error ("Mod_LoadTextures: Texture %s is not 16 aligned", mt->name);
			if (!mt->offsets[0])								notexture = true;

			// HACK HACK HACK	// gamehack
			if (COM_StringMatch (mt->name, "shot1sid") && mt->width == 32 && mt->height == 32 && CRC_Block((byte*)(mt+1), mt->width*mt->height) == 65393)
			{	// This texture in b_shell1.bsp has some of the first 32 pixels painted white.
				// They are invisible in software, but look really ugly in GL. So we just copy
				// 32 pixels from the bottom to make it look nice.
				memcpy (mt+1, (byte *)(mt+1) + 32*31, 32);
			}

		}

		{	// Populate the texture_t
			int			txwidth		= notexture ? r_notexture_mip->width : mt->width;
			int			txheight	= notexture ? r_notexture_mip->width : mt->height;
			int			dataoffset  = mt->offsets[0] + sizeof(texture_t) - sizeof(miptex_t);		// Structure size is different so data offset is different
			int			pixelformat = notexture ? Q1_BSPVERSION			 : loadmodel->bspversion;	// "No texture" is always QPAL
			int			palettesize = (pixelformat == HL_BSPVERSION) ? 2 + 768  : 0;				// 768 = 256 colors x 3 for RGB bytes
			int			pixels		= txwidth * txheight / 64 * 85 + palettesize;
			texture_t	*tx			= Hunk_AllocName (1, sizeof(texture_t) + pixels, loadname);		// Allocate the texture


			loadmodel->textures[i] = tx;						// Let the map know where the texture_t is
			memcpy (tx->name, mt->name, sizeof(tx->name));		// Change empty texture names to unnamed0, unnamed1, etc.

			if (!tx->name[0]) { snprintf (tx->name, sizeof(tx->name), "unnamed%d", i); Con_DevPrintf (DEV_MODEL, "Warning: unnamed texture in %s, renaming to %s\n", loadmodel->name, tx->name); }

			if (notexture)
				memcpy (tx+1, r_notexture_mip + 1, pixels);		// Handle notexture situation ...
			else
				memcpy (tx+1, mt+1, pixels);					// .. or copy the data over, the pixels immediately follow the structures

			// Fill in the data
			tx->width				= txwidth;
			tx->height				= txheight;
//			tx->source_pixel_format = pixelformat;
			tx->offsets[0]          = dataoffset;
//			tx->texture_type		= TextureTypeForName(tx->name, loadmodel->bspversion);
//			tx->processing_flags	= ProcessingFlagsForType (tx->texture_type, tx->source_pixel_format);

			if (isDedicated)
				continue;  // Baker: dedicated doesn't upload textures

			ImageWork_Start ("brushtex", tx->name);

			do
			{
				byte *native_data = (byte *)tx + tx->offsets[0];
				byte *RGBA_Data = NULL;

				// If it isn't the sky, try loading an external texture

				if (!ISSKYTEX(tx->name) && Mod_LoadTexturesExternal_Brush(tx, texture_flag))
					break;

				// 
				// At this point it will be native data (you know, not an external texture)
				//

				// Quake sky texture for world model (if not Half-Life map)
				if (loadmodel->isworldmodel  && loadmodel->bspversion != HL_BSPVERSION  && ISSKYTEX(tx->name))
				{
					Sky_OldSky_Load_NewMap (native_data);
					break;
				}

				// Half-Life texture
				if (loadmodel->bspversion == HL_BSPVERSION && (RGBA_Data = ConvertWad3ToRGBA(tx)))
				{
					int texmode = texture_flag | (ISALPHATEX(tx->name) ? TEX_ALPHA_TEST : 0);
					tx->gl_texturenum = GL_LoadPreprocess_Texture_RGBA_Pixels_And_Upload (RGBA_Data, tx->name, tx->width, tx->height, texmode);
					ImageWork_free (RGBA_Data);
					break;
				}

				// Kurok texture with mask color
				if (game_kurok.integer && Texture_QPAL_HasMaskColor255 (native_data, tx->width * tx->height))
				{
//					Con_Printf ("BModel: %s has masked texture %s\n", loadmodel->name, tx->name);
					tx->gl_texturenum = GL_LoadTexture (va("@mask_%s", tx->name), tx->width, tx->height, native_data, texture_flag | TEX_ALPHA_TEST, QPAL_BYTES_PER_PIXEL_IS_1);
//					loadmodel->modelflags |= MOD_RENDERFENCE;
					break;
				}
				
				// Alpha texture prefix "{" with mask color
				if (ISALPHATEX(tx->name) && Texture_QPAL_HasMaskColor255 (native_data, tx->width * tx->height))
				{
//					Con_Printf ("BModel: %s has masked texture %s\n", loadmodel->name, tx->name);
					tx->gl_texturenum = GL_LoadTexture (va("@mask_%s", tx->name), tx->width, tx->height, native_data, texture_flag | TEX_ALPHA_TEST, QPAL_BYTES_PER_PIXEL_IS_1);
					break;
				}

				// 
				// If we reached here we are loading the usual texture
				//

				tx->gl_texturenum = GL_LoadTexture (tx->name, tx->width, tx->height, native_data, texture_flag, QPAL_BYTES_PER_PIXEL_IS_1);

				// If it isn't Kurok and isn't a water texture, check for fullbright pixels and upload a fullbright texture 
				if (!game_kurok.integer && !ISTURBTEX(tx->name) && Texture_QPAL_HasFullbrights(native_data, tx->width * tx->height))
					tx->fb_texturenum = GL_LoadTexture (va("@fb_%s", tx->name), tx->width, tx->height, native_data, texture_flag | TEX_FULLBRIGHT, QPAL_BYTES_PER_PIXEL_IS_1);
		
			} while (0);

			ImageWork_Finish ();
		}
	}

// sequence the animations
	for (i=0 ; i<loadmodel->numtextures ; i++)
	{
		int				j, num, max, altmax;
		texture_t		*ptx, *ptx2;
		texture_t		*anims[10];
		texture_t		*altanims[10];

		ptx = loadmodel->textures[i];
		if (!ptx || ptx->name[0] != '+')
			continue;

		if (ptx->anim_next)
			continue;	// already sequenced

	// find the number of frames in the animation
		memset (anims, 0, sizeof(anims));
		memset (altanims, 0, sizeof(altanims));

		max = ptx->name[1];
		altmax = 0;
		if (max >= 'a' && max <= 'z')
			max -= 'a' - 'A';

		if (max >= '0' && max <= '9')
		{
			max -= '0';
			altmax = 0;
			anims[max] = ptx;
			max++;
		}
		else if (max >= 'A' && max <= 'J')
		{
			altmax = max - 'A';
			max = 0;
			altanims[altmax] = ptx;
			altmax++;
		}
		else
		{
			Host_Error ("Mod_LoadTextures_Brush: Bad animating texture %s", ptx->name);
		}

		for (j=i+1 ; j < loadmodel->numtextures ; j++)
		{
			ptx2 = loadmodel->textures[j];
			if (!ptx2 || ptx2->name[0] != '+')
				continue;
			if (COM_StringNOTMatch (ptx2->name+2, ptx->name+2)) // Why +2.
				continue;

			num = ptx2->name[1];
			if (num >= 'a' && num <= 'z')
				num -= 'a' - 'A';
			if (num >= '0' && num <= '9')
			{
				num -= '0';
				anims[num] = ptx2;
				if (num+1 > max)
					max = num + 1;
			}
			else if (num >= 'A' && num <= 'J')
			{
				num = num - 'A';
				altanims[num] = ptx2;
				if (num+1 > altmax)
					altmax = num+1;
			}
			else
			{
				Host_Error ("Mod_LoadTextures_Brush: Bad animating texture %s", ptx->name);
			}
		}

#define	ANIM_CYCLE	2
	// link them all together
		for (j=0 ; j<max ; j++)
		{
			ptx2 = anims[j];
			if (!ptx2)
				Host_Error ("Mod_LoadTextures_Brush: Missing frame %i of %s", j, ptx->name);
			ptx2->anim_total = max * ANIM_CYCLE;
			ptx2->anim_min = j * ANIM_CYCLE;
			ptx2->anim_max = (j+1) * ANIM_CYCLE;
			ptx2->anim_next = anims[(j+1)%max];
			if (altmax)
				ptx2->alternate_anims = altanims[0];
		}

		for (j=0 ; j<altmax ; j++)
		{
			ptx2 = altanims[j];
			if (!ptx2)
				Host_Error ("Mod_LoadTextures_Brush: Missing frame %i of %s", j, ptx->name);
			ptx2->anim_total = altmax * ANIM_CYCLE;
			ptx2->anim_min = j * ANIM_CYCLE;
			ptx2->anim_max = (j+1) * ANIM_CYCLE;
			ptx2->anim_next = altanims[(j+1)%altmax];
			if (max)
				ptx2->alternate_anims = anims[0];
		}
	}
}

#ifdef SUPPORTS_COLORED_LIGHTS
// joe: from FuhQuake
static byte *LoadColoredLighting (char *name, char **litfilename)
{
	byte		*data;


	if (!loadmodel->isworldmodel)
		return NULL;

	*litfilename = va("maps/%s.lit", host_worldname);
	data = QFS_LoadHunkFile (*litfilename, loadmodel->loadinfo.searchpath /*PATH LIMIT ME*/);

	if (!data)	// I generally dont like the idea of a lits folder.  Consider this more.
	{
		*litfilename = va("lits/%s.lit", host_worldname);
		data = QFS_LoadHunkFile (*litfilename, loadmodel->loadinfo.searchpath /*PATH LIMIT ME*/);
	}

	return data;
}
#endif

/*
=================
Mod_LoadLighting
=================
*/
static void Mod_LoadLighting (lump_t *l)
{
	int	i, lit_ver, mark;
	byte	*in, *out, *data, d;
	char	*litfilename;

	loadmodel->lightdata = NULL;


	if (l->filelen && loadmodel->bspversion == HL_BSPVERSION)
	{
		loadmodel->lightdata = Hunk_AllocName(1, l->filelen, loadname);
		memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
		return;
	}
	do
	{
		if (!external_lits.integer)
			break;

		// check for a .lit file
		mark = Hunk_LowMark ();
		data = LoadColoredLighting (loadmodel->name, &litfilename);

		if (!data)
			break;

		// Error conditions

		if (qfs_lastload.filelen < 8 || strncmp (data, "QLIT", 4))
		{
			Hunk_FreeToLowMark (mark);
			Con_Printf ("Corrupt .lit file (%s)...ignoring\n", StringTemp_SkipPath(litfilename));
			break;
		}


		if (l->filelen * 3 + 8 != qfs_lastload.filelen)
		{
			Hunk_FreeToLowMark (mark);
			Con_Printf ("Warning: .lit file (%s) has incorrect size\n", StringTemp_SkipPath(litfilename));
			break;
		}

		if ((lit_ver = LittleLong(((int *)data)[1])) != 1)
		{
			Hunk_FreeToLowMark (mark);
			Con_Printf ("Unknown .lit file version (v%d)\n", lit_ver);
			break;
		}
		Con_DevPrintf (DEV_MODEL, "Static coloured lighting loaded\n");

		loadmodel->lightdata = data + 8;
		in = mod_base + l->fileofs;
		out = loadmodel->lightdata;
		for (i=0 ; i<l->filelen ; i++)
		{
			int	b = max(out[3*i], max(out[3*i+1], out[3*i+2]));

			if (!b)
				out[3*i] = out[3*i+1] = out[3*i+2] = in[i];
			else
			{	// too bright
				float	r = in[i] / (float)b;

				out[3*i+0] = (int)(r * out[3*i+0]);
				out[3*i+1] = (int)(r * out[3*i+1]);
				out[3*i+2] = (int)(r * out[3*i+2]);
			}
		}
		return;
	} while (0);

	// no .lit found
	if (!l->filelen)
	{
		loadmodel->lightdata = NULL;
		return;
	}

	// Expand the white lighting data to color ...

	loadmodel->lightdata = Hunk_AllocName (3, l->filelen , va("%s_@lightdata", loadmodel->name));	// 3 = R + G + B bytes

	// place the file at the end, so it will not be overwritten until the very last write
	in = loadmodel->lightdata + l->filelen * 2;
	out = loadmodel->lightdata;
	memcpy (in, mod_base + l->fileofs, l->filelen);
	for (i = 0 ; i < l->filelen ; i++, out += 3)
	{
		d = *in++;
		out[0] = out[1] = out[2] = d;
	}
}

/*
=================
Mod_LoadVisibility
=================
*/
static void Mod_LoadVisibility (lump_t *l)
{
	if (!l->filelen)
	{
		Con_DevPrintf (DEV_MODEL, "No vis for model\n");
		loadmodel->visdata = NULL;
		return;
	}
	else
	{
		loadmodel->visdata = Hunk_AllocName (1, l->filelen, loadname);
		memcpy (loadmodel->visdata, mod_base + l->fileofs, l->filelen);
	}
}

// Baker: This has been rewritten but untested.  Unsure
// if it works since ... well ... I didn't test it.
#if SUPPORTS_HLBSP_EXTWADS	// Baker: This is parsing out the external WADs
static void Mod_ParseWadsFromEntityLump(char *data)
{
	char	*valuestring = StringTemp_ObtainValueFromClientWorldSpawn("wad");

	if (!valuestring)
		return; 	// Not found
	{
		int 	i, k, j = 0;
		char	value[1024];

		StringLCopy (value, valuestring, sizeof(value));

		for (i = 0; i < strlen(value); i++)
		{
			if (value[i] != ';' && value[i] != '\\' && value[i] != '/' && value[i] != ':')
				break;
		}

		if (!value[i])
			continue;

#pragma message ("Errr ... 'continue' what?  Shouldn't that be inside the for loop?")

		for ( ; i < sizeof(value); i++)
		{
			// ignore path - the \\ check is for HalfLife... stupid windoze 'programmers'...
			if (value[i] == '\\' || value[i] == '/' || value[i] == ':')
			{
				j = i + 1;
			}
			else if (value[i] == ';' || value[i] == 0)
			{
				k = value[i];
				value[i] = 0;
				if (value[j])
					WAD3_LoadTextureWadFile (value + j);
				j = i + 1;
				if (!k)
					break;
			}
		}
	}


}
#endif

/*
=================
Mod_LoadEntities
=================
*/
static void Mod_LoadEntities (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->entities = NULL;
		return;
	}

	loadmodel->entities		= Hunk_AllocName (1, l->filelen, loadname);

	memcpy (loadmodel->entities, mod_base + l->fileofs, l->filelen);

#if SUPPORTS_HLBSP_EXTWADS
	if (loadmodel->bspversion == HL_BSPVERSION && !isDedicated)
		Mod_ParseWadsFromEntityLump(loadmodel->entities);	// Baker: This is an external .ent file buster client-side
#endif
}


/*
=================
Mod_LoadVertexes
=================
*/
static void Mod_LoadVertexes (lump_t *l)
{
	dvertex_t	*in;
	mvertex_t	*out;
	int			i, count;

	if (loadmodel->isworldmodel)
	{
		VectorSet (cl.worldmins,  99999999,  99999999,  99999999);
		VectorSet (cl.worldmaxs, -99999999, -99999999, -99999999);
	}

	in						= (void *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))	Host_Error ("Mod_LoadVertexes: funny lump size in %s",loadmodel->name);

	count					= l->filelen / sizeof(*in);
	out						= Hunk_AllocName (count, sizeof(*out), loadname);

	loadmodel->vertexes		= out;
	loadmodel->numvertexes = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		out->position[0] = LittleFloat (in->point[0]);
		out->position[1] = LittleFloat (in->point[1]);
		out->position[2] = LittleFloat (in->point[2]);
		if (loadmodel->isworldmodel)	VectorExtendLimits (out->position, cl.worldmins, cl.worldmaxs);
	}
}


/*
=================
Mod_LoadSubmodels
=================
*/
static void Mod_LoadSubmodels (lump_t *l)
{
	dmodel_t	*in, *out;
	int		i, j, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadSubmodels: funny lump size in %s",loadmodel->name);

	count = l->filelen / sizeof(*in);
	if (count > MAX_MODELS)
		Host_Error ("Mod_LoadSubmodels: count > MAX_MODELS");

	out = Hunk_AllocName (count, sizeof(*out), loadname);

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
			out->origin[j] = LittleFloat (in->origin[j]);
		}
		for (j=0 ; j<MAX_MAP_HULLS ; j++)
			out->headnode[j] = LittleLong (in->headnode[j]);
		out->visleafs = LittleLong (in->visleafs);
		out->firstface = LittleLong (in->firstface);
		out->numfaces = LittleLong (in->numfaces);
	}

	// johnfitz -- check world visleafs -- adapted from bjp
	out = loadmodel->submodels;

	if (out->visleafs > MAX_MAP_LEAFS)
		Host_Error ("Mod_LoadSubmodels: too many visleafs (%d, max = %d) in %s", out->visleafs, MAX_MAP_LEAFS, loadmodel->name);

//	if (out->visleafs > 8192)
//		Con_Warning ("%i visleafs exceeds standard limit of 8192.\n", out->visleafs);

	if (out->visleafs > MAX_WINQUAKE_MAP_LEAFS)
		Con_Warning ("%i visleafs exceeds standard limit of %i.\n", out->visleafs, MAX_WINQUAKE_MAP_LEAFS);

	//johnfitz
}

/*
=================
Mod_LoadEdges
=================
*/
static void Mod_LoadEdges (lump_t *l)
{
	dedge_t	*in;
	medge_t	*out;
	int 	i, count;

	in = (void *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadEdges: funny lump size in %s", loadmodel->name);

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ((count + 1), sizeof(*out), loadname);

	loadmodel->edges = out;
	loadmodel->numedges = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		out->v[0] = (unsigned short)LittleShort (in->v[0]);
		out->v[1] = (unsigned short)LittleShort (in->v[1]);
	}
}

/*
=================
Mod_LoadTexinfo
=================
*/
static void Mod_LoadTexinfo (lump_t *l)
{
	texinfo_t	*in;
	mtexinfo_t	*out;
	int 		i, j, count, miptex;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadTexinfo: funny lump size in %s", loadmodel->name);

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count, sizeof(*out), loadname);

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<8 ; j++)
			out->vecs[0][j] = LittleFloat (in->vecs[0][j]);

		miptex = LittleLong (in->miptex);
		out->flags = LittleLong (in->flags);

		if (!loadmodel->textures)	// Baker: missing all textures situation?
		{
			out->texture = r_notexture_mip;	// checkerboard texture
			out->flags = 0;
		}
		else
		{
			if (miptex >= loadmodel->numtextures)
				Host_Error ("Mod_LoadTexinfo: miptex >= loadmodel->numtextures");

			out->texture = loadmodel->textures[miptex];
			if (!out->texture)
			{
				out->texture = r_notexture_mip;	// texture not found
				out->flags = 0;
			}
		}
	}
}

/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
static void CalcSurfaceExtents (msurface_t *s)
{
	float		mins[2], maxs[2], val;
	int			i, j, e, bmins[2], bmaxs[2];
	mvertex_t	*v;
	mtexinfo_t	*tex;

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = s->texinfo;

	for (i=0 ; i<s->numedges ; i++)
	{
		e = loadmodel->surfedges[s->firstedge+i];
		if (e >= 0)
			v = &loadmodel->vertexes[loadmodel->edges[e].v[0]];
		else
			v = &loadmodel->vertexes[loadmodel->edges[-e].v[1]];

		for (j=0 ; j<2 ; j++)
		{
			val = v->position[0] * tex->vecs[j][0] + v->position[1] * tex->vecs[j][1] + v->position[2] * tex->vecs[j][2] + tex->vecs[j][3];
			if (val < mins[j])
				mins[j] = val;
			if (val > maxs[j])
				maxs[j] = val;
		}
	}

	for (i=0 ; i<2 ; i++)
	{
		bmins[i] = floorf (mins[i] / 16);
		bmaxs[i] = ceilf (maxs[i] / 16);

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = (bmaxs[i] - bmins[i]) * 16;

		if (!(tex->flags & TEX_SPECIAL) && s->extents[i] > 512) // Baker: Fitz jacked extents to 2000 from 512 GLQuake/256 WinQuake
			Host_Error ("CalcSurfaceExtents: Bad surface extents");
	}
}

void GL_SubdivideSurface (const model_t *loadmodel_subd, msurface_t *fa);


/*
=================
Mod_LoadFaces
=================
*/
static void Mod_LoadFaces (lump_t *l)
{
	int			i, count, surfnum, planenum, side;
	dface_t		*in;
	msurface_t 	*out;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadFaces: funny lump size in %s", loadmodel->name);

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count, sizeof(*out), loadname);

#if FITZQUAKE_PROTOCOL
	//johnfitz -- warn mappers about exceeding old limits
//	if (count > 32767)
//		Con_Warning ("%i faces exceeds standard limit of 32767.\n", count);

	if (count > MAX_WINQUAKE_MAP_FACES)
		Con_Warning ("%i faces exceeds standard limit of %i.\n", count, MAX_WINQUAKE_MAP_FACES);

	//johnfitz
#endif

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	for (surfnum=0 ; surfnum<count ; surfnum++, in++, out++)
	{
		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleShort(in->numedges);
		out->flags = 0;

		planenum = LittleShort(in->planenum);

		if ((side = LittleShort(in->side)))
			out->flags |= SURF_PLANEBACK;

		out->plane = loadmodel->planes + planenum;

		out->texinfo = loadmodel->texinfo + LittleShort (in->texinfo);

		CalcSurfaceExtents (out);

	// lighting info

		for (i=0 ; i<MAXLIGHTMAPS ; i++)
			out->styles[i] = in->styles[i];
		i = LittleLong(in->lightofs);
		if (i == -1)
			out->samples = NULL;
		else
			out->samples = loadmodel->lightdata + (loadmodel->bspversion == HL_BSPVERSION ? i : i * 3);

	// MH does the overbright thing heere
#if SUPPORTS_OVERBRIGHT_SWITCH
		// mh - overbrights
		out->overbright_mode = light_overbright.integer;
#endif


	// set the drawing flags flag
		if (ISSKYTEX(out->texinfo->texture->name))	// sky
		{
			out->flags |= (SURF_DRAWSKY | SURF_DRAWTILED);
			GL_SubdivideSurface (loadmodel, out);	// cut up polygon for warps
			continue;
		}

		if (ISTURBTEX(out->texinfo->texture->name))	// turbulent
		{
			out->flags |= (SURF_DRAWLIQUID | SURF_DRAWTILED);
			for (i=0 ; i<2 ; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}
			GL_SubdivideSurface (loadmodel, out);	// cut up polygon for warps
			continue;
		}

		if (ISALPHATEX(out->texinfo->texture->name))
			out->flags |= SURF_DRAWFENCE;

		if (game_kurok.integer && ISGLASSYTEX(out->texinfo->texture->name))
			out->flags |= SURF_DRAWGLASSY;

		if (game_kurok.integer && ISREFLECTTEX(out->texinfo->texture->name))
			out->flags |= SURF_DRAWREFLECTIVE;

		// Baker: How do we figure out the gltexture num?
		{
			int GL_FindTextureNum (const char *identifier);
//			int texnum;
			if (game_kurok.integer && GL_FindTextureNum (va("@mask_%s", out->texinfo->texture->name))!= -1)
				out->flags |= SURF_DRAWFENCE;

		}

	}
}

/*
=================
Mod_SetParent
=================
*/
static void Mod_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;

	if (node->contents < 0)
		return;

	Mod_SetParent (node->children[0], node);
	Mod_SetParent (node->children[1], node);
}

/*
=================
Mod_LoadNodes
=================
*/
static void Mod_LoadNodes (lump_t *l)
{
	int		i, j, count, p;
	dnode_t		*in;
	mnode_t 	*out;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadNodes: funny lump size in %s", loadmodel->name);

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count, sizeof(*out), loadname);

#if FITZQUAKE_PROTOCOL
	//johnfitz -- warn mappers about exceeding old limits
//	if (count > 32767)
//		Con_Warning ("%i nodes exceeds standard limit of 32767.\n", count);

	if (count > MAX_WINQUAKE_MAP_NODES)
		Con_Warning ("%i nodes exceeds standard limit of %i.\n", count, MAX_WINQUAKE_MAP_NODES);

	//johnfitz
#endif

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}

		p = LittleLong(in->planenum);
		out->plane = loadmodel->planes + p;

#if FITZQUAKE_PROTOCOL
		out->firstsurface = (unsigned short)LittleShort (in->firstface); //johnfitz -- explicit cast as unsigned short
		out->numsurfaces = (unsigned short)LittleShort (in->numfaces); //johnfitz -- explicit cast as unsigned short
#else
		out->firstsurface = LittleShort (in->firstface);
		out->numsurfaces = LittleShort (in->numfaces);
#endif
		for (j=0 ; j<2 ; j++)
		{
#if FITZQUAKE_PROTOCOL
			//johnfitz -- hack to handle nodes > 32k, adapted from darkplaces
			p = (unsigned short)LittleShort(in->children[j]);
			if (p < count)
				out->children[j] = loadmodel->nodes + p;
			else
			{
				p = 65535 - p; //note this uses 65535 intentionally, -1 is leaf 0
				if (p < loadmodel->numleafs)
					out->children[j] = (mnode_t *)(loadmodel->leafs + p);
				else
				{
					Con_Printf("Mod_LoadNodes: invalid leaf index %i (file has only %i leafs)\n", p, loadmodel->numleafs);
					out->children[j] = (mnode_t *)(loadmodel->leafs); //map it to the solid leaf
				}
			}
			//johnfitz
#else
			p = LittleShort (in->children[j]);
			if (p >= 0)
				out->children[j] = loadmodel->nodes + p;
			else
				out->children[j] = (mnode_t *)(loadmodel->leafs + (-1 - p));
#endif
		}
	}

	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}



#if SUPPORTS_EXTERNAL_VIS

static void Mod_ProcessLeafs (dleaf_t *in, int filelen)
{
	mleaf_t 	*out;
	int			i, j, count, p;

	if (filelen % sizeof(*in))				Sys_Error ("Mod_ProcessLeafs: funny lump size in %s", loadmodel->name);

	count					= filelen / sizeof(*in);

//	out = Hunk_AllocName ( count*sizeof(*out), "USE_LEAF");
	out						= Hunk_AllocName (count, sizeof(*out), loadname);
#if FITZQUAKE_PROTOCOL
	//johnfitz
	if (count > MAX_MAP_LEAFS)				Host_Error ("Mod_LoadLeafs: %i leafs exceeds limit of %i.\n", count, MAX_MAP_LEAFS);
	if (count > MAX_WINQUAKE_MAP_LEAFS)		Con_Warning ("Mod_LoadLeafs: %i leafs exceeds standard limit of %i.\n", count, MAX_WINQUAKE_MAP_LEAFS);

	//johnfitz
#endif

	loadmodel->leafs		= out;
	loadmodel->numleafs		= count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}

		p					= LittleLong(in->contents);
		out->contents		= p;

#if FITZQUAKE_PROTOCOL
		out->firstmarksurface = loadmodel->marksurfaces + (unsigned short)LittleShort(in->firstmarksurface); //johnfitz -- unsigned short
		out->nummarksurfaces = (unsigned short)LittleShort(in->nummarksurfaces); //johnfitz -- unsigned short
#else
		out->firstmarksurface = loadmodel->marksurfaces + LittleShort (in->firstmarksurface);
		out->nummarksurfaces = LittleShort(in->nummarksurfaces);
#endif

		p					= LittleLong(in->visofs);
		out->compressed_vis = (p == -1) ?  NULL :  loadmodel->visdata + p;
		out->efrags			= NULL;

		for (j=0 ; j<4 ; j++)
			out->ambient_sound_level[j] = in->ambient_level[j];

#ifdef GLQUAKE
		// gl underwater warp
		// Baker: This marks the surface as underwater
		if (out->contents != CONTENTS_EMPTY)
		{
			for (j=0 ; j<out->nummarksurfaces ; j++)
				out->firstmarksurface[j]->flags |= SURF_UNDERWATER;
		}
#endif
	}
}

/*
=================
Mod_LoadLeafs
=================
*/
static void Mod_LoadLeafs (lump_t *l)
{
	dleaf_t 	*in = (void *)(mod_base + l->fileofs);

	Mod_ProcessLeafs (in, l->filelen);
}


#else
/*
=================
Mod_LoadLeafs
=================
*/
static void Mod_LoadLeafs (lump_t *l)
{
	dleaf_t 	*in			= (void *)(mod_base + l->fileofs);

	mleaf_t 	*out;
	int			i, j, count, p;
	
	if (l->filelen % sizeof(*in))			Host_Error ("Mod_LoadLeafs: funny lump size in %s", loadmodel->name);

	count					= l->filelen / sizeof(*in);
	out						= Hunk_AllocName (count, sizeof(*out), loadname);
#if FITZQUAKE_PROTOCOL
	//johnfitz
	if (count > MAX_MAP_LEAFS)
		Host_Error ("Mod_LoadLeafs: %i leafs exceeds limit of %i.\n", count, MAX_MAP_LEAFS);
	if (count > MAX_WINQUAKE_MAP_LEAFS)
		Con_Warning ("Mod_LoadLeafs: %i leafs exceeds standard limit of %i.\n", count, MAX_WINQUAKE_MAP_LEAFS);
#pragma message ("Quality assurance: Validate that 8192 or 32767 is MAX_WINQUAKE_MAP_LEAFS in WinQuake")
	//johnfitz
#endif

	loadmodel->leafs		= out;
	loadmodel->numleafs		= count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}

		p = LittleLong(in->contents);
		out->contents = p;

#if FITZQUAKE_PROTOCOL
		out->firstmarksurface = loadmodel->marksurfaces + (unsigned short)LittleShort(in->firstmarksurface); //johnfitz -- unsigned short
		out->nummarksurfaces = (unsigned short)LittleShort(in->nummarksurfaces); //johnfitz -- unsigned short
#else
		out->firstmarksurface = loadmodel->marksurfaces + LittleShort (in->firstmarksurface);
		out->nummarksurfaces = LittleShort(in->nummarksurfaces);
#endif

		p = LittleLong(in->visofs);
		out->compressed_vis = (p == -1) ?  NULL :  loadmodel->visdata + p;
		out->efrags			= NULL;

		for (j=0 ; j<4 ; j++)
			out->ambient_sound_level[j] = in->ambient_level[j];

		// gl underwater warp
		if (out->contents != CONTENTS_EMPTY)
		{
			for (j=0 ; j<out->nummarksurfaces ; j++)
				out->firstmarksurface[j]->flags |= SURF_UNDERWATER;
		}
	}
}
#endif

/*
=================
Mod_LoadClipnodes
=================
*/
static void Mod_LoadClipnodes (lump_t *l)
{
	dclipnode_t		*in		= (void *)(mod_base + l->fileofs);

	mclipnode_t		*out;											 //johnfitz -- was dclipnode_t
	int				i, count;
	hull_t			*hull;

	if (l->filelen % sizeof(*in))		Host_Error ("Mod_LoadClipnodes: funny lump size in %s", loadmodel->name);

	count					= l->filelen / sizeof(*in);
	out						= Hunk_AllocName (count, sizeof(*out), loadname);

#if FITZQUAKE_PROTOCOL
	//johnfitz -- warn about exceeding old limits
//	if (count > 32767)
//		Con_Warning ("%i clipnodes exceeds standard limit of 32767.\n", count);
	if (count > MAX_WINQUAKE_MAP_CLIPNODES)
		Con_Warning ("%i clipnodes exceeds standard limit of %i.\n", count, MAX_WINQUAKE_MAP_CLIPNODES);
	//johnfitz
#endif

	loadmodel->clipnodes	= out;
	loadmodel->numclipnodes = count;

	// Player Hull
	hull					= &loadmodel->hulls[1];
	hull->clipnodes			= out;
	hull->firstclipnode		= 0;
	hull->lastclipnode		= count-1;
	hull->planes			= loadmodel->planes;

	VectorSet (hull->clip_mins, -16, -16, -24);
	VectorSet (hull->clip_maxs,  16,  16,  32);

	hull->available			= true;

#if SUPPORTS_HLBSP
	if (loadmodel->bspversion == HL_BSPVERSION)
	{	// Player hull is taller
		hull->clip_mins[2]	= -36;
		hull->clip_maxs[2]	= 36;
	}
	else
#endif
	if (game_kurok.integer)
	{  // Player hull is skinnier
		hull->clip_mins[0]	= -12;
		hull->clip_mins[1]	= -12;

		hull->clip_maxs[0]	= 12;
		hull->clip_maxs[1]	= 12;
	}

	// Monster hull
	hull					= &loadmodel->hulls[2];
	hull->clipnodes			= out;
	hull->firstclipnode		= 0;
	hull->lastclipnode		= count-1;
	hull->planes			= loadmodel->planes;

	VectorSet (hull->clip_mins, -32, -32, -24);
	VectorSet (hull->clip_maxs,  32,  32,  64);

	hull->available			= true;

#if SUPPORTS_HLBSP
	if (loadmodel->bspversion == HL_BSPVERSION)
	{
		hull->clip_mins[2]	= -32;
		hull->clip_maxs[2]	= 32;
	}
#endif

	// Half-Life crouch hull
	hull					= &loadmodel->hulls[3];
	hull->clipnodes			= out;
	hull->firstclipnode		= 0;
	hull->lastclipnode		= count-1;
	hull->planes			= loadmodel->planes;

	VectorSet (hull->clip_mins, -16, -16,  -6);
	VectorSet (hull->clip_maxs,  16,  16,  30);

	hull->available			= false;

#if SUPPORTS_HLBSP
	if (loadmodel->bspversion == HL_BSPVERSION)
	{
		hull->clip_mins[2]	= -18;
		hull->clip_maxs[2]	= 18;
		hull->available		= true;
	}
#endif

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->planenum = LittleLong(in->planenum);

#if FITZQUAKE_PROTOCOL
		//johnfitz -- bounds check
		if (out->planenum < 0 || out->planenum >= loadmodel->numplanes)
			Host_Error ("Mod_LoadClipnodes: planenum out of bounds");
		//johnfitz

		//johnfitz -- support clipnodes > 32k
		out->children[0]	= (unsigned short)LittleShort(in->children[0]);
		out->children[1]	= (unsigned short)LittleShort(in->children[1]);

		if (out->children[0] >= count)
			out->children[0] -= 65536;
		if (out->children[1] >= count)
			out->children[1] -= 65536;
		//johnfitz
#else
		out->children[0]	= LittleShort(in->children[0]);
		out->children[1]	= LittleShort(in->children[1]);
#endif
	}
}

/*
=================
Mod_MakeHull0

Duplicate the drawing hull structure as a clipping hull
=================
*/
static void Mod_MakeHull0 (void)
{
	hull_t		*hull		= &loadmodel->hulls[0];
	mnode_t		*in			= loadmodel->nodes;
	int			count		= loadmodel->numnodes;
	mclipnode_t *out		= Hunk_AllocName (count, sizeof(*out), loadname);   //johnfitz -- was dclipnode_t

	mnode_t		*child;
	int			i, j;

	hull->clipnodes			= out;
	hull->firstclipnode		= 0;
	hull->lastclipnode		= count - 1;
	hull->planes			= loadmodel->planes;

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->planenum = in->plane - loadmodel->planes;
		for (j=0 ; j<2 ; j++)
		{
			child			= in->children[j];
			if (child->contents < 0)
				out->children[j] = child->contents;
			else
				out->children[j] = child - loadmodel->nodes;
		}
	}
}

/*
=================
Mod_LoadMarksurfaces
=================
*/
static void Mod_LoadMarksurfaces (lump_t *l)
{
	short		*in				= (void *)(mod_base + l->fileofs);

	int			i, j, count;
	msurface_t	**out;

	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadMarksurfaces: funny lump size in %s", loadmodel->name);

	count						= l->filelen / sizeof(*in);
	out							= Hunk_AllocName (count, sizeof(*out), loadname);

	loadmodel->marksurfaces		= out;
	loadmodel->nummarksurfaces	= count;

#if FITZQUAKE_PROTOCOL
	//johnfitz -- warn mappers about exceeding old limits
//	if (count > 32767)
//		Con_Warning ("%i marksurfaces exceeds standard limit of 32767.\n", count);

	if (count > MAX_WINQUAKE_MAP_MARKSURFACES)
		Con_Warning ("%i marksurfaces exceeds standard limit of %i.\n", count, MAX_WINQUAKE_MAP_MARKSURFACES);

	//johnfitz
#endif

	for (i=0 ; i<count ; i++)
	{
#if FITZQUAKE_PROTOCOL
		j = (unsigned short)LittleShort(in[i]); //johnfitz -- explicit cast as unsigned short
#else
		j = LittleShort(in[i]);
#endif
		if (j >= loadmodel->numsurfaces)
			Host_Error ("Mod_LoadMarksurfaces: bad surface number");
		out[i] = loadmodel->surfaces + j;
	}
}

/*
=================
Mod_LoadSurfedges
=================
*/
static void Mod_LoadSurfedges (lump_t *l)
{
	int		*in				= (void *)(mod_base + l->fileofs);

	int		i, count, *out;

	if (l->filelen % sizeof(*in))	Host_Error ("Mod_LoadSurfedges: funny lump size in %s", loadmodel->name);

	count					= l->filelen / sizeof(*in);
	out						= Hunk_AllocName (count, sizeof(*out), loadname);

	loadmodel->surfedges	= out;
	loadmodel->numsurfedges = count;

	for (i=0 ; i<count ; i++)
		out[i] = LittleLong (in[i]);
}


/*
=================
Mod_LoadPlanes
=================
*/
static void Mod_LoadPlanes (lump_t *l)
{
	int			i, j, count, bits;
	mplane_t	*out;
	dplane_t 	*in			= (void *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))	Host_Error ("Mod_LoadPlanes: funny lump size in %s", loadmodel->name);

	count					= l->filelen / sizeof(*in);
	out						= Hunk_AllocName (count, 2 * sizeof(*out), loadname);

	loadmodel->planes		= out;
	loadmodel->numplanes	= count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1 << j;
		}

		out->dist		= LittleFloat (in->dist);
		out->type		= LittleLong (in->type);
		out->signbits	= bits;
	}
}

//#define	ISTELETEX(name)	((name)[0] == '*' && (name)[1] == 't' && (name)[2] == 'e' && (name)[3] == 'l' && (name)[4] == 'e')
#if 0 // Baker: This doesn't quite work
/*
=================
Mod_DetectWaterTrans

detect if a model has been vised for translucent water
=================
*/
qbool Mod_DetectWaterTrans (model_t *mod)
{
	int		i, j;
	byte		*vis;
	mleaf_t		*leaf;
	msurface_t	**surf;

	// no visdata
	if (!mod->visdata)
		return true;

	for (i = 0 ; i < mod->numleafs ; i++)
	{
		// leaf 0 is the solid leaf, leafs go to numleafs + 1
		leaf = &mod->leafs[i+1];

		// not interested in these leafs
		if (leaf->contents >= CONTENTS_EMPTY || leaf->contents == CONTENTS_SOLID || leaf->contents == CONTENTS_SKY)
			continue;

		// check marksurfaces for a water texture
		surf = leaf->firstmarksurface;

		for (j = 0 ; j < leaf->nummarksurfaces ; j++, surf++)
		{
			// bad surf/texinfo/texture (some old maps have this from a bad qbsp)
			if (!surf || !(*surf) || !(*surf)->texinfo || !(*surf)->texinfo->texture)
				continue;

			// not interested in teleports
			if (!ISTELETEX((*surf)->texinfo->texture->name))
				goto LeafOK;
		}

		// no water/etc textures here
		continue;

LeafOK:
		// get the decompressed vis
		vis = Mod_DecompressVis (leaf->compressed_vis, mod);

		// check the other leafs
		for (j = 0 ; j < mod->numleafs ; j++)
		{
			// in the PVS
			if (vis[j>>3] & (1 << (j & 7)))
			{
				mleaf_t	*visleaf = &mod->leafs[j + 1];

				// the leaf we hit originally was under water/slime/lava, and a
				// leaf in it's pvs is above water/slime/lava.
				if (visleaf->contents == CONTENTS_EMPTY)
					return true;
			}
		}
	}

	// found nothing
	return false;
}
#endif


#if SUPPORTS_EXTERNAL_VIS


// 2001-12-28 .VIS support by Maddes  start
/*
=================
Mod_LoadExternalVisibility
=================
*/
static void Mod_LoadVisibilityExternal (FILE **fhandle)
{
	long	filelen=0;;

	// get visibility data length
	filelen = 0;
//	Sys_FileRead (fhandle, &filelen, 4);
	fread (&filelen, 1, 4, *fhandle);
	filelen = LittleLong(filelen);

	Con_DevPrintf(DEV_MODEL, "...%i bytes visibility data\n", filelen);

	// load visibility data
	if (!filelen)
	{
		loadmodel->visdata = NULL;
		return;
	}
	loadmodel->visdata = Hunk_AllocName (1, filelen, "EXT_VIS");
//	Sys_FileRead (fhandle, loadmodel->visdata, filelen);
	fread (loadmodel->visdata, 1, filelen, *fhandle);
}

/*
=================
Mod_LoadExternalLeafs
=================
*/
static void Mod_LoadLeafsExternal (FILE **fhandle)
{
	dleaf_t 	*in;
	long	filelen;

	// get leaf data length
	filelen = 0;
//	Sys_FileRead (fhandle, &filelen, 4);
	fread (&filelen, 1, 4, *fhandle);
	filelen = LittleLong(filelen);

	Con_DevPrintf(DEV_MODEL, "...%i bytes leaf data\n", filelen);

	// load leaf data
	if (!filelen)
	{
		loadmodel->leafs = NULL;
		loadmodel->numleafs = 0;
		return;
	}
	in = Hunk_AllocName (1, filelen, "EXT_LEAF");
//	Sys_FileRead (fhandle, in, filelen); --->  fread (dest, 1, count, handle)
	fread  (in, 1, filelen, *fhandle);

	Mod_ProcessLeafs (in, filelen);
}

// 2001-12-28 .VIS support by Maddes  start
#define VISPATCH_MAPNAME_LENGTH	32

typedef struct vispatch_s {
	char	mapname[VISPATCH_MAPNAME_LENGTH];	// map for which these data are for, always use strncpy and strncmp for this field
	int		filelen;		// length of data after VisPatch header (VIS+Leafs)
} vispatch_t;
// 2001-12-28 .VIS support by Maddes  end

static qbool Mod_FindVisibilityExternal (FILE **filehandle)
{
	char		visfilename[MAX_QPATH];

	snprintf (visfilename, sizeof(visfilename), "maps/%s.vis", sv.worldname /*loadmodel->name*/);	// Baker: be careful
	QFS_FOpenFile (visfilename, filehandle, loadmodel->loadinfo.searchpath); // COM_FOpenFile (name, &cls.demofile);

	if (!*filehandle)	return false;		// None

	{
		int			i, pos;
		vispatch_t	header;
		char		mapname[VISPATCH_MAPNAME_LENGTH+5];	// + ".vis" + EoS

		// search map in visfile

		COM_Copy_StripExtension			(loadname, mapname);
		COMD_ForceExtension	(mapname,  ".bsp" );

//		strncpy(mapname, loadname, VISPATCH_MAPNAME_LENGTH);
//		strcat(mapname, ".bsp");

		pos = 0;

		while ((i = fread (&header, 1, sizeof(struct vispatch_s), *filehandle))) // i will be length of read, continue while a read
		{
			header.filelen = LittleLong(header.filelen);	// Endian correct header.filelen
			pos += i;										// Advance the length of the break

			if (COM_StringMatchCaseless (header.mapname, loadmodel->name));	// found, get out
			{
				break;
			}

			pos += header.filelen;							// Advance the length of the filelength
			fseek (*filehandle, pos, SEEK_SET);
		}

		if (i != sizeof(struct vispatch_s))
		{
			fclose (*filehandle);
			return false; // Baker check visfilehandle here
		}
	}

	return true;

}
// 2001-12-28 .VIS support by Maddes  end

#endif

// sWorldCheck

// We might be dedicated server and setting cl info, but NetQuake doesn't have dedicated server only build.
// And we still might want software renderer in such a dedicated server type of build.  Not to render, but to
// be able to calculate lighting levels.  You could use some 3D calculations to determine if player is fully
// darkened for purposes of not sending the entity, for game logic involving hiding from monsters, or
// special capabilities or dangers in the dark (solar powered devices, likely to be killed by a grue, etc.)
// We'd need software lightmaps for that.

// Furthermore, we might be able to do some fake rendering and write to file a picture of what is going on
// on the server every once in a while.  Anyway, Baker dreaming on ....




#pragma message ("Quality assurance: We need some better method to check this.  Kurok start map or Travail start map isn't id1.  Still, Frikbot start map is id1 ... hmmm")
#pragma message ("So can't go on gamedir, but maybe rather loading folder.  Not really a big deal due to path limiters.")




/*
=================
Mod_LoadBrushModel
=================
*/
qbool GameHacks_Is_EXMY_Map (const char *mapname);
void Mod_LoadModel_Brush (model_t *mod, void *buffer)
{
	int			i, j;
	dheader_t	*header;
	dmodel_t 	*bm;

	loadmodel->modelformat = mod_brush;

	header = (dheader_t *)buffer;

	mod->bspversion = LittleLong (header->version);

	if (mod->bspversion != Q1_BSPVERSION && mod->bspversion != HL_BSPVERSION)
		Host_Error ("Mod_LoadBrushModel: %s has wrong version number (%i should be %i (Quake) or %i (HalfLife))", mod->name, mod->bspversion, Q1_BSPVERSION, HL_BSPVERSION);


	if (loadmodel->isworldmodel = COM_StringMatch (loadname, host_worldname))
		cl.original_map_for_exmy_folder_textures = GameHacks_Is_EXMY_Map(host_worldname);

// swap all the lumps
	mod_base = (byte *)header;

	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)header)[i] = LittleLong (((int *)header)[i]);

// load into heap
	Mod_LoadVertexes (&header->lumps[LUMP_VERTEXES]);
	if (loadmodel->isworldmodel)
		Con_DevPrintf (DEV_MODEL, "World bounds: %4.1f %4.1f %4.1f to %4.1f %4.1f %4.1f\n", cl.worldmins[0], cl.worldmins[1], cl.worldmins[2], cl.worldmaxs[0], cl.worldmaxs[1], cl.worldmaxs[2]);

	Mod_LoadEdges (&header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges (&header->lumps[LUMP_SURFEDGES]);
	Mod_LoadTextures_Brush (&header->lumps[LUMP_TEXTURES]);
	Mod_LoadLighting (&header->lumps[LUMP_LIGHTING]);
	Mod_LoadPlanes (&header->lumps[LUMP_PLANES]);
	Mod_LoadTexinfo (&header->lumps[LUMP_TEXINFO]);
	Mod_LoadFaces (&header->lumps[LUMP_FACES]);
	Mod_LoadMarksurfaces (&header->lumps[LUMP_MARKSURFACES]);

	{
#if SUPPORTS_EXTERNAL_VIS
		// 2001-12-28 .VIS support by Maddes  start
		FILE		*visfhandle;	// Baker: try to localize this var

		loadmodel->visdata = NULL;
		loadmodel->leafs = NULL;
		loadmodel->numleafs = 0;

		if (loadmodel->isworldmodel && external_vis.integer)
		{
			Con_DevPrintf (DEV_MODEL, "model_loadvisfiles so trying to open external vis file\n");

			if (Mod_FindVisibilityExternal (&visfhandle /* We should be passing infos here &visfilehandle*/) )	// File exists, valid and open
			{
				Con_DevPrintf (DEV_MODEL, "found valid external .vis file for map\n");
				Mod_LoadVisibilityExternal (&visfhandle);
				Mod_LoadLeafsExternal (&visfhandle);

				fclose  (visfhandle);

				if ((loadmodel->visdata == NULL) || (loadmodel->leafs == NULL) || (loadmodel->numleafs == 0))
				{
					Con_Printf("External VIS data are invalid!!!\n");
					goto do_standard_vis;
				}

				Con_Printf ("External .vis file: Using entfile maps/%s.vis\n", sv.worldname);
				goto skip_standard_vis;		// Success

			}
		}

do_standard_vis:
		// 2001-12-28 .VIS support by Maddes  end
	#endif

		Mod_LoadVisibility (&header->lumps[LUMP_VISIBILITY]);
		Mod_LoadLeafs (&header->lumps[LUMP_LEAFS]);
	}


#if SUPPORTS_EXTERNAL_VIS	// Baker: simply to avoid compiler warning
skip_standard_vis:
#endif

	Mod_LoadNodes (&header->lumps[LUMP_NODES]);
	Mod_LoadClipnodes (&header->lumps[LUMP_CLIPNODES]);
	Mod_LoadEntities (&header->lumps[LUMP_ENTITIES]);
	Mod_LoadSubmodels (&header->lumps[LUMP_MODELS]);

	Mod_MakeHull0 ();

//	Con_Printf ("Has transparent is %d\n", Mod_DetectWaterTrans (mod));
	mod->numframes = 2;		// regular and alternate animation

// set up the submodels (FIXME: this is confusing)


	// johnfitz -- okay, so that i stop getting confused every time i look at this loop, here's how it works:
	// we're looping through the submodels starting at 0.  Submodel 0 is the main model, so we don't have to
	// worry about clobbering data the first time through, since it's the same data.  At the end of the loop,
	// we create a new copy of the data to use the next time through.
	for (i = 0 ; i < mod->numsubmodels ; i++)
	{
		bm = &mod->submodels[i];

		mod->hulls[0].firstclipnode = bm->headnode[0];
		for (j=1 ; j<MAX_MAP_HULLS ; j++)
		{
			mod->hulls[j].firstclipnode = bm->headnode[j];
			mod->hulls[j].lastclipnode = mod->numclipnodes - 1;
		}

		mod->firstmodelsurface = bm->firstface;
		mod->nummodelsurfaces = bm->numfaces;

		VectorCopy (bm->maxs, mod->maxs);
		VectorCopy (bm->mins, mod->mins);

		mod->radius = RadiusFromBounds (mod->mins, mod->maxs);

		mod->numleafs = bm->visleafs;

		if (i < mod->numsubmodels - 1)
		{	// duplicate the basic information
			model_t *Mod_FindName (const char *name);	// extern function
			char	name[10];

			snprintf (name, sizeof(name), "*%i", i+1);
			loadmodel = Mod_FindName (name);
			*loadmodel = *mod;
			strcpy (loadmodel->name, name);
			mod = loadmodel;
		}
	}
}
