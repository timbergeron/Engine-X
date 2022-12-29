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
// r_surf.c: surface-related refresh code

#include "quakedef.h"

//float	wateralpha;
//int		r_renderflags;



static msurface_t  	*waterchain = NULL;
static msurface_t	*fencechain = NULL;
static msurface_t	*glassychain = NULL;


msurface_t  		*skychain = NULL;
msurface_t	**skychain_tail = &skychain;
static msurface_t	**waterchain_tail = &waterchain;
static msurface_t	**fencechain_tail = &fencechain;
static msurface_t	**glassychain_tail = &glassychain;

#define CHAIN_SURF_F2B(surf, chain_tail)		\
	{						\
		*(chain_tail) = (surf);			\
		(chain_tail) = &(surf)->texturechain;	\
		(surf)->texturechain = NULL;		\
	}

#define CHAIN_SURF_B2F(surf, chain) 			\
	{						\
		(surf)->texturechain = (chain);		\
		(chain) = (surf);			\
	}

void R_GenerateDynamicLightmaps (msurface_t *fa);

static glpoly_t	*fullbright_polys[MAX_GLTEXTURES];
static glpoly_t	*luma_polys[MAX_GLTEXTURES];
static qbool	drawfullbrights = false, drawlumas = false, drawcaustics = false;
glpoly_t	*caustics_polys = NULL;
static glpoly_t	*detail_polys = NULL;

#if SUPPORTS_TEXTURE_POINTER
static glpoly_t	*selection_polys = NULL;
#endif

/*
================
DrawGLPoly
================
*/
void DrawGLPoly (glpoly_t *glpoly)
{
	int		i;
	float	*verts;

	eglBegin (GL_POLYGON);
	verts = glpoly->verts[0];
	for (i=0 ; i<glpoly->numverts ; i++, verts+= VERTEXSIZE)
	{
		eglTexCoord2f (verts[3], verts[4]);
		eglVertex3fv (verts);
	}
	eglEnd ();
}

void R_RenderFullbrights (void)
{
	int			i;
	glpoly_t	*polys;

	if (!drawfullbrights)
		return;

	mglPushStates ();

	MeglDepthMask (GL_FALSE);	// don't bother writing Z
	MeglEnable (GL_ALPHA_TEST);

	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	mglFinishedStates ();

	for (i=1 ; i<MAX_GLTEXTURES ; i++)
	{
		if (!fullbright_polys[i])
			continue;
		GL_Bind (i);
		for (polys = fullbright_polys[i] ; polys ; polys = polys->fb_chain)
			DrawGLPoly (polys);
		fullbright_polys[i] = NULL;
	}

	MeglDisable (GL_ALPHA_TEST);
	MeglDepthMask (GL_TRUE);

	mglPopStates ();

	drawfullbrights = false;
}

void R_RenderLumas (void)
{
	int			i;
	glpoly_t	*polys;

	if (!drawlumas)
		return;

	MeglDepthMask (GL_FALSE);	// don't bother writing Z
	MeglEnable (GL_BLEND);
	MeglBlendFunc (GL_ONE, GL_ONE);
	// Baker: Lumas are an additive, they don't get an alpha test

	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	for (i=1 ; i<MAX_GLTEXTURES ; i++)
	{
		if (!luma_polys[i])
			continue;

		GL_Bind (i);

		for (polys = luma_polys[i] ; polys ; polys = polys->luma_chain)
			DrawGLPoly (polys);

		luma_polys[i] = NULL;
	}

	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MeglDepthMask (GL_TRUE);

	drawlumas = false;
}


void CalcReflectiveTexCoords (const float *verts, float *s, float *t)
{
	vec3_t		dir;
	float		length;

	VectorSubtract (verts, r_origin, dir);
	dir[2] *= 3;	// flatten the sphere

	length = VectorLength (dir);
	length = 6*63/length;

	dir[0] *= length;
	dir[1] *= length;

	*s = (dir[0]) * (1.0/256);
	*t = (dir[1]) * (1.0/256);
}



void EmitCausticsPolys (void)
{
	glpoly_t	*polys;
	int			i;
	float		s, t, *verts;
	extern		glpoly_t	*caustics_polys;

	if (!drawcaustics)
		return;

	GL_Bind (underwatertexture);
	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	MeglBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
	MeglEnable (GL_BLEND);

	for (polys = caustics_polys ; polys ; polys = polys->caustics_chain)
	{
		eglBegin (GL_POLYGON);
		for (i = 0, verts = polys->verts[0] ; i < polys->numverts ; i++, verts += VERTEXSIZE)
		{
			CalcCausticTexCoords (verts, &s, &t);

			eglTexCoord2f (s, t);
			eglVertex3fv (verts);
		}
		eglEnd ();
	}

	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MeglDisable (GL_BLEND);

	caustics_polys = NULL;
}

void EmitDetailPolys (void)
{
	int			i;
	float		*verts;
	glpoly_t	*polys;

	if (!detail_polys)
		return;

	GL_Bind (detailtexture);
	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	MeglBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
	MeglEnable (GL_BLEND);

	for (polys = detail_polys ; polys ; polys = polys->detail_chain)
	{
		eglBegin (GL_POLYGON);
		verts = polys->verts[0];
		for (i=0 ; i<polys->numverts ; i++, verts+=VERTEXSIZE)
		{
			eglTexCoord2f (verts[7]*18, verts[8]*18);
			eglVertex3fv (verts);
		}
		eglEnd();
	}

	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MeglDisable (GL_BLEND);

	detail_polys = NULL;
}

#if SUPPORTS_TEXTURE_POINTER
void EmitSelectionPolys (void)
{
	int			i;
	float		*verts;
	glpoly_t	*polys;

	if (!selection_polys)
		return;

	GL_Bind (selectiontexture);
	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	//if ((int)realtime & 1)
		MeglBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
	//else
	//	MeglBlendFunc (GL_ONE, GL_ONE);

	MeglEnable (GL_BLEND);
	if (tool_texturepointer.integer !=1) {MeglDisable (GL_DEPTH_TEST); MeglDisable (GL_CULL_FACE);}

	for (polys = selection_polys ; polys ; polys = polys->selection_chain)
	{
		eglBegin (GL_POLYGON);
		verts = polys->verts[0];
		for (i=0 ; i<polys->numverts ; i++, verts+=VERTEXSIZE)
		{
			eglTexCoord2f (verts[7]*18, verts[8]*18);
			eglVertex3fv (verts);
		}
		eglEnd();
	}

	if (tool_texturepointer.integer !=1) {MeglEnable (GL_DEPTH_TEST); MeglEnable (GL_CULL_FACE);}

	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MeglDisable (GL_BLEND);

	selection_polys = NULL;
}
#endif



/*
===============================================================================

				BRUSH MODELS

===============================================================================
*/


//extern	qbool	lightmap_modified[MAX_LIGHTMAPS];

/*
================
R_BlendLightmaps
================
*/
void R_RenderLightmaps (void)
{
	int			i, j;
	float		*verts;
	glpoly_t	*polys;

	MeglDepthMask (GL_FALSE);		// GL_FALSE = 0  don't bother writing Z
	MeglBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

	if (!r_lightmap.integer)
		MeglEnable (GL_BLEND);

	for (i=0 ; i<MAX_LIGHTMAPS ; i++)
	{
		if (!(polys = lightmap[i].polys))
			continue;

		GL_Bind (lightmap_textures + i);

//		Light_Upload_Lightmap_ChangedRegion (NULL, i);

		for ( ; polys ; polys = polys->chain)
		{
			eglBegin (GL_POLYGON);
			verts = polys->verts[0];
			for (j=0 ; j<polys->numverts ; j++, verts += VERTEXSIZE)
			{
				eglTexCoord2f (verts[5], verts[6]);
				eglVertex3fv (verts);
			}
			eglEnd ();
		}
		lightmap[i].polys = NULL;
	}

	MeglDisable (GL_BLEND);
	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MeglDepthMask (GL_TRUE);		// GLTRUE = 1 back to normal Z buffering
}



/*
================
R_DrawWaterSurfaces
================
*/
void R_DrawWaterSurfaces (void)
{
	float	wateralpha;
	msurface_t	*surf;

	// Baker: MH automatic underwater transparency
//	if (/*r_renderflags != 3 &&*/ !scene_novis.integer)
//	{
//		wateralpha = 1;
//	}
//	Con_Printf("r_renderflags is %d\n", r_renderflags);

	if (!waterchain)
		return;

	GL_DisableMultitexture ();

	wateralpha = CLAMP (0, r_water_alpha.floater, 1);

	if (wateralpha < 1.0)
	{
		MeglEnable (GL_BLEND);
		MeglColor4f (1, 1, 1, wateralpha);
		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		if (wateralpha < 0.9)
			MeglDepthMask (GL_FALSE);
	}

	
	for (surf = waterchain ; surf ; surf = surf->texturechain)
	{
		GL_Bind (surf->texinfo->texture->gl_texturenum);
		EmitWaterPolys (surf);
	}

	waterchain = NULL;
	waterchain_tail = &waterchain;

	if (wateralpha < 1.0)
	{
		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		MeglColor3ubv (color_white);
		MeglDisable (GL_BLEND);
		if (wateralpha < 0.9)
			MeglDepthMask (GL_TRUE);
	}
}

void R_DrawFenceChain (void)
{
	int			k;
	float		*verts;
	msurface_t	*surf;
	texture_t	*texture;

	if (!fencechain)
		return;

	MeglEnable (GL_ALPHA_TEST);

	for (surf = fencechain ; surf ; surf = surf->texturechain)
	{
		texture = surf->texinfo->texture;
		Light_GenerateDynamicLightmaps_For_Surface (surf);

		// bind the world texture
		GL_DisableMultitexture ();
		GL_Bind (texture->gl_texturenum);

		if (gl_mtexable)
		{
			// bind the lightmap texture
			GL_EnableMultitexture ();
			GL_Bind (lightmap_textures + surf->lightmaptexturenum);
			MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

			// update lightmap if its modified by dynamic lights
//			Light_Upload_Lightmap_ChangedRegion (surf, -1);
		}

		eglBegin (GL_POLYGON);
		verts = surf->polys->verts[0];
		for (k = 0 ; k < surf->polys->numverts ; k++, verts += VERTEXSIZE)
		{
			if (gl_mtexable)
			{
				qglMultiTexCoord2f (GL_TEXTURE0_ARB, verts[3], verts[4]);
				qglMultiTexCoord2f (GL_TEXTURE1_ARB, verts[5], verts[6]);
			}
			else
			{
				eglTexCoord2f (verts[3], verts[4]);
			}
			eglVertex3fv (verts);
		}
		eglEnd ();
	}

	fencechain = NULL;

	GL_DisableMultitexture ();
	MeglDisable (GL_ALPHA_TEST);

}


void R_DrawGlassyChain (void)
{
	int			k;
	float		*verts;
	msurface_t	*surf;
	texture_t	*texture;

	if (!glassychain)
		return;

	MeglEnable (GL_BLEND);

	MeglBlendFunc(GL_DST_COLOR, GL_SRC_COLOR); //2x modulate

	for (surf = glassychain ; surf ; surf = surf->texturechain)
	{
		texture = surf->texinfo->texture;
		Light_GenerateDynamicLightmaps_For_Surface (surf);

		// bind the world texture
		GL_DisableMultitexture ();
		GL_Bind (texture->gl_texturenum);

		if (gl_mtexable)
		{
			// bind the lightmap texture
			GL_EnableMultitexture ();
			GL_Bind (lightmap_textures + surf->lightmaptexturenum);
			MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

			// update lightmap if its modified by dynamic lights
//			Light_Upload_Lightmap_ChangedRegion (surf, -1);
		}

		eglBegin (GL_POLYGON);
		verts = surf->polys->verts[0];
		for (k = 0 ; k < surf->polys->numverts ; k++, verts += VERTEXSIZE)
		{
			float s, t;
			CalcReflectiveTexCoords (verts, &s, &t);

			if (gl_mtexable)
			{
				qglMultiTexCoord2f (GL_TEXTURE0_ARB, s, t);
				qglMultiTexCoord2f (GL_TEXTURE1_ARB, verts[5], verts[6]);
			}
			else
			{
				eglTexCoord2f (s, t);
			}
			eglVertex3fv (verts);
		}
		eglEnd ();
	}

	glassychain = NULL;

	MeglDisable (GL_BLEND);
//	glDepthMask (GL_TRUE);
	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	MeglColor3ubv (color_white);

	GL_DisableMultitexture ();
}

static void R_ClearTextureChains_ForModel (model_t *clmodel)
{
	int			i, waterline;
	texture_t	*texture;

	for (i=0; i < MAX_LIGHTMAPS; i++)
		lightmap[i].polys = NULL;

	memset (fullbright_polys, 0, sizeof(fullbright_polys));
	memset (luma_polys, 0, sizeof(luma_polys));

	for (i=0 ; i<clmodel->numtextures ; i++)
	{
		for (waterline=0 ; waterline<2 ; waterline++)
		{
			if ((texture = clmodel->textures[i]))
			{
				texture->texturechain[waterline] = NULL;
				texture->texturechain_tail[waterline] = &texture->texturechain[waterline];
			}
		}
	}

	r_notexture_mip->texturechain[0] = NULL;
	r_notexture_mip->texturechain_tail[0] = &r_notexture_mip->texturechain[0];
	r_notexture_mip->texturechain[1] = NULL;
	r_notexture_mip->texturechain_tail[1] = &r_notexture_mip->texturechain[1];

	skychain = NULL;
	skychain_tail = &skychain;

	// Baker: Because water surfaces are drawn after entities
	if (clmodel == cl.worldmodel)
	{
		waterchain = NULL;
		waterchain_tail = &waterchain;

		glassychain = NULL;
		glassychain_tail = &glassychain;

		// Baker: Really?  Won't this prevent the world from supporting alpha textures?  SOLVED!
		fencechain = NULL;
		fencechain_tail = &fencechain;
	}



}


/*
================
R_DrawWorld_Solid_TextureChains
================
*/
void R_DrawWorld_Solid_TextureChains (const model_t *model, const int contents, const float unused1, const int frame_brush)
{
	int			i, k, waterline, GL_LIGHTMAP_TEXTURE, GL_FB_TEXTURE, GL_CAUSTICS_TEXTURE;
	float		*verts;
	msurface_t	*surf;
	texture_t	*texture;
	qbool		mtex_lightmaps, mtex_fbs, doMtex1, doMtex2, /*doMtex3,*/ render_lightmaps = false;
	qbool		draw_fbs, draw_mtex_fbs, draw_mtex_caustics, can_mtex_lightmaps, can_mtex_fbs, can_mtex_caustics;


	if (r_pass_lumas.integer && r_pass_lumas_brush.integer)
	{
		can_mtex_lightmaps = gl_mtexable;		// Need 2 texture units
		can_mtex_fbs = gl_textureunits >= 3;	// Needs 3 texture units

		can_mtex_caustics = gl_textureunits >= 4;
	}
	else
	{
		can_mtex_lightmaps = gl_textureunits >= 3;			// Needs 3 texture units
		can_mtex_fbs = gl_textureunits >= 3 && gl_add_ext;	// Needs 3 texture units AND gl_add_ext

		can_mtex_caustics = gl_textureunits >= 4;
	}

	if (unused1 < 1)
		goto extra_shitty;

	GL_DisableMultitexture ();
	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);



	if (IS_FULL_LIGHT || IS_LIGHTMAP_ONLY || !cl.worldmodel->lightdata)
		goto shitty;


	for (i=0 ; i<model->numtextures ; i++)
	{
		if (!model->textures[i] || (!model->textures[i]->texturechain[0] && !model->textures[i]->texturechain[1]))
			continue;

		texture = R_TextureAnimation (model->textures[i], frame_brush);

		// bind the world texture
		GL_SelectTexture (GL_TEXTURE0_ARB);		// r_lightmap hint
		GL_Bind (texture->gl_texturenum);				// r_lightmap hint

		draw_fbs = texture->isLumaTexture || (r_pass_lumas.integer && r_pass_lumas_brush.integer);
		draw_mtex_fbs = draw_fbs && can_mtex_fbs;
		draw_mtex_caustics = r_pass_caustics.integer && underwatertexture && can_mtex_caustics;
		drawcaustics = r_pass_caustics.integer && underwatertexture && !draw_mtex_caustics; // Draw caustics if they should be draw but we cannot mtex

//		GL_DisableTMU (GL_TEXTURE3_ARB);
		if (gl_mtexable)
		{
			if (texture->isLumaTexture && (!r_pass_lumas.integer || !r_pass_lumas_brush.integer))		// Luma path: With fullbrights    ... lightmap then fullbright
			{													// Luma path: Without fullbrights ... fullbright then lightmap
				if (gl_add_ext)									//            Texture set has a texture flaw!  fullbright slight black
				{
					doMtex1 = true;
					GL_FB_TEXTURE = GL_TEXTURE1_ARB;
					GL_EnableTMU (GL_FB_TEXTURE);

					GL_Bind (texture->fb_texturenum);
					MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD); // Luma non-fullbright path

					mtex_lightmaps = can_mtex_lightmaps;
					mtex_fbs = true;

					if (mtex_lightmaps)
					{
						doMtex2 = true;
						GL_LIGHTMAP_TEXTURE = GL_TEXTURE2_ARB;
						GL_EnableTMU (GL_LIGHTMAP_TEXTURE);
						MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
					}
					else
					{
						doMtex2 = false;
						render_lightmaps = true;
					}
				}
				else
				{
					GL_DisableTMU (GL_TEXTURE1_ARB);
					render_lightmaps = true;
					doMtex1 = doMtex2 = mtex_lightmaps = mtex_fbs = false;
				}
			}
			else
			{
				doMtex1 = true;
				GL_LIGHTMAP_TEXTURE = GL_TEXTURE1_ARB;
				GL_EnableTMU (GL_LIGHTMAP_TEXTURE);
				// can this possibly be all?
				MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
//				eglTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
//				eglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);

				mtex_lightmaps = true;
				mtex_fbs = texture->fb_texturenum && draw_mtex_fbs;

				if (mtex_fbs)
				{
					doMtex2 = true;
					GL_FB_TEXTURE = GL_TEXTURE2_ARB;
					GL_EnableTMU (GL_FB_TEXTURE);
					GL_Bind (texture->fb_texturenum);
					MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,  texture->isLumaTexture ? GL_ADD :  GL_DECAL);
				}
				else
				{
					doMtex2 = false;
				}

//				if (doMtex2 && draw_mtex_caustics)


			}
		}
		else
		{
			render_lightmaps = true;
			doMtex1 = doMtex2 = mtex_lightmaps = mtex_fbs = false;
		}

		for (waterline=0 ; waterline<2 ; waterline++)
		{
			int didcaustics = -1;
			if (!(surf = model->textures[i]->texturechain[waterline]))
				continue;

#if 1
			// CAUSTICS
			if (draw_mtex_caustics && waterline)
			{
				if (doMtex2)
				{
					GL_CAUSTICS_TEXTURE = GL_TEXTURE3_ARB;
					didcaustics = 3;
				}
				else
				{
					GL_CAUSTICS_TEXTURE = GL_TEXTURE2_ARB;
					didcaustics = 2;
				}

				GL_EnableTMU (GL_CAUSTICS_TEXTURE);
				GL_Bind (underwatertexture);

				MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);		// Close ... too dark



			}

#endif

			for ( ; surf ; surf = surf->texturechain)
			{
				if (mtex_lightmaps)
				{
					// bind the lightmap texture
					GL_SelectTexture (GL_LIGHTMAP_TEXTURE);
					GL_Bind (lightmap_textures + surf->lightmaptexturenum);

					// update lightmap if it is modified by dynamic lights
					Light_GenerateDynamicLightmaps_For_Surface (surf);

					// Baker: Since we aren't doing the normal chain process, we perform the upload here
//					Light_Upload_Lightmap_ChangedRegion (surf, -1);
				}
				else
				{
					// Baker: Lightmap chain doesn't need generated for mtex?
					surf->polys->chain = lightmap[surf->lightmaptexturenum].polys;
					lightmap[surf->lightmaptexturenum].polys = surf->polys;

					// Baker: is there a good reason to NOT upload the lightmap here?
					Light_GenerateDynamicLightmaps_For_Surface (surf);
				}

#if 1

				if (model->surfaces == cl.worldmodel->surfaces && draw_mtex_caustics && ISUNDERWATER(contents)) // Thanks R00k!!
				{
					if (doMtex2)
					{
						GL_CAUSTICS_TEXTURE = GL_TEXTURE3_ARB;
						didcaustics = 3;
					}
					else
					{
						GL_CAUSTICS_TEXTURE = GL_TEXTURE2_ARB;
						didcaustics = 2;
					}

					GL_EnableTMU (GL_CAUSTICS_TEXTURE);
					GL_Bind (underwatertexture);

					MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);		// Close ... too dark



				}
#endif


				eglBegin (GL_POLYGON);
				verts = surf->polys->verts[0];
				for (k = 0 ; k < surf->polys->numverts ; k++, verts += VERTEXSIZE)
				{
					if (doMtex1)
					{
						if (surf->flags & SURF_DRAWREFLECTIVE)	// We don't chain this up as it is a simple s,t coord modification
						{
							float s, t;
							CalcReflectiveTexCoords (verts, &s, &t);
							qglMultiTexCoord2f (GL_TEXTURE0_ARB, s, t);
//							surf->flags |= SURF_SELECTED;
						}
						else
							qglMultiTexCoord2f (GL_TEXTURE0_ARB, verts[3], verts[4]);

						if (mtex_lightmaps)
							qglMultiTexCoord2f (GL_LIGHTMAP_TEXTURE, verts[5], verts[6]);

						if (mtex_fbs)
							qglMultiTexCoord2f (GL_FB_TEXTURE, verts[3], verts[4]);
#if 1
//((model->type == mod_brush) && r_pass_caustics.value && underwatertexture && ISUNDERWATER(contents))

						if ((draw_mtex_caustics && waterline)
//							|| (/*model->type == mod_brush*/ model && model->surfaces == cl.worldmodel->surfaces && draw_mtex_caustics && (surf->flags & SURF_UNDERWATER) /*&& ISUNDERWATER(contents)*/ )) // Thanks R00k!!
//							|| (model->type == mod_brush && draw_mtex_caustics && ISUNDERWATER(contents))) // Thanks R00k!!
//							|| (/model->type == mod_brush*/ model && model->surfaces == cl.worldmodel->surfaces && draw_mtex_caustics && ISUNDERWATER(contents))) // Thanks R00k!!
							|| (model->surfaces == cl.worldmodel->surfaces && draw_mtex_caustics && ISUNDERWATER(contents))) // Thanks R00k!!
						{

							float s, t;
							CalcCausticTexCoords (verts, &s, &t);
							qglMultiTexCoord2f (GL_CAUSTICS_TEXTURE, s, t);

						}
#endif
					}
					else
					{
						eglTexCoord2f (verts[3], verts[4]);
					}
					eglVertex3fv (verts);
				}
				eglEnd ();

//				if (waterline && r_pass_caustics.integer && underwatertexture)
//				{
//					surf->polys->caustics_chain = caustics_polys;
//					caustics_polys = surf->polys;
//				}
				if (!waterline && r_pass_detail.integer && detailtexture)
				{
					surf->polys->detail_chain = detail_polys;
					detail_polys = surf->polys;
				}
#if SUPPORTS_TEXTURE_POINTER
				if ((surf->flags & SURF_SELECTED) && selectiontexture)
				{
					surf->polys->selection_chain = selection_polys;
					selection_polys = surf->polys;
				}
#endif
				if (texture->fb_texturenum && draw_fbs && !mtex_fbs)
				{
					if (texture->isLumaTexture)
					{
						surf->polys->luma_chain = luma_polys[texture->fb_texturenum];
						luma_polys[texture->fb_texturenum] = surf->polys;
						drawlumas = true;
					}
					else
					{
						surf->polys->fb_chain = fullbright_polys[texture->fb_texturenum];
						fullbright_polys[texture->fb_texturenum] = surf->polys;
						drawfullbrights = true;
					}
				}
			} // End of surfaces loop

			if (didcaustics > 0)
				if (didcaustics == 3)
					GL_DisableTMU (GL_TEXTURE3_ARB);
				else
					GL_DisableTMU (GL_TEXTURE2_ARB);


		} //End of waterline loop



		if (doMtex2)
			GL_DisableTMU (GL_TEXTURE2_ARB);
		if (doMtex1)
			GL_DisableTMU (GL_TEXTURE1_ARB);

	}	// End of model num textures

	if (gl_mtexable)
		GL_SelectTexture (GL_TEXTURE0_ARB);

	// These are superchains ... well, sort of

	// Non-multitextured pathways
	if (r_pass_lumas.integer && r_pass_lumas_brush.integer)
	{
		if (render_lightmaps)
			R_RenderLightmaps ();		// One pass per lightmap texture number, per poly in chain
		if (drawfullbrights)
			R_RenderFullbrights ();		// One pass per GL texture, per poly in chain
		if (drawlumas)
			R_RenderLumas ();			// One pass per GL texture, per poly in chain
	}
	else
	{
		if (drawlumas)
			R_RenderLumas ();
		if (render_lightmaps)
			R_RenderLightmaps ();
		if (drawfullbrights)
			R_RenderFullbrights ();
	}

	EmitCausticsPolys ();				// One pass per poly in chain
	EmitDetailPolys ();					// One pass per poly in chain
#if SUPPORTS_TEXTURE_POINTER
	//if (tool_texturepointer.integer)
		EmitSelectionPolys ();
#endif
	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);	// ???

	return;

shitty:

	GL_DisableMultitexture ();
	if (IS_FULL_LIGHT)
		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	else
		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

	for (i=0 ; i<model->numtextures ; i++)
	{
		if (!model->textures[i] || (!model->textures[i]->texturechain[0] && !model->textures[i]->texturechain[1]))
			continue;

		texture = R_TextureAnimation (model->textures[i], frame_brush);

		// bind the world texture

		if (IS_FULL_LIGHT)
		{
			GL_SelectTexture (GL_TEXTURE0_ARB);		// r_lightmap hint
			GL_Bind (texture->gl_texturenum);				// r_lightmap hint
		}

		for (waterline=0 ; waterline<2 ; waterline++)
		{
			if (!(surf = model->textures[i]->texturechain[waterline]))
				continue;

			for ( ; surf ; surf = surf->texturechain)
			{
				if (cl.worldmodel->lightdata && r_lightmap.integer == 1)
				{
					// bind the lightmap texture
					GL_SelectTexture (GL_TEXTURE0_ARB);
					GL_Bind (lightmap_textures + surf->lightmaptexturenum);

					Light_GenerateDynamicLightmaps_For_Surface (surf);

//					Light_Upload_Lightmap_ChangedRegion (surf, -1);

				}

				eglBegin (GL_POLYGON);
				verts = surf->polys->verts[0];
				for (k = 0 ; k < surf->polys->numverts ; k++, verts += VERTEXSIZE)
				{
					if (IS_FULL_LIGHT)
						eglTexCoord2f (verts[3], verts[4]);
					else
						eglTexCoord2f (verts[5], verts[6]);
					eglVertex3fv (verts);
				}
				eglEnd ();

#if SUPPORTS_TEXTURE_POINTER
				if ((surf->flags & SURF_SELECTED) && selectiontexture /* And some cvar here */)
				{
					surf->polys->selection_chain = selection_polys;
					selection_polys = surf->polys;
				}
#endif

			}
		}

	}
#if SUPPORTS_TEXTURE_POINTER
	//if (tool_texturepointer.integer)
		EmitSelectionPolys ();
#endif
	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	return;

// Baker: d3d alpha entity work around
// However, since fullbright entities don't get lightmaps anyway
// Let us leave this for OpenGL too.
extra_shitty:

	// alpha entities

	GL_DisableMultitexture ();
	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	MeglEnable (GL_BLEND);
	MeglDepthMask (GL_FALSE);
	MeglColor4f (1, 1, 1, unused1);

	for (i=0 ; i<model->numtextures ; i++)
	{
		if (!model->textures[i] || (!model->textures[i]->texturechain[0] && !model->textures[i]->texturechain[1]))
			continue;

		texture = R_TextureAnimation (model->textures[i], frame_brush);

		// bind the world texture

		GL_SelectTexture (GL_TEXTURE0_ARB);		// r_lightmap hint
		GL_Bind (texture->gl_texturenum);				// r_lightmap hint

		for (waterline=0 ; waterline<2 ; waterline++)
		{
			if (!(surf = model->textures[i]->texturechain[waterline]))
				continue;

			for ( ; surf ; surf = surf->texturechain)
			{

				eglBegin (GL_POLYGON);
				verts = surf->polys->verts[0];
				for (k = 0 ; k < surf->polys->numverts ; k++, verts += VERTEXSIZE)
				{
					eglTexCoord2f (verts[3], verts[4]);
					eglVertex3fv (verts);
				}
				eglEnd ();
			}
		}

	}

	// Leave things the way they came

	MeglDepthMask (GL_TRUE);
	MeglDisable (GL_BLEND);
	MeglColor4f (1, 1, 1, 1);
	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	return;


}

#if 0
void GL_PolygonOffset (int offset)
{
	if (offset > 0)
	{
		MeglEnable (GL_POLYGON_OFFSET_FILL);
		MeglEnable (GL_POLYGON_OFFSET_LINE);
		eglPolygonOffset(1, offset);
	}
	else if (offset < 0)
	{
		MeglEnable (GL_POLYGON_OFFSET_FILL);
		MeglEnable (GL_POLYGON_OFFSET_LINE);
		eglPolygonOffset(-1, offset);
	}
	else
	{
		MeglDisable (GL_POLYGON_OFFSET_FILL);
		MeglDisable (GL_POLYGON_OFFSET_LINE);
	}
}
#endif

#define BACKFACE_EPSILON	0.01

/*
=================
R_DrawModel_Brush
=================
*/

//johnfitz -- struct for passing lerp information to drawing functions
typedef struct {
	short	pose1;
	short	pose2;

	vec3_t	origin;
	vec3_t	angles;

	qbool	full_light;
	float	shade_light;
	float	ambient_light;

	qbool	isLerpingAnimation;
	float	blend;
	float   iblend;

} lerpdata_t;
//johnfitz



/*
=================
R_SetupEntityTransform -- johnfitz -- set up transform part of lerpdata
=================
*/
static void R_SetupEntityTransform (entity_t *e, lerpdata_t *lerpdata)
{
	float blend;
	vec3_t		d;
	int			i;

	// if LERP_RESETMOVE, kill any lerps in progress
	if (e->lerpflags & LERP_RESETMOVE)
	{
		e->movelerpstart = 0;
		VectorCopy (e->origin, e->previousorigin);
		VectorCopy (e->origin, e->currentorigin);
		VectorCopy (e->angles, e->previousangles);
		VectorCopy (e->angles, e->currentangles);
		e->lerpflags -= LERP_RESETMOVE;
	}
	else if (!VectorCompare (e->origin, e->currentorigin) || !VectorCompare (e->angles, e->currentangles)) // origin/angles changed, start new lerp
	{
		e->movelerpstart = cl.time;
		VectorCopy (e->currentorigin, e->previousorigin);
		VectorCopy (e->origin,  e->currentorigin);
		VectorCopy (e->currentangles, e->previousangles);
		VectorCopy (e->angles,  e->currentangles);
	}

	//set up values
	if (scene_lerpmove.floater /* && e != &cl.viewent && e->lerpflags & LERP_MOVESTEP*/)
	{
		if (e->lerpflags & LERP_FINISH)
			blend = CLAMP (0, (cl.time - e->movelerpstart) / (e->lerpfinish - e->movelerpstart), 1);
		else
			blend = CLAMP (0, (cl.time - e->movelerpstart) / 0.1, 1);

		//translation
		VectorSubtract (e->currentorigin, e->previousorigin, d);
		lerpdata->origin[0] = e->previousorigin[0] + d[0] * blend;
		lerpdata->origin[1] = e->previousorigin[1] + d[1] * blend;
		lerpdata->origin[2] = e->previousorigin[2] + d[2] * blend;

		//rotation
		VectorSubtract (e->currentangles, e->previousangles, d);
		for (i = 0; i < 3; i++)
		{
			if (d[i] > 180)  d[i] -= 360;
			if (d[i] < -180) d[i] += 360;
		}
		lerpdata->angles[0] = e->previousangles[0] + d[0] * blend;
		lerpdata->angles[1] = e->previousangles[1] + d[1] * blend;
		lerpdata->angles[2] = e->previousangles[2] + d[2] * blend;
	}
	else //don't lerp
	{
		VectorCopy (e->origin, lerpdata->origin);
		VectorCopy (e->angles, lerpdata->angles);
	}

	// Baker: Something is wrong with brush model interpolation.  Especially static entities
	//        Fixed.  Static entities don't get new angles
	//		  So interpolation needed lerpmove reset flag in parse statics.
}

void R_DrawModel_Brush (entity_t *ent)	// Submodels ... not world
{
	int			i, /*k,*/ underwater;
	float		dot;
//	vec3_t		mins, maxs;
	msurface_t	*psurf;
	mplane_t	*pplane;
	model_t		*clmodel = ent->model;
//	qbool		rotated;
	float 		entalpha = ENTALPHA_DECODE(ent->alpha);
	vec3_t		brushmodel_origin;
	lerpdata_t	lerpdata;
	current_texture_num = -1;

	if (!r_drawmodels_brush.integer) return;
//	if (entalpha < 1 && !r_drawentities_alpha.integer) return;
	if (r_brushmodels_with_world.integer && ent->drawn == true) return;	// Drawn as part of the world draw pass
	
	//
	// setup pose/lerp data -- do it first so we don't miss updates due to culling
	//

	R_SetupEntityTransform (ent, &lerpdata);

	//
	// cull it
	//

	if (R_CullForEntity (ent/*, NULL)*/))
		return;


	// Get rid of Z-fighting for textures by offsetting the drawing of entity models compared to normal polygons. (Only works if gl_ztrick is turned off)
	MeglEnable(GL_POLYGON_OFFSET_FILL);


	R_ClearTextureChains_ForModel (clmodel);

//	if (entalpha < 1 )
//	{
//		MeglEnable (GL_BLEND);
//		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//		MeglColor4f (1, 1, 1, ent->transparency);
//		MeglDepthMask (GL_TRUE);
//		MeglColor4f (1, 1, 1, entalpha);	// back2forwards fix
//	}

//	VectorSubtract (r_refdef.vieworg, ent->origin, brushmodel_origin);
	VectorSubtract (r_refdef.vieworg, lerpdata.origin, brushmodel_origin);

	if (lerpdata.angles[0] || lerpdata.angles[1] || lerpdata.angles[2])
	{
		vec3_t	temp, forward, right, up;

		VectorCopy (brushmodel_origin, temp);
//		AngleVectors (ent->angles, forward, right, up);
		AngleVectors (lerpdata.angles, forward, right, up);

		brushmodel_origin[0] = DotProduct (temp, forward);
		brushmodel_origin[1] = -DotProduct (temp, right);
		brushmodel_origin[2] = DotProduct (temp, up);
	}

	psurf = &clmodel->surfaces[clmodel->firstmodelsurface];

	// calculate dynamic lighting for bmodel if it's not an instanced model
	if (clmodel->firstmodelsurface != 0)
		Light_PushDlights_DrawModel_Brush_Ent (clmodel);

	mglPushStates (); eglPushMatrix ();

//	eglTranslatef (ent->origin[0], ent->origin[1], ent->origin[2]);
//	eglRotatef (ent->angles[1], 0, 0, 1);
//	eglRotatef (ent->angles[0], 0, 1, 0);
//	eglRotatef (ent->angles[2], 1, 0, 0);

	R_RotateForEntity (lerpdata.origin, lerpdata.angles); //ent->origin, ent->angles);



	for (i=0 ; i<clmodel->nummodelsurfaces ; i++, psurf++)
	{
		// find which side of the node we are on
		pplane = psurf->plane;
		dot = PlaneDiff(brushmodel_origin, pplane);

		// draw the polygon
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
			if (psurf->flags & SURF_DRAWSKY)
			{
				CHAIN_SURF_B2F(psurf, skychain);
			}
			else if (psurf->flags & SURF_DRAWLIQUID)		// Baker: Shouldn't an entity know if it has transparent surfaces for sorting?
			{
				EmitWaterPolys (psurf);						// Baker: I thought submodels couldn't support this
			}
			else if (psurf->flags & SURF_DRAWGLASSY)	// Baker: Shouldn't an entity know if it has transparent surfaces for sorting?
			{
				EmitGlassyPolys (psurf);
			}
			else if (psurf->flags & SURF_DRAWFENCE)		// Baker: Shouldn't an entity know if it has transparent surfaces for sorting?
			{

				EmitFencePolys (psurf);
				//CHAIN_SURF_B2F(psurf, alphachain);
			}
			else
			{
				underwater = (psurf->flags & SURF_UNDERWATER) ? 1 : 0;
				CHAIN_SURF_B2F(psurf, psurf->texinfo->texture->texturechain[underwater]);
			}
		}
	}

	//draw the textures chains for the model
	R_DrawWorld_Solid_TextureChains (clmodel, (TruePointContents(brushmodel_origin)), entalpha, ent->frame);

	Sky_OldSky_DrawChain ();

	R_DrawGlassyChain ();
	R_DrawFenceChain ();


	eglPopMatrix (); mglPopStates ();

//	if (entalpha < 1.0)
//	{
//		MeglColor4f (1, 1, 1, 1);
//		MeglDisable (GL_BLEND);
//	}

	// Restore prior state / End of Get rid of Z-fighting
	MeglDisable(GL_POLYGON_OFFSET_FILL);

}

/*
===============================================================================

				WORLD MODEL

===============================================================================
*/

/*
================
R_RecursiveWorldNode
================
*/
static void R_Build_WorldChains_WorldNode_Recursive (mnode_t *node, int clipflags, const vec3_t testlocation)
{
	int		c, side, clipped, underwater;
	mplane_t	*plane, *clipplane;
	int			j;
	msurface_t	*surf, **mark;
	mleaf_t		*leaf;
	double		dot;

	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->visframe != r_visframecount)
		return;
	// Determine if node is on-screen
	for (c=0, clipplane=frustum ; c<4 ; c++, clipplane++)
	{
		if (!(clipflags & (1<<c)))
			continue;	// don't need to clip against it

		clipped = BOX_ON_PLANE_SIDE(node->minmaxs, node->minmaxs+3, clipplane);
		if (clipped == 2)
			return;
		else if (clipped == 1)
			clipflags &= ~(1<<c);	// node is entirely on screen
	}

// if a leaf node, draw stuff
	if (node->contents < 0)
	{
		// iterate through leaves, marking surfaces
		leaf = (mleaf_t *)node;
		if (leaf->nummarksurfaces)
			for (j=0, mark = leaf->firstmarksurface; j<leaf->nummarksurfaces; j++, mark++)
				(*mark)->visframe = r_framecount;

	// deal with static model fragments in this leaf
		if (leaf->efrags)
			Efrags_StoreEfrags (&leaf->efrags);

		return;
	}

// node is just a decision point, so go down the appropriate sides

// find which side of the node we are on
	plane = node->plane;
	dot = PlaneDiff(testlocation, plane);
	side = (dot >= 0) ? 0 : 1;

// recurse down the children, front side first
	R_Build_WorldChains_WorldNode_Recursive (node->children[side], clipflags, testlocation);

// draw stuff
	c = node->numsurfaces;

	if (c)
	{
		if (dot < 0 -BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;

		for (surf = cl.worldmodel->surfaces + node->firstsurface ; c ; c--, surf++)
		{
			if ((surf->flags & SURF_SELECTED) && tool_texturepointer.integer !=1)
			{
				// free pass
				// Baker: This allows drawing selected surfaces that are hidden from view
				// At least mostly ;)
			}
			else
			{
			if (surf->visframe != r_framecount)
				continue;

			if ((dot < 0) ^ !!(surf->flags & SURF_PLANEBACK))
				continue;		// wrong side
		}
			// if sorting by texture, just store it out
			if (surf->flags & SURF_DRAWSKY)
			{
				CHAIN_SURF_F2B(surf, skychain_tail);
			}
			else if (surf->flags & SURF_DRAWLIQUID)
			{
				CHAIN_SURF_F2B(surf, waterchain_tail);
			}
			else if (surf->flags & SURF_DRAWGLASSY)
			{
				CHAIN_SURF_F2B(surf, glassychain_tail);
			}
			else if (surf->flags & SURF_DRAWFENCE)
			{

				CHAIN_SURF_B2F(surf, fencechain);
			}
			else
			{
				underwater = (surf->flags & SURF_UNDERWATER) ? 1 : 0;
//				if (surf->flags & SURF_UNDERWATER) // Baker: MH automatic underwater transparency
//					r_renderflags = r_renderflags | 1;
//				if (!(surf->flags & SURF_UNDERWATER)) // Baker: MH automatic underwater transparency
//					r_renderflags = r_renderflags | 2;

				CHAIN_SURF_F2B(surf, surf->texinfo->texture->texturechain_tail[underwater]);
			}
		}
	}

	// recurse down the back side
	R_Build_WorldChains_WorldNode_Recursive (node->children[!side], clipflags, testlocation);
}


void R_Build_Chains_World (void)
{


//	memset (&ent, 0, sizeof(ent));
//	ent.model = cl.worldmodel;

	R_ClearTextureChains_ForModel (cl.worldmodel);

	//VectorCopy (r_refdef.vieworg, modelorg);

	//	the_currententity = &ent;
	current_texture_num = -1;

	// set up texture chains for the world
	R_Build_WorldChains_WorldNode_Recursive (cl.worldmodel->nodes, 15, r_refdef.vieworg);
}

void R_Build_Chains_World_SubModels (void)
{
	entity_t	*ent;
	int i;

	if (!r_brushmodels_with_world.integer) return;

	// Now add the brush models!
	for (i=0 ; i< cl_numvisedicts ; i++)	//0 is world...
	{	// Baker: no point of shadows on non-visible edicts?
		ent = cl_visedicts[i];

		if (!ent->model)
			continue;   // no model for ent

		if (!(ent->model->surfaces == cl.worldmodel->surfaces))
			continue;	// model isnt part of world

		{

			int			j, /*k,*/ underwater;
			float		dot;
//			vec3_t		mins, maxs;
			model_t		*clmodel = ent->model;
			msurface_t	*psurf =&clmodel->surfaces[clmodel->firstmodelsurface];
			mplane_t	*pplane;

//			qbool		rotated;
			float 		entalpha = ENTALPHA_DECODE(ent->alpha);
			qbool		hasorigin = false;
			vec3_t		brushmodel_origin;
	//		current_texture_num = -1;

			ent->drawn = false;

			if (ent->origin[0] || ent->origin[1] || ent->origin[2])
//			{
//				if (r_brushmodels_with_world.integer !=2)
					continue;		// Only r_brushmodels_with_world 2 draws non 0.  What about caustics?
			
//				hasorigin = true;
//			}						// Baker: Additional problem

			if (ent->angles[0] || ent->angles[1] || ent->angles[2])
				continue;  // Baker: Not able to handle this in my head without R_RotateForEntity yet ...
			
			if (!(cl_visedicts[i]->alpha == ENTALPHA_DEFAULT))
				continue;  // Entities with entity effects aren't drawn as part of world

			if (R_CullForEntity (ent/*, brushmodel_origin)*/))
				continue;

			VectorSubtract (r_refdef.vieworg, ent->origin, brushmodel_origin);

			// calculate dynamic lighting for bmodel if it's not an instanced model
			if (clmodel->firstmodelsurface != 0)
				Light_PushDlights_DrawModel_Brush_Ent (clmodel);

			for (j=0 ; j<clmodel->nummodelsurfaces ; j++, psurf++)
			{
				// find which side of the node we are on
				pplane = psurf->plane;
				dot = PlaneDiff(brushmodel_origin, pplane);
			
	//			if ((psurf->hasorigin = hasorigin))
	//				VectorCopy (brushmodel_origin, psurf->brushmodel_origin);

				// add the polygon to the draw list the polygon
				if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
					(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
				{
					if (psurf->flags & SURF_DRAWSKY)
					{
						CHAIN_SURF_B2F(psurf, skychain);
					}
					else if (psurf->flags & SURF_DRAWLIQUID)
					{
						EmitWaterPolys (psurf);
					}
					else if (psurf->flags & SURF_DRAWGLASSY)
					{
						EmitGlassyPolys (psurf);
					}
					else if (psurf->flags & SURF_DRAWFENCE)
					{

						CHAIN_SURF_B2F(psurf, fencechain);
					}
					else
					{
						underwater = (psurf->flags & SURF_UNDERWATER) ? 1 : 0;
						CHAIN_SURF_B2F(psurf, psurf->texinfo->texture->texturechain[underwater]);
					}
//					psurf->flags |= SURF_SELECTED;
				}
			} // End of surfaces loop

			ent->drawn = true;

		} // End of embedded function
	} // End of visedicts loop
}





