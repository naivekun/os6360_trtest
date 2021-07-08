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
* @file prvTgfIpv4UcRoutingAddManyIpLpmMng.c
*
* @brief IPV4 UC Routing when filling the Lpm using cpssDxChIpLpmIpv4UcPrefixAddManyByOctet.
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
static GT_U32  numOfExistingPrefixesToDel=0;

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 257705
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 257277 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_320K_LINES_CNS     257705 /* 265213 with PBR of 8K entries */

/* number of prefixes expected to be added in a device with 320K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 302077
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 302333 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_320K_LINES_CNS         302077 /* 310269 with PBR of 8K entries */

/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 92157
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 91901 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_120K_LINES_CNS     92157 /* 99837 with PBR of 8K entries */

/* number of prefixes expected to be added in a device with 120K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 106749
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 107005 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_120K_LINES_CNS         106749 /* 114685 with PBR of 8K entries */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when SWAP is used at the beginning of memory allocation       --> 23462
   expected value when SWAP is used before allocating a new memory  block       --> 23037
   expected value when Shrink is done - defragmentationEnable=GT_TRUE           --> 23462
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE     --> 23037 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_CNS     23462/* 0 with PBR of 8K entries */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
/* expected value when SWAP is used at the beginning of memory allocation                                       --> 28070
   expected value when SWAP is used before allocating a new memory  block                                       --> 27901
   expected value when 2 SWAP areas is in used                                                                  --> 27389
   expected value when Shrink is done - defragmentationEnable=GT_TRUE, currently not supported for share mode   --> 27389
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE                                     --> 27901 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_CNS         27389 /* 0 with PBR of 8K entries */

/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = do_not_share and using
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_160K_LINES_CNS    123049

/* number of prefixes expected to be added in a device with 160K LPM lines
   blocksAllocationMethod = share and using
   prvTgfIpLpmIpv4UcPrefixAddManyByOctet with
   startIpPrefix=0.0.0.1 and octet order is 3,2,1,0  */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_160K_LINES_CNS        144125

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpLpmMngIpv4UcRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpLpmMngIpv4UcRoutingAddManyConfigurationAndTrafficGenerate(GT_VOID)
{
    GT_STATUS   rc = GT_OK;
    GT_IPADDR   startIpAddr;
    GT_U32      numOfPrefixesToAdd;
    GT_U32      firstOctetToChange;
    GT_U32      secondOctetToChange;
    GT_U32      thirdOctetToChange;
    GT_U32      forthOctetToChange;
    GT_IPADDR   lastIpAddrAdded;
    GT_U32      numOfPrefixesAdded;
    GT_U32      numOfPrefixesExpected;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Create Ipv4 prefixes in a Virtual Router for the specified LPM DB
     */

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before adding new prefixes");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Adding Prefixes using  cpssDxChIpLpmIpv4UcPrefixAddManyByOctet\n "
                     "This function tries to add many sequential IPv4 Unicast prefixes according\n"
                     "to octet order selection and returns the number of prefixes successfully added. ====\n");


    numOfPrefixesToAdd=1000000;
    numOfPrefixesExpected=1000000;
    firstOctetToChange=3;
    secondOctetToChange=2;
    thirdOctetToChange=1;
    forthOctetToChange=0;

    startIpAddr.arIP[0]=0;
    startIpAddr.arIP[1]=0;
    startIpAddr.arIP[2]=0;
    startIpAddr.arIP[3]=1;

    rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, startIpAddr, prvTgfRouteEntryBaseIndex,
                                               numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                                               thirdOctetToChange,forthOctetToChange, &lastIpAddrAdded, &numOfPrefixesAdded);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAddManyByOctet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("\n==== HW Octet to Block Mapping after adding new prefixes");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);


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
            numOfPrefixesExpected = 18173;
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
            numOfPrefixesExpected = 18173;
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

    numOfExistingPrefixesToDel = numOfPrefixesAdded;

    /* check if we get the correct number of prefixed added */
    UTF_VERIFY_EQUAL2_STRING_MAC(numOfPrefixesExpected, numOfPrefixesAdded,
                                  "======= numOfPrefixesAdded[%d] is different then numOfPrefixesExpected value [%d] =======\n",
                                 numOfPrefixesAdded,numOfPrefixesExpected);

    rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. send packet to the first and last prefix added
     */

    /* AUTODOC: send packet to the last first added */
    prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], startIpAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

    /* AUTODOC: send packet to the last prefix added */
    prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], lastIpAddrAdded, PRV_TGF_NEXTHOPE_VLANID_CNS);

}

/**
* @internal prvTgfIpLpmMngIpv4UcRoutingAddManyConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpLpmMngIpv4UcRoutingAddManyConfigurationRestore(GT_VOID)
{
    /* -------------------------------------------------------------------------
     * 1. Restore Route Configuration
     */

    GT_STATUS   rc = GT_OK;
    GT_IPADDR   startIpAddr;
    GT_U32      firstOctetToChange;
    GT_U32      secondOctetToChange;
    GT_U32      thirdOctetToChange;
    GT_U32      forthOctetToChange;
    GT_IPADDR   lastIpAddrDeleted;
    GT_U32      numOfPrefixesDeleted;
    GT_U32      numOfPrefixesExpected=0;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;

     rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping before restore ");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Deleting Prefixes using  cpssDxChIpLpmIpv4UcPrefixDelManyByOctet\n "
                     "This function tries to delete many sequential IPv4 Unicast prefixes according\n"
                     "to octet order selection and returns the number of prefixes successfully deleted. ====\n");

    firstOctetToChange=3;
    secondOctetToChange=2;
    thirdOctetToChange=1;
    forthOctetToChange=0;

    startIpAddr.arIP[0]=0;
    startIpAddr.arIP[1]=0;
    startIpAddr.arIP[2]=0;
    startIpAddr.arIP[3]=1;

    rc = prvTgfIpLpmIpv4UcPrefixDelManyByOctet(prvTgfLpmDBId, 0, startIpAddr,numOfExistingPrefixesToDel,
                                               firstOctetToChange, secondOctetToChange,
                                               thirdOctetToChange,forthOctetToChange,
                                               &lastIpAddrDeleted, &numOfPrefixesDeleted);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDelManyByOctet: %d", prvTgfDevNum);

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
            numOfPrefixesExpected = 18173;
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
            numOfPrefixesExpected = 18173;
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

    /* check if we get the correct number of prefixed deleted */
    UTF_VERIFY_EQUAL2_STRING_MAC(numOfPrefixesExpected, numOfExistingPrefixesToDel,
                                  "======= numOfExistingPrefixesToDel[%d] is different then numOfPrefixesExpected value [%d] =======\n",
                                 numOfExistingPrefixesToDel,numOfPrefixesExpected);

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping after restore ");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    /* change blocks allocation method and reset LPM DB */
    prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E);

}

/**
* @internal prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig function
* @endinternal
*
* @brief   This function cleans CPSS appDemo default configurations -
*         LPM DB id 0 with VR id 0
*         changes the LPM RAM blocks allocation method to DYNAMIC
*         then restores the CPSS appDemo default configurations
* @param[in] blocksAllocationMethod   - the method of blocks allocation
*                                       GT_OK on success, or
*
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_VOID prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig
(
    IN PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT  blocksAllocationMethod
)
{
    GT_STATUS rc;
    GT_U32 value=0;

    /* AUTODOC: delete the default Virtual Router */
    rc = prvTgfIpLpmVirtualRouterDelDefault();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterDelDefault");

    /* AUTODOC: remove the default LPM DB */
    rc = prvTgfIpLpmDBDeleteDefault();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDBDeleteDefault");

    /* AUTODOC: Set the method of blocks allocation Memory for RAM based LPM shadow */
    switch (blocksAllocationMethod)
    {
    case PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E:
        value = 0;
        break;
    case PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E:
        value = 1;
        break;
    default:
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "test error: wrong LPM blocks allocation method");
    }

    /* LPM RAM Configuration for Dynamic LPM Management */
    rc = appDemoDbEntryAdd("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", value);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd");

    /* AUTODOC: restore the default LPM DB */
    rc = prvTgfIpLpmRamDBCreateDefault();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDBCreateDefault");

    /* AUTODOC: restore the default device */
    rc = prvTgfIpLpmDBDevListAddDefault();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDBDevListAddDefault");

    /* AUTODOC: restore the default VR */
    rc = prvTgfIpLpmVirtualRouterAddDefault();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterAddDefault");

}
/**
* @internal prvTgfIpLpmMngRoutingAddManyRestoreToDefault function
* @endinternal
*
* @brief   This function restores the CPSS appDemo to default configurations.
*
* @param[in] currentBlockAllocationMethod - the method of blocks allocation currently used at the system
* @param[in] currentLpmMemoryMode     - current lpm memory mode that is used at the system
* @param[in] currentPbrSize           - current size of PBR that is used at the system
*                                       None
*
* @note In case PBR size or LPM memory mode is different then default ,then system reset is triggered.
*       In case only the method of blocks allocationis different then default , then no system reset is triggered
*
*/

GT_VOID prvTgfIpLpmMngRoutingAddManyRestoreToDefault
(
    IN PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT    currentBlockAllocationMethod,
    IN PRV_TGF_LPM_RAM_MEM_MODE_ENT                    currentLpmMemoryMode,
    IN GT_U32                                          currentPbrSize
)

{
    GT_STATUS rc;
    GT_BOOL restartNeeded = GT_FALSE;


    if(currentLpmMemoryMode != PRV_TGF_IP_ADD_MANY_DEFAULT_LPM_RAM_MEMORY_MODE)
    {
         /* return default lpm mode */
        rc = appDemoDbEntryAdd("lpmMemMode", PRV_TGF_IP_ADD_MANY_DEFAULT_LPM_RAM_MEMORY_MODE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd");
        restartNeeded = GT_TRUE;
    }

    if(currentPbrSize != PRV_TGF_IP_ADD_MANY_DEFAULT_PBR_SIZE_CNS)
    {
        /* return default PBR size */
        rc = appDemoDbEntryAdd("maxNumOfPbrEntries", PRV_TGF_IP_ADD_MANY_DEFAULT_PBR_SIZE_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");
        restartNeeded = GT_TRUE;
    }

    if(currentBlockAllocationMethod != PRV_TGF_IP_ADD_MANY_DEFAULT_LPM_RAM_BLOCKS_ALLOCATION_METHOD)
    {
         /* LPM RAM Configuration for Dynamic LPM Management */
        rc = appDemoDbEntryAdd("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", PRV_TGF_IP_ADD_MANY_DEFAULT_LPM_RAM_BLOCKS_ALLOCATION_METHOD);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd");
        /*Only block allocation method is changed*/
        if(restartNeeded == GT_FALSE)
        {
            prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(PRV_TGF_IP_ADD_MANY_DEFAULT_LPM_RAM_BLOCKS_ALLOCATION_METHOD);
        }
    }

    if(restartNeeded == GT_TRUE)
    {
        prvTgfResetAndInitSystem();
    }

}

/**
* @internal prvTgfIpLpmMngRoutingAddManyPrepareIteration function
* @endinternal
*
* @brief   This function configure the CPSS appDemo to configurations required by the current test step.
*
* @param[in,out] currentBlockAllocationMethodPtr - the method of blocks allocation currently used at the system
*                                      currentLpmMemoryModePrt - current lpm memory mode that is used at the system
* @param[in,out] currentPbrSizePtr        - current size of PBR that is used at the system
* @param[in,out] currentBlockAllocationMethodPtr - the method of blocks allocation currently used at the system
*                                      currentLpmMemoryModePrt - current lpm memory mode that is used at the system
* @param[in,out] currentPbrSizePtr        - current size of PBR that is used at the system
*
* @param[out] newBlockAllocationMethod - the method of blocks allocation required by the current test step
* @param[out] newLpmMemoryMode         - the lpm memory mode required by the current test step
* @param[out] newPbrSize               - the size of PBR required by the current test step
* @param[out] iterationValidPtr        - GT_FALSE if current configuration is not supported by CPSS, GT_TRUE otherwise
*                                       None
*/

GT_VOID prvTgfIpLpmMngRoutingAddManyPrepareIteration
(
    INOUT PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT    *currentBlockAllocationMethodPtr,
    INOUT PRV_TGF_LPM_RAM_MEM_MODE_ENT                    *currentLpmMemoryModePtr,
    INOUT GT_U32                                          *currentPbrSizePtr,
    IN PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT       newBlockAllocationMethod,
    IN PRV_TGF_LPM_RAM_MEM_MODE_ENT                       newLpmMemoryMode,
    IN GT_U32                                             newPbrSize,
    OUT GT_BOOL                                           *iterationValidPtr
)
{
    GT_STATUS rc;
    GT_BOOL resetNeeded = GT_FALSE;
    PRV_TGF_LPM_RAM_CONFIG_STC              ramDbCfg;


    resetNeeded = GT_FALSE;

    /*PBR size changed , need to reset*/
    if(*currentPbrSizePtr != newPbrSize)
    {
        if(prvTgfResetModeGet() == GT_TRUE)
        {
            *currentPbrSizePtr = newPbrSize;

            rc = appDemoDbEntryAdd("maxNumOfPbrEntries", newPbrSize);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd(maxNumOfPbrEntries)");

            resetNeeded = GT_TRUE;
        }
        else
        {   /*reset not supported,skip*/

            *iterationValidPtr = GT_FALSE;
        }

    }
    /*Lpm memory mode changed  ,need to reset*/
    if(*currentLpmMemoryModePtr !=newLpmMemoryMode)
    {
        if(prvTgfResetModeGet() == GT_TRUE)
        {
            *currentLpmMemoryModePtr  = newLpmMemoryMode;

            /* LPM RAM Configuration for  LPM Memory mode */
            rc = appDemoDbEntryAdd("lpmMemMode", newLpmMemoryMode);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd");

            resetNeeded = GT_TRUE;
        }
        else
        {   /*reset not supported,skip*/

            *iterationValidPtr = GT_FALSE;
        }
    }

    if(*currentBlockAllocationMethodPtr != newBlockAllocationMethod)
    {
        *currentBlockAllocationMethodPtr = newBlockAllocationMethod;

        if(resetNeeded == GT_FALSE)
        {
            prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig(newBlockAllocationMethod);
        }
        /*only add to appDemodDb,we are going to do reset anyway*/
        else
        {
            /* LPM RAM Configuration for Dynamic LPM Management */
            rc = appDemoDbEntryAdd("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", newBlockAllocationMethod);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoDbEntryAdd");
        }
    }

    if(resetNeeded == GT_TRUE)
    {
        prvTgfResetAndInitSystem();
    }

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigGet: %d", prvTgfDevNum);
    cpssOsPrintf("Block size: %d K lines ,number of blocks %d\n",ramDbCfg.blocksSizeArray[0]/_4K,ramDbCfg.numOfBlocks);

}



