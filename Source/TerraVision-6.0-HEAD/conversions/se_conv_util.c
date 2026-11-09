/*
 * FILE       : se_conv_util.c
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

#ifdef WIN32
#define SE_TURN_OFF_CONVERSIONS_WIN32_IMPORT
#endif

#include <stdio.h>
#include <math.h>
#include "se_conv.h"

#ifdef WIN32
#undef SE_TURN_OFF_CONVERSIONS_WIN32_IMPORT
#endif


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_CoordEqual
 *
 *-----------------------------------------------------------------------------
 */
SE_BOOLEAN
SRM_CoordEqual
(
    const SE_COORD *coord1_ptr,
    const SE_COORD *coord2_ptr
)
{
    /*
     * If the spatial reference frames are different, we know they're not equal
     */
    if (!coord1_ptr || !coord2_ptr || (coord1_ptr->is_2D != coord2_ptr->is_2D))
        return SE_FALSE;

    /*
     * Compare the coordinates
     */
    switch (coord1_ptr->is_2D)
    {
        case SE_FALSE:
             return SRM_Coord3DEqual(&(coord1_ptr->u.coord_3d),
                                    &(coord2_ptr->u.coord_3d));
        case SE_TRUE:
        default:
             return SRM_Coord2DEqual(&(coord1_ptr->u.coord_2d),
                                    &(coord2_ptr->u.coord_2d));
    } /* end switch */
} /* end SRM_CoordEqual */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_DeriveCoord2DFrom3D
 *
 *-----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SRM_DeriveCoord2DFrom3D
(
    const SE_COORD_3D *coord_3d_ptr,
          SE_COORD_2D *coord_2d_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (!coord_2d_ptr)
    {
        status = SE_COORD_INVALID_DEST_COORD;
    }
    else if (!coord_3d_ptr)
    {
        status = SE_COORD_INVALID_SOURCE_COORD;
    }
    else
    {
        switch (coord_3d_ptr->spatial_reference_frame)
        {
            case SRM_GD_3D_SRF:
                 coord_2d_ptr->spatial_reference_frame = SRM_GD_2D_SRF;
                 coord_2d_ptr->u.gd_coord.geodetic_longitude = coord_3d_ptr->u.gd_coord.geodetic_longitude;
                 coord_2d_ptr->u.gd_coord.geodetic_latitude = coord_3d_ptr->u.gd_coord.geodetic_latitude;
                 break;
            case SRM_EC_3D_SRF:
                 coord_2d_ptr->spatial_reference_frame = SRM_EC_2D_SRF;
                 coord_2d_ptr->u.ec_coord.x = coord_3d_ptr->u.ec_coord.x;
                 coord_2d_ptr->u.ec_coord.y = coord_3d_ptr->u.ec_coord.y;
                 break;
            case SRM_PS_3D_SRF:
                 coord_2d_ptr->spatial_reference_frame = SRM_PS_2D_SRF;
                 coord_2d_ptr->u.ps_coord.x = coord_3d_ptr->u.ps_coord.x;
                 coord_2d_ptr->u.ps_coord.y = coord_3d_ptr->u.ps_coord.y;
                 break;
            case SRM_LCC_3D_SRF:
                 coord_2d_ptr->spatial_reference_frame = SRM_LCC_2D_SRF;
                 coord_2d_ptr->u.lcc_coord.x = coord_3d_ptr->u.lcc_coord.x;
                 coord_2d_ptr->u.lcc_coord.y = coord_3d_ptr->u.lcc_coord.y;
                 break;
            case SRM_TM_3D_SRF:
                 coord_2d_ptr->spatial_reference_frame = SRM_TM_2D_SRF;
                 coord_2d_ptr->u.tm_coord.x = coord_3d_ptr->u.tm_coord.x;
                 coord_2d_ptr->u.tm_coord.y = coord_3d_ptr->u.tm_coord.y;
                 break;
            case SRM_UTM_3D_SRF:
                 coord_2d_ptr->spatial_reference_frame = SRM_UTM_2D_SRF;
                 coord_2d_ptr->u.utm_coord.hemisphere = coord_3d_ptr->u.utm_coord.hemisphere;
                 coord_2d_ptr->u.utm_coord.x = coord_3d_ptr->u.utm_coord.x;
                 coord_2d_ptr->u.utm_coord.y = coord_3d_ptr->u.utm_coord.y;
                 break;
            case SRM_LTP_3D_SRF:
                 coord_2d_ptr->spatial_reference_frame = SRM_LTP_2D_SRF;
                 coord_2d_ptr->u.ltp_coord.x = coord_3d_ptr->u.ltp_coord.x;
                 coord_2d_ptr->u.ltp_coord.y = coord_3d_ptr->u.ltp_coord.y;
                 break;
            case SRM_LSR_3D_SRF:
                 coord_2d_ptr->spatial_reference_frame = SRM_LSR_2D_SRF;
                 coord_2d_ptr->u.lsr_coord.x = coord_3d_ptr->u.lsr_coord.x;
                 coord_2d_ptr->u.lsr_coord.y = coord_3d_ptr->u.lsr_coord.y;
                 break;
            case SRM_M_3D_SRF:
                 coord_2d_ptr->spatial_reference_frame = SRM_M_2D_SRF;
                 coord_2d_ptr->u.m_coord.x = coord_3d_ptr->u.m_coord.x;
                 coord_2d_ptr->u.m_coord.y = coord_3d_ptr->u.m_coord.y;
                 break;
            case SRM_OM_3D_SRF:
                 coord_2d_ptr->spatial_reference_frame = SRM_OM_2D_SRF;
                 coord_2d_ptr->u.om_coord.x = coord_3d_ptr->u.om_coord.x;
                 coord_2d_ptr->u.om_coord.y = coord_3d_ptr->u.om_coord.y;
                 break;
            case SRM_UPS_3D_SRF:
                 coord_2d_ptr->spatial_reference_frame = SRM_UPS_2D_SRF;
                 coord_2d_ptr->u.ups_coord.x = coord_3d_ptr->u.ups_coord.x;
                 coord_2d_ptr->u.ups_coord.y = coord_3d_ptr->u.ups_coord.y;
                 break;
            default:
                 status = SE_COORD_INVALID_SOURCE_COORD;
                 break;
        }
    }
    return status;
} /* end SRM_DeriveCoord2DFrom3D*/


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_DeriveCoord3DFrom2DForGivenElevation
 *
 *-----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SRM_DeriveCoord3DFrom2DForGivenElevation
(
    const SE_COORD_2D *coord_2d_ptr,
          SE_FLOAT64   elevation,
          SE_COORD_3D *coord_3d_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (!coord_3d_ptr)
    {
        status = SE_COORD_INVALID_DEST_COORD;
    }
    else if (!coord_2d_ptr)
    {
        status = SE_COORD_INVALID_SOURCE_COORD;
    }
    else
    {
        switch (coord_2d_ptr->spatial_reference_frame)
        {
            case SRM_GD_2D_SRF:
                 coord_3d_ptr->spatial_reference_frame = SRM_GD_3D_SRF;
                 coord_3d_ptr->u.gd_coord.geodetic_longitude = coord_2d_ptr->u.gd_coord.geodetic_longitude;
                 coord_3d_ptr->u.gd_coord.geodetic_latitude = coord_2d_ptr->u.gd_coord.geodetic_latitude;
                 coord_3d_ptr->u.gd_coord.elevation = elevation;
                 break;
            case SRM_EC_2D_SRF:
                 coord_3d_ptr->spatial_reference_frame = SRM_EC_3D_SRF;
                 coord_3d_ptr->u.ec_coord.x = coord_2d_ptr->u.ec_coord.x;
                 coord_3d_ptr->u.ec_coord.y = coord_2d_ptr->u.ec_coord.y;
                 coord_3d_ptr->u.ec_coord.z = elevation;
                 break;
            case SRM_PS_2D_SRF:
                 coord_3d_ptr->spatial_reference_frame = SRM_PS_3D_SRF;
                 coord_3d_ptr->u.ps_coord.x = coord_2d_ptr->u.ps_coord.x;
                 coord_3d_ptr->u.ps_coord.y = coord_2d_ptr->u.ps_coord.y;
                 coord_3d_ptr->u.ps_coord.z = elevation;
                 break;
            case SRM_LCC_2D_SRF:
                 coord_3d_ptr->spatial_reference_frame = SRM_LCC_3D_SRF;
                 coord_3d_ptr->u.lcc_coord.x = coord_2d_ptr->u.lcc_coord.x;
                 coord_3d_ptr->u.lcc_coord.y = coord_2d_ptr->u.lcc_coord.y;
                 coord_3d_ptr->u.lcc_coord.z = elevation;
                 break;
            case SRM_TM_2D_SRF:
                 coord_3d_ptr->spatial_reference_frame = SRM_TM_3D_SRF;
                 coord_3d_ptr->u.tm_coord.x = coord_2d_ptr->u.tm_coord.x;
                 coord_3d_ptr->u.tm_coord.y = coord_2d_ptr->u.tm_coord.y;
                 coord_3d_ptr->u.tm_coord.z = elevation;
                 break;
            case SRM_UTM_2D_SRF:
                 coord_3d_ptr->spatial_reference_frame = SRM_UTM_3D_SRF;
                 coord_3d_ptr->u.utm_coord.hemisphere = coord_2d_ptr->u.utm_coord.hemisphere;
                 coord_3d_ptr->u.utm_coord.x = coord_2d_ptr->u.utm_coord.x;
                 coord_3d_ptr->u.utm_coord.y = coord_2d_ptr->u.utm_coord.y;
                 coord_3d_ptr->u.utm_coord.z = elevation;
                 break;
            case SRM_LTP_2D_SRF:
                 coord_3d_ptr->spatial_reference_frame = SRM_LTP_3D_SRF;
                 coord_3d_ptr->u.ltp_coord.x = coord_2d_ptr->u.ltp_coord.x;
                 coord_3d_ptr->u.ltp_coord.y = coord_2d_ptr->u.ltp_coord.y;
                 coord_3d_ptr->u.ltp_coord.z = elevation;
                 break;
            case SRM_LSR_2D_SRF:
                 coord_3d_ptr->spatial_reference_frame = SRM_LSR_3D_SRF;
                 coord_3d_ptr->u.lsr_coord.x = coord_2d_ptr->u.lsr_coord.x;
                 coord_3d_ptr->u.lsr_coord.y = coord_2d_ptr->u.lsr_coord.y;
                 coord_3d_ptr->u.lsr_coord.z = elevation;
                 break;
            case SRM_M_2D_SRF:
                 coord_3d_ptr->spatial_reference_frame = SRM_M_3D_SRF;
                 coord_3d_ptr->u.m_coord.x = coord_2d_ptr->u.m_coord.x;
                 coord_3d_ptr->u.m_coord.y = coord_2d_ptr->u.m_coord.y;
                 coord_3d_ptr->u.m_coord.z = elevation;
                 break;
            case SRM_OM_2D_SRF:
                 coord_3d_ptr->spatial_reference_frame = SRM_OM_3D_SRF;
                 coord_3d_ptr->u.om_coord.x = coord_2d_ptr->u.om_coord.x;
                 coord_3d_ptr->u.om_coord.y = coord_2d_ptr->u.om_coord.y;
                 coord_3d_ptr->u.om_coord.z = elevation;
                 break;
            case SRM_UPS_2D_SRF:
                 coord_3d_ptr->spatial_reference_frame = SRM_UPS_3D_SRF;
                 coord_3d_ptr->u.ups_coord.x = coord_2d_ptr->u.ups_coord.x;
                 coord_3d_ptr->u.ups_coord.y = coord_2d_ptr->u.ups_coord.y;
                 coord_3d_ptr->u.ups_coord.z = elevation;
                 break;
            default:
                 status = SE_COORD_INVALID_SOURCE_COORD;
                 break;
        }
    }
    return status;
} /* end SRM_DeriveCoord3DFrom2DForGivenElevation*/


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_Coord2DEqual
 *
 *-----------------------------------------------------------------------------
 */
SE_BOOLEAN
SRM_Coord2DEqual
(
    const SE_COORD_2D *coord1_ptr,
    const SE_COORD_2D *coord2_ptr
)
{
    /*
     * If the spatial reference frames are different, we know they're not equal
     */
    if (!coord1_ptr || !coord2_ptr ||
        (coord1_ptr->spatial_reference_frame !=
         coord2_ptr->spatial_reference_frame))
        return SE_FALSE;

    /*
     * Compare the coordinates
     */
    switch (coord1_ptr->spatial_reference_frame)
    {
        case SRM_GD_2D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.gd_coord.geodetic_longitude ==
                      coord2_ptr->u.gd_coord.geodetic_longitude) &&
                     (coord1_ptr->u.gd_coord.geodetic_latitude ==
                      coord2_ptr->u.gd_coord.geodetic_latitude));

        case SRM_EC_2D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.ec_coord.x ==
                      coord2_ptr->u.ec_coord.x) &&
                     (coord1_ptr->u.ec_coord.y ==
                      coord2_ptr->u.ec_coord.y));

        case SRM_PS_2D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.ps_coord.x ==
                      coord2_ptr->u.ps_coord.x) &&
                     (coord1_ptr->u.ps_coord.y ==
                      coord2_ptr->u.ps_coord.y));

        case SRM_LCC_2D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.lcc_coord.x ==
                      coord2_ptr->u.lcc_coord.x) &&
                     (coord1_ptr->u.lcc_coord.y ==
                      coord2_ptr->u.lcc_coord.y));

        case SRM_TM_2D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.tm_coord.x ==
                      coord2_ptr->u.tm_coord.x) &&
                     (coord1_ptr->u.tm_coord.y ==
                      coord2_ptr->u.tm_coord.y));

        case SRM_UTM_2D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.utm_coord.hemisphere ==
                      coord2_ptr->u.utm_coord.hemisphere) &&
                     (coord1_ptr->u.utm_coord.x ==
                      coord2_ptr->u.utm_coord.x) &&
                     (coord1_ptr->u.utm_coord.y ==
                      coord2_ptr->u.utm_coord.y));

        case SRM_LTP_2D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.ltp_coord.x ==
                      coord2_ptr->u.ltp_coord.x) &&
                     (coord1_ptr->u.ltp_coord.y ==
                      coord2_ptr->u.ltp_coord.y));

        case SRM_LSR_2D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.lsr_coord.x ==
                      coord2_ptr->u.lsr_coord.x) &&
                     (coord1_ptr->u.lsr_coord.y ==
                      coord2_ptr->u.lsr_coord.y));

        case SRM_M_2D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.m_coord.x ==
                      coord2_ptr->u.m_coord.x) &&
                     (coord1_ptr->u.m_coord.y ==
                      coord2_ptr->u.m_coord.y));

        case SRM_OM_2D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.om_coord.x ==
                      coord2_ptr->u.om_coord.x) &&
                     (coord1_ptr->u.om_coord.y ==
                      coord2_ptr->u.om_coord.y));

        case SRM_UPS_2D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.ups_coord.x ==
                      coord2_ptr->u.ups_coord.x) &&
                     (coord1_ptr->u.ups_coord.y ==
                      coord2_ptr->u.ups_coord.y));

        default:
#ifdef CONV_DEBUG
        {
             FILE *f_ptr;

             f_ptr = SE_GetErrorFilePtr();
             fprintf(f_ptr, "[SEDRIS CONVERSIONS] "\
                     "SRM_Coord2DEqual: unknown SRF %s encountered\n",
                    SRM_PrintSRF2D(coord1_ptr->spatial_reference_frame));
        }
#endif
             return SE_FALSE;
    }
} /* end SRM_Coord2DEqual */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_Coord3DEqual
 *
 *-----------------------------------------------------------------------------
 */
SE_BOOLEAN
SRM_Coord3DEqual
(
    const SE_COORD_3D *coord1_ptr,
    const SE_COORD_3D *coord2_ptr
)
{
    /*
     * If the spatial reference frames are different, we know they're not equal
     */
    if (!coord1_ptr || !coord2_ptr ||
        (coord1_ptr->spatial_reference_frame !=
         coord2_ptr->spatial_reference_frame))
        return SE_FALSE;

    /*
     * Compare the coordinates
     */
    switch (coord1_ptr->spatial_reference_frame)
    {
        case SRM_GD_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.gd_coord.geodetic_longitude ==
                      coord2_ptr->u.gd_coord.geodetic_longitude) &&
                     (coord1_ptr->u.gd_coord.geodetic_latitude ==
                      coord2_ptr->u.gd_coord.geodetic_latitude) &&
                     (coord1_ptr->u.gd_coord.elevation ==
                      coord2_ptr->u.gd_coord.elevation));

        case SRM_GC_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.gc_coord.x ==
                      coord2_ptr->u.gc_coord.x) &&
                     (coord1_ptr->u.gc_coord.y ==
                      coord2_ptr->u.gc_coord.y) &&
                     (coord1_ptr->u.gc_coord.z ==
                      coord2_ptr->u.gc_coord.z));

        case SRM_GM_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.gm_coord.gm_latitude ==
                      coord2_ptr->u.gm_coord.gm_latitude) &&
                     (coord1_ptr->u.gm_coord.gm_longitude ==
                      coord2_ptr->u.gm_coord.gm_longitude) &&
                     (coord1_ptr->u.gm_coord.radius ==
                      coord2_ptr->u.gm_coord.radius));

        case SRM_GEI_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.gei_coord.right_ascension ==
                      coord2_ptr->u.gei_coord.right_ascension) &&
                     (coord1_ptr->u.gei_coord.declination ==
                      coord2_ptr->u.gei_coord.declination) &&
                     (coord1_ptr->u.gei_coord.radius ==
                      coord2_ptr->u.gei_coord.radius));

        case SRM_GSE_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.gse_coord.longitude ==
                      coord2_ptr->u.gse_coord.longitude) &&
                     (coord1_ptr->u.gse_coord.latitude ==
                      coord2_ptr->u.gse_coord.latitude) &&
                     (coord1_ptr->u.gse_coord.radius ==
                      coord2_ptr->u.gse_coord.radius));

        case SRM_GSM_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.gsm_coord.longitude ==
                      coord2_ptr->u.gsm_coord.longitude) &&
                     (coord1_ptr->u.gsm_coord.latitude ==
                      coord2_ptr->u.gsm_coord.latitude) &&
                     (coord1_ptr->u.gsm_coord.radius ==
                      coord2_ptr->u.gsm_coord.radius));

        case SRM_SM_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.sm_coord.longitude ==
                      coord2_ptr->u.sm_coord.longitude) &&
                     (coord1_ptr->u.sm_coord.latitude ==
                      coord2_ptr->u.sm_coord.latitude) &&
                     (coord1_ptr->u.sm_coord.radius ==
                      coord2_ptr->u.sm_coord.radius));

        case SRM_GCS_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.gcs_coord.cell_ID ==
                      coord2_ptr->u.gcs_coord.cell_ID) &&
                     (coord1_ptr->u.gcs_coord.x ==
                      coord2_ptr->u.gcs_coord.x) &&
                     (coord1_ptr->u.gcs_coord.y ==
                      coord2_ptr->u.gcs_coord.y) &&
                     (coord1_ptr->u.gcs_coord.z ==
                      coord2_ptr->u.gcs_coord.z));

        case SRM_EC_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.ec_coord.x ==
                      coord2_ptr->u.ec_coord.x) &&
                     (coord1_ptr->u.ec_coord.y ==
                      coord2_ptr->u.ec_coord.y) &&
                     (coord1_ptr->u.ec_coord.z ==
                      coord2_ptr->u.ec_coord.z));

        case SRM_PS_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.ps_coord.x ==
                      coord2_ptr->u.ps_coord.x) &&
                     (coord1_ptr->u.ps_coord.y ==
                      coord2_ptr->u.ps_coord.y) &&
                     (coord1_ptr->u.ps_coord.z ==
                      coord2_ptr->u.ps_coord.z));

        case SRM_LCC_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.lcc_coord.x ==
                      coord2_ptr->u.lcc_coord.x) &&
                     (coord1_ptr->u.lcc_coord.y ==
                      coord2_ptr->u.lcc_coord.y) &&
                     (coord1_ptr->u.lcc_coord.z ==
                      coord2_ptr->u.lcc_coord.z));

        case SRM_TM_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.tm_coord.x ==
                      coord2_ptr->u.tm_coord.x) &&
                     (coord1_ptr->u.tm_coord.y ==
                      coord2_ptr->u.tm_coord.y) &&
                     (coord1_ptr->u.tm_coord.z ==
                      coord2_ptr->u.tm_coord.z));

        case SRM_UTM_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.utm_coord.hemisphere ==
                      coord2_ptr->u.utm_coord.hemisphere) &&
                     (coord1_ptr->u.utm_coord.x ==
                      coord2_ptr->u.utm_coord.x) &&
                     (coord1_ptr->u.utm_coord.y ==
                      coord2_ptr->u.utm_coord.y) &&
                     (coord1_ptr->u.utm_coord.z ==
                      coord2_ptr->u.utm_coord.z));

        case SRM_LTP_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.ltp_coord.x ==
                      coord2_ptr->u.ltp_coord.x) &&
                     (coord1_ptr->u.ltp_coord.y ==
                      coord2_ptr->u.ltp_coord.y) &&
                     (coord1_ptr->u.ltp_coord.z ==
                      coord2_ptr->u.ltp_coord.z));

        case SRM_LSR_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.lsr_coord.x ==
                      coord2_ptr->u.lsr_coord.x) &&
                     (coord1_ptr->u.lsr_coord.y ==
                      coord2_ptr->u.lsr_coord.y) &&
                     (coord1_ptr->u.lsr_coord.z ==
                      coord2_ptr->u.lsr_coord.z));

        case SRM_M_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.m_coord.x ==
                      coord2_ptr->u.m_coord.x) &&
                     (coord1_ptr->u.m_coord.y ==
                      coord2_ptr->u.m_coord.y) &&
                     (coord1_ptr->u.m_coord.z ==
                      coord2_ptr->u.m_coord.z));

        case SRM_OM_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.om_coord.x ==
                      coord2_ptr->u.om_coord.x) &&
                     (coord1_ptr->u.om_coord.y ==
                      coord2_ptr->u.om_coord.y) &&
                     (coord1_ptr->u.om_coord.z ==
                      coord2_ptr->u.om_coord.z));

        case SRM_UPS_3D_SRF:
             return (SE_BOOLEAN)
                    ((coord1_ptr->u.ups_coord.x ==
                      coord2_ptr->u.ups_coord.x) &&
                     (coord1_ptr->u.ups_coord.y ==
                      coord2_ptr->u.ups_coord.y) &&
                     (coord1_ptr->u.ups_coord.z ==
                      coord2_ptr->u.ups_coord.z));

        default:
#ifdef CONV_DEBUG
        {
             FILE *f_ptr;

             f_ptr = SE_GetErrorFilePtr();
             fprintf(f_ptr, "[SEDRIS CONVERSIONS] "\
                     "SRM_Coord3DEqual: unknown SRF %s encountered\n",
                    SRM_PrintSRF3D(coord1_ptr->spatial_reference_frame));
        }
#endif
             return SE_FALSE;
    }
} /* end SRM_Coord3DEqual */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_SRF2DParametersEqual
 *
 *-----------------------------------------------------------------------------
 */
SE_BOOLEAN
SRM_SRF2DParametersEqual
(
    const SRM_SRF_2D_PARAMETERS *srf1_ptr,
    const SRM_SRF_2D_PARAMETERS *srf2_ptr
)
{
    /*
     * If the spatial reference frames are different, we know they're not equal
     */
    if (!srf1_ptr || !srf2_ptr ||
        (srf1_ptr->spatial_reference_frame !=
         srf2_ptr->spatial_reference_frame))
        return SE_FALSE;

    /*
     * Compare the spatial reference frame parameters
     */
    switch (srf1_ptr->spatial_reference_frame)
    {
        case SRM_GD_2D_SRF:
             return (SE_BOOLEAN)
                    (srf1_ptr->u.gd_parameters.horizontal_datum ==
                     srf2_ptr->u.gd_parameters.horizontal_datum);

        case SRM_EC_2D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.ec_parameters.horizontal_datum ==
                      srf2_ptr->u.ec_parameters.horizontal_datum) &&
                     (srf1_ptr->u.ec_parameters.geodetic_latitude ==
                      srf2_ptr->u.ec_parameters.geodetic_latitude) &&
                     (srf1_ptr->u.ec_parameters.geodetic_longitude ==
                      srf2_ptr->u.ec_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.ec_parameters.x_offset ==
                      srf2_ptr->u.ec_parameters.x_offset) &&
                     (srf1_ptr->u.ec_parameters.y_offset ==
                      srf2_ptr->u.ec_parameters.y_offset) &&
                     (srf1_ptr->u.ec_parameters.central_scale_factor ==
                      srf2_ptr->u.ec_parameters.central_scale_factor));

        case SRM_PS_2D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.ps_parameters.horizontal_datum ==
                      srf2_ptr->u.ps_parameters.horizontal_datum) &&
                     (srf1_ptr->u.ps_parameters.pole ==
                      srf2_ptr->u.ps_parameters.pole) &&
                     (srf1_ptr->u.ps_parameters.geodetic_longitude ==
                      srf2_ptr->u.ps_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.ps_parameters.central_scale_factor ==
                      srf2_ptr->u.ps_parameters.central_scale_factor) &&
                     (srf1_ptr->u.ps_parameters.x_offset ==
                      srf2_ptr->u.ps_parameters.x_offset) &&
                     (srf1_ptr->u.ps_parameters.y_offset ==
                      srf2_ptr->u.ps_parameters.y_offset));

        case SRM_LCC_2D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.lcc_parameters.horizontal_datum ==
                      srf2_ptr->u.lcc_parameters.horizontal_datum) &&
                     (srf1_ptr->u.lcc_parameters.north_parallel_geodetic_latitude ==
                      srf2_ptr->u.lcc_parameters.north_parallel_geodetic_latitude) &&
                     (srf1_ptr->u.lcc_parameters.south_parallel_geodetic_latitude ==
                      srf2_ptr->u.lcc_parameters.south_parallel_geodetic_latitude) &&
                     (srf1_ptr->u.lcc_parameters.geodetic_latitude ==
                      srf2_ptr->u.lcc_parameters.geodetic_latitude) &&
                     (srf1_ptr->u.lcc_parameters.geodetic_longitude ==
                      srf2_ptr->u.lcc_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.lcc_parameters.x_offset ==
                      srf2_ptr->u.lcc_parameters.x_offset) &&
                     (srf1_ptr->u.lcc_parameters.y_offset ==
                      srf2_ptr->u.lcc_parameters.y_offset));

        case SRM_TM_2D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.tm_parameters.horizontal_datum ==
                      srf2_ptr->u.tm_parameters.horizontal_datum) &&
                     (srf1_ptr->u.tm_parameters.geodetic_latitude ==
                      srf2_ptr->u.tm_parameters.geodetic_latitude) &&
                     (srf1_ptr->u.tm_parameters.geodetic_longitude ==
                      srf2_ptr->u.tm_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.tm_parameters.x_offset ==
                      srf2_ptr->u.tm_parameters.x_offset) &&
                     (srf1_ptr->u.tm_parameters.y_offset ==
                      srf2_ptr->u.tm_parameters.y_offset) &&
                     (srf1_ptr->u.tm_parameters.central_scale_factor ==
                      srf2_ptr->u.tm_parameters.central_scale_factor));

        case SRM_UTM_2D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.utm_parameters.horizontal_datum ==
                      srf2_ptr->u.utm_parameters.horizontal_datum) &&
                     (srf1_ptr->u.utm_parameters.x_offset ==
                      srf2_ptr->u.utm_parameters.x_offset) &&
                     (srf1_ptr->u.utm_parameters.y_offset ==
                      srf2_ptr->u.utm_parameters.y_offset) &&
                     (srf1_ptr->u.utm_parameters.zone ==
                      srf2_ptr->u.utm_parameters.zone));

        case SRM_LTP_2D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.ltp_parameters.horizontal_datum ==
                      srf2_ptr->u.ltp_parameters.horizontal_datum) &&
                     (srf1_ptr->u.ltp_parameters.geodetic_longitude ==
                      srf2_ptr->u.ltp_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.ltp_parameters.geodetic_latitude ==
                      srf2_ptr->u.ltp_parameters.geodetic_latitude) &&
                     (srf1_ptr->u.ltp_parameters.azimuth ==
                      srf2_ptr->u.ltp_parameters.azimuth) &&
                     (srf1_ptr->u.ltp_parameters.x_offset ==
                      srf2_ptr->u.ltp_parameters.x_offset) &&
                     (srf1_ptr->u.ltp_parameters.y_offset ==
                      srf2_ptr->u.ltp_parameters.y_offset));

        case SRM_LSR_2D_SRF:
             return (SE_BOOLEAN)
                    (srf1_ptr->u.lsr_parameters.forward_direction ==
                     srf2_ptr->u.lsr_parameters.forward_direction);

        case SRM_M_2D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.m_parameters.horizontal_datum ==
                      srf2_ptr->u.m_parameters.horizontal_datum) &&
                     (srf1_ptr->u.m_parameters.geodetic_longitude ==
                      srf2_ptr->u.m_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.m_parameters.central_scale_factor ==
                      srf2_ptr->u.m_parameters.central_scale_factor) &&
                     (srf1_ptr->u.m_parameters.x_offset ==
                      srf2_ptr->u.m_parameters.x_offset) &&
                     (srf1_ptr->u.m_parameters.y_offset ==
                      srf2_ptr->u.m_parameters.y_offset));

        case SRM_OM_2D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.om_parameters.horizontal_datum ==
                      srf2_ptr->u.om_parameters.horizontal_datum) &&
                     (srf1_ptr->u.om_parameters.geodetic_latitude1 ==
                      srf2_ptr->u.om_parameters.geodetic_latitude1) &&
                     (srf1_ptr->u.om_parameters.geodetic_longitude1 ==
                      srf2_ptr->u.om_parameters.geodetic_longitude1) &&
                     (srf1_ptr->u.om_parameters.geodetic_latitude2 ==
                      srf2_ptr->u.om_parameters.geodetic_latitude2) &&
                     (srf1_ptr->u.om_parameters.geodetic_longitude2 ==
                      srf2_ptr->u.om_parameters.geodetic_longitude2) &&
                     (srf1_ptr->u.om_parameters.x_offset ==
                      srf2_ptr->u.om_parameters.x_offset) &&
                     (srf1_ptr->u.om_parameters.y_offset ==
                      srf2_ptr->u.om_parameters.y_offset) &&
                     (srf1_ptr->u.om_parameters.central_scale_factor ==
                      srf2_ptr->u.om_parameters.central_scale_factor));

        case SRM_UPS_2D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.ups_parameters.horizontal_datum ==
                      srf2_ptr->u.ups_parameters.horizontal_datum) &&
                     (srf1_ptr->u.ups_parameters.pole ==
                      srf2_ptr->u.ups_parameters.pole) &&
                     (srf1_ptr->u.ups_parameters.x_offset ==
                      srf2_ptr->u.ups_parameters.x_offset) &&
                     (srf1_ptr->u.ups_parameters.y_offset ==
                      srf2_ptr->u.ups_parameters.y_offset));

        default:
#ifdef CONV_DEBUG
        {
             FILE *f_ptr;

             f_ptr = SE_GetErrorFilePtr();
             fprintf(f_ptr, "[SEDRIS CONVERSIONS] "\
                     "SRM_SRF2DParametersEqual: unknown SRF %s encountered\n",
                    SRM_PrintSRF2D(srf1_ptr->spatial_reference_frame));
        }
#endif
             return SE_FALSE;
    }
} /* end SRM_SRF2DParametersEqual */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_SRF3DParametersEqual
 *
 *-----------------------------------------------------------------------------
 */
SE_BOOLEAN
SRM_SRF3DParametersEqual
(
    const SRM_SRF_3D_PARAMETERS *srf1_ptr,
    const SRM_SRF_3D_PARAMETERS *srf2_ptr
)
{
    /*
     * If the spatial reference frames are different, we know they're not equal
     */
    if (!srf1_ptr || !srf2_ptr ||
        (srf1_ptr->spatial_reference_frame !=
         srf2_ptr->spatial_reference_frame))
        return SE_FALSE;

    /*
     * Compare the spatial reference frame parameters
     */
    switch (srf1_ptr->spatial_reference_frame)
    {
        case SRM_GD_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.gd_parameters.horizontal_datum ==
                      srf2_ptr->u.gd_parameters.horizontal_datum) &&
                     (srf1_ptr->u.gd_parameters.vertical_datum ==
                      srf2_ptr->u.gd_parameters.vertical_datum) &&
                     (srf1_ptr->u.gd_parameters.elevation_units ==
                      srf2_ptr->u.gd_parameters.elevation_units));

        case SRM_GC_3D_SRF:
             return SE_TRUE;
             /* GC has no additional system parameters */
        case SRM_GM_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.gm_parameters.gm_epoch ==
                      srf2_ptr->u.gm_parameters.gm_epoch) &&
                     (srf1_ptr->u.gm_parameters.gm_earth_radius ==
                      srf2_ptr->u.gm_parameters.gm_earth_radius));

        case SRM_GEI_3D_SRF:
             return (SE_BOOLEAN)
                    (srf1_ptr->u.gei_parameters.epoch ==
                     srf2_ptr->u.gei_parameters.epoch);

        case SRM_GSE_3D_SRF:
             return SE_TRUE;
             /* GSE has no additional system parameters */
        case SRM_GSM_3D_SRF:
             return SE_TRUE;
             /* GSM has no additional system parameters */
        case SRM_SM_3D_SRF:
             return SE_TRUE;
             /* SM has no additional system parameters */
        case SRM_GCS_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.gcs_parameters.x_offset ==
                      srf2_ptr->u.gcs_parameters.x_offset) &&
                     (srf1_ptr->u.gcs_parameters.y_offset ==
                      srf2_ptr->u.gcs_parameters.y_offset));

        case SRM_EC_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.ec_parameters.horizontal_datum ==
                      srf2_ptr->u.ec_parameters.horizontal_datum) &&
                     (srf1_ptr->u.ec_parameters.vertical_datum ==
                      srf2_ptr->u.ec_parameters.vertical_datum) &&
                     (srf1_ptr->u.ec_parameters.geodetic_latitude ==
                      srf2_ptr->u.ec_parameters.geodetic_latitude) &&
                     (srf1_ptr->u.ec_parameters.geodetic_longitude ==
                      srf2_ptr->u.ec_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.ec_parameters.x_offset ==
                      srf2_ptr->u.ec_parameters.x_offset) &&
                     (srf1_ptr->u.ec_parameters.y_offset ==
                      srf2_ptr->u.ec_parameters.y_offset) &&
                     (srf1_ptr->u.ec_parameters.central_scale_factor ==
                      srf2_ptr->u.ec_parameters.central_scale_factor) &&
                     (srf1_ptr->u.ec_parameters.z_units ==
                      srf2_ptr->u.ec_parameters.z_units));

        case SRM_PS_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.ps_parameters.horizontal_datum ==
                      srf2_ptr->u.ps_parameters.horizontal_datum) &&
                     (srf1_ptr->u.ps_parameters.vertical_datum ==
                      srf2_ptr->u.ps_parameters.vertical_datum) &&
                     (srf1_ptr->u.ps_parameters.pole ==
                      srf2_ptr->u.ps_parameters.pole) &&
                     (srf1_ptr->u.ps_parameters.geodetic_longitude ==
                      srf2_ptr->u.ps_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.ps_parameters.central_scale_factor ==
                      srf2_ptr->u.ps_parameters.central_scale_factor) &&
                     (srf1_ptr->u.ps_parameters.z_units ==
                      srf2_ptr->u.ps_parameters.z_units) &&
                     (srf1_ptr->u.ps_parameters.x_offset ==
                      srf2_ptr->u.ps_parameters.x_offset) &&
                     (srf1_ptr->u.ps_parameters.y_offset ==
                      srf2_ptr->u.ps_parameters.y_offset));

        case SRM_LCC_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.lcc_parameters.horizontal_datum ==
                      srf2_ptr->u.lcc_parameters.horizontal_datum) &&
                     (srf1_ptr->u.lcc_parameters.vertical_datum ==
                      srf2_ptr->u.lcc_parameters.vertical_datum) &&
                     (srf1_ptr->u.lcc_parameters.north_parallel_geodetic_latitude ==
                      srf2_ptr->u.lcc_parameters.north_parallel_geodetic_latitude) &&
                     (srf1_ptr->u.lcc_parameters.south_parallel_geodetic_latitude ==
                      srf2_ptr->u.lcc_parameters.south_parallel_geodetic_latitude) &&
                     (srf1_ptr->u.lcc_parameters.geodetic_latitude ==
                      srf2_ptr->u.lcc_parameters.geodetic_latitude) &&
                     (srf1_ptr->u.lcc_parameters.geodetic_longitude ==
                      srf2_ptr->u.lcc_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.lcc_parameters.z_units ==
                      srf2_ptr->u.lcc_parameters.z_units) &&
                     (srf1_ptr->u.lcc_parameters.x_offset ==
                      srf2_ptr->u.lcc_parameters.x_offset) &&
                     (srf1_ptr->u.lcc_parameters.y_offset ==
                      srf2_ptr->u.lcc_parameters.y_offset));

        case SRM_TM_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.tm_parameters.horizontal_datum ==
                      srf2_ptr->u.tm_parameters.horizontal_datum) &&
                     (srf1_ptr->u.tm_parameters.vertical_datum ==
                      srf2_ptr->u.tm_parameters.vertical_datum) &&
                     (srf1_ptr->u.tm_parameters.geodetic_latitude ==
                      srf2_ptr->u.tm_parameters.geodetic_latitude) &&
                     (srf1_ptr->u.tm_parameters.geodetic_longitude ==
                      srf2_ptr->u.tm_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.tm_parameters.x_offset ==
                      srf2_ptr->u.tm_parameters.x_offset) &&
                     (srf1_ptr->u.tm_parameters.y_offset ==
                      srf2_ptr->u.tm_parameters.y_offset) &&
                     (srf1_ptr->u.tm_parameters.central_scale_factor ==
                      srf2_ptr->u.tm_parameters.central_scale_factor) &&
                     (srf1_ptr->u.tm_parameters.z_units ==
                      srf2_ptr->u.tm_parameters.z_units));

        case SRM_UTM_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.utm_parameters.horizontal_datum ==
                      srf2_ptr->u.utm_parameters.horizontal_datum) &&
                     (srf1_ptr->u.utm_parameters.vertical_datum ==
                      srf2_ptr->u.utm_parameters.vertical_datum) &&
                     (srf1_ptr->u.utm_parameters.x_offset ==
                      srf2_ptr->u.utm_parameters.x_offset) &&
                     (srf1_ptr->u.utm_parameters.y_offset ==
                      srf2_ptr->u.utm_parameters.y_offset) &&
                     (srf1_ptr->u.utm_parameters.z_units ==
                      srf2_ptr->u.utm_parameters.z_units) &&
                     (srf1_ptr->u.utm_parameters.zone ==
                      srf2_ptr->u.utm_parameters.zone));

        case SRM_LTP_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.ltp_parameters.horizontal_datum ==
                      srf2_ptr->u.ltp_parameters.horizontal_datum) &&
                     (srf1_ptr->u.ltp_parameters.vertical_datum ==
                      srf2_ptr->u.ltp_parameters.vertical_datum) &&
                     (srf1_ptr->u.ltp_parameters.geodetic_longitude ==
                      srf2_ptr->u.ltp_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.ltp_parameters.geodetic_latitude ==
                      srf2_ptr->u.ltp_parameters.geodetic_latitude) &&
                     (srf1_ptr->u.ltp_parameters.azimuth ==
                      srf2_ptr->u.ltp_parameters.azimuth) &&
                     (srf1_ptr->u.ltp_parameters.x_offset ==
                      srf2_ptr->u.ltp_parameters.x_offset) &&
                     (srf1_ptr->u.ltp_parameters.y_offset ==
                      srf2_ptr->u.ltp_parameters.y_offset) &&
                     (srf1_ptr->u.ltp_parameters.z_units ==
                      srf2_ptr->u.ltp_parameters.z_units));

        case SRM_LSR_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.lsr_parameters.up_direction ==
                      srf2_ptr->u.lsr_parameters.up_direction) &&
                     (srf1_ptr->u.lsr_parameters.forward_direction ==
                      srf2_ptr->u.lsr_parameters.forward_direction));

        case SRM_M_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.m_parameters.horizontal_datum ==
                      srf2_ptr->u.m_parameters.horizontal_datum) &&
                     (srf1_ptr->u.m_parameters.vertical_datum ==
                      srf2_ptr->u.m_parameters.vertical_datum) &&
                     (srf1_ptr->u.m_parameters.geodetic_longitude ==
                      srf2_ptr->u.m_parameters.geodetic_longitude) &&
                     (srf1_ptr->u.m_parameters.central_scale_factor ==
                      srf2_ptr->u.m_parameters.central_scale_factor) &&
                     (srf1_ptr->u.m_parameters.x_offset ==
                      srf2_ptr->u.m_parameters.x_offset) &&
                     (srf1_ptr->u.m_parameters.y_offset ==
                      srf2_ptr->u.m_parameters.y_offset) &&
                     (srf1_ptr->u.m_parameters.z_units ==
                      srf2_ptr->u.m_parameters.z_units));

        case SRM_OM_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.om_parameters.horizontal_datum ==
                      srf2_ptr->u.om_parameters.horizontal_datum) &&
                     (srf1_ptr->u.om_parameters.vertical_datum ==
                      srf2_ptr->u.om_parameters.vertical_datum) &&
                     (srf1_ptr->u.om_parameters.geodetic_latitude1 ==
                      srf2_ptr->u.om_parameters.geodetic_latitude1) &&
                     (srf1_ptr->u.om_parameters.geodetic_longitude1 ==
                      srf2_ptr->u.om_parameters.geodetic_longitude1) &&
                     (srf1_ptr->u.om_parameters.geodetic_latitude2 ==
                      srf2_ptr->u.om_parameters.geodetic_latitude2) &&
                     (srf1_ptr->u.om_parameters.geodetic_longitude2 ==
                      srf2_ptr->u.om_parameters.geodetic_longitude2) &&
                     (srf1_ptr->u.om_parameters.x_offset ==
                      srf2_ptr->u.om_parameters.x_offset) &&
                     (srf1_ptr->u.om_parameters.y_offset ==
                      srf2_ptr->u.om_parameters.y_offset) &&
                     (srf1_ptr->u.om_parameters.central_scale_factor ==
                      srf2_ptr->u.om_parameters.central_scale_factor) &&
                     (srf1_ptr->u.om_parameters.z_units ==
                      srf2_ptr->u.om_parameters.z_units));

        case SRM_UPS_3D_SRF:
             return (SE_BOOLEAN)
                    ((srf1_ptr->u.ups_parameters.horizontal_datum ==
                      srf2_ptr->u.ups_parameters.horizontal_datum) &&
                     (srf1_ptr->u.ups_parameters.vertical_datum ==
                      srf2_ptr->u.ups_parameters.vertical_datum) &&
                     (srf1_ptr->u.ups_parameters.pole ==
                      srf2_ptr->u.ups_parameters.pole) &&
                     (srf1_ptr->u.ups_parameters.z_units ==
                      srf2_ptr->u.ups_parameters.z_units) &&
                     (srf1_ptr->u.ups_parameters.x_offset ==
                      srf2_ptr->u.ups_parameters.x_offset) &&
                     (srf1_ptr->u.ups_parameters.y_offset ==
                      srf2_ptr->u.ups_parameters.y_offset));

        default:
#ifdef CONV_DEBUG
        {
             FILE *f_ptr;

             f_ptr = SE_GetErrorFilePtr();
             fprintf(f_ptr, "[SEDRIS CONVERSIONS] "\
                     "SRM_SRF3DParametersEqual: unknown SRF %s encountered\n",
                    SRM_PrintSRF3D(srf1_ptr->spatial_reference_frame));
        }
#endif
             return SE_FALSE;
    }
} /* end SRM_SRF3DParametersEqual */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_SRFParametersEqual
 *
 *-----------------------------------------------------------------------------
 */
SE_BOOLEAN
SRM_SRFParametersEqual
(
    const SE_SRF_PARAMETERS *params1_ptr,
    const SE_SRF_PARAMETERS *params2_ptr
)
{
    /*
     * If the spatial reference frames are different, we know they're not equal
     */
    if (!params1_ptr || !params2_ptr ||
        (params1_ptr->is_2D != params2_ptr->is_2D))
        return SE_FALSE;

    /*
     * Compare the coordinates
     */
    switch (params1_ptr->is_2D)
    {
        case SE_FALSE:
             return SRM_SRF3DParametersEqual(&(params1_ptr->u.params_3d),
                                             &(params2_ptr->u.params_3d));
        case SE_TRUE:
        default:
             return SRM_SRF2DParametersEqual(&(params1_ptr->u.params_2d),
                                             &(params2_ptr->u.params_2d));
    } /* end switch */
} /* end SRM_SRFParametersEqual */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_GetUnits
 *
 *-----------------------------------------------------------------------------
 */
SRM_ELEVATION_UNITS_ENUM
SE_GetUnits
(
    const SRM_SRF_3D_PARAMETERS *params_ptr
)
{
    if (!params_ptr)
    {
#ifdef CONV_DEBUG
        FILE *f_ptr;

        f_ptr = SE_GetErrorFilePtr();
        fprintf(f_ptr, "[SEDRIS CONVERSIONS] SE_GetUnits: given NULL "\
                "parameter, defaulting to SRM_METERS\n");
#endif
        return SRM_METERS;
    }

    switch (params_ptr->spatial_reference_frame)
    {
        case SRM_GD_3D_SRF:
             return params_ptr->u.gd_parameters.elevation_units;

        case SRM_GC_3D_SRF:
             return SRM_METERS;

        case SRM_GM_3D_SRF:
             return SRM_METERS;

        case SRM_GEI_3D_SRF:
             return SRM_METERS;

        case SRM_GSE_3D_SRF:
             return SRM_METERS;

        case SRM_GSM_3D_SRF:
             return SRM_METERS;

        case SRM_SM_3D_SRF:
             return SRM_METERS;

        case SRM_GCS_3D_SRF:
             return SRM_METERS;

        case SRM_EC_3D_SRF:
             return params_ptr->u.ec_parameters.z_units;

        case SRM_PS_3D_SRF:
             return params_ptr->u.ps_parameters.z_units;

        case SRM_LCC_3D_SRF:
             return params_ptr->u.lcc_parameters.z_units;

        case SRM_TM_3D_SRF:
             return params_ptr->u.tm_parameters.z_units;

        case SRM_UTM_3D_SRF:
             return params_ptr->u.utm_parameters.z_units;

        case SRM_LTP_3D_SRF:
             return params_ptr->u.ltp_parameters.z_units;

        case SRM_LSR_3D_SRF:
             return SRM_METERS;

        case SRM_M_3D_SRF:
             return params_ptr->u.m_parameters.z_units;

        case SRM_OM_3D_SRF:
             return params_ptr->u.om_parameters.z_units;

        case SRM_UPS_3D_SRF:
             return params_ptr->u.ups_parameters.z_units;

        default:
#ifdef CONV_DEBUG
        {
             FILE *f_ptr;

             f_ptr = SE_GetErrorFilePtr();
             fprintf(f_ptr, "[SEDRIS CONVERSIONS] SE_GetUnits: unknown"\
                    " SRF %s encountered, defaulting to meters\n",
                    SRM_PrintSRF3D(params_ptr->spatial_reference_frame));
        }
#endif
             return SRM_METERS;
    }
} /* end SE_GetUnits */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_GetCorresponding2DSRF
 *
 *-----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SRM_GetCorresponding2DSRF
(
    SRM_SRF_3D_ENUM  srf_3D,
    SRM_SRF_2D_ENUM *srf_2D_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (!srf_2D_ptr)
        return SE_COORD_FAILURE;

    switch (srf_3D)
    {
        case SRM_GD_3D_SRF:
             *srf_2D_ptr = SRM_GD_2D_SRF;
             break;
        case SRM_EC_3D_SRF:
             *srf_2D_ptr = SRM_EC_2D_SRF;
             break;
        case SRM_PS_3D_SRF:
             *srf_2D_ptr = SRM_PS_2D_SRF;
             break;
        case SRM_LCC_3D_SRF:
             *srf_2D_ptr = SRM_LCC_2D_SRF;
             break;
        case SRM_TM_3D_SRF:
             *srf_2D_ptr = SRM_TM_2D_SRF;
             break;
        case SRM_UTM_3D_SRF:
             *srf_2D_ptr = SRM_UTM_2D_SRF;
             break;
        case SRM_LTP_3D_SRF:
             *srf_2D_ptr = SRM_LTP_2D_SRF;
             break;
        case SRM_LSR_3D_SRF:
             *srf_2D_ptr = SRM_LSR_2D_SRF;
             break;
        case SRM_M_3D_SRF:
             *srf_2D_ptr = SRM_M_2D_SRF;
             break;
        case SRM_OM_3D_SRF:
             *srf_2D_ptr = SRM_OM_2D_SRF;
             break;
        case SRM_UPS_3D_SRF:
             *srf_2D_ptr = SRM_UPS_2D_SRF;
             break;
        default:
             status = SE_COORD_FAILURE;
             break;
    }
    return status;
} /* end SRM_GetCorresponding2DSRF */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_GetCorresponding3DSRF
 *
 *-----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SRM_GetCorresponding3DSRF
(
    SRM_SRF_2D_ENUM  srf_2D,
    SRM_SRF_3D_ENUM *srf_3D_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (!srf_3D_ptr)
        return SE_COORD_FAILURE;

    switch (srf_2D)
    {
        case SRM_GD_2D_SRF:
             *srf_3D_ptr = SRM_GD_3D_SRF;
             break;
        case SRM_EC_2D_SRF:
             *srf_3D_ptr = SRM_EC_3D_SRF;
             break;
        case SRM_PS_2D_SRF:
             *srf_3D_ptr = SRM_PS_3D_SRF;
             break;
        case SRM_LCC_2D_SRF:
             *srf_3D_ptr = SRM_LCC_3D_SRF;
             break;
        case SRM_TM_2D_SRF:
             *srf_3D_ptr = SRM_TM_3D_SRF;
             break;
        case SRM_UTM_2D_SRF:
             *srf_3D_ptr = SRM_UTM_3D_SRF;
             break;
        case SRM_LTP_2D_SRF:
             *srf_3D_ptr = SRM_LTP_3D_SRF;
             break;
        case SRM_LSR_2D_SRF:
             *srf_3D_ptr = SRM_LSR_3D_SRF;
             break;
        case SRM_M_2D_SRF:
             *srf_3D_ptr = SRM_M_3D_SRF;
             break;
        case SRM_OM_2D_SRF:
             *srf_3D_ptr = SRM_OM_3D_SRF;
             break;
        case SRM_UPS_2D_SRF:
             *srf_3D_ptr = SRM_UPS_3D_SRF;
             break;
        default:
             status = SE_COORD_FAILURE;
             break;
    }
    return status;
} /* end SRM_GetCorresponding3DSRF */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix3X3Add
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_Matrix3X3Add
(
    const SE_MATRIX_3X3_TYPE *a_ptr,
    const SE_MATRIX_3X3_TYPE *b_ptr,
          SE_MATRIX_3X3_TYPE *result_ptr
)
{
    SE_UINT8 i, j;

    if (!a_ptr || !b_ptr)
        return SE_COORD_INVALID_SOURCE_MATRIX;
    else if (!result_ptr)
        return SE_COORD_INVALID_DEST_MATRIX;

    for (j = 0; j < 3; j++)
    {
        for (i = 0; i < 3; i++)
        {
            result_ptr->mat[i][j] = a_ptr->mat[i][j] + b_ptr->mat[i][j];
        }
    }
    return SE_COORD_SUCCESS;
} /* end SE_Matrix3X3Add */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix3X3Multiply
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_Matrix3X3Multiply
(
    const SE_MATRIX_3X3_TYPE *a_ptr,
    const SE_MATRIX_3X3_TYPE *b_ptr,
          SE_MATRIX_3X3_TYPE *result_ptr
)
{
    SE_UINT8 i, j;

    if (!a_ptr || !b_ptr)
        return SE_COORD_INVALID_SOURCE_MATRIX;
    else if (!result_ptr)
        return SE_COORD_INVALID_DEST_MATRIX;

    for (j = 0; j < 3; j++)
    {
        for (i = 0; i < 3; i++)
        {
            result_ptr->mat[i][j] = a_ptr->mat[i][0] * b_ptr->mat[0][j] +
                                    a_ptr->mat[i][1] * b_ptr->mat[1][j] +
                                    a_ptr->mat[i][2] * b_ptr->mat[2][j];
        }
    }
    return SE_COORD_SUCCESS;
} /* end SE_Matrix3X3Multiply */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix3X3Sub
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_Matrix3X3Sub
(
    const SE_MATRIX_3X3_TYPE *a_ptr,
    const SE_MATRIX_3X3_TYPE *b_ptr,
          SE_MATRIX_3X3_TYPE *result_ptr
)
{
    SE_UINT8 i, j;

    if (!a_ptr || !b_ptr)
        return SE_COORD_INVALID_SOURCE_MATRIX;
    else if (!result_ptr)
        return SE_COORD_INVALID_DEST_MATRIX;

    for (j = 0; j < 3; j++)
    {
        for (i = 0; i < 3; i++)
        {
            result_ptr->mat[i][j] = a_ptr->mat[i][j] - b_ptr->mat[i][j];
        }
    }
    return SE_COORD_SUCCESS;
} /* end SE_Matrix3X3Sub */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix4X4Add
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_Matrix4X4Add
(
    const SE_MATRIX_4X4_TYPE *a_ptr,
    const SE_MATRIX_4X4_TYPE *b_ptr,
          SE_MATRIX_4X4_TYPE *result_ptr
)
{
    SE_UINT8 i, j;

    if (!a_ptr || !b_ptr)
        return SE_COORD_INVALID_SOURCE_MATRIX;
    else if (!result_ptr)
        return SE_COORD_INVALID_DEST_MATRIX;


    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < 4; i++)
        {
            result_ptr->mat[i][j] = a_ptr->mat[i][j] + b_ptr->mat[i][j];
        }
    }
    return SE_COORD_SUCCESS;
} /* end SE_Matrix4X4Add */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix4X4Multiply
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_Matrix4X4Multiply
(
    const SE_MATRIX_4X4_TYPE *a_ptr,
    const SE_MATRIX_4X4_TYPE *b_ptr,
          SE_MATRIX_4X4_TYPE *result_ptr
)
{
    SE_UINT8 i, j;

    if (!a_ptr || !b_ptr)
        return SE_COORD_INVALID_SOURCE_MATRIX;
    else if (!result_ptr)
        return SE_COORD_INVALID_DEST_MATRIX;


    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < 4; i++)
        {
            result_ptr->mat[i][j] = a_ptr->mat[i][0] * b_ptr->mat[0][j] +
                                    a_ptr->mat[i][1] * b_ptr->mat[1][j] +
                                    a_ptr->mat[i][2] * b_ptr->mat[2][j] +
                                    a_ptr->mat[i][3] * b_ptr->mat[3][j];
        }
    }
    return SE_COORD_SUCCESS;
} /* end SE_Matrix4X4Multiply */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Matrix4X4Sub
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_Matrix4X4Sub
(
    const SE_MATRIX_4X4_TYPE *a_ptr,
    const SE_MATRIX_4X4_TYPE *b_ptr,
          SE_MATRIX_4X4_TYPE *result_ptr
)
{
    SE_UINT8 i, j;

    if (!a_ptr || !b_ptr)
        return SE_COORD_INVALID_SOURCE_MATRIX;
    else if (!result_ptr)
        return SE_COORD_INVALID_DEST_MATRIX;


    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < 4; i++)
        {
            result_ptr->mat[i][j] = a_ptr->mat[i][j] - b_ptr->mat[i][j];
        }
    }
    return SE_COORD_SUCCESS;
} /* end SE_Matrix4X4Sub */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_CrossProduct
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM SE_CrossProduct
(
   const SE_VECTOR_3_TYPE *a_ptr,
   const SE_VECTOR_3_TYPE *b_ptr,
         SE_VECTOR_3_TYPE *result_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (!a_ptr || !b_ptr)
    {
        status = SE_COORD_INVALID_SOURCE_VECTOR;
    }
    else if (!result_ptr)
    {
        status = SE_COORD_INVALID_DEST_VECTOR;
    }
    else
    {
        result_ptr->vector[0] = a_ptr->vector[1] * b_ptr->vector[2] -
                                b_ptr->vector[1] * a_ptr->vector[2];
        result_ptr->vector[1] = a_ptr->vector[2] * b_ptr->vector[0] -
                                b_ptr->vector[2] * a_ptr->vector[0];
        result_ptr->vector[2] = a_ptr->vector[0] * b_ptr->vector[1] -
                                a_ptr->vector[1] * b_ptr->vector[0];
    }
    return status;
} /* end SE_CrossProduct */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_DotProduct
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM SE_DotProduct
(
   const SE_VECTOR_3_TYPE *a_ptr,
   const SE_VECTOR_3_TYPE *b_ptr,
         SE_FLOAT64       *result_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (!a_ptr || !b_ptr)
    {
        status = SE_COORD_INVALID_SOURCE_VECTOR;
    }
    else if (!result_ptr)
    {
        status = SE_COORD_FAILURE;
    }
    else
    {
        (*result_ptr) = (a_ptr->vector[0] * b_ptr->vector[0]) +
                        (a_ptr->vector[1] * b_ptr->vector[1]) +
                        (a_ptr->vector[2] * b_ptr->vector[2]);
    }
    return status;
} /* end SE_DotProduct */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_VectorMagnitude
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_VectorMagnitude
(
    const SE_VECTOR_3_TYPE *vec_ptr,
          SE_FLOAT64       *result_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (!vec_ptr)
    {
        status = SE_COORD_INVALID_SOURCE_VECTOR;
    }
    else if (!result_ptr)
    {
        status = SE_COORD_FAILURE;
    }
    else
    {
        *result_ptr = sqrt((vec_ptr->vector[0] * vec_ptr->vector[0]) +
                           (vec_ptr->vector[1] * vec_ptr->vector[1]) +
                           (vec_ptr->vector[2] * vec_ptr->vector[2]));
    }
    return status;
} /* end SE_VectorMagnitude */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_VectorMultByScalar
 *
 *----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM
SE_VectorMultByScalar
(
    const SE_VECTOR_3_TYPE *vec_ptr,
          SE_FLOAT64        scalar,
          SE_VECTOR_3_TYPE *result_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;

    if (!vec_ptr)
    {
        status = SE_COORD_INVALID_SOURCE_VECTOR;
    }
    else if (!result_ptr)
    {
        status = SE_COORD_INVALID_DEST_VECTOR;
    }
    else
    {
        result_ptr->vector[0] = vec_ptr->vector[0] * scalar;
        result_ptr->vector[1] = vec_ptr->vector[1] * scalar;
        result_ptr->vector[2] = vec_ptr->vector[2] * scalar;
    }
    return status;
} /* end SE_VectorMultByScalar */
