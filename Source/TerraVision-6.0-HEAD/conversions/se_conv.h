/*
 * SEDRIS Conversions API
 *
 * FILE       : se_conv.h
 *
 * PROGRAMMERS: Bill Horan (SAIC), Howard Lu (SAIC),
 *              Dan Toms (SRI International)
 *
 * DESCRIPTION: provide reusable functions for conversions between:
 *                - color models (RGB, CMY, HSV, CMYK, HLS, YIQ)
 *                - coordinate systems (numerous)
 *                - feet-to-meters and meters-to-feet
 *                - degrees-to-radians and radians-to-degrees
 *
 *   The color model conversion algorithms are based on the algorithms
 *   published in "Achromatic and Colored Light", chapter 13 of
 *   Computer Graphics: Principles and Practice - Second Edition in C,
 *   by James D. Foley, Andries van Dam, Steven K. Feiner, and John F.
 *   Hughes, published by Addison-Wesley Publishing Company (Reprinted
 *   with corrections November 1992, November 1993, and July 1995),
 *   Copyright 1996, 1990 by Addison-Wesley Publishing Company, Inc.
 *
 *   Note that all of the color conversion functions expect values
 *   between 0.0 and 1.0, inclusive, for most or all of their parameters.
 *   No checking is done on these parameters, and no checking (no clamping)
 *   is performed on the results.  If a user passes in invalid values for
 *   the color conversion functions, the results are undefined.
 *
 * Release 3.0.3 - April 20, 2001
 *
 * For other release dates/detailed changes, contact SAIC.
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

#ifndef _SE_CONV_H_INCLUDED
#define _SE_CONV_H_INCLUDED

#ifndef WIN32    /* WIN32 platforms need to dllimport variables */
#define EXPORT_CONVERSIONS_SYMBOLS_WIN32
#else
#if defined (SE_TURN_OFF_CONVERSIONS_WIN32_IMPORT) || defined (_MAKE_WIN32_STATIC)
       /* exporting, so no dll import needed here */
#define EXPORT_CONVERSIONS_SYMBOLS_WIN32
#else
#define EXPORT_CONVERSIONS_SYMBOLS_WIN32 __declspec(dllimport)
       /* importing a variable from a dll */
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "se_stds.h"
#include "se_conv_print.h"
#include "se_conv_util.h"
#include "se_conv_valid.h"
#include "se_conv_default.h"

/*
 * CONST: CONV_DEBUG
 *
 *   Flag that controls whether the conversion functions of this API will
 *   have verbose error messages or be silent. If defined, they will be
 *   verbose; the default is that they will be silent.
 */
/*
#define CONV_DEBUG SE_TRUE
*/

/*
 * CONST: SE_PI
 *
 *   The ratio of the circumference of a circle to its diameter,
 *   set to the value of 3.14159265358979323846 within SEDRIS
 *   (defined and set in se_conv.c)
 */
EXPORT_CONVERSIONS_SYMBOLS_WIN32 extern const SE_FLOAT64 SE_PI;

/*
 * CONST: SE_DEGREES_PER_RADIAN = 180.0 / SE_PI
 *
 *   The number of degrees in one radian.  Used in the
 *   SE_ConvertRadiansToDegrees() function.  The actual value
 *   used is 57.2957795130823208768 (defined and set in se_conv.c).
 */
EXPORT_CONVERSIONS_SYMBOLS_WIN32 extern const SE_FLOAT64 SE_DEGREES_PER_RADIAN;


/*
 * CONST: SE_RADIANS_PER_DEGREE = SE_PI / 180.0
 *
 *   The number of radians in one degree.  Used in the
 *   SE_ConvertDegreesToRadians() function.  The actual value
 *   used is 0.0174532925199432957692 (defined and set in se_conv.c).
 */
EXPORT_CONVERSIONS_SYMBOLS_WIN32 extern const SE_FLOAT64 SE_RADIANS_PER_DEGREE;


/*
 * CONST: SE_METERS_PER_FEET
 *
 *   The number of meters in one foot.  Used in the
 *   SE_ConvertFeetToMeters() function.  The actual value
 *   used is 0.3048, which is an *exact* conversion factor
 *   from feet to meters.  The variable is defined and set in se_conv.c
 */
EXPORT_CONVERSIONS_SYMBOLS_WIN32 extern const SE_FLOAT64 SE_METERS_PER_FEET;


/*
 * CONST: SE_FEET_PER_METER = 1.0 / SE_METERS_PER_FEET
 *
 *   The number of feet in one meter.  Used in the
 *   SE_ConvertMetersToFeet() function.  The actual value
 *   used is 3.28083989501312335958 (defined and set in se_conv.c).
 */
EXPORT_CONVERSIONS_SYMBOLS_WIN32 extern const SE_FLOAT64 SE_FEET_PER_METER;


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_CheckCoordinateInSRF
 *
 *   This function determines whether a coordinate is valid, extended,
 *   or invalid with respect to a SRF. "Extended" coordinates fall outside
 *   of the well-defined extents of the SRF, however coordinate operations
 *   are supported on them but with reduced accuracy (for a variety of
 *   reasons). Invalid coordinates fall outside of the range of "extension"
 *   and are completely unsupported for use in coordinate operations.
 *
 * PARAMETERS:
 *
 *   SRF_params_ptr - a pointer to the SRF parameters.
 *
 *   source_coord_ptr - a pointer to the coordinate to be checked.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if the coordinate in valid in the specified SRF.
 *
 *   SE_COORD_EXTENDED_SOURCE_COORD - if the coordinate fell within the
 *     "extended" range of the specified SRF.
 *
 *   SE_COORD_INVALID_SOURCE_COORD - if source_coord_ptr is NULL or
 *     points to a coordinate which is invalid with respect to the source SRF.
 *
 *   SE_COORD_INVALID_SOURCE_SRF_PARAMETERS - if SRF_params_ptr is NULL,
 *     or points to SRF parameters which are invalid.
 *
 *   SE_COORD_FAILURE - if the coordinate operation failed for reasons
 *     unspecified.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SRM_CheckCoordinateInSRF
(
     const SE_SRF_PARAMETERS *SRF_params_ptr,
     const SE_COORD          *source_coord_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_CMYKtoCMY
 *
 *   This function converts a CMYK color value to a CMY color value.
 *
 *   The color model conversion algorithm is based on the algorithm
 *   published in section 13.3.2, "The CMY Model" of Computer Graphics:
 *   Principles and Practice - Second Edition in C, by James D. Foley,
 *   Andries van Dam, Steven K. Feiner, and John F. Hughes, published
 *   by Addison-Wesley Publishing Company (Reprinted with corrections
 *   November 1992, November 1993, and July 1995), Copyright 1996, 1990
 *   by Addison-Wesley Publishing Company, Inc.
 *
 *   Incoming data values are expected to be between 0.0 and 1.0, inclusive.
 *   Furthermore, after adding the incoming Black value to the incoming
 *   Cyan, Magenta, and Yellow values, all results are expected to be
 *   between 0.0 and 1.0, inclusive.  Note that no checking of parameters
 *   or results will be made by this function.  This function will *not*
 *   clamp a result even if the result is greater than 1.0.  This function
 *   simply performs the arithmetic of the following algorithm:
 *
 *     Cyan    = incoming Cyan    + incoming Black
 *     Magenta = incoming Magenta + incoming Black
 *     Yellow  = incoming Yellow  + incoming Black
 *
 * PARAMETERS:
 *
 *   original_color_ptr - a pointer to the incoming CMYK color
 *
 *   new_color_ptr - a pointer to where the converted CMY color will be stored
 *
 * RETURNS:
 *
 *   SE_COLOR_SUCCESS - if valid parameters were passed in.
 *     In this case, *new_color_ptr is set to the appropriate values.
 *
 *   SE_COLOR_NULL_PTR_OUT_PARAMETER - if new_color_ptr was NULL.
 *
 *   SE_COLOR_INVALID_ORIGINAL_COLOR - if original_color_ptr is NULL.
 *     In this case, *new_color_ptr is not modified.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COLOR_STATUS_CODE_ENUM
SE_CMYKtoCMY
(
    const SE_CMYK_DATA *original_color_ptr,
          SE_CMY_DATA  *new_color_ptr /* out parameter */
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_CMYtoCMYK
 *
 *   This function converts a CMY color value to a CMYK color value.
 *
 *   The color model conversion algorithm is based on the algorithm
 *   published in section 13.3.2, "The CMY Model" of Computer Graphics:
 *   Principles and Practice - Second Edition in C, by James D. Foley,
 *   Andries van Dam, Steven K. Feiner, and John F. Hughes, published
 *   by Addison-Wesley Publishing Company (Reprinted with corrections
 *   November 1992, November 1993, and July 1995), Copyright 1996, 1990
 *   by Addison-Wesley Publishing Company, Inc.
 *
 *   Incoming data values are expected to be between 0.0 and 1.0, inclusive.
 *
 *   The algorithm that will be applied is:
 *
 *     Black  = Minimum of (incoming Cyan,
 *                          incoming Magenta,
 *                          incoming Yellow)
 *
 *     Cyan    = incoming Cyan    - Black
 *     Magenta = incoming Magenta - Black
 *     Yellow  = incoming Yellow  - Black
 *
 * PARAMETERS:
 *
 *   original_color_ptr - a pointer to the incoming CMY color
 *
 *   new_color_ptr - a pointer to where the converted CMYK color will be stored
 *
 * RETURNS:
 *
 *   SE_COLOR_SUCCESS - if valid parameters were passed in.
 *     In this case, *new_color_ptr is set to the appropriate values.
 *
 *   SE_COLOR_NULL_PTR_OUT_PARAMETER - if new_color_ptr was NULL.
 *
 *   SE_COLOR_INVALID_ORIGINAL_COLOR - if original_color_ptr is NULL.
 *     In this case, *new_color_ptr is not modified.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COLOR_STATUS_CODE_ENUM
SE_CMYtoCMYK
(
    const SE_CMY_DATA  *original_color_ptr,
          SE_CMYK_DATA *new_color_ptr /* out parameter */
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ConvergenceOfMeridian
 *
 *   In general, a vector in a projected coordinate system must be rotated to
 *   account for the fact that meridians in such spatial reference frames are
 *   curved, and usually do not align with true north. More precisely, the
 *   rotation angle for transforming vectors must be determined.  This angle
 *   is called the Convergence Of The Meridian (COM) in Geodesy.
 *
 * PARAMETERS:
 *
 *   convert_params_ptr - the spatial reference frame (SRF) parameters of the
 *     source and destination SRFs
 *
 *   source_coord_ptr --
 *
 *   dest_cosine_gamma -- a pointer to where the cosine of gamma will be stored
 *
 *   dest_sine_gamma -- a pointer to where the sine of gamma will be stored
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in.
 *     In this case, *dest_3x3_ptr is set to the appropriate values.
 *
 *   SE_COORD_INVALID_COORD_SYSTEM_PAIR - if convert_params_ptr was NULL.
 *
 *   SE_COORD_INVALID_SOURCE_COORD - if source_coord_ptr is NULL and is
 *                                   required
 *
 *   SE_COORD_INVALID_DEST_COSINE_GAMMA -- if dest_cosine_gamma is NULL
 *
 *   SE_COORD_INVALID_DEST_SINE_GAMMA -- if dest_sine_gamma is NULL
 *
 *   SE_COORD_FAILURE - an unspecified error condition occurred
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SRM_ConvergenceOfMeridian
(
    const SE_CONVERT_COORD_SYSTEM_PAIR  convert_params_ptr,
    const SE_COORD                     *source_coord_ptr,
          SE_FLOAT64                   *dest_cosine_gamma,
          SE_FLOAT64                   *dest_sine_gamma
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertColorToGivenModel
 *
 *   This function converts a color data value in one color model to the
 *   equivalent color data value in another color model.  The three
 *   color models supported by this function are CMY (Cyan Magenta Yellow),
 *   HSV (Hue Saturation Value) and RGB (Red Green Blue).
 *
 *   The color model conversion algorithms are based on the algorithms
 *   published in 13.3.2, "The CMY Model", and 13.3.4, "The HSV Model"
 *   in Computer Graphics: Principles and Practice - Second Edition
 *   in C, by James D. Foley, Andries van Dam, Steven K. Feiner, and
 *   John F. Hughes, published by Addison-Wesley Publishing Company
 *   (Reprinted with corrections November 1992, November 1993, and July 1995),
 *   Copyright 1996, 1990 by Addison-Wesley Publishing Company, Inc.
 *
 *   All incoming data values are expected to be between 0.0 and 1.0,
 *   inclusive; the only exception is the value for Hue in the HSV
 *   color model, which is expected to be between 0.0 and 360.0, inclusive
 *   (360.0 will be automatically converted to 0.0).
 *
 *   The value of SE_POSITIVE_INFINITY will be returned for the Hue
 *   value of 'undefined'. When converting from HSV to any other system,
 *   if the Saturation value is 0.0, then the Hue value is ignored.
 *
 *   CMYK, HLS, YIQ SUPPORT NOTE:  Support for the CMYK (Cyan Magenta Yellow
 *     blacK), HLS (Hue Lightness Saturation), and YIQ color models is
 *     provided as a convenience, but it is provided in separate functions,
 *     e.g. the SE_CMYtoCMYK() and SE_CMYKtoCMY() functions.
 *
 * PARAMETERS:
 *
 *   original_color_ptr - a pointer to the incoming color, the color to convert
 *
 *   new_color_model - the target color model.  RGB, CMY, or HSV.
 *
 *   new_color_ptr - a pointer to where the converted color will
 *     be stored.  This can be the same location that original_color_ptr
 *     points to, but it does not have to be.
 *
 * RETURNS:
 *
 *   SE_COLOR_SUCCESS - if valid parameters were passed in.
 *     In this case, *new_color_ptr is set to the appropriate values.
 *
 *   SE_COLOR_NULL_PTR_OUT_PARAMETER - if new_color_ptr was NULL.
 *
 *   SE_COLOR_INVALID_ORIGINAL_COLOR - if original_color_ptr is NULL or if
 *     it is not a pointer to a valid set of Color Data values.  In this case,
 *     *new_color_ptr is not modified.
 *
 *   SE_COLOR_INVALID_NEW_COLOR_MODEL  - if new_color_model does not contain
 *     a legal value (must be SE_RGB_MODEL, SE_CMY_MODEL, or SE_HSV_MODEL).
 *     In this case, *new_color_ptr is not modified.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COLOR_STATUS_CODE_ENUM
SE_ConvertColorToGivenModel
(
    const SE_COLOR_DATA       *original_color_ptr,
          SE_COLOR_MODEL_ENUM  new_color_model,
          SE_COLOR_DATA       *new_color_ptr /* out parameter */
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ConvertCoordArray
 *
 *   This function converts an array of coordinates in one SRF to an equivalent
 *   array of coordinates in another SRF.
 *
 * PARAMETERS:
 *
 *   convert_params_ptr - a pointer to the optimized coordinate
 *     operations parameters.
 *
 *   array_size - the number of entries in the source coord array and the
 *     dest coord array (both arrays must have the same number of entries)
 *
 *   source_coord_array - the array of incoming coordinates,
 *     the coordinates to convert
 *
 *   dest_coord_array - the array where the converted coordinates will be
 *     stored.  This can be the same location/array that source_coord_array
 *     points to, but it does not have to be.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if the coordinate operation succeeded. In this
 *     case, all of the coordinates in dest_coord_array are set to the
 *     appropriate operation result.
 *
 *   SE_COORD_EXTENDED_SOURCE_COORD - if the operation succeeded, however
 *     one or more source coordinates fell within the "extended" range of the
 *     source SRF. In this case, all of the coordinates in dest_coord_array
 *     are set to the appropriate operation result.
 *
 *   SE_COORD_EXTENDED_DEST_COORD - if the operation succeeded, however
 *     one or more source coordinates after being converted fell within the
 *     "extended" range for the destination SRF, e.g. a GD coordinate converted
 *     to a UTM SRF and falling just outside the specified UTM zone, but not so
 *     far outside as to become invalid. In this case, all of the coordinates
 *     in dest_coord_array are set to the appropriate operation result.
 *
 *   SE_COORD_EXTENDED_SOURCE_AND_DEST_COORD - if the operation succeeded,
 *     however one or more source coordinates fell within the "extended" range
 *     of the source SRF, and after being converted also fell within the
 *     "extended" range for the destination SRF. In this case, all of the
 *     coordinates in dest_coord_array are set to the appropriate operation
 *     result.
 *
 *   NOTE: if either (SE_COORD_EXTENDED_SOURCE_COORD and
 *                    SE_COORD_EXTENDED_DEST_COORD) or
 *                    SE_COORD_EXTENDED_SOURCE_AND_DEST_COORD occurs then
 *            SE_COORD_EXTENDED_SOURCE_AND_DEST_COORD will be returned,
 *         else either SE_COORD_EXTENDED_SOURCE_COORD or
 *                     SE_COORD_EXTENDED_DEST_COORD
 *                     will be returned accordingly
 *
 *   SE_COORD_FAILURE - if the coordinate operation failed for reasons
 *     unspecified. In this case, *dest_coord_array is left UNCHANGED.
 *
 *   SE_COORD_UNSUPPORTED - if the coordinate operation (conversion and/or
 *     transformation) is unsupported. In this case, *dest_coord_array is
 *     left UNCHANGED.
 *
 *   SE_COORD_INVALID_SRF_PAIR - if convert_params_ptr was NULL or
 *     invalid or if coord_op_params_ptr was NULL or invalid. In this case,
 *     *dest_coord_array is left UNCHANGED.
 *
 *   SE_COORD_INVALID_SOURCE_COORD -   There are two situations when this
 *     condition is returned:
 *     1) if source_coord_array is NULL or
 *     points to an array containing an invalid value with respect to the
 *     source SRF. In this case, *dest_coord_array is left UNCHANGED. If the
 *     source_coord_array was not NULL, then the invalid coordinate(s) can be
 *     identified by repeatedly calling
 *     SE_ConvertCoordinateToGivenCoordSystem(), once for each coordinate in
 *     the array, and examining the status code for each operation.
 *     2) if the array_size has value ZERO.
 *
 *   SE_COORD_INVALID_DEST_COORD - if dest_coord_array is NULL, or points
 *     to an array containing a coordinate which after being converted is
 *     invalid for for the destination SRF, e.g. a GD coordinate of latitude 85
 *     degrees to a UTM SRF. In this case, *dest_coord_array is left UNCHANGED.
 *     If the dest_coord_array was not NULL, then the invalid coordinate(s) can
 *     be identified by repeatedly calling SE_ConvertCoordToGivenSystem(), once
 *     for each coordinate in the array, and examining the status code for each
 *     operation.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SRM_ConvertCoordArray
(
     const SE_CONVERT_COORD_SYSTEM_PAIR  convert_params_ptr,
           SE_PINT32                     array_size,
     const SE_COORD                     *source_coord_array,
           SE_COORD                     *dest_coord_array
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertCoordToGivenSystem
 *
 *   This function converts a coordinate in one spatial reference frame (SRF)
 *   to an equivalent coordinate in another SRF.
 *
 * PARAMETERS:
 *
 *   coord_op_params_ptr - a pointer to the optimized coordinate operations
 *     parameters.
 *
 *   source_coord_ptr - a pointer to the incoming coordinate, the coordinate
 *     to convert
 *
 *   dest_coord_ptr - a pointer to where the converted coordinate will
 *     be stored.  This can be the same location that source_coord_ptr
 *     points to, but it does not have to be.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in and the coordinate
 *     operation succeeded. In this case, *dest_coord_ptr is set to the
 *     appropriate operation result.
 *
 *   SE_COORD_EXTENDED_SOURCE_COORD - if the operation succeeded, but the
 *     source coordinate fell within the "extended" range of the source SRF.
 *     In this case, *dest_coord_ptr is set to the appropriate operation
 *     result.
 *
 *   SE_COORD_EXTENDED_DEST_COORD - if the operation succeeded, but the source
 *     coordinate after being converted fell within the "extended" range for
 *     the destination SRF, e.g. a GD coordinate converted to a UTM SRF and
 *     falling just outside the specified UTM zone, but not so far outside as
 *     to become invalid. In this case, *dest_coord_ptr is set to the
 *     appropriate operation result.
 *
 *   SE_COORD_EXTENDED_SOURCE_AND_DEST_COORD - if the operation succeeded, but
 *     the source coordinate fell within the "extended" range of the source
 *     SRF, and after being converted also fell within the "extended" range
 *     for the destination SRF. In this case, *dest_coord_ptr is set to the
 *     appropriate operation result.
 *
 *   SE_COORD_UNSUPPORTED - if the coordinate operation (conversion
 *     and/or transformation) is unsupported. In this case, *dest_coord_ptr
 *     is left unchanged.
 *
 *   SE_COORD_INVALID_SRF_PAIR - if coord_op_params_ptr was NULL or invalid.
 *     In this case, *dest_coord_ptr is left unchanged.
 *
 *   SE_COORD_INVALID_SOURCE_COORD - if source_coord_ptr is NULL or
 *     points to an invalid value with respect to the source SRF.
 *     In this case, *dest_coord_ptr is left unchanged.
 *
 *   SE_COORD_INVALID_DEST_COORD - if dest_coord_ptr is NULL, or the source
 *     coordinate after being converted is invalid for the destination SRF,
 *     e.g. a GD coordinate of latitude 85 degrees to a UTM SRF. In this
 *     case, *dest_coord_ptr is left unchanged.
 *
 *   SE_COORD_FAILURE - if the coordinate operation failed for reasons
 *     unspecified. In this case, *dest_coord_ptr is left unchanged.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_ConvertCoordToGivenSystem
(
    const SE_CONVERT_COORD_SYSTEM_PAIR  coord_op_params_ptr,
    const SE_COORD                     *source_coord_ptr,
          SE_COORD                     *dest_coord_ptr /* out parameter */
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertCoordToGivenSystemWithoutBoundaryChecking
 *
 *   This performs the same conversion as SE_ConvertCoordToGivenSystem()
 *   but skips boundary checks for use with trusted data in order to
 *   achieve increased speed.  As a result, operations may be "successfully"
 *   performed on invalid coordinates, or result in invalid coordinates.
 *
 * PARAMETERS:
 *
 *   coord_op_params_ptr - a pointer to the optimized coordinate operations
 *     parameters.
 *
 *   source_coord_ptr - a pointer to the incoming coordinate, the coordinate
 *     to convert
 *
 *   dest_coord_ptr - a pointer to where the converted coordinate will
 *     be stored.  This can be the same location that source_coord_ptr
 *     points to, but it does not have to be.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if the coordinate operation succeeded.
 *     In this case, *dest_coord_ptr is set to the appropriate operation
 *     result.
 *
 *   SE_COORD_UNSUPPORTED - if the coordinate operation (conversion
 *     and/or transformation) is unsupported. In this case, *dest_coord_ptr
 *     is left unchanged.
 *
 *   SE_COORD_INVALID_SRF_PAIR - if coord_op_params_ptr was NULL or invalid.
 *     In this case, *dest_coord_ptr is left unchanged.
 *
 *   SE_COORD_INVALID_SOURCE_COORD - if source_coord_ptr is NULL.
 *     In this case, *dest_coord_ptr is left unchanged.
 *
 *   SE_COORD_INVALID_DEST_COORD - if dest_coord_ptr is NULL
 *     In this case, *dest_coord_ptr is left unchanged.
 *
 *   SE_COORD_FAILURE - if the coordinate operation failed for reasons
 *     unspecified. In this case, *dest_coord_ptr is left unchanged.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_ConvertCoordToGivenSystemWithoutBoundaryChecking
(
    const SE_CONVERT_COORD_SYSTEM_PAIR  coord_op_params_ptr,
    const SE_COORD                     *source_coord_ptr,
          SE_COORD                     *dest_coord_ptr /* out parameter */
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertDegreesToRadians
 *
 *   Converts a number of degrees to a number of radians.
 *
 *   The incoming degrees parameter will be multiplied by the SE_FLOAT64
 *   constant SE_RADIANS_PER_DEGREE.  It would be faster to multiply by
 *   this constant yourself instead of calling this function.
 *
 * PARAMETERS:
 *
 *   degrees - the number of degrees to be converted into radians
 *
 * RETURNS:
 *
 *   number of radians
 *
 *-----------------------------------------------------------------------------
 */
extern SE_FLOAT64
SE_ConvertDegreesToRadians
(
    SE_FLOAT64 degrees
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertDistance
 *
 *   Convert from (feet or meters) to (feet or meters).
 *
 * PARAMETERS:
 *
 *   source_units - SRM_FEET or SRM_METERS
 *
 *   destination_units - SRM_FEET or SRM_METERS
 *
 *   elevation - the elevation to convert
 *
 * RETURNS:
 *
 *   a) original elevation if source_units==destination_units,
 *   b) the converted elevation if feet-meters conversion is needed,
 *   c) 0.0 if source or destination units are not SE_FEET or SE_METERS
 *
 *-----------------------------------------------------------------------------
 */
extern SE_FLOAT64
SE_ConvertDistance
(
    SRM_ELEVATION_UNITS_ENUM source_units,
    SRM_ELEVATION_UNITS_ENUM destination_units,
    SE_FLOAT64               elevation
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertFeetToMeters
 *
 *   Converts a length measured in feet into meters.
 *
 *   The incoming feet parameter will be multiplied by the SE_FLOAT64
 *   constant SE_METERS_PER_FEET.  It would be faster to multiply by
 *   this constant yourself instead of calling this function.
 *
 * PARAMETERS:
 *
 *   feet - the length measurement to be converted, in feet
 *
 * RETURNS:
 *
 *   the length measurement in meters.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_FLOAT64
SE_ConvertFeetToMeters
(
    SE_FLOAT64 feet
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ConvertMatrix3x3
 *
 *   Assumptions:
 *
 *   1) A 3x3 matrix (pointed to by source_3x3_ptr) exists in a spatial
 *      spatial frame SRF1.
 *
 *   2) There is a location, source_coord, in SRF1, associated with
 *      *source_3x3_ptr as the "origin" of the matrix.
 *
 *   The caller wishes to convert the source matrix from SRF1 into a different
 *   spatial reference frame SRF2, where the transformation from SRF1
 *   to SRF2 is specified by convert_params_ptr.
 *
 * PARAMETERS:
 *
 *   convert_params_ptr - the spatial reference frame (SRF) parameters of the
 *     source and destination SRFs
 *
 *   source_coord_ptr -- the "origin" of the source matrix
 *
 *   source_3x3_ptr -- a pointer to the incoming matrix, which is to be
 *                     converted
 *
 *   dest_3x3_ptr - a pointer to where the converted matrix will
 *     be stored.  This can be the same location that source_3x3_ptr
 *     points to, but it does not have to be.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in.
 *     In this case, *dest_3x3_ptr is set to the appropriate values.
 *
 *   SE_COORD_INVALID_COORD_SYSTEM_PAIR - if convert_params_ptr was NULL.
 *
 *   SE_COORD_INVALID_SOURCE_MATRIX - if source_3x3_ptr is NULL
 *
 *   SE_COORD_INVALID_SOURCE_COORD - if source_coord_ptr is NULL and is
 *                                   required
 *
 *   SE_COORD_INVALID_DEST_MATRIX - if dest_3x3_ptr is NULL
 *
 *   SE_COORD_FAILURE - an unspecified error condition occurred
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SRM_ConvertMatrix3x3
(
    const SE_CONVERT_COORD_SYSTEM_PAIR  convert_params_ptr,
    const SE_COORD                     *source_coord_ptr,
    const SE_MATRIX_3X3_TYPE           *source_3x3_ptr,
          SE_MATRIX_3X3_TYPE           *dest_3x3_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertMetersToFeet
 *
 *   Converts a length measured in meters into feet.
 *
 *   The incoming meters parameter will be multiplied by the SE_FLOAT64
 *   constant SE_FEET_PER_METER.  It would be faster to multiply by
 *   this constant yourself instead of calling this function.
 *
 * PARAMETERS:
 *
 *   meters - the length measurement to be converted, in meters
 *
 * RETURNS:
 *
 *   the length measurement in feet.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_FLOAT64
SE_ConvertMetersToFeet
(
    SE_FLOAT64 meters
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertRadiansToDegrees
 *
 *   Converts a number of radians to a number of degrees.
 *
 *   The incoming degrees parameter will be multiplied by the SE_FLOAT64
 *   constant SE_DEGREES_PER_RADIAN.  It would be faster to multiply by
 *   this constant yourself instead of calling this function.
 *
 * PARAMETERS:
 *
 *   radians - the number of radians to be converted into degrees
 *
 * RETURNS:
 *
 *   number of degrees
 *
 *-----------------------------------------------------------------------------
 */
extern SE_FLOAT64
SE_ConvertRadiansToDegrees
(
    SE_FLOAT64 radians
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ConvertVector3Type
 *
 *   Assumptions:
 *
 *   1) A vector (pointed to by source_vec_ptr) exists in a spatial reference
 *      frame SRF1. Consider this vector's data as an ordered triple,
 *      (x, y, z).
 *
 *   2) There is a location, conversion_location_src_ptr, in SRF1, associated
 *      with *source_vec_ptr as the "origin" of the vector.
 *
 *   3) For all spatial reference frame SRF1 *except* GD, GM, GEI, GSE, GSM,
 *      and SM, the vector (x, y, z) is in SRF1 coordinates.
 *         In GD, GM, GEI, GSE, GSM, and SM, (x, y, z) is in LTP coordinates
 *         for the canonical LTP located at conversion_location_src_ptr.
 *
 *   The caller wishes to convert (x, y, z) from SRF1 into a different
 *   spatial reference frame SRF2, where the transformation from SRF1
 *   to SRF2 is specified by convert_params_ptr.
 *
 * PARAMETERS:
 *
 *   convert_params_ptr - the spatial reference frame (SRF) parameters of the
 *     source and destination SRFs
 *
 *   conversion_location_src_ptr -- the "origin" of the source vector
 *
 *   source_vec_ptr -- a pointer to the incoming vector, which is to be
 *                     converted
 *
 *   dest_vec_ptr - a pointer to where the converted vector will
 *     be stored.  This can be the same location that source_vec_ptr
 *     points to, but it does not have to be.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - and *dest_vec_ptr is set to the appropriate values,
 *     if valid parameters were passed in. In this case,
 *
 *   SE_COORD_INVALID_COORD_SYSTEM_PAIR - and *dest_vec_ptr,
 *     *conversion_location_dest_ptr are left unchanged, if convert_params_ptr
 *     was NULL.
 *
 *   SE_COORD_INVALID_SOURCE_VECTOR - and *dest_vec_ptr,
 *     *conversion_location_dest_ptr are left unchanged, if source_vec_ptr is
 *     NULL.
 *
 *   SE_COORD_INVALID_SOURCE_COORD - and *dest_vec_ptr,
 *     *conversion_location_dest_ptr are left unchanged, if
 *     conversion_location_src_ptr is NULL and required.
 *
 *   SE_COORD_INVALID_DEST_VECTOR - and *dest_vec_ptr,
 *     *conversion_location_dest_ptr are left unchanged, if dest_vect_ptr
 *     is NULL.
 *
 *   SE_COORD_INVALID_DEST_COORD - and *dest_vec_ptr,
 *     *conversion_location_dest_ptr are left unchanged, if
 *     conversion_location_dest_ptr is NULL.
 *
 *   SE_COORD_FAILURE - an unspecified error condition occurred.
 *
 *   SE_COORD_UNSUPPORTED - if an invalid vector conversion was requested.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SRM_ConvertVector3Type
(
    const SE_CONVERT_COORD_SYSTEM_PAIR  convert_params_ptr,
    const SE_COORD                     *conversion_location_src_ptr,
    const SE_VECTOR_3_TYPE             *source_vec_ptr,
          SE_VECTOR_3_TYPE             *dest_vec_ptr
);


/*
 * COORDINATE CONVERSIONS
 *
 * Coordinate conversion is a multi-step process:
 *
 * 1) Specify the source and destination spatial reference frames (SRFs),
 *    including any parameters needed to define those systems. The
 *    SE_SRF_PARAMETERS contain the parameters for each SRF within the
 *    structure.
 *
 *    Make a call to the SE_CreateCoordConversionConstants() function,
 *    passing in the parameters for the source and destination SRFs,
 *    and receiving a SE_CONVERT_COORD_SYSTEM_PAIR.  Hold on to this
 *    SE_CONVERT_COORD_SYSTEM_PAIR - you will need it every time you want to
 *    convert a coordinate from the source SRF to the destination SRF.
 *
 * 2) Call either
 *      - SE_ConvertCoordToGivenSystem() OR
 *      - SE_ConvertCoordToGivenSystemWithoutBoundaryChecking()
 *
 *    Passing in the SE_CONVERT_COORD_SYSTEM_PAIR and the source coordinate,
 *    receiving the coordinate converted into the destination SRF.
 *
 *    Repeat step 2 as needed, once for every coordinate to convert
 *    between the source and destination SRF pairing defined in step 1.
 *
 *    Note - Not all SRF pairings are currently supported.
 *    See the explanation and table of supported conversions, below.
 *
 * 3) When finished converting coordinates between two SRFs, call
 *    SE_FreeCoordConversionConstants() to release the
 *    SE_CONVERT_COORD_SYSTEM_PAIR that was allocated by step 1.
 *
 *
 * DIRECTLY SUPPORTED CONVERSIONS:
 *
 * The following coordinate system conversions are directly supported:
 *
 *      GD <==> GC
 *
 *      GD <==> M
 *
 *      GD <==> EC
 *
 *      GD <==> PS
 *
 *      GD <==> LCC
 *
 *      GD <==> TM
 *
 *      GD <==> UTM
 *
 *      GD <==> LTP
 *
 *
 * REFLEXIVE CONVERSIONS:
 *
 * Reflexive conversions are also allowed.  For example, asking to convert
 * from GC to GC does not produce an error.  No actual work is done in the
 * conversion in this case, but no errors are returned for this conversion.
 * Some reflexive conversions, such as LCC to LCC, will convert their
 * elevation values from feet to meters or from meters to feet, depending on
 * the units specified in the source and destination SRFs. Also, some
 * reflexive conversions, such as GCS to GCS, will convert coordinates based
 * on the (possibly different) coordinate system offsets defined in the source
 * and destination SRF parameters.
 *
 *
 * INDIRECTLY SUPPORTED CONVERSIONS:
 *
 * Some conversions are supported as a two-step process, converting from
 * the source SRF to GD, and then converting from GD to the destination
 * SRFs.  Since two coordinate conversions are being done in these cases,
 * these cases can take twice as long and may have up to twice the error
 * compared to the directly supported conversions.  (Directly supported
 * conversions should not have an error of more than 0.001 meters, and the
 * indirect conversion errors should not exceed 0.002 meters.)  In the
 * future, we expect to add code so that the conversions that are currently
 * indirect will instead be directly supported.  For example, in the future
 * we expect to add code to directly convert from LCC to UTM, instead of
 * doing the two-step conversion currently required of LCC to GD and then GD
 * to UTM.  Sometimes indirectly supported conversions are referred to as
 * "transitive" or "chained" conversions. The user does not need to treat
 * these conversions any differently as far as the function calls are
 * concerned.  In order to convert from LCC to UTM, just ask for LCC to UTM.
 * The fact that an intermediate conversion step through GD is taking place
 * is automatically handled by the conversion code.
 *
 *
 * The user calls the same functions, regardless of whether the conversion
 * is directly supported, reflexive, or indirectly supported. If a conversion
 * is not supported between the two systems selected by the user, then the
 * call to SE_CreateCoordConversionConstants() will return the error code
 * SE_COORD_UNSUPPORTED.
 *
 * The following chart is provided as a reference to indicate which
 * conversions are supported.
 *
 *    KEY
 *
 *     D = Directly supported conversion
 *
 *     R = Reflexive conversion (these are supported)
 *
 *     I = Indirectly supported conversion
 *         (a.k.a. "chained" or "transitive")
 *
 *     . = NOT SUPPORTED
 *
 *
 *  DESTINATION
 *      |
 *      V
 *
 *   GD      R  D  .  .  .  .  .  .  D  .  D  D  D  D  D  .  D  .
 *
 *   GC      D  R  .  .  .  .  .  .  I  .  I  I  I  I  I  .  I  .
 *
 *   GM      .  .  R  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
 *
 *   GEI     .  .  .  R  .  .  .  .  .  .  .  .  .  .  .  .  .  .
 *
 *   GSE     .  .  .  .  R  .  .  .  .  .  .  .  .  .  .  .  .  .
 *
 *   GSM     .  .  .  .  .  R  .  .  .  .  .  .  .  .  .  .  .  .
 *
 *   SM      .  .  .  .  .  .  R  .  .  .  .  .  .  .  .  .  .  .
 *
 *   GCS     .  .  .  .  .  .  .  R  .  .  .  .  .  .  .  .  .  .
 *
 *   M       D  I  .  .  .  .  .  .  R  .  I  I  I  I  I  .  I  .
 *
 *   OM      .  .  .  .  .  .  .  .  .  R  .  .  .  .  .  .  .  .
 *
 *   EC      D  I  .  .  .  .  .  .  I  .  R  I  I  I  I  .  I  .
 *
 *   PS      D  I  .  .  .  .  .  .  I  .  I  R  I  I  I  .  I  .
 *
 *   LCC     D  I  .  .  .  .  .  .  I  .  I  I  R  I  I  .  I  .
 *
 *   TM      D  I  .  .  .  .  .  .  I  .  I  I  I  R  I  .  I  .
 *
 *   UTM     D  I  .  .  .  .  .  .  I  .  I  I  I  I  R  .  I  .
 *
 *   UPS     .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  R  .  .
 *
 *   LTP     D  I  .  .  .  .  .  .  I  .  I  I  I  I  I  .  R  .
 *
 *   LSR     .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  R
 *
 *           G  G  G  G  G  G  S  G  M  O  E  P  L  T  U  U  L  L
 * SOURCE--> D  C  M  E  S  S  M  C     M  C  S  C  M  T  P  T  S
 *                    I  E  M     S              C     M  S  P  R
 *
 *
 *   Supporting a Source to Destination conversion doesn't always mean being
 *   able to convert all coordinates between the two systems.  For example,
 *   when converting from GD to UTM, the GD latitude must be between -80.0
 *   and 84.0; otherwise, an error is produced.  If a call is made to
 *   SE_ConvertCoordToGivenSystem() to go from GD to UTM, but the GD latitude
 *   is less than -80.0, then SE_ConvertCoordToGivenSystem() will return
 *   the error value of SE_COORD_INVALID_DEST_COORD_SYSTEM_PARAMETERS, and
 *   no conversion will occur.  If the same call is made (if the same values
 *   are passed) to SE_ConvertCoordToGivenSystemWithoutBoundaryChecking(), then
 *   no error value is returned but the resulting conversion is meaningless.
 *   The WithoutBoundaryChecking() version of the conversion function is only
 *   meant to be called when the user is sure that the source and destination
 *   coordinates are both within the allowed conversion boundaries.  The
 *   WithoutBoundaryChecking() version of the conversion function is slightly
 *   faster than the regular conversion function, but the regular conversion
 *   function should be used unless the user knows for certain that the
 *   coordinates being converted fall within the supported boundaries.
 *
 */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_CreateCoordConversionConstants
 *
 *   This function converts parameters from a given (source, destination)
 *   pair of SRFs into an internal structure that will be used to optimize
 *   repeated coordinate conversion calls between those two SRFs.
 *
 *   Note that horizontal and vertical datum transformations are not
 *   currently supported.
 *
 * PARAMETERS:
 *
 *   source_SRF_params_ptr,
 *   dest_SRF_params_ptr - the inputs. Pointers to SE_SRF_PARAMETERS structures
 *     already filled out by the user for the source and destination SRFs that
 *     the user needs to convert between.
 *
 *   coord_op_params_ptr - OUT parameter; a pointer to an
 *     SE_CONVERT_COORD_SYSTEM_PAIR in the user's memory space
 *     that will be initialized by this call.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in. In this case,
 *     *coord_op_params_ptr will be set to point to a structure allocated by
 *     this function.  The structure will contain the optimized parameters for
 *     converting between the two SRFs. Do not attempt to access or modify this
 *     structure.  Simply call SE_FreeCoordConversionConstants() when finished
 *     with this structure (when you no longer need to convert coordinates
 *     between the two SRFs) in order to free the memory used by this
 *     structure.
 *
 *   SE_COORD_INVALID_SOURCE_SRF_PARAMETERS - if source_SRF_params_ptr is
 *     NULL or if the source SRF parameters are invalid.  In this case,
 *     *coord_op_params_ptr is set to NULL.
 *
 *   SE_COORD_INVALID_DEST_SRF_PARAMETERS - if dest_SRF_params_ptr is NULL
 *     or if the destination SRF parameters are invalid.  In this case,
 *     *coord_op_params_ptr is set to NULL.
 *
 *   SE_COORD_INVALID_SRF_PAIR - if coord_op_params_ptr was NULL.
 *
 *   SE_COORD_UNSUPPORTED - if a conversion is specified that is not supported.
 *     In this case, *coord_op_params_ptr is set to NULL.
 *
 *   SE_COORD_FAILURE - if a structure to store the optimized conversion
 *     parameters cannot be allocated (if the program is out of memory).
 *     In this case, *coord_op_params_ptr is set to NULL.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_CreateCoordConversionConstants
(
     const SE_SRF_PARAMETERS            *source_SRF_params_ptr,
     const SE_SRF_PARAMETERS            *dest_SRF_params_ptr,
           SE_CONVERT_COORD_SYSTEM_PAIR *coord_op_params_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_FreeCoordConversionConstants
 *
 *   Free the memory directly associated with the internal coordinate
 *   conversion parameters.  The memory was allocated by this
 *   API during an earlier call to SE_CreateCoordConversionConstants().
 *
 * PARAMETERS:
 *
 *   to_free - a pointer to the internal coordinate operations parameters
 *             to be freed.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if to_free pointed to a valid set of internal
 *     coordinate operations parameters and the memory was successfully freed.
 *
 *   SE_COORD_FAILURE - if to_free didn't point to a valid set of internal
 *     coordinate operations parameters or the memory was not successfully
 *     freed.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SE_FreeCoordConversionConstants
(
    SE_CONVERT_COORD_SYSTEM_PAIR to_free
);


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_GetWGS84GeoidalSeparation
 *
 *   Returns the height of the WGS84 geoid above or below the WGS84
 *   ellipsoid, at the specified geodetic coordinates, using a grid
 *   of height adjustments from the EGM96 gravity model.
 *
 *   This grid of height adjustments must be specified by the environmental
 *   variable $SE_WW15MGH_GRD_PATH at run time.
 *
 * PARAMETERS:
 *
 *   WGS84GeodeticLocation_ptr - pointer to a Geodetic location on the WGS84
 *     geoid, for which we want the geoidal separation from the WGS84
 *     ellipsoid.
 *
 *   GeoidalSeparation_ptr - pointer to an SE_FLOAT64 variable in the user's
 *     memory space, into which the geoidal separation at the specified
 *     location will be copied.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - and sets *GeoidalSeparation_ptr to the height of the
 *     WGS84 geoid above or below the WGS84 ellipsoid, at the specified
 *     coordinates.
 *
 *   SE_COORD_INVALID_SOURCE_COORD - and *GeoidalSeparation_ptr is left
 *     unaltered, if WGS84GeodeticLocation_ptr is NULL.
 *
 *   SE_COORD_INVALID_DEST_COORD - and *GeoidalSeparation_ptr is left
 *     unaltered, if GeoidalSeparation_ptr is NULL.
 *
 *   SE_COORD_MISSING_WGS84_GEOID_SEPARATION_FILE - and *GeoidalSeparation_ptr
 *     is left unaltered, if the environmental variable $SE_WW15MGH_GRD_PATH
 *     has not been set, or if the file at that location cannot be located
 *     or successfully opened.
 *
 *   SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE - and *GeoidalSeparation_ptr
 *     is left unaltered, if the geoidal separation file is not in a valid
 *     format.
 *
 *   SE_COORD_FAILURE - and *GeoidalSeparation_ptr is left unaltered, if
 *     the internal geoidal separation table cannot be allocated.
 *
 *----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SRM_GetWGS84GeoidalSeparation
(
    const SRM_GD_2D_COORD *WGS84GeodeticLocation_ptr,
          SE_FLOAT64      *GeoidalSeparation_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_HLStoRGB
 *
 *   This function converts a HLS color value to a RGB color value.
 *
 *   The color model conversion algorithm is based on the algorithm published
 *   in section 13.3.5, "The HLS Color Model" of Computer Graphics: Principles
 *   and Practice - Second Edition in C, by James D. Foley, Andries van Dam,
 *   Steven K. Feiner, and John F. Hughes, published by Addison-Wesley
 *   Publishing Company (Reprinted with corrections November 1992, November
 *   1993, and July 1995), Copyright 1996, 1990 by Addison-Wesley Publishing
 *   Company, Inc.
 *
 * PARAMETERS:
 *
 *   original_color_ptr - a pointer to the incoming HLS color
 *
 *   new_color_ptr - a pointer to where the converted RGB color will be stored
 *
 * RETURNS:
 *
 *   SE_COLOR_SUCCESS - if valid parameters were passed in.
 *     In this case, *new_color_ptr is set to the appropriate values.
 *
 *   SE_COLOR_NULL_PTR_OUT_PARAMETER - if new_color_ptr was NULL.
 *
 *   SE_COLOR_INVALID_ORIGINAL_COLOR - if original_color_ptr is NULL.
 *     In this case, *new_color_ptr is not modified.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COLOR_STATUS_CODE_ENUM
SE_HLStoRGB
(
    const SE_HLS_DATA *original_color_ptr,
          SE_RGB_DATA *new_color_ptr /* out parameter */
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_LocalTransformation
 *
 *   Computes the transformation matrix needed to transform a canonical
 *   LTP space, located at source_coord_ptr, when the specified coordinate
 *   conversion/transformation is invoked.
 *
 * PARAMETERS:
 *
 *   convert_params_ptr - the spatial reference frame (SRF) parameters of the
 *     source and destination SRFs
 *
 *   source_coord_ptr -- "origin" of the canonical LTP space to be transformed
 *
 *   dest_3x3_ptr -- a pointer to where the resulting transformation matrix
 *                   will be stored
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if valid parameters were passed in.
 *     In this case, *dest_3x3_ptr is set to the appropriate values.
 *
 *   SE_COORD_INVALID_COORD_SYSTEM_PAIR - if convert_params_ptr was NULL.
 *
 *   SE_COORD_INVALID_SOURCE_COORD - if source_coord_ptr is NULL and is
 *                                   required
 *
 *   SE_COORD_INVALID_DEST_MATRIX - if dest_3x3_ptr is NULL
 *
 *   SE_COORD_FAILURE - an unspecified error condition occurred
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COORD_STATUS_CODE_ENUM
SRM_LocalTransformation
(
    const SE_CONVERT_COORD_SYSTEM_PAIR  convert_params_ptr,
    const SE_COORD                     *source_coord_ptr,
          SE_MATRIX_3X3_TYPE           *dest_3x3_ptr
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_RGBtoHLS
 *
 *   This function converts a RGB color value to a HLS color value.
 *
 *   The color model conversion algorithm is based on the algorithm published
 *   in section 13.3.5, "The HLS Color Model" of Computer Graphics: Principles
 *   and Practice - Second Edition in C, by James D. Foley, Andries van Dam,
 *   Steven K. Feiner, and John F. Hughes, published by Addison-Wesley
 *   Publishing Company (Reprinted with corrections November 1992, November
 *   1993, and July 1995), Copyright 1996, 1990 by Addison-Wesley Publishing
 *   Company, Inc.
 *
 * PARAMETERS:
 *
 *   original_color_ptr - a pointer to the incoming RGB color
 *
 *   new_color_ptr - a pointer to where the converted HLS color will be stored
 *
 * RETURNS:
 *
 *   SE_COLOR_SUCCESS - if valid parameters were passed in.
 *     In this case, *new_color_ptr is set to the appropriate values.
 *
 *   SE_COLOR_NULL_PTR_OUT_PARAMETER - if new_color_ptr was NULL.
 *
 *   SE_COLOR_INVALID_ORIGINAL_COLOR - if original_color_ptr is NULL.
 *     In this case, *new_color_ptr is not modified.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COLOR_STATUS_CODE_ENUM
SE_RGBtoHLS
(
    const SE_RGB_DATA *original_color_ptr,
          SE_HLS_DATA *new_color_ptr /* out parameter */
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_RGBtoYIQ
 *
 *   This function converts a RGB color value to a YIQ color value.
 *
 *   The color model conversion algorithm is based on the algorithm published
 *   in section 13.3.3, "The YIQ Color Model" of Computer Graphics: Principles
 *   and Practice - Second Edition in C, by James D. Foley, Andries van Dam,
 *   Steven K. Feiner, and John F. Hughes, published by Addison-Wesley
 *   Publishing Company (Reprinted with corrections November 1992, November
 *   1993, and July 1995), Copyright 1996, 1990 by Addison-Wesley Publishing
 *   Company, Inc.
 *
 * PARAMETERS:
 *
 *   original_color_ptr - a pointer to the incoming RGB color
 *
 *   new_color_ptr - a pointer to where the converted YIQ color will be stored
 *
 * RETURNS:
 *
 *   SE_COLOR_SUCCESS - if valid parameters were passed in.
 *     In this case, *new_color_ptr is set to the appropriate values.
 *
 *   SE_COLOR_NULL_PTR_OUT_PARAMETER - if new_color_ptr was NULL.
 *
 *   SE_COLOR_INVALID_ORIGINAL_COLOR - if original_color_ptr is NULL.
 *     In this case, *new_color_ptr is not modified.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COLOR_STATUS_CODE_ENUM
SE_RGBtoYIQ
(
    const SE_RGB_DATA *original_color_ptr,
          SE_YIQ_DATA *new_color_ptr /* out parameter */
);


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_YIQtoRGB
 *
 *   This function converts a YIQ color value to a RGB color value.
 *
 *   The color model conversion algorithm is based on the algorithm published
 *   in section 13.3.3, "The YIQ Color Model" of Computer Graphics: Principles
 *   and Practice - Second Edition in C, by James D. Foley, Andries van Dam,
 *   Steven K. Feiner, and John F. Hughes, published by Addison-Wesley
 *   Publishing Company (Reprinted with corrections November 1992, November
 *   1993, and July 1995), Copyright 1996, 1990 by Addison-Wesley Publishing
 *   Company, Inc.
 *
 * PARAMETERS:
 *
 *   original_color_ptr - a pointer to the incoming YIQ color
 *
 *   new_color_ptr - a pointer to where the converted RGB color will be stored
 *
 * RETURNS:
 *
 *   SE_COLOR_SUCCESS - if valid parameters were passed in.
 *     In this case, *new_color_ptr is set to the appropriate values.
 *
 *   SE_COLOR_NULL_PTR_OUT_PARAMETER - if new_color_ptr was NULL.
 *
 *   SE_COLOR_INVALID_ORIGINAL_COLOR - if original_color_ptr is NULL.
 *     In this case, *new_color_ptr is not modified.
 *
 *-----------------------------------------------------------------------------
 */
extern SE_COLOR_STATUS_CODE_ENUM
SE_YIQtoRGB
(
    const SE_YIQ_DATA *original_color_ptr,
          SE_RGB_DATA *new_color_ptr /* out parameter */
);
#ifdef __cplusplus
}
#endif

#endif
