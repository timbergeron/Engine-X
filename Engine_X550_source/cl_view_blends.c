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
// cl_view_blends.c -- calculate color shifts and crap

#include "quakedef.h"



// default cshifts are too dark in GL so lighten them a little

// Baker: For the v_cshift command so it isn't messing with cshift_empty
static cshift_t	cshift_custom	= {{130, 80,  50}, 0};

static cshift_t	cshift_empty 	= {{130, 80,  50}, 0};
static cshift_t	cshift_water 	= {{130, 80,  50}, 100};
static cshift_t	cshift_slime 	= {{0,   25,   5}, 64};
static cshift_t	cshift_lava 	= {{255, 80,   0}, 64};   // Was 150 ... but that is WAY too much

static cshift_t	cshift_kwater 	= {{64,  64, 128}, 128 }; // Blue water for Kurok

/*
==============================================================================

				PALETTE FLASHES - EXTERNAL FACTORS

==============================================================================
*/


/*
==================
V_cshift_f - Allow QuakeC mods access to do cshifts

cshift_empty = {{130, 80, 50}, 0}
==================
*/

void Viewblends_NewMap (void)
{
	cshift_custom.percent = 0;
}

void Viewblends_DefineEmptyColor_cshiftcmd_f (void)
{
	if (!(Cmd_Argc () == 5))
	{
		Con_Printf ("Usage: %s <red> <green> <blue> <percent>\n", Cmd_Argv (0));
		Con_Printf ("       Each param range 0-255\n");
		return;
	}

	cshift_custom.destcolor[0]	= atoi(Cmd_Argv(1));
	cshift_custom.destcolor[1]	= atoi(Cmd_Argv(2));
	cshift_custom.destcolor[2]	= atoi(Cmd_Argv(3));
	cshift_custom.percent		= atoi(Cmd_Argv(4));
}

void Viewblends_CalculateDamage_ColorBucket_f (const float count, const int armor, const int blood)
{
	float	fraction;

	// Calculate the percent

	cl.cshifts[CSHIFT_DAMAGE].percent += 3*count;
	cl.cshifts[CSHIFT_DAMAGE].percent = CLAMP (0, cl.cshifts[CSHIFT_DAMAGE].percent, 150);

	// Apply cvar reduction option to percent

	fraction = CLAMP (0, vb_damagecshift.floater, 1);
	cl.cshifts[CSHIFT_DAMAGE].percent *= fraction;

	// Calculate the color from your shielding (armor) and the hurt

	if (armor > blood)
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 200;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 100;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 100;
	}
	else if (armor)
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 220;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 50;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 50;
	}
	else
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 255;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 0;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 0;
	}
}

/*
==================
V_BonusFlash_f

When you run over an item, the server sends this command
==================
*/
void Viewblends_SetBonus_ColorBucket_f (void)
{
	cl.cshifts[CSHIFT_BONUS].destcolor[0] = 215;
	cl.cshifts[CSHIFT_BONUS].destcolor[1] = 186;
	cl.cshifts[CSHIFT_BONUS].destcolor[2] = 69;
	cl.cshifts[CSHIFT_BONUS].percent = 50;
}

/*
==============================================================================

				PALETTE FLASHES - BUCKETS

==============================================================================
*/



/*
=============
V_SetContentsColor

Underwater, lava, etc each has a color shift
=============
*/
static void Viewblends_SetContents_ColorBucket (const int contents)
{
	// Caustics functions to let you know you are submerged without an annoying blend
	
	if (r_pass_caustics.integer)	
	{
		// If caustics, ignore content shift 
		// Unless mod is using cshift_custom
		if (contents == CONTENTS_EMPTY && cshift_custom.percent)
			cl.cshifts[CSHIFT_CONTENTS] = cshift_custom;
		else
			cl.cshifts[CSHIFT_CONTENTS] = cshift_empty;				
		return;
	}

	// No caustics ... kick it up old school
	switch (contents)
	{
	case CONTENTS_EMPTY:
	case CONTENTS_SOLID:
		if (cshift_custom.percent)
			cl.cshifts[CSHIFT_CONTENTS] = cshift_custom;
		else
			cl.cshifts[CSHIFT_CONTENTS] = cshift_empty;
		break;
	case CONTENTS_LAVA:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_lava;
		break;
	case CONTENTS_SLIME:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_slime;
		break;
	default:
		if (game_kurok.integer)
			cl.cshifts[CSHIFT_CONTENTS] = cshift_kwater;
		else 
			cl.cshifts[CSHIFT_CONTENTS] = cshift_water;
	}
}

/*
=============
V_CalcPowerupCshift
=============
*/
static void Viewblends_CalculatePowerup_ColorBucket (void)
{
	float	fraction;

	if (cl.items & IT_QUAD)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 255;
		fraction = CLAMP (0, vb_quadcshift.floater, 1);
		cl.cshifts[CSHIFT_POWERUP].percent = (25 + sinf(cl.ctime*100)*10) * fraction;
	}
	else if (cl.items & IT_SUIT)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		fraction = CLAMP (0, vb_suitcshift.floater, 1);
		cl.cshifts[CSHIFT_POWERUP].percent = 20 * fraction;
	}
	else if (cl.items & IT_INVISIBILITY)
	{
		if (r_viewmodel_ringalpha.floater < 1)	// Ring alpha lets you know you are invisible without an effect
		{
			cl.cshifts[CSHIFT_POWERUP].destcolor[0] =
			cl.cshifts[CSHIFT_POWERUP].destcolor[1] =
			cl.cshifts[CSHIFT_POWERUP].destcolor[2] =
			cl.cshifts[CSHIFT_POWERUP].percent      = 0;
			return;
		}

		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 100;
		fraction = CLAMP (0, vb_ringcshift.floater, 1);
		cl.cshifts[CSHIFT_POWERUP].percent = 70 * fraction;
	}
	else if (cl.items & IT_INVULNERABILITY)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		fraction = CLAMP (0, vb_pentcshift.floater, 1);
		cl.cshifts[CSHIFT_POWERUP].percent = 30 * fraction;
	}
	else cl.cshifts[CSHIFT_POWERUP].percent = 0;
}

void Viewblends_FadeDamageBonus_Buckets (void)
{
	// drop the damage value
	cl.cshifts[CSHIFT_DAMAGE].percent -= host_frametime * 150;

	if (cl.cshifts[CSHIFT_DAMAGE].percent <= 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;

	// drop the bonus value
	cl.cshifts[CSHIFT_BONUS].percent -= host_frametime * 100;

	if (cl.cshifts[CSHIFT_BONUS].percent <= 0)
		cl.cshifts[CSHIFT_BONUS].percent = 0;
}

/*
==============================================================================

				PALETTE FLASHES: CALCULATE V_BLEND (FINAL OUTPUT)

==============================================================================
*/

float			v_blend[4];

/*
=============
V_CalcBlend
=============
*/
//#define	CSHIFT_CONTENTS	0
//#define	CSHIFT_DAMAGE	1
//#define	CSHIFT_BONUS	2
//#define	CSHIFT_POWERUP	3
static void Viewblends_MixBuckets_GetFinalColor (void)
{
	float	r=0, g=0, b=0, a=0, a2=0;
	int	j;

	r = 0;
	g = 0;
	b = 0;
	a = 0;

	for (j=0 ; j<NUM_CSHIFTS ; j++)
	{
		// no shift
		if (!cl.cshifts[j].percent) continue;

		// calc alpha amount
		a2 = (float) cl.cshifts[j].percent / 255.0;

		// evaluate blends
		a = a + a2 * (1 - a);
		a2 = a2 / a;

		// blend it in
		r = r * (1 - a2) + cl.cshifts[j].destcolor[0] * a2;
		g = g * (1 - a2) + cl.cshifts[j].destcolor[1] * a2;
		b = b * (1 - a2) + cl.cshifts[j].destcolor[2] * a2;
	}

	// set final amounts
	v_blend[0] = r / 255.0;
	v_blend[1] = g / 255.0;
	v_blend[2] = b / 255.0;
	v_blend[3] = CLAMP (0, a, 1);

	// clamp blend 

	v_blend[0] = CLAMP (0, v_blend[0], 1);
	v_blend[1] = CLAMP (0, v_blend[1], 1);
	v_blend[2] = CLAMP (0, v_blend[2], 1);
}

/*
==============================================================================

				PALETTE FLASHES: THE WHOLE PROCESS (EXCLUDING EXTERNAL FACTORS)

==============================================================================
*/


void Viewblends_CalculateFrame (const int mapcontents)
{

	Viewblends_CalculatePowerup_ColorBucket ();					// Once per frame

	Viewblends_SetContents_ColorBucket (r_viewleaf->contents);	// Once per view
	Viewblends_MixBuckets_GetFinalColor ();						// Once per view

	// Post-calculation action
	// Technically shouldn't occur if paused ... but this isn't that important

// Moved to post-scene
//	Viewblends_FadeDamageBonus_Buckets ();						// Once post frame

}

