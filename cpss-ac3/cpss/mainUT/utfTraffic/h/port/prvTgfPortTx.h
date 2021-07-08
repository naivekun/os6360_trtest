/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfPortTxResourceHistogram.h
*
* DESCRIPTION:
*       Port Tx Queue features testing
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfPortTxh
#define __prvTgfPortTxh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPortTxSpSchedulerConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfPortTxSpSchedulerConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPortTxSpSchedulerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfPortTxSpSchedulerTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPortTxSpSchedulerRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPortTxSpSchedulerRestore
(
    GT_VOID
);

/**
* @internal prvTgfPortTxMapPhy2TxQCheck function
* @endinternal
*
* @brief   Map physical and TxQ ports and test results.
*/
GT_VOID prvTgfPortTxMapPhy2TxQCheck
(
    IN  GT_VOID
);

/**
* @internal prvPortTxFwsTcTailDropProfileWrRd function
* @endinternal
*
* @brief   test tail drop tables under FWS
*/
GT_VOID prvPortTxFwsTcTailDropProfileWrRd
(
    IN  GT_VOID
);

/**
* @internal prvPortTxFullBandwidthTcTailDropProfileWrRd function
* @endinternal
*
* @brief   test tail drop tables under full bandwidth traffic
*/
GT_VOID prvPortTxFullBandwidthTcTailDropProfileWrRd
(
    IN  GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortTxh */



