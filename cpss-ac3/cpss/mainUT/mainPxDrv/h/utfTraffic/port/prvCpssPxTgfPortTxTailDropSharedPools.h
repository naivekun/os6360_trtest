/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file prvCpssPxTgfPortTxTailDropSharedPools.h
*
* @brief "Shared Pools" enhanced UT for CPSS PX Port Tx Tail Drop APIs
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPxTgfPortTxTailDropSharedPools_h__
#define __prvCpssPxTgfPortTxTailDropSharedPools_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>


/**
* @internal prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationRestore function
* @endinternal
*
* @brief   Restore backuped configuration for "Shared Pools" test.
*/
GT_VOID prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationSet function
* @endinternal
*
* @brief   Set configuration for "Shared Pools" test:
*         1. Go over all Traffic Classes (0..7).
*         1.1. Configure Packet Type entry (2..9) - ingress frames will be
*         classified by MAC DA.
*         Note: SRC Port Map Table entry for egress port (1) by default
*         configured to disable forwarding for frames with
*         Packet Type 1 (same Packet Type as number of port).
*         So I use Packet Type entries > 1.
*         1.2. Configure CoS format entry - set Traffic Class (0..7) of ingress
*         frames for current Packet Type (2..9).
*         1.3. Set index of Destination Port Map Table Entry for Packet Type.
*         2. Set entry of Destination Port Map Table - disable forwarding to all
*         ports except receive port (1).
*         3. Disable DBA mode.
*         4. Bind egress port (1) to Tail Drop Profile 7.
*         5. Set port limits for Tail Drop Profile 7 - 300 buffers/descriptors.
*         6. Go over all Traffic Classes (0..7).
*         6.1. Set buffers/descriptors limits per Tail Drop Profile (7) and
*         Traffic Class (0..7): 200 buffers/descriptors for DP0/DP1/DP2.
*         6.2. Bind Traffic Class (0..7) to Shared Pool (0..7).
*         6.3. Set buffers/descriptors limit per Shared Pool: 70 + (0..7).
*/
GT_VOID prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationSet
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropSharedPoolsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for "Shared Pools" test.
*         1. Go over all Traffic Class (0..7).
*         1.1. Set MAC DA for transmitted frames
*         (00:01:02:03:AA:00 .. 00:01:02:03:AA:07)
*         1.2. Clear MAC counters for receive port.
*         1.3. Block TxQ (0..7) for receive port (1).
*         1.4. Transmit 300 frames to send port (0).
*         1.5. Release TxQ (0..7) for receive port (1).
*         1.6. Read MAC counters for receive port (1).
*         Expected value for Tx counters: 270 + (0..7) frames.
*/
GT_VOID prvCpssPxTgfPortTxTailDropSharedPoolsTrafficGenerate
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxTgfPortTxTailDropSharedPools_h__ */

