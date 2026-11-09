/*
 * SEDRIS Conversions API
 *
 * FILE       : se_conv_types.h
 *
 * PROGRAMMERS: Bill Horan (SAIC), Howard Lu (SAIC),
 *              Dan Toms (SRI International)
 *
 * DESCRIPTION: provide type definitions for
 *                - color models (RGB, CMY, HSV, CMYK, HLS, YIQ)
 *                - spatial reference frames (numerous)
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

#ifndef SE_CONVERSIONS_API_TYPES_H_INCLUDED
#define SE_CONVERSIONS_API_TYPES_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "se_stds.h"


/*
 * ENUM: SE_COORD_STATUS_CODE_ENUM
 *
 *   Return codes used by the SRM coordinate operations
 *   (conversion and transformation).
 */
typedef enum
{
    SE_COORD_SUCCESS,
       /*
        * Coordinate operation successful.
        */

    SE_COORD_INVALID_SOURCE_SRF_PARAMETERS,
       /*
        * The source spatial reference frame (SRF) parameters are wrong,
        * or the pointer is NULL.
        */

    SE_COORD_INVALID_DEST_SRF_PARAMETERS,
       /*
        * The destination spatial reference frame (SRF) parameters are
        * wrong, or the pointer is NULL.
        */

    SE_COORD_INVALID_SRF_PAIR,
       /*
        * The merged source+destination spatial reference frame (SRF)
        * parameters pointer is NULL or invalid.
        */

    SE_COORD_INVALID_SOURCE_COORD,
       /*
        * The source coordinate is NULL, or invalid within the source
        * spatial reference frame (SRF).
        */

    SE_COORD_EXTENDED_SOURCE_COORD,
       /*
        * The coordinate falls within extended range within the source
        * spatial reference frame (SRF).
        */

    SE_COORD_INVALID_DEST_COORD,
       /*
        * The destination coord is NULL, or invalid within the
        * destination SRF.
        */

    SE_COORD_EXTENDED_DEST_COORD,
       /*
        * The coordinate falls within extended range within the
        * destination SRF.
        */

    SE_COORD_EXTENDED_SOURCE_AND_DEST_COORD,
       /*
        * The coordinate falls within extended range within both the
        * source and destination SRFs.
        */

    SE_COORD_UNSUPPORTED,
       /*
        * The operation (conversion/transformation) is unsupported.
        */

    SE_COORD_INVALID_SOURCE_VECTOR,
       /*
        * The source vector is NULL.
        */

    SE_COORD_INVALID_DEST_VECTOR,
       /*
        * The destination vector is NULL.
        */

    SE_COORD_INVALID_SOURCE_MATRIX,
       /*
        * The source matrix is NULL.
        */

    SE_COORD_INVALID_DEST_MATRIX,
       /*
        * The destination matrix is NULL.
        */

    SE_COORD_INVALID_DEST_COSINE_GAMMA,
       /*
        * The destination cosine gamma is NULL.
        */

    SE_COORD_INVALID_DEST_SINE_GAMMA,
       /*
        * The destination sine gamma is NULL.
        */

    SE_COORD_MISSING_WGS84_GEOID_SEPARATION_FILE,
      /*
       * The geoidal separation file cannot be found, or cannot be accessed.
       */

    SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE,
      /*
       * The geoidal separation file is incomplete or improperly formatted.
       */

    SE_COORD_FAILURE
      /*
       * An error condition not covered by the above status codes occurred.
       */
} SE_COORD_STATUS_CODE_ENUM;


/*
 * ENUM: SE_COLOR_STATUS_CODE_ENUM
 *
 *   Return codes used by the SEDRIS Color Conversion functions.
 */
typedef enum
{
    SE_COLOR_SUCCESS,
    SE_COLOR_NULL_PTR_OUT_PARAMETER,
    SE_COLOR_INVALID_ORIGINAL_COLOR,
    SE_COLOR_INVALID_NEW_COLOR_MODEL
} SE_COLOR_STATUS_CODE_ENUM;



/**************************************************************************/
/*                         Types to support Color                         */
/**************************************************************************/

/*
 * STRUCT: SE_RGB_DATA
 *
 *   Used for Red Green Blue color model's data
 *
 *   red   = 0.0 fully off, red   = 1.0 fully on
 *   green = 0.0 fully off, green = 1.0 fully on
 *   blue  = 0.0 fully off, blue  = 1.0 fully on
 */
typedef struct
{
    SE_FLOAT64 red;
    SE_FLOAT64 green;
    SE_FLOAT64 blue;
} SE_RGB_DATA;


/*
 * STRUCT: SE_YIQ_DATA
 *
 *   Used for YIQ color model's data, which is a recoding of RGB for
 *   transmission efficiency and for downward compatibility with
 *   black-and-white television.
 *
 *   YIQ SUPPORT NOTE:  Support for the YIQ color model is provided as a
 *     convenience, but it is provided in separate functions, the
 *     SE_RGBtoYIQ() and SE_YIQtoRGB() functions. Thus, the SE_YIQ_DATA
 *     structure is not included in the general SE_COLOR_DATA structure.
 */
typedef struct
{
    SE_FLOAT64 y; /* luminance, not yellow */
    SE_FLOAT64 i;
    SE_FLOAT64 q;
} SE_YIQ_DATA;


/*
 * STRUCT: SE_CMY_DATA
 *
 *   Used for Cyan Magenta Yellow color model's data
 *
 *   cyan    = 0.0 fully off, cyan    = 1.0 fully on
 *   magenta = 0.0 fully off, magenta = 1.0 fully on
 *   yellow  = 0.0 fully off, yellow  = 1.0 fully on
 */
typedef struct
{
    SE_FLOAT64 cyan;
    SE_FLOAT64 magenta;
    SE_FLOAT64 yellow;
} SE_CMY_DATA;


/*
 * STRUCT: SE_CMYK_DATA
 *
 *   Used for Cyan Magenta Yellow Black color model's data
 *
 *   cyan    = 0.0 fully off, cyan    = 1.0 fully on
 *   magenta = 0.0 fully off, magenta = 1.0 fully on
 *   yellow  = 0.0 fully off, yellow  = 1.0 fully on
 *   black   = 0.0 fully off, black   = 1.0 fully on
 *
 *   CMYK SUPPORT NOTE:  Support for the CMYK (Cyan Magenta Yellow blacK)
 *     color model is provided as a convenience, but it is provided in
 *     separate functions, the SE_CMYtoCMYK() and SE_CMYKtoCMY() functions.
 *     Thus, the SE_CMYK_DATA structure is not included in the general
 *     SE_COLOR_DATA structure.
 */
typedef struct
{
    SE_FLOAT64 cyan;
    SE_FLOAT64 magenta;
    SE_FLOAT64 yellow;
    SE_FLOAT64 black;
} SE_CMYK_DATA;


/*
 * STRUCT: SE_HLS_DATA
 *
 *   Used for Hue Lightness Saturation color model's data
 *
 *   hue -- angle in degrees about the vertical axis of the HLS double
 *          hexcone, with red placed at 0 for consistency with HSV. Valid
 *          range from 0.0 to 360.0, unless saturation is zero, in which case
 *          hue is undefined.
 *
 *   lightness -- from 0.0 to 1.0 (0 percent to 100 percent)
 *
 *   saturation -- from 0.0 to 1.0
 *
 *   HLS SUPPORT NOTE:  Support for the HLS (Hue Lightness Saturation)
 *     color model is provided as a convenience, but it is provided in
 *     separate functions, the SE_RGBtoHLS() and SE_HLStoRGB() functions.
 *     Thus, the SE_HLS_DATA structure is not included in the general
 *     SE_COLOR_DATA structure.
 */
typedef struct
{
    SE_FLOAT64 hue;
    SE_FLOAT64 lightness;
    SE_FLOAT64 saturation;
} SE_HLS_DATA;


/*
 * STRUCT: SE_HSV_DATA
 *
 *   Used for Hue Saturation Value color model's data
 *
 *   hue - angle in degrees about the HSV hexcone
 *           0 degrees = red
 *         120 degrees = green
 *         240 degrees = blue
 *         (expected values are between 0.0 and 360.0)
 *         (  0.0 is included                        )
 *         (360.0 will automatically be reduced to   )
 *         (  0.0 by the functions in this API       )
 *
 *   saturation -  from 0.0 to 1.0 (0 percent to 100 percent)
 *
 *   value - from 0.0 to 1.0 (sometimes called brightness)
 */
typedef struct
{
    SE_FLOAT64 hue;
    SE_FLOAT64 saturation;
    SE_FLOAT64 value;
} SE_HSV_DATA;


/*
 * ENUM: SE_COLOR_MODEL_ENUM
 *
 *   Specifies the current color model of an SE_COLOR_DATA union (below).
 */
typedef enum
{
    SE_RGB_MODEL,
   /*
    * Red, Green, and Blue
    */

    SE_CMY_MODEL,
   /*
    * Cyan, Magenta, Yellow
    */

    SE_HSV_MODEL
   /*
    * Hue, Saturation, Value (also known as HSB for
    * Hue, Saturation, Brightness)
    */
} SE_COLOR_MODEL_ENUM;


/*
 * STRUCT: SE_COLOR_DATA
 *
 *   Used to translate between color models.  Used in the
 *   SE_ConvertColorToGivenModel() function.
 */
typedef struct
{
    SE_COLOR_MODEL_ENUM color_model;
    union
    {
        SE_CMY_DATA  cmy;
        SE_HSV_DATA  hsv;
        SE_RGB_DATA  rgb;
    } u;
} SE_COLOR_DATA;


/**************************************************************************/
/*        Coordinate System Parameters and Structures                     */
/**************************************************************************/

/*
 * ENUM: SRM_ELEVATION_UNITS_ENUM
 *
 *   Used to specify the unit of distance measurement used for
 *   height/z elevation in a spatial reference frame within the
 *   Spatial Reference Model (SRM).
 */
typedef enum
{
    SRM_FEET,
    SRM_METERS
} SRM_ELEVATION_UNITS_ENUM;

/*
 * POINTER TO STRUCT: SE_CONVERT_COORD_SYSTEM_PAIR
 *
 *   Forward reference to SE_ConvertCoordSystemParams,
 *   defined in se_coord_conv.c. This allows the definition
 *   of SE_ConvertCoordSystem params to be hidden,
 *   while allowing the declaration of functions which
 *   can use it via SE_CONVERT_COORD_SYSTEM_PAIR.
 *
 *   This structure is used to optimize multiple
 *   coordinate conversion calls without permitting
 *   the user to see the API's internal data structure.
 */
typedef struct SE_ConvertCoordSystemParams const *SE_CONVERT_COORD_SYSTEM_PAIR;


/*
 * ENUM: SRM_SRF_2D_ENUM
 *
 *   All valid 2D spatial reference frames (SRFs). Used within SE_COORD_2D and
 *   SE_SRF_2D_PARAMETERS to tell which 2D spatial reference frame is being
 *   referred to.
 *
 *   See the Spatial Reference Model for details.
 */
typedef enum
{
    SRM_GD_2D_SRF,
   /*
    * Geodetic (2D) Spatial Reference Frame
    *
    * (ORM/ERM Global Surface Origin, Fixed)
    */

    SRM_EC_2D_SRF = 8,
   /*
    * Equidistant Cylindrical (2D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_PS_2D_SRF,
   /*
    * Polar Stereographic (2D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_LCC_2D_SRF,
   /*
    * Lambert Conformal Conic (2D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_TM_2D_SRF,
   /*
    * Transverse Mercator (2D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_UTM_2D_SRF,
   /*
    * Universal Transverse Mercator (2D) SRF
    * (A family of sixty 2D SRFs, one for each zone)
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_LTP_2D_SRF,
   /*
    * Local Tangent Plane (2D) Spatial Reference Frame
    *
    * (ORM/ERM Local Surface Origin, Fixed)
    */

    SRM_LSR_2D_SRF,
   /*
    * Local Space Rectangular (2D) SRF
    *
    * (ORM-based SRF: Arbitrary ("point-of-interest") Origin)
    */

    SRM_M_2D_SRF,
   /*
    * Mercator (2D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_OM_2D_SRF,
   /*
    * Oblique Mercator (2D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_UPS_2D_SRF
   /*
    * Universal Polar Stereographic (2D) SRF
    * (A family of two 2D SRFs)
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */
} SRM_SRF_2D_ENUM;


/*
 * ENUM: SRM_SRF_3D_ENUM
 *
 *   All valid 3D spatial reference frames (SRFs). Used within SE_COORD_3D and
 *   SRM_SRF_3D_PARAMETERS to tell which 3D spatial reference frame is being
 *   referred to.
 *
 *   See the Spatial Reference Model for details.
 */
typedef enum
{
    SRM_GD_3D_SRF,
   /*
    * Geodetic (3D) Spatial Reference Frame
    *
    * (ORM/ERM Global Surface Origin, Fixed)
    */

    SRM_GC_3D_SRF,
   /*
    * Geocentric (3D) Spatial Reference Frame
    *
    * (ORM/ERM Mass-Center Origin, Fixed)
    */

    SRM_GM_3D_SRF,
   /*
    * Geomagnetic (3D) Spatial Reference Frame
    *
    * (ORM/ERM Global Surface Origin, Fixed)
    */

    SRM_GEI_3D_SRF,
   /*
    * Geocentric Equatorial Inertial (3D) SRF
    *
    * (ORM/ERM Mass-Center Origin, Inertial or Quasi-Inertial)
    */

    SRM_GSE_3D_SRF,
   /*
    * Geocentric Solar Ecliptic (3D) SRF
    *
    * (ORM/ERM Mass-Center Origin, Inertial or Quasi-Inertial)
    */

    SRM_GSM_3D_SRF,
   /*
    * Geocentric Solar Magnetospheric (3D) SRF
    *
    * (ORM/ERM Mass-Center Origin, Inertial or Quasi-Inertial)
    */

    SRM_SM_3D_SRF,
   /*
    * Solar Magnetic (3D) Spatial Reference Frame
    *
    * (ORM/ERM Mass-Center Origin, Inertial or Quasi-Inertial)
    */

    SRM_GCS_3D_SRF,
   /*
    * "Global Coordinate System" (3D) SRF
    *
    * (ORM/ERM Local Surface Origin, Fixed)
    */

    SRM_EC_3D_SRF,
   /*
    * Augmented Equidistant Cylindrical (3D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_PS_3D_SRF,
   /*
    * Augmented Polar Stereographic (3D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_LCC_3D_SRF,
   /*
    * Augmented Lambert Conformal Conic (3D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_TM_3D_SRF,
   /*
    * Augmented Transverse Mercator (3D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_UTM_3D_SRF,
   /*
    * Augmented Universal Transverse Mercator (3D) SRF
    * (A family of sixty 3D SRFs, one for each zone)
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_LTP_3D_SRF,
   /*
    * Local Tangent Plane (3D) Spatial Reference Frame
    *
    * (ORM/ERM Local Surface Origin, Fixed)
    */

    SRM_LSR_3D_SRF,
   /*
    * Local Space Rectangular (3D) SRF
    *
    * (ORM-based SRF: Arbitrary ("point-of-interest") Origin)
    */

    SRM_M_3D_SRF,
   /*
    * Augmented Mercator (3D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_OM_3D_SRF,
   /*
    * Augmented Oblique Mercator (3D) SRF
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */

    SRM_UPS_3D_SRF
   /*
    * Augmented Universal Polar Stereographic (3D) SRF
    * (A family of two 3D SRFs)
    *
    * (Projection-based SRF; ORM/ERM Global Surface Origin)
    */
} SRM_SRF_3D_ENUM;

/*
 * ENUM: SE_VERTICAL_DATUM_ENUM
 *
 *   The list of reference surfaces used to specify the vertical datum,
 *   which is used for defining a coordinate system within SEDRIS.  See
 *   the SEDRIS Spatial Reference Model (SRM) for more details.
 */
typedef enum
{
    SE_MSL_VDATUM,
   /*
    * Mean Sea Level
    */

    SE_WGS84G_VDATUM,
   /*
    * WGS 84 Geoid
    */

    SE_WGS84E_VDATUM,
   /*
    * WGS 84 Ellipsoid
    */

    SE_SPHERICAL_ACM_VDATUM,
   /*
    * R = 6371221.3 m.
    */

    SE_SPHERICAL_COA_VDATUM,
   /*
    * R = 6371229 m.
    */

    SE_SPHERICAL_NOG_VDATUM,
   /*
    * R = 6371000 m.
    */

    SE_SPHERICAL_MFE_VDATUM,
   /*
    * R = 6366707.02 m.
    */

    SE_SPHERICAL_MOD_M_VDATUM,
   /*
    * R = 6371230 m.
    */

    SE_SPHERICAL_MOD_S_VDATUM,
   /*
    * R = 6356910 m.
    */

    SE_SPHERICAL_MOD_T_VDATUM,
   /*
    * R = 6378390 m.
    */

    SE_SPHERICAL_MM5_VDATUM,
   /*
    * R = 6370000 m.
    */

    SE_EGM96_VDATUM,
   /*
    * Earth Gravity Model of 1996 Geoid
    */

    SE_NAVD88_VDATUM,
   /*
    * North American Vertical Datum of 1988
    */

    SE_NGVD29_VDATUM,
   /*
    * National Geodetic Vertical Datum of 1929
    */

    SE_NUM_VERTICAL_DATUMS
} SE_VERTICAL_DATUM_ENUM;


/*
 * ENUM: SE_HORIZONTAL_DATUM_ENUM
 *
 *   This is a list of the standard horizontal datums from Appendix B of the
 *   SEDRIS Spatial Reference Model (SRM).  The Prime Meridian for all listed
 *   datums is Greenwich.  See the SEDRIS SRM for details on how a coordinate
 *   system is affected by the selection of a horizontal datum.  See Appendix B
 *   of the SRM for a datum's reference ellipsoid as defined by SEDRIS, and for
 *   the three-parameter Molodensky transformation parameters used to convert
 *   from the specified datum to the WGS-84 datum. The National Imagery and
 *   Mapping Agency (NIMA) has defined geographic regions (generally, one or
 *   more countries) over which the specified datums are appropriately used.
 *   These regions are defined in Appendix B of the SRM (and are also noted in
 *   the comments, below).
 *
 *   For the horizontal datums defined in terms of a spherical earth model,
 *   the Prime Meridian remains Greenwich.  See Appendix B of the SRM
 *   for each datum's reference spheroid as defined by SEDRIS.  The ERM
 *   center/origin for all spherical datums is defined as identical to that of
 *   the WGS-84 ellipsoid, therefore the three-parameter Molodensky
 *   transformation parameters used to convert from the specified spherical
 *   datum to the WGS-84 datum are, by definition, all zero.  Spherical
 *   datums are applicable world-wide.
 */
typedef enum
{
    SE_ADI_A_HDATUM, /* Adindan - applicable to Ethiopia                */
    SE_ADI_B_HDATUM, /* Adindan - applicable to Sudan                   */
    SE_ADI_C_HDATUM, /* Adindan - applicable to Mali                    */
    SE_ADI_D_HDATUM, /* Adindan - applicable to Senegal                 */
    SE_ADI_E_HDATUM, /* Adindan - applicable to Burkina Faso            */
    SE_ADI_F_HDATUM, /* Adindan - applicable to Cameroon                */
    SE_ADI_M_HDATUM, /* Adindan - MEAN FOR Ethiopia, Sudan              */
    SE_AFG_HDATUM,   /* Afgooye - applicable to Somalia                 */
    SE_AIA_HDATUM,   /* Antigua Island Astro 1943 - applicable to       */
                     /*                 Antigua (Leeward Islands)       */
    SE_AIN_A_HDATUM, /* Ainel Abd 1970 - applicable to Bahrain          */
    SE_AIN_B_HDATUM, /* Ainel Abd 1970 - applicable to Saudi Arabia     */
    SE_ANO_HDATUM,   /* Annal Astro 1965 - applicable to Cocos Islands  */
    SE_ARF_A_HDATUM, /* Arc 1950 - applicable to Botswana               */
    SE_ARF_B_HDATUM, /* Arc 1950 - applicable to Lesotho                */
    SE_ARF_C_HDATUM, /* Arc 1950 - applicable to Malawi                 */
    SE_ARF_D_HDATUM, /* Arc 1950 - applicable to Swaziland              */
    SE_ARF_E_HDATUM, /* Arc 1950 - applicable to Zaire                  */
    SE_ARF_F_HDATUM, /* Arc 1950 - applicable to Zambia                 */
    SE_ARF_G_HDATUM, /* Arc 1950 - applicable to Zimbabwe               */
    SE_ARF_H_HDATUM, /* Arc 1950 - applicable to Burundi                */
    SE_ARF_M_HDATUM, /* Arc 1950 - MEAN FOR Botswana, Lesotho, Malawi,  */
                     /*     Swaziland, Zaire, Zambia, Zimbabwe          */
    SE_ARS_M_HDATUM, /* Arc 1960 - Mean Solution (Kenya & Tanzania) */
    SE_ASC_HDATUM,   /* Ascension Island 1958 - applicable to           */
                     /*                         Ascension Island        */
    SE_ASM_HDATUM,   /* Montserrat Island Astro 1958 - applicable to    */
                     /*                 Montserrat (Leeward Islands)    */
    SE_ASQ_HDATUM,   /* Astronomical Station 1952 - applicable to       */
                     /*                             Marcus Island       */
    SE_ATF_HDATUM,   /* Astro Beacon E 1945 - applicable to Iwo Jima    */
    SE_AUA_HDATUM,   /* Australian Geodetic 1966 - applicable to        */
                     /*                   Australia and Tasmania        */
    SE_AUG_HDATUM,   /* Australian Geodetic 1984 - applicable to        */
                     /*                   Australia and Tasmania        */
    SE_BAT_HDATUM,   /* Djakarta (Batavia) - applicable to Indonesia    */
                     /*                     (Sumatra)                   */
    SE_BER_HDATUM,   /* Bermuda 1957 - applicable to Bermuda            */
    SE_BID_HDATUM,   /* Bissau - applicable to Guinea-Bissau            */
    SE_BOO_HDATUM,   /* Bogota Observatory - applicable to Colombia     */
    SE_CAC_HDATUM,   /* Cape Canaveral - applicable to Bahamas, Florida */
    SE_CAI_HDATUM,   /* Campo Inchauspe - applicable to Argentina       */
    SE_CAO_HDATUM,   /* Canton Astro 1966 - applicable to Phoenix       */
                     /*                     Islands                     */
    SE_CAP_HDATUM,   /* Cape - applicable to South Africa               */
    SE_CGE_HDATUM,   /* Carthage - applicable to Tunisia                */
    SE_CHI_HDATUM,   /* Chatham Island Astro 1971 - applicable to       */
                     /*              New Zealand (Chatham Island)       */
    SE_CHU_HDATUM,   /* Chua Astro - applicable to Paraguay             */
    SE_COA_HDATUM,   /* Corrego Alegre - applicable to Brazil           */
    SE_DOB_HDATUM,   /* GUX1 Astro - applicable to Guadalcanal Island   */
    SE_EAS_HDATUM,   /* Easter Island 1967 - applicable to Easter Island*/
    SE_ENW_HDATUM,   /* Wake-Eniwetok 1960 - applicable to              */
                     /*                      Marshall Islands           */
    SE_EUR_A_HDATUM, /* European 1950 - MEAN FOR Austria, Denmark,      */
                     /*       France, West Germany, Netherlands,        */
                     /*       Switzerland                               */
    SE_EUR_B_HDATUM, /* European 1950 - applicable to Greece            */
    SE_EUR_C_HDATUM, /* European 1950 - applicable to Finland, Norway   */
    SE_EUR_D_HDATUM, /* European 1950 - applicable to Portugal, Spain   */
    SE_EUR_E_HDATUM, /* European 1950 - applicable to Cyprus            */
    SE_EUR_F_HDATUM, /* European 1950 - applicable to Egypt             */
    SE_EUR_G_HDATUM, /* European 1950 - applicable to England, Channel  */
                     /*            Islands, Scotland, Shetland Islands  */
    SE_EUR_H_HDATUM, /* European 1950 - applicable to Iran              */
    SE_EUR_I_HDATUM, /* European 1950 - applicable to Italy (Sardinia)  */
    SE_EUR_J_HDATUM, /* European 1950 - applicable to Italy (Sicily)    */
    SE_EUR_K_HDATUM, /* European 1950 - applicable to England, Ireland, */
                     /*                 Scotland, Shetland Islands      */
    SE_EUR_L_HDATUM, /* European 1950 - applicable to Malta             */
    SE_EUR_M_HDATUM, /* European 1950 - MEAN FOR Austria, Belgium,      */
                     /*        Denmark, Finland, France, West Germany,  */
                     /*        Gibraltar, Greece, Italy, Luxembourg,    */
                     /*        Netherlands, Norway, Portugal, Spain,    */
                     /*        Sweden, Switzerland                      */
    SE_EUS_HDATUM,   /* European 1979 - MEAN FOR Austria, Finland,      */
                     /*            Netherlands, Norway, Spain, Sweden,  */
                     /*            Switzerland                          */
    SE_FAH_HDATUM,   /* Oman - applicable to Oman                       */
    SE_FLO_HDATUM,   /* Observatorio Meterologico 1939 - applicable to  */
                     /*                   Azores (Corvo Flores Islands) */
    SE_FOT_HDATUM,   /* Fort Thomas 1955 - applicable to Nevis,         */
                     /*                    St. Kitts (Leeward Islands)  */
    SE_GAA_HDATUM,   /* Gan 1970 - applicable to Republic of Maldives   */
    SE_GEO_HDATUM,   /* Geodetic Datum 1949 - applicable to New Zealand */
    SE_GIZ_HDATUM,   /* DOS 1968 - applicable to New Georgia Islands    */
                     /*            (Gizo Island)                        */
    SE_GRA_HDATUM,   /* Graciosa Base SW 1948 - applicable to Azores    */
                     /*      Faial, Graciosa, Pico, Sao Jorge, Terceira)*/
    SE_GUA_HDATUM,   /* Guam 1963 - applicable to Guam                  */
    SE_HIT_HDATUM,   /* Provisional South Chilean 1963 - applicable to  */
                     /*      South Chile (Near 53 degrees South)        */
                     /*                  (Hito XVIII)                   */
    SE_HJO_HDATUM,   /* Hjorsey 1955 - applicable to Iceland            */
    SE_HKD_HDATUM,   /* Hong Kong 1963 - applicable to Hong Kong        */
    SE_HTN_HDATUM,   /* Hu-Tzu-Shan - applicable to Taiwan              */
    SE_IBE_HDATUM,   /* Bellevue (IGN) - applicable to Efate and        */
                     /*                  Erromango Islands              */
    SE_IND_B_HDATUM, /* Indian - applicable to Bangladesh               */
    SE_IND_I_HDATUM, /* Indian - applicable to India, Nepal             */
    SE_INF_A_HDATUM, /* Indian 1954 - applicable to Thailand, Vietnam   */
    SE_INH_A_HDATUM, /* Indian 1975 - applicable to Thailand            */
    SE_IRL_HDATUM,   /* Ireland 1965 - applicable to Ireland            */
    SE_ISG_HDATUM,   /* ISTS061 Astro 1968 - applicable to              */
                     /*                      South Georgia Islands      */
    SE_IST_HDATUM,   /* ISTS073 Astro 1969 - applicable to Diego Garcia */
    SE_JOH_HDATUM,   /* Johnston Island 1961 - applicable to Johnston   */
                     /*                        Island                   */
    SE_KAN_HDATUM,   /* Kandawala - applicable to Sri Lanka             */
    SE_KEA_HDATUM,   /* Kertau 1948 - applicable to West Malaysia and   */
                     /*               Singapore                         */
    SE_KEG_HDATUM,   /* Kerguelen Island 1949 - applicable to Kerguelen */
                     /*                         Island                  */
    SE_KUS_HDATUM,   /* Kusaie Astro 1951 - applicable to Caroline      */
                     /*                     Islands                     */
    SE_LCF_HDATUM,   /* L.C.5 Astro 1961 - applicable to Cayman Brac    */
                     /*                    Island                       */
    SE_LEH_HDATUM,   /* Leigon - applicable to Ghana                    */
    SE_LIB_HDATUM,   /* Liberia 1964 - applicable to Liberia            */
    SE_LUZ_A_HDATUM, /* Luzon - applicable to Philippines (excluding    */
                     /*         Mindanao)                               */
    SE_LUZ_B_HDATUM, /* Luzon - applicable to Philippines (Mindanao)    */
    SE_MAS_HDATUM,   /* Massawa - applicable to Ethiopia (Eritrea)      */
    SE_MER_HDATUM,   /* Merchich - applicable to Morocco                */
    SE_MID_HDATUM,   /* Midway Astro 1961 - applicable to Midway Islands*/
    SE_MIK_HDATUM,   /* Mahe 1971 - applicable to Mahe Island           */
    SE_MIN_A_HDATUM, /* Minna - applicable to Cameroon                  */
    SE_MIN_B_HDATUM, /* Minna - applicable to Nigeria                   */
    SE_MOD_HDATUM,   /* Rome 1940 - applicable to Italy (Sardinia)      */
    SE_MPO_HDATUM,   /* M'Poraloko - applicable to Gabon                */
    SE_MVS_HDATUM,   /* Viti Levu 1916 - applicable to Fiji             */
                     /*                  (Viti Levu Island)             */
    SE_NAH_A_HDATUM, /* Nahrwan - applicable to Oman (Masirah Island)   */
    SE_NAH_B_HDATUM, /* Nahrwan - applicable to United Arab Emirates    */
    SE_NAH_C_HDATUM, /* Nahrwan - applicable to Saudi Arabia            */
    SE_NAP_HDATUM,   /* Naparima BWI - applicable to Trinidad and Tobago*/
    SE_NAR_A_HDATUM, /* North American 1983 - applicable to Alaska      */
    SE_NAR_B_HDATUM, /* North American 1983 - applicable to Canada      */
    SE_NAR_C_HDATUM, /* North American 1983 - applicable to CONUS       */
    SE_NAR_D_HDATUM, /* North American 1983 - applicable to Mexico,     */
                     /*                       Central America           */
    SE_NAS_A_HDATUM, /* North American 1927 - MEAN FOR CONUS            */
                     /*        (East of Mississippi River)              */
                     /*        including Louisiana, Missouri, Minnesota */
    SE_NAS_B_HDATUM, /* North American 1927 - MEAN FOR CONUS            */
                     /*        (West of Mississippi River)              */
                     /*        excluding Louisiana, Missouri, Minnesota */
    SE_NAS_C_HDATUM, /* North American 1927 - MEAN FOR CONUS            */
    SE_NAS_D_HDATUM, /* North American 1927 - applicable to Alaska      */
    SE_NAS_E_HDATUM, /* North American 1927 - MEAN FOR CANADA           */
    SE_NAS_F_HDATUM, /* North American 1927 - applicable to Canada      */
                     /*                (Alberta, British Columbia)      */
    SE_NAS_G_HDATUM, /* North American 1927 - applicable to Canada      */
                     /*                (New Brunswick, Newfoundland,    */
                     /*                 Nova Scotia, Quebec)            */
    SE_NAS_H_HDATUM, /* North American 1927 - applicable to Canada      */
                     /*                (Manitoba, Ontario)              */
    SE_NAS_I_HDATUM, /* North American 1927 - applicable to Canada      */
                     /*           (Northwest Territories, Saskatchewan) */
    SE_NAS_J_HDATUM, /* North American 1927 - applicable to Canada      */
                     /*                       (Yukon)                   */
    SE_NAS_L_HDATUM, /* North American 1927 - applicable to Mexico      */
    SE_NAS_N_HDATUM, /* North American 1927 - MEAN FOR Belize, Costa    */
                     /*          Rica, El Salvador, Guatemala, Honduras,*/
                     /*          Nicaragua                              */
    SE_NAS_O_HDATUM, /* North American 1927 - applicable to Canal Zone  */
    SE_NAS_P_HDATUM, /* North American 1927 - MEAN FOR Antigua,         */
                     /*      Barbados, Barbuda, Caicos Islands, Cuba,   */
                     /*      Dominican Republic, Grand Cayman, Jamaica, */
                     /*      Turks Islands                              */
    SE_NAS_Q_HDATUM, /* North American 1927 - applicable to Bahamas     */
                     /*                (except San Salavador Island)    */
    SE_NAS_R_HDATUM, /* North American 1927 - applicable to Bahamas     */
                     /*                (San Salavador Island)           */
    SE_NAS_T_HDATUM, /* North American 1927 - applicable to Cuba        */
    SE_NAS_U_HDATUM, /* North American 1927 - applicable to Greenland   */
                     /*                (Hayes Peninsula)                */
    SE_OEG_HDATUM,   /* Old Egyptian 1907 - applicable to Egypt         */
    SE_OGB_A_HDATUM, /* Ordnance Survey Great Britain 1936 - applicable */
                     /*                                   to England    */
    SE_OGB_B_HDATUM, /* Ordnance Survey Great Britain 1936 - applicable */
                     /*                   to England, Isle of Man, Wales*/
    SE_OGB_C_HDATUM, /* Ordnance Survey Great Britain 1936 - applicable */
                     /*                   to Scotland, Shetland Islands */
    SE_OGB_D_HDATUM, /* Ordnance Survey Great Britain 1936 - applicable */
                     /*                                      to Wales   */
    SE_OGB_M_HDATUM, /* Ordnance Survey Great Britain 1936 - MEAN FOR   */
                     /*                 England, Isle of Man, Scotland, */
                     /*                 Shetland Islands, Wales         */
    SE_OHA_A_HDATUM, /* Old Hawaiian - applicable to Hawaii             */
    SE_OHA_B_HDATUM, /* Old Hawaiian - applicable to Kauai              */
    SE_OHA_C_HDATUM, /* Old Hawaiian - applicable to Maui               */
    SE_OHA_D_HDATUM, /* Old Hawaiian - applicable to Oahu               */
    SE_OHA_M_HDATUM, /* Old Hawaiian - MEAN FOR Hawaii, Kauai, Maui,    */
                     /*                Oahu                             */
    SE_PHA_HDATUM,   /* Ayabelle Lighthouse - applicable to Djibouti    */
    SE_PIT_HDATUM,   /* Pitcairn Astro 1967 - applicable to             */
                     /*                       Pitcairn Island           */
    SE_PLN_HDATUM,   /* Picodelas Nieves - applicable to Canary Islands */
    SE_POS_HDATUM,   /* Porto Santo 1936 - applicable to Porto Santo,   */
                     /*                           Madeira Islands       */
    SE_PRP_A_HDATUM, /* Provisional South American 1956 - applicable to */
                     /*                                   Bolivia       */
    SE_PRP_B_HDATUM, /* Provisional South American 1956 - applicable to */
                     /*           Northern Chile (near 19 degrees South)*/
    SE_PRP_C_HDATUM, /* Provisional South American 1956 - applicable to */
                     /*           Southern Chile (near 43 degrees South)*/
    SE_PRP_D_HDATUM, /* Provisional South American 1956 - applicable to */
                     /*                                   Colombia      */
    SE_PRP_E_HDATUM, /* Provisional South American 1956 - applicable to */
                     /*                                   Ecuador       */
    SE_PRP_F_HDATUM, /* Provisional South American 1956 - applicable to */
                     /*                                   Guyana        */
    SE_PRP_G_HDATUM, /* Provisional South American 1956 - applicable to */
                     /*                                   Peru          */
    SE_PRP_H_HDATUM, /* Provisional South American 1956 - applicable to */
                     /*                                   Venezuela     */
    SE_PRP_M_HDATUM, /* Provisional South American 1956 - MEAN FOR      */
                     /*     Bolivia, Chile, Colombia, Ecuador, Guyana,  */
                     /*     Peru, Venezuela                             */
    SE_PTB_HDATUM,   /* Point 58 - MEAN FOR Burkina Faso and Niger      */
    SE_PTN_HDATUM,   /* Pointe Noire 1948 - applicable to Congo         */
    SE_PUR_HDATUM,   /* Puerto Rico - applicable to Puerto Rico, Virgin */
                     /*               Islands                           */
    SE_QAT_HDATUM,   /* Qatar National - applicable to Qatar            */
    SE_QUO_HDATUM,   /* Qornoq - applicable to Greenland (South)        */
    SE_REU_HDATUM,   /* Reunion - applicable to Mascarene Islands       */
    SE_SAE_HDATUM,   /* Santo (DOS) 1965 - applicable to Espirito       */
                     /*                    Santo Island                 */
    SE_SAN_A_HDATUM, /* South American 1969 - applicable to Argentina   */
    SE_SAN_B_HDATUM, /* South American 1969 - applicable to Bolivia     */
    SE_SAN_C_HDATUM, /* South American 1969 - applicable to Brazil      */
    SE_SAN_D_HDATUM, /* South American 1969 - applicable to Chile       */
    SE_SAN_E_HDATUM, /* South American 1969 - applicable to Colombia    */
    SE_SAN_F_HDATUM, /* South American 1969 - applicable to Ecuador     */
                     /*                (excluding Galapagos Islands)    */
    SE_SAN_G_HDATUM, /* South American 1969 - applicable to Guyana      */
    SE_SAN_H_HDATUM, /* South American 1969 - applicable to Paraguay    */
    SE_SAN_I_HDATUM, /* South American 1969 - applicable to Peru        */
    SE_SAN_J_HDATUM, /* South American 1969 - applicable to Ecuador     */
                     /*                (Baltra,  Galapagos Islands)     */
    SE_SAN_K_HDATUM, /* South American 1969 - applicable to Trinidad &  */
                     /*                       Tobago                    */
    SE_SAN_L_HDATUM, /* South American 1969 - applicable to Venezuela   */
    SE_SAN_M_HDATUM, /* South American 1969 - MEAN FOR Argentina,       */
                     /*      Bolivia, Brazil, Chile, Colombia, Ecuador, */
                     /*      Guyana, Paraguay, Peru, Trinidad & Tobago, */
                     /*      Venezuela                                  */
    SE_SAO_HDATUM,   /* Sao Braz - applicable to Azores                 */
                     /*     (Sao Miguel, Santa Maria Islands)           */
    SE_SAP_HDATUM,   /* Sapper Hill 1943 - applicable to East Falkland  */
                     /*                    Island                       */
    SE_SCK_HDATUM,   /* Schwarzeck - applicable to Namibia              */
    SE_SGM_HDATUM,   /* Selvagem Grande 1938 - applicable to Salvage    */
                     /*                        Islands                  */
    SE_SHB_HDATUM,   /* Astro DOS 71/4 - applicable to St. Helena Island*/
    SE_SOA_HDATUM,   /* South Asia - applicable to Singapore            */
    SE_TDC_HDATUM,   /* Tristan Astro 1968 - applicable to Tristanda    */
                     /*                      Cunha                      */
    SE_TIL_HDATUM,   /* Timbalai 1948 - applicable to Brunei, East      */
                     /*                 Malaysia (Sabah, Sarawak)       */
    SE_TOY_A_HDATUM, /* Tokyo - applicable to Japan                     */
    SE_TOY_B_HDATUM, /* Tokyo - applicable to Korea                     */
    SE_TOY_C_HDATUM, /* Tokyo - applicable to Okinawa                   */
    SE_TOY_M_HDATUM, /* Tokyo - MEAN FOR Japan, Korea, Okinawa          */
    SE_TRN_HDATUM,   /* Astro Tern Island (FRIG) 1961 - applicable to   */
                     /*                                 Tern Island     */
    SE_W72_HDATUM,   /* WGS 1972 - applicable to Global Definition      */
    SE_W84_HDATUM,   /* WGS 1984 - applicable to Global Definition      */
    SE_WAK_HDATUM,   /* Wake Island Astro 1952 - applicable to Wake     */
                     /*                          Atoll                  */
    SE_ZAN_HDATUM,   /* Zanderij - applicable to Suriname               */

    SE_SPHERICAL_ACM_HDATUM,
   /*
    * R = 6371221.3 m; used by ACMES
    */

    SE_SPHERICAL_COA_HDATUM,
   /*
    * R = 6371229 m; used by COAMPS
    */

    SE_SPHERICAL_NOG_HDATUM,
   /*
    * R = 6371000 m; used by NOGAPS
    */

    SE_SPHERICAL_MFE_HDATUM,
   /*
    * R = 6366707.02 m; MultiGen Flat Earth
    */

    SE_SPHERICAL_MOD_M_HDATUM,
   /*
    * R = 6371230 m; used by MODTRAN
    *     (Midlatitude)
    */

    SE_SPHERICAL_MOD_S_HDATUM,
   /*
    * R = 6356910 m; used by MODTRAN (Subartic)
    */

    SE_SPHERICAL_MOD_T_HDATUM,
   /*
    * R = 6378390 m; used by MODTRAN (Tropical)
    */

    SE_SPHERICAL_MM5_HDATUM,
   /*
    * R = 6370000 m; used by MM5 (AFWA)
    */

    SE_AMA_HDATUM,   /* American Samoa 1962 - applicable to American     */
                     /*                        Samoa Islands             */
    SE_ARS_A_HDATUM, /* Arc 1960 - applicable to Kenya                   */
    SE_ARS_B_HDATUM, /* Arc 1960 - applicable to Tanzania                */
    SE_BUR_HDATUM,   /* Bukit Rimpah - applicable to Bangka & Belitung   */
                     /*                        Islands (Indonesia)       */
    SE_CAZ_HDATUM,   /* Camp Area Astro - applicable to McMurdo Camp     */
                     /*                        Area, Antartica           */
    SE_CCD_HDATUM,   /* S-JTSK - applicable to Czech Republic & Slovakia */
    SE_DAL_HDATUM,   /* Dabola - applicable to Guinea                    */
    SE_DID_HDATUM,   /* Deception Island - applicable to Deception       */
                     /*                        Island, Antartica         */
    SE_EST_HDATUM,   /* Coordinate System 1937 of Estonia - applicable   */
                     /*                                to Estonia        */
    SE_EUR_S_HDATUM, /* European 1950 - applicable to Iraq, Israel,      */
                     /*  Jordan, Lebanon, Kuwait, Saudi Arabia & Syria   */
    SE_EUR_T_HDATUM, /* European 1950 - applicable to Tunisia            */
    SE_GSE_HDATUM,   /* Gunung Segara - applicable to Kalimantan         */
                     /*                Island (Indonesia)                */
    SE_HEN_HDATUM,   /* Herat North - applicable to Afghanistan          */
    SE_HER_HDATUM,   /* Hermannskogel - applicable to Yugoslavia (prior  */
                     /*         to 1990), Slovenia, Croatia, Bosnia      */
                     /*        and Herzegovina & Serbia                  */
    SE_IDN_HDATUM,   /* Indonesian 1974 - applicable to Indonesia        */
    SE_IND_P_HDATUM, /* Indian - applicable to Pakistan                  */
    SE_ING_A_HDATUM, /* Indian 1960 - applicable to Vietnam              */
                     /*                (near 16 degrees N)               */
    SE_ING_B_HDATUM, /* Indian 1960 - applicable to Con Son Island       */
                     /*                               (Vietnam)          */
    SE_NAR_E_HDATUM, /* North American 1983 - applicable to Aleutian     */
                     /*                                Islands           */
    SE_NAR_H_HDATUM, /* North American 1983 - applicable to Hawaii       */
    SE_NAS_V_HDATUM, /* North American 1927 - applicable to Aleutian     */
                     /*                Islands (East of 180 degrees W)   */
    SE_NAS_W_HDATUM, /* North American 1927 - applicable to Aleutian     */
                     /*                Islands (West of 180 degrees W)   */
    SE_NSD_HDATUM,   /* North Sahara 1959 - applicable to Algeria        */
    SE_PUK_HDATUM,   /* Pulkovo 1942 - applicable to Russia              */
    SE_SPK_A_HDATUM, /* S-42 (Pulkovo 1942) - applicable to Hungary      */
    SE_SPK_B_HDATUM, /* S-42 (Pulkovo 1942) - applicable to Poland       */
    SE_SPK_C_HDATUM, /* S-42 (Pulkovo 1942) - applicable to Czech        */
                     /*                        Republic & Slovakia       */
    SE_SPK_D_HDATUM, /* S-42 (Pulkovo 1942) - applicable to Latvia       */
    SE_SPK_E_HDATUM, /* S-42 (Pulkovo 1942) - applicable to Kazakhstan   */
    SE_SPK_F_HDATUM, /* S-42 (Pulkovo 1942) - applicable to Albania      */
    SE_SPK_G_HDATUM, /* S-42 (Pulkovo 1942) - applicable to Romania      */
    SE_SRL_HDATUM,   /* Sierra Leone 1960 - applicable to Sierra Leone   */
    SE_TAN_HDATUM,   /* Tananarive Observatory 1925 - applicable to      */
                     /*                                Madagascar        */
    SE_VOI_HDATUM,   /* Voirol 1874 - applicable to Tunisia/Algeria      */
    SE_VOR_HDATUM,   /* Voirol 1960 - applicable to Algeria              */
    SE_YAC_HDATUM,   /* Yacare - applicable to Uruguay                   */
    SE_INH_A1_HDATUM,  /*  Indian 1975 - applicable to Thailand          */
    SE_TOY_B1_HDATUM,  /*  Tokyo - applicable to South Korea             */

    SE_NUM_HORIZONTAL_DATUMS
} SE_HORIZONTAL_DATUM_ENUM;


/*
 * STRUCT: SRM_GD_2D_PARAMETERS
 *
 *   The parameters needed to define the Geodetic (GD2) 2D Spatial Reference
 *   Frame (SRF). See the Spatial Reference Model for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;
} SRM_GD_2D_PARAMETERS;


/*
 * STRUCT: SRM_GD_3D_PARAMETERS
 *
 *   The parameters needed to define the Geodetic (GD) 3D Spatial Reference
 *   Frame (SRF). See the Spatial Reference Model for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;
    SE_VERTICAL_DATUM_ENUM   vertical_datum;

    SRM_ELEVATION_UNITS_ENUM elevation_units;
                            /* Spatial Reference Frame Units */
} SRM_GD_3D_PARAMETERS;


/*
 * STRUCT: SRM_GD_2D_COORD
 *
 *   A coordinate within the Geodetic (GD2) 2D Spatial Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_2D union.  Parallels the definition of the
 *   fields for the <GD Location 2D> class.
 */
typedef struct
{
    SE_FLOAT64 geodetic_longitude; /* in degrees */
    SE_FLOAT64 geodetic_latitude;  /* in degrees */
} SRM_GD_2D_COORD;


/*
 * STRUCT: SRM_GD_3D_COORD
 *
 *   A coordinate within the Geodetic (GD) 3D Spatial Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition of the
 *   fields for the <GD Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 geodetic_longitude; /* in degrees */
    SE_FLOAT64 geodetic_latitude;  /* in degrees */
    SE_FLOAT64 elevation;
              /*
               * Elevation or height; positive "outwards" along the direction
               * normal to the ORM/ERM
               */
} SRM_GD_3D_COORD;


/*
 * STRUCT: SRM_GC_3D_PARAMETERS
 *
 *   No parameters are needed to define the Geocentric (GC) 3D
 *   SRF, but a struct is declared for it anyway, to parallel the
 *   struct definitions of the other SRFs. The associated ORM/ERM
 *   is defined by the WGS-84 ellipsoid and mass-center.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SRM_GC_3D_PARAMETERS;


/*
 * STRUCT: SRM_GC_3D_COORD
 *
 *   A coordinate within the Geocentric (GC) 3D Spatial Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition of the
 *   fields for the <GC Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 x;
              /*
               * in meters; positive towards Prime Meridian in equatorial
               * plane; origin at mass-center
               */

    SE_FLOAT64 y;
              /*
               * in meters; positive towards 90 degrees east of Prime Meridian
               * in equatorial plane; origin at mass-center
               */

    SE_FLOAT64 z;
              /*
               * in meters; positive northwards along rotational axis origin
               * at mass-center
               */
} SRM_GC_3D_COORD;


/*
 * STRUCT: SRM_GM_3D_PARAMETERS
 *
 *   The parameters needed to define the Geomagnetic (GM) 3D Spatial
 *   Reference Frame (SRF). See the Spatial Reference Model for details.
 */
typedef struct
{
    /* Coordinate system orientation */
    SE_UINT16  gm_epoch;
              /* epoch definition in year Common Era */

    /* Specification of Object Reference Model (ORM) */
    SE_FLOAT64 gm_earth_radius;
              /* radius of the reference object; non-negative; in meters */
} SRM_GM_3D_PARAMETERS;


/*
 * STRUCT: SRM_GM_3D_COORD
 *
 *   A coordinate within the Geomagnetic (GM) 3D Spatial Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition
 *   of the fields for the <GM Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 gm_latitude;  /* in degrees */
    SE_FLOAT64 gm_longitude; /* in degrees */
    SE_FLOAT64 radius;
              /*
               * distance from the mass-center of the reference object;
               * non-negative; expressed in units of ORM/ERM radius
               */
} SRM_GM_3D_COORD;


/*
 * ENUM: SE_GEI_EPOCH_ENUM
 *
 *   The defined epochs for use within the Geocentric Equatorial Inertial
 *   (GEI) 3D Spatial Reference Frame (SRF).  See the Spatial Reference
 *   Model for details.
 */
typedef enum
{
    SE_ATD_EPOCH,       /* Aries-True-of-Date */
    SE_M50_EPOCH,       /* Aries-Mean-of-1950 */
    SE_FK5_J2000_EPOCH  /* Aries-mean-of-2000 */
} SE_GEI_EPOCH_ENUM;


/*
 * STRUCT: SRM_GEI_3D_PARAMETERS
 *
 *   The parameters needed to define the Geocentric Equatorial Inertial (GEI)
 *   3D Spatial Reference Frame (SRF).  See the Spatial Reference Model for
 *   details.
 *   The associated ORM/ERM is defined by the WGS-84 ellipsoid.
 */
typedef struct
{
    SE_GEI_EPOCH_ENUM epoch; /* Spatial Reference Frame Orientation */
} SRM_GEI_3D_PARAMETERS;


/*
 * STRUCT: SRM_GEI_3D_COORD
 *
 *   A coordinate within the Geocentric Equatorial Inertial (GEI) 3D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition
 *   of the fields for the <GEI Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 right_ascension;  /* in degrees of arc */
    SE_FLOAT64 declination;      /* in degrees of arc */
    SE_FLOAT64 radius;
              /*
               * distance from the mass-center of the reference object;
               * non-negative; in meters
               */
} SRM_GEI_3D_COORD;


/*
 * STRUCT: SRM_GSE_3D_PARAMETERS
 *
 *   No parameters are needed to define the Geocentric Solar Ecliptic (GSE) 3D
 *   Spatial Reference Frame (SRF), but a struct is declared for it anyway, to
 *   parallel the struct definitions of the other SRFs. The associated ORM/ERM
 *   is defined by the WGS-84 ellipsoid.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SRM_GSE_3D_PARAMETERS;


/*
 * STRUCT: SRM_GSE_3D_COORD
 *
 *   A coordinate within the Geocentric Solar Ecliptic (GSE) 3D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition of the
 *   fields for the <GSE Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 longitude; /* in degrees of arc */
    SE_FLOAT64 latitude;  /* in degrees of arc */
    SE_FLOAT64 radius;
              /*
               * distance from the mass-center of reference object;
               * non-negative; in meters
               */
} SRM_GSE_3D_COORD;


/*
 * STRUCT: SRM_GSM_3D_PARAMETERS
 *
 *   No parameters are needed to define the Geocentric Solar Magnetospheric
 *   (GSM) 3D Spatial Reference Frame (SRF), but a struct is declared for it
 *   anyway, to parallel the struct definitions of the other SRFs. The
 *   associated ORM/ERM is defined by the WGS-84 ellipsoid.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SRM_GSM_3D_PARAMETERS;


/*
 * STRUCT: SRM_GSM_3D_COORD
 *
 *   A coordinate within the Geocentric Solar Magnetospheric (GSM) 3D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition of
 *   the fields for the <GSM Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 longitude;  /* in degrees of arc */
    SE_FLOAT64 latitude;   /* in degrees of arc */
    SE_FLOAT64 radius;
              /*
               * distance from the mass-center of reference object;
               * non-negative; in meters
               */
} SRM_GSM_3D_COORD;


/*
 * STRUCT: SRM_SM_3D_PARAMETERS
 *
 *   No parameters are needed to define the Solar Magnetic (SM) 3D Spatial
 *   Reference Frame (SRF), but a struct is declared for it anyway, to parallel
 *   the struct definitions of the other SRFs.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SRM_SM_3D_PARAMETERS;


/*
 * STRUCT: SRM_SM_3D_COORD
 *
 *   A coordinate within the Solar Magnetic (SM) 3D Spatial Reference Frame
 *   (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition
 *   of the fields for the <SM Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 longitude;  /* in degrees of arc */
    SE_FLOAT64 latitude;   /* in degrees of arc */
    SE_FLOAT64 radius;
              /*
               * distance from the mass-center of reference object;
               * non-negative; in meters
               */
} SRM_SM_3D_COORD;


/*
 * STRUCT: SRM_GCS_3D_PARAMETERS
 *
 *   The parameters needed to define the GCS ("Global Coordinate System") 3D
 *   Spatial Reference Frame (SRF). See the Spatial Reference Model for
 *   details.
 *
 *   The associated ORM/ERM is defined by the WGS-84 ellipsoid,
 *   and the cell origins are specified within the SRF definition.
 */
typedef struct
{
    /* Origin Displacement of Coordinate System */
    SE_FLOAT64 x_offset; /* origin relative displacement in x */
    SE_FLOAT64 y_offset; /* origin relative displacement in y */
} SRM_GCS_3D_PARAMETERS;


/*
 * STRUCT: SRM_GCS_3D_COORD
 *
 *   A coordinate within the GCS ("Global Coordinate System") 3D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition of
 *   the fields for the <GCS Location 3D> class.
 */
typedef struct
{
    SE_PINT16  cell_ID; /* between 1 and 49778, inclusive            */
    SE_FLOAT64 x;       /* in meters, positive Eastward              */
    SE_FLOAT64 y;       /* in meters, positive Northward             */
    SE_FLOAT64 z;       /* in meters, the height or elevation value; */
                        /* positive along xy surface normal          */
} SRM_GCS_3D_COORD;


/*
 * STRUCT: SRM_M_2D_PARAMETERS
 *
 *   The parameters needed to define the Mercator (M) 2D Spatial
 *   Reference Frame (SRF). See the Spatial Reference Model for
 *   details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;

    /* Specification of Projection Basis */
    SE_FLOAT64 geodetic_longitude;
              /*
               * degrees; specifying the meridian of the
               * Y axis (positive northwards)
               */

    SE_FLOAT64 central_scale_factor;
              /* unitless; normally close to (<=) 1.0 */

    /* Origin Displacement of Coordinate System */
    SE_FLOAT64 x_offset; /* origin relative displacement in X */
    SE_FLOAT64 y_offset; /* origin relative displacement in Y */
} SRM_M_2D_PARAMETERS;


/*
 * STRUCT: SRM_M_3D_PARAMETERS
 *
 *   The parameters needed to define the Augmented Mercator (AM) 3D
 *   Spatial Reference Frame (SRF). See the Spatial Reference
 *   Model for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;
    SE_VERTICAL_DATUM_ENUM   vertical_datum;

   /* Specification of Projection Basis */
    SE_FLOAT64 geodetic_longitude;
              /*
               * in degrees; specifying the meridian of the
               * Y axis (positive northwards)
               */

    SE_FLOAT64 central_scale_factor;
              /* unitless; normally close to (<=) 1.0 */

   /*
    * Origin Displacement of Rectangular Coordinates
    */
    SE_FLOAT64 x_offset; /* origin relative displacement in x */
    SE_FLOAT64 y_offset; /* origin relative displacement in y */

    SRM_ELEVATION_UNITS_ENUM z_units; /* Spatial Reference Frame Units */
} SRM_M_3D_PARAMETERS;


/*
 * STRUCT: SRM_M_2D_COORD
 *
 *   A coordinate within the Mercator (M) 2D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_2D union.  Parallels the definition
 *   of the fields for the <M Location 2D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters; positive eastward */
    SE_FLOAT64 y; /* in meters; positive northward */
} SRM_M_2D_COORD;


/*
 * STRUCT: SRM_M_3D_COORD
 *
 *   A coordinate within the Mercator (M) Projected Coordinate
 *   System (PCS).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition
 *   of the fields for the M_Location_3D class.
 */
typedef struct
{
    SE_FLOAT64 x;  /* in meters; positive Eastward */
    SE_FLOAT64 y;  /* in meters; positive Northward */
    SE_FLOAT64 z;  /* Elevation or height; positive along xy surface normal */
} SRM_M_3D_COORD;


/*
 * STRUCT: SRM_OM_2D_PARAMETERS
 *
 *   The parameters needed to define the Oblique Mercator (OM) 2D Spatial
 *   Reference Frame (SRF). See the Spatial Reference Model for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;

    /* Specification of Projection Basis (central line) */
    SE_FLOAT64 geodetic_latitude1; /* in degrees; defines X origin */
    SE_FLOAT64 geodetic_longitude1; /* in degrees; defines Y origin */

    SE_FLOAT64 geodetic_latitude2; /* in degrees */
    SE_FLOAT64 geodetic_longitude2; /* in degrees */

    /* Origin Displacement of Coordinate System */
    SE_FLOAT64 x_offset; /* origin relative displacement in X */
    SE_FLOAT64 y_offset; /* origin relative displacement in Y */

    SE_FLOAT64 central_scale_factor;
              /* unitless; normally close to (<=) 1.0 */
} SRM_OM_2D_PARAMETERS;


/*
 * STRUCT: SRM_OM_3D_PARAMETERS
 *
 *   The parameters needed to define the Augmented Oblique Mercator (AOM) 3D
 *   Spatial Reference Frame (SRF). See the Spatial Reference Model for
 *   details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;
    SE_VERTICAL_DATUM_ENUM   vertical_datum;

   /*
    * Specification of Projection Basis (central line)
    */
    SE_FLOAT64 geodetic_latitude1;  /* in degrees; defines X origin */
    SE_FLOAT64 geodetic_longitude1; /* in degrees; defines Y origin */

    SE_FLOAT64 geodetic_latitude2;  /* in degrees */
    SE_FLOAT64 geodetic_longitude2; /* in degrees */

   /*
    * Origin Displacement of Rectangular Coordinates
    */
    SE_FLOAT64 x_offset; /* origin relative displacement in x */
    SE_FLOAT64 y_offset; /* origin relative displacement in y */

    SE_FLOAT64             central_scale_factor;
                          /* unitless; normally close to (<=) 1.0 */

    SRM_ELEVATION_UNITS_ENUM z_units; /* Spatial Reference Frame units */
} SRM_OM_3D_PARAMETERS;


/*
 * STRUCT: SRM_OM_2D_COORD
 *
 *   A coordinate within the Oblique Mercator (OM) 2D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_2D union.  Parallels the definition
 *   of the fields for the <OM Location 2D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters; positive "rightwards" of y axis */
    SE_FLOAT64 y; /* in meters; positive in direction from parametric
                   * location 1 to parametric location 2
                   */
} SRM_OM_2D_COORD;


/*
 * STRUCT: SRM_OM_3D_COORD
 *
 *   A coordinate within the Augmented Oblique Mercator (AOM) 3D
 *   Spatial Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition
 *   of the fields for the <OM Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters; positive "rightwards" of y axis */
    SE_FLOAT64 y;
              /*
               * in meters; positive in direction from parametric
               * location 1 to parametric location 2
               */
    SE_FLOAT64 z;
              /* Elevation or height; positive along xy surface normal */
} SRM_OM_3D_COORD;


/*
 * STRUCT: SRM_EC_2D_PARAMETERS
 *
 *   The parameters needed to define the Equidistant Cylindrical (EC) 2D
 *   spatial reference frame (SRF). See the SEDRIS Spatial Reference Model
 *   for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;

    /* Specification of Projection Basis (standard parallel) */
    SE_FLOAT64 geodetic_latitude;
              /*
               * degrees; standard parallel and origin of the projected X axis
               * (which is positive northwards)
               */

    /* Origin of Spatial Reference Frame */
    SE_FLOAT64 geodetic_longitude;
              /*
               * degrees; specifying the meridian of the Y axis;
               * positive northwards)
               */

    /* Origin Displacement of Coordinate System */
    SE_FLOAT64 x_offset; /* origin relative displacement in X */
    SE_FLOAT64 y_offset; /* origin relative displacement in Y */

    SE_FLOAT64 central_scale_factor;
              /*
               * unitless; normally close to (<=)1.0
               */
} SRM_EC_2D_PARAMETERS;


/*
 * STRUCT: SRM_EC_3D_PARAMETERS
 *
 *   The parameters needed to define the Augmented Equidistant Cylindrical
 *   (AEC) 3D Spatial Reference Frame (SRF).  See the SEDRIS Spatial Reference
 *   Model for details.
 */
typedef struct
{
   /*
    * Specification of Object Reference Model (ORM)
    */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;
    SE_VERTICAL_DATUM_ENUM   vertical_datum;

   /*
    * Specification of Projection Basis (standard parallel)
    */
    SE_FLOAT64 geodetic_latitude;
              /*
               * degrees; standard parallel and origin of the projected X
               * axis (which is positive northwards)
               */
    SE_FLOAT64 geodetic_longitude;
              /*
               * degrees; specifying the meridian of the Y axis;
               * positive northwards)
               */

   /*
    * Origin Displacement of Rectangular Coordinates
    */
    SE_FLOAT64 x_offset; /* origin relative displacement in x */
    SE_FLOAT64 y_offset; /* origin relative displacement in y */

    SE_FLOAT64 central_scale_factor;
              /* unitless; normally close to (<=)1.0 */

    SRM_ELEVATION_UNITS_ENUM z_units; /* Spatial Reference Frame units */
} SRM_EC_3D_PARAMETERS;


/*
 * STRUCT: SRM_EC_2D_COORD
 *
 *   A coordinate within the Equidistant Cylindrical (EC) 2D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_2D union.  Parallels the definition
 *   of the fields for the <EC Location 2D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters; positive eastward */
    SE_FLOAT64 y; /* in meters; positive northward */
} SRM_EC_2D_COORD;


/*
 * STRUCT: SRM_EC_3D_COORD
 *
 *   A coordinate within the Augmented Equidistant Cylindrical (AEC) 3D
 *   Spatial Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition
 *   of the fields for the <EC Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters; positive eastward */
    SE_FLOAT64 y; /* in meters; positive northward */
    SE_FLOAT64 z; /* Elevation or height; positive along XY surface normal */
} SRM_EC_3D_COORD;


/*
 * ENUM: SE_PS_POLE_ENUM
 *
 *   The pole choices used in defining the Origin of Rectangular Coordinates
 *   (center) in the Augmented Polar Stereographic (APS), Polar Stereographic
 *   (PS), Augmented Universal Polar Stereographic (AUPS), and Universal Polar
 *   Stereographic (UPS) Spatial Reference Frames (SRFs).  See the Spatial
 *   Reference Model (SRM) for details.
 */
typedef enum
{
    SE_NORTH_POLE,
    SE_SOUTH_POLE
} SE_PS_POLE_ENUM;


/*
 * STRUCT: SRM_PS_2D_PARAMETERS
 *
 *   The parameters needed to define the Polar Stereographic (PS) 2D Spatial
 *   Reference Frame (SRF). See the Spatial Reference Model for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;

    /* Specification of Projection Basis (origin) */
    SE_PS_POLE_ENUM pole; /* North or South */

    SE_FLOAT64      geodetic_longitude;
                   /*
                    * degrees; specifying the meridian of the positive Y axis
                    */

    SE_FLOAT64      central_scale_factor;
                   /* unitless; normally close to (<=) 1.0 */

   /*
    * Origin Displacement of Rectangular Coordinates
    */
    SE_FLOAT64 x_offset; /* origin relative displacement in x */
    SE_FLOAT64 y_offset; /* origin relative displacement in y */
} SRM_PS_2D_PARAMETERS;


/*
 * STRUCT: SRM_PS_3D_PARAMETERS
 *
 *   The parameters needed to define the Augmented Polar Stereographic (APS)
 *   3D Spatial Reference Frame (SRF).  See the Spatial Reference Model for
 *   details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;
    SE_VERTICAL_DATUM_ENUM   vertical_datum;

   /*
    * Specification of Projection Basis (origin)
    */
    SE_PS_POLE_ENUM pole; /* North or South */
    SE_FLOAT64      geodetic_longitude;
                   /*
                    * degrees; specifying the meridian of the positive Y axis
                    */

    SE_FLOAT64      central_scale_factor;
                   /* unitless; normally close to (<=) 1.0 */

    SRM_ELEVATION_UNITS_ENUM z_units; /* Spatial Reference Frame Units */

   /*
    * Origin Displacement of Rectangular Coordinates
    */
    SE_FLOAT64 x_offset; /* origin relative displacement in x */
    SE_FLOAT64 y_offset; /* origin relative displacement in y */
} SRM_PS_3D_PARAMETERS;


/*
 * STRUCT: SRM_PS_2D_COORD
 *
 *   A coordinate within the Polar Stereographic (PS) 2D Spatial Reference
 *   Frame (SRF).
 *
 *   Only used within the SE_COORD_2D union.  Parallels the definition of the
 *   fields for the <PS Location 2D> class.
 */
typedef struct
{
    SE_FLOAT64 x;
              /* in meters */
    SE_FLOAT64 y;
              /*
               * in meters along meridian defined by the applicable PS
               * Spatial Reference Frame (SRF) parameters
               */
} SRM_PS_2D_COORD;


/*
 * STRUCT: SRM_PS_3D_COORD
 *
 *   A coordinate within the Augmented Polar Stereographic (APS) 3D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition of the
 *   fields for the <PS Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 x;
              /* in meters */
    SE_FLOAT64 y;
              /*
               * in meters along meridian defined by the applicable APS Spatial
               * Reference Frame (SRF) parameters
               */
    SE_FLOAT64 z;
              /* Elevation or height; positive along XY surface normal */
} SRM_PS_3D_COORD;


/*
 * STRUCT: SRM_UPS_2D_PARAMETERS
 *
 *   The parameters needed to define the Universal Polar Stereographic (UPS) 2D
 *   Spatial Reference Frame (SRF).  See the Spatial Reference Model for
 *   details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;

    /* Specification of Projection Basis (origin) */
    SE_PS_POLE_ENUM pole; /* North or South */

   /*
    * Origin Displacement of Rectangular Coordinates
    */
    SE_FLOAT64 x_offset; /* origin relative displacement in x */
    SE_FLOAT64 y_offset; /* origin relative displacement in y */
} SRM_UPS_2D_PARAMETERS;


/*
 * STRUCT: SRM_UPS_3D_PARAMETERS
 *
 *   The parameters needed to define the Augmented Universal Polar
 *   Stereographic (AUPS) 3D Spatial Reference Frame (SRF).  See the Spatial
 *   Reference Model for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;
    SE_VERTICAL_DATUM_ENUM   vertical_datum;

    /* Specification of Projection Basis (origin) */
    SE_PS_POLE_ENUM          pole;   /* North or South */

   /*
    * Origin Displacement of Rectangular Coordinates
    */
    SE_FLOAT64 x_offset; /* origin relative displacement in x */
    SE_FLOAT64 y_offset; /* origin relative displacement in y */

    SRM_ELEVATION_UNITS_ENUM   z_units; /* Spatial Reference Frame Units */
} SRM_UPS_3D_PARAMETERS;


/*
 * STRUCT: SRM_UPS_2D_COORD
 *
 *   A coordinate within the Universal Polar Stereographic (UPS) 2D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_2D union.  Parallels the definition of the
 *   fields for the <UPS Location 2D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters */
    SE_FLOAT64 y;
              /*
               * in meters along meridian defined by the applicable UPS Spatial
               * Reference Frame (SRF) parameters
               */
} SRM_UPS_2D_COORD;


/*
 * STRUCT: SRM_UPS_3D_COORD
 *
 *   A coordinate within the Augmented Universal Polar Stereographic (AUPS) 3D
 *   Spatial Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition of the
 *   fields for the <UPS Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters */
    SE_FLOAT64 y;
              /*
               * in meters along meridian defined by the applicable AUPS
               * Spatial Reference Frame (SRF) parameters
               */
    SE_FLOAT64 z; /* Elevation or height; positive along XY surface normal */
} SRM_UPS_3D_COORD;


/*
 * STRUCT: SRM_LCC_2D_PARAMETERS
 *
 *   The parameters needed to define the Lambert Conformal Conic (LCC) 2D
 *   Spatial Reference Frame (SRF).  See the Spatial Reference Model for
 *   details.
 */
typedef struct
{
    /*
     * Specification of Object Reference Model (ORM)
     */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;

   /*
    * Standard Parallels
    */
    SE_FLOAT64 north_parallel_geodetic_latitude; /* in degrees */
    SE_FLOAT64 south_parallel_geodetic_latitude; /* in degrees */

   /*
    * Origin of Rectangular Coordinates
    */
    SE_FLOAT64 geodetic_latitude;
                /*
                 * in degrees; specifying the parallel of the X axis;
                 * positive eastwards
                 */
    SE_FLOAT64 geodetic_longitude;
                /*
                 * in degrees, specifying the meridian of the positive y axis
                 */

    /* Origin Displacement of Coordinate System*/
    SE_FLOAT64 x_offset;  /* origin relative displacement in X */
    SE_FLOAT64 y_offset;  /* origin relative displacement in Y */
} SRM_LCC_2D_PARAMETERS;


/*
 * STRUCT: SRM_LCC_3D_PARAMETERS
 *
 *   The parameters needed to define the Augmented Lambert Conformal Conic
 *   (ALCC) 3D Spatial Reference Frame (SRF).  See the Spatial Reference Model
 *   for details.
 */
typedef struct
{
    /*
     * Specification of Object Reference Model (ORM)
     */
     SE_HORIZONTAL_DATUM_ENUM horizontal_datum;
     SE_VERTICAL_DATUM_ENUM   vertical_datum;

    /*
     * Specification of Projection Basis (standard parallels)
     */
     SE_FLOAT64 north_parallel_geodetic_latitude; /* in degrees */
     SE_FLOAT64 south_parallel_geodetic_latitude; /* in degrees */

    /*
     * Origin of Spatial Reference Frame
     */
    SE_FLOAT64 geodetic_latitude;
                /*
                 * in degrees; specifying the parallel of the X axis;
                 * positive eastwards
                 */
    SE_FLOAT64 geodetic_longitude;
                /*
                 * in degrees, specifying the meridian of the positive y axis
                 */

    SRM_ELEVATION_UNITS_ENUM z_units; /* Spatial Reference Frame Units */

    /* Origin Displacement of Coordinate System*/
    SE_FLOAT64 x_offset;  /* origin relative displacement in X */
    SE_FLOAT64 y_offset;  /* origin relative displacement in Y */
} SRM_LCC_3D_PARAMETERS;


/*
 * STRUCT: SRM_LCC_2D_COORD
 *
 *   A coordinate within the Lambert Conformal Conic (LCC) 2D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_2D union.  Parallels the definition
 *   of the fields for the <LCC Location 2D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters; positive eastward */
    SE_FLOAT64 y; /* in meters; positive northward */
} SRM_LCC_2D_COORD;


/*
 * STRUCT: SRM_LCC_3D_COORD
 *
 *   A coordinate within the Augmented Lambert Conformal Conic (ALCC) 3D
 *   Spatial Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition
 *   of the fields for the <LCC Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters; positive eastward */
    SE_FLOAT64 y; /* in meters; positive northward */
    SE_FLOAT64 z; /* Elevation or height; positive along XY surface normal */
} SRM_LCC_3D_COORD;


/*
 * STRUCT: SRM_TM_2D_PARAMETERS
 *
 *   The parameters needed to define the Transverse Mercator (TM) 2D Spatial
 *   Reference Frame (SRF).  See the Spatial Reference Model for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;

    /* Specification of Projection Basis */
    SE_FLOAT64 geodetic_latitude;   /* in degrees; used to define origin */
    SE_FLOAT64 geodetic_longitude;
              /*
               * in degrees; specifying the meridian of the positive Y axis
               * (which is positive northwards)
               */

    /* Origin Displacement of Coordinate System */
    SE_FLOAT64 x_offset; /* origin relative displacement in X */
    SE_FLOAT64 y_offset; /* origin relative displacement in Y */

    SE_FLOAT64 central_scale_factor;
              /* unitless; normally close to (<=) 1.0 */
} SRM_TM_2D_PARAMETERS;


/*
 * STRUCT: SRM_TM_3D_PARAMETERS
 *
 *   The parameters needed to define the Augmented Transverse Mercator (ATM)
 *   3D Spatial Reference Frame (SRF).  See the Spatial Reference Model for
 *   details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;
    SE_VERTICAL_DATUM_ENUM   vertical_datum;

   /*
    * Specification of Projection Basis (SRF Origin)
    */
    SE_FLOAT64 geodetic_latitude;
               /* in degrees; used to define origin */
    SE_FLOAT64 geodetic_longitude;
               /* in degrees; specifying the meridian of the positive Y axis */

    /* Origin Displacement of Coordinate System */
    SE_FLOAT64 x_offset; /* origin relative displacement in x */
    SE_FLOAT64 y_offset; /* origin relative displacement in y */

    SE_FLOAT64 central_scale_factor;
               /* unitless; normally close to (<=) 1.0 */

    SRM_ELEVATION_UNITS_ENUM z_units; /* Spatial Reference Frame Units */
} SRM_TM_3D_PARAMETERS;


/*
 * STRUCT: SRM_TM_2D_COORD
 *
 *   A coordinate within the Transverse Mercator (TM) 2D Spatial Reference
 *   Frame (SRF).
 *
 *   Only used within the SE_COORD_2D union.  Parallels the definition of the
 *   fields for the <TM Location 2D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters; positive eastward */
    SE_FLOAT64 y; /* in meters; positive northward */
} SRM_TM_2D_COORD;


/*
 * STRUCT: SRM_TM_3D_COORD
 *
 *   A coordinate within the Augmented Transverse Mercator (ATM) 3D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition of the
 *   fields for the <TM Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 x;  /* in meters; positive Eastward */
    SE_FLOAT64 y;  /* in meters; positive Northward along meridian defined by
                    *            the applicable TM Coordinate System parameters
                    */
    SE_FLOAT64 z;  /* Elevation or height; positive along xy surface normal */
} SRM_TM_3D_COORD;


/*
 * STRUCT: SRM_UTM_2D_PARAMETERS
 *
 *   The parameters needed to define the Universal Transverse Mercator (UTM)
 *   2D Spatial Reference Frame (SRF).  See the Spatial Reference Model for
 *   details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;

    /* Origin Displacement of Coordinate System */
    SE_FLOAT64 x_offset; /* origin relative displacement in X */
    SE_FLOAT64 y_offset; /* origin relative displacement in Y */

    SE_PINT8 zone; /* between 1 and 60, inclusive */
} SRM_UTM_2D_PARAMETERS;


/*
 * STRUCT: SRM_UTM_3D_PARAMETERS
 *
 *   The parameters needed to define the Augmented Universal Transverse
 *   Mercator (AUTM) 3D Spatial Reference Frame (SRF).  See the Spatial
 *   Reference Model for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;
    SE_VERTICAL_DATUM_ENUM   vertical_datum;

    /* Origin Displacement of Coordinate System */
    SE_FLOAT64               x_offset; /* origin relative displacement in x */
    SE_FLOAT64               y_offset; /* origin relative displacement in y */

    SRM_ELEVATION_UNITS_ENUM z_units; /* Spatial Reference Frame Units */

    SE_PINT8 zone; /* between 1 and 60, inclusive */
} SRM_UTM_3D_PARAMETERS;


/*
 * ENUM: SE_HEMISPHERE_ENUM
 *
 *   Used to define the location of the proper hemisphere for UTM and AUTM
 *   coordinates.
 */
typedef enum
{
    SE_NORTHERN_HEMISPHERE,
    SE_SOUTHERN_HEMISPHERE
} SE_HEMISPHERE_ENUM;


/*
 * STRUCT: SRM_UTM_2D_COORD
 *
 *   A coordinate within the Universal Transverse Mercator (UTM) 2D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_2D union.  Parallels the definition of the
 *   fields for the <UTM Location 2D> class.
 */
typedef struct
{
    SE_HEMISPHERE_ENUM hemisphere;
    SE_FLOAT64         x; /* in meters; positive eastward */
    SE_FLOAT64         y; /* in meters; positive northward */
} SRM_UTM_2D_COORD;


/*
 * STRUCT: SRM_UTM_3D_COORD
 *
 *   A coordinate within the Augmented Universal Transverse Mercator (AUTM) 3D
 *   Spatial Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition of the
 *   fields for the <UTM Location 3D> class.
 */
typedef struct
{
    SE_HEMISPHERE_ENUM hemisphere;
    SE_FLOAT64         x;    /* in meters; positive Eastward     */
    SE_FLOAT64         y;    /* in meters; positive Northward    */
    SE_FLOAT64         z;    /* Elevation or height;             */
                             /* positive along xy surface normal */
} SRM_UTM_3D_COORD;


/*
 * STRUCT: SRM_LTP_2D_PARAMETERS
 *
 *  The parameters needed to define the Local Tangent Plane (LTP2) 2D Spatial
 *  Reference Frame (SRF). See the Spatial Reference Model for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;

    /* Origin of Spatial Reference Frame (center) */
    SE_FLOAT64 geodetic_longitude; /* in degrees */
    SE_FLOAT64 geodetic_latitude;  /* in degrees */

    SE_FLOAT64 azimuth;
              /*
               * angle, in arc degrees, positive clockwise, from geographic
               * north to the direction of the positive Y axis
               */

    /* Origin Displacement of Coordinate System */
    SE_FLOAT64 x_offset; /* origin relative displacement in X */
    SE_FLOAT64 y_offset; /* origin relative displacement in Y */
} SRM_LTP_2D_PARAMETERS;


/*
 * STRUCT: SRM_LTP_3D_PARAMETERS
 *
 *   The parameters needed to define the Local Tangent Plane (LTP) 3D Spatial
 *   Reference Frame (SRF). See the Spatial Reference Model for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_HORIZONTAL_DATUM_ENUM horizontal_datum;
    SE_VERTICAL_DATUM_ENUM   vertical_datum;

    /* Origin of Spatial Reference Frame (center) */
    SE_FLOAT64 geodetic_longitude; /* in degrees */
    SE_FLOAT64 geodetic_latitude;  /* in degrees */

    SE_FLOAT64 azimuth;
              /*
               * angle, in arc degrees, positive clockwise, from geographic
               * north to the direction of the positive Y axis
               */

    /* Origin Displacement of Coordinate System */
    SE_FLOAT64 x_offset; /* origin relative displacement in X */
    SE_FLOAT64 y_offset; /* origin relative displacement in Y */

    SRM_ELEVATION_UNITS_ENUM z_units; /* Spatial Reference Frame Units */
} SRM_LTP_3D_PARAMETERS;


/*
 * STRUCT: SRM_LTP_2D_COORD
 *
 *   A coordinate within the Local Tangent Plane (LTP2) 2D Spatial Reference
 *   Frame (SRF).
 *
 *   Only used within the SE_COORD_2D union. Parallels the definition of the
 *   fields for the <LTP Location 2D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters */
    SE_FLOAT64 y; /* in meters */
} SRM_LTP_2D_COORD;


/*
 * STRUCT: SRM_LTP_3D_COORD
 *
 *   A coordinate within the Local Tangent Plane (LTP) 3D Spatial Reference
 *   Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union. Parallels the definition of the
 *   fields for the <LTP Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters */
    SE_FLOAT64 y; /* in meters */
    SE_FLOAT64 z; /* Elevation or height; positive along xy surface normal */
} SRM_LTP_3D_COORD;


/*
 * ENUM: SE_DIRECTION_OF_UP_ENUM
 *
 *   Used to define the LSR Spatial Reference Frame.
 */
typedef enum
{
    SE_DIRECTION_OF_UP_POSITIVE_PRIMARY_AXIS,
    SE_DIRECTION_OF_UP_POSITIVE_SECONDARY_AXIS,
    SE_DIRECTION_OF_UP_POSITIVE_TERTIARY_AXIS,
    SE_DIRECTION_OF_UP_NEGATIVE_PRIMARY_AXIS,
    SE_DIRECTION_OF_UP_NEGATIVE_SECONDARY_AXIS,
    SE_DIRECTION_OF_UP_NEGATIVE_TERTIARY_AXIS
} SE_DIRECTION_OF_UP_ENUM;


/*
 * ENUM: SE_DIRECTION_OF_FORWARD_ENUM
 *
 *   Used to define the LSR and LSR2 Spatial Reference Frames.
 */
typedef enum
{
    SE_DIRECTION_OF_FORWARD_POSITIVE_PRIMARY_AXIS,
    SE_DIRECTION_OF_FORWARD_POSITIVE_SECONDARY_AXIS,
    SE_DIRECTION_OF_FORWARD_POSITIVE_TERTIARY_AXIS,
    SE_DIRECTION_OF_FORWARD_NEGATIVE_PRIMARY_AXIS,
    SE_DIRECTION_OF_FORWARD_NEGATIVE_SECONDARY_AXIS,
    SE_DIRECTION_OF_FORWARD_NEGATIVE_TERTIARY_AXIS
} SE_DIRECTION_OF_FORWARD_ENUM;


/*
 * STRUCT: SRM_LSR_2D_PARAMETERS
 *
 *   The parameters needed to define the Local Space Rectangular (LSR)
 *   2D Spatial Reference Frame (SRF). See the Spatial Reference Model
 *   for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_DIRECTION_OF_FORWARD_ENUM forward_direction;
} SRM_LSR_2D_PARAMETERS;


/*
 * STRUCT: SRM_LSR_3D_PARAMETERS
 *
 *   The parameters needed to define the Local Space Rectangular (LSR)
 *   3D Spatial Reference Frame (SRF). See the Spatial Reference Model
 *   for details.
 */
typedef struct
{
    /* Specification of Object Reference Model (ORM) */
    SE_DIRECTION_OF_UP_ENUM      up_direction;
    SE_DIRECTION_OF_FORWARD_ENUM forward_direction;
} SRM_LSR_3D_PARAMETERS;


/*
 * STRUCT: SRM_LSR_2D_COORD
 *
 *   A coordinate within the Local Space Rectangular (LSR2) 2D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_2D union.  Parallels the definition of the
 *   fields for the <LSR Location 2D> class.
 */
typedef struct
{
    SE_FLOAT64 x; /* in meters, unless used in unitless Model */
    SE_FLOAT64 y; /* in meters, unless used in unitless Model */
} SRM_LSR_2D_COORD;


/*
 * STRUCT: SRM_LSR_3D_COORD
 *
 *   A coordinate within the Local Space Rectangular (LSR) 3D Spatial
 *   Reference Frame (SRF).
 *
 *   Only used within the SE_COORD_3D union.  Parallels the definition
 *   of the fields for the <LSR Location 3D> class.
 */
typedef struct
{
    SE_FLOAT64 x;    /* in meters, unless used in unitless Model*/
    SE_FLOAT64 y;    /* in meters, unless used in unitless Model*/
    SE_FLOAT64 z;    /* in meters, unless used in unitless Model;
                      * positive along xy surface normal
                      */
} SRM_LSR_3D_COORD;


/*
 * STRUCT: SRM_SRF_2D_PARAMETERS
 *
 *   All parameters needed to define any 2D spatial reference frame within the
 *   SRM.
 */
typedef struct
{
    SRM_SRF_2D_ENUM spatial_reference_frame;
    union
    {
        SRM_GD_2D_PARAMETERS  gd_parameters;
                             /* Geodetic (2D) SRF */

        SRM_EC_2D_PARAMETERS  ec_parameters;
                             /* Equidistant Cylindrical (2D) SRF */

        SRM_PS_2D_PARAMETERS  ps_parameters;
                             /* Polar Stereographic (2D) SRF */

        SRM_LCC_2D_PARAMETERS lcc_parameters;
                             /* Lambert Conformal Conic (2D) SRF */

        SRM_TM_2D_PARAMETERS  tm_parameters;
                             /* Transverse Mercator (2D) SRF */

        SRM_UTM_2D_PARAMETERS utm_parameters;
                             /*
                              * Universal Transverse Mercator (2D) SRF
                              */

        SRM_LTP_2D_PARAMETERS ltp_parameters;
                             /* Local Tangent Plane (2D) SRF */

        SRM_LSR_2D_PARAMETERS lsr_parameters;
                             /* Local Space Rectangular (2D) SRF */

        SRM_M_2D_PARAMETERS   m_parameters;
                             /* Mercator (2D) SRF */

        SRM_OM_2D_PARAMETERS  om_parameters;
                             /* Oblique Mercator (2D) SRF */

        SRM_UPS_2D_PARAMETERS ups_parameters;
                             /*
                              * Universal Polar Stereographic (2D) SRF
                              */
    } u;
} SRM_SRF_2D_PARAMETERS;


/*
 * STRUCT: SRM_SRF_3D_PARAMETERS
 *
 *   All parameters needed to define any 3D spatial reference frame within the
 *   SRM.
 */
typedef struct
{
    SRM_SRF_3D_ENUM spatial_reference_frame;
    union
    {
        SRM_GD_3D_PARAMETERS  gd_parameters;
                             /* Geodetic (3D) SRF */

        SRM_GC_3D_PARAMETERS  gc_parameters;
                             /* Geocentric (3D) SRF */

        SRM_GM_3D_PARAMETERS  gm_parameters;
                             /* Geomagnetic (3D) SRF */

        SRM_GEI_3D_PARAMETERS  gei_parameters;
                              /* Geocentric Equatorial Inertial (3D) SRF */

        SRM_GSE_3D_PARAMETERS  gse_parameters;
                              /* Geocentric Solar Ecliptic (3D) SRF */

        SRM_GSM_3D_PARAMETERS  gsm_parameters;
                              /* Geocentric Solar Magnetospheric (3D) SRF */

        SRM_SM_3D_PARAMETERS   sm_parameters;
                              /* Solar Magnetic (3D) SRF */

        SRM_GCS_3D_PARAMETERS  gcs_parameters;
                              /* "Global Coordinate System" (3D) SRF */

        SRM_EC_3D_PARAMETERS   ec_parameters;
                              /* Augmented Equidistant Cylindrical (3D) SRF */

        SRM_PS_3D_PARAMETERS   ps_parameters;
                              /* Augmented Polar Stereographic (3D) SRF */

        SRM_LCC_3D_PARAMETERS  lcc_parameters;
                              /* Augmented Lambert Conformal Conic (3D) SRF */

        SRM_TM_3D_PARAMETERS   tm_parameters;
                              /* Augmented Transverse Mercator (3D) SRF */

        SRM_UTM_3D_PARAMETERS  utm_parameters;
                              /*
                               * Augmented Universal Transverse Mercator (3D)
                               * SRF
                               */

        SRM_LTP_3D_PARAMETERS  ltp_parameters;
                              /* Local Tangent Plane (3D) SRF */

        SRM_LSR_3D_PARAMETERS  lsr_parameters;
                              /* Local Space Rectangular (3D) SRF */

        SRM_M_3D_PARAMETERS    m_parameters;
                              /* Augmented Mercator (3D) SRF */

        SRM_OM_3D_PARAMETERS   om_parameters;
                              /* Augmented Oblique Mercator (3D) SRF */

        SRM_UPS_3D_PARAMETERS  ups_parameters;
                              /*
                               * Augmented Universal Polar Stereographic (3D)
                               * SRF
                               */
    } u;
} SRM_SRF_3D_PARAMETERS;


/*
 * STRUCT: SE_COORD_2D
 *
 *   A 'generic' 2D coordinate.
 */
typedef struct
{
    SRM_SRF_2D_ENUM  spatial_reference_frame;
    union
    {
        SRM_GD_2D_COORD  gd_coord;
        SRM_EC_2D_COORD  ec_coord;
        SRM_PS_2D_COORD  ps_coord;
        SRM_LCC_2D_COORD lcc_coord;
        SRM_TM_2D_COORD  tm_coord;
        SRM_UTM_2D_COORD utm_coord;
        SRM_LTP_2D_COORD ltp_coord;
        SRM_LSR_2D_COORD lsr_coord;
        SRM_M_2D_COORD   m_coord;
        SRM_OM_2D_COORD  om_coord;
        SRM_UPS_2D_COORD ups_coord;
    } u;
} SE_COORD_2D;


/*
 * STRUCT: SE_COORD_3D
 *
 *   A 'generic' 3D coordinate.
 */
typedef struct
{
    SRM_SRF_3D_ENUM  spatial_reference_frame;

    union
    {
        SRM_GD_3D_COORD   gd_coord;
        SRM_GC_3D_COORD   gc_coord;
        SRM_GM_3D_COORD   gm_coord;
        SRM_GEI_3D_COORD  gei_coord;
        SRM_GSE_3D_COORD  gse_coord;
        SRM_GSM_3D_COORD  gsm_coord;
        SRM_SM_3D_COORD   sm_coord;
        SRM_GCS_3D_COORD  gcs_coord;
        SRM_EC_3D_COORD   ec_coord;
        SRM_PS_3D_COORD   ps_coord;
        SRM_LCC_3D_COORD  lcc_coord;
        SRM_TM_3D_COORD   tm_coord;
        SRM_UTM_3D_COORD  utm_coord;
        SRM_LTP_3D_COORD  ltp_coord;
        SRM_LSR_3D_COORD  lsr_coord;
        SRM_M_3D_COORD    m_coord;
        SRM_OM_3D_COORD   om_coord;
        SRM_UPS_3D_COORD  ups_coord;
    } u;
} SE_COORD_3D;


/*
 * STRUCT: SE_COORD
 *
 *   A 'generic' coordinate.
 */
typedef struct
{
    SE_BOOLEAN is_2D;
    union
    {
        SE_COORD_3D coord_3d;
        SE_COORD_2D coord_2d;
    } u;
} SE_COORD;


/*
 * STRUCT: SE_SRF_PARAMETERS
 *
 *   A 'generic' set of spatial reference frame (SRF) parameters.
 */
typedef struct
{
    SE_BOOLEAN is_2D;
    union
    {
        SRM_SRF_3D_PARAMETERS params_3d;
        SRM_SRF_2D_PARAMETERS params_2d;
    } u;
} SE_SRF_PARAMETERS;


/*
 * STRUCT: SE_VECTOR_3_TYPE
 *
 *   Type definition for a 3 vector
 */
typedef struct
{
    SE_FLOAT64 vector[3];
} SE_VECTOR_3_TYPE;


/*
 * STRUCT: SE_MATRIX_3X3_TYPE
 *
 *   Type definition for 3X3 matrix used to orient models, grids, etc.
 *   in the currently scoped 'world' spatial reference frame.
 */
typedef struct
{
    SE_FLOAT64 mat[3][3];
} SE_MATRIX_3X3_TYPE;


/*
 * STRUCT: SE_MATRIX_4X4_TYPE
 *
 *   Type definition for 4X4 matrix used to orient and locate models,
 *   grids, etc. in the currently scoped 'world' spatial reference frame.
 */
typedef struct
{
    SE_FLOAT64 mat[4][4];
} SE_MATRIX_4X4_TYPE;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef/define/endif for SE_CONVERSIONS_API_TYPES_H_INCLUDED */
