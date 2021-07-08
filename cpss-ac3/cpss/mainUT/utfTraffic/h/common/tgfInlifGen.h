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
* @file tgfInlifGen.h
*
* @brief Generic API for Inlif
*
* @version   5
********************************************************************************
*/
#ifndef CHX_FAMILY
    /* we not want those includes !! */
    #define __tgfInlifGenh
#endif /*CHX_FAMILY*/

#ifndef __tgfInlifGenh
#define __tgfInlifGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssCommonDefs.h>

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_INLIF_TYPE_ENT
 *
 * @brief Inlif Type enumeration
*/
typedef enum{

    /** Support Port Inlif fields */
    PRV_TGF_INLIF_TYPE_PORT_E,

    /** Support VLAN Inlif fields */
    PRV_TGF_INLIF_TYPE_VLAN_E,

    /** Support TTI Inlif fields */
    PRV_TGF_INLIF_TYPE_EXTERNAL_E

} PRV_TGF_INLIF_TYPE_ENT;

/**
* @struct PRV_TGF_INLIF_ENTRY_STC
 *
 * @brief Struct for InLIF Entry Fields
*/
typedef struct{

    /** Enable or disable the L2 bridge */
    GT_BOOL bridgeEnable;

    /** Enable or disable auto */
    GT_BOOL autoLearnEnable;

    /** Enable\disable Sending New Source MAC address message to the CPU */
    GT_BOOL naMessageToCpuEnable;

    /** Enable or disable new source address storm prevention */
    GT_BOOL naStormPreventionEnable;

    /** @brief the command applied to Unknown MAC source address unicast packets
     *  unkSaUcCommand - the command applied to Unknown MAC destination address unicast packets
     */
    CPSS_PACKET_CMD_ENT unkSaUcCommand;

    CPSS_PACKET_CMD_ENT unkDaUcCommand;

    /** When set, new source address is not considered as a security breach */
    GT_BOOL unkSaNotSecurBreachEnable;

    /** Determines the Maximum Received Units (MRU) of packets on the interface */
    GT_U32 untaggedMruIndex;

    /** The command of unregistered non IP Multicast packets */
    CPSS_PACKET_CMD_ENT unregNonIpMcCommand;

    /** The command of unregistered IPv4 Multicast packets */
    CPSS_PACKET_CMD_ENT unregIpMcCommand;

    /** The command of unregistered IPv4 Broadcast packets */
    CPSS_PACKET_CMD_ENT unregIpv4BcCommand;

    /** The command of unregistered non */
    CPSS_PACKET_CMD_ENT unregNonIpv4BcCommand;

    /** GT_TRUE = enable IPv4 Unicast Routing on the Interface */
    GT_BOOL ipv4UcRouteEnable;

    /** GT_TRUE = enable IPv4 Multicast Routing on the interface */
    GT_BOOL ipv4McRouteEnable;

    /** GT_TRUE = enable IPv6 Unicast Routing on the Interface */
    GT_BOOL ipv6UcRouteEnable;

    /** GT_TRUE = enable IPv6 Multicast Routing on the interface */
    GT_BOOL ipv6McRouteEnable;

    /** GT_TRUE = enable MPLS switching on the interface */
    GT_BOOL mplsRouteEnable;

    /** Determines the Virtual Router ID for packets received on this interface */
    GT_U32 vrfId;

    /** redirect command in the router engine */
    GT_BOOL ipv4IcmpRedirectEnable;

    /** redirect command in the Router engine */
    GT_BOOL ipv6IcmpRedirectEnable;

    /** Bridge routed Interface */
    GT_BOOL bridgeRouterInterfaceEnable;

    /** Determines the IP security profile of packets */
    GT_U32 ipSecurityProfile;

    /** Enable\disable trapping/mirroring all IPv4 IGMP packets */
    GT_BOOL ipv4IgmpToCpuEnable;

    /** Enable\disable trapping/mirroring all IPv6 ICMP packets */
    GT_BOOL ipv6IcmpToCpuEnable;

    /** @brief GT_TRUE = IPv4 Broadcast UDP packets, are trapped or mirrored
     *  based on their destination UDP port trapped to the CPU
     */
    GT_BOOL udpBcRelayEnable;

    /** Enable\disable trapping/mirroring of ARP Broadcast packets */
    GT_BOOL arpBcToCpuEnable;

    /** GT_TRUE = The router will perform a lookup on the ARP packet */
    GT_BOOL arpBcToMeEnable;

    /** GT_TRUE = RIPv1 packets are mirrored or trapped to the CPU */
    GT_BOOL ripv1MirrorEnable;

    /** GT_TRUE = IPv4 packets with the LL MC DA */
    GT_BOOL ipv4LinkLocalMcCommandEnable;

    /** GT_TRUE = IPv6 packets with the LL MC DA */
    GT_BOOL ipv6LinkLocalMcCommandEnable;

    /** GT_TRUE = IPv6 Neighbor Solicitation */
    GT_BOOL ipv6NeighborSolicitationEnable;

    /** GT_TRUE = the packet is mirrored to the Ingress Analyzer port */
    GT_BOOL mirrorToAnalyzerEnable;

    /** GT_TRUE = the packet is mirrored to the CPU */
    GT_BOOL mirrorToCpuEnable;

} PRV_TGF_INLIF_ENTRY_STC;


/**
* @enum PRV_TGF_INLIF_PORT_MODE_ENT
 *
 * @brief Port InLIF Mode enumeration
*/
typedef enum{

    /** @brief the port is configured to support
     *  a single input logical interfaces.
     */
    PRV_TGF_INLIF_PORT_MODE_PORT_E,

    /** @brief The port is configured to support
     *  an input vlan inlif entry.
     */
    PRV_TGF_INLIF_PORT_MODE_VLAN_E

} PRV_TGF_INLIF_PORT_MODE_ENT;

/**
* @enum PRV_TGF_INLIF_UNK_UNREG_ADDR_ENT
 *
 * @brief This enum defines supported unkown/unreg addresses types
*/
typedef enum{

    /** Unknown unicast SA */
    PRV_TGF_INLIF_UNK_UC_SA_E,

    /** Unknown unicast DA */
    PRV_TGF_INLIF_UNK_UC_DA_E,

    /** Unregistered Non IP Multicast */
    PRV_TGF_INLIF_UNREG_NON_IP_MC_E,

    /** Unregistered Non IPv4 Broadcast */
    PRV_TGF_INLIF_UNREG_NON_IP4_BC_E,

    /** Unregistered IPv4 or IPv6 Multicast */
    PRV_TGF_INLIF_UNREG_IP_MC_E,

    /** Unregistered IPv4 Broadcast */
    PRV_TGF_INLIF_UNREG_IP_V4_BC_E

} PRV_TGF_INLIF_UNK_UNREG_ADDR_ENT;

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfInlifPortModeSet function
* @endinternal
*
* @brief   The function configured the InLIf Lookup mode for the Ingress port.
*
* @param[in] portIndex                - port index
* @param[in] inlifMode                - InLIF Lookup Mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifPortModeSet
(
    IN  GT_U8                                portIndex,
    IN  PRV_TGF_INLIF_PORT_MODE_ENT          inlifMode
);

/**
* @internal prvTgfInlifPortModeGet function
* @endinternal
*
* @brief   Get the InLIf Lookup mode for the Ingress port.
*         Each port is independent and can work in every mode.
* @param[in] portIndex                - port index
*
* @param[out] inlifModePtr             - (pointer to)InLIF Lookup Mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifPortModeGet
(
    IN  GT_U8                            portIndex,
    OUT PRV_TGF_INLIF_PORT_MODE_ENT      *inlifModePtr
);

/**
* @internal prvTgfInlifUnkUnregAddrCmdSet function
* @endinternal
*
* @brief   This function set the Inlif's unknown/unregistered addresses command
*
* @param[in] devNum                   - device number
* @param[in] inlifType                - inlif type port/vlan/external
* @param[in] inlifIndex               - inlif index in the table
* @param[in] addrType                 - Supported unkown/unreg addresses types
* @param[in] cmd                      - Flood:Flood the packet according to its VLAN assignment.
*                                      valid options:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_BRIDGE_E
*                                      Mirror:Mirror the packet to the CPU with specific CPU
*                                      code for Unknown Unicast.
*                                      Trap: Trap the packet to the CPU with specific CPU
*                                      code for Unknown Unicast.
*                                      Hard Drop: Hard Drop the packet.
*                                      Soft Drop: Soft Drop the packet.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_UNFIXABLE_ECC_ERROR   - the CPSS detected ECC error that can't
*                                       be fixed when read from the memory that
*                                       protected by ECC generated.
*                                       if entry can't be fixed due to 2 data errors
*                                       NOTE: relevant only to memory controller that
*                                       work with ECC protection , and the CPSS need
*                                       to emulate ECC protection.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not ExMxPm family
* @retval GT_FAIL                  - otherwise
*
* @note InLIF supported only for ExMxPm.
*
*/
GT_STATUS prvTgfInlifUnkUnregAddrCmdSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_INLIF_TYPE_ENT              inlifType,
    IN  GT_U32                              inlifIndex,
    IN  PRV_TGF_INLIF_UNK_UNREG_ADDR_ENT    addrType,
    IN  CPSS_PACKET_CMD_ENT                 cmd
);


/**
* @internal prvTgfInlifIntForceVlanModeOnTtSet function
* @endinternal
*
* @brief   Set the Force-Vlan-Mode-on-TT (Enable/Disable).
*         Enabling it means that all packets that have a hit at the TT
*         (Tunnel-Termination) lookup are implicitly assgined
*         a per-VLAN inlif (no matter what is the port mode).
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE = Force: If TT rule hit, IPCL Configuration
*                                      table index - packet VLAN assignment
*                                      GT_FALSE = Don't Force: Doesn't effect the existing
*                                      Policy Configuration table access logic
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifIntForceVlanModeOnTtSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfInlifIntForceVlanModeOnTtGet function
* @endinternal
*
* @brief   Get the Force-Vlan-Mode-on-TT status.
*         Enable means that all packets that have a hit at the TT
*         (Tunnel-Termination) lookup are implicitly assgined
*         a per-VLAN inlif (no matter what is the port mode).
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE = Force: If TT rule hit, IPCL Configuration
*                                      table index - packet VLAN assignment
*                                      GT_FALSE = Don't Force: Doesn't effect the existing
*                                      Policy Configuration table access logic
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifIntForceVlanModeOnTtGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);


/**
* @internal prvTgfInlifAutoLearnEnableSet function
* @endinternal
*
* @brief   Set the Automatic Learning (Disable/Enable).
*         When enabled, the device learns the new source MAC Addresses received
*         on this interface automatically and, according to the setting for <NA
*         Message To Cpu Enable>, sends or does not send NA messages to the
*         CPU.
*         When disabled the device does not learn the new source MAC Addresses
*         received on this interface automatically.
* @param[in] devNum                   - device number
* @param[in] inlifType                - inlif type port/vlan/external
* @param[in] inlifIndex               - inlif index in the table
*                                      (APPLICABLE RANGES: 0..63) for inLif type port or
*                                      (APPLICABLE RANGES: 0..4095) for inLif type vlan or
*                                      (APPLICABLE RANGES: Puma2 4096..65535; Puma3 4096..16383)
*                                      for inLif type external
* @param[in] enable                   - GT_TRUE = auto-learning enabled.
*                                      GT_FALSE = auto-learning disabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_UNFIXABLE_ECC_ERROR   - the CPSS detected ECC error that can't
*                                       be fixed when read from the memory that
*                                       protected by ECC generated.
*                                       if entry can't be fixed due to 2 data errors
*                                       NOTE: relevant only to memory controller that
*                                       work with ECC protection , and the CPSS need
*                                       to emulate ECC protection.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifAutoLearnEnableSet
(
    IN GT_U8                        devNum,
    IN PRV_TGF_INLIF_TYPE_ENT       inlifType,
    IN GT_U32                       inlifIndex,
    IN GT_BOOL                      enable
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfInlifGenh */



