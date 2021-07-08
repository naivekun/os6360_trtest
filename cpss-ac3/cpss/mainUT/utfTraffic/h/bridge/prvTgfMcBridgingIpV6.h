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
* @file prvTgfMcBridgingIpV6.h
*
* @brief Check IPM Bridging mode for Any-Source Multicast
* (ASM) snooping
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfMcBridgingIpV6h
#define __prvTgfMcBridgingIpV6h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfMcBridgingIpV6ConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfMcBridgingIpV6ConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfMcBridgingIpV6TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfMcBridgingIpV6TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfMcBridgingIpV6ConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfMcBridgingIpV6ConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfMcBridgingIpV6h */


