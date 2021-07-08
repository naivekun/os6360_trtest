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
* mvHwsBobcat2PortModeElements.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 12 $
*
*******************************************************************************/

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#define MV_BOBK_NW_PORT_START   56

/************************* definition *****************************************************/


/************************* Globals ********************************************************/
/* Last line of each SupModes array MUST be NON_SUP_MODE */
MV_HWS_PORT_INIT_PARAMS hwsBobKPort0SupModes[] = {
  /* port mode,   MAC Type     Mac Num   PCS Type   PCS Num   FEC        Speed      FirstLane  LanesList  LanesNum  Media         10Bit     FecSupported */
  {SGMII,         GEMAC_NET_SG,0,        GPCS_NET,      0,    FEC_OFF,    _1_25G,        0,    {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_ON, FEC_OFF        },
  {_1000Base_X,   GEMAC_NET_X, 0,        GPCS_NET,      0,    FEC_OFF,    _1_25G,        0,    {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_ON, FEC_OFF        },
  {SGMII2_5,      GEMAC_NET_X, 0,        GPCS_NET,      0,    FEC_OFF,    _3_125G,       0,    {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_ON, FEC_OFF        },
  {QSGMII,        QSGMII_MAC,  0,        QSGMII_PCS,    0,    FEC_OFF,    _5G ,          0,    {0,0,0,0},    1,     XAUI_MEDIA,  _20BIT_ON, FEC_OFF        },
  {NON_SUP_MODE,  MAC_NA,     NA_NUM,    PCS_NA,   NA_NUM,    FEC_OFF,   SPEED_NA,  NA_NUM,    {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_ON, FEC_NA}
};

MV_HWS_PORT_INIT_PARAMS hwsBobKPort1SupModes[] = {
  /* port mode,     MAC Type     Mac Num   PCS Type   PCS Num   FEC       Speed     FirstLane  LanesList  LanesNum  Media         10Bit     FecSupported */
    {QSGMII,        QSGMII_MAC,  1,        QSGMII_PCS,  1,    FEC_OFF,    _5G ,          0,    {0,0,0,0},    1,     XAUI_MEDIA,  _20BIT_ON, FEC_OFF        },
    {NON_SUP_MODE,  MAC_NA,     NA_NUM,    PCS_NA,   NA_NUM,  FEC_OFF,   SPEED_NA,  NA_NUM,    {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_ON, FEC_NA}
};

MV_HWS_PORT_INIT_PARAMS hwsBobKPort56SupModes[] = {
  /* port mode,    MAC Type    Mac Num    PCS Type   PCS Num  FEC        Speed     FirstLane   LanesList  LanesNum  Media         10Bit      FecSupported */
  {SGMII,          GEMAC_SG,   56,        GPCS,         56,   FEC_OFF,   _1_25G,        27,    {0,0,0,0},    1,     XAUI_MEDIA,   _10BIT_ON, FEC_OFF        },
  {_1000Base_X,    GEMAC_X,    56,        GPCS,         56,   FEC_OFF,   _1_25G,        27,    {0,0,0,0},    1,     XAUI_MEDIA,   _10BIT_ON, FEC_OFF        },
  {SGMII2_5,       GEMAC_X,    56,        GPCS,         56,   FEC_OFF,   _3_125G,       27,    {0,0,0,0},    1,     XAUI_MEDIA,   _10BIT_ON, FEC_OFF        },
  {QSGMII,         QSGMII_MAC, 56,        QSGMII_PCS,   56,   FEC_OFF,   _5G ,          27,    {0,0,0,0},    1,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF        },
  {_10GBase_KX4,   XGMAC,      56,        XPCS,         56,   FEC_OFF,   _3_125G,       24,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF        },
  {_10GBase_KX2,   XGMAC,      56,        XPCS,         56,   FEC_OFF,   _6_25G,        26,    {0,0,0,0},    2,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF        },
  {_10GBase_KR,    XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _10_3125G,     27,    {0,0,0,0},    1,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC },
  {_20GBase_KR,    XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _10_3125G,     26,    {0,0,0,0},    2,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC },
  {_40GBase_KR,    XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _10_3125G,     24,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC },
  {RXAUI,          XGMAC,      56,        XPCS,         56,   FEC_OFF,   _6_25G,        26,    {0,0,0,0},    2,     RXAUI_MEDIA,  _20BIT_ON, FEC_OFF|FC_FEC },
  {_20GBase_KX4,   XGMAC,      56,        XPCS,         56,   FEC_OFF,   _6_25G,        24,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF        },
  {_10GBase_SR_LR, XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _10_3125G,     27,    {0,0,0,0},    1,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF        },
  {_20GBase_SR_LR, XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _10_3125G,     26,    {0,0,0,0},    2,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC },
  {_40GBase_SR_LR, XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _10_3125G,     24,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC },
  {_12_1GBase_KR,  XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _12_5G,        27,    {0,0,0,0},    1,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC },
  {XLHGL_KR4,      XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _12_5G,        24,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC },
  {_12GBaseR,      XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _12_1875G,     27,    {0,0,0,0},    1,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC },
  {_48GBaseR,      XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _12_1875G,     24,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC },
  {_5GBaseR,       XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _5_15625G,     27,    {0,0,0,0},    1,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF        },
  {_29_09GBase_SR, XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _7_5G,         27,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF        },
  {_40GBase_CR4,   XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _10_3125G,     24,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC },
  {_40GBase_KR4,   XLGMAC,     56,        MMPCS,        56,   FEC_OFF,   _10_3125G,     24,    {0,0,0,0},    4,     XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC },
  {NON_SUP_MODE,   MAC_NA,     NA_NUM,    PCS_NA,   NA_NUM,   FEC_OFF,   SPEED_NA,  NA_NUM,    {0,0,0,0},    0,     XAUI_MEDIA,   _10BIT_ON, FEC_NA}

};

MV_HWS_PORT_INIT_PARAMS hwsBobKPort62SupModes[] = {
    /* port mode,    MAC Type    Mac Num    PCS Type   PCS Num  FEC        Speed     FirstLane   LanesList  LanesNum  Media         10Bit      FecSupported*/
    {SGMII,          GEMAC_NET_SG,  62,   GPCS_NET,       62,   FEC_OFF,   _1_25G,        20,    {0,0,0,0},    1,     XAUI_MEDIA,   _10BIT_ON, FEC_OFF        },
    {_1000Base_X,    GEMAC_NET_X,   62,   GPCS_NET,       62,   FEC_OFF,   _1_25G,        20,    {0,0,0,0},    1,     XAUI_MEDIA,   _10BIT_ON, FEC_OFF        },
    {NON_SUP_MODE,   MAC_NA,     NA_NUM,  PCS_NA,      NA_NUM,  FEC_OFF,   SPEED_NA,  NA_NUM,    {0,0,0,0},    0,     XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* map the right supported modes table for each port */
MV_HWS_PORT_INIT_PARAMS *hwsPortsBobKParamsSupModesMap[] =
{
    /*  0 */ hwsBobKPort0SupModes, /*  1 */ hwsBobKPort1SupModes, /*  2 */ hwsBobKPort1SupModes, /*  3 */ hwsBobKPort1SupModes,
    /*  4 */ hwsBobKPort0SupModes, /*  5 */ hwsBobKPort1SupModes, /*  6 */ hwsBobKPort1SupModes, /*  7 */ hwsBobKPort1SupModes,
    /*  8 */ hwsBobKPort0SupModes, /*  9 */ hwsBobKPort1SupModes, /* 10 */ hwsBobKPort1SupModes, /* 11 */ hwsBobKPort1SupModes,
    /* 12 */ hwsBobKPort0SupModes, /* 13 */ hwsBobKPort1SupModes, /* 14 */ hwsBobKPort1SupModes, /* 15 */ hwsBobKPort1SupModes,
    /* 16 */ hwsBobKPort0SupModes, /* 17 */ hwsBobKPort1SupModes, /* 18 */ hwsBobKPort1SupModes, /* 19 */ hwsBobKPort1SupModes,
    /* 20 */ hwsBobKPort0SupModes, /* 21 */ hwsBobKPort1SupModes, /* 22 */ hwsBobKPort1SupModes, /* 23 */ hwsBobKPort1SupModes,
    /* 24 */ hwsBobKPort0SupModes, /* 25 */ hwsBobKPort1SupModes, /* 26 */ hwsBobKPort1SupModes, /* 27 */ hwsBobKPort1SupModes,
    /* 28 */ hwsBobKPort0SupModes, /* 29 */ hwsBobKPort1SupModes, /* 30 */ hwsBobKPort1SupModes, /* 31 */ hwsBobKPort1SupModes,
    /* 32 */ hwsBobKPort0SupModes, /* 33 */ hwsBobKPort1SupModes, /* 34 */ hwsBobKPort1SupModes, /* 35 */ hwsBobKPort1SupModes,
    /* 36 */ hwsBobKPort0SupModes, /* 37 */ hwsBobKPort1SupModes, /* 38 */ hwsBobKPort1SupModes, /* 39 */ hwsBobKPort1SupModes,
    /* 40 */ hwsBobKPort0SupModes, /* 41 */ hwsBobKPort1SupModes, /* 42 */ hwsBobKPort1SupModes, /* 43 */ hwsBobKPort1SupModes,
    /* 44 */ hwsBobKPort0SupModes, /* 45 */ hwsBobKPort1SupModes, /* 46 */ hwsBobKPort1SupModes, /* 47 */ hwsBobKPort1SupModes,
    /* 48 */ NULL, /* 49 */ NULL,   /* 50 */ NULL, /* 51 */ NULL,   /* 52 */ NULL, /* 53 */ NULL,   /* 54 */ NULL, /* 55 */ NULL,
    /* 56 */ hwsBobKPort56SupModes, /* 57 */ hwsBobKPort56SupModes, /* 58 */ hwsBobKPort56SupModes, /* 59 */ hwsBobKPort56SupModes,
    /* 60 */ NULL, /* 61 */ NULL,
    /* 62 */ hwsBobKPort62SupModes,
    /* 63 */ NULL,  /* ports 63 is not in use */
    /* 64 */ hwsBobKPort56SupModes, /* 65 */ hwsBobKPort56SupModes, /* 66 */ hwsBobKPort56SupModes, /* 67 */ hwsBobKPort56SupModes,
    /* 68 */ hwsBobKPort56SupModes, /* 69 */ hwsBobKPort56SupModes, /* 70 */ hwsBobKPort56SupModes, /* 71 */ hwsBobKPort56SupModes
};

/* store serdes num for ports 56 - 71 */
GT_U16 bobK10GPortSerdesMapping[] =
    {27,     26,     25, 24,     /* 56 - 59 */
     NA_NUM, NA_NUM, 20, NA_NUM, /* 60 - 63 */
     35,     34,     33, 32,     /* 64 - 67 */
     31,     30,     29, 28};    /* 68 - 71 */

GT_U16 bobK20GPortSerdesMapping[] =
    {26,     24,      /* 56 & 58 */
     NA_NUM, NA_NUM,  /* 60 & 62 */
     34,     32,      /* 64 & 66 */
     30,     28};     /* 68 & 70 */

GT_U16 bobK40GPortSerdesMapping[] =
    {24,      /* 56 */
     NA_NUM,  /* 60 */
     32,      /* 64 */
     28};     /* 68 */

GT_U16 bobKQsgmiiGPortSerdesMapping[] =
    {27,      /* 56 */
     NA_NUM,  /* 60 */
     35,      /* 64 */
     31};     /* 68 */

/************************* functions ******************************************************/

GT_STATUS hwsBobKPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_INIT_PARAMS*    portParams
)
{
    GT_U16    laneCtnt = 0;
    GT_U16    *activeLanes = NULL;
    GT_U16    i = 0;

    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;

    if (portParams->portStandard == NON_SUP_MODE)
    {
        return GT_NOT_SUPPORTED;
    }
    if(phyPortNum >= MV_BOBK_NW_PORT_START)
    {
        /* check if 40G port mode */
        if((portParams->numOfActLanes == 4) && ((phyPortNum % 4) == 0))
        {
            portParams->firstLaneNum = bobK40GPortSerdesMapping[(phyPortNum - MV_BOBK_NW_PORT_START)/4];
        }
        /* check if 20G port mode */
        else if((portParams->numOfActLanes == 2) && ((phyPortNum % 2) == 0))
        {
            portParams->firstLaneNum =bobK20GPortSerdesMapping[(phyPortNum - MV_BOBK_NW_PORT_START)/2];
        }
        /* check if 10G port mode */
        else if(portParams->numOfActLanes == 1)
        {
            if(portParams->portStandard == QSGMII)
            {
                /* QSGMII is single lane port mode with special Serdes mapping */
                portParams->firstLaneNum =bobKQsgmiiGPortSerdesMapping[(phyPortNum - MV_BOBK_NW_PORT_START)/4];
            }
            else
            {
                portParams->firstLaneNum =bobK10GPortSerdesMapping[phyPortNum - MV_BOBK_NW_PORT_START];
            }
        }
    }
    else
    {
        portParams->firstLaneNum = (GT_U16)phyPortNum/4;
    }

    /* build active lane list */
    activeLanes = portParams->activeLanesList;
    for (i = portParams->firstLaneNum; /* build the active lanes list for this portMode */
         i < (portParams->firstLaneNum + portParams->numOfActLanes); i++)
    {
        activeLanes[laneCtnt++] = i;
    }
    return GT_OK;
}

GT_U32 hwsBobKPortParamsSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS         *curPortParams;
    MV_HWS_PORT_INIT_PARAMS         *newPortParams;
    MV_HWS_DEVICE_PORTS_ELEMENTS    hwsPortsBobKParams = hwsDevicesPortsElementsArray[devNum];
    GT_U32                          portModeIndex;

    /* avoid warnings */
    portGroup = portGroup;

    if (phyPortNum > hwsDeviceSpecInfo[devNum].portsNum)
    {
        return GT_OUT_OF_RANGE;
    }
    if (NULL == hwsPortsBobKParams)
    {
        return GT_NOT_INITIALIZED;
    }
    curPortParams = &hwsPortsBobKParams[phyPortNum].curPortParams;
    if (NON_SUP_MODE == portMode) /* Reset port */
    {
        /* initialize entry with default port params */
        hwsOsMemCopyFuncPtr(curPortParams, &hwsSingleEntryInitParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
        curPortParams->portMacNumber =  (GT_U16)phyPortNum;
        curPortParams->portPcsNumber =  (GT_U16)phyPortNum;
        curPortParams->firstLaneNum  =  (GT_U16)phyPortNum/4;
        return GT_OK;
    }

    newPortParams = hwsPortsElementsFindMode(hwsPortsBobKParams[phyPortNum].supModesCatalog, portMode, &portModeIndex);
    if (NULL == newPortParams || newPortParams->numOfActLanes == 0 || (phyPortNum % newPortParams->numOfActLanes) != 0)
    {
        return GT_NOT_SUPPORTED;
    }
    /* Copy the right port mode line from the port's specific supported modes list*/
    hwsOsMemCopyFuncPtr(curPortParams, newPortParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
    curPortParams->portMacNumber =  (GT_U16)phyPortNum;
    curPortParams->portPcsNumber =  (GT_U16)phyPortNum;
    curPortParams->firstLaneNum  =  (GT_U16)phyPortNum/4;
    curPortParams->portFecMode = hwsPortsBobKParams[phyPortNum].perModeFecList[portModeIndex];

    /* set the first lane and the active lanes list */
    return hwsBobKPortParamsGetLanes(devNum,portGroup,phyPortNum,curPortParams);
}
