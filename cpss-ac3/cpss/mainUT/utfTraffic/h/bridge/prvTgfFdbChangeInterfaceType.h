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
* @file prvTgfFdbChangeInterfaceType.h
*
* @brief Change Interface Type (port, trunk, vid, vidx)
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbChangeInterfaceTypeh
#define __prvTgfFdbChangeInterfaceTypeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfFdbChangeInterfaceTypeConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbChangeInterfaceTypeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbChangeInterfaceTypeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbChangeInterfaceTypeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbChangeInterfaceTypeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbChangeInterfaceTypeConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbChangeInterfaceTypeh */


