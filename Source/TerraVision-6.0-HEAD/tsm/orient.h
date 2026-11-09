#ifndef VECTOR_H
#define VECTOR_H

/* Define a vector.  */

typedef struct _Vector
	{
		double  x;
		double  y;
		double  z;
	} Vector, *VectorPtr;

/* Pass in position, look-at, and up vectors. */
/* Returns a rotation and angle quaternioin.  */

void Convert_Camera_Model( Vector *, Vector *, Vector *, Vector *, double * );


#endif
