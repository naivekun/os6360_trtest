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
* @file prvTgfBrgVlanForceNewDsa.h
*
* @brief New DSA tag assignment
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgVlanForceNewDsa
#define __prvTgfBrgVlanForceNewDsa

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfCommon.h>

/**
* @internal prvTgfBrgVlanForceNewDsaToCpuConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfBrgVlanForceNewDsaToCpuConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanForceNewDsaToCpuTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgVlanForceNewDsaToCpuTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanForceNewDsaToCpuConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanForceNewDsaToCpuConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgVlanForceNewDsa */

