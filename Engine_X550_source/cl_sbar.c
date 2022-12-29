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
// sbar.c -- status bar code
// Baker: Validated 6-27-2011.  Just scoreboard timer fix.

#include "quakedef.h"
#define stdxoffset ((vid.width - 320) >> 1)



#define SBAR_DEFAULT "0" // New sbar

//cvar_t	cl_hudstyle = {"cl_hudstyle", SBAR_DEFAULT, true};	// Baker: this name should be changed to vary from ProQuake --- messes with the defaults
//cvar_t  cl_scoreboard_clean = {"cl_scoreboard_clean", "0", true}; // Baker 3.99q: idea from FitzQuake's don't draw centerprint while paused

float scr_scoreboard_fillalpha = 0.7;

int		sb_updates;		// if >= vid.numpages, no update needed

#define STAT_MINUS	10		// num frame for '-' stats digit
mpic_t		*sb_nums[2][11];
mpic_t		*sb_colon;
static mpic_t		*sb_slash;
static mpic_t		*sb_ibar;
static mpic_t		*sb_sbar;
static mpic_t		*sb_scorebar;

static mpic_t		*sb_weapons[7][8];	// 0 is active, 1 is owned, 2-5 are flashes
static mpic_t		*sb_ammo[4];
static mpic_t		*sb_sigil[4];
static mpic_t		*sb_armor[3];
static mpic_t		*sb_items[32];

//mpic_t		*sb_faces[7][2];	// 0 is dead, 1-4 are alive (Incorrect data was: 0 is gibbed, 1 is dead, 2-6 are alive)
static mpic_t		*sb_faces[5][2];	// 0 is dead, 1-4 are alive (Incorrect data was: 0 is gibbed, 1 is dead, 2-6 are alive)
					// 0 is static, 1 is temporary animation
static mpic_t		*sb_face_invis;
static mpic_t		*sb_face_quad;
static mpic_t		*sb_face_invuln;
static mpic_t		*sb_face_invis_invuln;

static qbool	sb_showscores;

//int		engine.screen.statusbar_lines;		// scan lines to draw

static mpic_t		*rsb_invbar[2];
static mpic_t		*rsb_weapons[5];
static mpic_t		*rsb_items[2];
static mpic_t		*rsb_ammo[3];
static mpic_t		*rsb_teambord;		// PGM 01/19/97 - team color border

//MED 01/04/97 added two more weapons + 3 alternates for grenade launcher
static mpic_t		*hsb_weapons[7][5];   // 0 is active, 1 is owned, 2-5 are flashes
//MED 01/04/97 added array to simplify weapon parsing
static int		hipweapons[4] = {HIT_LASER_CANNON_BIT, HIT_MJOLNIR_BIT, 4, HIT_PROXIMITY_GUN_BIT};
//MED 01/04/97 added hipnotic items array
static mpic_t		*hsb_items[2];

void Sbar_MiniDeathmatchOverlay (void);
void Sbar_DeathmatchOverlay (void);
void Sbar_IntermissionNumber (int x, int y, int num, int digits, int color);

// by joe
static int	sbar_xofs;

/*
===============
Sbar_ShowScores

Tab key down
===============
*/
void Sbar_ShowScores_f (void)
{
	if (sb_showscores)
		return;
	sb_showscores = true;
	sb_updates = 0;
}

/*
===============
Sbar_DontShowScores

Tab key up
===============
*/
void Sbar_DontShowScores_f (void)
{
	sb_showscores = false;
	sb_updates = 0;
}

/*
===============
Sbar_Changed
===============
*/
void Sbar_Changed (void)
{
	sb_updates = 0;	// update next frame
}

//=============================================================================

// drawing routines are relative to the status bar location

/*
=============
Sbar_DrawPic
=============
*/
void Sbar_DrawPic (int x, int y, mpic_t *pic)
{
	Draw_Pic (x + sbar_xofs, y + (vid.height-SBAR_HEIGHT), pic);
}

/*
=============
Sbar_DrawSubPic
=============
JACK: Draws a portion of the picture in the status bar.
*/
void Sbar_DrawSubPic (int x, int y, mpic_t *pic, int srcx, int srcy, int width, int height)
{
	Draw_SubPic (x, y + (vid.height-SBAR_HEIGHT), pic, srcx, srcy, width, height);
}

/*
=============
Sbar_DrawTransPic
=============
*/
void Sbar_DrawTransPic (int x, int y, mpic_t *pic)
{
	Draw_TransPic (x + sbar_xofs, y + (vid.height-SBAR_HEIGHT), pic);
}

/*
================
Sbar_DrawCharacter

Draws one solid graphics character
================
*/
void Sbar_DrawCharacter (int x, int y, int num)
{
	Draw_Character (x + 4 + sbar_xofs, y + vid.height-SBAR_HEIGHT, num);
}

/*
================
Sbar_DrawString
================
*/
void Sbar_DrawString (int x, int y, char *str)
{
	Draw_String (x + sbar_xofs, y + (vid.height-SBAR_HEIGHT), str);
}

/*
=============
Sbar_itoa
=============
*/
int Sbar_itoa (int num, char *buf)
{
	char	*str;
	int	pow10, dig;

	str = buf;

	if (num < 0)
	{
		*str++ = '-';
		num = -num;
	}

	for (pow10 = 10 ; num >= pow10 ; pow10 *= 10)
		;

	do {
		pow10 /= 10;
		dig = num/pow10;
		*str++ = '0'+dig;
		num -= dig*pow10;
	} while (pow10 != 1);

	*str = 0;

	return str-buf;
}

/*
=============
Sbar_DrawNum
=============
*/
void Sbar_DrawNum (int x, int y, int num, int digits, int color)
{
	char	str[12], *ptr;
	int	l, frame;

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);
	if (l < digits)
		x += (digits-l)*24;

	while (*ptr)
	{
		frame = (*ptr == '-') ? STAT_MINUS : *ptr -'0';

		Sbar_DrawTransPic (x, y, sb_nums[color][frame]);
		x += 24;
		ptr++;
	}
}

//=============================================================================

static int		fragsort[MAX_SCOREBOARD];

static char	scoreboardtext[MAX_SCOREBOARD][20];
static int		scoreboardtop[MAX_SCOREBOARD];
static int		scoreboardbottom[MAX_SCOREBOARD];
static int		scoreboardcount[MAX_SCOREBOARD];
static int		scoreboardlines;

/*
===============
Sbar_SortFrags
===============
*/
void Sbar_SortFrags (void)
{
	int	i, j, k;

// sort by frags
	scoreboardlines = 0;
	for (i=0 ; i<cl.maxclients ; i++)
	{
		if (cl.scores[i].name[0])
		{
			fragsort[scoreboardlines] = i;
			scoreboardlines++;
		}
	}

	for (i=0 ; i<scoreboardlines ; i++)
		for (j=0 ; j<scoreboardlines-1-i ; j++)
			if (cl.scores[fragsort[j]].frags < cl.scores[fragsort[j+1]].frags)
			{
				k = fragsort[j];
				fragsort[j] = fragsort[j+1];
				fragsort[j+1] = k;
			}
}

/* JPG - added this for teamscores in status bar
==================
Sbar_SortTeamFrags
==================
*/
void Sbar_SortTeamFrags (void)
{
	int		i, j, k;

// sort by frags
	scoreboardlines = 0;
	for (i=0 ; i<14 ; i++)
	{
		if (cl.teamscores[i].colors)
		{
			fragsort[scoreboardlines] = i;
			scoreboardlines++;
		}
	}

	for (i=0 ; i<scoreboardlines ; i++)
		for (j=0 ; j<scoreboardlines-1-i ; j++)
			if (cl.teamscores[fragsort[j]].frags < cl.teamscores[fragsort[j+1]].frags)
			{
				k = fragsort[j];
				fragsort[j] = fragsort[j+1];
				fragsort[j+1] = k;
			}
}

int Sbar_ColorForMap (int m)
{
	return m < 128 ? m + 8 : m + 8;
}

/*
===============
Sbar_UpdateScoreboard
===============
*/
void Sbar_UpdateScoreboard (void)
{
	int		i, k, top, bottom;
	scoreboard_t	*s;

	Sbar_SortFrags ();

// draw the text
	memset (scoreboardtext, 0, sizeof(scoreboardtext));

	for (i=0 ; i<scoreboardlines; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		snprintf (&scoreboardtext[i][1], sizeof(&scoreboardtext[i][1]), "%3i %s", s->frags, s->name);

		top = s->colors & 0xf0;
		bottom = (s->colors & 15) << 4;
		scoreboardtop[i] = Sbar_ColorForMap (top);
		scoreboardbottom[i] = Sbar_ColorForMap (bottom);
	}
}


char *SkillTextForCurrentSkill (qbool bUseCvar)
{
//	For screenshots, use REAL setting.
//  For scoreboard,  use virtual setting (cvar)
	static char my_skill[30];
	//extern int current_skill;
//	int skill_hack;

	if (!sv.active)
	{	// Not a local game
		my_skill[0] = '\0';
		return my_skill;
	}

	switch (bUseCvar ? (int)pr_skill.floater : sv.current_skill_at_map_start)
	{
	case 0:  StringLCopy (my_skill, bUseCvar ? "åáóù"      : "easy"		);  break;
	case 1:  StringLCopy (my_skill, bUseCvar ? "îïòíáì"    : "normal"	);  break;
	case 2:  StringLCopy (my_skill, bUseCvar ? "èáòä"      : "hard"		);  break;
	case 3:  StringLCopy (my_skill, bUseCvar ? "îéçèôíáòå" : "nightmare");  break;
	default: StringLCopy (my_skill, "(special)"							);  break;
	}

	if (bUseCvar && (int)pr_skill.floater != sv.current_skill_at_map_start)
		snprintf (my_skill, sizeof(my_skill), "%s [*]", my_skill); // Add asterisk to indicate it hasn't taken effect

	return my_skill;
}

/*
===============
Sbar_SoloScoreboard
===============
*/
//extern int scr_showstuff_row;
void Sbar_SoloScoreboard (void)
{
	char	str[80];
	int	len, minutes, seconds, tens, units;


	snprintf (str, sizeof(str), "Monsters:%3i /%3i", cl.stats[STAT_MONSTERS], cl.stats[STAT_TOTALMONSTERS]);
	Sbar_DrawString (8, 4, str);

	snprintf (str, sizeof(str), "Secrets :%3i /%3i", cl.stats[STAT_SECRETS], cl.stats[STAT_TOTALSECRETS]);
	Sbar_DrawString (8, 12, str);

// time
	minutes = cl.ctime / 60;
	seconds = cl.ctime - 60*minutes;
	tens = seconds / 10;
	units = seconds - 10*tens;
	snprintf (str, sizeof(str), "Time :%3i:%i%i", minutes, tens, units);
	Sbar_DrawString (184, 4, str);

// draw level name
	len = strlen (cl.levelname);
	Sbar_DrawString (232 - len*4, 12, cl.levelname);
}

/*
===============
Sbar_DrawScoreboard
===============
*/
void Sbar_DrawScoreboard (void)
{
	Sbar_SoloScoreboard ();
	if (cl.gametype == GAME_DEATHMATCH)
		Sbar_DeathmatchOverlay ();
}

//=============================================================================

/*
===============
Sbar_DrawInventory
===============
*/
void Sbar_DrawInventory (void)
{
	int		i, flashon, ystart;
	char		num[6];
	float		time;
	qbool	headsup;

	headsup = !(!scr_hud_style.integer /* || scene_viewsize.floater < 100*/);

	if (!headsup)
	{
		if (rogue)
		{
			if (cl.stats[STAT_ACTIVEWEAPON] >= RIT_LAVA_NAILGUN)
				Sbar_DrawPic (0, -24, rsb_invbar[0]);
			else
				Sbar_DrawPic (0, -24, rsb_invbar[1]);
		}
		else
		{
			// Ok ... let the weird begin ...
			// This replacement element has alpha and is foobared for cl_hudstyle 0 (original HUD)
			// with gl_clear 0.

			// Baker:  I think I fixed this?  No. Actually it cannot be fixed.  Reason is that
			//         the texture needs alpha test for rendering the Quakeworld style HUD
			//         Since it can't have alpha and also not have alpha (unless we wanted undue pain to
			//         make a 2nd texture for no reason.  This has to be the case.
			MeglDisable (GL_ALPHA_TEST);
			Sbar_DrawPic (0, -24, sb_ibar);
			MeglEnable (GL_ALPHA_TEST);
		}
	}

// weapons
	ystart = hipnotic ? -100 : -68;
	for (i=0 ; i<7 ; i++)
	{
		if (cl.items & (IT_SHOTGUN << i))
		{
			time = cl.item_gettime[i];
			flashon = (int)((cl.time - time)*10);
			if (flashon < 0)
				flashon = 0;
			if (flashon >= 10)
				flashon = (cl.stats[STAT_ACTIVEWEAPON] == (IT_SHOTGUN << i)) ? 1 : 0;
			else
				flashon = (flashon % 5) + 2;

			if (headsup)
			{
				if (i || vid.height>200)
					Sbar_DrawSubPic ((vid.width-24), ystart - (7-i)*16, sb_weapons[flashon][i], 0, 0, 24, 16);
			}
			else
			{
				Sbar_DrawPic (i*24, -16, sb_weapons[flashon][i]);
			}

			if (flashon > 1)
				sb_updates = 0;		// force update to remove flash
		}
	}

// MED 01/04/97
// hipnotic weapons
	if (hipnotic)
	{
		int	grenadeflashing = 0;

		for (i=0 ; i<4 ; i++)
		{
			if (cl.items & (1 << hipweapons[i]))
			{
				time = cl.item_gettime[hipweapons[i]];
				flashon = (int)((cl.time - time) * 10);
				if (flashon < 0)
					flashon = 0;
				if (flashon >= 10)
					flashon = (cl.stats[STAT_ACTIVEWEAPON] == (1 << hipweapons[i])) ? 1 : 0;
				else
					flashon = (flashon % 5) + 2;

			// check grenade launcher
				if (i == 2)
				{
					if ((cl.items & HIT_PROXIMITY_GUN) && flashon)
					{
						grenadeflashing = 1;
						if (headsup)
						{
							if (i || vid.height>200)
								Sbar_DrawSubPic ((vid.width-24), ystart - 48, hsb_weapons[flashon][2], 0, 0, 24, 16);
						}
						else
						{
							Sbar_DrawPic (96, -16, hsb_weapons[flashon][2]);
						}
					}
				}
				else if (i == 3)
				{
					if (cl.items & (IT_SHOTGUN << 4))
					{
						if (!grenadeflashing)
						{
							if (flashon)
							{
								if (headsup)
								{
									if (i || vid.height>200)
										Sbar_DrawSubPic ((vid.width-24), ystart - 48, hsb_weapons[flashon][3], 0, 0, 24, 16);
								}
								else
								{
									Sbar_DrawPic (96, -16, hsb_weapons[flashon][3]);
								}
							}
							else
							{
								if (headsup)
								{
									if (i || vid.height>200)
										Sbar_DrawSubPic ((vid.width-24), ystart - 48, hsb_weapons[0][3], 0, 0, 24, 16);
								}
								else
								{
									Sbar_DrawPic (96, -16, hsb_weapons[0][3]);
								}
							}
						}
					}
					else
					{
						if (headsup)
						{
							if (i || vid.height>200)
								Sbar_DrawSubPic ((vid.width-24), ystart - 48, hsb_weapons[flashon][4], 0, 0, 24, 16);
						}
						else
						{
							Sbar_DrawPic (96, -16, hsb_weapons[flashon][4]);
						}
					}
				}
				else
				{
					if (headsup)
					{
						if (i || vid.height>200)
							Sbar_DrawSubPic ((vid.width-24), ystart + i*16, hsb_weapons[flashon][i], 0, 0, 24, 16);
					}
					else
					{
						Sbar_DrawPic (176 + (i*24), -16, hsb_weapons[flashon][i]);
					}
				}
				if (flashon > 1)
					sb_updates = 0;      // force update to remove flash
			}
		}
	}

// rogue weapons
	if (rogue)
	{	// check for powered up weapon
		if (cl.stats[STAT_ACTIVEWEAPON] >= RIT_LAVA_NAILGUN)
		{
			for (i=0 ; i<5 ; i++)
			{
				if (cl.stats[STAT_ACTIVEWEAPON] == (RIT_LAVA_NAILGUN << i))
					Sbar_DrawPic ((i+2)*24, -16, rsb_weapons[i]);
			}
		}
	}

// ammo counts
	for (i=0 ; i<4 ; i++)
	{
		snprintf (num, sizeof(num), "%3i", cl.stats[STAT_SHELLS+i]);
		if (headsup)
		{
			Sbar_DrawSubPic ((vid.width-42), -24 - (4-i)*11, sb_ibar, 3+(i*48), 0, 42, 11);
			if (num[0] != ' ')
				Draw_Character ((vid.width - 35), vid.height-SBAR_HEIGHT-24 - (4-i)*11, 18 + num[0] - '0');
			if (num[1] != ' ')
				Draw_Character ((vid.width - 27), vid.height-SBAR_HEIGHT-24 - (4-i)*11, 18 + num[1] - '0');
			if (num[2] != ' ')
				Draw_Character ((vid.width - 19), vid.height-SBAR_HEIGHT-24 - (4-i)*11, 18 + num[2] - '0');
		}
		else
		{
			if (num[0] != ' ')
				Sbar_DrawCharacter ((6*i+1)*8 - 2, -24, 18 + num[0] - '0');
			if (num[1] != ' ')
				Sbar_DrawCharacter ((6*i+2)*8 - 2, -24, 18 + num[1] - '0');
			if (num[2] != ' ')
				Sbar_DrawCharacter ((6*i+3)*8 - 2, -24, 18 + num[2] - '0');
		}
	}

	flashon = 0;

// items
	for (i=0 ; i<6 ; i++)
	{
		if (cl.items & (1<<(17+i)))
		{
			time = cl.item_gettime[17+i];
			if (time && time > cl.time - 2 && flashon)
			{	// flash frame
				sb_updates = 0;
			}
			else
			{	//MED 01/04/97 changed keys
				if (!hipnotic || (i>1))
				{
					Sbar_DrawPic (192 + i*16, -16, sb_items[i]);
				}
			}
			if (time && time > cl.time - 2)
			sb_updates = 0;
		}
	}

//MED 01/04/97 added hipnotic items
// hipnotic items
	if (hipnotic)
	{
		for (i=0 ; i<2 ; i++)
		{
			if (cl.items & (1<<(24+i)))
			{
				time = cl.item_gettime[24+i];
				if (time && time > cl.time - 2 && flashon)	// flash frame
					sb_updates = 0;
				else
					Sbar_DrawPic (288 + i*16, -16, hsb_items[i]);

				if (time && time > cl.time - 2)
					sb_updates = 0;
			}
		}
	}

// rogue items
	if (rogue)
	{
	// new rogue items
		for (i=0 ; i<2 ; i++)
		{
			if (cl.items & (1<<(29+i)))
			{
				time = cl.item_gettime[29+i];
				if (time && time > cl.time - 2 && flashon)	// flash frame
					sb_updates = 0;
				else
					Sbar_DrawPic (288 + i*16, -16, rsb_items[i]);
				if (time && time > cl.time - 2)
					sb_updates = 0;
			}
		}
	}
	else
	{
	// sigils
		for (i=0 ; i<4 ; i++)
		{
			if (cl.items & (1<<(28+i)))
			{
				time = cl.item_gettime[28+i];
				if (time && time > cl.time - 2 && flashon)	// flash frame
					sb_updates = 0;
				else
					Sbar_DrawPic (320-32 + i*8, -16, sb_sigil[i]);
				if (time && time > cl.time - 2)
					sb_updates = 0;
			}
		}
	}
}

//=============================================================================

/*
===============
Sbar_DrawFrags
===============
*/
void Sbar_DrawFrags (void)
{
	int				i, k, numscores;
	int				top, bottom;
	int				x, y, f;
	int				xofs;
	char			num[12];
	scoreboard_t	*s;
	int				teamscores, colors, ent, minutes, seconds, mask; // JPG - added these
	int				match_time; // JPG - added this

	// JPG - check to see if we should sort teamscores instead
	teamscores = scr_hud_teamscores.integer && cl.teamgame;
	if (teamscores)
		Sbar_SortTeamFrags();
	else
		Sbar_SortFrags ();

// draw the text
	numscores = scoreboardlines <= 4 ? scoreboardlines : 4;

	x = 23;
	if (cl.gametype == GAME_DEATHMATCH)
		xofs = 0;
	else
		xofs = (vid.width - 320)>>1;
	y = vid.height - SBAR_HEIGHT - 23;

	// JPG - check to see if we need to draw the timer
	if (scr_hud_timer.integer && (cl.minutes != 255))
	{
		if (numscores > 2)
			numscores = 2;
		mask = 0;
		if (cl.minutes == 254)
		{
			strcpy(num, "    SD");
			mask = 128;
		}
		else if (cl.minutes || cl.seconds)
		{
			if (cl.seconds >= 128)
				snprintf (num, sizeof(num), " -0:%02d", cl.seconds - 128);
			else
			{
				if (cl.match_pause_time)
					match_time = ceil(60.0 * cl.minutes + cl.seconds - (cl.match_pause_time - cl.last_match_time));
				else
					match_time = ceil(60.0 * cl.minutes + cl.seconds - (cl.time - cl.last_match_time));
				minutes = match_time / 60;
				seconds = match_time - 60 * minutes;
				snprintf (num, sizeof(num),  "%3d:%02d", minutes, seconds);
				if (!minutes)
					mask = 128;
			}
		}
		else
		{
			minutes = cl.time / 60;
			seconds = cl.time - 60 * minutes;
			minutes = minutes & 511;
			snprintf (num,  sizeof(num), "%3d:%02d", minutes, seconds);
		}

		for (i = 0 ; i < 6 ; i++)
			Sbar_DrawCharacter ((x+9+i)*8, -24, num[i] + mask);
	}

	for (i=0 ; i<numscores ; i++)
	{
		k = fragsort[i];

		// JPG - check for teamscores
		if (teamscores)
		{
			colors = cl.teamscores[k].colors;
			f = cl.teamscores[k].frags;
		}
		else
		{
			s = &cl.scores[k];
			if (!s->name[0])
				continue;
			colors = s->colors;
			f = s->frags;
		}

		// draw background
		top = colors & 0xf0;
		bottom = (colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill (xofs + x*8 + 10, y, 28, 4, top);
		Draw_Fill (xofs + x*8 + 10, y+4, 28, 3, bottom);

		// draw number
		snprintf (num,  sizeof(num), "%3i",f);

		Sbar_DrawCharacter ((x+1)*8 , -24, num[0]);
		Sbar_DrawCharacter ((x+2)*8 , -24, num[1]);
		Sbar_DrawCharacter ((x+3)*8 , -24, num[2]);

		// JPG - check for self's team
		ent = cl.player_point_of_view_entity - 1;

/*
		// Baker 3.99n: show color
//		if (cl_colorshow.integer && cl.maxclients>1)
//		{
//			Draw_AlphaFill	(12, 12, 16, 16, Sbar_ColorForMap((cl.scores[ent].colors & 15) & 15<<4), 0.8);	// Baker 3.99n: display pants color in top/left
*/

		if ((teamscores && ((colors & 15) == (cl.scores[ent].colors & 15))) || (!teamscores && (k == ent)))
		{
			Sbar_DrawCharacter (x*8+2, -24, 16);
			Sbar_DrawCharacter ((x+4)*8-4, -24, 17);
		}
		x += 4;
	}
}

//=============================================================================

/*
===============
Sbar_DrawFace
===============
*/
void Sbar_DrawFace (void)
{
	int	f, anim;

// PGM 01/19/97 - team color drawing
// PGM 03/02/97 - fixed so color swatch only appears in CTF modes
	if (rogue && (cl.maxclients != 1) && (pr_teamplay.floater > 3) && (pr_teamplay.floater < 7))
	{
		int		top, bottom, xofs;
		char		num[12];
		scoreboard_t	*s;

		s = &cl.scores[cl.player_point_of_view_entity - 1];
		// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15) << 4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		xofs = (cl.gametype == GAME_DEATHMATCH) ? 113 : (stdxoffset) + 113;

		Sbar_DrawPic (112, 0, rsb_teambord);
		Draw_Fill (xofs, vid.height-SBAR_HEIGHT+3, 22, 9, top);
		Draw_Fill (xofs, vid.height-SBAR_HEIGHT+12, 22, 9, bottom);

		// draw number
		f = s->frags;
		snprintf (num, sizeof(num), "%3i", f);

		if (top == 8)
		{
			if (num[0] != ' ')
				Sbar_DrawCharacter(109, 3, 18 + num[0] - '0');
			if (num[1] != ' ')
				Sbar_DrawCharacter(116, 3, 18 + num[1] - '0');
			if (num[2] != ' ')
				Sbar_DrawCharacter(123, 3, 18 + num[2] - '0');
		}
		else
		{
			Sbar_DrawCharacter ( 109, 3, num[0]);
			Sbar_DrawCharacter ( 116, 3, num[1]);
			Sbar_DrawCharacter ( 123, 3, num[2]);
		}

		return;
	}
// PGM 01/19/97 - team color drawing

	if ((cl.items & (IT_INVISIBILITY | IT_INVULNERABILITY)) == (IT_INVISIBILITY | IT_INVULNERABILITY))
	{
		Sbar_DrawPic (112, 0, sb_face_invis_invuln);
		return;
	}
	if (cl.items & IT_QUAD)
	{
		Sbar_DrawPic (112, 0, sb_face_quad);
		return;
	}
	if (cl.items & IT_INVISIBILITY)
	{
		Sbar_DrawPic (112, 0, sb_face_invis);
		return;
	}
	if (cl.items & IT_INVULNERABILITY)
	{
		Sbar_DrawPic (112, 0, sb_face_invuln);
		return;
	}

	f = cl.stats[STAT_HEALTH] / 20;
	f = CLAMP (0, f, 4);

	if (cl.time <= cl.faceanimtime)
	{
		anim = 1;
		sb_updates = 0;		// make sure the anim gets drawn over
	}
	else
	{
		anim = 0;
	}

	Sbar_DrawPic (112, 0, sb_faces[f][anim]);
}


/*
=============
Sbar_DrawNormal
=============
*/
void Sbar_DrawNormal (void)
	{
		if (!scr_hud_style.integer /* || scene_viewsize.floater < 100 */)
			Sbar_DrawPic (0, 0, sb_sbar);

	// keys (hipnotic only)
		//MED 01/04/97 moved keys here so they would not be overwritten
		if (hipnotic)
		{
			if (cl.items & IT_KEY1)
				Sbar_DrawPic (209, 3, sb_items[0]);
			if (cl.items & IT_KEY2)
				Sbar_DrawPic (209, 12, sb_items[1]);
		}

	// armor
		if (cl.items & IT_INVULNERABILITY)
		{
			Sbar_DrawNum (24, 0, 666, 3, 1);
			Sbar_DrawPic (0, 0, draw_disc);
		}
		else
		{
			Sbar_DrawNum (24, 0, cl.stats[STAT_ARMOR], 3, cl.stats[STAT_ARMOR] <= 25);

			if (rogue)
			{
				if (cl.items & RIT_ARMOR3)
					Sbar_DrawPic (0, 0, sb_armor[2]);
				else if (cl.items & RIT_ARMOR2)
					Sbar_DrawPic (0, 0, sb_armor[1]);
				else if (cl.items & RIT_ARMOR1)
					Sbar_DrawPic (0, 0, sb_armor[0]);
			}
			else
			{
				if (cl.items & IT_ARMOR3)
					Sbar_DrawPic (0, 0, sb_armor[2]);
				else if (cl.items & IT_ARMOR2)
					Sbar_DrawPic (0, 0, sb_armor[1]);
				else if (cl.items & IT_ARMOR1)
					Sbar_DrawPic (0, 0, sb_armor[0]);
			}
		}

	// face
		Sbar_DrawFace ();

	// health
		Sbar_DrawNum (136, 0, cl.stats[STAT_HEALTH], 3, cl.stats[STAT_HEALTH] <= 25);

	// ammo icon
		if (rogue)
		{
			if (cl.items & RIT_SHELLS)
				Sbar_DrawPic (224, 0, sb_ammo[0]);
			else if (cl.items & RIT_NAILS)
				Sbar_DrawPic (224, 0, sb_ammo[1]);
			else if (cl.items & RIT_ROCKETS)
				Sbar_DrawPic (224, 0, sb_ammo[2]);
			else if (cl.items & RIT_CELLS)
				Sbar_DrawPic (224, 0, sb_ammo[3]);
			else if (cl.items & RIT_LAVA_NAILS)
				Sbar_DrawPic (224, 0, rsb_ammo[0]);
			else if (cl.items & RIT_PLASMA_AMMO)
				Sbar_DrawPic (224, 0, rsb_ammo[1]);
			else if (cl.items & RIT_MULTI_ROCKETS)
				Sbar_DrawPic (224, 0, rsb_ammo[2]);
		}
		else
		{
			if (cl.items & IT_SHELLS)
				Sbar_DrawPic (224, 0, sb_ammo[0]);
			else if (cl.items & IT_NAILS)
				Sbar_DrawPic (224, 0, sb_ammo[1]);
			else if (cl.items & IT_ROCKETS)
				Sbar_DrawPic (224, 0, sb_ammo[2]);
			else if (cl.items & IT_CELLS)
				Sbar_DrawPic (224, 0, sb_ammo[3]);
		}

		Sbar_DrawNum (248, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 10);
	}




/*
===============
Sbar_Draw
===============
*/
void Sbar_Draw (void)
{
	qbool	headsup;

	headsup = !(!scr_hud_style.integer /*|| scene_viewsize.floater < 100*/);
	if (sb_updates >= vid.numpages && !headsup)
		return;

	if (scr_con_current == vid.height)
		return;		// console is full screen

//	scr_copyeverything = 1;

	sb_updates++;

	sbar_xofs = (scr_hud_center.integer && cl.gametype != GAME_DEATHMATCH ) ? stdxoffset : 0;

	if (scr_hud_style.integer == 2)
	{
		void Sbar_DrawStyle2 (void);
		Sbar_DrawStyle2 ();
		return;
	}

// top line
	if (sb_lines > 24)
	{
		Sbar_DrawInventory ();
//		if (!headsup || vid.width < 512)
		if (cl.gametype == GAME_DEATHMATCH) // Baker: this needs to be show in deathmatch ALWAYS!
			Sbar_DrawFrags ();
	}

// top line
	if (sb_lines > 0)
	{
		if (sb_showscores || cl.stats[STAT_HEALTH] <= 0)
		{
			Sbar_DrawPic (0, 0, sb_scorebar);
			Sbar_DrawScoreboard ();
		}
		else
		{
			Sbar_DrawNormal ();
		}
	}

#ifdef GLQUAKE
	if (sb_showscores || cl.stats[STAT_HEALTH] <= 0)
		sb_updates = 0;

	// clear unused areas in GL
	if (vid.width > 320 && !headsup)
	{	// left
		if (scr_hud_center.integer && cl.gametype != GAME_DEATHMATCH)
			Draw_TileClear (0, vid.height - sb_lines, sbar_xofs, sb_lines);

		// right
		Draw_TileClear (320 + sbar_xofs, vid.height - sb_lines, vid.width - (320 + sbar_xofs), sb_lines);
	}
#endif

	if (sb_lines > 0 && cl.gametype == GAME_DEATHMATCH /* && !scr_hud_center.integer*/)
	{
		Sbar_MiniDeathmatchOverlay ();
	}
}

qbool SB_IsShowScores (void)	{ return sb_showscores; }

//=============================================================================

/*
==================
Sbar_IntermissionNumber
==================
*/
void Sbar_IntermissionNumber (int x, int y, int num, int digits, int color)
{
	char	str[12], *ptr;
	int	l, frame;

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);
	if (l < digits)
		x += (digits-l)*24;

	while (*ptr)
	{
		frame = (*ptr == '-') ? STAT_MINUS : *ptr -'0';

		Draw_TransPic (x, y, sb_nums[color][frame]);
		x += 24;
		ptr++;
	}
}

/*
==================
Sbar_DeathmatchOverlay
==================
*/
void Sbar_DeathmatchOverlay (void)
{
	mpic_t			*pic;
	int				i, k, l, top, bottom, x, y, f;
	int				xofs = stdxoffset;
	int				yofs = 0;
	char			num[12];
	scoreboard_t	*s;
	int				j, ping; // JPG - added these

	// JPG 1.05 (rewrote this) - check to see if we need to update ping times
	if ((cl.last_ping_time < cl.time - 5) && scr_hud_scoreboard_pings.integer)
	{
		MSG_WriteByte (&cls.message, clc_stringcmd);
		SZ_Print (&cls.message, "ping\n");
		cl.last_ping_time = cl.time;
	}

	// JPG 1.05 - check to see if we should update IP status
	if (iplog_size && (cl.last_status_time < cl.time - 5))
	{
		MSG_WriteByte (&cls.message, clc_stringcmd);
		SZ_Print (&cls.message, "status\n");
		cl.last_status_time = cl.time;
	}

//	scr_copyeverything = 1;
//	scr_fullupdate = 0;

	pic = Draw_CachePic ("gfx/ranking.lmp");
	Draw_Pic  (xofs + 160 - pic->width/2, yofs, pic);

// scores
	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines;

	x = 80 + stdxoffset;
	y = 40;

	ping = 0;  // JPG - this will tell us if some client's ping is showing

#ifdef GLQUAKE
	Draw_AlphaFill	(x - 64,	y - 11, 328 , 10, 16, scr_scoreboard_fillalpha);	//inside
	Draw_Fill		(x - 64,	y - 12, 329 , 1	, 0);		//Border - Top
	Draw_Fill		(x - 64,	y - 12, 1	, 11, 0);		//Border - Left
	Draw_Fill		(x + 264,	y - 12, 1	, 11, 0);		//Border - Right
	Draw_Fill		(x - 64,	y - 1 , 329 , 1	, 0);		//Border - Bottom
	Draw_Fill		(x - 64,	y - 1,	329 , 1	, 0);		//Border - Top

	Draw_String		(x - 64,	y - 10, "  ping  frags   name");
#endif

	for (i=0 ; i<l ; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;
#ifdef GLQUAKE
		if (k == cl.player_point_of_view_entity - 1)
		{
			Draw_AlphaFill (x - 63, y, 328 , 10, 0 /*20*/, scr_scoreboard_fillalpha);
		}
		else
		{
			Draw_AlphaFill (x - 63, y, 328 , 10, 18, scr_scoreboard_fillalpha);
		}

		Draw_Fill (x - 64, y, 1, 10, 0);	//Border - Left
		Draw_Fill (x + 264, y, 1, 10, 0);	//Border - Right
#endif
	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15) << 4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill ( x, y, 40, 4, top);
		Draw_Fill ( x, y+4, 40, 4, bottom);

	// JPG - draw ping
		if (s->ping && scr_hud_scoreboard_pings.integer)
		{
			ping = 1;
			snprintf (num, sizeof(num), "%4i", s->ping);

			for (j=0 ; j<4 ; j++)
				Draw_Character(x-56+j*8, y, num[j]);
		}

	// draw number
		f = s->frags;
		snprintf (num, sizeof(num), "%3i", f);

		Draw_Character ( x+8 , y, num[0]);
		Draw_Character ( x+16 , y, num[1]);
		Draw_Character ( x+24 , y, num[2]);

		if (k == cl.player_point_of_view_entity - 1)
		{
#if 0
			Draw_Character (x + 40, y-1, 16);
			Draw_Character (x + 72, y-1, 17);

			//Draw_Character ( x - 8, y, 12);
			//Draw_AlphaFill (x + 40, y, 160 , 9, 18, 0.5);	//inside
			Draw_Fill (x - 1, y - 1	, 202 , 1, 0);	//Border - Top
			Draw_Fill (x - 1, y, 1, 9, 0);	//Border - Left
			Draw_Fill (x + 200, y, 1, 9, 0);	//Border - Right
			Draw_Fill (x - 1, y + 9, 202 , 1, 0);	//Border - Bottom
#endif

			Draw_Character(x, y, 16);	// JPG 3.00 from katua - draw [ ] around our score in the
			Draw_Character(x+32, y, 17);	// scoreboard overlay

		}
#ifdef GLQUAKE
		Draw_Fill		(x - 64,	y - 1 , 329 , 1	, 0);		//Border - Bottom
#endif

#if 0
{
	int				total;
	int				n, minutes, tens, units;

	// draw time
		total = cl.completed_time - s->entertime;
		minutes = (int)total/60;
		n = total - minutes*60;
		tens = n/10;
		units = n%10;

		snprintf (num, sizeof(num), "%3i:%i%i", minutes, tens, units);

		Draw_String ( x+48 , y, num);
}
#endif

	// draw name
		Draw_String (x+64, y, s->name);

		y += 10;
	}

#ifdef GLQUAKE
	Draw_Fill (x - 64, y, 329 , 1, 0);	//Border - Bottom
#endif

/*// JPG - draw "ping"
	if (ping)
	{
		strcpy(num, "ping");
		for (i = 0 ; i < 4 ; i++)
			Draw_Character(x-56+i*8, 30, num[i]+128);
	}
*/
}

/*
==================
Sbar_MiniDeathmatchOverlay
==================
*/
void Sbar_MiniDeathmatchOverlay (void)
{
	int				i, k, l, top, bottom, x, y, f, numlines;
	char		num[12];
	scoreboard_t	*s;

	if (vid.width < 512 || !sb_lines)
		return;

//	scr_copyeverything = 1;
//	scr_fullupdate = 0;

// scores
	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines;
	y = vid.height - sb_lines;
	numlines = sb_lines/8;
	if (numlines < 3)
		return;

	// find us
	for (i=0 ; i<scoreboardlines ; i++)
		if (fragsort[i] == cl.player_point_of_view_entity - 1)
			break;

    i= (i == scoreboardlines) ? 0 : i - numlines/2;
	i = CLAMP (0, i, scoreboardlines - numlines);

	x = 324;
	for ( ; i < scoreboardlines && y < vid.height - 8 ; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

	// draw colors
		top = s->colors & 0xf0;
		bottom = (s->colors & 15) << 4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

#ifdef GLQUAKE
		if (k == cl.player_point_of_view_entity - 1)
		{
			// Nifty Qrack code!!!!  Thanks Rook!
			Draw_AlphaFill (x-1, y, 168+2, 8+1, 0 /*18*/, 1); //scr_scoreboard_fillalpha);	//inside
		}
#endif

		Draw_Fill (x, y+1, 40, 3, top);
		Draw_Fill (x, y+4, 40, 4, bottom);

	// draw number
		f = s->frags;
		snprintf (num, sizeof(num), "%3i",f);

		Draw_Character (x+8 , y, num[0]);
		Draw_Character (x+16 , y, num[1]);
		Draw_Character (x+24 , y, num[2]);

		// brackets
		if (k == cl.player_point_of_view_entity - 1)
		{
			// Nifty Qrack code!!!!  Thanks Rook!
//			Draw_Fill (x - 1, y - 1	, 168+3 , 1, 0);	//Border - Top
//			Draw_Fill (x - 1, y, 1, 9, 0);	//Border - Left
//			Draw_Fill (x - 1 + 168+2, y, 1, 9, 0);	//Border - Right
//			Draw_Fill (x - 1, y + 9, 168+3 , 1, 0);	//Border - Bottom
			// End Nifty

			Draw_Character(x, y, 16);	// JPG 3.00 from katua - draw [ ] around our score in the
			Draw_Character(x+32, y, 17);	// scoreboard overlay
		}

#if 0
{
	int				total;
	int				n, minutes, tens, units;

	// draw time
		total = cl.completed_time - s->entertime;
		minutes = (int)total/60;
		n = total - minutes*60;
		tens = n/10;
		units = n%10;

		snprintf (num, sizeof(num), "%3i:%i%i", minutes, tens, units);

		Draw_String ( x+48 , y, num);
		}
#endif

	// draw name
		Draw_String (x+48, y, s->name);

		y += 9;
	}
}

/*
==================
Sbar_IntermissionOverlay
==================
*/
void Sbar_IntermissionOverlay (void)
{
	mpic_t	*pic;
	int		dig;
	int		num;
	int		xofs, yofs;

//	scr_copyeverything = 1;
//	scr_fullupdate = 0;

	if (cl.gametype == GAME_DEATHMATCH)
	{
		Sbar_DeathmatchOverlay ();
		return;
	}

	xofs = stdxoffset;
	yofs = 8; // Baker
	pic = Draw_CachePic ("gfx/complete.lmp");
	Draw_Pic (xofs + 64, 24, pic);		// yofs was 24?

	pic = Draw_CachePic ("gfx/inter.lmp");
	Draw_TransPic (xofs, 56, pic);		//56

	// time
	dig = cl.completed_time/60;
	Sbar_IntermissionNumber (xofs + 160, 64+ yofs, dig, 3, 0);
	num = cl.completed_time - dig*60;
	Draw_TransPic (xofs + 234, yofs + 64, sb_colon);
	Draw_TransPic (xofs + 246, yofs + 64, sb_nums[0][num/10]);
	Draw_TransPic (xofs + 270, yofs + 64,  sb_nums[0][num%10]);

	// secrets
	Sbar_IntermissionNumber (xofs + 160, yofs + 104, cl.stats[STAT_SECRETS], 3, 0);
	Draw_TransPic (xofs + 232, yofs + 104, sb_slash);
	Sbar_IntermissionNumber (xofs + 240, yofs + 104, cl.stats[STAT_TOTALSECRETS], 3, 0);

	// monsters
	Sbar_IntermissionNumber (xofs + 160, yofs + 144, cl.stats[STAT_MONSTERS], 3, 0);
	Draw_TransPic (xofs + 232, yofs + 144, sb_slash);
	Sbar_IntermissionNumber (xofs + 240, yofs + 144, cl.stats[STAT_TOTALMONSTERS], 3, 0);

}


/*
==================
Sbar_FinaleOverlay
==================
*/
void Sbar_FinaleOverlay (void)
{
	mpic_t	*pic;

//	scr_copyeverything = 1;

	pic = Draw_CachePic ("gfx/finale.lmp");
	Draw_TransPic ((vid.width-pic->width)/2, 16, pic);
}

int AmmoForGun(int gun)
{
	switch (gun)
	{
	case IT_SHOTGUN:
	case IT_SUPER_SHOTGUN:
		return cl.stats[STAT_SHELLS];

	case IT_NAILGUN:
	case IT_SUPER_NAILGUN:
		return cl.stats[STAT_NAILS];
	case IT_GRENADE_LAUNCHER:
	case IT_ROCKET_LAUNCHER:
		return cl.stats[STAT_ROCKETS];

	case IT_LIGHTNING:
		return cl.stats[STAT_CELLS];

	default:
		  // this shouldn't happen, but it will for hipnotic and quoth and maybe rogue
		return 1;
	}


}


/*
===============
Sbar_DrawStyle2
New SBar style
===============
*/

void Draw_FillRGB (int x, int y, int w, int h, float red, float green, float blue);
void Draw_FillRGBAngle (float x, float y, float w, float h, float red, float green, float blue, float slope);

#if 0
void Sbar_DrawStyle2x (void)
{
	float canvas_x = vid.width
#endif

void Sbar_DrawStyle2 (void)
{

//	Con_Printf("Hud style 2\n");

	// If dead (health <=0 or pressing TAB (sb_showscores)
	//    show deathmatch overlay
	if (sb_showscores || cl.stats[STAT_HEALTH] <= 0)
	{
		Sbar_DrawPic (0, 0, sb_scorebar);
		Sbar_DrawScoreboard ();		// Get out
		return;
	}


	// Alive without TAB

	// Draw the clock
	{
		int minutes, seconds, tens, units;
		int		xprime;
		char	str[80];

		minutes = (int)(cl.ctime / 60);
		seconds = cl.ctime - 60*minutes;
		tens = seconds / 10;
		units = seconds - 10*tens;

		snprintf (str, sizeof(str), "%3i:%i%i", minutes, tens, units);
		xprime = (vid.width/2)-((strlen(str)*8)/2);
		//Con_Printf("xprime %i\n", xprime);
		//Con_Printf("str %s\n", str);
		//Con_Printf("strlen str %i\n", strlen(str));
		Draw_String (xprime, 4, str);
	}

	// Draw the weapons you've got

	{
		void GLDRAW_CircleFilled (float x, float y, float radius, GLfloat red, GLfloat green, GLfloat blue, qbool antialias);
		void GLDRAW_CircleOutlined (float x, float y, float radius, GLfloat red, GLfloat green, GLfloat blue, float linewidth);
		int i;
		float weapons_count = 0;
		float weapon_icon_width = 15;
		float weapon_icon_spacing = 4;
		float cursor_pos_increment = weapon_icon_width + weapon_icon_spacing;
		float start_left, cursor_pos;
		float cur_weapon_width;

		// Count the weapons
		for (i=0 ; i<7 ; i++)
			if (cl.items & (IT_SHOTGUN << i))
				weapons_count++;

		cur_weapon_width =  weapons_count * weapon_icon_width + (weapons_count-1) * weapon_icon_spacing;
		cursor_pos = start_left = (vid.width /2) - cur_weapon_width/2;

		Draw_Fill (start_left - 4, vid.height - cursor_pos_increment -5, (cur_weapon_width) + 7 , cursor_pos_increment -10, 0);

		for (i=0 ; i<7 ; i++)
			if (cl.items & (IT_SHOTGUN << i))
			{
				qbool active_weapon = (cl.stats[STAT_ACTIVEWEAPON] == (IT_SHOTGUN << i)) ? 1 : 0;

				// Outline part


				// Draw
				if (active_weapon)
				{
					// Active weapon
					GLDRAW_CircleFilled		(cursor_pos + 7, vid.height - cursor_pos_increment, 9, 1,1,1, 1);
					GLDRAW_CircleFilled		(cursor_pos + 7, vid.height - cursor_pos_increment, 7, .75, 0.3, 0, 1);
				}
				else if (AmmoForGun(IT_SHOTGUN << i) != 0)
				{
					// Not active weapon; has weapon and ammo for it
					GLDRAW_CircleFilled		(cursor_pos + 7, vid.height - cursor_pos_increment, 9, 0.25, 0.25, 0.25, 1);
//					GLDRAW_CircleFilled		(cursor_pos + 7, vid.height - cursor_pos_increment, 7, .25, .1, .0, 1);
					GLDRAW_CircleFilled		(cursor_pos + 7, vid.height - cursor_pos_increment, 7, .15, .15, .15, 1);
				}
				else
				{
					// Weapon unavailable but has ammo
					GLDRAW_CircleFilled		(cursor_pos + 7, vid.height - cursor_pos_increment, 9, 0.25, 0.25, 0.25, 1);
					GLDRAW_CircleFilled		(cursor_pos + 7, vid.height - cursor_pos_increment, 7, 0, 0, 0, 1);

					{
						// Cross it out
						float x = cursor_pos + 7;
						float y = vid.height - cursor_pos_increment;
						float r = 7;
						float angle1 = .125 + 0.0;
						float angle2 = .125 + 0.25;
						float angle3 = .125 + 0.50;
						float angle4 = .125 + 0.75;

						mglPushStates ();
						MeglColor3f (1, 0, 0); // Red
						MeglDisable (GL_TEXTURE_2D);

						if (engine.Renderer->graphics_api != RENDERER_DIRECT3D) // glLineWidth isn't supported in the wrapper
							eglLineWidth (3); // #ifndef DX8QUAKE_NO_LINEWIDTH

						mglFinishedStates ();

						eglBegin (GL_LINES);
						eglVertex2f (x + sinf(angle2 * D_PI) * r, y + cosf(angle2 * D_PI) * r);
						eglVertex2f (x + sinf(angle4 * D_PI) * r, y + cosf(angle4 * D_PI) * r);
						eglEnd ();

						eglBegin (GL_LINES);
						eglVertex2f (x + sinf(angle1 * D_PI) * r, y + cosf(angle1 * D_PI) * r);
						eglVertex2f (x + sinf(angle3 * D_PI) * r, y + cosf(angle3 * D_PI) * r);
						eglEnd ();

						MeglColor3ubv (color_white); // Restore
						MeglEnable (GL_TEXTURE_2D);

						mglPopStates ();
					}
				}

				Draw_Character			(cursor_pos -4 + 7, (vid.height - cursor_pos_increment)-4, 49 + i+1);


				// Increment
				cursor_pos = cursor_pos + cursor_pos_increment;
			}

//		GLDRAW_Circle ((float)(vid.width / 2), (float)(60 + vid.height /2), 15, 1, 0.5, 0, 0);

//		x + sbar_xofs, y + (vid.height-SBAR_HEIGHT)


	}

	{
		float myang = 0.10;
		float x = 8;
		float xicon = x + 28;
		float xbar =  xicon +10;
		// Health ammo armor
		{

			float y = vid.height -60;

			Draw_String (x, y, va("%3d",cl.stats[STAT_HEALTH]));
			Draw_Alt_String (xicon, y, "H");
//			Draw_Pic (xicon, y, sb_ammo[0]);

			Draw_FillRGBAngle   (xbar, y, 80, 8, 0, 0, 0, myang);


			// Normal health
			if (cl.stats[STAT_HEALTH] <= 50)
				Draw_FillRGBAngle   (xbar, y, 80 * CLAMP (0,(float)cl.stats[STAT_HEALTH]/100, 1), 8, 1, 0, 0, myang);
			else if (cl.stats[STAT_HEALTH] <= 75)
				Draw_FillRGBAngle   (xbar, y, 80 * CLAMP (0,(float)cl.stats[STAT_HEALTH]/100, 1), 8, 1, 1, 0, myang);
			else if (cl.stats[STAT_HEALTH] <= 100)
				Draw_FillRGBAngle   (xbar, y, 80 * CLAMP (0.0f,(float)cl.stats[STAT_HEALTH]/100.0f, 1.0f), 8, 0, 1, 0, myang);
			else
			{
				float pctnormal = 100 / (float)cl.stats[STAT_HEALTH];
				int rando = ((int)(realtime * 10)%6);
				float hue;

				switch (rando)
				{
				case 0: rando = 0; break;
				case 1:
				case 5: rando = 1; break;
				case 2:
				case 4: rando = 2; break;
				case 3: rando = 3; break;
				default: rando =0; break;
				}

				hue = .15+rando*0.05;

				// Blue bar represents super-normal health
				Draw_FillRGBAngle   (xbar, y, 80, 8, hue, hue, 1, myang);
				Draw_FillRGBAngle   (xbar, y, 80 * pctnormal, 8, 0, 1, 0, myang);
			}
		}

		// Draw Armor

		{
			float y = vid.height -45;
			float pctnormal;

			Draw_String (x, y, va("%3d",cl.stats[STAT_ARMOR]));
			Draw_Alt_String (xicon, y, "A");

			Draw_FillRGBAngle   (xbar, y, 80, 8, 0, 0, 0, myang);
			if (cl.items & IT_ARMOR3)
			{
				pctnormal = ((float)CLAMP (0,cl.stats[STAT_ARMOR],200) / 200.0f);

				Draw_FillRGBAngle   (xbar, y, 80 * pctnormal, 8, 1, 0, 0, myang);
			}
			else if (cl.items & IT_ARMOR2)
			{
				pctnormal = ((float)CLAMP (0,cl.stats[STAT_ARMOR],150) / 150.0f);
				Draw_FillRGBAngle   (xbar, y, 80 * pctnormal, 8, 1, 1, 0, myang);
			}
			else if (cl.items & IT_ARMOR1)
			{
				pctnormal = ((float)CLAMP (0,cl.stats[STAT_ARMOR],100) / 100.0f);
				Draw_FillRGBAngle   (xbar, y, 80 * pctnormal, 8, 0, 1, 0, myang);
			}
		}

		// Draw Ammo
		{
			float y = vid.height - 30;
			float pctnormal = ((float)CLAMP (0,cl.stats[STAT_AMMO],100) / 100.0f);

			Draw_String (x, y, va("%3d",cl.stats[STAT_AMMO]));
			Draw_SubPic (xicon-33, y, sb_ibar,  3+(0*48), 0, 42, 11);

			Draw_FillRGBAngle   (xbar, y, 80, 8, 0, 0, 0, myang);

			if (cl.stats[STAT_AMMO] < 10)
				Draw_FillRGBAngle   (xbar, y, 80 * pctnormal, 8, 1, 0, 0, myang);
			else if (cl.stats[STAT_AMMO] < 20)
				Draw_FillRGBAngle   (xbar, y, 80 * pctnormal, 8, 1, 1, 0, myang);
			else
				Draw_FillRGBAngle   (xbar, y, 80 * pctnormal, 8, 0, 1, 0, myang);
		}
	}

	// If single player draw ther monsters
#if 0
	if (cl.gametype == GAME_COOP)
	{
		// At top of screen in middle
		Draw_AlphaFill

	}

#endif



	if (cl.gametype == GAME_DEATHMATCH)
	{
		// If deathmatch draw the frags
		Sbar_SortFrags ();

		{
			int i;
			int xfrag = vid.width - (3+1+15)*8 + 4;
			int xname = xfrag +32;
			int ytop  = 8;
			int yspace = 14;
			for (i=0 ; i < scoreboardlines && i < 4; i++)
			{
				Draw_AlphaFill    (xfrag-2, ytop + yspace * i - 2, 19*8 +4, 12, 254 , 0.2);
				Draw_String		(xfrag, ytop + yspace * i, va("%3i",cl.scores[fragsort[i]].frags));
				Draw_String		(xname, ytop + yspace * i, cl.scores[fragsort[i]].name);
			}
		}
	}

//	if (cl.gametype == GAME_COOP)




#if 0
	if deathmatch
		// Draw top
		// Draw winning
		// Draw frags
	else
		// Draw top with monsters

	draw health
	draw armor
	draw ammo

	draw what guns

	if deathmatch
		draw minioverlay side

	// Draw Game Clock
#endif

}


void Sbar_Hipnotic_Init (void)
{
	int	i;

	hsb_weapons[0][0] = Draw_PicFromWad ("inv_laser");
	hsb_weapons[0][1] = Draw_PicFromWad ("inv_mjolnir");
	hsb_weapons[0][2] = Draw_PicFromWad ("inv_gren_prox");
	hsb_weapons[0][3] = Draw_PicFromWad ("inv_prox_gren");
	hsb_weapons[0][4] = Draw_PicFromWad ("inv_prox");

	hsb_weapons[1][0] = Draw_PicFromWad ("inv2_laser");
	hsb_weapons[1][1] = Draw_PicFromWad ("inv2_mjolnir");
	hsb_weapons[1][2] = Draw_PicFromWad ("inv2_gren_prox");
	hsb_weapons[1][3] = Draw_PicFromWad ("inv2_prox_gren");
	hsb_weapons[1][4] = Draw_PicFromWad ("inv2_prox");

	for (i=0 ; i<5 ; i++)
	{
		hsb_weapons[2+i][0] = Draw_PicFromWad (va("inva%i_laser", i+1));
		hsb_weapons[2+i][1] = Draw_PicFromWad (va("inva%i_mjolnir", i+1));
		hsb_weapons[2+i][2] = Draw_PicFromWad (va("inva%i_gren_prox", i+1));
		hsb_weapons[2+i][3] = Draw_PicFromWad (va("inva%i_prox_gren", i+1));
		hsb_weapons[2+i][4] = Draw_PicFromWad (va("inva%i_prox", i+1));
	}

	hsb_items[0] = Draw_PicFromWad ("sb_wsuit");
	hsb_items[1] = Draw_PicFromWad ("sb_eshld");

// joe: better reload these, coz they might look different
	if (hipnotic)
	{
		sb_weapons[0][4] = Draw_PicFromWad ("inv_rlaunch");
		sb_weapons[1][4] = Draw_PicFromWad ("inv2_rlaunch");
		sb_items[0] = Draw_PicFromWad ("sb_key1");
		sb_items[1] = Draw_PicFromWad ("sb_key2");
	}
}

void Sbar_Rogue_Init (void)
{
	rsb_invbar[0] = Draw_PicFromWad ("r_invbar1");
	rsb_invbar[1] = Draw_PicFromWad ("r_invbar2");

	rsb_weapons[0] = Draw_PicFromWad ("r_lava");
	rsb_weapons[1] = Draw_PicFromWad ("r_superlava");
	rsb_weapons[2] = Draw_PicFromWad ("r_gren");
	rsb_weapons[3] = Draw_PicFromWad ("r_multirock");
	rsb_weapons[4] = Draw_PicFromWad ("r_plasma");

	rsb_items[0] = Draw_PicFromWad ("r_shield1");
	rsb_items[1] = Draw_PicFromWad ("r_agrav1");

// PGM 01/19/97 - team color border
	rsb_teambord = Draw_PicFromWad ("r_teambord");
// PGM 01/19/97 - team color border

	rsb_ammo[0] = Draw_PicFromWad ("r_ammolava");
	rsb_ammo[1] = Draw_PicFromWad ("r_ammomulti");
	rsb_ammo[2] = Draw_PicFromWad ("r_ammoplasma");
}


/*
===============
Sbar_LoadPics -- johnfitz -- load all the sbar pics
===============
*/
void Sbar_LoadPics (void)
{
	int	i;

	for (i=0 ; i<10 ; i++)
	{
		sb_nums[0][i] = Draw_PicFromWad (va("num_%i", i));
		sb_nums[1][i] = Draw_PicFromWad (va("anum_%i", i));
	}

	sb_nums[0][10] = Draw_PicFromWad ("num_minus");
	sb_nums[1][10] = Draw_PicFromWad ("anum_minus");

	sb_colon = Draw_PicFromWad ("num_colon");
	sb_slash = Draw_PicFromWad ("num_slash");

	sb_weapons[0][0] = Draw_PicFromWad ("inv_shotgun");
	sb_weapons[0][1] = Draw_PicFromWad ("inv_sshotgun");
	sb_weapons[0][2] = Draw_PicFromWad ("inv_nailgun");
	sb_weapons[0][3] = Draw_PicFromWad ("inv_snailgun");
	sb_weapons[0][4] = Draw_PicFromWad ("inv_rlaunch");
	sb_weapons[0][5] = Draw_PicFromWad ("inv_srlaunch");
	sb_weapons[0][6] = Draw_PicFromWad ("inv_lightng");

	sb_weapons[1][0] = Draw_PicFromWad ("inv2_shotgun");
	sb_weapons[1][1] = Draw_PicFromWad ("inv2_sshotgun");
	sb_weapons[1][2] = Draw_PicFromWad ("inv2_nailgun");
	sb_weapons[1][3] = Draw_PicFromWad ("inv2_snailgun");
	sb_weapons[1][4] = Draw_PicFromWad ("inv2_rlaunch");
	sb_weapons[1][5] = Draw_PicFromWad ("inv2_srlaunch");
	sb_weapons[1][6] = Draw_PicFromWad ("inv2_lightng");

	for (i=0 ; i<5 ; i++)
	{
		sb_weapons[2+i][0] = Draw_PicFromWad (va("inva%i_shotgun", i+1));
		sb_weapons[2+i][1] = Draw_PicFromWad (va("inva%i_sshotgun", i+1));
		sb_weapons[2+i][2] = Draw_PicFromWad (va("inva%i_nailgun", i+1));
		sb_weapons[2+i][3] = Draw_PicFromWad (va("inva%i_snailgun", i+1));
		sb_weapons[2+i][4] = Draw_PicFromWad (va("inva%i_rlaunch", i+1));
		sb_weapons[2+i][5] = Draw_PicFromWad (va("inva%i_srlaunch", i+1));
		sb_weapons[2+i][6] = Draw_PicFromWad (va("inva%i_lightng", i+1));
	}

	sb_ammo[0] = Draw_PicFromWad ("sb_shells");
	sb_ammo[1] = Draw_PicFromWad ("sb_nails");
	sb_ammo[2] = Draw_PicFromWad ("sb_rocket");
	sb_ammo[3] = Draw_PicFromWad ("sb_cells");

	sb_armor[0] = Draw_PicFromWad ("sb_armor1");
	sb_armor[1] = Draw_PicFromWad ("sb_armor2");
	sb_armor[2] = Draw_PicFromWad ("sb_armor3");

	sb_items[0] = Draw_PicFromWad ("sb_key1");
	sb_items[1] = Draw_PicFromWad ("sb_key2");
	sb_items[2] = Draw_PicFromWad ("sb_invis");
	sb_items[3] = Draw_PicFromWad ("sb_invuln");
	sb_items[4] = Draw_PicFromWad ("sb_suit");
	sb_items[5] = Draw_PicFromWad ("sb_quad");

	sb_sigil[0] = Draw_PicFromWad ("sb_sigil1");
	sb_sigil[1] = Draw_PicFromWad ("sb_sigil2");
	sb_sigil[2] = Draw_PicFromWad ("sb_sigil3");
	sb_sigil[3] = Draw_PicFromWad ("sb_sigil4");

	sb_faces[4][0] = Draw_PicFromWad ("face1");
	sb_faces[4][1] = Draw_PicFromWad ("face_p1");
	sb_faces[3][0] = Draw_PicFromWad ("face2");
	sb_faces[3][1] = Draw_PicFromWad ("face_p2");
	sb_faces[2][0] = Draw_PicFromWad ("face3");
	sb_faces[2][1] = Draw_PicFromWad ("face_p3");
	sb_faces[1][0] = Draw_PicFromWad ("face4");
	sb_faces[1][1] = Draw_PicFromWad ("face_p4");
	sb_faces[0][0] = Draw_PicFromWad ("face5");
	sb_faces[0][1] = Draw_PicFromWad ("face_p5");

	sb_face_invis = Draw_PicFromWad ("face_invis");
	sb_face_invuln = Draw_PicFromWad ("face_invul2");
	sb_face_invis_invuln = Draw_PicFromWad ("face_inv2");
	sb_face_quad = Draw_PicFromWad ("face_quad");


	sb_sbar = Draw_PicFromWad ("sbar");
	sb_ibar = Draw_PicFromWad ("ibar");
	sb_scorebar = Draw_PicFromWad ("scorebar");

//MED 01/04/97 added new hipnotic weapons
	if (hipnotic)
		Sbar_Hipnotic_Init ();

	if (rogue)
		Sbar_Rogue_Init ();
}

/*
===============
Sbar_Init -- johnfitz -- rewritten
===============
*/

void Sbar_Init (void)
{
	Cmd_AddCommand ("+showscores", Sbar_ShowScores_f);
	Cmd_AddCommand ("-showscores", Sbar_DontShowScores_f);

	Cvar_Registration_Client_Sbar ();

	Sbar_LoadPics ();
}
