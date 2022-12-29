/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2007-2008 Kristian Duske

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
//gl_fog.c -- global and volumetric fog

#include "quakedef.h"

//==============================================================================
//
//  GLOBAL FOG
//
//==============================================================================

static float fog_density;
static float kfog_start; // Kurok
static float kfog_end;   // Kurok
static float fog_red;
static float fog_green;
static float fog_blue;

static float old_density;
static float kold_start; // Kurok
static float kold_end;   // Kurok
static float old_red;
static float old_green;
static float old_blue;

static float fade_time; //duration of fade
static float fade_done; //time when fade will be done


/*
=============
Fog_Update

update internal variables
=============
*/
static void sFog_Update_Kurok (float start, float end, float red, float green, float blue, float time)
{
	//save previous settings for fade
	if (time > 0)
	{
		//check for a fade in progress
		if (fade_done > cl.time)
		{
			float f;//, d;

			f = (fade_done - cl.time) / fade_time;
			kold_start = f * kold_start + (1.0 - f) * kfog_start;
			kold_end = f * kold_end + (1.0 - f) * kfog_end;
			old_red = f * old_red + (1.0 - f) * fog_red;
			old_green = f * old_green + (1.0 - f) * fog_green;
			old_blue = f * old_blue + (1.0 - f) * fog_blue;
		}
		else
		{
			kold_start = kfog_start;
			kold_end = kfog_end;
			old_red = fog_red;
			old_green = fog_green;
			old_blue = fog_blue;
		}
	}

	kfog_start = start;
	kfog_end = end;
	fog_red = red;
	fog_green = green;
	fog_blue = blue;
	fade_time = time;
	fade_done = cl.time + time;
}

/*
=============
Fog_Update

update internal variables
=============
*/
static void sFog_Update (float density, float red, float green, float blue, float time)
{
	//save previous settings for fade
	if (time > 0)
	{
		//check for a fade in progress
		if (fade_done > cl.time)
		{
			float f;//, d; unused -- kristian

			f = (fade_done - cl.time) / fade_time;
			old_density = f * old_density + (1.0 - f) * fog_density;
			old_red = f * old_red + (1.0 - f) * fog_red;
			old_green = f * old_green + (1.0 - f) * fog_green;
			old_blue = f * old_blue + (1.0 - f) * fog_blue;
		}
		else
		{
			old_density = fog_density;
			old_red = fog_red;
			old_green = fog_green;
			old_blue = fog_blue;
		}
	}

	fog_density = density;
	fog_red = red;
	fog_green = green;
	fog_blue = blue;
	fade_time = time;
	fade_done = cl.time + time;
}

/*
=============
Fog_ParseServerMessage

handle an SVC_FOG message from server
=============
*/
void Fog_ParseServerMessage (void)
{
//	float start, end; // Kurok
	float density, red, green, blue, time;

#if 0  // Thank god Kurok isn't use svc_fog ... this would be another protocol quirk to kill
	if (game_kurok.integer)
    {
		start = MSG_ReadByte() / 255.0;
		end = MSG_ReadByte() / 255.0;
    }
	else
#endif
	density = MSG_ReadByte() / 255.0;
	red = MSG_ReadByte() / 255.0;
	green = MSG_ReadByte() / 255.0;
	blue = MSG_ReadByte() / 255.0;
	time = max(0.0, MSG_ReadShort() / 100.0);

#if 0
	if (game_kurok.integer)
		sFog_Update_Kurok (start, end, red, green, blue, time);
	else
#endif // Thank god Kurok isn't use svc_fog ... this would be another protocol quirk to kill
		sFog_Update (density, red, green, blue, time);
}

/*
=============
Fog_FogCommand_f

handle the 'fog' console command
=============
*/
static void sFog_FogCommand_f (void)
{
	if (game_kurok.integer)
	{
		switch (Cmd_Argc())
		{
		default:
		case 1:
			Con_Printf("usage:\n");
			Con_Printf("   fog <fade>\n");
			Con_Printf("   fog <start> <end>\n");
			Con_Printf("   fog <red> <green> <blue>\n");
			Con_Printf("   fog <fade> <red> <green> <blue>\n");
			Con_Printf("   fog <start> <end> <red> <green> <blue>\n");
			Con_Printf("   fog <start> <end> <red> <green> <blue> <fade>\n");
			Con_Printf("current values:\n");
			Con_Printf("   \"start\" is \"%f\"\n", kfog_start);
			Con_Printf("   \"end\" is \"%f\"\n", kfog_end);
			Con_Printf("   \"red\" is \"%f\"\n", fog_red);
			Con_Printf("   \"green\" is \"%f\"\n", fog_green);
			Con_Printf("   \"blue\" is \"%f\"\n", fog_blue);
			Con_Printf("   \"fade\" is \"%f\"\n", fade_time);
			break;
		case 2: //TEST
			sFog_Update_Kurok(kfog_start,
					   kfog_end,
					   fog_red,
					   fog_green,
					   fog_blue,
					   0.0);
			break;
		case 3:
			sFog_Update_Kurok(atof(Cmd_Argv(1)),
					   atof(Cmd_Argv(2)),
					   fog_red,
					   fog_green,
					   fog_blue,
					   0.0);
			break;
		case 4:
			sFog_Update_Kurok(kfog_start,
					   kfog_end,
					   CLAMP(0.0, atof(Cmd_Argv(1)), 100.0),
					   CLAMP(0.0, atof(Cmd_Argv(2)), 100.0),
					   CLAMP(0.0, atof(Cmd_Argv(3)), 100.0),
					   0.0);
			break;
		case 5: //TEST
			sFog_Update_Kurok(kfog_start,
					   kfog_end,
					   CLAMP(0.0, atof(Cmd_Argv(1)), 100.0),
					   CLAMP(0.0, atof(Cmd_Argv(2)), 100.0),
					   CLAMP(0.0, atof(Cmd_Argv(3)), 100.0),
					   0.0);
			break;
		case 6:
			sFog_Update_Kurok(atof(Cmd_Argv(1)),
					   atof(Cmd_Argv(2)),
					   CLAMP(0.0, atof(Cmd_Argv(3)), 100.0),
					   CLAMP(0.0, atof(Cmd_Argv(4)), 100.0),
					   CLAMP(0.0, atof(Cmd_Argv(5)), 100.0),
					   0.0);
			break;
		case 7:
			sFog_Update_Kurok(atof(Cmd_Argv(1)),
					   atof(Cmd_Argv(2)),
					   CLAMP(0.0, atof(Cmd_Argv(3)), 100.0),
					   CLAMP(0.0, atof(Cmd_Argv(4)), 100.0),
					   CLAMP(0.0, atof(Cmd_Argv(5)), 100.0),
					   0.0);
			break;
		}

		return;
	}

	// Normal use starts here
	switch (Cmd_Argc())
	{
	default:
	case 1:
		Con_Printf("usage:\n");
		Con_Printf("   fog <density>\n");
		Con_Printf("   fog <red> <green> <blue>\n");
		Con_Printf("   fog <density> <red> <green> <blue>\n");
		Con_Printf("current values:\n");
		Con_Printf("   \"density\" is \"%f\"\n", fog_density);
		Con_Printf("   \"red\" is \"%f\"\n", fog_red);
		Con_Printf("   \"green\" is \"%f\"\n", fog_green);
		Con_Printf("   \"blue\" is \"%f\"\n", fog_blue);
		break;
	case 2:
		sFog_Update(max(0.0, atof(Cmd_Argv(1))),
				   fog_red,
				   fog_green,
				   fog_blue,
				   0.0);
		break;
	case 3: //TEST
		sFog_Update(max(0.0, atof(Cmd_Argv(1))),
				   fog_red,
				   fog_green,
				   fog_blue,
				   atof(Cmd_Argv(2)));
		break;
	case 4:
		sFog_Update(fog_density,
				   CLAMP(0.0, atof(Cmd_Argv(1)), 1.0),
				   CLAMP(0.0, atof(Cmd_Argv(2)), 1.0),
				   CLAMP(0.0, atof(Cmd_Argv(3)), 1.0),
				   0.0);
		break;
	case 5:
		sFog_Update(max(0.0, atof(Cmd_Argv(1))),
				   CLAMP(0.0, atof(Cmd_Argv(2)), 1.0),
				   CLAMP(0.0, atof(Cmd_Argv(3)), 1.0),
				   CLAMP(0.0, atof(Cmd_Argv(4)), 1.0),
				   0.0);
		break;
	case 6: //TEST
		sFog_Update(max(0.0, atof(Cmd_Argv(1))),
				   CLAMP(0.0, atof(Cmd_Argv(2)), 1.0),
				   CLAMP(0.0, atof(Cmd_Argv(3)), 1.0),
				   CLAMP(0.0, atof(Cmd_Argv(4)), 1.0),
				   atof(Cmd_Argv(5)));
		break;
	}
}

/*
=============
Fog_ParseWorldspawn

called at map load
=============
*/

static void sFog_ResetLocals (void)
{
	//initially no fog
	fog_density = 0.0;
	old_density = 0.0;
	// Kurok starts here
	kfog_start = 50.0;
	kold_start = 0.0;

	kfog_end = 800.0;
	kold_end = 0.0;

	fog_red = 0.3;
	old_red = 0.0;

	fog_green = 0.3;
	old_green = 0.0;

	fog_blue = 0.3;
	old_blue = 0.0;
	// End Kurok
	fade_time = 0.0;
	fade_done = 0.0;
}

static void Fog_ParseWorldspawn (void)
{
	char	*valuestring;

	if ((valuestring = StringTemp_ObtainValueFromClientWorldSpawn("fog")))
	{

		// Baker: This looks dangerous and untrustworthy really ...
		if (game_kurok.integer)
			sscanf(valuestring, "%f %f %f %f %f", &kfog_start, &kfog_end, &fog_red, &fog_green, &fog_blue);

		else
			sscanf(valuestring, "%f %f %f %f", &fog_density, &fog_red, &fog_green, &fog_blue);
	}
}

/*
=============
Fog_GetColor

calculates fog color for this frame, taking into account fade times
=============
*/
float s, e;
static float *sFog_GetColor (void)
{
	static float c[4];
	float f;
	int i;

	if (game_kurok.integer)
	{
		if (fade_done > cl.time)
		{
			f = (fade_done - cl.time) / fade_time;
			s = f * kold_start + (1.0 - f) * kfog_start;
			e = f * kold_end + (1.0 - f) * kfog_end;
			c[0] = f * old_red + (1.0 - f) * fog_red * 0.01;
			c[1] = f * old_green + (1.0 - f) * fog_green * 0.01;
			c[2] = f * old_blue + (1.0 - f) * fog_blue * 0.01;
			c[3] = 1.0;
		}
		else
		{
			s = kfog_start;
			e = kfog_end;
			c[0] = fog_red * 0.01;
			c[1] = fog_green * 0.01;
			c[2] = fog_blue * 0.01;
			c[3] = 1.0;
		}
	}
	else
	{


		if (fade_done > cl.time)
		{
			f = (fade_done - cl.time) / fade_time;
			c[0] = f * old_red + (1.0 - f) * fog_red;
			c[1] = f * old_green + (1.0 - f) * fog_green;
			c[2] = f * old_blue + (1.0 - f) * fog_blue;
			c[3] = 1.0;
		}
		else
		{
			c[0] = fog_red;
			c[1] = fog_green;
			c[2] = fog_blue;
			c[3] = 1.0;
		}
	}

	//find closest 24-bit RGB value, so solid-colored sky can match the fog perfectly
	for (i=0;i<3;i++)
		c[i] = (float)(Q_rint(c[i] * 255)) / 255.0f;


	return c;
}

/*
=============
Fog_GetDensity

returns current density of fog
=============
*/
static float sFog_GetDensity (void)
{
	float f;

	if (game_kurok.integer)
	{
		if (fade_done > cl.time)
		{
			f = (fade_done - cl.time) / fade_time;
			return f * kold_end + (1.0 - f) * kfog_end;
		}
		else
			return kfog_end;

//		return;
	}


	if (fade_done > cl.time)
	{
		f = (fade_done - cl.time) / fade_time;
		return f * old_density + (1.0 - f) * fog_density;
	}
	else
		return fog_density;
}

/*
=============
Fog_SetupFrame

called at the beginning of each frame
=============
*/
void Fog_SetupFrame (void)
{

	MeglFogfv(GL_FOG_COLOR, sFog_GetColor());
	if (game_kurok.integer)
	{

// Baker: This needs to be in setup frame
// GL commands not in the frame are unacceptable
			float startfogdist = s;
			float endfogdist = e;
			float temp;

			if (startfogdist > endfogdist)
			{
				temp = startfogdist;
				startfogdist = endfogdist;
				endfogdist = temp;
			}
			MeglFogi(GL_FOG_MODE, GL_LINEAR);
			MeglFogf(GL_FOG_START, startfogdist);
			MeglFogf(GL_FOG_END, endfogdist);
	}
	else
	{
		MeglFogi(GL_FOG_MODE, GL_EXP2);
		MeglFogf(GL_FOG_DENSITY, sFog_GetDensity() / 64.0);
	}
}

/*
=============
Fog_EnableGFog

called before drawing stuff that should be fogged
=============
*/
void Fog_EnableGFog (void)
{
	if (r_fog.integer)
		if (sFog_GetDensity() > 0)
			MeglEnable(GL_FOG);
}

/*
=============
Fog_DisableGFog

called after drawing stuff that should be fogged
=============
*/
void Fog_DisableGFog (void)
{
	if (sFog_GetDensity() > 0)
		MeglDisable(GL_FOG);
}

/*
=============
Fog_StartAdditive

called before drawing stuff that is additive blended -- sets fog color to black
Baker: We aren't referencing this anywhere.  FitzQuake uses this for non-Mtex pathways
=============
*/
static void NotUsed_Yet_Fog_StartAdditive (void)
{
	vec3_t color = {0,0,0};

	if (sFog_GetDensity() > 0)
		MeglFogfv(GL_FOG_COLOR, color);
	}

/*
=============
Fog_StopAdditive

called after drawing stuff that is additive blended -- restores fog color
Baker: We aren't referencing this anywhere.  FitzQuake uses this for non-Mtex pathways
=============
*/
static void NotUsed_Yet_Fog_StopAdditive (void)
	{
	if (sFog_GetDensity() > 0)
		MeglFogfv(GL_FOG_COLOR, sFog_GetColor());
}

//==============================================================================
//
//  VOLUMETRIC FOG
//
//==============================================================================

//cvar_t r_vfog = {"r_vfog", "1"};

//void Fog_DrawVFog (void){}
//void Fog_MarkModels (void){}

//==============================================================================
//
//  INIT
//
//==============================================================================

/*
=============
Fog_NewMap

called whenever a map is loaded
=============
*/
void Fog_NewMap (void)
{
	sFog_ResetLocals ();
	Fog_ParseWorldspawn (); //for global fog
//	Fog_MarkModels (); //for volumetric fog
}






/*
=============
Fog_Init

called when quake initializes
=============
*/
void Fog_Init (void)
{
	Cmd_AddCommand ("fog", sFog_FogCommand_f);
	//sFog_Init_Reset ();

}
