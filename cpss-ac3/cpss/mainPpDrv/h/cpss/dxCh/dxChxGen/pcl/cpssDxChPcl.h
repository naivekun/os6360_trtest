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
* @file cpssDxChPcl.h
*
* @brief CPSS DxCh PCL lib API.
*
* @version   91
********************************************************************************
*/
#ifndef __cpssDxChPclh
#define __cpssDxChPclh

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>

/**
* General NOTEs about portGroupsBmp In TCAM related APIs:
*   1. In Falcon :
*     the TCAM is shared between 2 port groups.
*     the caller need to use 'representative' port groups , for example:
*     value 0x00000014 hold bits 2,4 represent TCAMs of port groups 2,3 and 4,5
*     value 0x00000041 hold bits 0,6 represent TCAMs of port groups 0,1 and 6,7
*     Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.(represent all port groups)
*
**/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Size of Bobcat2; Caelum; Bobcat3 action is 8 words                    */
/* Size of DxCh3, xCat, xCat3, Lion, xCat2 , Lion2 action is 4 words */
#define CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS 8

/* Rule entry size in words - defined by Bobcat2; Caelum; Bobcat3 size 7 x 3 words */
#define CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS  21

/**
* @enum CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT
 *
 * @brief This enum describes the type of second sublookup of Ingress PCL lookup0.
*/
typedef enum{

    /** @brief Partial lookup -
     *  in IPCL01 the Configuration table is accesses with the
     *  same index as the IPCL0-0 index.
     */
    CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_PARTIAL_E,

    /** @brief Full lookup -
     *  IPCL01 lookup is a full lookup that includes a Policy
     *  Configuration table lookup and a new key creation.
     *  IPCL0-1 accesses the IPCL0 Configuration table, and uses the
     *  <Enable Lookup0-1> and <PCL-ID for Lookup0-1> fields.
     *  Notice that IPCL0-1 and IPCL0-0 may access different entries
     *  in the configuration table, depending on the configuration
     *  table lookup mechanism.
     */
    CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_FULL_E

} CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT;

/**
* @enum CPSS_DXCH_PCL_RULE_OPTION_ENT
 *
 * @brief Enumerator for the TCAM rule write option flags.
*/
typedef enum{

    /** @brief write all fields
     *  of rule to TCAM but rule state is invalid
     *  (no match during lookups).
     *  The cpssDxChPclRuleValidStatusSet can turn the rule
     *  to valid state.
     *  (APPLICABLE DEVICES: DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E      = (0x1 << 0)

} CPSS_DXCH_PCL_RULE_OPTION_ENT;

/* Action definitions for Policy Rules */

/**
* @struct CPSS_DXCH_PCL_ACTION_MIRROR_STC
 *
 * @brief This structure defines the mirroring related Actions.
*/
typedef struct{

    /** @brief The CPU code assigned to packets
     *  Mirrored to CPU or Trapped to CPU due
     *  to a match in the Policy rule entry
     *  NOTE: the CPU code also acts as 'drop code' for soft/hard drop commands.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *
     *  Relevant for egress actions , for next devices:
     *  APPLICABLE DEVICES : Falcon.
     */
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;

    /** @brief Enables mirroring the packet to
     *  the ingress analyzer interface.
     *  GT_FALSE - Packet is not mirrored to ingress analyzer interface.
     *  GT_TRUE - Packet is mirrored to ingress analyzer interface.
     *
     */
    GT_BOOL mirrorToRxAnalyzerPort;

    /** @brief index of analyzer interface
     *  (APPLICABLE RANGES: 0..6)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Relevant only when (mirrorToRxAnalyzerPort == GT_TRUE)
     *
     */
    GT_U32 ingressMirrorToAnalyzerIndex;

    /** @brief TCP RST and FIN packets
     *  (i.e. TCP disconnect cases) can be mirrored to CPU
     *  GT_TRUE - TCP RST and FIN packets mirrored to CPU.
     *  GT_FALSE - TCP RST and FIN packets not mirrored to CPU.
     *  Relevant for xCat and above devices only.
     *  Comments:
     */
    GT_BOOL mirrorTcpRstAndFinPacketsToCpu;

    /** @brief Enables mirroring the packet to
     *  the egress analyzer interface.
     *  GT_FALSE - Packet is not mirrored to egress analyzer interface.
     *  GT_TRUE - Packet is mirrored to egress analyzer interface.
     *
     *  Relevant for egress actions , for next devices:
     *  APPLICABLE DEVICES : Falcon.
     */
    GT_BOOL mirrorToTxAnalyzerPortEn;

    /** @brief index of analyzer interface
     *  (APPLICABLE RANGES: 0..6)
     *  (APPLICABLE DEVICES: Falcon)
     *  Relevant only when (mirrorToTxAnalyzerPort == GT_TRUE)
     */
    GT_U32 egressMirrorToAnalyzerIndex;

    /** @brief egress mode
     *  Relevant only when (mirrorToTxAnalyzerPort == GT_TRUE)
     *
     *  Relevant for egress actions , for next devices:
     *  APPLICABLE DEVICES : Falcon.
     */
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirrorToAnalyzerMode;

} CPSS_DXCH_PCL_ACTION_MIRROR_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC
 *
 * @brief This structure defines the using of rule match counter.
*/
typedef struct{

    /** @brief Enables the binding of this
     *  policy action entry to the Policy Rule Match Counter<n>
     *  (0<=n<32) indexed by matchCounterIndex
     *  GT_FALSE = Match counter binding is disabled.
     *  GT_TRUE = Match counter binding is enabled.
     */
    GT_BOOL enableMatchCount;

    /** @brief A index one of the 32 Policy Rule Match Counter<n>
     *  (0<=n<32) The counter is incremented for every packet
     *  satisfying both of the following conditions:
     *  - Matching this rule.
     *  - The previous packet command is not hard drop.
     *  NOTE: for DxCh3 and above (APPLICABLE RANGES: 0..(16K-1))
     *  Comments:
     *  DxCh3 and above: the CNC mechanism is used for the match counters.
     */
    GT_U32 matchCounterIndex;

} CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC;


/**
* @enum CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT
 *
 * @brief Enumerator for ingress Tag1 UP assignment command.
*/
typedef enum{

    /** @brief Do not modify the <UP1> assigned to the packet
     *  by previous engines.
     */
    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E,

    /** @brief If packet does not contain Tag1
     *  assign according to action entry’s <UP1>,
     *  else retain previous engine <UP1> assignment
     */
    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E,

    /** @brief If packet contains Tag0 use UP0,
     *  else use action entry’s <UP1> field.
     */
    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E,

    /** Assign action entry’s <UP1> field to all packets. */
    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E

} CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT;

/**
* @enum CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT
 *
 * @brief Enumerator for egress Tag0 VID and UP assignment command.
*/
typedef enum{

    /** don't assign Tag0 value from entry. */
    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E,

    /** @brief assign Tag0 value from entry to outer Tag.
     *  It is Tag0 for not TS packets
     *  or Tunnel header Tag for TS packets.
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E,

    /** @brief assign Tag0 value from entry to Tag0
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E

} CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT;

/**
* @enum CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT
 *
 * @brief Enumerator for egress DSCP assignment command.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef enum{

    /** Keep previous packet DSCP/EXP settings. */
    CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E = GT_FALSE,

    /** @brief Modify Outer
     *  If packet is MPLS, modify EXP of
     *  packet's outer label to <DSCP EXP>.
     *  If the packet is tunneled, this refers to the tunnel header
     *  Otherwise, If packet is IP, modify the packet's DSCP to <DSCP EXP>.
     *  If the packet is tunneled, this refers to the tunnel header.
     *  (Backward Compatible mode).
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E = GT_TRUE,

    /** @brief Modify Inner DSCP;
     *  If packet is IP, modify the packet's DSCP to <DSCP EXP>.
     *  If the packet is tunneled, this refers to the passenger header.
     *  This mode is not applicable for MPLS packets;
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E

} CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT;


/*
 * Typedef: union CPSS_DXCH_PCL_ACTION_QOS_UNT
 *
 * Description:
 *     This union defines the packet's QoS attributes mark Actions.
 *
 * Fields:
 *
 *    ingress members:
 *    modifyDscp             - The Modify DSCP (or EXP for MPLS) QoS attribute of the packet.
 *                             Enables modification of the packet's DSCP field.
 *                             Only relevant if QoS precedence of the previous
 *                             QoS assignment mechanisms (Port, Protocol Based
 *                             QoS, and previous matching rule) is Soft.
 *                             Relevant for IPv4/IPv6 packets, only.
 *                             ModifyDSCP enables the following:
 *                               - Keep previous DSCP modification command.
 *                               - Enable modification of the DSCP field in
 *                                 the packet.
 *                               - Disable modification of the DSCP field in
 *                                 the packet.
 *    modifyUp              - The Modify UP QoS attribute of the packet.
 *                            Enables modification of the packet's
 *                            802.1p User Priority field.
 *                            Only relevant if QoS precedence of the previous
 *                            QoS assignment mechanisms  (Port, Protocol Based
 *                            QoS, and previous matching rule) is Soft.
 *                            ModifyUP enables the following:
 *                            - Keep previous QoS attribute <ModifyUP> setting.
 *                            - Set the QoS attribute <modifyUP> to 1.
 *                            - Set the QoS attribute <modifyUP> to 0.
 *    profileIndex          - The QoS Profile Attribute of the packet.
 *                             Only relevant if the QoS precedence of the
 *                             previous QoS Assignment Mechanisms (Port,
 *                             Protocol Based QoS, and previous matching rule)
 *                             is Soft and profileAssignIndex is set
 *                             to GT_TRUE.
 *                             The QoSProfile is used to index the QoSProfile
 *                             Table Entry and assign the QoS Parameters
 *                             which are TC, DP, UP, EXP and DSCP
 *                             to the packet.
 *                             Valid Range - 0..71 for DxCh1
 *                             0..127 for DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2
 *                             0..255 for Bobcat2; Caelum; Bobcat3, Aldrin, AC3X.
 *    profileAssignIndex     - Enable marking of QoS Profile Attribute of
 *                             the packet.
 *                             GT_TRUE - Assign <profileIndex> to the packet.
 *                             GT_FALSE - Preserve previous QoS Profile setting.
 *    profilePrecedence      - Marking of the QoSProfile Precedence.
 *                             Setting this bit locks the QoS parameters setting
 *                             from being modified by subsequent QoS
 *                             assignment engines in the ingress pipe.
 *                             QoSPrecedence enables the following:
 *                             - QoS precedence is soft and the packet's QoS
 *                               parameters may be overridden by subsequent
 *                               QoS assignment engines.
 *                             - QoS precedence is hard and the packet's QoS
 *                               parameters setting is performed until
 *                               this stage is locked. It cannot be overridden
 *                               by subsequent QoS assignment engines.
 *
 *    up1Cmd                 - (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *                              command for setting the Tag1 UP value.
 *    up1                    - (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *                              The UP1 value to assign to the packet
 *                              according to up1Cmd.
 *    egress members:
 *    modifyDscp             - enable modify DSCP (or EXP for MPLS)
 *                             For xCat; xCat3; Lion; xCat2; Lion2 as Boolean:
 *                             CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_CMD_KEEP_E - don't modify DSCP.
 *                             any other - modify DSCP.
 *                             For Bobcat2; Caelum; Bobcat3; Aldrin see enum comments.
 *    dscp                   - The DSCP field (or EXP for MPLS) set to the transmitted packets.
 *    modifyUp               - This field determines the Tag0 UP assigned
 *                             to the packet matching the EPCL entry.
 *    up                     - The 802.1p UP field set to the transmitted packets.
 *    up1ModifyEnable        - This field determines the Tag1 UP assigned
 *                             to the packet matching the EPCL entry.
 *                             GT_FALSE - Keep: Keep previous Tag1 UP
 *                                assigned to the packet.
 *                             GT_TRUE  - Modify: EPCL Action entry <UP1>
 *                                is assigned to tag 1 UP of the packet.
 *                            (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *    up1                    - The UP1 value to assign to Tag1 of
 *                             the packet according to <up1ModifyEnable>
 *                            (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *
 *  Comments:
 *           Egress Policy related actions supported only for DxCh2 and above
 *           devices
 *
 */
typedef union
{
    struct
    {
        CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT         modifyDscp;
        CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT         modifyUp;
        GT_U32                                        profileIndex;
        GT_BOOL                                       profileAssignIndex;
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT   profilePrecedence;
        CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT      up1Cmd;
        GT_U32                                        up1;
    } ingress;
    struct
    {
        CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT  modifyDscp;
        GT_U32                                        dscp;
        CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT      modifyUp;
        GT_U32                                        up;
        GT_BOOL                                       up1ModifyEnable;
        GT_U32                                        up1;
    } egress;
} CPSS_DXCH_PCL_ACTION_QOS_UNT;

/**
* @enum CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT
 *
 * @brief enumerator for PCL redirection target
*/
typedef enum{

    /** no redirection */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,

    /** redirection to output interface */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E,

    /** @brief Ingress control pipe not
     *  bypassed. If packet is triggered for routing,
     *  Redirect the packet to the Router Lookup Translation
     *  Table Entry specified in <routerLttIndex> bypassing
     *  DIP lookup.
     *  (APPLICABLE DEVICES: DxCh2, DxCh3, xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon).
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E,

    /** @brief redirect to virtual
     *  router with the specified Id (VRF ID)
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon).
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E,

    /** @brief replace MAC source address
     *  with specified value.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon).
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E,

    /** @brief assign logical source port
     *  (APPLICABLE DEVICES: xCat; xCat3) xCat device: applicable starting from revision C0
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E

} CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT;

/**
* @enum CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT
 *
 * @brief enumerator for PCL redirection types of the passenger packet.
*/
typedef enum{

    /** @brief The passenger packet
     *  is Ethernet.
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E,

    /** The passenger packet is IP. */
    CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E

} CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT;

/**
* @struct CPSS_DXCH_PCL_ACTION_REDIRECT_STC
 *
 * @brief This structure defines the redirection related Actions.
*/
typedef struct
{
    CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT  redirectCmd;
    union
    {
        struct
        {
            CPSS_INTERFACE_INFO_STC  outInterface;
            GT_BOOL                  vntL2Echo;
            GT_BOOL                  tunnelStart;
            GT_U32                   tunnelPtr;
            CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT tunnelType;
            GT_U32                   arpPtr;
            GT_BOOL                  modifyMacDa;
            GT_BOOL                  modifyMacSa;
        } outIf;
        GT_U32                       routerLttIndex;
        GT_U32                       vrfId;
        struct
        {
            GT_U32                   arpPtr;
            GT_ETHERADDR             macSa;
        } modifyMacSa;
        struct
        {
            GT_U32                   arpPtr;
            GT_BOOL                  modifyMacDa;
        } noRedirect;
        struct
        {
            CPSS_INTERFACE_INFO_STC  logicalInterface;
            GT_BOOL                  sourceMeshIdSetEnable;
            GT_U32                   sourceMeshId;
            GT_BOOL                  userTagAcEnable;
        } logicalSourceInterface;
    } data;

} CPSS_DXCH_PCL_ACTION_REDIRECT_STC;


/**
* @enum CPSS_DXCH_PCL_POLICER_ENABLE_ENT
 *
 * @brief enumerator for policer options.
*/
typedef enum{

    /** Meter and Counter are disabled. */
    CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E              = GT_FALSE,

    /** Both Meter and Counter enabled. */
    CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E = GT_TRUE,

    /** Meter only enabled. */
    CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E,

    /** Counter only enabled. */
    CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E

} CPSS_DXCH_PCL_POLICER_ENABLE_ENT;

/**
* @struct CPSS_DXCH_PCL_ACTION_POLICER_STC
 *
 * @brief This structure defines the policer related Actions.
*/
typedef struct{

    /** @brief policer enable,
     *  DxCh1, DxCh2 and DxCh3 supports only the following commands:
     *  - CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E or GT_FALSE
     *  - CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E or GT_TRUE
     *  xCat and above devices support all values of the enum
     */
    CPSS_DXCH_PCL_POLICER_ENABLE_ENT policerEnable;

    /** @brief policers table entry index
     *  Comments:
     */
    GT_U32 policerId;

} CPSS_DXCH_PCL_ACTION_POLICER_STC;

/**
* @enum CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT
 *
 * @brief Enumerator for ingress Tag1 VID assignment command.
*/
typedef enum{

    /** Do not modify Tag1 VID. */
    CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E,

    /** @brief The entry’s <Tag1 VID> is assigned as Tag1 VID for
     *  packets that do not have Tag1
     *  or Tag1-priority tagged packets.
     *  For packets received with Tag1 VID,
     *  retain previous engine assignment.
     */
    CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E,

    /** Assign action entry’s <VID1> field to all packets. */
    CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E

} CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT;

/*
 * Typedef: union CPSS_DXCH_PCL_ACTION_VLAN_UNT
 *
 * Description:
 *     This union defines the VLAN modification related Actions.
 *
 * Fields:
 *       INGRESS fields:
 *       modifyVlan  - VLAN id modification command
 *
 *       nestedVlan  -  When this field is set to GT_TRUE, this rule matching
 *                      flow is defined as an access flow. The VID of all
 *                      packets received on this flow is discarded and they
 *                      are assigned with a VID using the device's VID
 *                      assignment algorithms, as if they are untagged. When
 *                      a packet received on an access flow is transmitted via
 *                      a core port or a Cascading port, a VLAN tag is added
 *                      to the packet (on top of the existing VLAN tag, if
 *                      any). The VID field is the VID assigned to the packet
 *                      as a result of all VLAN assignment algorithms. The
 *                      802.1p User Priority field of this added tag may be
 *                      one of the following, depending on the ModifyUP QoS
 *                      parameter set to the packet at the end of the Ingress
 *                      pipe:
 *                      - If ModifyUP is GT_TRUE, it is the UP extracted
 *                      from the QoSProfile to QoS Table Entry<n>
 *                      - If ModifyUP is GT_FALSE, it is the original packet
 *                      802.1p User Priority field if it is tagged and is UP
 *                      if the original packet is untagged.
 *
 *       vlanId      - VLAN id used for VLAN id modification if command
 *                     not CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E
 *       precedence  - The VLAN Assignment precedence for the subsequent
 *                     VLAN assignment mechanism, which is the Policy engine
 *                     next policy-pass rule. Only relevant if the
 *                     VID precedence set by the previous VID assignment
 *                     mechanisms (Port, Protocol Based VLANs, and previous
 *                     matching rule) is Soft.
 *                     - Soft precedence The VID assignment can be overridden
 *                       by the subsequent VLAN assignment mechanism,
 *                       which is the Policy engine.
 *                     - Hard precedence The VID assignment is locked to the
 *                       last VLAN assigned to the packet and cannot be overridden.
 *       vlanId1Cmd  - (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *                     This field determines the Tag1 VID assigned
 *                     to the packet matching the IPCL entry.
 *       vlanId1     - (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *                     The Tag1 VID to assign to the packet
 *                     according to <vlanId1Cmd>.
 *       EGRESS fields:
 *       vlanCmd     - This field determines the Tag0 VID command
 *                     assigned to the packet matching the EPCL entry.
 *       vlanId      - The VID to assign to Tag0
 *                     of the packet according to <vlanCmd>
 *       vlanId1ModifyEnable - (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *                     This field determines the Tag1 VID
 *                     assigned to the packet matching the EPCL entry.
 *                     GT_TRUE - modify, GT_FALSE - don't modify.
 *       vlanId1     - (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *                     The VID to assign to Tag1 of the packet
 *                     according to <vlanId1ModifyEnable>.
 *
 *  Comments:
 *
 */
typedef union
{
    struct
    {
        CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT           modifyVlan;
        GT_BOOL                                        nestedVlan;
        GT_U32                                         vlanId;
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT    precedence;
        CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT  vlanId1Cmd;
        GT_U32                                         vlanId1;
    } ingress;
    struct
    {
        CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT       vlanCmd;
        GT_U32                                         vlanId;
        GT_BOOL                                        vlanId1ModifyEnable;
        GT_U32                                         vlanId1;
    } egress;
} CPSS_DXCH_PCL_ACTION_VLAN_UNT;

/**
* @struct CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC
 *
 * @brief This structure defines the IP unicast route parameters.
 * Dedicated to override the relevant field of
 * general action definitions.
 * APPLICABLE DEVICES: DxCh1, DxCh1_Diamond, xCat, xCat3, Lion, xCat2, Lion2.
*/
typedef struct{

    /** @brief Configure IP Unicast Routing Actions
     *  GT_TRUE - the action used for IP unicast routing
     *  GT_FALSE - the action is not used for IP unicast
     *  routing, all data of the structure
     *  ignored.
     */
    GT_BOOL doIpUcRoute;

    /** Route Entry ARP Index to the ARP Table (10 bit) */
    GT_U32 arpDaIndex;

    /** @brief Decrement IPv4 <TTL> or IPv6 <Hop Limit> enable
     *  GT_TRUE - TTL Decrement for routed packets is enabled
     *  GT_FALSE - TTL Decrement for routed packets is disabled
     */
    GT_BOOL decrementTTL;

    /** @brief Bypass Router engine TTL and Options Check
     *  GT_TRUE - the router engine bypasses the
     *  IPv4 TTL/Option check and the IPv6 Hop
     *  Limit/Hop-by-Hop check. This is used for
     *  IP-TO-ME host entries, where the packet
     *  is destined to this device
     *  GT_FALSE - the check is not bypassed
     */
    GT_BOOL bypassTTLCheck;

    /** @brief ICMP Redirect Check Enable
     *  GT_TRUE - the router engine checks if the next hop
     *  VLAN is equal to the packet VLAN
     *  assignment, and if so, the packet is
     *  mirrored to the CPU, in order to send an
     *  ICMP Redirect message back to the sender.
     *  GT_FALSE - the check disabled
     *  Comments:
     *  To configure IP Unicast route entry next elements of the action struct
     *  should be configured
     *  1. doIpUcRoute set to GT_TRUE
     *  2. redirection action set to CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E
     *  Out Interface data should be configured too.
     *  3. Policer should be disabled
     *  4. VLAN command CPSS_DXCH_PCL_ACTION_VLAN_CMD_MODIFY_ALL_E
     *  5. Nested VLAN should be disabled
     *  6. The packet command <pktCmd> should be set according to route entry
     *  purpose:
     *  - SOFT_DROP    - packet is dropped
     *  - HARD_DROP    - packet is dropped
     *  - TRAP      - packet is trapped to CPU with CPU code
     *  IPV4_UC_ROUTE or IPV6_UC_ROUTE
     *  - FORWARD     - packet is routed
     *  - MIRROR_TO_CPU  - packet is routed and mirrored to the CPU with
     *  CPU code IPV4_UC_ROUTE or IPV6_UC_ROUTE
     */
    GT_BOOL icmpRedirectCheck;

} CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC
 *
 * @brief This structure defines packet Source Id assignment.
*/
typedef struct{

    /** @brief assign Source Id enable:
     *  - GT_TRUE - Assign Source Id.
     *  - GT_FALSE - Don't assign Source Id.
     */
    GT_BOOL assignSourceId;

    /** @brief the Source Id value to be assigned
     *  Comments:
     */
    GT_U32 sourceIdValue;

} CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_OAM_STC
 *
 * @brief This structure defines packet OAM relevant fields.
*/
typedef struct{

    /** @brief indicates that a timestamp should be inserted into the
     *  packet:
     *  - GT_TRUE - enable timestamp insertion.
     *  - GT_FALSE - disable timestamp insertion.
     */
    GT_BOOL timeStampEnable;

    /** @brief When <Timestamp Enable> is enabled, indicates
     *  the offset index for the timestamp offset table.
     *  (APPLICABLE RANGES: 0..15)
     */
    GT_U32 offsetIndex;

    /** @brief binds the packet to an entry in the OAM Table.
     *  - GT_TRUE - enable OAM packet processing.
     *  - GT_FALSE - disable OAM packet processing.
     */
    GT_BOOL oamProcessEnable;

    /** @brief determines the set of UDBs where the key attributes
     *  (opcode, MEG level, RDI, MEG level) of the OAM message
     *  are taken from.
     *  (APPLICABLE RANGES: 0..1)
     *  Comments:
     */
    GT_U32 oamProfile;

} CPSS_DXCH_PCL_ACTION_OAM_STC;

/**
* @enum CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT
 *
 * @brief enumerator Controls the index used for IPCL lookup.
*/
typedef enum{

    /** @brief Retain;
     *  use SrcPort/VLAN <Lookup PCL Cfg Mode>.
     */
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E,

    /** @brief Override;
     *  use TTI/PCL Action <ipclConfigIndex>.
     */
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E

} CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT;

/**
* @struct CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC
 *
 * @brief This structure defines future lookups configuration.
*/
typedef struct{

    /** @brief index of Pcl Configuration table for next lookup.
     *  0 means no value to update.
     */
    GT_U32 ipclConfigIndex;

    /** @brief algorithm of selection
     *  index of PCL Cfg Table for IPCL lookup0_1
     */
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT pcl0_1OverrideConfigIndex;

    /** @brief algorithm of selection
     *  index of PCL Cfg Table for IPCL lookup1
     *  Comments:
     */
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT pcl1OverrideConfigIndex;

} CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC;

/**
* @struct CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC
 *
 * @brief This structure defines Source ePort Assignment.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    /** @brief GT_FALSE
     *  - GT_TRUE - Assign source ePort from sourcePortValue.
     */
    GT_BOOL assignSourcePortEnable;

    /** @brief Source ePort value.
     *  Comments:
     */
    GT_U32 sourcePortValue;

} CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC;

/**
 * @struct CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC
 *
 * @brief A structure that describes latency PCL action.
 */
typedef struct
{
    /** @brief Enable latency monitoring */
    GT_BOOL monitoringEnable;

    /** @brief Assign latency monitoring profile */
    GT_U32  latencyProfile;
} CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC;


/**
 * @enum CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENT
 *
 * @brief This enumeration lists all the Egress PCL Action PHA thread-id assignment modes.
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef enum{
    /** @brief the threadId is not modified by the EPCL action */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_DISABLED_E,
    /** @brief the threadId is modified by the EPCL action .
     *  NOTE: error : GT_NOT_INITIALIZED if the PHA library was not initialized.
    */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E,
    /** @brief the EPCL action set to bypass the PHA thread processing
     *  NOTE: error : GT_NOT_INITIALIZED if the PHA library was not initialized.
    */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_BYPASS_PHA_E
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENT;

/**
 * @enum CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_ENT
 *
 * @brief This enumeration lists all the Egress PCL Action PHA thread info types.
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef enum{
    /** @brief the PHA threadId is not used */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E,
    /*  @brief
     *  NOTE: error : GT_NOT_INITIALIZED if the PHA library was not initialized.
    */
    /** @brief type for thread to handle IOAM Ipv4/Ipv6 packets in the ingress switch in the system */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E,

    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD___LAST___E   /* not to be used */
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_ENT;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv4/ipv6 packets in the ingress switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{
    /** @brief indication that the IOAM includes both the E2E option and trace option
        GT_TRUE  - both the E2E option and trace option
        GT_FALSE - only one option exists
    */
    GT_BOOL  ioamIncludesBothE2EOptionAndTraceOption;
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_STC;

/**
 * @struct CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_UNT
 *
 * @brief This union if for all the Egress PCL Action PHA thread info types.
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef union{
    /** @brief dummy field for type that no extra info needed. */
    GT_U32 notNeeded;

    /** @brief EPCL info for thread to handle IOAM Ipv4/Ipv6 packets in the ingress switch in the system */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_STC  epclIoamIngressSwitch;
}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_UNT;

/**
 * @struct CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC
 *
 * @brief This structure defines the Egress PCL Action fields relate to PHA thread info.
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{
    /** @brief epcl pha thread Id assignment mode.
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENT      phaThreadIdAssignmentMode;
    /** @brief pha threadId . relevant only if phaThreadIdAssignmentMode == CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E
     * (APPLICABLE RANGES: 1..255).
     */
    GT_U32       phaThreadId;
    /** @brief epcl pha thread info type
     *  NOTE: relevant regardless to mode of 'phaThreadIdAssignmentMode'
     *
     *  IMPORTANT: for 'Get' function this parameter is [in] parameter !!!
     *      (if the HW value meaning that value other than
     *      CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E needed)
     *      because CPSS need to use it in order to parse the HW value and fill
     *      the SW 'phaThreadUnion' info.
     *      (CPSS not hold shadow for it , because 'per action' per 'port group'
     *      (and 'per device') too large shadow to support it ...)
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_ENT      phaThreadType;
    /** @brief union of all the epcl pha thread info.(according to phaThreadType)
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_UNT      phaThreadUnion;

}CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC;


/**
* @struct CPSS_DXCH_PCL_ACTION_STC
 *
 * @brief Policy Engine Action
*/
typedef struct{

    /** @brief packet command (forward, mirror
     *  hard-drop, soft-drop, or trap-to-cpu)
     */
    CPSS_PACKET_CMD_ENT pktCmd;

    /** @brief Action Stop
     *  GT_TRUE - to skip the following PCL lookups
     *  GT_FALSE - to continue with following PCL lookups
     *  Supported by DxCh3 and xCat above devices.
     *  DxCh1 and DxCh2 ignores the field.
     *  Relevant to Policy Action Entry only.
     */
    GT_BOOL actionStop;

    /** @brief the Bridge engine processed or bypassed
     *  GT_TRUE - the Bridge engine is bypassed.
     *  GT_FALSE - the Bridge engine is processed.
     */
    GT_BOOL bypassBridge;

    /** @brief the ingress pipe bypassed or not.
     *  GT_TRUE - the ingress pipe is bypassed.
     *  GT_FALSE - the ingress pipe is not bypassed.
     */
    GT_BOOL bypassIngressPipe;

    /** @brief GT_TRUE
     *  GT_FALSE - Action is used for the Ingress Policy
     */
    GT_BOOL egressPolicy;

    /** configuration of IPCL lookups. */
    CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC lookupConfig;

    /** @brief packet mirroring configuration
     *  Relevant for ingress actions only.
     *
     *  Relevant for egress actions , for next devices:
     *  (APPLICABLE DEVICES : Falcon.)
     */
    CPSS_DXCH_PCL_ACTION_MIRROR_STC mirror;

    /** match counter configuration */
    CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC matchCounter;

    /** packet QoS attributes modification configuration */
    CPSS_DXCH_PCL_ACTION_QOS_UNT qos;

    /** @brief packet Policy redirection configuration
     *  Relevant for ingress actions only.
     */
    CPSS_DXCH_PCL_ACTION_REDIRECT_STC redirect;

    /** @brief packet Policing configuration
     *  Relevant to Policy Action Entry only.
     */
    CPSS_DXCH_PCL_ACTION_POLICER_STC policer;

    /** packet VLAN modification configuration */
    CPSS_DXCH_PCL_ACTION_VLAN_UNT vlan;

    /** @brief special DxCh (not relevant for DxCh2 and above) Ip Unicast Route
     *  action parameters configuration.
     *  Relevant to Unicast Route Entry only.
     *  (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; xCat; xCat3; Lion; xCat2; Lion2)
     */
    CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC ipUcRoute;

    /** @brief packet source Id assignment
     *  Relevant to Policy Action Entry only.
     */
    CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC sourceId;

    /** OAM relevant fields. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_PCL_ACTION_OAM_STC oam;

    /** @brief flow Id. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Relevant for ingress actions only.
     */
    GT_U32 flowId;

    /** @brief source ePort Assignment.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC sourcePort;

    /** @brief GT_FALSE
     *  - GT_TRUE - Override the Mac2Me mechanism and set Mac2Me to 0x1.
     *  Ingress PCL Only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL setMacToMe;

    /** @brief When enabled, the MPLS G
     *  to an OAM Opcode that is used by the OAM engine.
     *  - GT_FALSE - Disable.
     *  - GT_TRUE - Enable.
     *  Egress PCL Only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL channelTypeToOpcodeMapEnable;

    /** @brief TM Queue ID assignment.
     *  This value is assigned to the outgoing descriptor.
     *  It may be subsequently modified by the TM Q-Mapper.
     *  Egress PCL Only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
     */
    GT_U32 tmQueueId;

    /** @brief GT_TRUE
     *  GT_FALSE configure action to preserve previous assignment.
     *  Relevant when VPLS mode init parameter is enabled.
     *  (APPLICABLE DEVICES: xCat, xCat3) xCat device: applicable starting from revision C0
     */
    GT_BOOL unknownSaCommandEnable;

    /** @brief Assigns the unknown
     *  Relevant when unknownSaCommandEnable == GT_TRUE.
     *  Relevant when VPLS mode init parameter is enabled.
     *  (APPLICABLE DEVICES: xCat, xCat3) xCat device: applicable starting from revision C0
     *  Comment:
     */
    CPSS_PACKET_CMD_ENT unknownSaCommand;

    /** @brief GT_TRUE - Terminate Cut Through packet mode (switch to Store and Forward).
     *         GT_FALSE - leave packet mode (CT or SF) unchanged.
     *   Terminate Cut Through packet mode (switch to Store and Forward).
     *   Relevant to Egress PCL only.
     *  (APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL terminateCutThroughMode;

    /** @brief Latency monitoring
     *  (APPLICABLE DEVICES: Falcon)
     */
    CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC latencyMonitor;

    /** @brief GT_TRUE - Skip FDB SA lookup.
     *   GT_FALSE - Don't modify the state of Skip FDB SA lookup.
     *  (APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL skipFdbSaLookup;

    /** @brief GT_TRUE - The Ingress PCL will trigger a dedicated
     *         interrupt towards the CPU.
     *   GT_FALSE - will not trigger.
     *   Relevant to Ingress PCL Only.
     *  (APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL triggerInterrupt;

     /** @brief GT_TRUE - The Exact Match action has a priority over the PCL action.
     *          GT_FALSE - The Exact Match action doesn't have a
     *          priority over the PCL action.
     *   (APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL exactMatchOverPclEn;

     /** @brief EPCL action info related to egress PHA thread processing.
     *   (APPLICABLE DEVICES: Falcon)
     */
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC     epclPhaInfo;
} CPSS_DXCH_PCL_ACTION_STC;


/**
* @enum CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT
 *
 * @brief This enum describes possible formats of Policy rules.
*/
typedef enum{

    /** @brief Standard (24B) L2
     *  AKA ingress Key #0
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E                        ,

    /** @brief Standard (24B)
     *  L2+IPv4/v6 QoS
     *  AKA - ingress Key #1
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E                     ,

    /** @brief Standard (24B)
     *  IPv4+L4
     *  AKA - ingress Key #2
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E                       ,

    /** @brief Standard (24B)
     *  DxCh specific IPV6 DIP (used for routing)
     *  AKA - ingress Key #12
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E                      ,

    /** @brief Extended (48B)
     *  L2+IPv4 + L4
     *  AKA - ingress Key #4
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E                      ,

    /** @brief Extended (48B)
     *  L2+IPv6
     *  AKA - ingress Key #5
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E                       ,

    /** @brief Extended (48B)
     *  L4+IPv6
     *  AKA - ingress Key #6
     *  key formats for Egress (DxCh2 and above device)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E                       ,

    /** Standard (24B) L2 */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E                         ,

    /** @brief Standard (24B)
     *  L2+IPv4/v6 QoS
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E                      ,

    /** @brief Standard (24B)
     *  IPv4+L4
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E                        ,

    /** @brief Extended (48B)
     *  L2+IPv4 + L4
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E                      ,

    /** @brief Extended (48B)
     *  L2+IPv6
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E                       ,

    /** @brief Extended (48B)
     *  L4+IPv6
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E                       ,

    /** @brief standard UDB key (24B)
     *  AKA ingress Key #3
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E                          ,

    /** @brief extended UDB key (48B)
     *  AKA ingress Key #7
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E                          ,

    /** @brief (ingress)
     *  Standard (24B) Routed ACL+QoS+IPv4
     *  AKA - ingress Key #10
     *  CH3 : key for IPv4 packets used in first lookup
     *  (APPLICABLE DEVICES: DxCh3, xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E          ,

    /** @brief (ingress)
     *  Extended (48B) Port/VLAN+QoS+IPv4
     *  AKA - ingress Key #8
     *  CH3 : key for IPv4 packets use in second lookup
     *  (APPLICABLE DEVICES: DxCh3, xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E           ,

    /** @brief (ingress)
     *  Ultra (72B) Port/VLAN+QoS+IPv6
     *  AKA - ingress Key #9
     *  CH3 : key for IPv6 packets used in first lookup
     *  (APPLICABLE DEVICES: DxCh3, xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E         ,

    /** @brief (ingress)
     *  Ultra (72B) Routed ACL+QoS+IPv6
     *  AKA - ingress Key #11
     *  CH3 : key for IPv6 packets use in second lookup.
     *  (APPLICABLE DEVICES: DxCh3, xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E        ,

    /** @brief (egress)
     *  Extended (48B) RACL/VACL IPv4
     *  CH3 : Custom egress key for not-IP, IPV4, ARP
     *  (APPLICABLE DEVICES: DxCh3, xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E                ,

    /** @brief (egress)
     *  Ultra (72B) RACL/VACL IPv6
     *  CH3 : Custom egress key for IPV6
     *  (APPLICABLE DEVICES: DxCh3, xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E              ,

    /** @brief (ingress)
     *  10 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E                           ,

    /** @brief (ingress)
     *  20 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E                           ,

    /** @brief (ingress)
     *  30 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E                           ,

    /** @brief (ingress)
     *  40 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E                           ,

    /** @brief (ingress)
     *  50 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E                           ,

    /** @brief (ingress)
     *  50 byte UDB and fixed fields
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E                           ,

    /** @brief (egress)
     *  10 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E                            ,

    /** @brief (egress)
     *  20 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E                            ,

    /** @brief (egress)
     *  30 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E                            ,

    /** @brief (egress)
     *  40 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E                            ,

    /** @brief (egress)
     *  50 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E                            ,

    /** @brief (egress)
     *  50 byte UDB and fixed fields
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E                            ,

    /** @brief (ingress)
     *  60 byte UDB only key
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E           ,

    /** @brief last element to
     *  calculate amount
     */
    CPSS_DXCH_PCL_RULE_FORMAT_LAST_E

} CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT;

/**
* @enum CPSS_DXCH_PCL_OFFSET_TYPE_ENT
 *
 * @brief Offset types for packet headers parsing used for user defined
 * bytes configuration
*/
typedef enum{

    /** offset from start of mac(L2) header */
    CPSS_DXCH_PCL_OFFSET_L2_E,

    /** @brief offset from start of L3 header.
     *  (APPLICABLE DEVICES: DxCh1, DxCh2, DxCh3)
     */
    CPSS_DXCH_PCL_OFFSET_L3_E,

    /** offset from start of L4 header */
    CPSS_DXCH_PCL_OFFSET_L4_E,

    /** @brief offset from start of
     *  IPV6 Extension Header .
     *  (APPLICABLE DEVICES: DxCh1, DxCh2, DxCh3)
     */
    CPSS_DXCH_PCL_OFFSET_IPV6_EXT_HDR_E,

    /** @brief the user defined byte used
     *  for TCP or UDP comparator, not for byte from packet
     *  (APPLICABLE DEVICES: DxCh1, DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2)
     */
    CPSS_DXCH_PCL_OFFSET_TCP_UDP_COMPARATOR_E,

    /** @brief offset from start of
     *  L3 header minus 2. Ethertype of IP.
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,

    /** @brief offset from
     *  Ethertype of MPLS minus 2.
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_OFFSET_MPLS_MINUS_2_E,

    /** @brief offset from Tunnel L2
     *  For IPCL refers to the beginning of the original packet’s MAC header.
     *  (prior to tunnel-termination).
     *  If packet was not tunnel-terminated - this anchor is invalid.
     *  For EPCL refers to the beginning of the outgoing packet’s MAC header
     *  (after tunnel-start).
     *  If packet was not tunnel-start this anchor is invalid;
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_OFFSET_TUNNEL_L2_E,

    /** @brief offset from Tunnel L3
     *  The beginning of the L3 header in tunnel minus 2 bytes.
     *  For IPCL refers to the beginning of original packet's header.
     *  (prior to tunnel termination).
     *  If packet was not tunnel terminated this anchor is invalid.
     *  Only fields in the tunnel can be selected.
     *  Offset cannot exceed 56 bytes.
     *  For EPCL valid only if packet is tunnel start.
     *  Points to the beginning of L4 header in the added tunnel header
     *  minus 2 bytes. Offset cannot exceed 64 bytes.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,

    /** @brief offset from Metadata.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_OFFSET_METADATA_E,

    /** @brief invalid UDB - contains 0 always
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_OFFSET_INVALID_E

} CPSS_DXCH_PCL_OFFSET_TYPE_ENT;

/**
* @enum CPSS_DXCH_PCL_PACKET_TYPE_ENT
 *
 * @brief Packet types.
*/
typedef enum{

    /** @brief IPV4 TCP
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,

    /** @brief IPV4 UDP
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,

    /** @brief MPLS
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E,

    /** @brief IPV4 Fragment
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E,

    /** @brief IPV4 Other
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,

    /** @brief Ethernet Other
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,

    /** @brief User Defined Ethertype
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE_E,

    /** @brief IPV6
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E,

    /** @brief User Defined Ethertype1
     *  (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,

    /** @brief User Defined Ethertype2
     *  (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E,

    /** @brief User Defined Ethertype3
     *  (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E,

    /** @brief User Defined Ethertype4
     *  (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E,

    /** @brief IPV6 TCP
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E,

    /** @brief IPV6 UDP
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E,

    /** @brief IPV6 not UDP and not TCP
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E,

    /** @brief User Defined Ethertype5
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E,

    /** @brief User Defined Ethertype6
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E,

    /** last value (for internal use) */
    CPSS_DXCH_PCL_PACKET_TYPE_LAST_E

} CPSS_DXCH_PCL_PACKET_TYPE_ENT;

/* Maximal UDB index. Devices Bobcat2, Caelum, Aldrin, AC3X support only 50 UDBs */
/* (APPLICABLE DEVICES: Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X) */
#define CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS 70

/* amount of replaceable UDBs in Ingress UDB only keys */
/* (APPLICABLE DEVICES: Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X) */
#define CPSS_DXCH_PCL_INGRESS_UDB_REPLACE_MAX_CNS  12

/**
* @struct CPSS_DXCH_PCL_UDB_SELECT_STC
 *
 * @brief This structure defines User Defined Bytes Selection
 * for UDB only keys.
 * (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    GT_U32 udbSelectArr[CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS];

    GT_BOOL ingrUdbReplaceArr[CPSS_DXCH_PCL_INGRESS_UDB_REPLACE_MAX_CNS];

    /** @brief UDBs 0,1 are replaced with (PCL
     *  padded with leading zeros.
     *  GT_TRUE - enable, GT_FALSE - disable.
     *  Relevant for Egress PCL only.
     */
    GT_BOOL egrUdb01Replace;

    /** @brief <UDB Valid> indication replaces bit 15 of the key,
     *  overriding any other configuration of bit 15 content,
     *  including the configuration in egrUdb01Replace.
     *  GT_TRUE - enable, GT_FALSE - disable.
     *  Relevant for Egress PCL only.
     *  Comments:
     */
    GT_BOOL egrUdbBit15Replace;

} CPSS_DXCH_PCL_UDB_SELECT_STC;

/**
* @enum CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT
 *
 * @brief User Defined Bytes override types.
*/
typedef enum{

    /** @brief VRF ID MSB
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,

    /** @brief VRF ID LSB
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,

    /** @brief Qos profile index
     *  (APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,

    /** @brief Trunk Hash
     *  (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,

    /** @brief Tag1 info
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_TAG1_INFO_E,

    /** @brief VRF ID MODE
     *  common for all key types
     *  (APPLICABLE DEVICES: xCat, xCat3)
     */
    CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MODE_E

} CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT;

/**
* @enum CPSS_DXCH_UDB_ERROR_CMD_ENT
 *
 * @brief This enum defines packet command taken in the UDB error case.
*/
typedef enum{

    /** continue Policy Lookup */
    CPSS_DXCH_UDB_ERROR_CMD_LOOKUP_E,

    /** trap packet to CPU */
    CPSS_DXCH_UDB_ERROR_CMD_TRAP_TO_CPU_E,

    /** hard drop packet */
    CPSS_DXCH_UDB_ERROR_CMD_DROP_HARD_E,

    /** soft drop packet */
    CPSS_DXCH_UDB_ERROR_CMD_DROP_SOFT_E

} CPSS_DXCH_UDB_ERROR_CMD_ENT;

/**
* @enum CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT
 *
 * @brief Type of access to Ingress / Egress PCL configuration Table.
 * Type of access defines how device calculates index of PCL
 * configuration Table for packet.
*/
typedef enum{

    /** @brief by (physical) PORT access mode.
     *  Ingress port or trunk id is used to calculate index of the PCL
     *  configuration table
     */
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E,

    /** @brief by VLAN access mode
     *  VLAN ID is used to calculate index of the PCL configuration table.
     */
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E,

    /** @brief by ePORT access mode.
     *  eport is used to calculate index of the PCL configuration table.
     *  for ingress : local device source ePort is used (12 LSBits)
     *  for egress : target ePort is used (12 LSBits)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E

} CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_ENT
 *
 * @brief This enum defines the ip-total-length check algorithm
 * that used for calculate the key "IP Header OK" bit.
*/
typedef enum{

    /** @brief compare
     *  iptotallengts with the size of L3 level part of the packet
     */
    CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_L3_E,

    /** @brief compare
     *  iptotallengts with the total size of the packet
     */
    CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_L2_E

} CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT
 *
 * @brief EPCL Key<SrcPort> field source selector for DSA tagged packets.
*/
typedef enum{

    /** For DSA tagged packet, <SrcPort> is taken from the DSA tag. */
    CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ORIGINAL_E,

    /** @brief For all packets (DSA tagged or not), <SrcPort> indicates the
     *  local device ingress physical port.
     */
    CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_LOCAL_E

} CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT;


/**
* @enum CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT
 *
 * @brief EPCL Key<Target port> field source selector.
*/
typedef enum{

    /** Local Device Physical Port. */
    CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E,

    /** Final Destination Port. */
    CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_FINAL_E

} CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT;

/*-------------------------------------------------------------------------*/
/*                           Policy Rules Formats                          */
/*-------------------------------------------------------------------------*/

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC
 *
 * @brief This structure describes the common segment of all key formats.
*/
typedef struct{

    /** @brief The PCL
     *  Only 2 MSBs of pclId are used when portListBmp is used
     *  (see notes below about portListBmp).
     */
    GT_U32 pclId;

    /** @brief MAC To Me
     *  1 - packet matched by MAC-To-Me lookup, 0 - not matched
     *  The field relevant only for DxCh3 and above devices.
     *  DxCh1 and DxCh2 ignore the field
     */
    GT_U8 macToMe;

    /** @brief The port number from which the packet ingressed the device.
     *  Port 63 is the CPU port.
     *  Field muxing description(APPLICABLE DEVICES: xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon):
     *  sourcePort is muxed with portListBmp[15:8]
     */
    GT_PHYSICAL_PORT_NUM sourcePort;

    /** @brief Together with sourcePort indicates the network port at which the packet
     *  was received.
     *  Field muxing description(APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon):
     *  sourceDevice is muxed with portListBmp[27:16].
     */
    GT_U32 sourceDevice;

    /** @brief Port list bitmap. (APPLICABLE DEVICES: xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     *  Relevant only when PCL lookup works in port-list mode.
     *  Field muxing description:
     *  portListBmp[7:0] is muxed with pclId[7:0],
     *  portListBmp[15:8] is muxed with sourcePort[7:0]
     *  portListBmp[27:16] is muxed with sourceTrunkId (APPLICABLE DEVICES: Lion2)
     *  or sourceDevice (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon).
     *  The portListBmp bits in rule's mask and pattern
     *  should be set to 0 (don't care) for matched ports.
     *  The portListBmp bits in rule's mask should be 1 and
     *  pattern should be set to 0 for not matched ports.
     *  Applying a rule for a packet received from the CPU requires
     *  the portListBmp pattern with all bits equal to zero. Hence
     *  rules cannot be shared between the CPU port and other ports
     *  in port-list mode.
     */
    CPSS_PORTS_BMP_STC portListBmp;

    /** @brief Flag indicating the packet Tag state
     *  For Ingress PCL Keys:
     *  For non-tunnel terminated packets:
     *  - If packet was received on DSA-tagged, this field is set
     *  to the DSA tag <SrcTagged> field.
     *  - If packet was received non-DSA-tagged, this field is set
     *  to 1 if the packet was received VLAN or Priority-tagged.
     *  For Ethernet-Over-xxx tunnel-terminated packets: Passenger
     *  packet VLAN tag format.
     *  For Egress PCL Keys:
     *  For non Tunnel Start packets: Transmitted packet's VLAN tag
     *  format after tag addition/removal/modification
     *  For Ethernet-Over-xxx Tunnel Start packets:
     *  Passenger packet's VLAN tag format after tag
     *  addition/removal/modification.
     *  0 = Packet is untagged.
     *  1 = Packet is tagged.
     */
    GT_U8 isTagged;

    /** @brief VLAN ID assigned to the packet.
     *  For DxCh, DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2
     *  VLAN Id is 12 bit.
     *  For Bobcat2; Caelum; Bobcat3; Aldrin devices support of 13-bit VLAN Id.
     *  Standard keys contain bits 11:0 only.
     *  Some keys contains all bits.
     */
    GT_U32 vid;

    /** The packet's 802.1p User Priority field. */
    GT_U32 up;

    /** @brief The QoS Profile assigned to the packet until this lookup
     *  stage, as assigned by the device,
     *  according to the QoS marking algorithm
     */
    GT_U32 qosProfile;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     *  Supported by DxCh2 and above.
     *  DxCh1 devices ignores the field.
     */
    GT_U8 isL2Valid;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     *  Comments:
     */
    GT_U8 isUdbValid;

} CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC
 *
 * @brief This structure describes the common segment of all extended key formats.
*/
typedef struct{

    /** @brief An indication that the packet is IPv6
     *  (if packet was identified as IP packet):
     *  0 = Non IPv6 packet.
     *  1 = IPv6 packet.
     *  An indication that the packet is FCoE
     *  (if packet was not identified as IP packet,
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon):
     *  0 = Non FCoE packet.
     *  1 = FCoE packet.
     */
    GT_U8 isIpv6;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    /** The DSCP field of the IPv4/6 header. */
    GT_U32 dscp;

    /** @brief Layer 4 information is valid.
     *  This field indicates that all the Layer 4
     *  information required for the search key is available
     *  and the IP header is valid.
     *  0 = Layer 4 information is not valid.
     *  1 = Layer 4 information is valid.
     *  Layer 4 information may not be available for any of the
     *  following reasons:
     *  - Layer 4 information is not included in the packet.
     *  For example, Layer 4 information
     *  isn't available in non-IP packets, or in IPv4
     *  non-initial-fragments.
     *  - Parsing failure: Layer 4 information is beyond
     *  the first 128B of the packet, or beyond
     *  IPv6 extension headers parsing capabilities.
     *  - IP header is invalid.
     */
    GT_U8 isL4Valid;

    /** @brief The following Layer 4 information is available for
     *  UDP and TCP packets - L4 Header Byte0 through Byte3,
     *  which contain the UDP/TCP destination and source ports.
     *  For TCP also L4 Header Byte13, which contains the TCP flags.
     *  For IGMP L4 Header Byte0, which contain the IGMP Type.
     *  For ICMP L4 Header Byte0 and Byte1,
     *  which contain the ICMP Type and Code fields
     *  and L4 Header Byte4<in l4Byte2> and Byte5 <in l4Byte3>,
     *  which contain the ICMP authentication field.
     *  The following Layer 4 information is available for packets
     *  that are not TCP, UDP, IGMP or ICMP:
     *  L4 Header Byte0 through Byte3 L4 Header Byte13
     */
    GT_U8 l4Byte0;

    /** see l4Byte0 */
    GT_U8 l4Byte1;

    /** see l4Byte0 */
    GT_U8 l4Byte2;

    /** see l4Byte0 */
    GT_U8 l4Byte3;

    /** see l4Byte0 */
    GT_U8 l4Byte13;

    /** @brief Indicates a valid IP header.
     *  0 = Packet IP header is invalid.
     *  1 = Packet IP header is valid.
     *  Comments:
     */
    GT_U8 ipHeaderOk;

} CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC
 *
 * @brief This structure describes the common segment
 * of all standard IP key formats.
*/
typedef struct{

    /** @brief An indication that the packet is IPv4
     *  (if packet was identified as IP packet):
     *  0 = Non IPv4 packet.
     *  1 = IPv4 packet.
     *  An indication that the packet is FCoE
     *  (if packet was not identified as IP packet,
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon):
     *  0 = Non FCoE packet.
     *  1 = FCoE packet.
     */
    GT_U8 isIpv4;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U32 ipProtocol;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U32 dscp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 isL4Valid;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte2;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte3;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 ipHeaderOk;

    /** @brief Identifies an IPv4 fragment.
     *  0 = Not an IPv4 packet or not an IPv4 fragment.
     *  1 = Packet is an IPv4 fragment (could be
     *  the first fragment or any subsequent fragment)
     *  Comments:
     */
    GT_U8 ipv4Fragmented;

} CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC
 *
 * @brief This structure describes the standard not-IP key.
*/
typedef struct{

    /** the common part for all formats (see above) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** @brief An indication that the packet is IPv4
     *  (if packet was identified as IP packet):
     *  0 = Non IPv4 packet.
     *  1 = IPv4 packet.
     *  An indication that the packet is FCoE
     *  (if packet was not identified as IP packet,
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon):
     *  0 = Non FCoE packet.
     *  1 = FCoE packet.
     */
    GT_U8 isIpv4;

    /** @brief Valid when <IsL2Valid> =1.
     *  If <L2 Encap Type> = 0, this field contains the
     *  <Dsap-Ssap> of the LLC NON-SNAP packet.
     *  If <L2 Encap Type> = 1, this field contains the <EtherType/>
     *  of the Ethernet V2 or LLC with SNAP packet.
     */
    GT_U16 etherType;

    /** @brief An indication that the packet is an ARP packet
     *  (identified by EtherType == 0x0806.)
     *  0 = Non ARP packet.
     *  1 = ARP packet.
     */
    GT_U8 isArp;

    /** @brief The Layer 2 encapsulation of the packet.
     *  For xCat2 and above
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 without LLC.
     *  2 = Reserved.
     *  3 = LLC with SNAP.
     *  For DxCh1, DxCh2, xCat, xCat3, Lion.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    GT_U8 udb15_17[3];

    /** @brief VRF Id (APPLICABLE DEVICES: xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[7:0] muxed with UDB 15
     *  vrfId[11:8] muxed with UDB 16
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash is muxed with UDB 17
     */
    GT_U32 trunkHash;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  muxed with UDB 16
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  bits [11:6] muxed with UDB 16
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  muxed with UDB 16
     */
    GT_U32 cfi1;

    GT_U8 udb23_26[4];

} CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC
 *
 * @brief This structure describes the standard IPV4/V6 L2_QOS key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all standard IP formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC commonStdIp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U8 isArp;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    GT_U8 udb18_19[2];

    /** @brief VRF Id (APPLICABLE DEVICES: xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[7:0] muxed with UDB 18
     *  vrfId[11:8] muxed with UDB 19
     */
    GT_U32 vrfId;

    GT_U8 udb27_30[4];

} CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC
 *
 * @brief This structure describes the standard IPV4_L4 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all standard IP formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC commonStdIp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U8 isArp;

    /** @brief Ethernet Broadcast packet.
     *  Valid when <IsL2Valid> =1.
     *  Indicates an Ethernet Broadcast packet
     *  (<MAC_DA> == FF:FF:FF:FF:FF:FF).
     *  0 = MAC_DA is not Broadcast.
     *  1 = MAC_DA is Broadcast.
     */
    GT_U8 isBc;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte0;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte1;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte13;

    GT_U8 udb20_22[3];

    /** @brief VRF Id (APPLICABLE DEVICES: xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[7:0] muxed with UDB 20
     *  vrfId[11:8] muxed with UDB 21
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 22
     */
    GT_U32 trunkHash;

    GT_U8 udb31_34[4];

} CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC
 *
 * @brief This structure describes the standard IPV6 DIP key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all standard IP formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC commonStdIp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U8 isArp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6ExtHdrExist;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6HopByHop;

    /** 16 bytes IPV6 destination address. */
    GT_IPV6ADDR dip;

    GT_U8 udb47_49[3];

    /** @brief User Defined Byte 0
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  Comments:
     *  Not supported by DxCh2 and DxCh3
     *  Used for Unicast IPV6 routing, lookup1 only
     */
    GT_U8 udb0;

} CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC
 *
 * @brief This structure describes the extended not-IPV6 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC commonExt;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 header destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U16 etherType;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U32 l2Encap;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC */
    GT_U8 ipv4Fragmented;

    GT_U8 udb0_5[6];

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief VRF Id (APPLICABLE DEVICES: xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[11:8] muxed with UDB 1
     *  vrfId[7:0] muxed with UDB 5
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 3
     */
    GT_U32 trunkHash;

    GT_U8 udb39_46[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC
 *
 * @brief This structure describes the extended IPV6+L2 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field. */
    GT_IPV6ADDR sip;

    /** IPV6 destination address highest 8 bits. */
    GT_U8 dipBits127to120;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6ExtHdrExist;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6HopByHop;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    GT_U8 udb6_11[6];

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief VRF Id (APPLICABLE DEVICES: xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[7:0] muxed with UDB 11
     *  vrfId[11:8] muxed with UDB 6
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 7
     */
    GT_U32 trunkHash;

    GT_U8 udb47_49[3];

    GT_U8 udb0_4[5];

} CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC
 *
 * @brief This structure describes the extended IPV6+L4 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field. */
    GT_IPV6ADDR sip;

    /** IPv6 destination IP address field. */
    GT_IPV6ADDR dip;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6ExtHdrExist;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC */
    GT_U8 isIpv6HopByHop;

    GT_U8 udb12_14[3];

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief VRF Id (APPLICABLE DEVICES: xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[7:0] muxed with UDB 14
     *  vrfId[11:8] muxed with UDB 12
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 13
     */
    GT_U32 trunkHash;

    GT_U8 udb15_22[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC
 *
 * @brief PCL Rule Key fields common to ingress "UDB" key formats.
 * xCat and above devices only
*/
typedef struct{

    /** @brief The PCL
     *  Only 2 MSBs of pclId are used when portListBmp is used
     *  (see notes below about portListBmp).
     */
    GT_U32 pclId;

    /** @brief MAC To Me
     *  1 - packet matched by MAC-To-Me lookup, 0 - not matched
     *  The field relevant only for DxCh3 and above devices.
     *  DxCh1 and DxCh2 ignore the field
     */
    GT_U8 macToMe;

    /** @brief The port number from which the packet ingressed the device.
     *  Port 63 is the CPU port.
     *  Field muxing description(APPLICABLE DEVICES: xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon):
     *  sourcePort is muxed with portListBmp[15:8]
     */
    GT_PHYSICAL_PORT_NUM sourcePort;

    /** @brief Together with sourcePort indicates the network port at which the packet
     *  was received.
     *  Field muxing description(APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon):
     *  sourceDevice is muxed with portListBmp[27:16].
     */
    GT_U32 sourceDevice;

    /** @brief Port list bitmap. (APPLICABLE DEVICES: xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     *  Relevant only when PCL lookup works in port-list mode.
     *  Field muxing description:
     *  portListBmp[7:0] is muxed with pclId[7:0],
     *  portListBmp[15:8] is muxed with sourcePort[7:0]
     *  portListBmp[27:16] is muxed with sourceTrunkId (APPLICABLE DEVICES: Lion2)
     *  or sourceDevice (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon).
     *  The portListBmp bits in rule's mask and pattern
     *  should be set to 0 (don't care) for matched ports.
     *  The portListBmp bits in rule's mask should be 1 and
     *  pattern should be set to 0 for not matched ports.
     *  Applying a rule for a packet received from the CPU requires
     *  the portListBmp pattern with all bits equal to zero. Hence
     *  rules cannot be shared between the CPU port and other ports
     *  in port-list mode.
     */
    CPSS_PORTS_BMP_STC portListBmp;

    /** VLAN ID assigned to the packet. */
    GT_U32 vid;

    /** The packet's 802.1p User Priority field. */
    GT_U32 up;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** IP DSCP or MPLS EXP */
    GT_U32 dscpOrExp;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     */
    GT_U8 isL2Valid;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    /** @brief The packet's VLAN Tag format . Valid when <IsL2Valid> = 1.
     *  0 = Untagged
     *  1 = Reserved.
     *  2 = PriorityTagged
     *  3 = IsTagged
     */
    GT_U32 pktTagging;

    /** @brief This flag indicates that the Layer 3 offset was
     *  successfully found. 0=Valid; 1=Invalid
     */
    GT_U8 l3OffsetInvalid;

    /** @brief 0=Other/Ivalid;
     *  1=TCP;
     *  2=ICMP;
     *  3=UDP;
     */
    GT_U32 l4ProtocolType;

    /** @brief 1
     *  2- IPv4 UDP
     *  4- MPLS
     *  8- IPv4 Fragment
     *  16 IPv4 Other
     *  32- Ethernet Other
     *  64- IPv6
     *  128- UDE
     */
    GT_U32 pktType;

    /** 0 */
    GT_U8 ipHeaderOk;

    /** @brief 0=Unicast; Known and Unknown Unicast
     *  1=Multicast; Known and Unknown Multicast
     *  2=NonARP BC; Not ARP Broadcast packet
     *  3=ARP BC; ARP Broadcast packet
     */
    GT_U32 macDaType;

    /** @brief This flag indicates that the Layer 4 offset was
     *  successfully found. 0=Valid; 1=Invalid
     */
    GT_U8 l4OffsetInvalid;

    /** @brief 0=LLC not Snap;
     *  1=Ethernet V2;
     *  2=reserved
     *  3=LLC Snap
     */
    GT_U32 l2Encapsulation;

    /** @brief Indicates that an IPv6 extension exists
     *  0=NotExists; Non-IPv6 packet or IPv6 extension header
     *  does not exists.
     *  1=Exists; Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6Eh;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is hop-by-hop 0=NonHopByHop; Non-IPv6 packet or
     *  IPv6 extension header type is not Hop-by-Hop Option Header.
     *  1=HopByHop; Packet is IPv6 and extension header type is
     *  Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC
 *
 * @brief PCL Rule ingress standard "UDB" Key fields.
 * xCat and above devices only
*/
typedef struct{

    /** fields common for ingress "UDB" styled keys */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC commonIngrUdb;

    /** @brief An indication that the packet is IPv4
     *  (if packet was identified as IP packet):
     *  0 = Non IPv4 packet.
     *  1 = IPv4 packet.
     *  An indication that the packet is FCoE
     *  (if packet was not identified as IP packet,
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon):
     *  0 = Non FCoE packet.
     *  1 = FCoE packet.
     */
    GT_U8 isIpv4;

    GT_U8 udb0_15[16];

    /** @brief VRF Id (APPLICABLE DEVICES: xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[7:0] muxed with UDB 0
     *  vrfId[11:8] muxed with UDB 1
     */
    GT_U32 vrfId;

    /** @brief QoS Profile(APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  qosProfile muxed with UDB2
     */
    GT_U32 qosProfile;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 3
     */
    GT_U32 trunkHash;

    GT_U8 udb35_38[4];

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC
 *
 * @brief PCL Rule ingress extended "UDB" Key fields.
 * xCat and above devices only
*/
typedef struct{

    /** fields common for ingress "UDB" styled keys */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC commonIngrUdb;

    /** @brief An indication that the packet is IPv6
     *  (if packet was identified as IP packet):
     *  0 = Non IPv6 packet.
     *  1 = IPv6 packet.
     *  An indication that the packet is FCoE
     *  (if packet was not identified as IP packet,
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon):
     *  0 = Non FCoE packet.
     *  1 = FCoE packet.
     */
    GT_U8 isIpv6;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    GT_U8 sipBits31_0[4];

    GT_U8 sipBits79_32orMacSa[6];

    GT_U8 sipBits127_80orMacDa[6];

    GT_U8 dipBits127_112[2];

    GT_U8 dipBits31_0[4];

    GT_U8 udb0_15[16];

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief VRF Id (APPLICABLE DEVICES: xCat, xCat3, Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[7:0] muxed with UDB 1
     *  vrfId[11:8] muxed with UDB 2
     */
    GT_U32 vrfId;

    /** @brief QoS Profile(APPLICABLE DEVICES: xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  qosProfile muxed with UDB5
     */
    GT_U32 qosProfile;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 3
     */
    GT_U32 trunkHash;

    GT_U8 udb23_30[8];

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC;

/* EGRESS KEY FORMATS */

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC
 *
 * @brief This structure describes the common segment of all egress key formats.
*/
typedef struct
{
    GT_U8                 valid;
    GT_U32                pclId;
    GT_PHYSICAL_PORT_NUM  sourcePort;
    CPSS_PORTS_BMP_STC    portListBmp;
    GT_U8                 isTagged;
    GT_U32                vid;
    GT_U32                up;
    GT_U8                 isIp;
    GT_U8                 isL2Valid;
    GT_U32                egrPacketType;
    struct
    {
        GT_U32            cpuCode;
        GT_U8             srcTrg;
    } toCpu;
    struct
    {
        GT_U32            tc;
        GT_U32            dp;
        GT_U8             egrFilterEnable;
    } fromCpu;
    struct
    {
        GT_U8             rxSniff;
    } toAnalyzer;
    struct
    {
        GT_U32            qosProfile;
        GT_TRUNK_ID       srcTrunkId;
        GT_U8             srcIsTrunk;
        GT_U8             isUnknown;
        GT_U8             isRouted;
    } fwdData;
    GT_HW_DEV_NUM         srcHwDev;
    GT_U32                sourceId;
    GT_U8                 isVidx;
    GT_U8                 tag1Exist;
    GT_U8                 isUdbValid;
} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC
 *
 * @brief This structure describes the common segment of all extended
 * egress key formats.
*/
typedef struct{

    /** @brief An indication that the packet is IPv6.
     *  0 = Non IPv6 packet.
     *  1 = IPv6 packet.
     */
    GT_U8 isIpv6;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    /** The DSCP field of the IPv4/6 header. */
    GT_U32 dscp;

    /** @brief Layer 4 information is valid.
     *  This field indicates that all the Layer 4
     *  information required for the search key is available
     *  and the IP header is valid.
     *  0 = Layer 4 information is not valid.
     *  1 = Layer 4 information is valid.
     *  Layer 4 information may not be available for any of the
     *  following reasons:
     *  - Layer 4 information is not included in the packet.
     *  For example, Layer 4 information
     *  isn't available in non-IP packets, or in IPv4
     *  non-initial-fragments.
     *  - Parsing failure: Layer 4 information is beyond
     *  the first 128B of the packet, or beyond
     *  IPv6 extension headers parsing capabilities.
     *  - IP header is invalid.
     */
    GT_U8 isL4Valid;

    /** @brief The following Layer 4 information is available for
     *  UDP and TCP packets - L4 Header Byte0 through Byte3,
     *  which contain the UDP/TCP destination and source ports.
     *  For TCP also L4 Header Byte13, which contains the TCP flags.
     *  For IGMP L4 Header Byte0, which contain the IGMP Type.
     *  For ICMP L4 Header Byte0 and Byte1,
     *  which contain the ICMP Type and Code fields
     *  and L4 Header Byte4<in l4Byte2> and Byte5 <in l4Byte3>,
     *  which contain the ICMP authentication field.
     *  The following Layer 4 information is available for packets
     *  that are not TCP, UDP, IGMP or ICMP:
     *  L4 Header Byte0 through Byte3 L4 Header Byte13
     */
    GT_U8 l4Byte0;

    /** see l4Byte0 */
    GT_U8 l4Byte1;

    /** see l4Byte0 */
    GT_U8 l4Byte2;

    /** see l4Byte0 */
    GT_U8 l4Byte3;

    /** see l4Byte0 */
    GT_U8 l4Byte13;

    /** @brief For non Tunnel Start packets:
     *  The transmitted packet TCP/UDP comparator result
     *  For Tunnel Start packets:
     *  The transmitted passenger packet TCP/UDP comparator result
     */
    GT_U32 egrTcpUdpPortComparator;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  Comments:
     */
    GT_U8 isUdbValid;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC
 *
 * @brief This structure describes the common segment
 * of all egress standard IP key formats.
*/
typedef struct{

    /** @brief An indication that the packet is IPv4.
     *  0 = Non IPv4 packet.
     *  1 = IPv4 packet.
     */
    GT_U8 isIpv4;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U32 ipProtocol;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U32 dscp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 isL4Valid;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte2;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte3;

    /** @brief Identifies an IPv4 fragment.
     *  0 = Not an IPv4 packet or not an IPv4 fragment.
     *  1 = Packet is an IPv4 fragment (could be
     *  the first fragment or any subsequent fragment)
     */
    GT_U8 ipv4Fragmented;

    /** @brief For non Tunnel Start packets:
     *  The transmitted packet TCP/UDP comparator result
     *  For Tunnel Start packets:
     *  The transmitted passenger packet TCP/UDP comparator result
     *  Comments:
     */
    GT_U32 egrTcpUdpPortComparator;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC;


/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC
 *
 * @brief This structure describes the standard egress not-IP key.
*/
typedef struct{

    /** the common part for all formats (see above) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** @brief An indication that the packet is IPv4.
     *  0 = Non IPv4 packet.
     *  1 = IPv4 packet.
     */
    GT_U8 isIpv4;

    /** @brief Valid when <IsL2Valid> =1.
     *  If <L2 Encap Type> = 0, this field contains the
     *  <Dsap-Ssap> of the LLC NON-SNAP packet.
     *  If <L2 Encap Type> = 1, this field contains the <EtherType/>
     *  of the Ethernet V2 or LLC with SNAP packet.
     */
    GT_U16 etherType;

    /** @brief An indication that the packet is an ARP packet
     *  (identified by EtherType == 0x0806.)
     *  0 = Non ARP packet.
     *  1 = ARP packet.
     */
    GT_U8 isArp;

    /** @brief The Layer 2 encapsulation of the packet.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    GT_U8 udb0_3[4];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC
 *
 * @brief This structure describes the standard egress IPV4/V6 L2_QOS key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** the common part for all standard IP formats */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC commonStdIp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U8 isArp;

    GT_U8 dipBits0to31[4];

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC */
    GT_U8 l4Byte13;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    GT_U8 udb4_7[4];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC
 *
 * @brief This structure describes the standard egress IPV4_L4 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** the common part for all standard IP formats */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC commonStdIp;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U8 isArp;

    /** @brief Ethernet Broadcast packet.
     *  Valid when <IsL2Valid> =1.
     *  Indicates an Ethernet Broadcast packet
     *  (<MAC_DA> == FF:FF:FF:FF:FF:FF).
     *  0 = MAC_DA is not Broadcast.
     *  1 = MAC_DA is Broadcast.
     */
    GT_U8 isBc;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte0;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte1;

    /** see l4Byte0 in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte13;

    GT_U8 udb8_11[4];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC
 *
 * @brief This structure describes the egress extended not-IPV6 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC commonExt;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 header destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC */
    GT_U16 etherType;

    /** @brief The Layer 2 encapsulation of the packet.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC */
    GT_U8 ipv4Fragmented;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  1 - MPLS, 0 - not MPLS.
     */
    GT_U8 isMpls;

    /** @brief (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  number of MPLS Labels (APPLICABLE RANGES: 0..3).
     */
    GT_U32 numOfMplsLabels;

    /** @brief (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  inner protocol type (2-bit value)
     */
    GT_U32 protocolTypeAfterMpls;

    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X) MPLS Label0. */
    GT_U32 mplsLabel0;

    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X) MPLS Exp0. */
    GT_U32 mplsExp0;

    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X) MPLS Label1. */
    GT_U32 mplsLabel1;

    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X) MPLS Exp1. */
    GT_U32 mplsExp1;

    GT_U8 udb12_19[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC
 *
 * @brief This structure describes the egress extended IPV6+L2 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field. */
    GT_IPV6ADDR sip;

    /** IPV6 destination address highest 8 bits. */
    GT_U8 dipBits127to120;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 isIpv6HopByHop;

    GT_U8 udb20_27[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC
 *
 * @brief This structure describes the egress extended IPV6+L4 key.
*/
typedef struct{

    /** the common part for all formats (see upper) */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC common;

    /** the common part for all extended formats */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field. */
    GT_IPV6ADDR sip;

    /** IPv6 destination IP address field. */
    GT_IPV6ADDR dip;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 isIpv6HopByHop;

    GT_U8 udb28_35[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC
 *
 * @brief This structure describes the common fields of ingress keys.
 * The fields placed at the same bits of the keys.
*/
typedef struct{

    /** @brief The PCL
     *  Only 2 MSBs of pclId are used when portListBmp is used
     *  (see notes below about portListBmp).
     */
    GT_U32 pclId;

    /** @brief MAC To Me
     *  1 - packet matched by MAC-To-Me lookup, 0 - not matched
     *  The field relevant only for DxCh3 and above devices.
     *  DxCh1 and DxCh2 ignore the field
     */
    GT_U8 macToMe;

    /** @brief The port number from which the packet ingressed the device.
     *  Port 63 is the CPU port.
     *  Field muxing description(APPLICABLE DEVICES: xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon):
     *  sourcePort is muxed with portListBmp[15:8]
     */
    GT_PHYSICAL_PORT_NUM sourcePort;

    /** @brief Together with sourcePort indicates the network port at which the packet
     *  was received.
     *  Field muxing description(APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon):
     *  sourceDevice is muxed with portListBmp[27:16].
     */
    GT_U32 sourceDevice;

    /** @brief Port list bitmap. (APPLICABLE DEVICES: xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     *  Relevant only when PCL lookup works in port-list mode.
     *  Field muxing description:
     *  portListBmp[7:0] is muxed with pclId[7:0],
     *  portListBmp[15:8] is muxed with sourcePort[7:0]
     *  portListBmp[27:16] is muxed with sourceTrunkId (APPLICABLE DEVICES: Lion2)
     *  or sourceDevice (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon).
     *  The portListBmp bits in rule's mask and pattern
     *  should be set to 0 (don't care) for matched ports.
     *  The portListBmp bits in rule's mask should be 1 and
     *  pattern should be set to 0 for not matched ports.
     *  Applying a rule for a packet received from the CPU requires
     *  the portListBmp pattern with all bits equal to zero. Hence
     *  rules cannot be shared between the CPU port and other ports
     *  in port-list mode.
     */
    CPSS_PORTS_BMP_STC portListBmp;

    /** @brief Flag indicating the packet Tag state
     *  For Ingress PCL Keys:
     *  For non-tunnel terminated packets:
     *  - If packet was received on DSA-tagged, this field is set
     *  to the DSA tag <SrcTagged> field.
     *  - If packet was received non-DSA-tagged, this field is set
     *  to 1 if the packet was received VLAN or Priority-tagged.
     *  For Ethernet-Over-xxx tunnel-terminated packets: Passenger
     *  packet VLAN tag format.
     */
    GT_U8 isTagged;

    /** VLAN ID assigned to the packet. */
    GT_U32 vid;

    /** The packet's 802.1p User Priority field. */
    GT_U32 up;

    /** 8 */
    GT_U32 tos;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 isL4Valid;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte0;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte1;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte2;

    /** see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte3;

    /** @brief see in CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC
     *  Comments:
     */
    GT_U8 l4Byte13;

} CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC
 *
 * @brief This structure describes the ingress Standard Routed Qos IPV4 Key.
 * CH3 : ingress standard IPV4 key format, used in first lookup.
*/
typedef struct{

    /** The common fields of ingress IP keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief packet type
     *  0 = Non of the following.
     *  1 = ARP.
     *  2 = IPv6 Over MPLS.
     *  3 = IPv4 Over MPLS.
     *  4 = MPLS.
     *  6 = IPv6.
     *  7 = IPv4.
     */
    GT_U32 pktType;

    /** @brief IP Fragmented
     *  0 = no fragment
     *  1 = first fragment
     *  2 = mid fragment
     *  3 = last fragment
     */
    GT_U8 ipFragmented;

    /** @brief IP Header information
     *  0 = Normal
     *  1 = IP options present
     *  2 = IP header validation fail
     *  3 = Is small offset
     */
    GT_U32 ipHeaderInfo;

    /** IP Packet length 14 */
    GT_U32 ipPacketLength;

    /** time to live */
    GT_U32 ttl;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[7:0] muxed with UDB 5
     */
    GT_U32 vrfId;

    /** @brief The transmitted packet TCP/UDP
     *  comparator result.
     *  Supported only for DxCh3 device and ignored for other.
     */
    GT_U32 tcpUdpPortComparators;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    /** @brief UDB5
     *  Supported only for DxChXCat and above devices
     *  and ignored for DxCh3 devices.
     */
    GT_U8 udb5;

    GT_U8 udb41_44[4];

} CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC
 *
 * @brief This structure describes the ingress Extended Port/VLAN Qos IPV4 Key.
 * CH3 : ingress extended IPV4 key format, used in second lookup.
*/
typedef struct{

    /** The common fields of ingress IP keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     */
    GT_U8 isL2Valid;

    /** @brief Ethernet Broadcast packet.
     *  Valid when <IsL2Valid> =1.
     *  Indicates an Ethernet Broadcast packet
     *  (<MAC_DA> == FF:FF:FF:FF:FF:FF).
     *  0 = MAC_DA is not Broadcast.
     *  1 = MAC_DA is Broadcast.
     */
    GT_U8 isBc;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** @brief An indication that the packet is an ARP packet
     *  (identified by EtherType == 0x0806.)
     *  0 = Non ARP packet.
     *  1 = ARP packet.
     */
    GT_U8 isArp;

    /** @brief The Layer 2 encapsulation of the packet.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** @brief Valid when <IsL2Valid> =1.
     *  If <L2 Encap Type> = 0, this field contains the
     *  <Dsap-Ssap> of the LLC NON-SNAP packet.
     *  If <L2 Encap Type> = 1, this field contains the <EtherType/>
     *  of the Ethernet V2 or LLC with SNAP packet.
     */
    GT_U16 etherType;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief IP Fragmented
     *  0 = no fragment
     *  1 = first fragment
     *  2 = mid fragment
     *  3 = last fragment
     */
    GT_U8 ipFragmented;

    /** @brief IP Header information
     *  0 = Normal
     *  1 = IP options present
     *  2 = IP header validation fail
     *  3 = Is small offset
     */
    GT_U32 ipHeaderInfo;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[11:8] muxed with UDB 14
     *  vrfId[7:0] muxed with UDB 13
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 15
     */
    GT_U32 trunkHash;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    GT_U8 UdbStdIpL2Qos[2];

    GT_U8 UdbStdIpV4L4[3];

    GT_U8 UdbExtIpv6L2[4];

    GT_U8 UdbExtIpv6L4[3];

    GT_U8 udb5_16[12];

    GT_U8 udb31_38[8];

} CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC
 *
 * @brief This structure describes the ingress Ultra Port/VLAN Qos IPV6 Key.
 * CH3: ingress ultra IPV6 L2 key format, used in first lookup.
*/
typedef struct{

    /** The common fields of ingress IP keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     */
    GT_U8 isL2Valid;

    /** IS ND */
    GT_U8 isNd;

    /** Ethernet Broadcast packet. */
    GT_U8 isBc;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** @brief The Layer 2 encapsulation of the packet.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** @brief Valid when <IsL2Valid> =1.
     *  If <L2 Encap Type> = 0, this field contains the
     *  <Dsap-Ssap> of the LLC NON-SNAP packet.
     *  If <L2 Encap Type> = 1, this field contains the <EtherType/>
     *  of the Ethernet V2 or LLC with SNAP packet.
     */
    GT_U16 etherType;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief IPv6 source IP address field.
     *  For ARP packets this field holds the sender's IPv6 address.
     */
    GT_IPV6ADDR sip;

    /** @brief IPv6 destination IP address field.
     *  For ARP packets this field holds the target IPv6 address.
     */
    GT_IPV6ADDR dip;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

    /** @brief Indicates a valid IP header.
     *  0 = Packet IP header is invalid.
     *  1 = Packet IP header is valid.
     */
    GT_U8 ipHeaderOk;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[11:8] muxed with UDB 0
     *  vrfId[7:0] muxed with UDB 12
     */
    GT_U32 vrfId;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    GT_U8 UdbExtNotIpv6[4];

    GT_U8 UdbExtIpv6L2[5];

    GT_U8 UdbExtIpv6L4[3];

    GT_U8 udb0_11[12];

    /** UDB 12 (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon) */
    GT_U8 udb12;

    GT_U8 udb39_40[2];

    /** @brief (The source port or trunk assigned to the packet.
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
     */
    GT_U32 srcPortOrTrunk;

    /** @brief Indicates packets received from a Trunk on a
     *  remote device or on the local device
     *  0 = Source is not a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is not a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD
     *  and DSA<SrcIsTrunk> =0
     *  1 = Source is a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD and
     *  DSA<SrcIsTrunk =1
     *  Relevant only when packet is of type is FORWARD.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 srcIsTrunk;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 1
     *  Comments:
     */
    GT_U32 trunkHash;

} CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC
 *
 * @brief This structure describes the ingress Ultra Routed Qos IPV6 Key.
 * CH3 : ingress ultra IPV6 MPLS key format, used in second lookup.
*/
typedef struct{

    /** The common fields of all ingress keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief packet type
     *  0 = Non of the following.
     *  1 = ARP.
     *  2 = IPv6 Over MPLS.
     *  3 = IPv4 Over MPLS.
     *  4 = MPLS.
     *  6 = IPv6.
     *  7 = IPv4.
     */
    GT_U32 pktType;

    /** IS ND */
    GT_U8 isNd;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** MPLS Outer Label */
    GT_U32 mplsOuterLabel;

    /** MPLS Outer Label EXP */
    GT_U32 mplsOuterLabExp;

    /** MPLS Outer Label S */
    GT_U8 mplsOuterLabSBit;

    /** IP Packet Length */
    GT_U32 ipPacketLength;

    /** IPv6 Header <Flow Label> */
    GT_U32 ipv6HdrFlowLabel;

    /** time to live */
    GT_U32 ttl;

    /** @brief IPv6 source IP address field.
     *  For ARP packets this field holds the sender's IPv6 address.
     */
    GT_IPV6ADDR sip;

    /** @brief IPv6 destination IP address field.
     *  For ARP packets this field holds the target IPv6 address.
     */
    GT_IPV6ADDR dip;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

    /** Is IPv6 Link local */
    GT_U8 isIpv6LinkLocal;

    /** Is IPv6 MLD */
    GT_U8 isIpv6Mld;

    /** Indicates a valid IP header. */
    GT_U8 ipHeaderOk;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[11:8] muxed with UDB 19
     *  vrfId[7:0] muxed with UDB 18
     */
    GT_U32 vrfId;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    GT_U8 UdbStdNotIp[2];

    GT_U8 UdbStdIpL2Qos[3];

    GT_U8 UdbStdIpV4L4[3];

    GT_U8 UdbExtNotIpv6[4];

    GT_U8 UdbExtIpv6L2[3];

    GT_U8 UdbExtIpv6L4[3];

    GT_U8 udb0_11[12];

    GT_U8 udb17_22[6];

    GT_U8 udb45_46[2];

    /** @brief (The source port or trunk assigned to the packet.
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
     */
    GT_U32 srcPortOrTrunk;

    /** @brief Indicates packets received from a Trunk on a
     *  remote device or on the local device
     *  0 = Source is not a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is not a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD
     *  and DSA<SrcIsTrunk> =0
     *  1 = Source is a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD and
     *  DSA<SrcIsTrunk =1
     *  Relevant only when packet is of type is FORWARD.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 srcIsTrunk;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 1
     *  Comments:
     */
    GT_U32 trunkHash;

} CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC
 *
 * @brief This structure describes the common part of egress IP keys.
 * The fields placed at the same bits of the keys.
*/
typedef struct{

    /** The PCL */
    GT_U32 pclId;

    /** @brief The port number from which the packet ingressed the device.
     *  Port 63 is the CPU port.
     */
    GT_PHYSICAL_PORT_NUM sourcePort;

    /** The QoS Profile assigned to the packet until this lookup */
    GT_U32 qosProfile;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     */
    GT_U8 isL2Valid;

    /** @brief original VLAN ID assigned to the packet.
     *  For DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2
     *  VLAN Id is 12 bit.
     *  For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X devices support of 16-bit VLAN Id.
     */
    GT_U32 originalVid;

    /** @brief Is Source information (srcDevOrTrunkId) is Trunk ID
     *  0 = srcDevOrTrunkId is source device number
     *  1 = srcDevOrTrunkId is trunk ID
     */
    GT_U8 isSrcTrunk;

    /** @brief Source device number or trunk ID. See isSrcTrunk.
     *  For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X devices used only for TrunkId,
     *  for srcHwDev value see separate field srcDev.
     */
    GT_U32 srcDevOrTrunkId;

    /** @brief Together with <SrcPort> and <srcHwDev> indicates the
     *  network port at which the packet was received.
     *  Relevant for all packet types exclude the
     *  FWD_DATA packed entered to the PP from Trunk
     *  Fields srcTrunkId and srcHwDev are muxed.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_HW_DEV_NUM srcHwDev;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** @brief An indication that the packet is an ARP packet
     *  (identified by EtherType == 0x0806.)
     *  0 = Non ARP packet.
     *  1 = ARP packet.
     */
    GT_U8 isArp;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 tag1Exist;

    /** @brief The Source ID assigned to the packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 sourceId;

    /** 8 */
    GT_U32 tos;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    /** time to live */
    GT_U32 ttl;

    /** @brief Layer 4 information is valid.
     *  This field indicates that all the Layer 4
     *  information required for the search key is available
     *  and the IP header is valid.
     *  0 = Layer 4 information is not valid.
     *  1 = Layer 4 information is valid.
     *  Layer 4 information may not be available for any of the
     *  following reasons:
     *  - Layer 4 information is not included in the packet.
     *  For example, Layer 4 information
     *  isn't available in non-IP packets, or in IPv4
     *  non-initial-fragments.
     *  - Parsing failure: Layer 4 information is beyond
     *  the first 128B of the packet, or beyond
     *  IPv6 extension headers parsing capabilities.
     *  - IP header is invalid.
     */
    GT_U8 isL4Valid;

    /** @brief The following Layer 4 information is available for
     *  UDP and TCP packets - L4 Header Byte0 through Byte3,
     *  which contain the UDP/TCP destination and source ports.
     *  For TCP also L4 Header Byte13, which contains the TCP flags.
     *  For IGMP L4 Header Byte0, which contain the IGMP Type.
     *  For ICMP L4 Header Byte0 and Byte1,
     *  which contain the ICMP Type and Code fields
     *  and L4 Header Byte4<in l4Byte2> and Byte5 <in l4Byte3>,
     *  which contain the ICMP authentication field.
     *  The following Layer 4 information is available for packets
     *  that are not TCP, UDP, IGMP or ICMP:
     *  L4 Header Byte0 through Byte3 L4 Header Byte13
     */
    GT_U8 l4Byte0;

    /** see l4Byte0 */
    GT_U8 l4Byte1;

    /** see l4Byte0 */
    GT_U8 l4Byte2;

    /** see l4Byte0 */
    GT_U8 l4Byte3;

    /** see l4Byte0 */
    GT_U8 l4Byte13;

    /** The transmitted packet TCP/UDP comparator result */
    GT_U32 tcpUdpPortComparators;

    /** traffic class 3 */
    GT_U32 tc;

    /** drop priority 2 */
    GT_U32 dp;

    /** @brief two bits field:
     *  0 - packet to CPU
     *  1 - packet from CPU
     *  2 - packet to ANALYZER
     *  3 - forward DATA packet
     */
    GT_U32 egrPacketType;

    /** @brief Source/Target or Tx Mirror
     *  For TO CPU packets this field is <SrcTrg> of DSA tag.
     *  For TO_TARGET_SNIFFER packets this field is inverted
     *  <RxSniff> of DSA tag.
     *  For FORWARD and FROM_CPU packets this field is 0
     */
    GT_U8 srcTrgOrTxMirror;

    /** 3 */
    GT_U32 assignedUp;

    /** target physical port */
    GT_PHYSICAL_PORT_NUM trgPhysicalPort;

    /** @brief Indicates if this packet is Ingress or Egress
     *  mirrored to the Analyzer.
     *  0 = This packet is Egress mirrored to the analyzer.
     *  1 = This packet is Ingress mirrored to the analyzer.
     *  Relevant only when packet is of type is TO_ANALYZER.
     */
    GT_U8 rxSniff;

    /** @brief Indicates that the packet has been routed either by the
     *  local device or some previous device in a cascaded system.
     *  0 = The packet has not been routed.
     *  1 = The packet has been routed.
     *  Relevant only when packet is of type is FORWARD.
     */
    GT_U8 isRouted;

    /** @brief An indication that the packet is IPv6.
     *  0 = Non IPv6 packet.
     *  1 = IPv6 packet.
     */
    GT_U8 isIpv6;

    /** @brief The CPU Code forwarded to the CPU.
     *  Relevant only when packet is of type is TO_CPU.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cpuCode;

    /** @brief The <SrcTrg> field extracted from the TO_CPU DSA<SrcTrg>:
     *  0 = The packet was sent to the CPU by the ingress pipe and
     *  the DSA tag contain attributes related to the
     *  packet ingress.
     *  1 = The packet was sent to the CPU by the egress pipe
     *  and the DSA tag contains attributes related to the
     *  packet egress.
     *  Relevant only when packet is of type is TO_CPU.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 srcTrg;

    /** @brief The <EgressFilterEn> extracted from
     *  FROM_CPU DSA<EgressFilterEn>:
     *  0 = FROM_CPU packet is subject to egress filtering,
     *  e.g. data traffic from the CPU
     *  1 = FROM_CPU packet is not subject egress filtering,
     *  e.g. control traffic from the CPU
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  Comments:
     */
    GT_U8 egrFilterEnable;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC;


/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC
 *
 * @brief This structure describes the Egress Extended (48B) RACL/VACL IPv4 key format.
 * CH3 : egress extended (48B) IPV4 key format.
*/
typedef struct{

    /** The common fields of egress IP keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC egressIpCommon;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** @brief IP V4 Options
     *  For non-tunnel start packets:
     *  0 = The transmitted packet is non-Ipv4 or an IPv4 options
     *  do not exist.
     *  1 = The transmitted packet is IPv4 and an IPv4 options
     *  exist.
     *  For tunnel-start packets:
     *  - always 0
     */
    GT_U8 ipv4Options;

    /** @brief Indicates that a packet is forwarded to a Multicast group.
     *  0 = The packet is a Unicast packet forwarded to a
     *  specific target port or trunk.
     *  1 = The packet is a multi-destination packet forwarded
     *  to a Multicast group.
     *  Relevant only for DxChXcat and above devices.
     */
    GT_U8 isVidx;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 isUdbValid;

    GT_U8 udb36_49[14];

    /** @brief UDB 0 (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  Comments:
     */
    GT_U8 udb0;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC
 *
 * @brief This structure describes the Egress Ultra (72B) RACL/VACL IPv6 key format.
 * CH3 : egress Ultra (72B) IPV6 key format.
*/
typedef struct{

    /** The common fields of egress IP keys. */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC egressIpCommon;

    /** @brief IPv6 source IP address field.
     *  For ARP packets this field holds the sender's IPv6 address.
     */
    GT_IPV6ADDR sip;

    /** IPv6 destination IP address field. */
    GT_IPV6ADDR dip;

    /** Is ND */
    GT_U8 isNd;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

    /** @brief Indicates that a packet is forwarded to a Multicast group.
     *  0 = The packet is a Unicast packet forwarded to a
     *  specific target port or trunk.
     *  1 = The packet is a multi-destination packet forwarded
     *  to a Multicast group.
     *  Relevant only for DxChXcat and above devices.
     */
    GT_U8 isVidx;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief Source port assign to the packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 srcPort;

    /** @brief Egress port of the packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 trgPort;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 isUdbValid;

    GT_U8 udb1_4[4];

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_REPLACED_STC
 *
 * @brief This structure describes the Ingress UDB only keys fields
 * replacing the user defined bytes and being enabled/disabled
 * by cpssDxChPclUserDefinedBytesSelectSet.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    /** @brief pclId UDB0 muxed with pclId[7:0],
     *  UDB1[1:0] muxed with pclId[9:8]
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 pclId;

    /** @brief UDB Valid: 1 when all UDBs in the key valid, otherwise 0,
     *  muxed with UDB1[7]
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 isUdbValid;

    /** @brief eVlan, UDB2 muxed with vid[7:0],
     *  UDB3 muxed with vid[12:8]
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 vid;

    /** @brief source ePort, UDB4 muxed with srcPort[7:0],
     *  UDB5 muxed with srcPort[12:8]
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 srcPort;

    /** @brief source Device Is Own
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 srcDevIsOwn;

    /** @brief tag1 VID, UDB6 muxed with vid1[7:0],
     *  UDB7[3:0] muxed with vid1[11:8]
     *  (APPLICABLE RANGES: 0..0xFFF)
     */
    GT_U32 vid1;

    /** @brief tag1 UP, UDB7[6:4] muxed with up1[2:0]
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 up1;

    /** @brief mac to me, UDB7[7] muxed with mac2me
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 macToMe;

    /** @brief qosProfile, UDB8 muxed with qosProfile[7:0],
     *  UDB9[1:0] muxed with qosProfile[9:8]
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 qosProfile;

    /** @brief flow Id, UDB10 muxed with flowId[7:0]
     *  UDB11[3:0] muxed with flowId[11:8]
     *  (APPLICABLE RANGES: 0..0xFFF)
     *  Comments:
     */
    GT_U32 flowId;

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_REPLACED_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB60_FIXED_STC
 *
 * @brief This structure describes the Ingress UDB60 keys fixed fields.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    /** @brief UDB Valid
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 isUdbValid;

    /** @brief PCL
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 pclId;

    /** @brief eVlan
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 vid;

    /** @brief source ePort
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 srcPort;

    /** @brief source Device Is Own
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 srcDevIsOwn;

    /** @brief tag1 VID
     *  (APPLICABLE RANGES: 0..0xFFF)
     */
    GT_U32 vid1;

    /** @brief tag1 UP
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 up1;

    /** @brief mac to me
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 macToMe;

    /** @brief qosProfile
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 qosProfile;

    /** @brief flow Id
     *  (APPLICABLE RANGES: 0..0xFFF)
     *  Comments:
     */
    GT_U32 flowId;

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB60_FIXED_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC
 *
 * @brief This structure describes the Ingress UDB only keys formats.
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 * CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E
 * (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    GT_U8 udb[CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS];

    /** replaced fixed fields. */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_REPLACED_STC replacedFld;

    /** @brief fixed fields,
     *  relevant only for CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E keys.
     *  Comments:
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB60_FIXED_STC udb60FixedFld;

} CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_REPLACED_STC
 *
 * @brief This structure describes the Egress UDB only keys fields
 * replacing the user defined bytes and being enabled/disabled
 * by cpssDxChPclUserDefinedBytesSelectSet.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    /** @brief PCL
     *  UDB1[1:0] muxed with PCL-ID[9:8]
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 pclId;

    /** @brief UDB Valid: 1 when all UDBs in the key valid, otherwise 0,
     *  muxed with UDB1[7]
     *  (APPLICABLE RANGES: 0..1)
     *  Comments:
     */
    GT_U8 isUdbValid;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_REPLACED_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB60_FIXED_STC
 *
 * @brief This structure describes the Egress UDB60 keys fixed fields.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    /** @brief PCL
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 pclId;

    /** @brief UDB Valid
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 isUdbValid;

    /** @brief eVlan
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 vid;

    /** @brief source ePort
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 srcPort;

    /** @brief target ePort
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 trgPort;

    /** @brief source Device
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 srcDev;

    /** @brief target Device
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 trgDev;

    /** @brief local device target physical port
     *  (APPLICABLE RANGES: 0..0xFF)
     *  Comments:
     */
    GT_PHYSICAL_PORT_NUM localDevTrgPhyPort;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB60_FIXED_STC;

/**
* @struct CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC
 *
 * @brief This structure describes the Egress UDB only keys formats.
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E
 * CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    GT_U8 udb[CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS];

    /** replaced fixed fields. */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_REPLACED_STC replacedFld;

    /** @brief fixed fields,
     *  relevant only for CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E keys.
     *  Comments:
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB60_FIXED_STC udb60FixedFld;

} CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC;


/**
* @union CPSS_DXCH_PCL_RULE_FORMAT_UNT
 *
 * @brief This union describes the PCL key.
 *
*/

typedef union{
    /** Standard Not IP packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC ruleStdNotIp;

    /** @brief Standard IPV4 and IPV6 packets
     *  L2 and QOS styled key
     */
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC ruleStdIpL2Qos;

    /** Standard IPV4 packet L4 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC ruleStdIpv4L4;

    /** Standard IPV6 packet DIP styled key (DxCh only) */
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC ruleStdIpv6Dip;

    /** Ingress Standard UDB styled packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC ruleIngrStdUdb;

    /** Extended Not IP and IPV4 packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC ruleExtNotIpv6;

    /** Extended IPV6 packet L2 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC ruleExtIpv6L2;

    /** Extended IPV6 packet L2 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC ruleExtIpv6L4;

    /** Ingress Extended UDB styled packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC ruleIngrExtUdb;

    /** Egress Standard Not IP packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC ruleEgrStdNotIp;

    /** @brief Egress Standard IPV4 and IPV6 packets
     *  L2 and QOS styled key
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC ruleEgrStdIpL2Qos;

    /** Egress Standard IPV4 packet L4 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC ruleEgrStdIpv4L4;

    /** Egress Extended Not IP and IPV4 packet key */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC ruleEgrExtNotIpv6;

    /** Egress Extended IPV6 packet L2 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC ruleEgrExtIpv6L2;

    /** Egress Extended IPV6 packet L2 styled key */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC ruleEgrExtIpv6L4;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E */
    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC ruleStdIpv4RoutedAclQos;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E */
    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC ruleExtIpv4PortVlanQos;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E */
    CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC ruleUltraIpv6PortVlanQos;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E */
    CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC ruleUltraIpv6RoutedAclQos;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC ruleEgrExtIpv4RaclVacl;

    /** see CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC ruleEgrUltraIpv6RaclVacl;

    /** @brief Ingress UDB only formats.
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E
     */
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC ruleIngrUdbOnly;

    /** @brief Egress UDB only formats.
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E
     *  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E
     *  Comments:
     */
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC ruleEgrUdbOnly;

} CPSS_DXCH_PCL_RULE_FORMAT_UNT;


/**
* @enum CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT
 *
 * @brief packet types that can be enabled/disabled for Egress PCL.
 * By default on all ports and on all packet types are disabled
*/
typedef enum{

    /** @brief Enable/Disable Egress Policy
     *  for Control packets FROM CPU.
     */
    CPSS_DXCH_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E,

    /** @brief Enable/Disable Egress Policy
     *  for data packets FROM CPU.
     */
    CPSS_DXCH_PCL_EGRESS_PKT_FROM_CPU_DATA_E,

    /** @brief Enable/Disable Egress Policy
     *  on TOCPU packets
     */
    CPSS_DXCH_PCL_EGRESS_PKT_TO_CPU_E,

    /** @brief Enable/Disable Egress Policy
     *  for TO ANALYZER packets.
     */
    CPSS_DXCH_PCL_EGRESS_PKT_TO_ANALYZER_E,

    /** @brief Enable/Disable Egress Policy
     *  for data packets that are tunneled in this device.
     */
    CPSS_DXCH_PCL_EGRESS_PKT_TS_E,

    /** @brief Enable/Disable Egress Policy
     *  for data packets that are not tunneled in this device.
     */
    CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E

} CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT;

/**
* @enum CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_ENT
 *
 * @brief Global Egress PCL or Ingress Policy
 * Configuration Table Access mode
*/
typedef enum{

    /** access by local port number */
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E,

    /** @brief access by remote device Id
     *  and Port number or by trunk Id,
     *  Used for Value Blade systems based on DX switch device.
     *  The remote port and device are taken from DSA tag and
     *  represent real source port and device for a packets.
     */
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_NON_LOCAL_PORT_E

} CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_ENT;


/**
* @enum CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_ENT
 *
 * @brief size of Ingress Policy / Egress PCL Configuration
 * table segment accessed by "non-local device and port"
*/
typedef enum{

    /** support 32 port remote devices */
    CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_32_E,

    /** support 64 port remote devices */
    CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_64_E

} CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_ENT;

/**
* @enum CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE_ENT
 *
 * @brief base of Ingress Policy / Egress PCL Configuration
 * table segment accessed
 * by "non-local device and port"
*/
typedef enum{

    /** the base is 0 */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E,

    /** the base is 1024 (only 32-port) */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE1024_E,

    /** the base is 2048 */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE2048_E,

    /** the base is 3072 (only 32-port) */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE3072_E

} CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE_ENT;

/**
* @enum CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_ENT
 *
 * @brief TCAM segment mode.
 * Defines whether the 4 TCAM segments assigned to the PCL are
 * regarded as one logical TCAM returning a single reply,
 * two separate TCAM segments returning two replies,
 * or four separate TCAM segments returning four replies.
 * The segment size measured in quarters of TCAM.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
 * Note: Bobcat3; Aldrin2 does not support it ! feature was removed.
 * and behavior is as 'CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E'
 * APIs that get this parameter will ignore it.
 * (see PRV_CPSS_DXCH_BOBCAT3_RM_TCAM_SEGMENT_MODE_OBSOLETE_WA_E)
*/
typedef enum{

    /** all TCAM segments are one logical TCAM */
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_1_TCAM_E,

    /** @brief segmentation into 2 lists,
     *  logically distributed 50% and 50%
     */
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_2_AND_2_E,

    /** @brief segmentation into 2 lists,
     *  logically distributed 25% [first] and 75% [second];
     */
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_1_AND_3_E,

    /** @brief segmentation into 2 lists,
     *  logically distributed 75% [first] and 25% [second];
     */
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_2_TCAMS_3_AND_1_E,

    /** segmentation into 4 lists, each - 25% */
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E

} CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_ENT;

/**
* @struct CPSS_DXCH_PCL_LOOKUP_CFG_STC
 *
 * @brief PCL Configuration Table Entry Per Lookup Attributes.
*/
typedef struct
{
    GT_BOOL                                   enableLookup;
    GT_U32                                    pclId;
    GT_BOOL                                   dualLookup;
    GT_U32                                    pclIdL01;
    struct
    {
        CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT    nonIpKey;
        CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT    ipv4Key;
        CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT    ipv6Key;
    }                                         groupKeyTypes;
    GT_BOOL                                   udbKeyBitmapEnable;
    CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_ENT       tcamSegmentMode;
    /** @brief PCL Sub Profile Id for exact match
     *   APPLICABLE DEVICE: Falcon.
     *   APPLICABLE RANGES: 0..7.
     */
    GT_U32                                    exactMatchLookupSubProfileId;

} CPSS_DXCH_PCL_LOOKUP_CFG_STC;

/**
* @struct CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC
 *
 * @brief global access mode to Ingress or Egress
 * PCL configuration tables.
*/
typedef struct{

    /** Ingress Policy local/non */
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_ENT ipclAccMode;

    /** Ingress Policy support 32/64 port remote devices */
    CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_ENT ipclMaxDevPorts;

    /** Ingress Policy Configuration table access base */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE_ENT ipclDevPortBase;

    /** Egress PCL local/non */
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_ENT epclAccMode;

    /** Egress PCL support 32/64 port remote devices */
    CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_ENT epclMaxDevPorts;

    /** @brief Egress PCL Configuration table access base
     *  Comments:
     *  - for DxCh2 and above all fields are relevant
     *  - for DxCh1 relevant only ipclAccMode member
     */
    CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE_ENT epclDevPortBase;

} CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC;

/**
* @enum CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT
 *
 * @brief enumerator for Egress PCL
 * VID and UP key fields content mode
*/
typedef enum{

    /** extract from packet Tag0 */
    CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_FROM_TAG0_E,

    /** extract from packet Tag1 */
    CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_FROM_TAG1_E

} CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_ENT
 *
 * @brief enumerator for OAM packet MEG Level mode
*/
typedef enum{

    /** @brief MEG Level is UDB2[7:5] (OAM profile0)
     *  or UDB3[7:5] (OAM profile1)
     */
    CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_UDB_E,

    /** MEG Level is MAC DA[2:0] */
    CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_MAC_DA_E,

    /** MEG Level is MPLS EXP[2:0] (egress only) */
    CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_EXP_E,

    /** MEG Level is QOS profile[2:0] (ingress only) */
    CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_QOS_E

} CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_ENT;

/**
* @enum CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_ENT
 *
 * @brief enumerator for OAM Packet Detection Mode mode
*/
typedef enum{

    /** @brief OAM packets
     *  are detected based on EPCL rules.
     */
    CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_STANDARD_E,

    /** @brief OAM packets
     *  are detected based on EPCL rule AND a dedicated
     *  Source ID value. This mode allows the CPU to mark
     *  OAM packets by sending them with a dedicated Source ID.
     */
    CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_SOURCE_ID_E

} CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_ENT;

/**
* @internal cpssDxChPclInit function
* @endinternal
*
* @brief   The function initializes the device for following configuration
*         and using Policy engine
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*
* @note NONE
*
*/
GT_STATUS cpssDxChPclInit
(
    IN GT_U8                           devNum
);

/**
* @internal cpssDxChPclUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
*                                      Relevant for DxCh1, DxCh2, DxCh3 devices
* @param[in] packetType               - packet Type
*                                      Used for Ingress Only
*                                      (APPLICABLE DEVICES xCat; xCat3; Lion; xCat2; Lion2.)
*                                      Used for Ingress and Egress
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress.(APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      Ignored, configure ingress.
*                                      (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2)
* @param[in] udbIndex                 - index of User Defined Byte to configure.
*                                      See format of rules to known indexes of UDBs
* @param[in] offset                   - the type of offset (see CPSS_DXCH_PCL_OFFSET_TYPE_ENT)
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
GT_STATUS cpssDxChPclUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN CPSS_PCL_DIRECTION_ENT               direction,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
);

/**
* @internal cpssDxChPclUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
*                                      Relevant for DxCh1, DxCh2, DxCh3 devices
* @param[in] packetType               - packet Type
*                                      Used for Ingress Only
*                                      (APPLICABLE DEVICES xCat; xCat3; Lion; xCat2; Lion2.)
*                                      Used for Ingress and Egress
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress.(APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      Ignored, configure ingress.
*                                      (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2)
* @param[in] udbIndex                 - index of User Defined Byte to configure.
*                                      See format of rules to known indexes of UDBs
*
* @param[out] offsetTypePtr            - (pointer to) The type of offset
*                                      (see CPSS_DXCH_PCL_OFFSET_TYPE_ENT)
* @param[out] offsetPtr                - (pointer to) The offset of the user-defined byte,
*                                      in bytes,from the place in the packet
*                                      indicated by the offset type.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
GT_STATUS cpssDxChPclUserDefinedByteGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN  CPSS_PCL_DIRECTION_ENT               direction,
    IN  GT_U32                               udbIndex,
    OUT CPSS_DXCH_PCL_OFFSET_TYPE_ENT        *offsetTypePtr,
    OUT GT_U8                                *offsetPtr
);

/**
* @internal cpssDxChPclRuleSet function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - format of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM. The rule index defines
*                                      order of action resolution in the cases of multiple
*                                      rules match with packet's search key. Action of the
*                                      matched rule with lowest index is taken in this case
*                                      With reference to Standard and Extended rules
*                                      indexes, the partitioning is as follows:
*                                      For DxCh and DxCh2 devices:
*                                      - Standard rules.
*                                      Rule index may be in the range from 0 up to 1023.
*                                      - Extended rules.
*                                      Rule index may be in the range from 0 up to 511.
*                                      Extended rule consumes the space of two standard
*                                      rules:
*                                      - Extended rule with index  0 <= n <= 511
*                                      is placed in the space of two standard rules with
*                                      indexes n and n + 512.
*                                      For DxCh3 device:
*                                      The TCAM has up to 3.5K (3584) rows.
*                                      See datasheet of particular device for TCAM size.
*                                      Each row can be used as:
*                                      - 4 standard rules
*                                      - 2 extended rules
*                                      - 1 extended and 2 standard rules
*                                      - 2 standard and 1 extended rules
*                                      The TCAM partitioning is as follows:
*                                      - Standard rules.
*                                      Rule index may be in the range from 0 up to 14335.
*                                      - Extended rules.
*                                      Rule index may be in the range from 0 up to 7167.
*                                      Extended rule consumes the space of two standard
*                                      rules:
*                                      - Extended rule with index  0 <= n < 3584
*                                      is placed in the space of two standard rules with
*                                      indexes n and n + 3584.
*                                      - Extended rule with index  3584 <= n < 7168
*                                      is placed in the space of two standard rules with
*                                      indexes n + 3584, n + 7168.
*                                      For xCat and above devices:
*                                      See datasheet of particular device for TCAM size.
*                                      For example describe the TCAM that has 1/4K (256) rows.
*                                      Each row can be used as:
*                                      - 4 standard rules
*                                      - 2 extended rules
*                                      - 1 extended and 2 standard rules
*                                      - 2 standard and 1 extended rules
*                                      The TCAM partitioning is as follows:
*                                      - Standard rules.
*                                      Rule index may be in the range from 0 up to 1K (1024).
*                                      - Extended rules.
*                                      Rule index may be in the range from 0 up to 0.5K (512).
*                                      Extended rule consumes the space of two standard
*                                      rules:
*                                      - Extended rule with index 2n (even index)
*                                      is placed in the space of two standard rules with
*                                      indexes 4n and 4n + 1.
*                                      - Extended rule with index 2n+1 (odd index)
*                                      is placed in the space of two standard rules with
*                                      indexes 4n+2 and 4n + 3.
* @param[in] ruleOptionsBmp           - Bitmap of rule's options.
*                                      The CPSS_DXCH_PCL_RULE_OPTION_ENT defines meaning of each bit.
*                                      Samples:
* @param[in] ruleOptionsBmp           = 0 - no options are defined.
*                                      Rule state is valid.
* @param[in] ruleOptionsBmp           = CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E -
*                                      write all fields of rule to TCAM but rule's
*                                      state is invalid (no match during lookups).
* @param[in] maskPtr                  - rule mask. The rule mask is AND styled one. Mask
*                                      bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM lookup.
*                                      The format of mask is defined by ruleFormat
* @param[in] patternPtr               - rule pattern.
*                                      The format of pattern is defined by ruleFormat
* @param[in] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The pattern and mask input structures should be set to zero
*       by an application before assignment of fields.
*       The function places all muxed fields to rule patern and mask
*       by using bitwise OR operation.
*       The function does not check validity of muxed (overlapped) fields.
*
*/
GT_STATUS cpssDxChPclRuleSet
(
    IN GT_U8                              devNum,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                             ruleIndex,
    IN CPSS_DXCH_PCL_RULE_OPTION_ENT      ruleOptionsBmp,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT      *patternPtr,
    IN CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclRuleParsedGet function
* @endinternal
*
* @brief   The function gets the Policy Rule valid status, Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - format of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM.
* @param[in] ruleOptionsBmp           - Bitmap of rule's options.
*                                      The CPSS_DXCH_PCL_RULE_OPTION_ENT defines meaning of each bit.
*                                      Samples:
* @param[in] ruleOptionsBmp           = 0 - no options are defined.
*
* @param[out] isRuleValidPtr           - GT_TRUE - rule is valid, GT_FALSE - rule is invalid
* @param[out] maskPtr                  - rule mask. The rule mask is AND styled one. Mask
*                                      bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM lookup.
*                                      The format of mask is defined by ruleFormat
* @param[out] patternPtr               - rule pattern.
*                                      The format of pattern is defined by ruleFormat
* @param[out] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_BAD_STATE             - when size is trash or not related to ruleFormat.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclRuleParsedGet
(
    IN  GT_U8                                            devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT               ruleFormat,
    IN  GT_U32                                           ruleIndex,
    IN  CPSS_DXCH_PCL_RULE_OPTION_ENT                    ruleOptionsBmp,
    OUT GT_BOOL                                          *isRuleValidPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *patternPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC                         *actionPtr
);

/**
* @internal cpssDxChPclRuleActionUpdate function
* @endinternal
*
* @brief   The function updates the Rule Action
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - size of Rule. (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclRuleSet.
* @param[in] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPclRuleActionUpdate
(
    IN GT_U8                              devNum,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex,
    IN CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclRuleActionGet function
* @endinternal
*
* @brief   The function gets the Rule Action
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - size of Rule. (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclRuleSet.
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
*                                      Needed for parsing
*
* @param[out] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPclRuleActionGet
(
    IN  GT_U8                              devNum,
    IN  CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN  GT_U32                             ruleIndex,
    IN CPSS_PCL_DIRECTION_ENT              direction,
    OUT CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclRuleInvalidate function
* @endinternal
*
* @brief   The function invalidates the Policy Rule.
*         For DxCh and DxCh2 devices start indirect rule write operation
*         with "garbage" pattern and mask content and "valid-flag" == 0
*         For DxCh3 devices calculates the TCAM position by ruleSize and ruleIndex
*         parameters and updates the first "valid" bit matching X/Y pair to (1,1)
*         i.e. don't match any value. If origin of valid or invalid rule found at the
*         specified TCAM position it will be invalid and available to back validation.
*         If the specified TCAM position contains the not first 24 byte segment of
*         extended rule, the rule also will be invalidated,
*         but unavailable to back validation.
*         If the garbage found in TCAM the X/Y bits will be updated to be as
*         in invalid rule.
*         The function does not check the TCAM contents.
*         GT_OK will be returned in each of described cases.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - size of Rule.(APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclRuleSet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Backward compatible styled API.
*       The cpssDxChPclRuleValidStatusSet recommended for using instead.
*
*/
GT_STATUS cpssDxChPclRuleInvalidate
(
    IN GT_U8                              devNum,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex
);

/**
* @internal cpssDxChPclRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the Policy rule.
*         The validation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Valid indication set.
*         The function does not check content of the rule before
*         write it back to TCAM
*         The invalidation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Invalid indication set.
*         If the given the rule found already in needed valid state
*         no write done. If the given the rule found with size
*         different from the given rule-size an error code returned.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - size of Rule.(APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM.
* @param[in] valid                    - new rule status: GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if in TCAM found rule of size different
*                                       from the specified
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclRuleValidStatusSet
(
    IN GT_U8                              devNum,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex,
    IN GT_BOOL                            valid
);

/**
* @internal cpssDxChPclRuleCopy function
* @endinternal
*
* @brief   The function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move Policy
*         Rule from old position to new one at first cpssDxChPclRuleCopy should be
*         called. And after this cpssDxChPclRuleInvalidate or
*         cpssDxChPclRuleValidStatusSet should be used
*         to invalidate Rule in old position.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - size of Rule.
* @param[in] ruleSrcIndex             - index of the rule in the TCAM from which pattern,
*                                      mask and action are taken.
* @param[in] ruleDstIndex             - index of the rule in the TCAM to which pattern,
*                                      mask and action are placed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclRuleCopy
(
    IN GT_U8                              devNum,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleSrcIndex,
    IN GT_U32                             ruleDstIndex
);

/**
* @internal cpssDxChPclRuleMatchCounterGet function
* @endinternal
*
* @brief   Get rule matching counters.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] counterIndex             - Counter Index (APPLICABLE RANGES: 0..31)
*
* @param[out] counterPtr               - pointer to the counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclRuleMatchCounterGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        counterIndex,
    OUT GT_U32                        *counterPtr
);

/**
* @internal cpssDxChPclRuleMatchCounterSet function
* @endinternal
*
* @brief   Set rule matching counters.
*         To reset value of counter use counterValue = 0
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] counterIndex             - Counter Index (APPLICABLE RANGES: 0..31)
* @param[in] counterValue             - counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclRuleMatchCounterSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        counterIndex,
    IN  GT_U32                        counterValue
);

/**
* @internal cpssDxChPclPortIngressPolicyEnable function
* @endinternal
*
* @brief   Enables/disables ingress policy per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - Ingress Policy enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortIngressPolicyEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChPclPortIngressPolicyEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable ingress policy status per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - Pointer to ingress policy status.
*                                      GT_TRUE - Ingress Policy is enabled.
*                                      GT_FALSE - Ingress Policy is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortIngressPolicyEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChPclPortLookupCfgTabAccessModeSet function
* @endinternal
*
* @brief   Configures VLAN/PORT/ePort access mode to Ingress or Egress PCL
*         configuration table per lookup.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*                                      In eArch devices
*                                      - for each ingress lookup portNum is default ePort.
*                                      - for egress lookup portNum is physical port.
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      lookup0 or lookup1
* @param[in] subLookupNum             - sub lookup number for lookup.
*                                      Supported only for  xCat and above devices.
*                                      Ignored for other devices.
*                                      xCat and above devices supports two sub lookups only for
*                                      ingress lookup CPSS_PCL_LOOKUP_0_E , (APPLICABLE RANGES: 0..1).
*                                      All other lookups have not sub lookups,
*                                      acceptable value is 0.
* @param[in] mode                     - PCL Configuration Table access mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - on bad entryIndex
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - since dxCh1 supports Ingress direction only, this API set the same access
*       type for both lookups regardless <lookupNum> parameter value for dxCh1.
*
*/
GT_STATUS cpssDxChPclPortLookupCfgTabAccessModeSet
(
    IN GT_U8                                          devNum,
    IN GT_PORT_NUM                                    portNum,
    IN CPSS_PCL_DIRECTION_ENT                         direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT                     lookupNum,
    IN GT_U32                                         subLookupNum,
    IN CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT mode
);

/**
* @internal cpssDxChPclPortLookupCfgTabAccessModeGet function
* @endinternal
*
* @brief   Gets VLAN/PORT/ePort access mode to Ingress or Egress PCL
*         configuration table per lookup.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices
*                                      - for each ingress lookup portNum is default ePort.
*                                      - for egress lookup portNum is physical port.
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      lookup0 or lookup1
* @param[in] subLookupNum             - sub lookup number for lookup.
*                                      Supported only for  xCat and above devices.
*                                      Ignored for other devices.
*                                      xCat and above devices supports two sub lookups only for
*                                      ingress lookup CPSS_PCL_LOOKUP_0_E , (APPLICABLE RANGES: 0..1).
*                                      All other lookups have not sub lookups,
*                                      acceptable value is 0.
*
* @param[out] modePtr                  - (pointer to)PCL Configuration Table access mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - for DxCh2 and above - full support,
*       - for DxCh1 supports Ingress direction only and set same access type
*       for both lookups regardless the <lookupNum> parameter value
*
*/
GT_STATUS cpssDxChPclPortLookupCfgTabAccessModeGet
(
    IN  GT_U8                                          devNum,
    IN  GT_PORT_NUM                                    portNum,
    IN  CPSS_PCL_DIRECTION_ENT                         direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT                     lookupNum,
    IN  GT_U32                                         subLookupNum,
    OUT CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT *modePtr
);

/**
* @internal cpssDxChPclCfgTblAccessModeSet function
* @endinternal
*
* @brief   Configures global access mode to Ingress or Egress PCL configuration tables.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] accModePtr               - global configuration of access mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note - for DxCh2 and above - full support,
*       - for DxCh1 supports Ingress direction only with 32 ports per device and the
*       non-local-device-entries-segment-base == 0
*
*/
GT_STATUS cpssDxChPclCfgTblAccessModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC   *accModePtr
);

/**
* @internal cpssDxChPclCfgTblAccessModeGet function
* @endinternal
*
* @brief   Get global access mode to Ingress or Egress PCL configuration tables.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] accModePtr               - global configuration of access mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note see cpssDxChPclCfgTblAccessModeSet
*
*/
GT_STATUS cpssDxChPclCfgTblAccessModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC   *accModePtr
);

/**
* @internal cpssDxChPclCfgTblSet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfaceInfoPtr         - interface data: port, VLAN, or index for setting a
*                                      specific PCL Configuration Table entry
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      For Ingress PCL: lookup0 or lookup1
*                                      For Egress PCL ignored
* @param[in] lookupCfgPtr             - lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note For DxCh2 and above - full support (beside the Standard IPV6 DIP key)
*       For DxCh1 - ingress only, keys for not-IP, IPV4 and IPV6 must
*       be of the same size, Standard IPV6 DIP key allowed only on lookup1
*       For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X that use INDEX interface type
*       to configure entry for ePort, index = ePort
*
*/
GT_STATUS cpssDxChPclCfgTblSet
(
    IN GT_U8                           devNum,
    IN CPSS_INTERFACE_INFO_STC        *interfaceInfoPtr,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_DXCH_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
);

/**
* @internal cpssDxChPclCfgTblGet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfaceInfoPtr         - interface data: port, VLAN, or index for getting a
*                                      specific PCL Configuration Table entry
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      For Ingress PCL: lookup0 or lookup1
*                                      For Egress PCL ignored
*
* @param[out] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note For DxCh2 and above - full support (beside the Standard IPV6 DIP key)
*       For DxCh1 - ingress only, keys for not-IP, IPV4 and IPV6 must
*       be the same size, Standard IPV6 DIP key allowed only on lookup1
*       For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X that use INDEX interface type
*       to configure entry for ePort, index = ePort
*
*/
GT_STATUS cpssDxChPclCfgTblGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_INTERFACE_INFO_STC        *interfaceInfoPtr,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    OUT CPSS_DXCH_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
);

/**
* @internal cpssDxChPclIngressPolicyEnable function
* @endinternal
*
* @brief   Enables Ingress Policy.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  ingress policy
*                                      GT_TRUE  - enable,
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclIngressPolicyEnable
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChPclIngressPolicyEnableGet function
* @endinternal
*
* @brief   Get status (Enable or Disable) of Ingress Policy.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable Policy
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclIngressPolicyEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPclTcamRuleSizeModeSet function
* @endinternal
*
* @brief   Set TCAM Rules size mode.
*         The rules TCAM may be configured to contain short rules (24 bytes), long
*         rules (48 bytes), or a mix of short rules and long rules.
*         The rule size may be set by global configuration or may be defined
*         dynamical according to PCL Configuration table settings.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond.
* @note   NOT APPLICABLE DEVICES:  DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mixedRuleSizeMode        - TCAM rule size mode
*                                      GT_TRUE  - mixed Rule Size Mode, rule size defined
*                                      by PCL configuration table
*                                      GT_FALSE - Rule size defined by ruleSize parameter
* @param[in] ruleSize                 - Rule size for not mixed mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclTcamRuleSizeModeSet
(
    IN GT_U8                       devNum,
    IN GT_BOOL                     mixedRuleSizeMode,
    IN CPSS_PCL_RULE_SIZE_ENT      ruleSize
);

/**
* @internal cpssDxChPclTcamRuleSizeModeGet function
* @endinternal
*
* @brief   Get TCAM Rules size mode.
*         The rules TCAM may be configured to contain short rules (24 bytes), long
*         rules (48 bytes), or a mix of short rules and long rules.
*         The rule size may be set by global configuration or may be defined
*         dynamical according to PCL Configuration table settings.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond.
* @note   NOT APPLICABLE DEVICES:  DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] mixedRuleSizeModePtr     - TCAM rule size mode
*                                      GT_TRUE  - mixed Rule Size Mode, rule size defined
*                                      by PCL configuration table
*                                      GT_FALSE - Rule size defined by ruleSize parameter
* @param[out] ruleSizePtr              - Rule size for not mixed mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPclTcamRuleSizeModeGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *mixedRuleSizeModePtr,
    OUT CPSS_PCL_RULE_SIZE_ENT      *ruleSizePtr
);

/**
* @internal cpssDxChPclUdeEtherTypeSet function
* @endinternal
*
* @brief   This function sets UDE Ethertype.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] index                    - UDE Ethertype index
*                                      (APPLICABLE RANGES: Lion, xCat2, Lion2 0..4; Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..6)
* @param[in] ethType                  - Ethertype value (APPLICABLE RANGES:  0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclUdeEtherTypeSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    IN  GT_U32          ethType
);

/**
* @internal cpssDxChPclUdeEtherTypeGet function
* @endinternal
*
* @brief   This function gets the UDE Ethertype.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] index                    - UDE Ethertype index
*                                      (APPLICABLE RANGES: Lion, xCat2, Lion2 0..4; Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..6)
*
* @param[out] ethTypePtr               - points to Ethertype value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclUdeEtherTypeGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_U32          *ethTypePtr
);

/**
* @internal cpssDxChPclTwoLookupsCpuCodeResolution function
* @endinternal
*
* @brief   Resolve the result CPU Code if both lookups has action commands
*         are either both TRAP or both MIRROR To CPU
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] lookupNum                - lookup number from which the CPU Code is selected
*                                      when action commands are either both TRAP or
*                                      both MIRROR To CPU
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclTwoLookupsCpuCodeResolution
(
    IN GT_U8                       devNum,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum
);

/**
* @internal cpssDxChPclInvalidUdbCmdSet function
* @endinternal
*
* @brief   Set the command that is applied when the policy key <User-Defined>
*         field cannot be extracted from the packet due to lack of header
*         depth (i.e, the field resides deeper than 128 bytes into the packet).
*         This command is NOT applied when the policy key <User-Defined>
*         field cannot be extracted due to the offset being relative to a layer
*         start point that does not exist in the packet. (e.g. the offset is relative
*         to the IP header but the packet is a non-IP).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] udbErrorCmd              - command applied to a packet:
*                                      continue lookup, trap to CPU, hard drop or soft drop
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclInvalidUdbCmdSet
(
    IN GT_U8                        devNum,
    IN CPSS_DXCH_UDB_ERROR_CMD_ENT  udbErrorCmd
);

/**
* @internal cpssDxChPclInvalidUdbCmdGet function
* @endinternal
*
* @brief   Get the command that is applied when the policy key <User-Defined>
*         field cannot be extracted from the packet due to lack of header
*         depth (i.e, the field resides deeper than 128 bytes into the packet).
*         This command is NOT applied when the policy key <User-Defined>
*         field cannot be extracted due to the offset being relative to a layer
*         start point that does not exist in the packet. (e.g. the offset is relative
*         to the IP header but the packet is a non-IP).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] udbErrorCmdPtr           - command applied to a packet:
*                                      continue lookup, trap to CPU, hard drop or soft drop
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPclInvalidUdbCmdGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_DXCH_UDB_ERROR_CMD_ENT  *udbErrorCmdPtr
);

/**
* @internal cpssDxChPclIpLengthCheckModeSet function
* @endinternal
*
* @brief   Set the mode of checking IP packet length.
*         To determine if an IP packet is a valid IP packet, one of the checks is a
*         length check to find out if the total IP length field reported in the
*         IP header is less or equal to the packet's length.
*         This function determines the check mode.
*         The results of this check sets the policy key <IP Header OK>.
*         The result is also used by the router engine to determine whether
*         to forward or trap/drop the packet.
*         There are two check modes:
*         1. For IPv4: ip_total_length <= packet's byte count
*         For IPv6: ip_total_length + 40 <= packet's byte count
*         2. For IPv4: ip_total_length + L3 Offset + 4 (CRC) <= packet's byte count,
*         For IPv6: ip_total_length + 40 +L3 Offset + 4 (CRC) <= packet's
*         byte count
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - IP packet length checking mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclIpLengthCheckModeSet
(
    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_ENT  mode
);

/**
* @internal cpssDxChPclIpLengthCheckModeGet function
* @endinternal
*
* @brief   Get the mode of checking IP packet length.
*         To determine if an IP packet is a valid IP packet, one of the checks is a
*         length check to find out if the total IP length field reported in the
*         IP header is less or equal to the packet's length.
*         This function determines the check mode.
*         The results of this check sets the policy key <IP Header OK>.
*         The result is also used by the router engine to determine whether
*         to forward or trap/drop the packet.
*         There are two check modes:
*         1. For IPv4: ip_total_length <= packet's byte count
*         For IPv6: ip_total_length + 40 <= packet's byte count
*         2. For IPv4: ip_total_length + L3 Offset + 4 (CRC) <= packet's byte count,
*         For IPv6: ip_total_length + 40 +L3 Offset + 4 (CRC) <= packet's
*         byte count
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - IP packet length checking mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPclIpLengthCheckModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_PCL_IP_LENGTH_CHECK_MODE_ENT  *modePtr
);

/**
* @internal cpssDxCh2PclEgressPolicyEnable function
* @endinternal
*
* @brief   Enables Egress Policy.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  Egress Policy
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported enable parameter value
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxCh2PclEgressPolicyEnable
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxCh2PclEgressPolicyEnableGet function
* @endinternal
*
* @brief   Get status of Egress Policy.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - status of Egress Policy
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL poinet
*/
GT_STATUS cpssDxCh2PclEgressPolicyEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPclEgressForRemoteTunnelStartEnableSet function
* @endinternal
*
* @brief   Enable/Disable the Egress PCL processing for the packets,
*         which Tunnel Start already done on remote ingress device.
*         These packets are ingessed with DSA Tag contains source port 60.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  Remote Tunnel Start Packets Egress Pcl
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressForRemoteTunnelStartEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_BOOL                       enable
);

/**
* @internal cpssDxChPclEgressForRemoteTunnelStartEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable of the Egress PCL processing for the packets,
*         which Tunnel Start already done on remote ingress device.
*         These packets are ingessed with DSA Tag contains source port 60.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable Remote Tunnel Start Packets Egress Pcl
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressForRemoteTunnelStartEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet function
* @endinternal
*
* @brief   Sets Egress Policy Configuration Table Access Mode for
*         Tunnel Start packets
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] cfgTabAccMode            - PCL Configuration Table access mode
*                                      CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E - VLAN ID
*                                      CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E - Normal
*                                      Normal means that access to PCL configuration table is done
*                                      according to settings done by cpssDxChPclCfgTblAccessModeSet
*                                      and cpssDxChPclPortLookupCfgTabAccessModeSet
*                                      as non tunnel start packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet
(
    IN  GT_U8                                            devNum,
    IN  CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT   cfgTabAccMode
);

/**
* @internal cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet function
* @endinternal
*
* @brief   Gets Egress Policy Configuration Table Access Mode for
*         Tunnel Start packets
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
*
* @param[out] cfgTabAccModePtr         - (pointer to) PCL Configuration Table access mode
*                                      CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E - VLAN ID
*                                      CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E - Normal
*                                      Normal means that access to PCL configuration table is done
*                                      according to settings done by cpssDxChPclCfgTblAccessModeSet
*                                      and cpssDxChPclPortLookupCfgTabAccessModeSet
*                                      as non tunnel start packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet
(
    IN  GT_U8                                            devNum,
    OUT CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT   *cfgTabAccModePtr
);

/**
* @internal cpssDxChPclEgressKeyFieldsVidUpModeSet function
* @endinternal
*
* @brief   Sets Egress Policy VID and UP key fields content mode
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] vidUpMode                - VID and UP key fields content mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressKeyFieldsVidUpModeSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   vidUpMode
);

/**
* @internal cpssDxChPclEgressKeyFieldsVidUpModeGet function
* @endinternal
*
* @brief   Gets Egress Policy VID and UP key fields content mode
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] vidUpModePtr             - (pointer to) VID and UP key fields
*                                      calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressKeyFieldsVidUpModeGet
(
    IN   GT_U8                                      devNum,
    OUT  CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   *vidUpModePtr
);

/**
* @internal cpssDxCh2EgressPclPacketTypesSet function
* @endinternal
*
* @brief   Enables/disables Egress PCL (EPCL) for set of packet types on port
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] pktType                  - packet type to enable/disable EPCL for it
* @param[in] enable                   -  EPCL for specific packet type
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note - after reset EPCL disabled for all packet types on all ports
*
*/
GT_STATUS cpssDxCh2EgressPclPacketTypesSet
(
    IN GT_U8                             devNum,
    IN GT_PHYSICAL_PORT_NUM              port,
    IN CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT pktType,
    IN GT_BOOL                           enable
);

/**
* @internal cpssDxCh2EgressPclPacketTypesGet function
* @endinternal
*
* @brief   Get status of Egress PCL (EPCL) for set of packet types on port
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] pktType                  - packet type to enable/disable EPCL for it
*
* @param[out] enablePtr                - enable EPCL for specific packet type
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxCh2EgressPclPacketTypesGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              port,
    IN  CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT pktType,
    OUT GT_BOOL                           *enablePtr
);

/**
* @internal cpssDxCh3PclTunnelTermForceVlanModeEnableSet function
* @endinternal
*
* @brief   The function enables/disables forcing of the PCL ID configuration
*         for all TT packets according to the VLAN assignment.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - force TT packets assigned to PCL
*                                      configuration table entry
*                                      GT_TRUE  - By VLAN
*                                      GT_FALSE - according to ingress port
*                                      per lookup settings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxCh3PclTunnelTermForceVlanModeEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_BOOL                       enable
);

/**
* @internal cpssDxCh3PclTunnelTermForceVlanModeEnableGet function
* @endinternal
*
* @brief   The function gets enable/disable of the forcing of the PCL ID configuration
*         for all TT packets according to the VLAN assignment.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) force TT packets assigned to PCL
*                                      configuration table entry
*                                      GT_TRUE  - By VLAN
*                                      GT_FALSE - according to ingress port
*                                      per lookup settings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxCh3PclTunnelTermForceVlanModeEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal cpssDxCh2PclTcpUdpPortComparatorSet function
* @endinternal
*
* @brief   Configure TCP or UDP Port Comparator entry
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] l4Protocol               - protocol, TCP or UDP
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..7)
* @param[in] l4PortType               - TCP/UDP port type, source or destination port
* @param[in] compareOperator          - compare operator FALSE, LTE, GTE, NEQ
* @param[in] value                    - 16 bit  to compare with
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxCh2PclTcpUdpPortComparatorSet
(
    IN GT_U8                             devNum,
    IN CPSS_PCL_DIRECTION_ENT            direction,
    IN CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN GT_U8                             entryIndex,
    IN CPSS_L4_PROTOCOL_PORT_TYPE_ENT    l4PortType,
    IN CPSS_COMPARE_OPERATOR_ENT         compareOperator,
    IN GT_U16                            value
);

/**
* @internal cpssDxCh2PclTcpUdpPortComparatorGet function
* @endinternal
*
* @brief   Get TCP or UDP Port Comparator entry
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] l4Protocol               - protocol, TCP or UDP
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..7)
*
* @param[out] l4PortTypePtr            - TCP/UDP port type, source or destination port
* @param[out] compareOperatorPtr       - compare operator FALSE, LTE, GTE, NEQ
* @param[out] valuePtr                 - 16 bit value to compare with
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxCh2PclTcpUdpPortComparatorGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_PCL_DIRECTION_ENT            direction,
    IN  CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN  GT_U8                             entryIndex,
    OUT CPSS_L4_PROTOCOL_PORT_TYPE_ENT    *l4PortTypePtr,
    OUT CPSS_COMPARE_OPERATOR_ENT         *compareOperatorPtr,
    OUT GT_U16                            *valuePtr
);

/**
* @internal cpssDxChPclRuleStateGet function
* @endinternal
*
* @brief   Get state (valid or not) of the rule and it's size
*         Old function for DxCh1 and DxCh2 devices and for
*         standard rules on DxCh3 devices.
*         The general function is cpssDxChPclRuleAnyStateGet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleIndex                - index of rule
*
* @param[out] validPtr                 -  rule's validity
*                                      GT_TRUE  - rule valid
*                                      GT_FALSE - rule invalid
* @param[out] ruleSizePtr              -  rule's size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclRuleStateGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT CPSS_PCL_RULE_SIZE_ENT        *ruleSizePtr
);

/**
* @internal cpssDxChPclRuleAnyStateGet function
* @endinternal
*
* @brief   Get state (valid or not) of the rule and it's size
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - size of rule
* @param[in] ruleIndex                - index of rule
*
* @param[out] validPtr                 -  rule's validity
*                                      GT_TRUE  - rule valid
*                                      GT_FALSE - rule invalid
* @param[out] ruleSizePtr              -  rule's size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclRuleAnyStateGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT CPSS_PCL_RULE_SIZE_ENT        *ruleSizePtr
);

/**
* @internal cpssDxChPclRuleGet function
* @endinternal
*
* @brief   The function gets the Policy Rule Mask, Pattern and Action in Hw format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ruleSize                 - size of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM.
*
* @param[out] maskArr[]                - rule mask          - 21 words.
* @param[out] patternArr[]             - rule pattern       - 21 words.
* @param[out] actionArr[]              - Policy rule action - 4 words.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclRuleGet
(
    IN  GT_U8                  devNum,
    IN  CPSS_PCL_RULE_SIZE_ENT ruleSize,
    IN  GT_U32                 ruleIndex,
    OUT GT_U32                 maskArr[],
    OUT GT_U32                 patternArr[],
    OUT GT_U32                 actionArr[]
);

/**
* @internal cpssDxChPclCfgTblEntryGet function
* @endinternal
*
* @brief   Gets the PCL configuration table entry in Hw format.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Ingress Policy or Egress Policy
*                                      (CPSS_PCL_DIRECTION_ENT member),
* @param[in] lookupNum                - Lookup number: lookup0 or lookup1
*                                      DxCh1-DxCh3 devices ignores the parameter
*                                      xCat and above supports the parameter
*                                      xCat and above devices has separate HW entries
*                                      for both ingress lookups.
*                                      Only lookup 0 is supported for egress.
* @param[in] entryIndex               - PCL configuration table entry Index
*
* @param[out] pclCfgTblEntryPtr        - pcl Cfg Tbl Entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_STATE             - in case of already bound pclHwId with same
*                                       combination of slot/direction
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note DxCh devices support ingress direction only
*
*/
GT_STATUS cpssDxChPclCfgTblEntryGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN  GT_U32                          entryIndex,
    OUT GT_U32                          *pclCfgTblEntryPtr
);

/**
* @internal cpssDxChPclLookupCfgPortListEnableSet function
* @endinternal
*
* @brief   The function enables/disables using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
* @param[in] enable                   - GT_TRUE  -  port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclLookupCfgPortListEnableSet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    IN  GT_BOOL                       enable
);

/**
* @internal cpssDxChPclLookupCfgPortListEnableGet function
* @endinternal
*
* @brief   The function gets enable/disable state of
*         using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclLookupCfgPortListEnableGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal cpssDxChPclPortGroupRuleSet function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] ruleFormat               - format of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM. The rule index defines
*                                      order of action resolution in the cases of multiple
*                                      rules match with packet's search key. Action of the
*                                      matched rule with lowest index is taken in this case
*                                      With reference to Standard and Extended rules
*                                      indexes, the partitioning is as follows:
*                                      For DxCh and DxCh2 devices:
*                                      - Standard rules.
*                                      Rule index may be in the range from 0 up to 1023.
*                                      - Extended rules.
*                                      Rule index may be in the range from 0 up to 511.
*                                      Extended rule consumes the space of two standard
*                                      rules:
*                                      - Extended rule with index  0 <= n <= 511
*                                      is placed in the space of two standard rules with
*                                      indexes n and n + 512.
*                                      For DxCh3 device:
*                                      The TCAM has 3.5K (3584) rows.
*                                      Each row can be used as:
*                                      - 4 standard rules
*                                      - 2 extended rules
*                                      - 1 extended and 2 standard rules
*                                      - 2 standard and 1 extended rules
*                                      The TCAM partitioning is as follows:
*                                      - Standard rules.
*                                      Rule index may be in the range from 0 up to 14335.
*                                      - Extended rules.
*                                      Rule index may be in the range from 0 up to 7167.
*                                      Extended rule consumes the space of two standard
*                                      rules:
*                                      - Extended rule with index  0 <= n < 3584
*                                      is placed in the space of two standard rules with
*                                      indexes n and n + 3584.
*                                      - Extended rule with index  3584 <= n < 7168
*                                      is placed in the space of two standard rules with
*                                      indexes n + 3584, n + 7168.
*                                      For xCat and above devices:
*                                      See datasheet of particular device for TCAM size.
*                                      For example describe the TCAM that has 1/4K (256) rows.
*                                      Each row can be used as:
*                                      - 4 standard rules
*                                      - 2 extended rules
*                                      - 1 extended and 2 standard rules
*                                      - 2 standard and 1 extended rules
*                                      The TCAM partitioning is as follows:
*                                      - Standard rules.
*                                      Rule index may be in the range from 0 up to 1K (1024).
*                                      - Extended rules.
*                                      Rule index may be in the range from 0 up to 0.5K (512).
*                                      Extended rule consumes the space of two standard
*                                      rules:
*                                      - Extended rule with index 2n (even index)
*                                      is placed in the space of two standard rules with
*                                      indexes 4n and 4n + 1.
*                                      - Extended rule with index 2n+1 (odd index)
*                                      is placed in the space of two standard rules with
*                                      indexes 4n+2 and 4n + 3.
* @param[in] ruleOptionsBmp           - Bitmap of rule's options.
*                                      The CPSS_DXCH_PCL_RULE_OPTION_ENT defines meaning of each bit.
*                                      Samples:
* @param[in] ruleOptionsBmp           = 0 - no options are defined.
*                                      Write rule to TCAM not using port-list format.
*                                      Rule state is valid.
* @param[in] ruleOptionsBmp           = CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E -
*                                      write all fields of rule to TCAM but rule's
*                                      state is invalid (no match during lookups).
* @param[in] maskPtr                  - rule mask. The rule mask is AND styled one. Mask
*                                      bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM lookup.
*                                      The format of mask is defined by ruleFormat
* @param[in] patternPtr               - rule pattern.
*                                      The format of pattern is defined by ruleFormat
* @param[in] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The pattern and mask input structures should be set to zero
*       by an application before assignment of fields.
*       The function places all muxed fields to rule patern and mask
*       by using bitwise OR operation.
*       The function does not check validity of muxed (overlapped) fields.
*
*/
GT_STATUS cpssDxChPclPortGroupRuleSet
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                             ruleIndex,
    IN CPSS_DXCH_PCL_RULE_OPTION_ENT      ruleOptionsBmp,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT      *patternPtr,
    IN CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclPortGroupRuleParsedGet function
* @endinternal
*
* @brief   The function gets the Policy Rule valid state, Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] ruleFormat               - format of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM.
* @param[in] ruleOptionsBmp           - Bitmap of rule's options.
*                                      The CPSS_DXCH_PCL_RULE_OPTION_ENT defines meaning of each bit.
*                                      Samples:
* @param[in] ruleOptionsBmp           = 0 - no options are defined.
*
* @param[out] isRuleValidPtr           - GT_TRUE - rule is valid, GT_FALSE - rule is invalid
* @param[out] maskPtr                  - rule mask. The rule mask is AND styled one. Mask
*                                      bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM lookup.
*                                      The format of mask is defined by ruleFormat
* @param[out] patternPtr               - rule pattern.
*                                      The format of pattern is defined by ruleFormat
* @param[out] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_BAD_STATE             - when size is trash or not related to ruleFormat.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortGroupRuleParsedGet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT               ruleFormat,
    IN GT_U32                                           ruleIndex,
    IN CPSS_DXCH_PCL_RULE_OPTION_ENT                    ruleOptionsBmp,
    OUT GT_BOOL                                         *isRuleValidPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                   *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                   *patternPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC                        *actionPtr
);

/**
* @internal cpssDxChPclPortGroupRuleActionUpdate function
* @endinternal
*
* @brief   The function updates the Rule Action
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] ruleSize                 - size of Rule. (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclPortGroupRuleSet.
* @param[in] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPclPortGroupRuleActionUpdate
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex,
    IN CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclPortGroupRuleActionGet function
* @endinternal
*
* @brief   The function gets the Rule Action
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] ruleSize                 - size of Rule. (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclPortGroupRuleSet.
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
*                                      Needed for parsing
*
* @param[out] actionPtr                - Policy rule action that applied on packet if packet's
*                                      search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPclPortGroupRuleActionGet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN  CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN  GT_U32                             ruleIndex,
    IN  CPSS_PCL_DIRECTION_ENT             direction,
    OUT CPSS_DXCH_PCL_ACTION_STC           *actionPtr
);

/**
* @internal cpssDxChPclPortGroupRuleInvalidate function
* @endinternal
*
* @brief   The function invalidates the Policy Rule.
*         For DxCh and DxCh2 devices start indirect rule write operation
*         with "garbage" pattern and mask content and "valid-flag" == 0
*         For DxCh3 devices calculates the TCAM position by ruleSize and ruleIndex
*         parameters and updates the first "valid" bit matching X/Y pair to (1,1)
*         i.e. don't match any value. If origin of valid or invalid rule found at the
*         specified TCAM position it will be invalid and available to back validation.
*         If the specified TCAM position contains the not first 24 byte segment of
*         extended rule, the rule also will be invalidated,
*         but unavailable to back validation.
*         If the garbage found in TCAM the X/Y bits will be updated to be as
*         in invalid rule.
*         The function does not check the TCAM contents.
*         GT_OK will be returned in each of described cases.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] ruleSize                 - size of Rule.(APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM. See cpssDxChPclPortGroupRuleSet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Backward compatible styled API.
*       The cpssDxChPclRuleValidStatusSet recommended for using instead.
*
*/
GT_STATUS cpssDxChPclPortGroupRuleInvalidate
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex
);

/**
* @internal cpssDxChPclPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the Policy rule.
*         The validation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Valid indication set.
*         The function does not check content of the rule before
*         write it back to TCAM
*         The invalidation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Invalid indication set.
*         If the given the rule found already in needed valid state
*         no write done. If the given the rule found with size
*         different from the given rule-size an error code returned.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] valid                    port group(s). If a bit of non  port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ruleSize                 - size of Rule.(APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
* @param[in] ruleIndex                - index of the rule in the TCAM.
* @param[in] valid                    - new rule status: GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if in TCAM found rule of size different
*                                       from the specified
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortGroupRuleValidStatusSet
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleIndex,
    IN GT_BOOL                            valid
);

/**
* @internal cpssDxChPclPortGroupRuleAnyStateGet function
* @endinternal
*
* @brief   Get state (valid or not) of the rule and it's size
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] ruleSize                 - size of rule. (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
* @param[in] ruleIndex                - index of rule
*
* @param[out] validPtr                 -  rule's validity
*                                      GT_TRUE  - rule valid
*                                      GT_FALSE - rule invalid
* @param[out] ruleSizePtr              -  rule's size
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPclPortGroupRuleAnyStateGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  CPSS_PCL_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                        ruleIndex,
    OUT GT_BOOL                       *validPtr,
    OUT CPSS_PCL_RULE_SIZE_ENT        *ruleSizePtr
);

/**
* @internal cpssDxChPclPortGroupRuleCopy function
* @endinternal
*
* @brief   The function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move Policy
*         Rule from old position to new one at first cpssDxChPclPortGroupRuleCopy should be
*         called. And after this cpssDxChPclPortGroupRuleInvalidate or
*         cpssDxChPclPortGroupRuleValidStatusSet should
*         be used to invalidate Rule in old position.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] ruleSize                 - size of Rule.
* @param[in] ruleSrcIndex             - index of the rule in the TCAM from which pattern,
*                                      mask and action are taken.
* @param[in] ruleDstIndex             - index of the rule in the TCAM to which pattern,
*                                      mask and action are placed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortGroupRuleCopy
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleSrcIndex,
    IN GT_U32                             ruleDstIndex
);

/**
* @internal cpssDxChPclPortGroupRuleGet function
* @endinternal
*
* @brief   The function gets the Policy Rule Mask, Pattern and Action in Hw format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] ruleSize                 - size of the Rule.
* @param[in] ruleIndex                - index of the rule in the TCAM.
*
* @param[out] maskArr[]                - rule mask          - 21  words.
* @param[out] patternArr[]             - rule pattern       - 21  words.
* @param[out] actionArr[]              - Policy rule action - 4 words.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupRuleGet
(
    IN  GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP     portGroupsBmp,
    IN  CPSS_PCL_RULE_SIZE_ENT ruleSize,
    IN  GT_U32                 ruleIndex,
    OUT GT_U32                 maskArr[],
    OUT GT_U32                 patternArr[],
    OUT GT_U32                 actionArr[]
);

/**
* @internal cpssDxChPclPortGroupCfgTblSet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceInfoPtr         - interface data: port, VLAN, or index for setting a
*                                      specific PCL Configuration Table entry
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      For Ingress PCL: lookup0 or lookup1
*                                      For Egress PCL ignored
* @param[in] lookupCfgPtr             - lookup configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note For DxCh2 and above - full support (beside the Standard IPV6 DIP key)
*       For DxCh1 - ingress only, keys for not-IP, IPV4 and IPV6 must
*       be of the same size, Standard IPV6 DIP key allowed only on lookup1
*       For Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X that use INDEX interface type
*       to configure entry for ePort, index = ePort
*
*/
GT_STATUS cpssDxChPclPortGroupCfgTblSet
(
    IN GT_U8                           devNum,
    IN GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN CPSS_INTERFACE_INFO_STC         *interfaceInfoPtr,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_DXCH_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
);

/**
* @internal cpssDxChPclPortGroupCfgTblGet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] interfaceInfoPtr         - interface data: port, VLAN, or index for getting a
*                                      specific PCL Configuration Table entry
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress
* @param[in] lookupNum                - Lookup number:
*                                      For Ingress PCL: lookup0 or lookup1
*                                      For Egress PCL ignored
*
* @param[out] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_BAD_PTR               - null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh2 and above - full support (beside the Standard IPV6 DIP key)
*       For DxCh1 - ingress only, keys for not-IP, IPV4 and IPV6 must
*       be the same size, Standard IPV6 DIP key allowed only on lookup1
*       For Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X that use INDEX interface type
*       to configure entry for ePort, index = ePort
*
*/
GT_STATUS cpssDxChPclPortGroupCfgTblGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  CPSS_INTERFACE_INFO_STC        *interfaceInfoPtr,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    OUT CPSS_DXCH_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
);

/**
* @internal cpssDxChPclPortGroupCfgTblEntryGet function
* @endinternal
*
* @brief   Gets the PCL configuration table entry in Hw format.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] direction                - Ingress Policy or Egress Policy
*                                      (CPSS_PCL_DIRECTION_ENT member),
* @param[in] lookupNum                - Lookup number: lookup0 or lookup1
*                                      DxCh1-DxCh3 devices ignores the parameter
*                                      xCat and above supports the parameter
*                                      xCat and above devices has separate HW entries
*                                      for both ingress lookups.
*                                      Only lookup 0 is supported for egress.
* @param[in] entryIndex               - PCL configuration table entry Index
*
* @param[out] pclCfgTblEntryPtr        - pcl Cfg Tbl Entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_STATE             - in case of already bound pclHwId with same
*                                       combination of slot/direction
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note DxCh devices support ingress direction only
*
*/
GT_STATUS cpssDxChPclPortGroupCfgTblEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  CPSS_PCL_DIRECTION_ENT          direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN  GT_U32                          entryIndex,
    OUT GT_U32                          *pclCfgTblEntryPtr
);

/**
* @internal cpssDxChPclPortGroupTcpUdpPortComparatorSet function
* @endinternal
*
* @brief   Configure TCP or UDP Port Comparator entry
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
* @param[in] value                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] l4Protocol               - protocol, TCP or UDP
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..7)
* @param[in] l4PortType               - TCP/UDP port type, source or destination port
* @param[in] compareOperator          - compare operator FALSE, LTE, GTE, NEQ
* @param[in] value                    - 16 bit  to compare with
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPclPortGroupTcpUdpPortComparatorSet
(
    IN GT_U8                             devNum,
    IN GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN CPSS_PCL_DIRECTION_ENT            direction,
    IN CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN GT_U8                             entryIndex,
    IN CPSS_L4_PROTOCOL_PORT_TYPE_ENT    l4PortType,
    IN CPSS_COMPARE_OPERATOR_ENT         compareOperator,
    IN GT_U16                            value
);

/**
* @internal cpssDxChPclPortGroupTcpUdpPortComparatorGet function
* @endinternal
*
* @brief   Get TCP or UDP Port Comparator entry
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] l4Protocol               - protocol, TCP or UDP
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..7)
*
* @param[out] l4PortTypePtr            - TCP/UDP port type, source or destination port
* @param[out] compareOperatorPtr       - compare operator FALSE, LTE, GTE, NEQ
* @param[out] valuePtr                 - 16 bit value to compare with
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPclPortGroupTcpUdpPortComparatorGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_PCL_DIRECTION_ENT            direction,
    IN  CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN  GT_U8                             entryIndex,
    OUT CPSS_L4_PROTOCOL_PORT_TYPE_ENT    *l4PortTypePtr,
    OUT CPSS_COMPARE_OPERATOR_ENT         *compareOperatorPtr,
    OUT GT_U16                            *valuePtr
);

/**
* @internal cpssDxChPclPortGroupUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ruleFormat               - rule format
*                                      Relevant for DxCh1, DxCh2, DxCh3 devices
* @param[in] packetType               - packet Type
*                                      Used for Ingress Only
*                                      (APPLICABLE DEVICES xCat; xCat3; Lion; xCat2; Lion2.)
*                                      Used for Ingress and Egress
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress.(APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      Ignored, configure ingress.
*                                      (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2)
* @param[in] udbIndex                 - index of User Defined Byte to configure.
*                                      See format of rules to known indexes of UDBs
* @param[in] offset                   - the type of offset (see CPSS_DXCH_PCL_OFFSET_TYPE_ENT)
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
GT_STATUS cpssDxChPclPortGroupUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN CPSS_PCL_DIRECTION_ENT               direction,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
);

/**
* @internal cpssDxChPclPortGroupUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ruleFormat               - rule format
*                                      Relevant for DxCh1, DxCh2, DxCh3 devices
* @param[in] packetType               - packet Type
*                                      Used for Ingress Only
*                                      (APPLICABLE DEVICES xCat; xCat3; Lion; xCat2; Lion2.)
*                                      Used for Ingress and Egress
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
* @param[in] direction                - Policy direction:
*                                      Ingress or Egress.(APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      Ignored, configure ingress.
*                                      (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2)
* @param[in] udbIndex                 - index of User Defined Byte to configure.
*                                      See format of rules to known indexes of UDBs
*
* @param[out] offsetTypePtr            - (pointer to) The type of offset
*                                      (see CPSS_DXCH_PCL_OFFSET_TYPE_ENT)
* @param[out] offsetPtr                - (pointer to) The offset of the user-defined byte,
*                                      in bytes,from the place in the packet
*                                      indicated by the offset type.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
GT_STATUS cpssDxChPclPortGroupUserDefinedByteGet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN  CPSS_PCL_DIRECTION_ENT               direction,
    IN  GT_U32                               udbIndex,
    OUT CPSS_DXCH_PCL_OFFSET_TYPE_ENT        *offsetTypePtr,
    OUT GT_U8                                *offsetPtr
);

/**
* @internal cpssDxChPclUserDefinedBytesSelectSet function
* @endinternal
*
* @brief   Set the User Defined Byte (UDB) Selection Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
*                                      Valid only UDB only key formats.
*                                      The parameter used also to determinate Ingress or Egress.
*                                      Relevant values are:
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon)
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E
* @param[in] packetType               - packet Type
* @param[in] lookupNum                - Lookup number. Ignored for egress.
* @param[in] udbSelectPtr             - (pointer to) structure with UDB Selection configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclUserDefinedBytesSelectSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum,
    IN  CPSS_DXCH_PCL_UDB_SELECT_STC         *udbSelectPtr
);

/**
* @internal cpssDxChPclUserDefinedBytesSelectGet function
* @endinternal
*
* @brief   Get the User Defined Byte (UDB) Selection Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
*                                      Valid only UDB only key formats.
*                                      The parameter used also to determinate Ingress or Egress.
*                                      Relevant values are:
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon)
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E
*                                      CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E
* @param[in] packetType               - packet Type
* @param[in] lookupNum                - Lookup number. Ignored for egress.
*
* @param[out] udbSelectPtr             - (pointer to) structure with UDB Selection configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on unsupported HW state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclUserDefinedBytesSelectGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum,
    OUT CPSS_DXCH_PCL_UDB_SELECT_STC         *udbSelectPtr
);

/**
* @internal cpssDxChPclPortGroupEgressKeyFieldsVidUpModeSet function
* @endinternal
*
* @brief   Sets Egress Policy VID and UP key fields content mode
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] vidUpMode                - VID and UP key fields content mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupEgressKeyFieldsVidUpModeSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PORT_GROUPS_BMP                         portGroupsBmp,
    IN  CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   vidUpMode
);

/**
* @internal cpssDxChPclPortGroupEgressKeyFieldsVidUpModeGet function
* @endinternal
*
* @brief   Gets Egress Policy VID and UP key fields content mode
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*
* @param[out] vidUpModePtr             - (pointer to) VID and UP key fields
*                                      calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupEgressKeyFieldsVidUpModeGet
(
    IN   GT_U8                                      devNum,
    IN   GT_PORT_GROUPS_BMP                         portGroupsBmp,
    OUT  CPSS_DXCH_PCL_EGRESS_KEY_VID_UP_MODE_ENT   *vidUpModePtr
);

/**
* @internal cpssDxChPclPortGroupLookupCfgPortListEnableSet function
* @endinternal
*
* @brief   The function enables/disables using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
* @param[in] enable                   - GT_TRUE  -  port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupLookupCfgPortListEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    IN  GT_BOOL                       enable
);

/**
* @internal cpssDxChPclPortGroupLookupCfgPortListEnableGet function
* @endinternal
*
* @brief   The function gets enable/disable state of
*         using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupLookupCfgPortListEnableGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal cpssDxChPclL3L4ParsingOverMplsEnableSet function
* @endinternal
*
* @brief   If enabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as IP packets for key selection and UDB usage.
*         If disabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as MPLS packets for key selection and UDB usage.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclL3L4ParsingOverMplsEnableSet
(
    IN  GT_U8                              devNum,
    IN  GT_BOOL                            enable
);

/**
* @internal cpssDxChPclL3L4ParsingOverMplsEnableGet function
* @endinternal
*
* @brief   If enabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as IP packets for key selection and UDB usage.
*         If disabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as MPLS packets for key selection and UDB usage.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclL3L4ParsingOverMplsEnableGet
(
    IN  GT_U8                              devNum,
    OUT GT_BOOL                            *enablePtr
);

/**
* @internal cpssDxChPclPortGroupOverrideUserDefinedBytesEnableSet function
* @endinternal
*
* @brief   The function enables or disables overriding of the
*         UDB content for specific UDB in specific key format
*         where it supported by HW.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ruleFormat               - rule format
* @param[in] udbOverrideType          - The type of the UDB configuration overriding.
* @param[in] enable                   - GT_TRUE -  overriding of UDB content
*                                      by udbOverrideType.
*                                      GT_FALSE - disable overriding of UDB content
*                                      by udbOverrideType, UDB configuration is used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupOverrideUserDefinedBytesEnableSet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  udbOverrideType,
    IN  GT_BOOL                              enable
);

/**
* @internal cpssDxChPclPortGroupOverrideUserDefinedBytesEnableGet function
* @endinternal
*
* @brief   The function get status (enabled or disabled) of the
*         overriding of the UDB content for specific UDB
*         in specific key format where it supported by HW.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ruleFormat               - rule format
* @param[in] udbOverrideType          - The type of the UDB configuration overriding.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - enable overriding of UDB content
*                                      by udbOverrideType.
*                                      GT_FALSE - disable overriding of UDB content
*                                      by udbOverrideType, UDB content may be overriden
*                                      by content of another udbOverrideType.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclPortGroupOverrideUserDefinedBytesEnableGet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  udbOverrideType,
    OUT GT_BOOL                              *enablePtr
);

/**
* @internal cpssDxChPclOverrideUserDefinedBytesEnableSet function
* @endinternal
*
* @brief   The function enables or disables overriding of the
*         UDB content for specific UDB in specific key format
*         where it supported by HW.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
* @param[in] udbOverrideType          - The type of the UDB configuration overriding.
* @param[in] enable                   - GT_TRUE -  overriding of UDB content
*                                      by udbOverrideType.
*                                      GT_FALSE - disable overriding of UDB content
*                                      by udbOverrideType, UDB configuration is used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclOverrideUserDefinedBytesEnableSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  udbOverrideType,
    IN  GT_BOOL                              enable
);

/**
* @internal cpssDxChPclOverrideUserDefinedBytesEnableGet function
* @endinternal
*
* @brief   The function get status (enabled or disabled) of the
*         overriding of the UDB content for specific UDB
*         in specific key format where it supported by HW.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - rule format
* @param[in] udbOverrideType          - The type of the UDB configuration overriding.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - enable overriding of UDB content
*                                      by udbOverrideType.
*                                      GT_FALSE - disable overriding of UDB content
*                                      by udbOverrideType, UDB content may be overriden
*                                      by content of another udbOverrideType.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclOverrideUserDefinedBytesEnableGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_UDB_OVERRIDE_TYPE_ENT  udbOverrideType,
    OUT GT_BOOL                              *enablePtr
);

/**
* @internal cpssDxChPclLookup0ForRoutedPacketsEnableSet function
* @endinternal
*
* @brief   Enables/disables PCL lookup0 for routed packets.
*         For not routed packets lookup0 always enable.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPclLookup0ForRoutedPacketsEnableSet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_NUM                        portNum,
    IN  GT_BOOL                            enable
);


/**
* @internal cpssDxChPclLookup0ForRoutedPacketsEnableGet function
* @endinternal
*
* @brief   Gets Enable/disable status of PCL lookup0 for routed packets.
*         For not routed packets lookup0 always enable.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS cpssDxChPclLookup0ForRoutedPacketsEnableGet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_NUM                        portNum,
    OUT GT_BOOL                            *enablePtr
);

/**
* @internal cpssDxChPclLookup1ForNotRoutedPacketsEnableSet function
* @endinternal
*
* @brief   Enables/disables PCL lookup1 for not routed packets.
*         For routed packets lookup1 always enable.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPclLookup1ForNotRoutedPacketsEnableSet
(
    IN  GT_U8                              devNum,
    IN  GT_BOOL                            enable
);

/**
* @internal cpssDxChPclLookup1ForNotRoutedPacketsEnableGet function
* @endinternal
*
* @brief   Gets Enable/disable status of PCL lookup1 for not routed packets.
*         For routed packets lookup1 always enable.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS cpssDxChPclLookup1ForNotRoutedPacketsEnableGet
(
    IN  GT_U8                              devNum,
    OUT GT_BOOL                            *enablePtr
);

/**
* @internal cpssDxChPclTcamEccDaemonEnableSet function
* @endinternal
*
* @brief   Enables or disables PCL TCAM ECC Daemon.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enalbe                   - GT_TRUE  - PCL TCAM ECC Daemon enable
*                                      GT_FALSE - PCL TCAM ECC Daemon disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPclTcamEccDaemonEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enalbe
);

/**
* @internal cpssDxChPclTcamEccDaemonEnableGet function
* @endinternal
*
* @brief   Gets status (Enabled/Disabled) of PCL TCAM ECC Daemon.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) PCL TCAM ECC Daemon status:
*                                      GT_TRUE  - PCL TCAM ECC Daemon enable
*                                      GT_FALSE - PCL TCAM ECC Daemon disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPclTcamEccDaemonEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPclIpMinOffsetSet function
* @endinternal
*
* @brief   Set the IP Minimum Offset parameter.
*         If packet is IPV4 fragment and has non zero offset
*         and it's offset less then IP Minimum Offset
*         the packet is counted as small offset.
*         The ipHeaderInfo field in PCL search key is set to be 3 i.e. "Ip Small Offset".
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] ipMinOffset              - IP Minimum Offset  parameter (APPLICABLE RANGES: 0..0x1FFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range minIpOffset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclIpMinOffsetSet
(
    IN  GT_U8                              devNum,
    IN  GT_U32                             ipMinOffset
);

/**
* @internal cpssDxChPclIpMinOffsetGet function
* @endinternal
*
* @brief   Get the IP Minimum Offset parameter.
*         If packet is IPV4 fragment and has non zero offset
*         and it's offset less then IP Minimum Offset
*         the packet is counted as small offset.
*         The ipHeaderInfo field in PCL search key is set to be 3 i.e. "Ip Small Offset".
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @param[out] ipMinOffsetPtr           - (pointer to)IP Minimum Offset  parameter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range minIpOffset
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclIpMinOffsetGet
(
    IN  GT_U8                              devNum,
    OUT GT_U32                             *ipMinOffsetPtr
);

/**
* @internal cpssDxChPclPortIngressLookup0Sublookup1TypeSet function
* @endinternal
*
* @brief   Configures Ingress PCL Lookup0 sublookup1 type.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] lookupType               - partial or full IPCL lookup0 sublookup1 type
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortIngressLookup0Sublookup1TypeSet
(
    IN GT_U8                                      devNum,
    IN CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT lookupType
);

/**
* @internal cpssDxChPclPortIngressLookup0Sublookup1TypeGet function
* @endinternal
*
* @brief   Configures Ingress PCL Lookup0 sublookup1 type.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @param[out] lookupTypePtr            - (pointer to)partial or full IPCL lookup0 sublookup1 type
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortIngressLookup0Sublookup1TypeGet
(
    IN  GT_U8                                     devNum,
    OUT CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT *lookupTypePtr
);

/**
* @internal cpssDxChPclPortListGroupingEnableSet function
* @endinternal
*
* @brief   Enable/disable port grouping mode per direction. When enabled, the 4 MSB
*         bits of the <Port List> field in the PCL keys are replaced by a 4-bit
*         <Port Group>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
* @param[in] enable                   - enable/disable port grouping mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortListGroupingEnableSet
(
    IN GT_U8                    devNum,
    IN CPSS_PCL_DIRECTION_ENT   direction,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChPclPortListGroupingEnableGet function
* @endinternal
*
* @brief   Get the port grouping mode enabling status per direction. When enabled,
*         the 4 MSB bits of the <Port List> field in the PCL keys are replaced by a
*         4-bit <Port Group>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
*
* @param[out] enablePtr                - (pointer to) the port grouping mode enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortListGroupingEnableGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChPclPortListPortMappingSet function
* @endinternal
*
* @brief   Set port group mapping and offset in port list for PCL working in Physical
*         Port List.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
* @param[in] portNum                  - port number
* @param[in] enable                   - port participate in the port list vector.
* @param[in] group                    - PCL port group
*                                      Relevant only if <enable> == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] offset                   -  in the port list bit vector.
*                                      Relevant only if <enable> == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..27)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When working in PCL Port List mode with port grouping enabled, <offset>
*       value in the range 23..27 has no influence.
*
*/
GT_STATUS cpssDxChPclPortListPortMappingSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN  GT_U32                  group,
    IN  GT_U32                  offset
);

/**
* @internal cpssDxChPclPortListPortMappingGet function
* @endinternal
*
* @brief   Get port group mapping and offset in port list for PCL working in Physical
*         Port List.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) port participate in the port list vector.
* @param[out] groupPtr                 - (pointer to) PCL port group
* @param[out] offsetPtr                - (pointer to) offset in the port list bit vector.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclPortListPortMappingGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_U32                  *groupPtr,
    OUT GT_U32                  *offsetPtr
);

/**
* @internal cpssDxChPclOamChannelTypeProfileToOpcodeMappingSet function
* @endinternal
*
* @brief   Set Mapping of MPLS Channel Type Profile to OAM opcode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] channelTypeProfile       - MPLS G-ACh Channel Type Profile
*                                      (APPLICABLE RANGES: 1..15)
* @param[in] opcode                   - OAM  (APPLICABLE RANGES: 0..255)
* @param[in] RBitAssignmentEnable     - replace Bit0 of opcode by packet R-flag
*                                      GT_TRUE  - Bit0 of result opcode is packet R-flag
*                                      GT_FALSE - Bit0 of result opcode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamChannelTypeProfileToOpcodeMappingSet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    channelTypeProfile,
    IN  GT_U32                    opcode,
    IN  GT_BOOL                   RBitAssignmentEnable
);

/**
* @internal cpssDxChPclOamChannelTypeProfileToOpcodeMappingGet function
* @endinternal
*
* @brief   Get Mapping of MPLS Channel Type Profile to OAM opcode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] channelTypeProfile       - MPLS G-ACh Channel Type Profile
*                                      (APPLICABLE RANGES: 1..15)
*
* @param[out] opcodePtr                - (pointer to)OAM opcode
* @param[out] RBitAssignmentEnablePtr  - (pointer to)replace Bit0 of opcode by packet R-flag
*                                      GT_TRUE  - Bit0 of result opcode is packet R-flag
*                                      GT_FALSE - Bit0 of result opcode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamChannelTypeProfileToOpcodeMappingGet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    channelTypeProfile,
    OUT GT_U32                    *opcodePtr,
    OUT GT_BOOL                   *RBitAssignmentEnablePtr
);

/**
* @internal cpssDxChPclOamMegLevelModeSet function
* @endinternal
*
* @brief   Set OAM MEG Level Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction.
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
* @param[in] megLevelMode             - OAM MEG Level Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamMegLevelModeSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_PCL_DIRECTION_ENT                    direction,
    IN  GT_U32                                    oamProfile,
    IN  CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_ENT  megLevelMode
);

/**
* @internal cpssDxChPclOamMegLevelModeGet function
* @endinternal
*
* @brief   Get OAM MEG Level Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction.
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
*
* @param[out] megLevelModePtr          - (pointer to) OAM MEG Level Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_STATE             - on unexpected value in HW
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamMegLevelModeGet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_PCL_DIRECTION_ENT                    direction,
    IN  GT_U32                                    oamProfile,
    OUT CPSS_DXCH_PCL_OAM_PKT_MEG_LEVEL_MODE_ENT  *megLevelModePtr
);

/**
* @internal cpssDxChPclOamRdiMatchingSet function
* @endinternal
*
* @brief   Set OAM RDI(Remote Direct Indicator) Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction.
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
* @param[in] rdiMask                  - Mask for RDI bit.(APPLICABLE RANGES: 0..255)
* @param[in] rdiPattern               - Pattern for RDI bit.(APPLICABLE RANGES: 0..255)
*                                      Extracted packet byte pointed by UDB4 (profile0) or UDB5 (profile1)
*                                      Result = ((packetByte & rdiMask) == rdiPattern) ? 0 : 1.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamRdiMatchingSet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  GT_U32                        oamProfile,
    IN  GT_U32                        rdiMask,
    IN  GT_U32                        rdiPattern
);

/**
* @internal cpssDxChPclOamRdiMatchingGet function
* @endinternal
*
* @brief   Get OAM RDI(Remote Direct Indicator) Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction.
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
*
* @param[out] rdiMaskPtr               - (pointer to)Mask for RDI bit.(APPLICABLE RANGES: 0..255)
* @param[out] rdiPatternPtr            - (pointer to)Pattern for RDI bit.(APPLICABLE RANGES: 0..255)
*                                      Extracted packet byte pointed by UDB4 (profile0) or UDB5 (profile1)
*                                      Result = ((packetByte & rdiMask) == rdiPattern) ? 0 : 1.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamRdiMatchingGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  GT_U32                        oamProfile,
    OUT GT_U32                        *rdiMaskPtr,
    OUT GT_U32                        *rdiPatternPtr
);

/**
* @internal cpssDxChPclOamEgressPacketDetectionSet function
* @endinternal
*
* @brief   Set OAM Egress Packet Detection parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Egress Packet Detection mode.
* @param[in] sourceId                 - packet Source Id. When mode ==
*                                      CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_SOURCE_ID_E
*                                      and packet source ID equal to sourceId value
*                                      the packet treated as OAM packet
*                                      regardless to PCL action results.
*                                      (APPLICABLE RANGES: 0..0xFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamEgressPacketDetectionSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_ENT  mode,
    IN  GT_U32                                    sourceId
);

/**
* @internal cpssDxChPclOamEgressPacketDetectionGet function
* @endinternal
*
* @brief   Get OAM Egress Packet Detection parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to)Egress Packet Detection mode.
* @param[out] sourceIdPtr              - (pointer to)packet Source Id. When mode ==
*                                      CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_SOURCE_ID_E
*                                      and packet source ID equal to sourceId value
*                                      the packet treated as OAM packet
*                                      regardless to PCL action results.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamEgressPacketDetectionGet
(
    IN  GT_U8                                     devNum,
    OUT CPSS_DXCH_PCL_OAM_EGRESS_DETECT_MODE_ENT  *modePtr,
    OUT GT_U32                                    *sourceIdPtr
);

/**
* @internal cpssDxChPclOamRFlagMatchingSet function
* @endinternal
*
* @brief   Set OAM R-Flag Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
* @param[in] rFlagMask                - Mask for R-Flag.(APPLICABLE RANGES: 0..255)
* @param[in] rFlagPattern             - Pattern for R-Flag.(APPLICABLE RANGES: 0..255)
*                                      Extracted packet byte pointed by UDB8 (profile0) or UDB9 (profile1)
*                                      Result = ((packetByte & rFlagMask) == rFlagPattern) ? 0 : 1.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamRFlagMatchingSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        oamProfile,
    IN  GT_U32                        rFlagMask,
    IN  GT_U32                        rFlagPattern
);

/**
* @internal cpssDxChPclOamRFlagMatchingGet function
* @endinternal
*
* @brief   Get OAM R-Flag Matching parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] oamProfile               - OAM profile (APPLICABLE RANGES: 0..1)
*
* @param[out] rFlagMaskPtr             - (pointer to)Mask for R-Flag.(APPLICABLE RANGES: 0..255)
* @param[out] rFlagPatternPtr          - (pointer to)Pattern for R-Flag.(APPLICABLE RANGES: 0..255)
*                                      Extracted packet byte pointed by UDB8 (profile0) or UDB9 (profile1)
*                                      Result = ((packetByte & rFlagMask) == rFlagPattern) ? 0 : 1.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclOamRFlagMatchingGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        oamProfile,
    OUT GT_U32                        *rFlagMaskPtr,
    OUT GT_U32                        *rFlagPatternPtr
);

/**
* @internal cpssDxChPclEgressSourcePortSelectionModeSet function
* @endinternal
*
* @brief   Set EPCL source port field selection mode for DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portSelectionMode        - Source port selection mode for DSA packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <SrcPort> always reflects the local device source physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <SrcPort> is taken from the DSA tag
*       if packet was received DSA tagged.
*
*/
GT_STATUS cpssDxChPclEgressSourcePortSelectionModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT portSelectionMode
);

/**
* @internal cpssDxChPclEgressSourcePortSelectionModeGet function
* @endinternal
*
* @brief   Get EPCL source port field selection mode for DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] portSelectionModePtr     - (pointer to) Source port selection mode for
*                                      DSA packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <SrcPort> always reflects the local device source physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <SrcPort> is taken from the DSA tag
*       if packet was received DSA tagged.
*
*/
GT_STATUS cpssDxChPclEgressSourcePortSelectionModeGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT *portSelectionModePtr
);

/**
* @internal cpssDxChPclEgressTargetPortSelectionModeSet function
* @endinternal
*
* @brief   Set EPCL target port field selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portSelectionMode        - Target port selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclEgressTargetPortSelectionModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT portSelectionMode
);

/**
* @internal cpssDxChPclEgressTargetPortSelectionModeGet function
* @endinternal
*
* @brief   Get EPCL target port field selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] portSelectionModePtr     - (pointer to) Target port selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclEgressTargetPortSelectionModeGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT
                                                        *portSelectionModePtr
);

/**
* @internal cpssDxChPclSourceIdMaskSet function
* @endinternal
*
* @brief   Set mask so only certain bits in the source ID will be modified due to PCL
*         action.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] lookupNum                - lookup number
* @param[in] mask                     - Source ID  (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclSourceIdMaskSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum,
    IN  GT_U32                      mask
);

/**
* @internal cpssDxChPclSourceIdMaskGet function
* @endinternal
*
* @brief   Get mask used for source ID modify due to PCL action.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] lookupNum                - lookup number
*
* @param[out] maskPtr                  - (pointer to) Source ID mask (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclSourceIdMaskGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum,
    OUT GT_U32                      *maskPtr
);

/**
* @internal cpssDxChPclIpPayloadMinSizeSet function
* @endinternal
*
* @brief   Set the IP payload minimum size. This value will determine the Layer 4
*         validity
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ipPayloadMinSize         - the IP payload minimum size
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The L4_Valid in the packet descriptor will be cleared if:
*       For IPv4: total length - header length < IP payload minimum size
*       For IPv6: payload length < IP payload minimum size
*
*/
GT_STATUS cpssDxChPclIpPayloadMinSizeSet
(
    IN GT_U8        devNum,
    IN GT_U32       ipPayloadMinSize
);

/**
* @internal cpssDxChPclIpPayloadMinSizeGet function
* @endinternal
*
* @brief   Get the IP payload minimum size. This value will determine the Layer 4
*         validity
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] ipPayloadMinSizePtr      - the IP payload minimum size
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The L4_Valid in the packet descriptor will be cleared if:
*       For IPv4: total length - header length < IP payload minimum size
*       For IPv6: payload length < IP payload minimum size
*
*/
GT_STATUS cpssDxChPclIpPayloadMinSizeGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *ipPayloadMinSizePtr
);


/**
* @internal cpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet function
* @endinternal
*
* @brief   Function enables/disables for ingress analyzed packet the using of VID from
*         the incoming packet to access the EPCL Configuration table and for lookup
*         keys generation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - use original VID
*                                      GT_FALSE - use VLAN assigned by the processing pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_BOOL      enable
);


/**
* @internal cpssDxChPclEgressRxAnalyzerUseOrigVidEnableGet function
* @endinternal
*
* @brief   Get state of the flag in charge of the using of VID of the incoming packet
*         for ingress analyzed packet to access the EPCL Configuration table and for
*         lookup keys generation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable/disable the using of original VID.
*                                      GT_TRUE  - use original VID
*                                      GT_FALSE - use VLAN assigned by the processing pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclEgressRxAnalyzerUseOrigVidEnableGet
(
    IN  GT_U8         devNum,
    OUT GT_BOOL       *enablePtr
);

/**
* @internal cpssDxChPclEgressSubLookupPhaInfoSet function
* @endinternal
*
* @brief   Function sets the masks for PHA related info that will
*          be overridden by the specific sub-lookup.
*       NOTE: relate only to EPCL (Egress PCL)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number
* @param[in] subLookupNum             - sub lookup number for lookup.
*                                      (APPLICABLE RANGES: 0..3).
* @param[in] phaMetadataMask          - the PHA 'meta data' mask for this sub-lookup.(32 bits mask)
* @param[in] phaThreadIdMask          - the PHA 'thread Id' mask for this sub-lookup.( 8 bits mask)
*                                      (APPLICABLE RANGES: 0..255).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or subLookupNum or phaThreadIdMask
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclEgressSubLookupPhaInfoSet
(
    IN GT_U8        devNum,
    IN GT_U32       subLookupNum,
    IN GT_U32       phaMetadataMask,
    IN GT_U32       phaThreadIdMask
);

/**
* @internal cpssDxChPclEgressSubLookupPhaInfoGet function
* @endinternal
*
* @brief   Function gets the masks for PHA related info that will
*          be overridden by the specific sub-lookup.
*       NOTE: relate only to EPCL (Egress PCL)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number
* @param[in] subLookupNum             - sub lookup number for lookup.
*                                      (APPLICABLE RANGES: 0..3).
* @param[out] phaMetadataMaskPtr      - (pointer to)the PHA 'meta data' mask for this sub-lookup.(32 bits mask)
* @param[out] phaThreadIdMaskPtr      - (pointer to)the PHA 'thread Id' mask for this sub-lookup.( 8 bits mask)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or subLookupNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPclEgressSubLookupPhaInfoGet
(
    IN GT_U8        devNum,
    IN GT_U32       subLookupNum,
    OUT GT_U32      *phaMetadataMaskPtr,
    OUT GT_U32      *phaThreadIdMaskPtr
);

/**
* @internal cpssDxChPclRulesDump function
* @endinternal
*
* @brief    Dump all valid PCL rules of specified size.
*
* @note     APPLICABLE DEVICES:   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                          (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                          bitmap must be set with at least one bit representing
*                                          valid port group(s). If a bit of non valid port group
*                                          is set then function returns GT_BAD_PARAM.
*                                          value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ruleSize                 - size of rule
* @param[in] startIndex               - index of first rule
* @param[in] rulesAmount              - number of rules to scan and dump
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_BAD_STATE             - For DxCh3 and above if cannot determinate
*                                       the rule size by found X/Y bits of compare mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPclRulesDump
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             startIndex,
    IN GT_U32                             rulesAmount
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPclh */

