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
* @file prvTgfIpv4UcRoutingAddRealPrefixes.c
*
* @brief IPV4 UC Routing when filling the Lpm using real costumer prefixes defined in a file.
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfIpv4UcRoutingAddMany.h>
#include <trunk/prvTgfTrunk.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>
#include <ip/prvTgfIpv4UcRoutingAddManyIpLpmMng.h>

extern GT_U8 prefixesOctetsAndLenRepresentation[];
extern GT_U32 prvTgfPrefixesOctetsAndLenRepresentationArraySizeGet(GT_VOID);

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* number of prefixed actually added and need to be deleted */
static GT_U32       numOfExistingPrefixesToDel=0;
/* the last valid prefixed read from the file and successfully added to the system */
static GT_IPADDR    lastIpAddrAdded;
static GT_U32       lastPrefixLen;
/* number of prefixes we should be expecting to be added to the LPM before we get the first fail */
static GT_U32       numOfPrefixesBeforeFirstFail=0;
/* the last valid prefix read from the file and successfully added to the system before the first fail */
static GT_IPADDR    lastIpAddrAddedBeforeFirstFail;
/* indicate if this is the first time we have a prefix add fail*/
static GT_BOOL      firstFailFlag = GT_TRUE;

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = do_not_share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 174353
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 173454 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS  174353
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 172277
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 171358 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS  172277

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 201262
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 201108 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS   201262
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 201262
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 201107 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS   201262

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = do_not_share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 169980
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 169113 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS   169980
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 167852
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 166935 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS   167852


/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = do_not_share and PBR uses 8K lines
   lpmMemMode  = PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 86266
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 85820 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS   86266
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 85479
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 83948 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS   85479

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = share and PBR uses 8K lines
   lpmMemMode  = PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 95864
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 95793 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS   95864
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 95805
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 95793 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS   95805


/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = share and PBR uses 8K lines
   lpmMemMode  = PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_HALF_MEMORY_CNS   42691
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_HALF_MEMORY_CNS   42625


/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = share and no PBR
   lpmMemMode  = PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 101537
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 101565 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS   101537
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 101508
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 101565 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS   101508


/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = share and no PBR
   lpmMemMode  = PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_HALF_MEMORY_CNS   48175
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_HALF_MEMORY_CNS   48148


/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = do_not_share and PBR uses 8K lines
   lpmMemMode  = PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 91949
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 91325 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS   91949
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 91765
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 85248 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS   91765


/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = do_not_share and PBR uses 8K lines
   lpmMemMode  = PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_HALF_MEMORY_CNS   43409
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_HALF_MEMORY_CNS   42966





/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 195897
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 196182 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS   195897
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 195897
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 195916 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS   195897


/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = do not share and PBR uses 8K lines
   lpmMemMode  = PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_HALF_MEMORY_CNS   38305
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_HALF_MEMORY_CNS   37625



/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = do_not_share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 67777
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 67692 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS   67777
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 67149
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 62629 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS   67149


/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when addition of SWAP memory in bank0 reduce number of entries --> 74519
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE      --> 74617 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS   74519
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 74518
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 74513 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS   74518


/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = do_not_share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 62497
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 62462 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS   62497
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 61601
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 58499 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS   61601

/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 69461
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 69616 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS   69461
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 69461
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 69462 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS   69461

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 21165
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 21014 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS   21165
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 20036
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 19905 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS   20036

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 22619
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 22702 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS   22619
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 22521
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 22578 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS 22521

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when SWAP is used before allocating a new memory  block     --> 16749
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 16880
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 16885
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 16702 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS   16886
/* expected value when SWAP is used at the beginning of memory allocation     --> 15490
   expected value when SWAP is used before allocating a new memory  block     --> 15349
   expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 15415
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE  --> 15456
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 15277 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS  15456

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when SWAP is used before allocating a new memory  block     --> 18782
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 18645
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 18569 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS   18645
/* expected value when SWAP is used before allocating a new memory  block     --> 18694
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 18625
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 18354 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS   18625

/* number of prefixes expected to be added in a device with 320K/120K LPM lines
   blocksAllocationMethod = do_not_share/share and PBR = 0/not 0
   and we add 1/10 from the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_NON_BASELINE_TEST_EXECUTION_CNS   26615
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_FOR_NON_BASELINE_TEST_EXECUTION_CNS   0

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share/share and PBR = 0/not 0
   and we add 1/10 from the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when SWAP is used at the beginning of memory allocation     --> 15780
   expected value when SWAP is used before allocating a new memory  block     --> 15674
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 15776
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 15780
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 15640 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_NON_BASELINE_TEST_EXECUTION_40K_LINES_CNS   15780
/* expected value when SWAP is used at the beginning of memory allocation     --> 15490
   expected value when SWAP is used before allocating a new memory  block     --> 15349
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 15415
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 15456
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 15277 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_FOR_NON_BASELINE_TEST_EXECUTION_40K_LINES_CNS   15456


static GT_STATUS prvTgfIpLpmCalculateExpectedNumberOfPrefixes
(
    IN  GT_BOOL                         baselineTestsExecutionFlag,
    IN  PRV_TGF_LPM_RAM_CONFIG_STC      * ramDbCfgPtr,
    IN  GT_U32                          maxNumOfPbrEntries,
    OUT GT_U32                          * numOfPrefixesExpectedPtr,
    OUT GT_U32                          * numOfPrefixesExpectedBeforeFirstFailPtr
);


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpLpmMngIpv4UcRoutingAddRealPrefixesConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpLpmMngIpv4UcRoutingAddRealPrefixesConfigurationAndTrafficGenerate(GT_VOID)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_LPM_RAM_CONFIG_STC              ramDbCfg;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_U32                                  i=0;
    GT_IPADDR                               ipPrefixAddr;
    GT_U32                                  prefixLen=0;
    GT_U32                                  numOfPrefixesAdded=0;
    GT_U32                                  numOfPrefixesExpected=0;
    GT_U32                                  numOfPrefixesExpectedBeforeFirstFail=0;
    GT_U32                                  numOfPacketsSent=0;
    GT_U32                                  numOfElementsInArray;
    GT_U32                                  start_sec  = 0;
    GT_U32                                  start_nsec = 0;
    GT_U32                                  end_sec  = 0;
    GT_U32                                  end_nsec = 0;
    GT_U32                                  temp_start_sec  = 0;
    GT_U32                                  temp_start_nsec = 0;
    GT_U32                                  temp_end_sec  = 0;
    GT_U32                                  temp_end_nsec = 0;
    GT_U32                                  diff_sec;
    GT_U32                                  temp_diff_sec=0,temp_diff_nsec=0;
    GT_U32                                  totalTimeOfAllValiditiChecks_nsec=0;
    GT_U32                                  totalTimeOfAllValiditiChecks_sec=0;
    GT_U32                                  value;
    GT_U32                                  maxNumOfPbrEntries=0;
    GT_U32                                  divider=0;

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigGet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Create Ipv4 prefixes in a Virtual Router for the specified LPM DB
     */

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before adding new prefixes");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Adding Prefixes using  prvTgfIpLpmIpv4UcPrefixAdd\n "
                     "This function tries to add many IPv4 Unicast prefixes according to file ====\n");

   /* fill a nexthope info for the prefix routeEntryBaseIndex=port_2 */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    if(appDemoDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        maxNumOfPbrEntries = value;
    else
        maxNumOfPbrEntries = _8K;
    if(maxNumOfPbrEntries > PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.routerNextHop)
    {
        maxNumOfPbrEntries = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.routerNextHop;
    }

    rc = prvTgfIpLpmCalculateExpectedNumberOfPrefixes(prvUtfBaselineTestsExecutionFlagGet(),
                                                 &ramDbCfg,maxNumOfPbrEntries,&numOfPrefixesExpected,&numOfPrefixesExpectedBeforeFirstFail);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test do not supported at the moment any other configuration: %d", prvTgfDevNum);


    numOfElementsInArray=prvTgfPrefixesOctetsAndLenRepresentationArraySizeGet();

    if (prvUtfBaselineTestsExecutionFlagGet()==GT_FALSE)
    {
        numOfElementsInArray=(numOfElementsInArray/10);
    }

    /* disable LPM validity check */
    prvTgfIpValidityCheckEnable(GT_FALSE);
    cpssOsTimeRT(&start_sec, &start_nsec);
    for(i=0;i<numOfElementsInArray; i=i+5)
    {
        ipPrefixAddr.arIP[0]= prefixesOctetsAndLenRepresentation[i];
        ipPrefixAddr.arIP[1]= prefixesOctetsAndLenRepresentation[i+1];
        ipPrefixAddr.arIP[2]= prefixesOctetsAndLenRepresentation[i+2];
        ipPrefixAddr.arIP[3]= prefixesOctetsAndLenRepresentation[i+3];
        prefixLen = prefixesOctetsAndLenRepresentation[i+4];

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0,
                                        ipPrefixAddr, prefixLen, &nextHopInfo, GT_FALSE, GT_TRUE);

        if((rc==GT_NO_RESOURCE)||(rc==GT_OUT_OF_PP_MEM))
        {
            if (firstFailFlag==GT_TRUE)
            {
                /* check if we get the correct number of prefixed added */
                UTF_VERIFY_EQUAL2_STRING_MAC(numOfPrefixesExpectedBeforeFirstFail ,numOfPrefixesAdded  ,
                                  "======= numOfPrefixesAdded[%d] is different then numOfPrefixesExpectedBeforeFirstFail value [%d] =======\n",
                                 numOfPrefixesAdded,numOfPrefixesExpectedBeforeFirstFail);

                lastIpAddrAddedBeforeFirstFail = lastIpAddrAdded;
                numOfPrefixesBeforeFirstFail = numOfPrefixesAdded;

                cpssOsPrintf("\n lastIpAddrAddedBeforeFirstFail = %d.%d.%d.%d/%d , numOfPrefixesBeforeFirstFail=%d \n",lastIpAddrAddedBeforeFirstFail.arIP[0],
                         lastIpAddrAddedBeforeFirstFail.arIP[1],lastIpAddrAddedBeforeFirstFail.arIP[2],lastIpAddrAddedBeforeFirstFail.arIP[3],prefixLen,numOfPrefixesBeforeFirstFail);
                firstFailFlag=GT_FALSE;
            }
            else
            {
                /*cpssOsPrintf("\n could not add ipPrefixAddr = %d.%d.%d.%d/%d \n",ipPrefixAddr.arIP[0],
                             ipPrefixAddr.arIP[1],ipPrefixAddr.arIP[2],ipPrefixAddr.arIP[3],prefixLen);                */
            }
            continue;
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
        }

        lastIpAddrAdded = ipPrefixAddr;
        lastPrefixLen = prefixLen;
        numOfPrefixesAdded++;

        if (prvUtfBaselineTestsExecutionFlagGet()==GT_TRUE)
        {
            if (numOfPrefixesAdded%5000==0)
            {
                cpssOsTimeRT(&temp_start_sec, &temp_start_nsec);
                cpssOsPrintf(".");

                /* enable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_TRUE);

                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

                /* disable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_FALSE);

                cpssOsTimeRT(&temp_end_sec, &temp_end_nsec);
                if(temp_end_nsec < temp_start_nsec)
                {
                    temp_end_nsec += 1000000000;
                    temp_end_sec  -= 1;
                }
                temp_diff_nsec  = temp_end_nsec  - temp_start_nsec;
                temp_diff_sec  = temp_end_sec  - temp_start_sec;
                totalTimeOfAllValiditiChecks_nsec +=  temp_diff_nsec;
                totalTimeOfAllValiditiChecks_sec +=  temp_diff_sec;
            }
        }
    }
    /* check if we get the correct number of prefixed added */
    UTF_VERIFY_EQUAL2_STRING_MAC(numOfPrefixesExpected,numOfPrefixesAdded ,
                                  "======= numOfPrefixesAdded[%d] is different then numOfPrefixesExpected value [%d] =======\n",
                                 numOfPrefixesAdded,numOfPrefixesExpected);

    numOfExistingPrefixesToDel = numOfPrefixesAdded;

    cpssOsPrintf("\n last ipPrefixAddr added = %d.%d.%d.%d/%d \n",lastIpAddrAdded.arIP[0],
                             lastIpAddrAdded.arIP[1],lastIpAddrAdded.arIP[2],lastIpAddrAdded.arIP[3],lastPrefixLen);

    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec - totalTimeOfAllValiditiChecks_sec - (totalTimeOfAllValiditiChecks_nsec/1000000000);

    cpssOsPrintf("\n total running time [%d] seconds, numOfPrefixesAdded=%d \n",
                 diff_sec,numOfPrefixesAdded);

    /* enable LPM validity check */
    prvTgfIpValidityCheckEnable(GT_TRUE);

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

    /* disable LPM validity check */
    prvTgfIpValidityCheckEnable(GT_FALSE);

    /* -------------------------------------------------------------------------
     * 2. send packet to all prefixes added
     */
    for(i=0;i<numOfElementsInArray;i=i+5)
    {
        ipPrefixAddr.arIP[0]= prefixesOctetsAndLenRepresentation[i];
        ipPrefixAddr.arIP[1]= prefixesOctetsAndLenRepresentation[i+1];
        ipPrefixAddr.arIP[2]= prefixesOctetsAndLenRepresentation[i+2];
        ipPrefixAddr.arIP[3]= prefixesOctetsAndLenRepresentation[i+3];

        if (prvUtfBaselineTestsExecutionFlagGet()==GT_TRUE)
        {
            /* in baseline we want to send a packet after every 2000 prefix
               when i=10000 --> prefix index=10000/5=2000 */
            divider=10000;
        }
        else
        {
            /* in non baseline we want to send a packet after every 20000 prefix
               when i=100000 --> prefix index=100000/5=20000 */
            divider=100000;
        }
        /* AUTODOC: send packet to the added prefix only if it a prefix before we got the first fail.
               only then we can be sure we will have traffic .
               in other cases we cannot be sure the prefix was successfully added */
        if((i%divider==0)&&((i/5)<(numOfPrefixesBeforeFirstFail==0 ? numOfPrefixesAdded : numOfPrefixesBeforeFirstFail)))
        {
           prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipPrefixAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);
           numOfPacketsSent++;
           cpssOsPrintf(".");
        }
    }

    /* AUTODOC: send packet to the last added prefix  */
    prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], lastIpAddrAdded, PRV_TGF_NEXTHOPE_VLANID_CNS);
    numOfPacketsSent++;
    cpssOsPrintf(".");
    cpssOsPrintf("\n numOfPacketsSent=%d \n", numOfPacketsSent);
}

/**
* @internal prvTgfIpLpmMngIpv4UcRoutingAddRealPrefixesConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpLpmMngIpv4UcRoutingAddRealPrefixesConfigurationRestore(GT_VOID)
{
    /* -------------------------------------------------------------------------
     * 1. Restore Route Configuration
     */

    GT_STATUS   rc = GT_OK;
    GT_U32      i=0;
    GT_IPADDR  ipPrefixAddr;
    GT_U32     prefixLen=0;
    GT_U32     numOfPrefixesDeleted=0;
    GT_U32     numOfPacketsSent=0;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32     numOfElementsInArray;
    GT_U32     start_sec  = 0;
    GT_U32     start_nsec = 0;
    GT_U32     end_sec  = 0;
    GT_U32     end_nsec = 0;
    GT_U32     temp_start_sec  = 0;
    GT_U32     temp_start_nsec = 0;
    GT_U32     temp_end_sec  = 0;
    GT_U32     temp_end_nsec = 0;
    GT_U32     diff_sec;
    GT_U32     temp_diff_sec=0,temp_diff_nsec=0;
    GT_U32     totalTimeOfAllPacketSendAndValidityCheck_nsec=0;
    GT_U32     totalTimeOfAllPacketSendAndValidityCheck_sec=0;
    GT_U32     divider=0;


    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigGet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before restore ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Deleting Prefixes using  prvTgfIpLpmIpv4UcPrefixDel\n "
                     "This function tries to delete IPv4 Unicast prefixes added according to a file ====\n");

    numOfElementsInArray=prvTgfPrefixesOctetsAndLenRepresentationArraySizeGet();
    if (prvUtfBaselineTestsExecutionFlagGet()==GT_FALSE)
    {
        numOfElementsInArray=numOfElementsInArray/10;
    }

    /* disable LPM validity check */
    prvTgfIpValidityCheckEnable(GT_FALSE);
    cpssOsTimeRT(&start_sec, &start_nsec);
    for(i=0;i<numOfElementsInArray; i=i+5)
    {
        ipPrefixAddr.arIP[0]= prefixesOctetsAndLenRepresentation[i];
        ipPrefixAddr.arIP[1]= prefixesOctetsAndLenRepresentation[i+1];
        ipPrefixAddr.arIP[2]= prefixesOctetsAndLenRepresentation[i+2];
        ipPrefixAddr.arIP[3]= prefixesOctetsAndLenRepresentation[i+3];
        prefixLen = prefixesOctetsAndLenRepresentation[i+4];

        if (prvUtfBaselineTestsExecutionFlagGet()==GT_TRUE)
        {
            /* in baseline we want to send a packet after every 2000 prefix
               when i=10000 --> prefix index=10000/5=2000 */
            divider=10000;
        }
        else
        {
            /* in non baseline we want to send a packet after every 20000 prefix
               when i=100000 --> prefix index=100000/5=20000 */
            divider=100000;
        }
         /* AUTODOC: send packet to the added prefix only if it a prefix before we got the first fail.
               only then we can be sure we will have traffic .
               in other cases we cannot be sure the prefix was successfully added */
        if((i%divider==0)&&((i/5)<(numOfPrefixesBeforeFirstFail==0 ? numOfExistingPrefixesToDel : numOfPrefixesBeforeFirstFail)))
        {
            cpssOsTimeRT(&temp_start_sec, &temp_start_nsec);
            /* AUTODOC: send packet to the deleted prefix */
            prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipPrefixAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);
            numOfPacketsSent++;
            cpssOsPrintf(".");

            cpssOsTimeRT(&temp_end_sec, &temp_end_nsec);
            if(temp_end_nsec < temp_start_nsec)
            {
                temp_end_nsec += 1000000000;
                temp_end_sec  -= 1;
            }
            temp_diff_nsec  = temp_end_nsec  - temp_start_nsec;
            temp_diff_sec  = temp_end_sec  - temp_start_sec;
            totalTimeOfAllPacketSendAndValidityCheck_nsec +=  temp_diff_nsec;
            totalTimeOfAllPacketSendAndValidityCheck_sec +=  temp_diff_sec;
        }
        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipPrefixAddr, prefixLen);
        if (rc!=GT_OK)
        {
            continue;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);
        numOfPrefixesDeleted++;

        if (prvUtfBaselineTestsExecutionFlagGet()==GT_TRUE)
        {
            if (numOfPrefixesDeleted%5000==0)
            {
                cpssOsTimeRT(&temp_start_sec, &temp_start_nsec);

                /* enable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_TRUE);

                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

                 /* disable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_FALSE);

                cpssOsTimeRT(&temp_end_sec, &temp_end_nsec);
                if(temp_end_nsec < temp_start_nsec)
                {
                    temp_end_nsec += 1000000000;
                    temp_end_sec  -= 1;
                }
                temp_diff_nsec  = temp_end_nsec  - temp_start_nsec;
                temp_diff_sec  = temp_end_sec  - temp_start_sec;
                totalTimeOfAllPacketSendAndValidityCheck_nsec +=  temp_diff_nsec;
                totalTimeOfAllPacketSendAndValidityCheck_sec +=  temp_diff_sec;
            }
        }
    }

    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec - totalTimeOfAllPacketSendAndValidityCheck_sec -(totalTimeOfAllPacketSendAndValidityCheck_nsec/1000000000);
    cpssOsPrintf("\n total running time [%d] seconds, numOfPrefixesDeleted=%d \n",
        diff_sec,numOfPrefixesDeleted);

    cpssOsPrintf("\n numOfPacketsSent=%d \n",numOfPacketsSent);

    /* check if we get the correct number of prefixed deleted */
    UTF_VERIFY_EQUAL2_STRING_MAC(numOfExistingPrefixesToDel , numOfPrefixesDeleted,
                                  "======= numOfExistingPrefixesToDel[%d] is different then numOfPrefixesDeleted value [%d] =======\n",
                                 numOfExistingPrefixesToDel,numOfPrefixesDeleted);
    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping after restore ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

     /* enable LPM validity check */
     prvTgfIpValidityCheckEnable(GT_TRUE);
     firstFailFlag=GT_TRUE;
}

/**
* @internal prvTgfIpLpmCalculateExpectedNumberOfPrefixesForNonBaselineTest function
* @endinternal
*
* @brief   Calculate expected number of prefixes inserted into LPM RAM for non-baseline execution
*
* @param[in] ramDbCfgPtr              -  ram db configuration
*
* @param[out] numOfPrefixesExpectedPtr - expected number of prefixes added until LPM RAM is at full capacity
* @param[out] numOfPrefixesExpectedBeforeFirstFailPtr -  expected number of prefixes added to LPM RAM  until first failure
*                                       GT_OK
*/

static GT_STATUS prvTgfIpLpmCalculateExpectedNumberOfPrefixesForNonBaselineTest
(
    IN  PRV_TGF_LPM_RAM_CONFIG_STC      * ramDbCfgPtr,
    OUT GT_U32                          * numOfPrefixesExpectedPtr,
    OUT GT_U32                          * numOfPrefixesExpectedBeforeFirstFailPtr
)
{
    switch(ramDbCfgPtr->blocksSizeArray[0])
    {
    case (_2K*4):
       *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_NON_BASELINE_TEST_EXECUTION_40K_LINES_CNS;
       *numOfPrefixesExpectedBeforeFirstFailPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_FOR_NON_BASELINE_TEST_EXECUTION_40K_LINES_CNS;
       break;
    case (_13K/2):
       /* Empirically seen at AC3X-48 */
       *numOfPrefixesExpectedPtr = 11419;
       *numOfPrefixesExpectedBeforeFirstFailPtr = 11091;
       break;
    default:
       /* blocksSizeArray[0]== (_6K or _16K)*4  */
       *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_NON_BASELINE_TEST_EXECUTION_CNS;
       *numOfPrefixesExpectedBeforeFirstFailPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_FOR_NON_BASELINE_TEST_EXECUTION_CNS;
    }

    return GT_OK;
}
/**
* @internal prvTgfIpLpmCalculateExpectedNumberOfPrefixesForBaselineTestNonSharing function
* @endinternal
*
* @brief   Calculate expected number of prefixes inserted into LPM RAM for baseline execution with LPM shadow configured
*         with block allocation method is "non - shared"
* @param[in] ramDbCfgPtr              -  ram db configuration
* @param[in] maxNumOfPbrEntries       - max number of pbr entries
*
* @param[out] numOfPrefixesExpectedPtr - expected number of prefixes added until LPM RAM is at full capacity
* @param[out] numOfPrefixesExpectedBeforeFirstFailPtr -  expected number of prefixes added to LPM RAM  until first failure
*
* @retval GT_NOT_SUPPORTED         - in case current configuration of ramDbCfgPtr is not supported by the test
* @retval GT_OK                    - in case current configuration of ramDbCfgPtr is supported by the test
*/


static GT_STATUS prvTgfIpLpmCalculateExpectedNumberOfPrefixesForBaselineTestNonSharing
(
    IN  PRV_TGF_LPM_RAM_CONFIG_STC      * ramDbCfgPtr,
    IN  GT_U32                          maxNumOfPbrEntries,
    OUT GT_U32                          * numOfPrefixesExpectedPtr,
    OUT GT_U32                          * numOfPrefixesExpectedBeforeFirstFailPtr
    )
{
    GT_STATUS rc = GT_OK;

    switch(ramDbCfgPtr->blocksSizeArray[0])
    {
    case (_2K*4):

        if(ramDbCfgPtr->lpmMemMode == PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E)
        {
            /* each block memory has 2K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            if (maxNumOfPbrEntries==0)
            {
                *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS;
            }
        }
        else
        {
            rc = GT_NOT_SUPPORTED;
        }

        break;
    case  (_6K*4):

        if(ramDbCfgPtr->lpmMemMode == PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E)
        {
            /* each block memory has 6K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
            if (maxNumOfPbrEntries==0)
            {
                *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS;
            }
        }
        else
        {
            rc = GT_NOT_SUPPORTED;
        }
        break;
    case  (_8K*4):
        /*the expected result is the same as with 320K lines but in half mode */
        if(ramDbCfgPtr->lpmMemMode == PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E)
        {
            if (maxNumOfPbrEntries==0)
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
            }

        }
        else
        {
            if (maxNumOfPbrEntries==0)
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_HALF_MEMORY_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_HALF_MEMORY_CNS;
            }

        }
        break;
    case  (_16K*4):

        if(ramDbCfgPtr->lpmMemMode == PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E)
        {
            /* each block memory has 16K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 320K Lines */
            if (maxNumOfPbrEntries==0)
            {
                *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS;
            }
        }
        else
        {
            if (maxNumOfPbrEntries==0)
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
            }

        }
        break;
    default:
        rc = GT_NOT_SUPPORTED;
        break;
    }

    return rc;
}
/**
* @internal prvTgfIpLpmCalculateExpectedNumberOfPrefixesForBaselineTestSharing function
* @endinternal
*
* @brief   Calculate expected number of prefixes inserted into LPM RAM for baseline execution with LPM shadow configured
*         with block allocation method is "shared"
* @param[in] ramDbCfgPtr              -  ram db configuration
* @param[in] maxNumOfPbrEntries       - max number of pbr entries
*
* @param[out] numOfPrefixesExpectedPtr - expected number of prefixes added until LPM RAM is at full capacity
* @param[out] numOfPrefixesExpectedBeforeFirstFailPtr -  expected number of prefixes added to LPM RAM  until first failure
*
* @retval GT_NOT_SUPPORTED         - in case current configuration of ramDbCfgPtr is not supported by the test
* @retval GT_OK                    - in case current configuration of ramDbCfgPtr is supported by the test
*/


static GT_STATUS prvTgfIpLpmCalculateExpectedNumberOfPrefixesForBaselineTestSharing
(
    IN  PRV_TGF_LPM_RAM_CONFIG_STC      * ramDbCfgPtr,
    IN  GT_U32                          maxNumOfPbrEntries,
    OUT GT_U32                          * numOfPrefixesExpectedPtr,
    OUT GT_U32                          * numOfPrefixesExpectedBeforeFirstFailPtr
)
{
    GT_STATUS rc = GT_OK;

    switch(ramDbCfgPtr->blocksSizeArray[0])
    {
    case (_2K*4):

        if(ramDbCfgPtr->lpmMemMode == PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E)
        {
            /* each block memory has 2K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            if (maxNumOfPbrEntries == 0)
            {
                *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS;
            }
        }
        else
        {
            rc = GT_NOT_SUPPORTED;
        }

        break;
    case  (_6K*4):

        if(ramDbCfgPtr->lpmMemMode == PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E)
        {
            /* each block memory has 6K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
            if (maxNumOfPbrEntries==0)
            {
                *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS;
            }
        }
        else
        {
            rc = GT_NOT_SUPPORTED;
        }
        break;

    case  (_8K*4):
        /*the expected result is the same as with 320K lines but in half mode */
        if(ramDbCfgPtr->lpmMemMode == PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E)
        {
            if (maxNumOfPbrEntries==0)
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
            }

        }
        else
        {
            if (maxNumOfPbrEntries==0)
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_HALF_MEMORY_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_HALF_MEMORY_CNS;
            }

        }
        break;
    case  (_16K*4):

        if(ramDbCfgPtr->lpmMemMode == PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E)
        {
            /* each block memory has 16K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 320K Lines */
            if (maxNumOfPbrEntries==0)
            {
                *numOfPrefixesExpectedPtr = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS;
            }
        }
        else
        {
            if (maxNumOfPbrEntries==0)
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
            }
            else
            {
                *numOfPrefixesExpectedPtr= PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
                *numOfPrefixesExpectedBeforeFirstFailPtr  = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_HALF_MEMORY_CNS;
            }

        }
        break;
    default:
        rc = GT_NOT_SUPPORTED;
        break;
    }

    return rc;
}



/**
* @internal prvTgfIpLpmCalculateExpectedNumberOfPrefixesForBaselineTest function
* @endinternal
*
* @brief   Calculate expected number of prefixes inserted into LPM RAM for baseline execution
*
* @param[in] ramDbCfgPtr              -  ram db configuration
* @param[in] maxNumOfPbrEntries       - max number of pbr entries
*
* @param[out] numOfPrefixesExpectedPtr - expected number of prefixes added until LPM RAM is at full capacity
* @param[out] numOfPrefixesExpectedBeforeFirstFailPtr -  expected number of prefixes added to LPM RAM  until first failure
*
* @retval GT_NOT_SUPPORTED         - in case current configuration of ramDbCfgPtr is not supported by the test
* @retval GT_OK                    - in case current configuration of ramDbCfgPtr is supported by the test
*/

static GT_STATUS prvTgfIpLpmCalculateExpectedNumberOfPrefixesForBaselineTest
(
    IN  PRV_TGF_LPM_RAM_CONFIG_STC      * ramDbCfgPtr,
    IN  GT_U32                          maxNumOfPbrEntries,
    OUT GT_U32                          * numOfPrefixesExpectedPtr,
    OUT GT_U32                          * numOfPrefixesExpectedBeforeFirstFailPtr
)
{
    GT_STATUS rc;

    switch(ramDbCfgPtr->blocksAllocationMethod)
    {
        case PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E:
            rc = prvTgfIpLpmCalculateExpectedNumberOfPrefixesForBaselineTestNonSharing(ramDbCfgPtr,maxNumOfPbrEntries,numOfPrefixesExpectedPtr,numOfPrefixesExpectedBeforeFirstFailPtr);
            break;
        case PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E:
            rc = prvTgfIpLpmCalculateExpectedNumberOfPrefixesForBaselineTestSharing(ramDbCfgPtr,maxNumOfPbrEntries,numOfPrefixesExpectedPtr,numOfPrefixesExpectedBeforeFirstFailPtr);
            break;
         default:
            rc = GT_BAD_PARAM;
            break;
    }

    return rc;
}


/**
* @internal prvTgfIpLpmCalculateExpectedNumberOfPrefixes function
* @endinternal
*
* @brief   Calculate expected number of prefixes inserted into LPM RAM
*
* @param[in] baselineTestsExecutionFlag - GT_TRUE if this is baseline execution,GT_FALSE otherwise
* @param[in] ramDbCfgPtr              -  ram db configuration
* @param[in] maxNumOfPbrEntries       - max number of pbr entries
*
* @param[out] numOfPrefixesExpectedPtr - expected number of prefixes added until LPM RAM is at full capacity
* @param[out] numOfPrefixesExpectedBeforeFirstFailPtr -  expected number of prefixes added to LPM RAM  until first failure
*
* @retval GT_NOT_SUPPORTED         - in case current configuration of ramDbCfgPtr is not supported by the test
* @retval GT_OK                    - in case current configuration of ramDbCfgPtr is supported by the test
*/

static GT_STATUS prvTgfIpLpmCalculateExpectedNumberOfPrefixes
(
    IN  GT_BOOL                         baselineTestsExecutionFlag,
    IN  PRV_TGF_LPM_RAM_CONFIG_STC      * ramDbCfgPtr,
    IN  GT_U32                          maxNumOfPbrEntries,
    OUT GT_U32                          * numOfPrefixesExpectedPtr,
    OUT GT_U32                          * numOfPrefixesExpectedBeforeFirstFailPtr
)
{
    GT_STATUS rc;

    if(baselineTestsExecutionFlag == GT_TRUE)
    {
       rc = prvTgfIpLpmCalculateExpectedNumberOfPrefixesForBaselineTest(ramDbCfgPtr,maxNumOfPbrEntries,numOfPrefixesExpectedPtr,numOfPrefixesExpectedBeforeFirstFailPtr);
    }
    else
    {   /* if we are running on non baseline test execution then we add only 1/10 of the amount of prefixes in the array,
                 so in all blocksAllocationMethod and with all PBR values in all devices we expect to get the
                 same number of entries */

       rc = prvTgfIpLpmCalculateExpectedNumberOfPrefixesForNonBaselineTest(ramDbCfgPtr,numOfPrefixesExpectedPtr,numOfPrefixesExpectedBeforeFirstFailPtr);
    }

    return rc;
}




