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
// mathlib.h

#ifndef __MATHLIB_H__
#define __MATHLIB_H__


typedef	float		vec_t;
typedef	vec_t		vec3_t[3];
typedef vec_t		vec_rgb_t[3];
typedef vec_t		vec_rgba_t[4];
extern	vec3_t		vec3_origin;

typedef	vec_t		vec5_t[5];

typedef	int			fixed4_t;
typedef	int			fixed8_t;
typedef	int			fixed16_t;

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define D_PI M_PI*2

struct mplane_s;

#define M_PI_DIV_180						(M_PI / 180.0) //johnfitz
#define DEG2RAD( a )						( (a) * M_PI_DIV_180 )

#define NANMASK								(255 << 23)
#define	IS_NAN(x)							(((*(int *) & x) & NANMASK) == NANMASK)

#define Q_rint(x)							((x) > 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))
#define CLAMP(min, x, max)					((x) < (min) ? (min) : (x) > (max) ? (max) : (x)) //johnfitz

#define DotProduct(x, y)					((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2])
#define VectorSubtract(a, b, c)				((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1], (c)[2] = (a)[2] - (b)[2])
#define VectorAdd(a, b, c)					((c)[0] = (a)[0] + (b)[0], (c)[1] = (a)[1] + (b)[1], (c)[2] = (a)[2] + (b)[2])
#define VectorAddFloat(a, b, c)				((c)[0] = (a)[0] + (b), (c)[1] = (a)[1] + (b), (c)[2] = (a)[2] + (b))
#define VectorCopy(a, b)					((b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2])
#define VectorClear(a)						((a)[0] = (a)[1] = (a)[2] = 0)
#define VectorNegate(a, b)					((b)[0] = -(a)[0], (b)[1] = -(a)[1], (b)[2] = -(a)[2])
#define VectorSet(v, x, y, z)				((v)[0] = (x), (v)[1] = (y), (v)[2] = (z))

#define VectorSetColor4f(v, r, g, b, a)		((v)[0] = (r), (v)[1] = (g), (v)[2] = (b), (v)[3] = (a))

#define ShortenAngle(s1)					if (s1 < -180) s1 += 360; else if (s1 >= 180) s1 -= 360	// No trailing semi-colon so usage can look "normal"

#define VectorShortenAngles(v)																					\
									if ((v)[0] < -180)															\
										(v)[0] += 360;															\
									else if ((v)[0] >= 180)														\
										(v)[0] -= 360;															\
									if ((v)[1] < -180)															\
										(v)[1] += 360;															\
									else if ((v)[1] >= 180)														\
										(v)[1] -= 360;															\
									if ((v)[2] < -180)															\
										(v)[2] += 360;															\
									else if ((v)[2] >= 180)														\
										(v)[2] -= 360;															

#if _MSC_VER <=1200 // MSVC6 ONLY
#define sqrtf(x)	sqrt(x)
#define cosf(x)		cos(x)
#define sinf(x)		sin(x)
#define atanf(x)	atan(x)
#define tanf(x)		tan(x)			// One justified tan instance
#define floorf(x)	floor(x)		// A few justified floor instances
#define ceilf(x)	ceil(x)		// Those that remain relate to time which is a double
#define fabsf(x)	fabs(x)
#define powf(x,y)	pow(x,y)
#define atan2f(x,y) atan2(x,y)
#endif

//void LerpVector (const vec3_t from, const vec3_t to, float frac, vec3_t out);
void LerpVector				(const vec3_t from, const vec3_t to, const float frac, vec3_t out);
void VectorExtendLimits		(const vec3_t newvalue, vec3_t minlimit, vec3_t maxlimit);

int ParseFloats				(char *s, float *f, int *f_size);

#define CrossProduct(v1, v2, x)					\
	((x)[0] = (v1)[1] * (v2)[2] - (v1)[2] * (v2)[1],	\
	(x)[1] = (v1)[2] * (v2)[0] - (v1)[0] * (v2)[2],		\
	(x)[2] = (v1)[0] * (v2)[1] - (v1)[1] * (v2)[0])

#define VectorSupCompare(v, w, m)								\
	(_mathlib_temp_float1 = m,								\
	(v)[0] - (w)[0] > -_mathlib_temp_float1 && (v)[0] - (w)[0] < _mathlib_temp_float1 &&	\
	(v)[1] - (w)[1] > -_mathlib_temp_float1 && (v)[1] - (w)[1] < _mathlib_temp_float1 &&	\
	(v)[2] - (w)[2] > -_mathlib_temp_float1 && (v)[2] - (w)[2] < _mathlib_temp_float1)

#define VectorL2Compare(v, w, m)				\
	(_mathlib_temp_float1 = (m) * (m),			\
	_mathlib_temp_vec1[0] = (v)[0] - (w)[0], _mathlib_temp_vec1[1] = (v)[1] - (w)[1], _mathlib_temp_vec1[2] = (v)[2] - (w)[2],\
	_mathlib_temp_vec1[0] * _mathlib_temp_vec1[0] +		\
	_mathlib_temp_vec1[1] * _mathlib_temp_vec1[1] +		\
	_mathlib_temp_vec1[2] * _mathlib_temp_vec1[2] < _mathlib_temp_float1)

#define VectorCompare(v, w)	((v)[0] == (w)[0] && (v)[1] == (w)[1] && (v)[2] == (w)[2])

#define VectorMultiplyAdd(a, _f, b, c)					\
do {								\
	_mathlib_temp_float1 = (_f);				\
	(c)[0] = (a)[0] + _mathlib_temp_float1 * (b)[0];	\
	(c)[1] = (a)[1] + _mathlib_temp_float1 * (b)[1];	\
	(c)[2] = (a)[2] + _mathlib_temp_float1 * (b)[2];	\
} while(0)

#define VectorMultiplyAddV(a, d, b, c)	\
do {									\
	(c)[0] = (a)[0] + (d)[0] * (b)[0];	\
	(c)[1] = (a)[1] + (d)[1] * (b)[1];	\
	(c)[2] = (a)[2] + (d)[2] * (b)[2];	\
} while(0)
	

#define VectorScale(in, _scale, out)					\
do {									\
	float	scale = (_scale);					\
	(out)[0] = (in)[0] * (scale); (out)[1] = (in)[1] * (scale); (out)[2] = (in)[2] * (scale);\
} while(0)

#define anglemod(a)	((360.0 / 65536) * ((int)((a) * (65536 / 360.0)) & 65535))

#define VectorNormalizeFast(_v)							\
do {										\
	_mathlib_temp_float1 = DotProduct((_v), (_v));				\
	if (_mathlib_temp_float1) {						\
		_mathlib_temp_float2 = 0.5f * _mathlib_temp_float1;		\
		_mathlib_temp_int1 = *((int *)&_mathlib_temp_float1);		\
		_mathlib_temp_int1 = 0x5f375a86 - (_mathlib_temp_int1 >> 1);	\
		_mathlib_temp_float1 = *((float *)&_mathlib_temp_int1);	\
		_mathlib_temp_float1 = _mathlib_temp_float1 * (1.5f - _mathlib_temp_float2 * _mathlib_temp_float1 * _mathlib_temp_float1);\
		VectorScale((_v), _mathlib_temp_float1, (_v));			\
	}									\
} while(0)

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)			\
	(((p)->type < 3)?					\
	(							\
		((p)->dist <= (emins)[(p)->type])?		\
			1					\
		:						\
		(						\
			((p)->dist >= (emaxs)[(p)->type])?	\
				2				\
			:					\
				3				\
		)						\
	)							\
	:							\
		BoxOnPlaneSide ((emins), (emaxs), (p)))

#define PlaneDist(point, plane) ((plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal))
#define PlaneDiff(point, plane) (((plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal)) - (plane)->dist)

void PerpendicularVector (vec3_t dst, const vec3_t src);
void VectorVectors (const vec3_t forward, vec3_t right, vec3_t up);

vec_t VectorLength (const vec3_t v);
vec_t VectorsLength2 (const vec3_t v1, const vec3_t v2);

#define FloatInterpolate(f1, _frac, f2)			\
	(_mathlib_temp_float1 = _frac,				\
	(f1) + _mathlib_temp_float1 * ((f2) - (f1)))



float VectorNormalize (vec3_t v);		// returns vector length

void R_ConcatRotations (const float in1[3][3], const float in2[3][3], float out[3][3]);
void R_ConcatTransforms (const float in1[3][4], const float in2[3][4],  float out[3][4]);




void FloorDivMod (const double numer, const double denom, int *quotient, int *rem);
int GreatestCommonDivisor (const int i1, const int i2);


//void vectoangles (vec3_t vec, vec3_t ang);
void vectoangles (const vec3_t vec, vec3_t ang);
void AngleVectors (const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);

int BoxOnPlaneSide (const vec3_t emins, const vec3_t emaxs, const struct mplane_s *p);


void RotatePointAroundVector (vec3_t dst, const vec3_t dir, const vec3_t point, const float degrees);


#ifdef MACOSX
typedef struct
{
	int left;
	int right;
	int bottom;
	int top;
} RECT;
#endif

extern	int		_mathlib_temp_int1,  _mathlib_temp_int2;
extern	float	_mathlib_temp_float1, _mathlib_temp_float2;
extern	vec3_t	_mathlib_temp_vec1;


float RadiusFromBounds (const vec3_t mins, const vec3_t maxs);

int IntegerMaxExtend (const int newvalue, int *MaxHolder);


#endif // __MATHLIB_H__

