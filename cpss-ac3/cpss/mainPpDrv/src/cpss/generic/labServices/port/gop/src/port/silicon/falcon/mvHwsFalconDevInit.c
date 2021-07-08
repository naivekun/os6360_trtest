/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvHwsFalconDevInit.c
*
* DESCRIPTION:
*     Falcon specific HW Services init
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>

#include <silicon/falcon/mvHwsFalconPortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>

#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mti400Mac/mvHwsMti400MacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/d2dMac/mvHwsD2dMacIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs400If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs200If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs100If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs50If.h>
#include <cpss/common/labServices/port/gop/port/pcs/d2dPcs/mvHwsD2dPcsIf.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/gw16/mvGw16If.h>

#include <cpssCommon/private/prvCpssEmulatorMode.h>
/*#include <cpss/common/h/cpss/generic/cpssHwInfo.h>*/
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>

extern int hwsBobKConvertSbusAddrToSerdes
(
    unsigned char devNum,
    GT_U32 *serdesNum,
    unsigned int  sbusAddr
);

MV_OS_GET_DEVICE_DRIVER_FUNC        hwsGetDeviceDriverFuncPtr = NULL;

HWS_UNIT_BASE_ADDR_CALC_BIND    falconBaseAddrCalcFunc = NULL;

/************************* definition *****************************************************/

#define RAVEN_NOT_INITIALIZED_BASE_ADDR     (0xFFFFFFFF)

#define RAVEN_MTI_PCS50_BASE                (0x00465000)
#define RAVEN_MTI_PCS100_BASE               (0x00464000)
#define RAVEN_MTI_PCS200_BASE               (0x00461000)
#define RAVEN_MTI_PCS400_BASE               (0x00460000)
#define RAVEN_MTI_LOW_SP_PCS_BASE           (0x00474000)

#define IND_OFFSET                          (0x1000)

#define RAVEN_SERDES_MAP_ARR_SIZE           21

static GT_U32 falconSerdesToAvagoMap[RAVEN_SERDES_MAP_ARR_SIZE] =
{
/* laneNum   SBC_Addr */
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
 /* [15] */      16,
 /* [16] */      17,
 /* [17] */      18,    /* TSEN */
 /* [18] */      19,    /* PMRO */
 /* [19] */      0xFD   /* SBM */

};

GT_U32 falconAvagoToSerdesMap[RAVEN_SERDES_MAP_ARR_SIZE] =
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
/* [11] */        10,
/* [12] */        11,
/* [13] */        12,
/* [14] */        13,
/* [15] */        14,
/* [16] */        15,
/* [17] */        16,
/* TSEN=18 */     0,
/* PMRO=19 */     0,
/* SBM=0xFD */    19

};

/************************* Globals *******************************************************/

extern MV_HWS_PORT_INIT_PARAMS *hwsPortsFalconParams[];

extern MV_HWS_PORT_INIT_PARAMS hwsFalconPort0SupModes[];
extern MV_HWS_PORT_INIT_PARAMS hwsFalconPort1SupModes[];
extern MV_HWS_PORT_INIT_PARAMS hwsFalconPort4SupModes[];
extern MV_HWS_PORT_INIT_PARAMS *hwsPortsFalconParamsSupModesMap[];

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL falconDbInitDone = GT_FALSE;
GT_U32  hwsFalconNumOfRavens = 0;
/************************* pre-declaration ***********************************************/


/*****************************************************************************************/

/*******************************************************************************
* hwsPcsIfInit
*
* DESCRIPTION:
*       Init all supported PCS types relevant for devices.
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
static GT_STATUS pcsIfInit(GT_U8 devNum)
{
    MV_HWS_PCS_FUNC_PTRS *hwsPcsFuncsPtr;

    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsD2dPcsIfInit(devNum, hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs50IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs100IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs200IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs400IfInit(hwsPcsFuncsPtr));

    return GT_OK;
}

/*******************************************************************************
* hwsMacIfInit
*
* DESCRIPTION:
*       Init all supported MAC types.
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
static GT_STATUS macIfInit(GT_U8 devNum)
{
    MV_HWS_MAC_FUNC_PTRS *hwsMacFuncsPtr;

    CHECK_STATUS(hwsMacGetFuncPtr(devNum, &hwsMacFuncsPtr));

    CHECK_STATUS(hwsD2dMacIfInit(devNum, hwsMacFuncsPtr));
    CHECK_STATUS(mvHwsMti100MacIfInit(hwsMacFuncsPtr));
    CHECK_STATUS(mvHwsMti400MacIfInit(hwsMacFuncsPtr));
    return GT_OK;
}

/*******************************************************************************
* hwsFalconSerdesIfInit
*
* DESCRIPTION:
*       Init all supported Serdes types.
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
static GT_STATUS hwsFalconSerdesIfInit(GT_U8 devNum)
{
    MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;

    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

    CHECK_STATUS(mvHwsAvago16nmIfInit(devNum, hwsSerdesFuncsPtr));

    CHECK_STATUS(mvHwsD2dPhyIfInit(devNum, hwsSerdesFuncsPtr));

    return GT_OK;
}

/**
* @internal hwsFalconIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsFalconIfPreInit
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

    hwsDeviceSpecInfo[devNum].devType = Falcon;
    hwsGetDeviceDriverFuncPtr = ((HWS_EXT_FUNC_STC_PTR*)funcPtr->extFunctionStcPtr)->getDeviceDriver;
    hwsPpHwTraceEnablePtr = ((HWS_EXT_FUNC_STC_PTR*)funcPtr->extFunctionStcPtr)->ppHwTraceEnable;
#ifndef ASIC_SIMULATION
    if (hwsGetDeviceDriverFuncPtr == NULL) {
        return GT_BAD_PTR;
    }
#endif

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }

    hwsDeviceSpecInfo[devNum].devType = Falcon;

    return GT_OK;
}

/*******************************************************************************
* hwsFalconIfInit
*
* DESCRIPTION:
*       Init all supported units needed for port initialization.
*       Must be called per device.
*
* INPUTS:
*       funcPtr - pointer to structure that hold the "os"
*                 functions needed be bound to HWS.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS hwsFalconIfInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    GT_U32  data, version;
    MV_HWS_DEV_TYPE devType;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }

#ifdef MV_HWS_REDUCED_BUILD
    if(falconDbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsFalconIfPreInit(devNum, funcPtr));
    }
#endif

    hwsDeviceSpecInfo[devNum].devType = Falcon;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].portsNum = HWS_FALCON_PORTS_NUM_CNS;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = AVAGO_16NM;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber = MAX_AVAGO_SERDES_NUMBER;
    hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber = RAVEN_SERDES_MAP_ARR_SIZE;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesToAvagoMapPtr = &falconSerdesToAvagoMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.avagoToSerdesMapPtr = &falconAvagoToSerdesMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride = GT_FALSE;
    hwsDeviceSpecInfo[devNum].serdesInfo.sbusAddrToSerdesFuncPtr = hwsBobKConvertSbusAddrToSerdes;
    hwsDeviceSpecInfo[devNum].portsParamsSupModesMap = hwsPortsFalconParamsSupModesMap;

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

#if 0
#ifndef MV_HWS_REDUCED_BUILD
    /* define IPC connection Type Host side */
    if (mvHwsServiceCpuEnableGet(devNum))
    {
        /* AP FW is loaded to Service CPU, use IPC mode - API execution at the Service CPU */

        /* !!! Replace HOST2HWS_LEGACY_MODE with HOST2SERVICE_CPU_IPC_CONNECTION once approved by CPSS !!! */
        hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE/*HOST2SERVICE_CPU_IPC_CONNECTION*/;
        /* Init HWS Firmware */
        CHECK_STATUS(mvHwsServiceCpuFwInit(devNum));
    }
    else
    {
        /* AP FW is NOT loaded to Service CPU, use Legacy mode - API execution at the Host */
        hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
    }
#else
    /* define IPC connection Type Service CPU side */
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
#endif /* MV_HWS_REDUCED_BUILD */
#endif /* if 0 */

    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;

    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);

    hwsDeviceSpecInfo[devNum].gopRev = GOP_16NM_REV1;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc = NULL;

    /* Configures port init / reset functions */
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc = mvHwsFalconPortInit;
#endif
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsFalconPortReset ;

    /* Configures  device handler functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portLbSetWaFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc   = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func     = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc      = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc = NULL;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** Falcon ifInit start ******\n");
    }
#endif

    hwsInitPortsModesParam(devNum,NULL);
    devType = devType; /* avoid warning */
    if(hwsInitDone[devNum] == GT_FALSE)
    {
#ifndef MV_HWS_REDUCED_BUILD
        CHECK_STATUS(hwsDevicePortsElementsCfg(devNum));
#endif
    }

    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(hwsFalconSerdesIfInit(devNum));

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));


    /* Update HWS After Firmware load and init */
    if(!cpssDeviceRunCheck_onEmulator())
    {
        if (mvHwsServiceCpuEnableGet(devNum))
            CHECK_STATUS(mvHwsServiceCpuFwPostInit(devNum));
    }
    /* Init configuration sequence executer */
    mvCfgSeqExecInit();
    falconDbInitDone = GT_TRUE;
    hwsInitDone[devNum] = GT_TRUE;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("******** Falcon ifInit end ********\n");
    }
#endif
    return GT_OK;
}


#ifndef MV_HWS_REDUCED_BUILD
/*******************************************************************************
* hwsSerdesIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported serdes types.
*
* INPUTS:
*       GT_U8 devNum
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static void serdesIfClose(GT_U8 devNum)
{
    mvHwsAvagoIfClose(devNum);
    mvHwsGw16IfClose(devNum);
}

/*******************************************************************************
* hwsPcsIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported PCS types.
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
static void pcsIfClose
(
    GT_U8 devNum
)
{
    hwsD2dPcsIfClose(devNum);
}

/*******************************************************************************
* hwsMacIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported MAC types.
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
static void macIfClose
(
    GT_U8 devNum
)
{
    hwsD2dMacIfClose(devNum);
}

/*******************************************************************************
* hwsFalconIfClose
*
* DESCRIPTION:
*       Free all resource allocated for ports initialization.
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
void hwsFalconIfClose(GT_U8 devNum)
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

    falconDbInitDone = GT_FALSE;

    serdesIfClose(devNum);
    pcsIfClose(devNum);
    macIfClose(devNum);

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsPortsElementsClose(devNum);
}

#endif

GT_STATUS hwsFalconRavenD2dCpllConfig
(
    IN GT_U8 devNum,
    IN GT_U8 ravenIdx,
    IN CPSS_HW_DRIVER_STC *driverPtr
)
{
   /* GT_U32  serdesNum; */ /* serdes iterator */
   /* GT_U32  data, bit;*/
    /******************************************************************************
     * CPLL initialization:
     *  CPLL_1 - used for D2D GW SERDESes -> configuration 156.25Mhz -> 156.25Mhz
     ******************************************************************************/
    /*hwsServerRegFieldSetFuncPtr(devNum, 0xF8250, 16, 1, 1));*/
   /* data = 0x10000;
    driverPtr->writeMask(driverPtr, 0, 0xF8250, &data, 1, 0x10000);*/
    /* D2D cpllNum == CPLL1 */
    /*bit = 25;*/
    /*CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, addr, &data));*/
   /* driverPtr->read(driverPtr, 0, DEVICE_GENERAL_CONTROL_18, &data, 1);
    data =  (data & ~(1 << bit));*/
    /*CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, addr, data));*/
    /*driverPtr->writeMask(driverPtr, 0, DEVICE_GENERAL_CONTROL_18, &data, 1, 0);*/
    ravenIdx = ravenIdx;
    return mvHwsCpllControl(driverPtr, devNum, CPLL1, GT_FALSE, MV_HWS_156MHz_IN, MV_HWS_156MHz_OUT);
}

GT_STATUS hwsFalconRavenGW16SerdesAutoStartInit
(
    IN GT_U8 devNum,
    IN GT_U8 ravenIdx,
    IN CPSS_HW_DRIVER_STC *driverPtr
)
{
    GT_U8       d2dIdx, d2dNum;
    /***********************************************************/
    /*     D2D auto start (autonegotiation) initialization     */
    /***********************************************************/
    /* In order to perform D2D/PHY configurations it is needed to setup links
       on all 6 devices and only then start to run basic init sequence */

    if (driverPtr != NULL) {
        /* for raven smi we write all raven in the same address */
        ravenIdx = 0;
    }

    for (d2dIdx = 0; d2dIdx < HWS_D2D_NUMBER_PER_RAVEN_CNS; d2dIdx++) {
        d2dNum = ravenIdx * HWS_D2D_STEP_PER_RAVEN_CNS + d2dIdx*2 + 1;

        if(!cpssDeviceRunCheck_onEmulator())
            CHECK_STATUS(mvHwsGW16SerdesAutoStartInit(driverPtr, devNum, d2dNum));

        CHECK_STATUS(mvHwsD2dPcsInit(driverPtr, devNum, d2dNum));
    }
    return GT_OK;
}



    /*assumption PEX device and SMI master decise were crated before */
    /* PEX - during init phase 1; SMI - before calling the function (cpssDxChPhyPortSmiInit)*/
GT_STATUS hwsFalconRavenD2dInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS           rc;
    GT_U32              ravenSmiID;
    GT_U32              ravenSmiBus;
    GT_U8               ravenIdx;
    /* init dual tile */
    CPSS_HW_DRIVER_STC  *ssmiPtr;
    GT_U32              data, counter;

    for (ravenIdx = 0; ravenIdx < hwsFalconNumOfRavens; ravenIdx++)
    {
        /* get ravenSmiID from ravenIdx - probably fix array*/
        ravenSmiID  = ravenIdx % HWS_FALCON_3_2_RAVEN_NUM_CNS;
        ravenSmiBus = ravenIdx / HWS_FALCON_3_2_RAVEN_NUM_CNS;
        rc = hwsGetDeviceDriverFuncPtr(devNum, 0, ravenSmiBus /*smi 0/1/2/3*/, ravenSmiID, &ssmiPtr);
        if ((rc != GT_OK) || (ssmiPtr == NULL))
        {
            hwsOsPrintf("hwsGetDeviceDriverFuncPtr returned with NULL pointer for ravenIdx = %d\n", ravenIdx);
            continue;
        }

         /*********************************/
        /*     Device initialization     */
        /*********************************
            Init stage polling
            This polling is on the init_stage field (bits 5:4) of the Server
            Status register. Polling is continued until this field is equal
            2'b11 (Fully Functional), or until the timeout expires (100ms).
        */
        data = 0;
        counter = 0;
        /*if(!cpssDeviceRunCheck_onEmulator())*/
        {
            while(data != 3)
            {
                ssmiPtr->read(ssmiPtr, SSMI_FALCON_ADDRESS_SPACE, DFX_SERVER_STATUS_REG_ADDR_CNS + RAVEN_DFX_BASE_ADDR, &data, 1);
                data = (data >> 4) & 3;
                if(counter == 100)
                {
                    hwsOsPrintf("ERROR: Device not ready, Slave Smi Addr = 0x%x\n", ravenSmiID);
                    return GT_FAIL;
                }
                counter++;
                if(data != 3)
                {
                    if(cpssDeviceRunCheck_onEmulator())
                    {
                        cpssOsTimerWkAfter(50);
                    }
                    cpssOsTimerWkAfter(1);
#ifdef ASIC_SIMULATION
                    data = 3;
#endif
                }
            }
        }
/*        rc = hwsFalconRavenD2dCpllConfig(devNum, ravenIdx, ssmiPtr);
        if(rc != GT_OK)
        {
            hwsOsPrintf("hwsFalconCpllConfig failed: ravenIdx = %d, rc = %d\n", ravenIdx, rc);
            return rc;
        }
*/
        rc = hwsFalconRavenGW16SerdesAutoStartInit(devNum, ravenIdx, ssmiPtr);
        if(rc != GT_OK)
        {
            hwsOsPrintf("hwsFalconCpllConfig failed: ravenIdx = %d, rc = %d\n", ravenIdx, rc);
            return rc;
        }

        /* destroy the device driver */
        ssmiPtr->destroy(ssmiPtr);
     }
    return GT_OK;
}

GT_STATUS hwsFalconEagleD2dInit
(
    IN GT_U8 devNum
)
{
    GT_U8       d2dIdx, d2dNum;

    /***********************************************************/
    /*     D2D auto start (autonegotiation) initialization     */
    /***********************************************************/
    /* In order to perform D2D/PHY configurations it is needed to setup links
       on all 6 devices and only then start to run basic init sequence
        Wrote register via map d2d_reg_block[0].RXC_RegFile.uvm_reg_map: d2d_reg_block[0].RXC_RegFile.autostart=0xf
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_CFG_ENA=0xd003d
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_8=0x100808
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_9=0x100808*/

    for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
        d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);

        if(!cpssDeviceRunCheck_onEmulator())
            CHECK_STATUS(mvHwsGW16SerdesAutoStartInit(NULL, devNum, d2dNum));

        CHECK_STATUS(mvHwsD2dPcsInit(NULL, devNum, d2dNum));
    }
    return GT_OK;
}

GT_STATUS hwsFalconCheckD2dStatus
(
    IN GT_U8 devNum,
    IN GT_U32 d2dNum

)
{
    GT_STATUS   rc;
    GT_U8       d2dIdx;

    if (d2dNum == HWS_D2D_ALL) {
        for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
            d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);

            /***********************************************************/
            /*     D2D PHY wait for auto start init                    */
            /***********************************************************/
            rc = mvHwsGW16SerdesAutoStartInitStatusGet(devNum, d2dNum);
            if(rc != GT_OK)
            {
                hwsOsPrintf("mvHwsGW16SerdesAutoStartInitStatusGet failed: d2dNum = %d, rc = %d\n", d2dNum, rc);
                return rc;
            }
        }
    }
    else
    {
        CHECK_STATUS(mvHwsGW16SerdesAutoStartInitStatusGet(devNum, d2dNum));

    }
    return GT_OK;
}
GT_STATUS hwsFalconD2dPcsResetEnable
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_BOOL enable
)
{
    GT_U32 d2dIdx;

    if (d2dNum != HWS_D2D_ALL)
    {
        CHECK_STATUS(mvHwsD2dPcsReset(devNum, d2dNum, enable));
    }
    else
    {
        for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
            d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0); /*eagle */
            CHECK_STATUS(mvHwsD2dPcsReset(devNum,d2dNum,enable));
            d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1); /*raven */
            CHECK_STATUS(mvHwsD2dPcsReset(devNum,d2dNum,enable));
        }
    }
    return GT_OK;
}
GT_STATUS hwsFalconD2dChannelEnable
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channel,
    GT_BOOL enable
)
{
    GT_U32 d2dIdx;
    if (d2dNum != HWS_D2D_ALL)
    {
        CHECK_STATUS(hwsD2dMacChannelEnable(devNum, d2dNum, channel, enable));
    }
    else
    {
        for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
            d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0); /*eagle */
            CHECK_STATUS(hwsD2dMacChannelEnable(devNum,d2dNum,channel,enable));
            d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1); /*raven */
            CHECK_STATUS(hwsD2dMacChannelEnable(devNum,d2dNum,channel,enable));
        }
    }
    return GT_OK;
}

/*INITIATE D2D MAC+PCS AND ACHIEVE LINK*/
GT_STATUS hwsFalconStep0
(
    IN GT_U8 devNum,
    IN GT_U32 numOfTiles
)
{
    hwsFalconNumOfRavens = numOfTiles * HWS_FALCON_3_2_RAVEN_NUM_CNS;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** Falcon step 0 start ******\n");
    }
#endif
    /* init raven D2D in order to use pex */
    CPSS_TBD_BOOKMARK_FALCON_EMULATOR
    /*
        Eagle:
        Wrote register via map d2d_reg_block[0].RXC_RegFile.uvm_reg_map: d2d_reg_block[0].RXC_RegFile.autostart=0xf
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_CFG_ENA=0xd003d
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_8=0x100008
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_9=0x100008
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_16=0x101800
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_17=0x101800
    */
    CHECK_STATUS(hwsFalconEagleD2dInit(devNum));
    /*
    Raven, done through SMI:
    Wrote register via map d2d_reg_block[1].RXC_RegFile.uvm_reg_map: d2d_reg_block[1].RXC_RegFile.autostart=0xf
    Wrote register via map d2d_reg_block[1].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_CFG_ENA=0xd003d
    */
    CHECK_STATUS(hwsFalconRavenD2dInit(devNum));
    /*
        Eagle: (pull indication is for Raven & Eagle)
        pull for bit [18:17] = 2'b11
        Read  register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_STATUS
    */
    CHECK_STATUS(hwsFalconCheckD2dStatus(devNum, HWS_D2D_ALL));

    /* D2D_cp is ready - we can use PEX for Raven */
    /* set d2d mac channel in reset */
    CHECK_STATUS(hwsFalconD2dChannelEnable(devNum, HWS_D2D_ALL, HWS_D2D_MAC_CHANNEL_ALL, GT_FALSE));

    /*set d2d pcs out of reset */
    CHECK_STATUS(hwsFalconD2dPcsResetEnable(devNum, HWS_D2D_ALL, GT_TRUE));
    /* enable fcu in msdb */
    CHECK_STATUS(mvHwsMsdbFcuEnable(devNum, HWS_D2D_ALL, GT_TRUE));
    /* enable tsu in mpf */
    CHECK_STATUS(mvHwsMpfTsuEnable(devNum, HWS_D2D_ALL, NON_SUP_MODE, GT_TRUE));

    /*if(!cpssDeviceRunCheck_onEmulator())*/
    {
         GT_U32 d2dIdx, d2dNum;
         for (d2dIdx = 0; d2dIdx < (hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS); d2dIdx++)
         {
            d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0); /*eagle */
            CHECK_STATUS(mvHwsD2dDisableAll50GChannel(devNum,d2dNum));
            d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1); /*raven */
            CHECK_STATUS(mvHwsD2dDisableAll50GChannel(devNum,d2dNum));
         }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("****** Falcon step 0 end ******\n");
    }
#endif
    return GT_OK;
}


GT_VOID hwsFalconAddrCalc
(
    GT_U8           devNum,
    HWS_UNIT_BASE_ADDR_TYPE_ENT addressType,
    GT_UOPT         portNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
)
{
    if(falconBaseAddrCalcFunc == NULL)
    {
        /* not implemented */
        *addressPtr = 0x11111111;
        return;
    }

    *addressPtr = regAddr + falconBaseAddrCalcFunc(devNum,addressType,portNum);
    return;
}
/**
* @internal hwsFalconUnitBaseAddrCalcBind function
* @endinternal
 *
* @param[in] cbFunc                   - the callback function
*/
GT_VOID hwsFalconUnitBaseAddrCalcBind
(
    HWS_UNIT_BASE_ADDR_CALC_BIND    cbFunc
)
{
    /* save the DB function */
    falconBaseAddrCalcFunc = cbFunc;
}

/**
* @internal hwsFalconSerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for Falcon
*
* @param[in] serdesNum                - global serdes lane number
* @param[in] regAddr                  - offset of required register in SD Unit (0x13000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsFalconSerdesAddrCalc
(
    GT_U8           devNum,
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
)
{
    if(falconBaseAddrCalcFunc == NULL)
    {
        /* not implemented */
        *addressPtr = 0x11111111;
        return;
    }
/*rrr*/
    *addressPtr = regAddr + falconBaseAddrCalcFunc(devNum,HWS_UNIT_BASE_ADDR_TYPE_SERDES_E,serdesNum);
    return;
}
/**
* @internal hwsFalconGopAddrCalc function
* @endinternal
*
* @brief   Calculate GOP register address for Falcon
*
* @param[in] portNum                  - global port number
* @param[in] regAddr                  - offset of required register in GOP Unit (0x10000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsFalconGopAddrCalc
(
    GT_U8           devNum,
    GT_UOPT         portNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
)
{
    if(falconBaseAddrCalcFunc == NULL)
    {
        /* not implemented */
        *addressPtr = 0x11111111;
        return;
    }

    *addressPtr = regAddr + falconBaseAddrCalcFunc(devNum,HWS_UNIT_BASE_ADDR_TYPE_GOP_E,portNum);
    return;
}

