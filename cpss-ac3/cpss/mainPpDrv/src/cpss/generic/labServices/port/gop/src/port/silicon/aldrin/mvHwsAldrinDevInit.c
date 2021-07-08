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
* @file mvHwsAldrinDevInit.c
*
* @brief Aldrin specific HW Services init
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
#include <silicon/aldrin/mvHwsAldrinPortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/ilknMac/mvHwsIlknMacIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/ilknPcs/mvHwsIlknPcsIf.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>

#ifndef MV_HWS_REDUCED_BUILD
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <mvDdr3TrainingIpPrvIf.h>
#include <mvHwsDdr3BobK.h>
#endif


/************************* definition *****************************************************/

#define ALDRIN_GIG_BASE        (0x10000000)
#define ALDRIN_XPCS_BASE       (0x10180400)
#define ALDRIN_XLG_BASE        (0x100C0000)
#define ALDRIN_MPCS_BASE       (0x10180000)

#define ALDRIN_SERDES_BASE     (0x13000000)

#define ALDRIN_PTP_BASE        (0x10180800)

#define IND_OFFSET (0x1000)

#define ALDRIN_SERDES_MAP_ARR_SIZE      34


static GT_U32 aldrinSerdesToAvagoMap[ALDRIN_SERDES_MAP_ARR_SIZE] =
{
/* laneNum   SBC_Serdes */
 /* [0]  */     18,
 /* [1]  */     17,
 /* [2]  */     19,
 /* [3]  */     16,
 /* [4]  */     20,
 /* [5]  */     15,
 /* [6]  */     21,
 /* [7]  */     14,
 /* [8]  */     12,
 /* [9]  */     23,
 /* [10] */     13,
 /* [11] */     22,
 /* [12] */     24,
 /* [13] */     11,
 /* [14] */     25,
 /* [15] */     10,
 /* [16] */     26,
 /* [17] */      8,
 /* [18] */     27,
 /* [19] */      7,
 /* [20] */     28,
 /* [21] */      6,
 /* [22] */     29,
 /* [23] */      5,
 /* [24] */     30,
 /* [25] */      4,
 /* [26] */     31,
 /* [27] */      3,
 /* [28] */      1,
 /* [29] */     33,
 /* [30] */      2,
 /* [31] */     32,
 /* [32] */     34
};

static GT_U32 aldrinAvagoToSerdesMap[ALDRIN_MAX_AVAGO_SERDES_NUMBER] =
{
/* SBC_Serdes  laneNum */
 /*  [0] */       0,
 /*  [1] */      28,
 /*  [2] */      30,
 /*  [3] */      27,
 /*  [4] */      25,
 /*  [5] */      23,
 /*  [6] */      21,
 /*  [7] */      19,
 /*  [8] */      17,
 /* TSEN=9 */     0,
 /*  [10] */     15,
 /*  [11] */     13,
 /*  [12] */      8,
 /*  [13] */     10,
 /*  [14] */      7,
 /*  [15] */      5,
 /*  [16] */      3,
 /*  [17] */      1,
 /*  [18] */      0,
 /*  [19] */      2,
 /*  [20] */      4,
 /*  [21] */      6,
 /*  [22] */     11,
 /*  [23] */      9,
 /*  [24] */     12,
 /*  [25] */     14,
 /*  [26] */     16,
 /*  [27] */     18,
 /*  [28] */     20,
 /*  [29] */     22,
 /*  [30] */     24,
 /*  [31] */     26,
 /*  [32] */     31,
 /*  [33] */     29,
 /*  [34] */     32
};

/************************* Globals *******************************************************/

extern MV_HWS_PORT_INIT_PARAMS *hwsPortsAldrinParams[];

extern MV_HWS_PORT_INIT_PARAMS hwsAldrinPort0SupModes[];
extern MV_HWS_PORT_INIT_PARAMS hwsAldrinPort32SupModes[];
extern MV_HWS_PORT_INIT_PARAMS *hwsPortsAldrinParamsSupModesMap[];

extern int hwsBobKConvertSbusAddrToSerdes
(
    unsigned char devNum,
    GT_U32 *serdesNum,
    unsigned int  sbusAddr
);

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL aldrinDbInitDone = GT_FALSE;

/************************* pre-declaration ***********************************************/
extern GT_STATUS hwsPulse1msRegCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  coreClk
);

/*****************************************************************************************/
/**
* @internal pcsIfInit function
* @endinternal
*
* @brief   Init all supported PCS types relevant for devices.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS pcsIfInit(GT_U8    devNum)
{
    MV_HWS_PCS_FUNC_PTRS *hwsPcsFuncsPtr;

    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsXPcsIfInit(hwsPcsFuncsPtr));      /* XPCS */
    CHECK_STATUS(mvHwsGPcs28nmIfInit(hwsPcsFuncsPtr));  /* GPCS */
    CHECK_STATUS(mvHwsMMPcs28nmIfInit(hwsPcsFuncsPtr)); /* MMPCS */

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

    CHECK_STATUS(hwsGeMac28nmIfInit(devNum, hwsMacFuncsPtr)); /* GE MAC */
    CHECK_STATUS(hwsXgMac28nmIfInit(hwsMacFuncsPtr)); /* XG MAC */
    CHECK_STATUS(hwsXlgMac28nmIfInit(devNum, hwsMacFuncsPtr));/* XLG MAC */
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
    /* ILKN/SG configuration is not done in Internal CPU */
    CHECK_STATUS(hwsGeMacSgIfInit(devNum, hwsMacFuncsPtr));   /* GE_SG MAC */
#endif
    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD

/**
* @internal mvHwsAldrinClkSelCfg function
* @endinternal
*
* @brief   Configures the DP/Core Clock Selector on port according to core clock,
*         port mode and port number.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAldrinClkSelCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
#ifndef MV_HWS_FREE_RTOS
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    GT_U32  coreClk = 0, coreClkHw, data;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    /*
        -   for port 32 in mode 2.5G SGMII
                for 480Mhz - DP Clock not require               + p_sync_bypass needs to be ACTIVE (8b TX I/F)
                for 365Mhz - DP Clock not require               + p_sync_bypass needs to be ACTIVE (8b TX I/F)
                for 250Mhz - 2.5G not supported in this core clock.
                for 200Mhz - 2.5G not supported in this core clock.

        -   for ports 0-31 in mode 2.5G SGMII
                for 480Mhz - DP Clock not require               + p_sync_bypass needs to be NOT_ACTIVE (64b TX I/F)
                for 365Mhz - DP Clock not require               + p_sync_bypass needs to be NOT_ACTIVE (64b TX I/F)
                for 250Mhz - DP clock require                   + p_sync_bypass needs to be NOT_ACTIVE (64b TX I/F)
                for 200Mhz - DP clock require                   + p_sync_bypass needs to be NOT_ACTIVE (64b TX I/F)
    */

    CHECK_STATUS(hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams));

    /* read core clock */
    CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc(devNum,&coreClk,&coreClkHw));

    data = ((portMode == SGMII2_5) && (coreClk < 333)) ? (1 << 5) : 0;

    /* Set DP Clock Selector */
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
                                    GIG_PORT_MAC_CONTROL_REGISTER4, data, (1 << 5)));

    if(portMode == SGMII2_5)
    {
        data = (phyPortNum == 32) ? 0 : (1 << 6);

        /* Set p_sync_bypass field */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
                                        GIG_PORT_MAC_CONTROL_REGISTER4, data, (1 << 6)));
    }

    if ((portMode == SGMII2_5) || (portMode == SGMII) || (portMode == _1000Base_X))
    {
        CHECK_STATUS(hwsPulse1msRegCfg(devNum, portGroup, curPortParams.portMacNumber, portMode, coreClk));
    }
#endif
    return GT_OK;
}

#endif /* #ifndef MV_HWS_REDUCED_BUILD */

/**
* @internal hwsAldrinSerdesIfInit function
* @endinternal
*
* @brief   Init all supported Serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrinSerdesIfInit(GT_U8 devNum)
{
  MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;

  CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

  CHECK_STATUS(mvHwsAvagoIfInit(devNum, hwsSerdesFuncsPtr));

  return GT_OK;
}

/**
* @internal hwsAldrinIfPreInit function
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
GT_STATUS hwsAldrinIfPreInit
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
        (funcPtr->osMemCopyPtr == NULL) ||
        (funcPtr->serdesRegSetAccess == NULL) ||(funcPtr->serdesRegGetAccess == NULL) ||
        (funcPtr->serverRegSetAccess == NULL) || (funcPtr->serverRegGetAccess == NULL) ||
        (funcPtr->registerSetAccess == NULL) || (funcPtr->registerGetAccess == NULL)
#ifndef MV_HWS_FREE_RTOS
         || (funcPtr->timerGet == NULL)
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

    hwsDeviceSpecInfo[devNum].devType = Aldrin;

    return GT_OK;
}

/**
* @internal hwsAldrinIfInit function
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
GT_STATUS hwsAldrinIfInit
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

#ifndef MV_HWS_REDUCED_BUILD
    GT_STATUS res;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecoveryInfo = systemRecoveryInfo;
#endif

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }

#ifdef MV_HWS_REDUCED_BUILD
    if(aldrinDbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsAldrinIfPreInit(devNum, funcPtr));
    }
#endif

    hwsDeviceSpecInfo[devNum].devType = Aldrin;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].portsNum = 33;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = AVAGO;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber = ALDRIN_MAX_AVAGO_SERDES_NUMBER;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesToAvagoMapPtr = &aldrinSerdesToAvagoMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber = ALDRIN_SERDES_MAP_ARR_SIZE;
    hwsDeviceSpecInfo[devNum].serdesInfo.avagoToSerdesMapPtr = &aldrinAvagoToSerdesMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride = GT_TRUE;
    hwsDeviceSpecInfo[devNum].serdesInfo.sbusAddrToSerdesFuncPtr = hwsBobKConvertSbusAddrToSerdes;
    for (i = 0; i < MV_HWS_MAX_CTLE_BIT_MAPS_NUM; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasOverride[i] = 0;
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[i] = 0;
    }
    hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum = 0;
    hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasMacNum = 0;

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
        CHECK_STATUS(mvHwsServiceCpuFwInit(devNum, 0, "AP_Aldrin"));
    }
    else
    {
        /* AP FW is NOT loaded to Service CPU, use Legacy mode - API execution at the Host */
        hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
    }
    hwsDeviceSpecInfo[devNum].portsParamsSupModesMap                  = hwsPortsAldrinParamsSupModesMap;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc       = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc  = NULL;
	/* Configures the DP/Core Clock Selector on port */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc        = mvHwsAldrinClkSelCfg;
#else
    /* define IPC connection Type Service CPU side */
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
#endif /* MV_HWS_REDUCED_BUILD */



    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);

    hwsDeviceSpecInfo[devNum].gopRev = GOP_28NM_REV3;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
    /* Configures port init / reset functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc = mvHwsGeneralPortInit;
#endif
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = mvHwsGeneralApPortInit;  /* for AP port */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsGeneralPortReset;

    /* Configures  device handler functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = mvHwsPortAvagoAutoTuneSetExt;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = NULL;                         /* TBD - Need to be implemented */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = NULL;                         /* TBD - Need to be implemented */
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
        CHECK_STATUS(hwsAldrinPortsElementsCfg(devNum, portGroup));
#endif
    }

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    /* Init port manager db */
#ifndef MV_HWS_FREE_RTOS
    CHECK_STATUS_EXT(mvHwsPortManagerInit(devNum), LOG_ARG_STRING_MAC("port manager init failed"));
#endif /* MV_HWS_FREE_RTOS */

#ifdef MV_HWS_FREE_RTOS
    CHECK_STATUS_EXT(mvHwsPortEnhanceTuneLitePhaseDeviceInit(devNum),
                     LOG_ARG_STRING_MAC("EnhanceTuneLite device init failed"));
#endif

#ifndef MV_HWS_REDUCED_BUILD
    /* Init SERDES unit relevant for current device */
    if (systemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
    {
        /* save current recovery state */
        tempSystemRecoveryInfo = systemRecoveryInfo;
        systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
        systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
    }
    res = hwsAldrinSerdesIfInit(devNum);
    systemRecoveryInfo = tempSystemRecoveryInfo;
    if (res != GT_OK)
    {
        return res;
    }

#else
    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(hwsAldrinSerdesIfInit(devNum));
#endif

    /* Update HWS After Firmware load and init */
    if (mvHwsServiceCpuEnableGet(devNum))
        CHECK_STATUS(mvHwsServiceCpuFwPostInit(devNum));

    /* Init configuration sequence executer */
    mvCfgSeqExecInit();

    mvUnitInfoSet(devType, GEMAC_UNIT,  ALDRIN_GIG_BASE,    IND_OFFSET);
    mvUnitInfoSet(devType, MMPCS_UNIT,  ALDRIN_MPCS_BASE,   IND_OFFSET);
    mvUnitInfoSet(devType, XPCS_UNIT,   ALDRIN_XPCS_BASE,   IND_OFFSET);
    mvUnitInfoSet(devType, XLGMAC_UNIT, ALDRIN_XLG_BASE,    IND_OFFSET);
    mvUnitInfoSet(devType, SERDES_UNIT, ALDRIN_SERDES_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, PTP_UNIT,    ALDRIN_PTP_BASE,    IND_OFFSET);

    aldrinDbInitDone = GT_TRUE;
    hwsInitDone[devNum] = GT_TRUE;

    return GT_OK;
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
* @internal hwsAldrinIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsAldrinIfClose(GT_U8 devNum)
{
    GT_U32  i;

    if (hwsInitDone[devNum])
    {
        hwsInitDone[devNum] = GT_FALSE;
    }

    aldrinDbInitDone = GT_FALSE;

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


