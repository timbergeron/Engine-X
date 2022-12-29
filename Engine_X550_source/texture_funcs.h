
#ifndef __TEXTURE_FUNCS_H__
#define __TEXTURE_FUNCS_H__

#define RGB__BYTES_PER_PIXEL_IS_3   3
#define RGBA_BYTES_PER_PIXEL_IS_4	4
#define QPAL_BYTES_PER_PIXEL_IS_1	1

qbool Check_RGBA_Pixels_For_AlphaChannel (const byte *data, const int imagesize_as_width_times_height);
void QPAL_To_RGBA (const byte *src, unsigned *dest, const int imagesize_as_width_times_height);
qbool Check_QPAL_Copy_To_RGBA_Return_True_If_AlphaChannel (const byte *src, unsigned *dest, const int imagesize_as_width_times_height);
void Check_QPAL_Copy_To_RGBA_Make_NonFullBright_Pixels_Transparent (const byte *src, unsigned *dest, const int imagesize_as_width_times_height);

int Find_Power_Of_Two_Size (const int this_size);

void Copy_RGBA_Pixels_Into_Larger_Buffer (const byte *src, byte *dest, const int src_width, const int src_height, const int dest_width, const int dest_height);
void Copy_Byte_Pixels_Into_Larger_Buffer (const byte *src, byte *dest, const int src_width, const int src_height, const int dest_width, const int dest_height);

void ScaleDimensions (const int width, const int height, int *scaled_width, int *scaled_height, const int mode);

qbool Texture_QPAL_HasFullbrights (const byte *qpal_pixels, const int size);
qbool Texture_QPAL_HasMaskColor255 (const byte *qpal_pixels, const int size);


void RGBA_Zero_Fill_AlphaChannel (byte *data, const int imagesize_as_width_times_height);

void RGBA_Fill_AlphaChannel_On_Non_Black (unsigned int *data, const int imagesize_as_width_times_height);

qbool QPAL_Colorize_Skin_To_QPAL (const byte *original_src, byte *dest, const int imagesize, const int myShirt, const int myPants);


byte *StringToRGB (char *s);

void ResampleTexture (const unsigned *indata, const int inwidth, const int inheight, unsigned *outdata, const int outwidth, const int outheight, const qbool quality);
void MipMap (byte *in, int *width, int *height);

void SpaceOutCharset (const byte *src, byte *dest, const int colwidth, const int imageheight, const int rowheight, const int bytes_per_pixel);

void FlipBuffer (byte *buffer, const int columns, const int rows, const int BytesPerPixel);


void Mod_FloodFillSkin (byte *skin, int skinwidth, int skinheight);

#endif // __TEXTURE_FUNCS_H__