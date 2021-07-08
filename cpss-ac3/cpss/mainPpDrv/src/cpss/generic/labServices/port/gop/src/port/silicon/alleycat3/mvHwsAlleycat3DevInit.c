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
* @file mvHwsAlleycat3DevInit.c
*
* @brief Bobcat specific HW Services init
*
* @version   23
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <silicon/alleycat3/mvHwsAlleycat3PortIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdes28nmPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nmRev3/mvComPhyH28nmRev3If.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nmRev3/mvComPhyH28nmRev3Db.h>

/************************* definition *****************************************************/
#define ALLEYCAT3_NW_GIG_BASE   (0x12000000)
#define ALLEYCAT3_FB_GIG_BASE   (0x10200000)
#define ALLEYCAT3_NW_XPCS_BASE  (0x12180400)
#define ALLEYCAT3_FB_XPCS_BASE  ALLEYCAT3_NW_XPCS_BASE
#define ALLEYCAT3_NW_XLG_BASE   (0x120C0000)
#define ALLEYCAT3_NW_MPCS_BASE  (0x12180000)
#define ALLEYCAT3_FB_MPCS_BASE  ALLEYCAT3_NW_MPCS_BASE

#define ALLEYCAT3_SERDES_BASE   (0x13000000)
#define ALLEYCAT3_SERDES_PHY_BASE   (0x13000800)

#define IND_OFFSET (0x1000)

/************************* Globals *******************************************************/
extern MV_HWS_PORT_INIT_PARAMS *hwsPortsAlleycat3Params[];

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL alleycat3DbInitDone = GT_FALSE;

/************************* pre-declaration ***********************************************/

extern GT_STATUS hwsAlleycat3Ports1To23Cfg (GT_U8 devNum, GT_U32 portGroup);

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
* @internal hwsAc3PcsIfInit function
* @endinternal
*
* @brief   Init all supported PCS types relevant for devices.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS hwsAc3PcsIfInit(GT_U8 devNum)
{
    MV_HWS_PCS_FUNC_PTRS *hwsPcsFuncsPtr;

    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsXPcsIfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsGPcs28nmIfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMMPcs28nmIfInit(hwsPcsFuncsPtr));

    return GT_OK;
}

/**
* @internal hwsAc3MacIfInit function
* @endinternal
*
* @brief   Init all supported MAC types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS hwsAc3MacIfInit(GT_U8 devNum)
{
    MV_HWS_MAC_FUNC_PTRS *hwsMacFuncsPtr;

    CHECK_STATUS(hwsMacGetFuncPtr(devNum, &hwsMacFuncsPtr));

    CHECK_STATUS(hwsGeMac28nmIfInit(devNum, hwsMacFuncsPtr));
    CHECK_STATUS(hwsGeMacSgIfInit(devNum, hwsMacFuncsPtr));
    CHECK_STATUS(hwsXgMac28nmIfInit(hwsMacFuncsPtr));
    CHECK_STATUS(hwsXlgMac28nmIfInit(devNum, hwsMacFuncsPtr));

    return GT_OK;
}

#if (!defined MV_HWS_REDUCED_BUILD)
/**
* @internal mvHwsAlleycat3ClkSelCfg function
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
GT_STATUS mvHwsAlleycat3ClkSelCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
#ifndef MV_HWS_FREE_RTOS
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    GT_U32  coreClk = 0, coreClkHw;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    CHECK_STATUS(hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams));

    /* read core clock */
    CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc(devNum,&coreClk,&coreClkHw));

    if ((portMode == SGMII2_5) || (portMode == SGMII) || (portMode == _1000Base_X))
    {
        if ((portMode == SGMII2_5) && (coreClk < 312))
        {
            /* Set DP Clock Selector */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
                            GIG_PORT_MAC_CONTROL_REGISTER4, (1 << 5), (1 << 5)));
        }
        else
        {
            /* Set Core Clock Selector */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
                            GIG_PORT_MAC_CONTROL_REGISTER4, 0, (1 << 5)));
        }

        CHECK_STATUS(hwsPulse1msRegCfg(devNum, portGroup, curPortParams.portMacNumber, portMode, coreClk));
    }
#endif
    return GT_OK;
}
#endif

/**
* @internal hwsAc3SerdesIfInit function
* @endinternal
*
* @brief   Init all supported SERDES types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS hwsAc3SerdesIfInit(GT_U8 devNum)
{
    MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;

    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));
    CHECK_STATUS(mvHwsComH28nmRev3IfInit(hwsSerdesFuncsPtr));

    return GT_OK;
}

/**
* @internal mvHwsAlleycat3IfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAlleycat3IfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr)
{
    GT_U32  portGroup = devNum;
    GT_U32  portNum;
    GT_U32  data, version;
    MV_HWS_DEV_TYPE devType;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    GT_U32  portMode;
    GT_STATUS res;
    GT_U32 i;

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }

    if(alleycat3DbInitDone == GT_FALSE)
    {
        if (funcPtr == NULL)
            return GT_BAD_PARAM;

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
            return GT_BAD_PARAM;
        }

        if (funcPtr->osMicroDelayPtr == NULL)
        {
            hwsOsMicroDelayPtr = hwsOsLocalMicroDelay;
        }
        else
        {
            hwsOsMicroDelayPtr = funcPtr->osMicroDelayPtr;
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
#ifndef MV_HWS_FREE_RTOS
        hwsTimerGetFuncPtr = funcPtr->timerGet;
#endif
    }

    hwsDeviceSpecInfo[devNum].devType = Alleycat3A0;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].portsNum = 32;
    hwsDeviceSpecInfo[devNum].gopRev = GOP_28NM_REV1;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = _50GBase_CR2_C /*LAST_PORT_MODE*/;
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

    devType = hwsDeviceSpecInfo[devNum].devType;

    hwsDeviceSpecInfo[devNum].serdesType = COM_PHY_28NM;

    /* define Host to HWS connection type  - now Legacy mode*/
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;

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

    /* Init HWS Firmware */
    if (mvHwsServiceCpuEnableGet(devNum))
        CHECK_STATUS(mvHwsServiceCpuFwInit(devNum, 0xffffffff, "AP_AC3"));

    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);

    /* AlleyCat3 has no redundancy lanes, thus the value of redundVectorGetFunc is NULL */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

#if (!defined MV_HWS_REDUCED_BUILD)
    /* Configures the DP/Core Clock Selector on port */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc    = mvHwsAlleycat3ClkSelCfg;
#endif

    /* Configures port init / reset functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc  = mvHwsAlleycat3PortInit;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = mvHwsAlleycat3ApPortInit;  /* for AP port */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsAlleycat3PortReset;

    /* Configures  device handler functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = mvHwsAlleycat3PortAutoTuneSetExt;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = mvHwsAlleycat3PortExtendedModeCfg;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = mvHwsAlleycat3PortExtendedModeCfgGet;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func     = mvHwsAlleycat3PortFixAlign90Ext;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portLbSetWaFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc   = NULL;

#ifndef CO_CPU_RUN
    hwsInitPortsModesParam(devNum,hwsPortsAlleycat3Params);
    if (hwsInitDone[devNum] == GT_FALSE)
    {
        CHECK_STATUS(hwsAlleycat3Ports1To23Cfg(devNum, portGroup));
    }
#endif


    /* Init all MAC units relevant for current device */
    CHECK_STATUS(hwsAc3MacIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(hwsAc3PcsIfInit(devNum));

    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(hwsAc3SerdesIfInit(devNum));

    /* Update HWS After Firmware load and init */
    if (mvHwsServiceCpuEnableGet(devNum))
        CHECK_STATUS(mvHwsServiceCpuFwPostInit(devNum));

    /* Init silicon specific base address and index for specified unit */
    mvUnitInfoSet(devType, GEMAC_UNIT, ALLEYCAT3_NW_GIG_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, MMPCS_UNIT, ALLEYCAT3_NW_MPCS_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, XPCS_UNIT, ALLEYCAT3_NW_XPCS_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, XLGMAC_UNIT, ALLEYCAT3_NW_XLG_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, SERDES_UNIT, ALLEYCAT3_SERDES_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, SERDES_PHY_UNIT, ALLEYCAT3_SERDES_PHY_BASE, IND_OFFSET);

    /* init active lines in ports DB */
    for (portNum = 0; (!hwsInitDone[devNum]) && (portNum < HWS_CORE_PORTS_NUM(devNum)); portNum++)
    {
        for (portMode = 0; portMode < HWS_DEV_PORT_MODES(devNum); portMode++)
        {
            res = mvHwsBuildActiveLaneList(devNum, portGroup, portNum, (MV_HWS_PORT_STANDARD)portMode);
            if ((res != GT_OK) && (res != GT_NOT_SUPPORTED))
            {
                return res;
            }
        }
    }

    alleycat3DbInitDone = GT_TRUE;

    hwsInitDone[devNum] = GT_TRUE;

    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal hwsAlleycat3IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsAlleycat3IfClose(GT_U8 devNum)
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
    alleycat3DbInitDone = GT_FALSE;

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsPortsElementsClose(devNum);
}
#endif



