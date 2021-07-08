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
* @file prvTgfPclPolicer.h
*
* @brief Second Lookup match
*
* @version   4
********************************************************************************
*/
#ifndef __prvTgfPclPolicer
#define __prvTgfPclPolicer

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclPolicerCounting function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPolicerCounting
(
    GT_VOID
);

/**
* @internal prvTgfPclPolicerMetering function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPolicerMetering
(
    GT_VOID
);

/**
* @internal prvTgfPclPolicerCountingFlowIdBased function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPolicerCountingFlowIdBased
(
   GT_VOID
);

/**
* @internal prvTgfPclPolicerCountingHierarchicalTest function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPolicerCountingHierarchicalTest
(
   GT_VOID
);

/**
* @internal prvTgfPclPolicerCountingHierarchicalReset function
* @endinternal
*
* @brief   Reset test configuration
*/
GT_VOID prvTgfPclPolicerCountingHierarchicalReset
(
   GT_VOID
);

/**
* @internal prvTgfPclPolicerDp2CfiMapTest function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPolicerDp2CfiMapTest
(
   GT_VOID
);

/**
* @internal prvTgfPclPolicerDp2CfiMapRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclPolicerDp2CfiMapRestore
(
   GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclPolicer */

