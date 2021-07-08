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
* @file prvTgfCncEgrQue.h
*
* @brief Centralized Counters (Cnc)
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfCncEgrQue
#define __prvTgfCncEgrQue

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCncTestEgrQueConfigure function
* @endinternal
*
* @brief   This function configures Cnc EgrQue tests
*/
GT_VOID prvTgfCncTestEgrQueConfigure
(
    GT_VOID
);

/**
* @internal prvTgfCncTestEgrQueTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncTestEgrQueTrafficGenerateAndCheck
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCncEgrQue */


