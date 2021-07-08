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
* @file prvTgfIpLpmDBCapacityUpdate.h
*
* @brief Basic IPV4 UC Routing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpLpmDBCapacityUpdateh
#define __prvTgfIpLpmDBCapacityUpdateh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfIpLpmDBCapacityUpdateBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpLpmDBCapacityUpdateBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpLpmDBCapacityUpdateRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfIpLpmDBCapacityUpdateRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpLpmDBCapacityUpdateRouteConfigurationUpdate function
* @endinternal
*
* @brief   Update Route Configuration
*/
GT_VOID prvTgfIpLpmDBCapacityUpdateRouteConfigurationUpdate
(
    GT_VOID
);

/**
* @internal prvTgfIpLpmDBCapacityUpdateTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpLpmDBCapacityUpdateTrafficGenerate
(
    GT_BOOL isPacketMatched
);

/**
* @internal prvTgfIpLpmDBCapacityUpdateConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfIpLpmDBCapacityUpdateConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpLpmDBCapacityUpdateh */


