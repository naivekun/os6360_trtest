/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <fcntl.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/userExit/dxCh/appDemoDxChEventHandle.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
/* cascading support */
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>

/* sructures definition for the use of Dx Cheetah Prestera core SW */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <gtExtDrv/drivers/pssBspApis.h>

#ifdef DRAGONITE_TYPE_A1

#ifdef _linux
#include <gtExtDrv/drivers/gtDragoniteDrv.h>
#endif

#include <cpss/extServices/cpssExtServices.h>
#include <cpss/generic/dragonite/cpssGenDragonite.h>
#include <cpss/generic/dragonite/private/prvCpssGenDragonite.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#ifdef ASIC_SIMULATION
#include <Globals.h>
#endif

#endif

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

/* Use KernPpDrv if BM + prestera driver */
#if defined(LINUX) && !defined(ASIC_SIMULATION) && !defined(LINUX_NOKM) && !defined(NO_KERN_PP_DRV)
#  define USE_KERN_PP_DRV
CPSS_HW_DRIVER_STC* cpssExtDrvPresteraKernDrvCreate
(
    IN  CPSS_HW_INFO_STC            *hwInfo,
    IN  GT_U32                      appAddrCompletionRegionsBmp
);
#endif


static GT_STATUS rd_ac3s_PolarityArrayGet
(
    IN  GT_U8 dev,
    OUT APPDEMO_SERDES_LANE_POLARITY_STC    **polarityArrayPtrPtr,
    OUT GT_U32                              *polarityArraySizePtr
);
static GT_STATUS rd_ac3s_PortInterfaceInit
(
    IN  GT_U8 dev
);
static GT_STATUS rd_ac3s_LedInit
(
    IN GT_U8 devNum
);

/*******************************************************
*  Includes and defines for PHYMAC 1540 extension
*******************************************************/
#include <gtOs/gtOsSem.h>
#include <extPhyM/phyMacInit.h>
#include <extPhyM/phyMacMSec.h>
#include <cpss/common/smi/cpssGenSmi.h>

#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/common/smi/cpssGenSmi.h>
/******************************************
 *  PHYMAC start/stop transaction semaphore
 ******************************************/
GT_SEM                  phyMacSemId;          /* The phymac transaction semaphore Id  */
GT_CHAR                 phyMacSemName[30] = "phymacTransactSem";    /* The semaphore name   */

/* outside PHY/SMI database */
phy1540MInfo phy1540MInfoDB[5] = {  { 0,0,0,0x4,0},
                                    { 0,1,0,0x5,1},
                                    { 0,2,0,0x6,2},
                                    { 0,3,0,0x7,3} };

/* size of PHY/SMI outside data base for PHYMAC object init */
GT_U32  phy1540MInfoDBSize = 5;

/* inside PHYMAC data base */
phy1540MDataBase phyMacDataBase;

/* pointer to PHYMAC object to be received by application from PHYMAC driver*/
CPSS_MACDRV_OBJ_STC * macDrvObjPtr = NULL;

/* was cpssResetSystem called or not */
static GT_BOOL xcat3BoardResetDone = GT_FALSE;


GT_STATUS gtAppDemoXcat2StackPortsModeSpeedSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode,
    IN  CPSS_PORT_SPEED_ENT  speed
);

static GT_STATUS xcat3PortPhy1548Init
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM  totalPortNum
);

static GT_STATUS xcat3PortPhy1680Init
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum
);

static GT_STATUS prvAppDemoXcat3sSerdesPolarityConfigSet
(
    IN GT_U8 devNum
);

/* Board types */
#define APP_DEMO_AC3_BOARD_DB_CNS           0       /* DB board */
#define APP_DEMO_AC3_BOARD_RD_MTL_TYPE1_CNS 1       /* MTL RD B2B Board Type1 - 48G + 4XG */
#define APP_DEMO_AC3_BOARD_RD_MTL_TYPE2_CNS 2       /* MTL RD B2B Board Type2 - 48G + 2XG + 2XXG */
#define APP_DEMO_AC3_BOARD_RD_MTL_TYPE3_CNS 4       /* MTL RD Board - 24G + 4SFP */
#define APP_DEMO_AC3S_BOARD_RD_MTL_TYPE1_CNS 5      /* xcat3s : MTL RD Board - 24G + 4SFP */
#define APP_DEMO_AC3_BOARD_INVALID_CNS      0xFF    /* invalid board type */

#define APP_DEMO_AC3BXH_WNC_OS6360P10       6
#define APP_DEMO_AC3BXH_WNC_OS6360P48       7
#define APP_DEMO_AC3BXH_WNC_OS6360P24       8

static GT_U32   xcat3BoardType  = APP_DEMO_AC3_BOARD_INVALID_CNS;

/**
* @struct PRV_APPDEMO_XCAT3_CASCADE_PORT_CONFIG_STC
 *
 * @brief XCAT3 cascade port configuration struct
*/
typedef struct{

    /** cascade port number */
    GT_PHYSICAL_PORT_NUM portNum;

    /** is Extended MAC used */
    GT_BOOL isExtendedMac;

    /** port speed */
    CPSS_PORT_SPEED_ENT speed;

    /** port interface mode */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;

    /** lanes number */
    GT_U32 lanesNum;

    GT_BOOL rxLanesPolarity[2];

    GT_BOOL txLanesPolarity[2];

} PRV_APPDEMO_XCAT3_CASCADE_PORT_CONFIG_STC;

/* Cascade port DB for xCat3 - 2 devices + 2 ports */
PRV_APPDEMO_XCAT3_CASCADE_PORT_CONFIG_STC xcat3CscdPortDb[2][2];

/*******************************************************************************
 * External variables
 ******************************************************************************/

static GT_U8    ppCounter = 0; /* Number of Packet processors. */
static GT_BOOL waSdmaPktsFromCpuEnabled = GT_FALSE;
static GT_U8 devNumWithCpu = 0; /* devNum with CPU enabled */

static GT_BOOL isZarlinkBoard = GT_FALSE; /* flag indicating if board contain Zarlink device */

#define APP_DEMO_INDEX_INTO_DESIGNATED_DEV_TABLE 1

#define ZARLINK_TWSI_DEV_ID 0x66
#define CPLD_TWSI_DEV_ID    0x18

/* Zarlink configuration sequence - <regAddr,regData> pairs*/
static GT_U8 zarlinkConfigurationSequence[31][2] =
{
    {0x16,0x22},
    {0x17,0x22},
    {0x18,0x22},
    {0x19,0x22},
    {0x1D,0x1a},
    {0x1F,0x03},
    {0x2A,0x90},
    {0x2B,0x04},
    {0x2C,0x03},
    {0x38,0x35},
    {0x39,0x0C},
    {0x3B,0x00},
    {0x3E,0x02},
    {0x43,0x02},
    {0x48,0x03},
    {0x51,0x7f},
    {0x52,0x66},
    {0x60,0xaf},
    {0x61,0x11},
    {0x63,0xe5},
    {0x65,0x01},
    {0x67,0x35},
    {0x68,0x0c},
    {0x69,0x06},
    {0x6A,0x12},
    {0x6B,0x37},
    {0x6C,0x06},
    {0x6D,0x99},
    {0x6E,0x06},
    {0x6F,0x7f},
    {0x70,0x01}
};


/**
* @struct PRV_APPDEMO_XCAT_PHY_DATA_STC
 *
 * @brief PHY init structure
*/
typedef struct{

    /** PHY page */
    GT_U16 page;

    /** address of PHY register */
    GT_U8 regAddr;

    /** @brief data to be written
     *  Comments:
     *  None
     */
    GT_U16 data;

} PRV_APPDEMO_XCAT_PHY_DATA_STC;

/* array for PHY1340 init */
static PRV_APPDEMO_XCAT_PHY_DATA_STC xcatC0NPPhyConfigArr[] =
{
    /*   page   regAddr   data  */
     {   0xfd,   0xa,    0xf     }
    ,{   0xfd,   0xb,    0x1cf0  }
    ,{   0xfd,   0x11,   0x2299  }
    ,{   0xfd,   0xb,    0x1d70  }
    ,{   0xff,   0x18,   0x2800  }
    ,{   0xff,   0x17,   0x2001  }
    ,{   0x0,    0x1d,   0x10    }
    ,{   0x0,    0x1e,   0x4018  }
    ,{   0x0,    0x1d,   0x0     }
    ,{   0x0,    0x1d,   0x3     }
    ,{   0x0,    0x1e,   0x2     }
    ,{   0x0,    0x1d,   0x0     }
    ,{   0x0,    0x0,    0x9140  }
    ,{   0xdead, 0xff,   0xdead  }
};
/**
* @internal hwIfTwsiProtocolReadReg function
* @endinternal
*
* @brief   TWSI read register according to Zarlink requested protocol.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  -  (pointer to)Data read from register.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwIfTwsiProtocolReadReg
(
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U8  *dataPtr
)
{
#if (!defined _linux) && (!defined LINUX_SIM) && (!defined ASIC_SIMULATION) && (defined XCAT_DRV)
    GT_STATUS rc;
    GT_U8   twsiRdDataBuff[4];
    GT_U8   regCharAddr[4] = {0,0,0,0};

    rc = bspTwsiWaitNotBusy();
    if (GT_OK != rc)
        return GT_FAIL;

    regCharAddr[0] = (GT_U8) regAddr;

    rc = bspTwsiMasterWriteTrans(devSlvId, regCharAddr,1, GT_FALSE);
    if (GT_OK != rc)
        return GT_FAIL;

    rc = bspTwsiMasterReadTrans(devSlvId, twsiRdDataBuff,1, GT_FALSE);
    if (GT_OK != rc)
        return GT_FAIL;

    *dataPtr = twsiRdDataBuff[0];

    return GT_OK;
#else
    return GT_FAIL;
#endif
}

/**
* @internal hwIfTwsiProtocolWriteReg function
* @endinternal
*
* @brief   TWSI write register according to Zarlink requested protocol.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to write to.
* @param[in] data                     - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwIfTwsiProtocolWriteReg
(
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    IN  GT_U8   data
)
{
    GT_STATUS rc;
#if (!defined _linux) && (!defined LINUX_SIM) && (!defined ASIC_SIMULATION) && (defined XCAT_DRV)
    GT_U8   regCharAddr[4] = {0,0,0,0};

    rc = bspTwsiWaitNotBusy();
    if (GT_OK != rc)
        return GT_FAIL;

    regCharAddr[0] = (GT_U8) regAddr;
    regCharAddr[1] = data;

    rc = bspTwsiMasterWriteTrans(devSlvId, regCharAddr,2, GT_FALSE);
#else
    rc = GT_FAIL;
#endif
    return rc;
}

/************************************************************************
*  PHYMAC start & stop transaction functions
*************************************************************************/
static GT_STATUS phymacStartTransaction
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL *currentPollingStatus
)
{
    GT_STATUS rc;
    GT_BOOL status;

    /* Take semaphore */
    osSemWait(phyMacSemId, 0);

    /* Get current polling status */
    rc= cpssDxChPhyAutonegSmiGet(devNum,portNum,&status);
    if(rc != GT_OK)
    {
        return GT_FAIL;
    }
    /* Set stop polling status if it run*/
    if (status == GT_TRUE) {
        if (currentPollingStatus != NULL) {
            *currentPollingStatus = GT_TRUE;
        }
        else
        {
            return GT_FAIL;
        }
        /* stop polling */
        rc= cpssDxChPhyAutonegSmiSet(devNum,portNum,GT_FALSE);
        if(rc != GT_OK)
        {
            return GT_FAIL;
        }
    }
    return GT_OK;
}

static GT_STATUS phymacStopTransaction
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  oldPollingStatus
)
{
    GT_STATUS rc;

    /* Set old polling status */
    /* stop polling */
    rc= cpssDxChPhyAutonegSmiSet(devNum,portNum,oldPollingStatus);
    if(rc != GT_OK)
    {
        return GT_FAIL;
    }

    /* Give semaphore */
    osSemSignal(phyMacSemId);

    return GT_OK;
}

#ifndef ASIC_SIMULATION
static GT_STATUS phymacMacSecEnable
(
    IN GT_U8   devNum,
    IN GT_U8   portNum,
    IN GT_BOOL  state
)
{
    GT_STATUS rc;
    GT_BOOL pollingStatus;
    GT_U16 regVal;

    rc = phymacStartTransaction(devNum,portNum,&pollingStatus);
    if( rc != GT_OK)
    {
        return rc;
    }

    /* set to page register page number 18 */
    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 18);
    if( rc != GT_OK)
    {
        return rc;
    }

    /* read reg 27*/
    rc = cpssDxChPhyPortSmiRegisterRead(devNum, portNum, 27, &regVal);
    if( rc != GT_OK)
    {
        return rc;
    }

    /* set MACPHY enable/disable */

    if (state == GT_TRUE) {
        regVal = regVal | 0x2000;
         }
    else
    {
        regVal = regVal & 0xDFFF;
    }

    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 27, regVal);
    if( rc != GT_OK)
    {
        return rc;
    }

    /* read reg 27 to check it */
    rc = cpssDxChPhyPortSmiRegisterRead(devNum, portNum, 27, &regVal);
    if( rc != GT_OK)
    {
        return rc;
    }

    /* set to page register page number 0 */
    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0x0);
    if( rc != GT_OK)
    {
        return rc;
    }


    rc = phymacStopTransaction(devNum,portNum,pollingStatus);
    if( rc != GT_OK)
    {
        return rc;
    }
             return GT_OK;
}

static GT_STATUS phymacMacSecGetPhyId
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_U16  *phyId
)
{
    GT_STATUS rc;
    GT_BOOL   pollingStatus;

    rc = phymacStartTransaction(devNum,portNum,&pollingStatus);
    if( rc != GT_OK)
    {
        return rc;
    }

    /* set to page register page number 0 */
    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22,0);
    if( rc != GT_OK)
    {
        return rc;
    }

    /* read reg 2*/
    rc = cpssDxChPhyPortSmiRegisterRead(devNum, portNum, 2, phyId);
    if( rc != GT_OK)
    {
        return rc;
    }

    rc = phymacStopTransaction(devNum,portNum,pollingStatus);
    if( rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}
#endif

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
    GT_U8 value;
#ifndef LINUX_NOKM
    APP_DEMO_PP_CONFIG appDemoPpConfigListTmp;
#endif
    GT_U32 devIdx;

    /* Getting the first devNum, which is the first device index in the appDemo device list
       and which other devices stored incrementally  */
    devIdx = SYSTEM_DEV_NUM_MAC(0);

    /* check if board contains Zalink device for Sync Ethernet testing */
    pciStat = hwIfTwsiProtocolReadReg(ZARLINK_TWSI_DEV_ID, 0x0, &value);
    if( (GT_OK == pciStat) && (0x91 == (value & 0xFF)) )
    {
        /* Zarlink device exists */
        DBG_TRACE(("Board contain Zarlink device!!!\n"));
        isZarlinkBoard = GT_TRUE;

        /* Zarlink configurations */
        /* use TWSI driver to addrress 0x66 (Zarlink) with zarlinkConfigurationSequence data */
        for( i = 0 ; i < sizeof(zarlinkConfigurationSequence)/sizeof(GT_8)/2 ; i++ )
        {
            pciStat = hwIfTwsiProtocolWriteReg(ZARLINK_TWSI_DEV_ID,
                                               zarlinkConfigurationSequence[i][0],
                                               zarlinkConfigurationSequence[i][1]);
            if( GT_OK != pciStat )
                return pciStat;
            osTimerWkAfter(100);
        }

        /* reset phys and xCat - TWSI to CPLD - addr 0x18, reg 3, 1 byte, 0xf3 value */
        pciStat = hwIfTwsiProtocolWriteReg(CPLD_TWSI_DEV_ID, 3, 0xf3);
        if( GT_OK != pciStat )
                return pciStat;
        osTimerWkAfter(2000);
    }

    if(boardRevId == 7)
    {
        pciStat = appDemoSysGetSmiInfo();
    }
    else
    {
        pciStat = appDemoSysGetPciInfo();
    }
    if(pciStat == GT_OK)
    {
        for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
        {
            if(appDemoPpConfigList[i].valid == GT_TRUE)
            {
                ppCounter++;
            }
        }
    }
    else
        ppCounter = 0;

    /* No PCI devices found */
    if(ppCounter == 0)
    {
        return GT_NOT_FOUND;
    }
    if (boardRevId == 8)  /* native MII*/
    {
        appDemoSysConfig.cpuEtherPortUsed = GT_TRUE;

        for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
        {
            if(appDemoPpConfigList[i].valid == GT_TRUE)
            {
                /*this is device with disabled CPU*/
                if (appDemoPpConfigList[i].deviceId & 0x20000)
                {
                    appDemoPpConfigList[i].cpuPortMode = CPSS_NET_CPU_PORT_MODE_NONE_E;
                }
                else
                {
                    appDemoPpConfigList[i].cpuPortMode = CPSS_NET_CPU_PORT_MODE_MII_E;
                }
            }
        }
    }

    if (boardRevId == 1)  /* AC3 + SGMII */
    {
        for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
        {
            if((appDemoPpConfigList[i].valid == GT_TRUE) && (appDemoPpConfigList[i].devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {
                appDemoSysConfig.cpuEtherPortUsed = GT_TRUE;
                appDemoRawSocketModeSet();

                /* Only first device has CPU, all others are disabled */
                if (i != 0)
                {
                    appDemoPpConfigList[i].cpuPortMode = CPSS_NET_CPU_PORT_MODE_NONE_E;
                }
                else
                {
                    appDemoPpConfigList[i].cpuPortMode = CPSS_NET_CPU_PORT_MODE_MII_E;
                }
            }
        }
    }

    *numOfPp    = ppCounter;
    *numOfFa    = 0;
    *numOfXbar  = 0;

    *isB2bSystem = ((ppCounter == 2) ? GT_TRUE : GT_FALSE);

    /******************************/
    /* XCAT3 B2B 48G board        */
    /******************************/
    if(appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        if (xcat3BoardType == APP_DEMO_AC3_BOARD_INVALID_CNS)
            xcat3BoardType = APP_DEMO_AC3_BOARD_DB_CNS; /* will be updated later to other RD type */
        if(ppCounter == 2)
        {
            /* First, PCI scan finds first PP1(with CPU disabled) - gets device_index 0,
               and then PP0 is scanned as device_index 1.
               Therefore DB enries 0 and 1 should be swapped */

#ifndef LINUX_NOKM /* no need to swap with NOKM */
            osMemCpy(&appDemoPpConfigListTmp, &appDemoPpConfigList[devIdx], sizeof(appDemoPpConfigListTmp));
            osMemCpy(&appDemoPpConfigList[devIdx], &appDemoPpConfigList[devIdx + 1], sizeof(appDemoPpConfigListTmp));
            osMemCpy(&appDemoPpConfigList[devIdx + 1], &appDemoPpConfigListTmp, sizeof(appDemoPpConfigListTmp));
#endif

            appDemoPpConfigList[devIdx].devNum = SYSTEM_DEV_NUM_MAC(0);
            appDemoPpConfigList[devIdx + 1].devNum = SYSTEM_DEV_NUM_MAC(1);

        }
    }

    return GT_OK;
}

/**
* @internal getPpPhase1Config function
* @endinternal
*
* @brief   Returns the configuration parameters for corePpHwPhase1Init().
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devIdx                   - The Pp Index to get the parameters for.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpPhase1Config
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devIdx,
    OUT CPSS_PP_PHASE1_INIT_PARAMS    *phase1Params
)
{
    CPSS_PP_PHASE1_INIT_PARAMS    localPpPh1Config = CPSS_PP_PHASE1_DEFAULT;
    GT_U32 cpuDisabled = 0;
    GT_STATUS rc;

    localPpPh1Config.devNum = devIdx;

    if (appDemoPpConfigList[devIdx].channel == CPSS_CHANNEL_PCI_E)
    {
        localPpPh1Config.mngInterfaceType = CPSS_CHANNEL_PEX_E;
        localPpPh1Config.deviceId =
            ((appDemoPpConfigList[devIdx].pciInfo.pciDevVendorId.devId << 16) |
             (appDemoPpConfigList[devIdx].pciInfo.pciDevVendorId.vendorId));

        switch (localPpPh1Config.deviceId)
        {
            case PRV_CPSS_DXCH_XCAT3_DEVICES_CASES_MAC:
                localPpPh1Config.mngInterfaceType = CPSS_CHANNEL_PEX_MBUS_E;
                localPpPh1Config.isrAddrCompletionRegionsBmp = 0x02;
                localPpPh1Config.appAddrCompletionRegionsBmp = 0x3C;
                break;
        }

        if (boardRevId == 11)
        {
            /* force to use kernel driver */
            localPpPh1Config.mngInterfaceType = CPSS_CHANNEL_PEX_KERNEL_E;
            localPpPh1Config.appAddrCompletionRegionsBmp = 0x0f; /* 4 region addr completion */
        }
        /* map resources */
        rc = extDrvPexConfigure(
                appDemoPpConfigList[devIdx].pciInfo.pciBusNum,
                appDemoPpConfigList[devIdx].pciInfo.pciIdSel,
                appDemoPpConfigList[devIdx].pciInfo.funcNo,
                (localPpPh1Config.mngInterfaceType == CPSS_CHANNEL_PEX_KERNEL_E)
                    ? MV_EXT_DRV_CFG_FLAG_DONT_MAP_E : 0,
                &(localPpPh1Config.hwInfo[0]));
        if (rc != GT_OK)
        {
            osPrintf("extDrvPexConfigure() failed, rc=%d\n", rc);
            return rc;
        }
#ifdef USE_KERN_PP_DRV
        if (localPpPh1Config.mngInterfaceType == CPSS_CHANNEL_PEX_KERNEL_E)
        {
            localPpPh1Config.hwInfo[0].driver = cpssExtDrvPresteraKernDrvCreate(
                    &(localPpPh1Config.hwInfo[0]), localPpPh1Config.appAddrCompletionRegionsBmp);
        }
#endif


    }
    else /* CPSS_CHANNEL_SMI_E */
    {
        localPpPh1Config.hwInfo[0].busType = CPSS_HW_INFO_BUS_TYPE_SMI_E;
        localPpPh1Config.hwInfo[0].hwAddr.devSel = appDemoPpConfigList[devIdx].smiInfo.smiIdSel;
        localPpPh1Config.deviceId         =
           ((appDemoPpConfigList[devIdx].smiInfo.smiDevVendorId.devId << 16) |
            (appDemoPpConfigList[devIdx].smiInfo.smiDevVendorId.vendorId));
        localPpPh1Config.mngInterfaceType = CPSS_CHANNEL_SMI_E;
    }

    if(2 == ppCounter)
    {
        switch(localPpPh1Config.deviceId)
        {
            case PRV_CPSS_DXCH_XCAT3_DEVICES_CASES_MAC:
                /* workaround: devIdx = 0 - this is device with CPU enabled, but in order to
                               get right INT vector the logic should be inverse */
                cpuDisabled = 1 - devIdx;
                break;
            default:
                cpuDisabled = localPpPh1Config.deviceId & 0x20000;
                break;
        }

        localPpPh1Config.isExternalCpuConnected = cpuDisabled;

        switch(localPpPh1Config.deviceId)
        {
            case PRV_CPSS_DXCH_XCAT3_DEVICES_CASES_MAC:
                /* do nothing */
                break;

            default:
                /* 156.25Mhz frequancy is not qualified,
                    Marvell strongly recommend to use the 125Mhz frequency for xCat */
                if((appDemoPpConfigList[devIdx].pciInfo.pciHeaderInfo[2] & 0xFF) < 2)
                    return GT_NOT_SUPPORTED;
            break;
        }
    }
    else
    {
        if((appDemoPpConfigList[devIdx].pciInfo.pciHeaderInfo[2] & 0xFF) < 2)
        {
            localPpPh1Config.serdesRefClock = APPDEMO_SERDES_REF_CLOCK_EXTERNAL_125_SINGLE_ENDED_E;
        }
    }
    if((appDemoPpConfigList[devIdx].pciInfo.pciHeaderInfo[2] & 0xFF) >= 2)
    {
          localPpPh1Config.serdesRefClock = APPDEMO_SERDES_REF_CLOCK_INTERNAL_125_E;
    }

    /* If board contains Zalink device for Sync Ethernet testing then configure    */
    /* SERDES reference clock to be taken from external pins using SD_REF_CLK_P/N. */
    if( GT_TRUE == isZarlinkBoard )
    {
        localPpPh1Config.serdesRefClock = APPDEMO_SERDES_REF_CLOCK_EXTERNAL_125_DIFF_E;
    }

    /* retrieve PP Core Clock from HW */
    localPpPh1Config.coreClk = APP_DEMO_CPSS_AUTO_DETECT_CORE_CLOCK_CNS;

    localPpPh1Config.uplinkCfg = CPSS_PP_NO_UPLINK_E;

    /* only revision 10 - is enabled for VPLS mode */
    localPpPh1Config.enableLegacyVplsModeSupport = (boardRevId == 10) ? GT_TRUE : GT_FALSE;

    osMemCpy(phase1Params,&localPpPh1Config,sizeof(CPSS_PP_PHASE1_INIT_PARAMS));

    /* Disable Flow Control for this revision */
    if(boardRevId == 3)
    {
        appDemoPpConfigList[devIdx].flowControlDisable = GT_TRUE;
    }

    return GT_OK;
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
    CPSS_DXCH_IMPLEMENT_WA_ENT   waFromCpss[CPSS_DXCH_IMPLEMENT_WA_LAST_E];
    GT_U32  additionalInfoBmpArr[CPSS_DXCH_IMPLEMENT_WA_LAST_E];
    GT_U32  waIndex=0;
    GT_U32  ii;
    GT_U8   devNum;
    GT_U32  ignoreTrunkWa = 0;/*do we want to ignore the WA for trunk */
    GT_U32  forceTrunkWa = 0;/*do we want to force the WA for trunk */
    GT_U32  ignorePacketSdmaWa;/*do we want to ignore the WA for sdma */
    GT_U32  ignoreAuSdmaWa; /* do we want to ignore the WA for AU Fifo */
    GT_U32  additionalTrunkWaInfoBmp; /* additional info for trun WA -
                                         EPCL cfg tbl mode, base index and rules
                                         location in TCAM */
    GT_U32  enableAddrFilterWa; /* do we want to enable Restricted Address Filtering or not */
    GT_U32  cpuDisabled = 0; /* hold internal CPU status */
    GT_U32  enablePortUnidirectWa = 0; /* do we want to enable WA for GE ports unidirect */

    /* use same string as for PSS */
    /* allow to state that the WA not needed */
    if(GT_OK != appDemoDbEntryGet("NO___GT_CORE_SYS_WA_OPTION_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E", &ignoreTrunkWa))
    {
        ignoreTrunkWa = 0;
    }

    /* allow to state that the WA must be used */
    if(GT_OK != appDemoDbEntryGet("FORCE___GT_CORE_SYS_WA_OPTION_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E", &forceTrunkWa))
    {
        forceTrunkWa = 0;
    }

    if(GT_OK != appDemoDbEntryGet("NO___GT_CORE_SYS_WA_OPTION_SDMA_PKTS_FROM_CPU_STACK_WA_E", &ignorePacketSdmaWa))
    {
        ignorePacketSdmaWa = 0;
    }

    if(GT_OK != appDemoDbEntryGet("NO___CPSS_DXCH_IMPLEMENT_WA_FDB_AU_FIFO_E", &ignoreAuSdmaWa))
    {
        ignoreAuSdmaWa = 0;
    }

    if(GT_OK != appDemoDbEntryGet("TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_ADDITIONAL_INFO_E", &additionalTrunkWaInfoBmp))
    {
        additionalTrunkWaInfoBmp = 0;
    }

    if(GT_OK != appDemoDbEntryGet("RESTRICTED_ADDRESS_FILTERING_WA_E", &enableAddrFilterWa))
    {
        enableAddrFilterWa = 0;
    }

    if(GT_OK != appDemoDbEntryGet("GT_CORE_SYS_WA_OPTION_GE_PORT_UNIDIRECT_WA_E", &enablePortUnidirectWa))
    {
        enablePortUnidirectWa = 0;
    }

    /* find dev with CPU port */
    for(ii = SYSTEM_DEV_NUM_MAC(0); ii < SYSTEM_DEV_NUM_MAC(ppCounter); ++ii)
    {
        devNum = appDemoPpConfigList[ii].devNum;

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
        {
            /* retrieve CPU enabled from Sampled at Reset - CPU_INT_EXT */
            rc = prvCpssHwPpGetRegField(devNum,0x0000002C,14,1,&cpuDisabled);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* WA - on xCat2 both CPUs enabled */
            cpuDisabled = devNum;
        }
        else
        {
            cpuDisabled = appDemoPpConfigList[ii].deviceId & 0x20000;
        }

        if (!(cpuDisabled))
        {
            devNumWithCpu = appDemoPpConfigList[ii].devNum;
        }
    }

    for(ii = SYSTEM_DEV_NUM_MAC(0); ii < SYSTEM_DEV_NUM_MAC(ppCounter); ++ii)
    {
        waIndex = 0;
        devNum = appDemoPpConfigList[ii].devNum;

        /* xCat & xCat3 devices */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            /* no need for SDMA emulation when native MII used*/
            if (boardRevId == 8)
            {
                ignorePacketSdmaWa = 1;
            }

            if((PRV_CPSS_PP_MAC(devNum)->revision >= 2) &&
               (boardRevId != 2) && (boardRevId != 6) && (boardRevId != 9))
            {
                if(ignorePacketSdmaWa == 0)
                {
                    /* state that application want CPSS to implement the CPU SDMA stuck WA */
                    waFromCpss[waIndex] =
                        CPSS_DXCH_IMPLEMENT_WA_SDMA_PKTS_FROM_CPU_STACK_E;
                    additionalInfoBmpArr[waIndex++] = devNumWithCpu;
                    waSdmaPktsFromCpuEnabled = GT_TRUE;
                }
                if(ignoreAuSdmaWa == 0)
                {
                    /* state that application want CPSS to implement the Register Read
                       the AU Fifo instead of using the AU DMA WA */
                    waFromCpss[waIndex] =
                        CPSS_DXCH_IMPLEMENT_WA_FDB_AU_FIFO_E;
                    additionalInfoBmpArr[waIndex++] = 0;
                }
            }

            if ((PRV_CPSS_PP_MAC(devNum)->revision >= 3) &&
                (boardRevId != 2) && (boardRevId != 6) && (boardRevId != 9))
            {
                /* XCAT A2, board revision 2 and 6 */
                /* disable  CPSS to implement the trunk WA */
                ignoreTrunkWa = 1;
            }

            if (enableAddrFilterWa == 1)
            {
                /* state that application want CPSS to implement the Restricted
                   Address Filtering WA */
                waFromCpss[waIndex] =
                    CPSS_DXCH_IMPLEMENT_WA_RESTRICTED_ADDRESS_FILTERING_E;
                additionalInfoBmpArr[waIndex++] = 0;
            }

        }
        else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
        {
            if(boardRevId == 1)
            {
                /* SDMA over RGMII for xCat2 */
                waFromCpss[waIndex] =
                    CPSS_DXCH_IMPLEMENT_WA_SDMA_PKTS_FROM_CPU_STACK_E;
                additionalInfoBmpArr[waIndex++] = devNumWithCpu;
                waSdmaPktsFromCpuEnabled = GT_TRUE;

            }
        }

        if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            /* XCAT 2,3 not need the WA , regardless to boardRevId */
            ignoreTrunkWa = 1;
        }

        if(ignoreTrunkWa == 0 || forceTrunkWa)
        {
            /* state that application want CPSS to implement the trunk WA */
            waFromCpss[waIndex] =
                CPSS_DXCH_IMPLEMENT_WA_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E;
            additionalInfoBmpArr[waIndex++] = additionalTrunkWaInfoBmp;
            /* save to DB --> will be used by the galtis Wrappers */
            appDemoPpConfigList[ii].wa.trunkToCpuPortNum = GT_TRUE;
        }
        else
        {
            /* this runtime must be without the 'WA' because we need to check
               the behavior without the WA */
        }

        if(enablePortUnidirectWa)
        {
            /* state that application want CPSS to implement the GE port unidirect
                mode WA */
            waFromCpss[waIndex++] =
                            CPSS_DXCH_IMPLEMENT_WA_GE_PORT_UNIDIRECT_E;
        }
#ifdef LINUX_NOKM
        {
            GT_U32  dmaSize = 0;

            extDrvGetDmaSize(&dmaSize);
            if (dmaSize < _512K)
            {
                /* state that application want CPSS to implement the Register Read
                   the AU Fifo instead of using the AU DMA WA */
                waFromCpss[waIndex] = CPSS_DXCH_IMPLEMENT_WA_FDB_AU_FIFO_E;
                additionalInfoBmpArr[waIndex++] = 0;
            }
        }
#endif

        if(waIndex)
        {
            rc = cpssDxChHwPpImplementWaInit(devNum,waIndex,&waFromCpss[0],
                                             &additionalInfoBmpArr[0]);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

extern GT_BOOL appDemo_LSP_is_linux_2_6_32;

#ifdef ASIC_SIMULATION
/*
    allow simulation to run one of the specific board types:
    APP_DEMO_AC3_BOARD_DB_CNS           0
    APP_DEMO_AC3_BOARD_RD_MTL_TYPE1_CNS 1
    APP_DEMO_AC3_BOARD_RD_MTL_TYPE2_CNS 2
    APP_DEMO_AC3_BOARD_RD_MTL_TYPE3_CNS 4
    APP_DEMO_AC3S_BOARD_RD_MTL_TYPE1_CNS 5
*/
static GT_U32   explicit_simulation_board_type = 0;
/* if needed then function need to be called before cpssInitSystem(...) */
GT_STATUS   simulation_xcat3_set_board_type(GT_U32 board_type)
{
    explicit_simulation_board_type = board_type;
    return GT_OK;
}
#endif /*ASIC_SIMULATION*/


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
    GT_STATUS   rc;         /* return code */
#ifndef ASIC_SIMULATION
    GT_U8  dev;

    dev = 0;
#endif

    rc = waInit(boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("waInit", rc);

#ifndef ASIC_SIMULATION

#if 0
    if (prvCpssDrvPpConfig[0]->hwInfo[0].busType != CPSS_HW_INFO_BUS_TYPE_MBUS_E)
    {
        /* Nothing to do: only DB board can work with external CPU */
        xcat3BoardType = APP_DEMO_AC3_BOARD_DB_CNS;
    }
    else
#endif
    {
        /* update RD board type */
        if((xcat3BoardType != APP_DEMO_AC3_BOARD_INVALID_CNS) && (!xcat3BoardResetDone))
        {
            GT_U32      regValue;   /* register value */
            GT_U32      skuFd, len;
            GT_U8       skuStr[16];

            /*
                IMPORTENT NOTE: It is agreed with UBOOT that board_id value
                                transfered to CPSS via User-Defined-Register 0x7c
            */
            /*
            rc = prvCpssHwPpGetRegField(0, 0x7C, 0, 8, &regValue);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssHwPpGetRegField", rc);
            */

            if ((skuFd = open("/sys/bus/i2c/devices/0-007f/product_id", O_RDONLY)) < 0)
                regValue = 0x71;
            else
            {
                memset(skuStr, 0, sizeof(skuStr));
                if ((len=read(skuFd, skuStr, sizeof(skuStr)-1)) < 0)
                {
                    regValue = 0x71;
                }
                else
                {
                    skuStr[len] = '\0';
                    regValue = strtol(skuStr, NULL, 16);
                }
                close(skuFd);
            }

            switch(regValue)
            {

                case 0x70:  /* AC3 customer board id */
                case 0x71:
                    xcat3BoardType = APP_DEMO_AC3BXH_WNC_OS6360P10;
                    break;
                case 0x77:
                    xcat3BoardType = APP_DEMO_AC3BXH_WNC_OS6360P48;
                    break;
                case 0x74:
                case 0x78:
                    xcat3BoardType = APP_DEMO_AC3BXH_WNC_OS6360P24;
                    break;

                case 0x30:
                    xcat3BoardType = APP_DEMO_AC3_BOARD_DB_CNS;
                    break;
                case 0x31: /* RD B2B Board - 48G + 4XG */
                    xcat3BoardType = APP_DEMO_AC3_BOARD_RD_MTL_TYPE1_CNS;
                    break;
                case 0x32: /* RD B2B Board - 48G + 2XG + 2XXG */
                    xcat3BoardType = APP_DEMO_AC3_BOARD_RD_MTL_TYPE2_CNS;
                    break;
                case 0x34: /* RD Board - 24G + 4SFP */
                    xcat3BoardType = APP_DEMO_AC3_BOARD_RD_MTL_TYPE3_CNS;
                    break;
                case 0x35: /* RD ac3s Board - 24G + 4SFP */
                    xcat3BoardType = APP_DEMO_AC3S_BOARD_RD_MTL_TYPE1_CNS;
                    break;
                default:
                    cpssOsPrintf("configBoardAfterPhase1: Unknown board type: %d\r\n", regValue);
                    xcat3BoardType = APP_DEMO_AC3BXH_WNC_OS6360P10;
                    break;
            }
        }

        if(GT_TRUE == xcat3BoardResetDone)
        {
            GT_U32 regAddr;  /* register address */
            GT_U32 regData;  /* register value*/

            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(dev)->
                         DFXServerUnitsDeviceSpecificRegs.deviceCtrl14;

            rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(dev, regAddr, 25, 1, &regData);
            if(rc != GT_OK)
            {
                return GT_BAD_STATE;
            }

            /* restore OOB port but not for case that CPSS run on MSYS.
               Because the procedure may kill OOB port and NFS will stuck CPU. */
            if((0 == regData) && (prvCpssDrvPpConfig[0]->hwInfo[0].busType != CPSS_HW_INFO_BUS_TYPE_MBUS_E))
            {
                MV_HWS_SERDES_CONFIG_STR    serdesConfig;

                cpssOsPrintf("Restore OOB port configuration after systemReset \n");

                /* configure SERDES TX interface */
                rc = mvHwsSerdesTxIfSelect(dev, 0, 10, COM_PHY_28NM, 3);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("mvHwsSerdesTxIfSelect", rc);
                if (GT_OK != rc)
                {
                    return rc;
                }
                rc = mvHwsSerdesTxIfSelect(dev, 0, 11, COM_PHY_28NM, 1);
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
                rc = mvHwsSerdesPowerCtrl(dev, 0, 10, GT_TRUE, &serdesConfig);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("mvHwsSerdesPowerCtrl", rc);
                if (GT_OK != rc)
                {
                    return rc;
                }

                rc = mvHwsSerdesPowerCtrl(dev, 0, 11, GT_TRUE, &serdesConfig);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("mvHwsSerdesPowerCtrl", rc);
                if (GT_OK != rc)
                {
                    return rc;
                }
            }
        }
        else
        {
            if((xcat3BoardType == APP_DEMO_AC3_BOARD_DB_CNS) &&
               (PRV_CPSS_DXCH_XCAT3_A0_CHECK_MAC(dev) == GT_TRUE) &&
               (prvCpssDrvPpConfig[dev]->hwInfo[0].busType == CPSS_HW_INFO_BUS_TYPE_MBUS_E) &&
               (appDemo_LSP_is_linux_2_6_32 == GT_TRUE))
            {
                GT_U32 cpuMacReg = 0x00072C0C;
                GT_U32 regData;

                /* OOB port may be killed during phase1Init because of PLL reconfiguration
                   for A0 revison. Reset OOB MAC to restore it's functionality. */
                osPrintf("Reset OOB MAC\n");
                osTimerWkAfter(10);

                cpuMacReg = 0x00072C08;
                prvCpssDrvHwPpReadInternalPciReg(dev,cpuMacReg,&regData);

                /* set Reset Bit to reset MAC */
                U32_SET_FIELD_MAC(regData, 6, 1, 1);
                prvCpssDrvHwPpWriteInternalPciReg(dev,cpuMacReg,regData);
                osTimerWkAfter(1);

                /* Un-Reset MAC */
                U32_SET_FIELD_MAC(regData, 6, 1, 0);
                prvCpssDrvHwPpWriteInternalPciReg(dev,cpuMacReg,regData);

                /* add time for LSP to take care of OOB port link change */
                osTimerWkAfter(10);
            }
        }
    }
#else
    /* simulation case - use RD_MTL_TYPE1 for B2B */
    if (ppCounter > 1)
    {
        xcat3BoardType = APP_DEMO_AC3_BOARD_RD_MTL_TYPE1_CNS;
    }

    if(explicit_simulation_board_type)
    {
        xcat3BoardType = explicit_simulation_board_type;
    }
#endif

    xcat3BoardResetDone = GT_FALSE;

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
    OUT CPSS_PP_PHASE2_INIT_PARAMS    *phase2Params
)
{
    CPSS_PP_PHASE2_INIT_PARAMS  localPpPh2Config = CPSS_PP_PHASE2_DEFAULT;
    GT_STATUS                   retVal = GT_OK;
    GT_U32                      tmpData;
    GT_U32                      rxDescNum = RX_DESC_NUM_DEF;
    GT_U32                      rxBufSize = RX_BUFF_SIZE_DEF;
    GT_U32                      txDescNum = TX_DESC_NUM_DEF;
    GT_U32                      auDescNum = AU_DESC_NUM_DEF;
#ifdef LINUX_NOKM
    GT_U32                      dmaSize = 0;
#endif


    localPpPh2Config.devNum     = oldDevNum;

    if(appDemoDbEntryGet("allocMethod", &tmpData) == GT_OK)
    {
        /* allow to update the default of the allocMethod */
        localPpPh2Config.netIfCfg.rxBufInfo.allocMethod = tmpData;

    }

    /* save the allocMethod*/
    appDemoPpConfigList[oldDevNum].allocMethod =  localPpPh2Config.netIfCfg.rxBufInfo.allocMethod;

    localPpPh2Config.deviceId   = appDemoPpConfigList[oldDevNum].deviceId;

    localPpPh2Config.fuqUseSeparate = GT_TRUE;
#ifdef LINUX_NOKM
    extDrvGetDmaSize(&dmaSize);
    if (dmaSize < _512K)
    {
        rxDescNum = 0;
        rxBufSize = 0;
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
        appDemoPpConfigList[oldDevNum].cpuPortMode = CPSS_NET_CPU_PORT_MODE_MII_E; /* TODO: none */
    }
    else if (dmaSize < _2M)
    {
        rxDescNum = 50;
        txDescNum = 50;
        auDescNum = 10;
        appDemoDbEntryAdd("rxDescNum", 50);
        appDemoDbEntryAdd("txDescNum", 50);
        appDemoDbEntryAdd("auDescNum", 10);
        appDemoDbEntryAdd("fuDescNum", 10);
    }
#endif
    if (appDemoPpConfigList[oldDevNum].cpuPortMode ==  CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        switch(localPpPh2Config.deviceId)
        {
            case PRV_CPSS_DXCH_XCAT3_DEVICES_CASES_MAC:

                if( appDemoDbEntryGet("useMultiNetIfSdma", &tmpData) == GT_NO_SUCH )
                {
                    localPpPh2Config.useMultiNetIfSdma = GT_TRUE;
                    if( appDemoDbEntryGet("skipTxSdmaGenDefaultCfg", &tmpData) == GT_NO_SUCH )
                    {

                        /* Enable Tx queue 3 to work in Tx queue generator mode */
                        retVal = appDemoDbEntryAdd("txGenQueue_3", GT_TRUE);
                        if(retVal != GT_OK)
                        {
                            return retVal;
                        }

                        /* Enable Tx queue 6 to work in Tx queue generator mode */
                        retVal = appDemoDbEntryAdd("txGenQueue_6", GT_TRUE);
                        if(retVal != GT_OK)
                        {
                            return retVal;
                        }
                    }
                }
                break;
            default:
                break;
        }
    }

    retVal = appDemoAllocateDmaMem(localPpPh2Config.deviceId, rxDescNum,
                                   rxBufSize, RX_BUFF_ALLIGN_DEF,
                                   txDescNum, auDescNum,
                                   &localPpPh2Config);

    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoAllocateDmaMem", retVal);

    osMemCpy(phase2Params,&localPpPh2Config,sizeof(CPSS_PP_PHASE2_INIT_PARAMS));

    return retVal;
}

/**
* @internal internalB2bXGPortConfig function
* @endinternal
*
* @brief   Internal function performs all needed configurations of 10G ports.
*
* @param[in] devNum                   - Device Id.
* @param[in] portNum                  - Port Number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS internalB2bXGPortConfig
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum
)
{
    GT_STATUS rc = GT_OK;

    CPSS_TBD_BOOKMARK

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        return rc;
    }

    if(PRV_CPSS_PP_MAC(devNum)->revision == 0)
    {
    /* set to DHX mode */
    rc = cpssDxChPortInterfaceModeSet(devNum,portNum,CPSS_PORT_INTERFACE_MODE_HX_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortInterfaceModeSet", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChPortSpeedSet(devNum,portNum,CPSS_PORT_SPEED_10000_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSpeedSet", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* swap lanes */
    rc = cpssDxChPortXgLanesSwapEnableSet(devNum,portNum,GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortXgLanesSwapEnableSet", rc);
    }

    return rc;
}

/**
* @internal internalXGPortConfig function
* @endinternal
*
* @brief   Internal function performs all needed configurations of 10G ports.
*
* @param[in] devNum                   - Device Id.
* @param[in] portNum                  - Port Number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS internalXGPortConfig
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum
)
{
    cpssOsPrintf("internalXGPortConfig().....devNum = 0x%x portNum= 0x%x ,\n",devNum,portNum);
    return GT_OK;
}

/**
* @internal xcat3CascadePortConfig function
* @endinternal
*
* @brief   XCAT3 cascade port configuration
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Cascade port number.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - HW failure
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS xcat3CascadePortConfig
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS           rc;             /* return code */
    CPSS_PORTS_BMP_STC  initPortsBmp;   /* bitmap of ports to init */
    GT_U32              portIndex;      /* port index in DB */
    GT_U32              serdesNum;      /* lanes number */
    GT_U32              startSerdes;    /* start serdes number */
    GT_U32              i;              /* serdes loop iterator */

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,portNum);

    cpssOsPrintf("Allen : xcat3CascadePortConfig devNum=%d, portNum=%d\n",devNum ,portNum);

    /* Allen */
    if (portNum == 0)
    {
        return GT_OK;
    }

    /* Klocwork fix */
    if(devNum > 1)
    {
        return GT_BAD_PARAM;
    }

    if(xcat3CscdPortDb[devNum][0].portNum == portNum)
    {
        portIndex = 0;
    }
    else if(xcat3CscdPortDb[devNum][1].portNum == portNum)
    {
        portIndex = 1;
    }
    else
    {
        return GT_BAD_PARAM;
    }

    if((portNum == 25) || (portNum ==27))
    {
        rc = cpssDxChPortExtendedModeEnableSet(devNum, portNum, xcat3CscdPortDb[devNum][portIndex].isExtendedMac);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortExtendedModeEnableSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = cpssDxChPortModeSpeedSet(devNum, &initPortsBmp, GT_TRUE, xcat3CscdPortDb[devNum][portIndex].ifMode, xcat3CscdPortDb[devNum][portIndex].speed);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, xcat3CscdPortDb[devNum][portIndex].ifMode, &startSerdes, &serdesNum);
    if(GT_OK != rc)
    {
        return rc;
    }

    for(i = 0; i < serdesNum; i++)
    {
        if((xcat3CscdPortDb[devNum][portIndex].rxLanesPolarity[i] != GT_FALSE) ||
           (xcat3CscdPortDb[devNum][portIndex].txLanesPolarity[i] != GT_FALSE))
        {
            rc = cpssDxChPortSerdesPolaritySet(devNum,
                                               portNum,
                                               (1 << i),
                                               xcat3CscdPortDb[devNum][portIndex].txLanesPolarity[i],
                                               xcat3CscdPortDb[devNum][portIndex].rxLanesPolarity[i]);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

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
    GT_STATUS               rc = GT_OK;     /* return code */
    CPSS_PORT_SPEED_ENT     speed;          /* port speed */


    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
       rc = xcat3CascadePortConfig(devNum, portNum);
        if(GT_OK != rc)
        {
             return rc;
         }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        /* For xCat2 B2B set cascade ports on both devices to SGMII 2.5 */
        rc = gtAppDemoXcat2StackPortsModeSpeedSet(devNum, portNum,
                                CPSS_PORT_INTERFACE_MODE_SGMII_E, CPSS_PORT_SPEED_2500_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("gtAppDemoXcat2StackPortsModeSpeedSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* set cascade port to HGS mode */
        rc = cpssDxChPortInterfaceModeSet(devNum,portNum,CPSS_PORT_INTERFACE_MODE_XGMII_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortInterfaceModeSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        speed = CPSS_PORT_SPEED_12000_E;
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) &&
           (PRV_CPSS_PP_MAC(devNum)->revision <= 2))
        {
            /* the xCat A1 works OK on 10G only */
            speed = CPSS_PORT_SPEED_10000_E;
        }

        rc = cpssDxChPortSpeedSet(devNum, portNum, speed);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSpeedSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortSerdesPowerStatusSet(devNum,portNum,CPSS_PORT_DIRECTION_BOTH_E,0xf,GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesPowerStatusSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* set the MRU of the cascade port to be big enough for DSA tag */
    rc = cpssDxChPortMruSet(devNum, portNum, 1536);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortMruSet", rc);

    return rc;
}

#ifdef DRAGONITE_TYPE_A1

extern GT_U32 calcChecksum
(
     IN const GT_VOID *msg
);

extern GT_VOID endiannessSet
(
    GT_VOID
);

extern GT_VOID dataSwap
(
    GT_VOID
);


GT_STATUS appDemoBspDragoniteGetIntVec(void);
GT_STATUS appDemoBspDragoniteSWDownload(void);
void debugDragoniteDump
(
    IN GT_BOOL memType,
    IN GT_U32 offset,
    IN GT_U32 mask
);
GT_STATUS appDemoCpssDragoniteReadData
(
    IN GT_U32 structType,
    IN GT_U32 offset,
    IN GT_U32 mask
);
GT_STATUS debugDragoniteRawWrite
(
    IN GT_BOOL memType,
    IN GT_U32 offset,
    IN GT_U32 mask,
    IN GT_U32 value
);
GT_STATUS debugDragoniteWrite
(
    IN GT_U32 structType,
    IN GT_U32 offset,
    IN GT_U32 mask,
    IN GT_U32 value
);

extern GT_32 gtPpFd;                /* pp file descriptor           */
static GT_U32 dragoniteIntVecNum;
extern GT_BOOL dragoniteDbgDumpsOn;
#define INLP_DBG_INFO(x)     if(dragoniteDbgDumpsOn) cpssOsPrintf x

GT_STATUS dragoniteDbgDumpsEn
(
    IN GT_BOOL dumpsOn
);

GT_STATUS appDemoCpssDragonitePortEnable
(
    IN GT_U32 portNum,
    IN GT_BOOL enable
);

GT_STATUS debugDragoniteWriteTrigger
(
    IN GT_BOOL memType /* GT_TRUE - comm prot mem; GT_FALSE - irq mem; */
);

#if (defined _linux) && (!defined LINUX_SIM)
extern GT_STATUS extDrvDragoniteSWDownload
(
    GT_VOID
);

extern GT_STATUS extDrvDragoniteGetIntVec
(
    OUT GT_U32 *intVec
);

extern GT_STATUS extDrvDragoniteInit
(
    GT_VOID
);

extern GT_STATUS extDrvDragoniteEnableSet
(
    IN  GT_BOOL enable
);
#endif

/**
* @internal appDemoCpssDragonitePortEnable function
* @endinternal
*
* @brief   This function is terminal wrapper to enable disable power supply on port
*         using standard CPSS API's
* @param[in] portNum                  - Dragonite port number
* @param[in] enable                   - port power on/off
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if port command table read failed
*                                       more see cpssDragoniteReadTrigger, cpssDragoniteReadData, cpssDragoniteWrite
*/
GT_STATUS appDemoCpssDragonitePortEnable
(
    IN GT_U32 portNum,
    IN GT_BOOL enable
)
{
    CPSS_GEN_DRAGONITE_DATA_STC data;
    GT_STATUS rc;

    if((rc = cpssDragoniteReadTrigger(CPSS_GEN_DRAGONITE_DATA_TYPE_PORT_COMMANDS_E)) != GT_OK)
    {
        cpssOsPrintf("appDemoCpssDragonitePortEnable:cpssDragoniteReadTrigger(CPSS_GEN_DRAGONITE_DATA_TYPE_PORT_COMMANDS_E) failed\n");
        debugDragoniteDump(GT_TRUE, 0x7fffffff, 0);
        return rc;
    }

    /* wait for Dragonite bring data */
    (void)cpssOsTimerWkAfter(200);

    if((rc = cpssDragoniteReadData(&data)) != GT_OK)
    {
        cpssOsPrintf("appDemoCpssDragonitePortEnable:cpssDragoniteReadData failed\n");
        debugDragoniteDump(GT_TRUE, 0x7fffffff, 0);
        return rc;
    }

    if(data.dataType != CPSS_GEN_DRAGONITE_DATA_TYPE_PORT_COMMANDS_E)
    {
        cpssOsPrintf("appDemoCpssDragonitePortEnable:cpssDragoniteReadData dataType=%d\n", data.dataType);
        debugDragoniteDump(GT_TRUE, 0x7fffffff, 0);
        return GT_FAIL;
    }

    data.dragoniteData.portCmd[portNum].portCtrl.pseEnable = enable;

    if((rc = cpssDragoniteWrite(&data)) != GT_OK)
    {
        cpssOsPrintf("appDemoCpssDragonitePortEnable:cpssDragoniteWrite failed\n");
        debugDragoniteDump(GT_TRUE, 0x7fffffff, 0);
        return rc;
    }

    /* wait for Dragonite bring data */
    (void)cpssOsTimerWkAfter(200);

    if((rc = cpssDragoniteReadData(&data)) != GT_OK)
    {
        cpssOsPrintf("appDemoCpssDragonitePortEnable:cpssDragoniteReadData failed\n");
        debugDragoniteDump(GT_TRUE, 0x7fffffff, 0);
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoBspDragoniteSWDownload function
* @endinternal
*
* @brief   This function is terminal wrapper to engage bsp/lsp api to download
*         Dragonite firmware to instruction part of shared memory
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoBspDragoniteSWDownload(void)
{
#ifndef ASIC_SIMULATION
    GT_STATUS rc;
#ifdef _linux

    if((rc = extDrvDragoniteSWDownload()) != GT_OK)
    {
        return rc;
    }

    /* for debug purposes only, actually lsp executes CRC
        check after FW download automatically */
    return extDrvDragoniteFwCrcCheck();

#else /* if not linux */

    GT_VOID   *sourcePtr = NULL; /* pointer to firmware */
    GT_U32    size;       /* size of firmware to download */
    extern GT_U8 binArrayStart[];
    extern GT_U8 binArrayEnd;

    CPSS_NULL_PTR_CHECK_MAC(binArrayStart);
    sourcePtr = binArrayStart;
    size = (GT_U32)(&binArrayEnd) - (GT_U32)binArrayStart;

    INLP_DBG_INFO(("appDemoBspDragoniteSWDownload:sourcePtr=0x%x,size=%d\n", sourcePtr, size));

    if((rc = bspDragoniteSWDownload(sourcePtr, size)) != GT_OK)
    {
        return rc;
    }

    /* for debug purposes only, actually bspDragoniteSWDownload
        executes CRC check after FW download automatically */
    return bspDragoniteFwCrcCheck();
#endif

#else /* if simulation */

    return bspDragoniteSWDownload(NULL, 0);

#endif
}

/**
* @internal appDemoBspDragoniteGetIntVec function
* @endinternal
*
* @brief   This function is terminal wrapper to engage bsp/lsp api to get
*         Dragonite interrupt vector number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note To see output must first engage dragoniteDbgDumpsEn 1
*
*/
GT_STATUS appDemoBspDragoniteGetIntVec(void)
{
    GT_STATUS rc = GT_OK;

#if (defined _linux) && (!defined LINUX_SIM)
    rc = extDrvDragoniteGetIntVec(&dragoniteIntVecNum);
#else
    rc = bspDragoniteGetIntVec(&dragoniteIntVecNum);
#endif

    if(rc != GT_OK)
    {
        INLP_DBG_INFO(("bspDragoniteGetIntVec fail:0x%x\n", rc));
    }
    else
    {
        INLP_DBG_INFO(("appDemoBspDragoniteGetIntVec:dragoniteIntVecNum=%d\n", dragoniteIntVecNum));
    }

    return rc;
}

/**
* @internal debugDragoniteDump function
* @endinternal
*
* @brief   This function is terminal wrapper to dump content of specific part of
*         Dragonite shared memory to terminal
* @param[in] memType                  - GT_TRUE - comm prot mem; GT_FALSE - irq mem;
* @param[in] offset                   - in words, 0x7fffffff - dump full mem of requested type;
*                                      (used signed mask, because of Linux command line treatment specific)
* @param[in] mask                     -  of bits to update in word (not relevant if offset==0x7fffffff)
*                                       None.
*
* @note To see output must first engage dragoniteDbgDumpsEn 1
*
*/
void debugDragoniteDump
(
    IN GT_BOOL memType,
    IN GT_U32 offset,
    IN GT_U32 mask
)
{
    GT_U32 i;
    GT_U32 *memPtr = NULL;
    GT_U32 memSize; /* in words */
    GT_U32 *addr;

    INLP_DBG_INFO(("debugDragoniteDump(%d, 0x%x, %d)\n", memType, offset, mask));

    cpssExtDrvDragoniteShMemBaseAddrGet((GT_U32*)(GT_VOID*)&irqMemPtr);
    if(irqMemPtr == NULL)
    {
        cpssOsPrintf("debugDragoniteDump: failed to get DragoniteShMemBaseAddr\n");
        return;
    }

    /* Init interrupts mechanism */
#ifdef ASIC_SIMULATION
    commMsgPtr = (GT_U32*)&InputMsg;
#else
    commMsgPtr = irqMemPtr + PRV_CPSS_DRAGONITE_COMM_MSG_MEM_OFFSET_CNS;
#endif

    INLP_DBG_INFO(("debugDragoniteDump:commMsgPtr=0x%08x,irqMemPtr=0x%08x\n", commMsgPtr, irqMemPtr));
    INLP_DBG_INFO(("Note: no swap done in this function\n"));

    if(memType)
    {
        memPtr = (GT_U32*)commMsgPtr;
        memSize = PRV_CPSS_DRAGONITE_MAX_DATA_SIZE_CNS >> 2;
        INLP_DBG_INFO(("----- COMM_MSG memory dump base address:0x%08x -------\n", (GT_U32)memPtr));
    }
    else
    {
        memPtr = irqMemPtr;
        memSize = PRV_CPSS_DRAGONITE_MAX_IRQ_MEM_SIZE_CNS >> 2;
        INLP_DBG_INFO(("----- IRQ memory dump base address:0x%08x -------\n", (GT_U32)memPtr));
    }

    endiannessSet();

    if(offset == 0x7fffffff)
    {
        INLP_DBG_INFO(("start address: 0x%08x, size:%d bytes\n", memPtr, memSize<<2));
        for(i = 0; i < memSize; i++)
        {
            INLP_DBG_INFO(("[0x%08x] 0x%08x\n", (GT_U32)(memPtr+i), (GT_U32)*((GT_U32*)(memPtr+i))));
        }
    }
    else
    {
        if(offset > memSize)
        {
            INLP_DBG_INFO(("Illigal offset: 0x%08x, size:%d bytes\n", offset, memSize<<2));
            return;
        }
        addr = (memType > 0) ? PRV_CPSS_DRAGONITE_MSG_DATA_PTR_MAC + offset : memPtr + offset;
        INLP_DBG_INFO(("address: 0x%08x, data:0x%x\n", addr, ((*(GT_U32*)(addr))&mask)));
    }

    INLP_DBG_INFO(("-------------------------------\n"));

    return;
}

/**
* @internal dragoniteRead function
* @endinternal
*
* @brief   This function executes read operation of specific
*         Dragonite's data structure
* @param[in] structType               - Dragonite structure type
*
* @param[out] dataPtr                  - pointer to dragonite data in application format
*                                       see cpssDragoniteReadTrigger and cpssDragoniteReadData
*/
static GT_STATUS dragoniteRead
(
    IN CPSS_GEN_DRAGONITE_DATA_TYPE_ENT structType,
    OUT CPSS_GEN_DRAGONITE_DATA_STC *dataPtr
)
{
    GT_U32 timeout;
    GT_STATUS rc = GT_OK;

    endiannessSet();

    timeout = 100;
    do
    {
        rc = cpssDragoniteReadTrigger(structType);
        if(rc == GT_NOT_READY)
        {
            if((timeout--) == 0)
            {
                cpssOsPrintf("dragoniteRead: cpssDragoniteReadTrigger timeout\n");
                return GT_NOT_READY;
            }
        }
        else if(rc != GT_OK)
        {
            cpssOsPrintf("dragoniteRead:cpssDragoniteReadTrigger fail:0x%x\n", rc);
            return rc;
        }
        else
            break;
    }while(timeout > 0);

    (void)cpssOsTimerWkAfter(200);

    timeout = 100;
    do
    {
        rc = cpssDragoniteReadData(dataPtr);
        if(rc == GT_NOT_READY)
        {
            if((timeout--) == 0)
            {
                cpssOsPrintf("dragoniteRead: cpssDragoniteReadData timeout\n");
                break;
            }
        }
        else if(rc != GT_OK)
        {
            cpssOsPrintf("dragoniteRead:cpssDragoniteReadData fail:0x%x\n", rc);
            break;
        }
        else
            break;
    }while(timeout > 0);

    return rc;
}

/**
* @internal appDemoCpssDragoniteReadData function
* @endinternal
*
* @brief   This function is terminal wrapper to execute read operation of specific
*         Dragonite's data structure
* @param[in] structType               -
*                                      Structure_Type Value
*                                      Config         0
*                                      System         1
*                                      chip           2
*                                      Port_Commands  3
*                                      Port_Counters  4
*                                      Port_Statuses  5
*                                      Port_Measurements  6
*                                      Port_Layer2        7
*                                      Debug          8
*                                      IRQ            10
* @param[in] offset                   - in words, use 0x7fffffff to write back full structure as is
* @param[in] mask                     -  of bits to update in word (not relevant if offset==0x7fffffff)
*                                       GT_OK always
*
* @retval GT_BAD_PARAM             - wrong structure type
*                                       more see cpssDragoniteReadData
*/
GT_STATUS appDemoCpssDragoniteReadData
(
    IN GT_U32 structType,
    IN GT_U32 offset,
    IN GT_U32 mask
)
{
    GT_STATUS rc = GT_OK;
    CPSS_GEN_DRAGONITE_DATA_STC dragoniteData;

    endiannessSet();

    if(structType == 10)
    {
        debugDragoniteDump(GT_FALSE, offset, mask);
    }
    else if(structType < 10)
    {
        rc = dragoniteRead((CPSS_GEN_DRAGONITE_DATA_TYPE_ENT)structType, &dragoniteData);

        /* dump memory content in any case */
        debugDragoniteDump(GT_TRUE, offset, mask);
    }
    else
    {
        cpssOsPrintf("appDemoCpssDragoniteReadData: wrong structType=%d\n", structType);
        return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal debugDragoniteWriteTrigger function
* @endinternal
*
* @brief   This function is terminal wrapper to trigger write operation of Dragonite
*         shared memory
* @param[in] memType                  - GT_TRUE   - communication protocol memory;
*                                      GT_FALSE  - irq memory;
*                                       GT_OK always
*/
GT_STATUS debugDragoniteWriteTrigger
(
    IN GT_BOOL memType
)
{
    endiannessSet();

    if(memType)
    {
        /* set write operation flag */
        PRV_CPSS_DRAGONITE_RW_FLAG_WRITE_SET_MAC;

        {
            GT_U32 tmpChsum;

            /* send pointer for start of checksum */
            tmpChsum = calcChecksum(PRV_CPSS_DRAGONITE_MSG_TYPE_LONG_PTR_MAC);
            /* add value of DRAGONITE_MO_FLAG, that is set on message send */
            tmpChsum += 0x80;

            PRV_CPSS_DRAGONITE_HDR_CHECKSUM_SET_MAC(tmpChsum);
        }

        INLP_DBG_INFO(("Dump message to be written (checksum already contains MO_FLAG):\n"));
        debugDragoniteDump(GT_TRUE, 0x7fffffff, 0);

        dataSwap();

        /* pass message to DRAGONITE controller */
        PRV_CPSS_DRAGONITE_MO_FLAG_DRAGONITE_SET_MAC;
    }
    else
    {
        INLP_DBG_INFO(("Dump updated IRQ memory before change ownership:\n"));
        debugDragoniteDump(GT_FALSE, 0x7fffffff, 0);

        /* pass memory ownership */
        PRV_CPSS_DRAGONITE_IRQ_IMO_FLAG_SET_MAC(irqMemPtr, 1);
    }

    return GT_OK;
}

/**
* @internal debugDragoniteRawWrite function
* @endinternal
*
* @brief   This function is terminal wrapper to update one word in Dragonite shared
*         memory
* @param[in] memType                  - GT_TRUE - comm prot mem; GT_FALSE - irq mem;
* @param[in] offset                   - in words, if 0x7fffffff nothing to do
* @param[in] mask                     -  of bits to update in word (not relevant if offset==0x7fffffff)
* @param[in] value                    - new  (not relevant if offset==0x7fffffff)
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_READY             - memory in Dragonite's ownership
* @retval GT_BAD_PTR               - bad pointer to shared memory
*
* @note Pay attention - no preliminary read, no write trigger after - just one
*       word raw update
*
*/
GT_STATUS debugDragoniteRawWrite
(
    IN GT_BOOL memType,
    IN GT_U32 offset,
    IN GT_U32 mask,
    IN GT_U32 value
)
{
    GT_U32 timeout;
    GT_U32 *memPtr = NULL;
    GT_U32 memSize; /* in words */
    GT_U32 *addr;

    cpssExtDrvDragoniteShMemBaseAddrGet((GT_U32*)(GT_VOID*)&irqMemPtr);
    if(irqMemPtr == NULL)
    {
        cpssOsPrintf("dragoniteRawWrite: failed to get DragoniteShMemBaseAddr\n");
        return GT_BAD_PTR;
    }

    /* Init interrupts mechanism */
#ifdef ASIC_SIMULATION
    commMsgPtr = (GT_U32*)&InputMsg;
#else
    commMsgPtr = irqMemPtr + PRV_CPSS_DRAGONITE_COMM_MSG_MEM_OFFSET_CNS;
#endif
    INLP_DBG_INFO(("dragoniteRawWrite:commMsgPtr=0x%08x,irqMemPtr=0x%08x\n", commMsgPtr, irqMemPtr));

    if(memType)
    {
        INLP_DBG_INFO(("----- COMM_MSG memory write -------\n"));
        memPtr = (GT_U32*)commMsgPtr;

        for(timeout = 100;
            (timeout > 0) && PRV_CPSS_DRAGONITE_MO_FLAG_GET_MAC;
            timeout--);
    }
    else
    {
        INLP_DBG_INFO(("----- IRQ memory write -------\n"));
        memPtr = irqMemPtr;
        for(timeout = 100;
            (timeout > 0) && PRV_CPSS_DRAGONITE_IRQ_IMO_FLAG_GET_MAC(((PRV_CPSS_DRAGONITE_IRQ_MEMORY_STC*)memPtr));
            timeout--);
    }

    if(timeout == 0)
    {
        cpssOsPrintf("dragoniteRawWrite:BUSY timeout\n");
        return GT_NOT_READY;
    }

    memSize = (memType) ? (PRV_CPSS_DRAGONITE_MAX_DATA_SIZE_CNS >> 2)
                            : (PRV_CPSS_DRAGONITE_MAX_IRQ_MEM_SIZE_CNS >> 2);

    endiannessSet();

    if(offset != 0x7fffffff)
    {
        if(offset >= memSize)
        {
            cpssOsPrintf("Illigal offset: 0x%08x, size:%d bytes\n", offset, memSize<<2);
            return GT_BAD_PARAM;
        }

        /* no need to swap here comm protocol data, because except for automatically brought by Dragonite
         * in first time system structure, other structures are swapped, by cpssDragoniteReadData
         * and then by dragoniteWriteTrigger, but irq data must be swapped
         */
        if(memType)
        {
            addr = PRV_CPSS_DRAGONITE_MSG_DATA_PTR_MAC + offset;
            *(GT_U32*)addr &= (~mask);            /* Turn the field off.                        */
            *(GT_U32*)addr |= (value & mask);     /* Insert the new value of field in its place.*/
        }
        else
        {
            addr = memPtr + offset;
            if(bigEndian)
            {/* if host cpu big-endian swap data before update */
                *addr = BYTESWAP_MAC(*addr);
            }
            *(GT_U32*)addr &= (~mask);            /* Turn the field off.                        */
            *(GT_U32*)addr |= (value & mask);     /* Insert the new value of field in its place.*/
            if(bigEndian)
            {/* if host cpu big-endian swap data back to Dragonite's little-endian format */
                *addr = BYTESWAP_MAC(*addr);
            }
        }

        cpssOsPrintf("address: 0x%08x, new value:0x%x\n", addr, *(GT_U32*)addr);
    }

    return GT_OK;
}

/**
* @internal debugDragoniteWrite function
* @endinternal
*
* @brief   This function is terminal wrapper executing Dragonite shared memory
*         write by read/update/write
* @param[in] structType               -
*                                      Structure_Type Value
*                                      Config         0
*                                      System         1
*                                      chip           2
*                                      Port_Commands  3
*                                      Port_Counters  4
*                                      Port_Statuses  5
*                                      Port_Measurements  6
*                                      Port_Layer2        7
*                                      Debug          8
*                                      IRQ            10
* @param[in] offset                   - in words, use 0x7fffffff to write back full structure as is
* @param[in] mask                     -  of bits to update in word (not relevant if offset==0x7fffffff)
* @param[in] value                    - new  (not relevant if offset==0x7fffffff)
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong structType
*                                       more see cpssDragoniteReadTrigger and cpssDragoniteReadData
*
* @note Pay attention - write executed by direct raw data write and not
*       by cpssDragoniteWrite
*
*/
GT_STATUS debugDragoniteWrite
(
    IN GT_U32 structType,
    IN GT_U32 offset,
    IN GT_U32 mask,
    IN GT_U32 value
)
{
    GT_STATUS rc = GT_OK;
    CPSS_GEN_DRAGONITE_DATA_STC dragoniteData;

    endiannessSet();

    if(structType == 10)
    {
        if((rc = debugDragoniteRawWrite(GT_FALSE,offset,mask,value)) != GT_OK)
        {
            return rc;
        }

        rc = debugDragoniteWriteTrigger(GT_FALSE);
    }
    else if(structType < 10)
    {
        if((rc = dragoniteRead((CPSS_GEN_DRAGONITE_DATA_TYPE_ENT)structType, &dragoniteData)) != GT_OK)
        {
            return rc;
        }

        if((rc = debugDragoniteRawWrite(GT_TRUE,offset,mask,value)) != GT_OK)
        {
            return rc;
        }

        rc = debugDragoniteWriteTrigger(GT_TRUE);
        if(rc == GT_OK)
        {
            (void)cpssOsTimerWkAfter(50); /* give time to prepare answer */
            rc = cpssDragoniteReadData(&dragoniteData);
            if(rc != GT_OK)
            {
                cpssOsPrintf("debugDragoniteWrite:cpssDragoniteReadData fail:0x%x\n", rc);
            }
        }
        INLP_DBG_INFO(("Dump answer for write operation\n"));
        debugDragoniteDump(GT_TRUE, 0x7fffffff, 0);
    }
    else
    {
        cpssOsPrintf("debugDragoniteWrite: wrong structType=%d\n", structType);
        return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal dragoniteDbgDumpsEn function
* @endinternal
*
* @brief   This function is terminal wrapper to enable/disable Dragonite debug
*         messages print.
*
* @note It's invented mostly for Linux, because in vxWorks debug dump flag
*       could be set directly
*
*/
GT_STATUS dragoniteDbgDumpsEn
(
    IN GT_BOOL dumpsOn
)
{
    dragoniteDbgDumpsOn = dumpsOn;
    cpssOsPrintf("dragoniteDbgDumpsEn:dragoniteDbgDumpsOn=%d\n", dragoniteDbgDumpsOn);

    return GT_OK;
}

/**
* @internal appDemoCpssDragoniteConfigWrite function
* @endinternal
*
* @brief   This function is terminal wrapper executing Dragonite configuration
*         structure write with mostly default values and then response check
*
* @retval GT_OK                    - on success,
*                                       otherwise see cpssDragoniteWrite and cpssDragoniteReadData
*/
GT_STATUS appDemoCpssDragoniteConfigWrite
(
    GT_VOID
)
{
    CPSS_GEN_DRAGONITE_DATA_STC xCatPkt;
    GT_STATUS rc;
    GT_U32 i, j, k;
    GT_U32  msccChipType, /* type of MSCC chip */
            maxChipNum,   /* maximum number of chips on board accordingly to
                            type of MSCC chip */
            maxPortNum; /* maximum number of ports per chip accordingly to
                            type of MSCC chip */

    cpssOsMemSet((void *)&xCatPkt, 0, sizeof(CPSS_GEN_DRAGONITE_DATA_STC));

    xCatPkt.dataType = CPSS_GEN_DRAGONITE_DATA_TYPE_CONFIG_E;

    /* default values, just capDis set 0 to support some proprietary protocol */
    xCatPkt.dragoniteData.config.dcDisconnectEn = GT_TRUE;
    xCatPkt.dragoniteData.config.externalSyncDis = GT_FALSE;
    xCatPkt.dragoniteData.config.capDis = GT_FALSE;
    xCatPkt.dragoniteData.config.disPortsOverride = GT_FALSE;
    xCatPkt.dragoniteData.config.rprDisable = GT_FALSE;
    xCatPkt.dragoniteData.config.vmainAtPolicyEn = GT_FALSE;
    xCatPkt.dragoniteData.config.class0EqAf = GT_FALSE;
    xCatPkt.dragoniteData.config.class123EqAf = GT_FALSE;
    xCatPkt.dragoniteData.config.classBypass2ndError = GT_FALSE;
    xCatPkt.dragoniteData.config.classErrorEq0 = GT_FALSE;
    xCatPkt.dragoniteData.config.classErrorEq4 = GT_FALSE;
    xCatPkt.dragoniteData.config.layer2En = GT_FALSE;
    xCatPkt.dragoniteData.config.portPrioritySetByPD = GT_FALSE;
    xCatPkt.dragoniteData.config.privateLableBit = GT_TRUE;
    xCatPkt.dragoniteData.config.lowPriDiscoForHiPriStartupEn = 0;
    xCatPkt.dragoniteData.config.portMethodLimitMode = 1;
    xCatPkt.dragoniteData.config.portMethodCalcMode = 1;
    xCatPkt.dragoniteData.config.guardBandValue = 0;
    xCatPkt.dragoniteData.config.startupHiPrOverride = GT_FALSE;
    xCatPkt.dragoniteData.config.IcutMaxAt = 0x84;
    xCatPkt.dragoniteData.config.tempAlarmTh = 0x184;
    xCatPkt.dragoniteData.config.vmainHighTh = 0x3bc;
    xCatPkt.dragoniteData.config.vmainAtLowTh = 0x313;
    xCatPkt.dragoniteData.config.vmainAfLowTh = 0x2b0;

    if(appDemoDbEntryGet("msccChipType", &msccChipType) != GT_OK)
    {/* by default use "old" chips architecture */
        msccChipType = 0;
    }

    if(0 == msccChipType)
    {
        maxChipNum = 8;
        maxPortNum = 12;
    }
    else
    {
        maxChipNum = 12;
        maxPortNum = 8;
    }

    for (i=0, k=0; i<maxChipNum; i++)
    {
        for (j=0; j<maxPortNum; j++, k++)
        {
            xCatPkt.dragoniteData.config.matrixPort[k].chipNumber = (GT_U8)i;
            xCatPkt.dragoniteData.config.matrixPort[k].physicalPortNumber = (GT_U8)j;
        }
    }
    xCatPkt.dragoniteData.config.commPollingTime = 0x19;
    xCatPkt.dragoniteData.config.irqGenTime = 0x19;

    if((rc = cpssDragoniteWrite(&xCatPkt)) != GT_OK)
    {
        cpssOsPrintf("appDemoCpssDragoniteConfigWrite:cpssDragoniteWrite fail:0x%x\n", rc);
        debugDragoniteDump(GT_TRUE, 0x7fffffff, 0);
        return rc;
    }

    (void)cpssOsTimerWkAfter(50); /* give time to prepare answer */
    rc = cpssDragoniteReadData(&xCatPkt);
    if(rc != GT_OK)
    {
        cpssOsPrintf("appDemoCpssDragoniteConfigWrite:cpssDragoniteReadData fail:0x%x\n", rc);
    }

    INLP_DBG_INFO(("Dump answer for write operation\n"));
    debugDragoniteDump(GT_TRUE, 0x7fffffff, 0);

    return rc;
}

/**
* @internal prvCpssDragoniteInit function
* @endinternal
*
* @brief   This function performs DRAGONITE initialization sequence.
*         Could be engaged after system reset or during regular work to
*         update firmware.
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_READY             - if DRAGONITE controller still not prepared System structure.
*
* @note not defined as static to enable call from terminal
*
*/
GT_STATUS prvCpssDragoniteInit
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    value;

    if(appDemoDbEntryGet("dragonite", &value) == GT_OK)
    {
        if(value == 1)
        {
#if (defined _linux) && (!defined LINUX_SIM)
            rc = extDrvDragoniteInit();
#else
            rc = bspDragoniteInit();
#endif
            if(rc == GT_FAIL)
            {
                return GT_HW_ERROR;
            }
            else if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            return GT_OK;
        }
    }
    else
    {
        return GT_OK;
    }

    if((rc = appDemoBspDragoniteSWDownload()) != GT_OK)
    {
        return rc;
    }

    (void)osTimerWkAfter(500); /* be sure download finished */

#if (defined _linux) && (!defined LINUX_SIM)
    if((rc = extDrvDragoniteEnableSet(GT_TRUE)) != GT_OK)
    {
        return rc;
    }
#else
    if((rc = bspDragoniteEnableSet(GT_TRUE)) != GT_OK)
    {
        return rc;
    }
#endif

    (void)osTimerWkAfter(500); /* give Dragonite time to boot-up */

    if((rc = appDemoBspDragoniteGetIntVec()) != GT_OK)
    {
        return rc;
    }

    rc = cpssDragoniteInit(dragoniteIntVecNum, dragoniteIntVecNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDragoniteInit", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoCpssDragoniteConfigWrite();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoCpssDragoniteConfigWrite", rc);

    return rc;
}

#endif /* DRAGONITE_TYPE_A1 */


GT_U8 phyAddr1680_binding[2][24] =
{
    {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, /*p11*/ 0xC, 0xD, 0xE, 0xF, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF /*p23*/},
    {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, /*p11*/ 0xC, 0xD, 0xE, 0xF, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF /*p23*/},
};

GT_U8 phyAddr1680_binding_MTL_RD_24G_4SFP[24] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};

GT_U8 phyAddr1680_binding_WNC_OS6360P10_8G_2G_2SFP[16] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};

GT_U8 phyAddr1680_binding_WNC_OS6360P24[24] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};

GT_U8 phyAddr1680_binding_WNC_OS6360P48[2][24] =
{
    {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 /*p23*/},
    {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 /*p23*/},
};

static GT_STATUS xcat3Phy1680AddrInit_WNC8G2G2SFP
(
    IN GT_U8 devNum
)
{
    GT_STATUS   retVal;     /* return code */
    GT_U8       port;       /* port number */

    cpssOsPrintf(" \n xcat3Phy1680AddrInit_WNC8G2G2SFP() devNum = %d\n", devNum);

    for (port = 0; port < 8 ;port++)
    {
        if( !PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, port) )
            continue;

        cpssOsPrintf(" port = %d, phyaddr= 0x%x \n", port, phyAddr1680_binding_WNC_OS6360P10_8G_2G_2SFP[port]);

        retVal = appDemoSetPortPhyAddr(devNum, port, phyAddr1680_binding_WNC_OS6360P10_8G_2G_2SFP[port]);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}

static GT_STATUS xcat3Phy1680AddrInit_WNCOS6360P24
(
    IN GT_U8 devNum
)
{
    GT_STATUS   retVal;     /* return code */
    GT_U8       port;       /* port number */

    cpssOsPrintf(" \n xcat3Phy1680AddrInit_WNCOS6360P24() devNum = %d\n", devNum);

    for (port = 0; port < 24 ;port++)
    {
        if( !PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, port) )
            continue;

        cpssOsPrintf(" port = %d, phyaddr= 0x%x \n", port, phyAddr1680_binding_WNC_OS6360P24[port]);

        retVal = appDemoSetPortPhyAddr(devNum, port, phyAddr1680_binding_WNC_OS6360P24[port]);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}

static GT_STATUS xcat3Phy1680AddrInit_WNCOS6360P48
(
    IN GT_U8 devNum
)
{
    GT_STATUS   retVal;     /* return code */
    GT_U8       port;       /* port number */
    GT_U8       total_port;

    cpssOsPrintf(" \n xcat3Phy1680AddrInit_WNCOS6360P48() devNum = %d\n", devNum);

    if (devNum == 0)
        total_port = 24;
    else
        total_port = 22;

    for (port = 0; port < total_port ;port++)
    {
        if( !PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, port) )
            continue;

        cpssOsPrintf(" port = %d, phyaddr= 0x%x \n", port, phyAddr1680_binding_WNC_OS6360P48[devNum][port]);

        retVal = appDemoSetPortPhyAddr(devNum, port, phyAddr1680_binding_WNC_OS6360P48[devNum][port]);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}

/**
* @internal xcat3Phy1680AddrInit function
* @endinternal
*
* @brief   xCat3 PHY1680 SMI address init.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS xcat3Phy1680AddrInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS   retVal;     /* return code */
    GT_U8       port;       /* port number */

    for (port = 0; port < 24;port++)
    {
        if( !PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, port) )
            continue;

        retVal = appDemoSetPortPhyAddr(devNum, port, phyAddr1680_binding[devNum][port]);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}

/**
* @internal xcat3Phy1680AddrInit_MTL24G4SFP function
* @endinternal
*
* @brief   xCat3 RD MTL 24G + 4SFP board PHY1680 SMI address init.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS xcat3Phy1680AddrInit_MTL24G4SFP
(
    IN GT_U8 devNum
)
{
    GT_STATUS   retVal;     /* return code */
    GT_U8       port;       /* port number */

    for (port = 0; port < 24;port++)
    {
        if( !PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, port) )
            continue;

        retVal = appDemoSetPortPhyAddr(devNum, port, phyAddr1680_binding_MTL_RD_24G_4SFP[port]);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}


/**
* @internal xcat3Phy1548AddrInit function
* @endinternal
*
* @brief   xCat3 PHY1548 SMI address init.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS xcat3Phy1548AddrInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS   retVal;             /* return code */
    GT_U8       portNum;            /* port number */
    GT_U8       phyAddr;            /* SMI address of PHY */

    /* port 0-23*/
    for(portNum = 0; portNum < 24; portNum++)
    {
        if( !PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portNum) )
            continue;

        phyAddr = (GT_U8)(4 + portNum%12);

        retVal = appDemoSetPortPhyAddr(devNum, portNum, phyAddr);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}


/**
* @internal xcat3LedInit function
* @endinternal
*
* @brief   xCat3 LED init.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS xcat3LedInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc; /* return code */
    GT_U32 ledInterfaceNum;    /* led stream interface number */
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_LED_CONF_STC               ledStreamConf;      /* led stream interface configuration structure */
    CPSS_LED_CLASS_MANIPULATION_STC ledClassParams;     /* led class manipulation configuration */
    CPSS_LED_GROUP_CONF_STC         ledGroupParams;     /* led group configuration parameters */

    for (ledInterfaceNum = 0; ledInterfaceNum < LED_MAX_NUM_OF_INTERFACE; ledInterfaceNum++)
    {
        /*Led stream configuration */
        ledStreamConf.ledOrganize = CPSS_LED_ORDER_MODE_BY_CLASS_E;
        ledStreamConf.disableOnLinkDown = GT_FALSE;
        ledStreamConf.blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
        ledStreamConf.blink0Duration  = CPSS_LED_BLINK_DURATION_4_E;
        ledStreamConf.blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
        ledStreamConf.blink1Duration  = CPSS_LED_BLINK_DURATION_4_E;
        ledStreamConf.pulseStretch  = CPSS_LED_PULSE_STRETCH_1_E;
        ledStreamConf.ledStart  = 0x0;
        ledStreamConf.ledEnd    = 0xFF;
        ledStreamConf.clkInvert = GT_TRUE;
        ledStreamConf.class5select  = CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
        ledStreamConf.class13select = CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;

        rc = cpssDxChLedStreamPortGroupConfigSet(devNum,
                                                 CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                 ledInterfaceNum, &ledStreamConf);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* XG group 0 configuration */
        if(PRV_CPSS_DXCH_XCAT3_A1_AND_ABOVE_CHECK_MAC(devNum) == GT_TRUE)
        {
            /* For xCAT3 A1 and above */
            ledGroupParams.classA = 0xA;
            ledGroupParams.classB = 0xB;
            ledGroupParams.classC = 0xF;
            ledGroupParams.classD = 0xF;
            rc = cpssDxChLedStreamPortGroupGroupConfigSet(devNum,
                                                          CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          ledInterfaceNum,
                                                          CPSS_DXCH_LED_PORT_TYPE_XG_E,
                                                          0, &ledGroupParams);
            if(GT_OK != rc)
            {
                return rc;
            }

            ledGroupParams.classA = 0xF;
            ledGroupParams.classB = 0xF;
            ledGroupParams.classC = 0xF;
            ledGroupParams.classD = 0xF;
            rc = cpssDxChLedStreamPortGroupGroupConfigSet(devNum,
                                                          CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          ledInterfaceNum,
                                                          CPSS_DXCH_LED_PORT_TYPE_XG_E,
                                                          1, &ledGroupParams);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
        else
        {
             /* For xCAT3 A0 */
            ledGroupParams.classA = 0x9;
            ledGroupParams.classB = 0xA;
            ledGroupParams.classC = 0x9;
            ledGroupParams.classD = 0xB;

            rc = cpssDxChLedStreamPortGroupGroupConfigSet(devNum,
                                                          CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                          ledInterfaceNum,
                                                          CPSS_DXCH_LED_PORT_TYPE_XG_E,
                                                          0, &ledGroupParams);
            if(GT_OK != rc)
            {
                return rc;
            }
       }


        /* GIG group 0 configuration */
        ledGroupParams.classA = 0x3;
        ledGroupParams.classB = 0x4;
        ledGroupParams.classC = 0x3;
        ledGroupParams.classD = 0x4;

        rc = cpssDxChLedStreamPortGroupGroupConfigSet(devNum,
                                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                      ledInterfaceNum,
                                                      CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                      0, &ledGroupParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* class 0 configuration */
        ledClassParams.invertEnable = GT_FALSE;
        ledClassParams.blinkEnable  = GT_TRUE;
        ledClassParams.blinkSelect  = 0;
        ledClassParams.forceEnable  = GT_FALSE;
        ledClassParams.forceData    = 0;

        rc = cpssDxChLedStreamPortGroupClassManipulationSet(devNum,
                                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            ledInterfaceNum,
                                                            CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                            4, &ledClassParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* class 10 configuration */
        ledClassParams.invertEnable = GT_TRUE;
        ledClassParams.blinkEnable  = GT_TRUE;
        ledClassParams.blinkSelect  = 0;
        ledClassParams.forceEnable  = GT_FALSE;
        ledClassParams.forceData    = 0;

        rc = cpssDxChLedStreamPortGroupClassManipulationSet(devNum,
                                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            ledInterfaceNum,
                                                            CPSS_DXCH_LED_PORT_TYPE_XG_E,
                                                            10, &ledClassParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* class 11 configuration */
        ledClassParams.invertEnable = GT_TRUE;
        ledClassParams.blinkEnable  = GT_TRUE;
        ledClassParams.blinkSelect  = 0;
        ledClassParams.forceEnable  = GT_FALSE;
        ledClassParams.forceData    = 0;

        rc = cpssDxChLedStreamPortGroupClassManipulationSet(devNum,
                                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            ledInterfaceNum,
                                                            CPSS_DXCH_LED_PORT_TYPE_XG_E,
                                                            11, &ledClassParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* For xCAT3 A1 and above */
        if(PRV_CPSS_DXCH_XCAT3_A1_AND_ABOVE_CHECK_MAC(devNum) == GT_TRUE)
        {
            /* Class 11 - RX Activity + TX Activity*/
            rc = cpssDxChLedStreamClassIndicationSet(devNum,
                                                     ledInterfaceNum,
                                                     11,
                                                     CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Class 10 - LINK */
            rc = cpssDxChLedStreamClassIndicationSet(devNum,
                                                     ledInterfaceNum,
                                                     10,
                                                     CPSS_DXCH_LED_INDICATION_LINK_E);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* class 10 (Link) configuration - no blinking, no inversion*/
            ledClassParams.invertEnable = GT_FALSE;
            ledClassParams.blinkEnable  = GT_FALSE;
            ledClassParams.blinkSelect  = 0;
            ledClassParams.forceEnable  = GT_FALSE;
            ledClassParams.forceData    = 0;

            rc = cpssDxChLedStreamPortGroupClassManipulationSet(devNum,
                                                                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                ledInterfaceNum,
                                                                CPSS_DXCH_LED_PORT_TYPE_XG_E,
                                                                10, &ledClassParams);

            if(GT_OK != rc)
            {
                return rc;
            }
        }
        /* For xCAT3 A0 */
        else
        {
             /* Class 9 - Link */
            rc = cpssDxChLedStreamClassIndicationSet(devNum,
                                                     ledInterfaceNum,
                                                     9,
                                                     CPSS_DXCH_LED_INDICATION_LINK_E);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Class 10 - RX Activity */
            rc = cpssDxChLedStreamClassIndicationSet(devNum,
                                                     ledInterfaceNum,
                                                     10,
                                                     CPSS_DXCH_LED_INDICATION_RX_ACT_E);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Class 11 - TX Activity */
            rc = cpssDxChLedStreamClassIndicationSet(devNum,
                                                     ledInterfaceNum,
                                                     11,
                                                     CPSS_DXCH_LED_INDICATION_TX_ACT_E);
            if(GT_OK != rc)
            {
                return rc;
            }
       }
    }

    for(portNum = 0; portNum < 28; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
        rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, portNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal configBoardAfterPhase2 function
* @endinternal
*
* @brief   This function performs all needed configurations that should be done
*         after phase2.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS configBoardAfterPhase2
(
    IN  GT_U8   boardRevId
)
{
    GT_U32  i;                                          /* loop index */
    GT_STATUS                       retVal;             /* return code */
    GT_U8                           portNum;            /* port number */
    GT_U32                          ledInterfaceNum;    /* led stream interface number */
    CPSS_LED_CONF_STC               ledStreamConf;      /* led stream interface configuration structure */
    CPSS_LED_GROUP_CONF_STC         ledGroupParams;     /* led group configuration parameters */
    CPSS_LED_CLASS_MANIPULATION_STC ledClassParams;     /* led class manipulation configuration */
    GT_U32                          halfDuplexLedGroup; /* led stream group used for HD indication */
    GT_U32                          fullDuplexLedGroup; /* led stream group used for FD indication */
    GT_U32                          fastSpeedLedGroup;  /* led stream group used for 10M/100M (fast) speed indication */
    GT_U32                          gigaSpeedLedGroup;  /* led stream group used for 1G (giga) speed indication */
    GT_BOOL                         feDevice = GT_FALSE; /* is device is FE one */
    GT_U8                           devNum;              /* device number */
    GT_U32                          trunkNum;            /* number of trunks in device */
    GT_U32                          cpuDisabled = 0;     /* status of internal CPU */

    if (ppCounter > 2)
    {
        return GT_FAIL;

    }

    for(i = SYSTEM_DEV_NUM_MAC(0); i < SYSTEM_DEV_NUM_MAC(ppCounter); ++i)
    {
        appDemoPpConfigList[i].internalCscdPortConfigFuncPtr = cascadePortConfig;

        devNum = appDemoPpConfigList[i].devNum;

        cpssOsPrintf(" configBoardAfterPhase2 i=%d devNum=%d\n", i,devNum);

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            cpssOsPrintf(" CPSS_PP_FAMILY_DXCH_XCAT3_E type=%d\n",xcat3BoardType);

            if(xcat3BoardType == APP_DEMO_AC3_BOARD_DB_CNS)
            {
                retVal = xcat3Phy1548AddrInit(devNum);
                if(retVal != GT_OK)
                {
                    return retVal;
                }

                retVal = xcat3LedInit(devNum);
                if(retVal != GT_OK)
                {
                    return retVal;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                /* APP_DEMO_AC3_BOARD_RD_MTL_TYPE1_CNS,
                   APP_DEMO_AC3_BOARD_RD_MTL_TYPE2_CNS,
                   APP_DEMO_AC3_BOARD_RD_MTL_TYPE3_CNS
                   APP_DEMO_AC3S_BOARD_RD_MTL_TYPE1_CNS*/
                if(xcat3BoardType == APP_DEMO_AC3S_BOARD_RD_MTL_TYPE1_CNS)
                {
                    retVal = rd_ac3s_LedInit(devNum);
                    if(retVal != GT_OK)
                    {
                        return retVal;
                    }
                }

                retVal = cpssDxChPhyAutoPollNumOfPortsSet(appDemoPpConfigList[i].devNum,
                                                          CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E,
                                                          CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E,
                                                          CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E,
                                                          CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E);
                if(retVal != GT_OK)
                {
                    return retVal;
                }

                if((xcat3BoardType == APP_DEMO_AC3_BOARD_RD_MTL_TYPE3_CNS) ||
                   (xcat3BoardType == APP_DEMO_AC3S_BOARD_RD_MTL_TYPE1_CNS))
                {
                    retVal = xcat3Phy1680AddrInit_MTL24G4SFP(devNum);
                }
                else
                {
                    if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P10)
                    {
                        cpssOsPrintf("devNum = %d\n", devNum);

                        retVal = xcat3Phy1680AddrInit_WNC8G2G2SFP(devNum);

                        retVal = cpssDxChPhyPortSmiInterfaceSet(devNum,24,CPSS_PHY_SMI_INTERFACE_0_E);
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", retVal);

                        retVal = cpssDxChPhyPortSmiInterfaceSet(devNum,25,CPSS_PHY_SMI_INTERFACE_0_E);
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", retVal);

                        retVal = appDemoSetPortPhyAddr(devNum,24,0x8);
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);

                        retVal = appDemoSetPortPhyAddr(devNum,25,0x9);
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);

                    }
                    else if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P24)
                    {
                        cpssOsPrintf("devNum = %d\n", devNum);

                        retVal = xcat3Phy1680AddrInit_WNCOS6360P24(devNum);

                        retVal = cpssDxChPhyPortSmiInterfaceSet(devNum,24,CPSS_PHY_XSMI_INTERFACE_0_E);
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", retVal);

                        retVal = appDemoSetPortPhyAddr(devNum,24,0xC);
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);

                        retVal = cpssDxChPhyPortSmiInterfaceSet(devNum,26,CPSS_PHY_XSMI_INTERFACE_0_E);
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", retVal);

                        retVal = appDemoSetPortPhyAddr(devNum,26,0xD);
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);
                    }
                    else if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P48)
                    {
                        cpssOsPrintf("devNum = %d\n", devNum);

                        retVal = xcat3Phy1680AddrInit_WNCOS6360P48(devNum);

                        if (devNum == 0)
                        {
                            retVal = cpssDxChPhyPortSmiInterfaceSet(devNum,24,CPSS_PHY_XSMI_INTERFACE_0_E);
                            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", retVal);

                            retVal = appDemoSetPortPhyAddr(devNum,24,0xA);
                            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);

                        }
                        else
                        {
                            retVal = cpssDxChPhyPortSmiInterfaceSet(devNum,27,CPSS_PHY_XSMI_INTERFACE_0_E);
                            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", retVal);

                            retVal = appDemoSetPortPhyAddr(devNum,27,0x9);
                            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);

                            retVal = cpssDxChPhyPortSmiInterfaceSet(devNum,24,CPSS_PHY_XSMI_INTERFACE_0_E);
                            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", retVal);

                            retVal = appDemoSetPortPhyAddr(devNum,24,0xC);
                            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);

                            retVal = cpssDxChPhyPortSmiInterfaceSet(devNum,25,CPSS_PHY_XSMI_INTERFACE_0_E);
                            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", retVal);

                            retVal = appDemoSetPortPhyAddr(devNum,25,0xD);
                            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);

                        }

                    }
                    else
                    {
                        retVal = xcat3Phy1680AddrInit(devNum);

                    }
                }

                if(retVal != GT_OK)
                {
                    return retVal;
                }
                else
                {
                    continue;
                }

            }
        }

/* Allen */
#if 0
        /* xCat devices */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
        {
            halfDuplexLedGroup = 2;
            fullDuplexLedGroup = 3;
            fastSpeedLedGroup = 0;
            gigaSpeedLedGroup = 1;
        }
        else /* xCat2 devices */
        {
            halfDuplexLedGroup = 3;
            fullDuplexLedGroup = 2;
            fastSpeedLedGroup = 1;
            gigaSpeedLedGroup = 0;
        }

        for( ledInterfaceNum = 0 ; ledInterfaceNum < LED_MAX_NUM_OF_INTERFACE ; ledInterfaceNum++ )
        {
            /*Led stream configuration */
            ledStreamConf.ledOrganize = CPSS_LED_ORDER_MODE_BY_PORT_E;
            ledStreamConf.disableOnLinkDown = GT_TRUE;
            ledStreamConf.blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
            ledStreamConf.blink0Duration  = CPSS_LED_BLINK_DURATION_6_E;
            ledStreamConf.blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_2_E;
            ledStreamConf.blink1Duration  = CPSS_LED_BLINK_DURATION_3_E;
            ledStreamConf.pulseStretch  = CPSS_LED_PULSE_STRETCH_3_E;
            ledStreamConf.ledStart  = 0x0;
            ledStreamConf.ledEnd    = 0xFF;
            ledStreamConf.clkInvert = GT_TRUE;
            ledStreamConf.class5select  = CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
            ledStreamConf.class13select = CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;

            retVal = cpssDxChLedStreamPortGroupConfigSet(appDemoPpConfigList[i].devNum,
                                                         CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                         ledInterfaceNum, &ledStreamConf);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortGroupConfigSet", retVal);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            /* group 0 configuration */
            ledGroupParams.classA = 0x4;
            ledGroupParams.classB = 0xB;
            ledGroupParams.classC = 0x4;
            ledGroupParams.classD = 0x1;

            retVal = cpssDxChLedStreamPortGroupGroupConfigSet(appDemoPpConfigList[i].devNum,
                                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                              ledInterfaceNum,
                                                              CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                              fastSpeedLedGroup,
                                                              &ledGroupParams);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortGroupGroupConfigSet", retVal);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            /* group 1 configuration */
            ledGroupParams.classA = 0x0;
            ledGroupParams.classB = 0x4;
            ledGroupParams.classC = 0x0;
            ledGroupParams.classD = 0x4;

            retVal = cpssDxChLedStreamPortGroupGroupConfigSet(appDemoPpConfigList[i].devNum,
                                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                              ledInterfaceNum,
                                                              CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                              gigaSpeedLedGroup,
                                                              &ledGroupParams);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortGroupGroupConfigSet", retVal);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            /* The board CPLD is using group 2 for full duplex indication and group 3 for half duplex */
            /* inducation to create the green\yellow light effect */
            /* group 2 configuration */
            ledGroupParams.classA = 0x2;
            ledGroupParams.classB = 0x2;
            ledGroupParams.classC = 0x2;
            ledGroupParams.classD = 0x2;

            retVal = cpssDxChLedStreamPortGroupGroupConfigSet(appDemoPpConfigList[i].devNum,
                                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                              ledInterfaceNum,
                                                              CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                              halfDuplexLedGroup,
                                                              &ledGroupParams);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortGroupGroupConfigSet", retVal);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            /* group 3 configuration */
            ledGroupParams.classA = 0x5;
            ledGroupParams.classB = 0x5;
            ledGroupParams.classC = 0x5;
            ledGroupParams.classD = 0x5;

            retVal = cpssDxChLedStreamPortGroupGroupConfigSet(appDemoPpConfigList[i].devNum,
                                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                              ledInterfaceNum,
                                                              CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                              fullDuplexLedGroup,
                                                              &ledGroupParams);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortGroupGroupConfigSet", retVal);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            /* class 2 configuration */
            ledClassParams.invertEnable = GT_FALSE;
            ledClassParams.blinkEnable  = GT_FALSE;
            ledClassParams.blinkSelect  = 0;
            ledClassParams.forceEnable  = GT_FALSE;
            ledClassParams.forceData    = 0;

            retVal = cpssDxChLedStreamPortGroupClassManipulationSet(appDemoPpConfigList[i].devNum,
                                                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                    ledInterfaceNum,
                                                                    CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                                    2, &ledClassParams);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortGroupClassManipulationSet", retVal);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            /* class 3 configuration */
            ledClassParams.invertEnable = GT_FALSE;
            ledClassParams.blinkEnable  = GT_FALSE;
            ledClassParams.blinkSelect  = 0;
            ledClassParams.forceEnable  = GT_FALSE;
            ledClassParams.forceData    = 0;

            retVal = cpssDxChLedStreamPortGroupClassManipulationSet(appDemoPpConfigList[i].devNum,
                                                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                    ledInterfaceNum,
                                                                    CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                                    3, &ledClassParams);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortGroupClassManipulationSet", retVal);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            /* class 4 configuration */
            ledClassParams.invertEnable = GT_TRUE;
            ledClassParams.blinkEnable  = GT_TRUE;
            ledClassParams.blinkSelect  = 1;
            ledClassParams.forceEnable  = GT_FALSE;
            ledClassParams.forceData    = 0;

            retVal = cpssDxChLedStreamPortGroupClassManipulationSet(appDemoPpConfigList[i].devNum,
                                                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                    ledInterfaceNum,
                                                                    CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                                    4, &ledClassParams);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortGroupClassManipulationSet", retVal);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            /* class 5 configuration */
            ledClassParams.invertEnable = GT_FALSE;
            ledClassParams.blinkEnable  = GT_FALSE;
            ledClassParams.blinkSelect  = 0;
            ledClassParams.forceEnable  = GT_FALSE;
            ledClassParams.forceData    = 0;

            retVal = cpssDxChLedStreamPortGroupClassManipulationSet(appDemoPpConfigList[i].devNum,
                                                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                    ledInterfaceNum,
                                                                    CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                                    5, &ledClassParams);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortGroupClassManipulationSet", retVal);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            /* class 6 configuration */
            ledClassParams.invertEnable = GT_FALSE;
            ledClassParams.blinkEnable  = GT_FALSE;
            ledClassParams.blinkSelect  = 0;
            ledClassParams.forceEnable  = GT_TRUE;
            ledClassParams.forceData    = 0;

            retVal = cpssDxChLedStreamPortGroupClassManipulationSet(appDemoPpConfigList[i].devNum,
                                                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                                    ledInterfaceNum,
                                                                    CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                                    6, &ledClassParams);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortGroupClassManipulationSet", retVal);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            /* debug configuration for class 10 for FLEX ports - "link" instead of "spanning tree" */
            retVal = cpssDxChLedStreamClassIndicationSet(appDemoPpConfigList[i].devNum,
                                                         ledInterfaceNum, 10,
                                                         CPSS_DXCH_LED_INDICATION_LINK_E);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamClassIndicationSet", retVal);
            if (retVal != GT_OK)
            {
                return retVal;
            }
        }

        /* Configure phy addresses */
        if (feDevice == GT_TRUE)
        {
            /* port 0-23*/
            for(portNum = 0; portNum < 24; portNum++)
            {
                if( !PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portNum) )
                    continue;

                retVal = appDemoSetPortPhyAddr(appDemoPpConfigList[i].devNum,portNum,
                                               (GT_U8)(portNum%16));
                CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);
                if(retVal != GT_OK)
                {
                    return retVal;
                }
            }
        }
        if (boardRevId == 5)
        {
            retVal = cpssDxChPhyPortSmiInterfaceSet(devNum,24,CPSS_PHY_SMI_INTERFACE_1_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", retVal);
            if(retVal != GT_OK)
            {
                return retVal;
            }

            retVal = cpssDxChPhyPortSmiInterfaceSet(devNum,25,CPSS_PHY_SMI_INTERFACE_1_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", retVal);
            if(retVal != GT_OK)
            {
                return retVal;
            }

            retVal = appDemoSetPortPhyAddr(appDemoPpConfigList[i].devNum,24,0x10);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);
            if(retVal != GT_OK)
            {
                return retVal;
            }

            retVal = appDemoSetPortPhyAddr(appDemoPpConfigList[i].devNum,25,0x11);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSetPortPhyAddr", retVal);
            if(retVal != GT_OK)
            {
                return retVal;
            }
        }
#endif
    }

    if (ppCounter > 1)
    {/* ppCounter == 2 */

        for(i = SYSTEM_DEV_NUM_MAC(0); i < SYSTEM_DEV_NUM_MAC(ppCounter); ++i)
        {
            /* Cascading */
            devNum = appDemoPpConfigList[i].devNum;

            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                /*
                    AC3 B2B (48G+4XG) cascade board configuration:
                    -------------------------------------------------------------
                        98DX3336                    |   98DX3236
                            devId = 0               |       devId = 1
                            internal CPU enabled    |       internal CPU disabled
                    -------------------------------------------------------------
                                L7                  |            L11
                    --- MAC_24(20G-KR) -------------------------------- MAC27 ---
                                L6                  |            L10
                    -------------------------------------------------------------
                                L8                  |            L8
                    --- MAC_26(20G-KR) -------------------------------- MAC26 ---
                                L9                  |            L9
                    -------------------------------------------------------------
                */

                /*
                    AC3 B2B (48G+2XG+2XXG) cascade board configuration:
                    -------------------------------------------------------------
                        98DX3336                    |   98DX3236
                            devId = 0               |       devId = 1
                            internal CPU enabled    |       internal CPU disabled
                    -------------------------------------------------------------
                                L7                  |            L11
                    --- MAC_24(20G-KR) -------------------------------- MAC27 ---
                                L6                  |            L10
                    -------------------------------------------------------------
                                L11                 |            L7
                    --- MAC_27(10G_KR)--------------------------------- MAC25 ---
                                                    |
                    -------------------------------------------------------------
                */

                /* get number of trunks in device. */
                retVal = cpssDxChCfgTableNumEntriesGet(appDemoPpConfigList[i].devNum,
                                                       CPSS_DXCH_CFG_TABLE_TRUNK_E,
                                                       &trunkNum);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCfgTableNumEntriesGet", retVal);
                if (retVal != GT_OK)
                {
                    return retVal;
                }

                appDemoPpConfigList[i].numberOfCscdTrunks = 1;
                appDemoPpConfigList[i].numberOfCscdPorts = 1;

                /* 0 - CPU enabled */
                /* 1 - CPU disabled */
                cpuDisabled = i;

                if (cpuDisabled == GT_TRUE)
                {
                    if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P10)
                    {
                        /* device CPSS_98DX3336_CNS with CPU disabled */
                        appDemoPpConfigList[i].cscdPortsArr[0].portNum = 27;
                        xcat3CscdPortDb[i][0].portNum = appDemoPpConfigList[i].cscdPortsArr[0].portNum;
                        xcat3CscdPortDb[i][0].ifMode = CPSS_PORT_INTERFACE_MODE_KR2_E;
                        xcat3CscdPortDb[i][0].isExtendedMac = GT_TRUE;
                        xcat3CscdPortDb[i][0].lanesNum = 2;
                        xcat3CscdPortDb[i][0].speed = CPSS_PORT_SPEED_20000_E;
                        xcat3CscdPortDb[i][0].rxLanesPolarity[0] = GT_FALSE;
                        xcat3CscdPortDb[i][0].txLanesPolarity[0] = GT_FALSE;
                        xcat3CscdPortDb[i][0].rxLanesPolarity[1] = GT_FALSE;
                        xcat3CscdPortDb[i][0].txLanesPolarity[1] = GT_FALSE;
                    } /* End of APP_DEMO_AC3BXH_WNC_OS6360P10 */
                    else if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P48)
                    {
                        /* device CPSS_98DX3336_CNS with CPU disabled */
                        appDemoPpConfigList[i].cscdPortsArr[0].portNum = 26;
                        xcat3CscdPortDb[i][0].portNum = appDemoPpConfigList[i].cscdPortsArr[0].portNum;
                        xcat3CscdPortDb[i][0].ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                        xcat3CscdPortDb[i][0].isExtendedMac = GT_FALSE;
                        xcat3CscdPortDb[i][0].lanesNum = 2;
                        xcat3CscdPortDb[i][0].speed = CPSS_PORT_SPEED_20000_E;
                        xcat3CscdPortDb[i][0].rxLanesPolarity[0] = GT_FALSE;
                        xcat3CscdPortDb[i][0].txLanesPolarity[0] = GT_FALSE;
                        xcat3CscdPortDb[i][0].rxLanesPolarity[1] = GT_FALSE;
                        xcat3CscdPortDb[i][0].txLanesPolarity[1] = GT_FALSE;
                    } /* End of APP_DEMO_AC3BXH_WNC_OS6360P48 */
                    else
                    {
                        return GT_BAD_STATE;
                    }
                }
                else
                {
                    if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P10)
                    {
                        /* device CPSS_98DX3336_CNS with CPU enabled */
                        appDemoPpConfigList[i].cscdPortsArr[0].portNum = 24;
                        xcat3CscdPortDb[i][0].portNum = appDemoPpConfigList[i].cscdPortsArr[0].portNum;
                        xcat3CscdPortDb[i][0].ifMode = CPSS_PORT_INTERFACE_MODE_KR2_E;
                        xcat3CscdPortDb[i][0].isExtendedMac = GT_FALSE;
                        xcat3CscdPortDb[i][0].lanesNum = 2;
                        xcat3CscdPortDb[i][0].speed = CPSS_PORT_SPEED_20000_E;
                        xcat3CscdPortDb[i][0].rxLanesPolarity[0] = GT_FALSE;
                        xcat3CscdPortDb[i][0].txLanesPolarity[0] = GT_FALSE;
                        xcat3CscdPortDb[i][0].rxLanesPolarity[1] = GT_FALSE;
                        xcat3CscdPortDb[i][0].txLanesPolarity[1] = GT_TRUE;
                    } /* End of APP_DEMO_AC3BXH_WNC_OS6360P10 */
                    else if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P48)
                    {
                        /* device CPSS_98DX3336_CNS with CPU enabled */
                        appDemoPpConfigList[i].cscdPortsArr[0].portNum = 26;
                        xcat3CscdPortDb[i][0].portNum = appDemoPpConfigList[i].cscdPortsArr[0].portNum;
                        xcat3CscdPortDb[i][0].ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                        xcat3CscdPortDb[i][0].isExtendedMac = GT_FALSE;
                        xcat3CscdPortDb[i][0].lanesNum = 2;
                        xcat3CscdPortDb[i][0].speed = CPSS_PORT_SPEED_20000_E;
                        xcat3CscdPortDb[i][0].rxLanesPolarity[0] = GT_FALSE;
                        xcat3CscdPortDb[i][0].txLanesPolarity[0] = GT_TRUE;
                        xcat3CscdPortDb[i][0].rxLanesPolarity[1] = GT_TRUE;
                        xcat3CscdPortDb[i][0].txLanesPolarity[1] = GT_FALSE;
                    } /* End of APP_DEMO_AC3BXH_WNC_OS6360P48 */
                    else
                    {
                        return GT_BAD_STATE;
                    }
                }

                appDemoPpConfigList[i].cscdPortsArr[0].cscdPortType = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E;
                appDemoPpConfigList[i].cscdPortsArr[0].trunkId = (GT_TRUNK_ID)trunkNum;
                appDemoPpConfigList[i].numberOfCscdTargetDevs = 1;
                appDemoPpConfigList[i].cscdTargetDevsArr[0].targetDevNum = (GT_U8)((0 == i) ? 1 : 0);
                appDemoPpConfigList[i].cscdTargetDevsArr[0].linkToTargetDev.linkNum = trunkNum;
                appDemoPpConfigList[i].cscdTargetDevsArr[0].linkToTargetDev.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
                appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].internal10GPortConfigFuncPtr = NULL;

            } /* End of CPSS_PP_FAMILY_DXCH_XCAT3_E */
            else
            {
                if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
                {
                    /* retrieve CPU enabled from Sampled at Reset - CPU_INT_EXT */
                    retVal = prvCpssHwPpGetRegField(devNum,0x0000002C,14,1,&cpuDisabled);
                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }

                    /* WA - on xCat2 both CPUs enabled */
                    cpuDisabled = devNum;
                }
                else
                {
                    cpuDisabled = appDemoPpConfigList[i].deviceId & 0x20000;

                }

                if (cpuDisabled)
                {
                    portNum = 24;
                }
                else
                {
                    portNum = 26;
                }

                /* get number of trunks in device. */
                retVal = cpssDxChCfgTableNumEntriesGet(appDemoPpConfigList[i].devNum,
                                                       CPSS_DXCH_CFG_TABLE_TRUNK_E,
                                                       &trunkNum);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCfgTableNumEntriesGet", retVal);
                if (retVal != GT_OK)
                {
                    return retVal;
                }

                appDemoPpConfigList[i].numberOfCscdTrunks = 1;
                appDemoPpConfigList[i].numberOfCscdPorts = 2;

                appDemoPpConfigList[i].cscdPortsArr[0].portNum = portNum;
                appDemoPpConfigList[i].cscdPortsArr[0].cscdPortType = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E;
                appDemoPpConfigList[i].cscdPortsArr[0].trunkId = (GT_TRUNK_ID)trunkNum;

                appDemoPpConfigList[i].cscdPortsArr[1].portNum = (GT_U8)(portNum + 1);
                appDemoPpConfigList[i].cscdPortsArr[1].cscdPortType = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E;
                appDemoPpConfigList[i].cscdPortsArr[1].trunkId = (GT_TRUNK_ID)trunkNum;

                appDemoPpConfigList[i].numberOfCscdTargetDevs = 1;

                appDemoPpConfigList[i].cscdTargetDevsArr[0].targetDevNum = (GT_U8)((0 == i) ? 1 : 0);
                appDemoPpConfigList[i].cscdTargetDevsArr[0].linkToTargetDev.linkNum = trunkNum;
                appDemoPpConfigList[i].cscdTargetDevsArr[0].linkToTargetDev.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;

                /* SFP ports - skip this for xCat2 */
                if(PRV_CPSS_PP_MAC(0)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E)
                {
                    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].internal10GPortConfigFuncPtr = internalB2bXGPortConfig;
                    portNum = (GT_U8)((portNum == 24) ? 26 : 24);
                    appDemoPpConfigList[i].numberOf10GPortsToConfigure = 2;
                    appDemoPpConfigList[i].ports10GToConfigureArr[0] = portNum;
                    appDemoPpConfigList[i].ports10GToConfigureArr[1] = portNum + 1;
                }
                else /* xCat2 */
                {
                    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].internal10GPortConfigFuncPtr = NULL;
                }

                /* Set CPU code table designated device on device not connected to CPU by RGMII*/
                if (((waSdmaPktsFromCpuEnabled == GT_TRUE) || boardRevId == 8) &&
                    (appDemoPpConfigList[i].devNum != devNumWithCpu))
                {
                    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;
                    retVal = cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(appDemoPpConfigList[i].devNum,
                                                 APP_DEMO_INDEX_INTO_DESIGNATED_DEV_TABLE,
                                                 PRV_CPSS_HW_DEV_NUM_MAC(devNumWithCpu));

                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChNetIfCpuCodeDesignatedDeviceTableSet", retVal);
                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }

                    /* get default entry */
                    retVal = cpssDxChNetIfCpuCodeTableGet(appDemoPpConfigList[i].devNum,
                                                      CPSS_NET_CONTROL_E,&entryInfo);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChNetIfCpuCodeTableGet", retVal);
                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }
                    entryInfo.designatedDevNumIndex = APP_DEMO_INDEX_INTO_DESIGNATED_DEV_TABLE;

                    /* set designatedDevNumIndex for all CPU codes */
                    retVal = cpssDxChNetIfCpuCodeTableSet(appDemoPpConfigList[i].devNum,
                                                      CPSS_NET_ALL_CPU_OPCODES_E,&entryInfo);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChNetIfCpuCodeTableSet", retVal);
                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }
                }
            }
        }
    }

    /* PHY FW download */
    if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P24 || xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P48)
    {
        /* PHY 88X3220 for P24, PHY 88E2010 for P48*/
        appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].internal10GPortConfigFuncPtr = internalXGPortConfig;
        appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].numberOf10GPortsToConfigure = 2;
        appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].ports10GToConfigureArr[0] = 24;
        appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].ports10GToConfigureArr[1] = 26;

        retVal = gtAppDemoXPhyFwDownload(0);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("gtAppDemoXPhyFwDownload", retVal);
        if (retVal != GT_OK)
        {
            return retVal;
        }
    }

    if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P48)
    {
        /* PHY 88X3220 */
        appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(1)].internal10GPortConfigFuncPtr = internalXGPortConfig;
        appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(1)].numberOf10GPortsToConfigure = 2;
        appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(1)].ports10GToConfigureArr[0] = 24;
        appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(1)].ports10GToConfigureArr[1] = 25;

        retVal = gtAppDemoXPhyFwDownload(1);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("gtAppDemoXPhyFwDownload", retVal);
        if (retVal != GT_OK)
        {
            return retVal;
        }
    }

#ifdef DRAGONITE_TYPE_A1
    retVal = prvCpssDragoniteInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDragoniteInit", retVal);
    return retVal;
#else
    return GT_OK;
#endif
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
    CPSS_PP_CONFIG_INIT_STC  localPpCfgParams = PP_LOGICAL_CONFIG_DEFAULT;
    GT_STATUS                rc;
    GT_U32                   value;

    localPpCfgParams.maxNumOfIpv4Prefixes = 3920;
    localPpCfgParams.numOfTrunks = APP_DEMO_127_TRUNKS_CNS;
    localPpCfgParams.maxNumOfVirtualRouters = 1;
    localPpCfgParams.lpmDbFirstTcamLine = 100;
    localPpCfgParams.lpmDbLastTcamLine = 1204;

    rc = cpssDxChCfgTableNumEntriesGet(devNum, CPSS_DXCH_CFG_TABLE_ROUTER_TCAM_E, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (value)
    {
        case 128:
            /* AlleyCat3 devices with 128 router TCAM lines */
            localPpCfgParams.maxNumOfIpv4Prefixes = 242;
            localPpCfgParams.maxNumOfIpv4McEntries = 20;
            localPpCfgParams.maxNumOfIpv6Prefixes = 35;
            localPpCfgParams.maxNumOfIpv6McGroups = 5;
            localPpCfgParams.lpmDbFirstTcamLine = 27;
            localPpCfgParams.lpmDbLastTcamLine = 127;
            break;

        case 256:
            /* AlleyCat3 devices with 256 router TCAM lines */
            localPpCfgParams.maxNumOfIpv4Prefixes = 484;
            localPpCfgParams.maxNumOfIpv4McEntries = 40;
            localPpCfgParams.maxNumOfIpv6Prefixes = 70;
            localPpCfgParams.maxNumOfIpv6McGroups = 10;
            localPpCfgParams.lpmDbFirstTcamLine = 55;
            localPpCfgParams.lpmDbLastTcamLine = 255;
            break;

        default:
            break;
    }

    /* if boardRevId == 4 or boardRevId == 6 or it's xCat2
       then device is configured to use policy based routing */
    if ((boardRevId == 4) || (boardRevId == 6) || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
    {
        localPpCfgParams.maxNumOfIpv4Prefixes = 400;
        localPpCfgParams.maxNumOfIpv6Prefixes = 100;
        localPpCfgParams.usePolicyBasedRouting = GT_TRUE;
        localPpCfgParams.routingMode = CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E;

        /* The appDemoDb string "usePolicyBasedRouting" is used by the enhanced
           ut in order to detect whether the system is configured to work in
           policy based routing or not. In case of policy based routing, add this
           string (unless the application explicitly added it). */
        if (appDemoDbEntryGet("usePolicyBasedRouting", &value) != GT_OK)
        {
            rc = appDemoDbEntryAdd("usePolicyBasedRouting", GT_TRUE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    if (boardRevId == 8)
    {
        localPpCfgParams.miiNumOfTxDesc = TX_DESC_NUM_DEF;
        localPpCfgParams.miiTxInternalBufBlockSize = (TX_DESC_NUM_DEF / 2) *
            CPSS_GEN_NETIF_MII_TX_INTERNAL_BUFF_SIZE_CNS;
        localPpCfgParams.miiRxBufSize = RX_BUFF_SIZE_DEF;
        localPpCfgParams.miiRxBufBlockSize = RX_DESC_NUM_DEF * RX_BUFF_SIZE_DEF;
    }

    if((boardRevId == 1) && (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
    {
        localPpCfgParams.miiNumOfTxDesc = TX_DESC_NUM_DEF;
        localPpCfgParams.miiTxInternalBufBlockSize = (TX_DESC_NUM_DEF / 2) *
            CPSS_GEN_NETIF_MII_TX_INTERNAL_BUFF_SIZE_CNS;
        localPpCfgParams.miiRxBufSize = RX_BUFF_SIZE_DEF;
        localPpCfgParams.miiRxBufBlockSize = RX_DESC_NUM_DEF * RX_BUFF_SIZE_DEF;
    }


    osMemCpy(ppLogInitParams,&localPpCfgParams,sizeof(CPSS_PP_CONFIG_INIT_STC));
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

    localLibInitParams.initClassifier            = GT_FALSE;
    localLibInitParams.initIpv6                  = GT_TRUE;
    localLibInitParams.initIpv4                  = GT_TRUE;

    localLibInitParams.initIpv4Filter            = GT_FALSE;
    localLibInitParams.initIpv4Tunnel            = GT_FALSE;
    localLibInitParams.initMpls                  = GT_FALSE;
    localLibInitParams.initMplsTunnel            = GT_FALSE;
    localLibInitParams.initPcl                   = GT_TRUE;

    /* there is no Policer lib init for CH3 devices */
    localLibInitParams.initPolicer               = GT_FALSE;

    osMemCpy(libInitParams,&localLibInitParams,
             sizeof(APP_DEMO_LIB_INIT_PARAMS));
    return GT_OK;
}

/**
* @internal cpuTx function
* @endinternal
*
* @brief   Local function used by workaround for SGMII 2.5GB to send packet from CPU
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_RESOURCE           - memory resource not available.
*/
static GT_STATUS cpuTx(GT_U8 port, GT_BOOL sync)
{
    CPSS_DXCH_NET_TX_PARAMS_STC pcktParams;
    GT_U8                       *buffList[1];
    GT_U32                      buffLenList[1];
    GT_U32                      numOfBufs;
    GT_STATUS                   rc;

    osMemSet(&pcktParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
    pcktParams.sdmaInfo.txQueue = 0;
    pcktParams.sdmaInfo.recalcCrc = 1;
    pcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;
    pcktParams.packetIsTagged = GT_FALSE;
    pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
    /* use here HW device number! */
    pcktParams.dsaParam.dsaInfo.fromCpu.srcHwDev = PRV_CPSS_HW_DEV_NUM_MAC(0);
    pcktParams.dsaParam.dsaInfo.fromCpu.cascadeControl = GT_FALSE;
    pcktParams.dsaParam.dsaInfo.fromCpu.tc = 0;
    pcktParams.dsaParam.dsaInfo.fromCpu.dp = 0;
    pcktParams.dsaParam.dsaInfo.fromCpu.egrFilterEn = GT_FALSE;
    pcktParams.dsaParam.dsaInfo.fromCpu.egrFilterRegistered = GT_FALSE;
    pcktParams.dsaParam.dsaInfo.fromCpu.srcId = 0;
    pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
    pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(0);
    pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = port;
    pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.vlanId = 1;
    pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged = GT_FALSE;
    pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU = GT_FALSE;
    pcktParams.dsaParam.commonParams.cfiBit = 0;
    pcktParams.dsaParam.commonParams.vid = 1;
    pcktParams.dsaParam.commonParams.vpt = 0;
    pcktParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;
    pcktParams.dsaParam.commonParams.dropOnSource = GT_FALSE;
    pcktParams.dsaParam.commonParams.packetIsLooped = GT_FALSE;

    numOfBufs = 1;
    buffLenList[0] = 64;
    buffList[0] = osMalloc(buffLenList[0]*sizeof(GT_U8));
    buffList[0][0] = 0;
    buffList[0][1] = 0x0;
    buffList[0][2] = 0x22;
    buffList[0][3] = 0x22;
    buffList[0][4] = 0x22;
    buffList[0][5] = 0x22;
    buffList[0][6] = 0;
    buffList[0][7] = 0x0;
    buffList[0][8] = 0x0;
    buffList[0][9] = 0x0;
    buffList[0][10] = 0x0;
    buffList[0][11] = 0x2;
    rc = cpssDxChNetIfSdmaTxQueueEnable(0, 0, GT_TRUE);

    if(sync == GT_TRUE)
    {
        rc = cpssDxChNetIfSdmaSyncTxPacketSend(0,&pcktParams,buffList, buffLenList, numOfBufs);
    }
    else
    {
        GT_UINTPTR                  txEventHndl;
        CPSS_UNI_EV_CAUSE_ENT       uniEventArr[1];
        GT_U32                      evBitmapArr[8];

        uniEventArr[0] = CPSS_PP_TX_BUFFER_QUEUE_E;
        cpssEventBind(uniEventArr, 1, &txEventHndl);

        pcktParams.sdmaInfo.evReqHndl = txEventHndl;

        rc = cpssDxChNetIfSdmaTxPacketSend(0,&pcktParams,buffList, buffLenList, numOfBufs);
        if(rc == GT_OK)
        {
            rc = cpssEventSelect(txEventHndl, NULL, evBitmapArr, 8);
        }
        cpssEventUnBind(uniEventArr, 1);
        cpssEventDestroy(txEventHndl);
    }

    osFree(buffList[0]);

    return rc;
}

/**
* @internal appDemoXcat3XlgUnidirectionalPortAdd function
* @endinternal
*
* @brief   Add port to list of port capatible to G Unidirectional issue.
*
* @param[in] dev      - device number
* @param[in] port     - port number
*
* @retval GT_OK       - on success,
* @retval GT_FAIL     - otherwise.
*/
GT_STATUS appDemoXcat3XlgUnidirectionalPortAdd
(
    IN  GT_U8  dev,
    IN  GT_U32 port
)
{
    GT_STATUS                    rc;
    CPSS_DXCH_IMPLEMENT_WA_ENT   waArr[1];
    GT_U32                       waInfoArr[1];

    waArr[0] = CPSS_DXCH_IMPLEMENT_WA_XCAT3_XLG_PORT_BUFFER_STUCK_UNIDIRECTIONAL_E;
    waInfoArr[0] = port;
    rc = cpssDxChHwPpImplementWaInit(dev, 1, &(waArr[0]), &(waInfoArr[0]));
    cpssOsPrintf("cpssDxChHwPpImplementWaInit error", rc);
    return rc;
}

/**
* @internal xcat3PortInterfaceInit function
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
static GT_STATUS xcat3PortInterfaceInit
(
    IN  GT_U8 dev,
    IN  GT_U8 boardRevId
)
{
    GT_STATUS               rc;                 /* return code */
    GT_U32                  initSerdesDefaults; /* should appDemo configure ports modes */
    CPSS_PORTS_BMP_STC      initPortsBmp;       /* bitmap of ports to init */
    GT_PHYSICAL_PORT_NUM    portNum;            /* port number */
    CPSS_DXCH_IMPLEMENT_WA_ENT   waArr[1];
    GT_U32                       waInfoArr[1];
    GT_U32                  noXlgUnidirectional;
    GT_U8                   total_port;

/* Allen */
#if 0
    noXlgUnidirectional = 0;
    appDemoDbEntryGet("noXlgUnidirectional", &noXlgUnidirectional);

    if (noXlgUnidirectional == 0)
    {
        for (portNum = 24; portNum < 28; portNum++)
        {
            waArr[0] = CPSS_DXCH_IMPLEMENT_WA_XCAT3_XLG_PORT_BUFFER_STUCK_UNIDIRECTIONAL_E;
            waInfoArr[0] = portNum;
            rc = cpssDxChHwPpImplementWaInit(dev, 1, &(waArr[0]), &(waInfoArr[0]));
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    rc = prvAppDemoXcat3sSerdesPolarityConfigSet(dev);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("prvAppDemoAc3sSerdesPolarityConfigSet", rc);
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

#endif


    /* QSGMII port 0-23 configuration */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);

    if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P10)
    {
        for (portNum = 0 ; portNum < 8; portNum++)
        {
            CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,portNum);
        }
    }
    else if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P24)
    {
        for (portNum = 0 ; portNum < 24; portNum++)
        {
            CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,portNum);
        }
    }
    else if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P48)
    {
        if(dev == 0)
            total_port = 24;
        else
            total_port = 22;

        for (portNum = 0 ; portNum < total_port; portNum++)
        {
            CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,portNum);
        }
    }
    else
    {
        for (portNum = 0 ; portNum < 24; portNum++)
        {
            CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,portNum);
        }
    }

    rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_QSGMII_E, CPSS_PORT_SPEED_1000_E);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
    if(GT_OK != rc)
    {
        return rc;
    }

    if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P24)
    {
        cpssDxChPortSerdesPolaritySet(0,  0, 1, GT_FALSE, GT_FALSE);
        cpssDxChPortSerdesPolaritySet(0,  4, 1, GT_FALSE, GT_FALSE);
        cpssDxChPortSerdesPolaritySet(0,  8, 1,  GT_TRUE, GT_FALSE);
        cpssDxChPortSerdesPolaritySet(0, 12, 1, GT_FALSE, GT_TRUE);
        cpssDxChPortSerdesPolaritySet(0, 16, 1, GT_FALSE, GT_TRUE);
        cpssDxChPortSerdesPolaritySet(0, 20, 1, GT_FALSE, GT_TRUE);
    }

    if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P48)
    {
        if(dev == 0)
        {   cpssDxChPortSerdesPolaritySet(0,  0, 1, GT_FALSE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(0,  4, 1, GT_FALSE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(0,  8, 1,  GT_TRUE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(0, 12, 1, GT_FALSE, GT_TRUE);
            cpssDxChPortSerdesPolaritySet(0, 16, 1, GT_FALSE, GT_TRUE);
            cpssDxChPortSerdesPolaritySet(0, 20, 1, GT_FALSE, GT_TRUE);
        }
        else
        {   cpssDxChPortSerdesPolaritySet(1,  0, 1, GT_FALSE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(1,  4, 1,  GT_TRUE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(1,  8, 1,  GT_TRUE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(1, 12, 1, GT_FALSE, GT_TRUE);
            cpssDxChPortSerdesPolaritySet(1, 16, 1, GT_FALSE, GT_TRUE);
            cpssDxChPortSerdesPolaritySet(1, 20, 1, GT_FALSE, GT_TRUE);
        }
    }

    if(xcat3BoardType == APP_DEMO_AC3_BOARD_DB_CNS)
    {
        /* 10GBASE-KR port 24-27 configuration */
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
        for (portNum = 24 ; portNum < 28; portNum++)
        {
            CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,portNum);
        }
        if (!CPSS_PORTS_BMP_IS_ZERO_MAC(&initPortsBmp))
        {
            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else if(xcat3BoardType == APP_DEMO_AC3_BOARD_RD_MTL_TYPE1_CNS)
    {
        /*device with CPU*/
        if(dev == 0)
        {
            rc = cpssDxChPortExtendedModeEnableSet(dev, 25, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortExtendedModeEnableSet", rc);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssDxChPortExtendedModeEnableSet(dev, 27, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortExtendedModeEnableSet", rc);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* 10GBASE-KR EXTENDED port 25+27 configuration */
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,25);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,27);
            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* DEV_0, PORT_27 - enable RX polarity swap */
            rc = cpssDxChPortSerdesPolaritySet(dev, 27, 1, GT_FALSE, GT_TRUE);
            if (rc != GT_OK)
            {
                return rc;
            }

        }
        else
        {
            rc = cpssDxChPortExtendedModeEnableSet(dev, 25, GT_FALSE);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* 10GBASE-KR port 24 configuration */
            /* 10GBASE-KR EXTENDED port 25 configuration */
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,24);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,25);
            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

    }
    else if(xcat3BoardType == APP_DEMO_AC3_BOARD_RD_MTL_TYPE2_CNS)
    {
        /*device with CPU*/
        if(dev == 0)
        {
            rc = cpssDxChPortExtendedModeEnableSet(dev, 25, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortExtendedModeEnableSet", rc);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* 10GBASE-KR port 25 configuration */
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 25);

            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* 20GBASE-KR port 26 configuration */
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 26);

            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR2_E, CPSS_PORT_SPEED_20000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* DEV_0, PORT_26, LANE_0 - enable TX polarity swap */
            rc = cpssDxChPortSerdesPolaritySet(dev, 26, 1, GT_TRUE, GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }

        }
        else
        {
            /* 10GBASE-KR port 24 configuration */
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 24);

            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* 20GBASE-KR port 26 configuration */
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 26);

            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR2_E, CPSS_PORT_SPEED_20000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* DEV_1, PORT_26, LANE_0 - enable TX polarity swap */
            rc = cpssDxChPortSerdesPolaritySet(dev, 26, 1, GT_TRUE, GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else if(xcat3BoardType == APP_DEMO_AC3_BOARD_RD_MTL_TYPE3_CNS)
    {
        /* 1G SGMII port 24-27 configuration */
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
        for (portNum = 24 ; portNum < 28; portNum++)
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,portNum);
        }
        rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_SGMII_E, CPSS_PORT_SPEED_1000_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* PORT_25 - enable TX polarity swap */
        rc = cpssDxChPortSerdesPolaritySet(dev, 25, 1, GT_TRUE, GT_FALSE);
        if (rc != GT_OK)
                        {
                                return rc;
                        }

        /* PORT_27 - enable TX and RX polarity swap */
        rc = cpssDxChPortSerdesPolaritySet(dev, 27, 1, GT_TRUE, GT_TRUE);
        if (rc != GT_OK)
                        {
                                return rc;
                        }
    }
    else if(xcat3BoardType == APP_DEMO_AC3S_BOARD_RD_MTL_TYPE1_CNS)
    {
        rc = rd_ac3s_PortInterfaceInit(dev);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("rd_ac3s_PortInterfaceInit", rc);
        if (rc != GT_OK)
        {
                return rc;
        }
    }
    else if(xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P10)
    {
        if(dev == 0)
        {
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 24);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 25);

            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_SGMII_E, CPSS_PORT_SPEED_1000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 26);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 27);

            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_SGMII_E, CPSS_PORT_SPEED_1000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else if(xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P24)
    {
        if(dev == 0)
        {
            rc = cpssDxChPortExtendedModeEnableSet(dev, 25, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortExtendedModeEnableSet", rc);
            if(GT_OK != rc)
            {
                return rc;
            }

            rc = cpssDxChPortExtendedModeEnableSet(dev, 27, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortExtendedModeEnableSet", rc);
            if(GT_OK != rc)
            {
                return rc;
            }

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 25);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 27);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 24);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 26);

            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            cpssDxChPortSerdesPolaritySet(0, 25, 1, GT_FALSE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(0, 27, 1, GT_FALSE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(0, 24, 1, GT_FALSE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(0, 26, 1, GT_FALSE, GT_FALSE);
        }
    }
    else if(xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P48)
    {
        if(dev == 0)
        {
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 24);

            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_SGMII_E, CPSS_PORT_SPEED_2500_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 25);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 27);

            rc = cpssDxChPortExtendedModeEnableSet(dev, 25, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortExtendedModeEnableSet", rc);
            if(GT_OK != rc)
            {
                return rc;
            }

            rc = cpssDxChPortExtendedModeEnableSet(dev, 27, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortExtendedModeEnableSet", rc);
            if(GT_OK != rc)
            {
                return rc;
            }

            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            cpssDxChPortSerdesPolaritySet(0, 24, 1, GT_FALSE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(0, 25, 1, GT_FALSE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(0, 27, 1, GT_FALSE, GT_TRUE);
        }
        else
        {
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 24);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 25);

            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, 27);

            rc = cpssDxChPortExtendedModeEnableSet(dev, 27, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortExtendedModeEnableSet", rc);
            if(GT_OK != rc)
            {
                return rc;
            }

            rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_SGMII_E, CPSS_PORT_SPEED_2500_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            cpssDxChPortSerdesPolaritySet(1, 27, 1, GT_FALSE, GT_TRUE);
            cpssDxChPortSerdesPolaritySet(1, 24, 1, GT_FALSE, GT_FALSE);
            cpssDxChPortSerdesPolaritySet(1, 25, 1, GT_FALSE, GT_FALSE);
        }
    }
#if 0
    /* After port create - configure PPM on every SERDES */
    for (portNum = 0 ; portNum < 28;)
    {
        if(PRV_CPSS_PP_MAC(dev)->phyPortInfoArray[portNum].portType == PRV_CPSS_PORT_NOT_EXISTS_E)
        {
            if(portNum < 24)
            {
                portNum += 4;
            }
            else
            {
                portNum++;
            }

            continue;
        }

        if( (xcat3BoardType == APP_DEMO_AC3_BOARD_RD_MTL_TYPE1_CNS   ||
         xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P10             ||
             xcat3BoardType == APP_DEMO_AC3_BOARD_RD_MTL_TYPE2_CNS)  &&
            (portNum > 23))
        {
            /* skip cascade ports */
            if((portNum == appDemoPpConfigList[dev].cscdPortsArr[0].portNum) ||
               (portNum == appDemoPpConfigList[dev].cscdPortsArr[1].portNum))
            {
                portNum++;
                continue;
            }
        }

        rc = cpssDxChPortSerdesPpmSet(dev, portNum, 60);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesPpmSet", rc);
        if (rc != GT_OK)
        {
            DBG_TRACE(("cpssDxChPortSerdesPpmSet failed for devNum = %d, portNum = %d\r\n", dev, portNum));
            return rc;
        }

        if(portNum < 24)
        {
            portNum += 4;
        }
        else
        {
            portNum++;
        }
    }
#endif

    return rc;
}

/**
* @internal xcat3PortFcInit function
* @endinternal
*
* @brief   xCat3 FC configuration.
*
* @param[in] dev                      - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS xcat3PortFcInit
(
    IN  GT_U8 dev
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM portNum;

    /* Flow Control Initializations */
    for(portNum = 0; portNum < 28; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);

        rc = cpssDxChPortMacSaLsbSet(dev, portNum, (GT_U8)portNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortMacSaLsbSet", rc);
        if (rc != GT_OK)
        {
            return rc;
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
    GT_U32  i;
    GT_U8   devNum;
    GT_STATUS rc;
    GT_U8 port;                             /* Port number */
    GT_U32  row;
#ifndef ASIC_SIMULATION
    GT_U8   localDevNum = 0;
    GT_U8   localPortNum;
    GT_U16  marvell_1540PhyId = 0x141;
    GT_U16  localPhyId = 0;
#endif
    GT_U8   total_port;

    GT_U32 regAddr;
    CPSS_DXCH_IMPLEMENT_WA_ENT   waArr[1];
    GT_U32  enableSgmii25GWaNp; /* do we want to enable dp_clk and sync_fifo for
                                    sgmii 2.5G config WA on network Ports*/
    GT_U32  enableSgmii25GWaSp;   /* do we want to enable dp_clk and sync_fifo for
                                        sgmii 2.5G config WA on stacking ports */
    GT_U32  enableSgmii25GWa; /* do we want to enable dp_clk and sync_fifo for
                                    sgmii 2.5G config WA on both NP and SP */
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    for(i = SYSTEM_DEV_NUM_MAC(0); i < SYSTEM_DEV_NUM_MAC(ppCounter); ++i)
    {
        devNum = appDemoPpConfigList[i].devNum;

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
            {
                rc = xcat3PortInterfaceInit(devNum, boardRevId);
                if(GT_OK != rc)
                {
                    return rc;
                }
                else
                {
                    if(xcat3BoardType == APP_DEMO_AC3_BOARD_DB_CNS)
                    {
                        rc = xcat3PortPhy1548Init(devNum, 24);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    else
                    {
                        if ( xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P10)
                        {
                            rc = xcat3PortPhy1680Init(devNum, 8);
                        }
                        else if ( xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P24)
                        {
                            rc = xcat3PortPhy1680Init(devNum, 24);
                        }
                        else if ( xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P48)
                        {
                            if(devNum == 0)
                                total_port = 24;
                            else
                                total_port = 22;

                            rc = xcat3PortPhy1680Init(devNum, total_port);
                        }
                        else
                        {
                            /*APP_DEMO_AC3_BOARD_RD_MTL_TYPE1_CNS,
                            APP_DEMO_AC3_BOARD_RD_MTL_TYPE2_CNS,
                            APP_DEMO_AC3_BOARD_RD_MTL_TYPE3_CNS*/

                            rc = xcat3PortPhy1680Init(devNum, 24);
                        }

                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }

                    rc = xcat3PortFcInit(devNum);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }

                    /* go to next device */
                    continue;
                }
            }
        }

#if 0
        if(PRV_CPSS_PP_MAC(devNum)->revision > 0)
        {
            if (boardRevId == 5)
            {
                /* set port24/25 to SGMII mode */
                for (port = 24; port < 26;port++)
                {
                    rc = cpssDxChPortInterfaceModeSet(devNum,port,CPSS_PORT_INTERFACE_MODE_SGMII_E);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortInterfaceModeSet", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                    rc = cpssDxChPortSpeedSet(devNum,port,CPSS_PORT_SPEED_1000_E);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSpeedSet", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                    rc = cpssDxChPortSerdesPowerStatusSet(devNum,port,CPSS_PORT_DIRECTION_BOTH_E,
                                                          0x8,GT_TRUE);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesPowerStatusSet", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }

                    rc = cpssDxChPortInbandAutoNegEnableSet(devNum,port,GT_TRUE);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortInbandAutoNegEnableSet", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }

                    /* Configure 88E1112 PHY  */
                    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0x16, 0x2);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0x0, 0x1140);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0x0, 0x9140);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0x16, 0x0);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0x16, 0x1);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0x10, 0x200);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0x16, 0x0);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }

                    CPSS_TBD_BOOKMARK /* Should use API */
                    /* set xg_lanes_swap. Only needed for 24/25 ports */
                    rc = prvCpssHwPpSetRegField(devNum,0x8c,port+4,1,1);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssHwPpSetRegField", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }

                }
                /* set port26/27 to QX mode */
                for (port = 26; port < 28;port++)
                {
                    rc = cpssDxChPortInterfaceModeSet(devNum,port,CPSS_PORT_INTERFACE_MODE_QX_E);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortInterfaceModeSet", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                    rc = cpssDxChPortSpeedSet(devNum,port,CPSS_PORT_SPEED_2500_E);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSpeedSet", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                    rc = cpssDxChPortSerdesPowerStatusSet(devNum,port,CPSS_PORT_DIRECTION_BOTH_E,
                                                          0x1,GT_TRUE);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesPowerStatusSet", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                }

            }
            /********** PHYMAC **************/
            if (boardRevId == 9)
            {
                /* create semaphore for start/stop phy macsec transactions */
                if (osSemBinCreate(phyMacSemName, OS_SEMB_FULL,
                                   &phyMacSemId) != GT_OK)
                {
                    return GT_FAIL;
                }

                /*  phy info data base init */

                phyMacDataBase.phyInfoPtr = &phy1540MInfoDB[0];
                phyMacDataBase.phyInfoDBSize = phy1540MInfoDBSize;

                /* init PHYMAC object */
                rc =  macDrv1540MInit(
                        cpssSmiRegisterWriteShort,
                        cpssSmiRegisterReadShort,
                        phymacStartTransaction,
                        phymacStopTransaction,
                        &phyMacDataBase,
                        &macDrvObjPtr);

                if (rc != GT_OK)
                {
                    return rc;
                }

                /* bind ports to phy object  */
                rc = cpssDxChCfgBindPortPhymacObject(0,0,macDrvObjPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = cpssDxChCfgBindPortPhymacObject(0,1,macDrvObjPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = cpssDxChCfgBindPortPhymacObject(0,2,macDrvObjPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = cpssDxChCfgBindPortPhymacObject(0,3,macDrvObjPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /******************************************
                *      EEE init
                *******************************************/
                rc = phyMacEEEInit(0,0);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = phyMacEEEInit(0,1);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = phyMacEEEInit(0,2);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = phyMacEEEInit(0,3);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /***********************************************
                *  JUMBO PACKET  - MACPHY MRU initialisation
                *     MRU value 0x3FFF set for MACPHY binded ports
                * Note: it is required to trunsmit jumbo packets
                *       through 1540M
                ************************************************/
                /* port 0 set MRU:  SW MAC - 1522, SS PHYMAC - 3FFF,PS PHYMAC - 3FFF */
                rc = cpssDxChPortMruSet(0,0,1522);
                if (rc != GT_OK) {
                    return rc;
                }
                /* port 1 set MRU:  SW MAC - 1522, SS PHYMAC - 3FFF,PS PHYMAC - 3FFF */
                rc = cpssDxChPortMruSet(0,1,1522);
                if (rc != GT_OK) {
                    return rc;
                }
                /* port 2 set MRU:  SW MAC - 1522, SS PHYMAC - 3FFF,PS PHYMAC - 3FFF */
                rc = cpssDxChPortMruSet(0,2,1522);
                if (rc != GT_OK) {
                    return rc;
                }
                /* port 3 set MRU:  SW MAC - 1522, SS PHYMAC - 3FFF,PS PHYMAC - 3FFF */
                rc = cpssDxChPortMruSet(0,3,1522);
                if (rc != GT_OK) {
                    return rc;
                }
            }

            /*********************************************
             *  Set MACSEC disabled for 1540 bypass mode
             **********************************************/
#ifndef ASIC_SIMULATION
            /* if bord have 1540 PHYs */
            if (boardRevId != 9)
            {
                for(localPortNum = 0; localPortNum < 48; localPortNum++)
                {
                    /* skip not existing ports */
                    if( !PRV_CPSS_PHY_PORT_IS_EXIST_MAC(localDevNum, localPortNum) )
                    {
                        continue;
                    }

                    rc = phymacMacSecGetPhyId(localDevNum, localPortNum, &localPhyId);

                    /* the rc may be NOT GT_OK for stack ports or ports without PHY */
                    if (rc == GT_OK)
                    {
                        /* if the PHY is 1540 disable MACPHY */
                        if (localPhyId == marvell_1540PhyId)
                        {
                            rc = phymacMacSecEnable(localDevNum, localPortNum, GT_FALSE);
                            CPSS_ENABLER_DBG_TRACE_RC_MAC("phymacMacSecEnable", rc);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }
                        }
                    }
                }
            }
#endif

            /**********************************************/

            /* initialize PHY1340 of GE network ports */
            for (port = 0; port < 24;port++)
            {
                if( !PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, port) )
                    continue;

                for(row = 0; xcatC0NPPhyConfigArr[row].data != 0xdead; row++)
                {
                    /* set page register #22 */
                    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0x16,
                                                 xcatC0NPPhyConfigArr[row].page);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite page", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                    /* do configuration */
                    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, port,
                                             xcatC0NPPhyConfigArr[row].regAddr,
                                             xcatC0NPPhyConfigArr[row].data);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite data", rc);
                    if( rc != GT_OK )
                    {
                        return rc;
                    }
                }
            }

        } /* if(PRV_CPSS_PP_MAC(devNum)->revision > 0) */
    }

    if(PRV_CPSS_PP_MAC(SYSTEM_DEV_NUM_MAC(0))->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        rc = appDemoDxChNetRxPacketCbRegister(appDemoDxChNetPtpOverUdpWaCb);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    for(i = SYSTEM_DEV_NUM_MAC(0); i < SYSTEM_DEV_NUM_MAC(ppCounter); ++i)
    {
        devNum = appDemoPpConfigList[i].devNum;
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
           (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
        {
            if(GT_OK != appDemoDbEntryGet("GT_CORE_SYS_WA_SGMII_2_5G_UNDER_TRAFFIC_CFG_E", &enableSgmii25GWa))
            {/* by default we don't want this WA to be engaged by appDemo, because majority of
                boards use QSGMII and QSGMII and 2.5G SGMII cant coexist(only if there is a per-port setting of
                dp_clk and sync_fifo). */
                enableSgmii25GWa = 0;
            }

            if(GT_OK != appDemoDbEntryGet("GT_CORE_SYS_WA_SGMII_2_5G_UNDER_TRAFFIC_NP_CFG_E", &enableSgmii25GWaNp))
            {
                enableSgmii25GWaNp = 0;
            }

            if(GT_OK != appDemoDbEntryGet("GT_CORE_SYS_WA_SGMII_2_5G_UNDER_TRAFFIC_SP_CFG_E", &enableSgmii25GWaSp))
            {
                enableSgmii25GWaSp = 0;
            }

            if(enableSgmii25GWa || enableSgmii25GWaNp)
            {/* before dp_clk and sync_fifo could be defined for 2.5G SGMII correct work, must be issued
                frame on 1G SGMII to prevent eggress fifo stuck;
                this part must be executed before engage CPSS_DXCH_IMPLEMENT_WA_SGMII_2_5G_UNDER_TRAFFIC_CFG_E
                it placed here, because implementation of traffic send from CPU is in responsibility of
                application */

                for (port = 0; port < 24; port++)
                {
                    if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[port].portType ==
                         PRV_CPSS_PORT_NOT_EXISTS_E)
                        continue;

                    /* run WA only on ports where SGMII could be defined, in DB boards case
                        it's first port in quadruplet */
                    if(port%4 != 0)
                        continue;

                    rc = cpssDxChPortInterfaceModeSet(devNum, port, CPSS_PORT_INTERFACE_MODE_SGMII_E);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, port, GT_FALSE);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = cpssDxChPortSpeedSet(devNum, port, CPSS_PORT_SPEED_1000_E);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, port, GT_FALSE);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = cpssDxChPortDuplexModeSet(devNum, port, CPSS_PORT_FULL_DUPLEX_E);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = cpssDxChPortFlowCntrlAutoNegEnableSet(devNum, port, GT_FALSE, GT_FALSE);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = cpssDxChPortFlowControlEnableSet(devNum, port, CPSS_PORT_FLOW_CONTROL_DISABLE_E);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = cpssDxChPortSerdesPowerStatusSet(devNum, port,
                                                          CPSS_PORT_DIRECTION_BOTH_E,
                                                          0x1, GT_TRUE);
                    if (rc!= GT_OK)
                    {
                        return rc;
                    }

                    /* force link up to cause packet to pass whole eggress pipe */
                    if((rc = cpssDxChPortForceLinkPassEnableSet(devNum,port,GT_TRUE)) != GT_OK)
                        return rc;

                    /* Enable analog loopback */
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[port/4]
                                .serdesExternalReg1;
                    rc = prvCpssDrvHwPpSetRegField(devNum,regAddr,2,1,1);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    /* wait for system to stabilize after ForceLinkPass and loopback enable */
                    osTimerWkAfter(200);

                    /* synchronous send from CPU, if send asynchronous must add delay before
                        loopback and forceLinkPass disable */
                    rc = cpuTx(port, GT_TRUE);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    /* disable loopback */
                    rc = prvCpssDrvHwPpSetRegField(devNum,regAddr,2,1,0);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    /* force link up disable */
                    if((rc = cpssDxChPortForceLinkPassEnableSet(devNum,port,GT_FALSE)) != GT_OK)
                        return rc;

                    /* wait for system to stabilize after ForceLinkPass and loopback disable */
                    osTimerWkAfter(200);
                }

                /* state that application want CPSS to implement the dp_clk and sync_fifo
                    for sgmii 2.5G configuration WA */
                waArr[0] = CPSS_DXCH_IMPLEMENT_WA_SGMII_2_5G_UNDER_TRAFFIC_NETWORKP_CFG_E;
                rc = cpssDxChHwPpImplementWaInit(devNum,1,&waArr[0],NULL);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }

            if(enableSgmii25GWa || enableSgmii25GWaSp)
            {
                /* state that application want CPSS to implement the dp_clk and sync_fifo
                    for sgmii 2.5G configuration WA */
                waArr[0] = CPSS_DXCH_IMPLEMENT_WA_SGMII_2_5G_UNDER_TRAFFIC_STACKP_CFG_E;
                rc = cpssDxChHwPpImplementWaInit(devNum,1,&waArr[0],NULL);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
#endif
    }

    return GT_OK;
}

/**
* @internal gtAppDemoXcat2SfpPortEnable function
* @endinternal
*
* @brief   This function configures MUX switching PHYs to SFPs to enable testing
*         of 2.5G mode on network ports
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (just network ports)
* @param[in] enable                   -   GT_TRUE  - use SFP port
*                                      GT_FALSE - use PHY port
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - hw error
* @retval GT_BAD_PARAM             - wrong device or port
* @retval GT_NOT_APPLICABLE_DEVICE - wrong devFamily
*
* @note Enable SFP must be done only after port configured to one of appropriate
*       modes i.e. 1000BaseX, SGMII 2.5G or SGMII 1G (while in case of SGMII 1G
*       on SFP port to get link you must enable ForceLinkPass and inband autoneg.
*       bypass, because SGMII 1G supposed to work over PHY).
*       And if you want to disable SFP and go back to default (PHY) you must
*       first configure QSGMII or other mode supported by regular network ports
*       on given device and only then call this function.
*
*/
GT_STATUS gtAppDemoXcat2SfpPortEnable
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      ioSelectorsRegAddr  = 0xB0; /* address of I/O Selectors
                                                        register */
    GT_U32      gppIoCtrlRegAddr    = 0x018001C8; /* address of GPP I/O Control
                                                        register */
    GT_U32      gppOutputRegAddr    = 0x018001C4; /* address of GPP Output
                                                        register */
    GT_U32      fieldData;      /* register field data */
    GT_U32      fieldOffset;    /* offset of field in register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
        ( PRV_CPSS_PP_MAC(devNum)->revision < 3 ))
    {/* PHY/SFP MUX exists just on XCAT2 B0 DB board */
        return GT_NOT_APPLICABLE_DEVICE;
    }

    /* PHY/SFP MUX made for network ports only */
    if(portNum > 23)
        return GT_BAD_PARAM;

    /* I'm not sure what could be effect of repeating write to ioSelectors
        register, so I prefer to check if needed configuration not done yet */
    rc = prvCpssDrvHwPpGetRegField(devNum, ioSelectorsRegAddr, 3, 2,
                                    &fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(fieldData != 0)
    {
        rc = prvCpssDrvHwPpSetRegField(devNum, ioSelectorsRegAddr, 3, 2, 0);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* in XCAT2 one serdes per quadruplet of network GE ports */
    switch(portNum/4)
    {
        case 0:
            fieldOffset = 6;
            break;
        case 1:
            fieldOffset = 7;
            break;
        case 2:
            fieldOffset = 8;
            break;
        case 3:
            fieldOffset = 9;
            break;
        case 4:
            fieldOffset = 0;
            break;
        case 5:
            fieldOffset = 1;
            break;
        default:
            return GT_BAD_PARAM;
    }
    /* I'm not sure what could be effect of repeating write to GPP I/O Ctrl
        register, so I prefer to check if needed configuration not done yet */
    rc = prvCpssDrvHwPpGetRegField(devNum, gppIoCtrlRegAddr, fieldOffset, 1,
                                    &fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(fieldData != 1)
    {
        rc = prvCpssDrvHwPpSetRegField(devNum, gppIoCtrlRegAddr, fieldOffset,
                                        1, 1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    fieldData = BOOL2BIT_MAC(enable);
    rc = prvCpssDrvHwPpSetRegField(devNum, gppOutputRegAddr, fieldOffset, 1,
                                    fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal gtAppDemoXcat2StackPortsModeSpeedSet function
* @endinternal
*
* @brief   Example of configuration sequence for stack ports of xcat2
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] ifMode                   - Interface mode.
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - hw error
* @retval GT_BAD_PARAM             - device not exists
* @retval GT_NOT_SUPPORTED         - wrong ifMode,speed
* @retval GT_NOT_APPLICABLE_DEVICE - wrong devFamily
*
* @note At this moment supported:
*       CPSS_PORT_INTERFACE_MODE_SGMII_E    10M, 100M, 1G, 2.5G
*       CPSS_PORT_INTERFACE_MODE_1000BASE_X_E  1G
*       CPSS_PORT_INTERFACE_MODE_100BASE_FX_E  100M
*
*/
GT_STATUS gtAppDemoXcat2StackPortsModeSpeedSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E)
        return GT_NOT_APPLICABLE_DEVICE;

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    if((portNum < 24) || (portNum > 27))
        return GT_BAD_PARAM;

    switch(ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_100BASE_FX_E:
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    switch(speed)
    {
        case CPSS_PORT_SPEED_10_E:
        case CPSS_PORT_SPEED_100_E:
            if((CPSS_PORT_INTERFACE_MODE_SGMII_E != ifMode) && (CPSS_PORT_INTERFACE_MODE_100BASE_FX_E != ifMode))
                return GT_NOT_SUPPORTED;
            break;

        case CPSS_PORT_SPEED_1000_E:
            if(CPSS_PORT_INTERFACE_MODE_100BASE_FX_E == ifMode)
                return GT_NOT_SUPPORTED;
            break;

        case CPSS_PORT_SPEED_2500_E:
            if(CPSS_PORT_INTERFACE_MODE_SGMII_E != ifMode)
                return GT_NOT_SUPPORTED;
            break;

        default:
            return GT_NOT_SUPPORTED;
    }

    if((rc = cpssDxChPortInterfaceModeSet(devNum, portNum, ifMode)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortSpeedSet(devNum, portNum, speed)) != GT_OK)
        return rc;

    if((rc = cpssDxChPortSerdesPowerStatusSet(devNum, portNum, CPSS_PORT_DIRECTION_BOTH_E,
                                                1, GT_TRUE)) != GT_OK)
    {
        return rc;
    }

    if((CPSS_PORT_INTERFACE_MODE_1000BASE_X_E == ifMode) ||
        (CPSS_PORT_INTERFACE_MODE_100BASE_FX_E == ifMode) ||
            (CPSS_PORT_SPEED_2500_E == speed))
    {
        if((rc = cpssDxChPortSpeedAutoNegEnableSet(devNum,portNum,GT_FALSE)) != GT_OK)
            return rc;

        if((rc = cpssDxChPortDuplexAutoNegEnableSet(devNum,portNum,GT_FALSE)) != GT_OK)
            return rc;

        if((rc = cpssDxChPortDuplexModeSet(devNum, portNum, CPSS_PORT_FULL_DUPLEX_E)) != GT_OK)
            return rc;
    }

    if((ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E) && (speed <= CPSS_PORT_SPEED_1000_E))
    {
        if((rc = cpssDxChPortInBandAutoNegBypassEnableSet(devNum,portNum,GT_FALSE)) != GT_OK)
        {
            return rc;
        }

        if((rc = cpssDxChPortInbandAutoNegEnableSet(devNum,portNum,GT_TRUE)) != GT_OK)
        {
            return rc;
        }
    }

    /* In 1000BaseX mode to get link with smartBits, or other partner that doesn't suppport
     * inband autoneg, InBandAutoNegBypass must be enabled
     */
    if((CPSS_PORT_INTERFACE_MODE_1000BASE_X_E == ifMode) || (CPSS_PORT_SPEED_2500_E == speed)
        || (CPSS_PORT_INTERFACE_MODE_100BASE_FX_E == ifMode))
    {
        if((rc = cpssDxChPortInBandAutoNegBypassEnableSet(devNum,portNum,GT_TRUE)) != GT_OK)
        {
            return rc;
        }

        if((rc = cpssDxChPortInbandAutoNegEnableSet(devNum,portNum,GT_TRUE)) != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal gtRdxCatBoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtRdxCatBoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    /* next static are and maybe changed from compilation time */
    dxChInitRegDefaults = GT_FALSE;

    ppCounter = 0;

    xcat3BoardResetDone = GT_TRUE;

    return GT_OK;
}

/**
* @internal gtDbDxXcat24G4HgsBoardReg function
* @endinternal
*
* @brief   Registration function for the xCat 24G + 4Hgs ports board (xCat devices).
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxXcat24G4HgsBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    if(boardCfgFuncs == NULL)
    {
        return GT_FAIL;
    }

    boardCfgFuncs->boardGetInfo                 = getBoardInfo;
    /*boardCfgFuncs->boardGetCoreCfgPh1Params     = getCoreSysCfgPhase1;*/
    boardCfgFuncs->boardGetPpPh1Params          = getPpPhase1Config;
    boardCfgFuncs->boardAfterPhase1Config       = configBoardAfterPhase1;
    /*boardCfgFuncs->boardGetCoreCfgPh2Params     = getCoreSysCfgPhase2;*/
    boardCfgFuncs->boardGetPpPh2Params          = getPpPhase2Config;
    boardCfgFuncs->boardAfterPhase2Config       = configBoardAfterPhase2;
    /*boardCfgFuncs->boardGetSysCfgParams         = getSysConfigParams;*/
    boardCfgFuncs->boardGetPpLogInitParams      = getPpLogicalInitParams;
    boardCfgFuncs->boardGetLibInitParams        = getTapiLibInitParams;
    boardCfgFuncs->boardAfterInitConfig         = afterInitBoardConfig;
    boardCfgFuncs->boardCleanDbDuringSystemReset= gtRdxCatBoardCleanDbDuringSystemReset;
    /*boardCfgFuncs->boardGetUserPorts            = getUserPorts;*/

    return GT_OK;
}

#define BAD_PHY_ADDR 0xFFFF
#define PHY_1548M_NUM_OF_PORTS_CNS 4
#define PHY_1680M_NUM_OF_PORTS_CNS 8

static APP_DEMO_QUAD_PHY_CFG_STC  PHY1548_Init_Array_phase_1[] =
{
    /* for all/first port, regaddr, regvalue, delay if need in ms */
    { AD_ALL_PORTS,           22, 0x0000 } /* RW u1 P0-3 R22 H0000 */

    /*---------------------------------------*/
    /* MACSEC disable                        */
    /*---------------------------------------*/
   ,{ AD_ALL_PORTS,           22, 0x0012 } /* RW u1 P0-3 R22 H0012 */
   ,{ AD_ALL_PORTS,           27, 0x0000 } /* RW u1 P0-3 R27 H0000 */

    /* ---------------------------------------*/
    /* QSGMII Amp                             */
    /* ---------------------------------------*/
   ,{ AD_ALL_PORTS,           22, 0x00FD }  /* RW u1 P0-3 R22 h00FD */
   ,{ AD_ALL_PORTS,           11, 0x1D70 }  /* RW u1 P0-3 R11 h1D70 */
    /* ---------------------------------------*/
    /* QSGMII block power down/up WA - ?      */
    /* ---------------------------------------*/
   ,{ AD_ALL_PORTS,           22, 0x0004 }  /* RW u1 P0-3 R22 h0004 */
   ,{ AD_ALL_PORTS,           26, 0x3802 }  /* RW u1 P0-3 R26 h3802 , Wait 300ms */
   ,{  AD_BAD_PORT,            0, 0x0000 }  /* end seq */
};

static APP_DEMO_QUAD_PHY_CFG_STC  PHY1548_Init_Array_phase_2[] =
{
    { AD_ALL_PORTS,           26, 0x3002 }  /* RW u1 P0-3 R26 h3002 , Wait 200ms */
   ,{  AD_BAD_PORT,            0, 0x0000 }  /* end seq */
};

static APP_DEMO_QUAD_PHY_CFG_STC  PHY1548_Init_Array_phase_3[] =
{
    /*--------------------------------------------------*/
    /* Incorrect Q_ANEG configuration errata workaround */
    /*--------------------------------------------------*/
    {  AD_ALL_PORTS,           22, 0x0004 } /* RW u1 P0-3 R22 h0004 */
   ,{  AD_ALL_PORTS,            0, 0x9140 } /* RW u1 P0-3 R0  h9140 */
    /* ---------------------------------------*/
    /* Soft-Reset                             */
    /* ---------------------------------------*/
   ,{ AD_ALL_PORTS,           22, 0x0000 }/* RW u1 P0-3 R22 H0000 */
   ,{ AD_ALL_PORTS,            0, 0x9140 }/* RW u1 P0-3 R0  H9140 */
   ,{  AD_BAD_PORT,            0, 0x0000 }  /* end seq */
};

APP_DEMO_QUAD_PHY_CFG_STC_PHASE_STC PHY1548_Init_Array[] =
{
     { &PHY1548_Init_Array_phase_1[0], 300 }
    ,{ &PHY1548_Init_Array_phase_2[0], 200 }
    ,{ &PHY1548_Init_Array_phase_3[0],   0 }
    ,{ NULL                          ,   0 }
};


/**
* @internal xcat3PortPhy1548Init function
* @endinternal
*
* @brief   Quad PHY1548 init sequence.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS xcat3PortPhy1548Init
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM  totalPortNum
)
{
    GT_STATUS       rc;             /* return code */
    GT_U32          firstPort;      /* port number */
    GT_U32          numOfPorts;     /* port number */
    GT_U32          phaseId;        /* phase number */
    GT_U32          ii;             /* loop iterator */
    APP_DEMO_QUAD_PHY_CFG_STC            *currentArrayPtr;/* pointer to PHY config structure */
    APP_DEMO_QUAD_PHY_CFG_STC_PHASE_STC  *phasePtr;      /* pointer to phase structure */
    GT_U32          ppPort;       /* port of PFY device */
    GT_U8           phyRegAddr;   /* PHY reg address */
    GT_U16          phyRegData;   /* PHY reg data */

    for (phaseId = 1, phasePtr = &PHY1548_Init_Array[0]; phasePtr->seqPtr != NULL; phaseId++, phasePtr++)
    {
        for (firstPort  = 0; firstPort  < totalPortNum; firstPort += PHY_1548M_NUM_OF_PORTS_CNS)
        {
            currentArrayPtr = phasePtr->seqPtr;
            for (ii = 0; currentArrayPtr->allPorts != AD_BAD_PORT; ii++, currentArrayPtr++)  /* loop over commands */
            {
                /* get number of PHY ports to config */
                if (currentArrayPtr->allPorts == AD_ALL_PORTS)
                {
                    numOfPorts = totalPortNum-firstPort;
                    if (numOfPorts > PHY_1548M_NUM_OF_PORTS_CNS)
                    {
                        numOfPorts = PHY_1548M_NUM_OF_PORTS_CNS;
                    }
                }
                else /* AD_FIRST_PORT */
                {
                    numOfPorts = 1;
                }

                for (ppPort = firstPort; ppPort < firstPort + numOfPorts; ppPort++)
                {
                    CPSS_ENABLER_PORT_SKIP_CHECK(devNum, ppPort);

                    phyRegAddr = (GT_U8)currentArrayPtr->phyRegAddr;
                    phyRegData = currentArrayPtr->phyRegData;

                    rc = cpssDxChPhyPortSmiRegisterWrite(devNum, ppPort, phyRegAddr, phyRegData);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintf("\n-->ERROR : xcat3PortPhy1548Init() failure: phase = %2d PP port[%d] Reg[%d] Data[%04X]\n", phaseId,ppPort, phyRegAddr, phyRegData);
                        return rc;
                    }
                }
            }
        }
        /* whether there is a need for a delay */
        if (phasePtr->sleepTimeAfter_ms > 0)
        {
            cpssOsTimerWkAfter(phasePtr->sleepTimeAfter_ms);
        }

    }

    return GT_OK;
}


/**
* @internal xcat3PortPhy1680Init function
* @endinternal
*
* @brief   Quad PHY1680 init sequence.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS xcat3PortPhy1680Init
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM  totalPortNum
)
{
    GT_STATUS       rc;             /* return code */
    GT_PHYSICAL_PORT_NUM  portNum;  /* port iterator */

    for(portNum = 0; portNum < totalPortNum; portNum++)
    {
        /* matrix mode */
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 4);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 27, 0x3FA0);
        if( rc != GT_OK)
        {
            return rc;
        }



        /* MACSec and PTP disable */
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0);
        if( rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0x12);
        if( rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 27, 0);
        if( rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0);
        if( rc != GT_OK)
        {
            return rc;
        }



        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0xFD);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 8, 0xB53);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 7, 0x200D);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* EEE init */
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0xFF);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 17, 0xB030);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 16, 0x215C);
        if( rc != GT_OK)
        {
            return rc;
        }


        /* LED0 for 1000Mbps, LED1 for link/act */
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 3);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 16, 0x1117);
        if( rc != GT_OK)
        {
            return rc;
        }


        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 22, 0x0);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 16, 0x3360);
        if( rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, 0, 0x9140);
        if( rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal appDemoFdbAccessTimeCheck function
* @endinternal
*
* @brief   Time estimation for FDB access time.
*
* @param[in] devNum                   - device number
* @param[in] loops                    - number of  to perform
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoFdbAccessTimeCheck
(
    IN  GT_U8 devNum,
    IN  GT_U32 loops
)
{
    GT_STATUS       rc;             /* return code */
    GT_U32          portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32          loopNum;
    GT_U32          fdbSize;
    GT_U32          entryIndex;
    GT_U32          regAddr;
    GT_U32          hwData[4];
    GT_U32          startSeconds;
    GT_U32          startNanoSeconds;
    GT_U32          stopSeconds;
    GT_U32          stopNanoSeconds;
    GT_U32          elapsedSeconds;
    GT_U32          elapsedNanoSeconds;

    if (loops == 0)
    {
        loops = 1000;
    }

    rc = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_FDB_E,&fdbSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssOsTimeRT(&startSeconds, &startNanoSeconds);
    if(rc != GT_OK)
    {
        return rc;
    }

    for( loopNum = 0 ; loopNum < loops; loopNum++)
    {
        regAddr = 0x0B400000;   /* FDB Memory address according to Cider */
        for( entryIndex = 0 ; entryIndex < fdbSize; entryIndex++)
        {
            /* read all data registers */
            rc =  prvCpssHwPpPortGroupReadRam(devNum,
                                                portGroupId,
                                                regAddr,
                                                4,/* num of words */
                                                &hwData[0]);
            if(rc != GT_OK)
            {
                return rc;
            }

            U32_SET_FIELD_IN_ENTRY_MAC(hwData,2,1,0);   /* Clear the hit bit */
            rc =  prvCpssHwPpPortGroupWriteRam(devNum,
                                                portGroupId,
                                                regAddr,
                                                4,/* num of words */
                                                &hwData[0]);
            if(rc != GT_OK)
            {
                return rc;
            }

            regAddr += 16;   /* Update reg. address to next line (16 bytes) */
        }

    }

    rc = cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (stopNanoSeconds >= startNanoSeconds)
    {
        elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
        elapsedSeconds = (stopSeconds - startSeconds);
    }
    else
    {
        elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
        elapsedSeconds = (stopSeconds - startSeconds) - 1;
    }
    cpssOsPrintf("total running [%d.%03d] secs (for [%d] loops, [%d] FDB size) \n",
        elapsedSeconds, elapsedNanoSeconds / (1000*1000),
        loops,
        fdbSize);
    elapsedNanoSeconds = (((long long)(elapsedSeconds % loops) * (1000*1000*1000)) + elapsedNanoSeconds) / loops;
    elapsedSeconds = (elapsedSeconds / loops);
    cpssOsPrintf("average per single loop [%d.%06d] secs  \n",
        elapsedSeconds, elapsedNanoSeconds / 1000);

    return GT_OK;
}

static APPDEMO_SERDES_LANE_POLARITY_STC  xCat3s_DB_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_FALSE,   GT_FALSE },
    { 1,    GT_TRUE,    GT_FALSE },
    { 2,    GT_TRUE,    GT_TRUE  },
    { 3,    GT_FALSE,   GT_TRUE  },
    { 4,    GT_FALSE,   GT_TRUE  },
    { 5,    GT_FALSE,   GT_TRUE  },
    { 6,    GT_FALSE,   GT_TRUE  },
    { 7,    GT_TRUE,    GT_FALSE },
    { 8,    GT_FALSE,   GT_TRUE  },
    { 9,    GT_FALSE,   GT_FALSE },
    { 10,   GT_FALSE,   GT_TRUE  },
    { 11,   GT_TRUE,    GT_FALSE }
};

/**
* @internal prvAppDemoXcat3sSerdesPolarityConfigSet function
* @endinternal
*
* @brief   Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*
* @note   APPLICABLE DEVICES: xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvAppDemoXcat3sSerdesPolarityConfigSet
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

    switch(devType)
    {
        case CPSS_XCAT3S_ALL_DEVICES_CASES_MAC:
            if(xcat3BoardType == APP_DEMO_AC3S_BOARD_RD_MTL_TYPE1_CNS)
            {
                rc = rd_ac3s_PolarityArrayGet(devNum,&currentPolarityArrayPtr,&polarityArraySize);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("rd_ac3s_PortInterfaceInit", rc);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
            currentPolarityArrayPtr = xCat3s_DB_PolarityArray;
            polarityArraySize = sizeof(xCat3s_DB_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
            }
            break;
        default:
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

static APPDEMO_SERDES_LANE_POLARITY_STC  xCat3s_RD_24G4XG_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_FALSE,    GT_FALSE },
    { 1,    GT_FALSE,    GT_FALSE },
    { 2,    GT_TRUE,     GT_TRUE  },
    { 3,    GT_FALSE,    GT_TRUE  },
    { 4,    GT_FALSE,    GT_TRUE  },
    { 5,    GT_FALSE,    GT_TRUE  },
    { 6,    GT_TRUE,     GT_TRUE  },
    { 7,    GT_FALSE,    GT_FALSE },
    { 8,    GT_TRUE,     GT_TRUE  },
    { 9,    GT_TRUE,     GT_FALSE },
    { 10,   GT_FALSE,    GT_TRUE  },
    { 11,   GT_FALSE,    GT_FALSE }
};

/**
* @internal rd_ac3s_LedInit function
* @endinternal
*
* @brief   xCat3s-rd LED init.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS rd_ac3s_LedInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc; /* return code */
    GT_U32 ledInterfaceNum;    /* led stream interface number */
    GT_U32  portNum;/* port Num */

    CPSS_LED_CONF_STC               ledStreamConf;      /* led stream interface configuration structure */
    CPSS_LED_CLASS_MANIPULATION_STC ledClassParams;     /* led class manipulation configuration */
    CPSS_LED_GROUP_CONF_STC         ledGroupParams;     /* led group configuration parameters */

    if(xcat3BoardType != APP_DEMO_AC3S_BOARD_RD_MTL_TYPE1_CNS)
    {
        return GT_FAIL;
    }

    cpssOsPrintf("rd_ac3s_LedInit enter \n") ;

    for(portNum = 24 ; portNum < 28 ; portNum++)
    {
        rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, portNum/*position*/);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    for (ledInterfaceNum = 0; ledInterfaceNum < 2; ledInterfaceNum++)
    {
        rc = cpssDxChLedStreamDirectModeEnableSet(devNum, ledInterfaceNum, GT_TRUE );
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = cpssDxChLedStreamDirectModeEnableSet(devNum, ledInterfaceNum, GT_TRUE );
        if(GT_OK != rc)
        {
            return rc;
        }

        /*Led stream configuration */
        ledStreamConf.ledOrganize = CPSS_LED_ORDER_MODE_BY_PORT_E;
        ledStreamConf.disableOnLinkDown = GT_FALSE;
        ledStreamConf.blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
        ledStreamConf.blink0Duration  = CPSS_LED_BLINK_DURATION_5_E;
        ledStreamConf.blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
        ledStreamConf.blink1Duration  = CPSS_LED_BLINK_DURATION_5_E;
        ledStreamConf.pulseStretch  = CPSS_LED_PULSE_STRETCH_1_E;
        ledStreamConf.ledStart  = 0x0;
        ledStreamConf.ledEnd    = 0xFF;
        ledStreamConf.clkInvert = GT_FALSE;
        ledStreamConf.class5select  = CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
        ledStreamConf.class13select = CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;
        ledStreamConf.invertEnable = GT_FALSE;
        rc = cpssDxChLedStreamConfigSet(devNum,ledInterfaceNum, &ledStreamConf);
        if(GT_OK != rc)
        {
            return rc;
        }

        ledClassParams.invertEnable =  GT_TRUE;
        ledClassParams.blinkEnable =  GT_FALSE ;
        ledClassParams.blinkSelect = CPSS_LED_BLINK_SELECT_0_E ;
        ledClassParams.forceEnable =  GT_FALSE ;
        ledClassParams.forceData =  GT_FALSE ;
        ledClassParams.pulseStretchEnable = GT_FALSE ;
        ledClassParams.disableOnLinkDown =  GT_FALSE ;
        rc = cpssDxChLedStreamClassManipulationSet(devNum, ledInterfaceNum , CPSS_DXCH_LED_PORT_TYPE_XG_E, 9, &ledClassParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        ledClassParams.invertEnable =  GT_FALSE;
        ledClassParams.blinkEnable =  GT_TRUE ;
        ledClassParams.blinkSelect = CPSS_LED_BLINK_SELECT_0_E ;
        ledClassParams.forceEnable =  GT_FALSE ;
        ledClassParams.forceData =  GT_FALSE ;
        ledClassParams.pulseStretchEnable = GT_FALSE ;
        ledClassParams.disableOnLinkDown =  GT_FALSE ;
        rc = cpssDxChLedStreamClassManipulationSet(devNum, ledInterfaceNum , CPSS_DXCH_LED_PORT_TYPE_XG_E, 10, &ledClassParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* For xCAT3 A1 and above */
        ledGroupParams.classA = 9;
        ledGroupParams.classB = 9;
        ledGroupParams.classC = 10;
        ledGroupParams.classD = 10;
        rc = cpssDxChLedStreamGroupConfigSet(devNum, ledInterfaceNum,
                 CPSS_DXCH_LED_PORT_TYPE_XG_E,
                 0, &ledGroupParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        ledGroupParams.classA = 9;
        ledGroupParams.classB = 9;
        ledGroupParams.classC = 10;
        ledGroupParams.classD = 10;
        rc = cpssDxChLedStreamGroupConfigSet(devNum, ledInterfaceNum,
                 CPSS_DXCH_LED_PORT_TYPE_XG_E,
                 1, &ledGroupParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = cpssDxChLedStreamClassIndicationSet(devNum, ledInterfaceNum, 9, CPSS_DXCH_LED_INDICATION_LINK_E) ;
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = cpssDxChLedStreamClassIndicationSet(devNum, ledInterfaceNum, 10, CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E) ;
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    rc = prvCpssHwPpWriteRegister(devNum, 0x09005110, 0x10E1);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssHwPpWriteRegister(devNum, 0x07005110, 0x10E1);
    if(GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal rd_ac3s_PortInterfaceInit function
* @endinternal
*
* @brief   xcat3s : Execute predefined ports configuration.
*
* @param[in] dev                      - device number
* @param[in] boardRevId               - revision ID
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS rd_ac3s_PortInterfaceInit
(
    IN  GT_U8 dev
)
{
    GT_STATUS           rc;             /* return code */
    CPSS_PORTS_BMP_STC  initPortsBmp;   /* bitmap of ports to init */
    GT_PHYSICAL_PORT_NUM  portNum;

    if(xcat3BoardType != APP_DEMO_AC3S_BOARD_RD_MTL_TYPE1_CNS)
    {
        return GT_FAIL;
    }

    /* 10GBASE-KR port 24-27 configuration */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
    for (portNum = 24 ; portNum < 28; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);
        CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,portNum);
    }

    if (!CPSS_PORTS_BMP_IS_ZERO_MAC(&initPortsBmp))
    {
        rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal rd_ac3s_PolarityArrayGet function
* @endinternal
*
* @brief   xcat3s : get the polarity array
*
* @param[in] dev                      - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS rd_ac3s_PolarityArrayGet
(
    IN  GT_U8 dev,
    OUT APPDEMO_SERDES_LANE_POLARITY_STC    **polarityArrayPtrPtr,
    OUT GT_U32                              *polarityArraySizePtr
)
{
    if(xcat3BoardType != APP_DEMO_AC3S_BOARD_RD_MTL_TYPE1_CNS)
    {
        return GT_FAIL;
    }

    cpssOsPrintf("prvAppDemoXcat3sSerdesPolarityConfigSet \n");
    *polarityArrayPtrPtr  = xCat3s_RD_24G4XG_PolarityArray;
    *polarityArraySizePtr = sizeof(xCat3s_RD_24G4XG_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
    return GT_OK;
}

/* Add to get WNC SKU ID */
GT_STATUS xcat3BoardTypeGet
(
    OUT GT_U32 *dataPtr
)
{

    *dataPtr = xcat3BoardType;

    return GT_OK;
}

static GT_STATUS ac3_LedInit_OS6360
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc; /* return code */
    GT_U32 ledInterfaceNum;    /* led stream interface number */
    GT_U32  portNum;/* port Num */

    CPSS_LED_CONF_STC               ledStreamConf;      /* led stream interface configuration structure */
    CPSS_LED_CLASS_MANIPULATION_STC ledClassParams;     /* led class manipulation configuration */
    CPSS_LED_GROUP_CONF_STC         ledGroupParams;     /* led group configuration parameters */

    for(portNum = 24 ; portNum < 28 ; portNum++)
    {
        rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, portNum);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    for (ledInterfaceNum = 0; ledInterfaceNum < 2; ledInterfaceNum++)
    {
        rc = cpssDxChLedStreamDirectModeEnableSet(devNum, ledInterfaceNum, GT_TRUE );
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = cpssDxChLedStreamDirectModeEnableSet(devNum, ledInterfaceNum, GT_TRUE );
        if(GT_OK != rc)
        {
            return rc;
        }

        /*Led stream configuration */
        ledStreamConf.ledOrganize = CPSS_LED_ORDER_MODE_BY_PORT_E;
        ledStreamConf.disableOnLinkDown = GT_FALSE;
        ledStreamConf.blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
        ledStreamConf.blink0Duration  = CPSS_LED_BLINK_DURATION_5_E;
        ledStreamConf.blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
        ledStreamConf.blink1Duration  = CPSS_LED_BLINK_DURATION_0_E;
        ledStreamConf.pulseStretch  = CPSS_LED_PULSE_STRETCH_1_E;
        ledStreamConf.ledStart  = 0x0;
        ledStreamConf.ledEnd    = 0xFF;
        ledStreamConf.clkInvert = GT_FALSE;
        ledStreamConf.class5select  = CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
        ledStreamConf.class13select = CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;
        ledStreamConf.invertEnable = GT_FALSE;
        rc = cpssDxChLedStreamConfigSet(devNum,ledInterfaceNum, &ledStreamConf);
        if(GT_OK != rc)
        {
            return rc;
        }

        ledClassParams.invertEnable =  GT_TRUE;
        ledClassParams.blinkEnable =  GT_FALSE ;
        ledClassParams.blinkSelect = CPSS_LED_BLINK_SELECT_0_E ;
        ledClassParams.forceEnable =  GT_FALSE ;
        ledClassParams.forceData =  GT_FALSE ;
        ledClassParams.pulseStretchEnable = GT_FALSE ;
        ledClassParams.disableOnLinkDown =  GT_FALSE ;
        rc = cpssDxChLedStreamClassManipulationSet(devNum, ledInterfaceNum , CPSS_DXCH_LED_PORT_TYPE_XG_E, 9, &ledClassParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        ledClassParams.invertEnable =  GT_FALSE;
        ledClassParams.blinkEnable =  GT_TRUE ;
        ledClassParams.blinkSelect = CPSS_LED_BLINK_SELECT_0_E ;
        ledClassParams.forceEnable =  GT_FALSE ;
        ledClassParams.forceData =  GT_FALSE ;
        ledClassParams.pulseStretchEnable = GT_FALSE ;
        ledClassParams.disableOnLinkDown =  GT_FALSE ;
        rc = cpssDxChLedStreamClassManipulationSet(devNum, ledInterfaceNum , CPSS_DXCH_LED_PORT_TYPE_XG_E, 10, &ledClassParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* For xCAT3 A1 and above */
        ledGroupParams.classA = 9;
        ledGroupParams.classB = 9;
        ledGroupParams.classC = 10;
        ledGroupParams.classD = 10;
        rc = cpssDxChLedStreamGroupConfigSet(devNum, ledInterfaceNum,
                 CPSS_DXCH_LED_PORT_TYPE_XG_E,
                 0, &ledGroupParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        ledGroupParams.classA = 9;
        ledGroupParams.classB = 9;
        ledGroupParams.classC = 10;
        ledGroupParams.classD = 10;
        rc = cpssDxChLedStreamGroupConfigSet(devNum, ledInterfaceNum,
                 CPSS_DXCH_LED_PORT_TYPE_XG_E,
                 1, &ledGroupParams);
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = cpssDxChLedStreamClassIndicationSet(devNum, ledInterfaceNum, 9, CPSS_DXCH_LED_INDICATION_LINK_E) ;
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = cpssDxChLedStreamClassIndicationSet(devNum, ledInterfaceNum, 10, CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E) ;
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

GT_STATUS wnc_xcat3LedInit(GT_VOID)
{
    GT_STATUS rc;

    if (xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P10 ||
		xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P48 ||
		xcat3BoardType == APP_DEMO_AC3BXH_WNC_OS6360P24)
    {
        cpssOsPrintf("Execute SFP LED initialization for OS6360.\n") ;
        if ((rc = ac3_LedInit_OS6360(0)) != GT_OK)
            return rc;
    }

    return GT_OK;
}
