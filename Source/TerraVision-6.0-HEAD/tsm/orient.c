/*
**    orient.c: Code to convert a from/at/up cmaera model to the orientation
**              model used by VRML.
**    Copyright (C) 1995  Stephen Chenney (schenney@cs.berkeley.edu)
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program; if not, write to the Free Software
**    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
** Written by Stephen Chenney, October 1995.
** schenney@cs.berkeley.edu
*/

/*
** As is this program prompts for a from vector, an at vector and an up
** vector, then calculates the orientation required to align the default
** VRML camera with the given camera.
**
** As a standalone program it does fine.
** You are welcome to incorporate it into any other program you write, and
** to modify it as required. However, it would be good if you left it all
** in one file, and left the GNU header at the top. You probably shouldn't
** sell it either.
*/

#include <stdio.h>
#include <math.h>
#include "orient.h"

/* Takes the modulus of v */
#define VMod(v)		(sqrt((v).x*(v).x + (v).y*(v).y + (v).z*(v).z))

/* Returns the dot product of v1 & v2 */
#define VDot(v1, v2)	((v1).x*(v2).x + (v1).y*(v2).y + (v1).z*(v2).z)

/* Fills the fields of a vector.	*/
#define VNew(a, b, c, r)	((r).x = (a), (r).y = (b), (r).z = (c))

#define VAdd(v1, v2, r)		((r).x = (v1).x + (v2).x , \
							 (r).y = (v1).y + (v2).y , \
							 (r).z = (v1).z + (v2).z )

#define VSub(v1, v2, r)		((r).x = (v1).x - (v2).x , \
							 (r).y = (v1).y - (v2).y , \
							 (r).z = (v1).z - (v2).z )

#define VCross(v1, v2, r)	((r).x = (v1).y * (v2).z - (v1).z * (v2).y , \
				 			 (r).y = (v1).z * (v2).x - (v1).x * (v2).z , \
				 			 (r).z = (v1).x * (v2).y - (v1).y * (v2).x )

#define VScalarMul(v, d, r)	((r).x = (v).x * (d) , \
				 			 (r).y = (v).y * (d) , \
				 			 (r).z = (v).z * (d) )

#define VUnit(v, t, r)		((t) = 1 / VMod(v) , \
				 			 VScalarMul(v, t, r) )

typedef struct _Quaternion {
	Vector	vect_part;
	double	real_part;
	} Quaternion;

Quaternion
Build_Rotate_Quaternion(Vector axis, double cos_angle)
{
	Quaternion	quat;
	double	sin_half_angle;
	double	cos_half_angle;
	double	angle;

	/* The quaternion requires half angles. */
	if ( cos_angle > 1.0 ) cos_angle = 1.0;
	if ( cos_angle < -1.0 ) cos_angle = -1.0;
	angle = acos(cos_angle);
	sin_half_angle = sin(angle / 2);
	cos_half_angle = cos(angle / 2);

	VScalarMul(axis, sin_half_angle, quat.vect_part);
	quat.real_part = cos_half_angle;

	return quat;
}

static Quaternion
QQMul(Quaternion *q1, Quaternion *q2)
{
	Quaternion	res;
	Vector		temp_v;

	res.real_part = q1->real_part * q2->real_part -
					VDot(q1->vect_part, q2->vect_part);
	VCross(q1->vect_part, q2->vect_part, res.vect_part);
	VScalarMul(q1->vect_part, q2->real_part, temp_v);
	VAdd(temp_v, res.vect_part, res.vect_part);
	VScalarMul(q2->vect_part, q1->real_part, temp_v);
	VAdd(temp_v, res.vect_part, res.vect_part);

	return res;
}

static void
Quaternion_To_Axis_Angle(Quaternion *q, Vector *axis, double *angle)
{
	double	half_angle;
	double	sin_half_angle;

	half_angle = acos(q->real_part);
	sin_half_angle = sin(half_angle);
	*angle = half_angle * 2;
	if ( sin_half_angle < 1e-8 && sin_half_angle > -1e-8 )
		VNew(1, 0, 0, *axis);
	else
	{
		sin_half_angle = 1 / sin_half_angle;
		VScalarMul(q->vect_part, sin_half_angle, *axis);
	}
}


void
Convert_Camera_Model(Vector *pos, Vector *at, Vector *up, Vector *res_axis,
                     double *res_angle)
{
	Vector		n, v;

	Quaternion	rot_quat;
	Vector		norm_axis;
	Quaternion	norm_quat;
	Quaternion	inv_norm_quat;
	Quaternion	y_quat, new_y_quat, rot_y_quat;
	Vector		new_y;

	double		temp_d;
	Vector		temp_v;

	/* n = (norm)(pos - at) */
	VSub(*at, *pos, n);
	VUnit(n, temp_d, n);

	/* v = (norm)(view_up - (view_up.n)n) */
	VUnit(*up, temp_d, *up);
	temp_d = VDot(*up, n);
	VScalarMul(n, temp_d, temp_v);
	VSub(*up, temp_v, v);
	VUnit(v, temp_d, v);

	VNew(n.y, -n.x, 0, norm_axis);
	if ( VDot(norm_axis, norm_axis) < 1e-8 )
	{
		/* Already aligned, or maybe inverted. */
		if ( n.z > 0.0 )
		{
			norm_quat.real_part = 0.0;
			VNew(0, 1, 0, norm_quat.vect_part);
		}
		else
		{
			norm_quat.real_part = 1.0;
			VNew(0, 0, 0, norm_quat.vect_part);
		}
	}
	else
	{
		VUnit(norm_axis, temp_d, norm_axis);
		norm_quat = Build_Rotate_Quaternion(norm_axis, -n.z);
	}

	/* norm_quat now holds the rotation needed to line up the view directions.
	** We need to find the rotation to align the up vectors also.
	*/

	/* Need to rotate the world y vector to see where it ends up. */
	/* Find the inverse rotation. */
	inv_norm_quat.real_part = norm_quat.real_part;
	VScalarMul(norm_quat.vect_part, -1, inv_norm_quat.vect_part);

	/* Rotate the y. */
	y_quat.real_part = 0.0;
	VNew(0, 1, 0, y_quat.vect_part);
	new_y_quat = QQMul(&norm_quat, &y_quat);
	new_y_quat = QQMul(&new_y_quat, &inv_norm_quat);
	new_y = new_y_quat.vect_part;

	/* Now need to find out how much to rotate about n to line up y. */
	VCross(new_y, v, temp_v);
	if ( VDot(temp_v, temp_v) < 1.e-8 )
	{
		/* The old and new may be pointing in the same or opposite. Need
		** to generate a vector perpendicular to the old or new y.
		*/
		VNew(0, -v.z, v.y, temp_v);
		if ( VDot(temp_v, temp_v) < 1.e-8 )
			VNew(v.z, 0, -v.x, temp_v);
	}
	VUnit(temp_v, temp_d, temp_v);
	rot_y_quat = Build_Rotate_Quaternion(temp_v, VDot(new_y, v));

	/* rot_y_quat holds the rotation about the initial camera direction needed
	** to align the up vectors in the final position.
	*/

	/* Put the 2 rotations together. */
	rot_quat = QQMul(&rot_y_quat, &norm_quat);

	/* Extract the axis and angle from the quaternion. */
	Quaternion_To_Axis_Angle(&rot_quat, res_axis, res_angle);
}


void
/*main()*/
orient_main()
{
	Vector		pos, at, up;
	Vector		rot_axis;
	double		rot_angle;

	printf("Enter camera location: ");
	scanf("%lg %lg %lg", &(pos.x), &(pos.y), &(pos.z));
	printf("Enter look at point: ");
	scanf("%lg %lg %lg", &(at.x), &(at.y), &(at.z));
	printf("Enter look up vector: ");
	scanf("%lg %lg %lg", &(up.x), &(up.y), &(up.z));

	Convert_Camera_Model(&pos, &at, &up, &rot_axis, &rot_angle);

	/* Print it out. */
	printf("orientation %1.5g %1.5g %1.5g %1.5g\n",
			rot_axis.x, rot_axis.y, rot_axis.z, rot_angle);
}
