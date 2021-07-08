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
* @file prvTgfEgressInterface.h
*
* @brief L2 MLL Egress Interface test definitions
*
* @version   4
********************************************************************************
*/
#ifndef __prvTgfEgressInterfaceh
#define __prvTgfEgressInterfaceh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfEgressInterfaceConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfEgressInterfaceConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfEgressInterfaceEntrySelectorTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic with different configuration of entrySelector field in
*         the "L2 MLL LTT Entry" and "L2 MLL Entry"
*/
GT_VOID prvTgfEgressInterfaceEntrySelectorTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfEgressInterfaceTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfEgressInterfaceTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfEgressInterfaceConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfEgressInterfaceConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfEgressInterfaceh */


