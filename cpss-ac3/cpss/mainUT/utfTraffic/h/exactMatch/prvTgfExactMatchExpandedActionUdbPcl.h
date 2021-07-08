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
* @file prvTgfExactMatchExpandedActionUdbPcl.h
*
* @brief Test Exact Match Expanded Action functionality with PCL Action Type
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchExpandedActionUdbPclh
#define __prvTgfExactMatchExpandedActionUdbPclh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>

/**
* @internal prvTgfExactMatchBasicExpandedDefaultActionPclInvalidateRule function
* @endinternal
*
* @brief   Invalidate PCL Rule
*/
GT_VOID prvTgfExactMatchExpandedActionUdbPclInvalidateRule
(
    GT_VOID
);
/**
* @internal GT_VOID prvTgfExactMatchExpandedActionUdbPclConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*/
GT_VOID prvTgfExactMatchExpandedActionUdbPclConfigSet
(
    GT_VOID
);
/**
* @internal GT_VOID prvTgfExactMatchExpandedActionUdbPclConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Expanded Action
*/
GT_VOID prvTgfExactMatchExpandedActionUdbPclConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchExpandedActionUdbPclInvalidateEmEntry function
* @endinternal
*
* @brief   Invalidate Exact Match Entry
*/
GT_VOID prvTgfExactMatchExpandedActionUdbPclInvalidateEmEntry
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchExpandedActionUdbPclExactMatchPriorityConfig function
* @endinternal
*
* @brief   Set TCAM Priority over Exact Match
* @param[in] exactMatchOverPclEn - GT_FALSE: TCAM priority over Exact Match
*                                - GT_TRUE: Exact Match priority over TCAM
*/
GT_VOID prvTgfExactMatchExpandedActionUdbPclExactMatchPriorityConfig
(
    GT_BOOL exactMatchOverPclEn
);
/**
* @internal GT_VOID prvTgfExactMatchExpandedActionUdbPclTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic, expect traffic to be trapped,dropped,forwarded
* @param[in] expectTrapTraffic   - GT_FALSE: expect no traffic
*                                - GT_TRUE: expect traffic to be trapped
*
* @param[in] expectForwardTraffic - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be forwarded
*
*/
GT_VOID prvTgfExactMatchExpandedActionUdbPclTrafficGenerate
(
    GT_BOOL     expectTrapTraffic,
    GT_BOOL     expectForwardTraffic
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
