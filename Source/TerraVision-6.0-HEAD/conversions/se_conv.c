/*
 * SEDRIS Conversions API
 *
 * FILE       : se_conv.c
 *
 * PROGRAMMERS: Bill Horan (SAIC), Howard Lu (SAIC)
 *
 * DESCRIPTION: provide reusable functions for conversions between:
 *                - color models (RGB, CMY, HSV, CMYK, HLS, YIQ)
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

#ifdef WIN32
#define SE_TURN_OFF_CONVERSIONS_WIN32_IMPORT
#endif

#include <stdio.h> /* needed for FILE */
#include <math.h>  /* needed for floor */
#include "se_conv.h"

#ifdef WIN32
#undef SE_TURN_OFF_CONVERSIONS_WIN32_IMPORT
#endif

/*
 * CONST: SE_PI
 */
const SE_FLOAT64 SE_PI = 3.14159265358979323846;


/*
 * CONST: SE_DEGREES_PER_RADIAN = 180.0 / SE_PI
 */
const SE_FLOAT64 SE_DEGREES_PER_RADIAN = 57.2957795130823208768;


/*
 * CONST: SE_RADIANS_PER_DEGREE = SE_PI / 180.0
 */
const SE_FLOAT64 SE_RADIANS_PER_DEGREE = 0.0174532925199432957692;


/*
 * CONST: SE_METERS_PER_FEET
 *
 *   0.3048 is an *exact* conversion factor from feet to meters
 */
const SE_FLOAT64 SE_METERS_PER_FEET = 0.3048;


/*
 * CONST: SE_FEET_PER_METER = 1.0 / SE_METERS_PER_FEET
 */
const SE_FLOAT64 SE_FEET_PER_METER = 3.28083989501312335958;


#define MINIMUM(a, b) ((a) < (b)) ? (a) : (b)
#define MAXIMUM(a, b) ((a) > (b)) ? (a) : (b)

/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: RGB_to_CMY
 *
 *   This function converts an RGB color to a CMY color.
 *
 *   The algorithm is based on the algorithm published in section 13.3.2,
 *   "The CMY Color Model" of Computer Graphics: Principles and Practice
 *   - Second Edition in C, by James D. Foley, Andries van Dam, Steven
 *   K. Feiner, and John F. Hughes, published by Addison-Wesley
 *   Publishing Company (Reprinted with corrections November 1992,
 *   November 1993, and July 1995), Copyright 1996, 1990 by
 *   Addison-Wesley Publishing Company, Inc.
 *
 *----------------------------------------------------------------------------
 */
static void RGB_to_CMY
(
    const SE_RGB_DATA *rgb_in_ptr,
          SE_CMY_DATA *cmy_out_ptr
)
{
    cmy_out_ptr->cyan    = 1.0 - rgb_in_ptr->red;
    cmy_out_ptr->magenta = 1.0 - rgb_in_ptr->green;
    cmy_out_ptr->yellow  = 1.0 - rgb_in_ptr->blue;
}


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: CMY_to_RGB
 *
 *   This function converts a CMY color to an RGB color.
 *
 *   The algorithm is based on the algorithm published in section 13.3.2,
 *   "The CMY Color Model" of Computer Graphics: Principles and Practice
 *   - Second Edition in C, by James D. Foley, Andries van Dam, Steven
 *   K. Feiner, and John F. Hughes, published by Addison-Wesley
 *   Publishing Company (Reprinted with corrections November 1992,
 *   November 1993, and July 1995), Copyright 1996, 1990 by
 *   Addison-Wesley Publishing Company, Inc.
 *
 *-----------------------------------------------------------------------
 */
static void CMY_to_RGB
(
    const SE_CMY_DATA *cmy_in_ptr,
          SE_RGB_DATA *rgb_out_ptr
)
{
    rgb_out_ptr->red   = 1.0 - cmy_in_ptr->cyan;
    rgb_out_ptr->green = 1.0 - cmy_in_ptr->magenta;
    rgb_out_ptr->blue  = 1.0 - cmy_in_ptr->yellow;
}


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: RGB_to_HSV
 *
 *   This function converts an RGB color to an HSV color.
 *
 *   The algorithm is based on the algorithm published in section 13.3.4,
 *   "The HSV Color Model" of Computer Graphics: Principles and Practice
 *   - Second Edition in C, by James D. Foley, Andries van Dam, Steven
 *   K. Feiner, and John F. Hughes, published by Addison-Wesley
 *   Publishing Company (Reprinted with corrections November 1992,
 *   November 1993, and July 1995), Copyright 1996, 1990 by
 *   Addison-Wesley Publishing Company, Inc.
 *
 *----------------------------------------------------------------------------
 */
static void RGB_to_HSV
(
    const SE_RGB_DATA *rgb_in_ptr,
          SE_HSV_DATA *hsv_out_ptr
)
{
    SE_FLOAT64 max = MAXIMUM(rgb_in_ptr->red,
                             MAXIMUM(rgb_in_ptr->green, rgb_in_ptr->blue));

    SE_FLOAT64 min = MINIMUM(rgb_in_ptr->red,
                             MINIMUM(rgb_in_ptr->green, rgb_in_ptr->blue));

    hsv_out_ptr->value = max;

    if (max==min) /* red, green, and blue were all identical */
    {
       /*
        * This is the achromatic case.
        */
        hsv_out_ptr->saturation = 0.0;
        hsv_out_ptr->hue        = SE_POSITIVE_INFINITY;
    }
    else
    {
        SE_FLOAT64 delta = max - min;

        hsv_out_ptr->saturation = delta / max;

        if (rgb_in_ptr->red == max)
        {
            hsv_out_ptr->hue =       (rgb_in_ptr->green - rgb_in_ptr->blue)
                               / delta;
        }
        else if (rgb_in_ptr->green == max)
        {
            hsv_out_ptr->hue = 2.0 + (rgb_in_ptr->blue  - rgb_in_ptr->red)
                               / delta;
        }
        else /* (rgb_in_ptr->blue == max) */
        {
            hsv_out_ptr->hue = 4.0 + (rgb_in_ptr->red   - rgb_in_ptr->green)
                               / delta;
        }

        hsv_out_ptr->hue *= 60.0;

        if (hsv_out_ptr->hue < 0.0)
            hsv_out_ptr->hue += 360.0;
    }

} /* end RGB_to_HSV */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: HSV_to_RGB
 *
 *   This function converts an HSV color to an RGB color.
 *
 *   The algorithm is based on the algorithm published in section 13.3.4,
 *   "The HSV Color Model" of Computer Graphics: Principles and Practice
 *   - Second Edition in C, by James D. Foley, Andries van Dam, Steven
 *   K. Feiner, and John F. Hughes, published by Addison-Wesley
 *   Publishing Company (Reprinted with corrections November 1992,
 *   November 1993, and July 1995), Copyright 1996, 1990 by
 *   Addison-Wesley Publishing Company, Inc.
 *
 *----------------------------------------------------------------------------
 */
static void HSV_to_RGB
(
    const SE_HSV_DATA *hsv_in_ptr,
          SE_RGB_DATA *rgb_out_ptr
)
{
    /* Achromatic color.  There is no hue. */
    if (hsv_in_ptr->saturation == 0.0)
    {
        /* The color is on the black-and-white center line. */
        rgb_out_ptr->red   = hsv_in_ptr->value;
        rgb_out_ptr->green = hsv_in_ptr->value;
        rgb_out_ptr->blue  = hsv_in_ptr->value;
    }
    else
    {
        SE_FLOAT64 fractional, reduced_hue, p, q, t;
        SE_INT32   region;

        /* ensure that the hue is between -360.0 and 360.0 */
        reduced_hue = fmod(hsv_in_ptr->hue, 360.0);

        /* ensure that the hue is between 0.0 and 360.0, not including 360.0 */
        if (reduced_hue < 0.0)
            reduced_hue += 360.0;

        reduced_hue /= 60.0;
        /* reduced hue = between 0.0 and 6.0, not including 6.0 */

        region = (SE_INT32) floor(reduced_hue);
        /* region now selects a 60 degree area   */

        /* in the Single-hexcone HSV color model */
        /* Region is now 0, 1, 2, 3, 4, or 5     */

        fractional = reduced_hue - region;

        p = hsv_in_ptr->value * (1.0 - hsv_in_ptr->saturation);
        q = hsv_in_ptr->value * (1.0 - (hsv_in_ptr->saturation * fractional));
        t = hsv_in_ptr->value * (1.0 - (hsv_in_ptr->saturation
                              * (1.0 - fractional)));

        switch (region)
        {
            case 0:
                 rgb_out_ptr->red   = hsv_in_ptr->value;
                 rgb_out_ptr->green = t;
                 rgb_out_ptr->blue  = p;
                 break;

            case 1:
                 rgb_out_ptr->red   = q;
                 rgb_out_ptr->green = hsv_in_ptr->value;
                 rgb_out_ptr->blue  = p;
                 break;

            case 2:
                 rgb_out_ptr->red   = p;
                 rgb_out_ptr->green = hsv_in_ptr->value;
                 rgb_out_ptr->blue  = t;
                 break;

            case 3:
                 rgb_out_ptr->red   = p;
                 rgb_out_ptr->green = q;
                 rgb_out_ptr->blue  = hsv_in_ptr->value;
                 break;

            case 4:
                 rgb_out_ptr->red   = t;
                 rgb_out_ptr->green = p;
                 rgb_out_ptr->blue  = hsv_in_ptr->value;
                 break;

            case 5:
                 rgb_out_ptr->red   = hsv_in_ptr->value;
                 rgb_out_ptr->green = p;
                 rgb_out_ptr->blue  = q;
                 break;

            default: /* mathematically impossible to reach this case */
            {
                 FILE *f_ptr;

                 f_ptr = SE_GetErrorFilePtr();
                 fprintf(f_ptr, "[SEDRIS_CONVERSIONS] HSV_to_RGB:"\
                         " mathematically impossible invalid region!\n");
            }
                 break;
        } /* end switch region */
    }
} /* end HSV_to_RGB */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertColorToGivenModel
 *
 *----------------------------------------------------------------------------
 */
SE_COLOR_STATUS_CODE_ENUM SE_ConvertColorToGivenModel
(
    const SE_COLOR_DATA       *original_color_ptr,
          SE_COLOR_MODEL_ENUM  new_color_model,
          SE_COLOR_DATA       *new_color_ptr /* out parameter */
)
{
    SE_COLOR_STATUS_CODE_ENUM return_code;

    if (new_color_ptr == NULL)
    {
        return_code = SE_COLOR_NULL_PTR_OUT_PARAMETER;
    }
    else if (original_color_ptr == NULL)
    {
        return_code = SE_COLOR_INVALID_ORIGINAL_COLOR;
    }
    else
    {
        SE_COLOR_DATA new_color = {SE_CMY_MODEL, {0.0, 0.0, 0.0}};
        SE_COLOR_DATA temp_color;

        return_code = SE_COLOR_SUCCESS;

        switch (new_color_model)
        {
            case SE_RGB_MODEL: /* RGB is the target color model */
                 new_color.color_model = SE_RGB_MODEL;

                 switch (original_color_ptr->color_model)
                 {
                     case SE_RGB_MODEL:
                          new_color.u.rgb = original_color_ptr->u.rgb;
                          break;

                     case SE_CMY_MODEL:
                          CMY_to_RGB(&original_color_ptr->u.cmy,
                                     &new_color.u.rgb);
                          break;

                     case SE_HSV_MODEL:
                          HSV_to_RGB(&original_color_ptr->u.hsv,
                                     &new_color.u.rgb);
                          break;

                     default:
                          return_code = SE_COLOR_INVALID_ORIGINAL_COLOR;
                          break;
                 }
                 break;

            case SE_CMY_MODEL: /* CMY is the target color model */
                 new_color.color_model = SE_CMY_MODEL;

                 switch (original_color_ptr->color_model)
                 {
                     case SE_RGB_MODEL:
                          RGB_to_CMY(&original_color_ptr->u.rgb,
                                     &new_color.u.cmy);
                          break;

                     case SE_CMY_MODEL:
                          new_color.u.cmy = original_color_ptr->u.cmy;
                          break;

                     case SE_HSV_MODEL:
                          HSV_to_RGB(&original_color_ptr->u.hsv,
                                     &temp_color.u.rgb);
                          RGB_to_CMY(&temp_color.u.rgb, &new_color.u.cmy);
                          break;

                     default:
                          return_code = SE_COLOR_INVALID_ORIGINAL_COLOR;
                          break;
                 }
                 break;

            case SE_HSV_MODEL: /* HSV is the target color model */
                 new_color.color_model = SE_HSV_MODEL;

                 switch (original_color_ptr->color_model)
                 {
                     case SE_RGB_MODEL:
                          RGB_to_HSV(&original_color_ptr->u.rgb,
                                     &new_color.u.hsv);
                          break;

                     case SE_CMY_MODEL:
                          CMY_to_RGB(&original_color_ptr->u.cmy,
                                     &temp_color.u.rgb);
                          RGB_to_HSV(&temp_color.u.rgb, &new_color.u.hsv);
                          break;

                     case SE_HSV_MODEL:
                          new_color.u.hsv = original_color_ptr->u.hsv;
                          break;

                     default:
                          return_code = SE_COLOR_INVALID_ORIGINAL_COLOR;
                          break;
                 }
                 break;

            default:
                 return_code = SE_COLOR_INVALID_NEW_COLOR_MODEL;
                 break;
        } /* end of 'switch' off of new_color_model */

        if (return_code == SE_COLOR_SUCCESS)
        {
           *new_color_ptr = new_color;
        }

    } /* end of 'if-else' to check for legal color model values */

    return return_code;
} /* end SE_ConvertColorToGivenModel */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_CMYtoCMYK
 *
 *----------------------------------------------------------------------------
 */
SE_COLOR_STATUS_CODE_ENUM SE_CMYtoCMYK
(
    const SE_CMY_DATA  *original_color_ptr,
          SE_CMYK_DATA *new_color_ptr /* out parameter */
)
{
    SE_COLOR_STATUS_CODE_ENUM return_code;

    if (new_color_ptr == NULL)
    {
        return_code = SE_COLOR_NULL_PTR_OUT_PARAMETER;
    }
    else if (original_color_ptr == NULL)
    {
        return_code = SE_COLOR_INVALID_ORIGINAL_COLOR;
    }
    else
    {
        /* Set black to the minimum of the (cyan, magenta, yellow) values */
        SE_FLOAT64 black = MINIMUM(original_color_ptr->cyan,
                                MINIMUM(original_color_ptr->magenta,
                                        original_color_ptr->yellow));

        new_color_ptr->cyan    = original_color_ptr->cyan - black;
        new_color_ptr->magenta = original_color_ptr->magenta - black;
        new_color_ptr->yellow  = original_color_ptr->yellow - black;
        new_color_ptr->black   = black;

        return_code = SE_COLOR_SUCCESS;
    }
    return return_code;
} /* end SE_CMYtoCMYK */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_CMYKtoCMY
 *
 *----------------------------------------------------------------------------
 */
SE_COLOR_STATUS_CODE_ENUM SE_CMYKtoCMY
(
    const SE_CMYK_DATA *original_color_ptr,
          SE_CMY_DATA  *new_color_ptr /* out parameter */
)
{
    SE_COLOR_STATUS_CODE_ENUM return_code;

    if (new_color_ptr == NULL)
        return_code = SE_COLOR_NULL_PTR_OUT_PARAMETER;
    else if (original_color_ptr == NULL)
        return_code = SE_COLOR_INVALID_ORIGINAL_COLOR;
    else
    {
        new_color_ptr->cyan    = original_color_ptr->cyan +
                                 original_color_ptr->black;

        new_color_ptr->magenta = original_color_ptr->magenta +
                                 original_color_ptr->black;

        new_color_ptr->yellow  = original_color_ptr->yellow +
                                 original_color_ptr->black;

        return_code = SE_COLOR_SUCCESS;
    }
    return return_code;
} /* end SE_CMYKtoCMY */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_RGBtoHLS
 *
 *-----------------------------------------------------------------------------
 */
SE_COLOR_STATUS_CODE_ENUM SE_RGBtoHLS
(
    const SE_RGB_DATA *original_color_ptr,
          SE_HLS_DATA *new_color_ptr /* out parameter */
)
{
    SE_COLOR_STATUS_CODE_ENUM return_code = SE_COLOR_SUCCESS;
    SE_FLOAT64                max, min, delta;

    if (new_color_ptr == NULL)
        return_code = SE_COLOR_NULL_PTR_OUT_PARAMETER;
    else if (original_color_ptr == NULL)
        return_code = SE_COLOR_INVALID_ORIGINAL_COLOR;
    else
    {
        max = MAXIMUM(original_color_ptr->red,
                      MAXIMUM(original_color_ptr->green,
                              original_color_ptr->blue));

        min = MINIMUM(original_color_ptr->red,
                      MINIMUM(original_color_ptr->green,
                              original_color_ptr->blue));

       /*
        * Compute lightness
        */
        new_color_ptr->lightness = (max+min)/2.0;

       /*
        * Compute saturation, hue
        */
        if (max == min)
        {
            /* achromatic case, because r = g = b */
            new_color_ptr->saturation = 0.0;
            new_color_ptr->hue        = 0.0; /* really undefined */
        }
        else
        {
            delta = max - min;

           /*
            * compute saturation
            */
            if (new_color_ptr->lightness <= 0.5)
            {
                new_color_ptr->saturation = delta/(max+min);
            }
            else
            {
                new_color_ptr->saturation = delta/(2.0 - max - min);
            }

           /*
            * compute hue
            */
            if (original_color_ptr->red == max)
            {
               /*
                * resulting color is between yellow and magenta
                */
                new_color_ptr->hue = (original_color_ptr->green -
                                      original_color_ptr->blue) / delta;
            }
            else if (original_color_ptr->green == max)
            {
               /*
                * resulting color is between cyan and yellow
                */
                new_color_ptr->hue = 2.0 +
                                     ((original_color_ptr->blue -
                                       original_color_ptr->red) / delta);
            }
            else if (original_color_ptr->blue == max)
            {
               /*
                * resulting color is between magenta and cyan
                */
                new_color_ptr->hue = 4.0 +
                                     ((original_color_ptr->red -
                                       original_color_ptr->green) / delta);
            }

           /*
            * Convert hue to nonnegative degrees
            */
            if (new_color_ptr->hue < 0.0)
            {
                new_color_ptr->hue = (new_color_ptr->hue * 60.0) + 360.0;
            }
            else
            {
                new_color_ptr->hue = new_color_ptr->hue * 60.0;
            }
        }
        return_code = SE_COLOR_SUCCESS;
    }
    return return_code;
} /* end SE_RGBtoHLS */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: ComputeHLSValue
 *
 *-----------------------------------------------------------------------------
 */
static SE_FLOAT64
ComputeHLSValue
(
    SE_FLOAT64 n1,
    SE_FLOAT64 n2,
    SE_FLOAT64 hue
)
{
    SE_FLOAT64 result = 0.0;

   /*
    * Make sure hue is between 0.0 and 360.0
    */
    if (hue > 360.0)
        hue = hue - 360.0;
    else if (hue < 0.0)
        hue = hue + 360.0;

    if (hue < 60.0)
    {
        result = n1 + (n2 - n1) * hue/60.0; 
    }
    else if (hue < 180.0)
    {
        result = n2;
    }
    else if (hue < 240.0)
    {
        result = n1 + (n2 - n1) * (240.0-hue)/60.0; 
    }
    else
        result = n1;

    return result;
} /* end ComputeHLSValue */


/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_HLStoRGB
 *
 *-----------------------------------------------------------------------------
 */
SE_COLOR_STATUS_CODE_ENUM SE_HLStoRGB
(
    const SE_HLS_DATA *original_color_ptr,
          SE_RGB_DATA *new_color_ptr /* out parameter */
)
{
    SE_COLOR_STATUS_CODE_ENUM return_code = SE_COLOR_SUCCESS;

    if (new_color_ptr == NULL)
        return_code = SE_COLOR_NULL_PTR_OUT_PARAMETER;
    else if (original_color_ptr == NULL)
        return_code = SE_COLOR_INVALID_ORIGINAL_COLOR;
    else if (original_color_ptr->saturation == 0.0)
    {
        new_color_ptr->red   = original_color_ptr->lightness;
        new_color_ptr->green = original_color_ptr->lightness;
        new_color_ptr->blue  = original_color_ptr->lightness;
    }
    else
    {
        SE_FLOAT64 m1, m2;

        if (original_color_ptr->lightness <= 0.5)
        {
            m2 = original_color_ptr->lightness *
                 (1.0 + original_color_ptr->saturation);
        }
        else
        {
            m2 = original_color_ptr->lightness + original_color_ptr->saturation
                 - (original_color_ptr->lightness *
                    original_color_ptr->saturation);
        }
        m1 = 2.0 * original_color_ptr->lightness - m2;

        new_color_ptr->red   = ComputeHLSValue(m1, m2,
                                               original_color_ptr->hue+120.0);
        new_color_ptr->green = ComputeHLSValue(m1, m2,
                                               original_color_ptr->hue);
        new_color_ptr->blue  = ComputeHLSValue(m1, m2,
                                               original_color_ptr->hue-120.0);
    }
    return return_code;
} /* end SE_HLStoRGB */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_RGB_to_YIQ
 *
 *----------------------------------------------------------------------------
 */
SE_COLOR_STATUS_CODE_ENUM SE_RGBtoYIQ
(
    const SE_RGB_DATA *original_color_ptr,
          SE_YIQ_DATA *new_color_ptr /* out parameter */
)
{
    SE_COLOR_STATUS_CODE_ENUM return_code = SE_COLOR_SUCCESS;

    if (new_color_ptr == NULL)
        return_code = SE_COLOR_NULL_PTR_OUT_PARAMETER;
    else if (original_color_ptr == NULL)
        return_code = SE_COLOR_INVALID_ORIGINAL_COLOR;
    else
    {
        new_color_ptr->y = ( 0.299 * original_color_ptr->red) +
                           ( 0.587 * original_color_ptr->green) +
                           ( 0.114 * original_color_ptr->blue);

        new_color_ptr->i = ( 0.596 * original_color_ptr->red) +
                           (-0.275 * original_color_ptr->green) +
                           (-0.321 * original_color_ptr->blue);

        new_color_ptr->q = ( 0.212 * original_color_ptr->red) +
                           (-0.528 * original_color_ptr->green) +
                           ( 0.311 * original_color_ptr->blue);
    }
    return return_code;
} /* end SE_RGBtoYIQ */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_YIQ_to_RGB
 *
 *----------------------------------------------------------------------------
 */
SE_COLOR_STATUS_CODE_ENUM SE_YIQtoRGB
(
    const SE_YIQ_DATA *original_color_ptr,
          SE_RGB_DATA *new_color_ptr /* out parameter */
)
{
    SE_COLOR_STATUS_CODE_ENUM return_code = SE_COLOR_SUCCESS;

    if (new_color_ptr == NULL)
        return_code = SE_COLOR_NULL_PTR_OUT_PARAMETER;
    else if (original_color_ptr == NULL)
        return_code = SE_COLOR_INVALID_ORIGINAL_COLOR;
    else
    {
        new_color_ptr->red   = ( 1.000 * original_color_ptr->y) +
                               ( 0.956 * original_color_ptr->i) +
                               ( 0.621 * original_color_ptr->q);

        new_color_ptr->green = ( 1.000 * original_color_ptr->y) +
                               (-0.272 * original_color_ptr->i) +
                               (-0.647 * original_color_ptr->q);

        new_color_ptr->blue = ( 1.000 * original_color_ptr->y) +
                              (-1.106 * original_color_ptr->i) +
                              ( 1.703 * original_color_ptr->q);
    }
    return return_code;
} /* end SE_YIQtoRGB */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertDistance
 *
 *----------------------------------------------------------------------------
 */
SE_FLOAT64
SE_ConvertDistance
(
    SRM_ELEVATION_UNITS_ENUM source_units,
    SRM_ELEVATION_UNITS_ENUM destination_units,
    SE_FLOAT64               distance
)
{
    SE_FLOAT64 answer;

    if (source_units == destination_units)
        answer = distance;

    else if (source_units      == SRM_FEET &&
             destination_units == SRM_METERS)
        answer = SE_ConvertFeetToMeters(distance);

    else if (source_units      == SRM_METERS &&
             destination_units == SRM_FEET)
        answer = SE_ConvertMetersToFeet(distance);

    else
        answer = 0.0;

    return answer;
} /* end SE_ConvertDistance */


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertFeetToMeters
 *
 *   Converts a length measured in feet into meters.
 *
 * PARAMETERS:
 *
 *   feet - the length measurement to be converted, in feet
 *
 * RETURNS: the length measurement in meters.
 *
 *----------------------------------------------------------------------------
 */
SE_FLOAT64
SE_ConvertFeetToMeters
(
    SE_FLOAT64 feet
)
{
    return feet * SE_METERS_PER_FEET;
}


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertMetersToFeet
 *
 *   Converts a length measured in meters into feet.
 *
 * PARAMETERS:
 *
 *   meters - the length measurement to be converted, in meters
 *
 * RETURNS: the length measurement in feet.
 *
 *----------------------------------------------------------------------------
 */
SE_FLOAT64 SE_ConvertMetersToFeet
(
    SE_FLOAT64 meters
)
{
    return meters * SE_FEET_PER_METER;
}


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertDegreesToRadians
 *
 *   Converts a number of degrees to a number of radians.
 *
 * PARAMETERS:
 *
 *   degrees - the number of degrees to be converted into radians
 *
 * RETURNS: number of radians
 *
 *----------------------------------------------------------------------------
 */
SE_FLOAT64 SE_ConvertDegreesToRadians
(
    SE_FLOAT64 degrees
)
{
    return degrees * SE_RADIANS_PER_DEGREE;
}


/*
 *----------------------------------------------------------------------------
 *
 * FUNCTION: SE_ConvertRadiansToDegrees
 *
 *   Converts a number of radians to a number of degrees.
 *
 * PARAMETERS:
 *
 *   radians - the number of radians to be converted into degrees
 *
 * RETURNS: number of degrees
 *
 *----------------------------------------------------------------------------
 */
SE_FLOAT64 SE_ConvertRadiansToDegrees
(
    SE_FLOAT64 radians
)
{
    return radians * SE_DEGREES_PER_RADIAN;
}
