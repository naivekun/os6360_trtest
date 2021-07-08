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
* @file prvTgfBasicIpv4UcLowLevelRouting.h
*
* @brief Basic IPV4 UC Routing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBasicIpv4UcLowLevelRoutingh
#define __prvTgfBasicIpv4UcLowLevelRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBasicIpv4UcLowLevelRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfBasicIpv4UcLowLevelRoutingBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4UcLowLevelRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfBasicIpv4UcLowLevelRoutingRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4UcLowLevelRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBasicIpv4UcLowLevelRoutingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4UcLowLevelRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv4UcLowLevelRoutingConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIpv4UcLowLevelRoutingh */


