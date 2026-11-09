/*
 * SEDRIS Conversions API
 *
 * FILE       : se_datums.h
 *
 * PROGRAMMERS: Dan Toms (SRI International), Bill Horan (SAIC)
 *
 * DESCRIPTION:
 *   This file is for internal use only; SEDRIS users should not include
 *   this file.
 *
 * Release 3.0.3 - April 20, 2001
 *
 * For other release dates/detailed changes, contact SRI or SAIC.
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
 * Ensure that the SEDRIS Conversions API Datums are only included once.
 */
#ifndef _SRM_DATUMS_H_INCLUDED
#define _SRM_DATUMS_H_INCLUDED


/*
 * Allow C++ compilers to use this API without any problems
 */
#ifdef __cplusplus
extern "C" {
#endif


#include "se_conv_types.h"


#define DATUM_CODE_LENGTH           7


#define DATUM_NAME_LENGTH          52


#define ELLIPSOID_CODE_LENGTH       6 /* from 3 to 6 for spheroid code */


#define ELLIPSOID_NAME_LENGTH      45



/*
 * ENUM: SE_ELLIPSOID_ENUM
 *
 *   The list of reference ellipsoids used in conjunction with the Horizontal
 *   Datum, which is used for defining a spatial reference frame within SEDRIS.
 *   This list is taken from Appendix A of the Spatial Reference Model (SRM).
 *   See the SRM for details on how the selection of a horizontal datum affects
 *   a spatial reference frame within SEDRIS.  See Appendix A for the
 *   definitions of the Semi-major Axis and Inverse Flattening values for each
 *   of the listed ellipsoids.
 */
typedef enum
{
    SE_AA_ELLIPSOID, /* Airy 1830 Ellipsoid                               */
    SE_AM_ELLIPSOID, /* Modified Airy Ellipsoid                           */
    SE_AN_ELLIPSOID, /* Australian National Ellipsoid                     */
    SE_BN_ELLIPSOID, /* Bessel 1841 (Namibia) Ellipsoid                   */
    SE_BR_ELLIPSOID, /* Bessel 1841 (Ethiopia Indonesia Japan Korea)      */
                     /*             Ellipsoid                             */
    SE_CC_ELLIPSOID, /* Clarke 1866 Ellipsoid                             */
    SE_CD_ELLIPSOID, /* Clarke 1880 Ellipsoid                             */
    SE_EA_ELLIPSOID, /* Everest (India 1830) Ellipsoid                    */
    SE_EB_ELLIPSOID, /* Everest (Brunei and E. Malaysia  - Sabah &        */
                     /*          Sarawak) Ellipsoid                       */
    SE_EC_ELLIPSOID, /* Everest (India 1956) Ellipsoid                    */
    SE_ED_ELLIPSOID, /* Everest (West Malaysia 1969) Ellipsoid            */
    SE_EE_ELLIPSOID, /* Everest (West Malaysia & Singapore 1948) Ellipsoid*/
    SE_EF_ELLIPSOID, /* Everest (Pakistan) Ellipsoid                      */
    SE_FA_ELLIPSOID, /* Modified Fischer 1960 Ellipsoid                   */
    SE_HE_ELLIPSOID, /* Helmert 1906 Ellipsoid                            */
    SE_HO_ELLIPSOID, /* Hough 1960 Ellipsoid                              */
    SE_ID_ELLIPSOID, /* Indonesian 1974 Ellipsoid                         */
    SE_IN_ELLIPSOID, /* International 1924 Ellipsoid                      */
    SE_KA_ELLIPSOID, /* Krassovsky 1940 Ellipsoid                         */
    SE_RF_ELLIPSOID, /* Geodetic Reference System 1980 (GRS 80) Ellipsoid */
    SE_SA_ELLIPSOID, /* South American 1969 Ellipsoid                     */
    SE_WD_ELLIPSOID, /* WGS 72 Ellipsoid                                  */
    SE_WE_ELLIPSOID, /* WGS 84 Ellipsoid                                  */


    SE_ACM_SPHEROID, /* R = 6371221.3 m; used by ACMES */
    SE_COA_SPHEROID, /* R = 6371229 m; used by COAMPS  */
    SE_NOG_SPHEROID, /* R = 6371000 m; used by NOGAPS  */
    SE_MFE_SPHEROID, /* R = 6366707.02 m; MultiGen Flat Earth */
    SE_MOD_M_SPHEROID, /* R = 6371230 m; used by MODTRAN (Midlatitude)   */
    SE_MOD_S_SPHEROID, /* R = 6356910 m; used by MODTRAN (Subartic) */
    SE_MOD_T_SPHEROID, /* R = 6378390 m; used by MODTRAN (Tropical) */
    SE_MM5__SPHEROID,  /* R = 6370000 m; used by MM5 (AFWA)         */


    SE_UNSPECIFIED_ELLIPSOID /* Unspecified ellipsoid                     */
} SE_ELLIPSOID_ENUM;



/*
 * STRUCT: Datum_Row
 *
 *   Defines a single entry in the datum table.
 */
typedef struct Datum_Table_Row
{


  char              Datum_Code[DATUM_CODE_LENGTH];


  char              Datum_Name[DATUM_NAME_LENGTH];


  SE_ELLIPSOID_ENUM Ellipsoid_Code;


  double A;


  double F;                       /* inverse flattening */


  double Parameters[6];           /* delta/error X, Y, Z */
} Datum_Row;



extern Datum_Row Datum_Table[SE_NUM_HORIZONTAL_DATUMS];


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintEllipsoid
 *
 *-----------------------------------------------------------------------------
 */
extern const char *
SE_PrintEllipsoid
(
    SE_ELLIPSOID_ENUM value
);
#ifdef __cplusplus
}
#endif


#endif

