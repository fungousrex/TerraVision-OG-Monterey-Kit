/*
 * FILE       : se_stds.c
 *
 * PROGRAMMERS: Bill Horan (SAIC)
 *
 * DESCRIPTION:
 *   This file initializes the __xxx_infinity arrays with the IEEE definitions
 *   for positive and negative infinity, taking into account whether the
 *   machine is Big Endian or Little Endian.
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

#include "se_stds.h"

#ifdef WIN32
#undef SE_TURN_OFF_CONVERSIONS_WIN32_IMPORT
#endif

const unsigned int __se_32_pos_infinity[1] = { 0x7f800000 };
const unsigned int __se_32_neg_infinity[1] = { 0xff800000 };

#ifdef SE_BIG_ENDIAN
    const unsigned int __se_64_pos_infinity[2] = { 0x7ff00000, 0x00000000 };
    const unsigned int __se_64_neg_infinity[2] = { 0xfff00000, 0x00000000 };
#else
    const unsigned int __se_64_pos_infinity[2] = { 0x00000000, 0x7ff00000 };
    const unsigned int __se_64_neg_infinity[2] = { 0x00000000, 0xfff00000 };
#endif
