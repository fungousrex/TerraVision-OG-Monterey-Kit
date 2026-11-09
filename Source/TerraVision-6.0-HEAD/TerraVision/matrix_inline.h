/* -*- tab-width: 2; -*-

  Name: matrix_inline.h
 
  Description: Miscellaneous matrix functions
 
  Revision History:
      $Id: matrix_inline.h,v 2.3.14.2 2002/10/31 05:26:44 reddy Exp $

  ----- BEGIN LICENSE BLOCK -----
  Version: MPL 1.1
 
  The contents of this file are subject to the Mozilla Public License Version
  1.1 (the "License"); you may not use this file except in compliance with
  the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  for the specific language governing rights and limitations under the
  License.
 
  The Original Code is the TerraVision 3D Terrain Visualization System.
 
  The Initial Developer of the Original Code is SRI International.

  Portions created by the Initial Developer are Copyright (C) 1992-2002
  the Initial Developer. All Rights Reserved.
 
  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
  ----- END LICENSE BLOCK -----
 
*/

exinline void
SubVector(float const a[3], float const b[3], float c[3])
{
  c[0] = a[0]-b[0];
  c[1] = a[1]-b[1];
  c[2] = a[2]-b[2];
}

exinline void
SubVectord(double const a[3], double const b[3], double c[3])
{
  c[0] = a[0]-b[0];
  c[1] = a[1]-b[1];
  c[2] = a[2]-b[2];
}

exinline void
AddVector(float const a[3], float const b[3], float c[3])
{
  c[0] = a[0]+b[0];
  c[1] = a[1]+b[1];
  c[2] = a[2]+b[2];
}

exinline void
AddVectord(double const a[3], double const b[3], double c[3])
{
  c[0] = a[0]+b[0];
  c[1] = a[1]+b[1];
  c[2] = a[2]+b[2];
}

exinline void
Scalard(double const a, double const b[3], double c[3])
{
  c[0] = a*b[0];
  c[1] = a*b[1];
  c[2] = a*b[2];
}

exinline void
Scalar(float const a, float const b[3], float c[3])
{
  c[0] = a*b[0];
  c[1] = a*b[1];
  c[2] = a*b[2];
}

exinline void
CopyVec(float *src, float *dst, int dims)
{  
  int i;
  for (i = 0; i < dims; i++)
    dst[i] = src[i];
}

exinline void
CopyVecd(double *src, double *dst, int dims)
{  
  int i;
  for (i = 0; i < dims; i++)
    dst[i] = src[i];
}

/*
 * Convert a double vector to a float vector
 */
exinline void  
ConvertDtoF(double *d, float *f, int dims)
{
  int i;
  for (i = 0; i < dims; i++)
    f[i] = (float)d[i];
}

/*
 * Convert a float vector to a double vector
 */
exinline void  
ConvertFtoD(float *f, double *d, int dims)
{
  int i;
  for (i = 0; i < dims; i++)
    d[i] = (double)f[i];
}

