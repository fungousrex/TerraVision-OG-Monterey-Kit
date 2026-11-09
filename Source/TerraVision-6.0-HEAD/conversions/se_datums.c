/*
 * SEDRIS Conversions API
 *
 * FILE       : se_datums.c
 *
 * PROGRAMMERS: Dan Toms (SRI International), Bill Horan (SAIC)
 *
 * DESCRIPTION:
 *   This file is internal to the conversions API.
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


#include "se_datums.h"


Datum_Row Datum_Table[SE_NUM_HORIZONTAL_DATUMS] =
{
 /* SE_ADI_A_HDATUM,        Adindan - applicable to Ethiopia              */
 /*                                                                        */
    {"ADI-A",                                                /* Datum Code */
     "ADINDAN, Ethiopia",                                    /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-165, 3, -11, 3, 206, 3}                               /* Parameters */
    },


 /* SE_ADI_B_HDATUM,        Adindan - applicable to Sudan                 */
 /*                                                                        */
    {"ADI-B",                                                /* Datum Code */
     "ADINDAN, Sudan",                                       /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-161, 3, -14, 5, 205, 3}                               /* Parameters */
    },


 /* SE_ADI_C_HDATUM,        Adindan - applicable to Mali                  */
 /*                                                                        */
    {"ADI-C",                                                /* Datum Code */
     "ADINDAN, Mali",                                        /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-123, 25, -20, 25, 220, 25}                            /* Parameters */
    },


 /* SE_ADI_D_HDATUM,        Adindan - applicable to Senegal               */
 /*                                                                        */
    {"ADI-D",                                                /* Datum Code */
     "ADINDAN, Senegal",                                     /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-128, 25, -18, 25, 224, 25}                            /* Parameters */
    },


 /* SE_ADI_E_HDATUM,        Adindan - applicable to Burkina Faso          */
 /*                                                                        */
    {"ADI-E",                                                /* Datum Code */
     "ADINDAN, Burkina Faso",                                /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-118, 25, -14, 25, 218, 25}                            /* Parameters */
    },


 /* SE_ADI_F_HDATUM,        Adindan - applicable to Cameroon              */
 /*                                                                        */
    {"ADI-F",                                                /* Datum Code */
     "ADINDAN, Cameroon",                                    /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-134, 25, -2, 25, 210, 25}                             /* Parameters */
    },


 /* SE_ADI_M_HDATUM,        Adindan - Mean Solution (Ethiopia & Sudan)    */
 /*                                                                        */
    {"ADI-M",                                                /* Datum Code */
     "ADINDAN, Mean Solution",                               /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-166, 5, -15, 5, 204, 3}                               /* Parameters */
    },


 /* SE_AFG_HDATUM,        Afgooye - applicable to Somalia                 */
 /*                                                                        */
    {"AFG",                                                  /* Datum Code */
     "AFGOOYE, Somalia",                                     /* Datum Name */
     SE_KA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378245,                                                         /* A */
     298.3,                                          /* Inverse Flattening */
     {-43, 25, -163, 25, 45, 25}                            /* Parameters */
    },


 /* SE_AIA_HDATUM,         Antigua Island Astro 1943 - applicable to      */
 /*                                         Antigua & Leeward Islands)     */
    {"AIA",                                                  /* Datum Code */
     "ANTIGUA ISLAND ASTRO 1943, Antigua",                   /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-270, 25, 13, 25, 62, 25}                              /* Parameters */
    },


 /* SE_AIN_A_HDATUM,        Ainel Abd 1970 - applicable to Bahrain Island */
 /*                                                                        */
    {"AIN-A",                                                /* Datum Code */
     "AIN EL ABD 1970, Bahrain Island",                      /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-150, 25, -250, 25, -1, 25}                           /* Parameters */
    },


 /* SE_AIN_B_HDATUM,        Ainel Abd 1970 - applicable to Saudi Arabia   */
 /*                                                                        */
    {"AIN-B",                                                /* Datum Code */
     "AIN EL ABD 1970, Saudi Arabia",                        /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-143, 10, -236, 10, 7, 10}                            /* Parameters */
    },


 /* SE_ANO_HDATUM,        Annal Astro 1965 - applicable to Cocos Islands  */
 /*                                                                        */
    {"ANO",                                                  /* Datum Code */
     "ANNA 1 ASTRO 1965, Cocos Islands",                     /* Datum Name */
     SE_AN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-491, 25, -22, 25, 435, 25}                            /* Parameters */
    },


 /* SE_ARF_A_HDATUM,        Arc 1950 - applicable to Botswana             */
 /*                                                                        */
    {"ARF-A",                                                /* Datum Code */
     "ARC 1950, Botswana",                                   /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-138, 3, -105, 5, -289, 3}                             /* Parameters */
    },


 /* SE_ARF_B_HDATUM,        Arc 1950 - applicable to Lesotho              */
 /*                                                                        */
    {"ARF-B",                                                /* Datum Code */
     "ARC 1950, Lesotho",                                    /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-125, 3, -108, 3, -295, 8}                           /* Parameters */
    },


 /* SE_ARF_C_HDATUM,        Arc 1950 - applicable to Malawi               */
 /*                                                                        */
    {"ARF-C",                                                /* Datum Code */
     "ARC 1950, Malawi",                                     /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-161, 9, -73, 24, -317, 8}                           /* Parameters */
    },


 /* SE_ARF_D_HDATUM,        Arc 1950 - applicable to Swaziland            */
 /*                                                                        */
    {"ARF-D",                                                /* Datum Code */
     "ARC 1950, Swaziland",                                  /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-134, 15, -105, 15, -295, 15}                          /* Parameters */
    },


 /* SE_ARF_E_HDATUM,        Arc 1950 - applicable to Zaire                */
 /*                                                                        */
    {"ARF-E",                                                /* Datum Code */
     "ARC 1950, Zaire",                                      /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-169, 25, -19, 25, -278, 25}                           /* Parameters */
    },


 /* SE_ARF_F_HDATUM,        Arc 1950 - applicable to Zambia               */
 /*                                                                        */
    {"ARF-F",                                                /* Datum Code */
     "ARC 1950, Zambia",                                     /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-147, 21, -74, 21, -283, 27}                           /* Parameters */
    },


 /* SE_ARF_G_HDATUM,        Arc 1950 - applicable to Zimbabwe             */
 /*                                                                        */
    {"ARF-G",                                                /* Datum Code */
     "ARC 1950, Zimbabwe",                                   /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-142, 5, -96, 8, -293, 11}                             /* Parameters */
    },


 /* SE_ARF_H_HDATUM,        Arc 1950 - applicable to Burundi              */
 /*                                                                        */
    {"ARF-H",                                                /* Datum Code */
     "ARC 1950, Burundi",                                    /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-153, 20, -5, 20, -292, 20}                            /* Parameters */
    },


 /* SE_ARF_M_HDATUM,        Arc 1950 - Mean Solution (Botswana, Lesotho,  */
 /*                            Malawi, Swaziland, Zaire, Zambia & Zimbabwe */
    {"ARF-M",                                                /* Datum Code */
     "ARC 1950, Mean Solution",                              /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-143, 20, -90, 33, -294, 20}                           /* Parameters */
    },


 /* SE_ARS_M_HDATUM,       Arc 1960 - Mean Solution (Kenya & Tanzania)    */
 /*                                                                        */
    {"ARS-M",                                                /* Datum Code */
     "ARC 1960, Kenya & Tanzania",                           /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-160, 20, -6, 20, -302, 20}                            /* Parameters */
    },


 /* SE_ASC_HDATUM,        Ascension Island 1958 - applicable to           */
 /*                                                Ascension Island        */
    {"ASC",                                                  /* Datum Code */
     "ASCENSION ISLAND 1958",                                /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-205, 25, 107, 25, 53, 25}                             /* Parameters */
    },


 /* SE_ASM_HDATUM,        Montserrat Island Astro 1958 - applicable to    */
 /*                                       Montserrat & Leeward Islands     */
    {"ASM",                                                  /* Datum Code */
     "MONTSERRAT ISLAND ASTRO 1958, Montserrat",             /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {174, 25, 359, 25, 365, 25}                             /* Parameters */
    },


 /* SE_ASQ_HDATUM,        Astronomical Station 1952 - applicable to       */
 /*                                                    Marcus Island       */
    {"ASQ",                                                  /* Datum Code */
     "ASTRO STATION 1952, Marcus Island",                    /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {124, 25, -234, 25, -25, 25}                            /* Parameters */
    },


 /* SE_ATF_HDATUM,        Astro Beacon E 1945 - applicable to Iwo Jima    */
 /*                                                                        */
    {"ATF",                                                  /* Datum Code */
     "ASTRO BEACON E 1845, Iwo Jima",                        /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {145, 25, 75, 25, -272, 25}                             /* Parameters */
    },


 /* SE_AUA_HDATUM,         Australian Geodetic 1966 - applicable to       */
 /*                                           Australia & Tasmania         */
    {"AUA",                                                  /* Datum Code */
     "AUSTRALIAN GEODETIC 1966",                             /* Datum Name */
     SE_AN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-133, 3, -48, 3, 148, 3}                               /* Parameters */
    },


 /* SE_AUG_HDATUM,         Australian Geodetic 1984 - applicable to       */
 /*                                           Australia & Tasmania         */
    {"AUG",                                                  /* Datum Code */
     "AUSTRALIAN GEODETIC 1984",                             /* Datum Name */
     SE_AN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-134, 2, -48, 2, 149, 2}                               /* Parameters */
    },


 /* SE_BAT_HDATUM,        Djakarta (Batavia) - applicable to Sumatra      */
 /*                                           (Indonesia)                  */
    {"BAT",                                                  /* Datum Code */
     "DJAKARTA (BATAVIA), Sumatra (Indonesia)",              /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {-377, 3, 681, 3, -50, 3}                               /* Parameters */
    },


 /* SE_BER_HDATUM,        Bermuda 1957 - applicable to Bermuda            */
 /*                                                                        */
    {"BER",                                                  /* Datum Code */
     "BERMUDA 1957, Bermuda",                                /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-73, 20, 213, 20, 296, 20}                             /* Parameters */
    },


 /* SE_BID_HDATUM,        Bissau - applicable to Guinea-Bissau            */
 /*                                                                        */
    {"BID",                                                  /* Datum Code */
     "BISSAU, Guinea-Bissau",                                /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-173, 25, 253, 25, 27, 25}                             /* Parameters */
    },


 /* SE_BOO_HDATUM,        Bogota Observatory - applicable to Colombia     */
 /*                                                                        */
    {"BOO",                                                  /* Datum Code */
     "BOGOTA OBSERVATORY, Columbia",                         /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {307, 6, 304, 5, -318, 6}                               /* Parameters */
    },


 /* SE_CAC_HDATUM,         Cape Canaveral - Mean Solution (Bahamas &      */
 /*                                                      Florida)          */
    {"CAC",                                                  /* Datum Code */
     "CAPE CANAVERAL, Bahamas & Florida",                    /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-2, 3, 151, 3, 181, 3}                                 /* Parameters */
    },


 /* SE_CAI_HDATUM,        Campo Inchauspe - applicable to Argentina       */
 /*                                                                        */
    {"CAI",                                                  /* Datum Code */
     "CAMPO INCHAUSPE 1969, Argentina",                      /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-148, 5, 136, 5, 90, 5}                                /* Parameters */
    },


 /* SE_CAO_HDATUM,        Canton Astro 1966 - applicable to Phoenix       */
 /*                                            Islands                     */
    {"CAO",                                                  /* Datum Code */
     "CANTON ASTRO 1966, Phoenix Islands",                   /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {298, 15, -304, 15, -375, 15}                           /* Parameters */
    },


 /* SE_CAP_HDATUM,        Cape - applicable to South Africa               */
 /*                                                                        */
    {"CAP",                                                  /* Datum Code */
     "CAPE, South Africa",                                   /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-136, 3, -108, 6, -292, 6}                             /* Parameters */
    },


 /* SE_CGE_HDATUM,        Carthage - applicable to Tunisia                */
 /*                                                                        */
    {"CGE",                                                  /* Datum Code */
     "CARTHAGE, Tunisia",                                    /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-263, 6, 6, 9, 431, 8}                                 /* Parameters */
    },


 /* SE_CHI_HDATUM,        Chatham Island Astro 1971 - applicable to       */
 /*                                     New Zealand (Chatham Island)       */
    {"CHI",                                                  /* Datum Code */
     "CHATHAM ISLAND ASTRO 1971, New Zealand",               /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {175, 15, -38, 15, 113, 15}                             /* Parameters */
    },


 /* SE_CHU_HDATUM,        Chua Astro - applicable to Paraguay             */
 /*                                                                        */
    {"CHU",                                                  /* Datum Code */
     "CHUA ASTRO, Paraguay",                                 /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-134, 6, 229, 9, -29, 5}                               /* Parameters */
    },


 /* SE_COA_HDATUM,        Corrego Alegre - applicable to Brazil           */
 /*                                                                        */
    {"COA",                                                  /* Datum Code */
     "CORREGO ALEGRE, Brazil",                               /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-206, 5, 172, 3, -6, 5}                                /* Parameters */
    },


 /* SE_DOB_HDATUM,        GUX1 Astro - applicable to Guadalcanal Island   */
 /*                                                                        */
    {"DOB",                                                  /* Datum Code */
     "GUX 1 ASTRO, Guadalcanal Island",                      /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {252, 25, -209, 25, -751, 25}                           /* Parameters */
    },


 /* SE_EAS_HDATUM,       Easter Island 1967 - applicable to Easter Island */
 /*                                                                        */
    {"EAS",                                                  /* Datum Code */
     "EASTER ISLAND 1967, Easter Island",                    /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {211, 25, 147, 25, 111, 25}                             /* Parameters */
    },


 /* SE_ENW_HDATUM,        Wake-Eniwetok 1960 - applicable to              */
 /*                                             Marshall Islands           */
    {"ENW",                                                  /* Datum Code */
     "WAKE-ENIWETOK 1960, Marshall Islands",                 /* Datum Name */
     SE_HO_ELLIPSOID,                                    /* Ellipsoid Code */
     6378270,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {102, 3, 52, 3, -38, 3}                                 /* Parameters */
    },


 /* SE_EUR_A_HDATUM,        European 1950 - Mean Solution (Austria,       */
 /*                              Denmark, France, FRG (Federal Republic    */
 /*                              of Germany), Netherlands & Switzerland)   */
    {"EUR-A",                                                /* Datum Code */
     "EUROPEAN 1950, Western Europe",                        /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-87, 3, -96, 3, -120, 3}                               /* Parameters */
    },


 /* SE_EUR_B_HDATUM,        European 1950 - applicable to Greece          */
 /*                                                                        */
    {"EUR-B",                                                /* Datum Code */
     "EUROPEAN 1950, Greece",                                /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-84, 25, -95, 25, -130, 25}                            /* Parameters */
    },


 /* SE_EUR_C_HDATUM,       European 1950 - applicable to Finland & Norway */
 /*                                                                        */
    {"EUR-C",                                                /* Datum Code */
     "EUROPEAN 1950, Finland & Norway",                      /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-87, 3, -95, 5, -120, 3}                               /* Parameters */
    },


 /* SE_EUR_D_HDATUM,       European 1950 - applicable to Portugal & Spain */
 /*                                                                        */
    {"EUR-D",                                                /* Datum Code */
     "EUROPEAN 1950, Portugal & Spain",                      /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-84, 5, -107, 6, -120, 3}                              /* Parameters */
    },


 /* SE_EUR_E_HDATUM,        European 1950 - applicable to Cyprus          */
 /*                                                                        */
    {"EUR-E",                                                /* Datum Code */
     "EUROPEAN 1950, Cyprus",                                /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-104, 15, -101, 15, -140, 15}                          /* Parameters */
    },


 /* SE_EUR_F_HDATUM,        European 1950 - applicable to Egypt           */
 /*                                                                        */
    {"EUR-F",                                                /* Datum Code */
     "EUROPEAN 1950, Egypt",                                 /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-130, 6, -117, 8, -151, 8}                             /* Parameters */
    },


 /* SE_EUR_G_HDATUM,       European 1950 - applicable to England, Channel */
 /*                                   Islands, Scotland, Shetland Islands  */
    {"EUR-G",                                                /* Datum Code */
     "EUROPEAN 1950, England, Channel & Scotland",           /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-86, 3, -96, 3, -120, 3}                               /* Parameters */
    },


 /* SE_EUR_H_HDATUM,        European 1950 - applicable to Iran            */
 /*                                                                        */
    {"EUR-H",                                                /* Datum Code */
     "EUROPEAN 1950, Iran",                                  /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-117, 9, -132, 12, -164, 11}                           /* Parameters */
    },


 /* SE_EUR_I_HDATUM,        European 1950 - applicable to Sardinia (Italy)*/
 /*                                                                        */
    {"EUR-I",                                                /* Datum Code */
     "EUROPEAN 1950, Sardinia (Italy)",                      /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-97, 25, -103, 25, -120, 25}                           /* Parameters */
    },


 /* SE_EUR_J_HDATUM,        European 1950 - applicable to Sicily (Italy)  */
 /*                                                                        */
    {"EUR-J",                                                /* Datum Code */
     "EUROPEAN 1950, Sicily (Italy)",                        /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-97, 20, -88, 20, -135, 20}                            /* Parameters */
    },


 /* SE_EUR_K_HDATUM,      European 1950 - applicable to England, Ireland, */
 /*                                        Scotland & Shetland Islands     */
    {"EUR-K",                                                /* Datum Code */
     "EUROPEAN 1950, England, Ireland, Scotland",            /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-86, 3, -96, 3, -120, 3}                               /* Parameters */
    },


 /* SE_EUR_L_HDATUM,        European 1950 - applicable to Malta           */
 /*                                                                        */
    {"EUR-L",                                                /* Datum Code */
     "EUROPEAN 1950, Malta",                                 /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-107, 25, -88, 25, -149, 25}                           /* Parameters */
    },


 /* SE_EUR_M_HDATUM,       European 1950 - Mean Solution (Austria,        */
 /*                               Belgium, Denmark, Finland, France,       */
 /*                               FRG (Federal Republic of Germany),       */
 /*                               Gibraltar, Greece, Italy, Luxembourg,    */
 /*                               Netherlands, Norway, Portugal, Spain,    */
 /*                               Sweden & Switzerland)                    */
    {"EUR-M",                                                /* Datum Code */
     "EUROPEAN 1950, Mean Solution",                         /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-87, 3, -98, 8, -121, 5}                               /* Parameters */
    },


 /* SE_EUS_HDATUM,       European 1979 - Mean Solution (Austria, Finland, */
 /*                                   Netherlands, Norway, Spain, Sweden & */
 /*                                   Switzerland                          */
    {"EUS",                                                  /* Datum Code */
     "EUROPEAN 1979, Mean Solution",                         /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-86, 3, -98, 3, -119, 3}                               /* Parameters */
    },


 /* SE_FAH_HDATUM,        Oman - applicable to Oman                       */
 /*                                                                        */
    {"FAH",                                                  /* Datum Code */
     "OMAN, Oman",                                           /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-346, 3, -1, 3, 224, 9}                                /* Parameters */
    },


 /* SE_FLO_HDATUM,    Observatorio Meterologico 1939 - applicable to      */
 /*                                         Corvo Flores Islands (Azores)  */
    {"FLO",                                                  /* Datum Code */
     "OBSERVATORIO MET. 1939, Corvo Flores Islands",         /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-425, 20, -169, 20, 81, 20}                            /* Parameters */
    },


 /* SE_FOT_HDATUM,        Fort Thomas 1955 - applicable to Nevis,         */
 /*                                          St. Kitts & Leeward Islands   */
    {"FOT",                                                  /* Datum Code */
     "FORT THOMAS 1955, Nevis & St. Kitts",                  /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-7, 25, 215, 25, 225, 25}                              /* Parameters */
    },


 /* SE_GAA_HDATUM,        Gan 1970 - applicable to Republic of Maldives   */
 /*                                                                        */
    {"GAA",                                                  /* Datum Code */
     "GAN 1970, Republic of Maldives",                       /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-133, 25, -321, 25, 50, 25}                            /* Parameters */
    },


 /* SE_GEO_HDATUM,        Geodetic Datum 1949 - applicable to New Zealand */
 /*                                                                        */
    {"GEO",                                                  /* Datum Code */
     "GEODETIC DATUM 1949, New Zealand",                     /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {84, 5, -22, 3, 209, 5}                                 /* Parameters */
    },


 /* SE_GIZ_HDATUM,    DOS 1968 - applicable to Gizo Island                */
 /*                                  (New Georgia Island)                  */
    {"GIZ",                                                  /* Datum Code */
     "DOS 1968, Gizo Island",                                /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {230, 25, -199, 25, -752, 25}                           /* Parameters */
    },


 /* SE_GRA_HDATUM,    Graciosa Base SW 1948 - applicable to Faial,        */
 /*                                  Graciosa, Pico, Sao Jorge & Terceira  */
 /*                                  Islands (Azores)                      */
    {"GRA",                                                  /* Datum Code */
     "GRACIOSA BASE SW 1948, Azores",                        /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-104, 3, 167, 3, -38, 3}                               /* Parameters */
    },


 /* SE_GUA_HDATUM,        Guam 1963 - applicable to Guam                  */
 /*                                                                        */
    {"GUA",                                                  /* Datum Code */
     "GUAM 1963, Guam",                                      /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-100, 3, -248, 3, 259, 3}                              /* Parameters */
    },


 /* SE_HIT_HDATUM,        Provisional South Chilean 1963 - applicable to  */
 /*                             South Chile (Near 53 degrees South)        */
 /*                                         (Hito XVIII)                   */
    {"HIT",                                                  /* Datum Code */
     "PROVISIONAL SOUTH CHILEAN 1963, S. Chile",             /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {16, 25, 196, 25, 93, 25}                               /* Parameters */
    },


 /* SE_HJO_HDATUM,        Hjorsey 1955 - applicable to Iceland            */
 /*                                                                        */
    {"HJO",                                                  /* Datum Code */
     "HJORSEY 1955, Iceland",                                /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-73, 3, 46, 3, -86, 6}                                 /* Parameters */
    },


 /* SE_HKD_HDATUM,        Hong Kong 1963 - applicable to Hong Kong        */
 /*                                                                        */
    {"HKD",                                                  /* Datum Code */
     "HONG KONG 1963, Hong Kong",                            /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-156, 25, -271, 25, -189, 25}                          /* Parameters */
    },


 /* SE_HTN_HDATUM,        Hu-Tzu-Shan - applicable to Taiwan              */
 /*                                                                        */
    {"HTN",                                                  /* Datum Code */
     "HU-TZU-SHAN, Taiwan",                                  /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-637, 15, -549, 15, -203, 15}                          /* Parameters */
    },


 /* SE_IBE_HDATUM,        Bellevue (IGN) - applicable to Efate &          */
 /*                                         Erromango Islands              */
    {"IBE",                                                  /* Datum Code */
     "BELLEVUE (IGN), Efate & Erromango Islands",            /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-127, 20, -769, 20, 472, 20}                           /* Parameters */
    },


 /* SE_IND_B_HDATUM,        Indian - applicable to Bangladesh             */
 /*                                                                        */
    {"IND-B",                                                /* Datum Code */
     "INDIAN, Bangladesh",                                   /* Datum Name */
     SE_EA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377276.345,                                                     /* A */
     300.8017,                                       /* Inverse Flattening */
     {282, 10, 726, 8, 254, 12}                              /* Parameters */
    },


 /* SE_IND_I_HDATUM,        Indian - applicable to India & Nepal          */
 /*                                                                        */
    {"IND-I",                                                /* Datum Code */
     "INDIAN, India & Nepal",                                /* Datum Name */
     SE_EC_ELLIPSOID,                                    /* Ellipsoid Code */
     6377301.243,                                                     /* A */
     300.8017,                                       /* Inverse Flattening */
     {295, 12, 736, 10, 257, 15}                             /* Parameters */
    },


 /* SE_INF_A_HDATUM,        Indian 1954 - applicable to Thailand          */
 /*                                                                        */
    {"INF-A",                                                /* Datum Code */
     "INDIAN 1954, Thailand",                                /* Datum Name */
     SE_EA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377276.345,                                                     /* A */
     300.8017,                                       /* Inverse Flattening */
     {217, 15, 823, 6, 299, 12}                              /* Parameters */
    },


 /* SE_INH_A_HDATUM,        Indian 1975 - applicable to Thailand          */
 /*                                                                        */
    {"INH-A",                                                /* Datum Code */
     "INDIAN 1975, Thailand",                                /* Datum Name */
     SE_EA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377276.345,                                                     /* A */
     300.8017,                                       /* Inverse Flattening */
     {209, 12, 818, 10, 290, 12}                             /* Parameters */
    },


 /* SE_IRL_HDATUM,        Ireland 1965 - applicable to Ireland            */
 /*                                                                        */
    {"IRL",                                                  /* Datum Code */
     "IRELAND 1965, Ireland",                                /* Datum Name */
     SE_AM_ELLIPSOID,                                    /* Ellipsoid Code */
     6377340.189,                                                     /* A */
     299.3249646,                                    /* Inverse Flattening */
     {506, 3, -122, 3, 611, 3}                               /* Parameters */
    },


 /* SE_ISG_HDATUM,        ISTS061 Astro 1968 - applicable to              */
 /*                                             South Georgia Island       */
    {"ISG",                                                  /* Datum Code */
     "ISTS 061 ASTRO 1968, S. Georgia Island",               /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-794, 25, 119, 25, -298, 25}                           /* Parameters */
    },


 /* SE_IST_HDATUM,        ISTS073 Astro 1969 - applicable to Diego Garcia */
 /*                                                                        */
    {"IST",                                                  /* Datum Code */
     "ISTS 073 ASTRO 1969, Diego Garcia",                    /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {208, 25, -435, 25, -229, 25}                           /* Parameters */
    },


 /* SE_JOH_HDATUM,        Johnston Island 1961 - applicable to Johnston   */
 /*                                               Island                   */
    {"JOH",                                                  /* Datum Code */
     "JOHNSTON ISLAND 1961, Johnston Island",                /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {189, 25, -79, 25, -202, 25}                            /* Parameters */
    },


 /* SE_KAN_HDATUM,        Kandawala - applicable to Sri Lanka             */
 /*                                                                        */
    {"KAN",                                                  /* Datum Code */
     "KANDAWALA, Sri Lanka",                                 /* Datum Name */
     SE_EA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377276.345,                                                     /* A */
     300.8017,                                       /* Inverse Flattening */
     {-97, 20, 787, 20, 86, 20}                              /* Parameters */
    },


 /* SE_KEA_HDATUM,        Kertau 1948 - applicable to West Malaysia &     */
 /*                                      Singapore                         */
    {"KEA",                                                  /* Datum Code */
     "KERTAU 1948, W. Malaysia & Singapore",                 /* Datum Name */
     SE_EE_ELLIPSOID,                                    /* Ellipsoid Code */
     6377304.063,                                                     /* A */
     300.8017,                                       /* Inverse Flattening */
     {-11, 10, 851, 8, 5, 6}                                 /* Parameters */
    },


 /* SE_KEG_HDATUM,        Kerguelen Island 1949 - applicable to Kerguelen */
 /*                                                Island                  */
    {"KEG",                                                  /* Datum Code */
     "KERGUELEN ISLAND 1949, Kerguelen Island",              /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {145, 25, -187, 25, 103, 25}                            /* Parameters */
    },


 /* SE_KUS_HDATUM,        Kusaie Astro 1951 - applicable to Caroline      */
 /*                              Islands & Fed. States of Micronesia       */
    {"KUS",                                                  /* Datum Code */
     "KUSAIE ASTRO 1951, Caroline Islands",                  /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {647, 25, 1777, 25, -1124, 25}                          /* Parameters */
    },


 /* SE_LCF_HDATUM,        L.C.5 Astro 1961 - applicable to Cayman Brac    */
 /*                                           Island                       */
    {"LCF",                                                  /* Datum Code */
     "L.C. 5 ASTRO 1961, Cayman Brac Island",                /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {42, 25, 124, 25, 147, 25}                              /* Parameters */
    },


 /* SE_LEH_HDATUM,        Leigon - applicable to Ghana                    */
 /*                                                                        */
    {"LEH",                                                  /* Datum Code */
     "LEIGON, Ghana",                                        /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-130, 2, 29, 3, 364, 2}                                /* Parameters */
    },


 /* SE_LIB_HDATUM,        Liberia 1964 - applicable to Liberia            */
 /*                                                                        */
    {"LIB",                                                  /* Datum Code */
     "LIBERIA 1964, Liberia",                                /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-90, 15, 40, 15, 88, 15}                               /* Parameters */
    },


 /* SE_LUZ_A_HDATUM,        Luzon - applicable to Philippines (excluding  */
 /*                                Mindanao Island)                        */
    {"LUZ-A",                                                /* Datum Code */
     "LUZON, Phillipines (excl. Mindanao)",                  /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-133, 8, -77, 11, -51, 9}                              /* Parameters */
    },


 /* SE_LUZ_B_HDATUM,        Luzon - applicable to Philippines             */
 /*                                              (Mindanao Island)         */
    {"LUZ-B",                                                /* Datum Code */
     "LUZON, Mindanao Island",                               /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-133, 25, -79, 25, -72, 25}                            /* Parameters */
    },


 /* SE_MAS_HDATUM,         Massawa - applicable to Ethiopia (Eritrea)     */
 /*                                                                        */
    {"MAS",                                                  /* Datum Code */
     "MASSAWA, Ethiopia (Eritrea)",                          /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {639, 25, 405, 25, 60, 25}                              /* Parameters */
    },


 /* SE_MER_HDATUM,         Merchich - applicable to Morocco               */
 /*                                                                        */
    {"MER",                                                  /* Datum Code */
     "MERCHICH, Morocco",                                    /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {31, 5, 146, 3, 47, 3}                                  /* Parameters */
    },


 /* SE_MID_HDATUM,       Midway Astro 1961 - applicable to Midway Islands */
 /*                                                                        */
    {"MID",                                                  /* Datum Code */
     "MIDWAY ASTRO 1961, Midway Islands",                    /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {912, 25, -58, 25, 1227, 25}                            /* Parameters */
    },


 /* SE_MIK_HDATUM,        Mahe 1971 - applicable to Mahe Island           */
 /*                                                                        */
    {"MIK",                                                  /* Datum Code */
     "MAHE 1971, Mahe Island",                               /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {41, 25, -220, 25, -134, 25}                            /* Parameters */
    },


 /* SE_MIN_A_HDATUM,        Minna - applicable to Cameroon                */
 /*                                                                        */
    {"MIN-A",                                                /* Datum Code */
     "MINNA, Cameroon",                                      /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-81, 25, -84, 25, 115, 25}                             /* Parameters */
    },


 /* SE_MIN_B_HDATUM,        Minna - applicable to Nigeria                 */
 /*                                                                        */
    {"MIN-B",                                                /* Datum Code */
     "MINNA, Nigeria",                                       /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-92, 3, -93, 6, 122, 5}                                /* Parameters */
    },


 /* SE_MOD_HDATUM,        Rome 1940 - applicable to Sardinia (Italy)      */
 /*                                                                        */
    {"MOD",                                                  /* Datum Code */
     "ROME 1940, Sardinia (Italy)",                          /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-225, 25, -65, 25, 9, 25}                              /* Parameters */
    },


 /* SE_MPO_HDATUM,        M'Poraloko - applicable to Gabon                */
 /*                                                                        */
    {"MPO",                                                  /* Datum Code */
     "M'PORALOKO, Gabon",                                    /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-74, 25, -130, 25, 42, 25}                             /* Parameters */
    },


 /* SE_MVS_HDATUM,       Viti Levu 1916 - applicable to Viti Levu         */
 /*                                        Island (Fiji Islands)           */
    {"MVS",                                                  /* Datum Code */
     "VITI LEVU 1916, Viti Levu Island",                     /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {51, 25, 391, 25, -36, 25}                              /* Parameters */
    },


 /* SE_NAH_A_HDATUM,     Nahrwan - applicable to Masirah Island (Oman)    */
 /*                                                                        */
    {"NAH-A",                                                /* Datum Code */
     "NAHRWAN, Masirah Island (Oman)",                       /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-247, 25, -148, 25, 369, 25}                           /* Parameters */
    },


 /* SE_NAH_B_HDATUM,        Nahrwan - applicable to United Arab Emirates  */
 /*                                                                        */
    {"NAH-B",                                                /* Datum Code */
     "NAHRWAN, United Arab Emirates",                        /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-249, 25, -156, 25, 381, 25}                           /* Parameters */
    },


 /* SE_NAH_C_HDATUM,        Nahrwan - applicable to Saudi Arabia          */
 /*                                                                        */
    {"NAH-C",                                                /* Datum Code */
     "NAHRWAN, Saudi Arabia",                                /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-243, 25, -192, 25, 477, 20}                           /* Parameters */
    },


 /* SE_NAP_HDATUM,        Naparima BWI - applicable to Trinidad & Tobago  */
 /*                                                                        */
    {"NAP",                                                  /* Datum Code */
     "NAPARIMA, Trinidad & Tobago",                          /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-10, 15, 375, 15, 165, 15}                             /* Parameters */
    },


 /* SE_NAR_A_HDATUM,        North American 1983 - applicable to Alaska    */
 /*                                (excluding Aleutian Islands)            */
    {"NAR-A",                                                /* Datum Code */
     "NORTH AMERICAN 1983, Alaska",                          /* Datum Name */
     SE_RF_ELLIPSOID,                                    /* Ellipsoid Code */
     6378137,                                                         /* A */
     298.257222101,                                  /* Inverse Flattening */
     {0, 2, 0, 2, 0, 2}                                      /* Parameters */
    },


 /* SE_NAR_B_HDATUM,        North American 1983 - applicable to Canada    */
 /*                                                                        */
    {"NAR-B",                                                /* Datum Code */
     "NORTH AMERICAN 1983, Canada",                          /* Datum Name */
     SE_RF_ELLIPSOID,                                    /* Ellipsoid Code */
     6378137,                                                         /* A */
     298.257222101,                                  /* Inverse Flattening */
     {0, 2, 0, 2, 0, 2}                                      /* Parameters */
    },


 /* SE_NAR_C_HDATUM,     North American 1983 - applicable to Continental  */
 /*                                      United States                     */
    {"NAR-C",                                                /* Datum Code */
     "NORTH AMERICAN 1983, Continental US",                  /* Datum Name */
     SE_RF_ELLIPSOID,                                    /* Ellipsoid Code */
     6378137,                                                         /* A */
     298.257222101,                                  /* Inverse Flattening */
     {0, 2, 0, 2, 0, 2}                                      /* Parameters */
    },


 /* SE_NAR_D_HDATUM,        North American 1983 - applicable to Mexico &  */
 /*                                              Central America           */
    {"NAR-D",                                                /* Datum Code */
     "NORTH AMERICAN 1983, Mexico",                          /* Datum Name */
     SE_RF_ELLIPSOID,                                    /* Ellipsoid Code */
     6378137,                                                         /* A */
     298.257222101,                                  /* Inverse Flattening */
     {0, 2, 0, 2, 0, 2}                                      /* Parameters */
    },


 /* SE_NAS_A_HDATUM,     North American 1927 - Eastern United States      */
 /*                        (Alabama, Connecticut, Delaware, District of    */
 /*                        Columbia, Florida, Georgia, Illinois, Indiana,  */
 /*                        Kentucky, Louisiana, Maine, Maryland,           */
 /*                        Massachusetts, Michigan, Minnesota,             */
 /*                        Mississippi, Missouri, New Hampshire, New       */
 /*                        Jersey, New York, North Carolina, Ohio,         */
 /*                        Pennsylvania, Rhode Island, South Carolina,     */
 /*                        Tennessee, Vermont, Virginia, West Virginia     */
 /*                        & Wisconsin)                                    */
    {"NAS-A",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Eastern US",                      /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-9, 5, 161, 5, 179, 8}                                 /* Parameters */
    },


 /* SE_NAS_B_HDATUM,     North American 1927 - Western United States      */
 /*                        (Arizona, Arkansas, California, Colorado,       */
 /*                        Idaho, Iowa, Kansas, Montana, Nebraska,         */
 /*                        Nevada, New Mexico, North Dakota, Oklahoma,     */
 /*                        Oregon, South Dakota, Texas, Utah, Washington   */
 /*                        & Wyoming)                                      */
    {"NAS-B",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Western US",                      /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
    {-8, 5, 159, 3, 175, 3}                                  /* Parameters */
    },


 /* SE_NAS_C_HDATUM,     North American 1927 - Mean Solution for          */
 /*                        Continental United States                       */
    {"NAS-C",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Mean Solution",                   /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-8, 5, 160, 5, 176, 6}                                 /* Parameters */
    },


 /* SE_NAS_D_HDATUM,  North American 1927 - applicable to Alaska          */
 /*                                      (excluding Aleutian Islands)      */
    {"NAS-D",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Alaska (excl. Aleutians)",        /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-5, 5, 135, 9, 172, 5}                                 /* Parameters */
    },


 /* SE_NAS_E_HDATUM,     North American 1927 - Mean Solution for Canada   */
 /*                                      (including Newfoundland)          */
    {"NAS-E",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Canada Mean",                     /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-10, 15, 158, 11, 187, 6}                              /* Parameters */
    },


 /* SE_NAS_F_HDATUM,        North American 1927 - applicable to Canada    */
 /*                                       (Alberta & British Columbia)     */
    {"NAS-F",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Alberta & BC",                    /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-7, 8, 162, 8, 188, 6}                                 /* Parameters */
    },


 /* SE_NAS_G_HDATUM,     North American 1927 - applicable to Eastern      */
 /*                                  Canada (New Brunswick, Newfoundland,  */
 /*                                       Nova Scotia & Quebec)            */
    {"NAS-G",                                                /* Datum Code */
     "NORTH AMERICAN 1927, E. Canada",                       /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-22, 6, 160, 6, 190, 3}                                /* Parameters */
    },


 /* SE_NAS_H_HDATUM,        North American 1927 - applicable to Canada    */
 /*                                       (Manitoba & Ontario)             */
    {"NAS-H",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Manitoba & Ontario",              /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
    {-9, 9, 157, 5, 184, 5}                                  /* Parameters */
    },


 /* SE_NAS_I_HDATUM,        North American 1927 - applicable to Canada    */
 /*                                 (Northwest Territories & Saskatchewan) */
    {"NAS-I",                                                /* Datum Code */
     "NORTH AMERICAN 1927, NW Territories",                  /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {4, 5, 159, 5, 188, 3}                                  /* Parameters */
    },


 /* SE_NAS_J_HDATUM,        North American 1927 - applicable to Canada    */
 /*                                              (Yukon)                   */
    {"NAS-J",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Yukon",                           /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-7, 5, 139, 8, 181, 3}                                 /* Parameters */
    },


 /* SE_NAS_L_HDATUM,         North American 1927 - applicable to Mexico   */
 /*                                                                        */
    {"NAS-L",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Mexico",                          /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
    {-12, 8, 130, 6, 190, 6}                                 /* Parameters */
    },


 /* SE_NAS_N_HDATUM,     North American 1927 - Central America (Belize,   */
 /*                                Costa Rica, El Salvador, Guatemala,     */
 /*                                Honduras & Nicaragua                    */
    {"NAS-N",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Central America",                 /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {0, 8, 125, 3, 194, 5}                                  /* Parameters */
    },


 /* SE_NAS_O_HDATUM,      North American 1927 - applicable to Canal Zone  */
 /*                                                                        */
    {"NAS-O",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Canal Zone",                      /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {0, 20, 125, 20, 201, 20}                               /* Parameters */
    },


 /* SE_NAS_P_HDATUM,     North American 1927 - Carribean (Antigua Island, */
 /*                            Barbados, Barbuda, Caicos Islands, Cuba,    */
 /*                            Dominican Republic, Grand Cayman, Jamaica   */
 /*                            & Turks Islands                             */
    {"NAS-P",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Caribbean",                       /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-3, 3, 142, 9, 183, 12}                                /* Parameters */
    },


 /* SE_NAS_Q_HDATUM,        North American 1927 - applicable to Bahamas   */
 /*                                       (except San Salavador Island)    */
    {"NAS-Q",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Bahamas",                         /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-4, 5, 154, 3, 178, 5}                                 /* Parameters */
    },


 /* SE_NAS_R_HDATUM,     North American 1927 - applicable to San          */
 /*                                      Salavador Island)                 */
    {"NAS-R",                                                /* Datum Code */
     "NORTH AMERICAN 1927, San Salvador Island",             /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {1, 25, 140, 25, 165, 25}                               /* Parameters */
    },


 /* SE_NAS_T_HDATUM,        North American 1927 - applicable to Cuba      */
 /*                                                                        */
    {"NAS-T",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Cuba",                            /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-9, 25, 152, 25, 178, 25}                              /* Parameters */
    },


 /* SE_NAS_U_HDATUM,        North American 1927 - applicable to Greenland */
 /*                                       (Hayes Peninsula)                */
    {"NAS-U",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Greenland",                       /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {11, 25, 114, 25, 195, 25}                              /* Parameters */
    },


 /* SE_OEG_HDATUM,        Old Egyptian 1907 - applicable to Egypt         */
 /*                                                                        */
    {"OEG",                                                  /* Datum Code */
     "OLD EGYPTIAN 1907, Egypt",                             /* Datum Name */
     SE_HE_ELLIPSOID,                                    /* Ellipsoid Code */
     6378200,                                                         /* A */
     298.3,                                          /* Inverse Flattening */
     {-130, 3, 110, 6, -13, 8}                               /* Parameters */
    },


 /* SE_OGB_A_HDATUM,      Ordnance Survey Great Britain 1936 - applicable */
 /*                                                          to England    */
    {"OGB-A",                                                /* Datum Code */
     "ORDNANCE GB 1936, England",                            /* Datum Name */
     SE_AA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377563.396,                                                     /* A */
     299.3249646,                                    /* Inverse Flattening */
     {371, 5, -112, 5, 434, 6}                               /* Parameters */
    },


 /* SE_OGB_B_HDATUM,      Ordnance Survey Great Britain 1936 - applicable */
 /*                                         to England, Isle of Man, Wales */
    {"OGB-B",                                                /* Datum Code */
     "ORDNANCE GB 1936, England & Wales",                    /* Datum Name */
     SE_AA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377563.396,                                                     /* A */
     299.3249646,                                    /* Inverse Flattening */
     {371, 10, -111, 10, 434, 15}                            /* Parameters */
    },


 /* SE_OGB_C_HDATUM,      Ordnance Survey Great Britain 1936 - applicable */
 /*                                         to Scotland & Shetland Islands */
    {"OGB-C",                                                /* Datum Code */
     "ORDNANCE GB 1936, Scotland & Shetland Isles",          /* Datum Name */
     SE_AA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377563.396,                                                     /* A */
     299.3249646,                                    /* Inverse Flattening */
     {384, 10, -111, 10, 425, 10}                            /* Parameters */
    },


 /* SE_OGB_D_HDATUM,      Ordnance Survey Great Britain 1936 - applicable */
 /*                                                             to Wales   */
    {"OGB-D",                                                /* Datum Code */
     "ORDNANCE GB 1936, Wales",                              /* Datum Name */
     SE_AA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377563.396,                                                     /* A */
     299.3249646,                                    /* Inverse Flattening */
     {370, 20, -108, 20, 434, 20}                            /* Parameters */
    },


 /* SE_OGB_M_HDATUM,     Ordnance Survey Great Britain 1936 - Mean        */
 /*                              Solution (England, Isle of Man,           */
 /*                              Scotland, Shetland Islands & Wales        */
    {"OGB-M",                                                /* Datum Code */
     "ORDNANCE GB 1936, Mean Solution",                      /* Datum Name */
     SE_AA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377563.396,                                                     /* A */
     299.3249646,                                    /* Inverse Flattening */
     {375, 10, -111, 10, 431, 15}                            /* Parameters */
    },
 /* SE_OHA_A_HDATUM,        Old Hawaiian - applicable to Hawaii           */
 /*                                                                        */
    {"OHA-A",                                                /* Datum Code */
     "OLD HAWAI'IAN (CC), Hawaii",                           /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {89, 25, -279, 25, -183, 25}                            /* Parameters */
    },


 /* SE_OHA_B_HDATUM,        Old Hawaiian - applicable to Kauai            */
 /*                                                                        */
    {"OHA-B",                                                /* Datum Code */
     "OLD HAWAI'IAN (CC), Kauai",                            /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {45, 20, -290, 20, -172, 20}                            /* Parameters */
    },


 /* SE_OHA_C_HDATUM,        Old Hawaiian - applicable to Maui             */
 /*                                                                        */
    {"OHA-C",                                                /* Datum Code */
     "OLD HAWAI'IAN (CC), Maui",                             /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {65, 25, -290, 25, -190, 25}                            /* Parameters */
    },


 /* SE_OHA_D_HDATUM,        Old Hawaiian - applicable to Oahu             */
 /*                                                                        */
    {"OHA-D",                                                /* Datum Code */
     "OLD HAWAI'IAN (CC), Oahu",                             /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {58, 10, -283, 6, -182, 6}                              /* Parameters */
    },


 /* SE_OHA_M_HDATUM,        Old Hawaiian - Mean Solution                  */
 /*                                                                        */
    {"OHA-M",                                                /* Datum Code */
     "OLD HAWAI'IAN (CC), Mean Solution",                    /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {61, 25, -285, 20, -181, 20}                            /* Parameters */
    },


 /* SE_PHA_HDATUM,        Ayabelle Lighthouse - applicable to Djibouti    */
 /*                                                                        */
    {"PHA",                                                  /* Datum Code */
     "AYABELLA LIGHTHOUSE, Djibouti",                        /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-79, 25, -129, 25, 145, 25}                            /* Parameters */
    },


 /* SE_PIT_HDATUM,        Pitcairn Astro 1967 - applicable to             */
 /*                                              Pitcairn Island           */
    {"PIT",                                                  /* Datum Code */
     "PITCAIRN ASTRO 1967, Pitcairn Island",                 /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {185, 25, 165, 25, 42, 25}                              /* Parameters */
    },


 /* SE_PLN_HDATUM,       Picodelas Nieves - applicable to Canary Islands  */
 /*                                                                        */
    {"PLN",                                                  /* Datum Code */
     "PICO DE LAS NIEVES, Canary Islands",                   /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-307, 25, -92, 25, 127, 25}                            /* Parameters */
    },


 /* SE_POS_HDATUM,       Porto Santo 1936 - applicable to Porto Santo &   */
 /*                                                  Madeira Islands       */
    {"POS",                                                  /* Datum Code */
     "PORTO SANTO 1936, Madeira Islands",                    /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-499, 25, -249, 25, 314, 25}                           /* Parameters */
    },


 /* SE_PRP_A_HDATUM,      Provisional South American 1956 - applicable to */
 /*                                                          Bolivia       */
    {"PRP-A",                                                /* Datum Code */
     "PROV. S AMERICAN 1956, Bolivia",                       /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-270, 5, 188, 11, -388, 14}                            /* Parameters */
    },


 /* SE_PRP_B_HDATUM,      Provisional South American 1956 - applicable to */
 /*                                  Northern Chile (near 19 degrees South)*/
    {"PRP-B",                                                /* Datum Code */
     "PROV. S AMERICAN 1956, Northern Chile",                /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-270, 25, 183, 25, -390, 25}                           /* Parameters */
    },


 /* SE_PRP_C_HDATUM,      Provisional South American 1956 - applicable to */
 /*                                  Southern Chile (near 43 degrees South)*/
    {"PRP-C",                                                /* Datum Code */
     "PROV. S AMERICAN 1956, Southern Chile",                /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-305, 20, 243, 20, -442, 20}                           /* Parameters */
    },


 /* SE_PRP_D_HDATUM,      Provisional South American 1956 - applicable to */
 /*                                                          Colombia      */
    {"PRP-D",                                                /* Datum Code */
     "PROV. S AMERICAN 1956, Colombia",                      /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-282, 15, 169, 15, -371, 15}                           /* Parameters */
    },


 /* SE_PRP_E_HDATUM,      Provisional South American 1956 - applicable to */
 /*                                                          Ecuador       */
    {"PRP-E",                                                /* Datum Code */
     "PROV. S AMERICAN 1956, Ecuador",                       /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-278, 3, 171, 5, -367, 3}                              /* Parameters */
    },


 /* SE_PRP_F_HDATUM,      Provisional South American 1956 - applicable to */
 /*                                                          Guyana        */
    {"PRP-F",                                                /* Datum Code */
     "PROV. S AMERICAN 1956, Guyana",                        /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-298, 6, 159, 14, -369, 5}                             /* Parameters */
    },


 /* SE_PRP_G_HDATUM,      Provisional South American 1956 - applicable to */
 /*                                                          Peru          */
    {"PRP-G",                                                /* Datum Code */
     "PROV. S AMERICAN 1956, Peru",                          /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-279, 6, 175, 8, -379, 12}                             /* Parameters */
    },


 /* SE_PRP_H_HDATUM,      Provisional South American 1956 - applicable to */
 /*                                                          Venezuela     */
    {"PRP-H",                                                /* Datum Code */
     "PROV. S AMERICAN 1956, Venezuela",                     /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-295, 9, 173, 14, -371, 15}                            /* Parameters */
    },


 /* SE_PRP_M_HDATUM,     Provisional South American 1956 - Mean Solution  */
 /*                           (Bolivia, Chile, Colombia, Ecuador, Guyana,  */
 /*                           Peru & Venezuela)                            */
   {"PRP-M",                                                 /* Datum Code */
     "PROV. S AMERICAN 1956, Mean Solution",                 /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-288, 17, 175, 27, -376, 27}                           /* Parameters */
    },


 /* SE_PTB_HDATUM,        Point 58 - Mean Solution (Burkina Faso & Niger) */
 /*                                                                        */
    {"PTB",                                                  /* Datum Code */
     "POINT 58, Burkina Faso & Niger",                       /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-106, 25, -129, 25, 165, 25}                           /* Parameters */
    },


 /* SE_PTN_HDATUM,        Pointe Noire 1948 - applicable to Congo         */
 /*                                                                        */
    {"PTN",                                                  /* Datum Code */
     "POINT NOIRE 1948, Congo",                              /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-148, 25, 51, 25, -291, 25}                            /* Parameters */
    },


 /* SE_PUR_HDATUM,        Puerto Rico - applicable to Puerto Rico &       */
 /*                                      Virgin Islands                    */
    {"PUR",                                                  /* Datum Code */
     "PUERTO RICO, Puerto Rico & Virgin Islands",            /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {11, 3, 72, 3, -101, 3}                                 /* Parameters */
    },


 /* SE_QAT_HDATUM,        Qatar National - applicable to Qatar            */
 /*                                                                        */
    {"QAT",                                                  /* Datum Code */
     "QATAR NATIONAL, Qatar",                                /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-128, 20, -283, 20, 22, 20}                            /* Parameters */
    },


 /* SE_QUO_HDATUM,       Qornoq - applicable to South Greenland           */
 /*                                                                        */
    {"QUO",                                                  /* Datum Code */
     "QORNOQ, South Greenland",                              /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {164, 25, 138, 25, -189, 32}                            /* Parameters */
    },


 /* SE_REU_HDATUM,        Reunion - applicable to Mascarene Islands       */
 /*                                                                        */
    {"REU",                                                  /* Datum Code */
     "REUNION, Mascarene Islands",                           /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {94, 25, -948, 25, -1262, 25}                           /* Parameters */
    },


 /* SE_SAE_HDATUM,        Santo (DOS) 1965 - applicable to Espirito       */
 /*                                           Santo Island                 */
    {"SAE",                                                  /* Datum Code */
     "SANTO (DOS) 1965, Espirito Santo Island",              /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {170, 22, 42, 25, 84, 25}                               /* Parameters */
    },


 /* SE_SAN_A_HDATUM,       South American 1969 - applicable to Argentina  */
 /*                                                                        */
    {"SAN-A",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Argentina",                       /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-62, 5, -1, 5, -37, 5}                                 /* Parameters */
    },


 /* SE_SAN_B_HDATUM,        South American 1969 - applicable to Bolivia   */
 /*                                                                        */
    {"SAN-B",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Bolivia",                         /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-61, 15, 2, 15, -48, 15}                               /* Parameters */
    },


 /* SE_SAN_C_HDATUM,        South American 1969 - applicable to Brazil    */
 /*                                                                        */
    {"SAN-C",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Brazil",                          /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-60, 3, -2, 5, -41, 5}                                 /* Parameters */
    },


 /* SE_SAN_D_HDATUM,        South American 1969 - applicable to Chile     */
 /*                                                                        */
    {"SAN-D",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Chile",                           /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-75, 15, -1, 8, -44, 11}                               /* Parameters */
    },


 /* SE_SAN_E_HDATUM,        South American 1969 - applicable to Colombia  */
 /*                                                                        */
    {"SAN-E",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Colombia",                        /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-44, 6, 6, 6, -36, 5}                                  /* Parameters */
    },


 /* SE_SAN_F_HDATUM,        South American 1969 - applicable to Ecuador   */
 /*                                       (excluding Galapagos Islands)    */
    {"SAN-F",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Ecuador",                         /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-48, 3, 3, 3, -44, 3}                                  /* Parameters */
    },


 /* SE_SAN_G_HDATUM,        South American 1969 - applicable to Guyana    */
 /*                                                                        */
    {"SAN-G",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Guyana",                          /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-53, 9, 3, 5, -47, 5}                                  /* Parameters */
    },


 /* SE_SAN_H_HDATUM,        South American 1969 - applicable to Paraguay  */
 /*                                                                        */
    {"SAN-H",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Paraguay",                        /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-61, 15, 2, 15, -33, 15}                               /* Parameters */
    },


 /* SE_SAN_I_HDATUM,        South American 1969 - applicable to Peru      */
 /*                                                                        */
    {"SAN-I",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Peru",                            /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-58, 5, 0, 5, -44, 5}                                  /* Parameters */
    },


 /* SE_SAN_J_HDATUM,        South American 1969 - applicable to Ecuador   */
 /*                                       (Baltra &  Galapagos Islands)    */
    {"SAN-J",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Baltra & Galapagos",              /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-47, 25, 26, 25, -42, 25}                              /* Parameters */
    },


 /* SE_SAN_K_HDATUM,      South American 1969 - applicable to Trinidad &  */
 /*                                              Tobago                    */
    {"SAN-K",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Trinidad & Tobago",               /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-45, 25, 12, 25, -33, 25}                              /* Parameters */
    },


 /* SE_SAN_L_HDATUM,        South American 1969 - applicable to Venezuela */
 /*                                                                        */
    {"SAN-L",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Venezuela",                       /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-45, 3, 8, 6, -33, 3}                                  /* Parameters */
    },


 /* SE_SAN_M_HDATUM,     South American 1969 - Mean Solution (Argentina,  */
 /*                            Bolivia, Brazil, Chile, Colombia, Ecuador,  */
 /*                            Guyana, Paraguay, Peru, Trinidad & Tobago,  */
 /*                            Venezuela                                   */
    {"SAN-M",                                                /* Datum Code */
     "SOUTH AMERICAN 1969, Mean Solution",                   /* Datum Name */
     SE_SA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.25,                                         /* Inverse Flattening */
     {-57, 15, 1, 6, -41, 9}                              /* Parameters */
    },


 /* SE_SAO_HDATUM,       Sao Braz - applicable to Sao Miguel & Santa      */
 /*                                           Maria Islands (Azores)       */
    {"SAO",                                                  /* Datum Code */
     "SAO BRAZ, Santa Maria Islands",                        /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-203, 25, 141, 25, 53, 25}                             /* Parameters */
    },


 /* SE_SAP_HDATUM,        Sapper Hill 1943 - applicable to East Falkland  */
 /*                                           Island                       */
    {"SAP",                                                  /* Datum Code */
     "SAPPER HILL 1943, East Falkland Island",               /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-355, 1, 21, 1, 72, 1}                                /* Parameters */
    },


 /* SE_SCK_HDATUM,        Schwarzeck - applicable to Namibia              */
 /*                                                                        */
    {"SCK",                                                  /* Datum Code */
     "SCHWARZECK, Namibia",                                  /* Datum Name */
     SE_BN_ELLIPSOID,                                    /* Ellipsoid Code */
     6377483.865,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {616, 20, 97, 20, -251, 20}                             /* Parameters */
    },


 /* SE_SGM_HDATUM,        Selvagem Grande 1938 - applicable to Salvage    */
 /*                                               Islands                  */
    {"SGM",                                                  /* Datum Code */
     "SELVAGEM GRADE 1938, Salvage Islands",                 /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-289, 25, -124, 25, 60, 25}                            /* Parameters */
    },


 /* SE_SHB_HDATUM,      Astro DOS 71/4 - applicable to St. Helena Island  */
 /*                                                                        */
    {"SHB",                                                  /* Datum Code */
     "ASTRO DOS 71/4, St. Helena Island",                    /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-320, 25, 550, 25, -494, 25}                           /* Parameters */
    },


 /* SE_SOA_HDATUM,        South Asia - applicable to Singapore            */
 /*                                                                        */
    {"SOA",                                                  /* Datum Code */
     "SOUTH ASIA, Singapore",                                /* Datum Name */
     SE_FA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378155,                                                         /* A */
     298.3,                                          /* Inverse Flattening */
     {7, 25, -10, 25, -26, 25}                               /* Parameters */
    },


 /* SE_TDC_HDATUM,        Tristan Astro 1968 - applicable to Tristan da   */
 /*                                             Cunha                      */
    {"TDC",                                                  /* Datum Code */
     "TRISTAN ASTRO 1968, Tristan da Cunha",                 /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-632, 25, 438, 25, -609, 25}                           /* Parameters */
    },


 /* SE_TIL_HDATUM,        Timbalai 1948 - applicable to Brunei & East     */
 /*                                        Malaysia (Sabah & Sarawak)      */
    {"TIL",                                                  /* Datum Code */
     "TIMBALAI 1948, Brunei & E Malaysia",                   /* Datum Name */
     SE_EB_ELLIPSOID,                                    /* Ellipsoid Code */
     6377298.556,                                                     /* A */
     300.8017,                                       /* Inverse Flattening */
     {-679, 10, 669, 10, -48, 12}                            /* Parameters */
    },


 /* SE_TOY_A_HDATUM,        Tokyo - applicable to Japan                   */
 /*                                                                        */
    {"TOY-A",                                                /* Datum Code */
     "TOKYO, Japan",                                         /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {-148, 8, 507, 5, 685, 8}                               /* Parameters */
    },


 /* SE_TOY_B_HDATUM,        Tokyo - applicable to South Korea             */
 /*                                                                        */
    {"TOY-B",                                                /* Datum Code */
     "TOKYO, South Korea",                                   /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {-146, 8, 507, 5, 687, 8}                               /* Parameters */
    },


 /* SE_TOY_C_HDATUM,        Tokyo - applicable to Okinawa                 */
 /*                                                                        */
    {"TOY-C",                                                /* Datum Code */
     "TOKYO, Okinawa",                                       /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {-158, 20, 507, 5, 676, 20}                             /* Parameters */
    },


 /* SE_TOY_M_HDATUM,     Tokyo - Mean Solution (Japan, Korea & Okinawa)   */
 /*                                                                        */
    {"TOY-M",                                                /* Datum Code */
     "TOKYO, Mean Solution",                                 /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {-148, 20, 507, 5, 685, 20}                            /* Parameters */
    },


 /* SE_TRN_HDATUM,        Astro Tern Island (FRIG) 1961 - applicable to   */
 /*                                                        Tern Island     */
    {"TRN",                                                  /* Datum Code */
     "ASTRO TERN ISLAND (FRIG) 1961, Tern Island",           /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {114, 25, -116, 25, -333, 25}                           /* Parameters */
    },


 /* SE_W72_HDATUM,        WGS 1972 - Global Definition                    */
 /*                                                                        */
    {"W72",                                                  /* Datum Code */
     "WGS 1972",                                             /* Datum Name */
     SE_WD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378135,                                                         /* A */
     298.26,                                         /* Inverse Flattening */
     {0, 0, 0, 0, 0, 0}                                      /* Parameters */
    },


 /* SE_W84_HDATUM,        WGS 1984 - Global Definition                    */
 /*                                                                        */
    {"W84",                                                  /* Datum Code */
     "WGS 1984",                                             /* Datum Name */
     SE_WE_ELLIPSOID,                                    /* Ellipsoid Code */
     6378137,                                                         /* A */
     298.257223563,                                  /* Inverse Flattening */
     {0, 0, 0, 0, 0, 0}                                      /* Parameters */
    },


 /* SE_WAK_HDATUM,        Wake Island Astro 1952 - applicable to Wake     */
 /*                                                        Atoll           */
    {"WAK",                                                  /* Datum Code */
     "WAKE ISLAND ASTRO 1952, Wake Atoll",                   /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {276, 25, -57, 25, 149, 25}                             /* Parameters */
    },


 /* SE_ZAN_HDATUM        Zanderij - applicable to Suriname                */
 /*                                                                        */
    {"ZAN",                                                  /* Datum Code */
     "ZANDERIJ, Suriname",                                   /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-265, 5, 120, 5, -358, 8}                              /* Parameters */
    },


 /* SE_SPHERICAL_ACM_HDATUM        used by ACMES                          */
 /*                                                                        */
    {"ACM",                                                  /* Datum Code */
     "ACMES, Spherical",                                     /* Datum Name */
     SE_ACM_SPHEROID,                                    /* Ellipsoid Code */
     6371221.3,                                                       /* A */
     0,                                              /* Inverse Flattening */
     {0, 0, 0, 0, 0, 0}                                      /* Parameters */
   },


 /* SE_SPHERICAL_COA_HDATUM        used by COAMPS                         */
 /*                                                                        */
    {"COA",                                                  /* Datum Code */
     "COAMPS, Spherical",                                    /* Datum Name */
     SE_COA_SPHEROID,                                    /* Ellipsoid Code */
     6371229,                                                         /* A */
     0,                                              /* Inverse Flattening */
     {0, 0, 0, 0, 0, 0}                                      /* Parameters */
   },


 /* SE_SPHERICAL_NOG_HDATUM        used by NOGAPS                         */
 /*                                                                        */
    {"NOG",                                                  /* Datum Code */
     "NOGAPS Spherical",                                     /* Datum Name */
     SE_NOG_SPHEROID,                                    /* Ellipsoid Code */
     6371000,                                                         /* A */
     0,                                              /* Inverse Flattening */
     {0, 0, 0, 0, 0, 0}                                      /* Parameters */
   },


 /* SE_SPHERICAL_MFE_HDATUM        used by Multigen Flat Earth            */
 /*                                                                        */
    {"MFE",                                                  /* Datum Code */
     "MULTIGEN FLAT EARTH, Spherical",                       /* Datum Name */
     SE_MFE_SPHEROID,                                    /* Ellipsoid Code */
     6366707.02,                                                      /* A */
     0,                                              /* Inverse Flattening */
     {0, 0, 0, 0, 0, 0}                                      /* Parameters */
   },


 /* SE_SPHERICAL_MOD_M_HDATUM      used by MODTRAN                        */
 /*                                                                        */
    {"MOD_M",                                                /* Datum Code */
     "MODTRAN Midlatitude, Spherical",                       /* Datum Name */
     SE_MOD_M_SPHEROID,                                  /* Ellipsoid Code */
     6371230,                                                         /* A */
     0,                                              /* Inverse Flattening */
     {0, 0, 0, 0, 0, 0}                                      /* Parameters */
   },


 /* SE_SPHERICAL_MOD_S_HDATUM      used by MODTRAN                        */
 /*                                                                        */
    {"MOD_S",                                                /* Datum Code */
     "MODTRAN Subarctic, Spherical",                         /* Datum Name */
     SE_MOD_S_SPHEROID,                                  /* Ellipsoid Code */
     6356910,                                                         /* A */
     0,                                              /* Inverse Flattening */
     {0, 0, 0, 0, 0, 0}                                      /* Parameters */
   },


 /* SE_SPHERICAL_MOD_T_HDATUM      used by MODTRAN                        */
 /*                                                                        */
    {"MOD_T",                                                /* Datum Code */
     "MODTRAN Tropical, Spherical",                          /* Datum Name */
     SE_MOD_T_SPHEROID,                                  /* Ellipsoid Code */
     6378390,                                                         /* A */
     0,                                              /* Inverse Flattening */
     {0, 0, 0, 0, 0, 0}                                      /* Parameters */
   },


 /* SE_SPHERICAL_MM5_HDATUM        used by MM5 (AFWA)                     */
 /*                                                                        */
    {"MM5",                                                  /* Datum Code */
     "MM5 AFWA, Spherical",                                  /* Datum Name */
     SE_MM5__SPHEROID,                                   /* Ellipsoid Code */
     6370000,                                                         /* A */
     0,                                              /* Inverse Flattening */
     {0, 0, 0, 0, 0, 0}                                      /* Parameters */
    },


 /* SE_AMA_HDATUM,       American Samoa 1962 - applicable to American     */
 /*                                              Samoa Islands             */
    {"AMA",                                                  /* Datum Code */
     "AMERICAN SAMOA 1962",                                  /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-115, 25, 118, 25, 426, 25}                            /* Parameters */
    },


 /* SE_ARS_A_HDATUM,     Arc 1960 - applicable to Kenya                   */
 /*                                                                        */
    {"ARS-A",                                                /* Datum Code */
     "ARC 1960, Kenya",                                      /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-157, 4, -2, 3, -299, 3}                               /* Parameters */
    },


 /* SE_ARS_B_HDATUM,     Arc 1960 - applicable to Tanzania                */
 /*                                                                        */
    {"ARS-B",                                                /* Datum Code */
     "ARC 1960, Tanzania",                                   /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-175, 6, -23, 9, -303, 10}                             /* Parameters */
    },


 /* SE_BUR_HDATUM,       Bukit Rimpah - applicable to Bangka & Belitung   */
 /*                                              Islands (Indonesia)       */
    {"BUR",                                                  /* Datum Code */
     "BUKIT RIMPAH, Banka & Belitung",                       /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {-384, -1, 664, -1, -48, -1}                            /* Parameters */
    },


 /* SE_CAZ_HDATUM,       Camp Area Astro - applicable to McMurdo Camp     */
 /*                                              Area, Antartica           */
    {"CAZ",                                                  /* Datum Code */
     "CAMP AREA ASTRO, McMurdo Camp",                        /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-104, -1, -129, -1, 239, -1}                           /* Parameters */
    },


 /* SE_CCD_HDATUM,       S-JTSK - applicable to Czech Republic & Slovakia */
 /*                                                                        */
    {"CCD",                                                  /* Datum Code */
     "S-JTSK, Czech Republic & Slovakia",                    /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {589, 4, 76, 2, 480, 3}                                 /* Parameters */
    },


 /* SE_DAL_HDATUM,       Dabola - applicable to Guinea                    */
 /*                                                                        */
    {"DAL",                                                  /* Datum Code */
     "DABOLA, Guinea",                                       /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-83, 15, 37, 15, 124, 15}                              /* Parameters */
    },


 /* SE_DID_HDATUM,       Deception Island - applicable to Deception       */
 /*                                              Island, Antartica         */
    {"DID",                                                  /* Datum Code */
     "DECEPTION ISLAND",                                     /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {260, 20, 12, 20, -147, 20}                             /* Parameters */
    },


 /* SE_EST_HDATUM,       Coordinate System 1937 of Estonia - applicable   */
 /*                                                      to Estonia        */
    {"EST",                                                  /* Datum Code */
     "COORDINATE SYSTEM 1937 OF ESTONIA",                    /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {374, 2, 150, 3, 588, 3}                                /* Parameters */
    },


 /* SE_EUR_S_HDATUM,     European 1950 - applicable to Iraq, Israel,      */
 /*                        Jordan, Lebanon, Kuwait, Saudi Arabia & Syria   */
    {"EUR-S",                                                /* Datum Code */
     "EUROPEAN 1950, SW Asia",                               /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-103, -1, -106, -1, -141, -1}                          /* Parameters */
    },


 /* SE_EUR_T_HDATUM,     European 1950 - applicable to Tunisia            */
 /*                                                                        */
    {"EUR-T",                                                /* Datum Code */
     "EUROPEAN 1950, Tunisia",                               /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-112, 25, -77, 25, -145, 25}                           /* Parameters */
    },


 /* SE_GSE_HDATUM,       Gunung Segara - applicable to Kalimantan         */
 /*                                      Island (Indonesia)                */
    {"GSE",                                                  /* Datum Code */
     "GUNANG SEGARA, Kalimantan",                            /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {-403, -1, 684, -1, 41, -1}                             /* Parameters */
    },


 /* SE_HEN_HDATUM,       Herat North - applicable to Afghanistan          */
 /*                                                                        */
    {"HEN",                                                  /* Datum Code */
     "HERAT NORTH, Afghanistan",                             /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-333, -1, -222, -1, 114, -1}                           /* Parameters */
    },


 /* SE_HER_HDATUM,       Hermannskogel - applicable to Yugoslavia (prior  */
 /*                               to 1990), Slovenia, Croatia, Bosnia      */
 /*                              and Herzegovina & Serbia                  */
    {"HER",                                                  /* Datum Code */
     "HERMANNSKOGEL, Yugoslovia",                            /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {682, -1, -203, -1, 480, -1}                            /* Parameters */
    },


 /* SE_IDN_HDATUM,       Indonesian 1974 - applicable to Indonesia        */
 /*                                                                        */
    {"IDN",                                                  /* Datum Code */
     "INDONESIAN 1974, Indonesia",                           /* Datum Name */
     SE_ID_ELLIPSOID,                                    /* Ellipsoid Code */
     6378160,                                                         /* A */
     298.247,                                        /* Inverse Flattening */
     {-24, 25, -15, 25, 5, 25}                               /* Parameters */
    },


 /* SE_IND_P_HDATUM,     Indian - applicable to Pakistan                  */
 /*                                                                        */
    {"IND-P",                                                /* Datum Code */
     "INDIAN, Pakistan",                                     /* Datum Name */
     SE_EF_ELLIPSOID,                                    /* Ellipsoid Code */
     6377309.613,                                                     /* A */
     300.8017,                                       /* Inverse Flattening */
     {283, -1, 682, -1, 231, -1}                             /* Parameters */
    },


 /* SE_ING_A_HDATUM,     Indian 1960 - applicable to Vietnam              */
 /*                                      (near 16 degrees N)               */
    {"ING-A",                                                /* Datum Code */
     "INDIAN 1960, Vietnam near 16 deg N",                   /* Datum Name */
     SE_EA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377276.345,                                                     /* A */
     300.8017,                                       /* Inverse Flattening */
     {198, 25, 881, 25, 317, 25}                             /* Parameters */
    },


 /* SE_ING_B_HDATUM,     Indian 1960 - applicable to Con Son Island       */
 /*                                                     (Vietnam)          */
    {"ING-B",                                                /* Datum Code */
     "INDIAN 1960, Con Son Island",                          /* Datum Name */
     SE_EA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377276.345,                                                     /* A */
     300.8017,                                       /* Inverse Flattening */
     {182, 25, 915, 25, 344, 25}                             /* Parameters */
    },


 /* SE_NAR_E_HDATUM,     North American 1983 - applicable to Aleutian     */
 /*                                                      Islands           */
    {"NAR-E",                                                /* Datum Code */
     "NORTH AMERICAN 1983, Aleutian Islands",                /* Datum Name */
     SE_RF_ELLIPSOID,                                    /* Ellipsoid Code */
     6378137,                                                         /* A */
     298.257222101,                                  /* Inverse Flattening */
     {-2, 5, 0, 2, 4, 5}                                     /* Parameters */
    },


 /* SE_NAR_H_HDATUM,     North American 1983 - applicable to Hawaii       */
 /*                                                                        */
    {"NAR-H",                                                /* Datum Code */
     "NORTH AMERICAN 1983, Hawaii",                          /* Datum Name */
     SE_RF_ELLIPSOID,                                    /* Ellipsoid Code */
     6378137,                                                         /* A */
     298.257222101,                                  /* Inverse Flattening */
     {1, 2, 1, 2, -1, 2}                                     /* Parameters */
    },


 /* SE_NAS_V_HDATUM,     North American 1927 - applicable to Aleutian     */
 /*                                      Islands (East of 180 degrees W)   */
    {"NAS-V",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Aleutian Islands E of 180 deg",   /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {-2, 6, 152, 8, 149, 10}                                /* Parameters */
    },


 /* SE_NAS_W_HDATUM,     North American 1927 - applicable to Aleutian     */
 /*                                      Islands (West of 180 degrees W)   */
    {"NAS-W",                                                /* Datum Code */
     "NORTH AMERICAN 1927, Aleutian Islands W of 180 deg",   /* Datum Name */
     SE_CC_ELLIPSOID,                                    /* Ellipsoid Code */
     6378206.4,                                                       /* A */
     294.9786982,                                    /* Inverse Flattening */
     {2, 10, 204, 10, 105, 10}                               /* Parameters */
    },


 /* SE_NSD_HDATUM,       North Sahara 1959 - applicable to Algeria        */
 /*                                                                        */
    {"NSD",                                                  /* Datum Code */
     "NORTH SAHARA 1959, Algeria",                           /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-186, 25, -93, 25, 310, 25}                            /* Parameters */
    },


 /* SE_PUK_HDATUM,       Pulkovo 1942 - applicable to Russia              */
 /*                                                                        */
    {"PUK",                                                  /* Datum Code */
     "PULKOVO 1942, Russia",                                 /* Datum Name */
     SE_KA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378245,                                                         /* A */
     298.3,                                          /* Inverse Flattening */
     {28, -1, -130, -1, -95, -1}                             /* Parameters */
    },


 /* SE_SPK_A_HDATUM,     S-42 (Pulkovo 1942) - applicable to Hungary      */
 /*                                                                        */
    {"SPK-A",                                                /* Datum Code */
     "S-42 (PULKOVO 1942), Hungary",                         /* Datum Name */
     SE_KA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378245,                                                         /* A */
     298.3,                                          /* Inverse Flattening */
     {28, 2, -121, 2, -77, 2}                                /* Parameters */
    },


 /* SE_SPK_B_HDATUM,     S-42 (Pulkovo 1942) - applicable to Poland       */
 /*                                                                        */
    {"SPK-B",                                                /* Datum Code */
     "S-42 (PULKOVO 1942), Poland",                          /* Datum Name */
     SE_KA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378245,                                                         /* A */
     298.3,                                          /* Inverse Flattening */
     {23, 4, -124, 2, -82, 4}                                /* Parameters */
    },


 /* SE_SPK_C_HDATUM,     S-42 (Pulkovo 1942) - applicable to Czech        */
 /*                                              Republic & Slovakia       */
    {"SPK-C",                                                /* Datum Code */
     "S-42 (PULKOVO 1942), Czech Republic & Slovakia",       /* Datum Name */
     SE_KA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378245,                                                         /* A */
     298.3,                                          /* Inverse Flattening */
     {26, 3, -121, 3, -78, 2}                                /* Parameters */
    },


 /* SE_SPK_D_HDATUM,     S-42 (Pulkovo 1942) - applicable to Latvia       */
 /*                                                                        */
    {"SPK-D",                                                /* Datum Code */
     "S-42 (PULKOVO 1942), Latvia",                          /* Datum Name */
     SE_KA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378245,                                                         /* A */
     298.3,                                          /* Inverse Flattening */
     {24, 2, -124, 2, -82, 2}                                /* Parameters */
    },


 /* SE_SPK_E_HDATUM,     S-42 (Pulkovo 1942) - applicable to Kazakhstan   */
 /*                                                                        */
    {"SPK-E",                                                /* Datum Code */
     "S-42 (PULKOVO 1942), Kazakhstan",                      /* Datum Name */
     SE_KA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378245,                                                         /* A */
     298.3,                                          /* Inverse Flattening */
     {15, 25, -130, 25, -84, 25}                             /* Parameters */
    },


 /* SE_SPK_F_HDATUM,     S-42 (Pulkovo 1942) - applicable to Albania      */
 /*                                                                        */
    {"SPK-F",                                                /* Datum Code */
     "S-42 (PULKOVO 1942), Albania",                         /* Datum Name */
     SE_KA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378245,                                                         /* A */
     298.3,                                          /* Inverse Flattening */
     {24, 3, -130, 3, -92, 3}                                /* Parameters */
    },


 /* SE_SPK_G_HDATUM,     S-42 (Pulkovo 1942) - applicable to Romania      */
 /*                                                                        */
    {"SPK-G",                                                /* Datum Code */
     "S-42 (PULKOVO 1942), Romania",                         /* Datum Name */
     SE_KA_ELLIPSOID,                                    /* Ellipsoid Code */
     6378245,                                                         /* A */
     298.3,                                          /* Inverse Flattening */
     {28, 3, -121, 5, -77, 3}                                /* Parameters */
    },


 /* SE_SRL_HDATUM,       Sierra Leone 1960 - applicable to Sierra Leone   */
 /*                                                                        */
    {"SRL",                                                  /* Datum Code */
     "SIERRA LEONE 1960",                                    /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-88, 15, 4, 15, 101, 15}                               /* Parameters */
    },


 /* SE_TAN_HDATUM,       Tananarive Observatory 1925 - applicable to      */
 /*                                                      Madagascar        */
    {"TAN",                                                  /* Datum Code */
     "TANANARIVE OBSERVATORY 1925, Madagascar",              /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-189, -1, -242, -1, -91, -1}                           /* Parameters */
    },


 /* SE_VOI_HDATUM,       Voirol 1874 - applicable to Tunisia/Algeria      */
 /*                                                                        */
    {"VOI",                                                  /* Datum Code */
     "VOIROL 1874, Tunisia/Algeria",                         /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-73, -1, -247, -1, 227, -1}                            /* Parameters */
    },


 /* SE_VOR_HDATUM,       Voirol 1960 - applicable to Algeria              */
 /*                                                                        */
    {"VOI",                                                  /* Datum Code */
     "VOIROL 1960, Algeria",                                 /* Datum Name */
     SE_CD_ELLIPSOID,                                    /* Ellipsoid Code */
     6378249.145,                                                     /* A */
     293.465,                                        /* Inverse Flattening */
     {-123, 25, -206, 25, 219, 25}                           /* Parameters */
    },


 /* SE_YAC_HDATUM,       Yacare - applicable to Uruguay                   */
 /*                                                                        */
    {"YAC",                                                  /* Datum Code */
     "YACARE, Uruguay",                                      /* Datum Name */
     SE_IN_ELLIPSOID,                                    /* Ellipsoid Code */
     6378388,                                                         /* A */
     297,                                            /* Inverse Flattening */
     {-155, -1, 171, -1, 37, -1}                             /* Parameters */
    },


 /* SE_INH_A1_HDATUM,        Indian 1975 - applicable to Thailand          */
 /*                                                                        */
    {"INH-A1",                                                /* Datum Code */
     "INDIAN 1975, Thailand",                                /* Datum Name */
     SE_EA_ELLIPSOID,                                    /* Ellipsoid Code */
     6377276.345,                                                     /* A */
     300.8017,                                       /* Inverse Flattening */
     {210, 3, 814, 2, 289, 3}                                /* Parameters */
    },


 /* SE_TOY_B1_HDATUM,        Tokyo - applicable to South Korea            */
 /*                                                                        */
    {"TOY-B1",                                               /* Datum Code */
     "TOKYO, South Korea",                                   /* Datum Name */
     SE_BR_ELLIPSOID,                                    /* Ellipsoid Code */
     6377397.155,                                                     /* A */
     299.1528128,                                    /* Inverse Flattening */
     {-147, 2, 506, 2, 687, 2}                               /* Parameters */
    }

};



/*
 *-----------------------------------------------------------------------------
 *
 * FUNCTION: SE_PrintEllipsoid
 *
 *-----------------------------------------------------------------------------
 */
const char *
SE_PrintEllipsoid
(
    SE_ELLIPSOID_ENUM value
)
{
    switch (value)
    {
        case SE_AA_ELLIPSOID:
             return "Airy 1830";
        case SE_AM_ELLIPSOID:
             return "Modified Airy";
        case SE_AN_ELLIPSOID:
             return "Australian National";
        case SE_BN_ELLIPSOID:
             return "Bessel 1841 (Namibia)";
        case SE_BR_ELLIPSOID:
             return "Bessel 1841 (Ethiopia Indonesia Japan Korea)";
        case SE_CC_ELLIPSOID:
             return "Clarke 1866";
        case SE_CD_ELLIPSOID:
             return "Clarke 1880";
        case SE_EA_ELLIPSOID:
             return "Everest (India 1830)";
        case SE_EB_ELLIPSOID:
             return "Everest (Brunei and E. Malaysia  - Sabah & Sarawak)";
        case SE_EC_ELLIPSOID:
             return "Everest (India 1956)";
        case SE_ED_ELLIPSOID:
             return "Everest (West Malaysia 1969)";
        case SE_EE_ELLIPSOID:
             return "Everest (West Malaysia & Singapore 1948)";
        case SE_EF_ELLIPSOID:
             return "Everest (Pakistan)";
        case SE_FA_ELLIPSOID:
             return "Modified Fischer 1960";
        case SE_HE_ELLIPSOID:
             return "Helmert 1906";
        case SE_HO_ELLIPSOID:
             return "Hough 1960";
        case SE_ID_ELLIPSOID:
             return "Indonesian 1974";
        case SE_IN_ELLIPSOID:
             return "International 1924";
        case SE_KA_ELLIPSOID:
             return "Krassovsky 1940";
        case SE_RF_ELLIPSOID:
             return "Geodetic Reference System 1980 (GRS 80)";
        case SE_SA_ELLIPSOID:
             return "South American 1969";
        case SE_WD_ELLIPSOID:
             return "WGS 72";
        case SE_WE_ELLIPSOID:
             return "WGS 84";
        case SE_ACM_SPHEROID:
             return "Spherical";
        case SE_COA_SPHEROID:
             return "Spherical";
        case SE_NOG_SPHEROID:
             return "Spherical";
        case SE_MFE_SPHEROID:
             return "MULTIGEN FLAT EARTH, Spherical";
        case SE_MOD_M_SPHEROID:
             return "MODTRAN Midlatitude, Spherical";
        case SE_MOD_S_SPHEROID:
             return "MODTRAN Subarctic, Spherical";
        case SE_MOD_T_SPHEROID:
             return "MODTRAN Tropical, Spherical";
        case SE_MM5__SPHEROID:
             return "MM5 AFWA, Spherical";


        case SE_UNSPECIFIED_ELLIPSOID:
        default:
             return "WHOA!!!!! INVALID/UNSPECIFIED ELLIPSOID";
    }
} /* end SE_PrintEllipsoid */
