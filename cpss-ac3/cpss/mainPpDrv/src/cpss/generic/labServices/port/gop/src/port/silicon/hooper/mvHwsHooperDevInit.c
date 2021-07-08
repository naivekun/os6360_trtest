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
* mvHwsHooperIfDevInit.c
*
* DESCRIPTION:
*       Hooper specific HW Services init
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <silicon/lion2/mvHwsLion2PortIf.h> /* Hooper and Lion2 have the same port functionality */
#include <silicon/hooper/mvHwsHooperPortIf.h> /* TBD - should be removed (and deleted) once Hooper
                                                       PortCfg is unified with Lion2*/
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/hglMac/mvHwsHglMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/hglPcs/mvHwsHglPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsV2If.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH/mvComPhyHIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyHRev2/mvComPhyHRev2If.h>


#define LION2_GIG_BASE (0x08800000)
#define LION2_XPCS_BASE (0x089c0200)
#define LION2_XLG_BASE  (0x088c0000)
#define LION2_MMPCS_BASE (0x088c0400)
#define LION2_SERDES_BASE (0x09800000)
#define SERDES_EXTERNAL_REGS (0x09800000)

#define GIG_IND_OFFSET   (0x1000)
#define XPCS_IND_OFFSET   (0x2000)
#define MMPCS_IND_OFFSET (0x1000)
#define MSM_IND_OFFSET (0x1000)
#define SERDES_IND_OFFSET  (0x400)


extern MV_HWS_PORT_INIT_PARAMS *hwsPortsLion2Params[];
/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL lion2DbInitDone = GT_FALSE;

extern GT_STATUS hwsPortFixAlign90ExtMultipleLaneOptimization (
    GT_U8 devNum, GT_U32 portGroup, GT_U32 phyPortNum,
    MV_HWS_PORT_STANDARD portMode,  GT_U32 optAlgoMask);

/**
* @internal pcsIfInit function
* @endinternal
*
* @brief   Init all supported PCS types relevant for Lion2 devices.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS pcsIfInit(GT_U8 devNum)
{
    MV_HWS_PCS_FUNC_PTRS *hwsPcsFuncsPtr;

    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsGPcsIfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMMPcsV2IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsHglPcsIfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsXPcsIfInit(hwsPcsFuncsPtr));

    return GT_OK;
}

/**
* @internal macIfInit function
* @endinternal
*
* @brief   Init all Lion2 supported MAC types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS macIfInit(GT_U8 devNum)
{
    MV_HWS_MAC_FUNC_PTRS *hwsMacFuncsPtr;

    CHECK_STATUS(hwsMacGetFuncPtr(devNum, &hwsMacFuncsPtr));

    CHECK_STATUS(hwsGeMacIfInit(devNum, hwsMacFuncsPtr));
    CHECK_STATUS(hwsGeMacSgIfInit(devNum, hwsMacFuncsPtr));
    CHECK_STATUS(hwsHglMacIfInit(hwsMacFuncsPtr));
    CHECK_STATUS(hwsXgMacIfInit(hwsMacFuncsPtr));
    CHECK_STATUS(hwsXlgMacIfInit(hwsMacFuncsPtr));

    return GT_OK;
}

/**
* @internal serdesIfInit function
* @endinternal
*
* @brief   Init all supported PCS types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS serdesIfInit(GT_U8 devNum)
{
    MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;

    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));
    CHECK_STATUS(mvHwsComHRev2IfInit(hwsSerdesFuncsPtr));

    return GT_OK;
}

/**
* @internal lion2RedundancyVectorGet function
* @endinternal
*
* @brief   Get SD vector.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS lion2RedundancyVectorGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  *sdVector
)
{
    GT_U32 data;
    GT_U32 accessAddr = MACRO_CONTROL;
    GT_U32 unitIndex;
    GT_U32 baseAddr;

    mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex);

    accessAddr  = baseAddr + MACRO_CONTROL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
    *sdVector = data & 0xF;
    accessAddr  = baseAddr + MACRO_CONTROL + 4*unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
    *sdVector += ((data & 0xF) << 6);
    accessAddr  = baseAddr + MACRO_CONTROL + 8*unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
    *sdVector += ((data & 0xF) << 12);
    accessAddr  = baseAddr + MACRO_CONTROL + 12*unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
    *sdVector += ((data & 0xF) << 18);

    return GT_OK;
}

/**
* @internal hwsHooperIfInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsHooperIfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr)
{
    GT_U32  portGroup = 0; /* During device Init all portGroups are initialized with the same DB configuration.  */
    GT_U32  portNum;
    GT_U32  portMode;
    GT_U32  data, version;
    MV_HWS_DEV_TYPE devType;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    GT_STATUS rc;
    GT_BOOL portActiveLanesInitialized;
    GT_U32 i;

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }

    if(lion2DbInitDone == GT_FALSE)
    {
        if (funcPtr == NULL)
            return GT_FAIL;

        if ((funcPtr->osTimerWkPtr == NULL) ||
            (funcPtr->osMemSetPtr == NULL) ||
            (funcPtr->osFreePtr == NULL) ||
            (funcPtr->osMallocPtr == NULL) ||
            (funcPtr->osExactDelayPtr == NULL) ||
            (funcPtr->osMemCopyPtr == NULL))
                return GT_BAD_PARAM;

        hwsOsExactDelayPtr = funcPtr->osExactDelayPtr;
        hwsOsTimerWkFuncPtr = funcPtr->osTimerWkPtr;
        hwsOsMemSetFuncPtr = funcPtr->osMemSetPtr;
        hwsOsFreeFuncPtr = funcPtr->osFreePtr;
        hwsOsMallocFuncPtr = funcPtr->osMallocPtr;
        hwsOsMemCopyFuncPtr = funcPtr->osMemCopyPtr;
        hwsRegisterSetFuncPtr = funcPtr->registerSetAccess;
        hwsRegisterGetFuncPtr = funcPtr->registerGetAccess;
    }

    /* Init configuration sequence executer */
    mvCfgSeqExecInit();

    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].portsNum = 12;
    hwsDeviceSpecInfo[devNum].gopRev = GOP_40NM_REV1;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = _48GBase_SR + 1;

    hwsDeviceSpecInfo[devNum].devType = HooperA0;
    hwsDeviceSpecInfo[devNum].serdesType = COM_PHY_H_REV2;
    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber = 0;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesToAvagoMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].serdesInfo.spicoNumber = 0;
    hwsDeviceSpecInfo[devNum].serdesInfo.avagoToSerdesMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride = GT_FALSE;
    hwsDeviceSpecInfo[devNum].serdesInfo.sbusAddrToSerdesFuncPtr = NULL;

    /* define Host to HWS connection type  - now Legacy mode*/
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;

    devType = hwsDeviceSpecInfo[devNum].devType;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[devType].deviceInfoFunc == NULL)
    {
        hwsDevFunc[devType].deviceInfoFunc = funcPtr->sysDeviceInfo;
    }

    /* get device id and version */
    hwsDevFunc[devType].deviceInfoFunc(devNum, &data, &version);

    hwsDevFunc[devType].redundVectorGetFunc = lion2RedundancyVectorGet;



    /* Configures port init / reset functions (Hooper device is similar to Lion2) */
    hwsDevFunc[devType].devPortinitFunc = mvHwsLion2PortInit;
    hwsDevFunc[devType].devPortResetFunc = mvHwsLion2PortReset;

    /* Configures  device handler functions */
    /* TBD - all this functionality should be taken from Lion2 */
    hwsDevFunc[devType].portAutoTuneStopFunc   = mvHwsPortLion2AutoTuneStop;
    hwsDevFunc[devType].pcsMarkModeFunc        = mvHwsHooperPCSMarkModeSet;
    hwsDevFunc[devType].portAutoTuneSetExtFunc = mvHwsLion2PortAutoTuneSetExt;
    hwsDevFunc[devType].portExtModeCfgFunc     = NULL;
    hwsDevFunc[devType].portExtModeCfgGetFunc  = mvHwsPortLion2ExtendedModeCfgGet;
    hwsDevFunc[devType].portLbSetWaFunc        = NULL;
    hwsDevFunc[devType].portAutoTuneSetWaFunc  = NULL;
    hwsDevFunc[devType].portFixAlign90Func     = NULL;

    /* Register silicon port modes parameters in the system */
    hwsInitPortsModesParam(devNum,hwsPortsLion2Params);

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(serdesIfInit(devNum));

    mvUnitInfoSet(devType, GEMAC_UNIT, LION2_GIG_BASE, GIG_IND_OFFSET);
    mvUnitInfoSet(devType, MMPCS_UNIT, LION2_MMPCS_BASE, MMPCS_IND_OFFSET);
    mvUnitInfoSet(devType, XPCS_UNIT, LION2_XPCS_BASE, XPCS_IND_OFFSET);
    mvUnitInfoSet(devType, HGLMAC_UNIT, LION2_MMPCS_BASE, MMPCS_IND_OFFSET);
    mvUnitInfoSet(devType, XLGMAC_UNIT, LION2_XLG_BASE, MSM_IND_OFFSET);
    mvUnitInfoSet(devType, SERDES_UNIT, LION2_SERDES_BASE, SERDES_IND_OFFSET);

    /* init active lines in ports DB. For every port in Hws databse, there is at least one valid entry,
       each entry contains information about it's serdes lanes, and the serdes lanes list which is initialized here.
       We check overall return code to deny a case where for a given port, non of it's serdes list was initialized. */
    for (portNum = 0; (!hwsInitDone[devNum]) && (portNum < HWS_CORE_PORTS_NUM(devNum)); portNum++)
    {
        /* For every port and port mode, initializing the variables responsible for marking whether or not the lanes list initialized */
        portActiveLanesInitialized = GT_FALSE;
        /* Iterating all port modes, only the modes the port suuports will initialize the proper lanes list */
        for (portMode = 0; portMode < HWS_DEV_PORT_MODES(devNum); portMode++)
        {
            rc = mvHwsBuildActiveLaneList(devNum, portGroup, portNum, (MV_HWS_PORT_STANDARD)portMode);
            /* Marking lanes list as initialized  */
            if (rc == GT_OK && portActiveLanesInitialized != GT_TRUE)
            {
                portActiveLanesInitialized = GT_TRUE;
            }
        }
        /* Lanes list was not initialized for any port mode of that port */
        if (portActiveLanesInitialized == GT_FALSE)
        {
            return GT_FAIL;
        }
    }

    /* init active lanes for extended port, since extended port parameters are taken from different DB.
       The check for serdeses lanes list is performed here as well. */
    for (portNum = HWS_CORE_PORTS_NUM(devNum); (!hwsInitDone[devNum]) && (portNum < HWS_CORE_PORTS_NUM(devNum)+2); portNum++)
    {
        /* For every port and port mode, initializing the variables responsible for marking whether or not the lanes list initialized */
        portActiveLanesInitialized = GT_FALSE;
        /* Iterating all port modes, only the modes the port suuports will initialize the proper lanes list */
        for (portMode = 0; portMode < HWS_DEV_PORT_MODES(devNum); portMode++)
        {
            rc = mvHwsBuildActiveLaneList(devNum, portGroup, portNum, (MV_HWS_PORT_STANDARD)portMode);
            /* Marking lanes list as initialized  */
            if (rc == GT_OK && portActiveLanesInitialized != GT_TRUE)
            {
                portActiveLanesInitialized = GT_TRUE;
            }
        }
        /* Lanes list was not initialized for any port mode of that port */
        if (portActiveLanesInitialized == GT_FALSE)
        {
            return GT_FAIL;
        }
    }

    lion2DbInitDone = GT_TRUE;
    hwsInitDone[devNum] = GT_TRUE;

    return GT_OK;
}

/**
* @internal hwsHooperIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsHooperIfClose(GT_U8 devNum)
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
    lion2DbInitDone = GT_FALSE;

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsPortsElementsClose(devNum);
}


