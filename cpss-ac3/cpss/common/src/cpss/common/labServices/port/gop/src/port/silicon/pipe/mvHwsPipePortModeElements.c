/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsPipePortModeElements.c
*
* @brief
*
* @version   1
********************************************************************************
*/

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>


/************************* definition *****************************************************/


/************************* Globals ********************************************************/
/* Last line of each SupMode array should be NON_SUP_MODE */
MV_HWS_PORT_INIT_PARAMS hwsPipePort0SupModes[] = {
    /* port mode,      MAC Type    Mac Num  PCS Type   PCS Num  FEC       Speed     FirstLane  LanesList LanesNum  Media          10Bit      FecSupported */
    { SGMII,          GEMAC_SG,    0,       GPCS,          0,   FEC_OFF,  _1_25G,        0,    {0,0,0,0},   1,     XAUI_MEDIA,   _10BIT_ON,  FEC_OFF              },
    { _1000Base_X,    GEMAC_X,     0,       GPCS,          0,   FEC_OFF,  _1_25G,        0,    {0,0,0,0},   1,     XAUI_MEDIA,   _10BIT_ON,  FEC_OFF              },
    { SGMII2_5,       GEMAC_X,     0,       GPCS,          0,   FEC_OFF,  _3_125G,       0,    {0,0,0,0},   1,     XAUI_MEDIA,   _10BIT_ON,  FEC_OFF              },
    { QSGMII,         QSGMII_MAC,  0,       QSGMII_PCS,    0,   FEC_OFF,  _5G,           0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF              },
    { RXAUI,          XGMAC,       0,       XPCS,          0,   FEC_OFF,  _6_25G,        0,    {0,0,0,0},   2,     RXAUI_MEDIA,  _20BIT_ON,  FEC_OFF              },
    { _2_5GBase_QX,   XGMAC,       0,       XPCS,          0,   FEC_OFF,  _3_125G,       0,    {0,0,0,0},   1,     XAUI_MEDIA,   _10BIT_OFF, FEC_OFF              },
    { _5GBase_DQX,    XGMAC,       0,       XPCS,          0,   FEC_OFF,  _6_25G,        0,    {0,0,0,0},   1,     XAUI_MEDIA,   _10BIT_OFF, FEC_OFF              },
    { _5GBase_HX,     XGMAC,       0,       XPCS,          0,   FEC_OFF,  _3_125G,       0,    {0,0,0,0},   2,     XAUI_MEDIA,   _10BIT_OFF, FEC_OFF              },
    { _10GBase_KX2,   XGMAC,       0,       XPCS,          0,   FEC_OFF,  _6_25G,        0,    {0,0,0,0},   2,     XAUI_MEDIA,   _10BIT_OFF, FEC_OFF              },/*DHX*/
    { _10GBase_KX4,   XGMAC,       0,       XPCS,          0,   FEC_OFF,  _3_125G,       0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF              },/*XAUI*/
    { _20GBase_KX4,   XGMAC,       0,       XPCS,          0,   FEC_OFF,  _6_25G,        0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF              },/*DR-XAUI*/
    { _10GBase_KR,    XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _10_3125G,     0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _20GBase_KR,    XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _10_3125G,     0,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _40GBase_KR,    XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _10_3125G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _10GBase_SR_LR, XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _10_3125G,     0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _20GBase_SR_LR, XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _10_3125G,     0,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _40GBase_SR_LR, XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _10_3125G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { XLHGL_KR4,      XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _12_5G,        0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _12GBaseR,      XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _12_1875G,     0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _48GBaseR,      XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _12_1875G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _5GBaseR,       XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _5_15625G,     0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF              },
    { _24GBase_KR2,   XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _12_1875G,     0,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _12_5GBase_KR,  XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _12_8906G,     0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _25GBase_KR2,   XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _12_8906G,     0,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _50GBase_KR4,   XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _12_8906G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _40GBase_CR4,   XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _10_3125G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _40GBase_KR4,   XLGMAC,      0,       MMPCS,         0,   FEC_OFF,  _10_3125G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { NON_SUP_MODE,   MAC_NA,    NA_NUM,    PCS_NA,     NA_NUM, FEC_OFF,  SPEED_NA,  NA_NUM,   {0,0,0,0},   0,     XAUI_MEDIA,   _10BIT_ON,  FEC_NA}
};
static MV_HWS_PORT_INIT_PARAMS hwsPipePort12SupModes[] = {
          /* port mode,   MAC Type    Mac Num    PCS Type    PCS Num    FEC     Speed      FirstLane LanesList LanesNum   Media         10Bit */
    { SGMII,          GEMAC_SG,  12,        GPCS,        12,    FEC_OFF,  _1_25G,       12,    {0,0,0,0},   1,     XAUI_MEDIA,   _10BIT_ON,  FEC_OFF              },
    { _1000Base_X,    GEMAC_X,   12,        GPCS,        12,    FEC_OFF,  _1_25G,       12,    {0,0,0,0},   1,     XAUI_MEDIA,   _10BIT_ON,  FEC_OFF              },
    { SGMII2_5,       GEMAC_X,   12,        GPCS,        12,    FEC_OFF,  _3_125G,      12,    {0,0,0,0},   1,     XAUI_MEDIA,   _10BIT_ON,  FEC_OFF              },
    { _10GBase_KR,    XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _10_3125G,    12,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _20GBase_KR,    XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _10_3125G,    12,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _40GBase_KR,    XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _10_3125G,    12,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _10GBase_SR_LR, XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _10_3125G,    12,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _20GBase_SR_LR, XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _10_3125G,    12,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _40GBase_SR_LR, XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _10_3125G,    12,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { XLHGL_KR4,      XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _12_5G,       12,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _12GBaseR,      XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _12_1875G,    12,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _48GBaseR,      XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _12_1875G,    12,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _5GBaseR,       XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _5_15625G,    12,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF              },
    { _24GBase_KR2,   XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _12_1875G,    12,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _12_5GBase_KR,  XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _12_8906G,    12,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _25GBase_KR2,   XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _12_8906G,    12,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _50GBase_KR4,   XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _12_8906G,    12,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _25GBase_KR,    XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   1,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2,   XLGMAC,    12,        CGPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   2,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _100GBase_KR4,  CGMAC,     12,        CGPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   4,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|RS_FEC       },
    { _25GBase_SR,    XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   1,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,   XLGMAC,    12,        CGPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   2,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _100GBase_SR4,  CGMAC,     12,        CGPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   4,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|RS_FEC       },
    { _100GBase_MLG,  XLGMAC,    12,        CGPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   4,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF              },
    { _107GBase_KR4,  CGMAC,     12,        CGPCS,       12,    RS_FEC,   _27_5G,       12,    {0,0,0,0},   4,     XAUI_MEDIA,   _40BIT_ON,  RS_FEC               },
    { _25GBase_KR_C,  XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   1,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C, XLGMAC,    12,        CGPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   2,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_KR2,   XLGMAC,    12,        CGPCS,       12,    FEC_OFF,  _20_625G,     12,    {0,0,0,0},   2,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_CR4,   XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _10_3125G,    12,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _25GBase_CR,    XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   1,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,   XLGMAC,    12,        CGPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   2,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _100GBase_CR4,  CGMAC,     12,        CGPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   4,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|RS_FEC       },
    { _25GBase_KR_S,  XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   1,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC       },
    { _25GBase_CR_S,  XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   1,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC       },
    { _25GBase_CR_C,  XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   1,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2_C, XLGMAC,    12,        CGPCS,       12,    FEC_OFF,  _25_78125G,   12,    {0,0,0,0},   2,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _52_5GBase_KR2, XLGMAC,    12,        CGPCS,       12,    FEC_OFF,  _27_1G,       12,    {0,0,0,0},   2,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},
    { _102GBase_KR4,  CGMAC,     12,        CGPCS,       12,    RS_FEC,   _26_25G,      12,    {0,0,0,0},   4,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|RS_FEC       },
    { _40GBase_KR4,   XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _10_3125G,    12,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC       },
    { _26_7GBase_KR,  XLGMAC,    12,        MMPCS,       12,    FEC_OFF,  _27_5G,       12,    {0,0,0,0},   1,     XAUI_MEDIA,   _40BIT_ON,  FEC_OFF|FC_FEC|RS_FEC},

    { NON_SUP_MODE,   MAC_NA,    NA_NUM,    PCS_NA,     NA_NUM, FEC_OFF,  SPEED_NA,  NA_NUM,   {0,0,0,0},   0,     XAUI_MEDIA,   _10BIT_ON,  FEC_NA}
};

/* map the right supported modes table for each port */
MV_HWS_PORT_INIT_PARAMS *hwsPortsPipeParamsSupModesMap[] =
{
    /*  0 */ hwsPipePort0SupModes, /*  1 */ hwsPipePort0SupModes, /*  2 */ hwsPipePort0SupModes, /*  3 */ hwsPipePort0SupModes,
    /*  4 */ hwsPipePort0SupModes, /*  5 */ hwsPipePort0SupModes, /*  6 */ hwsPipePort0SupModes, /*  7 */ hwsPipePort0SupModes,
    /*  8 */ hwsPipePort0SupModes, /*  9 */ hwsPipePort0SupModes, /* 10 */ hwsPipePort0SupModes, /* 11 */ hwsPipePort0SupModes,

    /* 12 */ hwsPipePort12SupModes, /* 13 */ hwsPipePort12SupModes, /* 14 */ hwsPipePort12SupModes, /* 15 */ hwsPipePort12SupModes,
};

/************************* functions ******************************************************/

