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
* @file mvHwsAldrinPortModeElements.c
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
MV_HWS_PORT_INIT_PARAMS hwsAldrinPort0SupModes[] = {

    /* port mode,     MAC Type     Mac Num   PCS Type      PCS Num   FEC     Speed     FirstLane LanesList  LanesNum   Media        10Bit      FecSupported */
    { SGMII,          GEMAC_SG,    0,        GPCS,          0,   FEC_OFF,   _1_25G,        0,    {0,0,0,0},   1,     XAUI_MEDIA,   _10BIT_ON,  FEC_OFF          },
    { _1000Base_X,    GEMAC_X,     0,        GPCS,          0,   FEC_OFF,   _1_25G,        0,    {0,0,0,0},   1,     XAUI_MEDIA,   _10BIT_ON,  FEC_OFF          },
    { SGMII2_5,       GEMAC_X,     0,        GPCS,          0,   FEC_OFF,   _3_125G,       0,    {0,0,0,0},   1,     XAUI_MEDIA,   _10BIT_ON,  FEC_OFF          },
    { QSGMII,         QSGMII_MAC,  0,        QSGMII_PCS,    0,   FEC_OFF,   _5G,           0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF          },
    { _10GBase_KX4,   XGMAC,       0,        XPCS,          0,   FEC_OFF,   _3_125G,       0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF          },
    { _10GBase_KX2,   XGMAC,       0,        XPCS,          0,   FEC_OFF,   _6_25G,        0,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF          },
    { _10GBase_KR,    XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _20GBase_KR,    XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _40GBase_KR,    XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { RXAUI,          XGMAC,       0,        XPCS,          0,   FEC_OFF,   _6_25G,        0,    {0,0,0,0},   2,     RXAUI_MEDIA,  _20BIT_ON,  FEC_OFF          },
    { _20GBase_KX4,   XGMAC,       0,        XPCS,          0,   FEC_OFF,   _6_25G,        0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF          },
    { _10GBase_SR_LR, XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _20GBase_SR_LR, XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _40GBase_SR_LR, XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { XLHGL_KR4,      XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_5G,        0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _12GBaseR,      XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_1875G,     0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _48GBaseR,      XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_1875G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _5GBaseR,       XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _5_15625G,     0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF          },
    { _24GBase_KR2,   XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_1875G,     0,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _12_5GBase_KR,  XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_8906G,     0,    {0,0,0,0},   1,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _25GBase_KR2,   XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_8906G,     0,    {0,0,0,0},   2,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _50GBase_KR4,   XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _12_8906G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _29_09GBase_SR, XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _7_5G,         0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF          },
    { _40GBase_CR4,   XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { _40GBase_KR4,   XLGMAC,      0,        MMPCS,         0,   FEC_OFF,   _10_3125G,     0,    {0,0,0,0},   4,     XAUI_MEDIA,   _20BIT_ON,  FEC_OFF|FC_FEC   },
    { NON_SUP_MODE,   MAC_NA,     NA_NUM,    PCS_NA,   NA_NUM,   FEC_OFF,   SPEED_NA,  NA_NUM,   {0,0,0,0},   0,     XAUI_MEDIA,   _10BIT_ON,  FEC_NA}
};

MV_HWS_PORT_INIT_PARAMS hwsAldrinPort32SupModes[] = {
    /* port mode,     MAC Type    Mac Num   PCS Type      PCS Num   FEC     Speed      FirstLane LanesList  LanesNum   Media       10Bit      FecSupported */
    { SGMII,          GEMAC_NET_SG,  32,   GPCS_NET,         32, FEC_OFF,    _1_25G,      32,    {0,0,0,0},    1,    XAUI_MEDIA,   _10BIT_ON,  FEC_OFF},
    { _1000Base_X,    GEMAC_NET_X,   32,   GPCS_NET,         32, FEC_OFF,    _1_25G,      32,    {0,0,0,0},    1,    XAUI_MEDIA,   _10BIT_ON,  FEC_OFF},
    { SGMII2_5,       GEMAC_NET_X,   32,   GPCS_NET,         32, FEC_OFF,   _3_125G,      32,    {0,0,0,0},    1,    XAUI_MEDIA,   _10BIT_ON,  FEC_OFF},
    { NON_SUP_MODE,   MAC_NA,     NA_NUM,    PCS_NA,   NA_NUM,   FEC_OFF,   SPEED_NA,  NA_NUM,   {0,0,0,0},    0,    XAUI_MEDIA,   _10BIT_ON,  FEC_NA}
};

/* map the right supported modes table for each port */
MV_HWS_PORT_INIT_PARAMS *hwsPortsAldrinParamsSupModesMap[] =
{
    /* Serdeses 0-31*/
    hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,
    hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,
    hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,
    hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,
    hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,
    hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,
    hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,
    hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,hwsAldrinPort0SupModes,
    /* Serdes 32 */
    hwsAldrinPort32SupModes
};

/************************* functions ******************************************************/

