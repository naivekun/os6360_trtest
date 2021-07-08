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
* @file tgfPortFlowControlPacketsCounter.h
*
* @brief Flow control packets counter check
*
* @version   1
********************************************************************************
*/
#ifndef __tgfPortFlowControlPacketsCounterh
#define __tgfPortFlowControlPacketsCounterh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfPortFCTrafficGenerate function
* @endinternal
*
*/
GT_VOID prvTgfPortFCTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPortFCCounterCheck function
* @endinternal
*
*/
GT_VOID prvTgfPortFCCounterCheck
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfPortFlowControlPacketsCounterh */



