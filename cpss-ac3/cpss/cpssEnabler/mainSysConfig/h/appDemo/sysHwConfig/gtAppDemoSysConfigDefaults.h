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
* @file gtAppDemoSysConfigDefaults.h
*
* @brief System configuration and initialization structres default values.
*
* @version   46
********************************************************************************
*/
#ifndef __gtAppDemoSysConfigDefaultsh
#define __gtAppDemoSysConfigDefaultsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <appDemo/os/appOs.h>
#include <appDemo/sysHwConfig/gtAppDemoPciConfig.h>


/*******************************************************************************
 * Defualt values for the different configuration structs.
 ******************************************************************************/

/****************************************************************************/
/* GT_PP_PHASE1_INIT_PARAMS struct default                                  */
/****************************************************************************/
#define INT_LINE_DEFAULT GT_PCI_INT_D


#define CPSS_SRAM_DEFAULT                                       \
    {                                                           \
        CPSS_SRAM_SIZE_2MB_E,      /*sramSize;       */         \
        CPSS_SRAM_FREQ_250_MHZ_E,  /*sramFrequency;  */         \
        CPSS_SRAM_RATE_DDR_E,      /*sramRate;       */         \
        CPSS_EXTERNAL_MEMORY_0_E,  /*externalMemory; */         \
        CPSS_SRAM_TYPE_NARROW_E    /*sramType;       */         \
    }

#define CPSS_RLDRAM_DEFAULT                                     \
    {                                                           \
        CPSS_SRAM_SIZE_2MB_E,      /*dramSize      */           \
        GT_FALSE,                  /*overwriteDef  */           \
        11,                        /*mrs           */           \
        1536,                      /*refreshLimit  */           \
        {9,8,8,6},                 /*dramTiming    */           \
        80000,                     /*pwrUpDelay    */           \
        2048                       /*pwrUpRefDelay */           \
    }


#define CPSS_PP_PHASE1_DEFAULT                                  \
{                                                               \
    0,                              /* devNum           */      \
    {                                                           \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF                                    \
    },                              /* hwInfo[]         */      \
    1,                              /* numOfPortGroups  */      \
    0,                              /* deviceId         */      \
    0,                              /* revision         */      \
    166,                            /* coreClk          */      \
    /* flowDramCfg   , dram parameters of SAMSUNG E-Ver
    (K4D261638E-TC40) at 250MHz      */                         \
    {                                                           \
        0,                          /* flowDramSize     */      \
        CPSS_DRAM_FREQ_250_MHZ_E,   /* dramFrequency    */      \
        GT_TRUE,                    /* overwriteDef     */      \
        0x31,                       /* mrs              */      \
        2,                          /* emrs             */      \
        /* struct dramTiming */                                 \
        {                                                       \
            0,                      /* tdqss            */      \
            3,                      /* trcd             */      \
            1,                      /* trp              */      \
            2,                      /* twr              */      \
            3,                      /* twtr             */      \
            7,                      /* tras             */      \
            1,                      /* trrd             */      \
            0,                      /* tmrd             */      \
            12,                     /* trfc             */      \
            0,                      /* rd2rdDelay       */      \
            0,                      /* rd2wrDelay       */      \
            7,                      /* writeCount       */      \
            7                       /* readCount        */      \
        },                                                      \
        1850,                       /* refreshCounter   */      \
        2,                          /* trCdWr           */      \
        0                           /* operation        */      \
    },                                                          \
    /* bufferDramCfg, dram parameters of
    SAMSUNG E-Ver (K4D263238E-GC33) at 300MHz*/                 \
    {                                                           \
        0,                          /* flowDramSize     */      \
        CPSS_DRAM_FREQ_300_MHZ_E,   /* dramFrequency    */      \
        GT_TRUE,                    /* overwriteDef     */      \
        4,                          /* mrs              */      \
        2,                          /* emrs             */      \
        /* struct dramTiming */                                 \
        {                                                       \
            0,                      /* tdqss            */      \
            3,                      /* trcd             */      \
            1,                      /* trp              */      \
            2,                      /* twr              */      \
            3,                      /* twtr             */      \
            7,                      /* tras             */      \
            1,                      /* trrd             */      \
            0,                      /* tmrd             */      \
            5,                      /* trfc             */      \
            0,                      /* rd2rdDelay       */      \
            1,                      /* rd2wrDelay       */      \
            7,                      /* writeCount       */      \
            7                       /* readCount        */      \
        },                                                      \
        2240,                       /* refreshCounter   */      \
        0,                          /* trCdWr           */      \
        0                           /* operation        */      \
    },                                                          \
    /* bufferMgmtCfg        */                                  \
    {                                                           \
        GT_FALSE,                   /* overwriteDef     */      \
        0,                          /* numPpBuffers     */      \
        0                           /* numUplinkBuffers */      \
    },                                                          \
    CPSS_PP_UPLINK_BACK_TO_BACK_E,  /* uplinkCfg        */      \
    CPSS_BUFFER_DRAM_USE_DEFAULT_DRAM_CONF_E, /* bufferDram */  \
    CPSS_FLOW_DRAM_USE_DEFAULT_DRAM_CONF_E,   /* flowDram */    \
    CPSS_CHANNEL_PCI_E,             /* mngInterfaceType */      \
    CPSS_TWIST_INTERNAL_E,          /* routingSramCfgType */    \
    0,                              /* numOfSramsCfg */         \
    {                               /* sram struct */           \
        {0,0,0,0,0},                                            \
        {0,0,0,0,0},                                            \
        {0,0,0,0,0},                                            \
        {0,0,0,0,0},                                            \
        {0,0,0,0,0}                                             \
    },                                                          \
    CPSS_BUF_1536_E,                /* maxBufSize           */  \
    CPSS_SYS_HA_MODE_ACTIVE_E,      /* sysHAState           */  \
    APPDEMO_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,        \
                                    /*serdesRefClock*/          \
    GT_FALSE,                      /* isExternalCpuConnected */ \
    GT_FALSE,/*tcamParityCalcEnable*/                           \
    /* relevant only to 8-region address completion mechanism*/ \
    2,/*isrAddrCompletionRegionsBmp*/  /* region 1 */           \
    0xFC, /*appAddrCompletionRegionsBmp*/  /* regions 2-7 */    \
    GT_FALSE                        /*enableLegacyVplsModeSupport */  \
}


#define GT_PHOENIX_REG_VAL_INFO_LIST                                        \
{                                                                           \
    {0x00000000, 0x00000000, 0x00000000, 0},                                \
    {0x00000001, 0x00000000, 0x00000000, 0},                                \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},                                \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},                                \
    {0x0190010c, 0x07000000, 0x06000000, 1},                                \
    {0x01900180, 0xFFFFFFFF, 0x00400000, 1},                                \
    {0x01900184, 0xFFFFFFFF, 0x10d80180, 1},                                \
    {0x02e00280, 0xFFFFFFFF, 0x00420000, 1},                                \
    {0x1900190 , 0xFFFFFFFF, 0x0       , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x0       , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x1040    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x2041    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x3081    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x40C2    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x50C2    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x6102    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x7103    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x8143    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x9184    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0xA184    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0xB1C4    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0xC205    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0xD205    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0xE246    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0xF246    , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x11287   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x122C7   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x132C7   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x14308   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x15348   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x16349   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x17389   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x18389   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x193CA   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x1A40A   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x1B40B   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x1C44B   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x1D48B   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x1E48C   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x1F4CC   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x204CD   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x2150D   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x2254D   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x2354E   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x2458E   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x255CF   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x265CF   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x2760F   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x27610   , 1},                                \
    {0x1900194 , 0xFFFFFFFF, 0x27650   , 1},                                \
    {0x1900000 , 0x00300000, 0x00300000, 1},                                \
    {0x03800054, 0xFFFFFFFF, 0x2880CE2 , 1},                                \
    {0x03800040, 0xFFFFFFFF, 0x0000001 , 150},                              \
    {0x03800054, 0xFFFFFFFF, 0x2890CE2 , 1},                                \
    {0x03800040, 0xFFFFFFFF, 0x0000002 , 150},                              \
    {0x03800054, 0xFFFFFFFF, 0x28A0CE2 , 1},                                \
    {0x03800040, 0xFFFFFFFF, 0x0000004 , 150},                              \
    {0x03800054, 0xFFFFFFFF, 0x28B0CE2 , 1},                                \
    {0x03800040, 0xFFFFFFFF, 0x0000008 , 150},                              \
    {0x03800054, 0xFFFFFFFF, 0x89140   , 1},                                \
    {0x03800040, 0xFFFFFFFF, 0x0001000 , 150},                              \
    {0x03800054, 0xFFFFFFFF, 0x99140   , 1},                                \
    {0x03800040, 0xFFFFFFFF, 0x0002000 , 150},                              \
    {0x03800054, 0xFFFFFFFF, 0xA9140   , 1},                                \
    {0x03800040, 0xFFFFFFFF, 0x0004000 , 150},                              \
    {0x03800054, 0xFFFFFFFF, 0xB9140   , 1},                                \
    {0x03800040, 0xFFFFFFFF, 0x0008000 , 150},                              \
    {0x03800040, 0xFFFFFFFF, 0x0000000 , 1},                                \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},                                \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0}                                 \
}



/* Dummy for competability, for init process. */
#define GT_DUMMY_REG_VAL_INFO_LIST                                          \
{                                                                           \
    {0x00000000, 0x00000000, 0x00000000, 0},                                \
    {0x00000001, 0x00000000, 0x00000000, 0},                                \
    {0x00000002, 0x00000000, 0x00000000, 0},                                \
    {0x00000003, 0x00000000, 0x00000000, 0},                                \
    {0x00000004, 0x00000000, 0x00000000, 0},                                \
    {0x00000005, 0x00000000, 0x00000000, 0},                                \
    {0x00000006, 0x00000000, 0x00000000, 0},                                \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter        */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter        */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0}     /* Delimiter        */      \
}

/****************************************************************************/
/* CPSS_PP_PHASE2_INIT_PARAMS struct default                                  */
/****************************************************************************/
#define RX_DESC_NUM_DEF         200
#define TX_DESC_NUM_DEF         1000
#define AU_DESC_NUM_DEF         2048
#define RX_BUFF_SIZE_DEF        1548
#define RX_BUFF_ALLIGN_DEF      1
#define APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF  7
#define APP_DEMO_RX_BUFF_ALLIGN_DEF      (1 << APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
#define APP_DEMO_RX_BUFF_SIZE_DEF        800

#define CPSS_PP_PHASE2_DEFAULT                                  \
{                                                               \
    0,                              /* devNum           */      \
    0,                              /* deviceId         */      \
    /* struct netIfCfg  */                                      \
    {                                                           \
        NULL,                       /* *txDescBlock     */      \
        4096,                       /* txDescBlockSize  */      \
        NULL,                       /* *rxDescBlock     */      \
        4096,                       /* rxDescBlockSize  */      \
        /* rxBufInfo        */                                  \
        {                                                       \
            CPSS_RX_BUFF_STATIC_ALLOC_E, /* allocMethod */      \
            { 13, 13, 13, 13,           /* bufferPercentage */  \
              12, 12, 12, 12 },                                 \
            RX_BUFF_SIZE_DEF,           /* rxBufSize    */      \
            0,                          /* headerOffset */      \
            GT_FALSE,                   /* buffersInCachedMem */\
            /* union buffData   */                              \
            {                                                   \
                /* struct staticAlloc   */                      \
                {                                               \
                    NULL,           /* *rxBufBlock      */      \
                    16000           /* rxBufBlockSize   */      \
                }                                               \
            }                                                   \
        }                                                       \
    },                                                          \
    /* struct auqCfg        */                                  \
    {                                                           \
        NULL,                       /* *auDescBlock     */      \
        4096                        /* auDescBlockSize  */      \
    },                                                          \
    0,                              /* hwDevNum         */      \
    /* struct fuqCfg        */                                  \
    {                                                           \
        NULL,                       /* *fuDescBlock     */      \
        0                           /* fuDescBlockSize  */      \
    },                                                          \
    GT_FALSE,                       /* fuqUseSeparate  */       \
    RX_BUFF_SIZE_DEF,               /* rxBufSize       */       \
    0,                              /* headerOffset    */       \
    GT_FALSE,                       /* noTraffic2CPU  */       \
    0,                              /* netifSdmaPortGroupId */  \
    CPSS_AU_MESSAGE_LENGTH_4_WORDS_E,/* auMessageLength */       \
    GT_FALSE,                      /* useSecondaryAuq */       \
    GT_FALSE                        /* useDoubleAuq */          \
}

#define GT_PCL_ACTION_SIZE_16_BYTE 16

/* 7 trunks supported by : EXMXTC , EXMXSAM  */
#define APP_DEMO_7_TRUNKS_CNS      7

/* 31 trunks supported by : DXSAL , EXMXTD , EXMXTG */
#define APP_DEMO_31_TRUNKS_CNS      31

/* 127 trunks supported by : DXCH , DXCH2, DXCH3 */
#define APP_DEMO_127_TRUNKS_CNS     127

/* 127 trunks supported by : EXMXPM */
#define APP_DEMO_255_TRUNKS_CNS     255

#define PP_LOGICAL_CONFIG_FDB_DEFAULT                                         \
{                                                                             \
    0,/*fdbTableMode = INTERNAL_SPLIT */                                      \
    0,/*fdbToLutRatio -- don't care for internal memory */                    \
    0,/*fdbHashSize    -- don't care for internal memory */                   \
    0 /*extFdbHashMode -- don't care for internal memory */                   \
}

#define PP_LOGICAL_CONFIG_EXTERNAL_TCAM_DEFAULT                               \
{                                                                             \
    0,/* actionLinesInMemoryControl0 */                                       \
    0,/* actionLinesInMemoryControl1 */                                       \
    NULL,/*extTcamRuleWriteFuncPtr   */                                       \
    NULL,/*extTcamRuleReadFuncPtr    */                                       \
    NULL,/*extTcamActionWriteFuncPtr */                                       \
    NULL /*extTcamActionReadFuncPtr  */                                       \
}

#define APP_DEMO_CPSS_LPM_RAM_MEM_BLOCKS_DEFAULT                               \
{                                                                              \
    0,                                                                             /* numOfBlocks*/ \
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},                                     /* blocksSizeArray*/ \
    CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E /* blocksAllocationMethod */  \
}
    
/****************************************************************************/
/* GT_PP_CONFIG struct default                                              */
/****************************************************************************/
#define PP_LOGICAL_CONFIG_DEFAULT                                            \
{                                                                    \
    500,                            /* maxNumOfLifs              */  \
    APP_DEMO_CPSS_INLIF_PORT_MODE_PORT_E, /* inlifPortMode       */  \
    50,                             /* maxNumOfPolicerEntries    */  \
    GT_TRUE,                        /* policerConformCountEn     */  \
    1000,                           /* maxNumOfNhlfe             */  \
    100,                            /* maxNumOfMplsIfs           */  \
    APP_DEMO_CPSS_MEMORY_LOCATION_EXTERNAL_E,/*ipMplsMemLocation */  \
    1024,                           /* maxNumOfPclAction         */  \
    GT_PCL_ACTION_SIZE_16_BYTE,     /* pclActionSize             */  \
    0,                              /* maxNumOfPceForIpPrefixes  */  \
    GT_FALSE,                       /* usePolicyBasedRouting     */  \
    GT_FALSE,                       /* usePolicyBasedDefaultMc   */  \
    0,                              /* maxNumOfPbrEntries        */  \
    4095,                           /* maxVid                    */  \
    1024,                           /* maxMcGroups               */  \
    5,                              /* maxNumOfVirtualRouters    */  \
    100,                            /* maxNumOfIpNextHop         */  \
    1000,                           /* maxNumOfIpv4Prefixes      */  \
    100,                            /* maxNumOfIpv4McEntries     */  \
    500,                            /* maxNumOfMll               */  \
    100,                            /* maxNumOfIpv6Prefixes      */  \
    50,                             /* maxNumOfIpv6McGroups      */  \
    500,                            /* maxNumOfTunnelEntries     */  \
    8,                              /* maxNumOfIpv4TunnelTerms   */  \
    100,                            /* maxNumOfTunnelTerm        */  \
    0/*GT_VLAN_INLIF_ENTRY_TYPE_REGULAR_E*/, /* vlanInlifEntryType  */  \
    GT_FALSE,                       /* ipMemDivisionOn           */  \
    APP_DEMO_CPSS_DXCH_TCAM_ROUTER_BASED_E,  /* routingMode      */  \
    0,                              /* ipv6MemShare              */  \
    APP_DEMO_31_TRUNKS_CNS,         /* numOfTrunks               */  \
    0,                              /* defIpv6McPclRuleIndex     */  \
    0,                              /* vrIpv6McPclId             */  \
    GT_TRUE,                        /* lpmDbPartitionEnable      */  \
    100,                            /* lpmDbFirstTcamLine        */  \
    1003,                           /* lpmDbLastTcamLine         */  \
    GT_TRUE,                        /* lpmDbSupportIpv4          */  \
    GT_TRUE,                        /* lpmDbSupportIpv6          */  \
    0,                              /* lpmDbNumOfMemCfg          */  \
    NULL,                           /* lpmDbMemCfgArray          */  \
    CPSS_DRAM_16MB_E,               /* extLpmMemSize             */  \
    APP_DEMO_CPSS_LPM_STAGES_MEM_ASSOCIATION_ALL_INTERNAL_E, /*lpmStagesMemAssociation*/\
    APP_DEMO_CPSS_LPM_RAM_MEM_BLOCKS_DEFAULT,/*lpmRamMemoryBlocksCfg*/\
    APP_DEMO_CPSS_MEMORY_LOCATION_EXTERNAL_E,/* ditMemLocation   */  \
    APP_DEMO_CPSS_DIT_SHARED_MEM_E, /* ditMemoryMode             */  \
    APP_DEMO_CPSS_DIT_IP_MC_DUAL_MODE_E, /* ditIpMcMode          */  \
    50,                             /* maxNumOfDitUcIpEntries    */  \
    50,                             /* maxNumOfDitUcMplsEntries  */  \
    50,                             /* maxNumOfDitUcOtherEntries */  \
    50,                             /* maxNumOfDitMcIpEntries    */  \
    50,                             /* maxNumOfDitMcMplsEntries  */  \
    50,                             /* maxNumOfDitMcOtherEntries */  \
    0,                              /* miiNumOfTxDesc */             \
    0,                              /* miiTxInternalBufBlockSize */  \
    {13, 13, 13, 13, 12, 12, 12, 12},/*miiBufferPercentage   */      \
    0,                              /*miiRxBufSize    */             \
    0,                              /*miiHeaderOffset */             \
    0                               /*miiRxBufBlockSize  */          \
}

/****************************************************************************/
/* APP_DEMO_LIB_INIT_PARAMS struct default                                  */
/****************************************************************************/
#define LIB_INIT_PARAMS_DEFAULT                                 \
{                                                               \
    GT_TRUE,                        /* initBridge       */      \
    GT_TRUE,                        /* initClassifier   */      \
    GT_TRUE,                        /* initCos          */      \
    GT_TRUE,                        /* initIpv4         */      \
    GT_FALSE,                       /* initIpv6         */      \
    GT_TRUE,                        /* initIpv4Filter   */      \
    GT_TRUE,                        /* initIpv4Tunnel   */      \
    GT_TRUE,                        /* initLif          */      \
    GT_TRUE,                        /* initMirror       */      \
    GT_TRUE,                        /* initMpls         */      \
    GT_TRUE,                        /* initMplsTunnel   */      \
    GT_TRUE,                        /* initNetworkIf    */      \
    GT_FALSE,                       /* initI2c          */      \
    GT_TRUE,                        /* initPhy          */      \
    GT_TRUE,                        /* initPort         */      \
    GT_TRUE,                        /* initTc           */      \
    GT_TRUE,                        /* initTrunk        */      \
    GT_TRUE,                        /* initPcl          */      \
    GT_FALSE,                       /* initTcam         */      \
    GT_TRUE,                        /* initPolicer      */      \
    GT_TRUE,                        /* Init action      */      \
    GT_FALSE                  /* sFlow, only for D1 and D-XG*/  \
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtAppDemoSysConfigDefaultsh */



