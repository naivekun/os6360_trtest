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
* @file prvTgfMcGroupRxMirror.h
*
* @brief Create MC groups
* Define Mirror and Analyzer ports.
* Send Packets to MC Group.
* Check that Analyzer port receives the packets too.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfMcGroupRxMirrorh
#define __prvTgfMcGroupRxMirrorh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfMcGroupRxMirrorConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfMcGroupRxMirrorConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfMcGroupRxMirrorTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfMcGroupRxMirrorTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfMcGroupRxMirrorConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfMcGroupRxMirrorConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfMcGroupRxMirrorh */


