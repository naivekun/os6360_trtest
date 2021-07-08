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
* @file prvTgfIpv4Ipv6UcRoutingAddManyIpLpmMng.c
*
* @brief IPV4 & IPV6 UC Routing when filling the Lpm using cpssDxChIpLpmIpv4UcPrefixAddManyByOctet.
* & cpssDxChIpLpmIpv6UcPrefixAddManyByOctet
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
#include <ip/prvTgfIpv6UcRoutingAddMany.h>
#include <ip/prvTgfIpv4UcRoutingAddMany.h>
#include <ip/prvTgfIpv4Ipv6UcRoutingAddManyIpLpmMng.h>
#include <ip/prvTgfIpv4UcRoutingAddManyIpLpmMng.h>
#include <trunk/prvTgfTrunk.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>

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
static GT_U32  numOfExistingIpv4PrefixesToDel=0;
static GT_U32  numOfExistingIpv6PrefixesToDel=0;

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 257533 the test added different number of ipv6
                                                                                           due to shrink implementation causing the ipv4 defined after
                                                                                           to add less entries since octet 3 reached 100% usage
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 271866 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_320K_LINES_CNS     257533

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 302330 less amount added due to new swap area in bank 0
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 302586 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_320K_LINES_CNS     302330

/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 89417  the test added different number of ipv6
                                                                                           due to shrink implementation causing the ipv4 defined after
                                                                                           to add less entries since octet 3 reached 100% usage
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 93690 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_120K_LINES_CNS     89417

/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 103930 less amount added due to new swap area in bank 0
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 104442 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_120K_LINES_CNS        103930

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when SWAP is used at the beginning of memory allocation     --> 23619
   expected value when SWAP is used before allocating a new memory  block     --> 22778
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 23619
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 22778 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_CNS     23619

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when SWAP is used at the beginning of memory allocation     --> 28227
   expected value when SWAP is used before allocating a new memory  block     --> 28154
   expected value when 2 SWAP areas is in used                                --> 27642
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 28067 */
   #define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_CNS      27642

   /* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_160K_LINES_CNS    123218

/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv6UcPrefixAddManyByOctet with
   startIpPrefix=1::1 and octet order is 15,14,..1,0
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_160K_LINES_CNS        144378
/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpLpmMngIpv4Ipv6UcRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpLpmMngIpv4Ipv6UcRoutingAddManyConfigurationAndTrafficGenerate(GT_VOID)
{
    GT_STATUS   rc = GT_OK;
    GT_IPADDR   startIpv4Addr;
    GT_U32      numOfPrefixesToAdd;
    GT_U32      numOfPrefixesExpected = 0;
    GT_U32      numOfPrefixesAdded = 0;
    GT_U32      firstOctetToChangeIpv4;
    GT_U32      secondOctetToChangeIpv4;
    GT_U32      thirdOctetToChangeIpv4;
    GT_U32      forthOctetToChangeIpv4;
    GT_IPADDR   lastIpv4AddrAdded;
    GT_U32      numOfIpv4PrefixesAdded=0;
    GT_IPV6ADDR   startIpv6Addr;
    GT_U32      firstOctetToChangeIpv6;
    GT_U32      secondOctetToChangeIpv6;
    GT_U32      thirdOctetToChangeIpv6;
    GT_U32      forthOctetToChangeIpv6;
    GT_U32      fifthOctetToChangeIpv6;
    GT_U32      sixthOctetToChangeIpv6;
    GT_U32      seventhOctetToChangeIpv6;
    GT_U32      eighthOctetToChangeIpv6;
    GT_U32      ninthOctetToChangeIpv6;
    GT_U32      tenthOctetToChangeIpv6;
    GT_U32      elevenOctetToChangeIpv6;
    GT_U32      twelveOctetToChangeIpv6;
    GT_U32      thirteenOctetToChangeIpv6;
    GT_U32      fourteenOctetToChangeIpv6;
    GT_U32      fifteenOctetToChangeIpv6;
    GT_U32      sixteenOctetToChangeIpv6;
    GT_IPV6ADDR   lastIpv6AddrAdded;
    GT_U32      numOfIpv6PrefixesAdded=0;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U8 i;

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Create Ipv4 prefixes in a Virtual Router for the specified LPM DB
     */

    numOfPrefixesToAdd = 500000;

    firstOctetToChangeIpv4=3;
    secondOctetToChangeIpv4=2;
    thirdOctetToChangeIpv4=1;
    forthOctetToChangeIpv4=0;

    firstOctetToChangeIpv6 = 15;
    secondOctetToChangeIpv6 = 14;
    thirdOctetToChangeIpv6 = 13;
    forthOctetToChangeIpv6 = 12;
    fifthOctetToChangeIpv6 = 11;
    sixthOctetToChangeIpv6 = 10;
    seventhOctetToChangeIpv6 = 9;
    eighthOctetToChangeIpv6 = 8;
    ninthOctetToChangeIpv6 = 7;
    tenthOctetToChangeIpv6 = 6;
    elevenOctetToChangeIpv6 = 5;
    twelveOctetToChangeIpv6 = 4;
    thirteenOctetToChangeIpv6 = 3;
    fourteenOctetToChangeIpv6 = 2;
    fifteenOctetToChangeIpv6 = 1;
    sixteenOctetToChangeIpv6 = 0;


    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before adding new prefixes");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage before adding new prefixes ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);


    startIpv6Addr.arIP[0]=1;
    for (i=1; i<15; i++)
    {
        startIpv6Addr.arIP[i] = 0;
    }
    startIpv6Addr.arIP[15]=1;

    PRV_UTF_LOG0_MAC("==== Adding Prefixes using  cpssDxChIpLpmIpv6UcPrefixAddManyByOctet\n "
                 "This function tries to add many sequential IPv6 Unicast prefixes according\n"
                 "to octet order selection and returns the number of prefixes successfully added. ====\n");

    rc = prvTgfIpLpmIpv6UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, startIpv6Addr, prvTgfRouteEntryBaseIndex,
                                               numOfPrefixesToAdd, firstOctetToChangeIpv6, secondOctetToChangeIpv6,
                                               thirdOctetToChangeIpv6,forthOctetToChangeIpv6, fifthOctetToChangeIpv6,
                                               sixthOctetToChangeIpv6, seventhOctetToChangeIpv6, eighthOctetToChangeIpv6,
                                               ninthOctetToChangeIpv6, tenthOctetToChangeIpv6, elevenOctetToChangeIpv6,
                                               twelveOctetToChangeIpv6, thirteenOctetToChangeIpv6, fourteenOctetToChangeIpv6,
                                               fifteenOctetToChangeIpv6, sixteenOctetToChangeIpv6, &lastIpv6AddrAdded,
                                               &numOfIpv6PrefixesAdded);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAddManyByOctet: %d", prvTgfDevNum);

    numOfExistingIpv6PrefixesToDel = numOfIpv6PrefixesAdded;
    numOfPrefixesAdded += numOfIpv6PrefixesAdded;
    PRV_UTF_LOG1_MAC("\n==== Added %d Ipv6 prefixes\n",numOfIpv6PrefixesAdded);

    /* -------------------------------------------------------------------------
     * 2. send packet to the first and last prefix added
     */

    /* AUTODOC: send packet to the last first added */
    prvTgfIpv6UcRoutingAddManyTrafficGenerate(startIpv6Addr);

    /* AUTODOC: send packet to the last prefix added */
    prvTgfIpv6UcRoutingAddManyTrafficGenerate(lastIpv6AddrAdded);

    startIpv4Addr.arIP[0]=0;
    startIpv4Addr.arIP[1]=0;
    startIpv4Addr.arIP[2]=0;
    startIpv4Addr.arIP[3]=1;

    PRV_UTF_LOG0_MAC("==== Adding Prefixes using  cpssDxChIpLpmIpv4UcPrefixAddManyByOctet\n "
                     "This function tries to add many sequential IPv4 Unicast prefixes according\n"
                     "to octet order selection and returns the number of prefixes successfully added. ====\n");

    rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, startIpv4Addr, prvTgfRouteEntryBaseIndex,
                                               numOfPrefixesToAdd, firstOctetToChangeIpv4, secondOctetToChangeIpv4,
                                               thirdOctetToChangeIpv4,forthOctetToChangeIpv4, &lastIpv4AddrAdded, &numOfIpv4PrefixesAdded);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAddManyByOctet: %d", prvTgfDevNum);

    numOfExistingIpv4PrefixesToDel = numOfIpv4PrefixesAdded;
    numOfPrefixesAdded += numOfIpv4PrefixesAdded;
    PRV_UTF_LOG1_MAC("\n==== Added %d Ipv4 prefixes\n",numOfIpv4PrefixesAdded);

    /* -------------------------------------------------------------------------
     * 2. send packet to the first and last prefix added
     */

    /* AUTODOC: send packet to the last first added */
    prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], startIpv4Addr, PRV_TGF_NEXTHOPE_VLANID_CNS);

    /* AUTODOC: send packet to the last prefix added */
    prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], lastIpv4AddrAdded, PRV_TGF_NEXTHOPE_VLANID_CNS);


    PRV_UTF_LOG0_MAC("\n==== HW Octet to Block Mapping after adding new prefixes");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage after adding new prefixes ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


    if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
    {
        switch(ramDbCfg.blocksSizeArray[0])
        {
        case (_6K*4):
            /* each block memory has 6K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_120K_LINES_CNS;
            break;
        case (_16K*4):
            /* each block memory has 16K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 320K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_320K_LINES_CNS;
            break;
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_CNS;
            break;
        case (_13K/2):
            /* Empirically seen at AC3X-48 */
            numOfPrefixesExpected = 18203;
            break;
        case (_8K*4):
            /* each block memory has 8K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 160K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_160K_LINES_CNS;
            break;
        default:
            /* the test does not support at the moment any other configuration */
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            break;
        }
    }
    else
    {
        /* if different octets of the same protocol can share the same block,
           we will have much more prefixes added then in case of no sharing */

        switch(ramDbCfg.blocksSizeArray[0])
        {
        case (_6K*4):
            /* each block memory has 6K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_120K_LINES_CNS;
            break;
        case (_16K*4):
            /* each block memory has 16K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 320K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_320K_LINES_CNS;
            break;
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_CNS;
            break;
        case (_13K/2):
            /* Empirically seen at AC3X-48 */
            numOfPrefixesExpected = 22381;
            break;
        case (_8K*4):
            /* each block memory has 8K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 160K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_160K_LINES_CNS;
            break;
        default:
            /* the test does not support at the moment any other configuration */
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            break;
        }
    }
    /* check if we get the correct number of prefixed added */
    UTF_VERIFY_EQUAL2_STRING_MAC(numOfPrefixesExpected, numOfPrefixesAdded,
                                  "======= numOfPrefixesAdded[%d] is different then numOfPrefixesExpected value [%d] =======\n",
                                 numOfPrefixesAdded,numOfPrefixesExpected);

}

/**
* @internal prvTgfIpLpmMngIpv4Ipv6UcRoutingAddManyConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpLpmMngIpv4Ipv6UcRoutingAddManyConfigurationRestore(GT_VOID)
{
    /* -------------------------------------------------------------------------
     * 1. Restore Route Configuration
     */

    GT_STATUS   rc = GT_OK;
    GT_IPADDR   startIpv4Addr;
    GT_U32      firstOctetToChangeIpv4;
    GT_U32      secondOctetToChangeIpv4;
    GT_U32      thirdOctetToChangeIpv4;
    GT_U32      forthOctetToChangeIpv4;
    GT_IPADDR   lastIpv4AddrDeleted;
    GT_U32      numOfIpv4PrefixesDeleted=0;
    GT_IPV6ADDR   startIpv6Addr;
    GT_U32      firstOctetToChangeIpv6;
    GT_U32      secondOctetToChangeIpv6;
    GT_U32      thirdOctetToChangeIpv6;
    GT_U32      forthOctetToChangeIpv6;
    GT_U32      fifthOctetToChangeIpv6;
    GT_U32      sixthOctetToChangeIpv6;
    GT_U32      seventhOctetToChangeIpv6;
    GT_U32      eighthOctetToChangeIpv6;
    GT_U32      ninthOctetToChangeIpv6;
    GT_U32      tenthOctetToChangeIpv6;
    GT_U32      elevenOctetToChangeIpv6;
    GT_U32      twelveOctetToChangeIpv6;
    GT_U32      thirteenOctetToChangeIpv6;
    GT_U32      fourteenOctetToChangeIpv6;
    GT_U32      fifteenOctetToChangeIpv6;
    GT_U32      sixteenOctetToChangeIpv6;
    GT_IPV6ADDR   lastIpv6AddrDeleted;
    GT_U32      numOfIpv6PrefixesDeleted;
    GT_U32      numOfPrefixesExpected=0;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32 i;

     rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before restore ");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage before restore ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    firstOctetToChangeIpv4=3;
    secondOctetToChangeIpv4=2;
    thirdOctetToChangeIpv4=1;
    forthOctetToChangeIpv4=0;

    firstOctetToChangeIpv6 = 15;
    secondOctetToChangeIpv6 = 14;
    thirdOctetToChangeIpv6 = 13;
    forthOctetToChangeIpv6 = 12;
    fifthOctetToChangeIpv6 = 11;
    sixthOctetToChangeIpv6 = 10;
    seventhOctetToChangeIpv6 = 9;
    eighthOctetToChangeIpv6 = 8;
    ninthOctetToChangeIpv6 = 7;
    tenthOctetToChangeIpv6 = 6;
    elevenOctetToChangeIpv6 = 5;
    twelveOctetToChangeIpv6 = 4;
    thirteenOctetToChangeIpv6 = 3;
    fourteenOctetToChangeIpv6 = 2;
    fifteenOctetToChangeIpv6 = 1;
    sixteenOctetToChangeIpv6 = 0;

    PRV_UTF_LOG0_MAC("==== Deleting Prefixes using  cpssDxChIpLpmIpv4UcPrefixDelManyByOctet\n "
                     "This function tries to delete many sequential IPv4 Unicast prefixes according\n"
                     "to octet order selection and returns the number of prefixes successfully deleted. ====\n");

    startIpv4Addr.arIP[0]=0;
    startIpv4Addr.arIP[1]=0;
    startIpv4Addr.arIP[2]=0;
    startIpv4Addr.arIP[3]=1;

    rc = prvTgfIpLpmIpv4UcPrefixDelManyByOctet(prvTgfLpmDBId, 0, startIpv4Addr,numOfExistingIpv4PrefixesToDel,
                                               firstOctetToChangeIpv4, secondOctetToChangeIpv4,
                                               thirdOctetToChangeIpv4,forthOctetToChangeIpv4,
                                               &lastIpv4AddrDeleted, &numOfIpv4PrefixesDeleted);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDelManyByOctet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Deleting Prefixes using  cpssDxChIpLpmIpv6UcPrefixDelManyByOctet\n "
                     "This function tries to delete many sequential IPv6 Unicast prefixes according\n"
                     "to octet order selection and returns the number of prefixes successfully deleted. ====\n");

    startIpv6Addr.arIP[0]=1;
    for (i=1; i<15; i++)
    {
        startIpv6Addr.arIP[i] = 0;
    }
    startIpv6Addr.arIP[15]=1;

    rc = prvTgfIpLpmIpv6UcPrefixDelManyByOctet(prvTgfLpmDBId, 0, startIpv6Addr,numOfExistingIpv6PrefixesToDel,
                                               firstOctetToChangeIpv6, secondOctetToChangeIpv6,
                                               thirdOctetToChangeIpv6, forthOctetToChangeIpv6, fifthOctetToChangeIpv6,
                                               sixthOctetToChangeIpv6, seventhOctetToChangeIpv6, eighthOctetToChangeIpv6,
                                               ninthOctetToChangeIpv6, tenthOctetToChangeIpv6, elevenOctetToChangeIpv6,
                                               twelveOctetToChangeIpv6, thirteenOctetToChangeIpv6, fourteenOctetToChangeIpv6,
                                               fifteenOctetToChangeIpv6, sixteenOctetToChangeIpv6,
                                               &lastIpv6AddrDeleted, &numOfIpv6PrefixesDeleted);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDelManyByOctet: %d", prvTgfDevNum);

    if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
    {
        switch(ramDbCfg.blocksSizeArray[0])
        {
        case (_6K*4):
            /* each block memory has 6K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_120K_LINES_CNS;
            break;
        case (_16K*4):
            /* each block memory has 16K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_320K_LINES_CNS;
            break;
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_CNS;
            break;
        case (_13K/2):
            /* Empirically seen at AC3X-48 */
            numOfPrefixesExpected = 18203;
            break;
        case (_8K*4):
            /* each block memory has 8K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 160K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_160K_LINES_CNS;
            break;
        default:
            /* the test does not support at the moment any other configuration */
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
        }
    }
    else
    {
        /* if different octets of the same protocol can share the same block,
           we will have much more prefixes added then in case of no sharing */

        switch(ramDbCfg.blocksSizeArray[0])
        {
        case (_6K*4):
            /* each block memory has 6K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_120K_LINES_CNS;
            break;
        case (_16K*4):
            /* each block memory has 16K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 120K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_320K_LINES_CNS;
            break;
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
               multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_CNS;
            break;
        case (_13K/2):
            /* Empirically seen at AC3X-48 */
            numOfPrefixesExpected = 22381;
            break;
        case (_8K*4):
            /* each block memory has 8K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 160K Lines */
            numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_160K_LINES_CNS;
            break;
        default:
            /* the test does not support at the moment any other configuration */
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
        }
    }

    /* check if we get the correct number of prefixed deleted */
    UTF_VERIFY_EQUAL2_STRING_MAC(numOfPrefixesExpected, numOfExistingIpv4PrefixesToDel+numOfExistingIpv6PrefixesToDel,
                                  "======= numOfExistingPrefixesToDel[%d] is different then numOfPrefixesExpected value [%d] =======\n",
                                 numOfExistingIpv4PrefixesToDel+numOfExistingIpv6PrefixesToDel,numOfPrefixesExpected);

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping after restore ");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage after restore ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    /* change blocks allocation method and reset LPM DB */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

}


