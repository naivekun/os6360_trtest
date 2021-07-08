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
* @file prvTgfFdbPerEgressPortUnregIpv6BcFiltering.h
*
* @brief Verify per-Egress port filtering for unregistered IPv6 BC packets.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbPerEgressPortUnregIpv6BcFilteringh
#define __prvTgfFdbPerEgressPortUnregIpv6BcFilteringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfFdbPerEgressPortUnregIpv6BcFilteringConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbPerEgressPortUnregIpv6BcFilteringConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerEgressPortUnregIpv6BcFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbPerEgressPortUnregIpv6BcFilteringTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerEgressPortUnregIpv6BcFilteringConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbPerEgressPortUnregIpv6BcFilteringConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbPerEgressPortUnregIpv6BcFilteringh */


