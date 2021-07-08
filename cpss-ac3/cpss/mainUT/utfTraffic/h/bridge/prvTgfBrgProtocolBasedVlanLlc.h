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
* @file prvTgfBrgProtocolBasedVlanLlc.h
*
* @brief Protocol-Based VLAN LLC/Non-SNAP for ALL packets
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgProtocolBasedVlanLlch
#define __prvTgfBrgProtocolBasedVlanLlch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgProtocolBasedVlanLlcConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgProtocolBasedVlanLlcConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgProtocolBasedVlanLlcTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgProtocolBasedVlanLlcTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgProtocolBasedVlanLlcConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgProtocolBasedVlanLlcConfigRestore
(
    GT_VOID
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgProtocolBasedVlanLlch */


