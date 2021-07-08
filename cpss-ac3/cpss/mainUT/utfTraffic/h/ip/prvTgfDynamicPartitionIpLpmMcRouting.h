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
* @file prvTgfDynamicPartitionIpLpmMcRouting.h
*
* @brief Checking Dynamic Partition of IPv4 MC
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfDynamicPartitionIpLpmMcRoutingh
#define __prvTgfDynamicPartitionIpLpmMcRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfDynamicPartitionIpLpmDbMcConfigurationSet function
* @endinternal
*
* @brief   Set IP LPM DB Base Configuration
*
* @param[in] lastLineIndex            - index of the last line in the TCAM,
*                                      used to create the new LPP DB
*                                       None
*/
GT_VOID prvTgfDynamicPartitionIpLpmDbMcConfigurationSet
(
    GT_U32 lastLineIndex
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragMcBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragMcBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragMcRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Base Configuration
*
* @param[in] protocol                 -  to use in prvTgfIpMcRouteEntriesWrite, affects mll definition
*                                       None
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragMcRouteConfigurationSet
(
     IN CPSS_IP_PROTOCOL_STACK_ENT protocol
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragIpv4McRouting function
* @endinternal
*
* @brief   A specific test that fit a 6 lines TCAM, the test add and delete prefixes
*         in such a way that we will have move of prefixes location when adding new
*         entries.
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv4McRouting
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragIpv6McRouting function
* @endinternal
*
* @brief   A specific test that fit a 7 lines TCAM, the test add and delete prefixes
*         in such a way that we will have move of prefixes location when adding new
*         entries.
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv6McRouting
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragIpv4McRoutingMovingDefault function
* @endinternal
*
* @brief   A specific test that fit a 6 lines TCAM, the test add and delete prefixes
*         in such a way that we will have move of prefixes location when adding new
*         entries.
*         The scenario in built in a way that the default entry must be moved.
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv4McRoutingMovingDefault
(
    GT_VOID
);

/**
* @internal prvTgfIpMcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] isIpv4Traffic            -     GT_TRUE:  generate ipv4 traffic,
*                                      GT_FALSE: generate ipv6 traffic.
* @param[in] ipAddrPtr                -     (pointer to) the IP address in case we want to send
*                                      traffic different from the default
* @param[in] ipGroupPrefixLen         - prefix len of ipv4 MC group
* @param[in] ipSrcAddrPtr             -  (pointer to) the IP src address in case we want to send
*                                      traffic different from the default
* @param[in] ipSrcPrefixLen           - prefix len of ipv4 MC source
*                                       None
*/
GT_VOID prvTgfIpMcRoutingTrafficGenerate
(
   IN GT_BOOL               isIpv4Traffic,
   IN GT_IP_ADDR_TYPE_UNT   *ipAddrPtr,
   IN GT_U32                ipGroupPrefixLen,
   IN GT_IP_ADDR_TYPE_UNT   *ipSrcAddrPtr,
   IN GT_U32                ipSrcPrefixLen
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragIpv4McRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv4McRoutingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragIpv6McRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv6McRoutingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDbMcConfigurationRestore function
* @endinternal
*
* @brief   Restore IP LPM DB Base Configuration
*/
GT_VOID prvTgfDynamicPartitionIpLpmDbMcConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragMcRoutingRestore function
* @endinternal
*
* @brief   Clean test scenario from costumer
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragMcRoutingRestore
(
    GT_VOID
);

/**
* @internal prvTgfDynamicPartitionIpLpmDefragMcRoutingRandomCostumerTestScenario function
* @endinternal
*
* @brief   Test scenario from costumer
*         A random IP v4/v6 MC prefixes Add/Delete Test.
*         we need to configure before cpssInitSystem:
*         lpmDbPartitionEnable=0x0
*         lpmDbFirstTcamLine=0
*         lpmDbLastTcamLine=2047
*         maxNumOfIpv4Prefixes=1
*         maxNumOfIpv4McEntries=1
*         maxNumOfIpv6Prefixes=1
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragMcRoutingRandomCostumerTestScenario
(
    GT_VOID
);


/**
* @internal prvTgfDynamicPartitionIpLpmDefragIpv4UcIpv6McRoutingRandomCostumerTestScenario function
* @endinternal
*
* @brief   Test scenario from costumer
*         A random IP v4/v6 MC prefixes Add/Delete Test.
*         we need to configure before cpssInitSystem:
*         lpmDbPartitionEnable=0x0
*         lpmDbFirstTcamLine=0
*         lpmDbLastTcamLine=1000
*         maxNumOfIpv4Prefixes=1
*         maxNumOfIpv4McEntries=1
*         maxNumOfIpv6Prefixes=1
*/
GT_VOID prvTgfDynamicPartitionIpLpmDefragIpv4UcIpv6McRoutingRandomCostumerTestScenario
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfDynamicPartitionIpLpmMcRoutingh */


