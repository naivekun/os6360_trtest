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
* @file gtDbDxFalcon.c
*
* @brief Initialization functions for the Falcon - SIP6 - board.
*
* @version   2
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/userExit/userEventHandler.h>
#include <gtExtDrv/drivers/gtUartDrv.h>
#ifdef INCLUDE_UTF
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#endif /* INCLUDE_UTF */
#include <gtOs/gtOsExc.h>

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#ifdef LINUX_NOKM
    extern GT_STATUS prvNoKmDrv_configure_dma_per_devNum(IN GT_U8 devNum , IN GT_U32    portGroupId);
#endif  /*LINUX_NOKM*/

/* offset used during HW device ID calculation formula */
extern GT_U8 appDemoHwDevNumOffset;
/* cpssInitSystem has been run */
extern GT_BOOL systemInitialized;

/* indication that during initialization the call to appDemoBc2PortListInit(...)
   was already done so no need to call it for second time. */
extern GT_U32  appDemo_PortsInitList_already_done;

extern CPSS_DXCH_PORT_MAP_STC  *appDemoPortMapPtr;
/* number of elements in appDemoPortMapPtr */
extern GT_U32                  appDemoPortMapSize;

extern GT_STATUS prvCpssDrvHwPpPrePhase1NextDevFamilySet(IN CPSS_PP_FAMILY_TYPE_ENT devFamily);
extern GT_STATUS appDemoDxLibrariesInit(
    IN  GT_U8                       dev,
    IN  APP_DEMO_LIB_INIT_PARAMS    *libInitParamsPtr,
    IN  CPSS_PP_CONFIG_INIT_STC     *sysConfigParamsPtr
);
extern GT_STATUS appDemoDxPpGeneralInit(GT_U8 dev);
extern GT_STATUS appDemoDxTrafficEnable(GT_U8 dev);
extern GT_STATUS boardAutoPollingConfigure(IN  GT_U8   dev,IN  GT_U32  boardType);
extern GT_STATUS appDemoBc2IpLpmRamDefaultConfigCalc(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);
extern GT_STATUS appDemoBc2PortListInit
(
    IN GT_U8 dev,
    IN PortInitList_STC * portInitList,
    IN GT_BOOL            skipCheckEnable
);
extern GT_STATUS appDemoDxHwDevNumChange
(
    IN  GT_U8                       dev,
    IN  GT_HW_DEV_NUM               hwDevNum
);
GT_STATUS appDemoBc2PIPEngineInit
(
    IN  GT_U8       boardRevId
);
GT_STATUS appDemoB2PtpConfig
(
    IN  GT_U8  devNum
);
extern GT_STATUS appDemoDxHwPpPhase1DbEntryInit
(
    IN  GT_U8                        dev,
    INOUT  CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   *cpssPpPhase1ParamsPtr,
    IN  CPSS_PP_DEVICE_TYPE         deviceId,
    OUT GT_U32                      *waIndexPtr,
    OUT CPSS_DXCH_IMPLEMENT_WA_ENT  *waFromCpss,
    OUT GT_BOOL                     *trunkToCpuWaEnabledPtr
);

static GT_STATUS   appDemoFalconMainUtForbidenTests(void);

static GT_STATUS falcon_appDemoDbUpdate
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_PP_DEVICE_TYPE     devType
);

#define _2_TILES_PORTS_OFFSET   56
#define TESTS_PORTS_OFFSET(devNum)  (_2_TILES_PORTS_OFFSET)

#define PRINT_SKIP_DUE_TO_DB_FLAG(reasonPtr , flagNamePtr) \
        cpssOsPrintf("NOTE: '%s' skipped ! (due to flag '%s') \n",  \
            reasonPtr , flagNamePtr)

#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

#define UNUSED_PARAM_MAC(x) x = x

#define FALCON_TXQ_PER_DQ   (8 * (FALCON_MAC_PER_DP + 1))
#define FALCON_MAC_PER_DP   8 /* -- without CPU port -- first ports are those with SERDES 28/56 */
#define FALCON_NUM_DP       8 /* DP port groups */

/* build TXQ_port from macPort, dpIndex */
#define FALCON_TXQ_PORT(macPort, dpIndex)   \
    (FALCON_TXQ_PER_DQ*(dpIndex) + (macPort))

/* build MAC_port from macPort, dpIndex */
#define FALCON_MAC_PORT(macPort, dpIndex)   \
    (FALCON_MAC_PER_DP*(dpIndex) + (macPort))

#define CPU_PORT    CPSS_CPU_PORT_NUM_CNS
/* value to not collide with TXQ of other port */
#define TXQ_CPU_PORT    300

typedef struct
{
    GT_PHYSICAL_PORT_NUM                startPhysicalPortNumber;
    GT_U32                              numOfPorts;
    GT_U32                              startGlobalDmaNumber;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    GT_U32                              jumpDmaPorts;/* allow to skip DMA ports (from startGlobalDmaNumber).
                                                        in case that mappingType is CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E
                                                        this field Contains the cascade port number*/
}FALCON_PORT_MAP_STC;

#define IGNORE_DMA_STEP 0 /*value 0 or 1 means 'ignore' */
/* jump DMA in steps of 2 because we currently limited to '128 physical ports' mode */
/* so the 12.8T device with the 256+ MACs ... need to use only limited MACs */
#define DMA_STEP_OF_2   2
#define DMA_STEP_OF_3   3
#define DMA_STEP_OF_4   4


/*stay with same DMA - needed for remote ports */
#define DMA_NO_STEP   0xFFFF
#define NO_REMOTE_PORT 0xFFFF

/*  in sip5_20 was 2 ports modes:
     256, 512.
    sip6 have 5 ports modes:
    64, 128, 256, 512, 1024.
    Note that falcon have 3.2 device, 6.4 device and 12.8 device, each of these devices can have the ports modes above so in total it's (3 * 5) = 15 different modes
    */

/* 64 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports).*/
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap_64_port_mode[] =
{
    /* physical ports 0..49 , mapped to MAC 0..32 */
     {0               , 49          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  65/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
     /* map 'CPU SDMA ports' 56..58 (no MAC/SERDES) DMA 66..68 */
    ,{56              , 3           ,  66/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_3_2_defaultMap_64_port_mode = sizeof(falcon_3_2_defaultMap_64_port_mode)/sizeof(falcon_3_2_defaultMap_64_port_mode[0]);


/* 128 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). */
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  65/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP }
    /* physical ports 64..68 , mapped to MAC 59..63 */
    ,{64              , (63-59)+1   ,  59       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP }
#if 0 /* muxed with MG[2] on DP[6] */
    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 69 , mapped to MAC 64 */
    ,{69              , 1           ,  64       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
#endif /*0*/
    /* map 'CPU SDMA ports' 80..82 (no MAC/SERDES) DMA 66..68 */
    ,{80              , 3           ,  66/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_3_2_defaultMap = sizeof(falcon_3_2_defaultMap)/sizeof(falcon_3_2_defaultMap[0]);

/* currently 256 and 512 modes are using 128 mode port map to avoid failed tests that not support other ports selection*/

/* 256 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap_256_port_mode[] =
{
    /* physical ports 128..186 , mapped to MAC 0..58 */
     {0              , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  65/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 192..196 , mapped to MAC 59..63 */
    ,{128             , (63-59)+1   ,  59       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 80..82 (no MAC/SERDES) DMA 66..68 */
    ,{80              , 3           ,  66/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_3_2_defaultMap_256_port_mode = sizeof(falcon_3_2_defaultMap_256_port_mode)/sizeof(falcon_3_2_defaultMap_256_port_mode[0]);


/* 1024 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap_1024_port_mode_remote_ports[] =
{
    /* native ports: not remote ports .physical ports 128..186 , mapped to MAC 0..58 */
     {(0 /*+ 128*/)              , 59/*56*/          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP }
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  65/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* remote physical ports 590..599 , mapped to MAC 59 , with phisical port 64. this is comment out to enable the enh-UT ports  58 12 36 0 */
   /* ,{590             , 10   ,  59       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,DMA_NO_STEP , 64}*/
    /* remote physical ports 600..609 , mapped to MAC 60 , with phisical port 65 */
    ,{600             , 10   ,  60       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,65}
    /* remote physical ports 610..619 , mapped to MAC 61 , with phisical port 66 */
    ,{610             , 10   ,  61       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,66}
    /* remote physical ports 620..629 , mapped to MAC 62 , with phisical port 67 */
    ,{620             , 9   ,  62       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 67}
    /* remote physical ports 630..639 , mapped to MAC 63 , with phisical port 68 */
    ,{630             , 9   ,  63       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 68}
#if 0 /* muxed with MG[2] on DP[6] */
    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 69 , mapped to MAC 64 */
    ,{69              , 1           ,  64       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
#endif /*0*/
    /* map 'CPU SDMA ports' 80..82 (no MAC/SERDES) DMA 66..68 */
    ,{80              , 3           ,  66/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}

};
static GT_U32   actulaNum_falcon_3_2_defaultMap_1024_port_mode_remote_ports = sizeof(falcon_3_2_defaultMap_1024_port_mode_remote_ports)/sizeof(falcon_3_2_defaultMap_1024_port_mode_remote_ports[0]);


/* we have PHYSICAL ports 0..59,63,64..119*/
/* this macro hold the number of ports from 64..119 */
#define NUM_PORTS_64_TO_119  ((119-64)+1)

/* 64 port mode: mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap_64_port_mode[] =
{
    /* physical ports 0..32 , mapped to MAC 0..32 */
     {0               , 49          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  130/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E   ,IGNORE_DMA_STEP}

    /* map 'CPU SDMA ports' 56 - 63 to mac 66 - 73 */
    ,{56              , 7           , 131/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_6_4_defaultMap_64_port_mode = sizeof(falcon_6_4_defaultMap_64_port_mode)/sizeof(falcon_6_4_defaultMap_64_port_mode[0]);

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap_256_port_mode[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0              , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 130 */
    ,{CPU_PORT/*63*/  , 1           ,  130/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 64..119 , mapped to MAC 65..119 */
    ,{64              ,NUM_PORTS_64_TO_119,  65 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* map 'CPU SDMA ports' */
    ,{121              , 7           , 131/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_6_4_defaultMap_256_port_mode = sizeof(falcon_6_4_defaultMap_256_port_mode)/sizeof(falcon_6_4_defaultMap_256_port_mode[0]);

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  130/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 64..119 , mapped to MAC 59..116 */
    ,{64              ,NUM_PORTS_64_TO_119,  59 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' */
    ,{121              , 7           , 131/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_6_4_defaultMap = sizeof(falcon_6_4_defaultMap)/sizeof(falcon_6_4_defaultMap[0]);

static FALCON_PORT_MAP_STC falcon_6_4_100G_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..116 with step 2 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  130/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}

    ,{64              , 5,             118,         CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2}

    /* map 'CPU SDMA ports' */
    ,{121              , 7           , 131/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_6_4_100G_defaultMap = sizeof(falcon_6_4_100G_defaultMap)/sizeof(falcon_6_4_100G_defaultMap[0]);


/***************************************************************/
/* flag to use the MAX number of macs that the device supports */
/* AND max SDMAs                                               */
/***************************************************************/
static GT_U32   modeMaxMac = 0;
static GT_U32   mode100G   = 0;


static FALCON_PORT_MAP_STC falcon_6_4_maxMAC[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  130/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 64..127 , mapped to MAC 59..123 */
    ,{64              ,64,             59 ,        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_6_4_maxMAC = sizeof(falcon_6_4_maxMAC)/sizeof(falcon_6_4_maxMAC[0]);

static FALCON_PORT_MAP_STC falcon_6_4_256_port_maxMAC[] =
{
    /* first 59 MACs : physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0        , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  130/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* next 128-59 MACs : physical ports 64..132 , mapped to MAC 59..127 */
    ,{64              ,(128-59)     ,   59       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* map 'CPU SDMA ports' */
    ,{133             , 7           , 131/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_6_4_256_port_maxMAC = sizeof(falcon_6_4_256_port_maxMAC)/sizeof(falcon_6_4_256_port_maxMAC[0]);

/* 1024 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap_1024_port_mode_remote_ports[] =
{
    /* native ports: first 29 MACs : physical ports 0..28 , mapped to MAC 0..28 */
     {0              , 28          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 29..64 */
     /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  130/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
       /* remote physical ports 600..609 , mapped to MAC 65 (DP 8) , with phisical port 75 */
    ,{600             , 10   ,  65     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 75}
    /* remote physical ports 610..619 , mapped to MAC 33 (DP 4) , with phisical port 76 */
    ,{610             , 10   ,  33       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,76}
    /* remote physical ports 620..629 , mapped to MAC 66 (DP 8), with phisical port 77 */
    ,{620             , 9   ,  66       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 77}
    /* remote physical ports 630..639 , mapped to MAC 33 , with phisical port 78 */
    ,{630             , 9   ,  35       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 78}
    /* next 67-77 MACs : physical ports 64..74 */
     ,{64              ,10     ,     67     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* next 77- 97 MACs : physical ports 100..119 */
     ,{100              ,20     ,     77     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' */
    ,{133             , 7           , 131/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actulaNum_falcon_6_4_defaultMap_1024_port_mode_remote_ports = sizeof(falcon_6_4_defaultMap_1024_port_mode_remote_ports)/sizeof(falcon_6_4_defaultMap_1024_port_mode_remote_ports[0]);

/*
NOTE: in 12.8 we select only 8 CPU SDMA ports although 16 available , because:
1. we work in 128 ports mode ... so using all 16 will reduce dramatically the number of regular ports.
2. we want to test 'non-consecutive' MG units so we use (MGs:1,3..5,10..13)
3. we test CPU_PORT (63) to work with MG1 instead of MG0 , because DP[4] (connected to MG0)
    may serve 'CPU network port'
*/


/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_64_port_mode[] =
{
    /* physical ports 0..32 , mapped to MAC 0..58 */
     {0               , 49          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 259 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  259/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{50              , 3           , 261/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{54              , 4          , 268/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_12_8_defaultMap_64_port_mode = sizeof(falcon_12_8_defaultMap_64_port_mode)/sizeof(falcon_12_8_defaultMap_64_port_mode[0]);

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_256_port_mode[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 259 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  259/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 7 CPU ports , the existence of next ports : in DP[8..11].
    meaning DMA ports : 64,72,80,88 */
    /* physical ports 64..109 , mapped to MAC 64..154 (in steps of 2) */
    ,{64              ,NUM_PORTS_64_TO_119-10,  128  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_2}
    /* physical ports 110..119 , mapped to MAC 176..216 (in steps of 4) */
   /* ,{110             ,                    10,  176 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_4,NO_REMOTE_PORT}*/

    /* map 'CPU SDMA ports' , DMA 261..263 of MG [3..5] */
    ,{121              , 3           , 261/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 268..270 of MG [10..13] */
    ,{124              , 4          , 268/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_12_8_defaultMap_256_port_mode = sizeof(falcon_12_8_defaultMap_256_port_mode)/sizeof(falcon_12_8_defaultMap_256_port_mode[0]);


/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 259 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  259/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 7 CPU ports , the existence of next ports : in DP[8..11].
    meaning DMA ports : 64,72,80,88 */
    /* physical ports 64..109 , mapped to MAC 64..154 (in steps of 2) */
    ,{64              ,NUM_PORTS_64_TO_119-10,  64  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_2}
    /* physical ports 110..119 , mapped to MAC 176..216 (in steps of 4) */
    ,{110             ,                    10,  176 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_4}

    /* map 'CPU SDMA ports' , DMA 261..263 of MG [3..5] */
    ,{121              , 3           , 261/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 268..270 of MG [10..13] */
    ,{124              , 4          , 268/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actulaNum_falcon_12_8_defaultMap = sizeof(falcon_12_8_defaultMap)/sizeof(falcon_12_8_defaultMap[0]);


/* 1024 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_1024_port_mode_remote_ports[] =
{
    /* native ports: first 29 MACs : physical ports 0..28 , mapped to MAC 0..28 */
     {0              , 28          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 29..64 */
     /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 259 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  259/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
      /* remote physical ports 600..609 , mapped to MAC 65 (DP 8) , with phisical port 75 */
    ,{600             , 10   ,  65     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,75}
    /* remote physical ports 610..619 , mapped to MAC 33 (DP 4) , with phisical port 76 */
    ,{610             , 10   ,  33       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,76}
    /* remote physical ports 620..629 , mapped to MAC 66 (DP 8), with phisical port 77 */
    ,{620             , 9   ,  160       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 77}
    /* remote physical ports 630..639 , mapped to MAC 33 , with phisical port 78 */
    ,{630             , 9   ,  35       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 78}
    /* next 67-77 MACs : physical ports 64..74 */
     ,{64              ,10     ,     67     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* next 77- 97 MACs : physical ports 100..119 */
     ,{100              ,20     ,     77     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 261..263 of MG [3..5] */
    ,{121              , 3           , 261/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 268..270 of MG [10..13] */
    ,{124              , 4          , 268/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actulaNum_falcon_12_8_defaultMap_1024_port_mode_remote_ports = sizeof(falcon_12_8_defaultMap_1024_port_mode_remote_ports)/sizeof(falcon_12_8_defaultMap_1024_port_mode_remote_ports[0]);



#define MAX_MAPPED_PORTS_CNS    1024
static  GT_U32  actualNum_cpssApi_falcon_defaultMap = 0;/*actual number of valid entries in cpssApi_falcon_defaultMap[] */
static CPSS_DXCH_PORT_MAP_STC   cpssApi_falcon_defaultMap[MAX_MAPPED_PORTS_CNS] =
{
    {0}
    /* filled at run time , by info from falcon_3_2_defaultMap[] */
};


/* the number of CPU SDMAs that we need resources for */
static GT_U32   numCpuSdmas = 0;

/* fill array of cpssApi_falcon_defaultMap[] and update actualNum_cpssApi_falcon_defaultMap */
static GT_STATUS fillDbForCpssPortMappingInfo(IN GT_U32 devNum)
{
    GT_STATUS   rc;
    FALCON_PORT_MAP_STC *currAppDemoInfoPtr;
    CPSS_DXCH_PORT_MAP_STC *currCpssInfoPtr;
    GT_U32  ii,jj;
    GT_U32  dmaPortOffset;
    GT_U32 numEntries;
    GT_U32 maxPhyPorts;


    if(actualNum_cpssApi_falcon_defaultMap)
    {
        /* already initialized */
        return GT_OK;
    }
    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        default:
        case 0:
        case 1:
            if (maxPhyPorts == 64)
            {
                 currAppDemoInfoPtr = &falcon_3_2_defaultMap_64_port_mode[0];
                 numEntries = actulaNum_falcon_3_2_defaultMap_64_port_mode;
            }
            else if (maxPhyPorts == 256)
            {
                currAppDemoInfoPtr = &falcon_3_2_defaultMap_256_port_mode[0];
                numEntries = actulaNum_falcon_3_2_defaultMap_256_port_mode;
            }
            else if (maxPhyPorts == 1024)
            {
                currAppDemoInfoPtr = &falcon_3_2_defaultMap_1024_port_mode_remote_ports[0];
                numEntries = actulaNum_falcon_3_2_defaultMap_1024_port_mode_remote_ports;
            }
            else /* 128, 512 */
            {
                currAppDemoInfoPtr = &falcon_3_2_defaultMap[0];
                numEntries = actulaNum_falcon_3_2_defaultMap;
            }
            break;
        case 2:
             if(mode100G)
            {
                currAppDemoInfoPtr = &falcon_6_4_100G_defaultMap[0];
                numEntries = actulaNum_falcon_6_4_100G_defaultMap;
            }
            else if (maxPhyPorts == 64)
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap_64_port_mode[0];
                numEntries = actulaNum_falcon_6_4_defaultMap_64_port_mode;
            }
            else if (maxPhyPorts == 256)
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap_256_port_mode[0];
                numEntries = actulaNum_falcon_6_4_defaultMap_256_port_mode;
            }
            else if (maxPhyPorts == 1024)
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap_1024_port_mode_remote_ports[0];
                numEntries = actulaNum_falcon_6_4_defaultMap_1024_port_mode_remote_ports;
            }
            else /* 128, 512*/
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap[0];
                numEntries = actulaNum_falcon_6_4_defaultMap;
            }

            if(modeMaxMac)
            {
                if (maxPhyPorts == 256)
                {
                    currAppDemoInfoPtr = &falcon_6_4_256_port_maxMAC[0];
                    numEntries = actulaNum_falcon_6_4_256_port_maxMAC;
                }
                else
                {
                    currAppDemoInfoPtr = &falcon_6_4_maxMAC[0];
                    numEntries = actulaNum_falcon_6_4_maxMAC;
                }
            }
            break;
    case 4:
            if (maxPhyPorts == 64)
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap_64_port_mode[0];
                numEntries = actulaNum_falcon_12_8_defaultMap_64_port_mode;
            }
            else if (maxPhyPorts == 256)
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap_256_port_mode[0];
                numEntries = actulaNum_falcon_12_8_defaultMap_256_port_mode;
            }
            else if (maxPhyPorts == 1024)
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap_1024_port_mode_remote_ports[0];
                numEntries = actulaNum_falcon_12_8_defaultMap_1024_port_mode_remote_ports;
            }
            else /* 128, 512*/
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap[0];
                numEntries = actulaNum_falcon_12_8_defaultMap;
            }
            break;
    }

    cpssOsMemSet(cpssApi_falcon_defaultMap,0,sizeof(cpssApi_falcon_defaultMap));

    currCpssInfoPtr    = &cpssApi_falcon_defaultMap[0];

    for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
    {
         if ((currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)&&(actualNum_cpssApi_falcon_defaultMap < maxPhyPorts))
            {
                /* first generate line that map between the mac number to physical port in the device.
                for remote ports the physical port number ( cascade number) is in field of "dma step"*/
                 currCpssInfoPtr->physicalPortNumber = currAppDemoInfoPtr->jumpDmaPorts;
                 currCpssInfoPtr->mappingType        = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
                 currCpssInfoPtr->interfaceNum       = currAppDemoInfoPtr->startGlobalDmaNumber;
                 currCpssInfoPtr->txqPortNumber      = 0;

                 actualNum_cpssApi_falcon_defaultMap++;
                 currCpssInfoPtr++;
                /* second, generate lines that map between remote physical port to the mac number*/
            }
        dmaPortOffset = 0;
        for(jj = 0 ; jj < currAppDemoInfoPtr->numOfPorts; jj++ , currCpssInfoPtr++)
        {
            if(actualNum_cpssApi_falcon_defaultMap >= maxPhyPorts)
            {
                rc = GT_FULL;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssApi_falcon_defaultMap is FULL, maxPhy port is %d ", maxPhyPorts);
                return rc;
            }

            currCpssInfoPtr->physicalPortNumber = currAppDemoInfoPtr->startPhysicalPortNumber + jj;
            currCpssInfoPtr->mappingType        = currAppDemoInfoPtr->mappingType;
            currCpssInfoPtr->interfaceNum       = currAppDemoInfoPtr->startGlobalDmaNumber + dmaPortOffset;
            /* convert the DMA to TXQ port number , because those need to be in consecutive range and in 8 TC steps */
            currCpssInfoPtr->txqPortNumber      = 0;


            if(currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                numCpuSdmas++;
            }

            actualNum_cpssApi_falcon_defaultMap++;

             if((currAppDemoInfoPtr->jumpDmaPorts != DMA_NO_STEP)&&(currAppDemoInfoPtr->mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E))
             {
                   if(currAppDemoInfoPtr->jumpDmaPorts >= 2)
                    {
                        dmaPortOffset += currAppDemoInfoPtr->jumpDmaPorts;
                    }
                    else
                    {
                        dmaPortOffset ++;
                    }
               }
        }
    }

    return GT_OK;
}

#define GM_FALCON_MAC_AND_TXQ_PORT_MAC(macPort) macPort,(8)

/* NOTE: the GM supports only single DP units , and single TXQ-DQ due to memory issues. */
/* so use only ports of DMA 0..8 , CPU port that uses the DMA 8                         */
static CPSS_DXCH_PORT_MAP_STC singlePipe_singleDp_cpssApi_falcon_defaultMap[] =
{ /* Port,            mappingType                           portGroupm,         intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*0*/  {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 0),  GT_FALSE,   GT_NA}
/*1*/ ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 1),  GT_FALSE,   GT_NA}
/*2*/ ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 2),  GT_FALSE,   GT_NA}
/*3*/ ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 3),  GT_FALSE,   GT_NA}
/*4*/ ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 4),  GT_FALSE,   GT_NA}
/*5*/ ,{  79, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 5),  GT_FALSE,   GT_NA}
/*6*/ ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 6),  GT_FALSE,   GT_NA}
/*7*/ ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 7),  GT_FALSE,   GT_NA}
/*8*/ ,{CPU_PORT, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 8),  GT_FALSE,   GT_NA}
};

static PortInitList_STC *falcon_force_PortsInitListPtr = NULL;

/* GM using single DP (only 8 MACs) */
static PortInitList_STC singlePipe_singleDp_portInitlist_falcon[] =
{
     { PORT_LIST_TYPE_LIST,      {0,18,36,58,54,64,79,80,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* 64 ports mode: Falcon ports in 100G mode */
static PortInitList_STC falcon_3_2_portInitlist_64_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,49,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E/*CPSS_PORT_SPEED_100G_E*/,  CPSS_PORT_INTERFACE_MODE_KR_E/*CPSS_PORT_INTERFACE_MODE_KR2_E*/ }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* -- 50G PORT MODE -- */

/* 128 ports mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_3_2_portInitlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,58,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_LIST,      {64,65,66,67,68,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* 256 ports mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_3_2_portInitlist_256_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,58,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_LIST,      {128,130,131,132,133,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* 1024 ports mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_3_2_portInitlist_1024_port_mode[] =
{

    /*{ PORT_LIST_TYPE_INTERVAL,  {128,186,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }*/
     { PORT_LIST_TYPE_INTERVAL,  {0,58,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_INTERVAL,  {/*590*/600,620,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_LIST,      {621,622,623,624,625,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E } /* remote ports*/
    ,{ PORT_LIST_TYPE_LIST,      {65,66,67,68,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E } /* cascade ports*/
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* -- 100G PORT MODE -- */

/* 128 ports mode: Falcon ports in 100G mode */
static PortInitList_STC falcon_3_2_100G_portInitlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,58,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
    ,{ PORT_LIST_TYPE_LIST,      {65,67,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* 256 ports mode: Falcon ports in 100G mode */
static PortInitList_STC falcon_3_2_100G_portInitlist_256_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,58,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
    ,{ PORT_LIST_TYPE_LIST,      {128,130,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* 1024 ports mode: Falcon ports in 100G mode */
static PortInitList_STC falcon_3_2_100G_portInitlist_1024_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {512,570,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
    ,{ PORT_LIST_TYPE_LIST,      {577,579,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E } /* remote ports */

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* 64 port mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_6_4_portInitlist_64_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,49,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* Falcon ports in 50G mode */
static PortInitList_STC falcon_6_4_portInitlist_256_port_mode[] =
{
     { PORT_LIST_TYPE_LIST,      {0,1,2,3,11,18,20,32,35,36,54,55,56,57,58, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
     /* ports 64..127*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,127,1,                            APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }

};


/* 1024 ports mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_6_4_portInitlist_1024_port_mode[] =
{

     { PORT_LIST_TYPE_INTERVAL,  {0,28,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,74,1,                            APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  {600,620,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_LIST,      {621,622,623,624,625,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E } /* remote ports*/
    ,{ PORT_LIST_TYPE_LIST,      {75,76,77,78,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E } /* cascade ports*/
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* Falcon ports in 50G mode */
static PortInitList_STC falcon_6_4_portInitlist[] =
{
     { PORT_LIST_TYPE_LIST,      {0,1,2,3,18,20,32,35,36,54,55,56,57,58, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
     /* ports 64..127*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,127,1,                            APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Falcon ports in 100G mode */
static PortInitList_STC falcon_6_4_100G_portInitlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,58,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,69,1,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* Falcon ports in 50G mode */
static PortInitList_STC falcon_12_8_portInitlist_64_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,       {0,49,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E    }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* Falcon ports in 50G mode */
static PortInitList_STC falcon_12_8_portInitlist_256_port_mode[] =
{
    { PORT_LIST_TYPE_LIST,      {0,1,2,3,18,20,32,35,36,54,55,56,57,58, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
         /* ports 64..127*/
   ,{ PORT_LIST_TYPE_INTERVAL,  {64,127,1,                           APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
   ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* 1024 ports mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_12_8_portInitlist_1024_port_mode[] =
{

     { PORT_LIST_TYPE_INTERVAL,  {0,28,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,74,1,                            APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  {600,620,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_LIST,      {621,622,623,624,625,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E } /* remote ports*/
    ,{ PORT_LIST_TYPE_LIST,      {75,76,77,78,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E } /* cascade ports*/
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Falcon ports in 50G mode */
static PortInitList_STC falcon_12_8_portInitlist[] =
{
     { PORT_LIST_TYPE_LIST,      {0,1,2,3,18,20,32,35,36,54,55,56,57,58, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
          /* ports 64..127*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,127,1,                           APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* max MACs Falcon ports in 50G mode */
static PortInitList_STC falcon_6_4_maxMAC_portInitlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,  58,1,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,255,1,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/**
* @internal falcon_getBoardInfo function
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
static GT_STATUS falcon_getBoardInfo
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
            MV_EXT_DRV_CFG_FLAG_EAGLE_E,
            hwInfoPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("extDrvPexConfigure", rc);

    return rc;
}

/**
* @internal falcon_getBoardInfoSimple function
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
static GT_STATUS falcon_getBoardInfoSimple
(
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO *pciInfo
)
{
    GT_STATUS   rc;

    rc = falcon_getBoardInfo(GT_TRUE/*firstDev*/, hwInfoPtr, pciInfo);
    if (rc != GT_OK) {return rc;}

    /* In our case we want to find just one prestera device on PCI bus*/
    rc = gtPresteraGetPciDev(GT_FALSE, pciInfo);
    if(rc == GT_OK)
    {
        osPrintf("More than one recognized device found on PCI bus!\n");
        return GT_INIT_ERROR;
    }

    /* give 'hint' to the CPSS that the device is of Falcon type .
       so it will know to access the MG registers in proper offset ...
       to read register 0x4c to get the 'deviceId'
    */
    prvCpssDrvHwPpPrePhase1NextDevFamilySet(CPSS_PP_FAMILY_DXCH_FALCON_E);

    return GT_OK;
}

#ifdef ASIC_SIMULATION
extern GT_U32 dx_simUserDefinedCoreClockMHz;
extern GT_VOID simCoreClockOverwrite
(
    GT_U32 simUserDefinedCoreClockMHz
);
#endif

/**
* @internal falcon_boardTypePrint function
* @endinternal
*
* @brief   This function prints type of Falcon board and Cider version.
*
* @param[in] boardName                - board name
* @param[in] devName                  - device name
*                                       none
*/
static GT_VOID falcon_boardTypePrint
(
    IN GT_CHAR  *boardName,
    IN GT_CHAR *devName
)
{
    GT_CHAR *environment;
    GT_CHAR *ciderVersionsArr[PRV_CPSS_DXCH_FALCON_CIDER_VERSION___LAST___E + 1] = {
        NULL /* -- PRV_CPSS_DXCH_FALCON_CIDER_VERSION__LATEST__CNS -- */
        ,STR(PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E)
        ,STR(PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_02_22_E)
        ,STR(PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_03_13_E)
        ,STR(PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E/*former18_04_22*/)
        ,STR(PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_07_09_E/*former18_04_22*/)
        ,NULL
    };
    PRV_CPSS_DXCH_FALCON_CIDER_VERSION_ENT ciderVersion;
    GT_CHAR *ciderVersionName = NULL;

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
    ciderVersion     = prvCpssDxchFalconCiderVersionGet();
    if(ciderVersion < PRV_CPSS_DXCH_FALCON_CIDER_VERSION___LAST___E)
    {
        ciderVersionName = ciderVersionsArr[ciderVersion];
    }

    if(ciderVersionName == NULL)
    {
        ciderVersionName = "unknown !?";
    }

    cpssOsPrintf("%s Board Type: %s [%s]\n", devName , boardName , environment);
    cpssOsPrintf("Using Cider Version [%s] \n", ciderVersionName);

}

/**
* @internal getPpPhase1ConfigSimple function
* @endinternal
*
* @brief   Returns the configuration parameters for cpssDxChHwPpPhase1Init().
*
* @param[in] devNum                   - The CPSS devNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of configBoardAfterPhase1.
*
*/
static GT_STATUS getPpPhase1ConfigSimple
(
    IN  GT_SW_DEV_NUM                       devNum,
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC  *phase1Params
)
{
    GT_STATUS rc;
    GT_U32  tmpValue , numOfTiles,portsFalcor,maxNumOfPhyPortsToUse;

    phase1Params->devNum = CAST_SW_DEVNUM(devNum);

/* retrieve PP Core Clock from HW */
    phase1Params->coreClock             = CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS;

    phase1Params->ppHAState             = CPSS_SYS_HA_MODE_ACTIVE_E;

    /* Eagle access mode */
    phase1Params->mngInterfaceType      = CPSS_CHANNEL_PEX_EAGLE_E;

    phase1Params->numOfPortGroups       = 1;

    numOfTiles = 1;/* default : we can not know from the 'PCI scan' */
    if(GT_OK == appDemoDbEntryGet("numOfTiles", &tmpValue))
    {
        if(tmpValue == 1 ||
           tmpValue == 2 ||
           tmpValue == 4 )
        {
            numOfTiles = tmpValue;
        }
    }

    if(mode100G)
    {
        /************************************************/
        /* 1 of every 2 MACs needs physical port number */
        /************************************************/
        portsFalcor = 2;
    }
    else
    {
        portsFalcor = 1;
    }

    /* set proper 'default' mode that can support the 'max MACs'*/
    if(modeMaxMac)
    {
        /* single tile : to support 65 MACs + 4 SDMAs */
        /* 2 tiles : to support 130 MACs + 8 SDMAs */
        /* 4 tiles : to support 258 MACs + 16 SDMAs */
        maxNumOfPhyPortsToUse = (numOfTiles * 128) / portsFalcor;

        if(maxNumOfPhyPortsToUse < 64)
        {
            /****************/
            /* minimal mode */
            /****************/
            maxNumOfPhyPortsToUse = 64;
        }
    }
    else
    {
        maxNumOfPhyPortsToUse = 128;  /*use 128 physical ports that gives 8K vlans or 64, 256, 512, 1024. 128 is the default */
    }

    cpssOsPrintf("The device considered with [%d] tiles to select [%d] 'ports mode' \n",
        numOfTiles,
        maxNumOfPhyPortsToUse);

    phase1Params->maxNumOfPhyPortsToUse = maxNumOfPhyPortsToUse;

    /* add Db Entry */
    rc = appDemoDxHwPpPhase1DbEntryInit(CAST_SW_DEVNUM(devNum),phase1Params,
        CPSS_98CX8520_CNS,/* dummy to represent 'falcon' family (one of falcon values) */
        NULL,NULL,NULL);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxHwPpPhase1DbEntryInit", rc);
    if(rc != GT_OK)
        return rc;

    /* check if appDemoDxHwPpPhase1DbEntryInit modified the value , and give indication about it */
    if(phase1Params->maxNumOfPhyPortsToUse != maxNumOfPhyPortsToUse)
    {
        cpssOsPrintf("NOTE : The 'ports mode' was explicitly changed to [%d] (from [%d]) \n",
            phase1Params->maxNumOfPhyPortsToUse,
            maxNumOfPhyPortsToUse);
    }

    return GT_OK;
}

/**
* @internal getPpPhase2ConfigSimple function
* @endinternal
*
* @brief   Returns the configuration parameters for cpssDxChHwPpPhase2Init().
*
* @param[in] devNum                   - The device number to get the parameters for.
* @param[in] devType                  - The Pp device type
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpPhase2ConfigSimple
(
    IN GT_SW_DEV_NUM       devNum,
    IN  CPSS_PP_DEVICE_TYPE         devType,
    OUT CPSS_DXCH_PP_PHASE2_INIT_INFO_STC  *phase2Params
)
{
    GT_STATUS                   rc;
    CPSS_PP_PHASE2_INIT_PARAMS  appDemo_ppPhase2Params = CPSS_PP_PHASE2_DEFAULT;
    GT_U32                      tmpData;
    GT_U32                      auDescNum,fuDescNum;
    GT_U32 rxDescNum = RX_DESC_NUM_DEF;
    GT_U32 txDescNum = TX_DESC_NUM_DEF;


    osMemSet(phase2Params,0,sizeof(CPSS_DXCH_PP_PHASE2_INIT_INFO_STC));

    if( appDemoDbEntryGet("useMultiNetIfSdma", &tmpData) == GT_NO_SUCH )
    {
        appDemo_ppPhase2Params.useMultiNetIfSdma = GT_TRUE;
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

    /* allow each tile with it's own number of AUQ size */
    auDescNum = AU_DESC_NUM_DEF * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
    /* allow each pipe with it's own number of FUQ size (for CNC upload) */
    /* NOTE: Fdb Upload still happen only on 'per tile' base (like AUQ) ...
       it is not per pipe triggered */
    fuDescNum = AU_DESC_NUM_DEF * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes;

    phase2Params->newDevNum                  = CAST_SW_DEVNUM(devNum);
    phase2Params->noTraffic2CPU = GT_FALSE;
    phase2Params->netifSdmaPortGroupId = 0;
    phase2Params->auMessageLength = CPSS_AU_MESSAGE_LENGTH_8_WORDS_E;

    /* mapping for 6.4 Emulator that due to lack of SDMA on the Linux OS */
    /* there is not enough SDMA allocation */
    /* Emulator fails with next error message:
        Allocating 400 bytes for DMA
        Allocating 38701 bytes for DMA
        memory DMA exhausted base=0xfae00000 curr=0xfafffac0 size=0x9740 end=0xfb000000
        Failed to allocate 38701 bytes, exiting
    */
    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles >= 2)
    {
        GT_U32  sdmaFactor;

        /* there is not enough DMA allocation for all the multiples */
        /* so keep the same alloc as in single tile */
        auDescNum /= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        fuDescNum /= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;

        /* let the CPU SDMAs ports get the same number as in single tile .    */
        /* we have max of 4 SDMAs per tile (so the numOfTiles >= sdmaFactor)  */
        /* currently in 12.8 we use only 8 out of 16 , so we need to allow them*/
        /* amount of resources like for 6.4                                   */
        /* otherwise test like : prvTgfIpv4NonExactMatchUcRouting will fail to*/
        /* send packets from the CPU because no resources (at least in WM win/linux) */
        if(numCpuSdmas)
        {
            sdmaFactor = (numCpuSdmas + 3)/4;

            rxDescNum /= sdmaFactor;
            txDescNum /= sdmaFactor;

            if(appDemoDbEntryGet("txGenNumOfDescBuff", &tmpData) == GT_OK)
            {
                /* explicitly stated .. do not override */
            }
            else
            {
                /* NOTE: the 512 is value used inside appDemoAllocateDmaMem(...)
                   if not set by the 'DB entry add' */

                rc = appDemoDbEntryAdd("txGenNumOfDescBuff", 512 / sdmaFactor);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    rc = appDemoDbEntryAdd("fuqUseSeparate", GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

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

    rc = appDemoAllocateDmaMem(devType,
                               /* Rx */
                               rxDescNum,RX_BUFF_SIZE_DEF,RX_BUFF_ALLIGN_DEF,
                               /* Tx */
                               txDescNum,
                               /* AUQ */
                               auDescNum,
                               &appDemo_ppPhase2Params);

    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoAllocateDmaMem", rc);
    if(rc != GT_OK)
        return rc;

    /* convert CPSS_PP_PHASE2_INIT_PARAMS to CPSS_DXCH_PP_PHASE2_INIT_INFO_STC */
    phase2Params->netIfCfg          = appDemo_ppPhase2Params.netIfCfg         ;
    phase2Params->auqCfg            = appDemo_ppPhase2Params.auqCfg           ;
    phase2Params->fuqUseSeparate    = appDemo_ppPhase2Params.fuqUseSeparate   ;
    phase2Params->fuqCfg            = appDemo_ppPhase2Params.fuqCfg           ;
    phase2Params->useSecondaryAuq   = appDemo_ppPhase2Params.useSecondaryAuq  ;
    phase2Params->useDoubleAuq      = appDemo_ppPhase2Params.useDoubleAuq     ;
    phase2Params->useMultiNetIfSdma = appDemo_ppPhase2Params.useMultiNetIfSdma;
    phase2Params->multiNetIfCfg     = appDemo_ppPhase2Params.multiNetIfCfg    ;

    return rc;
}


#ifdef GM_USED
GT_U32  useSinglePipeSingleDp = 1;
#else /*!GM_USED*/
GT_U32  useSinglePipeSingleDp = 0;
#endif /*!GM_USED*/

/* allow to modify the flag from the terminal */
GT_STATUS falcon_useSinglePipeSingleDp_Set(IN GT_BOOL useSingleDp)
{
    useSinglePipeSingleDp = useSingleDp;
    return GT_OK;
}

/**
* @internal falcon_initPortMappingStage function
* @endinternal
*
*/
static GT_STATUS falcon_initPortMappingStage
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PORT_MAP_STC *mapArrPtr;
    GT_U32 mapArrLen;

    switch(devType)
    {
        case CPSS_FALCON_ALL_DEVICES_CASES_MAC:
        default:
            if(useSinglePipeSingleDp == 0)
            {
                ARR_PTR_AND_SIZE_MAC(cpssApi_falcon_defaultMap, mapArrPtr, mapArrLen);
                mapArrLen = actualNum_cpssApi_falcon_defaultMap;
            }
            else
            {
                ARR_PTR_AND_SIZE_MAC(singlePipe_singleDp_cpssApi_falcon_defaultMap, mapArrPtr, mapArrLen);
            }
            break;
    }

    if(appDemoPortMapPtr && appDemoPortMapSize)
    {
        /* allow to 'FORCE' specific array by other logic */
        mapArrPtr = appDemoPortMapPtr;
        mapArrLen = appDemoPortMapSize;
    }

    rc = cpssDxChPortPhysicalPortMapSet(CAST_SW_DEVNUM(devNum), mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapSet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* it's for appdemo DB only */
    rc = appDemoDxChMaxMappedPortSet(CAST_SW_DEVNUM(devNum), mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChMaxMappedPortSet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal falcon_PortModeConfiguration function
* @endinternal
*
*/
static GT_STATUS falcon_PortModeConfiguration
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType
)
{
    GT_STATUS   rc;
    GT_U32  initSerdesDefaults;
    PortInitList_STC *portInitListPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;
    GT_U32 maxPhyPorts;

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
    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    switch(devType)
    {
        case CPSS_FALCON_ALL_DEVICES_CASES_MAC:
        default:
            if(useSinglePipeSingleDp == 0)
            {
                switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
                {
                    default:
                    case 0:
                    case 1:
                        if (maxPhyPorts == 64)
                        {
                            /* 100G mode*/
                             portInitListPtr = falcon_3_2_portInitlist_64_port_mode;
                        }
                        else if (maxPhyPorts == 256)
                        {
                           portInitListPtr = falcon_3_2_portInitlist_256_port_mode;
                             if(mode100G)
                            {
                                portInitListPtr = falcon_3_2_100G_portInitlist_256_port_mode;
                            }
                        }
                        else if (maxPhyPorts == 1024)
                        {
                            /* 50G mode, remote ports*/
                             portInitListPtr = falcon_3_2_portInitlist_1024_port_mode;
                             if(mode100G)
                            {
                                portInitListPtr = falcon_3_2_100G_portInitlist_1024_port_mode;
                            }
                        }
                        else /* 128, 512*/
                        {
                            portInitListPtr = falcon_3_2_portInitlist;
                            if(mode100G)
                            {
                                portInitListPtr = falcon_3_2_100G_portInitlist;
                            }
                        }
                        break;

                    case 2:
                        if(mode100G)
                        {
                            portInitListPtr = falcon_6_4_100G_portInitlist;
                        }
                        else if(modeMaxMac)
                        {
                            portInitListPtr = falcon_6_4_maxMAC_portInitlist;
                        }
                        else if (maxPhyPorts == 64)
                        {
                            /* 100G mode*/
                             portInitListPtr = falcon_6_4_portInitlist_64_port_mode;
                        }
                        else if (maxPhyPorts == 256)
                        {
                             portInitListPtr = falcon_6_4_portInitlist_256_port_mode;
                        }
                        else if (maxPhyPorts == 1024)
                        {
                             /* 50G mode, remote ports*/
                             portInitListPtr = falcon_6_4_portInitlist_1024_port_mode;
                        }
                        else /*128, 512*/
                        {
                            portInitListPtr = falcon_6_4_portInitlist;
                        }
                        break;

                    case 4:

                        if (maxPhyPorts == 64)
                        {
                            /* 100G mode*/
                             portInitListPtr = falcon_12_8_portInitlist_64_port_mode;
                        }
                        else if (maxPhyPorts == 256)
                        {
                            /* 50G mode*/
                            portInitListPtr = falcon_12_8_portInitlist_256_port_mode;
                        }
                        else if (maxPhyPorts == 1024)
                        {
                             /* 50G mode, remote ports*/
                             portInitListPtr = falcon_12_8_portInitlist_1024_port_mode;
                        }
                        else /*128, 512*/
                        {
                            portInitListPtr = falcon_12_8_portInitlist;
                        }

                        break;
                }

            }
            else
            {
                portInitListPtr = singlePipe_singleDp_portInitlist_falcon;
            }
            break;
    }

    if(falcon_force_PortsInitListPtr)
    {
        /* allow to 'FORCE' specific array by other logic */
        portInitListPtr = falcon_force_PortsInitListPtr;
    }

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        rc = appDemoBc2PortListInit(CAST_SW_DEVNUM(devNum),portInitListPtr,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* indication for function appDemoBc2PortInterfaceInit(...) to
           not call appDemoBc2PortListInit(...) */
        appDemo_PortsInitList_already_done = 1;
    }

    return GT_OK;
}

/**
* @internal falcon_initPhase1AndPhase2 function
* @endinternal
*
*/
static GT_STATUS falcon_initPhase1AndPhase2
(
    IN GT_SW_DEV_NUM       devNum,
    IN CPSS_HW_INFO_STC   *hwInfoPtr,
    IN GT_U32             devIndex,
    OUT CPSS_PP_DEVICE_TYPE     *devTypePtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC       cpssPpPhase1Info;     /* CPSS phase 1 PP params */
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC       cpssPpPhase2Info;     /* CPSS phase 2 PP params */
    static CPSS_REG_VALUE_INFO_STC regCfgList[] = GT_DUMMY_REG_VAL_INFO_LIST;            /* register values */
    GT_U32                  regCfgListSize;         /* Number of config functions for this board    */
    GT_HW_DEV_NUM           hwDevNum;
    CPSS_PP_DEVICE_TYPE     devType;
    GT_U32                  waIndex;
    static CPSS_DXCH_IMPLEMENT_WA_ENT falcon_WaList[] =
    {
        /* no current WA that need to force CPSS */
        CPSS_DXCH_IMPLEMENT_WA_LAST_E
    };

    osMemSet(&cpssPpPhase1Info, 0, sizeof(cpssPpPhase1Info));
    osMemSet(&cpssPpPhase2Info, 0, sizeof(cpssPpPhase2Info));


    rc = getPpPhase1ConfigSimple(devNum, &cpssPpPhase1Info);
    if (rc != GT_OK)
        return rc;

    cpssPpPhase1Info.hwInfo[0] = *hwInfoPtr;

    /* devType is retrieved in hwPpPhase1Part1*/
    rc = cpssDxChHwPpPhase1Init(&cpssPpPhase1Info, &devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpPhase1Init", rc);
    if (rc != GT_OK)
        return rc;

    *devTypePtr = devType;


    /* update the appDemo DB */
    rc = falcon_appDemoDbUpdate(devIndex, devNum , devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_appDemoDbUpdate", rc);
    if (rc != GT_OK)
        return rc;


#ifdef LINUX_NOKM
    rc = prvNoKmDrv_configure_dma_per_devNum(CAST_SW_DEVNUM(devNum),0/*portGroup*/);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("prvNoKmDrv_configure_dma_per_devNum", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
#endif /*LINUX_NOKM*/

    /* should be 10 */
    regCfgListSize = (sizeof(regCfgList)) /
                       (sizeof(CPSS_REG_VALUE_INFO_STC));

    rc = cpssDxChHwPpStartInit(CAST_SW_DEVNUM(devNum), GT_FALSE, regCfgList, regCfgListSize);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpStartInit", rc);
    if (rc != GT_OK)
        return rc;

    waIndex = 0;
    while(falcon_WaList[waIndex] != CPSS_DXCH_IMPLEMENT_WA_LAST_E)
    {
        waIndex++;
    }

    if(waIndex)
    {
        rc = cpssDxChHwPpImplementWaInit(CAST_SW_DEVNUM(devNum),waIndex,falcon_WaList, NULL);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    /*************************************************/
    /* fill once the DB of 'port mapping' to be used */
    /*************************************************/
    rc = fillDbForCpssPortMappingInfo(devNum);
    if (rc != GT_OK)
        return rc;

    rc = falcon_initPortMappingStage(devNum,devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_initPortMappingStage", rc);
    if (GT_OK != rc)
        return rc;

    /* memory related data, such as addresses and block lenghts, are set in this funtion*/
    rc = getPpPhase2ConfigSimple(devNum, devType, &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;

    rc = cpssDxChHwPpPhase2Init(CAST_SW_DEVNUM(devNum),  &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;

    hwDevNum = (devNum + appDemoHwDevNumOffset) & 0x3FF;

    /* set HWdevNum related values */
    rc = appDemoDxHwDevNumChange(CAST_SW_DEVNUM(devNum), hwDevNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal falcon_getNeededLibs function
* @endinternal
*
*/
static GT_STATUS falcon_getNeededLibs(
    OUT  APP_DEMO_LIB_INIT_PARAMS    *libInitParamsPtr,
    OUT  CPSS_PP_CONFIG_INIT_STC     *sysConfigParamsPtr
)
{
    osMemSet(libInitParamsPtr,0,sizeof(*libInitParamsPtr));
    osMemSet(sysConfigParamsPtr,0,sizeof(*sysConfigParamsPtr));

    libInitParamsPtr->initBridge = GT_TRUE;/*prvBridgeLibInit*/
    libInitParamsPtr->initIpv4   = GT_TRUE;/*prvIpLibInit*/
    libInitParamsPtr->initIpv6   = GT_TRUE;/*prvIpLibInit*/
    libInitParamsPtr->initPcl    = GT_TRUE;/*prvPclLibInit*/
    libInitParamsPtr->initTcam   = GT_TRUE;/*prvTcamLibInit*/
    libInitParamsPtr->initPort   = GT_TRUE;/*prvPortLibInit*/
    libInitParamsPtr->initPhy    = GT_TRUE;/*prvPhyLibInit*/
    libInitParamsPtr->initNetworkIf = GT_TRUE;/*prvNetIfLibInit*/
    libInitParamsPtr->initMirror = GT_TRUE;/*prvMirrorLibInit*/
    libInitParamsPtr->initTrunk  = GT_TRUE;/*prvTrunkLibInit*/
    libInitParamsPtr->initPha    = GT_TRUE;/*prvPhaLibInit*/

    sysConfigParamsPtr->numOfTrunks         = _4K;    /*used by prvTrunkLibInit*/
    sysConfigParamsPtr->lpmDbSupportIpv4    = GT_TRUE;/*used by prvIpLibInit*/
    sysConfigParamsPtr->lpmDbSupportIpv6    = GT_TRUE;/*used by prvIpLibInit*/
    /*NOTYE: info sysConfigParamsPtr->lpmRamMemoryBlocksCfg is set later : used by prvIpLibInit*/

    sysConfigParamsPtr->pha_packetOrderChangeEnable = GT_FALSE;/*used by prvPhaLibInit*/

    return GT_OK;
}



/**
* @internal appDemoFalconIpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] sharedTableMode          - shared tables mode
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoFalconIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT      sharedTableMode,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
)
{
    GT_U32 value;

    devNum = devNum;

    if(appDemoDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        ramDbCfgPtr->maxNumOfPbrEntries = value;
    else
        ramDbCfgPtr->maxNumOfPbrEntries = maxNumOfPbrEntries;

    if(appDemoDbEntryGet("sharedTableMode", &value) == GT_OK)
        ramDbCfgPtr->sharedMemCnfg = value;
    else
        ramDbCfgPtr->sharedMemCnfg = sharedTableMode;

    ramDbCfgPtr->lpmMemMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;/*the only mode for Falcon*/
        if(appDemoDbEntryGet("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", &value) == GT_OK)
        ramDbCfgPtr->blocksAllocationMethod  = (CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT)value;
    else
        ramDbCfgPtr->blocksAllocationMethod  = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;

    return GT_OK;
}




/**
* @internal falcon_initPpLogicalInit function
* @endinternal
*
*/
static GT_STATUS falcon_initPpLogicalInit
(
    IN GT_SW_DEV_NUM       devNum,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC    *lpmRamMemoryBlocksCfgPtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PP_CONFIG_INIT_STC    ppLogInitParams;
    GT_U32 value = 0;

    osMemSet(&ppLogInitParams ,0, sizeof(ppLogInitParams));

    ppLogInitParams.maxNumOfPbrEntries = _8K;/* same as SIP5 devices */
    ppLogInitParams.lpmMemoryMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    ppLogInitParams.sharedTableMode = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;

    if(appDemoDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
    {
        ppLogInitParams.maxNumOfPbrEntries = value;
    }

    if(appDemoDbEntryGet("sharedTableMode", &value) == GT_OK)
    {
        ppLogInitParams.sharedTableMode = value;
    }

    rc = cpssDxChCfgPpLogicalInit(CAST_SW_DEVNUM(devNum), &ppLogInitParams);
    if(rc != GT_OK)
        return rc;

    /* function must be called after cpssDxChCfgPpLogicalInit that update the value of :
        PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam */
    /* calculate info needed by 'prvDxCh2Ch3IpLibInit' (init of LPM manager) */
    rc = appDemoFalconIpLpmRamDefaultConfigCalc(CAST_SW_DEVNUM(devNum),
                                                ppLogInitParams.sharedTableMode,
                                                ppLogInitParams.maxNumOfPbrEntries,
                                                lpmRamMemoryBlocksCfgPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal falcon_appDemoDbUpdate function
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
static GT_STATUS falcon_appDemoDbUpdate
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_PP_DEVICE_TYPE     devType
)
{
    appDemoPpConfigList[devIndex].valid = GT_TRUE;
    appDemoPpConfigList[devIndex].devNum = CAST_SW_DEVNUM(devNum);
    appDemoPpConfigList[devIndex].deviceId = devType;
    appDemoPpConfigList[devIndex].devFamily = CPSS_PP_FAMILY_DXCH_FALCON_E;
    appDemoPpConfigList[devIndex].apiSupportedBmp = APP_DEMO_DXCH_FUNCTIONS_SUPPORT_CNS;

    return GT_OK;
}

/**
* @internal falcon_appDemoInitSequence function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for Falcon device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS falcon_appDemoInitSequence
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    GT_STATUS   rc;
    CPSS_PP_DEVICE_TYPE     devType;
    APP_DEMO_LIB_INIT_PARAMS    libInitParams;
    CPSS_PP_CONFIG_INIT_STC     sysConfigParams;
    GT_BOOL supportSystemReset = GT_TRUE;

    /* Set debug devType if needed */
    rc = appDemoDebugDeviceIdGet(CAST_SW_DEVNUM(devNum), &devType);
    if (GT_OK == rc)
    {
        rc = prvCpssDrvDebugDeviceIdSet(CAST_SW_DEVNUM(devNum), devType);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDrvDebugDeviceIdSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = falcon_initPhase1AndPhase2(devNum,hwInfoPtr,devIndex,&devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_initPhase1AndPhase2", rc);
    if (GT_OK != rc)
        return rc;

    rc = falcon_PortModeConfiguration(devNum,devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_PortModeConfiguration", rc);
    if (rc != GT_OK)
        return rc;

    rc = falcon_getNeededLibs(&libInitParams,&sysConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_getNeededLibs", rc);
    if(rc != GT_OK)
        return rc;

    rc = falcon_initPpLogicalInit(devNum,&sysConfigParams.lpmRamMemoryBlocksCfg);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_initPpLogicalInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoDxLibrariesInit(CAST_SW_DEVNUM(devNum),&libInitParams,&sysConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxLibrariesInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoDxPpGeneralInit(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxPpGeneralInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoBc2PIPEngineInit(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBc2PIPEngineInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoB2PtpConfig(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoB2PtpConfig", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoDxTrafficEnable(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxTrafficEnable", rc);
    if(rc != GT_OK)
        return rc;

    #ifdef GM_USED
    /* the GM not supports the 'soft reset' so we can not support system with it */
    supportSystemReset = GT_FALSE;
    #endif /*GM_USED*/
    if(cpssDeviceRunCheck_onEmulator())
    {
        CPSS_TBD_BOOKMARK_FALCON_EMULATOR
        /* until we make it happen ... do not try it on emulator ! */
        supportSystemReset = GT_FALSE;
    }

    rc = cpssDevSupportSystemResetSet(CAST_SW_DEVNUM(devNum), supportSystemReset);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevSupportSystemResetSet", rc);
    if (GT_OK != rc)
        return rc;

    return GT_OK;
}

/**
* @internal falcon_EventHandlerInit function
* @endinternal
*
*/
static GT_STATUS falcon_EventHandlerInit
(
    void
)
{
    GT_STATUS   rc;
    GT_U32 value;

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
    else
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

/**
* @internal falcon_localUtfInit function
* @endinternal
*
*/
static GT_STATUS falcon_localUtfInit
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

    appDemoFalconMainUtForbidenTests();

    return rc;
}

/**
* @internal falcon_EgfEftPortFilter function
* @endinternal
*
*/
static GT_STATUS falcon_EgfEftPortFilter
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           devIdx
)
{
    GT_STATUS   rc;
    GT_U32  portNum;
    GT_BOOL isLinkUp;

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles < 2)
    {
        return GT_OK;
    }

/*       as the interrupts from the device are not implemented , check manually
 *       if port is UP and set it as 'link UP' in the EGF-EFT unit */

    for(portNum = 64 ; portNum < appDemoPpConfigList[devIdx].maxPortNumber ; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(CAST_SW_DEVNUM(devNum),portNum);
        if (appDemoDxChSkipSip6RemotePorts(CAST_SW_DEVNUM(devNum),portNum) == GT_TRUE)
        {
            continue;
        }
        rc = cpssDxChPortLinkStatusGet(CAST_SW_DEVNUM(devNum),
            portNum,&isLinkUp);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortLinkStatusGet", rc);
            return rc;
        }

        if(isLinkUp == GT_TRUE)
        {
            rc = cpssDxChBrgEgrFltPortLinkEnableSet(CAST_SW_DEVNUM(devNum),
                portNum,
                    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E  /*don't filter*/);
            if(rc != GT_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgEgrFltPortLinkEnableSet", rc);
                return rc;
            }
        }
    }


    return GT_OK;
}

extern GT_STATUS prvCpssHwInitSip5IsInitTcamDefaultsDone(IN GT_U8   devNum);
/**
* @internal falcon_waitForTcamInitDone function
* @endinternal
*
*/
static GT_STATUS falcon_waitForTcamInitDone
(
    IN GT_SW_DEV_NUM    devNum
)
{
    if(!cpssDeviceRunCheck_onEmulator())
    {
        return GT_OK;
    }

    /* on the emulator , in order not to wait for it to finish all 36K entries ,
       we allow it to run in parallel , and only now we check if the default values
       finished to load */
    return prvCpssHwInitSip5IsInitTcamDefaultsDone(CAST_SW_DEVNUM(devNum));
}

/**
* @internal gtDbFalconBoardReg_SimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for Falcon device.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbFalconBoardReg_SimpleInit
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
    GT_U32      tmpData;

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */

    UNUSED_PARAM_MAC(boardRevId);

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
    if(dx_simUserDefinedCoreClockMHz)
    {
        simCoreClockOverwrite(dx_simUserDefinedCoreClockMHz);
    }
#endif

    /* Enable printing inside interrupt routine - supplied by extrernal drive */
    extDrvUartInit();

    /* Call to fatal_error initialization, use default fatal error call_back - supplied by mainOs */
    rc = osFatalErrorInit((FATAL_FUNC_PTR)NULL);
    if (rc != GT_OK)
        return rc;

    /* this function finds all Prestera devices on PCI bus */
    rc = falcon_getBoardInfoSimple(
        &hwInfo[0],
        &pciInfo);
    if (rc != GT_OK)
        return rc;

    appDemoPpConfigDevAmount = 1;

    falcon_boardTypePrint("DB" /*boardName*/, "Falcon" /*devName*/);

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    devIndex = 0;

    devNum = SYSTEM_DEV_NUM_MAC(devIndex);

    if(appDemoDbEntryGet("modeMaxMac", &tmpData) == GT_OK)
    {
        modeMaxMac = 1;
    }
    else
    {
        modeMaxMac = 0;
    }

    if(appDemoDbEntryGet("mode100G", &tmpData) == GT_OK)
    {
        mode100G = 1;
    }
    else
    {
        mode100G = 0;
    }



    rc = falcon_appDemoInitSequence(devIndex, devNum, hwInfo);
    if (rc != GT_OK)
        return rc;

    /* init the event handlers */
    rc = falcon_EventHandlerInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_EventHandlerInit", rc);
    if (rc != GT_OK)
        return rc;

    rc = falcon_localUtfInit(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_localUtfInit", rc);
    if (rc != GT_OK)
        return rc;

    rc = falcon_EgfEftPortFilter(devNum,devIndex);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_EgfEftPortFilter", rc);
    if (rc != GT_OK)
        return rc;

    rc = falcon_waitForTcamInitDone(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_waitForTcamInitDone", rc);
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
* @internal gtDbFalconBoardReg_BoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbFalconBoardReg_BoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    UNUSED_PARAM_MAC(boardRevId);

    appDemo_PortsInitList_already_done = 0;

    return GT_OK;
}
/**
* @internal gtDbPxFalconBoardReg function
* @endinternal
*
* @brief   Registration function for the Falcon board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbPxFalconBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    UNUSED_PARAM_MAC(boardRevId);

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardSimpleInit               = gtDbFalconBoardReg_SimpleInit;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbFalconBoardReg_BoardCleanDbDuringSystemReset;

    return GT_OK;
}

/************************************************************************/
/* Tracing specific registers access */
/************************************************************************/
#if 0
    /** Range parameteres: Base, size */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,
    /** Range parameteres: base, step, amount, size  */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E,
    /** Range parameteres: addr */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E,
    /** Range parameteres: base, step, amount  */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,
    /** marks end of array */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_LAST_E
#endif
static APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC falconPakcetBufferParseRegsArr[] =
{
     {0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x1, "TILE", {0x00000000, 0x20000000, 4, 0x20000000}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x2  , "PB_CENTER", {0x19000000, 0x00010000, 0}}
    ,{0x2, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,   0x100, "center",    {0x1000, 0x1000, 0}}
    ,{0x2, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,   0x200, "counter", {0x0000, 0x1000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x300, "PB_SMB_WRITE_ARBITER", {0x19030000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x3  , "PB_GPC_GRP_PACKET_WRITE_0", {0x19040000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x4,   "PB_GPC_GRP_CELL_READ_0", {0x19060000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x4,   "PB_GPC_GRP_CELL_READ_1", {0x19070000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x5,   "PB_GPC_GRP_PACKET_READ_0", {0x19080000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x5,   "PB_GPC_GRP_PACKET_READ_1", {0x19090000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x3,   "PB_GPC_GRP_PACKET_WRITE_1", {0x190A0000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x4,   "PB_GPC_GRP_CELL_READ_2", {0x190C0000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x4,   "PB_GPC_GRP_CELL_READ_3", {0x190D0000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x5,   "PB_GPC_GRP_PACKET_READ_2", {0x190E0000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x5,   "PB_GPC_GRP_PACKET_READ_3", {0x190F0000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x400, "PB_NEXT_POINTER_MEMO_0", {0x19100000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x400, "PB_NEXT_POINTER_MEMO_1", {0x19110000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x400, "PB_NEXT_POINTER_MEMO_2", {0x19120000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_0_0", {0x19130000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_0_1", {0x19140000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_1_0", {0x19150000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_1_1", {0x19160000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_2_0", {0x19170000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_2_1", {0x19180000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x1000, "CTRL_PIPE0", {0x08000000, 0x08000000, 0, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x1000, "CTRL_PIPE1", {0x10000000, 0x08000000, 0, 0}}
    ,{0x3,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x600, "main",         {0x1000, 0x1000}}
    ,{0x3,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x700, "packetWrite",  {0x0000, 0x0400, 4, 0x0400}}
    ,{0x4,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x800, "main",         {0x0200, 0x0100}}
    ,{0x4,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x900, "cellRead",     {0x0000, 0x0100, 2, 0x0100}}
    ,{0x5,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0xA00, "main",         {0x2000, 0x1000}}
    ,{0x5,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0xB00, "packetRead",   {0x0000, 0x1000, 2, 0x1000}}
    /* pbCenter*/
    ,{0x100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "npmRandomizerEnable", {0x0000}}
    ,{0x100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "tileId",              {0x0404}}
    ,{0x100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "tileMode",            {0x0408}}
    ,{0x100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "mswCollapseCfg",      {0x0410}}
    ,{0x100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "badAddress",          {0x0414}}
    /* pbCointer in  pbCenter*/
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "enablePbc",           {0x000C}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgTile",             {0x002C}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "enableMasterMode",    {0x0010}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "enableInterTileTransact",{0x0014}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeSmbCells",      {0x001C}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeSmbMcCells",    {0x0020}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeSmbSumCells",   {0x0030}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeNpmWords",      {0x0024}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeNpmMcWords",    {0x0028}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeNpmSumWords",   {0x0034}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgFillThreshold",    {0x0038}}
    /* pbSmbWriteArbiter */
    ,{0x300, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgEnable",           {0x0004}}
    /* gpcGrpPacketWrite */
    ,{0x600, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "npmRandomizerEnable", {0x0014}}
    ,{0x600, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "badAddress",          {0x0018}}
    /* gpcPacketWrite */
    ,{0x700, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgEnable",           {0x0000}}
    ,{0x700, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgList",             {0x0004}}
    ,{0x700, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgNxp",              {0x0008}}
    ,{0x700, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgWrt",              {0x000C}}
    ,{0x700, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgStat",             {0x0010}}
    /* gpcGrpCellRead */
    ,{0x800, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "badRegister",         {0x0014}}
    /* gpcCellRead */
    ,{0x900, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "initialization",      {0x0000}}
    ,{0x900, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "configuration",       {0x0004}}
    ,{0x900, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "aging",               {0x0008}}
    /* gpcGrpPacketRead */
    ,{0xA00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "badAddress",          {0x0014}}
    /* gpcPacketRead */
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "initialization",      {0x0000}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "miscEnable",          {0x0004}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "aging",               {0x0008}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "channelEnable",{0x0080, 4, 10}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "channelReset", {0x0180, 4, 10}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "channelShaper",{0x0280, 4, 10}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "packetCountEnable",   {0x0380}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "packetCountReset",    {0x0384}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "fifoSegmentStart",{0x0700, 4, 10}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "fifoSegmentNext", {0x0400, 4, 20}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgUpdateTrigger",    {0x0800}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E,        0, "arbiterPointHighSpeedChannel0En", {0x0804}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "arbiterPointGroup1", {0x0814, 4, 5}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "arbiterPointGroup2", {0x0828, 4, 5}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "arbiterCalendarSlot",{0x083C, 4, 32}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "arbiterCalendarMax",  {0x08BC}}
    /* npmMc */
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgEnable",           {0x0000}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgInit",             {0x0004}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgUnit",             {0x0300}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgRefs",             {0x0400}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "clrRefs",      {0x0410, 4, 4}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "insList",      {0x0210, 4, 4}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "intReinsertMask",     {0x011C}}
    /* smbMc */
    ,{0x500, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgEnable",           {0x0004}}
    ,{0x500, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgInit",             {0x0000}}
    /* TX_DMA*/
    ,{0x1000, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x1100, "TX_DMA",      {0x05560000, 0x20000, 4, 0x10000}}
    ,{0x1100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "TXD Channel Reset", {0x2100, 4, 9}}
    ,{0x1100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "TDS Channel Reset",        {0x4004}}

    /* end of list */
    ,{0, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_LAST_E}
};

void appDemoHwPpDrvRegTracingFalconPb()
{
    appDemoHwPpDrvRegTracingRegNameBind(falconPakcetBufferParseRegsArr);
    appDemoHwPpDrvRegTracingRegAddressesRangeSet(
        0xDFE00000, 0x19000000, /* Packet Buffer */
        0xDFFF0000, 0x0D560000, /* Pipe0 TX_DMA0 */ /* TX_FIFO 0x0D570000 sipped */
        0xDFFF0000, 0x0D580000, /* Pipe0 TX_DMA1 */
        0xDFFF0000, 0x0D5A0000, /* Pipe0 TX_DMA2 */
        0xDFFF0000, 0x0D5C0000, /* Pipe0 TX_DMA3 */
        0xDFFF0000, 0x15560000, /* Pipe1 TX_DMA0 */
        0xDFFF0000, 0x15580000, /* Pipe1 TX_DMA1 */
        0xDFFF0000, 0x155A0000, /* Pipe1 TX_DMA2 */
        0xDFFF0000, 0x155C0000, /* Pipe1 TX_DMA3 */
        0);
    appDemoHwPpDrvRegTracingBind(1);
}

GT_STATUS appDemoTxqDebugHelp
(
  GT_VOID
)
{
    cpssOsPrintf ("  1.Dump scheduler errors count and type - prvCpssDxChTxqFalconDumpSchedErrors (devNum)\n");
    cpssOsPrintf ("  2.Dump queue token bucket level and deficit - prvCpssDxChTxqDumpQueueStatus (devNum,tileNum,queueStart,queueEnd)\n");
    cpssOsPrintf ("  3.Dump A node (port) token bucket level and deficit - prvCpssDxChTxqDumpAlevelStatus(devNum,tileNum,aLevelStart,aLevelEnd)\n");
    cpssOsPrintf ("  4.Dump queue shaping configuration parameters - prvCpssDxChTxqDumpQueueShapingParameters(devNum,tileNum,queueStart,queueEnd)\n");
    cpssOsPrintf ("  5.Dump A node (port)  shaping configuration parameters - prvCpssDxChTxqDumpAnodeShapingParameters(devNum,tileNum,aLevelStart,aLevelEnd)\n");
    cpssOsPrintf ("  6.Dump PDX routing table and PDX pizza arbiter -  prvCpssFalconTxqUtilsPdxTablesDump(devNum,tileNum)\n");
    cpssOsPrintf ("  7.Dump PDQ queue to SDQ/PDS mapping -  prvCpssFalconTxqUtilsPsiConfigDump(devNum,tileNum)\n");
    cpssOsPrintf ("  8.Dump TxQ descriptor counters for specific tile and PDS  -  prvCpssDxChTxqFalconDescCounterDump(devNum,tileNum,pdsNum,queueStart,queueEnd)\n");
    cpssOsPrintf ("  9.Dump schedule profile attributes  -  prvCpssFalconTxqUtilsDumpSchedDbProfiles(devNum)\n");
    cpssOsPrintf ("  10.Dump mapping of local port to queues at SDQ -  prvCpssFalconTxqUtilsSdqPortRangeDump(devNum,tileNum,sdqNum)\n");
    cpssOsPrintf ("  11.Dump  dump queue attributes -  prvCpssFalconTxqUtilsSdqQueueAttributesDump(devNum,tileNum,sdqNum,num of q to dump)\n");
    cpssOsPrintf ("  12.Print port to schedule profile binding -  prvCpssFalconTxqUtilsDumpBindPortsToSchedDb(devNum)\n");
     cpssOsPrintf (" 13.Dump  physical port to P node and A node mapping(also PDQ Q numbers). -  prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump(devNum,tileNum)\n");
     cpssOsPrintf (" 14.Print the configure tree under a specific port (SW data base ,no read from HW) -  prvCpssFalconTxqPdqDumpPortSw(devNum,tileNum,portNum)\n");
     cpssOsPrintf (" 15.Print the configure tree under a specific port ( read from HW) -  prvCpssFalconTxqPdqDumpPortHw(devNum,tileNum,portNum)\n");

    return GT_OK;
}


#ifdef INCLUDE_UTF
extern GT_STATUS utfAddPreSkippedRule
(
    IN char*        suitLow,
    IN char*        suitHigh,
    IN char*        testLow,
    IN char*        testHigh
);
#else
    /* dummy for compilation */
    #define utfAddPreSkippedRule(a,b,c,d)   cpssOsPrintf("[%p][%p][%p][%p]\n",a,b,c,d)
#endif/*INCLUDE_UTF*/

typedef struct{
    IN char*        testName;
}FORBIDEN_TESTS;
typedef struct{
    IN char*        suiteName;
}FORBIDEN_SUITES ;

static void appDemoFalconForbidenTests(IN FORBIDEN_TESTS   forbidenTests[])
{
    GT_U32  ii = 0;

    while(forbidenTests[ii].testName)
    {
        utfAddPreSkippedRule(
            "",
            "",
            forbidenTests[ii].testName,
            forbidenTests[ii].testName);

        ii++;
    }

    return;
}
static void appDemoFalconForbidenSuites(IN FORBIDEN_SUITES   forbidenSuites[])
{
    GT_U32  ii = 0;

    while(forbidenSuites[ii].suiteName)
    {
        utfAddPreSkippedRule(
            forbidenSuites[ii].suiteName,
            forbidenSuites[ii].suiteName,
            "",
            "");

        ii++;
    }

    return;
}


/* make suite name and test name without "" to become a string */
#define TEST_NAME(test)    {#test}
/* make suite name to become a string */
#define SUITE_NAME(suite)   {#suite}
/*
    MainUT tests:
    list of tests/suites that forbidden because crash the CPSS or the device
   (fatal error in simulation on unknown address)
*/
static GT_STATUS   appDemoFalconMainUtForbidenTests(void)
{
    static FORBIDEN_TESTS mainUt_forbidenTests_CRASH[] =
    {
        /* test CRASH or FATAL ERROR ! */
         TEST_NAME(cpssDxChDiagResetAndInitControllerRegsNumGet    )
        /* test CRASH or FATAL ERROR ! */
        ,TEST_NAME(cpssDxChLedBc3FELedPosition                     )

        /* test fail an may fail others */
        /*doing remap that fail in TQX scheduler :
            ERROR GT_BAD_PARAM in function: falcon_xel_to_cpss_err_code, file prvcpssdxchtxqpdq.c, line[137].
            [prvCpssFalconTxqPdqPeriodicSchemeConfig , line[415]] failed : in pdqNum[0] (retCode_xel[-1])

            but this test may fail others
            */
        ,TEST_NAME(cpssDxChPortPhysicalPortMapSet                  )

        ,{NULL}/* must be last */
    };

    /* suites that most or all tests CRASH or FATAL ERROR ! */
    static FORBIDEN_SUITES mainUt_forbidenSuites_CRASH [] =
    {
         SUITE_NAME(cpssDxChIpLpm                                  )
        ,SUITE_NAME(cpssDxChDiagDataIntegrityTables                )
        ,SUITE_NAME(cpssDxChPortBufMg                              )
        ,SUITE_NAME(cpssDxChPortPfc                                )
        ,SUITE_NAME(cpssDxChPortCtrl                               )
        ,SUITE_NAME(cpssDxChPortStat                               )
        ,SUITE_NAME(cpssDxChPortEee                                )
        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_many_tests_fail [] =
    {
         TEST_NAME(/*cpssGenSmi.*/cpssSmiRegisterRead                                            )
        ,TEST_NAME(/*cpssGenSmi.*/cpssSmiRegisterWrite                                           )
        ,TEST_NAME(/*cpssDxChCscd.*/cpssDxChCscdHyperGPortCrcModeSet                             )
        ,TEST_NAME(/*cpssDxChCscd.*/cpssDxChCscdHyperGPortCrcModeGet                             )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesStatusGet                              )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTestEnableSet                          )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTestEnableGet                          )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTransmitModeSet                        )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTransmitModeGet                        )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagDeviceTemperatureThresholdSet                    )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagDeviceVoltageGet                                 )
        ,TEST_NAME(/*cpssDxChDiagDataIntegrity.*/cpssDxChDiagDataIntegrityErrorInfoGet           )
        ,TEST_NAME(/*cpssDxChHwInit.*/cpssDxChHwAccessInfoStoreTest                              )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamPortGroupClassManipulationSet      )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamPortGroupClassManipulationGet      )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamPortGroupConfigSet                 )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamPortGroupConfigGet                 )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamPortGroupGroupConfigSet            )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamPortGroupGroupConfigGet            )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamClassManipulationGet               )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamClassManipulationSet               )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamConfigGet                          )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamConfigSet                          )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamGroupConfigSet                     )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamGroupConfigGet                     )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamPortClassPolarityInvertEnableSet   )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamPortClassPolarityInvertEnableGet   )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamPortClassIndicationSet             )
        ,TEST_NAME(/*cpssDxChHwInitLedCtrl.*/cpssDxChLedStreamPortClassIndicationGet             )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyPortAddrSet                                     )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyPortAddrGet                                     )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyAutonegSmiGet                                   )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyAutonegSmiSet                                   )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiAutoMediaSelectSet                           )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiAutoMediaSelectGet                           )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiMdcDivisionFactorGet                         )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiMdcDivisionFactorSet                         )
        ,TEST_NAME(/*cpssDxChPhySmiPreInit.*/cpssDxChPhyAutoPollNumOfPortsSet                    )
        ,TEST_NAME(/*cpssDxChPhySmiPreInit.*/cpssDxChPhyAutoPollNumOfPortsGet                    )
        ,TEST_NAME(/*cpssDxChPortEcn.*/cpssDxChPortEcnMarkingTailDropProfileEnableSet            )
        ,TEST_NAME(/*cpssDxChPortEcn.*/cpssDxChPortEcnMarkingTailDropProfileEnableGet            )
        ,TEST_NAME(/*cpssDxChPortIfModeCfg.*/cpssDxChPortModeSpeedSet                            )
        ,TEST_NAME(/*cpssDxChPortPip.*/cpssDxChPortPipDropCounterGet                             )
        ,TEST_NAME(/*cpssDxChVnt.*/cpssDxChVntOamPortUnidirectionalEnableSet                     )
        ,TEST_NAME(/*cpssDxChVnt.*/cpssDxChVntOamPortUnidirectionalEnableGet                     )
        ,TEST_NAME(/*cpssDxChDiagBist.*/cpssDxChDiagBistResultsGet                               )
        ,TEST_NAME(/*cpssDxChDiagBist.*/cpssDxChDiagBistTriggerAllSet                            )
        ,TEST_NAME(/*cpssDxChDiagBist.*/cpssDxChDiagBistTriggerAllSetWithError                   )
        ,TEST_NAME(/*cpssDxChHwInitDeviceMatrix.*/cpssDxChHwInitDeviceMatrixCheckTest            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQueueingEnableSet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQueueTxEnableSet                             )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxFlushQueuesSet                               )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperEnableSet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperEnableGet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperProfileSet                             )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperProfileGet                             )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQShaperEnableSet                             )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQShaperEnableGet                             )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQShaperProfileSet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQShaperProfileGet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBindPortToSchedulerProfileSet                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBindPortToSchedulerProfileGet                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTx4TcTailDropProfileSet                        )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTx4TcTailDropProfileGet                        )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropProfileSet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropProfileGet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSniffedPcktDescrLimitSet                     )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSniffedPcktDescrLimitGet                     )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxMcastPcktDescrLimitSet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxWrrGlobalParamSet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxWrrGlobalParamGet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperGlobalParamsSet                        )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQWrrProfileSet                               )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQWrrProfileGet                               )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQArbGroupSet                                 )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQArbGroupGet                                 )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBindPortToDpSet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBindPortToDpGet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxDescNumberGet                                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBufNumberGet                                 )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTx4TcDescNumberGet                             )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTx4TcBufNumberGet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxToCpuShaperModeSet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxToCpuShaperModeGet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSharingGlobalResourceEnableSet               )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSharingGlobalResourceEnableGet               )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSharedGlobalResourceLimitsSet                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSharedGlobalResourceLimitsGet                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxWatchdogEnableGet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxWatchdogEnableSet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropUcEnableSet                          )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropUcEnableGet                          )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBufferTailDropEnableSet                      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBufferTailDropEnableGet                      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBuffersSharingMaxLimitSet                    )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBuffersSharingMaxLimitGet                    )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxDp1SharedEnableSet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxDp1SharedEnableGet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTcSharedProfileEnableSet                     )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTcSharedProfileEnableGet                     )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxMcastPcktDescrLimitGet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxRandomTailDropEnableGet                      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxRandomTailDropEnableSet                      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperGlobalParamsGet                        )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperTokenBucketMtuSet                      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperTokenBucketMtuGet                      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChTxPortAllShapersDisable                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChTxPortShapersDisable                               )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxInit                                         )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSharedPolicySet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSharedPolicyGet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperModeSet                                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperModeGet                                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperConfigurationSet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperConfigurationGet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperBaselineSet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperBaselineGet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSchedulerDeficitModeEnableSet                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSchedulerDeficitModeEnableGet                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxRequestMaskSet                               )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxRequestMaskGet                               )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxRequestMaskIndexSet                          )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxRequestMaskIndexGet                          )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxGlobalBufNumberGet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxGlobalDescNumberGet                          )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxMcastBufNumberGet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxMcastBuffersLimitGet                         )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxMcastBuffersLimitSet                         )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxMcastDescNumberGet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxProfileWeightedRandomTailDropEnableGet       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxProfileWeightedRandomTailDropEnableSet       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxResourceHistogramCounterGet                  )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxResourceHistogramThresholdGet                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxResourceHistogramThresholdSet                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSharedPoolLimitsSet                          )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSharedPoolLimitsGet                          )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSharedResourceBufNumberGet                   )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSharedResourceDescNumberGet                  )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSniffedBufNumberGet                          )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSniffedBuffersLimitGet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSniffedBuffersLimitSet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSniffedDescNumberGet                         )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropBufferConsumptionModeGet             )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropBufferConsumptionModeSet             )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropWrtdMasksGet                         )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropWrtdMasksSet                         )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTcProfileSharedPoolGet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTcProfileSharedPoolSet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxByteCountChangeEnableGet                     )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxByteCountChangeEnableSet                     )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxByteCountChangeValueGet                      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxByteCountChangeValueSet                      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortProfileTxByteCountChangeEnableSet              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortProfileTxByteCountChangeEnableGet              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQueueEnableGet                               )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQueueTxEnableGet                             )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQueueingEnableGet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperOnStackAsGigEnableGet                  )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperOnStackAsGigEnableSet                  )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxGlobalDescLimitSet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxGlobalDescLimitGet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxMcShaperMaskSet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxMcShaperMaskGet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQueueGlobalTxEnableSet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQueueGlobalTxEnableGet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropProfileBufferConsumptionModeSet      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropProfileBufferConsumptionModeGet      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSchedulerProfileBufferConsumptionModeSet     )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSchedulerProfileBufferConsumptionModeGet     )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperAvbModeEnableSet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxShaperAvbModeEnableGet                       )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQShaperAvbModeEnableSet                      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQShaperAvbModeEnableGet                      )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSchedulerProfileCountModeSet                 )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxSchedulerProfileCountModeGet                 )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropMaskSharedBuffEnableSet              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropMaskSharedBuffEnableGet              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropSharedBuffMaxLimitSet                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropSharedBuffMaxLimitGet                )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTx4TcMcBufNumberGet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxDbaAvailBuffSet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxDbaAvailBuffGet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxDbaEnableSet                                 )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxDbaEnableGet                                 )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBufferStatisticsEnableSet                    )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBufferStatisticsEnableGet                    )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxMaxBufferFillLevelSet                        )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxMaxBufferFillLevelGet                        )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQueueMaxBufferFillLevelSet                   )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQueueMaxBufferFillLevelGet                   )

        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTaiPtpPulseIterfaceSet                             )

        ,{NULL}/* must be last */
    };



    appDemoFalconForbidenTests(mainUt_forbidenTests_CRASH);
    appDemoFalconForbidenSuites(mainUt_forbidenSuites_CRASH);
    appDemoFalconForbidenTests(mainUt_forbidenTests_many_tests_fail);

    return GT_OK;
}

/* compare values of MG registers in the different MG units */
GT_STATUS   appDemoFalconMgRegCompare(void)
{
  static GT_U32   falconMgRegisters[] =
  {
       0x1D000034
      ,0x1D00003C
      ,0x1D00004C
      ,0x1D000050
      ,0x1D000058
      ,0x1D000060
      ,0x1D000084
      ,0x1D0000A8
      ,0x1D0000C0
      ,0x1D0000C4
      ,0x1D0000C8
      ,0x1D0000CC
      ,0x1D000154
      ,0x1D00015C
      ,0x1D000204
      ,0x1D00020C
      ,0x1D000210
      ,0x1D00023C
      ,0x1D000254
      ,0x1D0003F0
      ,0x1D0003F8
      ,0x1D0003FC
      ,0x1D00051C
      ,0x1D00260C
      ,0x1D00261C
      ,0x1D00262C
      ,0x1D00263C
      ,0x1D00264C
      ,0x1D00265C
      ,0x1D00266C
      ,0x1D00267C
      ,0x1D002680
      ,0x1D0026C0
      ,0x1D0026C4
      ,0x1D0026C8
      ,0x1D0026CC
      ,0x1D0026D0
      ,0x1D0026D4
      ,0x1D0026D8
      ,0x1D0026DC
      ,0x1D002700
      ,0x1D002704
      ,0x1D002708
      ,0x1D002710
      ,0x1D002714
      ,0x1D002718
      ,0x1D002720
      ,0x1D002724
      ,0x1D002728
      ,0x1D002730
      ,0x1D002734
      ,0x1D002738
      ,0x1D002740
      ,0x1D002744
      ,0x1D002748
      ,0x1D002750
      ,0x1D002754
      ,0x1D002758
      ,0x1D002760
      ,0x1D002764
      ,0x1D002768
      ,0x1D002770
      ,0x1D002774
      ,0x1D002778
      ,0x1D002800
      ,0x1D002814
      ,0x1D002818
      ,0x1D002860
      ,0x1D002864
      ,0x1D002870
      ,0x1D002874
      ,0x1D002878
      ,0x1D00287C
      ,0x1D002880
      ,0x1D002884
      ,0x1D002888
      ,0x1D00288C
      ,0x1D0028BC
      ,0x1D0028C8
      /* must be last*/
      ,GT_NA
    };
    GT_STATUS   rc;
    GT_U32  ii,jj;
    GT_U8 devNum = 0;
    GT_U32  offsetFromMg0BaseAddr[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    GT_U32  readValue[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    GT_U32  numMgs = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits;
    GT_U32  holdDiff;

    offsetFromMg0BaseAddr[0] = 0;
    cpssOsPrintf("RegAddr       MG[ 0]");

    for(jj = 1 ; jj < numMgs; jj++)
    {
        offsetFromMg0BaseAddr[jj] = prvCpssSip6OffsetFromFirstInstanceGet(devNum,jj,PRV_CPSS_DXCH_UNIT_MG_E);
        cpssOsPrintf("       MG[%1.2d]",jj);
    }
    cpssOsPrintf("\n");


    for(ii = 0 ; falconMgRegisters[ii] != GT_NA ; ii++)
    {
        holdDiff = 0;
        /*******************************************************/
        /* read the register from MG0 and compare to other MGs */
        /*******************************************************/
        for(jj = 0; jj < numMgs ; jj++)
        {
            rc = cpssDrvPpHwRegisterRead(devNum,0,falconMgRegisters[ii]+offsetFromMg0BaseAddr[jj],&readValue[jj]);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(jj > 0)
            {
                /* compare the values MGj to MG0 */
                if(readValue[jj] != readValue[0])
                {
                    holdDiff = 1;
                }
            }
        }

        if(holdDiff == 0)
        {
            cpssOsPrintf("[0x%8.8x] [0x%8.8x] same value all MGs \n",falconMgRegisters[ii],readValue[0]);
        }
        else
        {
            cpssOsPrintf("[0x%8.8x] ",falconMgRegisters[ii]);

            for(jj = 0 ; jj < numMgs ; jj++)
            {
                cpssOsPrintf("[0x%8.8x] ",readValue[jj]);
            }
            cpssOsPrintf("\n");
        }
    }

    return GT_OK;
}

