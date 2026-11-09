/*
 * SEDRIS Conversions API
 *
 * FILE       : se_conv_print.c
 *
 * PROGRAMMERS: Michele L. Worley (SAIC)
 *
 * DESCRIPTION: Provide functions for printing conversions API data
 *              structures.
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

#include "se_conv_print.h"

#ifdef WIN32
#undef SE_TURN_OFF_CONVERSIONS_WIN32_IMPORT
#endif

/*
 * GLOBAL VARIABLE: _OutHasBeenSet
 *
 *   Whether output file pointer has been set by the user. If
 *   SE_FALSE, use stdout.
 */
static SE_BOOLEAN _OutHasBeenSet = SE_FALSE;

/*
 * GLOBAL VARIABLE: conv_out_file_ptr
 *
 *   The output file pointer. If this has not been set, use stdout.
 */
static FILE *conv_out_file_ptr;

/*
 * GLOBAL VARIABLE: _MinWidth
 *
 *   Minimum field width. If the converted argument is smaller than
 *   _MinWidth, it will be padded on the left with spaces to make
 *   up the field width.
 */
static SE_UINT16 _MinWidth  = 12;

/*
 * GLOBAL_VARIABLE: _Precision
 *
 *   Number of digits to be printed after the decimal point, when printing
 *   a SE_FLOAT32 or SE_FLOAT64.
 */
static SE_UINT16 _Precision = 5;


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_SetOutFilePtr
 *
 *-----------------------------------------------------------------------------
 */
SE_COORD_STATUS_CODE_ENUM SE_SetOutFilePtr
(
    FILE *out_file_ptr
)
{
    SE_COORD_STATUS_CODE_ENUM status;

    if (out_file_ptr)
    {
        _OutHasBeenSet = SE_TRUE;
        conv_out_file_ptr = out_file_ptr;
        status = SE_COORD_SUCCESS;
    }
    else
    {
        status = SE_COORD_FAILURE;
    }
    return status;
} /* end SE_SetOutFilePtr */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_GetOutFilePtr
 *
 *-----------------------------------------------------------------------------
 */
FILE *
SE_GetOutFilePtr(void)
{
    if (_OutHasBeenSet)
        return conv_out_file_ptr;
    else
        return stdout;
} /* end SE_GetOutFilePtr */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_GetMinWidth
 *
 *-----------------------------------------------------------------------------
 */
SE_UINT16
SE_GetMinWidth(void)
{
    return _MinWidth;
} /* end SE_GetMinWidth */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_SetMinWidth
 *
 *-----------------------------------------------------------------------------
 */
void
SE_SetMinWidth
(
    SE_UINT16 min_width
)
{
    _MinWidth = min_width;
} /* end SE_SetMinWidth */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_GetPrecision
 *
 *-----------------------------------------------------------------------------
 */
SE_UINT16
SE_GetPrecision(void)
{
    return _Precision;
} /* end SE_GetPrecision */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_SetPrecision
 *
 *-----------------------------------------------------------------------------
 */
void
SE_SetPrecision
(
    SE_UINT16 precision
)
{
    _Precision = precision;
} /* end SE_SetPrecision */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_Indent
 *
 *    Indent the number of spaces appropriate for the given level.
 *
 * PARAMETERS:
 *
 *   level -- 1/2 the number of spaces to indent
 *
 *   out_file_ptr -- output file
 *
 * RETURNS:
 *
 *   nothing
 *
 *-----------------------------------------------------------------------------
 */
static void
SE_Indent
(
    SE_UINT16  level,
    FILE      *out_file_ptr
)
{
    SE_UINT16 i;

    for (i=1; i<level; i++)
        fprintf(out_file_ptr, "  ");
} /* end SE_Indent */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintBoolean
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintBoolean
(
    SE_BOOLEAN value
)
{
    if (value != 0)
        return "True";
    else
        return "False";
} /* end SE_PrintBoolean() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintChar
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintChar
(
    char value
)
{
    static char buffer[10];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%c", value);

    return buffer;
} /* end SE_PrintChar() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCMYData
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintCMYData
(
    const SE_CMY_DATA *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_CMY_DATA\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->cyan: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->cyan));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->magenta: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->magenta));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->yellow: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->yellow));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "cyan: %s\n", SE_PrintFloat64(value_ptr->cyan));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "magenta: %s\n", SE_PrintFloat64(value_ptr->magenta));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "yellow: %s\n", SE_PrintFloat64(value_ptr->yellow));
    }
} /* end SE_PrintCMYData */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCMYKData
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintCMYKData
(
    const SE_CMYK_DATA *value_ptr,
    const char         *name_ptr,
          SE_UINT16     level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_CMYK_DATA\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->cyan: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->cyan));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->magenta: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->magenta));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->yellow: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->yellow));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->black: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->black));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "cyan: %s\n", SE_PrintFloat64(value_ptr->cyan));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "magenta: %s\n", SE_PrintFloat64(value_ptr->magenta));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "yellow: %s\n", SE_PrintFloat64(value_ptr->yellow));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "black: %s\n", SE_PrintFloat64(value_ptr->black));
    }
} /* end SE_PrintCMYKData */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintColorData
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintColorData
(
    const SE_COLOR_DATA *value_ptr,
    const char          *name_ptr,
          SE_UINT16      level
)
{
    FILE *f_ptr;
    char  buffer[1024];

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_COLOR_DATA\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->color_model: %s\n", name_ptr,
                SE_PrintColorModel(value_ptr->color_model));
        switch (value_ptr->color_model)
        {
            case SE_RGB_MODEL:
                sprintf(buffer, "%s->u.rgb", name_ptr);
                SE_PrintRGBData(&(value_ptr->u.rgb), buffer, level);
                break;

            case SE_CMY_MODEL:
                sprintf(buffer, "%s->u.cmy", name_ptr);
                SE_PrintCMYData(&(value_ptr->u.cmy), buffer, level);
                break;

            case SE_HSV_MODEL:
                sprintf(buffer, "%s->u.hsv", name_ptr);
                SE_PrintHSVData(&(value_ptr->u.hsv), buffer, level);
                break;

        } /* end switch */
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "color_model: %s\n",
                SE_PrintColorModel(value_ptr->color_model));
        switch (value_ptr->color_model)
        {
            case SE_RGB_MODEL:
                sprintf(buffer, "u.rgb");
                SE_PrintRGBData(&(value_ptr->u.rgb), buffer, level);
                break;

            case SE_CMY_MODEL:
                sprintf(buffer, "u.cmy");
                SE_PrintCMYData(&(value_ptr->u.cmy), buffer, level);
                break;

            case SE_HSV_MODEL:
                sprintf(buffer, "u.hsv");
                SE_PrintHSVData(&(value_ptr->u.hsv), buffer, level);
                break;

        } /* end switch */
    }
} /* end SE_PrintColorData */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintColorModel
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintColorModel
(
    SE_COLOR_MODEL_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SE_RGB_MODEL:
             return "RGB Model";

        case SE_CMY_MODEL:
             return "CMY Model";

        case SE_HSV_MODEL:
             return "HSV Model";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SE_COLOR_MODEL_ENUM VALUE (%i)",
                    (int) value);
             break;
    } /* end switch */

    return buffer;
} /* end SE_PrintColorModel */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintColorStatusCode
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintColorStatusCode
(
    SE_COLOR_STATUS_CODE_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SE_COLOR_SUCCESS:
             return "SE_COLOR_SUCCESS";

        case SE_COLOR_NULL_PTR_OUT_PARAMETER:
             return "SE_COLOR_NULL_PTR_OUT_PARAMETER";

        case SE_COLOR_INVALID_ORIGINAL_COLOR:
             return "SE_COLOR_INVALID_ORIGINAL_COLOR";

        case SE_COLOR_INVALID_NEW_COLOR_MODEL:
             return "SE_COLOR_INVALID_NEW_COLOR_MODEL";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SE_COLOR_STATUS_CODE_ENUM VALUE"\
                    " (%i)",(int) value);
             break;
    } /* end switch */

    return buffer;
} /* end SE_PrintColorStatusCode */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintCoord
(
    const SE_COORD *value_ptr,
    const char     *name_ptr,
          SE_UINT16  level
)
{
    FILE *f_ptr;
    char  buffer[1024];

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->is_2D: %s\n", name_ptr,
                SE_PrintBoolean(value_ptr->is_2D));
        switch (value_ptr->is_2D)
        {
            case SE_FALSE:
                sprintf(buffer, "%s->u.coord_3d", name_ptr);
                SE_PrintCoord3D(&(value_ptr->u.coord_3d), buffer, level);
                break;

            case SE_TRUE:
                sprintf(buffer, "%s->u.coord_2d", name_ptr);
                SE_PrintCoord2D(&(value_ptr->u.coord_2d), buffer, level);
                break;

        } /* end switch */
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "is_2D: %s\n", SE_PrintBoolean(value_ptr->is_2D));
        switch (value_ptr->is_2D)
        {
            case SE_FALSE:
                sprintf(buffer, "u.coord_3d");
                SE_PrintCoord3D(&(value_ptr->u.coord_3d), buffer, level);
                break;

            case SE_TRUE:
                sprintf(buffer, "u.coord_2d");
                SE_PrintCoord2D(&(value_ptr->u.coord_2d), buffer, level);
                break;

        } /* end switch */
    }
} /* end SE_PrintCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCoord2D
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintCoord2D
(
    const SE_COORD_2D *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
)
{
    FILE *f_ptr;
    char  buffer[1024];

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_COORD_2D\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->spatial_reference_frame: %s\n", name_ptr,
                SRM_PrintSRF2D(value_ptr->spatial_reference_frame));
        switch (value_ptr->spatial_reference_frame)
        {
            case SRM_GD_2D_SRF:
                sprintf(buffer, "%s->u.gd_coord", name_ptr);
                SRM_PrintGD2DCoord(&(value_ptr->u.gd_coord), buffer, level);
                break;

            case SRM_EC_2D_SRF:
                sprintf(buffer, "%s->u.ec_coord", name_ptr);
                SRM_PrintEC2DCoord(&(value_ptr->u.ec_coord), buffer, level);
                break;

            case SRM_PS_2D_SRF:
                sprintf(buffer, "%s->u.ps_coord", name_ptr);
                SRM_PrintPS2DCoord(&(value_ptr->u.ps_coord), buffer, level);
                break;

            case SRM_LCC_2D_SRF:
                sprintf(buffer, "%s->u.lcc_coord", name_ptr);
                SRM_PrintLCC2DCoord(&(value_ptr->u.lcc_coord), buffer, level);
                break;

            case SRM_TM_2D_SRF:
                sprintf(buffer, "%s->u.tm_coord", name_ptr);
                SRM_PrintTM2DCoord(&(value_ptr->u.tm_coord), buffer, level);
                break;

            case SRM_UTM_2D_SRF:
                sprintf(buffer, "%s->u.utm_coord", name_ptr);
                SRM_PrintUTM2DCoord(&(value_ptr->u.utm_coord), buffer, level);
                break;

            case SRM_LTP_2D_SRF:
                sprintf(buffer, "%s->u.ltp_coord", name_ptr);
                SRM_PrintLTP2DCoord(&(value_ptr->u.ltp_coord), buffer, level);
                break;

            case SRM_LSR_2D_SRF:
                sprintf(buffer, "%s->u.lsr_coord", name_ptr);
                SRM_PrintLSR2DCoord(&(value_ptr->u.lsr_coord), buffer, level);
                break;

            case SRM_M_2D_SRF:
                sprintf(buffer, "%s->u.m_coord", name_ptr);
                SRM_PrintM2DCoord(&(value_ptr->u.m_coord), buffer, level);
                break;

            case SRM_OM_2D_SRF:
                sprintf(buffer, "%s->u.om_coord", name_ptr);
                SRM_PrintOM2DCoord(&(value_ptr->u.om_coord), buffer, level);
                break;

            case SRM_UPS_2D_SRF:
                sprintf(buffer, "%s->u.ups_coord", name_ptr);
                SRM_PrintUPS2DCoord(&(value_ptr->u.ups_coord), buffer, level);
                break;

        } /* end switch */
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "spatial_reference_frame: %s\n",
                SRM_PrintSRF2D(value_ptr->spatial_reference_frame));
        switch (value_ptr->spatial_reference_frame)
        {
            case SRM_GD_2D_SRF:
                sprintf(buffer, "u.gd_coord");
                SRM_PrintGD2DCoord(&(value_ptr->u.gd_coord), buffer, level);
                break;

            case SRM_EC_2D_SRF:
                sprintf(buffer, "u.ec_coord");
                SRM_PrintEC2DCoord(&(value_ptr->u.ec_coord), buffer, level);
                break;

            case SRM_PS_2D_SRF:
                sprintf(buffer, "u.ps_coord");
                SRM_PrintPS2DCoord(&(value_ptr->u.ps_coord), buffer, level);
                break;

            case SRM_LCC_2D_SRF:
                sprintf(buffer, "u.lcc_coord");
                SRM_PrintLCC2DCoord(&(value_ptr->u.lcc_coord), buffer, level);
                break;

            case SRM_TM_2D_SRF:
                sprintf(buffer, "u.tm_coord");
                SRM_PrintTM2DCoord(&(value_ptr->u.tm_coord), buffer, level);
                break;

            case SRM_UTM_2D_SRF:
                sprintf(buffer, "u.utm_coord");
                SRM_PrintUTM2DCoord(&(value_ptr->u.utm_coord), buffer, level);
                break;

            case SRM_LTP_2D_SRF:
                sprintf(buffer, "u.ltp_coord");
                SRM_PrintLTP2DCoord(&(value_ptr->u.ltp_coord), buffer, level);
                break;

            case SRM_LSR_2D_SRF:
                sprintf(buffer, "u.lsr_coord");
                SRM_PrintLSR2DCoord(&(value_ptr->u.lsr_coord), buffer, level);
                break;

            case SRM_M_2D_SRF:
                sprintf(buffer, "u.m_coord");
                SRM_PrintM2DCoord(&(value_ptr->u.m_coord), buffer, level);
                break;

            case SRM_OM_2D_SRF:
                sprintf(buffer, "u.om_coord");
                SRM_PrintOM2DCoord(&(value_ptr->u.om_coord), buffer, level);
                break;

            case SRM_UPS_2D_SRF:
                sprintf(buffer, "u.ups_coord");
                SRM_PrintUPS2DCoord(&(value_ptr->u.ups_coord), buffer, level);
                break;

        } /* end switch */
    }
} /* end SE_PrintCoord2D */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCoord3D
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintCoord3D
(
    const SE_COORD_3D *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
)
{
    FILE *f_ptr;
    char  buffer[1024];

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_COORD_3D\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->spatial_reference_frame: %s\n", name_ptr,
                SRM_PrintSRF3D(value_ptr->spatial_reference_frame));
        switch (value_ptr->spatial_reference_frame)
        {
            case SRM_GD_3D_SRF:
                sprintf(buffer, "%s->u.gd_coord", name_ptr);
                SRM_PrintGD3DCoord(&(value_ptr->u.gd_coord), buffer, level);
                break;

            case SRM_GC_3D_SRF:
                sprintf(buffer, "%s->u.gc_coord", name_ptr);
                SRM_PrintGC3DCoord(&(value_ptr->u.gc_coord), buffer, level);
                break;

            case SRM_GM_3D_SRF:
                sprintf(buffer, "%s->u.gm_coord", name_ptr);
                SRM_PrintGM3DCoord(&(value_ptr->u.gm_coord), buffer, level);
                break;

            case SRM_GEI_3D_SRF:
                sprintf(buffer, "%s->u.gei_coord", name_ptr);
                SRM_PrintGEI3DCoord(&(value_ptr->u.gei_coord), buffer, level);
                break;

            case SRM_GSE_3D_SRF:
                sprintf(buffer, "%s->u.gse_coord", name_ptr);
                SRM_PrintGSE3DCoord(&(value_ptr->u.gse_coord), buffer, level);
                break;

            case SRM_GSM_3D_SRF:
                sprintf(buffer, "%s->u.gsm_coord", name_ptr);
                SRM_PrintGSM3DCoord(&(value_ptr->u.gsm_coord), buffer, level);
                break;

            case SRM_SM_3D_SRF:
                sprintf(buffer, "%s->u.sm_coord", name_ptr);
                SRM_PrintSM3DCoord(&(value_ptr->u.sm_coord), buffer, level);
                break;

            case SRM_GCS_3D_SRF:
                sprintf(buffer, "%s->u.gcs_coord", name_ptr);
                SRM_PrintGCS3DCoord(&(value_ptr->u.gcs_coord), buffer, level);
                break;

            case SRM_EC_3D_SRF:
                sprintf(buffer, "%s->u.ec_coord", name_ptr);
                SRM_PrintEC3DCoord(&(value_ptr->u.ec_coord), buffer, level);
                break;

            case SRM_PS_3D_SRF:
                sprintf(buffer, "%s->u.ps_coord", name_ptr);
                SRM_PrintPS3DCoord(&(value_ptr->u.ps_coord), buffer, level);
                break;

            case SRM_LCC_3D_SRF:
                sprintf(buffer, "%s->u.lcc_coord", name_ptr);
                SRM_PrintLCC3DCoord(&(value_ptr->u.lcc_coord), buffer, level);
                break;

            case SRM_TM_3D_SRF:
                sprintf(buffer, "%s->u.tm_coord", name_ptr);
                SRM_PrintTM3DCoord(&(value_ptr->u.tm_coord), buffer, level);
                break;

            case SRM_UTM_3D_SRF:
                sprintf(buffer, "%s->u.utm_coord", name_ptr);
                SRM_PrintUTM3DCoord(&(value_ptr->u.utm_coord), buffer, level);
                break;

            case SRM_LTP_3D_SRF:
                sprintf(buffer, "%s->u.ltp_coord", name_ptr);
                SRM_PrintLTP3DCoord(&(value_ptr->u.ltp_coord), buffer, level);
                break;

            case SRM_LSR_3D_SRF:
                sprintf(buffer, "%s->u.lsr_coord", name_ptr);
                SRM_PrintLSR3DCoord(&(value_ptr->u.lsr_coord), buffer, level);
                break;

            case SRM_M_3D_SRF:
                sprintf(buffer, "%s->u.m_coord", name_ptr);
                SRM_PrintM3DCoord(&(value_ptr->u.m_coord), buffer, level);
                break;

            case SRM_OM_3D_SRF:
                sprintf(buffer, "%s->u.om_coord", name_ptr);
                SRM_PrintOM3DCoord(&(value_ptr->u.om_coord), buffer, level);
                break;

            case SRM_UPS_3D_SRF:
                sprintf(buffer, "%s->u.ups_coord", name_ptr);
                SRM_PrintUPS3DCoord(&(value_ptr->u.ups_coord), buffer, level);
                break;

        } /* end switch */
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "spatial_reference_frame: %s\n",
                SRM_PrintSRF3D(value_ptr->spatial_reference_frame));
        switch (value_ptr->spatial_reference_frame)
        {
            case SRM_GD_3D_SRF:
                sprintf(buffer, "u.gd_coord");
                SRM_PrintGD3DCoord(&(value_ptr->u.gd_coord), buffer, level);
                break;

            case SRM_GC_3D_SRF:
                sprintf(buffer, "u.gc_coord");
                SRM_PrintGC3DCoord(&(value_ptr->u.gc_coord), buffer, level);
                break;

            case SRM_GM_3D_SRF:
                sprintf(buffer, "u.gm_coord");
                SRM_PrintGM3DCoord(&(value_ptr->u.gm_coord), buffer, level);
                break;

            case SRM_GEI_3D_SRF:
                sprintf(buffer, "u.gei_coord");
                SRM_PrintGEI3DCoord(&(value_ptr->u.gei_coord), buffer, level);
                break;

            case SRM_GSE_3D_SRF:
                sprintf(buffer, "u.gse_coord");
                SRM_PrintGSE3DCoord(&(value_ptr->u.gse_coord), buffer, level);
                break;

            case SRM_GSM_3D_SRF:
                sprintf(buffer, "u.gsm_coord");
                SRM_PrintGSM3DCoord(&(value_ptr->u.gsm_coord), buffer, level);
                break;

            case SRM_SM_3D_SRF:
                sprintf(buffer, "u.sm_coord");
                SRM_PrintSM3DCoord(&(value_ptr->u.sm_coord), buffer, level);
                break;

            case SRM_GCS_3D_SRF:
                sprintf(buffer, "u.gcs_coord");
                SRM_PrintGCS3DCoord(&(value_ptr->u.gcs_coord), buffer, level);
                break;

            case SRM_EC_3D_SRF:
                sprintf(buffer, "u.ec_coord");
                SRM_PrintEC3DCoord(&(value_ptr->u.ec_coord), buffer, level);
                break;

            case SRM_PS_3D_SRF:
                sprintf(buffer, "u.ps_coord");
                SRM_PrintPS3DCoord(&(value_ptr->u.ps_coord), buffer, level);
                break;

            case SRM_LCC_3D_SRF:
                sprintf(buffer, "u.lcc_coord");
                SRM_PrintLCC3DCoord(&(value_ptr->u.lcc_coord), buffer, level);
                break;

            case SRM_TM_3D_SRF:
                sprintf(buffer, "u.tm_coord");
                SRM_PrintTM3DCoord(&(value_ptr->u.tm_coord), buffer, level);
                break;

            case SRM_UTM_3D_SRF:
                sprintf(buffer, "u.utm_coord");
                SRM_PrintUTM3DCoord(&(value_ptr->u.utm_coord), buffer, level);
                break;

            case SRM_LTP_3D_SRF:
                sprintf(buffer, "u.ltp_coord");
                SRM_PrintLTP3DCoord(&(value_ptr->u.ltp_coord), buffer, level);
                break;

            case SRM_LSR_3D_SRF:
                sprintf(buffer, "u.lsr_coord");
                SRM_PrintLSR3DCoord(&(value_ptr->u.lsr_coord), buffer, level);
                break;

            case SRM_M_3D_SRF:
                sprintf(buffer, "u.m_coord");
                SRM_PrintM3DCoord(&(value_ptr->u.m_coord), buffer, level);
                break;

            case SRM_OM_3D_SRF:
                sprintf(buffer, "u.om_coord");
                SRM_PrintOM3DCoord(&(value_ptr->u.om_coord), buffer, level);
                break;

            case SRM_UPS_3D_SRF:
                sprintf(buffer, "u.ups_coord");
                SRM_PrintUPS3DCoord(&(value_ptr->u.ups_coord), buffer, level);
                break;

        } /* end switch */
    }
} /* end SE_PrintCoord3D */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintCoordStatusCode
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintCoordStatusCode
(
    SE_COORD_STATUS_CODE_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SE_COORD_SUCCESS:
             return "SE_COORD_SUCCESS";

        case SE_COORD_INVALID_SOURCE_SRF_PARAMETERS:
             return "SE_COORD_INVALID_SOURCE_SRF_PARAMETERS";

        case SE_COORD_INVALID_DEST_SRF_PARAMETERS:
             return "SE_COORD_INVALID_DEST_SRF_PARAMETERS";

        case SE_COORD_INVALID_SRF_PAIR:
             return "SE_COORD_INVALID_SRF_PAIR";

        case SE_COORD_INVALID_SOURCE_COORD:
             return "SE_COORD_INVALID_SOURCE_COORD";

        case SE_COORD_EXTENDED_SOURCE_COORD:
             return "SE_COORD_EXTENDED_SOURCE_COORD";

        case SE_COORD_INVALID_DEST_COORD:
             return "SE_COORD_INVALID_DEST_COORD";

        case SE_COORD_EXTENDED_DEST_COORD:
             return "SE_COORD_EXTENDED_DEST_COORD";

        case SE_COORD_EXTENDED_SOURCE_AND_DEST_COORD:
             return "SE_COORD_EXTENDED_SOURCE_AND_DEST_COORD";

        case SE_COORD_UNSUPPORTED:
             return "SE_COORD_UNSUPPORTED";

        case SE_COORD_INVALID_SOURCE_VECTOR:
             return "SE_COORD_INVALID_SOURCE_VECTOR";

        case SE_COORD_INVALID_DEST_VECTOR:
             return "SE_COORD_INVALID_DEST_VECTOR";

        case SE_COORD_INVALID_SOURCE_MATRIX:
             return "SE_COORD_INVALID_SOURCE_MATRIX";

        case SE_COORD_INVALID_DEST_MATRIX:
             return "SE_COORD_INVALID_DEST_MATRIX";

        case SE_COORD_INVALID_DEST_COSINE_GAMMA:
             return "SE_COORD_INVALID_DEST_COSINE_GAMMA";

        case SE_COORD_INVALID_DEST_SINE_GAMMA:
             return "SE_COORD_INVALID_DEST_SINE_GAMMA";

        case SE_COORD_MISSING_WGS84_GEOID_SEPARATION_FILE:
             return "SE_COORD_MISSING_WGS84_GEOID_SEPARATION_FILE";

        case SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE:
             return "SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE";

        case SE_COORD_FAILURE:
             return "SE_COORD_FAILURE";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SE_COORD_STATUS_CODE_ENUM VALUE"\
                    " (%i)",(int) value);
             break;
    } /* end switch */

    return buffer;
} /* end SE_PrintCoordStatusCode */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintDirectionOfForward
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintDirectionOfForward
(
    SE_DIRECTION_OF_FORWARD_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SE_DIRECTION_OF_FORWARD_POSITIVE_PRIMARY_AXIS:
             return "Direction Of Forward Positive Primary Axis";

        case SE_DIRECTION_OF_FORWARD_POSITIVE_SECONDARY_AXIS:
             return "Direction Of Forward Positive Secondary Axis";

        case SE_DIRECTION_OF_FORWARD_POSITIVE_TERTIARY_AXIS:
             return "Direction Of Forward Positive Tertiary Axis";

        case SE_DIRECTION_OF_FORWARD_NEGATIVE_PRIMARY_AXIS:
             return "Direction Of Forward Negative Primary Axis";

        case SE_DIRECTION_OF_FORWARD_NEGATIVE_SECONDARY_AXIS:
             return "Direction Of Forward Negative Secondary Axis";

        case SE_DIRECTION_OF_FORWARD_NEGATIVE_TERTIARY_AXIS:
             return "Direction Of Forward Negative Tertiary Axis";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SE_DIRECTION_OF_FORWARD_ENUM"\
                    " VALUE (%i)",(int) value);
             break;
    } /* end switch */

    return buffer;
} /* end SE_PrintDirectionOfForward */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintDirectionOfUp
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintDirectionOfUp
(
    SE_DIRECTION_OF_UP_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SE_DIRECTION_OF_UP_POSITIVE_PRIMARY_AXIS:
             return "Direction Of Up Positive Primary Axis";

        case SE_DIRECTION_OF_UP_POSITIVE_SECONDARY_AXIS:
             return "Direction Of Up Positive Secondary Axis";

        case SE_DIRECTION_OF_UP_POSITIVE_TERTIARY_AXIS:
             return "Direction Of Up Positive Tertiary Axis";

        case SE_DIRECTION_OF_UP_NEGATIVE_PRIMARY_AXIS:
             return "Direction Of Up Negative Primary Axis";

        case SE_DIRECTION_OF_UP_NEGATIVE_SECONDARY_AXIS:
             return "Direction Of Up Negative Secondary Axis";

        case SE_DIRECTION_OF_UP_NEGATIVE_TERTIARY_AXIS:
             return "Direction Of Up Negative Tertiary Axis";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SE_DIRECTION_OF_UP_ENUM VALUE"\
                    " (%i)",(int) value);
             break;
    } /* end switch */

    return buffer;
} /* end SE_PrintDirectionOfUp */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintEC2DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintEC2DCoord
(
    const SRM_EC_2D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_EC_2D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
    }
} /* end SRM_PrintEC2DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintEC2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintEC2DParameters
(
    const SRM_EC_2D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_EC_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->central_scale_factor: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->central_scale_factor));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "central_scale_factor: %s\n",
                SE_PrintFloat64(value_ptr->central_scale_factor));
    }
} /* end SRM_PrintEC2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintEC3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintEC3DCoord
(
    const SRM_EC_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_EC_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintEC3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintEC3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintEC3DParameters
(
    const SRM_EC_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_EC_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->vertical_datum: %s\n", name_ptr,
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->central_scale_factor: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z_units: %s\n", name_ptr,
                SRM_PrintElevationUnits(value_ptr->z_units));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "vertical_datum: %s\n",
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "central_scale_factor: %s\n",
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z_units: %s\n",
                SRM_PrintElevationUnits(value_ptr->z_units));
    }
} /* end SRM_PrintEC3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintElevationUnits
 *
 *-----------------------------------------------------------------------------
 */
const char *
SRM_PrintElevationUnits
(
    SRM_ELEVATION_UNITS_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SRM_FEET:
             return "feet";

        case SRM_METERS:
             return "meters";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SRM_ELEVATION_UNITS_ENUM VALUE"\
                    " (%i)",(int) value);
             break;
    } /* end switch */

    return buffer;
} /* end SRM_PrintElevationUnits */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintFloat32
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintFloat32
(
    SE_FLOAT32 value
)
{
    static char buffer[100];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%*.*f",(int)_MinWidth, (int)_Precision, (double)value);

    return buffer;
} /* end SE_PrintFloat32() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintFloat64
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintFloat64
(
    SE_FLOAT64 value
)
{
    static char buffer[100];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%*.*f",(int)_MinWidth, (int)_Precision, (double)value);

    return buffer;
} /* end SE_PrintFloat64() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGC3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGC3DCoord
(
    const SRM_GC_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GC_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintGC3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGC3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGC3DParameters
(
    const SRM_GC_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GC_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->ignore_na: N/A\n", name_ptr);
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "ignore_na: N/A\n");
    }
} /* end SRM_PrintGC3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGCS3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGCS3DCoord
(
    const SRM_GCS_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GCS_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->cell_ID: %s\n", name_ptr,
                SE_PrintPint16(value_ptr->cell_ID));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "cell_ID: %s\n", SE_PrintPint16(value_ptr->cell_ID));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintGCS3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGCS3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGCS3DParameters
(
    const SRM_GCS_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GCS_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
    }
} /* end SRM_PrintGCS3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGD2DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGD2DCoord
(
    const SRM_GD_2D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GD_2D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude));
    }
} /* end SRM_PrintGD2DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGD2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGD2DParameters
(
    const SRM_GD_2D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GD_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
    }
} /* end SRM_PrintGD2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGD3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGD3DCoord
(
    const SRM_GD_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GD_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->elevation: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->elevation));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "elevation: %s\n",
                SE_PrintFloat64(value_ptr->elevation));
    }
} /* end SRM_PrintGD3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGD3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGD3DParameters
(
    const SRM_GD_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GD_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->vertical_datum: %s\n", name_ptr,
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->elevation_units: %s\n", name_ptr,
                SRM_PrintElevationUnits(value_ptr->elevation_units));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "vertical_datum: %s\n",
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "elevation_units: %s\n",
                SRM_PrintElevationUnits(value_ptr->elevation_units));
    }
} /* end SRM_PrintGD3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGEI3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGEI3DCoord
(
    const SRM_GEI_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GEI_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->right_ascension: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->right_ascension));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->declination: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->declination));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->radius: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->radius));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "right_ascension: %s\n",
                SE_PrintFloat64(value_ptr->right_ascension));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "declination: %s\n",
                SE_PrintFloat64(value_ptr->declination));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "radius: %s\n", SE_PrintFloat64(value_ptr->radius));
    }
} /* end SRM_PrintGEI3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGEI3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGEI3DParameters
(
    const SRM_GEI_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GEI_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->epoch: %s\n", name_ptr,
                SE_PrintGEIEpoch(value_ptr->epoch));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "epoch: %s\n", SE_PrintGEIEpoch(value_ptr->epoch));
    }
} /* end SRM_PrintGEI3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintGEIEpoch
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintGEIEpoch
(
    SE_GEI_EPOCH_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SE_ATD_EPOCH:
             return "ATD Epoch";

        case SE_M50_EPOCH:
             return "M50 Epoch";

        case SE_FK5_J2000_EPOCH:
             return "FK5 J2000 Epoch";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SE_GEI_EPOCH_ENUM VALUE (%i)",
                    (int) value);
             break;
    } /* end switch */

    return buffer;
} /* end SE_PrintGEIEpoch */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGM3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGM3DCoord
(
    const SRM_GM_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GM_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->gm_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->gm_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->gm_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->gm_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->radius: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->radius));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "gm_latitude: %s\n",
                SE_PrintFloat64(value_ptr->gm_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "gm_longitude: %s\n",
                SE_PrintFloat64(value_ptr->gm_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "radius: %s\n", SE_PrintFloat64(value_ptr->radius));
    }
} /* end SRM_PrintGM3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGM3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGM3DParameters
(
    const SRM_GM_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GM_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->gm_epoch: %s\n", name_ptr,
                SE_PrintUint16(value_ptr->gm_epoch));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->gm_earth_radius: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->gm_earth_radius));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "gm_epoch: %s\n", SE_PrintUint16(value_ptr->gm_epoch));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "gm_earth_radius: %s\n",
                SE_PrintFloat64(value_ptr->gm_earth_radius));
    }
} /* end SRM_PrintGM3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGSE3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGSE3DCoord
(
    const SRM_GSE_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GSE_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->radius: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->radius));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "longitude: %s\n",
                SE_PrintFloat64(value_ptr->longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "latitude: %s\n", SE_PrintFloat64(value_ptr->latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "radius: %s\n", SE_PrintFloat64(value_ptr->radius));
    }
} /* end SRM_PrintGSE3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGSE3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGSE3DParameters
(
    const SRM_GSE_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GSE_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->ignore_na: N/A\n", name_ptr);
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "ignore_na: N/A\n");
    }
} /* end SRM_PrintGSE3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGSM3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGSM3DCoord
(
    const SRM_GSM_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GSM_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->radius: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->radius));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "longitude: %s\n",
                SE_PrintFloat64(value_ptr->longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "latitude: %s\n", SE_PrintFloat64(value_ptr->latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "radius: %s\n", SE_PrintFloat64(value_ptr->radius));
    }
} /* end SRM_PrintGSM3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintGSM3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintGSM3DParameters
(
    const SRM_GSM_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_GSM_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->ignore_na: N/A\n", name_ptr);
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "ignore_na: N/A\n");
    }
} /* end SRM_PrintGSM3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintHemisphere
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintHemisphere
(
    SE_HEMISPHERE_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SE_NORTHERN_HEMISPHERE:
             return "NORTHERN HEMISPHERE";

        case SE_SOUTHERN_HEMISPHERE:
             return "SOUTHERN HEMISPHERE";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SE_HEMISPHERE_ENUM VALUE (%i)",
                    (int) value);
             break;
    } /* end switch */

    return buffer;
} /* end SE_PrintHemisphere */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintHLSData
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintHLSData
(
    const SE_HLS_DATA *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_HLS_DATA\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->hue: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->hue));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->lightness: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->lightness));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->saturation: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->saturation));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "hue: %s\n", SE_PrintFloat64(value_ptr->hue));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "lightness: %s\n",
                SE_PrintFloat64(value_ptr->lightness));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "saturation: %s\n",
                SE_PrintFloat64(value_ptr->saturation));
    }
} /* end SE_PrintHLSData */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintHorizontalDatum
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintHorizontalDatum
(
    SE_HORIZONTAL_DATUM_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SE_ADI_A_HDATUM:
             return "ADI A HDatum";

        case SE_ADI_B_HDATUM:
             return "ADI B HDatum";

        case SE_ADI_C_HDATUM:
             return "ADI C HDatum";

        case SE_ADI_D_HDATUM:
             return "ADI D HDatum";

        case SE_ADI_E_HDATUM:
             return "ADI E HDatum";

        case SE_ADI_F_HDATUM:
             return "ADI F HDatum";

        case SE_ADI_M_HDATUM:
             return "ADI M HDatum";

        case SE_AFG_HDATUM:
             return "AFG HDatum";

        case SE_AIA_HDATUM:
             return "AIA HDatum";

        case SE_AIN_A_HDATUM:
             return "AIN A HDatum";

        case SE_AIN_B_HDATUM:
             return "AIN B HDatum";

        case SE_ANO_HDATUM:
             return "ANO HDatum";

        case SE_ARF_A_HDATUM:
             return "ARF A HDatum";

        case SE_ARF_B_HDATUM:
             return "ARF B HDatum";

        case SE_ARF_C_HDATUM:
             return "ARF C HDatum";

        case SE_ARF_D_HDATUM:
             return "ARF D HDatum";

        case SE_ARF_E_HDATUM:
             return "ARF E HDatum";

        case SE_ARF_F_HDATUM:
             return "ARF F HDatum";

        case SE_ARF_G_HDATUM:
             return "ARF G HDatum";

        case SE_ARF_H_HDATUM:
             return "ARF H HDatum";

        case SE_ARF_M_HDATUM:
             return "ARF M HDatum";

        case SE_ARS_M_HDATUM:
             return "ARS M HDatum";

        case SE_ASC_HDATUM:
             return "ASC HDatum";

        case SE_ASM_HDATUM:
             return "ASM HDatum";

        case SE_ASQ_HDATUM:
             return "ASQ HDatum";

        case SE_ATF_HDATUM:
             return "ATF HDatum";

        case SE_AUA_HDATUM:
             return "AUA HDatum";

        case SE_AUG_HDATUM:
             return "AUG HDatum";

        case SE_BAT_HDATUM:
             return "BAT HDatum";

        case SE_BER_HDATUM:
             return "BER HDatum";

        case SE_BID_HDATUM:
             return "BID HDatum";

        case SE_BOO_HDATUM:
             return "BOO HDatum";

        case SE_CAC_HDATUM:
             return "CAC HDatum";

        case SE_CAI_HDATUM:
             return "CAI HDatum";

        case SE_CAO_HDATUM:
             return "CAO HDatum";

        case SE_CAP_HDATUM:
             return "CAP HDatum";

        case SE_CGE_HDATUM:
             return "CGE HDatum";

        case SE_CHI_HDATUM:
             return "CHI HDatum";

        case SE_CHU_HDATUM:
             return "CHU HDatum";

        case SE_COA_HDATUM:
             return "COA HDatum";

        case SE_DOB_HDATUM:
             return "DOB HDatum";

        case SE_EAS_HDATUM:
             return "EAS HDatum";

        case SE_ENW_HDATUM:
             return "ENW HDatum";

        case SE_EUR_A_HDATUM:
             return "EUR A HDatum";

        case SE_EUR_B_HDATUM:
             return "EUR B HDatum";

        case SE_EUR_C_HDATUM:
             return "EUR C HDatum";

        case SE_EUR_D_HDATUM:
             return "EUR D HDatum";

        case SE_EUR_E_HDATUM:
             return "EUR E HDatum";

        case SE_EUR_F_HDATUM:
             return "EUR F HDatum";

        case SE_EUR_G_HDATUM:
             return "EUR G HDatum";

        case SE_EUR_H_HDATUM:
             return "EUR H HDatum";

        case SE_EUR_I_HDATUM:
             return "EUR I HDatum";

        case SE_EUR_J_HDATUM:
             return "EUR J HDatum";

        case SE_EUR_K_HDATUM:
             return "EUR K HDatum";

        case SE_EUR_L_HDATUM:
             return "EUR L HDatum";

        case SE_EUR_M_HDATUM:
             return "EUR M HDatum";

        case SE_EUS_HDATUM:
             return "EUS HDatum";

        case SE_FAH_HDATUM:
             return "FAH HDatum";

        case SE_FLO_HDATUM:
             return "FLO HDatum";

        case SE_FOT_HDATUM:
             return "FOT HDatum";

        case SE_GAA_HDATUM:
             return "GAA HDatum";

        case SE_GEO_HDATUM:
             return "GEO HDatum";

        case SE_GIZ_HDATUM:
             return "GIZ HDatum";

        case SE_GRA_HDATUM:
             return "GRA HDatum";

        case SE_GUA_HDATUM:
             return "GUA HDatum";

        case SE_HIT_HDATUM:
             return "HIT HDatum";

        case SE_HJO_HDATUM:
             return "HJO HDatum";

        case SE_HKD_HDATUM:
             return "HKD HDatum";

        case SE_HTN_HDATUM:
             return "HTN HDatum";

        case SE_IBE_HDATUM:
             return "IBE HDatum";

        case SE_IND_B_HDATUM:
             return "IND B HDatum";

        case SE_IND_I_HDATUM:
             return "IND I HDatum";

        case SE_INF_A_HDATUM:
             return "INF A HDatum";

        case SE_INH_A_HDATUM:
             return "INH A HDatum";

        case SE_IRL_HDATUM:
             return "IRL HDatum";

        case SE_ISG_HDATUM:
             return "ISG HDatum";

        case SE_IST_HDATUM:
             return "IST HDatum";

        case SE_JOH_HDATUM:
             return "JOH HDatum";

        case SE_KAN_HDATUM:
             return "KAN HDatum";

        case SE_KEA_HDATUM:
             return "KEA HDatum";

        case SE_KEG_HDATUM:
             return "KEG HDatum";

        case SE_KUS_HDATUM:
             return "KUS HDatum";

        case SE_LCF_HDATUM:
             return "LCF HDatum";

        case SE_LEH_HDATUM:
             return "LEH HDatum";

        case SE_LIB_HDATUM:
             return "LIB HDatum";

        case SE_LUZ_A_HDATUM:
             return "LUZ A HDatum";

        case SE_LUZ_B_HDATUM:
             return "LUZ B HDatum";

        case SE_MAS_HDATUM:
             return "MAS HDatum";

        case SE_MER_HDATUM:
             return "MER HDatum";

        case SE_MID_HDATUM:
             return "MID HDatum";

        case SE_MIK_HDATUM:
             return "MIK HDatum";

        case SE_MIN_A_HDATUM:
             return "MIN A HDatum";

        case SE_MIN_B_HDATUM:
             return "MIN B HDatum";

        case SE_MOD_HDATUM:
             return "MOD HDatum";

        case SE_MPO_HDATUM:
             return "MPO HDatum";

        case SE_MVS_HDATUM:
             return "MVS HDatum";

        case SE_NAH_A_HDATUM:
             return "NAH A HDatum";

        case SE_NAH_B_HDATUM:
             return "NAH B HDatum";

        case SE_NAH_C_HDATUM:
             return "NAH C HDatum";

        case SE_NAP_HDATUM:
             return "NAP HDatum";

        case SE_NAR_A_HDATUM:
             return "NAR A HDatum";

        case SE_NAR_B_HDATUM:
             return "NAR B HDatum";

        case SE_NAR_C_HDATUM:
             return "NAR C HDatum";

        case SE_NAR_D_HDATUM:
             return "NAR D HDatum";

        case SE_NAS_A_HDATUM:
             return "NAS A HDatum";

        case SE_NAS_B_HDATUM:
             return "NAS B HDatum";

        case SE_NAS_C_HDATUM:
             return "NAS C HDatum";

        case SE_NAS_D_HDATUM:
             return "NAS D HDatum";

        case SE_NAS_E_HDATUM:
             return "NAS E HDatum";

        case SE_NAS_F_HDATUM:
             return "NAS F HDatum";

        case SE_NAS_G_HDATUM:
             return "NAS G HDatum";

        case SE_NAS_H_HDATUM:
             return "NAS H HDatum";

        case SE_NAS_I_HDATUM:
             return "NAS I HDatum";

        case SE_NAS_J_HDATUM:
             return "NAS J HDatum";

        case SE_NAS_L_HDATUM:
             return "NAS L HDatum";

        case SE_NAS_N_HDATUM:
             return "NAS N HDatum";

        case SE_NAS_O_HDATUM:
             return "NAS O HDatum";

        case SE_NAS_P_HDATUM:
             return "NAS P HDatum";

        case SE_NAS_Q_HDATUM:
             return "NAS Q HDatum";

        case SE_NAS_R_HDATUM:
             return "NAS R HDatum";

        case SE_NAS_T_HDATUM:
             return "NAS T HDatum";

        case SE_NAS_U_HDATUM:
             return "NAS U HDatum";

        case SE_OEG_HDATUM:
             return "OEG HDatum";

        case SE_OGB_A_HDATUM:
             return "OGB A HDatum";

        case SE_OGB_B_HDATUM:
             return "OGB B HDatum";

        case SE_OGB_C_HDATUM:
             return "OGB C HDatum";

        case SE_OGB_D_HDATUM:
             return "OGB D HDatum";

        case SE_OGB_M_HDATUM:
             return "OGB M HDatum";

        case SE_OHA_A_HDATUM:
             return "OHA A HDatum";

        case SE_OHA_B_HDATUM:
             return "OHA B HDatum";

        case SE_OHA_C_HDATUM:
             return "OHA C HDatum";

        case SE_OHA_D_HDATUM:
             return "OHA D HDatum";

        case SE_OHA_M_HDATUM:
             return "OHA M HDatum";

        case SE_PHA_HDATUM:
             return "PHA HDatum";

        case SE_PIT_HDATUM:
             return "PIT HDatum";

        case SE_PLN_HDATUM:
             return "PLN HDatum";

        case SE_POS_HDATUM:
             return "POS HDatum";

        case SE_PRP_A_HDATUM:
             return "PRP A HDatum";

        case SE_PRP_B_HDATUM:
             return "PRP B HDatum";

        case SE_PRP_C_HDATUM:
             return "PRP C HDatum";

        case SE_PRP_D_HDATUM:
             return "PRP D HDatum";

        case SE_PRP_E_HDATUM:
             return "PRP E HDatum";

        case SE_PRP_F_HDATUM:
             return "PRP F HDatum";

        case SE_PRP_G_HDATUM:
             return "PRP G HDatum";

        case SE_PRP_H_HDATUM:
             return "PRP H HDatum";

        case SE_PRP_M_HDATUM:
             return "PRP M HDatum";

        case SE_PTB_HDATUM:
             return "PTB HDatum";

        case SE_PTN_HDATUM:
             return "PTN HDatum";

        case SE_PUR_HDATUM:
             return "PUR HDatum";

        case SE_QAT_HDATUM:
             return "QAT HDatum";

        case SE_QUO_HDATUM:
             return "QUO HDatum";

        case SE_REU_HDATUM:
             return "REU HDatum";

        case SE_SAE_HDATUM:
             return "SAE HDatum";

        case SE_SAN_A_HDATUM:
             return "SAN A HDatum";

        case SE_SAN_B_HDATUM:
             return "SAN B HDatum";

        case SE_SAN_C_HDATUM:
             return "SAN C HDatum";

        case SE_SAN_D_HDATUM:
             return "SAN D HDatum";

        case SE_SAN_E_HDATUM:
             return "SAN E HDatum";

        case SE_SAN_F_HDATUM:
             return "SAN F HDatum";

        case SE_SAN_G_HDATUM:
             return "SAN G HDatum";

        case SE_SAN_H_HDATUM:
             return "SAN H HDatum";

        case SE_SAN_I_HDATUM:
             return "SAN I HDatum";

        case SE_SAN_J_HDATUM:
             return "SAN J HDatum";

        case SE_SAN_K_HDATUM:
             return "SAN K HDatum";

        case SE_SAN_L_HDATUM:
             return "SAN L HDatum";

        case SE_SAN_M_HDATUM:
             return "SAN M HDatum";

        case SE_SAO_HDATUM:
             return "SAO HDatum";

        case SE_SAP_HDATUM:
             return "SAP HDatum";

        case SE_SCK_HDATUM:
             return "SCK HDatum";

        case SE_SGM_HDATUM:
             return "SGM HDatum";

        case SE_SHB_HDATUM:
             return "SHB HDatum";

        case SE_SOA_HDATUM:
             return "SOA HDatum";

        case SE_TDC_HDATUM:
             return "TDC HDatum";

        case SE_TIL_HDATUM:
             return "TIL HDatum";

        case SE_TOY_A_HDATUM:
             return "TOY A HDatum";

        case SE_TOY_B_HDATUM:
             return "TOY B HDatum";

        case SE_TOY_C_HDATUM:
             return "TOY C HDatum";

        case SE_TOY_M_HDATUM:
             return "TOY M HDatum";

        case SE_TRN_HDATUM:
             return "TRN HDatum";

        case SE_W72_HDATUM:
             return "W72 HDatum";

        case SE_W84_HDATUM:
             return "W84 HDatum";

        case SE_WAK_HDATUM:
             return "WAK HDatum";

        case SE_ZAN_HDATUM:
             return "ZAN HDatum";

        case SE_SPHERICAL_ACM_HDATUM:
             return "Spherical ACM HDatum";

        case SE_SPHERICAL_COA_HDATUM:
             return "Spherical COA HDatum";

        case SE_SPHERICAL_NOG_HDATUM:
             return "Spherical NOG HDatum";

        case SE_SPHERICAL_MFE_HDATUM:
             return "Spherical MFE HDatum";

        case SE_SPHERICAL_MOD_M_HDATUM:
             return "Spherical MOD M HDatum";

        case SE_SPHERICAL_MOD_S_HDATUM:
             return "Spherical MOD S HDatum";

        case SE_SPHERICAL_MOD_T_HDATUM:
             return "Spherical MOD T HDatum";

        case SE_SPHERICAL_MM5_HDATUM:
             return "Spherical MM5 HDatum";

        case SE_AMA_HDATUM:
             return "AMA HDatum";

        case SE_ARS_A_HDATUM:
             return "ARS A HDatum";

        case SE_ARS_B_HDATUM:
             return "ARS B HDatum";

        case SE_BUR_HDATUM:
             return "BUR HDatum";

        case SE_CAZ_HDATUM:
             return "CAZ HDatum";

        case SE_CCD_HDATUM:
             return "CCD HDatum";

        case SE_DAL_HDATUM:
             return "DAL HDatum";

        case SE_DID_HDATUM:
             return "DID HDatum";

        case SE_EST_HDATUM:
             return "EST HDatum";

        case SE_EUR_S_HDATUM:
             return "EUR S HDatum";

        case SE_EUR_T_HDATUM:
             return "EUR T HDatum";

        case SE_GSE_HDATUM:
             return "GSE HDatum";

        case SE_HEN_HDATUM:
             return "HEN HDatum";

        case SE_HER_HDATUM:
             return "HER HDatum";

        case SE_IDN_HDATUM:
             return "IDN HDatum";

        case SE_IND_P_HDATUM:
             return "IND P HDatum";

        case SE_ING_A_HDATUM:
             return "ING A HDatum";

        case SE_ING_B_HDATUM:
             return "ING B HDatum";

        case SE_NAR_E_HDATUM:
             return "NAR E HDatum";

        case SE_NAR_H_HDATUM:
             return "NAR H HDatum";

        case SE_NAS_V_HDATUM:
             return "NAS V HDatum";

        case SE_NAS_W_HDATUM:
             return "NAS W HDatum";

        case SE_NSD_HDATUM:
             return "NSD HDatum";

        case SE_PUK_HDATUM:
             return "PUK HDatum";

        case SE_SPK_A_HDATUM:
             return "SPK A HDatum";

        case SE_SPK_B_HDATUM:
             return "SPK B HDatum";

        case SE_SPK_C_HDATUM:
             return "SPK C HDatum";

        case SE_SPK_D_HDATUM:
             return "SPK D HDatum";

        case SE_SPK_E_HDATUM:
             return "SPK E HDatum";

        case SE_SPK_F_HDATUM:
             return "SPK F HDatum";

        case SE_SPK_G_HDATUM:
             return "SPK G HDatum";

        case SE_SRL_HDATUM:
             return "SRL HDatum";

        case SE_TAN_HDATUM:
             return "TAN HDatum";

        case SE_VOI_HDATUM:
             return "VOI HDatum";

        case SE_VOR_HDATUM:
             return "VOR HDatum";

        case SE_YAC_HDATUM:
             return "YAC HDatum";

        case SE_INH_A1_HDATUM:
             return "INH A1 HDatum";

        case SE_TOY_B1_HDATUM:
             return "TOY B1 HDatum";

        case SE_NUM_HORIZONTAL_DATUMS:
             return "Num Horizontal Datums";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SE_HORIZONTAL_DATUM_ENUM VALUE"\
                    " (%i)",(int) value);
             break;
    } /* end switch */

    return buffer;
} /* end SE_PrintHorizontalDatum */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintHSVData
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintHSVData
(
    const SE_HSV_DATA *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_HSV_DATA\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->hue: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->hue));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->saturation: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->saturation));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->value: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->value));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "hue: %s\n", SE_PrintFloat64(value_ptr->hue));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "saturation: %s\n",
                SE_PrintFloat64(value_ptr->saturation));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "value: %s\n", SE_PrintFloat64(value_ptr->value));
    }
} /* end SE_PrintHSVData */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintInt16
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintInt16
(
    SE_INT16 value
)
{
    static char buffer[100];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%i", (int) value);

    return buffer;
} /* end SE_PrintInt16() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintInt32
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintInt32
(
    SE_INT32 value
)
{
    static char buffer[100];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%i", (int) value);

    return buffer;
} /* end SE_PrintInt32() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintInt8
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintInt8
(
    SE_INT8 value
)
{
    static char buffer[100];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%i", (int) value);

    return buffer;
} /* end SE_PrintInt8() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLCC2DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLCC2DCoord
(
    const SRM_LCC_2D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LCC_2D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
    }
} /* end SRM_PrintLCC2DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLCC2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLCC2DParameters
(
    const SRM_LCC_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LCC_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->north_parallel_geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->north_parallel_geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->south_parallel_geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->south_parallel_geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "north_parallel_geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->north_parallel_geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "south_parallel_geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->south_parallel_geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
    }
} /* end SRM_PrintLCC2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLCC3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLCC3DCoord
(
    const SRM_LCC_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LCC_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintLCC3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLCC3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLCC3DParameters
(
    const SRM_LCC_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LCC_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->vertical_datum: %s\n", name_ptr,
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->north_parallel_geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->north_parallel_geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->south_parallel_geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->south_parallel_geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z_units: %s\n", name_ptr,
                SRM_PrintElevationUnits(value_ptr->z_units));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "vertical_datum: %s\n",
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "north_parallel_geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->north_parallel_geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "south_parallel_geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->south_parallel_geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z_units: %s\n",
                SRM_PrintElevationUnits(value_ptr->z_units));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
    }
} /* end SRM_PrintLCC3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLSR2DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLSR2DCoord
(
    const SRM_LSR_2D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LSR_2D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
    }
} /* end SRM_PrintLSR2DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLSR2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLSR2DParameters
(
    const SRM_LSR_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LSR_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->forward_direction: %s\n", name_ptr,
                SE_PrintDirectionOfForward(value_ptr->forward_direction));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "forward_direction: %s\n",
                SE_PrintDirectionOfForward(value_ptr->forward_direction));
    }
} /* end SRM_PrintLSR2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLSR3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLSR3DCoord
(
    const SRM_LSR_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LSR_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintLSR3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLSR3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLSR3DParameters
(
    const SRM_LSR_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LSR_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->up_direction: %s\n", name_ptr,
                SE_PrintDirectionOfUp(value_ptr->up_direction));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->forward_direction: %s\n", name_ptr,
                SE_PrintDirectionOfForward(value_ptr->forward_direction));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "up_direction: %s\n",
                SE_PrintDirectionOfUp(value_ptr->up_direction));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "forward_direction: %s\n",
                SE_PrintDirectionOfForward(value_ptr->forward_direction));
    }
} /* end SRM_PrintLSR3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLTP2DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLTP2DCoord
(
    const SRM_LTP_2D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LTP_2D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
    }
} /* end SRM_PrintLTP2DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLTP2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLTP2DParameters
(
    const SRM_LTP_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LTP_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->azimuth: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->azimuth));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "azimuth: %s\n", SE_PrintFloat64(value_ptr->azimuth));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
    }
} /* end SRM_PrintLTP2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLTP3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLTP3DCoord
(
    const SRM_LTP_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LTP_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintLTP3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintLTP3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintLTP3DParameters
(
    const SRM_LTP_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_LTP_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->vertical_datum: %s\n", name_ptr,
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->azimuth: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->azimuth));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z_units: %s\n", name_ptr,
                SRM_PrintElevationUnits(value_ptr->z_units));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "vertical_datum: %s\n",
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "azimuth: %s\n", SE_PrintFloat64(value_ptr->azimuth));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z_units: %s\n",
                SRM_PrintElevationUnits(value_ptr->z_units));
    }
} /* end SRM_PrintLTP3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintM2DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintM2DCoord
(
    const SRM_M_2D_COORD *value_ptr,
    const char           *name_ptr,
          SE_UINT16       level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_M_2D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
    }
} /* end SRM_PrintM2DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintM2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintM2DParameters
(
    const SRM_M_2D_PARAMETERS *value_ptr,
    const char                *name_ptr,
          SE_UINT16            level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_M_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->central_scale_factor: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "central_scale_factor: %s\n",
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
    }
} /* end SRM_PrintM2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintM3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintM3DCoord
(
    const SRM_M_3D_COORD *value_ptr,
    const char           *name_ptr,
          SE_UINT16       level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_M_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintM3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintM3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintM3DParameters
(
    const SRM_M_3D_PARAMETERS *value_ptr,
    const char                *name_ptr,
          SE_UINT16            level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_M_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->vertical_datum: %s\n", name_ptr,
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->central_scale_factor: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z_units: %s\n", name_ptr,
                SRM_PrintElevationUnits(value_ptr->z_units));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "vertical_datum: %s\n",
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "central_scale_factor: %s\n",
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z_units: %s\n",
                SRM_PrintElevationUnits(value_ptr->z_units));
    }
} /* end SRM_PrintM3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintMatrix3X3Type
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintMatrix3X3Type
(
    const SE_MATRIX_3X3_TYPE *value_ptr,
    const char               *name_ptr,
          SE_UINT16           level
)
{
    FILE      *f_ptr;
    SE_UINT32  i, j;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_MATRIX_3X3_TYPE\n");
        return;
    }
    if (name_ptr)
    {
        for (i = 0; i < 3; i++)
        {
            SE_Indent(level, f_ptr);
            fprintf(f_ptr, "%s->mat[%d][0..2]: ", name_ptr, i);
            for (j = 0; j < 2; j++)
            {
                fprintf(f_ptr, "%s ", SE_PrintFloat64(value_ptr->mat[i][j]));
            } /* end for j */
            fprintf(f_ptr, "%s\n", SE_PrintFloat64(value_ptr->mat[i][j]));
        } /* end for i */
    }
    else
    {
        for (i = 0; i < 3; i++)
        {
            SE_Indent(level, f_ptr);
            fprintf(f_ptr, "mat[%d][0..2]: ", i);
            for (j = 0; j < 2; j++)
            {
                fprintf(f_ptr, "%s ", SE_PrintFloat64(value_ptr->mat[i][j]));
            } /* end for j */
            fprintf(f_ptr, "%s\n", SE_PrintFloat64(value_ptr->mat[i][j]));
        } /* end for i */
    }
} /* end SE_PrintMatrix3X3Type */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintMatrix4X4Type
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintMatrix4X4Type
(
    const SE_MATRIX_4X4_TYPE *value_ptr,
    const char               *name_ptr,
          SE_UINT16           level
)
{
    FILE      *f_ptr;
    SE_UINT32  i, j;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_MATRIX_4X4_TYPE\n");
        return;
    }
    if (name_ptr)
    {
        for (i = 0; i < 4; i++)
        {
            SE_Indent(level, f_ptr);
            fprintf(f_ptr, "%s->mat[%d][0..3]: ", name_ptr, i);
            for (j = 0; j < 3; j++)
            {
                fprintf(f_ptr, "%s ", SE_PrintFloat64(value_ptr->mat[i][j]));
            } /* end for j */
            fprintf(f_ptr, "%s\n", SE_PrintFloat64(value_ptr->mat[i][j]));
        } /* end for i */
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            SE_Indent(level, f_ptr);
            fprintf(f_ptr, "mat[%d][0..3]: ", i);
            for (j = 0; j < 3; j++)
            {
                fprintf(f_ptr, "%s ", SE_PrintFloat64(value_ptr->mat[i][j]));
            } /* end for j */
            fprintf(f_ptr, "%s\n", SE_PrintFloat64(value_ptr->mat[i][j]));
        } /* end for i */
    }
} /* end SE_PrintMatrix4X4Type */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintOM2DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintOM2DCoord
(
    const SRM_OM_2D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_OM_2D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
    }
} /* end SRM_PrintOM2DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintOM2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintOM2DParameters
(
    const SRM_OM_2D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_OM_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude1: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude1));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude1: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude1));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude2: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude2));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude2: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude2));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->central_scale_factor: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->central_scale_factor));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude1: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude1));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude1: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude1));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude2: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude2));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude2: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude2));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "central_scale_factor: %s\n",
                SE_PrintFloat64(value_ptr->central_scale_factor));
    }
} /* end SRM_PrintOM2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintOM3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintOM3DCoord
(
    const SRM_OM_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_OM_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintOM3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintOM3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintOM3DParameters
(
    const SRM_OM_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_OM_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->vertical_datum: %s\n", name_ptr,
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude1: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude1));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude1: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude1));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude2: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude2));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude2: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude2));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->central_scale_factor: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z_units: %s\n", name_ptr,
                SRM_PrintElevationUnits(value_ptr->z_units));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "vertical_datum: %s\n",
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude1: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude1));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude1: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude1));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude2: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude2));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude2: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude2));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "central_scale_factor: %s\n",
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z_units: %s\n",
                SRM_PrintElevationUnits(value_ptr->z_units));
    }
} /* end SRM_PrintOM3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintPint16
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintPint16
(
    SE_PINT16 value
)
{
    static char buffer[100];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%u", (unsigned int) value);

    return buffer;
} /* end SE_PrintPint16() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintPint32
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintPint32
(
    SE_PINT32 value
)
{
    static char buffer[100];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%u", (unsigned int) value);

    return buffer;
} /* end SE_PrintPint32() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintPint8
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintPint8
(
    SE_PINT8 value
)
{
    static char buffer[100];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%u", (unsigned int) value);

    return buffer;
} /* end SE_PrintPint8() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintPS2DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintPS2DCoord
(
    const SRM_PS_2D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_PS_2D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
    }
} /* end SRM_PrintPS2DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintPS2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintPS2DParameters
(
    const SRM_PS_2D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_PS_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->pole: %s\n", name_ptr,
                SE_PrintPSPole(value_ptr->pole));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->central_scale_factor: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "pole: %s\n", SE_PrintPSPole(value_ptr->pole));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "central_scale_factor: %s\n",
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
    }
} /* end SRM_PrintPS2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintPS3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintPS3DCoord
(
    const SRM_PS_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_PS_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintPS3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintPS3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintPS3DParameters
(
    const SRM_PS_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_PS_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->vertical_datum: %s\n", name_ptr,
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->pole: %s\n", name_ptr,
                SE_PrintPSPole(value_ptr->pole));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->central_scale_factor: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z_units: %s\n", name_ptr,
                SRM_PrintElevationUnits(value_ptr->z_units));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "vertical_datum: %s\n",
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "pole: %s\n", SE_PrintPSPole(value_ptr->pole));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "central_scale_factor: %s\n",
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z_units: %s\n",
                SRM_PrintElevationUnits(value_ptr->z_units));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
    }
} /* end SRM_PrintPS3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintPSPole
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintPSPole
(
    SE_PS_POLE_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SE_NORTH_POLE:
             return "North Pole";

        case SE_SOUTH_POLE:
             return "South Pole";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SE_PS_POLE_ENUM VALUE (%i)",(int)
                    value);
             break;
    } /* end switch */

    return buffer;
} /* end SE_PrintPSPole */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintRGBData
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintRGBData
(
    const SE_RGB_DATA *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_RGB_DATA\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->red: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->red));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->green: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->green));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->blue: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->blue));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "red: %s\n", SE_PrintFloat64(value_ptr->red));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "green: %s\n", SE_PrintFloat64(value_ptr->green));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "blue: %s\n", SE_PrintFloat64(value_ptr->blue));
    }
} /* end SE_PrintRGBData */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSM3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintSM3DCoord
(
    const SRM_SM_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_SM_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->radius: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->radius));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "longitude: %s\n",
                SE_PrintFloat64(value_ptr->longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "latitude: %s\n", SE_PrintFloat64(value_ptr->latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "radius: %s\n", SE_PrintFloat64(value_ptr->radius));
    }
} /* end SRM_PrintSM3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSM3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintSM3DParameters
(
    const SRM_SM_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_SM_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->ignore_na: N/A\n", name_ptr);
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "ignore_na: N/A\n");
    }
} /* end SRM_PrintSM3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSRF2D
 *
 *-----------------------------------------------------------------------------
 */
const char *
SRM_PrintSRF2D
(
    SRM_SRF_2D_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SRM_GD_2D_SRF:
             return "GD 2D SRF";

        case SRM_EC_2D_SRF:
             return "EC 2D SRF";

        case SRM_PS_2D_SRF:
             return "PS 2D SRF";

        case SRM_LCC_2D_SRF:
             return "LCC 2D SRF";

        case SRM_TM_2D_SRF:
             return "TM 2D SRF";

        case SRM_UTM_2D_SRF:
             return "UTM 2D SRF";

        case SRM_LTP_2D_SRF:
             return "LTP 2D SRF";

        case SRM_LSR_2D_SRF:
             return "LSR 2D SRF";

        case SRM_M_2D_SRF:
             return "M 2D SRF";

        case SRM_OM_2D_SRF:
             return "OM 2D SRF";

        case SRM_UPS_2D_SRF:
             return "UPS 2D SRF";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SRM_SRF_2D_ENUM VALUE (%i)",(int)
                    value);
             break;
    } /* end switch */

    return buffer;
} /* end SRM_PrintSRF2D */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSRF2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintSRF2DParameters
(
    const SRM_SRF_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;
    char  buffer[1024];

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_SRF_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->spatial_reference_frame: %s\n", name_ptr,
                SRM_PrintSRF2D(value_ptr->spatial_reference_frame));
        switch (value_ptr->spatial_reference_frame)
        {
            case SRM_GD_2D_SRF:
                sprintf(buffer, "%s->u.gd_parameters", name_ptr);
                SRM_PrintGD2DParameters(&(value_ptr->u.gd_parameters), buffer,
                  level);
                break;

            case SRM_EC_2D_SRF:
                sprintf(buffer, "%s->u.ec_parameters", name_ptr);
                SRM_PrintEC2DParameters(&(value_ptr->u.ec_parameters), buffer,
                  level);
                break;

            case SRM_PS_2D_SRF:
                sprintf(buffer, "%s->u.ps_parameters", name_ptr);
                SRM_PrintPS2DParameters(&(value_ptr->u.ps_parameters), buffer,
                  level);
                break;

            case SRM_LCC_2D_SRF:
                sprintf(buffer, "%s->u.lcc_parameters", name_ptr);
                SRM_PrintLCC2DParameters(&(value_ptr->u.lcc_parameters),
                  buffer, level);
                break;

            case SRM_TM_2D_SRF:
                sprintf(buffer, "%s->u.tm_parameters", name_ptr);
                SRM_PrintTM2DParameters(&(value_ptr->u.tm_parameters), buffer,
                  level);
                break;

            case SRM_UTM_2D_SRF:
                sprintf(buffer, "%s->u.utm_parameters", name_ptr);
                SRM_PrintUTM2DParameters(&(value_ptr->u.utm_parameters),
                  buffer, level);
                break;

            case SRM_LTP_2D_SRF:
                sprintf(buffer, "%s->u.ltp_parameters", name_ptr);
                SRM_PrintLTP2DParameters(&(value_ptr->u.ltp_parameters),
                  buffer, level);
                break;

            case SRM_LSR_2D_SRF:
                sprintf(buffer, "%s->u.lsr_parameters", name_ptr);
                SRM_PrintLSR2DParameters(&(value_ptr->u.lsr_parameters),
                  buffer, level);
                break;

            case SRM_M_2D_SRF:
                sprintf(buffer, "%s->u.m_parameters", name_ptr);
                SRM_PrintM2DParameters(&(value_ptr->u.m_parameters), buffer,
                  level);
                break;

            case SRM_OM_2D_SRF:
                sprintf(buffer, "%s->u.om_parameters", name_ptr);
                SRM_PrintOM2DParameters(&(value_ptr->u.om_parameters), buffer,
                  level);
                break;

            case SRM_UPS_2D_SRF:
                sprintf(buffer, "%s->u.ups_parameters", name_ptr);
                SRM_PrintUPS2DParameters(&(value_ptr->u.ups_parameters),
                  buffer, level);
                break;

        } /* end switch */
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "spatial_reference_frame: %s\n",
                SRM_PrintSRF2D(value_ptr->spatial_reference_frame));
        switch (value_ptr->spatial_reference_frame)
        {
            case SRM_GD_2D_SRF:
                sprintf(buffer, "u.gd_parameters");
                SRM_PrintGD2DParameters(&(value_ptr->u.gd_parameters), buffer,
                  level);
                break;

            case SRM_EC_2D_SRF:
                sprintf(buffer, "u.ec_parameters");
                SRM_PrintEC2DParameters(&(value_ptr->u.ec_parameters), buffer,
                  level);
                break;

            case SRM_PS_2D_SRF:
                sprintf(buffer, "u.ps_parameters");
                SRM_PrintPS2DParameters(&(value_ptr->u.ps_parameters), buffer,
                  level);
                break;

            case SRM_LCC_2D_SRF:
                sprintf(buffer, "u.lcc_parameters");
                SRM_PrintLCC2DParameters(&(value_ptr->u.lcc_parameters),
                  buffer, level);
                break;

            case SRM_TM_2D_SRF:
                sprintf(buffer, "u.tm_parameters");
                SRM_PrintTM2DParameters(&(value_ptr->u.tm_parameters), buffer,
                  level);
                break;

            case SRM_UTM_2D_SRF:
                sprintf(buffer, "u.utm_parameters");
                SRM_PrintUTM2DParameters(&(value_ptr->u.utm_parameters),
                  buffer, level);
                break;

            case SRM_LTP_2D_SRF:
                sprintf(buffer, "u.ltp_parameters");
                SRM_PrintLTP2DParameters(&(value_ptr->u.ltp_parameters),
                  buffer, level);
                break;

            case SRM_LSR_2D_SRF:
                sprintf(buffer, "u.lsr_parameters");
                SRM_PrintLSR2DParameters(&(value_ptr->u.lsr_parameters),
                  buffer, level);
                break;

            case SRM_M_2D_SRF:
                sprintf(buffer, "u.m_parameters");
                SRM_PrintM2DParameters(&(value_ptr->u.m_parameters), buffer,
                  level);
                break;

            case SRM_OM_2D_SRF:
                sprintf(buffer, "u.om_parameters");
                SRM_PrintOM2DParameters(&(value_ptr->u.om_parameters), buffer,
                  level);
                break;

            case SRM_UPS_2D_SRF:
                sprintf(buffer, "u.ups_parameters");
                SRM_PrintUPS2DParameters(&(value_ptr->u.ups_parameters),
                  buffer, level);
                break;

        } /* end switch */
    }
} /* end SRM_PrintSRF2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSRF3D
 *
 *-----------------------------------------------------------------------------
 */
const char *
SRM_PrintSRF3D
(
    SRM_SRF_3D_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SRM_GD_3D_SRF:
             return "GD 3D SRF";

        case SRM_GC_3D_SRF:
             return "GC 3D SRF";

        case SRM_GM_3D_SRF:
             return "GM 3D SRF";

        case SRM_GEI_3D_SRF:
             return "GEI 3D SRF";

        case SRM_GSE_3D_SRF:
             return "GSE 3D SRF";

        case SRM_GSM_3D_SRF:
             return "GSM 3D SRF";

        case SRM_SM_3D_SRF:
             return "SM 3D SRF";

        case SRM_GCS_3D_SRF:
             return "GCS 3D SRF";

        case SRM_EC_3D_SRF:
             return "EC 3D SRF";

        case SRM_PS_3D_SRF:
             return "PS 3D SRF";

        case SRM_LCC_3D_SRF:
             return "LCC 3D SRF";

        case SRM_TM_3D_SRF:
             return "TM 3D SRF";

        case SRM_UTM_3D_SRF:
             return "UTM 3D SRF";

        case SRM_LTP_3D_SRF:
             return "LTP 3D SRF";

        case SRM_LSR_3D_SRF:
             return "LSR 3D SRF";

        case SRM_M_3D_SRF:
             return "M 3D SRF";

        case SRM_OM_3D_SRF:
             return "OM 3D SRF";

        case SRM_UPS_3D_SRF:
             return "UPS 3D SRF";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SRM_SRF_3D_ENUM VALUE (%i)",(int)
                    value);
             break;
    } /* end switch */

    return buffer;
} /* end SRM_PrintSRF3D */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintSRF3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintSRF3DParameters
(
    const SRM_SRF_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;
    char  buffer[1024];

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_SRF_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->spatial_reference_frame: %s\n", name_ptr,
                SRM_PrintSRF3D(value_ptr->spatial_reference_frame));
        switch (value_ptr->spatial_reference_frame)
        {
            case SRM_GD_3D_SRF:
                sprintf(buffer, "%s->u.gd_parameters", name_ptr);
                SRM_PrintGD3DParameters(&(value_ptr->u.gd_parameters), buffer,
                  level);
                break;

            case SRM_GC_3D_SRF:
                sprintf(buffer, "%s->u.gc_parameters", name_ptr);
                SRM_PrintGC3DParameters(&(value_ptr->u.gc_parameters), buffer,
                  level);
                break;

            case SRM_GM_3D_SRF:
                sprintf(buffer, "%s->u.gm_parameters", name_ptr);
                SRM_PrintGM3DParameters(&(value_ptr->u.gm_parameters), buffer,
                  level);
                break;

            case SRM_GEI_3D_SRF:
                sprintf(buffer, "%s->u.gei_parameters", name_ptr);
                SRM_PrintGEI3DParameters(&(value_ptr->u.gei_parameters),
                  buffer, level);
                break;

            case SRM_GSE_3D_SRF:
                sprintf(buffer, "%s->u.gse_parameters", name_ptr);
                SRM_PrintGSE3DParameters(&(value_ptr->u.gse_parameters),
                  buffer, level);
                break;

            case SRM_GSM_3D_SRF:
                sprintf(buffer, "%s->u.gsm_parameters", name_ptr);
                SRM_PrintGSM3DParameters(&(value_ptr->u.gsm_parameters),
                  buffer, level);
                break;

            case SRM_SM_3D_SRF:
                sprintf(buffer, "%s->u.sm_parameters", name_ptr);
                SRM_PrintSM3DParameters(&(value_ptr->u.sm_parameters), buffer,
                  level);
                break;

            case SRM_GCS_3D_SRF:
                sprintf(buffer, "%s->u.gcs_parameters", name_ptr);
                SRM_PrintGCS3DParameters(&(value_ptr->u.gcs_parameters),
                  buffer, level);
                break;

            case SRM_EC_3D_SRF:
                sprintf(buffer, "%s->u.ec_parameters", name_ptr);
                SRM_PrintEC3DParameters(&(value_ptr->u.ec_parameters), buffer,
                  level);
                break;

            case SRM_PS_3D_SRF:
                sprintf(buffer, "%s->u.ps_parameters", name_ptr);
                SRM_PrintPS3DParameters(&(value_ptr->u.ps_parameters), buffer,
                  level);
                break;

            case SRM_LCC_3D_SRF:
                sprintf(buffer, "%s->u.lcc_parameters", name_ptr);
                SRM_PrintLCC3DParameters(&(value_ptr->u.lcc_parameters),
                  buffer, level);
                break;

            case SRM_TM_3D_SRF:
                sprintf(buffer, "%s->u.tm_parameters", name_ptr);
                SRM_PrintTM3DParameters(&(value_ptr->u.tm_parameters), buffer,
                  level);
                break;

            case SRM_UTM_3D_SRF:
                sprintf(buffer, "%s->u.utm_parameters", name_ptr);
                SRM_PrintUTM3DParameters(&(value_ptr->u.utm_parameters),
                  buffer, level);
                break;

            case SRM_LTP_3D_SRF:
                sprintf(buffer, "%s->u.ltp_parameters", name_ptr);
                SRM_PrintLTP3DParameters(&(value_ptr->u.ltp_parameters),
                  buffer, level);
                break;

            case SRM_LSR_3D_SRF:
                sprintf(buffer, "%s->u.lsr_parameters", name_ptr);
                SRM_PrintLSR3DParameters(&(value_ptr->u.lsr_parameters),
                  buffer, level);
                break;

            case SRM_M_3D_SRF:
                sprintf(buffer, "%s->u.m_parameters", name_ptr);
                SRM_PrintM3DParameters(&(value_ptr->u.m_parameters), buffer,
                  level);
                break;

            case SRM_OM_3D_SRF:
                sprintf(buffer, "%s->u.om_parameters", name_ptr);
                SRM_PrintOM3DParameters(&(value_ptr->u.om_parameters), buffer,
                  level);
                break;

            case SRM_UPS_3D_SRF:
                sprintf(buffer, "%s->u.ups_parameters", name_ptr);
                SRM_PrintUPS3DParameters(&(value_ptr->u.ups_parameters),
                  buffer, level);
                break;

        } /* end switch */
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "spatial_reference_frame: %s\n",
                SRM_PrintSRF3D(value_ptr->spatial_reference_frame));
        switch (value_ptr->spatial_reference_frame)
        {
            case SRM_GD_3D_SRF:
                sprintf(buffer, "u.gd_parameters");
                SRM_PrintGD3DParameters(&(value_ptr->u.gd_parameters), buffer,
                  level);
                break;

            case SRM_GC_3D_SRF:
                sprintf(buffer, "u.gc_parameters");
                SRM_PrintGC3DParameters(&(value_ptr->u.gc_parameters), buffer,
                  level);
                break;

            case SRM_GM_3D_SRF:
                sprintf(buffer, "u.gm_parameters");
                SRM_PrintGM3DParameters(&(value_ptr->u.gm_parameters), buffer,
                  level);
                break;

            case SRM_GEI_3D_SRF:
                sprintf(buffer, "u.gei_parameters");
                SRM_PrintGEI3DParameters(&(value_ptr->u.gei_parameters),
                  buffer, level);
                break;

            case SRM_GSE_3D_SRF:
                sprintf(buffer, "u.gse_parameters");
                SRM_PrintGSE3DParameters(&(value_ptr->u.gse_parameters),
                  buffer, level);
                break;

            case SRM_GSM_3D_SRF:
                sprintf(buffer, "u.gsm_parameters");
                SRM_PrintGSM3DParameters(&(value_ptr->u.gsm_parameters),
                  buffer, level);
                break;

            case SRM_SM_3D_SRF:
                sprintf(buffer, "u.sm_parameters");
                SRM_PrintSM3DParameters(&(value_ptr->u.sm_parameters), buffer,
                  level);
                break;

            case SRM_GCS_3D_SRF:
                sprintf(buffer, "u.gcs_parameters");
                SRM_PrintGCS3DParameters(&(value_ptr->u.gcs_parameters),
                  buffer, level);
                break;

            case SRM_EC_3D_SRF:
                sprintf(buffer, "u.ec_parameters");
                SRM_PrintEC3DParameters(&(value_ptr->u.ec_parameters), buffer,
                  level);
                break;

            case SRM_PS_3D_SRF:
                sprintf(buffer, "u.ps_parameters");
                SRM_PrintPS3DParameters(&(value_ptr->u.ps_parameters), buffer,
                  level);
                break;

            case SRM_LCC_3D_SRF:
                sprintf(buffer, "u.lcc_parameters");
                SRM_PrintLCC3DParameters(&(value_ptr->u.lcc_parameters),
                  buffer, level);
                break;

            case SRM_TM_3D_SRF:
                sprintf(buffer, "u.tm_parameters");
                SRM_PrintTM3DParameters(&(value_ptr->u.tm_parameters), buffer,
                  level);
                break;

            case SRM_UTM_3D_SRF:
                sprintf(buffer, "u.utm_parameters");
                SRM_PrintUTM3DParameters(&(value_ptr->u.utm_parameters),
                  buffer, level);
                break;

            case SRM_LTP_3D_SRF:
                sprintf(buffer, "u.ltp_parameters");
                SRM_PrintLTP3DParameters(&(value_ptr->u.ltp_parameters),
                  buffer, level);
                break;

            case SRM_LSR_3D_SRF:
                sprintf(buffer, "u.lsr_parameters");
                SRM_PrintLSR3DParameters(&(value_ptr->u.lsr_parameters),
                  buffer, level);
                break;

            case SRM_M_3D_SRF:
                sprintf(buffer, "u.m_parameters");
                SRM_PrintM3DParameters(&(value_ptr->u.m_parameters), buffer,
                  level);
                break;

            case SRM_OM_3D_SRF:
                sprintf(buffer, "u.om_parameters");
                SRM_PrintOM3DParameters(&(value_ptr->u.om_parameters), buffer,
                  level);
                break;

            case SRM_UPS_3D_SRF:
                sprintf(buffer, "u.ups_parameters");
                SRM_PrintUPS3DParameters(&(value_ptr->u.ups_parameters),
                  buffer, level);
                break;

        } /* end switch */
    }
} /* end SRM_PrintSRF3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintSRFParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintSRFParameters
(
    const SE_SRF_PARAMETERS *value_ptr,
    const char              *name_ptr,
          SE_UINT16          level
)
{
    FILE *f_ptr;
    char  buffer[1024];

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_SRF_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->is_2D: %s\n", name_ptr,
                SE_PrintBoolean(value_ptr->is_2D));
        switch (value_ptr->is_2D)
        {
            case SE_FALSE:
                sprintf(buffer, "%s->u.params_3d", name_ptr);
                SRM_PrintSRF3DParameters(&(value_ptr->u.params_3d), buffer,
                  level);
                break;

            case SE_TRUE:
                sprintf(buffer, "%s->u.params_2d", name_ptr);
                SRM_PrintSRF2DParameters(&(value_ptr->u.params_2d), buffer,
                  level);
                break;

        } /* end switch */
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "is_2D: %s\n", SE_PrintBoolean(value_ptr->is_2D));
        switch (value_ptr->is_2D)
        {
            case SE_FALSE:
                sprintf(buffer, "u.params_3d");
                SRM_PrintSRF3DParameters(&(value_ptr->u.params_3d), buffer,
                  level);
                break;

            case SE_TRUE:
                sprintf(buffer, "u.params_2d");
                SRM_PrintSRF2DParameters(&(value_ptr->u.params_2d), buffer,
                  level);
                break;

        } /* end switch */
    }
} /* end SE_PrintSRFParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintTM2DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintTM2DCoord
(
    const SRM_TM_2D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_TM_2D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
    }
} /* end SRM_PrintTM2DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintTM2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintTM2DParameters
(
    const SRM_TM_2D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_TM_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->central_scale_factor: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->central_scale_factor));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "central_scale_factor: %s\n",
                SE_PrintFloat64(value_ptr->central_scale_factor));
    }
} /* end SRM_PrintTM2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintTM3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintTM3DCoord
(
    const SRM_TM_3D_COORD *value_ptr,
    const char            *name_ptr,
          SE_UINT16        level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_TM_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintTM3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintTM3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintTM3DParameters
(
    const SRM_TM_3D_PARAMETERS *value_ptr,
    const char                 *name_ptr,
          SE_UINT16             level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_TM_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->vertical_datum: %s\n", name_ptr,
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_latitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->geodetic_longitude: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->central_scale_factor: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z_units: %s\n", name_ptr,
                SRM_PrintElevationUnits(value_ptr->z_units));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "vertical_datum: %s\n",
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_latitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_latitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "geodetic_longitude: %s\n",
                SE_PrintFloat64(value_ptr->geodetic_longitude));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "central_scale_factor: %s\n",
                SE_PrintFloat64(value_ptr->central_scale_factor));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z_units: %s\n",
                SRM_PrintElevationUnits(value_ptr->z_units));
    }
} /* end SRM_PrintTM3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintUint16
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintUint16
(
    SE_UINT16 value
)
{
    static char buffer[100];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%u", (unsigned int) value);

    return buffer;
} /* end SE_PrintUint16() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintUint32
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintUint32
(
    SE_UINT32 value
)
{
    static char buffer[100];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%u", (unsigned int) value);

    return buffer;
} /* end SE_PrintUint32() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintUint8
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintUint8
(
    SE_UINT8 value
)
{
    static char buffer[100];
               /*
                * for output string (or error message)
                */

    sprintf(buffer, "%u", (unsigned int) value);

    return buffer;
} /* end SE_PrintUint8() */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUPS2DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintUPS2DCoord
(
    const SRM_UPS_2D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_UPS_2D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
    }
} /* end SRM_PrintUPS2DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUPS2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintUPS2DParameters
(
    const SRM_UPS_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_UPS_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->pole: %s\n", name_ptr,
                SE_PrintPSPole(value_ptr->pole));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "pole: %s\n", SE_PrintPSPole(value_ptr->pole));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
    }
} /* end SRM_PrintUPS2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUPS3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintUPS3DCoord
(
    const SRM_UPS_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_UPS_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintUPS3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUPS3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintUPS3DParameters
(
    const SRM_UPS_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_UPS_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->vertical_datum: %s\n", name_ptr,
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->pole: %s\n", name_ptr,
                SE_PrintPSPole(value_ptr->pole));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z_units: %s\n", name_ptr,
                SRM_PrintElevationUnits(value_ptr->z_units));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "vertical_datum: %s\n",
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "pole: %s\n", SE_PrintPSPole(value_ptr->pole));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z_units: %s\n",
                SRM_PrintElevationUnits(value_ptr->z_units));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
    }
} /* end SRM_PrintUPS3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUTM2DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintUTM2DCoord
(
    const SRM_UTM_2D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_UTM_2D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->hemisphere: %s\n", name_ptr,
                SE_PrintHemisphere(value_ptr->hemisphere));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "hemisphere: %s\n",
                SE_PrintHemisphere(value_ptr->hemisphere));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
    }
} /* end SRM_PrintUTM2DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUTM2DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintUTM2DParameters
(
    const SRM_UTM_2D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_UTM_2D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->zone: %s\n", name_ptr,
                SE_PrintPint8(value_ptr->zone));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "zone: %s\n", SE_PrintPint8(value_ptr->zone));
    }
} /* end SRM_PrintUTM2DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUTM3DCoord
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintUTM3DCoord
(
    const SRM_UTM_3D_COORD *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_UTM_3D_COORD\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->hemisphere: %s\n", name_ptr,
                SE_PrintHemisphere(value_ptr->hemisphere));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x: %s\n", name_ptr, SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z: %s\n", name_ptr, SE_PrintFloat64(value_ptr->z));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "hemisphere: %s\n",
                SE_PrintHemisphere(value_ptr->hemisphere));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x: %s\n", SE_PrintFloat64(value_ptr->x));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z: %s\n", SE_PrintFloat64(value_ptr->z));
    }
} /* end SRM_PrintUTM3DCoord */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SRM_PrintUTM3DParameters
 *
 *-----------------------------------------------------------------------------
 */
void
SRM_PrintUTM3DParameters
(
    const SRM_UTM_3D_PARAMETERS *value_ptr,
    const char                  *name_ptr,
          SE_UINT16              level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SRM_UTM_3D_PARAMETERS\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->horizontal_datum: %s\n", name_ptr,
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->vertical_datum: %s\n", name_ptr,
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->x_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y_offset: %s\n", name_ptr,
                SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->z_units: %s\n", name_ptr,
                SRM_PrintElevationUnits(value_ptr->z_units));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->zone: %s\n", name_ptr,
                SE_PrintPint8(value_ptr->zone));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "horizontal_datum: %s\n",
                SE_PrintHorizontalDatum(value_ptr->horizontal_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "vertical_datum: %s\n",
                SE_PrintVerticalDatum(value_ptr->vertical_datum));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "x_offset: %s\n", SE_PrintFloat64(value_ptr->x_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y_offset: %s\n", SE_PrintFloat64(value_ptr->y_offset));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "z_units: %s\n",
                SRM_PrintElevationUnits(value_ptr->z_units));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "zone: %s\n", SE_PrintPint8(value_ptr->zone));
    }
} /* end SRM_PrintUTM3DParameters */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintVector3Type
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintVector3Type
(
    const SE_VECTOR_3_TYPE *value_ptr,
    const char             *name_ptr,
          SE_UINT16         level
)
{
    SE_UINT32  i;
    FILE      *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_VECTOR_3_TYPE\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->vector[0..2]: ", name_ptr);
        for (i = 0; i < 3; i++)
        {
            fprintf(f_ptr, "%s ", SE_PrintFloat64(value_ptr->vector[i]));
        } /* end for i */
        fprintf(f_ptr, "\n");
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "vector[0..2]: ");
        for (i = 0; i < 3; i++)
        {
            fprintf(f_ptr, "%s ", SE_PrintFloat64(value_ptr->vector[i]));
        } /* end for i */
        fprintf(f_ptr, "\n");
    }
} /* end SE_PrintVector3Type */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintVerticalDatum
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintVerticalDatum
(
    SE_VERTICAL_DATUM_ENUM value
)
{
    static char buffer[256];
               /*
                * for enumerator string (or error message)
                */

    switch (value)
    {
        case SE_MSL_VDATUM:
             return "MSL VDatum";

        case SE_WGS84G_VDATUM:
             return "WGS84G VDatum";

        case SE_WGS84E_VDATUM:
             return "WGS84E VDatum";

        case SE_SPHERICAL_ACM_VDATUM:
             return "Spherical ACM VDatum";

        case SE_SPHERICAL_COA_VDATUM:
             return "Spherical COA VDatum";

        case SE_SPHERICAL_NOG_VDATUM:
             return "Spherical NOG VDatum";

        case SE_SPHERICAL_MFE_VDATUM:
             return "Spherical MFE VDatum";

        case SE_SPHERICAL_MOD_M_VDATUM:
             return "Spherical MOD M VDatum";

        case SE_SPHERICAL_MOD_S_VDATUM:
             return "Spherical MOD S VDatum";

        case SE_SPHERICAL_MOD_T_VDATUM:
             return "Spherical MOD T VDatum";

        case SE_SPHERICAL_MM5_VDATUM:
             return "Spherical MM5 VDatum";

        case SE_EGM96_VDATUM:
             return "EGM96 VDatum";

        case SE_NAVD88_VDATUM:
             return "NAVD88 VDatum";

        case SE_NGVD29_VDATUM:
             return "NGVD29 VDatum";

        case SE_NUM_VERTICAL_DATUMS:
             return "Num Vertical Datums";

        default:
             sprintf(buffer,"ERROR - ILLEGAL SE_VERTICAL_DATUM_ENUM VALUE"\
                    " (%i)",(int) value);
             break;
    } /* end switch */

    return buffer;
} /* end SE_PrintVerticalDatum */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintYIQData
 *
 *-----------------------------------------------------------------------------
 */
void
SE_PrintYIQData
(
    const SE_YIQ_DATA *value_ptr,
    const char        *name_ptr,
          SE_UINT16    level
)
{
    FILE *f_ptr;

    f_ptr = SE_GetOutFilePtr();
    if (!value_ptr)
    {
        SE_Indent(level, f_ptr);
        if (name_ptr)
            fprintf(f_ptr, "%s is NULL, can't print\n", name_ptr);
        else
            fprintf(f_ptr, "Can't print NULL SE_YIQ_DATA\n");
        return;
    }
    if (name_ptr)
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->y: %s\n", name_ptr, SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->i: %s\n", name_ptr, SE_PrintFloat64(value_ptr->i));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "%s->q: %s\n", name_ptr, SE_PrintFloat64(value_ptr->q));
    }
    else
    {
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "y: %s\n", SE_PrintFloat64(value_ptr->y));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "i: %s\n", SE_PrintFloat64(value_ptr->i));
        SE_Indent(level, f_ptr);
        fprintf(f_ptr, "q: %s\n", SE_PrintFloat64(value_ptr->q));
    }
} /* end SE_PrintYIQData */
