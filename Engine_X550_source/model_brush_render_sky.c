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
// model_brush_render_sky.c -- sky textures


#include "quakedef.h"

#define	MAX_CLIP_VERTS	64


extern	msurface_t	*skychain;
extern	msurface_t	**skychain_tail;

static	float	skymins[2][6], skymaxs[2][6];
static	float	speedscale;	// for top sky and bottom sky
static  float   speedscale2;

qbool	r_skyboxloaded;


static	int	skytexorder[6] = {0, 2, 1, 3, 4, 5}; //for skybox

static	vec3_t	skyclip[6] = {
	{1, 1, 0},
	{1, -1, 0},
	{0, -1, 1},
	{0, 1, 1},
	{1, 0, 1},
	{-1, 0, 1}
};

// 1 = s, 2 = t, 3 = 2048
static	int	st_to_vec[6][3] =
{
	{3, -1, 2},
	{-3, 1, 2},
	{1, 3, 2},
	{-1, -3, 2},
	{-2, -1, 3},		// look straight up
	{2, -1, -3}			// look straight down
};

// s = [0]/[2], t = [1]/[2]
static	int	vec_to_st[6][3] =
{
	{-2, 3, 1},
	{2, 3, -1},

	{1, 3, 2},
	{-1, 3, -2},

	{-2, -1, 3},
	{-2, 1, -3}
};




/*
=============
Sky_OldSky_EmitSkyPolys
=============
*/
static void Sky_OldSky_EmitSkyPolys (msurface_t *fa, qbool mtex)
{
	glpoly_t	*p;
	float		*v;
	int			i;
	float		s, t;
	vec3_t		dir;
	float		length;
	float		ss, tt;

	for (p = fa->polys ; p ; p = p->next)
	{
		eglBegin (GL_POLYGON);
		for (i = 0, v = p->verts[0] ; i < p->numverts ; i++, v += VERTEXSIZE)
		{
			VectorSubtract (v, r_origin, dir);
			dir[2] *= 3;	// flatten the sphere

			length = VectorLength (dir);
			length = 6 * 63 / length;

			dir[0] *= length;
			dir[1] *= length;

			if (mtex)
			{
				s = (speedscale + dir[0]) * (1.0 / 128);
				t = (speedscale + dir[1]) * (1.0 / 128);

				ss = (speedscale2 + dir[0]) * (1.0 / 128);
				tt = (speedscale2 + dir[1]) * (1.0 / 128);
			}
			else
			{
				s = (speedscale + dir[0]) * (1.0 / 128);
				t = (speedscale + dir[1]) * (1.0 / 128);
			}

			if (mtex)
			{
				qglMultiTexCoord2f (GL_TEXTURE0_ARB, s, t);
				qglMultiTexCoord2f (GL_TEXTURE1_ARB, ss, tt);
			}
			else
			{
				eglTexCoord2f (s, t);
			}

			eglVertex3fv (v);
		}
		eglEnd ();
	}
}

static void Sky_FastSky_EmitFlatPoly (msurface_t *fa)
{
	glpoly_t	*p;
	float		*v;
	int		i;

	for (p = fa->polys ; p ; p = p->next)
	{
		eglBegin (GL_POLYGON);
		for (i = 0, v = p->verts[0] ; i < p->numverts ; i++, v += VERTEXSIZE)
			eglVertex3fv (v);
		eglEnd ();
	}
}



/*
=================
R_DrawSkyChain
=================
*/
void Sky_OldSky_DrawChain (void)
{
	msurface_t	*fa;
	byte		*col;

	if (!skychain)
		return;

	GL_DisableMultitexture ();

#if SUPPORTS_HLBSP
	if (r_fastsky.integer || cl.worldmodel->bspversion == HL_BSPVERSION)
	{
#else
	if (r_fastsky.integer)
	{
#endif
		MeglDisable (GL_TEXTURE_2D);

		col = StringToRGB (r_skycolor.string);
		MeglColor3ubv (col);

		for (fa = skychain ; fa ; fa = fa->texturechain)
			Sky_FastSky_EmitFlatPoly (fa);

		MeglEnable (GL_TEXTURE_2D);
		MeglColor3ubv (color_white);
	}
	else
	{
		if (gl_mtexable)
		{
			GL_Bind (solidskytexture);
			MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			GL_EnableMultitexture ();
			GL_Bind (alphaskytexture);
			MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

			speedscale = cl.ctime * 8;
			speedscale -= (int)speedscale & ~127;
			speedscale2 = cl.ctime * 16;
			speedscale2 -= (int)speedscale2 & ~127;

			for (fa = skychain ; fa ; fa = fa->texturechain)
				Sky_OldSky_EmitSkyPolys (fa, true);

			GL_DisableMultitexture ();
		}
		else
		{
			GL_Bind (solidskytexture);
			speedscale = cl.ctime * 8;
			speedscale -= (int)speedscale & ~127;

			for (fa = skychain ; fa ; fa = fa->texturechain)
				Sky_OldSky_EmitSkyPolys (fa, false);

			MeglEnable (GL_BLEND);
			GL_Bind (alphaskytexture);
			speedscale = cl.ctime * 16;
			speedscale -= (int)speedscale & ~127;

			for (fa = skychain ; fa ; fa = fa->texturechain)
				Sky_OldSky_EmitSkyPolys (fa, false);

			MeglDisable (GL_BLEND);
		}
	}

	skychain = NULL;
	skychain_tail = &skychain;
}

//===============================================================

/*
=============
Sky_OldSky_Load_NewMap

A sky texture is 256*128, with the right side being a masked overlay
==============
*/
void Sky_OldSky_Load_NewMap (const byte *qpal_src)
{
	int			i, j, p;
	int			r, g, b;
	unsigned	trans[128*128];
	unsigned	transpix;
	unsigned	*rgba;

//	src = (byte *)mt + mt->offsets[0];

	// make an average value for the back to avoid
	// a fringe on the top level

	r = g = b = 0;
	for (i=0 ; i<128 ; i++)
		for (j=0 ; j<128 ; j++)
		{
			p = qpal_src[i*256 + j + 128];
			rgba = &d_8to24table[p];
			trans[(i*128) + j] = *rgba;
			r += ((byte *)rgba)[0];
			g += ((byte *)rgba)[1];
			b += ((byte *)rgba)[2];
		}

	((byte *)&transpix)[0] = r / (128 * 128);
	((byte *)&transpix)[1] = g / (128 * 128);
	((byte *)&transpix)[2] = b / (128 * 128);
	((byte *)&transpix)[3] = 0;


	GL_Bind (solidskytexture);
	eglTexImage2D (GL_TEXTURE_2D, 0, gl_solid_bytes_per_pixel, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans); // UPLOAD ROGUE
	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (i=0 ; i<128 ; i++)
		for (j=0 ; j<128 ; j++)
		{
			p = qpal_src[i*256 + j];
			if (p == 0)
				trans[(i * 128) + j] =  transpix;
			else
				trans[(i * 128) + j] =  d_8to24table[p];
		}


	GL_Bind (alphaskytexture);
	eglTexImage2D (GL_TEXTURE_2D, 0, gl_alpha_bytes_per_pixel, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);  // UPLOAD ROGUE
	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static	char	*skybox_ext[6] = {"rt", "bk", "lf", "ft", "up", "dn"};

/*
==================
R_SetSky
==================
*/
static int Sky_SkyBox_SetSky (const char *skyname)
{
	int	i, error = 0;
	int	iwidth;
	int iheight;
	byte	*data[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

	ImageWork_Start ("Skybox", skyname);
	for (i=0 ; i<6 ; i++)
	{
		if (!(data[i] = GL_LoadExternalImage_RGBAPixels(va("gfx/env/%s%s", skyname, skybox_ext[i]), &iwidth, &iheight, 0, NULL /*PATH LIMIT ME*/)) &&
		    !(data[i] = GL_LoadExternalImage_RGBAPixels(va("gfx/env/%s_%s", skyname, skybox_ext[i]), &iwidth, &iheight, 0, NULL /* cl.worldmodel->loadinfo.searchpath */ /*PATH LIMIT ME*/)))
		{
			Con_Printf ("Couldn't load skybox \"%s\"\n", skyname);
			error = 1;

			goto cleanup;
		}
	}
	for (i=0 ; i<6 ; i++)
	{
		GL_Bind (skyboxtextures + i);
		GL_Upload32 ((unsigned int *)data[i], iwidth, iheight, 0);
	}
	r_skyboxloaded = true;

cleanup:
	for (i=0 ; i<6 ; i++)
	{
		if (data[5-i])	// LIFO
			ImageWork_free (data[i]);
		else
			break;
	}

	ImageWork_Finish ();
	return error;
}

qbool OnChange_r_skybox (cvar_t *var, const char *string)
{
	if (!string[0])
	{
		r_skyboxloaded = false;
		return false;
	}

	if (nehahra)
	{
		Cvar_SetFloatByRef (&r_oldsky, 0);
		StringLCopy (prev_skybox, string);
	}

	Sky_SkyBox_SetSky (string);
	return !r_skyboxloaded;
}

static void DrawSkyPolygon (int nump, vec3_t vecs)
{
	int	i, j, axis;
	float	s, t, dv, *vp;
	vec3_t	v, av;

	// decide which face it maps to
	VectorClear (v);
	for (i = 0, vp = vecs ; i < nump ; i++, vp += 3)
		VectorAdd (vp, v, v);
	av[0] = fabsf(v[0]);
	av[1] = fabsf(v[1]);
	av[2] = fabsf(v[2]);
	if (av[0] > av[1] && av[0] > av[2])
		axis = (v[0] < 0) ? 1 : 0;
	else if (av[1] > av[2] && av[1] > av[0])
		axis = (v[1] < 0) ? 3 : 2;
	else
		axis = (v[2] < 0) ? 5 : 4;

	// project new texture coords
	for (i=0 ; i<nump ; i++, vecs+=3)
	{
		j = vec_to_st[axis][2];
		dv = (j > 0) ? vecs[j - 1] : -vecs[-j - 1];

		j = vec_to_st[axis][0];
		s = (j < 0) ? -vecs[-j -1] / dv : vecs[j-1] / dv;

		j = vec_to_st[axis][1];
		t = (j < 0) ? -vecs[-j -1] / dv : vecs[j-1] / dv;

		if (s < skymins[0][axis])
			skymins[0][axis] = s;
		if (t < skymins[1][axis])
			skymins[1][axis] = t;
		if (s > skymaxs[0][axis])
			skymaxs[0][axis] = s;
		if (t > skymaxs[1][axis])
			skymaxs[1][axis] = t;
	}
}


static void Sky_SkyBox_ClipPoly (int nump, vec3_t vecs, int stage)
{
	float		*norm, *v, d, e, dists[MAX_CLIP_VERTS];
	qbool	front, back;
	int		sides[MAX_CLIP_VERTS], newc[2], i, j;
	vec3_t		newv[2][MAX_CLIP_VERTS];

	if (nump > MAX_CLIP_VERTS-2)
		Sys_Error ("Sky_SkyBox_ClipPoly: nump > MAX_CLIP_VERTS - 2");

	if (stage == 6)
	{	// fully clipped, so draw it
		DrawSkyPolygon (nump, vecs);
		return;
	}

	front = back = false;
	norm = skyclip[stage];
	for (i = 0, v = vecs ; i < nump ; i++, v += 3)
	{
		d = DotProduct (v, norm);
		if (d > ON_EPSILON)
		{
			front = true;
			sides[i] = SIDE_FRONT;
		}
		else if (d < -ON_EPSILON)
		{
			back = true;
			sides[i] = SIDE_BACK;
		}
		else
			sides[i] = SIDE_ON;
		dists[i] = d;
	}

	if (!front || !back)
	{	// not clipped
		Sky_SkyBox_ClipPoly (nump, vecs, stage+1);
		return;
	}

	// clip it
	sides[i] = sides[0];
	dists[i] = dists[0];
	VectorCopy (vecs, (vecs + (i*3)));
	newc[0] = newc[1] = 0;

	for (i=0, v=vecs ; i<nump ; i++, v+=3)
	{
		switch (sides[i])
		{
		case SIDE_FRONT:
			VectorCopy (v, newv[0][newc[0]]);
			newc[0]++;
			break;

		case SIDE_BACK:
			VectorCopy (v, newv[1][newc[1]]);
			newc[1]++;
			break;

		case SIDE_ON:
			VectorCopy (v, newv[0][newc[0]]);
			newc[0]++;
			VectorCopy (v, newv[1][newc[1]]);
			newc[1]++;
			break;
		}

		if (sides[i] == SIDE_ON || sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;

		d = dists[i] / (dists[i] - dists[i+1]);
		for (j=0 ; j<3 ; j++)
		{
			e = v[j] + d*(v[j+3] - v[j]);
			newv[0][newc[0]][j] = e;
			newv[1][newc[1]][j] = e;
		}
		newc[0]++;
		newc[1]++;
	}

	// continue
	Sky_SkyBox_ClipPoly (newc[0], newv[0][0], stage+1);
	Sky_SkyBox_ClipPoly (newc[1], newv[1][0], stage+1);
}

/*
=================
R_AddSkyBoxSurface
=================
*/
static void Sky_SkyBox_AddSurface (msurface_t *fa)
{
	int		i;
	vec3_t		verts[MAX_CLIP_VERTS];
	glpoly_t	*p;

	// calculate vertex values for sky box
	for (p = fa->polys ; p ; p = p->next)
	{
		for (i=0 ; i<p->numverts ; i++)
			VectorSubtract (p->verts[i], r_origin, verts[i]);
		Sky_SkyBox_ClipPoly (p->numverts, verts[0], 0);
	}
}

/*
==============
Sky_SkyBox_Clear
==============
*/
static void Sky_SkyBox_Clear (void)
{
	int	i;

	for (i=0 ; i<6 ; i++)
	{
		skymins[0][i] = skymins[1][i] = 9999;
		skymaxs[0][i] = skymaxs[1][i] = -9999;
	}
}

void Sky_SkyBox_EmitVertex (float s, float t, int axis)
{
	int	j, k, farclip;
	vec3_t		v, b;

	farclip = max((int)scene_farclip.floater, 4096);
	b[0] = s * (farclip >> 1);
	b[1] = t * (farclip >> 1);
	b[2] = (farclip >> 1);

	for (j=0 ; j<3 ; j++)
	{
		k = st_to_vec[axis][j];
		if (k < 0)
			v[j] = -b[-k-1];
		else
			v[j] = b[k-1];
		v[j] += r_origin[j];
	}

	// convert from range [-1,1] to [0,1]
	s = (s+1) * 0.5;
	t = (t+1) * 0.5;

	// avoid bilerp seam
	if (s < 1.0/512)
		s = 1.0 / 512;
	else if (s > 511.0/512)
		s = 511.0 / 512;
	if (t < 1.0/512)
		t = 1.0 / 512;
	else if (t > 511.0/512)
		t = 511.0 / 512;

	t = 1.0 - t;
	eglTexCoord2f (s, t);
	eglVertex3fv (v);
}


/*
==============
Sky_SkyBox_Draw
==============
*/
void Sky_SkyBox_Draw (void)
{
	int		i;
	msurface_t	*fa;

	if (!skychain)
		return;

	Sky_SkyBox_Clear ();	// Clears skynox mins and maxes
	for (fa = skychain ; fa ; fa = fa->texturechain)
		Sky_SkyBox_AddSurface (fa);

	GL_DisableMultitexture ();

	for (i=0 ; i<6 ; i++)
	{
		if (skymins[0][i] >= skymaxs[0][i] || skymins[1][i] >= skymaxs[1][i])
			continue;

		GL_Bind (skyboxtextures + skytexorder[i]);

		eglBegin (GL_QUADS);
		Sky_SkyBox_EmitVertex (skymins[0][i], skymins[1][i], i);
		Sky_SkyBox_EmitVertex (skymins[0][i], skymaxs[1][i], i);
		Sky_SkyBox_EmitVertex (skymaxs[0][i], skymaxs[1][i], i);
		Sky_SkyBox_EmitVertex (skymaxs[0][i], skymins[1][i], i);
		eglEnd ();
	}

	MeglDisable (GL_TEXTURE_2D);
	MeglColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	MeglEnable (GL_BLEND);
	MeglBlendFunc (GL_ZERO, GL_ONE);

	for (fa = skychain ; fa ; fa = fa->texturechain)
		Sky_FastSky_EmitFlatPoly (fa);

	MeglEnable (GL_TEXTURE_2D);
	MeglColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	MeglDisable (GL_BLEND);
	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	skychain = NULL;
	skychain_tail = &skychain;
}

/*
================
GL_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent and sky warps
can be done reasonably.
================
*/
extern msurface_t	*warpface;

void GL_SubdivideSurface (const model_t *loadmodel_subd, msurface_t *fa)
{
	vec3_t		verts[64];
	int			numverts;
	int			i;
	int			lindex;
	float		*vec;

	warpface = fa;

	// convert edges back to a normal polygon
	numverts = 0;
	for (i=0 ; i<fa->numedges ; i++)
	{
		lindex = loadmodel_subd->surfedges[fa->firstedge + i];

		if (lindex > 0)
			vec = loadmodel_subd->vertexes[loadmodel_subd->edges[lindex].v[0]].position;
		else
			vec = loadmodel_subd->vertexes[loadmodel_subd->edges[-lindex].v[1]].position;
		VectorCopy (vec, verts[numverts]);
		numverts++;
	}

	SubdividePolygon (numverts, verts[0]);
}



/*
=================
Sky_NewMap - From FitzQuake
=================
*/
void Sky_SkyBox_NewMap (void)
{
	char	*valuestring;

	// initially no sky

	Cvar_SetDefaultStringByRef (&r_skybox,"");//  R_SetSky (false); // Baker: FIXME!  We need to figure out a method to decide on with skyboxes!
	// Baker: Why are we using the cvar above but a function
	// below to set the sky?
	if ((valuestring = StringTemp_ObtainValueFromClientWorldSpawn("sky")))	 // quake lives ues q1sky key.  Do any custom maps use this?
			Cvar_SetDefaultStringByRef (&r_skybox, valuestring);	// Was ... Sky_SkyBox_SetSky (value);  But this doesn't change the cvar.

	if (cl.worldmodel->bspversion == HL_BSPVERSION)
		if ((valuestring = StringTemp_ObtainValueFromClientWorldSpawn("skyname")))	 // Baker: we are honoring nonstandard key skyname because Half-Life uses it as a standard
			Cvar_SetDefaultStringByRef (&r_skybox, valuestring ); // Was ... Sky_SkyBox_SetSky (value);  But this doesn't change the cvar.

}

void Sky_LoadSky_f (void)
{
	if (Cmd_Argc() >= 2)
		Cbuf_AddText (va("r_skybox %s", Cmd_Argv(1)));
}

