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
* @file prvTgfPortMruCheck.h
*
* @brief Port Profile MRU check - basic
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPortMruCheckh
#define __prvTgfPortMruCheckh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfPortProfileMruCheckConfigSet function
* @endinternal
*
* @brief   Set test configuration
*
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..7)
*                                       None
*/
GT_VOID prvTgfPortProfileMruCheckConfigSet
(
    IN GT_U32           profileId
);

/**
* @internal prvTgfPortProfileMruCheckTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPortProfileMruCheckTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPortProfileMruCheckConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..7)
*                                       None
*/
GT_VOID prvTgfPortProfileMruCheckConfigRestore
(
    IN GT_U32           profileId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortProfileMruCheckh */




