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
* @file mvHwsBobcat3DevInit.c
*
* @brief Bobcat3 specific HW Services init
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <silicon/bobk/mvHwsBobKPortIf.h>
#include <silicon/bobcat3/mvHwsBobcat3PortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcs28nmIf.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>

#ifndef MV_HWS_REDUCED_BUILD
#include <mvDdr3TrainingIpPrvIf.h>
#include <mvHwsDdr3BobK.h>
#endif

/************************* definition *****************************************************/

#define BOBCAT3_MPCS_BASE           (0x10180000)
#define BOBCAT3_CGPCS_BASE          (0x10300000)
#define BOBCAT3_GIG_BASE            (0x10000000)
#define BOBCAT3_XLG_BASE            (0x100C0000)
#define BOBCAT3_SERDES_BASE         (0x13000000)
#define BOBCAT3_CG_BASE             (0x10340000)
#define BOBCAT3_CG_RS_FEC_BASE      (0x10340800)
#define BOBCAT3_PTP_BASE            (0x10180800)

#define IND_OFFSET                  (0x1000)
#define CG_IND_OFFSET               (0x1000)

#define BOBCAT3_SERDES_MAP_ARR_SIZE         75

/* MACRO to add pipe offset to serdes sbus_address to match avago implementation */
#define HWS_AVAGO_SERDES_PIPE_MAC(sd,pipe)   \
        ( sd | pipe << 8 )

#define HWS_AVAGO_SERDES_PIPE_1_MAC(sd)   \
        HWS_AVAGO_SERDES_PIPE_MAC(sd,1)

extern HWS_UNIT_BASE_ADDR_CALC_BIND    falconBaseAddrCalcFunc;

/* laneNum for SBC_Unit_0: #0 to #35, and #72. laneNum for SBC_Unit_1: #36 to #71,
   and #73. TSEN for SBC_Unit_0 is #26, TSEN for SBC_Unit_1 is #66 */
static GT_U32 bobcat3SerdesToAvagoMap[BOBCAT3_MAX_AVAGO_SERDES_NUMBER] =
{
/* laneNum   SBC_Serdes */
 /* [0]  */      2,
 /* [1]  */      3,
 /* [2]  */      4,
 /* [3]  */      5,
 /* [4]  */      6,
 /* [5]  */      7,
 /* [6]  */      8,
 /* [7]  */      9,
 /* [8]  */      10,
 /* [9]  */      11,
 /* [10] */      12,
 /* [11] */      13,
 /* [12] */      14,
 /* [13] */      15,
 /* [14] */      16,
 /* [15] */      17,
 /* [16] */      18,
 /* [17] */      19,
 /* [18] */      20,
 /* [19] */      21,
 /* [20] */      22,
 /* [21] */      23,
 /* [22] */      24,
 /* [23] */      25,
 /* [24] */      27,
 /* [25] */      28,
 /* [26] */      29,
 /* [27] */      30,
 /* [28] */      31,
 /* [29] */      32,
 /* [30] */      33,
 /* [31] */      34,
 /* [32] */      35,
 /* [33] */      36,
 /* [34] */      37,
 /* [35] */      38,
 /* [36] */      HWS_AVAGO_SERDES_PIPE_1_MAC(2),  /* end of SBC_Unit_0 */
 /* [37]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(3),
 /* [38]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(4),
 /* [39]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(5),
 /* [40]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(6),
 /* [41]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(7),
 /* [42]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(8),
 /* [43]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(9),
 /* [44]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(10),
 /* [45]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(11),
 /* [46]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(12),
 /* [47] */      HWS_AVAGO_SERDES_PIPE_1_MAC(13),
 /* [48] */      HWS_AVAGO_SERDES_PIPE_1_MAC(14),
 /* [49] */      HWS_AVAGO_SERDES_PIPE_1_MAC(15),
 /* [50] */      HWS_AVAGO_SERDES_PIPE_1_MAC(16),
 /* [51] */      HWS_AVAGO_SERDES_PIPE_1_MAC(17),
 /* [52] */      HWS_AVAGO_SERDES_PIPE_1_MAC(18),
 /* [53] */      HWS_AVAGO_SERDES_PIPE_1_MAC(19),
 /* [54] */      HWS_AVAGO_SERDES_PIPE_1_MAC(20),
 /* [55] */      HWS_AVAGO_SERDES_PIPE_1_MAC(21),
 /* [56] */      HWS_AVAGO_SERDES_PIPE_1_MAC(22),
 /* [57] */      HWS_AVAGO_SERDES_PIPE_1_MAC(23),
 /* [58] */      HWS_AVAGO_SERDES_PIPE_1_MAC(24),
 /* [59] */      HWS_AVAGO_SERDES_PIPE_1_MAC(25),
 /* [60] */      HWS_AVAGO_SERDES_PIPE_1_MAC(27),
 /* [61] */      HWS_AVAGO_SERDES_PIPE_1_MAC(28),
 /* [62] */      HWS_AVAGO_SERDES_PIPE_1_MAC(29),
 /* [63] */      HWS_AVAGO_SERDES_PIPE_1_MAC(30),
 /* [64] */      HWS_AVAGO_SERDES_PIPE_1_MAC(31),
 /* [65] */      HWS_AVAGO_SERDES_PIPE_1_MAC(32),
 /* [66] */      HWS_AVAGO_SERDES_PIPE_1_MAC(33),
 /* [67] */      HWS_AVAGO_SERDES_PIPE_1_MAC(34),
 /* [68] */      HWS_AVAGO_SERDES_PIPE_1_MAC(35),
 /* [69] */      HWS_AVAGO_SERDES_PIPE_1_MAC(36),
 /* [70] */      HWS_AVAGO_SERDES_PIPE_1_MAC(37),
 /* [71] */      HWS_AVAGO_SERDES_PIPE_1_MAC(38),
 /* [72] */      1,
 /* [73] */      HWS_AVAGO_SERDES_PIPE_1_MAC(1) /* end of SBC_Unit_1 */
};

/* SBC_Serdes for a single SBC_Unit: #0 to #38.
   TSEN for a single SBC_Unit is #26*/
GT_U32 bobcat3AvagoToSerdesMap[BOBCAT3_SERDES_MAP_ARR_SIZE] =
{
/* SBC_Serdes  laneNum */
/* [0]  */       0, /* NOT USED*/
/* [1]  */       36,
/* [2]  */       0,
/* [3]  */       1,
/* [4]  */       2,
/* [5]  */       3,
/* [6]  */       4,
/* [7]  */       5,
/* [8]  */       6,
/* [9]  */       7,
/* [10] */       8,
/* [11] */       9,
/* [12] */      10,
/* [13] */      11,
/* [14] */      12,
/* [15] */      13,
/* [16] */      14,
/* [17] */      15,
/* [18] */      16,
/* [19] */      17,
/* [20] */      18,
/* [21] */      19,
/* [22] */      20,
/* [23] */      21,
/* [24] */      22,
/* [25] */      23,
/* [26] */       0,    /* TSEN */
/* [27] */      24,
/* [28] */      25,
/* [29] */      26,
/* [30] */      27,
/* [31] */      28,
/* [32] */      29,
/* [33] */      30,
/* [34] */      31,
/* [35] */      32,
/* [36] */      33,
/* [37] */      34,
/* [38] */      35    /* end of SBC_Unit*/
};

/************************* Globals *******************************************************/

extern MV_HWS_PORT_INIT_PARAMS *hwsPortsBobcat3Params[];

extern MV_HWS_PORT_INIT_PARAMS hwsBobcat3Port0SupModes[];
extern MV_HWS_PORT_INIT_PARAMS hwsBobcat3Port72SupModes[];
extern MV_HWS_PORT_INIT_PARAMS *hwsPortsBobcat3ParamsSupModesMap[];

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL bobcat3DbInitDone = GT_FALSE;

/************************* pre-declaration ***********************************************/
#if (!defined MV_HWS_REDUCED_BUILD)
extern GT_STATUS mvHwsGeneralClkSelCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);
#endif

/*****************************************************************************************/

/**
* @internal hwsBobcat3ConvertSbusAddrToSerdes function
* @endinternal
*
* @brief   Convert sbus address to serdes number.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int hwsBobcat3ConvertSbusAddrToSerdes
(
    unsigned char devNum,
    GT_U32 *serdesNum,
    unsigned int  sbusAddr
)
{
    GT_U32 localIndex = 0;
    GT_U32 tempValue = 0;

    if (sbusAddr & (1 << 8))
    {
        localIndex = 1;
        sbusAddr &= 0xFF;
    }

    if(sbusAddr > hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber)
    {
        return GT_BAD_PARAM;
    }

    tempValue = hwsDeviceSpecInfo[devNum].serdesInfo.avagoToSerdesMapPtr[sbusAddr];

    if (tempValue == 36)
    {
        *serdesNum = (localIndex == 0) ? 72 : 73;
    }
    else
    {
        *serdesNum = tempValue + 36 * localIndex;
    }
    return GT_OK;
}

/**
* @internal pcsIfInit function
* @endinternal
*
* @brief   Init all supported PCS types relevant for devices.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS pcsIfInit(GT_U8 devNum)
{
    MV_HWS_PCS_FUNC_PTRS *hwsPcsFuncsPtr;

    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsGPcs28nmIfInit(hwsPcsFuncsPtr));  /* GPCS */
    CHECK_STATUS(mvHwsMMPcs28nmIfInit(hwsPcsFuncsPtr)); /* MMPCS */
    CHECK_STATUS(mvHwsCgPcs28nmIfInit(devNum, hwsPcsFuncsPtr)); /* CGPCS */

    return GT_OK;
}

/**
* @internal macIfInit function
* @endinternal
*
* @brief   Init all supported MAC types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS macIfInit(GT_U8 devNum)
{
    MV_HWS_MAC_FUNC_PTRS *hwsMacFuncsPtr;

    CHECK_STATUS(hwsMacGetFuncPtr(devNum, &hwsMacFuncsPtr));

    CHECK_STATUS(hwsGeMac28nmIfInit (devNum, hwsMacFuncsPtr));  /* GE MAC  */
    CHECK_STATUS(hwsXgMac28nmIfInit (hwsMacFuncsPtr));  /* XG MAC  */
    CHECK_STATUS(hwsXlgMac28nmIfInit(devNum, hwsMacFuncsPtr));  /* XLG MAC */
    CHECK_STATUS(hwsCgMac28nmIfInit (devNum, hwsMacFuncsPtr));  /* CG MAC  */
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
    /* ILKN/SG configuration is not done in Internal CPU */
    CHECK_STATUS(hwsGeMacSgIfInit(devNum, hwsMacFuncsPtr));   /* GE_SG MAC */
#endif
    return GT_OK;
}

/**
* @internal hwsBobcat3SerdesIfInit function
* @endinternal
*
* @brief   Init all supported Serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobcat3SerdesIfInit(GT_U8 devNum)
{
  MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;

  CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

  CHECK_STATUS(mvHwsAvagoIfInit(devNum, hwsSerdesFuncsPtr));

  return GT_OK;
}

/**
* @internal hwsBobcat3IfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
* @param[in] funcPtr                  - pointer to structure that hold the "os"
*                                      functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobcat3IfPreInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    GT_U32 i;
    if (funcPtr == NULL)
        return GT_FAIL;

    if ((funcPtr->osTimerWkPtr == NULL) || (funcPtr->osMemSetPtr == NULL) ||
        (funcPtr->osFreePtr == NULL) || (funcPtr->osMallocPtr == NULL) ||
        (funcPtr->osExactDelayPtr == NULL) || (funcPtr->sysDeviceInfo == NULL) ||
        (funcPtr->osMemCopyPtr == NULL) || (funcPtr->serdesRegSetAccess == NULL) ||
        (funcPtr->serdesRegGetAccess == NULL) || (funcPtr->serverRegSetAccess == NULL) ||
        (funcPtr->serverRegGetAccess == NULL) ||
        (funcPtr->registerSetAccess == NULL) || (funcPtr->registerGetAccess == NULL)
#ifndef MV_HWS_FREE_RTOS
        || (funcPtr->timerGet == NULL)
        || (funcPtr->osStrCatPtr == NULL)
#endif
        )
    {
        return GT_BAD_PTR;
    }

    hwsOsExactDelayPtr = funcPtr->osExactDelayPtr;
    hwsOsTimerWkFuncPtr = funcPtr->osTimerWkPtr;
    hwsOsMemSetFuncPtr = funcPtr->osMemSetPtr;
    hwsOsFreeFuncPtr = funcPtr->osFreePtr;
    hwsOsMallocFuncPtr = funcPtr->osMallocPtr;
    hwsSerdesRegSetFuncPtr = funcPtr->serdesRegSetAccess;
    hwsSerdesRegGetFuncPtr = funcPtr->serdesRegGetAccess;
    hwsServerRegSetFuncPtr = funcPtr->serverRegSetAccess;
    hwsServerRegGetFuncPtr = funcPtr->serverRegGetAccess;
    hwsOsMemCopyFuncPtr = funcPtr->osMemCopyPtr;
    hwsOsStrCatFuncPtr = funcPtr->osStrCatPtr;
    hwsServerRegFieldSetFuncPtr = funcPtr->serverRegFieldSetAccess;
    hwsServerRegFieldGetFuncPtr = funcPtr->serverRegFieldGetAccess;
#ifndef MV_HWS_FREE_RTOS
    hwsTimerGetFuncPtr = funcPtr->timerGet;
#endif
    hwsRegisterSetFuncPtr = funcPtr->registerSetAccess;
    hwsRegisterGetFuncPtr = funcPtr->registerGetAccess;

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }

    hwsDeviceSpecInfo[devNum].devType = Bobcat3;

    return GT_OK;
}

/**
* @internal hwsBobcat3IfInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
* @param[in] funcPtr                  - pointer to structure that hold the "os"
*                                      functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobcat3IfInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    GT_U32  portGroup = devNum;
    GT_U32  data, version;
    MV_HWS_DEV_TYPE devType;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    GT_U32  i;

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }

#ifdef MV_HWS_REDUCED_BUILD
    if(bobcat3DbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsBobcat3IfPreInit(devNum, funcPtr));
    }
#endif

    hwsDeviceSpecInfo[devNum].devType = Bobcat3;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].portsNum = HWS_BC3_PORTS_NUM_CNS;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = AVAGO;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber = MAX_AVAGO_SERDES_NUMBER;
    hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber = BOBCAT3_SERDES_MAP_ARR_SIZE;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesToAvagoMapPtr = &bobcat3SerdesToAvagoMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.avagoToSerdesMapPtr = &bobcat3AvagoToSerdesMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride = GT_TRUE;
    hwsDeviceSpecInfo[devNum].serdesInfo.sbusAddrToSerdesFuncPtr = hwsBobcat3ConvertSbusAddrToSerdes;
    for (i = 0; i < MV_HWS_MAX_CTLE_BIT_MAPS_NUM; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasOverride[i] = 0;
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[i] = 0;
    }
    hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum = 0;
    hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum = 0;

    devType = hwsDeviceSpecInfo[devNum].devType;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc == NULL)
    {
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc = funcPtr->sysDeviceInfo;
    }

#ifndef MV_HWS_FREE_RTOS
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc == NULL)
    {
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc = funcPtr->coreClockGetPtr;
    }
#endif

#ifndef MV_HWS_REDUCED_BUILD
    /* define IPC connection Type Host side */
    if (mvHwsServiceCpuEnableGet(devNum))
    {
        /* AP FW is loaded to Service CPU, use IPC mode - API execution at the Service CPU */

        /* !!! Replace HOST2HWS_LEGACY_MODE with HOST2SERVICE_CPU_IPC_CONNECTION once approved by CPSS !!! */
        hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE/*HOST2SERVICE_CPU_IPC_CONNECTION*/;
        /* Init HWS Firmware */
        CHECK_STATUS(mvHwsServiceCpuFwInit(devNum, 0, "AP_Bobcat3"));
    }
    else
    {
        /* AP FW is NOT loaded to Service CPU, use Legacy mode - API execution at the Host */
        hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
    }

    if (mvHwsServiceCpuDbaEnableGet(devNum))
    {
        /* DBA FW is loaded to Second Service CPU */

        CHECK_STATUS(mvHwsServiceCpuDbaFwInit(devNum));
    }
    hwsDeviceSpecInfo[devNum].portsParamsSupModesMap                    = hwsPortsBobcat3ParamsSupModesMap;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc         = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc    = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc = mvHwsGeneralClkSelCfg;
#else
    /* define IPC connection Type Service CPU side */
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
#endif /* MV_HWS_REDUCED_BUILD */

    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);

    hwsDeviceSpecInfo[devNum].gopRev = GOP_28NM_REV3;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

    /* Configures port init / reset functions */
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc = mvHwsGeneralPortInit;
#endif
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = mvHwsGeneralApPortInit;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsGeneralPortReset;

    /* Configures  device handler functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = mvHwsPortAvagoAutoTuneSetExt;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = NULL;                         /* TBD - Need to be implemented */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = NULL;                         /* TBD - Need to be implemented */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portLbSetWaFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc   = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func     = NULL;

    hwsInitPortsModesParam(devNum,NULL);
    portGroup = portGroup;
    if(hwsInitDone[devNum] == GT_FALSE)
    {
#ifndef MV_HWS_REDUCED_BUILD
        CHECK_STATUS(hwsDevicePortsElementsCfg(devNum));
#else
        CHECK_STATUS(hwsBobcat3PortsElementsCfg(devNum, portGroup));
#endif
    }

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(hwsBobcat3SerdesIfInit(devNum));

    /* Init port manager db */
#ifndef MV_HWS_FREE_RTOS
    CHECK_STATUS_EXT(mvHwsPortManagerInit(devNum), LOG_ARG_STRING_MAC("port manager init failed"));
#endif /* MV_HWS_FREE_RTOS */

#ifdef MV_HWS_FREE_RTOS
    CHECK_STATUS_EXT(mvHwsPortEnhanceTuneLitePhaseDeviceInit(devNum),
                     LOG_ARG_STRING_MAC("EnhanceTuneLite device init failed"));
#endif

    /* Update HWS After Firmware load and init */
    if (mvHwsServiceCpuEnableGet(devNum))
        CHECK_STATUS(mvHwsServiceCpuFwPostInit(devNum));

    /* Init configuration sequence executer */
    mvCfgSeqExecInit();

    mvUnitInfoSet(devType,  SERDES_UNIT,     BOBCAT3_SERDES_BASE,     IND_OFFSET);
    mvUnitInfoSet(devType,  MMPCS_UNIT,      BOBCAT3_MPCS_BASE,       IND_OFFSET);
    mvUnitInfoSet(devType,  CGPCS_UNIT,      BOBCAT3_CGPCS_BASE,      CG_IND_OFFSET);
    mvUnitInfoSet(devType,  GEMAC_UNIT,      BOBCAT3_GIG_BASE,        IND_OFFSET);
    mvUnitInfoSet(devType,  XLGMAC_UNIT,     BOBCAT3_XLG_BASE,        IND_OFFSET);
    mvUnitInfoSet(devType,  CG_UNIT,         BOBCAT3_CG_BASE,         CG_IND_OFFSET);
    mvUnitInfoSet(devType,  CG_RS_FEC_UNIT,  BOBCAT3_CG_RS_FEC_BASE,  CG_IND_OFFSET);
    mvUnitInfoSet(devType,  PTP_UNIT,        BOBCAT3_PTP_BASE,        IND_OFFSET);

    bobcat3DbInitDone = GT_TRUE;
    hwsInitDone[devNum] = GT_TRUE;

    return GT_OK;
}


/**
* @internal hwsBobcat3SerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for BC3
*
* @param[in] serdesNum                - serdes lane number
* @param[in] regAddr                  - offset of required register in SD Unit
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsBobcat3SerdesAddrCalc
(
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
)
{
    GT_U32 pipe;

    if (serdesNum <= 71)
    {
        pipe = (serdesNum / 36) * 0x80000;
        *addressPtr = (0x13000000 + regAddr + 0x1000 * (serdesNum % 36)) | pipe;
    }
    else if ((serdesNum == 72) || (serdesNum == 73))
    {
        pipe = (serdesNum % 2) * 0x80000;
        *addressPtr = (0x13000000 + regAddr + (0x1000 * 36)) | pipe;
    }

    return;
}

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal serdesIfClose function
* @endinternal
*
* @brief   Free all resources allocated by supported serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static void serdesIfClose(GT_U8 devNum)
{
    mvHwsAvagoIfClose(devNum);
}

/**
* @internal hwsBobcat3IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsBobcat3IfClose(GT_U8 devNum)
{
    GT_U32  i;

    if (hwsInitDone[devNum])
    {
        hwsInitDone[devNum] = GT_FALSE;
    }

    /* if there is no active device remove DB */
    for (i = 0; i < HWS_MAX_DEVICE_NUM; i++)
    {
        if (hwsInitDone[i])
        {
            return;
        }
    }

    bobcat3DbInitDone = GT_FALSE;

    serdesIfClose(devNum);
#ifndef MV_HWS_FREE_RTOS
    /* we can use this API also in simulation, to be consistent with port manager hws APIs,
       they just return GT_OK */
    mvHwsPortManagerClose(devNum);
#endif /* MV_HWS_FREE_RTOS */

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsPortsElementsClose(devNum);
}

#endif

