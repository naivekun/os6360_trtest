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
* @file prvTgfLttEvidxRange.h
*
* @brief L2 MLL LTT eVIDX Range test definitions
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfLttEvidxRangeh
#define __prvTgfLttEvidxRangeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfLttEvidxRangeConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfLttEvidxRangeConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfLttEvidxRangeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfLttEvidxRangeTrafficGenerate
(
    GT_VOID
);
    
/**
* @internal prvTgfLttEvidxRangeConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfLttEvidxRangeConfigurationRestore
(
    GT_VOID
);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfLttEvidxRangeh */


