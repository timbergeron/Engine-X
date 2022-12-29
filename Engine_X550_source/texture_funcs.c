#include "quakedef.h"

// QPAL: Single byte per pixel

qbool Texture_QPAL_HasFullbrights (const byte *qpal_pixels, const int size)
{
	int	i;

	for (i=0 ; i<size ; i++)
		if (qpal_pixels[i] >= 224)
			return true;

	return false;
}

qbool Texture_QPAL_HasMaskColor255 (const byte *qpal_pixels, const int size)
{
	int	i;

	for (i=0 ; i<size ; i++)
		if (qpal_pixels[i] == 255)
			return true;

	return false;
}

void ResampleTextureLerpLine (byte *in, byte *out, int inwidth, int outwidth)
{
	int	j, xi, oldx = 0, f, fstep, endx = (inwidth - 1), lerp;

	fstep = (inwidth << 16) / outwidth;
	for (j = 0, f = 0 ; j < outwidth ; j++, f += fstep)
	{
		xi = f >> 16;
		if (xi != oldx)
		{
			in += (xi - oldx) * 4;
			oldx = xi;
		}

		if (xi < endx)
		{
			lerp = f & 0xFFFF;
			*out++ = (byte)((((in[4] - in[0]) * lerp) >> 16) + in[0]);
			*out++ = (byte)((((in[5] - in[1]) * lerp) >> 16) + in[1]);
			*out++ = (byte)((((in[6] - in[2]) * lerp) >> 16) + in[2]);
			*out++ = (byte)((((in[7] - in[3]) * lerp) >> 16) + in[3]);
		}
		else
		{
			// last pixel of the line has no pixel to lerp to
			*out++ = in[0];
			*out++ = in[1];
			*out++ = in[2];
			*out++ = in[3];
		}
	}
}

/*
================
ResampleTexture
================
*/
void ResampleTexture (const unsigned *indata, const int inwidth, const int inheight, unsigned *outdata, const int outwidth, const int outheight, const qbool quality)
{
	if (quality)
	{
		int		i, j, yi, oldy, f, fstep, endy = (inheight - 1), lerp;
		int		inwidth4 = inwidth * 4, outwidth4 = outwidth * 4;
		byte	*inrow, *out, *row1, *row2, *memalloc;

		out = (byte *)outdata;
		fstep = (inheight << 16) / outheight;

		memalloc = ImageWork_malloc (2 * outwidth4, "Resample texture with lerp on");
		row1 = memalloc;
		row2 = memalloc + outwidth4;

		inrow = (byte *)indata;
		oldy = 0;

		ResampleTextureLerpLine (inrow, row1, inwidth, outwidth);
		ResampleTextureLerpLine (inrow + inwidth4, row2, inwidth, outwidth);

		for (i = 0, f = 0 ; i < outheight ; i++, f += fstep)
		{
			yi = f >> 16;
			if (yi < endy)
			{
				lerp = f & 0xFFFF;

				if (yi != oldy)
				{
					inrow = (byte *)indata + inwidth4 * yi;
					if (yi == oldy+1)
						memcpy (row1, row2, outwidth4);
					else
						ResampleTextureLerpLine (inrow, row1, inwidth, outwidth);

					ResampleTextureLerpLine (inrow + inwidth4, row2, inwidth, outwidth);
					oldy = yi;
				}

				for (j = outwidth ; j ; j--)
				{
					out[0] = (byte)((((row2[0] - row1[0]) * lerp) >> 16) + row1[0]);
					out[1] = (byte)((((row2[1] - row1[1]) * lerp) >> 16) + row1[1]);
					out[2] = (byte)((((row2[2] - row1[2]) * lerp) >> 16) + row1[2]);
					out[3] = (byte)((((row2[3] - row1[3]) * lerp) >> 16) + row1[3]);
					out += 4;
					row1 += 4;
					row2 += 4;
				}
				row1 -= outwidth4;
				row2 -= outwidth4;
			}
			else
			{
				if (yi != oldy)
				{
					inrow = (byte *)indata + inwidth4 * yi;
					if (yi == oldy+1)
						memcpy(row1, row2, outwidth4);
					else
						ResampleTextureLerpLine (inrow, row1, inwidth, outwidth);
					oldy = yi;
				}
				memcpy (out, row1, outwidth4);
			}
		}
		ImageWork_free (memalloc);
	}
	else
	{
		int		i, j;
		unsigned int frac, fracstep, *inrow, *out;

		out = outdata;

		fracstep = inwidth * 0x10000 / outwidth;
		for (i = 0 ; i < outheight ; i++)
		{
			inrow = (int *)indata + inwidth * (i * inheight / outheight);
			frac = fracstep >> 1;
			j = outwidth - 4;
			while (j >= 0)
			{
				out[0] = inrow[frac >> 16]; frac += fracstep;
				out[1] = inrow[frac >> 16]; frac += fracstep;
				out[2] = inrow[frac >> 16]; frac += fracstep;
				out[3] = inrow[frac >> 16]; frac += fracstep;
				out += 4;
				j -= 4;
			}
			if (j & 2)
			{
				out[0] = inrow[frac >> 16]; frac += fracstep;
				out[1] = inrow[frac >> 16]; frac += fracstep;
				out += 2;
			}
			if (j & 1)
			{
				out[0] = inrow[frac >> 16]; frac += fracstep;
				out += 1;
			}
		}
	}
}

/*
================
MipMap

Operates in place, quartering the size of the texture
================
*/
void MipMap (byte *in, int *width, int *height)
{
	int		i, j, nextrow;
	byte	*out;

	nextrow = *width << 2;

	out = in;
	if (*width > 1)
	{
		*width >>= 1;
		if (*height > 1)
		{
			*height >>= 1;
			for (i = 0 ; i < *height ; i++, in += nextrow)
			{
				for (j = 0 ; j < *width ; j++, out += 4, in += 8)
				{
					out[0] = (in[0] + in[4] + in[nextrow+0] + in[nextrow+4]) >> 2;
					out[1] = (in[1] + in[5] + in[nextrow+1] + in[nextrow+5]) >> 2;
					out[2] = (in[2] + in[6] + in[nextrow+2] + in[nextrow+6]) >> 2;
					out[3] = (in[3] + in[7] + in[nextrow+3] + in[nextrow+7]) >> 2;
				}
			}
		}
		else
		{
			for (i = 0 ; i < *height ; i++)
			{
				for (j = 0 ; j < *width ; j++, out += 4, in += 8)
				{
					out[0] = (in[0] + in[4]) >> 1;
					out[1] = (in[1] + in[5]) >> 1;
					out[2] = (in[2] + in[6]) >> 1;
					out[3] = (in[3] + in[7]) >> 1;
				}
			}
		}
	}
	else if (*height > 1)
	{
		*height >>= 1;
		for (i = 0 ; i < *height ; i++, in += nextrow)
		{
			for (j = 0 ; j < *width ; j++, out += 4, in += 4)
			{
				out[0] = (in[0] + in[nextrow+0]) >> 1;
				out[1] = (in[1] + in[nextrow+1]) >> 1;
				out[2] = (in[2] + in[nextrow+2]) >> 1;
				out[3] = (in[3] + in[nextrow+3]) >> 1;
			}
		}
	}
}



// Image functions that don't require anything

qbool Check_RGBA_Pixels_For_AlphaChannel (const byte *data, const int imagesize_as_width_times_height)
{
	int i;
	for (i=0 ; i<imagesize_as_width_times_height ; i++)
		if (((((unsigned *)data)[i] >> 24) & 0xFF) < 255)	// (unsigned *) makes us step by 4 byte instead of 1 (byte *)
			return true;	// Alpha channel pixel found

	return false;	// No alpha channel exists.  100% solid alpha
}

qbool Check_QPAL_Copy_To_RGBA_Return_True_If_AlphaChannel (const byte *src, unsigned *dest, const int imagesize_as_width_times_height)
{
	int		i;
	int		this_pixel_qpal_color;
	qbool	isAlpha = false;

	for (i=0 ; i<imagesize_as_width_times_height ; i++)
	{
		this_pixel_qpal_color = src[i];

		if (this_pixel_qpal_color == 255)
			isAlpha = true;

		dest[i] = d_8to24table[this_pixel_qpal_color];				// fullbright
	}

	return isAlpha;
}


void Check_QPAL_Copy_To_RGBA_Make_NonFullBright_Pixels_Transparent (const byte *src, unsigned *dest, const int imagesize_as_width_times_height)
{
	int	i;
	int	this_pixel_qpal_color;

	for (i=0 ; i<imagesize_as_width_times_height ; i++)
	{
		this_pixel_qpal_color = src[i];

		if (this_pixel_qpal_color < 224)
			dest[i] = d_8to24table[this_pixel_qpal_color] & 0x00FFFFFF;	// transparent (Sets alpha channel to 0)
		else
			dest[i] = d_8to24table[this_pixel_qpal_color];				// fullbright
	}

}

// Returns info on if has fullbrights
qbool QPAL_Colorize_Skin_To_QPAL (const byte *original_src, byte *dest, const int imagesize, const int myShirt, const int myPants)
{
	byte		skin_color_mapped_pal[256];
	int			i;
	qbool		fullbrights_detected = false;

	// Blank our table so that mapping 1 to 1 normal coloring
	for (i=0 ; i<256 ; i++) skin_color_mapped_pal[i] = i;

	// Alter the rows 1 and 6 and colorize them to the Quake palette based on colors
	for (i=0 ; i<16 ; i++)
	{   // the artists made some backwards ranges. sigh.
		skin_color_mapped_pal[TOP_RANGE+i]    = (myShirt < 128) ? myShirt + i : myShirt + 15 - i;
		skin_color_mapped_pal[BOTTOM_RANGE+i] = (myPants < 128) ? myPants + i : myPants + 15 - i;
	}

	// Color the destination skin
	for (i = 0 ; i < imagesize ; i++)
	{
		dest[i] = skin_color_mapped_pal[original_src[i]];
		if (dest[i] >= 224)
			fullbrights_detected = true;
	}

	return fullbrights_detected;

}


void QPAL_To_RGBA (const byte *src, unsigned *dest, const int imagesize_as_width_times_height)
{
	int	i;
	int	this_pixel_qpal_color;

	// Baker: Do we need to memset to 0 or 255?
	for (i = 0 ; i < imagesize_as_width_times_height ; i++)
	{
		this_pixel_qpal_color = src[i];
		dest[i] = d_8to24table[this_pixel_qpal_color];
	}

}



int Find_Power_Of_Two_Size (const int this_size)
{
	int glsize;
	for (glsize = 1 ; glsize < this_size ; glsize <<= 1)
		;

	return glsize;
}

void Copy_RGBA_Pixels_Into_Larger_Buffer (const byte *src, byte *dest, const int src_width, const int src_height, const int dest_width, const int dest_height)
{
	int	i;
	for (i=0 ; i<src_height ; i++)
	{
		memcpy (dest, src, src_width * 4);
		src += src_width * 4;
		dest += dest_width * 4;
	}
}

void Copy_Byte_Pixels_Into_Larger_Buffer (const byte *src, byte *dest, const int src_width, const int src_height, const int dest_width, const int dest_height)
{
	int	i;
	for (i=0 ; i<src_height ; i++)
	{
		memcpy (dest, src, src_width);
		src += src_width;
		dest += dest_width;
	}
}


// Ever so slightly Quake locked
void ScaleDimensions (const int width, const int height, int *scaled_width, int *scaled_height, const int mode)
{
	int	maxsize = gl_max_size_default;

	*scaled_width  = Find_Power_Of_Two_Size (width);
	*scaled_height = Find_Power_Of_Two_Size (height);

	if (mode & TEX_MIPMAP)
	{
		*scaled_width  >>= (int)CLAMP (0, tex_picmip.integer, 3); // Baker: limit from 0 to 3
		*scaled_height >>= (int)CLAMP (0, tex_picmip.integer, 3); // Baker: limit from 0 to 3
	}

//	maxsize = (mode & TEX_MIPMAP) ? gl_max_size.floater : gl_max_size_default;

	*scaled_width  = CLAMP (1, *scaled_width,  maxsize);
	*scaled_height = CLAMP (1, *scaled_height, maxsize);
}

void RGBA_Zero_Fill_AlphaChannel (byte *data, const int imagesize_as_width_times_height)
{
	int i;
	for (i=0 ; i < imagesize_as_width_times_height ; i++)
	{
		if (data[i*4] == data[0] && data[i*4+1] == data[1] && data[i*4+2] == data[2])
			data[i*4+3] = 0;
	}
}

void RGBA_Fill_AlphaChannel_On_Non_Black (unsigned int *data, const int imagesize_as_width_times_height)
{
	int i;
	for (i=0 ; i < imagesize_as_width_times_height ; i++)
	{
		if (data[i] & 0x00FFFFFF)
			data[i] &= 0x00FFFFFF;	// Strip the alpha channel
		else
			data[i] |= 0xFF000000;	// Add the alpha channel
	}
}


byte *StringToRGB (char *s)
{
	byte		*col;
	static	byte	rgb[4];

	Cmd_TokenizeString (s);
	if (Cmd_Argc() == 3)
	{
		rgb[0] = (byte)atoi(Cmd_Argv(0));
		rgb[1] = (byte)atoi(Cmd_Argv(1));
		rgb[2] = (byte)atoi(Cmd_Argv(2));
	}
	else
	{
		col = (byte *)&d_8to24table[(byte)atoi(s)];
		rgb[0] = col[0];
		rgb[1] = col[1];
		rgb[2] = col[2];
	}
	rgb[3] = 0;

	return rgb;
}


void SpaceOutCharset (const byte *src, byte *dest, const int colwidth, const int imageheight, const int rowheight, const int bytes_per_pixel)
{
	int		numrows = imageheight / rowheight;	// Always 16 ... like 128/8 = 16 or 256/16 = 16 or 512 / 32 = 16
	int		bufsize;
	int		i;

	bufsize = sizeof(dest);

	if (bytes_per_pixel == QPAL_BYTES_PER_PIXEL_IS_1)
		memset (dest, 255, sizeof(dest));	// Fill with 255, QPAL transparent color
	else
		memset (dest,   0, sizeof(dest));	// Fill with 0,   RGBA transparent color (black with alpha of 0)

	for (i=0 ; i<numrows ; i++)
	{
		memcpy (dest, src, colwidth * rowheight * bytes_per_pixel);		// Copy the row
		src += colwidth * rowheight * bytes_per_pixel;					// Advance one  row in the source
		dest += colwidth * rowheight * bytes_per_pixel * 2;				// Advance two rows in the dest (double spacing it)
	}
}

void FlipBuffer (byte *buffer, const int columns, const int rows, const int BytesPerPixel)	// Flip the image because of GL's up = positive-y
{
    byte	*tb1, *tb2;
    int		offset1, offset2;
	int		i,bufsize;

    bufsize = columns * BytesPerPixel; // bufsize=widthBytes;

    tb1= ImageWork_malloc (bufsize, "Flip buffer");
    tb2= ImageWork_malloc (bufsize, "Flip buffer2");

    for (i=0;i<(rows+1)/2;i++)
    {
        offset1= i * bufsize;
        offset2=((rows-1)-i) * bufsize;

        memcpy(tb1,				buffer+offset1,	bufsize);
        memcpy(tb2,				buffer+offset2, bufsize);
        memcpy(buffer+offset1,	tb2,			bufsize);
        memcpy(buffer+offset2,	tb1,			bufsize);
    }

//	Q_free (tb1);
//	Q_free (tb2);
	ImageWork_free (tb1);
	ImageWork_free (tb2);
    return;
}

/*
static void SwapRows (int columns, int rows, int colordepth, byte *buffer)	// Flip the image because of GL's up = positive-y
{
	byte	*swaprow;
	int i, j;

	swaprow = ImageWork_malloc (columns * colordepth);
	for (i=0, j=rows-1; i < rows; i++, j--)    // line counter
	{
		SetWindowText(engine.platform->mainwindow, va("%s i %i j %i", "string", i,j));

		memcpy (&swaprow[0],        &buffer[i*columns], columns * colordepth);
		memcpy (&buffer[i*columns], &buffer[j*columns], columns * colordepth);
		memcpy (&buffer[j*columns], &swaprow[0],        columns * colordepth);
	}
	ImageWork_free (swaprow);
}
*/


//=========================================================

/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes - Ed
=================
*/

typedef struct
{
	short	x, y;
} floodfill_t;

// must be a power of 2
#define	FLOODFILL_FIFO_SIZE	0x1000
#define	FLOODFILL_FIFO_MASK	(FLOODFILL_FIFO_SIZE - 1)

#define FLOODFILL_STEP(off, dx, dy)							\
{															\
	if (pos[off] == fillcolor)								\
	{														\
		pos[off] = 255;										\
		fifo[inpt].x = x + (dx), fifo[inpt].y = y + (dy);	\
		inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;			\
	}								\
	else if (pos[off] != 255) fdc = pos[off];				\
}

/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes - Ed
=================
*/
void Mod_FloodFillSkin (byte *skin, int skinwidth, int skinheight)
{
	int			i, inpt = 0, outpt = 0, filledcolor = -1;
	byte		fillcolor = *skin;	// assume this is the pixel to fill
	floodfill_t	fifo[FLOODFILL_FIFO_SIZE];

	if (filledcolor == -1)
	{
		filledcolor = 0;
		// attempt to find opaque black in QPAL (Remember, this doesn't have to be the standard Quake Palette ...)
		for (i=0 ; i<256 ; ++i)
		{
			if (d_8to24table[i] == 255)	// RGB: 0,0,0 with A: 1 --> Solid black
			{
				filledcolor = i;
				break;
			}
		}
	}

	// can't fill to filled color or to transparent color (used as visited marker)
	if ((fillcolor == filledcolor) || (fillcolor == 255))
	{
		//printf( "not filling skin from %d to %d\n", fillcolor, filledcolor );
		return;
	}

	fifo[inpt].x = 0, fifo[inpt].y = 0;
	inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;

	while (outpt != inpt)
	{
		int		x = fifo[outpt].x;
		int		y = fifo[outpt].y;
		int		fdc = filledcolor;
		byte	*pos = &skin[x+skinwidth*y];

		outpt = (outpt + 1) & FLOODFILL_FIFO_MASK;

		if (x > 0)					FLOODFILL_STEP(-1, -1, 0);
		if (x < skinwidth - 1)		FLOODFILL_STEP(1, 1, 0);
		if (y > 0)					FLOODFILL_STEP(-skinwidth, 0, -1);
		if (y < skinheight - 1)		FLOODFILL_STEP(skinwidth, 0, 1);
		skin[x+skinwidth*y] = fdc;
	}
}