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
* @file prvTgfExactMatchUdbPcl.h
*
* @brief Test Exact Match functionality with PCL Action Type and UDB key
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchUdbPclh
#define __prvTgfExactMatchUdbPclh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>

/**
* @internal prvTgfExactMatchPclUdbMaskPatternBuild function
* @endinternal
*
* @brief   Build UDB : Pattern and Mask according to packet sent
*
* @param[in] udbAmount      - UDB Amount
* @param[in] packetPtr      - (pointer to) Info of the packet used in the test
* @param[in] offsetInPacket - offset of matched data in the packet
* @param[out]udbMaskPtr     - (pointer to) mask
* @param[out]udbPatternPtr  - (pointer to) pattern
*/
GT_VOID prvTgfExactMatchPclUdbMaskPatternBuild
(
    IN  GT_U32                 udbAmount,
    IN  TGF_PACKET_STC         *packetPtr,
    IN  GT_U32                 offsetInPacket,
    OUT GT_U8                  *udbMaskPtr,
    OUT GT_U8                  *udbPatternPtr
);
/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyGenericTest function
* @endinternal
*
* @brief   Generic Test on UDB Only Key
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
* @param[in] packetType               - packet type
* @param[in] packetTypeIndex          - 0 - not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] ruleSize                 - rule Size
* @param[in] offsetType               - offset Type
* @param[in] udbAmount                - UDB Amount
* @param[in] udbIndexLow              - The lowest UDB Index in the UDB range
* @param[in] udbOffsetLow             - UDB Offset Base of the field (the lowest byte offset)
* @param[in] udbOrderAscend           - UDB Order Ascend (GT_TRUE) or Descend (GT_FALSE)
* @param[in] packetPtr                - (pointer to) Info of the packet used in the test
* @param[in] offsetInPacket           - offset of matched data in the packet
*                                       None
*/
GT_VOID prvTgfExactMatchPclUdbOnlyKeysGenericConfig
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           packetTypeIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbIndexLow,
    IN GT_U32                           udbOffsetLow,
    IN GT_BOOL                          udbOrderAscend,
    IN TGF_PACKET_STC                   *packetPtr,
    IN GT_U32                           offsetInPacket
);
/**
* @internal prvTgfExactMatchUdb40EthOthreL2PclGenericConfig function
* @endinternal
*
* @brief   Set PCL generic test configuration, not related to Exact Match
*          Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet
*
* @note 40 first bytes from L2 header beginning.
*/
GT_VOID prvTgfExactMatchUdb40EthOthreL2PclGenericConfig
(
    GT_VOID
);
/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyGenericTest function
* @endinternal
*
* @brief   Generic Test on UDB Only Key
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
* @param[in] packetType               - packet type
* @param[in] ruleFormat               - rule Format
* @param[in] ruleSize                 - rule Size
* @param[in] offsetType               - offset Type
* @param[in] udbAmount                - UDB Amount
* @param[in] udbIndexLow              - The lowest UDB Index in the UDB range
*/
GT_VOID prvTgfExactMatchPclUdbOnlyKeysGenericRestore
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN CPSS_PCL_RULE_SIZE_ENT           ruleSize,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbIndexLow
);
/**
* @internal prvTgfExactMatchUdb40EthOthreL2PclGenericRestore function
* @endinternal
*
* @brief   Restore PCL generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchUdb40EthOthreL2PclGenericRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic, expect traffic to be trapped or dropped
* @param[in] expectNoTraffic   - GT_TRUE:  No traffic
*                                GT_FALSE: Expect Traffic
* @param[in] expectFdbForwarding - GT_TRUE:  FBB forwarding
*                                GT_FALSE: NO FBB forwarding
*/
GT_VOID prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate
(
    GT_BOOL     expectNoTraffic,
    GT_BOOL     expectFdbForwarding
);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
