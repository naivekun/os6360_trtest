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
* @file prvTgfOamGenericKeepalive.h
*
* @brief OAM Keepalive features testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfOamGenericKeepaliveh
#define __prvTgfOamGenericKeepaliveh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <oam/prvTgfOamGeneral.h>

/**
* @internal tgfOamKeepAliveAgeTest function
* @endinternal
*
* @brief   Keepalive age state verification
*/
GT_VOID tgfOamKeepAliveAgeTest
(
    GT_VOID
);

/**
* @internal tgfOamKeepAliveExcessDetectionTest function
* @endinternal
*
* @brief   Keepalive excess detection verification
*/
GT_VOID tgfOamKeepAliveExcessDetectionTest
(
    GT_VOID
);

/**
* @internal tgfOamKeepAliveRdiBitCheckingTest function
* @endinternal
*
* @brief   Keepalive RDI bit verification
*/
GT_VOID tgfOamKeepAliveRdiBitCheckingTest
(
    GT_VOID
);

/**
* @internal tgfOamKeepAlivePeriodFieldCheckingTest function
* @endinternal
*
* @brief   Keepalive period field verification
*/
GT_VOID tgfOamKeepAlivePeriodFieldCheckingTest
(
    GT_VOID
);

/**
* @internal tgfOamKeepAliveFlowHashCheckingTest function
* @endinternal
*
* @brief   2.4.5  Keepalive Flow Hash Checking
*/
GT_VOID tgfOamKeepAliveFlowHashCheckingTest
(
    GT_VOID
);

/**
* @internal tgfIOamKeepAliveFlowHashVerificationBitSelectionTest function
* @endinternal
*
* @brief   Test Ingress OAM hash bit selection
*/
GT_VOID tgfIOamKeepAliveFlowHashVerificationBitSelectionTest
(
    GT_VOID
);

/**
* @internal tgfEOamKeepAliveFlowHashVerificationBitSelectionConfigSet function
* @endinternal
*
* @brief   Config test for Egress OAM hash bit selection
*/
GT_VOID tgfEOamKeepAliveFlowHashVerificationBitSelectionConfigSet
(
    GT_VOID
);

/**
* @internal tgfEOamKeepAliveFlowHashVerificationBitSelectionTest function
* @endinternal
*
* @brief   Test Egress OAM hash bit selection
*/
GT_VOID tgfEOamKeepAliveFlowHashVerificationBitSelectionTest
(
    GT_VOID
);

/**
* @internal tgfEOamKeepAliveFlowHashVerificationBitSelectionRestore function
* @endinternal
*
* @brief   Restore configuration of test for Egress OAM hash bit selection
*/
GT_VOID tgfEOamKeepAliveFlowHashVerificationBitSelectionRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfOamGenericKeepaliveh */



