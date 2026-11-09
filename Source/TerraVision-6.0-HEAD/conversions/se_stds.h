/*
 * FILE       : se_stds.h
 *
 * PROGRAMMERS: Bill Horan (SAIC)
 *
 * DESCRIPTION:
 *
 *   CHANGE THIS FILE, IF NEEDED, TO MATCH YOUR MACHINE!
 *
 *   This file:
 *
 *     - defines whether your machine is Big or Little Endian
 *       (THIS MAY NEED TO BE CHANGED TO MATCH YOUR MACHINE)
 *
 *     - defines the 'base' SEDRIS types:
 *       (THESE MAY NEED TO BE CHANGED TO MATCH YOUR MACHINE)
 *              signed 8, 16, and 32 bit integers,
 *            unsigned 8, 16, and 32 bit integers,
 *                                32 and 64 bit floating point numbers
 *
 *     - defines a boolean type (SE_BOOLEAN), along with true and false
 *       values (SE_TRUE and SE_FALSE)
 *
 *     - defines constants that signify Positive and Negative Infinity within
 *       SEDRIS (the constants used are the appropriate IEEE values)
 *
 *     - defines constants that signify min and max values for the 'base'
 *       SEDRIS numeric types
 *
 *     - provides a convenient SE_EQUAL(a,b,epsilon) macro to test whether
 *       two floating point numbers are within epsilon of each other
 *       (to avoid the classic problem of using == to compare equality
 *        among floating point numbers)
 *
 *   YOU WILL NEED TO CHANGE THIS FILE:
 *     - IF YOUR MACHINE IS NOT BIG ENDIAN
 *     - IF YOUR MACHINE USES INTs OR FLOATs WITH DIFFERENT SIZES
 *       THAN THOSE USED IN THE PROVIDED DEFINITIONS
 *
 *   By 'default', this header file is designed for a Big Endian machine
 *   with a 32 bit integer.  The 'default' type definitions are:
 *
 *      SEDRIS Base Type     | machine specific type
 *  =========================+=======================
 *     signed  8 bit integer | signed char 
 *     signed 16 bit integer | short
 *     signed 32 bit integer | int
 *   unsigned  8 bit integer | unsigned char
 *   unsigned 16 bit integer | unsigned short
 *   unsigned 32 bit integer | unsigned int
 *   positive  8 bit integer | unsigned char 
 *   positive 16 bit integer | unsigned short
 *   positive 32 bit integer | unsigned int
 *            32 bit float   | float
 *            64 bit float   | double
 *
 *
 *   IF THESE "machine specific" TYPES ARE NOT CORRECT FOR YOUR MACHINE,
 *   THEN CHANGE THE DEFINITIONS TO THE APPROPRIATE MACHINE SPECIFIC TYPES
 *   USED IN THE "typedef" STATEMENTS TO BE CORRECT FOR YOUR MACHINE.
 *
 *   For example, if on your machine an int is 16 bits, and a long int is 32
 *   bits, then you would probably change the integer declarations to read
 *   as follows:
 *
 *      typedef signed char    SE_INT8;
 *      typedef int            SE_INT16;
 *      typedef long int       SE_INT32;
 *      
 *      typedef unsigned char  SE_UINT8;
 *      typedef unsigned int   SE_UINT16;
 *      typedef unsigned long  SE_UINT32;
 *      
 *      typedef unsigned char  SE_PINT8;
 *      typedef unsigned int   SE_PINT16;
 *      typedef unsigned long  SE_PINT32;
 *
 *   IF YOUR MACHINE IS NOT DEFINED AS A LITTLE ENDIAN MACHINE, THEN ADD THE
 *   APPROPRIATE #defined() TO THE LINE THAT READS
 *   "#if defined(CPUARCH_i386)..."
 *
 *   This file is the only SEDRIS file that should need to be changed due
 *   to machine (platform/OS) differences.
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

#ifndef _SE_STDTYPES_H_INCLUDED
#define _SE_STDTYPES_H_INCLUDED

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

#if defined(CPUARCH_i386) || defined(CPUARCH_i486)  || defined(CPUARCH_i586)  || defined(CPUARCH_i686) || defined(CPUARCH_alpha) || defined (WIN32)
#define SE_LITTLE_ENDIAN
#else
#define SE_BIG_ENDIAN
#endif

typedef signed char    SE_INT8;
typedef short          SE_INT16;
typedef int            SE_INT32;

typedef unsigned char  SE_UINT8;
typedef unsigned short SE_UINT16;
typedef unsigned int   SE_UINT32;

typedef unsigned char  SE_PINT8;
typedef unsigned short SE_PINT16;
typedef unsigned int   SE_PINT32;

typedef float          SE_FLOAT32;
typedef double         SE_FLOAT64;

typedef unsigned char  SE_BOOLEAN;

#define SE_FALSE ((SE_BOOLEAN) 0)
#define SE_TRUE  ((SE_BOOLEAN) 1)

/*
 * Used to define the max/min values.
 */
#define SE_MAX_INT8 127
#define SE_MIN_INT8 (-128)

#define SE_MAX_INT16 32767
#define SE_MIN_INT16 (-32768)

#define SE_MAX_INT32 2147483647
#define SE_MIN_INT32 (-2147483647-1)

#define SE_MAX_UINT8 255
#define SE_MIN_UINT8 0

#define SE_MAX_UINT16 65535
#define SE_MIN_UINT16 0

#define SE_MAX_UINT32 4294967295U
#define SE_MIN_UINT32 0

#define SE_MAX_PINT8 255
#define SE_MIN_PINT8 1

#define SE_MAX_PINT16 65535
#define SE_MIN_PINT16 1

#define SE_MAX_PINT32 4294967295U
#define SE_MIN_PINT32 1


/*
 * Used to define infinity values.
 */
EXPORT_CONVERSIONS_SYMBOLS_WIN32 extern const unsigned int
__se_32_pos_infinity[1];

EXPORT_CONVERSIONS_SYMBOLS_WIN32 extern const unsigned int
__se_32_neg_infinity[1];

EXPORT_CONVERSIONS_SYMBOLS_WIN32 extern const unsigned int
__se_64_pos_infinity[2];

EXPORT_CONVERSIONS_SYMBOLS_WIN32 extern const unsigned int
__se_64_neg_infinity[2];



/*
 * CONSTANTS: SE_POSITIVE_INFINITY_32, SE_NEGATIVE_INFINITY_32
 *            SE_POSITIVE_INFINITY,    SE_NEGATIVE_INFINITY
 *
 *   Infinity values.
 */
#define SE_POSITIVE_INFINITY_32 (*(SE_FLOAT32 *)__se_32_pos_infinity)
#define SE_NEGATIVE_INFINITY_32 (*(SE_FLOAT32 *)__se_32_neg_infinity)
#define SE_POSITIVE_INFINITY    (*(SE_FLOAT64 *)__se_64_pos_infinity)
#define SE_NEGATIVE_INFINITY    (*(SE_FLOAT64 *)__se_64_neg_infinity)

#define SE_EQUAL(a,b,delta) ( ((a) == (b)) || \
     ((a)-(delta) < (b) && (a)+(delta) > (b)) )

#ifdef __cplusplus
}
#endif

#endif /* ifndef/define/endif for _SE_STDTYPES_H_INCLUDED */
