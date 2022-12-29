#if 0


#if SUPPORTS_HLBSP

/*
=============================================================================
WAD3 Texture Loading for BSP 3.0 Support
=============================================================================
*/

#define TEXWAD_MAXIMAGES 16384

typedef struct {
	char name[MAX_QPATH];
	FILE *file;
	int position;
	int size;
} texwadlump_t;


#if SUPPORTS_HLBSP_EXTWADS


static texwadlump_t texwadlump[TEXWAD_MAXIMAGES];

void WAD3_LoadTextureWadFile (char *filename)
{
	lumpinfo_t *lumps, *lump_p;
	wadinfo_t header;
	int i, j, infotableofs, numlumps, lowmark;
	FILE *file;

	if (QFS_FOpenFile (va("textures/halflife/%s", filename), &file))
		goto loaded;
	if (QFS_FOpenFile (va("textures/%s", filename), &file))
		goto loaded;
	if (QFS_FOpenFile (filename, &file))
		goto loaded;

	Host_Error ("Couldn't load halflife wad \"%s\"\n", filename);

loaded:
	if (fread(&header, 1, sizeof(wadinfo_t), file) != sizeof(wadinfo_t))
	{
		Con_Printf ("WAD3_LoadTextureWadFile: unable to read wad header");
		return;
	}

	if (memcmp(header.identification, "WAD3", 4))
	{
		Con_Printf ("WAD3_LoadTextureWadFile: Wad file %s doesn't have WAD3 id\n",filename);
		return;
	}

	numlumps = LittleLong(header.numlumps);
	if (numlumps < 1 || numlumps > TEXWAD_MAXIMAGES)
	{
		Con_Printf ("WAD3_LoadTextureWadFile: invalid number of lumps (%i)\n", numlumps);
		return;
	}

	infotableofs = LittleLong(header.infotableofs);
	if (fseek(file, infotableofs, SEEK_SET))
	{
		Con_Printf ("WAD3_LoadTextureWadFile: unable to seek to lump table");
		return;
	}

	lowmark = Hunk_LowMark();
	if (!(lumps = Hunk_AllocName (numlumps, sizeof(lumpinfo_t), "WAD3")))
	{
		Con_Printf ("WAD3_LoadTextureWadFile: unable to allocate temporary memory for lump table");
		return;
	}

	if (fread(lumps, 1, sizeof(lumpinfo_t) * numlumps, file) != sizeof(lumpinfo_t) * numlumps)
	{
		Con_Printf ("WAD3_LoadTextureWadFile: unable to read lump table");
		Hunk_FreeToLowMark(lowmark);
		return;
	}

	for (i = 0, lump_p = lumps; i < numlumps; i++,lump_p++)
	{
		W_CleanupName (lump_p->name, lump_p->name);
		for (j = 0; j < TEXWAD_MAXIMAGES; j++)
		{
			if (!texwadlump[j].name[0] || COM_StringMatch (lump_p->name, texwadlump[j].name))
				break;
		}
		if (j == TEXWAD_MAXIMAGES)
			break; // we are full, don't load any more
		if (!texwadlump[j].name[0])
			StringLCopy (texwadlump[j].name, lump_p->name, sizeof(texwadlump[j].name));
		texwadlump[j].file = file;
		texwadlump[j].position = LittleLong(lump_p->filepos);
		texwadlump[j].size = LittleLong(lump_p->disksize);
	}

	Hunk_FreeToLowMark(lowmark);
	//leaves the file open
}
#endif

#if SUPPORTS_HLBSP
//converts paletted to rgba
static byte *ConvertWad3ToRGBA(texture_t *tex)
{
	byte *in, *data, *pal;
	int i, p, image_size;

	if (!tex->offsets[0])
		Sys_Error("ConvertWad3ToRGBA: tex->offsets[0] == 0");

	image_size = tex->width * tex->height;
	in = (byte *) ((byte *) tex + tex->offsets[0]);
	data = ImageWork_malloc (image_size * 4); // Baker

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
#endif

byte *WAD3_LoadTexture(miptex_t *mt)
{
	char texname[MAX_QPATH];
	int i, j, lowmark = 0;
	FILE *file;
	miptex_t *tex;
	byte *data;

	if (mt->offsets[0])
		return ConvertWad3ToRGBA(mt);

#ifdef SUPPORTS_HLBSP_EXT_WADS

	texname[sizeof(texname) - 1] = 0;
	W_CleanupName (mt->name, texname);
	for (i = 0; i < TEXWAD_MAXIMAGES; i++)
	{
		if (!texwadlump[i].name[0])
			break;
			
		if (COM_StringNOTMatch (texname, texwadlump[i].name))
			continue;

		file = texwadlump[i].file;
		if (fseek(file, texwadlump[i].position, SEEK_SET))
		{
			Con_Printf("WAD3_LoadTexture: corrupt WAD3 file");
			return NULL;
		}
		lowmark = Hunk_LowMark();
		tex = Hunk_AllocName (1, texwadlump[i].size. "WAD3tex");
		if (fread(tex, 1, texwadlump[i].size, file) < texwadlump[i].size)
		{
			Con_Printf("WAD3_LoadTexture: corrupt WAD3 file");
			Hunk_FreeToLowMark(lowmark);
			return NULL;
		}
		tex->width = LittleLong(tex->width);
		tex->height = LittleLong(tex->height);
		if (tex->width != mt->width || tex->height != mt->height)
		{
			Hunk_FreeToLowMark(lowmark);
			return NULL;
		}
		for (j = 0;j < MIPLEVELS;j++)
			tex->offsets[j] = LittleLong(tex->offsets[j]);
		data = ConvertWad3ToRGBA(tex);
		Hunk_FreeToLowMark(lowmark);
		return data;
	}
#endif

	return NULL;
}

#endif

#endif