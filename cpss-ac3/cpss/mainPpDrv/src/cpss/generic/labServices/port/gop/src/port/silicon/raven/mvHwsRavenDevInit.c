/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvHwsRavenDevInit.c
*
* DESCRIPTION:
*     Raven specific HW Services init
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

#if 0
#include <silicon/bobk/mvHwsBobKPortIf.h>
#include <silicon/bobcat3/mvHwsBobcat3PortIf.h>
#endif

#include <silicon/raven/mvHwsRavenPortIf.h>

#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/d2dMac/mvHwsD2dMacIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/d2dPcs/mvHwsD2dPcsIf.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/gw16/mvGw16If.h>


extern int hwsBobKConvertSbusAddrToSerdes
(
    unsigned char devNum,
    GT_U32 *serdesNum,
    unsigned int  sbusAddr
);


/************************* definition *****************************************************/

#define RAVEN_NOT_INITIALIZED_BASE_ADDR     (0xFFFFFFFF)

#define RAVEN_MPCS_BASE                     /*(0x10180000)*/    RAVEN_NOT_INITIALIZED_BASE_ADDR
#define RAVEN_CGPCS_BASE                    /*(0x10300000)*/    RAVEN_NOT_INITIALIZED_BASE_ADDR
#define RAVEN_GIG_BASE                      /*(0x10000000)*/    RAVEN_NOT_INITIALIZED_BASE_ADDR
#define RAVEN_XLG_BASE                      /*(0x100C0000)*/    RAVEN_NOT_INITIALIZED_BASE_ADDR
#define RAVEN_SERDES_BASE                   (0x3E000000)
#define RAVEN_CG_BASE                       /*(0x10340000)*/    RAVEN_NOT_INITIALIZED_BASE_ADDR
#define RAVEN_CG_RS_FEC_BASE                /*(0x10340800)*/    RAVEN_NOT_INITIALIZED_BASE_ADDR
#define RAVEN_PTP_BASE                      /*(0x10180800)*/    RAVEN_NOT_INITIALIZED_BASE_ADDR

#define IND_OFFSET                          (0x1000)
#define CG_IND_OFFSET                       /*(0x1000)*/        RAVEN_NOT_INITIALIZED_BASE_ADDR

#define RAVEN_SERDES_MAP_ARR_SIZE           21

static GT_U32 ravenSerdesToAvagoMap[RAVEN_SERDES_MAP_ARR_SIZE] =
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

GT_U32 ravenAvagoToSerdesMap[RAVEN_SERDES_MAP_ARR_SIZE] =
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

extern MV_HWS_PORT_INIT_PARAMS *hwsPortsRavenParams[];

extern MV_HWS_PORT_INIT_PARAMS hwsRavenPort0SupModes[];
extern MV_HWS_PORT_INIT_PARAMS *hwsPortsRavenParamsSupModesMap[];

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL ravenDbInitDone = GT_FALSE;

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

#if 0
    CHECK_STATUS(mvHwsGPcs28nmIfInit(hwsPcsFuncsPtr));  /* GPCS */
    CHECK_STATUS(mvHwsMMPcs28nmIfInit(hwsPcsFuncsPtr)); /* MMPCS */
    CHECK_STATUS(mvHwsCgPcs28nmIfInit(hwsPcsFuncsPtr)); /* CGPCS */
#endif
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

#if 0
    CHECK_STATUS(hwsGeMac28nmIfInit (devNum, hwsMacFuncsPtr));  /* GE MAC  */
    CHECK_STATUS(hwsXgMac28nmIfInit (devNum, hwsMacFuncsPtr));  /* XG MAC  */
    CHECK_STATUS(hwsXlgMac28nmIfInit(devNum, hwsMacFuncsPtr));  /* XLG MAC */
    CHECK_STATUS(hwsCgMac28nmIfInit (devNum, hwsMacFuncsPtr));  /* CG MAC  */
#ifndef MV_HWS_REDUCED_BUILD
    /* ILKN/SG configuration is not done in Internal CPU */
    CHECK_STATUS(hwsGeMacSgIfInit(devNum, hwsMacFuncsPtr));   /* GE_SG MAC */
#endif
#endif
    return GT_OK;
}

/*******************************************************************************
* hwsRavenSerdesIfInit
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
static GT_STATUS hwsRavenSerdesIfInit(GT_U8 devNum)
{
    MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;

    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

    /*CHECK_STATUS(mvHwsAvago16nmIfInit(devNum, hwsSerdesFuncsPtr));*/

    CHECK_STATUS(mvHwsD2dPhyIfInit(devNum, hwsSerdesFuncsPtr));

    return GT_OK;
}

/**
* @internal hwsRavenIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsRavenIfPreInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
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

    hwsDeviceSpecInfo[devNum].devType = Raven;

    return GT_OK;
}

/*******************************************************************************
* hwsRavenIfInit
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
GT_STATUS hwsRavenIfInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    GT_U32  data, version;
    MV_HWS_DEV_TYPE devType;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    GT_U32  i;

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }

#ifdef MV_HWS_REDUCED_BUILD
    if(ravenDbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsRavenIfPreInit(devNum, funcPtr));
    }
#endif

    hwsDeviceSpecInfo[devNum].devType = Raven;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].portsNum = 0;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = AVAGO_16NM;
    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber = MAX_AVAGO_SERDES_NUMBER;
    hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber = RAVEN_SERDES_MAP_ARR_SIZE;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesToAvagoMapPtr = &ravenSerdesToAvagoMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.avagoToSerdesMapPtr = &ravenAvagoToSerdesMap[0];
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride = GT_FALSE;
    hwsDeviceSpecInfo[devNum].serdesInfo.sbusAddrToSerdesFuncPtr = hwsBobKConvertSbusAddrToSerdes;

    devType = hwsDeviceSpecInfo[devNum].devType;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc == NULL)
    {
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc = funcPtr->sysDeviceInfo;
    }
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
#ifndef MV_HWS_REDUCED_BUILD
    hwsDeviceSpecInfo[devNum].portsParamsSupModesMap                    = hwsPortsRavenParamsSupModesMap;
#endif
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;

    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);

    hwsDeviceSpecInfo[devNum].gopRev = GOP_16NM_REV1;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc = NULL;

    /* Configures port init / reset functions */
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc = NULL;
#endif
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = NULL;

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

    hwsInitPortsModesParam(devNum,NULL);
    if(hwsInitDone[devNum] == GT_FALSE)
    {
#ifndef MV_HWS_REDUCED_BUILD
        CHECK_STATUS(hwsDevicePortsElementsCfg(devNum));
#endif
    }

    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(hwsRavenSerdesIfInit(devNum));

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));


    /* Update HWS After Firmware load and init */
    if (mvHwsServiceCpuEnableGet(devNum))
        CHECK_STATUS(mvHwsServiceCpuFwPostInit(devNum));

    /* Init configuration sequence executer */
    mvCfgSeqExecInit();

    mvUnitInfoSet(devType,  SERDES_UNIT,     RAVEN_SERDES_BASE,     IND_OFFSET);
#if 0
    mvUnitInfoSet(devType,  MMPCS_UNIT,      RAVEN_MPCS_BASE,       IND_OFFSET);
    mvUnitInfoSet(devType,  CGPCS_UNIT,      RAVEN_CGPCS_BASE,      CG_IND_OFFSET);
    mvUnitInfoSet(devType,  GEMAC_UNIT,      RAVEN_GIG_BASE,        IND_OFFSET);
    mvUnitInfoSet(devType,  XLGMAC_UNIT,     RAVEN_XLG_BASE,        IND_OFFSET);
    mvUnitInfoSet(devType,  CG_UNIT,         RAVEN_CG_BASE,         CG_IND_OFFSET);
    mvUnitInfoSet(devType,  CG_RS_FEC_UNIT,  RAVEN_CG_RS_FEC_BASE,  CG_IND_OFFSET);
    mvUnitInfoSet(devType,  PTP_UNIT,        RAVEN_PTP_BASE,        IND_OFFSET);
#endif

    ravenDbInitDone = GT_TRUE;
    hwsInitDone[devNum] = GT_TRUE;

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
* hwsRavenIfClose
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
void hwsRavenIfClose(GT_U8 devNum)
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

    ravenDbInitDone = GT_FALSE;

    serdesIfClose(devNum);

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsPortsElementsClose(devNum);
}

#endif

GT_STATUS hwsRavenCpllConfig
(
    IN GT_U8 devNum
)
{
    GT_U32      serdesNum;  /* serdes iterator */

    /******************************************************************************
     * CPLL initialization:
     *  CPLL_0 - used for network SERDESes -> configuration 156.25Mhz -> 156.25Mhz
     *  CPLL_1 - used for D2D GW SERDESes -> configuration 156.25Mhz -> 156.25Mhz
     ******************************************************************************/
    CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, 0xF8250, 16, 1, 1));

    /* CPLL_0 (network SERDES) initialization */
    for (serdesNum = 0; serdesNum < 17; serdesNum++)
    {
        CHECK_STATUS(mvHwsSerdesClockGlobalControl(devNum, 0, serdesNum,
                                                   MV_HWS_CPLL,
                                                   MV_HWS_156MHz_IN,
                                                   MV_HWS_156MHz_OUT));
    }


    /* CPLL_1 (D2D GW SERDES) initialization */
    CHECK_STATUS(mvHwsSerdesClockGlobalControl(devNum, 0, 17,
                                               MV_HWS_CPLL,
                                               MV_HWS_156MHz_IN,
                                               MV_HWS_156MHz_OUT));

    return GT_OK;
}

