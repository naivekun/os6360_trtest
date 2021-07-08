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
* @file prvTgfDropMode.h
*
* @brief Drop Mode tests for IPFIX declarations
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfDropModeh
#define __prvTgfDropModeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfIpfixDropModeSimpleTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixDropModeSimpleTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixDropModeSimpleTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixDropModeSimpleTestTrafficGenerate
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfDropModeh */

