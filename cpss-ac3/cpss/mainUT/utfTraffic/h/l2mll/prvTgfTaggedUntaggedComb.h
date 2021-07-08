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
* @file prvTgfTaggedUntaggedComb.h
*
* @brief L2 MLL Tagged\Untagged for Ingress\Egress combinations test definitions
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTaggedUntaggedCombh
#define __prvTgfTaggedUntaggedCombh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTaggedUntaggedCombConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfTaggedUntaggedCombConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTaggedUntaggedCombTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTaggedUntaggedCombTrafficGenerate
(
    GT_VOID
);
    
/**
* @internal prvTgfTaggedUntaggedCombConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfTaggedUntaggedCombConfigurationRestore
(
    GT_VOID
);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTaggedUntaggedCombh */


