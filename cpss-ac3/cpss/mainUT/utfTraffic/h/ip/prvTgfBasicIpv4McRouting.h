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
* @file prvTgfBasicIpv4McRouting.h
*
* @brief Basic IPV4 UC Routing
*
* @version   9
********************************************************************************
*/
#ifndef __prvTgfBasicIpv4McRoutingh
#define __prvTgfBasicIpv4McRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfIpGen.h>

/**
* @internal prvTgfBasicIpv4McRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfBasicIpv4McRoutingBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4McRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfBasicIpv4McRoutingRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource function
* @endinternal
*
* @brief   Set source and group for the packet and for the mc entry
*/
GT_VOID prvTgfBasicIpv4McRoutingDifferentMcEntriesSetGroupAndSource
(
    IN GT_IPADDR                    ipGrp,
    IN GT_U32                       ipGrpPrefixLen,
    IN GT_IPADDR                    ipSrc,
    IN GT_U32                       ipSrcPrefixLen,
    IN GT_BOOL                      flushMcTable
);

/**
* @internal prvTgfBasicIpv4McRoutingRouteConfigurationMllSilentDropSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] overwriteTtl             - GT_TRUE: Just overwrite the TTL value
*                                      GT_FALSE: do full configuration
* @param[in] ttlHopLimitThresholdValue - ttl value to set to L2 mll elements
*                                       None
*/
GT_VOID prvTgfBasicIpv4McRoutingRouteConfigurationMllSilentDropSet
(
    GT_BOOL     overwriteTtl,
    GT_U32      ttlHopLimitThresholdValue
);

/**
* @internal prvTgfBasicIpv4McRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBasicIpv4McRoutingTrafficGenerate
(
    GT_BOOL expectTraffic
);

/**
* @internal prvTgfBasicIpv4McRoutingMllSilentDropTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectSilentDrop         - GT_TRUE: expect silent drop packets - no traffic
*                                      GT_FALSE: expect NO silent drop packets - traffic exists
*                                       None
*/
GT_VOID prvTgfBasicIpv4McRoutingMllSilentDropTrafficGenerate
(
    GT_BOOL     expectSilentDrop
);

/**
* @internal prvTgfBasicIpv4McRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv4McRoutingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4McRoutingForDualHwDevConfigurationInit function
* @endinternal
*
* @brief   Set configuration for multi hemisphere devices:
*         - set specific ports numbers
*/
GT_VOID prvTgfBasicIpv4McRoutingForDualHwDevConfigurationInit
(
    GT_VOID
);


/**
* @internal prvTgfBasicIpv4McRoutingForDualHwDevConfigurationRestore function
* @endinternal
*
* @brief   Restore config for multi hemisphere devices
*/
GT_VOID prvTgfBasicIpv4McRoutingForDualHwDevConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4McRoutingRouteConfigurationDifferentPrefixLengthSet function
* @endinternal
*
* @brief   Set Prefix with different length configuration
*/
GT_VOID prvTgfBasicIpv4McRoutingRouteConfigurationDifferentPrefixLengthSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4McRoutingTrafficDifferentPrefixLengthGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBasicIpv4McRoutingTrafficDifferentPrefixLengthGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4McRoutingConfigurationDifferentPrefixLengthRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfBasicIpv4McRoutingConfigurationDifferentPrefixLengthRestore
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4McRoutingDifferentMcEntries function
* @endinternal
*
* @brief   main test func of tgfBasicIpv4McRoutingDifferentMcEntries
*/
GT_VOID prvTgfBasicIpv4McRoutingDifferentMcEntries
(
    GT_VOID
);


/**
* @internal prvTgfIpv4McRollBackCheck function
* @endinternal
*
* @brief   This functions cause to failers during prefix insertion and
*         check rollback
*/
GT_VOID  prvTgfIpv4McRollBackCheck
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIpv4McRoutingh */


