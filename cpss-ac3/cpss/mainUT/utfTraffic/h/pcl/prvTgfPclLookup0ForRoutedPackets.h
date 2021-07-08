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
* @file prvTgfPclLookup0ForRoutedPackets.h
*
* @brief Enable/disable PCL lookup 0 for routed packets.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPclLookup0ForRoutedPacketsh
#define __prvTgfPclLookup0ForRoutedPacketsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclLookup0ForRoutedPacketsConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclLookup0ForRoutedPacketsConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclLookup0ForRoutedPacketsMatchConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclLookup0ForRoutedPacketsMatchConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclLookup0ForRoutedPacketsConfigurationEnableSet function
* @endinternal
*
* @brief   Enable/disable PCL lookup 0 for routed packets.
*
* @param[in] enable                   - GT_TRUE:   the TTI lookup for IPv4 multicast
*                                      - GT_FALSE: disable the TTI lookup for IPv4 multicast
*                                       None
*/
GT_VOID prvTgfPclLookup0ForRoutedPacketsConfigurationEnableSet
(
    IN GT_BOOL     enable
);

/**
* @internal prvTgfPclLookup0ForRoutedPacketsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectTraffic            - GT_TRUE: expect traffic
*                                      GT_FALSE: expect no traffic
*                                       None
*/
GT_VOID prvTgfPclLookup0ForRoutedPacketsTrafficGenerate
(
    GT_BOOL     expectTraffic
);

/**
* @internal prvTgfPclLookup0ForRoutedPacketsConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore PCL Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfPclLookup0ForRoutedPacketsConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclLookup0ForRoutedPacketsh */


