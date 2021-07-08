/*******************************************************************************
*              (c), Copyright 2016, Marvell International Ltd.                 *
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
* @file prvTgfNetIfTxSdmaEvents.h
*
* @brief Tx SDMA event testing header file.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfNetIfTxSdmaEventsh
#define __prvTgfNetIfTxSdmaEventsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfNetIfTxSdmaEventsInit function
* @endinternal
*
* @brief   Initialize test:
*         - Save old event handler
*         - Set new event handler
*         - Save enabled state of tx queue
*         - Enable tx queue
*         - Save masked state of events
*         - Unmask events
*/
GT_VOID prvTgfNetIfTxSdmaEventsInit
(
    GT_VOID
);

/**
* @internal prvTgfNetIfTxSdmaEventsGenerate function
* @endinternal
*
* @brief   - Generate traffic
*         - Catch events
*/
GT_VOID prvTgfNetIfTxSdmaEventsGenerate
(
    GT_VOID
);

/**
* @internal prvTgfNetIfTxSdmaEventsRestore function
* @endinternal
*
* @brief   Restore configuration:
*         - Rextore masked state of events
*         - Restore enabled state of tx queue
*         - Restore default handler
*/
GT_VOID prvTgfNetIfTxSdmaEventsRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfNetIfTxSdmaEventsh */


