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
// gl_model.c -- model loading and caching
// Baker: Validated 6-27-2011.  Just external vis


// models are the only shared resource between a client and server running
// on the same machine.

#include "quakedef.h"

model_t	*loadmodel;
char	loadname[32];	// for hunk tags


#if FITZQUAKE_PROTOCOL
#define	MAX_MOD_KNOWN	2048 //johnfitz -- was 512
#else
#define	MAX_MOD_KNOWN	512
#endif

static	model_t	mod_known[MAX_MOD_KNOWN];
static	int	mod_numknown;


model_t *Mod_LoadModel (model_t *mod, qbool crash);


/*
===============
Mod_Init
===============
*/
void Mod_Init (void)
{
	Mod_BrushModel_Clear_Vis (); // memset (mod_novis, 0xff, sizeof(mod_novis));
}

/*
===============
Mod_Extradata

Caches the data if needed
===============
*/
void *Mod_Extradata (model_t *mod)
{
	void	*r;

	if ((r = Cache_Check (&mod->cache)))
		return r;

	Mod_LoadModel (mod, true);

	if (!mod->cache.data)
		Sys_Error ("Mod_Extradata: caching failed");

	return mod->cache.data;
}

/*
===================
Mod_ClearAll
===================
*/
void Mod_ClearAll (const int clear_level)
{
	int	i;
	model_t	*mod;
#ifdef SUPPORTS_GL_DELETETEXTURES
	static qbool NoFree, Done;

#endif

	// Baker: Is there a good reason we force the sprites and health boxes to reload every map?
	//        Seems quite unnecessary to look for and upload all those textures
	//        Just clear the submodels, sheesh!
	for (i = 0, mod = mod_known ; i < mod_numknown ; i++, mod++)
	{
		switch (clear_level)
		{
		case 0:		if (mod->modelformat != mod_alias &&	// Standard Quake.  Clears all non-alias models
						mod->modelformat != mod_md3)
						mod->needload = true;
					break;

//		case -1:	if (mod->modelformat == mod_brush)		// Baker: "Clear less" world submodels only; speed improvement I hope
//						mod->needload = true;				// And maybe we avoid silly uploading of box textures every map?
//					break;									// Confirm that is a problem.  I'd like to know I solved a real issue
//															// Not an imaginary one.  Baker: Only alias models are moved to cache

		case  1:	mod->needload = true;					// Clear everything
					break;
		}
	}

#if SUPPORTS_GL_DELETETEXTURES

	if (!Done)
	{
		// Some 3dfx miniGLs don't support eglDeleteTextures (i.e. do nothing)
		NoFree = isDedicated || COM_CheckParm ("-nofreetex");
		Done = true;
	}

	if (!NoFree)
		TexMgr_FreeTextures_With_ClearMemory ();
#endif

}

#ifdef _DEBUG
void Memory_Clear_f (void)
{
	Host_ClearMemory ();
}
#endif


/*
==================
Mod_FindName
==================
*/
model_t *Mod_FindName (const char *name)
{
	int	i;
	model_t	*mod;

	if (!name[0])
		Sys_Error ("Mod_FindName: NULL name");

// search the currently loaded models
	for (i = 0, mod = mod_known ; i < mod_numknown ; i++, mod++)
		if (COM_StringMatch (mod->name, name))
			break;

	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
			Sys_Error ("Mod_FindName: mod_numknown == MAX_MOD_KNOWN");

		strcpy (mod->name, name);
		mod->needload = true;
		mod_numknown++;
	}

	return mod;
}

/*
==================
Mod_TouchModel
==================
*/
void Mod_TouchModel (const char *name)
{
	model_t	*mod;

	mod = Mod_FindName (name);

	if (!mod->needload)
	{	// Baker: If the model doesn't need loaded check the Cache if alias or MD3
		//        Other model types don't go to cache
		if (mod->modelformat == mod_alias || mod->modelformat == mod_md3)
			Cache_Check (&mod->cache);
	}
}


/*
==================
Mod_ForName

Loads in a model for the given name
==================
*/
model_t *Mod_ForName (const char *name, qbool crash)
{
	model_t	*mod;

	mod = Mod_FindName (name);

	return Mod_LoadModel (mod, crash);
}



/*
==================
Mod_LoadModel

Loads a model into the cache
==================
*/
model_t *Mod_LoadModel (model_t *mod, qbool crash)
{
	void Mod_LoadModel_Alias  (model_t *mod, void *buffer);
	void Mod_LoadModel_Brush  (model_t *mod, void *buffer);
	void Mod_LoadModel_Sprite (model_t *mod, void *buffer);
	void Mod_LoadQ3Model	  (model_t *mod, void *buffer);

	unsigned	*buf;
	byte		stackbuf[1024];		// avoid dirtying the cache heap

	if (!mod->needload)
	{
		// Baker: Sprites and brush models aren't loaded into the cache
		if (mod->modelformat == mod_alias || mod->modelformat == mod_md3)
		{
			if (Cache_Check (&mod->cache))
				return mod;
			}
		 else
		{
			return mod;		// not cached at all
		}
	}

// because the world is so huge, load it one piece at a time
	if (!crash)
	{

	}

	// Baker: We probably are never path limiting this
	buf = (unsigned *)QFS_LoadStackFile(mod->name, stackbuf, sizeof(stackbuf), NULL /*PATH LIMIT ME*/);
#if SUPPORTS_MISSING_MODELS
	if (!buf && crash)
	{
		if (host_sub_missing_models.integer)
		{
			// Reload with another .mdl
			buf = (unsigned *)QFS_LoadStackFile("missing_model.mdl", stackbuf, sizeof(stackbuf), NULL /*DO NOT PATH LIMIT ME -- SPECIAL CIRCUMSTANCE*/);
			if (buf)
			{
				Con_Printf ("Missing model %s substituted\n", mod->name);
			}
		}
	}
#endif

	if (!buf)
	{
		if (crash)
			Host_Error ("Mod_LoadModel: %s not found", mod->name);
		return NULL;
	}

// allocate a new model
	QCOM_FileBase (mod->name, loadname);

	loadmodel = mod;
	// Update the path
	StringLCopy (mod->loadinfo.searchpath, qfs_lastload.datapath);

// fill it in

// call the apropriate loader
	mod->needload = false;

	switch (LittleLong(*(unsigned *)buf))
	{
		case IDPOLYHEADER:
			Mod_LoadModel_Alias  (mod, buf);
			break;

		case IDMD3HEADER:
			Mod_LoadQ3Model (mod, buf);
			break;

		case IDSPRITEHEADER:
			Mod_LoadModel_Sprite (mod, buf);
			break;

		default:
			Mod_LoadModel_Brush  (mod, buf);
			break;
	}

	return mod;
}

//=============================================================================

/*
================
Mod_Print_f
================
*/
void Mod_Print_f (void)
{
	int		i;
	model_t		*mod;

	Con_Printf ("Cached models:\n");
	for (i = 0, mod = mod_known ; i < mod_numknown ; i++, mod++)
	{
		Con_Printf ("%8p : %s\n", mod->cache.data, mod->name);
	}

}


/*
================
Mod_Print_f
================
*/
void Sizeof_Print_f (void)
{
	Con_Printf ("Client Static  - cls  %i:\n", sizeof(cls));
	Con_Printf ("Client State   - cl   %i:\n", sizeof(cl));
	Con_Printf ("Server State   - sv   %i:\n", sizeof(sv));
	Con_Printf ("Server edict          %i:\n", sizeof(edict_t));
	Con_Printf ("Client entity         %i:\n", sizeof(entity_t));
	Con_Printf ("Model                 %i:\n", sizeof(model_t));
	Con_Printf ("Alias model header    %i:\n", sizeof(aliashdr_t));
	Con_Printf ("Surface               %i:\n", sizeof(msurface_t));
	Con_Printf ("mnode                 %i:\n", sizeof(glpoly_t));
	Con_Printf ("Poly                  %i:\n", sizeof(mnode_t));
	Con_Printf ("Leaf                  %i:\n", sizeof(mleaf_t));
	Con_Printf ("Poly                  %i:\n", sizeof(mclipnode_t));
	Con_Printf ("Hull                  %i:\n", sizeof(hull_t));
	Con_Printf ("Lightmap              %i:\n", sizeof(lightmapinfo_t));
}
