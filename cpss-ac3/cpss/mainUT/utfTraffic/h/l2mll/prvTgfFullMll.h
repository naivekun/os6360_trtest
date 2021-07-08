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
* @file prvTgfFullMll.h
*
* @brief L2 MLL full Mll table test definitions
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfFullMllh
#define __prvTgfFullMllh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfFullMllConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfFullMllConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFullMllTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfFullMllTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFullMllConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfFullMllConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfFullMllOneListConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfFullMllOneListConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFullMllOneListTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfFullMllOneListTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFullMllOneListConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfFullMllOneListConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFullMllh */


