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
* @file prvCpssPxTgfPortTxTailDropVariousPorts.h
*
* @brief "Various Ports" enhanced UT for CPSS PX Port Tx Tail Drop APIs
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPxTgfPortTxTailDropVariousPorts_h__
#define __prvCpssPxTgfPortTxTailDropVariousPorts_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>


/**
* @internal prvCpssPxTgfPortTxTailDropVariousPortsConfigurationRestore function
* @endinternal
*
* @brief   Restore backuped configuration for "Various Ports" test.
*/
GT_VOID prvCpssPxTgfPortTxTailDropVariousPortsConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropVariousPortsConfigurationSet function
* @endinternal
*
* @brief   Set configuration for "Various Ports" test.
*         1. Configure Packet Type entry 0 - all ingress frames with MAC DA
*         00:01:02:03:AA:01 will be classified as Packet Type 0.
*         2. Set index of Destination Port Map Table entry for Packet Type 0.
*         3. Bind egress ports 1,2,3 to Tail Drop Profile 7.
*         4. Disable DBA mode.
*         5. Set buffers/descriptors limits per Tail Drop Profile 7 and
*         Traffic Class 7:
*         - for Drop Precedence 0 (green) - 1000 buffers/descriptors;
*         - for Drop Precedence 1 (yellow) - 500 buffers/descriptors;
*         - for Drop Precedence 2 (red)  - 200 buffers/descriptors.
*         6. Set port limits for Tail Drop Profile 7 - 1700 buffers/descriptors.
*/
GT_VOID prvCpssPxTgfPortTxTailDropVariousPortsConfigurationSet
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropVariousPortsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for "Various ports" test.
*         1. Go over all Drop Precedence (green/yellow/red).
*         1.1. Clear MAC counters.
*         1.2. Set Drop Precedence and Traffic Class (7) of ingress frames for
*         Packet Type (0).
*         1.3. Set entry of Destination Port Map Table - disable forwarding to
*         all ports except receive port (1/2/3)
*         1.4. Block TxQ 7 for receive port (1/2/3).
*         1.5. Transmit 1500 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
*         1.6. Release TxQ 7 for receive port (1/2/3).
*         1.7. Check MAC counters - Tx counter on receive port 1/2/3 should be
*         1000/500/200 frames respectively.
*/
GT_VOID prvCpssPxTgfPortTxTailDropVariousPortsTrafficGenerate
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxTgfPortTxTailDropVariousPorts_h__ */

