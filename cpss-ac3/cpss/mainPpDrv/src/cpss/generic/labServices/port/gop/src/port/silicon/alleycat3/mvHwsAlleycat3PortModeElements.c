/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsAlleycat3PortModeElements.c
*
* @brief
*
* @version   11
********************************************************************************
*/

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#ifdef WIN32
extern void printPortModeTable(MV_HWS_PORT_INIT_PARAMS **hwsPortsParams, GT_U32 numOfPorts);
#endif

/* Last line of each array MUST be NON_SUP_MODE */
static MV_HWS_PORT_INIT_PARAMS hwsPort0InitParams[] = {
 /* port mode,                MAC Type   Mac Num     PCS Type    PCS Num    FEC       Speed   FirstLane    LanesList  LanesNum   Media       10Bit       FecSupported */
  {_100Base_FX,              MAC_NA,     NA_NUM,     PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_OFF         },
  {SGMII,                    GEMAC_NET_SG,0,         GPCS_NET,      0,      FEC_OFF,  _1_25G,    0,        {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_ON,  FEC_OFF         },
  {_1000Base_X,              GEMAC_NET_X, 0,         GPCS_NET,      0,      FEC_OFF,  _1_25G,    0,        {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_ON,  FEC_OFF         },
  {SGMII2_5,                 GEMAC_NET_X, 0,         GPCS_NET,      0,      FEC_OFF,  _3_125G,   0,        {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_ON,  FEC_OFF         },
  {QSGMII,                   QSGMII_MAC,  0,         QSGMII_PCS,    0,      FEC_OFF,  _5G ,      0,        {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_NA,  FEC_OFF         },
  {_10GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_10GBase_KX2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_10GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_20GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_40GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_100GBase_KR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {HGL,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {RHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {CHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {RXAUI,                     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_20GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_10GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_20GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_40GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_12_1GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {XLHGL_KR4,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {HGL16G,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {HGS,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {HGS4,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_100GBase_SR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {CHGL_LR12,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {TCAM,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_12Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_16Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_24Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_12Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_16Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_12Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_16Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_16Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_24Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {CHGL11_LR12,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_4Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_8Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_4Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {INTLKN_8Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_2_5GBase_QX,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_5GBase_DQX,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_5GBase_HX,                MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_12GBaseR,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_5_625GBaseR,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_48GBaseR,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_12GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_48GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_5GBaseR,                  MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_22GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_24GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_12_5GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_25GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_50GBase_KR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_25GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_50GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_100GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_25GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_50GBase_SR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_100GBase_SR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_100GBase_MLG,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_107GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_25GBase_KR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_50GBase_KR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_40GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_29_09GBase_SR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_40GBase_CR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_25GBase_CR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_50GBase_CR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_100GBase_CR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_25GBase_KR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_25GBase_CR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_25GBase_CR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_50GBase_CR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },
  {_26_7GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA          },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA}
};

  static MV_HWS_PORT_INIT_PARAMS hwsPort24InitParams[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media         10Bit     FecSupported*/
  {_100Base_FX,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_OFF         },
  {SGMII,                     GEMAC_SG,   24,        GPCS_NET,      24,     FEC_OFF,  _1_25G,    6,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    24,        GPCS_NET,      24,     FEC_OFF,  _1_25G,    6,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {SGMII2_5,                  GEMAC_X,    24,        GPCS_NET,      24,     FEC_OFF,  _3_125G,   6,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {QSGMII,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KX4,              XGMAC,      24,        XPCS,          24,     FEC_OFF,  _3_125G,   6,        {0,0,0,0},   4,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_10GBase_KX2,              XGMAC,      24,        XPCS,          24,     FEC_OFF,  _6_25G,    6,        {0,0,0,0},   2,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_10GBase_KR,               XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _10_3125G, 6,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_KR,               XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _10_3125G, 6,        {0,0,0,0},   2,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_40GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RXAUI,                     XGMAC,      24,        XPCS,          24,     FEC_OFF,  _6_25G,    6,        {0,0,0,0},   2,      RXAUI_MEDIA, _10BIT_OFF,FEC_OFF         },
  {_20GBase_KX4,              XGMAC,      24,        XPCS,          24,     FEC_OFF,  _6_25G,    6,        {0,0,0,0},   4,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_10GBase_SR_LR,            XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _10_3125G, 6,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_SR_LR,            XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _10_3125G, 6,        {0,0,0,0},   2,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_40GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12_1GBase_KR,             XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _12_5G,    6,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {XLHGL_KR4,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL16G,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS4,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL_LR12,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {TCAM,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL11_LR12,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_2_5GBase_QX,              XGMAC,      24,        XPCS,          24,     FEC_OFF,  _3_125G,   6,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_DQX,               XGMAC,      24,        XPCS,          24,     FEC_OFF,  _6_25G,    6,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_HX,                XGMAC,      24,        XPCS,          24,     FEC_OFF,  _3_125G,   6,        {0,0,0,0},   2,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_12GBaseR,                 XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _12_1875G, 6,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _5_625G,   6,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_48GBaseR,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_48GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_5GBaseR,                  XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _5_15625G, 6,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_22GBase_SR,               XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _11_5625G, 6,        {0,0,0,0},   2,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_24GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12_5GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_SR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_MLG,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_107GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_29_09GBase_SR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_CR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_CR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_26_7GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static MV_HWS_PORT_INIT_PARAMS hwsPort25InitParams[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media         10Bit     FecSupported*/
  {_100Base_FX,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_OFF         },
  {SGMII,                     GEMAC_SG,   25,        GPCS_NET,      25,     FEC_OFF,  _1_25G,    7,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    25,        GPCS_NET,      25,     FEC_OFF,  _1_25G,    7,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {SGMII2_5,                  GEMAC_X,    25,        GPCS_NET,      25,     FEC_OFF,  _3_125G,   7,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {QSGMII,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KX2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KR,               XLGMAC,     25,        MMPCS,         25,     FEC_OFF,  _10_3125G, 7,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RXAUI,                     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_20GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_SR_LR,            XLGMAC,     25,        MMPCS,         25,     FEC_OFF,  _10_3125G, 7,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_OFF|FC_FEC  },
  {_12_1GBase_KR,             XLGMAC,     25,        MMPCS,         25,     FEC_OFF,  _12_5G,    7,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_NA          },
  {XLHGL_KR4,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL16G,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS4,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL_LR12,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {TCAM,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL11_LR12,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_2_5GBase_QX,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_5GBase_DQX,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_5GBase_HX,                MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12GBaseR,                 XLGMAC,     25,        MMPCS,      25,        FEC_OFF,  _12_1875G, 7,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     25,        MMPCS,      25,        FEC_OFF,  _5_625G,   7,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_48GBaseR,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_48GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_5GBaseR,                  XLGMAC,     25,        MMPCS,      25,        FEC_OFF,  _5_15625G, 7,        {0,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_24GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12_5GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_SR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_MLG,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_107GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_29_09GBase_SR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_CR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_CR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_26_7GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA          },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static MV_HWS_PORT_INIT_PARAMS hwsPort26InitParams[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media         10Bit     FecSupported*/
  {_100Base_FX,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {SGMII,                     GEMAC_SG,   26,        GPCS_NET,      26,     FEC_OFF,  _1_25G,    8,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    26,        GPCS_NET,      26,     FEC_OFF,  _1_25G,    8,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {SGMII2_5,                  GEMAC_X,    26,        GPCS_NET,      26,     FEC_OFF,  _3_125G,   8,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {QSGMII,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KX2,              XGMAC,      26,        XPCS,          26,     FEC_OFF,  _6_25G,    8,        {0,0,0,0},  2,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_10GBase_KR,               XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _10_3125G, 8,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_KR,               XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _10_3125G, 8,        {0,0,0,0},  2,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_40GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RXAUI,                     XGMAC,      26,        XPCS,          26,     FEC_OFF,  _6_25G,    8,        {0,0,0,0},  2,       RXAUI_MEDIA, _10BIT_OFF,FEC_OFF         },
  {_20GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_SR_LR,            XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _10_3125G, 8,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_SR_LR,            XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _10_3125G, 8,        {0,0,0,0},  2,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_40GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12_1GBase_KR,             XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _12_5G,    8,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {XLHGL_KR4,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL16G,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS4,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL_LR12,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {TCAM,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL11_LR12,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_2_5GBase_QX,              XGMAC,      26,        XPCS,          26,     FEC_OFF,  _3_125G,   8,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_DQX,               XGMAC,      26,        XPCS,          26,     FEC_OFF,  _6_25G,    8,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_HX,                XGMAC,      26,        XPCS,          26,     FEC_OFF,  _3_125G,   8,        {0,0,0,0},  2,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_12GBaseR,                 XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _12_1875G, 8,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _5_625G,   8,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_48GBaseR,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_48GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_5GBaseR,                  XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _5_15625G, 8,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_22GBase_SR,               XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _11_5625G, 8,        {0,0,0,0},  2,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_24GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12_5GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_SR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_MLG,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_107GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_29_09GBase_SR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_CR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_CR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_26_7GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static MV_HWS_PORT_INIT_PARAMS hwsPort27InitParams[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media         10Bit     FecSupported*/
  {_100Base_FX,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {SGMII,                     GEMAC_SG,   27,        GPCS_NET,      27,     FEC_OFF,  _1_25G,    9,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    27,        GPCS_NET,      27,     FEC_OFF,  _1_25G,    9,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {SGMII2_5,                  GEMAC_X,    27,        GPCS_NET,      27,     FEC_OFF,  _3_125G,   9,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {QSGMII,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KX2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KR,               XLGMAC,     27,        MMPCS,         27,     FEC_OFF,  _10_3125G, 9,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RXAUI,                     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_20GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_SR_LR,            XLGMAC,     27,        MMPCS,         27,     FEC_OFF,  _10_3125G, 9,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12_1GBase_KR,             XLGMAC,     27,        MMPCS,         27,     FEC_OFF,  _12_5G,    9,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {XLHGL_KR4,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL16G,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS4,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL_LR12,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {TCAM,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL11_LR12,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_2_5GBase_QX,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_5GBase_DQX,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_5GBase_HX,                MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12GBaseR,                 XLGMAC,     27,        MMPCS,      27,        FEC_OFF,  _12_1875G, 9,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     27,        MMPCS,      27,        FEC_OFF,  _5_625G,   9,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_48GBaseR,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_48GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_5GBaseR,                  XLGMAC,     27,        MMPCS,      27,        FEC_OFF,  _5_15625G, 9,        {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_24GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12_5GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_SR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_MLG,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_107GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_29_09GBase_SR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_CR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_CR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_26_7GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static MV_HWS_PORT_INIT_PARAMS hwsPort28InitParams[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media         10Bit     FecSupported*/
  {_100Base_FX,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {SGMII,                     GEMAC_SG,   28,        GPCS_NET,   28,        FEC_OFF,  _1_25G,    10,       {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    28,        GPCS_NET,   28,        FEC_OFF,  _1_25G,    10,       {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
#if 0
  {SGMII2_5,                  MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
#else
  {SGMII2_5,                  GEMAC_X,    28,        GPCS_NET,   28,        FEC_OFF,  _3_125G,   10,       {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
#endif
  {QSGMII,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KX2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KR,               XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _10_3125G, 10,       {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RXAUI,                     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_20GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_SR_LR,            XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _10_3125G, 10,       {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12_1GBase_KR,             XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _12_5G,    10,       {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {XLHGL_KR4,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL16G,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS4,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL_LR12,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {TCAM,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL11_LR12,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_2_5GBase_QX,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_5GBase_DQX,               XGMAC,      28,        XPCS,       28,        FEC_OFF,  _6_25G,    10,       {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_HX,                MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12GBaseR,                 XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _12_1875G, 10,       {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _5_625G,   10,       {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_48GBaseR,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_48GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_5GBaseR,                  XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _5_15625G, 10,       {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_24GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12_5GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_SR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_MLG,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_107GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_29_09GBase_SR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_CR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_CR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_26_7GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA          },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static MV_HWS_PORT_INIT_PARAMS hwsPort29InitParams[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media          10Bit     FecSupported*/
  {_100Base_FX,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {SGMII,                     GEMAC_SG,   29,        GPCS_NET,   29,        FEC_OFF,  _1_25G,    11,       {0,0,0,0},  1,        XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    29,        GPCS_NET,   29,        FEC_OFF,  _1_25G,    11,       {0,0,0,0},  1,        XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
#if 0
  {SGMII2_5,                  MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
#else
  {SGMII2_5,                  GEMAC_X,    29,        GPCS_NET,   29,        FEC_OFF,  _3_125G,   11,       {0,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
#endif
  {QSGMII,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KX2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_KR,               XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _10_3125G, 11,       {0,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_KR,               XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _10_3125G, 10,       {0,0,0,0},  2,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_40GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {RXAUI,                     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_20GBase_KX4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_10GBase_SR_LR,            XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _10_3125G, 11,       {0,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_SR_LR,            XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _10_3125G, 10,       {0,0,0,0},  2,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_40GBase_SR_LR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12_1GBase_KR,             XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _12_5G,    11,       {0,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {XLHGL_KR4,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGL16G,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS,                       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {HGS4,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR10,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL_LR12,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {TCAM,                      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_6_25G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_10_3125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_12Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_12_5G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_16Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_24Lanes_3_125G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {CHGL11_LR12,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_3_125G,      MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_4Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {INTLKN_8Lanes_6_25G,       MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_2_5GBase_QX,              XGMAC,      29,        XPCS,       29,        FEC_OFF,  _3_125G,   11,       {0,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_DQX,               XGMAC,      29,        XPCS,       29,        FEC_OFF,  _6_25G,    11,       {0,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_HX,                XGMAC,      29,        XPCS,       29,        FEC_OFF,  _3_125G,   10,       {0,0,0,0},  2,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_12GBaseR,                 XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _12_1875G, 11,       {0,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _5_625G,   11,       {0,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_48GBaseR,                 MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_48GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_5GBaseR,                  XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _5_15625G, 11,       {0,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_24GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_12_5GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_SR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_SR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_SR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_MLG,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_107GBase_KR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_KR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_KR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_29_09GBase_SR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_40GBase_CR4,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_100GBase_CR4,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_KR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_S,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_25GBase_CR_C,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_50GBase_CR2_C,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
  {_26_7GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static MV_HWS_PORT_INIT_PARAMS hwsPort31InitParams[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media          10Bit     FecSupported*/
    {_100Base_FX,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {SGMII,                   GEMAC_SG,   31,        GPCS_NET,      31,     FEC_OFF,  _1_25G,    10,       {0,0,0,0},  1,        XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
    {_1000Base_X,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {SGMII2_5,                MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {QSGMII,                  MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_10GBase_KX4,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_10GBase_KX2,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_10GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_20GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_40GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_100GBase_KR10,          MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {HGL,                     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {RHGL,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {CHGL,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {RXAUI,                   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_20GBase_KX4,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_10GBase_SR_LR,          MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_20GBase_SR_LR,          MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_40GBase_SR_LR,          MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_12_1GBase_KR,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {XLHGL_KR4,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {HGL16G,                  MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {HGS,                     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {HGS4,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_100GBase_SR10,          MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {CHGL_LR12,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {TCAM,                    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_12Lanes_6_25G,    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_16Lanes_6_25G,    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_24Lanes_6_25G,    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_12Lanes_10_3125G, MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_16Lanes_10_3125G, MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_12Lanes_12_5G,    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_16Lanes_12_5G,    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_16Lanes_3_125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_24Lanes_3_125G,   MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {CHGL11_LR12,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_4Lanes_3_125G,    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_8Lanes_3_125G,    MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_4Lanes_6_25G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {INTLKN_8Lanes_6_25G,     MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_2_5GBase_QX,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_5GBase_DQX,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_5GBase_HX,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_12GBaseR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_5_625GBaseR,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_48GBaseR,               MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_12GBase_SR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_48GBase_SR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_5GBaseR,                MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_22GBase_SR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_24GBase_KR2,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_12_5GBase_KR,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_25GBase_KR2,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_50GBase_KR4,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_25GBase_KR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_50GBase_KR2,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_100GBase_KR4,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_25GBase_SR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_50GBase_SR2,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_100GBase_SR4,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_100GBase_MLG,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_107GBase_KR4,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_25GBase_KR_C,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_50GBase_KR2_C,          MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_40GBase_KR2,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_29_09GBase_SR,          MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_40GBase_CR4,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_25GBase_CR,             MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_50GBase_CR2,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_100GBase_CR4,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_25GBase_KR_S,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_25GBase_CR_S,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_25GBase_CR_C,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_50GBase_CR2_C,          MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },
    {_26_7GBase_KR,           MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_OFF,  SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA          },

    {NON_SUP_MODE,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

/* array of all ports (32) init parameters */
MV_HWS_PORT_INIT_PARAMS *hwsPortsAlleycat3Params[] =
{
    hwsPort0InitParams,
    /* ports 4 - 20 init by hwsAlleycat3Ports0To23Cfg()
       since they are based on port 0 topology */
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL,

    hwsPort24InitParams,
    hwsPort25InitParams,
    hwsPort26InitParams,
    hwsPort27InitParams,
    hwsPort28InitParams,
    hwsPort29InitParams,
    NULL,
    hwsPort31InitParams
};

/* init ports 1 - 23 */
GT_STATUS hwsAlleycat3Ports1To23Cfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup
)
{
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts = hwsDevicesPortsElementsArray[devNum];
    MV_HWS_PORT_INIT_PARAMS         *curPortParams;
    GT_U8 portNum;
    GT_U8 portMode;

    /* avoid warning */
    portGroup = portGroup;
    for (portNum = 1; portNum <= 23; portNum++)
    {
        curPortParams = (MV_HWS_PORT_INIT_PARAMS *)hwsOsMallocFuncPtr(sizeof(hwsPort0InitParams));
        if (curPortParams == NULL)
        {
          return GT_NO_RESOURCE;
        }

        curDevicePorts[portNum].supModesCatalog = curPortParams;
        hwsOsMemCopyFuncPtr(curPortParams, hwsPort0InitParams, sizeof(hwsPort0InitParams));

        for (portMode = 0; portMode < hwsDeviceSpecInfo[devNum].lastSupPortMode; portMode++)
        {

            if (curPortParams[portMode].portStandard == NON_SUP_MODE)
            {
                break;
            }

            if (curPortParams[portMode].portMacType == MAC_NA)
            {
                continue;
            }

            if (((portNum % 4) != 0) && curPortParams[portMode].portStandard == QSGMII)
            {
                curPortParams[portMode].portMacNumber = portNum; /* in order to be able to check the link */
                curPortParams[portMode].portPcsNumber = portNum; /* in order to be able to set loopback on port */
                curPortParams[portMode].numOfActLanes = 0;
                curPortParams[portMode].firstLaneNum = 0;

                continue;
            }
            if ((portNum % 4) != 0)
            {
                if(   curPortParams[portMode].portStandard == SGMII
                   || curPortParams[portMode].portStandard == SGMII2_5
                   || curPortParams[portMode].portStandard == _1000Base_X)
                {
                    curPortParams[portMode].portMacNumber = NA_NUM; /* in order to be able to check the link */
                    curPortParams[portMode].portPcsNumber = NA_NUM; /* in order to be able to set loopback on port */
                    curPortParams[portMode].numOfActLanes = 0;
                    curPortParams[portMode].firstLaneNum = NA_NUM;
                    continue;
                }
            }

            curPortParams[portMode].portMacNumber = portNum;
            curPortParams[portMode].portPcsNumber = portNum;
            curPortParams[portMode].firstLaneNum = portNum / 4;

        }
    }

    return GT_OK;
}


