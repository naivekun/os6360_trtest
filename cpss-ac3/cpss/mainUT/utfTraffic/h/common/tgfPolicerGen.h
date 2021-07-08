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
* @file tgfPolicerGen.h
*
* @brief Generic API for Ingress Policing Engine API.
*
* @version   24
********************************************************************************
*/
#ifndef __tgfPolicerGenh
#define __tgfPolicerGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfCommon.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
    #include <cpss/dxCh/dxCh3/policer/cpssDxCh3Policer.h>
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/policer/cpssExMxPmPolicer.h>
#endif /* EXMXPM_FAMILY */


/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_POLICER_COLOR_COUNT_MODE_ENT
 *
 * @brief Enumeration for Color Counting mode.
*/
typedef enum{

    /** @brief Color Counting is done according
     *  to the packet's Conformance Level.
     */
    PRV_TGF_POLICER_COLOR_COUNT_CL_E,

    /** @brief Color Counting is done according
     *  to the packet's Drop Precedence.
     */
    PRV_TGF_POLICER_COLOR_COUNT_DP_E

} PRV_TGF_POLICER_COLOR_COUNT_MODE_ENT;

/**
* @enum PRV_TGF_POLICER_BILLING_CNTR_MODE_ENT
 *
 * @brief Billing Counters Mode. This enumeration controls the Billing
 * counters resolution.
*/
typedef enum{

    /** 1 Byte resolution. */
    PRV_TGF_POLICER_BILLING_CNTR_1_BYTE_E,

    /** @brief 16 Byte resolution.
     *  In 16 Byte resolution,
     *  Billing counter is rounded
     *  down, i.e. a 64 Billing
     *  counter packet is counted as
     *  4 while a 79 Billing counter
     *  packet is counted as 4 as
     *  well.
     */
    PRV_TGF_POLICER_BILLING_CNTR_16_BYTES_E,

    /** packet resolution. */
    PRV_TGF_POLICER_BILLING_CNTR_PACKET_E

} PRV_TGF_POLICER_BILLING_CNTR_MODE_ENT;

/**
* @enum PRV_TGF_POLICER_METER_RESOLUTION_ENT
 *
 * @brief Packet/Byte Based Meter resolution.
 * This enumeration controls the metering algorithm resolution:
 * packets per second or Bytes per second.
*/
typedef enum{

    /** Byte based Meter resolution. */
    PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E,

    /** @brief Packet based Meter
     *  resolution.
     */
    PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E

} PRV_TGF_POLICER_METER_RESOLUTION_ENT;

/**
* @enum PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_ENT
 *
 * @brief Enumeration for Management counters resolution.
*/
typedef enum{

    /** 1 Byte resolution. */
    PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_1B_E,

    /** 16 Byte resolution. */
    PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_16B_E

} PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_ENT;

/**
* @enum PRV_TGF_POLICER_MNG_CNTR_SET_ENT
 *
 * @brief Enumeration for Management Counters Set
*/
typedef enum{

    /** Management Counters Set #0 */
    PRV_TGF_POLICER_MNG_CNTR_SET0_E,

    /** Management Counters Set #1 */
    PRV_TGF_POLICER_MNG_CNTR_SET1_E,

    /** Management Counters Set #2 */
    PRV_TGF_POLICER_MNG_CNTR_SET2_E,

    /** Management Counters are disabled */
    PRV_TGF_POLICER_MNG_CNTR_DISABLED_E

} PRV_TGF_POLICER_MNG_CNTR_SET_ENT;

/**
* @enum PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT
 *
 * @brief Enumeration for Management Counters Type.
*/
typedef enum{

    /** @brief These Management Counters count
     *  marked GREEN Packet and DU that
     *  passed in this flow.
     */
    PRV_TGF_POLICER_MNG_CNTR_GREEN_E,

    /** @brief These Management Counters count
     *  marked YELLOW Packet and DU that
     *  passed in this flow.
     */
    PRV_TGF_POLICER_MNG_CNTR_YELLOW_E,

    /** @brief These Management Counters count
     *  marked RED Packet and DU that
     *  passed in this flow and were not
     *  dropped.
     */
    PRV_TGF_POLICER_MNG_CNTR_RED_E,

    /** @brief These Management Counters count
     *  marked RED Packet and DU that
     *  passed in this flow and were
     *  dropped.
     */
    PRV_TGF_POLICER_MNG_CNTR_DROP_E

} PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT;

/**
* @enum PRV_TGF_POLICER_ENTRY_TYPE_ENT
 *
 * @brief Enumeration for Policer Entry Type.
*/
typedef enum{

    /** Metering Policer Entry. */
    PRV_TGF_POLICER_ENTRY_METERING_E,

    /** Billing Policer Entry. */
    PRV_TGF_POLICER_ENTRY_BILLING_E

} PRV_TGF_POLICER_ENTRY_TYPE_ENT;

/**
* @enum PRV_TGF_POLICER_TT_PACKET_SIZE_MODE_ENT
 *
 * @brief Enumeration for Tunnel Termination Packet Size Mode.
*/
typedef enum{

    /** @brief Metering and counting
     *  of TT packets is
     *  performed according to
     *  the Ingress Metered
     *  Packet Size Mode.
     */
    PRV_TGF_POLICER_TT_PACKET_SIZE_REGULAR_E,

    /** @brief Metering and counting
     *  of TT packets is
     *  performed according to
     *  L3 datagram size only.
     *  This mode doesn't
     *  include the tunnel
     *  header size, the L2
     *  header size, and the
     *  packet CRC in the
     *  metering and counting.
     */
    PRV_TGF_POLICER_TT_PACKET_SIZE_PASSENGER_E

} PRV_TGF_POLICER_TT_PACKET_SIZE_MODE_ENT;

/**
* @enum PRV_TGF_POLICER_METER_MODE_ENT
 *
 * @brief Enumeration of the Meter modes
*/
typedef enum{

    /** a Single Rate Three Color Marking mode */
    PRV_TGF_POLICER_METER_MODE_SR_TCM_E,

    /** a Two Rate Three Color Marking mode */
    PRV_TGF_POLICER_METER_MODE_TR_TCM_E

} PRV_TGF_POLICER_METER_MODE_ENT;

/**
* @enum PRV_TGF_POLICER_NON_CONFORM_CMD_ENT
 *
 * @brief Structure contains Policer command for Non-Conforming Packets.
 * Conforming (i.e., In-profile) packets preserve their QoS
 * parameters setting and their forwarding decision, as set in the
 * previous engine in the ingress pipe. Non-conforming (i.e.,
 * Out-ofprofile) packets, are subject to forwarding decision
 * override and QoS parameter remarking according to the setting
 * of this field.
 * A non-conforming packet (marked by Red or Yellow color) can be
 * dropped by the metering engine, QoS profile remark or can be
 * forwarded.
*/
typedef enum{

    /** @brief NO CHANGE.
     *  Nonconforming packets preserve their QoS
     *  parameters setting and their forwarding decision,
     *  as set in the previous engine in the ingress pipe.
     */
    PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E,

    /** @brief DROP.
     *  Nonconforming packets preserve their QoS
     *  parameters setting as set in the previous engine in
     *  the ingress pipe, but their forwarding decision
     *  changes to Soft Drop or Hard Drop according to the
     *  setting of the Policer Drop Mode.
     */
    PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E,

    /** @brief REMARK. QoS Profile
     *  Remarking by table.
     *  Non-conforming packets preserve their forwarding
     *  decision as set in the previous engine in the
     *  ingress pipe, but their QoS parameters setting is
     *  modified as follows:
     *  - QoS Profile is extracted from the Policers QoS
     *  Remarking Table Entry according to CL.
     *  - ModifyUP is modified by this entry <modifyUP>.
     *  - ModifyDSCP is modified by this entry <modifyDSCP>.
     */
    PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_E,

    /** @brief REMARK. QoS Profile
     *  Remark by entry.
     *  Non-conforming packets preserve their forwarding
     *  decision as set in the previous engine in the ingress
     *  pipe, but their QoS parameters setting is modified as
     *  follows:
     *  - QoS Profile is modified by this entry <QosProfile>.
     *  - ModifyUP is modified by this entry <ModifyUP>.
     *  - ModifyDSCP is modified by this entry <ModifyDSCP>.
     *  No access to remarking table is done.
     *  Note: Relevant only for DxChXcat and above.
     */
    PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E

} PRV_TGF_POLICER_NON_CONFORM_CMD_ENT;

/**
* @struct PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC
 *
 * @brief Structure for Policer Management Counters Entry.
*/
typedef struct{

    /** @brief Data Unit Management Counter.
     *  DU size defines according to the assigned resolution
     *  of Management counters (1 Byte or 16 Bytes).
     *  Note: DxCh3 - only 32 LSB are used.
     *  DxChXcat and above - 42 bits are used.
     */
    GT_U64 duMngCntr;

    /** Packet 32 bit size Management Counter. */
    GT_U32 packetMngCntr;

} PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC;

/**
* @enum PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_ENT
 *
 * @brief Enumeration of compensation of DSA Tag-added byte count when
 * metering and counting packets are received via DSA-enabled ports.
*/
typedef enum{

    /** @brief Metering and billing operations include the DSA tags
     *  in the packet's Byte Count.
     */
    PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E,

    /** @brief DSA tags are subtracted from the packet byte count before
     *  performing metering or counting.
     */
    PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E

} PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_ENT;

/**
* @enum PRV_TGF_POLICER_TS_TAG_COUNTING_MODE_ENT
 *
 * @brief Configuration for byte counting, which determines whether
 * the timestamp tag should be included in the byte count computation or not.
*/
typedef enum{

    /** @brief In L2 counting mode, if the packet is timestamp tagged,
     *  the timestamp tag is included in the byte count computation.
     */
    PRV_TGF_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E,

    /** @brief In L2 counting mode, if the packet is timestamp tagged,
     *  the timestamp tag is NOT included in the byte count computation.
     */
    PRV_TGF_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E

} PRV_TGF_POLICER_TS_TAG_COUNTING_MODE_ENT;

/**
* @enum PRV_TGF_POLICER_MODIFY_DSCP_ENT
 *
 * @brief Modification mode of the transmitted packet’s IP Header DSCP.
*/
typedef enum{

    /** @brief Keep previous packet DSCP.
     *  Note: relevant only for Ingress Metering Entry.
     */
    PRV_TGF_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E,

    /** Disable modification of DSCP. */
    PRV_TGF_POLICER_MODIFY_DSCP_DISABLE_E,

    /** @brief Enable modification of DSCP.
     *  Note: For APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon,
     *  If the packet is tunneled, this refers to the tunnel header.
     */
    PRV_TGF_POLICER_MODIFY_DSCP_ENABLE_E,

    /** @brief Remark packet DSCP.
     *  If the packet is tunneled, this refers to the passenger header.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
     *  Note: relevant only for Egress Metering Entry.
     */
    PRV_TGF_POLICER_MODIFY_DSCP_ENABLE_INNER_E

} PRV_TGF_POLICER_MODIFY_DSCP_ENT;

/**
* @enum PRV_TGF_POLICER_MODIFY_UP_ENT
 *
 * @brief Modification mode for changing of IEEE 802.1p user priority.
*/
typedef enum{

    /** @brief Keep previous packet UP.
     *  Note: relevant only for Ingress Metering Entry.
     */
    PRV_TGF_POLICER_MODIFY_UP_KEEP_PREVIOUS_E,

    /** Disable modification of UP. */
    PRV_TGF_POLICER_MODIFY_UP_DISABLE_E,

    /** @brief Enable modification of UP.
     *  Note: For APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon,
     *  If the packet is tunneled, this refers to the tunnel header tag.
     */
    PRV_TGF_POLICER_MODIFY_UP_ENABLE_E,

    /** @brief Remark Tag0 UP of the packet.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
     *  Note: relevant only for Egress Metering Entry.
     */
    PRV_TGF_POLICER_MODIFY_UP_ENABLE_TAG0_E

} PRV_TGF_POLICER_MODIFY_UP_ENT;

/**
* @union PRV_TGF_POLICER_METER_TB_PARAMS_UNT
 *
 * @brief Union for single/dual Token Bucket parameters
 *
*/

typedef union{
    struct {
    /** Committed Information Rate in Kbps */
    GT_U32  cir;

    /** Committed Burst Size in bytes */
    GT_U32  cbs;

    /** Excess Burst Size in bytes */
    GT_U32  ebs;

    } srTcmParams;

    struct {
    /** Committed Information Rate in Kbps */
    GT_U32  cir;

    /** Committed Burst Size in bytes */
    GT_U32  cbs;

    /** Peak Information Rate in Kbps (1K = 1000) */
    GT_U32  pir;

    /** Peak Burst Size in bytes */
    GT_U32  pbs;

    } trTcmParams;

} PRV_TGF_POLICER_METER_TB_PARAMS_UNT;



/**
* @enum PRV_TGF_POLICER_REMARK_MODE_ENT
 *
 * @brief Enumeration of the Remark modes
*/
typedef enum{

    /** TC or UP is index in L2 remarking table */
    PRV_TGF_POLICER_REMARK_MODE_L2_E,

    /** @brief DSCP for IP packet or EXP for MPLS
     *  packets is index in L3 remarking table
     */
    PRV_TGF_POLICER_REMARK_MODE_L3_E

} PRV_TGF_POLICER_REMARK_MODE_ENT;

/**
* @enum PRV_TGF_POLICER_CMD_ENT
 *
 * @brief Enumeration of Policer Commands is applied to packets that were
 * classified as Red (out-of-profile) by the traffic meter
*/
typedef enum{

    /** performs no action on the packet */
    PRV_TGF_POLICER_CMD_NONE_E,

    /** the packet is SOFT or HARD dropped */
    PRV_TGF_POLICER_CMD_DROP_RED_E,

    /** preserve forwarding decision */
    PRV_TGF_POLICER_CMD_QOS_MARK_BY_ENTRY_E,

    /** preserve forwarding decision */
    PRV_TGF_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E

} PRV_TGF_POLICER_CMD_ENT;


/**
* @struct PRV_TGF_POLICER_ENTRY_STC
 *
 * @brief Structure for Policer Entry
*/
typedef struct{

    /** policer Enable */
    GT_BOOL policerEnable;

    /** policer's meter color mode */
    CPSS_POLICER_COLOR_MODE_ENT meterColorMode;

    /** enables counting In Profile and Out of Profile packets */
    GT_BOOL counterEnable;

    /** policing counters set index */
    GT_U32 counterSetIndex;

    /** policer command */
    PRV_TGF_POLICER_CMD_ENT cmd;

    /** QoSProfile assigned to non */
    GT_U32 qosProfile;

    /** remark mode */
    PRV_TGF_POLICER_REMARK_MODE_ENT remarkMode;

    /** meter color marking mode */
    PRV_TGF_POLICER_METER_MODE_ENT meterMode;

    /** affiliation of Policer Entry to Management Counter Set */
    PRV_TGF_POLICER_MNG_CNTR_SET_ENT mngCounterSet;

    /** token bucket parameters */
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;

    /** index of billing or policy counting entry. */
    GT_U32 countingEntryIndex;

    /** @brief enable/disable modifying User Priority.
     *  Note: For DxChXcat and above Egress, only Enable/Disable
     *  options are valid.
     *  Note: For APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon,
     *  relevant only for Ingress Metering Entry.
     */
    PRV_TGF_POLICER_MODIFY_UP_ENT modifyUp;

    /** @brief enable/disable modifying IP DCSP field.
     *  Note: For DxChXcat and above Egress, only Enable/Disable
     *  options are valid.
     *  Note: For APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon,
     *  relevant only for Ingress Metering Entry.
     */
    PRV_TGF_POLICER_MODIFY_DSCP_ENT modifyDscp;

    /** @brief enable/disable modifying DP field.
     *  Note: Supported for DxChXcat and above.
     *  Ignored by DxCh3.
     *  Note: Only Enable/Disable options are valid.
     *  Note: only for Egress Metering Entry.
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyDp;

    /** @brief enable/disable modifying TC field.
     *  (APPLICABLE DEVICES: Puma2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Note: Only Enable/Disable options are valid.
     *  Note: only for Egress Metering Entry.
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyTc;

    /** @brief Policer commands for Non
     *  Packets.
     */
    PRV_TGF_POLICER_NON_CONFORM_CMD_ENT yellowPcktCmd;

    /** Policer commands for Non */
    PRV_TGF_POLICER_NON_CONFORM_CMD_ENT redPcktCmd;

    /** @brief Policer commands for Conforming (Green)
     *  Packets. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
     *  Note: Relevant only for Ingress Metering Entry.
     */
    PRV_TGF_POLICER_NON_CONFORM_CMD_ENT greenPcktCmd;

    /** @brief enable/disable modifying EXP field.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Note: Only Enable/Disable options are valid.
     *  Note: only for Egress Metering Entry.
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyExp;

    /** @brief Metering algorithm resolution.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_POLICER_METER_RESOLUTION_ENT byteOrPacketCountingMode;

    /** @brief Metering packet\datagram size
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_POLICER_PACKET_SIZE_MODE_ENT packetSizeMode;

    /** @brief -  Defines whether metering of TT/TS packets
     *  are performed according to the <Packet Size Mode>;
     *  or according to L3 datagram size only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_POLICER_TT_PACKET_SIZE_MODE_ENT tunnelTerminationPacketSizeMode;

    /** @brief -  Enables compensation of DSA Tag-added byte count
     *  when metering packets are received
     *  via DSA-enabled ports.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_ENT dsaTagCountingMode;

    /** @brief -  Configuration for byte metering, which determines whether
     *  the timestamp tag should be included in the byte count
     *  computation or not.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_POLICER_TS_TAG_COUNTING_MODE_ENT timeStampCountingMode;

    /** @brief -  enable/disable modification of the transmitted packet's IP
     *  Header ECN field if the traffic has crossed the yellow
     *  threshold.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Note: only for Egress Metering Entry.
     */
    GT_BOOL yellowEcnMarkingEnable;

    /** @brief When enabled, packets that are marked as green by the metering mechanism
     *  are mirrored to the analyzer
     *
     *  Relevant for ingress and egress mirroring , for next devices:
     *  APPLICABLE DEVICES : Falcon.
     */
    GT_BOOL greenMirrorToAnalyzerEnable;

    /** @brief When enabled, packets that are marked as yellow by the metering mechanism
     *  are mirrored to the analyzer
     *
     *  Relevant for ingress and egress mirroring , for next devices:
     *  APPLICABLE DEVICES : Falcon.
     */
    GT_BOOL yellowMirrorToAnalyzerEnable;

    /** @brief When enabled, packets that are marked as red by the metering mechanism
     *  are mirrored to the analyzer
     *
     *  Relevant for ingress and egress mirroring , for next devices:
     *  APPLICABLE DEVICES : Falcon.
     */
    GT_BOOL redMirrorToAnalyzerEnable;

} PRV_TGF_POLICER_ENTRY_STC;

/**
* @enum PRV_TGF_POLICER_STAGE_TYPE_ENT
 *
 * @brief Enumeration for Policer Unit Type.
*/
typedef enum{

    /** Ingress Policer stage #0. */
    PRV_TGF_POLICER_STAGE_INGRESS_0_E,

    /** Ingress Policer stage #1. */
    PRV_TGF_POLICER_STAGE_INGRESS_1_E,

    /** Egress Policer stage. */
    PRV_TGF_POLICER_STAGE_EGRESS_E,

    PRV_TGF_POLICER_STAGE_NUM

} PRV_TGF_POLICER_STAGE_TYPE_ENT;


/**
* @enum PRV_TGF_POLICER_COUNTING_MODE_ENT
 *
 * @brief Enumeration for Policer Counting Mode.
*/
typedef enum{

    /** Counting is disabled. */
    PRV_TGF_POLICER_COUNTING_DISABLE_E,

    /** @brief Billing (per color counting)
     *  or IPFIX counting.
     *  Counting is triggered by
     *  Policy Action Entry or by
     *  Metering Entry.
     */
    PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E,

    /** @brief Policy activity counting mode:
     *  packet counters only. Counting
     *  is triggered by Policy Action
     *  Entry.
     */
    PRV_TGF_POLICER_COUNTING_POLICY_E,

    /** @brief VLAN counting mode: packet or
     *  byte counters. Counting is
     *  triggered for every packet
     *  according to VLAN-ID.
     */
    PRV_TGF_POLICER_COUNTING_VLAN_E

} PRV_TGF_POLICER_COUNTING_MODE_ENT;


/**
* @enum PRV_TGF_POLICER_REMARK_TABLE_TYPE_ENT
 *
 * @brief QoS parameter enumeration
*/
typedef enum{

    /** TC/UP Remark table. */
    PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E,

    /** DSCP Remark table. */
    PRV_TGF_POLICER_REMARK_TABLE_TYPE_DSCP_E,

    /** EXP Remark table. */
    PRV_TGF_POLICER_REMARK_TABLE_TYPE_EXP_E

} PRV_TGF_POLICER_REMARK_TABLE_TYPE_ENT;

/**
* @enum PRV_TGF_POLICER_E_ATTR_METER_MODE_ENT
 *
 * @brief Enumeration for ePort/eVLAN Metering mode.
*/
typedef enum{

    /** ePort/eVLAN based triggering is disabled. */
    PRV_TGF_POLICER_E_ATTR_METER_MODE_DISABLED_E,

    /** ePort based triggering is enabled. */
    PRV_TGF_POLICER_E_ATTR_METER_MODE_EPORT_ENABLED_E,

    /** eVLAN based triggering enabled. */
    PRV_TGF_POLICER_E_ATTR_METER_MODE_EVLAN_ENABLED_E

} PRV_TGF_POLICER_E_ATTR_METER_MODE_ENT;

/**
* @struct PRV_TGF_POLICER_QOS_PARAM_STC
 *
 * @brief The Quality of Service parameters.
*/
typedef struct{

    /** IEEE 802.1p User Priority (0..7) */
    GT_U32 up;

    /** IP DCSP field (0..63) */
    GT_U32 dscp;

    /** MPLS label EXP value (0..7) */
    GT_U32 exp;

    /** Drop Precedence value: GREEN, YELLOW or RED. */
    CPSS_DP_LEVEL_ENT dp;

    /** traffic class (0..7) */
    GT_U32 tc;

} PRV_TGF_POLICER_QOS_PARAM_STC;


/**
* @enum PRV_TGF_POLICER_STAGE_METER_MODE_ENT
 *
 * @brief Global stage mode.
*/
typedef enum{

    /** @brief Port-based metering is performed if
     *  the relevant port is enabled for metering.
     *  Port policing is done per device local source port
     *  for the Ingress Policer and per device local egress
     *  port for Egress Policer.
     */
    PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E,

    /** @brief Flow-based metering is performed if
     *  metering is enabled in Policy/TTI Action and
     *  the pointer is taken from the TTI/Policy Action.
     */
    PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E

} PRV_TGF_POLICER_STAGE_METER_MODE_ENT;



/**
* @enum PRV_TGF_POLICER_STORM_TYPE_ENT
 *
 * @brief Storm Types.
*/
typedef enum{

    /** Knowm Unicast */
    PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E,

    /** Unknowm Unicast */
    PRV_TGF_POLICER_STORM_TYPE_UC_UNKNOWN_E,

    /** Unregistered Multicast */
    PRV_TGF_POLICER_STORM_TYPE_MC_UNREGISTERED_E,

    /** Registered Multicast */
    PRV_TGF_POLICER_STORM_TYPE_MC_REGISTERED_E,

    /** Broadcast */
    PRV_TGF_POLICER_STORM_TYPE_BC_E,

    /** TCP-SYN */
    PRV_TGF_POLICER_STORM_TYPE_TCP_SYN_E

} PRV_TGF_POLICER_STORM_TYPE_ENT;


/**
* @enum PRV_TGF_POLICER_L2_REMARK_MODEL_ENT
 *
 * @brief Enumeration for L2 packets remarking model.
*/
typedef enum{

    /** @brief Traffic Class is index in the
     *  QoS table.
     */
    PRV_TGF_POLICER_L2_REMARK_MODEL_TC_E,

    /** @brief User Priority is index in the
     *  QoS table.
     */
    PRV_TGF_POLICER_L2_REMARK_MODEL_UP_E

} PRV_TGF_POLICER_L2_REMARK_MODEL_ENT;

/**
* @enum PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_ENT
 *
 * @brief Enumeration for Loss Measurement capture mode.
*/
typedef enum{

    /** @brief Copy;
     *  In this mode the counter is copied to <Green Counter Snapshot>.
     */
    PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_COPY_E = 0,

    /** @brief Insert;
     *  In this mode the counter is inserted into the packet.
     */
    PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_INSERT_E

} PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_ENT;


#if (defined CHX_FAMILY)
#define PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_0_E
#define PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E   CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_1_E
#define PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E   CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_2_E
#define PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_3_E
#else
/* Stub, used as argument for prvTgfPolicerMemorySizeModeSet() where ignored */
#define PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E  0
#define PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E   1
#define PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E   2
#define PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E  3
#endif

/**
* @enum PRV_TGF_POLICER_MEMORY_CTRL_MODE_ENT
 *
 * @brief Enumeration for Memory control mode.
 * Lower memory uses 256 entries. Upper memory uses the rest.
 * In case of xCat2: memory 1 has 172 entries, memory 2 has
 * 84 entries and memory 3 has 28 entries.
*/
typedef enum{

    /** @brief Policer 0
     *  uses the upper memory and Policer 1 uses the lower memory.
     *  In case of xCat2: Policer 1 uses two memories (1 and 2),
     *  Policer 0 uses one memory (3).
     */
    PRV_TGF_POLICER_MEMORY_CTRL_MODE_0_E,

    /** @brief Policer 0
     *  uses both memories.
     *  In case of xCat2: Policer 0 uses all three memories.
     */
    PRV_TGF_POLICER_MEMORY_CTRL_MODE_1_E,

    /** @brief Policer 1
     *  uses both memories.
     *  In case of xCat2: Policer 1 uses all three memories.
     */
    PRV_TGF_POLICER_MEMORY_CTRL_MODE_2_E,

    /** @brief Policer 1
     *  uses the upper memory and Policer 0 uses the lower memory.
     *  In case of xCat2: Policer 0 uses two memories (1 and 2),
     *  Policer 1 uses one memory (3).
     */
    PRV_TGF_POLICER_MEMORY_CTRL_MODE_3_E,

    /** @brief Policer 0 uses one memory (1),
     *  Policer 1 uses two memories (2 and 3).
     *  Applicable devices: xCat2
     *  Note: In this mode, when stage mode is Port, a compressed
     *  address select should be configured:
     *  PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E
     *  by cpssDxChPolicerPortModeAddressSelectSet().
     */
    PRV_TGF_POLICER_MEMORY_CTRL_MODE_4_E,

    /** @brief Policer 1 uses one memory (1),
     *  Policer 0 uses two memories (2 and 3).
     *  Applicable devices: xCat2
     *  Note: In this mode, when stage mode is Port, a compressed
     *  address select should be configured:
     *  PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E
     *  by cpssDxChPolicerPortModeAddressSelectSet().
     *  CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E - flexible mode that allow to set any
     *  partitions for ingress policer stage 0 , stage 1 and the egress
     *  policer stage. (see numEntriesIngressStage0,numEntriesIngressStage1
     *  in function cpssDxChPolicerMemorySizeModeSet(...))
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_POLICER_MEMORY_CTRL_MODE_5_E,

    PRV_TGF_POLICER_MEMORY_FLEX_MODE_E

} PRV_TGF_POLICER_MEMORY_CTRL_MODE_ENT;




/**
* @enum PRV_TGF_POLICER_VLAN_CNTR_MODE_ENT
 *
 * @brief Packet/Byte Vlan counter mode.
*/
typedef enum{

    /** Bytes Vlan counter mode. */
    PRV_TGF_POLICER_VLAN_CNTR_MODE_BYTES_E,

    /** Packets Vlan counter mode. */
    PRV_TGF_POLICER_VLAN_CNTR_MODE_PACKETS_E

} PRV_TGF_POLICER_VLAN_CNTR_MODE_ENT;

/**
* @struct PRV_TGF_POLICER_COUNTERS_STC
 *
 * @brief Structure for Policer Counters Entry
*/
typedef struct{

    /** Out */
    GT_U32 outOfProfileBytesCnt;

    /** In */
    GT_U32 inProfileBytesCnt;

} PRV_TGF_POLICER_COUNTERS_STC;


/**
* @enum PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT
 *
 * @brief Port Mode Address Select type.
*/
typedef enum{

    /** The address is (index,port_num). */
    PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E,

    /** @brief The address is (index,port_num), in
     *  addition all ingress policer memory is allocated to one stage.
     */
    PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_EXCLUSIVE_E,

    /** @brief Compressed;
     *  The address is (port_num,index).
     */
    PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E

} PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT;


/**
* @struct PRV_TGF_POLICER_TRIGGER_ENTRY_STC
 *
 * @brief Defines policer trigger entry.
*/
typedef struct{

    /** @brief GT_TRUE: Metering is performed
     *  GT_FALSE: Metering is not performed
     */
    GT_BOOL meteringEnable;

    /** @brief GT_TRUE: Counting is performed
     *  GT_FALSE: Counting is not performed
     */
    GT_BOOL countingEnable;

    /** Policer pointer that is used for metering/counting. */
    GT_U32 policerIndex;

    /** @brief GT_TRUE: Counting/metering is enabled for unicast
     *  known traffic.
     *  GT_FALSE: Counting/metering is disabled for unicast
     *  known traffic.
     */
    GT_BOOL ucKnownEnable;

    /** @brief Offset used for unicast known traffic (0
     *  The offset is compared to policerIndex.
     *  Note: Relevant only when ucKnownEnable = GT_TRUE.
     *  Note: In Egress stage, traffic type is ignored,
     *  thus the offset assumed to be 0x0.
     */
    GT_U32 ucKnownOffset;

    /** @brief GT_TRUE: Counting/metering is enabled for unicast
     *  unknown traffic.
     *  GT_FALSE: Counting/metering is disabled for unicast
     *  unknown traffic.
     */
    GT_BOOL ucUnknownEnable;

    /** @brief Offset used for unicast unknown traffic (0
     *  The offset is compared to policerIndex.
     *  Note: Relevant only when ucUnknownEnable = GT_TRUE.
     *  Note: In Egress stage, traffic type is ignored,
     *  thus the offset assumed to be 0x0.
     */
    GT_U32 ucUnknownOffset;

    /** @brief GT_TRUE: Counting/metering is enabled for
     *  registered multicast traffic.
     *  GT_FALSE: Counting/metering is disabled for
     *  registered multicast traffic.
     */
    GT_BOOL mcRegisteredEnable;

    /** @brief Offset used for registered multicast traffic (0
     *  The offset is compared to policerIndex.
     *  Note: Relevant only when mcRegisteredEnable = GT_TRUE.
     *  Note: In Egress stage, traffic type is ignored,
     *  thus the offset assumed to be 0x0.
     */
    GT_U32 mcRegisteredOffset;

    /** @brief GT_TRUE: Counting/metering is enabled for
     *  unregistered multicast traffic.
     *  GT_FALSE: Counting/metering is disabled for
     *  unregistered multicast traffic.
     */
    GT_BOOL mcUnregisteredEnable;

    /** @brief Offset used for unregistered multicast traffic (0
     *  The offset is compared to policerIndex.
     *  Note: Relevant only when mcUnregisteredEnable = GT_TRUE.
     *  Note: In Egress stage, traffic type is ignored,
     *  thus the offset assumed to be 0x0.
     */
    GT_U32 mcUnregisteredOffset;

    /** @brief GT_TRUE: Counting/metering is enabled for
     *  broadcast traffic.
     *  GT_FALSE: Counting/metering is disabled for
     *  broadcast traffic.
     */
    GT_BOOL bcEnable;

    /** @brief Offset used for broadcast traffic (0
     *  The offset is compared to policerIndex.
     *  Note: Relevant only when bcEnable = GT_TRUE.
     *  Note: In Egress stage, traffic type is ignored,
     *  thus the offset assumed to be 0x0.
     */
    GT_U32 bcOffset;

    /** @brief GT_TRUE: Counting/metering is enabled for
     *  TCP SYN traffic.
     *  GT_FALSE: Counting/metering is disabled for
     *  TCP SYN traffic.
     */
    GT_BOOL tcpSynEnable;

    /** @brief Offset used for TCP SYN traffic (0
     *  The offset is compared to policerIndex.
     *  Note: Relevant only when tcpSynEnable = GT_TRUE.
     *  Note: In Egress stage, traffic type is ignored,
     *  thus the offset assumed to be 0x0.
     */
    GT_U32 tcpSynOffset;

} PRV_TGF_POLICER_TRIGGER_ENTRY_STC;

/**
* @struct PRV_TGF_POLICER_BILLING_ENTRY_STC
 *
 * @brief Structure for Policer Billing Entry.
*/
typedef struct{

    /** @brief Green Billing counter, counts the number of
     *  Data Units (1 Byte/16 Bytes) of packets, marked Green
     *  that passed in this flow.
     *  Note: DxCh3 - only 32 LSB are used.
     *  DxChXcat and above - 42 bits are used.
     */
    GT_U64 greenCntr;

    /** @brief Yellow Billing counter, counts the number of
     *  Data Units (1 Byte/16 Bytes) of packets, marked Yellow
     *  that passed in this flow.
     *  Note: DxCh3 - only 32 LSB are used.
     *  DxChXcat and above - 42 bits are used.
     */
    GT_U64 yellowCntr;

    /** @brief Red Billing counter, counts the number of
     *  Data Units (1 Byte/16 Bytes) of packets, marked Red
     *  that passed in this flow.
     *  Note: DxCh3 - only 32 LSB are used.
     *  DxChXcat and above - 42 bits are used.
     */
    GT_U64 redCntr;

    /** @brief The resolution of billing counters.
     *  Applies only to the color counters.
     *  Note: Relevant for DxChXcat and above.
     */
    PRV_TGF_POLICER_BILLING_CNTR_MODE_ENT billingCntrMode;

    /** @brief -  When enabled, packets are counted in the green counter regardless
     *  of their color.
     *  In this case the red and yellow counters are not updated.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL billingCntrAllEnable;

    /** @brief -  Defines whether the lm_counter is updated to the Billing table or updated at the out Descriptor.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_ENT lmCntrCaptureMode;

    /** @brief -  Green Counter Snapshot LM Counting.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U64 greenCntrSnapshot;

    /** @brief -  Green Counter Snapshot Validity indication
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL greenCntrSnapshotValid;

    /** @brief -  Type of packet size
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_POLICER_PACKET_SIZE_MODE_ENT packetSizeMode;

    /** @brief -  Defines wether metering and counting of TT/TS packets
     *  are performed according to the <Packet Size Mode>;
     *  or according to L3 datagram size only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_POLICER_TT_PACKET_SIZE_MODE_ENT tunnelTerminationPacketSizeMode;

    /** @brief -  Enables compensation of DSA Tag-added byte count
     *  when metering and counting packets are received
     *  via DSA-enabled ports.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_POLICER_DSA_TAG_COUNTING_MODE_ENT dsaTagCountingMode;

    /** @brief -  Configuration for byte counting, which determines whether
     *  the timestamp tag should be included in the byte count
     *  computation or not.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_POLICER_TS_TAG_COUNTING_MODE_ENT timeStampCountingMode;

} PRV_TGF_POLICER_BILLING_ENTRY_STC;

/**
* @enum PRV_TGF_POLICER_BILLING_INDEX_MODE_ENT
 *
 * @brief Mode of Calculation Billing Entry index.
*/
typedef enum{

    /** @brief The Billing index is determined by the <Policer Index> from the IPCL/TTI/EPCL
     *  or by the index from the Metering table.
     */
    PRV_TGF_POLICER_BILLING_INDEX_MODE_STANDARD_E,

    /** @brief If the FlowID is in the configured range, the Billing index is
     *  determined by the packet’s Flow-ID. Otherwise,
     *  the index is determined in the standard way.
     */
    PRV_TGF_POLICER_BILLING_INDEX_MODE_FLOW_ID_E

} PRV_TGF_POLICER_BILLING_INDEX_MODE_ENT;

/**
* @struct PRV_TGF_POLICER_BILLING_INDEX_CFG_STC
 *
 * @brief Configuration of Calculation Billing Entry index.
*/
typedef struct{

    /** @brief Mode of Calculation Billing Entry index.
     *  billingIndexMode == PRV_TGF_POLICER_BILLING_INDEX_MODE_FLOW_ID_E
     *  but always written/read to/from HW.
     *  The formula for Billing Entry index in this mode is
     */
    PRV_TGF_POLICER_BILLING_INDEX_MODE_ENT billingIndexMode;

    /** @brief When the Billing entry is accessed using the packet’s Flow
     *  this field specifies the base index for Flow-ID access.
     *  billingFlowIdIndexBase + (flowId - billingMinFlowId).
     *  The packets with flow IDs out of range billingMinFlowId..billingMaxFlowId
     *  not counted.
     */
    GT_U32 billingFlowIdIndexBase;

    /** The minimal Flow */
    GT_U32 billingMinFlowId;

    /** @brief The maximal Flow
     *  All membres beside billingIndexMode relevant for HW behaviour only when
     */
    GT_U32 billingMaxFlowId;

} PRV_TGF_POLICER_BILLING_INDEX_CFG_STC;

/**
* @struct PRV_TGF_POLICER_HIERARCHICAL_TABLE_ENTRY_STC
 *
 * @brief Hierarchical Table Entry.
 * Used for remapping the values retrieved from TTI or PCL
 * action (that were used for first Policer stage) for the
 * second Policer stage.
*/
typedef struct{

    /** The new counting enable for remapping. */
    GT_BOOL countingEnable;

    /** The new metering enable for remapping. */
    GT_BOOL meteringEnable;

    /** The new policer pointer for remapping. */
    GT_U32 policerPointer;

} PRV_TGF_POLICER_HIERARCHICAL_TABLE_ENTRY_STC;

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertGenericToDxChPolicerStage function
* @endinternal
*
* @brief   Convert generic into device specific policer stage
*
* @param[in] policerStage             - (pointer to) policer stage
*
* @param[out] dxChStagePtr             - (pointer to) DxCh policer stage
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChPolicerStage
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT      policerStage,
    OUT CPSS_DXCH_POLICER_STAGE_TYPE_ENT    *dxChStagePtr
);
#endif /* CHX_FAMILY */

/**
* @internal prvTgfPolicerInit function
* @endinternal
*
* @brief   Init Traffic Conditioner facility on specified device
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - on illegal devNum
*/
GT_STATUS prvTgfPolicerInit
(
    GT_VOID
);

/**
* @internal prvTgfPolicerMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables metering per device.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerMeteringEnableSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfPolicerMeteringEnableGet function
* @endinternal
*
* @brief   Gets device metering status (Enable/Disable).
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - pointer to Enable/Disable metering
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerMeteringEnableGet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_BOOL                        *enablePtr
);

/**
* @internal prvTgfPolicerBillingCountingEnableSet function
* @endinternal
*
* @brief   Enables or disables Billing Counting.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerBillingCountingEnableSet
(
    IN GT_BOOL  enable
);

/**
* @internal prvTgfPolicerBillingCountingEnableGet function
* @endinternal
*
* @brief   Gets device Billing Counting status (Enable/Disable).
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - pointer to Enable/Disable Billing Counting
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerBillingCountingEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal prvTgfPolicerCountingModeSet function
* @endinternal
*
* @brief   Configures counting mode.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     -  counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
*/
GT_STATUS prvTgfPolicerCountingModeSet
(
    IN GT_U8                             devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    IN PRV_TGF_POLICER_COUNTING_MODE_ENT mode
);

/**
* @internal prvTgfPolicerCountingModeGet function
* @endinternal
*
* @brief   Gets the couning mode.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - pointer to Counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerCountingModeGet
(
    IN  GT_U8                             devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    OUT PRV_TGF_POLICER_COUNTING_MODE_ENT *modePtr
);

/**
* @internal prvTgfPolicerBillingCountersModeSet function
* @endinternal
*
* @brief   Sets the Billing Counters resolution.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or cntrMode.
*/
GT_STATUS prvTgfPolicerBillingCountersModeSet
(
    IN PRV_TGF_POLICER_BILLING_CNTR_MODE_ENT cntrMode
);


/**
* @internal prvTgfPolicerBillingCountersModeGet function
* @endinternal
*
* @brief   Gets the Billing Counters resolution.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] cntrModePtr              - pointer to the Billing Counters resolution
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerBillingCountersModeGet
(
    IN GT_U8                                  devNum,
    OUT PRV_TGF_POLICER_BILLING_CNTR_MODE_ENT *cntrModePtr
);

/**
* @internal prvTgfPolicerPacketSizeModeSet function
* @endinternal
*
* @brief   Sets metered Packet Size Mode that metering and billing is done
*         according to.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or packetSize.
*/
GT_STATUS prvTgfPolicerPacketSizeModeSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    IN CPSS_POLICER_PACKET_SIZE_MODE_ENT packetSize
);

/**
* @internal prvTgfPolicerPacketSizeModeGet function
* @endinternal
*
* @brief   Gets metered Packet Size Mode that metering and billing is done
*         according to.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] packetSizePtr            - pointer to the Type of packet size
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerPacketSizeModeGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT      stage,
    OUT CPSS_POLICER_PACKET_SIZE_MODE_ENT   *packetSizePtr
);

/**
* @internal prvTgfPolicerMeterResolutionSet function
* @endinternal
*
* @brief   Sets metering algorithm resolution
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or resolution.
*/
GT_STATUS prvTgfPolicerMeterResolutionSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN PRV_TGF_POLICER_METER_RESOLUTION_ENT resolution
);

/**
* @internal prvTgfPolicerMeterResolutionGet function
* @endinternal
*
* @brief   Gets metering algorithm resolution
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] resolutionPtr            - pointer to the Meter resolution: packet or Byte based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerMeterResolutionGet
(
    IN GT_U8                                    devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    OUT PRV_TGF_POLICER_METER_RESOLUTION_ENT *resolutionPtr
);

/**
* @internal prvTgfPolicerDropTypeSet function
* @endinternal
*
* @brief   Sets the Policer out-of-profile drop command type.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, dropType or stage.
*/
GT_STATUS prvTgfPolicerDropTypeSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN CPSS_DROP_MODE_TYPE_ENT        dropType
);

/**
* @internal prvTgfPolicerDropTypeGet function
* @endinternal
*
* @brief   Gets the Policer out-of-profile drop command type.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*
* @param[out] dropTypePtr              - pointer to the Policer Drop Type: Soft or Hard.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerDropTypeGet
(
    IN  GT_U8                               devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DROP_MODE_TYPE_ENT             *dropTypePtr
);

/**
* @internal prvTgfPolicerCountingColorModeSet function
* @endinternal
*
* @brief   Sets the Policer color counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
*/
GT_STATUS prvTgfPolicerCountingColorModeSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN PRV_TGF_POLICER_COLOR_COUNT_MODE_ENT mode
);

/**
* @internal prvTgfPolicerCountingColorModeGet function
* @endinternal
*
* @brief   Gets the Policer color counting mode.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - pointer to the color counting mode:
*                                      Drop Precedence or Conformance Level.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerCountingColorModeGet
(
    IN  GT_U8                                       devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT             stage,
    OUT PRV_TGF_POLICER_COLOR_COUNT_MODE_ENT     *modePtr
);

/**
* @internal prvTgfPolicerManagementCntrsResolutionSet function
* @endinternal
*
* @brief   Sets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or
*                                       Management Counters Set or cntrResolution.
*/
GT_STATUS prvTgfPolicerManagementCntrsResolutionSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT             stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT          cntrSet,
    IN PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_ENT   cntrResolution
);

/**
* @internal prvTgfPolicerManagementCntrsResolutionGet function
* @endinternal
*
* @brief   Gets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] cntrSet                  - Management Counters Set [0..2].
*
* @param[out] cntrResolutionPtr        - pointer to the Management Counters
*                                      resolution: 1 or 16 Bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or Mng Counters Set.
*/
GT_STATUS prvTgfPolicerManagementCntrsResolutionGet
(
    IN  GT_U8                                       devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT             stage,
    IN  PRV_TGF_POLICER_MNG_CNTR_SET_ENT         cntrSet,
    OUT PRV_TGF_POLICER_MNG_CNTR_RESOLUTION_ENT  *cntrResolutionPtr
);

/**
* @internal prvTgfPolicerPacketSizeModeForTunnelTermSet function
* @endinternal
*
* @brief   Sets size mode for metering and counting of tunnel terminated packets.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or ttPacketSizeMode.
*/
GT_STATUS prvTgfPolicerPacketSizeModeForTunnelTermSet
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT           stage,
    IN  PRV_TGF_POLICER_TT_PACKET_SIZE_MODE_ENT  ttPacketSizeMode
);

/**
* @internal prvTgfPolicerPacketSizeModeForTunnelTermGet function
* @endinternal
*
* @brief   Gets size mode for metering and counting of tunnel terminated packets.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] ttPacketSizeModePtr      - pointer to the Tunnel Termination
*                                      Packet Size Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerPacketSizeModeForTunnelTermGet
(
    IN  GT_U8                                       devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT             stage,
    OUT PRV_TGF_POLICER_TT_PACKET_SIZE_MODE_ENT  *ttPacketSizeModePtr
);

/**
* @internal prvTgfPolicerMeteringAutoRefreshScanEnableSet function
* @endinternal
*
* @brief   Enables or disables the metering Auto Refresh Scan mechanism.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshScanEnableSet
(
    IN GT_BOOL enable
);

/**
* @internal prvTgfPolicerMeteringAutoRefreshScanEnableGet function
* @endinternal
*
* @brief   Gets metering Auto Refresh Scan mechanism status (Enabled/Disabled).
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - pointer on Auto Refresh Scan mechanism status
*                                      (Enable/Disable) :
*                                      GT_TRUE  - Auto Refresh scan mechanism is enabled.
*                                      GT_FALSE - Auto Refresh scan mechanism is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshScanEnableGet
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal prvTgfPolicerMeteringAutoRefreshRangeSet function
* @endinternal
*
* @brief   Sets Metering Refresh Scan address range (Start and Stop addresses).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or illegal values
*                                       of startAddress and stopAddress.
* @retval GT_OUT_OF_RANGE          - on out of range of startAddress
*                                       or stopAddress.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshRangeSet
(
    IN GT_U32 startAddress,
    IN GT_U32 stopAddress
);

/**
* @internal prvTgfPolicerMeteringAutoRefreshRangeGet function
* @endinternal
*
* @brief   Gets Metering Refresh Scan address range (Start and Stop addresses).
*
* @param[in] devNum                   - physical device number.
*
* @param[out] startAddressPtr          - pointer to the beginning address of Refresh
*                                      Scan address range.
* @param[out] stopAddressPtr           - pointer to the end address of Refresh Scan
*                                      address range.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshRangeGet
(
    IN GT_U8                                devNum,
    OUT GT_U32                              *startAddressPtr,
    OUT GT_U32                              *stopAddressPtr
);

/**
* @internal prvTgfPolicerMeteringAutoRefreshIntervalSet function
* @endinternal
*
* @brief   Sets the time interval between two refresh access to metering table.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on out of range of interval value.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshIntervalSet
(
    IN GT_U32 interval
);

/**
* @internal prvTgfPolicerMeteringAutoRefreshIntervalGet function
* @endinternal
*
* @brief   Gets the time interval between two refresh access to metering table.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] intervalPtr              - pointer to the time interval between refresh of two
*                                      entries in micro seconds. Upon 270 Mhz core clock
*                                      value of DxCh3, the range [0..15907286 microS].
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerMeteringAutoRefreshIntervalGet
(
    IN GT_U8                                devNum,
    OUT GT_U32                              *intervalPtr
);

/**
* @internal prvTgfPolicerMeteringEntryRefresh function
* @endinternal
*
* @brief   Refresh the Policer Metering Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
*/
GT_STATUS prvTgfPolicerMeteringEntryRefresh
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U32                         entryIndex
);

/**
* @internal prvTgfPolicerPortMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables a port metering trigger for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] enable                   - Enable/Disable per-port metering for packets arriving
*                                      on this port:
*                                      GT_TRUE  - Metering is triggered on specified port.
*                                      GT_FALSE - Metering isn't triggered on specified port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
*/
GT_STATUS prvTgfPolicerPortMeteringEnableSet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfPolicerPortMeteringEnableGet function
* @endinternal
*
* @brief   Gets port status (Enable/Disable) of metering for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
*
* @param[out] enablePtr                - pointer on per-port metering status (Enable/Disable)
*                                      for packets arriving on specified port:
*                                      GT_TRUE  - Metering is triggered on specified port.
*                                      GT_FALSE - Metering isn't triggered on specified port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
*/
GT_STATUS prvTgfPolicerPortMeteringEnableGet
(
    IN  GT_U8                               devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                               portNum,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal prvTgfPolicerErrorGet function
* @endinternal
*
* @brief   Gets address and type of Policer Entry that had an ECC error.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] entryTypePtr             - pointer to the Type of Entry (Metering or Counting)
*                                      that had an error.
* @param[out] entryAddrPtr             - pointer to the Policer Entry that had an error.
*
* @retval GT_OK                    - on success.
* @retval GT_EMPTY                 - on missing error information.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerErrorGet
(
    IN  GT_U8                               devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    OUT PRV_TGF_POLICER_ENTRY_TYPE_ENT   *entryTypePtr,
    OUT GT_U32                              *entryAddrPtr
);

/**
* @internal prvTgfPolicerErrorCounterGet function
* @endinternal
*
* @brief   Gets the value of the Policer ECC Error Counter.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] cntrValuePtr             - pointer to the Policer ECC Error counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerErrorCounterGet
(
    IN  GT_U8                               devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *cntrValuePtr
);

/**
* @internal prvTgfPolicerManagementCountersSet function
* @endinternal
*
* @brief   Sets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for DxChXcat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set[0..2].
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
* @param[in] mngCntrPtr               - pointer to the Management Counters Entry must be set.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage or mngCntrType
*                                       or Management Counters Set number.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for DxChXcat and above devices.
*
*/
GT_STATUS prvTgfPolicerManagementCountersSet
(
    IN GT_U8                              devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT   mngCntrSet,
    IN PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT  mngCntrType,
    IN PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC *mngCntrPtr
);

/**
* @internal prvTgfPolicerManagementCountersGet function
* @endinternal
*
* @brief   Gets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for DxChXcat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set[0..2].
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
*
* @param[out] mngCntrPtr               - pointer to the requested Management Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage or mngCntrType
*                                       or Management Counters Set number.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for DxChXcat and above devices.
*
*/
GT_STATUS prvTgfPolicerManagementCountersGet
(
    IN GT_U8                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    IN  PRV_TGF_POLICER_MNG_CNTR_SET_ENT   mngCntrSet,
    IN  PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT  mngCntrType,
    OUT PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC *mngCntrPtr
);

/**
* @internal prvTgfPolicerEntryUseEarchEnable function
* @endinternal
*
* @brief   Setting flag indicating that the in the metering entry all
*         eArch related field must be explicitly configured.
* @param[in] enable                   - indication if eArch metering format is to be configured
*                                      explicitly.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvTgfPolicerEntryUseEarchEnable
(
    IN  GT_BOOL enable
);

/**
* @internal prvTgfPolicerEntrySet function
* @endinternal
*
* @brief   Set Policer Entry configuration
*
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for DxChXcat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - policer entry index
* @param[in] entryPtr                 - (pointer to) policer entry
*
* @param[out] tbParamsPtr              - (pointer to) actual policer token bucket parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, entryIndex or entry parameter
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfPolicerEntrySet
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN  GT_U32                               entryIndex,
    IN  PRV_TGF_POLICER_ENTRY_STC           *entryPtr,
    OUT PRV_TGF_POLICER_METER_TB_PARAMS_UNT *tbParamsPtr
);

/**
* @internal prvTgfPolicerEntryGet function
* @endinternal
*
* @brief   Get Policer Entry parameters
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for DxChXcat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - policer entry index
*
* @param[out] entryPtr                 - (pointer to) policer entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - on max number of retries checking if PP ready
*/
GT_STATUS prvTgfPolicerEntryGet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U32                         entryIndex,
    OUT PRV_TGF_POLICER_ENTRY_STC     *entryPtr
);

/**
* @internal prvTgfPolicerEntryMeterParamsCalculate function
* @endinternal
*
* @brief   Calculates Token Bucket parameters in the Application format without
*         HW update.
* @param[in] devNum                   - device number
* @param[in] tbInParamsPtr            - pointer to Token bucket input parameters.
* @param[in] meterMode                - Meter mode (SrTCM or TrTCM).
*
* @param[out] tbOutParamsPtr           - pointer to Token bucket output paramters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong devNum or meterMode.
*/
GT_STATUS prvTgfPolicerEntryMeterParamsCalculate
(
    IN  GT_U8                                   devNum,
    IN  PRV_TGF_POLICER_METER_MODE_ENT       meterMode,
    IN  PRV_TGF_POLICER_METER_TB_PARAMS_UNT  *tbInParamsPtr,
    OUT PRV_TGF_POLICER_METER_TB_PARAMS_UNT  *tbOutParamsPtr
);

/**
* @internal prvTgfPolicerBillingEntrySet function
* @endinternal
*
* @brief   Sets Policer Billing Counters.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
* @param[in] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out upon counter reset by
*                                       indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
*/
GT_STATUS prvTgfPolicerBillingEntrySet
(
    IN  GT_U8                                   devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  PRV_TGF_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
);

/**
* @internal prvTgfPolicerBillingEntryGet function
* @endinternal
*
* @brief   Gets Policer Billing Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
* @param[in] reset                    -  flag:
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read counters operation only.
*
* @param[out] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
*/
GT_STATUS prvTgfPolicerBillingEntryGet
(
    IN  GT_U8                                   devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  GT_BOOL                                 reset,
    OUT PRV_TGF_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
);

/**
* @internal prvTgfPolicerEntryInvalidate function
* @endinternal
*
* @brief   Invalidate Policer Entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or entryIndex
*/
GT_STATUS prvTgfPolicerEntryInvalidate
(
    IN  GT_U32                        entryIndex
);

/**
* @internal prvTgfPolicerQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes [0..127] of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colors.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 Qos Remarking Entry contains three indexes [0..1023] of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] qosProfileIndex          - index of Qos Remarking Entry will be set.
* @param[in] greenQosTableRemarkIndex - QoS profile (index in the Qos Table)
*                                      assigned to In-Profile packets with
*                                      the Green color. (APPLICABLE RANGES: 0..1023).
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.).
* @param[in] yellowQosTableRemarkIndex - QoS profile (index in the Qos Table)
*                                      assigned to Out-Of-Profile packets with
*                                      the Yellow color. (APPLICABLE RANGES: 0..127).
*                                      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 - (APPLICABLE RANGES: 0..1023).
* @param[in] redQosTableRemarkIndex   - QoS profile (index in the Qos Table)
*                                      assigned to Out-Of-Profile packets with
*                                      the Red color. (APPLICABLE RANGES: 0..127).
*                                      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2 - (APPLICABLE RANGES: 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or qosProfileIndex.
* @retval GT_OUT_OF_RANGE          - on greenQosTableRemarkIndex,
*                                       yellowQosTableRemarkIndex and
*                                       redQosTableRemarkIndex out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Supported only for Ingress Policer.
*
*/
GT_STATUS prvTgfPolicerQosRemarkingEntrySet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                         qosProfileIndex,
    IN GT_U32                         greenQosTableRemarkIndex,
    IN GT_U32                         yellowQosTableRemarkIndex,
    IN GT_U32                         redQosTableRemarkIndex
);

/**
* @internal prvTgfPolicerQosRemarkingEntryGet function
* @endinternal
*
* @brief   Gets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes [0..127] of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colours.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 Qos Remarking Entry contains three indexes [0..1023] of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] qosProfileIndex          - index of requested Qos Remarking Entry.
*
* @param[out] greenQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      In-Profile packets with the
*                                      Green color.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.).
* @param[out] yellowQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      Out-Of-Profile packets with the
*                                      Yellow color.
* @param[out] redQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      Out-Of-Profile packets with the Red
*                                      color.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or qosProfileIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Supported only for Ingress Policer.
*
*/
GT_STATUS prvTgfPolicerQosRemarkingEntryGet
(
    IN  GT_U8                               devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN GT_U32                               qosProfileIndex,
    OUT GT_U32                              *greenQosTableRemarkIndexPtr,
    OUT GT_U32                              *yellowQosTableRemarkIndexPtr,
    OUT GT_U32                              *redQosTableRemarkIndexPtr
);

/**
* @internal prvTgfPolicerEgressQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Egress Policer Re-Marking table Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkTableType,
*                                       remarkParamValue or confLevel.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on QoS parameter out of range
*/
GT_STATUS prvTgfPolicerEgressQosRemarkingEntrySet
(
    IN PRV_TGF_POLICER_REMARK_TABLE_TYPE_ENT remarkTableType,
    IN GT_U32                                remarkParamValue,
    IN CPSS_DP_LEVEL_ENT                     confLevel,
    IN PRV_TGF_POLICER_QOS_PARAM_STC         *qosParamPtr
);

/**
* @internal prvTgfPolicerEgressQosRemarkingEntryGet function
* @endinternal
*
* @brief   Gets Egress Policer Re-Marking table Entry.
*
* @param[in] devNum                   - device number.
* @param[in] remarkTableType          - Remark table type: DSCP, EXP or TC/UP.
* @param[in] remarkParamValue         - QoS parameter value.
* @param[in] confLevel                - Conformance Level: Green, Yellow, Red.
*
* @param[out] qosParamPtr              - pointer to the requested Re-Marking Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkTableType,
*                                       remarkParamValue or confLevel.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
GT_STATUS prvTgfPolicerEgressQosRemarkingEntryGet
(
    IN  GT_U8                                 devNum,
    IN  PRV_TGF_POLICER_REMARK_TABLE_TYPE_ENT remarkTableType,
    IN  GT_U32                                remarkParamValue,
    IN  CPSS_DP_LEVEL_ENT                     confLevel,
    OUT PRV_TGF_POLICER_QOS_PARAM_STC         *qosParamPtr
);

/**
* @internal prvTgfPolicerStageMeterModeSet function
* @endinternal
*
* @brief   Sets Policer Global stage mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
*/
GT_STATUS prvTgfPolicerStageMeterModeSet
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN  PRV_TGF_POLICER_STAGE_METER_MODE_ENT mode
);

/**
* @internal prvTgfPolicerStageMeterModeGet function
* @endinternal
*
* @brief   Gets Policer Global stage mode.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - (pointer to) Policer meter mode: FLOW or PORT.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerStageMeterModeGet
(
    IN  GT_U8                                       devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT             stage,
    OUT PRV_TGF_POLICER_STAGE_METER_MODE_ENT      *modePtr
);

/**
* @internal prvTgfPolicerMeteringOnTrappedPktsEnableSet function
* @endinternal
*
* @brief   Enable/Disable Ingress metering for Trapped packets.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] enable                   - GT_TRUE - policing is executed on all packets (data
*                                      and control).
*                                      GT_FALSE - policing is executed only on packets that
*                                      were asigned Forward (mirrored to analyzer
*                                      is inclusive) or Mirror to CPU commands by
*                                      previous processing stages.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
*/
GT_STATUS prvTgfPolicerMeteringOnTrappedPktsEnableSet
(
    IN  GT_U8                                   devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    IN GT_BOOL                                  enable
);

/**
* @internal prvTgfPolicerMeteringOnTrappedPktsEnableGet function
* @endinternal
*
* @brief   Get Ingress metering status (Enabled/Disabled) for Trapped packets.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*
* @param[out] enablePtr                - GT_TRUE - policing is executed on all packets (data
*                                      and control).
*                                      GT_FALSE - policing is executed only on packets that
*                                      were asigned Forward (mirrored to analyzer
*                                      is inclusive) or Mirror to CPU commands by
*                                      previous processing stages.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerMeteringOnTrappedPktsEnableGet
(
    IN  GT_U8                               devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal prvTgfPolicerPortStormTypeIndexSet function
* @endinternal
*
* @brief   Associates policing profile with source/target port and storm rate type.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] stormType                - storm type
* @param[in] index                    - policer  (0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, stormType ot port.
* @retval GT_OUT_OF_RANGE          - on out of range of index.
*/
GT_STATUS prvTgfPolicerPortStormTypeIndexSet
(
    IN  GT_U8                               devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                                portNum,
    IN PRV_TGF_POLICER_STORM_TYPE_ENT     stormType,
    IN GT_U32                               index
);


/**
* @internal prvTgfPolicerPortStormTypeIndexGet function
* @endinternal
*
* @brief   Get policing profile for given source/target port and storm rate type.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] stormType                - storm type
*
* @param[out] indexPtr                 - pointer to policer index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, stormType or portNum.
* @retval GT_OUT_OF_RANGE          - on out of range of port.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerPortStormTypeIndexGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U8                               portNum,
    IN  PRV_TGF_POLICER_STORM_TYPE_ENT    stormType,
    OUT GT_U32                              *indexPtr
);

/**
* @internal prvTgfPolicerL2RemarkModelSet function
* @endinternal
*
* @brief   Sets Policer L2 packets remarking model.
*
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] model                    - L2 packets remarking model. It defines QoS parameter,
*                                      which used as index in the Qos table:
*                                      User Priority or Traffic Class.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
*/
GT_STATUS prvTgfPolicerL2RemarkModelSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT      stage,
    IN PRV_TGF_POLICER_L2_REMARK_MODEL_ENT model
);

/**
* @internal prvTgfPolicerEgressL2RemarkModelGet function
* @endinternal
*
* @brief   Gets Egress Policer L2 packets remarking model.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] modelPtr                 - L2 packets remarking model. It defines QoS parameter,
*                                      which used as index in the Qos table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerEgressL2RemarkModelGet
(
    IN  GT_U8                                    devNum,
    OUT PRV_TGF_POLICER_L2_REMARK_MODEL_ENT    *modelPtr
);

/**
* @internal prvTgfPolicerQosUpdateEnableSet function
* @endinternal
*
* @brief   The function enables or disables QoS remarking of conforming packets.
*
* @param[in] enable                   - Enable/disable Qos update for conforming packets:
*                                      GT_TRUE  - Remark Qos parameters of conforming packets.
*                                      GT_FALSE - Keep incoming Qos parameters of conforming packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPolicerQosUpdateEnableSet
(
    IN GT_BOOL                          enable
);

/**
* @internal prvTgfPolicerEgressQosUpdateEnableGet function
* @endinternal
*
* @brief   The function get QoS remarking status of conforming packets
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - Enable/disable Qos update for conforming packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerEgressQosUpdateEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfPolicerVlanCntrSet function
* @endinternal
*
* @brief   Sets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] vid                      - VLAN ID. Bits [1:0] of VID are used to select the
*                                      relevant 32 bit counter inside the Counting Entry.
* @param[in] cntrValue                - Packet/Byte counter indexed by the packet's VID.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled VLAN Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or vid (VID range is limited by max
*                                       number of Policer Counting Entries) or stage.
*
* @note The VLAN counters are free-running no-sticky counters.
*
*/
GT_STATUS prvTgfPolicerVlanCntrSet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U16                         vid,
    IN  GT_U32                         cntrValue
);

/**
* @internal prvTgfPolicerVlanCntrGet function
* @endinternal
*
* @brief   Gets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] vid                      - VLAN ID. Bits [1:0] of VID are used to select the
*                                      relevant 32 bit counter inside the Counting Entry.
*
* @param[out] cntrValuePtr             - Packet/Byte counter indexed by the packet's VID.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled VLAN Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or vid (VID range is limited by max
*                                       number of Policer Counting Entries) or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
*
* @note The VLAN counters are free-running no-sticky counters.
*
*/
GT_STATUS prvTgfPolicerVlanCntrGet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U16                         vid,
    OUT GT_U32                         *cntrValuePtr
);

/**
* @internal prvTgfPolicerVlanCountingPacketCmdTriggerSet function
* @endinternal
*
* @brief   Enables or disables VLAN counting triggering according to the
*         specified packet command.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] cmdTrigger               - The packet command trigger.
* @param[in] enable                   - Enable/Disable VLAN Counting according to the packet
*                                      command trigger
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_SUPPORTED         - on not supported egress direction
*/
GT_STATUS prvTgfPolicerVlanCountingPacketCmdTriggerSet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN CPSS_PACKET_CMD_ENT            cmdTrigger,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfPolicerVlanCountingPacketCmdTriggerGet function
* @endinternal
*
* @brief   Gets VLAN counting triggering status (Enable/Disable) according to the
*         specified packet command.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] cmdTrigger               - The packet command trigger.
*
* @param[out] enablePtr                - Pointer to Enable/Disable VLAN Counting according
*                                      to the packet command trigger:
*                                      GT_TRUE  - Enable VLAN counting on packet with
*                                      triggered packet command.
*                                      GT_FALSE - Disable VLAN counting on packet with
*                                      triggered packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerVlanCountingPacketCmdTriggerGet
(
    IN  GT_U8                          devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  CPSS_PACKET_CMD_ENT            cmdTrigger,
    OUT GT_BOOL                        *enablePtr
);

/**
* @internal prvTgfPolicerVlanCountingModeSet function
* @endinternal
*
* @brief   Sets the Policer VLAN counters mode as byte or packet based.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - VLAN counters mode: Byte or Packet based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
*/
GT_STATUS prvTgfPolicerVlanCountingModeSet
(
    IN GT_U8                                    devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    IN PRV_TGF_POLICER_VLAN_CNTR_MODE_ENT     mode
);

/**
* @internal prvTgfPolicerVlanCountingModeGet function
* @endinternal
*
* @brief   Gets the Policer VLAN counters mode as byte or packet based.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - (pointer to) VLAN counters mode: Byte or Packet based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerVlanCountingModeGet
(
    IN  GT_U8                                    devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    OUT PRV_TGF_POLICER_VLAN_CNTR_MODE_ENT     *modePtr
);

/**
* @internal prvTgfPolicerPolicyCntrSet function
* @endinternal
*
* @brief   Sets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
* @param[in] cntrValue                - packets counter.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index (index range is limited by
*                                       max number of Policer Policy counters).
*/
GT_STATUS prvTgfPolicerPolicyCntrSet
(
    IN GT_U8                           devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U32                         index,
    IN  GT_U32                         cntrValue
);

/**
* @internal prvTgfPolicerPolicyCntrGet function
* @endinternal
*
* @brief   Gets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*
* @param[out] cntrValuePtr             - Pointer to the packet counter.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out.
*/
GT_STATUS prvTgfPolicerPolicyCntrGet
(
    IN GT_U8                           devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN  GT_U32                         index,
    OUT GT_U32                         *cntrValuePtr
);

/**
* @internal prvTgfPolicerCountersSet function
* @endinternal
*
* @brief   Set Policer Counters. To reset counters use zero values.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - policing counters set index
* @param[in] countersPtr              - (pointer to) counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfPolicerCountersSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        counterSetIndex,
    IN  PRV_TGF_POLICER_COUNTERS_STC *countersPtr
);

/**
* @internal prvTgfPolicerCountersGet function
* @endinternal
*
* @brief   Get Policer Counters
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - policing counters set index
*
* @param[out] countersPtr              - (pointer to) counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - on max number of retries checking if PP ready
*/
GT_STATUS prvTgfPolicerCountersGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        counterSetIndex,
    OUT PRV_TGF_POLICER_COUNTERS_STC *countersPtr
);

/**
* @internal prvTgfPolicerMemorySizeModeSet function
* @endinternal
*
* @brief   Sets internal table sizes and the way they are shared between the Ingress
*         policers.
* @param[in] devNum                   - device number.
* @param[in] mode                     - The  in which internal tables are shared.
* @param[in] numEntriesIngressStage0  - The number of metering/billing counters for
*                                      ingress policer stage 0
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
* @param[in] numEntriesIngressStage1  - The number of metering/billing counters for
*                                      ingress policer stage 1
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*                                      NOTE: the egress policer gets the 'rest' of the metering/billing counters
*                                      the that device support.
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
*/
GT_STATUS prvTgfPolicerMemorySizeModeSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_POLICER_MEMORY_CTRL_MODE_ENT mode,
    IN  GT_U32                              numEntriesIngressStage0,
    IN  GT_U32                              numEntriesIngressStage1
);

/**
* @internal prvTgfPolicerMemorySizeModeGet function
* @endinternal
*
* @brief   Gets internal table sizes and the way they are shared between the Ingress
*         policers.
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - The mode in which internal tables are shared.
* @param[out] numEntriesIngressStage0Ptr - (pointer to) The number of metering/billing counters for
*                                      ingress policer stage 0
*                                      NOTE: ignored if NULL
* @param[out] numEntriesIngressStage1Ptr - (pointer to) The number of metering/billing counters for
*                                      ingress policer stage 1
*                                      NOTE: ignored if NULL
* @param[out] numEntriesEgressStagePtr - (pointer to) The number of metering/billing counters for
*                                      egress policer stage
*                                      NOTE: ignored if NULL
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS prvTgfPolicerMemorySizeModeGet
(
    IN  GT_U8                                      devNum,
    OUT PRV_TGF_POLICER_MEMORY_CTRL_MODE_ENT     *modePtr,
    OUT GT_U32                                     *numEntriesIngressStage0Ptr,
    OUT GT_U32                                     *numEntriesIngressStage1Ptr,
    OUT GT_U32                                     *numEntriesEgressStagePtr
);

/**
* @internal prvTgfPolicerCountingWriteBackCacheFlush function
* @endinternal
*
* @brief   Flush internal Write Back Cache (WBC) of counting entries.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_TIMEOUT               - on time out.
*/
GT_STATUS prvTgfPolicerCountingWriteBackCacheFlush
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerConformanceLevelForce function
* @endinternal
*
* @brief   force the conformance level for the packets entering the policer
*         (traffic cond)
* @param[in] dp                       -  conformance level (drop precedence) - green/yellow/red
*
* @retval GT_OK                    - success, GT_FAIL otherwise
*/
GT_STATUS prvTgfPolicerConformanceLevelForce(
    IN  GT_U32      dp
);

/**
* @internal prvTgfPolicerStageCheck function
* @endinternal
*
* @brief   check if the device supports this policer stage
*
* @param[in] stage                    -  policer  to check
*
* @retval GT_TRUE                  - the device supports the stage
* @retval GT_FALSE                 - the device not supports the stage
*/
GT_BOOL prvTgfPolicerStageCheck(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);


/**
* @internal prvTgfPolicerPortModeAddressSelectSet function
* @endinternal
*
* @brief   Configure Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] type                     - Address select type: Full or Compressed.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or type.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerPortModeAddressSelectSet
(
    IN  GT_U8                                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT                      stage,
    IN  PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT      type
);

/**
* @internal prvTgfPolicerPortModeAddressSelectGet function
* @endinternal
*
* @brief   Get Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] typePtr                  - (pointer to) Address select type: Full or Compressed.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerPortModeAddressSelectGet
(
    IN  GT_U8                                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT                      stage,
    OUT PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT      *typePtr
);

/**
* @internal prvTgfPolicerCountingTriggerByPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Counting when port based metering is triggered for
*         the packet.
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - GT_TRUE - If counting mode is Billing, counting is performed
*                                      for every packet subject to port based metering.
*                                      If counting mode is not Billing, counting is
*                                      performed for these packets only if the Policy
*                                      engine enabled for Counting.
*                                      GT_FALSE - Counting is performed for these packets only
*                                      if the Policy engine enabled for Counting.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerCountingTriggerByPortEnableSet
(
    IN  GT_U8                                   devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT           stage,
    IN GT_BOOL                                  enable
);

/**
* @internal prvTgfPolicerCountingTriggerByPortEnableGet function
* @endinternal
*
* @brief   Get Counting trigger by port status.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - GT_TRUE - If counting mode is Billing, counting is performed
*                                      for every packet subject to port based metering.
*                                      If counting mode is not Billing, counting is
*                                      performed for these packets only if the Policy
*                                      engine enabled for Counting.
*                                      GT_FALSE - Counting is performed for these packets only
*                                      if the Policy engine enabled for Counting.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerCountingTriggerByPortEnableGet
(
    IN  GT_U8                                   devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT          stage,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal prvTgfPolicerEAttributesMeteringModeSet function
* @endinternal
*
* @brief   Set ePort/eVLAN attributes metering mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - ePort/eVLAN Metering mode
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerEAttributesMeteringModeSet
(
    IN GT_U8                                    devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT           stage,
    IN PRV_TGF_POLICER_E_ATTR_METER_MODE_ENT    mode
);

/**
* @internal prvTgfPolicerEAttributesMeteringModeGet function
* @endinternal
*
* @brief   Get ePort/eVLAN attributes metering mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - (pointer to) ePort/eVLAN Metering mode
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfPolicerEAttributesMeteringModeGet
(
    IN GT_U8                                    devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT           stage,
    OUT PRV_TGF_POLICER_E_ATTR_METER_MODE_ENT   *modePtr
);

/**
* @internal prvTgfPolicerTriggerEntrySet function
* @endinternal
*
* @brief   Set Policer trigger entry.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - entry index
* @param[in] entryPtr                 - (pointer to) Policer port trigger entry
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerTriggerEntrySet
(
    IN  GT_U8                                   devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT          stage,
    IN  GT_U32                                  entryIndex,
    IN  PRV_TGF_POLICER_TRIGGER_ENTRY_STC       *entryPtr
);

/**
* @internal prvTgfPolicerTriggerEntryGet function
* @endinternal
*
* @brief   Get Policer trigger entry.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - entry index
*
* @param[out] entryPtr                 - (pointer to) Policer port trigger entry
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerTriggerEntryGet
(
    IN  GT_U8                                   devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT          stage,
    IN  GT_U32                                  entryIndex,
    OUT PRV_TGF_POLICER_TRIGGER_ENTRY_STC       *entryPtr
);

/**
* @internal prvTgfPolicerQosParamsRemarkEntrySet function
* @endinternal
*
* @brief   Sets Policer Re-Marking table Entry.
*         Re-marking is the process of assigning new QoS parameters to the
*         packet, at the end of the metering process, based on the
*         Conformance Level the packet is assigned by metering process.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkProfileTableIndex,
*                                       remarkParamType or confLevel.
*                                       -------------------
*                                       |QoS param | Range |
* @retval |                        ------------------
*                                       | DSCP  | [0:63] |
*                                       | TC   | [0:7] |
*                                       | UP   | [0:7] |
*                                       | EXP   | [0:7] |
*                                       -------------------
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on QoS parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPolicerQosParamsRemarkEntrySet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT           stage,
    IN GT_U32                                   remarkProfileTableIndex,
    IN PRV_TGF_POLICER_REMARK_TABLE_TYPE_ENT    remarkParamType,
    IN GT_U32                                   remarkParamValue,
    IN CPSS_DP_LEVEL_ENT                        confLevel,
    IN PRV_TGF_POLICER_QOS_PARAM_STC            *qosParamPtr
);

/**
* @internal prvTgfPolicerTrappedPacketsBillingEnableSet function
* @endinternal
*
* @brief   Enables the billing algorithm for Trapped packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - enable/disable billing for Trapped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerTrappedPacketsBillingEnableSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT   stage,
    IN GT_BOOL                          enable
);

/**
* @internal prvTgfPolicerLossMeasurementCounterCaptureEnableSet function
* @endinternal
*
* @brief   Enables or disables Loss Measurement capture.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
*/
GT_STATUS prvTgfPolicerLossMeasurementCounterCaptureEnableSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN GT_BOOL                              enable
);

/**
* @internal prvTgfPolicerFlowIdCountingCfgSet function
* @endinternal
*
* @brief   Set Flow Id based counting configuration.
*
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] billingIndexCfgPtr       - (pointer to) billing index configuration structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on out of range data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfPolicerFlowIdCountingCfgSet
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    IN  PRV_TGF_POLICER_BILLING_INDEX_CFG_STC  *billingIndexCfgPtr
);

/**
* @internal prvTgfPolicerMruSet function
* @endinternal
*
* @brief   Sets the Policer Maximum Receive Unit size.
*
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mruSize                  - Policer MRU value in bytes, (APPLICABLE RANGES: 0..0xFFFF).
*                                      This value is used in the metering algorithm.
*                                      When the number of bytes in the bucket is lower than
*                                      this value a packet is marked as non conforming.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_OUT_OF_RANGE          - on mruSize out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfPolicerMruSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN GT_U32                               mruSize
);

/**
* @internal prvTgfPolicerMruGet function
* @endinternal
*
* @brief   Gets the Policer Maximum Receive Unit size.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] mruSizePtr               - pointer to the Policer MRU value in bytes.
*                                      This value is used in the metering algorithm.
*                                      When the number of bytes in the bucket is lower than
*                                      this value a packet is marked as non conforming.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
*/
GT_STATUS prvTgfPolicerMruGet
(
    IN  GT_U8                             devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    OUT GT_U32                            *mruSizePtr
);

/**
* @internal prvTgfPolicerHierarchicalTableEntrySet function
* @endinternal
*
* @brief   Set Hierarchical Table Entry configuration.
*
* @param[in] index                    - Hierarchical Table Entry index.
* @param[in] entryPtr                 - (pointer to) Hierarchical Table Entry structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or index
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on out of range data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Applicable only to Ingress #0 Policer Stage.
*
*/
GT_STATUS prvTgfPolicerHierarchicalTableEntrySet
(
    IN  GT_U32                                          index,
    IN  PRV_TGF_POLICER_HIERARCHICAL_TABLE_ENTRY_STC    *entryPtr
);

/**
* @internal prvTgfPolicerManagementCountersReset function
* @endinternal
*
* @brief   Resets the value of specified Management Counters
*
* @param[in] stage                    - policer stage
* @param[in] mngSet                   - management counter set
*                                       None
*/
GT_VOID prvTgfPolicerManagementCountersReset
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT mngSet
);

/**
* @internal prvTgfPolicerFullManagementCountersCheck function
* @endinternal
*
* @brief   Check Management and Billing counters value
*
* @param[in] stage                    - policer stage
* @param[in] mngSet                   - management counter set
* @param[in] expMngCounters[4]        - expected Management counters value
* @param[in] callIndex                - reference to this check sequence call
*                                       None
*/
GT_VOID prvTgfPolicerFullManagementCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT     mngSet,
    IN GT_U32                               expMngCounters[4][3],
    IN GT_U32                               callIndex
);

/**
* @internal prvTgfPolicerEgressCheckCounters function
* @endinternal
*
* @brief   Check Eth counters according to expected value
*
* @param[in] sendPortIndex            - sending port index
*                                      recPortindx     - receiving port index
* @param[in] expectedValue            - expected number of transmitted packets
* @param[in] callIdentifier           - function call identifier
* @param[in] packetInfoPtr            - the packet format to compare the captured packet with.
*                                       None
*/
GT_VOID prvTgfPolicerEgressCheckCounters
(
    IN GT_U32   sendPortIndex,
    IN GT_U32   recPortIndex,
    IN GT_U32   expectedValue,
    IN GT_U32   callIdentifier,
    IN TGF_PACKET_STC   *packetInfoPtr,
    IN GT_U32            prvTgfBurstCount
);

/**
* @internal prvTgfPolicerEgressInitEntrySet function
* @endinternal
*
* @brief   Initial metering entry setting
*
* @param[in] stage                    - Policer stage
* @param[in] portNum                  - egress port number
*                                       None
*/
GT_VOID prvTgfPolicerEgressInitEntrySet
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    GT_U32                         portNum
);

/**
* @internal prvTgfPolicerEgressTestPacketSend function
* @endinternal
*
* @brief   Send packet to selected port (by port index).
*
* @param[in] sendPortIndex            - sending port indexing place in test array ports.
* @param[in] recPortIndex             - receiving port indexing place in test array ports.
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] prvTgfBurstCount         - number of packets to send.
*                                       None
*/
GT_VOID prvTgfPolicerEgressTestPacketSend
(
    IN GT_U32           sendPortIndex,
    IN GT_U32           recPortIndex,
    IN TGF_PACKET_STC   *packetInfoPtr,
    IN GT_U32           prvTgfBurstCount
);

/**
* @internal prvTgfPolicerEgressIsByteCountCrcExclude function
* @endinternal
*
* @brief   Checks if need exclude 4 bytes CRC from byte count calculation for
*         egress policer. Bobcat2 excludes CRC from byte count.
*
* @retval GT_FALSE                 - do not exclude CRC.
* @retval GT_TRUE                  - exclude CRC from byte count.
*/
GT_BOOL prvTgfPolicerEgressIsByteCountCrcExclude
(
    void
);

/**
* @internal prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] enable                   - GT_TRUE  - Enable: Flow Based Metering to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Metering to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/

GT_STATUS prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableSet
(
    IN  GT_BOOL                       enable
);

/**
* @internal prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Enable: Flow Based Metering to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Metering to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableGet
(
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] enable                   - GT_TRUE  - Enable: Flow Based Billing to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Billing to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableSet
(
    IN  GT_BOOL                       enable
);

/**
* @internal prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Enable: Flow Based Billing to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Billing to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableGet
(
    OUT GT_BOOL                       *enablePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfPolicerGenh */


