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
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <silicon/bobcat2/mvHwsBobcat2PortIf.h>
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

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdes28nmPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nmRev3/mvComPhyH28nmRev3If.h>

#ifndef MV_HWS_REDUCED_BUILD
    #include <mvDdr3TrainingIpPrvIf.h>
    #include <mvHwsDdr3Bc2.h>
#endif

/*'global' variable that all HWS files can 'see' and use to diff from BC2*/
GT_U32 bobkSupported = 0;

/************************* definition *****************************************************/

#define BOBCAT2_NW_GIG_BASE     (0x10000000) /* ports 0 - 55 */
#define BOBCAT2_FB_GIG_BASE     (0x10200000) /* ports 56 - 71 */
#define BOBCAT2_NW_XPCS_BASE    (0x10180400)
#define BOBCAT2_FB_XPCS_BASE    (0x10380400)
#define BOBCAT2_NW_XLG_BASE     (0x100C0000) /* ports 0 - 55 */
/*#define BOBCAT2_FB_XLG_BASE     (0x102C0000)*/ /* ports 0 - 15 CHECK WITH VLADIMIR and CIDER */
#define BOBCAT2_NW_MPCS_BASE    (0x10180000) /* ports 48 - 55 */
#define BOBCAT2_FB_MPCS_BASE    (0x10380000) /* ports 56 - 71 */
#define BOBCAT2_ILKN_RF_BASE    (0x01F000000)
#define BOBCAT2_ILKN_BASE       (0x01F000800)

#define BOBCAT2_SERDES_BASE     (0x13000000)
#define BOBCAT2_SERDES_PHY_BASE (0x13000800)

#define BOBCAT2_PTP_BASE        (0x10180800)

#define IND_OFFSET (0x1000)

typedef enum
{
    DB_BOARD = 0,
    RD_BOARD = 1
}bc2BoardType;


/************************* Globals *******************************************************/

extern MV_HWS_PORT_INIT_PARAMS *hwsPortsBobcatParams[];

/* init per device */
static MV_INTLKN_REG *deviceInlkRegsShadow[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL bobcatDbInitDone = GT_FALSE;

/************************* pre-declaration ***********************************************/
static MV_INTLKN_REG* ilknRegDbGet(GT_U8 devNum, GT_U32 portGroup, GT_U32 address);
static GT_STATUS hwsILknRegDbInit(GT_U8 devNum);

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
static GT_STATUS pcsIfInit(GT_U8   devNum)
{
    MV_HWS_PCS_FUNC_PTRS *hwsPcsFuncsPtr;

    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsXPcsIfInit(hwsPcsFuncsPtr));      /* XPCS */
    CHECK_STATUS(mvHwsGPcs28nmIfInit(hwsPcsFuncsPtr));  /* GPCS */
    CHECK_STATUS(mvHwsMMPcs28nmIfInit(hwsPcsFuncsPtr)); /* MMPCS */
#ifndef MV_HWS_REDUCED_BUILD
    /* ILKN configuration is not done in Internal CPU */
    CHECK_STATUS(mvHwsIlknPcsIfInit(hwsPcsFuncsPtr));   /* ILKN */
#endif
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
#ifndef MV_HWS_REDUCED_BUILD
    /* ILKN/SG configuration is not done in Internal CPU */
    CHECK_STATUS(hwsGeMacSgIfInit(devNum, hwsMacFuncsPtr));   /* GE_SG MAC */
    CHECK_STATUS(hwsIlknMacIfInit(hwsMacFuncsPtr));           /* ILKN MAC */
#elif defined (MICRO_INIT)
    CHECK_STATUS(hwsGeMacSgIfInit(devNum, hwsMacFuncsPtr));   /* GE_SG MAC */
#endif
    return GT_OK;
}

#if (!defined MV_HWS_REDUCED_BUILD)
/**
* @internal mvHwsBobcat2ClkSelCfg function
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
static GT_STATUS mvHwsBobcat2ClkSelCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
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
* @internal serdesIfInit function
* @endinternal
*
* @brief   Init all supported Serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS serdesIfInit(GT_U8 devNum, GT_U32 devRevision)
{
    MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;

    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

    if (devRevision > 0)
    {
        CHECK_STATUS(mvHwsComH28nmRev3IfInit(hwsSerdesFuncsPtr));
    }
    else
    {
        CHECK_STATUS(mvHwsComH28nmIfInit(hwsSerdesFuncsPtr));
    }

    return GT_OK;
}

/**
* @internal bc2RedundancyVectorGet function
* @endinternal
*
* @brief   Get SD vector.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS bc2RedundancyVectorGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                 *sdVector
)
{

    devNum = devNum;
    portGroup = portGroup;

    /* there is no Redundancy in BC2 */
    *sdVector = 0x0;

    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal Getbc2BoardType function
* @endinternal
*
* @brief   Get board Type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS Getbc2BoardType(GT_U8 devNum, bc2BoardType *board)
{
    GT_U32 data = 0;
    GT_U32 portGroup = 0xFFFFFFFF;

    if (bobkSupported == 1 || hwsDeviceSpecInfo[devNum].gopRev >= GOP_28NM_REV2)
    {
        /* No LMS unit in BC2-B0 and BobK,
            so can't use BC2 addresses (that not exists any more) */
        *board = DB_BOARD;
        return GT_OK;
    }

    /* read phy to detect board */
    /* TBD should use SMI driver instead - SMI is not ready at this point yet*/

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum,  portGroup, 0x21004200, &data, 0));
    if (data & (1<<20))
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, 0x21004200, data & ~(1<<20), 0));
    }
    /* read phy 0 reg 2 for board type */
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, 0x21004054, 0x4400000, 0));
    hwsOsExactDelayPtr(devNum,0,10);
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum,  portGroup, 0x21004054, &data, 0));

    /*DEBUG("phy board register 0x%x\n", data);*/

    data &= 0xffff;

    if (data == 0x0141)
    {
        /* RD board */
        /*DEBUG("RD Board identify\n");*/
        *board = RD_BOARD;
    }
    else
    {
        /* by default suppose this is DB */
        /* DB board */
        /*DEBUG("DB Board identify\n");*/
        *board = DB_BOARD;
    }

    return GT_OK;
}

/**
* @internal TrainingIpIfInit function
* @endinternal
*
* @brief   Init training IP functionality.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS TrainingIpIfInit(GT_U8 devNum)
{
    /* DDR configuration is not done in Internal CPU */
    MV_HWS_TRAINING_IP_FUNC_PTRS *funcPtrArray;

    bc2BoardType board = DB_BOARD;

    CHECK_STATUS(Getbc2BoardType(devNum, &board));
    CHECK_STATUS(ddr3TipInitBc2(devNum, board));

    if (bobcatDbInitDone == GT_FALSE)
    {
        mvHwsTrainingIpGetFuncPtr(&funcPtrArray);
        mvHwsDdr3TipIfInit(funcPtrArray);
    }

    return GT_OK;
}
#endif /* MV_HWS_REDUCED_BUILD */

/**
* @internal hwsBobcat2IfInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobcat2IfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr)
{
    GT_U32  portGroup = devNum;
    GT_U32  portNum;
#ifndef MV_HWS_REDUCED_BUILD
    GT_U32  portMode;
    GT_STATUS res;
#endif /* MV_HWS_REDUCED_BUILD */
    GT_U32  data, version;
    MV_HWS_DEV_TYPE devType;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    GT_U32 i;

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }

    if (bobcatDbInitDone == GT_FALSE)
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
#if (!defined MV_HWS_REDUCED_BUILD)
            || (funcPtr->coreClockGetPtr == NULL)
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

    }

    hwsDeviceSpecInfo[devNum].devType = BobcatA0;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].portsNum = 72;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = _5GBaseR+1;
    hwsDeviceSpecInfo[devNum].serdesType = COM_PHY_28NM;
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
        CHECK_STATUS(mvHwsServiceCpuFwInit(devNum, 0xffffffff, "AP_Bobcat2"));

    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);

    if (version > 0)
    {
        hwsDeviceSpecInfo[devNum].gopRev = GOP_28NM_REV2;
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = NULL;
        /* hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portBetterAdapSetFunc = mvHwsBobcat2PortBetterAdaptationSet; */
    }
    else
    {
        hwsDeviceSpecInfo[devNum].gopRev = GOP_28NM_REV1;
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = mvHwsBobcat2PCSMarkModeSet;
        /* hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portBetterAdapSetFunc = NULL; */
    }

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = bc2RedundancyVectorGet;

    /* Configures the DP/Core Clock Selector on port */
#if (!defined MV_HWS_REDUCED_BUILD)
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc = mvHwsBobcat2ClkSelCfg;
#endif
    /* Configures port init / reset functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc  = mvHwsBobcat2PortInit;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = mvHwsBobcat2ApPortInit;  /* for AP port */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsBobcat2PortReset;

    /* Configures  device handler functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = mvHwsBobcat2PortAutoTuneSetExt;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = mvHwsBobcat2PortExtendedModeCfg;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = mvHwsBobcat2PortExtendedModeCfgGet;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func     = mvHwsBobcat2PortFixAlign90Ext;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portLbSetWaFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc   = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc  = NULL;


#ifndef CO_CPU_RUN
    hwsInitPortsModesParam(devNum,hwsPortsBobcatParams);
    if (hwsInitDone[devNum] == GT_FALSE)
    {
        hwsBobcatPorts47Cfg(devNum, portGroup);
        hwsBobcatPorts71Cfg(devNum, portGroup);
    }
#endif

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(serdesIfInit(devNum, version));

    /* Update HWS After Firmware load and init */
    if (mvHwsServiceCpuEnableGet(devNum))
        CHECK_STATUS(mvHwsServiceCpuFwPostInit(devNum));

    mvUnitInfoSet(devType, GEMAC_UNIT,       BOBCAT2_NW_GIG_BASE,        IND_OFFSET);
    mvUnitInfoSet(devType, MMPCS_UNIT,       BOBCAT2_NW_MPCS_BASE,       IND_OFFSET);
    mvUnitInfoSet(devType, XPCS_UNIT,        BOBCAT2_NW_XPCS_BASE,       IND_OFFSET);
    mvUnitInfoSet(devType, XLGMAC_UNIT,      BOBCAT2_NW_XLG_BASE,        IND_OFFSET);
    mvUnitInfoSet(devType, SERDES_UNIT,      BOBCAT2_SERDES_BASE,        IND_OFFSET);
    mvUnitInfoSet(devType, SERDES_PHY_UNIT,  BOBCAT2_SERDES_PHY_BASE,    IND_OFFSET);
    mvUnitInfoSet(devType, INTLKN_RF_UNIT,   BOBCAT2_ILKN_RF_BASE,       1 /* irrelevant since there is only 1 unit but needed for CPSS index check */);
    mvUnitInfoSet(devType, INTLKN_UNIT,      BOBCAT2_ILKN_BASE,          1 /* irrelevant since there is only 1 unit  but needed for CPSS index check */);
    mvUnitInfoSet(devType, PTP_UNIT,         BOBCAT2_PTP_BASE,           IND_OFFSET);


#ifndef MV_HWS_REDUCED_BUILD
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
#else
    /* Internal CPU receives the port parameters from the Host so no need to hold table per port mode */
    for (portNum = 0; portNum < HWS_CORE_PORTS_NUM(devNum); portNum++)
    {
        CHECK_STATUS(mvHwsBuildActiveLaneList(devNum, portGroup, portNum, (MV_HWS_PORT_STANDARD)0 /*portMode*/));
    }
#endif

    /* init ILKN reg DB (for write-only registers) */
    CHECK_STATUS(hwsILknRegDbInit(devNum));

#ifndef MV_HWS_REDUCED_BUILD
    /* DDR training initialization */
    res = TrainingIpIfInit(devNum);
    if ((res != GT_OK) && (res != GT_NOT_INITIALIZED))
    {
        return res;
    }
#endif /* MV_HWS_REDUCED_BUILD */

    bobcatDbInitDone = GT_TRUE;
    hwsInitDone[devNum] = GT_TRUE;

    return GT_OK;
}


#ifndef MV_HWS_REDUCED_BUILD

/**
* @internal hwsBobcat2IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsBobcat2IfClose(GT_U8 devNum)
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

    bobcatDbInitDone = GT_FALSE;

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsPortsElementsClose(devNum);
}
#endif /* MV_HWS_REDUCED_BUILD */
static MV_INTLKN_REG* ilknRegDbGet(GT_U8 devNum, GT_U32 portGroup, GT_U32 address)
{
    /* avoid warnings */
    portGroup = portGroup;
    address = address;

    return deviceInlkRegsShadow[devNum];
}

GT_STATUS hwsILknRegDbInit(GT_U8 devNum)
{
    GT_U8 shadowInd;

#ifdef VIPS_RUN
    shadowInd = devNum / 2;
#else
    shadowInd = devNum;
#endif

    /* pointer to a function which returns the ILKN registers DB */
    hwsIlknRegDbGetFuncPtr = ilknRegDbGet;

    if (deviceInlkRegsShadow[shadowInd] == NULL )
    {
        /* init device registers shadow with default values */
        deviceInlkRegsShadow[shadowInd] = (MV_INTLKN_REG *) hwsOsMallocFuncPtr(sizeof(MV_INTLKN_REG));

        deviceInlkRegsShadow[shadowInd]->ilkn0MacCfg0 = 8705;      /*ILKN_0_MAC_CFG_0 = 0,*/
        deviceInlkRegsShadow[shadowInd]->ilkn0ChFcCfg0 = 0;        /*ILKN_0_CH_FC_CFG_0 = 4,*/
        deviceInlkRegsShadow[shadowInd]->ilkn0MacCfg2 = 268500992; /*ILKN_0_MAC_CFG_2 = 0xC,*/
        deviceInlkRegsShadow[shadowInd]->ilkn0MacCfg3 = 2827;      /*ILKN_0_MAC_CFG_3 = 0x10,*/
        deviceInlkRegsShadow[shadowInd]->ilkn0MacCfg4 = 131585;    /*ILKN_0_MAC_CFG_4 = 0x14,*/
        deviceInlkRegsShadow[shadowInd]->ilkn0MacCfg6 = 0;         /*ILKN_0_MAC_CFG_6 = 0x1C,*/
        deviceInlkRegsShadow[shadowInd]->ilkn0ChFcCfg1 = 65280;    /*ILKN_0_CH_FC_CFG_1 = 0x60,*/
        deviceInlkRegsShadow[shadowInd]->ilkn0PcsCfg0 = 4128831;
        deviceInlkRegsShadow[shadowInd]->ilkn0PcsCfg1 = 16777215;  /*ILKN_0_PCS_CFG_1 = 0x204,*/
        deviceInlkRegsShadow[shadowInd]->ilkn0En = 0;              /*ILKN_0_EN = 0x20C,*/
        deviceInlkRegsShadow[shadowInd]->ilkn0StatEn = 0;          /*ILKN_0_STAT_EN = 0x238,*/
    }

    return GT_OK;
}


