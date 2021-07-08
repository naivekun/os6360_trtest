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
* @file prvTgfBrgIngressCountersSpanningTreeStateDrop.h
*
* @brief Verify that packets that are dropped due to Spanning Tree state
* drop are counted in field<LocalPropDisc> in counter-set0 and
* counter-set1 on different modes
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgIngressCountersSpanningTreeStateDroph
#define __prvTgfBrgIngressCountersSpanningTreeStateDroph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgIngressCountersSpanningTreeStateDropConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgIngressCountersSpanningTreeStateDropConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgIngressCountersSpanningTreeStateDropTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgIngressCountersSpanningTreeStateDropTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgIngressCountersSpanningTreeStateDropConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgIngressCountersSpanningTreeStateDropConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgIngressCountersSpanningTreeStateDroph */


