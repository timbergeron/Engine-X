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
// listbox_fixed.c -- a fixed size list box

#include "quakedef.h"

typedef	void (*myFunction_t) (void);


#define MAX_ITEMS	20


#define ALIGN_LEFT	0
#define ALIGN_RIGHT	1

#define BORDER_NONE 0

#define BACKSTYLE_TRANSPARENT	0
#define BACKSTYLE_OPAQUE		1

typedef struct
{
	const int			*LiveCondition;
	myFunction_t		 RunFunction;
	const char			*Text;
	int					 SpecialDraw;
} listitem_t;


typedef struct
{
	int			top, right, left, bottom;		// pixel x,y coords
	int			rows, columns;

	qbool		Alignment;						// 0 = left aligned, 1 = right aligned
	qbool		BackStyle;
	qbool		BorderStyle;
	vec_rgb_t	backcolor;						// rgb

	listitem_t	mylist[20];						// Up to 20 listitems
	int			listcount;						
} listbox_fixed_t;

listbox_fixed_t	displaybox;

//fps
//location coords
//location angles
//skill level
//cheats	(noclip, god, etc.)
//
//texture pointer


//char	display_fps[32];
//char	display_skill[20];

void ListBoxFixed_SetAlignment		(listbox_fixed_t *myListBox, const int myAlignment)		{	myListBox->Alignment	= myAlignment;		}
void ListBoxFixed_SetBorderStyle	(listbox_fixed_t *myListBox, const int myBorderStyle)	{	myListBox->BorderStyle	= myBorderStyle;	}
void ListBoxFixed_SetBackStyle		(listbox_fixed_t *myListBox, const int myBackStyle)		{	myListBox->BackStyle	= myBackStyle;		}


void ListBoxFixed_AddItem (listbox_fixed_t *myListBox, int *myLiveCondition, myFunction_t myRunFunction, const char *myText, const int mySpecialDraw)
{
	const int	newItem = myListBox->listcount;

	if (myListBox->listcount >= MAX_ITEMS)		Sys_Error ("Fixed ListBox Full");

	myListBox->mylist[newItem].LiveCondition	=	myLiveCondition;	//	This must be true for us to "run"
	myListBox->mylist[newItem].RunFunction		=	myRunFunction;		//	If we run, this is what we do
	myListBox->mylist[newItem].Text				=	myText;				//	This will be what we draw .... unless
	myListBox->mylist[newItem].SpecialDraw		=	mySpecialDraw;		//  This is non-zero

	myListBox->listcount ++;
}

extern int	volume_changed;

/*
===============
SCR_DrawVolume
===============
*/
char	display_volume [32];
void Volume_Changed_Think (void)	
{
	// This function will only be called if volume changed is true!
	extern float volume_time;

	{	// Phase one ... calculate the caption
		int		i;
		float	j;
		char	bar[11];
		
		for (i = 1, j = 0.1 ; i <= 10 ; i++, j += 0.1)	
			bar[i-1] = ((snd_volume.floater + 0.05) >= j) ? 139 : 11;

		bar[10] = 0;

		snprintf (display_volume, sizeof(display_volume), "volume %s", bar);
	}

	// Phase two ... determine whether or not to set volume_changed to false
	if (realtime > volume_time + 2.0)
		volume_changed = false;		// 2 seconds has elapsed
}

char	display_fps [32] = {0};
extern qbool movie_is_capturing;

void Frames_Per_Second_Think (void)
{	
	// This function will only be called if show_fps.integer is true
	static	double	lastframetime	= 0;
	static	int		frame_count		= 0;
	double			current_time	= Sys_DoubleTime ();
	double			elapsed_time	= current_time - lastframetime;

	frame_count ++;

	if (elapsed_time >= 1.0)	// If at least one second has elapsed, updated the frames per second
	{
		float		current_fps		= frame_count / elapsed_time;
		snprintf (display_fps, sizeof(display_fps), "%4.0f fps", current_fps + 0.5);	// Round up to next digit if fraction is at least .5

		if (movie_is_capturing || cls.demorecording)	// We will be adding a "dot"
		{
			StringLCat (display_fps, " ");
			if (movie_is_capturing)							StringLCat (display_fps, "\x89");	// Character 137
			if (cls.demorecording)							StringLCat (display_fps, "\x87");	// Character 135
		}

		// Reset the things we use to make these calculations

		frame_count		= 0;
		lastframetime	= current_time;
	}

}

#if SUPPORTS_TEXTURE_POINTER
char display_surface [32] = {0};
extern msurface_t	*texturepointer_surface;
extern char			*texturepointer_name;

void Surface_Pointer_Think (void)
{
	// Phase one ... calculate the caption
	
	display_surface[0] = 0;	// So if nothing is valid there is a zero length string

	do
	{
		if (!cl.worldmodel)				break;	// No worldmodel
		if (!texturepointer_surface)	break;	// No surface
		if (!texturepointer_name)		break;	// No texture name (however this could happen ... )

		{
			int surface_number = texturepointer_surface - cl.worldmodel->surfaces;
			snprintf (display_surface, sizeof(display_surface), "Texture: surf %i %s", surface_number, texturepointer_name);
		}
	} while (0);

}
#endif

char display_coords [32] = {0};
void Show_Coords_Think (void) 
{
	display_coords[0]= 0;
	if (!cl.worldmodel) return;

	snprintf (display_coords, sizeof(display_coords), "player coords: %i %i %i", 		
		(int)cl_entities[cl.player_point_of_view_entity].origin[0],
		(int)cl_entities[cl.player_point_of_view_entity].origin[1],
		(int)cl_entities[cl.player_point_of_view_entity].origin[2]);


}

char display_angles [32] = {0};
void Show_Angles_Think (void)
{
	display_angles[0]= 0;
	if (!cl.worldmodel) return;

	//player angles
	snprintf (display_angles, sizeof (display_angles), "pitch: %i yaw %i roll: %i",
		(int)cl.viewangles[PITCH],
		(int)cl.viewangles[YAW],
		(int)cl.viewangles[ROLL]);



}

char display_location [64] = {0};
void Show_Location_Think (void) 
{
	display_location[0]= 0;
	if (!cl.worldmodel) return;

	StringLCopy (display_location, LOC_GetLocation(cl_entities[cl.player_point_of_view_entity].origin));

}

char display_cheats [64] = {0};
void Show_Cheats_Think (void) 
{
	display_cheats[0]= 0;
	if (!cl.worldmodel) return;
	if (!sv.active)		return;		// We have no way of knowing

	do
	{
		int	player_flags = (int)sv_player->v.flags;

		if (player_flags & FL_GODMODE)					StringLCat (display_cheats, " [godmode]");	// Add godmode to list
		if (player_flags & FL_NOTARGET)					StringLCat (display_cheats, " [notarget]");	// Add notarget to list
		if (sv_player->v.movetype == MOVETYPE_NOCLIP)	StringLCat (display_cheats, " [noclip]");	// Add noclip to list .. in some mods this MIGHT be legitimately used, but I can't really think of one
		if (sv_freezenonclients.integer)				StringLCat (display_cheats, " [freeze]");	// Add freeze to list ... this is quasi-cheat
	} while (0);

}

void Draw_DemoProgressBar (const int pixel_right, const int pixel_top)
{
	void Draw_FillRGBA (float x, float y, float w, float h, float red, float green, float blue, float alpha);

	// Print demo stats
	const int		netprogress		= (cls._democurpos - cls._demostartpos);
	const float		pct_times100	= (float)( (float)netprogress/ (float)cls._demolength) *100;
	char			tempstring[30];
	//DrawFillRGB (vid.width - (5 /*"100% "*/ * 8 - 50 / *space for 50 pixels)

	snprintf (tempstring, sizeof(tempstring), "%3.0f%%", pct_times100);
	{
		const int		starttext_x = pixel_right - strlen(tempstring) * 8;
		const int		starttext_y = pixel_top;
		Draw_String		(starttext_x, starttext_y, tempstring); // JoeQuake has things all over the screen, so let's do top/left
		Draw_FillRGBA	(starttext_x - 50, starttext_y + 2, 50              , 4, 0, 0, 0, 1);
		Draw_FillRGBA	(starttext_x - 50, starttext_y + 2, (pct_times100/2), 4, 0, 1, 0, 1);
	}
}

qbool SB_IsShowScores (void);
void Draw_DisplayBox (void)
{
	int	i, x, y, strlen_Text;
	int row = 0;

	for (i=0; i < displaybox.listcount; i++)
	{
		// Check the conditional
		if (displaybox.mylist[i].LiveCondition)
			if (!*displaybox.mylist[i].LiveCondition)
				continue;	// There is a condition and this condition is not met

		if (displaybox.mylist[i].SpecialDraw == 1)
		{
			const qbool in_show_scores =  SB_IsShowScores();
			if (in_show_scores && cls.demofile)
			{
				Draw_DemoProgressBar (vid.width - 8, ((row++) + 4) * 8);
				continue;
			}
				
			if (!in_show_scores && cl.stats[STAT_HEALTH]>0)
				continue;		// We only draw the skill thing if scoreboard is up or dead ... and neither condition is met ...
		}

		// If there is a run function, run it.  It might generate the string or some such thing
		if (displaybox.mylist[i].RunFunction)
			displaybox.mylist[i].RunFunction ();

		if (!(strlen_Text = strlen(displaybox.mylist[i].Text)))
			continue;		// Don't do zero length strings
		
		x = vid.width - ((strlen_Text + 1) * 8);	
		y = ((row++) + 4) * 8;

		Draw_String (x, y, displaybox.mylist[i].Text);
	}

	// Finis
}


char display_skill[32] = {0};

void Show_Skill_Think (void)
{
	display_skill[0] = 0;
	if (!sv.active)							return;		// We don't know
	if (cl.gametype == GAME_DEATHMATCH)		return;		// Doesn't really matter if you aren't fighting the monsters

	snprintf (display_skill, sizeof(display_skill), "skill: %s", SkillTextForCurrentSkill (true));

}

#if 0
	if (cls.demoplayback && scr_showstuff_row >= 0)
	{
		void Draw_FillRGBA (float x, float y, float w, float h, float red, float green, float blue, float alpha);

		// Print demo stats
		int netprogress = (cls._democurpos - cls._demostartpos);
		float pct_times100 = (float)( (float)netprogress/ (float)cls._demolength) *100;
		int	starttext_x, starttext_y;
		//DrawFillRGB (vid.width - (5 /*"100% "*/ * 8 - 50 / *space for 50 pixels)

		snprintf (str, sizeof(str), "%3.0f%%", pct_times100);
		Draw_String ((starttext_x = vid.width - (strlen(str)+1) * 8), (starttext_y = scr_showstuff_row *8), str); // JoeQuake has things all over the screen, so let's do top/left
		Draw_FillRGBA (starttext_x - (50), starttext_y + 2, 50              , 4, 0, 0, 0, 1);
		Draw_FillRGBA (starttext_x - (50), starttext_y + 2, (pct_times100/2), 4, 0, 1, 0, 1);
		scr_showstuff_row ++;

	}
#endif

void DisplayBox_Init (void)
{
	ListBoxFixed_SetAlignment		(&displaybox, ALIGN_RIGHT										);			
	ListBoxFixed_SetBorderStyle		(&displaybox, BORDER_NONE										);
	ListBoxFixed_SetBackStyle		(&displaybox, BACKSTYLE_TRANSPARENT								);

	//								The Box    , Conditional cvar,  type	text source
	
	ListBoxFixed_AddItem			(	&displaybox,					// We are changing "displaybox"
										&volume_changed,				// The conditional is volume_changed, if volume changed is true we do the display process
										Volume_Changed_Think,			// This function is run prior to evaluating volume_changed, it may set it to false
										display_volume,					// This is the caption string
										0								// Do something irregular like draw a line
									);		

	ListBoxFixed_AddItem			(	&displaybox,					// We are changing "displaybox"
										&scr_show_fps.integer,				
										Frames_Per_Second_Think,		
										display_fps,					// This is the caption string
										0								// Do something irregular like draw a line
									);

	ListBoxFixed_AddItem			(	&displaybox,					// We are changing "displaybox"
										&scr_show_skill.integer,				
										Show_Skill_Think,		
										display_skill,					// This is the caption string
										1								// Do something irregular like draw a line
									);


	ListBoxFixed_AddItem			(	&displaybox,					// We are changing "displaybox"
										&tool_texturepointer.integer,				
										Surface_Pointer_Think,		
										display_surface,				// This is the caption string
										0								// Do something irregular like draw a line
									);


	ListBoxFixed_AddItem			(	&displaybox,					// We are changing "displaybox"
										&scr_show_coords.integer,				
										Show_Coords_Think,		
										display_coords,					// This is the caption string
										0								// Do something irregular like draw a line
									);

	ListBoxFixed_AddItem			(	&displaybox,					// We are changing "displaybox"
										&scr_show_angles.integer,				
										Show_Angles_Think,		
										display_angles,					// This is the caption string
										0								// Do something irregular like draw a line
									);

	ListBoxFixed_AddItem			(	&displaybox,					// We are changing "displaybox"
										&scr_show_location.integer,				
										Show_Location_Think,		
										display_location,				// This is the caption string
										0								// Do something irregular like draw a line
									);

	ListBoxFixed_AddItem			(	&displaybox,					// We are changing "displaybox"
										&scr_show_cheats.integer,				
										Show_Cheats_Think,		
										display_cheats,				// This is the caption string
										0								// Do something irregular like draw a line
									);


	
	// What remains ... recording dot
	// Skill to do demo progress bar
}