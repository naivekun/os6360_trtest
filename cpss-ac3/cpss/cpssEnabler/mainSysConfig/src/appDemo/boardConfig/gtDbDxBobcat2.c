/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtDbDxBobcat2.c
*
* @brief Initialization functions for the Bobcat2 - SIP5 - board.
*
* @version   118
********************************************************************************
*/
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/gtAppDemoTmConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtBoardsConfigFuncs.h>
#include <appDemo/boardConfig/gtDbDxBobcat2PhyConfig.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/gtDbDxBobcat2GenUtils.h>
#include <appDemo/boardConfig/gtDbDxBobcat2LedIfConfig.h>

#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/generic/cpssHwInit/cpssLedCtrl.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/private/utils/prvCpssTimeRtUtils.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/generic/tm/prvCpssTmCtl.h>


#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitGlobal2localDma.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPip.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>

/* TM glue */
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDram.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueQueueMap.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDrop.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueFlowControl.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGluePfc.h>
/* TM */
#include <cpss/generic/tm/cpssTmSched.h>

/* PTP */
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>


#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortInterlaken.h>

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
/* remove when no need in direct write to tables */
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagPacketGenerator.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>

#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsExc.h>

#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtExtDrv/drivers/gtUartDrv.h>
#include <gtExtDrv/drivers/gtI2cDrv.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

/* include the external services */
#include <cmdShell/common/cmdExtServices.h>

#ifndef ASIC_SIMULATION

/* for mtdParallelEraseFlashImage */
#include <88x3240/mtdApiTypes.h>
#include <88x3240/mtdHwCntl.h>
#include <88x3240/mtdAPI.h>
#include <88x3240/mtdFwDownload.h>
#include <88x3240/mtdHunit.h>
#include <88x3240/mtdCunit.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#endif


#define PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(x) x = x

extern void appDemoRtosOnSimulationInit(void);

extern GT_STATUS prvCpssTmCtlGlobalParamsInit
(
    IN GT_U8 devNum
);

extern GT_STATUS AN_WA_Task_Create
(
    GT_VOID
);

extern GT_STATUS bc2AppDemoPortsInit
(
    IN  GT_U8                           devNumber,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp,
    IN  GT_U32                          numOfPorts,
    ...
);

static GT_STATUS afterInitDeviceConfig
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

static GT_STATUS configDeviceAfterPhase1
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

static GT_STATUS configDeviceAfterPhase2
(
    IN GT_U8    devIdx,
    IN GT_U8    boardRevId
);


#ifndef ASIC_SIMULATION_NO_HW
extern GT_STATUS bc2AppDemoApInit
(
    IN  GT_U8   devNum
);
#endif

extern GT_BOOL  changeToDownHappend[CPSS_MAX_PORTS_NUM_CNS];
extern GT_BOOL  changeToUpHappend[CPSS_MAX_PORTS_NUM_CNS];

/*******************************************************************************
 * Global variables
 ******************************************************************************/
static GT_U8    ppCounter = 0;     /* Number of Packet processors. */
static GT_BOOL  noBC2CpuPort = GT_FALSE;
static CPSS_DXCH_IMPLEMENT_WA_ENT BC2WaList[] =
{
    CPSS_DXCH_IMPLEMENT_WA_BOBCAT2_REV_A0_40G_NOT_THROUGH_TM_IS_PA_30G_E,
    CPSS_DXCH_IMPLEMENT_WA_LAST_E
};

#define PLL4_FREQUENCY_1250000_KHZ_CNS  1250000
#define PLL4_FREQUENCY_1093750_KHZ_CNS  1093750
#define PLL4_FREQUENCY_1550000_KHZ_CNS  1550000

static GT_U32   bc2BoardType  = APP_DEMO_BC2_BOARD_DB_CNS;
static GT_U32   bc2BoardRevId = 0;
static GT_BOOL  bc2BoardResetDone = GT_FALSE;
static GT_BOOL  isBobkBoard = GT_FALSE;
static GT_U32   oobPort = 90;
static GT_BOOL  oobPortUsed = GT_FALSE;
static GT_BOOL  softResetSupported = GT_TRUE;

GT_STATUS appDemoBoardTypeGet
(
    OUT GT_U32 *boardTypePtr
)
{
    if(boardTypePtr == NULL)
    {
        return GT_BAD_PTR;
    }

    *boardTypePtr = bc2BoardType;

    return GT_OK;
}

CPSS_DXCH_PORT_MAP_STC  *appDemoPortMapPtr = NULL;
/* number of elements in appDemoPortMapPtr */
GT_U32                  appDemoPortMapSize = 0;

APPDEMO_BC2_SERDES_POLARITY_STC *appDemoPolarityArrayPtr = NULL;
GT_U32                           appDemoPolarityArraySize = 0;

/* indication that during initialization the call to appDemoBc2PortListInit(...)
   was already done so no need to call it for second time. */
GT_U32  appDemo_PortsInitList_already_done = 0;

/*
   when system is initialized with non default ports mapping,
   TM Ports Init is called after custom ports mapping is applied
*/
static GT_BOOL  isCustomPortsMapping = GT_FALSE;

static CPSS_DXCH_TM_GLUE_DRAM_CFG_STC bc2DramCfgArr[] =
{
    { /* DB board */
        5,  /* activeInterfaceNum */
        0x0,/* activeInterfaceMask - calculated from activeInterfaceNum */
        {   /*   csBitmask, mirrorEn, dqsSwapEn, ckSwapEn */
            {{   0x1,       GT_TRUE,  GT_FALSE,  GT_FALSE},
             {   0x1,       GT_TRUE,  GT_FALSE,  GT_FALSE},
             {   0x2,       GT_FALSE, GT_FALSE,  GT_FALSE},
             {   0x2,       GT_FALSE, GT_FALSE,  GT_FALSE}},
            CPSS_DRAM_SPEED_BIN_DDR3_1866M_E, /* speedBin */
            CPSS_DRAM_BUS_WIDTH_16_E,         /* busWidth */
            CPSS_DRAM_512MB_E,                /* memorySize */
            CPSS_DRAM_FREQ_667_MHZ_E,         /* memoryFreq */
            0,                                /* casWL */
            0,                                /* casL */
            CPSS_DRAM_TEMPERATURE_HIGH_E      /* interfaceTemp */
        }
    },
    { /* RD board */
        5,  /* activeInterfaceNum */
        0x0,/* activeInterfaceMask - calculated from activeInterfaceNum */
        {   /*   csBitmask, mirrorEn, dqsSwapEn, ckSwapEn */
            {{   0x1,       GT_FALSE,  GT_FALSE,  GT_FALSE},
             {   0x1,       GT_FALSE,  GT_FALSE,  GT_FALSE},
             {   0x2,       GT_FALSE, GT_FALSE,  GT_FALSE},
             {   0x2,       GT_FALSE, GT_FALSE,  GT_FALSE}},
            CPSS_DRAM_SPEED_BIN_DDR3_1866M_E, /* speedBin */
            CPSS_DRAM_BUS_WIDTH_16_E,         /* busWidth */
            CPSS_DRAM_512MB_E,                /* memorySize */
            CPSS_DRAM_FREQ_667_MHZ_E,         /* memoryFreq */
            0,                                /* casWL */
            0,                                /* casL */
            CPSS_DRAM_TEMPERATURE_HIGH_E      /* interfaceTemp */
        }
    },   /*mtl board */
    {
        4,  /* activeInterfaceNum */
        0x0,/* activeInterfaceMask - calculated from activeInterfaceNum */
        {   /*   csBitmask, mirrorEn, dqsSwapEn, ckSwapEn */
            {{   0x1,       GT_FALSE,  GT_FALSE,  GT_FALSE},
             {   0x1,       GT_FALSE,  GT_FALSE,  GT_FALSE},
             {   0x2,       GT_FALSE, GT_FALSE,  GT_FALSE},
             {   0x2,       GT_FALSE, GT_FALSE,  GT_FALSE}},
            CPSS_DRAM_SPEED_BIN_DDR3_1866M_E, /* speedBin */
            CPSS_DRAM_BUS_WIDTH_16_E,         /* busWidth */
            CPSS_DRAM_512MB_E,                /* memorySize */
            CPSS_DRAM_FREQ_667_MHZ_E,         /* memoryFreq */
            0,                                /* casWL */
            0,                                /* casL */
            CPSS_DRAM_TEMPERATURE_HIGH_E      /* interfaceTemp */
        }
    }
};
static GT_U8 bc2DramCfgArrSize = sizeof(bc2DramCfgArr) / sizeof(bc2DramCfgArr[0]);


#define BOBK_DRAM_CFG_IDX_CAELUM_CNS 0
#define BOBK_DRAM_CFG_IDX_CETUS_CNS  1

/* Dram Cfg by board types from bobkBoardTypeGet */
static CPSS_DXCH_TM_GLUE_DRAM_CFG_STC bobkDramCfgArr[] =
{
    {   /* APP_DEMO_CAELUM_BOARD_DB_CNS */
        3,   /* activeInterfaceNum */
        0x0B,/* activeInterfaceMask - DDR interfaces 0,1,3 are availble for TM */
        {   /*   csBitmask, mirrorEn, dqsSwapEn, ckSwapEn */
            {{   0x2,       GT_FALSE, GT_FALSE,  GT_FALSE},
             {   0x2,       GT_FALSE, GT_FALSE,  GT_FALSE},
             {   0x1,       GT_TRUE,  GT_FALSE,  GT_FALSE},
             {   0x1,       GT_TRUE,  GT_FALSE,  GT_FALSE}},
            CPSS_DRAM_SPEED_BIN_DDR3_1866M_E, /* speedBin */
            CPSS_DRAM_BUS_WIDTH_16_E,         /* busWidth */
            CPSS_DRAM_512MB_E,                /* memorySize */
            CPSS_DRAM_FREQ_667_MHZ_E,         /* memoryFreq */
            0,                                /* casWL */
            0,                                /* casL */
            CPSS_DRAM_TEMPERATURE_HIGH_E      /* interfaceTemp */
        }
    },
    {   /* APP_DEMO_CETUS_BOARD_DB_CNS */
        1,  /* activeInterfaceNum */
        0x0,/* activeInterfaceMask - calculated from activeInterfaceNum */
        {   /*   csBitmask, mirrorEn, dqsSwapEn, ckSwapEn */
            {{   0x1,       GT_FALSE,  GT_FALSE,  GT_FALSE},
             {   0x1,       GT_FALSE,  GT_FALSE,  GT_FALSE},
             {   0x2,       GT_TRUE,   GT_FALSE,  GT_FALSE},
             {   0x2,       GT_TRUE,   GT_FALSE,  GT_FALSE}},
            CPSS_DRAM_SPEED_BIN_DDR3_1866M_E, /* speedBin */
            CPSS_DRAM_BUS_WIDTH_16_E,         /* busWidth */
            CPSS_DRAM_512MB_E,                /* memorySize */
            CPSS_DRAM_FREQ_667_MHZ_E,         /* memoryFreq */
            0,                                /* casWL */
            0,                                /* casL */
            CPSS_DRAM_TEMPERATURE_HIGH_E      /* interfaceTemp */
        }
    }
};

static GT_U8 bobkDramCfgArrSize = sizeof(bobkDramCfgArr) / sizeof(bobkDramCfgArr);

GT_STATUS setDRAMIntNumber(GT_U32 intNum)
{
    GT_U8 i;

    for (i = 0; i < bobkDramCfgArrSize; i++)
    {
        bobkDramCfgArr[i].activeInterfaceNum = intNum;
    }

    for (i = 0; i < bc2DramCfgArrSize; i++)
    {
        bc2DramCfgArr[i].activeInterfaceNum = intNum;
    }

    osPrintf("numOfLads has change to: 0x%0x\n", intNum);

    return 0;
}

#ifdef LINUX_NOKM
static CPSS_HW_DRIVER_STC *i2cDrv;
#endif /* LINUX_NOKM */

/*tm scenario mode*/

CPSS_TM_SCENARIO appDemoTmScenarioMode = 0;

extern GT_STATUS appDemoIsTmSupported(GT_U8 devNum,GT_BOOL *isTmSupported); /* HW */
extern GT_BOOL appDemoIsTmEnabled(void);     /* SW */

/**
* @internal getBoardInfo function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] isB2bSystem              - GT_TRUE, the system is a B2B system.
* @param[out] numOfPp                  - Number of Packet processors in system.
* @param[out] numOfFa                  - Number of Fabric Adaptors in system.
* @param[out] numOfXbar                - Number of Crossbar devices in system.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getBoardInfo
(
    IN  GT_U8   boardRevId,
    OUT GT_U8   *numOfPp,
    OUT GT_U8   *numOfFa,
    OUT GT_U8   *numOfXbar,
    OUT GT_BOOL *isB2bSystem
)
{
    GT_U8       i;                      /* Loop index.                  */
    GT_STATUS   pciStat;

    pciStat = appDemoSysGetPciInfo();
    ppCounter = 0;
    if(pciStat == GT_OK)
    {
        for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
        {
            if(appDemoPpConfigList[i].valid == GT_TRUE)
            {
                /* PIPE shouldnt be include to PP counter */
                 switch (appDemoPpConfigList[i].devFamily)
                 {
                 case CPSS_PX_FAMILY_PIPE_E:
                     break;
                 default:
                     ppCounter++;
                     break;
                 }
            }
        }
    }


    /* No PCI devices found */
    if(ppCounter == 0)
    {
        return GT_NOT_FOUND;
    }

    *numOfPp    = ppCounter;
    *numOfFa    = 0;
    *numOfXbar  = 0;

    *isB2bSystem = GT_FALSE;/* no B2B with Bobcat2 devices */

    return GT_OK;
}

/**
* @internal getBoardInfoSimple function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] numOfPp                  - Number of Packet processors in system.
*                                      ppPhase1Params  - Phase1 parameters struct.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of getBoardInfo.
*
*/
static GT_STATUS getBoardInfoSimple
(
    IN  GT_U8   boardRevId,
    OUT GT_U8   *numOfPp,
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC  *phase1Params,
    OUT GT_PCI_INFO *pciInfo
)
{
    GT_STATUS   rc;

    /* In our case we want to find just one prestera device on PCI bus*/
    rc = gtPresteraGetPciDev(GT_TRUE, pciInfo);
    if(rc != GT_OK)
    {
        osPrintf("Could not find Prestera device on PCI bus!\n");
        return GT_NOT_FOUND;
    }

    ppCounter++;

    rc = extDrvPexConfigure(
            pciInfo->pciBusNum,
            pciInfo->pciIdSel,
            pciInfo->funcNo,
            MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E,
            &(phase1Params->hwInfo[0]));

    rc = gtPresteraGetPciDev(GT_FALSE, pciInfo);
    if(rc == GT_OK)
    {
        osPrintf("More then one Prestera device found on PCI bus!\n");
        return GT_INIT_ERROR;
    }

    *numOfPp    = 1;

    return GT_OK;
}

/*******************************************************************************
* getPpPhase1Config
*
* DESCRIPTION:
*       Returns the configuration parameters for cpssDxChHwPpPhase1Init().
*
* INPUTS:
*       boardRevId      - The board revision Id.
*       devIdx          - The Pp Index to get the parameters for.
*
* OUTPUTS:
*       ppPhase1Params  - Phase1 parameters struct.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
#if 0
CPSS_HW_DRIVER_STC *cpssHwDriverPexMbusCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  GT_U32      compRegionMask
);
#endif
static GT_STATUS getPpPhase1Config
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devIdx,
    OUT CPSS_PP_PHASE1_INIT_PARAMS  *phase1Params
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PP_PHASE1_INIT_PARAMS    localPpPh1Config = CPSS_PP_PHASE1_DEFAULT;
    GT_U32 coreClockValue;
    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);

    localPpPh1Config.devNum = devIdx;

    rc = extDrvPexConfigure(
            appDemoPpConfigList[devIdx].pciInfo.pciBusNum,
            appDemoPpConfigList[devIdx].pciInfo.pciIdSel,
            appDemoPpConfigList[devIdx].pciInfo.funcNo,
            MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E,
            &(localPpPh1Config.hwInfo[0]));
    if (rc != GT_OK)
    {
        osPrintf("extDrvPexConfigure() failed, rc=%d\n", rc);
        return rc;
    }
#if 0
    localPpPh1Config.hwInfo[0].driver = cpssHwDriverPexMbusCreateDrv(
        &(localPpPh1Config.hwInfo[0]), 0x3e);
#endif

    localPpPh1Config.deviceId =
        ((appDemoPpConfigList[devIdx].pciInfo.pciDevVendorId.devId << 16) |
         (appDemoPpConfigList[devIdx].pciInfo.pciDevVendorId.vendorId));

    localPpPh1Config.mngInterfaceType = CPSS_CHANNEL_PEX_E;

    /* retrieve PP Core Clock from HW */
    if(GT_OK == appDemoDbEntryGet("coreClock", &coreClockValue))
    {
        localPpPh1Config.coreClk = coreClockValue;
    }
    else
    {
        localPpPh1Config.coreClk = APP_DEMO_CPSS_AUTO_DETECT_CORE_CLOCK_CNS;
#if !defined(ASIC_SIMULATION)
    if (localPpPh1Config.hwInfo[0].busType != CPSS_HW_INFO_BUS_TYPE_MBUS_E)
    {
        GT_U32  regData;
        GT_BOOL doByteSwap;

        switch (localPpPh1Config.deviceId)
        {
            case CPSS_BOBK_ALL_DEVICES_CASES_MAC:

                #if defined(CPU_LE)
                    doByteSwap = GT_FALSE;
                #else
                    doByteSwap = GT_TRUE;
                #endif

                /* read Device Sample at Reset (SAR) Status<1> register to get PLL 0 Config field */
                rc = cpssDxChDiagRegRead(
                    localPpPh1Config.hwInfo[0].resource.resetAndInitController.start,
                    localPpPh1Config.mngInterfaceType,
                    CPSS_DIAG_PP_REG_PCI_CFG_E, /* use access without address competion for DFX registers */
                    0x000F8204,
                    &regData,
                    doByteSwap);

                if( GT_OK != rc )
                {
                    return rc;
                }

                if (U32_GET_FIELD_MAC(regData, 21, 3) == 7)
                {
                    /* Bobk devices with PLL bypass setting must use predefined coreClock.
                       Use maximal possible coreClock in the case */
                    localPpPh1Config.coreClk = 365;
                }
                break;
            default:
                break;
        }
    }
#endif
    }

    /* current Bobcat2 box has 156.25MHz differencial serdesRefClock */
    localPpPh1Config.serdesRefClock =  CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E;

    localPpPh1Config.uplinkCfg = CPSS_PP_NO_UPLINK_E;

    appDemoPpConfigList[devIdx].flowControlDisable = GT_TRUE;

    osMemCpy(phase1Params,&localPpPh1Config,sizeof(CPSS_PP_PHASE1_INIT_PARAMS));

    return rc;
}



static GT_STATUS getPpPhase1ConfigClear
(
    IN  GT_U8                       devIdx,
    IN  GT_U8                       boardRevId
)
{
#ifndef ASIC_SIMULATION
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);

    rc = extDrvPexRemove(
            appDemoPpConfigList[devIdx].pciInfo.pciBusNum,
            appDemoPpConfigList[devIdx].pciInfo.pciIdSel,
            appDemoPpConfigList[devIdx].pciInfo.funcNo);
    if (rc != GT_OK)
    {
        osPrintf("extDrvPexRemove() failed, rc=%d\n", rc);
    }
    return rc;
#else
    return GT_OK;
#endif
}


/**
* @internal getPpPhase1ConfigSimple function
* @endinternal
*
* @brief   Returns the configuration parameters for cpssDxChHwPpPhase1Init().
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devIdx                   - The Pp Index to get the parameters for.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of configBoardAfterPhase1.
*
*/
static GT_STATUS getPpPhase1ConfigSimple
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devIdx,
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC  *phase1Params
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);

    phase1Params->devNum = devIdx;

/* retrieve PP Core Clock from HW */
    phase1Params->coreClock             = CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS;

    phase1Params->mngInterfaceType      = CPSS_CHANNEL_PEX_E;
    phase1Params->ppHAState             = CPSS_SYS_HA_MODE_ACTIVE_E;
    /* current Bobcat2 box has 156.25MHz differencial serdesRefClock */
    phase1Params->serdesRefClock        = CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E;
    /* 8 Address Completion Region mode                        */
    phase1Params->mngInterfaceType      = CPSS_CHANNEL_PEX_MBUS_E;
    /* Address Completion Region 1 - for Interrupt Handling    */
    phase1Params->isrAddrCompletionRegionsBmp = 0x02;
    /* Address Completion Regions 2,3,4,5 - for Application    */
    phase1Params->appAddrCompletionRegionsBmp = 0x3C;
    /* Address Completion Regions 6,7 - reserved for other CPU */

    phase1Params->numOfPortGroups       = 0; /* irrelevant for BC2*/

    return rc;
}

extern GT_STATUS gtAppDemoTmGetTmPortsIds
(
    IN      GT_U8  dev,
    INOUT   GT_U32 *tmPortsArrSizePtr,
    OUT     GT_U32 *tmPortsArr
);

#if defined (INCLUDE_TM)
/**
* @internal prvBC2TmGlueQueueMapping function
* @endinternal
*
* @brief   Bobcat2 TM glue Queue Mapper configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvBC2TmGlueQueueMapping
(
    IN  GT_U8  devNum
)
{
    GT_STATUS                                           rc = GT_OK; /* Return code */
    GT_U32                                              bitIndex;   /* TM queue ID bit */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC    entry;      /* Table entry */
    GT_U32                                              entryIndex; /* Table entry index */
    GT_U32                                              tmPortsArr[64]; /* map of initialized TM Ports */
    GT_U32                                              tmPortsArrSize = 64; /* actual size of array */
    GT_U32                                              queueIdBase;  /* first queue id of egress physical port */

    queueIdBase = entry.queueIdBase = 1;

    rc = gtAppDemoTmGetTmPortsIds(devNum, &tmPortsArrSize, tmPortsArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Qmapper - the bit select table - all queues selected by <TM port, TC>
       entryIndex is target physical port */
    for(entryIndex = 0; entryIndex < 255; entryIndex++)
    {
        if (appDemoTmScenarioMode == CPSS_TM_2 || appDemoTmScenarioMode == CPSS_TM_3)
        {
            for (bitIndex = 0; bitIndex < 14; bitIndex++)
            {
                if(bitIndex < 3)
                {
                    /* Bits[0...2] - TM TC select type */
                    entry.bitSelectArr[bitIndex].selectType =
                            CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TM_TC_E;
                    entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
                }
                else if(bitIndex < 14)
                {
                    /* Bits[3...10] - local target PHY port select type */
                    entry.bitSelectArr[bitIndex].selectType =
                            CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_EPORT_E;
                    entry.bitSelectArr[bitIndex].bitSelector = bitIndex - 3;
                }
            }
        }
        else
        {
            GT_U32 usePhyPortAsIndex = 0; /* 0 - don't use physical port as index and set
                                                 queueIdBase to be first queue of the port
                                             1 - use physical port as index, queueIdBase is 1   */
            GT_U32 portExistInMap = 0;
            GT_U32 ii;

            /* check port's existence in tmPortsArr map */
            for (ii = 0; ii < tmPortsArrSize; ii++)
            {
                if (entryIndex == tmPortsArr[ii])
                {
                    portExistInMap = 1;
                    break;
                }
            }

            if (entryIndex < tmPortsArrSize)
            {
                /* check one to one mapping of port */
                if (entryIndex == tmPortsArr[entryIndex])
                {
                    usePhyPortAsIndex = 1;
                }
            }

            if (usePhyPortAsIndex || (0 == portExistInMap))
            {
                /* use base queue 1 for non existing port in map and
                   linear dependency port (ports 0..51).*/
                entry.queueIdBase = 1;
            }
            else
            {
                /* use first queue of a port */
                entry.queueIdBase = queueIdBase;
            }

            if (portExistInMap)
            {
                /* each port use 8 sequential queues.
                   Update variable to be first queue of next existing port  */
                queueIdBase += 8;
            }
            else
            {
                /* use queue map with physical port number */
                usePhyPortAsIndex = 1;
            }

            for (bitIndex = 0; bitIndex < 14; bitIndex++)
            {
                if(bitIndex < 3)
                {
                    /* Bits[0...2] - TM TC select type */
                    entry.bitSelectArr[bitIndex].selectType =
                        CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TM_TC_E;
                    entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
                }
                else if(usePhyPortAsIndex && bitIndex < 11)
                {
                    /* Bits[3...10] - local target PHY port select type */
                    entry.bitSelectArr[bitIndex].selectType =
                        CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_LOCAL_TARGET_PHY_PORT_E;
                    entry.bitSelectArr[bitIndex].bitSelector = bitIndex - 3;
                }
                else
                {
                    /* Bits[11...13] - zero select type */
                    entry.bitSelectArr[bitIndex].selectType =
                        CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ZERO_E;
                    entry.bitSelectArr[bitIndex].bitSelector = 0;
                }
            }
        }

        /* Call CPSS API function */
        rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, entryIndex, &entry);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGlueQueueMapBitSelectTableEntrySet", rc);
        if(rc != GT_OK)
        {
                return rc;
        }
    }

    return rc;
}

/**
* @internal prvBobkTmGlueQueueMapping function
* @endinternal
*
* @brief   Bobk TM glue Queue Mapper configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] totalQueusPerPort        - total tm queues per port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvBobkTmGlueQueueMapping
(
    IN  GT_U8  devNum,
    IN  GT_U32 totalQueusPerPort
)
{
    GT_STATUS                                           rc = GT_OK;        /* Return code */
    GT_U32                                              bitIndex;          /* TM queue ID bit */
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC    entry;             /* Table entry */
    GT_U32                                              entryIndex;        /* Table entry index */
    GT_U32                                              startQueue = 1;
    GT_U32                                              tmPortsArr[64];
    GT_U32                                              i,tmPortsArrSize = 64;
    entry.queueIdBase = 1;

    /* Qmapper - the bit select table - all queues selected by <TM port, TC> */
    if (appDemoTmScenarioMode == CPSS_TM_2 || appDemoTmScenarioMode == CPSS_TM_3)
    {
        for(entryIndex = 0; entryIndex < 255; entryIndex++)
        {
            for (bitIndex = 0; bitIndex < 14; bitIndex++)
            {
                if(bitIndex < 3)
                {
                    /* Bits[0...2] - TM TC select type */
                    entry.bitSelectArr[bitIndex].selectType =
                        CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TM_TC_E;
                    entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
                }
                else if(bitIndex < 14)
                {
                    /* Bits[3...10] - local target PHY port select type */
                    entry.bitSelectArr[bitIndex].selectType =
                        CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_EPORT_E;
                    entry.bitSelectArr[bitIndex].bitSelector = bitIndex - 3;
                }
            }

            /* Call CPSS API function */
            rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, entryIndex, &entry);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGlueQueueMapBitSelectTableEntrySet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        rc = gtAppDemoTmGetTmPortsIds(devNum, &tmPortsArrSize, tmPortsArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        for(i = 0; i < tmPortsArrSize; i++)
        {
            entryIndex = tmPortsArr[i];

            /* assuming 8 queues per each port and first queue is 1 */
            entry.queueIdBase = startQueue;
            startQueue += totalQueusPerPort;

            for (bitIndex = 0; bitIndex < 14; bitIndex++)
            {
                if(bitIndex < 3)
                {
                    /* Bits[0...2] - TM TC select type */
                    entry.bitSelectArr[bitIndex].selectType =
                        CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TM_TC_E;
                    entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
                }
                else
                {
                    /* Bits[11...13] - zero select type */
                    entry.bitSelectArr[bitIndex].selectType =
                            CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ZERO_E;
                    entry.bitSelectArr[bitIndex].bitSelector = 0;
                }
            }

            /* Call CPSS API function */
            rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, entryIndex, &entry);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGlueQueueMapBitSelectTableEntrySet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return rc;
}


/**
* @internal prvBobcat2TmGlueConfig function
* @endinternal
*
* @brief   Bobcat2 TM glue configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvBobcat2TmGlueConfig
(
    IN  GT_U8  devNum
)
{

        GT_STATUS                       rc = GT_OK;        /* Return code */
        CPSS_DXCH_TM_GLUE_DROP_MASK_STC dropMaskCfg;/* Drop mask configuration */
        GT_U32                          i, j, tmPorts;
        GT_U32                          hwDevNum;
        CPSS_INTERFACE_INFO_STC         physicalInfoPtr;
        CPSS_DXCH_PORT_MAP_STC          *portsMappingPtr;
        GT_U32                          portsMappingLen;
        GT_U32                          totalEportsPerPort;
        GT_U32                          totalQueuesPerPort;



        /*
           totalQueuesPerPort based on nummber of queues per port from appDemoTmScenario2Init, appDemoTmScenario3Init
           totalQueuesPerPort = cNodesNum * bNodesNum * aNodesNum * queuesNum
           total eports per port = totalQueuesPerPort / 8 TCs

           for BC2:
             cNodesNum = 4, bNodesNum = 5, aNodesNum = 10, queuesNum = 8
             totalQueuesPerPort = 1600
             total eports = 1600 / 8 = 200

           for BobK:
             cNodesNum = 2, bNodesNum = 4, aNodesNum = 5, queuesNum = 8
             totalQueuesPerPort = 384
             total eports = 320/8 = 40



        if(bc2BoardType == APP_DEMO_CETUS_BOARD_DB_CNS)
        */

        totalQueuesPerPort = appDemoTmGetTotalQueuesPerPort();

        if (isBobkBoard)
        {
            rc = prvBobkTmGlueQueueMapping(devNum, totalQueuesPerPort);
        }
            else
        {
            rc = prvBC2TmGlueQueueMapping(devNum);
        }

        /* Drop profile 0 - consider for drop P level Tail Drop only */
        osMemSet(&dropMaskCfg, 0, sizeof(dropMaskCfg));
        /* Set port tail drop recomenadtion for drop desision */
        dropMaskCfg.portTailDropUnmask = GT_TRUE;
        if (appDemoTmScenarioMode==CPSS_TM_2)
        {
            dropMaskCfg.qTailDropUnmask = GT_TRUE;
            /* Call CPSS API function */
            for (i = 0; i < 16; i++) {
                rc = cpssDxChTmGlueDropProfileDropMaskSet(devNum, 0, i, &dropMaskCfg);
            }
        }
        if (appDemoTmScenarioMode == CPSS_TM_3)
        {
            dropMaskCfg.qTailDropUnmask = GT_TRUE;
            dropMaskCfg.qWredDropUnmask = GT_TRUE;

            /* Call CPSS API function */
            for (i = 0; i < 16; i++)
            {
                rc = cpssDxChTmGlueDropProfileDropMaskSet(devNum, 0, i, &dropMaskCfg);
            }
        }
        else if (appDemoTmScenarioMode == CPSS_TM_48_PORT)
        {
            dropMaskCfg.redPacketsDropMode = CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ASK_TM_RESP_E;
            /* Call CPSS API function */
            for (i = 0; i < 16; i++)
            {
                rc = cpssDxChTmGlueDropProfileDropMaskSet(devNum, 0, i, &dropMaskCfg);
            }
        }
        else
        {
            rc = cpssDxChTmGlueDropProfileDropMaskSet(devNum, 0, 0, &dropMaskCfg);
        }

        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGlueDropProfileDropMaskSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Enable Egress Flow Control for TM to response physical ports Back-Pressure
           Global Enable ingress Priority Flow Control for caelum
        */
        rc = cpssDxChTmGlueFlowControlEnableSet(devNum, GT_TRUE, GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGlueFlowControlEgressEnableSet", rc);
        if (rc != GT_OK)
        {
            return GT_FAIL;
        }

        /* Enable TM Ports to response physical ports BP */
        rc = cpssTmSchedPortExternalBpSet(devNum, GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssTmSchedPortExternalBpSet", rc);
        if (rc != GT_OK)
        {
            return GT_FAIL;
        }

        hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

        /*configure eport - for each port 200 eports*/
        if (appDemoTmScenarioMode == CPSS_TM_2 || appDemoTmScenarioMode == CPSS_TM_3)
        {
            rc = appDemoDxChMaxMappedPortGet(devNum, &portsMappingLen, &portsMappingPtr);

            if (rc != GT_OK)
            {
                return GT_FAIL;
            }

            for (i = 0, tmPorts = 0; i < portsMappingLen && tmPorts < 10 ; i++)
            {
                if (portsMappingPtr[i].mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E) /* Skip CPU Port and ILKN ports */
                {
                    continue;
                }
                tmPorts++;

                physicalInfoPtr.devPort.hwDevNum= hwDevNum;
                physicalInfoPtr.devPort.portNum = portsMappingPtr[i].interfaceNum;
                physicalInfoPtr.type = CPSS_INTERFACE_PORT_E;
                totalEportsPerPort = totalQueuesPerPort / 8;
                for (j = 0; j < totalEportsPerPort; j++)
                {
                    rc= cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum , (i*totalEportsPerPort) + j, &physicalInfoPtr);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    rc= cpssDxChBrgGenMtuPortProfileIdxSet(devNum, (i*totalEportsPerPort) + j, 0);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }

    return rc;
}

#endif /*#if defined (INCLUDE_TM)*/

/**
* @internal appDemoBc2PtpTaiCheck function
* @endinternal
*
* @brief   PTP TOD dump for all TAI instances.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoBc2PtpTaiCheck
(
    IN  GT_U8  devNum
)
{
    CPSS_DXCH_PTP_TAI_ID_STC        taiId;      /* TAI Units identification */
    CPSS_DXCH_PTP_TOD_COUNT_STC     tod;        /* TOD Value */
    GT_STATUS                       rc;         /* return code */
    GT_U32                          iter, i, ii;
    CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT  captureId;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK_BOBCAT2_BOBK
        return GT_OK;
    }

    /* Configure Capture function in all TAIs */
    taiId.taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E;
    taiId.taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
    rc = cpssDxChPtpTodCounterFunctionSet(devNum, CPSS_PORT_DIRECTION_BOTH_E /* ignored*/,
                                          &taiId, CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear all captured TODs */
    for (iter = 0; iter < 2; iter++)
    {
        captureId = CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E + iter;

        /* loop on GOPs */
        for ( i = 0 ; i < 10 ; i++ )
        {
                /* GOP0 port0  - port15                               */
                /* GOP1 port16 - port31                               */
                /* GOP2 port32 - port47                               */
                /* GOP3 port48 - port51                               */
                /* GOP4 port52 - port55                               */
                /* GOP5 port56 - port59                               */
                /* GOP6 port60 - port63                               */
                /* GOP7 port64 - port67                               */
                /* GOP8 port68 - port71                               */

            if (i == 9)
            {
                 taiId.taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_GLOBAL_E;
            }
            else
            {
                taiId.taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_PORT_E;
                taiId.portNum = 0xFFFFFF; /* not legal value */
                if (i <= 3)
                {
                    taiId.portNum = i * 16;
                }
                else
                {
                    switch(i)
                    {
                        case 4: taiId.portNum = 52; break;
                        case 5: taiId.portNum = 56; break;
                        case 6: taiId.portNum = 80; break;
                        case 7: taiId.portNum = 64; break;
                        case 8: taiId.portNum = 68; break;
                        default: cpssOsPrintf(" ERROR\n"); return GT_FAIL;
                    }
                }
            }
            /* loop on TAIs */
            for ( ii = 0 ; ii < 2 ; ii++ )
            {
                taiId.taiNumber = ii;

                /* call device specific API */
                rc = cpssDxChPtpTaiTodGet(devNum, &taiId, captureId, &tod);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }


    /* trigger all TAIs simultaneously */
    rc = cpssDxChPtpTaiTodCounterFunctionAllTriggerSet(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    for (iter = 0; iter < 2; iter++)
    {
        cpssOsPrintf("TOD iteration %d\n", iter);
        captureId = CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E;

        /* loop on GOPs */
        for ( i = 0 ; i < 10 ; i++ )
        {
                /* GOP0 port0  - port15                               */
                /* GOP1 port16 - port31                               */
                /* GOP2 port32 - port47                               */
                /* GOP3 port48 - port51                               */
                /* GOP4 port52 - port55                               */
                /* GOP5 port56 - port59                               */
                /* GOP6 port60 - port63                               */
                /* GOP7 port64 - port67                               */
                /* GOP8 port68 - port71                               */

            if (i == 9)
            {
                 taiId.taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_GLOBAL_E;
            }
            else
            {
                taiId.taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_PORT_E;
                taiId.portNum = 0xFFFFFF; /* not legal value */
                if (i <= 3)
                {
                    taiId.portNum = i * 16;
                }
                else
                {
                    switch(i)
                    {
                        case 4: taiId.portNum = 52; break;
                        case 5: taiId.portNum = 56; break;
                        case 6: taiId.portNum = 80; break;
                        case 7: taiId.portNum = 64; break;
                        case 8: taiId.portNum = 68; break;
                        default: cpssOsPrintf(" ERROR\n"); return GT_FAIL;
                    }
                }
            }
            /* loop on TAIs */
            for ( ii = 0 ; ii < 2 ; ii++ )
            {
                taiId.taiNumber = ii;

                /* call device specific API */
                rc = cpssDxChPtpTaiTodGet(devNum, &taiId, captureId, &tod);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("cpssDxChPtpTaiTodGet failure Instance %d TAI %d rc %d\n", i, ii, rc);
                    return rc;
                }

                cpssOsPrintf("TOD %d %d seconds %d nanoseconds %d\n", i, ii, tod.seconds.l[0], tod.nanoSeconds);
            }
        }

        if (iter == 0)
        {
            cpssOsTimerWkAfter(1000);

            /* trigger all TAIs simultaneously */
            rc = cpssDxChPtpTaiTodCounterFunctionAllTriggerSet(devNum);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal appDemoB2PtpConfig function
* @endinternal
*
* @brief   Bobcat2 PTP and TAIs related configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoB2PtpConfig
(
    IN  GT_U8  devNum
)
{
    CPSS_DXCH_PTP_TAI_ID_STC        taiId;      /* TAI Units identification */
    CPSS_DXCH_PTP_TAI_TOD_STEP_STC  todStep;    /* TOD Step */
    GT_U32                          regAddr;    /* register address */
    GT_U32                          regData = 0;/* register value*/
    GT_U32                          pll4Frq = 1;/* PLL4 frequency (in khz) */
    GT_STATUS                       rc;         /* return code */
    GT_PHYSICAL_PORT_NUM            portNum;    /* port number */
    CPSS_DXCH_PTP_TSU_CONTROL_STC   control;    /* control structure */
    GT_U32                          ptpClkInKhz; /* PTP clock in KHz */

    if (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
    {
        /* Aldrin and Bobcat3 have 500 MHz PTP clock */
        ptpClkInKhz = 500000;
    }
    else
    {
        /* Get TAIs reference clock (its half of PLL4 frequency) */
        if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) != GT_FALSE)
        {
            regAddr =
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                        DFXServerUnitsDeviceSpecificRegs.deviceSAR2;
            rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,
                                                             regAddr, 12, 2,
                                                             &regData);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
        {
            switch(regData)
            {
                case 0: pll4Frq = PLL4_FREQUENCY_1250000_KHZ_CNS;
                        break;
                case 1: pll4Frq = PLL4_FREQUENCY_1093750_KHZ_CNS;
                        break;
                case 2: pll4Frq = PLL4_FREQUENCY_1550000_KHZ_CNS;
                        break;
                default: return GT_BAD_STATE;
            }
            ptpClkInKhz = pll4Frq / 2;
        }
        else
        {
            switch(regData)
            {
                case 0: ptpClkInKhz = 500000; /* 500 MHz */
                        break;
                case 1: ptpClkInKhz = 546875; /* 546.875MHz*/
                        break;
                default: return GT_BAD_STATE;
            }
        }
    }

    /* Configure TAIs nanosec step values */
    taiId.taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E;
    taiId.taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
    todStep.nanoSeconds = 1000000/ptpClkInKhz;
    todStep.fracNanoSeconds = (GT_U32)(0xFFFFFFFF *
                  (1000000.0/ptpClkInKhz - todStep.nanoSeconds) +
                  (1000000.0/ptpClkInKhz - todStep.nanoSeconds));

    rc = cpssDxChPtpTaiTodStepSet(devNum, &taiId, &todStep);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPtpTaiTodStepSet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(portNum = 0; portNum < (appDemoPpConfigList[devNum].maxPortNumber); portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPtpTsuControlGet(devNum, portNum, &control);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(control.unitEnable == GT_FALSE)
        {
            control.unitEnable = GT_TRUE;
            /* Timestamping unit enable */
            rc = cpssDxChPtpTsuControlSet(devNum, portNum, &control);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal bobcat2BoardTypePrint function
* @endinternal
*
* @brief   This function prints type of Bobcat2 board.
*
* @param[in] bc2BoardType             - board type.
*                                      0 - DB board
*                                      1 - RD MSI board
*                                      2 - RD MTL board
*                                       none
*/
static GT_VOID bobcat2BoardTypePrint
(
    IN GT_U8    dev,
    IN  GT_U32  bc2BoardType
)
{
    typedef struct
    {
        GT_U32 boardType;
        GT_CHAR *boardNamePtr;
    }APPDEMO_BOARDTYPE_2_STR_STC;

    static APPDEMO_BOARDTYPE_2_STR_STC boardNameLIst[] =
    {
         {  APP_DEMO_BC2_BOARD_DB_CNS,              "DB"         }
        ,{  APP_DEMO_BC2_BOARD_RD_MSI_CNS,          "RD MSI"     }
        ,{  APP_DEMO_BC2_BOARD_RD_MTL_CNS,          "RD MTL"     }
        ,{  APP_DEMO_CAELUM_BOARD_DB_CNS,           "CAELUM-DB"  }
        ,{  APP_DEMO_CETUS_BOARD_DB_CNS,            "DB"         }
        ,{  APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS,      "LEWIS-RD"   }
        ,{  APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS,    "CYGNUS-RD"  }
        ,{  APP_DEMO_ALDRIN_BOARD_DB_CNS,           "ALDRIN-DB"  }
        ,{  APP_DEMO_BOBCAT3_BOARD_DB_CNS,          "BOBCAT3-DB" }
        ,{  APP_DEMO_ALDRIN_BOARD_RD_CNS,           "ALDRIN-RD"  }
        ,{  APP_DEMO_BOBCAT3_BOARD_RD_CNS,          "BOBCAT3-RD" }
        ,{  APP_DEMO_ARMSTRONG_BOARD_RD_CNS,        "ARMSTRONG-RD"}
        ,{  APP_DEMO_ALDRIN2_BOARD_DB_CNS,          "ALDRIN2-DB"}
        ,{  APP_DEMO_ALDRIN2_BOARD_RD_CNS,          "ALDRIN2-RD"}
        ,{  (GT_U32)(~0)                   ,  NULL         }
    };

    typedef struct
    {
        GT_U32 boardType;
        GT_CHAR *boardNamePtr;
        GT_CHAR *deviceNamePtr;
    }ENHANCED_APPDEMO_BOARDTYPE_2_STR_STC;

    static ENHANCED_APPDEMO_BOARDTYPE_2_STR_STC explicit_boardNameLIst[] =
    {
         {  APP_DEMO_XCAT3X_A0_BOARD_DB_CNS , "DB"   ,"AC3X"}
        ,{  APP_DEMO_XCAT3X_Z0_BOARD_DB_CNS , "DB"   ,"AC3X-Z0"}

        ,{  (GT_U32)(~0)                   ,  NULL         ,NULL}
    };
    /* GT_CHAR *boardNameArr[4] = {"DB", "RD MSI", "RD MTL","Bad Type"}; */
    GT_CHAR  *boardName;
    GT_CHAR *devName;
    GT_CHAR *environment;
    GT_U32 i;

    boardName = NULL;
    devName = NULL;
    for (i = 0 ; explicit_boardNameLIst[i].boardNamePtr != NULL; i++)
    {
        if (explicit_boardNameLIst[i].boardType == bc2BoardType)
        {
            boardName = explicit_boardNameLIst[i].boardNamePtr;
            devName = explicit_boardNameLIst[i].deviceNamePtr;
            break;
        }
    }

    if(boardName == NULL)
    {
        switch(appDemoPpConfigList[dev].deviceId)
        {
            case CPSS_BOBK_CAELUM_DEVICES_CASES_MAC:
                devName = "BobK-Caelum";
                if (APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS == bc2BoardType)
                {
                    boardName = boardNameLIst[6].boardNamePtr;
                }
                else
                {
                    boardName = boardNameLIst[3].boardNamePtr;
                }

                if(appDemoPpConfigList[dev].deviceId == CPSS_98DX8332_Z0_CNS)
                {
                    devName = "Aldrin-Z0";
                    boardName = boardNameLIst[7].boardNamePtr;
                }

                break;
            case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                devName = "BobK-Cetus";
                if (APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS == bc2BoardType)
                    boardName = boardNameLIst[5].boardNamePtr;
                else
                    boardName = boardNameLIst[4].boardNamePtr;
                break;
            case CPSS_ALDRIN_DEVICES_CASES_MAC:
                devName = "Aldrin";
                if (APP_DEMO_ALDRIN_BOARD_DB_CNS == bc2BoardType)
                {
                    boardName = boardNameLIst[7].boardNamePtr;
                }
                else
                {
                    boardName = boardNameLIst[9].boardNamePtr;
                }
                break;
            case CPSS_BOBCAT3_ALL_DEVICES_CASES_MAC:
                devName = "Bobcat3";
                if (APP_DEMO_BOBCAT3_BOARD_DB_CNS == bc2BoardType)
                {
                    boardName = boardNameLIst[8].boardNamePtr;
                }
                else
                {
                    boardName = boardNameLIst[10].boardNamePtr;
                }
                break;
            case CPSS_ARMSTRONG_ALL_DEVICES_CASES_MAC:
                devName = "Armstrong";
                boardName = "Armstrong-DB";
                break;
            case CPSS_ALDRIN2_ALL_DEVICES_CASES_MAC:
                devName = "Aldrin2";
                if (APP_DEMO_ALDRIN2_BOARD_DB_CNS == bc2BoardType)
                {
                    boardName = boardNameLIst[12].boardNamePtr;
                }
                else
                {
                    boardName = boardNameLIst[13].boardNamePtr;
                }
                break;
            default:
            {
                devName = "Bobcat2";
                boardName = (GT_CHAR *)NULL;
                for (i = 0 ; boardNameLIst[i].boardNamePtr != NULL; i++)
                {
                    if (boardNameLIst[i].boardType == bc2BoardType)
                    {
                        boardName = boardNameLIst[i].boardNamePtr;
                        break;
                    }
                }
                if (boardName == NULL)
                {
                    boardName = "Bad Type";
                }
            }
        }
    }

#ifdef GM_USED
    environment = "GM (Golden Model) - simulation";
#elif defined ASIC_SIMULATION
    if(cpssDeviceRunCheck_onEmulator())
    {
        environment = "WM (White Model) - simulation (special EMULATOR Mode)";
    }
    else
    {
        environment = "WM (White Model) - simulation";
    }
#else
    if(cpssDeviceRunCheck_onEmulator())
    {
        environment = "HW (on EMULATOR)";
    }
    else
    {
        environment = "HW (Hardware)";
    }
#endif

    osPrintf("%s Board Type: %s [%s]\n", devName , boardName , environment);
}

#ifdef LINUX_NOKM
extern GT_STATUS extDrvGetDmaSize(OUT GT_U32 * dmaSize);
#endif

/**
* @internal waInit function
* @endinternal
*
* @brief   init the WA needed after phase1.
*
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS waInit
(
    IN  GT_U8   boardRevId
)
{
    GT_STATUS   rc;
    static CPSS_DXCH_IMPLEMENT_WA_ENT   waFromCpss[CPSS_DXCH_IMPLEMENT_WA_LAST_E];
    GT_U32  enableAddrFilterWa; /* do we want to enable Restricted Address
                                   Filtering or not */
    GT_U32  waIndex=0;
    GT_U8   devNum;
    GT_U32  dbEntryValue;
    GT_BOOL enable30GWa = GT_FALSE;
    GT_U32  ii;

    boardRevId = boardRevId;

    if(GT_OK != appDemoDbEntryGet("RESTRICTED_ADDRESS_FILTERING_WA_E", &enableAddrFilterWa))
    {
        enableAddrFilterWa = 0;
    }

    if(GT_OK == appDemoDbEntryGet("BC2_RevA0_40G_2_30GWA", &dbEntryValue))
    {
        enable30GWa = (GT_BOOL)dbEntryValue;
    }


    for(ii = SYSTEM_DEV_NUM_MAC(0); ii < SYSTEM_DEV_NUM_MAC(ppCounter); ++ii)
    {
        waIndex = 0;
        devNum = appDemoPpConfigList[ii].devNum;

        /* state that application want CPSS to implement the WA */
        if(GT_FALSE != enable30GWa)
        {/* if WA enable bit 16 in 0x600006C set on by default */
            waFromCpss[waIndex++] = CPSS_DXCH_IMPLEMENT_WA_BOBCAT2_REV_A0_40G_NOT_THROUGH_TM_IS_PA_30G_E;
        }

#ifdef LINUX_NOKM
        {
            GT_U32  dmaSize = 0;

            extDrvGetDmaSize(&dmaSize);
            if (dmaSize < _512K)
            {
                /* state that application want CPSS to implement the Register Read
                   of the AU Fifo instead of using the AU DMA WA */
                waFromCpss[waIndex++] = CPSS_DXCH_IMPLEMENT_WA_FDB_AU_FIFO_E;
            }
        }
#endif

        if((GT_TRUE == PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(devNum))||
           (GT_TRUE == PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum)))
        {
            if (enableAddrFilterWa == 1)
            {
                /* state that application want CPSS to implement the Restricted
                   Address Filtering WA */
                waFromCpss[waIndex++] =
                    CPSS_DXCH_IMPLEMENT_WA_RESTRICTED_ADDRESS_FILTERING_E;
            }
        }

        if(waIndex)
        {
            rc = cpssDxChHwPpImplementWaInit(devNum,waIndex,&waFromCpss[0], NULL);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/*******************************************************************************
* configBoardAfterPhase1 : relevant data structures for port mapping
*
*       This function performs all needed configurations that should be done
*******************************************************************************/

    /* bc2 mapping for scenario 8 */
static CPSS_DXCH_PORT_MAP_STC bc2TmEnableE8Map[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort,        tmEnable,      tmPortInd*/
      {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        0,           0,       GT_NA,          GT_TRUE,         0 }
     ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        0,           1,         1,            GT_FALSE,        0 }
     ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        0,           2,         2,            GT_FALSE,        0 }
     ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        0,           3,         3,            GT_FALSE,        0 }
     ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        0,           4,         4,            GT_FALSE,        0 }
     ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        0,           5,         5,            GT_FALSE,        0 }
     ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        0,           6,         6,            GT_FALSE,        0 }
     ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        0,           7,         7,            GT_FALSE,        0 }
     ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        0,           8,         8,            GT_FALSE,        0 }
     ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        0,           9,         9,            GT_FALSE,        0 }
     ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,            0,         GT_NA,      63,            GT_FALSE,        0 }
};

    /* bc2 mapping for scenarios 2 and 3 */
static CPSS_DXCH_PORT_MAP_STC bc2TmEnableE2Map[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort,        tmEnable,      tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,           0,       GT_NA,          GT_TRUE,         0 }
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,           1,       GT_NA,          GT_TRUE,         1 }
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,           2,       GT_NA,          GT_TRUE,         2 }
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,           3,       GT_NA,          GT_TRUE,         3 }
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,           4,       GT_NA,          GT_TRUE,         4 }
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,           5,       GT_NA,          GT_TRUE,         5 }
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,           6,       GT_NA,          GT_TRUE,         6 }
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,           7,       GT_NA,          GT_TRUE,         7 }
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,           8,       GT_NA,          GT_TRUE,         8 }
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,           9,       GT_NA,          GT_TRUE,         9 }
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,             0,         GT_NA,      63,            GT_FALSE,        0 }
};


    /* bobk mapping for scenario 8*/
static CPSS_DXCH_PORT_MAP_STC bobkTmEnableE8Map[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort,        tmEnable,      tmPortInd*/
     {  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          56,       GT_NA,          GT_TRUE,        56 }
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          57,        57,            GT_FALSE,        0 }
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          58,        58,            GT_FALSE,        0 }
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          59,        59,            GT_FALSE,        0 }
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,             0,         GT_NA,      63,            GT_FALSE,        0 }
                                                                                /* txQ 64 is reserved for tm */
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          64,        56,            GT_FALSE,        0 }
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          65,        65,            GT_FALSE,        0 }
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          66,        66,            GT_FALSE,        0 }
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          67,        67,            GT_FALSE,        0 }
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          68,        68,            GT_FALSE,        0 }
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          69,        69,            GT_FALSE,        0 }
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          70,        70,            GT_FALSE,        0 }
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          71,        71,            GT_FALSE,        0 }
};


    /* bobk mapping for scenarios 2-5 */
static CPSS_DXCH_PORT_MAP_STC bobkTmEnableE2Map[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort,        tmEnable,      tmPortInd*/
     {  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          56,       GT_NA,          GT_TRUE,         56 }
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          57,       GT_NA,          GT_TRUE,         57 }
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          58,       GT_NA,          GT_TRUE,         58 }
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          59,       GT_NA,          GT_TRUE,         59 }
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,             0,         GT_NA,      63,            GT_FALSE,        0  }
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          64,       GT_NA,          GT_TRUE,         64 }
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          65,       GT_NA,          GT_TRUE,         65 }
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          66,       GT_NA,          GT_TRUE,         66 }
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          67,       GT_NA,          GT_TRUE,         67 }
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          68,       GT_NA,          GT_TRUE,         68 }
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          69,       GT_NA,          GT_TRUE,         69 }
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          70,       GT_NA,          GT_TRUE,         70 }
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          71,       GT_NA,          GT_TRUE,         71 }
};


/* ports 56..59 , 64..71 */
static CPSS_DXCH_PORT_MAP_STC bobkCetusTmEnableMap[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort,         tmEnable,     tmPortInd*/
     {  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          56,       GT_NA,           GT_TRUE,        56}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          57,       GT_NA,           GT_TRUE,        57}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          58,       GT_NA,           GT_TRUE,        58}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          59,       GT_NA,           GT_TRUE,        59}
    ,{  90, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          62,        62,             GT_FALSE,       GT_NA}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,             0,         GT_NA,      63,             GT_FALSE,       0}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          64,       GT_NA,           GT_TRUE,        64}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          65,       GT_NA,           GT_TRUE,        65}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          66,       GT_NA,           GT_TRUE,        66}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          67,       GT_NA,           GT_TRUE,        67}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          68,       GT_NA,           GT_TRUE,        68}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          69,       GT_NA,           GT_TRUE,        69}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          70,       GT_NA,           GT_TRUE,        70}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,          71,       GT_NA,           GT_TRUE,        71}
};

static CPSS_DXCH_PORT_MAP_STC bobkCaelumTmEnable2412[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        0,    GT_NA,          GT_TRUE,                0}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        1,    GT_NA,          GT_TRUE,                1}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        2,    GT_NA,          GT_TRUE,                2}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        3,    GT_NA,          GT_TRUE,                3}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        4,    GT_NA,          GT_TRUE,                4}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        5,    GT_NA,          GT_TRUE,                5}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        6,    GT_NA,          GT_TRUE,                6}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        7,    GT_NA,          GT_TRUE,                7}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        8,    GT_NA,          GT_TRUE,                8}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        9,    GT_NA,          GT_TRUE,                9}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       10,    GT_NA,          GT_TRUE,               10}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       11,    GT_NA,          GT_TRUE,               11}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       12,    GT_NA,          GT_TRUE,               12}
    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       13,    GT_NA,          GT_TRUE,               13}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       14,    GT_NA,          GT_TRUE,               14}
    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       15,    GT_NA,          GT_TRUE,               15}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       16,    GT_NA,          GT_TRUE,               16}
    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       17,    GT_NA,          GT_TRUE,               17}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       18,    GT_NA,          GT_TRUE,               18}
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       19,    GT_NA,          GT_TRUE,               19}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       20,    GT_NA,          GT_TRUE,               20}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       21,    GT_NA,          GT_TRUE,               21}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       22,    GT_NA,          GT_TRUE,               22}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       23,    GT_NA,          GT_TRUE,               23}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       56,    GT_NA,          GT_TRUE,               56}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       57,    GT_NA,          GT_TRUE,               57}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       58,    GT_NA,          GT_TRUE,               58}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       59,    GT_NA,          GT_TRUE,               59}
    ,{  90, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       62,    62,             GT_FALSE,              GT_NA}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,             0,      GT_NA,  63,             GT_FALSE,              0}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       64,    GT_NA,          GT_TRUE,               64}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       65,    GT_NA,          GT_TRUE,               65}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       66,    GT_NA,          GT_TRUE,               66}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       67,    GT_NA,          GT_TRUE,               67}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       68,    GT_NA,          GT_TRUE,               68}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       69,    GT_NA,          GT_TRUE,               69}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       70,    GT_NA,          GT_TRUE,               70}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       71,    GT_NA,          GT_TRUE,               71}
};

static CPSS_DXCH_PORT_MAP_STC bobkCaelumTmEnable4812[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        0,    GT_NA,          GT_TRUE,                0}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        1,    GT_NA,          GT_TRUE,                1}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        2,    GT_NA,          GT_TRUE,                2}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        3,    GT_NA,          GT_TRUE,                3}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        4,    GT_NA,          GT_TRUE,                4}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        5,    GT_NA,          GT_TRUE,                5}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        6,    GT_NA,          GT_TRUE,                6}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        7,    GT_NA,          GT_TRUE,                7}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        8,    GT_NA,          GT_TRUE,                8}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,        9,    GT_NA,          GT_TRUE,                9}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       10,    GT_NA,          GT_TRUE,               10}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       11,    GT_NA,          GT_TRUE,               11}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       12,    GT_NA,          GT_TRUE,               12}
    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       13,    GT_NA,          GT_TRUE,               13}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       14,    GT_NA,          GT_TRUE,               14}
    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       15,    GT_NA,          GT_TRUE,               15}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       16,    GT_NA,          GT_TRUE,               16}
    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       17,    GT_NA,          GT_TRUE,               17}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       18,    GT_NA,          GT_TRUE,               18}
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       19,    GT_NA,          GT_TRUE,               19}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       20,    GT_NA,          GT_TRUE,               20}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       21,    GT_NA,          GT_TRUE,               21}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       22,    GT_NA,          GT_TRUE,               22}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       23,    GT_NA,          GT_TRUE,               23}
    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       24,    GT_NA,          GT_TRUE,               24}
    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       25,    GT_NA,          GT_TRUE,               25}
    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       26,    GT_NA,          GT_TRUE,               26}
    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       27,    GT_NA,          GT_TRUE,               27}
    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       28,    GT_NA,          GT_TRUE,               28}
    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       29,    GT_NA,          GT_TRUE,               29}
    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       30,    GT_NA,          GT_TRUE,               30}
    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       31,    GT_NA,          GT_TRUE,               31}
    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       32,    GT_NA,          GT_TRUE,               32}
    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       33,    GT_NA,          GT_TRUE,               33}
    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       34,    GT_NA,          GT_TRUE,               34}
    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       35,    GT_NA,          GT_TRUE,               35}
    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       36,    GT_NA,          GT_TRUE,               36}
    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       37,    GT_NA,          GT_TRUE,               37}
    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       38,    GT_NA,          GT_TRUE,               38}
    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       39,    GT_NA,          GT_TRUE,               39}
    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       40,    GT_NA,          GT_TRUE,               40}
    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       41,    GT_NA,          GT_TRUE,               41}
    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       42,    GT_NA,          GT_TRUE,               42}
    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       43,    GT_NA,          GT_TRUE,               43}
    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       44,    GT_NA,          GT_TRUE,               44}
    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       45,    GT_NA,          GT_TRUE,               45}
    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       46,    GT_NA,          GT_TRUE,               46}
    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       47,    GT_NA,          GT_TRUE,               47}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       56,    GT_NA,          GT_TRUE,               56}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       57,    GT_NA,          GT_TRUE,               57}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       58,    GT_NA,          GT_TRUE,               58}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       59,    GT_NA,          GT_TRUE,               59}
    ,{  90, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       62,    62,             GT_FALSE,              GT_NA}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,             0,      GT_NA,   63,            GT_FALSE,              0}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       64,    GT_NA,          GT_TRUE,               64}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       65,    GT_NA,          GT_TRUE,               65}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       66,    GT_NA,          GT_TRUE,               66}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       67,    GT_NA,          GT_TRUE,               67}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       68,    GT_NA,          GT_TRUE,               68}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       69,    GT_NA,          GT_TRUE,               69}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       70,    GT_NA,          GT_TRUE,               70}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0,       71,    GT_NA,          GT_TRUE,               71}
};

static CPSS_DXCH_PORT_MAP_STC bc2defaultMap_4220[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,              0}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,              0}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,              0}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,              0}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,              0}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,              0}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,              0}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,              0}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,              0}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,              0}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,              0}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,              0}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,              0}
    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,              0}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,              0}
    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,              0}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,              0}
    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,              0}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,              0}
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,              0}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,              0}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,              0}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,              0}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,              0}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,              0}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,              0}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,              0}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,              0}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,              0}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,              0}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,              0}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,              0}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,              0}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,              0}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,              0}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,              0}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,              0}
};

static CPSS_DXCH_PORT_MAP_STC bc2defaultMap_4221[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA}
    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA}
    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA}
    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA}
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,          GT_NA}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,          GT_NA}
    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,          GT_NA}
    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,          GT_NA}
    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,          GT_NA}
    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,          GT_NA}
    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,          GT_NA}
    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,      29,           GT_FALSE,          GT_NA}
    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,      30,           GT_FALSE,          GT_NA}
    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,      31,           GT_FALSE,          GT_NA}
    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA}
    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       33,      33,           GT_FALSE,          GT_NA}
    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       34,      34,           GT_FALSE,          GT_NA}
    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       35,      35,           GT_FALSE,          GT_NA}
    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       36,      36,           GT_FALSE,          GT_NA}
    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       37,      37,           GT_FALSE,          GT_NA}
    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       38,      38,           GT_FALSE,          GT_NA}
    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       39,      39,           GT_FALSE,          GT_NA}
    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       40,      40,           GT_FALSE,          GT_NA}
    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       41,      41,           GT_FALSE,          GT_NA}
    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       42,      42,           GT_FALSE,          GT_NA}
    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       43,      43,           GT_FALSE,          GT_NA}
    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       44,      44,           GT_FALSE,          GT_NA}
    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       45,      45,           GT_FALSE,          GT_NA}
    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       46,      46,           GT_FALSE,          GT_NA}
    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       47,      47,           GT_FALSE,          GT_NA}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,          GT_NA}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,          GT_NA}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,          GT_NA}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,          GT_NA}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,          GT_NA}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,          GT_NA}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,          GT_NA}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,          GT_NA}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,          GT_NA}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,          GT_NA}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,          GT_NA}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,          GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC caelumDefaultMap_4204[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,              0}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,              0}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,              0}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,              0}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,              0}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,              0}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,              0}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,              0}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,              0}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,              0}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,              0}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,              0}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,              0}
    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,              0}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,              0}
    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,              0}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,              0}
    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,              0}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,              0}
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,              0}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,              0}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,              0}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,              0}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,              0}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,              0}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,              0}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,              0}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,              0}
    ,{  90, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       62,      62,           GT_FALSE,              0}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,              0}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,              0}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,              0}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,              0}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,              0}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,              0}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,              0}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,              0}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,              0}
};

static CPSS_DXCH_PORT_MAP_STC bc2defaultMap_8216[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       52,      52,           GT_FALSE,              0}
    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       53,      53,           GT_FALSE,              0}
    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       54,      54,           GT_FALSE,              0}
    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       55,      55,           GT_FALSE,              0}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,              0}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,              0}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,              0}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,              0}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,              0}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,              0}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,              0}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,              0}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,              0}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,              0}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,              0}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,              0}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,              0}
};

static CPSS_DXCH_PORT_MAP_STC caelumDefaultMap_4212[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,              0}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,              0}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,              0}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,              0}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,              0}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,              0}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,              0}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,              0}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,              0}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,              0}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,              0}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,              0}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,              0}
    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,              0}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,              0}
    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,              0}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,              0}
    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,              0}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,              0}
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,              0}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,              0}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,              0}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,              0}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,              0}
    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,              0}
    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,              0}
    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,              0}
    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,              0}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,              0}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,              0}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,              0}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,              0}
    ,{  90, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       62,      62,           GT_FALSE,              0}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,              0}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,              0}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,              0}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,              0}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,              0}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,              0}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,              0}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,              0}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,              0}
};

static CPSS_DXCH_PORT_MAP_STC bc2defaultMap_8219[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,              0}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,              0}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,              0}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,              0}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,              0}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,              0}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,              0}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,              0}
    ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       48,      48,           GT_FALSE,              0}
    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       49,      49,           GT_FALSE,              0}
    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       50,      50,           GT_FALSE,              0}
    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       51,      51,           GT_FALSE,              0}
    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       52,      52,           GT_FALSE,              0}
    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       53,      53,           GT_FALSE,              0}
    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       54,      54,           GT_FALSE,              0}
    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       55,      55,           GT_FALSE,              0}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,              0}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,              0}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,              0}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,              0}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,              0}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,              0}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,              0}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,              0}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,              0}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,              0}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,              0}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,              0}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,              0}
};

static CPSS_DXCH_PORT_MAP_STC bc2defaultMap_8224[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       48,      48,           GT_FALSE,              0}
    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       49,      49,           GT_FALSE,              0}
    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       50,      50,           GT_FALSE,              0}
    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       51,      51,           GT_FALSE,              0}
    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       52,      52,           GT_FALSE,              0}
    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       53,      53,           GT_FALSE,              0}
    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       54,      54,           GT_FALSE,              0}
    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       55,      55,           GT_FALSE,              0}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,              0}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,              0}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,              0}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,              0}
    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       60,      60,           GT_FALSE,              0}
    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       61,      61,           GT_FALSE,              0}
    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       62,      62,           GT_FALSE,              0}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,              0}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,              0}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,              0}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,              0}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,              0}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,              0}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,              0}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,              0}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,              0}
};

static CPSS_DXCH_PORT_MAP_STC caelumDefaultMap[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA}
    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA}
    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA}
    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA}
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,          GT_NA}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,          GT_NA}
    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,          GT_NA}
    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,          GT_NA}
    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,          GT_NA}
    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,          GT_NA}
    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,          GT_NA}
    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,      29,           GT_FALSE,          GT_NA}
    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,      30,           GT_FALSE,          GT_NA}
    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,      31,           GT_FALSE,          GT_NA}
    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA}
    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       33,      33,           GT_FALSE,          GT_NA}
    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       34,      34,           GT_FALSE,          GT_NA}
    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       35,      35,           GT_FALSE,          GT_NA}
    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       36,      36,           GT_FALSE,          GT_NA}
    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       37,      37,           GT_FALSE,          GT_NA}
    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       38,      38,           GT_FALSE,          GT_NA}
    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       39,      39,           GT_FALSE,          GT_NA}
    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       40,      40,           GT_FALSE,          GT_NA}
    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       41,      41,           GT_FALSE,          GT_NA}
    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       42,      42,           GT_FALSE,          GT_NA}
    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       43,      43,           GT_FALSE,          GT_NA}
    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       44,      44,           GT_FALSE,          GT_NA}
    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       45,      45,           GT_FALSE,          GT_NA}
    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       46,      46,           GT_FALSE,          GT_NA}
    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       47,      47,           GT_FALSE,          GT_NA}
    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,          GT_NA}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,          GT_NA}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,          GT_NA}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,          GT_NA}
    ,{  90, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       62,      62,           GT_FALSE,          GT_NA}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,          GT_NA}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,          GT_NA}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,          GT_NA}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,          GT_NA}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,          GT_NA}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,          GT_NA}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,          GT_NA}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,          GT_NA}
};


static CPSS_DXCH_PORT_MAP_STC bc2defaultMap_4222[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,           GT_NA}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,           GT_NA}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,           GT_NA}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,           GT_NA}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,           GT_NA}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,           GT_NA}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,           GT_NA}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,           GT_NA}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,           GT_NA}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,           GT_NA}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,           GT_NA}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,           GT_NA}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,           GT_NA}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,           GT_NA}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,           GT_NA}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,           GT_NA}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,           GT_NA}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,           GT_NA}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,           GT_NA}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,           GT_NA}
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,           GT_NA}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,           GT_NA}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,           GT_NA}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,           GT_NA}
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,           GT_NA}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,           GT_NA}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,           GT_NA}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,           GT_NA}
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,           GT_NA}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,      29,           GT_FALSE,           GT_NA}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,      30,           GT_FALSE,           GT_NA}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,      31,           GT_FALSE,           GT_NA}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,           GT_NA}
   ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       33,      33,           GT_FALSE,           GT_NA}
   ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       34,      34,           GT_FALSE,           GT_NA}
   ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       35,      35,           GT_FALSE,           GT_NA}
   ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       36,      36,           GT_FALSE,           GT_NA}
   ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       37,      37,           GT_FALSE,           GT_NA}
   ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       38,      38,           GT_FALSE,           GT_NA}
   ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       39,      39,           GT_FALSE,           GT_NA}
   ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       40,      40,           GT_FALSE,           GT_NA}
   ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       41,      41,           GT_FALSE,           GT_NA}
   ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       42,      42,           GT_FALSE,           GT_NA}
   ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       43,      43,           GT_FALSE,           GT_NA}
   ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       44,      44,           GT_FALSE,           GT_NA}
   ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       45,      45,           GT_FALSE,           GT_NA}
   ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       46,      46,           GT_FALSE,           GT_NA}
   ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       47,      47,           GT_FALSE,           GT_NA}
   ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       60,      60,           GT_FALSE,           GT_NA}
   ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       61,      61,           GT_FALSE,           GT_NA} /* virtual router port */
   ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       62,      62,           GT_FALSE,           GT_NA}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,           GT_NA}
   ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,           GT_NA}
   ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,           GT_NA}
   ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,           GT_NA}
   ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,           GT_NA}
   ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,           GT_NA}
   ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,           GT_NA}
   ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,           GT_NA}
   ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,           GT_NA}
};
/* ports 56..59 , 62, 64..71 */
static CPSS_DXCH_PORT_MAP_STC cetusDefaultMap_56to59_62_64to71[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,          GT_NA}
    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,          GT_NA}
    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,          GT_NA}
    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,          GT_NA}
    ,{  90, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       62,      62,           GT_FALSE,          GT_NA}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,          GT_NA}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,          GT_NA}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,          GT_NA}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,          GT_NA}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,          GT_NA}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,          GT_NA}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,          GT_NA}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,          GT_NA}
};

/* ports 64..71 */
static CPSS_DXCH_PORT_MAP_STC bc2defaultMap_64to71[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {  90, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       62,      62,           GT_FALSE,          GT_NA}
    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,          GT_NA}
    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,          GT_NA}
    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,          GT_NA}
    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,          GT_NA}
    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,          GT_NA}
    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,          GT_NA}
    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,          GT_NA}
    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,          GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC bc2defaultMap_4253[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,           GT_NA}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       33,      33,           GT_FALSE,           GT_NA}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       34,      34,           GT_FALSE,           GT_NA}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       35,      35,           GT_FALSE,           GT_NA}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       36,      36,           GT_FALSE,           GT_NA}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       37,      37,           GT_FALSE,           GT_NA}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       38,      38,           GT_FALSE,           GT_NA}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       39,      39,           GT_FALSE,           GT_NA}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       40,      40,           GT_FALSE,           GT_NA}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       41,      41,           GT_FALSE,           GT_NA}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       42,      42,           GT_FALSE,           GT_NA}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       43,      43,           GT_FALSE,           GT_NA}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       44,      44,           GT_FALSE,           GT_NA}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       45,      45,           GT_FALSE,           GT_NA}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       46,      46,           GT_FALSE,           GT_NA}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       47,      47,           GT_FALSE,           GT_NA}
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       52,      52,           GT_FALSE,           GT_NA}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       53,      53,           GT_FALSE,           GT_NA}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       54,      54,           GT_FALSE,           GT_NA}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       55,      55,           GT_FALSE,           GT_NA}
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,           GT_NA}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,           GT_NA}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,           GT_NA}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,           GT_NA}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,           GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC aldrinZ0DefaultMap_8332_Z0[] =
{ /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,      64,           GT_FALSE,          GT_NA}
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,      65,           GT_FALSE,          GT_NA}
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,      66,           GT_FALSE,          GT_NA}
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,      67,           GT_FALSE,          GT_NA}
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,      68,           GT_FALSE,          GT_NA}
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,      69,           GT_FALSE,          GT_NA}
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,      70,           GT_FALSE,          GT_NA}
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,      71,           GT_FALSE,          GT_NA}
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,      56,           GT_FALSE,          GT_NA}
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,      57,           GT_FALSE,          GT_NA}
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,      58,           GT_FALSE,          GT_NA}
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,      59,           GT_FALSE,          GT_NA}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       44,      44,           GT_FALSE,          GT_NA}
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       40,      40,           GT_FALSE,          GT_NA}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       36,      36,           GT_FALSE,          GT_NA}
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA}
    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,          GT_NA}
    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,          GT_NA}
    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA}
    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA}
    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA}
    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA}
    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA}
    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA}
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       62,      62,           GT_FALSE,          GT_NA}
};

/*number of DQ ports in each of the 6 DQ units in TXQ */
#define BC3_NUM_PORTS_PER_DQ_IN_TXQ_CNS     SIP_5_20_DQ_NUM_PORTS_CNS
/*macro to convert local port and data path index to TXQ port */
#define BC3_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(localPort , dpIndex) \
    (localPort) + ((dpIndex) * BC3_NUM_PORTS_PER_DQ_IN_TXQ_CNS)
/* build TXQ_port from global mac port */
#define BC3_TXQ_PORT(globalMacPort)    BC3_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC((globalMacPort)%12,(globalMacPort)/12)

/* build TXQ_port for 'cpu port' */
#define BC3_TXQ_CPU_PORT    95 /* 12 */      /* BC3_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(12,0) */

#define BC3_CPU_PORT_MAC_AND_TXQ_PORT_MAC        GT_NA , BC3_TXQ_CPU_PORT

/* build pair of mac port and it's TXQ port*/
#define BC3_MAC_AND_TXQ_PORT_MAC(macPort)  \
    (macPort), BC3_TXQ_PORT(macPort)

static CPSS_DXCH_PORT_MAP_STC bc3defaultMap[] =
{ /* Port,            mappingType                           portGroupm,         intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*DP[0]*/
/* 0*/     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 0),  GT_FALSE,   GT_NA}
/* 1*/    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 1),  GT_FALSE,   GT_NA}
/* 2*/    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 2),  GT_FALSE,   GT_NA}
/* 3*/    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 3),  GT_FALSE,   GT_NA}
/* 4*/    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 4),  GT_FALSE,   GT_NA}
/* 5*/    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 5),  GT_FALSE,   GT_NA}
/* 6*/    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 6),  GT_FALSE,   GT_NA}
/* 7*/    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 7),  GT_FALSE,   GT_NA}
/* 8*/    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 8),  GT_FALSE,   GT_NA}
/* 9*/    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 9),  GT_FALSE,   GT_NA}
/*10*/    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(10),  GT_FALSE,   GT_NA}
/*11*/    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(11),  GT_FALSE,   GT_NA}
/*DP[1]*/
/* 0*/    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+ 0),  GT_FALSE,   GT_NA}
/* 1*/    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+ 1),  GT_FALSE,   GT_NA}
/* 2*/    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+ 2),  GT_FALSE,   GT_NA}
/* 3*/    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+ 3),  GT_FALSE,   GT_NA}
/* 4*/    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+ 4),  GT_FALSE,   GT_NA}
/* 5*/    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+ 5),  GT_FALSE,   GT_NA}
/* 6*/    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+ 6),  GT_FALSE,   GT_NA}
/* 7*/    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+ 7),  GT_FALSE,   GT_NA}
/* 8*/    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+ 8),  GT_FALSE,   GT_NA}
/* 9*/    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+ 9),  GT_FALSE,   GT_NA}
/*10*/    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+10),  GT_FALSE,   GT_NA}
/*11*/    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(1*12+11),  GT_FALSE,   GT_NA}
/*DP[2]*/
/* 0*/    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+ 0),  GT_FALSE,   GT_NA}
/* 1*/    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+ 1),  GT_FALSE,   GT_NA}
/* 2*/    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+ 2),  GT_FALSE,   GT_NA}
/* 3*/    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+ 3),  GT_FALSE,   GT_NA}
/* 4*/    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+ 4),  GT_FALSE,   GT_NA}
/* 5*/    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+ 5),  GT_FALSE,   GT_NA}
/* 6*/    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+ 6),  GT_FALSE,   GT_NA}
/* 7*/    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+ 7),  GT_FALSE,   GT_NA}
/* 8*/    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+ 8),  GT_FALSE,   GT_NA}
/* 9*/    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+ 9),  GT_FALSE,   GT_NA}
/*10*/    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+10),  GT_FALSE,   GT_NA}
/*11*/    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(2*12+11),  GT_FALSE,   GT_NA}
/*DP[3]*/
/* 0*/    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+ 0),  GT_FALSE,   GT_NA}
/* 1*/    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+ 1),  GT_FALSE,   GT_NA}
/* 2*/    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+ 2),  GT_FALSE,   GT_NA}
/* 3*/    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+ 3),  GT_FALSE,   GT_NA}
/* 4*/    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+ 4),  GT_FALSE,   GT_NA}
/* 5*/    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+ 5),  GT_FALSE,   GT_NA}
/* 6*/    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+ 6),  GT_FALSE,   GT_NA}
/* 7*/    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+ 7),  GT_FALSE,   GT_NA}
/* 8*/    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+ 8),  GT_FALSE,   GT_NA}
/* 9*/    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+ 9),  GT_FALSE,   GT_NA}
/*10*/    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+10),  GT_FALSE,   GT_NA}
/*11*/    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(3*12+11),  GT_FALSE,   GT_NA}
/*DP[4]*/
/* 0*/    ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+ 0),  GT_FALSE,   GT_NA}
/* 1*/    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+ 1),  GT_FALSE,   GT_NA}
/* 2*/    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+ 2),  GT_FALSE,   GT_NA}
/* 3*/    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+ 3),  GT_FALSE,   GT_NA}
/* 4*/    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+ 4),  GT_FALSE,   GT_NA}
/* 5*/    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+ 5),  GT_FALSE,   GT_NA}
/* 6*/    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+ 6),  GT_FALSE,   GT_NA}
/* 7*/    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+ 7),  GT_FALSE,   GT_NA}
/* 8*/    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+ 8),  GT_FALSE,   GT_NA}
/* 9*/    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+ 9),  GT_FALSE,   GT_NA}
/*10*/    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+10),  GT_FALSE,   GT_NA}
/*11*/    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(4*12+11),  GT_FALSE,   GT_NA}
/*DP[5]*/
/* 0*/    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+ 0),  GT_FALSE,   GT_NA}
/* 1*/    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+ 1),  GT_FALSE,   GT_NA}
/* 2*/    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+ 2),  GT_FALSE,   GT_NA}
/* 3*/    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+ 3),  GT_FALSE,   GT_NA}
/* 4*/    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+ 4),  GT_FALSE,   GT_NA}
/* 5*/    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+ 5),  GT_FALSE,   GT_NA}
/* 6*/    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+ 6),  GT_FALSE,   GT_NA}
/* 7*/    ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+ 7),  GT_FALSE,   GT_NA}
/* 8*/    ,{  73, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+ 8),  GT_FALSE,   GT_NA}
/* 9*/    ,{  74, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+ 9),  GT_FALSE,   GT_NA}
/*10*/    ,{  75, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+10),  GT_FALSE,   GT_NA}
/*11*/    ,{  76, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(5*12+11),  GT_FALSE,   GT_NA}
/********** special 2 last MAC ports ***************************/
/*DP[2]*/
/*12*/    ,{  77, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       72, BC3_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(12 , 2)    ,  GT_FALSE,   GT_NA}
/*DP[5]*/
/*12*/    ,{  78, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       73, BC3_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(12 , 5)    ,  GT_FALSE,   GT_NA}

/************************  DMAs only *******************/
/*DP[0]*/
/*12*/    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,       BC3_CPU_PORT_MAC_AND_TXQ_PORT_MAC,  GT_FALSE,   GT_NA}

/********** special 3 additional CPU SDMAs  ***************************/
/*DP[1]*/
/*12*/    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,       80,191,  GT_FALSE,   GT_NA}

/*12*/    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,       81,383,  GT_FALSE,   GT_NA}
/*DP[4]*/
/*12*/    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,       82,479,  GT_FALSE,   GT_NA}


};

/* NOTE: the GM supports only single DP units , and single TXQ-DQ due to memory issues. */
/* so use only ports of DMA 0..11 , CPU port that uses the DMA 12 */
static CPSS_DXCH_PORT_MAP_STC singlePipe_singleDp_bc3defaultMap[] =
{ /* Port,            mappingType                           portGroupm,         intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*0*/  {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 0),  GT_FALSE,   GT_NA}
/*1*/ ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 1),  GT_FALSE,   GT_NA}
/*2*/ ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 2),  GT_FALSE,   GT_NA}
/*3*/ ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 3),  GT_FALSE,   GT_NA}
/*4*/ ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 4),  GT_FALSE,   GT_NA}
/*5*/ ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 5),  GT_FALSE,   GT_NA}
/*6*/ ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 6),  GT_FALSE,   GT_NA}
/*7*/ ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 7),  GT_FALSE,   GT_NA}
/*8*/ ,{  79, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 8),  GT_FALSE,   GT_NA}
/*9*/ ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC( 9),  GT_FALSE,   GT_NA}
/*10*/,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(10),  GT_FALSE,   GT_NA}
/*11*/,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       BC3_MAC_AND_TXQ_PORT_MAC(11),  GT_FALSE,   GT_NA}
/*12*/,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,       BC3_CPU_PORT_MAC_AND_TXQ_PORT_MAC,  GT_FALSE,   GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC aldrinDefaultMap[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA}
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,          GT_NA}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,          GT_NA}
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,          GT_NA}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,          GT_NA}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,          GT_NA}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,          GT_NA}
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,          GT_NA}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,      29,           GT_FALSE,          GT_NA}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,      30,           GT_FALSE,          GT_NA}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,      31,           GT_FALSE,          GT_NA}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC aldrinMap_24_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA}
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,          GT_NA}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,          GT_NA}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
};


static CPSS_DXCH_PORT_MAP_STC aldrinMap_16_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC aldrinMap_18_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC aldrinMap_8_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC aldrinMap_12_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
};

/* on EMULATOR we have limited ports 24..32 only*/
static CPSS_DXCH_PORT_MAP_STC onEmulator_aldrinDefaultMap[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,          GT_NA}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,          GT_NA}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,          GT_NA}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,          GT_NA}
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,          GT_NA}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,      29,           GT_FALSE,          GT_NA}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,      30,           GT_FALSE,          GT_NA}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,      31,           GT_FALSE,          GT_NA}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
};
/*number of DQ ports in each of the 4 DQ units in TXQ */
#define ALDRIN2_NUM_PORTS_PER_DQ_IN_TXQ_CNS     25
/*macro to convert local port and data path index to TXQ port */
#define ALDRIN2_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(localPort , dpIndex) \
    (localPort) + ((dpIndex) * ALDRIN2_NUM_PORTS_PER_DQ_IN_TXQ_CNS)

#define ALDRIN2_DQ_INDEX_FROM_GLOBAL_PORT(globalMacPort)               \
    (((globalMacPort) < 24) ? ((globalMacPort) / 12) : /*0/1*/           \
     ((globalMacPort) < 72) ? (2 + ((globalMacPort)-24)/24) : /*2/3*/  \
     /*globalMacPort == 72*/ 0)

#define ALDRIN2_DQ_LOCAL_PORT_FROM_GLOBAL_PORT(globalMacPort)          \
    (((globalMacPort) < 24) ? ((globalMacPort) % 12) : /*0..11*/         \
     ((globalMacPort) < 72) ? ((globalMacPort) % 24) : /*0..23*/         \
     /*globalMacPort == 72*/ 24)

/* build TXQ_port from global mac port */
#define ALDRIN2_TXQ_PORT(globalMacPort)                            \
    ALDRIN2_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(                      \
        ALDRIN2_DQ_LOCAL_PORT_FROM_GLOBAL_PORT(globalMacPort),     \
        ALDRIN2_DQ_INDEX_FROM_GLOBAL_PORT(globalMacPort))

/* build TXQ_port for 'cpu port' */
#define ALDRIN2_TXQ_SDAM0_CPU_PORT    (25*3)+24/*99*/  /*(24 is for SDMA port in DQ[3]) */
#define ALDRIN2_TXQ_SDAM1_CPU_PORT    (25*1)+24/*49*/  /*(24 is for SDMA port in DQ[1])*/
#define ALDRIN2_TXQ_SDAM2_CPU_PORT    (25*2)+24/*74*/  /*(24 is for SDMA port in DQ[2])*/
#define ALDRIN2_TXQ_SDAM3_CPU_PORT    (25*0)+23/*23*/  /*(23 is for SDMA port in DQ[0])*/

#define ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC        GT_NA , ALDRIN2_TXQ_SDAM0_CPU_PORT
#define ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC        GT_NA , ALDRIN2_TXQ_SDAM1_CPU_PORT
#define ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC        GT_NA , ALDRIN2_TXQ_SDAM2_CPU_PORT
#define ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC        GT_NA , ALDRIN2_TXQ_SDAM3_CPU_PORT

/* build pair of mac port and it's TXQ port*/
#define ALDRIN2_MAC_AND_TXQ_PORT_MAC(macPort)  \
    (macPort), ALDRIN2_TXQ_PORT(macPort)

/* use same ports as 'BC3/ARMSTRONG' (but must have different TXQ-ports) */
static CPSS_DXCH_PORT_MAP_STC aldrin2defaultMap[] =
{ /* Port,            mappingType                       portGroupm,    intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*DP[0]*/
/* 0*/     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 0)         ,  GT_FALSE,   GT_NA}
/* 1*/    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 1)         ,  GT_FALSE,   GT_NA}
/* 2*/    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 2)         ,  GT_FALSE,   GT_NA}
/* 3*/    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 3)         ,  GT_FALSE,   GT_NA}
/* 4*/    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4)         ,  GT_FALSE,   GT_NA}
/* 5*/    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5)         ,  GT_FALSE,   GT_NA}
/* 6*/    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6)         ,  GT_FALSE,   GT_NA}
/* 7*/    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7)         ,  GT_FALSE,   GT_NA}
/* 8*/    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8)         ,  GT_FALSE,   GT_NA}
/* 9*/    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9)         ,  GT_FALSE,   GT_NA}
/*10*/    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10)         ,  GT_FALSE,   GT_NA}
/*11*/    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11)         ,  GT_FALSE,   GT_NA}
/*DP[1]*/
/* 0*/    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 0)/*12*/,  GT_FALSE,   GT_NA}
/* 1*/    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 1)/*13*/,  GT_FALSE,   GT_NA}
/* 2*/    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 2)/*14*/,  GT_FALSE,   GT_NA}
/* 3*/    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 3)/*15*/,  GT_FALSE,   GT_NA}
/* 4*/    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 4)/*16*/,  GT_FALSE,   GT_NA}
/* 5*/    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 5)/*17*/,  GT_FALSE,   GT_NA}
/* 6*/    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 6)/*18*/,  GT_FALSE,   GT_NA}
/* 7*/    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 7)/*19*/,  GT_FALSE,   GT_NA}
/* 8*/    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 8)/*20*/,  GT_FALSE,   GT_NA}
/* 9*/    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 9)/*21*/,  GT_FALSE,   GT_NA}
/*10*/    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+10)/*22*/,  GT_FALSE,   GT_NA}
/*11*/    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+11)/*23*/,  GT_FALSE,   GT_NA}
/*DP[2]*/
/* 0*/    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 0)/*24*/,  GT_FALSE,   GT_NA}
/* 1*/    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 1)/*25*/,  GT_FALSE,   GT_NA}
/* 2*/    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 2)/*26*/,  GT_FALSE,   GT_NA}
/* 3*/    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 3)/*27*/,  GT_FALSE,   GT_NA}
/* 4*/    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 4)/*28*/,  GT_FALSE,   GT_NA}
/* 5*/    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 5)/*29*/,  GT_FALSE,   GT_NA}
/* 6*/    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 6)/*30*/,  GT_FALSE,   GT_NA}
/* 7*/    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 7)/*31*/,  GT_FALSE,   GT_NA}
/* 8*/    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 8)/*32*/,  GT_FALSE,   GT_NA}
/* 9*/    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 9)/*33*/,  GT_FALSE,   GT_NA}
/*10*/    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+10)/*34*/,  GT_FALSE,   GT_NA}
/*11*/    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+11)/*35*/,  GT_FALSE,   GT_NA}
/*12*/    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+12)/*36*/,  GT_FALSE,   GT_NA}
/*13*/    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+13)/*37*/,  GT_FALSE,   GT_NA}
/*14*/    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+14)/*38*/,  GT_FALSE,   GT_NA}
/*15*/    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+15)/*39*/,  GT_FALSE,   GT_NA}
/*16*/    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+16)/*40*/,  GT_FALSE,   GT_NA}
/*17*/    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+17)/*41*/,  GT_FALSE,   GT_NA}
/*18*/    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+18)/*42*/,  GT_FALSE,   GT_NA}
/*19*/    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+19)/*43*/,  GT_FALSE,   GT_NA}
/*20*/    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+20)/*44*/,  GT_FALSE,   GT_NA}
/*21*/    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+21)/*45*/,  GT_FALSE,   GT_NA}
/*22*/    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+22)/*46*/,  GT_FALSE,   GT_NA}
/*23*/    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+23)/*47*/,  GT_FALSE,   GT_NA}
/*DP[3]*/
/* 0*/    ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 0)/*48*/,  GT_FALSE,   GT_NA}
/* 1*/    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 1)/*49*/,  GT_FALSE,   GT_NA}
/* 2*/    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 2)/*50*/,  GT_FALSE,   GT_NA}
/* 3*/    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 3)/*51*/,  GT_FALSE,   GT_NA}
/* 4*/    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 4)/*52*/,  GT_FALSE,   GT_NA}
/* 5*/    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 5)/*53*/,  GT_FALSE,   GT_NA}
/* 6*/    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 6)/*54*/,  GT_FALSE,   GT_NA}
/* 7*/    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 7)/*55*/,  GT_FALSE,   GT_NA}
/* 8*/    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 8)/*56*/,  GT_FALSE,   GT_NA}
/* 9*/    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 9)/*57*/,  GT_FALSE,   GT_NA}
/*10*/    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+10)/*58*/,  GT_FALSE,   GT_NA}
/*11*/    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+11)/*59*/,  GT_FALSE,   GT_NA}
/*12*/    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+12)/*60*/,  GT_FALSE,   GT_NA}
/*13*/    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+13)/*61*/,  GT_FALSE,   GT_NA}
/*14*/    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+14)/*62*/,  GT_FALSE,   GT_NA}
/*15*/    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+15)/*63*/,  GT_FALSE,   GT_NA}
/*16*/    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+16)/*64*/,  GT_FALSE,   GT_NA}
/*17*/    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+17)/*65*/,  GT_FALSE,   GT_NA}
/*18*/    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+18)/*66*/,  GT_FALSE,   GT_NA}
/*19*/    ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+19)/*67*/,  GT_FALSE,   GT_NA}
/*20*/    ,{  73, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+20)/*68*/,  GT_FALSE,   GT_NA}
/*21*/    ,{  74, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+21)/*69*/,  GT_FALSE,   GT_NA}
/*22*/    ,{  75, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+22)/*70*/,  GT_FALSE,   GT_NA}
/*23*/    ,{  76, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+23)/*71*/,  GT_FALSE,   GT_NA}
/********** special 1 last MAC port ***********************/
/*DP[0]*/
/*24*/    ,{  77, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(72)   /*72*//*TXQ-24*/ ,  GT_FALSE,   GT_NA}

/************************  DMAs only *******************/
/*DP[3]*/
/*24*/    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA}

/********** special 3 additional CPU SDMAs  ****************/
/*DP[1]*/
/*24*/    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-49*/,  GT_FALSE,   GT_NA}
/*DP[2]*/
/*24*/    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-74*/,  GT_FALSE,   GT_NA}
/*DP[0]*/
/*23*/    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-23*/,  GT_FALSE,   GT_NA}

};

/* NOTE: the GM supports only single DP units , and single TXQ-DQ due to memory issues. */
/* so use only ports of DMA 0..11 , CPU port that uses the DMA 23 */
/* use same numbers as in GM of BC3 */
static CPSS_DXCH_PORT_MAP_STC singleDp_aldrin2defaultMap[] =
{ /* Port,            mappingType                           portGroupm,         intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*0*/  {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 0),  GT_FALSE,   GT_NA}
/*1*/ ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 1),  GT_FALSE,   GT_NA}
/*2*/ ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 2),  GT_FALSE,   GT_NA}
/*3*/ ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 3),  GT_FALSE,   GT_NA}
/*4*/ ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4),  GT_FALSE,   GT_NA}
/*5*/ ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5),  GT_FALSE,   GT_NA}
/*6*/ ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6),  GT_FALSE,   GT_NA}
/*7*/ ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7),  GT_FALSE,   GT_NA}
/*8*/ ,{  79, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8),  GT_FALSE,   GT_NA}
/*9*/ ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9),  GT_FALSE,   GT_NA}
/*10*/,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10),  GT_FALSE,   GT_NA}
/*11*/,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,     0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11),  GT_FALSE,   GT_NA}
/************************  DMAs only *******************/
/*DP[0]*/
/*23*/,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,         0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC aldrin2defaultMap_48_ports[] =
{ /* Port,            mappingType                       portGroupm,    intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*DP[0]*/
/* 0*/     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 0)         ,  GT_FALSE,   GT_NA}
/* 1*/    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 1)         ,  GT_FALSE,   GT_NA}
/* 2*/    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 2)         ,  GT_FALSE,   GT_NA}
/* 3*/    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 3)         ,  GT_FALSE,   GT_NA}
/* 4*/    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4)         ,  GT_FALSE,   GT_NA}
/* 5*/    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5)         ,  GT_FALSE,   GT_NA}
/* 6*/    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6)         ,  GT_FALSE,   GT_NA}
/* 7*/    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7)         ,  GT_FALSE,   GT_NA}
/* 8*/    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8)         ,  GT_FALSE,   GT_NA}
/* 9*/    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9)         ,  GT_FALSE,   GT_NA}
/*10*/    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10)         ,  GT_FALSE,   GT_NA}
/*11*/    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11)         ,  GT_FALSE,   GT_NA}
/*DP[1]*/
/* 0*/    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 0)/*12*/,  GT_FALSE,   GT_NA}
/* 1*/    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 1)/*13*/,  GT_FALSE,   GT_NA}
/* 2*/    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 2)/*14*/,  GT_FALSE,   GT_NA}
/* 3*/    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 3)/*15*/,  GT_FALSE,   GT_NA}
/* 4*/    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 4)/*16*/,  GT_FALSE,   GT_NA}
/* 5*/    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 5)/*17*/,  GT_FALSE,   GT_NA}
/* 6*/    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 6)/*18*/,  GT_FALSE,   GT_NA}
/* 7*/    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 7)/*19*/,  GT_FALSE,   GT_NA}
/* 8*/    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 8)/*20*/,  GT_FALSE,   GT_NA}
/* 9*/    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 9)/*21*/,  GT_FALSE,   GT_NA}
/*10*/    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+10)/*22*/,  GT_FALSE,   GT_NA}
/*11*/    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+11)/*23*/,  GT_FALSE,   GT_NA}
/*DP[2]*/
/* 0*/    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 0)/*24*/,  GT_FALSE,   GT_NA}
/* 1*/    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 1)/*25*/,  GT_FALSE,   GT_NA}
/* 2*/    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 2)/*26*/,  GT_FALSE,   GT_NA}
/* 3*/    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 3)/*27*/,  GT_FALSE,   GT_NA}
/* 4*/    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 4)/*28*/,  GT_FALSE,   GT_NA}
/* 5*/    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 5)/*29*/,  GT_FALSE,   GT_NA}
/* 6*/    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 6)/*30*/,  GT_FALSE,   GT_NA}
/* 7*/    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 7)/*31*/,  GT_FALSE,   GT_NA}
/* 8*/    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 8)/*32*/,  GT_FALSE,   GT_NA}
/* 9*/    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 9)/*33*/,  GT_FALSE,   GT_NA}
/*10*/    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+10)/*34*/,  GT_FALSE,   GT_NA}
/*11*/    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+11)/*35*/,  GT_FALSE,   GT_NA}
/*DP[3]*/
/* 0*/    ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 0)/*48*/,  GT_FALSE,   GT_NA}
/* 1*/    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 1)/*49*/,  GT_FALSE,   GT_NA}
/* 2*/    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 2)/*50*/,  GT_FALSE,   GT_NA}
/* 3*/    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 3)/*51*/,  GT_FALSE,   GT_NA}
/* 4*/    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 4)/*52*/,  GT_FALSE,   GT_NA}
/* 5*/    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 5)/*53*/,  GT_FALSE,   GT_NA}
/* 6*/    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 6)/*54*/,  GT_FALSE,   GT_NA}
/* 7*/    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 7)/*55*/,  GT_FALSE,   GT_NA}
/* 8*/    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 8)/*56*/,  GT_FALSE,   GT_NA}
/* 9*/    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 9)/*57*/,  GT_FALSE,   GT_NA}
/*10*/    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+10)/*58*/,  GT_FALSE,   GT_NA}
/*11*/    ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+11)/*59*/,  GT_FALSE,   GT_NA}
/********** special 1 last MAC port ***********************/
/*DP[2]*/
/*23*/    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(47)   /*47*//*TXQ-24*/ ,  GT_FALSE,   GT_NA}
/************************  DMAs only *******************/
/*DP[3]*/
/*24*/    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA}

/********** special 3 additional CPU SDMAs  ****************/
/*DP[1]*/
/*24*/    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-49*/,  GT_FALSE,   GT_NA}
/*DP[2]*/
/*24*/    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-74*/,  GT_FALSE,   GT_NA}
/*DP[0]*/
/*23*/    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-23*/,  GT_FALSE,   GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC aldrin2defaultMap_24_ports[] =
{ /* Port,            mappingType                       portGroupm,    intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*DP[0]*/
/* 0*/     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 0)         ,  GT_FALSE,   GT_NA}
/* 1*/    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 1)         ,  GT_FALSE,   GT_NA}
/* 2*/    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 2)         ,  GT_FALSE,   GT_NA}
/* 3*/    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 3)         ,  GT_FALSE,   GT_NA}
/* 4*/    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4)         ,  GT_FALSE,   GT_NA}
/* 5*/    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5)         ,  GT_FALSE,   GT_NA}
/* 6*/    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6)         ,  GT_FALSE,   GT_NA}
/* 7*/    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7)         ,  GT_FALSE,   GT_NA}
/* 8*/    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8)         ,  GT_FALSE,   GT_NA}
/* 9*/    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9)         ,  GT_FALSE,   GT_NA}
/*10*/    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10)         ,  GT_FALSE,   GT_NA}
/*11*/    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11)         ,  GT_FALSE,   GT_NA}
/*DP[1]*/
/* 0*/    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 0)/*12*/,  GT_FALSE,   GT_NA}
/* 1*/    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 1)/*13*/,  GT_FALSE,   GT_NA}
/* 2*/    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 2)/*14*/,  GT_FALSE,   GT_NA}
/* 3*/    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 3)/*15*/,  GT_FALSE,   GT_NA}
/* 4*/    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 4)/*16*/,  GT_FALSE,   GT_NA}
/* 5*/    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 5)/*17*/,  GT_FALSE,   GT_NA}
/* 6*/    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 6)/*18*/,  GT_FALSE,   GT_NA}
/* 7*/    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 7)/*19*/,  GT_FALSE,   GT_NA}
/* 8*/    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 8)/*20*/,  GT_FALSE,   GT_NA}
/* 9*/    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 9)/*21*/,  GT_FALSE,   GT_NA}
/*10*/    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+10)/*22*/,  GT_FALSE,   GT_NA}
/*11*/    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+11)/*23*/,  GT_FALSE,   GT_NA}
/********** special 1 last MAC port ***********************/
/*DP[2]*/
/*23*/    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(47)   /*47*/,  GT_FALSE,   GT_NA}
/************************  DMAs only *******************/
/*DP[3]*/
/*24*/    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA}

/********** special 3 additional CPU SDMAs  ****************/
/*DP[1]*/
/*24*/    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-49*/,  GT_FALSE,   GT_NA}
/*DP[2]*/
/*24*/    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-74*/,  GT_FALSE,   GT_NA}
/*DP[0]*/
/*23*/    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-23*/,  GT_FALSE,   GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC aldrin2defaultMap_64_ports[] =
{ /* Port,            mappingType                       portGroupm,    intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*DP[0]*/
/* 4*/     {   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4)         ,  GT_FALSE,   GT_NA}
/* 5*/    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5)         ,  GT_FALSE,   GT_NA}
/* 6*/    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6)         ,  GT_FALSE,   GT_NA}
/* 7*/    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7)         ,  GT_FALSE,   GT_NA}
/* 8*/    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8)         ,  GT_FALSE,   GT_NA}
/* 9*/    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9)         ,  GT_FALSE,   GT_NA}
/*10*/    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10)         ,  GT_FALSE,   GT_NA}
/*11*/    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11)         ,  GT_FALSE,   GT_NA}
/*DP[1]*/
/* 4*/    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 4)/*16*/,  GT_FALSE,   GT_NA}
/* 5*/    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 5)/*17*/,  GT_FALSE,   GT_NA}
/* 6*/    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 6)/*18*/,  GT_FALSE,   GT_NA}
/* 7*/    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 7)/*19*/,  GT_FALSE,   GT_NA}
/* 8*/    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 8)/*20*/,  GT_FALSE,   GT_NA}
/* 9*/    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+ 9)/*21*/,  GT_FALSE,   GT_NA}
/*10*/    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+10)/*22*/,  GT_FALSE,   GT_NA}
/*11*/    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12+11)/*23*/,  GT_FALSE,   GT_NA}
/*DP[2]*/
/* 0*/    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 0)/*24*/,  GT_FALSE,   GT_NA}
/* 1*/    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 1)/*25*/,  GT_FALSE,   GT_NA}
/* 2*/    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 2)/*26*/,  GT_FALSE,   GT_NA}
/* 3*/    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 3)/*27*/,  GT_FALSE,   GT_NA}
/* 4*/    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 4)/*28*/,  GT_FALSE,   GT_NA}
/* 5*/    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 5)/*29*/,  GT_FALSE,   GT_NA}
/* 6*/    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 6)/*30*/,  GT_FALSE,   GT_NA}
/* 7*/    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 7)/*31*/,  GT_FALSE,   GT_NA}
/* 8*/    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 8)/*32*/,  GT_FALSE,   GT_NA}
/* 9*/    ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+ 9)/*33*/,  GT_FALSE,   GT_NA}
/*10*/    ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+10)/*34*/,  GT_FALSE,   GT_NA}
/*11*/    ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+11)/*35*/,  GT_FALSE,   GT_NA}
/*12*/    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+12)/*36*/,  GT_FALSE,   GT_NA}
/*13*/    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+13)/*37*/,  GT_FALSE,   GT_NA}
/*14*/    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+14)/*38*/,  GT_FALSE,   GT_NA}
/*15*/    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+15)/*39*/,  GT_FALSE,   GT_NA}
/*16*/    ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+16)/*40*/,  GT_FALSE,   GT_NA}
/*17*/    ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+17)/*41*/,  GT_FALSE,   GT_NA}
/*18*/    ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+18)/*42*/,  GT_FALSE,   GT_NA}
/*19*/    ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+19)/*43*/,  GT_FALSE,   GT_NA}
/*20*/    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+20)/*44*/,  GT_FALSE,   GT_NA}
/*21*/    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+21)/*45*/,  GT_FALSE,   GT_NA}
/*22*/    ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+22)/*46*/,  GT_FALSE,   GT_NA}
/*23*/    ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(24+23)/*47*/,  GT_FALSE,   GT_NA}
/*DP[3]*/
/* 0*/    ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 0)/*48*/,  GT_FALSE,   GT_NA}
/* 1*/    ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 1)/*49*/,  GT_FALSE,   GT_NA}
/* 2*/    ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 2)/*50*/,  GT_FALSE,   GT_NA}
/* 3*/    ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 3)/*51*/,  GT_FALSE,   GT_NA}
/* 4*/    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 4)/*52*/,  GT_FALSE,   GT_NA}
/* 5*/    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 5)/*53*/,  GT_FALSE,   GT_NA}
/* 6*/    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 6)/*54*/,  GT_FALSE,   GT_NA}
/* 7*/    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 7)/*55*/,  GT_FALSE,   GT_NA}
/* 8*/    ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 8)/*56*/,  GT_FALSE,   GT_NA}
/* 9*/    ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+ 9)/*57*/,  GT_FALSE,   GT_NA}
/*10*/    ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+10)/*58*/,  GT_FALSE,   GT_NA}
/*11*/    ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+11)/*59*/,  GT_FALSE,   GT_NA}
/*12*/    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+12)/*60*/,  GT_FALSE,   GT_NA}
/*13*/    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+13)/*61*/,  GT_FALSE,   GT_NA}
/*14*/    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+14)/*62*/,  GT_FALSE,   GT_NA}
/*15*/    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+15)/*63*/,  GT_FALSE,   GT_NA}
/*16*/    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+16)/*64*/,  GT_FALSE,   GT_NA}
/*17*/    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+17)/*65*/,  GT_FALSE,   GT_NA}
/*18*/    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(48+18)/*66*/,  GT_FALSE,   GT_NA}
/********** special 1 last MAC port ***********************/
/*DP[0]*/
/*24*/    ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(72)   /*72*//*TXQ-24*/ ,  GT_FALSE,   GT_NA}
/************************  DMAs only *******************/
/*DP[3]*/
/*24*/    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA}

/********** special 3 additional CPU SDMAs  ****************/
/*DP[1]*/
/*24*/    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-49*/,  GT_FALSE,   GT_NA}
/*DP[2]*/
/*24*/    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-74*/,  GT_FALSE,   GT_NA}
/*DP[0]*/
/*23*/    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-23*/,  GT_FALSE,   GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC aldrin2defaultMap_48_special_map_ports[] =
{ /* Port,            mappingType                       portGroupm,    intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*DP[0]*/
/* 0*/     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 0),  GT_FALSE,   GT_NA}
/* 1*/    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 1),  GT_FALSE,   GT_NA}
/* 2*/    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 2),  GT_FALSE,   GT_NA}
/* 3*/    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 3),  GT_FALSE,   GT_NA}
/* 4*/    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 4),  GT_FALSE,   GT_NA}
/* 5*/    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 5),  GT_FALSE,   GT_NA}
/* 6*/    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 6),  GT_FALSE,   GT_NA}
/* 7*/    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 7),  GT_FALSE,   GT_NA}
/* 8*/    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 8),  GT_FALSE,   GT_NA}
/* 9*/    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC( 9),  GT_FALSE,   GT_NA}
/*10*/    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(10),  GT_FALSE,   GT_NA}
/*11*/    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(11),  GT_FALSE,   GT_NA}
/*DP[1]*/
/* 0*/    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(12),  GT_FALSE,   GT_NA}
/* 1*/    ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(13),  GT_FALSE,   GT_NA}
/* 2*/    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(14),  GT_FALSE,   GT_NA}
/* 3*/    ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(15),  GT_FALSE,   GT_NA}
/* 4*/    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(16),  GT_FALSE,   GT_NA}
/* 5*/    ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(17),  GT_FALSE,   GT_NA}
/* 6*/    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(18),  GT_FALSE,   GT_NA}
/* 7*/    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(19),  GT_FALSE,   GT_NA}
/* 8*/    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(20),  GT_FALSE,   GT_NA}
/* 9*/    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(21),  GT_FALSE,   GT_NA}
/*10*/    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(22),  GT_FALSE,   GT_NA}
/*11*/    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(23),  GT_FALSE,   GT_NA}
/*DP[2]*/
/* 0*/    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(28),  GT_FALSE,   GT_NA}
/* 1*/    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(29),  GT_FALSE,   GT_NA}
/* 2*/    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(30),  GT_FALSE,   GT_NA}
/* 3*/    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(31),  GT_FALSE,   GT_NA}

/* 4*/    ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(36),  GT_FALSE,   GT_NA}
/* 5*/    ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(37),  GT_FALSE,   GT_NA}
/* 6*/    ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(38),  GT_FALSE,   GT_NA}
/* 7*/    ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(39),  GT_FALSE,   GT_NA}

/* 8*/    ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(44),  GT_FALSE,   GT_NA}
/* 9*/    ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(45),  GT_FALSE,   GT_NA}
/* 10*/   ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(46),  GT_FALSE,   GT_NA}
/* 11*/   ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(47),  GT_FALSE,   GT_NA}

/*DP[3]*/
/* 0*/    ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(52),  GT_FALSE,   GT_NA}
/* 1*/    ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(53),  GT_FALSE,   GT_NA}
/* 2*/    ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(54),  GT_FALSE,   GT_NA}
/* 3*/    ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(55),  GT_FALSE,   GT_NA}

/* 4*/    ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(60),  GT_FALSE,   GT_NA}
/* 5*/    ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(61),  GT_FALSE,   GT_NA}
/* 6*/    ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(62),  GT_FALSE,   GT_NA}
/* 7*/    ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(63),  GT_FALSE,   GT_NA}

/* 8*/    ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(68),  GT_FALSE,   GT_NA}
/* 9*/    ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(69),  GT_FALSE,   GT_NA}
/*10*/    ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(70),  GT_FALSE,   GT_NA}
/*11*/    ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, 0, ALDRIN2_MAC_AND_TXQ_PORT_MAC(71),  GT_FALSE,   GT_NA}

/************************  DMAs only *******************/
/*DP[3]*/
/*24*/    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG0_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-99*/,  GT_FALSE,   GT_NA}

/********** special 3 additional CPU SDMAs  ****************/
/*DP[1]*/
/*24*/    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG1_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-49*/,  GT_FALSE,   GT_NA}
/*DP[2]*/
/*24*/    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG2_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-74*/,  GT_FALSE,   GT_NA}
/*DP[0]*/
/*23*/    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     0, ALDRIN2_SDMA_MG3_CPU_PORT_MAC_AND_TXQ_PORT_MAC/*TXQ-23*/,  GT_FALSE,   GT_NA}
};

#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

/* SERDES number of MSYS OOB Port */
#define BC2_BOARD_MSYS_OOB_PORT_SERDES_CNS 21

/**
* @internal appDemoBc2IsInternalCpuEnabled function
* @endinternal
*
* @brief   Check if internal CPU enabled for Bobcat2 boards.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_TRUE                  - internal CPU enabled
* @retval GT_FALSE                 - internal CPU disabled
*/
GT_BOOL appDemoBc2IsInternalCpuEnabled
(
    IN  GT_U8  devNum
)
{
    /* image is build for MSYS. MSYS build may be used for AXP external CPUs also.
       Made additional checks to distinguish such cases. */

    GT_U32                          regAddr;  /* register address */
    GT_U32                          regData;  /* register value*/
    GT_STATUS                       rc;       /* return code */

    if (prvCpssDrvPpConfig[devNum]->hwInfo[0].busType != CPSS_HW_INFO_BUS_TYPE_MBUS_E)
        return GT_FALSE;

    switch(PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E: /* Bobcat2 and BobK devices */
            /* Get boot_mode SAR configuration */
            if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) != GT_FALSE)
            {
                regAddr =
                    PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                            DFXServerUnitsDeviceSpecificRegs.deviceSAR1;
                rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,
                                                                 regAddr, 13, 3,
                                                                 &regData);
                if(rc != GT_OK)
                {
                    return GT_FALSE;
                }

                /* CPU disabled when:
                   0x5 = No Boot; (i.e. CPU IDLE)
                   CPU enabled for all other options */
                if (regData != 5)
                {
                    if (CPSS_PP_SUB_FAMILY_NONE_E == PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devSubFamily)
                    {
                        /* make DDR assignment (msys_tm_ddr_sel) additional check for BC2 devices */
                        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,
                                                                         regAddr, 11, 1,
                                                                         &regData);
                        if((rc != GT_OK) || (regData == 0))
                        {
                            /* msys_tm_ddr_sel = 0 means that DDR is for TM only */
                            return GT_FALSE;
                        }
                    }
                    return GT_TRUE;
                }
            }
            break;

        default: /* Aldrin, AC3X, Bobcat3 does not have internal general purpose CPU */
            break;
    }

    /* CPU disabled */
    return GT_FALSE;
}

CPSS_DXCH_PORT_MAP_STC  *configBoardAfterPhase1MappingNoCPUCorrect
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_PORT_MAP_STC  *mapArrPtr,
    IN  GT_U32                   mapArrLen,
    OUT GT_U32                  *newMapArrLenPtr

)
{
    static                   CPSS_DXCH_PORT_MAP_STC newMapArr[256];
    CPSS_DXCH_PORT_MAP_STC  *mapPtr;
    CPSS_DXCH_PORT_MAP_STC  *newMapPtr;
    GT_U32    newLen;
    GT_U32    i;
    GT_BOOL   copyLine;

    newLen = 0;
    newMapPtr = &newMapArr[0];

    for (i = 0, mapPtr = mapArrPtr; i < mapArrLen; i++, mapPtr++)
    {
        copyLine = GT_TRUE;
        /* default map use MAC 63 for physical port 82.
            Need to use MAC 62 for "no CPU" systems to avoid collisions
            with configuration below. */
        if((82 == mapPtr->physicalPortNumber) &&
            (mapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E) &&
            (mapPtr->interfaceNum == 63))
        {
            copyLine = GT_FALSE;
            if (PRV_CPSS_PHY_PORT_IS_EXIST_MAC(SYSTEM_DEV_NUM_MAC(0), 62))
            {
                mapPtr->interfaceNum = 62;
                copyLine = GT_TRUE;
            }
        }


        /* change physical port 63 to be usual ethernet port but not SDMA one.
           Aldrin2 already has all MACs mapped to Physical ports. */
        if(mapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            copyLine = GT_FALSE;
            if ((63 == mapPtr->physicalPortNumber) && (!PRV_CPSS_SIP_5_25_CHECK_MAC(devNum)))
            {
                if (PRV_CPSS_PHY_PORT_IS_EXIST_MAC(SYSTEM_DEV_NUM_MAC(0), 63))
                {
                    mapPtr->physicalPortNumber = 83;
                    mapPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
                    mapPtr->interfaceNum = 63;
                    copyLine = GT_TRUE;
                }
            }
        }

        /* don't copy CPU */
        if(copyLine == GT_TRUE)
        {
            *newMapPtr = *mapPtr;
            newMapPtr++;
            newLen++;
        }
    }
    *newMapArrLenPtr = newLen;
    return  &newMapArr[0];
}

/**
* @internal configBoardAfterPhase1 function
* @endinternal
*
* @brief   This function performs all needed configurations that should be done
*         after phase1.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS configBoardAfterPhase1
(
    IN  GT_U8   boardRevId
)
{
    GT_U8                   dev;
    GT_STATUS               rc;
    GT_U32                  mapArrLen;
    CPSS_DXCH_PORT_MAP_STC  *mapArrPtr;
    GT_U32                  noCpu;
    GT_U32                  coreClockDB;
    GT_U32                  coreClockHW;
    GT_BOOL                 isTmSupported = GT_FALSE;
    GT_BOOL                 isTmEnabled = GT_FALSE;

    rc = configBoardAfterPhase1MappingGet(boardRevId,/*OUT*/&mapArrPtr,&mapArrLen);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (mapArrPtr == NULL)
    {
        cpssOsPrintf("\n-->ERROR : configBoardAfterPhase1() mapping : rev =%d is not supported\n", boardRevId);
        return GT_NOT_SUPPORTED;
    }

    rc = cpssDxChHwCoreClockGet(appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].devNum, &coreClockDB, &coreClockHW);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoDxChBoardTypeGet(SYSTEM_DEV_NUM_MAC(0), boardRevId, &bc2BoardType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("bobcat2BoardTypeGet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    switch (bc2BoardType)
    {
        case APP_DEMO_CAELUM_BOARD_DB_CNS:
        case APP_DEMO_CETUS_BOARD_DB_CNS:
        case APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS:
        case APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS:
            isBobkBoard = GT_TRUE;
            break;
        default:
            /*
            APP_DEMO_BC2_BOARD_DB_CNS:
            APP_DEMO_BC2_BOARD_RD_MSI_CNS:
            APP_DEMO_BC2_BOARD_RD_MTL_CNS:
            */
            isBobkBoard = GT_FALSE;
            break;
    }

    appDemoIsTmSupported(SYSTEM_DEV_NUM_MAC(0), &isTmSupported);

    isTmEnabled = appDemoIsTmEnabled();

    switch(appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].deviceId)
    {
        case CPSS_98DX4220_CNS:
            mapArrPtr = &bc2defaultMap_4220[0];
            mapArrLen = sizeof(bc2defaultMap_4220)/sizeof(bc2defaultMap_4220[0]);
            if(isTmEnabled && isTmSupported)
            {
                mapArrPtr = &bobkCaelumTmEnable2412[0];
                mapArrLen = sizeof(bobkCaelumTmEnable2412)/sizeof(bobkCaelumTmEnable2412[0]);
            }
            break;

        case CPSS_98DX4221_CNS:
            mapArrPtr = &bc2defaultMap_4221[0];
            mapArrLen = sizeof(bc2defaultMap_4221)/sizeof(bc2defaultMap_4221[0]);
            if(isTmEnabled && isTmSupported)
            {
                mapArrPtr = &bobkCaelumTmEnable4812[0];
                mapArrLen = sizeof(bobkCaelumTmEnable4812)/sizeof(bobkCaelumTmEnable4812[0]);
            }
            break;

        /******************/
        /* CAELUM devices */
        /******************/
        case CPSS_98DX4204_CNS:
        case CPSS_98DX4210_CNS:
        case CPSS_98DX3346_CNS:
            mapArrPtr = &caelumDefaultMap_4204[0];
            mapArrLen = sizeof(caelumDefaultMap_4204)/sizeof(caelumDefaultMap_4204[0]);
            if(isTmEnabled && isTmSupported)
            {
                mapArrPtr = &bobkCaelumTmEnable2412[0];
                mapArrLen = sizeof(bobkCaelumTmEnable2412)/sizeof(bobkCaelumTmEnable2412[0]);
            }
            break;
        case CPSS_98DX4212_CNS:
        case CPSS_98DX3345_CNS:
            mapArrPtr = &caelumDefaultMap_4212[0];
            mapArrLen = sizeof(caelumDefaultMap_4212)/sizeof(caelumDefaultMap_4212[0]);
            if(isTmEnabled && isTmSupported)
            {
                mapArrPtr = &bobkCaelumTmEnable2412[0];
                mapArrLen = sizeof(bobkCaelumTmEnable2412)/sizeof(bobkCaelumTmEnable2412[0]);
            }
            break;
        case CPSS_98DX42KK_CNS:
        case CPSS_98DX4203_CNS:
        case CPSS_98DX4211_CNS:
        case CPSS_98DX3347_CNS:
            mapArrPtr = &caelumDefaultMap[0];
            mapArrLen = sizeof(caelumDefaultMap)/sizeof(caelumDefaultMap[0]);

            if(isTmEnabled && isTmSupported)
            {
                mapArrPtr = &bobkCaelumTmEnable4812[0];
                mapArrLen = sizeof(bobkCaelumTmEnable4812)/sizeof(bobkCaelumTmEnable4812[0]);
            }
            break;
        case CPSS_98DX4222_CNS:
            mapArrPtr = &bc2defaultMap_4222[0];
            mapArrLen = sizeof(bc2defaultMap_4222)/sizeof(bc2defaultMap_4222[0]);
            break;
        case CPSS_98DX8216_CNS:
            mapArrPtr = &bc2defaultMap_8216[0];
            mapArrLen = sizeof(bc2defaultMap_8216)/sizeof(bc2defaultMap_8216[0]);
            break;
        case CPSS_98DX8219_CNS:
            mapArrPtr = &bc2defaultMap_8219[0];
            mapArrLen = sizeof(bc2defaultMap_8219)/sizeof(bc2defaultMap_8219[0]);
            break;
        case CPSS_98DX8224_CNS:
            mapArrPtr = &bc2defaultMap_8224[0];
            mapArrLen = sizeof(bc2defaultMap_8224)/sizeof(bc2defaultMap_8224[0]);
            break;
        case CPSS_98DX4253_CNS:
            mapArrPtr = &bc2defaultMap_4253[0];
            mapArrLen = sizeof(bc2defaultMap_4253)/sizeof(bc2defaultMap_4253[0]);
            break;
        case CPSS_98DX8332_Z0_CNS:
            mapArrPtr = &aldrinZ0DefaultMap_8332_Z0[0];
            mapArrLen = sizeof(aldrinZ0DefaultMap_8332_Z0)/sizeof(aldrinZ0DefaultMap_8332_Z0[0]);
            break;
        case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
            mapArrPtr = &cetusDefaultMap_56to59_62_64to71[0];
            mapArrLen = sizeof(cetusDefaultMap_56to59_62_64to71)/sizeof(cetusDefaultMap_56to59_62_64to71[0]);
            appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].portInitlist_AllPorts_used = 0;
            if(isTmEnabled)
            {
                mapArrPtr = &bobkCetusTmEnableMap[0];
                mapArrLen = sizeof(bobkCetusTmEnableMap)/sizeof(bobkCetusTmEnableMap[0]);
            }

            if (appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].deviceId ==  CPSS_98DX8208_CNS)
            {
                mapArrPtr = &bc2defaultMap_64to71[0];
                mapArrLen = sizeof(bc2defaultMap_64to71)/sizeof(bc2defaultMap_64to71[0]);
            }
            break;
        case CPSS_BOBCAT3_ALL_DEVICES_CASES_MAC:
        case CPSS_ARMSTRONG_ALL_DEVICES_CASES_MAC:
            {
                GT_U32  useSinglePipe = 0;
                #ifdef GM_USED
                    useSinglePipe = 1;
                #endif/* GM_USED */

                if(useSinglePipe)
                {
                    ARR_PTR_AND_SIZE_MAC(singlePipe_singleDp_bc3defaultMap,mapArrPtr,mapArrLen);
                }
                else
                {
                    ARR_PTR_AND_SIZE_MAC(bc3defaultMap,mapArrPtr,mapArrLen);
                }
            }



            dev = appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].devNum;

            if(PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev) == 512)
            {
                GT_U32  ii;
                for(ii = 0 ; ii < mapArrLen ; ii++)
                {
                    if(mapArrPtr[ii].physicalPortNumber != 63)
                    {
                        /* make sure that port number will be more than 256 */
                        mapArrPtr[ii].physicalPortNumber |= BIT_8;
                    }
                }
            }

            break;
        case CPSS_ALDRIN2_ALL_DEVICES_CASES_MAC:
            {
                GT_U32  useSingleDp = 0;
                #ifdef GM_USED
                    useSingleDp = 1;
                #endif/* GM_USED */
                if(useSingleDp)
                {
                    ARR_PTR_AND_SIZE_MAC(singleDp_aldrin2defaultMap,mapArrPtr,mapArrLen);
                }
                else
                {
                    if (appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].deviceId == CPSS_98EX5524_CNS ||
                        appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].deviceId == CPSS_98DX8524_CNS)
                    {
                        ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap_48_ports, mapArrPtr, mapArrLen);
                    }
                    else
                    if (appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].deviceId == CPSS_98DX8525_CNS)
                    {
                        ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap_24_ports, mapArrPtr, mapArrLen);
                    }
                    else
                    if (appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].deviceId == CPSS_98DX4310_CNS)
                    {
                        ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap_64_ports, mapArrPtr, mapArrLen);
                    }
                    else
                    if (appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].deviceId == CPSS_98DX8410_CNS)
                    {
                        ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap_48_special_map_ports, mapArrPtr, mapArrLen);
                    }
                    else
                    {
                        ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap, mapArrPtr, mapArrLen);
                        if (coreClockDB == 525)
                        {
                            /* Remove unused SDMA ports in order to meet BW limits */
                            mapArrLen -= 3;
                        }
                    }
                }
            }
            break;

        /******************/
        /* ALDRIN devices */
        /******************/
        case CPSS_98DX8308_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_8_ports,mapArrPtr,mapArrLen);
            break;
        case CPSS_98DX8312_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_12_ports,mapArrPtr,mapArrLen);
            break;
        case CPSS_98DX8316_CNS:
        case CPSS_98DXH831_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_16_ports,mapArrPtr,mapArrLen);
            break;
        case CPSS_98DX3256_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_18_ports,mapArrPtr,mapArrLen);
            break;
        case CPSS_98DX3257_CNS:
        case CPSS_98DX8324_CNS:
        case CPSS_98DXH832_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_24_ports,mapArrPtr,mapArrLen);
            break;
        case CPSS_98DX3255_CNS:
        case CPSS_98DX3258_CNS:
        case CPSS_98DX3259_CNS:
        case CPSS_98DX3248_CNS:
        case CPSS_98DX3249_CNS:
        case CPSS_98DX8314_CNS:
        case CPSS_98DX8315_CNS:
        case CPSS_98DX8332_CNS:
        case CPSS_98DXZ832_CNS:
        case CPSS_98DXH833_CNS:
            if(cpssDeviceRunCheck_onEmulator() &&
               (0 == cpssDeviceRunCheck_onEmulator_isAldrinFull()))
            {
                /* limited Aldrin without all ports */
                ARR_PTR_AND_SIZE_MAC(onEmulator_aldrinDefaultMap,mapArrPtr,mapArrLen);
            }
            else
            {
                ARR_PTR_AND_SIZE_MAC(aldrinDefaultMap,mapArrPtr,mapArrLen);
            }
            break;

        default:
            break;
    }

    /* TM related port mapping adjustments for BC2 only */
    if (!isBobkBoard)
    {
        if (appDemoTmScenarioMode == CPSS_TM_2 || appDemoTmScenarioMode == CPSS_TM_3)
        {
            mapArrPtr = &bc2TmEnableE2Map[0];
            mapArrLen = sizeof(bc2TmEnableE2Map)/sizeof(bc2TmEnableE2Map[0]);
        }

        if (appDemoTmScenarioMode == CPSS_TM_8)
        {
            mapArrPtr = &bc2TmEnableE8Map[0];
            mapArrLen = sizeof(bc2TmEnableE8Map)/sizeof(bc2TmEnableE8Map[0]);
        }
    }
    else
    {
        /* same mapping for cetus and caelum, (in caelum mapping: 56-69 is different from the default tm scenario 0-48 ) */
        if (appDemoTmScenarioMode >= CPSS_TM_2 && appDemoTmScenarioMode <= CPSS_TM_5)
        {
                        mapArrPtr = &bobkTmEnableE2Map[0];
            mapArrLen = sizeof(bobkTmEnableE2Map)/sizeof(bobkTmEnableE2Map[0]);
        }


        if (appDemoTmScenarioMode == CPSS_TM_8)
        {
                        mapArrPtr = &bobkTmEnableE8Map[0];
            mapArrLen = sizeof(bobkTmEnableE8Map)/sizeof(bobkTmEnableE8Map[0]);
        }
    }

    if(appDemoPortMapPtr && appDemoPortMapSize)
    {
        /* allow to 'FORCE' specific array by other logic */
        mapArrPtr = appDemoPortMapPtr;
        mapArrLen = appDemoPortMapSize;
    }




    rc = appDemoDbEntryGet("noCpu", &noCpu);
    if(rc != GT_OK)
    {
        noCpu = 0;
    }

    if(1 == noCpu)
    {
        mapArrPtr = configBoardAfterPhase1MappingNoCPUCorrect(appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].devNum, mapArrPtr,mapArrLen,/*OUT*/&mapArrLen);
    }

    for(dev = SYSTEM_DEV_NUM_MAC(0); dev < SYSTEM_DEV_NUM_MAC(ppCounter); dev++)
    {
        /* perform board type recognition */
        rc = appDemoDxChBoardTypeGet(dev,boardRevId,&bc2BoardType);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("bobcat2BoardTypeGet", rc);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* print board type */
        bobcat2BoardTypePrint(dev, bc2BoardType);

        rc = cpssDxChPortPhysicalPortMapSet(dev, mapArrLen, mapArrPtr);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapSet", rc);
        if (GT_OK != rc)
        {
            return rc;
        }

        rc = appDemoDxChMaxMappedPortSet(dev, mapArrLen, mapArrPtr);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChMaxMappedPortSet", rc);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* restore OOB port configuration after systemReset */
        if ((bc2BoardResetDone == GT_TRUE) && appDemoBc2IsInternalCpuEnabled(dev) &&  (!isBobkBoard))
        {
            MV_HWS_SERDES_CONFIG_STR    serdesConfig;

            /* configure SERDES TX interface */
            rc = mvHwsSerdesTxIfSelect(dev, 0, BC2_BOARD_MSYS_OOB_PORT_SERDES_CNS, COM_PHY_28NM, 1);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("mvHwsSerdesTxIfSelect", rc);
            if (GT_OK != rc)
            {
                return rc;
            }

            serdesConfig.baudRate = _1_25G;
            serdesConfig.media = XAUI_MEDIA;
            serdesConfig.busWidth = _10BIT_ON;
            serdesConfig.refClock = _156dot25Mhz;
            serdesConfig.refClockSource = PRIMARY;
            serdesConfig.rxEncoding = SERDES_ENCODING_NA;
            serdesConfig.serdesType = COM_PHY_28NM;
            serdesConfig.txEncoding = SERDES_ENCODING_NA;

            /* power UP and configure SERDES */
            rc = mvHwsSerdesPowerCtrl(dev, 0, BC2_BOARD_MSYS_OOB_PORT_SERDES_CNS, GT_TRUE, &serdesConfig);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("mvHwsSerdesPowerCtrl", rc);
            if (GT_OK != rc)
            {
                return rc;
            }
        }

        if(appDemoBc2IsInternalCpuEnabled(dev))
        {
            /* the embedded CPU not allow to do 'soft reset' */
            softResetSupported = GT_FALSE;
        }
        else
        {
            softResetSupported = GT_TRUE;
        }

        if(softResetSupported == GT_FALSE)
        {
            rc = cpssDevSupportSystemResetSet(dev, GT_FALSE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevSupportSystemResetSet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if(isBobkBoard)
        {
            oobPortUsed = GT_TRUE;
            oobPort = 90;
        }
        else
        {
            oobPortUsed = GT_FALSE;
        }

        if(oobPortUsed == GT_TRUE)
        {
            rc = cpssDevMustNotResetPortSet(dev, oobPort, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevMustNotResetPortSet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

    }

    rc = waInit(boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("waInit", rc);

    bc2BoardResetDone = GT_FALSE;

    return GT_OK;
}
static GT_U32   portInitlist_AllPorts_used = 0;
/**
* @internal configBoardAfterPhase1Simple function
* @endinternal
*
* @brief   This function performs all needed configurations that should be done
*         after phase1.
* @param[in] boardRevId               - The board revision Id.
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of configBoardAfterPhase1.
*
*/
static GT_STATUS configBoardAfterPhase1Simple
(
    IN  GT_U8   boardRevId,
    IN  GT_U8   devNum

)
{
    GT_U32  waIndex=0;
    GT_STATUS rc;
    GT_U32                   mapArrLen;
    CPSS_DXCH_PORT_MAP_STC  *mapArrPtr, *mapPtr;
    GT_U32    i;
    CPSS_PP_DEVICE_TYPE devType;
    CPSS_DXCH_CFG_DEV_INFO_STC   devInfo;

    rc = configBoardAfterPhase1MappingGet(boardRevId,/*OUT*/&mapArrPtr,&mapArrLen);
    if(rc != GT_OK)
        return rc;

    if (mapArrPtr == NULL)
    {
        cpssOsPrintf("\n-->ERROR : configBoardAfterPhase1() mapping : rev =%d is not supported", boardRevId);
        return GT_NOT_SUPPORTED;
    }

    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCfgDevInfoGet", rc);
    if(rc != GT_OK)
        return rc;

    portInitlist_AllPorts_used = 0;
    devType = devInfo.genDevInfo.devType;

    osPrintf("devType is 0x%X\n",devType);
    switch(devType)
    {
        case CPSS_98DX4220_CNS:
            mapArrPtr = &bc2defaultMap_4220[0];
            mapArrLen = sizeof(bc2defaultMap_4220)/sizeof(bc2defaultMap_4220[0]);
            break;
        case CPSS_98DX4221_CNS:
        case CPSS_BOBK_CAELUM_DEVICES_CASES_MAC:
            mapArrPtr = &bc2defaultMap_4221[0];
            mapArrLen = sizeof(bc2defaultMap_4221)/sizeof(bc2defaultMap_4221[0]);
            break;
        case CPSS_98DX8216_CNS:
            mapArrPtr = &bc2defaultMap_8216[0];
            mapArrLen = sizeof(bc2defaultMap_8216)/sizeof(bc2defaultMap_8216[0]);
            break;
        case CPSS_98DX8219_CNS:
            mapArrPtr = &bc2defaultMap_8219[0];
            mapArrLen = sizeof(bc2defaultMap_8219)/sizeof(bc2defaultMap_8219[0]);
            break;
        case CPSS_98DX8224_CNS:
            mapArrPtr = &bc2defaultMap_8224[0];
            mapArrLen = sizeof(bc2defaultMap_8224)/sizeof(bc2defaultMap_8224[0]);
            break;
        case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
            mapArrPtr = &cetusDefaultMap_56to59_62_64to71[0];
            mapArrLen = sizeof(cetusDefaultMap_56to59_62_64to71)/sizeof(cetusDefaultMap_56to59_62_64to71[0]);
            portInitlist_AllPorts_used = 1;
            break;
        case CPSS_BOBCAT3_ALL_DEVICES_CASES_MAC:
        case CPSS_ARMSTRONG_ALL_DEVICES_CASES_MAC:
            #ifdef GM_USED
                ARR_PTR_AND_SIZE_MAC(singlePipe_singleDp_bc3defaultMap,mapArrPtr,mapArrLen);
            #else /*not GM_USED */
                ARR_PTR_AND_SIZE_MAC(bc3defaultMap,mapArrPtr,mapArrLen);
            #endif/* not GM_USED */

            if(PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum) == 512)
            {
                GT_U32  ii;
                for(ii = 0 ; ii < mapArrLen ; ii++)
                {
                    if(mapArrPtr[ii].physicalPortNumber != 63)
                    {
                        /* make sure that port number will be more than 256 */
                        mapArrPtr[ii].physicalPortNumber |= BIT_8;
                    }
                }
            }
            break;
        case CPSS_ALDRIN2_ALL_DEVICES_CASES_MAC:
            {
                GT_U32  useSingleDp = 0;
                #ifdef GM_USED
                    useSingleDp = 1;
                #endif/* GM_USED */
                if(useSingleDp)
                {
                    ARR_PTR_AND_SIZE_MAC(singleDp_aldrin2defaultMap,mapArrPtr,mapArrLen);
                }
                else
                {
                    ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap,mapArrPtr,mapArrLen);
                }
            }
            break;
        default:
            break;
    }

    /* TM related port mapping adjustments */
    if (appDemoTmScenarioMode == CPSS_TM_2 || appDemoTmScenarioMode == CPSS_TM_3)
    {
        mapArrPtr = &bc2TmEnableE2Map[0];
        mapArrLen = sizeof(bc2TmEnableE2Map)/sizeof(bc2TmEnableE2Map[0]);
    }

    if (appDemoTmScenarioMode==CPSS_TM_8)
    {
        mapArrPtr = &bc2TmEnableE8Map[0];
        mapArrLen = sizeof(bc2TmEnableE8Map)/sizeof(bc2TmEnableE8Map[0]);
    }

    if(noBC2CpuPort)
    {
        for (i = 0, mapPtr = mapArrPtr; i < mapArrLen; i++, mapPtr++)
        {
            if(63 == mapPtr->physicalPortNumber)
            {
                mapPtr->physicalPortNumber = 83;
                mapPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
                mapPtr->interfaceNum = 63;
                break;
            }
        }
    }

    /* there is only one device in this setting */

    /* perform board type recognition - for RD and DB boards and their PHY configuration*/
    rc = appDemoDxChBoardTypeGet(devNum,boardRevId,&bc2BoardType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("bobcat2BoardTypeGet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }
    bc2BoardRevId = boardRevId;

    /* print board type */
    bobcat2BoardTypePrint(devNum,bc2BoardType);

    rc = cpssDxChPortPhysicalPortMapSet(devNum, mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapSet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* it's for appdemo DB only */
    rc = appDemoDxChMaxMappedPortSet(devNum, mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChMaxMappedPortSet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    while(BC2WaList[waIndex] != CPSS_DXCH_IMPLEMENT_WA_LAST_E)
    {
        waIndex++;
    }

    rc = cpssDxChHwPpImplementWaInit(devNum,waIndex,BC2WaList, NULL);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal allocateDmaMemSimple function
* @endinternal
*
* @brief   This function allocates memory for phase2 initialization stage, the
*         allocations include: Rx Descriptors / Buffer, Tx descriptors, Address
*         update descriptors.
* @param[in] devType                  - The Pp device type to allocate the memory for.
* @param[in] rxDescNum                - Number of Rx descriptors (and buffers) to allocate.
* @param[in] rxBufSize                - Size of each Rx Buffer to allocate.
* @param[in] rxBufAllign              - Ammount of allignment required on the Rx buffers.
* @param[in] txDescNum                - Number of Tx descriptors to allocate.
* @param[in] auDescNum                - Number of address update descriptors to allocate.
* @param[in,out] ppPhase2Params           - The device's Phase2 parameters.
* @param[in,out] ppPhase2Params           - The device's Phase2 parameters including the required
*                                      allocations.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of appDemoAllocateDmaMem
*
*/
GT_STATUS allocateDmaMemSimple
(
    IN      CPSS_PP_DEVICE_TYPE         devType,
    IN      GT_U32                      rxDescNum,
    IN      GT_U32                      rxBufSize,
    IN      GT_U32                      rxBufAllign,
    IN      GT_U32                      txDescNum,
    IN      GT_U32                      auDescNum,
    INOUT   CPSS_DXCH_PP_PHASE2_INIT_INFO_STC  *ppPhase2Params
)
{
    GT_U32                      tmpData;
    GT_U32                      *tmpPtr;
    GT_U32                      fuDescNum; /* number of FU descriptors ... allow to be diff then auDescNum */
    GT_BOOL                     txGenQueue[8];      /* Enable Tx queue to work in generator mode */
    GT_U32                      txGenNumOfDescBuff; /* Number of descriptors and buffers per Tx queue */
    GT_U32                      txGenBuffSize;      /* Size of buffer per Tx queue */
    GT_U32                      txGenDescSize;      /* Size of descriptor per Tx queue */
    GT_U32                      txQue;              /* Tx queue number */
    GT_STATUS                   rc = GT_OK;

    fuDescNum = auDescNum;

    for(txQue = 0; txQue < 8; txQue++)
    {
        txGenQueue[txQue] = GT_FALSE;
    }

    txGenNumOfDescBuff = 512;
    txGenBuffSize = 128 + 16; /* 16 bytes of eDsa tag */

    /* Au block size calc & malloc  */
    rc = cpssDxChHwAuDescSizeGet(devType, &tmpData);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwAuDescSizeGet",rc);
    if (GT_OK != rc)
        return rc;

    if (auDescNum == 0)
    {
        ppPhase2Params->auqCfg.auDescBlock = 0;
        ppPhase2Params->auqCfg.auDescBlockSize = 0;
    }
    else
    {
        ppPhase2Params->auqCfg.auDescBlockSize = tmpData * auDescNum;
        ppPhase2Params->auqCfg.auDescBlock =
            osCacheDmaMalloc(ppPhase2Params->auqCfg.auDescBlockSize +
                            tmpData);/*allocate space for one message more for alignment purposes
                                      NOTE: we not add it to the size , only to the buffer ! */
        if(ppPhase2Params->auqCfg.auDescBlock == NULL)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("allocateDmaMemSimple",rc);
            return GT_OUT_OF_CPU_MEM;
        }
        if(((GT_UINTPTR)ppPhase2Params->auqCfg.auDescBlock) % tmpData)
        {
            /* add to the size the extra value for alignment , to the actual size
               will be as needed , after the reduction in the cpss code */
            ppPhase2Params->auqCfg.auDescBlockSize += tmpData;
        }
    }

    if (fuDescNum == 0)
    {
        ppPhase2Params->fuqCfg.auDescBlock = 0;
        ppPhase2Params->fuqCfg.auDescBlockSize = 0;
    }
    else
    {
        /* Fu block size calc & malloc  */
        rc = cpssDxChHwAuDescSizeGet(devType,&tmpData);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwAuDescSizeGet",rc);
        if (GT_OK != rc)
            return rc;

        ppPhase2Params->fuqCfg.auDescBlockSize = tmpData * fuDescNum;
        ppPhase2Params->fuqCfg.auDescBlock =
            osCacheDmaMalloc(ppPhase2Params->fuqCfg.auDescBlockSize +
                            tmpData);/*allocate space for one message more for alignment purposes
                                      NOTE: we not add it to the size , only to the buffer ! */
        if(ppPhase2Params->fuqCfg.auDescBlock == NULL)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("allocateDmaMemSimple",GT_OUT_OF_CPU_MEM);
            return GT_OUT_OF_CPU_MEM;
        }

        if(((GT_UINTPTR)ppPhase2Params->fuqCfg.auDescBlock) % tmpData)
        {
            /* add to the size the extra value for alignment , to the actual size
               will be as needed , after the reduction in the cpss code */
            ppPhase2Params->fuqCfg.auDescBlockSize += tmpData;
        }
    }

    /* Tx block size calc & malloc  */
    rc = cpssDxChHwTxDescSizeGet(devType,&tmpData);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwTxDescSizeGet",rc);
    if (GT_OK != rc)
        return rc;

    ppPhase2Params->netIfCfg.txDescBlockSize = tmpData * txDescNum;
    ppPhase2Params->netIfCfg.txDescBlock =
        osCacheDmaMalloc(ppPhase2Params->netIfCfg.txDescBlockSize);
    if(ppPhase2Params->netIfCfg.txDescBlock == NULL)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("allocateDmaMemSimple",GT_OUT_OF_CPU_MEM);
        osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
        return GT_OUT_OF_CPU_MEM;
    }

    /* Rx block size calc & malloc  */
    rc = cpssDxChHwRxDescSizeGet(devType,&tmpData);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwRxDescSizeGet",rc);
    if (GT_OK != rc)
        return rc;

    ppPhase2Params->netIfCfg.rxDescBlockSize = tmpData * rxDescNum;
    ppPhase2Params->netIfCfg.rxDescBlock =
        osCacheDmaMalloc(ppPhase2Params->netIfCfg.rxDescBlockSize);
    if(ppPhase2Params->netIfCfg.rxDescBlock == NULL)
    {
        osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
        osCacheDmaFree(ppPhase2Params->netIfCfg.txDescBlock);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("allocateDmaMemSimple",GT_OUT_OF_CPU_MEM);
        return GT_OUT_OF_CPU_MEM;
    }

    /* init the Rx buffer allocation method */
    /* Set the system's Rx buffer size.     */
    if((rxBufSize % rxBufAllign) != 0)
    {
        rxBufSize = (rxBufSize + (rxBufAllign - (rxBufSize % rxBufAllign)));
    }

    if (ppPhase2Params->netIfCfg.rxBufInfo.allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E)
    {
        ppPhase2Params->netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockSize =
        rxBufSize * rxDescNum;

        /* status of RX buffers - cacheable or not, has been set in getPpPhase2ConfigSimple */
        /* If RX buffers should be cachable - allocate it from regular memory */
        if (GT_TRUE == ppPhase2Params->netIfCfg.rxBufInfo.buffersInCachedMem)
        {
            tmpPtr = osMalloc(((rxBufSize * rxDescNum) + rxBufAllign));
        }
        else
        {
            tmpPtr = osCacheDmaMalloc(((rxBufSize * rxDescNum) + rxBufAllign));
        }

        if(tmpPtr == NULL)
        {
            osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
            osCacheDmaFree(ppPhase2Params->netIfCfg.txDescBlock);
            osCacheDmaFree(ppPhase2Params->netIfCfg.rxDescBlock);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("allocateDmaMemSimple",GT_OUT_OF_CPU_MEM);
            return GT_OUT_OF_CPU_MEM;
        }

        if((((GT_UINTPTR)tmpPtr) % rxBufAllign) != 0)
        {
            tmpPtr = (GT_U32*)(((GT_UINTPTR)tmpPtr) +
                               (rxBufAllign - (((GT_UINTPTR)tmpPtr) % rxBufAllign)));
        }
        ppPhase2Params->netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockPtr = tmpPtr;
    }
    else if (ppPhase2Params->netIfCfg.rxBufInfo.allocMethod == CPSS_RX_BUFF_NO_ALLOC_E)
    {
        /* do not allocate rx buffers*/
    }
    else
    {
        /* dynamic RX buffer allocation currently is not supported by appDemo*/
        osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
        osCacheDmaFree(ppPhase2Params->netIfCfg.txDescBlock);
        osCacheDmaFree(ppPhase2Params->netIfCfg.rxDescBlock);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("allocateDmaMemSimple",GT_NOT_SUPPORTED);
        return GT_NOT_SUPPORTED;
    }

    if(ppPhase2Params->useMultiNetIfSdma)
    {
        CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC  * sdmaQueuesConfigPtr;

        /* Tx block size calc & malloc  */
        rc = cpssDxChHwTxDescSizeGet(devType,&txGenDescSize);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwTxDescSizeGet",rc);
        if (GT_OK != rc)
            return rc;


        for(txQue = 0; txQue < 8; txQue++)
        {
            if(txGenQueue[txQue] == GT_FALSE)
            {
                /* Regular mode is not supported yet */
                continue;
            }

            /* Generator mode */
            sdmaQueuesConfigPtr =
                &ppPhase2Params->multiNetIfCfg.txSdmaQueuesConfig[0][txQue];
            sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;
            sdmaQueuesConfigPtr->numOfTxDesc = txGenNumOfDescBuff / 2;

            sdmaQueuesConfigPtr->queueMode =
                CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E;
            sdmaQueuesConfigPtr->numOfTxBuff = sdmaQueuesConfigPtr->numOfTxDesc;

            sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize =
                (sdmaQueuesConfigPtr->numOfTxDesc + 1) * (txGenDescSize + txGenBuffSize);
            sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr =
                osCacheDmaMalloc(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
            sdmaQueuesConfigPtr->buffSize = txGenBuffSize;
        }
    }

    return rc;
}


/**
* @internal getPpPhase2Config function
* @endinternal
*
* @brief   Returns the configuration parameters for corePpHwPhase2Init().
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] oldDevNum                - The old Pp device number to get the parameters for.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpPhase2Config
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       oldDevNum,
    OUT CPSS_PP_PHASE2_INIT_PARAMS  *phase2Params
)
{
    GT_STATUS                   rc;
    CPSS_PP_PHASE2_INIT_PARAMS  localPpPh2Config = CPSS_PP_PHASE2_DEFAULT;
    GT_U32                      auDescNum;
    GT_U32                      fuDescNum;
    GT_U32                      rxDescNum = RX_DESC_NUM_DEF;
    GT_U32                      rxBufSize = RX_BUFF_SIZE_DEF;
    GT_U32                      txDescNum = TX_DESC_NUM_DEF;
    GT_U32                      tmpData;
#ifdef LINUX_NOKM
    GT_U32                      dmaSize = 0;
#endif

    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);

    localPpPh2Config.devNum     = oldDevNum;
    localPpPh2Config.deviceId   = appDemoPpConfigList[oldDevNum].deviceId;

    localPpPh2Config.fuqUseSeparate = GT_TRUE;

    /* Single AUQ, since single FDB unit */
    auDescNum = AU_DESC_NUM_DEF;

    fuDescNum = AU_DESC_NUM_DEF;

    if(appDemoDbEntryGet("fuDescNum", &tmpData) == GT_OK)
    {
        /* Someone already stated the number of fuDescNum, so we NOT override it ! */
    }
    else
    {
        /* Add the fuDescNum to the DB, to allow appDemoAllocateDmaMem(...) to use it ! */
        rc = appDemoDbEntryAdd("fuDescNum", fuDescNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if( appDemoDbEntryGet("useMultiNetIfSdma", &tmpData) == GT_NO_SUCH )
    {
        localPpPh2Config.useMultiNetIfSdma = GT_TRUE;
        if( appDemoDbEntryGet("skipTxSdmaGenDefaultCfg", &tmpData) == GT_NO_SUCH )
        {

            /* Enable Tx queue 3 to work in Tx queue generator mode */
            rc = appDemoDbEntryAdd("txGenQueue_3", GT_TRUE);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Enable Tx queue 6 to work in Tx queue generator mode */
            rc = appDemoDbEntryAdd("txGenQueue_6", GT_TRUE);
            if(rc != GT_OK)
            {
                 return rc;
             }
         }
    }

#ifdef LINUX_NOKM

    extDrvGetDmaSize(&dmaSize);
    if (dmaSize < _512K)
    {
        rxDescNum = 0;
        rxBufSize = 0;
        txDescNum = 0;
        auDescNum = 0;
        appDemoDbEntryAdd("rxDescNum", 0);
        appDemoDbEntryAdd("txDescNum", 0);
        appDemoDbEntryAdd("auDescNum", 0);
        appDemoDbEntryAdd("fuDescNum", 0);
        localPpPh2Config.useMultiNetIfSdma = GT_FALSE;
        appDemoDbEntryAdd("useMultiNetIfSdma", 0);
        localPpPh2Config.netIfCfg.rxBufInfo.allocMethod = CPSS_RX_BUFF_NO_ALLOC_E;
        /* set SGMII mode */
        appDemoSysConfig.cpuEtherPortUsed = GT_TRUE;
        appDemoPpConfigList[oldDevNum].cpuPortMode = CPSS_NET_CPU_PORT_MODE_MII_E;
    }
    else if (dmaSize < _2M)
    {
        /* Configure to use up to 512K of DMA */
        rxDescNum = 50;
        txDescNum = 50;
        auDescNum = 10;
        appDemoDbEntryAdd("rxDescNum", 50);
        appDemoDbEntryAdd("txDescNum", 50);
        appDemoDbEntryAdd("auDescNum", 10);
        appDemoDbEntryAdd("fuDescNum", 10);
    }
#endif /* LINUX_NOKM */

    rc = appDemoAllocateDmaMem(localPpPh2Config.deviceId, rxDescNum,
                               rxBufSize, RX_BUFF_ALLIGN_DEF,
                               txDescNum,
                               auDescNum, &localPpPh2Config);

    osMemCpy(phase2Params,&localPpPh2Config, sizeof(CPSS_PP_PHASE2_INIT_PARAMS));

    phase2Params->auMessageLength = CPSS_AU_MESSAGE_LENGTH_8_WORDS_E;

    return rc;
}

/**
* @internal getPpPhase2ConfigSimple function
* @endinternal
*
* @brief   Returns the configuration parameters for corePpHwPhase2Init().
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] oldDevNum                - The old Pp device number to get the parameters for.
* @param[in] devType                  - The Pp device type
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of getPpPhase2Config.
*
*/
static GT_STATUS getPpPhase2ConfigSimple
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       oldDevNum,
    IN  CPSS_PP_DEVICE_TYPE         devType,
    OUT CPSS_DXCH_PP_PHASE2_INIT_INFO_STC  *phase2Params
)
{
    GT_STATUS                   rc;
    GT_U32                      auDescNum;
    GT_U32                      i;

    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);
    osMemSet(phase2Params,0,sizeof(CPSS_DXCH_PP_PHASE2_INIT_INFO_STC));
    phase2Params->newDevNum                  = oldDevNum;


    phase2Params->fuqUseSeparate = GT_TRUE;
    phase2Params->netIfCfg.rxBufInfo.allocMethod = CPSS_RX_BUFF_STATIC_ALLOC_E;
    for(i=0;i<4;i++)
    {
        phase2Params->netIfCfg.rxBufInfo.bufferPercentage[i] = 13;
        phase2Params->netIfCfg.rxBufInfo.bufferPercentage[i+4] = 12;
    }
    phase2Params->netIfCfg.rxBufInfo.rxBufSize = RX_BUFF_SIZE_DEF;
    phase2Params->netIfCfg.rxBufInfo.headerOffset = 0;
    phase2Params->netIfCfg.rxBufInfo.buffersInCachedMem = GT_FALSE;
    phase2Params->netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockPtr = NULL;
    phase2Params->netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockSize = 16000;

    phase2Params->useSecondaryAuq = GT_FALSE;
    phase2Params->noTraffic2CPU = GT_FALSE;
    phase2Params->netifSdmaPortGroupId = 0;
    phase2Params->auMessageLength = CPSS_AU_MESSAGE_LENGTH_8_WORDS_E;
    phase2Params->useDoubleAuq = GT_FALSE;
    phase2Params->useMultiNetIfSdma = GT_FALSE;

    /* Single AUQ, since single FDB unit */
    auDescNum = AU_DESC_NUM_DEF;

    rc = allocateDmaMemSimple(devType, RX_DESC_NUM_DEF,
                               RX_BUFF_SIZE_DEF, RX_BUFF_ALLIGN_DEF,
                               TX_DESC_NUM_DEF,
                               auDescNum, phase2Params);

    CPSS_ENABLER_DBG_TRACE_RC_MAC("getPpPhase2ConfigSimple", rc);
    return rc;
}


/**
* @internal internalXGPortConfig function
* @endinternal
*
* @brief   Internal function performs all needed configurations of 10G ports.
*
* @param[in] devNum                   - Device Id.
*                                      portNum     - Port Number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS internalXGPortConfig
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  port_num
)
{

    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(devNum);
    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(port_num);

    return GT_OK;
}

/**
* @internal cascadePortConfig function
* @endinternal
*
* @brief   Additional configuration for cascading ports
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Cascade port number.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - HW failure
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS cascadePortConfig
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS       rc;

    /* set the MRU of the cascade port to be big enough for DSA tag */
    rc = cpssDxChPortMruSet(devNum, portNum, 1536);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortMruSet", rc);
    return rc;
}


/*--------------------------------------------------------------*
 * data base for autopolling                                    *
 *--------------------------------------------------------------*/
typedef struct
{
    CPSS_PHY_SMI_INTERFACE_ENT smiIf;
    GT_U32                     portNumMapped;
    GT_PHYSICAL_PORT_NUM       portNum[17];
    GT_U32                     smiAddr[17];
}APPDEMO_PORT_SMI_DEV_MAPPING_STC;


typedef struct
{
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT   sutoPollNumber   [CPSS_PHY_SMI_INTERFACE_MAX_E];
    APPDEMO_PORT_SMI_DEV_MAPPING_STC               port2SmiDevMapArr[CPSS_PHY_SMI_INTERFACE_MAX_E];
    GT_BOOL                                        isInbanAutNegEnable;
}APPDEMO_AUTOPOLL_BOARD_DEF_STC;

typedef struct
{
    GT_U32 boardType;
    APPDEMO_AUTOPOLL_BOARD_DEF_STC *portSmiDevMapping;
}APPDEMPO_AUTOPOLLING_SMI_CONFIG;


/* BUG at DB board of BC2: MAC 0-11 hard wired to SMI-2 instead of SMI-0, */
/* therefore autonegotiation via SMI will never work           */
/* RD board mac  0-15 wired to SMI 0           */
/*          mac 16-23       to SMI 1           */
/*          mac 24-39       to SMI 2           */
/*          mac 40-47       to SMI 3           */
/*-----------------------------------------------------------------------------------------------
 *           Serdes# MAC Port#   M_SMI#  Phy Address[H]         M_SMI#  Phy Address[H]
 *           ++++ DB Board ++++++++++BC2 +++++++++              ++  CAELUM  +++++++++++
 *               0   0 - 3           2   4 - 7                      0   0 - 3
 *               1   4 - 7           2   8 - B                      0   4 - 7
 *               2   8 - 11          2   C - F                      0   8 - B
 *               3   12 - 15         3   4 - 7                      1   4 - 7
 *               4   16 - 19         3   8 - B                      1   8 - B
 *               5   20 - 23         3   C - F                      1   C - F
 *               6   24 - 27         0   4 - 7                      2   0 - 3
 *               7   28 - 31         0   8 - B                      2   4 - 7
 *               8   32 - 35         0   C - F                      2   8 - B
 *               9   36 - 39         1   4 - 7                      3   4 - 7
 *               10  40 - 43         1   8 - B                      3   8 - B
 *               11  44 - 47         1   C - F                      3   C - F
 *           ++++ RD Board +++++++++++++++++++                  +++++++++++++++++++
 *               0   0 - 3           0   0 - 3
 *               1   4 - 7           0   4 - 7
 *               2   8 - 11          0   8 - B
 *               3   12 - 15         0   C - F
 *               4   16 - 19         1   0x10 - 0x13
 *               5   20 - 23         1   0x14 - 0x17
 *               6   24 - 27         2   0 - 3
 *               7   28 - 31         2   4 - 7
 *               8   32 - 35         2   8 - B
 *               9   36 - 39         2   C - F
 *               10  40 - 43         3   0x10 - 0x13
 *               11  44 - 47         3   0x14 - 0x17
 *
 *
 *---------------------------------------------------------------------------------------------------*/


APPDEMO_AUTOPOLL_BOARD_DEF_STC board_bc2_DB =
{
     {
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E
     }
    ,{
        {
             CPSS_PHY_SMI_INTERFACE_0_E
            ,12
            ,{  24,  25,  26,  27,  28, 29,   30,  31,  32,  33,  34,  35, APPDEMO_BAD_VALUE }
            ,{ 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_1_E
            ,12
            ,{  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47, APPDEMO_BAD_VALUE }
            ,{ 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_2_E
            ,12
            ,{   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11, APPDEMO_BAD_VALUE }
            ,{ 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_3_E
            ,12
            ,{  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23, APPDEMO_BAD_VALUE }
            ,{ 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, APPDEMO_BAD_VALUE }
        }
     }
    ,GT_TRUE /* enable inband polling */
};

APPDEMO_AUTOPOLL_BOARD_DEF_STC board_bc2_RD =
{
     {
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E
     }
    ,{
        {
             CPSS_PHY_SMI_INTERFACE_0_E
            ,16
            ,{   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15, APPDEMO_BAD_VALUE }
            ,{ 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_1_E
            ,8
            ,{   16,   17,   18,   19,   20,   21,   22,   23, APPDEMO_BAD_VALUE }
            ,{ 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_2_E
            ,16
            ,{   24, 25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39, APPDEMO_BAD_VALUE }
            ,{ 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_3_E
            ,8
            ,{   40,   41,   42,   43,   44,   45,   46,   47, APPDEMO_BAD_VALUE }
            ,{ 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, APPDEMO_BAD_VALUE }
        }
     }
    ,GT_FALSE /* don't enable inband polling */
};

APPDEMO_AUTOPOLL_BOARD_DEF_STC board_bobk_caelum_DB =
{
     {
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E
     }
    ,{
        {
             CPSS_PHY_SMI_INTERFACE_0_E
            ,12
            ,{   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11, APPDEMO_BAD_VALUE }
            ,{ 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_1_E
            ,12
            ,{  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23, APPDEMO_BAD_VALUE }
            ,{ 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_2_E
            ,12
            ,{  24,  25,  26,  27,  28, 29,   30,  31,  32,  33,  34,  35, APPDEMO_BAD_VALUE }
            ,{ 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_3_E
            ,12
            ,{  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47, APPDEMO_BAD_VALUE }
            ,{ 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, APPDEMO_BAD_VALUE }
        }
     }
    ,GT_FALSE /* don't enable inband polling */
};

APPDEMO_AUTOPOLL_BOARD_DEF_STC board_bobk_caelum_cygnus_RD =
{
     {
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E,
         CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E
     }
    ,{
        {
             CPSS_PHY_SMI_INTERFACE_0_E
            ,16
            ,{   0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15, APPDEMO_BAD_VALUE }
            ,{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_1_E
            ,8
            ,{  16,   17,   18,   19,   20,   21,   22,   23, APPDEMO_BAD_VALUE }
            ,{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_2_E
            ,16
            ,{  24,   25,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39, APPDEMO_BAD_VALUE }
            ,{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, APPDEMO_BAD_VALUE }
        }
       ,{
             CPSS_PHY_SMI_INTERFACE_3_E
            ,8
            ,{  40,   41,   42,   43,   44,   45,   46,   47,  APPDEMO_BAD_VALUE }
            ,{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, APPDEMO_BAD_VALUE }
        }
     }
    ,GT_FALSE /* don't enable inband polling */
};


APPDEMPO_AUTOPOLLING_SMI_CONFIG boardAutoPollingConfArr[] =
{
       { APP_DEMO_BC2_BOARD_DB_CNS,      &board_bc2_DB                             }
      ,{ APP_DEMO_BC2_BOARD_RD_MSI_CNS,  &board_bc2_RD                             }
      ,{ APP_DEMO_BC2_BOARD_RD_MTL_CNS,  &board_bc2_RD                             }
      ,{ APP_DEMO_CAELUM_BOARD_DB_CNS,   &board_bobk_caelum_DB                     }
      ,{ APP_DEMO_CETUS_BOARD_DB_CNS,    (APPDEMO_AUTOPOLL_BOARD_DEF_STC*)NULL     }
      /* Lewis RD has only 10G PHY 88x3240 no GE PHY */
      ,{ APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS, (APPDEMO_AUTOPOLL_BOARD_DEF_STC*)NULL  }
      ,{ APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS, &board_bobk_caelum_cygnus_RD      }
      ,{ APP_DEMO_ALDRIN_BOARD_DB_CNS,      (APPDEMO_AUTOPOLL_BOARD_DEF_STC*)NULL  }
      ,{ APP_DEMO_BOBCAT3_BOARD_DB_CNS,     (APPDEMO_AUTOPOLL_BOARD_DEF_STC*)NULL  }
      ,{ APP_DEMO_ALDRIN_BOARD_RD_CNS,   (APPDEMO_AUTOPOLL_BOARD_DEF_STC*)NULL     }

      ,{ APP_DEMO_XCAT3X_A0_BOARD_DB_CNS,   (APPDEMO_AUTOPOLL_BOARD_DEF_STC*)NULL     }
      ,{ APP_DEMO_XCAT3X_Z0_BOARD_DB_CNS,   (APPDEMO_AUTOPOLL_BOARD_DEF_STC*)NULL     }
      ,{ APP_DEMO_BOBCAT3_BOARD_RD_CNS,     (APPDEMO_AUTOPOLL_BOARD_DEF_STC*)NULL     }
      ,{ APP_DEMO_ARMSTRONG_BOARD_RD_CNS,   (APPDEMO_AUTOPOLL_BOARD_DEF_STC*)NULL     }

      ,{ APP_DEMO_ALDRIN2_BOARD_RD_CNS,     (APPDEMO_AUTOPOLL_BOARD_DEF_STC*)NULL     }

      ,{ APPDEMO_BAD_VALUE,              (APPDEMO_AUTOPOLL_BOARD_DEF_STC*)NULL     }
};


GT_STATUS boardAutopollingInfoGet
(
    IN  GT_U32 boardType,
    OUT GT_BOOL                         *isFoundPtr,
    OUT APPDEMO_AUTOPOLL_BOARD_DEF_STC **autoPollingInfoPtrPtr
)
{
    GT_U32 i;

    for (i = 0 ; boardAutoPollingConfArr[i].boardType !=  APPDEMO_BAD_VALUE; i++)
    {
        if ( boardAutoPollingConfArr[i].boardType == boardType)
        {
            *isFoundPtr = GT_TRUE;
            *autoPollingInfoPtrPtr = boardAutoPollingConfArr[i].portSmiDevMapping;
            return GT_OK;
        }
    }
    *isFoundPtr = GT_FALSE;
    *autoPollingInfoPtrPtr = (APPDEMO_AUTOPOLL_BOARD_DEF_STC *)NULL;
    return GT_OK;;
}


GT_STATUS bobKOobPortSmiConfig
(
    IN  GT_U8 dev,
    IN  GT_U32 boardType
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM port;
    GT_U8 phyAddr;
    CPSS_PHY_SMI_INTERFACE_ENT smiIf;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;

    switch(boardType)
    {
        case APP_DEMO_CAELUM_BOARD_DB_CNS:
            port = 90;
            phyAddr = 0;
            smiIf = CPSS_PHY_SMI_INTERFACE_1_E;
            break;

        case APP_DEMO_CETUS_BOARD_DB_CNS:
            port = 90;
            phyAddr = 0;
            smiIf = CPSS_PHY_SMI_INTERFACE_0_E;
            break;

        case APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS:
            port = 90;
            phyAddr = 0;
            smiIf = CPSS_PHY_SMI_INTERFACE_0_E;
            break;

        case APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS:
            port = 90;
            phyAddr = 0;
            smiIf = CPSS_PHY_SMI_INTERFACE_3_E;
            break;

        default:
            return GT_OK;
    }

    rc = cpssDxChPortPhysicalPortDetailedMapGet(dev, port, &portMap);
    if((rc != GT_OK) || (portMap.valid != GT_TRUE))
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("Please not that SMI init for OOB port was not done. Please add it (port with MAC[62]) to port mapping table\n", rc);
        return GT_OK;
    }

    if(portMap.portMap.macNum != 62)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortDetailedMapGet - please use port number from map table that mapped to MAC_62\n", GT_BAD_PARAM);
        return GT_BAD_PARAM;
    }

    rc = cpssDxChPhyPortAddrSet(dev, port, phyAddr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortAddrSet - please update map table for OOB port\n", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* configure SMI interface */
    rc = cpssDxChPhyPortSmiInterfaceSet(dev, port, smiIf);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet - please update map table for OOB port", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

GT_STATUS boardAutoPollingConfigure
(
    IN  GT_U8   dev,
    IN  GT_U32  boardType
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM  port;   /* port number */
    GT_U8                 phyAddr;/* PHY address */
    APPDEMO_AUTOPOLL_BOARD_DEF_STC * autoPollingInfoPtr;
    GT_BOOL                          isFound;
    GT_U32                           smiIfIdx;
    CPSS_PHY_SMI_INTERFACE_ENT       smiIf;
    GT_U32                           portIdx;

    rc = boardAutopollingInfoGet(bc2BoardType, /*OUT*/&isFound,&autoPollingInfoPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("boardAutopollingInfoGet", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (isFound == GT_FALSE)
    {
        cpssOsPrintf("\n-->ERROR : boardAutoPollingConfigure() Auto Polling Config: board =%d is not supported\n", bc2BoardType);
        return GT_NOT_SUPPORTED;
    }
    if (autoPollingInfoPtr != NULL)
    {
        rc = cpssDxChPhyAutoPollNumOfPortsSet(dev,
                                                autoPollingInfoPtr->sutoPollNumber[0],
                                                autoPollingInfoPtr->sutoPollNumber[1],
                                                autoPollingInfoPtr->sutoPollNumber[2],
                                                autoPollingInfoPtr->sutoPollNumber[3]);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyAutoPollNumOfPortsSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
        for (smiIfIdx = 0 ; smiIfIdx < CPSS_PHY_SMI_INTERFACE_MAX_E; smiIfIdx++)
        {
            smiIf = autoPollingInfoPtr->port2SmiDevMapArr[smiIfIdx].smiIf;
            for (portIdx = 0 ; autoPollingInfoPtr->port2SmiDevMapArr[smiIfIdx].portNum[portIdx] != APPDEMO_BAD_VALUE; portIdx++)
            {
                port    = autoPollingInfoPtr->port2SmiDevMapArr[smiIfIdx].portNum[portIdx];
                phyAddr = (GT_U8)autoPollingInfoPtr->port2SmiDevMapArr[smiIfIdx].smiAddr[portIdx];

                CPSS_ENABLER_PORT_SKIP_CHECK(dev,port);
                /* configure PHY address */

                /*cpssOsPrintf("\nPort %3d smiIf = %d phyAddr = 0x%x",port,smiIf,phyAddr);  */
                rc = cpssDxChPhyPortAddrSet(dev, port, phyAddr);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortAddrSet", rc);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* configure SMI interface */
                rc = cpssDxChPhyPortSmiInterfaceSet(dev, port, smiIf);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", rc);
                if(rc != GT_OK)
                {
                    return rc;
                }
                if (autoPollingInfoPtr->isInbanAutNegEnable == GT_TRUE)
                {
                    /* enable in-band auto-negotiation because out of band cannot
                        work on this DB board */
                    rc = cpssDxChPortInbandAutoNegEnableSet(dev, port, GT_TRUE);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortInbandAutoNegEnableSet", rc);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }

    return GT_OK;
}


/**
* @internal gtAppDemoXPhyFwDownload function
* @endinternal
*
* @brief   This function performs all needed configurations that should be done
*         after phase2.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
extern GT_STATUS gtAppDemoXPhyFwDownload
(
    IN  GT_U8   devNum
);

static GT_STATUS configBoardAfterPhase2
(
    IN  GT_U8   boardRevId
)
{
    GT_U8                 dev;
    GT_STATUS             rc = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    CPSS_SYSTEM_RECOVERY_INFO_STC saved_system_recovery;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    saved_system_recovery = system_recovery;

    for(dev = SYSTEM_DEV_NUM_MAC(0); dev < SYSTEM_DEV_NUM_MAC(ppCounter); dev++)
    {
        appDemoPpConfigList[dev].internal10GPortConfigFuncPtr = internalXGPortConfig;
        appDemoPpConfigList[dev].internalCscdPortConfigFuncPtr = cascadePortConfig;

#ifndef ASIC_SIMULATION

        /******************************************************/
                        CPSS_TBD_BOOKMARK_BOBCAT3
        /* BC3 board got stuck - code should be checked later */
        /******************************************************/
        if(system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {
            if(appDemoPpConfigList[dev].devFamily != CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
            {
                rc = gtAppDemoXPhyFwDownload(dev);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("gtAppDemoXPhyFwDownload", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
#endif
        pDev = PRV_CPSS_DXCH_PP_MAC(dev);

        /* Aldrin does not have OOB PHY auto polling */
        if ((pDev->genInfo.devFamily != CPSS_PP_FAMILY_DXCH_ALDRIN_E)  &&
            (pDev->genInfo.devFamily != CPSS_PP_FAMILY_DXCH_ALDRIN2_E) &&
            (pDev->genInfo.devFamily != CPSS_PP_FAMILY_DXCH_AC3X_E))
        {
            /* configure QSGMII ports PHY related mappings */
            if (   boardRevId == BOARD_REV_ID_DB_E     || boardRevId == BOARD_REV_ID_DB_TM_E
                || boardRevId == BOARD_REV_ID_RDMTL_E  || boardRevId == BOARD_REV_ID_RDMTL_TM_E
                || boardRevId == BOARD_REV_ID_RDMSI_E  || boardRevId == BOARD_REV_ID_RDMSI_TM_E)
            {
                if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
                {
                    /* if FastBoot is underway the traffic shouldn't be altered */
                    saved_system_recovery = system_recovery;
                    system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
                    system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
                    rc =  cpssSystemRecoveryStateSet(&system_recovery);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }

                /* OOB port SMI configuration */
                rc = bobKOobPortSmiConfig(dev, bc2BoardType);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("bobKOobPortSmiConfig", rc);
                if(rc != GT_OK)
                {
                    rc =  cpssSystemRecoveryStateSet(&saved_system_recovery);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    return rc;
                }

                rc = boardAutoPollingConfigure(dev,bc2BoardType);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("boardAutoPollingConfigure", rc);
                if (rc != GT_OK)
                {
                    rc =  cpssSystemRecoveryStateSet(&saved_system_recovery);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    return rc;
                }
                rc =  cpssSystemRecoveryStateSet(&saved_system_recovery);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                cpssOsPrintf("\n-->ERROR : configBoardAfterPhase2() PHY config : rev =%d is not supported", boardRevId);
                return GT_NOT_SUPPORTED;
            }
        }

        /* Init LED interfaces */
        rc = appDemoLedInterfacesInit(dev,bc2BoardType);
                if (rc != GT_OK)
                {
                        return rc;
                }
        return rc;
    }

    return GT_OK;
}

/**
* @internal configBoardAfterPhase2Simple function
* @endinternal
*
* @brief   This function performs all needed configurations that should be done
*         after phase2.
* @param[in] boardRevId               - The board revision Id.
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success,\
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of configBoardAfterPhase2.
*
*/

static GT_STATUS configBoardAfterPhase2Simple
(
    IN  GT_U8   boardRevId,
    GT_U8       devNum
)
{
    GT_PHYSICAL_PORT_NUM  port;   /* port number */
    GT_STATUS             rc;

    for(port = 0; port < CPSS_MAX_PORTS_NUM_CNS;port++)
    {
        if( !(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&PRV_CPSS_PP_MAC(devNum)->existingPorts, port)))
        {
            continue;
        }

        /* split ports between MC FIFOs for Multicast arbiter */
        rc = cpssDxChPortTxMcFifoSet(devNum, port, port%2);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortTxMcFifoSet", rc);
        if( GT_OK != rc )
        {
            return rc;
        }
    }

    /* configure QSGMII ports PHY related mappings */
    if (   boardRevId == BOARD_REV_ID_DB_E     || boardRevId == BOARD_REV_ID_DB_TM_E
        || boardRevId == BOARD_REV_ID_RDMTL_E  || boardRevId == BOARD_REV_ID_RDMTL_TM_E
        || boardRevId == BOARD_REV_ID_RDMSI_E  || boardRevId == BOARD_REV_ID_RDMSI_TM_E)
    {
        rc = boardAutoPollingConfigure(devNum,bc2BoardType);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        cpssOsPrintf("\n-->ERROR : configBoardAfterPhase2() PHY config : rev =%d is not supported", boardRevId);
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}


/**
* @internal appDemoDbBlocksAllocationMethodGet function
* @endinternal
*
* @brief   Get the blocks allocation method configured in the Init System
*/
static GT_STATUS appDemoDbBlocksAllocationMethodGet
(
    IN  GT_U8                                               dev,
    OUT CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT  *blocksAllocationMethodGet
)
{
    GT_U32 value;

    if(appDemoDbEntryGet("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", &value) == GT_OK)
        *blocksAllocationMethodGet = (CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT)value;
    else
        *blocksAllocationMethodGet = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;

    return GT_OK;
}

/**
* @internal appDemoBc2IpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoBc2IpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
)
{
    GT_U32 i=0;
    GT_STATUS rc = GT_OK;
    GT_U32 blockSizeInBytes;
    GT_U32 blockSizeInLines;
    GT_U32 lastBlockSizeInLines;
    GT_U32 lastBlockSizeInBytes;
    GT_U32 lpmRamNumOfLines;
    GT_U32 numOfPbrBlocks;
    GT_U32 maxNumOfPbrEntriesToUse;
    GT_U32 value;

    if(appDemoDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        maxNumOfPbrEntriesToUse = value;
    else
        maxNumOfPbrEntriesToUse = maxNumOfPbrEntries;

    /*relevant for BC3 only*/
    if(appDemoDbEntryGet("lpmMemMode", &value) == GT_OK)
        ramDbCfgPtr->lpmMemMode = value?CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E:CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    else
        ramDbCfgPtr->lpmMemMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;

    lpmRamNumOfLines = PRV_CPSS_DXCH_LPM_RAM_GET_NUM_OF_LINES_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam);

    /*if we are working in half memory mode - then do all the calculations for half size,return to the real values later in the code*/
    if(ramDbCfgPtr->lpmMemMode == CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        lpmRamNumOfLines/=2;
    }

    blockSizeInLines = (lpmRamNumOfLines/APP_DEMO_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS);
    if (blockSizeInLines==0)
    {
        /* can not create a shadow with the current lpmRam size */
        return GT_FAIL;
    }
    blockSizeInBytes = blockSizeInLines * 4;

    if (maxNumOfPbrEntriesToUse >= lpmRamNumOfLines)
    {
        /* No memory for Ip LPM */
        return GT_FAIL;
    }

    if(maxNumOfPbrEntriesToUse > blockSizeInLines)
    {
        numOfPbrBlocks = (maxNumOfPbrEntriesToUse + blockSizeInLines - 1) / blockSizeInLines;
        lastBlockSizeInLines = (numOfPbrBlocks*blockSizeInLines)-maxNumOfPbrEntriesToUse;
        if (lastBlockSizeInLines==0)/* PBR will fit exactly in numOfPbrBlocks */
        {
            ramDbCfgPtr->numOfBlocks = APP_DEMO_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS - numOfPbrBlocks;
            lastBlockSizeInLines = blockSizeInLines; /* all of last block for IP LPM */
        }
        else/* PBR will not fit exactly in numOfPbrBlocks and we will have in the last block LPM lines together with PBR lines*/
        {
            ramDbCfgPtr->numOfBlocks = APP_DEMO_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS - numOfPbrBlocks + 1;
        }
    }
    else
    {
        if (maxNumOfPbrEntriesToUse == blockSizeInLines)
        {
            ramDbCfgPtr->numOfBlocks = APP_DEMO_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS-1;
            lastBlockSizeInLines = blockSizeInLines;
        }
        else
        {
            ramDbCfgPtr->numOfBlocks = APP_DEMO_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS;
            lastBlockSizeInLines = blockSizeInLines - maxNumOfPbrEntriesToUse;
        }
    }

    /* number of LPM bytes ONLY when last block is shared between LPM and PBR */
    lastBlockSizeInBytes = lastBlockSizeInLines * 4;

    /*ram configuration should contain physical block sizes,we divided block sizes for a calcultion earlier in the code,
            so now return to real values*/

    if(ramDbCfgPtr->lpmMemMode == CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        blockSizeInBytes*=2;
        lastBlockSizeInBytes*=2;
    }

    for (i=0;i<ramDbCfgPtr->numOfBlocks-1;i++)
    {
        ramDbCfgPtr->blocksSizeArray[i] = blockSizeInBytes;
    }

    ramDbCfgPtr->blocksSizeArray[ramDbCfgPtr->numOfBlocks-1] =
        lastBlockSizeInBytes == 0 ?
            blockSizeInBytes :   /* last block is fully LPM (not PBR) */
            lastBlockSizeInBytes;/* last block uses 'x' for LPM , rest for PBR */

    /* reset other sections */
    i = ramDbCfgPtr->numOfBlocks;
    for (/*continue i*/;i<APP_DEMO_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS;i++)
    {
        ramDbCfgPtr->blocksSizeArray[i] = 0;
    }

    rc = appDemoDbBlocksAllocationMethodGet(devNum,&ramDbCfgPtr->blocksAllocationMethod);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal appDemoBc2IpLpmRamDefaultConfigCalc_elements function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*         and return the elements in the struct
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] numOfBlocksPtr           - (pointe to)the number of RAM blocks that LPM uses.
* @param[out] blocksSizeArray[]        - array that holds the sizes of the RAM blocks in bytes
* @param[out] blocksAllocationMethodPtr - (pointe to)the method of blocks allocation
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_VALUE             - on bad value.
*/
GT_STATUS appDemoBc2IpLpmRamDefaultConfigCalc_elements
(
    IN  GT_U8       devNum,
    IN  GT_U32      maxNumOfPbrEntries,
    OUT GT_U32      *numOfBlocksPtr,
    OUT GT_U32      blocksSizeArray[],
    OUT GT_U32      *blocksAllocationMethodPtr,
    OUT CPSS_DXCH_LPM_RAM_MEM_MODE_ENT      *lpmMemModePtr
)
{
    GT_U32 i=0;
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_LPM_RAM_CONFIG_STC ramDbCfg;

    cpssOsMemSet(&ramDbCfg,0,sizeof(CPSS_DXCH_LPM_RAM_CONFIG_STC));

    rc =appDemoBc2IpLpmRamDefaultConfigCalc(devNum,maxNumOfPbrEntries,&ramDbCfg);
    if (rc!=GT_OK)
    {
        return rc;
    }

    *numOfBlocksPtr = ramDbCfg.numOfBlocks;
    for (i=0;i<ramDbCfg.numOfBlocks;i++)
    {
        blocksSizeArray[i]=ramDbCfg.blocksSizeArray[i];
    }
    switch (ramDbCfg.blocksAllocationMethod)
    {
        case CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E:
            *blocksAllocationMethodPtr=0;
            break;
        case CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E:
            *blocksAllocationMethodPtr=1;
            break;
        default:
            return GT_BAD_VALUE;
    }

    *lpmMemModePtr = ramDbCfg.lpmMemMode;
    return rc;
}

/**
* @internal getPpLogicalInitParams function
* @endinternal
*
* @brief   Returns the parameters needed for sysPpLogicalInit() function.
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devNum                   - The Pp device number to get the parameters for.
*
* @param[out] ppLogInitParams          - Pp logical init parameters struct.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpLogicalInitParams
(
    IN  GT_U8           boardRevId,
    IN  GT_U8           devNum,
    OUT CPSS_PP_CONFIG_INIT_STC    *ppLogInitParams
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PP_CONFIG_INIT_STC  localPpCfgParams = PP_LOGICAL_CONFIG_DEFAULT;

    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);

    localPpCfgParams.numOfTrunks = _8K-1;
    localPpCfgParams.maxNumOfPbrEntries = _8K;

    if(appDemoPpConfigList[devNum].deviceId == CPSS_98DXH831_CNS ||
       appDemoPpConfigList[devNum].deviceId == CPSS_98DXH832_CNS ||
       appDemoPpConfigList[devNum].deviceId == CPSS_98DXH833_CNS)
    {
        localPpCfgParams.maxNumOfPbrEntries = _2K;
    }

    rc = appDemoBc2IpLpmRamDefaultConfigCalc(devNum,localPpCfgParams.maxNumOfPbrEntries,&(localPpCfgParams.lpmRamMemoryBlocksCfg));
    if (rc != GT_OK)
    {
        return rc;
    }
    osMemCpy(ppLogInitParams, &localPpCfgParams, sizeof(CPSS_PP_CONFIG_INIT_STC));

    return rc;
}

/**
* @internal getPpLogicalInitParamsSimple function
* @endinternal
*
* @brief   Returns the parameters needed for sysPpLogicalInit() function.
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devNum                   - The Pp device number to get the parameters for.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of getPpLogicalInitParams.
*
*/

static GT_STATUS getPpLogicalInitParamsSimple
(
    IN  GT_U8           boardRevId,
    IN  GT_U8           devNum,
    OUT CPSS_DXCH_PP_CONFIG_INIT_STC    *localPpCfgParams
)
{

    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);
    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(devNum);


    localPpCfgParams->routingMode  = CPSS_DXCH_TCAM_ROUTER_BASED_E;
    localPpCfgParams->maxNumOfPbrEntries = _8K;


    return GT_OK;
}


/**
* @internal getTapiLibInitParams function
* @endinternal
*
* @brief   Returns Tapi library initialization parameters.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] libInitParams            - Library initialization parameters.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getTapiLibInitParams
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devNum,
    OUT APP_DEMO_LIB_INIT_PARAMS    *libInitParams
)
{
    APP_DEMO_LIB_INIT_PARAMS  localLibInitParams = LIB_INIT_PARAMS_DEFAULT;

    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);
    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(devNum);

    localLibInitParams.initClassifier            = GT_FALSE;
    localLibInitParams.initIpv6                  = GT_TRUE;
    localLibInitParams.initIpv4                  = GT_TRUE;

    localLibInitParams.initIpv4Filter            = GT_FALSE;
    localLibInitParams.initIpv4Tunnel            = GT_FALSE;
    localLibInitParams.initMpls                  = GT_FALSE;
    localLibInitParams.initMplsTunnel            = GT_FALSE;
    localLibInitParams.initPcl                   = GT_TRUE;
    localLibInitParams.initTcam                  = GT_TRUE;

    /* there is no Policer lib init for Bobcat2 devices */
    localLibInitParams.initPolicer               = GT_FALSE;

    osMemCpy(libInitParams,&localLibInitParams,
             sizeof(APP_DEMO_LIB_INIT_PARAMS));
    return GT_OK;
}


/*******************************************************************************
* bcat2PortInterfaceInit
*
*******************************************************************************/
static PortInitList_STC portInitlist_29_100_48x1G_QXGMII_7x10G_KR[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0,47,1,     APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E  }
    ,{ PORT_LIST_TYPE_LIST,      {49,50,51,    APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,      {57,58,80,82, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {             APP_INV_PORT_CNS },  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};
/* next port list is 'theoretical' as the BW is larger then actual device supports.
   but the devices that should use it hold actual less MACs (ports).

   like : Cetus  : 0  GE ports , 12 XG ports.
          Caelum : 47 GE ports , 12 XG ports.

*/
static PortInitList_STC portInitlist_AllPorts[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0,47,1,  /* 0..47*/     APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E  }
    ,{ PORT_LIST_TYPE_INTERVAL,  {48,59,1,  /*48..59*/     APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,71,1,  /*64..71*/     APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,      {80,81,82, /*80,81,82*/   APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {             APP_INV_PORT_CNS },  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};

static PortInitList_STC portInitlist_29_1_16x1G_QXGMII_8x10G_KR[] =
{
     { PORT_LIST_TYPE_INTERVAL,  { 0,15,1,      APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E     }
    ,{ PORT_LIST_TYPE_INTERVAL,  { 24,31,1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {              APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
};




static GT_U32   convertSpeedToGig[CPSS_PORT_SPEED_NA_E] =
{
    /*CPSS_PORT_SPEED_10_E,     */ 1 /* 0 */     ,
    /*CPSS_PORT_SPEED_100_E,    */ 1 /* 1 */     ,
    /*CPSS_PORT_SPEED_1000_E,   */ 1 /* 2 */     ,
    /*CPSS_PORT_SPEED_10000_E,  */ 10 /* 3 */    ,
    /*CPSS_PORT_SPEED_12000_E,  */ 12 /* 4 */    ,
    /*CPSS_PORT_SPEED_2500_E,   */ 3 /* 5 */     ,
    /*CPSS_PORT_SPEED_5000_E,   */ 5 /* 6 */     ,
    /*CPSS_PORT_SPEED_13600_E,  */ 14 /* 7 */    ,
    /*CPSS_PORT_SPEED_20000_E,  */ 20 /* 8 */    ,
    /*CPSS_PORT_SPEED_40000_E,  */ 40 /* 9 */    ,
    /*CPSS_PORT_SPEED_16000_E,  */ 16 /* 10 */   ,
    /*CPSS_PORT_SPEED_15000_E,  */ 15 /* 11 */   ,
    /*CPSS_PORT_SPEED_75000_E,  */ 75 /* 12 */   ,
    /*CPSS_PORT_SPEED_100G_E,   */ 100 /* 13 */  ,
    /*CPSS_PORT_SPEED_50000_E,  */ 50 /* 14 */   ,
    /*CPSS_PORT_SPEED_140G_E,   */ 140 /* 15 */  ,

    /*CPSS_PORT_SPEED_11800_E,  */ 12 /* 16  */ ,/*used in combination with CPSS_PORT_INTERFACE_MODE_XHGS_E */
    /*CPSS_PORT_SPEED_47200_E,  */ 48 /* 17  */ ,/*used in combination with CPSS_PORT_INTERFACE_MODE_XHGS_E */
};
/* function to summarize the number of GIG needed for the port list */
static GT_U32  getNumGigBw(
    IN GT_U8             dev,
    IN PortInitList_STC * portInitPtr
)
{
    GT_U32  i;
    GT_U32  portNum,portIdx;
    GT_U32  summary = 1;/* 1G needed for the CPU SDMA port */

    for (i = 0 ; portInitPtr->entryType != PORT_LIST_TYPE_EMPTY; i++,portInitPtr++)
    {
        switch (portInitPtr->entryType)
        {
            case PORT_LIST_TYPE_INTERVAL:
                for (portNum = portInitPtr->portList[0] ; portNum <= portInitPtr->portList[1]; portNum += portInitPtr->portList[2])
                {
                    CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);

                    summary += convertSpeedToGig[portInitPtr->speed];
                }
                break;
            case PORT_LIST_TYPE_LIST:
                for (portIdx = 0 ; portInitPtr->portList[portIdx] != APP_INV_PORT_CNS; portIdx++)
                {
                    portNum = portInitPtr->portList[portIdx];
                    CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);

                    summary += convertSpeedToGig[portInitPtr->speed];
                }
                break;
            default:
                break;
        }
    }

    return summary;
}

typedef struct
{
    GT_PHYSICAL_PORT_NUM                    portNum;
    CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC     avagoSerdesTxTuneParams;
}APPDEMO_BC2_SERDES_TX_CONFIG_STC;

static APPDEMO_SERDES_LANE_POLARITY_STC  aldrinA0_DB_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 1,    GT_TRUE,    GT_TRUE  },
    { 2,    GT_TRUE,    GT_FALSE },
    { 3,    GT_TRUE,    GT_TRUE  },
    { 4,    GT_TRUE,    GT_FALSE },
    { 5,    GT_TRUE,    GT_TRUE  },
    { 6,    GT_TRUE,    GT_FALSE },
    { 7,    GT_FALSE,   GT_TRUE  },
    { 9,    GT_TRUE,    GT_FALSE },
    { 13,   GT_TRUE,    GT_FALSE },
    { 14,   GT_TRUE,    GT_FALSE },
    { 16,   GT_TRUE,    GT_FALSE },
    { 17,   GT_TRUE,    GT_FALSE },
    { 22,   GT_TRUE,    GT_FALSE },
    { 24,   GT_TRUE,    GT_FALSE },
    { 25,   GT_TRUE,    GT_FALSE },
    { 27,   GT_TRUE,    GT_FALSE },
    { 29,   GT_TRUE,    GT_FALSE }
};

static APPDEMO_SERDES_LANE_POLARITY_STC  aldrinA0_RD_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,     GT_TRUE,    GT_FALSE  },
    { 7,     GT_TRUE,    GT_FALSE  },
    { 8,     GT_FALSE,    GT_TRUE  },
    { 11,    GT_FALSE,    GT_TRUE },
    { 12,    GT_FALSE,    GT_TRUE  },
    { 15,    GT_FALSE,    GT_TRUE },
    { 16,    GT_FALSE,    GT_TRUE  },
    { 19,    GT_FALSE,    GT_TRUE },
    { 20,    GT_FALSE,    GT_TRUE  },
    { 23,    GT_FALSE,    GT_TRUE },
    { 24,    GT_FALSE,    GT_TRUE  },
    { 25,    GT_FALSE,    GT_TRUE },
    { 26,    GT_FALSE,    GT_TRUE },
    { 29,    GT_FALSE,    GT_TRUE },
    { 30,    GT_FALSE,    GT_TRUE }
};

static APPDEMO_SERDES_LANE_POLARITY_STC  aldrinZ0_DB_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 2,    GT_TRUE,    GT_FALSE },
    { 4,    GT_TRUE,    GT_FALSE },
    { 6,    GT_TRUE,    GT_FALSE },
    { 7,    GT_TRUE,    GT_FALSE },
    { 9,    GT_TRUE,    GT_FALSE },
    { 26,   GT_TRUE,    GT_FALSE },
    { 28,   GT_FALSE,   GT_TRUE  },
    { 29,   GT_TRUE,    GT_FALSE },
    { 30,   GT_TRUE,    GT_TRUE  },
    { 31,   GT_TRUE,    GT_FALSE },
    { 32,   GT_TRUE,    GT_TRUE  },
    { 33,   GT_TRUE,    GT_FALSE },
    { 34,   GT_TRUE,    GT_TRUE  }
};

static APPDEMO_SERDES_LANE_POLARITY_STC  cetus_LewisRD_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 24,   GT_TRUE,    GT_TRUE  },
    { 25,   GT_TRUE,    GT_FALSE },
    { 28,   GT_FALSE,   GT_TRUE  },
    { 29,   GT_TRUE,    GT_FALSE },
    { 30,   GT_FALSE,   GT_TRUE  },
    { 32,   GT_FALSE,   GT_TRUE  },
    { 33,   GT_TRUE,    GT_FALSE },
    { 34,   GT_FALSE,   GT_TRUE  },
    { 35,   GT_TRUE,    GT_FALSE }
};

static APPDEMO_SERDES_LANE_POLARITY_STC  caelum_CygnusRD_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 24,   GT_TRUE,    GT_TRUE  },
    { 25,   GT_FALSE,   GT_TRUE  },
    { 26,   GT_TRUE,    GT_TRUE  },
    { 27,   GT_FALSE,   GT_TRUE  },
    { 28,   GT_TRUE,    GT_TRUE  },
    { 30,   GT_TRUE,    GT_FALSE },
    { 32,   GT_TRUE,    GT_TRUE  },
    { 33,   GT_TRUE,    GT_TRUE  },
    { 34,   GT_FALSE,   GT_TRUE  },
    { 35,   GT_TRUE,    GT_TRUE  }
};

static APPDEMO_SERDES_LANE_POLARITY_STC  bobcat3_DB_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_FALSE,   GT_TRUE  },
    { 1,    GT_FALSE,   GT_TRUE  },
    { 2,    GT_FALSE,   GT_TRUE  },
    { 5,    GT_FALSE,   GT_TRUE  },
    { 6,    GT_FALSE,   GT_TRUE  },
    { 7,    GT_TRUE,    GT_FALSE },
    { 8,    GT_FALSE,   GT_TRUE  },
    { 9,    GT_FALSE,   GT_TRUE  },
    { 10,   GT_FALSE,   GT_TRUE  },
    { 13,   GT_FALSE,   GT_TRUE  },
    { 14,   GT_FALSE,   GT_TRUE  },
    { 15,   GT_TRUE,    GT_FALSE },
    { 16,   GT_TRUE,    GT_TRUE  },
    { 17,   GT_FALSE,   GT_TRUE  },
    { 18,   GT_FALSE,   GT_TRUE  },
    { 20,   GT_TRUE,    GT_FALSE },
    { 21,   GT_FALSE,   GT_TRUE  },
    { 22,   GT_TRUE,    GT_TRUE  },
    { 23,   GT_TRUE,    GT_FALSE },
    { 24,   GT_FALSE,   GT_TRUE  },
    { 25,   GT_FALSE,   GT_TRUE  },
    { 26,   GT_TRUE,    GT_TRUE  },
    { 27,   GT_TRUE,    GT_TRUE  },
    { 28,   GT_FALSE,   GT_TRUE  },
    { 29,   GT_FALSE,   GT_TRUE  },
    { 30,   GT_FALSE,   GT_TRUE  },
    { 31,   GT_TRUE,    GT_TRUE  },
    { 32,   GT_FALSE,   GT_TRUE  },
    { 33,   GT_TRUE,    GT_TRUE  },
    { 34,   GT_FALSE,   GT_TRUE  },
    { 35,   GT_TRUE,    GT_FALSE },
    { 36,   GT_TRUE,    GT_FALSE },
    { 37,   GT_TRUE,    GT_FALSE },
    { 38,   GT_TRUE,    GT_FALSE },
    { 39,   GT_FALSE,   GT_TRUE  },
    { 40,   GT_TRUE,    GT_FALSE },
    { 41,   GT_TRUE,    GT_FALSE },
    { 42,   GT_TRUE,    GT_FALSE },
    { 43,   GT_TRUE,    GT_TRUE  },
    { 44,   GT_TRUE,    GT_FALSE },
    { 45,   GT_TRUE,    GT_FALSE },
    { 46,   GT_TRUE,    GT_FALSE },
    { 47,   GT_TRUE,    GT_TRUE  },
    { 48,   GT_TRUE,    GT_FALSE },
    { 49,   GT_TRUE,    GT_FALSE },
    { 50,   GT_TRUE,    GT_FALSE },
    { 51,   GT_TRUE,    GT_TRUE  },
    { 53,   GT_TRUE,    GT_FALSE },
    { 54,   GT_TRUE,    GT_FALSE },
    { 55,   GT_TRUE,    GT_TRUE  },
    { 56,   GT_TRUE,    GT_FALSE },
    { 59,   GT_TRUE,    GT_TRUE  },
    { 62,   GT_TRUE,    GT_FALSE },
    { 64,   GT_TRUE,    GT_FALSE },
    { 65,   GT_TRUE,    GT_FALSE },
    { 66,   GT_TRUE,    GT_FALSE },
    { 67,   GT_TRUE,    GT_FALSE },
    { 68,   GT_TRUE,    GT_FALSE }
};

static APPDEMO_SERDES_LANE_POLARITY_STC  bobcat3_RD_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_FALSE,   GT_TRUE  },
    { 2,    GT_FALSE,   GT_TRUE  },
    { 4,    GT_FALSE,   GT_TRUE  },
    { 8,    GT_FALSE,   GT_TRUE  },
    { 9,    GT_FALSE,   GT_TRUE  },
    { 10,   GT_FALSE,   GT_TRUE  },
    { 12,   GT_FALSE,   GT_TRUE  },
    { 13,   GT_FALSE,   GT_TRUE  },
    { 14,   GT_FALSE,   GT_TRUE  },
    { 15,   GT_TRUE,    GT_FALSE  },
    { 17,   GT_FALSE,   GT_TRUE  },
    { 18,   GT_FALSE,   GT_TRUE  },
    { 21,   GT_FALSE,   GT_TRUE  },
    { 22,   GT_FALSE,   GT_TRUE  },
    { 26,   GT_TRUE,    GT_TRUE  },
    { 28,   GT_FALSE,   GT_TRUE  },
    { 30,   GT_TRUE,    GT_TRUE  },
    { 32,   GT_TRUE,    GT_TRUE  },
    { 34,   GT_FALSE,   GT_TRUE  },
    { 36,   GT_TRUE,    GT_TRUE  },
    { 38,   GT_TRUE,    GT_TRUE  },
    { 40,   GT_TRUE,    GT_TRUE  },
    { 42,   GT_TRUE,    GT_TRUE  },
    { 43,   GT_FALSE,   GT_TRUE  },
    { 44,   GT_TRUE,    GT_TRUE  },
    { 46,   GT_TRUE,    GT_TRUE  },
    { 47,   GT_FALSE,   GT_TRUE  },
    { 48,   GT_TRUE,    GT_TRUE  },
    { 50,   GT_TRUE,    GT_TRUE  },
    { 51,   GT_FALSE,   GT_TRUE  },
    { 52,   GT_FALSE,   GT_TRUE  },
    { 54,   GT_TRUE,    GT_TRUE  },
    { 55,   GT_FALSE,   GT_TRUE  },
    { 56,   GT_TRUE,    GT_TRUE  },
    { 58,   GT_FALSE,   GT_TRUE  },
    { 59,   GT_FALSE,   GT_TRUE  },
    { 60,   GT_FALSE,   GT_TRUE  },
    { 61,   GT_TRUE,    GT_FALSE },
    { 62,   GT_FALSE,   GT_TRUE  },
    { 64,   GT_FALSE,   GT_TRUE  },
    { 66,   GT_FALSE,   GT_TRUE  },
    { 68,   GT_FALSE,   GT_TRUE  },
    { 69,   GT_TRUE,    GT_FALSE },
    { 70,   GT_FALSE,   GT_TRUE  }
};

static APPDEMO_SERDES_LANE_POLARITY_STC  armstrong_RD_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_FALSE,   GT_TRUE  },
    { 1,    GT_TRUE,    GT_TRUE  },
    { 5,    GT_TRUE,    GT_FALSE },
    { 6,    GT_TRUE,    GT_FALSE },
    { 7,    GT_TRUE,    GT_FALSE },
    { 8,    GT_TRUE,    GT_TRUE  },
    { 9,    GT_TRUE,    GT_TRUE  },
    { 10,   GT_TRUE,    GT_FALSE },
    { 11,   GT_TRUE,    GT_FALSE },
    { 13,   GT_TRUE,    GT_FALSE },
    { 14,   GT_TRUE,    GT_FALSE },
    { 15,   GT_TRUE,    GT_FALSE },
    { 16,   GT_FALSE,   GT_TRUE  },
    { 17,   GT_TRUE,    GT_TRUE  },
    { 18,   GT_TRUE,    GT_FALSE },
    { 19,   GT_TRUE,    GT_FALSE },
    { 20,   GT_TRUE,    GT_FALSE },
    { 21,   GT_TRUE,    GT_FALSE },
    { 23,   GT_TRUE,    GT_FALSE },
    { 24,   GT_TRUE,    GT_TRUE  },
    { 25,   GT_TRUE,    GT_FALSE },
    { 26,   GT_TRUE,    GT_TRUE  },
    { 28,   GT_FALSE,   GT_TRUE  },
    { 30,   GT_FALSE,   GT_TRUE  },
    { 32,   GT_FALSE,   GT_TRUE  },
    { 33,   GT_TRUE,    GT_FALSE },
    { 34,   GT_FALSE,   GT_TRUE  },
    { 35,   GT_FALSE,   GT_TRUE  },
    { 37,   GT_TRUE,    GT_FALSE },
    { 39,   GT_FALSE,   GT_TRUE  },
    { 41,   GT_TRUE,    GT_FALSE },
    { 43,   GT_TRUE,    GT_TRUE  },
    { 45,   GT_TRUE,    GT_FALSE },
    { 47,   GT_TRUE,    GT_TRUE  },
    { 49,   GT_TRUE,    GT_FALSE },
    { 51,   GT_TRUE,    GT_TRUE  },
    { 52,   GT_TRUE,    GT_FALSE },
    { 53,   GT_TRUE,    GT_FALSE },
    { 55,   GT_TRUE,    GT_TRUE  },
    { 57,   GT_TRUE,    GT_FALSE },
    { 58,   GT_TRUE,    GT_FALSE },
    { 59,   GT_TRUE,    GT_TRUE  },
    { 60,   GT_FALSE,   GT_TRUE  },
    { 61,   GT_TRUE,    GT_FALSE },
    { 62,   GT_FALSE,   GT_TRUE  },
    { 64,   GT_FALSE,   GT_TRUE  },
    { 65,   GT_TRUE,    GT_FALSE },
    { 66,   GT_TRUE,    GT_TRUE  },
    { 68,   GT_TRUE,    GT_TRUE  },
    { 70,   GT_FALSE,   GT_TRUE  }
};

static APPDEMO_SERDES_LANE_POLARITY_STC  aldrin2_DB_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_FALSE,   GT_FALSE },
    { 1,    GT_TRUE,    GT_TRUE  },
    { 2,    GT_TRUE,    GT_FALSE },
    { 3,    GT_TRUE,    GT_FALSE },
    { 4,    GT_FALSE,   GT_TRUE  },
    { 5,    GT_FALSE,   GT_TRUE  },
    { 6,    GT_FALSE,   GT_TRUE  },
    { 7,    GT_TRUE,    GT_FALSE },
    { 8,    GT_TRUE,    GT_TRUE  },
    { 9,    GT_FALSE,   GT_TRUE  },
    { 10,   GT_FALSE,   GT_FALSE },
    { 11,   GT_TRUE,    GT_FALSE },
    { 12,   GT_TRUE,    GT_TRUE  },
    { 13,   GT_FALSE,   GT_TRUE  },
    { 14,   GT_FALSE,   GT_TRUE  },
    { 15,   GT_FALSE,   GT_FALSE },
    { 16,   GT_TRUE,    GT_TRUE  },
    { 17,   GT_TRUE,    GT_TRUE  },
    { 18,   GT_FALSE,   GT_FALSE },
    { 19,   GT_FALSE,   GT_FALSE },
    { 20,   GT_TRUE,    GT_TRUE  },
    { 21,   GT_TRUE,    GT_TRUE  },
    { 22,   GT_TRUE,    GT_FALSE },
    { 23,   GT_FALSE,   GT_FALSE },
    { 24,   GT_FALSE,   GT_TRUE  },
    { 25,   GT_FALSE,   GT_FALSE },
    { 26,   GT_TRUE,    GT_TRUE  },
    { 27,   GT_FALSE,   GT_FALSE },
    { 28,   GT_TRUE,    GT_FALSE },
    { 29,   GT_FALSE,   GT_FALSE },
    { 30,   GT_TRUE,    GT_FALSE },
    { 31,   GT_FALSE,   GT_FALSE },
    { 32,   GT_FALSE,   GT_FALSE },
    { 33,   GT_TRUE,    GT_FALSE },
    { 34,   GT_TRUE,    GT_FALSE },
    { 35,   GT_FALSE,   GT_FALSE },
    { 36,   GT_TRUE,    GT_FALSE },
    { 37,   GT_FALSE,   GT_FALSE },
    { 38,   GT_FALSE,   GT_FALSE },
    { 39,   GT_FALSE,   GT_FALSE },
    { 40,   GT_FALSE,   GT_FALSE },
    { 41,   GT_FALSE,   GT_FALSE },
    { 42,   GT_TRUE,    GT_FALSE },
    { 43,   GT_FALSE,   GT_FALSE },
    { 44,   GT_TRUE,    GT_FALSE },
    { 45,   GT_TRUE,    GT_FALSE },
    { 46,   GT_FALSE,   GT_TRUE  },
    { 47,   GT_FALSE,   GT_FALSE },
    { 48,   GT_FALSE,   GT_FALSE },
    { 49,   GT_TRUE,    GT_TRUE  },
    { 50,   GT_TRUE,    GT_FALSE },
    { 51,   GT_TRUE,    GT_TRUE  },
    { 52,   GT_FALSE,   GT_FALSE },
    { 53,   GT_TRUE,    GT_TRUE  },
    { 54,   GT_FALSE,   GT_FALSE },
    { 55,   GT_FALSE,   GT_TRUE  },
    { 56,   GT_TRUE,    GT_FALSE },
    { 57,   GT_FALSE,   GT_TRUE  },
    { 58,   GT_FALSE,   GT_FALSE },
    { 59,   GT_TRUE,    GT_TRUE  },
    { 60,   GT_TRUE,    GT_FALSE },
    { 61,   GT_FALSE,   GT_TRUE  },
    { 62,   GT_TRUE,    GT_FALSE },
    { 63,   GT_TRUE,    GT_TRUE  },
    { 64,   GT_FALSE,   GT_FALSE },
    { 65,   GT_TRUE,    GT_TRUE  },
    { 66,   GT_FALSE,   GT_FALSE },
    { 67,   GT_TRUE,    GT_TRUE  },
    { 68,   GT_TRUE,    GT_FALSE },
    { 69,   GT_TRUE,    GT_TRUE  },
    { 70,   GT_FALSE,   GT_FALSE },
    { 71,   GT_TRUE,    GT_FALSE },
    { 72,   GT_FALSE,   GT_FALSE }
};

static APPDEMO_SERDES_LANE_POLARITY_STC  aldrin2_RD_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_FALSE,   GT_FALSE },
    { 1,    GT_FALSE,   GT_TRUE  },
    { 2,    GT_TRUE,    GT_FALSE },
    { 3,    GT_FALSE,   GT_FALSE },
    { 4,    GT_FALSE,   GT_TRUE  },
    { 5,    GT_TRUE,    GT_TRUE  },
    { 6,    GT_TRUE,    GT_TRUE  },
    { 7,    GT_TRUE,    GT_FALSE },
    { 8,    GT_FALSE,   GT_TRUE  },
    { 9,    GT_FALSE,   GT_TRUE  },
    { 10,   GT_TRUE,    GT_FALSE },
    { 11,   GT_FALSE,   GT_FALSE },
    { 12,   GT_TRUE,    GT_TRUE  },
    { 13,   GT_FALSE,   GT_TRUE  },
    { 14,   GT_FALSE,   GT_TRUE  },
    { 15,   GT_FALSE,   GT_FALSE },
    { 16,   GT_TRUE,    GT_TRUE  },
    { 17,   GT_FALSE,   GT_TRUE  },
    { 18,   GT_FALSE,   GT_FALSE },
    { 19,   GT_TRUE,    GT_FALSE },
    { 20,   GT_TRUE,    GT_TRUE  },
    { 21,   GT_TRUE,    GT_TRUE  },
    { 22,   GT_FALSE,   GT_FALSE },
    { 23,   GT_TRUE,    GT_FALSE },
    { 24,   GT_TRUE,    GT_TRUE  },
    { 25,   GT_FALSE,   GT_TRUE  },
    { 26,   GT_TRUE,    GT_TRUE  },
    { 27,   GT_FALSE,   GT_TRUE  },
    { 28,   GT_TRUE,    GT_TRUE  },
    { 29,   GT_FALSE,   GT_TRUE  },
    { 30,   GT_TRUE,    GT_TRUE  },
    { 31,   GT_FALSE,   GT_TRUE  },
    { 32,   GT_FALSE,   GT_TRUE  },
    { 33,   GT_TRUE,    GT_TRUE  },
    { 34,   GT_TRUE,    GT_TRUE  },
    { 35,   GT_FALSE,   GT_TRUE  },
    { 36,   GT_TRUE,    GT_TRUE  },
    { 37,   GT_FALSE,   GT_TRUE  },
    { 38,   GT_TRUE,    GT_TRUE  },
    { 39,   GT_FALSE,   GT_TRUE  },
    { 40,   GT_TRUE,    GT_TRUE  },
    { 41,   GT_FALSE,   GT_TRUE  },
    { 42,   GT_TRUE,    GT_TRUE  },
    { 43,   GT_FALSE,   GT_TRUE  },
    { 44,   GT_TRUE,    GT_TRUE  },
    { 45,   GT_FALSE,   GT_TRUE  },
    { 46,   GT_TRUE,    GT_TRUE  },
    { 47,   GT_TRUE,    GT_TRUE  },
    { 48,   GT_TRUE,    GT_FALSE },
    { 49,   GT_TRUE,    GT_FALSE },
    { 50,   GT_FALSE,   GT_FALSE },
    { 51,   GT_FALSE,   GT_FALSE },
    { 52,   GT_TRUE,    GT_FALSE },
    { 53,   GT_TRUE,    GT_FALSE },
    { 54,   GT_TRUE,    GT_FALSE },
    { 55,   GT_FALSE,   GT_FALSE },
    { 56,   GT_FALSE,   GT_FALSE },
    { 57,   GT_TRUE,    GT_FALSE },
    { 58,   GT_TRUE,    GT_FALSE },
    { 59,   GT_TRUE,    GT_FALSE },
    { 60,   GT_FALSE,   GT_FALSE },
    { 61,   GT_TRUE,    GT_FALSE },
    { 62,   GT_FALSE,   GT_FALSE },
    { 63,   GT_TRUE,    GT_FALSE },
    { 64,   GT_TRUE,    GT_FALSE },
    { 65,   GT_FALSE,   GT_FALSE },
    { 66,   GT_TRUE,    GT_FALSE },
    { 67,   GT_TRUE,    GT_FALSE },
    { 68,   GT_FALSE,   GT_FALSE },
    { 69,   GT_FALSE,   GT_FALSE },
    { 70,   GT_TRUE,    GT_FALSE },
    { 71,   GT_TRUE,    GT_FALSE }
};

/**
* @internal prvAppDemoSerdesPolarityConfigSet function
* @endinternal
*
* @brief   Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvAppDemoSerdesPolarityConfigSet
(
    IN GT_U8 devNum
)
{
    GT_STATUS                           rc;
    GT_U32                              i;
    GT_U32                              polarityArraySize;
    APPDEMO_SERDES_LANE_POLARITY_STC    *currentPolarityArrayPtr;
    CPSS_PP_DEVICE_TYPE                 devType;
    CPSS_DXCH_CFG_DEV_INFO_STC          devInfo;

    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCfgDevInfoGet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    devType = devInfo.genDevInfo.devType;

    if(bc2BoardType == APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS)
    {
        currentPolarityArrayPtr = cetus_LewisRD_PolarityArray;
        polarityArraySize = sizeof(cetus_LewisRD_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
    }
    else if (bc2BoardType == APP_DEMO_ALDRIN_BOARD_DB_CNS)
    {
        if(devType == CPSS_98DX8332_Z0_CNS)
        {
            currentPolarityArrayPtr = aldrinZ0_DB_PolarityArray;
            polarityArraySize = sizeof(aldrinZ0_DB_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
        }
        else
        {
            currentPolarityArrayPtr = aldrinA0_DB_PolarityArray;
            polarityArraySize = sizeof(aldrinA0_DB_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
        }
    }
    else if (bc2BoardType == APP_DEMO_ALDRIN_BOARD_RD_CNS)
    {
        currentPolarityArrayPtr = aldrinA0_RD_PolarityArray;
        polarityArraySize = sizeof(aldrinA0_RD_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
    }
    else if(bc2BoardType == APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS)
    {
        currentPolarityArrayPtr = caelum_CygnusRD_PolarityArray;
        polarityArraySize = sizeof(caelum_CygnusRD_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
    }
    else if(bc2BoardType == APP_DEMO_BOBCAT3_BOARD_DB_CNS)
    {
        currentPolarityArrayPtr = bobcat3_DB_PolarityArray;
        polarityArraySize = sizeof(bobcat3_DB_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
    }
    else if(bc2BoardType == APP_DEMO_BOBCAT3_BOARD_RD_CNS)
    {
        currentPolarityArrayPtr = bobcat3_RD_PolarityArray;
        polarityArraySize = sizeof(bobcat3_RD_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
    }
    else if(bc2BoardType == APP_DEMO_ARMSTRONG_BOARD_RD_CNS)
    {
        currentPolarityArrayPtr = armstrong_RD_PolarityArray;
        polarityArraySize = sizeof(armstrong_RD_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
    }
    else if(bc2BoardType == APP_DEMO_ALDRIN2_BOARD_DB_CNS)
    {
        currentPolarityArrayPtr = aldrin2_DB_PolarityArray;
        polarityArraySize = sizeof(aldrin2_DB_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
    }
    else if(bc2BoardType == APP_DEMO_ALDRIN2_BOARD_RD_CNS)
    {
        currentPolarityArrayPtr = aldrin2_RD_PolarityArray;
        polarityArraySize = sizeof(aldrin2_RD_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
    }
    else
    {
        return GT_OK;
    }

    for (i = 0; i < polarityArraySize; i++)
    {
        rc = cpssDxChPortSerdesLanePolaritySet(devNum, 0,
                                               currentPolarityArrayPtr[i].laneNum,
                                               currentPolarityArrayPtr[i].invertTx,
                                               currentPolarityArrayPtr[i].invertRx);

        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesLanePolaritySet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal appDemoBc2PortInterfaceInit function
* @endinternal
*
* @brief   Execute predefined ports configuration.
*
* @param[in] dev                      - device number
* @param[in] boardRevId               - revision ID
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoBc2PortInterfaceInit
(
    IN  GT_U8 dev,
    IN  GT_U8 boardRevId
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      initSerdesDefaults,     /* should appDemo configure ports modes */
                bcat2PortsConfigType;   /* which ports modes configuration wanted:
                                           0 - ports 0-47-QSGMII,48-SGMII,49-51-KR_10G,56-62-RXAUI;*/
    GT_U32      coreClockDB;            /* core clock - DB value */
    GT_U32      coreClockHW;            /* core clock - HW value */
    GT_U32      i;                      /* iterator */
    PortInitList_STC        *portInitList;
    CPSS_PORTS_BMP_STC      portsBmp;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                          phyAddr;
    CPSS_PHY_XSMI_INTERFACE_ENT     xsmiInterface;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    bcat2PortsConfigType = 0;
    cpssOsMemSet(changeToDownHappend, 0, sizeof(GT_BOOL)*CPSS_MAX_PORTS_NUM_CNS);
    cpssOsMemSet(changeToUpHappend, 0, sizeof(GT_BOOL)*CPSS_MAX_PORTS_NUM_CNS);

    rc = prvAppDemoSerdesPolarityConfigSet(dev);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("prvAppDemoSerdesPolarityConfigSet", rc);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* check if user wants to init ports to default values */
    rc = appDemoDbEntryGet("initSerdesDefaults", &initSerdesDefaults);
    if(rc != GT_OK)
    {
        initSerdesDefaults = 1;
    }

    if(0 == initSerdesDefaults)
    {
        return GT_OK;
    }

    rc = cpssDxChHwCoreClockGet(dev,/*OUT*/&coreClockDB,&coreClockHW);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = bcat2PortInterfaceInitPortInitListGet(dev, boardRevId, bc2BoardType, coreClockDB,/*OUT*/&portInitList);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (NULL == portInitList)
    {
        cpssOsPrintf("\n-->ERROR : bcat2PortInterfaceInitPortInitListGet() : revId %d is not supported\n", boardRevId);
        return GT_NOT_SUPPORTED;
    }

    rc = appDemoDbEntryGet("bcat2PortsConfigType", &bcat2PortsConfigType);
    if(rc != GT_OK)
    {
        if (boardRevId == 1)
        {
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
            {
                /* currently use the array given by
                   bcat2PortInterfaceInitPortInitListGet */
                bcat2PortsConfigType = 0;
            }
            else
            if (coreClockDB == 362 || coreClockDB == 521)
            {
                /* use Port Config 100 (advanced UT mode) for 29,1,0 revision */
                bcat2PortsConfigType = 100;
            }
        }
        else
        {
            bcat2PortsConfigType = 0;
        }
    }

    switch(bcat2PortsConfigType)
    {
        case 100:
            portInitList = &portInitlist_29_100_48x1G_QXGMII_7x10G_KR[0];
        break;
    }
    if (PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devType == CPSS_98DX4253_CNS)
    {
        portInitList = &portInitlist_29_1_16x1G_QXGMII_8x10G_KR[0];
    }

    if(portInitlist_AllPorts_used ||
       appDemoPpConfigList[dev].portInitlist_AllPorts_used)
    {
        GT_U32  numGigNeeded;

        portInitList = portInitlist_AllPorts;

        /* calc the needed BW needed */
        numGigNeeded = getNumGigBw(dev,portInitList);

        /*
            state to get more BW than given by :
            Total Bandwidth @64B
            or by Max Bandwidth
        */
        rc = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev,numGigNeeded,CPSS_DXCH_MIN_SPEED_1000_Mbps_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS == bc2BoardType)
    {
        APPDEMO_BC2_SERDES_TX_CONFIG_STC avagoSerdesManualTuneArray[] =
        {
            /* connected to PHY1 */
              { 56, { 14, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }
            , { 57, { 14, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }
            , { 58, { 14, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }
            , { 59, { 14, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }

            /* connected to PHY2 */
            , { 68, { 13, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }
            , { 69, { 13, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }
            , { 70, { 13, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }
            , { 71, { 13, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }

            /* connected to PHY3 (combo) */
            , { 64, { 10, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }
            , { 65, { 10, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }
            , { 66, { 10, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }
            , { 67, { 10, 0 /* mv_txAmpAdj  NR */, 0, 0, 0 /* mv_slewRate NR */ } }
        };
        GT_U32 avagoSerdesManualTuneArraySize;

        cpssOsPrintf("RD_LEWIS ports configure ...\n");

        for (portNum = 56; portNum < 72; portNum++)
        {
            if ((60 == portNum) || (61 == portNum) || (62 == portNum) || (63 == portNum))
            { /* skip not existing ports */
                continue;
            }
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

            rc = cpssDxChPortRefClockSourceOverrideEnableSet(dev, portNum, GT_TRUE, CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortRefClockSourceOverrideEnableSet", rc);
            if(GT_OK != rc)
            {
                return rc;
            }

            rc = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                          CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                          CPSS_PORT_SPEED_10000_E);
            if(rc != GT_OK)
            {
                cpssOsPrintf("!!!10G SFP failed:portNum=%d!!!\n", portNum);
                continue;
            }
        }

        /* Lewis serdes TX parameter, we change Tx here , keep 88x3240 x-unit stays at LR mode. */
        avagoSerdesManualTuneArraySize = sizeof(avagoSerdesManualTuneArray) / sizeof(avagoSerdesManualTuneArray[0]);
        for (i = 0; i < avagoSerdesManualTuneArraySize; i++)
        {
            rc = cpssDxChPortSerdesManualTxConfigSet(dev,
                                                     avagoSerdesManualTuneArray[i].portNum,
                                                     0, /* laneNum */
                                                     &avagoSerdesManualTuneArray[i].avagoSerdesTxTuneParams);
            if (rc != GT_OK)
            {
                cpssOsPrintf("cpssDxChPortSerdesManualTxConfigSet:portNum=%d,rc=%d!!!\n", avagoSerdesManualTuneArray[i].portNum, rc);
                return rc;
            }
        }
    }
    else if(bc2BoardType == APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS)
    {
        prvCpssDrvHwPpResetAndInitControllerSetRegField(0, 0xf829c, 0, 1, 0) ;

        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x50000000, 0x5f2037);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x50000120, 0x55555555);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x50000004, 0x10012d);

        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102C0030, 0xc);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102C1030, 0x2c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102C2030, 0x4c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102C3030, 0x6c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102C8030, 0x8c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102C9030, 0xac);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102CA030, 0xcc);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102CB030, 0xec);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102CC030, 0x10c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102CD030, 0x12c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102CE030, 0x14c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102CF030, 0x16c);


        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x50000128, 0x00FFFFFF);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x5000012C, 0x00FFFFFF);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x50000130, 0x00FFFFFF);

        rc = appDemoBc2PortListInit(dev,portInitList,GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBc2PortListInit", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else if((APP_DEMO_ALDRIN_BOARD_DB_CNS == bc2BoardType) ||
            (APP_DEMO_BOBCAT3_BOARD_DB_CNS == bc2BoardType) ||
            (APP_DEMO_BOBCAT3_BOARD_RD_CNS == bc2BoardType) ||
            (APP_DEMO_ARMSTRONG_BOARD_RD_CNS == bc2BoardType))
    {
        rc = appDemoBc2PortListInit(dev,portInitList,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else if(APP_DEMO_ALDRIN_BOARD_RD_CNS == bc2BoardType)
    {
        rc = appDemoBc2PortListInit(dev,portInitList,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* LED behavior, need to refine ... Johnc */

        prvCpssDrvHwPpResetAndInitControllerSetRegField(0, 0xf829c, 0, 1, 0) ;

        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x21000000 , 0x5f2037);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x21000120, 0x55555555);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x21000004 , 0x0010012A);

        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x21000128, 0x00FFFFFF);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x2100012C, 0x00FFFFFF);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x21000130, 0x00FFFFFF);


        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100C8030, 0x0c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100C9030, 0x2c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100CA030, 0x4c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100CB030, 0x6c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100C0030, 0x8c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100C1030, 0xAc);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100C2030, 0xCc);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100C3030, 0xEc);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100C4030, 0x10c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100C5030, 0x12c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100C6030, 0x14c);
        prvCpssHwPpPortGroupWriteRegister(0, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x100C7030, 0x16c);



        for(xsmiInterface = CPSS_PHY_XSMI_INTERFACE_0_E;
            xsmiInterface <= CPSS_PHY_XSMI_INTERFACE_1_E; xsmiInterface++)
        {
            for(phyAddr = 0; phyAddr < 32; phyAddr++)
            {
                cpssXsmiPortGroupRegisterWrite(0, 0x0, xsmiInterface,  phyAddr,  0xf020, 31,  0x139);
                cpssXsmiPortGroupRegisterWrite(0, 0x0, xsmiInterface,  phyAddr,  0xf021, 31,  0x28);
                cpssXsmiPortGroupRegisterWrite(0, 0x0, xsmiInterface,  phyAddr,  0xf022, 31,  0x30);
            }
        }

        return rc;
    }
    else if(APP_DEMO_ALDRIN2_BOARD_RD_CNS == bc2BoardType)
    {
        rc = appDemoBc2PortListInit(dev,portInitList,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        for(xsmiInterface = CPSS_PHY_XSMI_INTERFACE_0_E; xsmiInterface <= CPSS_PHY_XSMI_INTERFACE_3_E; xsmiInterface++)
        {
            for(phyAddr = 0; phyAddr < 12; phyAddr++)
            {
                /* Set configuration for SGMII2.5G on XSMI-PHY */
                cpssXsmiPortGroupRegisterWrite(0, 0x0, xsmiInterface, phyAddr, 0x0, 0x1, 0x2058);
            }
        }

        cpssDxChLedStreamPortPositionSet(0,0,0);
        cpssDxChLedStreamPortPositionSet(0,1,1);
        cpssDxChLedStreamPortPositionSet(0,2,2);
        cpssDxChLedStreamPortPositionSet(0,3,3);
        cpssDxChLedStreamPortPositionSet(0,4,4);
        cpssDxChLedStreamPortPositionSet(0,5,5);
        cpssDxChLedStreamPortPositionSet(0,6,6);
        cpssDxChLedStreamPortPositionSet(0,7,7);
        cpssDxChLedStreamPortPositionSet(0,8,8);
        cpssDxChLedStreamPortPositionSet(0,9,9);
        cpssDxChLedStreamPortPositionSet(0,10,10);
        cpssDxChLedStreamPortPositionSet(0,11,11);

        cpssDxChLedStreamPortPositionSet(0,12,0);
        cpssDxChLedStreamPortPositionSet(0,13,1);
        cpssDxChLedStreamPortPositionSet(0,14,2);
        cpssDxChLedStreamPortPositionSet(0,15,3);
        cpssDxChLedStreamPortPositionSet(0,16,4);
        cpssDxChLedStreamPortPositionSet(0,17,5);
        cpssDxChLedStreamPortPositionSet(0,18,6);
        cpssDxChLedStreamPortPositionSet(0,19,7);
        cpssDxChLedStreamPortPositionSet(0,20,8);
        cpssDxChLedStreamPortPositionSet(0,21,9);
        cpssDxChLedStreamPortPositionSet(0,22,10);
        cpssDxChLedStreamPortPositionSet(0,23,11);

        cpssDxChLedStreamPortPositionSet(0,48,0);
        cpssDxChLedStreamPortPositionSet(0,49,1);
        cpssDxChLedStreamPortPositionSet(0,50,2);
        cpssDxChLedStreamPortPositionSet(0,51,3);
        cpssDxChLedStreamPortPositionSet(0,52,4);
        cpssDxChLedStreamPortPositionSet(0,53,5);
        cpssDxChLedStreamPortPositionSet(0,54,6);
        cpssDxChLedStreamPortPositionSet(0,55,7);
        cpssDxChLedStreamPortPositionSet(0,56,8);
        cpssDxChLedStreamPortPositionSet(0,57,9);
        cpssDxChLedStreamPortPositionSet(0,58,10);
        cpssDxChLedStreamPortPositionSet(0,64,11);
        cpssDxChLedStreamPortPositionSet(0,65,12);
        cpssDxChLedStreamPortPositionSet(0,66,13);
        cpssDxChLedStreamPortPositionSet(0,67,14);
        cpssDxChLedStreamPortPositionSet(0,68,15);
        cpssDxChLedStreamPortPositionSet(0,69,16);
        cpssDxChLedStreamPortPositionSet(0,70,17);
        cpssDxChLedStreamPortPositionSet(0,71,18);
        cpssDxChLedStreamPortPositionSet(0,72,19);
        cpssDxChLedStreamPortPositionSet(0,73,20);
        cpssDxChLedStreamPortPositionSet(0,74,21);
        cpssDxChLedStreamPortPositionSet(0,75,22);
        cpssDxChLedStreamPortPositionSet(0,76,23);

        cpssDxChLedStreamPortPositionSet(0,24,0);
        cpssDxChLedStreamPortPositionSet(0,25,1);
        cpssDxChLedStreamPortPositionSet(0,26,2);
        cpssDxChLedStreamPortPositionSet(0,27,3);
        cpssDxChLedStreamPortPositionSet(0,28,4);
        cpssDxChLedStreamPortPositionSet(0,29,5);
        cpssDxChLedStreamPortPositionSet(0,30,6);
        cpssDxChLedStreamPortPositionSet(0,31,7);
        cpssDxChLedStreamPortPositionSet(0,32,8);
        cpssDxChLedStreamPortPositionSet(0,33,9);
        cpssDxChLedStreamPortPositionSet(0,34,10);
        cpssDxChLedStreamPortPositionSet(0,35,11);
        cpssDxChLedStreamPortPositionSet(0,36,12);
        cpssDxChLedStreamPortPositionSet(0,37,13);
        cpssDxChLedStreamPortPositionSet(0,38,14);
        cpssDxChLedStreamPortPositionSet(0,39,15);
        cpssDxChLedStreamPortPositionSet(0,40,16);
        cpssDxChLedStreamPortPositionSet(0,41,17);
        cpssDxChLedStreamPortPositionSet(0,42,18);
        cpssDxChLedStreamPortPositionSet(0,43,19);
        cpssDxChLedStreamPortPositionSet(0,44,20);
        cpssDxChLedStreamPortPositionSet(0,45,21);
        cpssDxChLedStreamPortPositionSet(0,46,22);
        cpssDxChLedStreamPortPositionSet(0,47,23);


        return rc;
    }
    else
    {
        rc = appDemoBc2PortListInit(dev,portInitList,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* BC2 boards, using legacy polarity API */
    if (boardRevId == 4)
    {
        /*40G to 4*10G split cabel to run the test */
        cpssDxChPortSerdesPolaritySet(0, 59, 1, GT_TRUE,  GT_TRUE);
        cpssDxChPortSerdesPolaritySet(0, 58, 1, GT_TRUE,  GT_TRUE);
        cpssDxChPortSerdesPolaritySet(0, 57, 1, GT_FALSE, GT_FALSE);
        cpssDxChPortSerdesPolaritySet(0, 56, 1, GT_FALSE, GT_FALSE);
        cpssDxChPortSerdesPolaritySet(0, 71, 1, GT_FALSE, GT_FALSE);
        cpssDxChPortSerdesPolaritySet(0, 70, 1, GT_FALSE, GT_FALSE);
        cpssDxChPortSerdesPolaritySet(0, 69, 1, GT_FALSE, GT_FALSE);
        cpssDxChPortSerdesPolaritySet(0, 68, 1, GT_FALSE, GT_FALSE);
        cpssDxChPortSerdesPolaritySet(0, 64, 1, GT_FALSE, GT_TRUE);
        cpssDxChPortSerdesPolaritySet(0, 65, 1, GT_FALSE, GT_TRUE);
        cpssDxChPortSerdesPolaritySet(0, 66, 1, GT_FALSE, GT_TRUE);
        cpssDxChPortSerdesPolaritySet(0, 67, 1, GT_TRUE,  GT_TRUE);
    }
    if (boardRevId == 3)
    {
        cpssDxChPortSerdesPolaritySet(0, 66, 1, GT_FALSE, GT_TRUE);
        cpssDxChPortSerdesPolaritySet(0, 67, 1, GT_TRUE,  GT_TRUE);
        cpssDxChPortSerdesPolaritySet(0, 64, 1, GT_FALSE, GT_TRUE);
        cpssDxChPortSerdesPolaritySet(0, 65, 1, GT_FALSE, GT_TRUE);
    }

    if (boardRevId == 4  || boardRevId == 3)
    {
        if (prvCpssHwPpPortGroupWriteRegister(dev,
                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                              0x102c8030, 0xc) != GT_OK)
        {
            return GT_HW_ERROR;
        }

        /*led programming */
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102c9030, 0x2c);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102ca030, 0x4c);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102cb030, 0x6c);

        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102cc030, 0x8c);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102cd030, 0xac);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102ce030, 0xcc);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102cf030, 0xec);

        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102c0030, 0xc);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102c1030, 0x2c);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102c2030, 0x4c);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x102c3030, 0x6c);

        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x5000000c, 0x492249);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x50000000, 0xffc037);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x50000004, 0x2000137);

        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x2400000c, 0x492249);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x24000000, 0xffc037);
        prvCpssHwPpPortGroupWriteRegister(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, 0x24000004, 0x2000137);
    }

    return GT_OK;
}


#if defined (INCLUDE_TM)
/**
* @internal appDemoBc2DramOrTmInit function
* @endinternal
*
* @brief   Board specific configurations of TM and all related HW
*         or of TM related DRAM only.
* @param[in] dev                      - device number
* @param[in] flags                    - initialization flags
*                                      currently supported value 1 that means "DRAM only"
*                                      otherwise TM and all related HW initialized
*
* @param[out] dramIfBmpPtr             - pointer to bitmap of TM DRAM interfaces. may be NULL
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Used type of board recognized at last device (as in other code)
*
*/
GT_STATUS appDemoBc2DramOrTmInit
(
    IN  GT_U8       dev,
    IN  GT_U32      flags,
    OUT GT_U32     *dramIfBmpPtr
)
{
    GT_STATUS                            rc;
    CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC algoParams;
    CPSS_DXCH_TM_GLUE_DRAM_CFG_STC       *dramCfgPtr = NULL;
    GT_U32                               tmDramFailIgnore = 0;
    GT_U32                               data;

#ifdef ASIC_SIMULATION
    algoParams.algoType = CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_STATIC_E;
#else
    algoParams.algoType = CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_DYNAMIC_E;
#endif

    switch (bc2BoardType)
    {
        case APP_DEMO_CAELUM_BOARD_DB_CNS:
        case APP_DEMO_CETUS_BOARD_DB_CNS:
        case APP_DEMO_BC2_BOARD_DB_CNS:
        case APP_DEMO_ALDRIN_BOARD_DB_CNS:
        case APP_DEMO_BOBCAT3_BOARD_DB_CNS:
        case APP_DEMO_BOBCAT3_BOARD_RD_CNS:
        case APP_DEMO_ARMSTRONG_BOARD_RD_CNS:
            algoParams.performWriteLeveling = GT_TRUE;
            break;
        default:
            /*
            APP_DEMO_BC2_BOARD_RD_MSI_CNS:
            APP_DEMO_BC2_BOARD_RD_MTL_CNS:
                        APP_DEMO_ALDRIN_BOARD_RD_CNS:
            */
            algoParams.performWriteLeveling = GT_FALSE;
    }

    rc = cpssDxChTmGlueDramInitFlagsSet(dev, flags);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGlueDramInitFlagsSet", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (isBobkBoard)
    {
        if (bc2BoardType == APP_DEMO_CETUS_BOARD_DB_CNS)
            dramCfgPtr = &bobkDramCfgArr[BOBK_DRAM_CFG_IDX_CETUS_CNS];
        else
            dramCfgPtr = &bobkDramCfgArr[BOBK_DRAM_CFG_IDX_CAELUM_CNS];
        /*
        if (dramCfgPtr->activeInterfaceNum == 3)
        {
            cpssOsPrintf("Set Bobk-Caelum TM DDRs to 2\n");
            dramCfgPtr->activeInterfaceNum--;
        }
        */
    }
    else
    {
        dramCfgPtr = &bc2DramCfgArr[bc2BoardType];

        /* verify DDR interface 4 is allocated for msys */
        if (dramCfgPtr->activeInterfaceNum == 5)
        {
            /* check bit#11 on Device SAR1 Register */
            rc = cpssDrvHwPpResetAndInitControllerReadReg(dev, 0x000F8200 ,&data);
            if (GT_OK != rc)
                return rc;

            /*cpssOsPrintf("SAR1 data: 0x%0x\n", data);*/
            data = (data >> 11) & 0x1;

            if (data) /* msys*/
            {
                cpssOsPrintf("Warning: DDR interface 4 is used by MSYS and can not be used by TM\n");
                dramCfgPtr->activeInterfaceNum--;
            }
        }
    }

    if (dramIfBmpPtr)
    {
        if (dramCfgPtr->activeInterfaceMask == 0)
        {
            *dramIfBmpPtr = (1 << dramCfgPtr->activeInterfaceNum) - 1;
        }
        else
        {
            *dramIfBmpPtr = dramCfgPtr->activeInterfaceMask;
        }
    }

    rc = cpssDxChTmGlueDramInit(dev, dramCfgPtr, &algoParams);

    appDemoDbEntryGet("tmDramFailIgnore", &tmDramFailIgnore);

    if (rc != GT_OK && tmDramFailIgnore)
    {
        cpssOsPrintf("Ignore error: %d from cpssDxChTmGlueDramInit!\n", rc);
        rc = GT_OK;
    }

    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGlueDramInit", rc);

    return rc;
}
#endif /*#if defined (INCLUDE_TM)*/

/*
    API to enable Custom ports mapping (called befor initSystem)
    when system is initialized with custom ports mapping,
    TM Ports Init is called after custom ports mapping is applied.

*/
void appDemoBc2SetCustomPortsMapping
(
    IN GT_BOOL enable
)
{
    isCustomPortsMapping = enable;
}

GT_STATUS tmConditionalDumpTmUnitsErrors(GT_U8 devNum, char *fileNamePtr, GT_32 line, GT_BOOL dumpAlways);

/*
    API to configure TmPorts, with non default ports mapping
*/
GT_STATUS appDemoBc2TmPortsInit
(
    IN GT_U8 dev
)
{
    GT_STATUS   rc;
#if defined (INCLUDE_TM)
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileSet;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT alpha;
    GT_U32                            portMaxBuffLimit;
    GT_U32                            portMaxDescrLimit;
    GT_U8                             trafficClass;
    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS tailDropProfileParams;
    GT_U32      hwArray[2];   /* HW table data */
    CPSS_DXCH_TABLE_ENT tableType; /*table type*/
    GT_U32      appDemoDbValue;
    GT_BOOL     isTmSupported;
    GT_U32      i,dumpTmErrors = 0;
    APP_DEMO_PP_CONFIG *appDemoPpConfigPtr;
    CPSS_DXCH_PORT_MAP_STC  *portsMappingPtr;

    rc = appDemoIsTmSupported(dev,/*OUT*/&isTmSupported);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (!isTmSupported)
    {
        return GT_NOT_SUPPORTED;
    }

    if (GT_OK != appDemoDbEntryGet("externalMemoryInitFlags", &appDemoDbValue))
    {
        appDemoDbValue = 0;
    }

    rc = appDemoBc2DramOrTmInit(dev, appDemoDbValue, NULL);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBc2DramOrTmInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    appDemoPpConfigPtr = &appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(dev)];
    portsMappingPtr = (CPSS_DXCH_PORT_MAP_STC*)appDemoPpConfigPtr->portsMapArrPtr;

    if (portsMappingPtr == NULL || appDemoPpConfigPtr->portsMapArrLen == 0)
    {
        cpssOsPrintf("appDemoBc2TmPortsInit: Unknown ports mapping\n");
        return GT_FAIL;
    }

    switch (appDemoTmScenarioMode)
    {
        case CPSS_TM_48_PORT:
            rc = appDemoTmGeneral48PortsInit(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmGeneral48PortsInit", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_TM_2:
            rc = appDemoTmScenario2Init(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmScenario2Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_TM_3:
            rc = appDemoTmScenario3Init(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmScenario3Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_TM_4:
            rc = appDemoTmScenario4Init(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmScenario4Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_TM_5:
            rc = appDemoTmScenario5Init(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmScenario5Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_TM_6:
            rc = appDemoTmScenario6Init(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmScenario6Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        case CPSS_TM_7:
            rc = appDemoTmScenario7Init(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmScenario7Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        case CPSS_TM_8:
            rc = appDemoTmScenario8Init(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmScenario8Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        case CPSS_TM_9:
            rc = appDemoTmScenario9Init(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmScenario9Init", rc);
            if (rc != GT_OK)
            {
                    return rc;
            }
            break;
        case CPSS_TM_STRESS:
            rc = appDemoTmStressScenarioInit(dev,0);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmStressScenarioInit 0", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_TM_STRESS_1:
            rc = appDemoTmStressScenarioInit(dev,1);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmStressScenarioInit 1", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_TM_REV4:
            rc = appDemoTmRev4ScenarioInit(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmRev4ScenarioInit 1", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        case CPSS_TM_USER_20: /*Private version  test for customer */
            rc = appDemoTmScenario20Init(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmScenario20Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        default:
            rc = appDemoTmLibInitOnly(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTmLibInitOnly", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
    }

        rc = appDemoDbEntryGet("tmDumpErrors", &dumpTmErrors);
        if(rc != GT_OK)
        {
            dumpTmErrors = 0;
        }

        if (dumpTmErrors)
        {
            rc = tmConditionalDumpTmUnitsErrors(dev, __FILE__, __LINE__, GT_FALSE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("tmConditionalDumpTmUnitsErrors", rc);
        }

    /* TM related configurations */
    rc = prvBobcat2TmGlueConfig(dev);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("prvBobcat2TmGlueConfig", rc);

    for(profileSet = CPSS_PORT_TX_DROP_PROFILE_1_E;
         profileSet <= CPSS_PORT_TX_DROP_PROFILE_16_E; profileSet++)
    {
        rc = cpssDxChPortTxTailDropProfileGet(dev, profileSet,
                                              &alpha,
                                              &portMaxBuffLimit,
                                              &portMaxDescrLimit);
        if (rc != GT_OK)
        {
            return rc;
        }
        portMaxBuffLimit = 0xfffff;
        rc = cpssDxChPortTxTailDropProfileSet(dev, profileSet,
                                              alpha,
                                              portMaxBuffLimit,
                                              portMaxDescrLimit);
        if (rc != GT_OK)
        {
            return rc;
        }

        for(trafficClass = 0; trafficClass < 8; trafficClass++)
        {
            rc = cpssDxChPortTx4TcTailDropProfileGet(dev, profileSet,
                                                     trafficClass,
                                                     &tailDropProfileParams);
            if (rc != GT_OK)
            {
                return rc;
            }
            tailDropProfileParams.dp0MaxBuffNum = BIT_14-1; /* max value ?????  0x3FFFF; */
            rc = cpssDxChPortTx4TcTailDropProfileSet(dev, profileSet,
                                                     trafficClass,
                                                     &tailDropProfileParams);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for(i = 0; i < 128; i++)
    {
        CPSS_TBD_BOOKMARK_BOBCAT2
        /* remove this when cpssDxChPortTx4TcTailDropProfileSet/Get will support new table structure */
        /* Queue Limits DP0 - Enqueue */
        tableType = CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_E;
        hwArray[0] = 0xFFFFC019;
        hwArray[1] = 0xffff;
        rc = prvCpssDxChWriteTableEntry(dev, tableType, i, hwArray);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Queue Buffer Limits - Dequeue */
        tableType = CPSS_DXCH_SIP5_TABLE_TAIL_DROP_DQ_QUEUE_BUF_LIMITS_E;
        hwArray[0] = 0xffffffff;
        hwArray[1] = 0x3;
        rc = prvCpssDxChWriteTableEntry(dev, tableType, i, hwArray);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if (dumpTmErrors)
    {
        rc = tmConditionalDumpTmUnitsErrors(dev, __FILE__, __LINE__, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("tmConditionalDumpTmUnitsErrors", rc);
    }

#else
        rc =  GT_NOT_SUPPORTED;
    /*rc =  GT_OK;  ??? */
#endif /*#if defined (INCLUDE_TM)*/
    return rc;
}



/*--------------------------------------------------------------*
*   BC2 (2-5 DRAM interfaces)                                  *
*   +------------------+------------------+---------+          *
*   |    TM frequency  |  DRAM interfaces | TM BW   |          *
*   |  (MHz)           |    Number        |  Gbps   |          *
*   +------------------+------------------+---------+          *
*   |   933            |      5           |    120  |          *
*   |   933            |      4           |    100  |          *
*   |   933            |      3           |    75   |          *
*   |   933            |      2           |    50   |          *
*   |   800            |      5           |    109  |          *
*   |   800            |      4           |    87   |          *
*   |   800            |      3           |    65   |          *
*   |   800            |      2           |    43   |          *
*   +------------------+------------------+---------+          *
*
*--------------------------------------------------------------*/

#if defined (INCLUDE_TM)
GT_STATUS afterInitBoardConfig_TM_BW_Init
(
    GT_U8 dev
)
{
    GT_STATUS rc;
    GT_U32 tmBandwidthMbps;
    GT_U32 coreClockDB,coreClockHW;
    GT_U32 freqNum;
    GT_U32 ddrFreq;
    GT_U32 tempValue;

    rc = appDemoDbEntryGet("tmBw", &tmBandwidthMbps);
    if(rc != GT_OK)
    {
        rc = cpssDxChHwCoreClockGet(dev,/*OUT*/&coreClockDB,&coreClockHW);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChTMFreqGet(dev, &freqNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDxChTMFreqGet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
        switch(freqNum)
        {
            case 2:
                ddrFreq = 933;
                break;
            case 3:
                ddrFreq = 667;
                break;
            case 1:
                ddrFreq = 800;
                break;
            case 0:
            default:
                cpssOsPrintf("afterInitBoardConfig: illegal freqNum = %d\n", freqNum);
                return GT_BAD_STATE;
        }

        switch (bc2BoardType)
        {
            case APP_DEMO_BC2_BOARD_DB_CNS :
            case APP_DEMO_BC2_BOARD_RD_MSI_CNS:
            case APP_DEMO_BC2_BOARD_RD_MTL_CNS:
                switch(coreClockDB)
                {
                    case 175:
                        tempValue = 55000;
                        break;
                    case 221:
                        tempValue = 69000;
                        break;
                    case 250:
                        tempValue = 79000;
                        break;
                    case 362:
                        tempValue = 114000;
                        break;
                    case 521:
                        tempValue = 130000;
                        break;
                    default:
                        cpssOsPrintf("afterInitBoardConfig: BC2 : illegal coreClockDB = %d\n", coreClockDB);
                        return GT_BAD_STATE;
                }
                tmBandwidthMbps = MIN(bc2DramCfgArr[bc2BoardType].activeInterfaceNum * 25000, tempValue) * ddrFreq / 933;
            break;

            case APP_DEMO_CETUS_BOARD_DB_CNS:
                tmBandwidthMbps = 25000 * ddrFreq / 933;
            break;

            case APP_DEMO_CAELUM_BOARD_DB_CNS:
                switch(coreClockDB)
                {
                    case 365:
                        tempValue = 75000;
                        break;
                    case 250:
                        tempValue = 75000;
                        break;
                    case 200:
                        tempValue = 63000;
                        break;
                    case 167:
                        tempValue = 52000;
                        break;
                    default:
                        cpssOsPrintf("afterInitBoardConfig: illegal coreClockDB = %d\n", coreClockDB);
                        return GT_BAD_STATE;
                }
                tmBandwidthMbps = MIN(bobkDramCfgArr[BOBK_DRAM_CFG_IDX_CAELUM_CNS].activeInterfaceNum * 25000, tempValue) * ddrFreq / 933;
            break;
            default:
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("afterInitBoardConfig_TM_BW_Init() failed : not supported board", GT_NOT_SUPPORTED);
            }
        }
    }

    /* We round down to nearest 1G value (1000Mbps) */
    cpssOsPrintf("System TM Bandwidth(Mbps): %d rounded down to", tmBandwidthMbps);
    tmBandwidthMbps = (tmBandwidthMbps / 1000) * 1000;
    cpssOsPrintf(" %d(Mbps)\n", tmBandwidthMbps);
    rc = cpssDxChPortResourceTmBandwidthSet(dev, tmBandwidthMbps);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortResourceTmBandwidthSet", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}
#else /*#if defined (INCLUDE_TM)*/
/*************************************************************/
GT_STATUS afterInitBoardConfig_TM_BW_Init
(
    GT_U8 dev
)
{
    return GT_NOT_SUPPORTED;
}
#endif /*#if defined (INCLUDE_TM)*/

/**
* @internal appDemoBc2PIPEngineInit function
* @endinternal
*
* @brief   ASIC specific configurations : of PIP engine
*         initialization before port configuration.
* @param[in] dev                      - devNumber
*
* @retval GT_OK                    - on success,
*/
GT_STATUS appDemoBc2PIPEngineInit
(
    GT_U8 dev
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM    portNum;
    /*-------------------------------------------------------
     * CPSS-5826  Oversubscription (PIP) Engine Init Configuration
     * JUST for BC3
     *  1. Enable PIP
     *      cpssDxChPortPipGlobalEnableSet(GT_TRUE); -- default , don't configure
     *
     *
     *  2. Define thresholds :
     *      reg -0 :  0-15 - index 0   PIP Very High Priority
     *      reg -0 : 16-31 - index 1   PIP High Priority
     *      reg -1 :  0-15 - index 2   PIP Med Priority
     *      reg -1 : 16-31 - index 3   PIP Low High Priority
     *
     *
     *
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(0,1839), very high
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(1,1380), high
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(2,920),  medium
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(2,460),  low
     *
     *  3. for each port
     *             cpssDxChPortPipProfileSet(0) -- High Priority  (default)
     *             cpssDxChPortPipTrustEnableSet(FALSE)   -- Not trusted (not default)
     *     end
     *-------------------------------------------------------------------------------*/
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev)) /* BC3 , Aldrin2 */
    {
        typedef struct
        {
            GT_U32 priority;
            GT_U32 threshold;
        }BurstFifoThreshold_STC;

        #define BAD_VALUE (GT_U32)(~0)

        BurstFifoThreshold_STC threshArr[] =
        {
              {         0,       1839 /* "Very High Priority"*/}
             ,{         1,       1380 /* "High Priority"     */}
             ,{         2,        920 /* "Medium Priority"   */}
             ,{         3,        460 /* "Low Priority"      */}
             ,{ BAD_VALUE,  BAD_VALUE /*  NULL               */}
        };

        GT_U32 i;
        /*------------------------------------------------*
         * set thresholds
         *------------------------------------------------*/
        for (i = 0; threshArr[i].priority != BAD_VALUE; i++)
        {
            rc = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS,threshArr[i].priority,threshArr[i].threshold);
            if (rc != GT_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPipGlobalBurstFifoThresholdsSet()", rc);
            }
        }
        /*---------------------------------------------------------*
         * pass over all physical ports and set Pip Untrusted mode *
         *---------------------------------------------------------*/
       for (portNum = 0; portNum < (appDemoPpConfigList[dev].maxPortNumber); portNum++)
       {
            CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);

            if(prvCpssDxChPortRemotePortCheck(dev,portNum))
            {
                continue;
            }
            rc = cpssDxChPortPipTrustEnableSet(dev,portNum,GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPipTrustEnableSet() ", rc);
            }
        }
    }
    return GT_OK;
}

/**
* @internal afterInitBoardConfig function
* @endinternal
*
* @brief   Board specific configurations that should be done after board
*         initialization.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS afterInitBoardConfig
(
    IN  GT_U8       boardRevId
)
{
    GT_STATUS   rc, rc1;
    GT_U8       dev;
    GT_U32      skipPhyInit = 1;
    GT_U32      initSerdesDefaults;
    CPSS_PORTS_BMP_STC  portsMembers; /* VLAN members */
    CPSS_PORTS_BMP_STC  portsTagging; /* VLAN tagging */
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    CPSS_PORTS_BMP_STC  portsAdditionalMembers; /* VLAN members to add */
    GT_BOOL             isValid, isTmSupported, isTmEnabled; /* is Valid flag */
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    CPSS_SYSTEM_RECOVERY_INFO_STC saved_system_recovery;
    CPSS_SYSTEM_RECOVERY_INFO_STC tmp_system_recovery;

#if defined (INCLUDE_TM)
    GT_U32      tmIgnoreHwSupport = 0;
#endif /*#if defined (INCLUDE_TM)*/


#if(defined _linux)
#ifdef CPU_ARMADAXP_3_4_69
#ifndef ASIC_SIMULATION
    GT_U32      regValue;
#endif
#endif
#endif

    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);

#if(defined _linux)
#ifdef CPU_ARMADAXP_3_4_69
#ifndef ASIC_SIMULATION

    rc = hwIfTwsiInitDriver();
    if(GT_OK != rc)
    {
        return rc;
    }
#endif
#endif
#endif

#ifdef LINUX_NOKM
     i2cDrv = extDrvI2cCreateDrv(NULL);
     if( NULL == i2cDrv)
     {
         return GT_NO_RESOURCE;
     }
#endif /* LINUX_NOKM */

    for(dev = SYSTEM_DEV_NUM_MAC(0); dev < SYSTEM_DEV_NUM_MAC(ppCounter); dev++)
    {
        rc = cpssSystemRecoveryStateGet(&system_recovery);
        if (rc != GT_OK)
        {
          return rc;
        }

        if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
        {
            tmp_system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
            tmp_system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
            rc =  cpssSystemRecoveryStateSet(&tmp_system_recovery);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        rc = appDemoIsTmSupported(dev,/*OUT*/&isTmSupported);
        if (rc != GT_OK)
        {
            rc1 =  cpssSystemRecoveryStateSet(&system_recovery);
            if (rc1 != GT_OK)
            {
                return rc1;
            }
            return rc;
        }

        isTmEnabled = appDemoIsTmEnabled();

        if ((isTmSupported) && (isTmEnabled))
        {
            rc = afterInitBoardConfig_TM_BW_Init(dev);
            if(rc != GT_OK)
            {
                rc1 =  cpssSystemRecoveryStateSet(&system_recovery);
                if (rc1 != GT_OK)
                {
                    return rc1;
                }
                return rc;
            }

        }

        rc = appDemoBc2PIPEngineInit(dev);
        if(rc != GT_OK)
        {
            rc1 =  cpssSystemRecoveryStateSet(&system_recovery);
            if (rc1 != GT_OK)
            {
                return rc1;
            }
            return rc;
        }

        /* After all CG MAC related configuration were done,
           it is needed to disable GC MAC UNIT in order to reduce power consumption.
           Only requested CG MACs will be enabled during appDemoBc2PortInterfaceInit */
        if ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
            (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E))
        {
            /* Bobcat3 CG MAC unit disable */
            rc = cpssSystemRecoveryStateGet(&tmp_system_recovery);
            if (rc != GT_OK)
            {
              return rc;
            }
            saved_system_recovery = tmp_system_recovery;

            tmp_system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
            tmp_system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
            rc =  cpssSystemRecoveryStateSet(&tmp_system_recovery);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDxChCgMacUnitDisable(dev, GT_FALSE);
            rc1 =  cpssSystemRecoveryStateSet(&saved_system_recovery);
            if (rc1 != GT_OK)
            {
                return rc1;
            }
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        rc = appDemoBc2PortInterfaceInit(dev, boardRevId);
        if(rc != GT_OK)
        {
            rc1 =  cpssSystemRecoveryStateSet(&system_recovery);
            if (rc1 != GT_OK)
            {
                return rc1;
            }
            return rc;
        }

        if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
        {
            rc =  cpssSystemRecoveryStateSet(&system_recovery);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* clear additional default VLAN members */
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsAdditionalMembers);


        if(isTmSupported == GT_TRUE && isTmEnabled) /* for TM configs only */
        {
            /* if appDemoTmScenarioMode has not changed by appDemoTmScenarioModeSet */
            if(boardRevId == 4 && appDemoTmScenarioMode == 0)
                appDemoTmScenarioMode = CPSS_TM_REV4;
            /*
                when system is initialized with non default ports mapping,
                TM Ports Init is called after custom ports mapping is applied
            */
            if (!isCustomPortsMapping)
            {
                rc = appDemoBc2TmPortsInit(dev);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        else
        {
            if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.gop_ilkn.supported == GT_TRUE)
            {
                GT_PHYSICAL_PORT_NUM    chId;


                /* add ILKN channels to default VLAN, although they are not mapped yet */
                for(chId = 128; chId < 192; chId++)
                {
                    /* set the port as member of vlan */
                    CPSS_PORTS_BMP_PORT_SET_MAC(&portsAdditionalMembers, chId);
                }
            }
            #if defined (INCLUDE_TM)
            if(isTmSupported == GT_TRUE)
            {
                /* enable TM regs access*/
                rc= prvCpssTmCtlGlobalParamsInit(dev);
                if (rc)
                {
                    cpssOsPrintf("%s %d  ERROR : rc=%d\n ",__FILE__, __LINE__ , rc);
                    return rc;
                }
            }
            else if(isTmEnabled)
            {
                appDemoDbEntryGet("tmIgnoreHwSupport", &tmIgnoreHwSupport);

                if(tmIgnoreHwSupport)
                {
                    cpssOsPrintf("Dev: %d is not supported TM, has initialized without TM!\n", dev);
                }
                else
                {
                    cpssOsPrintf("Dev: %d is not supported TM, or wrong TM Dram Frequency settings!\n", dev);
                    return GT_FAIL;
                }
            }
            #endif /*#if defined (INCLUDE_TM)*/
        }
        rc = cpssSystemRecoveryStateGet(&system_recovery);
        if (rc != GT_OK)
        {
          return rc;
        }
        if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
         {
            /* add port 83 to default VLAN, although it could be not mapped */
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsAdditionalMembers, 83);

            /* read VLAN entry */
            rc = cpssDxChBrgVlanEntryRead(dev, 1, &portsMembers, &portsTagging,
                                          &cpssVlanInfo, &isValid, &portsTaggingCmd);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanEntryRead", rc);
            if (rc != GT_OK)
               return rc;

            /* add new ports as members, portsTaggingCmd is default - untagged */
            CPSS_PORTS_BMP_BITWISE_OR_MAC(&portsMembers, &portsMembers, &portsAdditionalMembers);

            /* Write default VLAN entry */
            rc = cpssDxChBrgVlanEntryWrite(dev, 1,
                                           &portsMembers,
                                           &portsTagging,
                                           &cpssVlanInfo,
                                           &portsTaggingCmd);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanEntryWrite", rc);
            if (rc != GT_OK)
               return rc;
        }
        rc = appDemoDbEntryGet("initSerdesDefaults", /*out */&initSerdesDefaults);
        if(rc != GT_OK)
        {
            initSerdesDefaults = 1;
        }

        rc = appDemoDbEntryGet("skipPhyInit", &skipPhyInit);
        if(rc != GT_OK)
        {
            if (initSerdesDefaults != 0) /* if initSerdesDefaults == 0 .i.e. skip port config, */
            {                            /*     than skip also phy config */
                skipPhyInit = 0;
            }
            else
            {
                skipPhyInit = 1;
            }
        }

        if((bc2BoardType == APP_DEMO_CETUS_BOARD_DB_CNS) ||
           (bc2BoardType == APP_DEMO_ALDRIN_BOARD_DB_CNS) ||
           (bc2BoardType == APP_DEMO_BOBCAT3_BOARD_DB_CNS)||
           (bc2BoardType == APP_DEMO_ALDRIN_BOARD_RD_CNS) ||
           (bc2BoardType == APP_DEMO_BOBCAT3_BOARD_RD_CNS) ||
           (bc2BoardType == APP_DEMO_ARMSTRONG_BOARD_RD_CNS))
        {
            /* board without PHY */
            skipPhyInit = 1;
        }

        if(skipPhyInit != 1)
        {
            /* configure PHYs */
            if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
            {
                rc = bobcat2BoardPhyConfig(boardRevId, dev);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("bobcat2BoardPhyConfig", rc);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        /* PTP (and TAIs) configurations */
        if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
        {
            /* PTP (and TAIs) configurations */
            rc = appDemoB2PtpConfig(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("prvBobcat2PtpConfig", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }


        #if defined (INCLUDE_TM)
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.trafficManager.supported)
        {
            /* Enable Ingress Flow Control for TM */
            rc = cpssDxChTmGluePfcResponseModeSet(dev, 0, CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E);


            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGluePfcResponseModeSet", rc);
        }
        #endif /*#if defined (INCLUDE_TM)*/

#if(defined _linux)
#ifdef CPU_ARMADAXP_3_4_69
#ifndef ASIC_SIMULATION

        /* Fix I2C access (for some random registers) from Armada XP card */
        rc = cpssDrvPpHwInternalPciRegRead(dev, 0, 0x1108c, &regValue);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDrvPpHwInternalPciRegRead", rc);

        /* Set '0' to bit_18 */
        U32_SET_FIELD_MAC(regValue, 18, 1, 0);

        rc = cpssDrvPpHwInternalPciRegWrite(dev, 0, 0x1108c, regValue);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDrvPpHwInternalPciRegWrite", rc);
#endif
#endif
#endif

    }
#define RUN_AN_WA 1

    if(GT_TRUE == RUN_AN_WA)
    {
        rc = AN_WA_Task_Create();
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal afterInitBoardConfigSimple function
* @endinternal
*
* @brief   Board specific configurations that should be done after board
*         initialization.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of afterInitBoardConfig.
*
*/
static GT_STATUS afterInitBoardConfigSimple
(
    IN  GT_U8       dev,
    IN  GT_U8       boardRevId
)
{
    GT_STATUS   rc;
    GT_BOOL     isTmEnabled;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        rc = appDemoBc2PortInterfaceInit(dev, boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("afterInitBoardConfigSimple", rc);
        if(rc != GT_OK)
            return rc;
    }

    isTmEnabled = appDemoIsTmEnabled();

    if(isTmEnabled) /* for TM configs only */
    {
        #if defined (INCLUDE_TM)
            appDemoBc2DramOrTmInit(dev, 0, NULL);

            osPrintf("build TM tree after init\n");

            /* TM related configurations */
            rc = prvBobcat2TmGlueConfig(dev);
        #else
            rc =  GT_NOT_SUPPORTED;
        #endif /*#if defined (INCLUDE_TM)*/



        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvBobcat2TmGlueConfig", rc);
        if (rc != GT_OK)
            return rc;
    }
#if 0
    rc = appDemoDbEntryGet("initSerdesDefaults", /*out */&initSerdesDefaults);
    if(rc != GT_OK)
    {
        initSerdesDefaults = 1;
    }
#endif
    /* configure PHYs */
    rc = bobcat2BoardPhyConfig(boardRevId, dev);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("boardPhyConfig", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
    {
        /* TBD: FE HA-3259 fix and removed from CPSS.
           Allow to the CPU to get the original vlan tag as payload after
           the DSA tag , so the info is not changed. */
        rc = cpssDxChBrgVlanForceNewDsaToCpuEnableSet(dev, GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanForceNewDsaToCpuEnableSet", rc);
        if( GT_OK != rc )
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanForceNewDsaToCpuEnableSet", rc);
            return rc;
        }

        /* RM of Bridge default values of Command registers have fixed in B0.
           But some bits need to be changed to A0 values.
           set bits 15..17 in PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).bridgeEngineConfig.bridgeCommandConfig0 */
        /* set the command of 'SA static moved' to be 'forward' because this command
           applied also on non security breach event ! */
        rc = cpssDxChBrgSecurBreachEventPacketCommandSet(dev,
            CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
            CPSS_PACKET_CMD_FORWARD_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgSecurBreachEventPacketCommandSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    #if defined (INCLUDE_TM)
    if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.trafficManager.supported)
    {
        /* Enable Ingress Flow Control for TM */
            rc = cpssDxChTmGluePfcResponseModeSet(dev, 0, CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGluePfcResponseModeSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    #endif /*#if defined (INCLUDE_TM)*/

    /* set VLAN 1 for TM check - if you don't want - comment it out */
    {
        CPSS_PORTS_BMP_STC  portsMembers;
        CPSS_PORTS_BMP_STC  portsTagging;
        CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
        CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
        GT_U8               port;           /* current port number      */

        /* Fill Vlan info */
        osMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
        /* default IP MC VIDX */
        cpssVlanInfo.unregIpmEVidx = 0xFFF;
        cpssVlanInfo.naMsgToCpuEn           = GT_TRUE;
        cpssVlanInfo.floodVidx              = 0xFFF;
        cpssVlanInfo.fidValue = 1;

        /* Fill ports and tagging members */
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
        osMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

        /* set all ports as VLAN members */
        for (port = 0; port < 48; port++)
        {
            CPSS_ENABLER_PORT_SKIP_CHECK(dev,port);

            /* set the port as member of vlan */
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, port);

            /* Set port pvid */
            rc = cpssDxChBrgVlanPortVidSet(dev, port, CPSS_DIRECTION_INGRESS_E,1);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanPortVidSet", rc);
            if(rc != GT_OK)
                 return rc;

            portsTaggingCmd.portsCmd[port] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        }

        /* Write default VLAN entry */
        rc = cpssDxChBrgVlanEntryWrite(dev, 1,
                                       &portsMembers,
                                       &portsTagging,
                                       &cpssVlanInfo,
                                       &portsTaggingCmd);

        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanEntryWrite", rc);
        if (rc != GT_OK)
           return rc;

    }


    return GT_OK;
}



/**
* @internal gtDbDxBobcat2BoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbDxBobcat2BoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    /* next static are and maybe changed from compilation time */
    ppCounter = 0;
    bc2BoardType = APP_DEMO_BC2_BOARD_DB_CNS;
    bc2BoardResetDone = GT_TRUE;
    dxChInitRegDefaults = GT_FALSE;
    return GT_OK;
}

/**
* @internal appDemoTmScenarioModeSet function
* @endinternal
*
* @brief   set tm scenario for 29,2
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] mode                     - tm scenario mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoTmScenarioModeSet
(
    IN  CPSS_TM_SCENARIO  mode
)
{
    appDemoTmScenarioMode = mode;
    return GT_OK;
}


/*------------------------------------------------------------------------------------------------------------------------
 *  /Cider/EBU/Bobcat2B/Bobcat2 {Current}/Reset and Init Controller/DFX Server Units - BC2 specific registers/Device SAR2
 *  address : 0x000F8204
 *  15-17  PLL_2_Config (Read-Only) TM clock frequency
 *    0x0 = Disabled;  Disabled; TM clock is disabled
 *    0x1 = 400MHz;    TM runs 400MHz, DDR3 runs 800MHz
 *    0x2 = 466MHz;    TM runs 466MHz, DDR3 runs 933MHz
 *    0x3 = 333MHz;    TM runs 333MHz, DDR3 runs 667MHz
 *    0x5 = Reserved0; TM runs from core clock, DDR3 runs 800MHz.; (TM design does not support DDR and TM_core from different source)
 *    0x6 = Reserved1; TM runs from core clock, DDR3 runs 933MHz.; (TM design does not support DDR and TM_core from different source)
 *    0x7 = Reserved; Reserved; PLL bypass
 *
 *    if SAR2 - PLL2  is 0 (disable)  , do we need to configure TM ????
 *    is TM supported ? probably not !!!
 *------------------------------------------------------------------------------------------------------------------------*/

GT_STATUS appDemoIsTmSupported(GT_U8 devNum, GT_BOOL * isSupportedPtr)
{


    #if defined (INCLUDE_TM)

    GT_STATUS rc;
    GT_BOOL isSupported;
    GT_U32  tmFreq;

    if(!appDemoPpConfigList[devNum].valid)
    {
        osPrintf("appDemoIsTmSupported: wrong devNum: %d\n", devNum);
        isSupported =  GT_FALSE;
    }
    else
    {
        isSupported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.TmSupported;
    }

    if (isSupported == GT_TRUE)
    {
        rc = prvCpssDxChTMFreqGet(devNum,/*OUT*/&tmFreq);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (tmFreq == 0)
        {
            isSupported =  GT_FALSE;
        }
    }
    *isSupportedPtr = isSupported;

    #else
    *isSupportedPtr = GT_FALSE;
    #endif /*#if defined (INCLUDE_TM)*/
    return GT_OK;

}

/* is TM init requiered by initSystem */
GT_BOOL appDemoIsTmEnabled(void)
{
    GT_BOOL retVal;

    switch (bc2BoardRevId)
    {
        case 2:
        case 4:
        case 6:
        case 12:
            retVal = GT_TRUE;
            break;
        default:
            retVal = GT_FALSE;
    }

    return retVal;
}


GT_STATUS portsConfigTest(GT_VOID)
{
    GT_STATUS   rc;
    GT_U32  portNum;
    CPSS_PORTS_BMP_STC portsBmp;
    GT_U32  i;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    for(portNum = 0; portNum < 48; portNum++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);
    }

    for(i = 0; i < 50; i++)
    {
        rc = cpssDxChPortModeSpeedSet(0, &portsBmp, GT_TRUE,
                                      CPSS_PORT_INTERFACE_MODE_QSGMII_E,
                                      CPSS_PORT_SPEED_1000_E);
        if(rc != GT_OK)
        {
            cpssOsPrintf("!!!qsgmii failed!!!\n");
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal cpssInitSimpleBobcat2 function
* @endinternal
*
* @brief   This is the main board initialization function for BC2 device.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/

static GT_STATUS cpssInitSimpleBobcat2
(
    IN  GT_U8  boardRevId
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       numOfPp;        /* Number of Pp's in system.                */
    GT_U32      start_sec  = 0;
    GT_U32      start_nsec = 0;
    GT_U32      end_sec  = 0;
    GT_U32      end_nsec = 0;
    GT_U32      diff_sec;
    GT_U32      diff_nsec;

    GT_U8                   devNum = 0;
    GT_U8                   hwDevNum = 16;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_INTERFACE_INFO_STC physicalInfo;
    GT_32                   intKey;     /* Interrupt key     */
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   cpssPpPhase1Info;     /* CPSS phase 1 PP params */
    GT_PCI_INFO pciInfo;
    CPSS_PP_DEVICE_TYPE     devType;
    static CPSS_REG_VALUE_INFO_STC regCfgList[] = GT_DUMMY_REG_VAL_INFO_LIST;            /* register values */
    GT_U32                  regCfgListSize;         /* Number of config functions for this board    */
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC   cpssPpPhase2Info;
    CPSS_DXCH_PP_CONFIG_INIT_STC    ppLogInitParams;
    GT_U32  numOfEports, numOfPhysicalPorts;

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */

    rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
        return rc;

#if (defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)
    /*simulation initialization*/
    appDemoRtosOnSimulationInit();

#endif /*(defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)*/

    /* Enable printing inside interrupt routine - supplied by extrernal drive */
    extDrvUartInit();

    /* Call to fatal_error initialization, use default fatal error call_back - supplied by mainOs */
    rc = osFatalErrorInit((FATAL_FUNC_PTR)NULL);
    if (rc != GT_OK)
        return rc;
    osMemSet(&cpssPpPhase1Info, 0, sizeof(cpssPpPhase1Info));

    /* this function finds all Prestera devices on PCI bus */
    getBoardInfoSimple(boardRevId,
        &numOfPp,
        &cpssPpPhase1Info,
        &pciInfo);
    if (rc != GT_OK)
        return rc;

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    rc = getPpPhase1ConfigSimple(boardRevId, devNum, &cpssPpPhase1Info);
    if (rc != GT_OK)
        return rc;

    osPrintf("cpssDxChHwPpPhase1Init\n");
    /* devType is retrieved in hwPpPhase1Part1*/
    rc = cpssDxChHwPpPhase1Init(&cpssPpPhase1Info, &devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpPhase1Init", rc);
    if (rc != GT_OK)
        return rc;


    /* should be 10 */
    regCfgListSize = (sizeof(regCfgList)) /
                       (sizeof(CPSS_REG_VALUE_INFO_STC));

    rc = cpssDxChHwPpStartInit(devNum, GT_FALSE, regCfgList, regCfgListSize);
    if (rc != GT_OK)
        return rc;

    rc = configBoardAfterPhase1Simple(boardRevId, devNum);
    if (rc != GT_OK)
        return rc;

    /* memory related data, such as addresses and block lenghts, are set in this funtion*/
    rc = getPpPhase2ConfigSimple(boardRevId, devNum, devType, &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;

    /* Lock the interrupts, this phase changes the interrupts nodes pool data */
    extDrvSetIntLockUnlock(INTR_MODE_LOCK, &intKey);

    rc = cpssDxChHwPpPhase2Init(devNum,  &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;


    rc = cpssDxChCfgHwDevNumSet(cpssPpPhase2Info.newDevNum, hwDevNum);
    if (rc != GT_OK)
        return rc;

    /* this is close to application job, if you want - you may remove this part*/
    /* set all hwDevNum in E2Phy mapping table for all ePorts */
    /* Enable configuration of drop for ARP MAC SA mismatch due to check per port */

    /* Loop on the first 256 (num of physical ports , and CPU port (63)) entries of the table */
    numOfPhysicalPorts =
        PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(cpssPpPhase2Info.newDevNum);

    for(portNum=0; portNum < numOfPhysicalPorts; portNum++)
    {
        rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(cpssPpPhase2Info.newDevNum,
                                                                 portNum,
                                                                 &physicalInfo);
        if(rc != GT_OK)
            return rc;

        /* If <Target Is Trunk> == 0 && <Use VIDX> == 0 */
        if(physicalInfo.type == CPSS_INTERFACE_PORT_E)
        {
            physicalInfo.devPort.hwDevNum = hwDevNum;
            rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(cpssPpPhase2Info.newDevNum,
                                                                     portNum,
                                                                     &physicalInfo);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", rc);
            if(rc != GT_OK)
                return rc;

            /* ARP MAC SA mismatch check per port configuration enabling */
            rc = cpssDxChBrgGenPortArpMacSaMismatchDropEnable(cpssPpPhase2Info.newDevNum,
                                                              portNum,
                                                              GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgGenPortArpMacSaMismatchDropEnable", rc);
            if(rc != GT_OK)
                return rc;
        }
    }

    /* Port Isolation is enabled if all three configurations are enabled:
       In the global TxQ registers, AND
       In the eVLAN egress entry, AND
       In the ePort entry

       For legacy purpose loop on all ePort and Trigger L2 & L3 Port
       Isolation filter for all ePorts */

        numOfEports = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(cpssPpPhase2Info.newDevNum);
    for(portNum=0; portNum < numOfEports; portNum++)
    {
        rc = cpssDxChNstPortIsolationModeSet(cpssPpPhase2Info.newDevNum,
                                             portNum,
                                             CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChNstPortIsolationModeSet", rc);
        if(rc != GT_OK)
            return rc;

        /* for legacy : enable per eport <Egress eVLAN Filtering Enable>
           because Until today there was no enable bit, egress VLAN filtering is
           always performed, subject to the global <BridgedUcEgressFilterEn>. */
        rc = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(cpssPpPhase2Info.newDevNum,
                                                          portNum,
                                                          GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgEgrFltVlanPortFilteringEnableSet", rc);
        if(rc != GT_OK)
            return rc;
    }

    extDrvSetIntLockUnlock(INTR_MODE_UNLOCK, &intKey);

/* we forced it to be DB board, change it for a different one if needed */
    rc = configBoardAfterPhase2Simple(BOARD_REV_ID_DB_E, devNum);
    if(rc != GT_OK)
        return rc;

    osMemSet(&ppLogInitParams ,0, sizeof(ppLogInitParams));
    rc = getPpLogicalInitParamsSimple(boardRevId, devNum, &ppLogInitParams);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChCfgPpLogicalInit(devNum, &ppLogInitParams);
    if(rc != GT_OK)
        return rc;

/* LIBs to initialize - use these functions as refference
    prvIpLibInit
    prvPclLibInit
    prvTcamLibInit
    */
 /*   getTapiLibInitParams
    localLibInitParams.initTcam                  = GT_TRUE;
    localLibInitParams.initNst                   = GT_TRUE;
 */

/* SMI init is a must in order to configure PHY's */
    rc = cpssDxChPhyPortSmiInit(devNum);
    if( rc != GT_OK)
        return rc;

/* specific board features configurations - see appDemoDxPpGeneralInit, afterInitBoardConfig,
appDemoEventRequestDrvnModeInit for reference*/

    /* Enable ports */
    for (portNum = 0; portNum < 0x53; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        rc = cpssDxChPortEnableSet(devNum, portNum, GT_TRUE);
        if (GT_OK != rc)
            return rc;
    }

    /* Enable device */
    rc = cpssDxChCfgDevEnable(devNum, GT_TRUE);
    if (GT_OK != rc)
        return rc;

    /* after board init configurations are relevant only for TM case */
    if(boardRevId == 6)
    {
        rc = afterInitBoardConfigSimple(devNum, 2);
        if (GT_OK != rc)
            return rc;
    }

    /* CPSSinit time measurement */

    rc = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    if(rc != GT_OK)
    {
        return rc;
    }

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }
    cpssOsPrintf("cpssInitSystem time: %d sec., %d nanosec.\n", seconds, nanoSec);


    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;

    cpssOsPrintf("Time processing the cpssInitSimple (from 'phase1 init') is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    return rc;
}


/**
* @internal gtDbDxBobcat2BoardReg function
* @endinternal
*
* @brief   Registration function for the BobCat2 (SIP5) board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxBobcat2BoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);

    if(boardCfgFuncs == NULL)
    {
        return GT_FAIL;
    }

    bc2BoardRevId = boardRevId;

    if((boardRevId == 5) || (boardRevId == 6) )
    {
        boardCfgFuncs->boardSimpleInit =  cpssInitSimpleBobcat2;
        return GT_OK;
    }

    boardCfgFuncs->boardGetInfo                 = getBoardInfo;
    boardCfgFuncs->boardGetPpPh1Params          = getPpPhase1Config;
    boardCfgFuncs->boardAfterPhase1Config       = configBoardAfterPhase1;
    boardCfgFuncs->boardGetPpPh2Params          = getPpPhase2Config;
    boardCfgFuncs->boardAfterPhase2Config       = configBoardAfterPhase2;
    boardCfgFuncs->boardGetPpLogInitParams      = getPpLogicalInitParams;
    boardCfgFuncs->boardGetLibInitParams        = getTapiLibInitParams;
    boardCfgFuncs->boardAfterInitConfig         = afterInitBoardConfig;
    boardCfgFuncs->deviceAfterInitConfig        = afterInitDeviceConfig;
    boardCfgFuncs->deviceAfterPhase1            = configDeviceAfterPhase1;
    boardCfgFuncs->deviceAfterPhase2            = configDeviceAfterPhase2;
    boardCfgFuncs->boardCleanDbDuringSystemReset= gtDbDxBobcat2BoardCleanDbDuringSystemReset;
    boardCfgFuncs->deviceCleanup              = getPpPhase1ConfigClear;

    return GT_OK;
}


/**
* @internal appDemoBc2AnPhyMode function
* @endinternal
*
* @brief   Configure Auto-Negotiation Phy Mode when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
* @param[in] portSpeed                - port speed (0 - 10Mbps, 1 - 100Mbps, 2 - 1000Mbps)
* @param[in] portDuplexMode           - port duplex mode (0 - Full Duplex, 1 - Half Duplex)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS appDemoBc2AnPhyMode
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed,
    IN  CPSS_PORT_DUPLEX_ENT    portDuplexMode
)
{
    GT_STATUS                               rc;
    CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC portAnAdvertisment;
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;
    CPSS_PORTS_BMP_STC                      portsBmp;
    GT_U32                                  portMacMap;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if((portSpeed != CPSS_PORT_SPEED_10_E) && (portSpeed != CPSS_PORT_SPEED_100_E) &&
            (portSpeed != CPSS_PORT_SPEED_1000_E))
        {
            return GT_BAD_PARAM;
        }
        if((portDuplexMode != CPSS_PORT_FULL_DUPLEX_E) && (portDuplexMode != CPSS_PORT_HALF_DUPLEX_E))
        {
            return GT_BAD_PARAM;
        }

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);

        rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode, portSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortAutoNegMasterModeEnableSet(devNum, portNum, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
        portAnAdvertisment.link = GT_TRUE;
        portAnAdvertisment.duplex = portDuplexMode;
        portAnAdvertisment.speed = portSpeed;
        rc = cpssDxChPortAutoNegAdvertismentConfigSet(devNum, portNum, &portAnAdvertisment);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPortInBandAutoNegBypassEnableSet(devNum, portNum, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPortInbandAutoNegEnableSet(devNum, portNum, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPortInbandAutoNegRestart(devNum,portNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }
    else
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }
}

#ifndef ASIC_SIMULATION


extern MTD_STATUS mtdParallelEraseFlashImage
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 ports[],
    IN MTD_U8 appData[],
    IN MTD_U32 appSize,
    IN MTD_U8 slaveData[],
    IN MTD_U32 slaveSize,
    IN MTD_U16 numPorts,
    OUT MTD_U16 erroredPorts[],
    OUT MTD_U16 *errCode
);

GT_STATUS mtlPhyFlashErase
(
    GT_VOID
)
{
    CPSS_OS_FILE fd = 0, fd_slave = 0;
    GT_U8 *buffer = NULL, *buffer_slave = NULL;
    GT_U32 nmemb, nmemb_slave;
    int             ret;
    GT_U16          status;

    GT_U16 phyAddr[32] = {  0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0 };


    GT_U16 errorPorts[32] = {   0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0 };

    GT_PHYSICAL_PORT_NUM numPort = 0;
    PRESTERA_INFO switchInfo = {0,0};
    GT_U32  phyType;

    char file_name[64];
    char file_name_slave[64];

    ret = appDemoDbEntryGet("phyType", &phyType);
    if (ret != GT_OK)
    {
        phyType = 3240;
    }

    if(3240 == phyType)
    {
        cpssOsSprintf(file_name, "%s", "88X32XX-FW.hdr");
        cpssOsSprintf(file_name_slave, "%s", "x3240flashdlslave_0_4_0_0_6270.bin");
    }
    else if(3340 == phyType)
    {
        cpssOsSprintf(file_name, "%s", "88X33xx-Z2-FW.hdr");
        cpssOsSprintf(file_name_slave, "%s", "flashdownloadslave.bin");
    }
    else
    {
        cpssOsPrintf("not supported PHY type\n");
        return GT_NOT_SUPPORTED;
    }

    /* create phy address array */
    if((APP_DEMO_CETUS_BOARD_DB_CNS == bc2BoardType) && (3240 == phyType))
    {
        phyAddr[0] = 2;
        phyAddr[1] = 6;
        phyAddr[2] = 10;
    }
    else
    {
        phyAddr[0] = 0;
        phyAddr[1] = 4;
        phyAddr[2] = 8;
    }

    numPort = (APP_DEMO_CETUS_BOARD_DB_CNS == bc2BoardType) ? 2 : 3;

    cpssOsPrintf("Load firmware file %s\n", file_name);

    fd = cpssOsFopen(file_name, "r");
    if (0 == fd)
    {
        cpssOsPrintf("open %s fail \n", file_name);
        return GT_ERROR;
    }

    nmemb = cpssOsFgetLength(fd);
    if (nmemb <= 0)
    {
        cpssOsPrintf("open %s fail \n", file_name);
        return GT_ERROR;
    }

    /* for store image */
    buffer = cpssOsMalloc(216 * 1024);
    if (NULL == buffer)
    {
        cpssOsPrintf(" buffer allocation fail \n");
        return GT_ERROR;
    }

    ret = cpssOsFread(buffer, 1, nmemb, fd);
    if (ret < 0)
    {
        cpssOsPrintf("Read from file fail\n");
        return GT_ERROR;
    }

    cpssOsPrintf("Load slave bin file %s\n", file_name_slave);
    fd_slave = cpssOsFopen(file_name_slave, "r");
    if (0 == fd_slave)
    {
        cpssOsPrintf("open %s fail \n", file_name_slave);
        return GT_ERROR;
    }

    nmemb_slave = cpssOsFgetLength(fd_slave);
    if (nmemb_slave <= 0)
    {
        cpssOsPrintf("slave bin length fail \n");
        return GT_ERROR;
    }

    /* 20KB for store slave image */
    buffer_slave = cpssOsMalloc(20 * 1024);
    if (NULL == buffer_slave)
    {
        cpssOsPrintf(" slave buffer allocation fail \n");
        return GT_ERROR;
    }

    ret = cpssOsFread(buffer_slave, 1, nmemb_slave, fd_slave);
    if (ret < 0)
    {
        cpssOsPrintf("Read from slave bin file fail\n");
        return GT_ERROR;
    }

    cpssOsPrintf("MTL downloading firmware to SPI flash phyAddr=%x,numPort=%d ...", phyAddr[0], numPort);
    return mtdParallelEraseFlashImage(&switchInfo, phyAddr, buffer, nmemb,
                                            buffer_slave, nmemb_slave,
                                            (GT_U16)numPort, errorPorts,
                                            &status);
}

#endif

/**
* @internal configBoardAfterPhase1MappingGetCetus function
* @endinternal
*
* @brief   This function gets relevant mapping for boardRevId,
*         if not found , selects default
* @param[in] boardRevId               - The board revision Id.
*                                      revId2PortMapList - mapping list
*                                      listSize          - mapping list size
*
* @param[out] mappingPtrPtr            - pointer to found mapping
* @param[out] mappingPtrPtr            - pointer to mapping size
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on bad ptr
* @retval GT_BAD_PARAM             - on empty list (list size = 0)
*
* @note Special version of configBoardAfterPhase1MappingGet()
*       to return correct mapping for Cetus during fastboot test
*
*/
GT_STATUS configBoardAfterPhase1MappingGetCetus
(
    IN  GT_U8                             boardRevId,
    OUT CPSS_DXCH_PORT_MAP_STC          **mappingPtrPtr,
    OUT GT_U32                           *mappingSizePtr
)
{
    *mappingPtrPtr = &cetusDefaultMap_56to59_62_64to71[0];
    *mappingSizePtr = sizeof(cetusDefaultMap_56to59_62_64to71)/sizeof(cetusDefaultMap_56to59_62_64to71[0]);

    return GT_OK;
}

/**
* @internal configBoardAfterPhase1MappingGetAldrin function
* @endinternal
*
* @brief   This function gets relevant mapping for boardRevId,
*         if not found , selects default
* @param[in] boardRevId               - The board revision Id.
*                                      revId2PortMapList - mapping list
*                                      listSize          - mapping list size
*
* @param[out] mappingPtrPtr            - pointer to found mapping
* @param[out] mappingPtrPtr            - pointer to mapping size
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on bad ptr
* @retval GT_BAD_PARAM             - on empty list (list size = 0)
*
* @note Special version of configBoardAfterPhase1MappingGet()
*       to return correct mapping for Aldrin during fastboot test
*
*/
GT_STATUS configBoardAfterPhase1MappingGetAldrin
(
    IN  GT_U8                             boardRevId,
    OUT CPSS_DXCH_PORT_MAP_STC          **mappingPtrPtr,
    OUT GT_U32                           *mappingSizePtr
)
{
    if(cpssDeviceRunCheck_onEmulator() &&
       (0 == cpssDeviceRunCheck_onEmulator_isAldrinFull()))
    {
        /* limited Aldrin without all ports */
        *mappingPtrPtr = &onEmulator_aldrinDefaultMap[0];
        *mappingSizePtr = sizeof(onEmulator_aldrinDefaultMap)/sizeof(onEmulator_aldrinDefaultMap[0]);
    }
    else
    {
        *mappingPtrPtr = &aldrinDefaultMap[0];
        *mappingSizePtr = sizeof(aldrinDefaultMap)/sizeof(aldrinDefaultMap[0]);
    }

    return GT_OK;
}

/**
* @internal configBoardAfterPhase1MappingGetBobcat3 function
* @endinternal
*
* @brief   This function gets relevant mapping for boardRevId,
*         if not found , selects default
* @param[in] boardRevId               - The board revision Id.
*                                      revId2PortMapList - mapping list
*                                      listSize          - mapping list size
*
* @param[out] mappingPtrPtr            - pointer to found mapping
* @param[out] mappingPtrPtr            - pointer to mapping size
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on bad ptr
* @retval GT_BAD_PARAM             - on empty list (list size = 0)
*
* @note Special version of configBoardAfterPhase1MappingGet()
*       to return correct mapping for Bobcat3 during fastboot test
*
*/
GT_STATUS configBoardAfterPhase1MappingGetBobcat3
(
    IN  GT_U8                             boardRevId,
    OUT CPSS_DXCH_PORT_MAP_STC          **mappingPtrPtr,
    OUT GT_U32                           *mappingSizePtr
)
{
    *mappingPtrPtr = &bc3defaultMap[0];
    *mappingSizePtr = sizeof(bc3defaultMap)/sizeof(bc3defaultMap[0]);

    return GT_OK;
}

/**
* @internal configBoardAfterPhase1MappingGetAldrin2 function
* @endinternal
*
* @brief   This function gets relevant mapping for boardRevId,
*         if not found , selects default
* @param[in] boardRevId               - The board revision Id.
*                                      revId2PortMapList - mapping list
*                                      listSize          - mapping list size
*
* @param[out] mappingPtrPtr            - pointer to found mapping
* @param[out] mappingPtrPtr            - pointer to mapping size
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on bad ptr
* @retval GT_BAD_PARAM             - on empty list (list size = 0)
*
* @note Special version of configBoardAfterPhase1MappingGet()
*       to return correct mapping for Aldrin2 during fastboot test
*
*/
GT_STATUS configBoardAfterPhase1MappingGetAldrin2
(
    IN  GT_U8                             boardRevId,
    OUT CPSS_DXCH_PORT_MAP_STC          **mappingPtrPtr,
    OUT GT_U32                           *mappingSizePtr
)
{
    *mappingPtrPtr = &aldrin2defaultMap[0];
    *mappingSizePtr = sizeof(aldrin2defaultMap)/sizeof(aldrin2defaultMap[0]);

    return GT_OK;
}

GT_STATUS bc3RoadShowInitSystem(GT_VOID)
{

    GT_STATUS rc;

    GT_U8 dev = 0;

    static PortInitList_STC bc3_roadShow[] =
    {
        { PORT_LIST_TYPE_LIST,      {0,4,8,12,16,20,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
       ,{ PORT_LIST_TYPE_INTERVAL,  {24,58,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
       ,{ PORT_LIST_TYPE_INTERVAL,  {79,83,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
       ,{ PORT_LIST_TYPE_INTERVAL,  {64,71,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
       ,{ PORT_LIST_TYPE_INTERVAL,  {72,73,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
       ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_QSGMII_E }
    };

    rc = appDemoDbEntryAdd("initSerdesDefaults", 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBc2PortListInit(dev, bc3_roadShow, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

GT_STATUS appDemoPrintLinkChangeFlagSet
(
    IN GT_U32   enable
);

#ifdef LINUX_NOKM
extern void NOKM_DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(GT_U32 new_milisec_pollingSleepTime);
#endif /*LINUX_NOKM*/
extern GT_STATUS autoInitSystem(void);

/* running Aldrin on emulator */
static GT_STATUS internal_onEmulator(GT_BOOL doInitSystem)
{
    /*appDemoTraceHwAccessEnable(0,2,1);*/

    /* shorten the number of memory accessing */
    appDemoDbEntryAdd ("noDataIntegrityShadowSupport",1);
    /* do not try to init the PHY libary */
    appDemoDbEntryAdd ("initPhy",0);
    /* emulate the 'DB' board ... not I2C to get it from ! */
    appDemoDbEntryAdd ("bc2BoardType",EXT_DRV_ALDRIN_DB_ID);

    appDemoPrintLinkChangeFlagSet(GT_TRUE);

#ifdef LINUX_NOKM
    NOKM_DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(200/*200 mills*/);/*do 5 iterations in second*/
#endif /*LINUX_NOKM*/

    /* state the CPSS/driver/HWS that the 'EMULATOR' is used */
    cpssDeviceRunSet_onEmulator();
#ifdef CPSS_LOG_ENABLE
    /* enable CPSS LOG */
    cpssLogEnableSet (1);
    /* enable LOG for ERRORs */
    cpssLogLibEnableSet (CPSS_LOG_LIB_ALL_E/*43 was 41*/,CPSS_LOG_TYPE_ERROR_E/*4*/,1);
#endif /*CPSS_LOG_ENABLE*/
    if(doInitSystem == GT_FALSE)
    {
        return GT_OK;
    }
    /* do the initialization */
    return autoInitSystem();
}

extern void prvCpssDrvTraceHwWriteBeforeAccessSet(IN GT_BOOL BeforeAccess);
extern void cpssDrvAddrComp_millisecSleepSet(IN GT_U32 timeout);


GT_STATUS aldrin_onEmulator(GT_VOID)
{
    return internal_onEmulator(GT_TRUE/*with cpssInitSystem*/);
}


GT_STATUS aldrin_onEmulator_quick(GT_U32    timeout)
{
    appDemoTraceHwAccessEnable(0,2,1);
    prvCpssDrvTraceHwWriteBeforeAccessSet(1);
    appDemoDbEntryAdd ("initSystemWithoutInterrupts",1);
    cpssDrvAddrComp_millisecSleepSet(timeout);
    return aldrin_onEmulator();
}




GT_STATUS aldrin_onEmulator_prepare(GT_VOID)
{
    return internal_onEmulator(GT_FALSE/*without cpssInitSystem*/);
}

GT_STATUS aldrin2_onEmulator_prepare(GT_VOID)
{
    GT_STATUS rc;
    rc = aldrin_onEmulator_prepare();
    if(rc != GT_OK)
    {
        return rc;
    }

    /************************************************************/
    /* ADD HERE extra or to override what was needed for Aldrin */
    /************************************************************/

    return GT_OK;
}


extern void trace_ADDRESS_NOT_SUPPORTED_MAC_set(GT_U32 enable);
extern GT_STATUS appDemoTrace_skipTrace_onAddress(
    IN GT_U32   address,
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
);

static GT_U32 falcon_emulator_deviceId = CPSS_98CX8520_CNS;/* default for 3.2T device */

GT_STATUS falcon_onEmulator_prepare(GT_VOID)
{
    GT_STATUS rc;
    rc =  internal_onEmulator(GT_FALSE/*without cpssInitSystem*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /************************************************/
    /* ADD HERE extra/override if needed for Falcon */
    /************************************************/
    if(falcon_emulator_deviceId)
    {
        /* Currently read value 0x8600 for device ID, not good for 3.2T device */
        appDemoDebugDeviceIdSet(0, falcon_emulator_deviceId);
    }

    /*
        allow to see Addresses that CPSS not access because forbidden in the Emulator
    */
    trace_ADDRESS_NOT_SUPPORTED_MAC_set(1);


    appDemoTrace_skipTrace_onAddress(0x1d000030/*address*/,0/*index*/);

    /* Emulator RTLF 2.0 uses Cider of '18_06_06' -> June 2018 */
    prvCpssDxchFalconCiderVersionSet(PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E);

#ifdef ASIC_SIMULATION
    /* the WM not hold 2 versions */
    /* WM uses Cider of '18_07_09' -> RTLF 3.0 */
    prvCpssDxchFalconCiderVersionSet(PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_07_09_E );
#endif /*ASIC_SIMULATION*/

#ifdef CPSS_LOG_ENABLE
    /**********************************/
    /* The Falcon got into maturity ! */
    /* we not need by default the     */
    /* enabling of errors !!!         */
    /**********************************/

    /* disable CPSS LOG */
    cpssLogEnableSet (0);
    /* disable LOG for ERRORs */
    cpssLogLibEnableSet (CPSS_LOG_LIB_ALL_E,CPSS_LOG_TYPE_ERROR_E,0);
#endif /*CPSS_LOG_ENABLE*/

    return GT_OK;
}

GT_STATUS falcon_onEmulator_prepare_tiles(IN GT_U32    numOfTiles)
{
    if(numOfTiles > 1)
    {
        /* indicate to use 'actual value' */
        falcon_emulator_deviceId = 0;
    }

    appDemoDbEntryAdd("numOfTiles", numOfTiles);

    return falcon_onEmulator_prepare();
}


#ifdef LINUX_NOKM
GT_STATUS bc2I2cReadSampleTest(
    IN GT_U8    bus_id,
    IN GT_U8    slave_address,
    IN EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN GT_U32   offset,
    IN GT_U32   buffer_size
)
{
    GT_STATUS rc;
    GT_U32  addrSpace;
    GT_U8  buffer[256];
    GT_U32 ii;

    if(buffer_size > 256)
    {
        cpssOsPrintf("buffer_size (%d) > 256\n", buffer_size);
        return GT_FAIL;
    }

    cpssOsPrintf("bc2I2cReadSampleTest: bus_id 0x%x, slave_address 0x%x, offset_type 0x%x, offset 0x%x, buffer_size 0x%x\n",
               bus_id, slave_address, offset_type, offset, buffer_size);

    addrSpace = slave_address + (bus_id << 8) + (offset_type << 16);

    rc = i2cDrv->read(i2cDrv, addrSpace, offset, (GT_U32 *)buffer, buffer_size);

    cpssOsPrintf("i2c driver read returns with rc %x\n", rc);

    if( GT_OK == rc )
    {
        for ( ii = 0; ii < buffer_size ; ii++)
        {
            cpssOsPrintf("0x%x: 0x%x\n", ii, buffer[ii]);
        }
    }

    return rc;
}

GT_STATUS bc2I2cWriteSampleTest(
    IN GT_U8    bus_id,
    IN GT_U8    slave_address,
    IN EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN GT_U32   offset,
    IN GT_U32   buffer_size,
    IN GT_U8    data0,
    IN GT_U8    data1
)
{
    GT_STATUS rc;
    GT_U32  addrSpace;
    GT_U8  buffer[256];

    cpssOsPrintf("bc2I2cWriteSampleTest: bus_id 0x%x, slave_address 0x%x, offset_type 0x%x, offset 0x%x, buffer_size 0x%x\n"
               "                           data0 0x%x, data1 0x%x\n",
               bus_id, slave_address, offset_type, offset, buffer_size, data0, data1);

    buffer[0] = data0;
    buffer[1] = data1;

    if(buffer_size > 2)
    {
        cpssOsPrintf("buffer_size (%d) > 2\n", buffer_size);
        return GT_FAIL;
    }

    addrSpace = slave_address + (bus_id << 8) + (offset_type << 16);

    rc = i2cDrv->writeMask(i2cDrv, addrSpace, offset, (GT_U32 *)buffer, buffer_size, 0);

    cpssOsPrintf("i2c driver write returns with rc %x\n", rc);

    return rc;
}

/* Keeping the defualt config register for the A2D device */
GT_U8  defaultConfigVal = 0;

/**
* @internal A2D_max113x_set function
* @endinternal
*
* @brief   Set the A2D max113x setup/configuration registers
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note Galtis:
*       None.
*
*/
GT_STATUS A2D_max113x_set(const char *i2cChannelPath, GT_U8 i2cAddr, GT_U8 setupVal, GT_U8 configVal)
{
    GT_STATUS   rc;
    GT_U8       bus_id;
    GT_U32      addrSpace;
    GT_U16      value;


    if (i2cChannelPath == NULL) {
        cpssOsPrintf("I2c path not set\n");
        return GT_FAIL;
    }

    bus_id = i2cChannelPath[9] - '0';

    addrSpace = i2cAddr + (bus_id << 8) + (EXT_DRV_I2C_OFFSET_TYPE_8_E << 16);

    /* Saving the defaultConfigVal to be used as part of the RD command */
    defaultConfigVal = configVal;

    /* Writing the setup register */
    value = (setupVal & 0xFF) | 0x80;
    rc = i2cDrv->writeMask(i2cDrv, addrSpace, 0, (GT_U32 *)&value, 1, 0);

    return rc;
}


/**
* @internal A2D_max113x_get function
* @endinternal
*
* @brief   Set the A2D max113x setup/configuration registers
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note Galtis:
*       None.
*
*/
GT_STATUS A2D_max113x_get(const char *i2cChannelPath, GT_U8 i2cAddr, GT_U8 channelNum, GT_U32 mult, GT_U32 divide, GT_U32 resistorVal)
{
    GT_STATUS   rc;
    GT_U8       bus_id;
    GT_U32      addrSpace;
    GT_U8       valHigh, valLow;
    GT_U16      value;
    GT_U8       configVal;
    GT_FLOAT32  calcValue;


    if (i2cChannelPath == NULL) {
        cpssOsPrintf("I2c path not set\n");
        return GT_FAIL;
    }

    bus_id = i2cChannelPath[9] - '0';

    addrSpace = i2cAddr + (bus_id << 8) + (EXT_DRV_I2C_OFFSET_TYPE_16_E << 16);

    /* Ensure the AIN is supports only 0..15 - these are the  */
    if (channelNum >= 0x10) {
        cpssOsPrintf("Wrong channel number: 0x%02x\n", channelNum);
        return GT_FAIL;
    }

    /* We are using the address phase to set the config register */
    configVal = (defaultConfigVal & 0xE1) | (channelNum << 1);

    rc = i2cDrv->read(i2cDrv, addrSpace, configVal, (GT_U32 *)&value, 2);
    if (GT_OK == rc) {
        valHigh = (GT_U8)(value & 0xFF) ;
        valLow  = (GT_U8)((value & 0xFF00)>>8);

        cpssOsPrintf("Register read value: 0x%02x  0x%02x\n", valHigh, valLow);

        /* Check the divide or mult numbers are not 0 */
        if ((divide == 0) || (mult == 0)) {
            cpssOsPrintf("mult/divide numbers must be non-zero: %d/%d\n", mult, divide);
            return GT_FAIL;
        }

        value = ((valHigh & 0x3)<<8) | valLow;
        calcValue = (GT_FLOAT32)((value * 2.048 * mult) / divide);

        if (resistorVal != 0)
            calcValue =  calcValue / resistorVal;

        cpssOsPrintf("Calculated value in float: %f\n", calcValue);
    } else {
        cpssOsPrintf("Fail to read A2D register\n");
    }

    return rc;
  }
#endif /* LINUX_NOKM */


/************************************************************/
/* Start:    APIs related to PP Insert / Remove              */
/************************************************************
@internal waDeviceInit function
* @endinternal
*
* @brief   init the WA for one device that needed after phase1.
* @param[in] devNum                   - Device number.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS waDeviceInit
(
    IN  GT_U8   devNum,
    IN  GT_U8   boardRevId
)
{
    GT_STATUS   rc;
    static CPSS_DXCH_IMPLEMENT_WA_ENT   waFromCpss[CPSS_DXCH_IMPLEMENT_WA_LAST_E];
    GT_U32  enableAddrFilterWa; /* do we want to enable Restricted Address
                                   Filtering or not */
    GT_U32  waIndex=0;
    GT_U32  dbEntryValue;
    GT_BOOL enable30GWa = GT_FALSE;

    boardRevId = boardRevId;

    if(GT_OK != appDemoDbEntryGet("RESTRICTED_ADDRESS_FILTERING_WA_E", &enableAddrFilterWa))
    {
        enableAddrFilterWa = 0;
    }

    if(GT_OK == appDemoDbEntryGet("BC2_RevA0_40G_2_30GWA", &dbEntryValue))
    {
        enable30GWa = (GT_BOOL)dbEntryValue;
    }

    {
        waIndex = 0;

        /* state that application want CPSS to implement the WA */
        if(GT_FALSE != enable30GWa)
        {/* if WA enable bit 16 in 0x600006C set on by default */
            waFromCpss[waIndex++] = CPSS_DXCH_IMPLEMENT_WA_BOBCAT2_REV_A0_40G_NOT_THROUGH_TM_IS_PA_30G_E;
        }

#ifdef LINUX_NOKM
        {
            GT_U32  dmaSize = 0;

            extDrvGetDmaSize(&dmaSize);
            if (dmaSize < _512K)
            {
                /* state that application want CPSS to implement the Register Read
                   of the AU Fifo instead of using the AU DMA WA */
                waFromCpss[waIndex++] = CPSS_DXCH_IMPLEMENT_WA_FDB_AU_FIFO_E;
            }
        }
#endif

        if((GT_TRUE == PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(devNum))||
           (GT_TRUE == PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum)))
        {
            if (enableAddrFilterWa == 1)
            {
                /* state that application want CPSS to implement the Restricted
                   Address Filtering WA */
                waFromCpss[waIndex++] =
                    CPSS_DXCH_IMPLEMENT_WA_RESTRICTED_ADDRESS_FILTERING_E;
            }
        }

        if(waIndex)
        {
            rc = cpssDxChHwPpImplementWaInit(devNum,waIndex,&waFromCpss[0], NULL);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}






/**
* @internal configDeviceAfterPhase1 function
* @endinternal
*
* @brief   This function performs all needed configurations for one device that should be done
*         after phase1.
* @param[in] devIdx                   - Device index.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS configDeviceAfterPhase1
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
)
{
    GT_STATUS               rc;
    GT_U32                  mapArrLen;
    CPSS_DXCH_PORT_MAP_STC  *mapArrPtr;
    GT_U32                  noCpu;
    GT_U32                  coreClockDB;
    GT_U32                  coreClockHW;
    GT_BOOL                 isTmSupported = GT_FALSE;
    GT_BOOL                 isTmEnabled = GT_FALSE;
    GT_U8                   devNum;

    devNum = appDemoPpConfigList[devIdx].devNum;

    rc = configBoardAfterPhase1MappingGet(boardRevId,/*OUT*/&mapArrPtr,&mapArrLen);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (mapArrPtr == NULL)
    {
        cpssOsPrintf("\n-->ERROR : configBoardAfterPhase1() mapping : rev =%d is not supported\n", boardRevId);
        return GT_NOT_SUPPORTED;
    }

    rc = cpssDxChHwCoreClockGet(devNum, &coreClockDB, &coreClockHW);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoDxChBoardTypeGet(devNum, boardRevId, &bc2BoardType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("bobcat2BoardTypeGet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    switch (bc2BoardType)
    {
        case APP_DEMO_CAELUM_BOARD_DB_CNS:
        case APP_DEMO_CETUS_BOARD_DB_CNS:
        case APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS:
        case APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS:
            isBobkBoard = GT_TRUE;
            break;
        default:
            /*
            APP_DEMO_BC2_BOARD_DB_CNS:
            APP_DEMO_BC2_BOARD_RD_MSI_CNS:
            APP_DEMO_BC2_BOARD_RD_MTL_CNS:
            */
            isBobkBoard = GT_FALSE;
            break;
    }

    appDemoIsTmSupported(devNum, &isTmSupported);

    isTmEnabled = appDemoIsTmEnabled();

    switch(appDemoPpConfigList[devIdx].deviceId)
    {
        case CPSS_98DX4220_CNS:
            mapArrPtr = &bc2defaultMap_4220[0];
            mapArrLen = sizeof(bc2defaultMap_4220)/sizeof(bc2defaultMap_4220[0]);
            if(isTmEnabled && isTmSupported)
            {
                mapArrPtr = &bobkCaelumTmEnable2412[0];
                mapArrLen = sizeof(bobkCaelumTmEnable2412)/sizeof(bobkCaelumTmEnable2412[0]);
            }
            break;

        case CPSS_98DX4221_CNS:
            mapArrPtr = &bc2defaultMap_4221[0];
            mapArrLen = sizeof(bc2defaultMap_4221)/sizeof(bc2defaultMap_4221[0]);
            if(isTmEnabled && isTmSupported)
            {
                mapArrPtr = &bobkCaelumTmEnable4812[0];
                mapArrLen = sizeof(bobkCaelumTmEnable4812)/sizeof(bobkCaelumTmEnable4812[0]);
            }
            break;

        /******************/
        /* CAELUM devices */
        /******************/
        case CPSS_98DX4204_CNS:
        case CPSS_98DX4210_CNS:
        case CPSS_98DX3346_CNS:
            mapArrPtr = &caelumDefaultMap_4204[0];
            mapArrLen = sizeof(caelumDefaultMap_4204)/sizeof(caelumDefaultMap_4204[0]);
            if(isTmEnabled && isTmSupported)
            {
                mapArrPtr = &bobkCaelumTmEnable2412[0];
                mapArrLen = sizeof(bobkCaelumTmEnable2412)/sizeof(bobkCaelumTmEnable2412[0]);
            }
            break;
        case CPSS_98DX4212_CNS:
        case CPSS_98DX3345_CNS:
            mapArrPtr = &caelumDefaultMap_4212[0];
            mapArrLen = sizeof(caelumDefaultMap_4212)/sizeof(caelumDefaultMap_4212[0]);
            if(isTmEnabled && isTmSupported)
            {
                mapArrPtr = &bobkCaelumTmEnable2412[0];
                mapArrLen = sizeof(bobkCaelumTmEnable2412)/sizeof(bobkCaelumTmEnable2412[0]);
            }
            break;
        case CPSS_98DX42KK_CNS:
        case CPSS_98DX4203_CNS:
        case CPSS_98DX4211_CNS:
        case CPSS_98DX3347_CNS:
            mapArrPtr = &caelumDefaultMap[0];
            mapArrLen = sizeof(caelumDefaultMap)/sizeof(caelumDefaultMap[0]);

            if(isTmEnabled && isTmSupported)
            {
                mapArrPtr = &bobkCaelumTmEnable4812[0];
                mapArrLen = sizeof(bobkCaelumTmEnable4812)/sizeof(bobkCaelumTmEnable4812[0]);
            }
            break;
        case CPSS_98DX4222_CNS:
            mapArrPtr = &bc2defaultMap_4222[0];
            mapArrLen = sizeof(bc2defaultMap_4222)/sizeof(bc2defaultMap_4222[0]);
            break;
        case CPSS_98DX8216_CNS:
            mapArrPtr = &bc2defaultMap_8216[0];
            mapArrLen = sizeof(bc2defaultMap_8216)/sizeof(bc2defaultMap_8216[0]);
            break;
        case CPSS_98DX8219_CNS:
            mapArrPtr = &bc2defaultMap_8219[0];
            mapArrLen = sizeof(bc2defaultMap_8219)/sizeof(bc2defaultMap_8219[0]);
            break;
        case CPSS_98DX8224_CNS:
            mapArrPtr = &bc2defaultMap_8224[0];
            mapArrLen = sizeof(bc2defaultMap_8224)/sizeof(bc2defaultMap_8224[0]);
            break;
        case CPSS_98DX4253_CNS:
            mapArrPtr = &bc2defaultMap_4253[0];
            mapArrLen = sizeof(bc2defaultMap_4253)/sizeof(bc2defaultMap_4253[0]);
            break;
        case CPSS_98DX8332_Z0_CNS:
            mapArrPtr = &aldrinZ0DefaultMap_8332_Z0[0];
            mapArrLen = sizeof(aldrinZ0DefaultMap_8332_Z0)/sizeof(aldrinZ0DefaultMap_8332_Z0[0]);
            break;
        case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
            mapArrPtr = &cetusDefaultMap_56to59_62_64to71[0];
            mapArrLen = sizeof(cetusDefaultMap_56to59_62_64to71)/sizeof(cetusDefaultMap_56to59_62_64to71[0]);
            appDemoPpConfigList[devIdx].portInitlist_AllPorts_used = 0;
            if(isTmEnabled)
            {
                mapArrPtr = &bobkCetusTmEnableMap[0];
                mapArrLen = sizeof(bobkCetusTmEnableMap)/sizeof(bobkCetusTmEnableMap[0]);
            }

            if (appDemoPpConfigList[devIdx].deviceId ==  CPSS_98DX8208_CNS)
            {
                mapArrPtr = &bc2defaultMap_64to71[0];
                mapArrLen = sizeof(bc2defaultMap_64to71)/sizeof(bc2defaultMap_64to71[0]);
            }
            break;
        case CPSS_BOBCAT3_ALL_DEVICES_CASES_MAC:
        case CPSS_ARMSTRONG_ALL_DEVICES_CASES_MAC:
            {
                GT_U32  useSinglePipe = 0;
                #ifdef GM_USED
                    useSinglePipe = 1;
                #endif/* GM_USED */

                if(useSinglePipe)
                {
                    ARR_PTR_AND_SIZE_MAC(singlePipe_singleDp_bc3defaultMap,mapArrPtr,mapArrLen);
                }
                else
                {
                    ARR_PTR_AND_SIZE_MAC(bc3defaultMap,mapArrPtr,mapArrLen);
                }
            }

            if(PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum) == 512)
            {
                GT_U32  ii;
                for(ii = 0 ; ii < mapArrLen ; ii++)
                {
                    if(mapArrPtr[ii].physicalPortNumber != 63)
                    {
                        /* make sure that port number will be more than 256 */
                        mapArrPtr[ii].physicalPortNumber |= BIT_8;
                    }
                }
            }

            break;
        case CPSS_ALDRIN2_ALL_DEVICES_CASES_MAC:
            {
                GT_U32  useSingleDp = 0;
                #ifdef GM_USED
                    useSingleDp = 1;
                #endif/* GM_USED */
                if(useSingleDp)
                {
                    ARR_PTR_AND_SIZE_MAC(singleDp_aldrin2defaultMap,mapArrPtr,mapArrLen);
                }
                else
                {
                    ARR_PTR_AND_SIZE_MAC(aldrin2defaultMap,mapArrPtr,mapArrLen);
                    if (coreClockDB == 525)
                    {
                        /* Remove unused SDMA ports in order to meet BW limits */
                        mapArrLen -= 3;
                    }
                }
            }
            break;

        /******************/
        /* ALDRIN devices */
        /******************/
        case CPSS_98DX8308_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_8_ports,mapArrPtr,mapArrLen);
            break;
        case CPSS_98DX8312_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_12_ports,mapArrPtr,mapArrLen);
            break;
        case CPSS_98DX8316_CNS:
        case CPSS_98DXH831_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_16_ports,mapArrPtr,mapArrLen);
            break;
        case CPSS_98DX3256_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_18_ports,mapArrPtr,mapArrLen);
            break;
        case CPSS_98DX3257_CNS:
        case CPSS_98DX8324_CNS:
        case CPSS_98DXH832_CNS:
            ARR_PTR_AND_SIZE_MAC(aldrinMap_24_ports,mapArrPtr,mapArrLen);
            break;
        case CPSS_98DX3255_CNS:
        case CPSS_98DX3258_CNS:
        case CPSS_98DX3259_CNS:
        case CPSS_98DX3248_CNS:
        case CPSS_98DX3249_CNS:
        case CPSS_98DX8314_CNS:
        case CPSS_98DX8315_CNS:
        case CPSS_98DX8332_CNS:
        case CPSS_98DXZ832_CNS:
        case CPSS_98DXH833_CNS:
            if(cpssDeviceRunCheck_onEmulator() &&
               (0 == cpssDeviceRunCheck_onEmulator_isAldrinFull()))
            {
                /* limited Aldrin without all ports */
                ARR_PTR_AND_SIZE_MAC(onEmulator_aldrinDefaultMap,mapArrPtr,mapArrLen);
            }
            else
            {
                ARR_PTR_AND_SIZE_MAC(aldrinDefaultMap,mapArrPtr,mapArrLen);
            }
            break;

        default:
            break;
    }

    /* TM related port mapping adjustments for BC2 only */
    if (!isBobkBoard)
    {
        if (appDemoTmScenarioMode == CPSS_TM_2 || appDemoTmScenarioMode == CPSS_TM_3)
        {
            mapArrPtr = &bc2TmEnableE2Map[0];
            mapArrLen = sizeof(bc2TmEnableE2Map)/sizeof(bc2TmEnableE2Map[0]);
        }

        if (appDemoTmScenarioMode == CPSS_TM_8)
        {
            mapArrPtr = &bc2TmEnableE8Map[0];
            mapArrLen = sizeof(bc2TmEnableE8Map)/sizeof(bc2TmEnableE8Map[0]);
        }
    }
    else
    {
        /* same mapping for cetus and caelum, (in caelum mapping: 56-69 is different from the default tm scenario 0-48 ) */
        if (appDemoTmScenarioMode >= CPSS_TM_2 && appDemoTmScenarioMode <= CPSS_TM_5)
        {
                        mapArrPtr = &bobkTmEnableE2Map[0];
            mapArrLen = sizeof(bobkTmEnableE2Map)/sizeof(bobkTmEnableE2Map[0]);
        }


        if (appDemoTmScenarioMode == CPSS_TM_8)
        {
                        mapArrPtr = &bobkTmEnableE8Map[0];
            mapArrLen = sizeof(bobkTmEnableE8Map)/sizeof(bobkTmEnableE8Map[0]);
        }
    }

    if(appDemoPortMapPtr && appDemoPortMapSize)
    {
        /* allow to 'FORCE' specific array by other logic */
        mapArrPtr = appDemoPortMapPtr;
        mapArrLen = appDemoPortMapSize;
    }

    rc = appDemoDbEntryGet("noCpu", &noCpu);
    if(rc != GT_OK)
    {
        noCpu = 0;
    }

    if(1 == noCpu)
    {
        mapArrPtr = configBoardAfterPhase1MappingNoCPUCorrect(devNum, mapArrPtr,mapArrLen,/*OUT*/&mapArrLen);
    }

    /* perform board type recognition */
    rc = appDemoDxChBoardTypeGet(devNum,boardRevId,&bc2BoardType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("bobcat2BoardTypeGet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* print board type */
    bobcat2BoardTypePrint(devNum, bc2BoardType);

    rc = cpssDxChPortPhysicalPortMapSet(devNum, mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapSet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = appDemoDxChMaxMappedPortSet(devNum, mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChMaxMappedPortSet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* restore OOB port configuration after systemReset */
    if ((bc2BoardResetDone == GT_TRUE) && appDemoBc2IsInternalCpuEnabled(devNum) &&  (!isBobkBoard))
    {
        MV_HWS_SERDES_CONFIG_STR    serdesConfig;

        /* configure SERDES TX interface */
        rc = mvHwsSerdesTxIfSelect(devNum, 0, BC2_BOARD_MSYS_OOB_PORT_SERDES_CNS, COM_PHY_28NM, 1);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("mvHwsSerdesTxIfSelect", rc);
        if (GT_OK != rc)
        {
            return rc;
        }

        serdesConfig.baudRate = _1_25G;
        serdesConfig.media = XAUI_MEDIA;
        serdesConfig.busWidth = _10BIT_ON;
        serdesConfig.refClock = _156dot25Mhz;
        serdesConfig.refClockSource = PRIMARY;
        serdesConfig.rxEncoding = SERDES_ENCODING_NA;
        serdesConfig.serdesType = COM_PHY_28NM;
        serdesConfig.txEncoding = SERDES_ENCODING_NA;

        /* power UP and configure SERDES */
        rc = mvHwsSerdesPowerCtrl(devNum, 0, BC2_BOARD_MSYS_OOB_PORT_SERDES_CNS, GT_TRUE, &serdesConfig);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("mvHwsSerdesPowerCtrl", rc);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    if(appDemoBc2IsInternalCpuEnabled(devNum))
    {
        /* the embedded CPU not allow to do 'soft reset' */
        softResetSupported = GT_FALSE;
    }
    else
    {
        softResetSupported = GT_TRUE;
    }

    if(softResetSupported == GT_FALSE)
    {
        rc = cpssDevSupportSystemResetSet(devNum, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevSupportSystemResetSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(isBobkBoard)
    {
        oobPortUsed = GT_TRUE;
        oobPort = 90;
    }
    else
    {
        oobPortUsed = GT_FALSE;
    }

    if(oobPortUsed == GT_TRUE)
    {
        rc = cpssDevMustNotResetPortSet(devNum, oobPort, GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevMustNotResetPortSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    rc = waDeviceInit(devNum, boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("waInit", rc);

    bc2BoardResetDone = GT_FALSE;

    return GT_OK;
}

static GT_STATUS configDeviceAfterPhase2
(
    IN GT_U8    devIdx,
    IN GT_U8    boardRevId
)
{
    GT_PHYSICAL_PORT_NUM  port;   /* port number */
    GT_STATUS             rc = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    CPSS_SYSTEM_RECOVERY_INFO_STC saved_system_recovery;
    GT_U8                 devNum;

    devNum = appDemoPpConfigList[devIdx].devNum;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    saved_system_recovery = system_recovery;

    appDemoPpConfigList[devIdx].internal10GPortConfigFuncPtr = internalXGPortConfig;
    appDemoPpConfigList[devIdx].internalCscdPortConfigFuncPtr = cascadePortConfig;

#ifndef ASIC_SIMULATION

    /******************************************************/
                    CPSS_TBD_BOOKMARK_BOBCAT3
    /* BC3 board got stuck - code should be checked later */
    /******************************************************/
    if(system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        if(appDemoPpConfigList[devIdx].devFamily != CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
            rc = gtAppDemoXPhyFwDownload(devNum);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("gtAppDemoXPhyFwDownload", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
#endif

    for(port = 0; port < CPSS_MAX_PORTS_NUM_CNS;port++)
    {
        if( !(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&PRV_CPSS_PP_MAC(devNum)->existingPorts, port)))
        {
            continue;
        }

        /* split ports between MC FIFOs for Multicast arbiter */
        rc = cpssDxChPortTxMcFifoSet(devNum, port, port%2);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortTxMcFifoSet", rc);
        if( GT_OK != rc )
        {
            return rc;
        }
    }

    pDev = PRV_CPSS_DXCH_PP_MAC(devIdx);

    /* Aldrin does not have OOB PHY auto polling */
    if ((pDev->genInfo.devFamily != CPSS_PP_FAMILY_DXCH_ALDRIN_E) &&
        (pDev->genInfo.devFamily != CPSS_PP_FAMILY_DXCH_ALDRIN2_E) &&
        (pDev->genInfo.devFamily != CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        /* configure QSGMII ports PHY related mappings */
        if (   boardRevId == BOARD_REV_ID_DB_E     || boardRevId == BOARD_REV_ID_DB_TM_E
            || boardRevId == BOARD_REV_ID_RDMTL_E  || boardRevId == BOARD_REV_ID_RDMTL_TM_E
            || boardRevId == BOARD_REV_ID_RDMSI_E  || boardRevId == BOARD_REV_ID_RDMSI_TM_E)
        {
            if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
            {
                /* if FastBoot is underway the traffic shouldn't be altered */
                saved_system_recovery = system_recovery;
                system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
                system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
                rc =  cpssSystemRecoveryStateSet(&system_recovery);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            /* OOB port SMI configuration */
            rc = bobKOobPortSmiConfig(devNum, bc2BoardType);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("bobKOobPortSmiConfig", rc);
            if(rc != GT_OK)
            {
                rc =  cpssSystemRecoveryStateSet(&saved_system_recovery);
                if (rc != GT_OK)
                {
                    return rc;
                }
                return rc;
            }

            rc = boardAutoPollingConfigure(devNum,bc2BoardType);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("boardAutoPollingConfigure", rc);
            if (rc != GT_OK)
            {
                rc =  cpssSystemRecoveryStateSet(&saved_system_recovery);
                if (rc != GT_OK)
                {
                    return rc;
                }
                return rc;
            }
            rc =  cpssSystemRecoveryStateSet(&saved_system_recovery);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            cpssOsPrintf("\n-->ERROR : configBoardAfterPhase2() PHY config : rev =%d is not supported", boardRevId);
            return GT_NOT_SUPPORTED;
        }
    }

    /* Init LED interfaces */
    rc = appDemoLedInterfacesInit(devIdx, bc2BoardType);
            if (rc != GT_OK)
            {
                    return rc;
            }
    return rc;
}

/**
* @internal afterInitDeviceConfig function
* @endinternal
*
* @brief   Device specific configurations that should be done after device
*         initialization.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS afterInitDeviceConfig
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
)
{
    GT_STATUS   rc, rc1;
    GT_U32      skipPhyInit = 1;
    GT_U32      initSerdesDefaults;
    CPSS_PORTS_BMP_STC  portsMembers; /* VLAN members */
    CPSS_PORTS_BMP_STC  portsTagging; /* VLAN tagging */
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    CPSS_PORTS_BMP_STC  portsAdditionalMembers; /* VLAN members to add */
    GT_BOOL             isValid, isTmSupported, isTmEnabled; /* is Valid flag */
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    CPSS_SYSTEM_RECOVERY_INFO_STC saved_system_recovery;
    CPSS_SYSTEM_RECOVERY_INFO_STC tmp_system_recovery;
    GT_U32                        devNum;
#if defined (INCLUDE_TM)
    GT_U32      tmIgnoreHwSupport = 0;
#endif /*#if defined (INCLUDE_TM)*/

    devNum = appDemoPpConfigList[devIdx].devNum;

#if(defined _linux)
#ifdef CPU_ARMADAXP_3_4_69
#ifndef ASIC_SIMULATION
    GT_U32      regValue;
#endif
#endif
#endif

    PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(boardRevId);

#if(defined _linux)
#ifdef CPU_ARMADAXP_3_4_69
#ifndef ASIC_SIMULATION

    rc = hwIfTwsiInitDriver();
    if(GT_OK != rc)
    {
        return rc;
    }
#endif
#endif
#endif

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
    {
        tmp_system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
        tmp_system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
        rc =  cpssSystemRecoveryStateSet(&tmp_system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = appDemoIsTmSupported(devNum,/*OUT*/&isTmSupported);
    if (rc != GT_OK)
    {
        rc1 =  cpssSystemRecoveryStateSet(&system_recovery);
        if (rc1 != GT_OK)
        {
            return rc1;
        }
        return rc;
    }

    isTmEnabled = appDemoIsTmEnabled();

    if ((isTmSupported) && (isTmEnabled))
    {
        rc = afterInitBoardConfig_TM_BW_Init(devNum);
        if(rc != GT_OK)
        {
            rc1 =  cpssSystemRecoveryStateSet(&system_recovery);
            if (rc1 != GT_OK)
            {
                return rc1;
            }
            return rc;
        }

    }

    rc = appDemoBc2PIPEngineInit(devNum);
    if(rc != GT_OK)
    {
        rc1 =  cpssSystemRecoveryStateSet(&system_recovery);
        if (rc1 != GT_OK)
        {
            return rc1;
        }
        return rc;
    }

    /* After all CG MAC related configuration were done,
        it is needed to disable GC MAC UNIT in order to reduce power consumption.
        Only requested CG MACs will be enabled during appDemoBc2PortInterfaceInit */
    if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E))
    {
        /* Bobcat3 CG MAC unit disable */
        rc = cpssSystemRecoveryStateGet(&tmp_system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
        saved_system_recovery = tmp_system_recovery;

        tmp_system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
        tmp_system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
        rc =  cpssSystemRecoveryStateSet(&tmp_system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChCgMacUnitDisable(devNum, GT_FALSE);
        rc1 =  cpssSystemRecoveryStateSet(&saved_system_recovery);
        if (rc1 != GT_OK)
        {
            return rc1;
        }
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    rc = appDemoBc2PortInterfaceInit(devNum, boardRevId);
    if(rc != GT_OK)
    {
        rc1 =  cpssSystemRecoveryStateSet(&system_recovery);
        if (rc1 != GT_OK)
        {
            return rc1;
        }
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
    {
        rc =  cpssSystemRecoveryStateSet(&system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* clear additional default VLAN members */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsAdditionalMembers);


    if(isTmSupported == GT_TRUE && isTmEnabled) /* for TM configs only */
    {
        /* if appDemoTmScenarioMode has not changed by appDemoTmScenarioModeSet */
        if(boardRevId == 4 && appDemoTmScenarioMode == 0)
            appDemoTmScenarioMode = CPSS_TM_REV4;
        /*
            when system is initialized with non default ports mapping,
            TM Ports Init is called after custom ports mapping is applied
        */
        if (!isCustomPortsMapping)
        {
            rc = appDemoBc2TmPortsInit(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        if(PRV_CPSS_DXCH_PP_MAC(devIdx)->hwInfo.gop_ilkn.supported == GT_TRUE)
        {
            GT_PHYSICAL_PORT_NUM    chId;


            /* add ILKN channels to default VLAN, although they are not mapped yet */
            for(chId = 128; chId < 192; chId++)
            {
                /* set the port as member of vlan */
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsAdditionalMembers, chId);
            }
        }
        #if defined (INCLUDE_TM)
        if(isTmSupported == GT_TRUE)
        {
            /* enable TM regs access*/
            rc= prvCpssTmCtlGlobalParamsInit(devNum);
            if (rc)
            {
                cpssOsPrintf("%s %d  ERROR : rc=%d\n ",__FILE__, __LINE__ , rc);
                return rc;
            }
        }
        else if(isTmEnabled)
        {
            appDemoDbEntryGet("tmIgnoreHwSupport", &tmIgnoreHwSupport);

            if(tmIgnoreHwSupport)
            {
                cpssOsPrintf("Dev: %d is not supported TM, has initialized without TM!\n", devNum);
            }
            else
            {
                cpssOsPrintf("Dev: %d is not supported TM, or wrong TM Dram Frequency settings!\n", devNum);
                return GT_FAIL;
            }
        }
        #endif /*#if defined (INCLUDE_TM)*/
    }
    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {
        /* add port 83 to default VLAN, although it could be not mapped */
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsAdditionalMembers, 83);

        /* read VLAN entry */
        rc = cpssDxChBrgVlanEntryRead(devNum, 1, &portsMembers, &portsTagging,
                                        &cpssVlanInfo, &isValid, &portsTaggingCmd);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanEntryRead", rc);
        if (rc != GT_OK)
            return rc;

        /* add new ports as members, portsTaggingCmd is default - untagged */
        CPSS_PORTS_BMP_BITWISE_OR_MAC(&portsMembers, &portsMembers, &portsAdditionalMembers);

        /* Write default VLAN entry */
        rc = cpssDxChBrgVlanEntryWrite(devNum, 1,
                                        &portsMembers,
                                        &portsTagging,
                                        &cpssVlanInfo,
                                        &portsTaggingCmd);

        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanEntryWrite", rc);
        if (rc != GT_OK)
            return rc;
    }
    rc = appDemoDbEntryGet("initSerdesDefaults", /*out */&initSerdesDefaults);
    if(rc != GT_OK)
    {
        initSerdesDefaults = 1;
    }

    rc = appDemoDbEntryGet("skipPhyInit", &skipPhyInit);
    if(rc != GT_OK)
    {
        if (initSerdesDefaults != 0) /* if initSerdesDefaults == 0 .i.e. skip port config, */
        {                            /*     than skip also phy config */
            skipPhyInit = 0;
        }
        else
        {
            skipPhyInit = 1;
        }
    }

    if((bc2BoardType == APP_DEMO_CETUS_BOARD_DB_CNS) ||
        (bc2BoardType == APP_DEMO_ALDRIN_BOARD_DB_CNS) ||
        (bc2BoardType == APP_DEMO_BOBCAT3_BOARD_DB_CNS)||
        (bc2BoardType == APP_DEMO_ALDRIN_BOARD_RD_CNS) ||
        (bc2BoardType == APP_DEMO_BOBCAT3_BOARD_RD_CNS) ||
        (bc2BoardType == APP_DEMO_ARMSTRONG_BOARD_RD_CNS))
    {
        /* board without PHY */
        skipPhyInit = 1;
    }

    if(skipPhyInit != 1)
    {
        /* configure PHYs */
        if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
        {
            rc = bobcat2BoardPhyConfig(boardRevId, devNum);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("bobcat2BoardPhyConfig", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* PTP (and TAIs) configurations */
    if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
    {
        /* PTP (and TAIs) configurations */
        rc = appDemoB2PtpConfig(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvBobcat2PtpConfig", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }


    #if defined (INCLUDE_TM)
    if(PRV_CPSS_DXCH_PP_MAC(devIdx)->hwInfo.trafficManager.supported)
    {
        /* Enable Ingress Flow Control for TM */
        rc = cpssDxChTmGluePfcResponseModeSet(devNum, 0, CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E);


        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGluePfcResponseModeSet", rc);
    }
    #endif /*#if defined (INCLUDE_TM)*/

#if(defined _linux)
#ifdef CPU_ARMADAXP_3_4_69
#ifndef ASIC_SIMULATION

    /* Fix I2C access (for some random registers) from Armada XP card */
    rc = cpssDrvPpHwInternalPciRegRead(devNum, 0, 0x1108c, &regValue);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDrvPpHwInternalPciRegRead", rc);

    /* Set '0' to bit_18 */
    U32_SET_FIELD_MAC(regValue, 18, 1, 0);

    rc = cpssDrvPpHwInternalPciRegWrite(devNum, 0, 0x1108c, regValue);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDrvPpHwInternalPciRegWrite", rc);
#endif
#endif
#endif

    return GT_OK;
}

/************************************************************/
/* end:    APIs related to PP Insert / Remove              */
/************************************************************/









