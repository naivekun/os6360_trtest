/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfNetIfRxSdmaPerformance.h
*
* DESCRIPTION:
*       Rx SDMA performance testing testing header file.
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*
*******************************************************************************/
#ifndef __prvTgfNetIfRxSdmaPerformanceh
#define __prvTgfNetIfRxSdmaPerformanceh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfNetIfRxSdmaPerformanceTestInit function
* @endinternal
*
* @brief   Various test initializations
*/
GT_VOID prvTgfNetIfRxSdmaPerformanceTestInit
(
    GT_VOID
);

/**
* @internal prvTgfNetIfRxToTxSdmaPerformanceTestInit function
* @endinternal
*
* @brief   Various test initializations
*/
GT_VOID prvTgfNetIfRxToTxSdmaPerformanceTestInit
(
    GT_VOID
);

/**
* @internal prvTgfNetIfRxSdmaPerformanceTestRestore function
* @endinternal
*
* @brief   Restore test configurations
*/
GT_VOID prvTgfNetIfRxSdmaPerformanceTestRestore
(
    GT_VOID
);

/**
* @internal prvTgfNetIfRxToTxSdmaPerformanceTestRestore function
* @endinternal
*
* @brief   Restore test configurations for Rx To Tx test
*/
GT_VOID prvTgfNetIfRxToTxSdmaPerformanceTestRestore
(
    GT_VOID
);

/**
* @internal prvTgfNetIfRxSdmaPerformanceAllSizeTest function
* @endinternal
*
* @brief   Rx SDMA performance test for predefined list of packet sizes
*         or range of packet sizes.
* @param[in] firstPacketSize          - first packet size, 0 - use predefined values below
* @param[in] lastPacketSize           - last packet size, used when firstPacketSize > 0
* @param[in] step                     -  of packet size values
* @param[in] quickCheck               - GT_FALSE - use full check of rate during 1 second
*                                      GT_TRUE  - use quick check. Use minimal time for rate calculation.
*                                      test runs quick but results are not accurate
* @param[in] rxToTxEnable             - GT_FALSE - do not perform calculations for Rx to Tx mode
*                                      GT_FALSE - perform calculations for Rx to Tx mode
*                                       None
*/
GT_VOID prvTgfNetIfRxSdmaPerformanceAllSizeTest
(
    GT_U32  firstPacketSize,
    GT_U32  lastPacketSize,
    GT_U32  step,
    GT_BOOL quickCheck,
    GT_BOOL rxToTxEnable
);

/**
* @internal prvTgfNetIfTxSdmaPerformanceListTest function
* @endinternal
*
* @brief   Tx SDMA performance test for list of packet sizes.
*/
GT_VOID prvTgfNetIfTxSdmaPerformanceListTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfNetIfRxSdmaPerformanceh */



