/*
 * SEDRIS Conversions API
 *
 * FILE       : se_conv_valid.h
 *
 * PROGRAMMERS: Michele L. Worley (SAIC)
 *
 * DESCRIPTION:
 *   This file is for internal use only. SEDRIS users should not include
 *   this file, but instead include se_conv.h.
 *
 *   This file defines the prototypes for the functions that validate the
 *   data structures used by the Conversions API; it is included by
 *   se_conv.h
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


#ifndef _SE_CONV_VALID_H_INCLUDED
#define _SE_CONV_VALID_H_INCLUDED

/*
 * Allow C++ compilers to use this API without any problems
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "se_conv_types.h"


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_SetErrorFilePtr
 *
 *   Sets the FILE pointer to which error messages are directed,
 *   if error messages are requested.
 *
 *   WARNING: If you close this file after calling this function,
 *            remember that the functions of se_conv.c will still
 *            try to use it.
 *
 * PARAMETERS:
 *
 *   err_file_ptr -- stream to which error messages are to be output
 *
 * RETURNS:
 *
 *   SE_COORD_FAILURE if err_file_ptr is NULL; in which case nothing is
 *      changed
 *   SE_COORD_SUCCESS otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_SetErrorFilePtr
(
    FILE *err_file_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_GetErrorFilePtr
 *
 *   Returns the FILE pointer to which error messages are directed.
 *
 * PARAMETERS:
 *
 *   nothing
 *
 * RETURNS:
 *
 *   FILE pointer to stream to which error messages are to be output
 *
 *-----------------------------------------------------------------------------
 */
extern FILE *
SE_GetErrorFilePtr(void);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidCMYData
 *
 *   Checks the information in the given SE_CMY_DATA; all values must be
 *   between 0.0 and 1.0, inclusive.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_CMY_DATA whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SE_CMY_DATA is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidCMYData
(
    const SE_CMY_DATA *value_ptr,
          SE_BOOLEAN   print_error,
          SE_UINT32   *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidCMYKData
 *
 *   Checks the information in the given SE_CMYK_DATA; all values must be
 *   between 0.0 and 1.0, inclusive.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_CMYK_DATA whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SE_CMYK_DATA is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidCMYKData
(
    const SE_CMYK_DATA  *value_ptr,
          SE_BOOLEAN    print_error,
          SE_UINT32    *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidColorData
 *
 *   Checks the information in the given SE_COLOR_DATA.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_COLOR_DATA whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SE_COLOR_DATA is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidColorData
(
    const SE_COLOR_DATA *value_ptr,
          SE_BOOLEAN     print_error,
          SE_UINT32     *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidColorModel
 *
 *   Checks if the given value is a valid SE_COLOR_MODEL_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidColorModel
(
    SE_COLOR_MODEL_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidColorStatusCode
 *
 *   Checks if the given value is a valid SE_COLOR_STATUS_CODE_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidColorStatusCode
(
    SE_COLOR_STATUS_CODE_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidCoord
 *
 *   Checks the information in the given SE_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_COORD whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SE_COORD is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidCoord
(
    const SE_COORD *value_ptr,
          SE_BOOLEAN  print_error,
          SE_UINT32 *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidCoord2D
 *
 *   Checks the information in the given SE_COORD_2D.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_COORD_2D whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SE_COORD_2D is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidCoord2D
(
    const SE_COORD_2D *value_ptr,
          SE_BOOLEAN   print_error,
          SE_UINT32   *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidCoord3D
 *
 *   Checks the information in the given SE_COORD_3D.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_COORD_3D whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SE_COORD_3D is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidCoord3D
(
    const SE_COORD_3D *value_ptr,
          SE_BOOLEAN   print_error,
          SE_UINT32   *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidCoordStatusCode
 *
 *   Checks if the given value is a valid SE_COORD_STATUS_CODE_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidCoordStatusCode
(
    SE_COORD_STATUS_CODE_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidDirectionOfForward
 *
 *   Checks if the given value is a valid SE_DIRECTION_OF_FORWARD_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidDirectionOfForward
(
    SE_DIRECTION_OF_FORWARD_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidDirectionOfUp
 *
 *   Checks if the given value is a valid SE_DIRECTION_OF_UP_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidDirectionOfUp
(
    SE_DIRECTION_OF_UP_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidEC2DParameters
 *
 *   Checks the information in the given SRM_EC_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_EC_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_EC_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidEC2DParameters
(
    const SRM_EC_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN            print_error,
          SE_UINT32            *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidEC3DParameters
 *
 *   Checks the information in the given SRM_EC_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_EC_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_EC_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidEC3DParameters
(
    const SRM_EC_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN            print_error,
          SE_UINT32            *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidElevationUnits
 *
 *   Checks if the given value is a valid SRM_ELEVATION_UNITS_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidElevationUnits
(
    SRM_ELEVATION_UNITS_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidGCS3DCoord
 *
 *   Checks the information in the given SRM_GCS_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GCS_3D_COORD whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_GCS_3D_COORD is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidGCS3DCoord
(
    const SRM_GCS_3D_COORD *value_ptr,
          SE_BOOLEAN        print_error,
          SE_UINT32        *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidGD2DCoord
 *
 *   Checks the information in the given SRM_GD_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GD_2D_COORD whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_GD_2D_COORD is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidGD2DCoord
(
    const SRM_GD_2D_COORD *value_ptr,
          SE_BOOLEAN       print_error,
          SE_UINT32       *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidGD2DParameters
 *
 *   Checks the information in the given SRM_GD_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GD_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_GD_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidGD2DParameters
(
    const SRM_GD_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN            print_error,
          SE_UINT32            *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidGD3DCoord
 *
 *   Checks the information in the given SRM_GD_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GD_3D_COORD whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_GD_3D_COORD is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidGD3DCoord
(
    const SRM_GD_3D_COORD *value_ptr,
          SE_BOOLEAN       print_error,
          SE_UINT32       *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidGD3DParameters
 *
 *   Checks the information in the given SRM_GD_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GD_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_GD_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidGD3DParameters
(
    const SRM_GD_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN            print_error,
          SE_UINT32            *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidGEI3DCoord
 *
 *   Checks the information in the given SRM_GEI_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GEI_3D_COORD whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_GEI_3D_COORD is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidGEI3DCoord
(
    const SRM_GEI_3D_COORD *value_ptr,
          SE_BOOLEAN        print_error,
          SE_UINT32        *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidGEI3DParameters
 *
 *   Checks the information in the given SRM_GEI_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GEI_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_GEI_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidGEI3DParameters
(
    const SRM_GEI_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidGEIEpoch
 *
 *   Checks if the given value is a valid SE_GEI_EPOCH_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidGEIEpoch
(
    SE_GEI_EPOCH_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidGM3DCoord
 *
 *   Checks the information in the given SRM_GM_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GM_3D_COORD whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_GM_3D_COORD is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidGM3DCoord
(
    const SRM_GM_3D_COORD *value_ptr,
          SE_BOOLEAN       print_error,
          SE_UINT32       *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidGM3DParameters
 *
 *   Checks the information in the given SRM_GM_3D_PARAMETERS. For the
 *   parameters to be valid, gm_epoch must be between 1945 and 2000.
 *
 * PARAMETERS:
 *
 *   value -- pointer to SRM_GM_3D_PARAMETERS whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_GM_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidGM3DParameters
(
    const SRM_GM_3D_PARAMETERS *value,
          SE_BOOLEAN            print_error,
          SE_UINT32            *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidGSE3DCoord
 *
 *   Checks the information in the given SRM_GSE_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GSE_3D_COORD whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_GSE_3D_COORD is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidGSE3DCoord
(
    const SRM_GSE_3D_COORD *value_ptr,
          SE_BOOLEAN        print_error,
          SE_UINT32        *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidGSM3DCoord
 *
 *   Checks the information in the given SRM_GSM_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GSM_3D_COORD whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_GSM_3D_COORD is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidGSM3DCoord
(
    const SRM_GSM_3D_COORD *value_ptr,
          SE_BOOLEAN        print_error,
          SE_UINT32        *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidHemisphere
 *
 *   Checks if the given value is a valid SE_HEMISPHERE_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidHemisphere
(
    SE_HEMISPHERE_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidHLSData
 *
 *   Checks the information in the given SE_HLS_DATA.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_HLS_DATA whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SE_HLS_DATA is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidHLSData
(
    const SE_HLS_DATA *value_ptr,
          SE_BOOLEAN   print_error,
          SE_UINT32   *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidHorizontalDatum
 *
 *   Checks if the given value is a valid SE_HORIZONTAL_DATUM_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidHorizontalDatum
(
    SE_HORIZONTAL_DATUM_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidHSVData
 *
 *   Checks the information in the given SE_HSV_DATA; all values must be between
 *   0.0 and 1.0, inclusive.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_HSV_DATA whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SE_HSV_DATA is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidHSVData
(
    const SE_HSV_DATA  *value_ptr,
          SE_BOOLEAN    print_error,
          SE_UINT32    *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidLCC2DParameters
 *
 *   Checks the information in the given SRM_LCC_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LCC_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_LCC_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidLCC2DParameters
(
    const SRM_LCC_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidLCC3DParameters
 *
 *   Checks the information in the given SRM_LCC_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LCC_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_LCC_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidLCC3DParameters
(
    const SRM_LCC_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidLSR2DParameters
 *
 *   Checks the information in the given SRM_LSR_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LSR_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_LSR_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidLSR2DParameters
(
    const SRM_LSR_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidLSR3DParameters
 *
 *   Checks the information in the given SRM_LSR_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LSR_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_LSR_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidLSR3DParameters
(
    const SRM_LSR_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidLTP2DParameters
 *
 *   Checks the information in the given SRM_LTP_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LTP_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_LTP_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidLTP2DParameters
(
    const SRM_LTP_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidLTP3DParameters
 *
 *   Checks the information in the given SRM_LTP_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LTP_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_LTP_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidLTP3DParameters
(
    const SRM_LTP_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidM2DParameters
 *
 *   Checks the information in the given SRM_M_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_M_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_M_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidM2DParameters
(
    const SRM_M_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN           print_error,
          SE_UINT32           *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidM3DParameters
 *
 *   Checks the information in the given SRM_M_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_M_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_M_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidM3DParameters
(
    const SRM_M_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN           print_error,
          SE_UINT32           *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidOM2DParameters
 *
 *   Checks the information in the given SRM_OM_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_OM_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_OM_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidOM2DParameters
(
    const SRM_OM_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN            print_error,
          SE_UINT32            *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidOM3DParameters
 *
 *   Checks the information in the given SRM_OM_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_OM_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_OM_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidOM3DParameters
(
    const SRM_OM_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN            print_error,
          SE_UINT32            *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidPint16
 *
 *   Checks that the given SE_PINT16 contains a valid value.
 *
 * PARAMETERS:
 *
 *   value -- SE_PINT16 whose contents are to be processed
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidPint16
(
    SE_PINT16 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidPint32
 *
 *   Checks that the given SE_PINT32 contains a valid value.
 *
 * PARAMETERS:
 *
 *   value -- SE_PINT32 whose contents are to be processed
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidPint32
(
    SE_PINT32 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidPint8
 *
 *   Checks that the given SE_PINT8 contains a valid value.
 *
 * PARAMETERS:
 *
 *   value -- SE_PINT8 whose contents are to be processed
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidPint8
(
    SE_PINT8 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidPS2DParameters
 *
 *   Checks the information in the given SRM_PS_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_PS_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_PS_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidPS2DParameters
(
    const SRM_PS_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN            print_error,
          SE_UINT32            *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidPS3DParameters
 *
 *   Checks the information in the given SRM_PS_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_PS_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_PS_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidPS3DParameters
(
    const SRM_PS_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN            print_error,
          SE_UINT32            *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidPSPole
 *
 *   Checks if the given value is a valid SE_PS_POLE_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidPSPole
(
    SE_PS_POLE_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidRGBData
 *
 *   Checks the information in the given SE_RGB_DATA; all values must be
 *   between 0.0 and 1.0, inclusive.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_RGB_DATA whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SE_RGB_DATA is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidRGBData
(
    const SE_RGB_DATA  *value_ptr,
          SE_BOOLEAN    print_error,
          SE_UINT32    *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidSM3DCoord
 *
 *   Checks the information in the given SRM_SM_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_SM_3D_COORD whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_SM_3D_COORD is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidSM3DCoord
(
    const SRM_SM_3D_COORD *value_ptr,
          SE_BOOLEAN       print_error,
          SE_UINT32       *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidSRF2D
 *
 *   Checks if the given value is a valid SRM_SRF_2D_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidSRF2D
(
    SRM_SRF_2D_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidSRF2DParameters
 *
 *   Checks the information in the given SRM_SRF_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_SRF_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_SRF_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidSRF2DParameters
(
    const SRM_SRF_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidSRF3D
 *
 *   Checks if the given value is a valid SRM_SRF_3D_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidSRF3D
(
    SRM_SRF_3D_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidSRF3DParameters
 *
 *   Checks the information in the given SRM_SRF_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_SRF_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_SRF_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidSRF3DParameters
(
    const SRM_SRF_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidSRFParameters
 *
 *   Checks the information in the given SE_SRF_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_SRF_PARAMETERS whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SE_SRF_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidSRFParameters
(
    const SE_SRF_PARAMETERS *value_ptr,
          SE_BOOLEAN         print_error,
          SE_UINT32         *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidTM2DParameters
 *
 *   Checks the information in the given SRM_TM_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_TM_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_TM_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidTM2DParameters
(
    const SRM_TM_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN            print_error,
          SE_UINT32            *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidTM3DParameters
 *
 *   Checks the information in the given SRM_TM_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_TM_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_TM_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidTM3DParameters
(
    const SRM_TM_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN            print_error,
          SE_UINT32            *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidUPS2DParameters
 *
 *   Checks the information in the given SRM_UPS_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UPS_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_UPS_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidUPS2DParameters
(
    const SRM_UPS_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidUPS3DParameters
 *
 *   Checks the information in the given SRM_UPS_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UPS_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_UPS_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidUPS3DParameters
(
    const SRM_UPS_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidUTM2DCoord
 *
 *   Checks the information in the given SRM_UTM_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UTM_2D_COORD whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_UTM_2D_COORD is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidUTM2DCoord
(
    const SRM_UTM_2D_COORD *value_ptr,
          SE_BOOLEAN        print_error,
          SE_UINT32        *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidUTM2DParameters
 *
 *   Checks the information in the given SRM_UTM_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UTM_2D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_UTM_2D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidUTM2DParameters
(
    const SRM_UTM_2D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidUTM3DCoord
 *
 *   Checks the information in the given SRM_UTM_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UTM_3D_COORD whose contents are to be checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_UTM_3D_COORD is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidUTM3DCoord
(
    const SRM_UTM_3D_COORD *value_ptr,
          SE_BOOLEAN        print_error,
          SE_UINT32        *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ValidUTM3DParameters
 *
 *   Checks the information in the given SRM_UTM_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UTM_3D_PARAMETERS whose contents are to be
 *                checked
 *
 *   print_error -- whether to print error messages
 *
 *   error_count_ptr -- pointer to a variable in which the error count for
 *     this SRM_UTM_3D_PARAMETERS is stored; if NULL, no counting is performed
 *
 * RETURNS:
 *
 *   SE_TRUE if value_ptr is non-NULL and *value_ptr contains valid information
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SRM_ValidUTM3DParameters
(
    const SRM_UTM_3D_PARAMETERS *value_ptr,
          SE_BOOLEAN             print_error,
          SE_UINT32             *error_count_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ValidVerticalDatum
 *
 *   Checks if the given value is a valid SE_VERTICAL_DATUM_ENUM.
 *
 * PARAMETERS:
 *
 *   value -- value to be checked
 *
 * RETURNS:
 *
 *   SE_TRUE if value is valid,
 *   SE_FALSE otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_BOOLEAN
SE_ValidVerticalDatum
(
    SE_VERTICAL_DATUM_ENUM value
);
#ifdef __cplusplus
}
#endif
#endif
