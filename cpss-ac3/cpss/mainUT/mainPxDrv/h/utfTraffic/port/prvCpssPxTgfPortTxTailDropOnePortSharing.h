/******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
*******************************************************************************
*/
/**
********************************************************************************
* @file prvCpssPxTgfPortTxTailDropOnePortSharing.h
*
* @brief "One Port Sharing" enhanced UT for CPSS PX Port Tx Tail Drop APIs
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPxTgfPortTxTailDropOnePortSharing_h__
#define __prvCpssPxTgfPortTxTailDropOnePortSharing_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>


/**
* @internal prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationRestore function
* @endinternal
*
* @brief   Restore backuped configuration for "One Port Sharing" test.
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationSet function
* @endinternal
*
* @brief   Set configuration for "One Port Sharing" test:
*         1. Configure Packet Type entry (0) - all ingress frames with MAC DA
*         00:01:02:03:AA:01 will be classified as Packet Type 0.
*         2. Set index of Destination Port Map Table entry for Packet Type 0.
*         3. Set entry of Destination Port Map Table - disable forwarding to all
*         ports except receive port (1).
*         4. Bind egress port (1) to Tail Drop Profile 7.
*         5. Disable DBA mode.
*         6. Set buffers/descriptors limits per Tail Drop Profile (7) and
*         Traffic Class (7):
*         - for Drop Precedence 0 (green) - 300 buffers/descriptors;
*         - for Drop Precedence 1 (yellow) - 200 buffers/descriptors;
*         - for Drop Precedence 2 (red)  - 100 buffers/descriptors;
*         7. Set port limits for Tail Drop Profile 7 - 400 buffers/descriptors.
*         8. Set limits for Shared Pool 2 - 70 buffers/descriptors.
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationSet
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortSharingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for "One Port Sharing" test.
*         1. Go over all Shared DP modes (disable/dp0/dp0-dp1/all).
*         1.1. Set Shared DP mode and Pool Number (2) for test Tail Drop Profile
*         and Traffic Class.
*         1.2. Clear MAC counter for receive port (1).
*         1.3. Go over all Drop Precedence.
*         1.3.1. Block TxQ (7) for receive port (1).
*         1.3.2. Set Drop Precedence (green/yellow/red) and Traffic Class (7) of
*         ingress frames for Packet Type (0).
*         1.3.3. Transmit 400 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
*         1.3.4. Check global buffers/descriptors allocation.
*         1.3.5. Release TxQ (7) for receive port (1).
*         1.4. Check MAC counter on receive port (1).
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortSharingTrafficGenerate
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxTgfPortTxTailDropOnePortSharing_h__ */

