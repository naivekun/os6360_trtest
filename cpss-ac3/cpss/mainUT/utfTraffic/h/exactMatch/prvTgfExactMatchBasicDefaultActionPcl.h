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
* @file prvTgfExactMatchBasicDefaultActionPcl.h
*
* @brief Test Exact Match Default Action functionality with PCL Action Type
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchBasicDefaultActionPclh
#define __prvTgfExactMatchBasicDefaultActionPclh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>

/**
* @internal prvTgfExactMatchBasicDefaultActionPclGenericConfig function
* @endinternal
*
* @brief   Set PCL generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclGenericConfig
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchBasicDefaultActionPclGenericRestore function
* @endinternal
*
* @brief   Restore PCL generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclGenericRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchBasicDefaultActionPclInvalidateRule function
* @endinternal
*
* @brief   Invalidate PCL Rule
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclInvalidateRule
(
    GT_VOID
);
/**
* @internal GT_VOID prvTgfExactMatchBasicDefaultActionPclConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Default Action
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclConfigSet
(
    GT_VOID
);
/**
* @internal GT_VOID prvTgfExactMatchBasicDefaultActionPclConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Default Action
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclConfigRestore
(
    GT_VOID
);
/**
* @internal GT_VOID prvTgfExactMatchBasicDefaultActionPclTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic, expect traffic to be trapped or dropped
*/
GT_VOID prvTgfExactMatchBasicDefaultActionPclTrafficGenerate
(
    GT_BOOL     expectNoTraffic
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
