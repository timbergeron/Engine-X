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
// r_part.c

#include "quakedef.h"


typedef enum
{
	pt_static, pt_grav, pt_slowgrav, pt_fire, pt_explode, pt_explode2, pt_blob, pt_blob2
} ptype_t;

typedef struct particle_s
{
	vec3_t	org;
	float	color;
	vec3_t	vel;
	float	ramp;
	float	die;
	ptype_t	type;
	struct particle_s *next;
} particle_t;


#define DEFAULT_NUM_PARTICLES	2048
#define ABSOLUTE_MIN_PARTICLES	512
#define ABSOLUTE_MAX_PARTICLES	8192

static	int	ramp1[8] = {0x6f, 0x6d, 0x6b, 0x69, 0x67, 0x65, 0x63, 0x61};
static	int	ramp2[8] = {0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x68, 0x66};
static	int	ramp3[8] = {0x6d, 0x6b, 6, 5, 4, 3};

static	particle_t	*particles, *active_particles, *free_particles;
static	int		r_numparticles;

vec3_t			r_pright, r_pup, r_ppn;


#ifdef GLQUAKE
static void Classic_LoadParticleTexures (const qbool bCircle, const int texturenum)
{
	int		i, x, y;
	unsigned int	data[32][32];

	mglPushStates ();
	GL_Bind (texturenum);

	mglFinishedStates ();
	// clear to transparent white 
	for (i=0 ; i<32*32 ; i++)
		((unsigned *)data)[i] = bCircle ? 0x00FFFFFF /* circle, alpha 0*/ : 0xFFFFFFFF /* solid is full alpha square*/;

	if (bCircle) // draw a circle in the top left corner
		for (x=0 ; x<16 ; x++)
		{
			for (y=0 ; y<16 ; y++)
			{
				if ((x - 7.5) * (x - 7.5) + (y - 7.5) * (y - 7.5) <= 8 * 8)
					data[y][x] = 0xFFFFFFFF;	// solid white
			}
		}
	

	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);		// Baker: //// uh?
	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);	// Nice.  GL_Upload32 will change.
	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Nice.  GL_Upload32 will change.
	GL_Upload32 ((unsigned *)data, 32, 32, bCircle ? (TEX_MIPMAP | TEX_ALPHA_TEST) : TEX_MIPMAP);

	mglPopStates ();
}
#endif

/*
===============
R_InitParticles
===============
*/
void Classic_InitParticles (void)
{
	int	i;

	if ((i = COM_CheckParm ("-particles")) && i+1 < com_argc)
	{
		r_numparticles = (int)(atoi(com_argv[i+1]));
		r_numparticles = CLAMP (ABSOLUTE_MIN_PARTICLES, r_numparticles, ABSOLUTE_MAX_PARTICLES);
	}
	else
	{
		r_numparticles = DEFAULT_NUM_PARTICLES;
	}

	particles = (particle_t *)Hunk_AllocName (r_numparticles, sizeof(particle_t), "classic:particles");

#ifdef GLQUAKE
	ImageWork_Start ("particles", "classic");
	Classic_LoadParticleTexures (true, classic_particletexture);
	Classic_LoadParticleTexures (false, classic_particletexture_blocky);
	ImageWork_Finish ();
#endif
}

/*
===============
R_EntityParticles
===============
*/

//#define NUMVERTEXNORMALS	162
extern	float	r_avertexnormals[NUMVERTEXNORMALS][3];
static vec3_t		avelocities[NUMVERTEXNORMALS];
static float		beamlength = 16;

void R_EntityParticles (entity_t *ent)
{
	int		i, count;
	particle_t	*p;
	float		angle, dist,sp, sy, cp, cy;
	vec3_t		forward;

	dist = 64;
	count = 50;

	if (!avelocities[0][0])
		for (i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = (rand() & 255) * 0.01;

	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		angle = cl.time * avelocities[i][0];
		sy = sinf(angle);
		cy = cosf(angle);
		angle = cl.time * avelocities[i][1];
		sp = sinf(angle);
		cp = cosf(angle);
//		angle = cl.time * avelocities[i][2];
//		sr = sinf(angle);
//		cr = cosf(angle);

		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 0.01;
		p->color = 0x6f;
		p->type = pt_explode;

		p->org[0] = ent->origin[0] + r_avertexnormals[i][0]*dist + forward[0]*beamlength;
		p->org[1] = ent->origin[1] + r_avertexnormals[i][1]*dist + forward[1]*beamlength;
		p->org[2] = ent->origin[2] + r_avertexnormals[i][2]*dist + forward[2]*beamlength;
	}
}

/*
===============
R_ClearParticles
===============
*/
void Classic_ClearParticles (void)
{
	int	i;

	free_particles = &particles[0];
	active_particles = NULL;

	for (i=0 ; i<r_numparticles ; i++)
		particles[i].next = &particles[i+1];
	particles[r_numparticles-1].next = NULL;
}


void R_ReadPointFile_f (void)
{
	FILE		*f;
	vec3_t		org;
	int		r, c;
	particle_t	*p;
	char		name[MAX_OSPATH];

	snprintf (name, sizeof(name), "maps/%s.pts", cl.worldname); // Baker: I do not think there is anything limiting us to server here.  Was sv.worldname

	QFS_FOpenFile (name, &f, NULL /*PATH LIMIT ME*/);
	if (!f)
	{
		Con_Printf ("couldn't open %s\n", name);
		return;
	}

	Con_Printf ("Reading %s...\n", name);
	c = 0;
	for ( ; ; )
	{
		r = fscanf (f,"%f %f %f\n", &org[0], &org[1], &org[2]);
		if (r != 3)
			break;
		c++;

		if (!free_particles)
		{
			Con_Printf ("Not enough free particles\n");
			break;
		}
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = 99999;
		p->color = (-c)&15;
		p->type = pt_static;
		VectorCopy (vec3_origin, p->vel);
		VectorCopy (org, p->org);
	}

	fclose (f);
	Con_Printf ("%i points read\n", c);
}

/*
===============
R_ParseParticleEffect

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect (void)
{
	int		i, count, color;
	vec3_t		org, dir;

	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();
	for (i=0 ; i<3 ; i++)
		dir[i] = MSG_ReadChar () * 0.0625;
	count = MSG_ReadByte ();
	color = MSG_ReadByte ();

// HACK: unfortunately the effect of the exploding barrel is quite poor
// in the original progs - no light, *sigh* - so I added some extras - joe
	if (count == 255)
	{
		dlight_t	*dl;

#ifdef GLQUAKE	// joe: nehahra has improved features for barrels
		if (nehahra)
			return;
#endif

		if (particle_explosiontype.integer == 3)
			R_RunParticleEffect (org, dir, 225, 50);
		else
			R_ParticleExplosion (org);

		// the missing light
		if (light_explosions.floater)
		{
			dl = Light_AllocDlight (0);
			VectorCopy (org, dl->origin);
			dl->radius = 150 + 200 * CLAMP (0, light_explosions.floater, 1);
			dl->die = cl.time + 0.5;
			dl->decay = 300;
#ifdef GLQUAKE
			dl->color_type = Light_SetDlightColor (light_explosions_color.floater, lt_explosion, true);
#endif
		}
	}
	else
	{
		R_RunParticleEffect (org, dir, color, count);
	}
}

/*
===============
R_ParticleExplosion
===============
*/
void Classic_ParticleExplosion (vec3_t org)
{
	int		i, j;
	particle_t	*p;

	if (particle_explosiontype.integer == 1)	// just a sprite
		return;

	for (i=0 ; i<1024 ; i++)
	{
		if (!free_particles)
			return;

		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 5;
		p->color = ramp1[0];
		p->ramp = rand() & 3;
		p->type = (i & 1) ? pt_explode : pt_explode2;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand() % 32) - 16);
			p->vel[j] = (rand() % 512) - 256;
		}
	}
}

/*
===============
R_ColorMappedExplosion		// joe: previously R_ParticleExplosion2
===============
*/
void Classic_ColorMappedExplosion (vec3_t org, int colorStart, int colorLength)
{
	int		i, j;
	particle_t	*p;
	int		colorMod = 0;

	if (particle_explosiontype.integer == 1)
		return;

	for (i=0 ; i<512 ; i++)
	{
		if (!free_particles)
			return;

		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 0.3;
		p->color = colorStart + (colorMod % colorLength);
		colorMod++;
		p->type = pt_blob;

		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand() % 32) - 16);
			p->vel[j] = (rand() % 512) - 256;
		}
	}
}

/*
===============
R_BlobExplosion
===============
*/
void Classic_BlobExplosion (vec3_t org)
{
	int		i, j;
	particle_t	*p;

	for (i=0 ; i<1024 ; i++)
	{
		if (!free_particles)
			return;

		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 1 + (rand() & 8) * 0.05;

		if (i & 1)
		{
			p->type = pt_blob;
			p->color = 66 + rand() % 6;
		}
		else
		{
			p->type = pt_blob2;
			p->color = 150 + rand() % 6;
		}

		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand() % 32) - 16);
			p->vel[j] = (rand() % 512) - 256;
		}
	}
}

/*
===============
R_RunParticleEffect
===============
*/
void Classic_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count)
{
	int		i, j;
	particle_t	*p;

	for (i=0 ; i<count ; i++)
	{
		if (!free_particles)
			return;

		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 0.1 * (rand() % 5);
		p->color = (color & ~7) + (rand() & 7);
		p->type = pt_grav;

		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand() & 15) - 8);
			p->vel[j] = dir[j] * 15;
		}
	}
}

/*
===============
R_LavaSplash
===============
*/
void Classic_LavaSplash (vec3_t org)
{
	int		i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	for (i=-16 ; i<16 ; i++)
	{
		for (j=-16 ; j<16 ; j++)
		{
			for (k=0 ; k<1 ; k++)
			{
				if (!free_particles)
					return;

				p = free_particles;
				free_particles = p->next;
				p->next = active_particles;
				active_particles = p;

				p->die = cl.time + 2 + (rand() & 31) * 0.02;
				p->color = 224 + (rand() & 7);
				p->type = pt_grav;

				dir[0] = j * 8 + (rand() & 7);
				dir[1] = i * 8 + (rand() & 7);
				dir[2] = 256;

				p->org[0] = org[0] + dir[0];
				p->org[1] = org[1] + dir[1];
				p->org[2] = org[2] + (rand() & 63);

				VectorNormalize (dir);
				vel = 50 + (rand() & 63);
				VectorScale (dir, vel, p->vel);
			}
		}
	}
}

/*
===============
R_TeleportSplash
===============
*/
void Classic_TeleportSplash (vec3_t org)
{
	int			i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	for (i=-16 ; i<16 ; i+=4)
	{
		for (j=-16 ; j<16 ; j+=4)
		{
			for (k=-24 ; k<32 ; k+=4)
			{
				if (!free_particles)
					return;

				p = free_particles;
				free_particles = p->next;
				p->next = active_particles;
				active_particles = p;

				p->die = cl.time + 0.2 + (rand() & 7) * 0.02;
				p->color = 7 + (rand() & 7);
				p->type = pt_grav;

				dir[0] = j * 8;
				dir[1] = i * 8;
				dir[2] = k * 8;

				p->org[0] = org[0] + i + (rand() & 3);
				p->org[1] = org[1] + j + (rand() & 3);
				p->org[2] = org[2] + k + (rand() & 3);

				VectorNormalize (dir);
				vel = 50 + (rand() & 63);
				VectorScale (dir, vel, p->vel);
			}
		}
	}
}

/*
===============
R_RocketTrail
===============
*/
void Classic_RocketTrail (vec3_t start, vec3_t end, vec3_t *trail_origin, trail_type_t type)
{
	vec3_t		point, delta, dir;
	float		len;
	int		i, j, num_particles;
	particle_t	*p;
	static	int	tracercount;

	VectorCopy (start, point);
	VectorSubtract (end, start, delta);
	if (!(len = VectorLength(delta)))
		goto done;
	VectorScale (delta, 1 / len, dir);	//unit vector in direction of trail

	switch (type)
	{
	case BLOOD_TRAIL:
		len /= 6;
		break;

	default:
		len /= 3;
		break;
	}

	if (!(num_particles = (int)len))
		goto done;

	VectorScale (delta, 1.0 / num_particles, delta);

	for (i=0 ; i < num_particles && free_particles ; i++)
	{
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		VectorClear (p->vel);
		p->die = cl.time + 2;

		if (cl_ent_disable_blood.integer && (type == BLOOD_TRAIL || type == SLIGHT_BLOOD_TRAIL))
			type = GRENADE_TRAIL ;

		switch (type)
		{
		case GRENADE_TRAIL:
			p->ramp = (particle_grenadetrail.integer == 2) ? (rand() & 3) : (rand() & 3) + 2;
			p->color = ramp3[(int)p->ramp];
			p->type = pt_fire;
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() % 6) - 3);
			break;

		case BLOOD_TRAIL:
			p->type = pt_grav;
			p->color = 67 + (rand() & 3);
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() % 6) - 3);
			break;

		case SLIGHT_BLOOD_TRAIL:
			p->type = pt_grav;
			p->color = 67 + (rand() & 3);
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() % 6) - 3);
			break;

		case TRACER1_TRAIL:
		case TRACER2_TRAIL:
			p->die = cl.time + 0.5;
			p->type = pt_static;
			p->color = (type == TRACER1_TRAIL) ? 52 + ((tracercount & 4) << 1) : 230 + ((tracercount & 4) << 1);
			tracercount++;

			VectorCopy (start, p->org);
			if (tracercount & 1)
			{
				p->vel[0] = 90 * dir[1];
				p->vel[1] = 90 * -dir[0];
			}
			else
			{
				p->vel[0] = 90 * -dir[1];
				p->vel[1] = 90 * dir[0];
			}
			break;

		case VOOR_TRAIL:
			p->color = 9*16 + 8 + (rand() & 3);
			p->type = pt_static;
			p->die = cl.time + 0.3;
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() & 15) - 8);
			break;

		case NEHAHRA_SMOKE:
			// nehahra smoke tracer
			p->color = 12 + (rand() & 3);
			p->type = pt_fire;
			p->die = cl.time + 1;
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() & 3) - 2);
			break;

		case ROCKET_TRAIL:
		case LAVA_TRAIL:
		default:
			p->ramp = (particle_rockettrail.integer == 2) ? (rand() & 3) + 2 : (rand() & 3);
			p->color = ramp3[(int)p->ramp];
			p->type = pt_fire;
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() % 6) - 3);
			break;
		}

		VectorAdd (point, delta, point);
	}

done:
	VectorCopy (point, *trail_origin);
}


/*
===============
CL_RunParticles -- johnfitz -- all the particle behavior, separated from R_DrawParticles
===============
*/
void CL_RunParticles (void)
{
	particle_t	*p, *kill;
	int		i;
	float  time1, time2, time3, dvel, frametime, grav;

	if (!active_particles)		return;

	frametime = fabs(cl.ctime - cl.oldtime);
	if (cl.paused)		// joe: pace from FuhQuake
		frametime = 0;
	time3 = frametime * 15;
	time2 = frametime * 10;
	time1 = frametime * 5;
	grav = frametime * sv_gravity.floater * 0.05;
	dvel = 4 * frametime;

	for ( ; ; )
	{
		kill = active_particles;
		if (kill && kill->die < cl.time)
		{
			active_particles = kill->next;
			kill->next = free_particles;
			free_particles = kill;
			continue;
		}
		break;
	}

	for (p = active_particles ; p ; p = p->next)
	{
		for ( ; ; )
		{
			kill = p->next;
			if (kill && kill->die < cl.time)
			{
				p->next = kill->next;
				kill->next = free_particles;
				free_particles = kill;
				continue;
			}
			break;
		}

		p->org[0] += p->vel[0] * frametime;
		p->org[1] += p->vel[1] * frametime;
		p->org[2] += p->vel[2] * frametime;

		switch (p->type)
		{
		case pt_static:
			break;

		case pt_fire:
			p->ramp += time1;
			if (p->ramp >= 6)
				p->die = -1;
			else
				p->color = ramp3[(int)p->ramp];
			p->vel[2] += grav;
			break;

		case pt_explode:
			p->ramp += time2;
			if (p->ramp >= 8)
				p->die = -1;
			else
				p->color = ramp1[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav * (qmb_explosions.integer ? 30: 1);	// QMB physics difference
			break;

		case pt_explode2:
			p->ramp += time3;
			if (p->ramp >= 8)
				p->die = -1;
			else
				p->color = ramp2[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] -= p->vel[i]*frametime;
			p->vel[2] -= grav * (qmb_explosions.integer ? 30: 1);	// QMB physics difference;
			break;

		case pt_blob:
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_blob2:
			for (i=0 ; i<2 ; i++)
				p->vel[i] -= p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_grav:
		case pt_slowgrav:
			p->vel[2] -= grav;
			break;
		}
	}
}

/*
===============
R_DrawParticles -- johnfitz -- moved all non-drawing code to CL_RunParticles
===============
*/
float r_partscale; // Really should be part of refdef
void R_DrawClassicParticles (void)
{
	particle_t		*p;
	float			scale;
	vec3_t			up, right, p_up, p_right, p_upright; //johnfitz -- p_ vectors
	byte			color[4]; //johnfitz -- particle transparency
//	float			alpha; //johnfitz -- particle transparency

	if (!r_drawparticles.integer)	return;
	if (!active_particles)			return;

	VectorScale (vup, 1.5, up);
	VectorScale (vright, 1.5, right);

	mglPushStates ();

	if (particle_blend.integer)
		GL_Bind (classic_particletexture_blocky);
	else
		GL_Bind (classic_particletexture);

	if (!particle_blend.integer)
		MeglEnable (GL_BLEND);

	MeglDepthMask (GL_FALSE);
	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	mglFinishedStates ();


	if (particle_blend.integer) //johnitz -- quads save fillrate
	{
		eglBegin (GL_QUADS);
		for (p=active_particles ; p ; p=p->next)
		{
			// hack a scale up to keep particles from disapearing
			scale = (p->org[0] - r_origin[0])*vpn[0] 
				  + (p->org[1] - r_origin[1])*vpn[1] 
				  + (p->org[2] - r_origin[2])*vpn[2];
			
			scale = 1 + scale * r_partscale;
			//johnfitz -- particle transparency and fade out
			*(int *)color = d_8to24table[(int)p->color];
			color[3] = 0; // No blending so no alpha  //(p->type == pt_fire) ? 255 * (6 - p->ramp) / 6 : 255;
			MeglColor3ubv(color);

			eglTexCoord2f (0,0);
			eglVertex3fv (p->org);

			eglTexCoord2f (0.5,0);
			VectorMultiplyAdd (p->org, scale, up, p_up);
			eglVertex3fv (p_up);

			eglTexCoord2f (0.5,0.5);
			VectorMultiplyAdd (p_up, scale, right, p_upright);
			eglVertex3fv (p_upright);

			eglTexCoord2f (0,0.5);
			VectorMultiplyAdd (p->org, scale, right, p_right);
			eglVertex3fv (p_right);

		}
		eglEnd ();
	}
	else //johnitz --  triangles save verts
	{
		eglBegin (GL_TRIANGLES);
		for (p=active_particles ; p ; p=p->next)
		{
			// hack a scale up to keep particles from disapearing

			// hack a scale up to keep particles from disapearing
			scale = (p->org[0] - r_origin[0])*vpn[0] + (p->org[1] - r_origin[1])*vpn[1] + (p->org[2] - r_origin[2])*vpn[2];
			scale = 1 + scale * r_partscale;

			*(int *)color = d_8to24table[(int)p->color];
			color[3] = (p->type == pt_fire) ? 255 * (6 - p->ramp) / 6 : 255; // Alpha

			MeglColor4ubv(color);

			eglTexCoord2f (0,0);
			eglVertex3fv (p->org);

			eglTexCoord2f (1,0);
			VectorMultiplyAdd (p->org, scale, up, p_up);
			eglVertex3fv (p_up);

			eglTexCoord2f (0,1);
			VectorMultiplyAdd (p->org, scale, right, p_right);
			eglVertex3fv (p_right);
		}
		eglEnd ();
	}

	if (!particle_blend.integer)
		MeglDisable (GL_BLEND);
	MeglDepthMask (GL_TRUE);
	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	MeglColor3ubv (color_white);

	mglPopStates ();
}

void Classic_DrawParticles (void)
{
//	CL_RunParticles ();		Baker: Moved to prescene
	R_DrawClassicParticles ();

}

void Particles_Init (void)
{
	Classic_InitParticles ();
#ifdef GLQUAKE
	QMB_InitParticles ();
#endif
}

void Particles_NewMap (void)
{
	Classic_ClearParticles ();
#ifdef GLQUAKE
	QMB_ClearParticles ();
#endif
}

void Particles_DrawParticles (void)
{
	Classic_DrawParticles ();
#ifdef GLQUAKE
	QMB_DrawParticles ();
#endif
}

void R_ColorMappedExplosion (vec3_t org, int colorStart, int colorLength)
{
#ifdef GLQUAKE
	if (qmb_initialized && qmb_explosions.integer)
		QMB_ColorMappedExplosion (org, colorStart, colorLength);
	else
#endif
		Classic_ColorMappedExplosion (org, colorStart, colorLength);
}

#define RunParticleEffect(var, org, dir, color, count)			\
	if (qmb_initialized && qmb_##var.integer)			\
		QMB_RunParticleEffect (org, dir, color, count);		\
	else								\
		Classic_RunParticleEffect (org, dir, color, count);

void R_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count)
{

	if (cl_ent_disable_blood.integer && (color == 73 || color == 225))
		color = 20;		// Switch to spark

	if (color == 73 || color == 225)
	{
		RunParticleEffect(blood, org, dir, color, count);
		return;
	}

	switch (count)
	{
	case 10:
	case 20:
	case 30:
		RunParticleEffect(spikes, org, dir, color, count);
		break;
	default:
		RunParticleEffect(gunshots, org, dir, color, count);
	}
}

void R_RocketTrail (vec3_t start, vec3_t end, vec3_t *trail_origin, trail_type_t type)
{
#ifdef GLQUAKE
	if (qmb_initialized && qmb_trails.integer)
		QMB_RocketTrail (start, end, trail_origin, type);
	else
#endif
		Classic_RocketTrail (start, end, trail_origin, type);
}

#ifdef GLQUAKE
#define ParticleFunction(var, name)			\
void R_##name (vec3_t org) {				\
	if (qmb_initialized && qmb_##var.integer)	\
		QMB_##name (org);			\
	else						\
		Classic_##name (org);			\
}
#else
#define ParticleFunction(var, name)	\
void R_##name (vec3_t org) {		\
	Classic_##name (org);		\
}
#endif


ParticleFunction(explosions,	 ParticleExplosion);
ParticleFunction(blobexplosions, BlobExplosion);
ParticleFunction(lavasplash,	 LavaSplash);
ParticleFunction(telesplash,	 TeleportSplash);

