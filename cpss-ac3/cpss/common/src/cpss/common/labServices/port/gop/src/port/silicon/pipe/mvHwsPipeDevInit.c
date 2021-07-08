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
* @file mvHwsPipeDevInit.c
*
* @brief Pipe specific HW Services init
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
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>

#include <cpss/common/labServices/port/gop/port/silicon/pipe/mvHwsPipePortIf.h>

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


/************************* definition *****************************************************/

#define PIPE_MPCS_BASE           (0x10180000)
#define PIPE_XPCS_BASE           (0x10180400)
#define PIPE_CGPCS_BASE          (0x10300000)
#define PIPE_GIG_BASE            (0x10000000)
#define PIPE_XLG_BASE            (0x100C0000)
#define PIPE_SERDES_BASE         (0x13000000)
#define PIPE_CG_BASE             (0x10340000)
#define PIPE_CG_RS_FEC_BASE      (0x10340800)
#define PIPE_PTP_BASE            (0x10180800)

#define IND_OFFSET                  (0x1000)
#define CG_IND_OFFSET               (0x1000)

#define PIPE_SERDES_MAP_ARR_SIZE         20


/*Access to the SERDES is done through the SBC Controller.
SBC Controller accesses the corresponding SERDES through the SBUS chain based on the address. Note that the Avago Temperature Sensor (TSEN) and Avago PMROs are also part of the chain.
PIPE has only one SBC comtroller. Chain order:
SBC-->SD#0-->…….-->SD#15-->PMRO0(near 25G)-->TSEN-->PMRO1(near 10G)->SBC
*/
static GT_U32 pipeSerdesToAvagoMap[PIPE_MAX_AVAGO_SERDES_NUMBER] =
{
/* laneNum   SBC_Serdes */
 /* [0]  */      1,
 /* [1]  */      2,
 /* [2]  */      3,
 /* [3]  */      4,
 /* [4]  */      5,
 /* [5]  */      6,
 /* [6]  */      7,
 /* [7]  */      8,
 /* [8]  */      9,
 /* [9]  */      10,
 /* [10] */      11,
 /* [11] */      12,
 /* [12] */      13,
 /* [13] */      14,
 /* [14] */      15,
 /* [15] */      16
};

GT_U32 pipeAvagoToSerdesMap[PIPE_SERDES_MAP_ARR_SIZE] =
{
/* SBC_Serdes  laneNum */
/* [ 0]  */       0, /*NOT USED*/
/* [ 1]  */       0,
/* [ 2]  */       1,
/* [ 3]  */       2,
/* [ 4]  */       3,
/* [ 5]  */       4,
/* [ 6]  */       5,
/* [ 7]  */       6,
/* [ 8]  */       7,
/* [ 9]  */       8,
/* [10]  */       9,
/* [11] */       10,
/* [12] */       11,
/* [13] */       12,
/* [14] */       13,
/* [15] */       14,
/* [16] */       15,
/* PMRO0=17 */    0,
/* TSEN=18 */     0,
/* PMRO1=19 */    0    /* end of SBC_Unit*/
};

/************************* Globals *******************************************************/

extern MV_HWS_PORT_INIT_PARAMS *hwsPortsPipeParams[];

extern MV_HWS_PORT_INIT_PARAMS hwsPipePort0SupModes[];
extern MV_HWS_PORT_INIT_PARAMS hwsPipePort12SupModes[];
extern MV_HWS_PORT_INIT_PARAMS *hwsPortsPipeParamsSupModesMap[];

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL pipeDbInitDone = GT_FALSE;

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
* @internal hwsPipeConvertSbusAddrToSerdes function
* @endinternal
*
* @brief   Convert sbus address to serdes number.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int hwsPipeConvertSbusAddrToSerdes
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
static GT_STATUS pcsIfInit(GT_U8 devNum)
{
    MV_HWS_PCS_FUNC_PTRS *hwsPcsFuncsPtr;

    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsXPcsIfInit(hwsPcsFuncsPtr));      /* XPCS */
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
#if !defined (MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
    /* ILKN/SG configuration is not done in Internal CPU */
    CHECK_STATUS(hwsGeMacSgIfInit(devNum, hwsMacFuncsPtr));   /* GE_SG MAC */
#endif
    return GT_OK;
}

/**
* @internal hwsPipeSerdesIfInit function
* @endinternal
*
* @brief   Init all supported Serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPipeSerdesIfInit(GT_U8 devNum)
{
  MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;

  CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

  CHECK_STATUS(mvHwsAvagoIfInit(devNum, hwsSerdesFuncsPtr));

  return GT_OK;
}

/**
* @internal hwsPipeIfPreInit function
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
GT_STATUS hwsPipeIfPreInit
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
    hwsRegisterSetFuncPtr = funcPtr->registerSetAccess;
    hwsRegisterGetFuncPtr = funcPtr->registerGetAccess;
    hwsOsMemCopyFuncPtr = funcPtr->osMemCopyPtr;
    hwsOsStrCatFuncPtr = funcPtr->osStrCatPtr;
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

    hwsDeviceSpecInfo[devNum].devType = Pipe;

    return GT_OK;
}

/**
* @internal hwsPipeIfInit function
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
GT_STATUS hwsPipeIfInit
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
    if(pipeDbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsPipeIfPreInit(devNum, funcPtr));
    }
#endif

    hwsDeviceSpecInfo[devNum].devType = Pipe;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].portsNum = HWS_PIPE_PORTS_NUM_CNS;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = AVAGO;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber = MAX_AVAGO_SERDES_NUMBER;
    hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber = PIPE_SERDES_MAP_ARR_SIZE;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesToAvagoMapPtr = &pipeSerdesToAvagoMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.avagoToSerdesMapPtr = &pipeAvagoToSerdesMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride = GT_TRUE;
    hwsDeviceSpecInfo[devNum].serdesInfo.sbusAddrToSerdesFuncPtr = hwsPipeConvertSbusAddrToSerdes;
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
        CHECK_STATUS(mvHwsServiceCpuFwInit(devNum, 0, "AP_PIPE"));
    }
    else
    {
        /* AP FW is NOT loaded to Service CPU, use Legacy mode - API execution at the Host */
        hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
    }
    hwsDeviceSpecInfo[devNum].portsParamsSupModesMap = hwsPortsPipeParamsSupModesMap;
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
        CHECK_STATUS(hwsPipePortsElementsCfg(devNum, portGroup));
#endif
    }

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(hwsPipeSerdesIfInit(devNum));

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

    mvUnitInfoSet(devType,  SERDES_UNIT,     PIPE_SERDES_BASE,     IND_OFFSET);
    mvUnitInfoSet(devType,  MMPCS_UNIT,      PIPE_MPCS_BASE,       IND_OFFSET);
    mvUnitInfoSet(devType,  XPCS_UNIT,       PIPE_XPCS_BASE,       IND_OFFSET);
    mvUnitInfoSet(devType,  CGPCS_UNIT,      PIPE_CGPCS_BASE,      CG_IND_OFFSET);
    mvUnitInfoSet(devType,  GEMAC_UNIT,      PIPE_GIG_BASE,        IND_OFFSET);
    mvUnitInfoSet(devType,  XLGMAC_UNIT,     PIPE_XLG_BASE,        IND_OFFSET);
    mvUnitInfoSet(devType,  CG_UNIT,         PIPE_CG_BASE,         CG_IND_OFFSET);
    mvUnitInfoSet(devType,  CG_RS_FEC_UNIT,  PIPE_CG_RS_FEC_BASE,  CG_IND_OFFSET);
    mvUnitInfoSet(devType,  PTP_UNIT,        PIPE_PTP_BASE,        IND_OFFSET);

    pipeDbInitDone = GT_TRUE;
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
* @internal hwsPipeIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsPipeIfClose(GT_U8 devNum)
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

    pipeDbInitDone = GT_FALSE;

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


