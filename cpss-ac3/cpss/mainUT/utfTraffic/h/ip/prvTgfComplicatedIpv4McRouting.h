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
* @file prvTgfComplicatedIpv4McRouting.h
*
* @brief Complicated IPV4 MC Routing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfComplicatedIpv4McRoutingh
#define __prvTgfComplicatedIpv4McRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfComplicatedIpv4McRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfComplicatedIpv4McRoutingBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfComplicatedIpv4McRoutingDefaultLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set default LTT Route Configuration
*/
GT_VOID prvTgfComplicatedIpv4McRoutingDefaultLttRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfComplicatedIpv4McRoutingDefaultTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic to defaults
*/
GT_VOID prvTgfComplicatedIpv4McRoutingDefaultTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfComplicatedIpv4McRoutingAdditionalRouteConfigurationSet function
* @endinternal
*
* @brief   Set additional Route Configuration
*/
GT_VOID prvTgfComplicatedIpv4McRoutingAdditionalRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfComplicatedIpv4McRoutingAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfComplicatedIpv4McRoutingAdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfComplicatedIpv4McRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfComplicatedIpv4McRoutingConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfComplicatedIpv4McRoutingh */


