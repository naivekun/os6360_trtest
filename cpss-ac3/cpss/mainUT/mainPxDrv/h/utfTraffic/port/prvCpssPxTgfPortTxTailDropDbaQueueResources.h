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
* @file prvCpssPxTgfPortTxTailDropDbaQueueResources.h
*
* @brief "DBA Queue Resources Allocation" enhanced UT for CPSS PX Port Tx
* Tail Drop APIs
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPxTgfPortTxTailDropDbaQueueResources_h__
#define __prvCpssPxTgfPortTxTailDropDbaQueueResources_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>


/**
* @internal prvCpssPxTgfPortTxTailDropDbaQueueConfigurationRestore function
* @endinternal
*
* @brief   Restore backuped configuration for "DBA Queue Resources Allocation"
*         test.
*/
GT_VOID prvCpssPxTgfPortTxTailDropDbaQueueConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropDbaQueueConfigurationSet function
* @endinternal
*
* @brief   Set configuration for "DBA Queue Resources Allocation" test:
*         1. Configure Packet Type entry (0) - all ingress frames with MAC DA
*         00:01:02:03:AA:01 will be classified as Packet Type 0.
*         2. Set index of Destination Port Map Table entry for Packet Type 0.
*         3. Set entry of Destination Port Map Table - disable forwarding to all
*         ports exclude receive port (1).
*         4. Bind egress port (1) to Tail Drop Profile 7.
*         5. Enable DBA mode.
*         6. Set configuration for Tail Drop Profile (7) and Traffic Class (7):
*         - for Drop Precedence 0 (green) - 300 buffers; Alpha 4;
*         - for Drop Precedence 1 (yellow) - 200 buffers; Alpha 2;
*         - for Drop Precedence 2 (red)  - 100 buffers; Alpha 0.5;
*         7. Set Alphas for default Tail Drop Profile (0) (all TCs) and all TCs
*         exclude TC 7 for Tail Drop Profile 7 to Alpha 0.
*         8. Set amount of available for DBA buffers to 1000.
*/
GT_VOID prvCpssPxTgfPortTxTailDropDbaQueueConfigurationSet
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropDbaQueueTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for "DBA Queue Resources Allocation" test:
*         1. Clear MAC counters.
*         2. Go over all Drop Precedences (green/yellow/red).
*         2.1. Get expected amount of available buffers (expectedValue).
*         2.2. Set Drop Precedence and Traffic Class (7) of ingress frames for
*         Packet Type (0).
*         2.3. Block TxQ (7) for receive port (1).
*         2.4. Transmit 1000 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
*         2.5. Release TxQ (7) for receive port (1).
*         2.6. Read MAC counters. Expected that amount of sent frames will be
*         in range [ expectedValue-10 .. expectedValue+10 ].
*/
GT_VOID prvCpssPxTgfPortTxTailDropDbaQueueTrafficGenerate
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxTgfPortTxTailDropDbaQueueResources_h__ */

