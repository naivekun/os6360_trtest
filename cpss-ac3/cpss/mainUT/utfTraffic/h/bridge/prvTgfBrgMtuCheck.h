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
* @file prvTgfBrgMtuCheck.h
*
* @brief target ePort MTU check - basic
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgMtuCheckh
#define __prvTgfBrgMtuCheckh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgGenMtuCheckConfigSet function
* @endinternal
*
* @brief   Set test configuration
*
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..3)
*                                       None
*/
GT_VOID prvTgfBrgGenMtuCheckConfigSet
(
    IN GT_U32   profileId
);

/**
* @internal prvTgfBrgGenMtuCheckTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgGenMtuCheckTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgGenMtuCheckConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..3)
*                                       None
*/
GT_VOID prvTgfBrgGenMtuCheckConfigRestore
(
    IN GT_U32   profileId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgMtuCheckh */


