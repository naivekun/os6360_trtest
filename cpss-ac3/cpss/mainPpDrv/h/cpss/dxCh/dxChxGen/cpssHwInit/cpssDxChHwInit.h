/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssDxChHwInit.h
*
* @brief Includes CPSS level basic Hw initialization functions, and data
* structures.
*
* The DxCh driver supports next 'families' :
* ---  DxCh    (first generation) :
* 10 GE ports : 98DX107
* 24GE+3XG ports : 98DX270,98DX273,98DX249,98DX269,98DX169
* 24GE+2XG ports : 98DX260,98DX263,98DX262,98DX268
* 24GE   ports : 98DX250,98DX253,98DX243,98DX248
* 16GE   ports : 98DX163,98DX166,98DX167
* 12GE+1XG ports : 98DX133
* 3XG   ports : 98DX803
* subFamilies :
* DxCh-Diamond :
* 10GE ports : 98DX106
*
* ---  DxCh2    (second generation) :
* 24GE+4XG ports : 98DX285
* 24GE+3XG ports : 98DX275
* 24GE+2XG ports : 98DX265
* 24GE   ports : 98DX255
* 12GE+1XG ports : 98DX145
* 4XG   ports : 98DX804
*
* ---  DxCh3    (3'rd generation)  :
* 24GE+4XG ports : 98DX5128,98DX5128_1,98DX287,98DX5129
* 24GE+2XG ports : 98DX5126,98DX5127
* 24GE   ports : CPSS_98DX5124_CNS
* 24 1/2.5 GE + 2XG ports : 98DX5156
* 24 1/2.5 GE + 2XG + 2XGS ports : 98DX5157
* 16 1/2.5 GE ports + 2XG : 98DX5154
* 16 1/2.5 GE ports + 2XG + 2XGS ports : 98DX5155
* 8 1/2.5 GE ports + 2XG : 98DX5152
* 4 1/2.5 GE ports + 2XG : 98DX5151
* 8 XG or 6XG + 2 Stack : 98DX8108,98DX8109
* 10 XG or 6XG + 4 Stack : 98DX8110,98DX8110_1,98DX8111
*
* ---  DxCh xCat  (4'th generation)  :
*
* FE    GE    Stack Stack   Stack   Stack
* GE   GE/2.5GE GE/2.5/  GE/2.5/
* (DQX)   10/12GE  5/10/12GE
* (DQX/DHX) (DQX/DHX/XAUI)
* 98DX1000 Series: AlleyCat-FE WEBSMART- 333MHz, 0K Cache, SPI, DDR2
* 98DX1025   24    0    4    0    0    0
* 98DX1026   24    0    2    2    0    0
* 98DX1027   24    0    2    2    0    0
*
* 98DX1100 Series: TomCat-FE (SMB L2+) - 600MHz, 16KBKB/128KB Cache
* 98DX1122   24    0    0    4    0    0
* 98DX1123   24    0    0    4    0    0
*
* 98DX2100 Series: Bobcat-FE (L3+ / Metro) - 800MHz, , 16KB/256K Cache
* 98DX2101   8     0    0    4    0    0
* 98DX2122   24    0    0    4    0    0
* 98DX2123   24    0    0    4    0    0
* 98DX2142   48    0    0    4    0    0
* 98DX2151   24    0    0    2    0    2
*
* 98DX3000 Series: AlleyCat-GE (Entry L2) UNMANAGED- 333MHz, 0K Cache, 256KB Internal SRAM
* 98DX3011   0    16    2    0    0    0
* 98DX3021   0    24    2    0    0    0
*
* 98DX3000 Series: AlleyCat-GE (Entry L2) - 333MHz, , 0K Cache, 128KB memory
* 98DX3001   0     8    2    0    0    0
* 98DX3010   0    16    0    0    0    0
* 98DX3011   0    16    2    0    0    0
* 98DX3020   0    24    0    0    0    0
* 98DX3021   0    24    2    0    0    0
*
* 98DX3100 Series: TomCat-GE (SMB L2+) - 600MHz, 16KBKB/128KB Cache
* 98DX3101   0     8    2    0    0    0
* 98DX3121   0    24    0    2    2    0
* 98DX3122   0    24    0    0    0    4
* 98DX3123   0    24    0    0    0    4
* 98DX3124   0    24    0    0    2    2
* 98DX3125   0    24    0    0    2    2
*
* 98DX4100 Series: Bobcat-GE (L3+ / Metro) - 800MHz,1,16KB/256KB Cache
* 98DX4101   0     8    0    4    0    0
* 98DX4121   0    24    0    4    0    0
* 98DX4122   0    24    0    0    0    4
* 98DX4123   0    24    0    0    0    4
* 98DX5158   0    24    0    0    0    4
* 98DX5159   0    24    0    0    0    4
*
*
* @note the statements of :
* 1. 'All DxCh Devices' means that the API applicable to all the
* listed 'families' and all their sub devices
* 2. 'DxCh2 and above' means that the API applicable to the
* listed 'families' below DxCh2 (including DxCh2) and all their
* sub devices (DxCh2 , DxCh3 , DxCh xCat ,...)
* 3. 'DxCh3 and above' means that the API applicable to the
* listed 'families' below DxCh3 (including DxCh3) and all their
* sub devices (DxCh3 , DxCh xCat ,...)
* 4. 'DxCh xCat and above' means that the API applicable to the
* listed 'families' below DxCh xCat (including DxCh xCat) and all their
* sub devices (DxCh xCat , Lion...)
* 5. 'Lion and above' means that the API applicable to the
* listed 'families' below Lion (including Lion) and all their
* sub devices (Lion ,...)
*
*
*
* @version   59
********************************************************************************
*/

#ifndef __cpssDxChHwInith
#define __cpssDxChHwInith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInfo.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChTables.h>

/* Defines the max number SRAMs.
The number includes CTRL SRAM (wide SRAM) and routing SRAM (narrow SRAM)*/
#define CPSS_DXCH_MAX_NUMBER_OF_EXTERNAL_SRAMS_CNS 5

/* Use this constant to retrieve core clock value from HW. */
#define CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS 0xFFFFFFFF


/*  number of AU messages that application need to allocate as extra DMA cache
*   memory for the AUQ (extra to the size needed for the regular application
*   requirements) that will be use by the CPSS , for a secondary AUQ to workaround (WA)
*   the problem described for PRV_CPSS_DXCH_FER_FDB_AUQ_LEARNING_AND_SCANING_DEADLOCK_WA_E
*
*   NOTE: actual size in bytes is :
*       --> CPSS_DXCH_PP_FDB_AUQ_DEADLOCK_EXTRA_MESSAGES_NUM_WA_CNS * descSizeInBytes
*       where descSizeInBytes retrieved from :
*       cpssDxChHwAuDescSizeGet(devNum,&descSizeInBytes);
*
*/
#define CPSS_DXCH_PP_FDB_AUQ_DEADLOCK_EXTRA_MESSAGES_NUM_WA_CNS  64

/**
* @enum CPSS_DXCH_PP_SERDES_REF_CLOCK_ENT
 *
 * @brief Defines SERDES reference clock type.
*/
typedef enum{

    /** @brief 25 MHz external
     *  single ended reference clock.
     */
    CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,

    /** @brief 125 MHz external
     *  single ended reference clock.
     */
    CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_125_SINGLE_ENDED_E,

    /** @brief 125 MHz external
     *  differential reference clock.
     */
    CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_125_DIFF_E,

    /** @brief 156.25 MHz external
     *  single ended reference clock.
     */
    CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_SINGLE_ENDED_E,

    /** @brief 156.25 MHz external
     *  differential reference clock.
     */
    CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,

    /** 125 MHz internal reference clock */
    CPSS_DXCH_PP_SERDES_REF_CLOCK_INTERNAL_125_E

} CPSS_DXCH_PP_SERDES_REF_CLOCK_ENT;

/**
* @struct CPSS_DXCH_PP_PHASE1_INIT_INFO_STC
 *
 * @brief This struct defines specific hardware parameters, those parameters are
 * determined according to the board specific hardware components.
*/
typedef struct{

    /** @brief Temporary device number to allow basic communication.
     *  (APPLICABLE RANGES: 0..31)
     *  hwInfo[]     - HW info (bus type, bus address, mappings, interrupts, etc)
     *  Look CPSS_HW_INFO_STC description for details
     */
    GT_U8 devNum;

    CPSS_HW_INFO_STC hwInfo[CPSS_MAX_PORT_GROUPS_CNS];

    /** @brief number of port groups that the device support , used to know
     *  the number of elements in the array of multiPortGroupsInfoPtr.
     *  For no multi-port-groups devices must be set to 1
     */
    GT_U32 numOfPortGroups;

    /** @brief The PP core clock in MHz.
     *  CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS may be used for
     *  core clock auto detection.
     */
    GT_U32 coreClock;

    /** @brief Management interface type (PCI/SMI/TWSI/PEX).
     *  This parameter is used to select driver which perform
     *  low-level access to Switching address space
     */
    CPSS_PP_INTERFACE_CHANNEL_ENT mngInterfaceType;

    /** CPU High Availability mode(Active or Standby). */
    CPSS_SYS_HA_MODE_ENT ppHAState;

    /** SERDES reference clock type. */
    CPSS_DXCH_PP_SERDES_REF_CLOCK_ENT serdesRefClock;

    /** @brief it's ignored for all not xCat devices
     *  it's ignored for xCat devices without internal CPU
     *  it's relevant only for xCat devices with internal CPU
     *  GT_TRUE - External CPU is connected to the PP. CPSS configures PP to work with external CPU
     *  even if internal CPU is active
     *  GT_FALSE- External CPU is not connected to the PP (only internal CPU active)
     */
    GT_BOOL isExternalCpuConnected;

    /** @brief GT_TRUE  - enable all TCAMs parity calculation
     *  GT_FALSE - disable all TCAMs parity calculation
     *  To enable parity checks by device, use cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_BOOL tcamParityCalcEnable;

    /** @brief Bitmap of configurable address completion regions
     *  used by interrupt handling routine.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 isrAddrCompletionRegionsBmp;

    /** @brief Bitmap of configurable address completion regions
     *  used by CPSS API.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
     *  Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X PPs that have 8 address completion regions.
     *  Region 0 provide access to the lower addresses that contain also
     *  configuration of address completion regions. It must never be configured.
     *  Lists (coded as ranges) of other regions can be assigned for several CPUs
     *  for interrupt handling (for such CPU that processes the PP interrupts)
     *  and for all other CPSS API using.
     *  These assigned ranges of regions must not have common members.
     *  These assigned ranges of regions must not contain zero range.
     */
    GT_U32 appAddrCompletionRegionsBmp;

    /** @brief GT_TRUE  - enable VPLS mode support
     *  GT_FALSE - disable VPLS mode support
     *  (APPLICABLE DEVICES: xCat, xCat3).
     *  xCat device: applicable starting from revision C0
     */
    GT_BOOL enableLegacyVplsModeSupport;

    /** @brief Set the number of physical ports that the device should support.
     *  (APPLICABLE DEVICES: Bobcat3).
     *  Bobcat3 device supports next value:
     *  0  - means default of 256 ports mode
     *  256 - means 256 ports mode
     *  512 - means 512 ports mode
     *  In Bobcat3 there is tradeoff between number of physical ports and some tables
     *  that hold all physical ports 'per entry'
     *  So Bobcat3 can support 256 physical ports like Bobcat2 :
     *  8K vlans , 4K STG , 4K trunks , 4K SST-ID
     *  Or Bobcat3 can support 512 physical ports :
     *  4K vlans, 2K STG , 2K trunks , 2K SST-ID , single device (8bit port-id in eDSA)
     *  NOTE: Next tables keep 256 ports only !!!  :
     *  L2 Port Isolation , L3 Port Isolation
     */
    GT_U32 maxNumOfPhyPortsToUse;

    /** @brief number of elements in dataIntegrityShadowPtr
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  NOTE :
     *  1. can use value CPSS_DXCH_SHADOW_TYPE_AS_MANY_HW_CNS
     *  in this case dataIntegrityShadowPtr is ignored !
     *  2. can use value CPSS_DXCH_SHADOW_TYPE_ALL_CPSS_CNS
     *  in this case dataIntegrityShadowPtr is ignored !
     */
    GT_U32 numOfDataIntegrityElements;

    /** @brief  GT_TRUE - Cut Through Enable, GT_FALSE - Cut Through Disable
     *  Configuration to optimize packet buffers utilization.
     *  It's impossible to configure Cut Through on any port if specified GT_FALSE.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2)
     */
    GT_BOOL cutThroughEnable;

    /** @brief (pointer to) list of logical tables that needs shadow support
     *  (needed for data integrity)
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_LOGICAL_TABLES_SHADOW_STC *dataIntegrityShadowPtr;

} CPSS_DXCH_PP_PHASE1_INIT_INFO_STC;
/* temporary, until next release */
#define CPSS_DXCH_PP_PHASE1_INIT_INFO_NEW_STC CPSS_DXCH_PP_PHASE1_INIT_INFO_STC

/**
* @struct CPSS_DXCH_PP_PHASE2_INIT_INFO_STC
 *
 * @brief PP Hw phase2 init Parameters.
*/
typedef struct{

    /** The new device number to assign to this PP. */
    GT_U8 newDevNum;

    /** Network interface configuration parameters. */
    CPSS_NET_IF_CFG_STC netIfCfg;

    /** @brief Address Update Queue configuration parameters.
     *  NOTE:
     *  1. when useSecondaryAuq = GT_TRUE , the cpss will use
     *     644 words from the AUQ for the 'Secondary AUQ'.
     *  2. next multi port group devices hold next number of AUQs:
     *      Lion2 : AUQ per port group.
     *      BC3 : single AUQ.
     *      Falcon : AUQ per every 2 port groups.
     *      the CPSS split the DMA memory between the multiple AUQs.
     */
    CPSS_AUQ_CFG_STC auqCfg;

    /** @brief GT_TRUE
     *  - GT_FALSE - use Address Update queue for FDB Upload messages
     *  The parameter used only for DxCh2 and above devices and
     *  ignored for another devices
     */
    GT_BOOL fuqUseSeparate;

    /** @brief FDB Upload Queue configuration parameters.The parameter
     *  used only for DxCh2 and above devices and ignored
     *  for another devices
     *  NOTE:
     *  1. next multi port group devices hold next number of FUQs:
     *      Lion2 : FUQ per port group.
     *      BC3 : single FUQ.
     *      Falcon : FUQ per port group.
     *      the CPSS split the DMA memory between the multiple FUQs.
     *      (to support CNC upload)
     */
    CPSS_AUQ_CFG_STC fuqCfg;

    /** @brief a flag to indicate that the application needs the cpss
     *  to implement the WA for the triggered action
     *  completion while AU queue is full. The triggered FDB action
     *  cannot be completed till AU queue is full. The 'Secondary AUQ'
     *  may be used in order to fix the problem.
     *  the Cpss will use a 'Secondary AUQ' , and for that will
     *  remove CPSS_DXCH_PP_FDB_AUQ_DEADLOCK_EXTRA_MESSAGES_NUM_WA_CNS
     *  entries from the AUQ.
     *  GT_TRUE - cpss need to implement the WA for the triggered FDB action
     *  cannot be completed till AU queue is full issue.
     *  cpss will use 644 words from the AUQ (see auqCfg) for that
     *  'Secondary AUQ'.
     *  GT_FALSE - cpss WILL NOT implement the WA for the triggered FDB action
     *  cannot be completed till AU queue is full issue.
     *  cpss WILL NOT use 644 words from the AUQ (see auqCfg) for that
     *  'Secondary AUQ'.
     *  When useDoubleAuq == GT_TRUE, CPSS will use 2644 words
     *  from the AUQ (see auqCfg) for that 'Secondary AUQ'.
     */
    GT_BOOL useSecondaryAuq;

    GT_BOOL noTraffic2CPU;

    /** @brief value of port group ID used for CPU network
     *  interface (relevant only for multi-port group devices while
     *  working in both(!!!) MII and SDMA mode).
     *  Relevant only if useMultiNetIfSdma == GT_FALSE.
     */
    GT_U32 netifSdmaPortGroupId;

    /** @brief The AU massage length. Extending the AU message increases
     *  the size of the message to more than 16 bytes
     */
    CPSS_AU_MESSAGE_LENGTH_ENT auMessageLength;

    /** @brief Support configuration of two AUQ memory regions.
     *  GT_TRUE - CPSS manages two AU Queues with the same size:
     *  auqCfg->auDescBlockSize / 2.
     *  GT_FALSE - CPSS manages single AU Queue with size:
     *  auqCfg->auDescBlockSize.
     */
    GT_BOOL useDoubleAuq;

    /** @brief Enable Multi
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL useMultiNetIfSdma;

    /** @brief Multi
     *  parameters.
     *  Relevant only if useMultiNetIfSdma == GT_TRUE.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
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
     *  system supports cache-coherency between the CPU & the
     *  PCI, those blocks can be in a cacheble area with no
     *  special handling
     *  If there is no cache coherency, then:
     *  1) the Rx/Tx descriptors must be allocated from non-cachable
     *  memory.
     *  2) the Rx data buffers and AUQ may be from cachable memory,
     *  if the user flushes the buffer memory cache prior to
     *  returning the buffer to the Rx pool, and cache-line
     *  reads do not cross data buffer boundaries.
     *  3.  The provided auDescBlock will be divided into AU descriptors. The
     *  size of each AU descriptor can be obtained via
     *  cpssDxChHwAuDescSizeGet(), The number of address update descriptors
     *  is calculated by:(auDescBlockSize / <size of a single descriptor> ).
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
     *  calling cpssDxChHwRxDescSizeGet().
     *  rxBufSize in the above calculations is received via TAPI
     *  sysConfig.h sysConfig() API.
     *  5.  The number of Tx descriptors to be allocated per Tx queue will be
     *  calculated as follows:
     *  (txDescBlockSize / <size of a single Tx desc.>) /
     *  <number of Rx queues>.
     *  The size of a single Tx descriptor can be obtained by calling
     *  cpssDxChHwTxDescSizeGet().
     *  6.  DxCh3 and above devices can upload the CNC 2048 counters block to the
     *  same FDB Upload Queue (fuqCfg). Two CNC counters takes place of one
     *  FU message.To upload whole CNC block the place for 1024 FU messages
     *  need to be allocated.
     *  7. about 'multi port groups' device :
     *  a. netIfCfg : Cpss use all SMDA relate allocation with port group
     *  defined by netifSdmaPortGroupId that will be the only one used
     *  for rx/tx traffic from/to cpu.
     *  b. auqCfg : Cpss divide the DMA space evenly between all active
     *  port groups.
     *  Because there is no logic to divide it non-evenly (also for B0
     *  with 'split FDB mode')
     *  c. fuqUseSeparate : Apply to all port groups.
     *  d. fuqCfg : Cpss divide the DMA configuration between all active
     *  port groups (to support CNC , and also for B0 with 'split FDB
     *  mode').
     */
    CPSS_MULTI_NET_IF_CFG_STC multiNetIfCfg;

} CPSS_DXCH_PP_PHASE2_INIT_INFO_STC;


/**
* @struct CPSS_DXCH_ADDR_DECODE_WINDOW_CONFIG_STC
 *
 * @brief The structure defines the address decoding window fields.
 * Address driven by one of the PP unit (e.g. accessing DDR via PCIe) must hit
 * one of the windows in order to be driven outside the PP e.g. on the PCIe.
 * Some of the windows help to generate full 64bit address.
 * PP units generate a 32 bits physical address to target unit. If the driven address
 * hit the window, then the PP will prefix the driven address with the
 * configured 32 bits (remap address high).
*/
typedef struct{

    /** @brief GT_TRUE to enable window, GT_FALSE to disable window.
     *  if GT_FALSE other fields ignored.
     */
    GT_BOOL enableWindow;

    /** Physical base address of the window, must be aligned to the window size. */
    GT_U32 baseAddress;

    /** @brief Size of the window to remap, in steps of 64K. Must a power of 2.
     *  i.e. 1 stands for 64K, 2 for 128K, 4 for 512K and 64K for 4G window size.
     */
    GT_U32 windowSize;

    /** @brief Specifies physical address bits[63:32] to be driven to the target interface.
     *  Relevant only for target interfaces that support more than 4 GB of address space.
     *  This field ignored for windows 5 and 6, which do not support remapping to 64bit addressing mode.
     */
    GT_U32 remapAddress;

} CPSS_DXCH_ADDR_DECODE_WINDOW_CONFIG_STC;


/**
* @enum CPSS_DXCH_IMPLEMENT_WA_ENT
 *
 * @brief enumeration for the types of WAs (workarounds that CPSS need to do) for
 * the DxCh devices
*/
typedef enum{

    /** @brief WA for xCat A1
     *  "wrong trunk id source port information of packet to CPU" erratum.
     *  NOTE: the request for this WA (if needed) must be before calling
     *  cpssDxChTrunkInit(...)
     */
    CPSS_DXCH_IMPLEMENT_WA_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E,

    /** @brief WA for xCat A1
     *  "packets of size 256x4 sent from CPU stack SDMA" erratum.
     *  NOTE: the request for this WA (if needed) must be before calling
     *  cpssDxChNetIfInit(...)
     */
    CPSS_DXCH_IMPLEMENT_WA_SDMA_PKTS_FROM_CPU_STACK_E,

    /** @brief WA for xCat A1
     *  The CPU may hang when there is Register read in parallel with
     *  one of the following:
     *  AU message
     *  SDMA – packet to / from CPU.
     *  To avoid this problem, Register Read the AU Fifo instead
     *  of using the AU DMA.
     *  The WA influents only AU messages treatment.
     *  An application should use cpssDxChHwPpImplementWaInit in order
     *  to enable WA for SDMA packet to/from CPU -
     *  CPSS_DXCH_IMPLEMENT_WA_SDMA_PKTS_FROM_CPU_STACK_E.
     *  NOTE: the request for this WA (if needed) must be before calling
     *  cpssDxChCfgPpLogicalInit (...).
     */
    CPSS_DXCH_IMPLEMENT_WA_FDB_AU_FIFO_E,

    /** @brief WA for Lion.
     *  FDB messages of : AA,TA,QR,FU related to entry of 'port' of 'own device' ,
     *  may be received in CPU from all port groups , and not only from
     *  the port group of the 'port'.
     *  the Cpss will filter those 'duplicated' messages , so application will not be aware to them.
     *  NOTEs:
     *  1. the request for this WA (if needed) should be before calling cpssDxChBrgFdbInit(...)
     *  2. the messages still occupy the AU/FU queue, until removed by cpss.
     *  a call to: cpssDxChBrgFdbAuMsgBlockGet(...) will remove those from AUQ.
     *  a call to: cpssDxChBrgFdbFuMsgBlockGet(...) will remove those from FUQ.
     *  a call to: cpssDxChBrgFdbAuqFuqMessagesNumberGet(...) will return
     *  value of 'num of messages' as if those messages not exists.
     *  3. for device with HW support of this filter , a call to this WA not needed.
     *  but when WA called , the CPSS will disable the HW filter support !
     */
    CPSS_DXCH_IMPLEMENT_WA_FDB_AU_FU_FROM_NON_SOURCE_PORT_GROUP_E,

    /** @brief WA for XCAT
     *  A1 and Lion.
     *  Traffic from the CPU via SDMA mechanism can corrupt the switch
     *  internal buffer memory.
     *  To avoid this, traffic sent from the CPU is padded with 8 bytes of
     *  zero’s, for byte counts of <byteblock>n+k (1<=k<=8) (including
     *  DSA tag), where n and k are integers and <byteblock> is 256 for
     *  xCat and 512 for Lion.
     *  NOTE: the request for this WA (if needed) must be after calling
     *  cpssDxChHwPpPhase1Init(...).
     */
    CPSS_DXCH_IMPLEMENT_WA_SDMA_PKTS_FROM_CPU_STACK_PADDING_E,

    /** @brief WA for XCAT/XCAT2/LION.
     *  There are some memory spaces that cause the CPU or the
     *  management interface of device to freeze. To avoid this, every
     *  time the memory is accessed, a test will be performed.
     *  If the memory address falls within the range of problematic
     *  addresses, then an error will be returned.
     *  The memory space will not be accessed.
     */
    CPSS_DXCH_IMPLEMENT_WA_RESTRICTED_ADDRESS_FILTERING_E,

    /** @brief WA for xCat
     *  When working through RGMII interface with external HW (such as
     *  external PHY/CPU), the switch RGMII over RGMII pins must be set
     *  to edge align mode.
     */
    CPSS_DXCH_IMPLEMENT_WA_RGMII_EDGE_ALIGN_MODE_E,

    /** @brief WA for dxCh3, revision B2.
     *  IP Multicast traffic under stress conditions may lead to device hang.
     *  (FEr#2263)
     */
    CPSS_DXCH_IMPLEMENT_WA_IP_MC_UNDER_STRESS_E,

    /** @brief WA for XCAT/XCAT2/LION.
     *  in GE port interface mode transmission does not function when
     *  unidirect mode is set.
     *  (FEr#2795)
     */
    CPSS_DXCH_IMPLEMENT_WA_GE_PORT_UNIDIRECT_E,

    /** @brief WA for XCAT/XCAT2.
     *  Port where enabled dp_clk hung upon link establish, on XCAT
     *  relevant for both network ports and stack ports, on XCAT2
     *  relevant only for stack ports.
     *  The name of enumerator is historical and misleading, pay
     *  attention this problem can happen on 1G speed too, the issue is
     *  dp_clk!
     *  For XCAT3, BOBCAT2, CAELUM and ALDRIN see PRV_CPSS_DXCH_IMPLEMENT_WA_2_5G_SGMII_LINK_UP_E
     *  (HWEr#3065)
     */
    CPSS_DXCH_IMPLEMENT_WA_2_5G_SGMII_LINK_UP_E,

    /** @brief WA for XCAT.
     *  enable two WA's below:
     */
    CPSS_DXCH_IMPLEMENT_WA_SGMII_2_5G_UNDER_TRAFFIC_CFG_E,

    /** WA for XCAT. */
    CPSS_DXCH_IMPLEMENT_WA_SGMII_2_5G_UNDER_TRAFFIC_NETWORKP_CFG_E,

    /** @brief WA for XCAT.
     *  While configuring dp_clk and sync_fifo for SGMII 2.5G traffic
     *  damaged on devices where dp_clk or sync_fifo are global bits
     *  per network ports and stacking ports. 1G modes can work with
     *  dp_clk, except for half duplex 10/100 configuration, so customers
     *  that may need this mode should not use this WA.
     *  QSGMII is not functional with these configurations, so we forced to
     *  split them to Network Ports configuration and Stack Ports config.
     */
    CPSS_DXCH_IMPLEMENT_WA_SGMII_2_5G_UNDER_TRAFFIC_STACKP_CFG_E,

    /** @brief WA for LION.
     *  If we manage to push burst of transactions from pex, it may cause
     *  a stuck on the Seredes registers SM. Each transaction to the serdes
     *  registers goes through this SM. If 2 consecutive (WR) transactions
     *  are issued, the SM “get lost”, and the next RD access will not get
     *  acknowledged and MG will hang. The minimal delay needed is
     *  ~170-200ns. The required delay is between access to same group
     *  of 4 SD (0-3, 4-7, etc’). Shortly speaking 2 dummy reads to MG (0x4c)
     *  register is needed to avoid the problem.
     */
    CPSS_DXCH_IMPLEMENT_WA_SERDES_INTERNAL_REG_ACCESS_E,

    /** @brief WA for LION2 B0
     *  IPM bridge copy get dropped when there is no member in the local
     *  hemisphere vlan
     *  WA solution:
     *  The WA use not connected to MAC ports 14 and 64+14 in
     *  the TxQ.
     *  Ports 14 of each hemisphere have Link UP in the TxQ unit
     *  The WA sets in every VLAN enabled for IP MC Routing Port
     *  14 and port 64+14 as a member.
     *  Packets in such VLANs go to ports 14 ands 64+14 TxQ
     *  queues as well as to other ports in VLANs.
     *  The WA uses the Tail drop to avoid packets stuck in the
     *  ports 14 ands 64+14 TxQ queues.
     *  The tails drop configurations is:
     *  - Tail Drop must be enabled
     *  - Configure Port 14 and port 64+14 to TailDropProfile 8
     *  NOTE: User can use only 7 out of 8 available profiles
     *  - Set Queue limits of profile 8 to all zeros
     *  (FE-7263524)
     */
    CPSS_DXCH_IMPLEMENT_WA_IPM_BRIDGE_COPY_GET_DROPPED_E,

    /** @brief WA for Lion2 A0(487 DIP) and
     *  B0(different implementation)
     *  after several connect/disconnect get GearBox sync + no alignment
     *  lock.
     */
    CPSS_DXCH_IMPLEMENT_WA_NO_ALLIGNMENT_LOCK_E,

    /** @brief The device supports duplication of traffic to ports within the assigned
     *  VLAN. The device incorporates two internal
     *  engines that are responsible for the replication to all ports.
     *  However, due to this erratum, In some scenarios, although the traffic bandwidth
     *  in a specific PortGroup does not exceed the allowed bandwidth,
     *  not all VLAN members will transmit the replication.
     *  This behavior is due to extra replication (relay port)
     *  that is performed between the two internal replication engines.
     *  The restriction applies to a packet size under 128B only.
     *  The problem relevant both to Assigned Multicast (VIDX) and VLAN flooded traffic.
     *  Workaround:
     *  Add the relay port to the VLAN only if
     *  ports from both hemisphere are members in the VLAN.
     *  (FE-6041254)
     */
    CPSS_DXCH_IMPLEMENT_WA_MC_BANDWIDTH_RESTRICTION_E,

    /** @brief WA for Lion2 B0(AN333)
     *  10G/20GR interfaces show false link while connected to interface
     *  with higher speed
     */
    CPSS_DXCH_IMPLEMENT_WA_DISMATCH_IF_LINK_E,

    /** @brief WA for LION2 B1.
     *  Lion2 device does not allow cut through traffic from slow port to faster one.
     *  The WA allows traffic from slow port with cut through enabled
     *  to be transmitted from faster port.
     */
    CPSS_DXCH_IMPLEMENT_WA_CUT_THROUGH_SLOW_TO_FAST_E,

    /** @brief - WA for Lion2 B0
     *  RXAUI sometimes fails to establish link.
     */
    CPSS_DXCH_IMPLEMENT_WA_RXAUI_LINK_E,

    /** @brief WA for Bobcat2 A0
     *  Flow Control AutoNegotiation for trispeed ports requires software intervention.
     *  (FE-1789640)
     */
    CPSS_DXCH_IMPLEMENT_WA_TRI_SPEED_PORT_AN_FC_E,

    /** @brief bobcat2 rev A0 : when 40G port don't pass through TM it shall get
     *  30G at TxQ (FE-8708889)
     */
    CPSS_DXCH_IMPLEMENT_WA_BOBCAT2_REV_A0_40G_NOT_THROUGH_TM_IS_PA_30G_E,

    /** removing wrong Mib counters after link down */
    CPSS_DXCH_IMPLEMENT_WA_WRONG_MIB_COUNTERS_LINK_DOWN_E,

    /** @brief WA for xCat (revision A2 and above) and xCat3
     *  Incorrect default value of bit 31 in the Routing TCAM Control register
     *  (RM#30230)
     */
    CPSS_DXCH_IMPLEMENT_WA_ROUTER_TCAM_RM_E,

    /** Bobcat2 device only: Adjust FIFO thresholds for 1G and 10G ports in order to get precise PTP timestamp value under traffic. */
    CPSS_DXCH_IMPLEMENT_WA_BOBCAT2_PTP_TIMESTAMP_E,

    CPSS_DXCH_IMPLEMENT_WA_100BASEX_AN_DISABLE_E,

    /** @brief WA for xCat3 XLG ports
     *   XLG MACs may inject packets with wrong byte count information to
     *   processing pipe when link changed from UP to Down. These packets
     *   may result in buffers stuck in devices.
     *   WA is based on enable of unidirectional feature in XLG MACs.
     */
    CPSS_DXCH_IMPLEMENT_WA_XCAT3_XLG_PORT_BUFFER_STUCK_UNIDIRECTIONAL_E,

    /** @brief last value indicator --> no to be used
     *  as WA for something.
     */
    CPSS_DXCH_IMPLEMENT_WA_LAST_E

} CPSS_DXCH_IMPLEMENT_WA_ENT;

/**
* @internal cpssDxChHwPpPhase1Init function
* @endinternal
*
* @brief   This function performs basic hardware configurations on the given PP, in
*         Hw registration phase.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] ppPhase1ParamsPtr        - Packet processor hardware specific parameters.
*
* @param[out] deviceTypePtr            - The Pp's device Id.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_ALREADY_EXIST         - the devNum already in use
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_VALUE             - the driver found unknown device type
* @retval GT_NOT_IMPLEMENTED       - the CPSS was not compiled properly
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwPpPhase1Init
(
    IN      CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   *ppPhase1ParamsPtr,
    OUT     CPSS_PP_DEVICE_TYPE                 *deviceTypePtr
);
/* temporary, until next release */
#define cpssDxChHwPpPhase1Init_new cpssDxChHwPpPhase1Init

/**
* @internal cpssDxChHwPpPhase2Init function
* @endinternal
*
* @brief   This function performs basic hardware configurations on the given PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] ppPhase2ParamsPtr        - Phase2 initialization parameters..
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
*/
GT_STATUS cpssDxChHwPpPhase2Init
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PP_PHASE2_INIT_INFO_STC       *ppPhase2ParamsPtr
);

/**
* @internal cpssDxChHwPpStartInit function
* @endinternal
*
* @brief   This phase performs the start-init operation on a given device, and sets
*         it's registers according to the registers value list passed by user.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to which the start-init operation
*                                      should be done.
* @param[in] loadFromEeprom           - GT_TRUE, trigger the device to re-load the eeprom.
* @param[in] initDataListPtr          - List of registers values to be set to the device.
* @param[in] initDataListLen          - Number of elements in ppRegConfigList
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - on timed out retries
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function should be invoked after cpssDxChHwPpPhase1Init() and
*       before cpssDxChHwPpPhase2Init().
*
*/
GT_STATUS cpssDxChHwPpStartInit
(
    IN  GT_U8               devNum,
    IN  GT_BOOL             loadFromEeprom,
    IN  CPSS_REG_VALUE_INFO_STC   *initDataListPtr,
    IN  GT_U32              initDataListLen
);

/**
* @internal cpssDxChHwPpImplementWaInit function
* @endinternal
*
* @brief   This function allow application to state which WA (workarounds) the CPSS
*         should implement.
*         NOTEs:
*         1. The function may be called several times with different needed WA.
*         2. The CPSS will implement the WA for the requested WA , even when the HW
*         not require the 'WA' anymore.
*         for example:
*         assume the xcat A1 has erratum regarding "wrong trunk id
*         source port information of packet to CPU" , and in the WA the CPSS
*         will limit application to specific trunk-IDs.
*         but if no longer have this problem , but the application
*         still request for the WA , the CPSS will continue to handle the
*         WA.
*         further more - application may ask the WA for ch3...
*         3. The function can be invoked only after cpssDxChHwPpPhase1Init().
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] numOfWa                  - number of WA needed (number of elements in
* @param[in] waArr[]                  and additionalInfoBmpArr[] arrays).
* @param[in] waArr[]                  - (array of) WA needed to implement
* @param[in] additionalInfoBmpArr[]   - (array of) bitmap for additional WA info.
*                                      The meaning of the additional info is located
*                                      in the description of the specific WA. May be NULL.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or numOfWa = 0 or bad WA value
* @retval GT_BAD_STATE             - the WA can't be called at this stage ,
*                                       because it's relate library already initialized.
*                                       meaning it is 'too late' to request for the WA.
* @retval GT_NOT_IMPLEMENTED       - the WA can't be implemented for the current device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - on timed out retries
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwPpImplementWaInit
(
    IN GT_U8                        devNum,
    IN GT_U32                       numOfWa,
    IN CPSS_DXCH_IMPLEMENT_WA_ENT   waArr[],
    IN GT_U32                       additionalInfoBmpArr[]
);

/**
* @internal cpssDxChPpHwImplementWaGet function
* @endinternal
*
* @brief   Gets information on whether a specific workaround was implemented
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] wa                       - the workaround
*
* @param[out] isImplemented            - GT_TRUE: the workaround is implemented on the device
*                                      GT_FALSE: the workaround is not implemented on the device
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or bad WA value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_IMPLEMENTED       - no information on whether the erratum was
*                                       implemented or not
*/
GT_STATUS cpssDxChPpHwImplementWaGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_IMPLEMENT_WA_ENT  wa,
    OUT GT_BOOL                     *isImplemented
);


/**
* @internal cpssDxChHwAuDescSizeGet function
* @endinternal
*
* @brief   This function returns the size in bytes of a single Address Update
*         descriptor, for a given device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devType                  - The PP's device type to return the descriptor's size for.
*
* @param[out] descSizePtr              The descrptor's size (in bytes).
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwAuDescSizeGet
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSizePtr
);


/**
* @internal cpssDxChHwRxDescSizeGet function
* @endinternal
*
* @brief   This function returns the size in bytes of a single Rx descriptor,
*         for a given device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devType                  - The PP's device type to return the descriptor's size for.
*
* @param[out] descSizePtr              The descrptor's size (in bytes).
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwRxDescSizeGet
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSizePtr
);


/**
* @internal cpssDxChHwTxDescSizeGet function
* @endinternal
*
* @brief   This function returns the size in bytes of a single Tx descriptor,
*         for a given device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devType                  - The PP's device type to return the descriptor's size for.
*
* @param[out] descSizePtr              The descrptor's size (in bytes).
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwTxDescSizeGet
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSizePtr
);


/**
* @internal cpssDxChHwRxBufAlignmentGet function
* @endinternal
*
* @brief   This function returns the required alignment in bytes of a RX buffer,
*         for a given device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devType                  - The PP's device type to return the descriptor's size for.
*
* @param[out] byteAlignmentPtr         - The buffer alignment (in bytes).
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwRxBufAlignmentGet
(
    IN  CPSS_PP_DEVICE_TYPE devType,
    OUT GT_U32    *byteAlignmentPtr
);


/**
* @internal cpssDxChHwPpSoftResetTrigger function
* @endinternal
*
* @brief   This routine issue soft reset for a specific pp.
* @brief   For Falcon resets Main Dies and Chiplets.
* @brief   Chiplets reset can be skipped enabling CPSS_HW_PP_RESET_SKIP_TYPE_CHIPLETS_E.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to reset.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Soft reset is the process in which selected entities (as configured by
*       using the cpssDxChHwPpSoftResetSkipParamSet API) are reset to their
*       default values.
*
*/
GT_STATUS cpssDxChHwPpSoftResetTrigger
(
    IN  GT_U8 devNum
);


/**
* @internal cpssDxChHwPpSoftResetSkipParamSet function
* @endinternal
*
* @brief   This routine configure skip parameters related to soft reset.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the skip parameter to set
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[in] skipEnable               - GT_FALSE: Do Not Skip
*                                      GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwPpSoftResetSkipParamSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    IN  GT_BOOL                         skipEnable

);

/**
* @internal cpssDxChHwPpSoftResetSkipParamGet function
* @endinternal
*
* @brief   This routine return configuration of skip parameters related to soft reset.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the skip parameter to set
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
*
* @param[out] skipEnablePtr            - GT_FALSE: Do Not Skip
*                                      GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChHwPpSoftResetSkipParamGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    OUT GT_BOOL                         *skipEnablePtr

);

/**
* @internal cpssDxChHwPpSoftResetChipletsTrigger function
* @endinternal
*
* @brief   This routine issue soft reset for a set of chiplets.
* @brief   Does not reset Main Dies.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] chipletsBmp              - bitmap of chiplet indexes to reset.
*                                       CPSS_CHIPLETS_UNAWARE_MODE_CNS for all chiplets of the device
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Soft reset is the process in which selected entities (as configured by
*       using the cpssDxChHwPpSoftResetChipletsSkipParamSet API) are reset to their
*       default values.
*
*/
GT_STATUS cpssDxChHwPpSoftResetChipletsTrigger
(
    IN  GT_U8                   devNum,
    IN  GT_CHIPLETS_BMP         chipletsBmp
);

/**
* @internal cpssDxChHwPpSoftResetChipletsSkipParamSet function
* @endinternal
*
* @brief   This routine configure skip parameters related to soft reset of set of chiplets.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] chipletsBmp              - bitmap of chiplet indexes to reset.
*                                       CPSS_CHIPLETS_UNAWARE_MODE_CNS for all chiplets of the device
* @param[in] skipType                 - the skip parameter to set
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[in] skipEnable               - GT_FALSE: Do Not Skip
*                                       GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwPpSoftResetChipletsSkipParamSet
(
    IN  GT_U8                           devNum,
    IN  GT_CHIPLETS_BMP                 chipletsBmp,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    IN  GT_BOOL                         skipEnable
);

/**
* @internal cpssDxChHwPpSoftResetChipletsSkipParamGet function
* @endinternal
*
* @brief   This routine get skip parameters related to soft reset of chiplet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] chipletsBmp              - bitmap of chiplet indexes to set skip reset state,
*                                       CPSS_CHIPLETS_UNAWARE_MODE_CNS for all chiplets of the device.
* @param[in] skipType                 - the skip parameter to get.
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[out] skipEnablePtr           - pointer to GT_FALSE: Do Not Skip
*                                                  GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHwPpSoftResetChipletsSkipParamGet
(
    IN  GT_U8                           devNum,
    IN  GT_CHIPLETS_BMP                 chipletsBmp,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    OUT GT_BOOL                         *skipEnablePtr
);

/**
* @internal cpssDxChHwPpInitStageGet function
* @endinternal
*
* @brief   Indicates the initialization stage of the device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to reset.
*
* @param[out] initStagePtr             - pointer to the stage of the device
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChHwPpInitStageGet
(
    IN  GT_U8                      devNum,
    OUT CPSS_HW_PP_INIT_STAGE_ENT  *initStagePtr
);

/**
* @internal cpssDxChCfgHwDevNumSet function
* @endinternal
*
* @brief   write HW device number
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] hwDevNum                 - HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgHwDevNumSet
(
    IN GT_U8            devNum,
    IN GT_HW_DEV_NUM    hwDevNum
);

/**
* @internal cpssDxChCfgHwDevNumGet function
* @endinternal
*
* @brief   Read HW device number
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] hwDevNumPtr              - Pointer to HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgHwDevNumGet
(
    IN GT_U8            devNum,
    OUT GT_HW_DEV_NUM   *hwDevNumPtr
);

/**
* @internal cpssDxChHwInterruptCoalescingSet function
* @endinternal
*
* @brief   Configures the interrupt coalescing parameters and enable\disable the
*         functionality.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: interrupt coalescing is enabled
*                                      GT_FALSE: interrupt coalescing is disabled
* @param[in] period                   - Minimal IDLE  between two consecutive interrupts.
*                                      The units of this input parameter is in nSec, the
*                                      resolution is in 320 nSec (the HW resolution for the
*                                      interrupt coalescing period is 0.32uSec). In case interrupt
*                                      coalescing is enabled (enable == GT_TRUE)
*                                      (APPLICABLE RANGES: 320..5242560).
*                                      Otherwise (enable == GT_FALSE), ignored.
* @param[in] linkChangeOverride       - GT_TRUE: A link change in one of the ports
*                                      results interrupt regardless of the (coalescing)
* @param[in] period
*                                      GT_FALSE: A link change in one of the ports
*                                      does not results interrupt immediately but
*                                      according to the (coalescing) period.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwInterruptCoalescingSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable,
    IN GT_U32   period,
    IN GT_BOOL  linkChangeOverride
);

/**
* @internal cpssDxChHwInterruptCoalescingGet function
* @endinternal
*
* @brief   Gets the interrupt coalescing configuration parameters.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE: interrupt coalescing is enabled
*                                      GT_FALSE: interrupt coalescing is disabled
* @param[out] periodPtr                - (pointer to) Minimal IDLE period between two consecutive
*                                      interrupts. The units are in nSec with resolution of
*                                      320nSec (due to HW resolution) and is relevant only in
*                                      case interrupt coalescing is enabled (enablePtr == GT_TRUE).
*                                      (APPLICABLE RANGES: 320..5242560).
* @param[out] linkChangeOverridePtr    - (pointer to)
*                                      GT_TRUE: A link change in one of the ports
*                                      results interrupt regardless of the (coalescing)
*                                      period.
*                                      GT_FALSE: A link change in one of the ports
*                                      does not results interrupt immediately but
*                                      according to the (coalescing) period.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The HW resolution for the interrupt coalescing period is 0.32uSec.
*
*/
GT_STATUS cpssDxChHwInterruptCoalescingGet
(
    IN GT_U8                devNum,
    OUT GT_BOOL         *enablePtr,
    OUT GT_U32            *periodPtr,
    OUT GT_BOOL     *linkChangeOverridePtr
);

/**
* @internal cpssDxChHwCoreClockGet function
* @endinternal
*
* @brief   This function returns the core clock value from cpss DB and from HW.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp's device number.
*
* @param[out] coreClkDbPtr             - Pp's core clock from cpss DB (MHz)
* @param[out] coreClkHwPtr             - Pp's core clock read from HW (MHz) or zero
*                                      if PLL configuration as sampled at reset could
*                                      not be mapped to core clock value.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwCoreClockGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *coreClkDbPtr,
    OUT GT_U32  *coreClkHwPtr
);

/**
* @internal cpssDxChHwInitLion2GeBackwardCompatibility function
* @endinternal
*
* @brief   Init parameters of serdes initialization for 1.25G frequncy accordingly to
*         HWS version 43. Proposed for Lion2 B0 only.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Must be call before cpssDxChHwPpPhase1Init.
*
*/
GT_VOID cpssDxChHwInitLion2GeBackwardCompatibility
(
    GT_VOID
);

/**
* @internal cpssDxChHwPpAddressDecodeWindowConfigSet function
* @endinternal
*
* @brief   This function perfroms the address decoding window configuration.
*         If a window is enabled and an address driven by one the PP's units hit the window,
*         the window's configuration applied on this address access.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] windowNumber             - window number (APPLICABLE RANGES: 0..5).
* @param[in] windowConfigPtr          - window's configuration.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, window size or window base address
* @retval GT_NOT_SUPPORTED         - if this feature not supported by the device.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwPpAddressDecodeWindowConfigSet
(
    IN  GT_U8                                     devNum,
    IN  GT_U32                                    windowNumber,
    IN  CPSS_DXCH_ADDR_DECODE_WINDOW_CONFIG_STC  *windowConfigPtr
);

/**
* @internal cpssDxChHwPpAddressDecodeWindowConfigGet function
* @endinternal
*
* @brief   This function returns the address decoding window configuration.
*         If a window is enabled and an address driven by one the PP's units hit the window,
*         the window's configuration applied on this address access.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] windowNumber             - window number (APPLICABLE RANGES: 0..5).
*
* @param[out] windowConfigPtr          - window's configuration.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, window size or window base address
* @retval GT_BAD_STATE             - get illegal value from HW.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChHwPpAddressDecodeWindowConfigGet
(
    IN  GT_U8                                     devNum,
    IN  GT_U32                                    windowNumber,
    OUT CPSS_DXCH_ADDR_DECODE_WINDOW_CONFIG_STC  *windowConfigPtr
);

/**
* @internal cpssDxChMicroInitBasicCodeGenerate function
* @endinternal
*
* @brief   This function performs basic and per feature code generation.
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] ppPhase1ParamsPtr        - Packet processor hardware specific  parameters.
* @param[in] featuresBitmap           - bitmap of cpss features participated in code generation.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_ALREADY_EXIST         - the devNum already in use
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_VALUE             - the driver found unknown device type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMicroInitBasicCodeGenerate
(
    IN CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *ppPhase1ParamsPtr,
    IN GT_U32 featuresBitmap
);

/**
* @internal cpssDxChGpioPhyConfigSet function
* @endinternal
*
* @brief   Set per-PHY GPIO configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports.
* @param[in] directionBitmap          - For bits 0..15:
*                                      bit#i set means GPIO#i is input,
*                                      bit#i not set means GPIO#i is output
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChGpioPhyConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          directionBitmap
);

/**
* @internal cpssDxChGpioPhyConfigGet function
* @endinternal
*
* @brief   Get per-PHY GPIO configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports.
*
* @param[out] modeBitmapPtr            - (Pointer to) For bits 0..15:
*                                      bit#i set means GPIO#i pin can be used as GPIO,
*                                      bit#i not set means GPIO#i pin cannot be used as GPIO
* @param[out] directionBitmapPtr       - (Pointer to) For bits 0..15:
*                                      bit#i set means GPIO#i is input,
*                                      bit#i not set means GPIO#i is output
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChGpioPhyConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                         *modeBitmapPtr,
    OUT GT_U32                         *directionBitmapPtr
);

/**
* @internal cpssDxChGpioPhyDataRead function
* @endinternal
*
* @brief   Read per-PHY input GPIO data.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports.
*
* @param[out] dataBitmapPtr            - (Pointer to) Bits 0..15 reflect the input value on
*                                      GPIO pins enabled and configured for input.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChGpioPhyDataRead
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                         *dataBitmapPtr
);

/**
* @internal cpssDxChGpioPhyDataWrite function
* @endinternal
*
* @brief   Write per-PHY output GPIO data.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports.
* @param[in] dataBitmap               - Bits 0..15 (when not masked) will be the output value on
*                                      GPIO pins enabled and configured for output.
* @param[in] dataBitmapMask           - For bits 0..15:
*                                      bit#i set means corresponding bit at dataBitmap will be used,
*                                      bit#i not set means corresponding bit at dataBitmap will be discarded.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChGpioPhyDataWrite
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          dataBitmap,
    IN  GT_U32                          dataBitmapMask
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChHwInith */

