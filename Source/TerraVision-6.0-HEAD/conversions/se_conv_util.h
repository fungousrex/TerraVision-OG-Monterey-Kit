/*
 * FILE       : se_conv_util.h
 *
 * PROGRAMMERS: Michele L. Worley (SAIC), Howard Lu (SAIC)
 *
 * DESCRIPTION:
 *   Conversions utilities.
 *
 * Release 3.0.3 - April 20, 2001
 */
/*
 *                             NOTICE
 * 
 * This software is provided openly and freely for use in representing and
 * interchanging environmental data & databases.
 * 
 * This software was developed for use by the United States Government with
 * unlimited rights.  The software was developed under contract
 * N61339-96-C-0104 by Science Applications International Corporation.
 * The software is unclassified and is deemed as Distribution A, approved
 * for Public Release.
 * 
 * Use by others is permitted only upon the ACCEPTANCE OF THE TERMS AND
 * CONDITIONS, AS STIPULATED UNDER THE FOLLOWING PROVISIONS:
 * 
 *    1. Recipient may make unlimited copies of this software and give
 *       copies to other persons or entities as long as the copies contain
 *       this NOTICE, and as long as the same copyright notices that
 *       appear on, or in, this software remain.
 * 
 *    2. Trademarks. All trademarks belong to their respective trademark
 *       holders.  Third-Party applications/software/information are
 *       copyrighted by their respective owners.
 * 
 *    3. Recipient agrees to forfeit all intellectual property and
 *       ownership rights for any version created from the modification
 *       or adaptation of this software, including versions created from
 *       the translation and/or reverse engineering of the software design.
 * 
 *    4. Transfer.  Recipient may not sell, rent, lease, or sublicense
 *       this software or any derivative.  Recipient may, however,
 *       enable another person or entity the rights to use this software,
 *       provided that this AGREEMENT and NOTICE is furnished along with
 *       the software and/or software system utilizing this software.
 * 
 *       All derivatives, revisions, modifications, created by the
 *       Recipient, to this software and/or related technical data shall
 *       be forwarded by the Recipient to the Government at the following
 *       address:
 * 
 *         Engineering Directorate
 *         Attention SEDRIS COR
 *         STRICOM
 *         12350 Research Parkway
 *         Orlando, FL 32826
 * 
 *         or via electronic mail to:  se-mgmt@sedris.org
 * 
 *    5. No Warranty. This software is being delivered to you AS IS
 *       and there is no warranty, EXPRESS or IMPLIED, as to its use
 *       or performance.
 * 
 *       The RECIPIENT ASSUMES ALL RISKS, KNOWN AND UNKNOWN, OF USING
 *       THIS SOFTWARE.  The DEVELOPER EXPRESSLY DISCLAIMS, and the
 *       RECIPIENT WAIVES, ANY and ALL PERFORMANCE OR RESULTS YOU MAY
 *       OBTAIN BY USING THIS SOFTWARE OR DOCUMENTATION.  THERE IS
 *       NO WARRANTY, EXPRESS OR, IMPLIED, AS TO NON-INFRINGEMENT OF
 *       THIRD PARTY RIGHTS, MERCHANTABILITY, OR FITNESS FOR ANY
 *       PARTICULAR PURPOSE.  IN NO EVENT WILL THE DEVELOPER, THE
 *       UNITED STATES GOVERNMENT OR ANYONE ELSE ASSOCIATED WITH THE
 *       DEVELOPMENT OF THIS SOFTWARE BE HELD LIABLE FOR ANY CONSEQUENTIAL,
 *       INCIDENTAL OR SPECIAL DAMAGES, INCLUDING ANY LOST PROFITS
 *       OR LOST SAVINGS WHATSOEVER.
 */

/*
 * COPYRIGHT 1998, SCIENCE APPLICATIONS INTERNATIONAL CORPORATION.
 *                 ALL RIGHTS RESERVED.
 * 
 * THIS SOFTWARE IS PROTOTYPE.  IT IS PROVIDED ON AN "AS IS" BASIS.  DEFECTS
 * AND ANOMALIES ARE PRESUMED TO EXIST.  THE SOFTWARE IS PROVIDED FREE OF
 * CHARGE AT THE REQUEST OF THE UNITED STATES GOVERNMENT.  THE RECIPIENT
 * ASSUMES ALL RISKS, KNOWN AND UNKNOWN, OF USING THE SOFTWARE.  SAIC EXPRESSLY
 * DISCLAIMS, AND THE RECIPIENT WAIVES, ANY AND ALL WARRANTIES, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE, AS WELL AS ANY WARRANTIES REGARDING TITLE
 * OR INFRINGEMENT OF THE INTELLECTUAL PROPERTY RIGHTS OF THIRD PARTIES.  THE
 * RECIPIENT, BY ACCEPTING AND USING THE ENCLOSED SOFTWARE, AGREES TO HOLD
 * HARMLESS SAIC FROM ANY LOSS, COSTS, OR DAMAGES INCURRED BY THE RECIPIENT,
 * ITS AFFILIATES, PARENTS, OR SUBSIDIARIES, AND THEIR RESPECTIVE EMPLOYEES,
 * OFFICERS AND AGENTS, ARISING OUT OF OR RELATING TO THEIR USE OF THE
 * SOFTWARE.
 * 
 * THIS WORK WAS PERFORMED UNDER U.S. GOVERNMENT CONTRACT NUMBER
 * N61339-96-C-0104.
 */

#ifndef _SRM_UTIL_H_INCLUDED
#define _SRM_UTIL_H_INCLUDED

/*
 * Allow C++ compilers to use this API without any problems
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "se_stds.h"
#include "se_conv_types.h"

/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_CoordEqual
 *
 *   Determines whether two coordinates are equal.
 *
 * PARAMETERS:
 *
 *   coord1_ptr -- pointer to the first coordinate to be compared.
 *
 *   coord2_ptr -- pointer to the second coordinate to be compared.
 *
 * RETURNS:
 *
 *   SE_TRUE if the coordinates are non-NULL, are in the same spatial
 *      reference frame, and are equal
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_CoordEqual
(
    const SE_COORD *coord1_ptr,
    const SE_COORD *coord2_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_Coord2DEqual
 *
 *   Determines whether two 2D coordinates are equal.
 *
 * PARAMETERS:
 *
 *   coord1_ptr -- pointer to the first coordinate to be compared.
 *
 *   coord2_ptr -- pointer to the second coordinate to be compared.
 *
 * RETURNS:
 *
 *   SE_TRUE if the coordinates are non-NULL, are in the same spatial
 *      reference frame, and are equal
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_Coord2DEqual
(
    const SE_COORD_2D *coord1_ptr,
    const SE_COORD_2D *coord2_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_Coord3DEqual
 *
 *   Determines whether two 3D coordinates are equal.
 *
 * PARAMETERS:
 *
 *   coord1_ptr -- pointer to the first coordinate to be compared.
 *
 *   coord2_ptr -- pointer to the second coordinate to be compared.
 *
 * RETURNS:
 *
 *   SE_TRUE if the coordinates are non-NULL, are in the same spatial
 *      reference frame, and are equal
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_Coord3DEqual
(
    const SE_COORD_3D *coord1_ptr,
    const SE_COORD_3D *coord2_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_DeriveCoord2DFrom3D
 *
 *   Given a 3D coordinate, expressed in an SRF that has a corresponding 2D
 *   SRF, constructs the corresponding 2D representation (i.e., without the
 *   z/elevation portion of the 3D, in the proper 2D SRF).
 *
 * PARAMETERS:
 * 
 *   coord_3d_ptr -- pointer to the 3D coordinate for which we are obtaining
 *     the corresponding 2D coordinate.
 *
 *   coord_2d_ptr -- pointer to the 2D coordinate that we are deriving.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in and all operations
 *     succeeded.
 *
 *   SE_COORD_INVALID_DEST_COORD - and *coord_2d_ptr is left unchanged, if
 *     coord_2d_ptr is NULL.
 *
 *   SE_COORD_INVALID_SOURCE_COORD - and *coord_2d_ptr is left unchanged,
 *     if coord_3d_ptr is NULL, or is expressed in a 3D SRF that has no
 *     corresponding 2D SRF.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SRM_DeriveCoord2DFrom3D
(
    const SE_COORD_3D *coord_3d_ptr,
          SE_COORD_2D *coord_2d_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_DeriveCoord3DFrom2DForGivenElevation
 *
 *   Given a 2D coordinate, expressed in an SRF that has a corresponding 3D
 *   SRF, and given a z/elevation, constructs the corresponding 3D
 *   representation of the given coordinate.
 *
 * PARAMETERS:
 *
 *   coord_2d_ptr -- pointer to the 2D coordinate for which we are obtaining a
 *     corresponding 3D coordinate.
 *
 *   elevation -- given z/elevation
 *
 *   coord_3d_ptr -- pointer to the 3D coordinate that we are deriving.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in and all operations
 *     succeeded.
 *
 *   SE_COORD_INVALID_DEST_COORD - and *coord_3d_ptr is left unaltered, if
 *     coord_3d_ptr is NULL.
 *
 *   SE_COORD_INVALID_SOURCE_COORD - and *coord_3d_ptr is left unaltered,
 *     if coord_2d_ptr is NULL or is expressed in a 2D SRF that has no
 *     corresponding 3D SRF.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SRM_DeriveCoord3DFrom2DForGivenElevation
(
    const SE_COORD_2D *coord_2d_ptr,
          SE_FLOAT64   elevation,
          SE_COORD_3D *coord_3d_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_SRFParametersEqual
 *
 *   Determines whether two sets of spatial reference frame (SRF) parameters
 *   are equal.
 *
 * PARAMETERS:
 *
 *   srf1_ptr -- pointer to the first set of SRF parameters to be compared.
 *
 *   srf2_ptr -- pointer to the second set of SRF parameters to be compared.
 *
 * RETURNS:
 *
 *   SE_TRUE if the SRF parameters are non-NULL, are in the same SRF, and are
 *           equal
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_SRFParametersEqual
(
    const SE_SRF_PARAMETERS *srf1_ptr,
    const SE_SRF_PARAMETERS *srf2_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_SRF2DParametersEqual
 *
 *   Determines whether two sets of 2D spatial reference frame (SRF) parameters
 *   are equal.
 *
 * PARAMETERS:
 *
 *   srf1_ptr -- pointer to the first set of SRF parameters to be compared.
 *
 *   srf2_ptr -- pointer to the second set of SRF parameters to be compared.
 *
 * RETURNS:
 *
 *   SE_TRUE if the SRF parameters are non-NULL, are in the same SRF, and are
 *           equal
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_SRF2DParametersEqual
(
    const SRM_SRF_2D_PARAMETERS *srf1_ptr,
    const SRM_SRF_2D_PARAMETERS *srf2_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_SRF3DParametersEqual
 *
 *   Determines whether two sets of 3D spatial reference frame (SRF) parameters
 *   are equal.
 *
 * PARAMETERS:
 *
 *   srf1_ptr -- pointer to the first set of SRF parameters to be compared.
 *
 *   srf2_ptr -- pointer to the second set of SRF parameters to be compared.
 *
 * RETURNS:
 *
 *   SE_TRUE if the SRF parameters are non-NULL, are in the same SRF, and are
 *           equal
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_SRF3DParametersEqual
(
    const SRM_SRF_3D_PARAMETERS *srf1_ptr,
    const SRM_SRF_3D_PARAMETERS *srf2_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_GetUnits
 *
 *   Given a set of 3D spatial reference frame parameters, returns the
 *   z/elevation units.
 *
 * PARAMETERS:
 *
 *   params_ptr -- IN parameters; pointer to spatial reference frame
 *     parameters for which z/elevation units are being retrieved.
 *
 * RETURNS:
 *
 *   SRM_METERS if params_ptr does not have elevation or z units, or if
 *      the spatial reference frame can't be identified.
 *   elevation or z units for params ptr, otherwise.
 *
 *-----------------------------------------------------------------------------
 */
extern SRM_ELEVATION_UNITS_ENUM
SE_GetUnits
(
    const SRM_SRF_3D_PARAMETERS *params_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_GetCorresponding2DSRF
 *
 *   Given a 3D spatial reference frame (SRF), retrieves the corresponding
 *   2D SRF, if there is one.
 *
 * PARAMETERS:
 *
 *   srf_3D -- 3D SRF for which we want the corresponding 2D SRF
 *
 *   srf_2D_ptr -- pointer to a variable, provided by the user, into which
 *     the corresponding 2D SRF's enumerator is copied, if there is one.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS if a valid parameter was passed in and it has a 2D
 *     equivalent.
 *
 *   SE_COORD_FAILURE otherwise.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SRM_GetCorresponding2DSRF
(
    SRM_SRF_3D_ENUM  srf_3D,
    SRM_SRF_2D_ENUM *srf_2D_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_GetCorresponding3DSRF
 *
 *   Given a 2D spatial reference frame (SRF), retrieves the corresponding
 *   3D SRF, if there is one.
 *
 * PARAMETERS:
 *
 *   srf_2D -- 2D SRF for which we want the corresponding 3D SRF
 *
 *   srf_3D_ptr -- pointer to a variable, provided by the user, into which
 *     the corresponding 3D SRF's enumerator is copied, if there is one.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS if a valid parameter was passed in and it has a 2D
 *     equivalent.
 *
 *   SE_COORD_FAILURE otherwise.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SRM_GetCorresponding3DSRF
(
    SRM_SRF_2D_ENUM  srf_2D,
    SRM_SRF_3D_ENUM *srf_3D_ptr
);


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix3X3Add
 *
 *   Adds 2 SE_MATRIX_3X3_TYPE matrices.
 *
 * PARAMETERS:
 *
 *   a_ptr -- pointer to 1st operand.
 *
 *   b_ptr -- pointer to 2nd operand.
 *
 *   result_out_ptr -- pointer to result.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in and all operations
 *     succeeded.
 *
 *   SE_COORD_INVALID_SOURCE_MATRIX - and *result_out_ptr is left unchanged, if
 *     a_ptr or b_ptr was NULL.
 *
 *   SE_COORD_INVALID_DEST_MATRIX - and *result_out_ptr is left unchanged, if
 *     result_out_ptr was NULL.
 *
 *----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_Matrix3X3Add
(
    const SE_MATRIX_3X3_TYPE *a_ptr,
    const SE_MATRIX_3X3_TYPE *b_ptr,
          SE_MATRIX_3X3_TYPE *result_out_ptr
);


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix3X3Multiply
 *
 *   Multiplies 2 SE_MATRIX_3X3_TYPE matrices.
 *
 * PARAMETERS:
 *
 *   a_ptr -- pointer to 1st operand.
 *
 *   b_ptr -- pointer to 2nd operand.
 *
 *   result_out_ptr -- pointer to result.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in and all operations
 *     succeeded.
 *
 *   SE_COORD_INVALID_SOURCE_MATRIX - if a_ptr or b_ptr was NULL.
 *
 *   SE_COORD_INVALID_DEST_MATRIX - if result_out_ptr was NULL.
 *
 *----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_Matrix3X3Multiply
(
    const SE_MATRIX_3X3_TYPE *a_ptr,
    const SE_MATRIX_3X3_TYPE *b_ptr,
          SE_MATRIX_3X3_TYPE *result_out_ptr
);


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix3X3Sub
 *
 *   Subtracts the matrix in *b_ptr from the matrix in *a_ptr.
 *
 * PARAMETERS:
 *
 *   a_ptr -- pointer to 1st operand.
 *
 *   b_ptr -- pointer to 2nd operand.
 *
 *   result_out_ptr -- pointer to result.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in and all operations
 *     succeeded.
 *
 *   SE_COORD_INVALID_SOURCE_MATRIX - if a_ptr or b_ptr was NULL.
 *
 *   SE_COORD_INVALID_DEST_MATRIX - if result_out_ptr was NULL.
 *
 *----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_Matrix3X3Sub
(
    const SE_MATRIX_3X3_TYPE *a_ptr,
    const SE_MATRIX_3X3_TYPE *b_ptr,
          SE_MATRIX_3X3_TYPE *result_out_ptr
);


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix4X4Add
 *
 *   Adds 2 SE_MATRIX_4X4_TYPE matrices.
 *
 * PARAMETERS:
 *
 *   a_ptr -- pointer to 1st operand.
 *
 *   b_ptr -- pointer to 2nd operand.
 *
 *   result_out_ptr -- pointer to result.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in and all operations
 *     succeeded.
 *
 *   SE_COORD_INVALID_SOURCE_MATRIX - if a_ptr or b_ptr was NULL.
 *
 *   SE_COORD_INVALID_DEST_MATRIX - if result_out_ptr was NULL.
 *
 *----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_Matrix4X4Add
(
    const SE_MATRIX_4X4_TYPE *a_ptr,
    const SE_MATRIX_4X4_TYPE *b_ptr,
          SE_MATRIX_4X4_TYPE *result_out_ptr
);


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix4X4Multiply
 *
 *   Multiplies 2 SE_MATRIX_4X4_TYPE matrices.
 *
 * PARAMETERS:
 *
 *   a_ptr -- pointer to 1st operand.
 *
 *   b_ptr -- pointer to 2nd operand.
 *
 *   result_out_ptr -- pointer to result.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in and all operations
 *     succeeded.
 *
 *   SE_COORD_INVALID_SOURCE_MATRIX - if a_ptr or b_ptr was NULL.
 *
 *   SE_COORD_INVALID_DEST_MATRIX - if result_out_ptr was NULL.
 *
 *----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_Matrix4X4Multiply
(
    const SE_MATRIX_4X4_TYPE *a_ptr,
    const SE_MATRIX_4X4_TYPE *b_ptr,
          SE_MATRIX_4X4_TYPE *result_out_ptr
);


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix4X4Sub
 *
 *   Subtracts the matrix in *b_ptr from the matrix in *a_ptr.
 *
 * PARAMETERS:
 *
 *   a_ptr -- pointer to 1st operand.
 *
 *   b_ptr -- pointer to 2nd operand.
 *
 *   result_out_ptr -- pointer to result.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in and all operations
 *     succeeded.
 *
 *   SE_COORD_INVALID_SOURCE_MATRIX - if a_ptr or b_ptr was NULL.
 *
 *   SE_COORD_INVALID_DEST_MATRIX - if result_out_ptr was NULL.
 *
 *----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_Matrix4X4Sub
(
    const SE_MATRIX_4X4_TYPE *a_ptr,
    const SE_MATRIX_4X4_TYPE *b_ptr,
          SE_MATRIX_4X4_TYPE *result_out_ptr
);


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_CrossProduct
 *
 *   Computes the cross product of 2 vectors.
 *
 * PARAMETERS:
 *
 *   a_ptr -- 1st vector operand.
 *
 *   b_ptr -- 2nd vector operand.
 *
 *   result_out_ptr -- pointer to an SE_VECTOR_3_TYPE variable in the user's
 *     memory space where the result will be stored.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in.
 *
 *   SE_COORD_INVALID_SOURCE_VECTOR - if a_ptr or b_ptr is NULL.
 *
 *   SE_COORD_INVALID_DEST_VECTOR - if result_out_ptr is NULL.
 *
 *----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_CrossProduct
(
    const SE_VECTOR_3_TYPE *a_ptr,
    const SE_VECTOR_3_TYPE *b_ptr,
          SE_VECTOR_3_TYPE *result_out_ptr
);


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_DotProduct
 *
 *   Computes the dot product of 2 vectors.
 *
 * PARAMETERS:
 *
 *   a_ptr -- 1st vector operand.
 *
 *   b_ptr -- 2nd vector operand.
 *
 *   result_out_ptr -- pointer to an SE_FLOAT64 variable in the user's memory
 *     space where the result will be stored.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in.
 *
 *   SE_COORD_INVALID_SOURCE_VECTOR - if a_ptr or b_ptr is NULL.
 *
 *   SE_COORD_FAILURE - if result_out_ptr is NULL.
 *
 *----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_DotProduct
(
    const SE_VECTOR_3_TYPE *a_ptr,
    const SE_VECTOR_3_TYPE *b_ptr,
          SE_FLOAT64       *result_out_ptr
);


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_VectorMagnitude
 *
 *   Computes the magnitude of a vector.
 *
 * PARAMETERS:
 *
 *   vec_ptr -- pointer to the vector operand.
 *
 *   result_out_ptr -- pointer to an SE_FLOAT64 variable in the user's memory
 *     space where the result will be stored.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in.
 *
 *   SE_COORD_INVALID_SOURCE_VECTOR - if vec_ptr is NULL.
 *
 *   SE_COORD_FAILURE - if result_out_ptr is NULL.
 *
 *----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_VectorMagnitude
(
    const SE_VECTOR_3_TYPE *vec_ptr,
          SE_FLOAT64       *result_out_ptr
);


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_VectorMultByScalar
 *
 *   Multiplies vec_ptr's vector by scalar, returns the result in
 *   *result_out_ptr.
 *
 * PARAMETERS:
 *
 *   vec_ptr -- pointer to the vector operand.
 *
 *   scalar -- the scalar multiplier.
 *
 *   result_out_ptr -- pointer to an SE_VECTOR_3_TYPE variable in the user's
 *     memory space where the result will be stored.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in.
 *
 *   SE_COORD_INVALID_SOURCE_VECTOR - if vec_ptr is NULL.
 *
 *   SE_COORD_INVALID_DEST_VECTOR - if result_out_ptr is NULL.
 *
 *----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_VectorMultByScalar
(
    const SE_VECTOR_3_TYPE *vec_ptr,
          SE_FLOAT64        scalar,
          SE_VECTOR_3_TYPE *result_out_ptr
);
#ifdef __cplusplus
}
#endif
#endif
