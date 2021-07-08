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
* @file prvTgfFdbSecureAutoLearnMovedSa.h
*
* @brief Test packets with moved SA dropping with Secure Auto Learn.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbSecureAutoLearnMovedSah
#define __prvTgfFdbSecureAutoLearnMovedSah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfFdbSecureAutoLearnMovedSaConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbSecureAutoLearnMovedSaConfigSet
(
    GT_VOID
);


/**
* @internal prvTgfFdbSecureAutoLearnMovedSaTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbSecureAutoLearnMovedSaTrafficGenerate
(
    GT_VOID
);


/**
* @internal prvTgfFdbSecureAutoLearnMovedSaConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbSecureAutoLearnMovedSaConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbSecureAutoLearnMovedSah */


