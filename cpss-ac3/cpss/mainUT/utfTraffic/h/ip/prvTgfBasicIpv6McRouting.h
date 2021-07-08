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
* @file prvTgfBasicIpv6McRouting.h
*
* @brief Basic IPV6 MC Routing
*
* @version   3
********************************************************************************
*/
#ifndef __prvTgfBasicIpv6McRoutingh
#define __prvTgfBasicIpv6McRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBasicIpv6McRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfBasicIpv6McRoutingBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv6McRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfBasicIpv6McRoutingRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSet function
* @endinternal
*
* @brief   Set Scope Checking Configuration
*
* @param[in] ipv6SourceSiteIdMode     - IPv6 source Site Id Mode
* @param[in] ipv6DestSiteIdMode       - IPv6 destination Site Id Mode
* @param[in] ipv6MCGroupScopeLevel    - IPv6 Multicast group scope level
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
* @param[in] scopeCommand             - action to be done on a packet that match the above
*                                      scope configuration.
*                                       None
*/
GT_VOID prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSet
(
    CPSS_IP_SITE_ID_ENT         ipv6SourceSiteIdMode,
    CPSS_IP_SITE_ID_ENT         ipv6DestSiteIdMode,
    CPSS_IPV6_PREFIX_SCOPE_ENT  ipv6MCGroupScopeLevel,
    GT_BOOL                     borderCrossed,
    CPSS_PACKET_CMD_ENT         scopeCommand
);

/**
* @internal prvTgfBasicIpv6McRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectNoTraffic          - whether to expect no traffic
*                                       None
*/
GT_VOID prvTgfBasicIpv6McRoutingTrafficGenerate
(
    GT_BOOL  expectNoTraffic
);

/**
* @internal prvTgfBasicIpv6McRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv6McRoutingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSave function
* @endinternal
*
* @brief   Save Scope Checking configuration
*/
GT_VOID prvTgfBasicIpv6McRoutingScopeCheckingConfigurationSave
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv6McRoutingScopeCheckingConfigurationRestore function
* @endinternal
*
* @brief   Restore Scope Checking configuration
*/
GT_VOID prvTgfBasicIpv6McRoutingScopeCheckingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfIpv6McRollBackCheck function
* @endinternal
*
* @brief   This functions cause to failers during prefix insertion and
*         check rollback
*/
GT_VOID  prvTgfIpv6McRollBackCheck
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIpv6McRoutingh */


