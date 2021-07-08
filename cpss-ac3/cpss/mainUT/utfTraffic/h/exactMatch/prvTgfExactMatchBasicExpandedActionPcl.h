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
* @file prvTgfExactMatchBasicExpandedActionPcl.h
*
* @brief Test Exact Match Expanded Action functionality with PCL Action Type
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchBasicExpandedActionPclh
#define __prvTgfExactMatchBasicExpandedActionPclh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>

/**
* @internal prvTgfExactMatchBasicExpandedActionPclGenericConfig
*           function
* @endinternal
*
* @brief   Set PCL generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchBasicExpandedActionPclGenericConfig
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchBasicExpandedActionPclGenericRestore function
* @endinternal
*
* @brief   Restore PCL generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchBasicExpandedActionPclGenericRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchBasicExpandedDefaultActionPclInvalidateRule function
* @endinternal
*
* @brief   Invalidate PCL Rule
*/
GT_VOID prvTgfExactMatchBasicExpandedActionPclInvalidateRule
(
    GT_VOID
);
/**
* @internal GT_VOID prvTgfExactMatchBasicExpandedActionPclConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*/
GT_VOID prvTgfExactMatchBasicExpandedActionPclConfigSet
(
    GT_VOID
);
/**
* @internal GT_VOID prvTgfExactMatchBasicExpandedActionPclConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Expanded Action
*/
GT_VOID prvTgfExactMatchBasicExpandedActionPclConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchBasicExpandedActionPclInvalidateEmEntry function
* @endinternal
*
* @brief   Invalidate Exact Match Entry
*/
GT_VOID prvTgfExactMatchBasicExpandedActionPclInvalidateEmEntry
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchBasicExpandedActionPclExactMatchPriorityConfig function
* @endinternal
*
* @brief   Set TCAM Priority over Exact Match
* @param[in] exactMatchOverPclEn - GT_FALSE: TCAM priority over Exact Match
*                                - GT_TRUE: Exact Match priority over TCAM
*/
GT_VOID prvTgfExactMatchBasicExpandedActionPclExactMatchPriorityConfig
(
    GT_BOOL exactMatchOverPclEn
);
/**
* @internal GT_VOID prvTgfExactMatchBasicExpandedActionPclTrafficGenerate function
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
GT_VOID prvTgfExactMatchBasicExpandedActionPclTrafficGenerate
(
    GT_BOOL     expectTrapTraffic,
    GT_BOOL     expectForwardTraffic
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
