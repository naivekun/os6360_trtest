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
* @file prvTgfIpv4Ipv6UcMcRoutingAddDelAndFillLpm.c
*
* @brief IPV4 & IPV6 UC Routing when filling the Lpm and using many Add and
* Delete operations.
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

#include <common/tgfIpGen.h>
#include <ip/prvTgfIpv4Ipv6UcMcRoutingAddDelAndFillLpm.h>

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 6 --> changing octet 6 then octet 7 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 9127
   expected value when SWAP is used before allocating a new memory  block     --> 8702
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 9127
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 8702 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_6_7_UC_OCTET_CNS     9127

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC source octet 12 then octet 13 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 1363
   expected value when SWAP is used before allocating a new memory  block     --> 1363
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 1363
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 1363
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 1363 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_SRC_CHANGE_CNS     1363

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 14 --> changing MC source octet 15 then octet 14 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 9127
   expected value when SWAP is used before allocating a new memory  block     --> 8702
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 9127
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 8702 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_SRC_CHANGE_CNS     9127

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC source octet 3 then octet 2 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 30631
   expected value when SWAP is used before allocating a new memory  block     --> 30206
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 30631
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 30206 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_SRC_CHANGE_CNS   30631

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 0 --> changing MC source octet 0 then octet 1 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 4773
   expected value when SWAP is used before allocating a new memory  block     --> 4773
   expected value when 2 SWAP areas is in used                                --> 4773
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 4773
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 4773
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 4773 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_SRC_CHANGE_CNS  4773

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC group octet 12 then octet 13 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 585
   expected value when SWAP is used before allocating a new memory  block     --> 585
   expected value when 2 SWAP areas is in used                                --> 511
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 511
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 511
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 585 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_GRP_CHANGE_CNS     511

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 14 --> changing MC group octet 15 then octet 14 sequentially */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_GRP_CHANGE_CNS     681


/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC group octet 3 then octet 2 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 2727
   expected value when SWAP is used before allocating a new memory  block     --> 2727
   expected value when 2 SWAP areas is in used                                --> 2727
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 2727
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 2727
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 2727 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_GRP_CHANGE_CNS     2727

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 0 --> changing MC group octet 0 then octet 1 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 4773
   expected value when SWAP is used before allocating a new memory  block     --> 4773
   expected value when 2 SWAP areas is in used                                --> 4773
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 4773
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 4773
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 4773 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_GRP_CHANGE_CNS     4773

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 6 --> changing UC octet 6 then octet 7 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 35049
   expected value when SWAP is used before allocating a new memory  block     --> 34558
   expected value when 2 SWAP areas is in used                                --> 34814
   expected value when defragmentationEnable=GT_TRUE                          --> 34814
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 34558 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_6_7_UC_OCTET_CNS     34814

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 14 --> changing MC source octet 15 then octet 14 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 35042
   expected value when SWAP is used before allocating a new memory  block     --> 34558
   expected value when 2 SWAP areas is in used                               --> 34814
   expected value when defragmentationEnable=GT_TRUE                         --> 34814
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 34558 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_SRC_CHANGE_CNS     34814

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC source octet 12 then octet 13 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 5224
   expected value when SWAP is used before allocating a new memory  block     --> 5191
   expected value when 2 SWAP areas is in used                                --> 5170
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 5195 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_SRC_CHANGE_CNS     5170

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC source octet 3 then octet 2 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 35043
   expected value when SWAP is used before allocating a new memory  block     --> 34558
   expected value when 2 SWAP areas is in used                                --> 34814
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 34558 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_SRC_CHANGE_CNS     34814

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 0 --> changing MC source octet 0 then octet 1 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 5282
   expected value when SWAP is used before allocating a new memory  block     --> 5287
   expected value when 2 SWAP areas is in used                                --> 5251
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 5285 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_SRC_CHANGE_CNS     5251


/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 14 --> changing MC group octet 15 then octet 14 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 778
   expected value when SWAP is used before allocating a new memory  block     --> 778
   expected value when 2 SWAP areas is in used                                --> 773
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 778 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_GRP_CHANGE_CNS     773

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC group octet 12 then octet 13 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 686
   expected value when SWAP is used before allocating a new memory  block     --> 686
   expected value when 2 SWAP areas is in used                                --> 682
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 686 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_GRP_CHANGE_CNS     682

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC group octet 3 then octet 2 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 3063
   expected value when SWAP is used before allocating a new memory  block     --> 3098
   expected value when 2 SWAP areas is in used                                --> 3049
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 3034 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_GRP_CHANGE_CNS     3049

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 0 --> changing MC group octet 0 then octet 1 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 5282
   expected value when SWAP is used before allocating a new memory  block     --> 5287
   expected value when 2 SWAP areas is in used                                --> 5251
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 5285 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_GRP_CHANGE_CNS 5251


/**
* @internal prvTgfIpLpmIpv4Ipv6UcPrefixAddSomePrefix function
* @endinternal
*
* @brief   Add some ipv4 and ipv6 Prefixes
*/
GT_STATUS prvTgfIpLpmIpv4Ipv6UcPrefixAddSomePrefix
(
    GT_VOID
)
{
        GT_STATUS rc = GT_OK;
        GT_BOOL                                 override;
        GT_BOOL                                 defragmentationEnable;
        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
        GT_IPV6ADDR                             unIpv6AddrArray[4];
        GT_U32                                  ipv6PrefixLenArray[4];
        GT_IPADDR                               unIpv4AddrArray[4];
        GT_U32                                  ipv4PrefixLenArray[4];
        GT_U32                                  i=0;

        cpssOsMemSet(&nextHopInfo, '\0', sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
        }
        else
        {
            nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
        }
        nextHopInfo.ipLttEntry.numOfPaths = 0;
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = 1;
        nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
        nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;


        cpssOsMemSet(&unIpv6AddrArray, '\0', sizeof(unIpv6AddrArray));
        unIpv6AddrArray[0].arIP[15] = 0x1;
        ipv6PrefixLenArray[0]=128;

        unIpv6AddrArray[1].arIP[1] = 3;
        ipv6PrefixLenArray[1]=64;

        unIpv6AddrArray[2].arIP[1] = 3;
        unIpv6AddrArray[2].arIP[15] = 0x1;
        ipv6PrefixLenArray[2]=128;

        unIpv6AddrArray[3].arIP[0] = 0xfe;
        unIpv6AddrArray[3].arIP[1] = 0x80;
        ipv6PrefixLenArray[3]=10;

        override = GT_FALSE;
        defragmentationEnable = GT_TRUE;

        for (i=0;i<4;i++)
        {
            rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6AddrArray[i], ipv6PrefixLenArray[i], &nextHopInfo, override, defragmentationEnable);
            if (rc == GT_OK)
            {
                cpssOsPrintf("[ipv6 unicast added]  = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]/%d,\n\r",
                             unIpv6AddrArray[i].arIP[0]
                            ,unIpv6AddrArray[i].arIP[1]
                            ,unIpv6AddrArray[i].arIP[2]
                            ,unIpv6AddrArray[i].arIP[3]
                            ,unIpv6AddrArray[i].arIP[4]
                            ,unIpv6AddrArray[i].arIP[5]
                            ,unIpv6AddrArray[i].arIP[6]
                            ,unIpv6AddrArray[i].arIP[7]
                            ,unIpv6AddrArray[i].arIP[8]
                            ,unIpv6AddrArray[i].arIP[9]
                            ,unIpv6AddrArray[i].arIP[10]
                            ,unIpv6AddrArray[i].arIP[11]
                            ,unIpv6AddrArray[i].arIP[12]
                            ,unIpv6AddrArray[i].arIP[13]
                            ,unIpv6AddrArray[i].arIP[14]
                            ,unIpv6AddrArray[i].arIP[15]
                            ,ipv6PrefixLenArray[i]);
            }
        }

        /* IPV4 */

        cpssOsMemSet(&unIpv4AddrArray, '\0', sizeof(unIpv4AddrArray));
        unIpv4AddrArray[0].arIP[0] = 127;
        ipv4PrefixLenArray[0]=8;

        unIpv4AddrArray[1].arIP[0] = 192;
        ipv4PrefixLenArray[1]=8;

        unIpv4AddrArray[2].arIP[0] = 192;
        unIpv4AddrArray[2].arIP[1] = 94;
        unIpv4AddrArray[2].arIP[2] = 20;
        ipv4PrefixLenArray[2]=24;

        unIpv4AddrArray[3].arIP[0] = 192;
        unIpv4AddrArray[3].arIP[1] = 94;
        unIpv4AddrArray[3].arIP[2] = 20;
        unIpv4AddrArray[3].arIP[3] = 39;
        ipv4PrefixLenArray[3]=32;

        for (i=0;i<4;i++)
        {
            rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, unIpv4AddrArray[i],ipv4PrefixLenArray[i], &nextHopInfo, override,defragmentationEnable);
            if (rc == GT_OK)
            {
                cpssOsPrintf("[ipv4 unicast added] = [%d]:[%d]:[%d]:[%d]/%d,\n\r",
                          unIpv4AddrArray[i].arIP[0]
                         ,unIpv4AddrArray[i].arIP[1]
                         ,unIpv4AddrArray[i].arIP[2]
                         ,unIpv4AddrArray[i].arIP[3]
                         ,ipv4PrefixLenArray[i]);

            }
        }
        return rc;
}

/**
* @internal prvTgfIpLpmIpv6UcPrefix64 function
* @endinternal
*
* @brief   Add ipv6 UC entries with prefixLen 64
*
* @param[in] totalNumIpUc             - number of ipv6 UC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the change.
*                                      next octet changed will be firstOctetIndexToChange+1
*                                      and so on
* @param[in] delete                   - is this an add or  operation
* @param[in] FirstByteIndex           - A value different then 0 will be given to the prefix
*                                      in location of  FirstByteIndex
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcPrefix64
(
    GT_U32  totalNumIpUc,
    GT_U32  firstOctetIndexToChange,
    GT_BOOL delete,
    GT_U32  FirstByteIndex,
    GT_BOOL order
)
{
    GT_STATUS rc = GT_BAD_PARAM;
    GT_U32    i;

    GT_IPV6ADDR                             unIpv6Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    PRV_TGF_LPM_RAM_CONFIG_STC              ramDbCfg;
    GT_U32                                  numOfPrefixesExpected=0;
    GT_U32                                  counter = 0;

    if (totalNumIpUc>= 256*256)
    {
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "totalNumIpUc should be less than 256*256, totalNumIpUc=%d\n\r",totalNumIpUc);
       return;
    }
    if (firstOctetIndexToChange>= 15)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "index should be less than 15, firstOctetIndexToChange=%d\n\r",firstOctetIndexToChange);
        return;
    }

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            numOfPrefixesExpected = 6103;
        }
        else
        {
            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_6_7_UC_OCTET_CNS;
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }

        }
    }
    else
    {
        /* if different octets of the same protocol can share the same block,
           we will have much more prefixes added then in case of no sharing */
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            numOfPrefixesExpected = 50944;
        }
        else
        {
            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_6_7_UC_OCTET_CNS;
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
        defragmentationEnable = GT_FALSE;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
        defragmentationEnable = GT_TRUE;
    }
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));
    unIpv6Addr.arIP[FirstByteIndex] = 0x5;
    ipPrefixLen = 64;
    override = GT_FALSE;


    cpssOsPrintf("test will take a while please wait....\n");
    for (i = 0; i < totalNumIpUc; i ++)
    {
        if(order == GT_FALSE)
        {
            if(i%256 == 0)
            {
                if(i!=0)
                {
                    (unIpv6Addr.arIP[firstOctetIndexToChange])++;
                }

                unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
            }
            else
            {
                (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;
            }
        }
        else
        {
            (unIpv6Addr.u32Ip[firstOctetIndexToChange]) =i+1;
        }

        if (delete==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, unIpv6Addr, ipPrefixLen);
        }
        else
        {
            rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6Addr, ipPrefixLen, &nextHopInfo, override, defragmentationEnable);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            /* remove remark in case debug is needed
              cpssOsPrintf("[ipv6 unicast added/deleted][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         unIpv6Addr.arIP[0]
                         ,unIpv6Addr.arIP[1]
                         ,unIpv6Addr.arIP[2]
                         ,unIpv6Addr.arIP[3]
                         ,unIpv6Addr.arIP[4]
                         ,unIpv6Addr.arIP[5]
                         ,unIpv6Addr.arIP[6]
                         ,unIpv6Addr.arIP[7]
                         ,unIpv6Addr.arIP[8]
                         ,unIpv6Addr.arIP[9]
                         ,unIpv6Addr.arIP[10]
                         ,unIpv6Addr.arIP[11]
                         ,unIpv6Addr.arIP[12]
                         ,unIpv6Addr.arIP[13]
                         ,unIpv6Addr.arIP[14]
                         ,unIpv6Addr.arIP[15]);*/
            if (rc == GT_OK)
            {
                counter++;
            }
        }
        else
        {
            break;
        }
    }
    if (i!=numOfPrefixesExpected)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding/deleting the amount wanted. added/deleted(%d/%d).\n",i,numOfPrefixesExpected);
    }
    else
    {
        cpssOsPrintf("\n We succeed in adding/deleting the amount wanted. added/deleted(%d/%d).\n",i,numOfPrefixesExpected);
    }
    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);
    return;
}


/**
* @internal prvTgfIpLpmIpv6UcPrefix128 function
* @endinternal
*
* @brief   Add and delete ipv6 UC entries with prefixLen 128
*
* @param[in] first_totalNumIpUc       - number of ipv6 UC entries to add
* @param[in] second_totalNumIpUc      - number of ipv6 UC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the change.
*                                      next octet changed will be firstOctetIndexToChange+1
*                                      and so on
* @param[in] delete                   - is this an add or  operation
* @param[in] FirstByteIndex           - A value different then 0 will be given to the prefix
*                                      in location of  FirstByteIndex
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcPrefix128
(
    GT_U32  first_totalNumIpUc,
    GT_U32  second_totalNumIpUc,
    GT_U32  firstOctetIndexToChange,
    GT_BOOL delete,
    GT_U32  FirstByteIndex,
    GT_BOOL order
)
{
    GT_STATUS rc = GT_BAD_PARAM;
    GT_U32    i;

    GT_IPV6ADDR                             unIpv6Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;

    if (first_totalNumIpUc>= 256*256)
    {
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "first_totalNumIpUc should be less than 256*256, first_totalNumIpUc=%d\n\r",first_totalNumIpUc);
       return;
    }
    if (second_totalNumIpUc>= 256*256)
    {
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "second_totalNumIpUc should be less than 256*256, totalNumIpUc=%d\n\r",second_totalNumIpUc);
       return;
    }
    if (firstOctetIndexToChange>= 15)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "index should be less than 15, firstOctetIndexToChange=%d\n\r",firstOctetIndexToChange);
        return;
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));
    unIpv6Addr.arIP[FirstByteIndex] = 0x5;
    ipPrefixLen = 128;
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;

    cpssOsPrintf("test will take a while please wait....\n");
    for (i = 0; i < first_totalNumIpUc; i ++)
    {
        if(order == GT_FALSE)
        {
            if (i % 256 == 0)
            {
                if(i!=0)
                {
                    (unIpv6Addr.arIP[firstOctetIndexToChange])++;
                }

                unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
            }

            else
            {
                (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;
            }
        }
        else
        {
            (unIpv6Addr.u32Ip[firstOctetIndexToChange]) =i+1;
        }

        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6Addr,ipPrefixLen, &nextHopInfo, override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            /*  remove remark in case debug is needed
              cpssOsPrintf("[ipv6 unicast added][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         unIpv6Addr.arIP[0]
                         ,unIpv6Addr.arIP[1]
                         ,unIpv6Addr.arIP[2]
                         ,unIpv6Addr.arIP[3]
                         ,unIpv6Addr.arIP[4]
                         ,unIpv6Addr.arIP[5]
                         ,unIpv6Addr.arIP[6]
                         ,unIpv6Addr.arIP[7]
                         ,unIpv6Addr.arIP[8]
                         ,unIpv6Addr.arIP[9]
                         ,unIpv6Addr.arIP[10]
                         ,unIpv6Addr.arIP[11]
                         ,unIpv6Addr.arIP[12]
                         ,unIpv6Addr.arIP[13]
                         ,unIpv6Addr.arIP[14]
                         ,unIpv6Addr.arIP[15]);
                         */
        }
        else
        {
            break;
        }
    }
    if (i!=first_totalNumIpUc)
    {
         UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,first_totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,first_totalNumIpUc);
    }

   cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));
   unIpv6Addr.arIP[FirstByteIndex] = 0x6;

   cpssOsPrintf("test will take a while please wait....\n");
   for (i = 0; i < second_totalNumIpUc; i ++)
   {
        if(i%256 == 0)
        {
            if(i!=0)
                (unIpv6Addr.arIP[firstOctetIndexToChange])++;

            unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
        }

        else
        {
            (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;
        }

        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6Addr,ipPrefixLen, &nextHopInfo, override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            /*  remove remark in case debug is needed
            cpssOsPrintf("[ipv6 unicast added][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                    i,
                    unIpv6Addr.arIP[0]
                    ,unIpv6Addr.arIP[1]
                    ,unIpv6Addr.arIP[2]
                    ,unIpv6Addr.arIP[3]
                    ,unIpv6Addr.arIP[4]
                    ,unIpv6Addr.arIP[5]
                    ,unIpv6Addr.arIP[6]
                    ,unIpv6Addr.arIP[7]
                    ,unIpv6Addr.arIP[8]
                    ,unIpv6Addr.arIP[9]
                    ,unIpv6Addr.arIP[10]
                    ,unIpv6Addr.arIP[11]
                    ,unIpv6Addr.arIP[12]
                    ,unIpv6Addr.arIP[13]
                    ,unIpv6Addr.arIP[14]
                    ,unIpv6Addr.arIP[15]);
                    */
        }
        else
        {
            break;
        }
   }
   if (i!=second_totalNumIpUc)
   {
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,second_totalNumIpUc);
   }
   else
   {
       cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,second_totalNumIpUc);
   }

   if (delete == GT_TRUE)
   {

        cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));

        /* a fixed ipSrc */
        unIpv6Addr.arIP[FirstByteIndex] = 0x5;

        cpssOsPrintf("\n<<<<<<<<START DELETE<<<<<<<\n\r");
        for (i = 0; i < first_totalNumIpUc; i ++)
        {
            if(i%256 == 0)
            {
                if(i!=0)
                (unIpv6Addr.arIP[firstOctetIndexToChange])++;

                unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
            }

            else
                (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;


            rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, unIpv6Addr, ipPrefixLen);

            if ((rc == GT_OK))
            {
                /*  remove remark in case debug is needed
                cpssOsPrintf("[ipv6 unicast deleted][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                             i,
                             unIpv6Addr.arIP[0]
                             ,unIpv6Addr.arIP[1]
                             ,unIpv6Addr.arIP[2]
                             ,unIpv6Addr.arIP[3]
                             ,unIpv6Addr.arIP[4]
                             ,unIpv6Addr.arIP[5]
                             ,unIpv6Addr.arIP[6]
                             ,unIpv6Addr.arIP[7]
                             ,unIpv6Addr.arIP[8]
                             ,unIpv6Addr.arIP[9]
                             ,unIpv6Addr.arIP[10]
                             ,unIpv6Addr.arIP[11]
                             ,unIpv6Addr.arIP[12]
                             ,unIpv6Addr.arIP[13]
                             ,unIpv6Addr.arIP[14]
                             ,unIpv6Addr.arIP[15]);
                             */
            }
            else
            {
                break;
            }
        }
        if (i!=second_totalNumIpUc)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,second_totalNumIpUc);
        }
        else
        {
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,second_totalNumIpUc);
        }

        cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));

        /* a fixed ipSrc */
        unIpv6Addr.arIP[FirstByteIndex] = 0x6;

        cpssOsPrintf("\n<<<<<<<<START DELETE<<<<<<<\n\r");
        for (i = 0; i < second_totalNumIpUc; i ++)
        {
            if(i%256 == 0)
            {
                if(i!=0)
                (unIpv6Addr.arIP[firstOctetIndexToChange])++;

                unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
            }

            else
                (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;

            rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, unIpv6Addr,ipPrefixLen);

            if ((rc == GT_OK))
            {
                /*  remove remark in case debug is needed
                cpssOsPrintf("[ipv6 unicast deleted][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                             i,
                             unIpv6Addr.arIP[0]
                             ,unIpv6Addr.arIP[1]
                             ,unIpv6Addr.arIP[2]
                             ,unIpv6Addr.arIP[3]
                             ,unIpv6Addr.arIP[4]
                             ,unIpv6Addr.arIP[5]
                             ,unIpv6Addr.arIP[6]
                             ,unIpv6Addr.arIP[7]
                             ,unIpv6Addr.arIP[8]
                             ,unIpv6Addr.arIP[9]
                             ,unIpv6Addr.arIP[10]
                             ,unIpv6Addr.arIP[11]
                             ,unIpv6Addr.arIP[12]
                             ,unIpv6Addr.arIP[13]
                             ,unIpv6Addr.arIP[14]
                             ,unIpv6Addr.arIP[15]);
                             */
            }
            else
            {
                break;
            }
        }
        if (i!=second_totalNumIpUc)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,second_totalNumIpUc);
        }
        else
        {
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,second_totalNumIpUc);
        }
     }
    return;
}

/**
* @internal prvTgfIpLpmIpv6McPrefixSrcChange function
* @endinternal
*
* @brief   Test how many IPv6 MC could be added into a LPM DB,
*         when source is changed and group stay with same value
* @param[in] totalNumIpMc             - number of ipv6 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] secondOctetIndexToChange - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6McPrefixSrcChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
)
{
    GT_STATUS                       rc=GT_BAD_PARAM;
    GT_U32                          i;
    IN GT_IPV6ADDR                  ipGroup ;
    IN GT_U32                       ipGroupPrefixLen;
    IN GT_IPV6ADDR                  ipSrc;
    IN GT_U32                       ipSrcPrefixLen;
    IN PRV_TGF_IP_LTT_ENTRY_STC     mcRouteLttEntry;
    IN GT_BOOL                      override;
    IN GT_BOOL                      defragmentationEnable;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32                          numOfPrefixesExpected=0;

    if (totalNumIpMc>= 256*256)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "totalNumIpMc should be less than 256*256, totalNumIpUc=%d\n\r",totalNumIpMc);
            return;
        }
    if ((firstOctetIndexToChange>15)||(secondOctetIndexToChange>15))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "index should be less than 15, firstOctetIndexToChange=%d, secondOctetIndexToChange=%d\n\r",
                             firstOctetIndexToChange,secondOctetIndexToChange);
            return;
        }
    if(firstOctetIndexToChange==secondOctetIndexToChange)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "indexes should be different\n\r");
        return;
    }

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
    {
        switch(ramDbCfg.blocksSizeArray[0])
        {
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_SRC_CHANGE_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_SRC_CHANGE_CNS;

                }
                else
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            }
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
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_SRC_CHANGE_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_SRC_CHANGE_CNS;
                }
                else
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            }

            break;
        default:
            /* the test does not support at the moment any other configuration */
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            break;
        }
    }

    /*create a MC prefixes*/
    cpssOsMemSet(&mcRouteLttEntry, '\0', sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
    mcRouteLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    mcRouteLttEntry.numOfPaths = 0;
    mcRouteLttEntry.routeEntryBaseIndex = 0;
    mcRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    mcRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipGroup, '\0', sizeof(GT_IPV6ADDR));
    cpssOsMemSet(&ipSrc, '\0', sizeof(GT_IPV6ADDR));

    ipGroupPrefixLen = prefixLengthGrp;  /*128;*/
    ipSrcPrefixLen = prefixLengthSrc;    /*128; */
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;

    ipGroup.arIP[1] = 0x02;
    ipGroup.arIP[2] = 0x00;
    ipGroup.arIP[14] = 0x01;
    ipGroup.arIP[15] = 0x05;

    ipGroup.arIP[0] = 0xff;


    for (i = 0; i < totalNumIpMc; i ++)
    {
        if(order == GT_FALSE)
        {
            if (i % 256 == 0)
            {
                if(i!=0)
                    (ipSrc.arIP[secondOctetIndexToChange])++;

                ipSrc.arIP[firstOctetIndexToChange] = 0;
            }

            else
                (ipSrc.arIP[firstOctetIndexToChange])++;
        }
        else
        {
             (ipSrc.u32Ip[firstOctetIndexToChange]) = i;
        }


        if (delete==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen);
        }
        else
        {
            rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen,
                                           &mcRouteLttEntry,override,defragmentationEnable);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
             /*  remove remark in case debug is needed
            cpssOsPrintf("[S, G][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         ipSrc.arIP[0]
                         ,ipSrc.arIP[1]
                         ,ipSrc.arIP[2]
                         ,ipSrc.arIP[3]
                         ,ipSrc.arIP[4]
                         ,ipSrc.arIP[5]
                         ,ipSrc.arIP[6]
                         ,ipSrc.arIP[7]
                         ,ipSrc.arIP[8]
                         ,ipSrc.arIP[9]
                         ,ipSrc.arIP[10]
                         ,ipSrc.arIP[11]
                         ,ipSrc.arIP[12]
                         ,ipSrc.arIP[13]
                         ,ipSrc.arIP[14]
                         ,ipSrc.arIP[15]);
                         */
        }
        else
            {
                break;
            }
    }
    if (i!=numOfPrefixesExpected)
    {
         if (delete==GT_TRUE)
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
         else
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    else
    {
        if (delete==GT_TRUE)
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    return;
}



/**
* @internal prvTgfIpLpmIpv6McPrefixGroupChange function
* @endinternal
*
* @brief   Test how many IPv6 MC could be added into a LPM DB,
*         when group is changed and source stay with same value
* @param[in] totalNumIpMc             - number of ipv6 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] firstOctetIndexToChange  - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6McPrefixGroupChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
)
{
    GT_STATUS                       rc = GT_BAD_PARAM;
    GT_U32                          i;
    IN GT_IPV6ADDR                  ipGroup ;
    IN GT_U32                       ipGroupPrefixLen;
    IN GT_IPV6ADDR                  ipSrc;
    IN GT_U32                       ipSrcPrefixLen;
    IN PRV_TGF_IP_LTT_ENTRY_STC     mcRouteLttEntry;
    IN GT_BOOL                      override;
    IN GT_BOOL                      defragmentationEnable;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32                          numOfPrefixesExpected=0;

    if (totalNumIpMc>= 256*256)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "totalNumIpMc should be less than 256*256, totalNumIpUc=%d\n\r",totalNumIpMc);
        return;
    }
    if ((firstOctetIndexToChange>15)||(secondOctetIndexToChange>15))
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "index should be less than 15, firstOctetIndexToChange=%d, secondOctetIndexToChange=%d\n\r",
                         firstOctetIndexToChange,secondOctetIndexToChange);
        return;
    }
    if(firstOctetIndexToChange==secondOctetIndexToChange)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "indexes should be different\n\r");
        return;
    }

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
    {
        switch(ramDbCfg.blocksSizeArray[0])
        {
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_GRP_CHANGE_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_GRP_CHANGE_CNS;

                }
                else
                {
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
            }
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
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_GRP_CHANGE_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_GRP_CHANGE_CNS;
                }
                else
                {
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
            }

            break;
        default:
            /* the test does not support at the moment any other configuration */
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            break;
        }
    }

    /*create a MC prefixes*/
    cpssOsMemSet(&mcRouteLttEntry, '\0', sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
    mcRouteLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    mcRouteLttEntry.numOfPaths = 0;
    mcRouteLttEntry.routeEntryBaseIndex = 0;
    mcRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    mcRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipGroup, '\0', sizeof(GT_IPV6ADDR));
    cpssOsMemSet(&ipSrc, '\0', sizeof(GT_IPV6ADDR));

    ipGroupPrefixLen = prefixLengthGrp;
    ipSrcPrefixLen = prefixLengthSrc;
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;

    ipGroup.arIP[0] = 0xff;

    /* a fixed ipSrc */
    ipSrc.arIP[1] = 0x02;
    ipSrc.arIP[2] = 0x00;
    ipSrc.arIP[14] = 0x01;
    ipSrc.arIP[15] = 0x05;

    for (i = 0; i < totalNumIpMc; i ++)
    {
        if(order == GT_FALSE)
        {
            if(i%256 == 0)
            {
                if(i!=0)
                    (ipGroup.arIP[secondOctetIndexToChange])++;

                ipGroup.arIP[firstOctetIndexToChange] = 0;
            }

            else
                (ipGroup.arIP[firstOctetIndexToChange])++;
        }
        else
        {
             (ipGroup.u32Ip[firstOctetIndexToChange]) =i;
        }

        if (delete==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen);
        }
        else
        {
            rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen,
                                           &mcRouteLttEntry,override,defragmentationEnable);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
             /*  remove remark in case debug is needed
            cpssOsPrintf("[S, G][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         ipGroup.arIP[0]
                         ,ipGroup.arIP[1]
                         ,ipGroup.arIP[2]
                         ,ipGroup.arIP[3]
                         ,ipGroup.arIP[4]
                         ,ipGroup.arIP[5]
                         ,ipGroup.arIP[6]
                         ,ipGroup.arIP[7]
                         ,ipGroup.arIP[8]
                         ,ipGroup.arIP[9]
                         ,ipGroup.arIP[10]
                         ,ipGroup.arIP[11]
                         ,ipGroup.arIP[12]
                         ,ipGroup.arIP[13]
                         ,ipGroup.arIP[14]
                         ,ipGroup.arIP[15]);
                         */
        }
        else
        {
             break;
        }
    }
    if (i!=numOfPrefixesExpected)
    {
        if (delete==GT_TRUE)
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    else
    {
        if (delete==GT_TRUE)
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    return;
}


/**
* @internal prvTgfIpLpmIpv6UcPrefix128AddDelMix function
* @endinternal
*
* @brief   Test how many IPv6 UC could be added into a LPM DB,
*         when doing scenario of Add and Delete prefixes in a mixed manner
* @param[in] totalNumIpUc             - number of ipv6 Uc entries to add
* @param[in] ipFirstByteValue         - value for the first octet
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] mixNum                   - the index from were we want to mix the add and delete operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcPrefix128AddDelMix
(
    GT_U32  totalNumIpUc,
    GT_U8   ipFirstByteValue,
    GT_U32  firstOctetIndexToChange,
    GT_U32  mixNum,
    GT_BOOL order
)
{
    GT_U32      entryNumNotMix = 0;
    GT_STATUS   rc = GT_BAD_PARAM;
    GT_U32      i = 0;

    GT_IPV6ADDR                             unIpv6Addr;
    GT_IPV6ADDR                             unIpv6AddrDel;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_U32 numOfPrefixesDeleted=0;
    GT_U32 numOfPrefixesAdded=0;

    entryNumNotMix = totalNumIpUc - mixNum;

    if ((totalNumIpUc+entryNumNotMix)>= 256*256)

    {
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "totalNumIpUc-mixNum should be less than (256*256)/2, totalNumIpUc=%d,mixNum=%d\n\r",totalNumIpUc,mixNum);
       return;
    }
    if (firstOctetIndexToChange>= 15)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "index should be less than 15, firstOctetIndexToChange=%d\n\r",firstOctetIndexToChange);
        return;
    }
    if (mixNum >= totalNumIpUc)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "mixNum should be less than totalNumIpUc\n\r");
        return;
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }

    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));
    unIpv6Addr.arIP[0] = ipFirstByteValue;
    cpssOsMemSet(&unIpv6AddrDel, '\0', sizeof(GT_IPV6ADDR));
    unIpv6AddrDel.arIP[0] = ipFirstByteValue;
    ipPrefixLen = 128;
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;


    for (i = 0; i < totalNumIpUc + entryNumNotMix; i ++)
    {
        if(order == 0)
        {
           if(i <= totalNumIpUc)
           {
                if(i%256 == 0)
                {
                    if(i!=0)
                    (unIpv6Addr.arIP[firstOctetIndexToChange])++;

                    unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
                }

                else
                (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;
           }

           if(i>=entryNumNotMix)
           {
                if((i-entryNumNotMix)%256 == 0)
                {
                    if(i!=entryNumNotMix)
                        (unIpv6AddrDel.arIP[firstOctetIndexToChange])++;

                    unIpv6AddrDel.arIP[firstOctetIndexToChange+1] = 0;
                }

                else
                    (unIpv6AddrDel.arIP[firstOctetIndexToChange+1])++;
           }

        }
        else
        {
            if(i <= totalNumIpUc)
                (unIpv6Addr.u32Ip[firstOctetIndexToChange]) =i+1;

            if(i>entryNumNotMix)
                (unIpv6AddrDel.u32Ip[firstOctetIndexToChange]) =i+1;
        }


        if (i<entryNumNotMix)
        {

            rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6Addr,ipPrefixLen, &nextHopInfo, override, defragmentationEnable);

            /*  remove remark in case debug is needed
            cpssOsPrintf("[ipv6 unicast added  ][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         unIpv6Addr.arIP[0]
                         ,unIpv6Addr.arIP[1]
                         ,unIpv6Addr.arIP[2]
                         ,unIpv6Addr.arIP[3]
                         ,unIpv6Addr.arIP[4]
                         ,unIpv6Addr.arIP[5]
                         ,unIpv6Addr.arIP[6]
                         ,unIpv6Addr.arIP[7]
                         ,unIpv6Addr.arIP[8]
                         ,unIpv6Addr.arIP[9]
                         ,unIpv6Addr.arIP[10]
                         ,unIpv6Addr.arIP[11]
                         ,unIpv6Addr.arIP[12]
                         ,unIpv6Addr.arIP[13]
                         ,unIpv6Addr.arIP[14]
                         ,unIpv6Addr.arIP[15]); */

            if(rc!=GT_OK)
            {
                break;
            }
            else
            {
                numOfPrefixesAdded++;
            }
        }
        else if (i>=totalNumIpUc)
        {
            rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, unIpv6AddrDel,ipPrefixLen);

            /*  remove remark in case debug is needed
            cpssOsPrintf("[ipv6 unicast deleted][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         unIpv6AddrDel.arIP[0]
                         ,unIpv6AddrDel.arIP[1]
                         ,unIpv6AddrDel.arIP[2]
                         ,unIpv6AddrDel.arIP[3]
                         ,unIpv6AddrDel.arIP[4]
                         ,unIpv6AddrDel.arIP[5]
                         ,unIpv6AddrDel.arIP[6]
                         ,unIpv6AddrDel.arIP[7]
                         ,unIpv6AddrDel.arIP[8]
                         ,unIpv6AddrDel.arIP[9]
                         ,unIpv6AddrDel.arIP[10]
                         ,unIpv6AddrDel.arIP[11]
                         ,unIpv6AddrDel.arIP[12]
                         ,unIpv6AddrDel.arIP[13]
                         ,unIpv6AddrDel.arIP[14]
                         ,unIpv6AddrDel.arIP[15]);*/
            if (rc != GT_OK)
            {
                break;
            }
            else
            {
                numOfPrefixesDeleted++;
            }
        }
        else
        {
             /* Mix add and delete */
             rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6Addr,ipPrefixLen, &nextHopInfo, override, defragmentationEnable);
             /*  remove remark in case debug is needed
             cpssOsPrintf("[ipv6 unicast added  ][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                             i,
                             unIpv6Addr.arIP[0]
                             ,unIpv6Addr.arIP[1]
                             ,unIpv6Addr.arIP[2]
                             ,unIpv6Addr.arIP[3]
                             ,unIpv6Addr.arIP[4]
                             ,unIpv6Addr.arIP[5]
                             ,unIpv6Addr.arIP[6]
                             ,unIpv6Addr.arIP[7]
                             ,unIpv6Addr.arIP[8]
                             ,unIpv6Addr.arIP[9]
                             ,unIpv6Addr.arIP[10]
                             ,unIpv6Addr.arIP[11]
                             ,unIpv6Addr.arIP[12]
                             ,unIpv6Addr.arIP[13]
                             ,unIpv6Addr.arIP[14]
                             ,unIpv6Addr.arIP[15]);*/

            if (rc != GT_OK)
            {
                break;
            }
            else
            {
                numOfPrefixesAdded++;
            }

            rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, unIpv6AddrDel,ipPrefixLen);
            /*  remove remark in case debug is needed
            cpssOsPrintf("[ipv6 unicast deleted][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                     i,
                     unIpv6AddrDel.arIP[0]
                     ,unIpv6AddrDel.arIP[1]
                     ,unIpv6AddrDel.arIP[2]
                     ,unIpv6AddrDel.arIP[3]
                     ,unIpv6AddrDel.arIP[4]
                     ,unIpv6AddrDel.arIP[5]
                     ,unIpv6AddrDel.arIP[6]
                     ,unIpv6AddrDel.arIP[7]
                     ,unIpv6AddrDel.arIP[8]
                     ,unIpv6AddrDel.arIP[9]
                     ,unIpv6AddrDel.arIP[10]
                     ,unIpv6AddrDel.arIP[11]
                     ,unIpv6AddrDel.arIP[12]
                     ,unIpv6AddrDel.arIP[13]
                     ,unIpv6AddrDel.arIP[14]
                     ,unIpv6AddrDel.arIP[15]);*/
            if (rc != GT_OK)
            {
                break;
            }
            else
            {
                numOfPrefixesDeleted++;
            }
        }
    }
   if (numOfPrefixesAdded!=totalNumIpUc)
    {
       UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding/deleting the amount wanted. added(%d/%d),  deleted(%d/%d).\n",numOfPrefixesAdded,totalNumIpUc,numOfPrefixesDeleted,totalNumIpUc);
    }
    else
    {
              cpssOsPrintf("\n We succeed in adding/deleting the amount wanted. added(%d/%d),deleted(%d/%d).\n",numOfPrefixesAdded,totalNumIpUc,numOfPrefixesDeleted,totalNumIpUc);
    }
    return;
}


/**
* @internal prvTgfIpLpmIpv4McPrefixSrcChange function
* @endinternal
*
* @brief   Test how many IPv4 MC could be added into a LPM DB,
*         when source is changed and group stay with same value
* @param[in] totalNumIpMc             - number of ipv4 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] secondOctetIndexToChange - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4McPrefixSrcChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
)
{
    GT_STATUS                       rc=GT_BAD_PARAM;
    GT_U32                          i;
    IN GT_IPADDR                    ipGroup ;
    IN GT_U32                       ipGroupPrefixLen;
    IN GT_IPADDR                    ipSrc;
    IN GT_U32                       ipSrcPrefixLen;
    IN PRV_TGF_IP_LTT_ENTRY_STC     mcRouteLttEntry;
    IN GT_BOOL                      override;
    IN GT_BOOL                      defragmentationEnable;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32                          numOfPrefixesExpected=0;

    if (totalNumIpMc>= 256*256)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "totalNumIpMc should be less than 256*256, totalNumIpUc=%d\n\r",totalNumIpMc);
            return;
        }
    if ((firstOctetIndexToChange>3)||(secondOctetIndexToChange>3))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "index should be less than 3, firstOctetIndexToChange=%d, secondOctetIndexToChange=%d\n\r",
                             firstOctetIndexToChange,secondOctetIndexToChange);
            return;
        }
    if(firstOctetIndexToChange==secondOctetIndexToChange)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "indexes should be different\n\r");
        return;
    }

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
    {
        switch(ramDbCfg.blocksSizeArray[0])
        {
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_SRC_CHANGE_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_SRC_CHANGE_CNS;

                }
                else
                {
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
            }
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
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_SRC_CHANGE_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_SRC_CHANGE_CNS;
                }
                else
                {
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
            }

            break;
        default:
            /* the test does not support at the moment any other configuration */
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            break;
        }
    }

    /*create a MC prefixes*/
    cpssOsMemSet(&mcRouteLttEntry, '\0', sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
    mcRouteLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    mcRouteLttEntry.numOfPaths = 0;
    mcRouteLttEntry.routeEntryBaseIndex = 0;
    mcRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    mcRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipGroup, '\0', sizeof(GT_IPADDR));
    cpssOsMemSet(&ipSrc, '\0', sizeof(GT_IPADDR));

    ipGroupPrefixLen = prefixLengthGrp;
    ipSrcPrefixLen = prefixLengthSrc;
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;

    ipGroup.arIP[0] = 224;
    ipGroup.arIP[1] = 0;
    ipGroup.arIP[2] = 0;
    ipGroup.arIP[3] = 0;

    for (i = 0; i < totalNumIpMc; i ++)
    {
        if(order == GT_FALSE)
        {
            if (i % 256 == 0)
            {
                if(i!=0)
                    (ipSrc.arIP[secondOctetIndexToChange])++;

                ipSrc.arIP[firstOctetIndexToChange] = 0;
            }

            else
                (ipSrc.arIP[firstOctetIndexToChange])++;
        }
        else
        {
             ipSrc.u32Ip = i;
        }


        if (delete==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen);
        }
        else
        {
            rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen,
                                           &mcRouteLttEntry,override,defragmentationEnable);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
             /*  remove remark in case debug is needed
            cpssOsPrintf("[S, G][%d] = [%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         ipSrc.arIP[0]
                         ,ipSrc.arIP[1]
                         ,ipSrc.arIP[2]
                         ,ipSrc.arIP[3]);
                         */
        }
        else
            {
                break;
            }
    }
    if (i!=numOfPrefixesExpected)
    {
         if (delete==GT_TRUE)
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
         else
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    else
    {
        if (delete==GT_TRUE)
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    return;
}



/**
* @internal prvTgfIpLpmIpv4McPrefixGroupChange function
* @endinternal
*
* @brief   Test how many IPv4 MC could be added into a LPM DB,
*         when group is changed and source stay with same value
* @param[in] totalNumIpMc             - number of ipv4 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] firstOctetIndexToChange  - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4McPrefixGroupChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
)
{
    GT_STATUS                       rc = GT_BAD_PARAM;
    GT_U32                          i;
    IN GT_IPADDR                    ipGroup ;
    IN GT_U32                       ipGroupPrefixLen;
    IN GT_IPADDR                    ipSrc;
    IN GT_U32                       ipSrcPrefixLen;
    IN PRV_TGF_IP_LTT_ENTRY_STC     mcRouteLttEntry;
    IN GT_BOOL                      override;
    IN GT_BOOL                      defragmentationEnable;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32                          numOfPrefixesExpected=0;

    if (totalNumIpMc>= 256*256)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "totalNumIpMc should be less than 256*256, totalNumIpUc=%d\n\r",totalNumIpMc);
        return;
    }
    if ((firstOctetIndexToChange>3)||(secondOctetIndexToChange>3))
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "index should be less than 3, firstOctetIndexToChange=%d, secondOctetIndexToChange=%d\n\r",
                         firstOctetIndexToChange,secondOctetIndexToChange);
        return;
    }
    if(firstOctetIndexToChange==secondOctetIndexToChange)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "indexes should be different\n\r");
        return;
    }

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
    {
        switch(ramDbCfg.blocksSizeArray[0])
        {
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_GRP_CHANGE_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_GRP_CHANGE_CNS;

                }
                else
                {
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
            }
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
        case (_2K*4):
            /* each block memory has 2K lines (not including the blocks holding PBR)
            multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
            if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_GRP_CHANGE_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_GRP_CHANGE_CNS;
                }
                else
                {
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
            }

            break;
        default:
            /* the test does not support at the moment any other configuration */
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            break;
        }
    }

    /*create a MC prefixes*/
    cpssOsMemSet(&mcRouteLttEntry, '\0', sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
    mcRouteLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    mcRouteLttEntry.numOfPaths = 0;
    mcRouteLttEntry.routeEntryBaseIndex = 0;
    mcRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    mcRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipGroup, '\0', sizeof(GT_IPADDR));
    cpssOsMemSet(&ipSrc, '\0', sizeof(GT_IPADDR));

    ipGroupPrefixLen = prefixLengthGrp;
    ipSrcPrefixLen = prefixLengthSrc;
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;

    ipGroup.arIP[0] = 224;

    /* a fixed ipSrc */
    ipSrc.arIP[0] = 0;
    ipSrc.arIP[1] = 0;
    ipSrc.arIP[2] = 0;
    ipSrc.arIP[3] = 0;

    for (i = 0; i < totalNumIpMc; i ++)
    {
        if(order == GT_FALSE)
        {
            if(i%256 == 0)
            {
                if(i!=0)
                    (ipGroup.arIP[secondOctetIndexToChange])++;

                ipGroup.arIP[firstOctetIndexToChange] = 0;
            }

            else
                (ipGroup.arIP[firstOctetIndexToChange])++;
        }
        else
        {
              ipSrc.u32Ip = i;
        }

        if (delete==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen);
        }
        else
        {
            rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen,
                                           &mcRouteLttEntry,override,defragmentationEnable);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
             /*  remove remark in case debug is needed
            cpssOsPrintf("[S, G][%d] = [%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         ipGroup.arIP[0]
                         ,ipGroup.arIP[1]
                         ,ipGroup.arIP[2]
                         ,ipGroup.arIP[3]);
                         */
        }
        else
        {
             break;
        }
    }
    if (i!=numOfPrefixesExpected)
    {
        if (delete==GT_TRUE)
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    else
    {
        if (delete==GT_TRUE)
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    return;
}

/**
* @internal prvTgfIpLpmIpv6UcOrderAndDisorder function
* @endinternal
*
* @brief   Add some ipv6 Prefixes in order and disorder manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv6 UC entries to add
* @param[in] randomEnable             - GT_TRUE: increment the octets in a disordered manner
*                                      GT_FALSE:increment the octets in a sequentially manner
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcOrderAndDisorder
(
    GT_U32  totalNumIpUc,
    GT_BOOL randomEnable,
    GT_BOOL defragEnable
)
{
    GT_U32                                  *prefixesArrayPtr = NULL;
    GT_U32                                  ulLoop = 0;
    GT_U32                                  ulIndex = 0;
    GT_U32                                  temp = 0;
    GT_IPV6ADDR                             ipv6Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  numberOfAddedPrefixes=0;
    GT_U32                                  minNumberOfPrefixesExpected;
    GT_U32                                  maxNumberOfPrefixesExpected;


    if (randomEnable==GT_FALSE && defragEnable==GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITHOUT defrag \n");
        if (totalNumIpUc<8701)
        {
            minNumberOfPrefixesExpected = totalNumIpUc;
            maxNumberOfPrefixesExpected = totalNumIpUc;
        }
        else
        {
            minNumberOfPrefixesExpected = 8701;
            maxNumberOfPrefixesExpected = 8701;
        }
    }
    else
    {
        if (randomEnable==GT_FALSE && defragEnable==GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITH defrag \n");
            if (totalNumIpUc<9126)
            {
                minNumberOfPrefixesExpected = totalNumIpUc;
                maxNumberOfPrefixesExpected = totalNumIpUc;
            }
            else
            {
                minNumberOfPrefixesExpected = 9126;
                maxNumberOfPrefixesExpected = 9126;
            }
        }
        else
        {
            if (randomEnable==GT_TRUE && defragEnable==GT_FALSE)
            {
                PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITHOUT defrag\n ");
                if (totalNumIpUc<3000)
                {
                    minNumberOfPrefixesExpected = totalNumIpUc;
                    maxNumberOfPrefixesExpected = totalNumIpUc;
                }
                else
                {
                    minNumberOfPrefixesExpected = 3000;
                    maxNumberOfPrefixesExpected = 6000;
                }
            }
            else
            {
                PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITH defrag \n ");
                if (totalNumIpUc<6000)
                {
                    minNumberOfPrefixesExpected = totalNumIpUc;
                    maxNumberOfPrefixesExpected = totalNumIpUc;
                }
                else
                {
                    minNumberOfPrefixesExpected = 6000;
                    maxNumberOfPrefixesExpected = 9126;
                }
            }
        }
    }
    prefixesArrayPtr = cpssOsMalloc(sizeof(GT_U32) * totalNumIpUc);

    /* xx::1 */
    for (ulLoop = 0; ulLoop < totalNumIpUc; ulLoop++)
    {
        prefixesArrayPtr[ulLoop] = ulLoop + 1;
    }

    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    if(randomEnable == GT_TRUE)
    {
        cpssOsSrand(cpssOsTime());
        for (ulLoop = totalNumIpUc; ulLoop > 0; ulLoop--)
        {
            ulIndex = cpssOsRand()%totalNumIpUc;
            temp = prefixesArrayPtr[ulIndex];
            prefixesArrayPtr[ulIndex] = prefixesArrayPtr[ulLoop - 1];
            prefixesArrayPtr[ulLoop - 1] = temp;
        }
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv6Addr, '\0', sizeof(GT_IPV6ADDR));
    ipv6Addr.arIP[1] = 0x3;
    ipPrefixLen = 128;
    override = GT_FALSE;
    defragmentationEnable = defragEnable;


    for (ulLoop = 0; ulLoop < totalNumIpUc; ulLoop++)
    {
        ipv6Addr.arIP[15] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv6Addr.arIP[14] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv6UcPrefixAdd(0, 0, ipv6Addr,ipPrefixLen, &nextHopInfo,
                                       override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }
    if ((numberOfAddedPrefixes>=minNumberOfPrefixesExpected)&&(numberOfAddedPrefixes<=maxNumberOfPrefixesExpected))
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv6UcPrefixAdd OK numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv6UcPrefixAdd FAIL numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv6UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    cpssOsFree(prefixesArrayPtr);

    return;
}

/**
* @internal prvTgfIpLpmIpv4UcOrderAndDisorder function
* @endinternal
*
* @brief   Add some ipv4 Prefixes in order and disorder manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
* @param[in] randomEnable             - GT_TRUE: increment the octets in a disordered manner
*                                      GT_FALSE:increment the octets in a sequentially manner
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcOrderAndDisorder
(
    GT_U32  totalNumIpUc,
    GT_BOOL randomEnable,
    GT_BOOL defragEnable
)
{
    GT_U32                                  *prefixesArrayPtr = NULL;
    GT_U32                                  ulLoop = 0;
    GT_U32                                  ulIndex = 0;
    GT_U32                                  temp = 0;
    GT_IPADDR                               ipv4Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  numberOfAddedPrefixes=0;
    GT_U32                                  minNumberOfPrefixesExpected;
    GT_U32                                  maxNumberOfPrefixesExpected;

    if (randomEnable==GT_FALSE && defragEnable==GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITHOUT defrag \n");
        if (totalNumIpUc<30205)
        {
            minNumberOfPrefixesExpected = totalNumIpUc;
            maxNumberOfPrefixesExpected = totalNumIpUc;
        }
        else
        {
            minNumberOfPrefixesExpected = 30205;
            maxNumberOfPrefixesExpected = 30205;
        }
    }
    else
    {
        if (randomEnable==GT_FALSE && defragEnable==GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITH defrag \n");
            if (totalNumIpUc<30630)
            {
                minNumberOfPrefixesExpected = totalNumIpUc;
                maxNumberOfPrefixesExpected = totalNumIpUc;
            }
            else
            {
                minNumberOfPrefixesExpected = 30630;
                maxNumberOfPrefixesExpected = 30630;
            }
        }
        else
        {
            if (randomEnable==GT_TRUE && defragEnable==GT_FALSE)
            {
                PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITHOUT defrag\n ");
                if (totalNumIpUc<15000)
                {
                    minNumberOfPrefixesExpected = totalNumIpUc;
                    maxNumberOfPrefixesExpected = totalNumIpUc;
                }
                else
                {
                    minNumberOfPrefixesExpected = 15000;
                    maxNumberOfPrefixesExpected = 20000;
                }
            }
            else
            {
                PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITH defrag \n ");
                if (totalNumIpUc<20000)
                {
                    minNumberOfPrefixesExpected = totalNumIpUc;
                    maxNumberOfPrefixesExpected = totalNumIpUc;
                }
                else
                {
                    minNumberOfPrefixesExpected = 20000;
                    maxNumberOfPrefixesExpected = 30630;
                }
            }
        }
    }
    prefixesArrayPtr = cpssOsMalloc(sizeof(GT_U32) * totalNumIpUc);

    /* xx::1 */
    for (ulLoop = 0; ulLoop < totalNumIpUc; ulLoop++)
    {
        prefixesArrayPtr[ulLoop] = ulLoop + 1;
    }

    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    if(randomEnable == GT_TRUE)
    {
        cpssOsSrand(cpssOsTime());
        for (ulLoop = totalNumIpUc; ulLoop > 0; ulLoop--)
        {
            ulIndex = cpssOsRand()%totalNumIpUc;
            temp = prefixesArrayPtr[ulIndex];
            prefixesArrayPtr[ulIndex] = prefixesArrayPtr[ulLoop - 1];
            prefixesArrayPtr[ulLoop - 1] = temp;
        }
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv4Addr, '\0', sizeof(GT_IPADDR));
    ipv4Addr.arIP[3] = 0x1;
    ipPrefixLen = 32;
    override = GT_FALSE;
    defragmentationEnable = defragEnable;

    for (ulLoop = 0; ulLoop < totalNumIpUc; ulLoop++)
    {
        ipv4Addr.arIP[3] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[2] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                                       override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }
    if ((numberOfAddedPrefixes>=minNumberOfPrefixesExpected)&&(numberOfAddedPrefixes<=maxNumberOfPrefixesExpected))
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    cpssOsFree(prefixesArrayPtr);

    return;
}

/**
* @internal prvTgfIpLpmIpv4UcDisorder function
* @endinternal
*
* @brief   Add some ipv4 Prefixes in disorder manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
*                                      randomEnable      - GT_TRUE: increment the octets in a disordered manner
*                                      GT_FALSE:increment the octets in a sequentially manner
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcDisorder
(
    GT_U32  totalNumIpUc,
    GT_BOOL defragEnable
)
{
    GT_STATUS                               rc=GT_OK;
    GT_IPADDR                               ipv4Addr;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_U32                                  numberOfAddedPrefixes=0;
    GT_U32                                  minNumberOfPrefixesExpected;
    GT_U32                                  maxNumberOfPrefixesExpected;

    if (defragEnable==GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITHOUT defrag\n ");
        if (totalNumIpUc<3200)
        {
            minNumberOfPrefixesExpected = totalNumIpUc;
            maxNumberOfPrefixesExpected = totalNumIpUc;
        }
        else
        {
            minNumberOfPrefixesExpected = 3200;
            maxNumberOfPrefixesExpected = 3500;
        }
    }
    else
    {
        PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITH defrag \n ");
        if (totalNumIpUc<3500)
        {
            minNumberOfPrefixesExpected = totalNumIpUc;
            maxNumberOfPrefixesExpected = totalNumIpUc;
        }
        else
        {
            minNumberOfPrefixesExpected = 3500;
            maxNumberOfPrefixesExpected = 3700;
        }
    }

    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv4Addr, '\0', sizeof(GT_IPADDR));
    ipv4Addr.arIP[3] = 0x1;
    defragmentationEnable = defragEnable;

    rc = prvTgfIpLpmDbgIpv4UcPrefixAddManyRandom(prvTgfLpmDBId,
                                               0,
                                               ipv4Addr,
                                               8,
                                               totalNumIpUc,
                                               GT_TRUE,
                                               defragmentationEnable,
                                               &numberOfAddedPrefixes);

    if ((numberOfAddedPrefixes>=minNumberOfPrefixesExpected)&&(numberOfAddedPrefixes<=maxNumberOfPrefixesExpected))
    {
        cpssOsPrintf("\n cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom OK numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    else
    {
        cpssOsPrintf("\n cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom FAIL numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    return;
}


/**
* @internal prvTgfIpLpmIpv6UcAddSpecificPrefix function
* @endinternal
*
* @brief   Add specific ipv6 Uc entry with prefix length = 64
*
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcAddSpecificPrefix
(
    GT_BOOL defragEnable
)
{
    GT_IPV6ADDR                             ipv6Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_STATUS                               rc = GT_OK;

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv6Addr, '\0', sizeof(GT_IPV6ADDR));
    ipPrefixLen = 64;
    override = GT_FALSE;
    defragmentationEnable = defragEnable;
    ipv6Addr.arIP[0] = 0x22;
    ipv6Addr.arIP[1] = 0x22;
    ipv6Addr.arIP[2] = 0x22;
    ipv6Addr.arIP[3] = 0x22;
    ipv6Addr.arIP[4] = 0x22;
    ipv6Addr.arIP[5] = 0x22;
    ipv6Addr.arIP[6] = 0x22;
    ipv6Addr.arIP[7] = 0x22;
    ipv6Addr.arIP[8] = 0x22;
    ipv6Addr.arIP[9] = 0x22;
    ipv6Addr.arIP[10] = 0x22;
    ipv6Addr.arIP[11] = 0x22;
    ipv6Addr.arIP[12] = 0x22;
    ipv6Addr.arIP[13] = 0x22;
    ipv6Addr.arIP[14] = 0x22;
    ipv6Addr.arIP[15] = 0x22;

    rc = prvTgfIpLpmIpv6UcPrefixAdd(0, 0, ipv6Addr,ipPrefixLen, &nextHopInfo,
                                    override, defragmentationEnable);

    if (defragmentationEnable==GT_FALSE)
    {
        if (rc==GT_OUT_OF_PP_MEM)
        {
            /* this is the expected value */
            cpssOsPrintf("\n prvTgfIpLpmIpv6UcAddSpecificPrefix return GT_OUT_OF_PP_MEM when defragmentationEnable=%d\n\n",defragmentationEnable);
            rc = GT_OK;
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_PP_MEM, rc, "prvTgfIpLpmIpv6UcAddSpecificPrefix,defragmentationEnable: %d", defragmentationEnable);
        }
    }
    else
    {
        if (rc!=GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcAddSpecificPrefix,defragmentationEnable: %d", defragmentationEnable);
        }
        else
        {
            cpssOsPrintf("\n prvTgfIpLpmIpv6UcAddSpecificPrefix return GT_OK when defragmentationEnable=%d\n\n",defragmentationEnable);
        }
    }
    return;
}

/**
* @internal prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop function
* @endinternal
*
* @brief   Add some Predefined and sequentially ipv4 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
* @param[in] ipv4Addr                 - ip address to start the loop from
* @param[in] ipPrefixLen              - prefix length
* @param[in] nextHopInfoPtr           - (pointer to) the next hop information
* @param[in] startLoop                - beginning of loop index
* @param[in] endLoop                  - end of loop index
* @param[in] numOfExpectedPrefixes    - number of prefixes expected to be added/deleted
* @param[in] firstOctetToChange       - first octet we want to change in the ip address
* @param[in] secondOctetToChange      - second octet we want to change in the ip address
* @param[in] addFlag                  - GT_TRUE: add prefixes
*                                      GT_FALSE: delete prefixes
* @param[in] defragEnable             - GT_TRUE: use defrag mechanism when adding the prefix
*                                      GT_FALSE: do not use defrag mechanism when adding the prefix
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop
(
    GT_IPADDR                               ipv4Addr,
    GT_U32                                  ipPrefixLen,
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *nextHopInfoPtr,
    GT_U32                                  startLoop,
    GT_U32                                  endLoop,
    GT_U32                                  numOfExpectedPrefixes,
    GT_U32                                  firstOctetToChange,
    GT_U32                                  secondOctetToChange,
    GT_BOOL                                 addFlag,
    GT_BOOL                                 defragEnable
)
{
    GT_U32                                  *prefixesArrayPtr = NULL;
    GT_U32                                  ulLoop = 0;
    GT_BOOL                                 override;
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  numberOfAddedPrefixes=0;

    prefixesArrayPtr = cpssOsMalloc(sizeof(GT_U32) * endLoop);

    for (ulLoop = 0; ulLoop < endLoop; ulLoop++)
    {
        prefixesArrayPtr[ulLoop] = ulLoop + 1;
    }

    override = GT_FALSE;

    /* add the prefixes */
    for (ulLoop = startLoop; ulLoop < endLoop; ulLoop++)
    {
        ipv4Addr.arIP[firstOctetToChange] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[secondOctetToChange] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        if (addFlag==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipv4Addr, ipPrefixLen, nextHopInfoPtr, override, defragEnable);
        }
        else
        {
            rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipv4Addr, ipPrefixLen);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }
    if (numberOfAddedPrefixes>=numOfExpectedPrefixes)
    {
        if (addFlag==GT_TRUE)
        {
            cpssOsPrintf("[last ipv4 unicast added] = [%d]:[%d]:[%d]:[%d] numberOfAddedPrefixes=%d,\n\r",
                     ipv4Addr.arIP[0],ipv4Addr.arIP[1],ipv4Addr.arIP[2],ipv4Addr.arIP[3],numberOfAddedPrefixes);
        }
        else
        {
            cpssOsPrintf("[last ipv4 unicast deleted] = [%d]:[%d]:[%d]:[%d] numberOfDeletedPrefixes=%d,\n\r",
                     ipv4Addr.arIP[0],ipv4Addr.arIP[1],ipv4Addr.arIP[2],ipv4Addr.arIP[3],numberOfAddedPrefixes);
        }
    }
    else
    {
        if (addFlag==GT_TRUE)
        {
            cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
        }
        else
        {
            cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixDel FAIL numberOfDeletedPrefixes= %d \n",numberOfAddedPrefixes);
        }
        return GT_FAIL;
    }

    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage after last ipv4Addr=%d:%d:%d:%d \n",
                 ipv4Addr.arIP[0],ipv4Addr.arIP[1],ipv4Addr.arIP[2],ipv4Addr.arIP[3]);
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);

    cpssOsFree(prefixesArrayPtr);

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4UcCheckMergeBanks function
* @endinternal
*
* @brief   Add some Predefined and sequentially ipv4 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
*         test the Merge bank mechanism
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcCheckMergeBanks
(
    GT_U32  totalNumIpUc
)
{
    GT_IPADDR                               ipv4Addr;
    GT_IPV6ADDR                             ipv6Addr;
    GT_U8                                   i=0;
    GT_U32                                  ipPrefixLen=32;
    GT_U32                                  ipv6PrefixLen=128;
    GT_BOOL                                 override;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_STATUS                               rc = GT_OK;

    GT_U32                                  startLoop;
    GT_U32                                  endLoop;
    GT_U32                                  firstOctetToChange;
    GT_U32                                  secondOctetToChange;
    GT_BOOL                                 addFlag;
    GT_BOOL                                 defragEnable;
    GT_U32                                  numOfExpectedPrefixes;

    /* IPV4 */

    PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITHOUT defrag \n");

    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv6Addr, '\0', sizeof(GT_IPV6ADDR));
    ipv6Addr.arIP[0]=0x22;
    ipv6Addr.arIP[1]=0x22;
    ipv6Addr.arIP[14]=0x44;
    ipv6Addr.arIP[15]=0x44;

    override = GT_FALSE;
    defragEnable = GT_FALSE;

    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, ipv6Addr,ipv6PrefixLen, &nextHopInfo, override, defragEnable);
    if (rc == GT_OK)
    {
        cpssOsPrintf("[ipv6 unicast added] = [%d]:[%d]:[%d]:[%d]:"
                     "[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:"
                     "[%d]:[%d]:[%d]:[%d]/%d,\n\r",
                 ipv6Addr.arIP[0],
                 ipv6Addr.arIP[1],
                 ipv6Addr.arIP[2],
                 ipv6Addr.arIP[3],
                 ipv6Addr.arIP[4],
                 ipv6Addr.arIP[5],
                 ipv6Addr.arIP[6],
                 ipv6Addr.arIP[7],
                 ipv6Addr.arIP[8],
                 ipv6Addr.arIP[9],
                 ipv6Addr.arIP[10],
                 ipv6Addr.arIP[11],
                 ipv6Addr.arIP[12],
                 ipv6Addr.arIP[13],
                 ipv6Addr.arIP[14],
                 ipv6Addr.arIP[15],
                 ipv6PrefixLen);
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv6UcPrefixAdd FAIL \n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }

    cpssOsMemSet(&ipv4Addr, '\0', sizeof(GT_IPADDR));
    ipv4Addr.arIP[0] = 3;
    ipv4Addr.arIP[1] = 3;
    ipv4Addr.arIP[2] = 3;
    ipv4Addr.arIP[3] = 3;
    ipPrefixLen = 32;

    /* add the first prefix 3.3.3.3 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipv4Addr,ipPrefixLen, &nextHopInfo, override, defragEnable);
    if (rc == GT_OK)
    {
        cpssOsPrintf("[ipv4 unicast added] = [%d]:[%d]:[%d]:[%d]/%d,\n\r",
                 ipv4Addr.arIP[0],
                 ipv4Addr.arIP[1],
                 ipv4Addr.arIP[2],
                 ipv4Addr.arIP[3],
                 ipPrefixLen);
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcPrefixAdd FAIL 3.3.3.3 was not added\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }

    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 3.3.3.3 ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    /* we want to add prefixes for all banks in octet 3 in such
       a way that we will be able to merge between the banks
       we start from bank 4 to 18, each loop i add  prefixes to bank i and i+1
       bank 0 is mapped to octet 0
       bank 1 is mapped to octet 1
       bank 2 is mapped to octet 2
       bank 3 is mapped to octet 3*/
    for (i=4;i<19;i++)
    {
        ipv4Addr.arIP[0] = 0;
        ipv4Addr.arIP[1] = 0;
        ipv4Addr.arIP[2] = 0;
        ipv4Addr.arIP[3] = 1;
        ipPrefixLen = 32;
        startLoop=(i-4)*1500;
        endLoop=startLoop+1500;
        numOfExpectedPrefixes=1500;
        firstOctetToChange=3;
        secondOctetToChange=2;
        addFlag=GT_TRUE;
        defragEnable=GT_FALSE;
         /* add 0.0.0.1 - 0.0.x.y*/
        rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                        startLoop,endLoop,numOfExpectedPrefixes,
                                                        firstOctetToChange,secondOctetToChange,
                                                        addFlag,defragEnable);

        if (rc==GT_OK)
        {
            cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks loop_%d OK\n",i);
        }
        else
        {
            cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL on loop_%d \n",i);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
        }


        ipv4Addr.arIP[0] = i;
        ipv4Addr.arIP[1] = i;
        ipv4Addr.arIP[2] = i;
        ipv4Addr.arIP[3] = i;
        ipPrefixLen = 32;
        startLoop=0;
        endLoop=460;
        numOfExpectedPrefixes=460;
        /* add 4.4.4.4 - 4.4.x.y or 5.5.5.5 -5.5.x.y or i.i.i.i - i.i.x.y*/
           rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                        startLoop,endLoop,numOfExpectedPrefixes,
                                                        firstOctetToChange,secondOctetToChange,
                                                        addFlag,defragEnable);
        if (rc==GT_OK)
        {
            cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks second loop_%d OK\n",i);
        }
        else
        {
            cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL on second loop_%d\n",i);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
        }

    }

    ipv4Addr.arIP[0] = 0;
    ipv4Addr.arIP[1] = 0;
    ipv4Addr.arIP[2] = 0;
    ipv4Addr.arIP[3] = 1;
    ipPrefixLen = 32;
    startLoop=(i-4)*1500;
    endLoop=totalNumIpUc;
    numOfExpectedPrefixes=466;
    defragEnable=GT_TRUE;
    /* add 0.0.0.1 - 0.0.x.y*/
    rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                    startLoop,endLoop,numOfExpectedPrefixes,
                                                    firstOctetToChange,secondOctetToChange,
                                                    addFlag,defragEnable);
    if (rc==GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks endLoop=totalNumIpUc OK\n");
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL on endLoop=totalNumIpUc \n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }

    /* free some of the prefixes */
    ipv4Addr.arIP[0] = 0;
    ipv4Addr.arIP[1] = 0;
    ipv4Addr.arIP[2] = 0;
    ipv4Addr.arIP[3] = 1;
    ipPrefixLen = 32;
    startLoop=0;
    endLoop=(i-4)*1500+466;
    numOfExpectedPrefixes=(i-4)*1500+466;
    addFlag = GT_FALSE;
    /* add 0.0.0.1 - 0.0.x.y*/
    rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                    startLoop,endLoop,numOfExpectedPrefixes,
                                                    firstOctetToChange,secondOctetToChange,
                                                    addFlag,defragEnable);
    if (rc==GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks endLoop=(i-4)*1500+500 OK\n");
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL on endLoop=(i-4)*1500+500 \n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }

    /* try to add prefixes with prefixLen/16
       if defrag=false we should get a fail
       if defrag=true we should have shrink+merge and succeed */
    ipv4Addr.arIP[0] = 20;
    ipv4Addr.arIP[1] = 20;
    ipv4Addr.arIP[2] = 12;
    ipv4Addr.arIP[3] = 13;
    ipPrefixLen = 16;
    startLoop=0;
    endLoop=3000;
    numOfExpectedPrefixes=1533;
    firstOctetToChange = 1;
    secondOctetToChange = 0;
    addFlag = GT_TRUE;
    defragEnable=GT_FALSE;
    /* add 20.20.12.13 - x.y/16*/
    rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                    startLoop,endLoop,numOfExpectedPrefixes,
                                                    firstOctetToChange,secondOctetToChange
                                                    ,addFlag,defragEnable);
    if (rc==GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks prefixLen/16 defrag=false OK\n");
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL on prefixLen/16 defrag=false  \n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }

    cpssDxChIpLpmDbgHwOctetPerProtocolPrint(0);
    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


     /* after we get a fail that there is no more space in memory
       we try to add some more prefixes with defragEnable=GT_TRUE
       we can see that now we can add mutch more */
    startLoop=1533;
    endLoop=40000;
    numOfExpectedPrefixes=21926;
    addFlag = GT_TRUE;
    defragEnable=GT_TRUE;

   /* if we only use simple defrag we can add 381 -
       this was checked manualy by skipping the merge part in the code
       if we use defrag + merge we can add 21926 meaning the merge
       added 21545 more then simple shrink */

    /* add 7.7.12.13 - x.y/16*/
    rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                    startLoop,endLoop,numOfExpectedPrefixes,
                                                    firstOctetToChange,secondOctetToChange,
                                                    addFlag,defragEnable);
    if (rc==GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks prefixLen/16 defrag=true OK\n");
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL prefixLen/16 defrag=true \n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }
    cpssDxChIpLpmDbgHwOctetPerProtocolPrint(0);
    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);
    return;
}
/**
* @internal prvTgfIpLpmIpv4UcOrderAndPredefined function
* @endinternal
*
* @brief   Add some Predefined and sequentially ipv4 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
* @param[in] flushFlag                - GT_TRUE: flush all entries before the test
*                                      GT_FALSE: do not flush all entries before the test
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcOrderAndPredefined
(
    GT_U32  totalNumIpUc,
    GT_BOOL flushFlag
)
{
    GT_U32                                  *prefixesArrayPtr = NULL;
    GT_U32                                  ulLoop = 0;
    GT_IPADDR                               ipv4Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  numberOfAddedPrefixes=0;
    GT_U32                                  numberOfPrefixesExpected;

    GT_IPADDR                               unIpv4AddrArray[4];
    GT_U32                                  ipv4PrefixLenArray[4];

    GT_IPADDR                               lastIpAddrAdded;
    GT_U32                                  numOfPrefixesAdded;

    /* IPV4 */

    cpssOsMemSet(&unIpv4AddrArray, '\0', sizeof(unIpv4AddrArray));
    /* first prefix to be added */
    unIpv4AddrArray[0].arIP[0] = 1;
    unIpv4AddrArray[0].arIP[1] = 1;
    unIpv4AddrArray[0].arIP[2] = 1;
    unIpv4AddrArray[0].arIP[3] = 3;
    ipv4PrefixLenArray[0]=32;

    /* second prefix to be added after first bank is full
       and before we mapped a new bank to the octet */
    unIpv4AddrArray[1].arIP[0] = 4;
    unIpv4AddrArray[1].arIP[1] = 4;
    unIpv4AddrArray[1].arIP[2] = 4;
    unIpv4AddrArray[1].arIP[3] = 4;
    ipv4PrefixLenArray[1]=32;

    /* third prefix to be added after second bank is full
       and before we mapped a new bank to the octet */
    unIpv4AddrArray[2].arIP[0] = 5;
    unIpv4AddrArray[2].arIP[1] = 5;
    unIpv4AddrArray[2].arIP[2] = 5;
    unIpv4AddrArray[2].arIP[3] = 5;
    ipv4PrefixLenArray[2]=32;


    PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITHOUT defrag \n");
    if (totalNumIpUc<28925)
    {
        numberOfPrefixesExpected = totalNumIpUc;
    }
    else
    {
        numberOfPrefixesExpected = 28925;
    }

    prefixesArrayPtr = cpssOsMalloc(sizeof(GT_U32) * totalNumIpUc);

    /* xx::1 */
    for (ulLoop = 0; ulLoop < totalNumIpUc; ulLoop++)
    {
        prefixesArrayPtr[ulLoop] = ulLoop + 1;
    }

    if (flushFlag==GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
        cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
        cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
        cpssDxChIpLpmIpv4McEntriesFlush(0,0);
        cpssDxChIpLpmIpv6McEntriesFlush(0,0);
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv4Addr, '\0', sizeof(GT_IPADDR));
    ipv4Addr.arIP[3] = 0x1;
    ipPrefixLen = 32;
    override = GT_FALSE;
    defragmentationEnable = GT_FALSE;

    /* add the first prefix 1.1.1.3 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, unIpv4AddrArray[0],ipv4PrefixLenArray[0], &nextHopInfo, override,defragmentationEnable);
    if ((rc == GT_OK)||(rc==GT_ALREADY_EXIST))
    {
        cpssOsPrintf("[ipv4 unicast added] = [%d]:[%d]:[%d]:[%d]/%d,\n\r",
                  unIpv4AddrArray[0].arIP[0]
                 ,unIpv4AddrArray[0].arIP[1]
                 ,unIpv4AddrArray[0].arIP[2]
                 ,unIpv4AddrArray[0].arIP[3]
                 ,ipv4PrefixLenArray[0]);

    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcPrefixAdd FAIL 1.1.1.3 was not added\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }


    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 1.1.1.3 ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    for (ulLoop = 0; ulLoop < 1532; ulLoop++)
    {
        ipv4Addr.arIP[3] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[2] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                                       override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }
    if (numberOfAddedPrefixes==1532)
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }


    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 4.4.4.4 BEFORE ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


    /* add the second prefix in the list and some more 4.4.4.4 - 4.4.x.y*/
    rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, unIpv4AddrArray[1],55,500, 3, 2, 1, 0, &lastIpAddrAdded,&numOfPrefixesAdded);
    if (rc == GT_OK)
    {
        cpssOsPrintf("[last ipv4 unicast added] = [%d]:[%d]:[%d]:[%d] numOfPrefixesAdded=%d,\n\r",
                  lastIpAddrAdded.arIP[0]
                 ,lastIpAddrAdded.arIP[1]
                 ,lastIpAddrAdded.arIP[2]
                 ,lastIpAddrAdded.arIP[3]
                 ,numOfPrefixesAdded);

    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcPrefixAddManyByOctet FAIL numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }

     /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 4.4.4.4 AFTER ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    for (ulLoop = 1532; ulLoop < 3324; ulLoop++)
    {
        ipv4Addr.arIP[3] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[2] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                                       override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }
    if (numberOfAddedPrefixes==3324)
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }

    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 5.5.5.5 BEFORE ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


    /* add the second prefix in the list and some more 5.5.5.5 - 5.5.x.y*/
    rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, unIpv4AddrArray[2],66,500, 3, 2, 1, 0, &lastIpAddrAdded,&numOfPrefixesAdded);
    if (rc == GT_OK)
    {
        cpssOsPrintf("[last ipv4 unicast added] = [%d]:[%d]:[%d]:[%d] numOfPrefixesAdded=%d,\n\r",
                  lastIpAddrAdded.arIP[0]
                 ,lastIpAddrAdded.arIP[1]
                 ,lastIpAddrAdded.arIP[2]
                 ,lastIpAddrAdded.arIP[3]
                 ,numOfPrefixesAdded);

    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcPrefixAddManyByOctet FAIL numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }

     /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 5.5.5.5 AFTER ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


    for (ulLoop = 3324; ulLoop < totalNumIpUc; ulLoop++)
    {
        ipv4Addr.arIP[3] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[2] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                                       override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }
    if (numberOfAddedPrefixes==numberOfPrefixesExpected)
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }

    /* print the block mapping state */
   cpssOsPrintf("==== HW Lpm Lines Counters usage - before remove\n  ");
   rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


    /* free some of the prefixes */
    for (ulLoop = 0; ulLoop < 3324; ulLoop++)
    {
        ipv4Addr.arIP[3] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[2] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixDel(0, 0, ipv4Addr,ipPrefixLen);

        if (rc!=GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
        }
    }
    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage after remove\n ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


    /* try to add prefixes with prefixLen/8
       if defrag=false we should get a fail
       if defrag=true we should have shrink+merge and succeed */
    ipv4Addr.arIP[2]=12;
    ipv4Addr.arIP[3]=13;
    ipPrefixLen = 16;
    numberOfAddedPrefixes=0;

    /* add some of the prefixes */
    for (ulLoop = 0; ulLoop < 5000; ulLoop++)
    {
        ipv4Addr.arIP[1] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[0] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                               override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }

    /* in case defragmentationEnable==GT_FALSE then we expect to add 1689 */
    numberOfPrefixesExpected = 1689;
     if (numberOfAddedPrefixes==numberOfPrefixesExpected)
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrint \n");
    prvTgfIpLpmDbgHwOctetPerProtocolPrint(0);
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters \n");
    prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters(0);


    defragmentationEnable=GT_TRUE;
    /* after we get a fail that there is no more space in memory
       we try to add some more prefixes with defragmentationEnable=GT_TRUE
       we can see that now we can add mutch more */
    for (ulLoop = numberOfPrefixesExpected; ulLoop < 10000; ulLoop++)
    {
        ipv4Addr.arIP[1] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[0] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                               override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }

   /* if we only use simple defrag we can add 1958 -
       this was checked manualy by skipping the merge part in the code
       if we use defrag + merge we can add 5542 meaning the merge
       added 3584 more then simple shrink */
    numberOfPrefixesExpected = 5542;
    if (numberOfAddedPrefixes==numberOfPrefixesExpected)
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrint \n");
    prvTgfIpLpmDbgHwOctetPerProtocolPrint(0);
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters \n");
    prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters(0);

    cpssOsFree(prefixesArrayPtr);

    return;
}

/**
* @internal prvTgfIpLpmIpUcOrderOctet0To6 function
* @endinternal
*
* @brief  Add some sequentially ipv6 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add

*/
GT_VOID prvTgfIpLpmIpUcOrderOctet0To6
(
    GT_U32                      totalNumIpUc,
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack,
    CPSS_UNICAST_MULTICAST_ENT  prefixType,
    GT_U32                      prefixLength,
    GT_U32                      srcPrefixLength
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  numberOfPrefixesExpected;

    GT_IPV6ADDR                             unIpv6Addr;
    GT_IPV6ADDR                             lastIpAddrAdded;
    GT_U32                                  numOfPrefixesAdded=0;


    PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITH defrag \n");
    if (totalNumIpUc<1364)
    {
        numberOfPrefixesExpected = totalNumIpUc;
    }
    else
    {
        numberOfPrefixesExpected = 1364;
    }

    cpssOsMemSet(&unIpv6Addr.arIP[0],0,sizeof(unIpv6Addr.arIP));

    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    /* change global prefixLength used in the prvTgfIpLpmIpv6UcPrefixAddManyByOctet API */
    rc = prvTgfIpLpmDbgPrefixLengthSet(protocolStack,prefixType,prefixLength,srcPrefixLength);
    if (rc!=GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmDbgPrefixLengthSet FAIL numberOfAddedPrefixes= %d \n",numOfPrefixesAdded);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmDbgPrefixLengthSet: %d", prvTgfDevNum);
    }
    /* add the prefixes */
    rc = prvTgfIpLpmIpv6UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, unIpv6Addr, 55, totalNumIpUc,
                                               0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                                               &lastIpAddrAdded,&numOfPrefixesAdded);

    if (rc == GT_OK)
    {
        cpssOsPrintf("[last ipv6 unicast added] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d] numOfPrefixesAdded=%d,\n\r",
                  lastIpAddrAdded.arIP[0]
                 ,lastIpAddrAdded.arIP[1]
                 ,lastIpAddrAdded.arIP[2]
                 ,lastIpAddrAdded.arIP[3]
                 ,lastIpAddrAdded.arIP[4]
                 ,lastIpAddrAdded.arIP[5]
                 ,lastIpAddrAdded.arIP[6]
                 ,lastIpAddrAdded.arIP[7]
                 ,lastIpAddrAdded.arIP[8]
                 ,lastIpAddrAdded.arIP[9]
                 ,lastIpAddrAdded.arIP[10]
                 ,lastIpAddrAdded.arIP[11]
                 ,lastIpAddrAdded.arIP[12]
                 ,lastIpAddrAdded.arIP[13]
                 ,lastIpAddrAdded.arIP[14]
                 ,lastIpAddrAdded.arIP[15]
                 ,numOfPrefixesAdded);
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcPrefixAddManyByOctet FAIL numberOfAddedPrefixes= %d \n",numOfPrefixesAdded);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }

     /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage ");
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrint \n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrint: %d", prvTgfDevNum);
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    if (numOfPrefixesAdded==numberOfPrefixesExpected)
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv6UcPrefixAddManyByOctet OK numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numOfPrefixesAdded, numberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv6UcPrefixAddManyByOctet FAIL numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numOfPrefixesAdded, numberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }

    rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, protocolStack , prefixType, GT_TRUE);
    if (rc!=GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmValidityCheck FAIL numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numOfPrefixesAdded, numberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmValidityCheck: %d", prvTgfDevNum);
    }


    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    cpssOsPrintf("==== HW Lpm Lines Counters usage ");
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrint \n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrint: %d", prvTgfDevNum);
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters \n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    /* change global prefixLength to default */
    rc = prvTgfIpLpmDbgPrefixLengthSet(protocolStack,prefixType,128,128);
    if(rc != GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmDbgPrefixLengthSet FAIL numberOfAddedPrefixes= %d \n",numOfPrefixesAdded);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmDbgPrefixLengthSet: %d", prvTgfDevNum);
    }
    return;
}







