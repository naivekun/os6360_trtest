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
* @file prvTgfBrgDroppingNonIpV4MacMulticastFiltering.h
*
* @brief Discard all non-Control-classified Ethernet packets
* with a MAC Multicast DA (but not the Broadcast MAC
* address) not corresponding to the IP Multicast range.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgDroppingNonIpV4MacMulticastFilteringh
#define __prvTgfBrgDroppingNonIpV4MacMulticastFilteringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgDroppingNonIpV4MacMulticastFilteringConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgDroppingNonIpV4MacMulticastFilteringConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgDroppingNonIpV4MacMulticastFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgDroppingNonIpV4MacMulticastFilteringTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgDroppingNonIpV4MacMulticastFilteringConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgDroppingNonIpV4MacMulticastFilteringConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgDroppingNonIpV4MacMulticastFilteringh */


