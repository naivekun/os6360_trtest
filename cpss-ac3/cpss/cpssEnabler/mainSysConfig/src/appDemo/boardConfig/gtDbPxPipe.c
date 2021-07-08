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
* @file gtDbPxPipe.c
*
* @brief Initialization functions for the PX - PIPE board.
*
* @version   1
********************************************************************************
*/
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/userExit/userEventHandler.h>
#ifdef INCLUDE_UTF
#include <utf/utfMain.h>
#endif /*INCLUDE_UTF*/
#include <gtExtDrv/drivers/gtUartDrv.h>
#include <gtOs/gtOsExc.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/px/cpssHwInit/cpssPxHwInit.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/ingress/cpssPxIngressHash.h>
#include <cpss/px/cnc/cpssPxCnc.h>
#include <cpss/px/cpssHwInit/cpssPxHwInitLedCtrl.h>
#include <cpss/px/port/cpssPxPortTxTailDrop.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/px/port/cpssPxPortAp.h>
#ifdef REFERENCEHAL_BPE_EXISTS
#include <cpssPxHalBpe.h>
#endif
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpss/px/cpssHwInit/cpssPxTables.h>
static GT_U32   px_boardRevId;
static CPSS_PP_DEVICE_TYPE  px_devType;
static CPSS_PX_INIT_INFO_STC pxInitInfoDb;

#define PRINT_SKIP_DUE_TO_DB_FLAG(reasonPtr , flagNamePtr) \
        cpssOsPrintf("NOTE: '%s' skipped ! (due to flag '%s') \n",  \
            reasonPtr , flagNamePtr)

extern GT_BOOL systemInitialized;

GT_STATUS   cpssPxAllResourcesSet(
    IN GT_SW_DEV_NUM            devNum
);

#define PX_MAX_PORTS_NUM_CNS    17
#define DQ0_NUM_PORTS           10 /* ports 0..9 in DQ 0 */
#define Gbps_TO_Mbps    1000

#ifdef ASIC_SIMULATION
extern GT_U32 px_simUserDefinedCoreClockMHz;
extern GT_VOID simCoreClockOverwrite_px
(
    GT_U32 simUserDefinedCoreClockMHz
);
#endif
/**
* @internal appDemoPxInitInfoGet function
* @endinternal
*
* @brief   Gets parameters used during Pipe initialization.
*
* @param[out] initInfo                 - (pointer to) parameters used during Pipe initialization.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - bad pointer.
*/
GT_STATUS appDemoPxInitInfoGet
(
    IN CPSS_PX_INIT_INFO_STC    *initInfo
)
{
    if( NULL == initInfo )
    {
        return GT_BAD_PTR;
    }

    *initInfo = pxInitInfoDb;
    return GT_OK;
}

/**
* @internal px_boardTypePrint function
* @endinternal
*
* @brief   This function prints type of PX board.
*
* @param[in] boardName                - board name
* @param[in] devName                  - device name
*                                       none
*/
static GT_VOID px_boardTypePrint
(
    IN GT_CHAR  *boardName,
    IN GT_CHAR *devName
)
{
    GT_CHAR *environment;

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

    cpssOsPrintf("%s Board Type: %s [%s]\n", devName , boardName , environment);
}

/**
* @internal px_getBoardInfo function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @param[in] firstDev                 - GT_TRUE - first device, GT_FALSE - next device
*
* @param[out] hwInfoPtr                - HW info of found device.
* @param[out] pciInfo                  - PCI info of found device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of getBoardInfo.
*
*/
static GT_STATUS px_getBoardInfo
(
    IN  GT_BOOL          firstDev,
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO      *pciInfo
)
{
    GT_STATUS   rc;

    /* In our case we want to find just one prestera device on PCI bus*/
    rc = gtPresteraGetPciDev(firstDev, pciInfo);
    if (rc != GT_OK)
    {
        if (firstDev != GT_FALSE)
        {
            osPrintf("Could not recognize any device on PCI bus!\n");
        }
        return GT_NOT_FOUND;
    }

    rc = extDrvPexConfigure(
            pciInfo->pciBusNum,
            pciInfo->pciIdSel,
            pciInfo->funcNo,
            MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E,
            hwInfoPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("extDrvPexConfigure", rc);

    return rc;
}

/**
* @internal px_getBoardInfoSimple function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of getBoardInfo.
*
*/
static GT_STATUS px_getBoardInfoSimple
(
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO *pciInfo
)
{
    GT_STATUS   rc;

    rc = px_getBoardInfo(GT_TRUE/*firstDev*/, hwInfoPtr, pciInfo);
    if (rc != GT_OK) {return rc;}

    /* In our case we want to find just one prestera device on PCI bus*/
    rc = gtPresteraGetPciDev(GT_FALSE, pciInfo);
    if(rc == GT_OK)
    {
        osPrintf("More than one recognized device found on PCI bus!\n");
        return GT_INIT_ERROR;
    }

    return GT_OK;
}
/**
* @internal px_initFirstStage function
* @endinternal
*
*/
static GT_STATUS px_initFirstStage
(
    IN GT_SW_DEV_NUM       devNum,
    IN CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    GT_STATUS               rc;
    CPSS_PX_INIT_INFO_STC   initInfo;
    GT_U32  value;
    GT_U32  apEnable;

    osMemSet(&initInfo, 0, sizeof(initInfo));

    initInfo.hwInfo = hwInfoPtr[0];
    /* 8 Address Completion Region mode                        */
    initInfo.mngInterfaceType      = CPSS_CHANNEL_PEX_MBUS_E;
    /* Address Completion Region 1 - for Interrupt Handling (BY CPSS driver (current CPU)) */
    initInfo.isrAddrCompletionRegionsBmp = 0x02;
    /* Address Completion Regions 2,3,4,5 - for CPSS driver (current CPU) */
    initInfo.appAddrCompletionRegionsBmp = 0x3C;
    /* Address Completion Regions 6,7 - reserved for other CPU */
    initInfo.numOfDataIntegrityElements = CPSS_PX_SHADOW_TYPE_ALL_CPSS_CNS;
    initInfo.dataIntegrityShadowPtr = NULL;
    initInfo.phaFirmwareType = CPSS_PX_PHA_FIRMWARE_TYPE_DEFAULT_E;

    /* Init cpss with no HW writes */
    rc = appDemoDbEntryGet("initRegDefaults", &value);
    if(rc == GT_OK)
    {
        initInfo.allowHwAccessOnly = (GT_BOOL)value;
    }

    rc = appDemoDbEntryGet("apEnable", &apEnable);
    if(rc != GT_OK)
    {/* by default disabled */
        apEnable = 0;
    }
    rc = cpssPxPortApEnableSet(devNum, apEnable);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxPortApEnableSet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Check if user wants to load its own PHA firmware */
    rc = appDemoDbEntryGet("phaFirmwareType", &value);
    if(rc == GT_OK)
    {
        initInfo.phaFirmwareType = (CPSS_PX_PHA_FIRMWARE_TYPE_ENT)value;
    }

    /* Power supplies number */
    rc = appDemoDbEntryGet("powerSuppliesNumbers", &value);
    if(rc == GT_OK)
    {
        initInfo.powerSuppliesNumbers = (CPSS_PX_POWER_SUPPLIES_NUMBER_ENT)value;
    }

    /* devType is retrieved in cpssPxHwInit */
    rc = cpssPxHwInit(devNum,&initInfo, &px_devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxHwInit", rc);
    if (rc != GT_OK)
        return rc;

    pxInitInfoDb = initInfo;
    return GT_OK;
}


/**
* @internal px_appDemoDbUpdate function
* @endinternal
*
* @brief   Update the AppDemo DB (appDemoPpConfigList[devIndex]) about the device
*
* @param[in] devIndex                 - the device index
* @param[in] devNum                   - the SW devNum
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS px_appDemoDbUpdate
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum
)
{
    appDemoPpConfigList[devIndex].valid = GT_TRUE;
    appDemoPpConfigList[devIndex].devNum = CAST_SW_DEVNUM(devNum);
    appDemoPpConfigList[devIndex].deviceId = px_devType;
    appDemoPpConfigList[devIndex].devFamily = CPSS_PX_FAMILY_PIPE_E;
    appDemoPpConfigList[devIndex].apiSupportedBmp = APP_DEMO_PX_FUNCTIONS_SUPPORT_CNS;

    return GT_OK;
}

#define APP_INV_PORT_CNS ((GT_U32)(~0))

typedef enum
{
     PORT_LIST_TYPE_EMPTY = 0
    ,PORT_LIST_TYPE_INTERVAL
    ,PORT_LIST_TYPE_LIST
}PortListType_ENT;

typedef struct
{
    PortListType_ENT             entryType;
    GT_PHYSICAL_PORT_NUM         portList[20];  /* depending on list type */
                                                /* interval : 0 startPort     */
                                                /*            1 stopPort      */
                                                /*            2 step          */
                                                /*            3 APP_INV_PORT_CNS */
                                                /* list     : 0...x  ports     */
                                                /*            APP_INV_PORT_CNS */
    CPSS_PORT_SPEED_ENT          speed;
    CPSS_PORT_INTERFACE_MODE_ENT interfaceMode;
}PortInitList_STC;

typedef PortInitList_STC *PortInitList_STC_PTR;

static PortInitList_STC portInitlist_pipe1008[] =
{
     /* 8*10G ports */
     { PORT_LIST_TYPE_INTERVAL,  {4,11,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
     /* 2*25G ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,13,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E , CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

static PortInitList_STC portInitlist_pipe1012[] =
{
     /* 12*10G ports */
     { PORT_LIST_TYPE_INTERVAL,  {0, 11,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
     /* 4*25G ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,15,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E , CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

static PortInitList_STC portInitlist_pipe1022[] =
{
     /* 4*10G ports */
     { PORT_LIST_TYPE_INTERVAL,  {4,7,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR2_E}
     /* 2*25G ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {13,13,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E , CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {15,15,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E , CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

static PortInitList_STC portInitlist_pipe1024[] =
{
     /* 8*10G ports */
     { PORT_LIST_TYPE_INTERVAL,  {4,11,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR2_E}
     /* 4*25G ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,15,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E , CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

static PortInitList_STC portInitlist_pipe1016[] =
{
     /* 16*10G ports */
     { PORT_LIST_TYPE_INTERVAL,  {0,15,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

static PortInitList_STC portInitlist_pipe1015[] =
{
     /* 12*5G ports */
     { PORT_LIST_TYPE_INTERVAL,  {0, 11,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_5000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
     /* 4*25G ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,15,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E , CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};


static CPSS_PX_PORT_MAP_STC pipe1008Map[] =
{ /* Port,            mappingType                 , intefaceNum */

    {   4, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         4}
   ,{   5, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         5}
   ,{   6, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         6}
   ,{   7, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         7}
   ,{   8, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         8}
   ,{   9, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         9}
   ,{  10, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        10}
   ,{  11, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        11}

   ,{  12, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        12}
   ,{  13, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        13}

   /* CPU port - SDMA */
   ,{  16, CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E,         GT_PX_NA}
};

static CPSS_PX_PORT_MAP_STC pipe1012Map[] =
{ /* Port,            mappingType                 , intefaceNum */

    {   0, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         0}
   ,{   1, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         1}
   ,{   2, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         2}
   ,{   3, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         3}
   ,{   4, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         4}
   ,{   5, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         5}
   ,{   6, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         6}
   ,{   7, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         7}
   ,{   8, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         8}
   ,{   9, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         9}
   ,{  10, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        10}
   ,{  11, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        11}

   ,{  12, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        12}
   ,{  13, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        13}
   ,{  14, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        14}
   ,{  15, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        15}

   /* CPU port - SDMA */
   ,{  16, CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E,         GT_PX_NA}
};

static CPSS_PX_PORT_MAP_STC pipe1022Map[] =
{ /* Port,            mappingType                 , intefaceNum */

    {   4, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         4}
   ,{   5, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         5}
   ,{   6, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         6}
   ,{   7, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         7}

   ,{  13, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        13}
   ,{  15, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        15}

   /* CPU port - SDMA */
   ,{  16, CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E,         GT_PX_NA}
};

static CPSS_PX_PORT_MAP_STC pipe1024Map[] =
{ /* Port,            mappingType                 , intefaceNum */
    {   4, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         4}
   ,{   5, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         5}
   ,{   6, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         6}
   ,{   7, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         7}
   ,{   8, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         8}
   ,{   9, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,         9}
   ,{  10, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        10}
   ,{  11, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        11}

   ,{  12, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        12}
   ,{  13, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        13}
   ,{  14, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        14}
   ,{  15, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,        15}

   /* CPU port - SDMA */
   ,{  16, CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E,         GT_PX_NA}
};

static PortInitList_STC portInitlist_pipe_coreClockLimited_288[] =
{
     /* GearBox : 2*25G KR2 --> 2*25G KR ports */
     { PORT_LIST_TYPE_INTERVAL,  {0 , 3, 2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR2_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,13, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};


static PortInitList_STC portInitlist_pipe_coreClockLimited_350[] =
{
     /* GearBox : 4*25G KR2 --> 4*25G KR ports */
     { PORT_LIST_TYPE_INTERVAL,  {0 , 7, 2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR2_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,15, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

static PortInitList_STC *px_force_PortsInitListPtr = NULL;

static PortInitList_STC px_force_Ports_50Gx4_0_4_12_14_PortsInitList[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,0,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR4_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {4,4,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR4_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,12,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR2_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {14,14,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR2_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};
/* allow to test specific ports speed */
GT_STATUS   px_force_Ports_50Gx4_0_4_12_14(void)
{
    px_force_PortsInitListPtr = px_force_Ports_50Gx4_0_4_12_14_PortsInitList;

    return GT_OK;
}

/* NOTE: this mode expected to FAIL because the HGS uses 4 serdes per port */
static PortInitList_STC px_force_Ports_48HGS_0_4_12_14_PortsInitList[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,0,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {4,4,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,12,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {14,14,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

/* allow to test specific ports speed */
GT_STATUS   px_force_Ports_48HGS_0_4_12_14(void)
{
    px_force_PortsInitListPtr = px_force_Ports_48HGS_0_4_12_14_PortsInitList;

    return GT_OK;
}

/* NOTE: this mode expected to PASS (HGS uses 4 serdes per port) */
static PortInitList_STC px_force_Ports_48HGS_0_4_8_12_PortsInitList[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,0,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {4,4,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {8,8,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,12,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

/* allow to test specific ports speed */
GT_STATUS   px_force_Ports_48HGS_0_4_8_12(void)
{
    px_force_PortsInitListPtr = px_force_Ports_48HGS_0_4_8_12_PortsInitList;

    return GT_OK;
}


/* NOTE: this mode expected to PASS (HGS uses 4 serdes per port) */
static PortInitList_STC px_force_Ports_48HGS_0_4_12_PortsInitList[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,0,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {4,4,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,12,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

/* allow to test specific ports speed */
GT_STATUS   px_force_Ports_48HGS_0_4_12(void)
{
    px_force_PortsInitListPtr = px_force_Ports_48HGS_0_4_12_PortsInitList;

    return GT_OK;
}


static PortInitList_STC px_force_Ports_25G_12_13_14_15_PortsInitList[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {12,12,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {13,13,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {14,14,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {15,15,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

/* allow to test specific ports speed */
GT_STATUS   px_force_Ports_25G_12_13_14_15(void)
{
    px_force_PortsInitListPtr = px_force_Ports_25G_12_13_14_15_PortsInitList;

    return GT_OK;
}

/* NOTE: this mode expected to PASS (HGS uses 4 serdes per port) */
static PortInitList_STC px_force_Ports_48HGS_0_4_8_100G_12_PortsInitList[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,0,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {4,4,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {8,8,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_47200_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E}

    ,{ PORT_LIST_TYPE_INTERVAL,  {12,12,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

/* allow to test specific ports speed */
GT_STATUS   px_force_Ports_48HGS_0_4_8_100G_12(void)
{
    px_force_PortsInitListPtr = px_force_Ports_48HGS_0_4_8_100G_12_PortsInitList;

    return GT_OK;
}


static PortInitList_STC px_force_Ports_10G_0_to_9_25G_12_to_15_PortsInitList[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0 , 9,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,15,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

/* allow to test specific ports speed */
GT_STATUS   px_force_Ports_10G_0_to_9_25G_12_to_15(void)
{
    px_force_PortsInitListPtr = px_force_Ports_10G_0_to_9_25G_12_to_15_PortsInitList;

    return GT_OK;
}


/* allow to test remove the 'force' of specific ports speed */
GT_STATUS   px_unforce_Ports(void)
{
    px_force_PortsInitListPtr = NULL;

    return GT_OK;
}

static PortInitList_STC px_force_Ports_25G_0_2_25G_12_13_PortsInitList[] =
{
     /* GearBox : 2*25G KR2 --> 2*25G KR ports */
     { PORT_LIST_TYPE_INTERVAL,  {0 , 3, 2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR2_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,13, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

/* allow to test specific ports speed */
GT_STATUS   px_force_Ports_25G_0_2_25G_12_13(void)
{
    px_force_PortsInitListPtr = px_force_Ports_25G_0_2_25G_12_13_PortsInitList;

    return GT_OK;
}

static PortInitList_STC px_force_Ports_25G_0_2_4_6_25G_12_to_15_PortsInitList[] =
{
     /* GearBox : 4*25G KR2 --> 4*25G KR ports */
     { PORT_LIST_TYPE_INTERVAL,  {0 , 7, 2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR2_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,15, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

/* allow to test specific ports speed */
GT_STATUS   px_force_Ports_25G_0_2_4_6_25G_12_to_15(void)
{
    px_force_PortsInitListPtr = px_force_Ports_25G_0_2_4_6_25G_12_to_15_PortsInitList;

    return GT_OK;
}

static PortInitList_STC px_force_Ports_12_5G_0_to_7_11_to_15_ports_PortsInitList[] =
{
    /* 0..7 */
     { PORT_LIST_TYPE_INTERVAL,  {0 , 7, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_12500_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 12..15 */
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,15, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_12500_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

/* allow to test specific ports speed */
GT_STATUS   px_force_Ports_12_5G_0_to_7_11_to_15(void)
{
    px_force_PortsInitListPtr = px_force_Ports_12_5G_0_to_7_11_to_15_ports_PortsInitList;

    return GT_OK;
}

static PortInitList_STC px_force_Ports_50G_12_PortsInitList[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {12 ,12,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR2_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

/* check that 50G not supported by 288CC */
GT_STATUS   px_force_Ports_50G_12(void)
{
    px_force_PortsInitListPtr = px_force_Ports_50G_12_PortsInitList;

    return GT_OK;
}
static PortInitList_STC px_force_Ports_100G_12_PortsInitList[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {12 ,12,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};

/* check that 100G not supported by 350CC */
GT_STATUS   px_force_Ports_100G_12(void)
{
    px_force_PortsInitListPtr = px_force_Ports_100G_12_PortsInitList;

    return GT_OK;
}


typedef struct{
    GT_U32  coreClock;/* in MHz  */
    GT_U32  totalBW;  /* in Gbps */
    GT_U32  dq0BW;    /* in Gbps */
    GT_U32  dq1BW;    /* in Gbps */
}CORE_CLOCK_BW_INFO;

/* info need to be synch with CPSS : prv_pipe_devBW[] */
static CORE_CLOCK_BW_INFO pipeCoreClockInfoArr[] =
{
     /*coreClock,totalBW ,dq0BW ,dq1BW*/
     {  288     , 101    , 60   ,  60}
    ,{  350     , 205    , 105  , 101}
    ,{  450     , 260    , 150  , 150}
    ,{  500     , 260    , 160  , 160}
    /* must be last */
    ,{    0     ,   0    ,   0  ,   0}
};

extern GT_U32 CPSS_SPEED_ENM_2_Mbps
(
    CPSS_PORT_SPEED_ENT speedEnm
);
/* convert CPSS_PORT_INTERFACE_MODE_ENT to 'number of serdeses'*/
GT_U32 px_CPSS_INTERFACE_ENM_2_num_of_SERDESes
(
    CPSS_PORT_INTERFACE_MODE_ENT ifEnm
)
{
    typedef struct
    {
        CPSS_PORT_INTERFACE_MODE_ENT ifEnm;
        GT_U32                       numOfSerdeses   ;
    }APPDEMO_IF_2_num_of_SERDESes_STC;

    #define UNKNOWN_NUM_SERDESES     1

    APPDEMO_IF_2_num_of_SERDESes_STC prv_prvif2num_of_SERDESes[] =
    {
         {  CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E,       UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E,        UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_MII_PHY_E,             UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_MII_E,                 UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_GMII_E,                UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_SGMII_E,               UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_XGMII_E,               UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,         UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_MGMII_E,               UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_QSGMII_E,              UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_QX_E,                  UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_HX_E,                  UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_RXAUI_E,               UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,          UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,          UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_XLG_E,                 UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_HGL_E,                 UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_CHGL_12_E,             UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_KR_E,                  1}
        ,{  CPSS_PORT_INTERFACE_MODE_KR2_E,                 2}
        ,{  CPSS_PORT_INTERFACE_MODE_KR4_E,                 4}
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR_E,               1}
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR2_E,              2}
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR4_E,              4}
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN4_E,               UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN8_E,               UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN12_E,              UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN16_E,              UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN24_E,              UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_XHGS_E,                4}
        ,{  CPSS_PORT_INTERFACE_MODE_XHGS_SR_E ,            UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E, UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_KR_C_E,                UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_CR_C_E,                UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_KR2_C_E,               UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_CR2_C_E,               UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_CR_E,                  UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_CR2_E,                 UNKNOWN_NUM_SERDESES}
        ,{  CPSS_PORT_INTERFACE_MODE_CR4_E,                 UNKNOWN_NUM_SERDESES}
        /* must be last */
        ,{  CPSS_PORT_INTERFACE_MODE_NA_E,                  0   }
    };

    GT_U32 i;
    for (i = 0 ; prv_prvif2num_of_SERDESes[i].ifEnm != CPSS_PORT_INTERFACE_MODE_NA_E; i++)
    {
        if (prv_prvif2num_of_SERDESes[i].ifEnm == ifEnm)
        {
            return prv_prvif2num_of_SERDESes[i].numOfSerdeses;
        }
    }

    return 0;
}


/* add the BW that a port requires to DQ[0] or to DQ[1] and state the used SERDESes */
static GT_STATUS updateTotalBwNeeded
(
    IN GT_U32                       portNum,
    IN CPSS_PORT_SPEED_ENT          speed,
    IN CPSS_PORT_INTERFACE_MODE_ENT interfaceMode,
    INOUT GT_U32                    *dq0TotalPtr,
    INOUT GT_U32                    *dq1TotalPtr,
    INOUT GT_U32                    *usedSerdesesBmpPtr
)
{
    GT_U32 speedMbps   = CPSS_SPEED_ENM_2_Mbps(speed);
    GT_U32 numSerdeses = px_CPSS_INTERFACE_ENM_2_num_of_SERDESes(interfaceMode);
    GT_U32 ii;
    GT_U32  totalSpeedPerPort;

    if(numSerdeses == 0)
    {
        cpssOsPrintf("ERROR converting interfaceMode[%s] to number of SERDESes \n",
            CPSS_IF_2_STR(interfaceMode));

        return GT_NOT_SUPPORTED;
    }

    if(speedMbps == 0)
    {
        cpssOsPrintf("ERROR converting speed[%s] to Mbps\n",
            CPSS_SPEED_2_STR(speed));

        return GT_NOT_SUPPORTED;
    }

    totalSpeedPerPort = speedMbps / numSerdeses;

    /* round the number (to nearest Gbits) , because port of 12.5G use in TXQ-DQ
       slices as if 13G port used */
    totalSpeedPerPort = 1000 * ((totalSpeedPerPort + 500/*Mbps */) / 1000);


    for(ii = portNum ; ii < (portNum + numSerdeses) ; ii++)
    {
        if(ii < DQ0_NUM_PORTS)
        {
            *dq0TotalPtr += totalSpeedPerPort;
        }
        else
        {
            *dq1TotalPtr += totalSpeedPerPort;
        }

        if((*usedSerdesesBmpPtr) & (1 << ii))
        {
            /* the SERDES already in use !!! */
            cpssOsPrintf("ERROR : port[%d] speed[%s] interface[%s] (need[%d] SERDESes) try to use SERDES index [%d] that already in use \n",
                portNum,
                CPSS_SPEED_2_STR(speed),
                CPSS_IF_2_STR(interfaceMode),
                numSerdeses,
                ii);
            return GT_ALREADY_EXIST;
        }

        /* state that SERDES in use */
        *usedSerdesesBmpPtr |= (1 << ii);
    }

    return GT_OK;
}

/* check if the 'forced' port list expected to be OK in a specific core clock
   The function uses px_force_PortsInitListPtr that expected to be set prior to
   calling this function
   NOTE : running this function not require cpssInitSystem !!!
          (it can run after but also before)
*/
GT_STATUS   px_is_forced_port_list_ok(IN GT_U32    coreClock)
{
    GT_STATUS   rc = GT_OK;
    GT_U32  i;
    GT_U32 portIdx;
    GT_U32 maxPortIdx = 0;
    PortInitList_STC * portInitPtr;
    CORE_CLOCK_BW_INFO  *currCoreClockInfoPtr = NULL;
    GT_U32  portNum;
    GT_U32  dq0Total = 0;/* in Gbps */
    GT_U32  dq1Total = 0;/* in Gbps */
    GT_U32  totalBW;  /* in Gbps */
    GT_U32  usedSerdesesBmp = 0;/* bit map of used serdeses */

    cpssOsPrintf("Start Test coreClock[%d] \n",
        coreClock);

    if(px_force_PortsInitListPtr == NULL)
    {
        rc = GT_BAD_PTR;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("px_force_PortsInitListPtr == NULL (you must set 'forced list' before calling this function)",
            rc);
        return rc;
    }

    for(i = 0 ; pipeCoreClockInfoArr[i].coreClock != 0 ; i++ )
    {
        if(pipeCoreClockInfoArr[i].coreClock == coreClock)
        {
            currCoreClockInfoPtr = &pipeCoreClockInfoArr[i];
            break;
        }
    }

    if(currCoreClockInfoPtr == NULL)
    {
        /* not found */
        rc = GT_NOT_FOUND;
        cpssOsPrintf("Core clock [%d] is not valid \n",
            coreClock);
        cpssOsPrintf("Next are the valid Core clocks : ");
        for(i = 0 ; pipeCoreClockInfoArr[i].coreClock != 0 ; i++ )
        {
            cpssOsPrintf("%d",
                pipeCoreClockInfoArr[i].coreClock);
        }

        cpssOsPrintf("\n\n");

        CPSS_ENABLER_DBG_TRACE_RC_MAC("coreClock not found",
            rc);
        return rc;
    }

    portInitPtr = px_force_PortsInitListPtr;


    for (i = 0 ; portInitPtr->entryType != PORT_LIST_TYPE_EMPTY; i++,portInitPtr++)
    {
        switch (portInitPtr->entryType)
        {
            case PORT_LIST_TYPE_INTERVAL:
                for (portNum = portInitPtr->portList[0] ; portNum <= portInitPtr->portList[1]; portNum += portInitPtr->portList[2])
                {
                    if(maxPortIdx >= PX_MAX_PORTS_NUM_CNS)
                    {
                        rc = GT_OUT_OF_RANGE;
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("too much ports in list", rc);
                        return rc;
                    }

                    /* add the BW of the port to DQ0 and/or DQ1 */
                    rc = updateTotalBwNeeded(portNum,
                        portInitPtr->speed,
                        portInitPtr->interfaceMode,
                        &dq0Total,
                        &dq1Total,
                        &usedSerdesesBmp);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    maxPortIdx++;
                }
            break;
            case PORT_LIST_TYPE_LIST:
                for (portIdx = 0 ; portInitPtr->portList[portIdx] != APP_INV_PORT_CNS; portIdx++)
                {
                    portNum = portInitPtr->portList[portIdx];
                    if(maxPortIdx >= PX_MAX_PORTS_NUM_CNS)
                    {
                        rc = GT_OUT_OF_RANGE;
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("too much ports in list", rc);
                        return rc;
                    }

                    /* add the BW of the port to DQ0 and/or DQ1 */
                    rc = updateTotalBwNeeded(portNum,
                        portInitPtr->speed,
                        portInitPtr->interfaceMode,
                        &dq0Total,
                        &dq1Total,
                        &usedSerdesesBmp);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    maxPortIdx++;
                }
            break;
            default:
                {
                    return GT_NOT_SUPPORTED;
                }
        }
    }

    /* add 1000Mbps for CPU port */
    dq1Total += 1 * Gbps_TO_Mbps;

    /* check that total  BW not overflow
       check the DQ0,DQ1 BW not overflow */
    if((currCoreClockInfoPtr->dq0BW*Gbps_TO_Mbps) < dq0Total)
    {
        rc = GT_NO_RESOURCE;
        cpssOsPrintf("ERROR : max allowed dq0 BW is [%d] but ask to get [%d] \n",
            currCoreClockInfoPtr->dq0BW*Gbps_TO_Mbps,
            dq0Total);
    }
    else
    {
        cpssOsPrintf("GOOD : max allowed dq0 BW is [%d] ask to get [%d] \n",
            currCoreClockInfoPtr->dq0BW*Gbps_TO_Mbps,
            dq0Total);
    }

    if((currCoreClockInfoPtr->dq1BW*Gbps_TO_Mbps) < dq1Total)
    {
        rc = GT_NO_RESOURCE;
        cpssOsPrintf("ERROR : max allowed dq1 BW is [%d] but ask to get [%d] (with 1Gbps for CPU port)\n",
            currCoreClockInfoPtr->dq1BW*Gbps_TO_Mbps,
            dq1Total);
    }
    else
    {
        cpssOsPrintf("GOOD : max allowed dq1 BW is [%d] ask to get [%d] (with 1Gbps for CPU port)\n",
            currCoreClockInfoPtr->dq1BW*Gbps_TO_Mbps,
            dq1Total);

    }

    totalBW = dq0Total + dq1Total;

    if((currCoreClockInfoPtr->totalBW*Gbps_TO_Mbps) < totalBW)
    {
        rc = GT_NO_RESOURCE;
        cpssOsPrintf("ERROR : max allowed BW in the device is [%d] but ask to get [%d] (with 1Gbps for CPU port)\n",
            currCoreClockInfoPtr->totalBW*Gbps_TO_Mbps,
            totalBW);
    }
    else
    {
        cpssOsPrintf("GOOD : max allowed BW in the device is [%d] ask to get [%d] (with 1Gbps for CPU port)\n",
            currCoreClockInfoPtr->totalBW*Gbps_TO_Mbps,
            totalBW);
    }


    cpssOsPrintf("Ended Test coreClock[%d] \n",
        coreClock);

    return rc;
}


#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])


/**
* @internal px_initPortMappingStage function
* @endinternal
*
*/
static GT_STATUS px_initPortMappingStage
(
    IN GT_SW_DEV_NUM       devNum
)
{
    GT_STATUS   rc;
    CPSS_PX_PORT_MAP_STC *mapArrPtr;
    GT_U32 mapArrLen;
    GT_U32 devIdx;

    if (GT_OK != appDemoDevIdxGet(devNum, &devIdx))
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("devIdx not found", GT_FAIL);
        return GT_FAIL;
    }

    switch(appDemoPpConfigList[devIdx].deviceId)
    {
        case CPSS_98PX1008_CNS:
            ARR_PTR_AND_SIZE_MAC(pipe1008Map, mapArrPtr, mapArrLen);
            break;
        case CPSS_98PX1022_CNS:
            ARR_PTR_AND_SIZE_MAC(pipe1022Map, mapArrPtr, mapArrLen);
            break;
        case CPSS_98PX1024_CNS:
            ARR_PTR_AND_SIZE_MAC(pipe1024Map, mapArrPtr, mapArrLen);
            break;
        default:
            ARR_PTR_AND_SIZE_MAC(pipe1012Map, mapArrPtr, mapArrLen);
            break;
    }



    rc = cpssPxPortPhysicalPortMapSet(devNum, mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxPortPhysicalPortMapSet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}


#define PX_RX_DESC_NUM_DEF         200
#define PX_TX_DESC_NUM_DEF         1000
#define PX_RX_BUFF_SIZE_DEF        1548
#define PX_RX_BUFF_ALLIGN_DEF      1


/**
* @internal px_initNetIfParamsGet function
* @endinternal
*
*/
static GT_STATUS px_initNetIfParamsGet
(
    OUT CPSS_NET_IF_CFG_STC *netIfCfgPtr
)
{
    GT_STATUS   rc;
    GT_U32 rxDescSize,rxBufAlignment,txDescSize;
    GT_U32 rxDescNum,rxBufSize,rxBufAllign,txDescNum;
    GT_U32  ii;
    GT_VOID*    tmpPtr;

    rxDescNum   = PX_RX_DESC_NUM_DEF;
    rxBufSize   = PX_RX_BUFF_SIZE_DEF;
    rxBufAllign = PX_RX_BUFF_ALLIGN_DEF,
    txDescNum   = PX_TX_DESC_NUM_DEF;

    cpssOsMemSet(netIfCfgPtr,0,sizeof(*netIfCfgPtr));

    netIfCfgPtr->txDescBlock = NULL;
    netIfCfgPtr->rxDescBlock = NULL;
    netIfCfgPtr->rxBufInfo.buffData.staticAlloc.rxBufBlockPtr = NULL;
    netIfCfgPtr->rxBufInfo.allocMethod = CPSS_RX_BUFF_STATIC_ALLOC_E;

    /* Tx block size calc & malloc  */
    rc = cpssPxHwDescSizeGet(px_devType,
                            &rxDescSize,
                            &rxBufAlignment,
                            &txDescSize,
                            NULL);/*cncDescSizePtr*/
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxHwDescSizeGet",rc);
    if (GT_OK != rc)
        return rc;

    netIfCfgPtr->txDescBlockSize = txDescSize * txDescNum;
    netIfCfgPtr->txDescBlock =
        osCacheDmaMalloc(netIfCfgPtr->txDescBlockSize);
    if(netIfCfgPtr->txDescBlock == NULL)
    {
        goto exit_on_error;
    }

    /* Rx block size calc & malloc  */
    netIfCfgPtr->rxDescBlockSize = rxDescSize * rxDescNum;
    netIfCfgPtr->rxDescBlock =
        osCacheDmaMalloc(netIfCfgPtr->rxDescBlockSize);
    if(netIfCfgPtr->rxDescBlock == NULL)
    {
        goto exit_on_error;
    }

    /* init the Rx buffer allocation method */
    /* Set the system's Rx buffer size.     */
    if((rxBufSize % rxBufAllign) != 0)
    {
        rxBufSize = (rxBufSize + (rxBufAllign - (rxBufSize % rxBufAllign)));
    }

    if (netIfCfgPtr->rxBufInfo.allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E)
    {
        netIfCfgPtr->rxBufInfo.buffData.staticAlloc.rxBufBlockSize =
        rxBufSize * rxDescNum;

        /* status of RX buffers - cacheable or not, has been set in getPpPhase2ConfigSimple */
        /* If RX buffers should be cachable - allocate it from regular memory */
        if (GT_TRUE == netIfCfgPtr->rxBufInfo.buffersInCachedMem)
        {
            tmpPtr = osMalloc(((rxBufSize * rxDescNum) + rxBufAllign));
        }
        else
        {
            tmpPtr = osCacheDmaMalloc(((rxBufSize * rxDescNum) + rxBufAllign));
        }

        if(tmpPtr == NULL)
        {
            goto exit_on_error;
        }

        if((((GT_UINTPTR)tmpPtr) % rxBufAllign) != 0)
        {
            tmpPtr = (GT_U32*)(((GT_UINTPTR)tmpPtr) +
                               (rxBufAllign - (((GT_UINTPTR)tmpPtr) % rxBufAllign)));
        }
        netIfCfgPtr->rxBufInfo.buffData.staticAlloc.rxBufBlockPtr = tmpPtr;

        netIfCfgPtr->rxBufInfo.rxBufSize = rxBufSize;

        for(ii=0 ; ii < 4 ; ii++)
        {
            netIfCfgPtr->rxBufInfo.bufferPercentage[ii+0] = 13;
            netIfCfgPtr->rxBufInfo.bufferPercentage[ii+4] = 12;
        }

    }
    else if (netIfCfgPtr->rxBufInfo.allocMethod == CPSS_RX_BUFF_NO_ALLOC_E)
    {
        /* do not allocate rx buffers*/
    }
    else
    {
        /* dynamic RX buffer allocation currently is not supported by appDemo*/
        goto exit_on_error;
    }

    return GT_OK;

exit_on_error:
    if(netIfCfgPtr->txDescBlock)
    {
        osCacheDmaFree(netIfCfgPtr->txDescBlock);
    }

    if(netIfCfgPtr->rxDescBlock)
    {
        osCacheDmaFree(netIfCfgPtr->rxDescBlock);
    }

    if(netIfCfgPtr->rxBufInfo.buffData.staticAlloc.rxBufBlockPtr)
    {
        if(GT_TRUE == netIfCfgPtr->rxBufInfo.buffersInCachedMem)
        {
            osFree(netIfCfgPtr->rxBufInfo.buffData.staticAlloc.rxBufBlockPtr);
        }
        else
        {
            osCacheDmaFree(netIfCfgPtr->rxBufInfo.buffData.staticAlloc.rxBufBlockPtr);
        }
    }

    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_initNetIfParamsGet",GT_OUT_OF_CPU_MEM);

    return GT_OUT_OF_CPU_MEM;

}

/**
* @internal px_initNetIfStage function
* @endinternal
*
*/
static GT_STATUS px_initNetIfStage
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS   rc;
    CPSS_NET_IF_CFG_STC   netIfCfg;
    char dbEntryName[64];
    GT_U32  tmpData;

    osSprintf(dbEntryName,"noSdmaRxTxPP%d", devNum);
    if((appDemoDbEntryGet(dbEntryName, &tmpData) == GT_OK) && (tmpData != 0))
    {
        /* Disable Rx/Tx SDMA for selected devNum */

        PRINT_SKIP_DUE_TO_DB_FLAG("cpssPxNetIfInit","noSdmaRxTxPP");

        return GT_OK;
    }

    rc = px_initNetIfParamsGet(&netIfCfg);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_initNetIfParamsGet", rc);
    if (rc != GT_OK)
        return rc;

    rc = cpssPxNetIfInit(devNum, &netIfCfg);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxNetIfInit", rc);
    if (rc != GT_OK)
        return rc;

    return GT_OK;
}

#define PORT_SKIP_CHECK(dev,port)                                             \
    {                                                                         \
        GT_BOOL     isValid;                                                  \
        rc = cpssPxPortPhysicalPortMapIsValidGet(devNum, portNum , &isValid); \
        if(rc != GT_OK || isValid == GT_FALSE)                                \
        {                                                                     \
            continue;                                                         \
        }                                                                     \
    }

/**
* @internal px_initPortStage function
* @endinternal
*
*/
static GT_STATUS px_initPortStage
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS   rc;
    GT_PHYSICAL_PORT_NUM    portNum;

    /* Enable ports (not needed for CPU port) */
    for (portNum = 0; portNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; portNum++)
    {
        PORT_SKIP_CHECK(devNum,portNum);

        rc = cpssPxPortEnableSet(devNum, portNum, GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxPortEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }


    return GT_OK;
}


/* flag to allow optimization of the amount of times that 'PIZZA' configured
   for 'X' ports.
   assumption is that caller knows that no traffic should exists in the device
   while 'optimizing'
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfConfigure_optimizeSet(IN GT_BOOL optimize);

typedef struct
{
    GT_PHYSICAL_PORT_NUM         portNum;
    CPSS_PORT_SPEED_ENT          speed;
    CPSS_PORT_INTERFACE_MODE_ENT interfaceMode;
}PortInitInternal_STC;

static GT_STATUS px_appDemoPortListInit
(
    IN GT_SW_DEV_NUM      devNum,
    IN PortInitList_STC * portInitList,
    IN GT_BOOL            skipCheckEnable
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 portIdx;
    GT_U32 maxPortIdx;
    CPSS_PORTS_BMP_STC initPortsBmp,/* bitmap of ports to init */
                      *initPortsBmpPtr;/* pointer to bitmap */
    GT_PHYSICAL_PORT_NUM portNum;
    /* PortInitList_STC * portInitList; */
    PortInitList_STC * portInitPtr;
    GT_U32         coreClockHW;
    static PortInitInternal_STC    portList[PX_MAX_PORTS_NUM_CNS];
    static CPSS_PX_DETAILED_PORT_MAP_STC portMap;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    for (portIdx = 0 ; portIdx < sizeof(portList)/sizeof(portList[0]); portIdx++)
    {
        portList[portIdx].portNum       = APP_INV_PORT_CNS;
        portList[portIdx].speed         = CPSS_PORT_SPEED_NA_E;
        portList[portIdx].interfaceMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    }

    maxPortIdx = 0;
    portInitPtr = portInitList;
    for (i = 0 ; portInitPtr->entryType != PORT_LIST_TYPE_EMPTY; i++,portInitPtr++)
    {
        switch (portInitPtr->entryType)
        {
            case PORT_LIST_TYPE_INTERVAL:
                for (portNum = portInitPtr->portList[0] ; portNum <= portInitPtr->portList[1]; portNum += portInitPtr->portList[2])
                {
                    if (skipCheckEnable)
                    {
                        PORT_SKIP_CHECK(dev, portNum);
                    }
                    if(maxPortIdx >= PX_MAX_PORTS_NUM_CNS)
                    {
                        rc = GT_OUT_OF_RANGE;
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("too much ports in list", rc);
                        return rc;
                    }
                    portList[maxPortIdx].portNum       = portNum;
                    portList[maxPortIdx].speed         = portInitPtr->speed;
                    portList[maxPortIdx].interfaceMode = portInitPtr->interfaceMode;
                    maxPortIdx++;
                }
            break;
            case PORT_LIST_TYPE_LIST:
                for (portIdx = 0 ; portInitPtr->portList[portIdx] != APP_INV_PORT_CNS; portIdx++)
                {
                    portNum = portInitPtr->portList[portIdx];
                    if (skipCheckEnable)
                    {
                        PORT_SKIP_CHECK(dev, portNum);
                    }
                    if(maxPortIdx >= PX_MAX_PORTS_NUM_CNS)
                    {
                        rc = GT_OUT_OF_RANGE;
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("too much ports in list", rc);
                        return rc;
                    }
                    portList[maxPortIdx].portNum       = portNum;
                    portList[maxPortIdx].speed         = portInitPtr->speed;
                    portList[maxPortIdx].interfaceMode = portInitPtr->interfaceMode;
                    maxPortIdx++;
                }
            break;
            default:
                {
                    return GT_NOT_SUPPORTED;
                }
        }
    }

    rc = cpssPxHwCoreClockGet(devNum,/*OUT*/&coreClockHW);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n device %d Clock %d MHz", devNum, coreClockHW);
    cpssOsPrintf("\n+----+------+-------+--------------+-----------------+-------------+");
    cpssOsPrintf("\n| #  | Port | Speed |      IF      |   mapping Type  | dma mac txq |");
    cpssOsPrintf("\n+----+------+-------+--------------+-----------------+-------------+");

    initPortsBmpPtr = &initPortsBmp;

    for (portIdx = 0 ; portIdx < maxPortIdx; portIdx++)
    {
        rc = cpssPxPortPhysicalPortDetailedMapGet(devNum,portList[portIdx].portNum,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        cpssOsPrintf("\n| %2d | %4d | %s | %s |",portIdx,
                                              portList[portIdx].portNum,
                                              CPSS_SPEED_2_STR(portList[portIdx].speed),
                                              CPSS_IF_2_STR(portList[portIdx].interfaceMode));
        cpssOsPrintf(" %-15s | %2d  %2d  %2d  |"
                                            ,portMapPtr->portMap.mappingType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E ? "CPU-SDMA" : "ETHERNET"
                                            ,portMapPtr->portMap.dmaNum
                                            ,portMapPtr->portMap.macNum
                                            ,portMapPtr->portMap.txqNum);

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(initPortsBmpPtr);
        CPSS_PORTS_BMP_PORT_SET_MAC(initPortsBmpPtr,portList[portIdx].portNum);

        if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {
            if(cpssDeviceRunCheck_onEmulator())
            {
                if(portIdx == 0)
                {
                    /* reduce the number of HW calls */
                    (void)prvCpssPxPortDynamicPizzaArbiterIfConfigure_optimizeSet(GT_TRUE/*optimize*/);
                }
                else
                if(portIdx == (maxPortIdx-1))
                {
                    /* restore to default .. allow 'last port' to set the PIZZA for ALL ports !!!*/
                    (void)prvCpssPxPortDynamicPizzaArbiterIfConfigure_optimizeSet(GT_FALSE/* STOP the optimization */);
                }
            }



            rc = cpssPxPortModeSpeedSet(devNum, initPortsBmpPtr, GT_TRUE,
                                        portList[portIdx].interfaceMode,
                                        portList[portIdx].speed);
            if(GT_OK != rc)
            {
                return rc;
            }

            if(rc != GT_OK)
            {
                cpssOsPrintf("\n--> ERROR : cpssPxPortModeSpeedSet(portNum=%d, ifMode=%d, speed=%d) :rc=%d\n",
                portList[portIdx].portNum, portList[portIdx].interfaceMode, portList[portIdx].speed, rc);
                return rc;
            }
        }
    }

    cpssOsPrintf("\n+----+------+-------+--------------+-----------------+-------------+");
    cpssOsPrintf("\n PORT INTERFACE Init Done. \n");
    return GT_OK;
}

/**
* @internal px_deviceEnableStage function
* @endinternal
*
*/
static GT_STATUS px_deviceEnableStage
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS   rc;

    /* Enable device */
    rc = cpssPxCfgDevEnable(devNum, GT_TRUE);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxCfgDevEnable", rc);
    if (rc != GT_OK)
        return rc;

    return GT_OK;
}

/**
* @internal px_forwardingTablesStage function
* @endinternal
*
*/
GT_STATUS px_forwardingTablesStage
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS   rc;
    GT_U32  fullPortsBmp;
    GT_U32  fullPortsBmp_excludeMe;
    GT_U32  fullPortsBmp_withCpu;
    GT_U32  entryIndex;
    GT_U32  PTS_Src_Idx_Constant;
    GT_U32  portNum;
    GT_U32  srcIndexFactor = 0x10;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC portKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT tableType;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormat;

    fullPortsBmp = 0;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));
    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    for (portNum = 0; portNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; portNum++)
    {
        PORT_SKIP_CHECK(devNum,portNum);
        fullPortsBmp |= 1<<portNum;
    }

    /* with the CPU port */
    fullPortsBmp_withCpu = fullPortsBmp | (1 << PRV_CPSS_PX_CPU_DMA_NUM_CNS);

    /* set each port with different profile , to allow src filtering */
    for (portNum = 0; portNum < (PRV_CPSS_PX_CPU_DMA_NUM_CNS+1); portNum++)
    {
        if(portNum != PRV_CPSS_PX_CPU_DMA_NUM_CNS)
        {
            PORT_SKIP_CHECK(devNum,portNum);
        }

        portKey.srcPortProfile = portNum;
        rc = cpssPxIngressPortPacketTypeKeySet(devNum, portNum, &portKey);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortPacketTypeKeySet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /* set that each 'packet type' will match the src port profile */
    keyMask.profileIndex = 0x7F;
    for(entryIndex = 0 ; entryIndex < 32; entryIndex++)/*pipe_PktTypeIdx*/
    {
        keyData.profileIndex = entryIndex;

        rc = cpssPxIngressPacketTypeKeyEntrySet(devNum, entryIndex, &keyData, &keyMask);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntrySet", rc);
        if (rc != GT_OK)
            return rc;


        /* validate the entry to allow match ... otherwise we get DROP */
        rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, entryIndex, GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntryEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /* set that each 'packet type' will have it's base index in the 'destination table'
        with 'all ports'  */
    tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E;
    packetTypeFormat.indexMax = BIT_12-1;/*12 bits*/
    for(entryIndex = 0 ; entryIndex < 32; entryIndex++)/*pipe_PktTypeIdx*/
    {
        rc = cpssPxIngressPortMapEntrySet(devNum, tableType, entryIndex, fullPortsBmp_withCpu, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapEntrySet(Destination table)", rc);
        if (rc != GT_OK)
            return rc;

        packetTypeFormat.indexConst = entryIndex;/* each packet type hold different destination index */

        rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum, tableType, entryIndex, &packetTypeFormat);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapPacketTypeFormatEntrySet(Destination table)", rc);
        if (rc != GT_OK)
            return rc;
    }

    tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E;
    packetTypeFormat.indexMax = BIT_11-1;/*11 bits*/
    for(entryIndex = 0 ; entryIndex < 32; entryIndex++)/*pipe_PktTypeIdx*/
    {
        PTS_Src_Idx_Constant = entryIndex * srcIndexFactor;
        packetTypeFormat.indexConst = PTS_Src_Idx_Constant;
        rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum, tableType, entryIndex, &packetTypeFormat);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapPacketTypeFormatEntrySet(Source table)", rc);
        if (rc != GT_OK)
            return rc;

        fullPortsBmp_excludeMe = fullPortsBmp & ~(1<<entryIndex);
        /* SRC_BMP : do src filtering based on src_profile */
        rc = cpssPxIngressPortMapEntrySet(devNum, tableType, PTS_Src_Idx_Constant, fullPortsBmp_excludeMe, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapEntrySet(Source table)", rc);
        if (rc != GT_OK)
            return rc;

        if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
        {
            rc = cpssPxIngressFilteringEnableSet(devNum, CPSS_DIRECTION_EGRESS_E,
                entryIndex, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressFilteringEnableSet", rc);
            if (rc != GT_OK)
                return rc;
        }
    }

    /* designated port : no trunk members */
    for(entryIndex = 0 ; entryIndex < 128 ; entryIndex++)
    {
        rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum,entryIndex,fullPortsBmp_withCpu);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressHashDesignatedPortsEntrySet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /* disable forwarding to all ports. Event handler will enable forwarding for
       connected ports with link UP. Leave CPU port to be as-is. */
    for (portNum = 0; portNum < PRV_CPSS_PX_CPU_DMA_NUM_CNS; portNum++)
    {
        rc = cpssPxIngressPortTargetEnableSet(devNum, portNum, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortTargetEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    return GT_OK;
}

/**
* @internal px_initCnc function
* @endinternal
*
*/
static GT_STATUS px_initCnc
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS rc;
    CPSS_DMA_QUEUE_CFG_STC mem; /* DMA host memory descriptor */
    mem.dmaDescBlockSize = (1024 /* counters per block */ * 2/*blocks*/ *
        2/*words per counter*/ * 4/*bytes per words*/ ) +
        15/* spare bytes for case when alignment is not 16 */;
    mem.dmaDescBlock = osCacheDmaMalloc(mem.dmaDescBlockSize);

    if (0 == mem.dmaDescBlock)
    {
        return GT_OUT_OF_CPU_MEM;
    }
    rc = cpssPxCncUploadInit(devNum, &mem);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxCncUploadInit", rc);
    if(GT_OK != rc)
    {
        osCacheDmaFree(mem.dmaDescBlock);
    }
    return rc;
}

/**
* @internal px_initPortTx function
* @endinternal
*
*/
static GT_STATUS px_initPortTx
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS   rc;
    GT_U32      tc;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    profileTcParam;

    /* DBA mode is enabled by default */

    /* Set amount of buffers[7552] available for dynamic allocation */
    rc = cpssPxPortTxTailDropDbaAvailableBuffSet(devNum, 7552);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxPortTxTailDropDbaAvailableBuffSet", rc);

    /* go over all profiles */
    for (profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
         profile <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E;
         profile++)
    {
        /* Set portMaxDescLimit = portMaxBuffLimit[8192] and portAlpha[0] */
        rc = cpssPxPortTxTailDropProfileSet(devNum, profile, 8192, 8192,
                                            CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxPortTxTailDropProfileSet", rc);

        /* go over all Traffic CLasses */
        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            /* Set alpha[ratio 1.0], MaxDescLimit [8192] and amount of buffers[0] per Profile,
             Traffic Class and Drop Precedence                           */
            rc = cpssPxPortTxTailDropProfileTcGet(devNum, profile, tc,
                    &profileTcParam);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxPortTxTailDropProfileTcSet", rc);

            profileTcParam.dp0MaxBuffNum = 0;
            profileTcParam.dp0QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
            profileTcParam.dp1MaxBuffNum = 0;
            profileTcParam.dp1QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
            profileTcParam.dp2MaxBuffNum = 0;
            profileTcParam.dp2QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
            profileTcParam.dp0MaxDescNum =
            profileTcParam.dp1MaxDescNum =
            profileTcParam.dp2MaxDescNum = 8192;

            rc = cpssPxPortTxTailDropProfileTcSet(devNum, profile, tc,
                    &profileTcParam);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxPortTxTailDropProfileTcSet", rc);
        }
    }

    return rc;
}

/**
* @internal px_initLed function
* @endinternal
*
*/
static GT_STATUS px_initLed
(
    IN GT_U32             boardIdx,
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS rc;
    GT_U32 ii;
    CPSS_LED_GROUP_CONF_STC groupParams;
    CPSS_PX_LED_CLASS_MANIPULATION_STC classParams;
    CPSS_PX_LED_CONF_STC ledConf;
    GT_U32 portMac;

    for(ii = 0; ii < 6; ii++)
    {
        rc = cpssPxLedStreamClassManipulationGet(devNum, ii, &classParams);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxLedStreamClassManipulationGet", rc);
        if( rc != GT_OK )
        {
            return rc;
        }

        if (boardIdx == 34)
        {
            /* RD board */
            if(2 != ii)
            {
                continue;
            }

            classParams.blinkEnable = GT_TRUE;
            classParams.blinkSelect = CPSS_LED_BLINK_SELECT_0_E;
            classParams.forceEnable = GT_FALSE;
            classParams.forceData   = 0;
            classParams.pulseStretchEnable  = GT_TRUE;
            classParams.disableOnLinkDown   = GT_TRUE;
        }
        else
        {
            if(2 == ii)
            {
                classParams.blinkEnable = GT_TRUE;
            }
            else
            {
                classParams.blinkEnable = GT_FALSE;
            }
            classParams.pulseStretchEnable = GT_FALSE;
        }

        rc = cpssPxLedStreamClassManipulationSet(devNum, ii, &classParams);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxLedStreamClassManipulationSet", rc);
        if( rc != GT_OK )
        {
            return rc;
        }
    }

    rc = cpssPxLedStreamConfigGet(devNum, &ledConf);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxLedStreamConfigGet", rc);
    if( rc != GT_OK )
    {
        return rc;
    }

    if (boardIdx == 34)
    {
        /* RD board */
        ledConf.ledOrganize       = CPSS_LED_ORDER_MODE_BY_CLASS_E;
        ledConf.blink0DutyCycle   = CPSS_LED_BLINK_DUTY_CYCLE_2_E;
        ledConf.blink0Duration    = CPSS_LED_BLINK_DURATION_2_E;
        ledConf.blink1DutyCycle   = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
        ledConf.blink1Duration    = CPSS_LED_BLINK_DURATION_0_E;
        ledConf.pulseStretch      = CPSS_LED_PULSE_STRETCH_1_E;
        ledConf.ledStart          = 64;
        ledConf.ledEnd            = 79;
        ledConf.invertEnable      = GT_FALSE;
        ledConf.ledClockFrequency = CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E;
    }
    else
    {
        /* DB board */
        ledConf.blink0Duration = CPSS_LED_BLINK_DURATION_5_E;
        ledConf.ledOrganize = CPSS_LED_ORDER_MODE_BY_PORT_E;
    }

    rc = cpssPxLedStreamConfigSet(devNum, &ledConf);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxLedStreamConfigSet", rc);
    if( rc != GT_OK )
    {
        return rc;
    }

    groupParams.classA = 1;
    groupParams.classB = 1;
    groupParams.classC = 2;
    groupParams.classD = 2;
    rc = cpssPxLedStreamGroupConfigSet(devNum, 0, &groupParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxLedStreamGroupConfigSet", rc);
    if( rc != GT_OK )
    {
        return rc;
    }
    rc = cpssPxLedStreamGroupConfigSet(devNum, 1, &groupParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxLedStreamGroupConfigSet", rc);
    if( rc != GT_OK )
    {
        return rc;
    }

    for (ii = 0; ii < PX_MAX_PORTS_NUM_CNS - 1; ii++)
    {
        rc = prvCpssPxPortPhysicalPortMapCheckAndConvert(devNum, ii,
            PRV_CPSS_PX_PORT_TYPE_MAC_E, &portMac);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssPxPortPhysicalPortMapCheckAndConvert", rc);
        if(GT_OK != rc)
        {
            continue;
        }

        rc = cpssPxLedStreamPortPositionSet(devNum, ii, ii);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxLedStreamPortPositionSet", rc);
        if( rc != GT_OK )
        {
            return rc;
        }

        if (boardIdx == 34)
        {
            rc = cpssPxLedStreamPortClassPolarityInvertEnableSet(devNum, ii, 2, GT_FALSE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxLedStreamPortClassPolarityInvertEnableSet", rc);
            if( rc != GT_OK )
            {
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal cpssInitSystemGet function
* @endinternal
*
*/
extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

/* RD board polarity database */
static APPDEMO_SERDES_LANE_POLARITY_STC  pxAppDemoSerdesPolarityRDArr[4][16] =
{
    /* laneNum  invertTx    invertRx */
    {
        { 0,    GT_TRUE,    GT_TRUE  },
        { 1,    GT_FALSE,   GT_TRUE  },
        { 2,    GT_TRUE,    GT_TRUE  },
        { 3,    GT_FALSE,   GT_TRUE  },
        { 4,    GT_TRUE,    GT_TRUE  },
        { 5,    GT_FALSE,   GT_TRUE  },
        { 6,    GT_TRUE,    GT_TRUE  },
        { 7,    GT_FALSE,   GT_TRUE  },
        { 8,    GT_TRUE,    GT_TRUE  },
        { 9,    GT_FALSE,   GT_TRUE  },
        { 10,   GT_TRUE,    GT_TRUE  },
        { 11,   GT_FALSE,   GT_TRUE  },
        { 12,   GT_TRUE,    GT_FALSE },
        { 13,   GT_TRUE,    GT_FALSE },
        { 14,   GT_FALSE,   GT_FALSE },
        { 15,   GT_TRUE,    GT_FALSE }
    },

    {
        { 0,    GT_TRUE,    GT_TRUE  },
        { 1,    GT_FALSE,   GT_TRUE  },
        { 2,    GT_TRUE,    GT_TRUE  },
        { 3,    GT_FALSE,   GT_TRUE  },
        { 4,    GT_TRUE,    GT_TRUE  },
        { 5,    GT_FALSE,   GT_TRUE  },
        { 6,    GT_TRUE,    GT_TRUE  },
        { 7,    GT_FALSE,   GT_TRUE  },
        { 8,    GT_TRUE,    GT_TRUE  },
        { 9,    GT_FALSE,   GT_TRUE  },
        { 10,   GT_TRUE,    GT_TRUE  },
        { 11,   GT_FALSE,   GT_TRUE  },
        { 12,   GT_FALSE,   GT_FALSE },
        { 13,   GT_FALSE,   GT_FALSE },
        { 14,   GT_FALSE,   GT_FALSE },
        { 15,   GT_FALSE,   GT_FALSE }
    },

    {
        { 0,    GT_TRUE,    GT_TRUE  },
        { 1,    GT_FALSE,   GT_TRUE  },
        { 2,    GT_TRUE,    GT_TRUE  },
        { 3,    GT_FALSE,   GT_TRUE  },
        { 4,    GT_TRUE,    GT_TRUE  },
        { 5,    GT_FALSE,   GT_TRUE  },
        { 6,    GT_TRUE,    GT_TRUE  },
        { 7,    GT_FALSE,   GT_TRUE  },
        { 8,    GT_TRUE,    GT_TRUE  },
        { 9,    GT_FALSE,   GT_TRUE  },
        { 10,   GT_TRUE,    GT_TRUE  },
        { 11,   GT_FALSE,   GT_TRUE  },
        { 12,   GT_FALSE,   GT_FALSE },
        { 13,   GT_FALSE,   GT_FALSE },
        { 14,   GT_FALSE,   GT_FALSE },
        { 15,   GT_FALSE,   GT_FALSE }
    },

    {
        { 0,    GT_TRUE,    GT_TRUE  },
        { 1,    GT_FALSE,   GT_TRUE  },
        { 2,    GT_TRUE,    GT_TRUE  },
        { 3,    GT_FALSE,   GT_TRUE  },
        { 4,    GT_TRUE,    GT_TRUE  },
        { 5,    GT_FALSE,   GT_TRUE  },
        { 6,    GT_TRUE,    GT_TRUE  },
        { 7,    GT_FALSE,   GT_TRUE  },
        { 8,    GT_TRUE,    GT_TRUE  },
        { 9,    GT_FALSE,   GT_TRUE  },
        { 10,   GT_TRUE,    GT_TRUE  },
        { 11,   GT_FALSE,   GT_TRUE  },
        { 12,   GT_FALSE,   GT_TRUE  },
        { 13,   GT_FALSE,   GT_FALSE },
        { 14,   GT_FALSE,   GT_FALSE },
        { 15,   GT_FALSE,   GT_FALSE }
    }
};

static GT_STATUS px_appDemoPortListPolaritySet
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS rc;
    GT_U32 laneNum;
    GT_U32 lanesArrSize;

    lanesArrSize = sizeof(pxAppDemoSerdesPolarityRDArr[0]) / sizeof(pxAppDemoSerdesPolarityRDArr[0][0]);

    for(laneNum = 0; laneNum < lanesArrSize; laneNum++)
    {
        rc = cpssPxPortSerdesLanePolaritySet(devNum, 0,
                                             pxAppDemoSerdesPolarityRDArr[devNum][laneNum].laneNum,
                                             pxAppDemoSerdesPolarityRDArr[devNum][laneNum].invertTx,
                                             pxAppDemoSerdesPolarityRDArr[devNum][laneNum].invertRx);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxPortSerdesLanePolaritySet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal px_appDemoInitSequence function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for PX Pipe device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
GT_STATUS px_appDemoInitSequence
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_HW_INFO_STC   *hwInfoPtr,
    IN PortInitList_STC   *portInitListPtr
)
{
    GT_STATUS   rc;
    GT_U32      initSerdesDefaults;
    GT_U32      devType;
    GT_U32      boardIdx;
    GT_U32      boardRevId;
    GT_U32      reloadEeprom;
    GT_U32  coreClockHW;

    GT_U32     value;
    GT_BOOL    allowHwAccessOnly = GT_FALSE;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    GT_BOOL     portMgr;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Init cpss with no HW writes */
    rc = appDemoDbEntryGet("initRegDefaults", &value);
    if(rc == GT_OK)
    {
        allowHwAccessOnly = (GT_BOOL)value;
    }

    if(px_force_PortsInitListPtr)
    {
        /* allow to 'FORCE' specific array by other logic */
        portInitListPtr = px_force_PortsInitListPtr;
    }


    /* Set debug devType if needed */
    rc = appDemoDebugDeviceIdGet(devNum, &devType);
    if (GT_OK == rc)
    {
        rc = prvCpssDrvDebugDeviceIdSet(devNum, devType);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDrvDebugDeviceIdSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = px_initFirstStage(devNum,hwInfoPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_initFirstStage", rc);
    if (GT_OK != rc)
        return rc;

    /* update the appDemo DB */
    rc = px_appDemoDbUpdate(devIndex, devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_appDemoDbUpdate", rc);
    if (rc != GT_OK)
        return rc;

    /* initialize only cpssDriver for HW access, skip the rest */
    if (allowHwAccessOnly == GT_TRUE)
    {
        CPSS_LOG_INFORMATION_MAC("Finished to initialize only cpssDriver for HW access, skip the rest");
        return GT_OK;
    }

    if (portInitListPtr == NULL)
    {
        switch(appDemoPpConfigList[devIndex].deviceId)
        {
            case CPSS_98PX1008_CNS:
                portInitListPtr = portInitlist_pipe1008;
                break;
            case CPSS_98PX1022_CNS:
                portInitListPtr = portInitlist_pipe1022;
                break;
            case CPSS_98PX1024_CNS:
                portInitListPtr = portInitlist_pipe1024;
                break;
            case CPSS_98PX1016_CNS:
                portInitListPtr = portInitlist_pipe1016;
                break;
            case CPSS_98PX1015_CNS:
                portInitListPtr = portInitlist_pipe1015;
                break;
            default:
                portInitListPtr = portInitlist_pipe1012;
                break;
        }

        rc = cpssPxHwCoreClockGet(devNum,/*OUT*/&coreClockHW);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(coreClockHW == 288)
        {
            /* we need to use limited ports with speeds */
            portInitListPtr = portInitlist_pipe_coreClockLimited_288;
        }
        else
        if(coreClockHW == 350)
        {
            portInitListPtr = portInitlist_pipe_coreClockLimited_350;
        }
    }

    rc = px_initNetIfStage(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_initNetIfStage", rc);
    if (GT_OK != rc)
        return rc;

    rc = px_initPortMappingStage(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_initPortMappingStage", rc);
    if (GT_OK != rc)
        return rc;

    /* check if user wants to init ports to default values */
    rc = appDemoDbEntryGet("initSerdesDefaults", &initSerdesDefaults);
    if(rc != GT_OK)
    {
        initSerdesDefaults = 1;
    }

    if (appDemoDbEntryGet("portMgr", &value) != GT_OK)
    {
        portMgr = GT_FALSE;
    }
    else
    {
        portMgr = (value == 1) ? GT_TRUE : GT_FALSE;
    }

    /* get type of the board */
    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    rc = px_initLed(boardIdx, devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_initLed", rc);
    if (GT_OK != rc)
        return rc;

    if(0 != initSerdesDefaults)
    {
        /* init speeds on the ports */
        rc = px_appDemoPortListInit(devNum,portInitListPtr,GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("px_appDemoPortListInit", rc);
        if (rc != GT_OK)
            return rc;
    }

    if(boardIdx == 34)
    {
        /* For RD board (4 devices) SERDES polarity should be configured */
        rc = px_appDemoPortListPolaritySet(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("px_appDemoPortListPolaritySet", rc);
        if (rc != GT_OK)
            return rc;
    }

    if (!portMgr)
    {
        /* enable the ports after setting the 'CG mac' as one */
        rc = px_initPortStage(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("px_initPortStage", rc);
        if (GT_OK != rc)
            return rc;
    }

    rc = px_deviceEnableStage(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_deviceEnableStage", rc);
    if (GT_OK != rc)
        return rc;

    if ( system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E )
    {
        rc = px_forwardingTablesStage(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("px_forwardingTablesStage", rc);
        if (GT_OK != rc)
            return rc;
    }

    rc = px_initCnc(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_initCnc", rc);
    if (GT_OK != rc)
        return rc;

    rc = px_initPortTx(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_initPortTx", rc);
    if (GT_OK != rc)
        return rc;

    rc = cpssDevSupportSystemResetSet(devNum, GT_TRUE);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevSupportSystemResetSet", rc);
    if (GT_OK != rc)
        return rc;

    return GT_OK;
}

/**
* @internal px_EventHandlerInit function
* @endinternal
*
*/
static GT_STATUS px_EventHandlerInit
(
    void
)
{
    GT_STATUS   rc;
    GT_U32 value;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    /* appDemoEventHandlerPreInit:
        needed to be called before:
        boardCfgFuncs.boardAfterInitConfig(...)
        and before:
        appDemoEventRequestDrvnModeInit()
        */
    rc = appDemoEventHandlerPreInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventHandlerPreInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }


    if (GT_OK == appDemoDbEntryGet("initSystemWithoutInterrupts", &value) && value)
    {
        /* we not want interrupts handling */
        PRINT_SKIP_DUE_TO_DB_FLAG("event handling","initSystemWithoutInterrupts");
    }
    else if ( system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E )
    {
        rc = appDemoEventRequestDrvnModeInit();
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventRequestDrvnModeInit", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}
#ifdef REFERENCEHAL_BPE_EXISTS
/**
* @internal CMDFS_RAM_INIT_EMBEDDED_REFHAL function
* @endinternal
*
*/
void CMDFS_RAM_INIT_EMBEDDED_REFHAL(void);
void utfSuit_cpssPxHalBpe(void);
void utfSuit_cpssPxTgfHalBpe(void);
static GT_STATUS px_HAL_init(IN GT_SW_DEV_NUM      devNum)
{
    GT_STATUS rc;

    rc = cpssPxHalBpeInit(devNum);
    if(rc != GT_OK) {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxHalBpeInit", rc);
        return rc;
    }
#ifdef INCLUDE_UTF
    utfDeclareTestType(UTF_TEST_TYPE_PX_E);
    utfSuit_cpssPxHalBpe();
    utfDeclareTestType(UTF_TEST_TYPE_PX_TRAFFIC_E);
    utfSuit_cpssPxTgfHalBpe();
#endif
#ifdef CMD_LUA_CLI
    /* add refereceHal Lua scripts */
    CMDFS_RAM_INIT_EMBEDDED_REFHAL();
#endif

    return rc;
}
static GT_VOID px_HAL_init_LUA()
{
#ifdef CMD_LUA_CLI
    /* add refereceHal Lua scripts */
    CMDFS_RAM_INIT_EMBEDDED_REFHAL();
#endif

}
#endif

#ifdef EMBEDDED_COMMANDS_EXISTS
void CMDFS_RAM_INIT_EMBEDDED_EMBCOM(void);
void cpssEcLuaInit (void);

static GT_VOID px_EMBCOM_init_LUA()
{
#ifdef CMD_LUA_CLI
    /* add refereceHal Lua scripts */
    cpssEcLuaInit();
#endif

}
#endif
/**
* @internal px_localUtfInit function
* @endinternal
*
*/
static GT_STATUS px_localUtfInit
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS rc = GT_OK;

#ifdef INCLUDE_UTF
    /* Initialize unit tests for CPSS */
    rc = utfPreInitPhase();
    if (rc != GT_OK)
    {
        utfPostInitPhase(rc);
        return rc;
    }

    rc = utfInit(CAST_SW_DEVNUM(devNum));
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("utfInit", rc);
        utfPostInitPhase(rc);
        return rc;
    }

    utfPostInitPhase(rc);
#endif /* INCLUDE_UTF */

    return rc;
}

/**
* @internal gtDbPxPipeBoardReg_SMI_Simple function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for PX Pipe device.
*
* @param[out] hwInfoPtr                - pointer to HW Info structure
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbPxPipeBoardReg_SMI_Simple(
    OUT CPSS_HW_INFO_STC *hwInfoPtr
)
{
    GT_STATUS rc;
    /* init SMI driver */
    osPrintf("Using SMI driver\n");
    extDrvSoCInit();
    /* WA: change smi speed */
    if (cpssHwDriverLookup("/SoC/internal-regs/smi@0x72004") != NULL)
    {
        CPSS_HW_DRIVER_STC *soc = cpssHwDriverLookup("/SoC/internal-regs");
        if (soc != NULL)
        {
            /* 0 - normal speed(/128), 1 - fast mode(/16), 2 - accel (/8) */
            GT_U32 data = 2;
            soc->writeMask(soc, 0, 0x72014, &data, 1, 0x3);
        }
    }

    rc = cpssHwDriverSlaveSmiConfigure("/smi0", 5, hwInfoPtr);
    /* TEMP, CPSS will do it later */
    if (rc == GT_OK)
        cpssHwDriverAddAlias(hwInfoPtr->driver, "/PP0");
    return GT_OK;
}

/**
* @internal gtDbPxPipeBoardReg_SimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for PX Pipe device.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbPxPipeBoardReg_SimpleInit
(
    IN  GT_U8  boardRevId
)
{
    GT_STATUS   rc;
    GT_U32      start_sec  = 0;
    GT_U32      start_nsec = 0;
    GT_U32      end_sec  = 0;
    GT_U32      end_nsec = 0;
    GT_U32      diff_sec;
    GT_U32      diff_nsec;
    CPSS_HW_INFO_STC   hwInfo[APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS];
    GT_SW_DEV_NUM           devNum;
    GT_U32      devIndex;/* device index in the array of appDemoPpConfigList[devIndex] */

    GT_PCI_INFO pciInfo;

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */
    GT_U32     value;
    GT_BOOL    allowHwAccessOnly = GT_FALSE;

    /* Init cpss with no HW writes */
    rc = appDemoDbEntryGet("initRegDefaults", &value);
    if(rc == GT_OK)
    {
        allowHwAccessOnly = (GT_BOOL)value;
    }

    px_boardRevId = boardRevId;

    rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
        return rc;

#if (defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)
    /*simulation initialization*/
    appDemoRtosOnSimulationInit();

#endif /*(defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)*/

#ifdef ASIC_SIMULATION
    /* allow setting clock after the SOFT-RESET during the 'system reset'
       and 'system init' */
    if(px_simUserDefinedCoreClockMHz)
    {
        simCoreClockOverwrite_px(px_simUserDefinedCoreClockMHz);
    }
#endif

    /* Enable printing inside interrupt routine - supplied by extrernal drive */
    extDrvUartInit();

    /* Call to fatal_error initialization, use default fatal error call_back - supplied by mainOs */
    rc = osFatalErrorInit((FATAL_FUNC_PTR)NULL);
    if (rc != GT_OK)
        return rc;

    if (boardRevId != 3)
    {
        /* this function finds all Prestera devices on PCI bus */
        rc = px_getBoardInfoSimple(
            &hwInfo[0],
            &pciInfo);
    }
    else /* boardRevId == 3, SMI driver, SMI devId==5 */
    {
        rc = gtDbPxPipeBoardReg_SMI_Simple(&hwInfo[0]);
    }

    if (rc != GT_OK)
        return rc;

    px_boardTypePrint("DB" /*boardName*/, "PIPE" /*devName*/);

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    devIndex = 0;

    devNum = SYSTEM_DEV_NUM_MAC(devIndex);

    rc = px_appDemoInitSequence(devIndex, devNum, hwInfo, NULL /*portInitListPtr*/);
    if (rc != GT_OK)
        return rc;

    /* initialize only cpssDriver for HW access, skip the rest */
    if (allowHwAccessOnly == GT_TRUE)
    {
        CPSS_LOG_INFORMATION_MAC("Finished to initialize only cpssDriver for HW access, skip the rest");
        return GT_OK;
    }

    /* init the event handlers */
    rc = px_EventHandlerInit();
    if (rc != GT_OK)
        return rc;

    /* 802.1BR*/
    if(px_boardRevId == 2)
    {
#ifdef REFERENCEHAL_BPE_EXISTS
        rc = px_HAL_init(devNum);
        if (rc != GT_OK)
            return rc;
#else
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemo has been built without referenceHal\n", GT_BAD_PARAM);
        return GT_BAD_PARAM;
#endif
    }
    else
    {
#ifdef REFERENCEHAL_BPE_EXISTS
        px_HAL_init_LUA();
#endif
    }
#ifdef EMBEDDED_COMMANDS_EXISTS
        px_EMBCOM_init_LUA();
#endif
    rc = px_localUtfInit(devNum);
    if (rc != GT_OK)
        return rc;

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

    systemInitialized = GT_TRUE;

    /* allow interrupts / appDemo tasks to stable */
    osTimerWkAfter(500);

    return rc;
}

/**
* @internal gtDbPxPipeBoardReg_AllDevicesSimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for PX Pipe device.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbPxPipeBoardReg_AllDevicesSimpleInit
(
    IN  GT_U8  boardRevId
)
{
    GT_STATUS   rc;
    GT_U32      start_sec  = 0;
    GT_U32      start_nsec = 0;
    GT_U32      end_sec  = 0;
    GT_U32      end_nsec = 0;
    GT_U32      diff_sec;
    GT_U32      diff_nsec;
    CPSS_HW_INFO_STC   hwInfo[APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS];
    GT_SW_DEV_NUM           devNum;
    GT_U32      devIndex;/* device index in the array of appDemoPpConfigList[devIndex] */
    GT_BOOL     firstDev;
    GT_PCI_INFO pciInfo;
#ifdef INCLUDE_UTF
    GT_U32      numOfDevs;
#endif /* INCLUDE_UTF */

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */
    static PortInitList_STC_PTR portListPtrArr[] =
    {
         portInitlist_pipe1012
    };
    static GT_U32 portListPtrArrSize = (sizeof(portListPtrArr) / sizeof(portListPtrArr[0]));

    px_boardRevId = boardRevId;

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

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    px_boardTypePrint("RD" /*boardName*/, "PIPE" /*devName*/);
    firstDev = GT_TRUE;
    for (devIndex = 0; (1); devIndex++)
    {
        if (boardRevId != 3)
        {
            /* this function finds all Prestera devices on PCI bus */
            rc = px_getBoardInfo(firstDev, &hwInfo[devIndex], &pciInfo);
            firstDev = GT_FALSE;
            if (rc != GT_OK) break;
        }
        else
        {
            /* boardRevId == 3, SMI driver, SMI devId==5 */
            if (firstDev != GT_TRUE)
            {
                rc = GT_NO_MORE;
                break;
            }
            firstDev = GT_FALSE;
            rc = gtDbPxPipeBoardReg_SMI_Simple(&hwInfo[0]);
            if (rc != GT_OK)
                break;
        }

        devNum = SYSTEM_DEV_NUM_MAC(devIndex);

        rc = px_appDemoInitSequence(
            devIndex, devNum, &(hwInfo[devIndex]),
            portListPtrArr[devIndex % portListPtrArrSize]);
        if (rc != GT_OK)
            return rc;

    }

#ifdef INCLUDE_UTF
    numOfDevs = devIndex; /* index at break moment */
#endif /* INCLUDE_UTF */

    /* init the event handlers */
    rc = px_EventHandlerInit();
    if (rc != GT_OK)
        return rc;

       /* 802.1BR*/
#ifdef REFERENCEHAL_BPE_EXISTS
    px_HAL_init_LUA();
#endif

#ifdef EMBEDDED_COMMANDS_EXISTS
        px_EMBCOM_init_LUA();
#endif

#ifdef INCLUDE_UTF
    /* Initialize unit tests for CPSS */
    rc = utfPreInitPhase();
    if (rc != GT_OK)
    {
        utfPostInitPhase(rc);
        return rc;
    }

    for (devIndex = 0; (devIndex < numOfDevs); devIndex++)
    {
        devNum = SYSTEM_DEV_NUM_MAC(devIndex);
        rc = utfInit(CAST_SW_DEVNUM(devNum));
        if (rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("utfInit", rc);
            utfPostInitPhase(rc);
            return rc;
        }
    }
    utfPostInitPhase(GT_OK);
#endif /* INCLUDE_UTF */

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

    systemInitialized = GT_TRUE;

    /* allow interrupts / appDemo tasks to stable */
    osTimerWkAfter(500);

    return rc;
}

/**
* @internal gtDbPxPipeBoardReg_BoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbPxPipeBoardReg_BoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    boardRevId = boardRevId;

    return GT_OK;
}

/****************/
/* LINKER STUBS */
/****************/

/* enum support */
typedef struct {
    int         enumValue;
    const char *string;
} enumDescr;

#define LUA_ENUM_STUB(enumName) \
    enumDescr enumName[] = {    \
        { 0, NULL }}


#ifdef PX_FAMILY
#ifndef CHX_FAMILY

/*number of ports in test*/
GT_U8  prvTgfPortsNum = 0;
/* dummy array of ports for tests */
GT_U32  prvTgfPortsArray[20];
GT_U8   prvTgfDevsArray[20] = {0};
#endif
#endif

/**
* @internal gtDbPxPipeBoardReg function
* @endinternal
*
* @brief   Registration function for the PX Pipe board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbPxPipeBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    boardRevId = boardRevId;

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardSimpleInit               = gtDbPxPipeBoardReg_SimpleInit;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbPxPipeBoardReg_BoardCleanDbDuringSystemReset;
    boardCfgFuncs->boardGetHwInfo                = px_getBoardInfoSimple;

    return GT_OK;
}

/**
* @internal gtRdPxPipeBoard4DevsReg function
* @endinternal
*
* @brief   Registration function for the PX Pipe 4 devices board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtRdPxPipeBoard4DevsReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    boardRevId = boardRevId;

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardSimpleInit               = gtDbPxPipeBoardReg_AllDevicesSimpleInit;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbPxPipeBoardReg_BoardCleanDbDuringSystemReset;

    return GT_OK;
}

GT_STATUS appDemoPrintLinkChangeFlagSet
(
    IN GT_U32   enable
);
#ifdef LINUX_NOKM
extern void NOKM_DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(GT_U32 new_milisec_pollingSleepTime);
#endif /*LINUX_NOKM*/

/* running  pipe on emulator */
static GT_STATUS internal_pipe_onEmulator(GT_BOOL doInitSystem)
{
    /*appDemoTraceHwAccessEnable(0,2,1);*/

    /* shorten the number of memory accessing */
    appDemoDbEntryAdd ("noDataIntegrityShadowSupport",1);
    /* do not try to init the PHY libary */
    appDemoDbEntryAdd ("initPhy",0);
    /* emulate the 'DB' board ... not I2C to get it from ! */
    /*appDemoDbEntryAdd ("bc2BoardType",EXT_DRV_PIPE_DB_ID);*/

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
    return cpssInitSystem (33,1,0);
}

extern void prvCpssDrvTraceHwWriteBeforeAccessSet(IN GT_BOOL BeforeAccess);
extern void cpssDrvAddrComp_millisecSleepSet(IN GT_U32 timeout);


GT_STATUS  pipe_onEmulator(GT_VOID)
{
    return internal_pipe_onEmulator(GT_TRUE/*with cpssInitSystem*/);
}



GT_STATUS  pipe_onEmulator_quick(GT_U32    timeout)
{
    appDemoTraceHwAccessEnable(0,2,1);
    prvCpssDrvTraceHwWriteBeforeAccessSet(1);
    appDemoDbEntryAdd ("initSystemWithoutInterrupts",1);
    cpssDrvAddrComp_millisecSleepSet(timeout);
    return  pipe_onEmulator();
}


GT_STATUS  pipe_onEmulator_prepare(GT_VOID)
{
    return internal_pipe_onEmulator(GT_FALSE/*with cpssInitSystem*/);
}

/* mark for SMI controller address */
#define SMI_MNG_ADDRESS_MARK_CNS 0xFFFFFFAA

/**
* @internal appDemoPxDbReset function
* @endinternal
*
* @brief   Px DB reset. (part of 'System reset')
*
* @param[in] devNum                   - device number
* @param[in] needHwReset              - indication to do HW reset
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoPxDbReset
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         needHwReset
    )
{
    GT_STATUS       rc;         /* To hold funcion return code  */
    static GT_CHAR  devMustNotResetPort_str[100];
    CPSS_PORTS_BMP_STC portBmp; /* bmp for port */
    GT_U32          port; /* port iterator */
    struct {
        GT_U32  regAddr;
        GT_U32  value;
    } regsToRestore[] = {
        /* Unit Default ID (UDID) Register */
        { 0x00000204, 0 },
        /* Window n registers (n=0..5)
         *    Base Address,
         *    Size,
         *    High Address Remap,
         *    Window Control
         */
        { 0x020c, 0 }, { 0x0210, 0 }, { 0x23c, 0 }, { 0x0254, 0 },
        { 0x0214, 0 }, { 0x0218, 0 }, { 0x240, 0 }, { 0x0258, 0 },
        { 0x021c, 0 }, { 0x0220, 0 }, { 0x244, 0 }, { 0x025c, 0 },
        { 0x0224, 0 }, { 0x0228, 0 }, { 0x248, 0 }, { 0x0260, 0 },
        { 0x022c, 0 }, { 0x0230, 0 }, { 0x24c, 0 }, { 0x0264, 0 },
        { 0x0234, 0 }, { 0x0248, 0 }, { 0x250, 0 }, { 0x0258, 0 },
        /* address completion are used by LSP of MSYS for
           eth port PHY managment */
        { 0x0124, 0 }, { 0x0128, 0 }, { 0x012c, 0 }, { 0x0130, 0 },
        { 0x0134, 0 }, { 0x0138, 0 }, { 0x013c, 0 },
        /* SMI Master controller configuration */
        { SMI_MNG_ADDRESS_MARK_CNS, 0 }, /* must be last before {0,0} */
        { 0, 0 } /* END */
    };
    GT_U32          ii;
    GT_UINTPTR      deviceBaseAddrOnPex;
    GT_U32          doMgDecodingRestore;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel; /* DIAG device channel access type */
    GT_U32          value;
    GT_BOOL         doByteSwap;

    if (PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) == 0)
    {
        return (GT_STATUS)GT_ERROR;
    }

    /* all SIP5 devices should use PEX_MBUS driver with 8 regions address completion. */
    ifChannel = CPSS_CHANNEL_PEX_MBUS_E;
    doMgDecodingRestore = 1;

    if (needHwReset == GT_FALSE)
    {
        doMgDecodingRestore = 0;
    }

    /* fill-in registers restore DB by run time values */
    for (ii = 0; regsToRestore[ii].regAddr != 0; ii++)
    {
        if (SMI_MNG_ADDRESS_MARK_CNS == regsToRestore[ii].regAddr)
        {
            regsToRestore[ii].regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.SMI.SMIMiscConfiguration;
        }
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBmp);

    cpssOsBzero(devMustNotResetPort_str, sizeof(devMustNotResetPort_str));
    /* power down all ports */
    for (port = 0; port < PX_MAX_PORTS_NUM_CNS; port++)
    {
        cpssOsSprintf(devMustNotResetPort_str, "devMustNotResetPort_str_d[%d]p[%d]", devNum, port);
        /* set the port in bmp */
        CPSS_PORTS_BMP_PORT_SET_MAC(&portBmp, port);
        rc = cpssPxPortModeSpeedSet(devNum, &portBmp, GT_FALSE,
                                      /*don't care*/CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                      /*don't care*/CPSS_PORT_SPEED_1000_E);
        if (rc != GT_OK)
        {
            /* do not break .. maybe port not exists but we not have here knowledge about it */
            if (port < PRV_CPSS_PP_MAC(devNum)->numOfPorts)
            {
                /* give indication about ports that are supposed to be in 'valid range'*/
                /*osPrintf("cpssPxPortModeSpeedSet : failed on port[%d] \n",
                    port);*/
            }
        }

        /* remove the port from bmp */
        CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portBmp, port);
    }

    if (needHwReset == GT_TRUE)
    {
        CPSS_HW_INFO_STC *hwInfoPtr;

        hwInfoPtr = cpssDrvHwPpHwInfoStcPtrGet(devNum, 0);
        if (hwInfoPtr == NULL) return GT_FAIL;
        deviceBaseAddrOnPex = hwInfoPtr->resource.switching.start;
        doByteSwap = GT_FALSE;

        /* read the registers */
        for (ii = 0; regsToRestore[ii].regAddr != 0; ii++)
        {
            rc = prvCpssDrvHwPpReadRegister(devNum, regsToRestore[ii].regAddr, &(regsToRestore[ii].value));
            CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDrvHwPpReadRegister", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        rc = cpssPxDiagRegRead(deviceBaseAddrOnPex,
                                 ifChannel,
                                 CPSS_DIAG_PP_REG_INTERNAL_E, 0x50,
                                 &value, doByteSwap);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxDiagRegRead", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (value == 0xAB110000)
        {
            doByteSwap = GT_TRUE;
        }

        /* Disable All Skip Reset options ,exclude PEX */
        /* this Enable Skip Reset for PEX */
        rc = cpssPxHwPpSoftResetSkipParamSet(devNum, CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxHwPpSoftResetSkipParamSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssPxHwPpSoftResetTrigger(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxHwPpSoftResetTrigger", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

    /* Soft Reset takes some time. Need to sleep before access device again.*/
    #ifdef ASIC_SIMULATION
        cpssOsTimerWkAfter(2000);
    #else
         /* Soft reset is active for 2000 core clock cycles. Waiting 1 millisecond should be enough */
        cpssOsTimerWkAfter(1);
    #endif /*ASIC_SIMULATION*/

    }
    else
    {
        deviceBaseAddrOnPex = 0; /* dummy.. will not be used */
        doByteSwap = GT_FALSE;  /* dummy.. will not be used */
    }

    osPrintf("appDemoPxDbReset : 'remove device from cpss' \n");
    rc = cpssPxCfgDevRemove(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxCfgDevRemove", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* let the device minimal time for reset before we restore next registers */
    osTimerWkAfter(1);

    if (doMgDecodingRestore)
    {
        osPrintf("appDemoPxDbReset : (after soft reset) restore 'MG Registers/Address Decoding' configurations \n");

        rc = cpssPxDiagRegRead(deviceBaseAddrOnPex,
                                 ifChannel,
                                 CPSS_DIAG_PP_REG_INTERNAL_E, 0x50,
                                 &value, doByteSwap);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxDiagRegRead", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        /*osPrintf("cpssPxDiagRegRead:deviceBaseAddrOnPex[0x%x] reg[0x50] with value[%d] doByteSwap[%d] \n",
            deviceBaseAddrOnPex,value,doByteSwap);*/


        /* restore the registers of :
           <MG_IP> MG_IP/MG Registers/Address Decoding/Base Address %n
           0x0000020C + n*8: where n (0-5) represents BA

           and:
           <MG_IP> MG_IP/MG Registers/Address Decoding/Size (S) %n
            0x00000210 + n*0x8: where n (0-5) represents SR

           and:
           <MG_IP> MG_IP/MG Registers/Address Decoding/High Address Remap %n
            0x0000023c + n*0x4: where n (0-5) represents HA


           and :
           <MG_IP> MG_IP/MG Registers/Address Decoding/Window Control Register%n
           0x00000254 + n*4: where n (0-5) represents n
        */

        /* write the saved registers */
        /* since the device was removed from cpss ...
           we must use the 'diag' functions to write to the device ... */
        /* restore the registers */
        for (ii = 0; regsToRestore[ii].regAddr != 0; ii++)
        {
            rc = cpssPxDiagRegWrite(deviceBaseAddrOnPex,
                                      ifChannel,
                                      CPSS_DIAG_PP_REG_INTERNAL_E,
                                      regsToRestore[ii].regAddr,
                                      regsToRestore[ii].value,
                                      doByteSwap);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxDiagRegWrite", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    osPrintf("appDemoPxDbReset - ended \n");

    return GT_OK;
}




