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
* @file prvTgfGlobalEport.h
*
* @brief L2 MLL Global ePort test definitions
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfGlobalEporth
#define __prvTgfGlobalEporth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfGlobalEportConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfGlobalEportConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfGlobalEportTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfGlobalEportTrafficGenerate
(
    GT_VOID
);
    
/**
* @internal prvTgfGlobalEportConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfGlobalEportConfigurationRestore
(
    GT_VOID
);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfGlobalEporth */


