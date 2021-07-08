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
* @file prvCpssPxTgfPortTxTailDropOnePort.h
*
* @brief "One Port" enhanced UT for CPSS PX Port Tx Tail Drop APIs
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPxTgfPortTxTailDropOnePort_h__
#define __prvCpssPxTgfPortTxTailDropOnePort_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>


/**
* @internal prvCpssPxTgfPortTxTailDropOnePortConfigurationRestore function
* @endinternal
*
* @brief   Restore backuped configuration for "One Port" test.
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortConfigurationSet function
* @endinternal
*
* @brief   Set configuration for "One Port" test.
*         1. Configure Packet Type entry (0) - all ingress frames with MAC DA
*         00:01:02:03:AA:01 will be classified as Packet Type 0.
*         2. Set index of Destination Port Map Table entry for Packet Type 0.
*         3. Set entry of Destination Port Map Table - disable forwarding to all
*         ports except receive port (1).
*         4. Bind egress port (1) to Tail Drop Profile 7.
*         5. Disable DBA mode.
*         6. Set buffers/descriptors limits per Tail Drop Profile (7) and
*         Traffic Class (7):
*         - for Drop Precedence 0 (green) - 1000 buffers/descriptors;
*         - for Drop Precedence 1 (yellow) - 500 buffers/descriptors;
*         - for Drop Precedence 2 (red)  - 200 buffers/descriptors;
*         7. Set port limits for Tail Drop Profile 7 - 1700 buffers/descriptors.
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortConfigurationSet
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropOnePortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for "One port" test.
*         1. Clear MAC counters.
*         2. Block TxQ (7) for receive port (1).
*         3. Go over all Drop Precedence (green/yellow/red).
*         3.1. Set Drop Precedence and Traffic Class (7) of ingress frames for
*         Packet Type (0).
*         3.2. Transmit 1100 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
*         3.3. Check count of global allocated buffers and descriptors - expected
*         values 1000 / 1000+500 / 1000+500+200.
*         4. Release TxQ (7) for receive port (1).
*         5. Check MAC counters - expected Tx counter on port 1 is 1700 frames.
*/
GT_VOID prvCpssPxTgfPortTxTailDropOnePortTrafficGenerate
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxTgfPortTxTailDropOnePort_h__ */

