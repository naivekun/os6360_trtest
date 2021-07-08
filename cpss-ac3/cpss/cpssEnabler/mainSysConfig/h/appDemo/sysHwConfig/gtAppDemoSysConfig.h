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
* @file gtAppDemoSysConfig.h
*
* @brief System configuration and initialization control.
*
* @version   66
********************************************************************************
*/
#ifndef __gtAppDemoSysConfigh
#define __gtAppDemoSysConfigh


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <appDemo/os/appOs.h>
#include <appDemo/sysHwConfig/gtAppDemoPciConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSmiConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoTwsiConfig.h>

#include <cpss/common/cpssTypes.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h> /* this is not ok - just need for CPSS_PP_FAMILY_TYPE_ENT to compile*/

#include <cpss/common/cpssHwInit/cpssHwInit.h>
#ifdef CHX_FAMILY
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
/* for cascadig required */
#include <cpss/generic/cscd/cpssGenCscd.h>

#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>
#else
#include <appDemo/sysHwConfig/nonSwitchStubs.h>
#endif /*CHX_FAMILY*/


/* constant for FX950 family type */
#define APPDEMO_FX950_FAMILY_TYPE   0x95

/* constant for FX950 device ID and vendor ID */
#define APPDEMO_FX950_DEV_ID_AND_VENDOR_ID   CPSS_98FX950_CNS

#define APP_DEMO_CPSS_NO_PP_CONNECTED_CNS 0xFFFFFFFF

#define APP_DEMO_CPSS_MAX_NUMBER_OF_SRAMS_CNS      10

/* Use this constant to retrieve core clock value from HW.
   Currently supported only for DxCh and ExMxPm devices*/
#define APP_DEMO_CPSS_AUTO_DETECT_CORE_CLOCK_CNS 0xFFFFFFFF

/* 4 port-groups for multi-port-group device */
#define APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS 8

/* max number of LPM blocks on eArch architecture */
#define APP_DEMO_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS    20

/* User configured bitmap for SERDES power down */
extern GT_U32 appDemoDxChPowerDownBmp;

extern GT_U32   trainingTrace;/* option to disable the print in runtime*/
#define TRAINING_DBG_PRINT_MAC(x) if(trainingTrace) cpssOsPrintSync x

/* cpss Reset State for event handlers  */
extern GT_U32   eventRequestDrvnModeReset;

/* flag to indicate wthether this is normal or hir app */
extern GT_BOOL isHirApp;
/**
* @enum APPDEMO_SERDES_REF_CLOCK_ENT
 *
 * @brief Defines SERDES reference clock type.
*/
typedef enum{

    /** @brief 25 MHz external
     *  single ended reference clock.
     */
    APPDEMO_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,

    /** @brief 125 MHz external
     *  single ended reference clock.
     */
    APPDEMO_SERDES_REF_CLOCK_EXTERNAL_125_SINGLE_ENDED_E,

    /** @brief 125 MHz external
     *  differential reference clock.
     */
    APPDEMO_SERDES_REF_CLOCK_EXTERNAL_125_DIFF_E,

    /** @brief 156.25 MHz external
     *  single ended reference clock.
     */
    APPDEMO_SERDES_REF_CLOCK_EXTERNAL_156_25_SINGLE_ENDED_E,

    /** @brief 156.25 MHz external
     *  differential reference clock.
     */
    APPDEMO_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,

    /** 125 MHz internal reference clock. */
    APPDEMO_SERDES_REF_CLOCK_INTERNAL_125_E

} APPDEMO_SERDES_REF_CLOCK_ENT;

/**
* @enum APPDEMO_EXMXPM_EXT_MEMORY_LOGICAL_MODE_ENT
 *
 * @brief Defines external memory logical bank arrangement.
*/
typedef enum{

    /** @brief DRAM consists of one logical group of 8 banks.
     *  All 8 banks of each DRAM has the same information.
     */
    APPDEMO_EXMXPM_EXT_MEMORY_LOGICAL_MODE_8BANKS_E,

    /** @brief DRAM is divided for 2 logical groups of banks.
     *  Each group (4 banks) has different information and thus
     *  memory capacity is doubled.
     */
    APPDEMO_EXMXPM_EXT_MEMORY_LOGICAL_MODE_4BANKS_E

} APPDEMO_EXMXPM_EXT_MEMORY_LOGICAL_MODE_ENT;



/**
* @struct APPDEMO_EXMXPM_EXT_DRAM_STATIC_CFG
 *
 * @brief Defines external memory DRAM static configuration data
*/
typedef struct{

    /** ifNum */
    GT_U32 ifNum;

    /** pupNum */
    GT_U32 pupNum;

    /** data written to 0x1538 */
    GT_U32 rdSmplDel;

    /** data written to 0x153C */
    GT_U32 rdReadyDel;

    /** PUP reg 2 */
    GT_U32 readLevelData;

} APPDEMO_EXMXPM_EXT_DRAM_STATIC_CFG;

/**
* @enum APPDEMO_DRAM_FREQUENCY_ENT
 *
 * @brief Defines the different Dram frequencies that a dram can have.
*/
typedef enum{

    APPDEMO_DRAM_FREQ_667_MHZ_E,

    APPDEMO_DRAM_FREQ_800_MHZ_E

} APPDEMO_DRAM_FREQUENCY_ENT;





/**
* @struct CPSS_PP_PHASE1_INIT_PARAMS
 *
 * @brief This struct defines specific hardware parameters, those parameters are
 * determined according to the board specific hardware components.
*/
typedef struct{

    /** @brief Temporary device number to allow basic communication.
     *  hwInfo[]     - HW info data
     */
    GT_U8 devNum;

    CPSS_HW_INFO_STC hwInfo[APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS];

    /** @brief must be always set.
     *  For 'non-multi-port-groups' device should be set to 1
     */
    GT_U32 numOfPortGroups;

    /** @brief The unique Prestera Packet Processor device ID,
     *  as read from its PCI register during PCI scan phase.
     *  (Including the vendor Id).
     */
    GT_U32 deviceId;

    /** @brief The device revision number,
     *  as read from its PCI register during PCI scan phase.
     */
    GT_U32 revision;

    /** @brief The PP core clock in MHz.
     *  APP_DEMO_CPSS_AUTO_DETECT_CORE_CLOCK_CNS may be used for
     *  core clock auto detection.
     */
    GT_U32 coreClk;

    /** External flow dram configuration parameters. */
    CPSS_DRAM_CFG_STC flowDramCfg;

    /** The buffers dram configuration parameters. */
    CPSS_DRAM_CFG_STC bufferDramCfg;

    /** Buffer management configuration parameters. */
    CPSS_BUFF_MGMT_CFG_STC bufferMgmtCfg;

    /** determine the uplink connection status. */
    CPSS_PP_UPLINK_CONFIG_ENT uplinkCfg;

    /** @brief type of buffer DRAM, according to this field the DRAM
     *  parameters changed in setPpRamConfigPhase1().
     */
    CPSS_BUFFER_DRAM_PARAM_ENT bufferDram;

    /** @brief type of flow DRAM, according to this field the DRAM
     *  parameters changed in setPpRamConfigPhase1().
     */
    CPSS_FLOW_DRAM_PARAM_ENT flowDram;

    /** @brief Management interface type (SMI/PCI)
     *  routingSramCfgType- Determines what's the routing memory configuration.
     *  This parameter is relevant to ExMx and ExMxPm devices except Puma3.
     */
    CPSS_PP_INTERFACE_CHANNEL_ENT mngInterfaceType;

    CPSS_NARROW_SRAM_CFG_ENT routingSramCfgType;

    /** @brief Number of valid entries in sramsCfg.
     *  This parameter is relevant to ExMx and ExMxPm devices except Puma3.
     *  hwDevNum     - HW device number. The PSS writes this device number in the
     *  PP instead of devNum. The field is used only for systems whith
     *  Coexistance Manager lib included.
     *  For other system hwDevNum is ignored
     */
    GT_U32 numOfSramsCfg;

    CPSS_SRAM_CFG_STC sramsCfg[CPSS_MAX_NUMBER_OF_EXTERNAL_SRAMS_CNS];

    /** @brief System maximum buffer size.
     *  eventsTaskPrio   - Default RTOS Priority to be assigned to the
     *  Interrupt task.
     *  eventsTaskStackSize- Stack size (in bytes) needed for user
     *  functions invoked from within the user exits
     *  context.
     *  The actual interrupt task stack size equals:
     *  eventsTaskStackSize + <internal stack size>,
     *  where <internal stack size> is the stack size
     *  needed for the interrupt task.
     */
    CPSS_BUF_MODE_ENT maxBufSize;

    /** system HA mode, e.g. active or standby */
    CPSS_SYS_HA_MODE_ENT sysHAState;

    /** SERDES reference clock type. */
    APPDEMO_SERDES_REF_CLOCK_ENT serdesRefClock;

    /** @brief it's ignored for all not xCat devices
     *  it's ignored for xCat devices without internal CPU
     *  it's relevant only for xCat devices with internal CPU
     *  GT_TRUE - External CPU is connected to the PP. CPSS configures PP to work with external CPU
     *  even if internal CPU is active
     *  GT_FALSE- External CPU is not connected to the PP (only internal CPU active)
     */
    GT_BOOL isExternalCpuConnected;

    /** @brief GT_TRUE
     *  GT_FALSE - disable all TCAMs parity calculation
     *  To enable parity checks by device, use cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_BOOL tcamParityCalcEnable;

    /** @brief Bitmap of configurable address completion regions
     *  used by interrupt hanling routine.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 isrAddrCompletionRegionsBmp;

    /** @brief Bitmap of configurable address completion regions
     *  used by CPSS API.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Region 0 provide access to the lower addresses that contain also
     *  configuration of address completion regions. It must never be configured.
     *  Lists (coded as ranges) of other regions can be assigned for several CPUs
     *  for interrupt handling (for such CPU that processes the PP interrupts)
     *  and for all other CPSS API using.
     *  These assigned ranges of regions must not have common members.
     *  These assigned ranges of regions must not contain zero range.
     */
    GT_U32 appAddrCompletionRegionsBmp;

    /** @brief GT_TRUE
     *  GT_FALSE - disable legacy VPLS mode support
     *  (APPLICABLE DEVICES: xCat; xCat3) xCat device: applicable starting from revision C0
     */
    GT_BOOL enableLegacyVplsModeSupport;

} CPSS_PP_PHASE1_INIT_PARAMS;


/**
* @struct CPSS_PP_PHASE2_INIT_PARAMS
 *
 * @brief PP Hw phase2 init Parameters.
*/
typedef struct{

    /** @brief The device number to assign to this PP. Range is
     *  0-127.
     */
    GT_U8 devNum;

    /** @brief The unique Prestera Packet Processor device ID,
     *  as returned from corePpHwPhase1Init().
     */
    CPSS_PP_DEVICE_TYPE deviceId;

    /** Network interface configuration parameters: */
    CPSS_NET_IF_CFG_STC netIfCfg;

    /** Address Update Queue configuration parameters: */
    CPSS_AUQ_CFG_STC auqCfg;

    /** @brief HW device number. The PSS writes this device number in the
     *  PP instead of devNum. The field is used only for systems with
     *  Coexistance Manager lib included.
     *  For other system hwDevNum is ignored
     */
    GT_HW_DEV_NUM hwDevNum;

    /** @brief FDB Upload Queue configuration parameters.The parameter
     *  used only for 98DX2x5 devices and ignored for another devices
     */
    CPSS_AUQ_CFG_STC fuqCfg;

    /** @brief GT_TRUE
     *  - GT_FALSE - use Address Update queue for FDB Upload messages
     *  The parameter used only for 98DX2x5 devices and ignored
     *  for another devices
     */
    GT_BOOL fuqUseSeparate;

    /** @brief The size of the SDMA Rx data buffer. If the Data
     *  buffer size is smaller than received packet
     *  size, the packet is "chained" over multiple
     *  buffers. This value is used by the CORE when
     *  initializing the RX descriptor rings.
     *  rxBufSize in the above calculations is received via TAPI
     *  sysConfig.h sysConfig() API.
     *  5.  The number of Tx descriptors to be allocated per Tx queue will be
     *  calculated as follows:
     *  (txDescBlockSize / <size of a single Tx desc.>) /
     *  <number of Rx queues>.
     *  The size of a single Tx descriptor can be obtained by calling
     *  coreGetTxDescSize().
     *  6.  The provided auDescBlock for FDB Upload (FU) messages will be divided
     *  into FU descriptors. The size of each FU descriptor is same as AU one
     *  and can be obtained via coreGetAuDescSize(). The number of
     *  FU descriptors is calculated by:
     *  (auDescBlockSize / <size of a single descriptor> ).
     */
    GT_U32 rxBufSize;

    /** @brief The number of bytes before the start of the Rx
     *  buffer to reserve for the applicaion use.
     *  This value is used by the CORE when
     *  initializing the RX descriptor rings.
     *  NOTE: This parameter must be synchronized among
     *  all system CPUs.
     */
    GT_U32 headerOffset;

    /** @brief whether application requires traffic to CPU
     *  (inverse logic,
     *  GT_FALSE means traffic to CPU
     *  G_TRUE  means no traffic to CPU
     */
    GT_BOOL noTraffic2CPU;

    /** @brief value of port group ID used for CPU network
     *  interface (relevant only for multi-port group devices while
     *  working in both(!!!) MII and SDMA modes).
     */
    GT_U32 netifSdmaPortGroupId;

    /** @brief The AU message length. Extending the AU message increases
     *  the size of the message to more than 16 bytes
     */
    CPSS_AU_MESSAGE_LENGTH_ENT auMessageLength;

    /** @brief the flag to indicate that the application needs the CPSS
     *  to implement the WA for the triggered action
     *  completion while AU queue is full. The triggered FDB action
     *  cannot be completed till AU queue is full. The 'Secondary AUQ'
     *  may be used in order to fix the problem.
     *  CPSS will use a 'Secondary AUQ' , and for that will
     *  remove CPSS_DXCH_PP_FDB_AUQ_DEADLOCK_EXTRA_MESSAGES_NUM_WA_CNS
     *  entries from the AUQ.
     *  GT_TRUE - CPSS need to implement the WA for the triggered FDB action
     *  cannot be completed till AU queue is full issue.
     *  cpss will use 644 words from the AUQ (see auqCfg) for that
     *  'Secondary AUQ'.
     *  GT_FALSE - CPSS WILL NOT implement the WA for the triggered FDB action
     *  cannot be completed till AU queue is full issue.
     *  cpss WILL NOT use 644 words from the AUQ (see auqCfg) for that
     *  'Secondary AUQ'.
     *  When useDoubleAuq == GT_TRUE, CPSS will use 2644 words
     *  from the AUQ (see auqCfg) for that 'Secondary AUQ'.
     */
    GT_BOOL useSecondaryAuq;

    /** @brief Support configuration of two AUQ memory regions.
     *  GT_TRUE - CPSS manages two AU Queues with the same size:
     *  auqCfg->auDescBlockSize / 2.
     *  GT_FALSE - CPSS manages single AU Queue with size:
     *  auqCfg->auDescBlockSize.
     */
    GT_BOOL useDoubleAuq;

    /** @brief support multi network interface SDMA configuration mode.
     *  GT_TRUE - Application use new buffers/ descriptors management.
     *  Application is required to supply new management configuration multiNetIfCfg.
     *  Application may use Tx Generator, Multi MGs.
     *  GT_FALSE - Application use the old legacy buffers/ descriptors management.
     */
    GT_BOOL useMultiNetIfSdma;

    /** @brief is ignored.
     *  Application can't use Tx Generator, Multi MGs.
     *  multiNetIfCfg - Multi network interface configuration parameters.
     *  Comments:
     *  1.  The Driver allocates Tx descriptors, Rx Descriptors, and Rx data
     *  buffers out of the respective blocks of memory provided by the user.
     *  The number of allocated items is a function of the block size, item
     *  structure size, and the HW alignment requirement for the specific
     *  item (and the Rx Buffer headerOffset as defined in sysConfig.h)
     *  2.  The Prestera PP PCI Host Interface directly updates the Rx/Tx
     *  descriptors, Rx data buffers, and the Address Update Queue.
     *  It is the user's responsibility to determine whether these memory
     *  blocks (txDescBlock, rxDescBlock, rxbufBlock, AUQ) are from
     *  cachable. non-cachable memory regions. For example if the
     *  system supports cach-coherency between the CPU & the
     *  PCI, those blocks can be in a cachable area with no
     *  special handling
     *  If there is no cache coherency, then:
     *  1) the Rx/Tx descriptors must be allocated from non-cachable
     *  memory.
     *  2) the Rx data buffers and AUQ may be from cachable memory,
     *  if the user flushes the buffer memory cache prior to
     *  returning the buffer to the Rx pool, and cache-line
     *  reads do not cross data buffer boundaries.
     *  3.  The provided auDescBlock will be divided into AU descriptors. The
     *  size of each AU descriptor can be obtained via coreGetAuDescSize(),
     *  The number of address update descriptors is calculated by:
     *  (auDescBlockSize / <size of a single descriptor> ).
     *  4.  The number of Rx descriptors to be allocated per Rx queue will be
     *  calculated as follows:
     *  -  if <allocation method> = STATIC_ALLOC, then the number of
     *  Rx descs. per Rx queue will be:
     *  (rxBufBlockSize / rxBufSize) / <number of Rx queues>
     *  while The Rx buffer size must be a multiple of 8.
     *  In this case the user must take care to provide an Rx Desc.
     *  block which is large enough to allocate an Rx desc. per
     *  allocated Rx buffer.
     *  -  if <allocation method> = DYNAMIC_ALLOC, then the number of
     *  Rx descriptors per queue will be:
     *  (rxDescBlockSize / <size of a single Rx desc.>) /
     *  <number of Rx queues>.
     *  The size of a single Rx descriptor can be obtained by
     *  calling coreGetRxDescSize().
     */
    CPSS_MULTI_NET_IF_CFG_STC multiNetIfCfg;

} CPSS_PP_PHASE2_INIT_PARAMS;

/**
* @struct APP_DEMO_CPSS_PP_CONFIG_EXTERNAL_TCAM_STC
 *
 * @brief Packet Processor (PP) initialization configuration
 * Parameters structure --> relevant to the external TCAM .
*/
typedef struct{

    GT_U32 actionLinesInMemoryControl0;

    GT_U32 actionLinesInMemoryControl1;

    void *extTcamRuleWriteFuncPtr;

    void *extTcamRuleReadFuncPtr;

    void *extTcamActionWriteFuncPtr;

    void *extTcamActionReadFuncPtr;

} APP_DEMO_CPSS_PP_CONFIG_EXTERNAL_TCAM_STC;

/**
* @enum APP_DEMO_CPSS_LPM_MEM_CFG_ENT
 *
 * @brief Defines the routing lpm memory configuration.
*/
typedef enum{

    /** Internal memory for TWIST type asic backward compatibility */
    APP_DEMO_CPSS_EXMX_TWIST_INTERNAL_E,

    /** Only external memory for TWIST type asic backward compatibility */
    APP_DEMO_CPSS_EXMX_TWIST_EXTERNAL_E,

    /** @brief Two external memories that work in 250 MHz and therefore in order
     *  to support wire speed the internal memories are used as well.
     */
    APP_DEMO_CPSS_EXMX_TWO_EXTERNAL_250MHZ_E,

    /** @brief Two external narrow srams each working in 300MHz; It supports IPv4
     *  and IPv6 routing in full wire speed.
     */
    APP_DEMO_CPSS_EXMX_TWO_EXTERNAL_300MHZ_E,

    /** @brief Support taking the Tiger as a drop in to TwistD boards. With
     *  emphasis on capacity (more IPv4 mc groups, less wire speed)
     */
    APP_DEMO_CPSS_EXMX_DROP_IN_CAPACITY_E,

    /** @brief Support taking the Tiger as a drop in to TwistD boards. With
     *  emphasis on speed (more wire speed, less IPv4 mc groups)
     */
    APP_DEMO_CPSS_EXMX_DROP_IN_SPEED_E,

    /** @brief Two internal narrow srams 32Kbyte each working 300 MHz. It
     *  supports IPv4 and IPv6 routing in full wire speed.
     */
    APP_DEMO_CPSS_EXMX_TWO_INTERNAL_E,

    /** One 64 Kbyte internal narrow sram for IPv4 only backward compatible mode. */
    APP_DEMO_CPSS_EXMX_ONE_INTERNAL_E,

    /** @brief No nram configuration
     *  ----- configurations for ExMxMx devices --------------
     */
    APP_DEMO_CPSS_EXMX_NRAM_NOT_EXISTS_E,

    /** LPM algorithm uses internal tables */
    APP_DEMO_CPSS_EXMXPM_LPM_ALL_INTERNAL_E,

    /** LPM algorithm uses three external tables */
    APP_DEMO_CPSS_EXMXPM_LPM_ALL_EXTERNAL_E,

    /** @brief LPM algorithm uses a single external memory. The other three
     *  tables are internal.
     */
    APP_DEMO_CPSS_EXMXPM_LPM_SINGLE_EXTERNAL_E,

    /** @brief LPM algorithm uses a single internal memory. The other three
     *  tables are external.
     */
    APP_DEMO_CPSS_EXMXPM_LPM_SINGLE_INTERNAL_E,

    /** @brief LPM algorithm uses 2 internal memory. The other 2
     *  tables are external.
     */
    APP_DEMO_CPSS_EXMXPM_LPM_DOUBLE_INTERNAL_E,

    /** Invalid LPM SRAM configuration */
    APP_DEMO_CPSS_EXMXPM_LPM_SRAM_NOT_EXISTS_E

} APP_DEMO_CPSS_LPM_MEM_CFG_ENT;

/**
* @enum APP_DEMO_CPSS_LPM_STAGES_MEM_ASSOCIATION_ENT
 *
 * @brief Controls the association of internal LPM LU Memories
 * 0 and 3 to LPM stages (0,4) and (3,7).
 * This configuration is valid for the LPM stages
 * enabled for internal memory LU.
 * (APPLICABLE DEVICES: PUMA3)
*/
typedef enum{

    /** @brief Optimize for All Internal;
     *  LPM Stages 0,4 are associarted with LPM0 (8K)
     *  LPM Stages 3,7 are associarted with LPM3 (64K)
     */
    APP_DEMO_CPSS_LPM_STAGES_MEM_ASSOCIATION_ALL_INTERNAL_E,

    /** @brief Optimize for All External;
     *  LPM Stages 0,4 are associarted with LPM3 (64K)
     *  LPM Stages 3,7 are associarted with LPM0 (8K)
     */
    APP_DEMO_CPSS_LPM_STAGES_MEM_ASSOCIATION_ALL_EXTERNAL_E

} APP_DEMO_CPSS_LPM_STAGES_MEM_ASSOCIATION_ENT;


/*
 * typedef: struct APP_DEMO_CPSS_IP_MEM_CFG_STC
 *
 * Description:
 *      This struct hold all the info needed to config a specific IP memory
 *      configuarion as part of LPM DB.
 *
 * Fields:
 *    routingSramCfgType- Determines what's the routing memory configuration.
 *    useLpmInternalSram - GT_TRUE: lpmInternalSramSizeArray is in use and not sramsSizeArray
 *                         GT_FALSE: sramsSizeArray is in use ant not lpmInternalSramSizeArray
 *                       (APPLICABLE DEVICES: Puma3)
 *    lpmInternalSramSizeArray  - Map flat lpm internal memory to lpm controllers.
 *                       LPM lookup is performed using 8 LPM stages.
 *                       Every stage performs lookup on 1 octet from the IPv4 address
 *                       or 2 consecutive octets from the IPv6 address.
 *                       There are 4 LPM memory controllers, each serves 2 lpm stages:
 *                       LPM controller 0 -> stage 0,4; LPM controller 1 -> stage 1,5;
 *                       LPM controller 2 -> stage 2,6; LPM controller 3 -> stage 3,7;
 *                       LPM stages could be configured for look up in external or internal memory.
 *                       Device incorporates 120K lines of internal lpm memory.
 *                       It is divided on 60 memory banks 2k lines each.
 *                       Every index of lpmInternalSramSizeArray fit to corresponding
 *                       LPM memory controller.(for example, index 0 to LPM controller 0).
 *                       Every member of lpmInternalSramSizeArray is a size value in Klines
 *                       (2k lines Multiplication from 0 to 120 Klines).
 *                       The total sum of all lpmInternalSramSizeArray members is not more than 120 Klines.
 *                       (APPLICABLE DEVICES: Puma3)
 *    sramsSizeArray    - The External Routing Sram sizes (aka Narrow Sram).
 *    numOfSramsSizes   - Number of valid entries in sramsSizeArray.
 *    extMemSize        - global external memory size in bytes (before taking in account
 *                        external memory logical bank organization). (APPLICABLE DEVICES: PUMA3)
 *    extMemBankMode    - external memory logical bank mode.(APPLICABLE DEVICES: PUMA3)
 */
typedef struct APP_DEMO_CPSS_IP_MEM_CFG_STCT
{
    APP_DEMO_CPSS_LPM_MEM_CFG_ENT   routingSramCfgType;
    GT_BOOL                         useLpmInternalSram;
    GT_U32                          lpmInternalSramSizeArray[CPSS_MAX_NUMBER_LPM_INTERNAL_SRAMS_CNS];
    CPSS_SRAM_SIZE_ENT              sramsSizeArray[APP_DEMO_CPSS_MAX_NUMBER_OF_SRAMS_CNS];
    GT_U32                          numOfSramsSizes;
    GT_U32                          extMemSize;
    APPDEMO_EXMXPM_EXT_MEMORY_LOGICAL_MODE_ENT extMemBankMode;
}APP_DEMO_CPSS_IP_MEM_CFG_STC;




/**
* @enum APP_DEMO_CPSS_DIT_MEMORY_MODE_ENT
 *
 * @brief Defines DIT memory mode
*/
typedef enum{

    /** @brief all different DIT types share the same memory (meaning same
     *  base address and same size)
     */
    APP_DEMO_CPSS_DIT_SHARED_MEM_E,

    /** @brief each DIT type has separate memory block; this mode is supported
     *  only on external memory
     */
    APP_DEMO_CPSS_DIT_SEPARATE_MEM_E

} APP_DEMO_CPSS_DIT_MEMORY_MODE_ENT;

/**
* @enum APP_DEMO_CPSS_DIT_IP_MC_MODE_ENT
 *
 * @brief Defines IP MC DIT type mode
*/
typedef enum{

    /** Each DIT table line contains two, reduced format, IP MC entries */
    APP_DEMO_CPSS_DIT_IP_MC_DUAL_MODE_E,

    /** IP MC entry has an extended format and uses a full line of the DIT table */
    APP_DEMO_CPSS_DIT_IP_MC_SINGLE_MODE_E

} APP_DEMO_CPSS_DIT_IP_MC_MODE_ENT;

/**
* @enum APP_DEMO_CPSS_INLIF_PORT_MODE_ENT
 *
 * @brief Port InLIF Mode enumeration
*/
typedef enum{

    /** @brief The InLIF-ID is set to be equal
     *  to the perport configured default InLIFID.
     */
    APP_DEMO_CPSS_INLIF_PORT_MODE_PORT_E,

    /** @brief The InLIF-ID is set to be equal
     *  to the VLANID.
     */
    APP_DEMO_CPSS_INLIF_PORT_MODE_VLAN_E

} APP_DEMO_CPSS_INLIF_PORT_MODE_ENT;

/**
* @enum APP_DEMO_CPSS_DXCH_ROUTING_MODE_ENT
 *
 * @brief Routing mode
*/
typedef enum{

    /** @brief PBR (Policy based routing)
     *  using PCL action as next hop.
     */
    APP_DEMO_CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E = 0,

    /** @brief Regular routing using router engine
     *  or PBR using next hop table.
     */
    APP_DEMO_CPSS_DXCH_TCAM_ROUTER_BASED_E         = 1

} APP_DEMO_CPSS_DXCH_ROUTING_MODE_ENT;


/**
* @enum APP_DEMO_CPSS_MEMORY_LOCATION_ENT
 *
 * @brief Defines whether data is located in Internal or External memory
*/
typedef enum{

    /** data is in internal memory */
    APP_DEMO_CPSS_MEMORY_LOCATION_INTERNAL_E,

    /** data is in external memory */
    APP_DEMO_CPSS_MEMORY_LOCATION_EXTERNAL_E

} APP_DEMO_CPSS_MEMORY_LOCATION_ENT;




/****************************************************************************************************/
/**
* @struct CPSS_PP_CONFIG_INIT_STC
 *
 * @brief System configuration Parameters struct.
*/
typedef struct{

    GT_U32 maxNumOfLifs;

    /** default inlif mode (per port / per vlan). */
    APP_DEMO_CPSS_INLIF_PORT_MODE_ENT inlifPortMode;

    /** @brief max number of Policer entries in the
     *  device.
     */
    GT_U32 maxNumOfPolicerEntries;

    /** @brief When enabled, the Policer Entry contains
     *  conformance counters (i.e. green, yellow, red
     *  counters), increasing the size of the Policer
     *  Entry. If conformance counters are not required,
     *  the entry size is reduced.
     *  (Allowing 50% more Policer Entries).
     */
    GT_BOOL policerConformCountEn;

    /** @brief max number of NHLFE entries in the PP :
     *  includes 16 reserved entries.
     */
    GT_U32 maxNumOfNhlfe;

    /** max number of Mpls interfaces in PP. */
    GT_U32 maxNumOfMplsIfs;

    /** @brief shows whether ip and MPLS entries allocated in
     *  external or internal memory:
     *  APP_DEMO_CPSS_MEMORY_LOCATION_INTERNAL_E - ip and MPLS entries allocated in internal memory
     *  APP_DEMO_CPSS_MEMORY_LOCATION_EXTERNAL_E - ip and MPLS entries allocated in external memory
     *  (APPLICABLE DEVICES: PUMA3)
     */
    APP_DEMO_CPSS_MEMORY_LOCATION_ENT ipMplsMemLocation;

    /** max number of PCL action in PP. */
    GT_U32 maxNumOfPclAction;

    /** @brief defines size (8/16 bytes)of PCL action entry
     *  in the Control SRAM
     */
    GT_U32 pclActionSize;

    GT_U32 maxNumOfPceForIpPrefixes;

    /** @brief whether to use policy based routing
     *  usePolicyBasedDefaultMc- whether to set default rules to match all MC traffic
     *  when working in policy based routing mode; for details
     *  refer to cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet
     */
    GT_BOOL usePolicyBasedRouting;

    GT_BOOL usePolicyBasedDefaultMc;

    /** @brief max number of LPM leafs that can be allocated in
     *  the RAM for policy based routing
     */
    GT_U32 maxNumOfPbrEntries;

    /** max VLAN ID in the device. */
    GT_U16 maxVid;

    /** @brief max number of Multicast group need to be
     *  supported.
     */
    GT_U16 maxMcGroups;

    /** max numbers of IPv4 virtual routers. */
    GT_U32 maxNumOfVirtualRouters;

    /** @brief max number of IPv4 and Ipv6 next hop entries in the
     *  PP.
     */
    GT_U32 maxNumOfIpNextHop;

    /** @brief max number of IPv4 prefixes in the PP.
     *  on TCAM based PP with LPM DB, this value is
     *  the number of IPv4 prefixes the LPM DB supports
     *  (refer to LPM DB creation for more details)
     */
    GT_U32 maxNumOfIpv4Prefixes;

    /** @brief max number of IPv4 and IPv6 Multicast Entries in the PP.
     *  on TCAM based PP with LPM DB, this value is
     *  the number of IPv4 multicast sources the LPM DB supports
     *  (refer to LPM DB creation for more details)
     */
    GT_U32 maxNumOfIpv4McEntries;

    /** @brief max number of Downstream IPv4 and Ipv6 Multicast
     *  interface entries (MLLs) in the PP.
     */
    GT_U32 maxNumOfMll;

    /** @brief max number of IPv6 prefixes in the PP.
     *  on TCAM based PP with LPM DB, this value is
     *  the number of IPv6 prefixes the LPM DB supports
     *  (refer to LPM DB creation for more details)
     */
    GT_U32 maxNumOfIpv6Prefixes;

    /** @brief max number of IPv6 Multicast groups.
     *  maxNumOfPceForIpPrefixes- max number of Pcl entries reserved for IP
     *  prefixes in the Pcl Tcam.
     */
    GT_U32 maxNumOfIpv6McGroups;

    /** max number of tunnel (of all types) entries */
    GT_U32 maxNumOfTunnelEntries;

    /** @brief the maximum number of ipv4 tunnel termination
     *  entries
     */
    GT_U32 maxNumOfIpv4TunnelTerms;

    /** @brief number of tunnel termination entries (or tunnel
     *  termination interface entries in case of Cheetah3)
     */
    GT_U32 maxNumOfTunnelTerm;

    /** @brief Determines the type of the vlan inlif entry
     *  (128 bits long or 42 bits long).
     *  1. GT_VLAN_INLIF_ENTRY_TYPE_REGULAR_E: It should be
     *  used when working in:
     *  A. INLIF_PER_LPORT_MODE.
     *  B.INLIF_PER_VLAN_MODE.
     *  C.INLIF_MIXED_MODE
     *  2. GT_VLAN_INLIF_ENTRY_TYPE_REDUCED_E: It enables
     *  using around 1/3 of the memory needed for vlan
     *  regular inlif entries. It should be used when
     *  working in:
     *  A. INLIF_MIXED_MODE
     */
    GT_U32 vlanInlifEntryType;

    /** if GT_FALSE, no mem devision between IPv4 and IPv6 takes place */
    GT_BOOL ipMemDivisionOn;

    /** PP routing mode: Policy based routing Or TCAM based Routing */
    APP_DEMO_CPSS_DXCH_ROUTING_MODE_ENT routingMode;

    /** @brief relevant only if ipMemDivisionOn is true, percentage
     *  of ip memory reserved for IPv6 use.
     */
    GT_U32 ipv6MemShare;

    /** Number of trunks. */
    GT_U32 numOfTrunks;

    /** PCL rule index for default IPv6 MC */
    GT_U32 defIpv6McPclRuleIndex;

    /** PCL ID to use for IPv6 MC */
    GT_U32 vrIpv6McPclId;

    /** whether static/dynamic partition of TCAM based LPM DB */
    GT_BOOL lpmDbPartitionEnable;

    /** first TCAM line allocated for TCAM based LPM DB */
    GT_U32 lpmDbFirstTcamLine;

    /** last TCAM line allocated for TCAM based LPM DB */
    GT_U32 lpmDbLastTcamLine;

    /** whether the LPM DB supports IPv4 */
    GT_BOOL lpmDbSupportIpv4;

    /** whether the LPM DB supports IPv6 */
    GT_BOOL lpmDbSupportIpv6;

    /** @brief number of memory configuration supported by the lpm db
     *  lpmDbMemCfgArray    - array of memory configuration supported by the lpm db
     */
    GT_U32 lpmDbNumOfMemCfg;

    APP_DEMO_CPSS_IP_MEM_CFG_STC *lpmDbMemCfgArray;

    /** @brief external LPM memory size in bytes. This parameter is
     *  relevant only for Puma 3 devices.
     *  lpmStagesMemAssociation- Controls the association of internal LPM LU Memories
     *  0 and 3 to LPM stages (0,4) and (3,7).
     *  This configuration is valid for the LPM stages
     *  enabled for internal memory LU.
     *  0x0 = Optimize for All Internal;
     *  LPM Stages 0,4 are associarted with LPM0 (8K)
     *  LPM Stages 3,7 are associarted with LPM3 (64K)
     *  0x1 = Optimize for All External;
     *  LPM Stages 0,4 are associarted with LPM3 (64K)
     *  LPM Stages 3,7 are associarted with LPM0 (8K)
     *  (APPLICABLE DEVICES: PUMA3)
     */
    GT_U32 extLpmMemSize;

    APP_DEMO_CPSS_LPM_STAGES_MEM_ASSOCIATION_ENT lpmStagesMemAssociation;

    /** This struct holds all the info needed to create the LPM DB. */
    CPSS_DXCH_LPM_RAM_CONFIG_STC lpmRamMemoryBlocksCfg;

    /** @brief shows whether dit entries allocated in
     *  external or internal memory:
     *  APP_DEMO_CPSS_MEMORY_LOCATION_INTERNAL_E - dit entries allocated in internal memory
     *  APP_DEMO_CPSS_MEMORY_LOCATION_EXTERNAL_E - dit entries allocated in external memory
     *  (APPLICABLE DEVICES: PUMA3)
     */
    APP_DEMO_CPSS_MEMORY_LOCATION_ENT ditMemLocation;

    /** @brief The DIT memory configuration (shared memory
     *  or separate memory)
     */
    APP_DEMO_CPSS_DIT_MEMORY_MODE_ENT ditMemoryMode;

    /** IP MC DIT type mode (dual or single) */
    APP_DEMO_CPSS_DIT_IP_MC_MODE_ENT ditIpMcMode;

    /** @brief Number of dit (downstream interface table)
     *  entries needed for UC IP.
     */
    GT_U32 maxNumOfDitUcIpEntries;

    /** @brief Number of dit (downstream interface table)
     *  entries needed for UC MPLS.
     */
    GT_U32 maxNumOfDitUcMplsEntries;

    /** @brief Number of dit (downstream interface table)
     *  entries needed for UC other than IP or MPLS.
     */
    GT_U32 maxNumOfDitUcOtherEntries;

    /** @brief Number of dit (downstream interface table)
     *  entries needed for MC IP.
     */
    GT_U32 maxNumOfDitMcIpEntries;

    /** @brief Number of dit (downstream interface table)
     *  entries needed for MC MPLS.
     */
    GT_U32 maxNumOfDitMcMplsEntries;

    /** @brief Number of dit (downstream interface table)
     *  entries needed for MC other than IP or MPLS.
     */
    GT_U32 maxNumOfDitMcOtherEntries;

    /** Number of requested TX descriptors. */
    GT_U32 miiNumOfTxDesc;

    /** @brief The raw size in bytes of host memory to be used
     *  for internal TX buffers.
     */
    GT_U32 miiTxInternalBufBlockSize;

    GT_U32 miiBufferPercentage[CPSS_MAX_RX_QUEUE_CNS];

    /** The size of the Rx data buffer. */
    GT_U32 miiRxBufSize;

    /** Packet header offset size. */
    GT_U32 miiHeaderOffset;

    /** @brief The raw size in byte of memory to be used for
     *  allocating Rx packet data buffers.
     *  COMMENTS:
     *  The maximum of the (VLAN table size + maximum number of MAC
     *  multicast entries) is specific to each packet processor type.
     *  Please refer to the specific packet process datasheet for details.
     *  The actual number of tunnels that can be configured is a trade of between
     *  the number of mc groups, the number of vlan entries and the number of tunnels.
     *  See the particular packet processor datasheet for details on the
     *  relationship between maxVid, maxMcGroups and the number of Tunnel
     *  entries available.
     */
    GT_U32 miiRxBufBlockSize;

    /** info needed for cpssDxChPhaInit(...) */
    GT_BOOL pha_packetOrderChangeEnable;

} CPSS_PP_CONFIG_INIT_STC;



/****************************************************************************************************/

/**
* @struct APP_DEMO_LIB_INIT_PARAMS
 *
 * @brief Includes tapi libraries initialization parameters.
*/
typedef struct
{
    GT_BOOL                 initBridge;
    GT_BOOL                 initClassifier;
    GT_BOOL                 initCos;
    GT_BOOL                 initIpv4;
    GT_BOOL                 initIpv6;
    GT_BOOL                 initIpv4Filter;
    GT_BOOL                 initIpv4Tunnel;
    GT_BOOL                 initLif;
    GT_BOOL                 initMirror;
    GT_BOOL                 initMpls;
    GT_BOOL                 initMplsTunnel;
    GT_BOOL                 initNetworkIf;
    GT_BOOL                 initI2c;
    GT_BOOL                 initPhy;
    GT_BOOL                 initPort;
    GT_BOOL                 initTc;
    GT_BOOL                 initTrunk;
    GT_BOOL                 initPcl;
    GT_BOOL                 initTcam;
    GT_BOOL                 initPolicer;
    GT_BOOL                 initAction;
    GT_BOOL                 initSflow;
    GT_BOOL                 initPha;    /* sip6 PHA init */
}APP_DEMO_LIB_INIT_PARAMS;


/*
 * Typedef: FUNCP_CPSS_PP_LOGICAL_INIT
 *
 * Description:
 *      Implements logical initialization for specific device.
 *      Device number is a part of system configuration parameters.
 *
 * Fields:
 *      devNum               -   device number
 *      logicalConfigParams  -   PP logical configuration parameters
 */
typedef GT_STATUS (*FUNCP_CPSS_PP_LOGICAL_INIT)
(
   IN  GT_U8                        devNum,
   IN  CPSS_PP_CONFIG_INIT_STC     *logicalConfigParams
);

/*
 * Typedef: FUNCP_CPSS_LIBRARIES_INIT
 *
 * Description:
 *      Initialize all CPSS libraries for specific device.
 *      It's entry point where all required CPSS libraries are initialized.
 *
 * Fields:
 *      dev                 -   device number
 *      libInitParamsPtr    -   initialization parameters
 *      sysConfigParamPtr   -   system configuration parameters
 */
typedef GT_STATUS (*FUNCP_CPSS_LIBRARIES_INIT)
(
    IN  GT_U8                       dev,
    IN  APP_DEMO_LIB_INIT_PARAMS    *libInitParamsPtr,
    IN  CPSS_PP_CONFIG_INIT_STC     *sysConfigParamsPtr
);

/*
 * Typedef: FUNCP_CPSS_NETWORK_IF_INIT
 *
 * Description:
 *      Initialize network interface for specific device.
 *
 * Fields:
 *      dev     -   device number
 */
typedef GT_STATUS (*FUNCP_CPSS_NETWORK_IF_INIT)
(
    IN  GT_U8   dev
);

/*
 * Typedef: FUNCP_CPSS_TRAFFIC_ENABLE
 *
 * Description:
 *      Enable traffic for specific device.
 *      Enable ports and device.
 *
 * Fields:
 *      dev     -   device number
 */
typedef GT_STATUS (*FUNCP_CPSS_TRAFFIC_ENABLE)
(
    IN  GT_U8   dev
);

/*
 * Typedef: FUNCP_CPSS_HW_PP_PHASE1_INIT
 *
 * Description:
 *      Implements HW phase 1 initialization.
 *
 * Fields:
 *      dev                         -   temp device id, could be changed later
 *      appCpssPpPhase1ParamsPtr    -   PP phase 1 init parameters
 *      ppDeviceTypePtr             -   output correct device type
 */
typedef GT_STATUS (*FUNCP_CPSS_HW_PP_PHASE1_INIT)
(
    IN  GT_U8                       dev,
    IN  CPSS_PP_PHASE1_INIT_PARAMS  *appCpssPpPhase1ParamsPtr,
    OUT CPSS_PP_DEVICE_TYPE         *ppDevTypePtr
);

/*
 * Typedef: FUNCP_CPSS_HW_PP_START_INIT
 *
 * Description:
 *      Implements PP start initialization.
 *
 * Fields:
 *      dev             -   device number
 *      fReloadEeprom   -   reload Eeprom flag
 *      initDataListPtr -   list of registers to init
 *      initDataListLen -   length of registers' list
 */
typedef GT_STATUS (*FUNCP_CPSS_HW_PP_START_INIT)
(
    IN  GT_U8                       dev,
    IN  GT_BOOL                     fReloadEeprom,
    IN  CPSS_REG_VALUE_INFO_STC     *initDataListPtr,
    IN  GT_U32                      initDataListLen
);

/*
 * Typedef: FUNCP_CPSS_HW_PP_PHASE2_INIT
 *
 * Description:
 *      Implements HW phase 2 initialization.
 *
 * Fields:
 *      oldDev                      -   old device id, used for phase 1
 *      appCpssPpPhase2ParamsPtr    -   PP phase 2 init parameters
 */
typedef GT_STATUS (*FUNCP_CPSS_HW_PP_PHASE2_INIT)
(
    IN  GT_U8                       oldDev,
    IN  CPSS_PP_PHASE2_INIT_PARAMS  *appCpssPpPhase2ParamsPtr
);

/*
 * Typedef: FUNCP_CPSS_PP_GENERAL_INIT
 *
 * Description:
 *      Implements general initialization for specific device.
 *
 * Fields:
 *      dev                  -   device id
 */
typedef GT_STATUS (*FUNCP_CPSS_PP_GENERAL_INIT)
(
    IN  GT_U8   dev
);

/**
* @struct APP_DEMO_SYS_CONFIG_FUNCS
 *
 * @brief Contains functions for general CPSS init flow.
*/
typedef struct{

    /** HW phase 1 init */
    FUNCP_CPSS_HW_PP_PHASE1_INIT cpssHwPpPhase1Init;

    /** PP start init */
    FUNCP_CPSS_HW_PP_START_INIT cpssHwPpStartInit;

    /** HW phase 2 init */
    FUNCP_CPSS_HW_PP_PHASE2_INIT cpssHwPpPhase2Init;

    /** @brief Logical init for device
     *  cpssNetworkIfInit  -  network interface init
     */
    FUNCP_CPSS_PP_LOGICAL_INIT cpssPpLogicalInit;

    /** All CPSS libraries init */
    FUNCP_CPSS_LIBRARIES_INIT cpssLibrariesInit;

    /** enable ports and devices */
    FUNCP_CPSS_TRAFFIC_ENABLE cpssTrafficEnable;

    /** @brief General initialization
     *  Comments:
     *  Since there is more than API could be used this
     *  structure is used to provide general initialization
     *  flow. In order to extend general initialization flow
     *  new function should be added here and implemented for
     *  supported API - ExMx or Dx.
     */
    FUNCP_CPSS_PP_GENERAL_INIT cpssPpGeneralInit;

} APP_DEMO_SYS_CONFIG_FUNCS;

/*******************************************************************************
* APP_DEMO_SYS_CONFIG_FUNCS_EXTENTION_GET_FUNC
*
* DESCRIPTION:
*       Gets the system configuration functions accordingly to given device type.
*       allow override of previos setting done by function appDemoSysConfigFuncsGet
*
* INPUTS:
*       deviceType  - device type.
*
* OUTPUTS:
*       sysConfigFuncsPtr  - system configuration functions.
*       apiSupportedBmpPtr - (pointer to)bmp of supported type of API to call
*
* RETURNS:
*       GT_OK           -   on success
*       GT_BAD_PTR      -   bad pointer
*       GT_NOT_FOUND    -   device type is unknown
*       GT_FAIL         -   on failure
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*APP_DEMO_SYS_CONFIG_FUNCS_EXTENTION_GET_FUNC)
(
    IN  GT_U32                      deviceType,
    OUT APP_DEMO_SYS_CONFIG_FUNCS   *sysConfigFuncsPtr,
    OUT GT_U32                      *apiSupportedBmpPtr
);

/* pointer to the function that allow override of setting done by appDemoSysConfigFuncsGet */
extern APP_DEMO_SYS_CONFIG_FUNCS_EXTENTION_GET_FUNC appDemoSysConfigFuncsExtentionGetPtr;

/* XG and Cascade ports support*/
/*******************************************************************************
* internalPortConfig
*
* DESCRIPTION:
*       Internal function performs board-specific configurations of 10G
*       or cascade ports.
*
* INPUTS:
*       devNum     - Device Id.
*       portNum    - Port Number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*FUNCP_PORT_CONFIG)
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
);

typedef struct
{
    CPSS_CSCD_PORT_TYPE_ENT     cscdPortType;
    GT_TRUNK_ID                 trunkId;
    GT_U8                       portNum;
} CSCD_INIT_PORT_DESC_STC;

typedef struct
{
    CPSS_CSCD_LINK_TYPE_STC   linkToTargetDev;
    GT_U8                     targetDevNum;
} CSCD_INIT_UC_TBL_STC;

/*******************************************************************************
* APP_DEMO_ETH_PRT_INIT_NETIF_FUNC
*
* DESCRIPTION:
*       Initialize the network interface structures, Rx buffers and Tx header
*       buffers (For a single device).
*
* INPUTS:
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*APP_DEMO_ETH_PRT_INIT_NETIF_FUNC)
(
    void
);

/* Cascading support - end */

/* used by apiSupportedBmp */
#define APP_DEMO_DXCH_FUNCTIONS_SUPPORT_CNS         BIT_2
#define APP_DEMO_DXCH2_FUNCTIONS_SUPPORT_CNS        BIT_3
#define APP_DEMO_DXSAL_FUNCTIONS_SUPPORT_CNS        BIT_4
#define APP_DEMO_DXCH3_FUNCTIONS_SUPPORT_CNS        BIT_6
#define APP_DEMO_DXCH_XCAT_FUNCTIONS_SUPPORT_CNS    BIT_7
#define APP_DEMO_DXCH_LION_FUNCTIONS_SUPPORT_CNS    BIT_8
#define APP_DEMO_PX_FUNCTIONS_SUPPORT_CNS           BIT_9

#define API_SUPPORTED_BMP_MAC(devNum)\
    appDemoDbByDevNum(devNum)->apiSupportedBmp

/* check if device support API of dxch */
#define IS_API_CH_DEV_MAC(devNum) \
    (API_SUPPORTED_BMP_MAC(devNum) & APP_DEMO_DXCH_FUNCTIONS_SUPPORT_CNS)

/* check if device support API of dxch2 */
#define IS_API_CH2_DEV_MAC(devNum) \
    (API_SUPPORTED_BMP_MAC(devNum) & APP_DEMO_DXCH2_FUNCTIONS_SUPPORT_CNS)

/* check if device support API of dxch3 */
#define IS_API_CH3_DEV_MAC(devNum) \
    (API_SUPPORTED_BMP_MAC(devNum) & APP_DEMO_DXCH3_FUNCTIONS_SUPPORT_CNS)

/* check if device support API of dxsal */
#define IS_API_SAL_DEV_MAC(devNum) \
    (API_SUPPORTED_BMP_MAC(devNum) & APP_DEMO_SAL_FUNCTIONS_SUPPORT_CNS)

/* check if device support API of PX */
#define IS_API_PX_DEV_MAC(devNum) \
    (API_SUPPORTED_BMP_MAC(devNum) & APP_DEMO_PX_FUNCTIONS_SUPPORT_CNS)


struct APP_DEMO_EXTERNAL_TCAM_KEY_LOOKUP_STCT;

/**
* @enum APP_DEMO_CPSS_MULTI_PORT_GROUP_FDB_MODE_ENT
 *
 * @brief Enumeration of the FDB modes
*/
typedef enum{

    /** @brief all port groups have the
     *  same FDB entries. (unified tables)
     *  APP_DEMO_CPSS_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_LINKE_E - the port groups of
     *  the device are split to 2 groups .
     *  Each port group in those groups may hold different FDB entries.
     *  But the 2 groups hold the same entries.
     */
    APP_DEMO_CPSS_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E,

    APP_DEMO_CPSS_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_LINKED_E,

    /** @brief each port group of
     *  the device may hold different FDB entries.
     */
    APP_DEMO_CPSS_MULTI_PORT_GROUP_FDB_MODE_LINKED_E

} APP_DEMO_CPSS_MULTI_PORT_GROUP_FDB_MODE_ENT;

/**
* @struct APP_DEMO_CPSS_DMA_INIT_INFO
 *
 * @brief The structure holds info for DMA related pointers
 * done during 'PP logical init'.
 * This info needed for HIR feature to deallocate DMA memory
*/
typedef struct{

    /** network interface configuration */
    CPSS_NET_IF_CFG_STC netIfCfg;

    /**  multiNetIfCfg - Multi network interface configuration parameters **/
    CPSS_MULTI_NET_IF_CFG_STC multiNetIfCfg;

    /** Address Update Messages Queue CFG */
    CPSS_AUQ_CFG_STC auqCfg;

    /** FDB Upload Messages Queue CFG */
    CPSS_AUQ_CFG_STC fuqCfg;

} APP_DEMO_CPSS_DMA_INIT_INFO;


/**
* @struct APP_DEMO_CPSS_HW_ACCESS_DB_STC
 *
 * @brief This struct defines HW Access parameters and holds the data.
*/
typedef struct{

    /** @brief Holds the following access parameters in a bitmap:
     *  bit 7..0  device number
     *  bit 15..8  port group Id
     *  bit 16   ISR Context (1 - callback called from ISR context,
     *  0 - callback called not from ISR)
     *  bit 23..17 address space
     *  bit 24   Is the access for memory (1 - the access is for Memory,
     *  0 - the access is for Register)
     *  Note: Valid in case of bit0 is 1.
     *  bit 25   Memory end flag (1 - this access entry is memory end,
     *  0 - this access entry is not memory end)
     *  bit 26   access type ( 0 - read , 1 write)
     */
    GT_U32 accessParamsBmp;

    /** HW access address */
    GT_U32 addr;

    /** mask for read/written data */
    GT_U32 mask;

    /** The HW access read/written data */
    GT_U32 data;

} APP_DEMO_CPSS_HW_ACCESS_DB_STC;


/**
* @struct APP_DEMO_CPSS_HW_ACCESS_INFO_STC
 *
 * @brief This struct defines HW Access Info.
*/
typedef struct{

    /** The Max size allocated for HW Access DB in words. */
    GT_U32 maxSize;

    /** Current HW Access DB size in words */
    GT_U32 curSize;

    /** @brief Current DB state.
     *  GT_TRUE: trace HW Access to db enabled
     *  GT_FALSE: trace HW Access to db disabled
     */
    GT_BOOL outputToDbIsActive;

    /** @brief GT_TRUE: There was an attempt to write to a data base when data base was full
     *  GT_FALSE: There was no attempt to write to a data base when data base was full
     */
    GT_BOOL corrupted;

} APP_DEMO_CPSS_HW_ACCESS_INFO_STC;


/**
* @struct APP_DEMO_PP_CONFIG
 *
 * @brief Holds Pp configuration parameters for initialization.
*/
typedef struct
{
    CPSS_PP_DEVICE_TYPE             deviceId;
    GT_U32                          apiSupportedBmp;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;  /*added*/
    GT_BOOL                         ppPhase1Done;
    GT_BOOL                         ppPhase2Done;
    GT_BOOL                         ppLogicalInitDone;
    GT_BOOL                         ppGeneralInitDone;
    CPSS_PP_INTERFACE_CHANNEL_ENT   channel;
    GT_PCI_INFO                     pciInfo;
    GT_SMI_INFO                     smiInfo;
    GT_TWSI_INFO                    twsiInfo;
    GT_U8                           devNum;
    GT_U8                           oldDevNum;
    GT_HW_DEV_NUM                   hwDevNum;
    GT_PCI_INT                      intLine;
    GT_BOOL                         valid;
    APP_DEMO_SYS_CONFIG_FUNCS       sysConfigFuncs;
    /*XG support*/
    FUNCP_PORT_CONFIG               internal10GPortConfigFuncPtr;
    GT_PHYSICAL_PORT_NUM            ports10GToConfigureArr[CPSS_MAX_PORTS_NUM_CNS];
    GT_U32                          numberOf10GPortsToConfigure;
    /* Cascading support */
    GT_U8                           numberOfCscdTrunks;
    GT_U8                           numberOfCscdTargetDevs;
    GT_U32                          numberOfCscdPorts;
    CSCD_INIT_UC_TBL_STC            cscdTargetDevsArr[CPSS_CSCD_MAX_DEV_IN_CSCD_CNS];
    CSCD_INIT_PORT_DESC_STC         cscdPortsArr[CPSS_MAX_PORTS_NUM_CNS];
    FUNCP_PORT_CONFIG               internalCscdPortConfigFuncPtr;

    GT_BOOL                         externalTcamUsed;
    struct APP_DEMO_EXTERNAL_TCAM_KEY_LOOKUP_STCT   *extTcamMngPtr;
    GT_BOOL                         flowControlDisable;

    /* start - multi-port-groups support */
    GT_U32                          numOfPortGroups;
    struct{
        GT_BOOL                     portGroupActive;
        GT_PCI_INFO                 portGroupPciInfo;
        GT_SMI_INFO                 portGroupSmiInfo;

        GT_PORT_GROUPS_BMP          unifiedFdbPortGroupsBmp;
        GT_U32                      rxPolarityBmp;
        GT_U32                      txPolarityBmp;
    } portGroupsInfo[APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS];

    CPSS_PORTS_BMP_STC              multiPortGroupRingPortsBmp;/* debug info */
    CPSS_PORTS_BMP_STC              relayPortGroupPortsBmp;    /* debug info */
    CPSS_PORTS_BMP_STC              uplinkPortGroupPortsBmp;   /* debug info */
    APP_DEMO_CPSS_MULTI_PORT_GROUP_FDB_MODE_ENT fdbMode;
    /* end - multi-port-groups support */

    CPSS_NET_CPU_PORT_MODE_ENT      cpuPortMode;
    CPSS_RX_BUFF_ALLOC_METHOD_ENT   allocMethod;
    APP_DEMO_CPSS_DMA_INIT_INFO     allocDmaInfo;

    struct{
        GT_BOOL trunkToCpuPortNum;
    }wa;

    GT_U32                          maxPortNumber;
    GT_BOOL                         devSupportSystemReset_HwSoftReset;

    GT_U32                          portInitlist_AllPorts_used;

    GT_U32                          portsMapArrLen;
    GT_VOID                        *portsMapArrPtr;
}APP_DEMO_PP_CONFIG;

extern GT_U32             appDemoPpConfigDevAmount;

/* appDemoPpConfigList - Holds the Pp's configuration parameters.
   allocate additional entries for multi portgroup devices. Because the PCI/PEX
   scan fill in DB starting from first SW devNum and for all port groups
   the *2 was added to solve kw issue 5182*/
#define APP_DEMO_PP_CONFIG_SIZE_CNS (PRV_CPSS_MAX_PP_DEVICES_CNS + (CPSS_MAX_PORT_GROUPS_CNS*2 - 1))
extern APP_DEMO_PP_CONFIG appDemoPpConfigList[APP_DEMO_PP_CONFIG_SIZE_CNS];

/* dummy devNum for non initialized value for "CPU Ethernet port" device */
#define NOT_INIT_CPU_ETHER_DEV  0xff

/**
* @struct APP_DEMO_CPU_ETHER_PORT_INFO_STC
 *
 * @brief Holds CPU Ether port configuration parameters for initialization.
 * relevant to system with CPU Ethernet port.
 * (in board with multi devices , only 1 device
 * connected to CPU)
*/
typedef struct{

    GT_U8 devNum;

    /** @brief Rx buffers allocation information.
     *  txHdrBlock_PTR - ptr to the Tx header buffer block.
     *  txHdrBlockSize - Tx header buffer block size.
     */
    CPSS_RX_BUF_INFO_STC rxBufInfo;

    GT_U32 *txHdrBufBlock_PTR;

    GT_U32 txHdrBufBlockSize;

    /** init function */
    APP_DEMO_ETH_PRT_INIT_NETIF_FUNC initFunc;

} APP_DEMO_CPU_ETHER_PORT_INFO_STC;

/**
* @struct APP_DEMO_SYS_CONFIG_STC
 *
 * @brief Holds system configuration parameters for initialization.
*/
typedef struct{

    /** @brief is the system use the cpu Ethernet port.
     *  (in board with multi devices , only 1 device
     *  connected to CPU)
     */
    GT_BOOL cpuEtherPortUsed;

    /** @brief info about the cpu Ether port
     *  relevant to system with CPU Ethernet port.
     */
    APP_DEMO_CPU_ETHER_PORT_INFO_STC cpuEtherInfo;

    /** @brief device number for the first device on board.
     *  ability to set different device numbers for stacking
     */
    GT_U8 firstDevNum;

    /** @brief the appDemo support/not support the AA messages to CPU
     *  GT_FALSE - not support processing of AA messages
     *  GT_TRUE - support processing of AA messages
     */
    GT_BOOL supportAaMessage;

    /** indication that the system force to be in auto learning */
    GT_BOOL forceAutoLearn;

} APP_DEMO_SYS_CONFIG_STC;

extern APP_DEMO_SYS_CONFIG_STC appDemoSysConfig;

/* convert local devNum got system devNum
 --> ability to set different device numbers for stacking */
#define SYSTEM_DEV_NUM_MAC(_devNum) (GT_U8)((_devNum)+appDemoSysConfig.firstDevNum)
/* Reverse macro */
#define SYSTEM_DEV_IDX_MAC(_devNum) (GT_U8)((_devNum)-appDemoSysConfig.firstDevNum)

#ifdef CHX_FAMILY
/* check that port has valid mapping or not CPU port */
#define CPSS_ENABLER_PORT_SKIP_CHECK(dev,port)                                                   \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(dev) == GT_TRUE)                    \
    {                                                                                            \
        GT_BOOL   _isCpu, _isValid;                                                                        \
        GT_STATUS _rc, _rc1;                                                                           \
        if ((port) >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))                                \
            break; /* no more */                                                                 \
        _rc = cpssDxChPortPhysicalPortMapIsCpuGet(dev, port, &_isCpu);                           \
        _rc1 = cpssDxChPortPhysicalPortMapIsValidGet(dev, port, &_isValid);                      \
        if((_rc != GT_OK) || (_isCpu == GT_TRUE) ||(_rc1 != GT_OK) || (_isValid != GT_TRUE ) )   \
            continue;                                                                            \
    }                                                                                            \
    else                                                                                         \
    {                                                                                            \
        if(PRV_CPSS_PP_MAC(dev)->phyPortInfoArray[port].portType == PRV_CPSS_PORT_NOT_EXISTS_E)  \
            continue;                                                                            \
    }
#endif


/*
 * Typedef: FUNCP_CPSS_MODULE_INIT
 *
 * Description:
 *      Specific CPSS module initialization routine.
 *
 * Fields:
 *      dev                 -   device number
 *      libInitParamsPtr    -   module initialization parameters
 *      sysConfigParamsPtr  -   system configuration parameters
 *
 */
typedef GT_STATUS (*FUNCP_CPSS_MODULE_INIT)
(
    IN  GT_U8                       dev,
    IN  APP_DEMO_LIB_INIT_PARAMS    *libInitParamsPtr,
    IN  CPSS_PP_CONFIG_INIT_STC     *sysConfigParamsPtr
);



/**
* @internal appDemoSysGetPciInfo function
* @endinternal
*
* @brief   Gets the Pci info for the mini-application usage.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoSysGetPciInfo
(
    GT_VOID
);

/**
* @internal appDemoHotInsRmvSysGetPciInfo function
* @endinternal
*
* @brief   Gets the Pci info for the mini-application usage.
* IN devIdx                        -   device number
* IN pciBus                        -   PCI Bus number
* IN pciDev                        -   PCI device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoHotInsRmvSysGetPciInfo
(
    IN GT_U8 devIdx,
    IN GT_U8 pciBus,
    IN GT_U8 pciDev
);

/**
* @internal appDemoSysGetSmiInfo function
* @endinternal
*
* @brief   Gets the SMI info for the mini-application usage.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoSysGetSmiInfo
(
    GT_VOID
);

/**
* @internal appDemoSysGetTwsiInfo function
* @endinternal
*
* @brief   Gets the SMI info for the mini-application usage.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoSysGetTwsiInfo
(
    GT_VOID
);

/**
* @internal appDemoSysConfigFuncsGet function
* @endinternal
*
* @brief   Gets the system configuration functions accordingly to given device type.
*
* @param[in] deviceType               - device type.
*
* @param[out] sysConfigFuncsPtr        - system configuration functions.
* @param[out] apiSupportedBmpPtr       - (pointer to)bmp of supported type of API to call
*
* @retval GT_OK                    -  on success
* @retval GT_BAD_PTR               -  bad pointer
* @retval GT_NOT_FOUND             -  device type is unknown
* @retval GT_FAIL                  -  on failure
*/
GT_STATUS appDemoSysConfigFuncsGet
(
    IN  GT_U32                      deviceType,
    OUT APP_DEMO_SYS_CONFIG_FUNCS   *sysConfigFuncsPtr,
    OUT GT_U32                      *apiSupportedBmpPtr
);

/**
* @internal appDemoDxChFdbFlush function
* @endinternal
*
* @brief   Deletes all addresses from FDB table.
*
* @param[in] dev                      - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS appDemoDxChFdbFlush
(
    IN GT_U8   dev
);

/**
* @internal appDemoDxChXcatSgmii2_5GbNetworkPortWa function
* @endinternal
*
* @brief   The workaround for SGMII 2.5GB
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] devNum                   - physical device number
* @param[in] portsBmp                 - physical ports bitmap
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_RESOURCE           - memory resource not available.
*/
GT_STATUS appDemoDxChXcatSgmii2_5GbNetworkPortWa
(
    GT_U8   devNum,
    GT_U32  portsBmp
);

/**
* @internal appDemoDxChPowerDownBmpSet function
* @endinternal
*
* @brief   Sets AppDemo power down SERDES bitmask.
*
* @param[in] powerDownBmp             - the value to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDxChPowerDownBmpSet
(
    IN GT_U32   powerDownBmp
);

/**
* @internal appDemoHwDevNumOffsetSet function
* @endinternal
*
* @brief   Sets value of appDemoHwDevNumOffset used during device HW ID calculation
*         based on device SW ID..
* @param[in] hwDevNumOffset           - value to save into appDemoHwDevNumOffset.
*                                       GT_OK
*/
GT_STATUS appDemoHwDevNumOffsetSet
(
    IN GT_U8 hwDevNumOffset
);

/**
* @internal appDemoDxChLion2SerdesAutoTuningRun function
* @endinternal
*
* @brief   Perform SERDES auto tune (RX training) on port if:
*         1. the port in link-up.
*         2. not CPU port
*         3. auto tune was not performed already
* @param[in] devNum                   - device number
* @param[in] portNum                  - global port number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS appDemoDxChLion2SerdesAutoTuningRun
(
    IN   GT_U8                            devNum,
    IN   GT_PHYSICAL_PORT_NUM             portNum
);


/**
* @internal appDemoDxChLion2PortLinkWa function
* @endinternal
*
* @brief   For 40G interfaces execute WA - connect/disconnect leaves link down problem
*         For 10/20G interfaces execute WA - false link up with dismatching interface
* @param[in] devNum                   - device number
* @param[in] portNum                  - global port number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS appDemoDxChLion2PortLinkWa
(
    IN   GT_U8                            devNum,
    IN   GT_PHYSICAL_PORT_NUM             portNum
);

/**
* @internal appDemoDxChLion2SerdesRxTrainigRun function
* @endinternal
*
* @brief   Run RX training only on port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - global port number
* @param[in] interface                - current  of port
*                                      speed   - current speed of port
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS appDemoDxChLion2SerdesRxTrainigRun
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    interface
);

/**
* @internal appDemoPfcEnableConfiguration function
* @endinternal
*
* @brief   Init PFC feature - with following static configuration - during init.
*         - 10G ports are supported only
*         - two traffic classes in PFC mode - fixed TC5 and TC6
*         - shared memory buffer management
*         - PFC counting mode in packets
*         - Token bucket baseline configuration - 0x3FFFC0
*         - for all ports:
*         . flow control global configuration: CPSS_PORT_FLOW_CONTROL_RX_TX_E
*         . flow control mode: CPSS_DXCH_PORT_FC_MODE_PFC_E
*         . periodic PFC enabled
*         . all ports are binded to PFC profile#1
*         - for PFC traffic classes (TC5 and TC6) - set PFC static configuration
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoPfcEnableConfiguration
(
    IN  GT_U8    devNum
);

/*******************************************************************************
* appDemoDbByDevNum
*
* DESCRIPTION:
*       get the AppDemo DB entry for the 'devNum'.
*
* INPUTS:
*       devNum - the 'cpss devNum'
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the appDemoDb
*
* COMMENTS:
*       None.
*
*******************************************************************************/
APP_DEMO_PP_CONFIG* appDemoDbByDevNum(IN GT_U8 devNum);

/**
* @internal waTriggerInterruptsMask function
* @endinternal
*
* @brief   Mask/unmask specific interrupts on port that can trigger WA's restart
*         or other not needed for now actions.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - current interface on port
* @param[in] operation                - mask/unmask
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Affected interrupts: CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
*       CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
*       CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E.
*
*/
GT_STATUS waTriggerInterruptsMask
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
);


/**
* @internal waTriggerInterruptsMaskNoLinkStatusChangeInterrupt function
* @endinternal
*
* @brief   Mask/unmask specific interrupts on port that can trigger WA's restart
*         or other not needed for now actions.
*         This function is almost the same as waTriggerInterruptsMask but uses
*         When it is no signal to not unmask CPSS_PP_PORT_LINK_STATUS_CHANGED_E interrupt
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - current interface on port
* @param[in] operation                - mask/unmask
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Affected interrupts: CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
*       CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E.
*
*/
GT_STATUS waTriggerInterruptsMaskNoLinkStatusChangeInterrupt
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
);


/**
* @internal appDemoDxChPortBc2PtpTimeStampFixWa function
* @endinternal
*
* @brief   Set the thresholds in ports Tx FIFO
*
* @note   APPLICABLE DEVICES:      Bobcat2
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note WA fix PTP timestamp problem
*
*/
GT_STATUS appDemoDxChPortBc2PtpTimeStampFixWa
(
    IN   GT_U8                            devNum
);

/**
* @internal appDemoDxAldrin2TailDropDbaEnableConfigure function
* @endinternal
*
* @brief   Enables/disables Tail Drop DBA. Relevant for Aldrin2 only, does nothing for other devices.
*
* @param[in] dev                      -   device number
* @param[in] enable                   -   enable (GT_TRUE) or disable (GT_FALSE) Tail Drop DBA
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoDxAldrin2TailDropDbaEnableConfigure
(
    IN  GT_U8     dev,
    IN  GT_BOOL   enable
);

/**
* @internal appDemoDxAldrin2TailDropDbaEnableSet function
* @endinternal
*
* @brief   Enables/disables Tail Drop DBA.
*          Relevant for Aldrin2 only, does nothing for other devices.
*          On first call saves the found DBA enable state for restore.
*
* @param[in] dev                      -   device number
* @param[in] enable                   -   enable (GT_TRUE) or disable (GT_FALSE) Tail Drop DBA
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoDxAldrin2TailDropDbaEnableSet
(
    IN  GT_U8     dev,
    IN  GT_BOOL   enable
);

/**
* @internal appDemoDxAldrin2TailDropDbaEnableRestore function
* @endinternal
*
* @brief   Restores Tail Drop DBA enable state. Relevant for Aldrin2 only, does nothing for other devices.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoDxAldrin2TailDropDbaEnableRestore
(
    IN  GT_U8     dev
);

/**
* @internal appDemoDxChSkipSip6RemotePorts function
* @endinternal
*
* @brief   falcon in port mode 1024 use remote ports that are 
*          not bind to any device
*
* @note   APPLICABLE DEVICES:      All.
* @param[in] devNum                - device number 
* @param[in] portNum               - port number 
*
* @retval GT_TRUE                    - on success
* @retval GT_FALSE                  - on error
*/
GT_BOOL appDemoDxChSkipSip6RemotePorts
(
     GT_U8   devNum,
     GT_U32  portNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtAppDemoSysConfigh */


