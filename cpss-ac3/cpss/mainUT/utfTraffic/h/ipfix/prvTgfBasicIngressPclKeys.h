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
* @file prvTgfBasicIngressPclKeys.h
*
* @brief Ingress PCL infrastructure for IPFIX
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBasicIngressPclKeysh
#define __prvTgfBasicIngressPclKeysh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfIpfixGen.h>

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS 0x8100

/* layer 1 overhead */
#define TGF_L1_OVERHEAD_CNS 20

/**
* @internal prvTgfIpfixVlanTestInit function
* @endinternal
*
* @brief   Initialize local vlan default settings
*         INPUTS:
*/
GT_VOID prvTgfIpfixVlanTestInit
(
    IN GT_U16           vlanId,
    CPSS_PORTS_BMP_STC  portsMembers
);

/**
* @internal prvTgfIpfixVlanRestore function
* @endinternal
*
* @brief   Restore\Cancel prvTgfIpfixVlanTestInit settings
*
* @param[in] vlanId                   - vlan id.
*                                      portMembers - bit map of ports belonging to the vlan.
*                                       None
*/
GT_VOID prvTgfIpfixVlanRestore
(
    IN GT_U16           vlanId,
    CPSS_PORTS_BMP_STC  portsMembers
);

/**
* @internal prvTgfIpfixTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number to send traffic from
* @param[in] packetInfoPtr            - PACKET to send
* @param[in] burstCount               - number of packets to send
*                                       None
*/
GT_VOID prvTgfIpfixTestPacketSend
(
    IN GT_U32           portNum,
    IN TGF_PACKET_STC *packetInfoPtr,
    IN GT_U32          burstCount
);

/**
* @internal prvTgfIpfixHelloTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixHelloTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixHelloTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 0
*         Success Criteria:
*         Packet is captured on ports 0,18,23.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 0x1234
*         Success Criteria:
*         Packet is captured on ports 0,18,23 and IPFIX entry is updated.
*/
GT_VOID prvTgfIpfixHelloTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixHelloTestRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIpfixHelloTestRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIngressPclKeyStdL2h */

