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
* @file prvTgfMcBridgingIpV4.h
*
* @brief Check IPM Bridging mode for Source-Specific
* Multicast (SSM) snooping
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfMcBridgingIpV4h
#define __prvTgfMcBridgingIpV4h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfMcBridgingIpV4ConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfMcBridgingIpV4ConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfMcBridgingIpV4TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfMcBridgingIpV4TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfMcBridgingIpV4ConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfMcBridgingIpV4ConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfMcBridgingIpV4h */


