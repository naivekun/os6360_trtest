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
* @file prvTgfDefaultIpv6McRouting.h
*
* @brief Default IPV6 MC Routing
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfDefaultIpv6McRoutingh
#define __prvTgfDefaultIpv6McRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfDefaultIpv6McRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDefaultIpv6McRoutingBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDefaultIpv6McRoutingDefaultLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set default LTT Route Configuration
*/
GT_VOID prvTgfDefaultIpv6McRoutingDefaultLttRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDefaultIpv6McRoutingDefaultTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic to defaults
*/
GT_VOID prvTgfDefaultIpv6McRoutingDefaultTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDefaultIpv6McRoutingAdditionalRouteConfigurationSet function
* @endinternal
*
* @brief   Set additional Route Configuration
*
* @param[in] testNumber               - number of test
*                                       None
*/
GT_VOID prvTgfDefaultIpv6McRoutingAdditionalRouteConfigurationSet
(
    IN GT_U32 testNumber
);

/**
* @internal prvTgfDefaultIpv6McRoutingAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] testNumber               - number of test
*                                       None
*/
GT_VOID prvTgfDefaultIpv6McRoutingAdditionalTrafficGenerate
(
    GT_U32 testNumber
);

/**
* @internal prvTgfDefaultIpv6McRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfDefaultIpv6McRoutingConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfDefaultIpv6McRoutingh */


