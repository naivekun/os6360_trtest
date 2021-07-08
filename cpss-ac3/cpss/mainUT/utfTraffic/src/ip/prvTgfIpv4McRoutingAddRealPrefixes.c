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
* @file prvTgfIpv4McRoutingAddRealPrefixes.c
*
* @brief IPV4 MC Routing when filling the Lpm using real costumer prefixes defined in a file.
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
#include <ip/prvTgfBasicIpv4McRouting.h>
#include <trunk/prvTgfTrunk.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>

extern GT_U8 prefixesOctetsAndLenRepresentation[];
extern GT_U32 prvTgfPrefixesOctetsAndLenRepresentationArraySizeGet(GT_VOID);

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* MLL pair pointer indexes */
static GT_U32        prvTgfMllPointerIndex     = 5;

/* the Route entry index for MC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 10;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* number of prefixed actually added and need to be deleted */
static GT_U32       numOfExistingPrefixesToDel=0;
/* the last valid prefixed read from the file and successfully added to the system */
static GT_IPADDR    lastGrpIpAddrAdded;
static GT_IPADDR    lastSrcIpAddrAdded;
static GT_U32       lastGrpPrefixLen;
static GT_U32       lastSrcPrefixLen;
/* number of prefixes we should be expecting to be added to the LPM before we get the first fail */
static GT_U32       numOfPrefixesBeforeFirstFail=0;
/* the last valid prefix read from the file and successfully added to the system before the first fail */
static GT_IPADDR    lastGrpIpAddrAddedBeforeFirstFail;
static GT_IPADDR    lastSrcIpAddrAddedBeforeFirstFail;
/* indicate if this is the first time we have a prefix add fail*/
static GT_BOOL      firstFailFlag = GT_TRUE;

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = do_not_share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 47675
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 47353 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS  47675
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 47618
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 47076 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS  47618

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 54032
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 54083 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS   54032
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 54028
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 53998 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS   54028

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = do_not_share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 47675
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 47353 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS   47675
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 47618
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 47076 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS   47618

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 52634
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 52673 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS   52634
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 52620
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 52655 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS   52620

/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 25308 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_CNS   25308
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 25293 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_CNS   25293

/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = do_not_share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_CNS   23433
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_CNS   23429

/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_CNS   26768
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_CNS   26766

/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = do_not_share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 22852*/
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_CNS   22852
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 22723 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_CNS   22723

/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 18558
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 18590 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS   18558
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 18553
    expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 18554 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS   18553

/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = do_not_share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 17649
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 17637 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS   17649
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 17647
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 17636 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS   17647

/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 19836
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 19878 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS   19836
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 19805
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 19838 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS   19805


/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = do_not_share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 17117
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 17078 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS   17117
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 17115
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 17039 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS   17115

/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 18558
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 18590 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS   18558
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 18553
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 18554 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS   18553

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 5719
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 5717 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS   5719
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 5718
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 5717 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS   5718

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share and PBR = 0
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 6384
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 6420 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS   6384
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 6373
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 6400 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS   6373

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when SWAP is used at the beginning of memory allocation    --> 4373
   expected value when SWAP is used before allocating a new memory  block    --> 4341
   expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 4375
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 4323 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS   4375
/* expected value when SWAP is used at the beginning of memory allocation    --> 4373
   expected value when SWAP is used before allocating a new memory  block    --> 4315
   expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 4367
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 4269 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS   4367
/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share and PBR uses 8K lines
   and we add the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 5012
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 5059 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS   5012
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 4988
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 5034 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS   4988

/* number of prefixes expected to be added in a device with 320K/120K LPM lines
   blocksAllocationMethod = do_not_share/share and PBR = 0/not 0
   and we add 1/20 from the prefixes in the file prvTgfIpv4McRoutingAddRealPrefixes_data */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_NON_BASELINE_TEST_EXECUTION_CNS   13308
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_FOR_NON_BASELINE_TEST_EXECUTION_CNS   0

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share/share and PBR = 0/not 0
   and we add 1/10 from the prefixes in the file prvTgfIpv4UcRoutingAddRealPrefixes_data */
/* expected value when SWAP is used at the beginning of memory allocation    --> 4373
   expected value when SWAP is used before allocating a new memory  block    --> 4341
   expected value when Shrink is done - defragmentationEnable=GT_TRUE        --> 4375
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 4323 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_NON_BASELINE_TEST_EXECUTION_40K_LINES_CNS   4375
/* expected value when SWAP is used at the beginning of memory allocation     --> 4373
   expected value when SWAP is used before allocating a new memory  block     --> 4315
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 4367
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 4269 */
#define PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_FOR_NON_BASELINE_TEST_EXECUTION_40K_LINES_CNS   4367

/* divider to use when dealing with different MC groups */
#define PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS   5000

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpLpmMngIpv4McRoutingAddRealPrefixesConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpLpmMngIpv4McRoutingAddRealPrefixesConfigurationAndTrafficGenerate(GT_VOID)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_LPM_RAM_CONFIG_STC              ramDbCfg;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC           mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT              protocol = CPSS_IP_PROTOCOL_IPV4_E;
    PRV_TGF_IP_LTT_ENTRY_STC                mcRouteLttEntry;
    GT_U32                                  i=0;
    GT_IPADDR                               ipGrp;
    GT_IPADDR                               ipSrc;
    GT_U32                                  grpPrefixLen=0;
    GT_U32                                  srcPrefixLen=0;
    GT_U32                                  numOfMcPrefixesAdded=0;
    GT_U32                                  numOfGroupsAdded=0;
    GT_U32                                  numOfMcPrefixesExpected=0;
    GT_U32                                  numOfMcPrefixesExpectedBeforeFirstFail=0;
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
    GT_BOOL                                 defragmentationEnable=GT_TRUE;

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigGet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Create Ipv4 prefixes in a Virtual Router for the specified LPM DB
     */

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before adding new prefixes");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Adding Prefixes using  prvTgfIpMcRouteEntriesWrite\n "
                     "This function tries to add many IPv4 Multicast prefixes according to file ====\n");

    /* create a MC Route entry */
     cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
     mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
     mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
     mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
     mcRouteEntryArray[0].ipv4MllPtr         = prvTgfMllPointerIndex;

     /* AUTODOC: add MC route entry with index 11, MLL Ptr 5 */
     rc = prvTgfIpMcRouteEntriesWrite((prvTgfRouteEntryBaseIndex+1),
                                      1,
                                      protocol,
                                      mcRouteEntryArray);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");

     /* fill a nexthope info for the LPM */
     cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
     mcRouteLttEntry.routeEntryBaseIndex = (prvTgfRouteEntryBaseIndex+1);
     mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

     if(appDemoDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
         maxNumOfPbrEntries = value;
     else
         maxNumOfPbrEntries = _8K;
     if(maxNumOfPbrEntries > PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.routerNextHop)
     {
         maxNumOfPbrEntries = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.routerNextHop;
     }

     /* if we are running on non baseline test execution then we add only 1/20 of the amount of prefixes in the array,
       so in all blocksAllocationMethod and with all PBR values in all devices we expect to get the
       same number of entries */
    if (prvUtfBaselineTestsExecutionFlagGet()==GT_FALSE)
    {
        switch(ramDbCfg.blocksSizeArray[0])
        {
        case (_2K*4):
           numOfMcPrefixesExpected = PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_NON_BASELINE_TEST_EXECUTION_40K_LINES_CNS;
           numOfMcPrefixesExpectedBeforeFirstFail = PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_FOR_NON_BASELINE_TEST_EXECUTION_40K_LINES_CNS;
           break;
        case (_13K/2):
            /* Empirically seen at AC3X-48 */
            numOfMcPrefixesExpected = 3354;
            numOfMcPrefixesExpectedBeforeFirstFail = 3199;
            break;
        default:
            /* blocksSizeArray[0]== (_6K or _16K)*4  */
            numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_FOR_NON_BASELINE_TEST_EXECUTION_CNS;
            numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_FOR_NON_BASELINE_TEST_EXECUTION_CNS;
            break;
        }
    }
    else
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
            if (ramDbCfg.blocksSizeArray[0] == (_2K * 4))
            {
                /* each block memory has 2K lines (not including the blocks holding PBR)
                   multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                if (maxNumOfPbrEntries==0)
                {
                    numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS;
                    numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS;
                }
                else
                {
                    numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS;
                    numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS;
                }
            }
            else
            {
                if (ramDbCfg.blocksSizeArray[0] == (_6K * 4))
                {
                    /* each block memory has 6K lines (not including the blocks holding PBR)
                       multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                    if (maxNumOfPbrEntries==0)
                    {
                        numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS;
                        numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS;
                    }
                    else
                    {
                        numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS;
                        numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS;
                    }
                }
                else if (ramDbCfg.blocksSizeArray[0] == (_8K * 4))
                {
                    /* each block memory has 8K lines (not including the blocks holding PBR)
                       multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                    if (maxNumOfPbrEntries==0)
                    {
                        numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_CNS;
                        numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_CNS;
                    }
                    else
                    {
                        numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_CNS;
                        numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_CNS;
                    }
                }
                else
                {
                    if(ramDbCfg.blocksSizeArray[0]==(_16K*4))
                    {
                        /* each block memory has 16K lines (not including the blocks holding PBR)
                        multiply by max of 20 blocks (in case there is no PBR) --> approximately 320K Lines */
                        if (maxNumOfPbrEntries==0)
                        {
                            numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS;
                            numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS;
                        }
                        else
                        {
                            numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS;
                            numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_NOT_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS;
                        }
                    }
                    else
                    {
                        /* the test do not supported at the moment any other configuration */
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test do not supported at the moment any other configuration: %d", prvTgfDevNum);

                    }
                }
            }
        }
        else
        {
            /* if different octets of the same protocol can share the same block,
               we will have much more prefixes added then in case of no sharing */
            if(ramDbCfg.blocksSizeArray[0]==(_2K*4))
            {
                /* each block memory has 2K lines (not including the blocks holding PBR)
                   multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                if (maxNumOfPbrEntries==0)
                {
                    numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS;
                    numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_40K_LINES_CNS;
                }
                else
                {
                    numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS;
                    numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_40K_LINES_CNS;
                }
            }
            else
            {
                if(ramDbCfg.blocksSizeArray[0]==(_6K*4))
                {
                    /* each block memory has 6K lines (not including the blocks holding PBR)
                       multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                    if (maxNumOfPbrEntries==0)
                    {
                        numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS;
                        numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_120K_LINES_CNS;
                    }
                    else
                    {
                        numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS;
                        numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_120K_LINES_CNS;
                    }
                }
                else if(ramDbCfg.blocksSizeArray[0]==(_8K*4))
                {
                    /* each block memory has 6K lines (not including the blocks holding PBR)
                       multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
                    if (maxNumOfPbrEntries==0)
                    {
                        numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_CNS;
                        numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_160K_LINES_CNS;
                    }
                    else
                    {
                        numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_CNS;
                        numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_160K_LINES_CNS;
                    }
                }
                else
                {
                    if(ramDbCfg.blocksSizeArray[0]==(_16K*4))
                    {
                        /* each block memory has 16K lines (not including the blocks holding PBR)
                        multiply by max of 20 blocks (in case there is no PBR) --> approximately 320K Lines */
                        if (maxNumOfPbrEntries==0)
                        {
                            numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS;
                            numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_NO_PBR_WITH_320K_LINES_CNS;
                        }
                        else
                        {
                           numOfMcPrefixesExpected =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS;
                           numOfMcPrefixesExpectedBeforeFirstFail =  PRV_TGF_MC_EXPECTED_PREFIXES_TO_BE_ADDED_BEFORE_FIRST_FAIL_ALLOW_SHARING_WITH_PBR_WITH_320K_LINES_CNS;
                        }
                    }
                    else
                    {
                        /* the test do not supported at the moment any other configuration */
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test do not supported at the moment any other configuration: %d", prvTgfDevNum);

                    }
                }/* if ramDbCfg.blocksSizeArray[0] != (_6K*4)*/
            }/* end of - dynamic with block sharing */
        }
    }/* end of - running on baseline test execution */

    numOfElementsInArray=prvTgfPrefixesOctetsAndLenRepresentationArraySizeGet();
    if (prvUtfBaselineTestsExecutionFlagGet()==GT_FALSE)
    {
        numOfElementsInArray=numOfElementsInArray/20;
    }

    /* disable LPM validity check */
    prvTgfIpValidityCheckEnable(GT_FALSE);
    cpssOsTimeRT(&start_sec, &start_nsec);
    for(i=0;i<numOfElementsInArray; i=i+5)
    {
        ipGrp.arIP[0]= ((prefixesOctetsAndLenRepresentation[((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)])%16)+ 224; /* range 224-239 */
        ipGrp.arIP[1]= (prefixesOctetsAndLenRepresentation[(((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)+1)]);
        ipGrp.arIP[2]= (prefixesOctetsAndLenRepresentation[(((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)+2)]);
        ipGrp.arIP[3]= (prefixesOctetsAndLenRepresentation[(((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)+3)]);
        grpPrefixLen = 32;

        if (i%PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS==0)
        {
            numOfGroupsAdded++;
        }

        ipSrc.arIP[0] = prefixesOctetsAndLenRepresentation[i];
        ipSrc.arIP[1]= prefixesOctetsAndLenRepresentation[i+1];
        ipSrc.arIP[2]= prefixesOctetsAndLenRepresentation[i+2];
        ipSrc.arIP[3]= (prefixesOctetsAndLenRepresentation[i+3]+(i/5))%255;
        srcPrefixLen = 32;

        /* AUTODOC: add IPv4 MC prefix: */
        rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, grpPrefixLen, ipSrc, srcPrefixLen,
                                   &mcRouteLttEntry, GT_FALSE, defragmentationEnable);

        if((rc==GT_NO_RESOURCE)||(rc==GT_OUT_OF_PP_MEM))
        {
            if (firstFailFlag==GT_TRUE)
            {
                /* check if we get the correct number of prefixed added */
                UTF_VERIFY_EQUAL2_STRING_MAC(numOfMcPrefixesExpectedBeforeFirstFail, numOfMcPrefixesAdded,
                                  "======= numOfMcPrefixesAdded[%d] is different then numOfMcPrefixesExpectedBeforeFirstFail value [%d] =======\n",
                                 numOfMcPrefixesAdded,numOfMcPrefixesExpectedBeforeFirstFail);

                lastGrpIpAddrAddedBeforeFirstFail = lastGrpIpAddrAdded;
                lastSrcIpAddrAddedBeforeFirstFail = lastSrcIpAddrAdded;
                numOfPrefixesBeforeFirstFail = numOfMcPrefixesAdded;

                cpssOsPrintf("\n lastGrpIpAddrAddedBeforeFirstFail = %d.%d.%d.%d/%d, lastSrcIpAddrAddedBeforeFirstFail = %d.%d.%d.%d/%d , numOfPrefixesBeforeFirstFail=%d \n",
                             lastGrpIpAddrAddedBeforeFirstFail.arIP[0],lastGrpIpAddrAddedBeforeFirstFail.arIP[1],
                             lastGrpIpAddrAddedBeforeFirstFail.arIP[2],lastGrpIpAddrAddedBeforeFirstFail.arIP[3],
                             grpPrefixLen,
                             lastSrcIpAddrAddedBeforeFirstFail.arIP[0],lastSrcIpAddrAddedBeforeFirstFail.arIP[1],
                             lastSrcIpAddrAddedBeforeFirstFail.arIP[2],lastSrcIpAddrAddedBeforeFirstFail.arIP[3],
                             srcPrefixLen,numOfPrefixesBeforeFirstFail);
                firstFailFlag=GT_FALSE;
            }
            else
            {
                /*cpssOsPrintf("\n could not add ipGrp = %d.%d.%d.%d/%d \n",ipGrp.arIP[0],
                             ipGrp.arIP[1],ipGrp.arIP[2],ipGrp.arIP[3],grpPrefixLen);                */
            }
            continue;
        }
        else
        {
            if (rc==GT_ALREADY_EXIST)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_ALREADY_EXIST, rc, "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);
            }
        }

        lastGrpIpAddrAdded = ipGrp;
        lastGrpPrefixLen = grpPrefixLen;
        lastSrcIpAddrAdded = ipSrc;
        lastSrcPrefixLen = srcPrefixLen;
        numOfMcPrefixesAdded++;

        if (prvUtfBaselineTestsExecutionFlagGet()==GT_TRUE)
        {
            if (numOfMcPrefixesAdded%5000==0)
            {
                cpssOsTimeRT(&temp_start_sec, &temp_start_nsec);
                cpssOsPrintf(".");

                /* enable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_TRUE);

                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

                /* disable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_FALSE);

                cpssOsTimeRT(&temp_end_sec, &temp_end_nsec);
                if(temp_end_nsec < temp_start_nsec)
                {
                    temp_end_nsec += 500000000;
                    temp_end_sec  -= 1;
                }
                temp_diff_nsec  = temp_end_nsec  - temp_start_nsec;
                temp_diff_sec  = temp_end_sec  - temp_start_sec;
                totalTimeOfAllValiditiChecks_nsec +=  temp_diff_nsec;
                totalTimeOfAllValiditiChecks_sec +=  temp_diff_sec;
            }
        }
    }
    cpssOsPrintf("numOfGroupsAdded=%d\n",numOfGroupsAdded);

    /* check if we get the correct number of prefixed added */
    UTF_VERIFY_EQUAL2_STRING_MAC(numOfMcPrefixesExpected, numOfMcPrefixesAdded,
                                  "======= numOfMcPrefixesAdded[%d] is different then numOfMcPrefixesExpected value [%d] =======\n",
                                 numOfMcPrefixesAdded,numOfMcPrefixesExpected);

    numOfExistingPrefixesToDel = numOfMcPrefixesAdded;

    cpssOsPrintf("\n last ipGrp added = %d.%d.%d.%d/%d, ipSrc added = %d.%d.%d.%d/%d \n",
                 lastGrpIpAddrAdded.arIP[0],lastGrpIpAddrAdded.arIP[1],
                 lastGrpIpAddrAdded.arIP[2],lastGrpIpAddrAdded.arIP[3],lastGrpPrefixLen,
                 lastSrcIpAddrAdded.arIP[0],lastSrcIpAddrAdded.arIP[1],
                 lastSrcIpAddrAdded.arIP[2],lastSrcIpAddrAdded.arIP[3],lastSrcPrefixLen);

    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec - totalTimeOfAllValiditiChecks_sec - (totalTimeOfAllValiditiChecks_nsec/500000000);

    cpssOsPrintf("\n total running time [%d] seconds, numOfMcPrefixesAdded=%d \n",
                 diff_sec,numOfMcPrefixesAdded);

    /* enable LPM validity check */
    prvTgfIpValidityCheckEnable(GT_TRUE);

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

    /* disable LPM validity check */
    prvTgfIpValidityCheckEnable(GT_FALSE);

    /* -------------------------------------------------------------------------
     * 2. send packet to some of the prefixes added
     */
    for(i=0;i<numOfElementsInArray;i=i+5)
    {
        ipGrp.arIP[0]= ((prefixesOctetsAndLenRepresentation[((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)])%16)+ 224; /* range 224-239 */
        ipGrp.arIP[1]= (prefixesOctetsAndLenRepresentation[(((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)+1)]);
        ipGrp.arIP[2]= (prefixesOctetsAndLenRepresentation[(((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)+2)]);
        ipGrp.arIP[3]= (prefixesOctetsAndLenRepresentation[(((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)+3)]);
        grpPrefixLen = 32;

        ipSrc.arIP[0] = prefixesOctetsAndLenRepresentation[i];
        ipSrc.arIP[1]= prefixesOctetsAndLenRepresentation[i+1];
        ipSrc.arIP[2]= prefixesOctetsAndLenRepresentation[i+2];
        ipSrc.arIP[3]= (prefixesOctetsAndLenRepresentation[i+3]+(i/5))%255;
        srcPrefixLen = 32;

        if (prvUtfBaselineTestsExecutionFlagGet()==GT_TRUE)
        {
            /* in baseline we want to send a packet after every 2000 prefix
               when i=5000 --> prefix index=5000/5=2000 */
            divider=5000;
        }
        else
        {
            /* in non baseline we want to send a packet after every 5000 prefix
               when i=50000 --> prefix index=50000/5=5000 */
            divider=50000;
        }
        /* AUTODOC: send packet to the added prefix only if it a prefix before we got the first fail.
               only then we can be sure we will have traffic .
               in other cases we cannot be sure the prefix was successfully added */
        if((i%divider==0)&&((i/5)<(numOfPrefixesBeforeFirstFail==0 ? numOfMcPrefixesAdded : numOfPrefixesBeforeFirstFail)))
        {
            prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp,grpPrefixLen,ipSrc,srcPrefixLen, GT_FALSE);
            prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);
            numOfPacketsSent++;
            cpssOsPrintf(".");
        }
    }

    /* AUTODOC: send packet to the last added prefix  */
    prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(lastGrpIpAddrAdded,lastGrpPrefixLen,lastSrcIpAddrAdded,lastSrcPrefixLen, GT_FALSE);
    prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);
    numOfPacketsSent++;
    cpssOsPrintf(".");
    cpssOsPrintf("\n numOfPacketsSent=%d \n", numOfPacketsSent);
}
/**
* @internal prvTgfIpLpmMngIpv4McRoutingAddRealPrefixesConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpLpmMngIpv4McRoutingAddRealPrefixesConfigurationRestore(GT_VOID)
{
    /* -------------------------------------------------------------------------
     * 1. Restore Route Configuration
     */

    GT_STATUS   rc = GT_OK;
    GT_U32      i=0;
    GT_IPADDR  ipGrp;
    GT_IPADDR  ipSrc;
    GT_U32     grpPrefixLen=0;
    GT_U32     srcPrefixLen=0;
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

    PRV_UTF_LOG0_MAC("==== Deleting Prefixes using  prvTgfIpLpmIpv4McEntryDel\n "
                     "This function tries to delete IPv4 Multicast prefixes added according to a file ====\n");

    numOfElementsInArray=prvTgfPrefixesOctetsAndLenRepresentationArraySizeGet();
    if (prvUtfBaselineTestsExecutionFlagGet()==GT_FALSE)
    {
        numOfElementsInArray=numOfElementsInArray/20;
    }
    /* disable LPM validity check */
    prvTgfIpValidityCheckEnable(GT_FALSE);
    cpssOsTimeRT(&start_sec, &start_nsec);
    for(i=0;i<numOfElementsInArray; i=i+5)
    {
        ipGrp.arIP[0]= ((prefixesOctetsAndLenRepresentation[((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)])%16)+ 224; /* range 224-239 */
        ipGrp.arIP[1]= (prefixesOctetsAndLenRepresentation[(((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)+1)]);
        ipGrp.arIP[2]= (prefixesOctetsAndLenRepresentation[(((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)+2)]);
        ipGrp.arIP[3]= (prefixesOctetsAndLenRepresentation[(((i/PRV_TGF_MC_DIVIDER_FOR_GROUP_COUNT_CNS)*5)+3)]);
        grpPrefixLen = 32;

        ipSrc.arIP[0] = prefixesOctetsAndLenRepresentation[i];
        ipSrc.arIP[1]= prefixesOctetsAndLenRepresentation[i+1];
        ipSrc.arIP[2]= prefixesOctetsAndLenRepresentation[i+2];
        ipSrc.arIP[3]= (prefixesOctetsAndLenRepresentation[i+3]+(i/5))%255;
        srcPrefixLen = 32;

        if (prvUtfBaselineTestsExecutionFlagGet()==GT_TRUE)
        {
            /* in baseline we want to send a packet after every 2000 prefix
               when i=5000 --> prefix index=5000/5=2000 */
            divider=5000;
        }
        else
        {
            /* in non baseline we want to send a packet after every 5000 prefix
               when i=50000 --> prefix index=50000/5=5000 */
            divider=50000;
        }
         /* AUTODOC: send packet to the added prefix only if it a prefix before we got the first fail.
               only then we can be sure we will have traffic .
               in other cases we cannot be sure the prefix was successfully added */
        if((i%divider==0)&&((i/5)<(numOfPrefixesBeforeFirstFail==0 ? numOfExistingPrefixesToDel : numOfPrefixesBeforeFirstFail)))
        {
            cpssOsTimeRT(&temp_start_sec, &temp_start_nsec);
            /* AUTODOC: send packet to the deleted prefix */
            prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp,grpPrefixLen,ipSrc,srcPrefixLen, GT_FALSE);
            prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);
            numOfPacketsSent++;
            cpssOsPrintf(".");

            cpssOsTimeRT(&temp_end_sec, &temp_end_nsec);
            if(temp_end_nsec < temp_start_nsec)
            {
                temp_end_nsec += 500000000;
                temp_end_sec  -= 1;
            }
            temp_diff_nsec  = temp_end_nsec  - temp_start_nsec;
            temp_diff_sec  = temp_end_sec  - temp_start_sec;
            totalTimeOfAllPacketSendAndValidityCheck_nsec +=  temp_diff_nsec;
            totalTimeOfAllPacketSendAndValidityCheck_sec +=  temp_diff_sec;
        }
        rc =  prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId,0,ipGrp,grpPrefixLen,ipSrc,srcPrefixLen);
        if (rc!=GT_OK)
        {
            continue;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryDel: %d", prvTgfDevNum);
        numOfPrefixesDeleted++;

        if (prvUtfBaselineTestsExecutionFlagGet()==GT_TRUE)
        {
            if (numOfPrefixesDeleted%5000==0)
            {
                cpssOsTimeRT(&temp_start_sec, &temp_start_nsec);

                /* enable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_TRUE);

                rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

                 /* disable LPM validity check */
                prvTgfIpValidityCheckEnable(GT_FALSE);

                cpssOsTimeRT(&temp_end_sec, &temp_end_nsec);
                if(temp_end_nsec < temp_start_nsec)
                {
                    temp_end_nsec += 500000000;
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
    diff_sec  = end_sec  - start_sec - totalTimeOfAllPacketSendAndValidityCheck_sec -(totalTimeOfAllPacketSendAndValidityCheck_nsec/500000000);
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

    /* AUTODOC: restore the packet: group 224.1.1.1/32, source 1.1.1.1/32 */
    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 1;
    ipGrp.arIP[2] = 1;
    ipGrp.arIP[3] = 1;

    ipSrc.arIP[0] = 1;
    ipSrc.arIP[1] = 1;
    ipSrc.arIP[2] = 1;
    ipSrc.arIP[3] = 1;

    prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource(ipGrp, 32, ipSrc, 32, GT_TRUE);
}

