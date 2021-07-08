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
* @file appDemoBoardConfig.c
*
* @brief Includes board specific initialization definitions and data-structures.
*
* @version   131
********************************************************************************
*/
#if defined(SHARED_MEMORY)
/* to allow vfork() and kill() */
#define _GNU_SOURCE
#endif
#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <gtExtDrv/drivers/gtCacheMng.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>
#include <gtExtDrv/drivers/gtTwsiHwCtrl.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <gtExtDrv/drivers/gtEthPortCtrl.h>
#include <gtExtDrv/drivers/gtHsuDrv.h>
#include <gtExtDrv/drivers/gtI2cDrv.h>
#include <gtExtDrv/drivers/gtHwIfDrv.h>
#include <gtExtDrv/drivers/gtDragoniteDrv.h>

#include <cpss/common/init/cpssInit.h>
#include <cpss/common/sw_version.h>
#include <cpss/extServices/cpssExtServices.h>

#include <gtOs/gtOsExc.h>
#include <gtOs/gtOsInet.h>
#include <gtStack/gtOsSocket.h>
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsDebug.h>

#ifdef  LINUX
#include <gtOs/gtOsStdLib.h>
#endif

#include <gtOs/gtOsMsgQ.h>
#include <gtUtil/gtBmPool.h>
#include <gtOs/gtOsStr.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsTimer.h>
#include <gtOs/gtOsStdLib.h>
#include <gtOs/gtOsMem.h>

#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbDxBobcat2PhyConfig.h>
#include <appDemo/confi/confi2.h>

#include <appDemo/userExit/userEventHandler.h>

#include <appDemo/boardConfig/gtDbDxAC3_Quagga_Utils.h>


#ifndef CHX_FAMILY
/*get some stubs*/
#include <appDemo/sysHwConfig/nonSwitchStubs.h>
#endif

#include <cpss/generic/version/cpssGenStream.h>

#ifdef CHX_FAMILY
/* For software CPSS version information */
#include <cpss/generic/version/cpssGenVersion.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpm.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNst.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <extUtils/cpssEnabler/appDemoFdbIpv6UcLinkedList.h>
#endif /*CHX_FAMILY*/
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <cmdShell/FS/cmdFS.h>


#if defined(SHARED_MEMORY)
/* Some ABI for BM with shared memory should use work-around for pointers to functions */
#include <gtOs/gtOsSharedFunctionPointers.h>
#include <gtOs/gtOsSharedUtil.h>

#include <sys/types.h>
#include <sys/signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#endif

#if (defined(LINUX) && !defined(ASIC_SIMULATION))
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <cpss/common/private/prvCpssGlobalMutex.h>

#ifdef IMPL_FA
#include <cpssFa/generic/version/gtFaVersion.h>
#endif  /* IMPL_FA */

#ifdef IMPL_XBAR
#include <cpssXbar/generic/version/gtXbarVersion.h>
#endif  /* IMPL_XBAR */

#ifdef CHX_FAMILY
        #include <extUtils/auEventHandler/auFdbUpdateLock.h>

        #include <cpss/dxCh/dxChxGen/version/cpssDxChVersion.h>
        #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
        #include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDescriptor.h>
        #include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
        #include <appDemo/utils/hsu/appDemoDxChHsuUtils.h>
        #include <appDemo/boardConfig/gtDbDxBobcat2UtilsSimClock.h>
        #include <appDemo/boardConfig/gtDbDxBobcat2LedIfConfig.h>
        #include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
        extern GT_STATUS appDemoConvert2DxChPhase1Info
        (
            IN  GT_U8                              dev,
            IN  CPSS_PP_PHASE1_INIT_PARAMS         *appCpssPpPhase1ParamsPtr,
            OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC  *cpssPpPhase1InfoPtr
        );

        extern GT_STATUS appDemoBc2PortInterfaceInit
        (
                IN  GT_U8 dev,
                IN  GT_U8 boardRevId
        );
        GT_STATUS bobcat2BoardPhyConfig
        (
                IN  GT_U8       boardRevId,
                IN  GT_U8       devNum
        );
        GT_STATUS cpssDxChPhyPortSmiInit
        (
                IN GT_U8 devNum
        );
        GT_STATUS cpssDxChBrgVlanPortDelete
        (
                IN GT_U8                        devNum,
                IN GT_U16                       vlanId,
                IN GT_PHYSICAL_PORT_NUM         portNum
        );

        GT_STATUS falcon_onEmulator_prepare(GT_VOID);

#endif  /* CHX_FAMILY */

#ifdef PX_FAMILY
        #include <cpss/px/cpssHwInit/cpssPxHwInit.h>
        #include <cpss/px/config/cpssPxCfgInit.h>
        #include <cpss/px/version/cpssPxVersion.h>
#endif  /* PX_FAMILY */

#if defined(SHARED_MEMORY) && defined(IMPL_GALTIS)
        extern APP_DEMO_SYS_CONFIG_STC appDemoSysConfig;
#ifdef CHX_FAMILY
        CPSS_DXCH_MP_SMI_TRANSMIT_PARAMS_STC ethPortParams;
#endif  /* CHX_FAMILY */
#endif /* defined(SHARED_MEMORY) && defined(IMPL_GALTIS) */


#include <cmdShell/common/cmdCommon.h>
#include <appDemo/userExit/userEventHandler.h>

#include <cpss/common/config/private/prvCpssConfigTypes.h>

#ifdef IMPL_GALTIS
        extern GT_U32 *enhUtUseCaptureToCpuPtr;
        #include <extUtils/trafficEngine/private/prvTgfInternalDefs.h>
#endif
#ifdef CHX_FAMILY
        #include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
        extern GT_U32 appDemoDxChTcamPclConvertedIndexGet_fromUT
        (
                IN     GT_U8                                devNum,
                IN     GT_U32                               index,
                IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
        );
        typedef GT_U32 (*APP_DEMO_DXCH_TCAM_PCL_CONVERTED_INDEX_GET_FUN)
        (
                IN     GT_U8                                devNum,
                IN     GT_U32                               index,
                IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
        );
        extern APP_DEMO_DXCH_TCAM_PCL_CONVERTED_INDEX_GET_FUN appDemoDxChTcamPclConvertedIndexGetPtr;


#endif /* CHX_FAMILY */


typedef GT_STATUS (*appDemoDbEntryGet_TYPE)
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);
extern appDemoDbEntryGet_TYPE appDemoDbEntryGet_func;

#if (!defined(FREEBSD) && !defined(UCLINUX) && !defined(LINUX) && !defined(_WIN32) && !defined(ASIC_SIMULATION))
        /* This is standard VxWorks function which returns the BSP version */
        extern char *sysBspRev(void);
#endif

#if (defined CHX_FAMILY)
        GT_STATUS gtAppDemoLionPortModeSpeedSet(GT_U8 devNum, GT_U8 portNum, CPSS_PORT_INTERFACE_MODE_ENT ifMode, CPSS_PORT_SPEED_ENT speed);

        GT_STATUS bc2AppDemoPortsConfig
        (
                IN  GT_U8                           devNumber,
                IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
                IN  CPSS_PORT_SPEED_ENT             speed,
                IN  GT_BOOL                         powerUp,
                IN  GT_U32                          numOfPorts,
                ...
        );

        GT_STATUS bobkAppDemoPortsConfig
        (
                IN  GT_U8                           devNumber,
                IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
                IN  CPSS_PORT_SPEED_ENT             speed,
                IN  GT_BOOL                         powerUp,
                IN  GT_U32                          squelch,
                IN  GT_U32                          numOfPorts,
                ...
        );

        GT_STATUS gtAppDemoXcat2StackPortsModeSpeedSet
        (
                IN  GT_U8     devNum,
                IN  GT_U8     portNum,
                IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode,
                IN  CPSS_PORT_SPEED_ENT  speed
        );
        GT_STATUS gtAppDemoXcat2SfpPortEnable
        (
                IN  GT_U8     devNum,
                IN  GT_U8     portNum,
                IN  GT_BOOL   enable
        );
        GT_STATUS util_appDemoDxChPortFWSRatesTableGet
        (
                IN  GT_U8 devNum,
                IN  GT_U32 arrSize,
                OUT CMD_PORT_FWS_RATE_STC *rateDbPtr
        );
#endif

#ifdef PX_FAMILY
        GT_STATUS util_appDemoPxPortFWSRatesTableGet
        (
                IN  GT_U8 devNum,
                IN  GT_U32 arrSize,
                OUT CMD_PORT_FWS_RATE_STC *rateDbPtr
        );
#endif

/**********************************************************************************
        if multiProcessAppDemo = 0 the appDemo run as regular (one process)
        if multiProcessAppDemo = 1 the appDemo run as multiprocess application
                the multiprocess application includes:
                - appDemo process
                - fdb learning process
                - RxTx process
        the variable multiProcessAppDemo used in file userEventHandler.c to define
        the event handler configuration
*********************************************************************************/
int multiProcessAppDemo = 0;

/*******************************************************************************
 * External variables
 ******************************************************************************/
GT_U32                          appDemoPpConfigDevAmount;
CPSS_PP_CONFIG_INIT_STC         ppUserLogicalConfigParams[PRV_CPSS_MAX_PP_DEVICES_CNS];
extern APP_DEMO_PP_CONFIG       appDemoPpConfigList[];
extern GT_BOOL                  systemInitialized;
extern GT_BOARD_LIST_ELEMENT    boardsList[];
extern GT_U32                   boardListLen;   /* Length of boardsList array.  */

/* is LSP 2.6.32 used flag */
GT_BOOL appDemo_LSP_is_linux_2_6_32 = GT_FALSE;

/* Enable printing inside interrupt routine. */
extern GT_STATUS extDrvUartInit
(
    GT_VOID
);

extern GT_STATUS px_forwardingTablesStage
(
    GT_SW_DEV_NUM      devNum
);

#ifdef  INCLUDE_UTF
/* Initialize Unit Test framework for CPSS */
extern GT_STATUS utfPreInitPhase
(
    GT_VOID
);

extern GT_STATUS utfInit
(
    GT_U8 firstDevNum
);
#endif

#ifdef  INCLUDE_UTF

extern GT_VOID utfPostInitPhase
(
    GT_STATUS st
);

GT_U32* utfTestTaskIdAddrGet
(
    GT_VOID
);

GT_STATUS prvUtfReduceLogSizeFlagSet
(
    GT_BOOL
);

GT_STATUS prvTgfBrgFdbFlush
(
    IN GT_BOOL                        includeStatic
);
extern GT_BOOL prvTgfFdbShadowUsed;
#endif  /* INCLUDE_UTF */

extern GT_STATUS wnc_xcat3LedInit();

/* when appDemoOnDistributedSimAsicSide == 1
   this image is running on simulation:
   1. our application on distributed "Asic side"
   2. the Other application that run on the "application side" is in
      charge on all Asic configurations
   3. our application responsible to initialize the cpss DB,
      to allow "Galtis wrappers" to be called on this side !
      (because the Other side application may have difficulty running
       "Galtis wrappers" from it's side)
*/
GT_U32  appDemoOnDistributedSimAsicSide = 0;

#ifdef ASIC_SIMULATION
    #include <gtStack/gtOsSocket.h>
    /* Simulation H files */
    #include <common/Version/sstream.h>
    #include <asicSimulation/SInit/sinit.h>
    #include <os/simTypesBind.h>
    #define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
    #include <os/simOsBindOwn.h>
    #undef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES

    #include <asicSimulation/SDistributed/sdistributed.h>
    #include <asicSimulation/SEmbedded/simFS.h>
    GT_U32 skernelStatusGet
    (
        void
    );

    #define BIND_FUNC(funcName) \
            simOsBindInfo.funcName = funcName

    #define BIND_LEVEL_FUNC(level,funcName)     \
            simOsBindInfo.level.funcName = funcName
    extern GT_U32 simulationSoftResetDone;
#endif /*ASIC_SIMULATION*/


#define BIND_APP_LEVEL_FUNC(infoPtr,level,funcName)     \
        infoPtr->level.funcName = funcName

#define BIND_APP2_LEVEL_FUNC(infoPtr,level,funcInLevel,funcName)     \
        infoPtr->level.funcInLevel = funcName

#define APP_DEMO_CV_INIT_MASK 0xC8

/* HW Access DB step size (this is the step to increment the DB each time,
  when its size is too small) */
#define APP_DEMO_CPSS_HW_ACCESS_DB_STEP_SIZE_CNS   _128K

/* HW Access DB maximal number of steps (Each step is made when db size is too small */

#define APP_DEMO_CPSS_HW_ACCESS_MAX_DB_STEP_COUNT   10

#define SCREEN_ROWS (255)
#define SCREEN_COLS (120)
#define BOARD_COLUMN_WIDTH (33)
#define REV_COLUMN_WIDTH (40)
#define TABLE_CROSS ('+')
#define TABLE_HOR ('-')
#define TABLE_VER ('|')

/*******************************************************************************
 * Local usage variables
 ******************************************************************************/
static GT_BOOL  appDemoPrePhase1Init = GT_FALSE;
static GT_BOOL  appDemoInitSystemOnlyPhase1and2 = GT_FALSE;
static GT_BOOL  appDemoInitSystemPhase1and2_done = GT_FALSE;
static GT_BOOL  gReloadEeprom;
static GT_BOOL  gIsB2bSystem;
static GT_BOOL  appDemoCpssInitialized = GT_FALSE;
static GT_BOOL  appDemoTrafficEnableDisableMode = GT_FALSE;
static GT_BOOL  appDemoInitRegDefaults = GT_FALSE;
static GT_BOARD_CONFIG_FUNCS   boardCfgFuncs;  /* Board specific configuration functions.  */
/*extern*/ GT_BOOL appDemoCpssInitSkipHwReset = GT_FALSE;/* indication that the appDemo initialize devices that skipped HW reset.
                    GT_TRUE  - the device is doing cpssInitSystem(...) after HW did NOT do soft/hard reset.
                    GT_FALSE - the device is doing cpssInitSystem(...) after HW did        soft/hard reset.
 */
/*extern*/ GT_U32    appDemoCpssCurrentDevIndex = 0;/* the appDemo device index in appDemoPpConfigList[] that currently initialized */

/* do we bypass the initialization of the events */
GT_BOOL appDemoBypassEventInitialize = GT_FALSE;
/* pointer to the function that allow override of setting done by appDemoSysConfigFuncsGet */
/* TBD - not supported yet */
APP_DEMO_SYS_CONFIG_FUNCS_EXTENTION_GET_FUNC appDemoSysConfigFuncsExtentionGetPtr = NULL;
#if defined  CPSS_USE_MUTEX_PROFILER
GT_BOOL   cpssInitSystemIsDone = GT_FALSE;
#endif

/* information about read and write hw access */
static APP_DEMO_CPSS_HW_ACCESS_INFO_STC  appDemoAccessInfo;

/* pointer to read and write  hw access db */
static APP_DEMO_CPSS_HW_ACCESS_DB_STC    *appDemoAccessDbPtr;
/* flag for tracing read hw access */
static GT_BOOL                            traceReadHwAccess = GT_FALSE;
/* flag for tracing write hw access */
static GT_BOOL                            traceWriteHwAccess = GT_FALSE;
/* flag for tracing write hw access */
static GT_BOOL                            traceDelayHwAccess = GT_FALSE;
/* trace output mode */
APP_DEMO_TRACE_OUTPUT_MODE_ENT appDemoTraceOutputMode = APP_DEMO_TRACE_OUTPUT_MODE_DIRECT_E;

/* first stored parameter of cpssInitSystem */
static GT_U32                             storedBoardIdx;
/* second stored parameter of cpssInitSystem */
static GT_U32                             storedBoardRevId;
/* third stored parameter of cpssInitSystem */
static GT_U32                             storedReloadEeprom;
/*stored value about the devices that pex/smi recognized on the 'board'*/
static GT_U8 storedNumOfPp,storedNumOfFa,storedNumOfXbar;

/**
* @enum APP_DEMO_CPSS_LOG_MODE_ENT
 *
 * @brief This enum defines the different Modes of the log.
*/
typedef enum{

    /** print the logs on the screen */
    APP_DEMO_CPSS_LOG_MODE_PRINT_E,

    /** write the log into a file on the memFS */
    APP_DEMO_CPSS_LOG_MODE_MEMFS_FILE_E,

    /** write the log into a file on a local FS */
    APP_DEMO_CPSS_LOG_MODE_LOCAL_FILE_E

} APP_DEMO_CPSS_LOG_MODE_ENT;


/* store the cpss Log print mode */
APP_DEMO_CPSS_LOG_MODE_ENT prvOsLogMode=APP_DEMO_CPSS_LOG_MODE_PRINT_E;
/** a cpss log thread mode */
APP_DEMO_CPSS_LOG_TASK_MODE_ENT prvOsLogThreadMode = APP_DEMO_CPSS_LOG_TASK_ALL_E;

/* an id of a task (thread) to be logged. Relevant only if prvOsLogThreadMode == APP_DEMO_CPSS_LOG_TASK_SINGLE_E */
static GT_U32 prvOsLogTaskId = 0;
static GT_U32 *prvOsLogTaskIdPtr = &prvOsLogTaskId;

/* a pointer to a memFS file to store the logs in */
static int prvOsLogMemFsFile = 0;
/* a pointer to a local file to store the logs in */
static FILE * prvOsLogLocalFile = NULL;
/* file pointers array per device per core that points */
/* to trace info for given device,core                 */
#ifdef ASIC_SIMULATION
static FILE *devCoreTraceFilePtr[PRV_CPSS_MAX_PP_DEVICES_CNS][APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS] = {{NULL}};
#define  APP_DEMO_FILE_MAX_LINE_LENGTH_CNS   128
#endif

/* macro to force casting between 2 functions prototypes */
#define FORCE_FUNC_CAST (void*)

#ifdef CHX_FAMILY
static GT_CHAR* FIELDS_ARRAY[] =
{
    "AppSpecificCPUCode",
    "ARPPtr",
    "BadIPxBC",
    "Bank",
    "BillingEn",
    "BufReuse",
    "BypassBridge_Dup",
    "BypassBridge",
    "BypassIngressPipe",
    "ByteCount",
    "CCFC",
    "CFI0",
    "CFI1",
    "CFI",
    "CFMPkt",
    "CongestedPortNumber",
    "CongestedQueueNumber",
    "ControlTC",
    "CopyTTL",
    "CPIDMatch",
    "CPU2CPUMirror",
    "CPUCode",
    "CPUMailToNeigborCPU",
    "CPUTrgDev",
    "DecTTL",
    "DIP bits 0..31",
    "DIP bits 32..63",
    "DIP bits 64..95",
    "DIP bits 96..127",
    "DIP",
    "DisableIPCL1_for_RoutedPackets",
    "DoRouteHa",
    "DoRoute",
    "DP",
    "DropCode",
    "DropOnSource",
    "DropPkt",
    "DSCP",
    "EgressFilterEn",
    "EgressFilterRegistered",
    "EgressTrgDev",
    "EgressTrgPort_dup_2_0",
    "EgressTrgPort",
    "EnableBridgeLogFlow",
    "EnableBridge",
    "EnableDsaTagManipulation",
    "EnableRouterLogFlow",
    "EnableSourceLocalFiltering",
    "Enable_IPCL0_lookup",
    "Enable_Policy",
    "EtherType",
    "ExcludedDevIsLocal",
    "ExcludedPort",
    "ExcludedTrunk",
    "ExcludeIsTrunk",
    "EXP",
    "FirstBuffer",
    "FloodMLLPointer",
    "FlowID",
    "FlowSubTemplate",
    "Force_IPCL1_configurations",
    "Fragmented",
    "FromCPUDP",
    "FromCPUTC",
    "HashMaskIndex",
    "IGMPNonQuery",
    "IGMPQuery",
    "InLIF_Mode",
    "InnerIsLLCNonSnap",
    "InnerL3Offset1_dup",
    "InnerL3Offset",
    "InnerPktType",
    "IPCL_Profile_Index",
    "IPLegal",
    "IPMgmntEn",
    "IPModifyDSCP",
    "IPModifyUP",
    "IPM",
    "IPQoSProfile",
    "Ipv4MulticastEn",
    "IPv4OptionField",
    "IPv4RIPv1",
    "Ipv4UnicastEn",
    "IPv4_Bad_GRE_Header",
    "IPv4_DF",
    "IPv6FlowLabel",
    "IPv6HBHExt",
    "IPv6HBHOptions",
    "Ipv6ICMPType",
    "IPv6ICMP",
    "Ipv6MulticastEn",
    "IPv6NonHBHExt",
    "IPv6SiteID",
    "Ipv6UnicastEn",
    "IPv6_EH",
    "IPxHeaderTotalLength",
    "IPxMACMcMismatch",
    "IPxMcLinkLocalProt",
    "IPxProtocol",
    "IP_Fragmented",
    "IP_Header_Info",
    "IsARPReply",
    "IsARP",
    "IsBC",
    "IsIPv4",
    "IsIPv4_ForKey",
    "IsIPv6LinkLocal",
    "IsIPv6MLD",
    "IsIPv6",
    "IsIPv6_ForKey",
    "IsIP",
    "IsIP_ForKey",
    "IsMPLS",
    "IsND",
    "IsPTP",
    "IsSYN",
    "L2Echo",
    "L2Encapsulation",
    "L2_Valid",
    "L3OffsetInvalid",
    "L3TotalOffset_or_InnerHeaderTotalOffset",
    "L4Byte13",
    "L4Bytes3_0",
    "L4OffsetInvalid",
    "L4Valid",
    "LLT",
    "LocalDeviceSrcTrunkID",
    "LocalDevSrcPort",
    "Lookup0_PCL_mode",
    "Lookup0_UseIndexFromDesc",
    "Lookup1_PCL_mode",
    "Lookup1_UseIndexFromDesc",
    "Lookup2_PCL_mode",
    "Lookup2_UseIndexFromDesc",
    "mac2me",
    "MacDaType",
    "MACDA bits 0..31",
    "MACDA bits 32..48",
    "MacSAArpSAMismatch",
    "MACSA bits 0..31",
    "MACSA bits 32..48",
    "MailBoxToNeighborCPU",
    "MarvellTaggedExtended",
    "MarvellTagged",
    "MCRPFFail",
    "MDB",
    "MeteringEn",
    "MLLEn",
    "MLLLast",
    "MLLPtr",
    "MLL",
    "MLL_Pointer",
    "ModifyDP",
    "ModifyDSCP",
    "ModifyDSCP_EXP",
    "ModifyEXP",
    "ModifyTC",
    "ModifyUP",
    "MPLSCmd",
    "MPLSLabel",
    "MPLS_Label0",
    "MPLS_Label1",
    "MPLS_Label2",
    "MPLS_OuterEXP",
    "MPLS_OuterLabel",
    "MPLS_OuterS_Bit",
    "NestedVlanAccessPort",
    "NHEMCRPFFailCmd",
    "Num_of_IngressTags",
    "OrigIsTrunk",
    "OrigQCNPrio",
    "OrigSrcDev",
    "OrigSrcPort",
    "OrigSrcTrunk",
    "OrigVID",
    "OuterIsLLCNonSnap",
    "OuterL3TotalOffset",
    "OuterSrcTag",
    "OutGoingMtagCmd",
    "OutLIF_Type",
    "PacketCmd",
    "PacketIsCNTagged",
    "PacketIsLLCSnap",
    "PacketReject",
    "PacketTOS",
    "PacketType",
    "PacketUP",
    "PCL1_IPv4_Key_Type",
    "PCL1_IPv6_Key_Type",
    "PCL1_Lookup_mode",
    "PCL1_Lookup_Type",
    "PCL1_NonIP_Key_Type",
    "PCL1_PCL_ID",
    "PCLAssignedSSTID",
    "PktCmd",
    "PktHash",
    "PktIsLooped",
    "PktTypeForKey",
    "PolicerEn",
    "PolicerPtr",
    "PolicyBasedRouted",
    "PolicyRTTIndex",
    "PortSpeedIndex_or_OrigQCNPrio",
    "PrioTagged",
    "PseudoWireID",
    "PTPTriggerType",
    "QCNPacket",
    "QCNqDelta",
    "QCNqFb",
    "QCNqOffset",
    "QCNqSaturated",
    "QCN",
    "QinQ",
    "QosProfilePrecedence",
    "QosProfile",
    "RecalcCRC",
    "RedirectCmd",
    "Reserved bits 0..31",
    "Reserved bits 32..63",
    "Reserved bits 64..65",
    "Reserved bits 64..90",
    "Reserved",
    "RGARPIndex",
    "RGBypassTTLandOptionChecks",
    "RGDecTTL",
    "RGEgressInterface",
    "RGFwdCmd",
    "RGICMP_RedirectCheckEn",
    "RGMirrorToIngressAnalyzerPort",
    "RGModifyDSCP",
    "RGModifyUP",
    "RGNHEVID",
    "RGPCERouteEntry",
    "RGQoSProfile",
    "RGQoS_Precedence",
    "RGQoS_ProfileMarkingEn",
    "Routed",
    "RouteToEntry",
    "RsrvdCopy",
    "RxSniff",
    "SIP bits 0..31",
    "SIP bits 32..63",
    "SIP bits 64..95",
    "SIP bits 96..127",
    "SniffTrgDev",
    "SniffTrgPort",
    "SniffVIDX",
    "SolicitationMulticastMessage",
    "SrcDevIsOwn",
    "SrcDev",
    "SrcIsLoopback",
    "SrcPort",
    "SrcTagged_dup",
    "SrcTagged",
    "SrcTCPUDPPortIs0",
    "SrcTrg",
    "Src_is_PE",
    "SST_ID",
    "SYNWithData",
    "TargetIsTrunk",
    "TCPFlags",
    "TCP_UDPDestPort",
    "TCP_UDP_SrcPort",
    "TC",
    "TrgDevice",
    "TrgDevice_VirtualSrcDev",
    "TrgDev",
    "TrgIsTrunk",
    "TrgPort",
    "TrgPort_VirtualSrcPort",
    "TrgTagged_dup",
    "TrgTagged",
    "TrgTrunkID",
    "Truncated",
    "TS_Passenger_Type",
    "TTL",
    "TunnelPtr",
    "TunnelStart",
    "TunnelTerminated",
    "TunnelType",
    "TxSampled",
    "UDB0",
    "UDB10",
    "UDB11",
    "UDB12",
    "UDB13",
    "UDB14",
    "UDB15",
    "UDB16",
    "UDB17",
    "UDB18",
    "UDB19",
    "UDB1",
    "UDB20",
    "UDB21",
    "UDB22",
    "UDB2",
    "UDB3",
    "UDB4",
    "UDB5",
    "UDB6",
    "UDB7",
    "UDB8",
    "UDB9",
    "UDB_BytesValid",
    "UP1",
    "UP",
    "UseInterfaceBasedIndex",
    "UseVIDX",
    "VID1",
    "VIDModified",
    "VIDX",
    "VID",
    "VID_or_InLIF_ID",
    "VID_Precedence",
    "VirtualInterface",
    "VLAN1_Exist",
    "VRF_ID",
    "VR_ID",
    "SrcOwnDev0",
    "CongestedQueueFillLevel"
};
#endif

/* Do common initialization for all families */
static GT_STATUS appDemoInitGlobalModuls
(
    IN  GT_U32  reloadEeprom
);

/* Cpss initialization */
GT_STATUS appDemoCpssInit
(
    GT_VOID
);

/* if not used shared memory or it's unix-like simulation do compile this */
#if !defined(SHARED_MEMORY)
static GT_STATUS cpssGetDefaultExtDrvFuncs(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC  *extDrvFuncBindInfoPtr
);


static GT_STATUS cpssGetDefaultOsBindFuncs(
    OUT CPSS_OS_FUNC_BIND_STC *osFuncBindPtr
);
#endif /* !defined(SHARED_MEMORY) */


#if (!defined(SHARED_MEMORY) || defined(LINUX_SIM))
static GT_STATUS cpssGetDefaultTraceFuncs(
    OUT CPSS_TRACE_FUNC_BIND_STC  *traceFuncBindInfoPtr
);
#endif

/* HW phase 1 initialization */
static GT_STATUS appDemoBoardPhase1Init
(
    IN  GT_U8                   boardRevId,
    IN  GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs,
    IN  GT_U8                   numOfDev,
    IN  GT_U8                   numOfFa,
    IN  GT_U8                   numOfXbar
);

/* HW phase 2 initialization */
static GT_STATUS appDemoBoardPhase2Init
(
    IN  GT_U8                   boardRevId,
    IN  GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs,
    IN  GT_U8                   numOfDev,
    IN  GT_U8                   numOfFa,
    IN  GT_U8                   numOfXbar
);

/* Logical phase initialization */
static GT_STATUS appDemoBoardLogicalInit
(
    IN  GT_U8                   boardRevId,
    IN  GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs,
    IN  GT_U8                   numOfDev,
    IN  GT_U8                   numOfFa,
    IN  GT_U8                   numOfXbar
);

/* General phase initialization */
static GT_STATUS appDemoBoardGeneralInit
(
    IN  GT_U8                   boardRevId,
    IN  GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs,
    IN  GT_U8                   numOfDev,
    IN  GT_U8                   numOfFa,
    IN  GT_U8                   numOfXbar
);

/* Updates PP phase 1 params according to app demo database. */
static GT_STATUS appDemoUpdatePpPhase1Params
(
    INOUT CPSS_PP_PHASE1_INIT_PARAMS    *paramsPtr
);

/* Updates PP phase 2 params according to app demo database. */
static GT_STATUS appDemoUpdatePpPhase2Params
(
    INOUT CPSS_PP_PHASE2_INIT_PARAMS    *paramsPtr
);

/* Updates PP logical init params according to app demo database. */
static GT_STATUS appDemoUpdatePpLogicalInitParams
(
    INOUT CPSS_PP_CONFIG_INIT_STC       *paramsPtr
);

/* Updates lib init params according to app demo database. */
static GT_STATUS appDemoUpdateLibInitParams
(
    INOUT APP_DEMO_LIB_INIT_PARAMS      *paramsPtr
);

void appDemoRtosOnSimulationInit
(
    void
);


static GT_STATUS appDemoSetHwAccessDbActiveState
(
    IN      GT_BOOL                             active,
    INOUT   APP_DEMO_CPSS_HW_ACCESS_DB_STC      **dbPtrPtr,
    INOUT   APP_DEMO_CPSS_HW_ACCESS_INFO_STC    *dbInfoPtr
);

static GT_STATUS appDemoTraceHwAccessAction
(
    IN GT_U8                                devNum,
    IN GT_U32                               portGroupId,
    IN GT_BOOL                              isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT         addrSpace,
    IN GT_U32                               addr,
    IN GT_U32                               length,
    IN GT_U32                               *dataPtr,
    IN GT_U32                               mask,
    IN APP_DEMO_TRACE_OUTPUT_MODE_ENT       outputMode,
    IN APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT         accessType
);

static GT_STATUS appDemoTraceStoreHwData
(
    IN GT_U8                                devNum,
    IN GT_U32                               portGroupId,
    IN GT_BOOL                              isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT         addrSpace,
    IN GT_U32                               addr,
    IN GT_U32                               length,
    IN GT_U32                               *dataPtr,
    IN GT_U32                               mask,
    IN APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT    accessType,
    INOUT APP_DEMO_CPSS_HW_ACCESS_DB_STC    **dbPtrPtr,
    INOUT APP_DEMO_CPSS_HW_ACCESS_INFO_STC  *infoDbPtr
);

#if (!defined(SHARED_MEMORY) || defined(LINUX_SIM))
static GT_STATUS appDemoTraceWriteHwAccess
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT         addrSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr,
    IN GT_U32      mask
);

static GT_STATUS appDemoTraceReadHwAccess
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT         addrSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr
);

static GT_STATUS appDemoTraceDelayHwAccess
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_U32      millisec
);
#endif
static GT_STATUS prvAppDemoTraceHwAccessEnable
(
    IN GT_U8                devNum,
    IN GT_U32               accessType,
    IN GT_BOOL              enable
);

static GT_STATUS prvAppDemoTraceHwAccessOutputModeSet
(
    IN GT_U32   mode
);

/* function to detect memory leakage between calls to it */
static GT_STATUS memoryLeakageDbg(void);
static GT_U32 initCounter = 0;
static GT_U32 resetCounter = 0;

#ifdef SHARED_MEMORY
static pid_t fdbLearningProcessId = 0;
static pid_t RxTxProcessId = 0;
#endif

static char scrBuf[SCREEN_ROWS * SCREEN_COLS];
static GT_U32 scrBufCursor;
GT_STATUS appDemoDeviceLogicalInit
(
    IN GT_U8    devIdx,
    IN GT_U8    boardRevId
);

GT_STATUS appDemoDeviceGeneralInit
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);


/*******************************************************************************
* FUNCP_TRACE_PRINTF_DATA
*
* DESCRIPTION:
*       Write a formatted string to the standard output stream.
*
* INPUTS:
*       format  - format string to write
*
* OUTPUTS:
*       None
*
* RETURNS:
*       The number of characters written, or a negative value if
*       an output error occurs.
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef int (*FUNCP_TRACE_PRINTF_DATA)(const char* format, ...);

static GT_VOID appDemoTracePrintHwData
(
    IN GT_U32                   addr,
    IN GT_U32                   length,
    IN GT_U32                   *dataPtr,
    IN GT_U32                   mask,
    IN FUNCP_TRACE_PRINTF_DATA  printFuncPtr
);

/* DB to store device type id */
static GT_U32   deviceIdDebug[PRV_CPSS_MAX_PP_DEVICES_CNS];

/* Flag for using device id from deviceIdDebug array */
static GT_BOOL  useDebugDeviceId = GT_FALSE;

/* flag to state that the trace will print also functions that return GT_OK */
/* usually we not care about 'GT_OK' only about fails */
/* the flag can be changed before running the cpssInitSystem(...) --> from terminal vxWorks */
GT_U32 appDemoTraceOn_GT_OK = 0;

/**
* @internal appDemoTraceOn_GT_OK_Set function
* @endinternal
*
* @brief   This function configures 'print returning GT_OK' functions flag
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note - in unix-like systems special function needed to configure appDemoTraceOn_GT_OK
*       specific ifdef's needless, because this function can't harm anything
*       - must be external to ensure it's not ommited by optimization.
*
*/
GT_VOID appDemoTraceOn_GT_OK_Set
(
    GT_U32 enable
)
{
    appDemoTraceOn_GT_OK = enable;
    return;
}

/*************************************************************************/
/* Switched tracing mechanism */

/*
 * Typedef: struct __TRACE_ENABLE_SEGMENT_STC
 *
 * Description: the structure for trace statement enabling rule
 *
 * Fields:
 *      nextPtr        - pointer to next rule (NULL is last).
 *      startLineNum   - start line number of line range
 *      endLineNum     - end line number of line range
 *      substingInPath - string to be checked to be substring of the file path
 *
 */
typedef struct __TRACE_ENABLE_SEGMENT_STC
{
    struct __TRACE_ENABLE_SEGMENT_STC *nextPtr;
    GT_U32 startLineNum;
    GT_U32 endLineNum;
    GT_U8  substingInPath[1]; /* the real length ahould be more */
} TRACE_ENABLE_SEGMENT_STC;

/* pointer to the first rule in the chain */
static TRACE_ENABLE_SEGMENT_STC *appDemoTraceEnableFilterPtr = NULL;

/**
* @internal appDemoTraceEnableFilterAdd function
* @endinternal
*
* @brief   This function adds trace filtering rule.
*         The rule is condition to enable the tracing statement
*         according to path of containing file and line number.
* @param[in] startLineNum             - the low bound of line range.
* @param[in] endLineNum               - the high bound of line range.
* @param[in] substingInPath           - the string to be matched in source file path.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoTraceEnableFilterAdd
(
    IN GT_U32 startLineNum,
    IN GT_U32 endLineNum,
    IN GT_U8  *substingInPath
)
{
    TRACE_ENABLE_SEGMENT_STC* node =
        (TRACE_ENABLE_SEGMENT_STC*)cpssOsMalloc(
            sizeof(TRACE_ENABLE_SEGMENT_STC) + cpssOsStrlen(substingInPath) + 1);
    if (node == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }
    node->nextPtr = appDemoTraceEnableFilterPtr;
    node->startLineNum = startLineNum;
    node->endLineNum = endLineNum;
    cpssOsStrCpy((GT_CHAR*)&(node->substingInPath[0]), (const GT_CHAR*)substingInPath);
    appDemoTraceEnableFilterPtr = node;
    return GT_OK;
}

/**
* @internal appDemoTraceEnableFilterReset function
* @endinternal
*
* @brief   This function resets all trace filtering rules
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_VOID appDemoTraceEnableFilterReset()
{
    TRACE_ENABLE_SEGMENT_STC* node = appDemoTraceEnableFilterPtr;
    TRACE_ENABLE_SEGMENT_STC* nextNode;
    while (node != NULL)
    {
        nextNode = node->nextPtr;
        cpssOsFree(node);
        node = nextNode;
    }
    appDemoTraceEnableFilterPtr = NULL;
}

/**
* @internal appDemoTraceEnableFilterCheck function
* @endinternal
*
* @brief   This function ckecks is trace printing enable for this trace statement
*
* @param[in] filePath                 - path to source file returned by __FILE__ macro.
* @param[in] lineNum                  - number of line returned by __LINE__ macro.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_BOOL appDemoTraceEnableFilterCheck
(
    IN GT_U8  *filePath,
    IN GT_U32 lineNum
)
{
    GT_U32 filePathLen = cpssOsStrlen(filePath);
    GT_U32 substrLen;
    GT_32  comparePos;
    TRACE_ENABLE_SEGMENT_STC* node = appDemoTraceEnableFilterPtr;
    for (; (node != NULL); node = node->nextPtr)
    {
        if ((lineNum < node->startLineNum) || (lineNum > node->endLineNum))
        {
            continue;
        }
        substrLen = cpssOsStrlen(node->substingInPath);
        if (substrLen == 0)
        {
            return GT_TRUE;
        }
        if (substrLen > filePathLen)
        {
            continue;
        }
        comparePos = (GT_32)(filePathLen - substrLen);
        for (; ((comparePos + 1) > 0); comparePos--)
        {
            if (filePath[comparePos] != node->substingInPath[0])
            {
                continue;
            }
            if (0 == cpssOsMemCmp(&(filePath[comparePos]), node->substingInPath, substrLen))
            {
                return GT_TRUE;
            }
        }
    }
    return GT_FALSE;
}

/*******************************************************************************
 * Public API implementation
 ******************************************************************************/

/**
* @internal appDemoShowCpssSwVersion function
* @endinternal
*
* @brief   This function returns CPSS SW version. Also, it calls
*         appDemoCpssInit() for CPSS initialization.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoShowCpssSwVersion(GT_VOID)
{
    GT_STATUS               rc = GT_OK;

#ifdef CHX_FAMILY
    CPSS_VERSION_INFO_STC   versionInfo;    /* Holds CPSS versions SW */
#endif /*CHX_FAMILY*/

#ifdef PX_FAMILY
    GT_VERSION              versionInfoPx;    /* Holds CPSS version info */
#endif

    #if (!defined(FREEBSD) && !defined(UCLINUX) && !defined(LINUX) && !defined(_WIN32) && !defined(ASIC_SIMULATION))
    /* This is standard VxWorks function which returns the BSP version */
    cpssOsPrintf("\nBSP  Version: %s", sysBspRev());
    #endif

#ifdef CHX_FAMILY
    rc = cpssDxChVersionGet(&versionInfo);
    if (rc == GT_OK)
    {
        cpssOsPrintf("\nCpss DxCh version: %s", versionInfo.version);
    }
#endif   /* CHX_FAMILY */

#ifdef PX_FAMILY
    rc = cpssPxVersionGet(&versionInfoPx);
    if (rc == GT_OK)
    {
        cpssOsPrintf("\nCpss Px version: %s", versionInfoPx.version);
    }
#endif   /* PX_FAMILY */

    cpssOsPrintf("\nCPSS Version Stream: %s\n",CPSS_STREAM_NAME_CNS);

	cpssOsPrintf("\nSW Version: %s\n\n",SW_VER);
    #ifdef ASIC_SIMULATION
        cpssOsPrintf("\nCHIP SIMULATION Version Stream: %s\n",CHIP_SIMULATION_STREAM_NAME_CNS);
        if (simFSiniFileDirectory[0] != 0)
        {
            cpssOsPrintf("The embedded FS INI file directory:\n%s\n", simFSiniFileDirectory);
        }
    #endif

#ifdef IMPL_FA
    rc = gtFaVersion(&versionInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
#endif  /* IMPL_FA */

#ifdef IMPL_XBAR
{
    GT_VERSION xbarVersion;
    rc = gtXbarVersion(&xbarVersion);
    if (rc != GT_OK)
    {
        return rc;
    }
}
#endif  /* IMPL_XBAR */


#if (defined(LINUX) && !defined(ASIC_SIMULATION))
    FILE *lspfile;
    char  lspbuf[4096] = {0};
    char  lspSubNameToCmp[] = "  LSP version: linux-2.6.32";
    GT_U32 lenToCmp;

    /* read lsp revision from /dev/shm/LSP_VERS.TXT file created from init.sh *
     * by command:  dmesg -s 32768 |grep -i LSP > /dev/shm/LSP_VERS.TXT       */
    if( (lspfile = fopen("/dev/shm/LSP_VERS.TXT", "r")) != NULL)
    {
        /* read revision line from file */
        if(fgets(lspbuf, sizeof(lspbuf), lspfile) != NULL)
        {
            cpssOsPrintf("LSP version:%s", lspbuf);

            lenToCmp = cpssOsStrlen(lspSubNameToCmp);
            if (0 == cpssOsStrNCmp(lspSubNameToCmp, lspbuf, lenToCmp))
            {
                appDemo_LSP_is_linux_2_6_32 = GT_TRUE;
            }
        }
        fclose(lspfile);
    }
#endif

    return rc;
}

/**
* @internal scrBufReset function
* @endinternal
*
* @brief   This function clears screen buffer and sets line pointer to 0
*/
GT_VOID scrBufReset
(
    GT_VOID
)
{
    osMemSet(scrBuf, '\0', SCREEN_ROWS * SCREEN_COLS);
    scrBufCursor = 0;
}

/**
* @internal scrBufShow function
* @endinternal
*
* @brief   Prints out screen buffer
*/
static GT_VOID scrBufShow
(
    GT_VOID
)
{
    GT_U32 ii;
    GT_BOOL eol;
    char *bufPtr = scrBuf;
    char *endPtr;

    for( ii = 0; ii < SCREEN_ROWS; ii++)
    {
        eol = GT_FALSE;
        for(endPtr = bufPtr + SCREEN_COLS - 1; endPtr >= bufPtr; endPtr--)
        {
            if(!eol && (*endPtr != '\0'))
            {
                eol = GT_TRUE;
                continue;
            }
            if(eol && (*endPtr ==  '\0'))
            {
                *endPtr = ' ';
            }
        }
        if(bufPtr[0] != '\0')
        {
            if((bufPtr[0] == ' ') && (bufPtr[1] == '\0'))
            {
                osPrintf("\n");
            }
            else
            {
                osPrintf("%s\n", bufPtr);
            }
        }
        bufPtr += SCREEN_COLS;
    }
}

/**
* @internal scrBufPrintf function
* @endinternal
*
* @brief   Prints formatted string to buffer
*
* @param[in] format                   - printf() style  string
*                                      ...    - args
*                                       Characters written count
*/
static GT_32 scrBufPrintf
(
    const char* format,
    ...
)
{
    char pbuf[256];
    GT_U32 ii;
    GT_U32 len;
    va_list args;
    GT_U32 cur;
    GT_32 written;

    osMemSet(pbuf, '\0', 256);
    va_start(args, format);
    ii = osVsprintf(pbuf, format, args);
    va_end(args);

    len = osStrlen(pbuf);
    for(ii = 0; ii < len; ii++)
    {
        if(pbuf[ii] == '\n')
        {
            pbuf[ii] = '\0';
        }
    }
    cur = 0;
    while(cur < len)
    {
        if(pbuf[cur] == '\0')
        {
            if((scrBufCursor % SCREEN_COLS) == 0)
            {
                scrBuf[scrBufCursor] = ' ';
            }
            scrBufCursor = (scrBufCursor / SCREEN_COLS) * SCREEN_COLS +
                SCREEN_COLS;
            cur++;
        }
        written = osSprintf(&scrBuf[scrBufCursor], "%s", &pbuf[cur]);
        cur += written;
        scrBufCursor += written;
    }
    return len;
}

/**
* @internal scrBufCoordPrintf function
* @endinternal
*
* @brief   Prints line to buffer and fits it too width by breaking it into multiple
*         lines on selected coordinates
* @param[in] row                      -  of buffer to start print from
* @param[in] col                      - column of buffer to start print from
* @param[in] width                    -  of row
* @param[in] format                   - printf() style  string
*                                      ...    - args
*                                       Printed lines count
*/
static GT_32 scrBufCoordPrintf
(
    GT_U32 row,
    GT_U32 col,
    GT_U32 width,
    const char* format,
    ...
)
{
    char pbuf[256];
    GT_U32 ii;
    GT_U32 len;
    va_list args;
    GT_U32 cur;
    GT_32 written;
    GT_U32 lastSpaceIdx = 0;
    GT_U32 sublineLen;
    GT_U32 localCursor;
    GT_U32 printedLines = 0;

    osMemSet(pbuf, '\0', 256);
    va_start(args, format);
    ii = osVsprintf(pbuf, format, args);
    va_end(args);

    len = osStrlen(pbuf);
    sublineLen = 0;
    for(ii = 0; ii < len; ii++)
    {
        if(pbuf[ii] == ' ')
        {
            lastSpaceIdx = ii;
        }
        sublineLen++;
        if((sublineLen > width) && (lastSpaceIdx != 0))
        {
            pbuf[lastSpaceIdx] = '\0';
            sublineLen = ii - lastSpaceIdx;
        }
    }
    cur = 0;
    localCursor = row * SCREEN_COLS + col;
    while(cur < len)
    {
        if(pbuf[cur] == '\0')
        {
            localCursor += SCREEN_COLS;
            cur++;
        }
        printedLines++;
        written = osSprintf(&scrBuf[localCursor], "%s", &pbuf[cur]);
        cur += written;
        scrBufCursor += written;
    }
    return printedLines;
}

/**
* @internal scrBufCoordPutc function
* @endinternal
*
* @brief   Prints character to buffer on designated coordinates
*
* @param[in] row                      -  of buffer to start print from
* @param[in] col                      - column of buffer to start print from
* @param[in] character                -  to print
*                                       None.
*/
static GT_VOID scrBufCoordPutc
(
    GT_U32 row,
    GT_U32 col,
    char   character
)
{
    scrBuf[row * SCREEN_COLS + col] = character;
}

/**
* @internal scrBufPrintHorLine function
* @endinternal
*
* @brief   Prints horizontal separating line of boards table
*
* @param[in] row                      -  of buffer to start print from
*                                       None.
*/
static GT_VOID scrBufPrintHorLine
(
    GT_U32 row
)
{
    osMemSet(&scrBuf[row * SCREEN_COLS], TABLE_HOR, BOARD_COLUMN_WIDTH +
        REV_COLUMN_WIDTH + 4);
    scrBufCoordPutc(row, 0, TABLE_CROSS);
    scrBufCoordPutc(row, BOARD_COLUMN_WIDTH + 1, TABLE_CROSS);
    scrBufCoordPutc(row, BOARD_COLUMN_WIDTH + REV_COLUMN_WIDTH + 3,
        TABLE_CROSS);
}

/**
* @internal appDemoShowSingleBoard function
* @endinternal
*
* @brief   This function displays single board which have an automatic
*         initialization support.
* @param[in] pBoardInfo               - (pointer to) the board info
* @param[in] id                       - the first parameter of gtInitSystem function
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note The printed table should look like:
*       | 02 - DB-MX610-48F4GS     |            |
*       |                | 01 - Rev 1.0     |
*       |                | 02 - Rev 2.5     |
*       +-------------------------------+-----------------------+
*
*/
GT_STATUS appDemoShowSingleBoard(
    IN GT_BOARD_LIST_ELEMENT   *pBoardInfo,
    IN GT_U32   id
)
{
    GT_U32  j; /* Loops index.                 */
    GT_U32 revLinesPrinted = 0;
    GT_U32 boardLinesPrinted = 0;
    GT_U32 currentRow = scrBufCursor / SCREEN_COLS;
    GT_U32 linesPrinted;
    GT_U32 lineIter;

    boardLinesPrinted = scrBufCoordPrintf(currentRow, 2, BOARD_COLUMN_WIDTH,
        "%02d - %s", id, pBoardInfo->boardName);
    revLinesPrinted = 1;

    /* Iterate through revisions */
    for(j = 0; j < pBoardInfo->numOfRevs; j++)
    {
        if(0 == osStrCmp(GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS,
            pBoardInfo->boardRevs[j]))
        {
            /* skip the not used revisions */
            continue;
        }

        /* Print revision*/
        scrBufCoordPrintf(currentRow + revLinesPrinted, BOARD_COLUMN_WIDTH + 3,
            REV_COLUMN_WIDTH, "%02d -", j + 1);
        revLinesPrinted += scrBufCoordPrintf(currentRow + revLinesPrinted,
            BOARD_COLUMN_WIDTH + 8, REV_COLUMN_WIDTH - 5, "%s",
            pBoardInfo->boardRevs[j]);
    }

    linesPrinted = (boardLinesPrinted > revLinesPrinted) ? boardLinesPrinted :
        revLinesPrinted;

    /* Draw table borders */
    currentRow += linesPrinted;
    for(lineIter = currentRow - linesPrinted; lineIter < currentRow;
        lineIter++)
    {
        scrBufCoordPutc(lineIter, 0, TABLE_VER);
        scrBufCoordPutc(lineIter, BOARD_COLUMN_WIDTH + 1, TABLE_VER);
        scrBufCoordPutc(lineIter, BOARD_COLUMN_WIDTH + REV_COLUMN_WIDTH + 3,
            TABLE_VER);
    }
    scrBufPrintHorLine(currentRow);
    currentRow++;
    scrBufCursor = currentRow * SCREEN_COLS;

    return GT_OK;
}

/**
* @internal appDemoShowBoardsList function
* @endinternal
*
* @brief   This function displays the boards list which have an automatic
*         initialization support.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note The printed table should look like:
*       +-------------------------------+-----------------------+
*       | Board name          | Revisions       |
*       +-------------------------------+-----------------------+
*       | 01 - RD-EX120-24G       |            |
*       |                | 01 - Rev 1.0     |
*       |                | 02 - Rev 2.0     |
*       +-------------------------------+-----------------------+
*       | 02 - DB-MX610-48F4GS     |            |
*       |                | 01 - Rev 1.0     |
*       |                | 02 - Rev 2.5     |
*       +-------------------------------+-----------------------+
*
*/
GT_STATUS appDemoShowBoardsList(GT_VOID)
{
    GT_BOARD_LIST_ELEMENT   *pBoardInfo;    /* Holds the board information. */
    GT_U32                  i;              /* Loops index.                 */
    GT_STATUS               rc = GT_OK;
    GT_U32                  line;

#if 0	
    if (systemInitialized != GT_TRUE)
    {
        /* do not show board list after system initialized */
    scrBufReset();
    scrBufPrintf("\nSupported boards:\n");

    /* Print table header */
    line = scrBufCursor / SCREEN_COLS;
    scrBufPrintHorLine(line);
    scrBufCoordPrintf(line + 1, 2, BOARD_COLUMN_WIDTH, "Board name");
    scrBufCoordPrintf(line + 1, BOARD_COLUMN_WIDTH + 3, REV_COLUMN_WIDTH,
        "Revision");
    scrBufPrintHorLine(line + 2);
    scrBufCoordPutc(line + 1, 0, TABLE_VER);
    scrBufCoordPutc(line + 1, BOARD_COLUMN_WIDTH + 1, TABLE_VER);
    scrBufCoordPutc(line + 1, BOARD_COLUMN_WIDTH + REV_COLUMN_WIDTH + 3,
        TABLE_VER);
    scrBufCursor += SCREEN_COLS * 3;

    /* Print lines of table with boards */
    for(i = 0; i < boardListLen; i++)
    {
        pBoardInfo = &(boardsList[i]);
        if(osStrlen(pBoardInfo->boardName) == 0)
        {
            continue;
        }

        /* print the line */
        appDemoShowSingleBoard(pBoardInfo,i+1);
    }
    scrBufPrintf("\n");
    scrBufShow();

    osPrintf("Call cpssInitSystem(index,boardRevId,reloadEeprom), where:\n");
    osPrintf("\tindex        - The index of the system to be initialized.\n");
    osPrintf("\tboardRevId   - The index of the board revision.\n");
    osPrintf("\treloadEeprom - Whether the device's eeprom should be reloaded \n"
             "\t\t\t after start-init.\n");

    }
#endif
#if 0 /* use config.txt */
	cpssInitSystem(19,2,0);
#endif
	
    /* show the versions after all other printings ... so it will have better visibility */
    rc = appDemoShowCpssSwVersion();
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoShowCpssSwVersion", rc);
        return rc;
    }

    osPrintf("\n");
#if 0 /* use config.txt */
	/*Set default settings*/
	if ( appDemoPpConfigDevAmount == 2  ) /*Quagga 48 ports*/
	{
		WNCLedSetting(QUAGGA_48_PORT); /*LED*/
		WNCFCSetting(QUAGGA_48_PORT); /*Flow control*/
#if 0
		WNCSFPTXSetting(QUAGGA_48_PORT);/*SFP TX setting*/
		WNCQSGMIISetting(QUAGGA_48_PORT);/*QSGMII TX setting*/
#endif
		WNCVODSetting(QUAGGA_48_PORT);/*VOD setting*/
	}
	else if ( appDemoPpConfigDevAmount == 1) /*Quagga 24 ports*/
	{
		WNCLedSetting(QUAGGA_24_PORT); /*LED*/
		WNCFCSetting(QUAGGA_24_PORT); /*Flow control*/
#if 0
		WNCSFPTXSetting(QUAGGA_24_PORT);/*SFP TX setting*/
		WNCQSGMIISetting(QUAGGA_24_PORT);/*QSGMII TX setting*/
#endif
		WNCVODSetting(QUAGGA_24_PORT);/*VOD setting*/
	}

	/*Set auto tune*/
	WNC_Set10GRxAutoTune();
#endif /* End of config.txt */

    /* Replace the LED initialization from configuration file (config.txt) to here. */
    rc = wnc_xcat3LedInit();
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("wnc_xcat3LedInit", rc);
        return rc;
    }

    return GT_OK;
} /* appDemoShowBoardsList */

/**
* @internal appDemoTrafficEnable function
* @endinternal
*
* @brief   Enables all PP devices and all ports
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS appDemoTrafficEnable
(
    GT_VOID
)
{
    GT_STATUS                   rc;             /* return code                 */
    GT_U32                      devIdx;         /* device index                */
    APP_DEMO_SYS_CONFIG_FUNCS*  sysCfgFuncs;    /* pointer to System Functions */
    GT_U8                       dev;            /* device number               */

    /* Enable all devices and all ports */
    for (devIdx = SYSTEM_DEV_NUM_MAC(0);
          (devIdx < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount)); devIdx++)
    {

        /* get init parameters from appdemo init array */
        sysCfgFuncs = &appDemoPpConfigList[devIdx].sysConfigFuncs;
        dev = appDemoPpConfigList[devIdx].devNum;

        if(sysCfgFuncs->cpssTrafficEnable)
        {
            /* Enable traffic for given device */
            if(GT_FALSE == appDemoTrafficEnableDisableMode)
            {
                rc = sysCfgFuncs->cpssTrafficEnable(dev);

                CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssTrafficEnable", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal localUtfInit function
* @endinternal
*
* @brief   init the UTF
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
#ifdef CHX_FAMILY
static GT_STATUS localUtfHirInit(
                    IN GT_U8 devIdx)
{
    GT_STATUS rc = GT_OK;

#ifdef INCLUDE_UTF
    /* Initialize unit tests for CPSS */
    rc = utfPreInitPhase();
    if (rc != GT_OK)
    {
        utfPostInitPhase(GT_OK);
        return rc;
    }

    rc = utfInit(appDemoPpConfigList[devIdx].devNum);
    if (rc != GT_OK)
    {
        utfPostInitPhase(GT_OK);
        return rc;
    }

    utfPostInitPhase(GT_OK);
#endif /* INCLUDE_UTF */

    return rc;
}
#endif

/**
* @internal localUtfInit function
* @endinternal
*
* @brief   init the UTF
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS localUtfInit(GT_VOID)
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

    rc = utfInit(appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].devNum);
    if (rc != GT_OK)
    {
        utfPostInitPhase(rc);
        return rc;
    }

    utfPostInitPhase(rc);
#endif /* INCLUDE_UTF */

    return rc;
}

/**
* @internal localInitSharedMemoryTasks function
* @endinternal
*
* @brief   Run fdbLearning abd RxTx processes
*         for multi process appDemo
*         multiProcessAppDemo set by cpssInitSystem
*         if argv[2] == 2
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS localInitSharedMemoryTasks(GT_VOID)
{
#ifdef SHARED_MEMORY

    if (multiProcessAppDemo == 1)
    {

        /*****************************************/
        /*   Run fdbLearning and RxTx processes  */
        /****************************************/

        fdbLearningProcessId = vfork();

        if(fdbLearningProcessId < 0)
        {
                printf("Run fdbLearningProcessId failed\n");
                return GT_FAIL;
        }

        if(fdbLearningProcessId == 0)
        {
#ifndef LINUX_SIM
                execl("/usr/bin/fdbLearning", "fdbLearning",NULL);
#else
                execlp("fdbLearning", "fdbLearning",NULL);
#endif
                _exit (1);
        }


        RxTxProcessId = vfork();

        if(RxTxProcessId < 0)
        {
                printf("Run fdbLearningProcessId failed\n");
                return GT_FAIL;
        }

        if(RxTxProcessId == 0)
        {
#ifndef LINUX_SIM
                execl("/usr/bin/RxTxProcess", "RxTxProcess",NULL);
#else
                execlp("RxTxProcess", "RxTxProcess",NULL);
#endif
                _exit (1);
        }

        /* TODO: sleep to make chance fdbLearning and RxTxProcess to start */

    #ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
        /* Shared Memory implementation should be updated to use native MII APIs. */
        CPSS_TBD_BOOKMARK

        /* set eth port parameters for RxTxProcess */
        ethPortParams.valid = GT_TRUE;
        ethPortParams.appDemoSysConfig = appDemoSysConfig;
        rc = cpssMultiProcComSetSmiTransmitParams(&ethPortParams);
    #endif

    }
#endif /* SHARED_MEMORY */

    return GT_OK;
}

/**
* @internal cpssInitSystem_afterBasicConfig function
* @endinternal
*
* @brief   This 'after basic config' part of board initialization function, the
*         function using pre-called cpssInitSystem parameters.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssInitSystem_afterBasicConfig
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_STATUS   stat;
    /* define parameters to keep code as less impact as possible */

    GT_U32  boardRevId = storedBoardRevId;

    GT_U8   numOfPp   = storedNumOfPp;       /* Number of Pp's in system.                */
    GT_U8   numOfFa   = storedNumOfFa;       /* Number of Fa's in system.                */
    GT_U8   numOfXbar = storedNumOfXbar;     /* Number of Xbar's in system.              */
    GT_U32  value     = 0;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(appDemoInitSystemPhase1and2_done == GT_FALSE)
    {
        /* must run first part before this one ... we not want to crash on non initialized variables */
        return GT_NOT_INITIALIZED;
    }

    /* appDemoEventHandlerPreInit:
        needed to be called before:
        boardCfgFuncs.boardAfterInitConfig(...)
        and before:
        appDemoEventRequestDrvnModeInit()
        */
    rc = appDemoEventHandlerPreInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventHandlerCounterMtxCreate", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*****************************************************************/
    /* Logic phase initialization                                    */
    /*****************************************************************/
    rc = appDemoBoardLogicalInit((GT_U8)boardRevId,
                                 &boardCfgFuncs,
                                 numOfPp,
                                 numOfFa,
                                 numOfXbar);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBoardLogicalInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
    stat=appDemoDbEntryGet("initSystemWithoutInterrupts", &value);
    if ((appDemoPrePhase1Init == GT_TRUE) && ((stat == GT_NO_SUCH) || ((stat == GT_OK) && (value == 0))))
    {
        /* for this special mode only events handler should be invoked */
        /* spawn the user event handler processes */
        rc = appDemoEventRequestDrvnModeInit();
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventRequestDrvnModeInit", rc);
        return rc;

    }

    /*****************************************************************/
    /* General phase initialization                                  */
    /*****************************************************************/
    rc = appDemoBoardGeneralInit((GT_U8)boardRevId,
                                 &boardCfgFuncs,
                                 numOfPp,
                                 numOfFa,
                                 numOfXbar);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBoardGeneralInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

#ifdef CHX_FAMILY
    rc = auFdbUpdateLockInit();
    if (rc != GT_OK)
        return rc;
#endif /*CHX_FAMILY*/

    if(appDemoOnDistributedSimAsicSide == 0)
    {
        /*****************************************************************/
        /* Perform board specific settings after logical phase           */
        /*****************************************************************/
        if (boardCfgFuncs.boardAfterInitConfig != NULL)
        {
            rc = boardCfgFuncs.boardAfterInitConfig((GT_U8)boardRevId);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardAfterInitConfig", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        stat=appDemoDbEntryGet("initSystemWithoutInterrupts", &value);
        if ((numOfPp + numOfFa + numOfXbar) != 0       &&
            (appDemoBypassEventInitialize == GT_FALSE) &&
            ( system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E ) &&
            ((stat == GT_NO_SUCH) || ((stat == GT_OK) && (value == 0))))
        {

            /* spawn the user event handler processes */
            rc = appDemoEventRequestDrvnModeInit();
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventRequestDrvnModeInit", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* Enable all PP devices and all ports */
    rc = appDemoTrafficEnable();
    if (rc != GT_OK)
    {
        return rc;
    }

    systemInitialized = GT_TRUE;

    /****************************************
    * Run fdbLearning abd RxTx processes
    * for multi process appDemo
    *
    *  multiProcessAppDemo set by cpssInitSystem
    *  if argv[2] == 2
    ****************************************/
    rc = localInitSharedMemoryTasks();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("localInitSharedMemoryTasks", rc);
    if (rc != GT_OK)
    {
        return rc;
    }


    /*****************************************************************/
    /* External modules initializations                              */
    /*****************************************************************/

    rc = localUtfInit();
    return rc;
}
#ifdef EMBEDDED_COMMANDS_EXISTS
#ifdef CHX_FAMILY
void cpssDxChEcLuaInit (void);

static GT_VOID dx_EMBCOM_init_LUA()
{
#ifdef CMD_LUA_CLI
    /* add Embeddded Commands Lua scripts */
    cpssDxChEcLuaInit();
#endif
}
#endif
#endif
/**
* @internal cpssInitSystem function
* @endinternal
*
* @brief   This is the main board initialization function.
*
* @param[in] boardIdx                 - The index of the board to be initialized from the board
*                                      list.
* @param[in] boardRevId               - Board revision Id.
* @param[in] reloadEeprom             - Whether the Eeprom should be reloaded when
*                                      corePpHwStartInit() is called.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssInitSystem
(
    IN  GT_U32  boardIdx,
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
)
{
    GT_STATUS               rc = GT_OK;
    GT_BOARD_LIST_ELEMENT   *pBoardInfo;    /* Holds the board information              */
    GT_U8                   numOfPp;        /* Number of Pp's in system.                */
    GT_U8                   numOfFa;        /* Number of Fa's in system.                */
    GT_U8                   numOfXbar;      /* Number of Xbar's in system.              */
    GT_U32                  value;/*tmp value*/
    GT_U32      start_sec  = 0;
    GT_U32      start_nsec = 0;
    GT_U32      end_sec  = 0;
    GT_U32      end_nsec = 0;
    GT_U32      diff_sec;
    GT_U32      diff_nsec;
    /*CPSS_SYSTEM_RECOVERY_INFO_STC   hsuInit = {CPSS_SYSTEM_RECOVERY_INIT_STATE_E,
        {GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE},CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E}; *//* To deny register access during init */
    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */
#ifdef CHX_FAMILY
    PRV_CPSS_DXCH_PP_REGS_ADDR_STC *regsAddr;
    GT_U32 devIdx;
#ifdef EMBEDDED_COMMANDS_EXISTS
    dx_EMBCOM_init_LUA();
#endif
#endif
    if (isHirApp)
    {

       cpssOsPrintf ("Running in HIR mode. Please run appDemo(Sim) without -hir flag\n");
       return GT_FAIL;
    }

    rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    if((appDemoDbEntryGet("falcon_onEmulator_prepare", &value) == GT_OK) && (value != 0))
    {
#ifdef CHX_FAMILY
        falcon_onEmulator_prepare();
#endif /*CHX_FAMILY*/
    }

    if((appDemoDbEntryGet("initSystemOnlyPhase1and2", &value) == GT_OK) && (value != 0))
    {
        appDemoInitSystemOnlyPhase1and2 = GT_TRUE;
    }

    rc = memoryLeakageDbg();
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("memoryLeakageDbg", rc);
        return rc;
    }
    /* indicate that we want ability to detect 'Cached DMA' overflow */
    osCacheDmaOverflowDetectionEnableSet(GT_TRUE);

    /* Init CPSS with register defaults and HW aceess */
    if((appDemoDbEntryGet("initRegDefaults", &value) == GT_OK) && (value != 0))
    {
        appDemoInitRegDefaults = GT_TRUE;

        /* when preventing from writing to HW the PP init stage will result in EEPROMDoneIntMemNotDone
           register 0x58 bits 17-18; EPROM initialization is performed and Internal memory initialization is not performed.
           The correct PP init stage should be FullyFunc, Pipe Initialization complete. */

            /* Use HSU to prevent init system from writing to HW during init */
            /*rc = cpssSystemRecoveryStateSet(&hsuInit);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssSystemRecoveryStateSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }*/
    }
    else
    {
        appDemoInitRegDefaults = GT_FALSE;
    }

                /*   set multiprocess or regular appDemo      */
    if ( (boardIdx & APP_DEMO_CV_INIT_MASK) == APP_DEMO_CV_INIT_MASK)
    {
        appDemoPrePhase1Init = GT_TRUE;
        boardIdx = boardIdx - APP_DEMO_CV_INIT_MASK;
    }

#ifdef SHARED_MEMORY

    if(reloadEeprom == 2)
        multiProcessAppDemo = 1;
    else
        multiProcessAppDemo = 0;

#endif


    /****************************************************/
    /* Check parameters                                 */
    /****************************************************/
    if ((boardIdx > boardListLen) || (boardIdx == 0))
    {
        rc = GT_BAD_PARAM;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardIdx is out of range", rc);
        return rc;
    }

    pBoardInfo = &(boardsList[boardIdx - 1]);
    /* Check if board registerFunc is not NULL */
    if (NULL == pBoardInfo->registerFunc)
    {
        rc = GT_NOT_SUPPORTED;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("This board type was not implemented (it is only 'place holder')", rc);
        return rc;
    }

    if (boardRevId > pBoardInfo->numOfRevs)
    {
        rc = GT_FAIL;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("Revision is out of range", rc);
        return rc;
    }

    storedBoardIdx     = boardIdx;
    storedBoardRevId   = boardRevId;
    storedReloadEeprom = reloadEeprom;

    /* check if we run RTOS on simulation , to connect remote simulation */
#if (defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)
    /*simulation initialization*/
    appDemoRtosOnSimulationInit();

#endif /*(defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)*/

#ifdef ASIC_SIMULATION
    while(simulationInitReady == 0)
    {
        /* wait for simulation to be in operational state */
        /* otherwise the init will fail for devices with multi-port groups , like:
        Lion,Lion2,macPhy
        or for systems with multiple devices */
        cpssOsTimerWkAfter(200);
        cpssOsPrintf(".");
    }
    cpssOsPrintf("\n");

    if(sasicgSimulationRole != SASICG_SIMULATION_ROLE_NON_DISTRIBUTED_E)
    {
        switch(sasicgSimulationRole)
        {
            case SASICG_SIMULATION_ROLE_DISTRIBUTED_APPLICATION_SIDE_E:
            case SASICG_SIMULATION_ROLE_DISTRIBUTED_APPLICATION_SIDE_VIA_BROKER_E:
                break;
            default:
                /*bypass the need to call from terminal*/
                appDemoOnDistributedSimAsicSide = 1;
                break;
        }

        cpssOsPrintf(" cpssInitSystem : initialize for Distributed Simulation [%s]\n",consoleDistStr[sasicgSimulationRole]);
    }
#endif /*ASIC_SIMULATION*/

    /* Enable printing inside interrupt routine. */
    extDrvUartInit();

    /* Call to fatal_error initialization, use default fatal error call_back */
    rc = osFatalErrorInit((FATAL_FUNC_PTR)NULL);
    if (rc != GT_OK)
    {
        return rc;
    }

#ifdef VXWORKS
    {
        /* init Debug Services by default */
        GT_BOOL forceNoOsDebugServicesInit = GT_FALSE;

        if(appDemoDbEntryGet("forceNoOsDebugServicesInit", &value) == GT_OK)
        {
            if (value == 0)
            {
                forceNoOsDebugServicesInit = GT_FALSE;
            }
            else
            {
                forceNoOsDebugServicesInit = GT_TRUE;
            }


        }

        if (forceNoOsDebugServicesInit == GT_FALSE)
        {
            /* Initialize the Debug Services */
            rc = osDebugServicesInit();
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

#endif


    /*****************************************************************/
    /* global initialization phase must be done before phase 1 and 2 */
    /*****************************************************************/

    /* Make sure all pointers are NULL in case user won't init all pointers */
    osMemSet(&boardCfgFuncs, 0, sizeof(GT_BOARD_CONFIG_FUNCS));

    /* Make sure all data is cleared */
    osMemSet(&appDemoSysConfig, 0, sizeof(appDemoSysConfig));

    /* ability to set different device numbers for stacking */
    if(appDemoDbEntryGet("firstDevNum", &value) == GT_OK)
    {
        appDemoSysConfig.firstDevNum = (GT_U8)value;
    }
    else
    {
        appDemoSysConfig.firstDevNum = 0;
    }

    /* Get board specific functions accordingly to board rev. id */
    rc = pBoardInfo->registerFunc((GT_U8)boardRevId, &boardCfgFuncs);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("pBoardInfo->registerFunc", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
        /* simplified Init */
    if(boardCfgFuncs.boardSimpleInit != NULL)
    {
        rc = boardCfgFuncs.boardSimpleInit(boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardSimpleInit", rc);
        return rc;
    }

    /* Do global initialization for AppDemo before all phases */
    rc = appDemoInitGlobalModuls(reloadEeprom);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoInitGlobalModuls", rc);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* Get board information */
    rc = boardCfgFuncs.boardGetInfo((GT_U8)boardRevId,
                                    &numOfPp,
                                    &numOfFa,
                                    &numOfXbar,
                                    &gIsB2bSystem);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardGetInfo", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* for fasten the "all PPs loops */
    appDemoPpConfigDevAmount = numOfPp;

    storedNumOfPp =    numOfPp;
    storedNumOfFa =    numOfFa;
    storedNumOfXbar =  numOfXbar;

    /* Does board specific settings before phase 1 */
    if (boardCfgFuncs.boardBeforePhase1Config != NULL)
    {
        rc = boardCfgFuncs.boardBeforePhase1Config((GT_U8)boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardBeforePhase1Config", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    /*****************************************************************/
    /* HW phase 1 initialization                                     */
    /*****************************************************************/
    rc = appDemoBoardPhase1Init((GT_U8)boardRevId,
                                &boardCfgFuncs,
                                numOfPp,
                                numOfFa,
                                numOfXbar);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBoardPhase1Init", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

#ifdef CHX_FAMILY
    /* Getting the first devNum, which is the first device index in the appDemo device list
       and which other devices stored incrementally  */
    devIdx = SYSTEM_DEV_NUM_MAC(0);
#endif

    /* Disable port VOS override */
    if(appDemoDbEntryGet("disableVosOverride", &value) == GT_OK)
    {
#ifdef CHX_FAMILY
        rc = cpssDxChPortVosOverrideControlModeSet(appDemoPpConfigList[devIdx].devNum, value);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortVosOverrideControlModeSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
#endif /*CHX_FAMILY*/
    }

    if (appDemoInitRegDefaults == GT_TRUE)
    {

#ifdef CHX_FAMILY
        if(PRV_CPSS_PP_MAC(appDemoPpConfigList[devIdx].devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            regsAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(appDemoPpConfigList[devIdx].devNum);
            /* Set CPU to work in SDMA mode */
            rc = prvCpssHwPpPortGroupSetRegField(appDemoPpConfigList[devIdx].devNum, 0, regsAddr->globalRegs.cpuPortCtrlReg, 1, 1, 1);
            /* Set <sel_port_sdma> in General Configuration register to be 0 */
            rc = prvCpssHwPpSetRegField(appDemoPpConfigList[devIdx].devNum, regsAddr->globalRegs.generalConfigurations, 0, 1, 0);
        }
#endif /*CHX_FAMILY*/

         /* when preventing from writing to HW the PP init stage will result in EEPROMDoneIntMemNotDone
           register 0x58 bits 17-18; EPROM initialization is performed and Internal memory initialization is not performed.
           The correct PP init stage should be FullyFunc, Pipe Initialization complete. */

            /* Retun HSU to normal state for HW access */
            /*hsuInit.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
            rc = cpssSystemRecoveryStateSet(&hsuInit);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssSystemRecoveryStateSet", rc);*/

        cpssOsPrintf("\nRegister defaults are used\n");
        /* Stop init system */
        return rc;
    }

    if((appDemoOnDistributedSimAsicSide == 0) && (appDemoPrePhase1Init == GT_FALSE))
    {
        /* Does board specific settings after phase 1 */
        if (boardCfgFuncs.boardAfterPhase1Config != NULL)
        {
            rc = boardCfgFuncs.boardAfterPhase1Config((GT_U8)boardRevId);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardAfterPhase1Config", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /*****************************************************************/
    /* HW phase 2 initialization                                     */
    /*****************************************************************/
    rc = appDemoBoardPhase2Init((GT_U8)boardRevId,
                                &boardCfgFuncs,
                                numOfPp,
                                numOfFa,
                                numOfXbar);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBoardPhase2Init", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
    if((appDemoOnDistributedSimAsicSide == 0) && (appDemoPrePhase1Init == GT_FALSE))
    {
        /* Does board specific settings after phase 2 */
        if (boardCfgFuncs.boardAfterPhase2Config != NULL)
        {
            rc = boardCfgFuncs.boardAfterPhase2Config((GT_U8)boardRevId);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardAfterPhase2Config", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* indicate that phase 1,2 done*/
    appDemoInitSystemPhase1and2_done = GT_TRUE;

    if (appDemoInitSystemOnlyPhase1and2 == GT_FALSE)
    {
        /* continue to 'after basic config' */
        rc = cpssInitSystem_afterBasicConfig();
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssInitSystem_afterBasicConfig", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
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
    }

    appDemoCpssInitSkipHwReset = GT_FALSE;/* clear it , (not really needed) */

    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;

    cpssOsPrintf("Time processing the cpssInitSystem (from 'phase1 init') is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);
#if defined  CPSS_USE_MUTEX_PROFILER
    cpssInitSystemIsDone = GT_TRUE;
#endif

    return rc;
} /* cpssInitSystem */

/**
* @internal cpssInitSystem_reducedAfterPhase2InitPortsAndTraffic function
* @endinternal
*
* @brief   function for basic testing of the "appDemoInitSystemOnlyPhase1and2" mode.
*         function implemented only for 'DXCH' for bobcat2.
*         run :
*         @@appDemoDbEntryAdd "initSystemOnlyPhase1and2",1
*         cpssInitSystem 29,1,0
*         cpssInitSystem_reducedAfterPhase2InitPortsAndTraffic
*         --> this will allow test 'flood' in vlan 1 and with 'add mac entry' (by configuration)
*         also to test 'single destination'.
*         NOTE: no 'controlled learning' as there are no 'tasks' that handle events ...
*         also event not enabled on the device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_INITIALIZED       - cpssInitSystem was not previously called.
*/
GT_STATUS cpssInitSystem_reducedAfterPhase2InitPortsAndTraffic(GT_VOID)
{
    GT_STATUS   rc = GT_OK;

    if(appDemoInitSystemPhase1and2_done == GT_FALSE)
    {
        /* must run first part before this one ... we not want to crash on non initialized variables */
        return GT_NOT_INITIALIZED;
    }

#ifdef CHX_FAMILY
    {
        GT_U32  boardRevId = storedBoardRevId;
        GT_U32  devIdx, numOfNetIfs;
        GT_U32  numOfDev = storedNumOfPp;
        GT_U8   dev;
        GT_U16  vlanId, i;
        GT_PHYSICAL_PORT_NUM portNum;
        GT_PHYSICAL_PORT_NUM cpuPortNumArr[CPSS_MAX_SDMA_CPU_PORTS_CNS]; /* array for multiple CPU ports*/
        /* loop the devices  */
        for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < SYSTEM_DEV_NUM_MAC(numOfDev); devIdx++)
        {
            if (appDemoPpConfigList[devIdx].valid != GT_TRUE)
            {
                continue;
            }

            if (appDemoPpConfigList[devIdx].devFamily == CPSS_PX_FAMILY_PIPE_E)
            {
                continue;
            }

            dev = appDemoPpConfigList[devIdx].devNum;

            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                /* init the ports */
                rc = appDemoBc2PortInterfaceInit(dev,(GT_U8)boardRevId);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBc2PortInterfaceInit", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }

                rc = cpssDxChPhyPortSmiInit(dev);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInit", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* init the phys */
                rc = bobcat2BoardPhyConfig((GT_U8)boardRevId,dev);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("bobcat2BoardPhyConfig", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
                /*
                    the default of HW is all 256 ports in vlan=1 (ports 0..255)

                    but we need to remove port 255 from vlan ...
                    because port 255 is 'force link up' ... for 'cpu_port' purposes
                    (used internally by the cpss implementation.. see use of
                    PRV_CPSS_DXCH_TARGET_LOCAL_LINK_STATUS_CPU_PORT_NUM_CNS)

                   also remove port 63 the 'cpu port' from vlan so it will not get 'flooding'
                */
                vlanId = 1;
                portNum = 255;
                rc = cpssDxChBrgVlanPortDelete(dev,vlanId,portNum);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanPortDelete", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }

                vlanId = 1;
                rc = prvCpssDxChPortMappingCPUPortGet(dev, cpuPortNumArr, &numOfNetIfs);
                if (rc != GT_OK)
                {
                    return rc;
                }
                for (i=0; i < numOfNetIfs; i++)
                {
                    rc = cpssDxChBrgVlanPortDelete(dev,vlanId,cpuPortNumArr[i]);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanPortDelete", rc);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }

            /*enabe the traffic on the device and on the ports */
            rc = appDemoPpConfigList[devIdx].sysConfigFuncs.cpssTrafficEnable(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssTrafficEnable", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
#endif /*CHX_FAMILY*/

    return rc;
}


/* function to detect memory leakage between calls to it */
static GT_STATUS memoryLeakageDbg(void)
{
    GT_STATUS rc;

    if(initCounter) /* not relevant for first time as can't compare with previous one */
    {
        if(initCounter >= 2)
        {
            /*
                in the case of initCounter == 1
                we will find huge amount of 'memory leaks' due to pointers that
                are going to be used after second initialization too.
                ('static' allocations , and DB allocations that re-used (not done free))

                the 'memory leakage' detected :
                not free memory that was alloc/realloc total of [9203056] bytes in [1596] elements
            */

            osPrintSync("cpssInitSystem : start print memory leaks \n");
            osMallocMemoryLeakageDbgPrintStandingAllocated();
            osPrintSync("cpssInitSystem : end print memory leaks \n");
        }

        osPrintSync("cpssInitSystem : state to STOP recording allocations and free management to find memory leaks \n");
        /*state to stop recording allocations and free management to find memory leaks */
        osMallocMemoryLeakageDbgAction(OSMEM_MEMORY_LEAKAGE_STATE_DBG_OFF_E);
    }

    if(resetCounter)
    {
        /* next should be done before :
            osMallocMemoryLeakageDbgAction(OSMEM_MEMORY_LEAKAGE_STATE_DBG_ON_E);

           and after
            osMallocMemoryLeakageDbgAction(OSMEM_MEMORY_LEAKAGE_STATE_DBG_OFF_E);

          because it originally happen before cpssInitSystem --> meaning in 'off state'
        */
        osPrintSync("cpssResetSystem : appdemo pre init (to be ready for cpssInitSystem) \n");
        rc = appDemoCpssInit();
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoCpssInit", rc);
            return rc;
        }
    }

    osPrintSync("cpssInitSystem : state to START recording allocations and free management to find memory leaks \n");
    /*state to start recording allocations and free management to find memory leaks */
    osMallocMemoryLeakageDbgAction(OSMEM_MEMORY_LEAKAGE_STATE_DBG_ON_E);

    initCounter++;

    return GT_OK;
}

/**
* @internal cpssInitSystemSet function
* @endinternal
*
* @brief   Function set the parameters for hir, specific to boardIdx and RevId
* which is hardcoded (for Aldrin)
*
* @param[out] boardIdxPtr              - (pointer to) The index of the board to be initialized
*                                      from the board list.
* @param[out] boardRevIdPtr            - (pointer to) Board revision Id.
* @param[out] reloadEepromPtr          - (pointer to) Whether the Eeprom should be reloaded when
*                                      corePpHwStartInit() is called.
*                                       None.
*/
void cpssInitSystemSet
(
    IN GT_U32  boardIdx,
    IN GT_U32  boardRevId
)
{
    storedBoardIdx = boardIdx;
    storedBoardRevId=boardRevId;
}

/**
* @internal cpssInitSystemGet function
* @endinternal
*
* @brief   Function sets parameters of cpss init system proccess.
*
* @param[out] boardIdxPtr              - (pointer to) The index of the board to be initialized
*                                      from the board list.
* @param[out] boardRevIdPtr            - (pointer to) Board revision Id.
* @param[out] reloadEepromPtr          - (pointer to) Whether the Eeprom should be reloaded when
*                                      corePpHwStartInit() is called.
*                                       None.
*/
void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
)
{
    *boardIdxPtr = storedBoardIdx;
    *boardRevIdPtr = storedBoardRevId;
    *reloadEepromPtr = storedReloadEeprom;
}

/**
* @internal cpssReInitSystem function
* @endinternal
*
* @brief   Run cpssInitSystem according to previously initialized parameters
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssReInitSystem
(
    GT_VOID
)
{
    GT_U32 boardIdx, boardRevId, reloadEeprom;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);
    return cpssInitSystem(boardIdx, boardRevId, reloadEeprom);
}



#ifdef CHX_FAMILY
/*******************************************************************************
* cpssMicroInitSequenceCreate
*
* DESCRIPTION:
*       Function generates micro-init config file.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       microInitParamsPtr - (pointer to) micro-init init parameters
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssMicroInitSequenceCreate
(
    IN APP_DEMO_MICRO_INIT_STC *microInitParamsPtr
)
{
    GT_STATUS                           rc;
    GT_BOARD_LIST_ELEMENT               *pBoardInfo = NULL;     /* Holds the board information              */
    CPSS_PP_PHASE1_INIT_PARAMS          corePpPhase1Params;     /* Phase 1 PP params                        */
    GT_U8                               devIdx;                 /* loop index                               */
    GT_U8                               numOfPp;                /* Number of Pp's in system.                */
    GT_U8                               numOfFa;                /* Number of Fa's in system.                */
    GT_U8                               numOfXbar;              /* Number of Xbar's in system.              */
    GT_U32                              boardIdx;
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   cpssPpPhase1Params;
    GT_U32                              boardType = 0;


     /* Make sure all pointers are NULL in case user won't init all pointers */
    osMemSet(&boardCfgFuncs, 0, sizeof(GT_BOARD_CONFIG_FUNCS));

    /* Make sure all data is cleared */
    osMemSet(&appDemoSysConfig, 0, sizeof(appDemoSysConfig));
    osMemSet(&cpssPpPhase1Params, 0, sizeof(CPSS_DXCH_PP_PHASE1_INIT_INFO_STC));
    switch (microInitParamsPtr->deviceId)
    {
    /* Bobcat2 devices */
    case CPSS_BOBCAT2_ALL_DEVICES_CASES_MAC:
        boardType = APP_DEMO_BC2_BOARD_DB_CNS;
        boardIdx = 29;
        break;
    case CPSS_BOBCAT3_ALL_DEVICES_CASES_MAC:
        boardType = APP_DEMO_BOBCAT3_BOARD_DB_CNS;
        boardIdx = 29;
        break;
    /* BobK devices */
    case CPSS_BOBK_CAELUM_DEVICES_CASES_MAC:
        boardType = APP_DEMO_CAELUM_BOARD_DB_CNS;
        boardIdx = 29;
        break;
    case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
        boardType = APP_DEMO_CETUS_BOARD_DB_CNS;
        boardIdx = 29;
        break;
    case CPSS_ALDRIN_DEVICES_CASES_MAC:
        boardType = APP_DEMO_ALDRIN_BOARD_DB_CNS;
        boardIdx = 29;
        break;
    case CPSS_ALDRIN2_ALL_DEVICES_CASES_MAC:
        boardType = APP_DEMO_ALDRIN2_BOARD_DB_CNS;
        boardIdx = 29;
        break;
    case CPSS_XCAT3_ALL_DEVICES_CASES_MAC:
        boardIdx = 19;
        break;
    default:
        return GT_NOT_APPLICABLE_DEVICE;
    }
    pBoardInfo = &(boardsList[boardIdx - 1]);

    /* Get board specific functions accordingly to board rev. id */
    rc = pBoardInfo->registerFunc((GT_U8)1, &boardCfgFuncs);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Do global initialization for AppDemo before all phases */
    rc = appDemoInitGlobalModuls(0);
    if (rc != GT_OK)
    {
        return rc;
    }
      /* Get board information */
    rc = boardCfgFuncs.boardGetInfo((GT_U8)1,
                                    &numOfPp,
                                    &numOfFa,
                                    &numOfXbar,
                                    &gIsB2bSystem);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Get board specific functions accordingly to board rev. id */
    for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < SYSTEM_DEV_NUM_MAC(numOfPp); devIdx++)
    {

#ifdef ASIC_SIMULATION
         /* set core clock */

        switch (microInitParamsPtr->deviceId)
        {
        /* Bobcat2 devices */
        case CPSS_BOBCAT2_ALL_DEVICES_CASES_MAC:
            rc = appDemoBcat2B0SimCoreClockSet(devIdx,microInitParamsPtr->coreClock);
            break;
        /* Bobcat3 devices */
        case CPSS_BOBCAT3_ALL_DEVICES_CASES_MAC:
            rc = appDemoBc3SimCoreClockSet(devIdx,microInitParamsPtr->coreClock);
            break;
        /* BobK devices */
        case CPSS_BOBK_ALL_DEVICES_CASES_MAC:
            rc = appDemoBobKSimCoreClockSet(devIdx,microInitParamsPtr->coreClock);
            break;
        case CPSS_ALDRIN_DEVICES_CASES_MAC:
            rc = appDemoAldrinSimCoreClockSet(devIdx,microInitParamsPtr->coreClock);
            break;
        case CPSS_ALDRIN2_ALL_DEVICES_CASES_MAC:
            rc = appDemoBc3SimCoreClockSet(devIdx,microInitParamsPtr->coreClock);
            break;
        case CPSS_XCAT3_ALL_DEVICES_CASES_MAC:
            break;
        default:
            return GT_NOT_APPLICABLE_DEVICE;
        }

        if (rc != GT_OK)
        {
            osPrintf(" can't set coreClock\n");
            return rc;
        }
#endif
        /* Get PP config of this device */
        rc = boardCfgFuncs.boardGetPpPh1Params((GT_U8)1,
                                                devIdx,
                                                &corePpPhase1Params);
        if (rc != GT_OK)
        {
            return rc;
        }
        corePpPhase1Params.mngInterfaceType = microInitParamsPtr->mngInterfaceType;
#ifdef ASIC_SIMULATION
        corePpPhase1Params.coreClk = microInitParamsPtr->coreClock;
#else
        corePpPhase1Params.coreClk = APP_DEMO_CPSS_AUTO_DETECT_CORE_CLOCK_CNS;
#endif
        if ((appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) ||
            (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
            (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E) ||
            (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
            (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E))
        {
            corePpPhase1Params.mngInterfaceType = CPSS_CHANNEL_PEX_MBUS_E;
            /* Address Completion Region 1 - for Interrupt Handling    */
            corePpPhase1Params.isrAddrCompletionRegionsBmp = 0x02;
            /* Address Completion Regions 2,3,4,5 - for Application    */
            corePpPhase1Params.appAddrCompletionRegionsBmp = 0x3C;
            /* Address Completion Regions 6,7 - reserved for other CPU */
        }

         /* force numOfPortGroups to be set */
        if (corePpPhase1Params.numOfPortGroups == 0)
            corePpPhase1Params.numOfPortGroups = 1;

        rc = appDemoConvert2DxChPhase1Info(devIdx,&corePpPhase1Params,&cpssPpPhase1Params);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* open trace */
        rc = appDemoTraceHwAccessOutputModeSet(APP_DEMO_TRACE_OUTPUT_MODE_DB_E);
        if (GT_OK != rc)
        {
            return rc;
        }
        rc = appDemoTraceHwAccessEnable(devIdx,APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E,GT_TRUE);
        if (GT_OK != rc)
        {
            return rc;
        }
        /* do all basic part + EPLR and Extended DSA bypass bridge WA if needed */
        rc = cpssDxChMicroInitBasicCodeGenerate(&cpssPpPhase1Params,microInitParamsPtr->featuresBitmap);
        if (GT_OK != rc)
        {
            return rc;
        }
        if (microInitParamsPtr->featuresBitmap & 0x4)
        {
            rc = appDemoLedInterfacesInit(devIdx,boardType);

            if (GT_OK != rc)
            {
                return rc;
            }
        }

        rc = appDemoTraceHwAccessEnable(devIdx,APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E,GT_FALSE);
        if (GT_OK != rc)
        {
            return rc;
        }
        rc = appDemoTraceHwAccessOutputModeSet(APP_DEMO_TRACE_OUTPUT_MODE_DIRECT_E);
        if (GT_OK != rc)
        {
            return rc;
        }
    }
    return GT_OK;
}

#endif  /*CHX_FAMILY*/

#ifndef CHX_FAMILY
#ifdef PX_FAMILY
/**
* @internal cpssMicroInitSequenceCreate function
* @endinternal
*
* @brief   Function generates micro-init config file.
*
* @param[out] microInitParamsPtr       - (pointer to) micro-init init parameters
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssMicroInitSequenceCreate
(
    IN APP_DEMO_MICRO_INIT_STC *microInitParamsPtr
)
{
    GT_STATUS                           rc;
    GT_BOARD_LIST_ELEMENT               *pBoardInfo = NULL;     /* Holds the board information              */
    GT_U32                              boardIdx;
    GT_SW_DEV_NUM                       devNum;
    GT_U32                              devIndex;/* device index in the array of appDemoPpConfigList[devIndex] */

/*    CPSS_PP_PHASE1_INIT_PARAMS          corePpPhase1Params;*/     /* Phase 1 PP params                        */
    /*CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   cpssPpPhase1Params;*/
    CPSS_HW_INFO_STC        hwInfo;
    GT_PCI_INFO             pciInfo;
    CPSS_PX_INIT_INFO_STC   initInfo;
    CPSS_PP_DEVICE_TYPE     px_devType;

    osMemSet(&initInfo, 0, sizeof(initInfo));

    devIndex = 0;
    devNum = SYSTEM_DEV_NUM_MAC(devIndex);

     /* Make sure all pointers are NULL in case user won't init all pointers */
    osMemSet(&boardCfgFuncs, 0, sizeof(GT_BOARD_CONFIG_FUNCS));

    /* Make sure all data is cleared */
    osMemSet(&appDemoSysConfig, 0, sizeof(appDemoSysConfig));
    /*osMemSet(&cpssPpPhase1Params, 0, sizeof(CPSS_DXCH_PP_PHASE1_INIT_INFO_STC));*/
    switch (microInitParamsPtr->deviceId)
    {
        /* Pipe devices */
        case CPSS_PIPE_ALL_DEVICES_CASES_MAC:
            boardIdx = 33;
            break;
        default:
            return GT_NOT_APPLICABLE_DEVICE;
    }
    pBoardInfo = &(boardsList[boardIdx - 1]);

    /* Get board specific functions accordingly to board rev. id */
    rc = pBoardInfo->registerFunc((GT_U8)1, &boardCfgFuncs);
    if (rc != GT_OK)
    {
        return rc;
    }
      /* Get board information */
    rc = boardCfgFuncs.boardGetHwInfo(&hwInfo, &pciInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* open trace */
    rc = appDemoTraceHwAccessOutputModeSet(APP_DEMO_TRACE_OUTPUT_MODE_DB_E);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = appDemoTraceHwAccessEnable(devNum,APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E,GT_TRUE);
    if (GT_OK != rc)
    {
        return rc;
    }

    initInfo.hwInfo = hwInfo;
    /* 8 Address Completion Region mode                        */
    initInfo.mngInterfaceType = microInitParamsPtr->mngInterfaceType;
    /* initInfo.mngInterfaceType      = CPSS_CHANNEL_PEX_MBUS_E; */
    /* Address Completion Region 1 - for Interrupt Handling (BY CPSS driver (current CPU)) */
    initInfo.isrAddrCompletionRegionsBmp = 0x02;
    /* Address Completion Regions 2,3,4,5 - for CPSS driver (current CPU) */
    initInfo.appAddrCompletionRegionsBmp = 0x3C;
    /* Address Completion Regions 6,7 - reserved for other CPU */
    initInfo.numOfDataIntegrityElements = 0;
    initInfo.dataIntegrityShadowPtr = NULL;
    initInfo.allowHwAccessOnly = GT_FALSE;
/*
#ifdef ASIC_SIMULATION
        corePpPhase1Params.coreClk = microInitParamsPtr->coreClock;
#else
        corePpPhase1Params.coreClk = APP_DEMO_CPSS_AUTO_DETECT_CORE_CLOCK_CNS;
#endif
*/
    rc = cpssPxMicroInitBasicCodeGenerate(devNum, &initInfo, microInitParamsPtr->featuresBitmap, &px_devType);
    if (GT_OK != rc)
    {
        return rc;
    }
    appDemoPpConfigList[devIndex].valid = GT_TRUE;
    appDemoPpConfigList[devIndex].devNum = CAST_SW_DEVNUM(devNum);
    appDemoPpConfigList[devIndex].deviceId = px_devType;
    appDemoPpConfigList[devIndex].devFamily = CPSS_PX_FAMILY_PIPE_E;
    appDemoPpConfigList[devIndex].apiSupportedBmp = APP_DEMO_PX_FUNCTIONS_SUPPORT_CNS;

    /* enable the ports after setting the 'CG mac' as one */
/*    rc = px_initPortStage(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_initPortStage", rc);
    if (GT_OK != rc)
        return rc;

    rc = px_deviceEnableStage(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("px_deviceEnableStage", rc);
    if (GT_OK != rc)
        return rc;
*/
/*----------------------------------------------------------------------------------------*/
    rc = appDemoTraceHwAccessEnable(devNum,APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E,GT_FALSE);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = appDemoTraceHwAccessOutputModeSet(APP_DEMO_TRACE_OUTPUT_MODE_DIRECT_E);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssMicroInitBasicForwardingTablesSet function
* @endinternal
*
*/
GT_STATUS cpssMicroInitBasicForwardingTablesSet
(
    IN GT_SW_DEV_NUM      devNum
)
{
    return px_forwardingTablesStage(devNum);
}
#endif /*PX_FAMILY*/
#endif /* #ifndef CHX_FAMILY */

/**
* @internal appDemoOsLog function
* @endinternal
*
* @brief   Function for printing the logs of cpss log
*
* @param[in] lib                      - the function will print the log of the functions in "lib".
* @param[in] type                     - the function will print the logs from "type".
* @param[in] format                   - usual printf  string.
*                                      ... - additional parameters.
*                                       None.
*/
GT_VOID appDemoOsLog
(
    IN    CPSS_LOG_LIB_ENT      lib,
    IN    CPSS_LOG_TYPE_ENT     type,
    IN    const char*           format,
    ...
)
{
    va_list args;
    char buffer[2048];
    GT_U32 tid = 0;

    /*cpssOsPrintf("Lib: %d\n",lib);
    cpssOsPrintf("Log type: %d\n",type);*/
    va_start(args, format);
    cpssOsVsprintf(buffer, format, args);
    va_end(args);

    if (prvOsLogThreadMode == APP_DEMO_CPSS_LOG_TASK_SINGLE_E
        || (prvOsLogThreadMode == APP_DEMO_CPSS_LOG_TASK_REF_E))
    {
        /* we can use prvOsLogTaskIdPtr in both cases because in case of
         * APP_DEMO_CPSS_LOG_TASK_SINGLE_E it will point to prvOsLogTaskId.*/
        if ((osTaskGetSelf(&tid) != GT_OK)
            || (prvOsLogTaskIdPtr == NULL)
            || (*prvOsLogTaskIdPtr != tid))
            return;
    }

    if (prvOsLogMode == APP_DEMO_CPSS_LOG_MODE_PRINT_E) /* print on the screen */
    {
        osPrintf(buffer);
    }

#ifdef CMD_LUA_CLI
    else if (prvOsLogMode == APP_DEMO_CPSS_LOG_MODE_MEMFS_FILE_E && prvOsLogMemFsFile >= 0) /* write to a memFs file */
    {
            if (cmdFS.write(prvOsLogMemFsFile,buffer,osStrlen(buffer)) < 0)
            {
                osPrintf("%s\n", cmdFS.lastError());
            }
    }
#endif  /* CMD_LUA_CLI */

#ifdef ASIC_SIMULATION
    else if (prvOsLogMode == APP_DEMO_CPSS_LOG_MODE_LOCAL_FILE_E && prvOsLogLocalFile >= 0) /* write to a local file */
    {
        if (fprintf(prvOsLogLocalFile, "%s", buffer) < 0)
        {
            osPrintf("log fprintf error\n");
        }
    }
#endif  /* ASIC_SIMULATION */
}

/**
* @internal appDemoOsLogModeSet function
* @endinternal
*
* @brief   Function for setting the mode of cpss log
*
* @param[in] mode                     -  to be set.
* @param[in] name                     - the  of the file.
*
* @retval GT_OK                    - if succsess
* @retval GT_BAD_PTR               - failed to open the file
*/
GT_STATUS appDemoOsLogModeSet
(
    IN    APP_DEMO_CPSS_LOG_MODE_ENT      mode,
    IN    GT_CHAR_PTR                     name
)
{
    /* close the log file descriptor (if needed) opened by previous mode */

    if (prvOsLogMode == APP_DEMO_CPSS_LOG_MODE_MEMFS_FILE_E && prvOsLogMemFsFile >= 0)
    {
#ifdef CMD_LUA_CLI
        cmdFS.close(prvOsLogMemFsFile);
#endif
        prvOsLogMemFsFile = -1;
    }
    else if (prvOsLogMode == APP_DEMO_CPSS_LOG_MODE_LOCAL_FILE_E && prvOsLogLocalFile != NULL)
    {
#ifdef ASIC_SIMULATION
            fclose(prvOsLogLocalFile);
#endif  /* ASIC_SIMULATION */
            prvOsLogLocalFile = NULL;
    }

    prvOsLogMode = mode;

    /* log into a file */
    if (mode == APP_DEMO_CPSS_LOG_MODE_MEMFS_FILE_E || mode == APP_DEMO_CPSS_LOG_MODE_LOCAL_FILE_E)
    {
        if (NULL == name)       /* log into a file with default name */
        {
#ifdef _WIN32            /* win32 */
            name = "c:\\temp\\dtemp\\cpssLog.txt";
#else                    /* linux */
            name = "testLog.txt";
#endif  /* _WIN32 */
        }

        if (mode == APP_DEMO_CPSS_LOG_MODE_MEMFS_FILE_E)
        {
#ifdef CMD_LUA_CLI
            prvOsLogMemFsFile = cmdFS.open(name,GEN_FS_O_WRONLY|GEN_FS_O_CREAT|GEN_FS_O_TRUNC);
#endif
            if (prvOsLogMemFsFile < 0)
            {
                osPrintf("failed to open memFS file: [%s] \n",name);
                return GT_BAD_PTR;
            }
        }
        else /* APP_DEMO_CPSS_LOG_MODE_LOCAL_FILE_E */
        {
#ifdef ASIC_SIMULATION
            prvOsLogLocalFile = fopen(name, "w");
#endif
            if (prvOsLogLocalFile == NULL)
            {
                osPrintf("failed to open local file: [%s] \n",name);
                return GT_BAD_PTR;
            }
        }
    }

    return GT_OK;
}

/**
* @internal appDemoOsLogStop function
* @endinternal
*
* @brief   Function for stop writing the logs into the file
*/
GT_VOID appDemoOsLogStop()
{

    if (prvOsLogMode == APP_DEMO_CPSS_LOG_MODE_MEMFS_FILE_E && prvOsLogMemFsFile >= 0)
    {
#ifdef CMD_LUA_CLI
            cmdFS.close(prvOsLogMemFsFile);
#endif
        prvOsLogMemFsFile = -1;
    }
    else if (prvOsLogMode == APP_DEMO_CPSS_LOG_MODE_LOCAL_FILE_E && prvOsLogLocalFile != NULL)
    {
#ifdef ASIC_SIMULATION
        fclose(prvOsLogLocalFile);
#endif  /* ASIC_SIMUMATION */
        prvOsLogLocalFile = NULL;
    }
}

/*******************************************************************************
 * Private functions implementation
 ******************************************************************************/

/**
* @internal appDemoInitGlobalModuls function
* @endinternal
*
* @brief   Initialize global settings for CPSS
*
* @param[in] reloadEeprom             -   reload eeprom flag
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS appDemoInitGlobalModuls
(
    IN  GT_U32  reloadEeprom
)
{
    GT_U32          i;            /* Loop index */

    /* save reload eeprom for future use */
    gReloadEeprom = (reloadEeprom == 1 ? GT_TRUE : GT_FALSE);

    /* Initialize the PP array with default parameters */
    for (i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        osMemSet(&appDemoPpConfigList[i], 0, sizeof(appDemoPpConfigList[i]));

        appDemoPpConfigList[i].ppPhase1Done = GT_FALSE;
        appDemoPpConfigList[i].ppPhase2Done = GT_FALSE;
        appDemoPpConfigList[i].ppLogicalInitDone = GT_FALSE;
        appDemoPpConfigList[i].ppGeneralInitDone = GT_FALSE;
        appDemoPpConfigList[i].valid = GT_FALSE;

        /* default value for cpu tx/rx mode */
        appDemoPpConfigList[i].cpuPortMode = CPSS_NET_CPU_PORT_MODE_SDMA_E;
        appDemoPpConfigList[i].allocMethod = CPSS_RX_BUFF_STATIC_ALLOC_E;

        /* cascading information */
        appDemoPpConfigList[i].numberOfCscdTrunks = 0;
        appDemoPpConfigList[i].numberOfCscdPorts = 0;
        appDemoPpConfigList[i].numberOfCscdTargetDevs = 0;
        appDemoPpConfigList[i].numberOf10GPortsToConfigure = 0;
        appDemoPpConfigList[i].internal10GPortConfigFuncPtr = NULL;
        appDemoPpConfigList[i].internalCscdPortConfigFuncPtr = NULL;
    }
    return GT_OK;
} /* appDemoInitGlobalModuls */

/*******************************************************************************
* cpssGetDefaultOsBindFuncs
*
* DESCRIPTION:
*       Receives default cpss bind from OS
* INPUTS:
*       None.
* OUTPUTS:
*       osFuncBindPtr - (pointer to) set of call back functions
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Function should reside into CPSS library to resolve correct
*       pointers to functions.
*
*******************************************************************************/
/* if not used shared memory or it's unix-like simulation do compile this */
#if !defined(SHARED_MEMORY)

static GT_STATUS cpssGetDefaultOsBindFuncs(
    OUT CPSS_OS_FUNC_BIND_STC *osFuncBindPtr
)
{

    osMemSet(osFuncBindPtr,0,sizeof(*osFuncBindPtr));

    /* bind the OS functions to the CPSS */
    osFuncBindPtr->osMemBindInfo.osMemBzeroFunc =             osBzero;
    osFuncBindPtr->osMemBindInfo.osMemSetFunc   =             osMemSet;
    osFuncBindPtr->osMemBindInfo.osMemCpyFunc   =             osMemCpy;
    osFuncBindPtr->osMemBindInfo.osMemMoveFunc  =             osMemMove;
    osFuncBindPtr->osMemBindInfo.osMemCmpFunc   =             osMemCmp;
    osFuncBindPtr->osMemBindInfo.osMemStaticMallocFunc =      osStaticMalloc;
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    osFuncBindPtr->osMemBindInfo.osMemMallocFunc =            osDbgMalloc_MemoryLeakageDbg;
    osFuncBindPtr->osMemBindInfo.osMemReallocFunc =           osRealloc_MemoryLeakageDbg;
    osFuncBindPtr->osMemBindInfo.osMemFreeFunc   =            osFree_MemoryLeakageDbg;
#else
    osFuncBindPtr->osMemBindInfo.osMemMallocFunc =            osDbgMalloc;
    osFuncBindPtr->osMemBindInfo.osMemReallocFunc =           osRealloc;
    osFuncBindPtr->osMemBindInfo.osMemFreeFunc   =            osFree;
#endif
    osFuncBindPtr->osMemBindInfo.osMemCacheDmaMallocFunc =    osCacheDmaMalloc;
    osFuncBindPtr->osMemBindInfo.osMemCacheDmaFreeFunc =      osCacheDmaFree;
    osFuncBindPtr->osMemBindInfo.osMemPhyToVirtFunc =         osPhy2Virt;
    osFuncBindPtr->osMemBindInfo.osMemVirtToPhyFunc =         osVirt2Phy;

    osFuncBindPtr->osStrBindInfo.osStrlenFunc       = osStrlen;
    osFuncBindPtr->osStrBindInfo.osStrCpyFunc       = osStrCpy;
    osFuncBindPtr->osStrBindInfo.osStrNCpyFunc      = osStrNCpy;
    osFuncBindPtr->osStrBindInfo.osStrChrFunc       = osStrChr;
    osFuncBindPtr->osStrBindInfo.osStrStrFunc       = osStrStr;
    osFuncBindPtr->osStrBindInfo.osStrRevChrFunc    = osStrrChr;
    osFuncBindPtr->osStrBindInfo.osStrCmpFunc       = osStrCmp;
    osFuncBindPtr->osStrBindInfo.osStrNCmpFunc      = osStrNCmp;
    osFuncBindPtr->osStrBindInfo.osStrCatFunc       = osStrCat;
    osFuncBindPtr->osStrBindInfo.osStrStrNCatFunc   = osStrNCat;
    osFuncBindPtr->osStrBindInfo.osStrChrToUpperFunc= osToUpper;
    osFuncBindPtr->osStrBindInfo.osStrTo32Func      = osStrTo32;
    osFuncBindPtr->osStrBindInfo.osStrToU32Func     = osStrToU32;
    osFuncBindPtr->osStrBindInfo.osStrTolFunc       = osStrTol;

    osFuncBindPtr->osSemBindInfo.osMutexCreateFunc     = osMutexCreate;
    osFuncBindPtr->osSemBindInfo.osMutexDeleteFunc     = osMutexDelete;
 #if defined  CPSS_USE_MUTEX_PROFILER
    osFuncBindPtr->osSemBindInfo.osMutexSetGlAttributesFunc = osMutexSetGlAttributes;
    osFuncBindPtr->osSemBindInfo.osCheckUnprotectedPerDeviceDbAccess = osCheckUnprotectedPerDeviceDbAccess;
 #endif

    osFuncBindPtr->osSemBindInfo.osMutexLockFunc       = osMutexLock;
    osFuncBindPtr->osSemBindInfo.osMutexUnlockFunc     = osMutexUnlock;

    osFuncBindPtr->osSemBindInfo.osSigSemBinCreateFunc = FORCE_FUNC_CAST osSemBinCreate;
#ifdef  LINUX
    osFuncBindPtr->osSemBindInfo.osSigSemMCreateFunc   = osSemMCreate;
    osFuncBindPtr->osSemBindInfo.osSigSemCCreateFunc   = osSemCCreate;
#endif
    osFuncBindPtr->osSemBindInfo.osSigSemDeleteFunc    = osSemDelete;
    osFuncBindPtr->osSemBindInfo.osSigSemWaitFunc      = osSemWait;
    osFuncBindPtr->osSemBindInfo.osSigSemSignalFunc    = osSemSignal;

    osFuncBindPtr->osIoBindInfo.osIoBindStdOutFunc  = osBindStdOut;
    osFuncBindPtr->osIoBindInfo.osIoPrintfFunc      = osPrintf;
#ifdef  LINUX
    osFuncBindPtr->osIoBindInfo.osIoVprintfFunc     = osVprintf;
#endif
    osFuncBindPtr->osIoBindInfo.osIoSprintfFunc     = osSprintf;
    osFuncBindPtr->osIoBindInfo.osIoSnprintfFunc    = osSnprintf;
#if defined(LINUX) || defined(VXWORKS) || defined(WIN32)
    osFuncBindPtr->osIoBindInfo.osIoVsprintfFunc    = osVsprintf;
    osFuncBindPtr->osIoBindInfo.osIoVsnprintfFunc   = osVsnprintf;
#endif
    osFuncBindPtr->osIoBindInfo.osIoPrintSynchFunc  = osPrintSync;
    osFuncBindPtr->osIoBindInfo.osIoGetsFunc        = osGets;

#if defined(LINUX) || defined(FREEBSD) || defined(WIN32)
    osFuncBindPtr->osIoBindInfo.osIoFlastErrorStrFunc = osFlastErrorStr;
    osFuncBindPtr->osIoBindInfo.osIoFopenFunc       = osFopen;
    osFuncBindPtr->osIoBindInfo.osIoFcloseFunc      = osFclose;
    osFuncBindPtr->osIoBindInfo.osIoRewindFunc      = osRewind;
    osFuncBindPtr->osIoBindInfo.osIoFprintfFunc     = osFprintf;
    osFuncBindPtr->osIoBindInfo.osIoFgets           = osFgets;
    osFuncBindPtr->osIoBindInfo.osIoFwriteFunc      = osFwrite;
    osFuncBindPtr->osIoBindInfo.osIoFreadFunc       = osFread;
    osFuncBindPtr->osIoBindInfo.osIoFgetLengthFunc  = osFgetLength;
    osFuncBindPtr->osIoBindInfo.osIoFatalErrorFunc  = (CPSS_OS_FATAL_ERROR_FUNC)osFatalError;
#endif

    osFuncBindPtr->osInetBindInfo.osInetNtohlFunc = osNtohl;
    osFuncBindPtr->osInetBindInfo.osInetHtonlFunc = osHtonl;
    osFuncBindPtr->osInetBindInfo.osInetNtohsFunc = osNtohs;
    osFuncBindPtr->osInetBindInfo.osInetHtonsFunc = osHtons;
    osFuncBindPtr->osInetBindInfo.osInetNtoaFunc  = osInetNtoa;

    osFuncBindPtr->osTimeBindInfo.osTimeWkAfterFunc = osTimerWkAfter;
    osFuncBindPtr->osTimeBindInfo.osTimeTickGetFunc = osTickGet;
    osFuncBindPtr->osTimeBindInfo.osTimeGetFunc     = osTime;
    osFuncBindPtr->osTimeBindInfo.osTimeRTFunc      = osTimeRT;
    osFuncBindPtr->osTimeBindInfo.osStrftimeFunc    = osStrftime;
#if defined(LINUX) || defined(VXWORKS) || defined(WIN32)
    osFuncBindPtr->osTimeBindInfo.osGetSysClockRateFunc = osGetSysClockRate;
#endif

#ifdef  LINUX
    osFuncBindPtr->osTimeBindInfo.osDelayFunc       = osDelay;
#endif

#if !defined(UNIX) || defined(ASIC_SIMULATION)
    osFuncBindPtr->osIntBindInfo.osIntEnableFunc   = osIntEnable;
    osFuncBindPtr->osIntBindInfo.osIntDisableFunc  = osIntDisable;
    osFuncBindPtr->osIntBindInfo.osIntConnectFunc  = osInterruptConnect;
    osFuncBindPtr->osIntBindInfo.osIntDisconnectFunc  = osInterruptDisconnect;
#endif
#if (!defined(FREEBSD) && !defined(UCLINUX)) || defined(ASIC_SIMULATION)
    /* this function required for sand_os_mainOs_interface.c
     * Now it is implemented for:
     *   all os with ASIC simulation
     *   vxWorks
     *   Win32
     *   Linux (stub which does nothing)
     */
    osFuncBindPtr->osIntBindInfo.osIntModeSetFunc  = FORCE_FUNC_CAST osSetIntLockUnlock;
#endif
    osFuncBindPtr->osRandBindInfo.osRandFunc  = osRand;
    osFuncBindPtr->osRandBindInfo.osSrandFunc = osSrand;

    osFuncBindPtr->osTaskBindInfo.osTaskCreateFunc = osTaskCreate;
    osFuncBindPtr->osTaskBindInfo.osTaskDeleteFunc = osTaskDelete;
    osFuncBindPtr->osTaskBindInfo.osTaskGetSelfFunc= osTaskGetSelf;
    osFuncBindPtr->osTaskBindInfo.osTaskLockFunc   = osTaskLock;
    osFuncBindPtr->osTaskBindInfo.osTaskUnLockFunc = osTaskUnLock;

#if defined(LINUX) || defined(VXWORKS) || defined(ASIC_SIMULATION)
    osFuncBindPtr->osStdLibBindInfo.osQsortFunc    = osQsort;
    osFuncBindPtr->osStdLibBindInfo.osBsearchFunc  = osBsearch;
#endif

    osFuncBindPtr->osMsgQBindInfo.osMsgQCreateFunc  = osMsgQCreate;
    osFuncBindPtr->osMsgQBindInfo.osMsgQDeleteFunc  = osMsgQDelete;
    osFuncBindPtr->osMsgQBindInfo.osMsgQSendFunc    = osMsgQSend;
    osFuncBindPtr->osMsgQBindInfo.osMsgQRecvFunc    = osMsgQRecv;
    osFuncBindPtr->osMsgQBindInfo.osMsgQNumMsgsFunc = osMsgQNumMsgs;
    osFuncBindPtr->osMsgQBindInfo.osMsgQNumMsgsFunc = osMsgQNumMsgs;
    osFuncBindPtr->osLogBindInfo.osLogFunc          = appDemoOsLog;

    osFuncBindPtr->osSocketInfo.osSocketLastErrorFunc = osSocketLastError;
    osFuncBindPtr->osSocketInfo.osSocketLastErrorStrFunc = osSocketLastErrorStr;
    osFuncBindPtr->osSocketInfo.osSocketTcpCreateFunc = osSocketTcpCreate;
    osFuncBindPtr->osSocketInfo.osSocketUdpCreateFunc = osSocketUdpCreate;
    osFuncBindPtr->osSocketInfo.osSocketTcpDestroyFunc = osSocketTcpDestroy;
    osFuncBindPtr->osSocketInfo.osSocketUdpDestroyFunc = osSocketUdpDestroy;
    osFuncBindPtr->osSocketInfo.osSocketCreateAddrFunc = osSocketCreateAddr;
    osFuncBindPtr->osSocketInfo.osSocketDestroyAddrFunc = osSocketDestroyAddr;
    osFuncBindPtr->osSocketInfo.osSocketBindFunc = osSocketBind;
    osFuncBindPtr->osSocketInfo.osSocketListenFunc = osSocketListen;
    osFuncBindPtr->osSocketInfo.osSocketAcceptFunc = osSocketAccept;
    osFuncBindPtr->osSocketInfo.osSocketConnectFunc = osSocketConnect;
    osFuncBindPtr->osSocketInfo.osSocketSetNonBlockFunc = osSocketSetNonBlock;
    osFuncBindPtr->osSocketInfo.osSocketSetBlockFunc = osSocketSetBlock;
    osFuncBindPtr->osSocketInfo.osSocketSendFunc = osSocketSend;
    osFuncBindPtr->osSocketInfo.osSocketSendToFunc = osSocketSendTo;
    osFuncBindPtr->osSocketInfo.osSocketRecvFunc = osSocketRecv;
    osFuncBindPtr->osSocketInfo.osSocketRecvFromFunc = osSocketRecvFrom;
    osFuncBindPtr->osSocketInfo.osSocketSetSocketNoLingerFunc = osSocketSetSocketNoLinger;
    osFuncBindPtr->osSocketInfo.osSocketExtractIpAddrFromSocketAddrFunc = osSocketExtractIpAddrFromSocketAddr;
    osFuncBindPtr->osSocketInfo.osSocketGetSocketAddrSizeFunc = osSocketGetSocketAddrSize;
    osFuncBindPtr->osSocketInfo.osSocketShutDownFunc = (CPSS_SOCKET_SHUTDOWN_FUNC)osSocketShutDown;

    osFuncBindPtr->osSocketSelectInfo.osSelectCreateSetFunc = osSelectCreateSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectEraseSetFunc = osSelectEraseSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectZeroSetFunc = osSelectZeroSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectAddFdToSetFunc = osSelectAddFdToSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectClearFdFromSetFunc = osSelectClearFdFromSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectIsFdSetFunc = osSelectIsFdSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectCopySetFunc = osSelectCopySet;
    osFuncBindPtr->osSocketSelectInfo.osSelectFunc = osSelect;
    osFuncBindPtr->osSocketSelectInfo.osSocketGetSocketFdSetSizeFunc = osSocketGetSocketFdSetSize;

    /* Now we should be sure that ALL pointers are set.
     * Occasionally you can add new field into the structure and forget
     * to initialize it!
     *
     * Warning! Here we suppose that compiler doesn't realign this structure!!!
     * Thus we can scan all words inside structure as pointers, there are no
     * alignment bytes there! */

    /* Wrong assumption !!! Compiler CAN realign structure!
    for(i=0; i<sizeof(*osFuncBindPtr); i+=4)
    {
        void **func_ptr=((unsigned char*)osFuncBindPtr) + i;
        if(*func_ptr == NULL)
        {
            osPrintf("Error: pointer into CPSS_OS_FUNC_BIND_STC struct is NULL (offset %d), sizeof(CPSS_OS_FUNC_BIND_STC)=%d\n",
                     i, sizeof(CPSS_OS_FUNC_BIND_STC));
            return GT_FAIL;
        }
    }
    */
    return GT_OK;
}

/**
* @internal cpssGetDefaultExtDrvFuncs function
* @endinternal
*
* @brief   Receives default cpss bind from extDrv
*
* @param[out] extDrvFuncBindInfoPtr    - (pointer to) set of call back functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Function should reside into CPSS library to resolve correct
*       pointers to functions.
*
*/
static GT_STATUS cpssGetDefaultExtDrvFuncs(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC  *extDrvFuncBindInfoPtr
)
{
    osMemSet(extDrvFuncBindInfoPtr,0,sizeof(*extDrvFuncBindInfoPtr));

    /* bind the external drivers functions to the CPSS */
    extDrvFuncBindInfoPtr->extDrvMgmtCacheBindInfo.extDrvMgmtCacheFlush      = FORCE_FUNC_CAST extDrvMgmtCacheFlush;
    extDrvFuncBindInfoPtr->extDrvMgmtCacheBindInfo.extDrvMgmtCacheInvalidate = FORCE_FUNC_CAST extDrvMgmtCacheInvalidate;

    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiInitDriverFunc      = hwIfSmiInitDriver;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiWriteRegFunc        = hwIfSmiWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiReadRegFunc         = hwIfSmiReadReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegRamReadFunc  = hwIfSmiTskRegRamRead;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegRamWriteFunc = hwIfSmiTskRegRamWrite;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegVecReadFunc  = hwIfSmiTskRegVecRead;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegVecWriteFunc = hwIfSmiTskRegVecWrite;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskWriteRegFunc    = hwIfSmiTaskWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskReadRegFunc     = hwIfSmiTaskReadReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiIntReadRegFunc      = hwIfSmiInterruptReadReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiIntWriteRegFunc     = hwIfSmiInterruptWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiDevVendorIdGetFunc  = extDrvSmiDevVendorIdGet;
/*  used only in linux -- will need to be under some kind of COMPILATION FLAG
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskWriteFieldFunc  = hwIfSmiTaskWriteRegField;
*/

#ifdef GT_I2C
    extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiInitDriverFunc = hwIfTwsiInitDriver;
    extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiWriteRegFunc   = hwIfTwsiWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiReadRegFunc    = hwIfTwsiReadReg;
#endif /* GT_I2C */


/*  XBAR related services */
#if defined(IMPL_FA) || defined(IMPL_XBAR)
    extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvI2cMgmtMasterInitFunc    = gtI2cMgmtMasterInit;
    extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtReadRegisterFunc     = FORCE_FUNC_CAST extDrvMgmtReadRegister;
    extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtWriteRegisterFunc    = FORCE_FUNC_CAST extDrvMgmtWriteRegister;
    extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtIsrReadRegisterFunc  = FORCE_FUNC_CAST extDrvMgmtIsrReadRegister;
    extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtIsrWriteRegisterFunc = FORCE_FUNC_CAST extDrvMgmtIsrWriteRegister;
#endif

    extDrvFuncBindInfoPtr->extDrvDmaBindInfo.extDrvDmaWriteDriverFunc = extDrvDmaWrite;
    extDrvFuncBindInfoPtr->extDrvDmaBindInfo.extDrvDmaReadFunc        = extDrvDmaRead;

    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortRxInitFunc            = extDrvEthPortRxInit;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxInitFunc            = extDrvEthPortTxInit;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortEnableFunc            = extDrvEthPortEnable;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortDisableFunc           = extDrvEthPortDisable;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxFunc                = extDrvEthPortTx;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortInputHookAddFunc      = extDrvEthInputHookAdd;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxCompleteHookAddFunc = extDrvEthTxCompleteHookAdd;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortRxPacketFreeFunc      = extDrvEthRxPacketFree;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxModeSetFunc         = FORCE_FUNC_CAST extDrvEthPortTxModeSet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketModeSetFunc  = extDrvEthRawSocketModeSet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketModeGetFunc  = extDrvEthRawSocketModeGet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvLinuxModeSetFunc  = extDrvLinuxModeSet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvLinuxModeGetFunc  = extDrvLinuxModeGet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketRxHookAddFunc = extDrvEthRawSocketRxHookAdd;

    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuMemBaseAddrGetFunc = extDrvHsuMemBaseAddrGet;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuWarmRestartFunc = extDrvHsuWarmRestart;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaEnableFunc = extDrvHsuInboundSdmaEnable;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaDisableFunc = extDrvHsuInboundSdmaDisable;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaStateGetFunc = extDrvHsuInboundSdmaStateGet;

#if defined (XCAT_DRV)
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthCpuCodeToQueueFunc        = extDrvEthCpuCodeToQueue;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPrePendTwoBytesHeaderSetFunc = extDrvEthPrePendTwoBytesHeaderSet;
#endif

    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntConnectFunc     = extDrvIntConnect;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntDisconnectFunc =  extDrvIntDisconnect;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntEnableFunc      = extDrvIntEnable;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntDisableFunc     = extDrvIntDisable;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntLockModeSetFunc = FORCE_FUNC_CAST extDrvSetIntLockUnlock;

    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciConfigWriteRegFunc        = extDrvPciConfigWriteReg;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciConfigReadRegFunc         = extDrvPciConfigReadReg;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDevFindFunc               = extDrvPciFindDev;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciIntVecFunc                = FORCE_FUNC_CAST extDrvGetPciIntVec;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciIntMaskFunc               = FORCE_FUNC_CAST extDrvGetIntMask;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciCombinedAccessEnableFunc  = extDrvEnableCombinedPciAccess;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDoubleWriteFunc           = extDrvPciDoubleWrite;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDoubleReadFunc            = extDrvPciDoubleRead;

#ifdef DRAGONITE_TYPE_A1
    extDrvFuncBindInfoPtr->extDrvDragoniteInfo.extDrvDragoniteShMemBaseAddrGetFunc = extDrvDragoniteShMemBaseAddrGet;
#endif

    return GT_OK;
}
#endif /* !defined(SHARED_MEMORY) */

#if !defined(SHARED_MEMORY) || defined(LINUX_SIM)

/*******************************************************************************
* appDemoPrintSynchronizedSet
*
* DESCRIPTION:
*       Set cpssOsPrintf synchronized or queued.
* INPUTS:
*       synchronized - GT_TRUE - synchronized, GT_FALSE - queued.
*       delay        - delay in miliseconds (when synchronized == GT_TRUE)
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - any case
*
* COMMENTS:
*
*******************************************************************************/
static GT_U32 synchronizedPrintDelay = 0;
#ifdef  LINUX
static char synchronizedPrintBuf[2048];
static int synchronizedPrintWithDelay(const char* format, ...)
{
    int rc;
    va_list args;
    va_start (args, format);
    osVsprintf(synchronizedPrintBuf, format, args);
    rc = osPrintSync(synchronizedPrintBuf);
    osTimerWkAfter(synchronizedPrintDelay);
    return rc;
}
#endif
GT_STATUS appDemoPrintSynchronizedSet(
    IN GT_BOOL   synchronized,
    IN GT_U32    delay
)
{
    synchronizedPrintDelay = delay;
    if (synchronized == GT_FALSE)
    {
        cpssOsPrintf = osPrintf;
    }
    else
    {
#ifdef  LINUX
        cpssOsPrintf = synchronizedPrintWithDelay;
#else
        cpssOsPrintf = osPrintSync;
#endif
    }
    return GT_OK;
}

/**
* @internal cpssGetDefaultTraceFuncs function
* @endinternal
*
* @brief   Receives default cpss bind from trace
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Function should reside into CPSS library to resolve correct
*       pointers to functions.
*
*/
static GT_STATUS cpssGetDefaultTraceFuncs(
    OUT CPSS_TRACE_FUNC_BIND_STC  *traceFuncBindInfoPtr
)
{
    osMemSet(traceFuncBindInfoPtr,0,sizeof(*traceFuncBindInfoPtr));

    /* bind the external drivers functions to the CPSS */
    traceFuncBindInfoPtr->traceHwBindInfo.traceHwAccessReadFunc  = appDemoTraceReadHwAccess;
    traceFuncBindInfoPtr->traceHwBindInfo.traceHwAccessWriteFunc = appDemoTraceWriteHwAccess;
    traceFuncBindInfoPtr->traceHwBindInfo.traceHwAccessDelayFunc = appDemoTraceDelayHwAccess;

    return GT_OK;
}

#endif
/**
* @internal appDemoCpssInit function
* @endinternal
*
* @brief   This function call CPSS to do initial initialization.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note This function must be called before phase 1 init.
*
*/
GT_STATUS appDemoCpssInit
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    CPSS_EXT_DRV_FUNC_BIND_STC  extDrvFuncBindInfo;
    CPSS_OS_FUNC_BIND_STC       osFuncBind;
    CPSS_TRACE_FUNC_BIND_STC    traceFuncBindInfo;

    if (appDemoCpssInitialized == GT_TRUE)
    {
        return GT_OK;
    }

#if defined(SHARED_MEMORY)
    /* Shared memory approach on ARM should use dlsym-based workaround for pointers to functions */
    rc = shrMemGetDefaultOsBindFuncsThruDynamicLoader(&osFuncBind);
    osFuncBind.osLogBindInfo.osLogFunc = appDemoOsLog;
#else
    rc = cpssGetDefaultOsBindFuncs(&osFuncBind);
#endif

    if(rc != GT_OK) return rc;

#if defined(SHARED_MEMORY)
    /* Shared memory approach on ARM should use dlsym-based workaround for pointers to functions */
    rc = shrMemGetDefaultExtDrvFuncs(&extDrvFuncBindInfo);
#else
    rc = cpssGetDefaultExtDrvFuncs(&extDrvFuncBindInfo);
#endif


#if defined(SHARED_MEMORY) && (!defined(LINUX_SIM))
    /* Shared memory approach on ARM should use dlsym-based workaround for pointers to functions */

    /* Shared memory for Trace should be implemented later */
    CPSS_TBD_BOOKMARK
    /* use NULL pointer to avoid trash stored by cpssExtServicesBind.
       The cpssExtServicesBind will use default "not implemented" callbacks in this case.  */
    osMemSet(&traceFuncBindInfo, 0, sizeof(traceFuncBindInfo));
#else
    rc = cpssGetDefaultTraceFuncs(&traceFuncBindInfo);
#endif

    if(rc != GT_OK) return rc;

    /* 1'st step */
    rc = cpssExtServicesBind(&extDrvFuncBindInfo, &osFuncBind, &traceFuncBindInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* 2'nd step */
    if (isHirApp==GT_FALSE)
    {
         rc = cpssPpInit();
         if(rc != GT_OK)
         {
             return rc;
         }

    }

#if defined(CPSS_LOG_ENABLE)
{
    char                        buffer[80];
    const char                * prvCpssLogErrorLogFileName;

#if defined  WIN32
    prvCpssLogErrorLogFileName = "c:\\temp\\cpss_log_entry";
#else
    prvCpssLogErrorLogFileName = "/tmp/cpss_log_entry";
#endif
    /* Create unique history file name from constant string and current time*/
    cpssOsSprintf(buffer, "%s_%d", prvCpssLogErrorLogFileName, osTime());
    /* Set log history file name */
    rc = cpssLogHistoryFileNameSet(buffer);
    if(rc != GT_OK)
    {
        return rc;
    }
}
#endif

#ifdef  IMPL_GALTIS
#ifdef SHARED_MEMORY
    enhUtUseCaptureToCpuPtr = (GT_U32*)osMalloc(sizeof(GT_U32)); /* allocated in shared area */
#endif
#ifdef IMPL_TGF
    tgfCmdCpssNetEnhUtUseCaptureToCpuPtr = enhUtUseCaptureToCpuPtr;
#endif /* IMPL_TGF */
#endif /* IMPL_GALTIS */

#ifdef IMPL_TGF
#ifdef CHX_FAMILY
    appDemoDxChTcamPclConvertedIndexGetPtr = appDemoDxChTcamPclConvertedIndexGet_fromUT;
#endif
    appDemoDbEntryGet_func = appDemoDbEntryGet;
#endif

    appDemoCpssInitialized = GT_TRUE;

    return GT_OK;
} /* appDemoCpssInit */



/**
* @internal appDemoBoardPhase1Init function
* @endinternal
*
* @brief   Perform phase1 initialization for all devices (Pp, Fa, Xbar).
*
* @param[in] boardRevId               - Board revision Id.
* @param[in] boardCfgFuncs            - Board configuration functions.
*                                      numOfPp         - Number of Pp's in system.
* @param[in] numOfFa                  - Number of Fa's in system.
* @param[in] numOfXbar                - Number of Xbar's in system.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS appDemoBoardPhase1Init
(
    IN  GT_U8                   boardRevId,       /* Revision of this board             */
    IN  GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs,   /* Config functions for this board    */
    IN  GT_U8                   numOfDev,         /* Number of devices in this board    */
    IN  GT_U8                   numOfFa,          /* Number of FAs in this board        */
    IN  GT_U8                   numOfXbar         /* Number of xbars in this board      */
)
{
    CPSS_PP_PHASE1_INIT_PARAMS      corePpPhase1Params;     /* Phase 1 PP params                            */
    CPSS_REG_VALUE_INFO_STC         *regCfgList;            /* Config functions for this board              */
    GT_U32                          regCfgListSize;         /* Number of config functions for this board    */
    GT_STATUS                       rc;                     /* Func. return val.                            */
    GT_U8                           devIdx;                 /* Loop index.                                  */
    CPSS_PP_DEVICE_TYPE             ppDevType;              /* CPSS Pp device type.                         */

#ifdef IMPL_XBARDRIVER
    GT_XBAR_CORE_SYS_CFG            xbarSysCfg;
    GT_XBAR_DEVICE                  xbarDevType;            /* Xbar device type.*/
    GT_XBAR_PHASE1_INIT_PARAMS      coreXbarPhase1Params;
#endif

#ifdef IMPL_FA
    GT_U8                           deviceNumber;           /* Real device number: FA or PP                 */
    GT_FA_PHASE1_INIT_PARAMS        coreFaPhase1Params;
    GT_FA_DEVICE                    faDevType;              /* Fa device type.  */
#endif
#ifdef IMPL_PP
    GT_U32                          value = 0;/*value from the DB*/
#endif /*IMPL_PP*/

    /* Set xbar configuration parameters.           */
#ifdef IMPL_XBARDRIVER
    if((boardCfgFuncs->boardGetXbarCfgParams != NULL) &&
       (numOfXbar != 0))
    {
        rc = boardCfgFuncs->boardGetXbarCfgParams((GT_U8)boardRevId, &xbarSysCfg);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetXbarCfgParams", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* CPSS works only in event driven mode */
        xbarSysCfg.eventRequestDriven = GT_TRUE;

        /* Initialize xbar core & driver dbases */
        rc = xbarSysConfig(&xbarSysCfg);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("xbarSysConfig", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
#endif

#ifdef IMPL_PP

    if((appDemoDbEntryGet("DualDeviceIdMode", &value) == GT_OK) && value)
    {
        /* We force the system to work with dual device IDs only */
        /* this option needed for checking the behavior of such system when the
           Lion2 device is not the tested device ! */

        /* at this stage we are after cpssPpInit(...) and before cpssDxChHwPpPhase1Init(...) */
        rc = cpssSystemDualDeviceIdModeEnableSet(GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* Set Pp Phase1 configuration parameters.      */
    for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < SYSTEM_DEV_NUM_MAC(numOfDev); devIdx++)
    {
        APP_DEMO_SYS_CONFIG_FUNCS*  sysCfgFuncs;

        /* Get PP config of this device */
        rc = boardCfgFuncs->boardGetPpPh1Params((GT_U8)boardRevId,
                                                devIdx,
                                                &corePpPhase1Params);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetPpPh1Params", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        value = 0;
        if ((appDemoDbEntryGet("dontOverrideSip5DefaultPciChannelType", &value) != GT_OK)
            || (value == 0))
        {
            /* Override SIP5 devices Default PCI compatible bus to use */
            /* 8 Address Completion Region mode                        */
            if ((appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) ||
                (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
                (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
                (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E) ||
                (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E))
            {
                corePpPhase1Params.mngInterfaceType = CPSS_CHANNEL_PEX_MBUS_E;
                /* Address Completion Region 1 - for Interrupt Handling    */
                corePpPhase1Params.isrAddrCompletionRegionsBmp = 0x02;
                /* Address Completion Regions 2,3,4,5 - for Application    */
                corePpPhase1Params.appAddrCompletionRegionsBmp = 0x3C;
                /* Address Completion Regions 6,7 - reserved for other CPU */
            }
        }

        /* override PP phase 1 parameters according to app demo database */
        rc = appDemoUpdatePpPhase1Params(&corePpPhase1Params);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoUpdatePpPhase1Params", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Update PP config list device number */
        appDemoPpConfigList[devIdx].devNum   = corePpPhase1Params.devNum;
        appDemoPpConfigList[devIdx].valid    = GT_TRUE;

        /* get family type to understand which API should be used below */
        /* There are two type of API here: EX and DX                    */
        rc = appDemoSysConfigFuncsGet(corePpPhase1Params.deviceId,
                                      &appDemoPpConfigList[devIdx].sysConfigFuncs,
                                      &appDemoPpConfigList[devIdx].apiSupportedBmp);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSysConfigFuncsGet", rc);
        if (GT_OK != rc)
        {
            return rc;
        }


        if(appDemoSysConfigFuncsExtentionGetPtr)
        {
            rc = appDemoSysConfigFuncsExtentionGetPtr(corePpPhase1Params.deviceId,
                                      &appDemoPpConfigList[devIdx].sysConfigFuncs,
                                      &appDemoPpConfigList[devIdx].apiSupportedBmp);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSysConfigFuncsExtentionGetPtr", rc);
            if (GT_OK != rc)
            {
                return rc;
            }
        }

        sysCfgFuncs = &appDemoPpConfigList[devIdx].sysConfigFuncs;

#ifdef ASIC_SIMULATION
        if(sasicgSimulationRole == SASICG_SIMULATION_ROLE_DISTRIBUTED_APPLICATION_SIDE_VIA_BROKER_E)
        {
            /* bind interrupts when work via broker */
            simDistributedRegisterInterrupt(corePpPhase1Params.hwInfo[0].irq.switching,
                    corePpPhase1Params.devNum,/* use the device ID as the signal ID */
                    sdistAppViaBrokerInterruptMaskMode);
        }
#endif /*ASIC_SIMULATION*/

        /* check if debug device id was set */
        if (useDebugDeviceId == GT_TRUE &&
            deviceIdDebug[appDemoPpConfigList[devIdx].devNum] != 0)
        {
            /* write device id to internal DB */
            rc = prvCpssDrvDebugDeviceIdSet(appDemoPpConfigList[devIdx].devNum,
                                            deviceIdDebug[appDemoPpConfigList[devIdx].devNum]);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDrvDebugDeviceIdSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (appDemoPrePhase1Init == GT_TRUE)
        {
            rc = prvCpssPrePhase1PpInit(PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_NO_PP_INIT_E);
            if (rc != GT_OK)
            {
                return rc;
            }

        }
        /* Do HW phase 1 */
        rc = sysCfgFuncs->cpssHwPpPhase1Init(appDemoPpConfigList[devIdx].devNum,
                                             &corePpPhase1Params,
                                             &ppDevType);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssHwPpPhase1Init", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Update PP config list element of device */
        appDemoPpConfigList[devIdx].ppPhase1Done = GT_TRUE;
        appDemoPpConfigList[devIdx].deviceId = ppDevType;
        appDemoPpConfigList[devIdx].maxPortNumber = PRV_CPSS_PP_MAC(appDemoPpConfigList[devIdx].devNum)->numOfPorts;


        if(PRV_CPSS_PP_MAC(appDemoPpConfigList[devIdx].devNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E &&
           PRV_CPSS_PP_MAC(appDemoPpConfigList[devIdx].devNum)->revision > 0 )
        {
            /*Puma3, starting revision B0*/
            appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_TRUE;
        }
        else if (PRV_CPSS_SIP_5_CHECK_MAC(appDemoPpConfigList[devIdx].devNum))
        {
            appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_TRUE;
        }
#ifdef ASIC_SIMULATION
        else    /* allow simulation to test it , without HW implications ... yet */
        if(PRV_CPSS_PP_MAC(appDemoPpConfigList[devIdx].devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_TRUE;
        }
        else
        if(PRV_CPSS_PP_MAC(appDemoPpConfigList[devIdx].devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_TRUE;
        }
#endif /*ASIC_SIMULATION*/
        else
        {
            appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_FALSE;
        }

        #ifdef GM_USED
        /* the GM not supports the 'soft reset' so we can not support system with it */
        appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_FALSE;
        #endif /*GM_USED*/


        /* If app demo inits in reg defaults mode skip the rest of the loop */
        if (appDemoInitRegDefaults == GT_TRUE)
            continue;

        /* Get list of registers to be configured.  */
        if (boardCfgFuncs->boardGetPpRegCfgList == NULL)
        {
            /* if there is no board-specific function, call the common one */
            rc = appDemoGetPpRegCfgList(ppDevType,
                                        gIsB2bSystem,
                                        &regCfgList,
                                        &regCfgListSize);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoGetPpRegCfgList", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            rc = boardCfgFuncs->boardGetPpRegCfgList(boardRevId,
                                                     appDemoPpConfigList[devIdx].devNum,
                                                     &regCfgList,
                                                     &regCfgListSize);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetPpRegCfgList", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (appDemoPrePhase1Init == GT_FALSE)
        {
            if(sysCfgFuncs->cpssHwPpStartInit)
            {
                /* Set PP's registers */
                rc = sysCfgFuncs->cpssHwPpStartInit(appDemoPpConfigList[devIdx].devNum,
                                                    gReloadEeprom,
                                                    regCfgList,
                                                    regCfgListSize);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssHwPpStartInit", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }/* for (Loop on devices) */
#endif /* IMPL_PP */

    /* If app demo inits in reg defaults mode return */
    if (appDemoInitRegDefaults == GT_TRUE)
        return GT_OK;


    /* Set Fa Phase1 configuration parameters.      */
#ifdef IMPL_FA
    if(boardCfgFuncs->boardGetFaPh1Params != NULL)
    {
        /* Assume num of PP, num of FA and num of XBAR are the same */
        for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < SYSTEM_DEV_NUM_MAC(numOfFa); devIdx++)
        {
            /* Get FA config of this device */
            rc = boardCfgFuncs->boardGetFaPh1Params((GT_U8)boardRevId,
                                                    devIdx,
                                                    &coreFaPhase1Params);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetFaPh1Params", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* Phase 1 init & registration of FA */

            rc = coreFaHwPhase1Init(&coreFaPhase1Params, &faDevType);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("coreFaHwPhase1Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Set Fa Start-Init configuration parameters.  */
            /* Get list of registers to be configured.  */
            if (boardCfgFuncs->boardGetFaRegCfgList == NULL)
            {
                /* if there is no board-specific function, call the common one */
                rc = appDemoGetFaRegCfgList(faDevType,
#ifdef IMPL_PP
                                            appDemoPpConfigList[devIdx].deviceId,
#else
                                            APP_DEMO_CPSS_NO_PP_CONNECTED_CNS,/*0xFFFFFFFF*/
#endif
                                            &regCfgList,
                                            &regCfgListSize);

                CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoGetFaRegCfgList", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                rc = boardCfgFuncs->boardGetFaRegCfgList(boardRevId,
                                                         appDemoPpConfigList[devIdx].devNum,
                                                         &regCfgList,
                                                         &regCfgListSize);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetFaRegCfgList", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            /* if NO PP used device number is FA device number */
            deviceNumber = (numOfDev > 0) ? appDemoPpConfigList[devIdx].devNum : devIdx;


            /* Set FA's registers */
            rc = coreFaHwStartInit(deviceNumber,
                                   GT_FALSE,
                                   (GT_REG_VALUE_INFO*)regCfgList,
                                   regCfgListSize);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("coreFaHwStartInit", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }/* Loop on FAs */
    }/* FA exists */
#endif /*IMPL_FA*/

#ifdef IMPL_XBARDRIVER
    /* Set xbar core Phase1 configuration parameters.    */
    if(boardCfgFuncs->boardGetXbarPh1Params != NULL)
    {
        /* Assume num of PP, num of FA and num of XBAR are the same */
        for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < SYSTEM_DEV_NUM_MAC(numOfXbar); devIdx++)
        {
            rc = boardCfgFuncs->boardGetXbarPh1Params((GT_U8)boardRevId,
                                                      devIdx,
                                                      &coreXbarPhase1Params);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetXbarPh1Params", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = coreXbarHwPhase1Init(&coreXbarPhase1Params, &xbarDevType);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("coreXbarHwPhase1Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }/* Loop on XBARs */
    } /* Xbar exists */
#endif /*IMPL_XBARDRIVER*/

    return GT_OK;
} /* appDemoBoardPhase1Init */

/**
* @internal appDemoBoardPhase2Init function
* @endinternal
*
* @brief   Perform phase2 initialization for all devices (Pp, Fa, Xbar).
*
* @param[in] boardRevId               - Board revision Id.
* @param[in] boardCfgFuncs            - Board configuration functions
* @param[in] numOfDev                 - Number of devices in devList
* @param[in] numOfFa                  - Number of Fa's in system
* @param[in] numOfXbar                - Number of Xbar's in system
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS appDemoBoardPhase2Init
(
    IN  GT_U8                   boardRevId,     /* Board revision Id             */
    IN  GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs, /* Board configuration functions */
    IN  GT_U8                   numOfDev,       /* Number of devices in devList  */
    IN  GT_U8                   numOfFa,        /* Number of Fa's in system      */
    IN  GT_U8                   numOfXbar       /* Number of Xbar's in system    */
)
{
    CPSS_PP_PHASE2_INIT_PARAMS          cpssPpPhase2Params;     /* PP phase 2 params                        */
    GT_STATUS                           rc;                     /* Func. return val.                        */
    GT_U8                               devIdx;                 /* Loop index.                              */


#ifdef IMPL_XBARDRIVER
    GT_XBAR_PHASE2_INIT_PARAMS          coreXbarPhase2Params;   /* XBAR phase 2 params */
#endif

#ifdef IMPL_FA
    GT_U8                               deviceNumber;           /* Real device number: FA or PP                 */
    GT_FA_PHASE2_INIT_PARAMS            coreFaPhase2Params;     /* FA phase 2 params */
#endif

#ifdef IMPL_PP
    /* Set Pp Phase2 configuration parameters */
    for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < SYSTEM_DEV_NUM_MAC(numOfDev); devIdx++)
    {
        APP_DEMO_SYS_CONFIG_FUNCS*  sysCfgFuncs;

        /* save value for use in appDemoAllocateDmaMem(...) */
        appDemoCpssCurrentDevIndex = devIdx;
        /* Get PP phase 2 params */
        rc = boardCfgFuncs->boardGetPpPh2Params(boardRevId,
                                                appDemoPpConfigList[devIdx].devNum,
                                                &cpssPpPhase2Params);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetPpPh2Params", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* override PP phase 2 parameters according to app demo database */
        rc = appDemoUpdatePpPhase2Params(&cpssPpPhase2Params);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoUpdatePpPhase2Params", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(appDemoOnDistributedSimAsicSide)
        {
            /* no need to init the external driver */
            appDemoSysConfig.cpuEtherInfo.initFunc = NULL;
            /* we must not init the HW */
            cpssPpPhase2Params.netIfCfg.txDescBlock = NULL;
            cpssPpPhase2Params.netIfCfg.rxDescBlock = NULL;
            cpssPpPhase2Params.auqCfg.auDescBlock   = NULL;
        }

        appDemoPpConfigList[devIdx].oldDevNum = appDemoPpConfigList[devIdx].devNum;
        sysCfgFuncs = &appDemoPpConfigList[devIdx].sysConfigFuncs;

        if(sysCfgFuncs->cpssHwPpPhase2Init)
        {

            /* PP HW phase 2 Init */
            rc = sysCfgFuncs->cpssHwPpPhase2Init(appDemoPpConfigList[devIdx].oldDevNum,
                                                 &cpssPpPhase2Params);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssHwPpPhase2Init", rc);

            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* Current PP init completed */
        appDemoPpConfigList[devIdx].ppPhase2Done = GT_TRUE;
    }
#endif /* IMPL_PP */
    /* Set Fa Phase2 configuration parameters.      */
#ifdef IMPL_FA
    if(boardCfgFuncs->boardGetFaPh2Params != NULL)
    {
        /* Assume num of PP, num of FA and num of XBAR are the same */
        for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < SYSTEM_DEV_NUM_MAC(numOfFa); devIdx++)
        {
            /* if NO PP used device number is FA device number */
            deviceNumber = (numOfDev > 0) ? appDemoPpConfigList[devIdx].devNum : devIdx;

            rc = boardCfgFuncs->boardGetFaPh2Params((GT_U8)boardRevId,
                                                    deviceNumber,
                                                    &coreFaPhase2Params);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetFaPh2Params", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            coreFaPhase2Params.devNum = deviceNumber;

            /* Init FA */
            rc = coreFaHwPhase2Init(&coreFaPhase2Params);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("coreFaHwPhase2Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
#endif /*IMPL_FA*/

    /* Set xbar Phase2 configuration parameters.    */
#ifdef IMPL_XBARDRIVER
    if(boardCfgFuncs->boardGetXbarPh2Params != NULL)
    {
        /* Assume num of PP, num of FA and num of XBAR are the same */
        for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < SYSTEM_DEV_NUM_MAC(numOfXbar); devIdx++)
        {
            /* Get XBAR phase 2 params */
            rc = boardCfgFuncs->boardGetXbarPh2Params((GT_U8)boardRevId,
                                                      appDemoPpConfigList[devIdx].devNum,
                                                      &coreXbarPhase2Params);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetXbarPh2Params", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            coreXbarPhase2Params.devNum = appDemoPpConfigList[devIdx].devNum;

            /* Init Xbar */
            rc = coreXbarHwPhase2Init(&coreXbarPhase2Params);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("coreXbarHwPhase2Init", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        } /* Loop on XBARs */
    } /* Xbar exists */
#endif /*IMPL_XBARDRIVER*/

    return GT_OK;
} /* appDemoBoardPhase2Init */

/**
* @internal appDemoBoardLogicalInit function
* @endinternal
*
* @brief   Perform logical phase initialization for all devices (Pp, Fa, Xbar).
*
* @param[in] boardRevId               - Board revision Id.
* @param[in] boardCfgFuncs            - Board configuration functions
* @param[in] numOfDev                 - Number of devices in devList
* @param[in] numOfFa                  - Number of Fa's in system
* @param[in] numOfXbar                - Number of Xbar's in system
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS appDemoBoardLogicalInit
(
    IN  GT_U8                   boardRevId,
    IN  GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs,
    IN  GT_U8                   numOfDev,
    IN  GT_U8                   numOfFa,
    IN  GT_U8                   numOfXbar
)
{
    GT_STATUS                   rc;
    CPSS_PP_CONFIG_INIT_STC     ppLogicalConfigParams;
    GT_U8                       devIdx;


    /* Set Pp Logical configuration parameters */
    for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < SYSTEM_DEV_NUM_MAC(numOfDev); devIdx++)
    {
        APP_DEMO_SYS_CONFIG_FUNCS*  sysCfgFuncs;

        /* update device config list */
        sysCfgFuncs = &appDemoPpConfigList[devIdx].sysConfigFuncs;


        /* Get PP logical init configuration */
        rc = boardCfgFuncs->boardGetPpLogInitParams(boardRevId,
                                                    appDemoPpConfigList[devIdx].devNum,
                                                    &ppLogicalConfigParams);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetPpLogInitParams", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* override logical init configuration according to app demo database */
        rc = appDemoUpdatePpLogicalInitParams(&ppLogicalConfigParams);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoUpdatePpLogicalInitParams", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Do CPSS logical init and fill PP_INFO structure */
        rc = sysCfgFuncs->cpssPpLogicalInit(appDemoPpConfigList[devIdx].devNum,
                                            &ppLogicalConfigParams);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssPpLogicalInit", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
        appDemoPpConfigList[devIdx].ppLogicalInitDone = GT_TRUE;
        osMemCpy(&ppUserLogicalConfigParams[appDemoPpConfigList[devIdx].devNum],
                 &ppLogicalConfigParams,
                 sizeof(CPSS_PP_CONFIG_INIT_STC));
    }

    return GT_OK;

} /* appDemoBoardLogicalInit */

/**
* @internal appDemoBoardGeneralInit function
* @endinternal
*
* @brief   Perform general initialization for all devices (Pp, Fa, Xbar).
*         This function includes initializations that common for all devices.
* @param[in] boardRevId               - Board revision Id.
* @param[in] boardCfgFuncs            - Board configuration functions.
*                                      numOfPp         - Number of Pp's in system.
* @param[in] numOfFa                  - Number of Fa's in system.
* @param[in] numOfXbar                - Number of Xbar's in system.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This function must be called after logical init.
*
*/
static GT_STATUS appDemoBoardGeneralInit
(
    IN  GT_U8                   boardRevId,       /* Revision of this board             */
    IN  GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs,   /* Config functions for this board    */
    IN  GT_U8                   numOfDev,         /* Number of devices in this board    */
    IN  GT_U8                   numOfFa,          /* Number of FAs in this board        */
    IN  GT_U8                   numOfXbar         /* Number of xbars in this board      */
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U8                       devIdx;
    APP_DEMO_LIB_INIT_PARAMS    libInitParams;
    CPSS_PP_CONFIG_INIT_STC     ppLogicalConfigParams;


    /* Do general configuration for all devices */
    for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < SYSTEM_DEV_NUM_MAC(numOfDev); devIdx++)
    {
        APP_DEMO_SYS_CONFIG_FUNCS*  sysCfgFuncs;
        GT_U8                       dev;

        /* get init parameters from appdemo init array */
        sysCfgFuncs = &appDemoPpConfigList[devIdx].sysConfigFuncs;
        dev = appDemoPpConfigList[devIdx].devNum;

        /* Get PP logical init configuration */
        rc = boardCfgFuncs->boardGetPpLogInitParams(boardRevId,
                                                    appDemoPpConfigList[devIdx].devNum,
                                                    &ppLogicalConfigParams);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetPpLogInitParams", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* override logical init configuration according to app demo database */
        rc = appDemoUpdatePpLogicalInitParams(&ppLogicalConfigParams);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoUpdatePpLogicalInitParams", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Get library initialization parameters */
        rc = boardCfgFuncs->boardGetLibInitParams(boardRevId,
                                                  appDemoPpConfigList[devIdx].devNum,
                                                  &libInitParams);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetLibInitParams", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* override library initialization parameters according to app demo database */
        rc = appDemoUpdateLibInitParams(&libInitParams);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoUpdateLibInitParams", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Initialize CPSS libraries accordingly to a given parameters */
        if(sysCfgFuncs->cpssLibrariesInit)
        {
            rc = sysCfgFuncs->cpssLibrariesInit(appDemoPpConfigList[devIdx].devNum,
                                                &libInitParams,
                                                &ppLogicalConfigParams);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssLibrariesInit", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if(sysCfgFuncs->cpssPpGeneralInit)
        {
            /* Do CPSS general initialization for given device id */
            rc = sysCfgFuncs->cpssPpGeneralInit(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssPpGeneralInit", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        appDemoPpConfigList[devIdx].ppGeneralInitDone = GT_TRUE;
    }
    return GT_OK;
} /* appDemoBoardGeneralInit */


/**
* @internal appDemoUpdatePpPhase1Params function
* @endinternal
*
* @brief   Updates PP phase 1 params according to app demo database.
*
* @param[in,out] paramsPtr                - points to params to be updated
* @param[in,out] paramsPtr                - points to params after update
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS appDemoUpdatePpPhase1Params
(
    INOUT CPSS_PP_PHASE1_INIT_PARAMS    *paramsPtr
)
{
    GT_U32  value;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    /* follow this sample code to override a parameter according        */
    /* to appDemo database entries                                      */
    /* please keep the convention that database entry name is the       */
    /* same as parameter to update                                      */
    /* if(appDemoDbEntryGet("[databse entry name]", &value) == GT_OK)   */
    /*    paramsPtr->[parameter to update] = value;                     */


    if(appDemoDbEntryGet("intVecNum", &value) == GT_OK)
    {
        paramsPtr->hwInfo[0].irq.switching = value;
    }

    /* allow this case also from 'specific board config file' (not only from appDemoDbEntryGet(...))*/
    if(paramsPtr->hwInfo[0].irq.switching == CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS/*0xFFFFFFFF*/)
    {
        appDemoBypassEventInitialize = GT_TRUE;
    }

    /* serdes reference clock */
    if(appDemoDbEntryGet("serdesRefClock", &value) == GT_OK)
    {
        switch(value)
        {
        case 1:
            paramsPtr->serdesRefClock = APPDEMO_SERDES_REF_CLOCK_EXTERNAL_125_SINGLE_ENDED_E;
            break;
        case 2:
            paramsPtr->serdesRefClock = APPDEMO_SERDES_REF_CLOCK_EXTERNAL_125_DIFF_E;
            break;
        case 5: paramsPtr->serdesRefClock = APPDEMO_SERDES_REF_CLOCK_INTERNAL_125_E;
            break;
        default:
            return GT_FAIL;
        }
    }

    /* TCAM Parity Daemon */
    if(appDemoDbEntryGet("tcamParityCalcEnable", &value) == GT_OK)
    {
        if(value == 0)
        {
            paramsPtr->tcamParityCalcEnable = GT_FALSE;
        }
        else
        {
            paramsPtr->tcamParityCalcEnable = GT_TRUE;
        }
    }

    /*PCI/PEX/PEX_MBUS alteration options*/
    switch (paramsPtr->mngInterfaceType)
    {
        case CPSS_CHANNEL_PCI_E:
        case CPSS_CHANNEL_PEX_E:
        case CPSS_CHANNEL_PEX_MBUS_E:
            if (appDemoDbEntryGet("mngInterfaceType", &value) == GT_OK)
            {
                switch (value)
                {
                    case CPSS_CHANNEL_PCI_E:
                    case CPSS_CHANNEL_PEX_E:
                    case CPSS_CHANNEL_PEX_MBUS_E:
                        paramsPtr->mngInterfaceType = value;
                        break;
                    default: break;
                }
            }
            break;
        default: break;
    }

    if(appDemoDbEntryGet("isrAddrCompletionRegionsBmp", &value) == GT_OK)
    {
        paramsPtr->isrAddrCompletionRegionsBmp = value;
    }

    if(appDemoDbEntryGet("appAddrCompletionRegionsBmp", &value) == GT_OK)
    {
        paramsPtr->appAddrCompletionRegionsBmp = value;
    }

    return GT_OK;
}

/**
* @internal appDemoUpdatePpPhase2Params function
* @endinternal
*
* @brief   Updates PP phase 2 params according to app demo database.
*
* @param[in,out] paramsPtr                - points to params to be updated
* @param[in,out] paramsPtr                - points to params after update
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS appDemoUpdatePpPhase2Params
(
    INOUT CPSS_PP_PHASE2_INIT_PARAMS    *paramsPtr
)
{
    GT_U32  value;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    /* follow this sample code to override a parameter according        */
    /* to appDemo database entries                                      */
    /* please keep the convention that database entry name is the       */
    /* same as parameter to update                                      */
    /* if(appDemoDbEntryGet("[databse entry name]", &value) == GT_OK)   */
    /*    paramsPtr->[parameter to update] = value;                     */

    if(appDemoDbEntryGet("noCpu", &value) == GT_OK)
    {
        paramsPtr->noTraffic2CPU = (GT_BOOL)value;
    }
    if(appDemoDbEntryGet("netifSdmaPortGroupId", &value) == GT_OK)
    {
        paramsPtr->netifSdmaPortGroupId = value;
    }

    return GT_OK;
}

/**
* @internal appDemoUpdatePpLogicalInitParams function
* @endinternal
*
* @brief   Updates PP logical init params according to app demo database.
*
* @param[in,out] paramsPtr                - points to params to be updated
* @param[in,out] paramsPtr                - points to params after update
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on wrong value
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS appDemoUpdatePpLogicalInitParams
(
    INOUT CPSS_PP_CONFIG_INIT_STC       *paramsPtr
)
{
    GT_U32  value;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    /* follow this sample code to override a parameter according        */
    /* to appDemo database entries                                      */
    /* please keep the convention that database entry name is the       */
    /* same as parameter to update                                      */
    /* if(appDemoDbEntryGet("[databse entry name]", &value) == GT_OK)   */
    /*    paramsPtr->[parameter to update] = value;                     */

    /* Inlif */
    if(appDemoDbEntryGet("maxNumOfLifs", &value) == GT_OK)
        paramsPtr->maxNumOfLifs = value;
    if(appDemoDbEntryGet("inlifPortMode", &value) == GT_OK)
        paramsPtr->inlifPortMode = (APP_DEMO_CPSS_INLIF_PORT_MODE_ENT)value;

    /* Policer */
    if(appDemoDbEntryGet("maxNumOfPolicerEntries", &value) == GT_OK)
        paramsPtr->maxNumOfPolicerEntries = value;
    if(appDemoDbEntryGet("policerConformCountEn", &value) == GT_OK)
        paramsPtr->policerConformCountEn = (GT_BOOL)value;

    /* MPLS    */
    if(appDemoDbEntryGet("maxNumOfNhlfe", &value) == GT_OK)
        paramsPtr->maxNumOfNhlfe = value;
    if(appDemoDbEntryGet("maxNumOfMplsIfs", &value) == GT_OK)
        paramsPtr->maxNumOfMplsIfs = value;
    if(appDemoDbEntryGet("ipMplsMemLocation", &value) == GT_OK)
    {
        switch(value)
        {
        case 0:
            paramsPtr->ipMplsMemLocation = APP_DEMO_CPSS_MEMORY_LOCATION_INTERNAL_E;
            break;
        case 1:
            paramsPtr->ipMplsMemLocation = APP_DEMO_CPSS_MEMORY_LOCATION_EXTERNAL_E;
            break;
        default:
            return GT_BAD_PARAM;
        }
    }

    /* PCL     */
    if(appDemoDbEntryGet("maxNumOfPclAction", &value) == GT_OK)
        paramsPtr->maxNumOfPclAction = value;
    if(appDemoDbEntryGet("pclActionSize", &value) == GT_OK)
        paramsPtr->pclActionSize = value;

    /* Policy-based routing  */
    if(appDemoDbEntryGet("maxNumOfPceForIpPrefixes", &value) == GT_OK)
        paramsPtr->maxNumOfPceForIpPrefixes = value;
    if(appDemoDbEntryGet("usePolicyBasedRouting", &value) == GT_OK)
        paramsPtr->usePolicyBasedRouting = value;
    if(appDemoDbEntryGet("usePolicyBasedDefaultMc", &value) == GT_OK)
        paramsPtr->usePolicyBasedDefaultMc = value;

    /* LPM PBR entries */
    if(appDemoDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        paramsPtr->maxNumOfPbrEntries = value;

    /* Bridge           */
    if(appDemoDbEntryGet("maxVid", &value) == GT_OK)
        paramsPtr->maxVid = (GT_U16)value;
    if(appDemoDbEntryGet("maxMcGroups", &value) == GT_OK)
        paramsPtr->maxMcGroups = (GT_U16)value;

    /* IP Unicast     */
    if(appDemoDbEntryGet("maxNumOfVirtualRouters", &value) == GT_OK)
        paramsPtr->maxNumOfVirtualRouters = value;
    if(appDemoDbEntryGet("maxNumOfIpNextHop", &value) == GT_OK)
        paramsPtr->maxNumOfIpNextHop = value;

    /* IPv4 Unicast     */
    if(appDemoDbEntryGet("maxNumOfIpv4Prefixes", &value) == GT_OK)
        paramsPtr->maxNumOfIpv4Prefixes = value;

    /* IPv4/IPv6 Multicast   */
    if(appDemoDbEntryGet("maxNumOfIpv4McEntries", &value) == GT_OK)
        paramsPtr->maxNumOfIpv4McEntries = value;
    if(appDemoDbEntryGet("maxNumOfMll", &value) == GT_OK)
        paramsPtr->maxNumOfMll = value;

    /* IPv6 Unicast     */
    if(appDemoDbEntryGet("maxNumOfIpv6Prefixes", &value) == GT_OK)
        paramsPtr->maxNumOfIpv6Prefixes = value;

    /* IPv6 Multicast   */
    if(appDemoDbEntryGet("maxNumOfIpv6McGroups", &value) == GT_OK)
        paramsPtr->maxNumOfIpv6McGroups = value;

    /* Tunnels */
    if(appDemoDbEntryGet("maxNumOfTunnelEntries", &value) == GT_OK)
        paramsPtr->maxNumOfTunnelEntries = value;
    if(appDemoDbEntryGet("maxNumOfIpv4TunnelTerms", &value) == GT_OK)
        paramsPtr->maxNumOfIpv4TunnelTerms = value;

    /* tunnel termination (of all types) / TTI entries - for Dx devices */
    if(appDemoDbEntryGet("maxNumOfTunnelTerm", &value) == GT_OK)
        paramsPtr->maxNumOfTunnelTerm = value;

    /* inlifs */
    if(appDemoDbEntryGet("vlanInlifEntryType", &value) == GT_OK)
        paramsPtr->vlanInlifEntryType = value;

    /* if GT_FALSE - no division of memory pools for the use of different IP versions */
    if(appDemoDbEntryGet("ipMemDivisionOn", &value) == GT_OK)
        paramsPtr->ipMemDivisionOn = (GT_BOOL)value;

    /* devision of memory between IPv4 and IPv6 prefixes*/
    if(appDemoDbEntryGet("ipv6MemShare", &value) == GT_OK)
        paramsPtr->ipv6MemShare = value;

    /* Number of trunks */
    if(appDemoDbEntryGet("numOfTrunks", &value) == GT_OK)
        paramsPtr->numOfTrunks = value;

    /* PCL rule index and PCL ID for defualt IPv6 MC entry for devices */
    /* where IPv6 MC group search is implemented in PCL                */
    if(appDemoDbEntryGet("defIpv6McPclRuleIndex", &value) == GT_OK)
        paramsPtr->defIpv6McPclRuleIndex = value;
    if(appDemoDbEntryGet("vrIpv6McPclId", &value) == GT_OK)
        paramsPtr->vrIpv6McPclId = value;

    /* TCAM LPM DB configurations */
    if(appDemoDbEntryGet("lpmDbPartitionEnable", &value) == GT_OK)
        paramsPtr->lpmDbPartitionEnable = (GT_BOOL)value;
    if(appDemoDbEntryGet("lpmDbFirstTcamLine", &value) == GT_OK)
        paramsPtr->lpmDbFirstTcamLine = value;
    if(appDemoDbEntryGet("lpmDbLastTcamLine", &value) == GT_OK)
        paramsPtr->lpmDbLastTcamLine = value;
    if(appDemoDbEntryGet("lpmDbSupportIpv4", &value) == GT_OK)
        paramsPtr->lpmDbSupportIpv4 = (GT_BOOL)value;
    if(appDemoDbEntryGet("lpmDbSupportIpv6", &value) == GT_OK)
        paramsPtr->lpmDbSupportIpv6 = (GT_BOOL)value;

    /* number of memory configurations supported by the lpm db */
    if(appDemoDbEntryGet("lpmDbNumOfMemCfg", &value) == GT_OK)
        paramsPtr->lpmDbNumOfMemCfg = value;

#ifdef CHX_FAMILY
    /* LPM RAM Configuration for Dynamic LPM Management */
    if(appDemoDbEntryGet("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", &value) == GT_OK)
        paramsPtr->lpmRamMemoryBlocksCfg.blocksAllocationMethod = (CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT)value;
#endif /*CHX_FAMILY*/

    /* array of memory configurations supported by the lpm db */
    if(appDemoDbEntryGet("lpmDbMemCfgArray->routingSramCfgType", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->routingSramCfgType = (APP_DEMO_CPSS_LPM_MEM_CFG_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->sramsSizeArray[0]", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->sramsSizeArray[0] = (CPSS_SRAM_SIZE_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->sramsSizeArray[1]", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->sramsSizeArray[1] = (CPSS_SRAM_SIZE_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->sramsSizeArray[2]", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->sramsSizeArray[2] = (CPSS_SRAM_SIZE_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->sramsSizeArray[2]", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->sramsSizeArray[2] = (CPSS_SRAM_SIZE_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->sramsSizeArray[3]", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->sramsSizeArray[3] = (CPSS_SRAM_SIZE_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->sramsSizeArray[4]", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->sramsSizeArray[4] = (CPSS_SRAM_SIZE_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->sramsSizeArray[5]", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->sramsSizeArray[5] = (CPSS_SRAM_SIZE_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->sramsSizeArray[6]", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->sramsSizeArray[6] = (CPSS_SRAM_SIZE_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->sramsSizeArray[7]", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->sramsSizeArray[7] = (CPSS_SRAM_SIZE_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->sramsSizeArray[8]", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->sramsSizeArray[8] = (CPSS_SRAM_SIZE_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->sramsSizeArray[9]", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->sramsSizeArray[9] = (CPSS_SRAM_SIZE_ENT)value;
    if(appDemoDbEntryGet("lpmDbMemCfgArray->numOfSramsSizes", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->numOfSramsSizes = value;

    if(appDemoDbEntryGet("lpmInternalSramSizeArray_elem0", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->lpmInternalSramSizeArray[0] = value;
    if(appDemoDbEntryGet("lpmInternalSramSizeArray_elem1", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->lpmInternalSramSizeArray[1] = value;
    if(appDemoDbEntryGet("lpmInternalSramSizeArray_elem2", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->lpmInternalSramSizeArray[2] = value;
    if(appDemoDbEntryGet("lpmInternalSramSizeArray_elem3", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->lpmInternalSramSizeArray[3] = value;

    if(appDemoDbEntryGet("lpmDbMemCfgArray->extMemBankMode", &value) == GT_OK)
    {
        switch (value)
        {
        case 0:
            paramsPtr->lpmDbMemCfgArray->extMemBankMode = APPDEMO_EXMXPM_EXT_MEMORY_LOGICAL_MODE_8BANKS_E;
            break;
        case 1:
            paramsPtr->lpmDbMemCfgArray->extMemBankMode = APPDEMO_EXMXPM_EXT_MEMORY_LOGICAL_MODE_4BANKS_E;
            break;
        default:
            return GT_BAD_PARAM;
        }
    }
    if(appDemoDbEntryGet("lpmDbMemCfgArray->extMemSize", &value) == GT_OK)
        paramsPtr->lpmDbMemCfgArray->extMemSize = value;

    if(appDemoDbEntryGet("extLpmMemSize", &value) == GT_OK)
        paramsPtr->extLpmMemSize = value;

    if(appDemoDbEntryGet("lpmStagesMemAssociation", &value) == GT_OK)
        paramsPtr->lpmStagesMemAssociation = (APP_DEMO_CPSS_LPM_STAGES_MEM_ASSOCIATION_ENT)value;

    /* DIT configuration */
    if(appDemoDbEntryGet("ditMemLocation", &value) == GT_OK)
    {
        switch(value)
        {
        case 0:
            paramsPtr->ditMemLocation = APP_DEMO_CPSS_MEMORY_LOCATION_INTERNAL_E;
            break;
        case 1:
            paramsPtr->ditMemLocation = APP_DEMO_CPSS_MEMORY_LOCATION_EXTERNAL_E;
            break;
        default:
            return GT_BAD_PARAM;
        }
    }

    if(appDemoDbEntryGet("ditMemoryMode", &value) == GT_OK)
        paramsPtr->ditMemoryMode = (APP_DEMO_CPSS_DIT_MEMORY_MODE_ENT)value;
    if(appDemoDbEntryGet("ditIpMcMode", &value) == GT_OK)
        paramsPtr->ditIpMcMode = (APP_DEMO_CPSS_DIT_IP_MC_MODE_ENT)value;
    if(appDemoDbEntryGet("maxNumOfDitUcIpEntries", &value) == GT_OK)
        paramsPtr->maxNumOfDitUcIpEntries = value;
    if(appDemoDbEntryGet("maxNumOfDitUcMplsEntries", &value) == GT_OK)
        paramsPtr->maxNumOfDitUcMplsEntries = value;
    if(appDemoDbEntryGet("maxNumOfDitUcOtherEntries", &value) == GT_OK)
        paramsPtr->maxNumOfDitUcOtherEntries = value;
    if(appDemoDbEntryGet("maxNumOfDitMcIpEntries", &value) == GT_OK)
        paramsPtr->maxNumOfDitMcIpEntries = value;
    if(appDemoDbEntryGet("maxNumOfDitMcMplsEntries", &value) == GT_OK)
        paramsPtr->maxNumOfDitMcMplsEntries = value;
    if(appDemoDbEntryGet("maxNumOfDitMcOtherEntries", &value) == GT_OK)
        paramsPtr->maxNumOfDitMcOtherEntries = value;
    if(appDemoDbEntryGet("pha_packetOrderChangeEnable", &value) == GT_OK)
        paramsPtr->pha_packetOrderChangeEnable = (GT_BOOL)value;

    /* note that external TCAM info parameters can't be modified */

    return GT_OK;
}

/**
* @internal appDemoUpdateLibInitParams function
* @endinternal
*
* @brief   Updates lib init params according to app demo database.
*
* @param[in,out] paramsPtr                - points to params to be updated
* @param[in,out] paramsPtr                - points to params after update
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS appDemoUpdateLibInitParams
(
    INOUT APP_DEMO_LIB_INIT_PARAMS      *paramsPtr
)
{
    GT_U32  value;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    /* follow this sample code to override a parameter according        */
    /* to appDemo database entries                                      */
    /* please keep the convention that database entry name is the       */
    /* same as parameter to update                                      */
    /* if(appDemoDbEntryGet("[databse entry name]", &value) == GT_OK)   */
    /*    paramsPtr->[parameter to update] = value;                     */

    if(appDemoDbEntryGet("initBridge", &value) == GT_OK)
        paramsPtr->initBridge = (GT_BOOL)value;
    if(appDemoDbEntryGet("initClassifier", &value) == GT_OK)
        paramsPtr->initClassifier = (GT_BOOL)value;
    if(appDemoDbEntryGet("initCos", &value) == GT_OK)
        paramsPtr->initCos = (GT_BOOL)value;
    if(appDemoDbEntryGet("initIpv4", &value) == GT_OK)
        paramsPtr->initIpv4 = (GT_BOOL)value;
    if(appDemoDbEntryGet("initIpv6", &value) == GT_OK)
        paramsPtr->initIpv6 = (GT_BOOL)value;
    if(appDemoDbEntryGet("initIpv4Filter", &value) == GT_OK)
        paramsPtr->initIpv4Filter = (GT_BOOL)value;
    if(appDemoDbEntryGet("initIpv4Tunnel", &value) == GT_OK)
        paramsPtr->initIpv4Tunnel = (GT_BOOL)value;
    if(appDemoDbEntryGet("initLif", &value) == GT_OK)
        paramsPtr->initLif = (GT_BOOL)value;
    if(appDemoDbEntryGet("initMirror", &value) == GT_OK)
        paramsPtr->initMirror = (GT_BOOL)value;
    if(appDemoDbEntryGet("initMpls", &value) == GT_OK)
        paramsPtr->initMpls = (GT_BOOL)value;
    if(appDemoDbEntryGet("initMplsTunnel", &value) == GT_OK)
        paramsPtr->initMplsTunnel = (GT_BOOL)value;
    if(appDemoDbEntryGet("initNetworkIf", &value) == GT_OK)
        paramsPtr->initNetworkIf = (GT_BOOL)value;
    if(appDemoDbEntryGet("initI2c", &value) == GT_OK)
        paramsPtr->initI2c = (GT_BOOL)value;
    if(appDemoDbEntryGet("initPhy", &value) == GT_OK)
        paramsPtr->initPhy = (GT_BOOL)value;
    if(appDemoDbEntryGet("initPort", &value) == GT_OK)
        paramsPtr->initPort = (GT_BOOL)value;
    if(appDemoDbEntryGet("initTc", &value) == GT_OK)
        paramsPtr->initTc = (GT_BOOL)value;
    if(appDemoDbEntryGet("initTrunk", &value) == GT_OK)
        paramsPtr->initTrunk = (GT_BOOL)value;
    if(appDemoDbEntryGet("initPcl", &value) == GT_OK)
        paramsPtr->initPcl = (GT_BOOL)value;
    if(appDemoDbEntryGet("initTcam", &value) == GT_OK)
        paramsPtr->initTcam = (GT_BOOL)value;
    if(appDemoDbEntryGet("initPolicer", &value) == GT_OK)
        paramsPtr->initPolicer = (GT_BOOL)value;
    if(appDemoDbEntryGet("initAction", &value) == GT_OK)
        paramsPtr->initAction = (GT_BOOL)value;
    if(appDemoDbEntryGet("initSflow", &value) == GT_OK)
        paramsPtr->initSflow = (GT_BOOL)value;
    if(appDemoDbEntryGet("initPha", &value) == GT_OK)
        paramsPtr->initPha = (GT_BOOL)value;

    return GT_OK;
}

/**
* @internal appDemoOnDistributedSimAsicSideSet function
* @endinternal
*
* @brief   set value to the appDemoOnDistributedSimAsicSide flag.
*
* @param[in] value                    -  to be set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - if called after initialization of system -- after cpssInitSystem
*
* @note Function is called from the TERMINAL
*
*/
GT_STATUS appDemoOnDistributedSimAsicSideSet
(
    IN GT_U32  value
)
{
    if(systemInitialized == GT_TRUE)
    {
        /* will not have effect if called after initialization of system */
        osPrintf("appDemoOnDistributedSimAsicSideSet : should be called before cpssInitSystem \n");

        return GT_BAD_STATE;
    }

    /* when appDemoOnDistributedSimAsicSide != 0
       this image is running on simulation:
       1. our application on distributed "Asic side"
       2. the Other application that run on the "application side" is in
          charge about all Asic configurations
       3. our application responsible to initialize the cpss DB.
       4. the main motivation is to allow "Galtis wrappers" to be called on
          this side !
          (because the Other side application may have difficult running
           "Galtis wrappers" from there)
    */
    appDemoOnDistributedSimAsicSide = value;

    return GT_OK;
}

#ifdef ASIC_SIMULATION
/**
* @internal appDemoRtosOnSimulationInit function
* @endinternal
*
* @brief   initialize the Simulation section on the "target board" part of the
*         "RTOS on simulation" .
*
* @note Function is called as first step of the cpssInitSystem
*
*/
void appDemoRtosOnSimulationInit
(
    void
)
{
    SIM_OS_FUNC_BIND_STC simOsBindInfo;
    static int initDone = 0;

    if(initDone)
    {
        /* allow to call this before cpssInitSystem */
        return;
    }

    /* Must be called before use any of OS functions. */
    osWrapperOpen(NULL);

    /**********************************************/
    /* bind the simulation with extra OS services */
    /**********************************************/
    {
        /* reset all fields of simOsBindInfo */
        osMemSet(&simOsBindInfo,0,sizeof(simOsBindInfo));

        /* let the simulation OS bind it's own functions to the simulation */
        simOsFuncBindOwnSimOs();

        BIND_FUNC(osWrapperOpen);

        BIND_LEVEL_FUNC(sockets,osSocketTcpCreate);
        BIND_LEVEL_FUNC(sockets,osSocketUdpCreate);
        BIND_LEVEL_FUNC(sockets,osSocketTcpDestroy);
        BIND_LEVEL_FUNC(sockets,osSocketUdpDestroy);
        BIND_LEVEL_FUNC(sockets,osSocketCreateAddr);
        BIND_LEVEL_FUNC(sockets,osSocketDestroyAddr);
        BIND_LEVEL_FUNC(sockets,osSocketBind);
        BIND_LEVEL_FUNC(sockets,osSocketListen);
        BIND_LEVEL_FUNC(sockets,osSocketAccept);
        BIND_LEVEL_FUNC(sockets,osSocketConnect);
        BIND_LEVEL_FUNC(sockets,osSocketSetNonBlock);
        BIND_LEVEL_FUNC(sockets,osSocketSetBlock);
        BIND_LEVEL_FUNC(sockets,osSocketSend);
        BIND_LEVEL_FUNC(sockets,osSocketSendTo);
        BIND_LEVEL_FUNC(sockets,osSocketRecv);
        BIND_LEVEL_FUNC(sockets,osSocketRecvFrom);
        BIND_LEVEL_FUNC(sockets,osSocketSetSocketNoLinger);
        BIND_LEVEL_FUNC(sockets,osSocketExtractIpAddrFromSocketAddr);
        BIND_LEVEL_FUNC(sockets,osSocketGetSocketAddrSize);
        BIND_LEVEL_FUNC(sockets,osSocketShutDown);

        BIND_LEVEL_FUNC(sockets,osSelectCreateSet);
        BIND_LEVEL_FUNC(sockets,osSelectEraseSet);
        BIND_LEVEL_FUNC(sockets,osSelectZeroSet);
        BIND_LEVEL_FUNC(sockets,osSelectAddFdToSet);
        BIND_LEVEL_FUNC(sockets,osSelectClearFdFromSet);
        BIND_LEVEL_FUNC(sockets,osSelectIsFdSet);
        BIND_LEVEL_FUNC(sockets,osSelectCopySet);
        BIND_LEVEL_FUNC(sockets,osSelect);
        BIND_LEVEL_FUNC(sockets,osSocketGetSocketFdSetSize);

        /* this needed for binding the OS of simulation with our OS functions */
        simOsFuncBind(&simOsBindInfo);
    }

    /************************************************/
    /* init the kernel of simulation -- the PPs,FAs */
    /************************************************/
    simulationLibInit();

    /* state that init was done */
    initDone = 1;

    return;
}
#endif /*ASIC_SIMULATION*/


/**
* @internal appDemoCpuEtherPortUsed function
* @endinternal
*
* @brief   is CPU Ethernet port used (and not SDMA)
*
* @retval GT_TRUE                  - if CPU Ethernet port used (and not SDMA)
* @retval GT_FALSE                 - otherwise
*/
static GT_BOOL appDemoCpuEtherPortUsed(void)
{
    return appDemoSysConfig.cpuEtherPortUsed;
}

/**
* @internal appDemoEventBind function
* @endinternal
*
* @brief   This routine binds a user process to unified event. The routine returns
*         a handle that is used when the application wants to wait for the event
*         (cpssEventSelect), receive the event(cpssEventRecv) or transmit a packet
*         using the Network Interface.
*         NOTE : the function does not mask/unmask the HW events in any device.
*         This is Application responsibility to unmask the relevant events
*         on the needed devices , using function cpssEventDeviceMaskSet
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] uniEventArr[]            - The unified event list.
* @param[in] arrLength                - The unified event list length.
*
* @param[out] hndlPtr                  - (pointer to) The user handle for the bounded events.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_FULL                  - when trying to set the "tx buffer queue unify event"
*                                       (CPSS_PP_TX_BUFFER_QUEUE_E) with other events in the same
*                                       handler
* @retval GT_ALREADY_EXIST         - one of the unified events already bound to another
*                                       handler (not applicable to CPSS_PP_TX_BUFFER_QUEUE_E)
*/
static GT_STATUS appDemoEventBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT uniEventArr[],
    IN  GT_U32               arrLength,
    OUT GT_UINTPTR           *hndlPtr
)
{
    return cpssEventBind(uniEventArr,arrLength,hndlPtr);
}

/**
* @internal appDemoEventSelect function
* @endinternal
*
* @brief   This function waiting for one of the events ,relate to the handler,
*         to happen , and gets a list of events (in array of bitmaps format) that
*         occurred .
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] hndl                     - The user handle for the bounded events.
* @param[in] timeoutPtr               - (pointer to) Wait timeout in milliseconds
*                                      NULL pointer means wait forever.
* @param[in] evBitmapArrLength        - The bitmap array length (in words).
*
* @param[out] evBitmapArr[]            - The bitmap array of the received events.
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - general failure
* @retval GT_TIMEOUT               - when the "time out" requested by the caller expired and no
*                                       event occurred during this period
* @retval GT_BAD_PARAM             - bad hndl parameter , the hndl parameter is not legal
*                                       (was not returned by cpssEventBind(...))
* @retval GT_BAD_PTR               - evBitmapArr parameter is NULL pointer
*                                       (and evBitmapArrLength != 0)
*/
static GT_STATUS appDemoEventSelect
(
    IN  GT_UINTPTR           hndl,
    IN  GT_U32              *timeoutPtr,
    OUT GT_U32              evBitmapArr[],
    IN  GT_U32              evBitmapArrLength
)
{
    return  cpssEventSelect(hndl,timeoutPtr,evBitmapArr,evBitmapArrLength);
}

/**
* @internal appDemoEventRecv function
* @endinternal
*
* @brief   This function gets general information about the selected unified event.
*         The function retrieve information about the device number that relate to
*         the event , and extra info about port number / priority queue that
*         relate to the event.
*         The function retrieve the info about the first occurrence of this event
*         in the queue.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] hndl                     - The user handle for the bounded events.
* @param[in] evCause                  - The specified unify event that info about it's first
*                                      waiting occurrence required.
*
* @param[out] evExtDataPtr             (pointer to)The additional date (port num / priority
*                                      queue number) the event was received upon.
* @param[out] evDevPtr                 - (pointer to)The device the event was received upon
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - general failure
* @retval GT_NO_MORE               - There is no more info to retrieve about the specified
*                                       event .
* @retval GT_BAD_PARAM             - bad hndl parameter ,
* @retval or hndl bound to CPSS_PP_TX_BUFFER_QUEUE_E --> not allowed
*                                       use dedicated "get tx ended info" function instead
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS appDemoEventRecv
(
    IN  GT_UINTPTR            hndl,
    IN  CPSS_UNI_EV_CAUSE_ENT evCause,
    OUT GT_U32              *evExtDataPtr,
    OUT GT_U8               *evDevPtr
)
{
    return cpssEventRecv(hndl,evCause,evExtDataPtr,evDevPtr);
}

/**
* @internal appDemoEventDeviceMaskSet function
* @endinternal
*
* @brief   This routine mask/unmasks an unified event on specific device.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] devNum                   - device number - PP/FA/Xbar device number -
*                                      depend on the uniEvent
*                                      if the uniEvent is in range of PP events , then devNum relate
*                                      to PP
*                                      if the uniEvent is in range of FA events , then devNum relate
*                                      to FA
*                                      if the uniEvent is in range of XBAR events , then devNum relate
*                                      to XBAR
* @param[in] uniEvent                 - The unified event.
* @param[in] operation                - the  : mask / unmask
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad unify event value or bad device number
*
* @note The OS interrupts and the ExtDrv interrupts are locked during the
*       operation of the function.
*
*/
static GT_STATUS appDemoEventDeviceMaskSet
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN CPSS_EVENT_MASK_SET_ENT  operation
)
{
    return cpssEventDeviceMaskSet(devNum,uniEvent,operation);
}


/**
* @internal appDemoPpConfigGet function
* @endinternal
*
* @brief   get pointer to the application's info about the PP
*
* @param[in] devIndex                 - index of the device in the Application DB (0..127)
*
* @param[out] ppConfigPtr              - the info about the PP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on devIndex > 127
*/
static GT_STATUS appDemoPpConfigGet
(
    IN GT_U8    devIndex,
    OUT CMD_APP_PP_CONFIG* ppConfigPtr
)
{
    if(ppConfigPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    if(devIndex >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_OUT_OF_RANGE;
    }

    ppConfigPtr->deviceId = appDemoPpConfigList[devIndex].deviceId;
    ppConfigPtr->devNum = appDemoPpConfigList[devIndex].devNum;
    ppConfigPtr->valid = appDemoPpConfigList[devIndex].valid;
    ppConfigPtr->wa.trunkToCpuPortNum = appDemoPpConfigList[devIndex].wa.trunkToCpuPortNum;

    return GT_OK;
}

/**
* @internal appDemoIsSystemInitialized function
* @endinternal
*
* @brief   is system initialized
*
* @retval GT_TRUE                  - system initialized
* @retval GT_FALSE                 - system NOT initialized
*/
static GT_BOOL appDemoIsSystemInitialized
(
    void
)
{
    return systemInitialized;
}

/**
* @internal prvAppDemoEventFatalErrorEnable function
* @endinternal
*
* @brief   Set fatal error handling type.
*
* @param[in] fatalErrorType           - fatal error handling type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong fatalErrorType
*/
static GT_STATUS prvAppDemoEventFatalErrorEnable
(
    IN GT_32 fatalErrorType
)
{
    APP_DEMO_FATAL_ERROR_TYPE  type = (APP_DEMO_FATAL_ERROR_TYPE) fatalErrorType;

    return appDemoEventFatalErrorEnable(type);
}


/**
* @internal appDemoPpConfigPrint function
* @endinternal
*
* @brief   Return system configuration that was set by user .
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS appDemoPpConfigPrint
(
    IN  GT_U8  devNum
)
{

#if (defined CHX_FAMILY)
       PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;


        osPrintf("\n");
        osPrintf("                  CPSS System configuration                \n");
        osPrintf(" __________________________________________________________\n");
        osPrintf("|          Config parameter            |        value      |\n");
        osPrintf("|__________________________________________________________|\n");
        osPrintf("|__________numOfTrunks_________________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].numOfTrunks);
        osPrintf("|__________maxNumOfVirtualRouters______|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfVirtualRouters);
        osPrintf("|__________maxNumOfIpNextHop___________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfIpNextHop);

        osPrintf("|__________maxNumOfMll_________________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfMll);
        osPrintf("|__________maxNumOfIpv4Prefixes________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfIpv4Prefixes);
        osPrintf("|__________maxNumOfIpv6Prefixes________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfIpv6Prefixes);

        osPrintf("|__________maxNumOfIpv4McEntries_______|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfIpv4McEntries);

        osPrintf("|__________maxNumOfPceForIpPrefixes____|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfPceForIpPrefixes);
        osPrintf("|__________usePolicyBasedRouting_______|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].usePolicyBasedRouting);
        osPrintf("|__________usePolicyBasedDefaultMc_____|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].usePolicyBasedDefaultMc);
        osPrintf("|__________maxNumOfPbrEntries__________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfPbrEntries);
        osPrintf("|__________maxNumOfTunnelEntries_______|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfTunnelEntries);
        osPrintf("|__________maxNumOfIpv4TunnelTerms_____|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfIpv4TunnelTerms);
        osPrintf("|__________lpmDbPartitionEnable________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].lpmDbPartitionEnable);
        osPrintf("|__________lpmDbSupportIpv4____________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].lpmDbSupportIpv4);
        osPrintf("|__________lpmDbSupportIpv6____________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].lpmDbSupportIpv6);
        osPrintf("|__________lpmDbFirstTcamLine__________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].lpmDbFirstTcamLine);
        osPrintf("|__________lpmDbLastTcamLine___________|________%-8d___|\n",
                 ppUserLogicalConfigParams[devNum].lpmDbLastTcamLine);

        osPrintf("|__________lpmRamBlocksAllocationMethod___|______%-8d___|\n",
                 ppUserLogicalConfigParams[devNum].lpmRamMemoryBlocksCfg.blocksAllocationMethod);

        osPrintf("|______________________________________________________|\n");
        osPrintf("|___________________other_params_______________________|\n");
        osPrintf("|______________________________________________________|\n");
        osPrintf("|__________maxMcGroups_________________|_______________|\n",
                    ppUserLogicalConfigParams[devNum].maxMcGroups);
        osPrintf("|__________maxVid______________________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxVid);
        osPrintf("|__________maxNumOfPclAction___________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfPclAction);
        osPrintf("|__________pclActionSize_______________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].pclActionSize);
        osPrintf("|__________policerConformCountEn_______|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].policerConformCountEn);
        osPrintf("|__________maxNumOfIpv6McGroups________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfIpv6McGroups);
        osPrintf("|__________maxNumOfTunnelTerm__________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].maxNumOfTunnelTerm);
        osPrintf("|__________vlanInlifEntryType__________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].vlanInlifEntryType);
        osPrintf("|__________ipMemDivisionOn_____________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].ipMemDivisionOn);
        osPrintf("|__________defIpv6McPclRuleIndex_______|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].defIpv6McPclRuleIndex);
        osPrintf("|__________vrIpv6McPclId_______________|________%-8d___|\n",
                    ppUserLogicalConfigParams[devNum].vrIpv6McPclId);
        osPrintf("|______________________________________________________|\n");
        osPrintf("|_________________some_table_sizes_____________________|\n");
        osPrintf("|______________________________________________________|\n");

        osPrintf("|__________amount of CNC blocks________|________%-8d___|\n",
                    fineTuningPtr->tableSize.cncBlocks);
        osPrintf("|__________entries num in  CNC block___|________%-8d___|\n",
                    fineTuningPtr->tableSize.cncBlockNumEntries);
        osPrintf("|__________bufferMemory________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.bufferMemory);
        osPrintf("|__________fdb_________________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.fdb);
        osPrintf("|__________mllPairs____________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.mllPairs);
        osPrintf("|__________egressPolicersNum___________|________%-8d___|\n",
                    fineTuningPtr->tableSize.egressPolicersNum);
        osPrintf("|__________policersNum_________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.policersNum);
        osPrintf("|__________policyTcamRaws______________|________%-8d___|\n",
                    fineTuningPtr->tableSize.policyTcamRaws);
        osPrintf("|__________router______________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.router);
        osPrintf("|__________tunnelTerm__________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.tunnelTerm);
        osPrintf("|__________tunnelStart_________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.tunnelStart);
        osPrintf("|__________routerArp___________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.routerArp);
        osPrintf("|__________routerNextHop_______________|________%-8d___|\n",
                    fineTuningPtr->tableSize.routerNextHop);
        osPrintf("|__________ecmpQos_____________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.ecmpQos);
        osPrintf("|__________lpmRam______________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.lpmRam);
        osPrintf("|__________stgNum______________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.stgNum);
        osPrintf("|__________transmitDescr_______________|________%-8d___|\n",
                    fineTuningPtr->tableSize.transmitDescr);
        osPrintf("|__________vidxNum_____________________|________%-8d___|\n",
                    fineTuningPtr->tableSize.vidxNum);

#endif  /* (defined CHX_FAMILY) */

        return GT_OK;

}




/**
* @internal applicationExtServicesBind function
* @endinternal
*
* @brief   the mainCmd calls this function , so the application (that implement
*         this function) will bind the mainCmd (and GaltisWrapper) with OS ,
*         external driver functions and other application functions
*
* @param[out] extDrvFuncBindInfoPtr    - (pointer to) set of external driver call back functions
* @param[out] osFuncBindPtr            - (pointer to) set of OS call back functions
* @param[out] osExtraFuncBindPtr       - (pointer to) set of extra OS call back functions (that CPSS not use)
* @param[out] extraFuncBindPtr         - (pointer to) set of extra call back functions (that CPSS not use) (non OS functions)
* @param[out] traceFuncBindPtr         - (pointer to) set of Trace call back functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note This function must be implemented by the Application !!!
*
*/
GT_STATUS   applicationExtServicesBind(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC   *extDrvFuncBindInfoPtr,
    OUT CPSS_OS_FUNC_BIND_STC        *osFuncBindPtr,
    OUT CMD_OS_FUNC_BIND_EXTRA_STC   *osExtraFuncBindPtr,
    OUT CMD_FUNC_BIND_EXTRA_STC      *extraFuncBindPtr,
    OUT CPSS_TRACE_FUNC_BIND_STC     *traceFuncBindPtr
)
{
    GT_STATUS   rc;


    if(extDrvFuncBindInfoPtr == NULL ||
       osFuncBindPtr == NULL ||
       osExtraFuncBindPtr == NULL ||
       extraFuncBindPtr == NULL ||
       traceFuncBindPtr == NULL)
    {
        return GT_BAD_PTR;
    }

#if defined(SHARED_MEMORY)
    /* Shared memory approach on ARM should use dlsym-based workaround for pointers to functions */
    rc = shrMemGetDefaultExtDrvFuncs(extDrvFuncBindInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = shrMemGetDefaultOsBindFuncsThruDynamicLoader(osFuncBindPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Shared memory for Trace should be implemented later */
    CPSS_TBD_BOOKMARK
#else
    rc = cpssGetDefaultExtDrvFuncs(extDrvFuncBindInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssGetDefaultOsBindFuncs(osFuncBindPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssGetDefaultTraceFuncs(traceFuncBindPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif

    osFuncBindPtr->osTaskBindInfo.osTaskCreateFunc = osTaskCreate;
    osFuncBindPtr->osTaskBindInfo.osTaskDeleteFunc = osTaskDelete;
/*    osFuncBindPtr->osTaskBindInfo.osTaskGetSelfFunc = osTaskGetSelf; */

    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketTcpCreate                       );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketTcpCreate                       );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketUdpCreate                       );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketTcpDestroy                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketUdpDestroy                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketCreateAddr                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketDestroyAddr                     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketBind                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketListen                          );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketAccept                          );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketConnect                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSetNonBlock                     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSetBlock                        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSend                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSendTo                          );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketRecv                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketRecvFrom                        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSetSocketNoLinger               );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketExtractIpAddrFromSocketAddr     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketGetSocketAddrSize               );
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketShutDown = (CPSS_SOCKET_SHUTDOWN_FUNC)osSocketShutDown;
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectCreateSet                       );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectEraseSet                        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectZeroSet                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectAddFdToSet                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectClearFdFromSet                  );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectIsFdSet                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectCopySet                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelect                                );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketGetSocketFdSetSize              );

    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osTasksBindInfo,osTaskGetSelf                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osTasksBindInfo,osSetTaskPrior                           );

    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolCreatePool        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolCreateDmaPool     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolReCreatePool      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolDeletePool        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolGetBuf            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolFreeBuf           );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolGetBufSize        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolExpandPool        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolGetBufFreeCnt     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolPrintStats        );

    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,cpuEthernetPortBindInfo,cmdIsCpuEtherPortUsed        ,appDemoCpuEtherPortUsed             );

    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,eventsBindInfo,cmdCpssEventBind         ,appDemoEventBind             );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,eventsBindInfo,cmdCpssEventSelect       ,appDemoEventSelect           );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,eventsBindInfo,cmdCpssEventRecv         ,appDemoEventRecv             );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,eventsBindInfo,cmdCpssEventDeviceMaskSet,appDemoEventDeviceMaskSet    );


    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDbEntryAdd ,appDemoDbEntryAdd );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDbEntryGet ,appDemoDbEntryGet );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDbDump     ,appDemoDbDump     );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppPpConfigGet,appDemoPpConfigGet);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdInitSystem    ,cpssInitSystem    );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdInitSystemGet ,cpssInitSystemGet );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdResetSystem   ,cpssResetSystem   );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdInitConfi     ,confi2InitSystem    );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppShowBoardsList         ,appDemoShowBoardsList    );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppIsSystemInitialized    ,appDemoIsSystemInitialized);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppPpConfigPrint,appDemoPpConfigPrint);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppAllowProcessingOfAuqMessages ,
                         appDemoAllowProcessingOfAuqMessages);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppTraceHwAccessEnable ,
                         prvAppDemoTraceHwAccessEnable);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppTraceHwAccessOutputModeSet ,
                         prvAppDemoTraceHwAccessOutputModeSet);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppStartHeapAllocCounter  ,osMemStartHeapAllocationCounter );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppGetHeapAllocCounter    ,osMemGetHeapAllocationCounter   );

    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppEventFatalErrorEnable ,
                         prvAppDemoEventFatalErrorEnable);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDemoEventsDataBaseGet ,
                         appDemoEventsDataBaseGet);


#if (defined CHX_FAMILY)
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdPortRateTableGet, util_appDemoDxChPortFWSRatesTableGet);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppBc2PortsConfig,bc2AppDemoPortsConfig);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppBobkPortsConfig,bobkAppDemoPortsConfig);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppXcat2StackPortsModeSpeedSet,gtAppDemoXcat2StackPortsModeSpeedSet);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppXcat2SfpPortEnable,gtAppDemoXcat2SfpPortEnable);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDxChHsuOldImagePreUpdatePreparation       ,appDemoDxChHsuOldImagePreUpdatePreparation       );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDxChHsuNewImageUpdateAndSynch             ,appDemoDxChHsuNewImageUpdateAndSynch             );

#endif

#if (defined PX_FAMILY)
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdPortRateTableGet, util_appDemoPxPortFWSRatesTableGet);
#endif
    return GT_OK;
}

/**
* @internal appDemoDbPpDump function
* @endinternal
*
* @brief   Dump App Demo DB
*
* @retval GT_OK                    - GT_OK ant case
*/
GT_STATUS appDemoDbPpDump(void)
{

    GT_U32 i, ii; /* iterator*/

    for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if(appDemoPpConfigList[i].valid)
        {
          osPrintf("Found device with devNum %d, device id 0x%8X, family %d, mng interface %d\n",
                   appDemoPpConfigList[i].devNum, appDemoPpConfigList[i].deviceId,
                   appDemoPpConfigList[i].devFamily, appDemoPpConfigList[i].channel);
          if(appDemoPpConfigList[i].channel==CPSS_CHANNEL_PCI_E ||
             appDemoPpConfigList[i].channel==CPSS_CHANNEL_PEX_E)
          {
              GT_PCI_INFO *pciInfo;
              if (appDemoPpConfigList[i].numOfPortGroups > 1)
              {
                  for (ii = 0; ii < appDemoPpConfigList[i].numOfPortGroups; ii++)
                  {
                      /* print PCI/PEX info */
                      pciInfo = &(appDemoPpConfigList[i].portGroupsInfo[ii].portGroupPciInfo);
                      osPrintf(" Port Group %d PCI/PEX addr %02x:%02x.%x\n", ii,
                                pciInfo->pciBusNum, pciInfo->pciIdSel, pciInfo->funcNo);
                  }
              }
              else
              {
                  pciInfo = &(appDemoPpConfigList[i].pciInfo);
                  osPrintf(" PCI/PEX addr %02x:%02x.%x\n",
                        pciInfo->pciBusNum, pciInfo->pciIdSel, pciInfo->funcNo);
              }

          }
          else if (appDemoPpConfigList[i].channel==CPSS_CHANNEL_SMI_E)
          {
              osPrintf("   SMI address %d\n",
                       appDemoPpConfigList[i].smiInfo.smiIdSel);
          }

          osPrintf(  "Init flags: ppPhase1Done %d, ppPhase2Done %d, ppLogicalInitDone %d, ppGeneralInitDone %d\n",
                     appDemoPpConfigList[i].ppPhase1Done, appDemoPpConfigList[i].ppPhase2Done,
                     appDemoPpConfigList[i].ppLogicalInitDone, appDemoPpConfigList[i].ppGeneralInitDone);

          osPrintf("   Interrupt line %d\n",
                   appDemoPpConfigList[i].intLine);

        }
    }


    return 0;
}


/**
* @internal appDemoDebugDeviceIdSet function
* @endinternal
*
* @brief   Set given device ID value for specific device.
*         Should be called before cpssInitSystem().
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on devIndex > 127
*/
GT_STATUS appDemoDebugDeviceIdSet
(
    IN GT_U8    devNum,
    IN GT_U32   deviceId
)
{
    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_OUT_OF_RANGE;
    }

    /* only 16 MSBits used by prvCpssDrvDebugDeviceIdSet(...) */
    osPrintf(" NOTE: only 16 MSBits [0x%4.4x] will be used by the 'cpssDriver' !! \n",
        (deviceId >> 16));

    deviceIdDebug[devNum] = deviceId;

    useDebugDeviceId = GT_TRUE;

    return GT_OK;
}

/**
* @internal appDemoDebugDeviceIdReset function
* @endinternal
*
* @brief   Reverts to HW device ID value for all devices.
*         Should be called before cpssInitSystem().
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDebugDeviceIdReset
(
    IN GT_VOID
)
{
    cpssOsMemSet(deviceIdDebug, 0, sizeof(deviceIdDebug));
    useDebugDeviceId = GT_FALSE;

    return GT_OK;
}

/**
* @internal appDemoDebugDeviceIdGet function
* @endinternal
*
* @brief   Get given device ID value for specific device.
*
* @param[out] deviceIdPtr              - pointer to device type
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - on not set debug device ID
* @retval GT_OUT_OF_RANGE          - on devIndex > 127
*/
GT_STATUS appDemoDebugDeviceIdGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *deviceIdPtr
)
{
    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_OUT_OF_RANGE;
    }

    if ((useDebugDeviceId == GT_FALSE) ||
        (0 == deviceIdDebug[devNum]))
    {
        return GT_NO_SUCH;
    }

    *deviceIdPtr = deviceIdDebug[devNum];

    return GT_OK;
}

/**
* @internal appDemoTracePrintHwData function
* @endinternal
*
* @brief   Print data of HW Trace.
*
* @param[in] addr                     - start address that the access was made to
* @param[in] length                   -  of the data that was written in words
* @param[in] dataPtr                  - (pointer to) data that was written
* @param[in] printFuncPtr             - (pointer to) print function
*                                       None.
*/
static GT_VOID appDemoTracePrintHwData
(
    IN GT_U32                   addr,
    IN GT_U32                   length,
    IN GT_U32                   *dataPtr,
    IN GT_U32                   mask,
    IN FUNCP_TRACE_PRINTF_DATA   printFuncPtr
)
{
    GT_U32  ii;

    printFuncPtr("ADDR  0x%08X", addr);
    printFuncPtr("  DATA  ");

    for (ii = 0; ii < length; ii++)
    {
        printFuncPtr("0x%08X", dataPtr[ii]);
        printFuncPtr(" ");

        /* the data is printed so, that 4 words are printed per line */
        if (ii % 4 == 3)
        {
            printFuncPtr("\n");
        }
    }

    if((length == 1) && (mask != 0xFFFFFFFF))
    {
        printFuncPtr(" MASK  0x%08X", mask);
        printFuncPtr("  ");
    }

}

/**
* @internal appDemoTraceStoreHwData function
* @endinternal
*
* @brief   Store data of HW Trace to HW Access DB.
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - ports group number
* @param[in] isrContext               - GT_TRUE: called from ISR context (cpssDrvPpHwIsrWrite,
*                                      cpssDrvPpHwIsrRead)
*                                      GT_FALSE: called from not ISR context.
* @param[in] addr                     - memory address space
* @param[in] addr                     - start address that the access was made to
* @param[in] length                   -  of the data that was written in words
* @param[in] dataPtr                  - (pointer to) data that was written
* @param[in] mask                     -  for data that was written
*                                      dbPtr       - (pointer to) read/write HW Access DB
* @param[in,out] infoDbPtr                - (pointer to) read/write HW Access Info
*                                       None.
*/
static GT_STATUS appDemoTraceStoreHwData
(
    IN GT_U8                                devNum,
    IN GT_U32                               portGroupId,
    IN GT_BOOL                              isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT         addrSpace,
    IN GT_U32                               addr,
    IN GT_U32                               length,
    IN GT_U32                               *dataPtr,
    IN GT_U32                               mask,
    IN APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT    accessType,
    INOUT APP_DEMO_CPSS_HW_ACCESS_DB_STC    **dbPtrPtr,
    INOUT APP_DEMO_CPSS_HW_ACCESS_INFO_STC  *infoDbPtr
)
{
    GT_U32  newDbSize;
    GT_U32  tmpAccessParamsBmp;
    GT_U32  ii;
    GT_U32  curDbIndex;

    /* check if we are in task context */
    if (isrContext == GT_FALSE)
    {
        /* task context, lock ISR */
        PRV_CPSS_INT_SCAN_LOCK();
    }

    /* check if there is enough space in DB */
    if ((infoDbPtr->maxSize - infoDbPtr->curSize) < length)
    {
        /*limit data base size to APP_DEMO_CPSS_HW_ACCESS_MAX_DB_STEP_COUNT -1  of incrementations */
        if((infoDbPtr->curSize + APP_DEMO_CPSS_HW_ACCESS_DB_STEP_SIZE_CNS) >(APP_DEMO_CPSS_HW_ACCESS_DB_STEP_SIZE_CNS*APP_DEMO_CPSS_HW_ACCESS_MAX_DB_STEP_COUNT))
        {

           if(infoDbPtr->corrupted == GT_FALSE)
           {
            osPrintf("Warning: The register access DB  has reached the limit . Current size %d (0x%x) . \n",infoDbPtr->curSize ,infoDbPtr->curSize);

            infoDbPtr->corrupted = GT_TRUE;
           }

            /* check if we are in task context */
            if (isrContext == GT_FALSE)
            {
                /* task context, unlock ISR */
                PRV_CPSS_INT_SCAN_UNLOCK();
            }

            return GT_OK;
        }

        /* reallocate the DB */
        newDbSize = infoDbPtr->curSize + APP_DEMO_CPSS_HW_ACCESS_DB_STEP_SIZE_CNS;

        *dbPtrPtr = cpssOsRealloc(*dbPtrPtr, (sizeof(APP_DEMO_CPSS_HW_ACCESS_DB_STC)* newDbSize));

        if (*dbPtrPtr == NULL)
        {
            /* check if we are in task context */
            if (isrContext == GT_FALSE)
            {
                /* task context, unlock ISR */
                PRV_CPSS_INT_SCAN_UNLOCK();
            }

            return GT_OUT_OF_CPU_MEM;
        }

        /* Update MAX size of allocated memory space */
        infoDbPtr->maxSize = newDbSize;
    }

    /* set device number */
    tmpAccessParamsBmp = devNum;
    /* set port group id */
    tmpAccessParamsBmp |= ((0xFF &portGroupId) << 8);
    /* set ISR Context */
    tmpAccessParamsBmp |= (BOOL2BIT_MAC(isrContext) << 16);
    /* set PP,PCI/PEX or Reset and Init Controller space */
    tmpAccessParamsBmp |= (addrSpace << 17);

    /* offset 26 is later on check again it's value, 0 means READ access
    1 means WRITE access. WRITE_DELAY should be marked as WRITE in order
    for later on to comply to condition "if (WRITE && address == 0)" */
   if(accessType == APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_E ||
      accessType == APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E)
   {
     /* update access type */
           tmpAccessParamsBmp |= (1 << 26);
   }

    /* set current DB index */
    curDbIndex = infoDbPtr->curSize;

    if (addr == 0)  /*delay,  save milisec as length*/
    {
        (*dbPtrPtr)[curDbIndex].addr = addr;
        (*dbPtrPtr)[curDbIndex].data = *dataPtr;
        (*dbPtrPtr)[curDbIndex].mask = mask;
        (*dbPtrPtr)[curDbIndex].accessParamsBmp = tmpAccessParamsBmp;
        /* move cursor to point to the next DB entry */
        infoDbPtr->curSize++;

    }
    else
    {
        if (length == 1)
        {
            /* the access is register access */

            (*dbPtrPtr)[curDbIndex].addr = addr;
            (*dbPtrPtr)[curDbIndex].data = *dataPtr;
            (*dbPtrPtr)[curDbIndex].mask = mask;
            (*dbPtrPtr)[curDbIndex].accessParamsBmp = tmpAccessParamsBmp;
            /* move cursor to point to the next DB entry */
            infoDbPtr->curSize++;
        }
        else
        {
            /* the access is memory access */
           tmpAccessParamsBmp |= (1 << 24);

           for (ii = 0; ii < length; ii++)
           {
               (*dbPtrPtr)[curDbIndex].addr = addr;
               (*dbPtrPtr)[curDbIndex].data = dataPtr[ii];
               (*dbPtrPtr)[curDbIndex].mask = 0xFFFFFFFF;
               (*dbPtrPtr)[curDbIndex].accessParamsBmp = tmpAccessParamsBmp;


               if (ii == (length - 1))
               {
                   /* Set the end bit of the memory (bit 25) */
                   (*dbPtrPtr)[curDbIndex].accessParamsBmp |= (1 << 25);
               }

               /* Increment Address */
               addr = addr + 4;
               /* move cursor to point to the next DB entry */
               curDbIndex++;
           }

           /* Update size of HW Access DB, the curSize points to the next DB entry */
           infoDbPtr->curSize = curDbIndex;
        }
    }


    /* check if we are in task context */
    if (isrContext == GT_FALSE)
    {
        /* task context, unlock ISR */
        PRV_CPSS_INT_SCAN_UNLOCK();
    }

    return GT_OK;
}

/* trace of ISR only */
GT_U32  appDemoTraceIsrOnly = 0;

/**
* @internal appDemoTraceIsrOnlySet function
* @endinternal
*
* @brief   Enable tracing of ISR only.
*
* @param[in] value                    - 0 - enable trace for all
*                                      1 - enable trace for ISR only
*                                       old state of ISR only trace.
*/
GT_U32 appDemoTraceIsrOnlySet
(
    IN GT_U32 value
)
{
    GT_U32 oldvalue = appDemoTraceIsrOnly; /* old value */

    appDemoTraceIsrOnly = value;
    return oldvalue;
}


/* indication to do sleep before print of register address */
static GT_BOOL appDemoTrace_do_millisecSleep = GT_FALSE;
/* the sleep in milli that needed for 'appDemoTrace_do_millisecSleep' */
static GT_U32  appDemoTrace_millisecSleep = 0;
/* the address that triggers the 'appDemoTrace_do_millisecSleep' */
static GT_U32  appDemoTrace_sleep_startAddr = 0;

/**
* @internal appDemoTraceHwAccessDoSlowPrint function
* @endinternal
*
* @brief   if the function appDemoTraceHwAccessAction get this address ,
*         it will start to 'sleep' before the print
*         it is needed when the 'dump' registers show a 'last print' but we not know
*         were the crash actually happen.
* @param[in] startAddr                - start address that need sleep
* @param[in] millisecSleep            - the sleep in millisec (value 0 meaning 'disable' the sleep)
*                                       None.
*/
GT_STATUS appDemoTraceHwAccessDoSlowPrint(
    IN GT_U32  startAddr,
    IN GT_U32  millisecSleep
)
{
    appDemoTrace_sleep_startAddr = startAddr;
    appDemoTrace_millisecSleep   = millisecSleep;

    /* reset the flag */
    appDemoTrace_do_millisecSleep = GT_FALSE;

    return GT_OK;
}

#define MAX_LIMITED_UNITS   16

static GT_BOOL  holdLimitedUnits = GT_FALSE;
static struct {
    GT_U32   baseAddressOfUnit; /* start base address of the unit */
    GT_U32   numOfBytesInUnit;  /* size of the unit (in bytes) - value 0 means 'remove' the unit  */
}limitToSpecificUnit[MAX_LIMITED_UNITS];

/* set the address as need to be skipped from the trace */
GT_STATUS appDemoTrace_limitToSpecificUnit(
    IN GT_U32   baseAddressOfUnit, /* start base address of the unit */
    IN GT_U32   numOfBytesInUnit,  /* size of the unit (in bytes) - value 0 means 'remove' the unit  */
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
)
{
    GT_U32  ii;

    if(index >= MAX_LIMITED_UNITS)
    {
        cpssOsPrintf("index[%d] must be less than [%d] \n",
            index,MAX_LIMITED_UNITS);
        return GT_BAD_PARAM;
    }

    limitToSpecificUnit[index].baseAddressOfUnit = baseAddressOfUnit;
    limitToSpecificUnit[index].numOfBytesInUnit  = numOfBytesInUnit;

    if(numOfBytesInUnit)
    {
        holdLimitedUnits = GT_TRUE;
    }
    else
    {
        /* check if we can set holdLimitedUnits = GT_FALSE */
        for(ii = 0 ; ii < MAX_LIMITED_UNITS; ii++)
        {
            if(limitToSpecificUnit[ii].numOfBytesInUnit)
            {
                /* at least one entry still need the support */
                break;
            }
        }

        if(ii == MAX_LIMITED_UNITS)
        {
            /* no entry need the units range */
            holdLimitedUnits = GT_FALSE;
        }
    }

    return GT_OK;
}

static GT_U32   isInLimitedUnit(
    IN GT_U32 address
)
{
    GT_U32  ii;

    for(ii = 0 ; ii < MAX_LIMITED_UNITS; ii++)
    {
        if(limitToSpecificUnit[ii].numOfBytesInUnit == 0)
        {
            continue;
        }

        if(address >= limitToSpecificUnit[ii].baseAddressOfUnit &&
           address <  limitToSpecificUnit[ii].baseAddressOfUnit + limitToSpecificUnit[ii].numOfBytesInUnit)
        {
            return 1;/* match the unit */
        }
    }

    return 0;
}

#define MAX_SKIP_ADDR   16
/* array of addresses that need to be skipped from the trace */
static GT_U32   skipTraceAddressArr[MAX_SKIP_ADDR];
/* set the address as need to be skipped from the trace */
GT_STATUS appDemoTrace_skipTrace_onAddress(
    IN GT_U32   address,
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
)
{
    if(index >= MAX_SKIP_ADDR)
    {
        cpssOsPrintf("index[%d] must be less than [%d] \n",
            index,MAX_SKIP_ADDR);
        return GT_BAD_PARAM;
    }

    skipTraceAddressArr[index] = address;

    return GT_OK;
}

/* check if the address need to be skipped from the trace */
static GT_BOOL is_skipTrace_onAddress(
    IN GT_U32 address
)
{
    GT_U32  ii;

    if(address == 0)
    {
        /* we not skip it */
        return GT_FALSE;
    }

    for(ii = 0 ; ii < MAX_SKIP_ADDR; ii++)
    {
        if(skipTraceAddressArr[ii] == address)
        {
            return GT_TRUE;
        }
    }

    if(holdLimitedUnits == GT_TRUE)
    {
        return isInLimitedUnit(address) ? GT_FALSE : GT_TRUE;
    }


    return GT_FALSE;
}


/**
* @internal appDemoTraceHwAccessAction function
* @endinternal
*
* @brief   Trace HW Access action: print or store.
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - ports group number
* @param[in] isrContext               - GT_TRUE: called from ISR context (cpssDrvPpHwIsrWrite,
*                                      cpssDrvPpHwIsrRead)
*                                      GT_FALSE: called from not ISR context.
* @param[in] addr                     - memory address space
* @param[in] addr                     - start address that the access was made to
* @param[in] length                   -  of the data that was written in words
* @param[in] dataPtr                  - (pointer to) data that was written
* @param[in] mask                     -  for data that was written
* @param[in] outputMode               - output mode: print, synchronious print and store
* @param[in] accessType               - access type: read or write
*                                       None.
*/
static GT_STATUS appDemoTraceHwAccessAction
(
    IN GT_U8                                devNum,
    IN GT_U32                               portGroupId,
    IN GT_BOOL                              isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT         addrSpace,
    IN GT_U32                               addr,
    IN GT_U32                               length,
    IN GT_U32                               *dataPtr,
    IN GT_U32                               mask,
    IN APP_DEMO_TRACE_OUTPUT_MODE_ENT       outputMode,
    IN APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT    accessType
)
{
    GT_STATUS                           rc;
    FUNCP_TRACE_PRINTF_DATA             printFuncPtr;

    #ifdef ASIC_SIMULATION
    FILE  *filePtr;
    GT_CHAR fileNameString[30] = {0};
    GT_CHAR *fileNamePtr;
    GT_CHAR buffer[APP_DEMO_FILE_MAX_LINE_LENGTH_CNS] = {0};
    GT_U32 i = 0;
    #endif
    if (appDemoTraceIsrOnly && (isrContext == GT_FALSE))
    {
        return GT_OK;
    }

    if(addrSpace == CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E &&
       is_skipTrace_onAddress(addr))
    {
        /* the address was registered as 'do not trace' it */
        return GT_OK;
    }

    /* check if we got to critical address that need to add sleep before the print */
    if(appDemoTrace_millisecSleep)
    {
        if(appDemoTrace_do_millisecSleep == GT_TRUE ||
           addr == appDemoTrace_sleep_startAddr )
        {
            appDemoTrace_do_millisecSleep = GT_TRUE;
            osTimerWkAfter(appDemoTrace_millisecSleep);
            osPrintf("..");
        }
    }

    /* switch output mode */
    switch (outputMode)
    {
        case APP_DEMO_TRACE_OUTPUT_MODE_DIRECT_E:
        case APP_DEMO_TRACE_OUTPUT_MODE_DIRECT_SYNC_E:
            /* print the data */

            printFuncPtr = (outputMode == APP_DEMO_TRACE_OUTPUT_MODE_DIRECT_E) ?
                osPrintf : osPrintSync;

            if ((accessType==APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E) || ((addr == 0) && (accessType == APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_E)))
            {
                printFuncPtr("DL");
                printFuncPtr("  ");
                printFuncPtr("%d", devNum);
                printFuncPtr("  ");
                printFuncPtr("%d", portGroupId);
                printFuncPtr(" ");
                printFuncPtr("\t\t\t\t%8d", *dataPtr);

            }
            else
            {
                if (accessType == APP_DEMO_TRACE_HW_ACCESS_TYPE_READ_E)
                {
                    printFuncPtr("RD");
                }
                else
                {
                    printFuncPtr("WR");
                }
                /* print devNum and portGroup */

                printFuncPtr("  ");
                printFuncPtr("%d", devNum);
                printFuncPtr("  ");
                printFuncPtr("%d", portGroupId);
                printFuncPtr("  ");

                if (isrContext)
                {
                    printFuncPtr("ISR");
                }
                else
                {
                    printFuncPtr("TSK");
                }
                printFuncPtr("  ");

                if (CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PCI_PEX_E == addrSpace)
                {
                    printFuncPtr("PEX");
                }
                else if (CPSS_TRACE_HW_ACCESS_ADDR_SPACE_RESET_AND_INIT_CTRL_E == addrSpace)
                {
                    printFuncPtr("DFX");
                }
                else if (CPSS_TRACE_HW_ACCESS_ADDR_SPACE_MG1_E == addrSpace)
                {
                    printFuncPtr("MG1");
                }
                else if (CPSS_TRACE_HW_ACCESS_ADDR_SPACE_MG2_E == addrSpace)
                {
                    printFuncPtr("MG2");
                }
                else if (CPSS_TRACE_HW_ACCESS_ADDR_SPACE_MG3_E == addrSpace)
                {
                    printFuncPtr("MG3");
                }
                else
                {
                    printFuncPtr("REG");
                }
                printFuncPtr("  ");
                /* print the data */
                appDemoTracePrintHwData(addr, length, dataPtr, mask, printFuncPtr);

            }


            printFuncPtr("\n");

            break;

        case APP_DEMO_TRACE_OUTPUT_MODE_DB_E:
            /* store the data */

            rc = appDemoTraceStoreHwData(devNum,
                                        portGroupId,
                                        isrContext,
                                        addrSpace,
                                        addr,
                                        length,
                                        dataPtr,
                                        mask,
                                        accessType,
                                         &appDemoAccessDbPtr,
                                        &appDemoAccessInfo);

            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case APP_DEMO_TRACE_OUTPUT_MODE_FILE_E:
            #ifdef ASIC_SIMULATION
                /* open trace data files for each core */
                if (devCoreTraceFilePtr[devNum][portGroupId] == NULL)
                {
                    sprintf(fileNameString, "device%u_asic%u_mem_dump.txt", devNum,portGroupId+1);
                    fileNamePtr = fileNameString;
                    devCoreTraceFilePtr[devNum][portGroupId] = fopen(fileNamePtr, "w");
                    if (devCoreTraceFilePtr[devNum][portGroupId] == NULL)
                    {
                        return GT_FAIL;
                    }
                }
                filePtr = devCoreTraceFilePtr[devNum][portGroupId];
                for (i = 0; i < length; i++)
                {
                    sprintf(buffer, "%08x %08x\n", addr, dataPtr[i]);
                    fprintf(filePtr, buffer);
                    addr += 4;
                    osMemSet(buffer,0,sizeof(buffer));
                }

            #else
               return GT_NOT_SUPPORTED;
            #endif
            break;
        default:
            return GT_BAD_STATE;
    }

    /* check if we got to critical address that need to add sleep before the print */
    if(appDemoTrace_do_millisecSleep)
    {
        osTimerWkAfter(appDemoTrace_millisecSleep);
        osPrintf("..A..");
    }

    return GT_OK;
}
#if (!defined(SHARED_MEMORY) || defined(LINUX_SIM))
/**
* @internal appDemoTraceWriteHwAccess function
* @endinternal
*
* @brief   Trace HW write access information.
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - ports group number
* @param[in] isrContext               - GT_TRUE: called from ISR context (cpssDrvPpHwIsrWrite,
*                                      cpssDrvPpHwIsrRead)
*                                      GT_FALSE: called from not ISR context.
* @param[in] addr                     - memory address space
* @param[in] addr                     - start address that the access was made to
* @param[in] length                   -  of the data that was written in words
* @param[in] dataPtr                  - (pointer to) data that was written
* @param[in] mask                     -  for data that was written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
static GT_STATUS appDemoTraceWriteHwAccess
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT         addrSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr,
    IN GT_U32      mask
)
{
    return appDemoTraceHwAccessAction(devNum,
                                      portGroupId,
                                      isrContext,
                                      addrSpace,
                                      addr,
                                      length,
                                      dataPtr,
                                      mask,
                                      appDemoTraceOutputMode,
                                      APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_E);
}

/**
* @internal appDemoTraceReadHwAccess function
* @endinternal
*
* @brief   Trace HW read access information.
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - ports group number
* @param[in] isrContext               - GT_TRUE: called from ISR context (cpssDrvPpHwIsrWrite,
*                                      cpssDrvPpHwIsrRead)
*                                      GT_FALSE: called from not ISR context.
* @param[in] addr                     - memory address space
* @param[in] addr                     - start address that the access was made to
* @param[in] length                   -  of the data that was written in words
* @param[in] dataPtr                  - (pointer to) data that was read
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
static GT_STATUS appDemoTraceReadHwAccess
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_BOOL     isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT         addrSpace,
    IN GT_U32      addr,
    IN GT_U32      length,
    IN GT_U32      *dataPtr
)
{
    return appDemoTraceHwAccessAction(devNum,
                                      portGroupId,
                                      isrContext,
                                      addrSpace,
                                      addr,
                                      length,
                                      dataPtr,
                                                                          0xFFFFFFFF,
                                      appDemoTraceOutputMode,
                                      APP_DEMO_TRACE_HW_ACCESS_TYPE_READ_E);
}


/**
* @internal appDemoTraceDelayHwAccess function
* @endinternal
*
* @brief   Trace HW write access information.
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - ports group number
* @param[in] millisec                 -  the delay in millisec
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
static GT_STATUS appDemoTraceDelayHwAccess
(
       IN GT_U8       devNum,
       IN GT_U32      portGroupId,
       IN GT_U32      millisec
)
{
    return appDemoTraceHwAccessAction(devNum,
                                      ((portGroupId==0xFFFFFFFF)?0:portGroupId),
                                      GT_FALSE,
                                      GT_FALSE,
                                      0,
                                      1,
                                      &millisec,
                                      0xFFFFFFFF,
                                      appDemoTraceOutputMode,
                                      APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E);
}
#endif
/**
* @internal appDemoSetHwAccessDbActiveState function
* @endinternal
*
* @brief   Update HW Access Read/Write DB state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad access type
*/
static GT_STATUS appDemoSetHwAccessDbActiveState
(
    IN            GT_BOOL                                                                         active,
    INOUT   APP_DEMO_CPSS_HW_ACCESS_DB_STC      **dbPtrPtr,
    INOUT   APP_DEMO_CPSS_HW_ACCESS_INFO_STC    *dbInfoPtr
)
{
    if (active == GT_TRUE)
    {
        /* Enable Tracing */

        if (dbInfoPtr->outputToDbIsActive == GT_FALSE)
        {
            if (*dbPtrPtr == NULL)
            {
                /* Initialize HW Access DB for the first time */
                *dbPtrPtr = osMalloc(APP_DEMO_CPSS_HW_ACCESS_DB_STEP_SIZE_CNS *
                                            sizeof(APP_DEMO_CPSS_HW_ACCESS_DB_STC));
                if (*dbPtrPtr == NULL)
                {
                    return GT_OUT_OF_CPU_MEM;
                }

                dbInfoPtr->maxSize = APP_DEMO_CPSS_HW_ACCESS_DB_STEP_SIZE_CNS;

                    /* The Access DB was already initialized.
                    Move cursor to point to the first access entry */
                dbInfoPtr->curSize = 0;
                dbInfoPtr->corrupted = GT_FALSE;
            }

       /* enable HW Access DB */
            dbInfoPtr->outputToDbIsActive = GT_TRUE;


        }
        else
            /* The HW Access DB is already initialized */
            return GT_OK;
    }
    else
    {
        /* Disable Tracing */

        if (dbInfoPtr->outputToDbIsActive == GT_TRUE)
        {
             if ((traceReadHwAccess == GT_FALSE) &&(traceWriteHwAccess == GT_FALSE))
             {
                /* disable HW Access DB */
                dbInfoPtr->outputToDbIsActive = GT_FALSE;
              }
             else
            {
                 /* if one of read or write is disabled, but the other is enabled,
                 the return code should not be failure, it is a legal state */
                 return GT_OK;
            }
        }
        else
            /* The HW Access DB is already disabled */
            return GT_OK;
    }

    return GT_OK;
}
#ifdef ASIC_SIMULATION
/**
* @internal prvAppDemoTraceHwFilesClose function
* @endinternal
*
* @brief   Close all Trace HW files.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on file close error
*/
static GT_STATUS prvAppDemoTraceHwFilesClose
(
    GT_VOID
)
{
    int rc = 0; /* fclose return */
    GT_U32 i,j;
    /* Mode is going to be changed. Close all open files */
    for (i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        for (j = 0; j < APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS; j++)
        {
            if (devCoreTraceFilePtr[i][j] != NULL)
            {
                rc = fclose(devCoreTraceFilePtr[i][j]);
                devCoreTraceFilePtr[i][j] = NULL;
                if (rc != 0)
                {
                    return GT_FAIL;
                }
            }
        }
    }
    return GT_OK;
}
#endif
/**
* @internal prvAppDemoTraceHwAccessEnable function
* @endinternal
*
* @brief   Trace HW read access information.
*
* @param[in] devNum                   - PP device number
* @param[in] accessType               - access type: read or write
* @param[in] enable                   - GT_TRUE:  tracing for given access type
*                                      GT_FALSE: disable tracing for given access type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad access type
*/
static GT_STATUS prvAppDemoTraceHwAccessEnable
(
    IN GT_U8                devNum,
    IN GT_U32               accessType,
    IN GT_BOOL              enable
)
{
    return appDemoTraceHwAccessEnable(devNum,
                                      (APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT)accessType,
                                      enable);
}

/**
* @internal appDemoTraceHwAccessEnable function
* @endinternal
*
* @brief   Trace HW read access information.
*
* @param[in] devNum                   - PP device number
* @param[in] accessType               - access type: read or write
* @param[in] enable                   - GT_TRUE:  tracing for given access type
*                                      GT_FALSE: disable tracing for given access type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad access type
*/
GT_STATUS appDemoTraceHwAccessEnable
(
    IN GT_U8                                devNum,
    IN APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT    accessType,
    IN GT_BOOL                              enable
)
{
    GT_STATUS                   rc;
    CPSS_DRV_HW_TRACE_TYPE_ENT  cpssDrvType;
    switch (accessType)
    {
        case APP_DEMO_TRACE_HW_ACCESS_TYPE_READ_E:
            /* Read HW Access */
            traceReadHwAccess = enable;

            if (appDemoTraceOutputMode == APP_DEMO_TRACE_OUTPUT_MODE_DB_E)
            {
                /* update Read HW Access DB */
                rc = appDemoSetHwAccessDbActiveState(enable,
                                                  &appDemoAccessDbPtr,
                                                  &appDemoAccessInfo);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            cpssDrvType = CPSS_DRV_HW_TRACE_TYPE_READ_E;


            break;
            /* No break*/
        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[unterminated_case] */
        case APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E:
            traceDelayHwAccess = enable;

        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[fallthrough] */
        case APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_E:
            /* Write HW Access */
             traceWriteHwAccess = enable;

            if (appDemoTraceOutputMode == APP_DEMO_TRACE_OUTPUT_MODE_DB_E)
            {
                /* update Write HW Access DB */
                rc = appDemoSetHwAccessDbActiveState(enable,
                                                  &appDemoAccessDbPtr,
                                                  &appDemoAccessInfo);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }


            if (accessType==APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E)
            {
                cpssDrvType = CPSS_DRV_HW_TRACE_TYPE_WRITE_DELAY_E;
            }
            else
            {
                cpssDrvType = CPSS_DRV_HW_TRACE_TYPE_WRITE_E;
            }

            if (enable==GT_FALSE) /*Delay access stops with write access*/
            {
                traceDelayHwAccess=GT_FALSE;
            }

            break;

        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[unterminated_case] */
        case APP_DEMO_TRACE_HW_ACCESS_TYPE_ALL_E: /* No break*/
            traceDelayHwAccess=enable;

        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[fallthrough] */
        case APP_DEMO_TRACE_HW_ACCESS_TYPE_BOTH_E:
            /* Read and Write HW Access */
             traceWriteHwAccess = enable;
            traceReadHwAccess = enable;

            if (appDemoTraceOutputMode == APP_DEMO_TRACE_OUTPUT_MODE_DB_E)
            {
                /* update Read HW Access DB */
                rc = appDemoSetHwAccessDbActiveState(enable,
                                                  &appDemoAccessDbPtr,
                                                  &appDemoAccessInfo);
                if (rc != GT_OK)
                {
                    return rc;
                }


            }


            if (accessType==APP_DEMO_TRACE_HW_ACCESS_TYPE_ALL_E)
            {
                cpssDrvType = CPSS_DRV_HW_TRACE_TYPE_ALL_E;
            }
            else
            {
                cpssDrvType = CPSS_DRV_HW_TRACE_TYPE_BOTH_E;
            }

            if (enable==GT_FALSE) /*Delay access stops with write access*/
            {
                traceDelayHwAccess=GT_FALSE;
            }


            break;

        default:
            return GT_BAD_PARAM;
    }
    if (enable == GT_FALSE)
    {
        /* trace hw access was disabled */
        #ifdef ASIC_SIMULATION
        if (appDemoTraceOutputMode == APP_DEMO_TRACE_OUTPUT_MODE_FILE_E)
        {
            rc = prvAppDemoTraceHwFilesClose();
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        #endif
    }
    return cpssDrvPpHwTraceEnable(devNum, cpssDrvType, enable);
}


/**
* @internal prvAppDemoTraceHwAccessOutputModeSet function
* @endinternal
*
* @brief   Set output tracing mode.
*
* @param[in] mode                     - output tracing mode: print, printSync or store.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on bad state
* @retval GT_BAD_PARAM             - on bad mode
*
* @note It is not allowed to change mode, while one of the HW Access DB
*       is enabled.
*
*/
static GT_STATUS prvAppDemoTraceHwAccessOutputModeSet
(
    IN GT_U32   mode
)
{
    return appDemoTraceHwAccessOutputModeSet((APP_DEMO_TRACE_OUTPUT_MODE_ENT)mode);
}
/**
* @internal appDemoTraceHwAccessOutputModeSet function
* @endinternal
*
* @brief   Set output tracing mode.
*
* @param[in] mode                     - output tracing mode: print, printSync or store.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on bad state
* @retval GT_BAD_PARAM             - on bad mode
*
* @note It is not allowed to change mode, while one of the HW Access DB
*       is enabled.
*
*/
GT_STATUS appDemoTraceHwAccessOutputModeSet
(
    IN APP_DEMO_TRACE_OUTPUT_MODE_ENT   mode
)
{
    GT_STATUS   rc;

    if (mode >= APP_DEMO_TRACE_OUTPUT_MODE_LAST_E)
    {
        return GT_BAD_PARAM;
    }

    if (appDemoTraceOutputMode == mode)
    {
        /* this mode is already set */
        return GT_OK;
    }

    if (appDemoAccessInfo.outputToDbIsActive == GT_TRUE)
    {
        /* check that  DB is enabled */
        return GT_BAD_STATE;
    }

    if (mode == APP_DEMO_TRACE_OUTPUT_MODE_DB_E)
    {
        if ((traceReadHwAccess == GT_TRUE) ||(traceWriteHwAccess == GT_TRUE))
        {
            /* update HW Access DB */
            rc = appDemoSetHwAccessDbActiveState(GT_TRUE,
                                              &appDemoAccessDbPtr,
                                              &appDemoAccessInfo);
            if (rc != GT_OK)
            {
                return rc;
            }
        }


    }
    if (appDemoTraceOutputMode == APP_DEMO_TRACE_OUTPUT_MODE_FILE_E)
    {
        #ifdef ASIC_SIMULATION
        rc = prvAppDemoTraceHwFilesClose();
        if (rc != GT_OK)
        {
            return rc;
        }
        #else
          return GT_NOT_SUPPORTED;
        #endif
    }
    appDemoTraceOutputMode = mode;

    return GT_OK;
}

/**
* @internal appDemoTraceHwAccessDbDump function
* @endinternal
*
* @brief   Dump HW Access DB.
*
* @param[in] accessType               - the type of DB to dump: read or write.
*                                       None.
*/
GT_STATUS appDemoTraceHwAccessDbDump
(
    IN APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT accessType
)
{
    APP_DEMO_CPSS_HW_ACCESS_INFO_STC    infoDb;         /* DB information */
    APP_DEMO_CPSS_HW_ACCESS_DB_STC      *accessDbPtr;   /* hw access DB */
    GT_U32                              portGroupId = 0;    /* port group ID */
    GT_BOOL                             isrContext = GT_FALSE;     /* flag for ISR context */
    CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT addrSpace = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E; /* flag for PCI */
    GT_U8                               devNum = 0;         /* device number */
    GT_U32                              addr = 0;           /* hw address */
    GT_U32                              length = 0;         /* length of the data */
    GT_U32                              dataArr[100];   /* array to store data */
    GT_U32                              ii;             /* loop iterator */
    GT_BOOL                             isMemNextWord = GT_FALSE;
    /* flag for next word in the memory */
    GT_BOOL                             dumpDataAndInfo = GT_FALSE;
    /* flag to dump data */
    GT_STATUS                           rc;             /* return code */
    APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT entryAccessType = APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_E;  /*set default due to compilation warning*/


    switch (accessType)
    {
    case APP_DEMO_TRACE_HW_ACCESS_TYPE_READ_E:
    case APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_E:
    case APP_DEMO_TRACE_HW_ACCESS_TYPE_BOTH_E:
        infoDb = appDemoAccessInfo;
        accessDbPtr = appDemoAccessDbPtr;
        break;

    default:
        return GT_BAD_PARAM;
    }

    if (infoDb.curSize == 0)
    {
        /* There is no Data in this DB */
        return GT_OK;
    }

    if(infoDb.corrupted == GT_TRUE)
    {
        osPrintf("\n\n\nWarning: The register access DB  has reached the limit  and loging has been stopped\n\n\n");
    }

    for (ii = 0; ii < infoDb.curSize; ii++)
    {
        /* parse the data from DB entry */

        /* check if the current word is not memory continuation */
        if (isMemNextWord)
        {
            /* we are still in the memory context */

            dataArr[length] = accessDbPtr[ii].data;
            length++;

            /* check if it is the last word of the current memory entry, bit25 */
            if ((accessDbPtr[ii].accessParamsBmp >> 25) & 0x1)
            {
                isMemNextWord = GT_FALSE;
                /* the data should be printed */
                dumpDataAndInfo = GT_TRUE;
            }
        }
        else
        {
            /* Register access or start of memory context */
            if(BIT2BOOL_MAC((accessDbPtr[ii].accessParamsBmp >> 26) & 0x1)  )
            {
                entryAccessType = APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_E;
            }
            else
            {
                entryAccessType = APP_DEMO_TRACE_HW_ACCESS_TYPE_READ_E;
            }

            /*dump only required access type*/
            if((accessType == APP_DEMO_TRACE_HW_ACCESS_TYPE_BOTH_E) ||(entryAccessType == accessType))
            {

                /* devNum */
                devNum = (accessDbPtr[ii].accessParamsBmp & 0xFF);
                /* portGroupId */
                portGroupId = ((accessDbPtr[ii].accessParamsBmp >> 8) & 0xFF);
                /* isrContext */
                isrContext = BIT2BOOL_MAC((accessDbPtr[ii].accessParamsBmp >> 16) & 0x1);
                /* addrSpace */
                addrSpace = ((accessDbPtr[ii].accessParamsBmp >> 17) & 0x7f);



                /* check if the data is for register access or memory access */
                if ((accessDbPtr[ii].accessParamsBmp >> 24) & 0x1)
                {
                    /* memory access */

                    /* set the first word data memory */
                    addr = accessDbPtr[ii].addr;
                    length = 0;
                    dataArr[length] = accessDbPtr[ii].data;
                    isMemNextWord = GT_TRUE;
                    length++;
                }
                else
                {
                    /* register access */
                    length = 1;
                    addr = accessDbPtr[ii].addr;
                    dataArr[0] = accessDbPtr[ii].data;
                    /* the data should be printed */
                    dumpDataAndInfo = GT_TRUE;
                }/* register access */
            }

        }/* register access or start of memory context */

        if (dumpDataAndInfo == GT_TRUE)
        {
            /* dump info and data */
            rc = appDemoTraceHwAccessAction(devNum,
                portGroupId,
                isrContext,
                addrSpace,
                addr,
                length,
                dataArr,
                0xFFFFFFFF,
                APP_DEMO_TRACE_OUTPUT_MODE_DIRECT_E,
                entryAccessType);
            if (rc != GT_OK)
            {
                return rc;
            }

            dumpDataAndInfo = GT_FALSE;
        }
    } /* for */

    return GT_OK;
}

/**
* @internal appDemoTraceHwAccessInfoCompare function
* @endinternal
*
* @brief   Compare given data and parameters with trace hw access DB info on
*         specified index.
* @param[in] dev                      - device number
* @param[in] accessType               - access type: read or write
* @param[in] index                    -  in the hw access db
* @param[in] portGroupId              - port group ID
* @param[in] isrContext               - GT_TRUE: ISR contextt
*                                      GT_FALSE: TASK context
* @param[in] addr                     - memory address space
* @param[in] addr                     - addresses to compare
* @param[in] mask                     -  to  the data from the db
* @param[in] data                     - data (read/written) to compare
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when there is no match between db info and input
*                                       parameters.
* @retval GT_BAD_STATE             - on unexpected ISR context
*/
GT_STATUS appDemoTraceHwAccessInfoCompare
(
    IN GT_U8                            dev,
    APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT   accessType,
    IN GT_U32                           index,
    IN GT_U32                           portGroupId,
    IN GT_BOOL                          isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT addrSpace,
    IN GT_U32                           addr,
    IN GT_U32                           mask,
    IN GT_U32                           data
)
{
    APP_DEMO_CPSS_HW_ACCESS_DB_STC      *accessDbPtr;
    APP_DEMO_CPSS_HW_ACCESS_DB_STC      accessDbEntry;
    GT_U8   writeBit;


    switch (accessType)
    {
        case APP_DEMO_TRACE_HW_ACCESS_TYPE_READ_E:
        case APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_E:
            accessDbPtr = appDemoAccessDbPtr;
            break;
        default:
            return GT_BAD_PARAM;
    }

    writeBit = (accessType == APP_DEMO_TRACE_HW_ACCESS_TYPE_READ_E)?0x0:0x1;

    accessDbEntry = accessDbPtr[index];

    /* check if the expected context is TASK and it is same as in DB,
       if not skip, this may be not expected ISR context */
    if (isrContext == GT_FALSE)
    {
        /* expected context is TASK */
        if ((BIT2BOOL_MAC((accessDbEntry.accessParamsBmp >> 16) & 0x1)) != isrContext)
        {
            /* skip */
            return GT_BAD_STATE;
        }
    }

    /* parse the data from DB entry and compare to input data and parameters */
    if ((accessDbEntry.addr != addr) || /* hw address */
        ((accessDbEntry.data & mask & accessDbEntry.mask) != (data & accessDbEntry.mask)) || /* hw data */
        ((accessDbEntry.accessParamsBmp & 0xFF) != dev) || /* devNum */
        (((accessDbEntry.accessParamsBmp >> 8) & 0xFF) != portGroupId) || /* portGroupId */
        ((BIT2BOOL_MAC((accessDbEntry.accessParamsBmp >> 16) & 0x1)) != isrContext) || /* isrContext */
        (((accessDbEntry.accessParamsBmp >> 17) & 0x7f) != (GT_U32)addrSpace)|| /* addrSpace */
        (((accessDbEntry.accessParamsBmp >> 26) & 0x1) != writeBit))/*read or write*/
    {
        /* there is no match between parameters or data */
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

typedef enum {
    APP_DEMO_DESCR_FIELD_TYPE_DEC_E,
    APP_DEMO_DESCR_FIELD_TYPE_HEX_E,
    APP_DEMO_DESCR_FIELD_TYPE_MAC_E,
    APP_DEMO_DESCR_FIELD_TYPE_IPV6_E,
    APP_DEMO_DESCR_FIELD_TYPE_IPV4_E,
    APP_DEMO_DESCR_FIELD_TYPE_HEX32_E
}APP_DEMO_DESCR_FIELD_TYPE_ENT;

static GT_BOOL appDemoDescrDumpRawDumpEn = GT_FALSE;

void appDemoDescrDumpRawDumpEnable
(
    IN GT_BOOL enable
)
{
    appDemoDescrDumpRawDumpEn = enable;
}

#ifdef CHX_FAMILY

static GT_STATUS appDemoDxChDescrDump
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT descriptorType,
    IN  GT_BOOL                           dumpAll
)
{
    GT_STATUS rc;
    GT_U32 i;  /* iterator */
    CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT descr;  /* iterator */
    GT_CHAR unitName[50];    /* unit name */
    /* pointer to descriptor information */
    CPSS_DXCH_DIAG_DESCRIPTOR_STC *descriptorPtr = NULL;
    APP_DEMO_DESCR_FIELD_TYPE_ENT fieldType; /* field type */
    GT_U32  *memoPtr;   /* memory pointer for unknown format */
    GT_U32   memoSize;  /* size of descriptor in bytes       */

    if(dumpAll == GT_FALSE)
    {
        descr = descriptorType;
    }
    else
    {
        descr = 0;
    }

    descriptorPtr = (CPSS_DXCH_DIAG_DESCRIPTOR_STC*)
                     cpssOsMalloc(sizeof(CPSS_DXCH_DIAG_DESCRIPTOR_STC));

    for(; descr < CPSS_DXCH_DIAG_DESCRIPTOR_LAST_E; descr++)
    {

        switch(descr)
        {
            case CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E:
                cpssOsStrCpy(unitName, "WR_DMA_TO_TTI");
                break;

            case CPSS_DXCH_DIAG_DESCRIPTOR_TTI_TO_PCL_E:
                cpssOsStrCpy(unitName, "TTI_TO_PCL");
                break;

            case CPSS_DXCH_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E:
                cpssOsStrCpy(unitName, "PCL_TO_BRIDGE");
                break;

            case CPSS_DXCH_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E:
                cpssOsStrCpy(unitName, "BRIDGE_TO_ROUTER");
                break;

            case CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E:
                /* invalid descriptor for xCat and xCat2*/
                if(((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                    (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                    (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))&&
                   (dumpAll == GT_TRUE))
                {
                    continue;
                }
                cpssOsStrCpy(unitName, "ROUTER_TO_INGRESS_POLICER_E");
                break;

            case CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E:
                cpssOsStrCpy(unitName, "POLICER0_TO_INGRESS_POLICER1");
                break;

            case CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E:
                cpssOsStrCpy(unitName, "INGRESS_POLICER1_TO_PRE_EGRESS");
                break;

            case CPSS_DXCH_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E:
                cpssOsStrCpy(unitName, "PRE_EGRESS_TO_TXQ");
                break;

            case CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E:
                cpssOsStrCpy(unitName, "TXQ_TO_HEADER_ALTERATION");
                break;

            case CPSS_DXCH_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E:
                cpssOsStrCpy(unitName, "EGRESS_POLICER_TO_EGRESS_PCL");
                break;

            default:
                cpssOsFree(descriptorPtr);
                return GT_BAD_PARAM;
        }

        /* Print in Raw Format */
        if (appDemoDescrDumpRawDumpEn != GT_FALSE)
        {
            memoSize = sizeof(CPSS_DXCH_DIAG_DESCRIPTOR_STC);
            memoPtr  = (GT_U32*)descriptorPtr;
            rc = cpssDxChDiagDescriptorPortGroupRawGet(
                devNum, portGroupsBmp, descr,
                &memoSize, memoPtr);
            if (rc != GT_OK)
            {
                cpssOsFree(descriptorPtr);
                return rc;
            }
            for (i = 0; (i < ((memoSize + 3) / 4)); i++)
            {
                if ((i % 8) == 0)
                {
                    cpssOsPrintf("%04X: ", (i * 4));
                }
                cpssOsPrintf("%08X ", memoPtr[i]);
                if ((i % 8) == 3)
                {
                    cpssOsPrintf("   ");
                }
                if ((i % 8) == 7)
                {
                    cpssOsPrintf("\n");
                }
            }
            cpssOsPrintf("\n");
        }

        rc = cpssDxChDiagDescriptorPortGroupGet(devNum, portGroupsBmp,
                                            descr, descriptorPtr);
        if(rc!= GT_OK)
        {
            cpssOsFree(descriptorPtr);
            return rc;
        }

        for(i = 0; i < CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_LAST_E; i++)
        {
            if(descriptorPtr->fieldValueValid[i] == GT_TRUE)
            {
                switch (i)
                {
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_ETHERTYPE_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB0_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB10_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB11_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB12_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB13_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB14_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB15_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB16_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB17_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB18_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB19_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB1_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB20_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB21_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB22_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB2_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB3_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB4_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB5_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB6_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB7_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB8_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB9_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_UDB_BYTESVALID_E:
                        fieldType = APP_DEMO_DESCR_FIELD_TYPE_HEX_E ;
                        break;
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_MACDA_0_31_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_MACSA_0_31_E:
                        fieldType = APP_DEMO_DESCR_FIELD_TYPE_MAC_E ;
                        break;
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_DIP_0_31_E:
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_SIP_0_31_E:
                        fieldType = APP_DEMO_DESCR_FIELD_TYPE_IPV6_E ;
                        break;
                    case CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_DIP_E:
                        fieldType = APP_DEMO_DESCR_FIELD_TYPE_IPV4_E ;
                        break;
                    default:
                        fieldType = APP_DEMO_DESCR_FIELD_TYPE_DEC_E;
                        break;
                }

                switch(fieldType)
                {
                    case APP_DEMO_DESCR_FIELD_TYPE_HEX_E:
                        cpssOsPrintf("%s; %s; 0x%X;\n", unitName, FIELDS_ARRAY[i],
                                     descriptorPtr->fieldValue[i]);
                        break;
                    case APP_DEMO_DESCR_FIELD_TYPE_MAC_E:
                        cpssOsPrintf("%s; %s; %02X:%02X:%02X:%02X:%02X:%02X;\n", unitName, FIELDS_ARRAY[i],
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i+1],8,8),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i+1],0,8),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i],24,8),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i],16,8),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i],8,8),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i],0,8));
                        /* the MAC use two fields, skip second one */
                        i++;
                        break;
                    case APP_DEMO_DESCR_FIELD_TYPE_IPV6_E:
                        cpssOsPrintf("%s; %s; %X:%X:%X:%X:%X:%X:%X:%X;\n", unitName, FIELDS_ARRAY[i],
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i+3],16,16),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i+3],0,16),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i+2],16,16),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i+2],0,16),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i+1],16,16),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i+1],0,16),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i],16,16),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i],0,16));
                        /* the IPV6 use four fields, skip three one */
                        i += 3;
                        break;
                    case APP_DEMO_DESCR_FIELD_TYPE_IPV4_E:
                        cpssOsPrintf("%s; %s; %d.%d.%d.%d;\n", unitName, FIELDS_ARRAY[i],
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i],24,8),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i],16,8),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i],8,8),
                                     U32_GET_FIELD_MAC(descriptorPtr->fieldValue[i],0,8));
                        break;
                    default:
                        cpssOsPrintf("%s; %s; %d;\n", unitName, FIELDS_ARRAY[i],
                                     descriptorPtr->fieldValue[i]);
                        break;
                }

            }
        }
        if(dumpAll == GT_FALSE)
        {
            break;
        }
        /* separate descriptors */
        cpssOsPrintf("\n\n");
    }

    cpssOsFree(descriptorPtr);
    return GT_OK;

}
#endif

GT_STATUS appDemoDescrDump
(
    IN GT_U8                   devNum,
    IN GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN GT_U32                  descriptorType,
    IN GT_BOOL                 dumpAll
)
{
#ifdef CHX_FAMILY
    return appDemoDxChDescrDump(devNum, portGroupsBmp, descriptorType, dumpAll);
#else
    return GT_NOT_SUPPORTED;
#endif
}

/**
* @internal appDemoTrafficDisableSet function
* @endinternal
*
* @brief   Set traffic enable/disable mode - API that will enable/disable
*         execution of traffic enable algorithm during cpssInitSystem
* @param[in] disable                  - GT_TRUE - do not enable traffic during cpssInitSystem
*                                      - GT_FALSE - enable traffic during cpssInitSystem
*                                       None
*/
GT_VOID appDemoTrafficDisableSet
(
    IN GT_BOOL disable
)
{
    appDemoTrafficEnableDisableMode = disable;
}

/**
* @internal appDemoTraceHwAccessDbLine function
* @endinternal
*
* @brief   returns a line of the trace database, either the read or write database,
*         delay uses the write database
* @param[in] index                    -  of the line to retreive
*                                      accessType - access type, either read, write or write and delay
*
* @param[out] dbLine                   - A non-null pointer to a APP_DEMO_CPSS_HW_ACCESS_DB_STC struct
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS appDemoTraceHwAccessDbLine
(
    GT_U32 index,
    OUT APP_DEMO_CPSS_HW_ACCESS_DB_STC * dbLine
)
{
    CPSS_NULL_PTR_CHECK_MAC(dbLine);

    if (index<appDemoAccessInfo.curSize)
    {
        dbLine->accessParamsBmp=appDemoAccessDbPtr[index].accessParamsBmp;
        dbLine->data=appDemoAccessDbPtr[index].data;
        dbLine->mask=appDemoAccessDbPtr[index].mask;
        dbLine->addr=appDemoAccessDbPtr[index].addr;
        return GT_OK;
    }
    else
    {
        return GT_BAD_PARAM;
    }

}

/**
* @internal appDemoTraceHwAccessClearDb function
* @endinternal
*
* @brief   Clear HW access db
*
* @retval GT_OK                    - on success else if failed
*/

GT_STATUS appDemoTraceHwAccessClearDb
(
    GT_VOID
)
{

    appDemoAccessInfo.curSize = 0;
    appDemoAccessInfo.corrupted = GT_FALSE;

    return GT_OK;

}

/**
* @internal appDemoTraceHwAccessDbIsCorrupted function
* @endinternal
*
* @brief   Check if the HW access data base has reached the limit
*
* @retval GT_OK                    - on success else if failed
*/

GT_STATUS appDemoTraceHwAccessDbIsCorrupted
(
    GT_BOOL * corruptedPtr
)
{

   * corruptedPtr = appDemoAccessInfo.corrupted ;

   return GT_OK;
}






/**
* @internal appDemoSupportAaMessageSet function
* @endinternal
*
* @brief   Configure the appDemo to support/not support the AA messages to CPU
*
* @param[in] supportAaMessage         -
*                                      GT_FALSE - not support processing of AA messages
*                                      GT_TRUE - support processing of AA messages
*                                       nothing
*/
GT_VOID appDemoSupportAaMessageSet
(
    IN GT_BOOL supportAaMessage
)
{
    appDemoSysConfig.supportAaMessage = supportAaMessage;
}

/* return GT_ABORT in callbacks: 0 - do not return GT_ABORT, != 0 - return GT_ABORT */
GT_U32 appDemoAccessToAddrAbort = 0;

/* register's address to trace */
GT_U32 appDemoAddrToTrace = 0;

/**
* @internal appDemoRegWriteCb function
* @endinternal
*
* @brief   Register write callback function for Hw access.
*
* @param[in] devNum                   -  The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to read from.
* @param[in] data                     - The value to be written.
* @param[in] stage                    -  for callback run.
*
* @retval GT_OK                    - callback success to get the data.
* @retval GT_FAIL                  - callback failed.
* @retval GT_ABORTED               - operation done and aborted by calback.
*
* @note When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
*       1. If callback returns GT_OK the parrent CPSS function continue processing.
*       2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
*       and return GT_OK to its caller.
*
*/
static GT_STATUS appDemoRegWriteCb
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    IN  GT_U32   regAddr,
    IN  GT_U32   data,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT  stage
)
{
    char *stageStr;
    stageStr = (CPSS_DRV_HW_ACCESS_STAGE_PRE_E == stage)? "PRE" : "POST";
    if (appDemoAddrToTrace == regAddr)
    {
        osPrintf("appDemoRegWriteCb devNum[%d] portGroupId[%d] regAddr[0X%08X] data[0X%08X] stage[%s]\n", devNum, portGroupId, regAddr, data, stageStr);
        if (appDemoAccessToAddrAbort)
        {
            return GT_ABORTED;
        }
    }
    return GT_OK;
}

/**
* @internal appDemoHwAccessTraceBind function
* @endinternal
*
* @brief   The function binds/unbinds a callback routines for HW access.
*
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - un bind callback routines.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoHwAccessTraceBind
(
    GT_BOOL                     bind
)
{
    CPSS_DRV_HW_ACCESS_OBJ_STC hwAccessObj;
    GT_STATUS rc;
    osMemSet(&hwAccessObj, 0, sizeof(hwAccessObj));
    hwAccessObj.hwAccessRegisterWriteFunc = appDemoRegWriteCb;

    rc = cpssDrvHwAccessObjectBind(&hwAccessObj, bind);
    return rc;
}

static GT_U32 appDemoHwAccessFailCounterLimit = 0;
static GT_U32 appDemoHwWriteAccessCounter = 0;
static GT_BOOL appDemoInsertHwFailEnable = GT_FALSE;

/**
* @internal appDemoWriteFailerCb function
* @endinternal
*
* @brief   write fail callback function for Hw access.
*
* @param[in] devNum                   -  The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to read from.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - The value to be written.
* @param[in] stage                    -  for callback run.
*
* @retval GT_OK                    - callback success to get the data.
* @retval GT_FAIL                  - callback failed.
*
* @note When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
*       1. If callback returns GT_OK the parrent CPSS function continue processing.
*       2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
*       and return GT_OK to its caller.
*
*/
static GT_STATUS appDemoWriteFailerCb
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    IN  GT_U32   regAddr,
    IN  GT_U32   length,
    IN  GT_U32   *data,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT  stage
)
{

    /* to prevent warnings */
    if (devNum);
    if (portGroupId);
    if (regAddr);
    if (length);
    if (data);
    if (appDemoInsertHwFailEnable == GT_TRUE)
    {
        if (stage == CPSS_DRV_HW_ACCESS_STAGE_PRE_E)
        {
            appDemoHwWriteAccessCounter++;
        }

        if (appDemoHwAccessFailCounterLimit > 0)
        {
            if (appDemoHwWriteAccessCounter >= appDemoHwAccessFailCounterLimit)
            {
                appDemoHwWriteAccessCounter = 0;
                appDemoHwAccessFailCounterLimit = 0;
                return GT_FAIL;
            }
        }
    }
    return GT_OK;
}


/**
* @internal appDemoHwAccessFailerBind function
* @endinternal
*
* @brief   The function binds/unbinds a appDemoWriteFailerCb callback for HW write access
*         and set hw write fail counter.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - un bind callback routines.
* @param[in] failCounter              - hw write fail counter: set hw write sequence
*                                      number on which write operation should fail.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoHwAccessFailerBind
(
    IN GT_BOOL                  bind,
    GT_U32                      failCounter
)
{
    CPSS_DRV_HW_ACCESS_OBJ_STC hwAccessObj;
    GT_STATUS rc;
    osMemSet(&hwAccessObj, 0, sizeof(hwAccessObj));
    hwAccessObj.hwAccessRamWriteFunc = appDemoWriteFailerCb;

    rc = cpssDrvHwAccessObjectBind(&hwAccessObj, bind);
    appDemoHwWriteAccessCounter = 0;
    if (bind == GT_TRUE)
    {
        appDemoInsertHwFailEnable = GT_TRUE;
        appDemoHwAccessFailCounterLimit  = failCounter;
    }
    else
    {
        appDemoInsertHwFailEnable = GT_FALSE;
        appDemoHwAccessFailCounterLimit = 0;
    }
    return rc;
}

/**
* @internal appDemoHwAccessCounterGet function
* @endinternal
*
* @brief   The function gets number of hw accesses
*
* @param[out] hwAccessCounterPtr       - points to hw access counter value.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoHwAccessCounterGet
(
    OUT GT_U32   *hwAccessCounterPtr
)
{
    *hwAccessCounterPtr = appDemoHwWriteAccessCounter;
    return GT_OK;
}

/************************************************************************/
/* Tracing specific registers access */
/************************************************************************/

/* filtering of devices and port groups not supported */
/* currently filtered  register addresses only        */

#define MAX_HW_PP_DRV_REG_TRACING_RANGES_CNS 64

/**
* @struct  appDemoHwPpDrvRegWriteTracingRangesArr
*
* @brief   Ranges for traced register addresses
*
*/
static struct
{
    /** register address mask - 0 value is a stamp of end of array */
    GT_U32 regAddrMask;
    /** register address pattern */
    GT_U32 regAddrPattern;
} appDemoHwPpDrvRegWriteTracingRangesArr[MAX_HW_PP_DRV_REG_TRACING_RANGES_CNS] = {{0}};

/**
* @internal appDemoHwPpDrvRegTracingRegAddressesRangeSet function
* @endinternal
*
* @brief Set register address ranges to be traced.
*
* @param[in] regAddrMask0    - The first register's address mask.
* @param[in] regAddrPattern0 - The first register's address pattern.
* @param[in] ...             - other pairs of register's addresses masks and patterns
*                              terminated by 0 value
*
*/
void appDemoHwPpDrvRegTracingRegAddressesRangeSet
(   IN GT_U32 regAddrMask0,
    IN GT_U32 regAddrPattern0,
    ...
)
{
    va_list argP;
    GT_U32 index;
    GT_U32 regAddrMask = regAddrMask0;
    GT_U32 regAddrPattern = regAddrPattern0;

    va_start(argP, regAddrPattern0);

    /* end array stamp */
    appDemoHwPpDrvRegWriteTracingRangesArr[
        MAX_HW_PP_DRV_REG_TRACING_RANGES_CNS - 1].regAddrMask = 0;

    for (index = 0; (index < (MAX_HW_PP_DRV_REG_TRACING_RANGES_CNS - 1)); index++)
    {
        appDemoHwPpDrvRegWriteTracingRangesArr[index].regAddrMask    = regAddrMask;
        appDemoHwPpDrvRegWriteTracingRangesArr[index].regAddrPattern = regAddrPattern;
        if (regAddrMask == 0) break;
        regAddrMask = va_arg(argP, GT_U32);
        regAddrPattern = va_arg(argP, GT_U32);
    }
    va_end(argP);
}

/**
* @internal appDemoHwPpDrvRegTracingIsTraced function
* @endinternal
*
* @brief   Check is register address is traced.
*
* @param[in] regAddr    - The register's address.
*
* @retval 1             - the register address should be traced.
* @retval 0             - the register address should not be traced.
*
*/
static int appDemoHwPpDrvRegTracingIsTraced
(
    IN GT_U32 regAddr
)
{
    GT_U32 i;
    for (i = 0; ((i < MAX_HW_PP_DRV_REG_TRACING_RANGES_CNS)
           && (appDemoHwPpDrvRegWriteTracingRangesArr[i].regAddrMask != 0)); i++)
    {
        if ((regAddr & appDemoHwPpDrvRegWriteTracingRangesArr[i].regAddrMask)
            == appDemoHwPpDrvRegWriteTracingRangesArr[i].regAddrPattern)
        {
            return 1;
        }
    }
    return 0;
}

/**
* @internal appDemoHwPpDrvRegTracingRegNameGet function
* @endinternal
*
* @brief   Get name (with path) of register by address.
*
* @param[in] rulesArr    - array of rules to reconize register by address.
* @param[in] regAddr     - address of register.
*
* @param[out] resultStr  - pointer to buffer for name of register.
*
*/
void appDemoHwPpDrvRegTracingRegNameGet
(
    IN APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC rulesArr[],
    IN GT_U32                                     regAddr,
    OUT char*                                     resultStr
)
{
    struct
    {
        GT_U32      applicableId;
        GT_U32      regAddr;
    } state;
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC   *rulePtr;
    GT_U32                                       ruleMatched;
    GT_U32                                       index;
    GT_U32                                       rangeBase;

    state.applicableId = 0;
    state.regAddr = regAddr;

    *resultStr = 0; /* default */
    while (1)
    {
        ruleMatched = 0;
        for (rulePtr = rulesArr; (ruleMatched == 0); rulePtr++)
        {
            /* all rules not applicable - finished */
            if (rulePtr->ruleType == APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_LAST_E) return;
            /* rule not applicable to state */
            if (state.applicableId != rulePtr->applicableId) continue;
            switch (rulePtr->ruleType)
            {
                case APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E:
                    /* check address */
                    if (state.regAddr != rulePtr->ruleParam[0]) continue;
                    /* the final stage reached */
                    osStrCat(resultStr, ".");
                    osStrCat(resultStr, rulePtr->resultName);
                    return;
                case APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E:
                    /* check low bound */
                    if (state.regAddr < rulePtr->ruleParam[0]) continue;
                    index = ((state.regAddr - rulePtr->ruleParam[0]) / rulePtr->ruleParam[1]);
                    /* check than index < amount */
                    if (index >= rulePtr->ruleParam[2]) continue;
                    /* check than regAdd == (addr + (index * step)) */
                    if (state.regAddr !=
                        (rulePtr->ruleParam[0] + (index * rulePtr->ruleParam[1]))) continue;
                    /* the final stage reached */
                    osStrCat(resultStr, ".");
                    osStrCat(resultStr, rulePtr->resultName);
                    resultStr += osStrlen(resultStr);
                    osSprintf(resultStr, "[%d]", index);
                    return;
                case APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E:
                    /* check low bound */
                    if (state.regAddr < rulePtr->ruleParam[0]) continue;
                    /* check high bound */
                    if (state.regAddr >= (rulePtr->ruleParam[0] + rulePtr->ruleParam[1])) continue;
                    /* rule matches regAddr */
                    ruleMatched = 1;
                    state.regAddr -= rulePtr->ruleParam[0];
                    state.applicableId = rulePtr->resultApplicableId;
                    osStrCat(resultStr, ".");
                    osStrCat(resultStr, rulePtr->resultName);
                    resultStr += osStrlen(resultStr);
                    break;
                case APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E:
                    /* check low bound */
                    if (state.regAddr < rulePtr->ruleParam[0]) continue;
                    index = ((state.regAddr - rulePtr->ruleParam[0]) / rulePtr->ruleParam[1]);
                    /* check than index < amount */
                    if (index >= rulePtr->ruleParam[2]) continue;
                    /* check than regAdd < (addr + (index * step) + size) */
                    rangeBase = (rulePtr->ruleParam[0] + (index * rulePtr->ruleParam[1]));
                    if (state.regAddr >= (rangeBase + rulePtr->ruleParam[3]))
                        continue;
                    /* rule matches regAddr */
                    ruleMatched = 1;
                    state.regAddr -= rangeBase;
                    state.applicableId = rulePtr->resultApplicableId;
                    osStrCat(resultStr, ".");
                    osStrCat(resultStr, rulePtr->resultName);
                    resultStr += osStrlen(resultStr);
                    osSprintf(resultStr, "[%d]", index);
                    resultStr += osStrlen(resultStr);
                    break;
                default: break;
            }
        }
    }
}

static APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC *appDemoHwPpDrvRegTracingRegAddrParsingPtr = NULL;

/**
* @internal appDemoHwPpDrvRegTracingRegNameBind function
* @endinternal
*
* @brief   Bind register names array.
*
*/
void appDemoHwPpDrvRegTracingRegNameBind
(
    IN APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC *regNameArrPtr
)
{
    appDemoHwPpDrvRegTracingRegAddrParsingPtr = regNameArrPtr;
}

/**
* @internal appDemoHwPpDrvRegTracingRegName function
* @endinternal
*
* @brief   Get name of register and data.
*
* @param[in] devNum     - Device number.
* @param[in] regAddr    - The register's address.
* @param[in] data       - The register's data.
*
* @retval 1             - (pointer to) string with name.
*
*/
static const char* appDemoHwPpDrvRegTracingRegName
(
    IN GT_U8  devNum,
    IN GT_U32 regAddr,
    IN GT_U32 data
)
{
    static char str[1024];

    /* yet not supported - avoid compiler warning */
    data = data;

    str[0] = 0;
    if (appDemoHwPpDrvRegTracingRegAddrParsingPtr)
    {
        appDemoHwPpDrvRegTracingRegNameGet(
            appDemoHwPpDrvRegTracingRegAddrParsingPtr,
            regAddr, str);
    }
    return str;
}

/**
* @internal appDemoHwPpDrvRegTracingMaskToFieldCnv function
* @endinternal
*
* @brief   Convert register read/write mask to field offset and length.
*
* @param[in]  mask                   - ead/write mask.
* @param[out] offsetPtr              - pointer to field offset.
* @param[out] lengthPtr              - pointer to field length.
*
* @retval GT_TRUE                  - conversion successfull.
* @retval GT_FALSE                 - conversion impossible.
*
*/
GT_BOOL appDemoHwPpDrvRegTracingMaskToFieldCnv
(
    IN   GT_U32  mask,
    OUT  GT_U32  *offsetPtr,
    OUT  GT_U32  *lengthPtr
)
{
    GT_U32 low, high;
    GT_U32 w_mask, w_len;
    if (mask == 0) return GT_FALSE;
    if (mask == 0xFFFFFFFF)
    {
        *offsetPtr = 0;
        *lengthPtr = 32;
        return GT_TRUE;
    }

    /* look for start one bit - skip all zeros from start */
    w_len = 16;
    low = 0;
    do
    {
        w_mask = (((1 << w_len) - 1) << low);
        if ((mask & w_mask) == 0)
        {
            low += w_len;
        }
        w_len /= 2;
    } while (w_len > 0);

    /* look for first zero bit - skip all zeros after first ones */
    w_len = ((32 - low) < 16) ? (32 - low) : 16;
    high = low;
    do
    {
        w_mask = (((1 << w_len) - 1) << high);
        if ((mask & w_mask) == w_mask)
        {
            high += w_len;
        }
        w_len /= 2;
    } while (w_len > 0);

    /* check that there ahe no ones over the high */
    w_mask = ((1 << high) - 1);
    if (mask & (~ w_mask)) return GT_FALSE;

    /* good result */
    *offsetPtr = low;
    *lengthPtr = (high - low);
    return GT_TRUE;
}

/**
* @internal appDemoHwPpDrvRegTracingWriteCb function
* @endinternal
*
* @brief   Register write callback function for Hw access.
*
* @param[in] devNum                   -  The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to write.
* @param[in] data                     - The value to be written.
* @param[in] stage                    -  for callback run.
*
* @retval GT_OK                    - callback success to get the data.
* @retval GT_FAIL                  - callback failed.
* @retval GT_ABORTED               - operation done and aborted by calback.
*
* @note When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
*       1. If callback returns GT_OK the parrent CPSS function continue processing.
*       2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
*       and return GT_OK to its caller.
*
*/
static GT_STATUS appDemoHwPpDrvRegTracingWriteCb
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    IN  GT_U32   regAddr,
    IN  GT_U32   data,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT  stage
)
{
    const char* name;

    if (CPSS_DRV_HW_ACCESS_STAGE_PRE_E != stage) return GT_OK;

    if (appDemoHwPpDrvRegTracingIsTraced(regAddr) == 0) return GT_OK;

    name = appDemoHwPpDrvRegTracingRegName(devNum,regAddr, data);
    osPrintf(
        "WRITE REG dev/pg %d/%d addr 0x%08X  data 0x%08X %s\n",
        devNum, portGroupId, regAddr, data, name);

    return GT_OK;
}

/**
* @internal appDemoHwPpDrvRegTracingRegFieldWriteCb function
* @endinternal
*
* @brief   Register field write callback function for Hw access.
*
* @param[in] devNum                   -  The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to write.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be write.
* @param[in] data                     - The value to be write.
* @param[in] stage                    -  for callback run.
*
* @retval GT_OK                    - callback success to get the data.
* @retval GT_FAIL                  - callback failed.
* @retval GT_ABORTED               - operation done and aborted by calback.
*
* @note When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
*       1. If callback returns GT_OK the parrent CPSS function continue processing.
*       2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
*       and return GT_OK to its caller.
*
*/
static GT_STATUS appDemoHwPpDrvRegTracingRegFieldWriteCb
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    IN  GT_U32   regAddr,
    IN GT_U32    fieldOffset,
    IN GT_U32    fieldLength,
    IN  GT_U32   data,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT  stage
)
{
    const char* name;

    if (fieldLength == 32)
    {
        /* write can call setField */
        return appDemoHwPpDrvRegTracingWriteCb(
            devNum, portGroupId, regAddr, data, stage);
    }

    if (CPSS_DRV_HW_ACCESS_STAGE_PRE_E != stage) return GT_OK;

    if (appDemoHwPpDrvRegTracingIsTraced(regAddr) == 0) return GT_OK;

    name = appDemoHwPpDrvRegTracingRegName(devNum,regAddr, data);
    osPrintf(
        "SET REG FLD dev/pg %d/%d addr 0x%08X  off %d len %d data 0x%X %s\n",
        devNum, portGroupId, regAddr, fieldOffset, fieldLength, data, name);

    return GT_OK;
}

/**
* @internal appDemoHwPpDrvRegTracingRegMaskWriteCb function
* @endinternal
*
* @brief   Register field write callback function for Hw access.
*
* @param[in] devNum                   -  The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to write.
* @param[in] mask                     - Mask for selecting the read bits.
* @param[in] data                     - The value to be write.
* @param[in] stage                    - stage for callback run.
*
* @retval GT_OK                    - callback success to get the data.
* @retval GT_FAIL                  - callback failed.
* @retval GT_ABORTED               - operation done and aborted by calback.
*
* @note When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
*       1. If callback returns GT_OK the parrent CPSS function continue processing.
*       2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
*       and return GT_OK to its caller.
*
*/
GT_STATUS appDemoHwPpDrvRegTracingRegMaskWriteCb
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    IN  GT_U32   regAddr,
    IN  GT_U32   mask,
    IN  GT_U32   data,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT  stage
)
{
    const char* name;
    GT_U32  fieldOffset;
    GT_U32  fieldLength;

    if (appDemoHwPpDrvRegTracingMaskToFieldCnv(
        mask, &fieldOffset, &fieldLength) != GT_FALSE)
    {
        return appDemoHwPpDrvRegTracingRegFieldWriteCb(
            devNum, portGroupId, regAddr,
            fieldOffset, fieldLength, (data >> fieldOffset), stage);
    }

    if (CPSS_DRV_HW_ACCESS_STAGE_PRE_E != stage) return GT_OK;

    if (appDemoHwPpDrvRegTracingIsTraced(regAddr) == 0) return GT_OK;

    name = appDemoHwPpDrvRegTracingRegName(devNum, regAddr, data);
    osPrintf(
        "SET REG MASK dev/pg %d/%d addr 0x%08X  mask 0x%08X data 0x%08X %s\n",
        devNum, portGroupId, regAddr, mask, data, name);

    return GT_OK;
}

/**
* @internal appDemoHwPpDrvRegTracingReadCb function
* @endinternal
*
* @brief   Register read callback function for Hw access.
*
* @param[in] devNum                   -  The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to read.
* @param[in] data                     - The value to be read.
* @param[in] stage                    -  for callback run.
*
* @retval GT_OK                    - callback success to get the data.
* @retval GT_FAIL                  - callback failed.
* @retval GT_ABORTED               - operation done and aborted by calback.
*
* @note When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
*       1. If callback returns GT_OK the parrent CPSS function continue processing.
*       2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
*       and return GT_OK to its caller.
*
*/
static GT_STATUS appDemoHwPpDrvRegTracingReadCb
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    IN  GT_U32   regAddr,
    IN  GT_U32   *data,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT  stage
)
{
    const char* name;

    if (CPSS_DRV_HW_ACCESS_STAGE_POST_E != stage) return GT_OK;

    if (appDemoHwPpDrvRegTracingIsTraced(regAddr) == 0) return GT_OK;

    name = appDemoHwPpDrvRegTracingRegName(devNum, regAddr, *data);
    osPrintf(
        "READ REG dev/pg %d/%d addr 0x%08X  data 0x%08X %s\n",
        devNum, portGroupId, regAddr, *data, name);

    return GT_OK;
}

/**
* @internal appDemoHwPpDrvRegTracingRegFieldReadCb function
* @endinternal
*
* @brief   Register field read callback function for Hw access.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                       devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to read.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
* @param[in] data                     - The value dot by read.
* @param[in] stage                    - for callback run.
*
* @retval GT_OK                    - callback success to get the data.
* @retval GT_FAIL                  - callback failed.
* @retval GT_ABORTED               - operation done and aborted by calback.
*
* @note When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
*       1. If callback returns GT_OK the parrent CPSS function continue processing.
*       2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
*       and return GT_OK to its caller.
*
*/
static GT_STATUS appDemoHwPpDrvRegTracingRegFieldReadCb
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    IN  GT_U32   regAddr,
    IN  GT_U32   fieldOffset,
    IN  GT_U32   fieldLength,
    IN  GT_U32   *data,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT  stage
)
{
    const char* name;

    if (fieldLength == 32)
    {
        /* reead can call getField */
        return appDemoHwPpDrvRegTracingReadCb(
            devNum, portGroupId, regAddr, data, stage);
    }

    if (CPSS_DRV_HW_ACCESS_STAGE_POST_E != stage) return GT_OK;

    if (appDemoHwPpDrvRegTracingIsTraced(regAddr) == 0) return GT_OK;

    name = appDemoHwPpDrvRegTracingRegName(devNum,regAddr, *data);
    osPrintf(
        "GET REG FLD dev/pg %d/%d addr 0x%08X  off %d len %d data 0x%X %s\n",
        devNum, portGroupId, regAddr, fieldOffset, fieldLength, *data, name);

    return GT_OK;
}

/**
* @internal appDemoHwPpDrvRegTracingRegMaskReadCb function
* @endinternal
*
* @brief   Register field read callback function for Hw access.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                       devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to read.
* @param[in] mask                     - Mask for selecting the read bits.
* @param[in] data                     - The value got by read.
* @param[in] stage                    - stage for callback run.
*
* @retval GT_OK                    - callback success to get the data.
* @retval GT_FAIL                  - callback failed.
* @retval GT_ABORTED               - operation done and aborted by calback.
*
* @note When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
*       1. If callback returns GT_OK the parrent CPSS function continue processing.
*       2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
*       and return GT_OK to its caller.
*
*/
GT_STATUS appDemoHwPpDrvRegTracingRegMaskReadCb
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    IN  GT_U32   regAddr,
    IN  GT_U32   mask,
    IN  GT_U32   *data,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT  stage
)
{
    const char* name;
    GT_U32  fieldOffset;
    GT_U32  fieldLength;
    GT_U32  shiftedData;

    if (appDemoHwPpDrvRegTracingMaskToFieldCnv(
        mask, &fieldOffset, &fieldLength) != GT_FALSE)
    {
        shiftedData = ((*data) >> fieldOffset);
        return appDemoHwPpDrvRegTracingRegFieldReadCb(
            devNum, portGroupId, regAddr,
            fieldOffset, fieldLength, &shiftedData, stage);
    }

    if (CPSS_DRV_HW_ACCESS_STAGE_POST_E != stage) return GT_OK;

    if (appDemoHwPpDrvRegTracingIsTraced(regAddr) == 0) return GT_OK;

    name = appDemoHwPpDrvRegTracingRegName(devNum, regAddr, *data);
    osPrintf(
        "GET REG MASK dev/pg %d/%d addr 0x%08X  mask 0x%08X data 0x%08X %s\n",
        devNum, portGroupId, regAddr, mask, *data, name);

    return GT_OK;
}

/**
* @internal appDemoHwPpDrvRegTracingBind function
* @endinternal
*
* @brief   The function binds/unbinds a callback routines for HW access.
*
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                        GT_FALSE - un bind callback routines.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoHwPpDrvRegTracingBind
(
    GT_BOOL                     bind
)
{
    CPSS_DRV_HW_ACCESS_OBJ_STC hwAccessObj;
    GT_STATUS rc;
    osMemSet(&hwAccessObj, 0, sizeof(hwAccessObj));
    hwAccessObj.hwAccessRegisterWriteFunc        = appDemoHwPpDrvRegTracingWriteCb;
    hwAccessObj.hwAccessRegisterFieldWriteFunc   = appDemoHwPpDrvRegTracingRegFieldWriteCb;
    hwAccessObj.hwAccessRegisterBitMaskWriteFunc = appDemoHwPpDrvRegTracingRegMaskWriteCb;
    hwAccessObj.hwAccessRegisterReadFunc         = appDemoHwPpDrvRegTracingReadCb;
    hwAccessObj.hwAccessRegisterFieldReadFunc    = appDemoHwPpDrvRegTracingRegFieldReadCb;
    hwAccessObj.hwAccessRegisterBitMaskReadFunc  = appDemoHwPpDrvRegTracingRegMaskReadCb;

    rc = cpssDrvHwAccessObjectBind(&hwAccessObj, bind);
    return rc;
}

/************************************************************************/
/* End of Tracing specific registers access */
/************************************************************************/


extern GT_STATUS wrCpssResetLib
(
    GT_VOID
);

extern GT_STATUS appDemoDxChDbReset
(
    IN  GT_U8  devNum,
    IN  GT_BOOL needHwReset
);

extern GT_STATUS appDemoExMxPmDbReset
(
    IN  GT_U8  devNum
);

extern GT_STATUS appDemoPxDbReset
(
    IN  GT_U8  devNum,
    IN  GT_BOOL needHwReset
);

/*
    CPSS_SYSTEM_RESET_TYPE_ENT - type of 'system reset' that need to do
    ===============
    CPSS_SYSTEM_RESET_TYPE_HW_SOFT_RESET_AND_SW_E - the HW will do soft reset ,
                            and SW need to clean all DB.
                            ready for new cpssInitSystem(...)
    CPSS_SYSTEM_RESET_TYPE_RESET_SW_AFTER_FAST_BOOT_E - the HW is not reset.
                            the SW already did 'fast boot' initialization
                            and need to cleanup.
                            ready for new 'regular' cpssInitSystem(...)
    CPSS_SYSTEM_RESET_TYPE_RESET_SW_IN_gtShutdownAndCoreRestart_E -
                            the SW already regular initialization
                            and need to cleanup.
                            ready for new 'regular' cpssInitSystem(...)

                            NOTEs:
                            1. this to emulate PSS fu function gtShutdownAndCoreRestart(...)
                            2. this to test that the re-init of cpss take info about DMA from:
                                    PRV_CPSS_DMA_RE_INIT_INFO_STC   dmaInfo;
                            3. the function not removes configurations from the HW
                               (vlans , TM , LPM , NH , FDB ...)
                               assumption is that runs between tests that cleared their config.

*/
typedef enum
{
    CPSS_SYSTEM_RESET_TYPE_HW_SOFT_RESET_AND_SW_E,
    CPSS_SYSTEM_RESET_TYPE_RESET_SW_AFTER_FAST_BOOT_E,
    CPSS_SYSTEM_RESET_TYPE_RESET_SW_IN_gtShutdownAndCoreRestart_E,

}CPSS_SYSTEM_RESET_TYPE_ENT;

static char* cpss_system_reset_type_names_arr[
    CPSS_SYSTEM_RESET_TYPE_RESET_SW_IN_gtShutdownAndCoreRestart_E+1] = {
    "CPSS_SYSTEM_RESET_TYPE_HW_SOFT_RESET_AND_SW_E",
    "CPSS_SYSTEM_RESET_TYPE_RESET_SW_AFTER_FAST_BOOT_E",
    "CPSS_SYSTEM_RESET_TYPE_RESET_SW_IN_gtShutdownAndCoreRestart_E"
};

/**
* @internal internal_cpssResetSystem function
* @endinternal
*
* @brief   API performs CPSS reset that includes device remove, mask interrupts,
*         queues and tasks destroy, all static variables/arrays clean.
* @param[in] resetType                - the reset type
*
* @retval GT_OK                    - on success else if failed
*/
static GT_STATUS internal_cpssResetSystem
(
    IN  CPSS_SYSTEM_RESET_TYPE_ENT     resetType
)
{
    GT_STATUS       rc;         /* To hold funcion return code  */
    GT_U32          i;          /* Loop indexes                 */
    GT_U8           devNum;     /* Device number                */


    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of Reset system */
    GT_BOOL needHwReset =
        (resetType == CPSS_SYSTEM_RESET_TYPE_HW_SOFT_RESET_AND_SW_E) ?
        GT_TRUE : GT_FALSE;/* indication that need HW reset */

    if(resetType > CPSS_SYSTEM_RESET_TYPE_RESET_SW_IN_gtShutdownAndCoreRestart_E)
    {
        return GT_BAD_PARAM;
    }

    if(initCounter == 0)
    {
        /* indication that the cpssInitSystem did not run even for first time ,
        so nothing to reset */
        /* this code for the 'Generic phase' between tests that do:
        1. cpssResetSystem
        2. cpssInitSystem
        */
        return GT_NOT_INITIALIZED;
    }

    if(NULL == (GT_PTR)boardCfgFuncs.boardCleanDbDuringSystemReset)
    {
        /* the specific board not implemented 'System reset' */
        return GT_NOT_SUPPORTED;
    }

    /* first check that all devices in the system supports the 'system reset' */
    if(needHwReset == GT_TRUE)
    {
        for (i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
        {
            if(appDemoPpConfigList[i].valid == GT_FALSE)
            {
                continue;
            }

            if(appDemoPpConfigList[i].devSupportSystemReset_HwSoftReset == GT_FALSE)
            {
                /* the device not support the 'System reset' */
                return GT_NOT_SUPPORTED;
            }
        }
    }

    osPrintSync("cpssResetSystem [%s]: started \n",
        cpss_system_reset_type_names_arr[resetType]);

    /******************************/
    /* take the time before start */
    /******************************/
    rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    appDemoPrePhase1Init = GT_FALSE;
    appDemoCpssInitialized = GT_FALSE;
#if defined  CPSS_USE_MUTEX_PROFILER
    cpssInitSystemIsDone = GT_FALSE;
#endif

#ifdef  INCLUDE_UTF
#ifdef CHX_FAMILY
    if(needHwReset == GT_FALSE && prvTgfFdbShadowUsed == GT_TRUE)
    {
        osPrintSync("cpssResetSystem : flush FDB and drain AUQ before killing appDemo tasks \n");
        /* the next function is aware to :
        1. AUQ state before starting the FLUSH
        2. aware to using 'shadow' ... do deleting entries in SHADOW.
            In GM/Emulator it would be one by one if trigger 'HW flush'
        */
#ifndef GM_USED
        prvTgfFdbShadowUsed = GT_FALSE;

        rc = prvTgfBrgFdbFlush(GT_TRUE/*includeStatic*/);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvTgfBrgFdbFlush - HW", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        prvTgfFdbShadowUsed = GT_TRUE;
#endif /*!GM_USED*/

        rc = prvTgfBrgFdbFlush(GT_TRUE/*includeStatic*/);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvTgfBrgFdbFlush - SHADOW", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
#endif /*CHX_FAMILY*/
#endif /*INCLUDE_UTF*/
    osPrintSync("cpssResetSystem : reset events engine and kill event handlers tasks \n");
    rc = appDemoEventRequestDrvnModeReset();
    if (rc != GT_OK)
    {
        goto exitCleanly_lbl;
    }

    for (i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if(appDemoPpConfigList[i].valid == GT_TRUE)
        {
            devNum = appDemoPpConfigList[i].devNum;
            #ifdef CHX_FAMILY
            rc = appDemoBrgFdbIpv6UcFreeAddrDataLinkedList(devNum);
            if(GT_OK != rc)
            {
                return rc;
            }
            #endif
            osPrintSync("cpssResetSystem : start remove device[%d] \n",
                devNum);

            /* next is done inside prvCpssPpConfigDevDbRelease(...)
                the DXCH remove device call it ...
                the puma should too... */
            /*rc = cpssBmPoolDelete(devPtr->intCtrl.txDescList[queueNum].poolId);*/

            if (resetType != CPSS_SYSTEM_RESET_TYPE_RESET_SW_AFTER_FAST_BOOT_E)
            {
                if(CPSS_PX_FAMILY_PIPE_E == appDemoPpConfigList[i].devFamily)
                {
#ifdef PX_FAMILY
                    rc = appDemoPxDbReset(devNum , needHwReset);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("actual remove device and soft reset not implemented ", rc);
#endif
                }
                else
                {
#ifdef CHX_FAMILY
                    rc = appDemoDxChDbReset(devNum , needHwReset);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChDbReset", rc);
#endif
                }

#ifdef ASIC_SIMULATION
                {
                    GT_U32 k = 4000;     /* Wait for a limited time */

                    /* Wait simulationSoftResetDone to be reset */
                    while((simulationSoftResetDone == 0) && (k > 0))
                    {
                        osTimerWkAfter(5);
                        k--;
                    }
                }
#endif
            }
            else
            {

                if(CPSS_PX_FAMILY_PIPE_E == appDemoPpConfigList[i].devFamily)
                {
#ifdef PX_FAMILY
                    /* no DB reset, only remove device */
                    rc = cpssPxCfgDevRemove(devNum);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxCfgDevRemove", rc);
                    if (rc != GT_OK)
                    {
                        goto exitCleanly_lbl;
                    }
#endif
                }
                else
                {
#ifdef CHX_FAMILY
                    /* no DB reset, only remove device */
                    rc = cpssDxChCfgDevRemove(devNum);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCfgDevRemove", rc);
                    if (rc != GT_OK)
                    {
                        goto exitCleanly_lbl;
                    }
#endif
                }

                /* do board specific type reset */
                rc = boardCfgFuncs.boardCleanDbDuringSystemReset(devNum);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardCleanDbDuringSystemReset", rc);

                goto exitCleanly_lbl;
            }

            if(GT_OK != rc)
            {
                goto exitCleanly_lbl;
            }

            osMemSet(&(appDemoPpConfigList[i]), 0, sizeof(appDemoPpConfigList[i]));

            appDemoPpConfigList[i].ppPhase1Done = GT_FALSE;
            appDemoPpConfigList[i].ppPhase2Done = GT_FALSE;
            appDemoPpConfigList[i].ppLogicalInitDone = GT_FALSE;
            appDemoPpConfigList[i].ppGeneralInitDone = GT_FALSE;
            appDemoPpConfigList[i].valid = GT_FALSE;

            /* default value for cpu tx/rx mode */
            appDemoPpConfigList[i].cpuPortMode = CPSS_NET_CPU_PORT_MODE_SDMA_E;
            appDemoPpConfigList[i].allocMethod = CPSS_RX_BUFF_STATIC_ALLOC_E;

            /* cascading information */
            appDemoPpConfigList[i].numberOfCscdTrunks = 0;
            appDemoPpConfigList[i].numberOfCscdPorts = 0;
            appDemoPpConfigList[i].numberOfCscdTargetDevs = 0;
            appDemoPpConfigList[i].numberOf10GPortsToConfigure = 0;
            appDemoPpConfigList[i].internal10GPortConfigFuncPtr = NULL;
            appDemoPpConfigList[i].internalCscdPortConfigFuncPtr = NULL;

            osPrintSync("cpssResetSystem : end remove device[%d] \n",
                devNum);
        }
    }

    if(needHwReset == GT_TRUE)
    {
        /*osPrintSync("cpssResetSystem : destroy generic global cpss DB \n");*/
        rc = cpssPpDestroy();
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPpDestroy", rc);
        if(GT_OK != rc)
        {
            goto exitCleanly_lbl;
        }
    }


    osPrintSync("cpssResetSystem : board specific type reset \n");
    /* do board specific type reset */
    rc = boardCfgFuncs.boardCleanDbDuringSystemReset((GT_U8)storedBoardIdx);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardCleanDbDuringSystemReset", rc);
    if(GT_OK != rc)
    {
        goto exitCleanly_lbl;
    }

#ifdef IMPL_GALTIS
    osPrintSync("cpssResetSystem : galtis wrappers reset \n");
    rc = wrCpssResetLib();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("wrCpssResetLib", rc);
    if(GT_OK != rc)
    {
        goto exitCleanly_lbl;
    }
#endif /* IMPL_GALTIS */
#ifdef IMPL_TGF
    tgfTrafficGeneratorSystemReset();
#endif /* IMPL_TGF */

    if(needHwReset == GT_TRUE)
    {
        /* destroy DMA only after other registered function detached from the DMA */
        osPrintSync("cpssResetSystem : destroy All DMA allocations \n");
        osCacheDmaFreeAll();
        CPSS_ENABLER_DBG_TRACE_RC_MAC("osCacheDmaFreeAll", rc);
        if(GT_OK != rc)
        {
            goto exitCleanly_lbl;
        }

        /********************************/
        /* wait until the reset is done */
        /********************************/
        #ifdef ASIC_SIMULATION
        #ifndef _WIN32 /* it seems that linux miss something */
            SIM_OS_MAC(simOsSleep)(100);
        #endif /*_WIN32*/
        while(skernelStatusGet())
        {
            osTimerWkAfter(10);
            osPrintf("7");
        }
        osPrintf("\n");
        #else /* not ASIC_SIMULATION */
        osTimerWkAfter(10);
        #endif /* not ASIC_SIMULATION */
    }

#ifdef CHX_FAMILY
    auFdbUpdateLockReset();
#endif /*CHX_FAMILY*/

#ifdef SHARED_MEMORY
    /* stop fdbLearning and RxTxProcess */
    osPrintf("stopping fdbLearning and RxTxProcess...");
    if (fdbLearningProcessId > 0)
    {
        kill(fdbLearningProcessId, SIGTERM);
        waitpid(fdbLearningProcessId, NULL, 0);
        fdbLearningProcessId = 0;
    }
    if (RxTxProcessId > 0)
    {
        kill(RxTxProcessId, SIGTERM);
        waitpid(RxTxProcessId, NULL, 0);
        RxTxProcessId = 0;
    }
    osPrintf(" done\n");
#endif


exitCleanly_lbl:


    /******************************/
    /* take the time after finish */
    /******************************/
    if (cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd) != GT_OK)
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

    osPrintSync("cpssResetSystem [%s] iteration#(%d) --  ended time: %d sec., %d nanosec.\n",
        cpss_system_reset_type_names_arr[resetType],
        resetCounter , seconds, nanoSec);
    resetCounter++;

    appDemoCpssInitSkipHwReset = (needHwReset == GT_TRUE) ? GT_FALSE : GT_TRUE;

    return rc;
}

/**
* @internal cpssResetSystem function
* @endinternal
*
* @brief   API performs CPSS reset that includes device remove, mask interrupts,
*         queues and tasks destroy, all static variables/arrays clean.
* @param[in] doHwReset                - indication to do HW reset
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS cpssResetSystem(
    IN GT_BOOL doHwReset
)
{
    return internal_cpssResetSystem(
        doHwReset == GT_TRUE ?
        CPSS_SYSTEM_RESET_TYPE_HW_SOFT_RESET_AND_SW_E :
        CPSS_SYSTEM_RESET_TYPE_RESET_SW_IN_gtShutdownAndCoreRestart_E);
}


/**
* @internal cpssResetSystemNoSoftReset function
* @endinternal
*
* @brief   API performs device remove and do board specific type reset.
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS cpssResetSystemNoSoftReset(void)
{
    return internal_cpssResetSystem(CPSS_SYSTEM_RESET_TYPE_RESET_SW_AFTER_FAST_BOOT_E);
}


/**
* @internal cpssDevSupportSystemResetSet function
* @endinternal
*
* @brief   API controls if SYSTEM reset is supported, it is used to disable the
*         support on devices that do support. vice versa is not possible
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS cpssDevSupportSystemResetSet(

    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_U32  i;
    static GT_CHAR  devForceModeSystemReset_HwSoftReset_str[100];

    cpssOsBzero(devForceModeSystemReset_HwSoftReset_str, sizeof(devForceModeSystemReset_HwSoftReset_str));

    for (i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if(appDemoPpConfigList[i].valid == GT_FALSE)
        {
            continue;
        }

        if(appDemoPpConfigList[i].devNum != devNum)
        {
            continue;
        }

        cpssOsSprintf(devForceModeSystemReset_HwSoftReset_str,"devForceModeSystemReset_HwSoftReset_str_%d",devNum);
        /* add to static DB that not reset by system reset */
        appDemoStaticDbEntryAdd(devForceModeSystemReset_HwSoftReset_str,enable ? 1 : 0);

        appDemoPpConfigList[i].devSupportSystemReset_HwSoftReset = enable;

        return GT_OK;
    }

    return GT_NOT_FOUND;
}

/**
* @internal cpssDevMustNotResetPortSet function
* @endinternal
*
* @brief   API controls if SYSTEM reset (without HW reset) should skip reset of
*         specific port.
*         this for example needed for 'OOB' port (CPU out of band port) that serves
*         , for example : 'TELNET' . so need skip reset this port to avoid
*         disconnection of the session.
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS cpssDevMustNotResetPortSet(

    IN  GT_U8       devNum,
    IN  GT_U32      portNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;
    static GT_CHAR  devMustNotResetPort_str[100];

    cpssOsBzero(devMustNotResetPort_str, sizeof(devMustNotResetPort_str));

    cpssOsSprintf(devMustNotResetPort_str,"devMustNotResetPort_str_d[%d]p[%d]",devNum,portNum);
    /* add to static DB that not reset by system reset */
    rc = appDemoStaticDbEntryAdd(devMustNotResetPort_str,enable ? 1 : 0);

    return rc ;
}
/**
* @internal boardPhyConfig function
* @endinternal
*
* @brief   Board specific PHY configurations after board initialization.
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS boardPhyConfig
(
    IN  GT_U8       boardRevId,
    IN  GT_U8       devNum
)
{
    GT_STATUS rc = GT_OK;

#ifdef CHX_FAMILY
    if (PRV_CPSS_PP_MAC(devNum)->devFamily ==  CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        rc = bobcat2BoardPhyConfig(boardRevId,devNum);
        return rc;
    }
#endif
    return rc;
}

/**
* @internal boardExtPhyConfig function
* @endinternal
*
* @brief   External PHY configuration for device.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad phyType or smiInterface
* @retval GT_NOT_READY             - smi is busy
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS boardExtPhyConfig
(
    IN  GT_U8       devNum,
    IN  GT_U32      phyType,
    IN  GT_U32      smiInterface,
    IN  GT_U32      smiStartAddr
)
{
    GT_STATUS rc = GT_OK;

    rc = appDemoBoardExternalPhyConfig(devNum, phyType, smiInterface, smiStartAddr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBoardExternalPhyConfig", rc);

    return rc;
}

/**
* @internal appDemoDevIdxGet function
* @endinternal
*
* @brief   Find device index in "appDemoPpConfigList" array based on device SW number.
*
* @param[in] devNum                   - SW device number
*
* @param[out] devIdxPtr                - (pointer to) device index in array "appDemoPpConfigList".
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_FAIL                  - no match found for devNum.
*/
GT_STATUS appDemoDevIdxGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *devIdxPtr
)
{
    GT_U32 ii;

    for ( ii = 0 ; ii < PRV_CPSS_MAX_PP_DEVICES_CNS ; ii++ )
    {
        if ( appDemoPpConfigList[ii].devNum == devNum)
        {
            *devIdxPtr = ii;
            return GT_OK;
        }
    }

    return GT_FAIL;
}


/**
* @internal appDemoCopyFileFromRamFs function
* @endinternal
*
* @brief   Copy file from RAMFS to local FS
*
* @param[in] srcFileName              - a source file name (in RAMFS)
* @param[in] destFileName             - a destination file name (in local FS)
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on srcFileName opening error
* @retval GT_CREATE_ERROR          - on destFileName creating error
* @retval GT_FAIL                  - on srcFileName reading error or destFileName writing error
*/
GT_STATUS appDemoCopyFileFromRamFs
(
    IN GT_CHAR_PTR srcFileName,
    IN GT_CHAR_PTR destFileName
)
{
    GT_STATUS rc = GT_OK;
#if (defined (ASIC_SIMULATION)) && (defined(CMD_LUA_CLI))
    char   buf[2048];
    int    fdSrc  = -1;
    FILE * fdDest = NULL;
    int    num;

    do
    {
        fdSrc = cmdFS.open(srcFileName, GEN_FS_O_RDONLY);
        if (fdSrc < 0)             /* no such file */
        {
            rc = GT_NOT_FOUND;
            break;
        }

        fdDest = fopen(destFileName, "w+");
        if (fdDest == NULL)             /* couldn't open file for writing */
        {
            rc = GT_CREATE_ERROR;
            break;
        }

        while ( (num = cmdFS.read(fdSrc, buf, sizeof(buf))) > 0)
        {
            if (fwrite(buf, 1, num, fdDest) < (GT_U32)num) /* file writing error */
            {
                rc = GT_FAIL;
                break;
            }
        }
        if (num < 0)             /* file reading error */
        {
            rc = GT_FAIL;
        }

    } while (0);

    /* cleanup */
    if (fdSrc >= 0)
    {
        cmdFS.close(fdSrc);
    }

    if (fdDest != NULL)
    {
        fclose(fdDest);
    }
#endif  /* ASIC_SIMULATION */
    return rc;
}

/**
* @internal appDemoOsLogThreadModeSet function
* @endinternal
*
* @brief   Function for setting the thread mode of cpss log
*
* @param[in] mode                     -    to be set.
* @param[in] tid                      -   the task (thread) id. Relevant for mode =
*                                      APP_DEMO_CPSS_LOG_TASK_SINGLE_E only
* @param[in] tidPtr                   - an adress the task (thread) id should be taken from.
*                                      Relevant for mode = APP_DEMO_CPSS_LOG_TASK_REF_E only
*                                       None.
*/
GT_VOID appDemoOsLogThreadModeSet
(
    IN    APP_DEMO_CPSS_LOG_TASK_MODE_ENT mode,
    IN    GT_U32                          tid,
    IN    GT_U32                         *tidPtr
)
{
    prvOsLogThreadMode = mode;
    if (mode == APP_DEMO_CPSS_LOG_TASK_SINGLE_E)
    {
        prvOsLogTaskId = tid;
        prvOsLogTaskIdPtr = &prvOsLogTaskId;
    }
    else if (mode == APP_DEMO_CPSS_LOG_TASK_REF_E)
    {
        prvOsLogTaskIdPtr = tidPtr;
    }

}


/**
* @internal appDemoOsLogMainUtOnly function
* @endinternal
*
* @brief   Enables CPSS API logging for main UT task(s)
*/
GT_STATUS appDemoOsLogMainUtOnly
(
    GT_VOID
)
{
#ifdef  INCLUDE_UTF
    appDemoOsLogThreadModeSet(APP_DEMO_CPSS_LOG_TASK_REF_E, 0, utfTestTaskIdAddrGet());
#endif  /* INCLUDE_UTF */
    return GT_OK;
}

/**
* @internal appDemoLogUtModeEnable function
* @endinternal
*
* @brief   Enable CPSS API logging (with redirecting an output to the file) for
*         API calls from main UT's only and set some other useful options
* @param[in] logFileName              - a file name where log  will be stored.
*                                       GT_OK.
*/
GT_STATUS appDemoLogUtModeEnable
(
    IN GT_CHAR_PTR logFileName
)
{
#ifdef CPSS_LOG_ENABLE
    GT_STATUS rc = GT_OK;

    /* enable a CPSS API logging for all libs */
    cpssLogEnableSet(GT_TRUE);
    rc = cpssLogLibEnableSet(CPSS_LOG_LIB_ALL_E, CPSS_LOG_TYPE_ALL_E, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* log main UT API calls only */
    appDemoOsLogMainUtOnly();

    /* add prefix to pointer's to distinguish from them usual hex values */
    cpssLogPointerFormatSet(CPSS_LOG_POINTER_FORMAT_PREFIX_E);

    /* don't log line number */
    cpssLogLineNumberEnableSet(GT_FALSE);

    /* enable some API calls skipping to reduce the log size */
#ifdef  INCLUDE_UTF
    prvUtfReduceLogSizeFlagSet(GT_TRUE);
#endif  /* INCLUDE_UTF */

    /* log into the file on local file system */
    rc = appDemoOsLogModeSet(APP_DEMO_CPSS_LOG_MODE_LOCAL_FILE_E, logFileName);
    if (rc != GT_OK)
    {
        return rc;
    }
#endif /* CPSS_LOG_ENABLE */

    return GT_OK;
}

/**
* @internal skipCoreDevicesOnThePex function
* @endinternal
*
* @brief   function to skip devices on the PCI/PEX , that are 'cores' of the device.
*         the Lion/Lion2/Hooper recognized on the PEX as the number of the cores.
* @param[in] numOfCoresToSkip         - the number of PEX devices to skip.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS skipCoreDevicesOnThePex(
    IN GT_U32 numOfCoresToSkip
)
{
    GT_STATUS rc;
    GT_PCI_INFO pciInfo;
    GT_U32  numOfCoresSkipped = 0;
    GT_U32  tmpDevId;

    while(numOfCoresToSkip > numOfCoresSkipped)
    {
        rc = gtPresteraGetPciDev(GT_FALSE, &pciInfo);
        if(rc != GT_OK)
        {
            cpssOsPrintf("Error: skipped only[%d] cores (needed to skip [%d] cores) \n",
                numOfCoresSkipped,
                numOfCoresToSkip);
            /* should not get error */
            return rc;
        }

        tmpDevId = (pciInfo.pciDevVendorId.devId << 16) |
                   (pciInfo.pciDevVendorId.vendorId/*0x11ab*/);

        numOfCoresSkipped++;

        cpssOsPrintf("(skipped core[%d]) Recognized [0x%8.8x] device on the PCI/PEX \n",
            numOfCoresSkipped,
            tmpDevId);
    }

    return GT_OK;
}

/**
* @internal complex_cpssInitSystem_autoParamsGet function
* @endinternal
*
* @brief   allow to update the parameters in case of complex multiple devices.
*         supports Complex boards:
*         a. BC2 + Lion22 - 31,2,0
*         b. BC2 + Hooper2 - 31,3,0
*         c. BC26 - 30,1,0
*         d. Pipe4 - 34,1,0
* @param[in] firstDevId               - the first device that was recognized on the PCI/PEX (on the board).
* @param[in] firstDevId_numOfCoresOnPex - the Lion/Lion2/Hooper recognized on the PEX as
*                                      the number of the cores
* @param[in] boardIdxPtr              - (pointer to) boardIdx    , according to 'first device'
* @param[in] boardRevIdPtr            - (pointer to) boardRevId  , according to 'first device'
*
* @param[out] boardIdxPtr              - (pointer to) updated boardIdx   , according to 'all devices'
* @param[out] boardRevIdPtr            - (pointer to) updated boardRevId , according to 'all devices'
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS complex_cpssInitSystem_autoParamsGet
(
    IN  GT_U32  firstDevId,
    IN  GT_U32  firstDevId_numOfCoresOnPex,
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr
)
{
    GT_STATUS rc;
    GT_PCI_INFO pciInfo;

    GT_U32  numOfPp = 1;
    GT_U32  tmpDevId,secondaryDevId = 0;
    GT_U32  numOfSecondaryPp = 0;
    CPSS_PP_FAMILY_TYPE_ENT  primaryFamily  ;
    CPSS_PP_FAMILY_TYPE_ENT  secondaryFamily;
    GT_U32  isHooper = 0;/* when 1 hooper exists (not lion2)*/
    GT_U32  numOfCoresOnPex = 0;/* the Lion/Lion2/Hooper recognized on the PEX as
                               the number of the cores */

    numOfCoresOnPex = firstDevId_numOfCoresOnPex;
    if(numOfCoresOnPex >= 2)
    {
        /* skip redundant cores */
        rc = skipCoreDevicesOnThePex(numOfCoresOnPex-1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    while(1)
    {
        rc = gtPresteraGetPciDev(GT_FALSE, &pciInfo);
        if(rc != GT_OK)
        {
            /* no more devices on the PCI/PEX*/
            break;
        }

        if(numOfPp == 1)
        {
            cpssOsPrintf("Recognized [0x%8.8x] device on the PCI/PEX \n",
                firstDevId);
        }

        /* another Marvell PP recognized on the PEX */
        numOfPp++;

        tmpDevId = (pciInfo.pciDevVendorId.devId << 16) |
                   (pciInfo.pciDevVendorId.vendorId/*0x11ab*/);

        cpssOsPrintf("Recognized [0x%8.8x] device on the PCI/PEX \n",
            tmpDevId);
        if(tmpDevId != firstDevId)
        {
            secondaryDevId = tmpDevId;
            numOfSecondaryPp ++;
        }

        /* handle 'multiple-cores' on the PCI/PEX */
        switch(tmpDevId)
        {
            case CPSS_LION2_PORT_GROUPS_01234567_DEVICES_CASES_MAC:
                numOfCoresOnPex = 8;
                break;
            case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
                numOfCoresOnPex = 4;
                break;
            default:
                numOfCoresOnPex = 0;
                break;
        }

        if(numOfCoresOnPex >= 2)
        {
            /* skip redundant cores */
            rc = skipCoreDevicesOnThePex(numOfCoresOnPex-1);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if(numOfPp == 1)
    {
        /* no more devices except for the 'first device' */
        /* nothing to update in the board parameters */
        return GT_OK;
    }

    cpssOsPrintf("Recognized total [%d] devices on the PCI/PEX \n",
        numOfPp);

    if(numOfSecondaryPp == 0)
    {
        /* we have multiple-devices of the same type */

        /* usually the multiple devices are configured by the same 'cpssInitSystem'
           as the single device .*/
        /* currently only '6*BC2' needs deferent parameters than the 'single' device .*/

        switch(firstDevId)
        {
            case CPSS_BOBCAT2_ALL_DEVICES_CASES_MAC:
                if(numOfPp == 6)
                {
                    cpssOsPrintf("Recognized 'complex' board of SIX BobCat2 \n");
                    *boardIdxPtr   = 30;
                    *boardRevIdPtr =  1;
                }
                break;
            case CPSS_PIPE_ALL_DEVICES_CASES_MAC:
                if(numOfPp == 4)
                {
                    cpssOsPrintf("Recognized 'complex' board of 4 * Pipe \n");
                    *boardIdxPtr   = 34;
                    *boardRevIdPtr =  1;
                }
                break;
            default:
                break;
        }

        return GT_OK;
    }

    cpssOsPrintf("Recognized total [%d] secondary devices (out of [%d] devices) on the PCI/PEX \n",
        numOfSecondaryPp,
        numOfPp);

    switch(firstDevId)
    {
        case CPSS_BOBCAT2_ALL_DEVICES_CASES_MAC:
            primaryFamily = CPSS_PP_FAMILY_DXCH_BOBCAT2_E;
            break;
        case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
            isHooper = 1;
            /* fall through */
        case CPSS_LION2_PORT_GROUPS_01234567_DEVICES_CASES_MAC:
            primaryFamily = CPSS_PP_FAMILY_DXCH_LION2_E;
            break;
        default:
            primaryFamily = CPSS_PP_FAMILY_LAST_E;
            break;
    }

    switch(secondaryDevId)
    {
        case CPSS_BOBCAT2_ALL_DEVICES_CASES_MAC:
            secondaryFamily = CPSS_PP_FAMILY_DXCH_BOBCAT2_E;
            break;
        case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
            isHooper = 1;
            /* fall through */
        case CPSS_LION2_PORT_GROUPS_01234567_DEVICES_CASES_MAC:
            secondaryFamily = CPSS_PP_FAMILY_DXCH_LION2_E;
            break;
        default:
            secondaryFamily = CPSS_PP_FAMILY_LAST_E;
            break;
    }

    if((numOfPp == 3) &&
       ((secondaryFamily == CPSS_PP_FAMILY_DXCH_LION2_E && primaryFamily   == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) ||
        (primaryFamily   == CPSS_PP_FAMILY_DXCH_LION2_E && secondaryFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)))
    {
        *boardIdxPtr   = 31;
        /* boardRevId == 1: two Lion2 */
        /* boardRevId == 2: two Lion2 and one Bobcat2 */
        /* boardRevId == 3: two Hoopers and one Bobcat2 */
        /* boardRevId == 4: one Hooper  and one Bobcat2 */
        if(isHooper)
        {
            cpssOsPrintf("Recognized 'complex' board of two Hoopers and one Bobcat2 \n");
            *boardRevIdPtr =  3;
        }
        else
        {
            cpssOsPrintf("Recognized 'complex' board of two Lion2 and one Bobcat2 \n");
            *boardRevIdPtr =  2;
        }
    }


    return GT_OK;
}


/**
* @internal autoInitSystem function
* @endinternal
*
* @brief   This is the 'auto' board initialization function.
*         the function gets from the PEX the device-ID convert to 'family' and
*         decide the cpssInitSystem parameters.
*         It supports next:
*         1. PCI/PEX boards , it reads devId of device(s) from the PCI/PEX.
*         2. According to devId , call one of the next cpssInitSystem:
*         a. Bc2,bobk,Aldrin,Bc3,Aldrin2 : 29,1,0
*         b. AC3x : 32,1,0
*         c. Falcon : 35,1,0
*         d. Lion2,Hooper : 27,1,0
*         e. xCat3 : 19,2,0
*         f. Pipe : 33,1,0
*         3. Complex boards:
*         a. BC2 + Lion22 - 31,2,0
*         b. BC2 + Hooper2 - 31,3,0
*         c. BC26 - 30,1,0
*         d. Pipe4 - 34,1,0
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS autoInitSystem(void)
{
    GT_STATUS rc;
    GT_PCI_INFO pciInfo;
    GT_U32      devId;
    GT_U32  boardIdx;
    GT_U32  boardRevId = 1;
    GT_U32  reloadEeprom = 0;
    GT_U32  numOfCoresOnPex = 0;/* the Lion/Lion2/Hooper recognized on the PEX as
                               the number of the cores */

    /* In our case we want to find just one prestera device on PCI bus*/
    rc = gtPresteraGetPciDev(GT_TRUE, &pciInfo);
    if(rc != GT_OK)
    {
        osPrintf("autoInitSystem : Could not find Prestera device on PCI bus!\n");
        return GT_NOT_FOUND;
    }

    devId = (pciInfo.pciDevVendorId.devId << 16) |
           (pciInfo.pciDevVendorId.vendorId/*0x11ab*/);

    osPrintf("autoInitSystem : First device detected on the PEX devId[0x%4.4x] \n",
        pciInfo.pciDevVendorId.devId);
    switch(devId)
    {
        case CPSS_PIPE_ALL_DEVICES_CASES_MAC:
            boardIdx = 33;
            break;
        case CPSS_FALCON_ON_PEX_ALL_DEVICES_CASES_MAC:
            boardIdx = 35;
            break;
        case CPSS_BOBCAT2_ALL_DEVICES_CASES_MAC:
        case CPSS_BOBK_ALL_DEVICES_CASES_MAC:
        case CPSS_ALDRIN_DEVICES_CASES_MAC:
        case CPSS_BOBCAT3_ALL_DEVICES_CASES_MAC:
        case CPSS_ARMSTRONG_ALL_DEVICES_CASES_MAC:
        case CPSS_ALDRIN2_ALL_DEVICES_CASES_MAC:
            boardIdx = 29;
            break;
        case CPSS_XCAT3_ALL_DEVICES_CASES_MAC:
            boardIdx   = 19;
            boardRevId = 2;
            break;
        case CPSS_LION2_PORT_GROUPS_01234567_DEVICES_CASES_MAC:
        case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
            boardIdx   = 27;
            break;
        default:
            osPrintf("devId[0x%x] : not supported by the 'autoInitSystem' \n",
                devId);
            return GT_NOT_SUPPORTED;
    }

    switch(devId)
    {
        /* xCat3x (previously recognized as 'Aldrin') */
        case  CPSS_98DX3255_CNS:
        case  CPSS_98DX3256_CNS:
        case  CPSS_98DX3257_CNS:
        case  CPSS_98DX3258_CNS:
        case  CPSS_98DX3259_CNS:
            boardIdx = 32;
            break;
        default:
            break;
    }

    /* handle 'multiple-cores' on the PCI/PEX */
    switch(devId)
    {
        case CPSS_LION2_PORT_GROUPS_01234567_DEVICES_CASES_MAC:
            numOfCoresOnPex = 8;
            break;
        case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
            numOfCoresOnPex = 4;
            break;
        default:
            break;
    }


    /* allow to update the parameters in case of complex multiple devices */
    /* like : 2*Lion2 + BC2 */
    /* like : 6*BC2 */
    rc = complex_cpssInitSystem_autoParamsGet(devId,numOfCoresOnPex,&boardIdx,&boardRevId);
    if(rc != GT_OK)
    {
        osPrintf("failed on 'complex' board , not get proper cpssInitSystem(...) parameters \n");
        return rc;
    }

    osPrintf("Calling : cpssInitSystem(%d,%d,%d)\n",boardIdx,boardRevId,reloadEeprom);

    return cpssInitSystem(boardIdx,boardRevId,reloadEeprom);
}


/**************DX_FAMILY******/
#ifdef CHX_FAMILY
/****************************/


/*************************************************************/
/*          HIR FEATURE-BEGIN                                */
/*        APIs related to PP Insert / Remove                 */
/************************************************************/


/**
* @internal appDemoDeviceTrafficEnable function
* @endinternal
*
* @brief   Enables all ports on PP.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
/* All functions HIR will be compiled only if they belong to DX_FAMILY */

static GT_STATUS appDemoDeviceTrafficEnable
(
    GT_U8   devIdx
)
{
    GT_STATUS                   rc;             /* return code                 */
    APP_DEMO_SYS_CONFIG_FUNCS*  sysCfgFuncs;    /* pointer to System Functions */
    GT_U8                       dev;            /* device number               */

    /* Enable all devices and all ports */
        /* get init parameters from appdemo init array */
    sysCfgFuncs = &appDemoPpConfigList[devIdx].sysConfigFuncs;
    dev = appDemoPpConfigList[devIdx].devNum;

    if(sysCfgFuncs->cpssTrafficEnable)
    {
        /* Enable traffic for given device */
        if(GT_FALSE == appDemoTrafficEnableDisableMode)
        {
            rc = sysCfgFuncs->cpssTrafficEnable(dev);

            CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssTrafficEnable", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssInitDevice_afterBasicConfig function
* @endinternal
*
* @brief   This 'after basic config' part of device initialization function after Pp insertion.
* @param[in] devIdx                - Device index.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS cpssInitDevice_afterBasicConfig
(
    GT_U8 devIdx
)
{

 GT_STATUS   rc=GT_FAIL;


     GT_STATUS   stat;
    /* define parameters to keep code as less impact as possible */

    GT_U32  boardRevId = storedBoardRevId;

    GT_U8   numOfPp   = storedNumOfPp;       /* Number of Pp's in system.                */
    GT_U8   numOfFa   = storedNumOfFa;       /* Number of Fa's in system.                */
    GT_U8   numOfXbar = storedNumOfXbar;     /* Number of Xbar's in system.              */
    GT_U32  value     = 0;
    GT_U32  portNum;
    GT_U32  devNum;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    devNum = appDemoPpConfigList[devIdx].devNum;
    rc  = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
          return rc;
    }


    rc = appDemoDeviceEventHandlerPreInit(devIdx);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDeviceEventHandlerPreInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*****************************************************************/
    /* Logic phase initialization                                    */
    /*****************************************************************/
    rc = appDemoDeviceLogicalInit(devIdx, (GT_U8)boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDeviceLogicalInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*****************************************************************/
    /* General phase initialization                                  */
    /*****************************************************************/
    rc = appDemoDeviceGeneralInit(devIdx, (GT_U8)boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDeviceGeneralInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(appDemoOnDistributedSimAsicSide == 0)
    {
        /*****************************************************************/
        /* Perform board specific settings after logical phase           */
        /*****************************************************************/
        if (boardCfgFuncs.deviceAfterInitConfig != NULL)
        {
            rc = boardCfgFuncs.deviceAfterInitConfig(devIdx, (GT_U8)boardRevId);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardAfterInitConfig", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        stat=appDemoDbEntryGet("initSystemWithoutInterrupts", &value);
        if ((numOfPp + numOfFa + numOfXbar) != 0       &&
            (appDemoBypassEventInitialize == GT_FALSE) &&
            ( system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E ) &&
            ((stat == GT_NO_SUCH) || ((stat == GT_OK) && (value == 0))))
        {

            /* spawn the user event handler processes */
            rc = appDemoDeviceEventRequestDrvnModeInit(devIdx);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDeviceEventRequestDrvnModeInit", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* Enable all PP devices and all ports */
    rc = appDemoDeviceTrafficEnable(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Link up the ports */
    for (portNum = 0; portNum < (appDemoPpConfigList[devIdx].maxPortNumber); portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        rc = cpssDxChPortForceLinkDownEnableSet(devNum, portNum, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }


    for (portNum = 0; portNum < (appDemoPpConfigList[devIdx].maxPortNumber); portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
        /* Enable PP-CPU Traffic */
        rc = cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Enable sending AU messages to CPU */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        GT_U32  regAddr;
        regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 30, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    for(portNum = 0 ; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        /* Enable forwarding NA message to CPU */
        rc =  cpssDxChBrgFdbNaToCpuPerPortSet(devNum,portNum,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Enable new address message Storm Prevention */
        rc =  cpssDxChBrgFdbNaStormPreventSet(devNum,portNum,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = localUtfHirInit(devIdx);

    /*****************************************************************/
    /* External modules initializations                              */
    /*****************************************************************/


    return rc; /* will return GT_FAIL if invoked in NON DX mode */
}


/**
* @internal appDemoPpConfigListReset function
* @endinternal
*
* @brief Initialize the PP array index with default parameters
*
* @param[in] devIdx     - device index (out of 128 devices)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/

static GT_STATUS appDemoPpConfigListReset
(
    IN  GT_U8   devIdx
)
{
    osMemSet(&appDemoPpConfigList[devIdx], 0, sizeof(appDemoPpConfigList[devIdx]));

    appDemoPpConfigList[devIdx].ppPhase1Done = GT_FALSE;
    appDemoPpConfigList[devIdx].ppPhase2Done = GT_FALSE;
    appDemoPpConfigList[devIdx].ppLogicalInitDone = GT_FALSE;
    appDemoPpConfigList[devIdx].ppGeneralInitDone = GT_FALSE;
    appDemoPpConfigList[devIdx].valid = GT_FALSE;

    /* default value for cpu tx/rx mode */
    appDemoPpConfigList[devIdx].cpuPortMode = CPSS_NET_CPU_PORT_MODE_SDMA_E;
    appDemoPpConfigList[devIdx].allocMethod = CPSS_RX_BUFF_STATIC_ALLOC_E;

    /* cascading information */
    appDemoPpConfigList[devIdx].numberOfCscdTrunks = 0;
    appDemoPpConfigList[devIdx].numberOfCscdPorts = 0;
    appDemoPpConfigList[devIdx].numberOfCscdTargetDevs = 0;
    appDemoPpConfigList[devIdx].numberOf10GPortsToConfigure = 0;
    appDemoPpConfigList[devIdx].internal10GPortConfigFuncPtr = NULL;
    appDemoPpConfigList[devIdx].internalCscdPortConfigFuncPtr = NULL;

    return GT_OK;
}

static GT_STATUS appDemoPpConfigSetValid
(
    IN  GT_U8   devIdx
)
{
    appDemoPpConfigList[devIdx].valid = GT_TRUE;
    return GT_OK;
}

/**
* @internal appDemoDevicePhase1Init function
* @endinternal
*
* @brief   Perform phase1 initialization for device.
* @param[in] devIdx                   - Device index.
* @param[in] boardRevId               - Board revision ID.
*
* @retval GT_OK                       - on success,
* @retval GT_FAIL                     - otherwise.
*/
static GT_STATUS appDemoDevicePhase1Init
(
    GT_U32  devIdx,
    GT_U32  boardRevId
)
{
    GT_STATUS rc;
    CPSS_PP_PHASE1_INIT_PARAMS      corePpPhase1Params;     /* Phase 1 PP params                            */
    APP_DEMO_SYS_CONFIG_FUNCS*      sysCfgFuncs;
    CPSS_PP_DEVICE_TYPE             ppDevType;              /* CPSS Pp device type.                         */
    CPSS_REG_VALUE_INFO_STC         *regCfgList;            /* Config functions for this board              */
    GT_U32                          regCfgListSize;         /* Number of config functions for this board    */
    GT_U32                          value = 0;/*value from the DB*/

    /* Get PP config of this device */
    rc = boardCfgFuncs.boardGetPpPh1Params((GT_U8)boardRevId,
                                            devIdx,
                                            &corePpPhase1Params);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardGetPpPh1Params", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    value = 0;
    if ((appDemoDbEntryGet("dontOverrideSip5DefaultPciChannelType", &value) != GT_OK)
        || (value == 0))
    {
        /* Override SIP5 devices Default PCI compatible bus to use */
        /* 8 Address Completion Region mode                        */
        if ((appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) ||
            (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
            (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E) ||
            (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
            (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E))
        {
            corePpPhase1Params.mngInterfaceType = CPSS_CHANNEL_PEX_MBUS_E;
            /* Address Completion Region 1 - for Interrupt Handling    */
            corePpPhase1Params.isrAddrCompletionRegionsBmp = 0x02;
            /* Address Completion Regions 2,3,4,5 - for Application    */
            corePpPhase1Params.appAddrCompletionRegionsBmp = 0x3C;
            /* Address Completion Regions 6,7 - reserved for other CPU */
        }
    }

    /* override PP phase 1 parameters according to app demo database */
    rc = appDemoUpdatePpPhase1Params(&corePpPhase1Params);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoUpdatePpPhase1Params", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Update PP config list device number */
    appDemoPpConfigList[devIdx].devNum   = corePpPhase1Params.devNum;
    appDemoPpConfigList[devIdx].valid    = GT_TRUE;

    /* get family type to understand which API should be used below */
    /* There are two type of API here: EX and DX                    */
    rc = appDemoSysConfigFuncsGet(corePpPhase1Params.deviceId,
                                    &appDemoPpConfigList[devIdx].sysConfigFuncs,
                                    &appDemoPpConfigList[devIdx].apiSupportedBmp);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSysConfigFuncsGet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    if(appDemoSysConfigFuncsExtentionGetPtr)
    {
        rc = appDemoSysConfigFuncsExtentionGetPtr(corePpPhase1Params.deviceId,
                                    &appDemoPpConfigList[devIdx].sysConfigFuncs,
                                    &appDemoPpConfigList[devIdx].apiSupportedBmp);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSysConfigFuncsExtentionGetPtr", rc);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    sysCfgFuncs = &appDemoPpConfigList[devIdx].sysConfigFuncs;

    /* check if debug device id was set */
    if (useDebugDeviceId == GT_TRUE)
    {
        /* write device id to internal DB */
        rc = prvCpssDrvDebugDeviceIdSet(appDemoPpConfigList[devIdx].devNum,
                                        deviceIdDebug[appDemoPpConfigList[devIdx].devNum]);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDrvDebugDeviceIdSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (appDemoPrePhase1Init == GT_TRUE)
    {
        rc = prvCpssPrePhase1PpInit(PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_NO_PP_INIT_E);
        if (rc != GT_OK)
        {
            return rc;
        }

    }
    /* Do HW phase 1 */
    rc = sysCfgFuncs->cpssHwPpPhase1Init(appDemoPpConfigList[devIdx].devNum,
                                            &corePpPhase1Params,
                                            &ppDevType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssHwPpPhase1Init", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Update PP config list element of device */
    appDemoPpConfigList[devIdx].ppPhase1Done = GT_TRUE;
    appDemoPpConfigList[devIdx].deviceId = ppDevType;
    appDemoPpConfigList[devIdx].maxPortNumber = PRV_CPSS_PP_MAC(appDemoPpConfigList[devIdx].devNum)->numOfPorts;

    if(PRV_CPSS_PP_MAC(appDemoPpConfigList[devIdx].devNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E &&
        PRV_CPSS_PP_MAC(appDemoPpConfigList[devIdx].devNum)->revision > 0 )
    {
        /*Puma3, starting revision B0*/
        appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_TRUE;
    }
    else if (PRV_CPSS_SIP_5_CHECK_MAC(appDemoPpConfigList[devIdx].devNum))
    {
        appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_TRUE;
    }
#ifdef ASIC_SIMULATION
    else    /* allow simulation to test it , without HW implications ... yet */
    if(PRV_CPSS_PP_MAC(appDemoPpConfigList[devIdx].devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_TRUE;
    }
    else
    if(PRV_CPSS_PP_MAC(appDemoPpConfigList[devIdx].devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_TRUE;
    }
#endif /*ASIC_SIMULATION*/
    else
    {
        appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_FALSE;
    }

    #ifdef GM_USED
    /* the GM not supports the 'soft reset' so we can not support system with it */
    appDemoPpConfigList[devIdx].devSupportSystemReset_HwSoftReset  = GT_FALSE;
    #endif /*GM_USED*/


    /* If app demo inits in reg defaults mode skip the rest of the loop */
    if (appDemoInitRegDefaults == GT_TRUE)
        return GT_OK;

    /* Get list of registers to be configured.  */
    if (boardCfgFuncs.boardGetPpRegCfgList == NULL)
    {
        /* if there is no board-specific function, call the common one */
        rc = appDemoGetPpRegCfgList(ppDevType,
                                    gIsB2bSystem,
                                    &regCfgList,
                                    &regCfgListSize);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoGetPpRegCfgList", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        rc = boardCfgFuncs.boardGetPpRegCfgList(boardRevId,
                                                    appDemoPpConfigList[devIdx].devNum,
                                                    &regCfgList,
                                                    &regCfgListSize);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetPpRegCfgList", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (appDemoPrePhase1Init == GT_FALSE)
    {
        if(sysCfgFuncs->cpssHwPpStartInit)
        {
            /* Set PP's registers */
            rc = sysCfgFuncs->cpssHwPpStartInit(appDemoPpConfigList[devIdx].devNum,
                                                gReloadEeprom,
                                                regCfgList,
                                                regCfgListSize);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssHwPpStartInit", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

     /* If app demo inits in reg defaults mode return */
    if (appDemoInitRegDefaults == GT_TRUE)
        return GT_OK;

    return GT_OK;
}

GT_STATUS appDemoUpdateBoardConfig
(
    GT_U32 boardRevId
)
{
    GT_STATUS   rc;
    GT_U8       numOfPp;        /* Number of Pp's in system.                */
    GT_U8       numOfFa;        /* Number of Fa's in system.                */
    GT_U8       numOfXbar;      /* Number of Xbar's in system.              */
    /* Get board information */
   rc = boardCfgFuncs.boardGetInfo((GT_U8)boardRevId,
                                    &numOfPp,
                                    &numOfFa,
                                    &numOfXbar,
                                    &gIsB2bSystem);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardGetInfo", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* for fasten the "all PPs loops */
    appDemoPpConfigDevAmount = numOfPp;
    storedNumOfPp =    numOfPp;
    storedNumOfFa =    numOfFa;
    storedNumOfXbar =  numOfXbar;

    return rc;
}

/**
* @internal appDemoBoardPhase2Init function
* @endinternal
*
* @brief   Perform phase2 initialization for all devices (Pp, Fa, Xbar).
*
* @param[in] boardRevId               - Board revision Id.
* @param[in] boardCfgFuncs            - Board configuration functions
* @param[in] numOfDev                 - Number of devices in devList
* @param[in] numOfFa                  - Number of Fa's in system
* @param[in] numOfXbar                - Number of Xbar's in system
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDevicePhase2Init
(
    IN  GT_U8                   devIdx,
    IN  GT_U8                   boardRevId     /* Board revision Id             */
)
{
    GT_STATUS rc;
    CPSS_PP_PHASE2_INIT_PARAMS          cpssPpPhase2Params;     /* PP phase 2 params                        */
    APP_DEMO_SYS_CONFIG_FUNCS*  sysCfgFuncs;

    /* save value for use in appDemoAllocateDmaMem(...) */
    appDemoCpssCurrentDevIndex = devIdx;
    /* Get PP phase 2 params */
    appDemoCpssInitSkipHwReset = GT_TRUE;
    rc = boardCfgFuncs.boardGetPpPh2Params(boardRevId,
                                            appDemoPpConfigList[devIdx].devNum,
                                            &cpssPpPhase2Params);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.boardGetPpPh2Params", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* override PP phase 2 parameters according to app demo database */
    rc = appDemoUpdatePpPhase2Params(&cpssPpPhase2Params);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoUpdatePpPhase2Params", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(appDemoOnDistributedSimAsicSide)
    {
        /* no need to init the external driver */
        appDemoSysConfig.cpuEtherInfo.initFunc = NULL;
        /* we must not init the HW */
        cpssPpPhase2Params.netIfCfg.txDescBlock = NULL;
        cpssPpPhase2Params.netIfCfg.rxDescBlock = NULL;
        cpssPpPhase2Params.auqCfg.auDescBlock   = NULL;
    }

    appDemoPpConfigList[devIdx].oldDevNum = appDemoPpConfigList[devIdx].devNum;
    sysCfgFuncs = &appDemoPpConfigList[devIdx].sysConfigFuncs;

    if(sysCfgFuncs->cpssHwPpPhase2Init)
    {

        /* PP HW phase 2 Init */
        rc = sysCfgFuncs->cpssHwPpPhase2Init(appDemoPpConfigList[devIdx].oldDevNum,
                                                &cpssPpPhase2Params);

        CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssHwPpPhase2Init", rc);

        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Current PP init completed */
    appDemoPpConfigList[devIdx].ppPhase2Done = GT_TRUE;

    return GT_OK;
}

/**
* @internal appDemoDeviceLogicalInit function
* @endinternal
*
* @brief   Perform logical phase initialization for all devices (Pp, Fa, Xbar).
* @param[in] devIdx                   - Device index.
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDeviceLogicalInit
(
    IN GT_U8    devIdx,
    IN GT_U8    boardRevId
)
{
    GT_STATUS                   rc;
    CPSS_PP_CONFIG_INIT_STC     ppLogicalConfigParams;

    APP_DEMO_SYS_CONFIG_FUNCS*  sysCfgFuncs;

    /* update device config list */
    sysCfgFuncs = &appDemoPpConfigList[devIdx].sysConfigFuncs;


    /* Get PP logical init configuration */
    rc = boardCfgFuncs.boardGetPpLogInitParams(boardRevId,
                                                appDemoPpConfigList[devIdx].devNum,
                                                &ppLogicalConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetPpLogInitParams", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* override logical init configuration according to app demo database */
    rc = appDemoUpdatePpLogicalInitParams(&ppLogicalConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoUpdatePpLogicalInitParams", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Do CPSS logical init and fill PP_INFO structure */
    rc = sysCfgFuncs->cpssPpLogicalInit(appDemoPpConfigList[devIdx].devNum,
                                        &ppLogicalConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssPpLogicalInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
    appDemoPpConfigList[devIdx].ppLogicalInitDone = GT_TRUE;
    osMemCpy(&ppUserLogicalConfigParams[appDemoPpConfigList[devIdx].devNum],
                &ppLogicalConfigParams,
                sizeof(CPSS_PP_CONFIG_INIT_STC));

    return GT_OK;

} /* appDemoDeviceLogicalInit */

/**
* @internal appDemoDeviceGeneralInit function
* @endinternal
*
* @brief   Perform general initialization for all devices (Pp, Fa, Xbar).
*         This function includes initializations that common for all devices.
* @param[in] devIdx                   - Device index.
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This function must be called after logical init.
*
*/
GT_STATUS appDemoDeviceGeneralInit
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
)
{
    GT_STATUS                   rc = GT_OK;
    APP_DEMO_LIB_INIT_PARAMS    libInitParams;
    CPSS_PP_CONFIG_INIT_STC     ppLogicalConfigParams;

    APP_DEMO_SYS_CONFIG_FUNCS*  sysCfgFuncs;
    GT_U8                       dev;

    /* get init parameters from appdemo init array */
    sysCfgFuncs = &appDemoPpConfigList[devIdx].sysConfigFuncs;
    dev = appDemoPpConfigList[devIdx].devNum;

    /* Get PP logical init configuration */
    rc = boardCfgFuncs.boardGetPpLogInitParams(boardRevId,
                                                appDemoPpConfigList[devIdx].devNum,
                                                &ppLogicalConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetPpLogInitParams", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* override logical init configuration according to app demo database */
    rc = appDemoUpdatePpLogicalInitParams(&ppLogicalConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoUpdatePpLogicalInitParams", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Get library initialization parameters */
    rc = boardCfgFuncs.boardGetLibInitParams(boardRevId,
                                                appDemoPpConfigList[devIdx].devNum,
                                                &libInitParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs->boardGetLibInitParams", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* override library initialization parameters according to app demo database */
    rc = appDemoUpdateLibInitParams(&libInitParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoUpdateLibInitParams", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Initialize CPSS libraries accordingly to a given parameters */
    if(sysCfgFuncs->cpssLibrariesInit)
    {
        rc = sysCfgFuncs->cpssLibrariesInit(appDemoPpConfigList[devIdx].devNum,
                                            &libInitParams,
                                            &ppLogicalConfigParams);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssLibrariesInit", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(sysCfgFuncs->cpssPpGeneralInit)
    {
        /* Do CPSS general initialization for given device id */
        rc = sysCfgFuncs->cpssPpGeneralInit(dev);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("sysCfgFuncs->cpssPpGeneralInit", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    appDemoPpConfigList[devIdx].ppGeneralInitDone = GT_TRUE;

    return GT_OK;
}


/**
  * @cpssSystemBaseInit function
  *
  * @brief This will do general initialization independent of Pp
  *
  * @retval GT_OK                    - on success,
  * @retval GT_FAIL                  - otherwise.
  */
static GT_STATUS internal_cpssSystemBaseInit
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;

    if(sysGenGlobalInfo.cpssPpInitCalled == GT_TRUE)
    {
        cpssOsPrintf("cpssSystemBaseInit:Already Initialized\n");
        return rc;
    }
    rc = cpssPpInit();
    if(rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("cpssSystemBaseInit:Success\n");

    return rc;
} /* cpssSystemBaseInit */


/**
* @internal internal_cpssPpInsert function
* @endinternal
*
* @brief   Insert packet processor.
* @param[in] devidx         - Device Index.
* @param[in] pciBusNum      - PCI bus number.
* @param[in] pciDevNum      - PCI Device number.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on wrond device number,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS internal_cpssPpInsert
(
    IN  GT_U8  devIdx,
    IN  GT_U8  pciBusNum,
    IN  GT_U8  pciDevNum
)
{

    GT_BOARD_LIST_ELEMENT   *pBoardInfo;    /* Holds the board information   */
    GT_STATUS rc;
    GT_STATUS   pciStat;
    GT_U32  boardIdx=29;
    GT_U32  boardRevId=1;
    GT_U32  start_sec  = 0;
    GT_U32  start_nsec = 0;
    GT_U32  ph1_start_sec  = 0;
    GT_U32  ph1_start_nsec = 0;
    GT_U32  end_sec  = 0;
    GT_U32  end_nsec = 0;
    GT_U32  diff_sec  = 0;
    GT_U32  diff_nsec = 0;
    GT_U8   i=0; /* loop index */

    if (!isHirApp)
    {
        cpssOsPrintf("Hir feature is not enabled\n");
        cpssOsPrintf("To enable HIR feature, Please run app with -hir flag\n");
        return GT_FAIL;
    }


    rc = cpssOsTimeRT(&start_sec, &start_nsec);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* devNum should be 0-31 so this number will be used both as hw device number and cpss device number */
    if(devIdx < 0 || devIdx >= APP_DEMO_MAX_HIR_SUPPORTED_PP_CNS)
    {
        cpssOsPrintf("Cpss Dev Num - supported range is 0-31");
        return GT_BAD_PARAM;
    }

    /* Check if device was previously Inserted */
    if (appDemoPpConfigList[devIdx].valid == GT_TRUE)
    {
        cpssOsPrintf("PP [%d] Already Inserted, pciBusNum %d, pciDevNum %d\n", devIdx,
                     appDemoPpConfigList[devIdx].pciInfo.pciBusNum, appDemoPpConfigList[devIdx].pciInfo.pciIdSel);
        return GT_BAD_PARAM;
    }


    for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if (appDemoPpConfigList[i].valid == GT_TRUE)
        {
            if(appDemoPpConfigList[i].pciInfo.pciBusNum == pciBusNum &&
               appDemoPpConfigList[i].pciInfo.pciIdSel == pciDevNum)
            {
                cpssOsPrintf("PP [%d] Already Inserted, pciBusNum %d, pciDevNum %d\n", i, pciBusNum, pciDevNum);
                return GT_BAD_PARAM;
            }
        }
    }


    if(appDemoPpConfigDevAmount== 0)
    {
        pBoardInfo = &(boardsList[boardIdx - 1]);

        /* Check if board registerFunc is not NULL */
        if (NULL == pBoardInfo->registerFunc)
        {
            rc = GT_NOT_SUPPORTED;
            CPSS_ENABLER_DBG_TRACE_RC_MAC("This board type was not implemented (it is only 'place holder')", rc);
            return rc;
        }

        /* Make sure all pointers are NULL in case user won't init all pointers */
        osMemSet(&boardCfgFuncs, 0, sizeof(GT_BOARD_CONFIG_FUNCS));

        /* Get board specific functions accordingly to board rev. id */
        rc = pBoardInfo->registerFunc((GT_U8)boardRevId, &boardCfgFuncs);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("pBoardInfo->registerFunc", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Enable printing inside interrupt routine. */
        extDrvUartInit();

        /* Call to fatal_error initialization, use default fatal error call_back */
        rc = osFatalErrorInit((FATAL_FUNC_PTR)NULL);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Do initialization for device specific AppDemo before all phases */
    appDemoPpConfigListReset(devIdx);

    /* Get the PCI info specific device on  bus,dev with devIdx*/
    pciStat= appDemoHotInsRmvSysGetPciInfo(devIdx, pciBusNum, pciDevNum);
    if(pciStat == GT_OK)
    {
        storedNumOfPp = 1;
        appDemoPpConfigSetValid(devIdx);
    } else {
        if(pciStat == GT_NO_MORE)
        {
            cpssOsPrintf("Prestera Device on pciBusNum:%d Not Present\n",pciBusNum);
        }
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoHotInsRmvSysGetPciInfo", pciStat);
        return pciStat;
    }

    if((CPSS_PP_FAMILY_DXCH_ALDRIN_E != appDemoPpConfigList[devIdx].devFamily) &&
       (CPSS_PP_FAMILY_DXCH_ALDRIN2_E != appDemoPpConfigList[devIdx].devFamily))
    {
        appDemoPpConfigList[devIdx].valid = GT_FALSE;
        cpssOsPrintf("HIR feature not supported\n");
        return GT_NOT_SUPPORTED;
    }

    cpssInitSystemSet(boardIdx, boardRevId);

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&ph1_start_sec, &ph1_start_nsec);

    /*****************************************************************/
    /* HW phase 1 initialization                                     */
    /*****************************************************************/
    /* Device specific settings phase 1 */
    rc = appDemoDevicePhase1Init(devIdx, boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDevicePhase1Init",rc);
    if (rc != GT_OK)
    {
        goto EXIT_CLEANLY_INSERT_LBL;
    }

    /* Does board specific settings after phase 1 */
    if (boardCfgFuncs.deviceAfterPhase1 != NULL)
    {
        rc = boardCfgFuncs.deviceAfterPhase1(devIdx, (GT_U8)boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.deviceAfterPhase1", rc);
        if (rc != GT_OK)
        {
            goto EXIT_CLEANLY_INSERT_LBL;
        }
    }

    /* Sets value of appDemoHwDevNumOffset to Zero.
       This makes HW Dev Id same as Sw DevNum.     */
    appDemoHwDevNumOffsetSet(0);

    /*****************************************************************/
    /* HW phase 2 initialization                                     */
    /*****************************************************************/
    rc = appDemoDevicePhase2Init(devIdx, boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDevicePhase2Init",rc);
    if (rc != GT_OK)
    {
        goto EXIT_CLEANLY_INSERT_LBL;
    }

    if (boardCfgFuncs.deviceAfterPhase2 != NULL)
    {
        /* Device specific settings after phase 2 */
        rc = boardCfgFuncs.deviceAfterPhase2(devIdx, (GT_U8)boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.deviceAfterPhase2",rc);
        if (rc != GT_OK)
        {
            goto EXIT_CLEANLY_INSERT_LBL;
        }
    }

    /* Device specific continue to 'after basic config' */
    rc = cpssInitDevice_afterBasicConfig(devIdx);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssInitDevice_afterBasicConfig",rc);
    if (rc != GT_OK)
    {
        goto EXIT_CLEANLY_INSERT_LBL;
    }

    appDemoPpConfigDevAmount = appDemoPpConfigDevAmount + 1;

EXIT_CLEANLY_INSERT_LBL:
    if(rc != GT_OK)
    {

        rc = appDemoPpConfigListReset(devIdx);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoPpConfigListReset", rc);
        if (rc != GT_OK)
        {
            return GT_OK;
        }
    }

    rc = cpssOsTimeRT(&end_sec, &end_nsec);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;

    cpssOsPrintf("cpssPpInsert Time is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    if(end_nsec < ph1_start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - ph1_start_sec;
    diff_nsec = end_nsec - ph1_start_nsec;

    cpssOsPrintf("Time processing the cpssPpInsert (from 'phase1 init') is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    return GT_OK;

}

extern GT_STATUS appDemoDxChLpmDbReset();

/**
* @internal internal_cpssPpRemove function
* @endinternal
*
* @brief   Remove packet processor.
* @param[in] devNum                - Device number.
* @param[in] managedRemoval        - by default set to TRUE.
                                     Will be false in case of unmanaged removal of Device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS internal_cpssPpRemove
(
    IN GT_U8    devNum,
    IN  GT_BOOL managedRemoval
)
{

    GT_STATUS                           rc=GT_FAIL;
    GT_PHYSICAL_PORT_NUM                portNum;
    GT_U32                              DevId;
    GT_U32                              vTcamMngItr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMng;
    GT_U8                               vTcamDev;
    GT_UINTPTR                          slItr;
    GT_U32                              lpmDb=0;
    GT_U32                              trunkId;
    CPSS_TRUNK_MEMBER_STC               trunkMembersArray[CPSS_MAX_PORTS_NUM_CNS];
    GT_U32                              numOfMembers;
    GT_U32                              trunkMemberItr;
    GT_U32    start_sec  = 0;
    GT_U32    start_nsec = 0;
    GT_U32    end_sec  = 0;
    GT_U32    end_nsec = 0;
    GT_U32    diff_sec  = 0;
    GT_U32    diff_nsec = 0;
    GT_U32 boardIdx, boardRevId, reloadEeprom;

    if (isHirApp == GT_FALSE)
    {
        cpssOsPrintf("Hir feature is not enabled (This feature is applicable only on DX Family). Please run app with -hir flag\n");
        return GT_FAIL;
    }


    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* Check if device was previously inserted*/
    if (appDemoPpConfigList[devNum].valid == GT_FALSE)
    {
        cpssOsPrintf("PP [%d] Not Inserted\n",devNum);
        return GT_BAD_PARAM;
    }

    if( (appDemoPpConfigList[devNum].ppPhase1Done == GT_FALSE) ||
        (appDemoPpConfigList[devNum].ppPhase2Done == GT_FALSE) ||
        (appDemoPpConfigList[devNum].ppLogicalInitDone == GT_FALSE) ||
        (appDemoPpConfigList[devNum].ppGeneralInitDone == GT_FALSE) )
    {
        cpssOsPrintf("PP [%d] Not Initialized\n",devNum);
        cpssOsPrintf("Init flags: ppPhase1Done %d, ppPhase2Done %d,ppLogicalInitDone %d, ppGeneralInitDone %d\n",
                     appDemoPpConfigList[devNum].ppPhase1Done,appDemoPpConfigList[devNum].ppPhase2Done,
                     appDemoPpConfigList[devNum].ppLogicalInitDone,appDemoPpConfigList[devNum].ppGeneralInitDone);

        return GT_NOT_INITIALIZED;
    }

    DevId = (appDemoPpConfigList[devNum].pciInfo.pciDevVendorId.devId << 16) |
                    (appDemoPpConfigList[devNum].pciInfo.pciDevVendorId.vendorId/*0x11ab*/);

    if(managedRemoval == GT_TRUE)
    {
        cpssOsPrintf("Managed Removal of PP on PCI/PEX Bus [0x%8.8x] device [0x%8.8x] \n",appDemoPpConfigList[devNum].pciInfo.pciBusNum, DevId);
    } else {
        cpssOsPrintf("Unmanaged Removal of PP on PCI/PEX Bus [0x%8.8x] device [0x%8.8x] \n",appDemoPpConfigList[devNum].pciInfo.pciBusNum, DevId);
    }

    rc = cpssOsTimeRT(&start_sec, &start_nsec);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* incase of unmanaged Removal of device Skip HW disable steps */
    if(managedRemoval == GT_TRUE)
    {
        /* Disable interrupts : interrupt bits masked int cause registers*/
        rc = cpssPpInterruptsDisable(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPpInterruptsDisable", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Link down the ports */
        for (portNum = 0; portNum < (appDemoPpConfigList[devNum].maxPortNumber); portNum++)
        {
            CPSS_ENABLER_PORT_SKIP_CHECK(appDemoPpConfigList[devNum].devNum, portNum);
            /* Link down ports */
            rc = cpssDxChPortForceLinkDownEnableSet(devNum, portNum, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortForceLinkDownEnableSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
#ifndef ASIC_SIMULATION
            /*Make Link partner down*/
            rc= cpssDxChPortSerdesTxEnableSet(devNum, portNum, GT_FALSE);
            if (rc != GT_OK && rc != GT_NOT_SUPPORTED)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesTxEnableSet", rc);
                return rc;
            }
#endif
        }
        /* Disable CPU traffic */
        for (portNum = 0; portNum < (appDemoPpConfigList[devNum].maxPortNumber); portNum++)
        {
            CPSS_ENABLER_PORT_SKIP_CHECK(appDemoPpConfigList[devNum].devNum, portNum);
            /* Disable PP-CPU Traffic-egress */
            rc = cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChNstPortEgressFrwFilterSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* Disable PP-CPU Traffic-ingress */
            rc = cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChNstPortIngressFrwFilterSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            GT_U32  regAddr;
            regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 30, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        for(portNum = 0 ; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            rc =  cpssDxChBrgFdbNaToCpuPerPortSet(devNum,portNum,GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }

        }

    }


    /* Remove device from LPM DB*/
    slItr = 0;
    while ((rc = prvCpssDxChIpLpmDbIdGetNext(&lpmDb, &slItr)) == GT_OK)
    {
        rc = cpssDxChIpLpmDBDevsListRemove(lpmDb, &devNum, 1);
        if (rc != GT_OK && rc != GT_NOT_FOUND)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChIpLpmDBDevsListRemove", rc);
            return rc;
        }
    }

    /* Delete LPM DB while removing last device*/
    if((appDemoPpConfigDevAmount-1) == 0)
    {
        rc = appDemoDxChLpmDbReset();
        if (rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChLpmDbReset", rc);
            return rc;
        }
    }

    /* Remove Trunks from device */
    for (trunkId = 0; trunkId < (PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numberOfTrunks); trunkId++)
    {
        if (prvCpssGenericTrunkDbIsValid(devNum, trunkId) == GT_OK)
        {
            /* Remove enabled trunk members */
            numOfMembers = CPSS_MAX_PORTS_NUM_CNS;
            rc = cpssDxChTrunkDbEnabledMembersGet(devNum, trunkId, &numOfMembers, trunkMembersArray);
            if (rc != GT_OK)
            {
                return rc;
            }
            for (trunkMemberItr = 0; trunkMemberItr < numOfMembers; trunkMemberItr++)
            {
                rc = cpssDxChTrunkMemberRemove(devNum, trunkId, &trunkMembersArray[trunkMemberItr]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            /* Remove disabled trunk members */
            numOfMembers = CPSS_MAX_PORTS_NUM_CNS;
            rc = cpssDxChTrunkDbDisabledMembersGet(devNum, trunkId, &numOfMembers, trunkMembersArray);
            if (rc != GT_OK)
            {
                return rc;
            }
            for (trunkMemberItr = 0; trunkMemberItr < numOfMembers; trunkMemberItr++)
            {
                rc = cpssDxChTrunkMemberRemove(devNum, trunkId, &trunkMembersArray[trunkMemberItr]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    /* SW cleanup */
    for (vTcamMngItr = 0; vTcamMngItr < CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS; vTcamMngItr++)
    {
        vTcamMng = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngItr);
        if (vTcamMng != NULL)
        {
            rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMng, &vTcamDev);
            if (rc == GT_NO_MORE)
            {
                continue;
            }
            else if (rc != GT_OK)
            {
                return rc;
            }
            if (devNum == vTcamDev)
            {
                /* Remove device from default vTcam manager */
                rc = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngItr, &vTcamDev, 1);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }


    /* Masks unified events specific to device*/
    if(managedRemoval == GT_TRUE)
    {
        rc = appDemoDeviceEventMaskSet(devNum, CPSS_EVENT_MASK_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDeviceEventMaskSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Remove Interrupt Nodes */
    /*    rc = prvCpssDrvEvReqQRemoveDev(devNum);
          CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDrvEvReqQRemoveDev", rc);
          if (rc != GT_OK)
          {
          return rc;
          }  */

    /* Delete Event DB when last device is removed */
    if((appDemoPpConfigDevAmount-1) == 0)
    {
        rc = appDemoDeviceEventRequestDrvnModeReset(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDeviceEventRequestDrvnModeReset", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
#ifndef ASIC_SIMULATION
    rc = appDemoDeAllocateDmaMem(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDeAllocateDmaMem", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
#endif

    /* Disable All Skip Reset options ,exclude PEX */
    /* this Enable Skip Reset for PEX */
    rc = cpssDxChHwPpSoftResetSkipParamSet(devNum, CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E, GT_FALSE);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpSoftResetSkipParamSet", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChHwPpSoftResetTrigger(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpSoftResetTrigger", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCfgDevRemove(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCfgDevRemove", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

#ifdef ASIC_SIMULATION
    {
        GT_U32 k = 4000;     /* Wait for a limited time */

        /* Wait simulationSoftResetDone to be reset */
        while((simulationSoftResetDone == 0) && (k > 0))
        {
            osTimerWkAfter(5);
            k--;
        }
    }
#endif
    if (boardCfgFuncs.deviceCleanup != NULL)
    {
        rc = boardCfgFuncs.deviceCleanup(devNum, boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCfgFuncs.deviceCleanup", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = appDemoPpConfigListReset(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoPpConfigListReset", rc);
    if (rc != GT_OK)
    {
        return GT_OK;
    }

    appDemoPpConfigDevAmount=appDemoPpConfigDevAmount-1;

    rc = cpssOsTimeRT(&end_sec, &end_nsec);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;

    cpssOsPrintf("cpssPpRemove Time is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    return rc;


}

/**************DX_FAMILY******/
#endif
/*****************************/

/**
* @internal cpssPpShowDevices function
* @endinternal
*
* @brief   List all PP devices' PCI info.
*
*/

void cpssPpShowDevices ()
{
    GT_U8 devIdx;

    cpssOsPrintf("DevNum   VendorId  DeviceId  pciBus  pciDev  pciFunc\n");
    for(devIdx = 0; devIdx < PRV_CPSS_MAX_PP_DEVICES_CNS; devIdx++)
    {
        if (appDemoPpConfigList[devIdx].valid == GT_TRUE)
        {
            cpssOsPrintf("   %d     0x%04x    0x%04x      %d       %d       %d\n",
                         appDemoPpConfigList[devIdx].devNum,
                         appDemoPpConfigList[devIdx].pciInfo.pciDevVendorId.vendorId,
                         appDemoPpConfigList[devIdx].pciInfo.pciDevVendorId.devId,
                         appDemoPpConfigList[devIdx].pciInfo.pciBusNum,
                         appDemoPpConfigList[devIdx].pciInfo.pciIdSel,
                         appDemoPpConfigList[devIdx].pciInfo.funcNo);
        }
    }
}

/**
* @internal cpssLspci function
* @endinternal
*
* @brief   List all detected Marvell PCI devices.
*
*/

void cpssLspci (void)
{
    GT_PCI_INFO pciInfo;
    GT_BOOL isFirst = GT_TRUE;

    cpssOsPrintf("PciBusNum   PciDevNum   PciFuncNum  VendorId  DeviceId\n");

    while(GT_OK == gtPresteraGetPciDev(isFirst, &pciInfo))
    {
        isFirst = GT_FALSE;
        cpssOsPrintf("    %d           %d           %d        0x%04x    0x%04x\n",
                     pciInfo.pciBusNum, pciInfo.pciIdSel, pciInfo.funcNo,
                     pciInfo.pciDevVendorId.vendorId, pciInfo.pciDevVendorId.devId);
    }
}

/**
* @internal cpssPciRemove function
* @endinternal
*
* @brief  Remove specific PCI device.
*
*/

void cpssPciRemove (
    IN  GT_U8 pciBus,
    IN  GT_U8 pciDev
)
{
#if (defined(LINUX) && !defined(ASIC_SIMULATION))
    char fname[128];
    int fd;
    cpssOsSprintf(fname, "/sys/bus/pci/devices/%04x:%02x:%02x.%x/remove",
                  0, pciBus, pciDev, 0);
    if((fd = open(fname, O_WRONLY)) < 0)
    {
        cpssOsPrintf("Failed to open /sys/bus/pci/devices for write\n");
        return;
    }
    if(write(fd, "1\n",2) != 2)
    {
        close(fd);
        return;
    }
    close(fd);
#endif
}

/**
* @internal cpssPciRescan function
* @endinternal
*
* @brief  Rescan all PCI devices.
*
*/

void cpssPciRescan (void)
{
#if (defined(LINUX) && !defined(ASIC_SIMULATION))
    int fd;
    if((fd= open("/sys/bus/pci/rescan", O_WRONLY)) < 0)
    {
        cpssOsPrintf("Failed to open /sys/bus/pci/rescan for write\n");
        return;

    }
    if(write(fd, "1\n",2) != 2)
    {
        close(fd);
        return;
    }
    close(fd);
#endif
}



/**
* @internal cpssBaseInitSystem function
* @endinternal
  * @brief This will do general initialization independent of Pp
  *
  * @retval GT_OK                    - on success,
  * @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssSystemBaseInit
(
    GT_VOID
)
{
    GT_STATUS rc= GT_OK;

#ifndef CHX_FAMILY
    cpssOsPrintf ("Feature is applicable only for DX family of Devices\n");
    rc = GT_FAIL;
    return rc;
#else
    /* for DX FAMILY */
    if (!isHirApp)
    {
        cpssOsPrintf ("Not Running in HIR mode. Please run appDemo(Sim) with -hir flag\n");
        rc = GT_FAIL;
        return rc;
    }

    rc = internal_cpssSystemBaseInit();
    if(rc != GT_OK)
    {
        return rc;
    }
    return rc;
#endif
}


/**
* @internal cpssPpInsert function
* @endinternal
*
* @brief   Insert packet processor.
* @param[in] pciBusNum      - PCI bus number.
* @param[in] pciDevNum      - PCI Device number.
* @param[in] devNum         - cpss Logical Device number.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on wrond device number,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssPpInsert
(
    IN  GT_U8 pciBusNum,
    IN  GT_U8 pciDevNum,
    IN  GT_U8 devNum
)
{
    GT_STATUS rc;

#ifndef CHX_FAMILY
cpssOsPrintf ("Feature is applicable only for DX family of Devices\n");
rc = GT_FAIL;
return rc;
#else
     if (!isHirApp)
     {
         cpssOsPrintf ("Not Running in HIR mode. Please run appDemo(Sim) with -hir flag\n");
         rc = GT_FAIL;
         return rc;
     }


    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPpInsert);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssPpInsert(devNum, pciBusNum, pciDevNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
#endif

}

/**
* @internal cpssPpRemove function
* @endinternal
*
* @brief   Remove packet processor.
* @param[in] devNum         - cpss Logical Device number.
* @param[in] managedRemoval - by default set to TRUE.
                              Will be false in case of unmanaged removal of Device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssPpRemove
(
    IN  GT_U8 devNum,
    IN  GT_BOOL managedRemoval
)
{
    GT_STATUS rc;

#ifndef CHX_FAMILY
cpssOsPrintf ("Feature is applicable only for DX family of Devices\n");
rc = GT_FAIL;
return rc;
#else
      if (!isHirApp)
      {
          cpssOsPrintf ("Not Running in HIR mode. Please run appDemo(Sim) with -hir flag\n");
          rc = GT_FAIL;
          return rc;
     }


    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPpRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    rc = internal_cpssPpRemove(devNum,managedRemoval);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
#endif
}

