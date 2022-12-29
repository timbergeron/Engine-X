
#ifndef __WINQUAKE_VIDEO_MODES_H__
#define __WINQUAKE_VIDEO_MODES_H__

typedef struct {
	modestate_t	displaymode;
	int			modenum;
	int			width;
	int			height;
	int			bpp;
	int			refreshrate; //johnfitz
	char		modedesc[17];
//	int			dib;
//	int			fullscreen;
//	int		halfscreen;
} vmode_t;

#define MAX_MODE_LIST		600

extern vmode_t	video_modes[MAX_MODE_LIST];
extern vmode_t	badmode;						// Dummy return mode for an invalid mode
extern int	num_video_modes;

extern int vid_default;

void Rebuild_Mode_Description (int mode);

// Windows stuff
extern HWND		mainwindow; //, dibwindow;
extern HDC		maindc;
extern HICON	hIcon;


// Drawing Area
extern RECT		ClientDrawingRect;

// Mouse Region Stuff

extern int		MouseRegionCenter_x, MouseRegionCenter_y, window_x, window_y, window_width, window_height;


// Desktop Functions

void VID_UpdateDesktopProperties (void);
void VID_DescribeDesktop_f (void);

#endif // __WINQUAKE_VIDEO_MODES_H__