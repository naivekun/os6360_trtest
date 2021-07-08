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
* @file prvTgfMaskProfileSkipAndCounters.h
*
* @brief L2 MLL Mask Profile, Skip counter & MLL counters test definitions
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfMaskProfileSkipAndCountersh
#define __prvTgfMaskProfileSkipAndCountersh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfMaskProfileSkipAndCountersConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfMaskProfileSkipAndCountersConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfMaskProfileSkipAndCountersTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfMaskProfileSkipAndCountersTrafficGenerate
(
    GT_VOID
);
    
/**
* @internal prvTgfMaskProfileSkipAndCountersConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfMaskProfileSkipAndCountersConfigurationRestore
(
    GT_VOID
);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfMaskProfileSkipAndCountersh */


