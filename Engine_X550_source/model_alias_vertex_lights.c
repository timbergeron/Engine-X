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
// model_alias.c -- alias model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

#include "quakedef.h"

float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

float	vlight_pitch = 45;
float	vlight_yaw = 45;
float	vlight_highcut = 128;
float	vlight_lowcut = 60;

#define NUMVERTEXNORMALS	162
extern	float	r_avertexnormals[NUMVERTEXNORMALS][3];

static byte	anorm_pitch[NUMVERTEXNORMALS];
static byte	anorm_yaw[NUMVERTEXNORMALS];

static byte	vlighttable[256][256];

float VertexLights_GetVertexLightValue (const int index, const float my_apitch, const float my_ayaw)
{
	int	pitchofs, yawofs;
	float	retval;

	pitchofs = anorm_pitch[index] + (my_apitch * 256 / 360);
	yawofs = anorm_yaw[index] + (my_ayaw * 256 / 360);
	while (pitchofs > 255)
		pitchofs -= 256;
	while (yawofs > 255)
		yawofs -= 256;
	while (pitchofs < 0)
		pitchofs += 256;
	while (yawofs < 0)
		yawofs += 256;

	retval = vlighttable[pitchofs][yawofs];

	return retval / 256;
}

float VertexLights_GetVertexLightValueMD3 (byte ppitch, byte pyaw, float apitch, float ayaw)
{
	int	pitchofs, yawofs;
	float	retval;

	pitchofs = ppitch + (apitch * 256 / 360);
	yawofs = pyaw + (ayaw * 256 / 360);
	while (pitchofs > 255)
		pitchofs -= 256;
	while (yawofs > 255)
		yawofs -= 256;
	while (pitchofs < 0)
		pitchofs += 256;
	while (yawofs < 0)
		yawofs += 256;

	retval = vlighttable[pitchofs][yawofs];

	return retval / 256;
}

float VertexLights_LerpVertexLightMD3 (byte ppitch1, byte pyaw1, byte ppitch2, byte pyaw2, float ilerp, float apitch, float ayaw)
{
	float	lightval1, lightval2, val;

	lightval1 = VertexLights_GetVertexLightValueMD3 (ppitch1, pyaw1, apitch, ayaw);
	lightval2 = VertexLights_GetVertexLightValueMD3 (ppitch2, pyaw2, apitch, ayaw);

	val = (lightval2 * ilerp) + (lightval1 * (1 - ilerp));

	return val;
}


float VertexLights_LerpVertexLight (const int index1, const int index2, const float ilerp, const float my_apitch, const float my_ayaw)
{
	float	lightval1, lightval2, val;

	lightval1 = VertexLights_GetVertexLightValue (index1, my_apitch, my_ayaw);
	lightval2 = VertexLights_GetVertexLightValue (index2, my_apitch, my_ayaw);

	val = (lightval2*ilerp) + (lightval1*(1-ilerp));

	return val;
}

static void sVertexLights_ResetAnormTable (void)
{
	int	i, j;
	float	forward, yaw, pitch, angle, sp, sy, cp, cy, precut;
	vec3_t	normal, lightvec;

	// Define the light vector here
	angle = DEG2RAD(vlight_pitch);
	sy = sinf(angle);
	cy = cosf(angle);
	angle = DEG2RAD(-vlight_yaw);
	sp = sinf(angle);
	cp = cosf(angle);
	lightvec[0] = cp*cy;
	lightvec[1] = cp*sy;
	lightvec[2] = -sp;

	// First thing that needs to be done is the conversion of the
	// anorm table into a pitch/yaw table

	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		if (r_avertexnormals[i][1] == 0 && r_avertexnormals[i][0] == 0)
		{
			yaw = 0;
			if (r_avertexnormals[i][2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = (int)(atan2(r_avertexnormals[i][1], r_avertexnormals[i][0]) * 57.295779513082320);
			if (yaw < 0)
				yaw += 360;

			forward = sqrtf(r_avertexnormals[i][0]*r_avertexnormals[i][0] + r_avertexnormals[i][1]*r_avertexnormals[i][1]);
			pitch = (int)(atan2(r_avertexnormals[i][2], forward) * 57.295779513082320);
			if (pitch < 0)
				pitch += 360;
		}
		anorm_pitch[i] = pitch * 256 / 360;
		anorm_yaw[i] = yaw * 256 / 360;
	}

	// Next, a light value table must be constructed for pitch/yaw offsets
	// DotProduct values

	// DotProduct values never go higher than 2, so store bytes as
	// (product * 127.5)

	for (i=0 ; i<256 ; i++)
	{
		angle = DEG2RAD(i * 360 / 256);
		sy = sinf(angle);
		cy = cosf(angle);
		for (j=0 ; j<256 ; j++)
		{
			angle = DEG2RAD(j * 360 / 256);
			sp = sinf(angle);
			cp = cosf(angle);

			normal[0] = cp*cy;
			normal[1] = cp*sy;
			normal[2] = -sp;

			precut = ((DotProduct(normal, lightvec) + 2) * 31.5);
			precut = (precut - (vlight_lowcut)) * 256 / (vlight_highcut - vlight_lowcut);
			if (precut > 255)
				precut = 255;
			if (precut < 0)
				precut = 0;
			vlighttable[i][j] = precut;
		}
	}
}

void VertexLights_CalcPitchYaw (const vec3_t origin, const vec3_t angles, const vec3_t my_vertexlight, const float radiusmax, float *set_pitch, float *set_yaw)
{

	// Radius max is a product of the for loop
	if (!radiusmax)
	{
		*set_pitch = 45;
		*set_yaw = 45;
	}
	else
	{
		vec3_t	dist, ang;
		VectorSubtract (my_vertexlight, origin, dist);
		vectoangles (dist, ang);
		*set_pitch = ang[0];
		*set_yaw = ang[1];
	}
}

void VertexLights_Init (void)
{
	sVertexLights_ResetAnormTable ();
}
