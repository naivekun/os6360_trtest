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
* @file cpssDxChPortSerdesCfg.c
*
* @brief CPSS implementation for SerDes configuration and control facility.
*
* @version   88
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH/mvComPhyEomIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmEomIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>

GT_BOOL skipLion2PortDelete = GT_FALSE;
extern GT_BOOL skipLion2PortDeleteSet(GT_BOOL skip)
{
    GT_BOOL oldState;
    oldState = skipLion2PortDelete;
    skipLion2PortDelete = skip;
    return oldState;
}
extern GT_STATUS mvHwsMmPcs40GConnectWa
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  pcsNum
);

extern GT_STATUS mvHwsXPcsConnectWa
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  portNum
);

/* #define SERDES_CFG_DBG */
#ifdef SERDES_CFG_DBG
#define SERDES_DBG_DUMP_MAC(x)     cpssOsPrintf x
#define SERDES_TUNE_DBG
#else
#define SERDES_DBG_DUMP_MAC(x)
#endif

/* #define SERDES_TUNE_DBG */
#ifdef SERDES_TUNE_DBG
#define SERDES_TUNE_DUMP_MAC(x)     cpssOsPrintf x
#else
#define SERDES_TUNE_DUMP_MAC(x)
#endif

#define PRV_CPSS_DXCH_PORT_SERDES_SPEED_HW_TO_SW_CONVERT_MAC(swSerdesSpeed, hwSerdesSpeed)\
    switch(hwSerdesSpeed)                                                                 \
    {                                                                                     \
        case SPEED_NA:                                                                    \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_NA_E;                             \
            break;                                                                        \
        case _1_25G:                                                                      \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;                           \
            break;                                                                        \
        case _3_125G:                                                                     \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_3_125_E;                          \
            break;                                                                        \
        case _3_33G:                                                                      \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_3_333_E;                          \
            break;                                                                        \
        case _3_75G:                                                                      \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_3_75_E;                           \
            break;                                                                        \
        case _4_25G:                                                                      \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_4_25_E;                           \
            break;                                                                        \
        case _5G:                                                                         \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_5_E;                              \
            break;                                                                        \
        case _6_25G:                                                                      \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_6_25_E;                           \
            break;                                                                        \
        case _7_5G:                                                                       \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_7_5_E;                            \
            break;                                                                        \
        case _10_3125G:                                                                   \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E;                        \
            break;                                                                        \
        case _11_25G:                                                                     \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_11_25_E;                          \
            break;                                                                        \
        case _11_5625G:                                                                   \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_11_5625_E;                        \
            break;                                                                        \
        case _12_5G:                                                                      \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_12_5_E;                           \
            break;                                                                        \
        case _10_9375G:                                                                   \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_10_9375_E;                        \
            break;                                                                        \
        case _12_1875G:                                                                   \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_12_1875_E;                        \
            break;                                                                        \
        case _5_625G:                                                                     \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_5_625_E;                          \
            break;                                                                        \
        case _5_15625G:                                                                   \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_5_156_E;                          \
            break;                                                                        \
        case _12_8906G:                                                                   \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_12_8906_E;                        \
            break;                                                                        \
        case _20_625G:                                                                    \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_20_625_E;                         \
            break;                                                                        \
        case _25_78125G:                                                                  \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_25_78125_E;                       \
            break;                                                                        \
        case _27_5G:                                                                      \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_27_5_E;                           \
            break;                                                                        \
        case _28_05G:                                                                     \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_28_05_E;                          \
            break;                                                                        \
        case _26_25G:                                                                     \
            swSerdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_26_25_E;                          \
            break;                                                                        \
        default:                                                                          \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                \
    }


CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC  *serdesRxauiTxConfig = NULL;
GT_U16                               numOfSerdesRxauiConfig = 0;

/* array defining serdes speed used in given interface mode for given port data speed */
/* APPLICABLE DEVICES:  DxChXcat and above. */
CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency[CPSS_PORT_INTERFACE_MODE_NA_E][CPSS_PORT_SPEED_NA_E] =
{                                               /* 10M                                      100M                                  1G                                        10G                                   12G                               2.5G                                  5G                                     13.6G                           20G                                   40G                                   16G                            15G                             75G                             100G                    */
/* CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E */  {CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E  */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_MII_E           */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_SGMII_E         */ ,{CPSS_DXCH_PORT_SERDES_SPEED_1_25_E , CPSS_DXCH_PORT_SERDES_SPEED_1_25_E, CPSS_DXCH_PORT_SERDES_SPEED_1_25_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_3_125_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_XGMII_E         */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_3_125_E,  CPSS_DXCH_PORT_SERDES_SPEED_3_75_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_4_25_E, CPSS_DXCH_PORT_SERDES_SPEED_6_25_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_5_E , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_MGMII_E         */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,   */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_1_25_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_GMII_E,         */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_MII_PHY_E,      */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_QX_E,           */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_3_125_E, CPSS_DXCH_PORT_SERDES_SPEED_6_25_E , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_HX_E,           */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_6_25_E ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_3_125_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_RXAUI_E,        */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_6_25_E ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,   */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_1_25_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_QSGMII_E,       */ ,{CPSS_DXCH_PORT_SERDES_SPEED_5_E    , CPSS_DXCH_PORT_SERDES_SPEED_5_E   , CPSS_DXCH_PORT_SERDES_SPEED_5_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_XLG_E,          */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   ,  CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_5_156_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E   */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_3_125_E,  CPSS_DXCH_PORT_SERDES_SPEED_3_75_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_4_25_E, CPSS_DXCH_PORT_SERDES_SPEED_6_25_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_KR_E            */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E,  CPSS_DXCH_PORT_SERDES_SPEED_12_5_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E , CPSS_DXCH_PORT_SERDES_SPEED_5_156_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E, CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E}
/* CPSS_PORT_INTERFACE_MODE_HGL_E           */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_3_125_E,  CPSS_DXCH_PORT_SERDES_SPEED_3_75_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_4_25_E, CPSS_DXCH_PORT_SERDES_SPEED_6_25_E, CPSS_DXCH_PORT_SERDES_SPEED_12_5_E , CPSS_DXCH_PORT_SERDES_SPEED_3_333_E, CPSS_DXCH_PORT_SERDES_SPEED_3_125_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_12_5_E}
/* CPSS_PORT_INTERFACE_MODE_CHGL_12_E       */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_3_125_E,  CPSS_DXCH_PORT_SERDES_SPEED_3_75_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_4_25_E, CPSS_DXCH_PORT_SERDES_SPEED_6_25_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_ILKN12_E        */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_3_125_E,  CPSS_DXCH_PORT_SERDES_SPEED_3_75_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_4_25_E, CPSS_DXCH_PORT_SERDES_SPEED_6_25_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_SR_LR_E         */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E,  CPSS_DXCH_PORT_SERDES_SPEED_12_5_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E , CPSS_DXCH_PORT_SERDES_SPEED_5_156_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E, CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E}
/* CPSS_PORT_INTERFACE_MODE_ILKN16_E        */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_3_125_E,  CPSS_DXCH_PORT_SERDES_SPEED_3_75_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_4_25_E, CPSS_DXCH_PORT_SERDES_SPEED_6_25_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_ILKN24_E        */ ,{CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_NA_E  , CPSS_DXCH_PORT_SERDES_SPEED_3_125_E,  CPSS_DXCH_PORT_SERDES_SPEED_3_75_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_4_25_E, CPSS_DXCH_PORT_SERDES_SPEED_6_25_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E   , CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E, CPSS_DXCH_PORT_SERDES_SPEED_NA_E}
};

/* Values for xCat A2 and above Short Reach 125MHz SerDes RefClk */
GT_U32 xcatSerdesPowerUpSequence[][CPSS_DXCH_PORT_SERDES_SPEED_NA_E+2] =
                            /*  Registers values per requested frequency                                                    | register address                                 | delay
                                                                                                                            |  offset in                                       | after
                                1.25G  3.125G  3.75G   6.25G     5G    4.25G    2.5G   reserved  reserved reserved reserved | _lpSerdesConfig                                  | set   */
{
/* PLL_INTP_REG1        */     {0x800A, 0x800A, 0x800A, 0x800A, 0x800A, 0x800A, 0x800A, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG1_OFFSET_E,   0   },
/* PLL_INTP_REG2        */     {0x6614, 0x5515, 0x4413, 0x4415, 0x4414, 0x4413, 0x5514, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG2_OFFSET_E,   0   },
/* PLL_INTP_REG3        */     {0xA150, 0xA164, 0xA13c, 0xA164, 0xA150, 0xA144, 0xA150, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG3_OFFSET_E,   0   },
/* PLL_INTP_REG4        */     {0xBAAB, 0xFBAA, 0x99AC, 0xFBAA, 0xBAAB, 0x99AC, 0xBAAB, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG4_OFFSET_E,   0   },
/* PLL_INTP_REG5        */     {0x8B2C, 0x8720, 0x872c, 0x8720, 0x882c, 0x872C, 0x882c, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG5_OFFSET_E,   0   },
/* ANALOG_REG           */     {0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_ANALOG_REG0_OFFSET_E,     0   },
/* CALIBRATION_REG1     */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG1_OFFSET_E,0   },
/* CALIBRATION_REG2     */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG2_OFFSET_E,0   },
/* CALIBRATION_REG3     */     {0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG3_OFFSET_E,0   },
/* CALIBRATION_REG5     */     {0x0018, 0x0018, 0x0018, 0x0018, 0x0018, 0x0018, 0x0018, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG5_OFFSET_E,0   },
/* TRANSMIT_REG0        */     {0x80C0, 0x80C0, 0xA0C0, 0xA0C0, 0xA0C0, 0xA0C0, 0x80C0, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_OFFSET_E,   10  },
/* TRANSMIT_REG0        */     {0x8060, 0x8060, 0xA060, 0xA060, 0xA060, 0xA060, 0x8060, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_OFFSET_E,   0   },
/* TRANSMIT_REG1        */     {0x7704, 0x7704, 0x7704, 0x7704, 0x7704, 0x7704, 0x7704, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_OFFSET_E,   0   },
/* TRANSMIT_REG2        */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG2_OFFSET_E,   0   },
/* RECEIVER_REG1        */     {0x905A, 0x905A, 0x905A, 0x90AA, 0x90AA, 0x905A, 0x905A, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG1_OFFSET_E,   0   },
/* RECEIVER_REG2        */     {0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG2_OFFSET_E,   0   },
/* FFE_REG              */     {0x0343, 0x0343, 0x0343, 0x0343, 0x0343, 0x0343, 0x0343, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_OFFSET_E,        0   },
/* SLC_REG              */     {0x423F, 0x423F, 0x423F, 0x423F, 0x423F, 0x423F, 0x423F, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_SLC_REG_OFFSET_E,         0   },
/* REFERENCE_REG1       */     {0x5555, 0x5555, 0x5555, 0x5555, 0x5555, 0x5555, 0x5555, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_REFERENCE_REG1_OFFSET_E,  0   },
/* RESET_REG            */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,      0   },
/* CALIBRATION_REG3     */     {0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG3_OFFSET_E,0   },
/* CALIBRATION_REG7     */     {0xDAC0, 0xDAC0, 0xDAC0, 0xDAC0, 0xDAC0, 0xDAC0, 0xDAC0, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_OFFSET_E,0   },
/* RESET_REG            */     {0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,      0   },
/* RESET_REG            */     {0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,      5   },
/* CALIBRATION_REG0     */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,0   },
/* CALIBRATION_REG0     */     {0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,0   },
/* CALIBRATION_REG0     */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,1   },
/* DFE_F0F1_REG         */     {0x000A, 0x000A, 0x0000, 0x000A, 0x0013, 0x0000, 0x0000, 0x000A,   0x0000,  0x0000,  0x0000, PRV_CPSS_SW_PTR_ENTRY_UNUSED,                       0   },
                               {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_SW_PTR_ENTRY_UNUSED,                       0   }
};

/* Values for xCat A1 Short Reach 125MHz SerDes RefClk */
GT_U32 xcatA1SerdesPowerUpSequence[][CPSS_DXCH_PORT_SERDES_SPEED_NA_E+2] =
                            /*  Registers values per requested frequency                                                    | register address                                 | delay
                                                                                                                            |  offset in                                       | after
                                1.25G  3.125G  3.75G   6.25G     5G    4.25G    2.5G   reserved  reserved reserved reserved | _lpSerdesConfig                                  | set   */
{
/* PLL_INTP_REG1        */     {0x800A, 0x800A, 0x800A, 0x800A, 0x800A, 0x800A, 0x800A, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG1_OFFSET_E,   0   },
/* PLL_INTP_REG2        */     {0x6614, 0x5515, 0x4413, 0x4415, 0x4414, 0x4413, 0x5514, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG2_OFFSET_E,   0   },
/* PLL_INTP_REG3        */     {0x8150, 0x8164, 0x813c, 0x8164, 0x8150, 0x8144, 0x8150, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG3_OFFSET_E,   1   },
/* PLL_INTP_REG3        */     {0xA150, 0xA164, 0xA13c, 0xA164, 0xA150, 0xA144, 0xA150, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG3_OFFSET_E,   0   },
/* PLL_INTP_REG4        */     {0xBAAB, 0xFBAA, 0x99AC, 0xFBAA, 0xBAAB, 0x99AC, 0xBAAB, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG4_OFFSET_E,   0   },
/* PLL_INTP_REG5        */     {0x8B2C, 0x8720, 0x872c, 0x8720, 0x882c, 0x872C, 0x882c, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG5_OFFSET_E,   0   },
/* ANALOG_REG           */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_ANALOG_REG0_OFFSET_E,     0   },
/* CALIBRATION_REG1     */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG1_OFFSET_E,0   },
/* CALIBRATION_REG2     */     {0xc011, 0xc011, 0xc011, 0xc011, 0xC011, 0xc011, 0xc011, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG2_OFFSET_E,0   },
/* CALIBRATION_REG3     */     {0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG3_OFFSET_E,0   },
/* CALIBRATION_REG5     */     {0x0018, 0x0018, 0x0018, 0x0018, 0x0018, 0x0018, 0x0018, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG5_OFFSET_E,0   },
/* TRANSMIT_REG0        */     {0x8060, 0x8060, 0xA060, 0xA060, 0xA060, 0xA060, 0x8060, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_OFFSET_E,   0   },
/* TRANSMIT_REG1        */     {0x7704, 0x7704, 0x7704, 0x7704, 0x7704, 0x7704, 0x7704, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_OFFSET_E,   0   },
/* TRANSMIT_REG2        */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG2_OFFSET_E,   0   },
/* RECEIVER_REG1        */     {0x905A, 0x905A, 0x905A, 0x90AA, 0x90AA, 0x905A, 0x905A, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG1_OFFSET_E,   0   },
/* RECEIVER_REG2        */     {0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG2_OFFSET_E,   0   },
/* FFE_REG              */     {0x0243, 0x0243, 0x0243, 0x0243, 0x0243, 0x0243, 0x0243, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_OFFSET_E,        0   },
/* SLC_REG              */     {0x423F, 0x423F, 0x403F, 0x423F, 0x423F, 0x423F, 0x403F, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_SLC_REG_OFFSET_E,         0   },
/* REFERENCE_REG1       */     {0x5554, 0x5554, 0x5554, 0x5554, 0x5554, 0x5554, 0x5554, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_REFERENCE_REG1_OFFSET_E,  0   },
/* RESET_REG            */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,      0   },
/* CALIBRATION_REG3     */     {0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG3_OFFSET_E,0   },
/* CALIBRATION_REG7     */     {0xE0C0, 0xE0C0, 0xE0C0, 0xE0C0, 0xE0C0, 0xE0C0, 0xE0C0, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_OFFSET_E,0   },
/* RESET_REG            */     {0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,      0   },
/* RESET_REG            */     {0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,      5   },
/* CALIBRATION_REG0     */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,0   },
/* CALIBRATION_REG0     */     {0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,0   },
/* CALIBRATION_REG0     */     {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,1   },
                               {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   0x0000,  0x0000,  0x0000, PRV_CPSS_SW_PTR_ENTRY_UNUSED,                       0   }
};

/* Values for xCat2 Short Reach 125MHz SerDes RefClk */
GT_U32 xcat2SerdesPowerUpSequence[][CPSS_DXCH_PORT_SERDES_SPEED_NA_E+2] =
                            /*  Registers values per requested frequency                                                       | register address                                  | delay
                                                                                                                               |  offset in                                        | after
                                  1.25G  3.125G  reserved reserved 5G   reserved reserved reserved reserved reserved reserved  | _lpSerdesConfig                                   | set   */
{
/* PLL_INTP_REG1        */      {0x800A, 0x800A, 0x0000, 0x0000, 0x800A, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG1_OFFSET_E,     0   },
/* PLL_INTP_REG2        */      {0x6614, 0x5515, 0x0000, 0x0000, 0x4414, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG2_OFFSET_E,     0   },
/* PLL_INTP_REG3        */      {0xA150, 0xA164, 0x0000, 0x0000, 0xA150, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG3_OFFSET_E,     1   },
/* PLL_INTP_REG4        */      {0xBAAB, 0xFBAA, 0x0000, 0x0000, 0xBAAB, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG4_OFFSET_E,     0   },
/* PLL_INTP_REG5        */      {0x8B2C, 0x8720, 0x0000, 0x0000, 0x882c, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG5_OFFSET_E,     0   },
/* ANALOG_REG           */      {0x2000, 0x2000, 0x0000, 0x0000, 0x2000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_ANALOG_REG0_OFFSET_E,       0   },
/* CALIBRATION_REG1     */      {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG1_OFFSET_E,  0   },
/* CALIBRATION_REG2     */      {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG2_OFFSET_E,  0   },
/* CALIBRATION_REG3     */      {0x4000, 0x4000, 0x0000, 0x0000, 0x4000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG3_OFFSET_E,  0   },
/* CALIBRATION_REG5     */      {0x0018, 0x0018, 0x0000, 0x0000, 0x0018, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG5_OFFSET_E,  0   },
/* TRANSMIT_REG0        */      {0x80C0, 0x80C0, 0x0000, 0x0000, 0xA0C0, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_OFFSET_E,     10  },
/* TRANSMIT_REG0        */      {0x8060, 0x8060, 0x0000, 0x0000, 0xA060, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_OFFSET_E,     0   },
/* TRANSMIT_REG1        */      {0x7F2D, 0x7F2D, 0x0000, 0x0000, 0x750C, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_OFFSET_E,     0   },
/* TRANSMIT_REG2        */      {0x0000, 0x0900, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG2_OFFSET_E,     0   },
/* RECEIVER_REG1        */      {0x905A, 0x905A, 0x0000, 0x0000, 0x90AA, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG1_OFFSET_E,     0   },
/* RECEIVER_REG2        */      {0x0800, 0x0800, 0x0000, 0x0000, 0x0800, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG2_OFFSET_E,     0   },
/* FFE_REG              */      {0x0266, 0x037F, 0x0000, 0x0000, 0x0371, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_OFFSET_E,          0   },
/* SLC_REG              */      {0x423F, 0x423F, 0x0000, 0x0000, 0x423F, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_SLC_REG_OFFSET_E,           0   },
/* REFERENCE_REG1       */      {0x5555, 0x5555, 0x0000, 0x0000, 0x5555, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_REFERENCE_REG1_OFFSET_E,    0   },
/* RESET_REG            */      {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,        0   },
/* CALIBRATION_REG3     */      {0x4000, 0x4000, 0x0000, 0x0000, 0x4000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG3_OFFSET_E,  0   },
/* CALIBRATION_REG7     */      {0xE0C0, 0xE0C0, 0x0000, 0x0000, 0xE0C0, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_OFFSET_E,  0   },
/* RESET_REG            */      {0x8040, 0x8040, 0x0000, 0x0000, 0x8040, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,        0   },
/* RESET_REG            */      {0x8000, 0x8000, 0x0000, 0x0000, 0x8000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,        5   },
/* CALIBRATION_REG0     */      {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,  0   },
/* CALIBRATION_REG0     */      {0x8000, 0x8000, 0x0000, 0x0000, 0x8000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,  0   },
/* CALIBRATION_REG0     */      {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,  1   },
                                {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    0x0000,  0x0000,  0x0000,    PRV_CPSS_SW_PTR_ENTRY_UNUSED,                         0   }
};

/* Values for Lion B0 Long Reach 156.25MHz SerDes RefClk */
GT_U32 lionSerdesPowerUpSequence[][CPSS_DXCH_PORT_SERDES_SPEED_NA_E+2] =
                            /* Registers values per requested frequency                                                     | register address                                  | delay
                                                                                                                            |  offset in                                        | after
                                 1.25G  3.125G  reserved 6.25G    5G   reserved reserved 5.156G  reserved reserved reserved | _lpSerdesConfig                                   | set   */
{
/* PLL_INTP_REG1        */      {0x800A, 0x800A, 0x0000, 0x800A, 0x800A, 0x0000, 0x0000, 0x800A,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG1_OFFSET_E,       0   },
/* PLL_INTP_REG2        */      {0x6614, 0x5515, 0x0000, 0x4415, 0x4414, 0x0000, 0x0000, 0x4414,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG2_OFFSET_E,       0   },
/* PLL_INTP_REG3        */      {0xA140, 0xA150, 0x0000, 0xA150, 0xA140, 0x0000, 0x0000, 0xA142,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG3_OFFSET_E,       0   },
/* PLL_INTP_REG4        */      {0xBAAB, 0xFBAA, 0x0000, 0xFBAA, 0xBAAB, 0x0000, 0x0000, 0xBAAB,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG4_OFFSET_E,       0   },
/* PLL_INTP_REG5        */      {0x8B2C, 0x8720, 0x0000, 0x8720, 0x882C, 0x0000, 0x0000, 0x882C,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG5_OFFSET_E,       0   },
/* ANALOG_REG           */      {0x2000, 0x2000, 0x0000, 0x2000, 0x2000, 0x0000, 0x0000, 0x2000,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_ANALOG_REG0_OFFSET_E,         0   },
/* CALIBRATION_REG1     */      {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG1_OFFSET_E,    0   },
/* CALIBRATION_REG2     */      {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG2_OFFSET_E,    0   },
/* CALIBRATION_REG3     */      {0x4000, 0x4000, 0x0000, 0x4000, 0x4000, 0x0000, 0x0000, 0x4000,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG3_OFFSET_E,    0   },
/* CALIBRATION_REG5     */      {0x0018, 0x0018, 0x0000, 0x0018, 0x0018, 0x0000, 0x0000, 0x0018,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG5_OFFSET_E,    0   },
/* TRANSMIT_REG0        */      {0x80C0, 0x80C0, 0x0000, 0xA0C0, 0xA0C0, 0x0000, 0x0000, 0xA0C0,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_OFFSET_E,       5   },
/* TRANSMIT_REG0        */      {0x8060, 0x8060, 0x0000, 0xA060, 0xA060, 0x0000, 0x0000, 0xA060,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_OFFSET_E,       0   },
/* TRANSMIT_REG1        */      {0x770A, 0x3A09, 0x0000, 0x7F2D, 0x7F2D, 0x0000, 0x0000, 0x7F2D,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_OFFSET_E,       0   },
/* TRANSMIT_REG2        */      {0x0000, 0x0000, 0x0000, 0x0100, 0x0100, 0x0000, 0x0000, 0x0100,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG2_OFFSET_E,       0   },
/* RECEIVER_REG1        */      {0x905A, 0x905A, 0x0000, 0x90AA, 0x90AA, 0x0000, 0x0000, 0x9005,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG1_OFFSET_E,       0   },
/* RECEIVER_REG2        */      {0x0800, 0x0800, 0x0000, 0x0800, 0x0800, 0x0000, 0x0000, 0x0800,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG2_OFFSET_E,       0   },
/* FFE_REG              */      {0x0266, 0x0336, 0x0000, 0x036F, 0x036F, 0x0000, 0x0000, 0x036F,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_OFFSET_E,            0   },
/* DFE_F0F1_REG         */      {0x000A, 0x000A, 0x0000, 0x000A, 0x0013, 0x0000, 0x0000, 0x000A,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_DFE_F0F1_COEFFICIENT_CTRL_OFFSET_E,0},
/* SLC_REG              */      {0x423F, 0x423F, 0x0000, 0x423F, 0x423F, 0x0000, 0x0000, 0x423F,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_SLC_REG_OFFSET_E,             0   },
/* REFERENCE_REG1       */      {0x5555, 0x5555, 0x0000, 0x5555, 0x5555, 0x0000, 0x0000, 0x5555,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_REFERENCE_REG1_OFFSET_E,      0   },
/* RESET_REG            */      {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,          0   },
/* CALIBRATION_REG3     */      {0x4000, 0x4000, 0x0000, 0x4000, 0x4000, 0x0000, 0x0000, 0x4000,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG3_OFFSET_E,    0   },
/* CALIBRATION_REG7     */      {0xE0C0, 0xE0C0, 0x0000, 0xE0C0, 0xE0C0, 0x0000, 0x0000, 0xE0C0,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_OFFSET_E,    0   },
/* RESET_REG            */      {0x8040, 0x8040, 0x0000, 0x8040, 0x8040, 0x0000, 0x0000, 0x8040,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,          0   },
/* RESET_REG            */      {0x8000, 0x8000, 0x0000, 0x8000, 0x8000, 0x0000, 0x0000, 0x8000,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,          5   },
/* CALIBRATION_REG0     */      {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,    0   },
/* CALIBRATION_REG0     */      {0x8000, 0x8000, 0x0000, 0x8000, 0x8000, 0x0000, 0x0000, 0x8000,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,    0   },
/* CALIBRATION_REG0     */      {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  0x0000,  0x0000,  0x0000,   PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,    0   },
                                {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  0x0000,  0x0000,  0x0000,   PRV_CPSS_SW_PTR_ENTRY_UNUSED,                           0   }
};

/* matrix [register][] = reg value */
/* Values for xCat 125MHz SerDes RefClk */
/* This matrix containes External Config Register values for
   preinit and postinit sequences. the left column values are intended for
   FLEX port in SGMII mode, the right column for all others. */
GT_U32 lpSerdesExtConfig[PRV_CPSS_DXCH_PORT_SERDES_EXT_REG_NUM_E][2] =
{
/* SGMII   other */
/*on Flex        */
/*   port        */
 {0x0000,  0x0000 }, /* External Config Register2 */
 {0xBE80,  0x3E80 }, /* External Config Register1 */
 {0x0008,  0x0008 }, /* External Config Register2 */
 {0x0018,  0x0018},  /* External Config Register2 */
 {0x0008,  0x0008},  /* External Config Register2 */
 {0x0028,  0x0028}   /* External Config Register2 */
};

/* Values for xCat2 125MHz SerDes RefClk */
GT_U32 lpSerdesExtConfig_xCat2_125Clk[PRV_CPSS_DXCH_PORT_SERDES_EXT_REG_NUM_E][2] =
{
/* 1.25G   other */
 {0x0000,  0x0000}, /* External Config Register2 */
 {0xBE80,  0x4E00}, /* External Config Register1 */
 {0x0008,  0x0008}, /* External Config Register2 */
 {0x0018,  0x0018}, /* External Config Register2 */
 {0x0008,  0x0008}, /* External Config Register2 */
 {0x0028,  0x0028}  /* External Config Register2 */
};

/* Values for Lion B0 156.25MHz SerDes RefClk
   Left column 1.25G and 5.156; right - all others */
GT_U32 lpSerdesExtConfig_Lion_B0_156Clk_LR[PRV_CPSS_DXCH_PORT_SERDES_EXT_REG_NUM_E][2] =
{
 {0x0000,  0x0000}, /* External Config Register2 */
 {0xCE00,  0x4E00}, /* External Config Register1 */
 {0x0008,  0x0008}, /* External Config Register2 */
 {0x0018,  0x0018}, /* External Config Register2 */
 {0x0008,  0x0008}, /* External Config Register2 */
 {0x013F,  0x013F}  /* SERDES Miscellaneous */
};

static GT_STATUS cheetah1_2_portSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
);

static GT_STATUS lionPortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
);

static GT_STATUS lion2PortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
);

/* array of pointers to port SerDes power set functions per DXCH ASIC family */
PRV_CPSS_DXCH_PORT_SERDES_POWER_STATUS_SET_FUN portSerdesPowerStatusSetFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1]=
{
/* CPSS_PP_FAMILY_CHEETAH_E     */  cheetah1_2_portSerdesPowerStatusSet,
/* CPSS_PP_FAMILY_CHEETAH2_E    */  cheetah1_2_portSerdesPowerStatusSet,
/* CPSS_PP_FAMILY_CHEETAH3_E    */  NULL,
/* CPSS_PP_FAMILY_DXCH_XCAT_E   */  prvCpssDxChXcatPortSerdesPowerStatusSet,
/* CPSS_PP_FAMILY_DXCH_XCAT3_E  */  prvCpssDxChXcatPortSerdesPowerStatusSet,
/* CPSS_PP_FAMILY_DXCH_LION_E   */  lionPortSerdesPowerStatusSet,
/* CPSS_PP_FAMILY_DXCH_XCAT2_E  */  prvCpssDxChXcatPortSerdesPowerStatusSet,
/* CPSS_PP_FAMILY_DXCH_LION2_E  */  lion2PortSerdesPowerStatusSet,
/* CPSS_PP_FAMILY_DXCH_LION3_E  */  lion2PortSerdesPowerStatusSet,
/* CPSS_PP_FAMILY_DXCH_BOBCAT2_E */ NULL
};

static GT_STATUS lion2ShareDisableWaExecute
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroupId,
    IN  GT_PHYSICAL_PORT_NUM    localPort,
    IN  GT_BOOL                 shareEnabled
);

/**
* @internal serDesConfig function
* @endinternal
*
* @brief   returns two dimantional array values from device specific arrays
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
*
* @param[out] serdesPowerUpSequencePtr - pointer to SERDES config array
* @param[out] serdesExtCfgPtr          - pointer to SERDES extended config array
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - if serdes reference clock is not 156.25
*/
GT_STATUS serDesConfig
(
    IN  GT_U8 devNum,
    OUT PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY **serdesPowerUpSequencePtr,
    OUT PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_ARRAY  **serdesExtCfgPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
       (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.serdesRefClock !=
                        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        *serdesPowerUpSequencePtr = lionSerdesPowerUpSequence;
        *serdesExtCfgPtr = lpSerdesExtConfig_Lion_B0_156Clk_LR;
        return GT_OK;
    }

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION_E:
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            *serdesPowerUpSequencePtr = lionSerdesPowerUpSequence;
            *serdesExtCfgPtr = lpSerdesExtConfig_Lion_B0_156Clk_LR;
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
            *serdesPowerUpSequencePtr = xcat2SerdesPowerUpSequence;
            *serdesExtCfgPtr = lpSerdesExtConfig_xCat2_125Clk;
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT_E:
            *serdesPowerUpSequencePtr = (PRV_CPSS_PP_MAC(devNum)->revision > 2)
                        ? xcatSerdesPowerUpSequence : xcatA1SerdesPowerUpSequence;
            *serdesExtCfgPtr = lpSerdesExtConfig;
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            *serdesPowerUpSequencePtr = xcatSerdesPowerUpSequence;
            *serdesExtCfgPtr = lpSerdesExtConfig;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortGenSerDesConfig function
* @endinternal
*
* @brief   Write to HW serdes power sequence
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - port group number
* @param[in] startSerdes              - first serdes to configure
* @param[in] serdesesNum              - number of serdeses to configure
* @param[in] serdesSpeed              - required serdes frequency
* @param[in] serdesPowerUpSequencePtr - (ptr to) serdes power up sequnce array
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChPortGenSerDesConfig
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   startSerdes,
    IN GT_U32   serdesesNum,
    IN CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesSpeed,
    IN PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY    *serdesPowerUpSequencePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      i,          /* iterator */
                cfgStep,    /* step in power up sequence */
                regAddr,    /* register address */
                regValue,   /* register value */
                regAddr2,    /* register address */
                regValue2,   /* register value */
                delay,      /* waiting time after configuration */
                *serdesRegsAddrStructPtr; /* (ptr to) registers addresses DB */
    CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr;
    GT_U32      lanesNumInDev;
    GT_U32      globalLaneNum;

    lanesNumInDev = prvCpssDxChHwInitNumOfSerdesGet(devNum);

    globalLaneNum = (CPSS_PORT_GROUP_UNAWARE_MODE_CNS == portGroupId) ? 0 :
                                                    (portGroupId * lanesNumInDev);
    for(cfgStep = 0;
        serdesPowerUpSequencePtr[cfgStep][CPSS_DXCH_PORT_SERDES_SPEED_NA_E]
            != PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        cfgStep++)
    {
        PRV_PER_SERDES_LOOP_MAC
        {
            serdesRegsAddrStructPtr = (GT_U32*)&PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i];
            regValue = serdesPowerUpSequencePtr[cfgStep][serdesSpeed];
            regAddr = serdesRegsAddrStructPtr[serdesPowerUpSequencePtr[cfgStep]
                                        [CPSS_DXCH_PORT_SERDES_SPEED_NA_E]];

            if(NULL != PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr)
            {/* if serdes fine tuning values DB initialized */

                if(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[globalLaneNum+i] != NULL)
                {
                    tuneValuesPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[globalLaneNum+i][serdesSpeed];

                    switch(serdesPowerUpSequencePtr[cfgStep][CPSS_DXCH_PORT_SERDES_SPEED_NA_E])
                    {
                        case PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_OFFSET_E:
                            SERDES_TUNE_DUMP_MAC(("FFE_REG0 0x%x:origVal=0x%x,", regAddr, regValue));
                            U32_SET_FIELD_MAC(regValue,0,4,tuneValuesPtr->ffeC);
                            U32_SET_FIELD_MAC(regValue,4,3,tuneValuesPtr->ffeR);
                            U32_SET_FIELD_MAC(regValue,8,2,tuneValuesPtr->ffeS);
                            SERDES_TUNE_DUMP_MAC(("newVal=0x%x\n", regValue));
                        break;

                        case PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_OFFSET_E:
                            SERDES_TUNE_DUMP_MAC(("TRANSMIT_REG0 0x%x:origVal=0x%x,", regAddr, regValue));
                            U32_SET_FIELD_MAC(regValue,15,1,BOOL2BIT_MAC(tuneValuesPtr->txEmphEn));
                            SERDES_TUNE_DUMP_MAC(("newVal=0x%x\n", regValue));
                        break;

                        case PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_OFFSET_E:
                            SERDES_TUNE_DUMP_MAC(("TRANSMIT_REG1 0x%x:origVal=0x%x,", regAddr, regValue));
                            U32_SET_FIELD_MAC(regValue,0,4,tuneValuesPtr->txEmphAmp);
                            U32_SET_FIELD_MAC(regValue,8,5,tuneValuesPtr->txAmp);
                            U32_SET_FIELD_MAC(regValue,13,1,tuneValuesPtr->txAmpAdj);
                            SERDES_TUNE_DUMP_MAC(("newVal=0x%x\n", regValue));
                        break;

                        case PRV_CPSS_DXCH_PORT_SERDES_DFE_F0F1_COEFFICIENT_CTRL_OFFSET_E:
                            SERDES_TUNE_DUMP_MAC(("DFE_F0F1 0x%x:origVal=0x%x,", regAddr, regValue));
                            U32_SET_FIELD_MAC(regValue,0,5,tuneValuesPtr->dfe);
                            SERDES_TUNE_DUMP_MAC(("newVal=0x%x\n", regValue));
                        break;

                        case PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_OFFSET_E:
                            SERDES_TUNE_DUMP_MAC(("CALIBRATION_REG7 0x%x:origVal=0x%x,", regAddr, regValue));
                            U32_SET_FIELD_MAC(regValue,8,7,tuneValuesPtr->sampler);
                            SERDES_TUNE_DUMP_MAC(("newVal=0x%x\n", regValue));
                        break;

                        case PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG1_OFFSET_E:
                            /* configure squelch in receiverReg0 before RECEIVER_REG1 configuration */
                            regAddr2 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            serdesConfig[i].receiverReg0;
                            if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr2,
                                                                            0, 4, &regValue2)) != GT_OK)
                                return rc;
                            SERDES_TUNE_DUMP_MAC(("receiverReg0 0x%x:origVal bits 0:3=0x%x,", regAddr2, regValue2));
                            if((rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr2,
                                                                            0, 4, tuneValuesPtr->sqlch)) != GT_OK)
                                return rc;
                            SERDES_TUNE_DUMP_MAC(("newVal=0x%x\n", tuneValuesPtr->sqlch));
                        break;

                        default:
                            break;
                    }
                }
            }

            if(PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_OFFSET_E !=
                serdesPowerUpSequencePtr[cfgStep][CPSS_DXCH_PORT_SERDES_SPEED_NA_E])
            {
                rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,regAddr,regValue);
            }
            else
            {
                /* don't touch bit 0 of transmit reg 0 it's set by
                    cpssDxChPortSerdesTxEnableSet */
                rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum,portGroupId,
                                                    regAddr,0xFFFFFFFE,regValue);
            }
            if (rc != GT_OK)
                return rc;

            SERDES_DBG_DUMP_MAC(("cfgStep=%d,regAddr=0x%x,regValue=0x%x;\n",
                                cfgStep, regAddr, regValue));
        }
        if((delay = serdesPowerUpSequencePtr[cfgStep]
                                    [CPSS_DXCH_PORT_SERDES_SPEED_NA_E+1]) > 0)
        {
            SERDES_DBG_DUMP_MAC(("HW_WAIT_MILLISECONDS_MAC(%d);\n", delay));
            HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,delay);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortIfModeSerdesNumGet function
* @endinternal
*
* @brief   Get number of first serdes and quantity of serdeses occupied by given
*         port in given interface mode. Extended function used directly only in
*         special cases when naturally error would be returned.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port media interface mode
*
* @param[out] startSerdesPtr           - first used serdes number
* @param[out] numOfSerdesLanesPtr      - quantity of serdeses occupied
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if port doesn't support given interface mode
* @retval GT_NOT_SUPPORTED         - wrong device family
*
* @note While working on Lion2/3 prior to call this function, port speed must
*       be defined.
*
*/
GT_STATUS prvCpssDxChPortIfModeSerdesNumGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    OUT GT_U32                          *startSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
)
{
    GT_STATUS       rc;             /* return code */
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U8           sdVecSize;      /* size of serdes redundancy array */
    GT_U16          *sdVectorPtr;   /* serdes redundancy array */
    GT_PHYSICAL_PORT_NUM localPort; /* number of port in its port group */
    GT_U32          portMacNum;     /* MAC number */
    GT_U32          portGroup;      /* local core number */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    *startSerdesPtr = portMacNum;
    *numOfSerdesLanesPtr = 0 ;
    if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
    {
        return GT_OK;
    }

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {/* xCat2 stack ports have only one SERDES.
        xCat2 each 4 network ports use same SERDES. */
        *startSerdesPtr = (portNum > 23) ? ((portNum-24)+6) : (localPort>>2);
        *numOfSerdesLanesPtr = 1;
        return GT_OK;
    }
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {/* for xCat flex links, for xCat network ports see below */
        *startSerdesPtr = (PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portNum) == GT_TRUE) ?
                                ((localPort%24)<<2) + 6 : (localPort>>2);
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        *startSerdesPtr = localPort<<1;
    }
    else if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
            (PRV_CPSS_SIP_5_CHECK_MAC(devNum))||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
    {
        if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
        {
            if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPort, _1000Base_X, &curPortParams); /* use here any single-serdes i/f to get local serdes of port */
                if (GT_OK != rc)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
                sdVecSize   = curPortParams.numOfActLanes;
                sdVectorPtr = curPortParams.activeLanesList;

                if (sdVecSize > 1)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                else
                {
                    *startSerdesPtr = sdVectorPtr[0];
                    *numOfSerdesLanesPtr = sdVecSize;
                }
            }
            else
            {
                *numOfSerdesLanesPtr = 0;
                *startSerdesPtr = 0;
            }
        }
        else
        {
            MV_HWS_PORT_STANDARD portMode;  /* port i/f mode and speed translated to BlackBox enum */
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode,
                                PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum),
                                &portMode);
            if(rc != GT_OK)
            {
                return rc;
            }
            if((ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E) ||
               (ifMode == CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E))
            {
                localPort = localPort&0xFFFFFFFC;
            }
            rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPort, portMode, &curPortParams);
            if (GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            sdVecSize   = curPortParams.numOfActLanes;
            sdVectorPtr = curPortParams.activeLanesList;

            *startSerdesPtr = sdVectorPtr[0];
            if(CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == ifMode)
            {
                *numOfSerdesLanesPtr = 0;
            }
            else
            {
                *numOfSerdesLanesPtr = sdVecSize;
            }
        }

        if(((*startSerdesPtr) + (*numOfSerdesLanesPtr)) > prvCpssDxChHwInitNumOfSerdesGet(devNum))
        {
            /* this check to avoid ACCESS VIOLATION on next DB:
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes]
            */

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "DevNum[%d] hold [%d] SERDESes but calculated SERDES[%d..%d] for portMacNum[%d] \n",
                devNum ,
                prvCpssDxChHwInitNumOfSerdesGet(devNum) ,
                (*startSerdesPtr) ,
                (*startSerdesPtr) + (*numOfSerdesLanesPtr) - 1,
                portMacNum
                );
        }

        return GT_OK;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    switch(ifMode)
    {
        /* for modes using just part of lanes posessed by port must return all
         * lanes, because they may be swapped - resposibility of application
         * to know which lanes to use and provide laneBmp in cpssDxChPortSerdesPowerStatusSet
         */
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_QX_E:
        case CPSS_PORT_INTERFACE_MODE_100BASE_FX_E:
            *numOfSerdesLanesPtr = 1;
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                if(PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portMacNum) == GT_TRUE)
                    *numOfSerdesLanesPtr = 4;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E:
            *numOfSerdesLanesPtr = 2;
            break;

        case CPSS_PORT_INTERFACE_MODE_HX_E:
        case CPSS_PORT_INTERFACE_MODE_RXAUI_E:
            /* RXAUI on FlexLink ports on XCAT uses serdes 0 and 2, thus we return all 4*/
            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) &&
               PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portNum) == GT_TRUE)
            {
                *numOfSerdesLanesPtr = 4;
            }
            else
            {
                *numOfSerdesLanesPtr = 2;
            }

            break;

        case CPSS_PORT_INTERFACE_MODE_XGMII_E: /* XAUI */
            *numOfSerdesLanesPtr = 4;
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
            {/* if neighbor port is in special mode - don't touch its serdeses or mac */
                if((localPort%2) == 0)
                {
                    if(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portNum+1) == CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E)
                        *numOfSerdesLanesPtr = 2;
                }
            }

            break;

        case CPSS_PORT_INTERFACE_MODE_XLG_E: /* 40G */
            *numOfSerdesLanesPtr = 8;

            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
            {/* in Lion use serdes of ports 8-11 (two per port),
                while configuring XLG on port 10 */
                *startSerdesPtr = 16;
            }
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

            break;

        case CPSS_PORT_INTERFACE_MODE_NA_E:
            *numOfSerdesLanesPtr = 0;
            *startSerdesPtr = 0;
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
            {
                /* in available Lion boxes port 10 capable for 40G support
                 * it's implemented by taking over serdeses of ports 8,9,10,11
                 * so when internal function asking about ports 8,9,11 while
                 * port 10 is XLG - return XLG
                 */
                if (localPort == 8 || localPort == 9 || localPort == 11)
                {
                    GT_PHYSICAL_PORT_NUM  xlgGlobalPortNum;/* global number of
                                                       port 10(40G main port) */
                    GT_U32 portGroupId;   /* number of port group of the port */

                    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portNum);
                    xlgGlobalPortNum = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,portGroupId,10);
                    if (PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,xlgGlobalPortNum) == CPSS_PORT_INTERFACE_MODE_XLG_E)
                    {
                        *numOfSerdesLanesPtr = 8;
                        *startSerdesPtr = 16;
                    }
                }
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortIfModeCheckAndSerdesNumGet function
* @endinternal
*
* @brief   Get number of first serdes and quantity of serdeses occupied by given
*         port in given interface mode
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port media interface mode
*
* @param[out] startSerdesPtr           - first used serdes number
* @param[out] numOfSerdesLanesPtr      - quantity of serdeses occupied
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if port doesn't support given interface mode
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS prvCpssDxChPortIfModeCheckAndSerdesNumGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    OUT GT_U32                          *startSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
)
{
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        GT_PHYSICAL_PORT_NUM localPort; /* number of port in port group */

        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
        if((ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
            ((ifMode == CPSS_PORT_INTERFACE_MODE_XGMII_E) && (localPort%2 != 0)) ||
            ((ifMode == CPSS_PORT_INTERFACE_MODE_HX_E) && (localPort%2 != 0)) ||
            ((ifMode == CPSS_PORT_INTERFACE_MODE_XLG_E) && (localPort != 10)))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChPortIfModeSerdesNumGet(devNum,portNum,ifMode,startSerdesPtr,numOfSerdesLanesPtr);
}

/**
* @internal prvCpssDxChPortNumberOfSerdesLanesGet function
* @endinternal
*
* @brief   Get number of first SERDES and quantity of SERDESes occupied by given
*         port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] firstSerdesPtr           - (pointer to) first used SERDES number
* @param[out] numOfSerdesLanesPtr      - (pointer to) quantity of SERDESes occupied
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - wrong device family
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortNumberOfSerdesLanesGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *firstSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
)
{
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* port media interface mode */
    GT_STATUS                       rc ;    /* return code */

    /* get interface type */
    rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get first SERDES and number of SERDES lanes */
    rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum, portNum, ifMode,
                                                    firstSerdesPtr,
                                                    numOfSerdesLanesPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxCh3SerdesSpeedModeGet function
* @endinternal
*
* @brief   Gets speed mode for specified port serdes on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] serdesSpeed_2500_EnPtr   - pointer to serdes speed mode.
*                                      GT_TRUE  - serdes speed is 2.5 gig.
*                                      GT_FALSE - other serdes speed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxCh3SerdesSpeedModeGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *serdesSpeed_2500_EnPtr
)
{
    GT_U32                regAddr;       /* register address */
    GT_U32                regValue;      /* register value */

    /* Get SERDES speed configuration */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        macRegs.perPortRegs[portNum].serdesSpeed1[0];
    if (prvCpssDrvHwPpPortGroupReadRegister(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            regAddr, &regValue) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *serdesSpeed_2500_EnPtr = (regValue == PRV_SERDES_SPEED_2500_REG_VAL_CNS) ?
                                GT_TRUE : GT_FALSE;
    return GT_OK;
}

/***********internal functions*************************************************/

/**
* @internal prvGetLpSerdesSpeed function
* @endinternal
*
* @brief   Gets LP serdes speed.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - port group Id, support multi-port-groups device
* @param[in] serdesNum                - number of first serdes of configured port
*
* @param[out] serdesSpeedPtr           - (pointer to) serdes speed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvGetLpSerdesSpeed
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               serdesNum,
    OUT CPSS_DXCH_PORT_SERDES_SPEED_ENT  *serdesSpeedPtr
)
{
    GT_U32      regAddr;            /* register address */
    GT_U32      regValue,regValue2; /* registers values */
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesSpeed;    /* serdes frequency */
    GT_STATUS   rc = GT_OK;         /* return code */
    GT_U32 firstInitializedSerdes; /* number of first already configured serdes */
    PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY *serdesPowerUpSequencePtr;
                                    /* pointer to serdes power up sequence */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_ARRAY  *serdesExtCfgPtr; /* pointer to
        external registers serdes power up configuration (just dummy here) */

    *serdesSpeedPtr = CPSS_DXCH_PORT_SERDES_SPEED_NA_E;
    rc = serDesConfig(devNum,&serdesPowerUpSequencePtr,&serdesExtCfgPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChLpGetFirstInitSerdes(devNum,portGroupId,serdesNum,&firstInitializedSerdes);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get PLL/INTP Register2 */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
        serdesConfig[firstInitializedSerdes].pllIntpReg2;
    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get PLL/INTP Register3 */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
        serdesConfig[firstInitializedSerdes].pllIntpReg3;
    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regValue2);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (serdesSpeed=CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;
          serdesSpeed < CPSS_DXCH_PORT_SERDES_SPEED_NA_E; serdesSpeed++)
    {
        if ((regValue == serdesPowerUpSequencePtr
                            [PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG2_INDEX_CNS]
                            [serdesSpeed]) &&
            ((regValue2 & 0xFFF)  == (serdesPowerUpSequencePtr
                            [PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG3_INDEX_CNS]
                            [serdesSpeed] & 0xFFF)))
            break;
    }

    *serdesSpeedPtr = serdesSpeed;

    return rc;
}

/**
* @internal prvCpssDxCh3PortSerdesPowerUpDownGet function
* @endinternal
*
* @brief   Get power up or down state to port and serdes.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] powerUpPtr               - (pointer to) power up state.
*                                      GT_TRUE  - power up
*                                      GT_FALSE - power down
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxCh3PortSerdesPowerUpDownGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *powerUpPtr
)
{
    GT_U32      regAddr, regAddr2;       /* register address */
    GT_U32      regData;                 /* register data    */
    GT_U32      regMask;                 /* register mask    */
    GT_U32      i;                       /* iterator */
    GT_U32      startSerdes; /* first SERDES number */
    GT_U32      serdesesNum; /* number of SERDESs in a port  */
    GT_STATUS   rc;         /* return code */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;  /* current port interface mode */

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
    {
        /* Get current state */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    macRegs.perPortRegs[portNum].serdesPowerUp1[0];
        regAddr2 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    macRegs.perPortRegs[portNum].serdesPowerUp2[0];
        /* SP or XG*/

        if( PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType == PRV_CPSS_PORT_XG_E )
        {
            regMask = 0xFF;
        }
        else /* Giga port */
        {
            regMask = 0x11 << (portNum%4) ;
        }

        rc = prvCpssDrvHwPpPortGroupReadRegBitMask(devNum, portGroupId,regAddr, regMask, &regData);
        if(rc != GT_OK)
        {
            return rc;
        }

        *powerUpPtr = GT_FALSE;
        if( regData == regMask )
        {
            rc = prvCpssDrvHwPpPortGroupReadRegBitMask(devNum, portGroupId,regAddr2, regMask, &regData);
            if(rc != GT_OK)
            {
                return rc;
            }

            if( regData == regMask )
            {
                *powerUpPtr = GT_TRUE;
            }
        }
    }
    else
    {   /* xCat and above */
        if((rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode)) != GT_OK)
            return rc;

        if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            /* SERDES was not initialized yet */
            *powerUpPtr = GT_FALSE;
            return GT_OK;
        }

        if((rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum, portNum, ifMode,
                                                           &startSerdes,
                                                           &serdesesNum)) != GT_OK)
        {
            return rc;
        }

        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
        {
            GT_BOOL txEnable;

            *powerUpPtr = GT_TRUE;

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesTxEnableGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *enablePtr)", devNum, portGroupId, startSerdes, HWS_DEV_SERDES_TYPE(devNum));
            rc = mvHwsSerdesTxEnableGet(devNum, portGroupId, startSerdes,
                                        HWS_DEV_SERDES_TYPE(devNum),&txEnable);

            if (rc == GT_NOT_INITIALIZED)
            {/* SERDES is down */
                *powerUpPtr = GT_FALSE;
                return GT_OK;
            }
            else
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
        }

        /* check if all serdes in port are initialized */
        for (i = startSerdes; i < startSerdes + serdesesNum;i++)
        {
            rc = prvCpssDxChLpCheckSerdesInitStatus(devNum, portGroupId, i);
            if ( (rc != GT_OK) && (rc != GT_NOT_INITIALIZED))
            {
                return rc;
            }
            if (rc == GT_NOT_INITIALIZED)
            {
                /* SERDES was not initialized yet */
                *powerUpPtr = GT_FALSE;
                return GT_OK;
            }
        }

        /* check Power state of SERDES */
        for (i = startSerdes; i < startSerdes + serdesesNum;i++)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                serdesConfig[i].powerReg;
            rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,regAddr, &regData);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* check that at least one SERDES is UP */
            if (regData != 0)
            {
                /* SERDES is UP */
                *powerUpPtr = GT_TRUE;
                return GT_OK;
            }
        }

        /* SERDES is DOWN */
        *powerUpPtr = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortLionSerdesSpeedSet function
* @endinternal
*
* @brief   Configure Lion SerDes to specific frequency
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] serdesSpeed              - speed on which serdes mist work
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortLionSerdesSpeedSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesSpeed
)
{
    GT_U32 portGroupId; /* port group number */
    GT_U32 regAddr;     /* register address */
    GT_U32 regValue;    /* register value */
    GT_U32 regMask;     /* mask for register data */
    GT_U32 i;           /* iterator */
    GT_PHYSICAL_PORT_NUM localPort; /* number of port in port group */
    GT_PHYSICAL_PORT_NUM globalPort;/* number of port in device */
    GT_U32 startSerdes = 0; /* number of first serdes used by port */
    GT_U32 serdesesNum = 0; /* number of serdeses used by port */
    GT_STATUS rc;       /* return code */
    GT_U32 extCfgIdx = 1;   /* column to use from external registers serdes
                                power up values array */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;    /* current interface mode of port */
    CPSS_DXCH_PORT_SERDES_SPEED_ENT  serdesSpeedPrev;   /* old speed
                                                 configured on ports serdeses */
    PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY    *serdesPowerUpSequencePtr;
                    /* ptr to serdes power up sequence array */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_ARRAY     *serdesExtCfgPtr;
                    /* ptr to serdes power up sequence external registers array */

    serdesPowerUpSequencePtr = lionSerdesPowerUpSequence;
    serdesExtCfgPtr = lpSerdesExtConfig_Lion_B0_156Clk_LR;

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    if((rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode)) != GT_OK)
        return rc;

    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,&startSerdes,&serdesesNum)) != GT_OK)
        return rc;

    /* if serdeses already configured as needed exit and if their configuration not compatible error */
    if((ifMode == CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E) && ((portNum%2) != 0))
    {
        if(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portNum-1) == CPSS_PORT_INTERFACE_MODE_XGMII_E)
        {
            rc = prvGetLpSerdesSpeed(devNum,portGroupId,startSerdes-2,&serdesSpeedPrev);
            if (rc != GT_OK)
            {
                return rc;
            }
            if(serdesSpeedPrev != serdesSpeed)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            else
                goto unreset_serdes_and_xpcs; /* needed speed already configured */
        }
    }

    PRV_PER_PORT_LOOP_MAC
    {/* Disable port and assert port reset */
        globalPort = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,
                                                        portGroupId,localPort);
        /* reset mac and XPCS/40GPCS */
        rc = prvCpssDxChPortXgPcsResetStateSet(devNum,globalPort,GT_TRUE);
        if(rc != GT_OK)
            return rc;

        rc = prvCpssDxChPortMacResetStateSet(devNum,globalPort,GT_TRUE);
        if(rc != GT_OK)
            return rc;
    }

    /* although serdes reset de-asserted immediately in prvCpssDxChLpPreInitSerdesSequence
     * to get access to serdes registers, must reset them here otherwise link won't
     * be reastablished for XGMII */
    rc = prvCpssDxChPortSerdesResetStateSet(devNum,portNum,startSerdes,serdesesNum,GT_TRUE);
    if (rc != GT_OK)
        return rc;

    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,20);

    if((serdesSpeed == CPSS_DXCH_PORT_SERDES_SPEED_1_25_E)
            || (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) == PRV_CPSS_PORT_GE_E))
    {/* Set mode to 10bit <SDSel10bitMode> */
        SERDES_DBG_DUMP_MAC(("extCfgIdx = 0;\n"));
        extCfgIdx = 0;
    }

    /* Pre Configuration for all serdeses, */
    /* enables access to serdes registers */
    PRV_PER_SERDES_LOOP_MAC
    {
        rc = prvCpssDxChLpPreInitSerdesSequence(devNum, portGroupId, i,
                serdesExtCfgPtr[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG1_E][extCfgIdx]);
        if (rc != GT_OK)
            return rc;
    }

    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,20);

    /* Configuration */
    rc = prvCpssDxChPortGenSerDesConfig(devNum,portGroupId,startSerdes,serdesesNum,
                                        serdesSpeed,serdesPowerUpSequencePtr);
    if (rc != GT_OK)
        return rc;

    /*--- End of SerDes Configuration --- */

unreset_serdes_and_xpcs:

    regMask = FIELD_MASK_NOT_MAC(11,1);

    PRV_PER_SERDES_LOOP_MAC
    {
        /* Set RxInit to 0x1 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].serdesExternalReg2;
        regValue = serdesExtCfgPtr[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_2_E][extCfgIdx];
        rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum, portGroupId,regAddr,regMask,regValue);
        if (rc != GT_OK)
            return rc;
        SERDES_DBG_DUMP_MAC(("regAddr=0x%08x,regValue=0x%08x\n", regAddr, regValue));
    }

    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,10);

    PRV_PER_SERDES_LOOP_MAC
    {
        /* Set RxInit to 0x0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].serdesExternalReg2;
        regValue = serdesExtCfgPtr[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_3_E][extCfgIdx];
        rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum, portGroupId,regAddr,regMask,regValue);
        if (rc != GT_OK)
            return rc;
        SERDES_DBG_DUMP_MAC(("regAddr=0x%08x,regValue=0x%08x\n", regAddr, regValue));
    }

    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,10);

    rc = prvCpssDxChPortSerdesResetStateSet(devNum,portNum,startSerdes,serdesesNum,GT_FALSE);
    if (rc != GT_OK)
        return rc;

    PRV_PER_PORT_LOOP_MAC
    {
        /* De assert PCS Reset */
        globalPort = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,portGroupId,localPort);
        if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,globalPort) == PRV_CPSS_PORT_XG_E)
        {
            rc = prvCpssDxChPortXgPcsResetStateSet(devNum,globalPort,GT_FALSE);
            if(rc != GT_OK)
                return rc;
        }
    }

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) == PRV_CPSS_PORT_XLG_E)
    {
        /* De assert 40GPCS reset */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->xlgRegs.pcs40GRegs.commonCtrl;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,9,1,0);
        if (rc != GT_OK)
            return rc;
        SERDES_DBG_DUMP_MAC(("regAddr=0x%08x, bit 9 to 0\n", regAddr));

        HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,10);
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPortLionSerdesPowerUpSet function
* @endinternal
*
* @brief   Set power up state to Lion port and serdes.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortLionSerdesPowerUpSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_U32      portGroupId;    /* port group number */
    GT_U32      regAddr;        /* register address */
    GT_STATUS   rc;             /* return code */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* current mac unit used by port */

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum);

    /* De-assert port reset - just for really working port */
    if(portMacType > PRV_CPSS_PORT_GE_E)
    { /* if not sgmii or 1000baseX */
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portNum,portMacType,&regAddr);
        if((rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                    1, 1, 1)) != GT_OK)
            return rc;
        SERDES_DBG_DUMP_MAC(("regAddr=0x%08x\n", regAddr));
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portNum,portMacType,&regAddr);
        if((rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                    6, 1, 0)) != GT_OK)
            return rc;
        SERDES_DBG_DUMP_MAC(("regAddr=0x%08x\n", regAddr));
    }

    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,20);

    if((rc = cpssDxChPortSerdesResetStateSet(devNum, portNum, GT_FALSE)) != GT_OK)
        return rc;

    return GT_OK;
}

/**
* @internal cheetah1_2_portSerdesPowerStatusSet function
* @endinternal
*
* @brief   Sets power state of SERDES port lanes according to port capabilities.
*         XG / XAUI ports: All 4 (TX or RX) lanes are set simultanuously.
*         HX / QX ports: lanes 0,1 or TX and RX may be set separately.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - may be either CPSS_PORT_DIRECTION_RX_E,
*                                      CPSS_PORT_DIRECTION_TX_E, or
*                                      CPSS_PORT_DIRECTION_BOTH_E.
* @param[in] lanesBmp                 - bitmap of SERDES lanes (bit 0-> lane 0, etc.)
* @param[in] powerUp                  - GT_TRUE  = power up, GT_FALSE = power down
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS cheetah1_2_portSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
)
{
    GT_STATUS       status;     /* return code */
    GT_U32          regAddr;    /* register address */
    GT_U32          regAddrReset;   /* register address for reset */
    GT_U32          data;       /* 32 bit register value */
    GT_U16          data16;     /* 16 bit register value */
    GT_U8           i;          /* iterator */
    GT_U32          pwrDn = (powerUp == GT_TRUE) ? 0:1; /* required serdes power state */
    GT_U32          maskRst = 0;    /* bitmap of serdes where reset required */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;  /* corrent port interface mode */

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portNum) < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    status = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
    if(status != GT_OK)
        return status;

    /* select the proper sequence */
    switch (ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
            /* A group of 4 lanes is controlled by the same bit */
            if (lanesBmp == 0xf)
            {
                regAddr = 0x8000;
                status = cpssDxChPhyPort10GSmiRegisterRead(devNum, portNum, 0, GT_FALSE,
                                                           (GT_U16)regAddr, 5, &data16);
                if ( status != GT_OK)
                    return status;

                /* to ensure correct assignment in BE mode */
                data = ((GT_U32)data16) & 0xFF;
                switch(direction)
                {
                    case CPSS_PORT_DIRECTION_RX_E:
                        U32_SET_FIELD_MAC(data, 7 , 1, pwrDn); /* RX */
                        break;
                    case CPSS_PORT_DIRECTION_BOTH_E:
                        U32_SET_FIELD_MAC(data, 7 , 1, pwrDn); /* RX */
                        U32_SET_FIELD_MAC(data, 11, 1, pwrDn); /* TX */
                        break;
                    case CPSS_PORT_DIRECTION_TX_E:
                        U32_SET_FIELD_MAC(data, 11, 1, pwrDn); /* TX */
                        break;
                    default:
                        status = GT_BAD_PARAM;
                        return status;
                }
                /* update register */
                data16 = (GT_U16)data;
                status = cpssDxChPhyPort10GSmiRegisterWrite(devNum, portNum, 0, GT_FALSE,
                                                            (GT_U16)regAddr, 5, data16);
            }
            else if (lanesBmp == 0)
            {
                /* no lane affacted */
                status = GT_OK;
            }
            else
            {
                /* HW cannot control lanes individually */
                status = GT_NOT_SUPPORTED;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_QX_E:
        case CPSS_PORT_INTERFACE_MODE_HX_E:

            /* Controlling lanes individually */
            if (lanesBmp & (~3))
            {
                /* invalid arguments */
                status = GT_BAD_PARAM;
            }
            else if (lanesBmp == 0)
            {
                /* no lane affacted */
                status = GT_OK;
            }
            else
            {
                /* Read HXPort25 or HXPort26 register, port must be >= 25 */
                if((25 != portNum) && (26 != portNum))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                regAddrReset = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        macRegs.perPortRegs->hxPortConfig0[portNum-25];
                regAddr = regAddrReset + 0x60;
                status = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, &data);
                if (status != GT_OK)
                    return status;

                /* Bit manipulation for lanes */
                for (i = 0; i < 2; ++i)
                {
                    if (lanesBmp & (1 << i) )
                    {
                        switch (direction)
                        {
                            case CPSS_PORT_DIRECTION_RX_E:
                                U32_SET_FIELD_MAC( data, i, 1, pwrDn);  /* RX */
                                break;
                            case CPSS_PORT_DIRECTION_BOTH_E:
                                U32_SET_FIELD_MAC( data, i, 1, pwrDn);  /* RX */
                                U32_SET_FIELD_MAC( data, i + 2, 1, pwrDn);  /* TX */
                                break;
                            case CPSS_PORT_DIRECTION_TX_E:
                                U32_SET_FIELD_MAC( data, i + 2, 1, pwrDn);  /* TX */
                                break;
                            default:
                                status = GT_BAD_PARAM;
                                return status;
                        }
                    }
                    /* Reset lane if power down both directions, else put it out of reset */
                    if ((data & (0x5 << i)) == (0x5ul << i))
                    {
                        data |= (0x10 << i);
                    }
                    else
                    {
                        data &= ~(0x10 << i);
                        maskRst |= (0x10 << i);
                    }
                }
                /* Power up/down sequences */
                if (GT_FALSE == powerUp)
                {
                    /* power down affected lanes and put them in reset state */
                    status = prvCpssDrvHwPpPortGroupWriteRegister(devNum,
                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, data);
                    if ( status != GT_OK)
                        return status;

                    /* Put port into reset state if all lanes are powered down */
                    if ( (data & 0xf) == 0xf)
                        status = prvCpssDrvHwPpPortGroupSetRegField(devNum,
                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddrReset, 11 , 1, 1);
                }
                else
                {
                    /* Power UP: put the port out of reset, or verify it is out of reset */
                    status = prvCpssDrvHwPpPortGroupSetRegField(devNum,
                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddrReset, 11 , 1, 0);
                    if ( status != GT_OK)
                        return status;

                    /* Force reset on lanes that are being powered up */
                    status = prvCpssDrvHwPpPortGroupWriteRegister(devNum,
                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, data | maskRst);
                    if ( status != GT_OK)
                        return status;

                    /* apply the new lanes state */
                    status = prvCpssDrvHwPpPortGroupWriteRegister(devNum,
                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, data);
                }
            }
            break;
        default:
            status = GT_FAIL;
            break;
    }

    return status;
}

/**
* @internal prvCpssDxChXcatPortSerdesPowerStatusSet function
* @endinternal
*
* @brief   Set power up or down state to port and serdes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - TX/RX/BOTH serdes  (ignored)
* @param[in] lanesBmp                 - lanes bitmap         (ignored)
* @param[in] powerUp                  - power up state:
*                                      GT_TRUE  - power up
*                                      GT_FALSE - power down
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChXcatPortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value   */
    GT_U32      i;          /* loop index       */
    GT_STATUS   rc;         /* return status    */
    GT_U32      startSerdes = 0, /* first serdes of port */
                serdesesNum = 0; /* number of ports serdeses */
    GT_U32      serdesExternalRegVal;   /* value of external register */
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesSpeed;    /* current serdes speed */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;  /* current ports interface mode */
    PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY    *serdesPowerUpSequencePtr;
                /* ptr to serdes power up sequence array */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_ARRAY     *serdesExtCfgPtr;
                /* ptr to serdes external registers configuration array */
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */

    lanesBmp = lanesBmp; /* to avoid warning */

    if(direction != CPSS_PORT_DIRECTION_BOTH_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        serdesPowerUpSequencePtr = (PRV_CPSS_PP_MAC(devNum)->revision > 2)
                        ? xcatSerdesPowerUpSequence : xcatA1SerdesPowerUpSequence;
        serdesExtCfgPtr = lpSerdesExtConfig;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        serdesPowerUpSequencePtr = xcat2SerdesPowerUpSequence;
        serdesExtCfgPtr = lpSerdesExtConfig_xCat2_125Clk;
    }
    else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if((rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode)) != GT_OK)
        return rc;

    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,
                                            &startSerdes,&serdesesNum)) != GT_OK)
        return rc;

    serdesExternalRegVal =
            serdesExtCfgPtr[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG1_E][1];
    if (PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portNum) == GT_TRUE)
    {
        if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) == PRV_CPSS_PORT_GE_E)
        {
            serdesExternalRegVal =
                serdesExtCfgPtr[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG1_E][0];
            SERDES_DBG_DUMP_MAC(("prvCpssDxChXcatPortSerdesPowerStatusSet:Set \
                                serdesExternalRegVal for SGMII=0x%x;portNum=%d\n",
                                serdesExternalRegVal, portNum));
        }
    }

    /* check if all serdes in port are initialized and make proper initialization
       if it is needed */
    rc = prvCpssDxChLpCheckAndInitNeededSerdes(devNum,
                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,startSerdes,serdesesNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    if(powerUp == GT_FALSE)
    {/* no need to configure power register for up, because then we assert serdes reset
      * and this register receives appropriate value automatically
      * NOTE: serdes power down not necessary causes link down - the only sign that it's
      *         down is - no traffic
      */
        PRV_PER_SERDES_LOOP_MAC
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].powerReg;
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,
                                CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
            SERDES_DBG_DUMP_MAC(("regAddr=0x%08x,regVal==0x%08x\n", regAddr, 0));
            HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,10);
        }
        return GT_OK;
    }

    if (PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portNum) == GT_TRUE)
    {
        rc = prvGetLpSerdesSpeed(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                    startSerdes, &serdesSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* can't determine serdes speed */
        if (serdesSpeed == CPSS_DXCH_PORT_SERDES_SPEED_NA_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    else /* NP */
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.extendedGlobalControl;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, 15, 6, &regValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(regValue & (1<<(portNum/4)))
        {/* sgmii 1G works on 1.25, 2.5G on 3.125 */
            rc = prvGetLpSerdesSpeed(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        startSerdes,&serdesSpeed);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* can't determine serdes speed */
            if (serdesSpeed == CPSS_DXCH_PORT_SERDES_SPEED_NA_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* for 2,5G port speed with 3,125G serdes speed use here value from 1.25 */
            serdesExternalRegVal = serdesExtCfgPtr[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG1_E][0];
        }
        else /* qsgmii */
        {
            serdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_5_E;
        }
    }

    /* Pre Configuration */
    PRV_PER_SERDES_LOOP_MAC
    {
        SERDES_DBG_DUMP_MAC(("prvCpssDxChXcatPortSerdesPowerStatusSet:\
                            serdesExternalRegVal=0x%x;portNum=%d,serdes=%d\n",
                            serdesExternalRegVal, portNum, i));
        rc = prvCpssDxChLpPreInitSerdesSequence(devNum,
                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,i,serdesExternalRegVal);
        if (rc != GT_OK)
            return rc;
    }
    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,5);

    /* Configuration */
    rc = prvCpssDxChPortGenSerDesConfig(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        startSerdes,serdesesNum,
                                        serdesSpeed,serdesPowerUpSequencePtr);
    if (rc != GT_OK)
        return rc;

    /* Post Configuration */
    PRV_PER_SERDES_LOOP_MAC
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].serdesExternalReg2;

        regValue = serdesExtCfgPtr[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_2_E][1];
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,
                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, regValue);
        if (rc != GT_OK)
            return rc;

        regValue = serdesExtCfgPtr[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_3_E][1];
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,
                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, regValue);
        if (rc != GT_OK)
            return rc;

        regValue = serdesExtCfgPtr[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_4_E][1];
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,
                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, regValue);
        if (rc != GT_OK)
            return rc;
    }

    /* unset xpcs and mac reset */
    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) >= PRV_CPSS_PORT_XG_E)
    {
        rc = prvCpssDxChPortXgPcsResetStateSet(devNum, portNum, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChPortMacResetStateSet(devNum,portNum,GT_FALSE);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal lionPortSerdesPowerStatusSet function
* @endinternal
*
* @brief   Set power up or down state to port and serdes.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - TX/RX/BOTH serdes configuration (ignored)
* @param[in] lanesBmp                 - lanes bitmap         (ignored)
* @param[in] powerUp                  - power up state:
*                                      GT_TRUE  - power up
*                                      GT_FALSE - power down
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
static GT_STATUS lionPortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      i;          /* loop index       */
    GT_STATUS   rc;         /* return status    */
    GT_U32      portGroupId;     /*the port group Id - support multi-port-groups device */
    GT_U32      startSerdes = 0, /* first serdes to configure */
                serdesesNum = 0; /* number of serdes used by port */
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesSpeed;    /* required serdes frequency */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;  /* current port interface mode */

    lanesBmp = lanesBmp; /* to avoid warning */

    if(direction != CPSS_PORT_DIRECTION_BOTH_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.serdesRefClock !=
                    CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if((rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode)) != GT_OK)
        return rc;

    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,
                                        &startSerdes,&serdesesNum)) != GT_OK)
        return rc;

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    /* check if all serdes in port are initialized and make proper initialization
       if it is needed */
    rc = prvCpssDxChLpCheckAndInitNeededSerdes(devNum,portGroupId,startSerdes,serdesesNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(powerUp == GT_FALSE)
    {/* no need to configure power register for up, because then we assert serdes reset
      * and this register receives appropriate value automatically
      * NOTE: serdes power down not necessary causes link down - the only sign that it's
      *         down is - no traffic
      */
        PRV_PER_SERDES_LOOP_MAC
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].powerReg;
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
            SERDES_DBG_DUMP_MAC(("regAddr=0x%08x,regVal==0x%08x\n", regAddr, 0));
            HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,10);
        }
        return GT_OK;
    }

    rc = prvGetLpSerdesSpeed(devNum, portGroupId, startSerdes, &serdesSpeed);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* can't determine serdes speed */
    if (serdesSpeed == CPSS_DXCH_PORT_SERDES_SPEED_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortLionSerdesSpeedSet(devNum,portNum,serdesSpeed);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortLionSerdesPowerUpSet(devNum, portNum);

    return rc;
}

/**
* @internal lion2PortSerdesTuning function
* @endinternal
*
* @brief   Configure serdes tuning values.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - core/port group number
* @param[in] sdVectorPtr              - array of numbers of serdeses occupied by port
* @param[in] sdVecSize                - size of serdeses array
* @param[in] ifMode                   - interface to configure on port
* @param[in] speed                    -  to configure on port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS lion2PortSerdesTuning
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U16                  *sdVectorPtr,
    IN  GT_U8                   sdVecSize,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT     speed
)
{
    GT_STATUS                       rc;             /* return code */
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesSpeed;    /* required serdes frequency */
    CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr;  /* ptr to specific entry in tuning array */
    GT_U32                          firstSerdesInCore; /* flat number of first serdes in port group */
    GT_U32                          i;              /* iterator */
    MV_HWS_AUTO_TUNE_RESULTS        tunParams;      /* tuning parameters in HWS format */

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr != NULL)
    {/* if serdes fine tuning values DB initialized */
        serdesSpeed = serdesFrequency[ifMode][speed];
        firstSerdesInCore = portGroup*PRV_CPSS_LION2_SERDES_NUM_CNS;
        for(i = 0; i < sdVecSize; i++)
        {
            if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[firstSerdesInCore+sdVectorPtr[i]])
            {/* if fine tuning values not defined for current serdes skip */
                continue;
            }

            tuneValuesPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr
                                    [firstSerdesInCore+sdVectorPtr[i]][serdesSpeed];

            /* RX tuning params */
            tunParams.sqleuch = tuneValuesPtr->sqlch;
            tunParams.ffeR = tuneValuesPtr->ffeR;
            tunParams.ffeC = tuneValuesPtr->ffeC;
            tunParams.align90 = tuneValuesPtr->align90;

            /* TX tuning params */
            tunParams.txAmp = tuneValuesPtr->txAmp;
            tunParams.txEmph0 = tuneValuesPtr->txEmphAmp;
            tunParams.txEmph1 = tuneValuesPtr->txEmph1;

            tunParams.sampler = tuneValuesPtr->sampler;
            cpssOsMemCpy(tunParams.dfeVals, tuneValuesPtr->dfeValsArray, sizeof(GT_32)*6);

            /* relevant for B0 and above */
            tunParams.txAmpAdj  = tuneValuesPtr->txAmpAdj;
            tunParams.txAmpShft = tuneValuesPtr->txAmpShft;
            tunParams.txEmph1En = tuneValuesPtr->txEmphEn1;
            tunParams.txEmph0En = tuneValuesPtr->txEmphEn;

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesManualRxConfig(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], portTuningMode[%d], sqlch[%d], ffeRes[%d], ffeCap[%d], dfeEn[%d], alig[%d])", devNum, portGroup, sdVectorPtr[i], HWS_DEV_SERDES_TYPE(devNum), ((CPSS_PORT_INTERFACE_MODE_KR_E == ifMode) ? StaticLongReach : StaticShortReach), tunParams.sqleuch, tunParams.ffeR, tunParams.ffeC, GT_TRUE /* dfeEn */, tunParams.align90);
            rc = mvHwsSerdesManualRxConfig(devNum, portGroup, sdVectorPtr[i],
                                            HWS_DEV_SERDES_TYPE(devNum),
                                            ((CPSS_PORT_INTERFACE_MODE_KR_E == ifMode ||
                                              (CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode && speed == CPSS_PORT_SPEED_40000_E)) ?
                                             StaticLongReach : StaticShortReach),
                                            tunParams.sqleuch,
                                            tunParams.ffeR,
                                            tunParams.ffeC,
                                            GT_TRUE /* dfeEn */,
                                            tunParams.align90);
            if(rc != GT_OK)
            {
                return rc;
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesManualTxConfig(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], txAmp[%d], txAmpAdj[%d], emph0[%d], emph1[%d], txAmpShft[%d])", devNum, portGroup, sdVectorPtr[i], HWS_DEV_SERDES_TYPE(devNum), tunParams.txAmp, tunParams.txAmpAdj, tunParams.txEmph0, tunParams.txEmph1, tunParams.txAmpShft);
            rc = mvHwsSerdesManualTxConfig(devNum, portGroup, sdVectorPtr[i],
                                           HWS_DEV_SERDES_TYPE(devNum),
                                           tunParams.txAmp,
                                           tunParams.txAmpAdj,
                                           tunParams.txEmph0,
                                           tunParams.txEmph1,
                                           tunParams.txAmpShft, tunParams.txEmph0En);

            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChBobkPortSerdesTuning function
* @endinternal
*
* @brief   Configure serdes tuning values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - core/port group number
*                                      sdVectorPtr - array of numbers of serdeses occupied by port
*                                      sdVecSize   - size of serdeses array
*                                      ifMode      - interface to configure on port
*                                      speed       - speed to configure on port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChBobkPortSerdesTuning
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  MV_HWS_PORT_STANDARD    portMode
)
{
    GT_STATUS                       rc;             /* return code */
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesSpeed;    /* required serdes frequency */
    CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr;  /* ptr to specific entry in tuning array */
    GT_U32                          i;              /* iterator */
    GT_U16                          *sdVectorPtr;
    GT_U8                           sdVecSize;
    GT_U32                          portMacMap;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;

    if(cpssDeviceRunCheck_onEmulator() )
    {
        return GT_OK;
    }
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr != NULL)
    {/* if serdes fine tuning values DB initialized */

        PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                                portNum,
                                                                                portMacMap);

        if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portMacMap, portMode, &curPortParams))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
        }
        sdVecSize   = curPortParams.numOfActLanes;
        sdVectorPtr = curPortParams.activeLanesList;

        /* convert HWS serdes speed to CPSS serdes speed enumerator */
        PRV_CPSS_DXCH_PORT_SERDES_SPEED_HW_TO_SW_CONVERT_MAC(serdesSpeed, curPortParams.serdesSpeed);

        for(i = 0; i < sdVecSize; i++)
        {
            if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[sdVectorPtr[i]])
            {/* if fine tuning values not defined for current serdes skip */
                continue;
            }


            /*cpssOsPrintf("\n**  prvCpssDxChBobkPortSerdesTuning VECTOR ALL NOT NULL \n");*/

            tuneValuesPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[sdVectorPtr[i]][serdesSpeed];

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsAvagoSerdesManualCtleConfig(devNum[%d], portGroup[%d], serdesNum[%d], dcGain[%d], lowFrequency[%d], highFrequency[%d], bandWidth[%d], loopBandwidth[%d], squelch[%d])",
                devNum, portGroup, sdVectorPtr[i], tuneValuesPtr->DC, tuneValuesPtr->LF, tuneValuesPtr->HF, tuneValuesPtr->BW,
                tuneValuesPtr->LB, tuneValuesPtr->sqlch);
            rc = mvHwsAvagoSerdesManualCtleConfig(devNum,  portGroup, sdVectorPtr[i],
                                            tuneValuesPtr->DC,
                                            tuneValuesPtr->LF,
                                            tuneValuesPtr->HF,
                                            tuneValuesPtr->BW,
                                            tuneValuesPtr->LB,
                                            tuneValuesPtr->sqlch);
            /*cpssOsPrintf("\n** CPSS setting ctle: DC %d, LF %d, HF %d, BW %d,LB %d, sqlch %d **\n",tuneValuesPtr->DC,
                                            tuneValuesPtr->LF,
                                            tuneValuesPtr->HF,
                                            tuneValuesPtr->BW,
                                            tuneValuesPtr->LB,
                                            tuneValuesPtr->sqlch);*/
            if(rc != GT_OK)
            {
                return rc;
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesManualTxConfig(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], txAmp[%d], txAmpAdj[%d], emph0[%d], emph1[%d], txAmpShft[%d])",
                devNum, portGroup, sdVectorPtr[i], HWS_DEV_SERDES_TYPE(devNum), tuneValuesPtr->txAmp, tuneValuesPtr->txAmpAdj,
                tuneValuesPtr->txEmphAmp, tuneValuesPtr->txEmph1, tuneValuesPtr->txAmpShft);
            rc = mvHwsSerdesManualTxConfig(devNum, portGroup, sdVectorPtr[i],
                                           HWS_DEV_SERDES_TYPE(devNum),
                                           tuneValuesPtr->txAmp,
                                           tuneValuesPtr->txAmpAdj,
                                           tuneValuesPtr->txEmphAmp,
                                           tuneValuesPtr->txEmph1,
                                           tuneValuesPtr->txAmpShft, GT_TRUE);

            /*cpssOsPrintf("\n** CPSS setting tx: txAmpAdj %d, txEmphAmp %d, txEmph1 %d, txAmpShft %d **\n",tuneValuesPtr->txAmp,
                                           tuneValuesPtr->txAmpAdj,
                                           tuneValuesPtr->txEmphAmp,
                                           tuneValuesPtr->txEmph1,
                                           tuneValuesPtr->txAmpShft);*/

            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
        }
    }


    /*cpssOsPrintf("\n** END prvCpssDxChBobkPortSerdesTuning **\n");*/
    return GT_OK;
}

/**
* @internal prvCpssDxChPortSerdesPolaritySet function
* @endinternal
*
* @brief   Configure the Polarity values on Serdeses if SW DB values initialized.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - core/port group number
* @param[in] sdVectorPtr              - array of numbers of serdeses occupied by port
* @param[in] sdVecSize                - size of serdeses array
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortSerdesPolaritySet
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroup,
    IN  GT_U16      *sdVectorPtr,
    IN  GT_U8       sdVecSize
)
{
    GT_STATUS   rc;
    GT_U32      i;
    PRV_DXCH_PORT_SERDES_POLARITY_CONFIG_STC   polarityValues;

    /* only for xCat3 + Bobk device and above */
    if ((!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) && ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(sdVectorPtr);

    if(cpssDeviceRunCheck_onEmulator())
    {
        return GT_OK;
    }

    /* if serdes Polarity SW DB values initialized */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr != NULL)
    {
        for(i=0; i < sdVecSize; i++)
        {
            polarityValues = PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr[sdVectorPtr[i]];

            /* skip serdes if both Tx & Rx polarity values not defined */
            if ((polarityValues.txPolarity == GT_FALSE) && (polarityValues.rxPolarity == GT_FALSE))
            {
                continue;
            }

            CPSS_LOG_INFORMATION_MAC("Calling: cpssDxChPortSerdesLanePolaritySet(devNum[%d], portGroup[%d], serdesNum[%d], invertTx[%d], invertRx[%d])",
                                     devNum, portGroup, sdVectorPtr[i], polarityValues.txPolarity, polarityValues.rxPolarity);

            rc = mvHwsSerdesPolarityConfig(devNum, portGroup, sdVectorPtr[i], HWS_DEV_SERDES_TYPE(devNum), polarityValues.txPolarity, polarityValues.rxPolarity);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPortSerdesPolaritySet: error in mvHwsSerdesPolarityConfig\n");
            }
        }
    }

    return GT_OK;
}

GT_STATUS lion2PortInfoGet
(
    IN   GT_U8                   devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  prvCpssDxChLion2PortInfo_STC * portInfoPtr
)
{
    GT_STATUS rc;
    GT_U32 localPort;
    GT_U32 regAddr;
    GT_U32 portGroup;
    GT_U32 i;


    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
    portGroup = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.rxdmaIfConfig[localPort/8];
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroup, regAddr,
                                         (localPort%8)*4+3, 1, &portInfoPtr->rxDmaSource);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* i/f width 64 or 256 bits */
    /* fieldData = (CPSS_PORT_SPEED_40000_E == speed) ? 0x2 : 0x0; */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.rxdmaIfConfig[localPort/8];
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroup, regAddr,
                                            (localPort%8)*4, 3, &portInfoPtr->rxDmaIfWidth);
    if(rc != GT_OK)
    {
        return rc;
    }


    for(i = 0; i < 2; i++)
    {/* MPPM0/1 - XLG(40G) Mode */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->mppm[i].xlgMode;
        if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            /* support 1 MPPM instead of 2 (bobcat2) */
            continue;
        }
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroup, regAddr,
                                             localPort, 1, &portInfoPtr->mppmXlgMode[i]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.thresholdOverrideEnable[localPort/6];
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroup, regAddr,
                                         ((localPort%6)*5+4), 1, &portInfoPtr->txDmaThhresholdOverrideEnable);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal lion2PortSerdesPowerStatusSet function
* @endinternal
*
* @brief   Set power up or down state to port and serdes.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - TX/RX/BOTH serdes configuration (ignored)
* @param[in] lanesBmp                 - lanes bitmap         (ignored)
* @param[in] powerUp                  - power up state:
*                                      GT_TRUE  - power up
*                                      GT_FALSE - power down
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
static GT_STATUS lion2PortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
)
{
    GT_STATUS                rc;                /* return code */
    GT_BOOL                  originalPortState; /* port enable state defined by application */
    GT_U32                   portGroup;         /* port group number for multi-port-group devs */
    MV_HWS_PORT_STANDARD     portMode, tmpPortMode; /* port interface mode */
    MV_HWS_REF_CLOCK_SUP_VAL refClock;          /* serdes reference clock */
    GT_PHYSICAL_PORT_NUM     localPort;         /* number of port in port group */
    GT_PHYSICAL_PORT_NUM     tmpLocalPort;      /* temporary number of port in port group */
    GT_PHYSICAL_PORT_NUM     tmpGlobalPort;     /* temporary number of port in device */
    GT_U32                   regAddr;           /* register address */
    GT_U32                   fieldData;         /* value to set in register fields */
    CPSS_PORT_SPEED_ENT      speed;             /* required port speed */
    GT_U32                   i;                 /* iterator */
    GT_U32                   macNum, macNumTmp; /* number of MAC used by port for given ifMode */
    GT_U8                    sdVecSize, sdVecSizeTmp; /* size of serdes redundancy array */
    GT_U16                   *sdVectorPtr, *sdVectorPtrTmp; /* serdes redundancy array */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;     /* interface mode to configure */
    GT_BOOL                  lbPort = GT_FALSE;
    GT_BOOL                         apEnable = GT_FALSE;/* is AP enabled on port */
    CPSS_DXCH_PORT_AP_PARAMS_STC    apParams;   /* AP parameters of port */
    GT_U32                          sliceNumUsed; /* number of pizza slices occupied by port */
    CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboParamsPtr;    /* ptr to entry in combo ports description array */
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    MV_HWS_PORT_INIT_PARAMS         tmpPortParams;

    lanesBmp = lanesBmp; /* to avoid warning */

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    if(direction != CPSS_PORT_DIRECTION_BOTH_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portNum);
    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum);
    if((CPSS_PORT_INTERFACE_MODE_HGL_E == ifMode) &&
        ((CPSS_PORT_SPEED_15000_E == speed) ||
         (CPSS_PORT_SPEED_16000_E == speed)))
    {
        originalPortState = GT_TRUE;
    }
    else
    {
        rc = prvCpssDxChPortEnableGet(devNum, portNum, &originalPortState);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(originalPortState != GT_FALSE)
        {
            rc = prvCpssDxChPortEnableSet(devNum,portNum,GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        if((GT_FALSE == powerUp) && (GT_NOT_INITIALIZED == rc))
            return GT_OK; /* port is already down */
        else
            return rc;
    }

    portGroup = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portNum);
    if(GT_FALSE == powerUp)
    {/* if required just power down, then exit here */

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, localPort, portMode, PORT_POWER_DOWN);
        rc = mvHwsPortReset(devNum, portGroup, localPort, portMode,
                            PORT_POWER_DOWN);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }

        /* WA for 40G power down*/
        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                       PRV_CPSS_DXCH_LION2_SHARE_EN_CHANGE_WA_E))
        {
            /* Disable sharing and reset TXDMA counters for next power up*/
            rc = lion2ShareDisableWaExecute(devNum, portGroup, localPort, GT_FALSE);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        rc = prvCpssDxChPortPizzaArbiterIfDelete(devNum,portNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChLion2PortTypeSet(devNum, portNum,
                                            CPSS_PORT_INTERFACE_MODE_NA_E,
                                            CPSS_PORT_SPEED_NA_E);
        if(rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum) =
            CPSS_PORT_INTERFACE_MODE_NA_E;
        PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portNum) = CPSS_PORT_SPEED_NA_E;

        if(originalPortState != GT_FALSE)
        {
            rc = prvCpssDxChPortEnableSet(devNum,portNum,GT_TRUE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }

    /* power down all ports which serdeses occupied by this port */
    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPort, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        return rc;
    }
    macNum      = curPortParams.portMacNumber;
    sdVecSize   = curPortParams.numOfActLanes;
    sdVectorPtr = curPortParams.activeLanesList;

    if(skipLion2PortDelete)
    {
        goto portCreate;
    }
    if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, portGroup, localPort, XLGMAC, RESET);
        rc = mvHwsMacReset(devNum, portGroup, localPort, portMode, XLGMAC, RESET);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, portGroup, localPort, GEMAC_SG, RESET);
        rc = mvHwsMacReset(devNum, portGroup, localPort, portMode, GEMAC_SG, RESET);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
        if(macNum%4==0)
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, portGroup, localPort, HGLMAC, RESET);
            rc = mvHwsMacReset(devNum, portGroup, localPort, portMode, HGLMAC, RESET);
            if (rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
        }
        /* reset just MMPCS which will be used for loopback */
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], physicalPort[%d], pcsType[%d], action[%d])", devNum, portGroup, localPort, MMPCS, RESET);
        rc = mvHwsPcsReset(devNum, portGroup, localPort, _10GBase_KR, MMPCS, RESET);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }

        lbPort = GT_TRUE;
    }

    for(i = 0; (i < sdVecSize) && (!lbPort); i++)
    {
        tmpLocalPort = (sdVectorPtr[i]/6)*4;
        if(sdVectorPtr[i]%6 > 3)
            continue; /* no ports on serdeses reserved for HGL */
        else
            tmpLocalPort += sdVectorPtr[i]%6;

        if(12 == tmpLocalPort)
            tmpLocalPort = 9;
        else if(14 == tmpLocalPort)
            tmpLocalPort = 11;
        else if(11 < tmpLocalPort)
            continue;

        tmpGlobalPort =
            PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,
                                            portGroup, tmpLocalPort);
        if((CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E ==
           PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,tmpGlobalPort))
           || (CPSS_PORT_INTERFACE_MODE_NA_E ==
               PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,tmpGlobalPort)))
        {/* don't touch loopback port, its serdes will be reseted by hwsPortInit;
            don't lose time for port that was not in use already */
            continue;
        }

        if((tmpLocalPort != 9) && (tmpLocalPort != 11))
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _10GBase_SR_LR, PORT_POWER_DOWN);
            /* power down XG MAC */
            rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, _10GBase_SR_LR,
                                PORT_POWER_DOWN);
            if (rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, SGMII, PORT_POWER_DOWN);
            /* power down GE MAC */
            rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, SGMII,
                                PORT_POWER_DOWN);
            if (rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }

            if(tmpLocalPort%2==0)
            {
                rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, tmpLocalPort, RXAUI, &tmpPortParams);
                if (GT_OK != rc)
                {
                    return rc;
                }
                macNumTmp       = tmpPortParams.portMacNumber;
                sdVecSizeTmp    = tmpPortParams.numOfActLanes;
                sdVectorPtrTmp  = tmpPortParams.activeLanesList;

                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], tmpLocalPort[%d], pcsType[%d], action[%d])", devNum, portGroup, tmpLocalPort, XPCS, RESET);
                rc = mvHwsPcsReset(devNum, portGroup, tmpLocalPort, RXAUI, XPCS, RESET);
                if(rc != GT_OK)
                {
                    CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                    return rc;
                }
            }

            /* power down HGL MAC - do it only for every first port in mini-GOP */
            if(tmpLocalPort%4==0)
            {
                if((HGL == portMode) || (HGL16G == portMode))
                {/* power down all units used by HGL */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, portMode, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, portMode,
                                        PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }
                }
                else
                {/* reset just MAC */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, portGroup, tmpLocalPort, HGLMAC, RESET);
                    rc = mvHwsMacReset(devNum, portGroup, tmpLocalPort, portMode, HGLMAC,
                                        RESET);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }
                }
            }

            if(tmpLocalPort != localPort)
            {/* if reseted port is not main configured port then invalidate
                its ifMode */
                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, tmpGlobalPort) =
                                            CPSS_PORT_INTERFACE_MODE_NA_E;
            }
        }
        else if(9 == tmpLocalPort)
        {
            if(localPort != 9)
            {/* other port can occupy port 9 only if it uses regular MAC */
                rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, 9, SGMII, &tmpPortParams);
                if (GT_OK != rc)
                {
                    return rc;
                }
                macNumTmp       = tmpPortParams.portMacNumber;
                sdVecSizeTmp    = tmpPortParams.numOfActLanes;
                sdVectorPtrTmp  = tmpPortParams.activeLanesList;

                if(9 == macNumTmp)
                {
                    /* power down XG MAC */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _10GBase_SR_LR, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, _10GBase_SR_LR,
                                        PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    /* power down GE MAC */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, SGMII, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, SGMII,
                                        PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    /* invalidate ifMode of port 9 only if it really used
                        regular MAC */
                    tmpGlobalPort =
                        PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,
                                                        portGroup, tmpLocalPort);
                    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,tmpGlobalPort) <
                                                            PRV_CPSS_PORT_XLG_E)
                    {
                        PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, tmpGlobalPort) =
                                                CPSS_PORT_INTERFACE_MODE_NA_E;
                    }
                }
            }
            else/* if port 9 is main configured port */
            {
                if(sdVecSize > 2)
                {/* if extended MAC used and occupied all serdeses of
                    mini-GOP #4 */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _40GBase_SR_LR, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort,
                                        _40GBase_SR_LR, PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, HGL, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, HGL,
                                        PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, 6/* xpcsNum of port 9 in XAUI*/, XPCS, RESET);
                    rc = mvHwsPcsReset(devNum, portGroup,
                                        9/* mac 12 (port 9) in XAUI(_10GBase_KX4) should reset PCS #6*/,
                                        _10GBase_KX4, XPCS, RESET);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum-1/*i.e. local port 8*/) >=
                                                            PRV_CPSS_PORT_XLG_E)
                    {/* in this case reset only XG and GE MAC to not harm port 8
                            if it uses serdes of port 9 */
                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, portGroup, tmpLocalPort, XLGMAC, RESET);
                        rc = mvHwsMacReset(devNum, portGroup, tmpLocalPort, portMode,
                                            XLGMAC, RESET);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }

                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, portGroup, tmpLocalPort, GEMAC_SG, RESET);
                        rc = mvHwsMacReset(devNum, portGroup, tmpLocalPort, portMode, GEMAC_SG,
                                            RESET);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }
                    }
                    else
                    {
                        /* power down XG MAC */
                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _10GBase_SR_LR, PORT_POWER_DOWN);
                        rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort,
                                            _10GBase_SR_LR, PORT_POWER_DOWN);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }

                        /* power down GE MAC */
                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, SGMII, PORT_POWER_DOWN);
                        rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort,
                                            SGMII, PORT_POWER_DOWN);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }
                    }
                }
                else if(sdVecSize == 2)
                {/* if extended MAC used but could be port 11 uses its extended
                    MAC too */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _20GBase_SR_LR, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort,
                                        _20GBase_SR_LR, PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, 6/* xpcsNum of port 9 in RXAUI*/, XPCS, RESET);
                    rc = mvHwsPcsReset(devNum, portGroup,
                                        9/* mac 12 (port 9) in RXAUI should reset PCS #6*/,
                                        RXAUI, XPCS, RESET);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum-1/*i.e. local port 8*/) >=
                                                            PRV_CPSS_PORT_XLG_E)
                    {/* in this case reset only XG and GE MAC to not harm port 8
                            if it uses serdes of port 9 */
                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, portGroup, tmpLocalPort, XLGMAC, RESET);
                        rc = mvHwsMacReset(devNum, portGroup, tmpLocalPort, portMode,
                                            XLGMAC, RESET);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }

                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, portGroup, tmpLocalPort, GEMAC_SG, RESET);
                        rc = mvHwsMacReset(devNum, portGroup, tmpLocalPort, portMode,  GEMAC_SG,
                                            RESET);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }
                    }
                    else
                    {
                        /* power down XG MAC */
                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _10GBase_SR_LR, PORT_POWER_DOWN);
                        rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort,
                                            _10GBase_SR_LR, PORT_POWER_DOWN);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }

                        /* power down GE MAC */
                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, SGMII, PORT_POWER_DOWN);
                        rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort,
                                            SGMII, PORT_POWER_DOWN);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }
                    }
                }
                else /* 1 serdes mode */
                {
                    /* power down XG MAC */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _10GBase_SR_LR, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort,
                                        _10GBase_SR_LR, PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    /* power down GE MAC */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, SGMII, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, SGMII,
                                        PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    /* no matter if port 9 uses regular or extended MAC reset
                        its extended PCS */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, 6/* xpcsNum of port 9 in RXAUI*/, XPCS, RESET);
                    rc = mvHwsPcsReset(devNum, portGroup,
                                        9/* mac 12 (port 9) in RXAUI should reset PCS #6*/,
                                        RXAUI,XPCS,RESET);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.
                       comboPortsInfoArray[portNum].macArray[0].macNum !=
                                                CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
                    { /* if it's combo port */
                        /* power down extended XG MAC */
                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _20GBase_SR_LR, PORT_POWER_DOWN);
                        rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort,
                                            _20GBase_SR_LR, PORT_POWER_DOWN);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }
                    }
                }
            }/* if(9 == localPort) */
        }
        else if(11 == tmpLocalPort)
        {
            if(localPort != 11)
            {
                rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, 11, SGMII, &tmpPortParams);
                if (GT_OK != rc)
                {
                    return rc;
                }
                macNumTmp       = tmpPortParams.portMacNumber;
                sdVecSizeTmp    = tmpPortParams.numOfActLanes;
                sdVectorPtrTmp  = tmpPortParams.activeLanesList;

                if((11 == macNumTmp) && (localPort != 9))
                {
                    /* power down XG MAC */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _10GBase_SR_LR, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, _10GBase_SR_LR,
                                        PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    /* power down GE MAC */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, SGMII, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, SGMII,
                                        PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    /* invalidate ifMode of port 11 only if it really used
                        regular MAC */
                    tmpGlobalPort =
                        PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,
                                                        portGroup, tmpLocalPort);
                    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,tmpGlobalPort) <
                                                            PRV_CPSS_PORT_XLG_E)
                    {
                        PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, tmpGlobalPort) =
                                                CPSS_PORT_INTERFACE_MODE_NA_E;
                    }
                }
                else if(9 == localPort)
                {
                    /* power down extended XG MAC */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _20GBase_SR_LR, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, _20GBase_SR_LR,
                                        PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    /* invalidate ifMode of port 11 only if it really used
                        extended XG MAC */
                    tmpGlobalPort =
                        PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,
                                                        portGroup, tmpLocalPort);
                    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,tmpGlobalPort) ==
                                                            PRV_CPSS_PORT_XLG_E)
                    {
                        PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, tmpGlobalPort) =
                                                CPSS_PORT_INTERFACE_MODE_NA_E;
                    }

                    if(14 == macNumTmp)
                    {/* power down extended GE MAC */
                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, SGMII, PORT_POWER_DOWN);
                        rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, SGMII,
                                            PORT_POWER_DOWN);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }

                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, 7/* xpcsNum of port 11 in RXAUI*/, XPCS, RESET);
                        rc = mvHwsPcsReset(devNum, portGroup,
                                            11/* mac #14 of port 11 in RXAUI should reset PCS #7*/,
                                            RXAUI, XPCS, RESET);
                        if(rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }

                        /* invalidate ifMode of port 11 only if it really used
                            extended GE/XG MAC */
                        if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,tmpGlobalPort) <
                                                                PRV_CPSS_PORT_XLG_E)
                        {
                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, tmpGlobalPort) =
                                                    CPSS_PORT_INTERFACE_MODE_NA_E;
                        }
                    }
                }
            }
            else /* if port 11 is main configured port */
            {
                if(1 == sdVecSize)
                {
                    /* power down XG MAC */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _10GBase_SR_LR, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, _10GBase_SR_LR,
                                        PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    /* power down GE MAC */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, SGMII, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, SGMII,
                                        PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.
                       comboPortsInfoArray[portNum].macArray[0].macNum !=
                                                CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
                    { /* if it's combo port */
                        /* power down extended XG MAC */
                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _20GBase_SR_LR, PORT_POWER_DOWN);
                        rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort,
                                            _20GBase_SR_LR, PORT_POWER_DOWN);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }
                    }
                }
                else /* if extended MAC used */
                {
                    /* power down XG MAC */
                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, _20GBase_SR_LR, PORT_POWER_DOWN);
                    rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, _20GBase_SR_LR,
                                        PORT_POWER_DOWN);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, 7/* xpcsNum of port 11 in RXAUI*/, XPCS, RESET);
                    rc = mvHwsPcsReset(devNum, portGroup,
                                        11/* mac #14 of port 11 in RXAUI should reset PCS #7*/,
                                        RXAUI, XPCS, RESET);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                        return rc;
                    }

                    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, 11, SGMII, &tmpPortParams);
                    if (GT_OK != rc)
                    {
                        return rc;
                    }
                    macNumTmp       = tmpPortParams.portMacNumber;
                    sdVecSizeTmp    = tmpPortParams.numOfActLanes;
                    sdVectorPtrTmp  = tmpPortParams.activeLanesList;

                    if(14 == macNumTmp)
                    {/* power down GE MAC */
                        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, SGMII, PORT_POWER_DOWN);
                        rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, SGMII,
                                            PORT_POWER_DOWN);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                            return rc;
                        }
                    }
                }

            } /* else if port 11 is main configured port */

        } /* else if(11 == tmpLocalPort) */

    } /* for(i = 0; i < sdVecSize; i++) */

portCreate:
    rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum,&refClock);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                               PRV_CPSS_DXCH_LION2_DISMATCH_PORTS_LINK_WA_E))
    {
        /* no need to check status, valid apEnable status will be in any case */
        (GT_VOID)cpssDxChPortApPortConfigGet(devNum, portNum, &apEnable, &apParams);

        /* don't run WA's if AP enabled on port */
        if((!apEnable) && ((CPSS_PORT_SPEED_10000_E == speed) &&
           ((CPSS_PORT_INTERFACE_MODE_KR_E == ifMode) ||
            (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode) ||
            (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode))))
        {/* force link down for case partner is in not matching mode */
            rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum, portNum, GT_TRUE);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d])", devNum, portGroup, localPort, portMode, lbPort, refClock, PRIMARY_LINE_SRC);
    cpssOsMemSet(&portInitInParam, 0,sizeof(portInitInParam));
    portInitInParam.lbPort = lbPort;
    portInitInParam.refClock = refClock;
    portInitInParam.refClockSource = PRIMARY_LINE_SRC;
    rc = mvHwsPortInit(devNum, portGroup, localPort, portMode, &portInitInParam);

    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    comboParamsPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portNum];
    if(comboParamsPtr->macArray[0].macNum != CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
    { /* if it's combo port */
        if (CPSS_PORT_SPEED_10000_E == speed)
        {
            portMode = _100GBase_KR10;
        }
        else if (CPSS_PORT_SPEED_1000_E == speed)
        {
            portMode = (SGMII == portMode) ? QSGMII : _100Base_FX;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, localPort, portMode, PORT_POWER_DOWN);
        rc = mvHwsPortReset(devNum, portGroup, localPort, portMode,
                            PORT_POWER_DOWN);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d])", devNum, portGroup, localPort, portMode, lbPort, refClock, PRIMARY_LINE_SRC);
        rc = mvHwsPortInit(devNum, portGroup, localPort, portMode, &portInitInParam);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }

        if (comboParamsPtr->preferredMacIdx != CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
        {
            rc = cpssDxChPortComboPortActiveMacSet(devNum, portNum,
                       &(comboParamsPtr->macArray[comboParamsPtr->preferredMacIdx]));
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {/* during port reset some configurations related to active port could
            be deleted, lets restore them, if it's first time port configured
            no harm - active MAC will be first in array */
            CPSS_DXCH_PORT_MAC_PARAMS_STC   activeMac;
            rc = cpssDxChPortComboPortActiveMacGet(devNum, portNum, &activeMac);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = cpssDxChPortComboPortActiveMacSet(devNum, portNum, &activeMac);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if(!lbPort)
        {/* update serdes registers with project specific values defined by application */
            rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPort, portMode, &tmpPortParams);
            if (GT_OK != rc)
            {
                return rc;
            }
            macNumTmp       = tmpPortParams.portMacNumber;
            sdVecSizeTmp    = tmpPortParams.numOfActLanes;
            sdVectorPtrTmp  = tmpPortParams.activeLanesList;
            rc = lion2PortSerdesTuning(devNum, portGroup, sdVectorPtrTmp,
                                        (GT_U32)sdVecSizeTmp, ifMode, speed);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        if((9 == localPort) || (11 == localPort))
        {/* set/unset extended mode configuration for ports 9,11*/

            /* can't use here cpssDxChPortExtendedModeEnableGet, because it purposed only for 1
               serdes interfaces */
            fieldData = BOOL2BIT_MAC(macNum != localPort);

            /* use extended or regular MAC for RX DMA */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.rxdmaIfConfig[localPort/8];
            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroup, regAddr,
                                                    (localPort%8)*4+3, 1, fieldData);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* use extended or regular MAC for TX DMA */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaExtendedPortsConfig;
            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroup, regAddr,
                                                 ((9 == localPort) ? 0 : 1), 1, fieldData);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if((CPSS_PORT_SPEED_10_E == speed) || (CPSS_PORT_SPEED_100_E == speed))
    {/* HWS doesn't support 10/100M, so after 1G configured by HWS, fix speed here */
        rc = geMacUnitSpeedSet(devNum, portNum, speed);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(!lbPort)
    {/* update serdes registers with project specific values defined by application */
        rc = lion2PortSerdesTuning(devNum, portGroup, sdVectorPtr, sdVecSize,
                                                                ifMode, speed);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* i/f width 64 or 256 bits */
    fieldData = 0x0;
    if (CPSS_PORT_SPEED_40000_E == speed || CPSS_PORT_SPEED_47200_E == speed)
    {
        fieldData = 0x2;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.rxdmaIfConfig[localPort/8];
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroup, regAddr,
                                            (localPort%8)*4, 3, fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }


    fieldData = 0x0;
    if (CPSS_PORT_SPEED_40000_E == speed || CPSS_PORT_SPEED_47200_E == speed)
    {
        fieldData = 0x1;
    }

    for(i = 0; i < 2; i++)
    {/* MPPM0/1 - XLG(40G) Mode */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->mppm[i].xlgMode;
        if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            /* support 1 MPPM instead of 2 (bobcat2) */
            continue;
        }
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroup, regAddr,
                                             localPort, 1, fieldData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* TXDMA - share Enable for ports 0,4,8,9 - for 40G or 47.2G */
    /* TXDMA - share Enable for ports 0,2,4,6,8,10 and 9,11 (extended ports) - for 20G port */
    fieldData = 0x0;
    if (CPSS_PORT_SPEED_40000_E == speed || CPSS_PORT_SPEED_47200_E == speed || CPSS_PORT_SPEED_20000_E == speed)
    {
        fieldData = 0x1;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.thresholdOverrideEnable[localPort/6];
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroup, regAddr,
                                         ((localPort%6)*5+4), 1, fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* WA for 40G power up*/
    if (CPSS_PORT_SPEED_40000_E == speed || CPSS_PORT_SPEED_47200_E == speed || CPSS_PORT_SPEED_20000_E == speed)
    {
        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_LION2_SHARE_EN_CHANGE_WA_E))
        {
            rc = lion2ShareDisableWaExecute(devNum, portGroup, localPort, GT_TRUE);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    rc = prvCpssDxChPortPizzaArbiterIfConfigure(devNum, portNum,
                                    PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portNum));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* in propriatary pizza's profiles could happen that port's HW is free,
        but pizza slices reassigned to other ports and some port remains
        with no resourses for traffic transmission */
    for(tmpLocalPort = 0; tmpLocalPort <= 11; tmpLocalPort++)
    {
        tmpGlobalPort =
            PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,
                                            portGroup, tmpLocalPort);

        if(portNum == tmpGlobalPort)
        {/* skip configured port itself */
            continue;
        }

        PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, tmpGlobalPort);

        rc = cpssDxChPortPizzaArbiterIfPortStateGet(devNum, portGroup, tmpLocalPort,
                                                    &sliceNumUsed);
        if (rc != GT_OK)
        {
            return rc;
        }

        if((0 == sliceNumUsed) && (PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,tmpGlobalPort)
                                   != CPSS_PORT_INTERFACE_MODE_NA_E)
                                                           && (PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, tmpGlobalPort)
                                                                   !=  CPSS_PORT_SPEED_NA_E))
        {/* reset units of port which has no more pizza slices, i.e. actually
            can't forward traffic, although it's HW units are free */
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,tmpGlobalPort),
                                                      PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, tmpGlobalPort),
                                                      &tmpPortMode);
            if(rc != GT_OK)
            {
                return rc;
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, tmpLocalPort, tmpPortMode,PORT_POWER_DOWN);
            rc = mvHwsPortReset(devNum, portGroup, tmpLocalPort, tmpPortMode,PORT_POWER_DOWN);
            if (rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }

            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, tmpGlobalPort) =  CPSS_PORT_INTERFACE_MODE_NA_E;
            PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, tmpGlobalPort)  =   CPSS_PORT_SPEED_NA_E;
        }
    }

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                               PRV_CPSS_DXCH_LION2_DISMATCH_PORTS_LINK_WA_E))
    {
        if (ifMode != CPSS_PORT_INTERFACE_MODE_HGL_E)
        {/* restore force link down state defined by application for modes,
            where no problem of link up on dismatching interfaces */
            if((apEnable) || (CPSS_PORT_SPEED_10000_E != speed) ||
               ((CPSS_PORT_INTERFACE_MODE_KR_E != ifMode) &&
                   (CPSS_PORT_INTERFACE_MODE_SR_LR_E != ifMode) &&
                (CPSS_PORT_INTERFACE_MODE_XHGS_E != ifMode)
                && (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E != ifMode)))
            {
                rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum,portNum,
               CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                                  info_PRV_CPSS_DXCH_LION2_DISMATCH_PORT_LINK_WA_E.
                                    portForceLinkDownBmpPtr,portNum));
                if(rc!=GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    switch(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum))
    {
        case PRV_CPSS_PORT_GE_E:
            rc = prvCpssDxChPortPeriodicFlowControlIntervalSelectionSet(devNum,
                                portNum, CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case PRV_CPSS_PORT_XG_E:
        case PRV_CPSS_PORT_XLG_E:
        case PRV_CPSS_PORT_HGL_E:
            rc = prvCpssDxChPortPeriodicFlowControlIntervalSelectionSet(devNum,
                                portNum, CPSS_DXCH_PORT_PERIODIC_FC_TYPE_XG_E);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(originalPortState != GT_FALSE)
    {
        if((PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum) ==
                                    CPSS_PORT_INTERFACE_MODE_HGL_E) &&
            ((PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portNum) ==
                                    CPSS_PORT_SPEED_15000_E) ||
            (PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portNum) ==
                                        CPSS_PORT_SPEED_16000_E)))
        {
            return GT_OK;
        }
        else
        {
            rc = prvCpssDxChPortEnableSet(devNum,portNum,GT_TRUE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortSerdesPowerStatusSet function
* @endinternal
*
* @brief   Sets power state of SERDES port lanes according to port capabilities.
*         XG / XAUI ports: All 4 (TX or RX) lanes are set simultanuously.
*         HX / QX ports: lanes 0,1 or TX and RX may be set separately.
*         For xcat3/bobcat2_B0 its not full power down but only rx & tx power down.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - may be either CPSS_PORT_DIRECTION_RX_E,
*                                      CPSS_PORT_DIRECTION_TX_E, or
*                                      CPSS_PORT_DIRECTION_BOTH_E.
* @param[in] lanesBmp                 - bitmap of SERDES lanes (bit 0-> lane 0, etc.)
* @param[in] powerUp                  - GT_TRUE  = power up, GT_FALSE = power down
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note DxCh3 and above devices supports only CPSS_PORT_DIRECTION_BOTH_E.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
)
{
    GT_STATUS rc;   /* return code */
    PRV_CPSS_DXCH_PORT_SERDES_POWER_STATUS_SET_FUN serdesPowerSetFuncPtr; /* pointer to [devFamily] specific function */
    GT_BOOL     originalPortState;  /* port enable state */
    CPSS_PORT_SPEED_ENT speed;/* current speed of port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_DIAMOND_E);

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "API not applicable for sip5_15 devices");
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum) || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
    {
        rc = prvCpssDxChPortSerdesPartialPowerDownSet(devNum, portNum, !powerUp, !powerUp);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {

        if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) == PRV_CPSS_PORT_NOT_EXISTS_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) == PRV_CPSS_PORT_FE_E)
        {
            /* no serdes support for FE ports */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if((serdesPowerSetFuncPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->devObj.portPtr->
                                                    setSerdesPowerStatus) == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
        }

        if((systemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
            && (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable))
        {/* do only if we during recovery */
            GT_BOOL   currentState;

            rc = prvCpssDxCh3PortSerdesPowerUpDownGet(devNum, portNum, &currentState);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(powerUp == currentState)
            {
                return GT_OK;
            }
        }

        /* disable port before port's units reset/power down to prevent traffic stuck */
        speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portNum);
        originalPortState = GT_FALSE;
        if((CPSS_PORT_INTERFACE_MODE_HGL_E !=
                                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum)) ||
            ((CPSS_PORT_SPEED_15000_E != speed) &&
             (CPSS_PORT_SPEED_16000_E != speed)))
        {
            rc = prvCpssDxChPortEnableGet(devNum, portNum, &originalPortState);
            if (rc != GT_OK)
            {
                return rc;
            }

            if(originalPortState != GT_FALSE)
            {
                rc = prvCpssDxChPortEnableSet(devNum, portNum, GT_FALSE);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        rc = serdesPowerSetFuncPtr(devNum, portNum, direction, lanesBmp, powerUp);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(originalPortState != GT_FALSE)
        {
            rc = prvCpssDxChPortEnableSet(devNum, portNum, GT_TRUE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
           if(powerUp)
           {
                rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum, portNum, GT_FALSE);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
           else
           {
                rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum, portNum, GT_TRUE);
                if(rc != GT_OK)
                {
                    return rc;
                }
           }
        }

    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesPowerStatusSet function
* @endinternal
*
* @brief   Sets power state of SERDES port lanes according to port capabilities.
*         XG / XAUI ports: All 4 (TX or RX) lanes are set simultanuously.
*         HX / QX ports: lanes 0,1 or TX and RX may be set separately.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - may be either CPSS_PORT_DIRECTION_RX_E,
*                                      CPSS_PORT_DIRECTION_TX_E, or
*                                      CPSS_PORT_DIRECTION_BOTH_E.
* @param[in] lanesBmp                 - bitmap of SERDES lanes (bit 0-> lane 0, etc.)
* @param[in] powerUp                  - GT_TRUE  = power up, GT_FALSE = power down
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note DxCh3 and above devices supports only CPSS_PORT_DIRECTION_BOTH_E.
*
*/
GT_STATUS cpssDxChPortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesPowerStatusSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, lanesBmp, powerUp));

    rc = internal_cpssDxChPortSerdesPowerStatusSet(devNum, portNum, direction, lanesBmp, powerUp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, lanesBmp, powerUp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortGroupSerdesPowerStatusSet function
* @endinternal
*
* @brief   Sets power state of SERDES port lanes per group
*         according to port capabilities.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portSerdesGroup          - port group number
*                                      DxCh3 Giga/2.5 G, xCat GE devices:
*                                      Ports       |    SERDES Group
*                                      0..3        |      0
*                                      4..7        |      1
*                                      8..11       |      2
*                                      12..15      |      3
*                                      16..19      |      4
*                                      20..23      |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      DxCh3 XG devices:
*                                      0           |      0
*                                      4           |      1
*                                      10          |      2
*                                      12          |      3
*                                      16          |      4
*                                      22          |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      xCat FE devices
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      Lion devices: Port == SERDES Group
* @param[in] powerUp                  - GT_TRUE  = power up, GT_FALSE = power down
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, portSerdesGroup
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get SERDES port group according to port by calling to
*       cpssDxChPortSerdesGroupGet.
*
*/
static GT_STATUS internal_cpssDxChPortGroupSerdesPowerStatusSet
(
    IN  GT_U8      devNum,
    IN  GT_U32     portSerdesGroup,
    IN  GT_BOOL    powerUp
)
{
    GT_PHYSICAL_PORT_NUM portNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_XCAT3_E | CPSS_LION2_E
                                          | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portSerdesGroup);
        return lionPortSerdesPowerStatusSet(devNum,portSerdesGroup,CPSS_PORT_DIRECTION_BOTH_E,0xF,powerUp);
    }
    else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        if(portSerdesGroup < 6) /* network ports */
        {
            portNum = portSerdesGroup<<2;

            if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) == PRV_CPSS_PORT_FE_E)
            {
                /* no serdes support for FE ports */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }
        else if (portSerdesGroup < 10) /* Flex Link ports */
        {
            portNum = portSerdesGroup + 18;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        return prvCpssDxChXcatPortSerdesPowerStatusSet(devNum,portNum,CPSS_PORT_DIRECTION_BOTH_E,0xF,powerUp);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChPortGroupSerdesPowerStatusSet function
* @endinternal
*
* @brief   Sets power state of SERDES port lanes per group
*         according to port capabilities.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portSerdesGroup          - port group number
*                                      DxCh3 Giga/2.5 G, xCat GE devices:
*                                      Ports       |    SERDES Group
*                                      0..3        |      0
*                                      4..7        |      1
*                                      8..11       |      2
*                                      12..15      |      3
*                                      16..19      |      4
*                                      20..23      |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      DxCh3 XG devices:
*                                      0           |      0
*                                      4           |      1
*                                      10          |      2
*                                      12          |      3
*                                      16          |      4
*                                      22          |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      xCat FE devices
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      Lion devices: Port == SERDES Group
* @param[in] powerUp                  - GT_TRUE  = power up, GT_FALSE = power down
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, portSerdesGroup
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get SERDES port group according to port by calling to
*       cpssDxChPortSerdesGroupGet.
*
*/
GT_STATUS cpssDxChPortGroupSerdesPowerStatusSet
(
    IN  GT_U8      devNum,
    IN  GT_U32     portSerdesGroup,
    IN  GT_BOOL    powerUp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortGroupSerdesPowerStatusSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portSerdesGroup, powerUp));

    rc = internal_cpssDxChPortGroupSerdesPowerStatusSet(devNum, portSerdesGroup, powerUp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portSerdesGroup, powerUp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChLpPreInitSerdesSequence function
* @endinternal
*
* @brief   Make pre-init sequence for given serdes.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - ports group number
* @param[in] serdesNum                - given SERDES number
* @param[in] serdesExtCnfg1Value      - value for Serdes External Cnfg 1 register .
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on not initialized serdes
*/
GT_STATUS prvCpssDxChLpPreInitSerdesSequence
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               serdesNum,
    IN  GT_U32               serdesExtCnfg1Value
)
{
    GT_U32  regAddr, regMask = 0xFFFFFFFF;
    GT_STATUS rc = GT_OK;
    GT_U32 extCfgReg2_0, extCfgReg2_1;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.serdesRefClock == CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E)
        {
            extCfgReg2_0 = lpSerdesExtConfig_Lion_B0_156Clk_LR[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_0_E][1];
            extCfgReg2_1 = lpSerdesExtConfig_Lion_B0_156Clk_LR[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_1_E][1];
        }
        else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        regMask = FIELD_MASK_NOT_MAC(11,1);
    }
    else /* xcat/xcat2 (TBD: use specific array) */
    {
        extCfgReg2_0 = lpSerdesExtConfig[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_0_E][1];
        extCfgReg2_1 = lpSerdesExtConfig[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_1_E][1];
    }

    /* Activate SerDes Reset */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesNum].serdesExternalReg2;
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum, portGroupId, regAddr, regMask, extCfgReg2_0);
    if (rc != GT_OK)
        return rc;

    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,10);

    SERDES_DBG_DUMP_MAC(("regAddr=0x%08x,regVal==0x%08x\n", regAddr, extCfgReg2_0));
    SERDES_DBG_DUMP_MAC(("SLEEP 10 msec\n"));

    /* Configuring reference clock accordingly to current interface mode */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesNum].serdesExternalReg1;
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr, serdesExtCnfg1Value);
    if (rc != GT_OK)
        return rc;

    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,10);

    SERDES_DBG_DUMP_MAC(("prvCpssDxChLpPreInitSerdesSequence:regAddr=0x%08x,serdesExtCnfg1Value==0x%08x\n",
                        regAddr, serdesExtCnfg1Value));
    SERDES_DBG_DUMP_MAC(("SLEEP 10 msec\n"));

    /* Deactivate SerDes Reset - must be done otherwise device stuck when try to access sersed under reset */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesNum].serdesExternalReg2;
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum, portGroupId, regAddr, regMask, extCfgReg2_1);

    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,10);

    SERDES_DBG_DUMP_MAC(("regAddr=0x%08x,regVal==0x%08x\n", regAddr, extCfgReg2_1));
    SERDES_DBG_DUMP_MAC(("SLEEP 10 msec\n"));

    return rc;
}


/**
* @internal prvCpssDxChLpCheckSerdesInitStatus function
* @endinternal
*
* @brief   Check LP SERDES initialization status.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - ports group number
* @param[in] serdesNum                - SERDES number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on not initialized serdes
*/
GT_STATUS prvCpssDxChLpCheckSerdesInitStatus
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               serdesNum
)
{
    GT_U32 regAddr;
    GT_STATUS rc = GT_OK;
    GT_U32 fieldData = 0;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
        serdesConfig[serdesNum].serdesExternalReg2;
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 3, 1, &fieldData);
    if (rc != GT_OK)
        return rc;
    if (fieldData == 0)
    {
        /* serdes is in reset and its registers are not available */
        rc = GT_NOT_INITIALIZED;
    }

    return rc;
}



/**
* @internal prvCpssDxChLpSerdesInit function
* @endinternal
*
* @brief   Make serdes initialization and enable access to serdes registers
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - ports group number
* @param[in] serdesNum2Init           - SERDES number that should be initialized
* @param[in] serdesNum2CopyInit       - previously initialized SERDES number to take init params from
* @param[in] isDefaultInit            - GT_TRUE:  use default params for SERDES init
*                                      GT_FALSE: use parameters of serdesNum2CopyInit for init
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on not initialized serdes
*/
static GT_STATUS prvCpssDxChLpSerdesInit
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               serdesNum2Init,
    IN  GT_U32               serdesNum2CopyInit,
    IN  GT_BOOL              isDefaultInit
)
{
    /* for ports in RXAUI/HGS/XGMII mode - set serdes speed correspondingly */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode; /* port inetrface mode */
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesSpeed; /* serdes speed/frequency */
    GT_U32 regAddr; /* register address */
    GT_U32 transmitterReg1Value = 0; /* value of TX1 reg */
    GT_U32 ffeRegValue = 0; /* value of FFE reg */
    GT_U32 pllIntpReg2Value; /* value of PLLIntp2 reg */
    GT_U32 pllIntpReg3Value; /* value of PLLIntp3 reg */
    GT_STATUS rc = GT_OK;           /* return code */
    GT_U32 xgMode = 0;          /* interface mode of XG port */
    PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY *serdesPowerUpSequencePtr;
                                    /* pointer to serdes power up sequence */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_ARRAY  *serdesExtCfgPtr; /* pointer to
        external registers serdes power up configuration */

    SERDES_DBG_DUMP_MAC(("prvCpssDxChLpSerdesInit(%d,%d,%d)\n", serdesNum2Init,
                 serdesNum2CopyInit, isDefaultInit));
    rc = serDesConfig(devNum,&serdesPowerUpSequencePtr,&serdesExtCfgPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (isDefaultInit == GT_TRUE)
    {
        /* default values will be used */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
        {
            serdesSpeed = serdesFrequency[CPSS_PORT_INTERFACE_MODE_RXAUI_E]
                                                    [CPSS_PORT_SPEED_10000_E];
        }
        else
        {
            /* set default pllIntpReg2 and pllIntpReg3 for flexLink ports serdes */
            if (serdesNum2Init >= 6)
            {
                /* get flexLink ports mode */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.sampledAtResetReg;
                if (prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,26,4,&xgMode) != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }

                /* for ports in RXAUI/HGS/XGMII mode set serdes speed correspondingly */
                if ((xgMode >> (serdesNum2Init-6) / 4) & 0x1)
                {
                    rc = cpssDxChPortInterfaceModeGet(devNum,24+((serdesNum2Init-6)/4),&ifMode);
                    /* if no interface set -> do nothing */
                    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                    {
                        return GT_OK;
                    }
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    serdesSpeed = (ifMode == CPSS_PORT_INTERFACE_MODE_RXAUI_E)
                                    ? CPSS_DXCH_PORT_SERDES_SPEED_6_25_E :
                                            CPSS_DXCH_PORT_SERDES_SPEED_3_125_E;
                }
                else
                {/* lets default serdes frequency fit 1G port speed */
                    serdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;
                }
            }
            else
            {   /* NP ports of xcat/xcat2 may work in QSGMII or SGMII only */
                rc = cpssDxChPortInterfaceModeGet(devNum,serdesNum2Init<<2,&ifMode);
                /* if no interface set -> do nothing */
                if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    return GT_OK;
                }
                if (rc != GT_OK)
                {
                    return rc;
                }
                serdesSpeed = (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
                                ? CPSS_DXCH_PORT_SERDES_SPEED_5_E :
                                            CPSS_DXCH_PORT_SERDES_SPEED_1_25_E;
            }
        }

        transmitterReg1Value = serdesPowerUpSequencePtr
                                [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_INDEX_E]
                                                                    [serdesSpeed];
        ffeRegValue = serdesPowerUpSequencePtr
                                    [PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_INDEX_E]
                                                                    [serdesSpeed];
        pllIntpReg2Value = serdesPowerUpSequencePtr
                            [PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG2_INDEX_CNS]
                                                                    [serdesSpeed];
        pllIntpReg3Value = serdesPowerUpSequencePtr
                            [PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG3_INDEX_CNS]
                                                                    [serdesSpeed];
    }
    else
    {
        /* init parameters is taken from another already initialized serdes */
        /* copy  pllIntpReg2 Register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            serdesConfig[serdesNum2CopyInit].pllIntpReg2;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &pllIntpReg2Value);
        if (rc != GT_OK)
            return rc;

        /* copy  pllIntpReg3 Register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            serdesConfig[serdesNum2CopyInit].pllIntpReg3;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &pllIntpReg3Value);
        if (rc != GT_OK)
            return rc;

        rc = prvGetLpSerdesSpeed(devNum, portGroupId, serdesNum2Init, &serdesSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* can't determine serdes speed */
        if (serdesSpeed == CPSS_DXCH_PORT_SERDES_SPEED_NA_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    /* Pre Configuration for serdesNum2Init, */
    /* enables access to serdes registers */
    rc = prvCpssDxChLpPreInitSerdesSequence(devNum, portGroupId, serdesNum2Init,
                                            serdesExtCfgPtr[PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG1_E]
                                    [(serdesSpeed == CPSS_DXCH_PORT_SERDES_SPEED_1_25_E) ? 0 : 1]);
    if (rc != GT_OK)
        return rc;

    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,5);

    if (isDefaultInit == GT_TRUE)
    {
        /* set transmitterReg1 register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            serdesConfig[serdesNum2Init].transmitterReg1;
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr, transmitterReg1Value);
        if (rc != GT_OK)
            return rc;

        /* set ffeReg register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesNum2Init].ffeReg;
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr, ffeRegValue);
        if (rc != GT_OK)
            return rc;
    }

    /* set  pllIntpReg2 and pllIntpReg3 for flexLink ports serdes */
    if ((serdesNum2Init >= 6) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    serdesConfig[serdesNum2Init].pllIntpReg2;
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr, pllIntpReg2Value);
        if (rc != GT_OK)
            return rc;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    serdesConfig[serdesNum2Init].pllIntpReg3;
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr, pllIntpReg3Value);
        if (rc != GT_OK)
            return rc;
    }
    return rc;
}


/**
* @internal prvCpssDxChLpCheckAndInitNeededSerdes function
* @endinternal
*
* @brief   Make serdes initialization and enable access to serdes registers
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - ports group number
* @param[in] startSerdes              - start SERDES number
* @param[in] serdesesNum              - number of  SERDES per port
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on not initialized serdes
*/
GT_STATUS prvCpssDxChLpCheckAndInitNeededSerdes
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               startSerdes,
    IN  GT_U32               serdesesNum
)
{
    GT_BOOL serdesInitMap[PRV_CPSS_DXCH_MAX_SERDES_NUM_PER_PORT_CNS] =
            {GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};
    GT_U32      initializedSerdesNum = 0xff;
    GT_U32      initialSerdeCounter = 0;
    GT_U32      i;
    GT_STATUS   rc = GT_OK;

    SERDES_DBG_DUMP_MAC(("prvCpssDxChLpCheckAndInitNeededSerdes:startSerdes=%d,serdesesNum=%d\n",
                         startSerdes, serdesesNum));

    /* now check if all serdeses of given port are initialized */
    for (i = startSerdes; i < startSerdes + serdesesNum;i++)
    {
        rc = prvCpssDxChLpCheckSerdesInitStatus(devNum,portGroupId,i);
        if ( (rc != GT_OK) && (rc != GT_NOT_INITIALIZED))
        {
            SERDES_DBG_DUMP_MAC(("prvCpssDxChLpCheckAndInitNeededSerdes:prvCpssDxChLpCheckSerdesInitStatus\n"));
            return rc;
        }
        if (rc != GT_NOT_INITIALIZED)
        {
            if(PRV_CPSS_DXCH_MAX_SERDES_NUM_PER_PORT_CNS <= (i - startSerdes))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            else
                serdesInitMap[i - startSerdes]= GT_TRUE;

            /* save  initialized serdes num */
            initializedSerdesNum = i;
            initialSerdeCounter++;
        }
    }
    if (initialSerdeCounter != serdesesNum)
    {
        /* not all serdeses are initialized */
        for (i = 0; i < serdesesNum; i++)
        {
            if(serdesInitMap[i] == GT_TRUE)
                continue;
            if (initializedSerdesNum != 0xFF)
            {
                /* there is at least one initialized, so take params from it */
                rc = prvCpssDxChLpSerdesInit(devNum,portGroupId,i+startSerdes,initializedSerdesNum,GT_FALSE);
            }
            else
            {
                /* default params */
                rc = prvCpssDxChLpSerdesInit(devNum,portGroupId,i+startSerdes,initializedSerdesNum,GT_TRUE);
            }
            if (rc != GT_OK)
            {
                SERDES_DBG_DUMP_MAC(("prvCpssDxChLpCheckAndInitNeededSerdes:prvCpssDxChLpSerdesInit(%d)\n",
                                     (initializedSerdesNum == 0xFF)));
                return rc;
            }
        }
    }
    return rc;
}


/**
* @internal prvCpssDxChLpGetFirstInitSerdes function
* @endinternal
*
* @brief   Get first initialize serdes per port
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - ports group number
* @param[in] startSerdes              - start SERDES number
*
* @param[out] initSerdesNumPtr         - pointer to first initialize serdes number per port
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on not initialized serdes
*/
GT_STATUS prvCpssDxChLpGetFirstInitSerdes
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               startSerdes,
    OUT GT_U32               *initSerdesNumPtr
)
{
    GT_U32 tmpStartSerdes, /* first serdes number */
            serdesNumPerPort; /* number of serdeses used by current port */
    GT_U32 i;   /* counter */
    GT_U32 firstInitializedSerdes =0xff; /* number of first initialized serdes of current port */
    GT_STATUS   rc = GT_OK; /* return code */
    GT_PHYSICAL_PORT_NUM globalPort;   /* port number in box (not in core) */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;    /* port interface mode */
    GT_PHYSICAL_PORT_NUM localPort;     /* number of port in port group */

    /* check if given serdes is initialized */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E ||
       PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        localPort = startSerdes/2;
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
        localPort = (startSerdes < 6) ? startSerdes*4 /* network ports connected by QSGMII four ports on one serdes */ :
                                            24+(startSerdes-6); /* stack ports of xcat2 has just one serdes */
    else /* CPSS_PP_FAMILY_DXCH_XCAT_E */
        localPort = (startSerdes < 6) ? startSerdes*4 /* network ports connected by QSGMII four ports on one serdes */ :
                                            24+((startSerdes-6)/4); /* stack ports of xcat has 4 serdes each */

    globalPort = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,portGroupId,localPort);
    rc = cpssDxChPortInterfaceModeGet(devNum,globalPort,&ifMode);
    if((rc != GT_OK) && (rc != GT_BAD_STATE))
    {
        return rc;
    }

    if((rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, globalPort, ifMode, &tmpStartSerdes,
                                                &serdesNumPerPort)) != GT_OK)
    {
        return rc;
    }

    for (i = tmpStartSerdes; i < tmpStartSerdes + serdesNumPerPort; i++)
    {
        rc = prvCpssDxChLpCheckSerdesInitStatus(devNum,portGroupId,i);
        if ((rc != GT_OK)&& (rc != GT_NOT_INITIALIZED))
        {
            return rc;
        }
        if(rc == GT_OK)
        {
            firstInitializedSerdes = i;
            break;
        }
    }
    if (firstInitializedSerdes != 0xff)
    {
        *initSerdesNumPtr = firstInitializedSerdes;
        rc = GT_OK;
    }
    else
    {
        rc = GT_NOT_INITIALIZED;
    }
    return rc;
}

/**
* @internal serdesSpeedSet function
* @endinternal
*
* @brief   Configure serdes registers uniqiue for specific frequency
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - the speed is not supported on the port
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong speed
*/
GT_STATUS serdesSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_U32 regAddr; /* register address */
    GT_U32 regValue;    /* register value */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;    /* interface mode */
    GT_STATUS rc;   /* return code */
    GT_U32 i;       /* iterator */
    GT_U32 startSerdes = 0; /* first serdes occupied by port */
    GT_U32 serdesesNum = 0; /* number of serdeses occupied by port */
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesSpeed = CPSS_DXCH_PORT_SERDES_SPEED_NA_E;
                                                            /* serdes frequency to configure */
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesSpeedPrev;    /* old serdes frequency */
    GT_U32      portGroupId;     /*the port group Id - support multi-port-groups device */
    PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY *serdesPowerUpSequencePtr;
                                    /* pointer to serdes power up sequence */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_ARRAY  *serdesExtCfgPtr; /* pointer to
        external registers serdes power up configuration */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                        | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    rc = serDesConfig(devNum,&serdesPowerUpSequencePtr,&serdesExtCfgPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    serdesSpeed = serdesFrequency[ifMode][speed];
    if(serdesSpeed == CPSS_DXCH_PORT_SERDES_SPEED_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,&startSerdes,&serdesesNum)) != GT_OK)
        return rc;

    /* check if all serdes in port are initialized and make proper initialization
       if it is needed */
    rc = prvCpssDxChLpCheckAndInitNeededSerdes(devNum,portGroupId,startSerdes,serdesesNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvGetLpSerdesSpeed(devNum,portGroupId,startSerdes,&serdesSpeedPrev);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* perform power down only if serdes speed changed */
    if (serdesSpeedPrev != serdesSpeed)
    {
        /* power down all serdes */
        for (i = startSerdes; i < startSerdes + serdesesNum;i++)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].powerReg;
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId,regAddr, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* set PLL/INTP Register2 and PLL/INTP Register3*/
        for (i = startSerdes; i < startSerdes + serdesesNum;i++)
        {
            regValue = serdesPowerUpSequencePtr[PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG2_INDEX_CNS]
                                                [serdesSpeed];
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].pllIntpReg2;
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId,regAddr, regValue);
            if (rc != GT_OK)
            {
                return rc;
            }

            HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,20);

            regValue = serdesPowerUpSequencePtr[PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG3_INDEX_CNS]
                                                [serdesSpeed];
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].pllIntpReg3;
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId,regAddr, regValue);
            if (rc != GT_OK)
            {
                return rc;
            }

            HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,20);
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortSerdesTuningSet function
* @endinternal
*
* @brief   SerDes fine tuning values set.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) where values
*                                      in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set same tuning parameters for all lanes defined in laneBmp in SW DB,
*       then cpssDxChPortSerdesPowerStatusSet will write them to HW.
*       WARNING: This API requires interface to be already configured on port,
*       kept for backward compatibility and highly advised not to use it.
*       The proper way is to use cpssDxChPortSerdesLaneTuningSet
*
*/
static GT_STATUS internal_cpssDxChPortSerdesTuningSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneBmp,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;   /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode; /* interface mode of port */
    GT_U32  startSerdes,    /* first serdes occupied by port */
            serdesNumPerPort; /* number of serdeses occupied by port in current
                                interface mode */
    GT_U32  i;      /* iterator */
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device or single
                            core for multi-core devices */
    GT_U32  serdesNumInDevice;    /* global number of serdes in multi-core device */
    GT_U32  portGroupId;
    GT_U32  maxSerdesNum;   /* maximum serdeses in device */
    GT_U32  portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E |
                                CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((serdesFrequency < CPSS_DXCH_PORT_SERDES_SPEED_1_25_E) ||
       (serdesFrequency >= CPSS_DXCH_PORT_SERDES_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
    if((rc != GT_OK) && (rc != GT_BAD_STATE))
        return rc;

    if((rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                               &startSerdes,
                                               &serdesNumPerPort)) != GT_OK)
    {
        return rc;
    }
    if(serdesNumPerPort == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "internal_cpssDxChPortSerdesTuningSet return code is [%d], serdesNumPerPort is 0", rc);
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        tuneValuesPtr->txAmpAdj = GT_FALSE;
        tuneValuesPtr->txAmpShft = GT_FALSE;
        if (tuneValuesPtr->txEmphAmp    > 32  ||
            tuneValuesPtr->txEmph1      > 32  ||
            tuneValuesPtr->txEmphAmp    < -32 ||
            tuneValuesPtr->txEmph1      < -32 ||
            tuneValuesPtr->txAmp        > 31  ||
            tuneValuesPtr->sqlch        > 310 ||
            tuneValuesPtr->DC           > 255 ||
            tuneValuesPtr->LF           > 15  ||
            tuneValuesPtr->HF           > 15  ||
            tuneValuesPtr->BW           > 15  ||
            tuneValuesPtr->LB           > 15)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
   }
   else if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ||
        (CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ||
        (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        /* for now just these parameters supported in Lion2 */
        if (tuneValuesPtr->txEmphAmp    >= BIT_4 ||
            tuneValuesPtr->txEmph1      >= BIT_4 ||
            tuneValuesPtr->txAmp        >= BIT_5 ||
            tuneValuesPtr->txAmpAdj     >= BIT_1 ||
            tuneValuesPtr->ffeC         >= BIT_4 ||
            tuneValuesPtr->ffeR         >= BIT_3 ||
            tuneValuesPtr->align90      >= BIT_7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (tuneValuesPtr->txEmphAmp    >= BIT_4 ||
            tuneValuesPtr->txAmp        >= BIT_5 ||
            tuneValuesPtr->txAmpAdj     >= BIT_1 ||
            tuneValuesPtr->ffeC         >= BIT_4 ||
            tuneValuesPtr->ffeR         >= BIT_3 ||
            tuneValuesPtr->ffeS         >= BIT_2 ||
            tuneValuesPtr->dfe          >= BIT_5 ||
            tuneValuesPtr->sampler      >= BIT_7 ||
            tuneValuesPtr->sqlch        >= BIT_4)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    lanesNumInDev = prvCpssDxChHwInitNumOfSerdesGet(devNum);

    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        maxSerdesNum = lanesNumInDev * PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.
                                                                numOfPortGroups;
    }
    else
    {
        maxSerdesNum = lanesNumInDev;
    }

    if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr)
    {
        GT_U32 memSize;

        memSize = sizeof(CPSS_DXCH_PORT_SERDES_TUNE_STC_PTR) * maxSerdesNum;

        PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr =
                        (CPSS_DXCH_PORT_SERDES_TUNE_STC_PTR*)cpssOsMalloc(memSize);
        if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        cpssOsMemSet(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr, 0, memSize);
    }

    serdesNumInDevice = startSerdes;
    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
        serdesNumInDevice += portGroupId*lanesNumInDev;
    }

    for(i = 0; (i < lanesNumInDev) && (serdesNumInDevice+i < maxSerdesNum); i++)
    {
        if((laneBmp & (1<<i)) == 0)
            continue;

        if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i])
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i] =
                (CPSS_DXCH_PORT_SERDES_TUNE_STC_PTR)cpssOsMalloc(
                    sizeof(CPSS_DXCH_PORT_SERDES_TUNE_STC)*CPSS_DXCH_PORT_SERDES_SPEED_NA_E);
            if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i])
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            cpssOsMemSet(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i], 0,
                        sizeof(CPSS_DXCH_PORT_SERDES_TUNE_STC)*CPSS_DXCH_PORT_SERDES_SPEED_NA_E);
        }

        PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                [serdesFrequency] = *tuneValuesPtr;

        SERDES_DBG_DUMP_MAC(("dfe=0x%x,ffeR=0x%x,ffeC=0x%x,sampler=0x%x,sqlch=0x%x,\
                        txEmphAmp=0x%x,txAmp=0x%x,txAmpAdj=0x%x,ffeS=0x%x,txEmphEn=0x%x\n",
                    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                                    [serdesFrequency].dfe,
                    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                                    [serdesFrequency].ffeR,
                    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                                    [serdesFrequency].ffeC,
                    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                                    [serdesFrequency].sampler,
                    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                                    [serdesFrequency].sqlch,
                    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                                    [serdesFrequency].txEmphAmp,
                    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                                    [serdesFrequency].txAmp,
                    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                                    [serdesFrequency].txAmpAdj,
                    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                                    [serdesFrequency].ffeS,
                    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                                    [serdesFrequency].txEmphEn));
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesTuningSet function
* @endinternal
*
* @brief   SerDes fine tuning values set.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) where values
*                                      in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set same tuning parameters for all lanes defined in laneBmp in SW DB,
*       then cpssDxChPortSerdesPowerStatusSet will write them to HW.
*
*/
GT_STATUS cpssDxChPortSerdesTuningSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneBmp,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesTuningSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneBmp, serdesFrequency, tuneValuesPtr));

    rc = internal_cpssDxChPortSerdesTuningSet(devNum, portNum, laneBmp, serdesFrequency, tuneValuesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneBmp, serdesFrequency, tuneValuesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesTuningGet function
* @endinternal
*
* @brief   SerDes fine tuning values get.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure to put tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reads values saved in SW DB by cpssDxChPortSerdesTuningSet or
*       if it was not called, from default matrix (...SerdesPowerUpSequence).
*
*/
static GT_STATUS internal_cpssDxChPortSerdesTuningGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;   /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode; /* interface mode of port */
    GT_U32  startSerdes,    /* first serdes occupied by port */
            serdesNumPerPort; /* number of serdeses occupied by port in current
                                interface mode */
    PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY *serdesPowerUpSequencePtr;
                                    /* pointer to serdes power up sequence */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_ARRAY  *serdesExtCfgPtr; /* pointer to
        external registers serdes power up configuration (just dummy here) */
    GT_U32  regAddr;    /* register address */
    GT_U32  portGroupId;    /* the port group Id - support multi-port-groups device */
    GT_U32  valueIndex; /* index of value in power up sequence array */
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device */
    GT_U32  portMacMap; /* number of mac mapped to this physical port */
    GT_U32  serdesNumInDevice;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E |
                                CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((serdesFrequency < CPSS_DXCH_PORT_SERDES_SPEED_1_25_E) ||
       (serdesFrequency >= CPSS_DXCH_PORT_SERDES_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
    if((rc != GT_OK) && (rc != GT_BAD_STATE))
        return rc;

    if((rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode, &startSerdes,
                                                &serdesNumPerPort)) != GT_OK)
    {
        return rc;
    }
    if(serdesNumPerPort == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "internal_cpssDxChPortSerdesTuningSet return code is [%d], serdesNumPerPort is 0", rc);
    }
    lanesNumInDev = prvCpssDxChHwInitNumOfSerdesGet(devNum);

    if(startSerdes+laneNum >= lanesNumInDev)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr)
        goto getDefaults;

    serdesNumInDevice = startSerdes + laneNum;
    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        serdesNumInDevice += portGroupId*lanesNumInDev;
    }

    if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice])
        goto getDefaults;

    *tuneValuesPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr
                                        [serdesNumInDevice][serdesFrequency];

    return GT_OK;

getDefaults:

    if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ||
        (CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ||
        (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        MV_HWS_AUTO_TUNE_RESULTS results;

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesAutoTuneResult(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *results)", devNum, portGroupId, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum));
        rc = mvHwsSerdesAutoTuneResult(devNum, portGroupId, startSerdes+laneNum,
                                        HWS_DEV_SERDES_TYPE(devNum),
                                        &results);
        if(rc != GT_OK)
                {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
                }

        tuneValuesPtr->sqlch    = results.sqleuch;
        tuneValuesPtr->ffeC     = results.ffeC;
        tuneValuesPtr->ffeR     = results.ffeR;
        tuneValuesPtr->sampler  = results.sampler;
        tuneValuesPtr->txEmphAmp = results.txEmph0;
        tuneValuesPtr->txAmp    = results.txAmp;
        tuneValuesPtr->txEmph1  = results.txEmph1;
        cpssOsMemCpy(tuneValuesPtr->dfeValsArray, results.dfeVals,
                     sizeof(GT_32)*CPSS_DXCH_PORT_DFE_VALUES_ARRAY_SIZE_CNS);

        if(PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum))
        {
            tuneValuesPtr->txAmpAdj = results.txAmpAdj;
            tuneValuesPtr->txAmpShft  = results.txAmpShft;
            tuneValuesPtr->txEmphEn1  = results.txEmph1En;
            tuneValuesPtr->txEmphEn  = results.txEmph0En;
        }
    }
    else
    {
        rc = serDesConfig(devNum,&serdesPowerUpSequencePtr,&serdesExtCfgPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        serdesConfig[startSerdes+laneNum].receiverReg0;
        if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                        0, 4, &tuneValuesPtr->sqlch)) != GT_OK)
            return rc;

        if(CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            tuneValuesPtr->dfe = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                            [PRV_CPSS_DXCH_PORT_SERDES_DFE_REG0_INDEX_E]
                                            [serdesFrequency],0,5);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                            serdesConfig[startSerdes+laneNum].dfeF0F1CoefCtrl;
            if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                            0, 5, &tuneValuesPtr->dfe)) != GT_OK)
                return rc;
        }

        tuneValuesPtr->ffeC = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_INDEX_E]
                                        [serdesFrequency],0,4);

        tuneValuesPtr->ffeR = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_INDEX_E]
                                        [serdesFrequency],4,3);

        tuneValuesPtr->ffeS = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_INDEX_E]
                                        [serdesFrequency],8,2);

        valueIndex = (CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
                    ? PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_LION_INDEX_E :
                        PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_XCAT_INDEX_E;
        tuneValuesPtr->sampler = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr[valueIndex]
                                        [serdesFrequency],8,7);

        tuneValuesPtr->txEmphAmp    = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_INDEX_E]
                                        [serdesFrequency],0,4);
        tuneValuesPtr->txAmp        = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_INDEX_E]
                                        [serdesFrequency],8,5);
        tuneValuesPtr->txAmpAdj     = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_INDEX_E]
                                        [serdesFrequency],13,1);

        tuneValuesPtr->txEmphEn     = BIT2BOOL_MAC(U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_INDEX_E]
                                        [serdesFrequency],15,1));
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesTuningGet function
* @endinternal
*
* @brief   SerDes fine tuning values get.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure to put tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reads values saved in SW DB by cpssDxChPortSerdesTuningSet or
*       if it was not called, from default matrix (...SerdesPowerUpSequence).
*
*/
GT_STATUS cpssDxChPortSerdesTuningGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesTuningGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesFrequency, tuneValuesPtr));

    rc = internal_cpssDxChPortSerdesTuningGet(devNum, portNum, laneNum, serdesFrequency, tuneValuesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesFrequency, tuneValuesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesLaneTuningSet function
* @endinternal
*
* @brief   SerDes lane fine tuning values set.
*         APPLICABLE DEVICES:
*         xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*         INPUTS:
*         devNum  - physical device number
*         portGroupNum - number of port group (local core), not used for non-multi-core
*         laneNum  - number of SERDES lane where values in tuneValuesPtr appliable
*         serdesFrequency - serdes speed for which to save tuning values
*         tuneValuesPtr  - (ptr to) structure with tuned values
*         txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*         In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*         txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*         APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*         emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*         emph1 - Controls the emphasis amplitude for Gen1 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*         txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*                                      txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*                                      txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*                                      emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*                                      emph1 - Controls the emphasis amplitude for Gen1 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*                                      txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set tuning parameters for lane in SW DB, then
*       cpssDxChPortSerdesPowerStatusSet or cpssDxChPortModeSpeedSet will write
*       them to HW.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesLaneTuningSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroupNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_U32      lanesNumInDev; /* number of serdes lanes in current device */
    GT_U32      laneNumInMultiCore;/* plane serdes lane number in
                                                    multi-port-group device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E |
                                CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    if(0 == (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp
             & (1<<(portGroupNum))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    lanesNumInDev = prvCpssDxChHwInitNumOfSerdesGet(devNum);

    if(laneNum >= lanesNumInDev)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((serdesFrequency < CPSS_DXCH_PORT_SERDES_SPEED_1_25_E) ||
       (serdesFrequency >= CPSS_DXCH_PORT_SERDES_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* for Caelum (Bobk) device and above */
    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        tuneValuesPtr->txAmpAdj = GT_FALSE;
        tuneValuesPtr->txAmpShft = GT_FALSE;

        if (tuneValuesPtr->txAmp >  31 ||
            tuneValuesPtr->sqlch > 310 ||
            tuneValuesPtr->DC    > 255 ||
            tuneValuesPtr->LF    >  15 ||
            tuneValuesPtr->HF    >  15 ||
            tuneValuesPtr->BW    >  15 ||
            tuneValuesPtr->LB    >  15)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if ((tuneValuesPtr->txEmphAmp > 31) || (tuneValuesPtr->txEmph1 > 31))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if (((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
            (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)          ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
        {
            if ((tuneValuesPtr->txEmphAmp < -31) || (tuneValuesPtr->txEmph1 < -31))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
        }

        if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* Assuming FALCON INCLUDED */
        {
            if ((tuneValuesPtr->txEmphAmp < 0) || (tuneValuesPtr->txEmph1 < 0))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
        }
    }
    else if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* for now just these parameters supported in Lion2 */
        if (tuneValuesPtr->txEmphAmp    >= BIT_4 ||
            tuneValuesPtr->txEmph1      >= BIT_4 ||
            tuneValuesPtr->txAmp        >= BIT_5 ||
            tuneValuesPtr->txAmpAdj     >= BIT_1 ||
            tuneValuesPtr->ffeC         >= BIT_4 ||
            tuneValuesPtr->ffeR         >= BIT_3 ||
            tuneValuesPtr->align90      >= BIT_7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (tuneValuesPtr->txEmphAmp    >= BIT_4 ||
            tuneValuesPtr->txAmp        >= BIT_5 ||
            tuneValuesPtr->txAmpAdj     >= BIT_1 ||
            tuneValuesPtr->ffeC         >= BIT_4 ||
            tuneValuesPtr->ffeR         >= BIT_3 ||
            tuneValuesPtr->ffeS         >= BIT_2 ||
            tuneValuesPtr->dfe          >= BIT_5 ||
            tuneValuesPtr->sampler      >= BIT_7 ||
            tuneValuesPtr->sqlch        >= BIT_4)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    /* if DB not allocated yet' lets allocate it */
    if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr)
    {
        GT_U32 memSize;

        if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            memSize = sizeof(CPSS_DXCH_PORT_SERDES_TUNE_STC_PTR) * lanesNumInDev
                             * PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.numOfPortGroups;
        }
        else
        {
            memSize = sizeof(CPSS_DXCH_PORT_SERDES_TUNE_STC_PTR) * lanesNumInDev;
        }
        PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr =
                        (CPSS_DXCH_PORT_SERDES_TUNE_STC_PTR*)cpssOsMalloc(memSize);
        if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        cpssOsMemSet(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr, 0, memSize);
    }

    laneNumInMultiCore = laneNum;
    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        laneNumInMultiCore += portGroupNum*lanesNumInDev;
    }

    /* if entry for required lane not allocated yet, lets allocate it */
    if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore])
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore] =
            (CPSS_DXCH_PORT_SERDES_TUNE_STC_PTR)cpssOsMalloc(
                sizeof(CPSS_DXCH_PORT_SERDES_TUNE_STC)*CPSS_DXCH_PORT_SERDES_SPEED_NA_E);
        if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore])
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        cpssOsMemSet(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore], 0,
                    sizeof(CPSS_DXCH_PORT_SERDES_TUNE_STC)*CPSS_DXCH_PORT_SERDES_SPEED_NA_E);
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore]
                                            [serdesFrequency] = *tuneValuesPtr;

    SERDES_DBG_DUMP_MAC(("dfe=0x%x,ffeR=0x%x,ffeC=0x%x,sampler=0x%x,sqlch=0x%x,\
                    txEmphAmp=0x%x,txAmp=0x%x,txAmpAdj=0x%x,ffeS=0x%x,txEmphEn=0x%x\n",
                PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore]
                                                                [serdesFrequency].dfe,
                PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore]
                                                                [serdesFrequency].ffeR,
                PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore]
                                                                [serdesFrequency].ffeC,
                PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore]
                                                                [serdesFrequency].sampler,
                PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore]
                                                                [serdesFrequency].sqlch,
                PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore]
                                                                [serdesFrequency].txEmphAmp,
                PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore]
                                                                [serdesFrequency].txAmp,
                PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore]
                                                                [serdesFrequency].txAmpAdj,
                PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore]
                                                                [serdesFrequency].ffeS,
                PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore]
                                                                [serdesFrequency].txEmphEn));

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesLaneTuningSet function
* @endinternal
*
* @brief   SerDes lane fine tuning values set.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set tuning parameters for lane in SW DB, then
*       cpssDxChPortSerdesPowerStatusSet or cpssDxChPortModeSpeedSet will write
*       them to HW.
*
*/
GT_STATUS cpssDxChPortSerdesLaneTuningSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroupNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesLaneTuningSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr));

    rc = internal_cpssDxChPortSerdesLaneTuningSet(devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesLaneTuningGet function
* @endinternal
*
* @brief   Get SerDes lane fine tuning values.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get tuning parameters for lane from SW DB.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesLaneTuningGet
(
    IN   GT_U8                   devNum,
    IN   GT_U32                  portGroupNum,
    IN   GT_U32                  laneNum,
    IN   CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT  CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS   rc;   /* return code */
    GT_U32      lanesNumInDev; /* number of serdes lanes in current device */
    GT_U32      laneNumInMultiCore;/* plane serdes lane number in
                                                    multi-port-group device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E |
                                CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    if(0 == (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp
             & (1<<(portGroupNum))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    lanesNumInDev = prvCpssDxChHwInitNumOfSerdesGet(devNum);

    if(laneNum >= lanesNumInDev)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((serdesFrequency < CPSS_DXCH_PORT_SERDES_SPEED_1_25_E) ||
       (serdesFrequency >= CPSS_DXCH_PORT_SERDES_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    laneNumInMultiCore = laneNum;
    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        laneNumInMultiCore += portGroupNum*lanesNumInDev;
    }

    if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr)
        goto getDefaults;

    if(NULL == PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNumInMultiCore])
        goto getDefaults;

    *tuneValuesPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr
                                        [laneNumInMultiCore][serdesFrequency];

    return GT_OK;

getDefaults:
    /* Defaults parameters get from current configuration - not according to serdesFrequency
    TBD - Defaults parameters should be get according to serdesFrequency */
    if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ||
        (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        MV_HWS_AUTO_TUNE_RESULTS results;


        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesAutoTuneResult(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *results)", devNum, portGroupNum, laneNum, HWS_DEV_SERDES_TYPE(devNum));
        rc = mvHwsSerdesAutoTuneResult(devNum, portGroupNum, laneNum,
                                        HWS_DEV_SERDES_TYPE(devNum),
                                        &results);
        if(rc != GT_OK)
                {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
                }

        tuneValuesPtr->sqlch    = results.sqleuch;
        tuneValuesPtr->ffeC     = results.ffeC;
        tuneValuesPtr->ffeR     = results.ffeR;
        tuneValuesPtr->sampler  = results.sampler;
        tuneValuesPtr->txEmphAmp = results.txEmph0;
        tuneValuesPtr->txAmp    = results.txAmp;
        tuneValuesPtr->txEmph1  = results.txEmph1;
        cpssOsMemCpy(tuneValuesPtr->dfeValsArray, results.dfeVals,
                     sizeof(GT_32)*CPSS_DXCH_PORT_DFE_VALUES_ARRAY_SIZE_CNS);

        if(PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum))
        {
            tuneValuesPtr->txAmpAdj = results.txAmpAdj;
            tuneValuesPtr->txAmpShft  = results.txAmpShft;
            tuneValuesPtr->txEmphEn1  = results.txEmph1En;
            tuneValuesPtr->txEmphEn  = results.txEmph0En;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
/* TBD - sfould be implemented code to get default serdes configuration parametwers */
#ifdef DEF_SERDES_PARAMS
        GT_U32  regAddr;
        PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY *serdesPowerUpSequencePtr;
                                        /* pointer to serdes power up sequence */
        PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_ARRAY  *serdesExtCfgPtr; /* pointer to
            external registers serdes power up configuration (just dummy here) */
        GT_U32  valueIndex;

        rc = serDesConfig(devNum,&serdesPowerUpSequencePtr,&serdesExtCfgPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(0 == PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            portGroupNum = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        serdesConfig[laneNum].receiverReg0;
        if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupNum, regAddr,
                                                        0, 4, &tuneValuesPtr->sqlch)) != GT_OK)
            return rc;

        if(CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            tuneValuesPtr->dfe = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                            [PRV_CPSS_DXCH_PORT_SERDES_DFE_REG0_INDEX_E]
                                            [serdesFrequency],0,5);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                            serdesConfig[laneNum].dfeF0F1CoefCtrl;
            if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupNum, regAddr,
                                                            0, 5, &tuneValuesPtr->dfe)) != GT_OK)
                return rc;
        }

        tuneValuesPtr->ffeC = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_INDEX_E]
                                        [serdesFrequency],0,4);

        tuneValuesPtr->ffeR = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_INDEX_E]
                                        [serdesFrequency],4,3);

        tuneValuesPtr->ffeS = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_INDEX_E]
                                        [serdesFrequency],8,2);

        valueIndex = (CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
                    ? PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_LION_INDEX_E :
                        PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_XCAT_INDEX_E;
        tuneValuesPtr->sampler = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr[valueIndex]
                                        [serdesFrequency],8,7);

        tuneValuesPtr->txEmphAmp    = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_INDEX_E]
                                        [serdesFrequency],0,4);
        tuneValuesPtr->txAmp        = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_INDEX_E]
                                        [serdesFrequency],8,5);
        tuneValuesPtr->txAmpAdj     = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_INDEX_E]
                                        [serdesFrequency],13,1);

        tuneValuesPtr->txEmphEn     = BIT2BOOL_MAC(U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_INDEX_E]
                                        [serdesFrequency],15,1));
#endif
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesLaneTuningGet function
* @endinternal
*
* @brief   Get SerDes lane fine tuning values.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get tuning parameters for lane from SW DB.
*
*/
GT_STATUS cpssDxChPortSerdesLaneTuningGet
(
    IN   GT_U8                   devNum,
    IN   GT_U32                  portGroupNum,
    IN   GT_U32                  laneNum,
    IN   CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT  CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesLaneTuningGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr));

    rc = internal_cpssDxChPortSerdesLaneTuningGet(devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesConfigGet function
* @endinternal
*
* @brief   Get SERDES configuration per port.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] serdesCfgPtr             - pointer to port serdes configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesConfigGet
(
    IN GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PORT_SERDES_CONFIG_STC  *serdesCfgPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_STATUS   rc;         /* return code */
    GT_U32      serdes, serdesesNum;     /* first serdes number, serdes quantity */
    GT_U32      portGroupId;     /*the port group Id - support multi-port-groups device */
    GT_U32       localPort;  /* local port - support multi-port-groups device */
    GT_U32      firstInitializedSerdes;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;    /* port interface mode */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT3_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(serdesCfgPtr);
    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portNum].serdesTxConfReg1[0];

        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Tx Driver output amplitude */
        serdesCfgPtr->txAmp = (regValue >> 1) & 0x1f;
        /* Pre- or de-emphasis enable signal */
        serdesCfgPtr->txEmphEn = BIT2BOOL_MAC(((regValue >> 7) & 0x1));
        /* emphasis type (pre/de-emphasis) */
        serdesCfgPtr->txEmphType = BIT2BOOL_MAC(((regValue >> 8) & 0x1));
        /* Emphasis level control bits */
        serdesCfgPtr->txEmphAmp = (regValue >> 10) & 0xf;
    }
    else /* xCat and above */
    {
        if((rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode)) != GT_OK)
            return rc;

        if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
        {
            if(((ifMode == CPSS_PORT_INTERFACE_MODE_XGMII_E) && (localPort%2 != 0)) ||
                ((ifMode == CPSS_PORT_INTERFACE_MODE_HX_E) && (localPort%2 != 0)) ||
                ((ifMode == CPSS_PORT_INTERFACE_MODE_XLG_E) && (localPort != 10)))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if((rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,&serdes,&serdesesNum)) != GT_OK)
            return rc;

        rc = prvCpssDxChLpGetFirstInitSerdes(devNum,portGroupId,serdes,&firstInitializedSerdes);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* get Transmitter Register 1 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            serdesConfig[firstInitializedSerdes].transmitterReg1;

        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,regAddr, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        serdesCfgPtr->txEmphAmp = regValue & 0xf;
        serdesCfgPtr->txAmp = (regValue >> 8) & 0x1f;
        serdesCfgPtr->txAmpAdj = (regValue >> 13) & 0x1;

        /* get FFE Register 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            serdesConfig[firstInitializedSerdes].ffeReg;

        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,regAddr, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        serdesCfgPtr->ffeCapacitorSelect = regValue & 0xf;
        serdesCfgPtr->ffeResistorSelect = (regValue >> 4) & 0x7;
        serdesCfgPtr->ffeSignalSwingControl = (regValue >> 8) & 0x3;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    serdesConfig[firstInitializedSerdes].transmitterReg0;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 15, 1, &regValue);
        if(rc != GT_OK)
        {
            return rc;
        }
        serdesCfgPtr->txEmphEn = BIT2BOOL_MAC(regValue);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesConfigGet function
* @endinternal
*
* @brief   Get SERDES configuration per port.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] serdesCfgPtr             - pointer to port serdes configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*
*/
GT_STATUS cpssDxChPortSerdesConfigGet
(
    IN GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PORT_SERDES_CONFIG_STC  *serdesCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesCfgPtr));

    rc = internal_cpssDxChPortSerdesConfigGet(devNum, portNum, serdesCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] startSerdes              - first SERDES number
* @param[in] serdesesNum              - number of SERDESes
* @param[in] state                    - Reset state
*                                      GT_TRUE  - Port SERDES is under Reset
*                                      GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortSerdesResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32    startSerdes,
    IN  GT_U32    serdesesNum,
    IN  GT_BOOL   state
)
{
    GT_U32    regValue;       /* register field value */
    GT_U32    regAddr;        /* register address */
    GT_U32    fieldOffset;    /* register field offset */
    GT_U32    i;              /* iterator */
    GT_U32    portGroupId;    /*the port group Id - support multi-port-groups device */
    GT_U32    localPort;
    GT_STATUS rc;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
       (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        GT_U32 portGroupId; /* port group number of configured port */
        GT_U32 i; /* iterator on serdeses */

        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portNum);

        for(i = startSerdes; i < startSerdes+serdesesNum; i++)
        {

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesReset(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], analogReset[%d], digitalReset[%d], syncEReset[%d])", devNum,portGroupId, i, HWS_DEV_SERDES_TYPE(devNum)/*Lion2/3 serdes type*/, state/*analogReset*/, state/*digitalReset*/, state/*syncEReset*/);
            rc = mvHwsSerdesReset(devNum,portGroupId, i,
                                    HWS_DEV_SERDES_TYPE(devNum)/*Lion2/3 serdes type*/,
                                    state/*analogReset*/,
                                    state/*digitalReset*/,
                                    state/*syncEReset*/);

            if(rc != GT_OK)
                        {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
                        }
        }

        return GT_OK;
    }

    regValue = (state == GT_TRUE) ? 0 : 1;
    fieldOffset = 3; /* sd_reset_in */

    for (i = startSerdes; i < startSerdes + serdesesNum; i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].serdesExternalReg2;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId , regAddr, fieldOffset, 1, regValue);
        if(rc !=GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        if((CPSS_PORT_INTERFACE_MODE_XGMII_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portNum))
            && (CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portNum+1))
            && (state == GT_TRUE))
        {   /* each bit in Serdes miscellaneous reg. resets all lanes of port accordingly to current
             * interface mode 4 for XAUI, 2 for RXAUI etc., so lets fake here 2 serdes mode
             */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.mediaInterface;
            if (prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, startSerdes, serdesesNum,
                                                    BIT_MASK_MAC(serdesesNum)) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,20);
        }

        /* Set Serdes reset latency */
        for (i = startSerdes; i < startSerdes + serdesesNum; i++)
        {
            regAddr = 0x9800010 + 0x400*i;
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId , regAddr, 8, 1, 1);
            if(rc !=GT_OK)
            {
                return rc;
            }
        }

        HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,10);

        /* sd_reset using Serdes miscellaneous register */
        regAddr = (localPort < 6) ? 0x9800010 : 0x9803010;
        /* if number of lanes is 8 we are in XLG mode - use bit 6 */
        fieldOffset = (serdesesNum < 8) ? (localPort % 6) : 6;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId , regAddr, fieldOffset, 1, regValue);
        if(rc !=GT_OK)
        {
            return rc;
        }

        HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,20);

        if((CPSS_PORT_INTERFACE_MODE_XGMII_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portNum))
            && (CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portNum+1))
            && (state == GT_TRUE))
        {/* restore original XAUI media interface mode */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.mediaInterface;
            if (prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, startSerdes, serdesesNum,
                                                    0) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }

    HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,20);

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port SERDES is under Reset
*                                      GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortSerdesResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
)
{
    GT_U32    startSerdes = 0, serdesesNum = 0;
    GT_STATUS rc;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    GT_U32    portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode)) != GT_OK)
        return rc;

    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,&startSerdes,&serdesesNum)) != GT_OK)
        return rc;

    return prvCpssDxChPortSerdesResetStateSet(devNum,portMacMap,startSerdes,serdesesNum,state);
}

/**
* @internal cpssDxChPortSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port SERDES is under Reset
*                                      GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSerdesResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesResetStateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssDxChPortSerdesResetStateSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesAutoTune function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note Directions for TX training execution:
*       1. As a pre-condition, ports on both sides of the link must be UP.
*       2. For 2 sides of the link call execute TX_TRAINING_CFG;
*       No need to maintain special timing sequence between them.
*       The CFG phase sets some parameters at the SerDes as a preparation
*       to the training phase.
*       3. After CFG is done, for both sides of the link, call TX_TRAINING_START;
*       No need to maintain special timing sequence between them.
*       4. Wait at least 0.5 Sec. (done by API inside).
*       5. Verify Training status by calling TX_TRAINING_STATUS.
*       No need to maintain special timing sequence between them.
*       This call provides the training status (OK/Failed) and terminates it.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesAutoTune
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup;  /* local core number */
    GT_U32                  phyPortNum; /* number of port in local core */
    MV_HWS_PORT_STANDARD    hwsPortMode;/* current ifMode of port in HWS format */
    GT_U32                                        optAlgoMask;/* bitmap of optimization algorithms */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboParamsPtr;/* parameters of combo port */
    CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;    /* current interface of port */
    MV_HWS_AUTO_TUNE_STATUS_RES tuneRes; /*status parmeter needed for some portTuningMode*/
    CPSS_PORTS_BMP_STC      *xlgUniDirPortsBmpPtr;
    GT_U32                  portXlgUnidir;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    xlgUniDirPortsBmpPtr =
        &(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
        info_PRV_CPSS_DXCH_PORTS_XLG_UNIDIRECTIONAL_WA_E.xlgUniDirPortsBmp);

    portXlgUnidir = CPSS_PORTS_BMP_IS_PORT_SET_MAC(xlgUniDirPortsBmpPtr, portNum);
    cpssIfMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);

    comboParamsPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portMacMap];
    hwsPortMode = NON_SUP_MODE;
    if(comboParamsPtr->macArray[0].macNum != CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
    { /* if it's combo port */
        CPSS_DXCH_PORT_MAC_PARAMS_STC   mac;
        rc = cpssDxChPortComboPortActiveMacGet(devNum, portNum, &mac);
        if(rc != GT_OK)
            return rc;
        if (mac.macNum != phyPortNum)
        {
            switch(cpssIfMode)
            {
                case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
                case CPSS_PORT_INTERFACE_MODE_KR_E:
                    hwsPortMode = _100GBase_KR10;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }
    }

    if(NON_SUP_MODE == hwsPortMode)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,cpssIfMode,
                                    PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap),
                                    &hwsPortMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("portMacMap=%d,ifMode=%d,speed=%d\n", portMacMap, cpssIfMode, PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap));
            return rc;
        }
    }

    optAlgoMask = PRV_CPSS_DXCH_PORT_SD_OPT_ALG_BMP_MAC(devNum, portMacMap);
    if(CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALL_E == optAlgoMask)
    {
        optAlgoMask = (CPSS_PORT_SERDES_TRAINING_OPTIMISATION_DFE_E
                                  | CPSS_PORT_SERDES_TRAINING_OPTIMISATION_FFE_E
                                  | CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALIGN90_E);
    }

    if (portXlgUnidir)
    {
        switch(portTuningMode)
        {
            case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E:
                /* protect MAC Reset under traffic and Disable Unidirectional. */
                rc = cpssDxChPortEnableSet(devNum, portNum, GT_FALSE);
                if (rc != GT_OK)
                {
                    return rc;
                }
                break;
            default: break;
        }
    }

    switch(portTuningMode)
    {
        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, TRxTuneCfg, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                            TRxTuneCfg, optAlgoMask, &tuneRes);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("phyPortNum=%d,hwsPortMode=%d,speed=%d\n", phyPortNum, hwsPortMode);
            }
            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, TRxTuneStart, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                            TRxTuneStart, optAlgoMask, &tuneRes);
            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, TRxTuneStatus, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                            TRxTuneStatus, optAlgoMask, &tuneRes);
            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, RxTrainingOnly, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                            RxTrainingOnly, optAlgoMask, &tuneRes);
            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneStop(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], stopRx[%d], stopTx[%d])", devNum,portGroup,phyPortNum,hwsPortMode,GT_FALSE,GT_TRUE);
            rc = mvHwsPortAutoTuneStop(devNum,portGroup,phyPortNum,hwsPortMode,GT_FALSE,GT_TRUE);
            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E:
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, RxTrainingAdative, optAlgoMask);
                rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                                RxTrainingAdative, optAlgoMask, &tuneRes);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E:
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) /* Assuming FALCON INCLUDED */
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, RxTrainingStopAdative, optAlgoMask);
                rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                                RxTrainingStopAdative, optAlgoMask, &tuneRes);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENHANCE_E:
            if(!(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            rc = cpssDxChPortSerdesEnhancedAutoTune(devNum,portNum,0,15);
            break;
        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_WAIT_FOR_FINISH_E:
            if(!(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "blocking rx-training not supported for device");
            }
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, RxTrainingWaitForFinish, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                    RxTrainingWaitForFinish, optAlgoMask, &tuneRes);
            break;
        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_VSR_E:
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) /* Assuming FALCON INCLUDED */
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, RxTrainingVsr, optAlgoMask);
                rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                                RxTrainingVsr, optAlgoMask, &tuneRes);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssDxChPortSerdesAutoTune function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note Directions for TX training execution:
*       1. As a pre-condition, ports on both sides of the link must be UP.
*       2. For 2 sides of the link call execute TX_TRAINING_CFG;
*       No need to maintain special timing sequence between them.
*       The CFG phase sets some parameters at the SerDes as a preparation
*       to the training phase.
*       3. After CFG is done, for both sides of the link, call TX_TRAINING_START;
*       No need to maintain special timing sequence between them.
*       4. Wait at least 0.5 Sec. (done by API inside).
*       5. Verify Training status by calling TX_TRAINING_STATUS.
*       No need to maintain special timing sequence between them.
*       This call provides the training status (OK/Failed) and terminates it.
*
*/
GT_STATUS cpssDxChPortSerdesAutoTune
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesAutoTune);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portTuningMode));

    rc = internal_cpssDxChPortSerdesAutoTune(devNum, portNum, portTuningMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portTuningMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesAlign90StateSet function
* @endinternal
*
* @brief   Start/stop align 90 algorithm on given port's serdes.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE - start
*                                      GT_FALSE - stop
* @param[in,out] serdesParamsPtr          - (ptr to) serdes parameters
* @param[in,out] serdesParamsPtr          - (ptr to) serdes parameters at start
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssDxChPortSerdesAlign90StateSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    INOUT CPSS_DXCH_PORT_ALIGN90_PARAMS_STC *serdesParamsPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup;  /* local core number */
    GT_U32                  phyPortNum; /* number of port in local core */
    MV_HWS_PORT_STANDARD    hwsPortMode;/* current ifMode of port in HWS format */
    GT_U32 portMacNum;      /* MAC number */

    CPSS_NULL_PTR_CHECK_MAC(serdesParamsPtr);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum),
                                PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum),
                                &hwsPortMode);
    if(rc != GT_OK)
        return rc;

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    if(state)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFixAlign90Start(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], *serdesParams{startAlign90[%d], rxTrainingCfg[%d], osDeltaMax[%d], adaptedFfeR[%d], adaptedFfeC[%d]})", devNum, portGroup, phyPortNum, hwsPortMode, serdesParamsPtr->startAlign90, serdesParamsPtr->rxTrainingCfg, serdesParamsPtr->osDeltaMax, serdesParamsPtr->adaptedFfeR, serdesParamsPtr->adaptedFfeC);
        rc = mvHwsPortFixAlign90Start(devNum, portGroup, phyPortNum, hwsPortMode,
                                        (MV_HWS_ALIGN90_PARAMS*)serdesParamsPtr);
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFixAlign90Stop(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], *serdesParams{startAlign90[%d], rxTrainingCfg[%d], osDeltaMax[%d], adaptedFfeR[%d], adaptedFfeC[%d]})", devNum, portGroup, phyPortNum, hwsPortMode, serdesParamsPtr->startAlign90, serdesParamsPtr->rxTrainingCfg, serdesParamsPtr->osDeltaMax, serdesParamsPtr->adaptedFfeR, serdesParamsPtr->adaptedFfeC);
        rc = mvHwsPortFixAlign90Stop(devNum, portGroup, phyPortNum, hwsPortMode,
                                        (MV_HWS_ALIGN90_PARAMS*)serdesParamsPtr);
    }
        if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    return rc;
}

/**
* @internal cpssDxChPortSerdesAlign90StateSet function
* @endinternal
*
* @brief   Start/stop align 90 algorithm on given port's serdes.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE - start
*                                      GT_FALSE - stop
* @param[in,out] serdesParamsPtr          - (ptr to) serdes parameters
* @param[in,out] serdesParamsPtr          - (ptr to) serdes parameters at start
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortSerdesAlign90StateSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    INOUT CPSS_DXCH_PORT_ALIGN90_PARAMS_STC *serdesParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesAlign90StateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state, serdesParamsPtr));

    rc = internal_cpssDxChPortSerdesAlign90StateSet(devNum, portNum, state, serdesParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state, serdesParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesAlign90StatusGet function
* @endinternal
*
* @brief   Get status of align 90 algorithm on given port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statusPtr                - (ptr to) serdes parameters at start
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssDxChPortSerdesAlign90StatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *statusPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup;  /* local core number */
    GT_U32                  phyPortNum; /* number of port in local core */
    MV_HWS_PORT_STANDARD    hwsPortMode;/* current ifMode of port in HWS format */
    GT_U32 portMacNum;      /* MAC number */

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum),
                                PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum),
                                &hwsPortMode);
    if(rc != GT_OK)
        return rc;

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFixAlign90Status(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], *statusPtr)", devNum, portGroup, phyPortNum, hwsPortMode, (MV_HWS_AUTO_TUNE_STATUS*)statusPtr);
    rc = mvHwsPortFixAlign90Status(devNum, portGroup, phyPortNum, hwsPortMode,
                                    (MV_HWS_AUTO_TUNE_STATUS*)statusPtr);

        if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    return rc;
}

/**
* @internal cpssDxChPortSerdesAlign90StatusGet function
* @endinternal
*
* @brief   Get status of align 90 algorithm on given port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statusPtr                - (ptr to) serdes parameters at start
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortSerdesAlign90StatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *statusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesAlign90StatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statusPtr));

    rc = internal_cpssDxChPortSerdesAlign90StatusGet(devNum, portNum, statusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesAutoTuneExt function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes including required
*         optimizations.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number, device, serdesOptAlgBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note See cpssDxChPortSerdesAutoTune.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesAutoTuneExt
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode,
    IN  GT_U32                                   serdesOptAlgBmp
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup;  /* local core number */
    GT_U32                  phyPortNum; /* number of port in local core */
    MV_HWS_PORT_STANDARD    hwsPortMode;/* current ifMode of port in HWS format */
    GT_U32                                        optAlgoMask;/* bitmap of optimization algorithms */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    MV_HWS_AUTO_TUNE_STATUS_RES tuneRes; /*status parmeter needed for some portTuningMode*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_XCAT3_E
                                             );
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap),
                                PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap),
                                &hwsPortMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacMap);

    if(CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALL_E == serdesOptAlgBmp)
    {
        optAlgoMask = (CPSS_PORT_SERDES_TRAINING_OPTIMISATION_DFE_E
                              | CPSS_PORT_SERDES_TRAINING_OPTIMISATION_FFE_E
                              | CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALIGN90_E);
    }
    else
    {
        optAlgoMask = serdesOptAlgBmp;
    }

    switch(portTuningMode)
    {
        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, TRxTuneCfg, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                TRxTuneCfg, optAlgoMask, &tuneRes);
            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)" ,devNum, portGroup, phyPortNum, hwsPortMode, TRxTuneStart, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                TRxTuneStart, optAlgoMask, &tuneRes);
            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, TRxTuneStatus, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                TRxTuneStatus, optAlgoMask, &tuneRes);
            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, RxTrainingOnly, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                            RxTrainingOnly, optAlgoMask, &tuneRes);
        break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneStop(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], stopRx[%d], stopTx[%d])", devNum, portGroup, phyPortNum, hwsPortMode, GT_FALSE, GT_TRUE);
            rc = mvHwsPortAutoTuneStop(devNum, portGroup, phyPortNum, hwsPortMode,
                                       GT_FALSE, GT_TRUE);
            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E:
            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
               ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
                (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)))
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, RxTrainingAdative, optAlgoMask);
                rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                                RxTrainingAdative, optAlgoMask, &tuneRes);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E:
            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
               ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
                (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)))
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, RxTrainingStopAdative, optAlgoMask);
                rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                                RxTrainingStopAdative, optAlgoMask, &tuneRes);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_VSR_E:
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) /* Assuming FALCON INCLUDED */
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, phyPortNum, hwsPortMode, RxTrainingVsr, optAlgoMask);
                rc = mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, hwsPortMode,
                                                                RxTrainingVsr, optAlgoMask, &tuneRes);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
        if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    return rc;
}

/**
* @internal cpssDxChPortSerdesAutoTuneExt function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes including required
*         optimizations.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number, device, serdesOptAlgBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note See cpssDxChPortSerdesAutoTune.
*
*/
GT_STATUS cpssDxChPortSerdesAutoTuneExt
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode,
    IN  GT_U32                                   serdesOptAlgBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesAutoTuneExt);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portTuningMode, serdesOptAlgBmp));

    rc = internal_cpssDxChPortSerdesAutoTuneExt(devNum, portNum, portTuningMode, serdesOptAlgBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portTuningMode, serdesOptAlgBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesAutoTuneStatusGet function
* @endinternal
*
* @brief   Get current status of RX and TX serdes auto-tuning on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] rxTuneStatusPtr          - RX tuning status
* @param[out] txTuneStatusPtr          - TX tuning status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDxChPortSerdesAutoTuneStatusGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
)
{
    GT_STATUS               rc;         /* return code */
    MV_HWS_PORT_STANDARD    hwsPortMode;/* current ifMode of port in HWS format */
    GT_U32                  localPort;  /* number of port in local core */
    GT_U32                  portGroupId;/* core number  */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    CPSS_NULL_PTR_CHECK_MAC(rxTuneStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(txTuneStatusPtr);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap),
                                PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap),
                                &hwsPortMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneStateCheck(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], *rxTune, *txTune)", devNum, portGroupId, localPort, hwsPortMode);
    rc = mvHwsPortAutoTuneStateCheck(devNum, portGroupId, localPort, hwsPortMode,
                                    (MV_HWS_AUTO_TUNE_STATUS*)rxTuneStatusPtr,
                                    (MV_HWS_AUTO_TUNE_STATUS*)txTuneStatusPtr);

        if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
        return rc;
}

/**
* @internal cpssDxChPortSerdesAutoTuneStatusGet function
* @endinternal
*
* @brief   Get current status of RX and TX serdes auto-tuning on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; xCat3;.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] rxTuneStatusPtr          - RX tuning status
* @param[out] txTuneStatusPtr          - TX tuning status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPortSerdesAutoTuneStatusGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesAutoTuneStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, rxTuneStatusPtr, txTuneStatusPtr));

    rc = internal_cpssDxChPortSerdesAutoTuneStatusGet(devNum, portNum, rxTuneStatusPtr, txTuneStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, rxTuneStatusPtr, txTuneStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

GT_STATUS prvCpssDxChPortSerdesAutoTuneStatusGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
)
{
    GT_STATUS rc;

    rc = cpssDxChPortSerdesAutoTuneStatusGet(devNum,
                                portNum,
                                (CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *)rxTuneStatusPtr,
                                (CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *)txTuneStatusPtr);

    return rc;
}


/**
* @internal internal_cpssDxChPortSerdesLanePolarityGet function
* @endinternal
*
* @brief   Get the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3; Bobcat2;
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
*
* @param[out] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[out] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get the Tx/Rx polarity parameters for lane from HW if serdes initialized
*       or from SW DB if serdes was not initialized
*
*/
static GT_STATUS internal_cpssDxChPortSerdesLanePolarityGet
(
    IN   GT_U8      devNum,
    IN   GT_U32     portGroupNum,
    IN   GT_U32     laneNum,
    OUT  GT_BOOL    *invertTx,
    OUT  GT_BOOL    *invertRx
)
{
    GT_STATUS   rc;
    GT_U32      lanesNumInDev;      /* number of serdes lanes in current device */
    GT_U32      laneNumInMultiCore; /* plane serdes lane number in multi-port-group device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
                                          CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(invertTx);
    CPSS_NULL_PTR_CHECK_MAC(invertRx);

    if(0 == (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (1<<(portGroupNum))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    lanesNumInDev = prvCpssDxChHwInitNumOfSerdesGet(devNum);

    if(laneNum >= lanesNumInDev)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    laneNumInMultiCore = laneNum;
    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        laneNumInMultiCore += portGroupNum * lanesNumInDev;
    }

    rc = mvHwsSerdesPolarityConfigGet(devNum, portGroupNum, laneNumInMultiCore, HWS_DEV_SERDES_TYPE(devNum), invertTx, invertRx);
    if((rc == GT_NOT_INITIALIZED) && (PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr != NULL))
    {
        *invertTx = PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr[laneNumInMultiCore].txPolarity;
        *invertRx = PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr[laneNumInMultiCore].rxPolarity;
    }
    else if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "internal_cpssDxChPortSerdesLanePolarityGet: error in mvHwsSerdesPolarityConfig, laneNum = %d\n", laneNumInMultiCore);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesLanePolarityGet function
* @endinternal
*
* @brief   Get the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3; Bobcat2;
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
*
* @param[out] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[out] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get the Tx/Rx polarity parameters for lane from HW if serdes initialized
*       or from SW DB if serdes was not initialized
*
*/
GT_STATUS cpssDxChPortSerdesLanePolarityGet
(
    IN   GT_U8      devNum,
    IN   GT_U32     portGroupNum,
    IN   GT_U32     laneNum,
    OUT  GT_BOOL    *invertTx,
    OUT  GT_BOOL    *invertRx
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesLanePolarityGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupNum, laneNum, invertTx, invertRx));

    rc = internal_cpssDxChPortSerdesLanePolarityGet(devNum, portGroupNum, laneNum, invertTx, invertRx);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupNum, laneNum, invertTx, invertRx));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesLanePolaritySet function
* @endinternal
*
* @brief   Set the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3; Bobcat2;
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
* @param[in] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[in] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - If serdes initialized: Set the Tx/Rx polarity parameters for lane
*       in both HW and SW DB
*       - If serdes was not initialized: Set the Tx/Rx polarity parameters
*       for lane only in SW DB
*
*/
static GT_STATUS internal_cpssDxChPortSerdesLanePolaritySet
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupNum,
    IN  GT_U32      laneNum,
    IN  GT_BOOL     invertTx,
    IN  GT_BOOL     invertRx
)
{
    GT_STATUS   rc;
    GT_U32      memSize;
    GT_U32      lanesNumInDev;      /* number of serdes lanes in current device */
    GT_U32      laneNumInMultiCore; /* plane serdes lane number in multi-port-group device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
                                          CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E);

    if(0 == (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (1<<(portGroupNum))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    lanesNumInDev = prvCpssDxChHwInitNumOfSerdesGet(devNum);

    if ((laneNum >= lanesNumInDev) || (invertTx > 1) || (invertRx > 1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* if SW DB was not allocated yet, allocate it here */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr == NULL)
    {
        if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            memSize = sizeof(PRV_DXCH_PORT_SERDES_POLARITY_CONFIG_STC) * lanesNumInDev * PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.numOfPortGroups;
        }
        else
        {
            memSize = sizeof(PRV_DXCH_PORT_SERDES_POLARITY_CONFIG_STC) * lanesNumInDev;
        }

        PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr = (PRV_DXCH_PORT_SERDES_POLARITY_CONFIG_STC *)cpssOsMalloc(memSize);

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        /* Zero the Tx/Rx polarity SW DB for all lanes */
        cpssOsMemSet(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr, 0, memSize);
    }

    laneNumInMultiCore = laneNum;
    if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        laneNumInMultiCore += portGroupNum * lanesNumInDev;
    }

    /* per Serdes number: save the Tx/Rx polarity values in SW DB */
    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr[laneNumInMultiCore].txPolarity = invertTx;
    PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr[laneNumInMultiCore].rxPolarity = invertRx;

    SERDES_DBG_DUMP_MAC(("invertTx=%d, invertRx=%d\n",
                         PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr[laneNumInMultiCore].txPolarity,
                         PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr[laneNumInMultiCore].rxPolarity));

    if (systemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        rc = mvHwsSerdesPolarityConfig(devNum, portGroupNum, laneNumInMultiCore, HWS_DEV_SERDES_TYPE(devNum), invertTx, invertRx);
        if ((rc != GT_OK) && (rc != GT_NOT_INITIALIZED))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "internal_cpssDxChPortSerdesLanePolaritySet: error in mvHwsSerdesPolarityConfig, laneNum = %d\n", laneNumInMultiCore);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesLanePolaritySet function
* @endinternal
*
* @brief   Set the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2;
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
* @param[in] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[in] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - If serdes initialized: Set the Tx/Rx polarity parameters for lane
*       in both HW and SW DB
*       - If serdes was not initialized: Set the Tx/Rx polarity parameters
*       for lane only in SW DB
*
*/
GT_STATUS cpssDxChPortSerdesLanePolaritySet
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupNum,
    IN  GT_U32      laneNum,
    IN  GT_BOOL     invertTx,
    IN  GT_BOOL     invertRx
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesLanePolaritySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupNum, laneNum, invertTx, invertRx));

    rc = internal_cpssDxChPortSerdesLanePolaritySet(devNum, portGroupNum, laneNum, invertTx, invertRx);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupNum, laneNum, invertTx, invertRx));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesPolaritySet function
* @endinternal
*
* @brief   Invert the Tx or Rx serdes polarity.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) to define
*                                      polarity on.
* @param[in] invertTx                   GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE  no invert
* @param[in] invertRx                   GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE  no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note This API should be egaged by application after serdes power up. Important to
*       wrap both steps i.e. "serdes power up" and "serdes polarity set" by port
*       disable and link_change interrupt lock and restore port enable and
*       reenable link change interrupt only after "serdes polarity set" to
*       prevent interrupt toggling during the process.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesPolaritySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               laneBmp,
    IN GT_BOOL              invertTx,
    IN GT_BOOL              invertRx
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      portGroupId;    /* port group */
    GT_U32      startSerdes;    /* first serdes of port */
    GT_U32      numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32      regAddr;        /* register address */
    GT_U32      value;          /* value to write to register */
    GT_U32      i;              /* iterator */
    GT_U32      lanesNumInDev;  /* number of serdes in device or single core */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                            devNum, portMacMap);

    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum,
                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap),
                            &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;

    lanesNumInDev = prvCpssDxChHwInitNumOfSerdesGet(devNum);

    if ((0 == numOfSerdesLanes) || ((startSerdes + numOfSerdesLanes) > lanesNumInDev))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    for (i = 0; (i < numOfSerdesLanes) && ((startSerdes + i) < lanesNumInDev); i++)
    {
        if(laneBmp & (1<<i))
        {
            if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
                (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesPolarityConfig(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], invertTx[%d], invertRx[%d])", devNum, portGroupId, startSerdes+i, HWS_DEV_SERDES_TYPE(devNum), invertTx, invertRx);
                rc = mvHwsSerdesPolarityConfig(devNum, portGroupId, startSerdes+i,
                                               HWS_DEV_SERDES_TYPE(devNum),
                                               invertTx, invertRx);

                if(rc != GT_OK)
                {
                    CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                    return rc;
                }
            }
            else
            {
                rc = prvCpssDxChLpCheckSerdesInitStatus(devNum, portGroupId,
                                                                startSerdes+i);
                if(rc != GT_OK)
                    return rc;

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                serdesConfig[startSerdes+i].digitalIfReg0;
                value = BOOL2BIT_MAC(invertTx) | (BOOL2BIT_MAC(invertRx)<<1);
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,
                                                        regAddr, 4, 2, value);
                if(rc != GT_OK)
                    return rc;
            }

        }/* if(laneBmp & (1<<i)) */

    }/* for(i = 0; i < numOfSerdesLanes; */

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesPolaritySet function
* @endinternal
*
* @brief   Invert the Tx or Rx serdes polarity.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) to define
*                                      polarity on.
* @param[in] invertTx                   GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE  no invert
* @param[in] invertRx                   GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE  no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note This API should be egaged by application after serdes power up. Important to
*       wrap both steps i.e. "serdes power up" and "serdes polarity set" by port
*       disable and link_change interrupt lock and restore port enable and
*       reenable link change interrupt only after "serdes polarity set" to
*       prevent interrupt toggling during the process.
*
*/
GT_STATUS cpssDxChPortSerdesPolaritySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               laneBmp,
    IN GT_BOOL              invertTx,
    IN GT_BOOL              invertRx
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesPolaritySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneBmp, invertTx, invertRx));

    rc = internal_cpssDxChPortSerdesPolaritySet(devNum, portNum, laneBmp, invertTx, invertRx);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneBmp, invertTx, invertRx));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesPolarityGet function
* @endinternal
*
* @brief   Get status of the Tx or Rx serdes polarity invert.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.) to
*                                      define polarity on.
*
* @param[out] invertTxPtr               (ptr to) GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE  no invert
* @param[out] invertRxPtr               (ptr to) GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE  no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssDxChPortSerdesPolarityGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32  laneNum,
    OUT GT_BOOL *invertTxPtr,
    OUT GT_BOOL *invertRxPtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      portGroupId;    /* port group */
    GT_U32      startSerdes;    /* first serdes of port */
    GT_U32      numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32      regAddr;        /* register address */
    GT_U32      value;          /* value to write to register */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(invertTxPtr);
    CPSS_NULL_PTR_CHECK_MAC(invertRxPtr);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum,
                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap),
                            &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;

    if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
    {
        if(laneNum > numOfSerdesLanes)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    {
        if(laneNum >= numOfSerdesLanes)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                            devNum, portMacMap);

    if ((PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesPolarityConfigGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *invertTx, *invertRx)", devNum, portGroupId, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum));
        rc = mvHwsSerdesPolarityConfigGet(devNum, portGroupId, startSerdes+laneNum,
                                          HWS_DEV_SERDES_TYPE(devNum), invertTxPtr,
                                          invertRxPtr);
        if(rc != GT_OK)
                {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
                }
    }
    else
    {
        rc = prvCpssDxChLpCheckSerdesInitStatus(devNum, portGroupId,
                                                startSerdes+laneNum);
        if(rc != GT_OK)
            return rc;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        serdesConfig[startSerdes+laneNum].digitalIfReg0;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,
                                                regAddr, 4, 2, &value);
        if(rc != GT_OK)
            return rc;

        *invertTxPtr = BIT2BOOL_MAC(value&0x1);
        *invertRxPtr = BIT2BOOL_MAC(value&0x2);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesPolarityGet function
* @endinternal
*
* @brief   Get status of the Tx or Rx serdes polarity invert.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.) to
*                                      define polarity on.
*
* @param[out] invertTxPtr               (ptr to) GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE  no invert
* @param[out] invertRxPtr               (ptr to) GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE  no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesPolarityGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32  laneNum,
    OUT GT_BOOL *invertTxPtr,
    OUT GT_BOOL *invertRxPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesPolarityGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, invertTxPtr, invertRxPtr));

    rc = internal_cpssDxChPortSerdesPolarityGet(devNum, portNum, laneNum, invertTxPtr, invertRxPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, invertTxPtr, invertRxPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortSerdesLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum  - physical device number
* @param[in] portNum - physical port number
* @param[in] mode    - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note  For Serdes Loopback modes:
*        - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS prvCpssDxChPortSerdesLoopbackModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT mode
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  portGroupId;/* core number of port */
    GT_U32                  localPort;  /* port number in local core */
    MV_HWS_PORT_STANDARD    portMode;   /* port interface in HWS format */
    MV_HWS_PORT_LB_TYPE     lbType;     /* loopback type in HWS format */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;/* current interface of port */
    CPSS_PORT_SPEED_ENT         speed;  /* current speed of port */
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U8                       sdVecSize=0;
    GT_U16                      *sdVectorPtr=NULL;

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
    speed  = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap);
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
        return rc;

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);

    if (PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_LION2_GIGE_MAC_LINK_STATUS_WA_E))
    {
        rc = prvCpssDxChPortLion2GeLinkStatusWaEnableSet(devNum, portMacMap,
                                                         ((CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E  != mode) &&
                                                          (CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E != mode)));
        if(rc != GT_OK)
            return rc;
    }

    if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_LION2_DISMATCH_PORTS_LINK_WA_E))
        || (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        || (CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        if((CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E  == mode) ||
           (CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E == mode))
        {
            /* restore application configuration for force link down if loopback enabled */
            rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum, portNum,
                               CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                                                              info_PRV_CPSS_DXCH_LION2_DISMATCH_PORT_LINK_WA_E.portForceLinkDownBmpPtr,
                                                              portMacMap));
            if(rc != GT_OK)
                return rc;
        }
    }

    /* only for Caelum (Bobk) device and above */
    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portMacMap, portMode, &curPortParams))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "hwsPortModeParamsGet returned null ");
        }

        sdVecSize   = curPortParams.numOfActLanes;
        sdVectorPtr = curPortParams.activeLanesList;
    }

    switch(mode)
    {
        case CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E:
            lbType = DISABLE_LB;
            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                rc = prvCpssDxChPortSerdesPolaritySet(devNum, 0, sdVectorPtr, sdVecSize);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChPortSerdesPolaritySet, portNum = %d\n", portMacMap);
                }
            }
            break;
        case CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E:
            lbType = TX_2_RX_LB;
            break;
        case CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E:
            lbType = TX_2_RX_DIGITAL_LB;
            break;
        case CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E:
            lbType = RX_2_TX_LB;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])", devNum, portGroupId, localPort, portMode, HWS_PMA, lbType);
    rc = mvHwsPortLoopbackSet(devNum, portGroupId, (portMode != QSGMII) ? localPort : (localPort & 0xFFFFFFFC),
                              portMode, HWS_PMA, lbType);
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_LION2_DISMATCH_PORTS_LINK_WA_E))
    {
        if((CPSS_PORT_SPEED_10000_E == speed) &&
           ((CPSS_PORT_INTERFACE_MODE_KR_E == ifMode) ||
               (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode) ||
            (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode)))
        {
            if((CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E == mode)
               || (CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E == mode))
            {/* run WA again to configure force link down state appropriate for
                current state of port */
                return prvCpssDxChPortLion2LinkFixWa(devNum, portMacMap);
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortSerdesLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum  - physical device number
* @param[in] portNum - physical port number
* @param[in] laneBmp - Not used
* @param[in] mode    - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note  For Serdes Loopback modes:
*        - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesLoopbackModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  laneBmp,
    IN  CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT mode
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      startSerdes;    /* first serdes of port */
    GT_U32      numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    /* avoid warning */
    laneBmp = laneBmp;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((mode < CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E) ||
       (mode >= CPSS_DXCH_PORT_SERDES_LOOPBACK_MAX_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap),
                                           &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;

    if(0 == numOfSerdesLanes)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    return prvCpssDxChPortSerdesLoopbackModeSet(devNum, portNum, mode);
}

/**
* @internal cpssDxChPortSerdesLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum  - physical device number
* @param[in] portNum - physical port number
* @param[in] laneBmp - Not used
* @param[in] mode    - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note  For Serdes Loopback modes:
*        - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS cpssDxChPortSerdesLoopbackModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  laneBmp,
    IN  CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT mode
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesLoopbackModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneBmp, mode));

    rc = internal_cpssDxChPortSerdesLoopbackModeSet(devNum, portNum, laneBmp, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneBmp, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesLoopbackModeGet function
* @endinternal
*
* @brief   Get current mode of loopback on SerDes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*                                      to read loopback status
*
* @param[out] modePtr                  - current loopback mode or none
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_SUPPORTED         - on not expected mode value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssDxChPortSerdesLoopbackModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  laneNum,
    OUT CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT *modePtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      portGroupId;    /* port group */
    GT_U32      startSerdes;    /* first serdes of port */
    GT_U32      numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32      regAddr;        /* register address */
    GT_U32      value;          /* value to write to register */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum,
                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap),
                            &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;

    if(laneNum >= numOfSerdesLanes)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                            devNum, portMacMap);

    if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E))
    {
        MV_HWS_SERDES_LB  lbMode;


        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesLoopbackGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *lbModePtr)", devNum, portGroupId, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum));
        rc = mvHwsSerdesLoopbackGet(devNum, portGroupId, startSerdes+laneNum,
                                        HWS_DEV_SERDES_TYPE(devNum), &lbMode);

        if(rc != GT_OK)
                {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
                }

        switch(lbMode)
        {
            case SERDES_LP_DISABLE:
                *modePtr = CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E;
                break;
            case SERDES_LP_AN_TX_RX:
                *modePtr = CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E;
                break;
            case SERDES_LP_DIG_TX_RX:
                *modePtr = CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E;
                break;
            case SERDES_LP_DIG_RX_TX:
                *modePtr = CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        rc = prvCpssDxChLpCheckSerdesInitStatus(devNum, portGroupId,
                                                startSerdes+laneNum);
        if(rc != GT_OK)
            return rc;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        serdesConfig[startSerdes+laneNum].testReg0;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,
                                                regAddr, 12, 3, &value);
        if(rc != GT_OK)
            return rc;

        switch(value)
        {
            case 0:
                *modePtr = CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E;
                break;
            case 1:
                *modePtr = CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E;
                break;
            case 2:
                *modePtr = CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E;
                break;
            case 4:
                *modePtr = CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesLoopbackModeGet function
* @endinternal
*
* @brief   Get current mode of loopback on SerDes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*                                      to read loopback status
*
* @param[out] modePtr                  - current loopback mode or none
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_SUPPORTED         - on not expected mode value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesLoopbackModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  laneNum,
    OUT CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesLoopbackModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, modePtr));

    rc = internal_cpssDxChPortSerdesLoopbackModeGet(devNum, portNum, laneNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortEomDfeResGet function
* @endinternal
*
* @brief   Returns the current DFE parameters.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] dfeResPtr                - current DFE V in millivolts
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*
* @note for sip 5 dfeResPtr size must be CPSS_DXCH_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS
*
*/
static GT_STATUS internal_cpssDxChPortEomDfeResGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  serdesNum,
    OUT GT_U32                 *dfeResPtr
)
{
    GT_STATUS rc;
    GT_U32 portGroupId;    /* port group */
    GT_U32 startSerdes;
    GT_U32 numOfSerdesLanes;
    GT_U32 globalSerdesNum;
    PRV_CPSS_GEN_PP_CONFIG_STC* pDev;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    MV_HWS_AUTO_TUNE_RESULTS    results;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
   PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E  );
    CPSS_NULL_PTR_CHECK_MAC(dfeResPtr);

    pDev = PRV_CPSS_PP_MAC(devNum);
    if(pDev->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum);
        if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Convert local serdes number to global*/
        rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum,
                                               ifMode,
                                               &startSerdes, &numOfSerdesLanes);
        globalSerdesNum = startSerdes + serdesNum;

        if(rc != GT_OK) return rc;


        if(serdesNum >= numOfSerdesLanes)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsEomGetDfeRes(devNum[%d], portGroup[%d], serdesNum[%d], *dfeRes)", devNum, portGroupId, globalSerdesNum);
        rc = mvHwsEomGetDfeRes(devNum, portGroupId, globalSerdesNum, dfeResPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
        return rc;
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(portMapShadowPtr->valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMapShadowPtr->portMap.macNum);
    }
    else
    {
        /* AC3 case */
        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum);
    }


    if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                           &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK) return rc;

    globalSerdesNum = startSerdes + serdesNum;

    if(serdesNum >= numOfSerdesLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsAvagoSerdesAutoTuneResult(devNum[%d], portGroup[%d], serdesNum[%d])", devNum, portGroupId, globalSerdesNum);
        rc = mvHwsAvagoSerdesAutoTuneResult(devNum, portGroupId, globalSerdesNum, &results);
        if(rc == GT_OK)
        {
            cpssOsMemCpy(dfeResPtr, results.DFE, sizeof(GT_U32) * CPSS_DXCH_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS);
        }
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsComH28nmEomGetDfeRes(devNum[%d], portGroup[%d], serdesNum[%d], *dfeRes)", devNum, portGroupId, globalSerdesNum);
        rc = mvHwsComH28nmEomGetDfeRes(devNum, portGroupId, globalSerdesNum, dfeResPtr);
    }
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}

/**
* @internal cpssDxChPortEomDfeResGet function
* @endinternal
*
* @brief   Returns the current DFE parameters.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] dfeResPtr                - current DFE V in millivolts
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*/
GT_STATUS cpssDxChPortEomDfeResGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  serdesNum,
    OUT GT_U32                 *dfeResPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEomDfeResGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesNum, dfeResPtr));

    rc = internal_cpssDxChPortEomDfeResGet(devNum, portNum, serdesNum, dfeResPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesNum, dfeResPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


#define DBG_PRINT_MATRIX  0

/**
* @internal internal_cpssDxChPortEomMatrixGet function
* @endinternal
*
* @brief   Returns the eye mapping matrix.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
* @param[in] samplingTime             - sampling time in usec, must be a positive value.
*
* @param[out] rowSizePtr               - number of rows in matrix
* @param[out] matrixPtr                - horizontal/vertical Rx eye matrix
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*
* @note The function initializes the EOM mechanism for the serdes,
*       gets the matrix and then closes the mechanism.
*       Due to it's size the CPSS_DXCH_PORT_EOM_MATRIX_STC struct should
*       be allocated on the heap.
*
*/
static GT_STATUS internal_cpssDxChPortEomMatrixGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    IN  GT_U32                           serdesNum,
    IN  GT_U32                           samplingTime,
    OUT GT_U32                          *rowSizePtr,
    OUT CPSS_DXCH_PORT_EOM_MATRIX_STC   *matrixPtr
)
{
    GT_U32 portGroupId;
    GT_STATUS rc;
    GT_U32 startSerdes;
    GT_U32 numOfSerdesLanes;
    GT_U32 globalSerdesNum;
    PRV_CPSS_GEN_PP_CONFIG_STC* pDev;
    GT_U32                      portMacNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E
                                          | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(rowSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(matrixPtr);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);

    if (samplingTime == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

   /* Convert local serdes number to global*/
    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum,
                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum),
                            /*OUT*/&startSerdes, &numOfSerdesLanes);
    globalSerdesNum = startSerdes + serdesNum;

    if (rc!=GT_OK)
        return rc;

    if (serdesNum >= numOfSerdesLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    cpssOsMemSet(matrixPtr, 0xFF, sizeof(CPSS_DXCH_PORT_EOM_MATRIX_STC));

    pDev = PRV_CPSS_PP_MAC(devNum);
    switch(pDev->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsEomInit(devNum[%d], portGroup[%d], serdesNum[%d])", devNum, portGroupId, globalSerdesNum);
            rc = mvHwsEomInit(devNum, portGroupId, globalSerdesNum);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsEomGetMatrix(devNum[%d], portGroup[%d], serdesNum[%d], timeout[%d], *rowSize, *upMatrix, *loMatrix)", devNum, portGroupId, globalSerdesNum, samplingTime);
            rc = mvHwsEomGetMatrix(devNum, portGroupId, globalSerdesNum, samplingTime, rowSizePtr,
                                   (GT_U32 *)&(matrixPtr->upperMatrix), (GT_U32 *)&(matrixPtr->lowerMatrix));

            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsEomClose(devNum[%d], portGroup[%d], serdesNum[%d])", devNum, portGroupId, globalSerdesNum);
            rc = mvHwsEomClose(devNum, portGroupId, globalSerdesNum);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
#if (DBG_PRINT_MATRIX == 1)
            printf("\nmvHwsComH28nmEomInit() ... ");
#endif
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsComH28nmEomInit(devNum[%d], portGroup[%d], serdesNum[%d])", devNum, portGroupId, globalSerdesNum);
            rc = mvHwsComH28nmEomInit(devNum, portGroupId, globalSerdesNum);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
#if (DBG_PRINT_MATRIX == 1)
            printf("OK");
            printf("\nmvHwsComH28nmEomGetMatrix() ... ");
#endif
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsComH28nmEomGetMatrix(devNum[%d], portGroup[%d], serdesNum[%d], timeout[%d], *rowSize, *upMatrix, *loMatrix)", devNum, portGroupId, globalSerdesNum, samplingTime);
            rc = mvHwsComH28nmEomGetMatrix(devNum, portGroupId, globalSerdesNum, samplingTime, rowSizePtr,
                                           (GT_U32 *)&(matrixPtr->upperMatrix), (GT_U32 *)&(matrixPtr->lowerMatrix));
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
#if (DBG_PRINT_MATRIX == 1)
            printf("OK");
            printf("\nmvHwsComH28nmEomClose() ... ");
#endif
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsComH28nmEomClose(devNum[%d], portGroup[%d], serdesNum[%d])", devNum, portGroupId, globalSerdesNum);
            rc = mvHwsComH28nmEomClose(devNum, portGroupId, globalSerdesNum);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
#if (DBG_PRINT_MATRIX == 1)
            printf("OK\n");
#endif
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    #if (DBG_PRINT_MATRIX == 1)
    {
        /*
        GT_U32 upperMatrix[CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS]
                                                [CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS];
        GT_U32 lowerMatrix[CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS]
                                                [CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS];
                                                */

        GT_U32 row;
        GT_U32 col;
        GT_U32 val;
        GT_U32 rowMax;
        GT_U32 colMax;

        rowMax = 18; /*CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS*2/3 */
        colMax = CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS/2;

        cpssOsPrintf("\n+------------------------+");
        cpssOsPrintf("\n|  Upper Matrix          |");
        cpssOsPrintf("\n+------------------------+");
        for (col = 0; col < colMax ; col ++)
        {
                cpssOsPrintf("\n %3d : ",col);
                for (row = 0 ; row < rowMax; row++)
                {
                        val = matrixPtr->upperMatrix[row][col];
                        if (val >= 100000)
                        {
                                val = 99999;
                        }
                        cpssOsPrintf(" %5d",val);
                }
        }
        cpssOsPrintf("\n+------------------------+");
        cpssOsPrintf("\n|  Lower Matrix          |");
        cpssOsPrintf("\n+------------------------+");
        for (col = 0; col < colMax ; col ++)
        {
                cpssOsPrintf("\n %3d :",col);
                for (row = 0 ; row < rowMax; row++)
                {
                        val = matrixPtr->lowerMatrix[row][col];
                        if (val >= 100000)
                        {
                                val = 99999;
                        }
                        cpssOsPrintf(" %5d",val);
                }
        }
    }
    #endif

    return GT_OK;
}

/**
* @internal cpssDxChPortEomMatrixGet function
* @endinternal
*
* @brief   Returns the eye mapping matrix.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
* @param[in] samplingTime             - sampling time in usec, must be a positive value.
*
* @param[out] rowSizePtr               - number of rows in matrix
* @param[out] matrixPtr                - horizontal/vertical Rx eye matrix
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*
* @note The function initializes the EOM mechanism for the serdes,
*       gets the matrix and then closes the mechanism.
*       Due to it's size the CPSS_DXCH_PORT_EOM_MATRIX_STC struct should
*       be allocated on the heap.
*
*/
GT_STATUS cpssDxChPortEomMatrixGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    IN  GT_U32                           serdesNum,
    IN  GT_U32                           samplingTime,
    OUT GT_U32                          *rowSizePtr,
    OUT CPSS_DXCH_PORT_EOM_MATRIX_STC   *matrixPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEomMatrixGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesNum, samplingTime, rowSizePtr, matrixPtr));

    rc = internal_cpssDxChPortEomMatrixGet(devNum, portNum, serdesNum, samplingTime, rowSizePtr, matrixPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesNum, samplingTime, rowSizePtr, matrixPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesEyeMatrixGet function
* @endinternal
*
* @brief   Returns the eye mapping matrix for SERDES.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - SERDES number
* @param[in] eye_inputPtr             - pointer to the serdes Eye monitoring input parameters structure
*
* @param[out] eye_resultsPtr           - pointer to the serdes Eye monitoring results structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - if not supported
* @retval else                     - on error
*
* @note The function allocated memory and gets the SERDES EOM matrix, vbtc and
*       hbtc calculation in text format.
*       After using this function application needs to free allocated memory, for example:
*       cpssOsFree(eye_resultsPtr->matrixPtr);
*       cpssOsFree(eye_resultsPtr->vbtcPtr);
*       cpssOsFree(eye_resultsPtr->hbtcPtr);
*
*/
static GT_STATUS internal_cpssDxChPortSerdesEyeMatrixGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  GT_U32                                serdesNum,
    IN CPSS_DXCH_PORT_SERDES_EYE_INPUT_STC   *eye_inputPtr,
    OUT CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC *eye_resultsPtr
)
{
    GT_STATUS rc;
    GT_U32    startSerdes;
    GT_U32    numOfSerdesLanes;
    GT_U32    portMacNum;
    GT_U32    dwell_bits_low_limit = 100000;
    GT_U32    dwell_bits_up_limit = 100000000;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E
                                          | CPSS_LION2_E | CPSS_XCAT3_E);
    CPSS_NULL_PTR_CHECK_MAC(eye_inputPtr);
    CPSS_NULL_PTR_CHECK_MAC(eye_resultsPtr);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if((eye_inputPtr->min_dwell_bits > eye_inputPtr->max_dwell_bits)
        || (eye_inputPtr->min_dwell_bits < dwell_bits_low_limit)
        || (eye_inputPtr->max_dwell_bits > dwell_bits_up_limit))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

   /* Convert local serdes number to global*/
    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum,
                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum),
                            /*OUT*/&startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
    {
        return rc;
    }
    if(serdesNum >= numOfSerdesLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
#if !defined(ASIC_SIMULATION) && !defined(CPSS_BLOB)
{
    MV_HWS_AVAGO_SERDES_EYE_GET_INPUT  eye_input;
    MV_HWS_AVAGO_SERDES_EYE_GET_RESULT eye_results;

    eye_resultsPtr->globalSerdesNum = startSerdes + serdesNum;
    eye_input.min_dwell_bits = eye_inputPtr->min_dwell_bits;
    eye_input.max_dwell_bits = eye_inputPtr->max_dwell_bits;

    rc = mvHwsAvagoSerdesEyeGet(devNum, eye_resultsPtr->globalSerdesNum, &eye_input, &eye_results);
    eye_resultsPtr->matrixPtr = eye_results.matrixPtr;
    eye_resultsPtr->x_points = eye_results.x_points;
    eye_resultsPtr->y_points = eye_results.y_points;
    eye_resultsPtr->vbtcPtr = eye_results.vbtcPtr;
    eye_resultsPtr->hbtcPtr = eye_results.hbtcPtr;
    eye_resultsPtr->height_mv = eye_results.height_mv;
    eye_resultsPtr->width_mui = eye_results.width_mui;
}
#else
    rc = GT_NOT_SUPPORTED;
#endif /*ASIC_SIMULATION*/
    return rc;
}

/**
* @internal cpssDxChPortSerdesEyeMatrixGet function
* @endinternal
*
* @brief   Returns the eye mapping matrix for SERDES.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - SERDES number
* @param[in] eye_inputPtr             - pointer to the serdes Eye monitoring input parameters structure
*
* @param[out] eye_resultsPtr           - pointer to the serdes Eye monitoring results structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - if not supported
* @retval else                     - on error
*
* @note The function allocated memory and gets the SERDES EOM matrix, vbtc and
*       hbtc calculation in text format.
*       After using this function application needs to free allocated memory, for example:
*       cpssOsFree(eye_resultsPtr->matrixPtr);
*       cpssOsFree(eye_resultsPtr->vbtcPtr);
*       cpssOsFree(eye_resultsPtr->hbtcPtr);
*
*/
GT_STATUS cpssDxChPortSerdesEyeMatrixGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  GT_U32                                serdesNum,
    IN  CPSS_DXCH_PORT_SERDES_EYE_INPUT_STC  *eye_inputPtr,
    OUT CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC *eye_resultsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesEyeMatrixGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesNum, eye_inputPtr, eye_resultsPtr));

    rc = internal_cpssDxChPortSerdesEyeMatrixGet(devNum, portNum, serdesNum, eye_inputPtr, eye_resultsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesNum, eye_inputPtr, eye_resultsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortEomBaudRateGet function
* @endinternal
*
* @brief   Returns the current system baud rate.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] baudRatePtr              - current system baud rate in pico-seconds.
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*/
static GT_STATUS internal_cpssDxChPortEomBaudRateGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_U32                    serdesNum,
    OUT GT_U32                   *baudRatePtr
)
{
    GT_U32 portGroupId;
    GT_U32 startSerdes;
    GT_U32 numOfSerdesLanes;
    GT_U32 globalSerdesNum;
    GT_U32 rc;
    PRV_CPSS_GEN_PP_CONFIG_STC* pDev;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E  );
    CPSS_NULL_PTR_CHECK_MAC(baudRatePtr);

    pDev = PRV_CPSS_PP_MAC(devNum);
    if(pDev->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        /* Convert local serdes number to global*/
        ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum);
        if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                               &startSerdes, &numOfSerdesLanes);
        globalSerdesNum = startSerdes + serdesNum;

        if(rc != GT_OK)
        {
            return rc;
        }

        if(serdesNum >= numOfSerdesLanes)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsEomGetUi(devNum[%d], portGroup[%d], serdesNum[%d], *curUi])",devNum, portGroupId, globalSerdesNum);
        rc = mvHwsEomGetUi(devNum, portGroupId, globalSerdesNum, baudRatePtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
        return rc;
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {

        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(portMapShadowPtr->valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMapShadowPtr->portMap.macNum);
    }
    else
    {
        /* AC3 case */
        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum);
    }

    if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                           &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;

    globalSerdesNum = startSerdes + serdesNum;

    if(serdesNum >= numOfSerdesLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)){
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsAvagoSerdeEomUiGet(devNum[%d], portGroup[%d], serdesNum[%d])", devNum, portGroupId, globalSerdesNum);
        rc = mvHwsAvagoSerdesEomUiGet(devNum, portGroupId, globalSerdesNum, baudRatePtr);
    } else {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsComH28nmEomGetUi(devNum[%d], portGroup[%d], serdesNum[%d])", devNum, portGroupId, globalSerdesNum);
        rc = mvHwsComH28nmEomGetUi(devNum, portGroupId, globalSerdesNum, baudRatePtr);
    }
    if(rc != GT_OK){
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}

/**
* @internal cpssDxChPortEomBaudRateGet function
* @endinternal
*
* @brief   Returns the current system baud rate.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] baudRatePtr              - current system baud rate in pico-seconds.
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*/
GT_STATUS cpssDxChPortEomBaudRateGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_U32                    serdesNum,
    OUT GT_U32                   *baudRatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEomBaudRateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesNum, baudRatePtr));

    rc = internal_cpssDxChPortEomBaudRateGet(devNum, portNum, serdesNum, baudRatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesNum, baudRatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortSerdesTxEnableSet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port
*                                      - GT_FALSE - Disable transmission of packets in
*                                      SERDES layer of a port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesTxEnableSet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      portGroupId;     /*the port group Id - support multi-port-groups device */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode; /* current interface of port */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portMacMap);

    if((PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_DXCH_LION_E) &&
       (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E))
    {
        GT_U32      regAddr;    /* register address */
        GT_U32      regValue;   /* register value */
        GT_U32      lane;       /* Serdes lane index  */
        GT_U32      startSerdes = 0, /* first serdes occupied by port */
                    serdesesNum = 0; /* number of serdeses occupied by port */

        rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,
                                                       &startSerdes,&serdesesNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Enable / Disable transmission of packets in SERDES layer of a port. */
        regValue = BOOL2BIT_MAC(!enable);
        for (lane = startSerdes; lane < (startSerdes + serdesesNum); lane++)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        serdesConfig[lane].serdesExternalReg2;
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                    2, 1, regValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        GT_U32                  phyPortNum;
        MV_HWS_PORT_STANDARD    portMode;

        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode,
                                    PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap),
                                    &portMode);
        if(rc != GT_OK)
            return rc;

        phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortTxEnable(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], enable[%d])",devNum, portGroupId, phyPortNum, portMode, enable);
        rc = mvHwsPortTxEnable(devNum, portGroupId, phyPortNum, portMode, enable);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesTxEnableSet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port
*                                      - GT_FALSE - Disable transmission of packets in
*                                      SERDES layer of a port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*/
GT_STATUS cpssDxChPortSerdesTxEnableSet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesTxEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortSerdesTxEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesTxEnableGet function
* @endinternal
*
* @brief   Get Enable / Disable transmission of packets in SERDES layer of a port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - Pointer to transmission of packets in SERDES
*                                      layer of a port.
*                                      - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port.
*                                      - GT_FALSE - Enable transmission of packets in
*                                      SERDES layer of a port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortSerdesTxEnableGet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_STATUS   rc;         /* return code */
    GT_U32      startSerdes = 0, serdesesNum = 0;
    GT_U32      portGroupId;     /*the port group Id - support multi-port-groups device */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode; /* current port interface mode */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,
                                                   &startSerdes,&serdesesNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portMacMap);

    if((PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_DXCH_LION_E) &&
       (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E))

    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    serdesConfig[startSerdes].serdesExternalReg2;
        /*  Get Enable / Disable transmission of packets in SERDES layer of a port. */
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 2,
                                                1, &regValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        *enablePtr = !BIT2BOOL_MAC(regValue);
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesTxEnableGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *enablePtr)", devNum, portGroupId, startSerdes,HWS_DEV_SERDES_TYPE(devNum));
        rc = mvHwsSerdesTxEnableGet(devNum, portGroupId, startSerdes,
                                    HWS_DEV_SERDES_TYPE(devNum), enablePtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesTxEnableGet function
* @endinternal
*
* @brief   Get Enable / Disable transmission of packets in SERDES layer of a port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - Pointer to transmission of packets in SERDES
*                                      layer of a port.
*                                      - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port.
*                                      - GT_FALSE - Enable transmission of packets in
*                                      SERDES layer of a port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSerdesTxEnableGet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesTxEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortSerdesTxEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal lion2ShareDisableWaExecute function
* @endinternal
*
* @brief   Run work around for erratum causing buffers and TXFIFO stuck as result
*         of changing of share state from enable to disable after some traffic
*         passed in TXDMA
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices
* @param[in] localPort                - number of port in local core
* @param[in] shareEnabled             -  is the share mode enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note API resets both DMA TXFifo and Prefetch counters, reset is done when a port
*       in shared mode is powered down or when a port is powered up to a mode which
*       requires share mode (40G).
*
*/
static GT_STATUS lion2ShareDisableWaExecute
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroupId,
    IN  GT_PHYSICAL_PORT_NUM    localPort,
    IN  GT_BOOL                 shareEnabled
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_U32      fieldData,  /* value of field in register */
                offset1,    /* field offset of first port/pair in register */
                offset2,    /* field offset of second port/pair in register */
                regAddr;    /* address of register */

    if((localPort%4 != 0) && (localPort != 9))
    {/* only on first port in mini-GOP or port 9 can configure 40G */
        return GT_OK;
    }

    /* If port is powered down disable sharing if enabled and reset TXDMA counters*/
    if (shareEnabled == GT_FALSE)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.thresholdOverrideEnable[localPort/6];
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                             ((localPort%6)*5+4), 1, &fieldData);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* if port is powered down and share was disabled, do not perform reset*/
        if(fieldData == 0)
        {
            return rc;
        }

        /* disable share mode*/
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, ((localPort%6)*5+4), 1, 0);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    /* Perform reset (on power up to 40G or power down from 40G)*/
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaCountersReset;

    offset1 = localPort/2;
    if (localPort < 9) /* ports 0-7 use bits 16-19 (bit per two ports)*/
    {
        offset2 = offset1;
    }
    else    /* ports 8-11 use bits 20-23 (bit per port)*/
    {
        offset2 = (localPort - 9)/2 + offset1 + 1;
    }

CPSS_TBD_BOOKMARK_LION2 /* removed prefetch reset to prevent problems with ports
                            8/9 */
    /* fieldData = (1<<offset1) | (1<<(16+offset2)); */
    fieldData = (1<<(16+offset2));

    /* reset counters of appropriate ports */
    rc = prvCpssHwPpPortGroupWriteRegBitMask(devNum, portGroupId, regAddr,
                                             fieldData/*mask*/, fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* let system time to reset */
    cpssOsTimerWkAfter(1);

    /* set again */
    rc = prvCpssHwPpPortGroupWriteRegBitMask(devNum, portGroupId, regAddr,
                                             fieldData/*mask*/, 0);

    if(rc != GT_OK)
    {
        return rc;
    }


    return rc;
}

/**
* @internal prvCpssDxChPortSerdesPpm_xCat_xCat2_Lion_Set function
* @endinternal
*
* @brief   Increase/decrease Tx clock on port (added/sub ppm).
*         Can be run only after port configured, not under traffic.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - number of physical port all serdeses occupied by it, in
*                                      currently configured interface mode will be configured
*                                      to required PPM
* @param[in] ppmValue                 - signed value - positive means speed up, negative means slow
*                                      down, 0 - means disable PPM.
*                                      (APPLICABLE RANGES: -100..100) PPM accoringly to protocol
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - ppmValue out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_BAD_STATE             - PPM value calculation wrong
*/
static GT_STATUS prvCpssDxChPortSerdesPpm_xCat_xCat2_Lion_Set
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32                ppmValue
)
{

  GT_STATUS rc;
  CPSS_PORT_INTERFACE_MODE_ENT ifMode;
  GT_U32      portGroupId;     /*the port group Id - support multi-port-groups device */
  GT_U32      regAddr, regAddrVal;
  GT_U32      lane;       /* Serdes lane index  */
  GT_U32      startSerdes = 0, /* first serdes occupied by port */
              serdesesNum = 0; /* number of serdeses occupied by port */

  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, ~(CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_XCAT3_E));

  if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) <= PRV_CPSS_PORT_FE_E)
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

  rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
  if(rc != GT_OK)
  {
    return rc;
  }

  if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
  {
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
  }

  portGroupId =
    PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);


  rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,
                                                 &startSerdes,&serdesesNum);
  if(rc != GT_OK)
  {
      return rc;
  }

  switch(ppmValue)
  {
    case 0: /* Setting 0 ppm */
      regAddrVal = 0x0;
    break;

    case 30: /* Adding 30 ppm */
      regAddrVal = 0x3ff;
    break;

    case 60: /* Adding 60 ppm */
      regAddrVal = 0x3fe;
    break;

    case 90: /* Adding 90 ppm */
      regAddrVal = 0x3fd;
    break;

    case -30: /* Decrease 30 ppm */
      regAddrVal = 0x1;
    break;

    case -60: /* Decrease 60 ppm */
      regAddrVal = 0x2;
    break;

    case -90: /* Decrease 90 ppm */
      regAddrVal = 0x3;
    break;

    default:
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
  }

  for (lane = startSerdes; lane < (startSerdes + serdesesNum); lane++)
  {
      regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[lane].powerReg;
      rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 11, 1, 1);
      if(rc != GT_OK)
      {
          return rc;
      }
  }

  /*   waiting 50mS */
  HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,50);

  for (lane = startSerdes; lane < (startSerdes + serdesesNum); lane++)
  {
      regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[lane].powerReg - 0x104;

      rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr, regAddrVal);
      if(rc != GT_OK)
      {
          return rc;
      }

      rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr, regAddrVal | 0x400);
      if(rc != GT_OK)
      {
        return rc;
      }
  }

  return GT_OK;
}

/**
* @internal internal_cpssDxChPortSerdesPpmSet function
* @endinternal
*
* @brief   Increase/decrease Tx clock on port (added/sub ppm).
*         Can be run only after port configured, not under traffic.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - number of physical port all serdeses occupied by it, in
*                                      currently configured interface mode will be configured
*                                      to required PPM
* @param[in] ppmValue                 - signed value - positive means speed up, negative means slow
*                                      down, 0 - means disable PPM.
*                                      (APPLICABLE RANGES: -100..100) PPM accoringly to protocol
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - ppmValue out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_BAD_STATE             - PPM value calculation wrong
*
* @note In Lion2 granularity of register is 30.5ppm, CPSS will translate ppmValue
*       to most close value to provided by application.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesPpmSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32                ppmValue
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  phyPortNum; /* number of port in local core */
    MV_HWS_PORT_STANDARD    portMode;   /* port ifMode in HWS format */
    MV_HWS_PPM_VALUE        portPPM;    /* PPM value in HWS format */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    if((ppmValue < -100) || (ppmValue > 100))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* For xCat, xCat2 and Lion */
    if (PRV_CPSS_PP_MAC(devNum)->appDevFamily & (CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E))
      return prvCpssDxChPortSerdesPpm_xCat_xCat2_Lion_Set(devNum, portNum,ppmValue);


    /* For Lion2 Bc2 use hwSwervices. */

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap),
                                PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap),
                                &portMode);
    if(rc != GT_OK)
        return rc;

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacMap);

    if(0 == ppmValue)
    {
        portPPM = ZERO_TAPS;
    }
    else if((ppmValue > 0) && (ppmValue <= 30))
    {
        portPPM = POS_1_TAPS;
    }
    else if((ppmValue > 30) && (ppmValue <= 60))
    {
        portPPM = POS_2_TAPS;
    }
    else if((ppmValue > 30) && (ppmValue <= 60))
    {
        portPPM = POS_2_TAPS;
    }
    else if((ppmValue > 60) && (ppmValue <= 100))
    {
        portPPM = POS_3_TAPS;
    }
    else if((ppmValue >= -30) && (ppmValue < 0))
    {
        portPPM = Neg_1_TAPS;
    }
    else if((ppmValue >= -60) && (ppmValue < -30))
    {
        portPPM = Neg_2_TAPS;
    }
    else if((ppmValue >= -100) && (ppmValue < -60))
    {
        portPPM = Neg_3_TAPS;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortPPMSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], portPPM[%d])", devNum, portGroup, phyPortNum, portMode, portPPM);
    if((CPSS_PORT_INTERFACE_MODE_QSGMII_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap)))
    {
        phyPortNum = phyPortNum &0xFFFFFFFC;
    }
    rc = mvHwsPortPPMSet(devNum, portGroup, phyPortNum, portMode, portPPM);
        if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    return rc;
}

/**
* @internal cpssDxChPortSerdesPpmSet function
* @endinternal
*
* @brief   Increase/decrease Tx clock on port (added/sub ppm).
*         Can be run only after port configured, not under traffic.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - number of physical port all serdeses occupied by it, in
*                                      currently configured interface mode will be configured
*                                      to required PPM
* @param[in] ppmValue                 - signed value - positive means speed up, negative means slow
*                                      down, 0 - means disable PPM.
*                                      (APPLICABLE RANGES: -100..100) PPM accoringly to protocol
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - ppmValue out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_BAD_STATE             - PPM value calculation wrong
*
* @note In Lion2 granularity of register is 30.5ppm, CPSS will translate ppmValue
*       to most close value to provided by application.
*
*/
GT_STATUS cpssDxChPortSerdesPpmSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32                ppmValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesPpmSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ppmValue));

    rc = internal_cpssDxChPortSerdesPpmSet(devNum, portNum, ppmValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ppmValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortSerdesPpm_xCat_xCat2_Lion_Get function
* @endinternal
*
* @brief   Get Tx clock increase/decrease status on port (added/sub ppm).
*         Can be run only after port configured.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - number of physical port all serdeses occupied by it, in
*                                      currently configured interface mode will be configured
*                                      to required PPM
*
* @param[out] ppmValuePtr              - PPM value currently used by given port.
*                                      0 - means PPM disabled.
*                                      (APPLICABLE RANGES: -90..90) PPM accoringly to protocol
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - ppmValue out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_BAD_STATE             - PPM value calculation wrong
*/
static GT_STATUS prvCpssDxChPortSerdesPpm_xCat_xCat2_Lion_Get
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32                *ppmValuePtr
)
{
  GT_STATUS rc;
  CPSS_PORT_INTERFACE_MODE_ENT ifMode;
  GT_U32      portGroupId;     /*the port group Id - support multi-port-groups device */
  GT_U32      regAddr, regAddrVal, firstPpm;
  GT_U32      lane;       /* Serdes lane index  */
  GT_U32      startSerdes = 0, /* first serdes occupied by port */
              serdesesNum = 0; /* number of serdeses occupied by port */

  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, ~(CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_XCAT3_E));

  if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) <= PRV_CPSS_PORT_FE_E)
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

  rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
  if(rc != GT_OK)
  {
    return rc;
  }

  if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
  {
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
  }

  portGroupId =
    PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);


  rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,
                                                 &startSerdes,&serdesesNum);
  if(rc != GT_OK)
  {
      return rc;
  }

  /* Read first Serdes lane */
  regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[startSerdes].powerReg - 0x104;
  rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &firstPpm);
  if(rc != GT_OK)
  {
      return rc;
  }

  /* Oon each related serdes value should be same */
  for (lane = startSerdes+1; lane < (startSerdes + serdesesNum); lane++)
  {
      regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[lane].powerReg - 0x104;
      rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regAddrVal);
      if(rc != GT_OK)
      {
        return rc;
      }
      if(firstPpm != regAddrVal)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
  }

  switch(firstPpm&0x3ff)
  {
    case 0: /* Setting 0 ppm */
      *ppmValuePtr = 0x0;
    break;

    case 0x3ff: /* Adding 30 ppm */
      *ppmValuePtr = 30;
    break;

    case 0x3fe: /* Adding 60 ppm */
      *ppmValuePtr = 60;
    break;

    case 0x3fd: /* Adding 90 ppm */
      *ppmValuePtr = 90;
    break;

    case 1: /* Decrease 30 ppm */
      *ppmValuePtr = -30;
    break;

    case 2: /* Decrease 60 ppm */
      *ppmValuePtr = -60;
    break;

    case 3: /* Decrease 90 ppm */
      *ppmValuePtr = -90;
    break;

    default:
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
  }

  return GT_OK;
}

/**
* @internal internal_cpssDxChPortSerdesPpmGet function
* @endinternal
*
* @brief   Get Tx clock increase/decrease status on port (added/sub ppm).
*         Can be run only after port configured.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - number of physical port all serdeses occupied by it, in
*                                      currently configured interface mode will be configured
*                                      to required PPM
*
* @param[out] ppmValuePtr              - PPM value currently used by given port.
*                                      0 - means PPM disabled.
*                                      (APPLICABLE RANGES: -100..100) PPM accoringly to protocol
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note In Lion2 granularity of register is 30.5ppm.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesPpmGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_32                *ppmValuePtr
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  portGroup;  /* local port group/core number */
    GT_U32                  phyPortNum; /* local port number in core */
    MV_HWS_PORT_STANDARD    portMode;   /* current port interface */
    MV_HWS_PPM_VALUE        portPPM;    /* PPM in HWS format */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(ppmValuePtr);

    /* For xCat, xCat2 and Lion */
    if (PRV_CPSS_PP_MAC(devNum)->appDevFamily & (CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E))
      return prvCpssDxChPortSerdesPpm_xCat_xCat2_Lion_Get(devNum, portNum, ppmValuePtr);


    /* For Lion2 Bc2 use hwSwervices. */

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap),
                                PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap),
                                &portMode);
    if(rc != GT_OK)
        return rc;

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacMap);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortPPMGet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], *portPPM)", devNum, portGroup, phyPortNum, portMode);
    if((CPSS_PORT_INTERFACE_MODE_QSGMII_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap)))
    {
        phyPortNum = phyPortNum &0xFFFFFFFC;
    }

    rc = mvHwsPortPPMGet(devNum, portGroup, phyPortNum, portMode, &portPPM);
    if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
        }

    switch(portPPM)
    {
        case ZERO_TAPS:
            *ppmValuePtr = 0;
            break;
        case Neg_3_TAPS:
            *ppmValuePtr = -90;
            break;
        case Neg_2_TAPS:
            *ppmValuePtr = -60;
            break;
        case Neg_1_TAPS:
            *ppmValuePtr = -30;
            break;
        case POS_1_TAPS:
            *ppmValuePtr = 30;
            break;
        case POS_2_TAPS:
            *ppmValuePtr = 60;
            break;
        case POS_3_TAPS:
            *ppmValuePtr = 90;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesPpmGet function
* @endinternal
*
* @brief   Get Tx clock increase/decrease status on port (added/sub ppm).
*         Can be run only after port configured.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - number of physical port all serdeses occupied by it, in
*                                      currently configured interface mode will be configured
*                                      to required PPM
*
* @param[out] ppmValuePtr              - PPM value currently used by given port.
*                                      0 - means PPM disabled.
*                                      (APPLICABLE RANGES: -100..100) PPM accoringly to protocol
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note In Lion2 granularity of register is 30.5ppm.
*
*/
GT_STATUS cpssDxChPortSerdesPpmGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_32                *ppmValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesPpmGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ppmValuePtr));

    rc = internal_cpssDxChPortSerdesPpmGet(devNum, portNum, ppmValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ppmValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvDxChPortSerdesPpmStatusGet function
* @endinternal
*
* @brief   Print Rx clock increase/decrease status on port (added/sub ppm).
*         Can be run only after port configured.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3,Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - number of physical port all serdeses occupied by it, in
*                                      currently configured interface mode will be configured
*                                      to required PPM
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS prvDxChPortSerdesPpmStatusGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{

  GT_STATUS rc;
  CPSS_PORT_INTERFACE_MODE_ENT ifMode;
  GT_U32      portGroupId;     /*the port group Id - support multi-port-groups device */
  GT_U32      regAddr, regAddrVal;
  GT_U32      lane;       /* Serdes lane index  */
  GT_U32      startSerdes = 0, /* first serdes occupied by port */
              serdesesNum = 0; /* number of serdeses occupied by port */


  PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, ~(CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_XCAT3_E));

  if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) <= PRV_CPSS_PORT_FE_E)
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);


/*  Flow:
[DL] yes there is:
Write to 0x9800280[14] = 1 // dtl_floop_en=1
Write to 0x9800280[9] = 1 //rx_foffst_extraction_en
Write to 0x9800284[15]=1 // rx_foffst_rd_req=1
Sleep(1ms)
Write to 0x9800284[15]=0 // rx_foffst_rd_req=0
Read ppm = 0x9800274[9:0]
Write to 0x9800280[9] = 0 //rx_foffst_extraction_en=0
Write to 0x9800280[14] = 0 // dtl_floop_en=0

*/

  rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
  if(rc != GT_OK)
  {
    return rc;
  }

  if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
  {
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
  }

  portGroupId =
    PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);


  rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,
                                                 &startSerdes,&serdesesNum);
  if(rc != GT_OK)
  {
      return rc;
  }

  for (lane = startSerdes; lane < (startSerdes + serdesesNum); lane++)
  {
      regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[lane].receiverReg2;

      rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 14, 1, 1);
      if(rc != GT_OK)
      {
          return rc;
      }

      rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 9, 1, 1);
      if(rc != GT_OK)
      {
          return rc;
      }

      rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr+4, 15, 1, 1);
      if(rc != GT_OK)
      {
          return rc;
      }
  }

  /*     waiting 50mS */
  HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,50);

  for (lane = startSerdes; lane < (startSerdes + serdesesNum); lane++)
  {
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[lane].receiverReg2;

    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr+4, 15, 1, 0);
    if(rc != GT_OK)
    {
      return rc;
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr-12, 0, 9, &regAddrVal);
    if(rc != GT_OK)
    {
    return rc;
    }
    cpssOsPrintf("Port %d Serdes %d (%X, %X), ppm %d\n", portNum, lane, regAddr, regAddr-12, regAddrVal);


    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 9, 1, 0);
    if(rc != GT_OK)
    {
      return rc;
    }

    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 14, 1, 0);
    if(rc != GT_OK)
    {
      return rc;
    }
  }

  return GT_OK;

}
/**
* @internal prvCpssDxChSerdesRefClockTranslateCpss2Hws function
* @endinternal
*
* @brief   Get serdes referense clock from CPSS DB and translate it to HWS format
*
* @param[in] devNum                   - physical device number
*
* @param[out] refClockPtr              - (ptr to) serdes referense clock in HWS format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - if value from CPSS DB not supported
*/
GT_STATUS prvCpssDxChSerdesRefClockTranslateCpss2Hws
(
    IN  GT_U8 devNum,
    OUT MV_HWS_REF_CLOCK_SUP_VAL *refClockPtr
)
{
    switch(PRV_CPSS_DXCH_PP_MAC(devNum)->port.serdesRefClock)
    {
        case CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E:
            *refClockPtr = MHz_25;
            break;

        case CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_125_SINGLE_ENDED_E:
        case CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_125_DIFF_E:
        case CPSS_DXCH_PP_SERDES_REF_CLOCK_INTERNAL_125_E:
            *refClockPtr = MHz_125;
            break;

        case CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_SINGLE_ENDED_E:
        case CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E:
            *refClockPtr = MHz_156;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPortLion2LinkFixWa function
* @endinternal
*
* @brief   For Lion2 A0 "no allignment lock WA"
*         For Lion2 B0 "40G connect/disconnect WA", "false link up WA"
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat; xCat3; xCat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChPortLion2LinkFixWa
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS rc;           /* return code */
    GT_U32    pcsNum;       /* pcs number used by port */
    GT_U32    portGroup;    /* local core number */
    GT_U32    localPort;    /* port number in local core */
    MV_HWS_PORT_STANDARD    portMode;
    CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboParamsPtr;
    CPSS_PORT_SPEED_ENT portSpeed;
    GT_U32   portMacMap;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                          | CPSS_LION_E | CPSS_XCAT2_E
                                          | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    portSpeed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap);

    portMode = ((CPSS_PORT_SPEED_10000_E == portSpeed)
                || (CPSS_PORT_SPEED_11800_E == portSpeed))
                    ? _10GBase_KR : _40GBase_KR;
    comboParamsPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portNum];
    if(comboParamsPtr->macArray[0].macNum != CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
    { /* if it's combo port */
        CPSS_DXCH_PORT_MAC_PARAMS_STC   mac;
        rc = cpssDxChPortComboPortActiveMacGet(devNum, portNum, &mac);
        if(rc != GT_OK)
        {
            return rc;
        }
        if (mac.macNum != localPort)
        {
            portMode = _100GBase_KR10;
        }
    }

    /* here not important KR or SR_LR */
    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPort, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        return rc;
    }
    pcsNum = curPortParams.portPcsNumber;


    if(PRV_CPSS_PP_MAC(devNum)->revision > 0)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMmPcs40GConnectWa(devNum[%d], portGroup[%d], pcsNum[%d])", devNum, portGroup, pcsNum);
        rc = mvHwsMmPcs40GConnectWa(devNum, portGroup, pcsNum);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            cpssOsPrintf("mvHwsMmPcs40GConnectWa:devNum=%d,portNum=%d,portGroup=%d,pcsNum=%d,rc=%d\n",
                            devNum, portNum, portGroup, pcsNum, rc);
            cpssOsPrintf("Probably you connected 10G link to port where 40G interface configured\n");
        }
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, pcsNum, MMPCS, UNRESET);
        rc = mvHwsPcsReset(devNum, portGroup, localPort, portMode, MMPCS, UNRESET);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, pcsNum, MMPCS, UNRESET);
        rc = mvHwsPcsReset(devNum, portGroup, localPort, portMode, MMPCS, UNRESET);
        if(rc != GT_OK)
        {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChPortLion2RxauiLinkFixWa function
* @endinternal
*
* @brief   Run RXAUI link WA
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChPortLion2RxauiLinkFixWa
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS rc;           /* return code */
    GT_U32    macNum;       /* mac number used by port */
    GT_U32    portGroup;    /* local core number */
    GT_U32    localPort;    /* port number in local core */
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                          | CPSS_LION_E | CPSS_XCAT2_E
                                          | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPort, RXAUI, &curPortParams);
    if (GT_OK != rc)
    {
        return rc;
    }
    macNum = curPortParams.portMacNumber;

    if(macNum != NA_NUM)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsXPcsConnectWa(devNum[%d], portGroup[%d], portNum[%d])", devNum, portGroup, localPort);
        rc = mvHwsXPcsConnectWa(devNum, portGroup, localPort);
                if(rc != GT_OK)
                {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                }
                return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSerdesManualConfig function
* @endinternal
*
* @brief   Service function for cpssDxChPortSerdesManualConfig API's - make common tasks:
*         check parameters and obtain common data
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesCfgPtr             - serdes configuration parameters
*
* @param[out] portGroupPtr             - core number of port
* @param[out] startSerdesPtr           - number of first serdes used by port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS prvCpssDxChPortSerdesManualConfig
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum,
    IN  GT_VOID                 *serdesCfgPtr,
    OUT GT_U32                  *portGroupPtr,
    OUT GT_U32                  *startSerdesPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32      numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */
        CPSS_PORT_INTERFACE_MODE_ENT    ifMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) && (PRV_CPSS_PP_MAC(devNum)->revision == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(serdesCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(portGroupPtr);
    CPSS_NULL_PTR_CHECK_MAC(startSerdesPtr);

    *portGroupPtr = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                            devNum, portMacMap);

        ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
        if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum,
                            ifMode,
                            startSerdesPtr, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;

    /* > and not >= to allow redundant serdes configuration */
    if(laneNum > numOfSerdesLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortSerdesManualTxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW.
*         APPLICABLE DEVICES:
*         xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*         INPUTS:
*         devNum - physical device number
*         portNum - physical port number
*         laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*         serdesTxCfgPtr - serdes Tx parameters:
*         txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*         In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*         txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*         APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*         emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*         emph1 - Controls the emphasis amplitude for Gen1 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*         txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesTxCfgPtr           - serdes Tx parameters:
*                                      txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*                                      txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*                                      emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*                                      emph1 - Controls the emphasis amplitude for Gen1 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*                                      txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssDxChPortSerdesTuningSet.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesManualTxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  startSerdes;    /* first serdes of port */

    rc = prvCpssDxChPortSerdesManualConfig(devNum, portNum, laneNum, serdesTxCfgPtr,
                                           &portGroup, &startSerdes);
    if(rc != GT_OK)
    {
        return rc;
    }

    if((serdesTxCfgPtr->txAmp & 0xFFFFFFE0) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if((serdesTxCfgPtr->txAmpAdjEn & 0xFFFFFFFE) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
         (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)) ||
         (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)          ||
         (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        if (serdesTxCfgPtr->emph0 < -31 ||
            serdesTxCfgPtr->emph0 >  31 ||
            serdesTxCfgPtr->emph1 < -31 ||
            serdesTxCfgPtr->emph1 >  31)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* Assuming FALCON INCLUDED */
    {
        if (serdesTxCfgPtr->emph0 <  0 ||
            serdesTxCfgPtr->emph0 > 31 ||
            serdesTxCfgPtr->emph1 <  0 ||
            serdesTxCfgPtr->emph1 > 31)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    { /* for Lion2 */
        if(serdesTxCfgPtr->emph0 < 0  ||
           serdesTxCfgPtr->emph0 > 15 ||
           serdesTxCfgPtr->emph1 < 0  ||
           serdesTxCfgPtr->emph1 > 15)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    if((serdesTxCfgPtr->txAmpShft & 0xFFFFFFFE) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesManualTxConfig(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], txAmp[%d], txAmpAdj[%d], emph0[%d], emph1[%d], txAmpShft[%d], txEmphEn[%d])", devNum, portGroup, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum), serdesTxCfgPtr->txAmp, serdesTxCfgPtr->txAmpAdjEn, serdesTxCfgPtr->emph0, serdesTxCfgPtr->emph1, serdesTxCfgPtr->txAmpShft, serdesTxCfgPtr->txEmphEn);
    rc = mvHwsSerdesManualTxConfig(devNum, portGroup, startSerdes+laneNum,
                                   HWS_DEV_SERDES_TYPE(devNum),
                                   serdesTxCfgPtr->txAmp,
                                   serdesTxCfgPtr->txAmpAdjEn,
                                   serdesTxCfgPtr->emph0,
                                   serdesTxCfgPtr->emph1,
                                   serdesTxCfgPtr->txAmpShft,
                                   serdesTxCfgPtr->txEmphEn);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Hws return code is [%d]", rc);
    }

    return rc;
}

/**
* @internal cpssDxChPortSerdesManualTxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW.
*         APPLICABLE DEVICES:
*         xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*         INPUTS:
*         devNum - physical device number
*         portNum - physical port number
*         laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*         serdesTxCfgPtr - serdes Tx parameters:
*         txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*         In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*         txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*         APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*         emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*         emph1 - Controls the emphasis amplitude for Gen1 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*         txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesTxCfgPtr           - serdes Tx parameters:
*                                      txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*                                      txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*                                      emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*                                      emph1 - Controls the emphasis amplitude for Gen1 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*                                      txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssDxChPortSerdesTuningSet.
*
*/
GT_STATUS cpssDxChPortSerdesManualTxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesManualTxConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesTxCfgPtr));

    rc = internal_cpssDxChPortSerdesManualTxConfigSet(devNum, portNum, laneNum, serdesTxCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesTxCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes TX.
*         APPLICABLE DEVICES:
*         xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*         INPUTS:
*         devNum - physical device number
*         portNum - physical port number
*         laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*         OUTPUTS:
*         serdesTxCfgPtr - serdes Tx parameters:
*         txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*         In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*         txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*         APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*         emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*         emph1 - Controls the emphasis amplitude for Gen1 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*         txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTxCfgPtr           - serdes Tx parameters:
*                                      txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*                                      txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*                                      emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*                                      emph1 - Controls the emphasis amplitude for Gen1 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*                                      txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssDxChPortSerdesManualTxConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  startSerdes;    /* first serdes of port */
    MV_HWS_AUTO_TUNE_RESULTS results; /* current serdes tuning values in HWS
                                        format */

    rc = prvCpssDxChPortSerdesManualConfig(devNum, portNum, laneNum,
                                           serdesTxCfgPtr, &portGroup,
                                           &startSerdes);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clean buffer to avoid random trash in LOG */
    cpssOsMemSet(&results, 0, sizeof(results));

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesAutoTuneResult(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *results)", devNum, portGroup, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum));
    rc = mvHwsSerdesAutoTuneResult(devNum, portGroup, startSerdes+laneNum,
                                   HWS_DEV_SERDES_TYPE(devNum), &results);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Hws return code is [%d]", rc);
    }

    serdesTxCfgPtr->txAmp       = results.txAmp;
    serdesTxCfgPtr->txAmpAdjEn  = results.txAmpAdj;
    serdesTxCfgPtr->emph0       = results.txEmph0;
    serdesTxCfgPtr->emph1       = results.txEmph1;
    serdesTxCfgPtr->txAmpShft   = results.txAmpShft;
    serdesTxCfgPtr->txEmphEn    = results.txEmph0En;

    return rc;
}

/**
* @internal cpssDxChPortSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes TX.
*         APPLICABLE DEVICES:
*         xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*         INPUTS:
*         devNum - physical device number
*         portNum - physical port number
*         laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*         OUTPUTS:
*         serdesTxCfgPtr - serdes Tx parameters:
*         txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*         In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*         txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*         APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*         emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*         emph1 - Controls the emphasis amplitude for Gen1 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*         txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTxCfgPtr           - serdes Tx parameters:
*                                      txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*                                      txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*                                      emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*                                      emph1 - Controls the emphasis amplitude for Gen1 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*                                      txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesManualTxConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesManualTxConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesTxCfgPtr));

    rc = internal_cpssDxChPortSerdesManualTxConfigGet(devNum, portNum, laneNum, serdesTxCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesTxCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesManualRxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssDxChPortSerdesTuningSet.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesManualRxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  startSerdes;    /* first serdes of port */
    GT_U32                  portMacNum;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    configParams;
    MV_HWS_PORT_STANDARD                portMode;

    rc = prvCpssDxChPortSerdesManualConfig(devNum, portNum, laneNum, serdesRxCfgPtr,
                                           &portGroup, &startSerdes);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum),
                PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum),
                &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        if((serdesRxCfgPtr->dcGain > 255) ||
           (serdesRxCfgPtr->ffeRes > 15)  ||
           (serdesRxCfgPtr->ffeCap > 15) ||
           (serdesRxCfgPtr->bandWidth > 15) ||
           (serdesRxCfgPtr->loopBandwidth > 15) ||
           (serdesRxCfgPtr->sqlch > 308))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }



        configParams.dcGain = serdesRxCfgPtr->dcGain;
        configParams.lowFrequency = serdesRxCfgPtr->ffeRes;
        configParams.highFrequency = serdesRxCfgPtr->ffeCap;
        configParams.bandWidth = serdesRxCfgPtr->bandWidth;
        configParams.loopBandwidth = serdesRxCfgPtr->loopBandwidth;
        configParams.squelch = serdesRxCfgPtr->sqlch;

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortManualCtleConfig(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], laneNum[%d], configParams{dcGain[%d], lowFrequency[%d], highFrequency[%d], bandWidth[%d], loopBandwidth[%d], squelch[%d]})",devNum, portGroup, portMacNum, portMode, (GT_U8)(laneNum), configParams.dcGain, configParams.lowFrequency, configParams.highFrequency, configParams.bandWidth, configParams.loopBandwidth, configParams.squelch);
        if(QSGMII == portMode)
        {
            rc = mvHwsPortManualCtleConfig(devNum, portGroup, portMacNum & 0xFFFC, portMode, (GT_U8)(laneNum), &configParams);
        }
        else
        {
            rc = mvHwsPortManualCtleConfig(devNum, portGroup, portMacNum, portMode, (GT_U8)(laneNum), &configParams);
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }

    }
    else
    {
        if((serdesRxCfgPtr->sqlch & 0xFFFFFFE0) != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if((serdesRxCfgPtr->ffeRes & 0xFFFFFFF8) != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if((serdesRxCfgPtr->ffeCap & 0xFFFFFFF0) != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if((serdesRxCfgPtr->align90 & 0xFFFFFF80) != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesManualRxConfig(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], portTuningMode[%d], sqlch[%d], ffeRes[%d], ffeCap[%d], dfeEn[%d], alig[%d])", devNum, portGroup, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum), ((CPSS_PORT_INTERFACE_MODE_KR_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portNum)) ? StaticLongReach : StaticShortReach), serdesRxCfgPtr->sqlch, serdesRxCfgPtr->ffeRes, serdesRxCfgPtr->ffeCap, GT_TRUE, serdesRxCfgPtr->align90);
        rc = mvHwsSerdesManualRxConfig(devNum, portGroup, startSerdes+laneNum,
                                       HWS_DEV_SERDES_TYPE(devNum),
                                       (((CPSS_PORT_INTERFACE_MODE_KR_E ==
                                        PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portNum)) ||
                                        _40GBase_CR4 == portMode) ?
                                        StaticLongReach : StaticShortReach),
                                        serdesRxCfgPtr->sqlch,
                                        serdesRxCfgPtr->ffeRes,
                                        serdesRxCfgPtr->ffeCap,
                                        GT_TRUE,
                                        serdesRxCfgPtr->align90);
        if(rc != GT_OK)
        {
             CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    }

    return rc;
}

/**
* @internal cpssDxChPortSerdesManualRxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssDxChPortSerdesTuningSet.
*
*/
GT_STATUS cpssDxChPortSerdesManualRxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesManualRxConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesRxCfgPtr));

    rc = internal_cpssDxChPortSerdesManualRxConfigSet(devNum, portNum, laneNum, serdesRxCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesRxCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes RX.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note For devices Bobk, Aldrin, AC3X, Bobcat3 the squelch transforms from units 0 - 15 to
*       units 0 - 308 by formula (x 16) + 68. So it's value can be not the same as it was set.
*       For example, was set the squelch = 110. It's transforms and safes inside as
*       integer part of (squelch - 68) / 16 = (integer part)(110 - 68) / 16 = 2
*       And we obtain after getting (2 16) + 68 = 100
*
*/
static GT_STATUS internal_cpssDxChPortSerdesManualRxConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  startSerdes;    /* first serdes of port */
    MV_HWS_PORT_MAN_TUNE_MODE   portTuningMode;
    GT_BOOL                 dfeEn;
    GT_U32                  portMacNum;
    MV_HWS_PORT_STANDARD    portMode;

    rc = prvCpssDxChPortSerdesManualConfig(devNum, portNum, laneNum, serdesRxCfgPtr,
                                           &portGroup, &startSerdes);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    configParams;

        PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                    PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum),
                    PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum),
                    &portMode);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = mvHwsAvagoSerdesManualCtleConfigGet(devNum, portGroup, startSerdes + laneNum, &configParams);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mvHwsAvagoSerdesManualCtleConfigGet return code is [%d]", rc);
        }
        serdesRxCfgPtr->bandWidth = configParams.bandWidth;
        serdesRxCfgPtr->dcGain = configParams.dcGain;
        serdesRxCfgPtr->ffeCap = configParams.highFrequency;
        serdesRxCfgPtr->loopBandwidth = configParams.loopBandwidth;
        serdesRxCfgPtr->ffeRes = configParams.lowFrequency;
        serdesRxCfgPtr->sqlch = configParams.squelch;
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesManualRxConfigGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *sqlchPtr, *ffeResPtr, *ffeCapPtr, *dfeEnPtr, *aligPtr)", devNum, portGroup, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum));
        rc = mvHwsSerdesManualRxConfigGet(devNum, portGroup, startSerdes+laneNum,
                                          HWS_DEV_SERDES_TYPE(devNum),
                                          &portTuningMode,
                                          &(serdesRxCfgPtr->sqlch),
                                          &(serdesRxCfgPtr->ffeRes),
                                          &(serdesRxCfgPtr->ffeCap),
                                          &dfeEn,
                                          &(serdesRxCfgPtr->align90));
       if(rc != GT_OK)
       {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Hws return code is [%d]", rc);
       }
    }

    return rc;
}

/**
* @internal cpssDxChPortSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes RX.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note For devices Bobk, Aldrin, AC3X, Bobcat3 the squelch transforms from units 0 - 15 to
*       units 0 - 308 by formula (x 16) + 68. So it's value can be not the same as it was set.
*       For example, was set the squelch = 110. It's transforms and safes inside as
*       integer part of (squelch - 68) / 16 = (integer part)(110 - 68) / 16 = 2
*       And we obtain after getting (2 16) + 68 = 100
*
*/
GT_STATUS cpssDxChPortSerdesManualRxConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesManualRxConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesRxCfgPtr));

    rc = internal_cpssDxChPortSerdesManualRxConfigGet(devNum, portNum, laneNum, serdesRxCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesRxCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortSerdesRxauiManualTxConfigSet
*           function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW for
*         RXAUI mode.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2;
*         Caelum; ; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfPorts               - Size of input array
* @param[in] serdesRxauiTxCfgPtr      - array of serdes Tx
*       parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_BAD_PTR               - NULL pointer
*
* @note
*
*/
static GT_STATUS prvCpssDxChPortSerdesRxauiManualTxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_U8                                numOfPorts,
    IN CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC  *serdesRxauiTxCfgPtr
)
{
    GT_STATUS               rc = GT_OK;   /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E | CPSS_XCAT3_E
                                          | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(serdesRxauiTxCfgPtr);

    serdesRxauiTxConfig = serdesRxauiTxCfgPtr;
    numOfSerdesRxauiConfig = numOfPorts;

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesRxauiManualTxConfigSet
*           function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW for
*         RXAUI mode.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2;
*         Caelum; ; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfPorts               - Size of input array
* @param[in] serdesRxauiTxCfgPtr      - array of serdes Tx
*       parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_BAD_PTR               - NULL pointer
*
* @note
*
*/
static GT_STATUS internal_cpssDxChPortSerdesRxauiManualTxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_U8                                numOfPorts,
    IN CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC  *serdesRxauiTxCfgPtr
)
{
    GT_STATUS               rc;   /* return code */

    rc = prvCpssDxChPortSerdesRxauiManualTxConfigSet(devNum, numOfPorts, serdesRxauiTxCfgPtr);
    return rc;

}


/**
* @internal cpssDxChPortSerdesRxauiManualTxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW for
*         RXAUI mode.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2;
*         Caelum; ; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfPorts               - Size of input array
* @param[in] serdesRxauiTxCfgPtr      - array of serdes Tx
*       parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_BAT_PTR               - NULL pointer
*
* @note
*
*/
GT_STATUS cpssDxChPortSerdesRxauiManualTxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_U8                                numOfPorts,
    IN CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC  *serdesRxauiTxCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesRxauiManualTxConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfPorts, serdesRxauiTxCfgPtr));

    rc = internal_cpssDxChPortSerdesRxauiManualTxConfigSet(devNum, numOfPorts, serdesRxauiTxCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfPorts, serdesRxauiTxCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes (true/false).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
static GT_STATUS internal_cpssDxChPortSerdesSignalDetectGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS                        rc;                               /* return code */
    GT_U32                           portGroupId;                      /* number of core in multi-port-group devices */
    GT_U32                           i;                                /* iterator */
    GT_U32                           portMacMap;                       /* number of mac mapped to this physical port */
    GT_U32                           portGroup;                        /* local core number */
    GT_U32                           phyPortNum;                       /* number of port in local core */
    MV_HWS_PORT_STANDARD             hwsPortMode;                      /* current ifMode of port in HWS format */
    CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboParamsPtr;                   /* parameters of combo port */
    CPSS_PORT_INTERFACE_MODE_ENT     cpssIfMode;                       /* current interface of port */
    MV_HWS_PORT_INIT_PARAMS         curPortParams;                    /* current port parameters */
    GT_U32                           curLanesList[HWS_MAX_SERDES_NUM]; /* current lanes list */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(signalStatePtr);
    *signalStatePtr = GT_FALSE; /* Init the value to avoid "non initialized" problem */

    if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacMap))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    cpssIfMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);

    comboParamsPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portMacMap];
    hwsPortMode = NON_SUP_MODE;
    if(comboParamsPtr->macArray[0].macNum != CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
    { /* if it's combo port */
        CPSS_DXCH_PORT_MAC_PARAMS_STC   mac;
        rc = cpssDxChPortComboPortActiveMacGet(devNum, portNum, &mac);
        if(rc != GT_OK)
            return rc;
        if (mac.macNum != phyPortNum)
        {
            switch(cpssIfMode)
            {
                case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
                case CPSS_PORT_INTERFACE_MODE_KR_E:
                    hwsPortMode = _100GBase_KR10;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }
    }

    if(NON_SUP_MODE == hwsPortMode)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,cpssIfMode,
                                    PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap),
                                    &hwsPortMode);
        if(rc != GT_OK)
            return rc;
    }


    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                            devNum, portMacMap);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, hwsPortMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    rc = mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, hwsPortMode, curLanesList);

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesSignalDetectGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *signalDet)",
            devNum, portGroupId, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum));
        rc = mvHwsSerdesSignalDetectGet(devNum, portGroupId,
                                        MV_HWS_SERDES_NUM(curLanesList[i]),
                                        HWS_DEV_SERDES_TYPE(devNum),
                                        signalStatePtr);
        if((rc != GT_OK) || (GT_FALSE == *signalStatePtr))
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            break;
        }
    }

    return rc;
}


GT_STATUS prvDxChPortSerdesSignalDetectLiveGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS                        rc;                               /* return code */
    GT_U32                           portGroupId;                      /* number of core in multi-port-group devices */
    GT_U32                           i;                                /* iterator */
    GT_U32                           portMacMap;                       /* number of mac mapped to this physical port */
    GT_U32                           portGroup;                        /* local core number */
    GT_U32                           phyPortNum;                       /* number of port in local core */
    MV_HWS_PORT_STANDARD             hwsPortMode;                      /* current ifMode of port in HWS format */
    CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboParamsPtr;                   /* parameters of combo port */
    CPSS_PORT_INTERFACE_MODE_ENT     cpssIfMode;                       /* current interface of port */
    MV_HWS_PORT_INIT_PARAMS          curPortParams;                    /* current port parameters */
    GT_U32                           curLanesList[HWS_MAX_SERDES_NUM]; /* current lanes list */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(signalStatePtr);

    if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacMap))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    cpssIfMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);

    comboParamsPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portMacMap];
    hwsPortMode = NON_SUP_MODE;
    if(comboParamsPtr->macArray[0].macNum != CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
    { /* if it's combo port */
        CPSS_DXCH_PORT_MAC_PARAMS_STC   mac;
        rc = cpssDxChPortComboPortActiveMacGet(devNum, portNum, &mac);
        if(rc != GT_OK)
            return rc;
        if (mac.macNum != phyPortNum)
        {
            switch(cpssIfMode)
            {
                case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
                case CPSS_PORT_INTERFACE_MODE_KR_E:
                    hwsPortMode = _100GBase_KR10;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }
    }

    if(NON_SUP_MODE == hwsPortMode)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,cpssIfMode,
                                    PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap),
                                    &hwsPortMode);
        if(rc != GT_OK)
            return rc;
    }


    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                            devNum, portMacMap);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, hwsPortMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    rc = mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, hwsPortMode, curLanesList);

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesSignalDetectGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *signalDet)",
            devNum, portGroupId, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum));

        rc = mvHwsAvagoSerdesSignalLiveDetectGet(devNum, portGroupId, MV_HWS_SERDES_NUM(curLanesList[i]), signalStatePtr);

        if((rc != GT_OK) || (GT_FALSE == *signalStatePtr))
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            break;
        }
    }

    return rc;
}


/**
* @internal prvCpssDxChPortSerdesNumberGet function
* @endinternal
*
* @brief   Return number of active serdeses and array of active serdeses numbers that port uses
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] numOfSerdesesPtr         - number of active serdeses that port uses
* @param[out] serdesesNumberPtr        - array of active serdeses numbers that port uses
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS prvCpssDxChPortSerdesNumberGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                 *numOfSerdesesPtr,
    OUT GT_U32                 *serdesesNumberPtr
)
{
    GT_STATUS                        rc;                               /* return code */
    GT_U32                           i;                                /* iterator */
    GT_U32                           portMacMap;                       /* number of mac mapped to this physical port */
    GT_U32                           portGroup;                        /* local core number */
    GT_U32                           phyPortNum;                       /* number of port in local core */
    MV_HWS_PORT_STANDARD             hwsPortMode;                      /* current ifMode of port in HWS format */
    CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboParamsPtr;                   /* parameters of combo port */
    CPSS_PORT_INTERFACE_MODE_ENT     cpssIfMode;                       /* current interface of port */
    MV_HWS_PORT_INIT_PARAMS          curPortParams;                    /* current port parameters */
    GT_U32                           curLanesList[HWS_MAX_SERDES_NUM]; /* current lanes list */

    *numOfSerdesesPtr = 0;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(numOfSerdesesPtr);
    CPSS_NULL_PTR_CHECK_MAC(serdesesNumberPtr);

    if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacMap))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    cpssIfMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);

    comboParamsPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portMacMap];
    hwsPortMode = NON_SUP_MODE;
    if(comboParamsPtr->macArray[0].macNum != CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
    { /* if it's combo port */
        CPSS_DXCH_PORT_MAC_PARAMS_STC   mac;
        rc = cpssDxChPortComboPortActiveMacGet(devNum, portNum, &mac);
        if(rc != GT_OK)
            return rc;
        if (mac.macNum != phyPortNum)
        {
            switch(cpssIfMode)
            {
                case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
                case CPSS_PORT_INTERFACE_MODE_KR_E:
                    hwsPortMode = _100GBase_KR10;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }
    }

    if(NON_SUP_MODE == hwsPortMode)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,cpssIfMode,
                                    PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap),
                                    &hwsPortMode);
        if(rc != GT_OK)
            return rc;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, hwsPortMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    rc = mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, hwsPortMode, curLanesList);

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {

        serdesesNumberPtr[*numOfSerdesesPtr] = MV_HWS_SERDES_NUM(curLanesList[i]);
        (*numOfSerdesesPtr)++;
    }
    return rc;
}


/**
* @internal internal_cpssDxChPortSerdesSquelchSet function
* @endinternal
*
* @brief   Set For port Threshold (Squelch) for signal OK.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] squelch                  - threshold for signal OK (0-15)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS internal_cpssDxChPortSerdesSquelchSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  squelch
)
{
    GT_STATUS rc;
    GT_U32 numOfSerdeses;
    GT_U32 serdesesNumber[HWS_MAX_SERDES_NUM];
#ifndef ASIC_SIMULATION
    GT_U32 i;
    GT_U32 rc1;
#endif
    GT_U32 portMacMap;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                    | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_BOBCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacMap))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    if(squelch > 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortSerdesNumberGet( devNum, portNum, &numOfSerdeses, serdesesNumber);
    if(GT_OK != rc)
    {
        return rc;
    }
#ifndef ASIC_SIMULATION
    for(i = 0; i < numOfSerdeses; i++)
    {
        rc1 = mvHwsAvagoSerdesSignalOkCfg(devNum,
                                          PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum),
                                          serdesesNumber[i],
                                          squelch);
        if(0 != rc1)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
#endif
    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesSquelchSet function
* @endinternal
*
* @brief   Set For port Threshold (Squelch) for signal OK.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] squelch                  - threshold for signal OK (0-15)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS cpssDxChPortSerdesSquelchSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  squelch
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesSquelchSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, squelch));

    rc = internal_cpssDxChPortSerdesSquelchSet(devNum, portNum, squelch);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, squelch));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPortSerdesSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes (true/false).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS cpssDxChPortSerdesSignalDetectGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesSignalDetectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, signalStatePtr));

    rc = internal_cpssDxChPortSerdesSignalDetectGet(devNum, portNum, signalStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, signalStatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesStableSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS internal_cpssDxChPortSerdesStableSignalDetectGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 interval = 100;
    GT_U32 window = 5;          /* 5 * 10msec = 50msec continuous time for stable signal indication */
    GT_U32 window_nosignal = 2; /* 2 * 10msec = 20msec continuous time for stable no signal indication */
    GT_U32 delayMS = 10;
    GT_BOOL signalStateFirst;
    GT_BOOL signalStateNext;
    GT_U32 stableInterval;
    GT_U32 portMacMap;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E
                                            | CPSS_LION_E | CPSS_XCAT_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    if(NULL == signalStatePtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    *signalStatePtr = 0;

    rc = cpssDxChPortSerdesSignalDetectGet(devNum, portNum, &signalStateFirst);
    if(GT_OK != rc)
    {
        return rc;
    }
    stableInterval = 0;
    for(i = 0; i < interval; i++)
    {
        rc = cpssDxChPortSerdesSignalDetectGet(devNum, portNum, &signalStateNext);
        if(GT_OK != rc)
        {
            return rc;
        }
        if(signalStateNext == signalStateFirst)
        {
            stableInterval++;
            if((GT_TRUE == signalStateFirst) && (stableInterval >= window))
            {
                *signalStatePtr = signalStateFirst;
                return GT_OK;
            }
            else if((GT_FALSE == signalStateFirst) && (stableInterval >= window_nosignal))
            {
                *signalStatePtr = signalStateFirst;
                return GT_OK;
            }
        }
        else
        {
            signalStateFirst = signalStateNext;
            stableInterval = 0;
        }
        cpssOsTimerWkAfter(delayMS);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChPortSerdesStableSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS cpssDxChPortSerdesStableSignalDetectGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesStableSignalDetectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, signalStatePtr));

    rc = internal_cpssDxChPortSerdesStableSignalDetectGet(devNum, portNum, signalStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, signalStatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesLaneSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on specific SerDes lane (true/false).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - number of local core
* @param[in] laneNum                  - number of required serdes lane
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssDxChPortSerdesLaneSignalDetectGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  laneNum,
    OUT GT_BOOL *signalStatePtr
)
{
        GT_STATUS rc;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);
    CPSS_NULL_PTR_CHECK_MAC(signalStatePtr);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesSignalDetectGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *signalDet)", devNum, portGroupId, laneNum, HWS_DEV_SERDES_TYPE(devNum));
    rc = mvHwsSerdesSignalDetectGet(devNum, portGroupId, laneNum,
                                        HWS_DEV_SERDES_TYPE(devNum),
                                        signalStatePtr);
        if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
        return rc;
}

/**
* @internal cpssDxChPortSerdesLaneSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on specific SerDes lane (true/false).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - number of local core
* @param[in] laneNum                  - number of required serdes lane
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesLaneSignalDetectGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  laneNum,
    OUT GT_BOOL *signalStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesLaneSignalDetectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, laneNum, signalStatePtr));

    rc = internal_cpssDxChPortSerdesLaneSignalDetectGet(devNum, portGroupId, laneNum, signalStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, laneNum, signalStatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesCDRLockStatusGet function
* @endinternal
*
* @brief   Return SERDES CDR lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] cdrLockPtr               - CRD lock state on serdes:
*                                      GT_TRUE  - CDR locked;
*                                      GT_FALSE - CDR not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - cdrLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssDxChPortSerdesCDRLockStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *cdrLockPtr
)
{
    GT_STATUS   rc;
    GT_U32      portGroupId;
    GT_U32      startSerdes,
                numOfSerdesLanes;
    GT_U32      i;
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                          | CPSS_LION_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(cdrLockPtr);

    if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacMap))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum,
                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap),
                            &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                            devNum, portMacMap);

    for(i = startSerdes; i < startSerdes+numOfSerdesLanes; i++)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesCdrLockStatusGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *cdrLock)", devNum, portGroupId, i, HWS_DEV_SERDES_TYPE(devNum));
        rc = mvHwsSerdesCdrLockStatusGet(devNum, portGroupId, i,
                                        HWS_DEV_SERDES_TYPE(devNum), cdrLockPtr);

        if((rc != GT_OK) || (GT_FALSE == *cdrLockPtr))
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            break;
        }
    }

    return rc;
}

/**
* @internal cpssDxChPortSerdesCDRLockStatusGet function
* @endinternal
*
* @brief   Return SERDES CDR lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] cdrLockPtr               - CRD lock state on serdes:
*                                      GT_TRUE  - CDR locked;
*                                      GT_FALSE - CDR not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - cdrLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesCDRLockStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *cdrLockPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesCDRLockStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cdrLockPtr));

    rc = internal_cpssDxChPortSerdesCDRLockStatusGet(devNum, portNum, cdrLockPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cdrLockPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesAutoTuneOptAlgSet function
* @endinternal
*
* @brief   Configure bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API configures field SW DB which will be used by
*       cpssDxChPortSerdesAutoTune.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesAutoTuneOptAlgSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serdesOptAlgBmp
)
{
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
         | CPSS_LION_E | CPSS_XCAT2_E  );
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    PRV_CPSS_DXCH_PORT_SD_OPT_ALG_BMP_MAC(devNum, portMacMap) = serdesOptAlgBmp;

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesAutoTuneOptAlgSet function
* @endinternal
*
* @brief   Configure bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API configures field SW DB which will be used by
*       cpssDxChPortSerdesAutoTune.
*
*/
GT_STATUS cpssDxChPortSerdesAutoTuneOptAlgSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serdesOptAlgBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesAutoTuneOptAlgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesOptAlgBmp));

    rc = internal_cpssDxChPortSerdesAutoTuneOptAlgSet(devNum, portNum, serdesOptAlgBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesOptAlgBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesAutoTuneOptAlgGet function
* @endinternal
*
* @brief   Get bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] serdesOptAlgBmpPtr       - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortSerdesAutoTuneOptAlgGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               *serdesOptAlgBmpPtr
)
{
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
         | CPSS_LION_E | CPSS_XCAT2_E  );
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(serdesOptAlgBmpPtr);

    *serdesOptAlgBmpPtr = PRV_CPSS_DXCH_PORT_SD_OPT_ALG_BMP_MAC(devNum, portMacMap);

    return GT_OK;
}

/**
* @internal cpssDxChPortSerdesAutoTuneOptAlgGet function
* @endinternal
*
* @brief   Get bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] serdesOptAlgBmpPtr       - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSerdesAutoTuneOptAlgGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               *serdesOptAlgBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesAutoTuneOptAlgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesOptAlgBmpPtr));

    rc = internal_cpssDxChPortSerdesAutoTuneOptAlgGet(devNum, portNum, serdesOptAlgBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesOptAlgBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesAutoTuneOptAlgRun function
* @endinternal
*
* @brief   Run training/auto-tuning optimisation algorithms on serdeses of port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortSerdesAutoTuneOptAlgRun
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serdesOptAlgBmp
)
{
    GT_STATUS               rc;         /* return code */
    MV_HWS_PORT_STANDARD    portMode;   /* port interface in HWS terms */
    GT_PHYSICAL_PORT_NUM    localPort;  /* number of port in local core */
    GT_U32                  portGroupId;/* number of local core */
    GT_U32                  portMacMap; /* number of mac mapped to this physical
                                            port */
    GT_BOOL                                        originalPortState;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
         | CPSS_LION_E | CPSS_XCAT2_E | CPSS_XCAT3_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap),
                                              PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap),
                                              &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);

    rc = prvCpssDxChPortEnableGet(devNum, portNum, &originalPortState);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(originalPortState != GT_FALSE)
    {
        rc = prvCpssDxChPortEnableSet(devNum,portNum,GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneOptimization(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], optAlgoMask[%d])", devNum, portGroupId, localPort, portMode, serdesOptAlgBmp);
    rc = mvHwsPortAutoTuneOptimization(devNum, portGroupId, localPort, portMode,
                                       serdesOptAlgBmp);
        if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }

    if(originalPortState != GT_FALSE)
    {
        rc = prvCpssDxChPortEnableSet(devNum,portNum,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal cpssDxChPortSerdesAutoTuneOptAlgRun function
* @endinternal
*
* @brief   Run training/auto-tuning optimisation algorithms on serdeses of port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSerdesAutoTuneOptAlgRun
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serdesOptAlgBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesAutoTuneOptAlgRun);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesOptAlgBmp));

    rc = internal_cpssDxChPortSerdesAutoTuneOptAlgRun(devNum, portNum, serdesOptAlgBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesOptAlgBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesAutoTuneResultsGet function
* @endinternal
*
* @brief   Read the results of SERDES auto tuning.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2,xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTunePtr            - serdes Tune parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssDxChPortSerdesAutoTuneResultsGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_TUNE_STC       *serdesTunePtr
)
{
    GT_STATUS                rc;           /* return code */
    GT_U32                   portGroup;    /* local core number */
    GT_U32                   startSerdes;  /* first serdes of port */
    MV_HWS_AUTO_TUNE_RESULTS results;      /* current serdes tuning values in HWS
                                              format */
    GT_U32                   portMacMap;   /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(serdesTunePtr);

    rc = prvCpssDxChPortSerdesManualConfig(devNum, portNum, laneNum,
                                           serdesTunePtr, &portGroup,
                                           &startSerdes);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Initialize local structure */
    cpssOsMemSet(&results, 0, sizeof(results));

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesAutoTuneResult(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *results)", devNum, portGroup, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum));
    rc = mvHwsSerdesAutoTuneResult(devNum, portGroup, startSerdes+laneNum,
                                   HWS_DEV_SERDES_TYPE(devNum), &results);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }
                  serdesTunePtr->ffeC           = (GT_U32 )results.ffeC      ;
                  serdesTunePtr->ffeR           = (GT_U32 )results.ffeR      ;
                  serdesTunePtr->sampler        = (GT_U32 )results.sampler   ;
                  serdesTunePtr->sqlch          = (GT_U32 )results.sqleuch   ;
                  serdesTunePtr->txAmp          = (GT_U32 )results.txAmp     ;
                  serdesTunePtr->txEmphAmp      = results.txEmph0;
                  serdesTunePtr->txEmph1        = results.txEmph1;
                  serdesTunePtr->align90        = (GT_U32 )results.align90   ;
                  serdesTunePtr->txAmpAdj       = (GT_U32 )results.txAmpAdj  ;
                  serdesTunePtr->txAmpShft      = (GT_BOOL)results.txAmpShft ;
                  serdesTunePtr->txEmphEn       = (GT_BOOL)results.txEmph0En ;
                  serdesTunePtr->txEmphEn1      = (GT_BOOL)results.txEmph1En ;
    cpssOsMemCpy( serdesTunePtr->dfeValsArray,             results.dfeVals,  sizeof(serdesTunePtr->dfeValsArray));

                  serdesTunePtr->dfe            = 0                          ; /* digital filter controls TBD:  xCat, xCat3, xCat2, Lion*/
                  serdesTunePtr->ffeS           = 0                          ; /* FFE signal swing control TBD: xCat, xCat3, xCat2, Lion*/

                  serdesTunePtr->DC            = (GT_U32 )results.DC         ;
                  serdesTunePtr->LF            = (GT_U32 )results.LF         ;
                  serdesTunePtr->HF            = (GT_U32 )results.HF         ;
                  serdesTunePtr->BW            = (GT_U32 )results.BW         ;
                  serdesTunePtr->LB            = (GT_U32 )results.LB         ;
                  serdesTunePtr->EO            = (GT_U32 )results.EO         ;
    cpssOsMemCpy( serdesTunePtr->DFE,                     results.DFE,       sizeof(serdesTunePtr->DFE));

    return rc;

}

/**
* @internal cpssDxChPortSerdesAutoTuneResultsGet function
* @endinternal
*
* @brief   Read the results of SERDES auto tuning.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2,xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTunePtr            - serdes Tune parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesAutoTuneResultsGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_TUNE_STC       *serdesTunePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesAutoTuneResultsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesTunePtr));

    rc = internal_cpssDxChPortSerdesAutoTuneResultsGet(devNum, portNum, laneNum, serdesTunePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesTunePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesEnhancedAutoTune function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2,xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssDxChPortSerdesEnhancedAutoTune
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   min_LF,
    IN  GT_U8                   max_LF
)
{
    GT_STATUS               rc;          /* return code */
    GT_U32                  portGroup;   /* local core number */
    GT_U32                  phyPortNum;  /* number of port in local core */
    MV_HWS_PORT_STANDARD    hwsPortMode; /* current ifMode of port in HWS format */
    GT_U32                  portMacNum;  /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_XCAT2_E| CPSS_BOBCAT2_E);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum),
                                PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,  portMacNum),
                                &hwsPortMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroup  = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortEnhanceTuneSet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], min_LF[%d], max_LF[%d])",
                             (GT_U8)devNum, (GT_U32)portGroup, (GT_U32)phyPortNum, (MV_HWS_PORT_STANDARD)hwsPortMode, (GT_U8)min_LF, (GT_U8)max_LF);
    rc = mvHwsPortEnhanceTuneSet(
        (GT_U8)                   devNum,
        (GT_U32)                  portGroup,
        (GT_U32)                  phyPortNum,
        (MV_HWS_PORT_STANDARD)    hwsPortMode,
        (GT_U8)                   min_LF,
        (GT_U8)                   max_LF);
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}
/**
* @internal cpssDxChPortSerdesEnhancedAutoTune function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2,xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesEnhancedAutoTune
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U8                  min_LF,
    IN  GT_U8                  max_LF
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesEnhancedAutoTune);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, min_LF, max_LF));

    rc = internal_cpssDxChPortSerdesEnhancedAutoTune(devNum, portNum, min_LF, max_LF);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, min_LF, max_LF));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesEnhancedTuneLite function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*                                      min_LF   - Minimum LF value that can be set on Serdes
*                                      max_LF   - Maximum LF value that can be set on Serdes
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssDxChPortSerdesEnhancedTuneLite
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   min_dly,
    IN  GT_U8                   max_dly
)
{
    GT_STATUS               rc;          /* return code */
    GT_U32                  portGroup;   /* local core number */
    GT_U32                  phyPortNum;  /* number of port in local core */
    MV_HWS_PORT_STANDARD    hwsPortMode; /* current ifMode of port in HWS format */
    GT_U32                  portMacNum;  /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_XCAT2_E| CPSS_BOBCAT2_E | CPSS_CAELUM_E);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum),
                                PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,  portMacNum),
                                &hwsPortMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "light enhaced tune hws port mode failed=%d", rc);
    }

    portGroup  = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortEnhanceTuneSet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], min_LF[%d], max_LF[%d])",
                             (GT_U8)devNum, (GT_U32)portGroup, (GT_U32)phyPortNum, (MV_HWS_PORT_STANDARD)hwsPortMode, (GT_U8)min_dly, (GT_U8)max_dly);

    rc = mvHwsPortEnhanceTuneLite((GT_U8)devNum,
                                  (GT_U32)portGroup,
                                  (GT_U32)phyPortNum,
                                  (MV_HWS_PORT_STANDARD)hwsPortMode,
                                  (GT_U8)min_dly,
                                  (GT_U8)max_dly);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling light enhanced tune failed=%d", rc);
    }

    return rc;
}

/**
* @internal cpssDxChPortSerdesEnhancedTuneLite function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*                                      min_LF   - Minimum LF value that can be set on Serdes (0...15)
*                                      max_LF   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesEnhancedTuneLite
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U8                  min_dly,
    IN  GT_U8                  max_dly
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesEnhancedTuneLite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, min_dly, max_dly));

    rc = internal_cpssDxChPortSerdesEnhancedTuneLite(devNum, portNum, min_dly, max_dly);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, min_dly, max_dly));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/* --------------------- debug functions --------------------------------- */

/**
* @internal prvDebugCpssDxChPortLpSerdesDump function
* @endinternal
*
* @brief   Dump Port Serdes Registers
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvDebugCpssDxChPortLpSerdesDump
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum
)
{
    GT_U32 regValue;    /* register value */
    GT_U32 regAddr;     /* register address */
    GT_U32 serdes,      /* iterator */
            serdesNum,  /* number of serdeses */
            serdesLast; /* last serdes number + 1*/
    GT_STATUS rc;       /* return code */
    GT_U32  portGroupId;    /* the port group Id - support multi-port-groups device */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;  /* current interface of port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        cpssOsPrintf("Error:Interface mode not defined\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((rc = prvCpssDxChPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,&serdes,&serdesNum)) != GT_OK)
        return rc;

    for(serdesLast = serdes + serdesNum; serdes < serdesLast ; serdes++)
    {
        cpssOsPrintf("\n\r");
        cpssOsPrintf("********************************************************\n");
        cpssOsPrintf("Dump registers for port = %d,  serdes = %d              \n",portNum,serdes);
        cpssOsPrintf("********************************************************\n");

        /* Serdes External Configuration 1 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].serdesExternalReg1;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Serdes External Configuration 2 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].serdesExternalReg2;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* PLL/INTP Register 1 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].pllIntpReg1;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* PLL/INTP Register 2 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].pllIntpReg2;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* PLL/INTP Register 3 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].pllIntpReg3;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* PLL/INTP Register 4 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].pllIntpReg4;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* PLL/INTP Register 5 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].pllIntpReg5;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

         /* Analog Reserved Register 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].analogReservReg;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

           /* Calibration Register 1 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].calibrationReg1;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Calibration Register 2 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].calibrationReg2;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Calibration Register 3 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].calibrationReg3;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Calibration Register 5 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].calibrationReg5;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

         /* Transmitter Register 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].transmitterReg0;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Transmitter Register 1 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].transmitterReg1;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Transmitter Register 2 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].transmitterReg2;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

         /* Receiver Register 1 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].receiverReg1;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Receiver Register 2 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].receiverReg2;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* FFE Register 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].ffeReg;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* SLC Register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].slcReg;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Reference Register 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].referenceReg;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Reset Register 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].resetReg;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

         /* Calibration Register 3 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].calibrationReg3;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Calibration Register 7 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].calibrationReg7;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Reset Register 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].resetReg;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Calibration Register 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].calibrationReg0;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Serdes External Configuration 2 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].serdesExternalReg2;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        cpssOsPrintf("**********************************************************************************\n");
        cpssOsPrintf("Dump Power Register 0, Digital Interface Register 0, Standalone Control Register 0\n");
        cpssOsPrintf("**********************************************************************************\n");
        /* Power Register 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].powerReg;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Digital Interface Register 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].digitalIfReg0;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);

        /* Standalone Control Register 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].standaloneCntrlReg;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId , regAddr,&regValue);
        if (rc != GT_OK)
           return rc;
        cpssOsPrintf("devNum = %d, portGroupId = %d , regAddr = 0x%X, data = 0x%X\n",devNum, portGroupId , regAddr,regValue);


    }/* for */

    return GT_OK;
}

/**
* @internal prvDebugCpssDxChPortHwModeGet function
* @endinternal
*
* @brief   Read from HW current interface and speed of port
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvDebugCpssDxChPortHwModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
)
{
    GT_STATUS                   rc;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;

    rc = prvCpssDxChPortInterfaceModeHwGet(devNum, portNum, &ifMode);
    if(rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChPortInterfaceModeHwGet fail:rc=%d\n", rc);
        return rc;
    }

    cpssOsPrintf("ifMode=%d,", ifMode);

    rc = prvCpssDxChPortSpeedHwGet(devNum, portNum, &speed);
    if(rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChPortSpeedHwGet fail:rc=%d\n", rc);
        return rc;
    }

    cpssOsPrintf("speed=%d\n", speed);

    return GT_OK;
}


/**
* @internal prvCpssElDbOperationToHwsCpssElDbOperation function
* @endinternal
*
* @brief   Conver Cpss EL DB OPERATION Enum to Hws EL DB OPERATION Enum.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] opPtr                    - el db operation.
*
* @param[out] hwsOpPtr                 - Hws el db operation.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
*
* @note Debug function.
*
*/
static GT_STATUS prvCpssElDbOperationToHwsCpssElDbOperation
(
    IN   CPSS_DXCH_PORT_EL_DB_OPERATION_ENT        *opPtr,
    OUT  MV_EL_DB_OPERATION                        *hwsOpPtr
)
{
    switch (*opPtr)
    {
        case CPSS_DXCH_PORT_EL_DB_WRITE_OP_E:
            *hwsOpPtr = WRITE_OP;
            break;
        case CPSS_DXCH_PORT_EL_DB_DELAY_OP_E:
            *hwsOpPtr = DELAY_OP;
            break;
        case CPSS_DXCH_PORT_EL_DB_POLLING_OP_E:
            *hwsOpPtr = POLLING_OP;
            break;
        case CPSS_DXCH_PORT_EL_DB_DUNIT_WRITE_OP_E:
            *hwsOpPtr = DUNIT_WRITE_OP;
            break;
        case CPSS_DXCH_PORT_EL_DB_DUNIT_POLLING_OP_E:
            *hwsOpPtr = DUNIT_POLLING_OP;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


/**
* @internal prvCpssComPhyHSubSeqToHwsComPhyHSubSeq function
* @endinternal
*
* @brief   Conver Cpss Com Phy H Sub Seq Enum to Hws Com Phy H Sub Seq Enum.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] seqType                  - sequence type.
*
* @param[out] HwsSeqTypePtr            - Hws sequence type.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
*
* @note Debug function.
*
*/
static GT_STATUS prvCpssComPhyHSubSeqToHwsComPhyHSubSeq
(
    IN  CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT    seqType,
    OUT MV_HWS_COM_PHY_H_SUB_SEQ                *HwsSeqTypePtr
)
{
    switch (seqType)
    {
        case CPSS_DXCH_PORT_SERDES_SD_RESET_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SD_RESET_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SD_UNRESET_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SD_UNRESET_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_RF_RESET_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_RF_RESET_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_RF_UNRESET_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_RF_UNRESET_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SYNCE_RESET_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SYNCE_RESET_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SYNCE_UNRESET_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SYNCE_UNRESET_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SERDES_POWER_UP_CTRL_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SERDES_POWER_UP_CTRL_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SERDES_POWER_DOWN_CTRL_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SERDES_POWER_DOWN_CTRL_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SERDES_RXINT_UP_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SERDES_RXINT_UP_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SERDES_RXINT_DOWN_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SERDES_RXINT_DOWN_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SERDES_WAIT_PLL_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SERDES_WAIT_PLL_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_1_25G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_1_25G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_3_125G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_3_125G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_3_75G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_3_75G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_4_25G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_4_25G_SEQ ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_5G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_5G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_6_25G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_6_25G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_7_5G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_7_5G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_10_3125G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_10_3125G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SD_LPBK_NORMAL_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SD_LPBK_NORMAL_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SD_ANA_TX_2_RX_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SD_ANA_TX_2_RX_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SD_DIG_TX_2_RX_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SD_DIG_TX_2_RX_SEQ  ;
            break;
        case CPSS_DXCH_PORT_SERDES_SD_DIG_RX_2_TX_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SD_DIG_RX_2_TX_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_PT_AFTER_PATTERN_NORMAL_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_PT_AFTER_PATTERN_NORMAL_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_PT_AFTER_PATTERN_TEST_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_PT_AFTER_PATTERN_TEST_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_RX_TRAINING_ENABLE_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_RX_TRAINING_ENABLE_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_RX_TRAINING_DISABLE_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_RX_TRAINING_DISABLE_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_TX_TRAINING_ENABLE_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_TX_TRAINING_ENABLE_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_TX_TRAINING_DISABLE_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_TX_TRAINING_DISABLE_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_12_5G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_12_5G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_3_3G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_3_3G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_11_5625G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_11_5625G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SERDES_PARTIAL_POWER_DOWN_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SERDES_PARTIAL_POWER_DOWN_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SERDES_PARTIAL_POWER_UP_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SERDES_PARTIAL_POWER_UP_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_11_25G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_11_25G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_CORE_RESET_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_CORE_RESET_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_CORE_UNRESET_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_CORE_UNRESET_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_FFE_TABLE_LR_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_FFE_TABLE_LR_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_FFE_TABLE_SR_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_FFE_TABLE_SR_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_10_9375G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_10_9375G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_12_1875G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_12_1875G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_5_625G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_5_625G_SEQ;
            break;
        case CPSS_DXCH_PORT_SERDES_SPEED_5_15625G_SEQ_E:
            (*HwsSeqTypePtr) = SERDES_SPEED_5_15625G_SEQ;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


/**
* @internal prvSeqLineToHwsSeqLine function
* @endinternal
*
* @brief   Conver Cpss sequence type to Hws sequence type.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] seqLinePtr               - sequence line.
* @param[in] wrParamPtr               - write op params.
* @param[in] delOpPtr                 - delay op params.
* @param[in] polParamPtr              - polling op params.
*
* @param[out] hwsSeqLinePtr            - Hws sequence line.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
*
* @note Debug function.
*
*/
static GT_STATUS prvSeqLineToHwsSeqLine
(
    IN      CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC *seqLinePtr,
    IN      MV_WRITE_OP_PARAMS                      *wrParamPtr,
    IN      MV_DELAY_OP_PARAMS                      *delOpPtr,
    IN      MV_POLLING_OP_PARAMS                    *polParamPtr,
    OUT     MV_CFG_ELEMENT                          *hwsSeqLinePtr
)
{
    prvCpssElDbOperationToHwsCpssElDbOperation(&(seqLinePtr->op), &(hwsSeqLinePtr->op));
    switch (seqLinePtr->op)
    {
    case CPSS_DXCH_PORT_EL_DB_WRITE_OP_E:
            wrParamPtr->indexOffset    = seqLinePtr->indexOffset;
            wrParamPtr->regOffset      = seqLinePtr->regOffset;
            wrParamPtr->data           = seqLinePtr->data;
            wrParamPtr->mask           = seqLinePtr->mask;
            hwsSeqLinePtr->params.unitId    = seqLinePtr->indexOffset;
            hwsSeqLinePtr->params.regOffset = seqLinePtr->regOffset;
            hwsSeqLinePtr->params.operData  = seqLinePtr->data;
            hwsSeqLinePtr->params.mask      = seqLinePtr->mask;
            break;
    case CPSS_DXCH_PORT_EL_DB_DELAY_OP_E:
            delOpPtr->delay            = seqLinePtr->delay;
            hwsSeqLinePtr->params.operData  = delOpPtr->delay;
            break;
    case CPSS_DXCH_PORT_EL_DB_POLLING_OP_E:
            polParamPtr->indexOffset   = seqLinePtr->indexOffset;
            polParamPtr->regOffset     = seqLinePtr->regOffset;
            polParamPtr->data          = seqLinePtr->data;
            polParamPtr->mask          = seqLinePtr->mask;
            hwsSeqLinePtr->params.unitId    = polParamPtr->indexOffset;
            hwsSeqLinePtr->params.regOffset = polParamPtr->regOffset;
            hwsSeqLinePtr->params.operData  = polParamPtr->data;
            hwsSeqLinePtr->params.mask      = polParamPtr->mask;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


/**
* @internal internal_cpssDxChPortSerdesSequenceSet function
* @endinternal
*
* @brief   Set SERDES sequence one line.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] firstLine                - the first line
* @param[in] seqType                  - sequence type
* @param[in] unitId                   - unit Id
* @param[in] seqLinePtr               - sequence line
* @param[in] numOfOp                  - number of op
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesSequenceSet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      portGroup,
    IN  GT_BOOL                                     firstLine,
    IN  CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT        seqType,
    IN  CPSS_DXCH_PORT_UNITS_ID_ENT                 unitId,
    IN  CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC     *seqLinePtr,
    IN  GT_U32                                      numOfOp
)
{
    GT_STATUS                   rc;         /* return code */
    MV_HWS_COM_PHY_H_SUB_SEQ    hwsSeqType;
    MV_CFG_ELEMENT              hwsSeqLine;
    MV_WRITE_OP_PARAMS          wrParam;
    MV_DELAY_OP_PARAMS          delOp;
    MV_POLLING_OP_PARAMS        polParam;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                            CPSS_CH1_E      | CPSS_CH1_DIAMOND_E    | CPSS_CH2_E    |  CPSS_CH3_E      |
                                            CPSS_XCAT2_E    | CPSS_BOBCAT2_E        | CPSS_XCAT3_E  |  CPSS_XCAT_E     | CPSS_LION_E  );

    CPSS_NULL_PTR_CHECK_MAC(seqLinePtr);
    if (seqLinePtr->op == CPSS_DXCH_PORT_EL_DB_WRITE_OP_E || seqLinePtr->op == CPSS_DXCH_PORT_EL_DB_POLLING_OP_E)
    {
        seqLinePtr->indexOffset = unitId;
    }
    rc = prvCpssComPhyHSubSeqToHwsComPhyHSubSeq(seqType, &hwsSeqType);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvSeqLineToHwsSeqLine(seqLinePtr, &wrParam, &delOp, &polParam, &hwsSeqLine);
    if (rc != GT_OK)
    {
        return rc;
    }

    CPSS_LOG_INFORMATION_MAC("Calling: hwsSerdesSeqSet(devNum[%d], portGroup[%d], firstLine[%d], seqType[%d], *seqLine, numOfOp[%d])", devNum, portGroup, firstLine, hwsSeqType, numOfOp);
    rc = hwsSerdesSeqSet(devNum, portGroup, firstLine, hwsSeqType, &hwsSeqLine, numOfOp);
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}

/**
* @internal cpssDxChPortSerdesSequenceSet function
* @endinternal
*
* @brief   Set SERDES sequence one line.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] firstLine                - the first line
* @param[in] seqType                  - sequence type
* @param[in] unitId                   - unit Id
* @param[in] seqLinePtr               - sequence line
* @param[in] numOfOp                  - number of op
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
GT_STATUS cpssDxChPortSerdesSequenceSet
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   portGroup,
    IN GT_BOOL                                  firstLine,
    IN CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT     seqType,
    IN CPSS_DXCH_PORT_UNITS_ID_ENT              unitId,
    IN CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC  *seqLinePtr,
    IN GT_U32                                   numOfOp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesSequenceSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroup, firstLine, seqType, seqLinePtr, numOfOp));

    rc = internal_cpssDxChPortSerdesSequenceSet(devNum, portGroup, firstLine, seqType, unitId, seqLinePtr, numOfOp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroup, firstLine, seqType, seqLinePtr, numOfOp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX data, TX data or both
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat3; xCat2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - number of port group (local core), not used for non-multi-core
* @param[in] serdesNum                - serdes number
* @param[in] numOfBits                - number of bits to inject to serdes data
* @param[in] direction                - whether  is TX, RX or both
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_INIT_ERROR            - on Hws initialization failre
*
* @note Debug function.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesErrorInject
(
    IN GT_U8    devNum,
    IN GT_U32   portGroup,
    IN GT_U32   serdesNum,
    IN GT_U32   numOfBits,
    IN CPSS_PORT_DIRECTION_ENT  direction
)
{
    GT_STATUS   rc;         /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "error inject is currently supported for sip_5_15 devices");
    }

    switch (direction)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            rc = mvHwsAvagoSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits, RX_DIRECTION);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Hws return code is %d", rc);
            }
            break;
        case CPSS_PORT_DIRECTION_TX_E:
            rc = mvHwsAvagoSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits, TX_DIRECTION);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Hws return code is %d", rc);
            }
            break;
        case CPSS_PORT_DIRECTION_BOTH_E:
            rc = mvHwsAvagoSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits, RX_DIRECTION);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Hws return code is %d", rc);
            }
            rc = mvHwsAvagoSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits, TX_DIRECTION);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Hws return code is %d", rc);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong direction=%d", direction);
    }

    return rc;
}

/**
* @internal cpssDxChPortSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX data, TX data or both
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat3; xCat2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - number of port group (local core), not used for non-multi-core
* @param[in] serdesNum                - serdes number
* @param[in] numOfBits                - number of bits to inject to serdes data
* @param[in] direction                - whether  is TX, RX or both
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_INIT_ERROR            - on Hws initialization failre
*
* @note Debug function.
*
*/
GT_STATUS cpssDxChPortSerdesErrorInject
(
    IN GT_U8    devNum,
    IN GT_U32   portGroup,
    IN GT_U32   serdesNum,
    IN GT_U32   numOfBits,
    IN CPSS_PORT_DIRECTION_ENT  direction
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesErrorInject);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroup, serdesNum, numOfBits, direction));

    rc = internal_cpssDxChPortSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits, direction);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroup, serdesNum, numOfBits, direction));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssHwsElDbOperationToCpssElDbOperation function
* @endinternal
*
* @brief   Conver Hws EL DB OPERATION Enum to Cpss EL DB OPERATION Enum.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] hwsOpPtr                 - Hws EL DB OPERATION.
*
* @param[out] opPtr                    - Cpss EL DB OPERATION.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
*
* @note Debug function.
*
*/
static GT_STATUS prvCpssHwsElDbOperationToCpssElDbOperation
(
    IN  MV_EL_DB_OPERATION                      *hwsOpPtr,
    OUT CPSS_DXCH_PORT_EL_DB_OPERATION_ENT      *opPtr
)
{
    switch (*hwsOpPtr)
    {
        case WRITE_OP:
            *opPtr = CPSS_DXCH_PORT_EL_DB_WRITE_OP_E;
            break;
        case DELAY_OP:
            *opPtr = CPSS_DXCH_PORT_EL_DB_DELAY_OP_E;
            break;
        case POLLING_OP:
            *opPtr = CPSS_DXCH_PORT_EL_DB_POLLING_OP_E;
            break;
        case DUNIT_WRITE_OP:
            *opPtr = CPSS_DXCH_PORT_EL_DB_DUNIT_WRITE_OP_E;
            break;
        case DUNIT_POLLING_OP:
            *opPtr = CPSS_DXCH_PORT_EL_DB_DUNIT_POLLING_OP_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvHwsSeqLineToSeqLine function
* @endinternal
*
* @brief   Conver Hws sequence line element to Cpss sequence line element.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] hwsSeqLinePtr            - Hws sequence line.
*
* @param[out] seqLinePtr               - sequence line.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
*
* @note Debug function.
*
*/
static GT_STATUS prvHwsSeqLineToSeqLine
(
    IN  MV_CFG_ELEMENT                          *hwsSeqLinePtr,
    OUT CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC *seqLinePtr
)
{
    prvCpssHwsElDbOperationToCpssElDbOperation(&(hwsSeqLinePtr->op), &(seqLinePtr->op));
    switch (hwsSeqLinePtr->op)
    {
        case POLLING_OP:
        case WRITE_OP:
            seqLinePtr->indexOffset    = hwsSeqLinePtr->params.unitId;
            seqLinePtr->regOffset      = hwsSeqLinePtr->params.regOffset;
            seqLinePtr->data           = hwsSeqLinePtr->params.operData;
            seqLinePtr->mask           = hwsSeqLinePtr->params.mask;
            break;
        case DELAY_OP:

            seqLinePtr->delay = (GT_U16)hwsSeqLinePtr->params.operData;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }
    return GT_OK;
}


/**
* @internal internal_cpssDxChPortSerdesSequenceGet function
* @endinternal
*
* @brief   Get SERDES sequence one line.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] seqType                  - sequence type
* @param[in] lineNum                  - line number
*
* @param[out] seqLinePtr               - sequence line
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
static GT_STATUS internal_cpssDxChPortSerdesSequenceGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      portGroup,
    IN  CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT        seqType,
    IN  GT_U32                                      lineNum,
    OUT CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC     *seqLinePtr
)
{
    GT_STATUS                   rc;         /* return code */
    MV_HWS_COM_PHY_H_SUB_SEQ    hwsSeqType;
    MV_CFG_ELEMENT              hwsSeqLine;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                            CPSS_CH1_E      | CPSS_CH1_DIAMOND_E    | CPSS_CH2_E    |  CPSS_CH3_E      |
                                            CPSS_XCAT2_E    | CPSS_BOBCAT2_E        | CPSS_XCAT3_E  |  CPSS_XCAT_E     | CPSS_LION_E  );

    rc = prvCpssComPhyHSubSeqToHwsComPhyHSubSeq(seqType, &hwsSeqType);
    if(rc != GT_OK)
        return rc;

    CPSS_NULL_PTR_CHECK_MAC(seqLinePtr);

    CPSS_LOG_INFORMATION_MAC("Calling: hwsSerdesSeqGet(devNum[%d], portGroup[%d], seqType[%d], *seqLine, lineNum[%d])", devNum, portGroup, hwsSeqType, lineNum);
    rc = hwsSerdesSeqGet(devNum, portGroup, hwsSeqType, &hwsSeqLine, lineNum);
    if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
        }

    rc = prvHwsSeqLineToSeqLine(&hwsSeqLine, seqLinePtr);

    return rc;
}

/**
* @internal cpssDxChPortSerdesSequenceGet function
* @endinternal
*
* @brief   Get SERDES sequence one line.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] seqType                  - sequence type
* @param[in] lineNum                  - line number
*
* @param[out] seqLinePtr               - sequence line
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
GT_STATUS cpssDxChPortSerdesSequenceGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  portGroup,
    IN  CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT    seqType,
    IN  GT_U32                                  lineNum,
    OUT CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC *seqLinePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSerdesSequenceGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroup, seqType, lineNum, seqLinePtr));

    rc = internal_cpssDxChPortSerdesSequenceGet(devNum, portGroup, seqType, lineNum, seqLinePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroup, seqType, lineNum, seqLinePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortSerdesFunctionsObjInit function
* @endinternal
*
* @brief   Init and bind common function pointers to DXCH port serdes functions.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChPortSerdesFunctionsObjInit
(
    IN GT_U8 devNum
)
{
    CPSS_LOG_INFORMATION_MAC("binding dxch port serdes functions");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* common functions bind - (currently for Port Manager use) */
    PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneStatusGetFunc = prvCpssDxChPortSerdesAutoTuneStatusGetWrapper;
    PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSigDetGetFunc = prvCpssDxChPortSerdesSignalDetectGetWrapper;
    PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppTuneExtSetFunc = prvCpssDxChPortSerdesAutoTuneExtWrapper;
    PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLaneTuneSetFunc = prvCpssDxChPortSerdesLaneTuningSetWrapper;
    PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppLaneTuneGetFunc = prvCpssDxChPortSerdesLaneTuningGetWrapper;
    PRV_CPSS_PP_MAC(devNum)->ppCommonPortFuncPtrsStc.ppSerdesNumGetFunc = prvCpssDxChPortIfModeSerdesNumGetWrapper;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSerdesAutoTuneStatusGetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function prvCpssDxChPortSerdesAutoTuneStatusGet
*         in order to use in common code.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] rxTuneStatusPtr          - (pointer to) rx tune status parameter
* @param[out] rxTuneStatusPtr          - (pointer to) tx tune status parameter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPortSerdesAutoTuneStatusGetWrapper
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatusTemp;
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatusTemp;

    CPSS_LOG_INFORMATION_MAC("inside PortSerdesAutoTuneStatusGetWrapper function wrapper");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    CPSS_NULL_PTR_CHECK_MAC(rxTuneStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(txTuneStatusPtr);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(CAST_SW_DEVNUM(devNum), portNum, portMacNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum),
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
         | CPSS_LION_E | CPSS_XCAT2_E  );

    rc = cpssDxChPortSerdesAutoTuneStatusGet(CAST_SW_DEVNUM(devNum), portNum, &rxTuneStatusTemp, &txTuneStatusTemp);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "AutoTuneStatusGetWrapper failed=%d", rc);
    }

    *rxTuneStatusPtr = rxTuneStatusTemp;
    *txTuneStatusPtr = txTuneStatusTemp;

    return GT_OK;
}


/**
* @internal prvCpssDxChPortSerdesSignalDetectGetWrapper function
* @endinternal
*
* @brief   convert input from GT_SW_DEV_NUM to U8
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*/
GT_STATUS prvCpssDxChPortSerdesSignalDetectGetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT GT_BOOL                        *signalStatePtr

)
{
    return cpssDxChPortSerdesSignalDetectGet(CAST_SW_DEVNUM(devNum), portNum, signalStatePtr);
}


/**
* @internal prvCpssDxChPortSerdesAutoTuneExtWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function cpssDxChPortSerdesAutoTune
*         with extended implementation in order to use in common code.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API contain extended implementation currently needed ony for
*       Port-Manager.
*
*/
GT_STATUS prvCpssDxChPortSerdesAutoTuneExtWrapper
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;

    CPSS_LOG_INFORMATION_MAC("inside PortSerdesAutoTuneExtWrapper function wrapper");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(CAST_SW_DEVNUM(devNum), portNum, portMacNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum),
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
         | CPSS_LION_E | CPSS_XCAT2_E  );

    if ( PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) )
    {
        /* TBD #1 - A part of this tune purpose is to raise SIGNAL_DETECT_CHANGE interrupts. In order to do that, we
        need to disable rx-idle-detector(seuqlch) from the Hw AND operation which is operated
        between the rx-idle-detector and the CDR lock. The reason is that wrong rx-idle-detector value
        can cause the rx training to not raise this interrupt. */
    }

    rc = cpssDxChPortSerdesAutoTune(CAST_SW_DEVNUM(devNum), portNum, (CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT)portTuningMode);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "AutoTuneExtWrapper failed=%d", rc);
    }

    if ( PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) )
    {
        /* TBD #2 - A part of this tune purpose is to raise SIGNAL_DETECT_CHANGE interrupts. In order to do that, we
        need to disable rx-idle-detector(seuqlch) from the Hw AND operation which is operated
        between the rx-idle-detector and the CDR lock. The reason is that wrong rx-idle-detector value
        can cause the rx training to not raise this interrupt. */
    }

    return rc;
}

/**
* @internal prvCpssDxChPortSerdesLaneTuningSetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function cpssDxChPortSerdesLaneTuningSet
*         in order to use in common code.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortSerdesLaneTuningSetWrapper
(
    IN GT_SW_DEV_NUM           devNum,
    IN GT_U32                  portGroupNum,
    IN GT_U32                  laneNum,
    IN CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN CPSS_PORT_SERDES_TUNE_STC  *tuneValuesPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_INFORMATION_MAC("inside PortSerdesLaneTuningSetWrapper function wrapper");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum), CPSS_CH1_E |
                                CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    rc = cpssDxChPortSerdesLaneTuningSet(CAST_SW_DEVNUM(devNum),portGroupNum,laneNum,
                                    (CPSS_DXCH_PORT_SERDES_SPEED_ENT)serdesFrequency,
                                    (CPSS_DXCH_PORT_SERDES_TUNE_STC*)tuneValuesPtr);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "PortSerdesLaneTuningSetWrapper failed=%d", rc);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortSerdesLaneTuningGetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function cpssDxChPortSerdesLaneTuningGet
*         in order to use in common code.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortSerdesLaneTuningGetWrapper
(
    IN   GT_SW_DEV_NUM  devNum,
    IN   GT_U32         portGroupNum,
    IN   GT_U32         laneNum,
    IN   CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT  CPSS_PORT_SERDES_TUNE_STC  *tuneValuesPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_INFORMATION_MAC("inside PortSerdesLaneTuningGetWrapper function wrapper");

    PRV_CPSS_DXCH_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum));
    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(CAST_SW_DEVNUM(devNum), CPSS_CH1_E |
                                CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    rc = cpssDxChPortSerdesLaneTuningGet(CAST_SW_DEVNUM(devNum),portGroupNum,laneNum,
                                    (CPSS_DXCH_PORT_SERDES_SPEED_ENT)serdesFrequency,
                                    (CPSS_DXCH_PORT_SERDES_TUNE_STC*)tuneValuesPtr);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "SerdesLaneTuningGetWrapper failed=%d", rc);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortIfModeSerdesNumGetWrapper function
* @endinternal
*
* @brief   convert input from GT_SW_DEV_NUM to U8
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*/
GT_STATUS prvCpssDxChPortIfModeSerdesNumGetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    OUT GT_U32                          *startSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
)
{
    return prvCpssDxChPortIfModeSerdesNumGet(CAST_SW_DEVNUM(devNum), portNum, ifMode, startSerdesPtr, numOfSerdesLanesPtr);
}

/**
* @internal prvCpssDxChPortSerdesLoopbackModeSetWrapper function
* @endinternal
*
* @brief   Configure loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) where to
*                                      set loopback (not used for Lion2)
* @param[in] mode                     - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Pay attention - when new loopback mode enabled on serdes lane,
*       previous mode disabled
*
*/
GT_STATUS prvCpssDxChPortSerdesLoopbackModeSetWrapper
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneBmp,
    IN  CPSS_PORT_SERDES_LOOPBACK_MODE_ENT  mode
)
{
    GT_STATUS rc;

    rc = cpssDxChPortSerdesLoopbackModeSet(CAST_SW_DEVNUM(devNum),portNum,laneBmp,(CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT)mode);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling cpssDxChPortSerdesLoopbackModeSet from wrapper failed=%d",rc);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortSerdesLanePolaritySetWrapper function
* @endinternal
*
* @brief   Wrapper function Set the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3; Bobcat2;
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
* @param[in] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[in] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - If serdes initialized: Set the Tx/Rx polarity parameters for lane
*       in both HW and SW DB
*       - If serdes was not initialized: Set the Tx/Rx polarity parameters
*       for lane only in SW DB
*
*/
GT_STATUS prvCpssDxChPortSerdesLanePolaritySetWrapper
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32      portGroupNum,
    IN  GT_U32      laneNum,
    IN  GT_BOOL     invertTx,
    IN  GT_BOOL     invertRx
)
{
    GT_STATUS rc;

    rc = cpssDxChPortSerdesLanePolaritySet(CAST_SW_DEVNUM(devNum),portGroupNum,laneNum,invertTx,invertRx);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssDxChPortSerdesLanePolaritySet from wrapper failed=%d",rc);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortSerdesAutoTuneResultsGetWrapper
*           function
* @endinternal
*
* @brief   Wrapper function Get the SerDes Tune Result.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3; Bobcat2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of port
* @param[in] laneNum                  - number of SERDES lane
* @param[out] serdesTunePtr           - Pointer to Tuning
*       result.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortSerdesAutoTuneResultsGetWrapper
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 laneNum,
    OUT CPSS_PORT_SERDES_TUNE_STC *serdesTunePtr
)
{
    GT_STATUS rc;

    rc = cpssDxChPortSerdesAutoTuneResultsGet(CAST_SW_DEVNUM(devNum), portNum, laneNum, (CPSS_DXCH_PORT_SERDES_TUNE_STC*)serdesTunePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling cpssDxChPortSerdesAutoTuneResultsGet from wrapper failed=%d",rc);
    }

    return rc;
}

/**
* @internal prvCpssDxChAutoNeg1GSgmiiWrapper function
* @endinternal
*
* @brief   Auto-Negotiation sequence for 1G QSGMII/SGMII
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of port
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChAutoNeg1GSgmiiWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_BOOL                        enable
)
{
    GT_STATUS rc;

    rc = cpssDxChPortInbandAutoNegEnableSet(CAST_SW_DEVNUM(devNum), portNum, enable);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = cpssDxChPortDuplexAutoNegEnableSet(CAST_SW_DEVNUM(devNum),portNum, enable);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = cpssDxChPortSpeedAutoNegEnableSet(CAST_SW_DEVNUM(devNum),portNum, enable);
    if (rc != GT_OK)
    {
        return rc;
    }

    return rc;
}

/**
* @internal prvDebugCpssDxChPortTrxTune function
* @endinternal
*
* @brief   Run TRX training on pair of ports - needed at this moment for HW testing
*         because TRX training must be started on both connected ports more or less
*         simultaneously and automatic TRX training mechanism too complicated for that.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum0                 - first physical port number
* @param[in] portNum1                 - second physical port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS prvDebugCpssDxChPortTrxTune
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum0,
    IN  GT_PHYSICAL_PORT_NUM                     portNum1
)
{
    GT_STATUS rc;

    rc = cpssDxChPortSerdesAutoTune(devNum, portNum0,
                        CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChPortSerdesAutoTune port %d failed\n", portNum0);
        return rc;
    }

    rc = cpssDxChPortSerdesAutoTune(devNum, portNum1,
                        CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChPortSerdesAutoTune port %d failed\n", portNum1);
        return rc;
    }

    return GT_OK;
}


/* ----- end of debug section; please don't place below real API's ------ */

