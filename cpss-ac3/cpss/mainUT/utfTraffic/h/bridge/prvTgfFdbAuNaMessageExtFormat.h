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
* @file prvTgfFdbAuNaMessageExtFormat.h
*
* @brief Bridge FDB AU NA Message with new format UT.
*
* @version   2.0
********************************************************************************
*/
#ifndef __prvTgfFdbAuNaMessageExtFormat
#define __prvTgfFdbAuNaMessageExtFormat

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRV_TGF_FDB_NA_MSG_EXT_FMT_MAC_DA_CNS    {0x00, 0x00, 0x00, 0x00, 0x11, 0xAA}
#define PRV_TGF_FDB_NA_MSG_EXT_FMT_MAC_SA_CNS    {0x00, 0x00, 0x00, 0x00, 0x11, 0xBB}

/**
* @internal prvTgfFdbAuNaMessageExtFormatConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbAuNaMessageExtFormatWithFdbBankIndex function
* @endinternal
*
* @brief   Disable learning.
*         Send tagged packet with unknown unicast.
*         Generate traffic:
*         Send tagged packet to be learned on diferent FDB banks:
*         Success Criteria:
*         The AU NA message is updated correctly including the extended
*         parameters: entryOffset (FDB bank index).
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatWithFdbBankIndex
(
    void
);

/**
* @internal prvTgfFdbAuNaMessageExtFormatWithTag1DueToNewAddrTrafficGenerate function
* @endinternal
*
* @brief   Disable learning.
*         Send double tagged packet with unknown unicast.
*         Generate traffic:
*         Send from port1 double tagged packet:
*         Success Criteria:
*         The AU NA message is updated correctly including the extended
*         parameters: vid1, up0 and isMoved.
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatWithTag1DueToNewAddrTrafficGenerate
(
    void
);

/**
* @internal prvTgfFdbAuNaMessageExtFormatDueToMovedPortTrafficGenerate function
* @endinternal
*
* @brief   Learn MAC on FDB on port2.
*         Disable learning.
*         Send single tagged packet with known unicast, but from different
*         source (port).
*         Generate traffic:
*         Send from port1 single tagged packet:
*         Success Criteria:
*         The AU NA message is updated correctly including the extended
*         parameters: up0, isMoved oldSrcId, oldDstInterface and
*         oldAssociatedDevNum.
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatDueToMovedPortTrafficGenerate
(
    void
);

/**
* @internal prvTgfFdbAuNaMessageExtFormatDueToMovedTrunkTrafficGenerate function
* @endinternal
*
* @brief   Learn MAC on FDB on trunk.
*         Disable learning.
*         Send single tagged packet with known unicast, but from different
*         source (port).
*         Generate traffic:
*         Send from port1 single tagged packet:
*         Success Criteria:
*         The AU NA message is updated correctly including the extended
*         parameters: up0, isMoved oldSrcId, oldDstInterface and
*         oldAssociatedDevNum.
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatDueToMovedTrunkTrafficGenerate
(
    void
);

/**
* @internal prvTgfFdbAuNaMessageExtFormatConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatConfigurationRestore
(
    void
);

/**
* @internal prvTgfFdbAuStuckWaBasicAuqTest function
* @endinternal
*
* @brief   Execute basic test running Address Update (AU) queue workaround
*         for single and double AU queue
*/
GT_VOID prvTgfFdbAuStuckWaBasicAuqTest
(
    GT_VOID
);

/**
* @internal prvTgfFdbAuStuckWaDoubleQueueTest function
* @endinternal
*
* @brief   Execute comprehensive test running Address Update (AU) queue workaround
*         for double AU queue
*/
GT_VOID prvTgfFdbAuStuckWaDoubleQueueTest
(
    GT_VOID
);
/**
* @internal prvTgfFdbAuNaMessageExtFormatPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] isPktDoubleTagged        - GT_TRUE: packet is double tagged
*                                      GT_FALSE: packet is single tagged
* @param[in] portNum                  - port number to send packet from
*                                       None
*/
GT_VOID prvTgfFdbAuNaMessageExtFormatPacketSend
(
    IN GT_BOOL  isPktDoubleTagged,
    IN GT_U32    portNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbAuNaMessageExtFormat */



