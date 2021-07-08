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
* @file prvCpssPxTgfPortTxTailDropDbaPortResources.h
*
* @brief "DBA Port Resources Allocation" enhanced UT for CPSS PX Port Tx
* Tail Drop APIs
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPxTgfPortTxTailDropDbaPortResources_h__
#define __prvCpssPxTgfPortTxTailDropDbaPortResources_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>


/**
* @internal prvCpssPxTgfPortTxTailDropDbaPortConfigurationRestore function
* @endinternal
*
* @brief   Restore backuped configuration for "DBA Port Resources Allocation"
*         test.
*/
GT_VOID prvCpssPxTgfPortTxTailDropDbaPortConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropDbaPortConfigurationSet function
* @endinternal
*
* @brief   Set configuration for "DBA Port Resources Allocation" test:
*         1. Configure Packet Type entry (0) - all ingress frames with MAC DA
*         00:01:02:03:AA:01 will be classified as Packet Type 0.
*         2. Set index of Destination Port Map Table entry for Packet Type 0.
*         3. Set Traffic Class (7) of ingress frames for Packet Type (0).
*         4. Set entry of Destination Port Map Table - disable forwarding to all
*         ports exclude receive port (1).
*         5. Bind egress port (1) to Tail Drop Profile 7.
*         6. Enable DBA mode.
*         7. Set Alpha0 for Tail Drop (TC,DP) configuration for Profiles 0 and 7.
*         8. Set Alpha0 for Tail Drop per port configuration for Profile (7).
*         9. Set amount of available for DBA buffers to 1000.
*/
GT_VOID prvCpssPxTgfPortTxTailDropDbaPortConfigurationSet
(
    GT_VOID
);

/**
* @internal prvCpssPxTgfPortTxTailDropDbaPortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for "DBA Port Resources Allocation" test:
*         1. Clear MAC counters.
*         2. Go over all Alphas.
*         2.1. Set portAlpha for receive port.
*         2.2. Get expected amount of available buffers (expectedValue).
*         2.3. Block TxQ (7) for receive port (1).
*         2.4. Transmit 1000 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
*         2.5. Release TxQ (7) for receive port (1).
*         2.6. Read MAC counters. Expected that amount of sent frames will be
*         in range [ expectedValue-10 .. expectedValue+10 ].
*/
GT_VOID prvCpssPxTgfPortTxTailDropDbaPortTrafficGenerate
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxTgfPortTxTailDropDbaPortResources_h__ */

