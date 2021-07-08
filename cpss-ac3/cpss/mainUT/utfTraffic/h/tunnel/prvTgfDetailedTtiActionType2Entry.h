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
* @file prvTgfDetailedTtiActionType2Entry.h
*
* @brief Verify the functionality of TTI entry
*
* @version   4
********************************************************************************
*/
#ifndef __prvTgfDetailedTtiActionType2Entryh
#define __prvTgfDetailedTtiActionType2Entryh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunnelTermTag1VlanCommandFieldConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set rule index to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - create VLAN 5 and VLAN 6
*         - enable Ethernet TTI lookup
*         - set Ethernet TTI rule to match packets with VLAN tags 5 & 7
*         and specific MAC DA address
*         - set TTI action with the following parameters:
*         forward to egress interface
*         set Tag0 VLAN from entry with Tag0 VLAN value 6
*         set Tag1 VLAN from entry with Tag1 VLAN value 8
*         - set Egress Tag field in VLAN 6 entry to <outer Tag1, inner Tag0>
*/
GT_VOID prvTgfTunnelTermTag1VlanCommandFieldConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTag1VlanCommandFieldTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send QinQ packet with matching VLANS:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x8988 (MIM Ether type)
*         iSid: 0x123456
*         iUP: 0x5
*         iDP: 0
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
* @param[in] keyType                  - TTI key type
*                                       None
*/
GT_VOID prvTgfTunnelTermTag1VlanCommandFieldTrafficGenerate
(
    IN  PRV_TGF_TTI_KEY_TYPE_ENT    keyType
);

/**
* @internal prvTgfTunnelTermTag1VlanCommandFieldConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermTag1VlanCommandFieldConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustExpQosFieldConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set rule index to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - create VLAN 5 and VLAN 6
*         - enable Ethernet TTI lookup
*         - set MPLS TTI rule to match MPLS packet with 2 labels
*         - set TTI action to terminate the tunnel and set trust EXP field to true
*         - set EXP to QoS profile mapping entry with EXP matching the EXP of
*         the outer MPLS label
*         - set IPv4 prefix that will route the tunnel passenger packet
*/
GT_VOID prvTgfTunnelTermTrustExpQosFieldConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustExpQosFieldTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send QinQ packet with matching VLANS:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x8847 (MPLS Ether type)
*         Success Criteria:
*         Verify that the QoS fields are set according to the QoS profile
*         assigned in the EXP to QoS table
*/
GT_VOID prvTgfTunnelTermTrustExpQosFieldTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustExpQosFieldConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermTrustExpQosFieldConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTrustExpQosFieldNonTermConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set rule index to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - create VLAN 5 and VLAN 6
*         - set port default QoS profile
*         - set MPLS TTI rule to match MPLS packet with 3 labels
*         - set TTI action with pop 2 MPLS command
*         - set trust EXP field to true
*         - set redirect packet to egress interface
*         - set EXP to QoS profile entry
*/
GT_VOID prvTgfTunnelTrustExpQosFieldNonTermConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTrustExpQosFieldNonTermPop3ConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set rule index to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - create VLAN 5 and VLAN 6
*         - set port default QoS profile
*         - set MPLS TTI rule to match MPLS packet with 3 labels
*         - set TTI action with pop 3 MPLS command
*         - set trust EXP field to true
*         - set redirect packet to egress interface
*         - set EXP to QoS profile entry
*/
GT_VOID prvTgfTunnelTrustExpQosFieldNonTermPop3ConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTrustExpQosFieldNonTermTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send QinQ packet with matching VLANS:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x8847 (MPLS Ether type)
*         Success Criteria:
*         Packet is forward correctly with 1 MPLS label
*         QoS fields are set according to the QoS profile matching
*         the EXP field in the last MPLS label that was not popped
*/
GT_VOID prvTgfTunnelTrustExpQosFieldNonTermTrafficGenerate
(
    GT_VOID
);


/**
* @internal prvTgfTunnelTrustExpQosFieldNonTermPop3TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send QinQ packet with matching VLANS:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x8847 (MPLS Ether type)
*         Success Criteria:
*         Packet is forward correctly with 1 MPLS label
*         QoS fields are set according to the QoS profile matching
*         the EXP field in the last MPLS label that was not popped
*/
GT_VOID prvTgfTunnelTrustExpQosFieldNonTermPop3TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTrustExpQosFieldNonTermConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTrustExpQosFieldNonTermConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustAnyQosFieldConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set rule index to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - create VLAN 5 and VLAN 6
*         - set port default QoS profile
*         - set IPv4 TTI rule to match IPv4 tunneled packets
*         - set TTI action to terminate the tunnel, not to trust any QoS fields
*         and with default QoS profile index
*         - set IPv4 prefix that will route the tunnel passenger packet
*/
GT_VOID prvTgfTunnelTermTrustAnyQosFieldConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustAnyQosFieldTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send Ipv4-Over-Ipv4 packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x0800 (Ipv4 Ether type)
*         Success Criteria:
*         Packet is forward correctly with 1 MPLS label
*         QoS fields are set according to the QoS profile matching
*         the EXP field in the last MPLS label that was not popped
*/
GT_VOID prvTgfTunnelTermTrustAnyQosFieldTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustAnyQosFieldConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermTrustAnyQosFieldConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustDscpQosFieldConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set rule index to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - create VLAN 5 and VLAN 6
*         - set port default QoS profile
*         - set IPv4 TTI rule to match IPv4 tunneled packets
*         - set TTI action to terminate the tunnel and to trust DSCP
*         - set DSCP to QoS profile mapping entry with DSCP matching the DSCP
*         of the passenger packet
*         - set IPv4 prefix that will route the tunnel passenger packet
*/
GT_VOID prvTgfTunnelTermTrustDscpQosFieldConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustDscpQosFieldTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send Ipv4-Over-Ipv4 packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x0800 (Ipv4 Ether type)
*         Success Criteria:
*         Packet is forward correctly with 1 MPLS label
*         QoS fields are set according to the QoS profile matching
*         the EXP field in the last MPLS label that was not popped
*/
GT_VOID prvTgfTunnelTermTrustDscpQosFieldTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustDscpQosFieldConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermTrustDscpQosFieldConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustUpQosFieldConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set rule index to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - create VLAN 5 and VLAN 6
*         - set port default QoS profile
*         - set IPv4 TTI rule to match IPv4 tunneled packets
*         - set TTI action to terminate the tunnel and to trust DSCP
*         - set DSCP to QoS profile mapping entry with DSCP matching the DSCP
*         of the passenger packet
*         - set IPv4 prefix that will route the tunnel passenger packet
*/
GT_VOID prvTgfTunnelTermTrustUpQosFieldConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustUpQosFieldTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send Ipv4-Over-Ipv4 packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x0800 (Ipv4 Ether type)
*         Success Criteria:
*         Packet is forward correctly with 1 MPLS label
*         QoS fields are set according to the QoS profile matching
*         the EXP field in the last MPLS label that was not popped
*/
GT_VOID prvTgfTunnelTermTrustUpQosFieldTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrustUpQosFieldConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermTrustUpQosFieldConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTag1VlanCommandUdbConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfTunnelTermTag1VlanCommandUdbConfigurationSet(GT_VOID);


/**
* @internal prvTgfTunnelTermTag1VlanCommandUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermTag1VlanCommandUdbConfigRestore(GT_VOID);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfDetailedTtiActionType2Entryh */

