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
* @file mvHwsBobcat3PortModeElements.c
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
/* Last line of each SupModes array MUST be NON_SUP_MODE */
MV_HWS_PORT_INIT_PARAMS hwsBobcat3Port0SupModes[] = {
  /* port mode,       MAC Type     Mac Num   PCS Type   PCS Num  FEC        Speed      FirstLane LanesList  LanesNum  Media         10Bit      FecSupported*/
    { SGMII,          GEMAC_SG,    0,        GPCS,          0,   FEC_OFF,   _1_25G,        0,    {0,0,0,0},    1,     XAUI_MEDIA,   _10BIT_ON, FEC_OFF                  },
    { _1000Base_X,    GEMAC_X,     0,        GPCS,          0,   FEC_OFF,   _1_25G,        0,    {0,0,0,0},    1,     XAUI_MEDIA,   _10BIT_ON, FEC_OFF                  },
    { SGMII2_5,       GEMAC_X,     0,        GPCS,          0,   FEC_OFF,   _3_125G,       0,    {0,0,0,0},    1,     XAUI_MEDIA,   _10BIT_ON, FEC_OFF                  },
    { _10GBase_KR,    XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},    1,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _20GBase_KR,    XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},    2,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _40GBase_KR,    XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _10GBase_SR_LR, XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},    1,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _20GBase_SR_LR, XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},    2,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _40GBase_SR_LR, XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { XLHGL_KR4,      XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_5G,        0,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _12GBaseR,      XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_1875G,     0,    {0,0,0,0},    1,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _48GBaseR,      XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_1875G,     0,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _5GBaseR,       XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _5_15625G,     0,    {0,0,0,0},    1,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF                  },
    { _24GBase_KR2,   XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_1875G,     0,    {0,0,0,0},    2,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _12_5GBase_KR,  XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_8906G,     0,    {0,0,0,0},    1,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _25GBase_KR2,   XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_8906G,     0,    {0,0,0,0},    2,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _50GBase_KR4,   XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_8906G,     0,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _25GBase_KR,    XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    1,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC    },
    { _50GBase_KR2,   XLGMAC,      0,        CGPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    2,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC           },
    { _100GBase_KR4,  CGMAC,       0,        CGPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    4,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC           },
    { _25GBase_SR,    XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    1,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC    },
    { _50GBase_SR2,   XLGMAC,      0,        CGPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    2,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC           },
    { _100GBase_SR4,  CGMAC,       0,        CGPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    4,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC           },
    { _100GBase_MLG,  XLGMAC,      0,        CGPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    4,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF                  },
    { _107GBase_KR4,  CGMAC,       0,        CGPCS,         0,   RS_FEC,    _27_5G,        0,    {0,0,0,0},    4,     XAUI_MEDIA,   _40BIT_ON, RS_FEC                   },
    { _25GBase_KR_C,  XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    1,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC    },
    { _50GBase_KR2_C, XLGMAC,      0,        CGPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    2,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC           },
    { _40GBase_KR2,   XLGMAC,      0,        CGPCS,         0,   FEC_OFF,   _20_625G,      0,    {0,0,0,0},    2,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC    },
    { _29_09GBase_SR, XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _7_5G,         0,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF                  },
    { _40GBase_CR4,   XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _25GBase_CR,    XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    1,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC    },
    { _50GBase_CR2,   XLGMAC,      0,        CGPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    2,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC           },
    { _100GBase_CR4,  CGMAC,       0,        CGPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    4,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC           },
    { _25GBase_KR_S,  XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    1,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC           },
    { _25GBase_CR_S,  XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    1,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC           },
    { _25GBase_CR_C,  XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    1,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC    },
    { _50GBase_CR2_C, XLGMAC,      0,        CGPCS,         0,   FEC_OFF,   _25_78125G,    0,    {0,0,0,0},    2,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC           },
    { _102GBase_KR4,  CGMAC,       0,        CGPCS,         0,   RS_FEC,    _26_25G,       0,    {0,0,0,0},    4,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC           },
    { _40GBase_KR4,   XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _26_7GBase_KR,  XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _27_5G,        0,    {0,0,0,0},    1,     XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC    },

    { NON_SUP_MODE,   MAC_NA,     NA_NUM,    PCS_NA,   NA_NUM,   FEC_OFF,   SPEED_NA,  NA_NUM,   {0,0,0,0},    0,     XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

MV_HWS_PORT_INIT_PARAMS hwsBobcat3Port72SupModes[] = {
    /* port mode,     MAC Type   Mac Num     PCS Type    PCS Num FEC        Speed      FirstLane LanesList  LanesNum  Media         10Bit      FecSupported*/
    { SGMII,          GEMAC_SG,  72,         GPCS,       72,     FEC_OFF,   _1_25G,        72,   {0,0,0,0},     1,    XAUI_MEDIA,   _10BIT_ON, FEC_OFF                  },
    { _1000Base_X,    GEMAC_X,   72,         GPCS,       72,     FEC_OFF,   _1_25G,        72,   {0,0,0,0},     1,    XAUI_MEDIA,   _10BIT_ON, FEC_OFF                  },
    { SGMII2_5,       GEMAC_X,   72,         GPCS,       72,     FEC_OFF,   _3_125G,       72,   {0,0,0,0},     1,    XAUI_MEDIA,   _10BIT_ON, FEC_OFF                  },
    { _10GBase_KR,    XLGMAC,    72,         MMPCS,      72,     FEC_OFF,   _10_3125G,     72,   {0,0,0,0},     1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _10GBase_SR_LR, XLGMAC,    72,         MMPCS,      72,     FEC_OFF,   _10_3125G,     72,   {0,0,0,0},     1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _12GBaseR,      XLGMAC,    72,         MMPCS,      72,     FEC_OFF,   _12_1875G,     72,   {0,0,0,0},     1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },
    { _5GBaseR,       XLGMAC,    72,         MMPCS,      72,     FEC_OFF,   _5_15625G,     72,   {0,0,0,0},     1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF                  },
    { _12_5GBase_KR,  XLGMAC,    72,         MMPCS,      72,     FEC_OFF,   _12_8906G,     72,   {0,0,0,0},     1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC           },

    { NON_SUP_MODE,   MAC_NA,     NA_NUM,    PCS_NA,   NA_NUM,   FEC_OFF,   SPEED_NA,  NA_NUM,   {0,0,0,0},     0,     XAUI_MEDIA,  _10BIT_ON, FEC_NA}
};

/* map the right supported modes table for each port */
MV_HWS_PORT_INIT_PARAMS *hwsPortsBobcat3ParamsSupModesMap[] =
{
    /*  0 */ hwsBobcat3Port0SupModes, /*  1 */ hwsBobcat3Port0SupModes, /*  2 */ hwsBobcat3Port0SupModes, /*  3 */ hwsBobcat3Port0SupModes,
    /*  4 */ hwsBobcat3Port0SupModes, /*  5 */ hwsBobcat3Port0SupModes, /*  6 */ hwsBobcat3Port0SupModes, /*  7 */ hwsBobcat3Port0SupModes,
    /*  8 */ hwsBobcat3Port0SupModes, /*  9 */ hwsBobcat3Port0SupModes, /* 10 */ hwsBobcat3Port0SupModes, /* 11 */ hwsBobcat3Port0SupModes,
    /* 12 */ hwsBobcat3Port0SupModes, /* 13 */ hwsBobcat3Port0SupModes, /* 14 */ hwsBobcat3Port0SupModes, /* 15 */ hwsBobcat3Port0SupModes,
    /* 16 */ hwsBobcat3Port0SupModes, /* 17 */ hwsBobcat3Port0SupModes, /* 18 */ hwsBobcat3Port0SupModes, /* 19 */ hwsBobcat3Port0SupModes,
    /* 20 */ hwsBobcat3Port0SupModes, /* 21 */ hwsBobcat3Port0SupModes, /* 22 */ hwsBobcat3Port0SupModes, /* 23 */ hwsBobcat3Port0SupModes,
    /* 24 */ hwsBobcat3Port0SupModes, /* 25 */ hwsBobcat3Port0SupModes, /* 26 */ hwsBobcat3Port0SupModes, /* 27 */ hwsBobcat3Port0SupModes,
    /* 28 */ hwsBobcat3Port0SupModes, /* 29 */ hwsBobcat3Port0SupModes, /* 30 */ hwsBobcat3Port0SupModes, /* 31 */ hwsBobcat3Port0SupModes,
    /* 32 */ hwsBobcat3Port0SupModes, /* 33 */ hwsBobcat3Port0SupModes, /* 34 */ hwsBobcat3Port0SupModes, /* 35 */ hwsBobcat3Port0SupModes,
    /* 36 */ hwsBobcat3Port0SupModes, /* 37 */ hwsBobcat3Port0SupModes, /* 38 */ hwsBobcat3Port0SupModes, /* 39 */ hwsBobcat3Port0SupModes,
    /* 40 */ hwsBobcat3Port0SupModes, /* 41 */ hwsBobcat3Port0SupModes, /* 42 */ hwsBobcat3Port0SupModes, /* 43 */ hwsBobcat3Port0SupModes,
    /* 44 */ hwsBobcat3Port0SupModes, /* 45 */ hwsBobcat3Port0SupModes, /* 46 */ hwsBobcat3Port0SupModes, /* 47 */ hwsBobcat3Port0SupModes,
    /* 48 */ hwsBobcat3Port0SupModes, /* 49 */ hwsBobcat3Port0SupModes, /* 50 */ hwsBobcat3Port0SupModes, /* 51 */ hwsBobcat3Port0SupModes,
    /* 52 */ hwsBobcat3Port0SupModes, /* 53 */ hwsBobcat3Port0SupModes, /* 54 */ hwsBobcat3Port0SupModes, /* 55 */ hwsBobcat3Port0SupModes,
    /* 56 */ hwsBobcat3Port0SupModes, /* 57 */ hwsBobcat3Port0SupModes, /* 58 */ hwsBobcat3Port0SupModes, /* 59 */ hwsBobcat3Port0SupModes,
    /* 60 */ hwsBobcat3Port0SupModes, /* 61 */ hwsBobcat3Port0SupModes, /* 62 */ hwsBobcat3Port0SupModes, /* 63 */ hwsBobcat3Port0SupModes,
    /* 64 */ hwsBobcat3Port0SupModes, /* 65 */ hwsBobcat3Port0SupModes, /* 66 */ hwsBobcat3Port0SupModes, /* 67 */ hwsBobcat3Port0SupModes,
    /* 68 */ hwsBobcat3Port0SupModes, /* 69 */ hwsBobcat3Port0SupModes, /* 70 */ hwsBobcat3Port0SupModes, /* 71 */ hwsBobcat3Port0SupModes,
    /* 72 */ hwsBobcat3Port72SupModes,/* 73 */ hwsBobcat3Port72SupModes
};

/************************* functions ******************************************************/

