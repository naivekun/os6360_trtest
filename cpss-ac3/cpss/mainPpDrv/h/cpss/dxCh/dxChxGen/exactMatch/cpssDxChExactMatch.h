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
* @file cpssDxChExactMatch.h
*
* @brief CPSS Exact Match declarations.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChExactMatchh
#define __cpssDxChExactMatchh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>

/* max key size in bytes (47 byte --> 12 words --> 376 bits of data) */
#define CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS 47

/* max number of banks in Exact Match */
#define CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS   16

/**
 * @enum CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT
 *
 * @brief Exact Match key size
 * APPLICABLE DEVICES: Falcon.
 *
*/
typedef enum{
    /** Exact Match key size 5 Bytes (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,

    /** Exact Match key size 14 Bytes (APPLICABLE DEVICES: Falcon */
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,

    /** Exact Match key size 33 Bytes (APPLICABLE DEVICES: Falcon */
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,

    /** Exact Match key size 47 Bytes (APPLICABLE DEVICES: Falcon */
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,

    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E

} CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT;

/**
* @enum CPSS_DXCH_EXACT_MATCH_CLIENT_ENT
 *
*  @brief Exact Match clients.
*  APPLICABLE DEVICES: Falcon.
*/
typedef enum{

    /** EXACT MATCH client tunnel termination (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E,

    /** EXACT MATCH client ingress policy 0 (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E,

    /** EXACT MATCH client ingress policy 1 (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E,

   /** EXACT MATCH client egress policy (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E,

    CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E

} CPSS_DXCH_EXACT_MATCH_CLIENT_ENT;



/**
 * @struct CPSS_EXACT_MATCH_PROFILE_KEY_PARAMS_STC
 *
 * @brief Exact Match Profile key parameters
*/
typedef struct{

    /** @brief key Size */
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT keySize;

    /** @brief Start of Exact Match Search (Byte offset from the
     *         TCAM key. */
    GT_U32 keyStart;

    /** @brief Mask the bits in the Exact Match Search Key */
    GT_U8 mask[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];

} CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC;


/**
* @enum CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT
 *
 * @brief Exact Match action type enum
*/
typedef enum{

    /** Exact Match action type is TTI action (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E   = 0,

    /** Exact Match action type is PCL action (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E   = 1,

    /** Exact Match action type is EPCL action (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E   = 2,

    CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E

} CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT;


/**
 * @enum CPSS_DXCH_EXACT_MATCH_ACTION_UNT
 *
 * @brief Exact Match action union
*/
typedef union{
    /** Exact Match action is TTI action (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_TTI_ACTION_2_STC ttiAction;

    /** Exact Match action is PCL action (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_PCL_ACTION_STC   pclAction;

} CPSS_DXCH_EXACT_MATCH_ACTION_UNT;

/**
* @struct CPSS_DXCH_EXACT_MATCH_USE_TTI_ACTIONS_STC
*
*  @brief EXACT MATCH use TTI TCAM rule action parameters.
*
*         Determines whether the associated byte is assigned
*         from the Exact Match reduces Action, or assigned a
*         default value.
*         GT_FALSE = Assigned default value
*         GT_TRUE = Assigned from Action bytes of the exact
*         match Entry
*
*         The overwritten mean field will be taken from reduced
*         Action (aka replacing value at expended action)
*/
typedef struct{

    /** @brief Command assigned to packets matching this TTI rule
     *         valid values: forward, mirror, trap, hard_drop_soft_drop
     *  bits 0-2  */
    GT_BOOL overwriteExpandedActionCommand;

    /** @brief If ActionCommand = TRAP or MIRROR or SOFT/HARD DROP,
     *         this code is the CPU/DROP Code passed to the CPU as
     *         part of the packet.
     *  bits 3-10  */
    GT_BOOL overwriteExpandedActionUserDefinedCpuCode;

     /** @brief Enables mirroring the packet to an Ingress Analyzer interface.
      *         The field is three bits long, and can indicate one of seven possible analyzers.
      *         A value of ZERO indicates that mirroring is not enabled.
      *         If the port configuration assigns a different
      *         analyzer index, the higher index wins.
      *  bits 11-13  */
    GT_BOOL overwriteExpandedActionMirrorToIngressAnalyzerIndex;

   /** @brief where to redirect the packet
    *  0x0 = NoRedirect; Do not redirect this packet.
    *  0x1 = Egress Interface; Redirect this packet to the
    *  configured Egress Interface.
    *  0x2 = IPNextHop; Policy-based routing: assign index to
    *  Router Lookup Translation Table (LTT).
    *  0x4 = Assign VRF-ID; Assign VRF-ID from the TTI Action
    *  entry. The packet is not redirected.
    *  bits 14-16 */
    GT_BOOL overwriteExpandedActionRedirectCommand;

    /** @brief egress interface when
    *           redirect command = "Egress Interface"
    * bits 17 - target is lag
    * bits 17-32 eVidx
    * bits 18-29 trunk ID
    * bits 18-31 target ePort
    * bits 32-41 target device
    * bit 42 useVidx */

    GT_BOOL overwriteExpandedActionEgressInterface;

    /** @brief VRF
     * bits 17-28 */
    GT_BOOL overwriteExpandedActionVrfId;

    /** @brief The flow ID when redirect command = "No Redirect"
     *   bits 17-29 */
    GT_BOOL overwriteExpandedActionFlowId;

    /** @brief The IP Next Hop Entry Index
     *      when redirect command = "IP Next Hop"
     *   bits 17-34 */
    GT_BOOL overwriteExpandedActionRouterLttPtr;

    /** @brief Used for building hash key and IPCL keys.
        Direct index to one of 7 UDB configuration table user defined
        EtherType entries, from index 9 to 15 (UDE entries in index 6 and 8
        are not accessible using this direct index). Used for extracting
        packet header fields for packets that differ in their header format,
        but share the same EtherType (e.g., MPLS and MPLS-TP).
        0 - Disabled: Use packet EtherType to select the UDB
        configuration table entry
        1- Assign UDB configuration table index 9
        2 - Assign UDB configuration table index 10
        .....
        7 - Assign UDB configuration table index 15.
        relevant when redirect command = "No Redirect"
        bits 30-32 */
    GT_BOOL overwriteExpandedActionIpclUdbConfigTable;

    /** @brief If set, this a network testing flow.
     *  Therefore the packet should be redirected to an
     *  egress interface where prior to transmission the packet's
     *  MAC SA and MAC DA are switched.
     *  NOTE: This should always be accompanied with
     *  Redirect Command = 1 (Policy Switching).
     *  bit 43   */
    GT_BOOL overwriteExpandedActionVntl2Echo;

    /** @brief When set, packet is redirected to a tunnel start
     *         entry pointed by Tunnel_Index
     * NOTE: This field is valid only when Redirect Command =
     * Redirect to Egress Interface (Policy-switching)
     * bit 44 */
    GT_BOOL overwriteExpandedActionTunnelStart;

    /** @brief This field is valid only when
     *  Redirect_Command="Egress_Interface" and
     *  TunnelStart="False"
     *  bits 45-62 */
    GT_BOOL overwriteExpandedActionArpPtr;

    /** @brief This field is valid only when
     *  Redirect_Command="Egress_Interface" and TunnelStart="True"
     *  bits 45-60 */
    GT_BOOL overwriteExpandedActionTunnelIndex;


     /** @brief Controls the index used for IPCL0 lookup
      *  relevant when Redirect_Command !="Egress_Interface"
      *  bit 51 */
    GT_BOOL overwriteExpandedActionPcl0OverrideConfigIndex;

    /** @brief Controls the index used for IPCL1 lookup
     *  relevant when Redirect_Command !="Egress_Interface"
     *  bit 50 */
    GT_BOOL overwriteExpandedActionPcl1OverrideConfigIndex;

    /** @brief Controls the index used for IPCL2 lookup
     *  relevant when Redirect_Command !="Egress_Interface"
     *  bit 49 */
    GT_BOOL overwriteExpandedActionPcl2OverrideConfigIndex;

    /** @brief Pointer to IPCL configuration entry to be used when
     *  fetching IPCL parameters.Relevant when TTI Action "PCL0/1/2
     *  Override Config Index" = Override. Min-Max: 0x0 - 0x10FF
     *  relevant when Redirect_Command !="Egress
     * bits 52-64 */
    GT_BOOL overwriteExpandedActionIpclConfigIndex;

    /** @brief Type of passenger packet being to be encapsulated.
        This field is valid only when Redirect Command = 1
        (Policy-switching) and TunnelStart = 1 When packet is not TT
         and MPLS_Command != NOP, the Tunnel_Type bit must be set
         to OTHER. 0x0 = Ethernet; 0x1 = Other;
         bit 61 */
    GT_BOOL overwriteExpandedActionTsPassengerPacketType;

    /** @brief When enabled, the packet is marked for MAC DA
     *         modification, and the ARP_Index (bits 45-62)
     *         specifies the new MAC DA.
     *  relevant when Redirect_Command =="Egress_Interface"
     *  bit 63 */
    GT_BOOL overwriteExpandedActionModifyMacDa;

     /** @brief When enabled, the packet is marked for MAC SA
      *         modification - similar to routed packets.
      *  relevant when Redirect_Command =="Egress_Interface"
      *  bit 64 */
    GT_BOOL overwriteExpandedActionModifyMacSa;

    /** @brief If set, the packet is bound to the a counter
     *         specified in CNCCounterIndex
     *  bit 65 */
    GT_BOOL overwriteExpandedActionBindToCentralCounter;

    /** @brief Counter index to be used if the BindToCNCCounter
     *  is set.
     *  bits 66-79 */
    GT_BOOL overwriteExpandedActionCentralCounterIndex;

    /** @brief  If set, the packet is bound to the a meter
     *          specified in PolicerIndex.
     *  bit 80 */
    GT_BOOL overwriteExpandedActionBindToPolicerMeter;

    /** @brief If set, the packet is bound to the counter specified
     *         in the PolicerIndex.
     *  NOTE: When both BindToPolicerMeter and BindToPolicerCounter
     *  are set, the PolicerIndex is used for metering and the
     *  counter index is assigned by the metering entry.
     *  bit 81 */
    GT_BOOL overwriteExpandedActionBindToPolicer;

    /** @brief  Traffic profile or counter to be used when the
     *  BindToPolicerMeter or BindToPolicerCounter are set.
     *  bits 82-95 */
    GT_BOOL overwriteExpandedActionPolicerIndex;

     /** if set, then the sourceId is assign to the packet
      *  bit 96 */
    GT_BOOL overwriteExpandedActionSourceIdSetEnable;

    /** @brief Source ID (also known as SST-ID) that is assign to
     *         the packet if SourceIDAssignmentEnable is set.
     *  bits 97-108 */
    GT_BOOL overwriteExpandedActionSourceId;

    /** @brief if set, the packet will not have any searches
      *  in the IPCL mechanism
      *  bit 109 */
    GT_BOOL overwriteExpandedActionActionStop;

    /** @brief if set, the packet isn't subject to any bridge
      * mechanisms bit 110 */
    GT_BOOL overwriteExpandedActionBridgeBypass;

    /** If set, the ingress engines are all bypassed for this packet
     *  bit 111 */
    GT_BOOL overwriteExpandedActionIngressPipeBypass;

    /** @brief VLAN0 precedence options
     *  0x0 = Soft; VID assigned by TTI can be modified by the
     *  following VLAN assignment mechanisms.
     *  0x1 = Hard; VID assigned by TTI cannot be modified by the
     *  following VLAN assignment mechanisms.
     *  bit 112   */
    GT_BOOL overwriteExpandedActionTag0VlanPrecedence;

    /** @brief enable/disable nested vlan
        If set, the packet is classified as Untagged.
        NOTE: Relevant only if the packet is Ethernet or
        tunnel-terminated Ethernet-over-X.
        bit 113 */
    GT_BOOL overwriteExpandedActionNestedVlanEnable;

   /** @brief tag0 vlan command. This field determines the eVLAN
    *         assigned to the packet
    *  bits 114-116 */
    GT_BOOL overwriteExpandedActionTag0VlanCmd;

    /** @brief tag0 VLAN
     *  NOTE: This field is valid only when VLAN0Command!= 0.
     *  bits 117-129 */
    GT_BOOL overwriteExpandedActionTag0VlanId;

    /** @brief tag1 vlan command
     *  This field determines the VLAN1 assigned to the packet
     *  matching the TTI entry. If the packet is TT, the following
     *  applies to the passenger Ethernet packet; otherwise the
     *  following applies to the packet as it was received.
     *  bit 130 */
    GT_BOOL overwriteExpandedActionTag1VlanCmd;

    /** @brief tag1 VLAN
     *  bits 131-142 */
    GT_BOOL overwriteExpandedActionTag1VlanId;

 /** @brief  QoS precedence options
    0x0 = Soft; QoS Parameters may be modified by the following engines.
    0x1 = Hard; QoS Parameters cannot be modified by the
    following engines.
    bit 143 */
    GT_BOOL overwriteExpandedActionQosPrecedence;

    /** @brief QoS profile This field is 7-bit wide although the QoS
     *         mechanism supports 10 bit QoS.
     *  bits 144-150 */
    GT_BOOL overwriteExpandedActionQosProfile;

   /** @brief modify DSCP mode (or EXP for MPLS)
     *  (refer to CPSS_DXCH_TTI_MODIFY_DSCP_ENT)
     *   Controls the different options of DSCP/EXP modifications
     *   0x0 = Keep Previous; Do not modify the previous
     *   0x1 = Enable; Enable packet DSCP/EXP modification
     *   0x2 = Disable; Disable packet DSCP/EXP modification
         bits 151-152 */
    GT_BOOL overwriteExpandedActionModifyDscp;

    /** @brief Control the different options of tag0 UP
       *         modifications
       *  bits 153-154 */
    GT_BOOL overwriteExpandedActionModifyTag0;

    /** @brief Control the different options of QoS Profile assignments.
        0x0 = False; Assign QoS profile based on TTI-AE QoS fields.
        0x1 = True; Keep prior QoS Profile assignment.
        bit 155 */
    GT_BOOL overwriteExpandedActionKeepPreviousQoS;

     /** @brief When set, enables the QoS Profile to be assigned
      *         by the UP2QoS algorithm.
      *  bit 156 */
    GT_BOOL overwriteExpandedActionTrustUp;

    /** @brief When set, enables the QoS Profile to be assigned
     *         by the DSCP2Qos algorithm.
     *  bit 157 */
    GT_BOOL overwriteExpandedActionTrustDscp;

    /** @brief When set, enables the QoS Profile to be assigned
     *         by the EXP2QoS algorithm
     *  bit 158 */
    GT_BOOL overwriteExpandedActionTrustExp;

    /** @brief When set, enables the DSCP to be re-assigned by
     *         DSCP2DSCP algorithm.
     *  bit 159 */
    GT_BOOL overwriteExpandedActionRemapDscp;

    /** @brief  tag0 UP assignment
     *  bits 160-162 */
    GT_BOOL overwriteExpandedActionTag0Up;

    /** @brief tag1 UP command (refer to
     *  CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT) When the packet is TT,
     *  Tag1 identification is based on passenger header.
     *  bits 163-164 */
    GT_BOOL overwriteExpandedActionTag1UpCommand;

    /** @brief tag1 UP assignment
     *  bits 165-167 */
    GT_BOOL overwriteExpandedActionTag1Up;

    /** @brief This field is the tunnel passenger packet type when
     *  either terminating a tunnel (TTI Action Tunnel_Terminate> =
     *  Enabled) or parsing a non-MPLS transit tunnel(TTI Action<
     *  Passenger_Parsing_of_Non-MPLS_Transit_Tunnels> = 1)
     *  This field is not applicable to transit MPLS packets.
     *  bits 168-169 */
    GT_BOOL overwriteExpandedActionTtPassengerPacketType;

    /** @brief If set, the packet's TTL is assigned according to
     *  the tunnel header rather than from passenger packet's
     *  TTL/Hop-Limit.
     *  bit 170 */
    GT_BOOL overwriteExpandedActionCopyTtlExpFromTunnelHeader;

    /** @brief whether the packet's tunnel header is removed;
     *  must not be set for non-tunneled packets
     *  bit 171
     */
    GT_BOOL overwriteExpandedActionTunnelTerminate;

     /** @brief MPLS Action applied to the packet
      *  bits 172-174   */
    GT_BOOL overwriteExpandedActionMplsCommand;

    /** @brief Interface-based CRC hash mask selection.
        This field determines which mask is used in the
        CRC-based hash. A value of 0x0 means that the mask is not
        determined by the TTI action entry.
        bits 175-178 */
    GT_BOOL overwriteExpandedActionHashMaskIndex;

     /** @brief the 'Trust Qos Mapping Table Index'
      *  There are 12 global Trust QoS Mapping Table used for Trust
      *  L2, L3, EXP, DSCP-to-DSCP mapping. This field selects which
      *  of the 12 global sets of Trust QoS Mapping tables is
      *  used.
      *  bits 179-182 */
    GT_BOOL overwriteExpandedActionQosMappingTableIndex;

     /** @brief This is used for supporting MPLS L-LSP QoS, where the
        scheduling traffic class is derived from the MPLS label
        and the drop precedence is derived from the EXP. NOTE: This
        field is only relevant if TTI Action TRUST_EXP is unset,
        and TTI Action Keep_QoS is unset
        0x0 = Disabled;
        0x1 = Enabled; the packet is assigned a QoSProfile that is
        the sum of the TTI Action QoS_Profile + packet outer label
        EXP bit 183 */
    GT_BOOL overwriteExpandedActionMplsLLspQoSProfileEnable;

    /** @brief TTL to be used in MPLS header that is Pushed or
        Swapped according to MPLS Command. If (TTL == 0) the TTL is
        taken from the packet's header, else TTL is set according to
        this field. NOTE: If the original packet does not contain
        TTL and a label is pushed or swapped, this field must be
        assigned a value different than 0. NOTE: This field is relevant
        for MPLS packets that have a match in the TTI lookup or for
        non-MPLS packets when the TTI action MPLS command is PUSH.
        bits 184-191 */
   GT_BOOL overwriteExpandedActionMplsTtl;

    /** @brief If set, the TTL is decremented by one.
     *  This field is relevant for MPLS packet that are not tunnel terminated.
     *  NOTE: When MPLS_Command is NOP, this field should be disabled
     *  bit 192 */
    GT_BOOL overwriteExpandedActionEnableDecrementTtl;

    /** @brief if enabled new source ePort number is taken from
     *  TTI Action Entry Source_ePort>
     *  bit 193 */
    GT_BOOL overwriteExpandedActionSourceEPortAssignmentEnable;

    /** @brief Source ePort number assigned by TTI entry when
        Source_ePort_Assignment_Enable = Enabled
        NOTE: Overrides any previous assignment of source ePort
        number.
        bits 194-207 */
    GT_BOOL overwriteExpandedActionSourceEPort;

    /** @brief This field defines the tunnel header length in resolution of 2B,
     *  where the tunnel header is relative to the TTI Action
     *  Tunnel_Header_Length_Anchor_Type.
     *  If '0' then the TT Header Len is default TT
     *  bits 208-213 */
    GT_BOOL overwriteExpandedActionTtHeaderLength;

    /** @brief When enabled, the TTI Action defines where the
     *  parser should start parsing the PTP header. The location is
     *  defined by TTI Action PTP_Offset
     *  bit 214 */
    GT_BOOL overwriteExpandedActionIsPtpPacket;

    /** @brief  When enabled, it indicates that a timestamp
     *  should be inserted into the packet. relevant when
     *  IsPtpPacket = "0"
     *  bit 215 */
    GT_BOOL overwriteExpandedActionOamTimeStampEnable;

    /** @brief When TTI Action IsPTPPacket is enabled, this field
     *  specifies the PTP Trigger Type.
     *  bits 215-216 */
    GT_BOOL overwriteExpandedActionPtpTriggerType;

    /** @brief When TTI Action IsPTPPacket is enabled, this field
     *  defines the byte offset to start of the PTP header, relative
     *  to the start of the L3 header.
     *  NOTE: This offset resolution is 2 bytes.
     *  relevant when IsPtpPacket = "1"
     *  bits 217-222 */
    GT_BOOL overwriteExpandedActionPtpOffset;

    /** @brief When TTI Action TimestampEnable is enabled, this
      *  field indicates the offset index itno the timestamp offset
      *  table. relevant when IsPtpPacket = "0"
      *  bits 216-222 */
    GT_BOOL overwriteExpandedActionOamOffsetIndex;

    /** @brief When this field is set, and there is a label with
      *        GAL_Value or OAL_Value anywhere in the label stack,
      *        OAM processing is enabled for this packet.
      * bit 223 */
    GT_BOOL overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable;

    /** @brief Enable for OAM Processing
     *  bit 224 */
    GT_BOOL overwriteExpandedActionOamProcessEnable;

        /** @brief When set, indicates that the MPLS packet contains
         *         a control word immediately following the bottom of
         *         the label stack.
         *  bit 225 */
    GT_BOOL overwriteExpandedActionCwBasedPw;

    /** @brief Enables TTL expiration exception command
     *         assignment for Pseudo Wire
     *  bit 226 */
    GT_BOOL overwriteExpandedActionTtlExpiryVccvEnable;

    /** @brief When set, indicates that this MPLS stack includes
     *         a flow (entropy) label at the bottom of the stack.
     *         The PW label is one label above the bottom of the
     *         label stack.
     *  bit 227 */
    GT_BOOL overwriteExpandedActionPwe3FlowLabelExist;

        /** @brief When set, indicates that this PW-CW supports
         *         E-Tree Root/Leaf indication via Control Word L bit
         *  bit 228 */
    GT_BOOL overwriteExpandedActionPwCwBasedETreeEnable;

    /** @brief The mode to select one of 12 mapping tables.
      *  0x0 = according to "Trust QoS Mapping Table Select";
      *  The table is selected  based on the TTI Action
      *  TrustQoSMappingTableSelect>;
      *  0x1 = according to packet UP;The table is selected based on
      *  the packet VLAN tag UP. In this case, the table selected is
      *  in the range of 0-7. Based on ePort configuration, the
      *  packet UP used can be taken from either Tag0 or Tag1.
         bit 229  */
    GT_BOOL overwriteExpandedActionQosUseUpAsIndexEnable;

    /** @brief When protection switching is enabled on this flow,
     *  this field indicates whether the received packet arrived on
     *  the working or protection path.
     *  bit 230 */
    GT_BOOL overwriteExpandedActionRxIsProtectionPath;

    /** @brief Enables Rx protection switching filtering.
     *  bit 231 */
    GT_BOOL overwriteExpandedActionRxProtectionSwitchEnable;

    /** @brief Override MAC2ME check and set to 1.
     *  bit 232  */
    GT_BOOL overwriteExpandedActionSetMacToMe;

    /** @brief The OAM profile determines the set of IPCL key UDBs
      * used to contain the OAM message attributes
      * (opcode, MEG level, RDI, MEG level).
      * bit 233 */
    GT_BOOL overwriteExpandedActionOamProfile;

        /** @brief Determines whether to apply the non-data CW packet
         *  command to the packet with non-data CW
         *  bit 234 */
    GT_BOOL overwriteExpandedActionApplyNonDataCwCommand;

    /** @brief Controls whether to continue with the next TTI Lookup
     *  bit 237 */
    GT_BOOL overwriteExpandedActionContinueToNextTtiLookup;

    /** @brief When enabled, the MPLS G-ACh Channel Type is
     *  mapped to an OAM Opcode that is used by the OAM engine
     *  bit 238 */
    GT_BOOL overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable;

    /** @brief This feature enables passenger parsing of transit MPLS tunnels.
      * This configuration is relevant only IF Packet is MPLS, AND
      * TTI Action TunnelTerminate = Disabled, AND Legacy global
      * EnableL3L4ParsingoverMPLS is disabled
      * bits 239-240   */
    GT_BOOL overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode;

    /** @brief This is a generic mechanism to parse the passenger
     *  of transit tunnel packets other than MPLS, for
     *  example, IP-based tunnels, MiM tunnels, etc.
     *  bit 241 */
    GT_BOOL overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable;

     /** @brief Enables skipping the FDB SA Lookup:
     * 0: If ingress port is network port then don't skip the SA
     * Lookup. If the ingress port is eDSA Cascade port then don't
     * modify the state received in eDSA SkipFdbSALookup field.
     * 1: Skip the FDB SA lookup
     * bit 242 */
    GT_BOOL overwriteExpandedActionSkipFdbSaLookupEnable;

    /** @brief Sets that this Device is an End Node of IPv6 Segment Routing
     *  bit 243 */
    GT_BOOL overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable;

    /** @brief The Anchor Type used by the TTI Action
     *  TunnelHeaderLength
       '0': L3 Anchor - TTI Action TunnelHeaderLength is relative to
        the start of the L3 header.
       '1': L4 Anchor - TTI Action TunnelHeaderLength is relative
        to the start of the L4 header.
        bit 244 */
    GT_BOOL overwriteExpandedActionTunnelHeaderStartL4Enable;

}CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC
*
*  @brief EXACT MATCH use PCL TCAM rule action parameters.
*
*         Determines whether the associated byte is assigned
*         from the Exact Match reduces Action, or assigned a
*         default value.
*         GT_FALSE = Assigned default value
*         GT_TRUE = Assigned from Action bytes of the exact
*         match Entry
*
*         The overwritten mean field will be taken from reduced
*         Action (aka replacing value at expended action)
*/
typedef struct{

    /** @brief If ActionCommand = TRAP or MIRROR or SOFT/HARD DROP,
     *         this code is the CPU/DROP Code passed to the CPU as
     *         part of the packet.
     *  bits 0-7 for IPCL
     *  bits 170-177 for EPCL */
    GT_BOOL     overwriteExpandedActionUserDefinedCpuCode;

    /** @brief Packet commands assigned to packets matching this
     *  rule (forward, mirror hard-drop, soft-drop, or trap-to-cpu)
     *  bits 8-10 for IPCL
     *  bits 0-2 for EPCL
     */
    GT_BOOL overwriteExpandedActionPktCmd;

    /** @brief where to redirect the packet
     *  0x0 = NoRedirect; Do not redirect this packet.
     *  0x1 = Egress Interface; Redirect this packet to the
     *  configured Egress Interface.
     *  0x2 = IPNextHop; Policy-based routing: assign index to
     *  Router Lookup Translation Table (LTT).
     *  0x4 = Assign VRF-ID; Assign VRF-ID from the TTI Action
     *  entry. The packet is not redirected.
        bits 11-13 IPCL only */
    GT_BOOL overwriteExpandedActionRedirectCommand;

    /** @brief egress interface when
     *           redirect command = "Egress Interface"
     * bits 17 - target is lag
     * bits 17-32 eVidx
     * bits 18-29 trunk ID
     * bits 18-31 target ePort
     * bits 32-41 target device
     * bit 16 useVidx IPCL only */
    GT_BOOL overwriteExpandedActionEgressInterface;

    /** @brief this field set the Expanded action for MAC SA
     *  The MAC SA that is sent to the bridge engine when
     *  RedirectCommand == Replace MACSA
     *  NOTE: This field is muxed with a set of other IPCL-AE fields
     *  (Egress interface fields Policer Index)
     *  bits 16-43 macSA[27:0]
     *  bits 62-81 macSA[47:28] IPCL only */
    GT_BOOL overwriteExpandedActionMacSa;

    /** @brief The IP Next Hop Entry Index when redirect command =
     *  "IP Next Hop"
     *  bits 33-50
     *  IPCL only
     *  SIP6_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E */
    GT_BOOL overwriteExpandedActionRouterLttPtr;

    /** @brief If set, this a network testing flow.
     *  Therefore the packet should be redirected to an
     *  egress interface where prior to transmission the packet's
     *  MAC SA and MAC DA are switched.
     *  NOTE: This should always be accompanied with
     *  Redirect Command = 1 (Policy Switching).
     *  bit 42 VNTL2Echo
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E   */
    GT_BOOL overwriteExpandedActionVntl2Echo;

    /** @brief When set, packet is redirected to a tunnel start
     *         entry pointed by Tunnel_Index
     * NOTE: This field is valid only when Redirect Command =
     * Redirect to Egress Interface (Policy-switching)
     * bit 43
     * IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E   */
    GT_BOOL overwriteExpandedActionTunnelStart;

    /** @brief This field is valid only when
     *  Redirect_Command="Egress_Interface" and
     *  TunnelStart="False"
     *  bits 44-61
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E   */
    GT_BOOL overwriteExpandedActionArpPtr;

    /** @brief This field is valid only when
     *  Redirect_Command="Egress_Interface" and TunnelStart="True"
     *  bits 44-59
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E */
    GT_BOOL overwriteExpandedActionTunnelIndex;

    /** @brief Type of passenger packet being to be encapsulated.
     *   This field is valid only when Redirect Command = 1 (Policy-switching)
     *   and TunnelStart = 1 When packet is not TT and MPLS_Command != NOP,
     *   the Tunnel_Type bit must be set to OTHER.
     *   0x0 = Ethernet; 0x1 = Other;
     *   bit 60 IPCL only
     *    SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E */
    GT_BOOL overwriteExpandedActionTsPassengerPacketType;

    /** @brief vrf-id
     *  bits 52-63 VRF-ID
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E */
    GT_BOOL overwriteExpandedActionVrfId;

    /** @brief Action Stop
     *  GT_TRUE - to skip the following PCL lookups
     *  GT_FALSE - to continue with following PCL lookups
     *  Relevant to Policy Action Entry only.
     *  bit 14 - IPCL only   */
    GT_BOOL overwriteExpandedActionActionStop;

    /** @brief Override MAC2ME check and set to 1
     *  0x0 = Do not override Mac2Me mechanism;
     *  0x1 = Set Mac2Me to 1; Override the Mac2Me mechanism and set
     *  Mac2Me to 0x1;
     *  bit 15 - IPCL only  */
    GT_BOOL overwriteExpandedActionSetMacToMe;

    /** @brief see CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC
     *  bit 16 IPCL1 override Config Index
     *  - IPCL only */
    GT_BOOL overwriteExpandedActionPCL1OverrideConfigIndex;

    /** @brief see CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC
     *  bit 17 PCL2 override Config Index
     *  - IPCL only */
    GT_BOOL overwriteExpandedActionPCL2OverrideConfigIndex;

    /** @brief see CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC
     *   bit 18-30 IPC Configuration Index
    *  - IPCL only */
    GT_BOOL overwriteExpandedActionIPCLConfigurationIndex;

    /** @brief packet Policing configuration
     *  see CPSS_DXCH_PCL_ACTION_POLICER_STC
     *  for IPCL: bit 64 bindToPolicerMeter
     *            bit 65 bindToPolicerCounter
     *  for EPCL: bit 62 bindToPolicerCounter
     *            bit 63 bindToPolicerMeter  */
    GT_BOOL overwriteExpandedActionPolicerEnable;

    /** @brief packet Policing configuration
     *  see CPSS_DXCH_PCL_ACTION_POLICER_STC
     *  for IPCL: bits 66-79
     *  for EPCL: bits 64-77  */
    GT_BOOL overwriteExpandedActionPolicerIndex;

    /** @brief packet mirroring configuration
     *  see CPSS_DXCH_PCL_ACTION_MIRROR_STC
     *   for EPCL: bit 165-166 -Egress Mirroring Mode */
     GT_BOOL overwriteExpandedActionMirrorMode;

    /** @brief packet mirroring configuration
     *  see CPSS_DXCH_PCL_ACTION_MIRROR_STC
     *  for IPCL only :bit 82 -Enable Mirror TCP RST/FIN */
     GT_BOOL overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu;

    /** @brief packet mirroring configuration
     *  see CPSS_DXCH_PCL_ACTION_MIRROR_STC
     *  for IPCL: bits 83-85 - Mirror To Analyzer Port
     *  for EPCL: bits 167-169 -Egress Analyzer Index */
     GT_BOOL overwriteExpandedActionMirror;

    /** @brief this field set the Expanded action modify MAC DA
     *  bits 86
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E   */
    GT_BOOL overwriteExpandedActionModifyMacDA;

    /** @brief this field set the Expanded action modify MAC SA
     *  bits 87
     *  IPCL only SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E   */
    GT_BOOL overwriteExpandedActionModifyMacSA;

    /** @brief the Bridge engine processed or bypassed
     *  GT_TRUE - the Bridge engine is bypassed.
     *  GT_FALSE - the Bridge engine is processed.
     *  NOTE: This field should be set if this packet if <Redirect
     *  Command> = Redirect to Egress Interface
     *  bit 88 - IPCL only  */
    GT_BOOL overwriteExpandedActionBypassBridge;

    /** @brief the ingress pipe bypassed or not.
     *  GT_TRUE - the ingress pipe is bypassed.
     *  GT_FALSE - the ingress pipe is not bypassed.
     *  NOTE: This field should be set if this packet if <Redirect
     *  Command> = Redirect to Egress Interface.
     *  bit 89 - IPCL only  */
    GT_BOOL overwriteExpandedActionBypassIngressPipe;

    /** @brief packet VLAN modification configuration
    *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
    *  for IPCL Only:
    *  bit 90 -Enable Nested VLAN */
    GT_BOOL overwriteExpandedActionNestedVlanEnable;

    /** @brief packet VLAN modification configuration
    *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
    *  for IPCL Only:
    *  bit 120 VLANPrecedence */
    GT_BOOL overwriteExpandedActionVlanPrecedence;

    /** @brief packet VLAN modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for IPCL: bit 121-122 VLANCommand
     *  for EPCL: bit 48-49 tag0 VID cmd */
    GT_BOOL overwriteExpandedActionVlan0Command;

    /** @brief packet VLAN modification configuration
    *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
    *  for IPCL: 123-135 VID0
    *  for EPCL: 50-61 tag0 VID */
    GT_BOOL overwriteExpandedActionVlan0;

    /** @brief packet VLAN modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for IPCL: 152-153 vlan1_cmd
     *  for EPCL: 31 tag1 VID cmd */
    GT_BOOL overwriteExpandedActionVlan1Command;

    /** @brief packet VLAN modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for IPCL: 156-167 VID1
     *  for EPCL: 33-44 tag1 VID */
    GT_BOOL overwriteExpandedActionVlan1;

    /** @brief packet source Id assignment
     *  see CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC,
     *  bit 91 -Source-ID Assignment Enable
     * - IPCL only */
    GT_BOOL overwriteExpandedActionSourceIdEnable;

    /** @brief packet source Id assignment
     *  see CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC
     *  bits 92-103 Source-ID
     * - IPCL only */
    GT_BOOL overwriteExpandedActionSourceId;

    /** @brief match counter configuration
     *  see CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC
     * for IPCL:
     * bit 105 - Bind To CNC Counter
     * bits 106-119 - CNC Counter Index
     * for EPCL:
     * bit 16 - Bind To CNC Counter
     * bits 17-30 - CNC Counter Index */
     GT_BOOL overwriteExpandedActionMatchCounterEnable;

     /** @brief match counter configuration
      *  see CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC
      * for IPCL:
      * bit 105 - Bind To CNC Counter
      * bits 106-119 - CNC Counter Index
      * for EPCL:
      * bit 16 - Bind To CNC Counter
      * bits 17-30 - CNC Counter Index */
     GT_BOOL overwriteExpandedActionMatchCounterIndex;

    /** @brief packet QoS attributes
     *  see CPSS_DXCH_PCL_ACTION_QOS_UNT
     * for IPCL only: bit 136  */
    GT_BOOL overwriteExpandedActionQosProfileMakingEnable;

    /** @brief packet QoS attributes
    *  see CPSS_DXCH_PCL_ACTION_QOS_UNT
    * for IPCL only: bit 137
    */
    GT_BOOL overwriteExpandedActionQosPrecedence;

    /** @brief packet QoS attributes
     *  see CPSS_DXCH_PCL_ACTION_QOS_UNT
     * for IPCL only: bit 138-147  */
    GT_BOOL overwriteExpandedActionQoSProfile;

    /** @brief packet QoS attributes
     *  see CPSS_DXCH_PCL_ACTION_QOS_UNT
     * for IPCL : bit 148-149 ModifyDSCP
     * for EPCL: bits 12-13 modify dscp/exp */
    GT_BOOL overwriteExpandedActionQoSModifyDSCP;

    /** @brief packet UP1 modification configuration
     *  see CPSS_DXCH_PCL_ACTION_QOS_UNT
     *  IPCL only : 154-155 UP command */
    GT_BOOL overwriteExpandedActionUp1Command;

    /** @brief packet UP1 modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for IPCL: 150-151 modify UP1
     *  for EPCL: bit 32 modify UP1 */
    GT_BOOL overwriteExpandedActionModifyUp1;

    /** @brief packet UP1 modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for IPCL: 168-170 UP1
     *  for EPCL: 45-47 UP1 */
    GT_BOOL overwriteExpandedActionUp1;

    /** @brief packet DSCP/EXP modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for EPCL only: bit 3-8 dscp/exp */
    GT_BOOL overwriteExpandedActionDscpExp;

    /** @brief packet UP0 modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for EPCL only: 9-11 UP0 */
    GT_BOOL overwriteExpandedActionUp0;

    /** @brief packet UP0 modification configuration
     *  see CPSS_DXCH_PCL_ACTION_VLAN_UNT
     *  for EPCL only: 14-15 modify Up0  */
    GT_BOOL overwriteExpandedActionModifyUp0;

     /** @brief OAM configuration Timestamp Enable
      *  see CPSS_DXCH_PCL_ACTION_OAM_STC
      *  for IPCL:bit 192
      *  for EPCL: bit 93 */
     GT_BOOL overwriteExpandedActionOamTimestampEnable;

     /** @brief OAM configuration offset_index
     *  see CPSS_DXCH_PCL_ACTION_OAM_STC
     * for IPCL:bit 193-199
     * for EPCL: bits 94-100 */
     GT_BOOL overwriteExpandedActionOamOffsetIndex;

     /** @brief OAM configuration processing Enable
      *  see CPSS_DXCH_PCL_ACTION_OAM_STC
      *  for IPCL: bit 200
      *  for EPCL: bit 91 */
     GT_BOOL overwriteExpandedActionOamProcessingEnable;

     /** @brief OAM configuration Profile
      *  see CPSS_DXCH_PCL_ACTION_OAM_STC
      * for IPCL:201 OAM Profile
      * for EPCL:92 OAM Profile  */
     GT_BOOL overwriteExpandedActionOamProfile;

     /** @brief OAM configuration Channel Type to Opcode Mapping En
      * When enabled, the MPLS G-ACh Channel Type is mapped
      * to an OAM Opcode that is used by the OAM engine.
      * for EPCL only: bit 101  */
     GT_BOOL overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable;

    /** @brief The flow ID assigned to the packet
     *  (the value in this field is assigned into Desc "FlowID").
     *  The value 0x0 represents do not assign Flow ID. The PCL
     *  overrides the existing Flow ID value if and only if this
     *  field in the PCL action is non-zero.
     *  bit 202-214 for IPCL
     *  bit 78-90 for EPCL
     */
    GT_BOOL overwriteExpandedActionFlowId;

    /** @brief source ePort Assignment.
     *  see CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC
     *  bit 216 - Assign Source ePort Enable
     * - IPCL only */
    GT_BOOL overwriteExpandedActionSourcePortEnable;

    /** @brief source ePort Assignment.
     *  see CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC,
     *  bits 217-230 Source ePort
     * - IPCL only */
    GT_BOOL overwriteExpandedActionSourcePort;

    /** @brief Latency monitoring
     *  see CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC
     * for IPCL: bits 231-239
     * for EPCL: bits 178-186  */
    GT_BOOL overwriteExpandedActionLatencyMonitor;

    /** @brief Latency monitoring Enable
     *  see CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC
     * for IPCL: bit 240
     * for EPCL: bit 187 */
    GT_BOOL overwriteExpandedActionLatencyMonitorEnable;

    /** @brief Enables skipping the FDB SA Lookup:
     * 0: Don't modify the state of Skip FDB SA lookup
     * 1: Skip the FDB SA lookup bit 241 - IPCL only */
    GT_BOOL overwriteExpandedActionSkipFdbSaLookup;

    /** @brief When set, the IPCL will trigger a dedicated interrupt
     *  towards the CPU
     *  bit 242 - IPCL only  */
    GT_BOOL overwriteExpandedActionTriggerInterrupt;

    /** @brief Enable assignment of PHA Metadata:
     *  bit 123 - EPCL only  */
    GT_BOOL overwriteExpandedActionPhaMetadataAssignEnable;

    /** @brief PHA Metadata assigned
     *  bit 124-155 - EPCL only
     */
    GT_BOOL overwriteExpandedActionPhaMetadata;

    /** @brief Enable assignment of PHA Thread Number
     *  bit 156 - EPCL only */
    GT_BOOL overwriteExpandedActionPhaThreadNumberAssignEnable;

    /** @brief PHA Thread number assigned
     *  bit 157-164 - EPCL only  */
    GT_BOOL overwriteExpandedActionPhaThreadNumber;

    /** @brief This field indicates the RX that this packet should be
     *  treated as Store and Forward in the same manner Cut through
     *  Slow to Fast are treated.
     *  bit 188 - EPCL only  */
    GT_BOOL overwriteExpandedActionCutThroughTerminateId;

}CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC;

/**
 * @enum CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT
 *
 * @brief Exact Match Expanded Action Origin
 * APPLICABLE DEVICES: Falcon.
 *
*/
typedef union{
    /** Exact Match action is TTI action (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC   ttiExpandedActionOrigin;

    /** Exact Match action is PCL action(APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC   pclExpandedActionOrigin;

} CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT;

/**
 * @enum CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT
 *
 * @brief Exact Match lookup number
 * APPLICABLE DEVICES: Falcon.
 *
*/
typedef enum{
    /** Exact Match first lookup (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E,

    /** Exact Match second lookup (APPLICABLE DEVICES: Falcon) */
    CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E,

    CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E

} CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT;

/**
* @struct CPSS_EXACT_MATCH_KEY_STC
 *
 * @brief Exact Match key
*/
typedef struct{

    /** @brief key Size */
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT keySize;

    GT_U8 pattern[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];

} CPSS_DXCH_EXACT_MATCH_KEY_STC;

/**
* @struct CPSS_EXACT_MATCH_ENTRY_STC
 *
 * @brief Exact Match Entry
*/
typedef struct{

    /** @brief key Size */
    CPSS_DXCH_EXACT_MATCH_KEY_STC         key;

    /** @brief lookup number */
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT lookupNum;

} CPSS_DXCH_EXACT_MATCH_ENTRY_STC;

/**
* @internal cpssDxChExactMatchTtiProfileIdSet function
* @endinternal
*
* @brief   Sets the Exact Match Profile Id for TTI keyType
*
* NOTE: Client lookup for given ttiLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum         - device number
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[in] enableExactMatchLookup- enable Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[in] profileId    - Exact Match profile identifier
*                           (APPLICABLE RANGES: 1..15)
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_OUT_OF_RANGE       - parameter not in valid range.
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: legacy key type not supported:
*       CPSS_DXCH_TTI_KEY_IPV4_E,
*       CPSS_DXCH_TTI_KEY_MPLS_E,
*       CPSS_DXCH_TTI_KEY_ETH_E,
*       CPSS_DXCH_TTI_KEY_MIM_E
*
*/
GT_STATUS cpssDxChExactMatchTtiProfileIdSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum,
    IN  GT_BOOL                             enableExactMatchLookup,
    IN  GT_U32                              profileId
);

/**
* @internal cpssDxChExactMatchTtiProfileIdGet function
* @endinternal
*
* @brief   Gets the  Exact Match Profile Id form TTI keyType
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum         - device number
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[out] profileIdPtr  - (pointer to) Exact Match profile
*                             identifier
*                           (APPLICABLE RANGES: 1..15)
* @retval GT_OK                  - on success
* @retval GT_HW_ERROR            - on hardware error
* @retval GT_OUT_OF_RANGE        - parameter not in valid range.
* @retval GT_BAD_PARAM           - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChExactMatchTtiProfileIdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum,
    OUT GT_BOOL                             *enableExactMatchLookupPtr,
    OUT GT_U32                              *profileIdPtr
);

/**
* @internal cpssDxChExactMatchPclProfileIdSet function
* @endinternal
*
* @brief  Sets the Exact Match Profile Id for PCL packet type
*
* NOTE: Client lookup for given pclLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                               Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress
* @param[in] packetType               - PCL packet type
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: 0..7)
* @param[in] pclLookupNum             - pcl lookup number
* @param[in] enableExactMatchLookup- enable Exact Match lookup
*                           GT_TRUE: trigger Exact Match Lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[in] profileId    - Exact Match profile identifier
*       (APPLICABLE RANGES: 1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchPclProfileIdSet
(
   IN  GT_U8                               devNum,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   IN  GT_BOOL                             enableExactMatchLookup,
   IN  GT_U32                              profileId
);

/**
* @internal cpssDxChExactMatchPclProfileIdGet function
* @endinternal
*
* @brief  Gets the Exact Match Profile Id form PCL packet type
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                               Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] packetType               - PCL packet type
* @param[in] direction                - ingress/egress
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: 0..7)
* @param[in] pclLookupNum             - pcl lookup number
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger Exact Match Lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[in] profileIdPtr - (pointer to)Exact Match profile
*       identifier (APPLICABLE RANGES: 1..15)
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchPclProfileIdGet
(
   IN  GT_U8                               devNum,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   OUT GT_BOOL                             *enableExactMatchLookupPtr,
   OUT GT_U32                              *profileIdPtr
);

/**
* @internal cpssDxChExactMatchClientLookupSet function
* @endinternal
*
* @brief   Set global configuration to determine the client of
*          the first Exact Match lookup and the client of the
*          second Exact Match lookup.
*
*   NOTE: this API should be called before configuring
*      Exact Match Profile Id for TTI keyType (cpssDxChExactMatchTtiProfileIdSet)or
*      Exact Match Profile Id for PCL/EPCL packet type (cpssDxChExactMatchPclProfileIdSet)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum               - the device number
* @param[in] exactMatchLookupNum  - exact match lookup number
* @param[in] clientType           - client type (TTI/EPCL/IPCL)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChExactMatchClientLookupSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    IN  CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientType
);

/**
* @internal cpssDxChExactMatchClientLookupGet function
* @endinternal
*
* @brief   Get global configuration that determine the client of
*          the first Exact Match lookup and the client of the
*          second Exact Match lookup.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in]  devNum               - the device number
* @param[in]  exactMatchLookupNum  - exact match lookup number
* @param[out] clientTypePtr        - (pointer to) client type
*                                   (TTI/EPCL/IPCL)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChExactMatchClientLookupGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    OUT CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            *clientTypePtr
);

/**
* @internal cpssDxChExactMatchActivityBitEnableSet function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum              - device number
* @param[in] enable              - GT_TRUE - enable refreshing
*                                  GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchActivityBitEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChExactMatchActivityBitEnableGet function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] enablePtr          - (pointer to)
* @param[in] enablePtr          - (pointer to)
*                               GT_TRUE - enable refreshing
*                               GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchActivityBitEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChExactMatchActivityStatusGet function
* @endinternal
*
* @brief   Return the Exact Match activity bit for a given
*          entry.The bit is set by the device when the entry is
*          matched in the Exact Match Lookup. The bit is reset
*          by the CPU as part of the aging process.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in]  devNum             - device number
* @param[in] portGroupsBmp       - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in]  exactMatchEntryIndex - Exact Match entry index
* @param[in]  exactMatchClearActivity - set activity bit to 0
* @param[out] exactMatchActivityStatusPtr  - (pointer to)
*                  GT_FALSE = Not Refreshed; next_age_pass;
*                  Entry was matched since the last reset;
*                  GT_TRUE = Refreshed; two_age_pass; Entry was
*                  not matched since the last reset;
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum 
* @retval GT_NOT_FOUND             - on entry not found 
* @retval GT_OUT_OF_RANGE          - on exactMatchEntryIndex out of range
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchActivityStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  exactMatchEntryIndex,
    IN  GT_BOOL                 exactMatchClearActivity,
    OUT GT_BOOL                 *exactMatchActivityStatusPtr
);

/**
* @internal cpssDxChExactMatchProfileKeyParamsSet function
* @endinternal
*
* @brief   Sets the Exact Match Profile key parameters
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match
*            profile identifier (APPLICABLE RANGES: 1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchProfileKeyParamsSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC     *keyParamsPtr
);


/**
* @internal cpssDxChExactMatchProfileKeyParamsGet function
* @endinternal
*
* @brief   Gets the Exact Match Profile key parameters
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                 identifier (APPLICABLE RANGES:1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchProfileKeyParamsGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          exactMatchProfileIndex,
    OUT CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    *keyParamsPtr
);

/**
* @internal cpssDxChExactMatchProfileDefaultActionSet function
* @endinternal
*
* @brief   Sets the default Action in case there is no match in the
*          Exact Match lookup and in the TCAM lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                    identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[in] actionPtr          - (pointer to)Exact Match Action
* @param[in] defActionEn        - Enable using Profile
*                               Table default Action in case
*                               there is no match in the Exact
*                               Match lookup and in the TCAM
*                               lookup
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchProfileDefaultActionSet
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    IN GT_BOOL                                  defActionEn
);

/**
* @internal cpssDxChExactMatchProfileDefaultActionGet function
* @endinternal
*
* @brief   Gets the default Action in case there is no match in
*          the Exact Match lookup and in the TCAM lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                     identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[out] actionPtr         - (pointer to)Exact Match Action
* @param[out] defActionEnPtr    - (pointer to) Enable using
*                               Profile Table default Action in
*                               case there is no match in the
*                               Exact Match lookup and in the
*                               TCAM lookup
*
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_BAD_PTR            - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchProfileDefaultActionGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   exactMatchProfileIndex,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    OUT GT_BOOL                                  *defActionEnPtr
);

/**
* @internal cpssDxChExactMatchExpandedActionSet function
* @endinternal
*
* @brief  Sets the expanded action for Exact Match Profile. The
*         API also sets for each action attribute whether to
*         take it from the Exact Match entry action or from
*         the Exact Match profile
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex - Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] actionPtr         - (pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: This API must be called before configuration of
*         Exact Match Entry by API cpssDxChExactMatchEntrySet
*
*         The Expander Action Entry cannot be changed if an
*         Exact Match Rule is using this entry
*/
GT_STATUS cpssDxChExactMatchExpandedActionSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT            actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 *actionPtr,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT *expandedActionOriginPtr
);

/**
* @internal cpssDxChExactMatchExpandedActionGet function
* @endinternal
*
* @brief   Gets the action for Exact Match in case of a match in
*          Exact Match lookup The API also sets for each action
*          attribute whether to take it from the Exact Match
*          entry action or from the Exact Match profile
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex -Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[out]actionPtr         -(pointer to)Exact Match Action
* @param[out]expandedActionOriginPtr - (pointer to) Whether to
*                   use the action attributes from the Exact
*                   Match rule action or from the profile
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchExpandedActionGet
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    *expandedActionOriginPtr
);

/**
* @internal cpssDxChExactMatchPortGroupEntrySet function
* @endinternal
*
* @brief   Sets the exact match entry and its action
*
*   NOTE: this API should be called when there is a valid entry
*   in expandedActionProfile in the Profile Expander Table
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index - entry index in the exact match table
* @param[in] expandedActionIndex-Exact Match profile identifier
*                     (APPLICABLE RANGES:1..15)
* @param[in] entryPtr      - (pointer to)Exact Match entry
* @param[in] actionType    - Exact Match Action Type(TTI or PCL)
* @param[in] actionPtr     - (pointer to)Exact Match Action (TTI
*                            Action or PCL Action)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchPortGroupEntrySet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index,
    IN GT_U32                                   expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC          *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr
);

/**
* @internal cpssDxChExactMatchPortGroupEntryGet function
* @endinternal
*
* @brief   Gets the exact match entry and its action
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                 NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] index - entry index in the exact match table
* @param[out]validPtr   - (pointer to) is the entry valid or not
* @param[out] actionType - Exact Match Action Type(TTI or PCL)
* @param[out] actionPtr - (pointer to)Exact Match Action
*                         (TTI Action or PCL Action)
* @param[out] entryPtr  - (pointer to)Exact Match entry
* @param[out] expandedActionIndexPtr-(pointer to)Exact Match
*             profile identifier (APPLICABLE RANGES:1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchPortGroupEntryGet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index,
    OUT GT_BOOL                                 *validPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   *actionTypePtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT        *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ENTRY_STC         *entryPtr,
    OUT GT_U32                                  *expandedActionIndexPtr
);

/**
* @internal cpssDxChExactMatchPortGroupEntryInvalidate function
* @endinternal
*
* @brief   Invalidate the exact match entry
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index - entry index in the Exact Match table
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchPortGroupEntryInvalidate
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN GT_U32                               index
);


/**
* @internal cpssDxChExactMatchHashCrcMultiResultsByParamsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the Exact Match table).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                           - device number
* @param[in] entryKeyPtr                      - (pointer to) entry key
* @param[in] numberOfElemInCrcMultiHashArrPtr - (pointer to) number of valid
*                                                elements in the exactMatchCrcMultiHashArr
* @param[out] exactMatchCrcMultiHashArr[]     - (array of) 'multi hash' CRC results.
*                                               index in this array is entry inside the bank
*                                               + bank Id'
*                                               size of exactMatchCrcMultiHashArr must be 16
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: the function doesn't access the HW,
*       and do only SW calculations.
*
* The output of the function is indexes that should be
* used for setting the Exact Match entry with cpssDxChExactMatchPortGroupEntrySet.
* Entry with Key size 5B  occupy 1 entry
* Entry with Key size 19B occupy 2 entries
* Entry with Key size 33B occupy 3 entries
* Entry with Key size 47B occupy 4 entries
*
* The logic is as follow according to the key size and bank number.
* keySize   exactMatchSize    function output
* 5 bytes     4 banks         x,y,z,w      (4 separate indexes)
* 19 bytes    4 banks         x,x+1,y,y+1  (2 indexes to be used in pairs)
* 33 bytes    4 banks         x,x+1,x+2    (1 indexes to be used for a single entry)
* 47 bytes    4 banks         x,x+1,x+2,x+3(1 indexes to be used for a single entry)
*
* keySize   exactMatchSize    function output
* 5 bytes     8 banks         x,y,z,w,a,b,c,d            (8 separate indexes)
* 19 bytes    8 banks         x,x+1,y,y+1,z,z+1,w,w+1    (4 indexes to be used in pairs)
* 33 bytes    8 banks         x,x+1,x+2,y,y+1,y+2        (2 indexes to be used for a single entry)
* 47 bytes    8 banks         x,x+1,x+2,x+3,y,y+1,y+2,y+3(2 indexes to be used for a single entry)
*
* and so on for 16 banks
*
* The cpssDxChExactMatchPortGroupEntrySet should get only the first index to be used in the API
* The API set the consecutive indexes according to the key size
*
* keySize   exactMatchSize    function input
* 5 bytes     4 banks         x or y or z or w(4 separate indexes options)
* 19 bytes    4 banks         x or y          (2 indexes options)
* 33 bytes    4 banks         x               (1 indexes option)
* 47 bytes    4 banks         x               (1 indexes option)
*
* keySize   exactMatchSize    function output
* 5 bytes     8 banks         x or y or z or w or a or b or c or d(8 separate indexes options)
* 19 bytes    8 banks         x or y or z or w(4 indexes options)
* 33 bytes    8 banks         x or y          (2 indexes options)
* 47 bytes    8 banks         x or y          (2 indexes options)
*
*/
GT_STATUS cpssDxChExactMatchHashCrcMultiResultsByParamsCalc
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_STC   *entryKeyPtr,
    OUT GT_U32                          *numberOfElemInCrcMultiHashArrPtr,
    OUT GT_U32                          exactMatchCrcMultiHashArr[]
);

/**
* @internal cpssDxChExactMatchRulesDump function
* @endinternal
*
* @brief    Debug API - Dump all valid Exact Match rules
*
* @note     APPLICABLE DEVICES:  Falcon.
*
* @note     NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                          bitmap must be set with at least one bit representing
*                                          valid port group(s). If a bit of non valid port group
*                                          is set then function returns GT_BAD_PARAM.
*                                          value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] startIndex               - index of first rule
* @param[in] rulesAmount              - number of rules to scan and dump
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChExactMatchRulesDump
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             startIndex,
    IN GT_U32                             rulesAmount
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChExactMatchh */

