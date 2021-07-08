/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *2
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvTgfExactMatchBasicDefaultActionTti.h
*
* @brief Test Exact Match Default Action functionality with TTI Action Type
*
* @version   1
********************************************************************************
*/

#ifndef __prvTgfExactMatchBasicDefaultActionTtih
#define __prvTgfExactMatchBasicDefaultActionTtih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>
#include <utf/private/prvUtfExtras.h>

/**
* @internal prvTgfExactMatchBasicDefaultActionTtiGenericConfigSet function
* @endinternal
*
* @brief   Set TTI generic test configuration, not related to Exact Match
*
* @note Used 4 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiGenericConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchBasicDefaultActionTtiGenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiGenericConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchBasicDefaultActionTtiConfigSet function
* @endinternal
*
* @brief Set TTI test configuration related to Exact Match Default Action
*
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchBasicDefaultActionTtiConfigRestore function
* @endinternal
*
* @brief   Restore TTI test configuration related to Exact Match
*          Default Action
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchBasicDefaultActionTtiCommandTrap function
* @endinternal
*
* @brief   set default action command to be trap with cpu code [502]
*
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiCommandTrap
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchBasicDefaultActionTtiRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM
*
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiRuleValidStatusSet
(
    GT_BOOL state
);

/**
* @internal prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate function
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
GT_VOID prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate
(
    GT_BOOL     expectTrapTraffic,
    GT_BOOL     expectForwardTraffic
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvTgfExactMatchBasicDefaultActionTtih */


