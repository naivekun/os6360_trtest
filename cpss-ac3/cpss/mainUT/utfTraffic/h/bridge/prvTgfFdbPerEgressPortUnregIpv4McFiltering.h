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
* @file prvTgfFdbPerEgressPortUnregIpv4McFiltering.h
*
* @brief Verify per-Egress port filtering for unregistered IPv4 MC packets.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbPerEgressPortUnregIpv4McFilteringh
#define __prvTgfFdbPerEgressPortUnregIpv4McFilteringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfFdbPerEgressPortUnregIpv4McFilteringConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbPerEgressPortUnregIpv4McFilteringConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerEgressPortUnregIpv4McFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbPerEgressPortUnregIpv4McFilteringTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerEgressPortUnregIpv4McFilteringConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbPerEgressPortUnregIpv4McFilteringConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbPerEgressPortUnregIpv4McFilteringh */


