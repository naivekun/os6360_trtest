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
* @file prvTgfFdbPerInlifPortUnregIpv6McFiltering.h
*
* @brief Verify MC IPv6 packets filtering thru InLIF port commandset.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbPerInlifPortUnregIpv6McFilteringh
#define __prvTgfFdbPerInlifPortUnregIpv6McFilteringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfFdbPerInlifPortUnregIpv6McFilteringConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbPerInlifPortUnregIpv6McFilteringConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerInlifPortUnregIpv6McFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbPerInlifPortUnregIpv6McFilteringTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerInlifPortUnregIpv6McFilteringConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbPerInlifPortUnregIpv6McFilteringConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbPerInlifPortUnregIpv6McFilteringh */


