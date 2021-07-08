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
* @file prvTgfPclUdb.h
*
* @brief User Defined Bytes tests
*
* @version   10
********************************************************************************
*/
#ifndef __prvTgfPclUdbh
#define __prvTgfPclUdbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPclGen.h>

/**
* @internal prvTgfPclIpv4TcpConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4TcpConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4TcpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 TCP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 TCP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4TcpTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4TcpAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4TcpAdditionalConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4TcpAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 TCP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4TcpAdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4TcpConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv4TcpConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4UdpConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4UdpConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4UdpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4UdpTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4UdpAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4UdpAdditionalConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4UdpAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4UdpAdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4UdpConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv4UdpConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4FragmentConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4FragmentConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4FragmentTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4FragmentTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4FragmentAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4FragmentAdditionalConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4FragmentAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4FragmentAdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4FragmentConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv4FragmentConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4OtherConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4OtherConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4OtherTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4OtherTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4OtherAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4OtherAdditionalConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4OtherAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4OtherAdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4OtherConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv4OtherConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclEthernetOtherConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L2_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclEthernetOtherConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclEthernetOtherTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclEthernetOtherTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclEthernetOtherAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclEthernetOtherAdditionalConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclEthernetOtherAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclEthernetOtherAdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclEthernetOtherConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclEthernetOtherConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclMplsConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_MPLS_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclMplsConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclMplsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclMplsTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclMplsAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_MPLS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclMplsAdditionalConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclMplsAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclMplsAdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclMplsConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclMplsConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclUdeConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_UDE_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L2_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclUdeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclUdeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclUdeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclUdeAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_UDE_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclUdeAdditionalConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclUdeAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclUdeAdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclUdeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclUdeConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv6ConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv6ConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv6TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv6TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv6AdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv6AdditionalConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv6AdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv6AdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv6ConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv6ConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv6TcpConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv6TcpConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv6TcpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv6TcpTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv6TcpAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv6TcpAdditionalConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv6TcpAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv6TcpAdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv6TcpConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv6TcpConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclEthernetOtherVridConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L2_E
*         - set useVrId to GT_TRUE
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclEthernetOtherVridConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclEthernetOtherVridTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclEthernetOtherVridTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclEthernetOtherVridAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set useVrId to GT_TRUE
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclEthernetOtherVridAdditionalConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclEthernetOtherVridAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclEthernetOtherVridAdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclEthernetOtherVridConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclEthernetOtherVridConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4UdpQosConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set useQosProfile to GT_TRUE
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4UdpQosConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4UdpQosTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4UdpQosTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4UdpQosAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set useQosProfile to GT_TRUE
*         - set UDB offset to 0
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4UdpQosAdditionalConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4UdpQosAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4UdpQosAdditionalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclIpv4UdpQosConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv4UdpQosConfigRestore
(
    GT_VOID
);


/**
* @internal prvTgfPclRuleStdNotIpUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdNotIpUdbConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclRuleStdNotIpUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdNotIpUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdNotIpUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdNotIpUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdNotIpUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdNotIpUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdNotIpUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleStdNotIpUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpL2QosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpL2QosUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpL2QosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpL2QosUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpL2QosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpL2QosUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpL2QosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpL2QosUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpL2QosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleStdIpL2QosUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv4L4UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv4L4UdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv4L4UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv4L4UdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv4L4UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv4L4UdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv4L4UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv4L4UdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv4L4UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleStdIpv4L4UdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleIngrStdUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleIngrStdUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleIngrStdUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleIngrStdUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleIngrStdUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleIngrStdUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleIngrStdUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleIngrStdUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleIngrStdUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleIngrStdUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtNotIpv6UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtNotIpv6UdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtNotIpv6UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtNotIpv6UdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtNotIpv6UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtNotIpv6UdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtNotIpv6UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtNotIpv6UdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtNotIpv6UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleExtNotIpv6UdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleIngrExtUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleIngrExtUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleIngrExtUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleIngrExtUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleIngrExtUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleIngrExtUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleIngrExtUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleIngrExtUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleIngrExtUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleIngrExtUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv6L2UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv6L2UdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv6L2UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv6L2UdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv6L2UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv6L2UdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv6L2UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv6L2UdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv6L2UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleExtIpv6L2UdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv6L4UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv6L4UdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv6L4UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv6L4UdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv6L4UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv6L4UdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv6L4UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv6L4UdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv6L4UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleExtIpv6L4UdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv6DipUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv6DipUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv6DipUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv6DipUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv6DipUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv6DipUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv6DipUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv6DipUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv6DipUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleStdIpv6DipUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleUltraIpv6PortVlanQosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleUltraIpv6PortVlanQosUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleUltraIpv6PortVlanQosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleUltraIpv6PortVlanQosUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleUltraIpv6PortVlanQosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleUltraIpv6PortVlanQosUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleUltraIpv6PortVlanQosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleUltraIpv6PortVlanQosUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleUltraIpv6PortVlanQosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleUltraIpv6PortVlanQosUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleUltraIpv6RoutedAclQosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleUltraIpv6RoutedAclQosUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleUltraIpv6RoutedAclQosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleUltraIpv6RoutedAclQosUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleUltraIpv6RoutedAclQosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleUltraIpv6RoutedAclQosUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleUltraIpv6RoutedAclQosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleUltraIpv6RoutedAclQosUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleUltraIpv6RoutedAclQosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleUltraIpv6RoutedAclQosUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv4RoutedAclQosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv4RoutedAclQosUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv4RoutedAclQosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv4RoutedAclQosUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv4RoutedAclQosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv4RoutedAclQosUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv4RoutedAclQosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv4RoutedAclQosUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleStdIpv4RoutedAclQosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleStdIpv4RoutedAclQosUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv4PortVlanQosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv4PortVlanQosUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv4PortVlanQosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv4PortVlanQosUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv4PortVlanQosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv4PortVlanQosUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv4PortVlanQosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv4PortVlanQosUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleExtIpv4PortVlanQosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleExtIpv4PortVlanQosUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdNotIpUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdNotIpUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdNotIpUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdNotIpUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdNotIpUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdNotIpUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdNotIpUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdNotIpUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdNotIpUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrStdNotIpUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdIpL2QosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdIpL2QosUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdIpL2QosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdIpL2QosUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdIpL2QosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdIpL2QosUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdIpL2QosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdIpL2QosUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdIpL2QosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrStdIpL2QosUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdIpv4L4UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdIpv4L4UdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdIpv4L4UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdIpv4L4UdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdIpv4L4UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdIpv4L4UdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdIpv4L4UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdIpv4L4UdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrStdIpv4L4UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrStdIpv4L4UdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtNotIpv6UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtNotIpv6UdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtNotIpv6UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtNotIpv6UdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtNotIpv6UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtNotIpv6UdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtNotIpv6UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtNotIpv6UdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtNotIpv6UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrExtNotIpv6UdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv6L4UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L4UdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv6L4UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L4UdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv6L4UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L4UdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv6L4UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L4UdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv6L4UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L4UdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv4RaclVaclUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv4RaclVaclUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv4RaclVaclUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv4RaclVaclUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv4RaclVaclUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv4RaclVaclUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv4RaclVaclUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv4RaclVaclUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrExtIpv4RaclVaclUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrExtIpv4RaclVaclUdbConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrUltraIpv6RaclVaclUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrUltraIpv6RaclVaclUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrUltraIpv6RaclVaclUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrUltraIpv6RaclVaclUdbTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrUltraIpv6RaclVaclUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrUltraIpv6RaclVaclUdbAdditionalConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrUltraIpv6RaclVaclUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrUltraIpv6RaclVaclUdbAdditionalTrafficGenerate
(
    GT_VOID
);
/**
* @internal prvTgfPclRuleEgrUltraIpv6RaclVaclUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrUltraIpv6RaclVaclUdbConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbTestInit function
* @endinternal
*
* @brief   Set initial test settings:
*         - set VLAN entry
*         - set default port VLAN ID
*         - init PCL engine
*         - configure the User Defined Bytes
*         - sets the Policy rule
* @param[in] ruleIndex                - index of the rule in the TCAM
* @param[in] ruleFormat               - format of the Rule
* @param[in] packetType               - packet Type
* @param[in] offsetType               - the type of offset
* @param[in] useVrId                  - override VR ID
* @param[in] useQosProfile            - override QoS profile
* @param[in] packetInfoPtr            - (pointer to) packet fields info
*                                       None
*/
GT_VOID prvTgfPclUdbTestInit
(
    IN GT_U32                               ruleIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT          packetType,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT          offsetType,
    IN GT_BOOL                              useVrId,
    IN GT_BOOL                              useQosProfile,
    IN TGF_PACKET_STC                      *packetInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclUdbh */


