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
* @file prvTgfExactMatchBasicExpandedActionTti.h
*
* @brief Test Exact Match Expanded Action functionality with TTI Action Type
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchBasicExpandedActionTtih
#define __prvTgfExactMatchBasicExpandedActionTtih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>
#include <utf/private/prvUtfExtras.h>

/**
* @internal prvTgfExactMatchBasicExpandedActionTtiGenericConfigSet function
* @endinternal
*
* @brief   Set TTI generic test configuration, not related to Exact Match
*
* @note Used 4 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfExactMatchBasicExpandedActionTtiGenericConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchBasicExpandedActionGenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchBasicExpandedActionTtiGenericConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchBasicExpandedActionTtiConfigSet function
* @endinternal
*
* @brief Set TTI test configuration related to Exact Match Default Action
*
*/
GT_VOID prvTgfExactMatchBasicExpandedActionTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchBasicExpandedActionTtiConfigRestore function
* @endinternal
*
* @brief   Restore TTI test configuration related to Exact Match
*          Default Action
*/
GT_VOID prvTgfExactMatchBasicExpandedActionTtiConfigRestore
(
    GT_VOID
);


/**
* @internal prvTgfExactMatchBasicExpandedActionTtiInvalidateEmEntry function
* @endinternal
*
* @brief   Invalidate Exact Match Entry
*/
GT_VOID prvTgfExactMatchBasicExpandedActionTtiInvalidateEmEntry
(
    GT_VOID
);


/**
* @internal prvTgfExactMatchBasicExpandedActionTtiExactMatchPriorityConfig function
* @endinternal
*
* @brief   Set TCAM Priority over Exact Match
* @param[in] exactMatchOverTtiEn - GT_FALSE: TCAM priority over Exact Match
*                                - GT_TRUE : Exact Match priority over TCAM
*/
GT_VOID prvTgfExactMatchBasicExpandedActionTtiExactMatchPriorityConfig
(
    GT_BOOL exactMatchOverTtiEn
);

/**
* @internal prvTgfExactMatchBasicExpandedActionTtiRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM
*
*/
GT_VOID prvTgfExactMatchBasicExpandedActionTtiRuleValidStatusSet
(
    GT_BOOL state
);

/**
* @internal prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
* @param[in] expectTrapTraffic   - GT_FALSE: expect no traffic
*                                - GT_TRUE: expect traffic to be trapped
*
* @param[in] expectForwardTraffic - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be forwarded
*
*/
GT_VOID prvTgfExactMatchBasicExpandedActionTtiTrafficGenerate
(
    GT_BOOL     expectTrapTraffic,
    GT_BOOL     expectForwardTraffic
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfExactMatchBasicExpandedActionTtih */

