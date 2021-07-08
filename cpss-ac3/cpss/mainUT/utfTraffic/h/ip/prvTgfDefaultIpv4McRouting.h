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
* @file prvTgfDefaultIpv4McRouting.h
*
* @brief Default IPV4 MC Routing
*
* @version   4
********************************************************************************
*/
#ifndef __prvTgfDefaultIpv4McRoutingh
#define __prvTgfDefaultIpv4McRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfDefaultIpv4McRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDefaultIpv4McRoutingBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDefaultIpv4McRoutingDefaultLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set default LTT Route Configuration
*/
GT_VOID prvTgfDefaultIpv4McRoutingDefaultLttRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDefaultIpv4McRoutingDefaultTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic to defaults
*/
GT_VOID prvTgfDefaultIpv4McRoutingDefaultTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDefaultIpv4McRoutingAdditionalRouteConfigurationSet function
* @endinternal
*
* @brief   Set additional Route Configuration
*
* @param[in] testNumber               - number of test
*                                       None
*/
GT_VOID prvTgfDefaultIpv4McRoutingAdditionalRouteConfigurationSet
(
    GT_U32 testNumber
);

/**
* @internal prvTgfDefaultIpv4McRoutingAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] testNumber               - number of test
*                                       None
*/
GT_VOID prvTgfDefaultIpv4McRoutingAdditionalTrafficGenerate
(
    GT_U32 testNumber
);

/**
* @internal prvTgfDefaultIpv4McRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfDefaultIpv4McRoutingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfDefaultIpv4McRoutingForDualHwDevConfigurationInit function
* @endinternal
*
* @brief   Set configuration for multi hemisphere devices:
*         - set specific ports numbers
*/
GT_VOID prvTgfDefaultIpv4McRoutingForDualHwDevConfigurationInit
(
    GT_VOID
);

/**
* @internal prvTgfDefaultIpv4McRoutingForDualHwDevConfigurationRestore function
* @endinternal
*
* @brief   Restore config for multi hemisphere devices
*/
GT_VOID prvTgfDefaultIpv4McRoutingForDualHwDevConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfDefaultIpv4McRoutingMllBridgeEnableTraffic function
* @endinternal
*
* @brief   Generate traffic with MLL based bridging
*/
GT_VOID prvTgfDefaultIpv4McRoutingMllBridgeEnableTraffic
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfDefaultIpv4McRoutingh */


