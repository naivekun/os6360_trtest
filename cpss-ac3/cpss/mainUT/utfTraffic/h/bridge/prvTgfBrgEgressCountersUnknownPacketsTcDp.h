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
* @file prvTgfBrgEgressCountersUnknownPacketsTcDp.h
*
* @brief Verify that unknown packets are received
* in <outMcFrames> field according the
* configured Tc/ DP
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgEgressCountersUnknownPacketsTcDph
#define __prvTgfBrgEgressCountersUnknownPacketsTcDph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgEgressCountersUnknownPacketsTcDpConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgEgressCountersUnknownPacketsTcDpConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgEgressCountersUnknownPacketsTcDpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgEgressCountersUnknownPacketsTcDpTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgEgressCountersUnknownPacketsTcDpConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgEgressCountersUnknownPacketsTcDpConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgEgressCountersUnknownPacketsTcDph */


