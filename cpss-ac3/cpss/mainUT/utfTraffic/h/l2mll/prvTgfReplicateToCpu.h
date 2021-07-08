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
* @file prvTgfReplicateToCpu.h
*
* @brief L2 MLL MC replicate to CPU test definitions
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfReplicateToCpuh
#define __prvTgfReplicateToCpuh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfReplicateToCpuConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfReplicateToCpuConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfReplicateToCpuTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfReplicateToCpuTrafficGenerate
(
    GT_VOID
);
    
/**
* @internal prvTgfReplicateToCpuConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfReplicateToCpuConfigurationRestore
(
    GT_VOID
);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfReplicateToCpuh */


