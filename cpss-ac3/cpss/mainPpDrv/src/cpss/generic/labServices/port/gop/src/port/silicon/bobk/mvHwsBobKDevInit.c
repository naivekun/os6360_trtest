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
* mvHwsBobcatIfDevInit.c
*
* DESCRIPTION:
*       Bobcat specific HW Services init
*
* FILE REVISION NUMBER:
*       $Revision: 20 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <silicon/bobk/mvHwsBobKPortIf.h>
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
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <mvDdr3TrainingIpPrvIf.h>
#include <mvHwsDdr3BobK.h>
#endif

/************************* definition *****************************************************/

#define BOBK_GIG_BASE        (0x10000000)
#define BOBK_XPCS_BASE       (0x10180400)
#define BOBK_XLG_BASE        (0x100C0000)
#define BOBK_MPCS_BASE       (0x10180000)

#define BOBK_SERDES_BASE     (0x13000000)

#define BOBK_PTP_BASE        (0x10180800)

#define IND_OFFSET (0x1000)

#define  BOBK_SERDES_MAP_ARR_SIZE       27

#define NEW_PORT_INIT_SEQ

static GT_U32 bobkSerdesToAvagoMap[BOBK_MAX_AVAGO_SERDES_NUMBER] =
{
  /*  [0] */ 15,
  /*  [1] */ 16,
  /*  [2] */ 17,
  /*  [3] */ 18,
  /*  [4] */ 19,
  /*  [5] */ 20,
  /*  [6] */ 21,
  /*  [7] */ 22,
  /*  [8] */ 23,
  /*  [9] */ 24,
  /*  [10]*/ 25,
  /*  [11]*/ 26,
  /*  [12]*/  0,
  /*  [13]*/  0,
  /*  [14]*/  0,
  /*  [15]*/  0,
  /*  [16]*/  0,
  /*  [17]*/  0,
  /*  [18]*/  0,
  /*  [19]*/  0,
  /*  [20]*/ 14, /* Logical SERDES 20  == Physical SERDES 14 on SERDES chain */
  /*  [21]*/  0,
  /*  [22]*/  0,
  /*  [23]*/  0,
  /*  [24]*/ 13, /* Logical SERDES 24  == Physical SERDES 13 on SERDES chain */
  /*  [25]*/ 12, /* Logical SERDES 25  == Physical SERDES 12 on SERDES chain */
  /*  [26]*/ 11, /* Logical SERDES 26  == Physical SERDES 11 on SERDES chain */
  /*  [27]*/ 10, /* Logical SERDES 27  == Physical SERDES 10 on SERDES chain */
  /*  [28]*/ 5,  /* Logical SERDES 28  == Physical SERDES 05 on SERDES chain */
  /*  [29]*/ 6,  /* Logical SERDES 29  == Physical SERDES 06 on SERDES chain */
  /*  [30]*/ 7,  /* Logical SERDES 30  == Physical SERDES 07 on SERDES chain */
  /*  [31]*/ 8,  /* Logical SERDES 31  == Physical SERDES 08 on SERDES chain */
  /*  [32]*/ 4,  /* Logical SERDES 32  == Physical SERDES 04 on SERDES chain */
  /*  [33]*/ 3,  /* Logical SERDES 33  == Physical SERDES 03 on SERDES chain */
  /*  [34]*/ 2,  /* Logical SERDES 34  == Physical SERDES 02 on SERDES chain */
  /*  [35]*/ 1   /* Logical SERDES 35  == Physical SERDES 01 on SERDES chain */

    /* Temp Sensor = Physical SPICO 09 on SERDES chain */
};

static GT_U32 bobkAvagoToSerdesMap[BOBK_SERDES_MAP_ARR_SIZE] =
{
/*    [0] */  0,
/*    [1] */ 35,
/*    [2] */ 34,
/*    [3] */ 33,
/*    [4] */ 32,
/*    [5] */ 28,
/*    [6] */ 29,
/*    [7] */ 30,
/*    [8] */ 31,
/*    [9] */  0,
/*    [10]*/ 27,
/*    [11]*/ 26,
/*    [12]*/ 25,
/*    [13]*/ 24,
/*    [14]*/ 20,
/*    [15]*/ 0,
/*    [16]*/ 1,
/*    [17]*/ 2,
/*    [18]*/ 3,
/*    [19]*/ 4,
/*    [20]*/ 5,
/*    [21]*/ 6,
/*    [22]*/ 7,
/*    [23]*/ 8,
/*    [24]*/ 9,
/*    [25]*/ 10,
/*    [26]*/ 11

};

/************************* Globals *******************************************************/

extern MV_HWS_PORT_INIT_PARAMS *hwsPortsBobKParams[];

extern MV_HWS_PORT_INIT_PARAMS hwsBobKPort0SupModes[];
extern MV_HWS_PORT_INIT_PARAMS hwsBobKPort1SupModes[];
extern MV_HWS_PORT_INIT_PARAMS hwsBobKPort56SupModes[];
extern MV_HWS_PORT_INIT_PARAMS hwsBobKPort62SupModes[];
extern MV_HWS_PORT_INIT_PARAMS *hwsPortsBobKParamsSupModesMap[];

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL bobKDbInitDone = GT_FALSE;

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
* @internal hwsBobKConvertSbusAddrToSerdes function
* @endinternal
*
* @brief   Convert sbus address to serdes number.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int hwsBobKConvertSbusAddrToSerdes
(
    unsigned char devNum,
    GT_U32 *serdesNum,
    unsigned int  sbusAddr
)
{
    if(sbusAddr > hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber)
    {
        return GT_BAD_PARAM;
    }

    *serdesNum = hwsDeviceSpecInfo[devNum].serdesInfo.avagoToSerdesMapPtr[sbusAddr];

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
#if !defined(MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT) /* no reduced build required for uInit */
    /* ILKN/SG configuration is not done in Internal CPU */
    CHECK_STATUS(hwsGeMacSgIfInit(devNum, hwsMacFuncsPtr));   /* GE_SG MAC */
#endif
    return GT_OK;
}


#ifndef MV_HWS_REDUCED_BUILD

/**
* @internal mvHwsBobKClkSelCfg function
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
static GT_STATUS mvHwsBobKClkSelCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
#ifndef MV_HWS_FREE_RTOS
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    GT_U32  coreClk = 0,coreClkHw, data;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams);

    /* read core clock */
    CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc(devNum,&coreClk,&coreClkHw));
    data = ((portMode == SGMII2_5) && (coreClk < 312)) ? (1 << 5) : 0;

    /* Set DP Clock Selector */
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
                                    GIG_PORT_MAC_CONTROL_REGISTER4, data, (1 << 5)));

    if ((portMode == SGMII2_5) || (portMode == SGMII) || (portMode == _1000Base_X))
    {
        CHECK_STATUS(hwsPulse1msRegCfg(devNum, portGroup, curPortParams.portMacNumber, portMode, coreClk));
    }
#endif

    return GT_OK;
}
#endif
/**
* @internal hwsBobKSerdesIfInit function
* @endinternal
*
* @brief   Init all supported Serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobKSerdesIfInit(GT_U8 devNum)
{
  MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;

  CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

  CHECK_STATUS(mvHwsAvagoIfInit(devNum, hwsSerdesFuncsPtr));

  return GT_OK;
}

/*******************************************************************************
* TrainingIpIfInit
*
* DESCRIPTION:
*       Init training IP functionality.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#ifndef MV_HWS_REDUCED_BUILD
static GT_STATUS TrainingIpIfInit(GT_U8 devNum)
{
    MV_HWS_TRAINING_IP_FUNC_PTRS *funcPtrArray;

    CHECK_STATUS(ddr3TipInitBobK(devNum, 0 /* board ID*/));
    if(bobKDbInitDone == GT_FALSE)
    {
        mvHwsTrainingIpGetFuncPtr(&funcPtrArray);
        mvHwsDdr3TipIfInit(funcPtrArray);
    }

    return GT_OK;
}
#endif

/**
* @internal hwsBobKIfPreInit function
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
GT_STATUS hwsBobKIfPreInit
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
    hwsRegisterSetFuncPtr = funcPtr->registerSetAccess;
    hwsRegisterGetFuncPtr = funcPtr->registerGetAccess;
    hwsOsMemCopyFuncPtr = funcPtr->osMemCopyPtr;
    hwsServerRegFieldSetFuncPtr = funcPtr->serverRegFieldSetAccess;
    hwsServerRegFieldGetFuncPtr = funcPtr->serverRegFieldGetAccess;
#ifndef MV_HWS_FREE_RTOS
    hwsTimerGetFuncPtr = funcPtr->timerGet;
#endif

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsDeviceSpecInfo[devNum].devType = BobK;

    return GT_OK;
}

/**
* @internal hwsBobKIfInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobKIfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr)
{
    GT_U32  portGroup = devNum;
    GT_U32  data, version;
    MV_HWS_DEV_TYPE devType;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
#ifndef MV_HWS_REDUCED_BUILD
    GT_STATUS res;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecoveryInfo = systemRecoveryInfo;
#endif
    GT_U32 i;

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }

#ifdef MV_HWS_REDUCED_BUILD
    if(bobKDbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsBobKIfPreInit(devNum, funcPtr));
    }
#endif

    hwsDeviceSpecInfo[devNum].devType = BobK;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].portsNum = 72;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = NON_SUP_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = AVAGO;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber = BOBK_MAX_AVAGO_SERDES_NUMBER;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesToAvagoMapPtr = &bobkSerdesToAvagoMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber = BOBK_SERDES_MAP_ARR_SIZE;
    hwsDeviceSpecInfo[devNum].serdesInfo.avagoToSerdesMapPtr = &bobkAvagoToSerdesMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.sbusAddrToSerdesFuncPtr = hwsBobKConvertSbusAddrToSerdes;

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
        res = mvHwsServiceCpuFwInit(devNum, 0xffffffff, "AP_BobK_MSYS");
        if (res == GT_NO_SUCH)
        {
            /* MSYS not available, fallback to CM3 */
            res = mvHwsServiceCpuFwInit(devNum, 0, "AP_BobK_CM3");
        }
        if (res != GT_OK)
        {
            return res;
        }
    }
    else
    {
        /* AP FW is NOT loaded to Service CPU, use Legacy mode - API execution at the Host */
        hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
    }
    hwsDeviceSpecInfo[devNum].portsParamsSupModesMap                 = hwsPortsBobKParamsSupModesMap;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc      = hwsBobKPortParamsSet;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc = hwsBobKPortParamsGetLanes;

    /* Configures the DP/Core Clock Selector on port */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc = mvHwsBobKClkSelCfg;

#else
    /* define IPC connection Type Service CPU side */
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
#endif /* MV_HWS_REDUCED_BUILD */

    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);

    /* Serdes VOS override should be true by default on revision 1 and above */
    if (version > 0)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride = GT_TRUE;
    }
    else
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride = GT_FALSE;
    }

    for (i = 0; i < MV_HWS_MAX_CTLE_BIT_MAPS_NUM; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasOverride[i] = 0;
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[i] = 0;
    }
    hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum = 0;
    hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum = 0;

    hwsDeviceSpecInfo[devNum].gopRev = GOP_28NM_REV3;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

    /* functions not relevant for BobK CM3 AP FW (non micro-init)
       due to space limit */
#if !defined(MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
    /* Configures port init / reset functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc = mvHwsGeneralPortInit;
#endif /* !defined(MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT) */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = mvHwsGeneralApPortInit;  /* for AP port */
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
        CHECK_STATUS(hwsBobKPortsElementsCfg(devNum, portGroup));
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

#ifndef MV_HWS_REDUCED_BUILD
    /* Init SERDES unit relevant for current device */
    if (systemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
    {
        /* save current recovery state */
        tempSystemRecoveryInfo = systemRecoveryInfo;
        systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
        systemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
    }
    res = hwsBobKSerdesIfInit(devNum);
    systemRecoveryInfo = tempSystemRecoveryInfo;
    if (res != GT_OK)
    {
        return res;
    }
#else
    CHECK_STATUS(hwsBobKSerdesIfInit(devNum));
#endif

    /* Update HWS After Firmware load and init */
    if (mvHwsServiceCpuEnableGet(devNum))
    {
        CHECK_STATUS(mvHwsServiceCpuFwPostInit(devNum));
    }

    /* Init configuration sequence executer */
    mvCfgSeqExecInit();

    mvUnitInfoSet(devType, GEMAC_UNIT,  BOBK_GIG_BASE,    IND_OFFSET);
    mvUnitInfoSet(devType, MMPCS_UNIT,  BOBK_MPCS_BASE,   IND_OFFSET);
    mvUnitInfoSet(devType, XPCS_UNIT,   BOBK_XPCS_BASE,   IND_OFFSET);
    mvUnitInfoSet(devType, XLGMAC_UNIT, BOBK_XLG_BASE,    IND_OFFSET);
    mvUnitInfoSet(devType, SERDES_UNIT, BOBK_SERDES_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, PTP_UNIT,    BOBK_PTP_BASE,    IND_OFFSET);

    /* Only devices with TM supported should perform DDR training */
#ifndef MV_HWS_REDUCED_BUILD
    switch (PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_98DX4235_CNS:
        case CPSS_98DX42KK_CNS:
        case CPSS_98DX4203_CNS:
        case CPSS_98DX4204_CNS:
            /* DDR training initialization */
            res = TrainingIpIfInit(devNum);
            if ((res != GT_OK) && (res != GT_NOT_INITIALIZED))
            {
                return res;
            }
            break;

        default:
            break;
    }
#endif

    bobKDbInitDone = GT_TRUE;
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
* @internal hwsBobKIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsBobKIfClose(GT_U8 devNum)
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

    bobKDbInitDone = GT_FALSE;

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



