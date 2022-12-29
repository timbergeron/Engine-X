/*
Copyright (C) 2001-2002       A Nourai

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
// image.c -- handling images

#include "quakedef.h"

#if SUPPORTS_LIBJPEG
#ifndef _WIN32 // Headers
#include "jpeg-linux/jpeglib.h"	// FIXME!!!
#else
#pragma comment (lib,    "libjpeg/lib/libjpeg.lib")
#include "libjpeg\includes\jpeglib.h"
#endif
#endif

#if SUPPORTS_LIBPNG
#include "png.h"
#endif

#define	IMAGE_MAX_DIMENSIONS	4096

//int	image_width, image_height;

/*
=========================================================

			Targa

=========================================================
*/

#define TGA_MAXCOLORS 16384

/* Definitions for image types. */
#define TGA_Null	0	/* no image data */
#define TGA_Map		1	/* Uncompressed, color-mapped images. */
#define TGA_RGB		2	/* Uncompressed, RGB images. */
#define TGA_Mono	3	/* Uncompressed, black and white images. */
#define TGA_RLEMap	9	/* Runlength encoded color-mapped images. */
#define TGA_RLERGB	10	/* Runlength encoded RGB images. */
#define TGA_RLEMono	11	/* Compressed, black and white images. */
#define TGA_CompMap	32	/* Compressed color-mapped data, using Huffman, Delta, and runlength encoding. */
#define TGA_CompMap4	33	/* Compressed color-mapped data, using Huffman, Delta, and runlength encoding. 4-pass quadtree-type process. */

/* Definitions for interleave flag. */
#define TGA_IL_None	0	/* non-interleaved. */
#define TGA_IL_Two	1	/* two-way (even/odd) interleaving */
#define TGA_IL_Four	2	/* four way interleaving */
#define TGA_IL_Reserved	3	/* reserved */

/* Definitions for origin flag */
#define TGA_O_UPPER	0	/* Origin in lower left-hand corner. */
#define TGA_O_LOWER	1	/* Origin in upper left-hand corner. */

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;

int fgetLittleShort (FILE *f)
{
	byte	b1, b2;

	b1 = fgetc(f);
	b2 = fgetc(f);

	return (short)(b1 + b2*256);
}

/*
=============
Image_LoadTGA
=============
*/
// Baker: sigh ... this is Quake containminated
byte *Image_LoadTGA_FromOpenFile (FILE *fin, int *my_image_width, int *my_image_height)
{
	int		w, h, x, y, realrow, truerow, baserow, i, temp1, temp2, pixel_size, map_idx;
	int		RLE_count, RLE_flag, size, interleave, origin;
	qbool	mapped, rlencoded;
	byte		*data, *dst, r, g, b, a, j, k, l, *ColorMap;
	TargaHeader	header;

	/* load file */
	if (!fin)
		return NULL;

	header.id_length = fgetc (fin);
	header.colormap_type = fgetc (fin);
	header.image_type = fgetc (fin);

	header.colormap_index = fgetLittleShort (fin);
	header.colormap_length = fgetLittleShort (fin);
	header.colormap_size = fgetc (fin);
	header.x_origin = fgetLittleShort (fin);
	header.y_origin = fgetLittleShort (fin);
	header.width = fgetLittleShort (fin);
	header.height = fgetLittleShort (fin);
	header.pixel_size = fgetc (fin);
	header.attributes = fgetc (fin);

	if (header.width > IMAGE_MAX_DIMENSIONS || header.height > IMAGE_MAX_DIMENSIONS)
	{
		Con_Printf ("TGA image exceeds maximum supported dimensions\n");
#pragma message ("Quality assurance: State the file name and the violating dimension and the max dimension")
//		fclose (fin);		Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

//	if ((matchwidth && header.width != matchwidth) || (matchheight && header.height != matchheight))
//	{
//		Con_Printf ("TGA image matchwidth or matchheight\n");
//		fclose (fin);
//		return NULL;
//	}

	if (header.id_length != 0)
		fseek (fin, header.id_length, SEEK_CUR);

	/* validate TGA type */
	switch (header.image_type)
	{
	case TGA_Map:
	case TGA_RGB:
	case TGA_Mono:
	case TGA_RLEMap:
	case TGA_RLERGB:
	case TGA_RLEMono:
		break;

	default:
		Con_DevPrintf (DEV_IMAGE, "Unsupported TGA image: Only type 1 (map), 2 (RGB), 3 (mono), 9 (RLEmap), 10 (RLERGB), 11 (RLEmono) TGA images supported\n");
		//fclose (fin);  // Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

	/* validate color depth */
	switch (header.pixel_size)
	{
	case 8:
	case 15:
	case 16:
	case 24:
	case 32:
		break;

	default:
		Con_Printf ("Unsupported TGA image: Only 8, 15, 16, 24 or 32 bit images (with colormaps) supported\n");
		//fclose (fin);	// Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

	r = g = b = a = l = 0;

	/* if required, read the color map information. */
	ColorMap = NULL;
	mapped = (header.image_type == TGA_Map || header.image_type == TGA_RLEMap) && header.colormap_type == 1;
	if (mapped)
	{
		/* validate colormap size */
		switch (header.colormap_size)
		{
		case 8:
		case 15:
		case 16:
		case 32:
		case 24:
			break;

		default:
			Con_Printf ("Unsupported TGA image %s: Only 8, 15, 16, 24 or 32 bit colormaps supported\n");
			//fclose (fin);	// Baker: Didn't open file, shouldn't be closing it
			return NULL;
		}

		temp1 = header.colormap_index;
		temp2 = header.colormap_length;
		if ((temp1 + temp2 + 1) >= TGA_MAXCOLORS)
		{
			//fclose (fin);	// Baker: Didn't open file, shouldn't be closing it
			return NULL;
		}
		ColorMap = ImageWork_malloc (TGA_MAXCOLORS * 4, "temp TGA color map");
		map_idx = 0;
		for (i = temp1 ; i < temp1 + temp2 ; ++i, map_idx += 4)
		{
			/* read appropriate number of bytes, break into rgb & put in map. */
			switch (header.colormap_size)
			{
			case 8:	/* grey scale, read and triplicate. */
				r = g = b = getc (fin);
				a = 255;
				break;

			case 15:	/* 5 bits each of red green and blue. */
						/* watch byte order. */
				j = getc (fin);
				k = getc (fin);
				l = ((unsigned int)k << 8) + j;
				r = (byte)(((k & 0x7C) >> 2) << 3);
				g = (byte)((((k & 0x03) << 3) + ((j & 0xE0) >> 5)) << 3);
				b = (byte)((j & 0x1F) << 3);
				a = 255;
				break;

			case 16:	/* 5 bits each of red green and blue, 1 alpha bit. */
						/* watch byte order. */
				j = getc (fin);
				k = getc (fin);
				l = ((unsigned int)k << 8) + j;
				r = (byte)(((k & 0x7C) >> 2) << 3);
				g = (byte)((((k & 0x03) << 3) + ((j & 0xE0) >> 5)) << 3);
				b = (byte)((j & 0x1F) << 3);
				a = (k & 0x80) ? 255 : 0;
				break;

			case 24:	/* 8 bits each of blue, green and red. */
				b = getc (fin);
				g = getc (fin);
				r = getc (fin);
				a = 255;
				l = 0;
				break;

			case 32:	/* 8 bits each of blue, green, red and alpha. */
				b = getc (fin);
				g = getc (fin);
				r = getc (fin);
				a = getc (fin);
				l = 0;
				break;
			}
			ColorMap[map_idx+0] = r;
			ColorMap[map_idx+1] = g;
			ColorMap[map_idx+2] = b;
			ColorMap[map_idx+3] = a;
		}
	}

	/* check run-length encoding. */
	rlencoded = (header.image_type == TGA_RLEMap || header.image_type == TGA_RLERGB || header.image_type == TGA_RLEMono);
	RLE_count = RLE_flag = 0;

	*my_image_width = w = header.width;
	*my_image_height = h = header.height;

	size = w * h * 4;
	data = ImageWork_malloc (size * 1, "TGA loader");

	/* read the Targa file body and convert to portable format. */
	pixel_size = header.pixel_size;
	origin = (header.attributes & 0x20) >> 5;
	interleave = (header.attributes & 0xC0) >> 6;
	truerow = baserow = 0;
	for (y=0 ; y<h ; y++)
	{
		realrow = truerow;
		if (origin == TGA_O_UPPER)
			realrow = h - realrow - 1;

		dst = data + realrow * w * 4;

		for (x=0 ; x<w ; x++)
		{
			/* check if run length encoded. */
			if (rlencoded)
			{
				if (!RLE_count)
				{
					/* have to restart run. */
					i = getc (fin);
					RLE_flag = (i & 0x80);
					if (!RLE_flag)	// stream of unencoded pixels
						RLE_count = i + 1;
					else		// single pixel replicated
						RLE_count = i - 127;
					/* decrement count & get pixel. */
					--RLE_count;
				}
				else
				{
					/* have already read count & (at least) first pixel. */
					--RLE_count;
					if (RLE_flag)
						/* replicated pixels. */
						goto PixEncode;
				}
			}

			/* read appropriate number of bytes, break into RGB. */
			switch (pixel_size)
			{
			case 8:	/* grey scale, read and triplicate. */
				r = g = b = l = getc (fin);
				a = 255;
				break;

			case 15:	/* 5 bits each of red green and blue. */
						/* watch byte order. */
				j = getc (fin);
				k = getc (fin);
				l = ((unsigned int)k << 8) + j;
				r = (byte)(((k & 0x7C) >> 2) << 3);
				g = (byte)((((k & 0x03) << 3) + ((j & 0xE0) >> 5)) << 3);
				b = (byte)((j & 0x1F) << 3);
				a = 255;
				break;

			case 16:	/* 5 bits each of red green and blue, 1 alpha bit. */
						/* watch byte order. */
				j = getc (fin);
				k = getc (fin);
				l = ((unsigned int)k << 8) + j;
				r = (byte)(((k & 0x7C) >> 2) << 3);
				g = (byte)((((k & 0x03) << 3) + ((j & 0xE0) >> 5)) << 3);
				b = (byte)((j & 0x1F) << 3);
				a = (k & 0x80) ? 255 : 0;
				break;

			case 24:	/* 8 bits each of blue, green and red. */
				b = getc (fin);
				g = getc (fin);
				r = getc (fin);
				a = 255;
				l = 0;
				break;

			case 32:	/* 8 bits each of blue, green, red and alpha. */
				b = getc (fin);
				g = getc (fin);
				r = getc (fin);
				a = getc (fin);
				l = 0;
				break;

			default:
				Con_Printf ("Malformed TGA image: Illegal pixel_size '%d'\n", pixel_size);
				// fclose (fin);	// Baker: Didn't open file, shouldn't be closing it
				ImageWork_free (data);
				if (mapped)
					ImageWork_free (ColorMap);
				return NULL;
			}

PixEncode:
			if (mapped)
			{
				map_idx = l * 4;
				*dst++ = ColorMap[map_idx+0];
				*dst++ = ColorMap[map_idx+1];
				*dst++ = ColorMap[map_idx+2];
				*dst++ = ColorMap[map_idx+3];
			}
			else
			{
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				*dst++ = a;
			}
		}

		if (interleave == TGA_IL_Four)
			truerow += 4;
		else if (interleave == TGA_IL_Two)
			truerow += 2;
		else
			truerow++;
		if (truerow >= h)
			truerow = ++baserow;
	}

	if (mapped)
		ImageWork_free (ColorMap);
	//fclose (fin);	// Baker: Didn't open file, shouldn't be closing it

	return data;
}

/*
=============
Image_WriteTGA
=============
*/
qbool Image_WriteTGA (const char *absolute_filename, const byte *pixels, const int width, const int height, const int BitsPerPixel, const qbool IsUpsideDown)
{
	byte		*buffer, temp;
	int			i;
	int			size = width * height * ((BitsPerPixel == 32) ? 4 : 3);
	qbool		retval = true;
	int			buffersize_tga = size + 18;  // pixel data plus the header

	//DMSG (va("Allocating %i %i %i = size %i", width, height, bpp, size));

	buffer = ImageWork_malloc (buffersize_tga, "Temp write TGA");	// Allocate memory

	// Header
	memset (buffer, 0, 18);
	buffer[2] = 2;
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = BitsPerPixel;  // 32 or 24;
	if (IsUpsideDown)
		buffer[17] = 0x20; //upside-down attribute

	// Pixel data
	memcpy (buffer + 18, pixels, size);
	{
		int bytes = BitsPerPixel / 8;
		for (i = 18 ; i < size + 18 ; i += bytes)
		{
			temp = buffer[i];

			// Swap red and blue bytes
			buffer[i] = buffer[i+2];
			buffer[i+2] = temp;
		}
	}

	{
		FILE	*f;
		if (!(f = FS_fopen_write(absolute_filename, "wb", FS_CREATE_PATH)))   // FWRITE
		{
			retval = false;
			goto fail;
		}

		fwrite (buffer, 1,buffersize_tga, f);
		fclose (f);
	}

fail:
	ImageWork_free (buffer);

	return retval;
}

#if SUPPORTS_LIBPNG
/*
=========================================================

			PNG

=========================================================
*/

static void PNG_IO_user_read_data (png_structp png_ptr, png_bytep data, png_size_t length)
{
	FILE	*f = (FILE *)png_get_io_ptr(png_ptr);

	fread (data, 1, length, f);
}

static void PNG_IO_user_write_data (png_structp png_ptr, png_bytep data, png_size_t length)
{
	FILE	*f = (FILE *)png_get_io_ptr(png_ptr);

	fwrite (data, 1, length, f);
}

static void PNG_IO_user_flush_data (png_structp png_ptr)
{
	FILE	*f = (FILE *)png_get_io_ptr(png_ptr);

	fflush (f);
}


/*
=============
Image_LoadPNG
=============
*/
byte *Image_LoadPNG_FromOpenFile (const FILE *fin, int *my_image_width, int *my_image_height)
{
	int			y, width, height, bitdepth, colortype;
	int			interlace, compression, filter, bytesperpixel;
	byte		header[8], **rowpointers, *data;
	png_structp	png_ptr;
	png_infop	pnginfo;
	unsigned long	rowbytes;

	if (!fin)
		return NULL;

	fread (header, 1, 8, fin);

	if (png_sig_cmp(header, 0, 8))
	{
		Con_Printf ("Invalid PNG image %s\n");
		//fclose (fin);	// Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

	if (!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
	{
		Con_Printf ("Invalid PNG image error 2\n");
		//fclose (fin);	// Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

	if (!(pnginfo = png_create_info_struct(png_ptr)))
	{
		Con_Printf ("Invalid PNG image error 3\n");
		png_destroy_read_struct (&png_ptr, &pnginfo, NULL);
		//fclose (fin);		// Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

	if (setjmp(png_ptr->jmpbuf))
	{
		Con_Printf ("Invalid PNG image error 4\n");
		png_destroy_read_struct (&png_ptr, &pnginfo, NULL);
		//fclose (fin);		// Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

	png_set_read_fn (png_ptr, fin, PNG_IO_user_read_data);
	png_set_sig_bytes (png_ptr, 8);
	png_read_info (png_ptr, pnginfo);
	png_get_IHDR (png_ptr, pnginfo, (png_uint_32 *)&width, (png_uint_32 *)&height, &bitdepth, &colortype, &interlace, &compression, &filter);

	if (width > IMAGE_MAX_DIMENSIONS || height > IMAGE_MAX_DIMENSIONS)
	{
		Con_Printf ("PNG image exceeds maximum supported dimensions\n");
#pragma message ("Quality assurance: State the file name and the violating dimension and the max dimension")
		png_destroy_read_struct (&png_ptr, &pnginfo, NULL);
		//fclose (fin);	// Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

//	if ((matchwidth && width != matchwidth) || (matchheight && height != matchheight))
//	{
//		Con_Printf ("PNG matchwidth or PNG matchheight\n");
//		png_destroy_read_struct (&png_ptr, &pnginfo, NULL);
//		fclose (fin);
//		return NULL;
//	}

	if (colortype == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb (png_ptr);
		png_set_filler (png_ptr, 255, PNG_FILLER_AFTER);
	}

	if (colortype == PNG_COLOR_TYPE_GRAY && bitdepth < 8)
		png_set_gray_1_2_4_to_8 (png_ptr);

	if (png_get_valid(png_ptr, pnginfo, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha (png_ptr);

	if (colortype == PNG_COLOR_TYPE_GRAY || colortype == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb (png_ptr);

	if (colortype != PNG_COLOR_TYPE_RGBA)
		png_set_filler (png_ptr, 255, PNG_FILLER_AFTER);

	if (bitdepth < 8)
		png_set_expand (png_ptr);
	else if (bitdepth == 16)
		png_set_strip_16 (png_ptr);

	png_read_update_info (png_ptr, pnginfo);
	rowbytes = png_get_rowbytes (png_ptr, pnginfo);
	bytesperpixel = png_get_channels (png_ptr, pnginfo);
	bitdepth = png_get_bit_depth (png_ptr, pnginfo);

	if (bitdepth != 8 || bytesperpixel != 4)
	{
		Con_DevPrintf ("Unsupported PNG image: Bad color depth and/or bpp\n");
		png_destroy_read_struct (&png_ptr, &pnginfo, NULL);
		//fclose (fin);	// Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

	data = ImageWork_malloc (height * rowbytes);
	rowpointers = ImageWork_malloc (height * sizeof(*rowpointers));

	for (y=0 ; y<height ; y++)
		rowpointers[y] = data + y * rowbytes;

	*my_image_width = width;
	*my_image_height = height;

	png_read_image (png_ptr, rowpointers);
	png_read_end (png_ptr, NULL);

	png_destroy_read_struct (&png_ptr, &pnginfo, NULL);
	ImageWork_free (rowpointers);
	//fclose (fin);	// Baker: Didn't open file, shouldn't be closing it

	return data;
}

/*
=============
Image_WritePNG
=============
*/
qbool Image_WritePNG (const char *absolute_filename, const int compression, byte *pixels, const int width, const int height)
{
	int			i, bpp = 3, pngformat, width_sign;
	FILE		*fp;
	png_structp	png_ptr;
	png_infop	info_ptr;
	png_byte	**rowpointers;

	width_sign = (width < 0) ? -1 : 1;
	width = abs(width);

	if (!(fp = FS_fopen(absolute_filename, "wb", FS_CREATE_PATH)))
		return false;

	if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
	{
		fclose (fp);
		return false;
	}

	if (!(info_ptr = png_create_info_struct(png_ptr)))
	{
		png_destroy_write_struct (&png_ptr, (png_infopp)NULL);
		fclose (fp);
		return false;
	}

	if (setjmp(png_ptr->jmpbuf))
	{
		png_destroy_write_struct (&png_ptr, &info_ptr);
		fclose (fp);
		return false;
	}

	png_set_write_fn (png_ptr, fp, PNG_IO_user_write_data, PNG_IO_user_flush_data);
	png_set_compression_level (png_ptr, CLAMP (Z_NO_COMPRESSION, compression, Z_BEST_COMPRESSION));

	pngformat = (bpp == 4) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
	png_set_IHDR (png_ptr, info_ptr, width, height, 8, pngformat, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info (png_ptr, info_ptr);

	rowpointers = Q_malloc (height * sizeof(*rowpointers));
	for (i=0 ; i<height ; i++)
		rowpointers[i] = pixels + i * width_sign * width * bpp;

	png_write_image (png_ptr, rowpointers);
	png_write_end (png_ptr, info_ptr);
	png_destroy_write_struct (&png_ptr, &info_ptr);
	ImageWork_free (rowpointers);
	fclose (fp);

	return true;
}


/*
=============
Image_WritePNGPLTE
=============
*/
qbool Image_WritePNGPLTE (char *absolute_filename, int compression, byte *pixels, int width, int height, byte *palette)
{
	int		rowbytes = width;

	int		i;
	FILE		*fp;
	png_structp	png_ptr;
	png_infop	info_ptr;
	png_byte	**rowpointers;

	if (!(fp = FS_fopen(absolute_filename, "wb", FS_CREATE_PATH)))
		return false;

	if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
	{
		fclose (fp);
		return false;
	}

	if (!(info_ptr = png_create_info_struct(png_ptr)))
	{
		png_destroy_write_struct (&png_ptr, (png_infopp)NULL);
		fclose (fp);
		return false;
	}

	if (setjmp(png_ptr->jmpbuf))
	{
		png_destroy_write_struct (&png_ptr, &info_ptr);
		fclose (fp);
		return false;
	}

	png_set_write_fn (png_ptr, fp, PNG_IO_user_write_data, PNG_IO_user_flush_data);
	png_set_compression_level (png_ptr, CLAMP (Z_NO_COMPRESSION, compression, Z_BEST_COMPRESSION));

	png_set_IHDR (png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_PLTE (png_ptr, info_ptr, (png_color *)palette, 256);
	png_write_info (png_ptr, info_ptr);

	rowpointers = ImageWork_malloc (height * sizeof(*rowpointers));
	for (i=0 ; i<height ; i++)
		rowpointers[i] = pixels + i * rowbytes;

	png_write_image (png_ptr, rowpointers);
	png_write_end (png_ptr, info_ptr);
	png_destroy_write_struct (&png_ptr, &info_ptr);
	ImageWork_free (rowpointers);
	fclose (fp);

	return true;
}

#endif // SUPPORTS_LIBPNG


#if SUPPORTS_LIBJPEG
/*
=========================================================

			JPEG

=========================================================
*/



/*
=============
Image_LoadJPEG
=============
*/
byte *Image_LoadJPEG_FromOpenFile (FILE *fin, int *my_image_width, int *my_image_height)
{
	int	i, row_stride;
	byte	*data, *scanline, *p;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	if (!fin)
		return NULL;

	cinfo.err = jpeg_std_error (&jerr);
	jpeg_create_decompress (&cinfo);
	jpeg_stdio_src (&cinfo, fin);
	jpeg_read_header (&cinfo, true);
	jpeg_start_decompress (&cinfo);

	if (cinfo.image_width > IMAGE_MAX_DIMENSIONS || cinfo.image_height > IMAGE_MAX_DIMENSIONS)
	{
		Con_Printf ("JPEG image exceeds maximum supported dimensions\n");
#pragma message ("Quality assurance: State the file name and the violating dimension and the max dimension")
		return NULL;
	}

//	if ((matchwidth && cinfo.image_width != matchwidth) || (matchheight && cinfo.image_height != matchheight))
//	{
//		jpeg_finish_decompress (&cinfo);
//		jpeg_destroy_decompress (&cinfo);
//		Con_Printf ("JPEG matchwidth or matchheight\n");
//		return NULL;
//	}

	data = ImageWork_malloc (cinfo.image_width * cinfo.image_height * 4, "JPEG Load");
	row_stride = cinfo.output_width * cinfo.output_components;
	scanline = ImageWork_malloc (row_stride, "JPEG scanline");

	p = data;
	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines (&cinfo, &scanline, 1);

		// convert the image to RGBA
		switch (cinfo.output_components)
		{
		// RGB images
		case 3:
			for (i=0 ; i<row_stride ; )
			{
				*p++ = scanline[i++];
				*p++ = scanline[i++];
				*p++ = scanline[i++];
				*p++ = 255;
			}
			break;

		// greyscale images (default to it, just in case)
		case 1:
		default:
			for (i=0 ; i<row_stride ; i++)
			{
				*p++ = scanline[i];
				*p++ = scanline[i];
				*p++ = scanline[i];
				*p++ = 255;
			}
			break;
		}
	}

	*my_image_width = cinfo.image_width;
	*my_image_height = cinfo.image_height;

	jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);
	ImageWork_free (scanline);
	//fclose (fin);	// Baker: Didn't open file, shouldn't be closing it

	return data;
}

/*
=============
Image_WriteJPEG
=============
*/
qbool Image_WriteJPEG (const char *absolute_filename, const int compression, byte *pixels, const int width, const int height)
{
	byte	*scanline;
	FILE	*fout;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	if (!(fout = FS_fopen_write(absolute_filename, "wb", FS_CREATE_PATH)))
		return false;

	cinfo.err = jpeg_std_error (&jerr);
	jpeg_create_compress (&cinfo);
	jpeg_stdio_dest (&cinfo, fout);

	cinfo.image_width = abs(width);
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults (&cinfo);
	jpeg_set_quality (&cinfo, CLAMP (0, compression, 100), true);
	jpeg_start_compress (&cinfo, true);

	while (cinfo.next_scanline < height)
	{
		scanline = &pixels[cinfo.next_scanline*width*3];
		jpeg_write_scanlines (&cinfo, &scanline, 1);
	}

	jpeg_finish_compress (&cinfo);
	jpeg_destroy_compress (&cinfo);
	fclose (fout);

	return true;
}

#endif

/*
=========================================================

			PCX

=========================================================
*/




/*
typedef struct
{
	char		manufacturer;
	char		version;
	char		encoding;
	char		bits_per_pixel;
	unsigned short	xmin,ymin,xmax,ymax;
	unsigned short	hres,vres;
	unsigned char	palette[48];
	char		reserved;
	char		color_planes;
	unsigned short	bytes_per_line;
	unsigned short	palette_type;
	char		filler[58];
	unsigned char	data;			// unbounded
} pcx_t;
*/
/*
==============
Image_WritePCX
==============
*/
/*
int Image_WritePCX (char *filename, byte *data, int width, int height, byte *palette)
{
	int	rowbytes = width;

	int	i, j, length;
	pcx_t	*pcx;
	byte	*pack;

	if (!(pcx = ImageWork_malloc(width * height * 2 + 1000)))
		return false;

	pcx->manufacturer = 0x0a;	// PCX id
	pcx->version = 5;		// 256 color
 	pcx->encoding = 1;		// uncompressed
	pcx->bits_per_pixel = 8;	// 256 color
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = LittleShort((short)(width-1));
	pcx->ymax = LittleShort((short)(height-1));
	pcx->hres = LittleShort((short)width);
	pcx->vres = LittleShort((short)height);
	memset (pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes = 1;		// chunky image
	pcx->bytes_per_line = LittleShort((short)width);
	pcx->palette_type = LittleShort(2);		// not a grey scale
	memset (pcx->filler, 0, sizeof(pcx->filler));

// pack the image
	pack = &pcx->data;

	for (i=0 ; i<height ; i++)
	{
		for (j=0 ; j<width ; j++)
		{
			if ((*data & 0xc0) != 0xc0)
				*pack++ = *data++;
			else
			{
				*pack++ = 0xc1;
				*pack++ = *data++;
			}
		}

		data += rowbytes - width;
	}

// write the palette
	*pack++ = 0x0c;	// palette ID byte
	for (i=0 ; i<768 ; i++)
		*pack++ = *palette++;

// write output file
	length = pack - (byte *)pcx;
	if (!QFS_WriteFile(filename, pcx, length))
	{
		ImageWork_free (pcx);
		return false;
	}

	ImageWork_free (pcx);
	return true;
}
*/

//=========================================================

void Image_Init (void)
{

	Cvar_Registration_Client_Image ();
}


//==============================================================================
//
//  PCX
//
//==============================================================================

typedef struct
{
    char			signature;
    char			version;
    char			encoding;
    char			bits_per_pixel;
    unsigned short	xmin,ymin,xmax,ymax;
    unsigned short	hdpi,vdpi;
    byte			colortable[48];
    char			reserved;
    char			color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    char			filler[58];
} pcxheader_t;




/*
============
Image_LoadPCX
============
*/

byte *Image_LoadPCX_FromOpenFile (FILE *fin, int *my_image_width, int *my_image_height)
{
	pcxheader_t	pcx;
	int			x, y, w, h, readbyte, runlength, start;
	byte		*p, *data;
	byte		palette[768];

	start = ftell (fin); //save start of file (since we might be inside a pak file, SEEK_SET might not be the start of the pcx)

	fread(&pcx, sizeof(pcx), 1, fin);
	pcx.xmin = (unsigned short)LittleShort (pcx.xmin);
	pcx.ymin = (unsigned short)LittleShort (pcx.ymin);
	pcx.xmax = (unsigned short)LittleShort (pcx.xmax);
	pcx.ymax = (unsigned short)LittleShort (pcx.ymax);
	pcx.bytes_per_line = (unsigned short)LittleShort (pcx.bytes_per_line);

	if (pcx.signature != 0x0A)
	{
//		Sys_Error ("Not a valid PCX file", loadfilename);
		Con_DevPrintf (DEV_IMAGE, "Not a valid PCX file\n");
		//fclose (fin);  // Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

	if (pcx.version != 5)
	{
//		Sys_Error ("'%s' is version %i, should be 5", loadfilename, pcx.version);
		Con_DevPrintf (DEV_IMAGE, "Not a version 5 PCX file\n");
		//fclose (fin);  // Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

	if (pcx.encoding != 1 || pcx.bits_per_pixel != 8 || pcx.color_planes != 1)
	{
//		Sys_Error ("'%s' has wrong encoding or bit depth", loadfilename);
		Con_DevPrintf (DEV_IMAGE, "PCX has wrong encoding or bit depth\n");
		//fclose (fin);  // Baker: Didn't open file, shouldn't be closing it
		return NULL;
	}

	w = pcx.xmax - pcx.xmin + 1;
	h = pcx.ymax - pcx.ymin + 1;

	data = ImageWork_malloc((w*h+1)*4, "PCX loader"); //+1 to allow reading padding byte on last line

	//load palette
	fseek (fin, start + qfs_lastload.filelen - 768, SEEK_SET);
	fread (palette, 1, 768, fin);

	//back to start of image data
	fseek (fin, start + sizeof(pcx), SEEK_SET);

	for (y=0; y<h; y++)
	{
		p = data + y * w * 4;

		for (x=0; x<(pcx.bytes_per_line); ) //read the extra padding byte if necessary
		{
			readbyte = fgetc(fin);

			if(readbyte >= 0xC0)
			{
				runlength = readbyte & 0x3F;
				readbyte = fgetc(fin);
			}
			else
				runlength = 1;

			while(runlength--)
			{
				p[0] = palette[readbyte*3];
				p[1] = palette[readbyte*3+1];
				p[2] = palette[readbyte*3+2];
				p[3] = 255;
				p += 4;
				x++;
			}
		}
	}

	fclose(fin);
	// Baker: Didn't open file, shouldn't be closing it

	*my_image_width = w;
	*my_image_height = h;
	return data;
}
