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
* @file prvTgfFdbPerInlifPortUnregIpv4BcFiltering.h
*
* @brief Verify BC IPv4 packets filtering thru InLIF port commandset.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbPerInlifPortUnregIpv4BcFilteringh
#define __prvTgfFdbPerInlifPortUnregIpv4BcFilteringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfFdbPerInlifPortUnregIpv4BcFilteringConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbPerInlifPortUnregIpv4BcFilteringConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerInlifPortUnregIpv4BcFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbPerInlifPortUnregIpv4BcFilteringTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerInlifPortUnregIpv4BcFilteringConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbPerInlifPortUnregIpv4BcFilteringConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbPerInlifPortUnregIpv4BcFilteringh */


