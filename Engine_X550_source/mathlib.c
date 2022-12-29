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
// mathlib.c -- math primitives

#include "quakedef.h"

vec3_t	vec3_origin = {0, 0, 0};

int	_mathlib_temp_int1, _mathlib_temp_int2;
float	_mathlib_temp_float1, _mathlib_temp_float2;
vec3_t	_mathlib_temp_vec1;

void ProjectPointOnPlane (vec3_t dst, const vec3_t p, const vec3_t normal)
{
	float	d;
	vec3_t	n;
	float 	inv_denom;

	inv_denom = 1.0F / DotProduct(normal, normal);

	d = DotProduct(normal, p) * inv_denom;

	VectorScale (normal, inv_denom, n);
	VectorMultiplyAdd (p, -d, n, dst);
}

void VectorExtendLimits (const vec3_t newvalue, vec3_t minlimit, vec3_t maxlimit)
{
	int i;

	for (i=0; i<3; i++)
	{
		if (newvalue[i] < minlimit[i])	minlimit[i] = newvalue[i];
		if (newvalue[i] > maxlimit[i])	maxlimit[i] = newvalue[i];
	}

}

// assumes "src" is normalized
void PerpendicularVector (vec3_t dst, const vec3_t src)
{
	if (!src[0])
	{
		VectorSet (dst, 1, 0, 0);
	}
	else if (!src[1])
	{
		VectorSet (dst, 0, 1, 0);
	}
	else if (!src[2])
	{
		VectorSet (dst, 0, 0, 1);
	}
	else
	{
		VectorSet (dst, -src[1], src[0], 0);
		VectorNormalizeFast (dst);
	}
}

void VectorVectors (const vec3_t forward, vec3_t right, vec3_t up)
{
	PerpendicularVector (right, forward);
	CrossProduct (right, forward, up);
}

void LerpVector (const vec3_t from, const vec3_t to, const float frac, vec3_t out)
{
	out[0] = from[0] + frac * (to[0] - from[0]);
	out[1] = from[1] + frac * (to[1] - from[1]);
	out[2] = from[2] + frac * (to[2] - from[2]);
}
void RotatePointAroundVector (vec3_t dst, const vec3_t dir, const vec3_t point, const float degrees)
{
	float	t0, t1, angle, c, s;
	vec3_t	vr, vu, vf;

	angle = DEG2RAD(degrees);
	c = cosf(angle);
	s = sinf(angle);
	VectorCopy (dir, vf);
	VectorVectors (vf, vr, vu);

	t0 = vr[0] * c + vu[0] * -s;
	t1 = vr[0] * s + vu[0] * c;
	dst[0] = (t0 * vr[0] + t1 * vu[0] + vf[0] * vf[0]) * point[0]
		+ (t0 * vr[1] + t1 * vu[1] + vf[0] * vf[1]) * point[1]
		+ (t0 * vr[2] + t1 * vu[2] + vf[0] * vf[2]) * point[2];

	t0 = vr[1] * c + vu[1] * -s;
	t1 = vr[1] * s + vu[1] * c;
	dst[1] = (t0 * vr[0] + t1 * vu[0] + vf[1] * vf[0]) * point[0]
		+ (t0 * vr[1] + t1 * vu[1] + vf[1] * vf[1]) * point[1]
		+ (t0 * vr[2] + t1 * vu[2] + vf[1] * vf[2]) * point[2];

	t0 = vr[2] * c + vu[2] * -s;
	t1 = vr[2] * s + vu[2] * c;
	dst[2] = (t0 * vr[0] + t1 * vu[0] + vf[2] * vf[0]) * point[0]
		+ (t0 * vr[1] + t1 * vu[1] + vf[2] * vf[1]) * point[1]
		+ (t0 * vr[2] + t1 * vu[2] + vf[2] * vf[2]) * point[2];
}

#if 0
/*
==================
BOPS_Error

Split out like this for ASM to call.
==================
*/
void BOPS_Error (void)
{
	Sys_Error ("BoxOnPlaneSide: Bad signbits");
}
#endif

//#if !id386

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide (const vec3_t emins, const vec3_t emaxs, const mplane_t *p)
{
	switch (p->signbits)
	{
	default:
	case 0:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) >= p->dist) |
				(((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) < p->dist) << 1));

	case 1:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) >= p->dist) |
				(((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) < p->dist) << 1));

	case 2:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) >= p->dist) |
				(((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) < p->dist) << 1));

	case 3:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) >= p->dist) |
				(((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) < p->dist) << 1));

	case 4:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) >= p->dist) |
				(((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));

	case 5:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) >= p->dist) |
				(((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));

	case 6:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) >= p->dist) |
				(((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));

	case 7:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) >= p->dist) |
				(((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));
	}
}

//#endif

void vectoangles (const vec3_t vec, vec3_t ang)
{
	float	forward, yaw, pitch;

	if (!vec[1] && !vec[0])
	{
		yaw = 0;
		pitch = (vec[2] > 0) ? 90 : 270;
	}
	else
	{
		yaw = atan2 (vec[1], vec[0]) * 180 / M_PI;
		if (yaw < 0)
			yaw += 360;

		forward = sqrtf (vec[0]*vec[0] + vec[1]*vec[1]);
		pitch = atan2f (vec[2], forward) * 180 / M_PI;
		if (pitch < 0)
			pitch += 360;
	}

	ang[0] = pitch;
	ang[1] = yaw;
	ang[2] = 0;
}

void AngleVectors (const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float	angle, sr, sp, sy, cr, cp, cy, temp;

	if (angles[YAW])
	{
		angle = DEG2RAD(angles[YAW]);
		sy = sinf(angle);
		cy = cosf(angle);
	}
	else
	{
		sy = 0;
		cy = 1;
	}

	if (angles[PITCH])
	{
		angle = DEG2RAD(angles[PITCH]);
		sp = sinf(angle);
		cp = cosf(angle);
	}
	else
	{
		sp = 0;
		cp = 1;
	}

	if (forward)
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if (right || up)
	{
		if (angles[ROLL])
		{
			angle = DEG2RAD(angles[ROLL]);
			sr = sinf(angle);
			cr = cosf(angle);

			if (right)
			{
				temp = sr * sp;
				right[0] = -1 * temp * cy + cr * sy;
				right[1] = -1 * temp * sy - cr * cy;
				right[2] = -1 * sr * cp;
			}

			if (up)
			{
				temp = cr * sp;
				up[0] = (temp * cy + sr * sy);
				up[1] = (temp * sy - sr * cy);
				up[2] = cr * cp;
			}
		}
		else
		{
			if (right)
			{
				right[0] = sy;
				right[1] = -cy;
				right[2] = 0;
			}

			if (up)
			{
				up[0] = sp * cy ;
				up[1] = sp * sy;
				up[2] = cp;
			}
		}
	}
}

vec_t VectorLength (const vec3_t v)
{
	float	length;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	return sqrtf(length);
}

vec_t VectorsLength2 (const vec3_t v1, const vec3_t v2)
{
	vec3_t	v3;
	float	length;

	VectorSubtract (v1, v2, v3);

	length = v3[0] * v3[0] + v3[1] * v3[1] + v3[2] * v3[2];
	return sqrtf(length);
}



float VectorNormalize (vec3_t v)
{
	float	length;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrtf(length);

	if (length)
		VectorScale (v, 1 / length, v);

	return length;
}


/*
================
R_ConcatRotations
================
*/
void R_ConcatRotations (const float in1[3][3], const float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
}


/*
================
R_ConcatTransforms
================
*/
void R_ConcatTransforms (const float in1[3][4], const float in2[3][4],  float out[3][4])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3];
}

/*
===================
FloorDivMod

Returns mathematically correct (floor-based) quotient and remainder for
numer and denom, both of which should contain no fractional part. The
quotient must fit in 32 bits.
====================
*/

void FloorDivMod (const double numer, const double denom, int *quotient, int *rem)
{
	int	q, r;
	double	x;

#ifndef PARANOID
	if (denom <= 0.0)
		Sys_Error ("FloorDivMod: bad denominator %d\n", denom);

//	if ((floor(numer) != numer) || (floor(denom) != denom))
//		Sys_Error ("FloorDivMod: non-integer numer or denom %f %f\n",
//				numer, denom);
#endif

	if (numer >= 0.0)
	{

		x = floor(numer / denom);
		q = (int)x;
		r = (int)floorf(numer - (x * denom));
	}
	else
	{
	// perform operations with positive values, and fix mod to make floor-based
		x = floor(-numer / denom);
		q = -(int)x;
		r = (int)floorf(-numer - (x * denom));
		if (r != 0)
		{
			q--;
			r = (int)denom - r;
		}
	}

	*quotient = q;
	*rem = r;
}


/*
===================
GreatestCommonDivisor
====================
*/
int GreatestCommonDivisor (const int i1, const int i2)
{
	if (i1 > i2)
	{
		if (i2 == 0)
			return (i1);
		return GreatestCommonDivisor (i2, i1 % i2);
	}
	else
	{
		if (i1 == 0)
			return (i2);
		return GreatestCommonDivisor (i1, i2 % i1);
	}
}


int ParseFloats(char *s, float *f, int *f_size)
{
	int i, argc;

	if (!s || !f || !f_size)
		Sys_Error("ParseFloats() wrong params");

	if (f_size[0] <= 0)
		return (f_size[0] = 0); // array have no size, unusual but no crime

	Cmd_TokenizeString(s);
	argc = Cmd_Argc();
	argc = min(Cmd_Argc(), f_size[0]);

	for(i = 0; i < argc; i++)
		f[i] = atof(Cmd_Argv(i));

	for( ; i < f_size[0]; i++)
		f[i] = 0; // zeroing unused elements

	return (f_size[0] = argc);
}


#if 0

//transform 4d vector by a 4d matrix.
void Matrix4_Transform4(float *matrix, float *vector, float *product)
{
	product[0] = matrix[0]*vector[0] + matrix[4]*vector[1] + matrix[8]*vector[2] + matrix[12]*vector[3];
	product[1] = matrix[1]*vector[0] + matrix[5]*vector[1] + matrix[9]*vector[2] + matrix[13]*vector[3];
	product[2] = matrix[2]*vector[0] + matrix[6]*vector[1] + matrix[10]*vector[2] + matrix[14]*vector[3];
	product[3] = matrix[3]*vector[0] + matrix[7]*vector[1] + matrix[11]*vector[2] + matrix[15]*vector[3];
}

//be aware that this generates two sorts of matricies depending on order of a+b
void Matrix4_Multiply(float *a, float *b, float *out)
{
	out[0]  = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
	out[1]  = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
	out[2]  = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
	out[3]  = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

	out[4]  = a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
	out[5]  = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
	out[6]  = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
	out[7]  = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];

	out[8]  = a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
	out[9]  = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
	out[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
	out[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];

	out[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
	out[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
	out[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
	out[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
}


//This function is GL stylie (use as 2nd arg to ML_MultMatrix4).
float *Matrix4_NewRotation(float a, float x, float y, float z)
{
	static float ret[16];
	float c = cosf(a* M_PI / 180.0);
	float s = sinf(a* M_PI / 180.0);

	ret[0] = x*x*(1-c)+c;
	ret[4] = x*y*(1-c)-z*s;
	ret[8] = x*z*(1-c)+y*s;
	ret[12] = 0;

	ret[1] = y*x*(1-c)+z*s;
    ret[5] = y*y*(1-c)+c;
	ret[9] = y*z*(1-c)-x*s;
	ret[13] = 0;

	ret[2] = x*z*(1-c)-y*s;
	ret[6] = y*z*(1-c)+x*s;
	ret[10] = z*z*(1-c)+c;
	ret[14] = 0;

	ret[3] = 0;
	ret[7] = 0;
	ret[11] = 0;
	ret[15] = 1;
	return ret;
}

//This function is GL stylie (use as 2nd arg to ML_MultMatrix4).
float *Matrix4_NewTranslation(float x, float y, float z)
{
	static float ret[16];
	ret[0] = 1;
	ret[4] = 0;
	ret[8] = 0;
	ret[12] = x;

	ret[1] = 0;
    ret[5] = 1;
	ret[9] = 0;
	ret[13] = y;

	ret[2] = 0;
	ret[6] = 0;
	ret[10] = 1;
	ret[14] = z;

	ret[3] = 0;
	ret[7] = 0;
	ret[11] = 0;
	ret[15] = 1;
	return ret;
}

void ML_ModelViewMatrix(float *modelview, vec3_t viewangles, vec3_t vieworg)
{
	float tempmat[16];
	//load identity.
	memset(modelview, 0, sizeof(*modelview)*16);
//#if FULLYGL
	modelview[0] = 1;
	modelview[5] = 1;
	modelview[10] = 1;
	modelview[15] = 1;

	Matrix4_Multiply(modelview, Matrix4_NewRotation(-90,  1, 0, 0), tempmat);	    // put Z going up
	Matrix4_Multiply(tempmat, Matrix4_NewRotation(90,  0, 0, 1), modelview);	    // put Z going up
/*#else
	//use this lame wierd and crazy identity matrix..
	modelview[2] = -1;
	modelview[4] = -1;
	modelview[9] = 1;
	modelview[15] = 1;
#endif*/
	//figure out the current modelview matrix

	//I would if some of these, but then I'd still need a couple of copys
	Matrix4_Multiply(modelview, Matrix4_NewRotation(-viewangles[2],  1, 0, 0), tempmat);
	Matrix4_Multiply(tempmat, Matrix4_NewRotation(-viewangles[0],  0, 1, 0), modelview);
	Matrix4_Multiply(modelview, Matrix4_NewRotation(-viewangles[1],  0, 0, 1), tempmat);

	Matrix4_Multiply(tempmat, Matrix4_NewTranslation(-vieworg[0],  -vieworg[1],  -vieworg[2]), modelview);	    // put Z going up
}

void ML_ProjectionMatrix(float *proj, float wdivh, float fovy)
{
	float xmin, xmax, ymin, ymax;
	float nudge = 1;

	//proj
	ymax = 4 * tanf( fovy * M_PI / 360.0 );
	ymin = -ymax;

	xmin = ymin * wdivh;
	xmax = ymax * wdivh;

	proj[0] = (2*4) / (xmax - xmin);
	proj[4] = 0;
	proj[8] = (xmax + xmin) / (xmax - xmin);
	proj[12] = 0;

	proj[1] = 0;
	proj[5] = (2*4) / (ymax - ymin);
	proj[9] = (ymax + ymin) / (ymax - ymin);
	proj[13] = 0;

	proj[2] = 0;
	proj[6] = 0;
	proj[10] = -1  * nudge;
	proj[14] = -2*4 * nudge;

	proj[3] = 0;
	proj[7] = 0;
	proj[11] = -1;
	proj[15] = 0;
}

//returns fractions of screen.
//uses GL style rotations and translations and stuff.
//3d -> screen (fixme: offscreen return values needed)
void ML_Project (vec3_t in, vec3_t out, vec3_t viewangles, vec3_t vieworg, float wdivh, float fovy)
{
	float modelview[16];
	float proj[16];

	ML_ModelViewMatrix(modelview, viewangles, vieworg);
	ML_ProjectionMatrix(proj, wdivh, fovy);

	{
		float v[4], tempv[4];
		v[0] = in[0];
		v[1] = in[1];
		v[2] = in[2];
		v[3] = 1;

		Matrix4_Transform4(modelview, v, tempv);
		Matrix4_Transform4(proj, tempv, v);

		v[0] /= v[3];
		v[1] /= v[3];
		v[2] /= v[3];

		out[0] = (1+v[0])/2;
		out[1] = (1+v[1])/2;
		out[2] = (1+v[2])/2;
	}
}
#endif

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds (const vec3_t mins, const vec3_t maxs)
{
	int		i;
	vec3_t		corner;

	for (i=0 ; i<3 ; i++)
		corner[i] = fabsf(mins[i]) > fabsf(maxs[i]) ? fabsf(mins[i]) : fabsf(maxs[i]);

	return VectorLength(corner);
}

int IntegerMaxExtend (const int newvalue, int *MaxHolder) // Assumes greater than 0 numbers
{
	if (newvalue <= *MaxHolder)
		return 0;

	*MaxHolder = newvalue;

	return *MaxHolder;

}
