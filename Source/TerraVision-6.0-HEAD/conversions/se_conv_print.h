/*
 * SEDRIS Conversions API
 *
 * FILE       : se_conv_print.h
 *
 * PROGRAMMERS: Michele L. Worley(SAIC)
 *
 * DESCRIPTION: provide reusable printing functions for types used for:
 *                - color models (RGB, CMY, HSV, CMYK)
 *                - coordinate systems (numerous)
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


/*
 * Ensure that se_conv_types.h is included only once.
 */
#ifndef _SE_CONVERSIONS_PRINT_H_INCLUDED
#define _SE_CONVERSIONS_PRINT_H_INCLUDED

/*
 * Allow C++ compilers to use this API without any problems
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> /* needed for FILE definition */
#include "se_conv_types.h"


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_SetOutFilePtr
 *
 *   Sets the FILE pointer to which output is directed.
 *
 *   WARNING: If you close this file after calling this function,
 *            remember that the SE_xxx functions will still
 *            try to use it.
 *
 * PARAMETERS:
 *
 *   out_file_ptr -- stream to which output is to be directed
 *
 * RETURNS:
 *
 *   SE_COORD_FAILURE if err_file_ptr is NULL; in which case nothing is
 *      changed
 *
 *   SE_COORD_SUCCESS otherwise
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_SetOutFilePtr
(
    FILE *out_file_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_GetOutFilePtr
 *
 *   Returns the FILE pointer to which output is directed.
 *
 * PARAMETERS:
 *
 *   nothing
 *
 * RETURNS:
 *
 *   FILE pointer to stream to which output is to be directed
 *
 *-----------------------------------------------------------------------------
 */
extern FILE *
SE_GetOutFilePtr(void);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_GetMinWidth
 *
 *   Retrieves the minimum width used when printing floating-point numbers.
 *
 * PARAMETERS:
 *
 *   nothing
 *
 * RETURNS:
 *
 *   min_width -- minimum field width. If the converted argument is smaller
 *                than min_width, it will be padded on the left with spaces
 *                to make up the field width.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_UINT16
SE_GetMinWidth(void);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_SetMinWidth
 *
 *   Sets the minimum width used when printing floating-point numbers.
 *
 * PARAMETERS:
 *
 *   min_width -- minimum field width. If the converted argument is smaller
 *                than min_width, it will be padded on the left with spaces
 *                to make up the field width.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_SetMinWidth
(
    SE_UINT16 min_width
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_GetPrecision
 *
 *   Retrieves the precision used when printing floating-point numbers.
 *
 * PARAMETERS:
 *
 *   nothing
 *
 * RETURNS:
 *
 *   precision -- number of digits to be printed after the decimal point, when
 *                printing an SE_FLOAT32 or SE_FLOAT64
 *
 *-----------------------------------------------------------------------------
 */
extern SE_UINT16
SE_GetPrecision(void);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_SetPrecision
 *
 *   Sets the precision used when printing floating-point numbers.
 *
 * PARAMETERS:
 *
 *   precision -- number of digits to be printed after the decimal point, when
 *                when printing an SE_FLOAT32 or SE_FLOAT64
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_SetPrecision
(
    SE_UINT16 precision
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintBoolean
 *
 *   Translates the information in the given SE_BOOLEAN into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_BOOLEAN to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintBoolean
(
    SE_BOOLEAN value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintChar
 *
 *   Translates the information in the given char into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- char to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintChar
(
    char value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCMYData
 *
 *   Prints the information in the given SE_CMY_DATA.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_CMY_DATA whose contents are to be printed; if
 *                NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintCMYData
(
    const SE_CMY_DATA *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCMYKData
 *
 *   Prints the information in the given SE_CMYK_DATA.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_CMYK_DATA whose contents are to be printed; if
 *                NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintCMYKData
(
    const SE_CMYK_DATA *value_ptr,
    const char         *name_ptr,
          SE_UINT16     level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintColorData
 *
 *   Prints the information in the given SE_COLOR_DATA.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_COLOR_DATA whose contents are to be printed; if
 *                NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintColorData
(
    const SE_COLOR_DATA *value_ptr,
    const char          *name_ptr,
          SE_UINT16      level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintColorModel
 *
 *   Translates the given SE_COLOR_MODEL_ENUM into a buffer that is suitable
 *   for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_COLOR_MODEL_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintColorModel
(
    SE_COLOR_MODEL_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintColorStatusCode
 *
 *   Translates the given SE_COLOR_STATUS_CODE_ENUM into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_COLOR_STATUS_CODE_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintColorStatusCode
(
    SE_COLOR_STATUS_CODE_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCoord
 *
 *   Prints the information in the given SE_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_COORD whose contents are to be printed; if
 *                NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintCoord
(
    const SE_COORD  *value_ptr,
    const char      *name_ptr,
          SE_UINT16  level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCoord2D
 *
 *   Prints the information in the given SE_COORD_2D.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_COORD_2D whose contents are to be printed; if
 *                NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintCoord2D
(
    const SE_COORD_2D *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCoord3D
 *
 *   Prints the information in the given SE_COORD_3D.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_COORD_3D whose contents are to be printed; if
 *                NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintCoord3D
(
    const SE_COORD_3D *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCoordStatusCode
 *
 *   Translates the given SE_COORD_STATUS_CODE_ENUM into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_COORD_STATUS_CODE_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintCoordStatusCode
(
    SE_COORD_STATUS_CODE_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintDirectionOfForward
 *
 *   Translates the given SE_DIRECTION_OF_FORWARD_ENUM into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_DIRECTION_OF_FORWARD_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintDirectionOfForward
(
    SE_DIRECTION_OF_FORWARD_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintDirectionOfUp
 *
 *   Translates the given SE_DIRECTION_OF_UP_ENUM into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_DIRECTION_OF_UP_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintDirectionOfUp
(
    SE_DIRECTION_OF_UP_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintEC2DCoord
 *
 *   Prints the information in the given SRM_EC_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_EC_2D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintEC2DCoord
(
    const SRM_EC_2D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintEC2DParameters
 *
 *   Prints the information in the given SRM_EC_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_EC_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintEC2DParameters
(
    const SRM_EC_2D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintEC3DCoord
 *
 *   Prints the information in the given SRM_EC_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_EC_3D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintEC3DCoord
(
    const SRM_EC_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintEC3DParameters
 *
 *   Prints the information in the given SRM_EC_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_EC_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintEC3DParameters
(
    const SRM_EC_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintElevationUnits
 *
 *   Translates the given SRM_ELEVATION_UNITS_ENUM into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SRM_ELEVATION_UNITS_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SRM_PrintElevationUnits
(
    SRM_ELEVATION_UNITS_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintFloat32
 *
 *   Translates the information in the given SE_FLOAT32 into a buffer that is
 *   suitable for printing. The precision and minimum field width to be used
 *   are set by SE_SetPrecision and SE_SetMinWidth.
 *
 * PARAMETERS:
 *
 *   value -- SE_FLOAT32 to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintFloat32
(
    SE_FLOAT32 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintFloat64
 *
 *   Translates the information in the given SE_FLOAT64 into a buffer that is
 *   suitable for printing. The precision and minimum field width to be used
 *   are set by SE_SetPrecision and SE_SetMinWidth.
 *
 * PARAMETERS:
 *
 *   value -- SE_FLOAT64 to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintFloat64
(
    SE_FLOAT64 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGC3DCoord
 *
 *   Prints the information in the given SRM_GC_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GC_3D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGC3DCoord
(
    const SRM_GC_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGC3DParameters
 *
 *   Prints the information in the given SRM_GC_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GC_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGC3DParameters
(
    const SRM_GC_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGCS3DCoord
 *
 *   Prints the information in the given SRM_GCS_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GCS_3D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGCS3DCoord
(
    const SRM_GCS_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGCS3DParameters
 *
 *   Prints the information in the given SRM_GCS_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GCS_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGCS3DParameters
(
    const SRM_GCS_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGD2DCoord
 *
 *   Prints the information in the given SRM_GD_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GD_2D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGD2DCoord
(
    const SRM_GD_2D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGD2DParameters
 *
 *   Prints the information in the given SRM_GD_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GD_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGD2DParameters
(
    const SRM_GD_2D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGD3DCoord
 *
 *   Prints the information in the given SRM_GD_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GD_3D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGD3DCoord
(
    const SRM_GD_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGD3DParameters
 *
 *   Prints the information in the given SRM_GD_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GD_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGD3DParameters
(
    const SRM_GD_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGEI3DCoord
 *
 *   Prints the information in the given SRM_GEI_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GEI_3D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGEI3DCoord
(
    const SRM_GEI_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGEI3DParameters
 *
 *   Prints the information in the given SRM_GEI_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GEI_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGEI3DParameters
(
    const SRM_GEI_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintGEIEpoch
 *
 *   Translates the given SE_GEI_EPOCH_ENUM into a buffer that is suitable for
 *   printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_GEI_EPOCH_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintGEIEpoch
(
    SE_GEI_EPOCH_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGM3DCoord
 *
 *   Prints the information in the given SRM_GM_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GM_3D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGM3DCoord
(
    const SRM_GM_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGM3DParameters
 *
 *   Prints the information in the given SRM_GM_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GM_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGM3DParameters
(
    const SRM_GM_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGSE3DCoord
 *
 *   Prints the information in the given SRM_GSE_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GSE_3D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGSE3DCoord
(
    const SRM_GSE_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGSE3DParameters
 *
 *   Prints the information in the given SRM_GSE_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GSE_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGSE3DParameters
(
    const SRM_GSE_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGSM3DCoord
 *
 *   Prints the information in the given SRM_GSM_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GSM_3D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGSM3DCoord
(
    const SRM_GSM_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGSM3DParameters
 *
 *   Prints the information in the given SRM_GSM_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_GSM_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintGSM3DParameters
(
    const SRM_GSM_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintHemisphere
 *
 *   Translates the given SE_HEMISPHERE_ENUM into a buffer that is suitable for
 *   printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_HEMISPHERE_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintHemisphere
(
    SE_HEMISPHERE_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintHLSData
 *
 *   Prints the information in the given SE_HLS_DATA.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_HLS_DATA whose contents are to be printed; if
 *                NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintHLSData
(
    const SE_HLS_DATA *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintHorizontalDatum
 *
 *   Translates the given SE_HORIZONTAL_DATUM_ENUM into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_HORIZONTAL_DATUM_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintHorizontalDatum
(
    SE_HORIZONTAL_DATUM_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintHSVData
 *
 *   Prints the information in the given SE_HSV_DATA.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_HSV_DATA whose contents are to be printed; if
 *                NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintHSVData
(
    const SE_HSV_DATA *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintInt16
 *
 *   Translates the information in the given SE_INT16 into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_INT16 to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintInt16
(
    SE_INT16 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintInt32
 *
 *   Translates the information in the given SE_INT32 into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_INT32 to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintInt32
(
    SE_INT32 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintInt8
 *
 *   Translates the information in the given SE_INT8 into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_INT8 to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintInt8
(
    SE_INT8 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLCC2DCoord
 *
 *   Prints the information in the given SRM_LCC_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LCC_2D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLCC2DCoord
(
    const SRM_LCC_2D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLCC2DParameters
 *
 *   Prints the information in the given SRM_LCC_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LCC_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLCC2DParameters
(
    const SRM_LCC_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLCC3DCoord
 *
 *   Prints the information in the given SRM_LCC_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LCC_3D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLCC3DCoord
(
    const SRM_LCC_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLCC3DParameters
 *
 *   Prints the information in the given SRM_LCC_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LCC_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLCC3DParameters
(
    const SRM_LCC_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLSR2DCoord
 *
 *   Prints the information in the given SRM_LSR_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LSR_2D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLSR2DCoord
(
    const SRM_LSR_2D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLSR2DParameters
 *
 *   Prints the information in the given SRM_LSR_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LSR_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLSR2DParameters
(
    const SRM_LSR_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLSR3DCoord
 *
 *   Prints the information in the given SRM_LSR_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LSR_3D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLSR3DCoord
(
    const SRM_LSR_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLSR3DParameters
 *
 *   Prints the information in the given SRM_LSR_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LSR_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLSR3DParameters
(
    const SRM_LSR_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLTP2DCoord
 *
 *   Prints the information in the given SRM_LTP_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LTP_2D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLTP2DCoord
(
    const SRM_LTP_2D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLTP2DParameters
 *
 *   Prints the information in the given SRM_LTP_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LTP_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLTP2DParameters
(
    const SRM_LTP_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLTP3DCoord
 *
 *   Prints the information in the given SRM_LTP_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LTP_3D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLTP3DCoord
(
    const SRM_LTP_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLTP3DParameters
 *
 *   Prints the information in the given SRM_LTP_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_LTP_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintLTP3DParameters
(
    const SRM_LTP_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintM2DCoord
 *
 *   Prints the information in the given SRM_M_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_M_2D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintM2DCoord
(
    const SRM_M_2D_COORD *value_ptr,
    const char           *name_ptr,
          SE_UINT16       level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintM2DParameters
 *
 *   Prints the information in the given SRM_M_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_M_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintM2DParameters
(
    const SRM_M_2D_PARAMETERS *value_ptr,
    const char                *name_ptr,
          SE_UINT16            level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintM3DCoord
 *
 *   Prints the information in the given SRM_M_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_M_3D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintM3DCoord
(
    const SRM_M_3D_COORD *value_ptr,
    const char           *name_ptr,
          SE_UINT16       level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintM3DParameters
 *
 *   Prints the information in the given SRM_M_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_M_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintM3DParameters
(
    const SRM_M_3D_PARAMETERS *value_ptr,
    const char                *name_ptr,
          SE_UINT16            level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintMatrix3X3Type
 *
 *   Prints the information in the given SE_MATRIX_3X3_TYPE.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_MATRIX_3X3_TYPE whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintMatrix3X3Type
(
    const SE_MATRIX_3X3_TYPE *value_ptr,
    const char               *name_ptr,
          SE_UINT16           level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintMatrix4X4Type
 *
 *   Prints the information in the given SE_MATRIX_4X4_TYPE.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_MATRIX_4X4_TYPE whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintMatrix4X4Type
(
    const SE_MATRIX_4X4_TYPE *value_ptr,
    const char               *name_ptr,
          SE_UINT16           level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintOM2DCoord
 *
 *   Prints the information in the given SRM_OM_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_OM_2D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintOM2DCoord
(
    const SRM_OM_2D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintOM2DParameters
 *
 *   Prints the information in the given SRM_OM_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_OM_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintOM2DParameters
(
    const SRM_OM_2D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintOM3DCoord
 *
 *   Prints the information in the given SRM_OM_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_OM_3D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintOM3DCoord
(
    const SRM_OM_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintOM3DParameters
 *
 *   Prints the information in the given SRM_OM_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_OM_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintOM3DParameters
(
    const SRM_OM_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintPint16
 *
 *   Translates the information in the given SE_PINT16 into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_PINT16 to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintPint16
(
    SE_PINT16 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintPint32
 *
 *   Translates the information in the given SE_PINT32 into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_PINT32 to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintPint32
(
    SE_PINT32 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintPint8
 *
 *   Translates the information in the given SE_PINT8 into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_PINT8 to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintPint8
(
    SE_PINT8 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintPS2DCoord
 *
 *   Prints the information in the given SRM_PS_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_PS_2D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintPS2DCoord
(
    const SRM_PS_2D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintPS2DParameters
 *
 *   Prints the information in the given SRM_PS_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_PS_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintPS2DParameters
(
    const SRM_PS_2D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintPS3DCoord
 *
 *   Prints the information in the given SRM_PS_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_PS_3D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintPS3DCoord
(
    const SRM_PS_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintPS3DParameters
 *
 *   Prints the information in the given SRM_PS_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_PS_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintPS3DParameters
(
    const SRM_PS_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintPSPole
 *
 *   Translates the given SE_PS_POLE_ENUM into a buffer that is suitable for
 *   printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_PS_POLE_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintPSPole
(
    SE_PS_POLE_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintRGBData
 *
 *   Prints the information in the given SE_RGB_DATA.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_RGB_DATA whose contents are to be printed; if
 *                NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintRGBData
(
    const SE_RGB_DATA *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSM3DCoord
 *
 *   Prints the information in the given SRM_SM_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_SM_3D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintSM3DCoord
(
    const SRM_SM_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSM3DParameters
 *
 *   Prints the information in the given SRM_SM_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_SM_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintSM3DParameters
(
    const SRM_SM_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSRF2D
 *
 *   Translates the given SRM_SRF_2D_ENUM into a buffer that is suitable for
 *   printing.
 *
 * PARAMETERS:
 *
 *   value -- SRM_SRF_2D_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SRM_PrintSRF2D
(
    SRM_SRF_2D_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSRF2DParameters
 *
 *   Prints the information in the given SRM_SRF_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_SRF_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintSRF2DParameters
(
    const SRM_SRF_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSRF3D
 *
 *   Translates the given SRM_SRF_3D_ENUM into a buffer that is suitable for
 *   printing.
 *
 * PARAMETERS:
 *
 *   value -- SRM_SRF_3D_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SRM_PrintSRF3D
(
    SRM_SRF_3D_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSRF3DParameters
 *
 *   Prints the information in the given SRM_SRF_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_SRF_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintSRF3DParameters
(
    const SRM_SRF_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintSRFParameters
 *
 *   Prints the information in the given SE_SRF_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_SRF_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintSRFParameters
(
    const SE_SRF_PARAMETERS *value_ptr,
    const char              *name_ptr,
          SE_UINT16          level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintTM2DCoord
 *
 *   Prints the information in the given SRM_TM_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_TM_2D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintTM2DCoord
(
    const SRM_TM_2D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintTM2DParameters
 *
 *   Prints the information in the given SRM_TM_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_TM_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintTM2DParameters
(
    const SRM_TM_2D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintTM3DCoord
 *
 *   Prints the information in the given SRM_TM_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_TM_3D_COORD whose contents are to be printed;
 *                if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintTM3DCoord
(
    const SRM_TM_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintTM3DParameters
 *
 *   Prints the information in the given SRM_TM_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_TM_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintTM3DParameters
(
    const SRM_TM_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintUint16
 *
 *   Translates the information in the given SE_UINT16 into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_UINT16 to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintUint16
(
    SE_UINT16 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintUint32
 *
 *   Translates the information in the given SE_UINT32 into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_UINT32 to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintUint32
(
    SE_UINT32 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintUint8
 *
 *   Translates the information in the given SE_UINT8 into a buffer that is
 *   suitable for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_UINT8 to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing an error message if value is invalid; otherwise,
 *   contains the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintUint8
(
    SE_UINT8 value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUPS2DCoord
 *
 *   Prints the information in the given SRM_UPS_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UPS_2D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintUPS2DCoord
(
    const SRM_UPS_2D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUPS2DParameters
 *
 *   Prints the information in the given SRM_UPS_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UPS_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintUPS2DParameters
(
    const SRM_UPS_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUPS3DCoord
 *
 *   Prints the information in the given SRM_UPS_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UPS_3D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintUPS3DCoord
(
    const SRM_UPS_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUPS3DParameters
 *
 *   Prints the information in the given SRM_UPS_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UPS_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintUPS3DParameters
(
    const SRM_UPS_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUTM2DCoord
 *
 *   Prints the information in the given SRM_UTM_2D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UTM_2D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintUTM2DCoord
(
    const SRM_UTM_2D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUTM2DParameters
 *
 *   Prints the information in the given SRM_UTM_2D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UTM_2D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintUTM2DParameters
(
    const SRM_UTM_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUTM3DCoord
 *
 *   Prints the information in the given SRM_UTM_3D_COORD.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UTM_3D_COORD whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintUTM3DCoord
(
    const SRM_UTM_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUTM3DParameters
 *
 *   Prints the information in the given SRM_UTM_3D_PARAMETERS.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SRM_UTM_3D_PARAMETERS whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SRM_PrintUTM3DParameters
(
    const SRM_UTM_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintVector3Type
 *
 *   Prints the information in the given SE_VECTOR_3_TYPE.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_VECTOR_3_TYPE whose contents are to be
 *                printed; if NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintVector3Type
(
    const SE_VECTOR_3_TYPE *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintVerticalDatum
 *
 *   Translates the given SE_VERTICAL_DATUM_ENUM into a buffer that is suitable
 *   for printing.
 *
 * PARAMETERS:
 *
 *   value -- SE_VERTICAL_DATUM_ENUM to be processed
 *
 * RETURNS:
 *
 *   static buffer, containing error message if value is out of range;
 *   otherwise, buffer is set to the string name equivalent to value.
 *
 *   Note that if you want to keep the results of this function, you must
 *   copy them out of the pointer that is returned, because the contents of
 *   the buffer will be overwritten the next time this function is called.
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintVerticalDatum
(
    SE_VERTICAL_DATUM_ENUM value
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintYIQData
 *
 *   Prints the information in the given SE_YIQ_DATA.
 *
 * PARAMETERS:
 *
 *   value_ptr -- pointer to SE_YIQ_DATA whose contents are to be printed; if
 *                NULL, prints error message and returns to caller
 *
 *   name_ptr -- optional; name of value_ptr field at the caller's level
 *
 *   level -- current indentation level. For each level, the output is
 *            indented 4 spaces; e.g., level = 0 means no indentation;
 *            level = 1 causes an indent of 4 spaces; level = 2 indents
 *            8 spaces, level = 3 indents 12 spaces, etc.
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
extern void
SE_PrintYIQData
(
    const SE_YIQ_DATA *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
);
#ifdef __cplusplus
}
#endif

#endif /* #ifndef/define/endif for _SE_CONVERSIONS_PRINT_H_INCLUDED */
