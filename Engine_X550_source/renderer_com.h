
#ifndef __RENDERER_COM_H__
#define __RENDERER_COM_H__

typedef struct
{
	char	RendererText[20];
	int		graphics_api;			 //1 = OpenGL, 2 = Direct3D, 0 = Uninitialized
	qbool	initialized;
} renderer_com_t;

#endif // __RENDERER_COM_H__