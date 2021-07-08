/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBrgGenVid1InLearnFDBCheck.h
*
* DESCRIPTION:
*       Check correctness of Vid1 field filling in the learned FDB entry
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfBrgGenVid1InLearnFDBCheckh
#define __prvTgfBrgGenVid1InLearnFDBCheckh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfBrgGenVidInLearnFDBCheckTrafficGeneratorAf function
* @endinternal
*
* @brief   Generate traffic and compare counters
*/
GT_VOID prvTgfBrgGenVidInLearnFDBCheckTrafficGeneratorAf
(
    GT_VOID
);

/**
* @internal prvTgfBrgGenVidInLearnFDBCheckRestoreAf function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfBrgGenVidInLearnFDBCheckRestoreAf
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prvTgfBrgGenVid1InLearnFDBCheck */


