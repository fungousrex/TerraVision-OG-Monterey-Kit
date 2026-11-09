/*
 * SEDRIS Conversions API
 *
 * FILE       : se_coord_conv.c
 *
 * PROGRAMMERS: Bill Horan (SAIC), Howard Lu (SAIC),
 *              Dan Toms (SRI International)
 *
 * DESCRIPTION: provide reusable functions for conversions between:
 *                - coordinate systems (numerous)
 *
 * Release 3.0.3 - April 20, 2001
 */

/*
 *                              NOTICE
 *
 * This software is provided openly and freely for use in representing and
 * interchanging environmental data & databases.
 *
 * This software was developed for use by the United States Government with
 * unlimited rights.  The software was developed for the U.S. Army (STRICOM)
 * under a contract with the Naval Air Warfare Center Training Systems
 * Division, contract N61339-98-C-0024  by SRI International.  The software is
 * unclassified and is deemed as Distribution A, approved for
 * Public Release.
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
#ifdef WIN32
#define SE_TURN_OFF_CONVERSIONS_WIN32_IMPORT
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "se_conv.h"
#include "se_datums.h"

#ifdef WIN32
#undef SE_TURN_OFF_CONVERSIONS_WIN32_IMPORT
#endif

#ifdef CONV_DEBUG
/*
 * ENUM: SE_CONVERSION_ERROR_ENUM
 *
 *   Used to index the conversion error messages in conv_error_msg.
 */
typedef enum
{
    SE_UTM_SOURCE_COORD_ERROR,
    SE_INVALID_CONVERSION_ERROR,
    SE_NOT_SUPPORTED_CONVERSION_ERROR,
    SE_GC2GD_ERROR,
    SE_GD2PS_ERROR,
    SE_GEI2GEI_EPOCH_ERROR,
    SE_OM2OM_ERROR,
    SE_UPS2UPS_ERROR,
    SE_GD2GD_ERROR,
    SE_LTP2LTP_ERROR,
    SE_LSR2LSR_AXIS_ERROR
} SE_CONVERSION_ERROR_ENUM;

/*
 * GLOBAL VARIABLE: conv_error_msg
 *
 *   Used for frequently used conversion error messages; indexed
 *   by the enumerators of SE_CONVERSION_ERROR_ENUM.
 */
static char *conv_error_msg[] =
{
    "[SEDRIS CONVERSIONS] UTM Original Coordinate Error: ",
    "[SEDRIS CONVERSIONS] Internal conversion error, invalid "\
        "conversion type %d",
    "[SEDRIS CONVERSIONS] Error: Conversion from SRF %s to SRF %s "\
        "not supported.\n",
    "[SEDRIS CONVERSIONS] Error: GC to GD ",
    "[SEDRIS CONVERSIONS] GD to PS Error: GD latitude %f out of bounds\n"\
        "  must be -80.0>=source_lat (south pole) or\n"\
        "  source_lat>=84.0) (north pole)\n",
   "[SEDRIS_CONVERSIONS] Error: "\
       "attempt to convert GEI->GEI with different epochs)\n",
   "[SEDRIS_CONVERSIONS] Error: "\
       "attempt to convert OM->OM with different "\
       "parameters (other than z_units or offsets)\n",
   "[SEDRIS_CONVERSIONS] Error: "\
       "attempt to convert UPS->UPS with different "\
       "parameters (other than z_units)\n",
   "[SEDRIS_CONVERSIONS] Error: attempt to convert GD->GD"\
       " with different hdata and/or vdata\n",
   "[SEDRIS_CONVERSIONS] Error: "\
       "attempt to convert LTP->LTP with different"\
       " parameters (other than z_units or offsets)\n",
   "[SEDRIS_CONVERSIONS] Error: attempt to "\
       "convert LSR->LSR with different axes)\n"
};
#endif

#define SE_TM_EXTENDED_LONGITUDE_RANGE 3.5
#define SE_TM_LONGITUDE_RANGE 3.0

/*
 * MACRO: STATUS_CODE_IS_GOOD
 *
 *   Checks whether a coord status code is valid (i.e., does
 *   not indicate an error condition.)
 */
#define STATUS_CODE_IS_GOOD(status)                       \
    (status == SE_COORD_SUCCESS)                   ||  \
    (status == SE_COORD_EXTENDED_SOURCE_COORD)     ||  \
    (status == SE_COORD_EXTENDED_DEST_COORD)

/*
 * STATIC POINTER: SE_SEPARATION_TABLE
 *
 *   Used to hold geoidal separation table, set to NULL
 *   if the table is not loaded
 */
static SE_FLOAT32 *SE_separation_table = NULL;

/*
 * STRUCT: SE_ERM_DATA
 *
 *   Used to hold precomputed constants derived from a given ERM
 */
typedef struct
{
    SE_FLOAT64 A,
               B,
               F,
               C,
               Eps2,
               Eps,
               Epps2,
               EpsH,
               Eps25,
               A2,
               C2;
} SE_ERM_DATA;

#define SE_FIX_LONGITUDE(lon)  if (lon>SE_PI) lon =lon - 2*SE_PI;            \
                               else if (lon < -SE_PI) lon = lon +2*SE_PI
/*
 * STRUCT: SE_EC_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to an EC transformation
 */
typedef struct
{
    SE_FLOAT64 x_offset, y_offset,
               latitude_origin, longitude_origin,scaled_radius;
} SE_EC_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_OM_OM_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to an OM to OM transformation
 */
typedef struct
{
    SE_FLOAT64 x_offset, y_offset;
} SE_OM_OM_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GD_M_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a GD to M (mercator)
 *   transformation
 */
typedef struct
{
    SE_FLOAT64 CR11, CR12, CR13, CR14, CR15, CR21, CR22, CR23, CR24, CR25,
               scale,
               x_offset,
               y_offset,
               longitude_origin;
} SE_GD_M_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_M_GD_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a M to GD transformation.
 */
typedef struct
{
    SE_FLOAT64 b1, b2, b3, b4, longitude_origin,x_offset,y_offset,scale;
} SE_M_GD_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GC_GD_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a GC to GD transformation
 */
typedef struct
{
    SE_FLOAT64 B1, B2, B3, B4, B5, Eps21,C254,CEEps2,CEE,TwoCEE,C2DA,tem,ARat1,ARat2,BRat1,BRat2;
} SE_GC_GD_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GD_GC_SPECIFIC_CONSTANTS
 *
 *   No precomputed constants are needed to convert from GD
 *   to GC in SEDRIS, but a struct is declared for the conversion
 *   anyway, to parallel the struct definitions of the other systems.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SE_GD_GC_SPECIFIC_CONSTANTS;

/*
 * STRUCT: SE_GD_LCC1_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a GC to LCC1
 *   transformation
 */
typedef struct
{
    SE_FLOAT64 CR11, CR12, CR13, CR14, CR15, CR21, CR22, CR23, CR24, CR25,
               xlonz, xl, pz, rz, rho_origin,x_offset,y_offset;
    SE_HEMISPHERE_ENUM hemisphere;
} SE_GD_LCC1_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_LCC1_GD_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a LCC1 to GD
 *   transformation
 */
typedef struct
{
    SE_FLOAT64 b1, b2, b3, b4, xlonz, xl, pz, rz, rz2, ex,rho_origin,x_offset,y_offset;
} SE_LCC1_GD_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GD_LCC2_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a GD to LCC2
 *   transformation
 */
typedef struct
{
    SE_FLOAT64 CR11, CR12, CR13, CR14, CR15, CR21, CR22, CR23, CR24, CR25,
               xlonz, rx, rz, p1, xl,x_offset,y_offset;
    SE_HEMISPHERE_ENUM hemisphere;
} SE_GD_LCC2_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_LCC2_GD_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a LCC2 to GD
 *   transformation
 */
typedef struct
{
    SE_FLOAT64 b1, b2, b3, b4, xlonz, rz, xl, ex, rz2, p1,x_offset,y_offset;
} SE_LCC2_GD_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_PS_GD_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a PS to GD transformation
 */
typedef struct
{
    SE_FLOAT64      b1, b2, b3, b4, xk,x_offset,y_offset;
    SE_PS_POLE_ENUM pole;
} SE_PS_GD_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GD_PS_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a PS to GD transformation
 */
typedef struct
{
    SE_FLOAT64      CR11, CR12, CR13, CR14, CR15, CR21, CR22, CR23, CR24, CR25,
                    xk,x_offset,y_offset;
    SE_PS_POLE_ENUM pole;
} SE_GD_PS_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GD_UTM_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a GD to UTM transformation
 */
typedef struct
{
    SE_FLOAT64 Cdb6,
               Cdb24,
               Cdb120,
               Cdb720,
               scale,
               poly1b,
               poly2b,
               poly3b,
               poly4b,
               poly5b,
               x_offset,
               y_offset;
    SE_PINT8   zone;
} SE_GD_UTM_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GD_TM_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a GD to TM transformation
 */
typedef struct
{
    SE_FLOAT64 Cdb6,
               Cdb24,
               Cdb120,
               Cdb720,
               CScale,
               poly1b,
               poly2b,
               poly3b,
               poly4b,
               poly5b,
               x_offset,
               y_offset,
               longitude_origin,
               smz;
} SE_GD_TM_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_TM_GD_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a TM to GD transformation
 */
typedef struct
{
    SE_FLOAT64 Con2,
               EF,
               Con6,
               Con24,
               Con120,
               Con720,
               CScale,
               polx2b,
               polx3b,
               polx4b,
               polx5b,
               conap,
               x_offset,
               y_offset,
               longitude_origin,
               smz;
} SE_TM_GD_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_UTM_GD_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a UTM to GD transformation
 */
typedef struct
{
    SE_FLOAT64 Con2,
               EF,
               Con6,
               Con24,
               Con120,
               Con720,
               CScale,
               Cnst3,
               polx2b,
               polx3b,
               polx4b,
               polx5b,
               polx6b,
               polx8b,
               conap,
               x_offset,
               y_offset;
    SE_PINT8   zone;
} SE_UTM_GD_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GC_GC_SPECIFIC_CONSTANTS
 *
 *   No parameters are needed to define the GeoCentric Coordinate (GC)
 *   System in SEDRIS, but a struct is declared for the conversion
 *   anyway, to parallel the struct definitions of the other systems.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SE_GC_GC_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GM_GM_SPECIFIC_CONSTANTS
 *
 *   No parameters are needed to define the GeoCentric Coordinate (GM)
 *   System in SEDRIS, but a struct is declared for the conversion
 *   anyway, to parallel the struct definitions of the other systems.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SE_GM_GM_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GD_GD_SPECIFIC_CONSTANTS
 *
 *   Currently, SE_GD_3D_PARAMETERS must match except for their distance_units
 *   for a legal conversion, but a struct is declared for the conversion
 *   anyway, to parallel the struct definitions of the other systems.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SE_GD_GD_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GCS_GCS_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a GCS to GCS transformation
 */
typedef struct
{
    SE_FLOAT64 x_offset, y_offset;
} SE_GCS_GCS_SPECIFIC_CONSTANTS;

 /*
 * STRUCT: SE_UPS_UPS_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to a UPS to UPS transformation
 */
typedef struct
{
    SE_FLOAT64 x_offset, y_offset;
} SE_UPS_UPS_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GEI_GEI_SPECIFIC_CONSTANTS
 *
 *   Currently, SE_GEI_3D_PARAMETERS must match for a legal
 *   conversion, but a struct is declared for the conversion
 *   anyway, to parallel the struct definitions of the other
 *   systems.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SE_GEI_GEI_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GSE_GSE_SPECIFIC_CONSTANTS
 *
 *   No parameters are needed to define the Geocentric Solar Ecliptic
 *   (GSE) Coordinate System in SEDRIS, but a struct is declared for
 *   the conversion anyway, to parallel the struct definitions of the
 *   other systems.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SE_GSE_GSE_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_GSM_GSM_SPECIFIC_CONSTANTS
 *
 *   No parameters are needed to define the Geocentric Solar Magnetospheric
 *   (GSM) Coordinate System in SEDRIS, but a struct is declared for
 *   the conversion anyway, to parallel the struct definitions of the
 *   other systems.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SE_GSM_GSM_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_SM_SM_SPECIFIC_CONSTANTS
 *
 *   No parameters are needed to define the Solar Magnetic (SM)
 *   Coordinate System in SEDRIS, but a struct is declared for
 *   the conversion anyway, to parallel the struct definitions of the
 *   other systems.
 */
typedef struct
{
    SE_INT16 ignore_na;
} SE_SM_SM_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_LSR_LSR_SPECIFIC_CONSTANTS
 *
 *   Currently, SE_LSR_3D_PARAMETERS must match for a legal
 *   conversion, but a struct is declared for the conversion
 *   anyway, to parallel the struct definitions of the other
 *   systems.
 */
typedef struct
{
    SE_MATRIX_3X3_TYPE flip_matrix;
} SE_LSR_LSR_SPECIFIC_CONSTANTS;


/*
 * STRUCT: SE_LTP_SPECIFIC_CONSTANTS
 *
 *   Used to hold precomputed constants specific to transformations
 *   involving LTP.
 */
typedef struct
{
    SE_FLOAT64 x_offset, y_offset, Sin_Origin_Lat, Cos_Origin_Lat,
               Sin_Origin_Lon, Cos_Origin_Lon,Azimuth,
               Sin_Lat_Sin_Azimuth, Cos_Azimuth,
               Cos_Lat_Sin_Azimuth,Sin_Lat_Cos_Azimuth,
               Sin_Azimuth,Cos_Lat_Cos_Azimuth,U0,W0,V0;
} SE_LTP_SPECIFIC_CONSTANTS;


#define MINIMUM(a, b) ((a) < (b)) ? (a) : (b)
#define MAXIMUM(a, b) ((a) > (b)) ? (a) : (b)

/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: lsr_forward_flip
 *
 *   Handles the 2D cases, and is used to initialize some of the 3D
 *   cases (those where the up_direction for both SRFs is on the
 *   tertiary axis).
 *
 *----------------------------------------------------------------------------
 */
static void
lsr_forward_flip
(
    SE_DIRECTION_OF_FORWARD_ENUM   src_dir,
    SE_DIRECTION_OF_FORWARD_ENUM   dest_dir,
    SE_LSR_LSR_SPECIFIC_CONSTANTS *lsr_spec
)
{
    lsr_spec->flip_matrix.mat[0][0] = 0.0;
    lsr_spec->flip_matrix.mat[0][1] = 0.0;
    lsr_spec->flip_matrix.mat[0][2] = 0.0;
    lsr_spec->flip_matrix.mat[1][0] = 0.0;
    lsr_spec->flip_matrix.mat[1][1] = 0.0;
    lsr_spec->flip_matrix.mat[1][2] = 0.0;
    lsr_spec->flip_matrix.mat[2][0] = 0.0;
    lsr_spec->flip_matrix.mat[2][1] = 0.0;
    lsr_spec->flip_matrix.mat[2][2] = 0.0;

    if (src_dir == dest_dir)
    {
        /*
         * Identity -- dest x == src x,
         *             dest y == src y
         */
         lsr_spec->flip_matrix.mat[0][0] = 1.0;
         lsr_spec->flip_matrix.mat[1][1] = 1.0;
    }
    else if (((src_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_PRIMARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_PRIMARY_AXIS))
             ||
             ((src_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_PRIMARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_PRIMARY_AXIS)))
    {
        /*
         * Flip the sign on x, y stays the same
         */
         lsr_spec->flip_matrix.mat[0][0] = -1.0;
         lsr_spec->flip_matrix.mat[1][1] =  1.0;
    }
    else if (((src_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_SECONDARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_SECONDARY_AXIS))
             ||
             ((src_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_SECONDARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_SECONDARY_AXIS)))
    {
        /*
         * Flip the sign on y, x stays the same
         */
         lsr_spec->flip_matrix.mat[0][0] =  1.0;
         lsr_spec->flip_matrix.mat[1][1] = -1.0;
    }
    else if (((src_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_PRIMARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_SECONDARY_AXIS))
             ||
             ((src_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_PRIMARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_SECONDARY_AXIS))
             ||
             ((src_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_SECONDARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_PRIMARY_AXIS))
             ||
             ((src_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_SECONDARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_PRIMARY_AXIS)))
    {
        /*
         * Transpose x, y without sign changes
         */
         lsr_spec->flip_matrix.mat[0][1] = 1.0;
         lsr_spec->flip_matrix.mat[1][0] = 1.0;
    }
    else if (((src_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_PRIMARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_SECONDARY_AXIS))
             ||
             ((src_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_PRIMARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_SECONDARY_AXIS)))
    {
        /*
         * Transpose x, y, changing sign on x
         *    dest x =   src y
         *    dest y = - src x
         */
         lsr_spec->flip_matrix.mat[0][1] =   1.0;
         lsr_spec->flip_matrix.mat[1][0] = - 1.0;
    }
    else if (((src_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_SECONDARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_PRIMARY_AXIS))
             ||
             ((src_dir == SE_DIRECTION_OF_FORWARD_NEGATIVE_SECONDARY_AXIS) &&
              (dest_dir == SE_DIRECTION_OF_FORWARD_POSITIVE_PRIMARY_AXIS)))
    {
        /*
         * Transpose x, y, changing sign on y
         *    dest x = - src y
         *    dest y =   src x
         */
         lsr_spec->flip_matrix.mat[0][1] = - 1.0;
         lsr_spec->flip_matrix.mat[1][0] =   1.0;
    }
} /* end lsr_forward_flip */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_lsr_constants
 *
 *----------------------------------------------------------------------------
 */
static SE_COORD_STATUS_CODE_ENUM
set_lsr_constants
(
    const SE_SRF_PARAMETERS             *src_srf_ptr,
    const SE_SRF_PARAMETERS             *dest_srf_ptr,
          SE_LSR_LSR_SPECIFIC_CONSTANTS *lsr_spec
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (src_srf_ptr->is_2D && dest_srf_ptr->is_2D)
    {
        lsr_forward_flip
        (
            src_srf_ptr->u.params_2d.u.lsr_parameters.forward_direction,
            dest_srf_ptr->u.params_2d.u.lsr_parameters.forward_direction,
            lsr_spec
        );
    }
    else if (!src_srf_ptr->is_2D && !dest_srf_ptr->is_2D)
    {
        if (((src_srf_ptr->u.params_3d.u.lsr_parameters.up_direction ==
                  SE_DIRECTION_OF_UP_POSITIVE_TERTIARY_AXIS) &&
             (dest_srf_ptr->u.params_3d.u.lsr_parameters.up_direction ==
                  SE_DIRECTION_OF_UP_POSITIVE_TERTIARY_AXIS))
            ||
            ((src_srf_ptr->u.params_3d.u.lsr_parameters.up_direction ==
                  SE_DIRECTION_OF_UP_NEGATIVE_TERTIARY_AXIS) &&
             (dest_srf_ptr->u.params_3d.u.lsr_parameters.up_direction ==
                  SE_DIRECTION_OF_UP_NEGATIVE_TERTIARY_AXIS)))
        {
            lsr_forward_flip
            (
                src_srf_ptr->u.params_3d.u.lsr_parameters.forward_direction,
                dest_srf_ptr->u.params_3d.u.lsr_parameters.forward_direction,
                lsr_spec
            );
            lsr_spec->flip_matrix.mat[2][2] = 1.0;
        }
        else if (((src_srf_ptr->u.params_3d.u.lsr_parameters.up_direction ==
                  SE_DIRECTION_OF_UP_POSITIVE_TERTIARY_AXIS) &&
                 (dest_srf_ptr->u.params_3d.u.lsr_parameters.up_direction ==
                  SE_DIRECTION_OF_UP_NEGATIVE_TERTIARY_AXIS))
            ||
                 ((src_srf_ptr->u.params_3d.u.lsr_parameters.up_direction ==
                  SE_DIRECTION_OF_UP_NEGATIVE_TERTIARY_AXIS) &&
                  (dest_srf_ptr->u.params_3d.u.lsr_parameters.up_direction ==
                  SE_DIRECTION_OF_UP_POSITIVE_TERTIARY_AXIS)))
        {
            lsr_forward_flip
            (
                src_srf_ptr->u.params_3d.u.lsr_parameters.forward_direction,
                dest_srf_ptr->u.params_3d.u.lsr_parameters.forward_direction,
                lsr_spec
            );
            lsr_spec->flip_matrix.mat[2][2] = -1.0;
        }
        else
        {
            status = SE_COORD_UNSUPPORTED; /* $$$ revisit */
        }
    }
    else
    {
        status = SE_COORD_UNSUPPORTED; /* $$$ revisit */
    }
    return status;
} /* end set_lsr_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_ltp_constants
 *
 *   Computes and stores the constants used for the conversions involving LTP:
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   ltp - a pointer to the parameters of a (destination) LTP system
 *
 *   ltp_spec - a pointer to where the constants will be stored
 *              for the specific conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the ltp_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void
set_ltp_constants
(
    const SE_ERM_DATA               *e_constants,
    const SE_SRF_PARAMETERS         *srf,
          SE_LTP_SPECIFIC_CONSTANTS *ltp_spec
)
{
   SE_FLOAT64 Origin_Height,temp,r,rn;

   SE_FLOAT64  geodetic_longitude,geodetic_latitude,azimuth,x_offset,y_offset;

   if(srf->is_2D == SE_TRUE)  /* get 2d parameter values  */
   {
      geodetic_longitude = srf->u.params_2d.u.ltp_parameters.geodetic_longitude;
      geodetic_latitude  = srf->u.params_2d.u.ltp_parameters.geodetic_latitude;
      azimuth            = srf->u.params_2d.u.ltp_parameters.azimuth;
      x_offset           = srf->u.params_2d.u.ltp_parameters.x_offset;
      y_offset           = srf->u.params_2d.u.ltp_parameters.y_offset;
   }
   else                      /* get 3d parameters values */
   {
      geodetic_longitude = srf->u.params_3d.u.ltp_parameters.geodetic_longitude;
      geodetic_latitude  = srf->u.params_3d.u.ltp_parameters.geodetic_latitude;
      azimuth            = srf->u.params_3d.u.ltp_parameters.azimuth;
      x_offset           = srf->u.params_3d.u.ltp_parameters.x_offset;
      y_offset           = srf->u.params_3d.u.ltp_parameters.y_offset;
   }

   Origin_Height = 0.0;


   ltp_spec->Azimuth = azimuth * SE_RADIANS_PER_DEGREE;

   if (ltp_spec->Azimuth > SE_PI)
       ltp_spec->Azimuth -= SE_PI*2;

   ltp_spec->x_offset = x_offset;
   ltp_spec->y_offset = y_offset;

   ltp_spec->Cos_Origin_Lon = cos(geodetic_longitude * SE_RADIANS_PER_DEGREE);
   ltp_spec->Sin_Origin_Lon = sin(geodetic_longitude * SE_RADIANS_PER_DEGREE);
   ltp_spec->Sin_Origin_Lat = sin(geodetic_latitude * SE_RADIANS_PER_DEGREE);
   ltp_spec->Cos_Origin_Lat = cos(geodetic_latitude * SE_RADIANS_PER_DEGREE);
   ltp_spec->Sin_Azimuth = sin(ltp_spec->Azimuth);
   ltp_spec->Cos_Azimuth = cos(ltp_spec->Azimuth);

   ltp_spec->Sin_Lat_Sin_Azimuth = ltp_spec->Sin_Origin_Lat
                                    * ltp_spec->Sin_Azimuth;

   ltp_spec->Sin_Lat_Cos_Azimuth = ltp_spec->Sin_Origin_Lat
                                    * ltp_spec->Cos_Azimuth;
   ltp_spec->Cos_Lat_Cos_Azimuth = ltp_spec->Cos_Origin_Lat
                                    * ltp_spec->Cos_Azimuth;
   ltp_spec->Cos_Lat_Sin_Azimuth = ltp_spec->Cos_Origin_Lat
                                    * ltp_spec->Sin_Azimuth;


   if(e_constants->Eps!=0) /* ellipsoid */
   {
      rn = e_constants->A / sqrt(1 - e_constants->Eps2 *
      ltp_spec->Sin_Origin_Lat * ltp_spec->Sin_Origin_Lat);

      r = rn + Origin_Height;

      temp = Origin_Height + rn * (1.0 - e_constants->Eps2);
   }
   else  /* spheroid */
   {
      r = Origin_Height + e_constants->A;
      temp = r;
   }


   ltp_spec->U0 = r * ltp_spec->Cos_Origin_Lat * ltp_spec->Cos_Origin_Lon;

   ltp_spec->V0 = r * ltp_spec->Cos_Origin_Lat * ltp_spec->Sin_Origin_Lon;

   ltp_spec->W0 = temp * ltp_spec->Sin_Origin_Lat;
}


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_ec_constants
 *
 *   Computes and stores the constants used for conversions involving EC
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   ec - a pointer to the parameters of a (destination) EC system
 *
 *   ec_gd_spec - a pointer to where the constants will be stored
 *                   for a EC to GD conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the ec_gd_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void set_ec_constants
(
    const SE_ERM_DATA                 *e_constants,
    const SE_SRF_PARAMETERS           *srf,
          SE_EC_SPECIFIC_CONSTANTS *ec_gd_spec
)
{
   SE_FLOAT64  geodetic_latitude, geodetic_longitude, x_offset,y_offset,central_scale_factor;

   if(srf->is_2D == SE_TRUE)  /* get 2d parameter values  */
   {
      geodetic_latitude    = srf->u.params_2d.u.ec_parameters.geodetic_latitude;
      geodetic_longitude   = srf->u.params_2d.u.ec_parameters.geodetic_longitude;
      x_offset             = srf->u.params_2d.u.ec_parameters.x_offset;
      y_offset             = srf->u.params_2d.u.ec_parameters.y_offset;
      central_scale_factor = srf->u.params_2d.u.ec_parameters.central_scale_factor;
   }
   else                       /* get 3d parameters values */
   {
      geodetic_latitude    = srf->u.params_3d.u.ec_parameters.geodetic_latitude;
      geodetic_longitude   = srf->u.params_3d.u.ec_parameters.geodetic_longitude;
      x_offset             = srf->u.params_3d.u.ec_parameters.x_offset;
      y_offset             = srf->u.params_3d.u.ec_parameters.y_offset;
      central_scale_factor = srf->u.params_2d.u.ec_parameters.central_scale_factor;
   }


   ec_gd_spec->x_offset = x_offset;
   ec_gd_spec->y_offset = y_offset;
   ec_gd_spec->latitude_origin  = geodetic_latitude * SE_RADIANS_PER_DEGREE;
   ec_gd_spec->longitude_origin = geodetic_longitude * SE_RADIANS_PER_DEGREE;

   /*
    * only valid for sphere; ellipsoid not supported
    */

   ec_gd_spec->scaled_radius = e_constants->A * central_scale_factor;

} /* end set_ec_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_gd_to_m_constants
 *
 *   Computes and stores the constants used for converting from GD to M.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   m - a pointer to the parameters of a (destination) M system
 *
 *   gd_m_spec - a pointer to where the constants will be stored
 *                  for a GD to M conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the gd_m_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void set_gd_to_m_constants
(
    const SE_ERM_DATA                *e_constants,
    const SE_SRF_PARAMETERS          *srf,
          SE_GD_M_SPECIFIC_CONSTANTS *gd_m_spec
)
{
   SE_FLOAT64 AA1, AA2, AA3, AA4, AA5, AA6, AA7, AA8, AA9,
              AB1, AB2, AB3, AB4, AB5, AB6, AB7, AB8, AB9,
              BB1, BB2, BB3, BB4, BB5;

   SE_FLOAT64 geodetic_longitude, central_scale_factor,x_offset,y_offset;

   if(srf->is_2D)    /* get 2d parameters */
   {
      geodetic_longitude   = srf->u.params_2d.u.m_parameters.geodetic_longitude;
      central_scale_factor = srf->u.params_2d.u.m_parameters.central_scale_factor;
      x_offset             = srf->u.params_2d.u.m_parameters.x_offset;
      y_offset             = srf->u.params_2d.u.m_parameters.y_offset;
   }
   else
   {                 /* get 3d parameters */
      geodetic_longitude   = srf->u.params_3d.u.m_parameters.geodetic_longitude;
      central_scale_factor = srf->u.params_3d.u.m_parameters.central_scale_factor;
      x_offset             = srf->u.params_3d.u.m_parameters.x_offset;
      y_offset             = srf->u.params_3d.u.m_parameters.y_offset;
   }

   gd_m_spec->longitude_origin  = geodetic_longitude * SE_RADIANS_PER_DEGREE;
   gd_m_spec->scale             = central_scale_factor;
   gd_m_spec->x_offset          = x_offset;
   gd_m_spec->y_offset          = y_offset;

   AA1= 0.100000000003490E+01;
   AA2=-0.643155723158021;
   AA3=-0.333332134894985;
   AA4=-0.241457540671514E-04;
   AA5= 0.143376648162652;
   AA6= 0.356844276587295;
   AA7=-0.333332875955149;
   AA8= 0.0;
   AA9= 0.0;

   BB1=AA1;
   BB2=e_constants->Eps2*AA2;
   BB3=e_constants->Eps2*(AA3+e_constants->Eps*(AA4+e_constants->Eps*AA5));
   BB4=e_constants->Eps2*AA6;
   BB5=e_constants->Eps2*(AA7+e_constants->Eps*(AA8+e_constants->Eps*AA9));

   gd_m_spec->CR11=BB3/BB5-.187E-11;
   gd_m_spec->CR12=(BB2-BB4*BB3/BB5)/BB5;
   gd_m_spec->CR13=(BB1-BB3/BB5)/BB5;
   gd_m_spec->CR14=1.0/BB5;
   gd_m_spec->CR15=BB4/BB5;

   AB1= 0.999999999957885;
   AB2=-0.115979311942142E+01;
   AB3=-0.333339671395063;
   AB4= 0.276473457331734E-03;
   AB5= 0.587786240368508;
   AB6=-0.159793128888088;
   AB7=-0.333333465982150;
   AB8= 0.746505041501704E-04;
   AB9=-0.701559218182283E-01;

   BB1=AB1;
   BB2=e_constants->Eps2*AB2;
   BB3=e_constants->Eps2*(AB3+e_constants->Eps*(AB4+e_constants->Eps*AB5));
   BB4=e_constants->Eps2*AB6;
   BB5=e_constants->Eps2*(AB7+e_constants->Eps*(AB8+e_constants->Eps*AB9));

   gd_m_spec->CR21=BB3/BB5+.2689E-11;
   gd_m_spec->CR22=(BB2-BB4*BB3/BB5)/BB5;
   gd_m_spec->CR23=(BB1-BB3/BB5)/BB5;
   gd_m_spec->CR24=1.0/BB5;
   gd_m_spec->CR25=BB4/BB5;
} /* end set_gd_to_m_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_m_to_gd_constants
 *
 *   Computes and stores the constants used for converting from M to GD.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   m - a pointer to the parameters of a (source) M system
 *
 *   m_gd_spec - a pointer to where the constants will be stored
 *                   for an M to GD conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the m_gd_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void set_m_to_gd_constants
(
    const SE_ERM_DATA                *e_constants,
    const SE_SRF_PARAMETERS          *srf,
          SE_M_GD_SPECIFIC_CONSTANTS *m_gd_spec
)
{
   SE_FLOAT64 pa2, pa4, pa6, pa8;

   SE_FLOAT64 geodetic_longitude, central_scale_factor,x_offset,y_offset;

   if(srf->is_2D)    /* get 2d parameters */
   {
      geodetic_longitude   = srf->u.params_2d.u.m_parameters.geodetic_longitude;
      central_scale_factor = srf->u.params_2d.u.m_parameters.central_scale_factor;
      x_offset             = srf->u.params_2d.u.m_parameters.x_offset;
      y_offset             = srf->u.params_2d.u.m_parameters.y_offset;
   }
   else
   {                 /* get 3d parameters */
      geodetic_longitude   = srf->u.params_3d.u.m_parameters.geodetic_longitude;
      central_scale_factor = srf->u.params_3d.u.m_parameters.central_scale_factor;
      x_offset             = srf->u.params_3d.u.m_parameters.x_offset;
      y_offset             = srf->u.params_3d.u.m_parameters.y_offset;
   }

   m_gd_spec->longitude_origin  = geodetic_longitude * SE_RADIANS_PER_DEGREE;
   m_gd_spec->scale             = central_scale_factor;
   m_gd_spec->x_offset          = x_offset;
   m_gd_spec->y_offset          = y_offset;

   /* NOTE:  There are no spherical constants, so we save a second by
             not computing the ellipsoid constants needlessly. -- dan
   */

   if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
   {
     pa2 = e_constants->Eps2 / 2.0 + pow(e_constants->Eps2, 2.0) * 5.0 / 24.0;

     pa2 += pow(e_constants->Eps2, 3.0) / 12.0 +
            pow(e_constants->Eps2, 4.0) * 13.0 / 360.0;

     pa4 = pow(e_constants->Eps2, 2.0) * 7.0 / 48.0 +
           pow(e_constants->Eps2, 3.0) * 29.0 / 240.0;

     pa4 += pow(e_constants->Eps2, 4.0) * 811.0 / 11520.0;

     pa6 = pow(e_constants->Eps2, 3.0) * 7.0 / 120.0 +
           pow(e_constants->Eps2, 4.0) * 81.0 / 1120.0;

     pa8 = pow(e_constants->Eps2, 4.0) * 4279.0 / 161280.0;

     m_gd_spec->b1 = pa2 *    2.0 + pa4 *   4.0 + pa6 *  6.0 + pa8 * 8.0;
     m_gd_spec->b2 = pa4 *   -8.0 - pa6 *  32.0 - pa8 * 80.0;
     m_gd_spec->b3 = pa6 *   32.0 + pa8 * 192.0;
     m_gd_spec->b4 = pa8 * -128.0;

   }

} /* end set_m_to_gd_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_lcc1_to_gd_constants
 *
 *   Computes and stores the constants used for converting from LCC1 to GD.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   lcc - a pointer to the parameters of a (source) LCC system
 *
 *   lcc1_gd_spec - a pointer to where the constants will be stored
 *                   for an LCC1 to GD conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the lcc1_gd_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void
set_lcc1_to_gd_constants
(
    const SE_ERM_DATA                   *e_constants,
    const SE_SRF_PARAMETERS             *srf,
          SE_LCC1_GD_SPECIFIC_CONSTANTS *lcc1_gd_spec
)
{
    SE_FLOAT64 phiz, pz, xl, rz, rz2, ex, xl2, cl2, rn2, latitude_origin,
               sin_org, porg, pa2, pa4, pa6, pa8,x_offset,y_offset;

    if (srf->is_2D == SE_TRUE) /* get 2d parameters    */
    {
        phiz                 = srf->u.params_2d.u.lcc_parameters.
                               north_parallel_geodetic_latitude;
        lcc1_gd_spec->xlonz  = srf->u.params_2d.u.lcc_parameters.
                               geodetic_longitude;
        latitude_origin      = srf->u.params_2d.u.lcc_parameters.
                               geodetic_latitude;
        x_offset             = srf->u.params_2d.u.lcc_parameters.x_offset;
        y_offset             = srf->u.params_2d.u.lcc_parameters.y_offset;
    }
    else
    {
        phiz                 = srf->u.params_3d.u.lcc_parameters.
                               north_parallel_geodetic_latitude;
        lcc1_gd_spec->xlonz  = srf->u.params_3d.u.lcc_parameters.
                               geodetic_longitude;
        latitude_origin      = srf->u.params_3d.u.lcc_parameters.
                               geodetic_latitude;
        x_offset             = srf->u.params_3d.u.lcc_parameters.x_offset;
        y_offset             = srf->u.params_3d.u.lcc_parameters.y_offset;
    }

    lcc1_gd_spec->x_offset = x_offset;
    lcc1_gd_spec->y_offset = y_offset;

    /* phiz == standard parallel */
    phiz *= SE_RADIANS_PER_DEGREE;

    /* xlonz == meridian y-axis */
    lcc1_gd_spec->xlonz *= SE_RADIANS_PER_DEGREE;

    latitude_origin *= SE_RADIANS_PER_DEGREE;

    sin_org = sin(latitude_origin);

    porg = tan(SE_PI/4 + 0.5 * latitude_origin);

    xl  = sin(phiz);
    ex  = 1 / (xl+xl);
    xl2 = xl * xl;
    cl2 = 1 - xl2;


    pz = tan(SE_PI/4 + .5 * phiz);

    if (e_constants->Eps!=0) /* do ellipsoid calculations */
    {
        pa2 = e_constants->Eps2 / 2.0 +
              pow(e_constants->Eps2, 2.0) * 5.0 / 24.0;

        pa2 += pow(e_constants->Eps2, 3.0) / 12.0 +
               pow(e_constants->Eps2, 4.0) * 13.0 / 360.0;

        pa4 = pow(e_constants->Eps2, 2.0) * 7.0 / 48.0 +
            pow(e_constants->Eps2, 3.0) * 29.0 / 240.0;

        pa4 += pow(e_constants->Eps2, 4.0) * 811.0 / 11520.0;

        pa6 = pow(e_constants->Eps2, 3.0) * 7.0 / 120.0 +
            pow(e_constants->Eps2, 4.0) * 81.0 / 1120.0;

        pa8 = pow(e_constants->Eps2, 4.0) * 4279.0 / 161280.0;

        lcc1_gd_spec->b1 = pa2 *    2.0 + pa4 *   4.0 + pa6 *  6.0 + pa8 * 8.0;
        lcc1_gd_spec->b2 = pa4 *   -8.0 - pa6 *  32.0 - pa8 * 80.0;
        lcc1_gd_spec->b3 = pa6 *   32.0 + pa8 * 192.0;
        lcc1_gd_spec->b4 = pa8 * -128.0;


        pz = pow( (1.0 - e_constants->Eps * xl)/(e_constants->Eps * xl + 1.0),
             e_constants->EpsH) *  pz;

        rn2 = e_constants->A2 / (1-e_constants->Eps2 * xl2);
        rz2 = rn2 * cl2 / xl2;

        porg = pow((1.0 - e_constants->Eps  * sin_org) /
                   (e_constants->Eps * sin_org + 1.0), e_constants->EpsH)*porg;
    }
    else /* do sphere calculations */
    {
        rz2 = e_constants->A2*cl2/xl2;
    }

    rz  = sqrt(rz2);

    lcc1_gd_spec->pz  = pz;
    lcc1_gd_spec->rz  = rz;
    lcc1_gd_spec->rz2 = rz2;
    lcc1_gd_spec->xl  = xl;
    lcc1_gd_spec->ex  = ex;

    /* prevent divide by zero  */
    if (fabs(porg)<=.000000000001)
        pz = 0;
    else
        pz = pz / porg;

    lcc1_gd_spec->rho_origin = rz * pow(pz , xl);
} /* end set_lcc1_to_gd_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_gd_to_lcc1_constants
 *
 *   Computes and stores the constants used for converting from GD to LCC1.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   lcc - a pointer to the parameters of a (destination) LCC1 system
 *
 *   gd_lcc1_spec - a pointer to where the constants will be stored
 *                   for a GD to LCC1 conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the gd_lcc1_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void
set_gd_to_lcc1_constants
(
    const SE_ERM_DATA                    *e_constants,
    const SE_SRF_PARAMETERS              *srf,
          SE_GD_LCC1_SPECIFIC_CONSTANTS  *gd_lcc1_spec
)
{
    SE_FLOAT64 AA1, AA2, AA3, AA4, AA5, AA6, AA7, AA8, AA9,
               AB1, AB2, AB3, AB4, AB5, AB6, AB7, AB8, AB9,
               BB1, BB2, BB3, BB4, BB5;
    SE_FLOAT64 cl2, phiz, pz, rn2, rz2, xl, rz, xl2, porg, latitude_origin,
               sin_org, x_offset, y_offset;

    if (srf->is_2D == SE_TRUE) /* get 2d parameters    */
    {
        phiz                 = srf->u.params_2d.u.lcc_parameters.
                               north_parallel_geodetic_latitude;
        gd_lcc1_spec->xlonz  = srf->u.params_2d.u.lcc_parameters.
                               geodetic_longitude;
        latitude_origin      = srf->u.params_2d.u.lcc_parameters.
                               geodetic_latitude;
        x_offset             = srf->u.params_2d.u.lcc_parameters.x_offset;
        y_offset             = srf->u.params_2d.u.lcc_parameters.y_offset;
    }
    else
    {
        phiz                 = srf->u.params_3d.u.lcc_parameters.
                               north_parallel_geodetic_latitude;
        gd_lcc1_spec->xlonz  = srf->u.params_3d.u.lcc_parameters.
                               geodetic_longitude;
        latitude_origin      = srf->u.params_3d.u.lcc_parameters.
                               geodetic_latitude;
        x_offset             = srf->u.params_3d.u.lcc_parameters.x_offset;
        y_offset             = srf->u.params_3d.u.lcc_parameters.y_offset;
    }

    gd_lcc1_spec->x_offset = x_offset;
    gd_lcc1_spec->y_offset = y_offset;

    /* phiz == standard parallel */
    phiz *= SE_RADIANS_PER_DEGREE;

    latitude_origin *= SE_RADIANS_PER_DEGREE;

    /* xlonz == meridian y-axis */
    gd_lcc1_spec->xlonz *= SE_RADIANS_PER_DEGREE;

    xl  = sin(phiz);
    xl2 = xl*xl;
    cl2 = 1-xl2;

    rn2 = e_constants->A2 / (1 - e_constants->Eps2 * xl2);
    rz2 = rn2*cl2/xl2;
    rz  = sqrt(rz2);

    pz = tan(SE_PI/4 + 0.5 * phiz);

    sin_org = sin(latitude_origin);

    porg = tan(SE_PI/4 + 0.5 * latitude_origin);

    if (e_constants->Eps!=0) /* ellipsoid calcs */
    {
        AA1= 0.100000000003490E+01;
        AA2=-0.643155723158021;
        AA3=-0.333332134894985;
        AA4=-0.241457540671514E-04;
        AA5= 0.143376648162652;
        AA6= 0.356844276587295;
        AA7=-0.333332875955149;
        AA8= 0.0;
        AA9= 0.0;

        BB1=AA1;
        BB2=e_constants->Eps2*AA2;
        BB3=e_constants->Eps2*(AA3+e_constants->Eps*(AA4+e_constants->Eps*AA5));
        BB4=e_constants->Eps2*AA6;
        BB5=e_constants->Eps2*(AA7+e_constants->Eps*(AA8+e_constants->Eps*AA9));

        gd_lcc1_spec->CR11=BB3/BB5-.187E-11;
        gd_lcc1_spec->CR12=(BB2-BB4*BB3/BB5)/BB5;
        gd_lcc1_spec->CR13=(BB1-BB3/BB5)/BB5;
        gd_lcc1_spec->CR14=1.0/BB5;
        gd_lcc1_spec->CR15=BB4/BB5;

        AB1= 0.999999999957885;
        AB2=-0.115979311942142E+01;
        AB3=-0.333339671395063;
        AB4= 0.276473457331734E-03;
        AB5= 0.587786240368508;
        AB6=-0.159793128888088;
        AB7=-0.333333465982150;
        AB8= 0.746505041501704E-04;
        AB9=-0.701559218182283E-01;

        BB1=AB1;
        BB2=e_constants->Eps2*AB2;
        BB3=e_constants->Eps2*(AB3+e_constants->Eps*(AB4+e_constants->Eps*AB5));
        BB4=e_constants->Eps2*AB6;
        BB5=e_constants->Eps2*(AB7+e_constants->Eps*(AB8+e_constants->Eps*AB9));

        gd_lcc1_spec->CR21=BB3/BB5-.2689E-11;
        gd_lcc1_spec->CR22=(BB2-BB4*BB3/BB5)/BB5;
        gd_lcc1_spec->CR23=(BB1-BB3/BB5)/BB5;
        gd_lcc1_spec->CR24=1.0/BB5;
        gd_lcc1_spec->CR25=BB4/BB5;

        pz  = pow((1.0 - e_constants->Eps * xl) / (e_constants->Eps * xl + 1.0),
                   e_constants->EpsH) * pz;

        porg = pow((1.0 - e_constants->Eps  * sin_org) /
                   (e_constants->Eps * sin_org + 1.0), e_constants->EpsH)*porg;
    }
    else /* for sphere */
    {
        rz = sqrt(e_constants->A2*cl2/xl2);
    }

    gd_lcc1_spec->xl=xl;
    gd_lcc1_spec->pz=pz;
    gd_lcc1_spec->rz=rz;

    /* prevent divide by zero  */
    if (fabs(porg)<=.000000000001)
        pz = 0;
    else
        pz = pz / porg;

    gd_lcc1_spec->rho_origin = rz * pow(pz , xl);
} /* end set gd_to_lcc1_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_lcc2_to_gd_constants
 *
 *   Computes and stores the constants used for converting from LCC2 to GD.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   lcc - a pointer to the parameters of a (source) LCC system
 *
 *   lcc2_gd_spec - a pointer to where the constants will be stored
 *                   for an LCC2 to GD conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the lcc2_gd_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void
set_lcc2_to_gd_constants
(
    const SE_ERM_DATA                   *e_constants,
    const SE_SRF_PARAMETERS             *srf,
          SE_LCC2_GD_SPECIFIC_CONSTANTS *lcc2_gd_spec
)
{
    SE_FLOAT64 phiz1, phiz2, latitude_origin, sfx, pz, s1, s12, c12, p1,
               s2, s22, p2, rn12, rn22, temp, xl, rz, ex, c22, rz2,
               pa2, pa4, pa6, pa8,x_offset,y_offset;

    if (srf->is_2D == SE_TRUE) /* get 2d parameters    */
    {
        phiz1                = srf->u.params_2d.u.lcc_parameters.
                               north_parallel_geodetic_latitude;
        phiz2                = srf->u.params_2d.u.lcc_parameters.
                               south_parallel_geodetic_latitude;
        lcc2_gd_spec->xlonz  = srf->u.params_2d.u.lcc_parameters.
                               geodetic_longitude;
        latitude_origin      = srf->u.params_2d.u.lcc_parameters.
                               geodetic_latitude;
        x_offset             = srf->u.params_2d.u.lcc_parameters.x_offset;
        y_offset             = srf->u.params_2d.u.lcc_parameters.y_offset;
    }
    else
    {
        phiz1                = srf->u.params_3d.u.lcc_parameters.
                               north_parallel_geodetic_latitude;
        phiz2                = srf->u.params_3d.u.lcc_parameters.
                               south_parallel_geodetic_latitude;
        lcc2_gd_spec->xlonz  = srf->u.params_3d.u.lcc_parameters.
                               geodetic_longitude;
        latitude_origin      = srf->u.params_3d.u.lcc_parameters.
                               geodetic_latitude;
        x_offset             = srf->u.params_3d.u.lcc_parameters.x_offset;
        y_offset             = srf->u.params_3d.u.lcc_parameters.y_offset;
    }

    lcc2_gd_spec->x_offset = x_offset;
    lcc2_gd_spec->y_offset = y_offset;

    /* Convert to radians  */
    phiz1 *= SE_RADIANS_PER_DEGREE;
    phiz2 *= SE_RADIANS_PER_DEGREE;

    /* phix == fixed latitude */
    latitude_origin *= SE_RADIANS_PER_DEGREE;

    /* xlonz == meridian y-axis */
    lcc2_gd_spec->xlonz *= SE_RADIANS_PER_DEGREE;

    pa2  = e_constants->Eps2 / 2.0 + pow(e_constants->Eps2, 2) * 5.0 / 24.0;

    pa2 += pow(e_constants->Eps2, 3) / 12.0 +
           pow(e_constants->Eps2, 4) * 13.0 / 360.0;

    pa4  = pow(e_constants->Eps2, 2) * 7.0 / 48.0 +
           pow(e_constants->Eps2, 3) * 29.0 / 240.0;

    pa4 += pow(e_constants->Eps2, 4) * 811.0 / 11520.0;

    pa6  = pow(e_constants->Eps2, 3) * 7.0 / 120.0 +
           pow(e_constants->Eps2, 4) * 81.0 / 1120.0;

    pa8  = pow(e_constants->Eps2, 4) * 4279.0 / 161280.0;

    lcc2_gd_spec->b1 = pa2 *    2.0 + pa4 *   4.0 + pa6 *  6.0 + pa8 * 8.0;
    lcc2_gd_spec->b2 = pa4 *   -8.0 - pa6 *  32.0 - pa8 * 80.0;
    lcc2_gd_spec->b3 = pa6 *   32.0 + pa8 * 192.0;
    lcc2_gd_spec->b4 = pa8 * -128.0;


    sfx = sin(latitude_origin);
    s1  = sin(phiz1);
    s2  = sin(phiz2);
    s12 = s1 * s1;
    c12 = 1 - s12;
    s22 = s2 * s2;
    c22 = 1 - s22;

    pz = tan(SE_PI/4.0 + .5 * latitude_origin);

    p1 = tan(SE_PI/4.0 + .5 * phiz1);
    p2 = tan(SE_PI/4.0 + .5 * phiz2);

    if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
    {
        pz = pz *pow( (1.0 - e_constants->Eps * sfx) /
             (e_constants->Eps * sfx + 1.0),e_constants->EpsH);

        p1  = p1 * pow( (1.0 - e_constants->Eps * s1) /
             (e_constants->Eps * s1 + 1.0),e_constants->EpsH);

        p2  = p2 * pow( (1.0 - e_constants->Eps * s2) /
             (e_constants->Eps * s2 + 1.0),e_constants->EpsH);

        rn12 = e_constants->A2 / (1.0 - e_constants->Eps2 * s12);
        rn22 = e_constants->A2 / (1.0 - e_constants->Eps2 * s22);

        temp = p2 / p1;

        xl  = .5 * log((rn12 * c12) / (c22 * rn22)) / log(temp);
    }
    else /* compute spherical values */
    {
        temp = p2 / p1;
        rn12=e_constants->A2;
        xl = .5 * log(c12/c22)/log(temp);
    }
    rz2 = rn12 * c12 / (xl*xl);

    rz  = sqrt(rz2) * pow(p1/pz,xl);

    ex  = 1 / (xl+xl);

    lcc2_gd_spec->rz  = rz;
    lcc2_gd_spec->xl  = xl;
    lcc2_gd_spec->ex  = ex;
    lcc2_gd_spec->rz2 = rz2;
    lcc2_gd_spec->p1  = p1;
} /* end set_lcc2_to_gd_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_gd_to_lcc2_constants
 *
 *   Computes and stores the constants used for converting from GD to LCC2.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   lcc - a pointer to the parameters of a (destination) LCC2 system
 *
 *   gd_lcc2_spec - a pointer to where the constants will be stored
 *                   for a GD to LCC2 conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the gd_lcc2_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void set_gd_to_lcc2_constants
(
    const SE_ERM_DATA                    *e_constants,
    const SE_SRF_PARAMETERS              *srf,
          SE_GD_LCC2_SPECIFIC_CONSTANTS  *gd_lcc2_spec
)
{
   SE_FLOAT64 AA1,AA2,AA3,AA4,AA5,AA6,AA7,AA8,AA9,
              AB1,AB2,AB3,AB4,AB5,AB6,AB7,AB8,AB9,
              BB1,BB2,BB3,BB4,BB5;
   SE_FLOAT64 phiz1, phiz2, latitude_origin,sfx,pz,s1,s12,c12,p1,
            s2,s22,p2,rn12,rn22,temp,xl,rx2,rz,c22,rx,x_offset,y_offset;

   if (srf->is_2D == SE_TRUE) /* get 2d parameters    */
   {
       phiz1                = srf->u.params_2d.u.lcc_parameters.
                              north_parallel_geodetic_latitude;
       phiz2                = srf->u.params_2d.u.lcc_parameters.
                              south_parallel_geodetic_latitude;
       gd_lcc2_spec->xlonz  = srf->u.params_2d.u.lcc_parameters.
                              geodetic_longitude;
       latitude_origin      = srf->u.params_2d.u.lcc_parameters.
                              geodetic_latitude;
       x_offset             = srf->u.params_2d.u.lcc_parameters.x_offset;
       y_offset             = srf->u.params_2d.u.lcc_parameters.y_offset;
   }
   else
   {
       phiz1                = srf->u.params_3d.u.lcc_parameters.
                              north_parallel_geodetic_latitude;
       phiz2                = srf->u.params_3d.u.lcc_parameters.
                              south_parallel_geodetic_latitude;
       gd_lcc2_spec->xlonz  = srf->u.params_3d.u.lcc_parameters.
                              geodetic_longitude;
       latitude_origin      = srf->u.params_3d.u.lcc_parameters.
                              geodetic_latitude;
       x_offset             = srf->u.params_3d.u.lcc_parameters.x_offset;
       y_offset             = srf->u.params_3d.u.lcc_parameters.y_offset;
   }
   gd_lcc2_spec->x_offset = x_offset;
   gd_lcc2_spec->y_offset = y_offset;

   /* phiz == standard parallels */

   phiz1 *= SE_RADIANS_PER_DEGREE;

   phiz2 *= SE_RADIANS_PER_DEGREE;

   /* phix == fixed latitude */

   latitude_origin *= SE_RADIANS_PER_DEGREE;

   /* xlonz == meridian y-axis */
   gd_lcc2_spec->xlonz *= SE_RADIANS_PER_DEGREE;

   /*
    * test whether this is a spherical transformation(Eps=0);
    * if not, then we compute these constants
    */
   if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
   {
       AA1= 0.100000000003490E+01;
       AA2=-0.643155723158021;
       AA3=-0.333332134894985;
       AA4=-0.241457540671514E-04;
       AA5= 0.143376648162652;
       AA6= 0.356844276587295;
       AA7=-0.333332875955149;
       AA8= 0.0;
       AA9= 0.0;

       BB1=AA1;
       BB2=e_constants->Eps2*AA2;
       BB3=e_constants->Eps2*(AA3+e_constants->Eps*(AA4+e_constants->Eps*AA5));
       BB4=e_constants->Eps2*AA6;
       BB5=e_constants->Eps2*(AA7+e_constants->Eps*(AA8+e_constants->Eps*AA9));

       gd_lcc2_spec->CR11=BB3/BB5-.187E-11;
       gd_lcc2_spec->CR12=(BB2-BB4*BB3/BB5)/BB5;
       gd_lcc2_spec->CR13=(BB1-BB3/BB5)/BB5;
       gd_lcc2_spec->CR14=1.0/BB5;
       gd_lcc2_spec->CR15=BB4/BB5;

       AB1= 0.999999999957885;
       AB2=-0.115979311942142E+01;
       AB3=-0.333339671395063;
       AB4= 0.276473457331734E-03;
       AB5= 0.587786240368508;
       AB6=-0.159793128888088;
       AB7=-0.333333465982150;
       AB8= 0.746505041501704E-04;
       AB9=-0.701559218182283E-01;

       BB1=AB1;
       BB2=e_constants->Eps2*AB2;
       BB3=e_constants->Eps2*(AB3+e_constants->Eps*(AB4+e_constants->Eps*AB5));
       BB4=e_constants->Eps2*AB6;
       BB5=e_constants->Eps2*(AB7+e_constants->Eps*(AB8+e_constants->Eps*AB9));

       gd_lcc2_spec->CR21=BB3/BB5-.2689E-11;
       gd_lcc2_spec->CR22=(BB2-BB4*BB3/BB5)/BB5;
       gd_lcc2_spec->CR23=(BB1-BB3/BB5)/BB5;
       gd_lcc2_spec->CR24=1.0/BB5;
       gd_lcc2_spec->CR25=BB4/BB5;
   } /* end if not spherical */

   sfx = sin(latitude_origin);
   s1  = sin(phiz1);
   s2  = sin(phiz2);

   s12 = s1*s1;
   c12 = 1-s12;

   s22 = s2*s2;
   c22 = 1-s22;

   pz = tan(SE_PI/4.0 + (0.5 * latitude_origin));
   p1 = tan(SE_PI/4.0 + (0.5 * phiz1));
   p2 = tan(SE_PI/4.0 + (0.5*phiz2));

   if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
   {
      pz  = pz * pow((1.0 - e_constants->Eps * sfx)/
            (e_constants->Eps * sfx + 1.0),e_constants->EpsH);

      p1  = p1 * pow((1.0 - e_constants->Eps * s1)/
            (e_constants->Eps * s1 + 1.0),e_constants->EpsH);

      p2  = p2 * pow((1.0 - e_constants->Eps * s2)/
            (e_constants->Eps * s2 + 1.0),e_constants->EpsH);


      rn12 = e_constants->A2 / (1.0 - (e_constants->Eps2 * s12));
      rn22 = e_constants->A2 / (1.0 - (e_constants->Eps2 * s22));
      temp = p2/p1;
      xl  = .5 * log( (rn12 * c12)/( c22 * rn22 ) ) / log(temp);
   }
   else /* compute spherical values */
   {
       rn12 = e_constants->A2;
       temp = p2 / p1;
       xl = 0.5 * log( c12 / c22) / log(temp);
   }
   rx2 = rn12 * c12 / (xl * xl);
   rx  = sqrt(rx2);
   rz  = rx * pow(p1 / pz,xl);

   gd_lcc2_spec->rx = rx;
   gd_lcc2_spec->rz = rz;
   gd_lcc2_spec->xl = xl;
   gd_lcc2_spec->p1 = p1;
} /* end set_gd_to_lcc2_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_ps_to_gd_constants
 *
 *   Computes and stores the constants used for converting from PS to GD.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   ps - a pointer to the parameters of a (source) PS system
 *
 *   ps_gd_spec - a pointer to where the constants will be stored
 *                 for a PS to GD conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the ps_gd_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void set_ps_to_gd_constants
(
    const SE_ERM_DATA                 *e_constants,
    const SE_SRF_PARAMETERS           *srf,
          SE_PS_GD_SPECIFIC_CONSTANTS *ps_gd_spec
)
{
   SE_FLOAT64 pa2, pa4, pa6, pa8,x_offset,y_offset;
   SE_PS_POLE_ENUM pole;

   /* Note:  The ps geodetic_longitude parameter is not currently
             used.  -- dan
   */

   if(srf->is_2D == SE_TRUE)  /* get 2d parameters */
   {
      pole = srf->u.params_2d.u.ps_parameters.pole;
      x_offset = srf->u.params_2d.u.ps_parameters.x_offset;
      y_offset = srf->u.params_2d.u.ps_parameters.y_offset;
   }
   else                       /* get 3d parameters */
   {
      pole = srf->u.params_3d.u.ps_parameters.pole;
      x_offset = srf->u.params_3d.u.ps_parameters.x_offset;
      y_offset = srf->u.params_3d.u.ps_parameters.y_offset;
   }

   ps_gd_spec->pole = pole;
   ps_gd_spec->x_offset = x_offset;
   ps_gd_spec->y_offset = y_offset;

   if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
   {
      pa2  = e_constants->Eps2 /2.0 +
            pow(e_constants->Eps2, 2) * 5.0 / 24.0;

      pa2 += pow(e_constants->Eps2, 3) / 12.0 +
            pow(e_constants->Eps2, 4) * 13.0 / 360.0;

      pa4  = pow(e_constants->Eps2, 2) * 7.0 / 48.0 +
            pow(e_constants->Eps2, 3) * 29.0 / 240.0;

      pa4 += pow(e_constants->Eps2, 4) * 811.0 / 11520.0;

      pa6  = pow(e_constants->Eps2, 3) * 7.0 / 120.0 +
            pow(e_constants->Eps2, 4) * 81.0 / 1120.0;

      pa8  = pow(e_constants->Eps2, 4) * 4279.0 / 161280.0;

      ps_gd_spec->b1 = pa2 *    2.0 + pa4 *   4.0 + pa6 *  6.0 + pa8 * 8.0;

      ps_gd_spec->b2 = pa4 *   -8.0 - pa6 *  32.0 - pa8 * 80.0;

      ps_gd_spec->b3 = pa6 *   32.0 + pa8 * 192.0;

      ps_gd_spec->b4 = pa8 * -128.0;

      ps_gd_spec->xk = pow(((1.0 - e_constants->Eps) /
                            (e_constants->Eps + 1.0)),
                           e_constants->EpsH);

      ps_gd_spec->xk = e_constants->A2 * 2.0 * ps_gd_spec->xk /
                       e_constants->C;
   }
   else /* compute spherical values */
   {
     ps_gd_spec->xk = e_constants->A * 2.0;
   }
} /* end set_ps_to_gd_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_gd_to_ps_constants
 *
 *   Computes and stores the constants used for converting from GD to PS.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   ps - a pointer to the parameters of a (destination) PS system
 *
 *   gd_ps_spec - a pointer to where the constants will be stored
 *                 for a GD to PS conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the gd_ps_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void set_gd_to_ps_constants
(
    const SE_ERM_DATA                  *e_constants,
    const SE_SRF_PARAMETERS            *srf,
          SE_GD_PS_SPECIFIC_CONSTANTS  *gd_ps_spec
)
{
   SE_FLOAT64 AA1, AA2, AA3, AA4, AA5, AA6, AA7, AA8, AA9;
   SE_FLOAT64 AB1, AB2, AB3, AB4, AB5, AB6, AB7, AB8, AB9;
   SE_FLOAT64 BB1, BB2, BB3, BB4, BB5,x_offset,y_offset;

   SE_PS_POLE_ENUM pole;

   /* Note:  The ps geodetic_longitude parameter is not currently
             used.  -- dan
   */

   if(srf->is_2D == SE_TRUE)  /* get 2d parameters */
   {
      pole = srf->u.params_2d.u.ps_parameters.pole;
      x_offset = srf->u.params_2d.u.ps_parameters.x_offset;
      y_offset = srf->u.params_2d.u.ps_parameters.y_offset;
   }
   else                       /* get 3d parameters */
   {
      pole = srf->u.params_3d.u.ps_parameters.pole;
      x_offset = srf->u.params_3d.u.ps_parameters.x_offset;
      y_offset = srf->u.params_3d.u.ps_parameters.y_offset;
   }

   gd_ps_spec->pole = pole;
   gd_ps_spec->x_offset = x_offset;
   gd_ps_spec->y_offset = y_offset;

   if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
   {
      AA1= 0.100000000003490E+01;
      AA2=-0.643155723158021;
      AA3=-0.333332134894985;
      AA4=-0.241457540671514E-04;
      AA5= 0.143376648162652;
      AA6= 0.356844276587295;
      AA7=-0.333332875955149;
      AA8= 0.0;
      AA9= 0.0;

      BB1=AA1;
      BB2=e_constants->Eps2*AA2;
      BB3=e_constants->Eps2*(AA3+e_constants->Eps*(AA4+e_constants->Eps*AA5));
      BB4=e_constants->Eps2*AA6;
      BB5=e_constants->Eps2*(AA7+e_constants->Eps*(AA8+e_constants->Eps*AA9));

      gd_ps_spec->CR11=BB3/BB5-.187E-11;
      gd_ps_spec->CR12=(BB2-BB4*BB3/BB5)/BB5;
      gd_ps_spec->CR13=(BB1-BB3/BB5)/BB5;
      gd_ps_spec->CR14=1.0/BB5;
      gd_ps_spec->CR15=BB4/BB5;

      AB1= 0.999999999957885;
      AB2=-0.115979311942142E+01;
      AB3=-0.333339671395063;
      AB4= 0.276473457331734E-03;
      AB5= 0.587786240368508;
      AB6=-0.159793128888088;
      AB7=-0.333333465982150;
      AB8= 0.746505041501704E-04;
      AB9=-0.701559218182283E-01;

      BB1=AB1;
      BB2=e_constants->Eps2*AB2;
      BB3=e_constants->Eps2*(AB3+e_constants->Eps*(AB4+e_constants->Eps*AB5));
      BB4=e_constants->Eps2*AB6;
      BB5=e_constants->Eps2*(AB7+e_constants->Eps*(AB8+e_constants->Eps*AB9));

      gd_ps_spec->CR21=BB3/BB5+.2689E-11;
      gd_ps_spec->CR22=(BB2-BB4*BB3/BB5)/BB5;
      gd_ps_spec->CR23=(BB1-BB3/BB5)/BB5;
      gd_ps_spec->CR24=1.0/BB5;
      gd_ps_spec->CR25=BB4/BB5;

      gd_ps_spec->xk = e_constants->A2 * 2.0 / e_constants->C *
                       pow(((1.0 - e_constants->Eps) /(e_constants->Eps + 1.0)),
                       e_constants->EpsH);

   }
   else /* compute spherical values */
   {
      gd_ps_spec->xk = e_constants->A * 2.0;
   }
} /* end set_gd_to_ps_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_gd_to_tm_constants
 *
 *   Computes and stores the constants used for converting from GD to TM.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   tm - a pointer to the parameters of a (destination) TM system
 *
 *   gd_tm_spec - a pointer to where the constants will be stored
 *                  for a GD to TM conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the gd_tm_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void set_gd_to_tm_constants
(
    const SE_ERM_DATA                  *e_constants,
    const SE_SRF_PARAMETERS            *srf,
          SE_GD_TM_SPECIFIC_CONSTANTS *gd_tm_spec
)
{
   SE_FLOAT64 polx2b, polx3b, polx4b, polx5b,s1,s12,c1,latitude_origin,
              longitude_origin,central_scale_factor,x_offset,y_offset;

   if(srf->is_2D == SE_TRUE)  /* get 2d parameters */
   {
      latitude_origin      = srf->u.params_2d.u.tm_parameters.geodetic_latitude;
      longitude_origin     = srf->u.params_2d.u.tm_parameters.geodetic_longitude;
      central_scale_factor = srf->u.params_2d.u.tm_parameters.central_scale_factor;
      x_offset             = srf->u.params_2d.u.tm_parameters.x_offset;
      y_offset             = srf->u.params_2d.u.tm_parameters.y_offset;
   }
   else  /* get 3d parameters */
   {
      latitude_origin      = srf->u.params_3d.u.tm_parameters.geodetic_latitude;
      longitude_origin     = srf->u.params_3d.u.tm_parameters.geodetic_longitude;
      central_scale_factor = srf->u.params_3d.u.tm_parameters.central_scale_factor;
      x_offset             = srf->u.params_3d.u.tm_parameters.x_offset;
      y_offset             = srf->u.params_3d.u.tm_parameters.y_offset;
   }

   latitude_origin *= SE_RADIANS_PER_DEGREE;
   gd_tm_spec->longitude_origin = longitude_origin * SE_RADIANS_PER_DEGREE;
   gd_tm_spec->CScale = central_scale_factor;
   gd_tm_spec->x_offset = x_offset;
   gd_tm_spec->y_offset = y_offset;

   gd_tm_spec->Cdb6   = .166666666666667;
   gd_tm_spec->Cdb24  = .0416666666666667;
   gd_tm_spec->Cdb120 = .00833333333333333;
   gd_tm_spec->Cdb720 = .00138888888888888;

   if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
   {
      s1  = sin(latitude_origin);
      s12 = s1 * s1;
      c1  = cos(latitude_origin);

      polx2b = 1.0 * e_constants->Eps2 + 1.0 / 4.0 * pow(e_constants->Eps2, 2)
              + 15.0/128.0 * pow(e_constants->Eps2, 3) - 455.0 / 4096.0
              * pow(e_constants->Eps2, 4);

      polx2b = 3.0/8.0 * polx2b;

      polx3b = 1.0     * pow(e_constants->Eps2, 2) + 3.0 / 4.0 * pow(e_constants->Eps2, 3)
              - 77.0 / 128.0 * pow(e_constants->Eps2, 4);

      polx3b = 15.0 / 256.0 * polx3b;

      polx4b = pow(e_constants->Eps2, 3) - 41.0 / 32.0 * pow(e_constants->Eps2, 4);

      polx4b = polx4b * 35.0 / 3072.0;

      polx5b =  -315.0/131072.0 * pow(e_constants->Eps2, 4);

      gd_tm_spec->poly1b = 1.0
              - (1.0/4.0 * e_constants->Eps2)
              - (3.0/64.0 * pow(e_constants->Eps2, 2))
              - (5.0/256.0 * pow(e_constants->Eps2, 3))
              - (175.0/16384.0 * pow(e_constants->Eps2, 4));

      gd_tm_spec->poly2b = polx2b * -2.0 + polx3b * 4.0
                               - polx4b * 6.0
                               + polx5b * 8.0;

      gd_tm_spec->poly3b = polx3b * -8.0 + polx4b * 32.0 - polx5b * 80.0;

      gd_tm_spec->poly4b = polx4b * -32.0 + polx5b * 192.0;

      gd_tm_spec->poly5b = polx5b * -128.0;

      gd_tm_spec->smz = s1 * c1 * (gd_tm_spec->poly2b + s12 *
                           ( gd_tm_spec->poly3b + s12 *
                           ( gd_tm_spec->poly4b +s12 * gd_tm_spec->poly5b)));
      gd_tm_spec->smz = e_constants->A *
                           (gd_tm_spec->poly1b * latitude_origin +
                            gd_tm_spec->smz);
   }
   else /* compute spherical values */
       gd_tm_spec->smz = e_constants->A * latitude_origin;
} /* end set_gd_to_tm_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_tm_to_gd_constants
 *
 *   Computes and stores the constants used for converting from TM to GD.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   tm - a pointer to the parameters of a (source) TM system
 *
 *   tm_gd_spec - a pointer to where the constants will be stored
 *                  for a TM to GD conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the tm_gd_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void set_tm_to_gd_constants
(
    const SE_ERM_DATA                   *e_constants,
    const SE_SRF_PARAMETERS             *srf,
          SE_TM_GD_SPECIFIC_CONSTANTS *tm_gd_spec
)
{
   SE_FLOAT64 polx1a, polx2a, polx4a, polx6a, polx8a,con,s1,c1,s12,
               latitude_origin, poly1b,poly2b,poly3b,poly4b,poly5b,polx2b,
               polx3b,polx4b,polx5b,longitude_origin,
               central_scale_factor,x_offset,y_offset;

   if(srf->is_2D == SE_TRUE)  /* get 2d parameters */
   {
      latitude_origin      = srf->u.params_2d.u.tm_parameters.geodetic_latitude;
      longitude_origin     = srf->u.params_2d.u.tm_parameters.geodetic_longitude;
      central_scale_factor = srf->u.params_2d.u.tm_parameters.central_scale_factor;
      x_offset             = srf->u.params_2d.u.tm_parameters.x_offset;
      y_offset             = srf->u.params_2d.u.tm_parameters.y_offset;
   }
   else  /* get 3d parameters */
   {
      latitude_origin      = srf->u.params_3d.u.tm_parameters.geodetic_latitude;
      longitude_origin     = srf->u.params_3d.u.tm_parameters.geodetic_longitude;
      central_scale_factor = srf->u.params_3d.u.tm_parameters.central_scale_factor;
      x_offset             = srf->u.params_3d.u.tm_parameters.x_offset;
      y_offset             = srf->u.params_3d.u.tm_parameters.y_offset;
   }

   latitude_origin *= SE_RADIANS_PER_DEGREE;
   tm_gd_spec->longitude_origin = longitude_origin * SE_RADIANS_PER_DEGREE;
   tm_gd_spec->CScale = central_scale_factor;
   tm_gd_spec->x_offset = x_offset;
   tm_gd_spec->y_offset = y_offset;

   tm_gd_spec->Con2   = 2;
   tm_gd_spec->Con6   = .166666666666667;
   tm_gd_spec->Con24  = 4 * .0416666666666667;
   tm_gd_spec->Con120 = .00833333333333333;
   tm_gd_spec->Con720 = 4 * .00138888888888888;

   s1  = sin(latitude_origin);
   c1  = cos(latitude_origin);
   s12 = s1 * s1;

   if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
   {
      tm_gd_spec->EF = e_constants->F/(2.0-e_constants->F);
      con = 1.0-e_constants->Eps2;

      tm_gd_spec->Con2 /=con;
      tm_gd_spec->Con24 /=con;
      tm_gd_spec->Con720 /=con;

      polx1a = 1.0 - e_constants->Eps2 / 4.0
            - 3.0/   64.0 * pow(e_constants->Eps2, 2)
            - 5.0/  256.0 * pow(e_constants->Eps2, 3)
          - 175.0/16384.0 * pow(e_constants->Eps2, 4);

      tm_gd_spec->conap  = e_constants->A * polx1a;

      polx2a = 3.0/2.0 * tm_gd_spec->EF
           - 27.0/32.0 * pow(tm_gd_spec->EF, 3);

      polx4a = 21.0/16.0 * pow(tm_gd_spec->EF, 2)
            - 55.0/32.0 * pow(tm_gd_spec->EF, 4);

      polx6a = 151.0/96.0 * pow(tm_gd_spec->EF, 3);

      polx8a = 1097.0/512.0 * pow(tm_gd_spec->EF, 4);

      /* add polys for computing smz */

      polx2b = 1.0     * e_constants->Eps2 +
              1.0/4.0 * pow(e_constants->Eps2, 2)
           + 15.0/128.0 * pow(e_constants->Eps2, 3)
           - 455.0/4096.0 * pow(e_constants->Eps2, 4);

      polx2b = 3.0/8.0 * polx2b;

      polx3b = 1.0     * pow(e_constants->Eps2, 2) +
              3.0/4.0 * pow(e_constants->Eps2, 3)
           - 77.0/128.0 * pow(e_constants->Eps2, 4);

      polx3b = 15.0 / 256.0 * polx3b;

      polx4b = pow(e_constants->Eps2, 3) -
              41.0/32.0 * pow(e_constants->Eps2, 4);

      polx4b = polx4b * 35.0 / 3072.0;

      polx5b =  -315.0/131072.0 * pow(e_constants->Eps2, 4);

      poly1b = 1.0 - (1.0/4.0 * e_constants->Eps2)
           - (3.0/64.0 * pow(e_constants->Eps2, 2))
           - (5.0/256.0 * pow(e_constants->Eps2, 3))
           - (175.0/16384.0 * pow(e_constants->Eps2, 4));

      poly2b = polx2b * -2.0 + polx3b * 4.0
                            - polx4b * 6.0
                            + polx5b * 8.0;

      poly3b = polx3b * -8.0 + polx4b * 32.0
                            - polx5b * 80.0;

      poly4b = polx4b * -32.0
                            + polx5b * 192.0;

      poly5b = polx5b * -128.0;

      /* these are different polxb's than above */

      tm_gd_spec->polx2b = polx2a * 2.0 + polx4a * 4.0 +
                           polx6a * 6.0 + polx8a * 8.0;

      tm_gd_spec->polx3b = polx4a * -8.0 - polx6a * 32.0
                            - 80.0 *polx8a;
      tm_gd_spec->polx4b = polx6a * 32.0 + 192.0*polx8a;
      tm_gd_spec->polx5b = -128.0 *polx8a;

      tm_gd_spec->smz = s1 * c1 * (poly2b + s12 * (poly3b + s12 *
                  (poly4b + s12 * poly5b)));
      tm_gd_spec->smz = e_constants->A * (poly1b * latitude_origin +
                                          tm_gd_spec->smz);
   }
   else /* compute spherical values */
   {
      tm_gd_spec->smz = e_constants->A * latitude_origin;
      tm_gd_spec->conap  = e_constants->A;
   }
} /* end set_tm_to_gd_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_gd_to_utm_constants
 *
 *   Computes and stores the constants used for converting from GD to UTM.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   utm - a pointer to the parameters of a (destination) UTM system
 *
 *   gd_utm_spec - a pointer to where the constants will be stored
 *                  for a GD to UTM conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the gd_utm_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void set_gd_to_utm_constants
(
    const SE_ERM_DATA                  *e_constants,
    const SE_SRF_PARAMETERS            *srf,
          SE_GD_UTM_SPECIFIC_CONSTANTS *gd_utm_spec
)
{
    SE_FLOAT64 polx2b, polx3b, polx4b, polx5b,x_offset,y_offset;
    int zone;

    if(srf->is_2D == SE_TRUE) /* get 2d parameters */
    {
       x_offset  = srf->u.params_2d.u.utm_parameters.x_offset;
       y_offset  = srf->u.params_2d.u.utm_parameters.y_offset;
       zone      = srf->u.params_2d.u.utm_parameters.zone;
    }
    else                      /* get 3d parameters */
    {
       x_offset  = srf->u.params_3d.u.utm_parameters.x_offset;
       y_offset  = srf->u.params_3d.u.utm_parameters.y_offset;
       zone      = srf->u.params_3d.u.utm_parameters.zone;
    }

    gd_utm_spec->x_offset = x_offset;
    gd_utm_spec->y_offset = y_offset;
    gd_utm_spec->zone     = zone;

   /*
    * THESE ARE THE COEFFICIENTS FOR S¯ GIVEN IN
    * TEC-SR-7.
    * NOTE THAT THE COEFFICIENTS ARE NUMBERED SLIGHTLY
    * DIFFERENTLY THAN IN TEC-SR-7.  SINCE THIS IS
    * INITIALIZATION DATA NO ATTEMPT WAS MADE TO CODE
    * EFFICIENTLY BUT RATHER TO RETAIN CLARITY.
    */

    gd_utm_spec->scale = .9996;
    gd_utm_spec->Cdb6   = .166666666666667;
    gd_utm_spec->Cdb24  = .0416666666666667;
    gd_utm_spec->Cdb120 = .00833333333333333;
    gd_utm_spec->Cdb720 = .00138888888888888;

    if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
    {
        polx2b = e_constants->Eps2 +   1.0/4.0 * pow(e_constants->Eps2, 2)
              + 15.0/128.0 * pow(e_constants->Eps2, 3)
              - 455.0/4096.0 * pow(e_constants->Eps2, 4);

        polx2b = 3.0/8.0 * polx2b;

        polx3b = 1.0     * pow(e_constants->Eps2, 2) +
                 3.0/4.0 * pow(e_constants->Eps2, 3)
              - 77.0/128.0 * pow(e_constants->Eps2, 4);

        polx3b = 15.0 / 256.0 * polx3b;

        polx4b = pow(e_constants->Eps2, 3) -
                 41.0/32.0 * pow(e_constants->Eps2, 4);

        polx4b = polx4b * 35.0 / 3072.0;

        polx5b =  -315.0/131072.0 * pow(e_constants->Eps2, 4);

        gd_utm_spec->poly1b = 1.0
              - (1.0/4.0 * e_constants->Eps2)
              - (3.0/64.0 * pow(e_constants->Eps2, 2))
              - (5.0/256.0 * pow(e_constants->Eps2, 3))
              - (175.0/16384.0 * pow(e_constants->Eps2, 4));

        gd_utm_spec->poly2b = polx2b * -2.0 + polx3b * 4.0
                               - polx4b * 6.0
                               + polx5b * 8.0;

        gd_utm_spec->poly3b = polx3b * -8.0 + polx4b * 32.0
                               - polx5b * 80.0;

        gd_utm_spec->poly4b = polx4b * -32.0
                               + polx5b * 192.0;

        gd_utm_spec->poly5b = polx5b * -128.0;
    }

} /* end set_gd_to_utm_constants */

/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_utm_to_gd_constants
 *
 *   Compute and store the constants used for converting from UTM to GD.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   utm - a pointer to the parameters of a (source) UTM system
 *
 *   utm_gd_spec - a pointer to where the constants will be stored
 *                  for a UTM to GD conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the utm_gd_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void set_utm_to_gd_constants
(
    const SE_ERM_DATA                  *e_constants,
    const SE_SRF_PARAMETERS            *srf,
          SE_UTM_GD_SPECIFIC_CONSTANTS *utm_gd_spec
)
{
   SE_FLOAT64 polx1a, polx2a, polx4a, polx6a, polx8a,con;
   SE_FLOAT64 x_offset,y_offset;
   int        zone;

   if(srf->is_2D == SE_TRUE) /* get 2d parameters */
   {
      x_offset  = srf->u.params_2d.u.utm_parameters.x_offset;
      y_offset  = srf->u.params_2d.u.utm_parameters.y_offset;
      zone      = srf->u.params_2d.u.utm_parameters.zone;
   }
   else                      /* get 3d parameters */
   {
      x_offset  = srf->u.params_3d.u.utm_parameters.x_offset;
      y_offset  = srf->u.params_3d.u.utm_parameters.y_offset;
      zone      = srf->u.params_3d.u.utm_parameters.zone;
   }

   utm_gd_spec->x_offset = x_offset;
   utm_gd_spec->y_offset = y_offset;
   utm_gd_spec->zone     = zone;

   utm_gd_spec->EF     = e_constants->F/(2.0-e_constants->F);
   utm_gd_spec->Cnst3  = 30.0 / SE_PI ;
   utm_gd_spec->CScale = .9996;
   utm_gd_spec->Con2   = 2;
   utm_gd_spec->Con6   = .166666666666667;
   utm_gd_spec->Con24  = 4 * .0416666666666667;
   utm_gd_spec->Con120 = .00833333333333333;
   utm_gd_spec->Con720 = 4 * .001388888888888882;

   if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
   {
      con = 1.0 - (e_constants->Eps2);

      utm_gd_spec->Con2   = 2/con;
      utm_gd_spec->Con24  = utm_gd_spec->Con24/con;
      utm_gd_spec->Con720 = utm_gd_spec->Con720/con;

      polx1a = 1.0 - e_constants->Eps2 / 4.0
            - 3.0/   64.0 * pow(e_constants->Eps2, 2)
            - 5.0/  256.0 * pow(e_constants->Eps2, 3)
          - 175.0/16384.0 * pow(e_constants->Eps2, 4);

      utm_gd_spec->conap  = e_constants->A * polx1a;

      polx2a = 3.0/2.0 * utm_gd_spec->EF
           - 27.0/32.0 * pow(utm_gd_spec->EF, 3);

      polx4a = 21.0/16.0 * pow(utm_gd_spec->EF, 2)
            - 55.0/32.0 * pow(utm_gd_spec->EF, 4);

      polx6a = 151.0/96.0 * pow(utm_gd_spec->EF, 3);

      polx8a = 1097.0/512.0 * pow(utm_gd_spec->EF, 4);


      utm_gd_spec->polx2b = polx2a * 2.0 + polx4a * 4.0 +
                            polx6a * 6.0 + polx8a * 8.0;

      utm_gd_spec->polx3b = polx4a * -8.0 - polx6a * 32.0
                            - 80.0 *polx8a;
      utm_gd_spec->polx4b = polx6a * 32.0 + 192.0*polx8a;
      utm_gd_spec->polx5b = -128.0 *polx8a;
   }
   else /* compute spherical values */
      utm_gd_spec->conap=e_constants->A;
} /* end set_utm_to_gd_constants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: set_gc_to_gd_constants
 *
 *   Computes and stores the constants used for converting from GC to GD.
 *
 * PARAMETERS:
 *
 *   e_constants - pointer to the converted ellipse constants
 *
 *   gd - a pointer to the parameters of a (destination) GD system
 *
 *   gc_gd_spec - a pointer to where the constants will be stored
 *                  for a GC to GD conversion
 *
 * RETURNS: nothing
 *   Instead of returning a value, this function sets constants in the
 *   the structure pointed to by the gc_gd_spec parameter.
 *
 *----------------------------------------------------------------------------
 */
static void set_gc_to_gd_constants
(
    const SE_ERM_DATA                  *e_constants,
    const SE_SRF_PARAMETERS            *srf,
          SE_GC_GD_SPECIFIC_CONSTANTS *gc_gd_spec
)
{
   /*
    * Note:  GC has parameters, so pull ellipsoid from GD
    */

   SE_ELLIPSOID_ENUM ellipsoid;
   SE_FLOAT64 CE2;

   if(srf->is_2D == SE_TRUE)
   {
      ellipsoid = Datum_Table[srf->u.params_2d.u.gd_parameters.horizontal_datum].Ellipsoid_Code;
   }
   else
   {
      ellipsoid = Datum_Table[srf->u.params_3d.u.gd_parameters.horizontal_datum].Ellipsoid_Code;

   }

   CE2 = e_constants->A2 - e_constants->C2;

   gc_gd_spec->Eps21 = e_constants->Eps2 - 1.0;
   gc_gd_spec->C254   = 54.0 * e_constants->C2;
   gc_gd_spec->CEEps2 = e_constants->Eps2 * CE2;
   gc_gd_spec->CEE =  e_constants->Eps2 * e_constants->Eps2;
   gc_gd_spec->TwoCEE = 2 * gc_gd_spec->CEE;
   gc_gd_spec->C2DA   = e_constants->C2 / e_constants->A;
   gc_gd_spec->tem    = CE2 / e_constants->C2;

   gc_gd_spec->ARat1  = pow((e_constants->A + 50005.0),2);
   gc_gd_spec->ARat2  = (gc_gd_spec->ARat1) /
                         pow((e_constants->C+50005.0),2);
   gc_gd_spec->BRat1  = pow((e_constants->A-10005.0),2);
   gc_gd_spec->BRat2  = (gc_gd_spec->BRat1) /
                         pow((e_constants->C-10005.0),2);
   switch(ellipsoid)
   {
      case SE_AA_ELLIPSOID:
      gc_gd_spec->B1=0.100224625101002E+01;
      gc_gd_spec->B2=-0.390460083288143E-04;
      gc_gd_spec->B3=0.240320189037138E+12;
      gc_gd_spec->B4=0.133719260538966E+14;
      gc_gd_spec->B5=0.984592458596161E+00;
      break;

      case SE_AM_ELLIPSOID:
      gc_gd_spec->B1=0.100224625075368E+01;
      gc_gd_spec->B2=-0.390459999900173E-04;
      gc_gd_spec->B3=0.240303394872251E+12;
      gc_gd_spec->B4=0.133709931084284E+14;
      gc_gd_spec->B5=0.984592460957268E+00;

      case SE_AN_ELLIPSOID:
      gc_gd_spec->B1=0.100225444204291E+01;
      gc_gd_spec->B2=-0.393265865328642E-04;
      gc_gd_spec->B3=0.241224183614829E+12;
      gc_gd_spec->B4=0.133734497953958E+14;
      gc_gd_spec->B5=0.984537329904682E+00;
      break;

      case SE_BN_ELLIPSOID:
      gc_gd_spec->B1=0.100224755857767E+01;
      gc_gd_spec->B2=-0.390907329587448E-04;
      gc_gd_spec->B3=0.240451345711468E+12;
      gc_gd_spec->B4=0.133714386800225E+14;
      gc_gd_spec->B5=0.984583658241425E+00;
      break;

      case SE_BR_ELLIPSOID:
      gc_gd_spec->B1=0.100224755848503E+01;
      gc_gd_spec->B2=-0.390907295861776E-04;
      gc_gd_spec->B3=0.240444817155162E+12;
      gc_gd_spec->B4=0.133710761751998E+14;
      gc_gd_spec->B5=0.984583659174593E+00;
      break;

      case SE_CC_ELLIPSOID:
      gc_gd_spec->B1=0.100227973833064E+01;
      gc_gd_spec->B2=-0.401992216347506E-04;
      gc_gd_spec->B3=0.243880311870541E+12;
      gc_gd_spec->B4=0.133706476542196E+14;
      gc_gd_spec->B5=0.984367129494488E+00;
      break;

      case SE_CD_ELLIPSOID:
      gc_gd_spec->B1=0.100229163655042E+01;
      gc_gd_spec->B2=-0.406128691119799E-04;
      gc_gd_spec->B3=0.245130859537107E+12;
      gc_gd_spec->B4=0.133694175832394E+14;
      gc_gd_spec->B5=0.984287099560456E+00;
      break;

      case SE_EA_ELLIPSOID:
      gc_gd_spec->B1=0.100223509569433E+01;
      gc_gd_spec->B2=-0.386654527077781E-04;
      gc_gd_spec->B3=0.239128650594500E+12;
      gc_gd_spec->B4=0.133720475335561E+14;
      gc_gd_spec->B5=0.984667548438242E+00;
      break;

      case SE_EB_ELLIPSOID:
      gc_gd_spec->B1=0.100223509571370E+01;
      gc_gd_spec->B2=-0.386654536421315E-04;
      gc_gd_spec->B3=0.239130314228725E+12;
      gc_gd_spec->B4=0.133721404499933E+14;
      gc_gd_spec->B5=0.984667548190955E+00;
      break;

      case SE_EC_ELLIPSOID:
      gc_gd_spec->B1=0.100223509572519E+01;
      gc_gd_spec->B2=-0.386654535736479E-04;
      gc_gd_spec->B3=0.239130514500415E+12;
      gc_gd_spec->B4=0.133721515789086E+14;
      gc_gd_spec->B5=0.984667548183952E+00;
      break;

      case SE_ED_ELLIPSOID:
      gc_gd_spec->B1=0.100223509571418E+01;
      gc_gd_spec->B2=-0.386654534742569E-04;
      gc_gd_spec->B3=0.239130097288096E+12;
      gc_gd_spec->B4=0.133721283147546E+14;
      gc_gd_spec->B5=0.984667548227722E+00;
      break;

      case SE_EE_ELLIPSOID:
      gc_gd_spec->B1=0.100223509572941E+01;
      gc_gd_spec->B2=-0.386654536185472E-04;
      gc_gd_spec->B3=0.239130725532003E+12;
      gc_gd_spec->B4=0.133721633543519E+14;
      gc_gd_spec->B5=0.984667548165743E+00;
      break;

      case SE_FA_ELLIPSOID:
      gc_gd_spec->B1=0.100225438677758E+01;
      gc_gd_spec->B2=-0.393246903633930E-04;
      gc_gd_spec->B3=0.241216653453483E+12;
      gc_gd_spec->B4=0.133733602228679E+14;
      gc_gd_spec->B5=0.984537701867943+00;
      break;

      case SE_HE_ELLIPSOID:
      gc_gd_spec->B1=0.100225405980964E+01;
      gc_gd_spec->B2=-0.393134714450825E-04;
      gc_gd_spec->B3=0.241187107678002E+12;
      gc_gd_spec->B4=0.133736622218590E+14;
      gc_gd_spec->B5=0.984539902154414E+00;
      break;

      case SE_HO_ELLIPSOID:
      gc_gd_spec->B1=0.100226404159165E+01;
      gc_gd_spec->B2=-0.396566479838024E-04;
      gc_gd_spec->B3=0.242239266725463E+12;
      gc_gd_spec->B4=0.133727724324161E+14;
      gc_gd_spec->B5=0.984472732821240E+00;
      break;

      case SE_IN_ELLIPSOID:
      gc_gd_spec->B1=0.100226404171336E+01;
      gc_gd_spec->B2=-0.396566527027543E-04;
      gc_gd_spec->B3=0.242248216831710E+12;
      gc_gd_spec->B4=0.133732658091673E+14;
      gc_gd_spec->B5=0.984472731538793E+00;
      break;

      case SE_KA_ELLIPSOID:
      gc_gd_spec->B1=0.100225405984221E+01;
      gc_gd_spec->B2=-0.393134735007025E-04;
      gc_gd_spec->B3=0.241190507521621E+12;
      gc_gd_spec->B4=0.133738505527850E+14;
      gc_gd_spec->B5=0.984539901633388E+00;
      break;

      case SE_RF_ELLIPSOID:
      gc_gd_spec->B1=0.100225438679850E+01;
      gc_gd_spec->B2=-0.393246905835223E-04;
      gc_gd_spec->B3=0.241216653573729E+12;
      gc_gd_spec->B4=0.133733601038204E+14;
      gc_gd_spec->B5=0.984537701810060E+00;
      break;

      case SE_SA_ELLIPSOID:
      gc_gd_spec->B1=0.100225444204291E+01;
      gc_gd_spec->B2=-0.393265865328642E-04;
      gc_gd_spec->B3=0.241224183614829E+12;
      gc_gd_spec->B4=0.133734497953958E+14;
      gc_gd_spec->B5=0.984537329904682E+00;
      break;

      case SE_WD_ELLIPSOID:
      gc_gd_spec->B1= 0.100225436554849E+01;
      gc_gd_spec->B2=-0.393239617638985E-04;
      gc_gd_spec->B3= 0.241214275377192E+12;
      gc_gd_spec->B4= 0.133733543378504E+14;
      gc_gd_spec->B5= 0.984537844768728E+00;
      break;

      case SE_WE_ELLIPSOID:
      default:

      gc_gd_spec->B1=0.100225438677758E+01;
      gc_gd_spec->B2=-0.393246903633930E-04;
      gc_gd_spec->B3=0.241216653453483E+12;
      gc_gd_spec->B4=0.133733602228679E+14;
      gc_gd_spec->B5=0.984537701867943E+00;

   } /* end switch */
} /* end set_gc_to_gd_constants */


/*
 * ENUM: SE_CONVERT_TYPE_ENUM
 *
 *   Used to specify to conversion type taking place
 *
 */
typedef enum
{
   NO_CONVERSION,
   E_TO_G,  /* only used to signify height conversion: ellipsoid to geoid */
   G_TO_E,  /* only used to signify height conversion: geoid to ellipsoid */
   GD_TO_LTP,  /* * These                                                    */
   LTP_TO_GD, /* are only used for Vector Conversions                     */
   LTP_TO_LTP,
   M_TO_LTP,
   LTP_TO_M,
   EC_TO_LTP,
   LTP_TO_EC,
   PS_TO_LTP,
   LTP_TO_PS,
   TM_TO_LTP,
   LTP_TO_TM,
   UTM_TO_LTP,
   LTP_TO_UTM,
   LTP_TO_LCC,
   LCC_TO_LTP, /* this is the end of the vector only types                */

   GC_LTP,      /* the rest are both coordinate and some vector types      */
   LTP_GC,
   GD_M,
   M_GD,
   OM_OM,
   GD_EC,
   EC_GD,
   GC_GD,
   GD_GC,
   UTM_GD,
   GD_UTM,
   GD_TM,
   TM_GD,
   GD_PS,
   PS_GD,
   LCC1_GD,
   GD_LCC1,
   LCC2_GD,
   GD_LCC2,
   GC_GC,
   GM_GM,
   GD_GD,
   GEI_GEI,
   GSE_GSE,
   GSM_GSM,
   SM_SM,
   GCS_GCS,
   LSR_LSR,
   UPS_UPS
} SE_CONVERT_TYPE_ENUM;

/*
 * STRUCT: SE_CONVERT_COORD_SYSTEM_CONSTANTS
 *
 */
typedef struct SE_Coord_System_Constants
{
   /*
    * units for the source spatial reference frame
    */
    SRM_ELEVATION_UNITS_ENUM source_units;

   /*
    * units for the destination spatial reference frame
    */
    SRM_ELEVATION_UNITS_ENUM dest_units;

   /*
    * the transformation type - also selects which field is valid
    * within the following union
    */

    /* specifies coodinate conversion type                       */
    SE_CONVERT_TYPE_ENUM convert_type;

    /* specifies the height conversion type                        */
    SE_CONVERT_TYPE_ENUM height_convert_type;

    /* specifies the vector conversion type                       */
    SE_CONVERT_TYPE_ENUM vector_convert_type;

    /* this flag is set to denote that the dest srf is 2D or 3D */
    SE_BOOLEAN dest_srf_is_2D;

    /* this flag is set in SRM_ConvergenceOfMeridian
     * to compute and save the sine and cos of COM/gamma
     * when necessary
     */

    SE_BOOLEAN compute_com;

    SE_FLOAT64 sin_gamma, cos_gamma;
   /*
    * pre-computed constants specific to a transformation
    */
    union
    {
        SE_LTP_SPECIFIC_CONSTANTS     ltp;
        SE_M_GD_SPECIFIC_CONSTANTS    m_gd;
        SE_GD_M_SPECIFIC_CONSTANTS    gd_m;
        SE_OM_OM_SPECIFIC_CONSTANTS   om_om;
        SE_EC_SPECIFIC_CONSTANTS      ec;
        SE_GC_GD_SPECIFIC_CONSTANTS   gc_gd;
        SE_GD_GC_SPECIFIC_CONSTANTS   gd_gc;
        SE_TM_GD_SPECIFIC_CONSTANTS   tm_gd;
        SE_GD_TM_SPECIFIC_CONSTANTS   gd_tm;
        SE_UTM_GD_SPECIFIC_CONSTANTS  utm_gd;
        SE_GD_UTM_SPECIFIC_CONSTANTS  gd_utm;
        SE_GD_PS_SPECIFIC_CONSTANTS   gd_ps;
        SE_PS_GD_SPECIFIC_CONSTANTS   ps_gd;
        SE_LCC1_GD_SPECIFIC_CONSTANTS lcc1_gd;
        SE_GD_LCC1_SPECIFIC_CONSTANTS gd_lcc1;
        SE_LCC2_GD_SPECIFIC_CONSTANTS lcc2_gd;
        SE_GD_LCC2_SPECIFIC_CONSTANTS gd_lcc2;
        SE_GC_GC_SPECIFIC_CONSTANTS   gc_gc;
        SE_GM_GM_SPECIFIC_CONSTANTS   gm_gm;
        SE_GD_GD_SPECIFIC_CONSTANTS   gd_gd;
        SE_GEI_GEI_SPECIFIC_CONSTANTS gei_gei;
        SE_GSE_GSE_SPECIFIC_CONSTANTS gse_gse;
        SE_GSM_GSM_SPECIFIC_CONSTANTS gsm_gsm;
        SE_SM_SM_SPECIFIC_CONSTANTS   sm_sm;
        SE_GCS_GCS_SPECIFIC_CONSTANTS gcs_gcs;
        SE_LSR_LSR_SPECIFIC_CONSTANTS lsr_lsr;
        SE_UPS_UPS_SPECIFIC_CONSTANTS ups_ups;
    } specific_constants;
} SE_COORD_SYSTEM_CONSTANTS;

/*
 * DEFINE: SRM_CONSTANTS_DEPTH
 *
 *   This #define speficies the maximum size of the constants
 *   array that is a member of SE_ConvertCoordSystemParams
 *   Currently, the most constants every needed is 3
 */
#define SRM_CONSTANTS_DEPTH 3

/*
 * STRUCT: SE_CONVERT_COORD_SYSTEM_PARAMS
 *
 *   This structure holds the conversion parameters
 *   and any pre-computed constants
 */
typedef struct SE_ConvertCoordSystemParams
{
   /*
    * The pre-computed ERM data
    *
    * These constants are computed based on the horizontal datum of the
    * source system.  In those cases where "chained" or "transitive"
    * conversions are being done, the intermediate GD system will use the
    * same horizontal datum as the original source system, so the same
    * ERM constants will apply.
    */
   SE_ERM_DATA ellipse_constants;

   /* This is an array of pointers to SE_COORD_SYSTEM_CONSTANTS
    * the size of the array is #defined to be SRM_CONSTANTS_DEPTH,
    * which should equal the maximum number of constants ever required
    * in a chained conversion.
    */

   SE_COORD_SYSTEM_CONSTANTS *constants[SRM_CONSTANTS_DEPTH];

   /*
    * This additional constants pointers  are only used for "chained" or
    * "transitive" conversions.  For example, if the user requests
    * a UTM to LCC conversion, it would be stored in this structure
    * as two separate sets of conversions constants.  The UTM-to-GD
    * constants would be stored at index 0, and
    * the GD-to-LCC constants would be stored at index 1.  Index 2 will be
    * set to NULL.
    */

    /* Other internal data can be stored here as well */
} SE_CONVERT_COORD_SYSTEM_PARAMS;


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_CheckCoordinateInSRF
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SRM_CheckCoordinateInSRF
(
    const SE_SRF_PARAMETERS *SRF_params_ptr,
    const SE_COORD          *source_coord_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (!SE_ValidSRFParameters(SRF_params_ptr, SE_TRUE, NULL))
    {
        status = SE_COORD_INVALID_SOURCE_SRF_PARAMETERS;
    }
    else if (!SE_ValidCoord(source_coord_ptr, SE_TRUE, NULL))
    {
        status = SE_COORD_INVALID_SOURCE_COORD;
    }
    else if (SRF_params_ptr->is_2D)
    {
        if (!source_coord_ptr->is_2D)
        {
           /*
            * 3D points aren't defined in 2D SRFs
            */
            status = SE_COORD_INVALID_SOURCE_COORD;
        }
        else if (SRF_params_ptr->u.params_2d.spatial_reference_frame
              != source_coord_ptr->u.coord_2d.spatial_reference_frame)
        {
           /*
            * point is not defined in this scope
            */
            status = SE_COORD_INVALID_SOURCE_COORD;
        }
       /*
        *--------------------------------------------
        * $$$ need to add further checks here for UTM 2D
        *--------------------------------------------
        */
    }
    else if (source_coord_ptr->is_2D)
    {
        SRM_SRF_3D_ENUM src_3d_srf;

       /*
        * SRF_params_ptr is 3D, but source_coord_ptr is 2D
        */
        if (SRM_GetCorresponding3DSRF(source_coord_ptr->u.coord_2d.
            spatial_reference_frame, &src_3d_srf) != SE_COORD_SUCCESS)
        {
           /*
            * There is no 3D SRF corresponding to the
            * 2D SRF of source_coord_ptr
            */
            status = SE_COORD_FAILURE;
        }
        else if (SRF_params_ptr->u.params_3d.spatial_reference_frame
               != src_3d_srf)
        {
           /*
            * point is not defined in this scope
            */
            status = SE_COORD_INVALID_SOURCE_COORD;
        }

       /*
        *--------------------------------------------
        * $$$ need to add further checks here for UTM
        *--------------------------------------------
        */
    }
    else
    {
       /*
        * Both SRF_params_ptr and source_coord_ptr are 3D
        */
        if (SRF_params_ptr->u.params_3d.spatial_reference_frame
         != source_coord_ptr->u.coord_3d.spatial_reference_frame)
        {
           /*
            * point is not defined in this scope
            */
            status = SE_COORD_INVALID_SOURCE_COORD;
        }
       /*
        *--------------------------------------------
        * $$$ need to add further checks here for GCS, UTM
        *--------------------------------------------
        */
    }
    return status;
} /* end SRM_CheckCoordinateInSRF */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_InitializeGeoid
 *
 *   Reads geoid separation data from the file indicated by the
 *   $SE_WW15MGH_GRD_PATH environment variable, and
 *   builds the geoid separation table from it.
 *
 * RETURNS:
 *
 *   SE_COORD_SUCCESS - if the geoidal separation table has already been
 *     loaded, or was successfully loaded during this function call.
 *
 *   SE_COORD_MISSING_WGS84_GEOID_SEPARATION_FILE - if the environment
 *     variable specifying the location of the geoidal separation file
 *     has not been set, or if the file specified cannot be opened
 *     in read-only mode.
 *
 *   SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE - if the specified
 *     file is invalid.
 *
 *   SE_COORD_FAILURE - if memory could not be allocated for the geoidal
 *     separation table.
 *
 *----------------------------------------------------------------------------
 */

#include "ww15mgh16.c"

static SE_UINT32  GeoidScaleFactor,NumbGeoidCols,NumbGeoidRows;

static SE_COORD_STATUS_CODE_ENUM
SRM_InitializeGeoid(void)
{
         SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;
   const SE_INT16    NumbHeaderItems = 6;
         SE_INT32    ItemsDiscarded, ItemsRead;
         FILE       *GeoidHeightFile;
         SE_FLOAT32  geoid_header[6];
         SE_UINT32   size_of_array,rows,cols,ElevationsRead;
         char       *FileName;

#ifdef CONV_DEBUG
    FILE                           *f_ptr;

    f_ptr = SE_GetErrorFilePtr();
#endif

   /*
    * if the table has already been loaded, return success
    */
    if (SE_separation_table != NULL)
        return status;

    FileName = getenv("SE_WW15MGH_GRD_PATH");

    if(FileName==NULL)
    {
      SE_separation_table = default_geoid_sep;
      
      GeoidScaleFactor = (SE_UINT32) (1.0/default_geoid_header[4]);

      rows = (SE_UINT32) ((default_geoid_header[1] - default_geoid_header[0]) /
			  default_geoid_header[4]) +1;

      cols = (SE_UINT32) ((default_geoid_header[3] - default_geoid_header[2]) /
			  default_geoid_header[5]) +1;

      NumbGeoidCols = cols;
      NumbGeoidRows = rows;

      return status;
    }

   /*
    *  Open the File READONLY, or Return Error Condition
    */
    if (( GeoidHeightFile = fopen(FileName, "r")) == NULL)
    {
#ifdef CONV_DEBUG
        fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_InitializeGeoid]\n"\
                "    Can't open file: %s!",FileName);
#endif
        return SE_COORD_MISSING_WGS84_GEOID_SEPARATION_FILE;
    }

   /*
    *  Skip the Header Line within the file.
    */
    ItemsDiscarded = 0;
    while ((status == SE_COORD_SUCCESS) &&
           (ItemsDiscarded < NumbHeaderItems))
    {
        if (feof(GeoidHeightFile))
           status = SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
        else if (ferror(GeoidHeightFile))
           status = SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
        else
        {
            ItemsRead = fscanf(GeoidHeightFile, "%f",
                               &geoid_header[ItemsDiscarded]);

            if (ItemsRead > 0)
            {
              ItemsDiscarded = ItemsDiscarded + 1;
            }
        }
    }

   /*
    *  Determine if header read properly, or NOT
    */
    if ((status != SE_COORD_SUCCESS) ||
        geoid_header[0] !=  -90.0 ||
        geoid_header[1] !=   90.0 ||
        geoid_header[2] !=    0.0 ||
        geoid_header[3] !=  360.0 ||
        geoid_header[4] !=  .250000 ||
        geoid_header[5] !=  .250000 ||
        ItemsDiscarded != NumbHeaderItems)
    {
#ifdef CONV_DEBUG
        fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_InitializeGeoid]\n"\
                "    Error reading geoid separation table header!");
#endif
        return SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
    }

   /*
    * Allocate space for heights array (3 step process)
    *
    * (1) compute rows by dividing total degrees of latitude by the
    *     step size + 1 (should equal 721 rows)
    *
    * (2) compute columns by dividing total degrees of longitude by
    *     the step size + 1 (should equal 1441 columns)
    *
    * (3) allocate memory
    */
    rows = (SE_UINT32) ((geoid_header[1] - geoid_header[0]) /
                        geoid_header[4]) +1;

    cols = (SE_UINT32) ((geoid_header[3] - geoid_header[2]) /
                        geoid_header[5]) +1;
    size_of_array = rows * cols;

    GeoidScaleFactor = (SE_UINT32) (1.0/geoid_header[4]);
    NumbGeoidCols = cols;
    NumbGeoidRows = rows;

    if ((SE_separation_table = (SE_FLOAT32 *) calloc(size_of_array,
        sizeof(SE_FLOAT32))) == NULL)
    {
#ifdef CONV_DEBUG
        fprintf(f_ptr, "[SEDRIS CONVERSIONS::SRM_InitializeGeoid]\n"\
                "    Not enough free memory.  %i bytes required",
        size_of_array * sizeof(SE_FLOAT32));
#endif
        status = SE_COORD_FAILURE;
    }
    else
    {
       /*
        *  Extract elements from the file
        */
        ElevationsRead = 0;
        while ((status == SE_COORD_SUCCESS) &&
               (ElevationsRead < size_of_array))
        {
            if (feof(GeoidHeightFile))
                status = SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
            else if (ferror(GeoidHeightFile))
                status = SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
            else
            {
                ItemsRead = fscanf(GeoidHeightFile, "%f",
                                   &SE_separation_table[ElevationsRead]);
                if (ItemsRead > 0)
                {
                    ElevationsRead = ElevationsRead + 1;
                }
            }
        }

       /*
        *  Determine if all elevations of file read properly, or NOT
        */
        if ((status != SE_COORD_SUCCESS) ||
            (ElevationsRead != size_of_array))
        {
#ifdef CONV_DEBUG
            fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_InitializeGeoid]\n"\
                    "    Error reading geoid separation table header!");
#endif
            status = SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
        }
    }
    fclose(GeoidHeightFile);
    return status;
}


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_GetWGS84GeoidalSeparation
 *
 *   Returns the height of the WGS84 geoid above or below the WGS84
 *   ellipsoid, at the specified geodetic coordinates, using a grid
 *   of height adjustments from the EGM96 gravity model.
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SRM_GetWGS84GeoidalSeparation
(
    const SRM_GD_2D_COORD *WGS84GeodeticLocation_ptr,
          SE_FLOAT64      *GeoidalSeparation_ptr
)
{
    SE_UINT32  Index;
    SE_FLOAT64 DeltaX, DeltaY,
               ElevationSE, ElevationSW, ElevationNE, ElevationNW,
               OffsetX, OffsetY, PostX, PostY, UpperY, LowerY;
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

   /*
    * If the geoidal separation table hasn't been loaded, then load it now.
    * If there's an error in loading, we must return.
    */
    if ((SE_separation_table == NULL) &&
        ((status=SRM_InitializeGeoid()) != SE_COORD_SUCCESS))
    {
        return status;
    }

   /*
    *  Compute X and Y Offsets into Geoid Height Array.
    */
    if (WGS84GeodeticLocation_ptr->geodetic_longitude < 0.0)
    {
        OffsetX = (WGS84GeodeticLocation_ptr->geodetic_longitude + 360.0)
                  * GeoidScaleFactor;
    }
    else
    {
        OffsetX = WGS84GeodeticLocation_ptr->geodetic_longitude
	  * GeoidScaleFactor;
    }

    OffsetY = (90.0 - WGS84GeodeticLocation_ptr->geodetic_latitude)
              * GeoidScaleFactor;

   /*
    *  Find Four Nearest Geoid Height Cells for specified Latitude, Longitude;
    *  Assumes that (0,0) of Geoid Height Array is at Northwest corner.
    */
    PostX = floor( OffsetX );
    if ((PostX + 1) == NumbGeoidCols)
        PostX--;
    PostY = floor( OffsetY );
    if ((PostY + 1) == NumbGeoidRows)
        PostY--;

    Index = (SE_UINT32)(PostY * NumbGeoidCols + PostX);
    ElevationNW = SE_separation_table[ Index ];
    ElevationNE = SE_separation_table[ Index+ 1 ];

    Index = (SE_UINT32)((PostY + 1) * NumbGeoidCols + PostX);
    ElevationSW = SE_separation_table[ Index ];
    ElevationSE = SE_separation_table[ Index + 1 ];

   /*
    *  Perform Bi-Linear Interpolation to compute Height above Ellipsoid
    */
    DeltaX = OffsetX - PostX;
    DeltaY = OffsetY - PostY;

    UpperY = ElevationNW + DeltaX * ( ElevationNE - ElevationNW );
    LowerY = ElevationSW + DeltaX * ( ElevationSE - ElevationSW );

   *GeoidalSeparation_ptr = UpperY + DeltaY * ( LowerY - UpperY );

    return status;
}


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: CreateConstants
 *
 *----------------------------------------------------------------------------
 */
static SE_COORD_STATUS_CODE_ENUM
CreateConstants
(
          SE_CONVERT_COORD_SYSTEM_PARAMS *output_ptr,
          int                             c_index,
          SE_CONVERT_TYPE_ENUM            convert_type,
          SE_CONVERT_TYPE_ENUM            height_convert_type,
    const SE_SRF_PARAMETERS              *src_srf_ptr,
    const SE_SRF_PARAMETERS              *dest_srf_ptr,
          SE_ERM_DATA                    *e_constants
)
{
   SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

   /* if debugging is enabled, set the stderr pointer */

#ifdef CONV_DEBUG
   FILE                           *f_ptr;

   f_ptr = SE_GetErrorFilePtr();
#endif

   if(c_index < 0 || c_index >= SRM_CONSTANTS_DEPTH)
      status = SE_COORD_FAILURE;


   if(status == SE_COORD_SUCCESS)
   {
      /*
       * Make sure that there aren't any constants already here. If there are,
       * it is a coding error in the calling sequence in
       * SE_CreateCoordinateConversionConstants().
       */
      if(output_ptr->constants[c_index] != NULL)
      {
#ifdef CONV_DEBUG
         fprintf(f_ptr,"[SEDRIS CONVERSIONS::CreateConstants] Internal "\
                 "error, attempted to re-allocate constants!");
#endif
          status = SE_COORD_FAILURE;
      }
      else
      {
          output_ptr->constants[c_index] = (SE_COORD_SYSTEM_CONSTANTS *)
                   calloc(1, sizeof(SE_COORD_SYSTEM_CONSTANTS));

          if (!output_ptr->constants[c_index])
              status = SE_COORD_FAILURE;
      }
   }
   if (status != SE_COORD_SUCCESS)
       return status;

   /*
    * Get units only if they exist.
    */
    if(src_srf_ptr->is_2D==SE_FALSE && dest_srf_ptr->is_2D==SE_FALSE)
    {
       /*
        * if these are the initial constants, set the source/dest units
        */
        if (c_index == 0)
        {
           /*
            * setup the source and destination units for the first conversion
            */
            output_ptr->constants[0]->source_units =
               SE_GetUnits(&(src_srf_ptr->u.params_3d));
            output_ptr->constants[0]->dest_units =
               SE_GetUnits(&(src_srf_ptr->u.params_3d));
        }
        else
        {
           /*
            * set the units based on the dest of the initial constants
            */
            output_ptr->constants[c_index]->source_units =
               output_ptr->constants[0]->dest_units;
            output_ptr->constants[c_index]->dest_units =
               output_ptr->constants[0]->dest_units;
        }
    }


  /*
   * initialize conversion types
   */
   output_ptr->constants[c_index]->convert_type = convert_type;

   output_ptr->constants[c_index]->height_convert_type = height_convert_type;


   /*
    * vector_convert_type is sometimes equal to the convert_type.
    * It will be changed later if required.
    */
    output_ptr->constants[c_index]->vector_convert_type = convert_type;

    switch(convert_type)
    {
       /* do the special case X to X cases first */
        case LSR_LSR:
             status = set_lsr_constants
                      (
                          src_srf_ptr,
                          dest_srf_ptr,
                         &output_ptr->constants[c_index]->
                          specific_constants.lsr_lsr
                      );
             break;

        case GCS_GCS:     /* GCS is only 3d      */
             if (dest_srf_ptr->is_2D == SE_FALSE)
             {
                 switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
                 {
                     case SRM_GCS_3D_SRF: /* GCS to GCS */
                         /* only 3D */
                          output_ptr->constants[c_index]->specific_constants.
                              gcs_gcs.x_offset =
                              src_srf_ptr->u.params_3d.u.gcs_parameters.
                                      x_offset -
                              dest_srf_ptr->u.params_3d.u.gcs_parameters.
                                      x_offset;

                          output_ptr->constants[c_index]->specific_constants.
                              gcs_gcs.y_offset =
                              src_srf_ptr->u.params_3d.u.gcs_parameters.
                                       y_offset -
                              dest_srf_ptr->u.params_3d.u.gcs_parameters.
                                       y_offset;
                          break;

                    default:
#ifdef CONV_DEBUG
             fprintf(f_ptr,
             conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
             SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
             SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));
#endif
                         status = SE_COORD_UNSUPPORTED;
                         break;
                 }
             } /* else dest is 2D, which is an error */
             else
             {
#ifdef CONV_DEBUG
          fprintf(f_ptr,
          conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
          SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
          SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
#endif
                 status = SE_COORD_UNSUPPORTED;
             }
             break;

        case GEI_GEI:
            /* GEI is only 3D */
            if (dest_srf_ptr->is_2D == SE_FALSE)
            {
                switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
                {
                    case SRM_GEI_3D_SRF: /* GEI to GEI */
                         if(src_srf_ptr->u.params_3d.u.gei_parameters.epoch !=
                            dest_srf_ptr->u.params_3d.u.gei_parameters.epoch)
                         {
#ifdef CONV_DEBUG
               fprintf(f_ptr, conv_error_msg[SE_GEI2GEI_EPOCH_ERROR]);
#endif
                             status = SE_COORD_UNSUPPORTED;
                         }
                         break;

                    default:
#ifdef CONV_DEBUG
             fprintf(f_ptr,
             conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
             SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
             SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));
#endif
                         status = SE_COORD_UNSUPPORTED;
                         break;
                }
            }
            else /* dest is 2D, which is an error */
            {
#ifdef CONV_DEBUG
             fprintf(f_ptr,
             conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
             SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
             SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
#endif
                status = SE_COORD_UNSUPPORTED;
            }
            break;

       case OM_OM:
            if (dest_srf_ptr->is_2D == SE_FALSE) /* dest is 3D */
      {
         switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
         {
            case SRM_OM_3D_SRF: /* OM to OM */

            if (( src_srf_ptr->u.params_3d.u.om_parameters.horizontal_datum !=
                  dest_srf_ptr->u.params_3d.u.om_parameters.horizontal_datum) ||
                ( src_srf_ptr->u.params_3d.u.om_parameters.vertical_datum !=
                  dest_srf_ptr->u.params_3d.u.om_parameters.vertical_datum) ||
                ( src_srf_ptr->u.params_3d.u.om_parameters.geodetic_latitude1 !=
                  dest_srf_ptr->u.params_3d.u.om_parameters.geodetic_latitude1) ||
                ( src_srf_ptr->u.params_3d.u.om_parameters.geodetic_latitude2 !=
                  dest_srf_ptr->u.params_3d.u.om_parameters.geodetic_latitude2) ||
                ( src_srf_ptr->u.params_3d.u.om_parameters.geodetic_longitude1 !=
                  dest_srf_ptr->u.params_3d.u.om_parameters.geodetic_longitude1) ||
                ( src_srf_ptr->u.params_3d.u.om_parameters.geodetic_longitude2 !=
                  dest_srf_ptr->u.params_3d.u.om_parameters.geodetic_longitude2) ||
                ( src_srf_ptr->u.params_3d.u.om_parameters.central_scale_factor !=
                  dest_srf_ptr->u.params_3d.u.om_parameters.central_scale_factor)
               )
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr, conv_error_msg[SE_OM2OM_ERROR]);
#endif
               status = SE_COORD_UNSUPPORTED;
            }
            else
            {
               output_ptr->constants[c_index]->specific_constants.om_om.x_offset =
                  src_srf_ptr->u.params_3d.u.om_parameters.x_offset -
                  dest_srf_ptr->u.params_3d.u.om_parameters.x_offset;
               output_ptr->constants[c_index]->specific_constants.om_om.y_offset =
                  src_srf_ptr->u.params_3d.u.om_parameters.y_offset -
                  dest_srf_ptr->u.params_3d.u.om_parameters.y_offset;

            }
            break;

            default:
#ifdef CONV_DEBUG
             fprintf(f_ptr,
             conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
             SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
             SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));
#endif
            status = SE_COORD_UNSUPPORTED;
            break;
         } /* end switch */
      } /* end OM to OM dest 3D */
      else /* dest OM is 2D    */
      {
         switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
         {
            case SRM_OM_2D_SRF:
            if (( src_srf_ptr->u.params_3d.u.om_parameters.horizontal_datum !=
               dest_srf_ptr->u.params_2d.u.om_parameters.horizontal_datum) ||
             ( src_srf_ptr->u.params_3d.u.om_parameters.geodetic_latitude1 !=
               dest_srf_ptr->u.params_2d.u.om_parameters.geodetic_latitude1) ||
             ( src_srf_ptr->u.params_3d.u.om_parameters.geodetic_latitude2 !=
               dest_srf_ptr->u.params_2d.u.om_parameters.geodetic_latitude2) ||
             ( src_srf_ptr->u.params_3d.u.om_parameters.geodetic_longitude1 !=
               dest_srf_ptr->u.params_2d.u.om_parameters.geodetic_longitude1) ||
             ( src_srf_ptr->u.params_3d.u.om_parameters.geodetic_longitude2 !=
               dest_srf_ptr->u.params_2d.u.om_parameters.geodetic_longitude2) ||
             ( src_srf_ptr->u.params_3d.u.om_parameters.central_scale_factor !=
               dest_srf_ptr->u.params_2d.u.om_parameters.central_scale_factor)
            )
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr, conv_error_msg[SE_OM2OM_ERROR]);
#endif
               status = SE_COORD_UNSUPPORTED;
            }
            else
            {
                output_ptr->constants[c_index]->specific_constants.
                    om_om.x_offset =
                  src_srf_ptr->u.params_3d.u.om_parameters.x_offset -
                  dest_srf_ptr->u.params_2d.u.om_parameters.x_offset;
                output_ptr->constants[c_index]->specific_constants.
                    om_om.y_offset =
                  src_srf_ptr->u.params_3d.u.om_parameters.y_offset -
                  dest_srf_ptr->u.params_2d.u.om_parameters.y_offset;
            }

         default:
#ifdef CONV_DEBUG
             fprintf(f_ptr,
             conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
             SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
             SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
#endif
         status = SE_COORD_UNSUPPORTED;
         } /* end switch OM to OM dest 2D */
      } /* end OM to X */
      break;

       case UPS_UPS:
      if(dest_srf_ptr->is_2D == SE_FALSE)
      {
         switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
         {
            case SRM_UPS_3D_SRF: /* only ups to ups is supported */
            if((src_srf_ptr->u.params_3d.u.ups_parameters.vertical_datum !=
                dest_srf_ptr->u.params_3d.u.ups_parameters.vertical_datum) ||
               (src_srf_ptr->u.params_3d.u.ups_parameters.pole !=
                dest_srf_ptr->u.params_3d.u.ups_parameters.pole))
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr,conv_error_msg[SE_UPS2UPS_ERROR]);
#endif
                status = SE_COORD_UNSUPPORTED;
            }
            else
            {
                output_ptr->constants[c_index]->convert_type = UPS_UPS;
                output_ptr->constants[c_index]->specific_constants.
                      ups_ups.x_offset =
                   src_srf_ptr->u.params_3d.u.ups_parameters.x_offset -
                   dest_srf_ptr->u.params_3d.u.ups_parameters.x_offset;

                output_ptr->constants[c_index]->specific_constants.
                   ups_ups.y_offset =
                   src_srf_ptr->u.params_3d.u.ups_parameters.y_offset -
                   dest_srf_ptr->u.params_3d.u.ups_parameters.y_offset;
            }
            break;

            default:
#ifdef CONV_DEBUG
                fprintf(f_ptr,
                conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                SRM_PrintSRF3D(src_srf_ptr->u.params_3d.
                              spatial_reference_frame),
                SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.
                              spatial_reference_frame));
#endif

            status = SE_COORD_UNSUPPORTED;
         }
      }
      else
      {
          switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
          {
            case SRM_UPS_2D_SRF: /* only ups to ups is supported */
            if((src_srf_ptr->u.params_3d.u.ups_parameters.pole !=
                dest_srf_ptr->u.params_2d.u.ups_parameters.pole))
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr,conv_error_msg[SE_UPS2UPS_ERROR]);
#endif
                status = SE_COORD_UNSUPPORTED;
            }
            else
            {
                output_ptr->constants[c_index]->convert_type = UPS_UPS;

                output_ptr->constants[c_index]->specific_constants.
                            ups_ups.x_offset =
                   src_srf_ptr->u.params_3d.u.ups_parameters.x_offset -
                   dest_srf_ptr->u.params_2d.u.ups_parameters.x_offset;

                output_ptr->constants[c_index]->specific_constants.
                            ups_ups.y_offset =
                   src_srf_ptr->u.params_3d.u.ups_parameters.y_offset -
                   dest_srf_ptr->u.params_2d.u.ups_parameters.y_offset;
            }
            break;

            default:
#ifdef CONV_DEBUG
                fprintf(f_ptr,
                conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                SRM_PrintSRF3D(src_srf_ptr->u.params_3d.
                              spatial_reference_frame),
                SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.
                              spatial_reference_frame));
#endif

            status = SE_COORD_UNSUPPORTED;
         }
      }
      break;

       case GM_GM:
       if(dest_srf_ptr->is_2D == SE_FALSE) /* GM TO X_3D */
       {
         switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
         {
            case SRM_GM_3D_SRF: /* GM to GM */
            if (SRM_SRFParametersEqual == SE_FALSE)
            {
#ifdef CONV_DEBUG
                fprintf(f_ptr,
                conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                SRM_PrintSRF3D(src_srf_ptr->u.params_3d.
                              spatial_reference_frame),
                SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.
                              spatial_reference_frame));
#endif
                status = SE_COORD_UNSUPPORTED;
            }
            break;

            default:
#ifdef CONV_DEBUG
             fprintf(f_ptr,
             conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
             SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
             SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));
#endif

            status = SE_COORD_UNSUPPORTED;
         } /* end switch GM to X */
      }
      else /* dest is 2D, this is an error */
      {
#ifdef CONV_DEBUG
          fprintf(f_ptr,
          conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
          SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
          SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
#endif

         status = SE_COORD_UNSUPPORTED;
      }
      break;

      case GSE_GSE:
      if (dest_srf_ptr->is_2D == SE_FALSE)
      {
         switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
         {
             case SRM_GSE_3D_SRF: /* GSE to GSE */
                  break;

             default:
#ifdef CONV_DEBUG
                  fprintf(f_ptr,
                          conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
             SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
             SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));
#endif

            status = SE_COORD_UNSUPPORTED;
         }
      }
      else
      {
#ifdef CONV_DEBUG
             fprintf(f_ptr,
             conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
             SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
             SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
#endif
          status = SE_COORD_UNSUPPORTED;
      }
      break;

      case GSM_GSM:
      if(dest_srf_ptr->is_2D == SE_FALSE)
      {
         switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
         {
            case SRM_GSM_3D_SRF: /* GSM to GSM */
            break;

            default:
#ifdef CONV_DEBUG
                fprintf(f_ptr,
                conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));
#endif
            status = SE_COORD_UNSUPPORTED;
         }
      }
      else /* 2d not supported */
      {
#ifdef CONV_DEBUG
                fprintf(f_ptr,
                conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
#endif
            status = SE_COORD_UNSUPPORTED;
      }
      break;

      case SM_SM:
      if(dest_srf_ptr->is_2D)
      {
         switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
         {
            case SRM_SM_3D_SRF: /* SM to SM */
            break;

            default:
#ifdef CONV_DEBUG
            fprintf(f_ptr,
            conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
            SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
            SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));
#endif
            status = SE_COORD_UNSUPPORTED;
         }
      }
      else
      {
#ifdef CONV_DEBUG
          fprintf(f_ptr,
          conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
          SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
          SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
#endif
         status = SE_COORD_UNSUPPORTED;
      }
      break;

      case GD_GD:
           break;

      case GC_GC:
           break;

      case GD_GC:
           output_ptr->constants[c_index]->dest_units = SRM_METERS;
           break;

      case GC_GD:
           set_gc_to_gd_constants
           (
             e_constants,
             dest_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.gc_gd
           );
           break;

      case GC_LTP:
           set_ltp_constants
           (
              e_constants,
              dest_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.ltp
           );
           break;

      case LTP_GC:
           set_ltp_constants
           (
             e_constants,
             src_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.ltp
           );
           break;

      case GD_EC:
           set_ec_constants
           (
              e_constants,
              dest_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.ec
           );
           break;

      case EC_GD:
           set_ec_constants
           (
              e_constants,
              src_srf_ptr,
              &output_ptr->constants[c_index]->specific_constants.ec
           );
           break;

      case GD_M:
           set_gd_to_m_constants
           (
              e_constants,
              dest_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.gd_m
           );
           break;

      case M_GD:
           set_m_to_gd_constants
           (
              e_constants,
              src_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.m_gd
           );
           break;

      case GD_PS:
           set_gd_to_ps_constants
           (
              e_constants,
              dest_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.gd_ps
           );
           break;

      case PS_GD:
           set_ps_to_gd_constants
           (
              e_constants,
              src_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.ps_gd
           );
           break;

      case GD_TM:
           set_gd_to_tm_constants
           (
              e_constants,
              dest_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.gd_tm
           );
           break;

      case TM_GD:
           set_tm_to_gd_constants
           (
             e_constants,
             src_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.tm_gd
           );
           break;

      case GD_UTM:
           set_gd_to_utm_constants
           (
              e_constants,
              dest_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.gd_utm
           );
           break;

      case UTM_GD:
           set_utm_to_gd_constants
           (
              e_constants,
              src_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.utm_gd
           );
           break;

      case GD_LCC1:
           set_gd_to_lcc1_constants
           (
              e_constants,
              dest_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.gd_lcc1
           );
           break;

      case LCC1_GD:
           set_lcc1_to_gd_constants
           (
              e_constants,
              src_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.lcc1_gd
           );
           break;

      case GD_LCC2:
           set_gd_to_lcc2_constants
           (
              e_constants,
              dest_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.gd_lcc2
           );
           break;

      case LCC2_GD:
           set_lcc2_to_gd_constants
           (
              e_constants,
              src_srf_ptr,
             &output_ptr->constants[c_index]->specific_constants.lcc2_gd
           );
           break;

      default:
           status = SE_COORD_FAILURE;
           break;
   } /* end switch statement */
   return status;
}


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_CreateCoordConversionConstants
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_CreateCoordConversionConstants
(
     const SE_SRF_PARAMETERS            *src_srf_ptr,
     const SE_SRF_PARAMETERS            *dest_srf_ptr,
           SE_CONVERT_COORD_SYSTEM_PAIR *convert_coord_system_ptr
)
{
   SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

   /*
    * A ptr to hold the output data temporarily
    */
    SE_CONVERT_COORD_SYSTEM_PARAMS *output_ptr = NULL;

   /*
    * If SE_TRUE, then the ellipse constants need to be computed.
    * If SE_FALSE, then do not compute the ellipse constants.
    */
    SE_BOOLEAN compute_e_constants;

   /*
    * A shortcut to the converted ellipse constants
    */
    SE_ERM_DATA *e_constants = NULL;

    SE_HORIZONTAL_DATUM_ENUM source_hdatum = SE_W84_HDATUM,
                             dest_hdatum = SE_W84_HDATUM;
    SE_VERTICAL_DATUM_ENUM source_vdatum = SE_WGS84E_VDATUM,
                           dest_vdatum = SE_WGS84E_VDATUM;

    SE_BOOLEAN gc_is_source_or_dest = SE_FALSE;
    SE_CONVERT_TYPE_ENUM   height_convert_type = NO_CONVERSION;

    int i; /* $$$ int??? */

   /*
    * Only used if a "chaining" or "transitive" conversion is required
    */
    SE_SRF_PARAMETERS temp_srf;

#ifdef CONV_DEBUG
   /*
    * if debugging is enabled, set the stderr pointer
    */
    FILE                           *f_ptr;

    f_ptr = SE_GetErrorFilePtr();
#endif

   /*
    * validate pointer and parameters
    */
    if (convert_coord_system_ptr == NULL)
    {
        status = SE_COORD_INVALID_SRF_PAIR;
    }
    else if (!src_srf_ptr ||
             !SE_ValidSRFParameters(src_srf_ptr,SE_TRUE, NULL))
    {
        status = SE_COORD_INVALID_SOURCE_SRF_PARAMETERS;
    }
    else if (!dest_srf_ptr ||
             !SE_ValidSRFParameters(dest_srf_ptr,SE_TRUE, NULL))
    {
        status = SE_COORD_INVALID_DEST_SRF_PARAMETERS;
    } /* end if */

   /*
    * if anything is wrong, we return now
    */
    if (status != SE_COORD_SUCCESS)
    {
       if (convert_coord_system_ptr)
           *convert_coord_system_ptr = NULL;
       return status;
    }


   /*
    ***************************************************************************
    * Check source parameters.
    ***************************************************************************
    */
    compute_e_constants = SE_FALSE;

   /*
    * get the source horizontal datum
    */
    if (src_srf_ptr->is_2D == SE_TRUE)
    {
        switch (src_srf_ptr->u.params_2d.spatial_reference_frame)
        {
            case SRM_GD_2D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_2d.u.gd_parameters.
                                 horizontal_datum;
                 break;

            case SRM_LSR_2D_SRF:
                 /*
                  * Only LSR to LSR is supported.
                  */
                 if (dest_srf_ptr->is_2D &&
                     (dest_srf_ptr->u.params_2d.spatial_reference_frame
                      != SRM_LSR_2D_SRF))
                 {
                     status = SE_COORD_UNSUPPORTED;
                 }
                 else if (!dest_srf_ptr->is_2D ||
                     (dest_srf_ptr->u.params_3d.spatial_reference_frame
                      != SRM_LSR_3D_SRF))
                 {
                     status = SE_COORD_UNSUPPORTED;
                 }
                 break;

            case SRM_OM_2D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_2d.u.om_parameters.
                                 horizontal_datum;
                 break;

            case SRM_M_2D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_2d.u.m_parameters.
                                 horizontal_datum;
                 break;

            case SRM_EC_2D_SRF:
                 source_hdatum = src_srf_ptr->u.params_2d.u.ec_parameters.
                                 horizontal_datum;
                 compute_e_constants = SE_TRUE;
                 break;

            case SRM_UPS_2D_SRF:
                 /* only UPS to UPS is supported */
                 if (dest_srf_ptr->is_2D &&
                     (dest_srf_ptr->u.params_2d.spatial_reference_frame !=
                      SRM_UPS_2D_SRF))
                 {
                     status = SE_COORD_UNSUPPORTED;
                 }
                 else if (!dest_srf_ptr->is_2D &&
                          (dest_srf_ptr->u.params_3d.spatial_reference_frame
                           != SRM_UPS_3D_SRF))
                 {
                     status = SE_COORD_UNSUPPORTED;
                 }
                 break;

            case SRM_PS_2D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_2d.u.ps_parameters.
                                 horizontal_datum;
                 break;

            case SRM_LCC_2D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_2d.u.lcc_parameters.
                                 horizontal_datum;
                 break;

            case SRM_TM_2D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_2d.u.tm_parameters.
                                 horizontal_datum;
                 break;

            case SRM_UTM_2D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_2d.u.utm_parameters.
                                 horizontal_datum;
                 break;

            case SRM_LTP_2D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_2d.u.ltp_parameters.
                                 horizontal_datum;
                 break;

            default:
                 status = SE_COORD_FAILURE;
                 break;
        } /* end switch on source parameters */
    } /* end 2d */
    else /* is 3d */
    {
       /* get the source horizontal datum */
        switch (src_srf_ptr->u.params_3d.spatial_reference_frame)
        {
            case SRM_GD_3D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_3d.u.gd_parameters.
                                 horizontal_datum;
                 source_vdatum = src_srf_ptr->u.params_3d.u.gd_parameters.
                                 vertical_datum;
                 break;

            case SRM_GC_3D_SRF:
                 compute_e_constants = SE_TRUE;
                 gc_is_source_or_dest = SE_TRUE;
                 break;

            case SRM_GM_3D_SRF:
                 break;

            case SRM_GEI_3D_SRF:
                 break;

            case SRM_GSE_3D_SRF:
                 break;

            case SRM_GSM_3D_SRF:
                 break;

            case SRM_SM_3D_SRF:
                 break;

            case SRM_GCS_3D_SRF:
                 break;

            case SRM_LSR_3D_SRF:
                 if (dest_srf_ptr->is_2D &&
                     (dest_srf_ptr->u.params_2d.spatial_reference_frame
                      != SRM_LSR_2D_SRF))
                 {
                     status = SE_COORD_UNSUPPORTED;
                 }
                 else if (!dest_srf_ptr->is_2D &&
                          (dest_srf_ptr->u.params_3d.spatial_reference_frame
                           != SRM_LSR_3D_SRF))
                 {
                     status = SE_COORD_UNSUPPORTED;
                 }
                 break;

            case SRM_OM_3D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_3d.u.om_parameters.
                                 horizontal_datum;
                 source_vdatum = src_srf_ptr->u.params_3d.u.om_parameters.
                                 vertical_datum;
                 break;

            case SRM_M_3D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_3d.u.m_parameters.
                                 horizontal_datum;
                 source_vdatum = src_srf_ptr->u.params_3d.u.m_parameters.
                                 vertical_datum;
                 break;

            case SRM_EC_3D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_3d.u.ec_parameters.
                                 horizontal_datum;
                 source_vdatum = src_srf_ptr->u.params_3d.u.ec_parameters.
                                 vertical_datum;
                 break;

            case SRM_UPS_3D_SRF:
                /* only UPS to UPS is supported */
                 if (dest_srf_ptr->is_2D &&
                     (dest_srf_ptr->u.params_2d.spatial_reference_frame
                      != SRM_UPS_2D_SRF))
                 {
                     status = SE_COORD_UNSUPPORTED;
                 }
                 else if (!dest_srf_ptr->is_2D &&
                          (dest_srf_ptr->u.params_3d.spatial_reference_frame
                           != SRM_UPS_3D_SRF))
                 {
                     status = SE_COORD_UNSUPPORTED;
                 }
                 break;

            case SRM_PS_3D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_3d.u.ps_parameters.
                                 horizontal_datum;
                 source_vdatum = src_srf_ptr->u.params_3d.u.ps_parameters.
                                 vertical_datum;
                 break;

            case SRM_LCC_3D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_3d.u.lcc_parameters.
                                 horizontal_datum;
                 source_vdatum = src_srf_ptr->u.params_3d.u.lcc_parameters.
                                 vertical_datum;
                 break;

            case SRM_TM_3D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_3d.u.tm_parameters.
                                 horizontal_datum;
                 source_vdatum = src_srf_ptr->u.params_3d.u.tm_parameters.
                                 vertical_datum;
                 break;

            case SRM_UTM_3D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_3d.u.utm_parameters.
                                 horizontal_datum;
                 source_vdatum = src_srf_ptr->u.params_3d.u.utm_parameters.
                                 vertical_datum;
                 break;

            case SRM_LTP_3D_SRF:
                 compute_e_constants = SE_TRUE;
                 source_hdatum = src_srf_ptr->u.params_3d.u.ltp_parameters.
                                 horizontal_datum;
                 source_vdatum = src_srf_ptr->u.params_3d.u.ltp_parameters.
                                 vertical_datum;
                 break;

            default:
                 status = SE_COORD_FAILURE;
                 break;
        } /* end switch on source parameters */
    } /* end else 3d */

   /*
    ***************************************************************************
    * get destination datums.
    ***************************************************************************
    */

    if (dest_srf_ptr->is_2D == SE_TRUE)
    {
      switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
      {
         case SRM_GD_2D_SRF:
              compute_e_constants = SE_TRUE;
              dest_hdatum = dest_srf_ptr->u.params_2d.u.gd_parameters.
                            horizontal_datum;
              break;

         case SRM_LSR_2D_SRF:
         /* only LSR to LSR is supported */
         if (src_srf_ptr->is_2D &&
             (src_srf_ptr->u.params_2d.spatial_reference_frame!=SRM_LSR_2D_SRF))
         {
             status = SE_COORD_UNSUPPORTED;
         }
         else if (!src_srf_ptr->is_2D &&
                  (src_srf_ptr->u.params_3d.spatial_reference_frame
                      !=SRM_LSR_3D_SRF))
         {
             status = SE_COORD_UNSUPPORTED;
         }
         break;

         case SRM_OM_2D_SRF:
              compute_e_constants = SE_TRUE;
              dest_hdatum = dest_srf_ptr->u.params_2d.u.om_parameters.
                            horizontal_datum;
              break;

         case SRM_M_2D_SRF:
              compute_e_constants = SE_TRUE;
              dest_hdatum = dest_srf_ptr->u.params_2d.u.m_parameters.
                            horizontal_datum;
              break;

         case SRM_EC_2D_SRF:
              dest_hdatum = dest_srf_ptr->u.params_2d.u.ec_parameters.
                            horizontal_datum;
              compute_e_constants = SE_TRUE;
              break;

         case SRM_UPS_2D_SRF:
             /* only UPS to UPS is supported */
              if (src_srf_ptr->is_2D &&
                  (src_srf_ptr->u.params_2d.spatial_reference_frame
                   != SRM_UPS_2D_SRF))
              {
                  status = SE_COORD_UNSUPPORTED;
              }
              else if (!src_srf_ptr->is_2D &&
                       (src_srf_ptr->u.params_3d.spatial_reference_frame !=
                        SRM_UPS_3D_SRF))
              {
                  status = SE_COORD_UNSUPPORTED;
              }
              break;

         case SRM_PS_2D_SRF:
              compute_e_constants = SE_TRUE;
              dest_hdatum = dest_srf_ptr->u.params_2d.u.ps_parameters.
                            horizontal_datum;
              break;

         case SRM_LCC_2D_SRF:
              compute_e_constants = SE_TRUE;
              dest_hdatum = dest_srf_ptr->u.params_2d.u.lcc_parameters.
                            horizontal_datum;
              break;

         case SRM_TM_2D_SRF:
              compute_e_constants = SE_TRUE;
              dest_hdatum = dest_srf_ptr->u.params_2d.u.tm_parameters.
                            horizontal_datum;
              break;

         case SRM_UTM_2D_SRF:
              compute_e_constants = SE_TRUE;
              dest_hdatum = dest_srf_ptr->u.params_2d.u.utm_parameters.
                            horizontal_datum;
              break;

         case SRM_LTP_2D_SRF:
              compute_e_constants = SE_TRUE;
              dest_hdatum = dest_srf_ptr->u.params_2d.u.ltp_parameters.
                            horizontal_datum;
              break;

         default:
              status = SE_COORD_FAILURE;
              break;
       } /* end switch on dest parameters */
   }
   else /* its 3d */
   {
      switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
      {
         case SRM_GD_3D_SRF:

         compute_e_constants = SE_TRUE;
         dest_hdatum = dest_srf_ptr->u.params_3d.u.gd_parameters.horizontal_datum;
         dest_vdatum = dest_srf_ptr->u.params_3d.u.gd_parameters.vertical_datum;
         break;

         case SRM_GC_3D_SRF:
         compute_e_constants = SE_TRUE;
         gc_is_source_or_dest = SE_TRUE;
         break;

         case SRM_GM_3D_SRF:
         break;

         case SRM_GEI_3D_SRF:
         break;

         case SRM_GSE_3D_SRF:
         break;

         case SRM_GSM_3D_SRF:
         break;

         case SRM_SM_3D_SRF:
         break;

         case SRM_GCS_3D_SRF:
         gc_is_source_or_dest = SE_TRUE;
         break;

         case SRM_LSR_3D_SRF:
         /* only lsr to lsr is supported */

         if(src_srf_ptr->is_2D == SE_TRUE)
         {
            if(src_srf_ptr->u.params_2d.spatial_reference_frame != SRM_LSR_2D_SRF)
               status = SE_COORD_UNSUPPORTED;
         }
         else
            if(src_srf_ptr->u.params_3d.spatial_reference_frame != SRM_LSR_3D_SRF)
               status = SE_COORD_UNSUPPORTED;
         break;

         case SRM_OM_3D_SRF:
         compute_e_constants = SE_TRUE;
         dest_hdatum = dest_srf_ptr->u.params_3d.u.om_parameters.horizontal_datum;
         dest_vdatum = dest_srf_ptr->u.params_3d.u.om_parameters.vertical_datum;
         break;

         case SRM_M_3D_SRF:
         compute_e_constants = SE_TRUE;
         dest_hdatum = dest_srf_ptr->u.params_3d.u.m_parameters.horizontal_datum;
         dest_vdatum = dest_srf_ptr->u.params_3d.u.m_parameters.vertical_datum;
         break;

         case SRM_EC_3D_SRF:

         dest_hdatum = dest_srf_ptr->u.params_3d.u.ec_parameters.horizontal_datum;
         dest_vdatum = dest_srf_ptr->u.params_3d.u.ec_parameters.vertical_datum;

         compute_e_constants = SE_TRUE;

         /* EC only supports spherical earth models */
         break;

         case SRM_UPS_3D_SRF:
         /* only UPS to UPS is supported */

         if(src_srf_ptr->is_2D == SE_TRUE)
         {
            if(src_srf_ptr->u.params_2d.spatial_reference_frame != SRM_UPS_2D_SRF)
               status = SE_COORD_UNSUPPORTED;
         }
         else
            if(src_srf_ptr->u.params_3d.spatial_reference_frame != SRM_UPS_3D_SRF)
               status = SE_COORD_UNSUPPORTED;
         break;

         case SRM_PS_3D_SRF:
         compute_e_constants = SE_TRUE;
         dest_hdatum = dest_srf_ptr->u.params_3d.u.ps_parameters.horizontal_datum;
         dest_vdatum = dest_srf_ptr->u.params_3d.u.ps_parameters.vertical_datum;
         break;

         case SRM_LCC_3D_SRF:
         compute_e_constants = SE_TRUE;
         dest_hdatum = dest_srf_ptr->u.params_3d.u.lcc_parameters.horizontal_datum;
         dest_vdatum = dest_srf_ptr->u.params_3d.u.lcc_parameters.vertical_datum;
         break;

         case SRM_TM_3D_SRF:
         compute_e_constants = SE_TRUE;
         dest_hdatum = dest_srf_ptr->u.params_3d.u.tm_parameters.horizontal_datum;
         dest_vdatum = dest_srf_ptr->u.params_3d.u.tm_parameters.vertical_datum;
         break;

         case SRM_UTM_3D_SRF:
         compute_e_constants = SE_TRUE;
         dest_hdatum = dest_srf_ptr->u.params_3d.u.utm_parameters.horizontal_datum;
         dest_vdatum = dest_srf_ptr->u.params_3d.u.utm_parameters.vertical_datum;
         break;

         case SRM_LTP_3D_SRF:
         compute_e_constants = SE_TRUE;
         dest_hdatum = dest_srf_ptr->u.params_3d.u.ltp_parameters.horizontal_datum;
         dest_vdatum = dest_srf_ptr->u.params_3d.u.ltp_parameters.vertical_datum;
         break;

         default:
         status = SE_COORD_FAILURE;
         break;
       } /* end switch on dest parameters */
   } /* end else 3d */

   /*
    * GC has no parameters, so we get them from the other SRF involved
    */
   if (gc_is_source_or_dest == SE_TRUE)
   {
      /*
       * can only check for SRM_GC_3D_SRF in a 3D SRF
       */
       if (src_srf_ptr->is_2D != SE_TRUE)
       {
          /*
           * If the source is GC, then use the datums from the other SRF.
           * Otherwise, the destination is GC, so set the destination datums
           * to equal those of the source SRF.
           */
           if (src_srf_ptr->u.params_3d.spatial_reference_frame==SRM_GC_3D_SRF)
           {
               source_hdatum = dest_hdatum;
               source_vdatum = dest_vdatum;
           }
           else
           {
               dest_hdatum = source_hdatum;
               dest_vdatum = source_vdatum;
           }
       }
       else /* if the source is 2D then in cannot be GC, so use the datum's
             * from the dest SRF
             */
       {
           dest_hdatum = source_hdatum;
           dest_vdatum = source_vdatum;
       }
   }

   /* check that source and dest horizontal datums are the same
    * horizontal datums are not implemented yet
    */

   if(source_hdatum != dest_hdatum)
   {
#ifdef CONV_DEBUG
      fprintf(f_ptr,"[SEDRIS CONVERSIONS] Attempted conversion with "\
              "different source and destination horizontal datums.\n"\
              "Horizontal datum shifts are currently unsupported.\n");
#endif
      status = SE_COORD_UNSUPPORTED;
   }

   /* if the parameters are valid then create the data structure */

   if(status == SE_COORD_SUCCESS)
   {
     output_ptr = calloc (1, sizeof(SE_CONVERT_COORD_SYSTEM_PARAMS));
     if (!output_ptr)
         status = SE_COORD_FAILURE;

   }

   /* if we have errors, quit now */
   if (status != SE_COORD_SUCCESS)
   {
      *convert_coord_system_ptr = NULL;
      return status;
   }

   /* Initialize the constants array member to be NULL */
   /* SRM_CONSTANTS_DEPTH is a #define             */

   for(i=0;i < SRM_CONSTANTS_DEPTH ;i++)
      output_ptr->constants[i] = NULL;

  /*
   * set up height conversion type, only relevant if both srfs are 3D
   */
   if (src_srf_ptr->is_2D == SE_FALSE && dest_srf_ptr->is_2D == SE_FALSE)
   {
      switch(source_vdatum)
      {
         /* spherical vdatums must match, or it is unsupported */
         case SE_SPHERICAL_ACM_VDATUM:
         case SE_SPHERICAL_COA_VDATUM:
         case SE_SPHERICAL_NOG_VDATUM:
         case SE_SPHERICAL_MFE_VDATUM:
         case SE_SPHERICAL_MOD_M_VDATUM:
         case SE_SPHERICAL_MOD_S_VDATUM:
         case SE_SPHERICAL_MOD_T_VDATUM:
         case SE_SPHERICAL_MM5_VDATUM:
         if(source_vdatum!=dest_vdatum)
         {
#ifdef CONV_DEBUG
            fprintf(f_ptr,"[SEDRIS CONVERSIONS] Both "\
                    "spatial reference frames must have the same "\
                    "spherical vertical datum specified");
#endif
            free(output_ptr);
            return SE_COORD_UNSUPPORTED;
         }
         break;

         case SE_EGM96_VDATUM:
         case SE_NAVD88_VDATUM:
         case SE_NGVD29_VDATUM:
#ifdef CONV_DEBUG
         fprintf(f_ptr,"[SEDRIS CONVERSIONS] Conversions involving %s "\
                 "are not currently supported",
                 SE_PrintVerticalDatum(source_vdatum));
#endif
         free(output_ptr);
         return SE_COORD_UNSUPPORTED;


         case SE_WGS84E_VDATUM: /* source is ellipsoid */
         switch(dest_vdatum)
         {
            case SE_MSL_VDATUM:
            case SE_WGS84G_VDATUM:
            /* ellipsoid to geoid */
            height_convert_type = E_TO_G;
            break;

            case SE_EGM96_VDATUM:
            case SE_NAVD88_VDATUM:
            case SE_NGVD29_VDATUM:
#ifdef CONV_DEBUG
            fprintf(f_ptr,"[SEDRIS CONVERSIONS] Conversions involving %s "\
                    "are not currently supported",
                    SE_PrintVerticalDatum(dest_vdatum));
#endif
            free(output_ptr);
            return SE_COORD_UNSUPPORTED;


            /* spherical vdatums must match, or it is unsupported */
            case SE_SPHERICAL_ACM_VDATUM:
            case SE_SPHERICAL_COA_VDATUM:
            case SE_SPHERICAL_NOG_VDATUM:
            case SE_SPHERICAL_MFE_VDATUM:
            case SE_SPHERICAL_MOD_M_VDATUM:
            case SE_SPHERICAL_MOD_S_VDATUM:
            case SE_SPHERICAL_MOD_T_VDATUM:
            case SE_SPHERICAL_MM5_VDATUM:

#ifdef CONV_DEBUG
            fprintf(f_ptr,"[SEDRIS CONVERSIONS] Both "\
                    "spatial reference frames must have the "\
                    "same spherical vertical datum specified");
#endif
            free(output_ptr);
            return SE_COORD_UNSUPPORTED;

            default:
                 height_convert_type = NO_CONVERSION;
                 break;
         }
         break;

         case SE_MSL_VDATUM:     /* source is geoid */
         case SE_WGS84G_VDATUM:
         switch(dest_vdatum)
         {
            case SE_WGS84E_VDATUM:
            /* geoid to ellipsoid */
            height_convert_type = G_TO_E;
            break;

            case SE_EGM96_VDATUM:
            case SE_NAVD88_VDATUM:
            case SE_NGVD29_VDATUM:
#ifdef CONV_DEBUG
            fprintf(f_ptr,"[SEDRIS CONVERSIONS] Conversions involving %s"\
                    " are not currently supported",
                    SE_PrintVerticalDatum(dest_vdatum));
#endif
            free(output_ptr);
            return SE_COORD_UNSUPPORTED;


            /* spherical vdatums must match, or it is unsupported */
            case SE_SPHERICAL_ACM_VDATUM:
            case SE_SPHERICAL_COA_VDATUM:
            case SE_SPHERICAL_NOG_VDATUM:
            case SE_SPHERICAL_MFE_VDATUM:
            case SE_SPHERICAL_MOD_M_VDATUM:
            case SE_SPHERICAL_MOD_S_VDATUM:
            case SE_SPHERICAL_MOD_T_VDATUM:
            case SE_SPHERICAL_MM5_VDATUM:
#ifdef CONV_DEBUG
            fprintf(f_ptr,"[SEDRIS CONVERSIONS] Both "\
                    "spatial reference frames must have the same "\
                    "spherical vertical datum specified");
#endif
            free(output_ptr);
            return SE_COORD_UNSUPPORTED;

            default:
                 height_convert_type = NO_CONVERSION;
                 break;
         }
         break;

         default:
              height_convert_type = NO_CONVERSION;
              break;
      } /* end switch on source vdatum */
   }

   if(compute_e_constants == SE_TRUE)
   {
      e_constants=&output_ptr->ellipse_constants;

     /*
      * Get A and F from the Datum Table.  If F=0 then we are doing a
      * spherical transformation
      */

      e_constants->A = Datum_Table[source_hdatum].A;
      e_constants->F = Datum_Table[source_hdatum].F;

     /* Create the ERM constants */

     /*
      * if F equals 0 then this is a spherical transformation,
      * if so don't divide by zero
      */

     if (e_constants->F!=0)
        e_constants->F  = 1/(e_constants->F);
     else
        e_constants->B = e_constants->A;

      e_constants->C      =(e_constants->A) * (1-e_constants->F);
      e_constants->Eps2   =(e_constants->F) * (2.0-e_constants->F);
      e_constants->Eps25  =.25 * (e_constants->Eps2);
      e_constants->Epps2  =(e_constants->Eps2) / (1.0 - e_constants->Eps2);
      e_constants->Eps    =sqrt(e_constants->Eps2);
      e_constants->EpsH   =(e_constants->Eps) / 2.0;
      e_constants->A2     =(e_constants->A) * (e_constants->A);
      e_constants->C2     = e_constants->C * e_constants->C;

   } /* end if compute_e_constants */


   /*
    * We have filled in the global ERM data. Now we need to fill in the
    * specific data constants.
    */

   /*
    * we need to very careful here with our pointer references
    * to check for 2D or 3D problems
    */

   /*
    * This big block is for when the source SRF is 3D
    */
   if(src_srf_ptr->is_2D == SE_FALSE)
   {
      /* Source SRF is 3D               */
      switch (src_srf_ptr->u.params_3d.spatial_reference_frame)
      {
         /* LSR to LSR, axes must be the same */
         /* The above checking ensures that this is only LSR to LSR */
         case SRM_LSR_3D_SRF:
              status = CreateConstants
                       (
                           output_ptr,
                           0,
                           LSR_LSR,
                           height_convert_type,
                           src_srf_ptr,
                           dest_srf_ptr,
                           e_constants
                       );
              break;

         case SRM_GCS_3D_SRF: /* Case is GCS to GCS */
              status = CreateConstants
                       (
                           output_ptr,
                           0,
                           GCS_GCS,
                           height_convert_type,
                           src_srf_ptr,
                           dest_srf_ptr,
                           e_constants
                       );
              break; /* end GCS to X */

         case SRM_OM_3D_SRF:  /* Case is OM_3D to OM */
         status = CreateConstants(output_ptr,0,OM_OM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
         break;

         case SRM_GEI_3D_SRF: /* Case is GEI to GEI */
         status = CreateConstants(output_ptr,0,GEI_GEI,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
         break; /* end GEI to X */

         case SRM_GM_3D_SRF: /* Case is GM to GM */
         status = CreateConstants(output_ptr,0,GM_GM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
         break;

         case SRM_GSE_3D_SRF: /* Case is GSE to X */
         status = CreateConstants(output_ptr,0,GSM_GSM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
         break;

         case SRM_GSM_3D_SRF: /* Case is GSM to GSM */
         status = CreateConstants(output_ptr,0,GSM_GSM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
         break;

         case SRM_UPS_3D_SRF: /* Case is UPS to UPS only */
         status = CreateConstants(output_ptr,0,UPS_UPS,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
         break;

         case SRM_SM_3D_SRF: /* Case is SM to X */
         status = CreateConstants(output_ptr,0,SM_SM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
         break;

         case SRM_LCC_3D_SRF:   /* Case is LCC 3D to X(2D or 3D) Constants */

         /*
          * If the two parallels are equal, then we are doing LCC with one parallel (LCC1)
          */

         if (src_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
            src_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
         {
            /* source is 3D, check dest */

            if(dest_srf_ptr->is_2D == SE_FALSE) /* dest is 3D */
            {
               /* these cases are all LCC1 3d to 3d */
               switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
               {
                  case SRM_LCC_3D_SRF: /* LCC1 to GD to LCC1 */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_GD_3D_SRF: /* LCC1 to GD */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_LTP_3D_SRF: /* LCC1 to LTP */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                  {
                     output_ptr->constants[0]->vector_convert_type = LCC_TO_LTP;
                     status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break; /* end lcc1 to ltp */

                  case SRM_EC_3D_SRF: /* LCC1 to EC */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_GC_3D_SRF: /* LCC1 to GC */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_M_3D_SRF: /* LCC1 to M */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_PS_3D_SRF: /* LCC1 to PS */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_TM_3D_SRF: /* LCC1 to TM */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_UTM_3D_SRF: /* LCC1 to UTM */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  default:

                  #ifdef CONV_DEBUG

                  fprintf(f_ptr,
                  conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                  SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                  SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

                  #endif

                  status = SE_COORD_UNSUPPORTED;
               } /* end switch lcc1 to X 3d to 3D    */
            } /* end dest is 3D */
            else /* LCC1 to X dest SRF is 2D    */
            {
               switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
               {
                  case SRM_LCC_2D_SRF: /* LCC1 to GD to LCC1 */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_GD_2D_SRF: /* LCC1 to GD */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_LTP_2D_SRF: /* LCC1 to LTP */
                  output_ptr->constants[0]->vector_convert_type = LCC_TO_LTP;

                  if(status == SE_COORD_SUCCESS)
                  {
                     status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                     status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break; /* end lcc1 to ltp */

                  case SRM_EC_2D_SRF: /* LCC1 to EC */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_M_2D_SRF: /* LCC1 to M */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_PS_2D_SRF: /* LCC1 to PS */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_TM_2D_SRF: /* LCC1 to TM */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_UTM_2D_SRF: /* LCC1 to UTM */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  default:

                  #ifdef CONV_DEBUG

                  fprintf(f_ptr,
                  conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                  SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                  SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

                  #endif

                  status = SE_COORD_UNSUPPORTED;
               } /* end switch lcc1 to X source SRF 3D, dest SRF 2D    */
            } /* end lcc 1, source is 3D */
         }
         else /* LCC2, parallels are not equal */
         {
            if(dest_srf_ptr->is_2D == SE_FALSE) /* 3D LCC2 to 3D LCC2 */
            {
               switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
               {
                  case SRM_LCC_3D_SRF: /* LCC2 to LCC2 */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_GD_3D_SRF: /* LCC2 to GD */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_LTP_3D_SRF: /* LCC2 to LTP */

                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                  {
                     output_ptr->constants[0]->vector_convert_type = LCC_TO_LTP;
                     status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break; /* end lcc1 to ltp */

                  case SRM_EC_3D_SRF: /* LCC2 to EC */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_GC_3D_SRF: /* LCC2 to GC */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_M_3D_SRF: /* LCC2 to M */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_PS_3D_SRF: /* LCC2 to PS */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_TM_3D_SRF: /* LCC2 to TM */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_UTM_3D_SRF: /* LCC2 to UTM */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  default:
#ifdef CONV_DEBUG
                  fprintf(f_ptr,
                  conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                  SRM_PrintSRF3D(src_srf_ptr->u.params_3d.
                                spatial_reference_frame),
                  SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.
                                spatial_reference_frame));
#endif
                  status = SE_COORD_UNSUPPORTED;

               } /* end switch lcc2 to x 3D*/
            } /* end lcc2 to x 3D */
            else /* lcc2 to x 2d */
            {
                switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
                {
                    case SRM_LCC_2D_SRF: /* LCC2 to LCC2 */
                         status = CreateConstants
                                  (output_ptr,0,LCC2_GD,height_convert_type,
                                   src_srf_ptr,dest_srf_ptr,e_constants);
                         if(status == SE_COORD_SUCCESS)
                             status = CreateConstants
                                      (
                                          output_ptr,1,GD_LCC2,
                                          NO_CONVERSION,
                                          src_srf_ptr,
                                          dest_srf_ptr,
                                          e_constants
                                      );
                         break;

                    case SRM_GD_2D_SRF: /* LCC2 to GD */
                         status = CreateConstants
                                  (output_ptr,0,LCC2_GD,height_convert_type,
                                   src_srf_ptr,dest_srf_ptr,e_constants);
                         break;

                    case SRM_LTP_2D_SRF: /* LCC2 to LTP */
                         status = CreateConstants
                                  (
                                      output_ptr,0,LCC2_GD,
                                      height_convert_type,
                                      src_srf_ptr,dest_srf_ptr,e_constants
                                  );
                         if (status == SE_COORD_SUCCESS)
                         {
                            output_ptr->constants[0]->vector_convert_type =
                                        LCC_TO_LTP;
                            status = CreateConstants
                                     (
                                         output_ptr,1,GD_GC,NO_CONVERSION,
                                         src_srf_ptr,dest_srf_ptr,e_constants
                                     );
                         }
                         if(status == SE_COORD_SUCCESS)
                            status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break; /* end lcc1 to ltp */

                  case SRM_EC_2D_SRF: /* LCC2 to EC */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_M_2D_SRF: /* LCC2 to M */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_PS_2D_SRF: /* LCC2 to PS */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_TM_2D_SRF: /* LCC2 to TM */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_UTM_2D_SRF: /* LCC2 to UTM */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  default:
                  #ifdef CONV_DEBUG
                  fprintf(f_ptr,
                  conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                  SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                  SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
                  #endif
                  status = SE_COORD_UNSUPPORTED;

               } /* end switch lcc2 to x 2d*/
            } /* end lcc2 to x 2d */
         } /* end lcc2 */
         break; /* End of LCC to X Constants */

         case SRM_M_3D_SRF:  /* Case is M_3D to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* dest is 3D */
         {
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_M_3D_SRF: /* M to GD to M */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_3D_SRF: /* M to GD */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_3D_SRF: /* M to LTP */

               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = M_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* M to EC */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* M to GC */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* M to LCC */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1, we know the dest is 3d */
                  if(dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_PS_3D_SRF: /* M_3D to PS_3D */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* M to TM */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* M to UTM */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:
               #ifdef CONV_DEBUG
                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));
               #endif
               status = SE_COORD_UNSUPPORTED;
            } /* switch end M to X */
         } /* end M_3D to 3D */
         else /* M_3D to 2D  */
         {
            switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_M_2D_SRF: /* M to GD to M */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_2D_SRF: /* M to GD */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_2D_SRF: /* M to LTP */

               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = M_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* M to EC */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* M to LCC */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC2, we know the dest is 2d */
                  if(dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_PS_2D_SRF: /* M_3D to PS_3D */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_2D_SRF: /* M to TM */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_2D_SRF: /* M to UTM */
                status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:
               #ifdef CONV_DEBUG
                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
               #endif
               status = SE_COORD_UNSUPPORTED;
            } /* switch end M to X */
         }
         break;

         case SRM_EC_3D_SRF:  /* Case is EC to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* EC_3d TO X 3D */
         {
            switch(dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_GD_3D_SRF: /* EC to GD */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* EC to GC */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* EC to GD to EC */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* EC_3D to LCC_3D */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1, we know the dest is 3d */
                  if(dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_LTP_3D_SRF: /* EC to LTP */

               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = EC_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_M_3D_SRF: /* EC to M */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_3D_SRF: /* EC to PS */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* EC to TM */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* EC to UTM */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;

            } /* end switch EC to X_3D */
         }
         else /* EC_3D to X_2D */
         {
            switch(dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_GD_2D_SRF: /* EC to GD */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* EC to GD to EC */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* EC_2D to LCC_2D */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1, we know the dest is 2d */
                  if(dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_LTP_2D_SRF: /* EC to LTP */

               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = EC_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_M_2D_SRF: /* EC to M */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_2D_SRF: /* EC to PS */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_2D_SRF: /* EC to TM */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_2D_SRF: /* EC to UTM */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch EC to X_3D */
         }
         break;

         case SRM_PS_3D_SRF:  /* Case is PS to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* PS_3D to X_3D */
         {
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_PS_3D_SRF: /* PS to GD to PS_3D */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_3D_SRF: /* PS to GD */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_3D_SRF: /* PS to LTP */

               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = PS_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* PS to EC */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* PS to GC */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* PS to LCC */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1, we know the dest is 3d */
                  if(dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_M_3D_SRF: /* PS to M */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* PS to TM */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* PS to UTM */
                status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch PS to X */
         }
         else /* PS_3D to X_2D */
         {
           switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_PS_2D_SRF: /* PS to GD to PS_3D */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_2D_SRF: /* PS to GD */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_2D_SRF: /* PS to LTP */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = PS_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* PS to EC */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* PS to LCC */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1, we know the dest is 2d */
                  if(dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_M_2D_SRF: /* PS to M */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_2D_SRF: /* PS to TM */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_2D_SRF: /* PS to UTM */
                status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch PS to X_2D */
         } /* end else */
         break;

         case SRM_GD_3D_SRF: /* Case is GD to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* GD_3D to X_3D */
         {
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_GD_3D_SRF: /* GD to GD */
               status = CreateConstants(output_ptr,0,GD_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_3D_SRF: /* GD to LTP */
               status = CreateConstants(output_ptr,0,GD_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = GD_TO_LTP;
                  status = CreateConstants(output_ptr,1,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               break;

               case SRM_EC_3D_SRF: /* GD to EC */
               status = CreateConstants(output_ptr,0,GD_EC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* GD to GC */
               status = CreateConstants(output_ptr,0,GD_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* GD to LCC */
               /* LCC1 */
               if (dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                   dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
               {
                  status = CreateConstants(output_ptr,0,GD_LCC1,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               else /* LCC2 */
               {
                  status = CreateConstants(output_ptr,0,GD_LCC2,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               break;

               case SRM_PS_3D_SRF: /* GD to PS */
               status = CreateConstants(output_ptr,0,GD_PS,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_M_3D_SRF: /* GD to M */
               status = CreateConstants(output_ptr,0,GD_M,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* GD to TM */
               status = CreateConstants(output_ptr,0,GD_TM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* GD to UTM */
               status = CreateConstants(output_ptr,0,GD_UTM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch gd to x_3D */
         }
         else /* gd_3d to x_2d */
         {
            switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_GD_2D_SRF: /* GD to GD */
               status = CreateConstants(output_ptr,0,GD_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_2D_SRF: /* GD to LTP */
               status = CreateConstants(output_ptr,0,GD_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = GD_TO_LTP;
                  status = CreateConstants(output_ptr,1,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               break;

               case SRM_EC_2D_SRF: /* GD to EC */
               status = CreateConstants(output_ptr,0,GD_EC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* GD to LCC */
               /* LCC1 */
               if (dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                   dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
               {
                  status = CreateConstants(output_ptr,0,GD_LCC1,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               else /* LCC2 */
               {
                  status = CreateConstants(output_ptr,0,GD_LCC2,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               break;

               case SRM_PS_2D_SRF: /* GD to PS */
               status = CreateConstants(output_ptr,0,GD_PS,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_M_2D_SRF: /* GD to M */
               status = CreateConstants(output_ptr,0,GD_M,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_2D_SRF: /* GD to TM */
               status = CreateConstants(output_ptr,0,GD_TM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_2D_SRF: /* GD to UTM */
               status = CreateConstants(output_ptr,0,GD_UTM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:
#ifdef CONV_DEBUG
                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
#endif
               status = SE_COORD_UNSUPPORTED;
            } /* end switch gd to x_2D */
         }
         break;  /* End GD to X Constants */

         case SRM_GC_3D_SRF: /* Case is GC to X Constants */
         /*
          * GC has no parameters on its own, so the horizontal and
          * vertical datums are pulled from the other SRF in the pair
          */
         if (dest_srf_ptr->is_2D == SE_FALSE) /* GC to X_3D   */
         {
            temp_srf.is_2D = SE_FALSE;
            temp_srf.u.params_3d.spatial_reference_frame = SRM_GD_3D_SRF;

            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_GC_3D_SRF: /* GC to GC */
               /* just copies coordinates, as it is 3d only and has no params */
               status = CreateConstants
                        (
                            output_ptr, 0, GC_GC, height_convert_type,
                            src_srf_ptr,dest_srf_ptr,e_constants
                        );
               break;

               case SRM_GD_3D_SRF: /* GC to GD */
               status = CreateConstants
                        (
                            output_ptr, 0, GC_GD, height_convert_type,
                            src_srf_ptr,dest_srf_ptr,e_constants
                        );
               break;

               case SRM_LTP_3D_SRF: /* GC to LTP */
               status = CreateConstants
                        (
                            output_ptr, 0, GC_LTP, height_convert_type,
                            src_srf_ptr,dest_srf_ptr,e_constants
                        );
               break;

               case SRM_EC_3D_SRF: /* GC to EC */
                    temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                        dest_srf_ptr->u.params_3d.u.
                        ec_parameters.horizontal_datum;
                    temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                        dest_srf_ptr->u.params_3d.u.
                        ec_parameters.vertical_datum;
                    temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                        dest_srf_ptr->u.params_3d.u.ec_parameters.z_units;

                    status = CreateConstants
                             (
                                 output_ptr, 0, GC_GD, height_convert_type,
                                 src_srf_ptr,&temp_srf,e_constants
                             );
                    if (status == SE_COORD_SUCCESS)
                        status = CreateConstants
                                 (
                                     output_ptr, 1, GD_EC, NO_CONVERSION,
                                     src_srf_ptr,dest_srf_ptr,e_constants
                                 );
                    break;

               case SRM_LCC_3D_SRF: /* GC to LCC */
               temp_srf.u.params_3d.u.gd_parameters.horizontal_datum = dest_srf_ptr->u.params_3d.u.lcc_parameters.horizontal_datum;
               temp_srf.u.params_3d.u.gd_parameters.vertical_datum   = dest_srf_ptr->u.params_3d.u.lcc_parameters.vertical_datum;
               temp_srf.u.params_3d.u.gd_parameters.elevation_units  = dest_srf_ptr->u.params_3d.u.lcc_parameters.z_units;

               status = CreateConstants(output_ptr,0,GC_GD,height_convert_type,src_srf_ptr,&temp_srf,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1 */
                  if (dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                      dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_M_3D_SRF: /* GC to M */
                    temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                        dest_srf_ptr->u.params_3d.u.
                        m_parameters.horizontal_datum;
                    temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                        dest_srf_ptr->u.params_3d.u.
                        m_parameters.vertical_datum;
                    temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                        dest_srf_ptr->u.params_3d.u.m_parameters.z_units;

                    status = CreateConstants
                             (
                                 output_ptr, 0, GC_GD, height_convert_type,
                                 src_srf_ptr,&temp_srf,e_constants
                             );
                    if (status == SE_COORD_SUCCESS)
                        status = CreateConstants
                                 (
                                     output_ptr,
                                     1,
                                     GD_M,
                                     NO_CONVERSION,
                                     src_srf_ptr,
                                     dest_srf_ptr,
                                     e_constants
                                 );
                    break;

               case SRM_PS_3D_SRF: /* GC to PS */
               temp_srf.u.params_3d.u.gd_parameters.horizontal_datum = dest_srf_ptr->u.params_3d.u.ps_parameters.horizontal_datum;
               temp_srf.u.params_3d.u.gd_parameters.vertical_datum   = dest_srf_ptr->u.params_3d.u.ps_parameters.vertical_datum;
               temp_srf.u.params_3d.u.gd_parameters.elevation_units  = dest_srf_ptr->u.params_3d.u.ps_parameters.z_units;

               status = CreateConstants(output_ptr,0,GC_GD,height_convert_type,src_srf_ptr,&temp_srf,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* GC to TM */
               temp_srf.u.params_3d.u.gd_parameters.horizontal_datum = dest_srf_ptr->u.params_3d.u.tm_parameters.horizontal_datum;
               temp_srf.u.params_3d.u.gd_parameters.vertical_datum   = dest_srf_ptr->u.params_3d.u.tm_parameters.vertical_datum;
               temp_srf.u.params_3d.u.gd_parameters.elevation_units  = dest_srf_ptr->u.params_3d.u.tm_parameters.z_units;

               status = CreateConstants(output_ptr,0,GC_GD,height_convert_type,src_srf_ptr,&temp_srf,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* GC to UTM */
               temp_srf.u.params_3d.u.gd_parameters.horizontal_datum = dest_srf_ptr->u.params_3d.u.utm_parameters.horizontal_datum;
               temp_srf.u.params_3d.u.gd_parameters.vertical_datum   = dest_srf_ptr->u.params_3d.u.utm_parameters.vertical_datum;
               temp_srf.u.params_3d.u.gd_parameters.elevation_units  = dest_srf_ptr->u.params_3d.u.utm_parameters.z_units;

               status = CreateConstants(output_ptr,0,GC_GD,height_convert_type,src_srf_ptr,&temp_srf,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* End GC to X */
         }
         else /* GC to X_2D */
         {
            temp_srf.is_2D = SE_TRUE;
            temp_srf.u.params_2d.spatial_reference_frame = SRM_GD_2D_SRF;

            switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_GD_2D_SRF: /* GC to GD */
               status = CreateConstants(output_ptr,0,GC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_2D_SRF: /* GC to LTP */
               status = CreateConstants(output_ptr,0,GC_LTP,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* GC to EC */

               temp_srf.u.params_2d.u.gd_parameters.horizontal_datum = dest_srf_ptr->u.params_2d.u.ec_parameters.horizontal_datum;

               status = CreateConstants(output_ptr,0,GC_GD,height_convert_type,src_srf_ptr,&temp_srf,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break;

               case SRM_LCC_2D_SRF: /* GC to LCC */

               temp_srf.u.params_2d.u.gd_parameters.horizontal_datum = dest_srf_ptr->u.params_2d.u.lcc_parameters.horizontal_datum;

               status = CreateConstants(output_ptr,0,GC_GD,height_convert_type,src_srf_ptr,&temp_srf,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1 */
                  if (dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                      dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_M_2D_SRF: /* GC to M */

               temp_srf.u.params_2d.u.gd_parameters.horizontal_datum = dest_srf_ptr->u.params_2d.u.m_parameters.horizontal_datum;

               status = CreateConstants(output_ptr,0,GC_GD,height_convert_type,src_srf_ptr,&temp_srf,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_2D_SRF: /* GC to PS */

               temp_srf.u.params_2d.u.gd_parameters.horizontal_datum = dest_srf_ptr->u.params_2d.u.ps_parameters.horizontal_datum;
               status = CreateConstants(output_ptr,0,GC_GD,height_convert_type,src_srf_ptr,&temp_srf,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_2D_SRF: /* GC to TM */

               temp_srf.u.params_2d.u.gd_parameters.horizontal_datum = dest_srf_ptr->u.params_2d.u.tm_parameters.horizontal_datum;

               status = CreateConstants(output_ptr,0,GC_GD,height_convert_type,src_srf_ptr,&temp_srf,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_2D_SRF: /* GC to UTM */

               temp_srf.u.params_2d.u.gd_parameters.horizontal_datum = dest_srf_ptr->u.params_2d.u.utm_parameters.horizontal_datum;

               status = CreateConstants(output_ptr,0,GC_GD,height_convert_type,src_srf_ptr,&temp_srf,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* End GC to X_2D*/
         }
         break;

         case SRM_TM_3D_SRF:  /* Case is TM to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* TM_3D TO X_3D */
         {
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_TM_3D_SRF: /* TM to GD to TM */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break; /* end tm to tm*/

               case SRM_GD_3D_SRF: /* TM to GD */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_3D_SRF: /* TM to LTP */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = TM_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* TM to EC */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* TM to GC */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* TM to LCC */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1 */
                  if (dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                      dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break; /* end tm to lcc */

               case SRM_M_3D_SRF: /* TM to M */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_3D_SRF: /* TM to PS */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* TM to UTM */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch on TM  to X */
         }
         else /* TM_3D TO X_2D */
         {
            switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_TM_2D_SRF: /* TM to GD to TM */

               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break; /* end tm to tm*/

               case SRM_GD_2D_SRF: /* TM to GD */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_2D_SRF: /* TM to LTP */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = TM_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* TM to EC */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* TM to LCC */
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1 */
                  if (dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                      dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break; /* end tm to lcc */

               case SRM_M_2D_SRF: /* TM to M */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_2D_SRF: /* TM to PS */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break;

               case SRM_UTM_2D_SRF: /* TM to UTM */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch on TM  to X_2D*/
         }
         break;

         case SRM_UTM_3D_SRF: /* Case is UTM to X */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* UTM_3D TO X_3D */
         {
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_UTM_3D_SRF: /* UTM to GD to UTM */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_3D_SRF: /* UTM to GD */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_3D_SRF: /* UTM to LTP */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = UTM_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* UTM to EC */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* UTM to GC */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* UTM to LCC */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1 */
                  if (dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                      dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break; /* end Utm to lcc */

               case SRM_M_3D_SRF: /* UTM to M */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_3D_SRF: /* UTM to PS */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* UTM to TM */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch on UTM  to X */
         }
         else /* UTM_3D TO X_2D */
         {
           switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_UTM_2D_SRF: /* UTM to GD to UTM */

               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break; /* end UTM to UTM*/

               case SRM_GD_2D_SRF: /* UTM to GD */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_2D_SRF: /* UTM to LTP */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = UTM_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* UTM to EC */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* UTM to LCC */
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1 */
                  if (dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                      dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break; /* end UTM to lcc */

               case SRM_M_2D_SRF: /* UTM to M */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_2D_SRF: /* UTM to PS */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break;

               case SRM_TM_2D_SRF: /* UTM to TM */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch on TM  to X_2D*/
         }
         break;

         case SRM_LTP_3D_SRF:  /* Case is LTP to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* LTP TO X_3D */
         {
            temp_srf.is_2D = SE_FALSE;
            temp_srf.u.params_3d.spatial_reference_frame = SRM_GD_3D_SRF;
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_LTP_3D_SRF: /* LTP to GC to LTP */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               /* end ltp to ltp */
               break;

               case SRM_GC_3D_SRF: /* LTP to GC */
               status = CreateConstants(output_ptr,1,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_3D_SRF:
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* LTP to EC */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.ec_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.ec_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.ec_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* LTP to LCC */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {

                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
                  if(status == SE_COORD_SUCCESS)
                  {
                     /* LCC1 */
                     if (dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                         dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                     {
                        status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                     }
                     else /* LCC2 */
                     {
                        status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                     }
                  }
               }
               break; /* end ltp to lcc */

               case SRM_M_3D_SRF: /* LTP to M */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.m_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.m_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.m_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* LTP to TM */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.tm_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.tm_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.tm_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* LTP to UTM */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.utm_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.utm_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.utm_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_3D_SRF: /* LTP to PS */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.ps_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.ps_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.ps_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }

               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch ltp to X_3D */
         }
         else /* ltp_3d to x_2d */
         {
            temp_srf.is_2D = SE_TRUE;
            temp_srf.u.params_2d.spatial_reference_frame = SRM_GD_2D_SRF;
            switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_LTP_2D_SRF: /* LTP to GC to LTP */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               /* end ltp to ltp */
               break;

               case SRM_GD_2D_SRF:
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* LTP to EC */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_2d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.ec_parameters.horizontal_datum;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* LTP to LCC */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {

                  temp_srf.u.params_2d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_2d.u.lcc_parameters.horizontal_datum;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
                  if(status == SE_COORD_SUCCESS)
                  {
                     /* LCC1 */
                     if (dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                         dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                     {
                        status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                     }
                     else /* LCC2 */
                     {
                        status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                     }
                  }
               }
               break; /* end ltp to lcc */

               case SRM_M_2D_SRF: /* LTP to M */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_2d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_2d.u.m_parameters.horizontal_datum;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_2D_SRF: /* LTP to TM */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_2d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_2d.u.tm_parameters.horizontal_datum;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_2D_SRF: /* LTP to UTM */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_2d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.utm_parameters.horizontal_datum;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_2D_SRF: /* LTP to PS */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_2d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_2d.u.ps_parameters.horizontal_datum;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }

               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch ltp to X_3D */

         }
         break;

         default:


         #ifdef CONV_DEBUG
         fprintf(f_ptr, "Error: Unknown source SRF %s",SRM_PrintSRF3D
                     (src_srf_ptr->u.params_3d.spatial_reference_frame));
         #endif

         status = SE_COORD_INVALID_SOURCE_SRF_PARAMETERS;
     } /* end switch on source srf */
 } /* end if 3D */
else /* mirror for 2d */
{
      /* Source SRF is 2D               */
      switch (src_srf_ptr->u.params_2d.spatial_reference_frame)
      {
         case SRM_UPS_2D_SRF: /* Case is UPS to UPS only */
         status = CreateConstants
                  (
                      output_ptr,
                      0,
                      UPS_UPS,
                      height_convert_type,
                      src_srf_ptr,
                      dest_srf_ptr,
                      e_constants
                  );
         break;

         case SRM_LCC_2D_SRF:   /* Case is LCC 2D to X(2D or 3D) Constants */

         /*
          * If the two parallels are equal, then we are doing LCC with one parallel (LCC1)
          */

         if (src_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
            src_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
         {
            /* source is 2D, check dest */

            if(dest_srf_ptr->is_2D == SE_FALSE) /* dest is 3D */
            {
               /* these cases are all LCC1 2d to 3d */
               switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
               {
                  case SRM_LCC_3D_SRF: /* LCC1 to GD to LCC1 */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_GD_3D_SRF: /* LCC1 to GD */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_LTP_3D_SRF: /* LCC1 to LTP */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                  {
                     output_ptr->constants[0]->vector_convert_type = LCC_TO_LTP;
                     status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break; /* end lcc1 to ltp */

                  case SRM_EC_3D_SRF: /* LCC1 to EC */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_GC_3D_SRF: /* LCC1 to GC */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_M_3D_SRF: /* LCC1 to M */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_PS_3D_SRF: /* LCC1 to PS */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_TM_3D_SRF: /* LCC1 to TM */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_UTM_3D_SRF: /* LCC1 to UTM */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  default:

                  #ifdef CONV_DEBUG

                  fprintf(f_ptr,
                  conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                  SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                  SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

                  #endif

                  status = SE_COORD_UNSUPPORTED;
               } /* end switch lcc1 to X 3d to 3D    */
            } /* end dest is 3D */
            else /* LCC1 to X dest SRF is 2D    */
            {
               switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
               {
                  case SRM_LCC_2D_SRF: /* LCC1 to GD to LCC1 */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_GD_2D_SRF: /* LCC1 to GD */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_LTP_2D_SRF: /* LCC1 to LTP */
                  output_ptr->constants[0]->vector_convert_type = LCC_TO_LTP;

                  if(status == SE_COORD_SUCCESS)
                  {
                     status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                     status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break; /* end lcc1 to ltp */

                  case SRM_EC_2D_SRF: /* LCC1 to EC */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_M_2D_SRF: /* LCC1 to M */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_PS_2D_SRF: /* LCC1 to PS */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_TM_2D_SRF: /* LCC1 to TM */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_UTM_2D_SRF: /* LCC1 to UTM */
                  status = CreateConstants(output_ptr,0,LCC1_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  default:

                  #ifdef CONV_DEBUG

                  fprintf(f_ptr,
                  conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                  SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                  SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

                  #endif

                  status = SE_COORD_UNSUPPORTED;
               } /* end switch lcc1 to X source SRF 3D, dest SRF 2D    */
            } /* end lcc 1, source is 3D */
         }
         else /* LCC2, parallels are not equal */
         {
            if(dest_srf_ptr->is_2D == SE_FALSE) /* 2D LCC2 to 3D LCC2 */
            {
               switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
               {
                  case SRM_LCC_3D_SRF: /* LCC2 to LCC2 */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_GD_3D_SRF: /* LCC2 to GD */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_LTP_3D_SRF: /* LCC2 to LTP */

                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                  {
                     output_ptr->constants[0]->vector_convert_type = LCC_TO_LTP;
                     status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break; /* end lcc1 to ltp */

                  case SRM_EC_3D_SRF: /* LCC2 to EC */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_GC_3D_SRF: /* LCC2 to GC */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_M_3D_SRF: /* LCC2 to M */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_PS_3D_SRF: /* LCC2 to PS */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_TM_3D_SRF: /* LCC2 to TM */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_UTM_3D_SRF: /* LCC2 to UTM */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  default:
                  #ifdef CONV_DEBUG
                  fprintf(f_ptr,
                  conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                  SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                  SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));
                  #endif
                  status = SE_COORD_UNSUPPORTED;

               } /* end switch lcc2 to x 3D*/
            } /* end lcc2 to x 3D */
            else /* lcc2 to x 2d */
            {
               switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
               {
                  case SRM_LCC_2D_SRF: /* LCC2 to LCC2 */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_GD_2D_SRF: /* LCC2 to GD */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_LTP_2D_SRF: /* LCC2 to LTP */

                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                  {
                     output_ptr->constants[0]->vector_convert_type = LCC_TO_LTP;
                     status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break; /* end lcc1 to ltp */

                  case SRM_EC_2D_SRF: /* LCC2 to EC */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_M_2D_SRF: /* LCC2 to M */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_PS_2D_SRF: /* LCC2 to PS */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_TM_2D_SRF: /* LCC2 to TM */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  case SRM_UTM_2D_SRF: /* LCC2 to UTM */
                  status = CreateConstants(output_ptr,0,LCC2_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
                  if(status == SE_COORD_SUCCESS)
                     status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  break;

                  default:
                  #ifdef CONV_DEBUG
                  fprintf(f_ptr,
                  conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                  SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                  SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
                  #endif
                  status = SE_COORD_UNSUPPORTED;

               } /* end switch lcc2 to x 2d*/
            } /* end lcc2 to x 2d */
         } /* end lcc2 */
         break; /* End of LCC to X Constants */

         case SRM_M_2D_SRF:  /* Case is M_3D to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* dest is 3D */
         {
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_M_3D_SRF: /* M to GD to M */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_3D_SRF: /* M to GD */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_3D_SRF: /* M to LTP */

               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = M_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* M to EC */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* M to GC */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* M to LCC */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1, we know the dest is 3d */
                  if(dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_PS_3D_SRF: /* M_3D to PS_3D */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* M to TM */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* M to UTM */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:
               #ifdef CONV_DEBUG
                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));
               #endif
               status = SE_COORD_UNSUPPORTED;
            } /* switch end M to X */
         } /* end M_2D to 3D */
         else /* M_2D to 2D  */
         {
            switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_M_2D_SRF: /* M to GD to M */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_2D_SRF: /* M to GD */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_2D_SRF: /* M to LTP */

               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = M_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* M to EC */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* M to LCC */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC2, we know the dest is 2d */
                  if(dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_PS_2D_SRF: /* M_3D to PS_3D */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_2D_SRF: /* M to TM */
               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_2D_SRF: /* M to UTM */
                status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:
               #ifdef CONV_DEBUG
                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));
               #endif
               status = SE_COORD_UNSUPPORTED;
            } /* switch end M to X */
         }
         break;

         case SRM_EC_2D_SRF:  /* Case is EC to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* EC_3d TO X 3D */
         {
            switch(dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_GD_3D_SRF: /* EC to GD */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* EC to GC */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                 status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* EC to GD to EC */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* EC_3D to LCC_3D */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1, we know the dest is 3d */
                  if(dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_LTP_3D_SRF: /* EC to LTP */

               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = EC_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_M_3D_SRF: /* EC to M */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_3D_SRF: /* EC to PS */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* EC to TM */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* EC to UTM */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;

            } /* end switch EC to X_3D */
         }
         else /* EC_2D to X_2D */
         {
            switch(dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_GD_2D_SRF: /* EC to GD */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* EC to GD to EC */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* EC_2D to LCC_2D */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1, we know the dest is 2d */
                  if(dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_LTP_2D_SRF: /* EC to LTP */

               status = CreateConstants(output_ptr,0,M_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = EC_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_M_2D_SRF: /* EC to M */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_2D_SRF: /* EC to PS */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_2D_SRF: /* EC to TM */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_2D_SRF: /* EC to UTM */
               status = CreateConstants(output_ptr,0,EC_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch EC to X_3D */
         }
         break;

         case SRM_PS_2D_SRF:  /* Case is PS to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* PS_2D to X_3D */
         {
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_PS_3D_SRF: /* PS to GD to PS_3D */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_3D_SRF: /* PS to GD */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_3D_SRF: /* PS to LTP */

               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = PS_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* PS to EC */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* PS to GC */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* PS to LCC */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1, we know the dest is 3d */
                  if(dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_M_3D_SRF: /* PS to M */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* PS to TM */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* PS to UTM */
                status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch PS to X */
         }
         else /* PS_2D to X_2D */
         {
           switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_PS_2D_SRF: /* PS to GD to PS_3D */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_2D_SRF: /* PS to GD */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_2D_SRF: /* PS to LTP */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = PS_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* PS to EC */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* PS to LCC */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1, we know the dest is 2d */
                  if(dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                     dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break;

               case SRM_M_2D_SRF: /* PS to M */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_2D_SRF: /* PS to TM */
               status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_2D_SRF: /* PS to UTM */
                status = CreateConstants(output_ptr,0,PS_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch PS to X_2D */
         } /* end else */
         break;

         case SRM_GD_2D_SRF: /* Case is GD to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* GD_2D to X_3D */
         {
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_GD_3D_SRF: /* GD to GD */
               status = CreateConstants(output_ptr,0,GD_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_3D_SRF: /* GD to LTP */
               status = CreateConstants(output_ptr,0,GD_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = GD_TO_LTP;
                  status = CreateConstants(output_ptr,1,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               break;

               case SRM_EC_3D_SRF: /* GD to EC */
               status = CreateConstants(output_ptr,0,GD_EC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* GD to GC */
               status = CreateConstants(output_ptr,0,GD_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* GD to LCC */
               /* LCC1 */
               if (dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                   dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
               {
                  status = CreateConstants(output_ptr,0,GD_LCC1,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               else /* LCC2 */
               {
                  status = CreateConstants(output_ptr,0,GD_LCC2,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               break;

               case SRM_PS_3D_SRF: /* GD to PS */
               status = CreateConstants(output_ptr,0,GD_PS,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_M_3D_SRF: /* GD to M */
               status = CreateConstants(output_ptr,0,GD_M,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* GD to TM */
               status = CreateConstants(output_ptr,0,GD_TM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* GD to UTM */
               status = CreateConstants(output_ptr,0,GD_UTM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch gd to x_3D */
         }
         else /* gd_2d to x_2d */
         {
            switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_GD_2D_SRF: /* GD to GD */
               status = CreateConstants(output_ptr,0,GD_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_2D_SRF: /* GD to LTP */
               status = CreateConstants(output_ptr,0,GD_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = GD_TO_LTP;
                  status = CreateConstants(output_ptr,1,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               break;

               case SRM_EC_2D_SRF: /* GD to EC */
               status = CreateConstants(output_ptr,0,GD_EC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* GD to LCC */
               /* LCC1 */
               if (dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                   dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
               {
                  status = CreateConstants(output_ptr,0,GD_LCC1,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               else /* LCC2 */
               {
                  status = CreateConstants(output_ptr,0,GD_LCC2,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               break;

               case SRM_PS_2D_SRF: /* GD to PS */
               status = CreateConstants(output_ptr,0,GD_PS,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_M_2D_SRF: /* GD to M */
               status = CreateConstants(output_ptr,0,GD_M,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_2D_SRF: /* GD to TM */
               status = CreateConstants(output_ptr,0,GD_TM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_2D_SRF: /* GD to UTM */
               status = CreateConstants(output_ptr,0,GD_UTM,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch gd to x_2D */
         }
         break;  /* End GD to X Constants */



         case SRM_TM_2D_SRF:  /* Case is TM to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* TM_3D TO X_3D */
         {
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_TM_3D_SRF: /* TM to GD to TM */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break; /* end tm to tm*/

               case SRM_GD_3D_SRF: /* TM to GD */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_3D_SRF: /* TM to LTP */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = TM_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* TM to EC */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* TM to GC */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* TM to LCC */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1 */
                  if (dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                      dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break; /* end tm to lcc */

               case SRM_M_3D_SRF: /* TM to M */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_3D_SRF: /* TM to PS */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* TM to UTM */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch on TM  to X */
         }
         else /* TM_2D TO X_2D */
         {
            switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_TM_2D_SRF: /* TM to GD to TM */

               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break; /* end tm to tm*/

               case SRM_GD_2D_SRF: /* TM to GD */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_2D_SRF: /* TM to LTP */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = TM_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* TM to EC */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* TM to LCC */
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1 */
                  if (dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                      dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break; /* end tm to lcc */

               case SRM_M_2D_SRF: /* TM to M */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_2D_SRF: /* TM to PS */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break;

               case SRM_UTM_2D_SRF: /* TM to UTM */
               status = CreateConstants(output_ptr,0,TM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch on TM  to X_2D*/
         }
         break;

         case SRM_UTM_2D_SRF: /* Case is UTM to X */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* UTM_2D TO X_3D */
         {
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_UTM_3D_SRF: /* UTM to GD to UTM */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_3D_SRF: /* UTM to GD */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_3D_SRF: /* UTM to LTP */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = UTM_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* UTM to EC */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GC_3D_SRF: /* UTM to GC */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* UTM to LCC */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1 */
                  if (dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                      dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break; /* end Utm to lcc */

               case SRM_M_3D_SRF: /* UTM to M */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_3D_SRF: /* UTM to PS */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* UTM to TM */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch on UTM  to X */
         }
         else /* UTM_2D TO X_2D */
         {
           switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
               case SRM_UTM_2D_SRF: /* UTM to GD to UTM */

               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break; /* end UTM to UTM*/

               case SRM_GD_2D_SRF: /* UTM to GD */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LTP_2D_SRF: /* UTM to LTP */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  output_ptr->constants[0]->vector_convert_type = UTM_TO_LTP;
                  status = CreateConstants(output_ptr,1,GD_GC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_2D_SRF: /* UTM to EC */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_2D_SRF: /* UTM to LCC */
               if(status == SE_COORD_SUCCESS)
               {
                  /* LCC1 */
                  if (dest_srf_ptr->u.params_2d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                      dest_srf_ptr->u.params_2d.u.lcc_parameters.south_parallel_geodetic_latitude)
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
                  else /* LCC2 */
                  {
                     status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                  }
               }
               break; /* end UTM to lcc */

               case SRM_M_2D_SRF: /* UTM to M */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_2D_SRF: /* UTM to PS */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break;

               case SRM_TM_2D_SRF: /* UTM to TM */
               status = CreateConstants(output_ptr,0,UTM_GD,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);

               break;

               default:

               #ifdef CONV_DEBUG

                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF3D(src_srf_ptr->u.params_3d.spatial_reference_frame),
                   SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.spatial_reference_frame));

               #endif

               status = SE_COORD_UNSUPPORTED;
            } /* end switch on TM  to X_2D*/
         }
         break;

         case SRM_LTP_2D_SRF:  /* Case is LTP to X Constants */
         if(dest_srf_ptr->is_2D == SE_FALSE) /* LTP TO X_3D */
         {
            temp_srf.is_2D = SE_FALSE;
            temp_srf.u.params_3d.spatial_reference_frame = SRM_GD_3D_SRF;
            switch (dest_srf_ptr->u.params_3d.spatial_reference_frame)
            {
               case SRM_LTP_3D_SRF: /* LTP to GC to LTP */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GC_LTP,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               /* end ltp to ltp */
               break;

               case SRM_GC_3D_SRF: /* LTP to GC */
               status = CreateConstants(output_ptr,1,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_GD_3D_SRF:
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_EC_3D_SRF: /* LTP to EC */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.ec_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.ec_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.ec_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_EC,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_LCC_3D_SRF: /* LTP to LCC */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {

                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.lcc_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
                  if(status == SE_COORD_SUCCESS)
                  {
                     /* LCC1 */
                     if (dest_srf_ptr->u.params_3d.u.lcc_parameters.north_parallel_geodetic_latitude ==
                         dest_srf_ptr->u.params_3d.u.lcc_parameters.south_parallel_geodetic_latitude)
                     {
                        status = CreateConstants(output_ptr,1,GD_LCC1,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                     }
                     else /* LCC2 */
                     {
                        status = CreateConstants(output_ptr,1,GD_LCC2,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
                     }
                  }
               }
               break; /* end ltp to lcc */

               case SRM_M_3D_SRF: /* LTP to M */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.m_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.m_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.m_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_M,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_TM_3D_SRF: /* LTP to TM */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.tm_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.tm_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.tm_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_TM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_UTM_3D_SRF: /* LTP to UTM */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.utm_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.utm_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.utm_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }
               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_UTM,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               case SRM_PS_3D_SRF: /* LTP to PS */
               status = CreateConstants(output_ptr,0,LTP_GC,height_convert_type,src_srf_ptr,dest_srf_ptr,e_constants);
               if(status == SE_COORD_SUCCESS)
               {
                  temp_srf.u.params_3d.u.gd_parameters.horizontal_datum =
                     dest_srf_ptr->u.params_3d.u.ps_parameters.horizontal_datum;

                  temp_srf.u.params_3d.u.gd_parameters.vertical_datum =
                     dest_srf_ptr->u.params_3d.u.ps_parameters.vertical_datum;

                  temp_srf.u.params_3d.u.gd_parameters.elevation_units =
                     dest_srf_ptr->u.params_3d.u.ps_parameters.z_units;

                  status = CreateConstants(output_ptr,1,GC_GD,NO_CONVERSION,src_srf_ptr,&temp_srf,e_constants);
               }

               if(status == SE_COORD_SUCCESS)
                  status = CreateConstants(output_ptr,2,GD_PS,NO_CONVERSION,src_srf_ptr,dest_srf_ptr,e_constants);
               break;

               default:
#ifdef CONV_DEBUG
                   fprintf(f_ptr,
                   conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                   SRM_PrintSRF2D(src_srf_ptr->u.params_2d.spatial_reference_frame),
                   SRM_PrintSRF3D(dest_srf_ptr->u.params_3d.spatial_reference_frame));
#endif
               status = SE_COORD_UNSUPPORTED;
            } /* end switch ltp to X_3D */
         }
         else /* ltp_2d to x_2d */
         {
            temp_srf.is_2D = SE_TRUE;
            temp_srf.u.params_2d.spatial_reference_frame = SRM_GD_2D_SRF;
            switch (dest_srf_ptr->u.params_2d.spatial_reference_frame)
            {
                case SRM_LTP_2D_SRF: /* LTP to GC to LTP */
                     status = CreateConstants
                              (
                                  output_ptr,
                                  0,
                                  LTP_GC,
                                  height_convert_type,
                                  src_srf_ptr,
                                  dest_srf_ptr, /* $$$ ? not temp ? */
                                  e_constants
                              );
                     if (status == SE_COORD_SUCCESS)
                         status = CreateConstants
                                  (
                                      output_ptr,
                                      1,
                                      GC_LTP,
                                      NO_CONVERSION,
                                      src_srf_ptr, /* $$$ ? not temp ? */
                                      dest_srf_ptr,
                                      e_constants
                                  );
                     break;

                case SRM_GD_2D_SRF:
                     status = CreateConstants
                              (
                                  output_ptr,
                                  0,
                                  LTP_GC,
                                  height_convert_type,
                                  src_srf_ptr,
                                  dest_srf_ptr, /* $$$ ? not temp ? */
                                  e_constants
                              );
                     if (status == SE_COORD_SUCCESS)
                         status = CreateConstants
                                  (
                                      output_ptr,
                                      1,
                                      GC_GD,
                                      NO_CONVERSION,
                                      src_srf_ptr, /* $$$ ? not temp ? */
                                      dest_srf_ptr,
                                      e_constants
                                  );
                     break;

                case SRM_EC_2D_SRF: /* LTP to EC */
                     status = CreateConstants
                              (
                                  output_ptr,
                                  0,
                                  LTP_GC,
                                  height_convert_type,
                                  src_srf_ptr,
                                  dest_srf_ptr, /* $$$ ? not temp ? */
                                  e_constants
                              );
                     if (status == SE_COORD_SUCCESS)
                     {
                         temp_srf.u.params_2d.u.gd_parameters.
                                  horizontal_datum =
                           dest_srf_ptr->u.params_3d.u.ec_parameters.
                                  horizontal_datum;

                         status = CreateConstants
                                  (
                                      output_ptr,
                                      1,
                                      GC_GD,
                                      NO_CONVERSION,
                                      src_srf_ptr, /* $$$ ? not prev output */
                                     &temp_srf,
                                      e_constants
                                  );
                     }
                     if (status == SE_COORD_SUCCESS)
                         status = CreateConstants
                                  (
                                      output_ptr,
                                      2,
                                      GD_EC,
                                      NO_CONVERSION,
                                      src_srf_ptr, /* $$$? */
                                      dest_srf_ptr,
                                      e_constants
                                  );
                     break;

                case SRM_LCC_2D_SRF: /* LTP to LCC */
                     status = CreateConstants
                              (
                                  output_ptr,
                                  0,
                                  LTP_GC,
                                  height_convert_type,
                                  src_srf_ptr,
                                  dest_srf_ptr,
                                  e_constants
                              );
                     if (status == SE_COORD_SUCCESS)
                     {
                         temp_srf.u.params_2d.u.gd_parameters.
                             horizontal_datum =
                             dest_srf_ptr->u.params_3d.u.lcc_parameters.
                             horizontal_datum;

                         status = CreateConstants
                                  (
                                      output_ptr,
                                      1,
                                      GC_GD,
                                      NO_CONVERSION,
                                      src_srf_ptr,
                                     &temp_srf,
                                      e_constants
                                  );
                         if (status == SE_COORD_SUCCESS)
                         {
                             /* LCC1 */
                             if (dest_srf_ptr->u.params_2d.u.lcc_parameters.
                                 north_parallel_geodetic_latitude ==
                                 dest_srf_ptr->u.params_2d.u.lcc_parameters.
                                 south_parallel_geodetic_latitude)
                             {
                                 status = CreateConstants
                                          (
                                              output_ptr,
                                              1,
                                              GD_LCC1,
                                              NO_CONVERSION,
                                              src_srf_ptr, /* $$$? */
                                              dest_srf_ptr,
                                              e_constants
                                          );
                             }
                             else /* LCC2 */
                             {
                                 status = CreateConstants
                                          (
                                              output_ptr,
                                              1,
                                              GD_LCC2,
                                              NO_CONVERSION,
                                              src_srf_ptr, /* $$$? */
                                              dest_srf_ptr,
                                              e_constants
                                          );
                             }
                         }
                     }
                     break; /* end ltp to lcc */

                case SRM_M_2D_SRF: /* LTP to M */
                     status = CreateConstants
                              (
                                  output_ptr,
                                  0,
                                  LTP_GC,
                                  height_convert_type,
                                  src_srf_ptr,
                                  dest_srf_ptr,
                                  e_constants
                              );
                     if (status == SE_COORD_SUCCESS)
                     {
                         temp_srf.u.params_2d.u.gd_parameters.
                             horizontal_datum =
                             dest_srf_ptr->u.params_2d.u.m_parameters.
                             horizontal_datum;

                         status = CreateConstants
                                  (
                                      output_ptr,
                                      1,
                                      GC_GD,
                                      NO_CONVERSION,
                                      src_srf_ptr,
                                      &temp_srf,
                                      e_constants
                                  );
                     }
                     if (status == SE_COORD_SUCCESS)
                         status = CreateConstants
                                  (
                                      output_ptr,
                                      2,
                                      GD_M,
                                      NO_CONVERSION,
                                      src_srf_ptr,
                                      dest_srf_ptr,
                                      e_constants
                                  );
                     break;

                case SRM_TM_2D_SRF: /* LTP to TM */
                     status = CreateConstants
                              (
                                  output_ptr,
                                  0,
                                  LTP_GC,
                                  height_convert_type,
                                  src_srf_ptr,
                                  dest_srf_ptr,
                                  e_constants
                              );
                     if (status == SE_COORD_SUCCESS)
                     {
                         temp_srf.u.params_2d.u.gd_parameters.
                             horizontal_datum =
                             dest_srf_ptr->u.params_2d.u.tm_parameters.
                             horizontal_datum;

                         status = CreateConstants
                                  (
                                      output_ptr,
                                      1,
                                      GC_GD,
                                      NO_CONVERSION,
                                      src_srf_ptr,
                                      &temp_srf,
                                      e_constants
                                  );
                     }
                     if (status == SE_COORD_SUCCESS)
                         status = CreateConstants
                                  (
                                      output_ptr,
                                      2,
                                      GD_TM,
                                      NO_CONVERSION,
                                      src_srf_ptr,
                                      dest_srf_ptr,
                                      e_constants
                                  );
                     break;

                case SRM_UTM_2D_SRF: /* LTP to UTM */
                     status = CreateConstants
                              (
                                  output_ptr,
                                  0,
                                  LTP_GC,
                                  height_convert_type,
                                  src_srf_ptr,
                                  dest_srf_ptr,
                                  e_constants
                              );
                     if (status == SE_COORD_SUCCESS)
                     {
                         temp_srf.u.params_2d.u.gd_parameters.
                             horizontal_datum =
                         dest_srf_ptr->u.params_3d.u.utm_parameters.
                             horizontal_datum;

                         status = CreateConstants
                                  (
                                      output_ptr,
                                      1,
                                      GC_GD,
                                      NO_CONVERSION,
                                      src_srf_ptr,
                                      &temp_srf,
                                      e_constants
                                  );
                     }
                     if (status == SE_COORD_SUCCESS)
                         status = CreateConstants
                                  (
                                      output_ptr,2,GD_UTM,NO_CONVERSION,
                                      src_srf_ptr,dest_srf_ptr,e_constants
                                  );
                     break;

                case SRM_PS_2D_SRF: /* LTP to PS */
                     status = CreateConstants
                              (
                                  output_ptr,0,LTP_GC,height_convert_type,
                                  src_srf_ptr,dest_srf_ptr,e_constants
                              );
                     if (status == SE_COORD_SUCCESS)
                     {
                         temp_srf.u.params_2d.u.gd_parameters.
                             horizontal_datum =
                         dest_srf_ptr->u.params_2d.u.ps_parameters.
                             horizontal_datum;

                         status = CreateConstants
                                  (
                                      output_ptr,1,GC_GD,NO_CONVERSION,
                                      src_srf_ptr,&temp_srf,e_constants
                                  );
                     }
                     if (status == SE_COORD_SUCCESS)
                         status = CreateConstants
                                  (
                                      output_ptr,2,GD_PS,NO_CONVERSION,
                                      src_srf_ptr,dest_srf_ptr,e_constants
                                  );
                     break;

                default:
#ifdef CONV_DEBUG
                     fprintf(f_ptr,
                             conv_error_msg[SE_NOT_SUPPORTED_CONVERSION_ERROR],
                             SRM_PrintSRF2D(src_srf_ptr->u.params_2d.
                                           spatial_reference_frame),
                             SRM_PrintSRF2D(dest_srf_ptr->u.params_2d.
                                           spatial_reference_frame));
#endif
                     status = SE_COORD_UNSUPPORTED;
            } /* end switch ltp to X_3D */
        }
        break;

        default:
#ifdef CONV_DEBUG
             fprintf(f_ptr, "Error: Unknown source SRF %s",SRM_PrintSRF2D
                     (src_srf_ptr->u.params_2d.spatial_reference_frame));
#endif
             status = SE_COORD_INVALID_SOURCE_SRF_PARAMETERS;
             break;
     } /* end switch on source srf */
 }

   /*
    * If the vector convert type is unset, then either it isn't going
    * to be used, or its value must equal the convert_type, so it's
    * easier to do set it here than everywhere above.
    */
    if (output_ptr->constants[0]->vector_convert_type == NO_CONVERSION)
        output_ptr->constants[0]->vector_convert_type =
            output_ptr->constants[0]->convert_type;

   /*
    * Set all constants to the same dimension.
    */
    for(i = 0;i<SRM_CONSTANTS_DEPTH;++i)
        if (output_ptr->constants[i]!=NULL)
            output_ptr->constants[i]->dest_srf_is_2D = dest_srf_ptr->is_2D;

    if(status == SE_COORD_SUCCESS)
    {
        *convert_coord_system_ptr = output_ptr; /* assign the output pointer */
    }
    else
    {
        SE_FreeCoordConversionConstants(output_ptr);
        *convert_coord_system_ptr = NULL;
    }
    return status;
} /* end SE_CreateCoordConversionConstants */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertCoordToGivenSystem1
 *
 *----------------------------------------------------------------------------
 */
static SE_COORD_STATUS_CODE_ENUM
SE_ConvertCoordToGivenSystem1
(
    const SE_CONVERT_COORD_SYSTEM_PAIR  convert_params_ptr,
          int                           c_index,
    const SE_COORD                     *src_coord_ptr,
          SE_COORD                     *dest_coord_ptr
)
{
   SE_FLOAT64  abslat, absl, al, al2, arg, axlon0, cee=0, clat, clon, dlon,
               p, poly1, poly2, r, rn, Rnc2a2, Rnhc, s1, s12, sl,
               slat, slat2, slon, sm, t, temp, tn2, tx;

   SE_FLOAT64  alpha, arg2, b3, b4, b5, b6, b6x, BF, cf, cki, cl, cp, cp2, cu,
               d1, d2, den, drz, eta2, gee, phi1, q, r1, r2, ro, roe, rr, s,
               sk2, ski, sp, sp2, su, su2, t2, tanz2, testb, testu, thet, top,
               top2, tp, tp2, u, v, w, w2, wp2, wp, xarg, xki, xlon0, z, z2,
               zo, zp2;

   SE_FLOAT64 source_lat=0.0, source_lon=0.0, source_elv=0.0,
              source_x=0.0, source_y=0.0, source_z=0.0;

   SE_FLOAT64 u_MINUS_U0, v_MINUS_V0,u_prime,v_prime,w_prime,u_double_prime,
              v_double_prime,zone;

   SE_FLOAT64 *dest_lat_ptr = NULL, *dest_lon_ptr = NULL, *dest_elv_ptr = NULL,
              *dest_x_ptr = NULL, *dest_y_ptr = NULL, *dest_z_ptr = NULL,
              delta_h,dummy;

   const SE_ERM_DATA *e_constants = NULL;

   SE_COORD_SYSTEM_CONSTANTS *constants = NULL;
                             /* pointer to the current constants */

   SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;
                             /* status code to return */

   SE_HEMISPHERE_ENUM hemisphere;  /* used for utm */

   SRM_GD_2D_COORD wgs84_location;
                  /* used to pass to SRM_GetWGS84GeoidalSeparation */

   /* Shortcuts to the specific constants */

   SE_LTP_SPECIFIC_CONSTANTS     *ltp_spec     = NULL;
   SE_EC_SPECIFIC_CONSTANTS      *ec_spec      = NULL;
   SE_GC_GD_SPECIFIC_CONSTANTS   *gc_gd_spec   = NULL;
   SE_LCC1_GD_SPECIFIC_CONSTANTS *lcc1_gd_spec = NULL;
   SE_LCC2_GD_SPECIFIC_CONSTANTS *lcc2_gd_spec = NULL;
   SE_M_GD_SPECIFIC_CONSTANTS    *m_gd_spec    = NULL;
   SE_PS_GD_SPECIFIC_CONSTANTS   *ps_gd_spec   = NULL;
   SE_TM_GD_SPECIFIC_CONSTANTS   *tm_gd_spec   = NULL;
   SE_UTM_GD_SPECIFIC_CONSTANTS  *utm_gd_spec  = NULL;
   SE_GD_LCC1_SPECIFIC_CONSTANTS *gd_lcc1_spec = NULL;
   SE_GD_LCC2_SPECIFIC_CONSTANTS *gd_lcc2_spec = NULL;
   SE_GD_M_SPECIFIC_CONSTANTS    *gd_m_spec    = NULL;
   SE_GD_PS_SPECIFIC_CONSTANTS   *gd_ps_spec   = NULL;
   SE_GD_TM_SPECIFIC_CONSTANTS   *gd_tm_spec   = NULL;
   SE_GD_UTM_SPECIFIC_CONSTANTS  *gd_utm_spec  = NULL;
   SE_GCS_GCS_SPECIFIC_CONSTANTS *gcs_gcs_spec = NULL;
   SE_OM_OM_SPECIFIC_CONSTANTS   *om_om_spec   = NULL;
   SE_LSR_LSR_SPECIFIC_CONSTANTS *lsr_lsr_spec = NULL;

#ifdef CONV_DEBUG
   FILE *f_ptr = NULL;

   f_ptr = SE_GetErrorFilePtr();
#endif

    /* Check General Coordinate Bounds ******************* */

   if (!dest_coord_ptr)
       return SE_COORD_INVALID_DEST_COORD;
   if (!src_coord_ptr || !SE_ValidCoord(src_coord_ptr, SE_TRUE, NULL))
        return SE_COORD_INVALID_SOURCE_COORD;
   if (!convert_params_ptr)
       return SE_COORD_INVALID_SOURCE_SRF_PARAMETERS;

   e_constants=&convert_params_ptr->ellipse_constants;
   constants = convert_params_ptr->constants[c_index];

   /*
    * Perform bounds-checking and setup for conversion
    */
   if (src_coord_ptr->is_2D == SE_FALSE)
   {
      switch (src_coord_ptr->u.coord_3d.spatial_reference_frame)
      {
         /* no checking required*/
         case SRM_GEI_3D_SRF:
         case SRM_GSE_3D_SRF:
         case SRM_GSM_3D_SRF:
         case SRM_SM_3D_SRF:
         case SRM_GCS_3D_SRF:
              break;

         case SRM_LSR_3D_SRF:
              source_x = src_coord_ptr->u.coord_3d.u.lsr_coord.x;
              source_y = src_coord_ptr->u.coord_3d.u.lsr_coord.y;
              source_z = src_coord_ptr->u.coord_3d.u.lsr_coord.z;
              break;

         case SRM_PS_3D_SRF:
              source_x = src_coord_ptr->u.coord_3d.u.ps_coord.x;
              source_y = src_coord_ptr->u.coord_3d.u.ps_coord.y;
              source_z = src_coord_ptr->u.coord_3d.u.ps_coord.z;

              switch (constants->convert_type)
              {
                 case PS_GD:
                      source_x += constants->specific_constants.ps_gd.x_offset;
                      source_y += constants->specific_constants.ps_gd.y_offset;
                      break;

                 default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from PS!\n");
#endif
                      return SE_COORD_UNSUPPORTED;
              } /* end switch PS */
              break;

         case SRM_LCC_3D_SRF:
              source_x = src_coord_ptr->u.coord_3d.u.lcc_coord.x;
              source_y = src_coord_ptr->u.coord_3d.u.lcc_coord.y;
              source_z = src_coord_ptr->u.coord_3d.u.lcc_coord.z;
              switch (constants->convert_type)
              {
                 case LCC1_GD:
                      source_x += constants->specific_constants.
                                             lcc1_gd.x_offset;
                      source_y += constants->specific_constants.
                                             lcc1_gd.y_offset;
                      break;
                 case LCC2_GD:
                      source_x += constants->specific_constants.
                                             lcc2_gd.x_offset;
                      source_y += constants->specific_constants.
                                             lcc2_gd.y_offset;
                      break;

                 default:
#ifdef      CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from LCC!\n");
#endif
                      return SE_COORD_UNSUPPORTED;
              } /* end switch LCC */

         break;

         case SRM_GC_3D_SRF:
              source_x = src_coord_ptr->u.coord_3d.u.gc_coord.x;
              source_y = src_coord_ptr->u.coord_3d.u.gc_coord.y;
              source_z = src_coord_ptr->u.coord_3d.u.gc_coord.z;

              switch (constants->convert_type)
              {
                 case GC_LTP:
                 case GC_GD:
                      break;

                 default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from GC!\n");
#endif
                 return SE_COORD_UNSUPPORTED;
              } /* end switch GC */
              break;

         case SRM_M_3D_SRF:
         source_x = src_coord_ptr->u.coord_3d.u.m_coord.x;
         source_y = src_coord_ptr->u.coord_3d.u.m_coord.y;
         source_z = src_coord_ptr->u.coord_3d.u.m_coord.z;

         switch (constants->convert_type)
         {
            case M_GD:
                 source_x += constants->specific_constants.m_gd.x_offset;
                 source_y += constants->specific_constants.m_gd.y_offset;
                 break;

            default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from M!\n");
#endif
                 return SE_COORD_UNSUPPORTED;
            } /* end switch M */
            break;

         case SRM_TM_3D_SRF:
         source_x = src_coord_ptr->u.coord_3d.u.tm_coord.x;
         source_y = src_coord_ptr->u.coord_3d.u.tm_coord.y;
         source_z = src_coord_ptr->u.coord_3d.u.tm_coord.z;

         switch (constants->convert_type)
         {
            case TM_GD:
                 source_x += constants->specific_constants.tm_gd.x_offset;
                 source_y += constants->specific_constants.tm_gd.y_offset;
                 break;

            default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from TM!\n");
#endif

                 return SE_COORD_UNSUPPORTED;
         } /* end switch TM */
         break;

         case SRM_EC_3D_SRF:
         source_x = src_coord_ptr->u.coord_3d.u.ec_coord.x;
         source_y = src_coord_ptr->u.coord_3d.u.ec_coord.y;
         source_z = src_coord_ptr->u.coord_3d.u.ec_coord.z;

         switch (constants->convert_type)
         {
            case EC_GD:
                 source_x += constants->specific_constants.ec.x_offset;
                 source_y += constants->specific_constants.ec.y_offset;
                 break;

            default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from EC!\n");
#endif
                 return SE_COORD_UNSUPPORTED;
         } /* end switch SE_EC */
         break;

         case SRM_GD_3D_SRF:
         source_lat = src_coord_ptr->u.coord_3d.u.gd_coord.geodetic_latitude;
         source_lon = src_coord_ptr->u.coord_3d.u.gd_coord.geodetic_longitude;
         source_elv = src_coord_ptr->u.coord_3d.u.gd_coord.elevation;

         switch (constants->convert_type)
         {
            case GD_GD:
            case GD_EC:
            case GD_LCC1:
            case GD_LCC2:
                 break;

            case GD_M:
            if ((source_lat<-89.0) || (source_lat>89.0))
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr, "Mercator latitude is limited to "\
                      "-89 to 89  degrees; source lat was %f\n",
                       source_lat);
#endif
               return SE_COORD_INVALID_DEST_COORD;
            }
            break;

            case GD_TM:
            if ((source_lat<-80.0) || (source_lat>84.0))
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr,
                  "[SEDRIS CONVERSIONS] GD to TM error: "\
                  "Source latitude out of bounds -80<lat<84)\n"\
                  "Source latitude: %e  Source longitude: %e\n",
                  source_lat, source_lon);
#endif
               return SE_COORD_INVALID_SOURCE_COORD;
            }
            break;

            case GD_UTM:
            if ((source_lat<-80.0) || (source_lat>84.0))
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr,
                        "[SEDRIS CONVERSIONS] GD to UTM error: "\
                        "source lat out of bounds -80<lat<84)    "\
                        "Source latitude: %e  Source longitude: %e\n",
                        source_lat, source_lon);
#endif
               return SE_COORD_INVALID_DEST_SRF_PARAMETERS;
            }
            break;

            case GD_PS:
            /*
             * Note: PS is valid only for polar regions.  Valid
             *       latitudes must be in the range from 84 north
             *       to the pole, or -80 south to the pole
             */

            if ((-80.0<source_lat) && (source_lat<84.0))
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr, conv_error_msg[SE_GD2PS_ERROR],
                                  source_lat);
#endif
               return SE_COORD_INVALID_DEST_SRF_PARAMETERS;
            }
            break;

            case GD_GC:
                 break;

            default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from GD!\n");
#endif
                 return SE_COORD_UNSUPPORTED;
         } /* end switch gd */
         break;

         case SRM_LTP_3D_SRF:

         source_x = src_coord_ptr->u.coord_3d.u.ltp_coord.x;
         source_y = src_coord_ptr->u.coord_3d.u.ltp_coord.y;
         source_z = src_coord_ptr->u.coord_3d.u.ltp_coord.z;

         switch (constants->convert_type)
         {
            case LTP_GC:
                 source_x += constants->specific_constants.ltp.x_offset;
                 source_y += constants->specific_constants.ltp.y_offset;
                 break;

            default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from LTP!\n");
#endif
                 return SE_COORD_UNSUPPORTED;
         }
         break;

         case SRM_UTM_3D_SRF:

         source_x = src_coord_ptr->u.coord_3d.u.utm_coord.x;
         source_y = src_coord_ptr->u.coord_3d.u.utm_coord.y;
         source_z = src_coord_ptr->u.coord_3d.u.utm_coord.z;
         hemisphere = src_coord_ptr->u.coord_3d.u.utm_coord.hemisphere;

         switch (constants->convert_type)
         {
            case UTM_GD:

            source_x += constants->specific_constants.utm_gd.x_offset;
            source_y += constants->specific_constants.utm_gd.y_offset;

            /* Check x (easting) */

            if (0 && (source_x <100000.0 ||source_x >900000.0))
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr, "%s: x/easting %f "\
                        "x_offset %f is out of bounds,\n"\
                        "Range(meters): (100,000<x<900,000)\n",
                        conv_error_msg[SE_UTM_SOURCE_COORD_ERROR],
                        src_coord_ptr->u.coord_3d.u.utm_coord.x,
                        constants->specific_constants.utm_gd.x_offset);
#endif
               return SE_COORD_INVALID_SOURCE_COORD;
            }

            /* Check y (northing) */
            if (source_y <0 || source_y >10000000)
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr,
                 "%s: y/northing %f y_offset %f is out of "\
                  "bounds,\nRange(meters): (0<y<10,000,000)\n",
                  conv_error_msg[SE_UTM_SOURCE_COORD_ERROR],
                  src_coord_ptr->u.coord_3d.u.utm_coord.y,
                  constants->specific_constants.utm_gd.y_offset);
#endif
               return SE_COORD_INVALID_SOURCE_COORD;
            }
            break;

            default:
#ifdef CONV_DEBUG
            fprintf(f_ptr, "Attempted unsupported conversion from "\
                             "UTM!\n");
#endif
            return SE_COORD_UNSUPPORTED;
            }
            break;

         default:
         return SE_COORD_INVALID_SOURCE_COORD;
      } /* end 3d */
   } /* else 2d */
   else
   {
      switch (src_coord_ptr->u.coord_2d.spatial_reference_frame)
      {
         case SRM_LSR_2D_SRF: /* no checking required*/
              source_x = src_coord_ptr->u.coord_2d.u.lsr_coord.x;
              source_y = src_coord_ptr->u.coord_2d.u.lsr_coord.y;
              break;

         case SRM_PS_2D_SRF:
         source_x = src_coord_ptr->u.coord_2d.u.ps_coord.x;
         source_y = src_coord_ptr->u.coord_2d.u.ps_coord.y;
         switch (constants->convert_type)
         {
            case PS_GD:
                 source_x += constants->specific_constants.ps_gd.x_offset;
                 source_y += constants->specific_constants.ps_gd.y_offset;
                 break;

            default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from PS!\n");
#endif
                 return SE_COORD_UNSUPPORTED;
         } /* end switch PS */
         break;

         case SRM_LCC_2D_SRF:
         source_x = src_coord_ptr->u.coord_2d.u.lcc_coord.x;
         source_y = src_coord_ptr->u.coord_2d.u.lcc_coord.y;

         switch (constants->convert_type)
         {
            case LCC1_GD:
                 source_x += constants->specific_constants.lcc1_gd.x_offset;
                 source_y += constants->specific_constants.lcc1_gd.y_offset;
                 break;

            case LCC2_GD:
                 source_x += constants->specific_constants.lcc2_gd.x_offset;
                 source_y += constants->specific_constants.lcc2_gd.y_offset;
                 break;

            default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr,
                         "Attempted unsupported conversion from LCC!\n");
#endif
                 return SE_COORD_UNSUPPORTED;
         } /* end switch lcc */
         break;

         case SRM_M_2D_SRF:
         source_x = src_coord_ptr->u.coord_2d.u.m_coord.x;
         source_y = src_coord_ptr->u.coord_2d.u.m_coord.y;
         switch (constants->convert_type)
         {
            case M_GD:
                 source_x += constants->specific_constants.m_gd.x_offset;
                 source_y += constants->specific_constants.m_gd.y_offset;
                 break;

            default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from M!\n");
#endif
                 return SE_COORD_UNSUPPORTED;
            } /* end switch M */
            break;

         case SRM_TM_2D_SRF:
         source_x = src_coord_ptr->u.coord_2d.u.tm_coord.x;
         source_y = src_coord_ptr->u.coord_2d.u.tm_coord.y;

         switch (constants->convert_type)
         {
            case TM_GD:
                 source_x += constants->specific_constants.tm_gd.x_offset;
                 source_y += constants->specific_constants.tm_gd.y_offset;
                 break;

            default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from TM!\n");
#endif
                 return SE_COORD_UNSUPPORTED;
         } /* end switch TM */
         break;

         case SRM_EC_2D_SRF:
         source_x = src_coord_ptr->u.coord_2d.u.ec_coord.x;
         source_y = src_coord_ptr->u.coord_2d.u.ec_coord.y;

         switch (constants->convert_type)
         {
            case EC_GD:
                 source_x += constants->specific_constants.ec.x_offset;
                 source_y += constants->specific_constants.ec.y_offset;
                 break;

            default:
#ifdef CONV_DEBUG
                 fprintf(f_ptr, "Attempted unsupported conversion from EC!\n");
#endif
                 return SE_COORD_UNSUPPORTED;
         } /* end switch SE_EC */
         break;

         case SRM_GD_2D_SRF:
         source_lat = src_coord_ptr->u.coord_2d.u.gd_coord.geodetic_latitude;
         source_lon = src_coord_ptr->u.coord_2d.u.gd_coord.geodetic_longitude;

         switch (constants->convert_type)
         {
            case GD_GD:
            case GD_EC:
            case GD_LCC1:
            case GD_LCC2:
            break;

            case GD_M:

            if ((source_lat<-89.0) || (source_lat>89.0))
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr, "Mercator latitude is limited to "\
                      "-89 to 89  degrees; source lat was %f\n",
                       source_lat);
#endif
               return SE_COORD_INVALID_DEST_COORD;
            }
            break;

            case GD_TM:

            if ((source_lat<-80.0) || (source_lat>84.0))
            {
#ifdef CONV_DEBUG
               fprintf(f_ptr,
                  "[SEDRIS CONVERSIONS] GD to TM error: "\
                  "Source latitude out of bounds -80<lat<84)\n"\
                  "Source latitude: %e  Source longitude: %e\n",
                  source_lat, source_lon);
#endif
               return SE_COORD_INVALID_SOURCE_COORD;
            }
            break;

            case GD_UTM:

            if ((source_lat<-80.0) || (source_lat>84.0))
            {
               #ifdef CONV_DEBUG
               fprintf(f_ptr,
                        "[SEDRIS CONVERSIONS] GD to UTM error: "\
                        "source lat out of bounds -80<lat<84)    "\
                        "Source latitude: %e  Source longitude: %e\n",
                        source_lat, source_lon);
               #endif
               return SE_COORD_INVALID_DEST_SRF_PARAMETERS;
            }
            break;

            case GD_PS:
            /*
             * Note: PS is valid only for polar regions.  Valid
             *       latitudes must be in the range from 84 north
             *       to the pole, or -80 south to the pole
             */

            if ((-80.0<source_lat) && (source_lat<84.0))
            {
               #ifdef CONV_DEBUG
               fprintf(f_ptr, conv_error_msg[SE_GD2PS_ERROR],
                                  source_lat);
               #endif
               return SE_COORD_INVALID_DEST_SRF_PARAMETERS;
            }
            break;

            case GD_GC:
                 break;

            default:
            #ifdef CONV_DEBUG
            fprintf(f_ptr, "Attempted unsupported conversion from "\
                             "GD!\n");
            #endif
            return SE_COORD_UNSUPPORTED;
         } /* end switch gd */
         break;

         case SRM_LTP_2D_SRF:

         source_x = src_coord_ptr->u.coord_2d.u.ltp_coord.x;
         source_y = src_coord_ptr->u.coord_2d.u.ltp_coord.y;

         switch (constants->convert_type)
         {
            case LTP_GC:
            source_x += constants->specific_constants.ltp.x_offset;

            source_y += constants->specific_constants.ltp.y_offset;

            source_z = 0;

            break;

            default:

            #ifdef CONV_DEBUG

            fprintf(f_ptr, "Attempted unsupported conversion from "\
                             "LTP!\n");

            #endif

            return SE_COORD_UNSUPPORTED;

         }
         break;

         case SRM_UTM_2D_SRF:

         source_x = src_coord_ptr->u.coord_2d.u.utm_coord.x;
         source_y = src_coord_ptr->u.coord_2d.u.utm_coord.y;
         hemisphere = src_coord_ptr->u.coord_2d.u.utm_coord.hemisphere;

         switch (constants->convert_type)
         {

            case UTM_GD:

            source_x += constants->specific_constants.utm_gd.x_offset;
            source_y += constants->specific_constants.utm_gd.y_offset;

            /* Check x (easting) */

            if (0 && (source_x <100000.0 ||source_x >900000.0))
            {

               #ifdef CONV_DEBUG

               fprintf(f_ptr, "%s: x/easting %f "\
                        "x_offset %f is out of bounds,\n"\
                        "Range(meters): (100,000<x<900,000)\n",
                        conv_error_msg[SE_UTM_SOURCE_COORD_ERROR],
                        src_coord_ptr->u.coord_2d.u.utm_coord.x,
                        constants->specific_constants.utm_gd.x_offset);
               #endif

               return SE_COORD_INVALID_SOURCE_COORD;
            }

            /* Check y (northing) */
            if (source_y <0 || source_y >10000000)
            {

               #ifdef CONV_DEBUG

               fprintf(f_ptr,
                 "%s: y/northing %f y_offset %f is out of "\
                  "bounds,\nRange(meters): (0<y<10,000,000)\n",
                  conv_error_msg[SE_UTM_SOURCE_COORD_ERROR],
                  src_coord_ptr->u.coord_2d.u.utm_coord.y,
                  constants->specific_constants.utm_gd.y_offset);
               #endif

               return SE_COORD_INVALID_SOURCE_COORD;

            }
            break;

            default:

            #ifdef CONV_DEBUG

            fprintf(f_ptr, "Attempted unsupported conversion from "\
                             "UTM!\n");
            #endif

            return SE_COORD_UNSUPPORTED;
            }
            break;

         default:
         return SE_COORD_INVALID_SOURCE_COORD;
      } /* end 2d */
   }

   /*
   * Note that these transformations are in no particular order.
   */
   switch (constants->convert_type)
   {
      /*
       * no unit conversion for these XXX to XXX cases
       */
       case GC_GC:
       case GM_GM:
       case GEI_GEI:
       case GSE_GSE:
       case GSM_GSM:
       case SM_SM:
            *dest_coord_ptr = *src_coord_ptr;
            break;
       case LSR_LSR:
            lsr_lsr_spec=&constants->specific_constants.lsr_lsr;
            if (constants->dest_srf_is_2D == SE_TRUE)
            {
                if (src_coord_ptr->is_2D)
                {
                    dest_coord_ptr->is_2D = SE_TRUE;
                    dest_coord_ptr->u.coord_2d.
                                    spatial_reference_frame = SRM_LSR_2D_SRF;
                    dest_coord_ptr->u.coord_2d.u.lsr_coord.x =
                        (lsr_lsr_spec->flip_matrix.mat[0][0] * source_x) +
                        (lsr_lsr_spec->flip_matrix.mat[0][1] * source_y);
                    dest_coord_ptr->u.coord_2d.u.lsr_coord.y =
                        (lsr_lsr_spec->flip_matrix.mat[1][0] * source_x) +
                        (lsr_lsr_spec->flip_matrix.mat[1][1] * source_y);
                }
                else
                {
                    return SE_COORD_INVALID_SOURCE_COORD;
                }
            }
            else /* 3D */
            {
                if (src_coord_ptr->is_2D)
                {
                    return SE_COORD_INVALID_SOURCE_COORD;
                }
                else
                {
                    dest_coord_ptr->is_2D = SE_FALSE;
                    dest_coord_ptr->u.coord_3d.
                                    spatial_reference_frame = SRM_LSR_3D_SRF;
                    dest_coord_ptr->u.coord_3d.u.lsr_coord.x =
                        (lsr_lsr_spec->flip_matrix.mat[0][0] * source_x) +
                        (lsr_lsr_spec->flip_matrix.mat[0][1] * source_y) +
                        (lsr_lsr_spec->flip_matrix.mat[0][2] * source_z);
                    dest_coord_ptr->u.coord_3d.u.lsr_coord.y =
                        (lsr_lsr_spec->flip_matrix.mat[1][0] * source_x) +
                        (lsr_lsr_spec->flip_matrix.mat[1][1] * source_y) +
                        (lsr_lsr_spec->flip_matrix.mat[1][2] * source_z);
                    dest_coord_ptr->u.coord_3d.u.lsr_coord.z =
                        (lsr_lsr_spec->flip_matrix.mat[2][0] * source_x) +
                        (lsr_lsr_spec->flip_matrix.mat[2][1] * source_y) +
                        (lsr_lsr_spec->flip_matrix.mat[2][2] * source_z);
                }
            }
            break;

       case GCS_GCS:
           /* always 3D */
            gcs_gcs_spec=&constants->specific_constants.gcs_gcs;
           *dest_coord_ptr = *src_coord_ptr;

            dest_coord_ptr->u.coord_3d.u.gcs_coord.x += gcs_gcs_spec->x_offset;
            dest_coord_ptr->u.coord_3d.u.gcs_coord.y += gcs_gcs_spec->y_offset;
           /*
            * always meters, so no unit conversion
            */
            break;

      case GD_GD:

      if (constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_GD_2D_SRF;
         dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_latitude  = source_lat;
         dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_longitude = source_lon;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
         dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_latitude  = source_lat;
         dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_longitude = source_lon;

         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_elv -= delta_h;
            break;

            case G_TO_E:
            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            source_elv += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.gd_coord.elevation =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_elv
         );

      }
      break;



      case OM_OM: /* OM to OM only provides height and unit conversion */

      om_om_spec = &constants->specific_constants.om_om;

      if (constants->dest_srf_is_2D == SE_TRUE)
      {
          dest_coord_ptr->is_2D = SE_TRUE;
          dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_OM_2D_SRF;
          dest_coord_ptr->u.coord_2d.u.om_coord.x = source_x;
          dest_coord_ptr->u.coord_2d.u.om_coord.x += om_om_spec->x_offset;

          dest_coord_ptr->u.coord_2d.u.om_coord.y = source_y;
          dest_coord_ptr->u.coord_2d.u.om_coord.y += om_om_spec->y_offset;
      }
      else
      {
          dest_coord_ptr->is_2D = SE_FALSE;
          dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_OM_3D_SRF;
          dest_coord_ptr->u.coord_3d.u.om_coord.x = source_x;
          dest_coord_ptr->u.coord_3d.u.om_coord.x += om_om_spec->x_offset;
          dest_coord_ptr->u.coord_3d.u.om_coord.y = source_y;
          dest_coord_ptr->u.coord_3d.u.om_coord.y += om_om_spec->y_offset;

         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_z -= delta_h;
            break;

            case G_TO_E:
            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            source_z += delta_h;
            break;

            default:
                 break;
         } /* end switch */
         dest_coord_ptr->u.coord_3d.u.om_coord.z =
            SE_ConvertDistance
            (
               constants->source_units,
               constants->dest_units,
               source_z
            );
      }
      break;

      case LTP_GC:

      /* source_x, source_y, and source_z already set */

      ltp_spec = &constants->specific_constants.ltp;

      /* destination is 3D by definition */

      dest_coord_ptr->is_2D = SE_FALSE;
      dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_GC_3D_SRF;

      /*
      * Convert ltp z value to meters, per gc definition
      */

      if(src_coord_ptr->is_2D == SE_FALSE)
      {
         source_z =
            SE_ConvertDistance(constants->source_units,SRM_METERS,source_z);
      }

      if (ltp_spec->Azimuth == 0.0)
      {
          u_prime = -source_y * ltp_spec->Sin_Origin_Lat +
                     source_z * ltp_spec->Cos_Origin_Lat;
          v_prime = source_x;
          w_prime = source_y * ltp_spec->Cos_Origin_Lat +
                    source_z * ltp_spec->Sin_Origin_Lat;
      }
      else
      {
          u_prime = source_x * ltp_spec->Sin_Lat_Sin_Azimuth -
                    source_y * ltp_spec->Sin_Lat_Cos_Azimuth +
                    source_z * + ltp_spec->Cos_Origin_Lat;

          v_prime = source_x * ltp_spec->Cos_Azimuth +
                    source_y * ltp_spec->Sin_Azimuth;

          w_prime = -source_x * ltp_spec->Cos_Lat_Sin_Azimuth +
                     source_y * ltp_spec->Cos_Lat_Cos_Azimuth +
                     source_z * ltp_spec->Sin_Origin_Lat;
      }

      u_double_prime = u_prime * ltp_spec->Sin_Origin_Lon -
                       v_prime * ltp_spec->Sin_Origin_Lon;
      v_double_prime = u_prime * ltp_spec->Sin_Origin_Lon +
                       v_prime * ltp_spec->Cos_Origin_Lon;;
      /* w_double_prime = w_prime */

      dest_coord_ptr->u.coord_3d.u.gc_coord.x = u_double_prime + ltp_spec->U0;
      dest_coord_ptr->u.coord_3d.u.gc_coord.y = v_double_prime + ltp_spec->V0;
      dest_coord_ptr->u.coord_3d.u.gc_coord.z = w_prime + ltp_spec->W0;
      break;

      case GC_LTP: /* GC to LTP */

      ltp_spec = &constants->specific_constants.ltp;

      u_MINUS_U0 = source_x - ltp_spec->U0;
      v_MINUS_V0 = source_y - ltp_spec->V0;

      u_prime = u_MINUS_U0 * ltp_spec->Cos_Origin_Lon + v_MINUS_V0 * ltp_spec->Sin_Origin_Lon;
      v_prime = -u_MINUS_U0 * ltp_spec->Sin_Origin_Lon + v_MINUS_V0 * ltp_spec->Cos_Origin_Lon;
      w_prime = source_z - ltp_spec->W0;

      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_LTP_2D_SRF;
         dest_x_ptr = &dest_coord_ptr->u.coord_2d.u.ltp_coord.x;
         dest_y_ptr = &dest_coord_ptr->u.coord_2d.u.ltp_coord.y;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_LTP_3D_SRF;
         dest_x_ptr = &dest_coord_ptr->u.coord_3d.u.ltp_coord.x;
         dest_y_ptr = &dest_coord_ptr->u.coord_3d.u.ltp_coord.y;
         dest_z_ptr = &dest_coord_ptr->u.coord_3d.u.ltp_coord.z;

         if(ltp_spec->Azimuth == 0.0)
         {
            dest_coord_ptr->u.coord_3d.u.ltp_coord.z =
               u_prime * ltp_spec->Cos_Origin_Lat
               + w_prime * ltp_spec->Sin_Origin_Lat;
         }
         else
         {

            dest_coord_ptr->u.coord_3d.u.ltp_coord.z =
               u_prime * ltp_spec->Cos_Origin_Lat
               + w_prime * ltp_spec->Sin_Origin_Lat;
         }

         /*
         * Convert Z or H value to proper units
         */

         /* source, GC, is in meters already */
         if(constants->dest_units != SRM_METERS)
         {
            dest_coord_ptr->u.coord_3d.u.ltp_coord.z =
            SE_ConvertDistance
            (
               constants->source_units,
               constants->dest_units,
               dest_coord_ptr->u.coord_3d.u.ltp_coord.z
            );
         }
      }


      if (ltp_spec->Azimuth == 0.0)
      {
       *dest_x_ptr = v_prime;
       *dest_y_ptr = -u_prime * ltp_spec->Sin_Origin_Lat
                     + w_prime * ltp_spec->Cos_Origin_Lat;
      }
      else
      {

       *dest_x_ptr = u_prime * ltp_spec->Sin_Lat_Sin_Azimuth
                     + v_prime * ltp_spec->Cos_Azimuth
                     - w_prime * ltp_spec->Cos_Lat_Sin_Azimuth;

       *dest_y_ptr = - u_prime * ltp_spec->Sin_Lat_Cos_Azimuth
                     + v_prime * ltp_spec->Sin_Azimuth
                     + w_prime * ltp_spec->Cos_Lat_Cos_Azimuth;

      }
      /* apply offsets */

      *dest_x_ptr -= ltp_spec->x_offset;
      *dest_y_ptr -= ltp_spec->y_offset;
      break;/* end GC to LTP */


      case GD_EC:

      /* for spheres only, no height conversion*/
      ec_spec=&constants->specific_constants.ec;

      if(constants->dest_srf_is_2D == SE_TRUE)
      {

         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_EC_2D_SRF;
         dest_x_ptr = &dest_coord_ptr->u.coord_2d.u.ec_coord.x;
         dest_y_ptr = &dest_coord_ptr->u.coord_2d.u.ec_coord.y;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_EC_3D_SRF;
         dest_x_ptr = &dest_coord_ptr->u.coord_3d.u.ec_coord.x;
         dest_y_ptr = &dest_coord_ptr->u.coord_3d.u.ec_coord.y;


        /*
         * Store Z  value and convert to proper units
         */

         dest_coord_ptr->u.coord_3d.u.ec_coord.z =
         SE_ConvertDistance
         (
         constants->source_units,
         constants->dest_units,
         source_elv
         );
      }

      source_lat *= SE_RADIANS_PER_DEGREE;
      source_lon *= SE_RADIANS_PER_DEGREE;


      /*
      * The formula for GD to EC is
      *
      * x = x_offset +
      *     (CSF*R*(lon - origin_longitude)*(cos(origin_latitude)))
      *
      * y = y_offset + (CSF*R*(lat - origin_latitude))
      */

      source_lon -=ec_spec->longitude_origin;

      /* fix quadrant on longitude */
      if (source_lon < -SE_PI)
         source_lon +=2*SE_PI;
      else if (source_lon > SE_PI)
         source_lon -=2*SE_PI;

      *dest_x_ptr = ec_spec->x_offset + (ec_spec->scaled_radius *
                    (source_lon)  *
                    cos(ec_spec->latitude_origin));

      *dest_y_ptr = ec_spec->y_offset + (ec_spec->scaled_radius *
                    (source_lat - ec_spec->latitude_origin));

      break;/* end GD to EC */

      case GD_LCC1:

      gd_lcc1_spec=&constants->specific_constants.gd_lcc1;
      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_LCC_2D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_2d.u.lcc_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_2d.u.lcc_coord.y;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_LCC_3D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_3d.u.lcc_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_3d.u.lcc_coord.y;

         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_elv -= delta_h;
            break;

            case G_TO_E:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_elv += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.lcc_coord.z =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_elv
         );

      }

      source_lat *= SE_RADIANS_PER_DEGREE;
      source_lon *= SE_RADIANS_PER_DEGREE;

      sl   = sin(source_lat);
      if(e_constants->Eps!=0) /* do ellipsoid calcs */
      {
         absl = fabs(sl);
         if (absl<=35*SE_RADIANS_PER_DEGREE)
         {
           p = gd_lcc1_spec->CR11 +
             (gd_lcc1_spec->CR12 * absl + gd_lcc1_spec->CR13) /
             (gd_lcc1_spec->CR14 + absl * (gd_lcc1_spec->CR15+absl));
         }
      else
      {
         p = gd_lcc1_spec->CR21 +
             (gd_lcc1_spec->CR22 * absl + gd_lcc1_spec->CR23) /
             (gd_lcc1_spec->CR24 + absl * (gd_lcc1_spec->CR25+absl));
      }

      if (sl < 0.0)
         p = 1 / p;

      if (sl< 89.0 * SE_RADIANS_PER_DEGREE)
         p = p * sqrt((1+sl) / (1-sl));
      else
         p = p * tan(SE_PI/4 + 0.5 * source_lat);

      } /* end if ellipsoid */
      else /* for sphere */
      {
         if (sl<.9998476952)
            p = sqrt((1+sl) / (1-sl));
         else
            p = tan(SE_PI/4 + .5 * source_lat);

      }

      r = gd_lcc1_spec->rz * pow(gd_lcc1_spec->pz/p,gd_lcc1_spec->xl);
      dlon=source_lon - gd_lcc1_spec->xlonz;

      /* fix quadrant on longitude */

      SE_FIX_LONGITUDE(dlon);

      arg=gd_lcc1_spec->xl * dlon;
      s1 = sin(arg);
      t = cos(arg);

      *dest_x_ptr=r * s1;
      *dest_y_ptr= gd_lcc1_spec->rho_origin - (r * t);

      if(constants->compute_com == SE_TRUE)
      {
         constants->sin_gamma = s1;
         constants->cos_gamma = t;
      }

      /* fix signs */
      if (gd_lcc1_spec->xl < 0.0)
      {
         *dest_x_ptr=-*dest_x_ptr;
         *dest_y_ptr=-*dest_y_ptr;
      }

      *dest_x_ptr -= gd_lcc1_spec->x_offset;
      *dest_y_ptr -= gd_lcc1_spec->y_offset;
      break;

      case GD_LCC2:

      gd_lcc2_spec=&constants->specific_constants.gd_lcc2;

      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_LCC_2D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_2d.u.lcc_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_2d.u.lcc_coord.y;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_LCC_3D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_3d.u.lcc_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_3d.u.lcc_coord.y;

         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_elv -= delta_h;
            break;

            case G_TO_E:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_elv += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.lcc_coord.z =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_elv
         );

      }

      source_lat *= SE_RADIANS_PER_DEGREE;
      source_lon *= SE_RADIANS_PER_DEGREE;

      sl = sin(source_lat);

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
        absl = fabs(sl);

        if (absl<=35.5*SE_RADIANS_PER_DEGREE)
        {
            p = gd_lcc2_spec->CR11+
                (gd_lcc2_spec->CR12 * sl + gd_lcc2_spec->CR13) /
                (gd_lcc2_spec->CR14 + sl * (gd_lcc2_spec->CR15+sl));
        }
        else
        {
            p = gd_lcc2_spec->CR21+
                (gd_lcc2_spec->CR22 * sl + gd_lcc2_spec->CR23) /
                (gd_lcc2_spec->CR24 + sl * (gd_lcc2_spec->CR25+sl));
        }

       /*
        * if sl < 89 degrees
        */
        if (sl<.9998476952)
            p=p * sqrt((1.0+sl)/(1.0-sl));
        else
            p=p * tan(SE_PI/4.0 + .5 * source_lat);
      }
      else /* compute spherical values */
      {
        if (sl<.9998476952)
            p=sqrt((1.0+sl)/(1.0-sl));
        else
            p=tan(SE_PI/4.0 + .5 * source_lat);
      }
      r=gd_lcc2_spec->rx*pow(gd_lcc2_spec->p1/p,gd_lcc2_spec->xl);

      dlon=source_lon - gd_lcc2_spec->xlonz;

      /* fix quadrant on longitude */
      SE_FIX_LONGITUDE(dlon);

      arg=gd_lcc2_spec->xl*dlon;

      s1 = sin(arg);
      t = cos(arg);

      *dest_x_ptr=r * s1;
      *dest_y_ptr=gd_lcc2_spec->rz - r * t;

      if(constants->compute_com == SE_TRUE)
      {
         constants->sin_gamma = s1;
         constants->cos_gamma = t;
      }

      if (gd_lcc2_spec->xl<0)
      {
        *dest_x_ptr=-*dest_x_ptr;
        *dest_y_ptr=-*dest_y_ptr;
      }
      *dest_x_ptr -= gd_lcc2_spec->x_offset;
      *dest_y_ptr -= gd_lcc2_spec->y_offset;
      break; /* end GD to LCC2 */

      case GD_M:    /***** BEGIN GD TO M *****/

      gd_m_spec=&constants->specific_constants.gd_m;
      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_M_2D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_2d.u.m_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_2d.u.m_coord.y;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_M_3D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_3d.u.m_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_3d.u.m_coord.y;

         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_elv -= delta_h;
            break;

            case G_TO_E:
            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_elv += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.m_coord.z =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_elv
         );
      }

      source_lat *= SE_RADIANS_PER_DEGREE;
      source_lon *= SE_RADIANS_PER_DEGREE;

      sl = sin(source_lat);

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
         absl = fabs(sl);

         if (absl <= (35.5*SE_RADIANS_PER_DEGREE))
            p = gd_m_spec->CR11 +
                (gd_m_spec->CR12 * absl + gd_m_spec->CR13)
              / (gd_m_spec->CR14 + absl * (gd_m_spec->CR15 + absl));
         else
            p = gd_m_spec->CR21 +
                (gd_m_spec->CR22 * absl + gd_m_spec->CR23)
              / (gd_m_spec->CR24 + absl * (gd_m_spec->CR25 + absl));

         if (sl < 0)
            p = 1/p;

         p = p * sqrt((1 + sl)/(1 - sl));

      }
      else /* compute spherical values */
         p = sqrt((1 + sl)/(1 - sl));

      source_lon = source_lon - gd_m_spec->longitude_origin;

      /* fix quadrant on longitude */
      SE_FIX_LONGITUDE(source_lon);

      /* COMPUTE NORTHING AND EASTING */

      *dest_x_ptr = e_constants->A * source_lon * gd_m_spec->scale;
      *dest_x_ptr -= gd_m_spec->x_offset;

      *dest_y_ptr = e_constants->A * log(p) * gd_m_spec->scale;
      *dest_y_ptr -= gd_m_spec->y_offset;
      break; /* end GD to M */

      case GD_TM:

      gd_tm_spec=&constants->specific_constants.gd_tm;
      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_TM_2D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_2d.u.tm_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_2d.u.tm_coord.y;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_TM_3D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_3d.u.tm_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_3d.u.tm_coord.y;

         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_elv -= delta_h;
            break;

            case G_TO_E:
            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_elv += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.tm_coord.z =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_elv
         );
      }

      temp = fabs(source_lon) - fabs(gd_tm_spec->longitude_origin*SE_DEGREES_PER_RADIAN);
      if (temp > SE_TM_LONGITUDE_RANGE)
      {

         status = SE_COORD_EXTENDED_SOURCE_COORD;

         if(temp > SE_TM_EXTENDED_LONGITUDE_RANGE)
         {
            #ifdef CONV_DEBUG

              fprintf(f_ptr, "[SEDRIS CONVERSIONS] GD to TM error: "       \
                      "Source longitude out of bounds.  Longitude must be "\
                      "within +/- 3.5 degrees of longitude origin\n"       \
                      "Source latitude: %e  Source longitude: %e "         \
                      "Longitude Origin: %e\n",                            \
                      source_lat, source_lon,                              \
                      gd_tm_spec->longitude_origin*SE_DEGREES_PER_RADIAN);

            #endif

            return SE_COORD_INVALID_SOURCE_COORD;
         }
      }
      source_lat *= SE_RADIANS_PER_DEGREE;
      source_lon *= SE_RADIANS_PER_DEGREE;

      slat  = sin(source_lat);
      clat  = cos(source_lat);
      tx    = slat / clat;
      tn2 = tx * tx;
      slat2 = slat * slat;

      dlon = source_lon - gd_tm_spec->longitude_origin;
      al = (dlon) * clat;
      al2 = al * al;

      poly1 = 1.0 - tn2;
      poly2 = 5.0 + tn2 * (tn2 - 18.0);

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
         /* USE IN-LINE SQUARE ROOT */

         temp = e_constants->Eps25*slat2;
         temp = 0.5 - temp;
         temp = temp + (temp - .25 )/temp;
         rn   = (e_constants->A)/temp;
         cee = e_constants->Epps2 * clat * clat;

         if(constants->compute_com == SE_TRUE)
         {
            SE_FLOAT64 ds,p1,p2,p3,com,bot;

            ds = dlon * slat;

            p1 = (1.0 + tn2 + cee * (3.0 + 2.0 * cee)) / 3.0;
            p2 = cee * (33.0 - 60.0 * tn2 + cee * (11.0 -24.0 *tn2));
            p2 = cee * (15.0 + tn2 * (cee * (35.0 - 40.0 * tn2 + p2)));
            p2 = (2.0 + tn2 * (4.0 + 2.0 * tn2) + p2) / 15.0;
            p3 = 17.0 * pow(1.0 + tn2,3) / 315.0;

            com = ds * ( 1.0 + al2 * (p1 + al2 * (p2 + al2 * p3)));

            if(com == 0.0)
            {
               constants->sin_gamma = 0.0;
               constants->cos_gamma = 1.0;
            }
            else
            {
               bot = sqrt(1.0 + com * com);
               constants->sin_gamma = com / bot;
               constants->cos_gamma = 1.0  / bot;
            }
         } /* end compute sin and cos of com */

         poly1=poly1 + cee;
         poly2=poly2  + cee * (14.0 - tn2 * 58.0);

         sm = slat * clat * (gd_tm_spec->poly2b + slat2 *
             (gd_tm_spec->poly3b + slat2 *
             (gd_tm_spec->poly4b + slat2 * gd_tm_spec->poly5b)));
         sm = e_constants->A * (gd_tm_spec->poly1b * source_lat + sm);
      }
      else /* compute spherical values */
      {
         rn=e_constants->A;
         sm = e_constants->A * source_lat;

         if(constants->compute_com == SE_TRUE)
         {
            SE_FLOAT64 ds,p1,p2,p3,com,bot;

            ds = dlon * slat;

            p1 = (1.0 + tn2)/ 3.0;
            p2 = (2.0 + tn2 * (4.0 + 2.0 * tn2)) / 15.0;
            p3 = 17.0 * pow(1.0 + tn2,3) / 315.0;

            com = ds * ( 1.0 + al2 * (p1 + al2 * (p2 + al2 * p3)));

            if(com == 0.0)
            {
               constants->sin_gamma = 0.0;
               constants->cos_gamma = 1.0;
            }
            else
            {
               bot = sqrt(1.0 + com * com);
               constants->sin_gamma = com / bot;
               constants->cos_gamma = 1.0  / bot;
            }
         } /* end compute sin and cos of com */
      }

      /* COMPUTE EASTING */
      *dest_x_ptr = gd_tm_spec->CScale * rn * al
                    * (1.0 + al2 *(gd_tm_spec->Cdb6
                    * poly1 +gd_tm_spec->Cdb120 * al2 * poly2));                                                                                     \
      *dest_x_ptr -= gd_tm_spec->x_offset;

      /* COMPUTE NORTHING */
      poly1 = 5.0 - tn2;
      poly2 = 61.0 + tn2 * (tn2 - 58.0);

      if (e_constants->Eps!=0) /* ellipsoid calcs */
      {
        poly1 = poly1+cee * (cee * 4.0 + 9.0);
        poly2 = poly2  + cee * (270.0 - tn2 * 330.0);
      }

      *dest_y_ptr = gd_tm_spec->CScale *
                    (sm - gd_tm_spec->smz + rn * tx * al2 *
                    (0.5 + al2 * (gd_tm_spec->Cdb24 * poly1 +
                                  gd_tm_spec->Cdb720 * al2 * poly2)));
      *dest_y_ptr -= gd_tm_spec->y_offset;

      break; /* end GD to TM */

      case GD_UTM:

      gd_utm_spec=&constants->specific_constants.gd_utm;

      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_UTM_2D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_2d.u.utm_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_2d.u.utm_coord.y;

         if (source_lat < 0)
            dest_coord_ptr->u.coord_2d.u.utm_coord.hemisphere=SE_SOUTHERN_HEMISPHERE;
         else
            dest_coord_ptr->u.coord_2d.u.utm_coord.hemisphere=SE_NORTHERN_HEMISPHERE;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_UTM_3D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_3d.u.utm_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_3d.u.utm_coord.y;

         if (source_lat < 0)
            dest_coord_ptr->u.coord_3d.u.utm_coord.hemisphere=SE_SOUTHERN_HEMISPHERE;
         else
            dest_coord_ptr->u.coord_3d.u.utm_coord.hemisphere=SE_NORTHERN_HEMISPHERE;

         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_elv -= delta_h;
            break;

            case G_TO_E:
            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_elv += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.utm_coord.z =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_elv
         );
      }


      /* compute what the zone should be */

      zone = (SE_UINT8)(31 + source_lon/6);
      if (zone <=0)
         zone = 1;
      else if (zone >60)
         zone=60;

      /* compute central meridian */

      axlon0 = (SE_FLOAT64)(zone * 6 - 183);


      if(zone == gd_utm_spec->zone)
         status = SE_COORD_SUCCESS;
      else
      {
         if((fabs(axlon0) - fabs(source_lon)) > SE_TM_EXTENDED_LONGITUDE_RANGE)
            return SE_COORD_INVALID_SOURCE_COORD;
         else
            status = SE_COORD_EXTENDED_DEST_COORD;

      }

      axlon0 *= SE_RADIANS_PER_DEGREE;
      source_lat *= SE_RADIANS_PER_DEGREE;
      source_lon *= SE_RADIANS_PER_DEGREE;

      slat  = sin(source_lat);
      clat  = cos(source_lat);
      tx    = slat / clat;
      slat2 = slat * slat;

      if (e_constants->Eps!=0) /* compute ellipsoidal values */
      {
        /* USE IN-LINE SQUARE ROOT */
        temp = e_constants->Eps25*slat2;
        temp = 0.5 - temp;
        temp = temp + (temp - .25 )/temp;
        rn   = (e_constants->A)/temp;
      }
      else /* compute spherical values */
        rn = e_constants->A;

      /* COMPUTE UTM COORDINATES */

      dlon = source_lon -axlon0;
      al = (dlon) * clat;
      al2 = al * al;

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
         sm = slat * clat * (gd_utm_spec->poly2b + slat2 *
             (gd_utm_spec->poly3b + slat2 *
             (gd_utm_spec->poly4b + slat2 * gd_utm_spec->poly5b)));

         sm = e_constants->A * (gd_utm_spec->poly1b * source_lat + sm);
      }
      else /* compute spherical values */
         sm = e_constants->A * source_lat;

      tn2 = tx * tx;

      poly1 = 1.0 - tn2;
      poly2 = 5.0 + tn2 * (tn2 - 18.0);

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
         cee = e_constants->Epps2 * clat * clat;
         poly1=poly1 + cee;
         poly2=poly2  + cee * (14.0 - tn2 * 58.0);

         if(constants->compute_com == SE_TRUE)
         {
            SE_FLOAT64 ds,p1,p2,p3,com,bot;

            ds = dlon * slat;

            p1 = (1.0 + tn2 + cee * (3.0 + 2.0 * cee)) / 3.0;
            p2 = cee * (33.0 - 60.0 * tn2 + cee * (11.0 -24.0 *tn2));
            p2 = cee * (15.0 + tn2 * (cee * (35.0 - 40.0 * tn2 + p2)));
            p2 = (2.0 + tn2 * (4.0 + 2.0 * tn2) + p2) / 15.0;
            p3 = 17.0 * pow(1.0 + tn2,3) / 315.0;

            com = ds * ( 1.0 + al2 * (p1 + al2 * (p2 + al2 * p3)));

            if(com == 0.0)
            {
               constants->sin_gamma = 0.0;
               constants->cos_gamma = 1.0;
            }
            else
            {
               bot = sqrt(1.0 + com * com);
               constants->sin_gamma = com / bot;
               constants->cos_gamma = 1.0  / bot;
            }
         } /* end compute sin and cos of com */
      }
      else /* see if we need to compute com for spheres */
      {
         if(constants->compute_com == SE_TRUE)
         {
            SE_FLOAT64 ds,p1,p2,p3,com,bot;

            ds = dlon * slat;

            p1 = (1.0 + tn2)/ 3.0;
            p2 = (2.0 + tn2 * (4.0 + 2.0 * tn2)) / 15.0;
            p3 = 17.0 * pow(1.0 + tn2,3) / 315.0;

            com = ds * ( 1.0 + al2 * (p1 + al2 * (p2 + al2 * p3)));

            if(com == 0.0)
            {
               constants->sin_gamma = 0.0;
               constants->cos_gamma = 1.0;
            }
            else
            {
               bot = sqrt(1.0 + com * com);
               constants->sin_gamma = com / bot;
               constants->cos_gamma = 1.0  / bot;
            }
         } /* end compute sin and cos of com */
      }



      /* COMPUTE EASTING */
      *dest_x_ptr = gd_utm_spec->scale * rn * al
                     * (1.0 + al2 *(gd_utm_spec->Cdb6
                     * poly1 +gd_utm_spec->Cdb120 * al2 * poly2));

      *dest_x_ptr += 5.0E5 - gd_utm_spec->x_offset;

      /* COMPUTE NORTHING */
      poly1 = 5.0 - tn2;
      poly2 = 61.0 + tn2 * (tn2 - 58.0);

      if (e_constants->Eps!=0) /* ellipsoid calcs */
      {
        poly1 = poly1+cee * (cee * 4.0 + 9.0);
        poly2 = poly2  + cee * (270.0 - tn2 * 330.0);
      }

      *dest_y_ptr = gd_utm_spec->scale * (sm + rn * tx * al2 *
                 (0.5 + al2 * (gd_utm_spec->Cdb24 * poly1 +
                               gd_utm_spec->Cdb720 * al2 * poly2)));

      if (source_lat < 0.0)
        *dest_y_ptr += 1.0E7 - gd_utm_spec->y_offset;
      else
        *dest_y_ptr -= gd_utm_spec->y_offset;

      break; /* end GD to UTM */

      case GD_PS:    /***** BEGIN GD TO PS *****/

      gd_ps_spec=&constants->specific_constants.gd_ps;

      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_PS_2D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_2d.u.ps_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_2d.u.ps_coord.y;

      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_PS_3D_SRF;
         dest_x_ptr=&dest_coord_ptr->u.coord_3d.u.ps_coord.x;
         dest_y_ptr=&dest_coord_ptr->u.coord_3d.u.ps_coord.y;

         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_elv -= delta_h;
            break;

            case G_TO_E:

               wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_elv += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.ps_coord.z =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_elv
         );
      }

      source_lat *= SE_RADIANS_PER_DEGREE;
      source_lon *= SE_RADIANS_PER_DEGREE;

      abslat = fabs(source_lat);
      sl     = sin(abslat);

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
         if (abslat <= (35.5*SE_RADIANS_PER_DEGREE))
         {
            p = gd_ps_spec->CR11 +
                  (gd_ps_spec->CR12 * sl + gd_ps_spec->CR13)
                / (gd_ps_spec->CR14 + sl * (gd_ps_spec->CR15 + sl));
         }
         else
         {
            p = gd_ps_spec->CR21 +
                  (gd_ps_spec->CR22 * sl + gd_ps_spec->CR23)
                / (gd_ps_spec->CR24 + sl * (gd_ps_spec->CR25 + sl));
         }
      }
      else /* compute spherical values */
         p=1;

      if (source_lat > -1.553343)
         t = sqrt((1.0-sl)/(1.0+sl));

      else
         t = tan(SE_PI/4.0 - .5 * source_lat);

      r = gd_ps_spec->xk * t / p;

      slon = sin(source_lon);
      clon = cos(source_lon);

      /* COMPUTE NORTHING AND EASTING */

      *dest_x_ptr = r * slon;

      if (gd_ps_spec->pole == SE_NORTH_POLE)
        *dest_y_ptr = -r * clon;
      else \
        *dest_y_ptr = r * clon;
      /* end GD PS */
      break;


      case GD_GC:

      /* GC is 3D by definition */
      dest_coord_ptr->is_2D = SE_FALSE;
      dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_GC_3D_SRF;

      source_lat *= SE_RADIANS_PER_DEGREE;
      source_lon *= SE_RADIANS_PER_DEGREE;

      /*
      * Convert elevation to proper units
      * (since GC is the destination, the elevation will be converted
      *  into meters, since GC is in meters by definition)
      */
      source_elv = SE_ConvertDistance
                (
                    constants->source_units,
                    SRM_METERS,
                    source_elv
                );

      /* gd to gc, does not use height conversion, per gc definition
       * however, this call may be in the middle of a chain, say
       * GD to LTP, which may specifiy a height conversion, so we
       * do it here, if needed
       */

      switch(constants->height_convert_type)
      {
         case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

         source_elv -= delta_h;
         break;

         case G_TO_E:
            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
         source_elv += delta_h;
         break;

         default:

         break;
      }

      slat  = sin(source_lat);
      slat2 = slat * slat;
      clat  = cos(source_lat);

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
        temp   = e_constants->Eps25*slat2;
        temp   = 0.5 - temp;
        temp   = temp + (temp - .25 )/temp;
        rn     = (e_constants->A)/temp;
        Rnc2a2 = rn * e_constants->C2 / e_constants->A2;
      }
      else /* compute spherical values */
      {
        rn     = e_constants->A;
        Rnc2a2 = rn;
      }
      Rnhc = (rn + source_elv) * clat;

      dest_coord_ptr->u.coord_3d.u.gc_coord.x = Rnhc * cos(source_lon);
      dest_coord_ptr->u.coord_3d.u.gc_coord.y = Rnhc * sin(source_lon);
      dest_coord_ptr->u.coord_3d.u.gc_coord.z =(Rnc2a2 + source_elv) * slat;
      break;

      case EC_GD:

      /* source_x,source_y,source_z already assigned */

      ec_spec=&constants->specific_constants.ec;

      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_GD_2D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_latitude;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_latitude;
      } /* end else 3d */


      *dest_lat_ptr = ec_spec->latitude_origin +
                    source_y / ec_spec->scaled_radius;
      *dest_lon_ptr = ec_spec->longitude_origin + source_x /
                    (ec_spec->scaled_radius *
                     cos(ec_spec->latitude_origin));

      SE_FIX_LONGITUDE(*dest_lon_ptr);

      *dest_lat_ptr *= SE_DEGREES_PER_RADIAN;
      *dest_lon_ptr *= SE_DEGREES_PER_RADIAN;

      if(constants->dest_srf_is_2D == SE_FALSE)
      {
         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_z -= delta_h;
            break;

            case G_TO_E:

               wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_z += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.gd_coord.elevation =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_z
         );
      }
      break; /* end EC to GD */

      case M_GD:    /***** BEGIN M TO GD *****/

      m_gd_spec=&constants->specific_constants.m_gd;

      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_GD_2D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_latitude;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_latitude;
      } /* end else 3d */

      source_x /= m_gd_spec->scale;
      source_y /= m_gd_spec->scale;

      /* COMPUTE LONGITUDE */
      *dest_lon_ptr = source_x / e_constants->A +
                    m_gd_spec->longitude_origin;

      tanz2 = exp(- source_y/e_constants->A);
      z = 2 * atan(tanz2);

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
        /* Compute Latitude */

        /*
         * Use identity to get SIN and COS of xki without trig
         * functions
         */
        t2  = tanz2 * tanz2;
        den = 1 + t2;
        cki = 2 * tanz2/den;
        ski=(1-t2)/den;

        /* Use inverse power series for latitude */
        sk2 = ski * ski;

        xki=SE_PI/2 - z;

        *dest_lat_ptr = xki + ski * cki * (m_gd_spec->b1 + sk2 *
                        (m_gd_spec->b2 + sk2 *
                        (m_gd_spec->b3 + sk2 * m_gd_spec->b4)));
      }
      else /* compute spherical values */
        *dest_lat_ptr = SE_PI/2 - z;

      SE_FIX_LONGITUDE(*dest_lon_ptr);

      /* convert to degrees */
      *dest_lat_ptr *= SE_DEGREES_PER_RADIAN;
      *dest_lon_ptr *= SE_DEGREES_PER_RADIAN;

      if(constants->dest_srf_is_2D == SE_FALSE)
      {
         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_z -= delta_h;
            break;

            case G_TO_E:
              wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_z += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.gd_coord.elevation =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_z
         );
      }

      break; /* end M to GD */

      case LCC2_GD:

      lcc2_gd_spec=&constants->specific_constants.lcc2_gd;
      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_GD_2D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_latitude;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_latitude;
      } /* end else 3d */

      /*
      * LATITUDE FIRST USING INVERSE POWER SERIES
      */
      if (lcc2_gd_spec->xl < 0)
      {
        source_x = -source_x;
        source_y = -source_y;
      }

      drz = lcc2_gd_spec->rz - source_y;

      /* Get r2 */
      r2    = (source_x * source_x) + (drz * drz);

      tanz2 = pow(r2/lcc2_gd_spec->rz2, lcc2_gd_spec->ex)/
                lcc2_gd_spec->p1;

      if (e_constants->Eps != 0.0) /* compute ellipsoidal values */
      {
       /*
        * Use Identity to get sin & cos without trig functions
        */
        t2  = tanz2 * tanz2;
        den = 1 + t2;
        cki = 2 * tanz2 / den;
        ski = (1-t2) / den;
       /* Sin and Cos of xki now done */

       /* Use inverse power series for latitude */
        sk2 = ski * ski;
        z   = 2 * atan(tanz2);
        xki = SE_PI/2 - z;

        *dest_lat_ptr = xki +
                        ski * cki * (lcc2_gd_spec->b1 + sk2 *
                        (lcc2_gd_spec->b2 + sk2 *
                        (lcc2_gd_spec->b3 + sk2 *
                         lcc2_gd_spec->b4)));
      }
      else /* compute spherical values */
      {
        *dest_lat_ptr = SE_PI/2 - 2* atan(tanz2);
      }

      thet = atan2(source_x, drz);

      *dest_lon_ptr =
        thet/lcc2_gd_spec->xl + lcc2_gd_spec->xlonz;

      SE_FIX_LONGITUDE(*dest_lon_ptr);

      *dest_lat_ptr  *= SE_DEGREES_PER_RADIAN;
      *dest_lon_ptr *= SE_DEGREES_PER_RADIAN;

      if(constants->compute_com == SE_TRUE)
      {
         arg  = lcc2_gd_spec->xl * (*dest_lon_ptr - lcc2_gd_spec->xlonz);

         constants->sin_gamma = sin(arg);
         constants->cos_gamma = cos(arg);
      }

      if(constants->dest_srf_is_2D == SE_FALSE)
      {
         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_z -= delta_h;
            break;

            case G_TO_E:
            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_z += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.gd_coord.elevation =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_z
         );
      }
      break;

      case LCC1_GD:

      lcc1_gd_spec = &constants->specific_constants.lcc1_gd;

      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_GD_2D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_latitude;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_latitude;
      } /* end else 3d */

      if (lcc1_gd_spec->xl<0)
      {
         source_x = -source_x;
         source_y = -source_y;
      }

      drz  = lcc1_gd_spec->rho_origin - source_y;
      thet = atan2(source_x, drz);
      *dest_lon_ptr = thet / lcc1_gd_spec->xl +
                      lcc1_gd_spec->xlonz;

      /* get latitude using inverse power series */

      /* Get r squared  */
      r2 = source_x * source_x + drz * drz;
      tanz2 = pow(r2 / lcc1_gd_spec->rz2, lcc1_gd_spec->ex) /
               lcc1_gd_spec->pz;

      if (e_constants->Eps!=0) /* do ellipsoid calculations */
      {
         /* Use Identity to get sin & cos without trig functions */
         t2  = tanz2 * tanz2;
         den = 1+t2;
         cki = 2 * tanz2 / den;
         ski = (1-t2) / den;

         sk2 = ski * ski;
         z   = 2 * atan(tanz2);
         xki = SE_PI/2 - z;

         *dest_lat_ptr = xki + ski * cki *
            (lcc1_gd_spec->b1 + sk2 * (lcc1_gd_spec->b2 + sk2   *
            (lcc1_gd_spec->b3 + sk2 * lcc1_gd_spec->b4)));
      }
      else /* do spherical calculations */
         *dest_lat_ptr = (SE_PI/2) - 2  *atan(tanz2);

      SE_FIX_LONGITUDE(*dest_lon_ptr);

      if(constants->compute_com == SE_TRUE)
      {
         arg  = lcc1_gd_spec->xl * (*dest_lon_ptr - lcc1_gd_spec->xlonz);

         constants->sin_gamma = sin(arg);
         constants->cos_gamma = cos(arg);
      }

      *dest_lat_ptr  *= SE_DEGREES_PER_RADIAN;
      *dest_lon_ptr  *= SE_DEGREES_PER_RADIAN;

      if(constants->dest_srf_is_2D == SE_FALSE)
      {
         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_z -= delta_h;
            break;

            case G_TO_E:
            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_z += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.gd_coord.elevation =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_z
         );
      }

      break;

      case TM_GD:

      tm_gd_spec=&constants->specific_constants.tm_gd;

      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_GD_2D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_latitude;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_latitude;
      } /* end else 3d */

      u = (tm_gd_spec->smz + source_y/tm_gd_spec->CScale)
        / tm_gd_spec->conap;

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
        su = sin(u);
        cu = cos(u);
        su2 = su * su;

        temp = tm_gd_spec->polx2b + su2 *
              (tm_gd_spec->polx3b + su2 *
              (tm_gd_spec->polx4b + su2 * tm_gd_spec->polx5b));

        phi1 = u + su * cu * temp;
      }
      else /* compute spherical values */
         phi1=u;

      /*
      * COMPUTE VARIABLE COEFFICIENTS FOR FINAL RESULT
      * COMPUTE THE VARIABLE COEFFICIENTS OF THE LAT AND LON
      * EXPANSIONS
      */
      sp = sin(phi1);
      cp = cos(phi1);
      tp = sp / cp;
      tp2 = tp * tp;
      sp2 = sp * sp;
      cp2 = cp * cp;

      b3 = 1.0 + tp2 + tp2;
      b4 = 5 + tp2 * 3;
      b5 = 5 + tp2 * (tp2 * 24.0 + 28.0);
      b6  = 61 + tp2 * (tp2 * 45.0 + 90.0);

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
         eta2 = e_constants->Epps2 * cp2;
         top = .25-(sp2*(e_constants->Eps2 / 4));

         /* inline sq root*/

         temp = e_constants->Eps25*sp2;
         temp =0.5 - temp;
         temp = temp + (temp - .25 )/temp;
         rn=(e_constants->A)/temp;
         d1 = source_x / (tm_gd_spec->CScale *rn);
         d2 = d1 * d1;

         if(constants->compute_com == SE_TRUE)
         {
            SE_FLOAT64 p1,p2,p3,com,bot;

            p1 = 1.0 - eta2 * (1.0 + 2.0 * eta2);
            p1 = p1 / 3.0;
            p2 = eta2 * (11.0 - 24.0 * tp2);
            p2 = eta2 * (20.0 - 27.0 * tp2 +p2);
            p2 = eta2 * (9.0 + 3.0 * tp2 + p2);
            p2 = 2.0 + eta2 * (2.0 + 6.0 * tp2 + p2);
            p2 = p2 / 15.0;
            p3 = 17.0 / 315.0;

            com = tp * d1 * (1.0 - d2 * (p1 - d2 * (p2 + d2 * p3 )));

            if(com == 0.0)
            {
               constants->sin_gamma = 0.0;
               constants->cos_gamma = 1.0;
            }
            else
            {
              bot = sqrt(1.0 + com * com);
              constants->sin_gamma = com / bot;
              constants->cos_gamma = 1.0 / bot;
            }
         }

         b3 = b3 + eta2;
         b4 = b4 + tp2 *(- 9 * eta2) + eta2 * ( 1 - 4 * eta2);
         b5 = b5 + eta2 * (tp2 * 8.0 + 6.0);
         b6x = 46 - 3.0 * eta2 + tp2 * (-252.0 - tp2 * 90.0);
         b6x = eta2 * (b6x + eta2 * tp2 * (tp2 * 225.0 - 66.0));
         b6  = b6x + b6;
      }
      else /* compute spherical values */
      {
        rn  = e_constants->A;
        top = .25;
        d1 = source_x / (tm_gd_spec->CScale *rn);
        d2 = d1 * d1;

        if(constants->compute_com == SE_TRUE)
        {
            SE_FLOAT64 p1,p2,p3,com,bot;

            p1 = 1.0/3.0;
            p2 = 2.0/15.0;
            p3 = 17.0 / 350.0;

            com = tp * d1 * (1.0 - d2 * (p1 - d2 * (p2 + d2 * p3 )));

            if(com == 0.0)
            {
               constants->sin_gamma = 0.0;
               constants->cos_gamma = 1.0;
            }
            else
            {
              bot = sqrt(1.0 + com * com);
              constants->sin_gamma = com / bot;
              constants->cos_gamma = 1.0 / bot;
            }
         }
      }

      *dest_lat_ptr = phi1 - tp * top * (d2 * (tm_gd_spec->Con2 +
                      d2 * ((-tm_gd_spec->Con24) * b4 +
                      d2 * tm_gd_spec->Con720 * b6)));

      *dest_lon_ptr = tm_gd_spec->longitude_origin +
                                 d1 * (1.0 + d2 * (-tm_gd_spec->Con6 * b3
                               + d2 * tm_gd_spec->Con120 * b5))
                               / cp;

      /* TRANSVERSE MERCATOR COMPUTATIONS DONE */

      *dest_lat_ptr  *= SE_DEGREES_PER_RADIAN;
      *dest_lon_ptr  *= SE_DEGREES_PER_RADIAN;


      /* test if coord is within range */

      temp = fabs(*dest_lon_ptr) -
             fabs(tm_gd_spec->longitude_origin * SE_DEGREES_PER_RADIAN);

      temp = fabs(temp);
      if(temp > SE_TM_LONGITUDE_RANGE)
      {
         if(temp > SE_TM_EXTENDED_LONGITUDE_RANGE)
            return SE_COORD_INVALID_SOURCE_COORD;
         else
            status = SE_COORD_EXTENDED_DEST_COORD;
      }


      if(constants->dest_srf_is_2D == SE_FALSE)
      {
         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_z -= delta_h;
            break;

            case G_TO_E:

               wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_z += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.gd_coord.elevation =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_z
         );
      }
      /* end tm to gd  */
      break;

      case UTM_GD:

      utm_gd_spec=&constants->specific_constants.utm_gd;

      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_GD_2D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_latitude;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_latitude;
      } /* end else 3d */

      /*
      * set source_x to (utm_coord.x + x_offset - 500000.0)/CScale
      */
      source_x = (source_x - 500000.0)/utm_gd_spec->CScale;

      if (hemisphere == SE_NORTHERN_HEMISPHERE)
       /*
        * source_y already is set to (utm_coord.y + y_offset)
        *
        * set source_y to (utm_coord.y + y_offset)/CScale
        */
        source_y /= utm_gd_spec->CScale;
      else
       /*
        * source_y already is set to (utm_coord.y + y_offset)
        *
        * set source_y to (utm_coord.y + y_offset - 1.0E7)/CScale
        */
        source_y = (source_y - 1.0E7)/utm_gd_spec->CScale;


      u = source_y / utm_gd_spec->conap;

      xlon0= (6.0 * (SE_FLOAT64)utm_gd_spec->zone - 183.0)
           * SE_RADIANS_PER_DEGREE;

      /* TEST U TO SEE IF AT POLES */
      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
        su = sin(u);
        cu = cos(u);
        su2 = su * su;

       /*
        * THE SNYDER FORMULA FOR PHI1 IS OF THE FORM
        * PHI1=U+POLY2A*SIN(2U)+POLY3A*SIN(4U)+POLY4ASIN(6U)+...
        * BY USING MULTIPLE ANGLE TRIGONOMETRIC IDENTITIES AND
        * APPROPRIATE FACTORING, JUST THE SINE AND COSINE ARE REQUIRED
        * NOW READY TO GET PHI1
        */
        temp = utm_gd_spec->polx2b + su2 *
              (utm_gd_spec->polx3b + su2 *
              (utm_gd_spec->polx4b + su2 * utm_gd_spec->polx5b));

        phi1 = u + su * cu * temp;
      }
      else /* compute spherical values */
        phi1=u;

      /*
      * COMPUTE VARIABLE COEFFICIENTS FOR FINAL RESULT
      * COMPUTE THE VARIABLE COEFFICIENTS OF THE LAT AND LON
      * EXPANSIONS
      */
      sp = sin(phi1);
      cp = cos(phi1);
      tp = sp / cp;
      tp2 = tp * tp;
      sp2 = sp * sp;
      cp2 = cp * cp;

      b3 = 1.0 + tp2 + tp2;

      b4 = 5 + tp2 * 3;

      b5 = 5 + tp2 * (tp2 * 24.0 + 28.0);

      b6 = 61 +tp2 * (tp2 * 45.0 + 90.0);

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
        eta2 = e_constants->Epps2 * cp2;
        top = .25-(sp2*(e_constants->Eps2 / 4));

       /* inline sq root*/

        temp = e_constants->Eps25*sp2;
        temp =0.5 - temp;
        temp = temp + (temp - .25 )/temp;

        rn=(e_constants->A)/temp;

        b3 = b3 + eta2;
        b4 = b4 + tp2 *(- 9 * eta2) + eta2 * ( 1 - 4 * eta2);
        b5 = b5 + eta2 * (eta2 * 8.0 + 6.0);
        b6x = 46 - 3.0 * eta2 + tp2 * (-252.0 - tp2 * 90.0);
        b6x = eta2 * (b6x + eta2 * tp2 * (tp2 * 225.0 - 66.0));
        b6 = b6x + b6;
      }
      else /* compute spherical values */
      {
        rn=e_constants->A;
        top=.25;
      }

      d1 = source_x / rn;
      d2 = d1 * d1;

      if(constants->compute_com == SE_TRUE)
      {
         SE_FLOAT64 p1,p2,p3,com,bot;

         if(e_constants->Eps!=0.0) /* ellipsoid */
         {
            p1 = 1.0 - eta2 * (1.0 + 2.0 * eta2);
            p1 = p1 / 3.0;
            p2 = eta2 * (11.0 - 24.0 * tp2);
            p2 = eta2 * (20.0 - 27.0 * tp2 +p2);
            p2 = eta2 * (9.0 + 3.0 * tp2 + p2);
            p2 = 2.0 + eta2 * (2.0 + 6.0 * tp2 + p2);
            p2 = p2 / 15.0;
            p3 = 17.0 / 315.0;

            com = tp * d1 * (1.0 - d2 * (p1 - d2 * (p2 + d2 * p3 )));
         }
         else
         {
            p1 = 1.0/3.0;
            p2 = 2.0/15.0;
            p3 = 17.0 / 350.0;

            com = tp * d1 * (1.0 - d2 * (p1 - d2 * (p2 + d2 * p3 )));
         }

         if (com == 0.0)
         {
            constants->sin_gamma = 0.0;
            constants->cos_gamma = 1.0;
         }
         else
         {
           bot = sqrt(1.0 + com * com);
           constants->sin_gamma = com / bot;
           constants->cos_gamma = 1.0 / bot;
         }
      }
      *dest_lat_ptr = phi1 - tp * top * (d2 * (utm_gd_spec->Con2 +
                      d2 * ((-utm_gd_spec->Con24) * b4 +
                      d2 * utm_gd_spec->Con720 * b6)));

      *dest_lon_ptr = xlon0 + d1 * (1.0 + d2 * (-utm_gd_spec->Con6 * b3
                        + d2 * utm_gd_spec->Con120 * b5))
                         / cp;

      *dest_lat_ptr  *= SE_DEGREES_PER_RADIAN;
      *dest_lon_ptr *= SE_DEGREES_PER_RADIAN;


      /* test if coord is within range */

      /* compute the zone according to the new longitude */

      zone = (SE_UINT8)(31 + *dest_lon_ptr/6);
      if (zone <=0)
         zone = 1;
      else if (zone >60)
         zone=60;

      if(zone != utm_gd_spec->zone)
      {

         if((fabs(xlon0) - fabs(*dest_lon_ptr)) > SE_TM_EXTENDED_LONGITUDE_RANGE)
            return SE_COORD_INVALID_SOURCE_COORD;
         else
            status = SE_COORD_EXTENDED_DEST_COORD;
      }

      if(constants->dest_srf_is_2D == SE_FALSE)
      {
         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_z -= delta_h;
            break;

            case G_TO_E:
            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_z += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.gd_coord.elevation =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_z
         );
      }
      break;

      case GC_GD:   /**************  GC to GD ***************/
      {
         SE_BOOLEAN done_by_special_case = SE_FALSE;

         gc_gd_spec=&constants->specific_constants.gc_gd;

         if(constants->dest_srf_is_2D == SE_TRUE)
         {
            dest_coord_ptr->is_2D = SE_TRUE;
            dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_GD_2D_SRF;
            dest_lon_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_longitude;
            dest_lat_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_latitude;
            dest_elv_ptr = &dummy;
         }
         else
         {
            dest_coord_ptr->is_2D = SE_FALSE;
            dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
            dest_lon_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_longitude;
            dest_lat_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_latitude;
            dest_elv_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.elevation;
         } /* end else 3d */

         /* Check for special cases */
         if (source_x == 0.0)
         {
            if (source_y > 0.0) *dest_lon_ptr = SE_PI/2;
            else
            {
               if (source_y < 0.0) *dest_lon_ptr = -SE_PI/2;
               else /* y == 0 */
               {
                  if (source_z > 0)
                  {
                     *dest_lat_ptr = SE_PI/2;
                     *dest_lon_ptr = 0.0;
                     *dest_elv_ptr = source_z - e_constants->C;

                     /* Convert to proper units */
                     if (constants->dest_units == SRM_FEET)
                        *dest_elv_ptr *= SE_FEET_PER_METER;
                     #ifdef CONV_DEBUG
                        fprintf(f_ptr, "Warning %s: At North Pole, Longitude is not defined.\n",
                                conv_error_msg[SE_GC2GD_ERROR]);
                     #endif

                     done_by_special_case = SE_TRUE;

                  } /* end if z> 0 */
                  else if (source_z < 0.0)
                  {
                     *dest_lat_ptr = -SE_PI/2;
                     *dest_lon_ptr   =  0.0;
                     *dest_elv_ptr = - (source_z + e_constants->C);

                     /* Convert to proper units */
                     if (constants->dest_units == SRM_FEET)
                        *dest_elv_ptr *= SE_FEET_PER_METER;

                     #ifdef CONV_DEBUG

                     fprintf(f_ptr, "Warning %s: At South Pole, Longitude is not defined.\n",
                        conv_error_msg[SE_GC2GD_ERROR]);

                     #endif

                     done_by_special_case = SE_TRUE;
                     } /* end if z < 0 */
                     else /* at origin */
                     {
                        #ifdef CONV_DEBUG

                        fprintf(f_ptr,"%s: At origin.",conv_error_msg[SE_GC2GD_ERROR]);

                        #endif

                        return SE_COORD_INVALID_DEST_COORD;

                     } /* end else at origin */
               } /* end else y == 0 */
            } /* end else y < 0 */
         } /* end if x == 0 */

         /* end of special cases */

         if (!done_by_special_case)
         {
            w2=source_x * source_x + source_y * source_y;
            w=sqrt(w2);
            z2=source_z * source_z;

            if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
            {
               testu=w2 + gc_gd_spec->ARat2 * z2;
               testb=w2 + gc_gd_spec->BRat2 * z2;

               if ((testb > gc_gd_spec->BRat1) &&
                   (testu < gc_gd_spec->ARat1))
               {
                  /*
                  * POINT IS BETWEEN-10 KILOMETERS AND 50 KILOMETERS
                  */
                  BF= (gc_gd_spec->B1 +
                    (gc_gd_spec->B2 * w2 + gc_gd_spec->B3) /
                    (gc_gd_spec->B4 + w2 * gc_gd_spec->B5 + z2));

                   top = source_z * BF;
                   top2=top*top;

                   rr=top2+w2;

                   q=sqrt(rr);

                   s12=top2/rr;

                   temp = e_constants->Eps25*s12;
                   temp =0.5 - temp;
                   temp = temp + (temp - .25 )/temp;
                   rn=(e_constants->A)/temp;

                  /*
                   ******************************************************
                   * TEST FOR H NEAR POLE. if SIN(¯)**2 <= SIN(45.)**2
                   * THEN NOT NEAR A POLE.
                   ******************************************************
                   */
                   if (s12 <= .50)
                       *dest_elv_ptr = q-rn;
                   else
                       *dest_elv_ptr = q/BF + (gc_gd_spec->Eps21 * rn);
               }
               else /* Do Exact Solution  ************ */
               {
                   wp2=source_x * source_x + source_y * source_y;

                   zp2=source_z * source_z;

                  /*
                   * we are not allowed to be within 50 Kilometers of
                   * the center of the earth
                   */
                   if (wp2 + zp2 < 50000)
                   {
                     #ifdef CONV_DEBUG
                     fprintf(f_ptr, "[SEDRIS CONVERSIONS] Point within 50 "\
                          "kilometers of the earth's center.  Outside "\
                          "SEDRIS requirement range\n");
                     #endif

                     return SE_COORD_INVALID_DEST_COORD;
                   }
                   wp=sqrt(wp2);

                   cf=gc_gd_spec->C254 * zp2;

                   gee=wp2 - (gc_gd_spec->Eps21 * zp2)
                       - gc_gd_spec->CEEps2;

                   alpha=cf / (gee*gee);

                   cl=gc_gd_spec->CEE * wp2 * alpha / gee;

                   arg2=cl * (cl + 2.0);

                   s1=1.0 + cl + sqrt(arg2);

                   s=pow(s1,(1.0/3.0));

                   p=alpha / (3.0 * pow(( s + (1.0/s) + 1.0),2));

                   xarg= 1.0 + (gc_gd_spec->TwoCEE * p);

                   q=sqrt(xarg);

                   r2= -p * (2.0 * (1.0 - e_constants->Eps2) * zp2
                        / ( q * ( 1.0 + q) ) + wp2);
                   r1=(1.0 + (1.0 / q));
                   r2 /=e_constants->A2;

                  /*
                   * DUE TO PRECISION ERRORS THE ARGUMENT MAY BECOME
                   * NEGATIVE. IF SO SET THE ARGUMENT TO ZERO.
                   */
                   if (r1+r2 > 0.0)
                       ro = e_constants->A * sqrt( .50 * (r1+r2));
                   else
                     ro=0.0;

                   ro=ro - p * e_constants->Eps2 * wp / ( 1.0 + q);

                   roe = e_constants->Eps2 * ro;
                   arg = pow(( wp - roe),2) + zp2;
                   v   = sqrt(arg - e_constants->Eps2 * zp2);

                   zo=gc_gd_spec->C2DA * source_z / v;

                  *dest_elv_ptr = sqrt(arg)*(1.0 - gc_gd_spec->C2DA/v);

                  /* Convert to proper units */
                   if (constants->dest_units == SRM_FEET)
                       *dest_elv_ptr *= SE_FEET_PER_METER;

                   top=source_z+ gc_gd_spec->tem*zo;

                   w = wp;
               } /* end of Exact solution */
            }
            else /* compute spherical values */
            {
               top=source_z;
               *dest_elv_ptr=sqrt(w2 + z2)-e_constants->A;
            }

            /* Convert to proper units */
            if (constants->dest_units == SRM_FEET)
               *dest_elv_ptr *= SE_FEET_PER_METER;

            *dest_lat_ptr = atan(top / w);
            *dest_lon_ptr = atan2(source_y, source_x);
         }

           *dest_lat_ptr *= SE_DEGREES_PER_RADIAN;
           *dest_lon_ptr *= SE_DEGREES_PER_RADIAN;
      }
      break; /* end GC to GD */

      case PS_GD:    /***** BEGIN PS TO GD *****/

      ps_gd_spec=&constants->specific_constants.ps_gd;

      /* compute sin and cos of gamma/com          */

      if(constants->compute_com == SE_TRUE)
      {
         SE_FLOAT64 bot;

         bot = sqrt(source_x * source_x + source_y * source_y);

         constants->cos_gamma = source_y / bot;
         constants->sin_gamma = source_x / bot;

         if(ps_gd_spec->pole == SE_NORTH_POLE)
            constants->cos_gamma = -constants->cos_gamma;
      }

      if(constants->dest_srf_is_2D == SE_TRUE)
      {
         dest_coord_ptr->is_2D = SE_TRUE;
         dest_coord_ptr->u.coord_2d.spatial_reference_frame = SRM_GD_2D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_2d.u.gd_coord.geodetic_latitude;
      }
      else
      {
         dest_coord_ptr->is_2D = SE_FALSE;
         dest_coord_ptr->u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
         dest_lon_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_longitude;
         dest_lat_ptr = &dest_coord_ptr->u.coord_3d.u.gd_coord.geodetic_latitude;
      } /* end else 3d */

      /* COMPUTE LONGITUDE */
      if (ps_gd_spec->pole == SE_NORTH_POLE)
        *dest_lon_ptr = atan2(source_x,-source_y);
      else
        *dest_lon_ptr = atan2(source_x, source_y);

      /*
      * Compute Latitude
      */
      r2 = source_x * source_x + source_y * source_y;
      r = sqrt(r2);

      tanz2 = r / ps_gd_spec->xk;

      if (e_constants->Eps!=0.0) /* compute ellipsoidal values */
      {
        /*
         * Use identity to get SIN and COS of xki without trig
         * functions
         */
        t2  = tanz2 * tanz2;
        den = 1 + t2;
        cki = 2 * tanz2/den;
        ski=(1-t2)/den;

        /* Use inverse power series for latitude */
        sk2 = ski * ski;
        z = 2 * atan(tanz2);
        xki=SE_PI/2 - z;

        *dest_lat_ptr = xki + ski * cki * (ps_gd_spec->b1 + sk2 *
                        (ps_gd_spec->b2 + sk2 *
                        (ps_gd_spec->b3 + sk2 * ps_gd_spec->b4)));
      }
      else /* compute spherical values */
         *dest_lat_ptr = SE_PI/2 - 2*atan(tanz2);

      if (ps_gd_spec->pole != SE_NORTH_POLE)
        *dest_lat_ptr *= -1;

      *dest_lat_ptr *= SE_DEGREES_PER_RADIAN;
      *dest_lon_ptr *= SE_DEGREES_PER_RADIAN;

      if(constants->dest_srf_is_2D == SE_FALSE)
      {
         switch(constants->height_convert_type)
         {
            case E_TO_G:

            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;

            if(constants->source_units == SRM_FEET)
               delta_h = SE_ConvertDistance(SRM_METERS, SRM_FEET, delta_h);

            source_z -= delta_h;
            break;

            case G_TO_E:
            wgs84_location.geodetic_latitude = source_lat;
            wgs84_location.geodetic_longitude = source_lon;
            status = SRM_GetWGS84GeoidalSeparation(&wgs84_location, &delta_h);

            /* in case there was an error in SRM_GetWGS84GeoidalSeparation */

            if(status != SE_COORD_SUCCESS)
               return status;
            source_z += delta_h;
            break;

            default:

            break;
         }

         /*
         * Store z value and convert to proper units
         */
         dest_coord_ptr->u.coord_3d.u.gd_coord.elevation =
         SE_ConvertDistance
         (
            constants->source_units,
            constants->dest_units,
            source_z
         );
      }
      break; /* end PS to GD */

      default:

      #ifdef CONV_DEBUG

      fprintf(f_ptr, conv_error_msg[SE_INVALID_CONVERSION_ERROR],
             (int) constants->convert_type);

      #endif

      return SE_COORD_INVALID_SRF_PAIR;
   } /* end switch convert_type */

   /*
    * make sure the constants index is not more than the max
    */
   if(++c_index < SRM_CONSTANTS_DEPTH)
   {
      /*
       * now we can safely reference the constants pointer array
       * if there are more constants, then there are more conversions
       * to do
       */

      if (convert_params_ptr->constants[c_index] != NULL &&
          convert_params_ptr->constants[c_index]->
                              convert_type != NO_CONVERSION)
      {
          status = SE_ConvertCoordToGivenSystem1(convert_params_ptr, c_index,
                   dest_coord_ptr,dest_coord_ptr);
      }
   }

   return status;
} /* end SE_ConvertCoordToGivenSystem1 */

/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertCoordToGivenSystem
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_ConvertCoordToGivenSystem
(
    const SE_CONVERT_COORD_SYSTEM_PAIR  convert_params_ptr,
    const SE_COORD                     *src_coord_ptr,
          SE_COORD                     *dest_coord_ptr
)
{
    return SE_ConvertCoordToGivenSystem1 (convert_params_ptr, 0,
					  src_coord_ptr,
					  dest_coord_ptr);
} /* end SE_ConvertCoordToGivenSystem */

/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertCoordToGivenSystemWithoutBoundaryChecking
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_ConvertCoordToGivenSystemWithoutBoundaryChecking
(
    const SE_CONVERT_COORD_SYSTEM_PAIR  convert_params_ptr,
    const SE_COORD                     *src_coord_ptr,
          SE_COORD                     *dest_coord_ptr
)
{
    return SE_ConvertCoordToGivenSystem
           (
               convert_params_ptr,
               src_coord_ptr,
               dest_coord_ptr
           );
} /* end SE_ConvertCoordToGivenSystemWithoutBoundaryChecking*/


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_FreeCoordConversionConstants
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_FreeCoordConversionConstants
(
    SE_CONVERT_COORD_SYSTEM_PAIR pair
)
{
   SE_CONVERT_COORD_SYSTEM_PARAMS *to_free;
   SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;
   SE_UINT16 i;

   /* Cast away const */
   to_free = (SE_CONVERT_COORD_SYSTEM_PARAMS *) pair;

   if (to_free == NULL)
      status = SE_COORD_FAILURE;
   else
   {
      /* free the geoid separation table  */
      if (SE_separation_table !=NULL)
      {
         free(SE_separation_table);

         SE_separation_table = NULL;
      }

      /* free the constants, if they exist  */
      for(i=0;i<SRM_CONSTANTS_DEPTH;++i)
      {
         if (to_free->constants[i] != NULL)
         {
             free(to_free->constants[i]);
             to_free->constants[i] = NULL;
         }
      }
      free(to_free);
   }
   return status;
}


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ConvergenceOfMeridian
 *
 *-----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SRM_ConvergenceOfMeridian
(
    const SE_CONVERT_COORD_SYSTEM_PAIR convert_params_ptr,
    const SE_COORD                     *vec_origin_ptr,
          SE_FLOAT64                   *dest_cosine_gamma,
          SE_FLOAT64                   *dest_sine_gamma
)
{
   SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;
   SE_COORD_SYSTEM_CONSTANTS *constants;
   SE_COORD temp_coord;

   int c_index;

#ifdef CONV_DEBUG
   FILE *f_ptr;

   f_ptr = SE_GetErrorFilePtr();
#endif

   if (!convert_params_ptr)
       status = SE_COORD_INVALID_SRF_PAIR;
   else if (!vec_origin_ptr)
       status = SE_COORD_INVALID_SOURCE_COORD;
   else if (!dest_cosine_gamma)
       status = SE_COORD_INVALID_DEST_COSINE_GAMMA;
   else if (!dest_sine_gamma)
       status = SE_COORD_INVALID_DEST_SINE_GAMMA;


   if(!SE_ValidCoord(vec_origin_ptr,SE_TRUE,NULL))
      status = SE_COORD_INVALID_SOURCE_COORD;

   /* always reset the constants index for this function */

   c_index = 0;

   /* make sure we can safely reference the constants pointer array */
   if(c_index < SRM_CONSTANTS_DEPTH)
   {

      constants = convert_params_ptr->constants[c_index];

      /* it should never be NULL, if it is then the constants are not
         being created correctly
      */
      if(constants == NULL)
      {
#ifdef CONV_DEBUG
         fprintf(f_ptr, "[SEDRIS CONVERSIONS] SRM_ConvergenceOfMeridian"\
                 " constants pointer was NULL!");
#endif

         status = SE_COORD_FAILURE;
      }
   }

   /* return on fatal errors */

   if(!STATUS_CODE_IS_GOOD(status))
      return status;

   switch(constants->vector_convert_type)
   {
      /* special cases where answers are zero   */
      case GD_M:
      case LTP_TO_M:
           *dest_sine_gamma = 0;
           *dest_cosine_gamma = 0;
           status = SE_COORD_SUCCESS;
           break;

      case M_GD:
      case M_TO_LTP:
           *dest_sine_gamma = 0;
           *dest_cosine_gamma = 0;
           status = SE_COORD_SUCCESS;
           break;


      case GD_EC:
      case LTP_TO_EC:
           *dest_sine_gamma = 0;
           *dest_cosine_gamma = 0;
           status = SE_COORD_SUCCESS;
           break;

      case EC_TO_LTP:
      case EC_GD:
           *dest_sine_gamma = 0;
           *dest_cosine_gamma = 0;
           status = SE_COORD_SUCCESS;
           break;

      /* end special cases */

      case LTP_TO_PS:
           /* no break */

      case GD_PS:
      /* the longitude is not modified by the ps longitude origin in order to
       * be consistent with the coordinate conversion function.
       */

      /* no conversion required to obtain sin/cos of com */
      if(vec_origin_ptr->is_2D == SE_TRUE)
      {
         if(vec_origin_ptr->u.coord_2d.spatial_reference_frame != SRM_GD_2D_SRF)
         {
#ifdef CONV_DEBUG
            fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian] For conversion type "\
                    "%i the SRF of the source coord is required to be %s.",
                    constants->vector_convert_type,SRM_PrintSRF2D(vec_origin_ptr->u.coord_2d.spatial_reference_frame));
#endif
            status = SE_COORD_INVALID_SOURCE_COORD;
         }
         else
         {
            *dest_cosine_gamma = cos(vec_origin_ptr->u.coord_2d.u.gd_coord.geodetic_longitude);
            *dest_sine_gamma = sin(vec_origin_ptr->u.coord_2d.u.gd_coord.geodetic_longitude);

            if(vec_origin_ptr->u.coord_2d.u.gd_coord.geodetic_latitude >= 0)
               *dest_cosine_gamma = -(*dest_cosine_gamma);
         }
      }
      else /* 3d */
      {
         if(vec_origin_ptr->u.coord_3d.spatial_reference_frame != SRM_GD_3D_SRF)
         {
#ifdef CONV_DEBUG
            fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian] For conversion type "\
                    "%i the SRF of the source coord is required to be %s.",
                    constants->vector_convert_type,SRM_PrintSRF3D(vec_origin_ptr->u.coord_3d.spatial_reference_frame));
#endif
            status = SE_COORD_INVALID_SOURCE_COORD;
         }
         else
         {
            *dest_cosine_gamma = cos(vec_origin_ptr->u.coord_3d.u.gd_coord.geodetic_longitude);
            *dest_sine_gamma = sin(vec_origin_ptr->u.coord_3d.u.gd_coord.geodetic_longitude);

            if(vec_origin_ptr->u.coord_3d.u.gd_coord.geodetic_latitude >= 0)
               *dest_cosine_gamma = -(*dest_cosine_gamma);
         }

      }
      break;

      case PS_GD:
      case PS_TO_LTP:
      /* the longitude is not modified by the ps longitude origin in order to
       * be consistent with the coordinate conversion function.
       */

      /* no conversion required to obtain sin/cos of com */
      if(vec_origin_ptr->is_2D == SE_TRUE)
      {
         if(vec_origin_ptr->u.coord_2d.spatial_reference_frame != SRM_PS_2D_SRF)
         {
            #ifdef CONV_DEBUG

            fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian] For conversion type "\
                    "%i the SRF of the source coord is required to be %s.",
                    constants->vector_convert_type,SRM_PrintSRF2D(vec_origin_ptr->u.coord_2d.spatial_reference_frame));
            #endif

            status = SE_COORD_INVALID_SOURCE_COORD;
         }
         else
         {
            SE_FLOAT64 bot;

            bot = sqrt(vec_origin_ptr->u.coord_2d.u.ps_coord.x *
                       vec_origin_ptr->u.coord_2d.u.ps_coord.x +
                       vec_origin_ptr->u.coord_2d.u.ps_coord.y *
                       vec_origin_ptr->u.coord_2d.u.ps_coord.y);

            *dest_cosine_gamma = vec_origin_ptr->u.coord_2d.u.ps_coord.x/bot;
            *dest_sine_gamma = vec_origin_ptr->u.coord_2d.u.ps_coord.y/bot;

            if(constants->specific_constants.ps_gd.pole == SE_NORTH_POLE)
               *dest_cosine_gamma = -(*dest_cosine_gamma);
         }
      }
      else /* 3d */
      {
         if(vec_origin_ptr->u.coord_3d.spatial_reference_frame != SRM_GD_3D_SRF)
         {

            #ifdef CONV_DEBUG

            fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian] For conversion type "\
                    "%i the SRF of the source coord is required to be %s.",
                    constants->vector_convert_type,SRM_PrintSRF3D(vec_origin_ptr->u.coord_3d.spatial_reference_frame));
            #endif
            status = SE_COORD_INVALID_SOURCE_COORD;
         }
         else
         {
            SE_FLOAT64 bot;

            bot = sqrt(vec_origin_ptr->u.coord_3d.u.ps_coord.x *
                       vec_origin_ptr->u.coord_3d.u.ps_coord.x +
                       vec_origin_ptr->u.coord_3d.u.ps_coord.y *
                       vec_origin_ptr->u.coord_3d.u.ps_coord.y);

            *dest_cosine_gamma = vec_origin_ptr->u.coord_3d.u.ps_coord.x/bot;
            *dest_sine_gamma = vec_origin_ptr->u.coord_3d.u.ps_coord.y/bot;

            if(constants->specific_constants.ps_gd.pole == SE_NORTH_POLE)
               *dest_cosine_gamma = -(*dest_cosine_gamma);
         }
      }
      break;

      /* these cases all have an origin in GD */

      case LTP_TO_TM:
      case LTP_TO_UTM:
      case LTP_TO_LCC:
           /* set constants index to constants for GD to X */
           c_index = 2;

           /* no break */

      case GD_TM:
      case GD_UTM:
      case GD_LCC1:
      case GD_LCC2:

      if(vec_origin_ptr->is_2D == SE_TRUE)
      {
         if(vec_origin_ptr->u.coord_2d.spatial_reference_frame!=SRM_GD_2D_SRF)
         {
#ifdef CONV_DEBUG
            fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian] For conversion type "\
                    "%i the the SRF of the source coord is required to be %s.",
                    constants->vector_convert_type,SRM_PrintSRF2D(vec_origin_ptr->u.coord_2d.spatial_reference_frame));
            #endif

            status = SE_COORD_INVALID_SOURCE_COORD;
         }
      }
      else /* 3d */
      {
         if (vec_origin_ptr->u.coord_3d.spatial_reference_frame!=SRM_GD_3D_SRF)
             status = SE_COORD_INVALID_SOURCE_COORD;

      }
      /*  conversion required to obtain sin/cos of com */

      status = SE_ConvertCoordToGivenSystem1(convert_params_ptr,c_index,vec_origin_ptr,&temp_coord);

      if(!STATUS_CODE_IS_GOOD(status))
      {
#ifdef CONV_DEBUG
          fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian]"\
                  " Origin Conversion failed");
#endif
      }
      else
      {
         *dest_cosine_gamma = convert_params_ptr->constants[0]->cos_gamma;
         *dest_sine_gamma = convert_params_ptr->constants[0]->sin_gamma;
      }
      break;

      case LCC_TO_LTP:
      case LCC1_GD:
      case LCC2_GD:

      if(vec_origin_ptr->is_2D == SE_TRUE)
      {
         if(vec_origin_ptr->u.coord_2d.spatial_reference_frame != SRM_LCC_2D_SRF)
         {
            #ifdef CONV_DEBUG

            fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian] For conversion type "\
                    "%i the the SRF of the source coord is required to be %s.",
                    constants->vector_convert_type,SRM_PrintSRF2D(vec_origin_ptr->u.coord_2d.spatial_reference_frame));
            #endif

            status = SE_COORD_INVALID_SOURCE_COORD;
         }
      }
      else /* 3d */
      {
         if(vec_origin_ptr->u.coord_3d.spatial_reference_frame != SRM_LCC_3D_SRF)
         {
            #ifdef CONV_DEBUG

            fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian] For conversion type "\
                    "%i the the SRF of the source coord is required to be %s.",
                    constants->vector_convert_type,SRM_PrintSRF3D(vec_origin_ptr->u.coord_3d.spatial_reference_frame));
            #endif

            status = SE_COORD_INVALID_SOURCE_COORD;
         }
      }
      /*  conversion required to obtain sin/cos of com */

      status = SE_ConvertCoordToGivenSystem(convert_params_ptr,vec_origin_ptr,&temp_coord);

      if(!STATUS_CODE_IS_GOOD(status))
      {
         #ifdef CONV_DEBUG

         fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian] Origin Conversion failed");

         #endif
      }
      else
      {
         *dest_cosine_gamma = convert_params_ptr->constants[0]->cos_gamma;
         *dest_sine_gamma = convert_params_ptr->constants[0]->sin_gamma;
      }
      break;

      case TM_TO_LTP:
      case TM_GD:

      if(vec_origin_ptr->is_2D == SE_TRUE)
      {
         if(vec_origin_ptr->u.coord_2d.spatial_reference_frame != SRM_TM_2D_SRF)
         {
            #ifdef CONV_DEBUG

            fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian] For conversion type "\
                    "%i the the SRF of the source coord is required to be %s.",
                    constants->vector_convert_type,SRM_PrintSRF2D(vec_origin_ptr->u.coord_2d.spatial_reference_frame));
            #endif

            status = SE_COORD_INVALID_SOURCE_COORD;
         }
      }
      else /* 3d */
      {
         if(vec_origin_ptr->u.coord_3d.spatial_reference_frame != SRM_TM_3D_SRF)
         {
            #ifdef CONV_DEBUG

            fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian] For conversion type "\
                    "%i the the SRF of the source coord is required to be %s.",
                    constants->vector_convert_type,SRM_PrintSRF3D(vec_origin_ptr->u.coord_3d.spatial_reference_frame));
            #endif

            status = SE_COORD_INVALID_SOURCE_COORD;
         }
      }
      /*  conversion required to obtain sin/cos of com */

      status = SE_ConvertCoordToGivenSystem
               (convert_params_ptr,vec_origin_ptr,&temp_coord);

      if(!STATUS_CODE_IS_GOOD(status))
      {
#ifdef CONV_DEBUG
          fprintf(f_ptr,"[SEDRIS CONVERSIONS::"\
                  "SRM_ConvergenceOfMeridian] Origin Conversion failed");
#endif
      }
      else
      {
         *dest_cosine_gamma = convert_params_ptr->constants[0]->cos_gamma;
         *dest_sine_gamma = convert_params_ptr->constants[0]->sin_gamma;
      }
      break;

      case UTM_TO_LTP:
      case UTM_GD:
           if (vec_origin_ptr->is_2D == SE_TRUE)
           {
               if (vec_origin_ptr->u.coord_2d.spatial_reference_frame
                   != SRM_UTM_2D_SRF)
               {
#ifdef CONV_DEBUG
                   fprintf(f_ptr,"[SEDRIS CONVERSIONS::"\
                           "SRM_ConvergenceOfMeridian] For conversion type "\
                           "%i the SRF of the source coord is required to "\
                           "be %s.", constants->vector_convert_type,
                           SRM_PrintSRF2D(vec_origin_ptr->u.coord_2d.
                                          spatial_reference_frame));
#endif
                   status = SE_COORD_INVALID_SOURCE_COORD;
               }
           }
           else if (vec_origin_ptr->u.coord_3d.spatial_reference_frame
                    != SRM_UTM_3D_SRF)
           {
#ifdef CONV_DEBUG
               fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian]"\
                       " For conversion type %i the SRF of the source coord "\
                       " is required to be %s.",
                       constants->vector_convert_type,
                       SRM_PrintSRF3D(vec_origin_ptr->u.coord_3d.
                                      spatial_reference_frame));
#endif
               status = SE_COORD_INVALID_SOURCE_COORD;
           }

          /*
           * conversion required to obtain sin/cos of C. of M.
           */
           status = SE_ConvertCoordToGivenSystem
                    (convert_params_ptr,vec_origin_ptr,&temp_coord);

           if (!STATUS_CODE_IS_GOOD(status))
           {
#ifdef CONV_DEBUG
               fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvergenceOfMeridian]"\
                       " Origin Conversion failed");
#endif
           }
           else
           {
               *dest_cosine_gamma = convert_params_ptr->constants[0]
                                    ->cos_gamma;
               *dest_sine_gamma = convert_params_ptr->constants[0]
                                    ->sin_gamma;
           }
           break;

        default:
#ifdef CONV_DEBUG
             fprintf(f_ptr,"[SEDRIS CONVERSIONS::"\
                     "SRM_ConvergenceOfMeridian] Unsupported type."\
                     "Type id is: %d", (int) constants->convert_type);
#endif
             status = SE_COORD_UNSUPPORTED;
             break;
    } /* end switch on convert type */


    return status;
} /* end SRM_ConvergenceOfMeridian */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ConvertCoordArray
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SRM_ConvertCoordArray
(
    const SE_CONVERT_COORD_SYSTEM_PAIR  convert_params_ptr,
          SE_PINT32                     array_size,
    const SE_COORD                     *source_coord_ptr,
          SE_COORD                     *dest_coord_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM  status = SE_COORD_SUCCESS;
    SE_COORD                  *interim_coord_ptr;
                              /*
                               * an interim memory space, used to hold
                               * the converted coordinates to avoid
                               * overwriting dest_coord_ptr in case
                               * of an error condition
                               */
    SE_UINT32                  i;
    SE_BOOLEAN                 invalid_source = SE_FALSE,
                               invalid_dest = SE_FALSE;

    if (!convert_params_ptr)
        status = SE_COORD_INVALID_SRF_PAIR;
    else if (!source_coord_ptr || (array_size == 0))
        status = SE_COORD_INVALID_SOURCE_COORD;
    else if ((interim_coord_ptr=(SE_COORD *) calloc(array_size,
                                             sizeof(SE_COORD))) == NULL)
    {
        status = SE_COORD_FAILURE;
    }
    else
    {
       /*
        * Apply the conversions to all the coordinates in the array.
        */
        for (i=0; (status == SE_COORD_SUCCESS) &&
                  (i < (SE_UINT32)array_size); i++)
        {
            SE_COORD_STATUS_CODE_ENUM interim_status;

            interim_status = SE_ConvertCoordToGivenSystem
                             (
                                 convert_params_ptr,
                                &source_coord_ptr[i],
                                &interim_coord_ptr[i]
                             );

            switch (interim_status)
            {
                /*
                 * If an error condition is raised, then return the
                 * condition without changing the *dest_coord_ptr
                 */
                 case SE_COORD_FAILURE:
                 case SE_COORD_UNSUPPORTED:
                 case SE_COORD_INVALID_SRF_PAIR:
                 case SE_COORD_INVALID_SOURCE_COORD:
                 case SE_COORD_INVALID_DEST_COORD:
                      status = interim_status;
                      break;

                /* otherwise register the returned condition */
                 case SE_COORD_EXTENDED_SOURCE_COORD:
                      invalid_source = SE_TRUE;
                      break;
                 case SE_COORD_EXTENDED_DEST_COORD:
                      invalid_dest = SE_TRUE;
                      break;
                 case SE_COORD_EXTENDED_SOURCE_AND_DEST_COORD:
                      invalid_source = SE_TRUE;
                      invalid_dest = SE_TRUE;
                      break;
                 case SE_COORD_SUCCESS:
                 default:
                      /* do nothing; */
                      break;
            } /* end switch */
        } /* end for i */

        if (status == SE_COORD_SUCCESS)
        {
           /*
            * copy the interim array to the destination array
            */
            memcpy(dest_coord_ptr, interim_coord_ptr,
                   array_size * sizeof(SE_COORD));

            /* assess the return condition */
            if (invalid_source == SE_TRUE && invalid_dest == SE_TRUE)
                status = SE_COORD_EXTENDED_SOURCE_AND_DEST_COORD;
            else if (invalid_source == SE_TRUE)
                status = SE_COORD_EXTENDED_SOURCE_COORD;
            else if (invalid_dest == SE_TRUE)
                status = SE_COORD_EXTENDED_DEST_COORD;
        }
        free(interim_coord_ptr); /* free the interim array */
    }
    return status;
} /* end SRM_ConvertCoordArray */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ConvertMatrix3x3
 *
 *-----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SRM_ConvertMatrix3x3
(
    const SE_CONVERT_COORD_SYSTEM_PAIR convert_params_ptr,
    const SE_COORD                     *src_coord_ptr,
    const SE_MATRIX_3X3_TYPE           *source_3x3_ptr,
          SE_MATRIX_3X3_TYPE           *dest_3x3_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (!convert_params_ptr)
        status = SE_COORD_INVALID_SRF_PAIR;
    else if (!src_coord_ptr)
        status = SE_COORD_INVALID_SOURCE_COORD;
    else if (!source_3x3_ptr)
        status = SE_COORD_INVALID_SOURCE_MATRIX;
    else if (!dest_3x3_ptr)
        status = SE_COORD_INVALID_DEST_MATRIX;
    else
        status = SE_COORD_FAILURE; /* $$$ function not implemented yet */

    return status;
} /* end SRM_ConvertMatrix3x3 */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_ConvertVector3Type
 *
 *-----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SRM_ConvertVector3Type
(
    const SE_CONVERT_COORD_SYSTEM_PAIR convert_params_ptr,
    const SE_COORD                     *conversion_location_src_ptr,
    const SE_VECTOR_3_TYPE             *src_vec_ptr,
          SE_VECTOR_3_TYPE             *dest_vec_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;
    SE_COORD_SYSTEM_CONSTANTS *constants;
    SE_FLOAT64 sin_gamma, cos_gamma;

#ifdef CONV_DEBUG
    FILE *f_ptr;

    f_ptr = SE_GetErrorFilePtr();
#endif

   if (!convert_params_ptr)
       status = SE_COORD_INVALID_SRF_PAIR;
   else if (!src_vec_ptr)
       status = SE_COORD_INVALID_SOURCE_VECTOR;
   else if (!dest_vec_ptr)
       status = SE_COORD_INVALID_DEST_VECTOR;
   else
   {
       /*
        * assign constants
        */
        constants = convert_params_ptr->constants[0];

       /*
        * We don't need an origin for gc <> ltp vector rotations,
        * so don't require it for those cases, and don't try to
        * validate it for them if provided.
        */

        if (constants->vector_convert_type != GC_LTP ||
            constants->vector_convert_type != LTP_GC)
        {
            if (!conversion_location_src_ptr)
                status = SE_COORD_INVALID_SOURCE_COORD;
            else if (!SE_ValidCoord(conversion_location_src_ptr,SE_TRUE,NULL))
                status = SE_COORD_INVALID_SOURCE_COORD;
        }
    }

    if (status == SE_COORD_SUCCESS)
    {
       /*
        * set up vector conversion
        */
        switch(constants->vector_convert_type)
        {
           /* LTP to GC */
           /* This is just a coordinate conversion on the vector components */
            case LTP_GC:
            {
                SE_COORD ltp_coord;
                ltp_coord.is_2D = SE_FALSE;
                ltp_coord.u.coord_3d.spatial_reference_frame = SRM_LTP_3D_SRF;

                ltp_coord.u.coord_3d.u.ltp_coord.x = src_vec_ptr->vector[0];
                ltp_coord.u.coord_3d.u.ltp_coord.y = src_vec_ptr->vector[1];
                ltp_coord.u.coord_3d.u.ltp_coord.z = src_vec_ptr->vector[2];

                status = SE_ConvertCoordToGivenSystem
                         (
                              convert_params_ptr,
                              &ltp_coord,&ltp_coord
                         );
                if (STATUS_CODE_IS_GOOD(status))
                {
                    dest_vec_ptr->vector[0] = ltp_coord.u.coord_3d.u.
                                              ltp_coord.x;
                    dest_vec_ptr->vector[1] = ltp_coord.u.coord_3d.u.
                                              ltp_coord.y;
                    dest_vec_ptr->vector[2] = ltp_coord.u.coord_3d.u.
                                              ltp_coord.z;
                }
            }
            break;

         case GC_LTP:
             /*
              * This is just a coordinate conversion on the vector components
              */
         {
              SE_COORD gc_coord;
              gc_coord.is_2D = SE_FALSE;
              gc_coord.u.coord_3d.spatial_reference_frame = SRM_GC_3D_SRF;

              gc_coord.u.coord_3d.u.gc_coord.x = src_vec_ptr->vector[0];
              gc_coord.u.coord_3d.u.gc_coord.y = src_vec_ptr->vector[1];
              gc_coord.u.coord_3d.u.gc_coord.z = src_vec_ptr->vector[2];

              status = SE_ConvertCoordToGivenSystem
                       (convert_params_ptr,&gc_coord,&gc_coord);
              if (STATUS_CODE_IS_GOOD(status))
              {
                 dest_vec_ptr->vector[0] = gc_coord.u.coord_3d.u.gc_coord.x;
                 dest_vec_ptr->vector[1] = gc_coord.u.coord_3d.u.gc_coord.y;
                 dest_vec_ptr->vector[2] = gc_coord.u.coord_3d.u.gc_coord.z;
              }
         }
         break;


         /* X to LTP */
         case PS_GD:
         case LCC1_GD:
         case LCC2_GD:
         case M_GD:
         case EC_GD:
         case TM_GD:
         case UTM_GD:
         case PS_TO_LTP:
         case LCC_TO_LTP:
         case M_TO_LTP:
         case EC_TO_LTP:
         case TM_TO_LTP:
         case UTM_TO_LTP:
             /* get sin and cos of COM/gamma */
              status = SRM_ConvergenceOfMeridian
                       (
                           convert_params_ptr,
                           conversion_location_src_ptr,
                          &cos_gamma,&sin_gamma
                       );

             /* make sure SRM_ConvergenceOfMeridian didn't fail */

              if (!STATUS_CODE_IS_GOOD(status))
              {
#ifdef CONV_DEBUG
                   fprintf(f_ptr,"[SEDRIS CONVERSIONS::"
                           "SRM_ConvertVector3Type SRM_ConvergenceOfMeridian"
                           " returned unsuccessfully, can't to proceed.\n");
#endif
                   return status;
              }

             /* rotate u,v,w to u_prime, v_prime,w_prime */

             /* u_prime = cos_com * u + sin_com * v */

              dest_vec_ptr->vector[0] = cos_gamma * src_vec_ptr->vector[0] +
                                        sin_gamma * src_vec_ptr->vector[1];

             /* v_prime = - sin_com * u + sin_com * v */

              dest_vec_ptr->vector[1] = -sin_gamma * src_vec_ptr->vector[0] +
                                        sin_gamma * src_vec_ptr->vector[1];

             /* w_prime = w        */

              dest_vec_ptr->vector[2] = src_vec_ptr->vector[2];
              break;

         /* LTP to X */
         case GD_PS:
         case LTP_TO_PS:
         case GD_LCC1:
         case GD_LCC2:
         case GD_M:
         case GD_EC:
         case GD_TM:
         case GD_UTM:
         case LTP_TO_LCC:
         case LTP_TO_M:
         case LTP_TO_EC:
         case LTP_TO_TM:
         case LTP_TO_UTM:

         /* let SRM_ConvergenceOfMeridian do most of the error checking */

         /* get sin and cos of COM/gamma     */

         status = SRM_ConvergenceOfMeridian
                  (
                      convert_params_ptr,
                      conversion_location_src_ptr,
                     &cos_gamma,
                     &sin_gamma
                  );

         /* make sure SRM_ConvergenceOfMeridian didn't fail     */

         if(!STATUS_CODE_IS_GOOD(status))
         {
#ifdef CONV_DEBUG
            fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_ConvertVector3Type"\
                    " SRM_ConvergenceOfMeridian returned " \
                    "unsuccessfully, unable to proceed.\n");
#endif
            return status;
         }
         /* rotate ltp to pcs */

         /* u_prime = cos_com * u - sin_com * v */

         dest_vec_ptr->vector[0] = cos_gamma * src_vec_ptr->vector[0] -
                                   sin_gamma * src_vec_ptr->vector[1];

         /* v_prime = sin_com * u + sin_com * v */

         dest_vec_ptr->vector[1] = sin_gamma * src_vec_ptr->vector[0] +
                                   sin_gamma * src_vec_ptr->vector[1];

         /* w_prime = w        */
         dest_vec_ptr->vector[2] = src_vec_ptr->vector[2];
         break;

         /* default case should never happen */
         default:
#ifdef CONV_DEBUG
         fprintf(f_ptr,"[SEDRIS CONVERSIONS] Unsupported vector conversion."\
                       "Type id is: %d",(int) constants->convert_type);
#endif

         status = SE_COORD_UNSUPPORTED;
         break;
      } /* end switch on convert type */
   }
   return status;
} /* end SRM_ConvertVector3Type */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_LocalTransformation
 *
 *-----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SRM_LocalTransformation
(
    const SE_CONVERT_COORD_SYSTEM_PAIR convert_params_ptr,
    const SE_COORD                     *src_coord_ptr,
          SE_MATRIX_3X3_TYPE           *dest_3x3_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (!convert_params_ptr)
        status = SE_COORD_INVALID_SRF_PAIR;
    else if (!src_coord_ptr)
        status = SE_COORD_INVALID_SOURCE_COORD;
    else if (!dest_3x3_ptr)
        status = SE_COORD_INVALID_DEST_MATRIX;
    else
        status = SE_COORD_FAILURE; /* $$$ function not implemented yet */

    return status;
} /* end SRM_LocalTransformation */
