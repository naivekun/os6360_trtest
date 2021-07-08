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
* @file cpssDxChIpLpmUT.c
*
* @brief Unit tests for cpssDxChIpLpm, that provides
* the CPSS DXCH LPM Hierarchy manager.
*
* @version   80
********************************************************************************
*/
/* includes */

#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/resourceManager/cpssDxChTcamManager.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

/* lpm db skip list ptr */
extern GT_VOID *prvCpssDxChIplpmDbSL;

/* Defines */

/* Tests use this vlan id for testing VLAN functions */
#define IP_LPM_TESTED_VLAN_ID_CNS  100

/* Default lpmDbId */
#define IP_LPM_DEFAULT_DB_ID_CNS            1

/* Default vrId */
#define IP_LPM_DEFAULT_VR_ID_CNS            0

/* Invalid lpmDBId */
#define IP_LPM_INVALID_DB_ID_CNS            55555

/* Invalid vrId */
#define IP_LPM_INVALID_VR_ID_CNS            77777

/* memory in lines reserved for PBR from the LPM memory */
#define IP_LPM_PBR_SIZE_CNS                 _8KB

/* max number of LPM blocks on eArch architecture */
#define IP_LPM_MAX_NUM_OF_LPM_BLOCKS_CNS    20

/* return number of LPM lines in ram according to number of prefixes supported.
   We use the rule of average 10Bytes for Ipv4 prefix and 20Bytes for Ipv6 prefix.
   each line holds 4 byte so one prefix hold 10 Bytes = 2.5 Lines. */
#define IP_LPM_RAM_GET_NUM_OF_LINES_MAC(_numOfPrefixes)        \
    ((GT_U32)(_numOfPrefixes*2.5))

/* Internal functions forward declaration */

/* Creates two LpmDBs */
static GT_STATUS prvUtfCreateDefaultLpmDB(GT_U8 dev);

/* Creates LpmDB for multicast */
static GT_STATUS prvUtfCreateMcLpmDB(GT_U8 dev);

/* Creates two LpmDBs and Virtual Router */
static GT_STATUS prvUtfCreateLpmDBAndVirtualRouterAdd(IN GT_U8 dev);

/* Creates LpmDB and Virtual Routers and add dev to the created LpmDB */
static GT_STATUS prvUtfCreateLpmDBWithDeviceAndVirtualRouterAdd();

/* Delete LpmDB with devices added to it and Virtual Routers.  */
static GT_STATUS prvUtfDeleteLpmDBWithDeviceAndVirtualRouter();

/* Compare PclIpUcActionEntries. */
static void prvUtfComparePclIpUcActionEntries(IN GT_U32                    lpmDBId,
                                              IN GT_U32                    vrId,
                                              IN CPSS_DXCH_PCL_ACTION_STC* pclActionReceivePtr,
                                              IN CPSS_DXCH_PCL_ACTION_STC* pclActionExpectedPtr);

/* Compare IpLttEntries. */
static void prvUtfCompareIpLttEntries(IN GT_U32                      lpmDBId,
                                      IN GT_U32                      vrId,
                                      IN CPSS_DXCH_IP_LTT_ENTRY_STC* IpLttEntryReceivePtr,
                                      IN CPSS_DXCH_IP_LTT_ENTRY_STC* IpLttEntryExpectedPtr);

/* Fills returns PclIpUcActionEntry with default values. */
static void prvUtfSetDefaultActionEntry(IN CPSS_DXCH_PCL_ACTION_STC* pclActionPtr);

/* Fills returns IpLttEntry with default values. */
static void prvUtfSetDefaultIpLttEntry(IN GT_U8                         devId,
                                       IN CPSS_DXCH_IP_LTT_ENTRY_STC*   ipLttEntry);

/* get the shadow type of a given device*/
static GT_STATUS prvUtfGetDevShadow(GT_U8 dev,CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT *shadowType);

extern GT_STATUS appDemoBc2IpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);

/**
* @internal prvUtfIpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculates the default RAM LPM DB configuration for LPM managment.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PTR               - Null pointer
*/
GT_STATUS prvUtfIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                        devNum,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC *ramDbCfgPtr
)
{

    GT_STATUS rc ;

    CPSS_NULL_PTR_CHECK_MAC(ramDbCfgPtr);

    rc = appDemoBc2IpLpmRamDefaultConfigCalc(devNum,
                                             PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->moduleCfg.ip.maxNumOfPbrEntries,
                                             ramDbCfgPtr);

    return rc;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmDBCreate
(
    IN GT_U32                                       lpmDBId,
    IN CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType,
    IN CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack,
    IN CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           *memoryCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDBCreate)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{firstIndex [0], lastIndex [100]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10],
        numOfIpv4McSourcePrefixes [5], numOfIpv6Prefixes [10]}
    Expected: GT_OK.
    1.2. Call with lpmDBId [1], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [200], lastIndex [1000]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]}
    Expected: GT_OK.
    1.3. Call with the same lpmDBId [0] (already created) and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with lpmDBId [4], wrong enum values shadowType
        and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with lpmDBId [2], wrong enum values protocolStack
        and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call with lpmDBId [5], indexesRangePtr{firstIndex [0],
        lastIndex[0xFFFFFFFF]} (index should be in range 0-1023)
    Expected: NON GT_OK.
    1.7. Call with lpmDBId [6], indexesRangePtr{firstIndex [10], lastIndex[10] }
    Expected: NON GT_OK.
    1.8. Call with lpmDBId [7], indexesRangePtr{firstIndex [10], lastIndex[0] }
    Expected: NOT GT_OK.
    1.10. Call with lpmDBId [9], partitionEnable [GT_TRUE] and
        tcamLpmManagerCapcityCfgPtr[NULL] (relevant only if partitionEnable = GT_TRUE)
        and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.11. Call with lpmDBId [10], indexesRangePtr [NULL] and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.12. Call with lpmDBId [11], partitionEnable [GT_TRUE] and
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [0xFFFFFFFF],
        numOfIpv4McSourcePrefixes [5], numOfIpv6Prefixes [10]}.
    Expected: NON GT_OK.
    1.13. Call with lpmDBId [12], partitionEnable [GT_TRUE] and
        tcamLpmManagerCapcityCfgPtr {numOfIpv4Prefixes [10],
        numOfIpv4McSourcePrefixes [0xFFFFFFFF], numOfIpv6Prefixes [10]}.
    Expected: NON GT_OK.
    1.14. Call with lpmDBId [13], partitionEnable [GT_TRUE] and
        tcamLpmManagerCapcityCfgPtr {numOfIpv4Prefixes [10],
        numOfIpv4McSourcePrefixes [5], numOfIpv6Prefixes [0xFFFFFFFF]}.
    Expected: NON GT_OK.
    1.15. Call with lpmDBId [14], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[0],
        blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E]}.
    Expected: GT_BAD_PARAM.
    1.16. Call with lpmDBId [14], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[1],
        blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E]}.
    Expected: GT_OK.
    1.17. Call with lpmDBId [15], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[20],
        blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E]}.
    Expected: GT_OK.
    1.18. Call with lpmDBId [16], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[21],
        blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E]}.
    Expected: GT_BAD_PARAM.
    1.19.Call with lpmDBId [16], blocksSizeArray[0,...,16K,_8K], numOfBlocks[17],
        blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E]}.
    Expected: GT_BAD_PARAM.
    1.20. Call with lpmDBId [16], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[17],
        blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E]}.
    Expected: GT_OK.
    1.21.Call with lpmDBId [17], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[17],
        blocksAllocationMethod[wrong enum]}.
    Expected: GT_BAD_PARAM.
    1.22.Call with lpmDBId [17], blocksSizeArray[_32K,...,16K,_8K], numOfBlocks[20],
        blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E]}.
    Expected: GT_OK.
    1.23.Call with lpmDBId [27], numOfBlocks[20],
                lpmMemMode[wrong enum]}.
                Expected: GT_BAD_PARAM.
     1.24.Call with lpmDBId [28], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[20],
                    blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E],
                    lpmMemMode[CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E]}.
                    Expected: GT_OK.


*/
    GT_STATUS st        = GT_OK;
    GT_U8     dev;

    GT_U32                                       lpmDBId         = 0;
    GT_U32                                       maxTcamIndex    = 0;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType      = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
    CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack   = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC tcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;
    GT_U32                                       defaultNumOfBlocks=0, defaultBlocksSizeArray_0=0;
    GT_BOOL                                      firstDevNonSip5=GT_TRUE;/* used for multi device system.
                                                             shadow will be created only for
                                                             first device call, all other calls should
                                                             return GT_ALREADY_EXIST */
    GT_BOOL                                      firstDevSip5=GT_TRUE;/* used for multi device system.
                                                             shadow will be created only for
                                                             first device call, all other calls should
                                                             return GT_ALREADY_EXIST */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&lpmMemoryConfig, sizeof(lpmMemoryConfig));
    cpssOsBzero((GT_VOID*)&indexesRange, sizeof(indexesRange));
    cpssOsBzero((GT_VOID*)&tcamLpmManagerCapcityCfg, sizeof(tcamLpmManagerCapcityCfg));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfGetDevShadow(dev, &shadowType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            /* get max Tcam index */
            maxTcamIndex = (CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E == shadowType) ?
                           (PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.policyTcamRaws) :
                           (PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router);
        }

        /*
            1.1. Call with lpmDBId [0],
                           protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
                           indexesRangePtr{firstIndex [0], lastIndex [100]},
                           ramDbCfg{numOfBlocks[20],
                                    blocksSizeArray[_16K,...,16K,_8K],
                                    blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E]},
                           partitionEnable[GT_TRUE],
                           tcamLpmManagerCapcityCfgPtr{numOfIpv4Prefixes [10],
                                                       numOfIpv4McSourcePrefixes [5],
                                                       numOfIpv6Prefixes [10]}
            Expected: GT_OK.
        */
        lpmDBId         = 0;
        protocolStack   = CPSS_IP_PROTOCOL_IPV4V6_E;
        if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            lpmMemoryConfig.tcamDbCfg.partitionEnable = GT_TRUE;

            indexesRange.firstIndex = 0;
            indexesRange.lastIndex  = 100;
            lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &indexesRange;

            tcamLpmManagerCapcityCfg.numOfIpv4Prefixes         = 10;
            tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 5;
            tcamLpmManagerCapcityCfg.numOfIpv6Prefixes         = 10;
            lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;

            lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = NULL;
        }
        else
        {
            st = prvUtfIpLpmRamDefaultConfigCalc(dev,&lpmMemoryConfig.ramDbCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);
            /* keep defaults for reconstruct */
            defaultNumOfBlocks = lpmMemoryConfig.ramDbCfg.numOfBlocks;
            defaultBlocksSizeArray_0 = lpmMemoryConfig.ramDbCfg.blocksSizeArray[0];
        }

        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
        st = GT_ALREADY_EXIST == st ? GT_OK : st;
        if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, %d, lpmMemoryConfig.tcamDbCfg.partitionEnable = %d",
                                         lpmDBId, shadowType, protocolStack, lpmMemoryConfig.tcamDbCfg.partitionEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                                                    "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                    "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                         lpmDBId, shadowType, protocolStack,
                                         lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                         lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
        }

        /*
            1.2. Call with lpmDBId [1],
                           protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
                           indexesRangePtr{ firstIndex [200], lastIndex [1000]},
                           partitionEnable[GT_TRUE],
                           tcamLpmManagerCapcityCfgPtr{numOfIpv4Prefixes [10],
                                                       numOfIpv4McSourcePrefixes [5],
                                                       numOfIpv6Prefixes [10]}
            Expected: GT_OK.
        */
        lpmDBId    = 1;

        if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            indexesRange.firstIndex = 200;
            indexesRange.lastIndex  = maxTcamIndex - 1;
        }

        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
        if((firstDevNonSip5==GT_FALSE)||(firstDevSip5==GT_FALSE))
        {
            st = GT_ALREADY_EXIST == st ? GT_OK : st;
        }
        if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, %d, lpmMemoryConfig.tcamDbCfg.partitionEnable = %d",
                                         lpmDBId, shadowType, protocolStack, lpmMemoryConfig.tcamDbCfg.partitionEnable);
        }
        else
        {
           UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                                                    "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                    "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                         lpmDBId, shadowType, protocolStack,
                                         lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                         lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
        }

        /*
            1.3. Call with the same lpmDBId [0] (already created)
                           and other parameters from 1.1.
            Expected: NOT GT_OK.
        */
        lpmDBId    = 0;

        if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            indexesRange.firstIndex = 0;
            indexesRange.lastIndex  = 100;
        }

        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, shadowType);

        /*
            1.4. Call with lpmDBId [4], wrong enum values shadowType
                           and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        lpmDBId = 4;

        UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmDBCreate
                            (lpmDBId, shadowType, protocolStack, &lpmMemoryConfig),
                            shadowType);

        st = prvUtfGetDevShadow(dev, &shadowType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        /*
            1.5. Call with lpmDBId [2], wrong enum values protocolStack
                           and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        lpmDBId = 2;

        UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmDBCreate
                            (lpmDBId, shadowType, protocolStack, &lpmMemoryConfig),
                            protocolStack);

        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;

        if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {


            /*
                1.6. Call with lpmDBId [5], indexesRangePtr{firstIndex [0],
                          lastIndex[0xFFFFFFFF]} (index should be in range 0-1023)
                Expected: NON GT_OK.
            */
            lpmDBId = 5;

            indexesRange.firstIndex = 0;
            indexesRange.lastIndex  = 0xFFFFFFFF;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, indexesRangePtr->lastIndex = %d",
                                             lpmDBId, lpmMemoryConfig.tcamDbCfg.indexesRangePtr->lastIndex);

            indexesRange.lastIndex = 100;

            /*
                1.7. Call with lpmDBId [6], indexesRangePtr{firstIndex [10], lastIndex[10] }
                Expected: NON GT_OK.
            */
            lpmDBId = 6;

            indexesRange.firstIndex = 10;
            indexesRange.lastIndex  = 10;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, lpmMemoryConfig.tcamDbCfg->indexesRangePtr.firstIndex = %d, lpmMemoryConfig.tcamDbCfg->indexesRangePtr.lastIndex = %d",
                    lpmDBId, lpmMemoryConfig.tcamDbCfg.indexesRangePtr->firstIndex,
                    lpmMemoryConfig.tcamDbCfg.indexesRangePtr->lastIndex);

            indexesRange.firstIndex = 0;
            indexesRange.lastIndex  = 100;

            /*
                1.8. Call with lpmDBId [7], indexesRangePtr{firstIndex [10], lastIndex[0] }
                Expected: NOT GT_OK.
            */
            lpmDBId = 7;

            indexesRange.firstIndex = 10;
            indexesRange.lastIndex  = 0;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, lpmMemoryConfig.tcamDbCfg->indexesRangePtr.firstIndex = %d,"
                    "lpmMemoryConfig.tcamDbCfg->indexesRangePtr.lastIndex = %d",
                                 lpmDBId, lpmMemoryConfig.tcamDbCfg.indexesRangePtr->firstIndex,
                                 lpmMemoryConfig.tcamDbCfg.indexesRangePtr->lastIndex);

            indexesRange.firstIndex = 0;
            indexesRange.lastIndex  = 100;

            /*
                1.10. Call with lpmDBId [9], partitionEnable [GT_TRUE]
                                and tcamLpmManagerCapcityCfgPtr[NULL]
                                (relevant only if partitionEnable = GT_TRUE.)
                                and other parameters from 1.1.
                Expected: GT_BAD_PTR.
            */
            lpmDBId         = 9;
            lpmMemoryConfig.tcamDbCfg.partitionEnable = GT_TRUE;
            lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = NULL;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                          "%d, lpmMemoryConfig.tcamDbCfg.partitionEnable = %d, lpmMemoryConfig.tcamDbCfg->tcamLpmManagerCapcityCfgPtr-> = NULL",
                                                          lpmDBId, lpmMemoryConfig.tcamDbCfg.partitionEnable);

            /*
                1.11. Call with lpmDBId [10], indexesRangePtr [NULL]
                      and other parameters from 1.1.
                Expected: GT_BAD_PTR.
            */
            lpmDBId = 10;
            lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
            lpmMemoryConfig.tcamDbCfg.indexesRangePtr = NULL;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, indexesRangePtr = NULL", lpmDBId);

            /*
                1.12. Call with lpmDBId [11], partitionEnable [GT_TRUE]
                      and tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [0xFFFFFFFF],
                      numOfIpv4McSourcePrefixes [5], numOfIpv6Prefixes [10]}.
                Expected: NON GT_OK.
            */
            lpmDBId = 11;

            lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &indexesRange;
            tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = 0xFFFFFFFF;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "%d, lpmMemoryConfig.tcamDbCfg->tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes = %d",
                                lpmDBId, lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes);

            tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = 10;

            /*
                1.13. Call with lpmDBId [12], partitionEnable [GT_TRUE]
                        and tcamLpmManagerCapcityCfgPtr {numOfIpv4Prefixes [10],
                                                         numOfIpv4McSourcePrefixes [0xFFFFFFFF],
                                                         numOfIpv6Prefixes [10]}.
                Expected: NON GT_OK.
            */
            lpmDBId = 12;
            tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 0xFFFFFFFF;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            /* numOfIpv4McSourcePrefixes is not used for CPSS_DXCH_IP_LPM_TCAM_CHEETAH_SHADOW_E*/
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_CHEETAH_E)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes = %d",
                                lpmDBId, lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes = %d",
                                 lpmDBId, lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes);
            }


            tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 5;

            /*
                1.14. Call with lpmDBId [13],
                            partitionEnable [GT_TRUE]
                            and tcamLpmManagerCapcityCfgPtr {numOfIpv4Prefixes [10],
                                                             numOfIpv4McSourcePrefixes [5],
                                                             numOfIpv6Prefixes [0xFFFFFFFF]}.
                Expected: NON GT_OK.
            */
            lpmDBId = 13;
            tcamLpmManagerCapcityCfg.numOfIpv6Prefixes = 0xFFFFFFFF;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                    "%d, tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes = %d",
                                      lpmDBId, lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes);

            tcamLpmManagerCapcityCfg.numOfIpv6Prefixes = 10;

            firstDevNonSip5=GT_FALSE;
        }
        else    /* CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E */
        {
            /*
                1.15. Call with lpmDBId [14], blocksSizeArray[_16K,...,16K,_8K] numOfBlocks[0],
                blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E]}.
                Expected: GT_BAD_PARAM.
            */
            lpmDBId = 14;
            lpmMemoryConfig.ramDbCfg.numOfBlocks = 0;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            if (firstDevSip5==GT_TRUE)
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_BAD_PARAM, st, "%d, %d, %d, "
                                                    "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                    "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                         lpmDBId, shadowType, protocolStack,
                                         lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                         lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }
            else
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_ALREADY_EXIST, st, "%d, %d, %d, "
                                                    "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                    "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                         lpmDBId, shadowType, protocolStack,
                                         lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                         lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }


            /*
                1.16. Call with lpmDBId [14], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[1],
                blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E]}.
                Expected: GT_OK.
            */
            lpmMemoryConfig.ramDbCfg.numOfBlocks = 1;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            if (firstDevSip5==GT_TRUE)
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                                                        "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                        "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                             lpmDBId, shadowType, protocolStack,
                                             lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                             lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }
            else
            {
                    UTF_VERIFY_EQUAL5_STRING_MAC(GT_ALREADY_EXIST, st, "%d, %d, %d, "
                                                        "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                        "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                             lpmDBId, shadowType, protocolStack,
                                             lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                             lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }

            /*
                 1.17. Call with lpmDBId [15], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[20],                                                                                                         .
                 blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E]}.
                Expected: GT_OK.
            */
            lpmDBId = 15;
            lpmMemoryConfig.ramDbCfg.numOfBlocks = defaultNumOfBlocks;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            if (firstDevSip5==GT_TRUE)
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                                                        "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                        "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                             lpmDBId, shadowType, protocolStack,
                                             lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                             lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }
            else
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_ALREADY_EXIST, st, "%d, %d, %d, "
                                                    "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                    "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                         lpmDBId, shadowType, protocolStack,
                                         lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                         lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }

            /*
                1.18. Call with lpmDBId [16], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[21],                                                                                                         .
                blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E]}.
                Expected: GT_BAD_PARAM.
            */
            lpmDBId = 16;
            lpmMemoryConfig.ramDbCfg.numOfBlocks = defaultNumOfBlocks+ 1;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            if (firstDevSip5==GT_TRUE)
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_BAD_PARAM, st, "%d, %d, %d, "
                                                        "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                        "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                             lpmDBId, shadowType, protocolStack,
                                             lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                             lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }
            else
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_ALREADY_EXIST, st, "%d, %d, %d, "
                                                        "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                        "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                             lpmDBId, shadowType, protocolStack,
                                             lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                             lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }

            /*
                1.19.Call with lpmDBId [16],blocksSizeArray[0,...,16K,_8K], numOfBlocks[17],                                                                                                         .
                blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E]}.
                Expected: GT_BAD_PARAM.
            */
           lpmMemoryConfig.ramDbCfg.numOfBlocks = 17;
           lpmMemoryConfig.ramDbCfg.blocksSizeArray[0]=0;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            if (firstDevSip5==GT_TRUE)
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_BAD_PARAM, st, "%d, %d, %d, "
                                                        "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                        "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                             lpmDBId, shadowType, protocolStack,
                                             lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                             lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }
            else
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_ALREADY_EXIST, st, "%d, %d, %d, "
                                                        "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                        "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                             lpmDBId, shadowType, protocolStack,
                                             lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                             lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }
            /*
                 1.20. Call with lpmDBId [16], numOfBlocks[15],                                                                                                      .
                 blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E]}.
                Expected: GT_OK.
            */
            lpmMemoryConfig.ramDbCfg.blocksSizeArray[0] = defaultBlocksSizeArray_0;/* in bytes */
            lpmMemoryConfig.ramDbCfg.numOfBlocks = 15;
            lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            if (firstDevSip5==GT_TRUE)
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                                                        "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                        "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                             lpmDBId, shadowType, protocolStack,
                                             lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                             lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }
            else
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_ALREADY_EXIST, st, "%d, %d, %d, "
                                                    "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                    "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                         lpmDBId, shadowType, protocolStack,
                                         lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                         lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }

            /*
                1.21.Call with lpmDBId [17], numOfBlocks[15],
                blocksAllocationMethod[wrong enum]}.
                Expected: GT_BAD_PARAM.
            */
            lpmDBId = 17;
            if (firstDevSip5==GT_TRUE)
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmDBCreate
                                    (lpmDBId, shadowType, protocolStack, &lpmMemoryConfig),
                                    lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }
            /*
                 1.22.Call with lpmDBId [17], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[20],
                blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E]}.
                Expected: GT_OK.
            */

            lpmMemoryConfig.ramDbCfg.blocksSizeArray[0] = defaultBlocksSizeArray_0;/* in bytes */
            lpmMemoryConfig.ramDbCfg.numOfBlocks = defaultNumOfBlocks;

            lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E;

            st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
            if (firstDevSip5==GT_TRUE)
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                                                        "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                        "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                             lpmDBId, shadowType, protocolStack,
                                             lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                             lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }
            else
            {
                UTF_VERIFY_EQUAL5_STRING_MAC(GT_ALREADY_EXIST, st, "%d, %d, %d, "
                                                        "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                        "lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = %d",
                                             lpmDBId, shadowType, protocolStack,
                                             lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                             lpmMemoryConfig.ramDbCfg.blocksAllocationMethod);
            }

                        /*
                1.23.Call with lpmDBId [27], numOfBlocks[20],
                lpmMemMode[wrong enum]}.
                Expected: GT_BAD_PARAM.
            */
            lpmDBId = 27;
            if (firstDevSip5==GT_TRUE)
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmDBCreate
                                    (lpmDBId, shadowType, protocolStack, &lpmMemoryConfig),
                                    lpmMemoryConfig.ramDbCfg.lpmMemMode);
            }

    /*
                     1.24.Call with lpmDBId [28], blocksSizeArray[_16K,...,16K,_8K], numOfBlocks[20],
                    blocksAllocationMethod[CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E],
                    lpmMemMode[CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E]}.
                    Expected: GT_OK.
                */

                lpmMemoryConfig.ramDbCfg.lpmMemMode= CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E;

                st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
                if (firstDevSip5==GT_TRUE)
                {
                    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                                                            "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                            "lpmMemoryConfig.ramDbCfg.lpmMemMode = %d",
                                                 lpmDBId, shadowType, protocolStack,
                                                 lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                                 lpmMemoryConfig.ramDbCfg.lpmMemMode);
                }
                else
                {
                    UTF_VERIFY_EQUAL5_STRING_MAC(GT_ALREADY_EXIST, st, "%d, %d, %d, "
                                                            "lpmMemoryConfig.ramDbCfg.numOfBlocks = %d,"
                                                            "lpmMemoryConfig.ramDbCfg.lpmMemMode = %d",
                                                 lpmDBId, shadowType, protocolStack,
                                                 lpmMemoryConfig.ramDbCfg.numOfBlocks,
                                                 lpmMemoryConfig.ramDbCfg.lpmMemMode);
                }


            firstDevSip5=GT_FALSE;
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmDBDelete
(
    IN  GT_U32      lpmDbId
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDBDelete)
{
/*
    1.1. Call cpssDxChIpLpmDBCreate with lpmDbId[9]
              and other valid parameters.
    Expected: GT_OK
    1.2. Call cpssDxChIpLpmDBDelete with lpmDbId[9]
    Expected: GT_OK
    1.3. Call with lpmDBId [9] (already deleted)
    Expected: NOT GT_OK
    1.4. Call with lpmDBId [10] (not created)
    Expected: NOT GT_OK
    1.5. Call prvUtfCreateLpmDBAndVirtualRouterAdd (create def LmpDB and VR)
              and other valid parameters.
    Expected: GT_OK
    1.6. Call with not empty lpmDB [1] (VR created)
    Expected: NOT GT_OK
    1.7. Call cpssDxChIpLpmVirtualRouterDel with lpmDbId[1], vrId[0]
    Expected: GT_OK
    1.8. Call with empty lpmDBId [1] (VR removed)
    Expected: GT_OK.
*/
    GT_STATUS                                     st = GT_OK;
    GT_U8                                         dev = utfFirstDevNumGet();
    GT_U32                                        lpmDBId = 0;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT              shadowType, tmpShadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                    protocolStack;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC  indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC  tcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT            lpmMemoryConfig;


    cpssOsBzero((GT_VOID*)&indexesRange, sizeof(indexesRange));
    cpssOsBzero((GT_VOID*)&tcamLpmManagerCapcityCfg, sizeof(tcamLpmManagerCapcityCfg));
    cpssOsBzero((GT_VOID*)&lpmMemoryConfig, sizeof(lpmMemoryConfig));

    /*
        1.1. Call cpssDxChIpLpmDBCreate with lpmDbId[9]
                  and other valid parameters.
        Expected: GT_OK
    */

    /* Create LpmDB [9] */
    lpmDBId                                             = 9;
    shadowType                                          = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
    protocolStack                                       = CPSS_IP_PROTOCOL_IPV4V6_E;
    indexesRange.firstIndex                             = 0;
    indexesRange.lastIndex                              = 100;
    lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &indexesRange;
    lpmMemoryConfig.tcamDbCfg.partitionEnable           = GT_TRUE;
    tcamLpmManagerCapcityCfg.numOfIpv4Prefixes          = 10;
    tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes  = 10;
    tcamLpmManagerCapcityCfg.numOfIpv6Prefixes          = 10;

    st = prvUtfGetDevShadow(dev, &tmpShadowType);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tmpShadowType);
    if (tmpShadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    }

    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = NULL;
    }
    else
    {
        st = prvUtfIpLpmRamDefaultConfigCalc(dev,&lpmMemoryConfig.ramDbCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);
    }

    st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate: %d", lpmDBId);

    /*
        1.2. Call cpssDxChIpLpmDBDelete with lpmDBId [9]
        Expected: GT_OK
    */
    lpmDBId = 9;

    st = cpssDxChIpLpmDBDelete(lpmDBId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

    /*
        1.3. Call with lpmDBId [9] (already deleted)
        Expected: NOT GT_OK
    */
    st = cpssDxChIpLpmDBDelete(lpmDBId);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

    /*
        1.4. Call with lpmDBId [10] (not created)
        Expected: NOT GT_OK
    */
    lpmDBId = 10;

    st = cpssDxChIpLpmDBDelete(lpmDBId);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

    /*
        1.5. Call prvUtfCreateLpmDBAndVirtualRouterAdd (create def LmpDB and VR)
                  and other valid parameters.
        Expected: GT_OK
    */
    st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

    /*
        1.6. Call with not empty lpmDB [1] (VR created)
        Expected: NOT GT_OK
    */
    lpmDBId = 1;

    st = cpssDxChIpLpmDBDelete(lpmDBId);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

    /*
        1.7. Call cpssDxChIpLpmVirtualRouterDel with lpmDbId[1], vrId[0]
        Expected: GT_OK
    */
    lpmDBId = 1;

    st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmVirtualRouterDel: %d", lpmDBId);

    /*
        1.8. Call with empty lpmDBId [1] (VR removed)
        Expected: GT_OK.
    */
    if(GT_OK == st)
    {
        lpmDBId = 1;

        st = cpssDxChIpLpmDBDelete(lpmDBId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmDBConfigGet
(
    IN  GT_U32                             lpmDBId,
    OUT CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT   *shadowTypePtr,
    OUT CPSS_IP_PROTOCOL_STACK_ENT         *protocolStackPtr,
    OUT CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT *memoryCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDBConfigGet)
{
/*
    1.1. Call with default lpmDBId [0].  and other valid parameters.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmDBCreate with lpmDbId[100]
              and other valid parameters.
    Expected: GT_OK
    1.3. Call with lpmDBId [100].
              and other valid parameters.
    Expected: GT_OK and the same config as was created.
    1.4. Call cpssDxChIpLpmDBDelete with lpmDbId[100]
    Expected: GT_OK
    1.5. Call with wrong lpmDBId [100] (deleted)
              and other valid parameters.
    Expected: NOT GT_OK.
    1.6. Call with wrong shadowTypePtr [NULL].
              and other valid parameters.
    Expected: GT_BAD_PTR.
    1.7. Call with wrong protocolStackPtr [NULL].
              and other valid parameters.
    Expected: GT_BAD_PTR.
    1.8. Call with wrong indexesRangePtr [NULL].
              and other valid parameters.
    Expected: GT_BAD_PTR.
    1.9. Call with wrong tcamLpmManagerCapcityCfgPtr [NULL].
              and other valid parameters.
    Expected: GT_BAD_PTR.
    1.10. Call with tcamManagerHandlerPtr [NULL].
              and other valid parameters.
    Expected: GT_OK.
    1.11. Call with wrong lpmMemoryConfig [NULL].
               and other valid parameters.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U32                                              lpmDBId;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT                   shadowType, tmpShadowType, shadowTypeGet;
    CPSS_IP_PROTOCOL_STACK_ENT                          protocolStack, protocolStackGet;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC        indexesRange, indexesRangeGet;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC        tcamLpmManagerCapcityCfg, tcamLpmManagerCapcityCfgGet;
    GT_BOOL                                             isEqual;
    GT_U32                                              expected_ipv4_prefixes;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT                  lpmMemoryConfig, lpmMemoryConfigGet;
    GT_U8                                               dev = utfFirstDevNumGet();
    GT_U32                                              i;

    cpssOsBzero((GT_VOID*)&lpmMemoryConfigGet, sizeof(lpmMemoryConfigGet));
    cpssOsBzero((GT_VOID*)&lpmMemoryConfig, sizeof(lpmMemoryConfig));

    st = prvUtfGetDevShadow(dev, &tmpShadowType);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tmpShadowType);

    if (tmpShadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        /* Zero structures first */
        cpssOsMemSet(&indexesRange, 0, sizeof(indexesRange));
        cpssOsMemSet(&tcamLpmManagerCapcityCfg, 0, sizeof(tcamLpmManagerCapcityCfg));
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &indexesRange;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
        lpmMemoryConfigGet.tcamDbCfg.indexesRangePtr = &indexesRangeGet;
        lpmMemoryConfigGet.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfgGet;
    }
    else
    {
        lpmMemoryConfig.ramDbCfg.numOfBlocks=0;
        cpssOsMemSet(&lpmMemoryConfig.ramDbCfg.blocksSizeArray, 0, sizeof(lpmMemoryConfig.ramDbCfg.blocksSizeArray));
        lpmMemoryConfig.ramDbCfg.blocksAllocationMethod=CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;

        lpmMemoryConfigGet.ramDbCfg.numOfBlocks=0;
        cpssOsMemSet(&lpmMemoryConfigGet.ramDbCfg.blocksSizeArray, 0, sizeof(lpmMemoryConfigGet.ramDbCfg.blocksSizeArray));
        lpmMemoryConfigGet.ramDbCfg.blocksAllocationMethod=CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;

        lpmMemoryConfigGet.ramDbCfg.lpmMemMode = lpmMemoryConfig.ramDbCfg.lpmMemMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    }

    /*
        1.1. Call with default lpmDBId [0] and other valid parameters.
        Expected: GT_OK.
    */
    lpmDBId = 0;

    st = cpssDxChIpLpmDBConfigGet(lpmDBId, &shadowType, &protocolStack, &lpmMemoryConfig);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

    /*
        1.2. Call cpssDxChIpLpmDBCreate with lpmDbId[100]
                  and other valid parameters.
        Expected: GT_OK
    */
    protocolStack                                       = CPSS_IP_PROTOCOL_IPV4V6_E;
    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        shadowType                                          = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
        indexesRange.firstIndex                             = 50;
        indexesRange.lastIndex                              = 90;
        lpmMemoryConfig.tcamDbCfg.partitionEnable           = GT_TRUE;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr     = NULL;
        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes          = 9;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes  = 8;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes          = 7;
    }
    else
    {
        lpmMemoryConfig.ramDbCfg.numOfBlocks=10;
        for (i=0;i<lpmMemoryConfig.ramDbCfg.numOfBlocks;i++)
        {
            lpmMemoryConfig.ramDbCfg.blocksSizeArray[i]=_12K;
        }
        lpmMemoryConfig.ramDbCfg.blocksAllocationMethod=CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E;
    }
    lpmDBId = 100;

    st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack,
                               &lpmMemoryConfig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate: %d", lpmDBId);

    /*
        1.3. Call with lpmDBId [100].
                  and other valid parameters.
        Expected: GT_OK and the same config as was created.
    */
    st = cpssDxChIpLpmDBConfigGet(lpmDBId, &shadowTypeGet, &protocolStackGet,
                                  &lpmMemoryConfigGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

    /* check shadowType and protocolStack */
    UTF_VERIFY_EQUAL2_STRING_MAC(shadowType, shadowTypeGet,
                           "get another shadowType than was set: %d",
                            shadowType, shadowTypeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(protocolStack, protocolStackGet,
                           "get another protocolStack than was set: %d, %d",
                            protocolStack, protocolStackGet);

    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        /* check indexesRange */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&indexesRange,
                                     (GT_VOID*)&indexesRangeGet,
                   sizeof (CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                "get another indexesRange than was set: %d", lpmDBId);

        /* check tcamLpmManagerCapcityCfg fields */
        UTF_VERIFY_EQUAL2_STRING_MAC(
                tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes,
                tcamLpmManagerCapcityCfgGet.numOfIpv4McSourcePrefixes,
                   "get another numOfIpv4McSourcePrefixes than was set: %d, %d",
                        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes,
                        tcamLpmManagerCapcityCfgGet.numOfIpv4McSourcePrefixes);

        UTF_VERIFY_EQUAL2_STRING_MAC(
                tcamLpmManagerCapcityCfg.numOfIpv6Prefixes,
                tcamLpmManagerCapcityCfgGet.numOfIpv6Prefixes,
                   "get another numOfIpv6Prefixes than was set: %d, %d",
                        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes,
                        tcamLpmManagerCapcityCfgGet.numOfIpv6Prefixes);

        /* calculate num of expected ipv4 prefixes */
        expected_ipv4_prefixes =
        /* total num of entries */
        (indexesRange.lastIndex - indexesRange.firstIndex + 1)*4
            -
        /* total not ipv4 entries */
        (tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes  +
         tcamLpmManagerCapcityCfg.numOfIpv6Prefixes * 4);

        /* check result */
        UTF_VERIFY_EQUAL2_STRING_MAC(
                expected_ipv4_prefixes,
                tcamLpmManagerCapcityCfgGet.numOfIpv4Prefixes,
                   "get another numOfIpv4Prefixes than expected: %d, %d",
                        expected_ipv4_prefixes,
                        tcamLpmManagerCapcityCfgGet.numOfIpv4Prefixes);
    }
    else
    {
        /* check lpmMemoryConfig.ramDbCfg fields */
        UTF_VERIFY_EQUAL2_STRING_MAC(
                lpmMemoryConfig.ramDbCfg.numOfBlocks,
                lpmMemoryConfigGet.ramDbCfg.numOfBlocks,
                   "get another lpmMemoryConfig.ramDbCfg.numOfBlocks than was set: %d, %d",
                        lpmMemoryConfig.ramDbCfg.numOfBlocks,
                        lpmMemoryConfigGet.ramDbCfg.numOfBlocks);

        /* check indexesRange */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&lpmMemoryConfig.ramDbCfg.blocksSizeArray,
                                     (GT_VOID*)&lpmMemoryConfigGet.ramDbCfg.blocksSizeArray,
                   lpmMemoryConfig.ramDbCfg.numOfBlocks * sizeof (GT_U32))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                "get another lpmMemoryConfig.ramDbCfg.blocksSizeArray than was set: %d", lpmDBId);

        UTF_VERIFY_EQUAL2_STRING_MAC(
                lpmMemoryConfig.ramDbCfg.blocksAllocationMethod,
                lpmMemoryConfigGet.ramDbCfg.blocksAllocationMethod,
                   "get another lpmMemoryConfig.ramDbCfg.blocksAllocationMethod than was set: %d, %d",
                        lpmMemoryConfig.ramDbCfg.blocksAllocationMethod,
                        lpmMemoryConfigGet.ramDbCfg.blocksAllocationMethod);

         UTF_VERIFY_EQUAL2_STRING_MAC(
                lpmMemoryConfig.ramDbCfg.lpmMemMode,
                lpmMemoryConfigGet.ramDbCfg.lpmMemMode,
                   "get another lpmMemoryConfig.ramDbCfg.lpmMemMode than was set: %d, %d",
                        lpmMemoryConfig.ramDbCfg.lpmMemMode,
                        lpmMemoryConfigGet.ramDbCfg.lpmMemMode);

    }

    /*
        1.4. Call cpssDxChIpLpmDBDelete with lpmDbId[100]
        Expected: GT_OK
    */
    st = cpssDxChIpLpmDBDelete(lpmDBId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

    /*
        1.5. Call with wrong lpmDBId [100] (deleted)
                  and other valid parameters.
        Expected: NOT GT_OK.
    */
    st = cpssDxChIpLpmDBConfigGet(lpmDBId, &shadowType, &protocolStack, &lpmMemoryConfig);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

    /*
        1.6. Call with wrong shadowTypePtr [NULL]
                  and other valid parameters.
        Expected: GT_BAD_PTR.
    */
    lpmDBId = 0;

    st = cpssDxChIpLpmDBConfigGet(lpmDBId, NULL, &protocolStack, &lpmMemoryConfig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, shadowTypePtr = NULL", lpmDBId);

    /*
        1.7. Call with wrong protocolStackPtr [NULL]
                  and other valid parameters.
        Expected: GT_BAD_PTR.
    */
    st = cpssDxChIpLpmDBConfigGet(lpmDBId, &shadowType, NULL, &lpmMemoryConfig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, protocolStackPtr = NULL", lpmDBId);

    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        /*
            1.8. Call with wrong indexesRangePtr [NULL]
                  and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr = NULL;
        st = cpssDxChIpLpmDBConfigGet(lpmDBId, &shadowType, &protocolStack, &lpmMemoryConfig);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, indexesRangePtr = NULL", lpmDBId);

        /*
            1.9. Call with wrong tcamLpmManagerCapcityCfgPtr [NULL]
                      and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &indexesRange;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = NULL;
        st = cpssDxChIpLpmDBConfigGet(lpmDBId, &shadowType, &protocolStack, &lpmMemoryConfig);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, partitionEnablePtr = NULL", lpmDBId);

        /*
            1.10. Call with tcamManagerHandlerPtr [NULL]
                       and other valid parameters.
            Expected: GT_OK.
        */
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = NULL;
        st = cpssDxChIpLpmDBConfigGet(lpmDBId, &shadowType, &protocolStack, &lpmMemoryConfig);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                       "%d, tcamManagerHandlerPtr = NULL", lpmDBId);
    }
    /*
        1.11. Call with wrong lpmMemoryConfig [NULL].
        Expected: GT_BAD_PTR.
    */
    st = cpssDxChIpLpmDBConfigGet(lpmDBId, &shadowType, &protocolStack, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, lpmMemoryConfig = NULL", lpmDBId);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmDBDevListAdd
(
    IN GT_U32   lpmDBId,
    IN GT_U8    devList[],
    IN GT_U32   numOfDevs
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDBDevListAdd)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0 / 1],
                     protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
                     indexesRangePtr{ firstIndex [0/ 200],
                                      lastIndex [100/ 1000]},
                     partitionEnable[GT_TRUE],
                     tcamLpmManagerCapcityCfgPtr{numOfIpv4Prefixes [10],
                                                 numOfIpv4McSourcePrefixes [5],
                                                 numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call with lpmDBId [1], devList [dev1, dev2 ... devN], numOfDevs[numOfDevs].
    Expected: GT_OK.
    1.3. Call cpssDxChIpLpmDBDevListGet with numOfDevs which is < than was set
         by cpssDxChIpLpmDBDevListAdd
    Expected: not GT_OK and same numOfDevs than was set.
    1.4. Call cpssDxChIpLpmDBDevListGet with numOfDevs which is = than was set
         by cpssDxChIpLpmDBDevListAdd
    Expected: GT_OK and same devList.
    1.5. Call with not valid LPM DB id lpmDBId [10], devList [10], numOfDevs[1].
    Expected: NOT GT_OK.
    1.6. Call with lpmDBId [1], devList [{PRV_CPSS_MAX_PP_DEVICES_CNS}], numOfDevs[1].
    Expected: NOT GT_OK.
    1.7. Call with lpmDBId [1], devList [NULL], numOfDevs[1]
    Expected: GT_BAD_PTR
    1.8. Call with lpmDBId [11], devList [1], numOfDevs[1], numOfBlocks = PRV_CPSS_DXCH_LPM_RAM_MAX_NUM_OF_MEMORY_BLOCKS_CNS/2
    Expected: GT_OK
    1.9. Call with lpmDBId [12], devList [1], numOfDevs[1], numOfBlocks = PRV_CPSS_DXCH_LPM_RAM_MAX_NUM_OF_MEMORY_BLOCKS_CNS
         blocksSizeArray[i] *=2;
    Expected: GT_NO_RESOURCE
     1.10. Call with lpmDBId [44], devList [1], numOfDevs[1].Try to add device that have LPM memory mode
                different from LPM memory mode configured at LPM db

            Expected: GT_FAIL
 */

    GT_STATUS st        = GT_OK;
    GT_U8     dev, devNum;

    GT_U32    lpmDBId   = 0;
    GT_U8     devList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32    numOfDevs = 0;
    GT_U8     devListGet[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32    numOfDevsGet = 0;
    GT_U32    i = 0;
    CPSS_DXCH_LPM_RAM_CONFIG_STC appDemoRamDbCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT  lpmMemoryCfg;
    PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT deviceLpmMemoryMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&lpmMemoryCfg, sizeof(lpmMemoryCfg));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfDevs = 0;

        /*
            1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0 / 1],
                         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
                         indexesRangePtr{ firstIndex [0/ 200],
                                          lastIndex [100/ 1000]},
                          partitionEnable[GT_TRUE],
                          tcamLpmManagerCapcityCfgPtr{numOfIpv4Prefixes [10],
                                                      numOfIpv4McSourcePrefixes [5],
                                                      numOfIpv6Prefixes [10]} to create LPM DB.
            Expected: GT_OK.
        */
        st = prvUtfCreateDefaultLpmDB(dev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateDefaultLpmDB");

        /*
            1.2. Call with lpmDBId [1], devList [dev1, dev2 ... devN], numOfDevs[numOfDevs].
            Expected: GT_OK.
        */
        lpmDBId   = 0;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            devList[numOfDevs++] = devNum;
        }


        /* for lpmDBId = 0 dev 0 was already added in the init phase,
           so we will get GT_ALREADY_EXIST */
        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_ALREADY_EXIST, st, "%d, numOfDevs = %d", lpmDBId, numOfDevs);

        /*  1.3.  */
        numOfDevsGet = numOfDevs - 1;
        st = cpssDxChIpLpmDBDevListGet(lpmDBId, &numOfDevsGet,devListGet);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId);

        UTF_VERIFY_EQUAL1_STRING_MAC(numOfDevs, numOfDevsGet,
                                "get another numOfDevs than was set: %d", dev);

        /*  1.4.  */
        numOfDevsGet = numOfDevs;
        st = cpssDxChIpLpmDBDevListGet(lpmDBId, &numOfDevsGet,devListGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId);

        UTF_VERIFY_EQUAL1_STRING_MAC(numOfDevs, numOfDevsGet,
                                "get another numOfDevs than was set: %d", dev);

        for(i=0; i<numOfDevsGet; i++)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(devList[i], devListGet[i],
                            "get another devList[%d] than was set: %d", i, dev);
        }

        /*
            1.5. Call with not valid LPM DB id lpmDBId [10], devList [10], numOfDevs[1].
            Expected: NOT GT_OK.
        */
        lpmDBId    = 10;
        devList[0] = 10;
        numOfDevs  = 1;

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.6. Call with lpmDBId [1], devList [{PRV_CPSS_MAX_PP_DEVICES_CNS}], numOfDevs[1].
            Expected: NOT GT_OK.
        */
        lpmDBId    = 1;
        devList[0] = PRV_CPSS_MAX_PP_DEVICES_CNS;
        numOfDevs  = 1;

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, devList[0] = %d",
                                                     lpmDBId, devList[0]);

        devList[0] = 0;

        /*
            1.7. Call with lpmDBId [1], devList [NULL], numOfDevs[1]
            Expected: GT_BAD_PTR
        */
        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, NULL, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, devList[] = NULL", lpmDBId);


        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            lpmDBId    = 11;

            st = appDemoBc2IpLpmRamDefaultConfigCalc(dev,
                                                 PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->moduleCfg.ip.maxNumOfPbrEntries,
                                                 &appDemoRamDbCfg);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "appDemoBc2IpLpmRamDefaultConfigCalc");

            lpmMemoryCfg.ramDbCfg.numOfBlocks = appDemoRamDbCfg.numOfBlocks/2;
            for (i=0;i<lpmMemoryCfg.ramDbCfg.numOfBlocks;i++)
            {
                lpmMemoryCfg.ramDbCfg.blocksSizeArray[i] = appDemoRamDbCfg.blocksSizeArray[i];
            }
            lpmMemoryCfg.ramDbCfg.blocksAllocationMethod = appDemoRamDbCfg.blocksAllocationMethod;



            st = cpssDxChIpLpmDBCreate(lpmDBId, CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E, CPSS_IP_PROTOCOL_IPV4V6_E, &lpmMemoryCfg);
            if(st!=GT_ALREADY_EXIST)
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate");

            devList[0] = dev;
            numOfDevs  = 1;

            /*
            1.8. Call with lpmDBId [11], devList [1], numOfDevs[1], numOfBlocks = PRV_CPSS_DXCH_LPM_RAM_MAX_NUM_OF_MEMORY_BLOCKS_CNS/2
            Expected: GT_OK
            */
            st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, devList[] = NULL", lpmDBId);

            lpmDBId    = 12;

            lpmMemoryCfg.ramDbCfg.numOfBlocks = appDemoRamDbCfg.numOfBlocks;
            for (i=0;i<lpmMemoryCfg.ramDbCfg.numOfBlocks;i++)
            {
                lpmMemoryCfg.ramDbCfg.blocksSizeArray[i] = appDemoRamDbCfg.blocksSizeArray[i]*2;
            }

            st = cpssDxChIpLpmDBCreate(lpmDBId, CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E, CPSS_IP_PROTOCOL_IPV4V6_E, &lpmMemoryCfg);
            if(st!=GT_ALREADY_EXIST)
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate");

            /*
            1.9. Call with lpmDBId [12], devList [1], numOfDevs[1], numOfBlocks = PRV_CPSS_DXCH_LPM_RAM_MAX_NUM_OF_MEMORY_BLOCKS_CNS
                 blocksSizeArray[i] *=2;
            Expected: GT_NO_RESOURCE
            */
            st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_RESOURCE, st, "%d, devList[] = NULL", lpmDBId);

                /*
            1.10. Call with lpmDBId [44], devList [1], numOfDevs[1].Try to add device that have LPM memory mode
                different from LPM memory mode configured at LPM db

            Expected: GT_FAIL
            */

            lpmDBId    = 44;

            for (i=0;i<lpmMemoryCfg.ramDbCfg.numOfBlocks;i++)
            {
                lpmMemoryCfg.ramDbCfg.blocksSizeArray[i] = appDemoRamDbCfg.blocksSizeArray[i];
            }

            deviceLpmMemoryMode = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.lpm.lpmMemMode;

            if(deviceLpmMemoryMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
            {
              lpmMemoryCfg.ramDbCfg.lpmMemMode =  CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
            }
            else
            {
               lpmMemoryCfg.ramDbCfg.lpmMemMode =  CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E;
            }

            st = cpssDxChIpLpmDBCreate(lpmDBId, CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E, CPSS_IP_PROTOCOL_IPV4V6_E, &lpmMemoryCfg);

            if(st == GT_ALREADY_EXIST)
            {
                st = cpssDxChIpLpmDBDelete(lpmDBId);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBDelete");

                st = cpssDxChIpLpmDBCreate(lpmDBId, CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E, CPSS_IP_PROTOCOL_IPV4V6_E, &lpmMemoryCfg);

            }
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate");

            st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, lpmDBId);



        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmDBDevsListRemove
(
    IN GT_U32   lpmDBId,
    IN GT_U8    devList[],
    IN GT_U32   numOfDevs
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDBDevsListRemove)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0 / 1],
             protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
             indexesRangePtr{ firstIndex [0/ 200],
                              lastIndex [100/ 1000]},
             partitionEnable[GT_TRUE],
             tcamLpmManagerCapcityCfgPtr{numOfIpv4Prefixes [10],
                                         numOfIpv4McSourcePrefixes [5],
                                         numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmDBDevListAdd with lpmDBId [0 / 1],
                         devList [0, 1, 2, 3, 4] / [5, 6, 7],
                         numOfDevs[5] / [3] to adds devices to an existing LPM DB.
    Expected: GT_OK.
    1.3. Call with lpmDBId [0 / 1], devList [0, 1, 2, 3, 4] / [5, 6, 7], numOfDevs[3].
    Expected: GT_OK.
    1.4. Call with lpmDBId [0], devList [3, 4], numOfDevs[2].
    Expected: GT_OK.
    1.5. Call with lpmDBId [0], devList [0, 1, 2, 3, 4], numOfDevs[5] (already removed).
    Expected: NOT GT_OK.
    1.6. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.7. Call with lpmDBId [1], devList [{PRV_CPSS_MAX_PP_DEVICES_CNS}], numOfDevs[1].
    Expected: NOT GT_OK.
    1.8. Call with lpmDBId [1], devList [NULL], numOfDevs[1]
    Expected: GT_BAD_PTR
*/
    GT_STATUS st        = GT_OK;
    GT_U8     dev, devNum;

    GT_U32    lpmDBId   = 0;
    GT_U8     devList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32    numOfDevs = 0;
    GT_BOOL   firstDev=GT_TRUE;/* used for multi device system.
                                  shadow will be created only for
                                  first device call, all other calls should
                                  return GT_ALREADY_EXIST */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfDevs = 0;
        /*
            1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0 / 1],
                     protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
                     indexesRangePtr{ firstIndex [0/ 200],
                                      lastIndex [100/ 1000]},
                      partitionEnable[GT_TRUE],
                      tcamLpmManagerCapcityCfgPtr{numOfIpv4Prefixes [10],
                                                  numOfIpv4McSourcePrefixes [5],
                                                  numOfIpv6Prefixes [10]} to create LPM DB.
            Expected: GT_OK.
        */
        st = prvUtfCreateDefaultLpmDB(dev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateDefaultLpmDB");

        /*
            1.2. Call cpssDxChIpLpmDBDevListAdd with lpmDBId [0 / 1],
                                 devList [0, 1, 2, 3, 4] / [5, 6, 7],
                                 numOfDevs[5] / [3] to adds devices to an existing LPM DB.
            Expected: GT_OK.
        */
        lpmDBId = 0;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            devList[numOfDevs++] = devNum;
        }

        /* for lpmDBId = 0 dev 0 was already added in the init phase,
           so we will get GT_ALREADY_EXIST,
           unless this is not the first device in the loop --> in this case the lpmDBId = 0
           was deleted in next stages and we will get GT_OK*/
        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
        if(firstDev==GT_TRUE)
        {
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_ALREADY_EXIST, st,
            "cpssDxChIpLpmDBDevListAdd: %d, numOfDevs = %d", lpmDBId, numOfDevs);
        }
        else
        {
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChIpLpmDBDevListAdd: %d, numOfDevs = %d", lpmDBId, numOfDevs);
        }

        lpmDBId = 1;

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChIpLpmDBDevListAdd: %d, numOfDevs = %d", lpmDBId, numOfDevs);

        /*
            1.3. Call with lpmDBId [0 / 1], devList [0, 1, 2, 3, 4] / [5, 6, 7],
                           numOfDevs[3].
            Expected: GT_OK.
        */
        lpmDBId   = 0;

        st = cpssDxChIpLpmDBDevsListRemove(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId, numOfDevs);

        lpmDBId = 1;

        st = cpssDxChIpLpmDBDevsListRemove(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId, numOfDevs);

        /*
            1.5. Call with lpmDBId [0],
                           devList [0, 1, 2, 3, 4],
                           numOfDevs[5] (already removed).
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmDBDevsListRemove(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                        "%d, numOfDevs = %d", lpmDBId, numOfDevs);

        /*
            1.6. Call with not valid LPM DB id lpmDBId [10]
                           and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmDBDevsListRemove(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.7. Call with lpmDBId [1], devList [{PRV_CPSS_MAX_PP_DEVICES_CNS}],
                           numOfDevs[1].
            Expected: NOT GT_OK.
        */
        lpmDBId    = 1;
        devList[0] = PRV_CPSS_MAX_PP_DEVICES_CNS;
        numOfDevs  = 1;

        st = cpssDxChIpLpmDBDevsListRemove(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                    "%d, devList[0] = %d", lpmDBId, devList[0]);

        devList[0] = 0;

        /*
            1.8. Call with lpmDBId [1], devList [NULL], numOfDevs[1]
            Expected: GT_BAD_PTR
        */
        st = cpssDxChIpLpmDBDevsListRemove(lpmDBId, NULL, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, devList[] = NULL", lpmDBId);

        firstDev=GT_FALSE;
    }
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDBDevListGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0 / 1],
                     protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
                     indexesRangePtr{ firstIndex [0/ 200],
                                      lastIndex [100/ 1000]},
                     partitionEnable[GT_TRUE],
                     tcamLpmManagerCapcityCfgPtr{numOfIpv4Prefixes [10],
                                                 numOfIpv4McSourcePrefixes [5],
                                                 numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmDBDevListADD with lpmDBId [0], devList [dev1, dev2 ... devN], numOfDevs
    Expected: GT_OK.
    1.3. Call cpssDxChIpLpmDBDevListGet with lpmDBId [0], not NULL devListArray
         and not NULL numOfDevsPtr with value[numOfDevs+1] which > than was set in 1.2.
    Expected: GT_OK.
    1.4. Call cpssDxChIpLpmDBDevListGet with lpmDBId [0], not NULL devListArray
         and not NULL numOfDevsPtr with value[numOfDevs-1] which < than was set in 1.2.
    Expected: not GT_OK.
    1.5. Call with not valid LPM DB id lpmDBId [10], devList [10], numOfDevs[1].
    Expected: NOT GT_OK.
    1.6. Call with NULL numOfDevsPtr
    Expected: GT_BAD_PTR.
    1.7. Call with NULL devListArray
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st        = GT_OK;
    GT_U8     dev, devNum;

    GT_U32    lpmDBId   = 0;
    GT_U8     devList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32    numOfDevs = 0;
    GT_U32    numOfDevsGet = 0;
    GT_BOOL   firstDev=GT_TRUE;/* used for multi device system.
                                 shadow will be created only for
                                 first device call, all other calls should
                                 return GT_ALREADY_EXIST */


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfDevs = 0;
        /*  1.1. */
        st = prvUtfCreateDefaultLpmDB(dev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateDefaultLpmDB");

        /*  1.2. */
        lpmDBId   = 0;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            devList[numOfDevs++] = devNum;
        }

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
        if(firstDev==GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId, numOfDevs);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_ALREADY_EXIST, st, "%d, numOfDevs = %d", lpmDBId, numOfDevs);
        }
        /*  1.3. */
        numOfDevsGet = numOfDevs+1;
        st = cpssDxChIpLpmDBDevListGet(lpmDBId, &numOfDevsGet , devList);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId);

        /*  1.4. */
        numOfDevsGet = numOfDevs-1;
        st = cpssDxChIpLpmDBDevListGet(lpmDBId, &numOfDevsGet , devList);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId);

        /*  1.5. */
        lpmDBId   = 10;
        st = cpssDxChIpLpmDBDevListGet(lpmDBId, &numOfDevsGet , devList);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId);
        lpmDBId   = 0;

        /*  1.6. */
        st = cpssDxChIpLpmDBDevListGet(lpmDBId , NULL , devList);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numOfDevs = %d", lpmDBId);

        /*  1.7. */
        st = cpssDxChIpLpmDBDevListGet(lpmDBId, &numOfDevsGet ,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numOfDevs = %d", lpmDBId);

        firstDev=GT_FALSE;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmVirtualRouterAdd
(
    IN GT_U32                                 lpmDBId,
    IN GT_U32                                 vrId,
    IN CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defUcNextHopInfoPtr,
    IN CPSS_DXCH_IP_LTT_ENTRY_STC             *defMcRouteLttEntryPtr,
    IN CPSS_IP_PROTOCOL_STACK_ENT             protocolStack
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmVirtualRouterAdd)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate to create default LPM DB configuration.
         See cpssDxChIpLpmDBCreate description.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmDBDevListAdd with lpmDBId [0 / 1],
         devList [0, 1, 2, 3, 4] / [5, 6, 7],
         numOfDevs[5] / [3] to adds devices to an existing LPM DB.
    Expected: GT_OK
    1.3. Call with lpmDBId [0], vrId[0], defUcNextHopInfo filled with
         default configuration, defMcRouteLttEntry [NULL] (support only unicast),
         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_ALREADY_EXIST.
    1.4. Call with lpmDBId [1], vrId[0], defUcNextHopInfo filled with
         default configuration, defMcRouteLttEntry filled with default
         configuration (support both: unicast and multicast) and
         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK .
    1.5. Call cpssDxChIpLpmVirtualRouterGet with the same params.
    Expected: GT_OK and the same values.
    1.6. Call with lpmDBId [3], vrId[0], defUcNextHopInfo filled with
         default configuration, defMcRouteLttEntry [NULL]
         (support only unicast) and protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK .
    1.7. Call cpssDxChIpLpmVirtualRouterGet with the same params.
    Expected: GT_OK and the same values.
    1.8. Call with lpmDBId [4], vrId[0], defUcNextHopInfo [NULL]
         defMcRouteLttEntry filled with default configuration
         (support only multicast) and
         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK .
    1.9. Call cpssDxChIpLpmVirtualRouterGet with the same params.
    Expected: GT_OK and the same values.
    1.10. Call with lpmDBId [0], vrId[0],
             wrong enum values defUcNextHopInfoPtr->pclIpUcAction.pktCmd,
             defMcRouteLttEntry[NULL] (support only UC),
             protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_BAD_PARAM.
    1.11. Call with lpmDBId [1], vrId[0],
            wrong enum values defUcNextHopInfoPtr->ipLttEntry.routeType
            defMcRouteLttEntry {routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
            numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
            sipSaCheckMismatchEnable[GT_TRUE],
            ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
            protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_BAD_PARAM.
    1.12. Call with correct numOfPaths [5/7] ipv4/ipv6 for unicast and multicast.
    Expected: GT_OK.
    1.13. Call with correct numOfPaths [8/63] ipv4/ipv6 for unicast and multicast.
    Expected: GT_OK for lion and above and not GT_OK for other.
    1.14. Call with wrong numOfPaths [64] (out of range).
    Check ipv4/ipv6 for unicast and multicast.
    Expected: NOT GT_OK
    1.15. Call with only supportIpv4Mc enabled (other disabled).
            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportIpv4Mc = GT_TRUE;
            vrConfigInfo.supportIpv6Uc = GT_FALSE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;
    Expected: GT_OK.
    1.15.1 Call with only supportFcoe enabled (other disabled) and lpmDBId = 6 (created with FCOE enabled).
            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportIpv4Mc = GT_FALSE;
            vrConfigInfo.supportIpv6Uc = GT_FALSE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;
            vrConfigInfo.supportFcoe   = GT_TRUE;
    Expected: GT_OK.
    1.16. Call with all types disabled.
            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportIpv4Mc = GT_FALSE;
            vrConfigInfo.supportIpv6Uc = GT_FALSE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;
    Expected: NOT GT_OK.
    1.17. Call with lpmDBId [1], vrId[0],
            out of range defUcNextHopInfoPtr->ipLttEntry.routeEntryBaseIndex [4096]
            defMcRouteLttEntry {routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                               numOfPaths[0],
                               routeEntryBaseIndex[0],
                               ucRPFCheckEnable[GT_TRUE],
                               sipSaCheckMismatchEnable[GT_TRUE],
                               ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
            protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: NOT GT_OK.
    1.18. Call with lpmDBId [1], vrId[0],
            out of range defUcNextHopInfoPtr->ipLttEntry.
            ipv6MCGroupScopeLevel [wrong enum values]
            defMcRouteLttEntry {routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                   numOfPaths[0],
                                   routeEntryBaseIndex[0],
                                   ucRPFCheckEnable[GT_TRUE],
                                   sipSaCheckMismatchEnable[GT_TRUE],
                                   ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
            protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_BAD_PARAM.
    1.19. Call with not valid LPM DB id lpmDBId [10].
    Expected: NOT GT_OK.
    1.20. Call with lpmDBId [0],
                invalid defUcNextHopInfoPtr->pclIpUcAction.mirror.cpuCode
                [CPSS_NET_MIRROR_PCL_RST_FIN_E] (Tiger only),
                and other parameters the same as in 1.2.
    Expected: NON GT_OK.
    1.21. Call with lpmDBId [0],
                     wrong enum values defUcNextHopInfoPtr->pclIpUcAction.mirror.cpuCode
                     and other parameters the same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.22. Call with lpmDBId [0],
             defUcNextHopInfoPtr->pclIpUcAction.matchCounter
             [{enableMatchCount = GT_TRUE; matchCounterIndex = 33 (out of range)}]
             and other parameters the same as in 1.2.
    Expected: NON GT_OK.
    1.23. Call with lpmDBId [0],
              defUcNextHopInfoPtr->pclIpUcAction.qos.ingress.modifyDscp [wrong enum values]
              and other parameters the same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.24. Call with lpmDBId [0],
                     defUcNextHopInfoPtr->pclIpUcAction.qos.ingress.modifyUp [wrong enum values]
                     and other parameters the same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.25. Call with lpmDBId [0],
             defUcNextHopInfoPtr->pclIpUcAction.qos.ingress.profilePrecedence
             [wrong enum values]
             and other parameters the same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.26. Call with lpmDBId [0],
             defUcNextHopInfoPtr->pclIpUcAction.redirect.redirectCmd [wrong enum values]
             and other parameters the same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.27. Call with lpmDBId [0],
          defUcNextHopInfoPtr->pclIpUcAction.policer[{policerEnable = GT_TRUE;
                                                      policerId = 0xFFFFFFFF}]
                     and other parameters the same as in 1.2.
    Expected: NON GT_OK.
    1.28. Call with lpmDBId [0],
             defUcNextHopInfoPtr->pclIpUcAction.vlan
                        [{egressTaggedModify = GT_TRUE;
                          modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E,
                          vlanId = 4096}]
             and other parameters the same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.29. Call with lpmDBId [0],
             defUcNextHopInfoPtr->pclIpUcAction.vlan[{egressTaggedModify = GT_FALSE;
                                                      modifyVlan = wrong enum values}]
             and other parameters the same as in 1.2.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                              st            = GT_OK;
    GT_U8                                  dev, devNum;

    GT_U32                                 lpmDBId       = 0;
    GT_U32                                 vrId          = 0;

    GT_U8                                  devList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                                 numOfDevs = 0;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfoGet;
    GT_BOOL                                isEqual = GT_FALSE;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT      shadowType;
    GT_BOOL                                firstDev=GT_TRUE;/* used for multi device system.
                                                               shadow will be created only for
                                                               first device call, all other calls should
                                                               return GT_ALREADY_EXIST */


    cpssOsBzero((GT_VOID*) &vrConfigInfo, sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));
    cpssOsBzero((GT_VOID*) &vrConfigInfoGet, sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfDevs = 0;

        st = prvUtfGetDevShadow(dev, &shadowType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        /*
            1.1. Call cpssDxChIpLpmDBCreate to create default LPM DB configuration.
                 See cpssDxChIpLpmDBCreate description.
            Expected: GT_OK.
        */
        st = prvUtfCreateDefaultLpmDB(dev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateDefaultLpmDB");

        /*
            1.2. Call cpssDxChIpLpmDBDevListAdd with lpmDBId [0 / 1],
                 devList [0, 1, 2, 3, 4] / [5, 6, 7],
                 numOfDevs[5] / [3] to adds devices to an existing LPM DB.
            Expected: GT_OK.
        */
        lpmDBId = 0;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            devList[numOfDevs++] = devNum;
        }

        /* since the shadow configuration is done not per device but per system,
           all following commands will be as expected only for the first device in the loop,
           then we will start to get not as expected result, such as GT_ALREADY_EXIST,
           GT_NOT_FOUND or un-equal parameters in the get functions */
        if (firstDev==GT_TRUE)
        {

            /* for lpmDBId = 0 dev 0 was already added in the init phase,
               so we will get GT_ALREADY_EXIST */
            st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_ALREADY_EXIST, st,
                "cpssDxChIpLpmDBDevListAdd: %d, numOfDevs = %d", lpmDBId, numOfDevs);

            lpmDBId = 1;

            st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpLpmDBDevListAdd: %d, numOfDevs = %d", lpmDBId, numOfDevs);
            /*
                1.3. Call with lpmDBId [0], vrId[0], defUcNextHopInfo filled with
                     default configuration, defMcRouteLttEntry [NULL] (support only unicast),
                     protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                Expected: GT_ALREADY_EXIST.
            */
            lpmDBId = 0;
            vrId    = 0;
            vrConfigInfo.supportIpv4Mc = GT_FALSE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportIpv6Uc = GT_TRUE;

            vrConfigInfo.supportFcoe   = GT_FALSE;

            /* Fill defUcNextHopInfoPtr structure */
            if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
            {
                prvUtfSetDefaultActionEntry(&vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction);
                prvUtfSetDefaultActionEntry(&vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction);
            }
            else
            {
                prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry);
                prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry);
            }

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_ALREADY_EXIST, st,"%d, %d", lpmDBId, vrId);

            /*
                1.4. Call with lpmDBId [1], vrId[0], defUcNextHopInfo filled with
                     default configuration, defMcRouteLttEntry filled with default
                     configuration (support both: unicast and multicast) and
                     protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                Expected: GT_OK .
            */
            lpmDBId = 1;
            vrConfigInfo.supportIpv4Mc = !(prvUtfIsPbrModeUsed());
            vrConfigInfo.supportIpv6Mc = !(prvUtfIsPbrModeUsed());
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportIpv6Uc = GT_TRUE;

            vrConfigInfo.supportFcoe   = GT_FALSE;

            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv4McRouteLttEntry);
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv6McRouteLttEntry);

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%d, %d", lpmDBId, vrId);
            /*
                1.5. Call cpssDxChIpLpmVirtualRouterGet with the same params.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChIpLpmVirtualRouterGet (lpmDBId, vrId, &vrConfigInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%d, %d", lpmDBId, vrId);
            if (GT_OK == st)
            {
                /*check supportIpv4Uc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, vrConfigInfoGet.supportIpv4Uc,
                                    "supportIpv4Uc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv4Uc);

                /*check vrConfigInfoGet.defIpv4UcNextHopInfo.ipLttEntry*/
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry,
                                             (GT_VOID*)&vrConfigInfoGet.defIpv4UcNextHopInfo.ipLttEntry,
                           sizeof (CPSS_DXCH_IP_LTT_ENTRY_STC))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                        "get another defIpv4UcNextHopInfo than was set: %d", dev);
                /*check supportIpv6Uc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, vrConfigInfoGet.supportIpv6Uc,
                                    "supportIpv6Uc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv6Uc);

                /*check vrConfigInfo.defIpv6UcNextHopInfoGet.ipLttEntry*/
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry,
                                             (GT_VOID*)&vrConfigInfoGet.defIpv6UcNextHopInfo.ipLttEntry,
                                             sizeof (CPSS_DXCH_IP_LTT_ENTRY_STC))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                        "get another defIpv6UcNextHopInfo than was set: %d", dev);

                /*check supportIpv4Mc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(!prvUtfIsPbrModeUsed(), vrConfigInfoGet.supportIpv4Mc,
                                    "supportIpv4Mc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv4Mc);

                if (GT_FALSE == prvUtfIsPbrModeUsed())
                {
                    /*check vrConfigInfoGet.defIpv4McRouteLttEntry*/
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*)&vrConfigInfo.defIpv4McRouteLttEntry,
                                                 (GT_VOID*)&vrConfigInfoGet.defIpv4McRouteLttEntry,
                                                 sizeof (CPSS_DXCH_IP_LTT_ENTRY_STC))) ? GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                            "get another defIpv4McRouteLttEntry than was set: %d", dev);
                }

                /*check supportIpv6Mc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(!prvUtfIsPbrModeUsed(), vrConfigInfoGet.supportIpv6Mc,
                                    "supportIpv6Mc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv6Mc);

                if (GT_FALSE == prvUtfIsPbrModeUsed())
                {
                    /*check vrConfigInfoGet.defIpv4McRouteLttEntry*/
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*)&vrConfigInfo.defIpv6McRouteLttEntry,
                                                 (GT_VOID*)&vrConfigInfoGet.defIpv6McRouteLttEntry,
                                                 sizeof (CPSS_DXCH_IP_LTT_ENTRY_STC))) ? GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                            "get another defIpv6McRouteLttEntry than was set: %d", dev);
                }
            }

            /*
                1.6. Call with lpmDBId [3], vrId[0], defUcNextHopInfo filled with
                     default configuration, defMcRouteLttEntry [NULL]
                     (support only unicast) and protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                Expected: GT_OK .
            */
            lpmDBId = 3;
            vrConfigInfo.supportIpv4Mc = GT_FALSE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%d, %d", lpmDBId, vrId);
            /*
                1.7. Call cpssDxChIpLpmVirtualRouterGet with the same params.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChIpLpmVirtualRouterGet (lpmDBId, vrId, &vrConfigInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%d, %d", lpmDBId, vrId);

            if (GT_OK == st)
            {
                /*check supportIpv4Uc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, vrConfigInfoGet.supportIpv4Uc,
                                    "supportIpv4Uc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv4Uc);

                /*check vrConfigInfoGet.defIpv4UcNextHopInfo.ipLttEntry*/
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry,
                                             (GT_VOID*)&vrConfigInfoGet.defIpv4UcNextHopInfo.ipLttEntry,
                           sizeof (CPSS_DXCH_IP_LTT_ENTRY_STC))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                        "get another defIpv4UcNextHopInfo than was set: %d", dev);
                /*check supportIpv6Uc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, vrConfigInfoGet.supportIpv6Uc,
                                    "supportIpv6Uc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv6Uc);

                /*check vrConfigInfo.defIpv6UcNextHopInfoGet.ipLttEntry*/
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry,
                                             (GT_VOID*)&vrConfigInfoGet.defIpv6UcNextHopInfo.ipLttEntry,
                                             sizeof (CPSS_DXCH_IP_LTT_ENTRY_STC))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                        "get another defIpv6UcNextHopInfo than was set: %d", dev);

                /*check supportIpv4Mc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, vrConfigInfoGet.supportIpv4Mc,
                                    "supportIpv4Mc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv4Mc);

                /*check supportIpv6Mc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, vrConfigInfoGet.supportIpv6Mc,
                                    "supportIpv6Mc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv6Mc);
            }

            /* MC is not supported in PBR mode */
            if (GT_FALSE == prvUtfIsPbrModeUsed())
            {
                /*
                    1.8. Call with lpmDBId [4], vrId[0], defUcNextHopInfo [NULL]
                         defMcRouteLttEntry filled with default configuration
                         (support only multicast) and
                         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                    Expected: GT_OK .
                */
                lpmDBId = 4;
                vrConfigInfo.supportIpv4Mc = GT_TRUE;
                vrConfigInfo.supportIpv6Mc = GT_TRUE;
                vrConfigInfo.supportIpv4Uc = GT_FALSE;
                vrConfigInfo.supportIpv6Uc = GT_FALSE;

                prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv4McRouteLttEntry);
                prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv6McRouteLttEntry);

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%d, %d", lpmDBId, vrId);
                /*
                    1.9. Call cpssDxChIpLpmVirtualRouterGet with the same params.
                    Expected: GT_OK and the same values.
                */
                st = cpssDxChIpLpmVirtualRouterGet (lpmDBId, vrId, &vrConfigInfoGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%d, %d", lpmDBId, vrId);

                if (GT_OK == st)
                {
                    /*check supportIpv4Uc*/
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, vrConfigInfoGet.supportIpv4Uc,
                                        "supportIpv4Uc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv4Uc);

                    /*check supportIpv6Uc*/
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, vrConfigInfoGet.supportIpv6Uc,
                                        "supportIpv6Uc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv6Uc);

                    /*check supportIpv4Mc*/
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, vrConfigInfoGet.supportIpv4Mc,
                                        "supportIpv4Mc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv4Mc);

                    /*check vrConfigInfoGet.defIpv4McRouteLttEntry*/
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*)&vrConfigInfo.defIpv4McRouteLttEntry,
                                                 (GT_VOID*)&vrConfigInfoGet.defIpv4McRouteLttEntry,
                                                 sizeof (CPSS_DXCH_IP_LTT_ENTRY_STC))) ? GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                            "get another defIpv4McRouteLttEntry than was set: %d", dev);
                    /*check supportIpv6Mc*/
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, vrConfigInfoGet.supportIpv6Mc,
                                        "supportIpv6Mc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv6Mc);

                    /*check vrConfigInfoGet.defIpv4McRouteLttEntry*/
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*)&vrConfigInfo.defIpv6McRouteLttEntry,
                                                 (GT_VOID*)&vrConfigInfoGet.defIpv6McRouteLttEntry,
                                                 sizeof (CPSS_DXCH_IP_LTT_ENTRY_STC))) ? GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                            "get another defIpv6McRouteLttEntry than was set: %d", dev);
                }
            }

            /*
                1.10. Call with lpmDBId [0], vrId[0],
                         wrong enum values defUcNextHopInfoPtr->pclIpUcAction.pktCmd,
                         defMcRouteLttEntry[NULL] (support only UC),
                         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                Expected: GT_BAD_PARAM.
            */
            if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
            {
                lpmDBId = 0;
                vrId = 1;
                vrConfigInfo.supportIpv4Mc = GT_FALSE;
                vrConfigInfo.supportIpv6Mc = GT_FALSE;
                vrConfigInfo.supportIpv4Uc = GT_TRUE;
                vrConfigInfo.supportIpv6Uc = GT_TRUE;
                vrConfigInfo.supportFcoe   = GT_FALSE;

                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                                    vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.pktCmd);

                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                                    vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.pktCmd);
            }

            /*
                1.11. Call with lpmDBId [1], vrId[0],
                        wrong enum values defUcNextHopInfoPtr->ipLttEntry.routeType
                        defMcRouteLttEntry {routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                        numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
                        sipSaCheckMismatchEnable[GT_TRUE],
                        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
                        protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                Expected: GT_BAD_PARAM.
            */

            lpmDBId = 1;
            vrId = 0;
            vrConfigInfo.supportIpv4Mc = !(prvUtfIsPbrModeUsed());;
            vrConfigInfo.supportIpv6Mc = !(prvUtfIsPbrModeUsed());;
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportIpv6Uc = GT_TRUE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths     = 0;
            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths     = 0;

            vrConfigInfo.defIpv4McRouteLttEntry.routeEntryBaseIndex      = 0;
            vrConfigInfo.defIpv4McRouteLttEntry.ucRPFCheckEnable         = GT_TRUE;
            vrConfigInfo.defIpv4McRouteLttEntry.sipSaCheckMismatchEnable = GT_TRUE;
            vrConfigInfo.defIpv4McRouteLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

            vrConfigInfo.defIpv6McRouteLttEntry.routeEntryBaseIndex      = 0;
            vrConfigInfo.defIpv6McRouteLttEntry.ucRPFCheckEnable         = GT_TRUE;
            vrConfigInfo.defIpv6McRouteLttEntry.sipSaCheckMismatchEnable = GT_TRUE;
            vrConfigInfo.defIpv6McRouteLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

            for(enumsIndex = 0; enumsIndex < utfInvalidEnumArrSize; enumsIndex++)
            {
                vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeType =
                                                            utfInvalidEnumArr[enumsIndex];
                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                if (shadowType == CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E)
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_ALREADY_EXIST, st,
                          "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.routeType = %d",
                          lpmDBId, vrId,  vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeType);
                }
                else
                {
                   UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
                      "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.routeType = %d",
                      lpmDBId, vrId,  vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeType);
                }
            }

            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeType
                                                    = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;

            for(enumsIndex = 0; enumsIndex < utfInvalidEnumArrSize; enumsIndex++)
            {
                vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.routeType =
                                                            utfInvalidEnumArr[enumsIndex];
                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                if (shadowType == CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E)
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_ALREADY_EXIST, st,
                           "%d, %d, defIpv6UcNextHopInfoPtr->ipLttEntry.routeType = %d",
                           lpmDBId, vrId,  vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.routeType);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
                       "%d, %d, defIpv6UcNextHopInfoPtr->ipLttEntry.routeType = %d",
                       lpmDBId, vrId,  vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.routeType);
                }
            }
            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.routeType
                                                       = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;

            vrConfigInfo.defIpv4McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
            vrConfigInfo.defIpv6McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;

            st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
            /*
                1.12. Call with correct numOfPaths [5/7] ipv4/ipv6 for unicast and multicast.
                Expected: GT_OK.
            */
            /* call with numOfPaths [5] for ipv4 Uc */
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 5;

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

            /* restore configuration */
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;

            st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            /* call with numOfPaths [7] for ipv4 Uc */
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 7;

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                   lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

            /* restore configuration */
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;

            st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            /* call with numOfPaths [5] for ipv6 Uc */
            vrConfigInfo.supportIpv6Uc = GT_TRUE;

            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 5;

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "%d, %d, defIpv6UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                   lpmDBId, vrId, vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths);

            /* restore configuration */
            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;

            st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

            vrConfigInfo.supportIpv6Uc = GT_FALSE;

            /* call with numOfPaths [7] for ipv4 Uc */
            vrConfigInfo.supportIpv6Uc = GT_TRUE;

            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 7;

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "%d, %d, defIpv6UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                   lpmDBId, vrId, vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths);

            /* restore configuration */
            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;

            st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

            vrConfigInfo.supportIpv6Uc = GT_FALSE;

            /* MC is not supported in PBR mode */
            if (GT_FALSE == prvUtfIsPbrModeUsed())
            {
                /* call with numOfPaths [5] for ipv4 Mc */
                vrConfigInfo.supportIpv4Mc = GT_TRUE;

                vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths = 5;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4McRouteLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths);

                /* restore configuration */
                vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths = 0;

                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

                vrConfigInfo.supportIpv4Mc = GT_FALSE;

                /* call with numOfPaths [7] for ipv4 Mc */
                vrConfigInfo.supportIpv4Mc = GT_TRUE;

                vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths = 7;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4McRouteLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths);

                /* restore configuration */
                vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths = 0;

                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

                vrConfigInfo.supportIpv4Mc = GT_FALSE;

                /* call with numOfPaths [5] for ipv6 Mc */
                vrConfigInfo.supportIpv6Mc = GT_TRUE;

                vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths = 5;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv6McRouteLttEntryPtr.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths);

                /* restore configuration */
                vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths = 0;

                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

                vrConfigInfo.supportIpv6Mc = GT_FALSE;

                /* call with numOfPaths [7] for ipv4 Mc */
                vrConfigInfo.supportIpv6Mc = GT_TRUE;

                vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths = 7;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv6McRouteLttEntryPtr.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths);

                /* restore configuration */
                vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths = 0;

                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

                vrConfigInfo.supportIpv6Mc = GT_FALSE;
            }

            /*
                1.13. Call with correct numOfPaths [8/63] ipv4/ipv6 for unicast and multicast.
                Expected: GT_OK for lion and above and not GT_OK for other.
            */
            /* call with numOfPaths [8] for ipv4 Uc */
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 8;

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            if ((CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily) || prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

                /* restore configuration */
                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }

            /* restore configuration */
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            /* call with numOfPaths [63] for ipv4 Uc */
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 63;

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

                /* restore configuration */
                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }

            /* restore configuration */
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            /* call with numOfPaths [8] for ipv6 Uc */
            vrConfigInfo.supportIpv6Uc = GT_TRUE;

            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 8;

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

                /* restore configuration */
                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }

            /* restore configuration */
            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfigInfo.supportIpv6Uc = GT_FALSE;

            /* call with numOfPaths [63] for ipv4 Uc */
            vrConfigInfo.supportIpv6Uc = GT_TRUE;

            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 63;

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

                /* restore configuration */
                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }

            /* restore configuration */
            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfigInfo.supportIpv6Uc = GT_FALSE;

            /* MC is not supported in PBR mode */
            if (GT_FALSE == prvUtfIsPbrModeUsed())
            {
                /* call with numOfPaths [8] for ipv4 Mc */
                vrConfigInfo.supportIpv4Mc = GT_TRUE;

                vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths = 8;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                           "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                           lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

                    /* restore configuration */
                    st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                           "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                           lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
                }

                /* restore configuration */
                vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths = 0;
                vrConfigInfo.supportIpv4Mc = GT_FALSE;

                /* call with numOfPaths [63] for ipv4 Mc */
                vrConfigInfo.supportIpv4Mc = GT_TRUE;
                vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths = 63;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                           "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                           lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

                    /* restore configuration */
                    st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                           "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                           lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
                }

                /* restore configuration */
                vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths = 0;
                vrConfigInfo.supportIpv4Mc = GT_FALSE;

                /* call with numOfPaths [8] for ipv6 Mc */
                vrConfigInfo.supportIpv6Mc = GT_TRUE;
                vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths = 8;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                           "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                           lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

                    /* restore configuration */
                    st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                           "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                           lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
                }

                /* restore configuration */
                vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths = 0;
                vrConfigInfo.supportIpv6Mc = GT_FALSE;

                /* call with numOfPaths [63] for ipv4 Mc */
                vrConfigInfo.supportIpv6Mc = GT_TRUE;

                vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths = 63;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                           "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                           lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

                    /* restore configuration */
                    st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                           "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                           lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
                }

                /* restore configuration */
                vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths = 0;
                vrConfigInfo.supportIpv6Mc = GT_FALSE;
            }

            /*
                1.14. Call with wrong numOfPaths [64] (out of range).
                Check ipv4/ipv6 for unicast and multicast.
                Expected: NOT GT_OK
            */
            /* LTT Entry is not supported in PBR mode, numOfPaths is not applicable
               to SIP5*/
            if ((GT_FALSE == prvUtfIsPbrModeUsed()) &&
                (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E))
            {
                /* call with numOfPaths [64] for ipv4 Uc */
                vrConfigInfo.supportIpv4Uc = GT_TRUE;
                vrConfigInfo.supportFcoe   = GT_FALSE;

                vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 64;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

                /* restore configuration */
                vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;
                vrConfigInfo.supportIpv4Uc = GT_FALSE;
                vrConfigInfo.supportFcoe   = GT_FALSE;

                /* call with numOfPaths [64] for ipv6 Uc */
                vrConfigInfo.supportIpv6Uc = GT_TRUE;
                vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 64;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv6UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths);

                /* restore configuration */
                vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
                vrConfigInfo.supportIpv6Uc = GT_FALSE;

                /* call with numOfPaths [64] for ipv4 Mc */
                vrConfigInfo.supportIpv4Mc = GT_TRUE;
                vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths = 64;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                           "%d, %d, defIpv4McRouteLttEntry.numOfPaths = %d",
                           lpmDBId, vrId, vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths);

                /* restore configuration */
                vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths = 0;
                vrConfigInfo.supportIpv4Mc = GT_FALSE;

                /* call with numOfPaths [64] for ipv6 Mc */
                vrConfigInfo.supportIpv6Mc = GT_TRUE;
                vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths = 64;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                           "%d, %d, defIpv6McRouteLttEntryPtr.numOfPaths = %d",
                           lpmDBId, vrId, vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths);

                /* restore configuration */
                vrConfigInfo.defIpv6McRouteLttEntry.numOfPaths = 0;
                vrConfigInfo.supportIpv6Mc = GT_FALSE;
            }

            /*
                1.15. Call with only supportIpv4Mc enabled (other disabled).
                        vrConfigInfo.supportIpv4Uc = GT_FALSE;
                        vrConfigInfo.supportIpv4Mc = GT_TRUE;
                        vrConfigInfo.supportIpv6Uc = GT_FALSE;
                        vrConfigInfo.supportIpv6Mc = GT_FALSE;
                Expected: GT_OK.
            */
            /* MC is not supported in PBR mode */
            if (GT_FALSE == prvUtfIsPbrModeUsed())
            {
                vrConfigInfo.supportIpv4Uc = GT_FALSE;
                vrConfigInfo.supportIpv4Mc = GT_TRUE;
                vrConfigInfo.supportIpv6Uc = GT_FALSE;
                vrConfigInfo.supportIpv6Mc = GT_FALSE;
                vrConfigInfo.supportFcoe   = GT_FALSE;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "%d, %d, call with enabled only supportIpv4Mc", lpmDBId, vrId);

                if(PRV_CPSS_SIP_5_CHECK_MAC(dev) == GT_TRUE)
                {
                    /*
                        1.15.1 Call with only supportFcoe enabled (other disabled) and lpmDBId = 6 (created with FCOE enabled).
                                vrConfigInfo.supportIpv4Uc = GT_FALSE;
                                vrConfigInfo.supportIpv4Mc = GT_FALSE;
                                vrConfigInfo.supportIpv6Uc = GT_FALSE;
                                vrConfigInfo.supportIpv6Mc = GT_FALSE;
                                vrConfigInfo.supportFcoe   = GT_TRUE;
                        Expected: GT_OK.
                    */
                    vrConfigInfo.supportIpv4Uc = GT_FALSE;
                    vrConfigInfo.supportIpv4Mc = GT_FALSE;
                    vrConfigInfo.supportIpv6Uc = GT_FALSE;
                    vrConfigInfo.supportIpv6Mc = GT_FALSE;
                    vrConfigInfo.supportFcoe   = GT_TRUE;

                    st = cpssDxChIpLpmVirtualRouterAdd(6 /*lpmDBId*/, vrId, &vrConfigInfo);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "%d, %d, call with enabled only supportFcoe", lpmDBId, vrId);
                }

                if(GT_OK == st)
                {
                    /*
                        1.16. Call with all types disabled.
                                vrConfigInfo.supportIpv4Uc = GT_FALSE;
                                vrConfigInfo.supportIpv4Mc = GT_FALSE;
                                vrConfigInfo.supportIpv6Uc = GT_FALSE;
                                vrConfigInfo.supportIpv6Mc = GT_FALSE;
                        Expected: NOT GT_OK.
                    */
                    vrConfigInfo.supportIpv4Uc = GT_FALSE;
                    vrConfigInfo.supportIpv4Mc = GT_FALSE;
                    vrConfigInfo.supportIpv6Uc = GT_FALSE;
                    vrConfigInfo.supportIpv6Mc = GT_FALSE;
                    vrConfigInfo.supportFcoe   = GT_FALSE;

                    st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                           "%d, %d, call with disabled all features", lpmDBId, vrId);

                    /* restore configuration */
                    st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

                    vrConfigInfo.supportIpv4Mc = GT_TRUE;
                }
            }

            /*
                1.17. Call with lpmDBId [1], vrId[0],
                        out of range defUcNextHopInfoPtr->ipLttEntry.routeEntryBaseIndex [4096]
                        defMcRouteLttEntry {routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                           numOfPaths[0],
                                           routeEntryBaseIndex[0],
                                           ucRPFCheckEnable[GT_TRUE],
                                           sipSaCheckMismatchEnable[GT_TRUE],
                                           ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
                        protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                Expected: NOT GT_OK.
            */
            /* LTT Entry is not supported in PBR mode */
            if (GT_FALSE == prvUtfIsPbrModeUsed())
            {
                /* check ipv4 */
                vrConfigInfo.supportFcoe   = GT_FALSE;

                vrConfigInfo.supportIpv4Uc = GT_TRUE;
                vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex =
                                    PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerNextHop;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.routeEntryBaseIndex = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex);

                if(GT_OK == st)
                {
                    /* restore configuration */
                    st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
                }

                vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;

                /* check ipv6 */
                vrConfigInfo.supportIpv6Uc = GT_TRUE;
                vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex =
                                   PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerNextHop;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, %d, defIpv6UcNextHopInfoPtr->ipLttEntry.routeEntryBaseIndex = %d",
                        lpmDBId, vrId, vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex);

                if(GT_OK == st)
                {
                    /* restore configuration */
                    st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
                }

                vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
            }

            /*
                1.18. Call with lpmDBId [1], vrId[0],
                        out of range defUcNextHopInfoPtr->ipLttEntry.
                        ipv6MCGroupScopeLevel [wrong enum values]
                        defMcRouteLttEntry {routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                               numOfPaths[0],
                                               routeEntryBaseIndex[0],
                                               ucRPFCheckEnable[GT_TRUE],
                                               sipSaCheckMismatchEnable[GT_TRUE],
                                               ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
                        protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                (lpmDBId, vrId, &vrConfigInfo),
                                vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel);

            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                (lpmDBId, vrId, &vrConfigInfo),
                                vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel);

            /*
                1.19. Call with not valid lpmDBId [10], vrId[4096].
                Expected: NOT GT_OK.
            */
            lpmDBId = 10;

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

            lpmDBId = 0;
            vrId = 4096;
            st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);
            vrId = 0;
            /* Test cases for Cheetah+ devices */
            if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
            {
                /*
                    1.20. Call with lpmDBId [0],
                                invalid defUcNextHopInfoPtr->pclIpUcAction.mirror.cpuCode
                                [CPSS_NET_MIRROR_PCL_RST_FIN_E] (Tiger only),
                                and other parameters the same as in 1.2.
                    Expected: NON GT_OK.
                */
                lpmDBId = 0;
                vrConfigInfo.supportIpv4Uc = GT_TRUE;
                vrConfigInfo.supportIpv6Uc = GT_TRUE;
                vrConfigInfo.supportIpv4Mc = GT_FALSE;
                vrConfigInfo.supportIpv6Mc = GT_FALSE;
                vrConfigInfo.supportFcoe   = GT_FALSE;

                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.mirror.cpuCode = CPSS_NET_MIRROR_PCL_RST_FIN_E;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                                "%d, %d, defIpv4UcNextHopInfoPtr->pclIpUcAction.mirror.cpuCode = %d",
                                lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.mirror.cpuCode);

                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.mirror.cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E;

                vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.mirror.cpuCode = CPSS_NET_MIRROR_PCL_RST_FIN_E;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                                "%d, %d, defIpv6UcNextHopInfoPtr->pclIpUcAction.mirror.cpuCode = %d",
                                lpmDBId, vrId, vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.mirror.cpuCode);

                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.mirror.cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E;

                /*
                    1.22. Call with lpmDBId [0],
                             defUcNextHopInfoPtr->pclIpUcAction.matchCounter
                             [{enableMatchCount = GT_TRUE; matchCounterIndex = 33 (out of range)}]
                             and other parameters the same as in 1.2.
                    Expected: NON GT_OK.
                */
                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.matchCounter.enableMatchCount  = GT_TRUE;
                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.matchCounter.matchCounterIndex = 33;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, %d, defIpv4UcNextHopInfoPtr->pclIpUcAction.matchCounter.matchCounterIndex = %d",
                     lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.matchCounter.matchCounterIndex);

                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.matchCounter.enableMatchCount  = GT_FALSE;
                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.matchCounter.matchCounterIndex = 0;

                vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.matchCounter.enableMatchCount  = GT_TRUE;
                vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.matchCounter.matchCounterIndex = 33;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, %d, defIpv6UcNextHopInfoPtr->pclIpUcAction.matchCounter.matchCounterIndex = %d",
                     lpmDBId, vrId, vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.matchCounter.matchCounterIndex);

                vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.matchCounter.enableMatchCount  = GT_FALSE;
                vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.matchCounter.matchCounterIndex = 0;

                /*
                    1.23. Call with lpmDBId [0],
                              defUcNextHopInfoPtr->pclIpUcAction.qos.ingress.modifyDscp [wrong enum values]
                              and other parameters the same as in 1.2.
                    Expected: GT_BAD_PARAM.
                */
                vrId = 2;
                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                                    vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.qos.ingress.modifyDscp);

                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                                    vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.qos.ingress.modifyDscp);

                /*
                    1.24. Call with lpmDBId [0],
                                     defUcNextHopInfoPtr->pclIpUcAction.qos.ingress.modifyUp [wrong enum values]
                                     and other parameters the same as in 1.2.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                                    vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.qos.ingress.modifyUp);

                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                                    vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.qos.ingress.modifyUp);

                /*
                    1.25. Call with lpmDBId [0],
                             defUcNextHopInfoPtr->pclIpUcAction.qos.ingress.profilePrecedence
                             [wrong enum values]
                             and other parameters the same as in 1.2.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                  vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.qos.ingress.profilePrecedence);

                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                  vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.qos.ingress.profilePrecedence);

                /*
                    1.26. Call with lpmDBId [0],
                             defUcNextHopInfoPtr->pclIpUcAction.redirect.redirectCmd [wrong enum values]
                             and other parameters the same as in 1.2.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                  vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.redirect.redirectCmd);

                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                  vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.redirect.redirectCmd);

                /*
                    1.27. Call with lpmDBId [0],
                          defUcNextHopInfoPtr->pclIpUcAction.policer[{policerEnable = GT_TRUE;
                                                                      policerId = 0xFFFFFFFF}]
                                     and other parameters the same as in 1.2.
                    Expected: NON GT_OK.
                */
                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.policer.policerEnable = GT_TRUE;
                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.policer.policerId     = 0xFFFFFFFF;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                                "%d, %d, defIpv4UcNextHopInfoPtr->pclIpUcAction.policer.policerId = %d",
                                 lpmDBId, vrId,  vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.policer.policerId);

                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.policer.policerEnable = GT_FALSE;
                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.policer.policerId     = 0;

                vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.policer.policerEnable = GT_TRUE;
                vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.policer.policerId     = 0xFFFFFFFF;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                                "%d, %d, defIpv6UcNextHopInfoPtr->pclIpUcAction.policer.policerId = %d",
                                 lpmDBId, vrId,  vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.policer.policerId);

                vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.policer.policerEnable = GT_FALSE;
                vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.policer.policerId     = 0;

                /*
                    1.28. Call with lpmDBId [0],
                             defUcNextHopInfoPtr->pclIpUcAction.vlan
                                        [{egressTaggedModify = GT_TRUE;
                                          modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E,
                                          vlanId = 4096}]
                             and other parameters the same as in 1.2.
                    Expected: GT_OUT_OF_RANGE.
                */
                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.vlan.ingress.vlanId     = PRV_CPSS_MAX_NUM_VLANS_CNS;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OUT_OF_RANGE, st,
                                "%d, %d, defIpv4UcNextHopInfoPtr->pclIpUcAction.vlan.ingress.vlanId = %d",
                                lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.vlan.ingress.vlanId);

                vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
                vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.vlan.ingress.vlanId     = PRV_CPSS_MAX_NUM_VLANS_CNS;

                st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OUT_OF_RANGE, st,
                                "%d, %d, defIpv6UcNextHopInfoPtr->pclIpUcAction.vlan.ingress.vlanId = %d",
                                lpmDBId, vrId, vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.vlan.ingress.vlanId);

                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
                vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.vlan.ingress.vlanId     = IP_LPM_TESTED_VLAN_ID_CNS;

                /*
                    1.29. Call with lpmDBId [0],
                             defUcNextHopInfoPtr->pclIpUcAction.vlan[{egressTaggedModify = GT_FALSE;
                                                                      modifyVlan = wrong enum values}]
                             and other parameters the same as in 1.2.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                                    vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.vlan.ingress.modifyVlan);

                UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmVirtualRouterAdd
                                    (lpmDBId, vrId, &vrConfigInfo),
                                    vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction.vlan.ingress.modifyVlan);
            }
        }
        firstDev=GT_FALSE;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmVirtualRouterDel
(
    IN GT_U32                                 lpmDBId,
    IN GT_U32                                 vrId
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmVirtualRouterDel)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Create default LPM configuration and add virtual routers
         with prvUtfCreateLpmDBAndVirtualRouterAdd.
    Expected: GT_OK.
    1.2. Call with lpmDBId [0], vrId[0].
    Expected: GT_OK.
    1.3. Call with lpmDBId [0] and already removed vrId[0].
    Expected: NOT GT_OK.
    1.4. Call with lpmDBId [0] and out of range vrId[1].
    Expected: NOT GT_OK.
    1.5. Call with not valid LPM DB id lpmDBId [10], and vrId [0].
    Expected: NOT GT_OK.
*/
    GT_STATUS st      = GT_OK;
    GT_U8     dev;

    GT_U32    lpmDBId = 0;
    GT_U32    vrId    = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Create default LPM configuration and add virtual routers
                 with prvUtfCreateLpmDBAndVirtualRouterAdd.
            Expected: GT_OK.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.2. Call with lpmDBId [0], vrId[0].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.3. Call with lpmDBId [0] and already removed vrId[0].
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.4. Call with lpmDBId [0] and out of range vrId[4096].
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.5. Call with not valid LPM DB id lpmDBId [10], and vrId [0].
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4UcPrefixAdd
(
    IN GT_U32                                   lpmDBId,
    IN GT_U32                                   vrId,
    IN GT_IPADDR                                ipAddr,
    IN GT_U32                                   prefixLen,
    IN CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT   *nextHopInfoPtr,
    IN GT_BOOL                                  override
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4UcPrefixAdd)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Create default LPM configuration and add virtual routers
         with prvUtfCreateLpmDBAndVirtualRouterAdd.
    Expected: GT_OK.
    1.2. Call with lpmDBId [1], vrId[0], ipAddr[10.16.0.1], prefixLen[32],
        nextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
        numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE], ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK.
    1.3. Call with lpmDBId [1], vrId[0], ipAddr[10.16.0.1], prefixLen[32],
           nextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                      numOfPaths[0],
                                      routeEntryBaseIndex[0],
                                      ucRPFCheckEnable[GT_TRUE],
                                      sipSaCheckMismatchEnable[GT_TRUE],
                                      ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK
    1.4. Call with override [GT_FALSE] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.5. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.6. Call with not supported vrId [4096] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.7. Call with not multicast ip address range (224-239)
    Expected: NOT GT_OK.
    1.8. Call with reserved ip address range (240-255)
    Expected: GT_OK.
    1.9. Call with out of range prefixLen [33] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.10. Call with lpmDBId [1] wrong enum values nextHopInfoPtr->ipLttEntry.routeType
        and other parameters from 1.5.
    Expected: GT_BAD_PARAM.
    1.11. Call with lpmDBId [1], correct nextHopInfoPtr->ipLttEntry.numOfPaths
                     ([5 / 7] - for lion and above and [49 / 63] for other)
                     and other parameters from 1.5.
    Expected: GT_OK.
    1.12. Call with lpmDBId [1],
                     out of range nextHopInfoPtr->ipLttEntry.numOfPaths
                     ([64 / 65] - for lion and above and [8 / 9] for other)
                     and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.13. Call with lpmDBId [1] out of range nextHopInfoPtr->ipLttEntry.
        numOfPaths[routeEntryBaseIndex] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.14. Call with lpmDBId [1] out of range nextHopInfoPtr->ipLttEntry.
        ipv6MCGroupScopeLevel [wrong enum values] and other parameters from 1.5.
    Expected: GT_BAD_PARAM.
    1.15. Call with lpmDBId [1] nextHopInfoPtr[NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
    1.16. Call with lpmDBId [0] wrong enum values nextHopInfoPtr->pclIpUcAction.pktCmdp
        and other parameters from 1.4.
    Expected: GT_BAD_PARAM.
    1.17. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0 / 1] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId     = 0;
    GT_U32                                 vrId        = 0;
    GT_IPADDR                              ipAddr;
    GT_U32                                 prefixLen   = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override    = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_FALSE;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT      shadowType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfGetDevShadow(dev, &shadowType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        /*
            1.1. Create default LPM configuration and add virtual routers
                 with prvUtfCreateLpmDBAndVirtualRouterAdd.
            Expected: GT_OK.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.2. Call with lpmDBId [0], vrId[0], ipAddr[10.15.0.1], prefixLen[32],
                   nextHopInfoPtr->pclIpUcAction{pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                             mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                    mirrorToRxAnalyzerPort = GT_FALSE},
                             matchCounter{enableMatchCount = GT_FALSE,
                                          matchCounterIndex = 0},
                             qos{egressPolicy = GT_FALSE, modifyDscp = GT_FALSE, modifyUp = GT_FALSE,
                                 qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                                        profilePrecedence=GT_FALSE] ] },
                             redirect {CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                             data[routerLttIndex=0] },
                             policer { policerEnable=GT_FALSE, policerId=0 },
                             vlan {egressTaggedModify=GT_FALSE,
                                   modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                   nestedVlan=GT_FALSE, vlanId=100,
                                   precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                              ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                           override [GT_TRUE].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        prefixLen = 32;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);
        }

        override  = GT_TRUE;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override,defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, %d, override = %d",
                                        lpmDBId, vrId, prefixLen, override);

        /*
            1.3. Call with lpmDBId [1], vrId[0], ipAddr[10.16.0.1], prefixLen[32],
                   nextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                              numOfPaths[0],
                                              routeEntryBaseIndex[0],
                                              ucRPFCheckEnable[GT_TRUE],
                                              sipSaCheckMismatchEnable[GT_TRUE],
                                              ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
            Expected: GT_OK
        */
        lpmDBId = 1;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 16;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override,defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, %d, override = %d",
                                        lpmDBId, vrId, prefixLen, override);

        /*
            1.4. Call with override [GT_FALSE] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        override = GT_FALSE;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override,defragmentationEnable);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, override = %d", lpmDBId, vrId, override);

        override = GT_TRUE;

        /*
            1.5. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.6. Call with not supported vrId [4096] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.7. Call with not multicast ip address range (224-239)
            Expected: NOT GT_OK.
        */
        for(ipAddr.arIP[0] = 224; ipAddr.arIP[0] < 240; ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, lpmDBId, vrId, ipAddr.arIP[0]);
        }

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.8. Call with not reserved ip address range (240-255)
            Expected: GT_OK.
        */
        for(ipAddr.arIP[0] = 240; ipAddr.arIP[0] < 255; ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, lpmDBId, vrId, ipAddr.arIP[0]);
        }

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.9. Call with out of range prefixLen [33] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        prefixLen = 33;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d", lpmDBId, prefixLen);

        prefixLen = 32;

        if (CPSS_PP_FAMILY_CHEETAH_E != PRV_CPSS_PP_MAC(dev)->devFamily && !prvUtfIsPbrModeUsed())
        {
            /*
                1.10. Call with lpmDBId [1]
                                wrong enum values nextHopInfoPtr->ipLttEntry.routeType
                                and other parameters from 1.5.
                Expected: GT_BAD_PARAM.
            */
            lpmDBId = 1;
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmIpv4UcPrefixAdd
                                (lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable),
                                nextHopInfo.ipLttEntry.routeType);

            /* numOfPaths is not applicable to SIP5 */
            if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
            {
                /*
                    1.11. Call with lpmDBId [1], correct nextHopInfoPtr->ipLttEntry.numOfPaths
                                     ([5 / 7] - for lion and above and [49 / 63] for other)
                                     and other parameters from 1.5.
                    Expected: GT_OK.
                */
                /*call with 49 or 5 values */
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 49;
                }
                else
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 5;
                }

                st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen,
                                                        &nextHopInfo, override, defragmentationEnable);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                          lpmDBId, vrId, nextHopInfo.ipLttEntry.numOfPaths);

                /*call with 63 or 7 values */
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 63;
                }
                else
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 7;
                }

                st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen,
                                                        &nextHopInfo, override, defragmentationEnable);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                          lpmDBId, vrId, nextHopInfo.ipLttEntry.numOfPaths);


                /*
                    1.12. Call with lpmDBId [1],
                                     out of range nextHopInfoPtr->ipLttEntry.numOfPaths
                                     ([64 / 65] - for lion and above and [8 / 9] for other)
                                     and other parameters from 1.5.
                    Expected: NOT GT_OK.
                */
                /*call with 64 or 8 values */
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 64;
                }
                else
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 8;
                }

                st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen,
                                                        &nextHopInfo, override, defragmentationEnable);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                          lpmDBId, vrId, nextHopInfo.ipLttEntry.numOfPaths);

                /*call with 65 or 9 values */
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 65;
                }
                else
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 9;
                }

                st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen,
                                                        &nextHopInfo, override, defragmentationEnable);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                          lpmDBId, vrId, nextHopInfo.ipLttEntry.numOfPaths);

                nextHopInfo.ipLttEntry.numOfPaths = 0;

                /*
                    1.13. Call with lpmDBId [1]
                                     out of range nextHopInfoPtr->ipLttEntry.routeEntryBaseIndex[4096]
                                     and other parameters from 1.5.
                    Expected: NOT GT_OK.
                */
                nextHopInfo.ipLttEntry.routeEntryBaseIndex =
                                    PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerNextHop;

                st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr,
                                                  prefixLen, &nextHopInfo, override, defragmentationEnable);

                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                            "%d, %d, nextHopInfoPtr->ipLttEntry.routeEntryBaseIndex",
                            lpmDBId, vrId, nextHopInfo.ipLttEntry.routeEntryBaseIndex);
            }

            nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;

            /*
                1.14. Call with lpmDBId [1]
                                 wrong enum values nextHopInfoPtr->ipLttEntry.ipv6MCGroupScopeLevel
                                 and other parameters from 1.5.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmIpv4UcPrefixAdd
                                (lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable),
                                nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel);

            /*
                1.15. Call with lpmDBId [1] nextHopInfoPtr[NULL] and other parameters from 1.5.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, NULL, override, defragmentationEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);
        }

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            /*
                1.16. Call with lpmDBId [0]
                         wrong enum values nextHopInfoPtr->pclIpUcAction.pktCmdp
                         and other parameters from 1.4.
                Expected: GT_BAD_PARAM.
            */
            lpmDBId = 0;
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);

            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmIpv4UcPrefixAdd
                                (lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable),
                                nextHopInfo.pclIpUcAction.pktCmd);
        }

        /*
            1.17. Call cpssDxChIpLpmIpv4UcPrefixesFlush with
                  lpmDBId [0 / 1] and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId[0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Call with lpmDBId[1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4UcPrefixAddBulk
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArray
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4UcPrefixAddBulk)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
         indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
         tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
         numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0],
         defUcNextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
         mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
         matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
         qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
         qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
         redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
         policer { policerEnable=GT_FALSE, policerId=0 },
         vlan { egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
         nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
         ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
         defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK Cheetha else GT_BAD_PRT
    1.3. Call with lpmDBId[0], ipv4PrefixArrayLen[1], ipv4PrefixArray{vrId[0],
         ipAddr[10.15.0.1], prefixLen[32], nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
         mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
         matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
         qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
         qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ]},
         redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
         policer { policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
         modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
         precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
         ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
         override[GT_TRUE], returnStatus[0]}
    Expected: GT_OK.
    1.4. Call with not supported ipv4PrefixArray->vrId[10] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.5. Call with out of range ipv4PrefixArray->prefixLen [33] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.6. Call with lpmDBId [1], correct nextHopInfoPtr->ipLttEntry.numOfPaths
                     ([5 / 7] - for lion and above and [49 / 63] for other)
                     and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.7. Call with lpmDBId [1],
                     out of range nextHopInfoPtr->ipLttEntry.numOfPaths
                     ([64 / 65] - for lion and above and [8 / 9] for other)
                     and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.9. Call with ipv4PrefixArray[NULL] and other parameters from 1.3.
    Expected: GT_BAD_PTR.
    1.10. Call with not multicast ip address range (224-239)
    Expected: NOT GT_OK.
    1.11. Call with reserved ip address range (240-255)
    Expected: GT_OK.
    1.12. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                              lpmDBId   = 0;
    GT_U32                              prefixLen = 0;
    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC ipv4PrefixArray;
    GT_U32                              vrId      = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&ipv4PrefixArray, sizeof(ipv4PrefixArray));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call with lpmDBId[0], ipv4PrefixArrayLen[1], ipv4PrefixArray{vrId[0],
                   ipAddr[10.15.0.1], prefixLen[32],
                   nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                   mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                   mirrorToRxAnalyzerPort = GT_FALSE},
                   matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
                   qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                   qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                   profilePrecedence=GT_FALSE] ] },
                   redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                   policer { policerEnable=GT_FALSE, policerId=0 },
                   vlan { egressTaggedModify=GT_FALSE,
                   modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                   vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                   ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                   override[GT_TRUE], returnStatus[0]}
            Expected: GT_OK.
        */
        lpmDBId   = 0;
        prefixLen = 1;

        ipv4PrefixArray.ipAddr.arIP[0] = 10;
        ipv4PrefixArray.ipAddr.arIP[1] = 15;
        ipv4PrefixArray.ipAddr.arIP[2] = 0;
        ipv4PrefixArray.ipAddr.arIP[3] = 1;

        ipv4PrefixArray.prefixLen = 32;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&ipv4PrefixArray.nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &ipv4PrefixArray.nextHopInfo.ipLttEntry);
        }

        ipv4PrefixArray.override                = GT_TRUE;
        ipv4PrefixArray.returnStatus            = GT_OK;
        ipv4PrefixArray.vrId = 0;

        st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, prefixLen);

        /*
            1.4. Call with not supported ipv4PrefixArray->vrId[10]
                 and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        ipv4PrefixArray.vrId = 10;

        st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipv4PrefixArray->vrId = %d",
                                         lpmDBId, ipv4PrefixArray.vrId);

        ipv4PrefixArray.vrId = 0;

        /*
            1.5. Call with out of range ipv4PrefixArray->prefixLen [33]
                 and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        ipv4PrefixArray.prefixLen = 33;

        st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipv4PrefixArray->prefixLen = %d",
                                         lpmDBId, ipv4PrefixArray.prefixLen);

        ipv4PrefixArray.prefixLen = 32;

        if ((CPSS_PP_FAMILY_CHEETAH_E != PRV_CPSS_PP_MAC(dev)->devFamily) &&
            (!PRV_CPSS_SIP_5_CHECK_MAC(dev)) &&
            !prvUtfIsPbrModeUsed())
        {
            /*
                1.6. Call with lpmDBId [1], correct nextHopInfoPtr->ipLttEntry.numOfPaths
                                 ([5 / 7] - for lion and above and [49 / 63] for other)
                                 and other parameters from 1.3.
                Expected: GT_OK.
            */
            /*call with 49 or 5 values */
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 49;
            }
            else
            {
                ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 5;
            }

            st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                      lpmDBId, vrId, ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths);

            /*call with 63 or 7 values */
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 63;
            }
            else
            {
                ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 7;
            }

            st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                      lpmDBId, vrId, ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths);

            /*
                1.7. Call with lpmDBId [1],
                                 out of range nextHopInfoPtr->ipLttEntry.numOfPaths
                                 ([64 / 65] - for lion and above and [8 / 9] for other)
                                 and other parameters from 1.3.
                Expected: NOT GT_OK.
            */
            /*call with 64 or 8 values */
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 64;
            }
            else
            {
                ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 8;
            }

            st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);

            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                      lpmDBId, vrId, ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths);

            /*call with 65 or 9 values */
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 65;
            }
            else
            {
                ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 9;
            }

            st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);

            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                      lpmDBId, vrId, ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths);

            ipv4PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 0;
        }

        /*
            1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.9. Call with ipv4PrefixArray[NULL] and other parameters from 1.3.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ipv4PrefixArray = NULL", lpmDBId);

        /*
            1.10. Call with not multicast ip address range (224-239)
            Expected: NOT GT_OK.
        */
        for(ipv4PrefixArray.ipAddr.arIP[0] = 224;
            ipv4PrefixArray.ipAddr.arIP[0] < 240;
            ipv4PrefixArray.ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, lpmDBId, prefixLen, ipv4PrefixArray.ipAddr.arIP[0]);
        }

        ipv4PrefixArray.ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.11. Call with reserved ip address range (240-255)
            Expected: GT_OK.
        */
        for(ipv4PrefixArray.ipAddr.arIP[0] = 240;
            ipv4PrefixArray.ipAddr.arIP[0] < 255;
            ipv4PrefixArray.ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, lpmDBId, prefixLen, ipv4PrefixArray.ipAddr.arIP[0]);
        }

        ipv4PrefixArray.ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.12. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0 / 1]
                 and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId[0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Call with lpmDBId[1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4UcPrefixDel
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               vrId,
    IN GT_IPADDR                            ipAddr,
    IN GT_U32                               prefixLen
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4UcPrefixDel)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK Cheetah else GT_BAD_PTR
    1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
        prefixLen[32], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.4. Call with lpmDBId [0], vrId[0], ipAddr[10.15.0.1], prefixLen[32].
    Expected: GT_OK.
    1.5. For Cheetah+ call with lpmDBId [0], vrId[0], ipAddr[10.15.0.1] (already deleted), prefixLen[32].
    Expected: NOT GT_OK.
    1.6. For Cheetah+ call with lpmDBId [0], vrId[0], ipAddr[10.15.0.2] (wrong IP address), prefixLen[32].
    Expected: NOT GT_OK.
    1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.8. Call with not supported vrId [4096] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.9. Call with not multicast ip address range (224-239)
    Expected: NOT GT_OK.
    1.10. Call with reserved ip address range (240-255)
    Expected: GT_OK.
    1.11. Call with out of range prefixLen [33] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.12. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId   = 0;
    GT_U32                                 vrId      = 0;
    GT_IPADDR                              ipAddr;
    GT_U32                                 prefixLen = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override  = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
                            prefixLen[32],
                            nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                            mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                            mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount
                            = GT_FALSE, matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE,
                            modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                            ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                            profilePrecedence=GT_FALSE] ] }, redirect {
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                            policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                            egressTaggedModify=GT_FALSE,
                            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                            nestedVlan=GT_FALSE, vlanId=100,
                            precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                            override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        prefixLen = 32;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);
        }

        override  = GT_TRUE;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpLpmIpv4UcPrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.4. Call with lpmDBId [0], vrId[0], ipAddr[10.15.0.1], prefixLen[32].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        prefixLen = 32;

        st = cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            /*
                1.5. For Cheetah+ call with lpmDBId [0], vrId[0], ipAddr[10.15.0.1] (already
                deleted), prefixLen[32].
                Expected: NOT GT_OK.
            */
            st = cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

            /*
                1.6. For Cheetah+ call with lpmDBId [0], vrId[0], ipAddr[10.15.0.2] (wrong IP
                address), prefixLen[32].
                Expected: NOT GT_OK.
            */
            ipAddr.arIP[0] = 10;
            ipAddr.arIP[1] = 15;
            ipAddr.arIP[2] = 0;
            ipAddr.arIP[3] = 2;

            st = cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /*
            1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.8. Call with not supported vrId [10] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.9. Call with not multicast ip address range (224-239)
            Expected: GT_BAD_PARAM.
        */
        for(ipAddr.arIP[0] = 224; ipAddr.arIP[0] < 240; ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, lpmDBId, vrId, ipAddr.arIP[0]);
        }

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.10. Call with reserved ip address range (240-255)
            Expected: GT_OK.
        */
        for(ipAddr.arIP[0] = 240; ipAddr.arIP[0] < 255; ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpLpmIpv4UcPrefixAdd: %d, %d, %d, override = %d",
                                         lpmDBId, vrId, prefixLen, override);

            st = cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, lpmDBId, vrId, ipAddr.arIP[0]);
        }

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.11. Call with out of range prefixLen [33] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        prefixLen = 33;

        st = cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d", lpmDBId, prefixLen);

        prefixLen = 32;

        /*
            1.12. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0 / 1] and vrId[0] to
            invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId[0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Call with lpmDBId[1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4UcPrefixDelBulk
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArray
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4UcPrefixDelBulk)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. - 1.2.
    1.3. Call cpssDxChIpLpmIpv4UcPrefixAddBulk with lpmDBId[0],
           ipv4PrefixArrayLen[1],
           ipv4PrefixArray{vrId[0],
                           ipAddr[10.15.0.1],
                           prefixLen[32],
                           nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                           mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                           mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                           enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                           egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                           modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                           profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ]
                           }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                           data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE,
                           policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                           modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                           nestedVlan=GT_FALSE, vlanId=100,
                           precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                           ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE,
                           GT_FALSE } },
                           override[GT_TRUE],
                           returnStatus[0]}
    Expected: GT_OK.
    1.4. Call with lpmDBId[0],
                   ipv4PrefixArrayLen[1],
                   ipv4PrefixArray{vrId[0],
                                   ipAddr[10.15.0.1],
                                   prefixLen[32],
                                   nextHopInfoPtr{pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                                   mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                   mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                                   enableMatchCount = GT_FALSE, matchCounterIndex = 0
                                   }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                                   modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                                   profileAssignIndex=GT_FALSE,
                                   profilePrecedence=GT_FALSE] ] }, redirect {
                                   CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                                   data[routerLttIndex=0] }, policer {
                                   policerEnable=GT_FALSE, policerId=0 }, vlan {
                                   egressTaggedModify=GT_FALSE,
                                   modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                   nestedVlan=GT_FALSE, vlanId=100,
                                   precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
                                   }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE,
                                   GT_FALSE, GT_FALSE } },
                   override[GT_TRUE],
                   returnStatus[0]}
    Expected: GT_OK.
        1.5. For Cheetah+ call with lpmDBId[0],
                ipv4PrefixArrayLen[1], ipv4PrefixArray{vrId[0], ipAddr[10.15.0.1],
                prefixLen[32], nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount =
                GT_FALSE, matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE,
                modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] }, redirect
                { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                egressTaggedModify=GT_FALSE,
                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                nestedVlan=GT_FALSE, vlanId=100,
                precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute
                { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                override[GT_TRUE], returnStatus[0]}
                (already deleted)
        Expected: NOT GT_OK.
    1.6. Call with not supported ipv4PrefixArray->vrId[10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.7. Call with out of range ipv4PrefixArray->prefixLen [33]
         and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.9. Call with not multicast ip address range (224-239)
    Expected: NOT GT_OK.
    1.10. Call with reserved ip address range (240-255)
    Expected: GT_OK.
    1.11. Call with ipv4PrefixArray[NULL] and other parameters from 1.4.
    Expected: GT_BAD_PTR.
    1.12. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0 / 1]
        and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                              lpmDBId     = 0;
    GT_U32                              arrayLen    = 0;
    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC ipv4Prefix;
    GT_U32                              vrId        = 0;
    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC ipv4PrefixArray;
    GT_U32                              prefixLen   = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&ipv4Prefix, sizeof(ipv4Prefix));
    cpssOsBzero((GT_VOID*)&ipv4PrefixArray, sizeof(ipv4PrefixArray));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4UcPrefixAddBulk with lpmDBId[0],
                   ipv4PrefixArrayLen[1],
                   ipv4PrefixArray{vrId[0],
                                   ipAddr[10.15.0.1],
                                   prefixLen[32],
                                   nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                                   mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                   mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                                   enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                                   egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                                   modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                                   profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ]
                                   }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                                   data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE,
                                   policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                                   modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                   nestedVlan=GT_FALSE, vlanId=100,
                                   precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                                   ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE,
                                   GT_FALSE } },
                                   override[GT_TRUE],
                                   returnStatus[0]}
            Expected: GT_OK.
        */
        lpmDBId   = 0;
        prefixLen = 1;

        ipv4PrefixArray.ipAddr.arIP[0] = 10;
        ipv4PrefixArray.ipAddr.arIP[1] = 15;
        ipv4PrefixArray.ipAddr.arIP[2] = 0;
        ipv4PrefixArray.ipAddr.arIP[3] = 1;

        ipv4PrefixArray.prefixLen = 32;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&ipv4PrefixArray.nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &ipv4PrefixArray.nextHopInfo.ipLttEntry);
        }

        ipv4PrefixArray.override                = GT_TRUE;
        ipv4PrefixArray.returnStatus            = GT_OK;
        ipv4PrefixArray.vrId = 0;

        st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, prefixLen);

        /*
            1.4. Call with lpmDBId[0],
                           ipv4PrefixArrayLen[1],
                           ipv4PrefixArray{vrId[0],
                                           ipAddr[10.15.0.1],
                                           prefixLen[32],
                                           nextHopInfoPtr{pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                                           mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                           mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                                           enableMatchCount = GT_FALSE, matchCounterIndex = 0
                                           }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                                           modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                                           profileAssignIndex=GT_FALSE,
                                           profilePrecedence=GT_FALSE] ] }, redirect {
                                           CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                                           data[routerLttIndex=0] }, policer {
                                           policerEnable=GT_FALSE, policerId=0 }, vlan {
                                           egressTaggedModify=GT_FALSE,
                                           modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                           nestedVlan=GT_FALSE, vlanId=100,
                                           precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
                                           }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE,
                                           GT_FALSE, GT_FALSE } },
                           override[GT_TRUE],
                           returnStatus[0]}
            Expected: GT_OK.
        */
        lpmDBId  = 0;
        arrayLen = 1;

        ipv4Prefix.vrId = 0;

        ipv4Prefix.ipAddr.arIP[0] = 10;
        ipv4Prefix.ipAddr.arIP[1] = 15;
        ipv4Prefix.ipAddr.arIP[2] = 0;
        ipv4Prefix.ipAddr.arIP[3] = 1;

        ipv4Prefix.prefixLen = 32;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&ipv4PrefixArray.nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &ipv4PrefixArray.nextHopInfo.ipLttEntry);
        }

        ipv4Prefix.override                = GT_TRUE;
        ipv4Prefix.returnStatus            = GT_OK;

        st = cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDBId, arrayLen, &ipv4Prefix);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, arrayLen);

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            /*
                1.5. For Cheetah+ call with lpmDBId[0],
                        ipv4PrefixArrayLen[1], ipv4PrefixArray{vrId[0], ipAddr[10.15.0.1],
                        prefixLen[32], nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount =
                        GT_FALSE, matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE,
                        modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                        profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] }, redirect
                        { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                        policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                        egressTaggedModify=GT_FALSE,
                        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                        nestedVlan=GT_FALSE, vlanId=100,
                        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute
                        { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                        override[GT_TRUE], returnStatus[0]}
                        (already deleted)
                Expected: NOT GT_OK.
            */
            st = cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDBId, arrayLen, &ipv4Prefix);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, arrayLen);
        }

        /*
            1.6. Call with not supported ipv4PrefixArray->vrId[10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        ipv4Prefix.vrId = 10;

        st = cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDBId, arrayLen, &ipv4Prefix);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "%d, ipv4PrefixArray->vrId = %d", lpmDBId, ipv4Prefix.vrId);

        ipv4Prefix.vrId = 0;

        /*
            1.7. Call with out of range ipv4PrefixArray->prefixLen [33]
                 and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        ipv4Prefix.prefixLen = 33;

        st = cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDBId, arrayLen, &ipv4Prefix);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                    "%d, ipv4PrefixArray->prefixLen = %d", lpmDBId, ipv4Prefix.prefixLen);

        ipv4Prefix.prefixLen = 32;

        /*
            1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDBId, arrayLen, &ipv4Prefix);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.9. Call with not multicast ip address range (224-239)
            Expected: NOT GT_OK.
        */
        for(ipv4Prefix.ipAddr.arIP[0] = 224;
            ipv4Prefix.ipAddr.arIP[0] < 240;
            ipv4Prefix.ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDBId, arrayLen, &ipv4Prefix);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, lpmDBId, prefixLen, ipv4Prefix.ipAddr.arIP[0]);
        }

        ipv4Prefix.ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.10. Call with reserved ip address range (240-255)
            Expected: GT_OK.
        */
        for(ipv4Prefix.ipAddr.arIP[0] = 240;
            ipv4Prefix.ipAddr.arIP[0] < 255;
            ipv4Prefix.ipAddr.arIP[0]++)
        {

            ipv4PrefixArray.ipAddr.arIP[0] = ipv4Prefix.ipAddr.arIP[0];

            st = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, prefixLen, &ipv4PrefixArray);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, prefixLen);

            st = cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDBId, arrayLen, &ipv4Prefix);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, lpmDBId, vrId, ipv4Prefix.ipAddr.arIP[0]);
        }

        ipv4Prefix.ipAddr.arIP[0] = 10; /* restore ip */
        ipv4PrefixArray.ipAddr.arIP[0] = 10;


        /*
            1.11. Call with ipv4PrefixArray[NULL] and other parameters from 1.4.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDBId, arrayLen, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, ipv4PrefixArray = NULL", lpmDBId);

        /*
            1.12. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0 / 1]
                and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId[0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Call with lpmDBId[1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4UcPrefixesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4UcPrefixesFlush)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK Cheetha else GT_BAD_PRT
    1.3. Call with lpmDBId [0], vrId[0].
    Expected: GT_OK.
    1.4. Call with not supported vrId [4096] and other parameters from 1.3.
    Expected: NOT GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                   lpmDBId = 0;
    GT_U32                   vrId    = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call with lpmDBId [0], vrId[0].
            Expected: GT_OK.
        */
        lpmDBId = 1;
        vrId    = 0;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        /*
            1.4. Call with not supported vrId [1] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        vrId = 0;
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        /*
            Call with wrong  vrId [4096].
            Expected: NOT GT_OK.
        */

        vrId = 4096;
        lpmDBId = 0;
        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4UcPrefixSearch
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_IPADDR                               ipAddr,
    IN  GT_U32                                  prefixLen,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT GT_U32                                  *rowIndexPtr,
    OUT GT_U32                                  *columnIndexPtr

)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4UcPrefixSearch)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK Cheetha else GT_BAD_PRT
    1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
        prefixLen[32], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.4. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[32] and non-null nextHopInfoPtr
    Expected: GT_OK and the same nextHopInfoPtr.
    1.5. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[24] and non-null nextHopInfoPtr
    Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.
    1.6. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[24] and non-null nextHopInfoPtr
    Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.
    1.7. For Cheetah+ call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[32] and
        non-null nextHopInfoPtr
    Expected: NOT GT_OK.
    1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.9. Call with not supported vrId [4096] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.10. Call with not multicast ip address range (224-239)
    Expected: NOT GT_OK.
    1.11. Call with reserved ip address range (240-255)
    Expected: GT_OK.
    1.12. Call with out of range prefixLen [33] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.13. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[32] and nextHopInfoPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId     = 0;
    GT_U32                                 vrId        = 0;
    GT_IPADDR                              ipAddr;
    GT_U32                                 prefixLen   = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfoGet;
    GT_BOOL                                override    = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_FALSE;
    GT_U32                                 rowIndex = 0;
    GT_U32                                 columnIndex = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&nextHopInfoGet, sizeof(nextHopInfoGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
                      prefixLen[32], nextHopInfoPtr->pclIpUcAction { pktCmd =
                      CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                      CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                      matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos
                      { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                      ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                      profilePrecedence=GT_FALSE] ] }, redirect {
                      CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                      policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                      egressTaggedModify=GT_FALSE,
                      modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                      vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                      ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                      override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        prefixLen = 32;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);
        }

        override = GT_TRUE;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.4. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[32] and non-null nextHopInfoPtr
            Expected: GT_OK and the same nextHopInfoPtr.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        prefixLen = 32;

        st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                             &nextHopInfoGet,&rowIndex,&columnIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLen);

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfComparePclIpUcActionEntries(lpmDBId, vrId, &nextHopInfoGet.pclIpUcAction,
                                                                &nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfCompareIpLttEntries(lpmDBId, vrId, &nextHopInfoGet.ipLttEntry, &nextHopInfo.ipLttEntry);
        }

        /*
            1.5. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[24] and non-null
                nextHopInfoPtr
            Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.
        */
        prefixLen = 24;

        st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                             &nextHopInfoGet,&rowIndex,&columnIndex);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLen);

        /*
            1.6. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[24] and non-null nextHopInfoPtr
            Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.
        */
        prefixLen = 24;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 255;

        st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                             &nextHopInfoGet,&rowIndex,&columnIndex);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLen);

        /*
            1.7. For Cheetah+ call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[32]
                and non-null nextHopInfoPtr
            Expected: NOT GT_OK.
        */
        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prefixLen = 32;

            st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                                  &nextHopInfoGet,&rowIndex,&columnIndex);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLen);
        }

        /*
            1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                             &nextHopInfoGet,&rowIndex,&columnIndex);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.9. Call with not supported vrId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */

        vrId = 4096;

        st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                             &nextHopInfoGet,&rowIndex,&columnIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.10. Call with not multicast ip address range (224-239)
            Expected: NOT GT_OK.
        */
        for(ipAddr.arIP[0] = 224; ipAddr.arIP[0] < 240; ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                                 &nextHopInfoGet,&rowIndex,&columnIndex);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, lpmDBId, vrId, ipAddr.arIP[0]);
        }

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.11. Call with not reserved ip address range (240-255)
            Expected: GT_OK.
        */
        for(ipAddr.arIP[0] = 240; ipAddr.arIP[0] < 255; ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixAdd: %d, %d, %d, override = %d",
                                         lpmDBId, vrId, prefixLen, override);

            st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                                 &nextHopInfoGet,&rowIndex,&columnIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, lpmDBId, vrId, ipAddr.arIP[0]);
        }

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.12. Call with out of range prefixLen [33] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        prefixLen = 33;

        st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                             &nextHopInfoGet,&rowIndex,&columnIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d", lpmDBId, prefixLen);

        prefixLen = 32;

        /*
            1.13. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[32] and nextHopInfoPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        prefixLen = 32;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                             NULL,&rowIndex,&columnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);

        /*
            1.14. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId    = 0;
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4UcPrefixGet
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_IPADDR                               ipAddr,
    OUT GT_U32                                  *prefixLenPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT GT_U32                                  *rowIndexPtr,
    OUT GT_U32                                  *columnIndexPtr

)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4UcPrefixGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK Cheetha else GT_BAD_PRT
    1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
        prefixLen[32], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.4. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[32] and non-null nextHopInfoPtr
    Expected: GT_OK and the same nextHopInfoPtr.
    1.5. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[24] and non-null nextHopInfoPtr
    Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.
    1.6. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[24] and non-null nextHopInfoPtr
    Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.
    1.7. For Cheetah+ call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[32] and
        non-null nextHopInfoPtr
    Expected: NOT GT_OK.
    1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.9. Call with not supported vrId [4096] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.10. Call with not multicast ip address range (224-239)
    Expected: NOT GT_OK.
    1.11. Call with reserved ip address range (240-255)
    Expected: GT_OK.
    1.12. Call with out of range prefixLen [33] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.13. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[32] and nextHopInfoPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId     = 0;
    GT_U32                                 vrId        = 0;
    GT_IPADDR                              ipAddr;
    GT_U32                                 prefixLen   = 0;
    GT_U32                                 prefixLenGet;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfoGet;
    GT_BOOL                                override    = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_FALSE;
    GT_U32                                 rowIndex;
    GT_U32                                 columnIndex;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                     UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_XCAT2_E | UTF_LION2_E);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&nextHopInfoGet, sizeof(nextHopInfoGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
                      prefixLen[32], nextHopInfoPtr->pclIpUcAction { pktCmd =
                      CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                      CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                      matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos
                      { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                      ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                      profilePrecedence=GT_FALSE] ] }, redirect {
                      CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                      policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                      egressTaggedModify=GT_FALSE,
                      modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                      vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                      ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                      override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        prefixLen = 32;

        prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

        override = GT_TRUE;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.4. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[32] and non-null nextHopInfoPtr
            Expected: GT_OK and the same nextHopInfoPtr and prefixLenPtr.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        prefixLen = 32;

        st = cpssDxChIpLpmIpv4UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          &nextHopInfoGet,&rowIndex,&columnIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLenGet);


        prvUtfCompareIpLttEntries(lpmDBId, vrId, &nextHopInfoGet.ipLttEntry, &nextHopInfo.ipLttEntry);

        UTF_VERIFY_EQUAL1_STRING_MAC(prefixLen, prefixLenGet,
                "got another prefixLen then was set: %d", prefixLenGet);

        /*
            1.6. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[24] and non-null nextHopInfoPtr
            Expected: GT_OK and the same nextHopInfoPtr.
        */
        prefixLen = 24;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 255;

        st = cpssDxChIpLpmIpv4UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          &nextHopInfoGet,&rowIndex,&columnIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(prefixLen, prefixLenGet,
                "got another prefixLen then was set: %d", prefixLenGet);

        /*
            1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          &nextHopInfoGet,&rowIndex,&columnIndex);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.9. Call with not supported vrId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */

        vrId = 4096;

        st = cpssDxChIpLpmIpv4UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          &nextHopInfoGet,&rowIndex,&columnIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.10. Call with not multicast ip address range (224-239)
            Expected: NOT GT_OK.
        */
        for(ipAddr.arIP[0] = 224; ipAddr.arIP[0] < 240; ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                              &nextHopInfoGet,&rowIndex,&columnIndex);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, lpmDBId, vrId, ipAddr.arIP[0]);
        }

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.11. Call with not reserved ip address range (240-255)
            Expected: GT_OK.
        */
        for(ipAddr.arIP[0] = 240; ipAddr.arIP[0] < 255; ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixAdd: %d, %d, %d, override = %d",
                                         lpmDBId, vrId, prefixLen, override);

            st = cpssDxChIpLpmIpv4UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                              &nextHopInfoGet,&rowIndex,&columnIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, lpmDBId, vrId, ipAddr.arIP[0]);
        }

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.13. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[32] and nextHopInfoPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        prefixLen = 32;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        st = cpssDxChIpLpmIpv4UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          NULL,&rowIndex,&columnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);

        /*
            1.14. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId    = 0;
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4UcPrefixGetNext
(
    IN    GT_U32                                    lpmDBId,
    IN    GT_U32                                    vrId,
    INOUT GT_IPADDR                                 *ipAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *nextHopInfoPtr,
    OUT GT_U32                                      *rowIndexPtr,
    OUT GT_U32                                      *columnIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4UcPrefixGetNext)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK Cheetha else GT_BAD_PRT
    1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
        prefixLen[32], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.4. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.2],
        prefixLen[32], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.5. Call with lpmDBId[0], vrId[0], ipAddrPtr [10.15.0.1], prefixLenPtr [32] and non-null
        nextHopInfoPtr
    Expected: GT_OK and the same prefixLenPtr  and nextHopInfoPtr.
    1.6. For Cheetah+ call with lpmDBId[0], vrId[0], ipAddrPtr [10.15.0.2], prefixLenPtr [32]
    and non-null nextHopInfoPtr
    Expected: NOT GT_OK (the last prefix reached).
    1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.8. Call with not supported vrId [4096] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.9. Call with out of range prefixLenPtr [33] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.10. Call with lpmDBId[0], vrId[0], ipAddrPtr [NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
    1.11. Call with not multicast ip address range (224-239)
    Expected: GT_BAD_PARAM.
    1.12. Call with reserved ip address range (240-255)
    Expected: GT_OK.
    1.13. Call with lpmDBId[0], vrId[0], prefixLenPtr [NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
    1.14. Call with lpmDBId[0], vrId[0], nextHopInfoPtr [NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId     = 0;
    GT_U32                                 vrId        = 0;
    GT_IPADDR                              ipAddr;
    GT_IPADDR                              ipAddrTmp;
    GT_U32                                 prefixLen   = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfoGet;
    GT_BOOL                                override    = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_FALSE;
    GT_U32                                 rowIndex = 0;
    GT_U32                                 columnIndex = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&nextHopInfoGet, sizeof(nextHopInfoGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0],
                 vrId[0], ipAddr[10.15.0.1], prefixLen[32], nextHopInfoPtr->pclIpUcAction {
                 pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                 CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                 matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                 egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                 ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                 profilePrecedence=GT_FALSE] ] }, redirect {
                 CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
                 policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                 modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                 vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                 ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                 override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        prefixLen = 32;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);
        }

        override  = GT_TRUE;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.4. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0],
                 vrId[0], ipAddr[10.15.0.2], prefixLen[32], nextHopInfoPtr->pclIpUcAction {
                 pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                 CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                 matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                 egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                 ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                 profilePrecedence=GT_FALSE] ] }, redirect {
                 CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
                 policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                 modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                 vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                 ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                 override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 2;

        prefixLen = 32;

        override  = GT_TRUE;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4UcPrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.5. Call with lpmDBId[0], vrId[0], ipAddrPtr [10.15.0.1],
                 prefixLenPtr [32] and non-null nextHopInfoPtr
            Expected: GT_OK and the same prefixLenPtr  and nextHopInfoPtr.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        ipAddrTmp.arIP[0] = 10;
        ipAddrTmp.arIP[1] = 15;
        ipAddrTmp.arIP[2] = 0;
        ipAddrTmp.arIP[3] = 2;

        prefixLen = 32;

        st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                               &nextHopInfoGet, &rowIndex, &columnIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfComparePclIpUcActionEntries(lpmDBId, vrId,
                                    &nextHopInfoGet.pclIpUcAction, &nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfCompareIpLttEntries(lpmDBId, vrId, &nextHopInfoGet.ipLttEntry,
                                                        &nextHopInfo.ipLttEntry);
        }

        /* Verifying ipAddr */
        UTF_VERIFY_EQUAL2_STRING_MAC(ipAddr.arIP[0], ipAddrTmp.arIP[0],
                   "get another ipAddrPtr[0] than was set: %d, %d", lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipAddr.arIP[1], ipAddrTmp.arIP[1],
                   "get another ipAddrPtr[1] than was set: %d, %d", lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipAddr.arIP[2], ipAddrTmp.arIP[2],
                   "get another ipAddrPtr[2] than was set: %d, %d", lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipAddr.arIP[3], ipAddrTmp.arIP[3],
                   "get another ipAddrPtr[3] than was set: %d, %d", lpmDBId, vrId);

        /*
            1.6. For Cheetah+ call with lpmDBId[0], vrId[0], ipAddrPtr [10.15.0.2],
            prefixLenPtr [32] and non-null nextHopInfoPtr
            Expected: NOT GT_OK (the last prefix reached).
        */
        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            ipAddr.arIP[0] = 10;
            ipAddr.arIP[1] = 15;
            ipAddr.arIP[2] = 0;
            ipAddr.arIP[3] = 2;

            st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                                   &nextHopInfoGet, &rowIndex, &columnIndex);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /*
            1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                              &nextHopInfoGet, &rowIndex, &columnIndex);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.8. Call with not supported vrId [4096] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */

        vrId = 4096;

        st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                               &nextHopInfoGet, &rowIndex, &columnIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.9. Call with out of range prefixLenPtr [33] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        prefixLen = 33;

        st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                               &nextHopInfoGet, &rowIndex, &columnIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d", lpmDBId, prefixLen);

        prefixLen = 32;

        /*
            1.10. Call with lpmDBId[0], vrId[0], ipAddrPtr [NULL] and other parameters from 1.5.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId, vrId, NULL, &prefixLen,
                                              &nextHopInfoGet, &rowIndex, &columnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipAddrPtr = NULL", lpmDBId, vrId);

        /*
            1.11. Call with not multicast ip address range (224-239)
            Expected: GT_BAD_PARAM.
        */
        for(ipAddr.arIP[0] = 224; ipAddr.arIP[0] < 240; ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                                   &nextHopInfoGet, &rowIndex, &columnIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, lpmDBId, vrId, ipAddr.arIP[0]);
        }

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.12. Call with not reserved ip address range (240-255)
            Expected: GT_NOT_FOUND.
        */
        for(ipAddr.arIP[0] = 240; ipAddr.arIP[0] < 255; ipAddr.arIP[0]++)
        {
            st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                                   &nextHopInfoGet, &rowIndex, &columnIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_FOUND, st, lpmDBId, vrId, ipAddr.arIP[0]);
        }

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.13. Call with lpmDBId[0], vrId[0], prefixLenPtr [NULL] and other parameters from 1.5.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId, vrId, &ipAddr, NULL,
                                              &nextHopInfoGet, &rowIndex, &columnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, prefixLenPtr = NULL", lpmDBId, vrId);

        /*
            1.14. Call with lpmDBId[0], vrId[0], nextHopInfoPtr [NULL] and other parameters from 1.5.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                               NULL, &rowIndex, &columnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);

        /*
            1.15. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0]
                  and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId    = 0;
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4McEntryAdd
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPADDR                    *ipGroupPtr,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPADDR                    *ipSrcPtr,
    IN GT_U32                       ipSrcPrefixLen,
    IN CPSS_DXCH_IP_LTT_ENTRY_STC   *mcRouteLttEntryPtr,
    IN GT_BOOL                      override,
    IN GT_BOOL                      defragmentationEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4McEntryAdd)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. - 1.3.
    1.4. Call with lpmDBId[0 / 1], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
        ipSrc[224.16.0.1], ipSrcPrefixLen[32],
        mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
        routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
        defragmentationEnable[GT_FALSE].
    Expected: GT_OK (no any constraints for any devices).
    1.5. Call with lpmDBId [1],
                                vrId[0],
                                wrong enum values mcRouteLttEntryPtr->routeType
                                and othe parameters from 1.4.
    Expected: GT_BAD_PARAM.
    1.6. Call with lpmDBId [1], vrId[0],
                                out of range mcRouteLttEntryPtr->numOfPaths [8 / 63]
                                and othe parameters from 1.4.
    Expected: GT_OK for Lion and above and not GT_OK for other.
    1.7. Call with lpmDBId [1], vrId[0],
                                out of range mcRouteLttEntryPtr->numOfPaths [64]
                                and othe parameters from 1.4.
    Expected: NOT GT_OK
    1.8. Call with lpmDBId [1], vrId[0],
                                out of range mcRouteLttEntryPtr->routeEntryBaseIndex [4096]
                                and othe parameters from 1.4.
    Expected: NOT GT_OK.
    1.9. Call with lpmDBId [1], vrId[0],
                                wrong enum values mcRouteLttEntryPtr->ipv6MCGroupScopeLevel
                                and othe parameters from 1.4.
    Expected: GT_BAD_PARAM.
    1.10. Call with lpmDBId [1], out of range vrId[10] and other valid parameters.
    Expected: NOT GT_OK.
    1.10. Call with lpmDBId [0], out of range vrId[10] and other valid parameters.
    Expected: NOT GT_OK.
    1.11. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.12. Call with out of range ipGroupPrefixLen [33] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.13. Call with out of range ipSrcPrefixLen [33] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.14. Call with lpmDBId [1], vrId[0], mcRouteLttEntryPtr [NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.15. Call cpssDxChIpLpmIpv4McEntriesFlush with with lpmDBId [0 / 1],
          vrId[0] to invalidate entries.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;



    GT_U32                     lpmDBId          = 0;
    GT_U32                     vrId             = 0;
    GT_IPADDR                  ipGroup;
    GT_U32                     ipGroupPrefixLen = 0;
    GT_IPADDR                  ipSrc;
    GT_U32                     ipSrcPrefixLen   = 0;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntry;
    GT_BOOL                    override         = GT_FALSE;
    GT_BOOL                    defragEnable     = GT_FALSE;

    GT_U8                      devList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                     numOfDevs = 1;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        devList[0] = dev;


        /*
            1.1. - 1.3.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.4. Call with lpmDBId[0 / 1], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
                ipSrc[224.16.0.1], ipSrcPrefixLen[32],
                mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
                routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
                sipSaCheckMismatchEnable[GT_TRUE],
                ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
                defragmentationEnable[GT_FALSE].
            Expected: GT_OK (no any constraints for any devices).
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 224;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 32;

        ipSrc.arIP[0] = 224;
        ipSrc.arIP[1] = 15;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 0;

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
        }

        /*
            1.5. Call with lpmDBId [1], vrId[0],
                                        wrong enum values mcRouteLttEntryPtr->routeType
                                        and othe parameters from 1.4.
            Expected: GT_BAD_PARAM.
        */
        lpmDBId = 1;
        vrId    = 0;

        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmIpv4McEntryAdd
                                (lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                 ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable),
                                mcRouteLttEntry.routeType);
        }

        /*
            1.6. Call with lpmDBId [1], vrId[0],
                                        out of range mcRouteLttEntryPtr->numOfPaths [8 / 63]
                                        and othe parameters from 1.4.
            Expected: GT_OK for Lion and above and not GT_OK for other.
        */
        /* call with mcRouteLttEntry.numOfPaths = 8 */
        mcRouteLttEntry.numOfPaths = 8;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                     ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);

        if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcRouteLttEntry.numOfPaths = %d",
                                         lpmDBId, vrId, mcRouteLttEntry.numOfPaths);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcRouteLttEntry.numOfPaths = %d",
                                           lpmDBId, vrId, mcRouteLttEntry.numOfPaths);
        }

        /* call with mcRouteLttEntry.numOfPaths = 63 */
        mcRouteLttEntry.numOfPaths = 63;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                     ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);

        if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcRouteLttEntry.numOfPaths = %d",
                                         lpmDBId, vrId, mcRouteLttEntry.numOfPaths);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcRouteLttEntry.numOfPaths = %d",
                                           lpmDBId, vrId, mcRouteLttEntry.numOfPaths);
        }

        mcRouteLttEntry.numOfPaths = 0;

        /* numOfPaths is not applicable to Lion 3 or Bobcat 2 */
        if ((!PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
            /*
                1.7. Call with lpmDBId [1], vrId[0],
                                            out of range mcRouteLttEntryPtr->numOfPaths [64]
                                            and othe parameters from 1.4.
                Expected: NOT GT_OK
            */
            mcRouteLttEntry.numOfPaths = 64;

            st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);

            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcRouteLttEntry.numOfPaths = %d",
                                           lpmDBId, vrId, mcRouteLttEntry.numOfPaths);
        }

        mcRouteLttEntry.numOfPaths = 0;

        /*
            1.8. Call with lpmDBId [1], vrId[0],
                           out of range mcRouteLttEntryPtr->routeEntryBaseIndex [4096]
                                        and othe parameters from 1.4.
            Expected: NOT GT_OK.
        */
        mcRouteLttEntry.routeEntryBaseIndex =
                            PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerNextHop;


        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_ALREADY_EXIST, st, "%d, numOfDevs = %d", lpmDBId, numOfDevs);

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                     ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, %d, mcRouteLttEntryPtr->routeEntryBaseIndex = %d",
                                         lpmDBId, vrId, mcRouteLttEntry.routeEntryBaseIndex);

        mcRouteLttEntry.routeEntryBaseIndex = 0;

        /*
            1.9. Call with lpmDBId [1], vrId[0],
                                        wrong enum values mcRouteLttEntryPtr->ipv6MCGroupScopeLevel
                                        and othe parameters from 1.4.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmIpv4McEntryAdd
                                (lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                 ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable),
                                mcRouteLttEntry.ipv6MCGroupScopeLevel);
        }

        /*
            1.10. Call with lpmDBId [1], out of range vrId[4096] and other valid parameters.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                     ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.10. Call with lpmDBId [0], out of range vrId[10] and other valid parameters.
            Expected: NOT GT_OK.
        */
        lpmDBId = 0;
        vrId    = 10;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.11. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.12. Call with out of range ipGroupPrefixLen [33] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        ipGroupPrefixLen = 33;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                              ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipGroupPrefixLen = %d",
                                        lpmDBId, ipGroupPrefixLen);

        ipGroupPrefixLen = 32;

        /*
            1.13. Call with out of range ipSrcPrefixLen [33] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        ipSrcPrefixLen = 33;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                        "%d, ipSrcPrefixLen = %d", lpmDBId, ipSrcPrefixLen);

        ipSrcPrefixLen = 32;

        /*
            1.14. Call with lpmDBId [1], vrId[0], mcRouteLttEntryPtr [NULL]
                  and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, NULL, override, defragEnable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                            "%d, %d, mcRouteLttEntryPtr = NULL", lpmDBId, vrId);

        /*
            1.15. Call cpssDxChIpLpmIpv4McEntriesFlush with with lpmDBId [0 / 1],
                  vrId[0] to invalidate entries.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId [0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /* Call with lpmDBId [1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4McEntryDel
(
    IN GT_U32       lpmDBId,
    IN GT_U32       vrId,
    IN GT_IPADDR    *ipGroupPtr,
    IN GT_U32       ipGroupPrefixLen,
    IN GT_IPADDR    *ipSrcPtr,
    IN GT_U32       ipSrcPrefixLen
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4McEntryDel)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [1], vrId[0],
        defUcNextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
        numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, defMcRouteLttEntryPtr {
        routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0], routeEntryBaseIndex[0],
        ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK .
    1.3. Call cpssDxChIpLpmIpv4McEntryAdd with lpmDBId[0], vrId[0], ipGroup[224.15.0.1],
        ipGroupPrefixLen[32], ipSrc[224.16.0.1], ipSrcPrefixLen[32],
        mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
        routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
        defragmentationEnable[GT_FALSE].
    Expected: GT_OK.
    1.4. Call with lpmDBId[0], vrId[0], ipGroup[10.15.0.1], ipGroupPrefixLen[32],
        ipSrc[10.16.0.1], ipSrcPrefixLen[32],.
    Expected: GT_OK.
    1.5. Call with lpmDBId[0], vrId[0], ipGroup[10.15.0.1], ipGroupPrefixLen[32],
        ipSrc[10.16.0.1], ipSrcPrefixLen[32] (already deleted).
    Expected: NOT GT_OK.
    1.6. Call with lpmDBId [0 or 1], out of range vrId[10] and other valid parameters.
    Expected: NOT GT_OK.
    1.7. Call with out of range ipGroupPrefixLen [33] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.8. Call with out of range ipSrcPrefixLen [33] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.9. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.10. Call cpssDxChIpLpmIpv4McEntriesFlush with with lpmDBId [1], vrId[0] to invalidate
        entries.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                     lpmDBId          = 0;
    GT_U32                     vrId             = 0;
    GT_IPADDR                  ipGroup;
    GT_U32                     ipGroupPrefixLen = 0;
    GT_IPADDR                  ipSrc;
    GT_U32                     ipSrcPrefixLen   = 0;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntry;
    GT_BOOL                    override         = GT_FALSE;
    GT_BOOL                    defragEnable     = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4McEntryAdd with lpmDBId[1],
                 vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32], ipSrc[224.16.0.1],
                 ipSrcPrefixLen[32],
                 mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                 numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
                 sipSaCheckMismatchEnable[GT_TRUE],
                 ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                 override[GT_TRUE], defragmentationEnable[GT_FALSE].
            Expected: GT_OK
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 224;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 32;

        ipSrc.arIP[0] = 224;
        ipSrc.arIP[1] = 15;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv4McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                "cpssDxChIpLpmIpv4McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                     lpmDBId, vrId, override, defragEnable);
        }

        /*
            1.4. Call with lpmDBId[0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
                           ipSrc[224.16.0.1], ipSrcPrefixLen[32].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 224;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 32;

        ipSrc.arIP[0] = 224;
        ipSrc.arIP[1] = 15;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        st = cpssDxChIpLpmIpv4McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                                                          ipSrcPrefixLen);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /*
            1.6. Call with lpmDBId [0], out of range vrId[4096] and other valid parameters.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv4McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                                                          ipSrcPrefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.6. Call with lpmDBId [1], out of range vrId[10] and other valid parameters.
                Expected: NOT GT_OK.
        */
        lpmDBId = 1;
        vrId    = 10;

        st = cpssDxChIpLpmIpv4McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.7. Call with out of range ipGroupPrefixLen [33] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        ipGroupPrefixLen = 33;

        st = cpssDxChIpLpmIpv4McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipGroupPrefixLen = %d", lpmDBId, ipGroupPrefixLen);

        ipGroupPrefixLen = 32;

        /*
            1.8. Call with out of range ipSrcPrefixLen [33] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        ipSrcPrefixLen = 33;

        st = cpssDxChIpLpmIpv4McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipSrcPrefixLen = %d", lpmDBId, ipSrcPrefixLen);

        ipSrcPrefixLen = 32;

        /*
            1.9. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        /*
            1.10. Call cpssDxChIpLpmIpv4McEntriesFlush with with lpmDBId [1], vrId[0] to invalidate entries.
            Expected: GT_OK.
        */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4McEntriesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4McEntriesFlush)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0],
        defUcNextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer
        { policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },  defMcRouteLttEntryPtr
        [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK for not Cheetha devices else GT_BAD_PRT.
    1.3. Call with lpmDBId [0], vrId[0].
    Expected: GT_OK.
    1.4. Call with not valid LPM DB id lpmDBId [10] and other parameters from 3.
    Expected: NOT GT_OK.
    1.5. Call with lpmDBId [0 or 1], out of range vrId[4096] and other valid parameters.
    Expected: NOT GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                   lpmDBId = 0;
    GT_U32                   vrId    = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call with lpmDBId [0],
                           vrId[0].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /*
            1.4. Call with not valid LPM DB id lpmDBId [10]
                           and other parameters from 3.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.5. Call with lpmDBId [0],
                           out of range vrId[10] and other valid parameters.
            Expected: NOT GT_OK.
        */
        vrId = 10;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.5. Call with lpmDBId [1],
                               out of range vrId[10]
                               and other valid parameters.
                Expected: NOT GT_OK.
        */
        lpmDBId = 1;
        vrId    = 4096;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4McEntryGetNext
(
    IN    GT_U32                        lpmDBId,
    IN    GT_U32                        vrId,
    INOUT GT_IPADDR                     *ipGroupPtr,
    INOUT GT_U32                        *ipGroupPrefixLenPtr,
    INOUT GT_IPADDR                     *ipSrcPtr,
    INOUT GT_U32                        *ipSrcPrefixLenPtr,
    OUT   CPSS_DXCH_IP_LTT_ENTRY_STC    *mcRouteLttEntryPtr,
    OUT GT_U32                          *gRowIndexPtr,
    OUT GT_U32                          *gColumnIndexPtr,
    OUT GT_U32                          *sRowIndexPtr,
    OUT GT_U32                          *sColumnIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4McEntryGetNext)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0],
        defUcNextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
        numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, defMcRouteLttEntryPtr {
        routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0], routeEntryBaseIndex[0],
        ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK .
    1.3. Call cpssDxChIpLpmIpv4McEntryAdd for 10 times with lpmDBId[0], vrId[0],
        ipGroup[224.15.0.1], ipGroupPrefixLen[32], ipSrc[224.16.0.1], ipSrcPrefixLen[32],
        mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
        routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
        defragmentationEnable[GT_FALSE].
    Expected: GT_OK.
    1.4. Call with lpmDBId[0], vrId[0], ipGroupPtr[224.15.0.1...9], ipGroupPrefixLenPtr [32],
        ipSrcPtr[224.16.0.1...9], ipSrcPrefixLenPtr[32] and non-null mcRouteLttEntryPtr.
    Expected: GT_OK and ipGroupPtr[224.15.0.2...10], ipGroupPrefixLenPtr[32],
        ipSrcPtr[224.16.0.2...10], ipSrcPrefixLenPtr[32] and the same mcRouteLttEntryPtr.
    1.5. Call with lpmDBId [0 or 1], out of range vrId[10] and other valid parameters.
    Expected: NOT GT_OK.
    1.6. Call with out of range ipGroupPrefixLenPtr [33] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.7. Call with out of range ipSrcPrefixLenPtr [33] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.9. Call with lpmDBId [0], vrId[0], ipGroupPtr[NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.10. Call with lpmDBId [0], vrId[0], ipGroupPrefixLenPtr [NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.11. Call with lpmDBId [0], vrId[0], ipSrcPtr [NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.12. Call with lpmDBId [0], vrId[0], ipSrcPrefixLenPtr [NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.13. Call with lpmDBId [0], vrId[0], mcRouteLttEntryPtr [NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.14. Call cpssDxChIpLpmIpv4McEntriesFlush with with lpmDBId [0 / 1], vrId[0] to invalidate entries.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                     lpmDBId          = 0;
    GT_U32                     vrId             = 0;
    GT_IPADDR                  ipGroup;
    GT_U32                     ipGroupPrefixLen = 0;
    GT_IPADDR                  ipSrc;
    GT_U32                     ipSrcPrefixLen   = 0;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntry;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntryGet;
    GT_BOOL                    override         = GT_FALSE;
    GT_BOOL                    defragEnable     = GT_FALSE;
    GT_U8                      iter             = 0;
    GT_U32                          gRowIndex = 0;
    GT_U32                          gColumnIndex = 0;
    GT_U32                          sRowIndex = 0;
    GT_U32                          sColumnIndex = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    cpssOsBzero((GT_VOID*)&mcRouteLttEntryGet, sizeof(mcRouteLttEntryGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4McEntryAdd for 10 times with lpmDBId[0],
                 vrId[0], ipGroup[224.15.0.1..10], ipGroupPrefixLen[32], ipSrc[224.16.0.1..10],
                 ipSrcPrefixLen[32],
                 mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                 numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
                 sipSaCheckMismatchEnable[GT_TRUE],
                 ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                 override[GT_TRUE], defragmentationEnable[GT_FALSE].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 224;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 32;

        ipSrc.arIP[0] = 224;
        ipSrc.arIP[1] = 16;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        /* add ipGroup[224.15.0.1], ipSrc[224.16.0.1] */
        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                             ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv4McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv4McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
        }


        for (iter = 0; iter < 10; iter++)
        {
            ipGroup.arIP[3] = (GT_U8)(ipGroup.arIP[3] + 1);

            ipSrc.arIP[3] = (GT_U8)(ipSrc.arIP[3] + 1);

            st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                             ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
            if (prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                        "cpssDxChIpLpmIpv4McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
            }
            else
            {
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                        "cpssDxChIpLpmIpv4McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
            }
        }

        /*
            1.4. Call with lpmDBId[0], vrId[0], ipGroupPtr[224.15.0.1], ipGroupPrefixLenPtr
                [32], ipSrcPtr[224.16.0.1], ipSrcPrefixLenPtr[32] and non-null mcRouteLttEntryPtr.
            Expected: GT_OK and ipGroupPtr[10.15.0.2...10], ipGroupPrefixLenPtr[32],
                ipSrcPtr[10.16.0.2...10], ipSrcPrefixLenPtr[32] and the same mcRouteLttEntryPtr.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 224;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 32;

        ipSrc.arIP[0] = 224;
        ipSrc.arIP[1] = 16;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        for (iter = 0; iter < 10; iter++)
        {
            st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                                 &ipSrcPrefixLen, &mcRouteLttEntryGet,
                                                 &gRowIndex,&gColumnIndex, &sRowIndex,&sColumnIndex);
            if (prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

                if (GT_OK == st)
                {
                    /* Verifying values */
                    prvUtfCompareIpLttEntries(lpmDBId, vrId, &mcRouteLttEntryGet, &mcRouteLttEntry);

                    UTF_VERIFY_EQUAL2_STRING_MAC(iter+2 , ipGroup.arIP[3],
                               "get another ipGroup.arIP[3] than was set: %d, %d", lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(iter+2, ipSrc.arIP[3],
                               "get another ipSrc.arIP[3] than was set: %d, %d", lpmDBId, vrId);
                }
            }
        }

        /* check that when ipGroup=0.0.0.0 we get the first MC entry in table and GT_OK */

        ipGroup.arIP[0] = 0;
        ipGroup.arIP[1] = 0;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 0;
        ipGroupPrefixLen = 0;

        ipSrc.arIP[0] = 0;
        ipSrc.arIP[1] = 0;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 0;
        ipSrcPrefixLen = 0;

        st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                             &ipSrcPrefixLen, &mcRouteLttEntryGet,
                                             &gRowIndex,&gColumnIndex, &sRowIndex,&sColumnIndex);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

            if (GT_OK == st)
            {
                /* Verifying values */
                prvUtfCompareIpLttEntries(lpmDBId, vrId, &mcRouteLttEntryGet, &mcRouteLttEntry);

                UTF_VERIFY_EQUAL2_STRING_MAC(224 , ipGroup.arIP[0],
                           "get another ipGroup.arIP[3] than was set: %d, %d", lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(224, ipSrc.arIP[0],
                           "get another ipSrc.arIP[3] than was set: %d, %d", lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(15 , ipGroup.arIP[1],
                           "get another ipGroup.arIP[3] than was set: %d, %d", lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(16, ipSrc.arIP[1],
                           "get another ipSrc.arIP[3] than was set: %d, %d", lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(0 , ipGroup.arIP[2],
                           "get another ipGroup.arIP[3] than was set: %d, %d", lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(0, ipSrc.arIP[2],
                           "get another ipSrc.arIP[3] than was set: %d, %d", lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(1 , ipGroup.arIP[3],
                           "get another ipGroup.arIP[3] than was set: %d, %d", lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(1, ipSrc.arIP[3],
                           "get another ipSrc.arIP[3] than was set: %d, %d", lpmDBId, vrId);
            }

        }
        /*
            1.5. Call with lpmDBId [0], out of range vrId[4096] and other valid parameters.
            Expected: NOT GT_OK.
        */
        ipGroup.arIP[0] = 224;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 32;

        ipSrc.arIP[0] = 224;
        ipSrc.arIP[1] = 16;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        vrId = 4096;

        st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                         &ipSrcPrefixLen, &mcRouteLttEntryGet,&gRowIndex,
                                         &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.5. Call with lpmDBId [1], out of range vrId[4096] and other valid parameters.
            Expected: NOT GT_OK.
        */
        lpmDBId = 1;
        vrId    = 4096;

        st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                             &ipSrcPrefixLen, &mcRouteLttEntryGet,&gRowIndex,
                                             &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.6. Call with out of range ipGroupPrefixLenPtr [33] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        ipGroupPrefixLen = 33;

        st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                             &ipSrcPrefixLen, &mcRouteLttEntryGet,&gRowIndex,
                                             &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipGroupPrefixLenPtr = %d",
                                                        lpmDBId, ipGroupPrefixLen);

        ipGroupPrefixLen = 32;

        /*
            1.7. Call with out of range ipSrcPrefixLenPtr [33] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        ipSrcPrefixLen = 33;

        st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                             &ipSrcPrefixLen, &mcRouteLttEntryGet,&gRowIndex,
                                             &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipSrcPrefixLenPtr = %d",
                                                        lpmDBId, ipSrcPrefixLen);

        ipSrcPrefixLen = 32;

        /*
            1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                             &ipSrcPrefixLen, &mcRouteLttEntryGet,&gRowIndex,
                                             &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.9. Call with lpmDBId [0], vrId[0], ipGroupPtr[NULL] and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, NULL, &ipGroupPrefixLen, &ipSrc,
                                             &ipSrcPrefixLen, &mcRouteLttEntryGet,&gRowIndex,
                                             &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipGroupPtr = NULL", lpmDBId, vrId);

        /*
            1.10. Call with lpmDBId [0], vrId[0], ipGroupPrefixLenPtr [NULL]
                  and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, &ipGroup, NULL, &ipSrc,
                                             &ipSrcPrefixLen, &mcRouteLttEntryGet,&gRowIndex,
                                             &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipGroupPrefixLenPtr = NULL",
                                                                            lpmDBId, vrId);

        /*
            1.11. Call with lpmDBId [0], vrId[0], ipSrcPtr [NULL] and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, NULL,
                                             &ipSrcPrefixLen, &mcRouteLttEntryGet,&gRowIndex,
                                             &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipSrcPtr = NULL", lpmDBId, vrId);

        /*
            1.12. Call with lpmDBId [0], vrId[0], ipSrcPrefixLenPtr [NULL] and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                             NULL, &mcRouteLttEntryGet,&gRowIndex,
                                             &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipSrcPrefixLenPtr = NULL", lpmDBId, vrId);

        /*
            1.13. Call with lpmDBId [0], vrId[0], mcRouteLttEntryPtr [NULL] and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                             &ipSrcPrefixLen, NULL,&gRowIndex,
                                             &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, mcRouteLttEntryPtr = NULL", lpmDBId, vrId);

        /*
            1.14. Call cpssDxChIpLpmIpv4McEntriesFlush with with lpmDBId [0 / 1],
                                                                 vrId[0] to invalidate entries.
            Expected: GT_OK.
        */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /* Call with lpmDBId [1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4McEntrySearch
(
    IN  GT_U32                      lpmDBId,
    IN  GT_U32                      vrId,
    IN  GT_IPADDR                   *ipGroupPtr,
    IN  GT_U32                      ipGroupPrefixLen,
    IN  GT_IPADDR                   *ipSrcPtr,
    IN  GT_U32                      ipSrcPrefixLen,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC  *mcRouteLttEntryPtr,
    OUT GT_U32                      *gRowIndexPtr,
    OUT GT_U32                      *gColumnIndexPtr,
    OUT GT_U32                      *sRowIndexPtr,
    OUT GT_U32                      *sColumnIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4McEntrySearch)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0],
        defUcNextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
        numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, defMcRouteLttEntryPtr {
        routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0], routeEntryBaseIndex[0],
        ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK .
    1.4. Call cpssDxChIpLpmIpv4McEntryAdd for 10 times with lpmDBId[0], vrId[0],
        ipGroup[224.15.0.1], ipGroupPrefixLen[32], ipSrc[224.16.0.1], ipSrcPrefixLen[32],
        mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
        routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
        defragmentationEnable[GT_FALSE].
    Expected: GT_OK.
    1.5. Call with lpmDBId[0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen [32],
        ipSrc[224.16.0.1], ipSrcPrefixLen[32] and non-null mcRouteLttEntry.
    Expected: GT_OK and the same mcRouteLttEntryPtr.
    1.6. Call with lpmDBId [0 or 1], out of range vrId[10] and other valid parameters.
    Expected: NOT GT_OK.
    1.7. Call with out of range ipGroupPrefixLen [33] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.8. Call with out of range ipSrcPrefixLen [33] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.9. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.10. Call with lpmDBId [0], vrId[0], mcRouteLttEntryPtr [NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.11. Call cpssDxChIpLpmIpv4McEntriesFlush with with lpmDBId [0/1], vrId[0] to invalidate entries.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                     lpmDBId          = 0;
    GT_U32                     vrId             = 0;
    GT_IPADDR                  ipGroup;
    GT_U32                     ipGroupPrefixLen = 0;
    GT_IPADDR                  ipSrc;
    GT_U32                     ipSrcPrefixLen   = 0;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntry;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntryGet;
    GT_BOOL                    override         = GT_FALSE;
    GT_BOOL                    defragEnable     = GT_FALSE;
    GT_U8                      iter             = 0;
    OUT GT_U32                 gRowIndex  = 0;
    OUT GT_U32                 gColumnIndex = 0;
    OUT GT_U32                 sRowIndex = 0;
    OUT GT_U32                 sColumnIndex = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4McEntryAdd for 10 times with lpmDBId[0],
                 vrId[0], ipGroup[224.15.0.1..10], ipGroupPrefixLen[32], ipSrc[224.16.0.1..10],
                 ipSrcPrefixLen[32],
                 mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                 numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
                 sipSaCheckMismatchEnable[GT_TRUE],
                 ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                 override[GT_TRUE], defragmentationEnable[GT_FALSE].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 224;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 32;

        ipSrc.arIP[0] = 224;
        ipSrc.arIP[1] = 16;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        /* add ipGroup[224.15.0.1],ipSrc[224.16.0.1] */
        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv4McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv4McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
        }

        for (iter = 0; iter < 10; iter++)
        {
            ipGroup.arIP[3] = (GT_U8) (ipGroup.arIP[3] + 1);

            ipSrc.arIP[3] = (GT_U8) (ipSrc.arIP[3] + 1);

            st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                             ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
            if (prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                        "cpssDxChIpLpmIpv4McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
            }
            else
            {
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                        "cpssDxChIpLpmIpv4McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
            }
        }

        /*
            1.5. Call with lpmDBId[0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen [32],
                ipSrc[224.16.0.1], ipSrcPrefixLen[32] and non-null mcRouteLttEntry.
            Expected: GT_OK and the same mcRouteLttEntryPtr.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 224;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 32;

        ipSrc.arIP[0] = 224;
        ipSrc.arIP[1] = 16;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        for (iter = 0; iter < 10; iter++)
        {
            st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                                ipSrcPrefixLen, &mcRouteLttEntryGet,&gRowIndex,
                                                &gColumnIndex, &sRowIndex,&sColumnIndex);
            if (prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

                prvUtfCompareIpLttEntries(lpmDBId,vrId, &mcRouteLttEntryGet, &mcRouteLttEntry);
            }
        }

        /*
            1.6. Call with lpmDBId [0], out of range vrId[4096] and other valid parameters.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                        ipSrcPrefixLen, &mcRouteLttEntry,&gRowIndex,
                                         &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.7. Call with lpmDBId [1], out of range vrId[4096] and other valid parameters.
            Expected: NOT GT_OK.
        */
        lpmDBId = 1;
        vrId    = 4096;

        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                            ipSrcPrefixLen, &mcRouteLttEntry,&gRowIndex,
                                            &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.8. Call with out of range ipGroupPrefixLen [33] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        ipGroupPrefixLen = 33;

        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                            ipSrcPrefixLen, &mcRouteLttEntry,&gRowIndex,
                                            &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipGroupPrefixLen = %d", lpmDBId, ipGroupPrefixLen);

        ipGroupPrefixLen = 32;

        /*
            1.9. Call with out of range ipSrcPrefixLen [33] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        ipSrcPrefixLen = 33;

        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                            ipSrcPrefixLen, &mcRouteLttEntry,&gRowIndex,
                                            &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipSrcPrefixLen = %d", lpmDBId, ipSrcPrefixLen);

        ipSrcPrefixLen = 32;

        /*
            1.10. Call with lpmDBId [0], vrId[0], mcRouteLttEntryPtr [NULL] and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                            ipSrcPrefixLen, NULL,&gRowIndex,
                                             &gColumnIndex, &sRowIndex,&sColumnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, mcRouteLttEntryPtr = NULL", lpmDBId, vrId);

        /*
            1.11. Call cpssDxChIpLpmIpv4McEntriesFlush with with lpmDBId [0/1], vrId[0] to invalidate entries.
            Expected: GT_OK.
        */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        lpmDBId = 1;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv4McEntrySearch
(
    IN  GT_U32                      lpmDBId,
    IN  GT_U32                      vrId,
    IN  GT_IPADDR                   *ipGroupPtr,
    IN  GT_U32                      ipGroupPrefixLen,
    IN  GT_IPADDR                   *ipSrcPtr,
    IN  GT_U32                      ipSrcPrefixLen,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC  *mcRouteLttEntryPtr,
    OUT GT_U32                      *gRowIndexPtr,
    OUT GT_U32                      *gColumnIndexPtr,
    OUT GT_U32                      *sRowIndexPtr,
    OUT GT_U32                      *sColumnIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4McEntryDelSearch)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0],
        defUcNextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
        numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, defMcRouteLttEntryPtr {
        routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0], routeEntryBaseIndex[0],
        ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK .
    1.3. Call cpssDxChIpLpmIpv4McEntryAdd with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[149.218.213.102], ipSrcPrefixLen[32],
                 mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                    numOfPaths[0], routeEntryBaseIndex[0],
                                    ucRPFCheckEnable[GT_TRUE],
                                    sipSaCheckMismatchEnable[GT_TRUE],
                                    ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                 override[GT_TRUE], defragmentationEnable[GT_FALSE].
    Expected: GT_OK.
    1.4. Call cpssDxChIpLpmIpv4McEntryAdd with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[10.99.0.102], ipSrcPrefixLen[32],
                 mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                    numOfPaths[0], routeEntryBaseIndex[0],
                                    ucRPFCheckEnable[GT_TRUE],
                                    sipSaCheckMismatchEnable[GT_TRUE],
                                    ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                 override[GT_TRUE], defragmentationEnable[GT_FALSE].
    Expected: GT_OK.
    1.5. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[149.218.213.102], ipSrcPrefixLen[32],
                 and non-null mcRouteLttEntry.
    Expected: GT_OK and the same mcRouteLttEntryPtr.
    1.6. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[10.99.0.102], ipSrcPrefixLen[32],
                 and non-null mcRouteLttEntry.
    Expected: GT_OK and the same mcRouteLttEntryPtr.
    1.7. Call cpssDxChIpLpmIpv4McEntryDel with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[10.99.0.102], ipSrcPrefixLen[32].
    Expected: GT_OK.
    1.8. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[149.218.213.102], ipSrcPrefixLen[32],
                 and non-null mcRouteLttEntry.
    Expected: GT_OK and the same mcRouteLttEntryPtr.
    1.9. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[10.99.0.102], ipSrcPrefixLen[32],
                 and non-null mcRouteLttEntry.
    Expected: NOT GT_OK.
    1.10. Call cpssDxChIpLpmIpv4McEntryAdd with lpmDBId[0], vrId[0],
                  ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                  ipSrc[10.99.0.102], ipSrcPrefixLen[32],
                  mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                     numOfPaths[0], routeEntryBaseIndex[0],
                                     ucRPFCheckEnable[GT_TRUE],
                                     sipSaCheckMismatchEnable[GT_TRUE],
                                     ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                  override[GT_TRUE], defragmentationEnable[GT_FALSE].
    Expected: GT_OK.
    1.11. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                  ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                  ipSrc[149.218.213.102], ipSrcPrefixLen[32],
                  and non-null mcRouteLttEntry.
    Expected: GT_OK and the same mcRouteLttEntryPtr.
    1.12. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                  ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                  ipSrc[10.99.0.102], ipSrcPrefixLen[32],
                  and non-null mcRouteLttEntry.
    Expected: GT_OK and the same mcRouteLttEntryPtr.
    1.13. Call cpssDxChIpLpmIpv4McEntriesFlush with with lpmDBId [0/1], vrId[0] to invalidate entries.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                     lpmDBId          = 0;
    GT_U32                     vrId             = 0;
    GT_U32                     ipGroupPrefixLen = 0;
    GT_U32                     ipSrcPrefixLen   = 0;
    GT_BOOL                    override         = GT_FALSE;
    GT_BOOL                    defragEnable     = GT_FALSE;
    GT_U32                     gRowIndex        = 0;
    GT_U32                     gColumnIndex     = 0;
    GT_U32                     sRowIndex        = 0;
    GT_U32                     sColumnIndex     = 0;
    GT_IPADDR                  ipGroup = {0};
    GT_IPADDR                  ipSrc0  = {0};
    GT_IPADDR                  ipSrc1  = {0};
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntry    = {0};
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntryGet = {0};


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4McEntryAdd with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[149.218.213.102], ipSrcPrefixLen[32],
                 mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                    numOfPaths[0], routeEntryBaseIndex[0],
                                    ucRPFCheckEnable[GT_TRUE],
                                    sipSaCheckMismatchEnable[GT_TRUE],
                                    ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                 override[GT_TRUE], defragmentationEnable[GT_FALSE].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 225;
        ipGroup.arIP[1] = 1;
        ipGroup.arIP[2] = 1;
        ipGroup.arIP[3] = 17;

        ipGroupPrefixLen = 32;

        ipSrc0.arIP[0] = 149;
        ipSrc0.arIP[1] = 218;
        ipSrc0.arIP[2] = 213;
        ipSrc0.arIP[3] = 102;

        ipSrcPrefixLen = 32;

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc0,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, lpmDBId, vrId, override, defragEnable);
        }

        /*
            1.4. Call cpssDxChIpLpmIpv4McEntryAdd with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[10.99.0.102], ipSrcPrefixLen[32],
                 mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                    numOfPaths[0], routeEntryBaseIndex[0],
                                    ucRPFCheckEnable[GT_TRUE],
                                    sipSaCheckMismatchEnable[GT_TRUE],
                                    ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                 override[GT_TRUE], defragmentationEnable[GT_FALSE].
            Expected: GT_OK.
        */
        ipSrc1.arIP[0] = 10;
        ipSrc1.arIP[1] = 99;
        ipSrc1.arIP[2] = 0;
        ipSrc1.arIP[3] = 102;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc1,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, lpmDBId, vrId, override, defragEnable);
        }

        /*
            1.5. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[149.218.213.102], ipSrcPrefixLen[32],
                 and non-null mcRouteLttEntry.
            Expected: GT_OK and the same mcRouteLttEntryPtr.
        */
        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc0,
                                            ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex,
                                            &gColumnIndex, &sRowIndex, &sColumnIndex);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /*
            1.6. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[10.99.0.102], ipSrcPrefixLen[32],
                 and non-null mcRouteLttEntry.
            Expected: GT_OK and the same mcRouteLttEntryPtr.
        */
        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc1,
                                            ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex,
                                            &gColumnIndex, &sRowIndex, &sColumnIndex);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /*
            1.7. Call cpssDxChIpLpmIpv4McEntryDel with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[10.99.0.102], ipSrcPrefixLen[32].
            Expected: GT_OK.
        */
        st = cpssDxChIpLpmIpv4McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                         &ipSrc1, ipSrcPrefixLen);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /*
            1.8. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[149.218.213.102], ipSrcPrefixLen[32],
                 and non-null mcRouteLttEntry.
            Expected: GT_OK and the same mcRouteLttEntryPtr.
        */
        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc0,
                                            ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex,
                                            &gColumnIndex, &sRowIndex, &sColumnIndex);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

            prvUtfCompareIpLttEntries(lpmDBId,vrId, &mcRouteLttEntryGet, &mcRouteLttEntry);
        }

        /*
            1.9. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                 ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                 ipSrc[10.99.0.102], ipSrcPrefixLen[32],
                 and non-null mcRouteLttEntry.
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc1,
                                            ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex,
                                            &gColumnIndex, &sRowIndex, &sColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.10. Call cpssDxChIpLpmIpv4McEntryAdd with lpmDBId[0], vrId[0],
                  ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                  ipSrc[10.99.0.102], ipSrcPrefixLen[32],
                  mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                     numOfPaths[0], routeEntryBaseIndex[0],
                                     ucRPFCheckEnable[GT_TRUE],
                                     sipSaCheckMismatchEnable[GT_TRUE],
                                     ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                  override[GT_TRUE], defragmentationEnable[GT_FALSE].
            Expected: GT_OK.
        */
        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc1,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, lpmDBId, vrId, override, defragEnable);
        }

        /*
            1.11. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                  ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                  ipSrc[149.218.213.102], ipSrcPrefixLen[32],
                  and non-null mcRouteLttEntry.
            Expected: GT_OK and the same mcRouteLttEntryPtr.
        */
        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc0,
                                            ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex,
                                            &gColumnIndex, &sRowIndex, &sColumnIndex);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

            prvUtfCompareIpLttEntries(lpmDBId,vrId, &mcRouteLttEntryGet, &mcRouteLttEntry);
        }

        /*
            1.12. Call cpssDxChIpLpmIpv4McEntrySearch with lpmDBId[0], vrId[0],
                  ipGroup[225.1.1.17], ipGroupPrefixLen[32],
                  ipSrc[10.99.0.102], ipSrcPrefixLen[32],
                  and non-null mcRouteLttEntry.
            Expected: GT_OK and the same mcRouteLttEntryPtr.
        */
        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc1,
                                            ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex,
                                            &gColumnIndex, &sRowIndex, &sColumnIndex);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

            prvUtfCompareIpLttEntries(lpmDBId,vrId, &mcRouteLttEntryGet, &mcRouteLttEntry);
        }

        /*
            1.13. Call cpssDxChIpLpmIpv4McEntriesFlush with with lpmDBId [0/1], vrId[0] to invalidate entries.
            Expected: GT_OK.
        */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        lpmDBId = 1;

        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6UcPrefixAdd
(
    IN GT_U32                                   lpmDBId,
    IN GT_U32                                   vrId,
    IN GT_IPV6ADDR                              *ipAddrPtr,
    IN GT_U32                                   prefixLen,
    IN CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT   *nextHopInfoPtr,
    IN GT_BOOL                                  override,
    IN GT_BOOL                                  defragmentationEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6UcPrefixAdd)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0 / 1],
        protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E], indexesRangePtr{ firstIndex [0], lastIndex
        [1024]}, partitionEnable[GT_TRUE], tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10],
        numOfIpv4McSourcePrefixes [5], numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK for Cheetha devices else GT_BAD_PRT
    1.3. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [1], vrId[0],
        defUcNextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
        numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, defMcRouteLttEntryPtr {
        routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0], routeEntryBaseIndex[0],
        ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK .
    1.4. Call with lpmDBId [0], vrId[0], ipAddr[10.15.0.1], prefixLen[128],
        nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
        CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
        enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE,
        modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
        profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] }, redirect {
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.5. Call with lpmDBId [1], vrId[0], ipAddr[10.16.0.1], prefixLen[128],
        nextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
        routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK.
    1.6. Call with override [GT_FALSE] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.8. Call with not supported vrId [4096] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.9. Call with out of range prefixLen [129] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.10. For not Cheetah+ call with lpmDBId [1] out of range
        nextHopInfoPtr->ipLttEntry.routeType [wrong enum values] and other parameters from 1.5.
    Expected: GT_BAD_PARAM.
    1.11. Call with lpmDBId [1], correct nextHopInfoPtr->ipLttEntry.numOfPaths
                     ([5 / 7] - for lion and above and [49 / 63] for other)
                     and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.12. Call with lpmDBId [1],
                     out of range nextHopInfoPtr->ipLttEntry.numOfPaths
                     ([64 / 65] - for lion and above and [8 / 9] for other)
                     and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.13. Call with lpmDBId [1] out of range nextHopInfoPtr->ipLttEntry.
        numOfPaths[routeEntryBaseIndex] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.14. Call with lpmDBId [1] out of range nextHopInfoPtr->ipLttEntry. ipv6MCGroupScopeLevel
        [wrong enum values] and other parameters from 1.5.
    Expected: GT_BAD_PARAM
    1.15. Call with lpmDBId [1] nextHopInfoPtr[NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
    1.16. For Cheetah+ call with lpmDBId [0] out of range
        nextHopInfoPtr->pclIpUcAction.pktCmdp[wrong enum values] and other parameters from 1.4.
    Expected: GT_BAD_PARAM.
    1.17. Call with not multicast ip address range (255)
    Expected: GT_BAD_PARAM.
    1.18. Call cpssDxChIpLpmIpv6UcPrefixesFlush with lpmDBId [0 / 1] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId      = 0;
    GT_U32                                 vrId         = 0;
    GT_IPV6ADDR                            ipAddr;
    GT_U32                                 prefixLen    = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override     = GT_FALSE;
    GT_BOOL                                defragEnable = GT_FALSE;

    cpssOsBzero((GT_VOID*) &ipAddr, sizeof(ipAddr));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.3.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.4. Call with lpmDBId [0], vrId[0], ipAddr[10.15.0.1], prefixLen[32],
               nextHopInfoPtr->pclIpUcAction{pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                                             mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                                    mirrorToRxAnalyzerPort = GT_FALSE},
                                             matchCounter{enableMatchCount = GT_FALSE,
                                                          matchCounterIndex = 0},
                                             qos{egressPolicy = GT_FALSE,
                                                 modifyDscp = GT_FALSE,
                                                 modifyUp = GT_FALSE,
                                                 qos [ ingress[profileIndex=0,
                                                 profileAssignIndex=GT_FALSE,
                                                 profilePrecedence=GT_FALSE] ] },
                                             redirect {CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                                             data[routerLttIndex=0] },
                                             policer { policerEnable=GT_FALSE, policerId=0 },
                                             vlan {egressTaggedModify=GT_FALSE,
                                                   modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                                   nestedVlan=GT_FALSE,
                                                   vlanId=100,
                                                   precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                                              ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
               override [GT_TRUE].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        prefixLen = 128;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);
        }

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragEnable);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, override = %d, defragmentationEnable = %d",
                                     lpmDBId, vrId, prefixLen, override, defragEnable);

        /*
            1.5. Call with lpmDBId [1], vrId[0], ipAddr[10.16.0.1], prefixLen[128],
                   nextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                              numOfPaths[0],
                                              routeEntryBaseIndex[0],
                                              ucRPFCheckEnable[GT_TRUE],
                                              sipSaCheckMismatchEnable[GT_TRUE],
                                              ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
            Expected: GT_OK.
        */
        lpmDBId = 1;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 16;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo,
                                                                    override, defragEnable);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, override = %d, defragmentationEnable = %d",
                                     lpmDBId, vrId, prefixLen, override, defragEnable);

        /*
            1.6. Call with override [GT_FALSE] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        override = GT_FALSE;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo,
                                                                    override, defragEnable);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, override = %d", lpmDBId, vrId, override);

        override = GT_TRUE;

        /*
            1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo,
                                                                    override, defragEnable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.8. Call with not supported vrId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */

        vrId = 4096;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo,
                                                                    override, defragEnable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.9. Call with out of range prefixLen [129] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        prefixLen = 129;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo,
                                                                    override, defragEnable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d", lpmDBId, prefixLen);

        prefixLen = 128;

        if (CPSS_PP_FAMILY_CHEETAH_E != PRV_CPSS_PP_MAC(dev)->devFamily && !prvUtfIsPbrModeUsed())
        {
            /*
                1.10. For not Cheetah+ call with lpmDBId [1]
                                wrong enum values nextHopInfoPtr->ipLttEntry.routeType
                                and other parameters from 1.5.
                Expected: GT_BAD_PARAM.
            */
            lpmDBId = 1;

            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmIpv6UcPrefixAdd
                                (lpmDBId, vrId, &ipAddr, prefixLen,
                                 &nextHopInfo, override, defragEnable),
                                nextHopInfo.ipLttEntry.routeType);

            /* numOfPaths is not applicable for SIP5 */
            if ((!PRV_CPSS_SIP_5_CHECK_MAC(dev)))
            {
                /*
                    1.11. Call with lpmDBId [1], correct nextHopInfoPtr->ipLttEntry.numOfPaths
                         ([5 / 7] - for lion and above and [49 / 63] for other)
                                     and other parameters from 1.5.
                    Expected: GT_OK.
                */
                /*call with 49 or 5 values */
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 49;
                }
                else
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 5;
                }

                st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo,
                                                                         override, defragEnable);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                          lpmDBId, vrId, nextHopInfo.ipLttEntry.numOfPaths);

                /*call with 63 or 7 values */
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 63;
                }
                else
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 7;
                }

                st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo,
                                                                         override, defragEnable);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                          lpmDBId, vrId, nextHopInfo.ipLttEntry.numOfPaths);

                /*
                    1.12. Call with lpmDBId [1],
                                     out of range nextHopInfoPtr->ipLttEntry.numOfPaths
                                     ([64 / 65] - for lion and above and [8 / 9] for other)
                                     and other parameters from 1.5.
                    Expected: NOT GT_OK.
                */
                /*call with 64 or 8 values */
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 64;
                }
                else
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 8;
                }

                st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo,
                                                                         override, defragEnable);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                          lpmDBId, vrId, nextHopInfo.ipLttEntry.numOfPaths);

                /*call with 65 or 9 values */
                if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 65;
                }
                else
                {
                    nextHopInfo.ipLttEntry.numOfPaths = 9;
                }

                st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo,
                                                                         override, defragEnable);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                          lpmDBId, vrId, nextHopInfo.ipLttEntry.numOfPaths);

                nextHopInfo.ipLttEntry.numOfPaths = 0;

                /*
                    1.13. Call with lpmDBId [1] out of range
                        nextHopInfoPtr->ipLttEntry.routeEntryBaseIndex[4096] and other parameters from 1.5.
                    Expected: NOT GT_OK.
                */
                nextHopInfo.ipLttEntry.routeEntryBaseIndex = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerNextHop;

                st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo,
                                                                            override, defragEnable);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                                                 "%d, %d, nextHopInfoPtr->ipLttEntry.routeEntryBaseIndex",
                                                 lpmDBId, vrId, nextHopInfo.ipLttEntry.routeEntryBaseIndex);
            }

            nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;

            /*
                1.14. Call with lpmDBId [1]
                             wrong enum values nextHopInfoPtr->ipLttEntry.ipv6MCGroupScopeLevel
                             and other parameters from 1.5.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmIpv6UcPrefixAdd
                                (lpmDBId, vrId, &ipAddr, prefixLen,
                                 &nextHopInfo, override, defragEnable),
                                nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel);

            /*
                1.15. Call with lpmDBId [1]
                                             nextHopInfoPtr[NULL]
                                             and other parameters from 1.5.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, NULL, override, defragEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);
        }

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            /*
                1.16. For Cheetah+ call with lpmDBId [0]
                                             wrong enum values nextHopInfoPtr->pclIpUcAction.pktCmdp
                                             and other parameters from 1.4.
                Expected: GT_BAD_PARAM.
            */
            lpmDBId = 0;
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);

            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmIpv6UcPrefixAdd
                                (lpmDBId, vrId, &ipAddr, prefixLen,
                                 &nextHopInfo, override, defragEnable),
                                nextHopInfo.pclIpUcAction.pktCmd);
        }

        /*
            1.17. Call with not multicast ip address range (255)
            Expected: GT_BAD_PARAM.
        */
        ipAddr.arIP[0] = 255;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragEnable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, lpmDBId, vrId, ipAddr.arIP[0]);

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.18. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0 / 1] and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId[0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Call with lpmDBId[1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6UcPrefixAddBulk
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC  *ipv6PrefixArray,
    IN GT_BOOL                              defragmentationEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6UcPrefixAddBulk)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK for Cheetha devices else GT_BAD_PRT
    1.3. Call with lpmDBId[0], ipv6PrefixArrayLen [1], ipv6PrefixArray{vrId[0],
        ipAddr[10.15.0.1], prefixLen[128], nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        override[GT_TRUE], returnStatus[0]}
    Expected: GT_OK.
    1.4. Call with not supported ipv6PrefixArray->vrId[1] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.5. Call with out of range ipv6PrefixArray->prefixLen [129] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.6. Call with lpmDBId [1], correct nextHopInfoPtr->ipLttEntry.numOfPaths
                     ([5 / 7] - for lion and above and [49 / 63] for other)
                     and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.7. Call with lpmDBId [1], out of range nextHopInfoPtr->ipLttEntry.numOfPaths
                     ([64 / 65] - for lion and above and [8 / 9] for other)
                     and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.9. Call with ipv6PrefixArray [NULL] and other parameters from 1.3.
    Expected: GT_BAD_PTR.
    1.10. Call with not multicast ip address range (255)
    Expected: GT_BAD_PARAM.
    1.11. Call cpssDxChIpLpmIpv6UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                              lpmDBId      = 0;
    GT_U32                              prefixLen    = 0;
    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC ipv6PrefixArray;
    GT_U32                              vrId         = 0;
    GT_BOOL                             defragEnable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&ipv6PrefixArray, sizeof(ipv6PrefixArray));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call with lpmDBId[0], ipv4PrefixArrayLen[1], ipv4PrefixArray{vrId[0],
                       ipAddr[10.15.0.1], prefixLen[128],
                       nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                       CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                       matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos
                       { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                       ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                       profilePrecedence=GT_FALSE] ] }, redirect {
                       CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                       policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                       egressTaggedModify=GT_FALSE,
                       modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                       vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                       ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                       override[GT_TRUE], returnStatus[0]}
            Expected: GT_OK.
        */
        lpmDBId   = 0;
        prefixLen = 1;

        ipv6PrefixArray.ipAddr.u32Ip[0] = 10;
        ipv6PrefixArray.ipAddr.u32Ip[1] = 15;
        ipv6PrefixArray.ipAddr.u32Ip[2] = 0;
        ipv6PrefixArray.ipAddr.u32Ip[3] = 1;

        ipv6PrefixArray.prefixLen = 128;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&ipv6PrefixArray.nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &ipv6PrefixArray.nextHopInfo.ipLttEntry);
        }

        ipv6PrefixArray.override                = GT_TRUE;
        ipv6PrefixArray.returnStatus            = GT_OK;
        ipv6PrefixArray.vrId = 0;

        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen, &ipv6PrefixArray, defragEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, prefixLen);

        /*
            1.4. Call with not supported ipv6PrefixArray->vrId[10] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        ipv6PrefixArray.vrId = 10;

        st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen, &ipv6PrefixArray, defragEnable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipv6PrefixArray->vrId = %d",
                                         lpmDBId, ipv6PrefixArray.vrId);

        ipv6PrefixArray.vrId = 0;

        /*
            1.5. Call with out of range ipv6PrefixArray->prefixLen [129] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        ipv6PrefixArray.prefixLen = 129;

        st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen, &ipv6PrefixArray, defragEnable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipv6PrefixArray->prefixLen = %d",
                                         lpmDBId, ipv6PrefixArray.prefixLen);

        ipv6PrefixArray.prefixLen = 128;


        if ((CPSS_PP_FAMILY_CHEETAH_E != PRV_CPSS_PP_MAC(dev)->devFamily) &&
            (!PRV_CPSS_SIP_5_CHECK_MAC(dev)) &&
            !prvUtfIsPbrModeUsed())
        {
            /*
                1.6. Call with lpmDBId [1], correct nextHopInfoPtr->ipLttEntry.numOfPaths
                     ([5 / 7] - for lion and above and [49 / 63] for other)
                                 and other parameters from 1.5.
                Expected: GT_OK.
            */
            /*call with 49 or 5 values */
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 49;
            }
            else
            {
                ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 5;
            }

            st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen,
                                        &ipv6PrefixArray, defragEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                      lpmDBId, vrId, ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths);

            /*call with 63 or 7 values */
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 63;
            }
            else
            {
                ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 7;
            }

            st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen,
                                        &ipv6PrefixArray, defragEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                      lpmDBId, vrId, ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths);

            /*
                1.7. Call with lpmDBId [1],
                                 out of range nextHopInfoPtr->ipLttEntry.numOfPaths
                                 ([64 / 65] - for lion and above and [8 / 9] for other)
                                 and other parameters from 1.5.
                Expected: NOT GT_OK.
            */
            /*call with 64 or 8 values */
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 64;
            }
            else
            {
                ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 8;
            }

            st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen,
                                        &ipv6PrefixArray, defragEnable);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                      lpmDBId, vrId, ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths);

            /*call with 65 or 9 values */
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 65;
            }
            else
            {
                ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 9;
            }

            st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen,
                                        &ipv6PrefixArray, defragEnable);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                      lpmDBId, vrId, ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths);

            ipv6PrefixArray.nextHopInfo.ipLttEntry.numOfPaths = 0;
        }

        /*
            1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen, &ipv6PrefixArray, defragEnable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.9. Call with ipv6PrefixArray[NULL] and other parameters from 1.3.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen, NULL, defragEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ipv6PrefixArray = NULL", lpmDBId);

        /*
            1.10. Call with not multicast ip address range (255)
            Expected: GT_BAD_PARAM.
        */
        ipv6PrefixArray.ipAddr.arIP[0] = 255;

        st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen, &ipv6PrefixArray, defragEnable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, lpmDBId, vrId, ipv6PrefixArray.ipAddr.arIP[0]);

        ipv6PrefixArray.ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.11. Call cpssDxChIpLpmIpv6UcPrefixesFlush with lpmDBId [0 / 1]
                and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId[0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Call with lpmDBId[1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6UcPrefixDel
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               vrId,
    IN GT_IPV6ADDR                          *ipAddrPtr,
    IN GT_U32                               prefixLen
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6UcPrefixDel)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK.
    1.3. Call cpssDxChIpLpmIpv6UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
        prefixLen[128], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.4. Call with lpmDBId [0], vrId[0], ipAddr[10.15.0.1], prefixLen[128].
    Expected: GT_OK.
    1.5. Call with lpmDBId [0], vrId[0], ipAddr[10.15.0.1] (already deleted), prefixLen[128].
    Expected: NOT GT_OK.
    1.6. Call with lpmDBId [0], vrId[0], ipAddr[10.15.0.2] (wrong IP address), prefixLen[128].
    Expected: NOT GT_OK.
    1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.8. Call with not supported vrId [4096] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.9. Call with out of range prefixLen [129] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.10. Call with not multicast ip address range (255)
    Expected: GT_BAD_PARAM.
    1.11. Call cpssDxChIpLpmIpv6UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId      = 0;
    GT_U32                                 vrId         = 0;
    GT_IPV6ADDR                            ipAddr;
    GT_U32                                 prefixLen    = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override     = GT_FALSE;
    GT_BOOL                                defragEnable = GT_FALSE;

    cpssOsBzero((GT_VOID*) &ipAddr, sizeof(ipAddr));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv6UcPrefixAdd with lpmDBId [0],
                 vrId[0], ipAddr[10.15.0.1], prefixLen[128], nextHopInfoPtr->pclIpUcAction {
                 pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                 CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                 matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                 egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                 ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                 profilePrecedence=GT_FALSE] ] }, redirect {
                 CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
                 policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                 modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                 vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                 ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                 override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        prefixLen = 128;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);
        }

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo,
                                                                override, defragEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpLpmIpv6UcPrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.4. Call with lpmDBId [0], vrId[0], ipAddr[10.15.0.1], prefixLen[128].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        prefixLen = 128;

        st = cpssDxChIpLpmIpv6UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.5. Call with lpmDBId [0], vrId[0], ipAddr[10.15.0.1] (already deleted), prefixLen[128].
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmIpv6UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.6. Call with lpmDBId [0], vrId[0], ipAddr[10.15.0.2] (wrong IP address), prefixLen[128].
            Expected: NOT GT_OK.
        */
        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 2;

        st = cpssDxChIpLpmIpv6UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.8. Call with not supported vrId [10] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv6UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.9. Call with out of range prefixLen [129] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        prefixLen = 129;

        st = cpssDxChIpLpmIpv6UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d", lpmDBId, prefixLen);

        prefixLen = 128;

        /*
            1.10. Call with not multicast ip address range (255)
            Expected: GT_BAD_PARAM.
        */
        ipAddr.arIP[0] = 255;

        st = cpssDxChIpLpmIpv6UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, lpmDBId, vrId, ipAddr.arIP[0]);

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.11. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0 / 1]
                  and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId[0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Call with lpmDBId[1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6UcPrefixDelBulk
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC  *ipv6PrefixArray
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6UcPrefixDelBulk)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. - 1.2.
    1.3. Call cpssDxChIpLpmIpv6UcPrefixAddBulk with lpmDBId[0],
       ipv4PrefixArrayLen[1],
       ipv4PrefixArray{vrId[0],
                       ipAddr[10.15.0.1],
                       prefixLen[128],
                       nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                       mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                       mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                       enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                       egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                       qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                       profilePrecedence=GT_FALSE] ] }, redirect {
                       CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0]
                       }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                       egressTaggedModify=GT_FALSE,
                       modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                       nestedVlan=GT_FALSE, vlanId=100,
                       precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                       ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE,
                       GT_FALSE } },
                       override[GT_TRUE],
                       returnStatus[0]}
    Expected: GT_OK.
    1.4. Call with lpmDBId[0],
                   ipv4PrefixArrayLen[1],
                   ipv4PrefixArray{vrId[0],
                                   ipAddr[10.15.0.1],
                                   prefixLen[128],
                                   nextHopInfoPtr{pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                                   mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                   mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                                   enableMatchCount = GT_FALSE, matchCounterIndex = 0
                                   }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                                   modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                                   profileAssignIndex=GT_FALSE,
                                   profilePrecedence=GT_FALSE] ] }, redirect {
                                   CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                                   data[routerLttIndex=0] }, policer {
                                   policerEnable=GT_FALSE, policerId=0 }, vlan {
                                   egressTaggedModify=GT_FALSE,
                                   modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                   nestedVlan=GT_FALSE, vlanId=100,
                                   precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
                                   }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE,
                                   GT_FALSE, GT_FALSE } },
                   override[GT_TRUE],
                   returnStatus[0]}
    Expected: GT_OK.
    1.5. Call with lpmDBId[0],
                                ipv4PrefixArrayLen[1],
                                ipv4PrefixArray{vrId[0], ipAddr[10.15.0.1],
                                prefixLen[128], nextHopInfoPtr{ pktCmd =
                                CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                                CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                                enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
                                qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                                modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                                profileAssignIndex=GT_FALSE,
                                profilePrecedence=GT_FALSE] ] }, redirect {
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                                data[routerLttIndex=0] }, policer {
                                policerEnable=GT_FALSE, policerId=0 }, vlan {
                                egressTaggedModify=GT_FALSE,
                                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                nestedVlan=GT_FALSE, vlanId=100,
                                precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
                                }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE,
                                GT_FALSE, GT_FALSE } },
                                override[GT_TRUE],
                                returnStatus[0]} (already deleted)
    Expected: NOT GT_OK.
    1.6. Call with not supported ipv4PrefixArray->vrId[10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.7. Call with out of range ipv4PrefixArray->prefixLen [129] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.9. Call with ipv4PrefixArray[NULL] and other parameters from 1.4.
    Expected: GT_BAD_PTR.
    1.10. Call with correct nextHopInfoPtr->ipLttEntry.numOfPaths
                     ([5 / 7] - for lion and above and [49 / 63] for other)
                     and other parameters from 1.3.
    Expected: GT_OK.
    1.11. Call with lpmDBId [1],
                     out of range nextHopInfoPtr->ipLttEntry.numOfPaths
                     ([64 / 65] - for lion and above and [8 / 9] for other)
                     and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.12. Call with not multicast ip address range (255)
    Expected: GT_BAD_PARAM.
    1.13. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0 / 1]
          and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                              lpmDBId      = 0;
    GT_U32                              arrayLen     = 0;
    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC ipv6Prefix;
    GT_U32                              vrId         = 0;
    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC ipv6PrefixArray;
    GT_U32                              prefixLen    = 0;
    GT_BOOL                             defragEnable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&ipv6Prefix, sizeof(ipv6Prefix));
    cpssOsBzero((GT_VOID*)&ipv6PrefixArray, sizeof(ipv6PrefixArray));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv6UcPrefixAddBulk with lpmDBId[0],
               ipv4PrefixArrayLen[1],
               ipv4PrefixArray{vrId[0],
                               ipAddr[10.15.0.1],
                               prefixLen[128],
                               nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                               mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                               mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                               enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                               egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                               qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                               profilePrecedence=GT_FALSE] ] }, redirect {
                               CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0]
                               }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                               egressTaggedModify=GT_FALSE,
                               modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                               nestedVlan=GT_FALSE, vlanId=100,
                               precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                               ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE,
                               GT_FALSE } },
                               override[GT_TRUE],
                               returnStatus[0]}
            Expected: GT_OK.
        */
        lpmDBId   = 0;
        prefixLen = 1;

        ipv6PrefixArray.ipAddr.u32Ip[0] = 10;
        ipv6PrefixArray.ipAddr.u32Ip[1] = 15;
        ipv6PrefixArray.ipAddr.u32Ip[2] = 0;
        ipv6PrefixArray.ipAddr.u32Ip[3] = 1;

        ipv6PrefixArray.prefixLen = 128;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&ipv6PrefixArray.nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &ipv6PrefixArray.nextHopInfo.ipLttEntry);
        }

        ipv6PrefixArray.override                = GT_TRUE;
        ipv6PrefixArray.returnStatus            = GT_OK;
        ipv6PrefixArray.vrId = 0;

        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen, &ipv6PrefixArray, defragEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, prefixLen);

        /*
            1.4. Call with lpmDBId[0],
                           ipv4PrefixArrayLen[1],
                           ipv4PrefixArray{vrId[0],
                                           ipAddr[10.15.0.1],
                                           prefixLen[128],
                                           nextHopInfoPtr{pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                                           mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                           mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                                           enableMatchCount = GT_FALSE, matchCounterIndex = 0
                                           }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                                           modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                                           profileAssignIndex=GT_FALSE,
                                           profilePrecedence=GT_FALSE] ] }, redirect {
                                           CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                                           data[routerLttIndex=0] }, policer {
                                           policerEnable=GT_FALSE, policerId=0 }, vlan {
                                           egressTaggedModify=GT_FALSE,
                                           modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                           nestedVlan=GT_FALSE, vlanId=100,
                                           precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
                                           }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE,
                                           GT_FALSE, GT_FALSE } },
                           override[GT_TRUE],
                           returnStatus[0]}
            Expected: GT_OK.
        */
        lpmDBId  = 0;
        arrayLen = 1;

        ipv6Prefix.vrId = 0;

        ipv6Prefix.ipAddr.u32Ip[0] = 10;
        ipv6Prefix.ipAddr.u32Ip[1] = 15;
        ipv6Prefix.ipAddr.u32Ip[2] = 0;
        ipv6Prefix.ipAddr.u32Ip[3] = 1;

        ipv6Prefix.prefixLen = 128;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&ipv6PrefixArray.nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &ipv6PrefixArray.nextHopInfo.ipLttEntry);
        }

        ipv6Prefix.override                = GT_TRUE;
        ipv6Prefix.returnStatus            = GT_OK;

        st = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, arrayLen, &ipv6Prefix);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, arrayLen);

        /*
            1.5. Call with lpmDBId[0],
                                        ipv4PrefixArrayLen[1],
                                        ipv4PrefixArray{vrId[0], ipAddr[10.15.0.1],
                                        prefixLen[128], nextHopInfoPtr{ pktCmd =
                                        CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                                        CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                                        enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
                                        qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                                        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                                        profileAssignIndex=GT_FALSE,
                                        profilePrecedence=GT_FALSE] ] }, redirect {
                                        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                                        data[routerLttIndex=0] }, policer {
                                        policerEnable=GT_FALSE, policerId=0 }, vlan {
                                        egressTaggedModify=GT_FALSE,
                                        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                        nestedVlan=GT_FALSE, vlanId=100,
                                        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
                                        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE,
                                        GT_FALSE, GT_FALSE } },
                                        override[GT_TRUE],
                                        returnStatus[0]} (already deleted)
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, arrayLen, &ipv6Prefix);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, arrayLen);

        /*
            1.6. Call with not supported ipv4PrefixArray->vrId[10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        ipv6Prefix.vrId = 10;

        st = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, arrayLen, &ipv6Prefix);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipv6PrefixArray->vrId = %d",
                                                                lpmDBId, ipv6Prefix.vrId);

        ipv6Prefix.vrId = 0;

        /*
            1.7. Call with out of range ipv4PrefixArray->prefixLen [129] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        ipv6Prefix.prefixLen = 129;

        st = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, arrayLen, &ipv6Prefix);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipv6PrefixArray->prefixLen = %d",
                                                            lpmDBId, ipv6Prefix.prefixLen);

        ipv6Prefix.prefixLen = 128;

        /*
            1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */

        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, arrayLen, &ipv6Prefix);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.9. Call with ipv4PrefixArray[NULL] and other parameters from 1.4.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, arrayLen, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, ipv6PrefixArray = NULL", lpmDBId);

        /*
            1.10. Call with correct nextHopInfoPtr->ipLttEntry.numOfPaths
                             ([5 / 7] - for lion and above and [49 / 63] for other)
                             and other parameters from 1.3.
            Expected: GT_OK.
        */
        /*call with 49 or 5 values */
        if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths = 49;
        }
        else
        {
            ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths = 5;
        }
        /* add prefixes first */
        st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen, &ipv6PrefixArray, defragEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, prefixLen);

        st = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, arrayLen, &ipv6Prefix);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                      lpmDBId, vrId, ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths);

        /*call with 63 or 7 values */
        if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths = 63;
        }
        else
        {
            ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths = 7;
        }

        /* add prefixes first */
        st = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, prefixLen, &ipv6PrefixArray, defragEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, prefixLen);

        st = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, arrayLen, &ipv6Prefix);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                  lpmDBId, vrId, ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths);

        /*
            1.11. Call with lpmDBId [1],
                             out of range nextHopInfoPtr->ipLttEntry.numOfPaths
                             ([64 / 65] - for lion and above and [8 / 9] for other)
                             and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        /*call with 64 or 8 values */
        if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths = 64;
        }
        else
        {
            ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths = 8;
        }

        st = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, arrayLen, &ipv6Prefix);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                  lpmDBId, vrId, ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths);

        /*call with 65 or 9 values */
        if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths = 65;
        }
        else
        {
            ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths = 9;
        }

        st = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, arrayLen, &ipv6Prefix);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, %d, nextHopInfoPtr->ipLttEntry.numOfPaths %d",
                  lpmDBId, vrId, ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths);

        ipv6Prefix.nextHopInfo.ipLttEntry.numOfPaths = 0;

        /*
            1.12. Call with not multicast ip address range (255)
            Expected: GT_BAD_PARAM.
        */
        ipv6Prefix.ipAddr.arIP[0] = 255;

        st = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, arrayLen, &ipv6Prefix);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, lpmDBId, vrId, ipv6Prefix.ipAddr.arIP[0]);

        ipv6Prefix.ipAddr.arIP[0] = 10; /* restore ip */


        /*
            1.13. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0 / 1]
                  and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId[0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Call with lpmDBId[1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6UcPrefixesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6UcPrefixesFlush)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK.
    1.3. Call with lpmDBId [0], vrId[0].
    Expected: GT_OK.
    1.4. Call with not valid vrId [4096] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.5. Call with not supported lpmDBId [10] and other parameters from 1.3.
    Expected: NOT GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                   lpmDBId = 0;
    GT_U32                   vrId    = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call with lpmDBId [0], vrId[0].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.4. Call with not valid vrId[4096] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.5. Call with not supported lpmDBId [10] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */

        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6UcPrefixSearch
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_IPV6ADDR                             ipAddr,
    IN  GT_U32                                  prefixLen,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT GT_U32                                  *tcamRowIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6UcPrefixSearch)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK.
    1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
        prefixLen[128], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.4. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[128] and non-null
        nextHopInfoPtr
    Expected: GT_OK and the same nextHopInfoPtr.
    1.5. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[124] and non-null
        nextHopInfoPtr
    Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.
    1.6. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[124] and non-null
        nextHopInfoPtr
    Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.
    1.7. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[128] and non-null
        nextHopInfoPtr
    Expected: NOT GT_OK.
    1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.9. Call with not supported vrId [4096] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.10. Call with out of range prefixLen [129] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.11. Call with ipAddrPtr[NULL] and other parameters from 1.4.
    Expected: GT_BAD_PTR.
    1.12. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[128] and nextHopInfoPtr[NULL].
    Expected: GT_BAD_PTR.
    1.13. Call with not multicast ip address range (255)
    Expected: GT_BAD_PARAM.
    1.14. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId      = 0;
    GT_U32                                 vrId         = 0;
    GT_IPV6ADDR                            ipAddr;
    GT_U32                                 prefixLen    = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfoGet;
    GT_BOOL                                override     = GT_FALSE;
    GT_BOOL                                defragEnable = GT_FALSE;
    GT_U32                                 tcamRowIndex = 0;
    GT_U32                                 tcamColumIndex = 0;

    cpssOsBzero((GT_VOID*) &ipAddr, sizeof(ipAddr));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&nextHopInfoGet, sizeof(nextHopInfoGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv6UcPrefixAdd with lpmDBId [0],
                 vrId[0], ipAddr[10.15.0.1], prefixLen[128], nextHopInfoPtr->pclIpUcAction {
                 pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                 CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                 matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                 egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                 ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                 profilePrecedence=GT_FALSE] ] }, redirect {
                 CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
                 policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                 modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                 vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                 ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                 override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        prefixLen = 128;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);
        }

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen,
                                          &nextHopInfo, override, defragEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                                "cpssDxChIpLpmIpv6UcPrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.4. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1],
                 prefixLen[128] and non-null nextHopInfoPtr
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        prefixLen = 128;

        st = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                              &nextHopInfoGet, &tcamRowIndex,
                                              &tcamColumIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLen);

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfComparePclIpUcActionEntries(lpmDBId, vrId, &nextHopInfoGet.pclIpUcAction,
                                                                &nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfCompareIpLttEntries(lpmDBId, vrId, &nextHopInfoGet.ipLttEntry, &nextHopInfo.ipLttEntry);
        }

        /*
            1.5. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[124] and non-null nextHopInfoPtr
            Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.

        */
        prefixLen = 124;

        st = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                             &nextHopInfoGet, &tcamRowIndex,
                                             &tcamColumIndex);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLen);

        /*
            1.6. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[124] and non-null nextHopInfoPtr
            Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.

        */
        prefixLen = 124;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 255;

        st = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                             &nextHopInfoGet, &tcamRowIndex,
                                             &tcamColumIndex);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLen);

        /*
            1.7. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[128] and non-null nextHopInfoPtr
            Expected: NOT GT_OK.
        */
        prefixLen = 128;

        st = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                             &nextHopInfoGet, &tcamRowIndex,
                                             &tcamColumIndex);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLen);

        /*
            1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                              &nextHopInfoGet, &tcamRowIndex,
                                              &tcamColumIndex);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.9. Call with not supported vrId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                              &nextHopInfoGet, &tcamRowIndex,
                                              &tcamColumIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.10. Call with out of range prefixLen [129] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        prefixLen = 129;

        st = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                              &nextHopInfoGet, &tcamRowIndex,
                                              &tcamColumIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d", lpmDBId, prefixLen);

        prefixLen = 128;

        /*
            1.11. Call with ipAddrPtr[NULL] and other parameters from 1.4.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId, NULL, prefixLen,
                                              &nextHopInfoGet, &tcamRowIndex,
                                              &tcamColumIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipAddrPtr = NULL", lpmDBId, vrId);

        /*
            1.12. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[32] and nextHopInfoPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        prefixLen = 32;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        st = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                              NULL, &tcamRowIndex,&tcamColumIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);

        /*
            1.13. Call with not multicast ip address range (255)
            Expected: GT_BAD_PARAM.
        */
        ipAddr.arIP[0] = 255;

        st = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId, &ipAddr, prefixLen,
                                              &nextHopInfoGet, &tcamRowIndex, &tcamColumIndex);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, lpmDBId, vrId, ipAddr.arIP[0]);

        ipAddr.arIP[0] = 10; /* restore ip */


        /*
            1.14. Call cpssDxChIpLpmIpv6UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId    = 0;
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6UcPrefixGet
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_IPV6ADDR                             *ipAddrPtr,
    OUT GT_U32                                  *prefixLenPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT GT_U32                                  *tcamRowIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6UcPrefixGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK.
    1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
        prefixLen[128], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.4. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[128] and non-null
        nextHopInfoPtr
    Expected: GT_OK and the same nextHopInfoPtr.
    1.5. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[124] and non-null
        nextHopInfoPtr
    Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.
    1.6. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[124] and non-null
        nextHopInfoPtr
    Expected: GT_OK for Cheetah+ and NOT GT_OK for others and the same nextHopInfoPtr.
    1.7. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[128] and non-null
        nextHopInfoPtr
    Expected: NOT GT_OK.
    1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.9. Call with not supported vrId [4096] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.10. Call with out of range prefixLen [129] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.11. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[128] and nextHopInfoPtr[NULL].
    Expected: GT_BAD_PTR.
    1.12. Call with not multicast ip address range (255)
    Expected: GT_BAD_PARAM.
    1.13. Call with invalid ipAddrPtr[null] and other parameters from 1.4.
    Expected: GT_BAD_PTR.
    1.14. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId      = 0;
    GT_U32                                 vrId         = 0;
    GT_IPV6ADDR                            ipAddr;
    GT_U32                                 prefixLen    = 0;
    GT_U32                                 prefixLenGet;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfoGet;
    GT_BOOL                                override     = GT_FALSE;
    GT_BOOL                                defragEnable = GT_FALSE;
    GT_U32                                 tcamRowIndex = 0;
    GT_U32                                 tcamColumIndex = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                     UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
                                     UTF_XCAT2_E | UTF_LION2_E);

    cpssOsBzero((GT_VOID*)&ipAddr, sizeof(ipAddr));
    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&nextHopInfoGet, sizeof(nextHopInfoGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv6UcPrefixAdd with lpmDBId [0],
                 vrId[0], ipAddr[10.15.0.1], prefixLen[128], nextHopInfoPtr->pclIpUcAction {
                 pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                 CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                 matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                 egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                 ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                 profilePrecedence=GT_FALSE] ] }, redirect {
                 CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
                 policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                 modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                 vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                 ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                 override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        prefixLen = 128;

        prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen,
                                          &nextHopInfo, override, defragEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                                "cpssDxChIpLpmIpv6UcPrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.4. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1],
                 prefixLen[128] and non-null nextHopInfoPtr
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        prefixLen = 128;

        st = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          &nextHopInfoGet, &tcamRowIndex,
                                          &tcamColumIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLenGet);

        prvUtfCompareIpLttEntries(lpmDBId, vrId, &nextHopInfoGet.ipLttEntry, &nextHopInfo.ipLttEntry);

        UTF_VERIFY_EQUAL1_STRING_MAC(prefixLen, prefixLenGet,
                "got another prefixLen then was set: %d", prefixLenGet);

        /*
            1.5. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[124] and non-null nextHopInfoPtr
            Expected: GT_OK and the same nextHopInfoPtr different prefixLen.

        */
        prefixLen = 124;

        st = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          &nextHopInfoGet, &tcamRowIndex,
                                          &tcamColumIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLenGet);

        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(prefixLen, prefixLenGet,
                "got same prefixLen that was set: %d", prefixLenGet);

        /*
            1.6. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[124] and non-null nextHopInfoPtr
            Expected: GT_OK and the same nextHopInfoPtr different prefixLen.

        */
        prefixLen = 124;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 255;

        st = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          &nextHopInfoGet, &tcamRowIndex,
                                          &tcamColumIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLenGet);

        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(prefixLen, prefixLenGet,
                "got same prefixLen that was set: %d", prefixLenGet);

        /*
            1.7. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.255], prefixLen[128] and non-null nextHopInfoPtr
            Expected: GT_OK with different prefixLen.
        */
        prefixLen = 128;

        st = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          &nextHopInfoGet, &tcamRowIndex,
                                          &tcamColumIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLenGet);

        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(prefixLen, prefixLenGet,
                "got same prefixLen that was set: %d", prefixLenGet);

        /*
            1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          &nextHopInfoGet, &tcamRowIndex,
                                          &tcamColumIndex);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.9. Call with not supported vrId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          &nextHopInfoGet, &tcamRowIndex,
                                          &tcamColumIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.11. Call with lpmDBId[0], vrId[0], ipAddr[10.15.0.1], prefixLen[32] and nextHopInfoPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        prefixLen = 32;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        st = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          NULL, &tcamRowIndex,&tcamColumIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);

        /*
            1.12. Call with not multicast ip address range (255)
            Expected: GT_BAD_PARAM.
        */
        ipAddr.arIP[0] = 255;

        st = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrId, &ipAddr, &prefixLenGet,
                                          &nextHopInfoGet, &tcamRowIndex, &tcamColumIndex);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, lpmDBId, vrId, ipAddr.arIP[0]);

        ipAddr.arIP[0] = 10; /* restore ip */


        /*
            1.13. Call with invalid ipAddrPtr[null] and other parameters from 1.4.
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrId, NULL, &prefixLenGet,
                                          &nextHopInfoGet, &tcamRowIndex,&tcamColumIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipAddrPtr = NULL", lpmDBId, vrId);


        /*
            1.14. Call cpssDxChIpLpmIpv6UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId    = 0;
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6UcPrefixGetNext
(
    IN    GT_U32                                    lpmDBId,
    IN    GT_U32                                    vrId,
    INOUT GT_IPV6ADDR                               *ipAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *nextHopInfoPtr,
    OUT GT_U32                                      *tcamRowIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6UcPrefixGetNext)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK.
    1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
        prefixLen[128], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.4. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.2],
        prefixLen[128], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.5. Call with lpmDBId[0], vrId[0], ipAddrPtr [10.15.0.1], prefixLenPtr [128] and non-null nextHopInfoPtr
    Expected: GT_OK and the same prefixLenPtr  and nextHopInfoPtr.
    1.6. Call with lpmDBId[0], vrId[0], ipAddrPtr [10.15.0.2], prefixLenPtr [128] and non-null nextHopInfoPtr
    Expected: NOT GT_OK (the last prefix reached).
    1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.8. Call with not supported vrId [4096] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.9. Call with out of range prefixLenPtr [129] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.10. Call with lpmDBId[0], vrId[0], ipAddrPtr [NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
    1.11. Call with lpmDBId[0], vrId[0], prefixLenPtr [NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
    1.12. Call with lpmDBId[0], vrId[0], nextHopInfoPtr [NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
    1.13. Call with not multicast ip address range (255)
    Expected: GT_BAD_PARAM.
    1.14. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId      = 0;
    GT_U32                                 vrId         = 0;
    GT_IPV6ADDR                            ipAddr;
    GT_IPV6ADDR                            ipAddrTmp;
    GT_U32                                 prefixLen    = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfoGet;
    GT_BOOL                                override     = GT_FALSE;
    GT_BOOL                                defragEnable = GT_FALSE;
    GT_U32                                 tcamRowIndex = 0;
    GT_U32                                 tcamColumnIndex = 0;


    cpssOsBzero((GT_VOID*)&ipAddr, sizeof(ipAddr));
    cpssOsBzero((GT_VOID*)&ipAddrTmp, sizeof(ipAddrTmp));
    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&nextHopInfoGet, sizeof(nextHopInfoGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0],
                 vrId[0], ipAddr[10.15.0.1], prefixLen[128], nextHopInfoPtr->pclIpUcAction {
                 pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                 CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                 matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                 egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                 ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                 profilePrecedence=GT_FALSE] ] }, redirect {
                 CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
                 policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                 modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                 vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                 ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                 override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        prefixLen = 128;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);
        }

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv6UcPrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.4. Call cpssDxChIpLpmIpv6UcPrefixAdd with lpmDBId [0],
                 vrId[0], ipAddr[10.15.0.2], prefixLen[128], nextHopInfoPtr->pclIpUcAction {
                 pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                 CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                 matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                 egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                 ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                 profilePrecedence=GT_FALSE] ] }, redirect {
                 CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
                 policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                 modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                 vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                 ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                 override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 2;

        prefixLen = 128;

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);
        }

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv6UcPrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.5. Call with lpmDBId[0], vrId[0], ipAddrPtr [10.15.0.1],
                    prefixLenPtr [128] and non-null nextHopInfoPtr
            Expected: GT_OK and the same prefixLenPtr  and nextHopInfoPtr.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        ipAddrTmp.u32Ip[0] = 10;
        ipAddrTmp.u32Ip[1] = 15;
        ipAddrTmp.u32Ip[2] = 0;
        ipAddrTmp.u32Ip[3] = 1;

        prefixLen = 128;

        st = cpssDxChIpLpmIpv6UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                               &nextHopInfoGet, &tcamRowIndex,
                                               &tcamColumnIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        if (CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily || prvUtfIsPbrModeUsed())
        {
            prvUtfComparePclIpUcActionEntries(lpmDBId, vrId, &nextHopInfoGet.pclIpUcAction,
                                                                &nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfCompareIpLttEntries(lpmDBId, vrId, &nextHopInfoGet.ipLttEntry,
                                                                &nextHopInfo.ipLttEntry);
        }

        /* Verifying ipAddr */
        UTF_VERIFY_EQUAL2_STRING_MAC(ipAddr.arIP[0], ipAddrTmp.arIP[0],
                   "get another ipAddrPtr[0] than was set: %d, %d", lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipAddr.arIP[1], ipAddrTmp.arIP[1],
                   "get another ipAddrPtr[1] than was set: %d, %d", lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipAddr.arIP[2], ipAddrTmp.arIP[2],
                   "get another ipAddrPtr[2] than was set: %d, %d", lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipAddr.arIP[3], ipAddrTmp.arIP[3],
                   "get another ipAddrPtr[3] than was set: %d, %d", lpmDBId, vrId);

        /*
            1.6. Call with lpmDBId[0], vrId[0], ipAddrPtr [10.15.0.2],
                 prefixLenPtr [128] and non-null nextHopInfoPtr
            Expected: NOT GT_OK (the last prefix reached).
        */
        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 2;

        st = cpssDxChIpLpmIpv6UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                              &nextHopInfoGet, &tcamRowIndex,&tcamColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                               &nextHopInfoGet, &tcamRowIndex,
                                               &tcamColumnIndex);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.8. Call with not supported vrId [4096] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv6UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                               &nextHopInfoGet, &tcamRowIndex,
                                               &tcamColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.9. Call with out of range prefixLenPtr [129] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        prefixLen = 129;

        st = cpssDxChIpLpmIpv6UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                               &nextHopInfoGet, &tcamRowIndex,
                                               &tcamColumnIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLenPtr = %d", lpmDBId, prefixLen);

        prefixLen = 128;

        /*
            1.10. Call with lpmDBId[0], vrId[0], ipAddrPtr [NULL] and other parameters from 1.5.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6UcPrefixGetNext(lpmDBId, vrId, NULL, &prefixLen,
                                              &nextHopInfoGet, &tcamRowIndex,
                                              &tcamColumnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipAddrPtr = NULL", lpmDBId, vrId);

        /*
            1.11. Call with lpmDBId[0], vrId[0], prefixLenPtr [NULL] and other parameters from 1.5.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6UcPrefixGetNext(lpmDBId, vrId, &ipAddr, NULL,
                                              &nextHopInfoGet, &tcamRowIndex,
                                              &tcamColumnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, prefixLenPtr = NULL", lpmDBId, vrId);

        /*
            1.12. Call with lpmDBId[0], vrId[0], nextHopInfoPtr [NULL] and other parameters from 1.5.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                              NULL, &tcamRowIndex,
                                              &tcamColumnIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);

        /*
            1.13. Call with not multicast ip address range (255)
            Expected: GT_BAD_PARAM.
        */
        ipAddr.arIP[0] = 255;

        st = cpssDxChIpLpmIpv6UcPrefixGetNext(lpmDBId, vrId, &ipAddr, &prefixLen,
                                               &nextHopInfoGet, &tcamRowIndex, &tcamColumnIndex);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, lpmDBId, vrId, ipAddr.arIP[0]);

        ipAddr.arIP[0] = 10; /* restore ip */

        /*
            1.14. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId    = 0;
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6McEntryAdd
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPV6ADDR                  ipGroupPtr,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPV6ADDR                  ipSrcPtr,
    IN GT_U32                       ipSrcPrefixLen,
    IN CPSS_DXCH_IP_LTT_ENTRY_STC   *mcRouteLttEntryPtr,
    IN GT_BOOL                      override,
    IN GT_BOOL                      defragmentationEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6McEntryAdd)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. - 1.3.
    1.4. Call with lpmDBId[0 / 1], vrId[0], ipGroup[255.15.0.1], ipGroupPrefixLen[32],
        ipSrc[255.16.0.1], ipSrcPrefixLen[32],
        mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
        routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
        defragmentationEnable[GT_FALSE].
    Expected: GT_OK (no any constraints for any devices).
    1.5. Call with lpmDBId [1], vrId[0],
         wrong enum values mcRouteLttEntryPtr->routeType and other parameters from 1.4.
    Expected: GT_BAD_PARAM.
    1.6. Call with lpmDBId [1], vrId[0],
                                out of range mcRouteLttEntryPtr->numOfPaths [8 / 63]
                                and othe parameters from 1.4.
    Expected: GT_OK for Lion and above and not GT_OK for other.
    1.7. Call with lpmDBId [1], vrId[0],
                                out of range mcRouteLttEntryPtr->numOfPaths [64]
                                and othe parameters from 1.4.
    Expected: NOT GT_OK
    1.8. Call with lpmDBId [1], vrId[0],
                    out of range mcRouteLttEntryPtr->routeEntryBaseIndex [4096]
                    and othe parameters from 1.4.
    Expected: NOT GT_OK.
    1.9. Call with lpmDBId [1], vrId[0],
                    wrong enum values mcRouteLttEntryPtr->ipv6MCGroupScopeLevel
                    and othe parameters from 1.4.
    Expected: GT_BAD_PARAM.
    1.10. Call with lpmDBId [1], out of range vrId[4096] and other valid parameters.
    Expected: NOT GT_OK.
    1.11. Call with out of range ipGroupPrefixLen [129] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.12. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.13. Call with lpmDBId [1], vrId[0], mcRouteLttEntryPtr [NULL]
    and other valid parameters.
    Expected: GT_BAD_PTR.
    1.14. Call cpssDxChIpLpmIpv6McEntriesFlush with with lpmDBId [0 / 1],
          vrId[0] to invalidate entries.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;



    GT_U32                     lpmDBId          = 0;
    GT_U32                     vrId             = 0;
    GT_IPV6ADDR                ipGroup;
    GT_U32                     ipGroupPrefixLen = 0;
    GT_IPV6ADDR                ipSrc;
    GT_U32                     ipSrcPrefixLen   = 0;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntry;
    GT_BOOL                    override         = GT_FALSE;
    GT_BOOL                    defragEnable     = GT_FALSE;
    GT_U8                      devList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                     numOfDevs = 1;

    cpssOsBzero((GT_VOID*) &ipGroup, sizeof(ipGroup));
    cpssOsBzero((GT_VOID*) &ipSrc, sizeof(ipSrc));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        devList[0] = dev;
        /*
            1.1. - 1.3.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.4. Call with lpmDBId[0 / 1], vrId[0], ipGroup[255.15.0.1], ipGroupPrefixLen[32],
                ipSrc[255.16.0.1], ipSrcPrefixLen[32],
                mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
                routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
                sipSaCheckMismatchEnable[GT_TRUE],
                ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
                defragmentationEnable[GT_FALSE].
            Expected: GT_OK (no any constraints for any devices).
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 255;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 128;

        ipSrc.arIP[0] = 255;
        ipSrc.arIP[1] = 15;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 0;

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
        }

        /*
            1.5. Call with lpmDBId [1], vrId[0],
                 wrong enum values mcRouteLttEntryPtr->routeType and other parameters from 1.4.
            Expected: GT_BAD_PARAM.
        */
        lpmDBId = 1;
        vrId    = 0;

        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmIpv6McEntryAdd
                                (lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                 ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable),
                                mcRouteLttEntry.routeType);
        }

        /*
            1.6. Call with lpmDBId [1], vrId[0],
                                        out of range mcRouteLttEntryPtr->numOfPaths [8 / 63]
                                        and othe parameters from 1.4.
            Expected: GT_OK for Lion and above and not GT_OK for other.
        */
        /* call with mcRouteLttEntry.numOfPaths = 8 */
        mcRouteLttEntry.numOfPaths = 8;

        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                     ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);

        if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcRouteLttEntry.numOfPaths = %d",
                                           lpmDBId, vrId, mcRouteLttEntry.numOfPaths);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcRouteLttEntry.numOfPaths = %d",
                                           lpmDBId, vrId, mcRouteLttEntry.numOfPaths);
        }

        /* call with mcRouteLttEntry.numOfPaths = 63 */
        mcRouteLttEntry.numOfPaths = 63;

        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                     ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);

        if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcRouteLttEntry.numOfPaths = %d",
                                           lpmDBId, vrId, mcRouteLttEntry.numOfPaths);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcRouteLttEntry.numOfPaths = %d",
                                           lpmDBId, vrId, mcRouteLttEntry.numOfPaths);
        }

        mcRouteLttEntry.numOfPaths = 0;

        /* numOfPaths is not applicable to Lion 3 or Bobcat 2 */
        if ((!PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
        /*
                1.7. Call with lpmDBId [1], vrId[0],
                                            out of range mcRouteLttEntryPtr->numOfPaths [64]
                                            and othe parameters from 1.4.
                Expected: NOT GT_OK
            */
            mcRouteLttEntry.numOfPaths = 64;

            st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);

            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcRouteLttEntry.numOfPaths = %d",
                                           lpmDBId, vrId, mcRouteLttEntry.numOfPaths);
        }

        mcRouteLttEntry.numOfPaths = 0;

        /*
            1.8. Call with lpmDBId [1], vrId[0],
                            out of range mcRouteLttEntryPtr->routeEntryBaseIndex [4096]
                            and othe parameters from 1.4.
            Expected: NOT GT_OK.
        */
        mcRouteLttEntry.routeEntryBaseIndex = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerNextHop;

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_ALREADY_EXIST, st, "%d, numOfDevs = %d", lpmDBId, numOfDevs);

        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                     ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcRouteLttEntryPtr->routeEntryBaseIndex = %d",
                                         lpmDBId, vrId, mcRouteLttEntry.routeEntryBaseIndex);

        mcRouteLttEntry.routeEntryBaseIndex = 0;

        /*
            1.9. Call with lpmDBId [1], vrId[0],
                            wrong enum values mcRouteLttEntryPtr->ipv6MCGroupScopeLevel
                            and othe parameters from 1.4.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmIpv6McEntryAdd
                                (lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                 ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable),
                                mcRouteLttEntry.ipv6MCGroupScopeLevel);
        }

        /*
            1.10. Call with lpmDBId [1], out of range vrId[4096] and other valid parameters.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                     ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.11. Call with out of range ipGroupPrefixLen [129] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        ipGroupPrefixLen = 129;

        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                            ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, ipGroupPrefixLen = %d", lpmDBId, ipGroupPrefixLen);

        ipGroupPrefixLen = 128;

        /*
            1.12. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                            ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.13. Call with lpmDBId [1], vrId[0], mcRouteLttEntryPtr [NULL]
            and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, NULL, override, defragEnable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                    "%d, %d, mcRouteLttEntryPtr = NULL", lpmDBId, vrId);

        /*
            1.14. Call cpssDxChIpLpmIpv6McEntriesFlush with with lpmDBId [0 / 1],
                  vrId[0] to invalidate entries.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId [0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /* Call with lpmDBId [1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6McEntryDel
(
    IN GT_U32       lpmDBId,
    IN GT_U32       vrId,
    IN GT_IPV6ADDR  *ipGroupPtr,
    IN GT_U32       ipGroupPrefixLen,
    IN GT_IPV6ADDR  *ipSrcPtr,
    IN GT_U32       ipSrcPrefixLen
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6McEntryDel)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [1], vrId[0],
        defUcNextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
        numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, defMcRouteLttEntryPtr {
        routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0], routeEntryBaseIndex[0],
        ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK .
    1.3. Call cpssDxChIpLpmIpv4McEntryAdd with lpmDBId[0], vrId[0], ipGroup[255.15.0.1],
        ipGroupPrefixLen[32], ipSrc[255.16.0.1], ipSrcPrefixLen[32],
        mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
        routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
        defragmentationEnable[GT_FALSE].
    Expected: GT_OK.
    1.4. Call with lpmDBId[0], vrId[0], ipGroup[255.15.0.1], ipGroupPrefixLen[32],
        ipSrc[255.16.0.1], ipSrcPrefixLen[32],.
    Expected: GT_OK.
    1.5. Call with lpmDBId[0], vrId[0], ipGroup[255.15.0.1], ipGroupPrefixLen[32],
        ipSrc[255.16.0.1], ipSrcPrefixLen[32] (already deleted).
    Expected: NOT GT_OK.
    1.6. Call with lpmDBId [0 or 1], out of range vrId[4096] and other valid parameters.
    Expected: NOT GT_OK.
    1.7. Call with out of range ipGroupPrefixLen [129] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.8. Call with out of range ipSrcPrefixLen [129] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.9. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.10. Call cpssDxChIpLpmIpv6McEntriesFlush with with lpmDBId [1], vrId[0] to invalidate entries.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                     lpmDBId          = 0;
    GT_U32                     vrId             = 0;
    GT_IPV6ADDR                ipGroup;
    GT_U32                     ipGroupPrefixLen = 0;
    GT_IPV6ADDR                ipSrc;
    GT_U32                     ipSrcPrefixLen   = 0;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntry;
    GT_BOOL                    override         = GT_FALSE;
    GT_BOOL                    defragEnable     = GT_FALSE;

    cpssOsBzero((GT_VOID*) &ipGroup, sizeof(ipGroup));
    cpssOsBzero((GT_VOID*) &ipSrc, sizeof(ipSrc));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv6McEntryAdd with lpmDBId[1],
                 vrId[0], ipGroup[255.15.0.1], ipGroupPrefixLen[32], ipSrc[255.16.0.1],
                 ipSrcPrefixLen[32],
                 mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                 numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
                 sipSaCheckMismatchEnable[GT_TRUE],
                 ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                 override[GT_TRUE], defragmentationEnable[GT_FALSE].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 255;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 128;

        ipSrc.arIP[0] = 255;
        ipSrc.arIP[1] = 15;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv6McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
        }
        else
        {
               UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv6McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
        }

        /*
            1.4. Call with lpmDBId[0], vrId[0], ipGroup[255.15.0.1], ipGroupPrefixLen[32],
                    ipSrc[255.16.0.1], ipSrcPrefixLen[32].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 255;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 128;

        ipSrc.arIP[0] = 255;
        ipSrc.arIP[1] = 15;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        st = cpssDxChIpLpmIpv6McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                                            &ipSrc, ipSrcPrefixLen);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /*
            1.5. Call with lpmDBId[0],
                                        vrId[0],
                                        ipGroup[255.15.0.1],
                                        ipGroupPrefixLen[32],
                                        ipSrc[255.16.0.1],
                                        ipSrcPrefixLen[32] (already deleted).
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmIpv6McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                                            &ipSrc, ipSrcPrefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.6. Call with lpmDBId [0],
                           out of range vrId[4096]
                           and other valid parameters.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv6McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                                            &ipSrc, ipSrcPrefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.6. Call with lpmDBId [1],
                               out of range vrId[4096]
                               and other valid parameters.
                Expected: NOT GT_OK.
        */
        lpmDBId = 1;
        vrId    = 4096;

        st = cpssDxChIpLpmIpv6McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                                            &ipSrc, ipSrcPrefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.7. Call with out of range ipGroupPrefixLen [129] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        ipGroupPrefixLen = 129;

        st = cpssDxChIpLpmIpv6McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                                            &ipSrc, ipSrcPrefixLen);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipGroupPrefixLen = %d",
                                                    lpmDBId, ipGroupPrefixLen);

        ipGroupPrefixLen = 128;

        /*
            1.8. Call with out of range ipSrcPrefixLen [129] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        ipSrcPrefixLen = 129;

        st = cpssDxChIpLpmIpv6McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                                            &ipSrc, ipSrcPrefixLen);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipSrcPrefixLen = %d",
                                                          lpmDBId, ipSrcPrefixLen);

        ipSrcPrefixLen = 128;

        /*
            1.9. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                                            &ipSrc, ipSrcPrefixLen);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        /*
            1.10. Call cpssDxChIpLpmIpv6McEntriesFlush with with lpmDBId [1],
                  vrId[0] to invalidate entries.
            Expected: GT_OK.
        */
        lpmDBId = 1;

        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6McEntriesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6McEntriesFlush)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0], defUcNextHopInfoPtr{
        pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
        mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount = GT_FALSE,
        matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
        modifyUp=GT_FALSE , qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
        egressTaggedModify=GT_FALSE, modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
        nestedVlan=GT_FALSE, vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
        }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
        defMcRouteLttEntryPtr [NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK.
    1.3. Call with lpmDBId [0], vrId[0].
    Expected: GT_OK.
    1.4. Call with not valid LPM DB id lpmDBId [10] and other parameters from 3.
    Expected: NOT GT_OK.
    1.5. Call with lpmDBId [0 or 1], out of range vrId[4096] and other valid parameters.
    Expected: NOT GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                   lpmDBId = 0;
    GT_U32                   vrId    = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call with lpmDBId [0],
                           vrId[0].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /*
            1.4. Call with not valid LPM DB id lpmDBId [10]
                           and other parameters from 3.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.5. Call with lpmDBId [0],
                           out of range vrId[4096] and other valid parameters.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.5. Call with lpmDBId [1],
                               out of range vrId[4096]
                               and other valid parameters.
                Expected: NOT GT_OK.
        */
        lpmDBId = 1;
        vrId    = 4096;

        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6McEntryGetNext
(
    IN    GT_U32                      lpmDBId,
    IN    GT_U32                      vrId,
    INOUT GT_IPV6ADDR                 *ipGroupPtr,
    INOUT GT_U32                      *ipGroupPrefixLenPtr,
    INOUT GT_IPV6ADDR                 *ipSrcPtr,
    INOUT GT_U32                      *ipSrcPrefixLenPtr,
    OUT   CPSS_DXCH_IP_LTT_ENTRY_STC  *mcRouteLttEntryPtr,
    OUT   GT_U32                      *gRowIndexPtr,
    OUT   GT_U32                      *sRowIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6McEntryGetNext)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0],
        defUcNextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
        numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, defMcRouteLttEntryPtr {
        routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0], routeEntryBaseIndex[0],
        ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK .
    1.3. Call cpssDxChIpLpmIpv4McEntryAdd for 10 times with lpmDBId[0], vrId[0],
        ipGroup[255.15.0.1..10], ipGroupPrefixLen[32], ipSrc[255.16.0.1..10], ipSrcPrefixLen[32],
        mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
        routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
        defragmentationEnable[GT_FALSE].
    Expected: GT_OK.
    1.4. Call with lpmDBId[0], vrId[0], ipGroupPtr[255.15.0.1..9], ipGroupPrefixLenPtr [32],
        ipSrcPtr[255.16.0.1..9], ipSrcPrefixLenPtr[32] and non-null mcRouteLttEntryPtr.
    Expected: GT_OK and ipGroupPtr[255.15.0.2..10], ipGroupPrefixLenPtr[32],
        ipSrcPtr[255.16.0.2..10], ipSrcPrefixLenPtr[32] and the same mcRouteLttEntryPtr.
    1.5. Call with lpmDBId [0 or 1], out of range vrId[4096] and other valid parameters.
    Expected: NOT GT_OK.
    1.6. Call with out of range ipGroupPrefixLenPtr [129] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.7. Call with out of range ipSrcPrefixLenPtr [129] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.8. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.9. Call with lpmDBId [0], vrId[0], ipGroupPtr[NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.10. Call with lpmDBId [0], vrId[0], ipGroupPrefixLenPtr [NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.11. Call with lpmDBId [0], vrId[0], ipSrcPtr [NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.12. Call with lpmDBId [0], vrId[0], ipSrcPrefixLenPtr [NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.13. Call with lpmDBId [0], vrId[0], mcRouteLttEntryPtr [NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.14. Call cpssDxChIpLpmIpv6McEntriesFlush with with lpmDBId [0/1], vrId[0] to invalidate entries.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                     lpmDBId          = 0;
    GT_U32                     vrId             = 0;
    GT_IPV6ADDR                ipGroup;
    GT_U32                     ipGroupPrefixLen = 0;
    GT_IPV6ADDR                ipSrc;
    GT_U32                     ipSrcPrefixLen   = 0;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntry;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntryGet;
    GT_BOOL                    override         = GT_FALSE;
    GT_BOOL                    defragEnable     = GT_FALSE;
    GT_U8                      iter             = 0;
    GT_U32                     gRowIndex = 0;
    GT_U32                     sRowIndex = 0;

    cpssOsBzero((GT_VOID*) &ipGroup, sizeof(ipGroup));
    cpssOsBzero((GT_VOID*) &ipSrc, sizeof(ipSrc));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4McEntryAdd for 10 times with lpmDBId[0],
                 vrId[0], ipGroup[255.15.0.1..10], ipGroupPrefixLen[32], ipSrc[255.16.0.1..10],
                 ipSrcPrefixLen[32],
                 mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                 numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
                 sipSaCheckMismatchEnable[GT_TRUE],
                 ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                 override[GT_TRUE], defragmentationEnable[GT_FALSE].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 255;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 128;

        ipSrc.arIP[0] = 255;
        ipSrc.arIP[1] = 16;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        /* add ipGroup[255.15.0.1], ipSrc[255.16.0.1] */
        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                             ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv6McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv6McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
        }

        for (iter = 0; iter < 10; iter++)
        {
            ipGroup.arIP[3] = (GT_U8) (ipGroup.arIP[3] + 1);

            ipSrc.arIP[3] = (GT_U8) (ipSrc.arIP[3] + 1);

            st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                             ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
            if (prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                        "cpssDxChIpLpmIpv6McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
            }
            else
            {
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                        "cpssDxChIpLpmIpv6McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
            }
        }

        /*
            1.4. Call with lpmDBId[0], vrId[0], ipGroupPtr[255.15.0.1], ipGroupPrefixLenPtr
                [32], ipSrcPtr[255.16.0.1], ipSrcPrefixLenPtr[32] and non-null mcRouteLttEntryPtr.
            Expected: GT_OK and ipGroupPtr[255.15.0.2..10], ipGroupPrefixLenPtr[32],
                ipSrcPtr[255.16.0.2..10], ipSrcPrefixLenPtr[32] and the same mcRouteLttEntryPtr.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 255;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 128;

        ipSrc.arIP[0] = 255;
        ipSrc.arIP[1] = 16;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        for (iter = 0; iter < 10; iter++)
        {
           st = cpssDxChIpLpmIpv6McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                   &ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex, &sRowIndex);
           if (prvUtfIsPbrModeUsed())
           {
               UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
           }
           else
           {
               UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

               /* Verifying values */
               prvUtfCompareIpLttEntries(lpmDBId, vrId, &mcRouteLttEntryGet, &mcRouteLttEntry);

               UTF_VERIFY_EQUAL2_STRING_MAC(iter + 2, ipGroup.arIP[3],
                           "get another ipGroup.arIP[3] than was set: %d, %d", lpmDBId, vrId);
               UTF_VERIFY_EQUAL2_STRING_MAC(iter + 2, ipSrc.arIP[3],
                           "get another ipSrc.arIP[3] than was set: %d, %d", lpmDBId, vrId);
           }
        }

        /*
            1.5. Call with lpmDBId [0],
                           out of range vrId[4096] and other valid parameters.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv6McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                         &ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex, &sRowIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);


        /*
            1.5. Call with lpmDBId [1],
                               out of range vrId[4096]
                               and other valid parameters.
            Expected: NOT GT_OK.
        */
        lpmDBId = 1;
        vrId    = 4096;

        st = cpssDxChIpLpmIpv6McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                   &ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex, &sRowIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.6. Call with out of range ipGroupPrefixLenPtr [129] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        ipGroupPrefixLen = 129;

        st = cpssDxChIpLpmIpv6McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                   &ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex, &sRowIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipGroupPrefixLenPtr = %d",
                                                        lpmDBId, ipGroupPrefixLen);

        ipGroupPrefixLen = 128;

        /*
            1.7. Call with out of range ipSrcPrefixLenPtr [129] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        ipSrcPrefixLen = 129;

        st = cpssDxChIpLpmIpv6McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                             &ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex, &sRowIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipSrcPrefixLenPtr = %d", lpmDBId, ipSrcPrefixLen);

        ipSrcPrefixLen = 128;

        /*
            1.8. Call with not valid LPM DB id lpmDBId [10]
                           and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmIpv6McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                              &ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex, &sRowIndex);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.9. Call with lpmDBId [0],
                           vrId[0],
                           ipGroupPtr[NULL]
                           and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6McEntryGetNext(lpmDBId, vrId, NULL, &ipGroupPrefixLen, &ipSrc,
                                   &ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex, &sRowIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipGroupPtr = NULL", lpmDBId, vrId);

        /*
            1.10. Call with lpmDBId [0],
                           vrId[0],
                           ipGroupPrefixLenPtr [NULL]
                           and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6McEntryGetNext(lpmDBId, vrId, &ipGroup, NULL, &ipSrc,
                                   &ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex, &sRowIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipGroupPrefixLenPtr = NULL", lpmDBId, vrId);

        /*
            1.11. Call with lpmDBId [0], vrId[0], ipSrcPtr [NULL] and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, NULL,
                                             &ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex, &sRowIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipSrcPtr = NULL", lpmDBId, vrId);

        /*
            1.12. Call with lpmDBId [0],
                            vrId[0],
                            ipSrcPrefixLenPtr [NULL]
                            and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                             NULL, &mcRouteLttEntryGet, &gRowIndex, &sRowIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipSrcPrefixLenPtr = NULL", lpmDBId, vrId);

        /*
            1.13. Call with lpmDBId [0],
                            vrId[0],
                            mcRouteLttEntryPtr [NULL]
                            and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6McEntryGetNext(lpmDBId, vrId, &ipGroup, &ipGroupPrefixLen, &ipSrc,
                                             &ipSrcPrefixLen, NULL, &gRowIndex, &sRowIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, mcRouteLttEntryPtr = NULL",
                                                                            lpmDBId, vrId);

        /*
            1.14. Call cpssDxChIpLpmIpv6McEntriesFlush with with lpmDBId [0/1], vrId[0]
                    to invalidate entries.
            Expected: GT_OK.
        */
        lpmDBId = 0;

        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        lpmDBId = 1;

        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmIpv6McEntrySearch
(
    IN  GT_U32                      lpmDBId,
    IN  GT_U32                      vrId,
    IN  GT_IPV6ADDR                 ipGroup,
    IN  GT_U32                      ipGroupPrefixLen,
    IN  GT_IPV6ADDR                 ipSrc,
    IN  GT_U32                      ipSrcPrefixLen,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC  *mcRouteLttEntryPtr,
    OUT GT_U32                      *gRowIndexPtr,
    OUT GT_U32                      *sRowIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6McEntrySearch)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
        indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
        tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10], numOfIpv4McSourcePrefixes [5],
        numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId [0], vrId[0],
        defUcNextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
        numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, defMcRouteLttEntryPtr {
        routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0], routeEntryBaseIndex[0],
        ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK .
    1.4. Call cpssDxChIpLpmIpv4McEntryAdd for 10 times with lpmDBId[0], vrId[0],
        ipGroup[255.15.0.1..10], ipGroupPrefixLen[32], ipSrc[255.16.0.1..10], ipSrcPrefixLen[32],
        mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
        routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE], sipSaCheckMismatchEnable[GT_TRUE],
        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
        defragmentationEnable[GT_FALSE].
    Expected: GT_OK.
    1.5. Call with lpmDBId[0], vrId[0], ipGroup[255.15.0.1], ipGroupPrefixLen [32],
    ipSrc[255.16.0.1], ipSrcPrefixLen[32] and non-null mcRouteLttEntry.
    Expected: GT_OK and the same mcRouteLttEntryPtr.
    1.6. Call with lpmDBId [0 or 1], out of range vrId[4096] and other valid parameters.
    Expected: NOT GT_OK.
    1.7. Call with out of range ipGroupPrefixLen [129] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.8. Call with out of range ipSrcPrefixLen [129] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.9. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.10. Call with lpmDBId [0], vrId[0], mcRouteLttEntryPtr [NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.11. Call with ipGroupPtr[NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.12. Call with ipSrcPtr[NULL] and other valid parameters.
    Expected: GT_BAD_PTR.
    1.13. Call cpssDxChIpLpmIpv6McEntriesFlush with with lpmDBId [0/1], vrId[0] to invalidate entries.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                     lpmDBId          = 0;
    GT_U32                     vrId             = 0;
    GT_IPV6ADDR                ipGroup;
    GT_U32                     ipGroupPrefixLen = 0;
    GT_IPV6ADDR                ipSrc;
    GT_U32                     ipSrcPrefixLen   = 0;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntry;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntryGet;
    GT_BOOL                    override         = GT_FALSE;
    GT_BOOL                    defragEnable     = GT_FALSE;
    GT_U8                      iter             = 0;
    GT_U32                     gRowIndex;
    GT_U32                     sRowIndex;

    cpssOsBzero((GT_VOID*) &ipGroup, sizeof(ipGroup));
    cpssOsBzero((GT_VOID*) &ipSrc, sizeof(ipSrc));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. - 1.2.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.3. Call cpssDxChIpLpmIpv4McEntryAdd for 10 times with lpmDBId[0],
                 vrId[0], ipGroup[255.15.0.1..10], ipGroupPrefixLen[32], ipSrc[255.16.0.1..10],
                 ipSrcPrefixLen[32],
                 mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                 numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
                 sipSaCheckMismatchEnable[GT_TRUE],
                 ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]},
                 override[GT_TRUE], defragmentationEnable[GT_FALSE].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 255;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 128;

        ipSrc.arIP[0] = 255;
        ipSrc.arIP[1] = 16;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        /* add ipGroup[255.15.0.1],ipSrc[255.16.0.1] */
        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                             ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv6McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv6McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
        }

        for (iter = 0; iter < 10; iter++)
        {
            ipGroup.arIP[3] = (GT_U8) (ipGroup.arIP[3] + 1);

            ipSrc.arIP[3] = (GT_U8) (ipSrc.arIP[3] + 1);

            st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                             ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
            if (prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                        "cpssDxChIpLpmIpv6McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
            }
            else
            {
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLpmIpv6McEntryAdd: %d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
            }
        }

        /*
            1.5. Call with lpmDBId[0], vrId[0], ipGroup[255.15.0.1], ipGroupPrefixLen [32],
                ipSrc[255.16.0.1], ipSrcPrefixLen[32] and non-null mcRouteLttEntry.
            Expected: GT_OK and the same mcRouteLttEntryPtr.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 255;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 128;

        ipSrc.arIP[0] = 255;
        ipSrc.arIP[1] = 16;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        for (iter = 0; iter < 10; iter++)
        {
            st = cpssDxChIpLpmIpv6McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                    ipSrcPrefixLen, &mcRouteLttEntryGet, &gRowIndex, &sRowIndex);
            if (prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

                prvUtfCompareIpLttEntries(lpmDBId,vrId, &mcRouteLttEntryGet, &mcRouteLttEntry);
            }
        }

        /*
            1.6. Call with lpmDBId [0], out of range vrId[4096] and other valid parameters.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmIpv6McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                        ipSrcPrefixLen, &mcRouteLttEntry, &gRowIndex, &sRowIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.7. Call with lpmDBId [1], out of range vrId[10] and other valid parameters.
            Expected: NOT GT_OK.
        */
        lpmDBId = 1;
        vrId    = 10;

        st = cpssDxChIpLpmIpv6McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                            ipSrcPrefixLen, &mcRouteLttEntry, &gRowIndex, &sRowIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.8. Call with out of range ipGroupPrefixLen [129] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        ipGroupPrefixLen = 129;

        st = cpssDxChIpLpmIpv6McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                            ipSrcPrefixLen, &mcRouteLttEntry, &gRowIndex, &sRowIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipGroupPrefixLen = %d", lpmDBId, ipGroupPrefixLen);

        ipGroupPrefixLen = 128;

        /*
            1.9. Call with out of range ipSrcPrefixLen [129] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        ipSrcPrefixLen = 129;

        st = cpssDxChIpLpmIpv6McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                            ipSrcPrefixLen, &mcRouteLttEntry, &gRowIndex, &sRowIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipSrcPrefixLen = %d", lpmDBId, ipSrcPrefixLen);

        ipSrcPrefixLen = 128;

        /*
            1.10. Call with lpmDBId [0], vrId[0], mcRouteLttEntryPtr [NULL] and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                            ipSrcPrefixLen, NULL, &gRowIndex, &sRowIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, mcRouteLttEntryPtr = NULL", lpmDBId, vrId);

        /*
            1.11. Call with ipGroupPtr[NULL] and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6McEntrySearch(lpmDBId, vrId, NULL, ipGroupPrefixLen, &ipSrc,
                                            ipSrcPrefixLen, &mcRouteLttEntry, &gRowIndex, &sRowIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipGroupPtr = NULL", lpmDBId, vrId);

        /*
            1.12. Call with ipSrcPtr[NULL] and other valid parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6McEntrySearch(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, NULL,
                                            ipSrcPrefixLen, &mcRouteLttEntry, &gRowIndex, &sRowIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, ipSrcPtr = NULL", lpmDBId, vrId);

        /*
            1.13. Call cpssDxChIpLpmIpv6McEntriesFlush with with lpmDBId [0/1], vrId[0] to invalidate entries.
            Expected: GT_OK.
        */

        lpmDBId = 0;

        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        lpmDBId = 1;

        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmDBMemSizeGet
(
    IN    GT_U32 lpmDBId,
    INOUT GT_U32 *lpmDbSizePtr,
    IN    GT_U32 numOfEntriesInStep,
    INOUT GT_U32 *iterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDBMemSizeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non valid lpmDBId [100], lpmDbSizePtr[0]
    Expected: NOT GT_OK.
    1.2. Call cpssDxChIpLpmDBCreate with lpmDBId [100]
    Expected: GT_OK.
    1.3. Call with lpmDBId [100], lpmDbSizePtr[0]
    Expected: GT_OK.
    1.4. Call with lpmDbSizePtr[NULL] and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call cpssDxChIpLpmDBDelete with lpmDBId [100]
    Expected: GT_OK
*/
    GT_STATUS   st           = GT_OK;

    GT_U32      lpmDBId      = 0;
    GT_U32      lpmDbSize    = 0;
    GT_U8       dev;

    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT                shadowType, tmpShadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    tcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT              lpmMemoryConfig;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&indexesRange, sizeof(indexesRange));
    cpssOsBzero((GT_VOID*)&lpmMemoryConfig, sizeof(lpmMemoryConfig));
    cpssOsBzero((GT_VOID*)&tcamLpmManagerCapcityCfg, sizeof(tcamLpmManagerCapcityCfg));

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        lpmDBId   = 100;
        lpmDbSize = 0;

        /*
            1.1. Call with non valid lpmDBId [100], lpmDbSizePtr[0].
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmDBMemSizeGet(lpmDBId,&lpmDbSize);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,lpmDbSize);

        /*
            1.2. Call cpssDxChIpLpmDBCreate with lpmDBId [100]
            Expected: GT_OK.
        */
        shadowType                                          = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
        protocolStack                                       = CPSS_IP_PROTOCOL_IPV4V6_E;
        indexesRange.firstIndex                             = 100;
        indexesRange.lastIndex                              = 199;
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr           = &indexesRange;
        lpmMemoryConfig.tcamDbCfg.partitionEnable             = GT_TRUE;
        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes          = 10;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes  = 10;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes          = 10;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr     = NULL;

        st = prvUtfGetDevShadow(dev, &tmpShadowType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tmpShadowType);
        if (tmpShadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;

            st = prvUtfIpLpmRamDefaultConfigCalc(dev,&lpmMemoryConfig.ramDbCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);
        }

        st = cpssDxChIpLpmDBCreate(lpmDBId,shadowType,protocolStack,
                                   &lpmMemoryConfig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate");

        /*
            1.3. Call with lpmDBId [100], lpmDbSizePtr[0], numOfEntriesInStep[1], iterPtr[0].
            Expected: GT_OK.
        */
        st = cpssDxChIpLpmDBMemSizeGet(lpmDBId,&lpmDbSize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,lpmDbSize);

        lpmDbSize = 0;

        /*
            1.4. Call with lpmDbSizePtr[NULL] and other parameters from 1.1.
        Expected: GT_BAD_PTR.
    */
        st = cpssDxChIpLpmDBMemSizeGet(lpmDBId, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR,st,lpmDBId,lpmDbSize);


        /*
            1.5. Call cpssDxChIpLpmDBDelete with lpmDBId [100]
            Expected: GT_OK
        */
        st = cpssDxChIpLpmDBDelete(lpmDBId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,lpmDbSize);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmDBExport
(
    IN    GT_U32 lpmDBId,
    OUT   GT_U32 *lpmDbPtr,
    IN    GT_U32 numOfEntriesInStep,
    INOUT GT_U32 *iterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDBExport)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non valid lpmDBId [100], smallBuffer, numInStep[1], iter[0].
    Expected: NOT GT_OK.
    1.2. Call cpssDxChIpLpmDBCreate with lpmDBId [100]
    Expected: GT_OK.
    1.3. Call with lpmDbPtr[NULL] and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with numInStep[0] and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with iter[NULL] and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.6. Get lpmDBId [100] LPM DB data size
    Expected: GT_OK.
    1.7. Call with lpmDBId [100], lpmDbPtr[lpmDbDataPtr], iter[0]
         and various values of numOfEntriesInStep
    Expected: GT_OK.
    1.8. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId[100], vrId[1]
    Expected: GT_OK
    1.9. Add some prefixes to lpmDBId[100], vrId[1]
    Expected: GT_OK
    1.10. Get lpmDBId [100] LPM DB data size
    Expected: GT_OK.
    1.11. Call with lpmDBId [100], lpmDbPtr[lpmDbDataPtr], iter[0]
          and various values of numOfEntriesInStep
    Expected: GT_OK
    1.12. Call cpssDxChIpLpmVirtualRouterDel with lpmDBId[100], vrId[1]
    Expected: GT_OK
    1.13. Call cpssDxChIpLpmDBDelete with lpmDBId [100]
    Expected: GT_OK
*/
    GT_STATUS   st           = GT_OK;

    GT_U32      lpmDBId      = 0;
    GT_U32      lpmDbSize    = 0;
    GT_U32      exportSize    = 0;
    GT_U32      tempExportSize = 0;
    GT_UINTPTR  iter         = 0;
    GT_U32      maxExportSize = 0;
    GT_U8       dev;

    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT                shadowType, tmpShadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    tcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT              lpmMemoryConfig;

    GT_U32                                          vrId;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC                  vrConfig;

    GT_IPADDR                                       ipAddr;
    GT_U32                                          prefixLen;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT          nextHopInfo;
    GT_BOOL                                         override;
    GT_BOOL                                         defragmentationEnable = GT_FALSE;

    GT_U32                                          *lpmDbDataPtr = NULL;
    GT_U32                                          smallBuffer[_1K];

    /* prepare device iterator */
    CPSS_TBD_BOOKMARK_EARCH /* The test should run on Bobcat2 after adding LPM import/export support. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*)&indexesRange, sizeof(indexesRange));
    cpssOsBzero((GT_VOID*)&tcamLpmManagerCapcityCfg, sizeof(tcamLpmManagerCapcityCfg));
    cpssOsBzero((GT_VOID*)&lpmMemoryConfig, sizeof(lpmMemoryConfig));
    cpssOsBzero((GT_VOID*)&vrConfig, sizeof(vrConfig));
    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non valid lpmDBId [100], smallBuffer, numInStep[1], iter[0].
            Expected: NOT GT_OK.
        */
        lpmDBId   = 100;
        exportSize = 0xffffffff; /* one iteration*/
        iter      = 0;
        st = cpssDxChIpLpmDBExport(lpmDBId,smallBuffer,&exportSize,&iter);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,iter);

        /*
            1.2. Call cpssDxChIpLpmDBCreate with lpmDBId [100]
            Expected: GT_OK.
        */
        shadowType                                          = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
        protocolStack                                       = CPSS_IP_PROTOCOL_IPV4V6_E;
        indexesRange.firstIndex                             = 100;
        indexesRange.lastIndex                              = 199;
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr           = &indexesRange;
        lpmMemoryConfig.tcamDbCfg.partitionEnable             = GT_TRUE;
        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes          = 10;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes  = 10;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes          = 10;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr     = NULL;

        st = prvUtfGetDevShadow(dev, &tmpShadowType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tmpShadowType);
        if (tmpShadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
        }

        st = cpssDxChIpLpmDBCreate(lpmDBId,shadowType,protocolStack,
                                   &lpmMemoryConfig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate");

        /*
            1.3. Call with lpmDbPtr[NULL] and other parameters from 1.1.
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmDBExport(lpmDBId,NULL,&exportSize,&iter);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,iter);

        /*
            1.4. Call with numInStep[0] and other parameters from 1.1.
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmDBExport(lpmDBId,smallBuffer,NULL,&iter);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,iter);

        /*
            1.5. Call with iter[NULL] and other parameters from 1.1.
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmDBExport(lpmDBId,smallBuffer,&exportSize, NULL);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,iter);

        /*
            1.6. Get lpmDBId [100] LPM DB data size
            Expected: GT_OK.
        */
        lpmDbSize = 0;
        iter = 0;
        st = cpssDxChIpLpmDBMemSizeGet(lpmDBId,&lpmDbSize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,lpmDbSize);

        /* allocate memory for LPM DB data */
        lpmDbDataPtr = (GT_U32*)cpssOsMalloc(lpmDbSize);
        if (lpmDbDataPtr == NULL)
        {
            prvUtfLogMessage("fail to allocate memory\n",NULL,0);
            return;
        }

        /*
            1.7. Call with lpmDBId [100], lpmDbPtr[lpmDbDataPtr], iter[0]
                 and various values of numOfEntriesInStep
            Expected: GT_OK.
        */
        maxExportSize = lpmDbSize + 243;
        for (exportSize = _1K ; exportSize < maxExportSize ; exportSize += 111)
        {
            iter = 0;
            tempExportSize = exportSize;
            do
            {
                st = cpssDxChIpLpmDBExport(lpmDBId,lpmDbDataPtr,&exportSize,&iter);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK,st,lpmDBId,exportSize,iter);
                exportSize = tempExportSize + exportSize;
            } while (iter != 0);
        }

        /* free LPM DB data memory */
        cpssOsFree(lpmDbDataPtr);

        /*
            1.8. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId[100], vrId[1]
            Expected: GT_OK
        */
        vrId = 1;
        vrConfig.supportIpv4Uc = 1;
        cpssOsMemSet(&vrConfig.defIpv4UcNextHopInfo,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        vrConfig.supportIpv4Mc = 1;
        cpssOsMemSet(&vrConfig.defIpv4McRouteLttEntry,0,sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfig.supportIpv6Uc = 1;
        cpssOsMemSet(&vrConfig.defIpv6UcNextHopInfo,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        vrConfig.supportIpv6Mc = 1;
        cpssOsMemSet(&vrConfig.defIpv6McRouteLttEntry,0,sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfig.supportFcoe = 0;
        cpssOsMemSet(&vrConfig.defaultFcoeForwardingNextHopInfo,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId,vrId,&vrConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,vrId);

        /*
            1.9. Add some prefixes to lpmDBId[100], vrId[1]
            Expected: GT_OK
        */
        ipAddr.arIP[0] = 1; ipAddr.arIP[1] = 1; ipAddr.arIP[2] = 1; ipAddr.arIP[3] = 3;
        prefixLen = 32;
        cpssOsMemSet(&nextHopInfo,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        override = GT_FALSE;
        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId,vrId,&ipAddr,prefixLen,&nextHopInfo,override, defragmentationEnable);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,st);

        ipAddr.arIP[0] = 5; ipAddr.arIP[1] = 5; ipAddr.arIP[2] = 4; ipAddr.arIP[3] = 3;
        prefixLen = 32;
        cpssOsMemSet(&nextHopInfo,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        override = GT_FALSE;
        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId,vrId,&ipAddr,prefixLen,&nextHopInfo,override, defragmentationEnable);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,st);

        /*
            1.10. Get lpmDBId [100] LPM DB data size
            Expected: GT_OK.
        */
        lpmDbSize = 0;
        iter = 0;
        st = cpssDxChIpLpmDBMemSizeGet(lpmDBId,&lpmDbSize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,lpmDbSize);

        /* allocate memory for LPM DB data */
        lpmDbDataPtr = (GT_U32*)cpssOsMalloc(lpmDbSize);
        if (lpmDbDataPtr == NULL)
        {
            prvUtfLogMessage("fail to allocate memory\n",NULL,0);
            return;
        }

        /*
            1.11. Call with lpmDBId [100], lpmDbPtr[lpmDbDataPtr], iter[0]
                  and various values of numOfEntriesInStep
            Expected: GT_OK
        */
        maxExportSize = lpmDbSize + 221;
        for (exportSize = _1K + 1 ; exportSize < maxExportSize ; exportSize+=19)
        {
            iter = 0;
            tempExportSize = exportSize;
            do
            {
                st = cpssDxChIpLpmDBExport(lpmDBId,lpmDbDataPtr,&exportSize,&iter);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK,st,lpmDBId,exportSize,iter);
                exportSize = tempExportSize + exportSize;
            } while (iter != 0);
        }

        /* free LPM DB data memory */
        cpssOsFree(lpmDbDataPtr);

        /*
            1.12. Call cpssDxChIpLpmVirtualRouterDel with lpmDBId[100], vrId[1]
            Expected: GT_OK
        */
        vrId = 1;
        st = cpssDxChIpLpmVirtualRouterDel(lpmDBId,vrId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK,st,lpmDBId,lpmDbSize,iter);

        /*
            1.13. Call cpssDxChIpLpmDBDelete with lpmDBId [100]
            Expected: GT_OK
        */
        st = cpssDxChIpLpmDBDelete(lpmDBId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK,st,lpmDBId,lpmDbSize,iter);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmDBImport
(
    IN    GT_U32 lpmDBId,
    IN    GT_U32 *lpmDbPtr,
    IN    GT_U32 numOfEntriesInStep,
    INOUT GT_U32 *iterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDBImport)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non valid lpmDBId [100], smallBuffer, numInStep[1], iter[0].
    Expected: NOT GT_OK.
    1.2. Call cpssDxChIpLpmDBCreate with lpmDBId [100]
    Expected: GT_OK.
    1.3. Call with lpmDbPtr[NULL] and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with numInStep[0] and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with iter[NULL] and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.6. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId[100], vrId[1]
    Expected: GT_OK
    1.7. Add some UC prefixes to lpmDBId[100], vrId[1]
    Expected: GT_OK
    1.8. Get UC prefixes TCAM location
    Expected: GT_OK
    1.9. Get lpmDBId [100] LPM DB data size
    Expected: GT_OK.
    1.10. Get lpmDBId [100] LPM DB data
    Expected: GT_OK
    1.11. Call with lpmDBId [100], lpmDbPtr[lpmDbDataPtr], iter[0]
          and various values of numOfEntriesInStep
    Expected: GT_OK
    1.12. Call cpssDxChIpLpmVirtualRouterDel with lpmDBId[100], vrId[1]
    Expected: GT_OK
    1.13. Call cpssDxChIpLpmDBDelete with lpmDBId [100]
    Expected: GT_OK
*/
    GT_STATUS   st           = GT_OK;

    GT_U32      lpmDBId      = 0;
    GT_U32      lpmDbSize    = 0;
    GT_U32      importSize     = 0;
    GT_U32      tempImportSize = 0;

    GT_UINTPTR  iter         = 0;
    GT_U32      maxImportSize  = 0;
    GT_U8       dev;

    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT                shadowType, tmpShadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    tcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT              lpmMemoryConfig;

    GT_U32                                          vrId;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC                  vrConfig;

    GT_IPADDR                                       ipAddr;
    GT_U32                                          prefixLen;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT          nextHopInfo;
    GT_BOOL                                         override;
    GT_BOOL                                         defragmentationEnable = GT_FALSE;

    GT_U32                                          *lpmDbDataPtr = NULL;
    GT_U32                                          smallBuffer[_1K];
    GT_U32                                          i;

    struct
    {
        CPSS_IP_PROTOCOL_STACK_ENT      protocolStack;
        GT_U8                           ipAddr[16];
        GT_U32                          prefixLen;
        GT_U32                          tcamRow;
        GT_U32                          tcamColumn;
    } ucTcamLocation[10];

    GT_U32      ucTcamLocationsUsed;
    GT_U32      tcamRow = 0;
    GT_U32      tcamColumn = 0;
    GT_U32      prefixesAfterImport;

    /* prepare device iterator */
    CPSS_TBD_BOOKMARK_EARCH /* The test should run on Bobcat2 after adding LPM import/export support. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*)&indexesRange, sizeof(indexesRange));
    cpssOsBzero((GT_VOID*)&tcamLpmManagerCapcityCfg, sizeof(tcamLpmManagerCapcityCfg));
    cpssOsBzero((GT_VOID*)&lpmMemoryConfig, sizeof(lpmMemoryConfig));
    cpssOsBzero((GT_VOID*)&vrConfig, sizeof(vrConfig));
    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));


    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non valid lpmDBId [100], smallBuffer, numInStep[1], iter[0].
            Expected: NOT GT_OK.
        */
        lpmDBId   = 100;
        importSize = 0xffffffff;
        iter      = 0;
        st = cpssDxChIpLpmDBImport(lpmDBId,smallBuffer,&importSize,&iter);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,iter);

        /*
            1.2. Call cpssDxChIpLpmDBCreate with lpmDBId [100]
            Expected: GT_OK.
        */
        shadowType                                          = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
        protocolStack                                       = CPSS_IP_PROTOCOL_IPV4V6_E;
        indexesRange.firstIndex                             = 100;
        indexesRange.lastIndex                              = 199;
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr           = &indexesRange;
        lpmMemoryConfig.tcamDbCfg.partitionEnable             = GT_TRUE;
        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes          = 10;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes  = 10;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes          = 10;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr     = NULL;

        st = prvUtfGetDevShadow(dev, &tmpShadowType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tmpShadowType);
        if (tmpShadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
        }

        st = cpssDxChIpLpmDBCreate(lpmDBId,shadowType,protocolStack,
                                   &lpmMemoryConfig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate");

        /*
            1.3. Call with lpmDbPtr[NULL] and other parameters from 1.1.
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmDBImport(lpmDBId,NULL,&importSize,&iter);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,iter);

        /*
            1.4. Call with importSize[0], and NULL and other parameters from 1.1.
            Expected: NOT GT_OK.
        */
        importSize = 0;
        st = cpssDxChIpLpmDBImport(lpmDBId,smallBuffer,&importSize,&iter);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK,st,lpmDBId,importSize,iter);
        st = cpssDxChIpLpmDBImport(lpmDBId,smallBuffer,NULL,&iter);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,iter);
        importSize = 0xffffffff;

        /*
            1.5. Call with iter[NULL] and other parameters from 1.1.
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmDBImport(lpmDBId,smallBuffer,&importSize,NULL);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,iter);

        /*
            1.6. Call cpssDxChIpLpmVirtualRouterAdd with lpmDBId[100], vrId[1]
            Expected: GT_OK
        */
        vrId = 1;
        vrConfig.supportIpv4Uc = 1;
        cpssOsMemSet(&vrConfig.defIpv4UcNextHopInfo,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        vrConfig.supportIpv4Mc = 1;
        cpssOsMemSet(&vrConfig.defIpv4McRouteLttEntry,0,sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfig.supportIpv6Uc = 1;
        cpssOsMemSet(&vrConfig.defIpv6UcNextHopInfo,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        vrConfig.supportIpv6Mc = 1;
        cpssOsMemSet(&vrConfig.defIpv6McRouteLttEntry,0,sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfig.supportFcoe = 0;
        cpssOsMemSet(&vrConfig.defaultFcoeForwardingNextHopInfo,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId,vrId,&vrConfig);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,vrId);

        /*
            1.7. Add some UC prefixes to lpmDBId[100], vrId[1]
            Expected: GT_OK
        */
        ipAddr.arIP[0] = 1; ipAddr.arIP[1] = 1; ipAddr.arIP[2] = 1; ipAddr.arIP[3] = 3;
        prefixLen = 32;
        cpssOsMemSet(&nextHopInfo,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        override = GT_FALSE;
        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId,vrId,&ipAddr,prefixLen,&nextHopInfo,override, defragmentationEnable);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,st);

        ipAddr.arIP[0] = 5; ipAddr.arIP[1] = 5; ipAddr.arIP[2] = 4; ipAddr.arIP[3] = 3;
        prefixLen = 24;
        cpssOsMemSet(&nextHopInfo,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        override = GT_FALSE;
        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId,vrId,&ipAddr,prefixLen,&nextHopInfo,override, defragmentationEnable);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,st);

        /* init the TCAM location array */
        ucTcamLocationsUsed = 0;

        /*
            1.8. Get UC prefixes TCAM location
            Expected: GT_OK
        */
        ipAddr.arIP[0] = 0; ipAddr.arIP[1] = 0; ipAddr.arIP[2] = 0; ipAddr.arIP[3] = 0;
        prefixLen = 0;
        st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId,vrId,&ipAddr,prefixLen,&nextHopInfo,&tcamRow,&tcamColumn);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,st);
        if (st == GT_OK)
        {
            ucTcamLocation[ucTcamLocationsUsed].protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
            cpssOsMemCpy(ucTcamLocation[ucTcamLocationsUsed].ipAddr,ipAddr.arIP,4*sizeof(GT_U8));
            ucTcamLocation[ucTcamLocationsUsed].prefixLen     = prefixLen;
            ucTcamLocation[ucTcamLocationsUsed].tcamRow       = tcamRow;
            ucTcamLocation[ucTcamLocationsUsed].tcamColumn    = tcamColumn;
            ucTcamLocationsUsed++;
        }

        do
        {
            st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId,vrId,&ipAddr,&prefixLen,&nextHopInfo,&tcamRow,&tcamColumn);
            if (st != GT_NOT_FOUND)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,st);
            }
            if (st == GT_OK)
            {
                ucTcamLocation[ucTcamLocationsUsed].protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
                cpssOsMemCpy(ucTcamLocation[ucTcamLocationsUsed].ipAddr,ipAddr.arIP,4*sizeof(GT_U8));
                ucTcamLocation[ucTcamLocationsUsed].prefixLen     = prefixLen;
                ucTcamLocation[ucTcamLocationsUsed].tcamRow       = tcamRow;
                ucTcamLocation[ucTcamLocationsUsed].tcamColumn    = tcamColumn;
                ucTcamLocationsUsed++;
            }
        } while ((st == GT_OK) && (ucTcamLocationsUsed < 10));

        /*
            1.9. Get lpmDBId [100] LPM DB data size
            Expected: GT_OK.
        */
        lpmDbSize = 0;
        st = cpssDxChIpLpmDBMemSizeGet(lpmDBId,&lpmDbSize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,lpmDbSize);

        /* allocate memory for LPM DB data */
        lpmDbDataPtr = (GT_U32*)cpssOsMalloc(lpmDbSize);
        if (lpmDbDataPtr == NULL)
        {
            prvUtfLogMessage("fail to allocate memory\n",NULL,0);
            return;
        }

        /*
            1.10. Get lpmDBId [100] LPM DB data
            Expected: GT_OK
        */
        importSize = 0xffffffff;
        iter = 0;
        do
        {
            st = cpssDxChIpLpmDBExport(lpmDBId,lpmDbDataPtr,&importSize,&iter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK,st,lpmDBId,iter);
        } while (iter != 0);

        /*
            1.11. Call with lpmDBId [100], lpmDbPtr[lpmDbDataPtr], iter[0]
                  and various values of importSize
            Expected: GT_OK
        */
        maxImportSize = lpmDbSize + 221;
        for (importSize = _1K + 7 ; importSize < maxImportSize ; importSize+=119)
        {
            /* delete the VR */
            vrId = 1;
            st = cpssDxChIpLpmVirtualRouterDel(lpmDBId,vrId);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK,st,lpmDBId,lpmDbSize,importSize,iter);

            /* import the LPM DB data */
            iter = 0;
            tempImportSize = importSize;
            do
            {
                st = cpssDxChIpLpmDBImport(lpmDBId,lpmDbDataPtr,&importSize,&iter);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK,st,lpmDBId,importSize,iter);
                importSize = tempImportSize + importSize;
            } while (iter != 0);

            prefixesAfterImport = 0;

            /* verify the UC prefix tcam location */
            ipAddr.arIP[0] = 0; ipAddr.arIP[1] = 0; ipAddr.arIP[2] = 0; ipAddr.arIP[3] = 0;
            prefixLen = 0;
            st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId,vrId,&ipAddr,prefixLen,&nextHopInfo,&tcamRow,&tcamColumn);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,st);
            if (st == GT_OK)
            {
                prefixesAfterImport++;
                for (i = 0 ; i < ucTcamLocationsUsed ; i++)
                {
                    /* check if ucTcamLocation entry match this prefix */
                    if ((ucTcamLocation[i].protocolStack == CPSS_IP_PROTOCOL_IPV4_E) &&
                        (cpssOsMemCmp(ucTcamLocation[i].ipAddr,ipAddr.arIP,sizeof(GT_U8)*(prefixLen/8)) == 0) &&
                        (ucTcamLocation[i].prefixLen == prefixLen))
                    {
                        /* verify TCAM location is the same */
                        UTF_VERIFY_EQUAL0_STRING_MAC(ucTcamLocation[i].tcamRow,tcamRow,"tcam row doesn't match after import");
                        UTF_VERIFY_EQUAL0_STRING_MAC(ucTcamLocation[i].tcamColumn,tcamColumn,"tcam column doesn't match after import");
                        break;
                    }
                    /* check if the prefix was found in the array */
                    if (i == ucTcamLocationsUsed)
                    {
                        UTF_VERIFY_EQUAL0_STRING_MAC(0,1,"prefix wasn't found after import");
                    }
                }
            }

            do
            {
                st = cpssDxChIpLpmIpv4UcPrefixGetNext(lpmDBId,vrId,&ipAddr,&prefixLen,&nextHopInfo,&tcamRow,&tcamColumn);
                if (st != GT_NOT_FOUND)
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,st);
                }
                if (st == GT_OK)
                {
                    prefixesAfterImport++;
                    for (i = 0 ; i < ucTcamLocationsUsed ; i++)
                    {
                        /* check if ucTcamLocation entry match this prefix */
                        if ((ucTcamLocation[i].protocolStack == CPSS_IP_PROTOCOL_IPV4_E) &&
                            (cpssOsMemCmp(ucTcamLocation[i].ipAddr,ipAddr.arIP,sizeof(GT_U8)*(prefixLen/8)) == 0) &&
                            (ucTcamLocation[i].prefixLen == prefixLen))
                        {
                            /* verify TCAM location is the same */
                            UTF_VERIFY_EQUAL0_STRING_MAC(ucTcamLocation[i].tcamRow,tcamRow,"tcam row doesn't match after import");
                            UTF_VERIFY_EQUAL0_STRING_MAC(ucTcamLocation[i].tcamColumn,tcamColumn,"tcam column doesn't match after import");
                            break;
                        }
                        /* check if the prefix was found in the array */
                        if (i == ucTcamLocationsUsed)
                        {
                            UTF_VERIFY_EQUAL0_STRING_MAC(0,1,"prefix wasn't found after import");
                        }
                    }
                }
            } while ((st == GT_OK) && (ucTcamLocationsUsed < 10));

            /* verify that number of prefixes before import match number of prefixes after import */
            UTF_VERIFY_EQUAL0_STRING_MAC(prefixesAfterImport,ucTcamLocationsUsed,"number of prefixes doesn't match after import");
        }

        /*
            1.12. Call cpssDxChIpLpmVirtualRouterDel with lpmDBId[100], vrId[1]
            Expected: GT_OK
        */
        vrId = 1;
        st = cpssDxChIpLpmVirtualRouterDel(lpmDBId,vrId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK,st,lpmDBId,lpmDbSize,iter);

        /*
            1.13. Call cpssDxChIpLpmDBDelete with lpmDBId [100]
            Expected: GT_OK
        */
        st = cpssDxChIpLpmDBDelete(lpmDBId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK,st,lpmDBId,lpmDbSize,iter);
    }
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet
(
    IN  GT_U32  lpmDBId,
    IN  GT_U32  pclIdArrayLen,
    IN  GT_U32  pclIdArray[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [2],
                 protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
                 indexesRangePtr{ firstIndex [200],
                                  lastIndex [1000]},
                 partitionEnable[GT_TRUE],
                 tcamLpmManagerCapcityCfgPtr{numOfIpv4Prefixes [10],
                                             numOfIpv4McSourcePrefixes [5],
                                             numOfIpv6Prefixes [10]}
                 to create LPM DB.
    Expected: GT_OK.
    1.2. Call with multicast lpmDb configuration
         (lpmDB without virtual routers).
    Expected: GT_OK.
    1.3. Call cpssDxChIpLpmDBCreate with lpmDBId [0 / 1],
         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
         indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
         tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10],
         numOfIpv4McSourcePrefixes [5], numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.4. Call with lpmDb with Vr. The LPM DB must be empty when
         using this API (can't contain any prefixes or virtual routers).
    Expected: GT_BAD_STATE.
    1.5. Call with lpmDb [4] (not exists).
    Expected: NOT GT_OK.
*/
    GT_U8      dev       = 0;
    GT_STATUS  st        = GT_OK;

    GT_U32     lpmDBId   = 0;
    GT_U32     pclIdArray[1];

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [2],
                         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
                         indexesRangePtr{ firstIndex [200],
                                          lastIndex [1000]},
                         partitionEnable[GT_TRUE],
                         tcamLpmManagerCapcityCfgPtr{numOfIpv4Prefixes [10],
                                                     numOfIpv4McSourcePrefixes [5],
                                                     numOfIpv6Prefixes [10]}
                         to create LPM DB.
            Expected: GT_OK.
        */

        /* create lpmDBId [2] for policy based routing*/
        st = prvUtfCreateMcLpmDB(dev);
        if(st != GT_NOT_IMPLEMENTED)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateMcLpmDB");
        }




        if (GT_OK == st)
        {
            /*
                1.2. Call with multicast lpmDb configuration
                     (lpmDB without virtual routers).
                Expected: GT_OK.
            */
            lpmDBId  = 2;
            pclIdArray[0] = 0;

            st = cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet(lpmDBId,1,pclIdArray);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d", lpmDBId);
        }

        /*
            1.3. Call cpssDxChIpLpmDBCreate with lpmDBId [0 / 1],
                 protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
                 indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
                 tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10],
                 numOfIpv4McSourcePrefixes [5], numOfIpv6Prefixes [10]} to create LPM DB.
            Expected: GT_OK.
        */

        /* create lpmDBId [0] for policy based routing*/
        st = prvUtfCreateDefaultLpmDB(dev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateDefaultLpmDB");

        if (GT_OK == st)
        {
            /*
                1.4. Call with lpmDb with Vr. The LPM DB must be empty when
                     using this API (can't contain any prefixes or virtual routers).
                Expected: GT_BAD_STATE.
            */

            lpmDBId  = 0;
            pclIdArray[0] = 0;

            st = cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet(lpmDBId,1,pclIdArray);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_STATE, st, "%d", lpmDBId);

        }

        /*
            1.5. Call with lpmDb [4] (not exists).
            Expected: NOT GT_OK.
        */
        lpmDBId  = 6;
        pclIdArray[0] = 0;

        st = cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet(lpmDBId,1,pclIdArray);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d", lpmDBId);

    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet
(
    IN      GT_U32                          lpmDBId,
    OUT     GT_BOOL                         *defaultMcUsedPtr,
    OUT     CPSS_IP_PROTOCOL_STACK_ENT      *protocolStackPtr,
    OUT     GT_U32                          defaultIpv4RuleIndexArray[],
    INOUT   GT_U32                          *defaultIpv4RuleIndexArrayLenPtr,
    OUT     GT_U32                          defaultIpv6RuleIndexArray[],
    INOUT   GT_U32                          *defaultIpv6RuleIndexArrayLenPtr,
    OUT     GT_U32                          pclIdArray[],
    INOUT   GT_U32                          *pclIdArrayLenPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet)
{
/*
    ITERATE_DEVICES (DxChx and above)
    1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0],
         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
         indexesRangePtr{ firstIndex [0], lastIndex [1024]}, partitionEnable[GT_TRUE],
         tcamLpmManagerCapcityCfgPtr{ numOfIpv4Prefixes [10],
         numOfIpv4McSourcePrefixes [5], numOfIpv6Prefixes [10]} to create LPM DB.
    Expected: GT_OK.
    1.2. Call with lpmDBId [0].
    Expected: GT_OK.
    1.3. Call with defaultMcUsedPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with defaultIpv4RuleIndexArray [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call with defaultIpv6RuleIndexArray [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_U8      dev       = 0;
    GT_STATUS  st        = GT_OK;
    GT_U32     lpmDBId   = 0;

    GT_BOOL                     defaultMcUsedPtr;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStackPtr;
    GT_U32                      defaultIpv4RuleIndexArray[1];
    GT_U32                      defaultIpv4RuleIndexArrayLen = 1;
    GT_U32                      defaultIpv6RuleIndexArray[1];
    GT_U32                      defaultIpv6RuleIndexArrayLen = 1;
    GT_U32                      pclIdArray[1];
    GT_U32                      pclIdArrayLen = 1;

    /*
        1.1. Call cpssDxChIpLpmDBCreate with lpmDBId [0 / 1],
                     protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E],
                     indexesRangePtr{ firstIndex [0/ 200],
                                      lastIndex [100/ 1000]},
                     partitionEnable[GT_TRUE],
                     tcamLpmManagerCapcityCfgPtr{numOfIpv4Prefixes [10],
                                                 numOfIpv4McSourcePrefixes [5],
                                                 numOfIpv6Prefixes [10]}
                     to create LPM DB.
        Expected: GT_OK.
    */
        /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfCreateDefaultLpmDB(dev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateDefaultLpmDB");

        /*
            1.2. Call with default  lpmDb configuration [0].
            Expected: GT_OK.
        */

        lpmDBId  = 0;

        st = cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet(lpmDBId, &defaultMcUsedPtr,&protocolStackPtr,
                defaultIpv4RuleIndexArray, &defaultIpv4RuleIndexArrayLen,
                defaultIpv6RuleIndexArray, &defaultIpv6RuleIndexArrayLen,
                pclIdArray, &pclIdArrayLen);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d lpmDbPtr", lpmDBId);

        /*
            1.3. Call with defaultMcUsedPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet(lpmDBId, NULL,&protocolStackPtr,
                defaultIpv4RuleIndexArray, &defaultIpv4RuleIndexArrayLen,
                defaultIpv6RuleIndexArray, &defaultIpv6RuleIndexArrayLen,
                pclIdArray, &pclIdArrayLen);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d", lpmDBId);

        /*
            1.4. Call with defaultIpv4RuleIndexArray [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet(lpmDBId, &defaultMcUsedPtr,&protocolStackPtr,
                NULL, &defaultIpv4RuleIndexArrayLen,
                defaultIpv6RuleIndexArray, &defaultIpv6RuleIndexArrayLen,
                pclIdArray, &pclIdArrayLen);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d", lpmDBId);

        /*
            1.5. Call with defaultIpv6RuleIndexArray [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet(lpmDBId, &defaultMcUsedPtr,&protocolStackPtr,
                defaultIpv4RuleIndexArray, &defaultIpv4RuleIndexArrayLen,
                NULL, &defaultIpv6RuleIndexArrayLen,
                pclIdArray, &pclIdArrayLen);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d", lpmDBId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmVirtualRouterGet
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    OUT GT_BOOL                                 *supportUcPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *defUcNextHopInfoPtr,
    OUT GT_BOOL                                 *supportMcPtr,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC              *defMcRouteLttEntryPtr,
    OUT CPSS_IP_PROTOCOL_STACK_ENT              *protocolStackPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmVirtualRouterGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with lpmDb [0] and vrId.
    Expected: GT_OK.
    1.2. Call with lpmDBId [3] and vrId [1] (support only Uc routing).
    Expected: GT_OK and supportUc [GT_TRUE], supportMc [GT_FALSE].
    1.3. Call with lpmDBId [1] and vrId [1] (support only Mc routing).
    Expected: GT_OK and supportUc [GT_FALSE], supportMc [GT_TRUE].
    1.4. Call with lpmDb [0] and non-existent vrId [2].
    Expected: GT_NOT_FOUND.
    1.5. Call with empty lpmDb [1].
    Expected: GT_NOT_FOUND.
    1.6. Call with wrong &vrConfigInfoGet [NULL].
    Expected: GT_BAD_PTR.

*/
    GT_U8      dev       = 0;
    GT_STATUS  st        = GT_OK;

    GT_U32     lpmDBId   = 0;
    GT_U32     vrId      = 0;

    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfoGet = {0};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd");

        /*
            1.1. Call with lpmDBId [0] and vrId [0].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        st = cpssDxChIpLpmVirtualRouterGet(lpmDBId, vrId, &vrConfigInfoGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d lpmDbPtr", lpmDBId);

        /*
            1.2. Call with lpmDBId [3] and vrId [1] (support only Uc routing).
            Expected: GT_OK and supportUc [GT_TRUE], supportMc [GT_FALSE].
        */
        lpmDBId = prvUtfIsPbrModeUsed() ? 1 : 3;
        vrId    = prvUtfIsPbrModeUsed() ? 0 : 1;

        st = cpssDxChIpLpmVirtualRouterGet(lpmDBId, vrId, &vrConfigInfoGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d lpmDbPtr", lpmDBId);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(vrConfigInfoGet.supportIpv4Uc, GT_TRUE, "lpmDbPtr = %d, vrId = %d",
                                                                            lpmDBId, vrId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vrConfigInfoGet.supportIpv6Uc, GT_TRUE, "lpmDbPtr = %d, vrId = %d",
                                                                            lpmDBId, vrId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vrConfigInfoGet.supportIpv4Mc, GT_FALSE, "lpmDbPtr = %d, vrId = %d",
                                                                            lpmDBId, vrId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vrConfigInfoGet.supportIpv6Mc, GT_FALSE, "lpmDbPtr = %d, vrId = %d",
                                                                            lpmDBId, vrId);
        }

        /*
            1.3. Call with lpmDBId [1] and vrId [1] (support only Mc routing).
            Expected: GT_OK and supportUc [GT_FALSE], supportMc [GT_TRUE].
        */
        /* MC is not supported in PBR mode */
        if (GT_FALSE == prvUtfIsPbrModeUsed())
        {
            lpmDBId = 4;
            vrId    = 1;

            st = cpssDxChIpLpmVirtualRouterGet(lpmDBId, vrId, &vrConfigInfoGet);

            if (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH2_E)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d lpmDbPtr", lpmDBId);
                if (GT_OK == st)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(vrConfigInfoGet.supportIpv4Uc, GT_FALSE, "lpmDbPtr = %d, vrId = %d",
                                                                                    lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(vrConfigInfoGet.supportIpv6Uc, GT_FALSE, "lpmDbPtr = %d, vrId = %d",
                                                                                    lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(vrConfigInfoGet.supportIpv4Mc, GT_TRUE, "lpmDbPtr = %d, vrId = %d",
                                                                                    lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(vrConfigInfoGet.supportIpv6Mc, GT_TRUE, "lpmDbPtr = %d, vrId = %d",
                                                                                    lpmDBId, vrId);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d lpmDbPtr", lpmDBId);
            }
        }

        /*
            1.4. Call with lpmDb [0] and non-existent vrId [2].
            Expected: GT_NOT_FOUND.
        */
        lpmDBId = 0;
        vrId    = 2;

        st = cpssDxChIpLpmVirtualRouterGet(lpmDBId, vrId, &vrConfigInfoGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st, "%d lpmDbPtr", lpmDBId);

        /*
            1.5. Call with empty lpmDb [1].
            Expected: GT_NOT_FOUND.
        */
        lpmDBId = 1;

        st = cpssDxChIpLpmVirtualRouterGet(lpmDBId, vrId, &vrConfigInfoGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st, "%d lpmDbPtr", lpmDBId);



        /*
            1.6. Call with wrong vrConfigInfoGet [NULL].
            Expected: GT_BAD_PTR.
        */
        lpmDBId = 0;

        st = cpssDxChIpLpmVirtualRouterGet(lpmDBId, vrId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d lpmDbPtr", lpmDBId);


        /*
            1.7. Call with wrong  vrId [4096].
            Expected: NOT GT_OK.
        */

        vrId = 4096;
        st = cpssDxChIpLpmVirtualRouterGet(lpmDBId, vrId, &vrConfigInfoGet);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);


    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmVirtualRouterSharedAdd
(
    IN GT_U32                                 lpmDBId,
    IN GT_U32                                 vrId,
    IN CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defUcNextHopInfoPtr,
    IN CPSS_DXCH_IP_LTT_ENTRY_STC             *defMcRouteLttEntryPtr,
    IN CPSS_IP_PROTOCOL_STACK_ENT             protocolStack
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmVirtualRouterSharedAdd)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call cpssDxChIpLpmDBCreate to create default LPM DB configuration.
         See cpssDxChIpLpmDBCreate description.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmDBDevListAdd with lpmDBId [0 / 1],
         devList [0, 1, 2, 3, 4] / [5, 6, 7],
         numOfDevs[5] / [3] to adds devices to an existing LPM DB.
    Expected: GT_OK.
    1.3. Call with lpmDBId [1], vrId[0], defUcNextHopInfo filled with
         default configuration, defMcRouteLttEntry filled with default
         configuration (support both: unicast and multicast) and
         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_NOT_SUPPORTED (multicast not supported currently).
    1.4. Call with lpmDBId [3], vrId[0], defUcNextHopInfo filled with
         default configuration, defMcRouteLttEntry [NULL]
         (support only unicast) and protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK .
        1.5. Call cpssDxChIpLpmVirtualRouterGet with the same params.
        Expected: GT_OK and the same values.
    1.6. Call with lpmDBId [4], vrId[0], defUcNextHopInfo [NULL]
         defMcRouteLttEntry filled with default configuration
         (support only multicast) and
         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_NOT_SUPPORTED (multicast not supported yet) .
    1.7. Call with lpmDBId [1], out of range vrId [10] and other valid parameters.
    Expected: NOT GT_OK.
    1.8. Call with not valid lpmDBId [10].
    Expected: NOT GT_OK.
    1.9. Call with lpmDBId [0], vrId[0], defUcNextHopInfoPtr[NULL],
          defMcRouteLttEntry[NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: NOT GT_OK.
    1.10. Call with correct numOfPaths [5/7] ipv4/ipv6 for unicast and multicast.
    Expected: GT_OK.
    1.11. Call with correct numOfPaths [8/63] ipv4/ipv6 for unicast and multicast.
    Expected: GT_OK for lion and above and not GT_OK for other.
    1.12. Call with wrong numOfPaths [64] (out of range).
    Check ipv4/ipv6 for unicast and multicast.
    Expected: NOT GT_OK
    1.13. Call with wrong dev family.
    Expected: GT_NOT_SUPPORTED (only cheetah3 and above supported).
*/
    GT_STATUS                              st            = GT_OK;
    GT_U8                                  dev, devNum;

    GT_U32                                 lpmDBId       = 0;
    GT_U32                                 vrId          = 0;

    GT_U8                                  devList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                                 numOfDevs = 0;
    GT_BOOL                                isEqual = GT_FALSE;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfoGet;

    cpssOsBzero((GT_VOID*) &vrConfigInfo, sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));
    cpssOsBzero((GT_VOID*) &vrConfigInfoGet, sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                     UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                     UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfDevs = 0;
        /*
            1.1. Call cpssDxChIpLpmDBCreate to create default LPM DB configuration.
                 See cpssDxChIpLpmDBCreate description.
            Expected: GT_OK.
        */
        st = prvUtfCreateDefaultLpmDB(dev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateDefaultLpmDB");

        /*
            1.2. Call cpssDxChIpLpmDBDevListAdd with lpmDBId [0 / 1],
                 devList [0, 1, 2, 3, 4] / [5, 6, 7],
                 numOfDevs[5] / [3] to adds devices to an existing LPM DB.
            Expected: GT_OK.
        */
        lpmDBId = 0;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            devList[numOfDevs++] = devNum;
        }


        /* for lpmDBId = 0 dev 0 was already added in the init phase,
           so we will get GT_ALREADY_EXIST */
        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_ALREADY_EXIST, st,
            "cpssDxChIpLpmDBDevListAdd: %d, numOfDevs = %d", lpmDBId, numOfDevs);

        lpmDBId = 1;

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);

        /* when caling this test alone we get GT_OK, when calling this test as
          part of the package we will get GT_ALREADY_EXIST since it was already
          defined in the tests above */
        if(st==GT_ALREADY_EXIST)
            st=GT_OK;

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChIpLpmDBDevListAdd: %d, numOfDevs = %d", lpmDBId, numOfDevs);

        /* DxCh3 shadow type and above are supported. PBR shadow types are not supported */
        if ((CPSS_PP_FAMILY_CHEETAH3_E <= PRV_CPSS_PP_MAC(dev)->devFamily) &&
            (GT_FALSE == prvUtfIsPbrModeUsed()))
        {
            /*
                1.3. Call with lpmDBId [1], vrId[0], defUcNextHopInfo filled with
                     default configuration, defMcRouteLttEntry filled with default
                     configuration (support both: unicast and multicast) and
                     protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                Expected: GT_NOT_SUPPORTED (multicast not supported currently).
            */
            lpmDBId = 1;
            vrConfigInfo.supportIpv4Mc = GT_TRUE;
            vrConfigInfo.supportIpv6Mc = GT_TRUE;
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportIpv6Uc = GT_TRUE;
            vrConfigInfo.supportFcoe = GT_FALSE;
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry);
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry);
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv4McRouteLttEntry);
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv6McRouteLttEntry);
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defaultFcoeForwardingNextHopInfo.ipLttEntry);

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st,"%d, %d", lpmDBId, vrId);

            /*
                1.4. Call with lpmDBId [3], vrId[0], defUcNextHopInfo filled with
                     default configuration, defMcRouteLttEntry [NULL]
                     (support only unicast) and protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                Expected: GT_OK .
            */
            vrConfigInfo.supportIpv4Mc = GT_FALSE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;
            lpmDBId = 3;

            /*remove any virtual routers for prevent GT_BAD_PARAM by prefixes conflict
              when run current LPM suit*/
            for(vrId = 0; vrId < 100; vrId++)
            {
                st = cpssDxChIpLpmVirtualRouterGet(lpmDBId, vrId, &vrConfigInfoGet);
                if (GT_OK == st)
                {
                    st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d", lpmDBId, vrId);
                }
            }

            vrId = 2;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%d, %d", lpmDBId, vrId);

            if (GT_OK == st)
            {
                /*
                    1.5. Call cpssDxChIpLpmVirtualRouterGet with the same params.
                    Expected: GT_OK and the same values.
                */
                st = cpssDxChIpLpmVirtualRouterGet(lpmDBId, vrId, &vrConfigInfoGet);

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d", lpmDBId, vrId);

                /*check supportIpv4Uc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, vrConfigInfoGet.supportIpv4Uc,
                                    "SupportIpv4Uc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv4Uc);

                /*check defIpv4UcNextHopInfoGet.ipLttEntry*/
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry,
                                             (GT_VOID*)&vrConfigInfoGet.defIpv4UcNextHopInfo.ipLttEntry,
                           sizeof (CPSS_DXCH_IP_LTT_ENTRY_STC))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                        "get another defIpv4UcNextHopInfo than was set: %d", dev);

                /*check supportIpv6Uc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, vrConfigInfoGet.supportIpv6Uc,
                                    "SupportIpv6Uc don't equal GT_TRUE, %d", vrConfigInfoGet.supportIpv6Uc);

                /*check defIpv6UcNextHopInfoGet.ipLttEntry*/
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry,
                                             (GT_VOID*)&vrConfigInfoGet.defIpv6UcNextHopInfo.ipLttEntry,
                           sizeof (CPSS_DXCH_IP_LTT_ENTRY_STC))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                        "get another defIpv6UcNextHopInfo than was set: %d", dev);


                /*check supportIpv4Mc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, vrConfigInfoGet.supportIpv4Mc,
                                    "SupportIpv4Mc don't equal GT_FALSE, %d", vrConfigInfoGet.supportIpv4Mc);
                /*check supportIpv6Mc*/
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, vrConfigInfoGet.supportIpv6Mc,
                                    "SupportIpv6Mc don't equal GT_FALSE, %d", vrConfigInfoGet.supportIpv6Mc);

            }

            /*
                1.6. Call with lpmDBId [4], vrId[0], defUcNextHopInfo [NULL]
                     defMcRouteLttEntry filled with default configuration
                     (support only multicast) and
                     protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                Expected: GT_NOT_SUPPORTED (multicast not supported yet) .
            */
            vrConfigInfo.supportIpv4Mc = GT_TRUE;
            vrConfigInfo.supportIpv6Mc = GT_TRUE;
            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportIpv6Uc = GT_FALSE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            lpmDBId = 4;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st,"%d, %d", lpmDBId, vrId);

            vrConfigInfo.supportIpv4Mc = GT_FALSE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;

            /*
                1.7. Call with lpmDBId [1], out of range vrId [10] and other valid parameters.
                Expected: NOT GT_OK.
            */
            lpmDBId = 1;
            vrId = 10;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

            vrId = 0;

            /*
                1.8. Call with not valid lpmDBId [10].
                Expected: NOT GT_OK.
            */
            lpmDBId = 10;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

            lpmDBId = 0;

            /*
                1.9. Call with lpmDBId [0], vrId[0], defUcNextHopInfoPtr[NULL],
                      defMcRouteLttEntry[NULL], protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E].
                Expected: NOT GT_OK.
            */
            vrConfigInfo.supportIpv4Mc = GT_FALSE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;
            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportIpv6Uc = GT_FALSE;
            vrConfigInfo.supportFcoe   = GT_FALSE;
            lpmDBId = 0;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
             "%d, %d, supportIpv4_Ipv6Uc = GT_FALSE, supportIpv4_Ipv6Mc = GT_FALSE", lpmDBId, vrId);


            /*
                1.10. Call with correct numOfPaths [5/7] ipv4/ipv6 for unicast and multicast.
                Expected: GT_OK.
            */
            lpmDBId = 1;

            /*remove any virtual routers for prevent GT_BAD_PARAM by prefixes conflict
              when run current LPM suit*/
            for(vrId = 0; vrId < 100; vrId++)
            {
                st = cpssDxChIpLpmVirtualRouterGet(lpmDBId, vrId, &vrConfigInfoGet);
                if (GT_OK == st)
                {
                    st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d", lpmDBId, vrId);
                }
            }

            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportIpv4Mc = GT_FALSE;
            vrConfigInfo.supportIpv6Uc = GT_TRUE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            /* call with numOfPaths [5] for ipv4 Uc */
            vrConfigInfo.supportIpv4Uc = GT_TRUE;

            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 5;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                   lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

            /* restore configuration */
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            if (GT_OK == st)
            {
                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
            }

            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportFcoe = GT_FALSE;

            /* call with numOfPaths [7] for ipv4 Uc */
            vrConfigInfo.supportIpv4Uc = GT_TRUE;

            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 7;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                   lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

            /* restore configuration */
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;

            if (GT_OK == st)
            {
                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
            }

            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportFcoe = GT_FALSE;

            /* call with numOfPaths [5] for ipv6 Uc */
            vrConfigInfo.supportIpv6Uc = GT_TRUE;

            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 5;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "%d, %d, defIpv6UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                   lpmDBId, vrId, vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths);

            /* restore configuration */
            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;

            if (GT_OK == st)
            {
                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
            }

            vrConfigInfo.supportIpv6Uc = GT_FALSE;

            /* call with numOfPaths [7] for ipv4 Uc */
            vrConfigInfo.supportIpv6Uc = GT_TRUE;

            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 7;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "%d, %d, defIpv6UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                   lpmDBId, vrId, vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths);

            /* restore configuration */
            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;

            if (GT_OK == st)
            {
                st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
            }

            vrConfigInfo.supportIpv6Uc = GT_FALSE;

            /*
                1.11. Call with correct numOfPaths [8/63] ipv4/ipv6 for unicast and multicast.
                Expected: GT_OK for lion and above and not GT_OK for other.
            */
            /* call with numOfPaths [8] for ipv4 Uc */
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportFcoe = GT_FALSE;

            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 8;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }

            /* restore configuration */
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);

            /* GT_NOT_FOUND is possible result for not LION family -  in case virtual
               router was not really added before */
            if(st != GT_NOT_FOUND)
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportFcoe = GT_FALSE;

            /* call with numOfPaths [63] for ipv4 Uc */
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportFcoe = GT_FALSE;

            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 63;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }

            /* restore configuration */
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);

            /* GT_NOT_FOUND is possible result for not LION family -  in case virtual
               router was not really added before */
            if(st != GT_NOT_FOUND)
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportFcoe = GT_FALSE;

            /* call with numOfPaths [8] for ipv6 Uc */
            vrConfigInfo.supportIpv6Uc = GT_TRUE;

            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 8;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }

            /* restore configuration */
            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
            /* GT_NOT_FOUND is possible result for not LION family -  in case virtual
               router was not really added before */
            if(st != GT_NOT_FOUND)
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

            vrConfigInfo.supportIpv6Uc = GT_FALSE;
            /* call with numOfPaths [63] for ipv4 Uc */
            vrConfigInfo.supportIpv6Uc = GT_TRUE;

            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 63;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            if (CPSS_PP_FAMILY_DXCH_LION_E <= PRV_CPSS_PP_MAC(dev)->devFamily)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                       lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);
            }

            /* restore configuration */
            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
            /* GT_NOT_FOUND is possible result for not LION family -  in case virtual
               router was not really added before */
            if(st != GT_NOT_FOUND)
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);
            vrConfigInfo.supportIpv6Uc = GT_FALSE;

            /*
                1.12. Call with wrong numOfPaths [64] (out of range).
                Check ipv4/ipv6 for unicast and multicast.
                Expected: NOT GT_OK
            */
            /* call with numOfPaths [64] for ipv4 Uc */
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportFcoe = GT_FALSE;

            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 64;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                   "%d, %d, defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                   lpmDBId, vrId, vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths);

            /* restore configuration */
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportFcoe = GT_FALSE;

            /* call with numOfPaths [64] for ipv6 Uc */
            vrConfigInfo.supportIpv6Uc = GT_TRUE;

            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 64;

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                   "%d, %d, defIpv6UcNextHopInfoPtr->ipLttEntry.numOfPaths = %d",
                   lpmDBId, vrId, vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths);

            /* restore configuration */
            vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfigInfo.supportIpv6Uc = GT_FALSE;

        }
        else
        {
            /*
                1.13. Call with wrong dev family.
                Expected: GT_NOT_SUPPORTED (only cheetah3 and above supported).
            */
            vrConfigInfo.supportIpv4Mc = GT_FALSE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportIpv6Uc = GT_TRUE;
            vrConfigInfo.supportFcoe = GT_FALSE;

            lpmDBId = 1;
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry);
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry);

            st = cpssDxChIpLpmVirtualRouterSharedAdd(lpmDBId, vrId, &vrConfigInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st,"%d, %d", lpmDBId, vrId);
        }

        /* return to default case were no devices are added to lpmDbId*/
        lpmDBId = 1;

        st = cpssDxChIpLpmDBDevsListRemove(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId, numOfDevs);


    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmDBCapacityGet
(
    IN GT_U32                                 lpmDBId,
    OUT GT_BOOL                                         *partitionEnablePtr,
    OUT CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    *indexesRangePtr,
    OUT CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    *tcamLpmManagerCapcityCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDBCapacityGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call cpssDxChIpLpmDBCreate to create default LPM DB configuration.
         See cpssDxChIpLpmDBCreate description.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmDBCapacityGet with lpmDBId [1].
    Expected: GT_OK.
    1.3. Call cpssDxChIpLpmDBCapacityGet with lpmDBId [1] and
        wrong partitionEnablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call cpssDxChIpLpmDBCapacityGet with lpmDBId [1] and
        wrong indexesRangePtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call cpssDxChIpLpmDBCapacityGet with lpmDBId [1] and
        wrong tcamLpmManagerCapcityCfgPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call with not valid LPM DB id lpmDBId [10].
    Expected: NOT GT_OK.
*/
    GT_STATUS  st  = GT_OK;
    GT_U8      dev;

    GT_U32     lpmDBId = 0;
    GT_BOOL                                      partitionEnable;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC tcamLpmManagerCapcityCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                     UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                     UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChIpLpmDBCreate to create default LPM DB configuration.
                 See cpssDxChIpLpmDBCreate description.
            Expected: GT_OK.
        */
        st = prvUtfCreateDefaultLpmDB(dev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateDefaultLpmDB");

        /*
            1.2. Call cpssDxChIpLpmDBCapacityGet with lpmDBId [0].
            Expected: GT_OK.
        */
        st = cpssDxChIpLpmDBCapacityGet(lpmDBId, &partitionEnable,
                                        &indexesRange, &tcamLpmManagerCapcityCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        /*
            1.3. Call cpssDxChIpLpmDBCapacityGet with lpmDBId [0] and
                wrong partitionEnablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmDBCapacityGet(lpmDBId, NULL,
                                        &indexesRange, &tcamLpmManagerCapcityCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, partitionEnablePtr = NULL", lpmDBId);

        /*
            1.4. Call cpssDxChIpLpmDBCapacityGet with lpmDBId [0] and
                wrong indexesRangePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmDBCapacityGet(lpmDBId, &partitionEnable,
                                        NULL, &tcamLpmManagerCapcityCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, indexesRangePtr = NULL", lpmDBId);

        /*
            1.5. Call cpssDxChIpLpmDBCapacityGet with lpmDBId [0] and
                wrong tcamLpmManagerCapcityCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmDBCapacityGet(lpmDBId, &partitionEnable, &indexesRange, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, tcamLpmManagerCapcityCfgPtr = NULL", lpmDBId);

        /*
            1.6. Call with not valid LPM DB id lpmDBId [10].
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmDBCapacityGet(lpmDBId, &partitionEnable,
                                        &indexesRange, &tcamLpmManagerCapcityCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;
    }
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmVirtualRouterAdd
(
    IN GT_U32                                 lpmDBId,
    IN GT_U32                                 vrId,
    IN  CPSS_DXCH_IP_LPM_VR_CONFIG_STC        *vrConfigPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmVirtualRouterAddSupportIpv4McOnly)
{
/*
    1.1. Call cpssDxChIpLpmDBCreate with lpmDbId [1]
                                         shadowType
                                         protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E]
                                         memoryCfgPtr[non NULL]
    Expected: GT_OK.
    1.2. Call with lpmDbId [1],
           vrId [0]
           and vrConfigPtr {supportUcIpv4 [GT_FALSE],
                            defaultUcIpv4RouteEntry {routeEntryBaseIndex [0],
                                                     numOfPaths [1],
                                                     routeType [CPSS_DXCH_IP_ECMP_ROUTE_ENTRY_GROUP_E],
                                                     ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
                            supportMcIpv4 [GT_TRUE],
                            defaultMcIpv4RouteEntry {routeEntryBaseIndex [1],
                                                     numOfPaths [1],
                                                     routeType [CPSS_DXCH_IP_ECMP_ROUTE_ENTRY_GROUP_E],
                                                     ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
                            supportUcIpv6 [GT_FALSE],
                            defaultUcIpv6RouteEntry {routeEntryBaseIndex [2],
                                                     numOfPaths [1],
                                                     routeType [CPSS_DXCH_IP_ECMP_ROUTE_ENTRY_GROUP_E],
                                                     ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
                            supportMcIpv6 [GT_FALSE],
                            defaultMcIpv6RouteEntry {routeEntryBaseIndex [3],
                                                     numOfPaths [1],
                                                     routeType [CPSS_DXCH_IP_ECMP_ROUTE_ENTRY_GROUP_E],
                                                     ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}

    Expected: GT_OK.

    1.3. Call with lpmDbId [1],
                   numOfDevs [numOfDevs]
                   and devListArray [dev1, dev2 ... devN].
    Expected: GT_OK.
    1.4. Call cpssDxChIpLpmDbDevListGet with the same numOfDevsPtr
                                               and non-NULL devListArray.
    Expected: GT_OK and the same numOfDevs and devListArray.


    1.5. Call cpssDxChIpLpmVirtualRouterGet with the same lpmDbId,
                                                         vrId and non-NULL vrConfigPtr.
    Expected: GT_OK and the same vrConfigPtr.

    1.6. Call with ipGroup (224.1.1.1)
                   ipGroupPrefixLen[32]
                   ipSrc.arIP [10.17.0.1 / 10.17.0.2],
                   ipSrcPrefixLen [32],
                   mcRouteLttEntryPtr {routeEntryBaseIndex [0],
                                      numOfPaths [1],
                                      routeType [CPSS_DXCH_IP_ECMP_ROUTE_ENTRY_GROUP_E]},
                   override [GT_FALSE].
    Expected: GT_OK.
    1.7. Call cpssDxChIpLpmIpv4McEntrySearch with the same lpmDbId,
                                                 vrId,
                                                 ipGroup,
                                                 ipGroupPrefixLen
                                                 ipSrc,
                                                 ipSrcPrefixLen
                                                 and non-NULL mcRouteLttEntryPtr.
    Expected: GT_OK and the same mcRouteLttEntryPtr.

    1.8. Call cpssDxChIpLpmIpv4McEntriesFlush with lpmDbId [1],
                                                     vrId [0] to invalidate changes.
    Expected: GT_OK.
    1.9. Call cpssDxChIpLpmDBDevsListRemove with lpmDbId [1],
                                                  numOfDevs [numOfDevs]
                                                  and devListArray [dev1, dev2 .. devN] to remove added devices.
    1.10. Call cpssDxChIpLpmVirtualRouterDel with lpmDbId [1]
                                                  and vrId [0] to delete virtual routers.
    Expected: GT_OK.
*/
    GT_STATUS   st      = GT_OK;
    GT_U32      lpmDbId = IP_LPM_DEFAULT_DB_ID_CNS;
    GT_U32      vrId    = IP_LPM_DEFAULT_VR_ID_CNS;

    CPSS_DXCH_IP_LPM_VR_CONFIG_STC    vrConfig;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC    vrConfigGet;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC    vrConfigDef;
    GT_BOOL                           restoreVrConfig = GT_FALSE;

    GT_U8       dev, devNum;
    GT_U32      numOfDevs    = 0;
    GT_U8       devListArray[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32      numOfDevsGet = 0;
    GT_U8       devListArrayGet[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_BOOL     isEqual      = GT_FALSE;

    GT_IPADDR                               ipGroup;
    GT_IPADDR                               ipSrc;
    GT_U32                                  ipSrcPrefixLen = 0;
    GT_U32                                  ipGroupPrefixLen = 0;
    CPSS_DXCH_IP_LTT_ENTRY_STC              mcRouteLttEntry;
    CPSS_DXCH_IP_LTT_ENTRY_STC              mcRouteLttEntryGet;
    GT_BOOL                                 override       = GT_FALSE;
    GT_BOOL                                 defragmentationEnable  = GT_FALSE;
    GT_U32                                  tcamGroupRowIndex = 0;
    GT_U32                                  tcamGroupColumnIndex = 0;
    GT_U32                                  tcamSrcRowIndex = 0;
    GT_U32                                  tcamSrcColumnIndex;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT       shadowType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&vrConfig, sizeof(vrConfig));
    cpssOsBzero((GT_VOID*)&vrConfigGet, sizeof(vrConfigGet));
    cpssOsBzero((GT_VOID*)&vrConfigDef, sizeof(vrConfigDef));
    cpssOsBzero((GT_VOID*)&mcRouteLttEntry, sizeof(mcRouteLttEntry));
    cpssOsBzero((GT_VOID*)&mcRouteLttEntryGet, sizeof(mcRouteLttEntryGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfDevs = 0;

        st = prvUtfGetDevShadow(dev, &shadowType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        /*
        1.1. Call cpssDxChIpLpmDBCreate with lpmDbId [1]
                                             shadowType
                                             protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E]
                                             memoryCfgPtr[non NULL]
        Expected: GT_OK.
       */
        st = prvUtfCreateDefaultLpmDB(dev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateDefaultLpmDB");

        /*
           1.2. Call with lpmDbId [1],
                       vrId [0]
                       and vrConfigPtr {supportUcIpv4 [GT_FALSE],
                                        defaultUcIpv4RouteEntry {routeEntryBaseIndex [0],
                                                                 numOfPaths [1],
                                                                 routeType [CPSS_DXCH_IP_ECMP_ROUTE_ENTRY_GROUP_E],
                                                                 ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
                                        supportMcIpv4 [GT_TRUE],
                                        defaultMcIpv4RouteEntry {routeEntryBaseIndex [1],
                                                                 numOfPaths [1],
                                                                 routeType [CPSS_DXCH_IP_ECMP_ROUTE_ENTRY_GROUP_E],
                                                                 ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
                                        supportUcIpv6 [GT_FALSE],
                                        defaultUcIpv6RouteEntry {routeEntryBaseIndex [2],
                                                                 numOfPaths [1],
                                                                 routeType [CPSS_DXCH_IP_ECMP_ROUTE_ENTRY_GROUP_E],
                                                                 ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
                                        supportMcIpv6 [GT_FALSE],
                                        defaultMcIpv6RouteEntry {routeEntryBaseIndex [3],
                                                                 numOfPaths [1],
                                                                 routeType [CPSS_DXCH_IP_ECMP_ROUTE_ENTRY_GROUP_E],
                                                                 ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
          Expected: GT_OK.
        */

        /* Call with lpmDbId [1] */
        lpmDbId = IP_LPM_DEFAULT_DB_ID_CNS;
        vrId    = IP_LPM_DEFAULT_VR_ID_CNS;

        vrConfig.supportIpv4Uc = GT_FALSE;
        vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
        vrConfig.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 1;
        vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        vrConfig.defIpv4UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;

        vrConfig.supportIpv4Mc = GT_TRUE;
        vrConfig.defIpv4McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        vrConfig.defIpv4McRouteLttEntry.routeEntryBaseIndex = 1;
        vrConfig.defIpv4McRouteLttEntry.numOfPaths = 1;
        vrConfig.defIpv4McRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        vrConfig.defIpv4McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        vrConfig.defIpv4McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;

        vrConfig.supportIpv6Uc = GT_FALSE;
        vrConfig.defIpv6UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        vrConfig.defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 2;
        vrConfig.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 1;
        vrConfig.defIpv6UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        vrConfig.defIpv6UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        vrConfig.defIpv6UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;

        vrConfig.supportIpv6Mc = GT_FALSE;
        vrConfig.defIpv6McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        vrConfig.defIpv6McRouteLttEntry.routeEntryBaseIndex = 3;
        vrConfig.defIpv6McRouteLttEntry.numOfPaths = 1;
        vrConfig.defIpv6McRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        vrConfig.defIpv6McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        vrConfig.defIpv6McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;

        vrConfig.supportFcoe = GT_FALSE;
        vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
        vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.numOfPaths = 1;
        vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;

        st = cpssDxChIpLpmVirtualRouterAdd(lpmDbId, vrId, &vrConfig);

        if(st==GT_ALREADY_EXIST)
        {
            st = GT_OK;
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDbId, vrId);

            restoreVrConfig = GT_TRUE;

            st = cpssDxChIpLpmVirtualRouterGet(lpmDbId, vrId, &vrConfigDef);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDbId, vrId);

            st = cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDbId, vrId);

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDbId, vrId, &vrConfig);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDbId, vrId);


        }

        /* new VR chcek get values same as set */
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDbId, vrId);

        /*
            1.3. Call cpssDxChIpLpmVirtualRouterGet with the same lpmDbId,
                                                                 vrId and non-NULL vrConfigPtr.
            Expected: GT_OK and the same vrConfigPtr.
        */
        st = cpssDxChIpLpmVirtualRouterGet(lpmDbId, vrId, &vrConfigGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChIpLpmVirtualRouterGet: %d, %d", lpmDbId, vrId);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(vrConfig.supportIpv4Uc, vrConfigGet.supportIpv4Uc,
                   "get another vrConfigPtr->supportIpv4Uc than was set: %d", lpmDbId);

        UTF_VERIFY_EQUAL1_STRING_MAC(vrConfig.supportIpv4Mc, vrConfigGet.supportIpv4Mc,
                   "get another vrConfigPtr->supportIpv4Mc than was set: %d", lpmDbId);

        UTF_VERIFY_EQUAL1_STRING_MAC(vrConfig.defIpv4McRouteLttEntry.routeEntryBaseIndex,
                                     vrConfigGet.defIpv4McRouteLttEntry.routeEntryBaseIndex,
                   "get another vrConfigPtr->defIpv4McRouteLttEntry.routeEntryBaseIndex than was set: %d", lpmDbId);
        UTF_VERIFY_EQUAL1_STRING_MAC(vrConfig.defIpv4McRouteLttEntry.numOfPaths,
                                     vrConfigGet.defIpv4McRouteLttEntry.numOfPaths,
                   "get another vrConfigPtr->defIpv4McRouteLttEntry.numOfPaths than was set: %d", lpmDbId);
        UTF_VERIFY_EQUAL1_STRING_MAC(vrConfig.defIpv4McRouteLttEntry.routeType,
                                     vrConfigGet.defIpv4McRouteLttEntry.routeType,
                   "get another vrConfig.defIpv4McRouteLttEntry.routeType than was set: %d", lpmDbId);

        UTF_VERIFY_EQUAL1_STRING_MAC(vrConfig.supportIpv6Uc, vrConfigGet.supportIpv6Uc,
                   "get another vrConfigPtr->supportIpv6Uc than was set: %d", lpmDbId);

        UTF_VERIFY_EQUAL1_STRING_MAC(vrConfig.supportIpv6Mc, vrConfigGet.supportIpv6Mc,
                   "get another vrConfigPtr->supportIpv6Mc than was set: %d", lpmDbId);


        /*
            1.4. Call with lpmDbId [1],
                           numOfDevs [numOfDevs]
                           and devListArray [dev1, dev2 .. devN].
            Expected: GT_OK.
        */

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

       /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            devListArray[numOfDevs++] = devNum;
        }

        st = cpssDxChIpLpmDBDevListAdd(lpmDbId, devListArray, numOfDevs);
        if(st==GT_ALREADY_EXIST)
            st = GT_OK;
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDbId, numOfDevs);

        /*
            1.5. Call cpssDxChIpLpmDbDevListGet with the same numOfDevsPtr
                                                       and non-NULL devListArray.
            Expected: GT_OK and the same numOfDevs and devListArray.
        */
        numOfDevsGet = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssDxChIpLpmDBDevListGet(lpmDbId, &numOfDevsGet, devListArrayGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChIpLpmDbDevListGet: %d, %d", lpmDbId, numOfDevsGet);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfDevs, numOfDevsGet,
                   "get another numOfDevs than was set: %d", lpmDbId);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) devListArray,
                                     (GT_VOID*) devListArrayGet,
                                     sizeof(devListArray[0]) * numOfDevs)) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                   "get another devListArray[] than was set: %d", lpmDbId);


        /*
            1.6. Call with ipGroup (224.1.1.1)
                           ipGroupPrefixLen[32]
                           ipSrc.arIP [10.17.0.1 / 10.17.0.2],
                           ipSrcPrefixLen [32],
                           mcRouteLttEntryPtr {routeEntryBaseIndex [0],
                                              numOfPaths [1],
                                              routeType [CPSS_DXCH_IP_ECMP_ROUTE_ENTRY_GROUP_E]},
                           override [GT_FALSE].
            Expected: GT_OK.
        */
        /* Call with ipSrc.arIP [10.17.0.1] */
        ipGroup.arIP[0] = 224;
        ipGroup.arIP[1] = 1;
        ipGroup.arIP[2] = 1;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 32;

        ipSrc.arIP[0] = 10;
        ipSrc.arIP[1] = 17;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        mcRouteLttEntry.routeType                = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        mcRouteLttEntry.numOfPaths               = 1;
        mcRouteLttEntry.routeEntryBaseIndex      = 0;
        mcRouteLttEntry.ucRPFCheckEnable         = GT_FALSE;
        mcRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        mcRouteLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

        override = GT_FALSE;
        defragmentationEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDbId, vrId, &ipGroup, ipGroupPrefixLen,
                                         &ipSrc, ipSrcPrefixLen, &mcRouteLttEntry,
                                         override, defragmentationEnable);

        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"%d, %d, ipSrcPrefixLen = %d, override = %d",
                                     lpmDbId, vrId, ipSrcPrefixLen, override);
        /*
            1.7. Call cpssDxChIpLpmIpv4McEntrySearch with the same lpmDbId,
                                                         vrId,
                                                         ipGroup,
                                                         ipGroupPrefixLen,
                                                         ipSrc,
                                                         ipSrcPrefixLen
                                                         and non-NULL mcRouteLttEntryPtr.
            Expected: GT_OK and the same mcRouteLttEntryPtr.
        */
        st = cpssDxChIpLpmIpv4McEntrySearch(lpmDbId, vrId, &ipGroup,ipGroupPrefixLen,
                                           &ipSrc, ipSrcPrefixLen, &mcRouteLttEntryGet,
                                           &tcamGroupRowIndex,&tcamGroupColumnIndex,
                                           &tcamSrcRowIndex, &tcamSrcColumnIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "cpssDxChIpLpmIpv4McEntrySearch: %d, %d, ipSrcPrefixLen = %d", lpmDbId, vrId, ipSrcPrefixLen);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(mcRouteLttEntry.routeType, mcRouteLttEntryGet.routeType,
                   "get another mcRouteLttEntryPtr->routeType than was set: %d, %d", lpmDbId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(mcRouteLttEntry.numOfPaths, mcRouteLttEntryGet.numOfPaths,
                   "get another mcRouteLttEntryPtr->numOfPaths than was set: %d, %d", lpmDbId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(mcRouteLttEntry.routeEntryBaseIndex, mcRouteLttEntryGet.routeEntryBaseIndex,
                   "get another mcRouteLttEntryPtr->routeEntryBaseIndex than was set: %d, %d", lpmDbId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(mcRouteLttEntry.ucRPFCheckEnable, mcRouteLttEntryGet.ucRPFCheckEnable,
                   "get another mcRouteLttEntryPtr->ucRPFCheckEnable than was set: %d, %d", lpmDbId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(mcRouteLttEntry.sipSaCheckMismatchEnable, mcRouteLttEntryGet.sipSaCheckMismatchEnable,
                   "get another mcRouteLttEntryPtr->sipSaCheckMismatchEnable than was set: %d, %d", lpmDbId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(mcRouteLttEntry.ipv6MCGroupScopeLevel, mcRouteLttEntryGet.ipv6MCGroupScopeLevel,
                   "get another mcRouteLttEntryPtr->ipv6MCGroupScopeLevel than was set: %d, %d", lpmDbId, vrId);

        /* restore VR */
        if(restoreVrConfig==GT_TRUE)
        {
            st = cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDbId, vrId);

            st = cpssDxChIpLpmVirtualRouterAdd(lpmDbId, vrId, &vrConfigDef);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDbId, vrId);
        }

        /*
            1.8. Call cpssDxChIpLpmIpv4McEntriesFlush with lpmDbId [1],
                                                             vrId [0] to invalidate changes.
            Expected: GT_OK.
        */
        /* Delete Prefixes */
        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDbId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpLpmIpv4McEntriesFlush: %d, %d", lpmDbId, vrId);


         /*
            1.9. Call cpssDxChIpLpmDBDevsListRemove with lpmDbId [1],
                                                          numOfDevs [numOfDevs]
                                                          and devListArray [dev1, dev2 � devN] to remove added diveces.
            Expected: GT_OK.
        */
        lpmDbId = 1;

        st = cpssDxChIpLpmDBDevsListRemove(lpmDbId, devListArray, numOfDevs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChIpLpmDBDevsListRemove: %d, %d", lpmDbId, numOfDevs);

        /*
            1.10. Call cpssDxChIpLpmVirtualRouterDel with lpmDbId [1]
                                                          and vrId [0] to delete virtual routers.
            Expected: GT_OK.
        */

        /* Call with lpmDbId [1] and vrId [0] */
        lpmDbId = 1;
        vrId    = 0;

        st = cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"cpssDxChIpLpmVirtualRouterDel: %d, %d", lpmDbId, vrId);

        st = cpssDxChIpLpmDBDelete(lpmDbId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDBDelete: %d", lpmDbId);
    }
}
/*--------------------------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDbgHwValidation)
{
/*
    ITERATE_DEVICES (DXCH)
    1.1. Call with vrId [0/1]
                   isIpv4 [GT_FALSE and GT_TRUE],
                   isUnicast [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call with out of range vrId[77777].
    Expected: not GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                                  vrId = IP_LPM_DEFAULT_VR_ID_CNS;
    CPSS_IP_PROTOCOL_STACK_ENT              protocol = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_UNICAST_MULTICAST_ENT              prefixType = CPSS_UNICAST_E;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT       shadowType;


   /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvUtfGetDevShadow(dev, &shadowType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);
        if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            continue;
        }

         /* Delete defauld LpmDB and Virtual Routers */
        st = prvUtfDeleteLpmDBWithDeviceAndVirtualRouter();
        if(st == GT_NOT_FOUND)
            st=GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfDeleteLpmDBWithDeviceAndVirtualRouter");


        st = prvUtfCreateLpmDBWithDeviceAndVirtualRouterAdd();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd");


        /*  1.1.  */
        vrId = IP_LPM_DEFAULT_VR_ID_CNS+1;
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_UNICAST_E;

        st = cpssDxChIpLpmDbgHwValidation(dev, vrId, protocol, prefixType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vrId = IP_LPM_DEFAULT_VR_ID_CNS+1;
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_MULTICAST_E;

        st = cpssDxChIpLpmDbgHwValidation(dev, vrId, protocol, prefixType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        vrId = IP_LPM_DEFAULT_VR_ID_CNS;
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_UNICAST_E;

        st = cpssDxChIpLpmDbgHwValidation(dev, vrId, protocol, prefixType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vrId = IP_LPM_DEFAULT_VR_ID_CNS;
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_MULTICAST_E;

        st = cpssDxChIpLpmDbgHwValidation(dev, vrId, protocol, prefixType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.3.  */
        vrId = IP_LPM_DEFAULT_VR_ID_CNS;
        protocol = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_UNICAST_E;

        st = cpssDxChIpLpmDbgHwValidation(dev, vrId, protocol, prefixType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vrId = IP_LPM_DEFAULT_VR_ID_CNS;
        protocol = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_MULTICAST_E;

        st = cpssDxChIpLpmDbgHwValidation(dev, vrId, protocol, prefixType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  restore  */
        vrId = IP_LPM_DEFAULT_VR_ID_CNS;
        protocol = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_MULTICAST_E;

        /* Delete defauld LpmDB and Virtual Routers */
        st = prvUtfDeleteLpmDBWithDeviceAndVirtualRouter();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfDeleteLpmDBWithDeviceAndVirtualRouter");
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpLpmDbgHwValidation(dev, vrId, protocol, prefixType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpLpmDbgHwValidation(dev, vrId, protocol, prefixType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*--------------------------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDbgShadowValidityCheck)
{
/*
    ITERATE_DEVICES (ExMxPm)
    1.1. Call with lpmDbId[1],
                   vrId [0/1],
                   protocol [CPSS_IP_PROTOCOL_IPV4_E/
                             CPSS_IP_PROTOCOL_IPV6_E/
                             CPSS_IP_PROTOCOL_IPV4V6_E],
                   prefixType [CPSS_UNICAST_E/
                               CPSS_MULTICAST_E/
                               CPSS_UNICAST_MULTICAST_E],
                   returnOnFailure [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call with out of range vrId[77777].
    Expected: not GT_OK.
    1.3. Call with out of range lpmDbId[55555].
    Expected: not GT_OK.
    1.4. Call with wrong enum value protocol.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong enum value prefixType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                   st              = GT_OK;
    GT_U32                      lpmDbId         = IP_LPM_DEFAULT_DB_ID_CNS;
    GT_U32                      vrId            = IP_LPM_DEFAULT_VR_ID_CNS;
    CPSS_IP_PROTOCOL_STACK_ENT  protocol        = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_UNICAST_MULTICAST_ENT  prefixType      = CPSS_UNICAST_E;
    GT_BOOL                     returnOnFailure = GT_FALSE;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT       shadowType;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvUtfGetDevShadow(dev, &shadowType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);
        if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            continue;
        }

        /* Delete defauld LpmDB and Virtual Routers */
        st = prvUtfDeleteLpmDBWithDeviceAndVirtualRouter();
        if(st == GT_NOT_FOUND)
            st=GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfDeleteLpmDBWithDeviceAndVirtualRouter");

        st = prvUtfCreateLpmDBWithDeviceAndVirtualRouterAdd();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd");

        /*  1.1.  */
        lpmDbId             = 0;
        vrId                = IP_LPM_DEFAULT_VR_ID_CNS;
        protocol            = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType          = CPSS_UNICAST_E;
        returnOnFailure     = GT_FALSE;

        st = cpssDxChIpLpmDbgShadowValidityCheck(lpmDbId, vrId, protocol, prefixType, returnOnFailure);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*  1.1.  */
        vrId = 0;
        protocol = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_UNICAST_E;
        returnOnFailure = GT_TRUE;

        st = cpssDxChIpLpmDbgShadowValidityCheck(lpmDbId, vrId, protocol, prefixType, returnOnFailure);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*  1.1.  */
        vrId = 1;
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_UNICAST_MULTICAST_E;
        returnOnFailure = GT_FALSE;

        st = cpssDxChIpLpmDbgShadowValidityCheck(lpmDbId, vrId, protocol, prefixType, returnOnFailure);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*  1.1.  */
        vrId = 0;
        protocol = CPSS_IP_PROTOCOL_IPV4V6_E;
        prefixType = CPSS_UNICAST_MULTICAST_E;
        returnOnFailure = GT_TRUE;

        st = cpssDxChIpLpmDbgShadowValidityCheck(lpmDbId, vrId, protocol, prefixType, returnOnFailure);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*  1.1.  */
        vrId = 1;
        protocol = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_UNICAST_MULTICAST_E;
        returnOnFailure = GT_FALSE;

        st = cpssDxChIpLpmDbgShadowValidityCheck(lpmDbId, vrId, protocol, prefixType, returnOnFailure);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*  1.2.  */
        vrId = IP_LPM_INVALID_VR_ID_CNS;

        st = cpssDxChIpLpmDbgShadowValidityCheck(lpmDbId, vrId, protocol, prefixType, returnOnFailure);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
        vrId = 0;


        /*  1.3.  */
        lpmDbId = IP_LPM_INVALID_DB_ID_CNS;

        st = cpssDxChIpLpmDbgShadowValidityCheck(lpmDbId, vrId, protocol, prefixType, returnOnFailure);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
        vrId = 0;

        lpmDbId = IP_LPM_DEFAULT_DB_ID_CNS;

        /*  1.4.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmDbgShadowValidityCheck(lpmDbId, vrId, protocol, prefixType, returnOnFailure),
                          protocol);

        /*  1.5.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmDbgShadowValidityCheck(lpmDbId, vrId, protocol, prefixType, returnOnFailure),
                          prefixType);

        /* Delete defauld LpmDB and Virtual Routers */
        st = prvUtfDeleteLpmDBWithDeviceAndVirtualRouter();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfDeleteLpmDBWithDeviceAndVirtualRouter");
    }
}

/*--------------------------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmDbgHwShadowSyncValidityCheck)
{
/*
    ITERATE_DEVICES (DXCH)
    1.1. Call with vrId [0/1]
                   isIpv4 [GT_FALSE and GT_TRUE],
                   isUnicast [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call with out of range vrId[77777].
    Expected: not GT_OK.
    1.3. Call with out of range lpmDbId[55555].
    Expected: not GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32                                  lpmDbId = IP_LPM_DEFAULT_DB_ID_CNS;
    GT_U32                                  vrId = IP_LPM_DEFAULT_VR_ID_CNS;
    CPSS_IP_PROTOCOL_STACK_ENT              protocol = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_UNICAST_MULTICAST_ENT              prefixType = CPSS_UNICAST_E;
    GT_BOOL                                 returnOnFailure = GT_FALSE;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT       shadowType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvUtfGetDevShadow(dev, &shadowType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);
        if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            continue;
        }

         /* Delete defauld LpmDB and Virtual Routers */
        st = prvUtfDeleteLpmDBWithDeviceAndVirtualRouter();
        if(st == GT_NOT_FOUND)
            st=GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfDeleteLpmDBWithDeviceAndVirtualRouter");


        st = prvUtfCreateLpmDBWithDeviceAndVirtualRouterAdd();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd");


        /*  1.1.  */
        vrId = IP_LPM_DEFAULT_VR_ID_CNS+1;
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_UNICAST_E;
        returnOnFailure = GT_FALSE;

        st = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(dev, lpmDbId, vrId, protocol, prefixType,returnOnFailure);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vrId = IP_LPM_DEFAULT_VR_ID_CNS+1;
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_MULTICAST_E;
        returnOnFailure = GT_TRUE;

        st = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(dev, lpmDbId, vrId, protocol, prefixType,returnOnFailure);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        vrId = IP_LPM_DEFAULT_VR_ID_CNS;
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_UNICAST_E;
        returnOnFailure = GT_FALSE;

        st = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(dev, lpmDbId, vrId, protocol, prefixType,returnOnFailure);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vrId = IP_LPM_DEFAULT_VR_ID_CNS;
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_MULTICAST_E;
        returnOnFailure = GT_TRUE;

        st = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(dev, lpmDbId, vrId, protocol, prefixType,returnOnFailure);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.3.  */
        vrId = IP_LPM_DEFAULT_VR_ID_CNS;
        protocol = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_UNICAST_E;
        returnOnFailure = GT_FALSE;

        st = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(dev, lpmDbId, vrId, protocol, prefixType,returnOnFailure);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vrId = IP_LPM_DEFAULT_VR_ID_CNS;
        protocol = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_MULTICAST_E;
        returnOnFailure = GT_TRUE;

        st = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(dev, lpmDbId, vrId, protocol, prefixType,returnOnFailure);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Call with out of range lpmDbId[55555].
           Expected: not GT_OK.*/

        lpmDbId = IP_LPM_INVALID_DB_ID_CNS;

        st = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(dev, lpmDbId, vrId, protocol, prefixType,returnOnFailure);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

        /* Call with out of range vrId[77777].
           Expected: not GT_OK.*/

        lpmDbId = IP_LPM_DEFAULT_DB_ID_CNS;
        vrId = IP_LPM_INVALID_VR_ID_CNS;

        st = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(dev, lpmDbId, vrId, protocol, prefixType,returnOnFailure);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

        /*  restore  */
        vrId = IP_LPM_DEFAULT_VR_ID_CNS;
        protocol = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_MULTICAST_E;
        returnOnFailure = GT_FALSE;

        /* Delete defauld LpmDB and Virtual Routers */
        st = prvUtfDeleteLpmDBWithDeviceAndVirtualRouter();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfDeleteLpmDBWithDeviceAndVirtualRouter");
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(dev, lpmDbId, vrId, protocol, prefixType,returnOnFailure);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(dev, lpmDbId, vrId, protocol, prefixType,returnOnFailure);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/* This ip structs contains full ipv4 prefix with prefix length and
   exist variable that shows whether this prefix was inserted or not*/
typedef struct arrPrefix_stc
{
    GT_U8 exist;
    GT_U8 ipAddr0;
    GT_U8 ipAddr1;
    GT_U8 ipAddr2;
    GT_U8 ipAddr3;
    GT_U8 prefix;
}ARR_PREFIX_STC;

/* all ipAddrArr_scenario arrays are intended to create special
   test conditions with big number non-valid nodes in trie. */
static ARR_PREFIX_STC ipAddrArr_scenario1[] =
{
    {0,0,0,0,0,8 },
    {0,193,0,61,36,16 },
    {0,193,0,61,36,32 },
    {0,2,2,2,36,24 },
    {0,2,2,2,36,32 },
    {0,2,2,3,36,24 },
    {0,2,2,3,36,32 },
    {0,2,2,4,36,24 },
    {0,2,2,4,36,32},
    {0,5,1,1,36,16},
    {0,5,1,1,36,32},
    {0,84,1,1,2,24},
    {0,84,1,1,2,32},
    {0,60,0,0,51,24},
    {0,60,0,0,51,32},
    {0,100,0,0,51,24},
    {0,100,0,0,51,32},
    {0,5,1,0,3,32     },
    {0,2,2,3,47,32    },
    {0,193,0,61,21,32 },
    {0,193,0,61,20,32 },
    {0,2,2,4,52,32    },
    {0,100,0,0,52,32 },
    {0,193,0,61,2,32 },
    {0,193,0,61,22,32 },
    {0,193,0,61,3,32 },
    {0,193,0,61,1,32 },
    {0,2,2,2,12,32   },
    {0,2,2,2,1,32     },
    {0,2,2,2,10,32   },
    {0,5,1,3,92,32   },
    {0,2,2,3,2,32     },
    {0,5,1,0,1,32     },
    {0,2,2,3,6,32     },
    {0,2,2,4,50,32   },
    {0,60,0,0,102,32},
    {0,60,0,0,103,32 },
    {0,2,2,2,50,32   },
    {0,193,0,61,4,32},
    {0,60,0,0,100,32},
    {0,60,0,0,104,32},
    {0,82,0,0,0,8      },
    {0,192,168,48,0,24},
    {0,0xFF,0xFF,0xFF,0xFF,0xFF   }
};

static ARR_PREFIX_STC ipAddrArr_scenario2[] =
{
    {0,0,0,0,0,8      },
    {0,191,0,0,125,24 },
    {0,191,0,0,125,32 },
    {0,100,0,0,125,24 },
    {0,100,0,0,125,32 },
    {0,200,0,0,125,24 },
    {0,200,0,0,125,32 },
    {0,30,0,0,125,24  },
    {0,30,0,0,125,32 },
    {0,40,0,0,125,24 },
    {0,40,0,0,125,32 },
    {0,50,0,0,125,24 },
    {0,50,0,0,125,32 },
    {0,60,0,0,125,24 },
    {0,60,0,0,125,32 },
    {0,70,0,0,125,24 },
    {0,70,0,0,125,32 },
    {0,3,3,3,3,32 },
    {0,4,4,4,4,32 },
    {0,5,5,5,5,32 },
    {0,6,6,6,6,32 },
    {0,7,7,7,7,32 },
    {0,8,8,8,8,32 },
    {0,9,9,9,9,32 },
    {0,10,10,10,10,32 },
    {0,11,11,11,11,32 },
    {0,12,12,12,0,24 },
    {0,30,2,0,0,24 },
    {0,30,3,0,0,24 },
    {0,30,4,0,0,24 },
    {0,40,2,0,0,24 },
    {0,40,3,0,0,24 },
    {0,40,4,0,0,24 },
    {0,50,2,0,0,24 },
    {0,50,3,0,0,24 },
    {0,50,4,0,0,24 },
    {0,60,2,0,0,24 },
    {0,60,3,0,0,24 },
    {0,60,4,0,0,24 },
    {0,70,2,0,0,24 },
    {0,70,3,0,0,24  },
    {0,70,4,0,0,24 },
    {0,100,2,0,0,24 },
    {0,100,3,0,0,24 },
    {0,100,4,0,0,24 },
    {0,200,2,0,0,24 },
    {0,200,3,0,0,24 },
    {0,200,4,0,0,24 },
    {0,191,0,0,11,32  },
    {0,191,0,0,122,32 },
    {0,191,0,0,35,32 },
    {0,191,0,0,200,32 },
    {0,100,0,0,126,32 },
    {0,100,0,0,127,32 },
    {0,100,0,0,128,32 },
    {0,100,0,0,129,32 },
    {0,100,0,0,130,32 },
    {0,100,0,0,131,32 },
    {0,100,0,0,132,32 },
    {0,100,0,0,133,32 },
    {0,100,0,0,134,32 },
    {0,100,0,0,135,32 },
    {0,100,0,0,136,32 },
    {0,100,0,0,137,32 },
    {0,100,0,0,138,32 },
    {0,100,0,0,139,32 },
    {0,100,0,0,140,32 },
    {0,100,0,0,141,32 },
    {0,100,0,0,142,32 },
    {0,100,0,0,143,32 },
    {0,100,0,0,144,32 },
    {0,100,0,0,145,32 },
    {0,50,0,0,155,32  },
    {0,100,0,0,146,32 },
    {0,100,0,0,147,32 },
    {0,100,0,0,148,32 },
    {0,100,0,0,149,32 },
    {0,100,0,0,150,32 },
    {0,100,0,0,151,32 },
    {0,100,0,0,152,32 },
    {0,100,0,0,153,32 },
    {0,100,0,0,154,32 },
    {0,100,0,0,155,32 },
    {0,191,0,0,9,32   },
    {0,0xFF,0xFF,0xFF,0xFF,0xFF   }
};


/*
 cpssDxChIpLpmRandomResizeTest
(
    IN GT_U32                                       lpmDBId,
    IN CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType,
    IN CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN GT_BOOL                                      partitionEnable,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmRandomResizeTest)
{
/*
    1.1. Call cpssDxChIpLpmDBCreate with lpmDbId[1]
              and other valid parameters.
    Expected: GT_OK
    1.2. Call cpssDxChIpLpmDBDevListAdd for lpmDbId[1]
    Expected: GT_OK
    1.3 fill vrConfigInfo for PBR and Router Tcam routing mode.
    Expected: GT_OK.
    1.4. Call cpssDxChIpLpmVirtualRouterAdd for lpmDbId[1],vrId[0] .
    Expected: GT_OK.
    1.5.Perform  cyclic test that contained add ipv4 uc prefix, delete ipv4 uc prefixes
        and conditional resize (reduce,enlarge). There are for scenarious.
        First two are taking prefixes and prefix lenght by random base from the fix prefix array
        and resize conditions are the same.
        Third scenario is similar but enlarge resize delta is differ.
        Fourth scenario generates all prefies and its lengths by random manner.
        Each scenario is limited by action counter. (Action is add, del, resize).

    1.6. When test is ended flush ipv4 prefixes.
    Expected: GT_OK.
    1.7. Delete virtual router vrId[0].
    1.8. Delete lpmDBId[1] . Expected: GT_OK.
    Expected: GT_OK.
*/

    GT_STATUS                                    st = GT_OK;
    GT_U8                                        dev, devNum;
    GT_U8                                        devList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                                       numOfDevs = 0;
    GT_U32                                       lpmDBId;
    GT_U32                                       vrId;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    GT_BOOL                                      partitionEnable ;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC tcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT       nextHopInfo;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC               vrConfigInfo;
    GT_IPADDR                                    ipAddr;
    GT_IPADDR                                    ipAddr1;
    ARR_PREFIX_STC                               *prefixPtr = NULL;
    GT_U32                                       size = 0;
    GT_U32                                       count = 1;
    GT_U32                                       countTotal = 1;
    GT_U32                                       max = 0;
    GT_U32                                       i = 0;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC             tcamRange;
    GT_VOID                                      *tcamManagerHandlerPtr;
    GT_U32                                       tcamSize;
    GT_U32                                       enlargeResizeDelta = 0;
    GT_U32                                       reduceResizeDelta = 0;

    PRV_CPSS_DXCH_LPM_SHADOW_STC                 tmpLpmDb;
    PRV_CPSS_DXCH_LPM_SHADOW_STC                 *ipLpmShadowPtr;
    GT_U32                                       seed;
    GT_U32                                       scenario;
    GT_U32                                       index;
    GT_U32                                       j;
    GT_BOOL                                      isExactPrefixExist = GT_TRUE;
    GT_U32                                       bytesNumber = 0;
    GT_U32                                       tempBytesNumber = 0;
    GT_U32                                       remainedBits = 0;
    GT_U32                                       x = 0;
    GT_U32                                       y = 0;
    GT_BOOL                                      defragmentationEnable = GT_FALSE;

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
    /* Skip this test for GM - performance reason */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;
    /* skip to reduce size of CPSS API log */
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&indexesRange, sizeof(indexesRange));
    cpssOsBzero((GT_VOID*)&tcamLpmManagerCapcityCfg, sizeof(tcamLpmManagerCapcityCfg));
    cpssOsBzero((GT_VOID*)&lpmMemoryConfig, sizeof(lpmMemoryConfig));
    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&vrConfigInfo, sizeof(vrConfigInfo));
    cpssOsBzero((GT_VOID*)&tcamRange, sizeof(tcamRange));
    cpssOsBzero((GT_VOID*)&tmpLpmDb, sizeof(tmpLpmDb));

    /* get random seed */
    seed = prvUtfSeedFromStreamNameGet();
    /* set specific seed for random generator */
    cpssOsSrand(seed);
    /* prepare device iterator */

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfDevs = 0;

        /* Create LpmDB [1] */
        lpmDBId                                             = 19;
        vrId                                                = 0;
        protocolStack                                       = CPSS_IP_PROTOCOL_IPV4_E;
        indexesRange.firstIndex                             = 0;
        indexesRange.lastIndex                              = 1;
        tcamSize                                            = 8;
        partitionEnable                                     = GT_TRUE;
        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes          = 8;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes  = 0;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes          = 0;

        if (prvUtfIsPbrModeUsed() == GT_TRUE)
        {
           shadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E;
        }
        else
        {
            shadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
        }


        /*1.1. Call cpssDxChIpLpmDBCreate with lpmDbId[1], numOfIpv4Prefixes = 8,
               numOfIpv4McSourcePrefixes = 0; numOfIpv6Prefixes = 0. Expected: GT_OK.*/
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &indexesRange;
        lpmMemoryConfig.tcamDbCfg.partitionEnable = partitionEnable;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = NULL;
        st = cpssDxChIpLpmDBCreate(lpmDBId,shadowType,protocolStack,
                                   &lpmMemoryConfig);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate: %d", lpmDBId);
        if (prvUtfIsPbrModeUsed() == GT_TRUE)
        {
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);
        }
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            devList[numOfDevs++] = devNum;
        }
        /*1.2. Call cpssDxChIpLpmDBDevListAdd for lpmDbId[1]. Expected: GT_OK. */

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId, numOfDevs);

        /* 1.3 fill vrConfigInfo for PBR and Router Tcam routing mode. Expected: GT_OK. */
        cpssOsMemSet(&vrConfigInfo,0,sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));
        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc= GT_FALSE;
        vrConfigInfo.supportIpv4Uc = GT_TRUE;
        vrConfigInfo.supportIpv6Uc= GT_FALSE;
        vrConfigInfo.supportFcoe= GT_FALSE;
        if (prvUtfIsPbrModeUsed() == GT_TRUE)
        {
            vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
            vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
            vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.ipUcRoute.doIpUcRoute = GT_TRUE;
        }
        else
        {
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths               = 0;
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex      = 0;
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeType                = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.ucRPFCheckEnable         = GT_FALSE;
        }
        /*1.4. Call cpssDxChIpLpmVirtualRouterAdd for lpmDbId[1],vrId[0] . Expected: GT_OK. */

        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"VirtualRouterAdd: lpmDBId = %d, vrId = %d", lpmDBId, vrId);

        tmpLpmDb.lpmDbId =lpmDBId;
        ipLpmShadowPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
        if (ipLpmShadowPtr == NULL)
        {
            st = utfFailureMsgLog("Can't find the lpm DB.\n", NULL, 0);
            return;
        }
        tcamManagerHandlerPtr = ((PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC*)ipLpmShadowPtr->shadow)->tcamManagerHandlerPtr;

        /* 1.5. Perform  cyclic test that contained add ipv4 uc prefix, delete ipv4 uc prefixes
                end conditional resize (reduce,enlarge). There are for scenarious.
                First two are taking prefixes and prefix lenght by random base from the fix prefix array
                and resize conditions are the same.
                Third scenario is similar but enlarge resize delta is differ.
                Fourth scenario generates all prefies and its lengths by random manner.
                Each scenario is limited by action counter. (Action is add, del, resize).
                After each action trie validity check is done.
                When current scenario is finished, all prefixes are deleted and tcam size set to
                initial value.
         */
        for (scenario = 0; scenario < 4; scenario++ )
        {
            if(scenario == 0)
            {
                prefixPtr = ipAddrArr_scenario1;
                size = sizeof(ipAddrArr_scenario1)/sizeof(ARR_PREFIX_STC);
                enlargeResizeDelta = 8;
                reduceResizeDelta = 8;
            }
            if(scenario == 1)
            {
                prefixPtr = ipAddrArr_scenario2;
                size = sizeof(ipAddrArr_scenario2)/sizeof(ARR_PREFIX_STC);
                enlargeResizeDelta = 8;
                reduceResizeDelta = 8;
            }
            if(scenario == 2)
            {
                prefixPtr = ipAddrArr_scenario1;
                size = sizeof(ipAddrArr_scenario1)/sizeof(ARR_PREFIX_STC);
                for (i = 0; i < size - 1; i++)
                {
                    ipAddrArr_scenario1[i].exist = 0;
                }
                enlargeResizeDelta = 12;
                reduceResizeDelta = 8;
            }

            if(scenario == 3)
            {
                prefixPtr = ipAddrArr_scenario1;
                size = sizeof(ipAddrArr_scenario1)/sizeof(ARR_PREFIX_STC);

                /* fill array for scenario 1  with random prefixes*/
                for (i = 0; i < size - 1; i++)
                {
                    ipAddrArr_scenario1[i].exist = 0;
                    isExactPrefixExist = GT_TRUE;
                    while (isExactPrefixExist == GT_TRUE)
                    {
                        isExactPrefixExist = GT_FALSE;
                        ipAddrArr_scenario1[i].ipAddr0 = cpssOsRand()% 224;
                        ipAddrArr_scenario1[i].ipAddr1 = cpssOsRand()% 256;
                        ipAddrArr_scenario1[i].ipAddr2 = cpssOsRand()% 256;
                        ipAddrArr_scenario1[i].ipAddr3 = cpssOsRand()% 256;
                        ipAddrArr_scenario1[i].prefix = 0;
                        /* prefix length is not equal to 0 */
                        while (ipAddrArr_scenario1[i].prefix == 0)
                        {
                            ipAddrArr_scenario1[i].prefix =  cpssOsRand()% 32;
                        }
                        /* verify there was no such prefix before*/
                        for (j = 0; j < i; j++)
                        {
                            if (ipAddrArr_scenario1[i].prefix == ipAddrArr_scenario1[j].prefix)
                            {
                                /* the prefix length is the same. Now compare prefixes bits.
                                 (number of compared bits is prefix length)*/
                                ipAddr.arIP[0] = ipAddrArr_scenario1[i].ipAddr0;
                                ipAddr.arIP[1] = ipAddrArr_scenario1[i].ipAddr1;
                                ipAddr.arIP[2] = ipAddrArr_scenario1[i].ipAddr2;
                                ipAddr.arIP[3] = ipAddrArr_scenario1[i].ipAddr3;
                                ipAddr1.arIP[0] = ipAddrArr_scenario1[j].ipAddr0;
                                ipAddr1.arIP[1] = ipAddrArr_scenario1[j].ipAddr1;
                                ipAddr1.arIP[2] = ipAddrArr_scenario1[j].ipAddr2;
                                ipAddr1.arIP[3] = ipAddrArr_scenario1[j].ipAddr3;

                                bytesNumber = ipAddrArr_scenario1[i].prefix/8;
                                tempBytesNumber = bytesNumber;
                                remainedBits = ipAddrArr_scenario1[i].prefix%8;
                                while (tempBytesNumber > 0)
                                {
                                    tempBytesNumber--;
                                    if (ipAddr.arIP[tempBytesNumber] != ipAddr1.arIP[tempBytesNumber])
                                    {
                                        isExactPrefixExist = GT_FALSE;
                                        break;
                                    }
                                    else
                                    {
                                        isExactPrefixExist = GT_TRUE;
                                    }
                                }
                                if ( (isExactPrefixExist == GT_TRUE) || (bytesNumber == 0))
                                {
                                    if (remainedBits > 0)
                                    {
                                        x = ipAddr.arIP[bytesNumber]>>(8 - remainedBits);
                                        y = ipAddr1.arIP[bytesNumber]>>(8 - remainedBits);
                                        if (x == y)
                                        {
                                            isExactPrefixExist =  GT_TRUE;
                                            break;
                                        }
                                        else
                                        {
                                            isExactPrefixExist = GT_FALSE;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                enlargeResizeDelta = 8;
                reduceResizeDelta = 8;
            }

            while(1)
            {
                index = cpssOsRand();
                index = index%size;
                if(prefixPtr[index].ipAddr0 == 0xFF)
                    continue;
                ipAddr.arIP[0] = prefixPtr[index].ipAddr0;
                ipAddr.arIP[1] = prefixPtr[index].ipAddr1;
                ipAddr.arIP[2] = prefixPtr[index].ipAddr2;
                ipAddr.arIP[3] = prefixPtr[index].ipAddr3;
                if(prefixPtr[index].exist == 0)
                {
                    st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr,
                                                      prefixPtr[index].prefix,
                                                      &nextHopInfo, GT_FALSE, defragmentationEnable);
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmIpv4UcPrefixAdd: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                 scenario, tcamSize,count,countTotal);
                    st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                    if (st!=GT_OK)
                    {
                        cpssOsPrintf("fail in %d.%d.%d.%d/%d\n", ipAddr.arIP[0],ipAddr.arIP[1],ipAddr.arIP[2],ipAddr.arIP[3],prefixPtr[index].prefix);
                        st = cpssDxChIpLpmDbgPatTriePrint(lpmDBId,vrId,0,0);
                        if (st !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                            return;
                        }

                        st = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                        if (st !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                            return;
                        }

                        return;
                    }
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDbgPatTrieValidityCheck after prefix add: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                 scenario, tcamSize,count,countTotal);
                    prefixPtr[index].exist = 1;
                    count ++;
                    if(count == tcamSize)
                    {
                        /* enlarge tcam size */
                        tcamSize += enlargeResizeDelta;
                        tcamRange.firstLine= 0;
                        tcamRange.lastLine= tcamSize/4-1;
                        st =  cpssDxChTcamManagerRangeUpdate(tcamManagerHandlerPtr,
                                                             &tcamRange,
                                                             CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E);
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChTcamManagerRangeUpdate - resize+: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                 scenario, tcamSize,count,countTotal);
                        st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after ManagerRangeUpdate - resize+: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                     scenario, tcamSize,count,countTotal);
                        indexesRange.firstIndex = 0;
                        indexesRange.lastIndex = tcamRange.lastLine;
                        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 0;
                        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes= 0;
                        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = tcamSize;
                        st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, &indexesRange, &tcamLpmManagerCapcityCfg);
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDBCapacityUpdate - resize+: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                 scenario, tcamSize,count,countTotal);
                        st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after cpssDxChIpLpmDBCapacityUpdate - resize+: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                     scenario, tcamSize,count,countTotal);
                    }
                  }
                  else if(prefixPtr[index].exist == 1)
                  {
                      st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr,
                                                        prefixPtr[index].prefix,
                                                        &nextHopInfo, GT_TRUE, defragmentationEnable);
                      UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmIpv4UcPrefixAdd override: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                   scenario, tcamSize,count,countTotal);
                      if (st!=GT_OK)
                      {
                        cpssOsPrintf("fail in %d.%d.%d.%d\n", ipAddr.arIP[0],ipAddr.arIP[1],ipAddr.arIP[2],ipAddr.arIP[3]);
                        st = cpssDxChIpLpmDbgPatTriePrint(lpmDBId,vrId,0,0);
                        if (st !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                            return;
                        }

                        st = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                        if (st !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                            return;
                        }

                        return;
                    }
                      prefixPtr[index].exist = 2;
                      st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                      UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after cpssDxChIpLpmIpv4UcPrefixAdd override:: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                   scenario, tcamSize,count,countTotal);
                  }
                  else
                  {
                      st = cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr,
                                                        prefixPtr[index].prefix);
                      UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmIpv4UcPrefixDel : scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                   scenario, tcamSize,count,countTotal);
                      if (st!=GT_OK)
                      {
                        cpssOsPrintf("fail in %d.%d.%d.%d\n", ipAddr.arIP[0],ipAddr.arIP[1],ipAddr.arIP[2],ipAddr.arIP[3]);
                        st = cpssDxChIpLpmDbgPatTriePrint(lpmDBId,vrId,0,0);
                        if (st !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                            return;
                        }

                        st = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                        if (st !=GT_OK)
                        {
                            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                            return;
                        }

                        return;
                      }
                      prefixPtr[index].exist = 0;
                      count --;
                      st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                      UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after cpssDxChIpLpmIpv4UcPrefixDel: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                   scenario, tcamSize,count,countTotal);

                      if(count == tcamSize-12)
                      {
                        /* reduce tcam size */
                        tcamSize -= reduceResizeDelta;
                        indexesRange.firstIndex = 0;
                        indexesRange.lastIndex = (tcamSize+3)/4-1;
                        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 0;
                        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes= 0;
                        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = tcamSize;
                        st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, &indexesRange, &tcamLpmManagerCapcityCfg);
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDBCapacityUpdate - resize-: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                 scenario, tcamSize,count,countTotal);
                        st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after cpssDxChIpLpmDBCapacityUpdate - resize+: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                     scenario, tcamSize,count,countTotal);

                        tcamRange.firstLine = 0;
                        tcamRange.lastLine = indexesRange.lastIndex;
                        st =  cpssDxChTcamManagerRangeUpdate(tcamManagerHandlerPtr,
                                                             &tcamRange,
                                                             CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E);
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChTcamManagerRangeUpdate - resize-: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                 scenario, tcamSize,count,countTotal);
                        st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after cpssDxChTcamManagerRangeUpdate - resize-: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                     scenario, tcamSize,count,countTotal);
                      }
                  }
                  if (count>max) max = count;
                  countTotal ++;
                  if (countTotal > 125000)
                  {
                      break;
                  }
            }
            countTotal = 1;
            count = 1;
            /* delete all prefixes*/
            st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmIpv4UcPrefixesFlush: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                     scenario, tcamSize,count,countTotal);
            st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after Flush: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                         scenario, tcamSize,count,countTotal);

            /* resize to tcam size 8 */
            tcamSize = 8;
            indexesRange.firstIndex = 0;
            indexesRange.lastIndex = (tcamSize+3)/4-1;
            tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 0;
            tcamLpmManagerCapcityCfg.numOfIpv6Prefixes= 0;
            tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = tcamSize;

            st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, &indexesRange, &tcamLpmManagerCapcityCfg);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDBCapacityUpdate -after scenario: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                     scenario, tcamSize,count,countTotal);
            st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after cpssDxChIpLpmDBCapacityUpdate - resize+: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                         scenario, tcamSize,count,countTotal);
            tcamRange.firstLine = 0;
            tcamRange.lastLine = indexesRange.lastIndex;

            st =  cpssDxChTcamManagerRangeUpdate(tcamManagerHandlerPtr,
                                                 &tcamRange,
                                                 CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChTcamManagerRangeUpdate after scenario: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                     scenario, tcamSize,count,countTotal);
            st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after ManagerRangeUpdate - resize+: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                         scenario, tcamSize,count,countTotal);
        }
        /* restore configuration */
        /*1.6. delete all ipv4 prefixes  Expected: GT_OK. */
        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,"cpssDxChIpLpmIpv4UcPrefixesFlush: scenario = %d, tcamSize = %d,prefixCounter = %d,totalCount = %d\n",
                                                 scenario, tcamSize,count,countTotal);
        /* 1.7. Delete virtual router vrId[0]. Expected: GT_OK.   */
        st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

        /* 1.8. Delete lpmDBId[1] . Expected: GT_OK. */
        st = cpssDxChIpLpmDBDelete(lpmDBId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);
    }
}


/* all ipAddrArr_scenario arrays are intended to create special
   test conditions with big number non-valid nodes in trie. */
static ARR_PREFIX_STC ipAddrArr_smallScenario[] =
{
    {0,1,1,1,1,32 },
    {0,1,1,1,1,30 },
    {0,1,1,1,1,28 },
    {0,1,1,1,1,26 },
    {0,1,1,1,1,24 },
    {0,1,1,1,1,22 },
    {0,1,1,1,1,20 },
    {0,1,1,1,1,18 },
    {0,1,1,1,1,16 },
    {0,1,1,1,1,14 },
    {0,1,1,1,1,12 }
};

/*
 cpssDxChIpLpmIpv4UcSmallResizeTest
(
    IN GT_U32                                       lpmDBId,
    IN CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType,
    IN CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN GT_BOOL                                      partitionEnable,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4UcSmallResizeTest)
{
/*
    1.1. Call cpssDxChIpLpmDBCreate with lpmDbId[19]
              and other valid parameters.
    Expected: GT_OK
    1.2. Call cpssDxChIpLpmDBDevListAdd for lpmDbId[19]
    Expected: GT_OK
    1.3 fill vrConfigInfo for PBR and Router Tcam routing mode.
    Expected: GT_OK.
    1.4. Call cpssDxChIpLpmVirtualRouterAdd for lpmDbId[19],vrId[0] .
    Expected: GT_OK.
    1.5.Add ipv4 uc prefix, delete ipv4 uc prefixes
        and conditional resize (reduce).
    1.6. When test is ended flush ipv4 prefixes.
    Expected: GT_OK.
    1.7. Delete virtual router vrId[0].
    1.8. Delete lpmDBId[19] . Expected: GT_OK.
    Expected: GT_OK.
*/

    GT_STATUS                                    st = GT_OK;
    GT_U8                                        dev, devNum;
    GT_U8                                        devList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                                       numOfDevs = 0;
    GT_U32                                       lpmDBId;
    GT_U32                                       vrId;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    GT_BOOL                                      partitionEnable ;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC tcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT       nextHopInfo;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC               vrConfigInfo;
    GT_IPADDR                                    ipAddr;
    ARR_PREFIX_STC                               *prefixPtr = NULL;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC             tcamRange;
    GT_VOID                                      *tcamManagerHandlerPtr;
    GT_U32                                       tcamSize;
    GT_U32                                       reduceResizeDelta = 0;

    PRV_CPSS_DXCH_LPM_SHADOW_STC                 tmpLpmDb;
    PRV_CPSS_DXCH_LPM_SHADOW_STC                 *ipLpmShadowPtr;
    GT_U32                                       index;
    GT_BOOL                                      defragmentationEnable = GT_FALSE;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*)&indexesRange, sizeof(indexesRange));
    cpssOsBzero((GT_VOID*)&tcamLpmManagerCapcityCfg, sizeof(tcamLpmManagerCapcityCfg));
    cpssOsBzero((GT_VOID*)&lpmMemoryConfig, sizeof(lpmMemoryConfig));
    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&vrConfigInfo, sizeof(vrConfigInfo));
    cpssOsBzero((GT_VOID*)&tcamRange, sizeof(tcamRange));
    cpssOsBzero((GT_VOID*)&tmpLpmDb, sizeof(tmpLpmDb));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfDevs = 0;

        /* Create LpmDB [1] */
        lpmDBId                                             = 19;
        vrId                                                = 0;
        protocolStack                                       = CPSS_IP_PROTOCOL_IPV4_E;
        indexesRange.firstIndex                             = 0;
        indexesRange.lastIndex                              = 2;
        tcamSize                                            = 12;
        partitionEnable                                     = GT_TRUE;
        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes          = 12;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes  = 0;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes          = 0;

        if (prvUtfIsPbrModeUsed() == GT_TRUE)
        {
           shadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E;
        }
        else
        {
           shadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
        }


        /*1.1. Call cpssDxChIpLpmDBCreate with lpmDbId[19], numOfIpv4Prefixes = 8,
               numOfIpv4McSourcePrefixes = 0; numOfIpv6Prefixes = 0. Expected: GT_OK.*/
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &indexesRange;
        lpmMemoryConfig.tcamDbCfg.partitionEnable = partitionEnable;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = NULL;
        st = cpssDxChIpLpmDBCreate(lpmDBId,shadowType,protocolStack,&lpmMemoryConfig);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate: %d", lpmDBId);
        if (prvUtfIsPbrModeUsed() == GT_TRUE)
        {
            prvUtfSetDefaultActionEntry(&nextHopInfo.pclIpUcAction);
        }
        else
        {
            prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);
        }
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            devList[numOfDevs++] = devNum;
        }
        /*1.2. Call cpssDxChIpLpmDBDevListAdd for lpmDbId[19]. Expected: GT_OK. */

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId, numOfDevs);

        /* 1.3 fill vrConfigInfo for PBR and Router Tcam routing mode. Expected: GT_OK. */
        cpssOsMemSet(&vrConfigInfo,0,sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));
        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc= GT_FALSE;
        vrConfigInfo.supportIpv4Uc = GT_TRUE;
        vrConfigInfo.supportIpv6Uc= GT_FALSE;
        vrConfigInfo.supportFcoe=GT_FALSE;
        if (prvUtfIsPbrModeUsed() == GT_TRUE)
        {
            vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
            vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
            vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction.ipUcRoute.doIpUcRoute = GT_TRUE;
        }
        else
        {
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths               = 0;
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex      = 0;
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeType                = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
            vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.ucRPFCheckEnable         = GT_FALSE;
        }
        /*1.4. Call cpssDxChIpLpmVirtualRouterAdd for lpmDbId[19],vrId[0] . Expected: GT_OK. */

        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"VirtualRouterAdd: lpmDBId = %d, vrId = %d", lpmDBId, vrId);

        tmpLpmDb.lpmDbId =lpmDBId;
        ipLpmShadowPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
        if (ipLpmShadowPtr == NULL)
        {
            st = utfFailureMsgLog("Can't find the lpm DB.\n", NULL, 0);
            return;
        }
        tcamManagerHandlerPtr = ((PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC*)ipLpmShadowPtr->shadow)->tcamManagerHandlerPtr;

        /* 1.5.Add ipv4 uc prefix, delete ipv4 uc prefixes and conditional resize (reduce,enlarge).*/
        prefixPtr = ipAddrArr_smallScenario;
        reduceResizeDelta = 4;

        /* add prefixes 1.1.1.1/[32,30,28,26,24,22,20,18]*/
        for (index=0;index<11;index++)
        {
            ipAddr.arIP[0] = prefixPtr[index].ipAddr0;
            ipAddr.arIP[1] = prefixPtr[index].ipAddr1;
            ipAddr.arIP[2] = prefixPtr[index].ipAddr2;
            ipAddr.arIP[3] = prefixPtr[index].ipAddr3;

            st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr,prefixPtr[index].prefix,&nextHopInfo, GT_FALSE, defragmentationEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmIpv4UcPrefixAdd:tcamSize = %d\n",tcamSize);
            st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
            if (st!=GT_OK)
            {
                cpssOsPrintf("Add fail in %d.%d.%d.%d/%d\n", ipAddr.arIP[0],ipAddr.arIP[1],ipAddr.arIP[2],ipAddr.arIP[3],prefixPtr[index].prefix);
                st = cpssDxChIpLpmDbgPatTriePrint(lpmDBId,vrId,0,0);
                if (st !=GT_OK)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                    return;
                }

                st = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                if (st !=GT_OK)
                {
                    PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                    return;
                }
                return;
            }
        }

        /*delete prefix 1.1.1.1/20,16,14,12*/
        for (index=10;index>5; index--)
        {
            /* we do not want to delete 1.1.1.1/18 */
            if (index==7)
            {
                continue;
            }
            ipAddr.arIP[0] = prefixPtr[index].ipAddr0;
            ipAddr.arIP[1] = prefixPtr[index].ipAddr1;
            ipAddr.arIP[2] = prefixPtr[index].ipAddr2;
            ipAddr.arIP[3] = prefixPtr[index].ipAddr3;

            st = cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr,prefixPtr[index].prefix);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmIpv4UcPrefixAdd:tcamSize = %d\n",tcamSize);
            st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
            if (st!=GT_OK)
            {
                cpssOsPrintf("Delete fail in %d.%d.%d.%d/%d\n", ipAddr.arIP[0],ipAddr.arIP[1],ipAddr.arIP[2],ipAddr.arIP[3],prefixPtr[index].prefix);
                st = cpssDxChIpLpmDbgPatTriePrint(lpmDBId,vrId,0,0);
                if (st !=GT_OK)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                    return;
                }

                st = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
                if (st !=GT_OK)
                {
                    PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                    return;
                }
                return;
            }
        }

        cpssOsPrintf("before reduce\n");
        st = cpssDxChIpLpmDbgPatTriePrint(lpmDBId,vrId,0,0);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        st = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        /* reduce the tcam from 3 lines to 2 lines --> default should be moved from
           (row2,column3) to (row1,colums3) leading to the move of 1.1.1.1/18 from
           (row1,column3) to (row1,column2)*/
        tcamSize -= reduceResizeDelta;
        indexesRange.firstIndex = 0;
        indexesRange.lastIndex = (tcamSize+3)/4-1;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 0;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes= 0;
        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = tcamSize;
        st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, &indexesRange, &tcamLpmManagerCapcityCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDBCapacityUpdate - resize-: tcamSize = %d\n",tcamSize);
        st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after cpssDxChIpLpmDBCapacityUpdate - resize+: tcamSize = %d\n",tcamSize);

        tcamRange.firstLine = 0;
        tcamRange.lastLine = indexesRange.lastIndex;
        st =  cpssDxChTcamManagerRangeUpdate(tcamManagerHandlerPtr,
                                             &tcamRange,
                                             CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChTcamManagerRangeUpdate - resize-: tcamSize = %d\n",tcamSize);
        st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after cpssDxChTcamManagerRangeUpdate - resize-: tcamSize = %d\n",tcamSize);
        cpssOsPrintf("after reduce\n");
        st = cpssDxChIpLpmDbgPatTriePrint(lpmDBId,vrId,0,0);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        st = cpssDxChIpLpmDbgRouteTcamDump(GT_TRUE);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        /* restore configuration */
        /*1.6. delete all ipv4 prefixes  Expected: GT_OK. */
        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmIpv4UcPrefixesFlush: tcamSize = %d\n",tcamSize);

        st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after Flush:tcamSize = %d\n",tcamSize);

        /* 1.7. Delete virtual router vrId[0]. Expected: GT_OK.   */
        st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

        /* 1.8. Delete lpmDBId[1] . Expected: GT_OK. */
        st = cpssDxChIpLpmDBDelete(lpmDBId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);
    }
}

#if 0
/* all ipv4McAddrArr_smallScenario arrays are intended to create special
   test conditions with big number non-valid nodes in trie. */
static ARR_PREFIX_STC ipv4McGrpAddrArr_smallScenario[] =
{
    {0,225,1,1,1,32 },
    {0,225,1,1,1,30 },
    {0,225,1,1,1,28 },
    {0,225,1,1,1,26 },
    {0,225,1,1,1,24 },
    {0,225,1,1,1,22 },
    {0,225,1,1,1,20 },
    {0,225,1,1,1,18 },
    {0,225,1,1,1,16 },
    {0,225,1,1,1,14 },
    {0,225,1,1,1,12 }
};
static ARR_PREFIX_STC ipv4McSrcAddrArr_smallScenario[] =
{
    {0,1,1,1,1,32 },
    {0,1,1,1,1,30 },
    {0,1,1,1,1,28 },
    {0,1,1,1,1,26 },
    {0,1,1,1,1,24 },
    {0,1,1,1,1,22 },
    {0,1,1,1,1,20 },
    {0,1,1,1,1,18 },
    {0,1,1,1,1,16 },
    {0,1,1,1,1,14 },
    {0,1,1,1,1,12 }
};
#endif

/*
 cpssDxChIpLpmIpv4McSmallResizeTest
(
    IN GT_U32                                       lpmDBId,
    IN CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType,
    IN CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN GT_BOOL                                      partitionEnable,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4McSmallResizeTest)
{
/*
    1.1. Call cpssDxChIpLpmDBCreate with lpmDbId[19]
              and other valid parameters.
    Expected: GT_OK
    1.2. Call cpssDxChIpLpmDBDevListAdd for lpmDbId[19]
    Expected: GT_OK
    1.3 fill vrConfigInfo for PBR and Router Tcam routing mode.
    Expected: GT_OK.
    1.4. Call cpssDxChIpLpmVirtualRouterAdd for lpmDbId[1],vrId[0] .
    Expected: GT_OK.
    1.5.Add ipv4 Mc prefix, delete ipv4 Mc prefixes
        and conditional resize (reduce).
    1.6. When test is ended flush ipv4 prefixes.
    Expected: GT_OK.
    1.7. Delete virtual router vrId[0].
    1.8. Delete lpmDBId[19] . Expected: GT_OK.
    Expected: GT_OK.
*/
#if 0
    GT_STATUS                                    st = GT_OK;
    GT_U8                                        dev, devNum;
    GT_U8                                        devList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                                       numOfDevs = 0;
    GT_U32                                       lpmDBId;
    GT_U32                                       vrId;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    GT_BOOL                                      partitionEnable ;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC tcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;
    CPSS_DXCH_IP_LTT_ENTRY_STC                   mcRouteLttEntry;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC               vrConfigInfo;
    GT_IPADDR                                    grpIpAddr;
    GT_IPADDR                                    srcIpAddr;
    ARR_PREFIX_STC                               *grpPrefixPtr = NULL;
    ARR_PREFIX_STC                               *srcPrefixPtr = NULL;
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC             tcamRange;
    GT_VOID                                      *tcamManagerHandlerPtr;
    GT_U32                                       tcamSize;
    GT_U32                                       reduceResizeDelta = 0;

    PRV_CPSS_DXCH_LPM_SHADOW_STC                 tmpLpmDb;
    PRV_CPSS_DXCH_LPM_SHADOW_STC                 *ipLpmShadowPtr;
    GT_U32                                       index;
#endif
    /* due to a bug in IPv4 MC TCAM resize this test will be skiped.
       when the bug is fixed the "#if 0" should be removed */
    SKIP_TEST_MAC

#if 0

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    if (prvUtfIsPbrModeUsed() == GT_TRUE)
    {
        /* skip this test, PBR mode do not support MC*/
        SKIP_TEST_MAC
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfDevs = 0;

        /* Create LpmDB [1] */
        lpmDBId                                             = 19;
        vrId                                                = 0;
        protocolStack                                       = CPSS_IP_PROTOCOL_IPV4_E;
        indexesRange.firstIndex                             = 0;
        indexesRange.lastIndex                              = 2;
        tcamSize                                            = 12;
        partitionEnable                                     = GT_TRUE;
        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes          = 6;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes  = 6;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes          = 0;

        shadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;

        /*1.1. Call cpssDxChIpLpmDBCreate with lpmDbId[19], numOfIpv4Prefixes = 6,
               numOfIpv4McSourcePrefixes = 6; numOfIpv6Prefixes = 0. Expected: GT_OK.*/
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &indexesRange;
        lpmMemoryConfig.tcamDbCfg.partitionEnable = partitionEnable;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = NULL;
        st = cpssDxChIpLpmDBCreate(lpmDBId,shadowType,protocolStack,&lpmMemoryConfig);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmDBCreate: %d", lpmDBId);

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            devList[numOfDevs++] = devNum;
        }
        /*1.2. Call cpssDxChIpLpmDBDevListAdd for lpmDbId[19]. Expected: GT_OK. */

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, numOfDevs = %d", lpmDBId, numOfDevs);

        /* 1.3 fill vrConfigInfo for PBR and Router Tcam routing mode. Expected: GT_OK. */
        cpssOsMemSet(&vrConfigInfo,0,sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));
        vrConfigInfo.supportIpv4Mc = GT_TRUE;
        vrConfigInfo.supportIpv6Mc= GT_FALSE;
        vrConfigInfo.supportIpv4Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Uc= GT_FALSE;

        vrConfigInfo.defIpv4McRouteLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        vrConfigInfo.defIpv4McRouteLttEntry.numOfPaths               = 0;
        vrConfigInfo.defIpv4McRouteLttEntry.routeEntryBaseIndex      = 0;
        vrConfigInfo.defIpv4McRouteLttEntry.routeType                = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        vrConfigInfo.defIpv4McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        vrConfigInfo.defIpv4McRouteLttEntry.ucRPFCheckEnable         = GT_FALSE;

        /*1.4. Call cpssDxChIpLpmVirtualRouterAdd for lpmDbId[19],vrId[0] . Expected: GT_OK. */

        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"VirtualRouterAdd: lpmDBId = %d, vrId = %d", lpmDBId, vrId);

        tmpLpmDb.lpmDbId =lpmDBId;
        ipLpmShadowPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
        if (ipLpmShadowPtr == NULL)
        {
            st = utfFailureMsgLog("Can't find the lpm DB.\n", NULL, 0);
            return;
        }
        tcamManagerHandlerPtr = ((PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC*)ipLpmShadowPtr->shadow)->tcamManagerHandlerPtr;

        /* 1.5.Add ipv4 uc prefix, delete ipv4 uc prefixes and conditional resize (reduce,enlarge).*/
        grpPrefixPtr = ipv4McGrpAddrArr_smallScenario;
        srcPrefixPtr = ipv4McSrcAddrArr_smallScenario;
        reduceResizeDelta = 4;

        /* add group prefixes 225.1.1.1/[32,30,28,26,24] with Src 1.1.1.1/[32,30,28,26,24] */
        for (index=0;index<5;index++)
        {
            grpIpAddr.arIP[0] = grpPrefixPtr[index].ipAddr0;
            grpIpAddr.arIP[1] = grpPrefixPtr[index].ipAddr1;
            grpIpAddr.arIP[2] = grpPrefixPtr[index].ipAddr2;
            grpIpAddr.arIP[3] = grpPrefixPtr[index].ipAddr3;

            srcIpAddr.arIP[0] = srcPrefixPtr[index].ipAddr0;
            srcIpAddr.arIP[1] = srcPrefixPtr[index].ipAddr1;
            srcIpAddr.arIP[2] = srcPrefixPtr[index].ipAddr2;
            srcIpAddr.arIP[3] = srcPrefixPtr[index].ipAddr3;

            st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId,
                                             &grpIpAddr,grpPrefixPtr[index].prefix,
                                             &srcIpAddr,srcPrefixPtr[index].prefix,
                                             &mcRouteLttEntry,
                                             GT_FALSE,
                                             GT_FALSE);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmIpv4UcPrefixAdd:tcamSize = %d\n",tcamSize);
            st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
            if (st!=GT_OK)
            {
                cpssOsPrintf("Add fail in grp %d.%d.%d.%d/%d, src %d.%d.%d.%d/%d\n",
                             grpIpAddr.arIP[0],grpIpAddr.arIP[1],grpIpAddr.arIP[2],grpIpAddr.arIP[3],grpPrefixPtr[index].prefix,
                             srcIpAddr.arIP[0],srcIpAddr.arIP[1],srcIpAddr.arIP[2],srcIpAddr.arIP[3],srcPrefixPtr[index].prefix);
                st = cpssDxChIpPatTriePrint(lpmDBId,vrId,0,1);
                if (st !=GT_OK)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                    return;
                }

                st = dumpRouteTcam(GT_TRUE);
                if (st !=GT_OK)
                {
                    PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                    return;
                }
                return;
            }
        }

        st = cpssDxChIpPatTriePrint(lpmDBId,vrId,0,1);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        st = dumpRouteTcam(GT_TRUE);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        /*delete group prefixes 225.1.1.1/[28,26,24] with Src 1.1.1.1/[28,26,24] */
        for (index=0;index<5; index++)
        {
            /* we do not want to delete group prefixes 225.1.1.1/[32,30] with Src 1.1.1.1/[32,30]*/
            if ((index==0)||(index==1))
            {
                continue;
            }
            grpIpAddr.arIP[0] = grpPrefixPtr[index].ipAddr0;
            grpIpAddr.arIP[1] = grpPrefixPtr[index].ipAddr1;
            grpIpAddr.arIP[2] = grpPrefixPtr[index].ipAddr2;
            grpIpAddr.arIP[3] = grpPrefixPtr[index].ipAddr3;

            srcIpAddr.arIP[0] = srcPrefixPtr[index].ipAddr0;
            srcIpAddr.arIP[1] = srcPrefixPtr[index].ipAddr1;
            srcIpAddr.arIP[2] = srcPrefixPtr[index].ipAddr2;
            srcIpAddr.arIP[3] = srcPrefixPtr[index].ipAddr3;

            st = cpssDxChIpLpmIpv4McEntryDel(lpmDBId, vrId, grpIpAddr,grpPrefixPtr[index].prefix,srcIpAddr,srcPrefixPtr[index].prefix);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmIpv4UcPrefixAdd:tcamSize = %d\n",tcamSize);
            st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
            if (st!=GT_OK)
            {
                cpssOsPrintf("Delete fail in grp %d.%d.%d.%d/%d, src %d.%d.%d.%d/%d\n",
                             grpIpAddr.arIP[0],grpIpAddr.arIP[1],grpIpAddr.arIP[2],grpIpAddr.arIP[3],grpPrefixPtr[index].prefix,
                             srcIpAddr.arIP[0],srcIpAddr.arIP[1],srcIpAddr.arIP[2],srcIpAddr.arIP[3],srcPrefixPtr[index].prefix);
                st = cpssDxChIpPatTriePrint(lpmDBId,vrId,0,1);
                if (st !=GT_OK)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                    return;
                }

                st = dumpRouteTcam(GT_TRUE);
                if (st !=GT_OK)
                {
                    PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
                    return;
                }
                return;
            }
        }

        cpssOsPrintf("before reduce\n");
        st = cpssDxChIpPatTriePrint(lpmDBId,vrId,0,1);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        st = dumpRouteTcam(GT_TRUE);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        /* reduce the tcam from 3 lines to 2 lines -->
           225.1.1.0/30 should be moved from (row1,column1) to (row0,column2)
           1.1.1.0/30 should be moved from (row1,column2) to (row0,column3)
           224.0.0.0/4 should be moved from (row2,column3) to (row1,column0) */
        tcamSize -= reduceResizeDelta;
        indexesRange.firstIndex = 0;
        indexesRange.lastIndex = (tcamSize+3)/4-1;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = tcamSize/2;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes= 0;
        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = tcamSize/2;
        st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, &indexesRange, &tcamLpmManagerCapcityCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDBCapacityUpdate - resize-: tcamSize = %d\n",tcamSize);
        st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after cpssDxChIpLpmDBCapacityUpdate - resize+: tcamSize = %d\n",tcamSize);

        tcamRange.firstLine = 0;
        tcamRange.lastLine = indexesRange.lastIndex;
        st =  cpssDxChTcamManagerRangeUpdate(tcamManagerHandlerPtr,
                                             &tcamRange,
                                             CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChTcamManagerRangeUpdate - resize-: tcamSize = %d\n",tcamSize);
        st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after cpssDxChTcamManagerRangeUpdate - resize-: tcamSize = %d\n",tcamSize);
        cpssOsPrintf("after reduce\n");
        st = cpssDxChIpPatTriePrint(lpmDBId,vrId,0,1);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        st = dumpRouteTcam(GT_TRUE);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        /* restore configuration */
        /*1.6. delete all ipv4 prefixes  Expected: GT_OK. */
        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmIpv4McEntriesFlush: tcamSize = %d\n",tcamSize);

        st = cpssDxChIpPatTriePrint(lpmDBId,vrId,0,1);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("cpssDxChIpPatTriePrint\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        st = dumpRouteTcam(GT_TRUE);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG0_MAC("dumpRouteTcam\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfIpValidityCheck");
            return;
        }

        st = cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChIpLpmDbgPatTrieValidityCheck after Flush:tcamSize = %d\n",tcamSize);

        /* 1.7. Delete virtual router vrId[0]. Expected: GT_OK.   */
        st = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d",lpmDBId, vrId);

        /* 1.8. Delete lpmDBId[1] . Expected: GT_OK. */
        st = cpssDxChIpLpmDBDelete(lpmDBId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);
    }
#endif
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmDBCapacityUpdate
(
    IN GT_U32                                       lpmDBId,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
)
*/
#if 0
UTF_TEST_CASE_MAC(cpssDxChIpLpmDBCapacityUpdate)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call cpssDxChIpLpmDBCreate to create default LPM DB configuration.
         See cpssDxChIpLpmDBCreate description.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [1]
        and indexesRange.firstIndex = 200;
            indexesRange.lastIndex = 1000;
            tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = 11;
            tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 22;
            tcamLpmManagerCapcityCfg.numOfIpv6Prefixes = 33;
    Expected: GT_OK for PBR and non-PBR modes (fit in range).
    1.3. Call cpssDxChIpLpmDBCapacityGet with the same params.
    Expected: GT_OK and the same values.
    1.4. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [1]
        and indexesRange.firstIndex = 0;
            indexesRange.lastIndex = 10;
            tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = 11;
            tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 22;
            tcamLpmManagerCapcityCfg.numOfIpv6Prefixes = 33;
    Expected: GT_OK for PBR and NOT GT_OK for non-PBR mode (not fit in range).
    1.5. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [0]
        and indexesRange.firstIndex = 0;
            indexesRange.lastIndex  = 5;
            tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = 11;
            tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 22;
            tcamLpmManagerCapcityCfg.numOfIpv6Prefixes = 33;
    Expected: NOT GT_OK in PBR and non-PBR modes (sum of prefixes not fit in range).
    1.6. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [0]
        and indexesRange.firstIndex = 10;
            indexesRange.lastIndex  = 0;
    Expected: NOT GT_OK (firstIndex should be less than last).
    1.7. Call with not valid lpmDBId [10].
    Expected: NOT GT_OK.
    1.8. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [0] and
        wrong indexesRangePtr [NULL].
    Expected: GT_BAD_PTR.
    1.9. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [0] and
        wrong tcamLpmManagerCapcityCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st  = GT_OK;
    GT_U8      dev;

    GT_U32     lpmDBId = 0;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC tcamLpmManagerCapcityCfg;

    GT_BOOL                                      partitionEnableGet;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRangeGet;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC tcamLpmManagerCapcityCfgGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChIpLpmDBCreate to create default LPM DB configuration.
                 See cpssDxChIpLpmDBCreate description.
            Expected: GT_OK.
        */
        st = prvUtfCreateDefaultLpmDB(dev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvUtfCreateDefaultLpmDB");

        /*
            1.2. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [1]
                and indexesRange.firstIndex = 0;
                    indexesRange.lastIndex = 1000;
                    tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = 11;
                    tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 22;
                    tcamLpmManagerCapcityCfg.numOfIpv6Prefixes = 33;
            Expected: GT_OK for PBR and non-PBR modes (fit in range).
        */
        lpmDBId = 1;

        indexesRange.firstIndex = 200;
        indexesRange.lastIndex = 1000;

        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = 11;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 22;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes = 33;

        st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, &indexesRange, &tcamLpmManagerCapcityCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        if(GT_OK == st)
        {
            /*
                1.3. Call cpssDxChIpLpmDBCapacityGet with the same params.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChIpLpmDBCapacityGet(lpmDBId, &partitionEnableGet,
                                            &indexesRangeGet, &tcamLpmManagerCapcityCfgGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

            /*check partitionEnable*/
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, partitionEnableGet,
                   "partitionEnable not equal GT_TRUE, %d", partitionEnableGet);

            /*check indexesRange.firstIndex*/
            UTF_VERIFY_EQUAL2_STRING_MAC(indexesRange.firstIndex, indexesRangeGet.firstIndex,
                                          "indexesRange.firstIndex were not updated, %d, %d",
                                         indexesRange.firstIndex, indexesRangeGet.firstIndex);

            /*check indexesRange.lastIndex*/
            UTF_VERIFY_EQUAL2_STRING_MAC(indexesRange.lastIndex, indexesRangeGet.lastIndex,
                                          "indexesRange.lastIndex were not updated, %d, %d",
                                         indexesRange.lastIndex, indexesRangeGet.lastIndex);

            /*check tcamLpmManagerCapcityCfg.numOfIpv4Prefixes*/
            UTF_VERIFY_EQUAL2_STRING_MAC(tcamLpmManagerCapcityCfg.numOfIpv4Prefixes,
                                         tcamLpmManagerCapcityCfgGet.numOfIpv4Prefixes,
                   "tcamLpmManagerCapcityCfg.numOfIpv4Prefixes were not updated, %d, %d",
                                         tcamLpmManagerCapcityCfg.numOfIpv4Prefixes,
                                         tcamLpmManagerCapcityCfgGet.numOfIpv4Prefixes);

            /*check tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes*/
            UTF_VERIFY_EQUAL2_STRING_MAC(tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes,
                                         tcamLpmManagerCapcityCfgGet.numOfIpv4McSourcePrefixes,
                   "tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes were not updated, %d, %d",
                                         tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes,
                                         tcamLpmManagerCapcityCfgGet.numOfIpv4McSourcePrefixes);

            /*check tcamLpmManagerCapcityCfg.numOfIpv6Prefixes*/
            UTF_VERIFY_EQUAL2_STRING_MAC(tcamLpmManagerCapcityCfg.numOfIpv6Prefixes,
                                         tcamLpmManagerCapcityCfgGet.numOfIpv6Prefixes,
                   "tcamLpmManagerCapcityCfg.numOfIpv6Prefixes were not updated, %d, %d",
                                         tcamLpmManagerCapcityCfg.numOfIpv6Prefixes,
                                         tcamLpmManagerCapcityCfgGet.numOfIpv6Prefixes);
        }

        /*
            1.4. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [1]
                and indexesRange.firstIndex = 0;
                    indexesRange.lastIndex = 10;
                    tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = 11;
                    tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 22;
                    tcamLpmManagerCapcityCfg.numOfIpv6Prefixes = 33;
            Expected: GT_OK for PBR and NOT GT_OK for non-PBR mode (not fit in range).
        */
        lpmDBId = 1;
        indexesRange.firstIndex = 0;
        indexesRange.lastIndex = 10;
        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = 11;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 22;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes = 33;

        st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, &indexesRange, &tcamLpmManagerCapcityCfg);

        if( prvUtfIsPbrModeUsed() )
        {
            /* 11 lines * 4 entries >= 11 + 33  */
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);
        }
        else
        {
            /* 11 lines * 4 entries <= 11 + (33 * 4)  */
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);
        }

        /*
            1.5. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [0]
                and indexesRange.firstIndex = 0;
                    indexesRange.lastIndex  = 5;
                    tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = 11;
                    tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 22;
                    tcamLpmManagerCapcityCfg.numOfIpv6Prefixes = 33;
            Expected: NOT GT_OK in PBR and non-PBR modes (sum of prefixes not fit in range).
        */
        indexesRange.lastIndex = 5;

        st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, &indexesRange, &tcamLpmManagerCapcityCfg);

        /* 6 lines * 4 entries < 11 + 33  */
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        indexesRange.lastIndex = 1000;

        /*
            1.6. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [0]
                and indexesRange.firstIndex = 10;
                    indexesRange.lastIndex  = 0;
            Expected: NOT GT_OK (firstIndex should be less than last).
        */
        indexesRange.firstIndex = 10;
        indexesRange.lastIndex  = 0;

        st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, &indexesRange, &tcamLpmManagerCapcityCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        indexesRange.firstIndex = 0;
        indexesRange.lastIndex  = 1000;

        /*
            1.7. Call with not valid lpmDBId [10].
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, &indexesRange, &tcamLpmManagerCapcityCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.8. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [0] and
                wrong indexesRangePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, NULL, &tcamLpmManagerCapcityCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, indexesRangePtr = NULL", lpmDBId);

        /*
            1.9. Call cpssDxChIpLpmDBCapacityUpdate with lpmDBId [0] and
                wrong tcamLpmManagerCapcityCfgPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmDBCapacityUpdate(lpmDBId, &indexesRange, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, tcamLpmManagerCapcityCfgPtr = NULL", lpmDBId);
    }
}
#endif

UTF_TEST_CASE_MAC(cpssDxChIpLpmActivityBitEnableGet)
{
    /*
    ITERATE_DEVICES (BobK and above)
    1.1. Call cpssDxChIpLpmActivityBitEnableGet to get default LPM Activity Bit configuration.
    Expected: GT_OK.
    */

    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_BOOL     activityBit;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
        1.1. Call cpssDxChIpLpmActivityBitEnableGet to get default LPM Activity Bit configuration.
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmActivityBitEnableGet(dev, &activityBit);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmActivityBitEnableGet: %d", dev);
    }
}

UTF_TEST_CASE_MAC(cpssDxChIpLpmActivityBitEnableSet)
{
    /*
    ITERATE_DEVICES (BobK and above)
    1.1. Call cpssDxChIpLpmActivityBitEnableGet to get default LPM Activity Bit configuration.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmActivityBitEnableSet to set LPM Activity Bit to True.
    Expected: GT_OK.
    1.3. Call cpssDxChIpLpmActivityBitEnableSet to set LPM Activity Bit to False.
    Expected: GT_OK.
    1.4. Call cpssDxChIpLpmActivityBitEnableSet to restore saved LPM Activity Bit.
    Expected: GT_OK.
    */

    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_BOOL     activityBit;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
        1.1. Call cpssDxChIpLpmActivityBitEnableGet to get default LPM Activity Bit configuration.
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmActivityBitEnableGet(dev, &activityBit);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmActivityBitEnableGet: %d", dev);

        /*
        1.2. Call cpssDxChIpLpmActivityBitEnableSet to set LPM Activity Bit to True.
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmActivityBitEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmActivityBitEnableSet: %d", dev);

        /*
        1.3. Call cpssDxChIpLpmActivityBitEnableSet to set LPM Activity Bit to False.
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmActivityBitEnableSet(dev, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmActivityBitEnableSet: %d", dev);

        /*
        1.3. Call cpssDxChIpLpmActivityBitEnableSet to restore saved LPM Activity Bit.
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmActivityBitEnableSet(dev, activityBit);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpLpmActivityBitEnableSet: %d", dev);
    }
}

UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4UcPrefixActivityStatusGet)
{
    /*
    ITERATE_DEVICES (BobK and above)
    1.1. Create default LPM configuration and add virtual routers
         with prvUtfCreateLpmDBAndVirtualRouterAdd.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1], prefixLen[32],
            nextHopInfoPtr->ipLttEntry{
                routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E],
                numOfPaths[0],
                routeEntryBaseIndex[0],
                ucRPFCheckEnable[GT_FALSE],
                sipSaCheckMismatchEnable[GT_FALSE],
                ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]
            }
    Expected: GT_OK.
    1.3 Call cpssDxChIpLpmIpv4UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
            prefixLen[32], clearActivity[GT_FALSE].
    Expected: GT_OK.
    1.4 Call cpssDxChIpLpmIpv4UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.2],
            prefixLen[32], clearActivity[GT_FALSE].
    Expected: GT_NOT_FOUND.
    1.5 Call cpssDxChIpLpmIpv4UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddrPtr[NULL],
            prefixLen[32], clearActivity[GT_FALSE].
    Expected: GT_BAD_PTR.
    1.6 Call cpssDxChIpLpmIpv4UcPrefixActivityStatusGet with lpmDBId [0], vrId[10], ipAddr[10.15.0.1],
            prefixLen[32], clearActivity[GT_FALSE].
    Expected: GT_NOT_FOUND.
    1.7 Call cpssDxChIpLpmIpv4UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
            prefixLen[22], clearActivity[GT_FALSE].
    Expected: GT_NOT_SUPPORTED.
    1.8 Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
    */

    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_BOOL     activityStatus;
    GT_U32                                 lpmDBId     = 0;
    GT_U32                                 vrId        = 0;
    GT_IPADDR                              ipAddr;
    GT_U32                                 prefixLen   = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override    = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Create default LPM configuration and add virtual routers
                 with prvUtfCreateLpmDBAndVirtualRouterAdd.
            Expected: GT_OK.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.2. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1], prefixLen[32],
                 nextHopInfoPtr->ipLttEntry{
                        routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E],
                        numOfPaths[0],
                        routeEntryBaseIndex[0],
                        ucRPFCheckEnable[GT_FALSE],
                        sipSaCheckMismatchEnable[GT_FALSE],
                        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]
                    }
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.arIP[0] = 10;
        ipAddr.arIP[1] = 15;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 1;

        prefixLen = 32;

        prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

        override  = GT_TRUE;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, %d, override = %d", lpmDBId, vrId, prefixLen, override);

        /*
        1.3 Call cpssDxChIpLpmIpv4UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
                prefixLen[32], clearActivity[GT_FALSE].
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmIpv4UcPrefixActivityStatusGet(lpmDBId, vrId, &ipAddr, prefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, %d", lpmDBId, vrId, prefixLen);

        /*
        1.4 Call cpssDxChIpLpmIpv4UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.2],
                prefixLen[32], clearActivity[GT_FALSE].
        Expected: GT_NOT_FOUND.
        */
        ipAddr.arIP[3] = 2;
        st = cpssDxChIpLpmIpv4UcPrefixActivityStatusGet(lpmDBId, vrId, &ipAddr, prefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_FOUND, st, "%d, %d, %d", lpmDBId, vrId, prefixLen);
        ipAddr.arIP[3] = 1;
        /*
        1.5 Call cpssDxChIpLpmIpv4UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddrPtr[NULL],
                prefixLen[32], clearActivity[GT_FALSE].
        Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv4UcPrefixActivityStatusGet(lpmDBId, vrId, NULL, prefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, %d", lpmDBId, vrId, prefixLen);

        /*
        1.6 Call cpssDxChIpLpmIpv4UcPrefixActivityStatusGet with lpmDBId [0], vrId[10], ipAddr[10.15.0.1],
                prefixLen[32], clearActivity[GT_FALSE].
        Expected: GT_NOT_FOUND.
        */
        vrId = 10;
        st = cpssDxChIpLpmIpv4UcPrefixActivityStatusGet(lpmDBId, vrId, &ipAddr, prefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_FOUND, st, "%d, %d, %d", lpmDBId, vrId, prefixLen);
        vrId = 0;

        /*
        1.7 Call cpssDxChIpLpmIpv4UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
                prefixLen[22], clearActivity[GT_FALSE].
        Expected: GT_NOT_SUPPORTED.
        */
        prefixLen = 22;
        st = cpssDxChIpLpmIpv4UcPrefixActivityStatusGet(lpmDBId, vrId, &ipAddr, prefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_SUPPORTED, st, "%d, %d, %d", lpmDBId, vrId, prefixLen);
        prefixLen = 32;

        /*
        1.8 Call cpssDxChIpLpmIpv4UcPrefixesFlush with
              lpmDBId[0] and vrId[0] to invalidate changes.
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6UcPrefixActivityStatusGet)
{
    /*
    ITERATE_DEVICES (BobK and above)
    1.1. Create default LPM configuration and add virtual routers
         with prvUtfCreateLpmDBAndVirtualRouterAdd.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmIpv6UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1], prefixLen[128],
            nextHopInfoPtr->ipLttEntry{
                routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E],
                numOfPaths[0],
                routeEntryBaseIndex[0],
                ucRPFCheckEnable[GT_FALSE],
                sipSaCheckMismatchEnable[GT_FALSE],
                ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]
            }
    Expected: GT_OK.
    1.3 Call cpssDxChIpLpmIpv6UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
            prefixLen[128], clearActivity[GT_FALSE].
    Expected: GT_OK.
    1.4 Call cpssDxChIpLpmIpv6UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.2],
            prefixLen[128], clearActivity[GT_FALSE].
    Expected: GT_NOT_FOUND.
    1.5 Call cpssDxChIpLpmIpv6UcPrefixActivityStatusGet with lpmDBId [0], vrId[10], ipAddr[10.15.0.1],
            prefixLen[128], clearActivity[GT_FALSE].
    Expected: GT_NOT_FOUND.
    1.6 Call cpssDxChIpLpmIpv6UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
            prefixLen[22], clearActivity[GT_FALSE].
    Expected: GT_NOT_SUPPORTED.
    1.7 Call cpssDxChIpLpmIpv6UcPrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
    */

    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_BOOL                                 activityStatus;
    GT_U32                                  lpmDBId      = 0;
    GT_U32                                  vrId         = 0;
    GT_IPV6ADDR                             ipAddr;
    GT_U32                                  prefixLen    = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override     = GT_FALSE;
    GT_BOOL                                 defragEnable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Create default LPM configuration and add virtual routers
                 with prvUtfCreateLpmDBAndVirtualRouterAdd.
            Expected: GT_OK.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.2. Call cpssDxChIpLpmIpv6UcPrefixAdd with lpmDBId [0], vrId[0], ipAddr[10.15.0.1], prefixLen[128],
                 nextHopInfoPtr->ipLttEntry{
                        routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E],
                        numOfPaths[0],
                        routeEntryBaseIndex[0],
                        ucRPFCheckEnable[GT_FALSE],
                        sipSaCheckMismatchEnable[GT_FALSE],
                        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]
                    }, defragEnable[GT_FALSE];
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipAddr.u32Ip[0] = 10;
        ipAddr.u32Ip[1] = 15;
        ipAddr.u32Ip[2] = 0;
        ipAddr.u32Ip[3] = 1;

        prefixLen = 128;

        prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

        override  = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &nextHopInfo, override, defragEnable);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, override = %d, defragmentationEnable = %d",
                                     lpmDBId, vrId, prefixLen, override, defragEnable);

        /*
        1.3 Call cpssDxChIpLpmIpv6UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
                prefixLen[128], clearActivity[GT_FALSE].
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmIpv6UcPrefixActivityStatusGet(lpmDBId, vrId, &ipAddr, prefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, %d", lpmDBId, vrId, prefixLen);

        /*
        1.4 Call cpssDxChIpLpmIpv6UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.2],
                prefixLen[128], clearActivity[GT_FALSE].
        Expected: GT_NOT_FOUND.
        */
        ipAddr.arIP[3] = 2;
        st = cpssDxChIpLpmIpv6UcPrefixActivityStatusGet(lpmDBId, vrId, &ipAddr, prefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_FOUND, st, "%d, %d, %d", lpmDBId, vrId, prefixLen);
        ipAddr.arIP[3] = 1;

        /*
        1.5 Call cpssDxChIpLpmIpv6UcPrefixActivityStatusGet with lpmDBId [0], vrId[10], ipAddr[10.15.0.1],
                prefixLen[128], clearActivity[GT_FALSE].
        Expected: GT_NOT_FOUND.
        */
        vrId = 10;
        st = cpssDxChIpLpmIpv6UcPrefixActivityStatusGet(lpmDBId, vrId, &ipAddr, prefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_FOUND, st, "%d, %d, %d", lpmDBId, vrId, prefixLen);
        vrId = 0;

        /*
        1.6 Call cpssDxChIpLpmIpv6UcPrefixActivityStatusGet with lpmDBId [0], vrId[0], ipAddr[10.15.0.1],
                prefixLen[22], clearActivity[GT_FALSE].
        Expected: GT_NOT_SUPPORTED.
        */
        prefixLen = 22;
        st = cpssDxChIpLpmIpv6UcPrefixActivityStatusGet(lpmDBId, vrId, &ipAddr, prefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_SUPPORTED, st, "%d, %d, %d", lpmDBId, vrId, prefixLen);
        prefixLen = 128;

        /*
        1.7 Call cpssDxChIpLpmIpv6UcPrefixesFlush with
              lpmDBId[0] and vrId[0] to invalidate changes.
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv4McEntryActivityStatusGet)
{
    /*
    ITERATE_DEVICES (BobK and above)
    1.1. Create default LPM configuration and add virtual routers
         with prvUtfCreateLpmDBAndVirtualRouterAdd.
    Expected: GT_OK.
    1.2 Call cpssDxChIpLpmIpv4McEntryAdd with lpmDBId[0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
            ipSrc[10.15.0.1], ipSrcPrefixLen[32],
            mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
                routeEntryBaseIndex[0], ucRPFCheckEnable[GT_FALSE],
                sipSaCheckMismatchEnable[GT_FALSE],
                ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
                defragmentationEnable[GT_FALSE].
    Expected: GT_OK.
    1.3 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
            ipSrc[10.15.0.1], ipSrcPrefixLen[32], clearActivity[GT_FALSE].
    Expected: GT_OK.
    1.4 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
            ipSrc[10.15.0.2], ipSrcPrefixLen[32], clearActivity[GT_FALSE].
    Expected: GT_NOT_FOUND.
    1.5 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.2], ipGroupPrefixLen[32],
            ipSrc[10.15.0.1], ipSrcPrefixLen[32], clearActivity[GT_FALSE].
    Expected: GT_NOT_FOUND.
    1.6 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
            ipSrc[10.15.0.1], ipSrcPrefixLen[22], clearActivity[GT_FALSE].
    Expected: GT_NOT_SUPPORTED.
    1.7 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[22],
            ipSrc[10.15.0.1], ipSrcPrefixLen[32], clearActivity[GT_FALSE].
    Expected: GT_NOT_SUPPORTED.
    1.8 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[10], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
            ipSrc[10.15.0.1], ipSrcPrefixLen[32], clearActivity[GT_FALSE].
    Expected: GT_NOT_FOUND.
    1.9 Call cpssDxChIpLpmIpv4McEntriesFlush with
          lpmDBId[0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
    */

    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_BOOL                     activityStatus;
    GT_U32                      lpmDBId          = 0;
    GT_U32                      vrId             = 0;
    GT_IPADDR                   ipGroup;
    GT_U32                      ipGroupPrefixLen = 0;
    GT_IPADDR                   ipSrc;
    GT_U32                      ipSrcPrefixLen   = 0;
    CPSS_DXCH_IP_LTT_ENTRY_STC  mcRouteLttEntry;
    GT_BOOL                     override         = GT_FALSE;
    GT_BOOL                     defragEnable     = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E);

    cpssOsBzero((GT_VOID*)&mcRouteLttEntry, sizeof(mcRouteLttEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Create default LPM configuration and add virtual routers
                 with prvUtfCreateLpmDBAndVirtualRouterAdd.
            Expected: GT_OK.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.2 Call cpssDxChIpLpmIpv4McEntryAdd with lpmDBId[0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
                    ipSrc[10.15.0.1], ipSrcPrefixLen[32],
                    mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
                        routeEntryBaseIndex[0], ucRPFCheckEnable[GT_FALSE],
                        sipSaCheckMismatchEnable[GT_FALSE],
                        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
                        defragmentationEnable[GT_FALSE].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.arIP[0] = 224;
        ipGroup.arIP[1] = 15;
        ipGroup.arIP[2] = 0;
        ipGroup.arIP[3] = 1;

        ipGroupPrefixLen = 32;

        ipSrc.arIP[0] = 10;
        ipSrc.arIP[1] = 15;
        ipSrc.arIP[2] = 0;
        ipSrc.arIP[3] = 1;

        ipSrcPrefixLen = 32;

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
        }

        /*
        1.3 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
                ipSrc[10.15.0.1], ipSrcPrefixLen[32], clearActivity[GT_FALSE].
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmIpv4McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d", lpmDBId, vrId);

        /*
        1.4 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
                ipSrc[10.15.0.2], ipSrcPrefixLen[32], clearActivity[GT_FALSE].
        Expected: GT_NOT_FOUND.
        */
        ipSrc.arIP[3] = 2;
        st = cpssDxChIpLpmIpv4McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_FOUND, st, "%d, %d", lpmDBId, vrId);
        ipSrc.arIP[3] = 1;

        /*
        1.5 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.2], ipGroupPrefixLen[32],
                ipSrc[10.15.0.1], ipSrcPrefixLen[32], clearActivity[GT_FALSE].
        Expected: GT_NOT_FOUND.
        */
        ipGroup.arIP[3] = 2;
        st = cpssDxChIpLpmIpv4McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_FOUND, st, "%d, %d", lpmDBId, vrId);
        ipGroup.arIP[3] = 1;

        /*
        1.6 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
                ipSrc[10.15.0.1], ipSrcPrefixLen[22], clearActivity[GT_FALSE].
        Expected: GT_NOT_SUPPORTED.
        */
        ipSrcPrefixLen = 22;
        st = cpssDxChIpLpmIpv4McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st, "%d, %d", lpmDBId, vrId);
        ipSrcPrefixLen = 32;

        /*
        1.7 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[22],
                ipSrc[10.15.0.1], ipSrcPrefixLen[32], clearActivity[GT_FALSE].
        Expected: GT_NOT_SUPPORTED.
        */
        ipGroupPrefixLen = 22;
        st = cpssDxChIpLpmIpv4McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st, "%d, %d", lpmDBId, vrId);
        ipGroupPrefixLen = 32;

        /*
        1.8 Call cpssDxChIpLpmIpv4McEntryActivityStatusGet with lpmDBId [0], vrId[10], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
                ipSrc[10.15.0.1], ipSrcPrefixLen[32], clearActivity[GT_FALSE].
        Expected: GT_NOT_FOUND.
        */
        vrId = 10;
        st = cpssDxChIpLpmIpv4McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_FOUND, st, "%d, %d", lpmDBId, vrId);
        vrId = 0;

        /*
        1.9 Call cpssDxChIpLpmIpv4McEntriesFlush with
              lpmDBId[0] and vrId[0] to invalidate changes.
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

UTF_TEST_CASE_MAC(cpssDxChIpLpmIpv6McEntryActivityStatusGet)
{
    /*
    ITERATE_DEVICES (BobK and above)
    1.1. Create default LPM configuration and add virtual routers
         with prvUtfCreateLpmDBAndVirtualRouterAdd.
    Expected: GT_OK.
    1.2 Call cpssDxChIpLpmIpv6McEntryAdd with lpmDBId[0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[128],
            ipSrc[10.15.0.1], ipSrcPrefixLen[128],
            mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
                routeEntryBaseIndex[0], ucRPFCheckEnable[GT_FALSE],
                sipSaCheckMismatchEnable[GT_FALSE],
                ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
                defragmentationEnable[GT_FALSE].
    Expected: GT_OK.
    1.3 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[128],
            ipSrc[10.15.0.1], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
    Expected: GT_OK.
    1.4 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[128],
            ipSrc[10.15.0.2], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
    Expected: GT_NOT_FOUND.
    1.5 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.2], ipGroupPrefixLen[128],
            ipSrc[10.15.0.1], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
    Expected: GT_NOT_FOUND.
    1.6 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[128],
            ipSrc[10.15.0.1], ipSrcPrefixLen[22], clearActivity[GT_FALSE].
    Expected: GT_NOT_SUPPORTED.
    1.7 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[22],
            ipSrc[10.15.0.1], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
    Expected: GT_NOT_SUPPORTED.
    1.8 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[10], ipGroup[224.15.0.1], ipGroupPrefixLen[128],
            ipSrc[10.15.0.1], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
    Expected: GT_NOT_FOUND.
    1.9 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroupPtr[NULL], ipGroupPrefixLen[128],
            ipSrc[10.15.0.1], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
    Expected: GT_BAD_PTR.
    1.10 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[128],
            ipSrcPtr[NULL], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
    Expected: GT_BAD_PTR.
    1.11 Call cpssDxChIpLpmIpv6McEntriesFlush with
          lpmDBId[0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
    */

    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_BOOL                     activityStatus;
    GT_U32                      lpmDBId          = 0;
    GT_U32                      vrId             = 0;
    GT_IPV6ADDR                 ipGroup;
    GT_U32                      ipGroupPrefixLen = 0;
    GT_IPV6ADDR                 ipSrc;
    GT_U32                      ipSrcPrefixLen   = 0;
    CPSS_DXCH_IP_LTT_ENTRY_STC  mcRouteLttEntry;
    GT_BOOL                     override         = GT_FALSE;
    GT_BOOL                     defragEnable     = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT3_E | UTF_LION2_E | UTF_BOBCAT2_E);

    cpssOsBzero((GT_VOID*)&mcRouteLttEntry, sizeof(mcRouteLttEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Create default LPM configuration and add virtual routers
                 with prvUtfCreateLpmDBAndVirtualRouterAdd.
            Expected: GT_OK.
        */
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);

        /*
            1.2 Call cpssDxChIpLpmIpv6McEntryAdd with lpmDBId[0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[32],
                    ipSrc[10.15.0.1], ipSrcPrefixLen[32],
                    mcRouteLttEntryPtr{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E], numOfPaths[0],
                        routeEntryBaseIndex[0], ucRPFCheckEnable[GT_FALSE],
                        sipSaCheckMismatchEnable[GT_FALSE],
                        ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}, override[GT_TRUE],
                        defragmentationEnable[GT_FALSE].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        ipGroup.u32Ip[0] = 255;
        ipGroup.u32Ip[1] = 15;
        ipGroup.u32Ip[2] = 0;
        ipGroup.u32Ip[3] = 1;

        ipGroupPrefixLen = 128;

        ipSrc.u32Ip[0] = 10;
        ipSrc.u32Ip[1] = 15;
        ipSrc.u32Ip[2] = 0;
        ipSrc.u32Ip[3] = 1;

        ipSrcPrefixLen = 128;

        prvUtfSetDefaultIpLttEntry(dev, &mcRouteLttEntry);

        override     = GT_TRUE;
        defragEnable = GT_FALSE;

        st = cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc,
                                         ipSrcPrefixLen, &mcRouteLttEntry, override, defragEnable);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, override = %d, defragmentationEnable = %d",
                                             lpmDBId, vrId, override, defragEnable);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, override = %d, defragmentationEnable = %d",
                                         lpmDBId, vrId, override, defragEnable);
        }

        /*
        1.3 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[128],
                ipSrc[10.15.0.1], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmIpv6McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d", lpmDBId, vrId);

        /*
        1.4 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[128],
                ipSrc[10.15.0.2], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
        Expected: GT_NOT_FOUND.
        */
        ipSrc.u32Ip[3] = 2;
        st = cpssDxChIpLpmIpv6McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_FOUND, st, "%d, %d", lpmDBId, vrId);
        ipSrc.u32Ip[3] = 1;

        /*
        1.5 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.2], ipGroupPrefixLen[128],
                ipSrc[10.15.0.1], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
        Expected: GT_NOT_FOUND.
        */
        ipGroup.u32Ip[3] = 2;
        st = cpssDxChIpLpmIpv6McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_FOUND, st, "%d, %d", lpmDBId, vrId);
        ipGroup.u32Ip[3] = 1;

        /*
        1.6 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[128],
                ipSrc[10.15.0.1], ipSrcPrefixLen[22], clearActivity[GT_FALSE].
        Expected: GT_NOT_SUPPORTED.
        */
        ipSrcPrefixLen = 22;
        st = cpssDxChIpLpmIpv6McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st, "%d, %d", lpmDBId, vrId);
        ipSrcPrefixLen = 128;

        /*
        1.7 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[22],
                ipSrc[10.15.0.1], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
        Expected: GT_NOT_SUPPORTED.
        */
        ipGroupPrefixLen = 22;
        st = cpssDxChIpLpmIpv6McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st, "%d, %d", lpmDBId, vrId);
        ipGroupPrefixLen = 128;

        /*
        1.8 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[10], ipGroup[224.15.0.1], ipGroupPrefixLen[128],
                ipSrc[10.15.0.1], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
        Expected: GT_NOT_FOUND.
        */
        vrId = 10;
        st = cpssDxChIpLpmIpv6McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_FOUND, st, "%d, %d", lpmDBId, vrId);
        vrId = 0;

        /*
        1.9 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroupPtr[NULL], ipGroupPrefixLen[128],
                ipSrc[10.15.0.1], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
        Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6McEntryActivityStatusGet(lpmDBId, vrId, NULL, ipGroupPrefixLen, &ipSrc, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d", lpmDBId, vrId);

        /*
        1.10 Call cpssDxChIpLpmIpv6McEntryActivityStatusGet with lpmDBId [0], vrId[0], ipGroup[224.15.0.1], ipGroupPrefixLen[128],
                ipSrcPtr[NULL], ipSrcPrefixLen[128], clearActivity[GT_FALSE].
        Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmIpv6McEntryActivityStatusGet(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen, NULL, ipSrcPrefixLen, GT_FALSE, &activityStatus);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d", lpmDBId, vrId);

        /*
        1.11 Call cpssDxChIpLpmIpv6McEntriesFlush with
              lpmDBId[0] and vrId[0] to invalidate changes.
        Expected: GT_OK.
        */
        st = cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmFcoePrefixAdd
(
    IN GT_U32                                   lpmDBId,
    IN GT_U32                                   vrId,
    IN GT_FCID                                  *fcoeAddrPtr,
    IN GT_U32                                   prefixLen,
    IN CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT   *nextHopInfoPtr,
    IN GT_BOOL                                  override
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmFcoePrefixAdd)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Create default LPM configuration and add virtual routers.
    Expected: GT_OK.
    1.2. Call with lpmDBId [1], vrId[0], ipAddr[10.16.0.1], prefixLen[32],
        nextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
        numOfPaths[0], routeEntryBaseIndex[0], ucRPFCheckEnable[GT_TRUE],
        sipSaCheckMismatchEnable[GT_TRUE], ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK.
    1.3. Call with lpmDBId [1], vrId[0], ipAddr[10.16.0.1], prefixLen[32],
           nextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                      numOfPaths[0],
                                      routeEntryBaseIndex[0],
                                      ucRPFCheckEnable[GT_TRUE],
                                      sipSaCheckMismatchEnable[GT_TRUE],
                                      ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
    Expected: GT_OK
    1.4. Call with override [GT_FALSE] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.5. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.6. Call with not supported vrId [4096] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.7. Call with out of range prefixLen [33] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.8. Call with lpmDBId [1] wrong enum values nextHopInfoPtr->ipLttEntry.routeType
        and other parameters from 1.5.
    Expected: GT_BAD_PARAM.
    1.9. Call with lpmDBId [1] out of range nextHopInfoPtr->ipLttEntry.
        ipv6MCGroupScopeLevel [wrong enum values] and other parameters from 1.5.
    Expected: GT_BAD_PARAM.
    1.10. Call with lpmDBId [1] nextHopInfoPtr[NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
    1.11. Call with lpmDBId [1] fcoeAddrPtr[NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
    1.12. Call with lpmDBId [0] wrong enum values nextHopInfoPtr->pclIpUcAction.pktCmdp
        and other parameters from 1.4.
    Expected: GT_BAD_PARAM.
    1.13. Call cpssDxChIpLpmIpv4UcPrefixesFlush with lpmDBId [0 / 1] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId     = 0;
    GT_U32                                 vrId        = 0;
    GT_U32                                 prefixLen   = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override    = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_FALSE;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT      shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    GT_FCID                                fcoeAddr;
    CPSS_IP_PROTOCOL_STACK_ENT             protocolStack   = CPSS_IP_PROTOCOL_FCOE_E;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT     memoryCfg;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo;
    GT_U32                                 notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Create default LPM configuration and add virtual routers
            Expected: GT_OK.
        */

        /* restore configuration after other UTs */
        /* remove changed virtual routers first */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmVirtualRouterDel(1, 0);
        cpssDxChIpLpmDBDelete(0);
        cpssDxChIpLpmDBDelete(1);

        lpmDBId = 0;
        vrId    = 0;
        st = prvUtfIpLpmRamDefaultConfigCalc(dev, &memoryCfg.ramDbCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, shadowType);

        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &memoryCfg);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                "memoryCfg.ramDbCfg.numOfBlocks = %d,"
                "memoryCfg.ramDbCfg.blocksAllocationMethod = %d",
                lpmDBId, shadowType, protocolStack,
                memoryCfg.ramDbCfg.numOfBlocks,
                memoryCfg.ramDbCfg.blocksAllocationMethod);

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, &dev, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        lpmDBId = 1;
        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &memoryCfg);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                "memoryCfg.ramDbCfg.numOfBlocks = %d,"
                "memoryCfg.ramDbCfg.blocksAllocationMethod = %d",
                lpmDBId, shadowType, protocolStack,
                memoryCfg.ramDbCfg.numOfBlocks,
                memoryCfg.ramDbCfg.blocksAllocationMethod);

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, &dev, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_FALSE;
        vrConfigInfo.supportIpv4Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Uc = GT_FALSE;
        vrConfigInfo.supportFcoe   = GT_TRUE;
        /* Fill defUcNextHopInfoPtr structure */
        prvUtfSetDefaultActionEntry(&vrConfigInfo.defaultFcoeForwardingNextHopInfo.pclIpUcAction);

        /* add virtual router for 0 db*/
        lpmDBId = 0;
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* add virtual router for 1 db*/
        lpmDBId = 1;
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.2. Call with lpmDBId [0], vrId[0], fcoeAddr[0x112233], prefixLen[24],
                   nextHopInfoPtr->pclIpUcAction{pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                             mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                    mirrorToRxAnalyzerPort = GT_FALSE},
                             matchCounter{enableMatchCount = GT_FALSE,
                                          matchCounterIndex = 0},
                             qos{egressPolicy = GT_FALSE, modifyDscp = GT_FALSE, modifyUp = GT_FALSE,
                                 qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                                        profilePrecedence=GT_FALSE] ] },
                             redirect {CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                             data[routerLttIndex=0] },
                             policer { policerEnable=GT_FALSE, policerId=0 },
                             vlan {egressTaggedModify=GT_FALSE,
                                   modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                   nestedVlan=GT_FALSE, vlanId=100,
                                   precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                              ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                           override [GT_TRUE].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x33;

        prefixLen = 24;

        prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

        override  = GT_TRUE;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, %d, override = %d",
                                        lpmDBId, vrId, prefixLen, override);

        /*
            1.3. Call with lpmDBId [1], vrId[0], fcoeAddr[0x112233], prefixLen[24],
                   nextHopInfoPtr->ipLttEntry{routeType[CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E],
                                              numOfPaths[0],
                                              routeEntryBaseIndex[0],
                                              ucRPFCheckEnable[GT_TRUE],
                                              sipSaCheckMismatchEnable[GT_TRUE],
                                              ipv6MCGroupScopeLevel[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E]}
            Expected: GT_OK
        */

        lpmDBId = 1;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, %d, override = %d",
                                        lpmDBId, vrId, prefixLen, override);

        /*
            1.4. Call with override [GT_FALSE] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        override = GT_FALSE;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, override = %d", lpmDBId, vrId, override);

        override = GT_TRUE;

        /*
            1.5. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.6. Call with not supported vrId [4096] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.7. Call with out of range prefixLen [33] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        prefixLen = 33;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d", lpmDBId, prefixLen);

        prefixLen = 24;

        /*
            1.8. Call with lpmDBId [1]
                            wrong enum values nextHopInfoPtr->ipLttEntry.routeType
                            and other parameters from 1.5.
            Expected: GT_BAD_PARAM.
        */
        lpmDBId = 1;

        UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmFcoePrefixAdd
                            (lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable),
                            nextHopInfo.ipLttEntry.routeType);

        /*
            1.9. Call with lpmDBId [1]
                             wrong enum values nextHopInfoPtr->ipLttEntry.ipv6MCGroupScopeLevel
                             and other parameters from 1.5.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpLpmFcoePrefixAdd
                            (lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable),
                            nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel);

        /*
            1.10. Call with lpmDBId [1] nextHopInfoPtr[NULL] and other parameters from 1.5.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, NULL, override, defragmentationEnable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);

        /*
            1.11. Call with lpmDBId [1] fcoeAddrPtr[NULL] and other parameters from 1.5.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, NULL, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, %d, fcoeAddrPtr = NULL", lpmDBId, vrId);

        /*
            1.12. Call cpssDxChIpLpmFcoePrefixDel with
                  lpmDBId [0 / 1] and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId[0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmFcoePrefixDel(lpmDBId , vrId, &fcoeAddr, prefixLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Call with lpmDBId[1] */
        lpmDBId = 1;

        st = cpssDxChIpLpmFcoePrefixDel(lpmDBId , vrId, &fcoeAddr, prefixLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmVirtualRouterDel(1, 0);
        cpssDxChIpLpmDBDelete(0);
        cpssDxChIpLpmDBDelete(1);
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmFcoePrefixSearch
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_FCID                                 *fcoeAddrPtr,
    IN  GT_U32                                  prefixLen,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr

)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmFcoePrefixSearch)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Create default LPM configuration and add virtual routers.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmFcoePrefixAdd with lpmDBId [0], vrId[0], ipAddr[0x112233],
        prefixLen[24], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.3. Call with lpmDBId[0], vrId[0], ipAddr[0x112233], prefixLen[24] and non-null nextHopInfoPtr
    Expected: GT_OK and the same nextHopInfoPtr.
    1.4. Call with lpmDBId[0], vrId[0], ipAddr[0x112233], prefixLen[16] and non-null nextHopInfoPtr
    Expected: NOT GT_OK.
    1.5. Call with lpmDBId[0], vrId[0], ipAddr[0x1122FF], prefixLen[16] and non-null nextHopInfoPtr
    Expected: NOT GT_OK.
    1.6. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.7. Call with not supported vrId [4096] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.8. Call with out of range prefixLen [33] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.9. Call with lpmDBId[0], vrId[0], ipAddr[0x112233], prefixLen[24] and nextHopInfoPtr[NULL].
    Expected: GT_BAD_PTR.
    1.10. Call with lpmDBId[0], vrId[0], fcoeAddrPtr[NULL], prefixLen[24] and and non-null nextHopInfoPtr.
    Expected: GT_BAD_PTR.
    1.11. Call cpssDxChIpLpmFcoePrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId     = 0;
    GT_U32                                 vrId        = 0;
    GT_U32                                 prefixLen   = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfoGet;
    GT_BOOL                                override    = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_FALSE;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT      shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    GT_FCID                                fcoeAddr;
    CPSS_IP_PROTOCOL_STACK_ENT             protocolStack   = CPSS_IP_PROTOCOL_FCOE_E;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT     memoryCfg;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo;
    GT_U32                                 notAppFamilyBmp;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&nextHopInfoGet, sizeof(nextHopInfoGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1
        */
        /* restore configuration after other UTs */
        /* remove changed virtual routers first */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);

        lpmDBId = 0;
        vrId    = 0;
        st = prvUtfIpLpmRamDefaultConfigCalc(dev, &memoryCfg.ramDbCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, shadowType);

        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &memoryCfg);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                "memoryCfg.ramDbCfg.numOfBlocks = %d,"
                "memoryCfg.ramDbCfg.blocksAllocationMethod = %d",
                lpmDBId, shadowType, protocolStack,
                memoryCfg.ramDbCfg.numOfBlocks,
                memoryCfg.ramDbCfg.blocksAllocationMethod);

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, &dev, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_FALSE;
        vrConfigInfo.supportIpv4Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Uc = GT_FALSE;
        vrConfigInfo.supportFcoe   = GT_TRUE;
        /* Fill defUcNextHopInfoPtr structure */
        prvUtfSetDefaultActionEntry(&vrConfigInfo.defaultFcoeForwardingNextHopInfo.pclIpUcAction);

        /* add virtual router for 0 db*/
        lpmDBId = 0;
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.2. Call cpssDxChIpLpmIpv4UcPrefixAdd with lpmDBId [0], vrId[0], fcoeAddr[0x112233],
                      prefixLen[24], nextHopInfoPtr->pclIpUcAction { pktCmd =
                      CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                      CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                      matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos
                      { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                      ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                      profilePrecedence=GT_FALSE] ] }, redirect {
                      CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                      policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                      egressTaggedModify=GT_FALSE,
                      modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                      vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                      ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                      override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x33;

        prefixLen = 24;

        prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

        override = GT_TRUE;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmFcoePrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.3. Call with lpmDBId[0], vrId[0], ipAddr[0x112233], prefixLen[24] and non-null nextHopInfoPtr
            Expected: GT_OK and the same nextHopInfoPtr.
        */
        st = cpssDxChIpLpmFcoePrefixSearch(lpmDBId, vrId, &fcoeAddr, prefixLen,
                                             &nextHopInfoGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLen);

        prvUtfCompareIpLttEntries(lpmDBId, vrId, &nextHopInfoGet.ipLttEntry, &nextHopInfo.ipLttEntry);

        /*
            1.4. Call with lpmDBId[0], vrId[0], ipAddr[0x112233], prefixLen[16] and non-null nextHopInfoPtr
            Expected: NOT GT_OK.
        */
        prefixLen = 16;

        st = cpssDxChIpLpmFcoePrefixSearch(lpmDBId, vrId, &fcoeAddr, prefixLen,
                                             &nextHopInfoGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLen);

        /*
            1.5. Call with lpmDBId[0], vrId[0], ipAddr[0x1122FF], prefixLen[16] and non-null nextHopInfoPtr
            Expected: NOT GT_OK.
        */
        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0xFF;

        prefixLen = 16;

        st = cpssDxChIpLpmFcoePrefixSearch(lpmDBId, vrId, &fcoeAddr, prefixLen,
                                             &nextHopInfoGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLen);

        /*
            1.6. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x33;

        prefixLen = 24;

        st = cpssDxChIpLpmFcoePrefixSearch(lpmDBId, vrId, &fcoeAddr, prefixLen,
                                             &nextHopInfoGet);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.7. Call with not supported vrId [4096] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */

        vrId = 4096;

        st = cpssDxChIpLpmFcoePrefixSearch(lpmDBId, vrId, &fcoeAddr, prefixLen,
                                             &nextHopInfoGet);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.8. Call with out of range prefixLen [33] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        prefixLen = 33;

        st = cpssDxChIpLpmFcoePrefixSearch(lpmDBId, vrId, &fcoeAddr, prefixLen,
                                             &nextHopInfoGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d", lpmDBId, prefixLen);

        /*
            1.9. Call with lpmDBId[0], vrId[0], ipAddr[0x112233], prefixLen[24] and nextHopInfoPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        prefixLen = 24;

        st = cpssDxChIpLpmFcoePrefixSearch(lpmDBId, vrId, &fcoeAddr, prefixLen,
                                             NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);

        /*
            1.10. Call with lpmDBId[0], vrId[0], fcoeAddrPtr[NULL], prefixLen[24] and and non-null nextHopInfoPtr.
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpLpmFcoePrefixSearch(lpmDBId, vrId, NULL, prefixLen, &nextHopInfoGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, fcoeAddrPtr = NULL", lpmDBId, vrId);

        /*
            1.11. Call cpssDxChIpLpmFcoePrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId    = 0;
        lpmDBId = 0;

        st = cpssDxChIpLpmFcoePrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Restore configuration */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmFcoePrefixGet
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_FCID                                 *fcoeAddrPtr;
    OUT GT_U32                                  *prefixLenPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmFcoePrefixGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Create default LPM configuration and add virtual routers.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmFcoePrefixAdd with lpmDBId [0], vrId[0], fcoeAddr[0x112233],
        prefixLen[24], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.3. Call with lpmDBId[0], vrId[0], fcoeAddr[0x112233], prefixLen[24] and non-null nextHopInfoPtr
    Expected: GT_OK and the same nextHopInfoPtr.
    1.4. Call with lpmDBId[0], vrId[0], fcoeAddr[0x1122FF], prefixLen[16] and non-null nextHopInfoPtr
    Expected: NOT GT_OK.
    1.5. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.6. Call with not supported vrId [4096] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.7. Call with lpmDBId[0], vrId[0], fcoeAddr[0x112233], prefixLen[32] and nextHopInfoPtr[NULL].
    Expected: GT_BAD_PTR.
    1.8. Call with fcoeAddrPtr[NULL] and other parameters from 1.7.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId     = 0;
    GT_U32                                 vrId        = 0;
    GT_U32                                 prefixLen   = 0;
    GT_U32                                 prefixLenGet;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfoGet;
    GT_BOOL                                override    = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_FALSE;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT      shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    GT_FCID                                fcoeAddr;
    CPSS_IP_PROTOCOL_STACK_ENT             protocolStack   = CPSS_IP_PROTOCOL_FCOE_E;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT     memoryCfg;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo;
    GT_U32                                 notAppFamilyBmp;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&nextHopInfoGet, sizeof(nextHopInfoGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1.
        */
        /* restore configuration after other UTs */
        /* remove changed virtual routers first */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);

        lpmDBId = 0;
        vrId    = 0;
        st = prvUtfIpLpmRamDefaultConfigCalc(dev, &memoryCfg.ramDbCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, shadowType);

        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &memoryCfg);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                "memoryCfg.ramDbCfg.numOfBlocks = %d,"
                "memoryCfg.ramDbCfg.blocksAllocationMethod = %d",
                lpmDBId, shadowType, protocolStack,
                memoryCfg.ramDbCfg.numOfBlocks,
                memoryCfg.ramDbCfg.blocksAllocationMethod);

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, &dev, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_FALSE;
        vrConfigInfo.supportIpv4Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Uc = GT_FALSE;
        vrConfigInfo.supportFcoe   = GT_TRUE;
        /* Fill defUcNextHopInfoPtr structure */
        prvUtfSetDefaultActionEntry(&vrConfigInfo.defaultFcoeForwardingNextHopInfo.pclIpUcAction);

        /* add virtual router for 0 db*/
        lpmDBId = 0;
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.2. Call cpssDxChIpLpmFcoePrefixAdd with lpmDBId [0], vrId[0], fcoeAddr[0x112233],
                      prefixLen[24], nextHopInfoPtr->pclIpUcAction { pktCmd =
                      CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                      CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                      matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos
                      { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                      ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                      profilePrecedence=GT_FALSE] ] }, redirect {
                      CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                      policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                      egressTaggedModify=GT_FALSE,
                      modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                      vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                      ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                      override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x33;

        prefixLen = 24;

        prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

        override = GT_TRUE;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmFcoePrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.3. Call with lpmDBId[0], vrId[0], fcoeAddr[0x112233], prefixLen[24] and non-null nextHopInfoPtr
            Expected: GT_OK and the same nextHopInfoPtr and prefixLenPtr.
        */
        lpmDBId = 0;
        vrId    = 0;

        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x33;

        prefixLen = 24;

        st = cpssDxChIpLpmFcoePrefixGet(lpmDBId, vrId, &fcoeAddr, &prefixLenGet,
                                          &nextHopInfoGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLenGet);


        prvUtfCompareIpLttEntries(lpmDBId, vrId, &nextHopInfoGet.ipLttEntry, &nextHopInfo.ipLttEntry);

        UTF_VERIFY_EQUAL1_STRING_MAC(prefixLen, prefixLenGet,
                "got another prefixLen then was set: %d", prefixLenGet);

        /*
            1.4. Call with lpmDBId[0], vrId[0], fcoeAddr[0x1122FF], prefixLen[16] and non-null nextHopInfoPtr
            Expected: GT_OK and the same nextHopInfoPtr.
        */
        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0xFF;

        prefixLen = 16;

        st = cpssDxChIpLpmFcoePrefixGet(lpmDBId, vrId, &fcoeAddr, &prefixLenGet,
                                          &nextHopInfoGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(prefixLen, prefixLenGet,
                "got another prefixLen then was set: %d", prefixLenGet);

        /*
            1.5. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x33;

        prefixLen = 24;

        st = cpssDxChIpLpmFcoePrefixGet(lpmDBId, vrId, &fcoeAddr, &prefixLenGet,
                                          &nextHopInfoGet);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.6. Call with not supported vrId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */

        vrId = 4096;

        st = cpssDxChIpLpmFcoePrefixGet(lpmDBId, vrId, &fcoeAddr, &prefixLenGet,
                                          &nextHopInfoGet);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.7. Call with lpmDBId[0], vrId[0], fcoeAddr[0x112233], prefixLen[24] and nextHopInfoPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpLpmFcoePrefixGet(lpmDBId, vrId, &fcoeAddr, &prefixLenGet,
                                          NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);

        /*
            1.8. Call with fcoeAddrPtr[NULL] and  other parameters from 1.7.
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChIpLpmFcoePrefixGet(lpmDBId, vrId, NULL, &prefixLenGet,  &nextHopInfoGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, fcoeAddrPtr = NULL", lpmDBId, vrId);

        /*
            1.9. Call cpssDxChIpLpmFcoePrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId    = 0;
        lpmDBId = 0;

        st = cpssDxChIpLpmFcoePrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Restore configuration */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmFcoePrefixGetNext
(
    IN    GT_U32                                    lpmDBId,
    IN    GT_U32                                    vrId,
    INOUT GT_FCID                                   fcoeAddr;
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *nextHopInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmFcoePrefixGetNext)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Create default LPM configuration and add virtual routers.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmFcoePrefixAdd with lpmDBId [0], vrId[0], fcoeAddr[0x112233],
        prefixLen[24], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.3. Call cpssDxChIpLpmFcoePrefixAdd with lpmDBId [0], vrId[0], fcoeAddr[0x112244],
        prefixLen[24], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.4. Call with lpmDBId[0], vrId[0], fcoeAddrPtr [0x112233], prefixLenPtr [24] and non-null
        nextHopInfoPtr
    Expected: GT_OK and the same prefixLenPtr  and nextHopInfoPtr.
    1.5. call with lpmDBId[0], vrId[0], fcoeAddrPtr [0x112244], prefixLenPtr [24]
    and non-null nextHopInfoPtr
    Expected: NOT GT_OK (the last prefix reached).
    1.6. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.7. Call with not supported vrId [4096] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.8. Call with out of range prefixLenPtr [33] and other parameters from 1.5.
    Expected: NOT GT_OK.
    1.9. Call with lpmDBId[0], vrId[0], fcoeAddrPtr [NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
    1.10. Call with lpmDBId[0], vrId[0], prefixLenPtr [NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
    1.11. Call with lpmDBId[0], vrId[0], nextHopInfoPtr [NULL] and other parameters from 1.5.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId     = 0;
    GT_U32                                 vrId        = 0;
    GT_FCID                                fcoeAddr;
    GT_FCID                                fcoeAddrTmp;
    GT_U32                                 prefixLen   = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfoGet;
    GT_BOOL                                override    = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_FALSE;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT      shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    CPSS_IP_PROTOCOL_STACK_ENT             protocolStack   = CPSS_IP_PROTOCOL_FCOE_E;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT     memoryCfg;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo;
    GT_U32                                 notAppFamilyBmp;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));
    cpssOsBzero((GT_VOID*)&nextHopInfoGet, sizeof(nextHopInfoGet));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1.
        */
        /* restore configuration after other UTs */
        /* remove changed virtual routers first */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);

        lpmDBId = 0;
        vrId    = 0;
        st = prvUtfIpLpmRamDefaultConfigCalc(dev, &memoryCfg.ramDbCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, shadowType);

        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &memoryCfg);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                "memoryCfg.ramDbCfg.numOfBlocks = %d,"
                "memoryCfg.ramDbCfg.blocksAllocationMethod = %d",
                lpmDBId, shadowType, protocolStack,
                memoryCfg.ramDbCfg.numOfBlocks,
                memoryCfg.ramDbCfg.blocksAllocationMethod);

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, &dev, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_FALSE;
        vrConfigInfo.supportIpv4Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Uc = GT_FALSE;
        vrConfigInfo.supportFcoe   = GT_TRUE;
        /* Fill defUcNextHopInfoPtr structure */
        prvUtfSetDefaultActionEntry(&vrConfigInfo.defaultFcoeForwardingNextHopInfo.pclIpUcAction);

        /* add virtual router for 0 db*/
        lpmDBId = 0;
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.2. Call cpssDxChIpLpmFcoePrefixAdd with lpmDBId [0],
                 vrId[0], fcoeAddr[0x112233], prefixLen[24], nextHopInfoPtr->pclIpUcAction {
                 pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                 CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                 matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                 egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                 ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                 profilePrecedence=GT_FALSE] ] }, redirect {
                 CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
                 policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                 modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                 vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                 ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                 override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x33;

        prefixLen = 24;

        prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

        override  = GT_TRUE;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmFcoePrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.3. Call cpssDxChIpLpmFcoePrefixAdd with lpmDBId [0],
                 vrId[0], fcoeAddr[0x112244], prefixLen[24], nextHopInfoPtr->pclIpUcAction {
                 pktCmd = CPSS_PACKET_CMD_FORWARD_E, mirror{cpuCode =
                 CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
                 matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                 egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                 ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                 profilePrecedence=GT_FALSE] ] }, redirect {
                 CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
                 policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                 modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                 vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                 ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                 override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x44;

        prefixLen = 24;

        override  = GT_TRUE;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChIpLpmFcoePrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.4. Call with lpmDBId[0], vrId[0], fcoeAddrPtr [0x112233],
                 prefixLenPtr [32] and non-null nextHopInfoPtr
            Expected: GT_OK and the same prefixLenPtr  and nextHopInfoPtr.
        */
        lpmDBId = 0;
        vrId    = 0;

        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x33;

        fcoeAddrTmp.fcid[0] = 0x11;
        fcoeAddrTmp.fcid[1] = 0x22;
        fcoeAddrTmp.fcid[2] = 0x44;

        prefixLen = 24;

        st = cpssDxChIpLpmFcoePrefixGetNext(lpmDBId, vrId, &fcoeAddr, &prefixLen,
                                               &nextHopInfoGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        prvUtfCompareIpLttEntries(lpmDBId, vrId, &nextHopInfoGet.ipLttEntry,
                                                    &nextHopInfo.ipLttEntry);

        /* Verifying fcoeAddr */
        UTF_VERIFY_EQUAL2_STRING_MAC(fcoeAddr.fcid[0], fcoeAddrTmp.fcid[0],
                   "get another fcoeAddrPtr[0] than was set: %d, %d", lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(fcoeAddr.fcid[1], fcoeAddrTmp.fcid[1],
                   "get another fcoeAddrPtr[1] than was set: %d, %d", lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_STRING_MAC(fcoeAddr.fcid[2], fcoeAddrTmp.fcid[2],
                   "get another fcoeAddrPtr[2] than was set: %d, %d", lpmDBId, vrId);

        /*
            1.5. call with lpmDBId[0], vrId[0], fcoeAddrPtr [0x112244],
            prefixLenPtr [24] and non-null nextHopInfoPtr
            Expected: NOT GT_OK (the last prefix reached).
        */
        if (prvUtfIsPbrModeUsed())
        {
            fcoeAddr.fcid[0] = 0x11;
            fcoeAddr.fcid[1] = 0x22;
            fcoeAddr.fcid[2] = 0x44;

            st = cpssDxChIpLpmFcoePrefixGetNext(lpmDBId, vrId, &fcoeAddr, &prefixLen,
                                                   &nextHopInfoGet);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        }

        /*
            1.6. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmFcoePrefixGetNext(lpmDBId, vrId, &fcoeAddr, &prefixLen,
                                              &nextHopInfoGet);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.7. Call with not supported vrId [4096] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */

        vrId = 4096;

        st = cpssDxChIpLpmFcoePrefixGetNext(lpmDBId, vrId, &fcoeAddr, &prefixLen,
                                               &nextHopInfoGet);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.8. Call with out of range prefixLenPtr [33] and other parameters from 1.5.
            Expected: NOT GT_OK.
        */
        prefixLen = 33;

        st = cpssDxChIpLpmFcoePrefixGetNext(lpmDBId, vrId, &fcoeAddr, &prefixLen,
                                               &nextHopInfoGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d", lpmDBId, prefixLen);

        prefixLen = 24;

        /*
            1.9. Call with lpmDBId[0], vrId[0], fcoeAddrPtr [NULL] and other parameters from 1.5.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmFcoePrefixGetNext(lpmDBId, vrId, NULL, &prefixLen,
                                              &nextHopInfoGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, fcoeAddrPtr = NULL", lpmDBId, vrId);

        /*
            1.10. Call with lpmDBId[0], vrId[0], prefixLenPtr [NULL] and other parameters from 1.5.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmFcoePrefixGetNext(lpmDBId, vrId, &fcoeAddr, NULL,
                                              &nextHopInfoGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, prefixLenPtr = NULL", lpmDBId, vrId);

        /*
            1.11. Call with lpmDBId[0], vrId[0], nextHopInfoPtr [NULL] and other parameters from 1.5.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmFcoePrefixGetNext(lpmDBId, vrId, &fcoeAddr, &prefixLen,
                                               NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nextHopInfoPtr = NULL", lpmDBId, vrId);

        /*
            1.12. Call cpssDxChIpLpmFcoePrefixesFlush with lpmDBId [0]
                  and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId    = 0;
        lpmDBId = 0;

        st = cpssDxChIpLpmFcoePrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Restore configuration */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmFcoePrefixesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmFcoePrefixesFlush)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Create default LPM configuration and add virtual routers.
    Expected: GT_OK.
    1.2. Call with lpmDBId [0], vrId[0].
    Expected: GT_OK.
    1.3. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.2.
    Expected: NOT GT_OK.
    1.4. Call with not supported vrId [4096] and other parameters from 1.3.
    Expected: NOT GT_OK.
*/
    GT_STATUS                              st = GT_OK;
    GT_U8                                  dev;

    GT_U32                                 lpmDBId = 0;
    GT_U32                                 vrId    = 0;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT      shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    CPSS_IP_PROTOCOL_STACK_ENT             protocolStack   = CPSS_IP_PROTOCOL_FCOE_E;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT     memoryCfg;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo;
    GT_U32                                 notAppFamilyBmp;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.
        */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmVirtualRouterDel(1, 0);
        cpssDxChIpLpmDBDelete(0);
        cpssDxChIpLpmDBDelete(1);

        lpmDBId = 0;
        vrId    = 0;
        st = prvUtfIpLpmRamDefaultConfigCalc(dev, &memoryCfg.ramDbCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, shadowType);

        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &memoryCfg);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                "memoryCfg.ramDbCfg.numOfBlocks = %d,"
                "memoryCfg.ramDbCfg.blocksAllocationMethod = %d",
                lpmDBId, shadowType, protocolStack,
                memoryCfg.ramDbCfg.numOfBlocks,
                memoryCfg.ramDbCfg.blocksAllocationMethod);

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, &dev, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        lpmDBId = 1;
        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &memoryCfg);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                "memoryCfg.ramDbCfg.numOfBlocks = %d,"
                "memoryCfg.ramDbCfg.blocksAllocationMethod = %d",
                lpmDBId, shadowType, protocolStack,
                memoryCfg.ramDbCfg.numOfBlocks,
                memoryCfg.ramDbCfg.blocksAllocationMethod);

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, &dev, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_FALSE;
        vrConfigInfo.supportIpv4Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Uc = GT_FALSE;
        vrConfigInfo.supportFcoe   = GT_TRUE;
        /* Fill defUcNextHopInfoPtr structure */
        prvUtfSetDefaultActionEntry(&vrConfigInfo.defaultFcoeForwardingNextHopInfo.pclIpUcAction);

        /* add virtual router for 0 db*/
        lpmDBId = 0;
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* add virtual router for 1 db*/
        lpmDBId = 1;
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.2. Call with lpmDBId [0], vrId[0].
            Expected: GT_OK.
        */
        lpmDBId = 1;
        vrId    = 0;

        st = cpssDxChIpLpmFcoePrefixesFlush(lpmDBId, vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);
        /*
            1.3. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.2.
            Expected: NOT GT_OK.
        */
        vrId = 0;
        lpmDBId = 10;

        st = cpssDxChIpLpmFcoePrefixesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        /*
            1.4. Call with wrong  vrId [4096].
            Expected: NOT GT_OK.
        */

        vrId = 4096;
        lpmDBId = 0;
        st = cpssDxChIpLpmFcoePrefixesFlush(lpmDBId, vrId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        /* Restore configuration */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmVirtualRouterDel(1, 0);
        cpssDxChIpLpmDBDelete(0);
        cpssDxChIpLpmDBDelete(1);
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmFcoePrefixDel
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               vrId,
    IN GT_FCID                              *fcoeAddrPtr,
    IN GT_U32                               prefixLen
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmFcoePrefixDel)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Create default LPM configuration and add virtual routers.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmFcoePrefixAdd with lpmDBId [0], vrId[0], fcoeAddr[0x112233],
        prefixLen[24], nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
        egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
        ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] }, policer {
        policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
        precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute {
        doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } }, override [GT_TRUE].
    Expected: GT_OK.
    1.3. Call with lpmDBId [0], vrId[0], fcoeAddr[0x112233], prefixLen[24].
    Expected: GT_OK.
    1.4. Call with lpmDBId [0], vrId[0], fcoeAddr[0x112233] (already deleted), prefixLen[24].
    Expected: NOT GT_OK.
    1.5. Call with lpmDBId [0], vrId[0], fcoeAddr[0x112244] (wrong FCOE address), prefixLen[24].
    Expected: NOT GT_OK.
    1.6. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.7. Call with not supported vrId [4096] and other parameters from 1.2.
    Expected: NOT GT_OK.
    1.8. Call with out of range prefixLen [33] and other parameters from 1.2.
    Expected: NOT GT_OK.
    1.9. Call cpssDxChIpLpmFcoePrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                 lpmDBId   = 0;
    GT_U32                                 vrId      = 0;
    GT_FCID                                fcoeAddr;
    GT_U32                                 prefixLen = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override  = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_FALSE;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT      shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    CPSS_IP_PROTOCOL_STACK_ENT             protocolStack   = CPSS_IP_PROTOCOL_FCOE_E;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT     memoryCfg;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo;
    GT_U32                                 notAppFamilyBmp;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&nextHopInfo, sizeof(nextHopInfo));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1.
        */
        /* restore configuration after other UTs */
        /* remove changed virtual routers first */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);

        lpmDBId = 0;
        vrId    = 0;
        st = prvUtfIpLpmRamDefaultConfigCalc(dev, &memoryCfg.ramDbCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, shadowType);

        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &memoryCfg);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                "memoryCfg.ramDbCfg.numOfBlocks = %d,"
                "memoryCfg.ramDbCfg.blocksAllocationMethod = %d",
                lpmDBId, shadowType, protocolStack,
                memoryCfg.ramDbCfg.numOfBlocks,
                memoryCfg.ramDbCfg.blocksAllocationMethod);

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, &dev, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_FALSE;
        vrConfigInfo.supportIpv4Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Uc = GT_FALSE;
        vrConfigInfo.supportFcoe   = GT_TRUE;
        /* Fill defUcNextHopInfoPtr structure */
        prvUtfSetDefaultActionEntry(&vrConfigInfo.defaultFcoeForwardingNextHopInfo.pclIpUcAction);

        /* add virtual router for 0 db*/
        lpmDBId = 0;
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.2. Call cpssDxChIpLpmFcoePrefixAdd with lpmDBId [0], vrId[0], fcoeAddr[0x112233],
                            prefixLen[24],
                            nextHopInfoPtr->pclIpUcAction { pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                            mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                            mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount
                            = GT_FALSE, matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE,
                            modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [
                            ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                            profilePrecedence=GT_FALSE] ] }, redirect {
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                            policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                            egressTaggedModify=GT_FALSE,
                            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                            nestedVlan=GT_FALSE, vlanId=100,
                            precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                            override [GT_TRUE].
            Expected: GT_OK.
        */

        lpmDBId = 0;
        vrId    = 0;

        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x33;

        prefixLen = 24;

        prvUtfSetDefaultIpLttEntry(dev, &nextHopInfo.ipLttEntry);

        override  = GT_TRUE;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpLpmFcoePrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        /*
            1.3. Call with lpmDBId [0], vrId[0], fcoeAddr[0x112233], prefixLen[24].
            Expected: GT_OK.
        */
        lpmDBId = 0;
        vrId    = 0;

        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x33;

        prefixLen = 24;

        st = cpssDxChIpLpmFcoePrefixDel(lpmDBId, vrId, &fcoeAddr, prefixLen);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.4. call with lpmDBId [0], vrId[0], fcoeAddr[0x112233] (already
            deleted), prefixLen[24].
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmFcoePrefixDel(lpmDBId, vrId, &fcoeAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.5. call with lpmDBId [0], vrId[0], fcoeAddr[0x112244] (wrong FCOE
            address), prefixLen[32].
            Expected: NOT GT_OK.
        */
        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x44;

        st = cpssDxChIpLpmFcoePrefixDel(lpmDBId, vrId, &fcoeAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /*
            1.6. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        fcoeAddr.fcid[0] = 0x11;
        fcoeAddr.fcid[1] = 0x22;
        fcoeAddr.fcid[2] = 0x33;

        st = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId, &fcoeAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpLpmFcoePrefixAdd: %d, %d, %d, override = %d",
                                     lpmDBId, vrId, prefixLen, override);

        lpmDBId = 10;

        st = cpssDxChIpLpmFcoePrefixDel(lpmDBId, vrId, &fcoeAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.7. Call with not supported vrId [10] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        vrId = 4096;

        st = cpssDxChIpLpmFcoePrefixDel(lpmDBId, vrId, &fcoeAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        vrId = 0;

        /*
            1.8. Call with out of range prefixLen [33] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        prefixLen = 33;

        st = cpssDxChIpLpmFcoePrefixDel(lpmDBId, vrId, &fcoeAddr, prefixLen);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d", lpmDBId, prefixLen);

        prefixLen = 24;

        /*
            1.9. Call cpssDxChIpLpmFcoePrefixesFlush with lpmDBId [0 / 1] and vrId[0] to
            invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId[0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmFcoePrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Restore configuration */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmFcoePrefixAddBulk
(
    IN GT_U32                                          lpmDBId,
    IN GT_U32                                          fcoeAddrPrefixArrayLen,
    IN CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    *fcoeAddrPrefixArrayPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmFcoePrefixAddBulk)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Create default LPM configuration and add virtual routers.
    Expected: GT_OK.
    1.2. Call with lpmDBId[0], fcoeAddrPrefixArrayLen[1], fcoeAddrPrefixArray{vrId[0],
         fcoeAddr[0x112233], prefixLen[24], nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
         mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E, mirrorToRxAnalyzerPort = GT_FALSE},
         matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
         qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
         qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ]},
         redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
         policer { policerEnable=GT_FALSE, policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
         modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE, vlanId=100,
         precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
         ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
         override[GT_TRUE], returnStatus[0]}
    Expected: GT_OK.
    1.3. Call with not supported fcoeAddrPrefixArray->vrId[10] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.4. Call with out of range fcoeAddrPrefixArray->prefixLen [33] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.5. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.6. Call with fcoeAddrPrefixArray[NULL] and other parameters from 1.3.
    Expected: GT_BAD_PTR.
    1.7. Call cpssDxChIpLpmFcoePrefixesFlush with lpmDBId [0] and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                          lpmDBId   = 0;
    GT_U32                                          prefixArrLen = 0;
    CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    fcoeAddrPrefixArray;
    GT_U32                                          vrId      = 0;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT               shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack   = CPSS_IP_PROTOCOL_FCOE_E;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT              memoryCfg;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC                  vrConfigInfo;
    GT_U32                                          notAppFamilyBmp;
    GT_U32                                          prefixLenGet;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT          nextHopInfoGet;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&fcoeAddrPrefixArray, sizeof(fcoeAddrPrefixArray));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1.
        */
        /* restore configuration after other UTs */
        /* remove changed virtual routers first */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);

        lpmDBId = 0;
        vrId    = 0;
        st = prvUtfIpLpmRamDefaultConfigCalc(dev, &memoryCfg.ramDbCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, shadowType);

        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &memoryCfg);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                "memoryCfg.ramDbCfg.numOfBlocks = %d,"
                "memoryCfg.ramDbCfg.blocksAllocationMethod = %d",
                lpmDBId, shadowType, protocolStack,
                memoryCfg.ramDbCfg.numOfBlocks,
                memoryCfg.ramDbCfg.blocksAllocationMethod);

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, &dev, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_FALSE;
        vrConfigInfo.supportIpv4Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Uc = GT_FALSE;
        vrConfigInfo.supportFcoe   = GT_TRUE;
        /* Fill defUcNextHopInfoPtr structure */
        prvUtfSetDefaultActionEntry(&vrConfigInfo.defaultFcoeForwardingNextHopInfo.pclIpUcAction);

        /* add virtual router for 0 db*/
        lpmDBId = 0;
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);


        /*
            1.2. Call with lpmDBId[0], fcoeAddrPrefixArrayLen[1], fcoeAddrPrefixArray{vrId[0],
                   fcoeAddr[0x112233], prefixLen[24],
                   nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                   mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                   mirrorToRxAnalyzerPort = GT_FALSE},
                   matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
                   qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                   qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                   profilePrecedence=GT_FALSE] ] },
                   redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                   policer { policerEnable=GT_FALSE, policerId=0 },
                   vlan { egressTaggedModify=GT_FALSE,
                   modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                   vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                   ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                   override[GT_TRUE], returnStatus[0]}
            Expected: GT_OK.
        */
        lpmDBId         = 0;
        prefixArrLen    = 1;

        fcoeAddrPrefixArray.fcoeAddr.fcid[0] = 0x11;
        fcoeAddrPrefixArray.fcoeAddr.fcid[1] = 0x22;
        fcoeAddrPrefixArray.fcoeAddr.fcid[2] = 0x33;

        fcoeAddrPrefixArray.prefixLen = 24;

        prvUtfSetDefaultIpLttEntry(dev, &fcoeAddrPrefixArray.nextHopInfo.ipLttEntry);

        fcoeAddrPrefixArray.override                = GT_TRUE;
        fcoeAddrPrefixArray.returnStatus            = GT_OK;
        fcoeAddrPrefixArray.vrId = 0;

        st = cpssDxChIpLpmFcoePrefixAddBulk(lpmDBId, prefixArrLen, &fcoeAddrPrefixArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, prefixArrLen);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, fcoeAddrPrefixArray.returnStatus);

        st = cpssDxChIpLpmFcoePrefixGet(lpmDBId, vrId,  &(fcoeAddrPrefixArray.fcoeAddr), &prefixLenGet,
                                          &nextHopInfoGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLenGet);

        /*
            1.3. Call with not supported fcoeAddrPrefixArray->vrId[10]
                 and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        fcoeAddrPrefixArray.vrId = 10;

        st = cpssDxChIpLpmFcoePrefixAddBulk(lpmDBId, prefixArrLen, &fcoeAddrPrefixArray);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, fcoeAddrPrefixArray->vrId = %d",
                                         lpmDBId, fcoeAddrPrefixArray.vrId);

        fcoeAddrPrefixArray.vrId = 0;

        /*
            1.4. Call with out of range fcoeAddrPrefixArray->prefixLen [33]
                 and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        fcoeAddrPrefixArray.prefixLen = 33;

        st = cpssDxChIpLpmFcoePrefixAddBulk(lpmDBId, prefixArrLen, &fcoeAddrPrefixArray);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, fcoeAddrPrefixArray->prefixLen = %d",
                                         lpmDBId, fcoeAddrPrefixArray.prefixLen);

        fcoeAddrPrefixArray.prefixLen = 24;

        /*
            1.5. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmFcoePrefixAddBulk(lpmDBId, prefixArrLen, &fcoeAddrPrefixArray);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.6. Call with fcoeAddrPrefixArray[NULL] and other parameters from 1.3.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmFcoePrefixAddBulk(lpmDBId, prefixArrLen, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, fcoeAddrPrefixArray = NULL", lpmDBId);

        /*
            1.7. Call cpssDxChIpLpmFcoePrefixesFlush with lpmDBId [0]
                 and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;
        lpmDBId = 0;

        st = cpssDxChIpLpmFcoePrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Restore configuration */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLpmFcoePrefixDelBulk
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               fcoeAddrPrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *fcoeAddrPrefixArray
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLpmFcoePrefixDelBulk)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Create default LPM configuration and add virtual routers.
    Expected: GT_OK.
    1.2. Call cpssDxChIpLpmFcoePrefixAddBulk with lpmDBId[0],
           fcoeAddrPrefixArrayLen[1],
           fcoeAddrPrefixArray{vrId[0],
                           fcoeAddr[0x112233],
                           prefixLen[24],
                           nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                           mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                           mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                           enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                           egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                           modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                           profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ]
                           }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                           data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE,
                           policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                           modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                           nestedVlan=GT_FALSE, vlanId=100,
                           precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                           ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE,
                           GT_FALSE } },
                           override[GT_TRUE],
                           returnStatus[0]}
    Expected: GT_OK.
    1.3. Call with lpmDBId[0],
                   fcoeAddrPrefixArrayLen[1],
                   fcoeAddrPrefixArray{vrId[0],
                                   fcoeAddr[0x112233],
                                   prefixLen[24],
                                   nextHopInfoPtr{pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                                   mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                   mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                                   enableMatchCount = GT_FALSE, matchCounterIndex = 0
                                   }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                                   modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                                   profileAssignIndex=GT_FALSE,
                                   profilePrecedence=GT_FALSE] ] }, redirect {
                                   CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                                   data[routerLttIndex=0] }, policer {
                                   policerEnable=GT_FALSE, policerId=0 }, vlan {
                                   egressTaggedModify=GT_FALSE,
                                   modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                   nestedVlan=GT_FALSE, vlanId=100,
                                   precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
                                   }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE,
                                   GT_FALSE, GT_FALSE } },
                   override[GT_TRUE],
                   returnStatus[0]}
    Expected: GT_OK.
    1.4. Call with lpmDBId[0],
            fcoeAddrPrefixArrayLen[1], fcoeAddrPrefixArray{vrId[0], fcoeAddr[0x112233],
            prefixLen[24], nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
            mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
            mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount =
            GT_FALSE, matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE,
            modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
            profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] }, redirect
            { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
            policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
            egressTaggedModify=GT_FALSE,
            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
            nestedVlan=GT_FALSE, vlanId=100,
            precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute
            { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
            override[GT_TRUE], returnStatus[0]}
            (already deleted)
    Expected: NOT GT_OK.
    1.5. Call with not supported fcoeAddrPrefixArray->vrId[10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.6. Call with out of range fcoeAddrPrefixArray->prefixLen [33]
         and other parameters from 1.3.
    Expected: NOT GT_OK.
    1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
    Expected: NOT GT_OK.
    1.8. Call with fcoeAddrPrefixArray[NULL] and other parameters from 1.4.
    Expected: GT_BAD_PTR.
    1.9. Call cpssDxChIpLpmFcoePrefixesFlush with lpmDBId [0]
        and vrId[0] to invalidate changes.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_U32                                          lpmDBId     = 0;
    GT_U32                                          prefixArrLen = 0;
    CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    fcoeAddrPrefixArray;
    CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    fcoeAddrPrefix;
    GT_U32                                          vrId        = 0;
    GT_U32                                          prefixLen   = 0;
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT               shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack   = CPSS_IP_PROTOCOL_FCOE_E;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT              memoryCfg;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC                  vrConfigInfo;
    GT_U32                                          notAppFamilyBmp;
    GT_U32                                          prefixLenGet;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT          nextHopInfoGet;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&fcoeAddrPrefix, sizeof(fcoeAddrPrefix));
    cpssOsBzero((GT_VOID*)&fcoeAddrPrefixArray, sizeof(fcoeAddrPrefixArray));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1.
        */
        /* restore configuration after other UTs */
        /* remove changed virtual routers first */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);

        lpmDBId = 0;
        vrId    = 0;
        st = prvUtfIpLpmRamDefaultConfigCalc(dev, &memoryCfg.ramDbCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, shadowType);

        st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &memoryCfg);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st, "%d, %d, %d, "
                "memoryCfg.ramDbCfg.numOfBlocks = %d,"
                "memoryCfg.ramDbCfg.blocksAllocationMethod = %d",
                lpmDBId, shadowType, protocolStack,
                memoryCfg.ramDbCfg.numOfBlocks,
                memoryCfg.ramDbCfg.blocksAllocationMethod);

        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, &dev, 1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, shadowType);

        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_FALSE;
        vrConfigInfo.supportIpv4Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Uc = GT_FALSE;
        vrConfigInfo.supportFcoe   = GT_TRUE;
        /* Fill defUcNextHopInfoPtr structure */
        prvUtfSetDefaultActionEntry(&vrConfigInfo.defaultFcoeForwardingNextHopInfo.pclIpUcAction);

        /* add virtual router for 0 db*/
        lpmDBId = 0;
        st = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);


        /*
            1.2. Call cpssDxChIpLpmFcoePrefixAddBulk with lpmDBId[0],
                   fcoeAddrPrefixArrayLen[1],
                   fcoeAddrPrefixArray{vrId[0],
                                   fcoeAddr[0x112233],
                                   prefixLen[24],
                                   nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                                   mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                   mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                                   enableMatchCount = GT_FALSE, matchCounterIndex = 0 }, qos {
                                   egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                                   modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                                   profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ]
                                   }, redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                                   data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE,
                                   policerId=0 }, vlan { egressTaggedModify=GT_FALSE,
                                   modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                   nestedVlan=GT_FALSE, vlanId=100,
                                   precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
                                   ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE,
                                   GT_FALSE } },
                                   override[GT_TRUE],
                                   returnStatus[0]}
            Expected: GT_OK.
        */
        lpmDBId   = 0;
        prefixLen = 1;

        fcoeAddrPrefixArray.fcoeAddr.fcid[0] = 0x11;
        fcoeAddrPrefixArray.fcoeAddr.fcid[1] = 0x22;
        fcoeAddrPrefixArray.fcoeAddr.fcid[2] = 0x33;

        fcoeAddrPrefixArray.prefixLen = 24;

        prvUtfSetDefaultIpLttEntry(dev, &fcoeAddrPrefixArray.nextHopInfo.ipLttEntry);

        fcoeAddrPrefixArray.override                = GT_TRUE;
        fcoeAddrPrefixArray.returnStatus            = GT_OK;
        fcoeAddrPrefixArray.vrId = 0;

        st = cpssDxChIpLpmFcoePrefixAddBulk(lpmDBId, prefixLen, &fcoeAddrPrefixArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, prefixLen);

        /*
            1.3. Call with lpmDBId[0],
                           fcoeAddrPrefixArrayLen[1],
                           fcoeAddrPrefixArray{vrId[0],
                                           fcoeAddr[0x112233],
                                           prefixLen[24],
                                           nextHopInfoPtr{pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                                           mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                                           mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter {
                                           enableMatchCount = GT_FALSE, matchCounterIndex = 0
                                           }, qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE,
                                           modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                                           profileAssignIndex=GT_FALSE,
                                           profilePrecedence=GT_FALSE] ] }, redirect {
                                           CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                                           data[routerLttIndex=0] }, policer {
                                           policerEnable=GT_FALSE, policerId=0 }, vlan {
                                           egressTaggedModify=GT_FALSE,
                                           modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                           nestedVlan=GT_FALSE, vlanId=100,
                                           precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
                                           }, ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE,
                                           GT_FALSE, GT_FALSE } },
                           override[GT_TRUE],
                           returnStatus[0]}
            Expected: GT_OK.
        */
        lpmDBId  = 0;
        prefixArrLen = 1;

        fcoeAddrPrefix.vrId = 0;

        fcoeAddrPrefix.fcoeAddr.fcid[0] = 0x11;
        fcoeAddrPrefix.fcoeAddr.fcid[1] = 0x22;
        fcoeAddrPrefix.fcoeAddr.fcid[2] = 0x33;

        fcoeAddrPrefix.prefixLen = 24;

        prvUtfSetDefaultIpLttEntry(dev, &fcoeAddrPrefixArray.nextHopInfo.ipLttEntry);

        fcoeAddrPrefix.override                = GT_TRUE;
        fcoeAddrPrefix.returnStatus            = GT_OK;

        st = cpssDxChIpLpmFcoePrefixDelBulk(lpmDBId, prefixArrLen, &fcoeAddrPrefix);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, prefixArrLen);

        st = cpssDxChIpLpmFcoePrefixGet(lpmDBId, vrId, &(fcoeAddrPrefix.fcoeAddr), &prefixLenGet,
                                          &nextHopInfoGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(fcoeAddrPrefix.prefixLen, prefixLenGet, "%d, %d, prefixLen = %d", lpmDBId, vrId, prefixLenGet);

        /*
            1.4. call with lpmDBId[0],
                    fcoeAddrPrefixArrayLen[1], fcoeAddrPrefixArray{vrId[0], fcoeAddr[0x112233],
                    prefixLen[24], nextHopInfoPtr{ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
                    mirror{cpuCode = CPSS_NET_CONTROL_DEST_MAC_TRAP_E,
                    mirrorToRxAnalyzerPort = GT_FALSE}, matchCounter { enableMatchCount =
                    GT_FALSE, matchCounterIndex = 0 }, qos { egressPolicy=GT_FALSE,
                    modifyDscp=GT_FALSE, modifyUp=GT_FALSE , qos [ ingress[profileIndex=0,
                    profileAssignIndex=GT_FALSE, profilePrecedence=GT_FALSE] ] }, redirect
                    { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E, data[routerLttIndex=0] },
                    policer { policerEnable=GT_FALSE, policerId=0 }, vlan {
                    egressTaggedModify=GT_FALSE,
                    modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                    nestedVlan=GT_FALSE, vlanId=100,
                    precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E }, ipUcRoute
                    { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } },
                    override[GT_TRUE], returnStatus[0]}
                    (already deleted)
            Expected: NOT GT_OK.
        */
        st = cpssDxChIpLpmFcoePrefixDelBulk(lpmDBId, prefixArrLen, &fcoeAddrPrefix);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, prefixArrLen);

        /*
            1.5. Call with not supported fcoeAddrPrefixArray->vrId[10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        fcoeAddrPrefix.vrId = 10;

        st = cpssDxChIpLpmFcoePrefixDelBulk(lpmDBId, prefixArrLen, &fcoeAddrPrefix);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "%d, fcoeAddrPrefixArray->vrId = %d", lpmDBId, fcoeAddrPrefix.vrId);

        fcoeAddrPrefix.vrId = 0;

        /*
            1.6. Call with out of range fcoeAddrPrefixArray->prefixLen [33]
                 and other parameters from 1.3.
            Expected: NOT GT_OK.
        */
        fcoeAddrPrefix.prefixLen = 33;

        st = cpssDxChIpLpmFcoePrefixDelBulk(lpmDBId, prefixArrLen, &fcoeAddrPrefix);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                    "%d, fcoeAddrPrefixArray->prefixLen = %d", lpmDBId, fcoeAddrPrefix.prefixLen);

        fcoeAddrPrefix.prefixLen = 24;

        /*
            1.7. Call with not valid LPM DB id lpmDBId [10] and other parameters from 1.4.
            Expected: NOT GT_OK.
        */
        lpmDBId = 10;

        st = cpssDxChIpLpmFcoePrefixDelBulk(lpmDBId, prefixArrLen, &fcoeAddrPrefix);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, lpmDBId);

        lpmDBId = 0;

        /*
            1.8. Call with fcoeAddrPrefixArray[NULL] and other parameters from 1.4.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLpmFcoePrefixDelBulk(lpmDBId, prefixArrLen, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, fcoeAddrPrefixArray = NULL", lpmDBId);

        /*
            1.9. Call cpssDxChIpLpmFcoePrefixesFlush with lpmDBId [0 / 1]
                and vrId[0] to invalidate changes.
            Expected: GT_OK.
        */
        vrId = 0;

        /* Call with lpmDBId[0] */
        lpmDBId = 0;

        st = cpssDxChIpLpmFcoePrefixesFlush(lpmDBId , vrId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, lpmDBId, vrId);

        /* Restore configuration */
        cpssDxChIpLpmVirtualRouterDel(0, 0);
        cpssDxChIpLpmDBDelete(0);
        st = prvUtfCreateLpmDBAndVirtualRouterAdd(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCreateLpmDBAndVirtualRouterAdd: %d", dev);
    }
}

/*
 * Configuration of cpssDxChIpLpm suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChIpLpm)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDBCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDBDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDBConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDBDevListAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDBDevsListRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDBDevListGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmVirtualRouterAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmVirtualRouterDel)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4UcPrefixAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4UcPrefixAddBulk)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4UcPrefixDel)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4UcPrefixDelBulk)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4UcPrefixesFlush)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4UcPrefixSearch)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4UcPrefixGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4UcPrefixGetNext)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4McEntryAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4McEntryDel)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4McEntriesFlush)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4McEntryGetNext)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4McEntrySearch)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4McEntryDelSearch)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6UcPrefixAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6UcPrefixAddBulk)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6UcPrefixDel)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6UcPrefixDelBulk)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6UcPrefixesFlush)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6UcPrefixSearch)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6UcPrefixGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6UcPrefixGetNext)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6McEntryAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6McEntryDel)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6McEntriesFlush)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6McEntryGetNext)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6McEntrySearch)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDBMemSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDBExport)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDBImport)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet)    
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmVirtualRouterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmVirtualRouterSharedAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDBCapacityGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmRandomResizeTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4UcSmallResizeTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4McSmallResizeTest)
    /*UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDBCapacityUpdate)*/
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmVirtualRouterAddSupportIpv4McOnly)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDbgHwValidation)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDbgShadowValidityCheck)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmDbgHwShadowSyncValidityCheck)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmActivityBitEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmActivityBitEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4UcPrefixActivityStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6UcPrefixActivityStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv4McEntryActivityStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmIpv6McEntryActivityStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmFcoePrefixAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmFcoePrefixSearch)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmFcoePrefixGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmFcoePrefixGetNext)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmFcoePrefixesFlush)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmFcoePrefixDel)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmFcoePrefixAddBulk)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLpmFcoePrefixDelBulk)

UTF_SUIT_END_TESTS_MAC(cpssDxChIpLpm)


/**
* @internal prvUtfComparePclIpUcActionEntries function
* @endinternal
*
* @brief   This routine compare PclIpUcActionEntries.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id.
* @param[in] pclActionReceivePtr      - (pointer to) Received value.
* @param[in] pclActionExpectedPtr     - (pointer to) Expected value.
*/
static void prvUtfComparePclIpUcActionEntries
(
    IN GT_U32                      lpmDBId,
    IN GT_U32                      vrId,
    IN CPSS_DXCH_PCL_ACTION_STC*   pclActionReceivePtr,
    IN CPSS_DXCH_PCL_ACTION_STC*   pclActionExpectedPtr
)
{
    if ((NULL == pclActionReceivePtr) || (NULL == pclActionExpectedPtr))
    {
        return;
    }

    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->pktCmd, pclActionReceivePtr->pktCmd,
     "get another pclActionExpectedPtr->pktCmd than was set: %d, %d", lpmDBId, vrId);

    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->mirror.cpuCode,
                                 pclActionReceivePtr->mirror.cpuCode,
     "get another pclActionExpectedPtr->mirror.cpuCode than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->mirror.mirrorToRxAnalyzerPort,
                                 pclActionReceivePtr->mirror.mirrorToRxAnalyzerPort,
     "get another pclActionExpectedPtr->mirror.mirrorToRxAnalyzerPort than was set: %d, %d", lpmDBId, vrId);

    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->matchCounter.enableMatchCount,
                                 pclActionReceivePtr->matchCounter.enableMatchCount,
     "get another pclActionExpectedPtr->matchCounter.enableMatchCount than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->matchCounter.matchCounterIndex,
                                 pclActionReceivePtr->matchCounter.matchCounterIndex,
     "get another pclActionExpectedPtr->matchCounter.matchCounterIndex than was set: %d, %d", lpmDBId, vrId);

    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->egressPolicy,
                                 pclActionReceivePtr->egressPolicy,
     "get another pclActionExpectedPtr->egressPolicy than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->qos.ingress.modifyDscp,
                                 pclActionReceivePtr->qos.ingress.modifyDscp,
     "get another pclActionExpectedPtr->qos.ingress.modifyDscp than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->qos.ingress.modifyUp,
                                 pclActionReceivePtr->qos.ingress.modifyUp,
     "get another pclActionExpectedPtr->qos.ingress.modifyUp than was set: %d, %d", lpmDBId, vrId);

    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->qos.ingress.profileIndex,
                                 pclActionReceivePtr->qos.ingress.profileIndex,
     "get another pclActionExpectedPtr->qos.ingress.profileIndex than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->qos.ingress.profileAssignIndex,
                                 pclActionReceivePtr->qos.ingress.profileAssignIndex,
     "get another pclActionExpectedPtr->qos.ingress.profileAssignIndex than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->qos.ingress.profilePrecedence,
                                 pclActionReceivePtr->qos.ingress.profilePrecedence,
     "get another pclActionExpectedPtr->qos.ingress.profilePrecedence than was set: %d, %d", lpmDBId, vrId);

    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->redirect.redirectCmd,
                                 pclActionReceivePtr->redirect.redirectCmd,
     "get another pclActionExpectedPtr->redirect.redirectCmd than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->redirect.data.routerLttIndex,
                                 pclActionReceivePtr->redirect.data.routerLttIndex,
     "get another pclActionExpectedPtr->redirect.data.routerLttIndex than was set: %d, %d", lpmDBId, vrId);

    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->policer.policerEnable,
                                 pclActionReceivePtr->policer.policerEnable,
     "get another pclActionExpectedPtr->policer.policerEnable than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->policer.policerId,
                                 pclActionReceivePtr->policer.policerId,
     "get another pclActionExpectedPtr->policer.policerId than was set: %d, %d", lpmDBId, vrId);

    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->vlan.ingress.modifyVlan,
                                 pclActionReceivePtr->vlan.ingress.modifyVlan,
     "get another pclActionExpectedPtr->vlan.ingress.modifyVlan than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->vlan.ingress.vlanId,
                                 pclActionReceivePtr->vlan.ingress.vlanId,
     "get another pclActionExpectedPtr->vlan.ingress.vlanId than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->vlan.ingress.precedence,
                                 pclActionReceivePtr->vlan.ingress.precedence,
     "get another pclActionExpectedPtr->vlan.ingress.precedence than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->vlan.ingress.nestedVlan,
                                 pclActionReceivePtr->vlan.ingress.nestedVlan,
     "get another pclActionExpectedPtr->vlan.ingress.nestedVlan than was set: %d, %d", lpmDBId, vrId);

    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->ipUcRoute.doIpUcRoute,
                                 pclActionReceivePtr->ipUcRoute.doIpUcRoute,
     "get another pclActionExpectedPtr->ipUcRoute.doIpUcRoute than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->ipUcRoute.arpDaIndex,
                                 pclActionReceivePtr->ipUcRoute.arpDaIndex,
     "get another pclActionExpectedPtr->ipUcRoute.arpDaIndex than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->ipUcRoute.decrementTTL,
                                 pclActionReceivePtr->ipUcRoute.decrementTTL,
     "get another pclActionExpectedPtr->ipUcRoute.decrementTTL than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->ipUcRoute.bypassTTLCheck,
                                 pclActionReceivePtr->ipUcRoute.bypassTTLCheck,
     "get another pclActionExpectedPtr->ipUcRoute.bypassTTLCheck than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(pclActionExpectedPtr->ipUcRoute.icmpRedirectCheck,
                                 pclActionReceivePtr->ipUcRoute.icmpRedirectCheck,
     "get another pclActionExpectedPtr->ipUcRoute.icmpRedirectCheck than was set: %d, %d", lpmDBId, vrId);

    return;
}

/**
* @internal prvUtfCompareIpLttEntries function
* @endinternal
*
* @brief   This routine compare IpLttEntries.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id.
* @param[in] IpLttEntryReceivePtr     - (pointer to) Received value.
* @param[in] IpLttEntryExpectedPtr    - (pointer to) Expected value.
*/
static void prvUtfCompareIpLttEntries
(
    IN GT_U32                        lpmDBId,
    IN GT_U32                        vrId,
    IN CPSS_DXCH_IP_LTT_ENTRY_STC*   IpLttEntryReceivePtr,
    IN CPSS_DXCH_IP_LTT_ENTRY_STC*   IpLttEntryExpectedPtr
)
{
    if ((NULL == IpLttEntryReceivePtr) || (NULL == IpLttEntryExpectedPtr))
    {
        return;
    }

    UTF_VERIFY_EQUAL2_STRING_MAC(IpLttEntryExpectedPtr->routeType, IpLttEntryReceivePtr->routeType,
     "get another IpLttEntryExpectedPtr->routeType than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(IpLttEntryExpectedPtr->numOfPaths, IpLttEntryReceivePtr->numOfPaths,
     "get another IpLttEntryExpectedPtr->numOfPaths than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(IpLttEntryExpectedPtr->routeEntryBaseIndex,
                                 IpLttEntryReceivePtr->routeEntryBaseIndex,
     "get another IpLttEntryExpectedPtr->routeEntryBaseIndex than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(IpLttEntryExpectedPtr->ucRPFCheckEnable,
                                 IpLttEntryReceivePtr->ucRPFCheckEnable,
     "get another IpLttEntryExpectedPtr->ucRPFCheckEnable than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(IpLttEntryExpectedPtr->sipSaCheckMismatchEnable,
                                 IpLttEntryReceivePtr->sipSaCheckMismatchEnable,
     "get another IpLttEntryExpectedPtr->sipSaCheckMismatchEnable than was set: %d, %d", lpmDBId, vrId);
    UTF_VERIFY_EQUAL2_STRING_MAC(IpLttEntryExpectedPtr->ipv6MCGroupScopeLevel,
                                 IpLttEntryReceivePtr->ipv6MCGroupScopeLevel,
     "get another IpLttEntryExpectedPtr->ipv6MCGroupScopeLevel than was set: %d, %d", lpmDBId, vrId);

    return;
}


/**
* @internal prvUtfSetDefaultActionEntry function
* @endinternal
*
* @brief   This routine fills returns PclIpUcActionEntry with default values.
*
* @param[in] pclActionPtr             - (pointer to) PclIpUcActionEntry.
*/
static void prvUtfSetDefaultActionEntry
(
    IN CPSS_DXCH_PCL_ACTION_STC*   pclActionPtr
)
{
    if (NULL == pclActionPtr)
    {
        return;
    }

    cpssOsBzero((GT_VOID*) pclActionPtr, sizeof(CPSS_DXCH_PCL_ACTION_STC));

    pclActionPtr->pktCmd = CPSS_PACKET_CMD_FORWARD_E;

    pclActionPtr->mirror.cpuCode                = CPSS_NET_CONTROL_DEST_MAC_TRAP_E;
    pclActionPtr->mirror.mirrorToRxAnalyzerPort      = 0;

    pclActionPtr->matchCounter.enableMatchCount  = GT_FALSE;
    pclActionPtr->matchCounter.matchCounterIndex = 0;

    pclActionPtr->egressPolicy     = GT_FALSE;
    pclActionPtr->qos.ingress.modifyDscp   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    pclActionPtr->qos.ingress.modifyUp     = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;

    pclActionPtr->qos.ingress.profileIndex       = 0;
    pclActionPtr->qos.ingress.profileAssignIndex = GT_FALSE;
    pclActionPtr->qos.ingress.profilePrecedence  = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    pclActionPtr->redirect.redirectCmd         = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
    pclActionPtr->redirect.data.routerLttIndex = 0;

    pclActionPtr->policer.policerEnable = GT_FALSE;
    pclActionPtr->policer.policerId     = 0;

    /*pclActionPtr->vlan.ingress.egressTaggedModify = GT_FALSE;*/
    pclActionPtr->vlan.ingress.modifyVlan         = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
    pclActionPtr->vlan.ingress.vlanId             = IP_LPM_TESTED_VLAN_ID_CNS;
    pclActionPtr->vlan.ingress.precedence         = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    pclActionPtr->ipUcRoute.doIpUcRoute       = GT_FALSE;
    pclActionPtr->ipUcRoute.arpDaIndex        = 0;
    pclActionPtr->ipUcRoute.decrementTTL      = GT_FALSE;
    pclActionPtr->ipUcRoute.bypassTTLCheck    = GT_FALSE;
    pclActionPtr->ipUcRoute.icmpRedirectCheck = GT_FALSE;

    return;
}

/**
* @internal prvUtfSetDefaultIpLttEntry function
* @endinternal
*
* @brief   This routine fills returns IpLttEntry with default values.
*
* @param[in] devId                    - device ID
*                                      pclActionPtr - (pointer to) IpLttEntry.
*/
static void prvUtfSetDefaultIpLttEntry
(
    IN GT_U8                         devId,
    IN CPSS_DXCH_IP_LTT_ENTRY_STC*   ipLttEntry
)
{
    if (NULL == ipLttEntry)
    {
        return;
    }

    cpssOsBzero((GT_VOID*) ipLttEntry, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));

    if ((PRV_CPSS_SIP_5_CHECK_MAC(devId)))
    {
        ipLttEntry->routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    else
    {
        ipLttEntry->routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }
    ipLttEntry->numOfPaths               = 0;
    ipLttEntry->routeEntryBaseIndex      = 0;
    ipLttEntry->ucRPFCheckEnable         = GT_TRUE;
    ipLttEntry->sipSaCheckMismatchEnable = GT_TRUE;
    ipLttEntry->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
}

/**
* @internal prvUtfCreateDefaultLpmDB function
* @endinternal
*
* @brief   This routine creates two LpmDBs
*/
static GT_STATUS prvUtfCreateDefaultLpmDB(GT_U8 dev)
{
    GT_STATUS                                    retVal          = GT_OK;
    GT_U32                                       lpmDBId         = 0;
    GT_U32                                       maxTcamIndex    = 0;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType      = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
    CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack   = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC tcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;

    cpssOsBzero((GT_VOID*)&indexesRange, sizeof(indexesRange));
    cpssOsBzero((GT_VOID*)&tcamLpmManagerCapcityCfg, sizeof(tcamLpmManagerCapcityCfg));
    cpssOsBzero((GT_VOID*)&lpmMemoryConfig, sizeof(lpmMemoryConfig));

    if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
    {
        shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    }
    else
    {
        /* set shadow type according to device family */
        switch (PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                shadowType = prvUtfIsPbrModeUsed() ? CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E
                                                   : CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
                break;
            case CPSS_PP_FAMILY_DXCH_LION2_E:
                shadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
                break;
            default:
                return GT_BAD_PARAM;
        }
    }

    protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        lpmMemoryConfig.tcamDbCfg.partitionEnable = GT_TRUE;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = NULL;

        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes         = 10;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 5;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes         = 10;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;

        /* get max Tcam index */
        maxTcamIndex = (CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E == shadowType) ?
                       (PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.policyTcamRaws) :
                       (PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router);
    }
    else
    {
        retVal = prvUtfIpLpmRamDefaultConfigCalc(dev,&lpmMemoryConfig.ramDbCfg);
        if (retVal!=GT_OK)
        {
            return retVal;
        }
    }

    /* Create LpmDB [0] */
    lpmDBId = 0;

    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        indexesRange.firstIndex = 0;
        indexesRange.lastIndex  = 100;
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &indexesRange;
    }

    retVal = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);

    if((retVal != GT_OK)&&(retVal!=GT_ALREADY_EXIST))
        return retVal;

    /* Create LpmDB [1] */
    lpmDBId = 1;

    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        indexesRange.firstIndex = 200;
        indexesRange.lastIndex  = maxTcamIndex - 1;
    }

    retVal = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);

    if((retVal != GT_OK)&&(retVal!=GT_ALREADY_EXIST))
        return retVal;


    /* LpmDB [2] is used in prvUtfCreateMcLpmDB for multicast */


    /* Create LpmDB [3] */
    lpmDBId = 3;

    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        indexesRange.firstIndex = maxTcamIndex / 2;
        indexesRange.lastIndex  = maxTcamIndex - 1;
    }

    retVal = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);

    if((retVal != GT_OK)&&(retVal!=GT_ALREADY_EXIST))
        return retVal;

    /* Create LpmDB [4] */
    lpmDBId = 4;

    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        indexesRange.firstIndex = 101;
        indexesRange.lastIndex  = 199;
    }

    retVal = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);

    if((retVal != GT_OK)&&(retVal!=GT_ALREADY_EXIST))
        return retVal;

    /* Create LpmDB [5] */
    lpmDBId = 5;

    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        indexesRange.firstIndex = 1001;
        indexesRange.lastIndex  = 1020;
    }

    retVal = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);

    retVal = GT_ALREADY_EXIST == retVal ? GT_OK : retVal;

    /* Create LpmDB [6] for FCOE */
    if(PRV_CPSS_SIP_5_CHECK_MAC(dev) == GT_TRUE)
    {
        lpmDBId = 6;

        if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
        {
            indexesRange.firstIndex = 1001;
            indexesRange.lastIndex  = 1020;
        }
        protocolStack = CPSS_IP_PROTOCOL_FCOE_E;

        retVal = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);

        retVal = GT_ALREADY_EXIST == retVal ? GT_OK : retVal;
    }

    return retVal;
}

/**
* @internal prvUtfCreateMcLpmDB function
* @endinternal
*
* @brief   This routine create LpmDB for testing multicast
*/
static GT_STATUS prvUtfCreateMcLpmDB(GT_U8 dev)
{
    GT_STATUS                                    retVal          = GT_OK;
    GT_U32                                       lpmDBId         = 0;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType      = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
    CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack   = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC tcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;

    cpssOsBzero((GT_VOID*)&indexesRange, sizeof(indexesRange));
    cpssOsBzero((GT_VOID*)&tcamLpmManagerCapcityCfg, sizeof(tcamLpmManagerCapcityCfg));
    cpssOsBzero((GT_VOID*)&lpmMemoryConfig, sizeof(lpmMemoryConfig));

    if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
    {
        shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    }
    else
    {
       /* set shadow type according to device family */
       switch (PRV_CPSS_PP_MAC(dev)->devFamily)
       {
           case CPSS_PP_FAMILY_DXCH_XCAT3_E:
           case CPSS_PP_FAMILY_DXCH_LION2_E:
               shadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E;
               break;
           default:
               return GT_NOT_IMPLEMENTED;
       }
    }

    protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        lpmMemoryConfig.tcamDbCfg.partitionEnable = GT_TRUE;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = NULL;

        tcamLpmManagerCapcityCfg.numOfIpv4Prefixes         = 10;
        tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = 5;
        tcamLpmManagerCapcityCfg.numOfIpv6Prefixes         = 10;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
    }

    /* Create LpmDB */
    lpmDBId = 2;

    if (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
    {
        indexesRange.firstIndex = 200;
        indexesRange.lastIndex  = 1000;
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &indexesRange;
    }

    retVal = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack, &lpmMemoryConfig);

    retVal = GT_ALREADY_EXIST == retVal ? GT_OK : retVal;

    return retVal;
}

/**
* @internal prvUtfCreateLpmDBAndVirtualRouterAdd function
* @endinternal
*
* @brief   This routine creates Virtual Routers based on prvUtfCreateDefaultLpmDB
*         configuration of lpmDBId.
* @param[in] dev                      - device ID
*
* @note Cheetah2 doesn't support multiply virtual routers
*       (can be only one vrId in one lpmDBId)
*
*/
static GT_STATUS prvUtfCreateLpmDBAndVirtualRouterAdd
(
    IN GT_U8 dev
)
{
    GT_STATUS                              retVal        = GT_OK;
    GT_U32                                 lpmDBId       = 0;
    GT_U32                                 vrId          = 0;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo;

    cpssOsBzero((GT_VOID*) &vrConfigInfo, sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));


    /* restore configuration after other UTs */
    /* remove changed virtual routers first */
    cpssDxChIpLpmVirtualRouterDel(0, 0);
    cpssDxChIpLpmVirtualRouterDel(1, 0);

    /* Create LpmDB */
    /* prepare device iterator */
    retVal = prvUtfNextNotApplicableDeviceReset(&dev, UTF_NONE_FAMILY_E);
    if (GT_OK != retVal)
    {
        return retVal;
    }

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        retVal = prvUtfCreateDefaultLpmDB(dev);
        if (GT_OK != retVal)
        {
            return retVal;
        }

        /* Adding Virtual Router for device */
        if ((CPSS_PP_FAMILY_CHEETAH_E == PRV_CPSS_PP_MAC(dev)->devFamily) || prvUtfIsPbrModeUsed())
        {
            vrConfigInfo.supportIpv4Mc = GT_FALSE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportIpv6Uc = GT_TRUE;
            vrConfigInfo.supportFcoe   = GT_FALSE;
            /* Fill defUcNextHopInfoPtr structure */
            prvUtfSetDefaultActionEntry(&vrConfigInfo.defIpv4UcNextHopInfo.pclIpUcAction);
            prvUtfSetDefaultActionEntry(&vrConfigInfo.defaultFcoeForwardingNextHopInfo.pclIpUcAction);
            prvUtfSetDefaultActionEntry(&vrConfigInfo.defIpv6UcNextHopInfo.pclIpUcAction);

            /* add virtual router for 0 db*/
            lpmDBId = 0;
            retVal = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            if(retVal != GT_OK)
            {
                if(retVal==GT_ALREADY_EXIST)
                    continue;
                else
                    return retVal;
            }
            /* add virtual router for 1 db*/
            lpmDBId = 1;
            retVal = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            if(retVal != GT_OK)
            {
                if(retVal==GT_ALREADY_EXIST)
                    continue;
                else
                    return retVal;
            }
        }
        else
        {
            vrConfigInfo.supportIpv4Mc = GT_TRUE;
            vrConfigInfo.supportIpv6Mc = GT_TRUE;
            vrConfigInfo.supportIpv4Uc = GT_TRUE;
            vrConfigInfo.supportIpv6Uc = GT_TRUE;
            vrConfigInfo.supportFcoe   = GT_FALSE;
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry);
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defaultFcoeForwardingNextHopInfo.ipLttEntry);
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry);
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv4McRouteLttEntry);
            prvUtfSetDefaultIpLttEntry(dev, &vrConfigInfo.defIpv6McRouteLttEntry);

            /* add virtual router 0 for 0 db*/
            lpmDBId = 0;
            vrId    = 0;
            retVal = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            if((retVal != GT_OK)&&(retVal!=GT_ALREADY_EXIST))
                return retVal;

            /* add virtual router 1 for 0 db (support only UC)*/
            lpmDBId = 3;
            vrId    = 1;
            vrConfigInfo.supportIpv4Mc = GT_FALSE;
            vrConfigInfo.supportIpv6Mc = GT_FALSE;

            retVal = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            if((retVal != GT_OK)&&(retVal!=GT_ALREADY_EXIST))
                return retVal;

            /* add virtual router 0 for 1 db*/
            lpmDBId = 1;
            vrId    = 0;
            vrConfigInfo.supportIpv4Mc = GT_TRUE;
            vrConfigInfo.supportIpv6Mc = GT_TRUE;

            retVal = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            if((retVal != GT_OK)&&(retVal!=GT_ALREADY_EXIST))
                return retVal;

            /* add virtual router 1 for 1 db (support only MC)*/
            vrConfigInfo.supportIpv4Mc = GT_TRUE;
            vrConfigInfo.supportIpv6Mc = GT_TRUE;
            vrConfigInfo.supportIpv4Uc = GT_FALSE;
            vrConfigInfo.supportIpv6Uc = GT_FALSE;
            vrConfigInfo.supportFcoe   = GT_FALSE;

            lpmDBId = 4;
            vrId    = 1;
            retVal = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfigInfo);
            if((retVal != GT_OK)&&(retVal!=GT_ALREADY_EXIST))
                return retVal;
        }
    }

    retVal = GT_ALREADY_EXIST == retVal ? GT_OK : retVal;

    return retVal;
}

/**
* @internal prvUtfCreateLpmDBWithDeviceAndVirtualRouterAdd function
* @endinternal
*
* @brief   This routine creates LpmDB and Virtual Router for Puma device.
*         This routine also add devices to the created LpmDb.
*/
static GT_STATUS prvUtfCreateLpmDBWithDeviceAndVirtualRouterAdd()
{
    GT_STATUS   retVal  = GT_OK;
    GT_U32      lpmDBId = IP_LPM_DEFAULT_DB_ID_CNS;
    GT_U32      vrId    = IP_LPM_DEFAULT_VR_ID_CNS;

    CPSS_DXCH_IP_LPM_VR_CONFIG_STC      vrConfig;
    CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT routeType;
    GT_BOOL                             firstDev=GT_TRUE;/* used for multi device system.
                                                            shadow will be created only for
                                                            first device call, all other calls should
                                                            return GT_ALREADY_EXIST */

    GT_U8       dev       = 0;
    GT_U8       devNum    = 0;
    GT_U8       devListArray[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32      numOfDevs = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&dev,retVal, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&vrConfig, sizeof(vrConfig));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (firstDev==GT_TRUE)
        {
            numOfDevs = 0;

            /* Create LpmDB */
            retVal = prvUtfCreateDefaultLpmDB(dev);
            if (GT_OK != retVal)
            {
                return retVal;
            }

            cpssOsBzero((GT_VOID*) &devListArray, sizeof(devListArray));

            /*
            3. Call cpssDxChIpLpmDBDevListAdd to create dev list for every IpLpmDB
                      with lpmDbId [1 .. 3].
                           numOfDevs [numOfDevs]
                           and devListArray [dev1, dev2 .. devN].
                Expected: GT_OK.
            */
            /* prepare device iterator */
            PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum,retVal, UTF_NONE_FAMILY_E);

            /* Go over all active devices. */
            while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
            {
                devListArray[numOfDevs++] = devNum;
            }

            lpmDBId = IP_LPM_DEFAULT_DB_ID_CNS;

            retVal = cpssDxChIpLpmDBDevListAdd(lpmDBId, devListArray, numOfDevs);
            if (GT_OK != retVal)
            {
                return retVal;
            }

            lpmDBId = IP_LPM_DEFAULT_DB_ID_CNS + 2;

            retVal = cpssDxChIpLpmDBDevListAdd(lpmDBId, devListArray, numOfDevs);
            if (GT_OK != retVal)
            {
                return retVal;
            }

            if((PRV_CPSS_SIP_5_CHECK_MAC(dev)))
            {
                routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
            }
            else
            {
                routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
            }


            /* Adding Virtual Router 1, 0 */
            lpmDBId = IP_LPM_DEFAULT_DB_ID_CNS;
            vrId    = IP_LPM_DEFAULT_VR_ID_CNS;

            vrConfig.supportIpv4Uc = GT_TRUE;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

            vrConfig.supportIpv4Mc = GT_TRUE;
            vrConfig.defIpv4McRouteLttEntry.routeType = routeType;
            vrConfig.defIpv4McRouteLttEntry.routeEntryBaseIndex = 1;
            vrConfig.defIpv4McRouteLttEntry.numOfPaths = 0;
            vrConfig.defIpv4McRouteLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv4McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
            vrConfig.defIpv4McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;

            vrConfig.supportIpv6Uc = GT_TRUE;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 2;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;

            vrConfig.supportIpv6Mc = GT_TRUE;
            vrConfig.defIpv6McRouteLttEntry.routeType = routeType;
            vrConfig.defIpv6McRouteLttEntry.routeEntryBaseIndex = 3;
            vrConfig.defIpv6McRouteLttEntry.numOfPaths = 0;
            vrConfig.defIpv6McRouteLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv6McRouteLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defIpv6McRouteLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

            vrConfig.supportFcoe = GT_FALSE;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

            retVal = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfig);

            if ((GT_OK != retVal) && (GT_ALREADY_EXIST != retVal))
            {
                return retVal;
            }

            /* Adding Virtual Router 1, 1 */
            lpmDBId = IP_LPM_DEFAULT_DB_ID_CNS;
            vrId    = IP_LPM_DEFAULT_VR_ID_CNS + 1;

            vrConfig.supportIpv4Uc = GT_TRUE;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 4;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

            vrConfig.supportIpv4Mc = GT_TRUE;
            vrConfig.defIpv4McRouteLttEntry.routeType = routeType;
            vrConfig.defIpv4McRouteLttEntry.routeEntryBaseIndex = 5;
            vrConfig.defIpv4McRouteLttEntry.numOfPaths = 0;
            vrConfig.defIpv4McRouteLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv4McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
            vrConfig.defIpv4McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;

            vrConfig.supportIpv6Uc = GT_FALSE;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 2;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;

            vrConfig.supportIpv6Mc = GT_FALSE;
            vrConfig.defIpv6McRouteLttEntry.routeType = routeType;
            vrConfig.defIpv6McRouteLttEntry.routeEntryBaseIndex = 3;
            vrConfig.defIpv6McRouteLttEntry.numOfPaths = 0;
            vrConfig.defIpv6McRouteLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv6McRouteLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defIpv6McRouteLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

            vrConfig.supportFcoe = GT_FALSE;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.routeEntryBaseIndex = 4;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_TRUE;
            retVal = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfig);

            if ((GT_OK != retVal) && (GT_ALREADY_EXIST != retVal))
            {
                return retVal;
            }

            /* Adding Virtual Router 3, 0 */
            lpmDBId = IP_LPM_DEFAULT_DB_ID_CNS + 2;
            vrId    = IP_LPM_DEFAULT_VR_ID_CNS;

            vrConfig.supportIpv4Uc = GT_TRUE;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

            vrConfig.supportIpv4Mc = GT_TRUE;
            vrConfig.defIpv4McRouteLttEntry.routeType = routeType;
            vrConfig.defIpv4McRouteLttEntry.routeEntryBaseIndex = 1;
            vrConfig.defIpv4McRouteLttEntry.numOfPaths = 0;
            vrConfig.defIpv4McRouteLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv4McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
            vrConfig.defIpv4McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;

            vrConfig.supportIpv6Uc = GT_TRUE;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 2;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;

            vrConfig.supportIpv6Mc = GT_TRUE;
            vrConfig.defIpv6McRouteLttEntry.routeType = routeType;
            vrConfig.defIpv6McRouteLttEntry.routeEntryBaseIndex = 3;
            vrConfig.defIpv6McRouteLttEntry.numOfPaths = 0;
            vrConfig.defIpv6McRouteLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv6McRouteLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defIpv6McRouteLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

            vrConfig.supportFcoe = GT_FALSE;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

            retVal = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfig);

            if (GT_ALREADY_EXIST == retVal)
            {
                retVal = GT_OK;
            }

            /* Adding Virtual Router 3, 1 */
            lpmDBId = IP_LPM_DEFAULT_DB_ID_CNS + 2;
            vrId    = IP_LPM_DEFAULT_VR_ID_CNS + 1;

            vrConfig.supportIpv4Uc = GT_TRUE;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 4;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defIpv4UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

            vrConfig.supportIpv4Mc = GT_TRUE;
            vrConfig.defIpv4McRouteLttEntry.routeType = routeType;
            vrConfig.defIpv4McRouteLttEntry.routeEntryBaseIndex = 5;
            vrConfig.defIpv4McRouteLttEntry.numOfPaths = 0;
            vrConfig.defIpv4McRouteLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv4McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
            vrConfig.defIpv4McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;

            vrConfig.supportIpv6Uc = GT_FALSE;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 2;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
            vrConfig.defIpv6UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;

            vrConfig.supportIpv6Mc = GT_FALSE;
            vrConfig.defIpv6McRouteLttEntry.routeType = routeType;
            vrConfig.defIpv6McRouteLttEntry.routeEntryBaseIndex = 3;
            vrConfig.defIpv6McRouteLttEntry.numOfPaths = 0;
            vrConfig.defIpv6McRouteLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defIpv6McRouteLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defIpv6McRouteLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

            vrConfig.supportFcoe = GT_FALSE;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.routeType = routeType;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.routeEntryBaseIndex = 4;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.numOfPaths = 0;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel=CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_TRUE;
            vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_TRUE;
            retVal = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfig);

            if ((GT_OK != retVal) && (GT_ALREADY_EXIST != retVal))
            {
                return retVal;
            }
        }
        firstDev=GT_FALSE;
    }

    return retVal;
}

/**
* @internal prvUtfDeleteLpmDBWithDeviceAndVirtualRouter function
* @endinternal
*
* @brief   This routine delete default LpmDB with devices added to it and
*         Virtual Router for Puma device.
*/
static GT_STATUS prvUtfDeleteLpmDBWithDeviceAndVirtualRouter()
{
    GT_STATUS   retVal  = GT_OK;
    GT_U32      lpmDbId = IP_LPM_DEFAULT_DB_ID_CNS;
    GT_U32      vrId    = IP_LPM_DEFAULT_VR_ID_CNS;


    GT_U8       dev       = 0;
    GT_U8       devListArray[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32      numOfDevs = 0;

    cpssOsBzero((GT_VOID*) &devListArray, sizeof(devListArray));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&dev,retVal, UTF_NONE_FAMILY_E);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        devListArray[numOfDevs++] = dev;
    }

    /* Delete Virtual Router with vrId = 0 and lpmDbId = 1 */
    retVal = cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
    if ((GT_OK != retVal) && (GT_NOT_FOUND != retVal))
    {
        return retVal;
    }

    /* Delete Virtual Router with vrId = 1 and lpmDbId = 1 */
    lpmDbId = 1;
    vrId    = 1;

    retVal = cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
    if ((GT_OK != retVal) && (GT_NOT_FOUND != retVal))
    {
        return retVal;
    }

    lpmDbId = 1;

    retVal = cpssDxChIpLpmDBDevsListRemove(lpmDbId, devListArray,numOfDevs);
    if ((GT_OK != retVal) && (GT_NOT_FOUND != retVal))
    {
        return retVal;
    }

    /* Delete LpmDB with lpmDbId = 1*/
    retVal = cpssDxChIpLpmDBDelete(lpmDbId);
    if ((GT_OK != retVal) && (GT_NOT_FOUND != retVal))
    {
        return retVal;
    }

    /* Delete Virtual Router with vrId = 0 and lpmDbId = 3*/
    lpmDbId = 3;
    vrId    = 0;

    retVal = cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
    if ((GT_OK != retVal) && (GT_NOT_FOUND != retVal))
    {
        return retVal;
    }

    /* Delete Virtual Router with vrId = 1 and lpmDbId = 3*/
    lpmDbId = 3;
    vrId    = 1;

    retVal = cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
    if ((GT_OK != retVal) && (GT_NOT_FOUND != retVal))
    {
        return retVal;
    }

    lpmDbId = 3;

    retVal = cpssDxChIpLpmDBDevsListRemove(lpmDbId, devListArray,numOfDevs);
    if ((GT_OK != retVal) && (GT_NOT_FOUND != retVal))
    {
        return retVal;
    }
    /* Delete LpmDB with lpmDbId = 3 */
    retVal = cpssDxChIpLpmDBDelete(lpmDbId);
    if (GT_NOT_FOUND == retVal)
    {
        retVal = GT_OK;
    }

    return retVal;
}


/**
* @internal prvUtfGetDevShadow function
* @endinternal
*
* @brief   This routine getting dev shadow.
*/
static GT_STATUS prvUtfGetDevShadow(GT_U8 dev,CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT *shadowType)
{

    if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
    {
        *shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
        return GT_OK;
    }

    /* set shadow type according to device family */
    switch (PRV_CPSS_PP_MAC(dev)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            *shadowType = prvUtfIsPbrModeUsed() ? CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E
                                                : CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
            break;
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            *shadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
            break;

        default:
            return GT_BAD_PARAM;
    }
    return GT_OK;
}

