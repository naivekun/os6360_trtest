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
* @file cpssDxCh3Policer.h
*
* @brief CPSS DxCh3 Policing Engine API and above.
* The Policing Engine (or Traffic Conditioner) is responsible for:
* - Ingress Metering - implemented using SrTCM Token Bucket,
* TrTCM Token Bucket and Two Color Mode algorithms.
* - Egress Metering - implemented for xCat and above devices
* - Ingress Billing - byte/packet counters that keep track of amount
* of traffic belonging to a flow.
* - Egress Billing - implemented for xCat and above devices
* Re-Marking - QoS assigned to the packet according to metering
* results.
*
* @version   38
********************************************************************************
*/

#ifndef __cpssDxCh3Policerh
#define __cpssDxCh3Policerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/policer/cpssGenPolicerTypes.h>
#include <cpss/generic/cos/cpssCosTypes.h>

/**
* @enum CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT
 *
 * @brief Enumeration for Color Counting mode.
*/
typedef enum{

    /** @brief Color Counting is done according
     *  to the packet's Conformance Level.
     */
    CPSS_DXCH3_POLICER_COLOR_COUNT_CL_E,

    /** @brief Color Counting is done according
     *  to the packet's Drop Precedence.
     */
    CPSS_DXCH3_POLICER_COLOR_COUNT_DP_E

} CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT;

/**
* @enum CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT
 *
 * @brief Billing Counters Mode. This enumeration controls the Billing
 * counters resolution.
*/
typedef enum{

    /** 1 Byte resolution. */
    CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E,

    /** @brief 16 Byte resolution.
     *  In 16 Byte resolution,
     *  Billing counter is rounded
     *  down, i.e. a 64 Billing
     *  counter packet is counted as
     *  4 while a 79 Billing counter
     *  packet is counted as 4 as
     *  well.
     */
    CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E,

    /** packet resolution. */
    CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E

} CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT;

/**
* @enum CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT
 *
 * @brief Packet/Byte Based Meter resolution.
 * This enumeration controls the metering algorithm resolution:
 * packets per second or Bytes per second.
*/
typedef enum{

    /** Byte based Meter resolution. */
    CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E,

    /** @brief Packet based Meter
     *  resolution.
     */
    CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E

} CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT;

/**
* @enum CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT
 *
 * @brief Enumeration for Management counters resolution.
*/
typedef enum{

    /** 1 Byte resolution. */
    CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_1B_E,

    /** 16 Byte resolution. */
    CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_16B_E

} CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT;

/**
* @enum CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT
 *
 * @brief Enumeration for Management Counters Set.
*/
typedef enum{

    /** Management Counters Set #0. */
    CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E,

    /** Management Counters Set #1. */
    CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E,

    /** Management Counters Set #2. */
    CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E,

    /** @brief Management Counters are
     *  disabled for the Policer
     *  Entry.
     *  Notes: each Metering Entry may be associated with one of 3
     *  Management Counters Set, or not at all.
     */
    CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E

} CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT;

/**
* @enum CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT
 *
 * @brief Enumeration for Management Counters Type.
*/
typedef enum{

    /** @brief These Management Counters count
     *  marked GREEN Packet and DU that
     *  passed in this flow.
     */
    CPSS_DXCH3_POLICER_MNG_CNTR_GREEN_E,

    /** @brief These Management Counters count
     *  marked YELLOW Packet and DU that
     *  passed in this flow.
     */
    CPSS_DXCH3_POLICER_MNG_CNTR_YELLOW_E,

    /** @brief These Management Counters count
     *  marked RED Packet and DU that
     *  passed in this flow and were not
     *  dropped.
     */
    CPSS_DXCH3_POLICER_MNG_CNTR_RED_E,

    /** @brief These Management Counters count
     *  marked RED Packet and DU that
     *  passed in this flow and were
     *  dropped.
     */
    CPSS_DXCH3_POLICER_MNG_CNTR_DROP_E

} CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT;

/**
* @enum CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT
 *
 * @brief Enumeration for Policer Entry Type.
*/
typedef enum{

    /** Metering Policer Entry. */
    CPSS_DXCH3_POLICER_ENTRY_METERING_E,

    /** Billing Policer Entry. */
    CPSS_DXCH3_POLICER_ENTRY_BILLING_E

} CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT;

/**
* @enum CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT
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
    CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E,

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
    CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E

} CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT;

/**
* @enum CPSS_DXCH3_POLICER_METER_MODE_ENT
 *
 * @brief Enumeration of the Meter modes.
*/
typedef enum{

    /** @brief a Single Rate Three Color
     *  Marking mode.
     */
    CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E,

    /** @brief a Two Rate Three Color Marking
     *  mode.
     */
    CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E,

    /** @brief Two rate metering according to
     *  MEF with CF=0.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_METER_MODE_MEF0_E,

    /** @brief Two rate metering according to
     *  MEF with CF=1.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_METER_MODE_MEF1_E,

    /** @brief Indicates that this is
     *  the first entry of a MEF 10.3 envelope.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E,

    /** @brief Indicates that this
     *  entry belongs to a MEF 10.3 envelope,
     *  but is not the first entry of the envelope.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E

} CPSS_DXCH3_POLICER_METER_MODE_ENT;

/**
* @enum CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT
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
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E,

    /** @brief DROP.
     *  Nonconforming packets preserve their QoS
     *  parameters setting as set in the previous engine in
     *  the ingress pipe, but their forwarding decision
     *  changes to Soft Drop or Hard Drop according to the
     *  setting of the Policer Drop Mode.
     */
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E,

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
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E,

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
     *  Note: Relevant only for xCat and above.
     */
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E

} CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT;

/**
* @enum CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_ENT
 *
 * @brief Enumeration of the LM Counter Capture Mode.
*/
typedef enum{

    /** In this mode the counter is copied to <Green Counter Snapshot>. */
    CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_COPY_E,

    /** In this mode the counter is inserted into the packet. */
    CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_INSERT_E

} CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_ENT;

/**
* @enum CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_ENT
 *
 * @brief Enumeration of compensation of DSA Tag-added byte count when
 * metering and counting packets are received via DSA-enabled ports.
*/
typedef enum{

    /** @brief Metering and billing operations include the DSA tags
     *  in the packet's Byte Count.
     */
    CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E,

    /** @brief DSA tags are subtracted from the packet byte count before
     *  performing metering or counting.
     */
    CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E

} CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_ENT;

/**
* @enum CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT
 *
 * @brief Configuration for byte counting, which determines whether
 * the timestamp tag should be included in the byte count computation or not.
*/
typedef enum{

    /** @brief In L2 counting mode, if the packet is timestamp tagged,
     *  the timestamp tag is included in the byte count computation.
     */
    CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E,

    /** @brief In L2 counting mode, if the packet is timestamp tagged,
     *  the timestamp tag is NOT included in the byte count computation.
     */
    CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E

} CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT;

/**
* @struct CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC
 *
 * @brief Structure for Policer Management Counters Entry.
*/
typedef struct{

    /** @brief Data Unit Management Counter.
     *  DU size defines according to the assigned resolution
     *  of Management counters (1 Byte or 16 Bytes).
     *  Note: DxCh3 - only 32 LSB are used.
     *  xCat and above - 42 bits are used.
     */
    GT_U64 duMngCntr;

    /** Packet 32 bit size Management Counter. */
    GT_U32 packetMngCntr;

} CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC;


/**
* @struct CPSS_DXCH3_POLICER_BILLING_ENTRY_STC
 *
 * @brief Structure for Policer Billing Entry.
*/
typedef struct{

    /** @brief Green Billing counter, counts the number of
     *  Data Units (1 Byte/16 Bytes) of packets, marked Green
     *  that passed in this flow.
     *  Note: DxCh3 - only 32 LSB are used.
     *  xCat and above - 42 bits are used.
     */
    GT_U64 greenCntr;

    /** @brief Yellow Billing counter, counts the number of
     *  Data Units (1 Byte/16 Bytes) of packets, marked Yellow
     *  that passed in this flow.
     *  Note: DxCh3 - only 32 LSB are used.
     *  xCat and above - 42 bits are used.
     */
    GT_U64 yellowCntr;

    /** @brief Red Billing counter, counts the number of
     *  Data Units (1 Byte/16 Bytes) of packets, marked Red
     *  that passed in this flow.
     *  Note: DxCh3 - only 32 LSB are used.
     *  xCat and above - 42 bits are used.
     */
    GT_U64 redCntr;

    /** @brief The resolution of billing counters.
     *  Applies only to the color counters.
     *  Note: Relevant for xCat and above.
     */
    CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT billingCntrMode;

    /** @brief -  When enabled, packets are counted in the green counter regardless
     *  of their color.
     *  In this case the red and yellow counters are not updated.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL billingCntrAllEnable;

    /** @brief -  Defines whether the LM counter is updated to the Billing table or inserted to the packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_ENT lmCntrCaptureMode;

    /** @brief -  Snapshot of the Green traffic counter.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U64 greenCntrSnapshot;

    /** @brief -  Green Counter Snapshot Validity indication
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL greenCntrSnapshotValid;

    /** @brief -  Type of packet size
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  (APPLICABLE VALUES:
     *  CPSS_POLICER_PACKET_SIZE_L3_ONLY_E
     *  CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E
     *  CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E)
     */
    CPSS_POLICER_PACKET_SIZE_MODE_ENT packetSizeMode;

    /** @brief -  Defines whether counting of TT/TS packets
     *  are performed according to the <Packet Size Mode>;
     *  or according to L3 datagram size only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT tunnelTerminationPacketSizeMode;

    /** @brief -  Enables compensation of DSA Tag-added byte count
     *  when counting packets are received
     *  via DSA-enabled ports.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_ENT dsaTagCountingMode;

    /** @brief -  Configuration for byte counting, which determines whether
     *  the timestamp tag should be included in the byte count
     *  computation or not.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT timeStampCountingMode;

} CPSS_DXCH3_POLICER_BILLING_ENTRY_STC;

/**
* @union CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT
 *
 * @brief Union for single/dual Token Bucket parameters.
 * The set of meter parameters depends on the Meter Mode:
 * - srTcmParams - Single Rate Three Color Marking mode params;
 * - trTcmParams - Two Rate Three Color Marking mode params.
 * trTcmParams also used for MEF0,MEF1,MEF10.3
 * pir/pbs fields should contain EIR/EBS values
 *
*/

typedef union{
    struct {
    /** Committed Information Rate in Kbps (1K = 1000); */
    GT_U32  cir;

    /** Committed Burst Size in bytes; */
    GT_U32  cbs;

    /** Excess Burst Size in bytes; */
    GT_U32  ebs;

    } srTcmParams;

    struct {
    /** Committed Information Rate in Kbps (1K = 1000); */
    GT_U32  cir;

    /** Committed Burst Size in bytes; */
    GT_U32  cbs;

    /** Peak Information Rate in Kbps (1K = 1000); */
    GT_U32  pir;

    /** Peak Burst Size in bytes; */
    GT_U32  pbs;

    } trTcmParams;

    struct {
    /** Committed Information Rate in Kbps (1K = 1000); */
    GT_U32  cir;

    /** Committed Burst Size in bytes; */
    GT_U32  cbs;

    /** Exceed Information Rate in Kbps (1K = 1000); */
    GT_U32  eir;

    /** Exceed Burst Size in bytes; */
    GT_U32  ebs;

    /** Maximal Committed Information Rate in Kbps (1K = 1000); */
    GT_U32  maxCir;

    /** Maximal Exceed Information Rate in Kbps (1K = 1000); */
    GT_U32  maxEir;

    } envelope;

} CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT;


/**
* @enum CPSS_DXCH_POLICER_REMARK_MODE_ENT
 *
 * @brief IP/MPLS Remark mode.
*/
typedef enum{

    /** Use TC/UP for packets remarking. */
    CPSS_DXCH_POLICER_REMARK_MODE_L2_E,

    /** @brief Use DSCP for IP packets remarking
     *  or use EXP for MPLS packets remarking.
     */
    CPSS_DXCH_POLICER_REMARK_MODE_L3_E

} CPSS_DXCH_POLICER_REMARK_MODE_ENT;

/**
* @enum CPSS_DXCH_POLICER_MODIFY_DSCP_ENT
 *
 * @brief Modification mode of the transmitted packets IP Header DSCP.
*/
typedef enum{

    /** @brief Keep previous packet DSCP.
     *  Note: relevant only for Ingress Metering Entry.
     */
    CPSS_DXCH_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E =

    CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,

    /** Disable modification of DSCP. */
    CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E =

    CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,

    /** @brief Enable modification of DSCP.
     *  Note: For APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon,
     *  If the packet is tunneled, this refers to the tunnel header.
     */
    CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E =

    CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E,

    /** @brief Remark packet DSCP.
     *  If the packet is tunneled, this refers to the passenger header.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
     *  Note: relevant only for Egress Metering Entry.
     */
    CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_INNER_E

} CPSS_DXCH_POLICER_MODIFY_DSCP_ENT;

/**
* @enum CPSS_DXCH_POLICER_MODIFY_UP_ENT
 *
 * @brief Modification mode for changing of IEEE 802.1p user priority.
*/
typedef enum{

    /** @brief Keep previous packet UP.
     *  Note: relevant only for Ingress Metering Entry.
     */
    CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E =

    CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,

    /** Disable modification of UP. */
    CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E =

    CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,

    /** @brief Enable modification of UP.
     *  Note: For APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon,
     *  If the packet is tunneled, this refers to the tunnel header tag.
     */
    CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E =

    CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E,

    /** @brief Remark Tag0 UP of the packet.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
     *  Note: relevant only for Egress Metering Entry.
     */
    CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_TAG0_E

} CPSS_DXCH_POLICER_MODIFY_UP_ENT;

/**
* @struct CPSS_DXCH3_POLICER_METERING_ENTRY_STC
 *
 * @brief Structure for Policer Entry Metering Part.
*/
typedef struct{

    /** index of billing or policy counting entry. */
    GT_U32 countingEntryIndex;

    /** @brief affiliation of the Policer Entry to the
     *  Management Counter Set.
     */
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT mngCounterSet;

    /** the meter color mode (blind/aware). */
    CPSS_POLICER_COLOR_MODE_ENT meterColorMode;

    /** meter color marking mode: srTCM or trTCM. */
    CPSS_DXCH3_POLICER_METER_MODE_ENT meterMode;

    /** Token Bucket Parameters. */
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT tokenBucketParams;

    /** Modifying mode of user priority. */
    CPSS_DXCH_POLICER_MODIFY_UP_ENT modifyUp;

    /** @brief Modifying mode for IP DCSP field
     *  Note: For APPLICABLE DEVICES other then
     *  (Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X) also EXP for MPLS.
     */
    CPSS_DXCH_POLICER_MODIFY_DSCP_ENT modifyDscp;

    /** @brief enable/disable modifying DP field.
     *  Note: Supported for xCat and above.
     *  Ignored by DxCh3.
     *  Note: Only Enable/Disable options are valid.
     *  Note: only for Egress Metering Entry.
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyDp;

    /** @brief enable/disable modifying EXP field.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Note: Only Enable/Disable options are valid.
     *  Note: only for Egress Metering Entry.
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyExp;

    /** @brief enable/disable modifying TC field.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Note: Only Enable/Disable options are valid.
     *  Note: only for Egress Metering Entry.
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyTc;

    /** @brief Policer commands for Conforming (Green)
     *  Packets. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
     *  Note: Relevant only for Ingress Metering Entry.
     */
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT greenPcktCmd;

    /** @brief Policer commands for Non
     *  Packets.
     *  Note: Relevant only for Ingress Metering Entry.
     */
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT yellowPcktCmd;

    /** Policer commands for Non */
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT redPcktCmd;

    /** @brief Policer Qos Profile. Relevant for non
     *  packets when yellowPcktCmd or redPcktCmd is
     *  "Qos profile mark by entry".
     *  Note: Supported for xCat and above.
     *  Ignored by DxCh3.
     *  Note: Relevant only for Ingress Metering Entry.
     */
    GT_U32 qosProfile;

    /** @brief Remark mode, controls the QoS remarking table
     *  access for IP and MPLS packets.
     *  Note: Supported for xCat and above.
     *  Ignored by DxCh3.
     *  Note: only for Egress Metering Entry.
     *  Note: Non-IP/MPLS packets always access the
     *  QoS remarking table using the L2 mode.
     *  Note: In L2 mode, TC/UP selection is configured
     *  by cpssDxChPolicerEgressL2RemarkModelSet API.
     */
    CPSS_DXCH_POLICER_REMARK_MODE_ENT remarkMode;

    /** @brief Metering algorithm resolution.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT byteOrPacketCountingMode;

    /** @brief Metering packet\datagram size
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  (APPLICABLE VALUES:
     *  CPSS_POLICER_PACKET_SIZE_L3_ONLY_E
     *  CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E
     *  CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E)
     */
    CPSS_POLICER_PACKET_SIZE_MODE_ENT packetSizeMode;

    /** @brief -  Defines whether metering of TT/TS packets
     *  are performed according to the <Packet Size Mode>;
     *  or according to L3 datagram size only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT tunnelTerminationPacketSizeMode;

    /** @brief -  Enables compensation of DSA Tag-added byte count
     *  when metering packets are received
     *  via DSA-enabled ports.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_ENT dsaTagCountingMode;

    /** @brief -  Configuration for byte metering, which determines whether
     *  the timestamp tag should be included in the byte count
     *  computation or not.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT timeStampCountingMode;

    /** @brief -  enable/disable modification of the transmitted packet's IP
     *  Header ECN field if the traffic has crossed the yellow
     *  threshold.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Note: only for Egress Metering Entry.
     */
    GT_BOOL yellowEcnMarkingEnable;

    /** @brief The coupling flag selects between these options.
     *  GT_FALSE = SPILL TO NEXT: The extra tokens of the CIR bucket
     *  are spilled to the CIR bucket of the next rank.
     *  GT_TRUE = SPILL TO SAME: The extra tokens of the CIR bucket
     *  are spilled to the EIR bucket of the same rank.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL couplingFlag;

    /** @brief index in Max Rate table entry used to limit the actual rate
     *  of buckets0-1.
     *  (APPLICABLE RANGES: 0..127)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 maxRateIndex;

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

} CPSS_DXCH3_POLICER_METERING_ENTRY_STC;

/**
* @enum CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT
 *
 * @brief Enumeration of the Maximal Bucket Envelope Sizes.
*/
typedef enum{

    /** maximal size is 2 entries */
    CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_2_E,

    /** maximal size is 4 entries */
    CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_4_E,

    /** maximal size is 8 entries */
    CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_8_E

} CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT;

/**
* @enum CPSS_DXCH_POLICER_STAGE_TYPE_ENT
 *
 * @brief Enumeration for Policer Unit Type.
*/
typedef enum{

    /** Ingress Policer stage #0. */
    CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,

    /** Ingress Policer stage #1. */
    CPSS_DXCH_POLICER_STAGE_INGRESS_1_E,

    /** Egress Policer stage. */
    CPSS_DXCH_POLICER_STAGE_EGRESS_E

} CPSS_DXCH_POLICER_STAGE_TYPE_ENT;


/**
* @enum CPSS_DXCH_POLICER_COUNTING_MODE_ENT
 *
 * @brief Enumeration for Policer Counting Mode.
*/
typedef enum{

    /** Counting is disabled. */
    CPSS_DXCH_POLICER_COUNTING_DISABLE_E,

    /** @brief Billing (per color counting)
     *  or IPFIX counting.
     *  Counting is triggered by Policy Action Entry
     *  or by Metering Entry.
     *  IPFIX counting is not relevant for xCat2.
     */
    CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E,

    /** @brief Policy activity counting mode:
     *  packet counters only. Counting is triggered by
     *  Policy Action Entry.
     */
    CPSS_DXCH_POLICER_COUNTING_POLICY_E,

    /** @brief VLAN counting mode: packet or
     *  byte counters.
     *  Counting is triggered for every packet according
     *  to VLAN-ID.
     *  (APPLICABLE DEVICES: xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X)
     */
    CPSS_DXCH_POLICER_COUNTING_VLAN_E

} CPSS_DXCH_POLICER_COUNTING_MODE_ENT;


/**
* @enum CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT
 *
 * @brief QoS parameter enumeration
*/
typedef enum{

    /** TC/UP Remark table. */
    CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_TC_UP_E,

    /** DSCP Remark table. */
    CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_DSCP_E,

    /** EXP Remark table. */
    CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_EXP_E

} CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT;

/**
* @enum CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT
 *
 * @brief Enumeration for selecting what algorithm is
 * selected to convert the metering SW parameters
 * to selected HW ones.
*/
typedef enum{

    /** @brief HW metering
     *  parameters are configured such that highest granularity
     *  for CIR\PIR is selected. CBS, EBS and PBS are not used while
     *  choosing the rate type. This is the default method.
     */
    CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E,

    /** @brief HW metering
     *  parameters are configured such that most accurate CBS\PBS
     *  is selected, while the CIR\PIR configured value is in allowed
     *  range derived from the allowed deviation.
     */
    CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E

} CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT;

/**
* @struct CPSS_DXCH_POLICER_QOS_PARAM_STC
 *
 * @brief The Quality of Service parameters.
*/
typedef struct{

    /** IEEE 802.1p User Priority (APPLICABLE RANGES: 0..7) */
    GT_U32 up;

    /** IP DCSP field (APPLICABLE RANGES: 0..63) */
    GT_U32 dscp;

    /** MPLS label EXP value (APPLICABLE RANGES: 0..7) */
    GT_U32 exp;

    /** Drop Precedence value: GREEN, YELLOW or RED. */
    CPSS_DP_LEVEL_ENT dp;

} CPSS_DXCH_POLICER_QOS_PARAM_STC;


/**
* @enum CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT
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
    CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E,

    /** @brief Flow-based metering is performed if
     *  metering is enabled in Policy/TTI Action and
     *  the pointer is taken from the TTI/Policy Action.
     */
    CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E

} CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT;



/**
* @enum CPSS_DXCH_POLICER_STORM_TYPE_ENT
 *
 * @brief Storm Types.
*/
typedef enum{

    /** Knowm Unicast */
    CPSS_DXCH_POLICER_STORM_TYPE_UC_KNOWN_E,

    /** Unknowm Unicast */
    CPSS_DXCH_POLICER_STORM_TYPE_UC_UNKNOWN_E,

    /** Unregistered Multicast */
    CPSS_DXCH_POLICER_STORM_TYPE_MC_UNREGISTERED_E,

    /** Registered Multicast */
    CPSS_DXCH_POLICER_STORM_TYPE_MC_REGISTERED_E,

    /** Broadcast */
    CPSS_DXCH_POLICER_STORM_TYPE_BC_E,

    /** TCP-SYN */
    CPSS_DXCH_POLICER_STORM_TYPE_TCP_SYN_E

} CPSS_DXCH_POLICER_STORM_TYPE_ENT;


/**
* @enum CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT
 *
 * @brief Enumeration for L2 packets remarking model.
*/
typedef enum{

    /** @brief Traffic Class is index in the
     *  QoS table.
     */
    CPSS_DXCH_POLICER_L2_REMARK_MODEL_TC_E,

    /** @brief User Priority is index in the
     *  QoS table.
     */
    CPSS_DXCH_POLICER_L2_REMARK_MODEL_UP_E

} CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT;


#define CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_0_E
#define CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E   CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_1_E
#define CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E   CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_2_E
#define CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_3_E


/**
* @enum CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT
 *
 * @brief Enumeration for Memory control mode.
 * Lower memory uses 256 entries. Upper memory uses the rest.
 * In case of xCat2: memory 1 has 172 entries, memory 2 has
 * 84 entries and memory 3 has 28 entries.
 * In case of Bobcat2, Caelum, Bobcat3, Aldrin, AC3X see functional spec of specific
 * device to get memory sizes.
*/
typedef enum{

    /** @brief Policer 0
     *  uses the upper memory and Policer 1 uses the lower memory.
     *  In case of xCat2, Bobcat2, Caelum, Bobcat3, Aldrin, AC3X: Policer 0 uses two memories (1 and 2),
     *  Policer 1 uses one memory (3).
     */
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_0_E,

    /** @brief Policer 0
     *  uses both memories.
     *  In case of xCat2, Bobcat2, Caelum, Bobcat3, Aldrin, AC3X: Policer 0 uses all three memories.
     */
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_1_E,

    /** @brief Policer 1
     *  uses both memories.
     *  In case of xCat2, Bobcat2, Caelum, Bobcat3, Aldrin, AC3X: Policer 1 uses all three memories.
     */
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_2_E,

    /** @brief Policer 1
     *  uses the upper memory and Policer 0 uses the lower memory.
     *  In case of xCat2, Bobcat2, Caelum, Bobcat3, Aldrin, AC3X: Policer 1 uses two memories (1 and 2),
     *  Policer 0 uses one memory (3).
     */
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_3_E,

    /** @brief Policer 0 uses one memory (1),
     *  Policer 1 uses two memories (2 and 3).
     *  (APPLICABLE DEVICES: xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Note for xCat2: In this mode, when stage mode is Port, a compressed
     *  address select should be configured:
     *  CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E
     *  by cpssDxChPolicerPortModeAddressSelectSet().
     */
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_4_E,

    /** @brief Policer 1 uses one memory (1),
     *  Policer 0 uses two memories (2 and 3).
     *  (APPLICABLE DEVICES: xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Note for xCat2: In this mode, when stage mode is Port, a compressed
     *  address select should be configured:
     *  CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E
     *  by cpssDxChPolicerPortModeAddressSelectSet().
     */
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E,

    /** @brief flexible mode that allow to set any
     *  partitions for ingress policer stage 0 , stage 1 and the egress
     *  policer stage. (see numEntriesIngressStage0,numEntriesIngressStage1
     *  in function cpssDxChPolicerMemorySizeModeSet(...))
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E

} CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT;


/**
* @enum CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT
 *
 * @brief Packet/Byte Vlan counter mode.
*/
typedef enum{

    /** Bytes Vlan counter mode. */
    CPSS_DXCH_POLICER_VLAN_CNTR_MODE_BYTES_E,

    /** Packets Vlan counter mode. */
    CPSS_DXCH_POLICER_VLAN_CNTR_MODE_PACKETS_E

} CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT;

/**
* @enum CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT
 *
 * @brief Port Mode Address Select type.
*/
typedef enum{

    /** The address is (index,port_num). */
    CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E,

    /** @brief Compressed;
     *  The address is (port_num,index).
     */
    CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E

} CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT;

/**
* @enum CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT
 *
 * @brief Enumeration for ePort/eVLAN Metering mode.
*/
typedef enum{

    /** ePort/eVLAN based triggering is disabled. */
    CPSS_DXCH_POLICER_E_ATTR_METER_MODE_DISABLED_E,

    /** ePort based triggering is enabled. */
    CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EPORT_ENABLED_E,

    /** eVLAN based triggering enabled. */
    CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EVLAN_ENABLED_E

} CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT;

/**
* @struct CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC
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

    /** @brief Offset used for unicast known traffic (APPLICABLE RANGES: 0..6).
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

    /** @brief Offset used for unicast unknown traffic (APPLICABLE RANGES: 0..6).
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

    /** @brief Offset used for registered multicast traffic (APPLICABLE RANGES: 0..6).
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

    /** @brief Offset used for unregistered multicast traffic (APPLICABLE RANGES: 0..6).
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

    /** @brief Offset used for broadcast traffic (APPLICABLE RANGES: 0..6).
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

    /** @brief Offset used for TCP SYN traffic (APPLICABLE RANGES: 0..6).
     *  The offset is compared to policerIndex.
     *  Note: Relevant only when tcpSynEnable = GT_TRUE.
     *  Note: In Egress stage, traffic type is ignored,
     *  thus the offset assumed to be 0x0.
     */
    GT_U32 tcpSynOffset;

} CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC;

/**
* @enum CPSS_DXCH_POLICER_BILLING_INDEX_MODE_ENT
 *
 * @brief Mode of Calculation Billing Entry index.
 * APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*/
typedef enum{

    /** @brief The Billing index is determined by the <Policer Index> from the IPCL/TTI/EPCL
     *  or by the index from the Metering table.
     */
    CPSS_DXCH_POLICER_BILLING_INDEX_MODE_STANDARD_E,

    /** @brief If the FlowID is in the configured range, the Billing index is
     *  determined by the packets Flow-ID. Otherwise,
     *  the index is determined in the standard way.
     */
    CPSS_DXCH_POLICER_BILLING_INDEX_MODE_FLOW_ID_E

} CPSS_DXCH_POLICER_BILLING_INDEX_MODE_ENT;

/**
* @struct CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC
 *
 * @brief Configuration of Calculation Billing Entry index.
*/
typedef struct{

    /** @brief Mode of Calculation Billing Entry index.
     *  billingIndexMode == CPSS_DXCH_POLICER_BILLING_INDEX_MODE_FLOW_ID_E
     *  but always written/read to/from HW.
     *  The formula for Billing Entry index in this mode is
     */
    CPSS_DXCH_POLICER_BILLING_INDEX_MODE_ENT billingIndexMode;

    /** @brief When the Billing entry is accessed using the packets Flow
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

} CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC;

/**
* @struct CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC
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

} CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC;


/**
* @struct CPSS_DXCH_POLICER_BUCKETS_STATE_STC
 *
 * @brief Current Metering Buckets state.
*/
typedef struct{

    /** last Update Time of bucket0. */
    GT_U32 lastTimeUpdate0;

    /** last Update Time of bucket1. */
    GT_U32 lastTimeUpdate1;

    /** wrap around of bucket0. */
    GT_BOOL wrapAround0;

    /** wrap around of bucket1. */
    GT_BOOL wrapAround1;

    /** current size of bucket0. */
    GT_U32 bucketSize0;

    /** current size of bucket1. */
    GT_U32 bucketSize1;

    /** @brief metering sign of bucket0 is positive.
     *  (APPLICABLE DEVICES: Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL bucketSignPositive0;

    /** @brief metering sign of bucket1 is positive.
     *  (APPLICABLE DEVICES: Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL bucketSignPositive1;

} CPSS_DXCH_POLICER_BUCKETS_STATE_STC;


/**
* @internal cpssDxCh3PolicerMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables metering per device.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] enable                   - Enable/disable metering:
*                                      GT_TRUE  - metering is enabled on the device.
*                                      GT_FALSE - metering is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note If metering is enabled, it can be triggered
*       either by Policy engine or per port.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_BOOL                              enable
);

/**
* @internal cpssDxCh3PolicerMeteringEnableGet function
* @endinternal
*
* @brief   Gets device metering status (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] enablePtr                - pointer to Enable/Disable metering:
*                                      GT_TRUE  - metering is enabled on the device.
*                                      GT_FALSE - metering is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note If metering is enabled, it can be triggered
*       either by Policy engine or per port.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxCh3PolicerBillingCountingEnableSet function
* @endinternal
*
* @brief   Enables or disables Billing Counting.
*         Billing is the process of counting the amount of traffic that
*         belongs to a flow and can be activated only by metering.
*         When metering is activated per source port, Billing counting
*         cannot be enabled.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - enable/disable Billing Counting:
*                                      GT_TRUE  - Billing Counting is enabled on the device.
*                                      GT_FALSE - Billing Counting is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerBillingCountingEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxCh3PolicerBillingCountingEnableGet function
* @endinternal
*
* @brief   Gets device Billing Counting status (Enable/Disable).
*         Billing is the process of counting the amount of traffic that
*         belongs to a flow and can be activated only by metering.
*         When metering is activated per source port, Billing counting
*         cannot be enabled.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - pointer to Enable/Disable Billing Counting:
*                                      GT_TRUE  - Billing Counting is enabled on the device.
*                                      GT_FALSE - Billing Counting is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerBillingCountingEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChPolicerCountingModeSet function
* @endinternal
*
* @brief   Configures counting mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
* @param[in] mode                     -  counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerCountingModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_POLICER_COUNTING_MODE_ENT      mode
);

/**
* @internal cpssDxChPolicerCountingModeGet function
* @endinternal
*
* @brief   Gets the couning mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
*
* @param[out] modePtr                  - pointer to Counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerCountingModeGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_COUNTING_MODE_ENT     *modePtr
);

/**
* @internal cpssDxCh3PolicerBillingCountersModeSet function
* @endinternal
*
* @brief   Sets the Billing Counters resolution.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] cntrMode                 - Billing Counters resolution: 1 Byte,
*                                      16 Byte or packet.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or cntrMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerBillingCountersModeSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT     cntrMode
);


/**
* @internal cpssDxCh3PolicerBillingCountersModeGet function
* @endinternal
*
* @brief   Gets the Billing Counters resolution.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] cntrModePtr              - pointer to the Billing Counters resolution:
*                                      1 Byte, 16 Byte or packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerBillingCountersModeGet
(
    IN GT_U8                                        devNum,
    OUT CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT    *cntrModePtr
);

/**
* @internal cpssDxCh3PolicerPacketSizeModeSet function
* @endinternal
*
* @brief   Sets metered Packet Size Mode that metering and billing is done
*         according to.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] packetSize               - Type of packet size:
*                                      - L3 datagram size only (this does not include
*                                      the L2 header size and packets CRC).
*                                      - L2 packet length including.
*                                      - L1 packet length including
*                                      (preamble + IFG + FCS).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or packetSize.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPacketSizeModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_POLICER_PACKET_SIZE_MODE_ENT    packetSize
);

/**
* @internal cpssDxCh3PolicerPacketSizeModeGet function
* @endinternal
*
* @brief   Gets metered Packet Size Mode that metering and billing is done
*         according to.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] packetSizePtr            - pointer to the Type of packet size:
*                                      - L3 datagram size only (this does not include
*                                      the L2 header size and packets CRC).
*                                      - L2 packet length including.
*                                      - L1 packet length including (preamble + IFG + FCS).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPacketSizeModeGet
(
    IN  GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT      stage,
    OUT CPSS_POLICER_PACKET_SIZE_MODE_ENT    *packetSizePtr
);

/**
* @internal cpssDxCh3PolicerMeterResolutionSet function
* @endinternal
*
* @brief   Sets metering algorithm resolution: packets per
*         second or bytes per second.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] resolution               - packet/Byte based Meter resolution.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or resolution.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeterResolutionSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT  resolution
);

/**
* @internal cpssDxCh3PolicerMeterResolutionGet function
* @endinternal
*
* @brief   Gets metering algorithm resolution: packets per
*         second or bytes per second.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] resolutionPtr            - pointer to the Meter resolution: packet or Byte based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeterResolutionGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT *resolutionPtr
);

/**
* @internal cpssDxCh3PolicerDropTypeSet function
* @endinternal
*
* @brief   Sets the Policer out-of-profile drop command type.
*         This setting controls if non-conforming dropped packets
*         (Red or Yellow) will be SOFT DROP or HARD DROP.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] dropType                 - Policer Drop Type: Soft or Hard.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, dropType or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerDropTypeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_DROP_MODE_TYPE_ENT              dropType
);

/**
* @internal cpssDxCh3PolicerDropTypeGet function
* @endinternal
*
* @brief   Gets the Policer out-of-profile drop command type.
*         This setting controls if non-conforming dropped packets
*         (Red or Yellow) will be SOFT DROP or HARD DROP.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] dropTypePtr              - pointer to the Policer Drop Type: Soft or Hard.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerDropTypeGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DROP_MODE_TYPE_ENT             *dropTypePtr
);

/**
* @internal cpssDxCh3PolicerCountingColorModeSet function
* @endinternal
*
* @brief   Sets the Policer color counting mode.
*         The Color Counting can be done according to the packet's
*         Drop Precedence or Conformance Level.
*         This affects both the Billing and Management counters.
*         If the packet was subject to remarking, the drop precedence used
*         here is AFTER remarking.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mode                     - Color counting mode: Drop Precedence or
*                                      Conformance Level.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerCountingColorModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT  mode
);

/**
* @internal cpssDxCh3PolicerCountingColorModeGet function
* @endinternal
*
* @brief   Gets the Policer color counting mode.
*         The Color Counting can be done according to the packet's
*         Drop Precedence or Conformance Level.
*         This affects both the Billing and Management counters.
*         If the packet was subject to remarking, the drop precedence used
*         here is AFTER remarking.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] modePtr                  - pointer to the color counting mode:
*                                      Drop Precedence or Conformance Level.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerCountingColorModeGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT     *modePtr
);

/**
* @internal cpssDxCh3PolicerManagementCntrsResolutionSet function
* @endinternal
*
* @brief   Sets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] cntrSet                  - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] cntrResolution           - Management Counters resolution: 1 or 16 Bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or
*                                       Management Counters Set or cntrResolution.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerManagementCntrsResolutionSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT          cntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT   cntrResolution
);

/**
* @internal cpssDxCh3PolicerManagementCntrsResolutionGet function
* @endinternal
*
* @brief   Gets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] cntrSet                  - Management Counters Set (APPLICABLE RANGES: 0..2).
*
* @param[out] cntrResolutionPtr        - pointer to the Management Counters
*                                      resolution: 1 or 16 Bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or Mng Counters Set.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerManagementCntrsResolutionGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT         cntrSet,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT  *cntrResolutionPtr
);

/**
* @internal cpssDxCh3PolicerPacketSizeModeForTunnelTermSet function
* @endinternal
*
* @brief   Sets size mode for metering and counting of tunnel terminated packets.
*         The Policer provides the following modes to define packet size:
*         - Regular packet metering and counting. The packet size is defined
*         by the cpssDxCh3PolicerPacketSizeModeSet().
*         - Passenger packet metering and counting.
*         Metering and counting of TT packets is performed according
*         to L3 datagram size only. This mode does not include the tunnel
*         header size, the L2 header size, and the packet CRC in the metering
*         and counting.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] ttPacketSizeMode         - Tunnel Termination Packet Size Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or ttPacketSizeMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPacketSizeModeForTunnelTermSet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT  ttPacketSizeMode
);

/**
* @internal cpssDxCh3PolicerPacketSizeModeForTunnelTermGet function
* @endinternal
*
* @brief   Gets size mode for metering and counting of tunnel terminated packets.
*         The Policer provides the following modes to define packet size:
*         - Regular packet metering and counting. The packet size is defined
*         by the cpssDxCh3PolicerPacketSizeModeSet().
*         - Passenger packet metering and counting.
*         Metering and counting of TT packets is performed according
*         to L3 datagram size only. This mode does not include the tunnel
*         header size, the L2 header size, and the packet CRC in the metering
*         and counting.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] ttPacketSizeModePtr      - pointer to the Tunnel Termination
*                                      Packet Size Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPacketSizeModeForTunnelTermGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT  *ttPacketSizeModePtr
);

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet function
* @endinternal
*
* @brief   Enables or disables the metering Auto Refresh Scan mechanism.
*         If no traffic passes through a meter (long periods of "silence") the
*         meter state variables do not get refreshed. This can cause a problem of
*         state misinterpreted once the flow traffic is restarted, because the
*         timers can have already wrapped around.
*         To avoid this problem the software must initiate a refresh transaction
*         on the meter every ten minutes.
*         This CPU intensive metering refresh maintanance can be offloaded by
*         Auto Refresh Scan mechanism.
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - Enable/disable Auto Refresh Scan mechanism:
*                                      GT_TRUE  - enable Auto Refresh scan mechanism on
*                                      the device.
*                                      GT_FALSE - disable Auto Refresh scan mechanism on
*                                      the device.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The CPU is still capable to perform any Refresh Transactions during
*       the Auto Refresh Scan operation.
*       Once the CPU triggers Refresh transaction, the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet
(
    IN GT_U8                                devNum,
    IN GT_BOOL                              enable
);

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet function
* @endinternal
*
* @brief   Gets metering Auto Refresh Scan mechanism status (Enabled/Disabled).
*         If no traffic passes through a meter (long periods of "silence") the
*         meter state variables do not get refreshed. This can cause a problem of
*         state misinterpreted once the flow traffic is restarted, because the
*         timers can have already wrapped around.
*         To avoid this problem the software must initiate a refresh transaction
*         on the meter every ten minutes.
*         This CPU intensive metering refresh maintanance can be offloaded by
*         Auto Refresh Scan mechanism.
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The CPU is still capable to perform any Refresh Transactions during
*       the Auto Refresh Scan operation.
*       Once the CPU triggers Refresh transaction, the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshRangeSet function
* @endinternal
*
* @brief   Sets Metering Refresh Scan address range (Start and Stop addresses).
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] startAddress             - beginning of the address range to be scanned
*                                      by the Auto Refresh Scan mechanism (APPLICABLE RANGES: 0..1023).
* @param[in] stopAddress              - end of the address range to be scanned by the
*                                      Auto Refresh Scan mechanism (APPLICABLE RANGES: 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or illegal values
*                                       of startAddress and stopAddress.
* @retval GT_OUT_OF_RANGE          - on out of range of startAddress
*                                       or stopAddress.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshRangeSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               startAddress,
    IN GT_U32                               stopAddress
);

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshRangeGet function
* @endinternal
*
* @brief   Gets Metering Refresh Scan address range (Start and Stop addresses).
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshRangeGet
(
    IN GT_U8                                devNum,
    OUT GT_U32                              *startAddressPtr,
    OUT GT_U32                              *stopAddressPtr
);

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshIntervalSet function
* @endinternal
*
* @brief   Sets the time interval between two refresh access to metering table.
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] interval                 - time  between refresh of two entries
*                                      in micro seconds. Upon 270 Mhz core clock value
*                                      of DxCh3, (APPLICABLE RANGES: 0..15907286) microS.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on out of range of interval value.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshIntervalSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               interval
);

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshIntervalGet function
* @endinternal
*
* @brief   Gets the time interval between two refresh access to metering table.
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] intervalPtr              - pointer to the time interval between refresh of two
*                                      entries in micro seconds. Upon 270 Mhz core clock
*                                      value of DxCh3, (APPLICABLE RANGES: 0..15907286) microS.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshIntervalGet
(
    IN GT_U8                                devNum,
    OUT GT_U32                              *intervalPtr
);

/**
* @internal cpssDxCh3PolicerMeteringEntryRefresh function
* @endinternal
*
* @brief   Refresh the Policer Metering Entry.
*         Meter Bucket's state parameters refreshing are needed in order to
*         prevent mis-behavior due to wrap around of timers.
*         The wrap around problem can occur when there are long periods of
*         'silence' on the flow and the bucket's state parameters do not get
*         refreshed (meter is in the idle state). This causes a problem once the
*         flow traffic is re-started since the timers might have already wrapped
*         around which can cause a situation in which the bucket's state is
*         mis-interpreted and the incoming packet is marked as non-conforming
*         even though the bucket was actually supposed to be full.
*         To prevent this from happening the CPU needs to trigger a meter
*         refresh transaction at least once every 10 minutes per meter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Once the CPU triggers Refresh transaction the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringEntryRefresh
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              entryIndex
);

/**
* @internal cpssDxCh3PolicerPortMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables a port metering trigger for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] enable                   - Enable/Disable per-port metering for packets arriving
*                                      on this port:
*                                      GT_TRUE  - Metering is triggered on specified port.
*                                      GT_FALSE - Metering isn't triggered on specified port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPortMeteringEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_BOOL                              enable
);

/**
* @internal cpssDxCh3PolicerPortMeteringEnableGet function
* @endinternal
*
* @brief   Gets port status (Enable/Disable) of metering for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPortMeteringEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxCh3PolicerMruSet function
* @endinternal
*
* @brief   Sets the Policer Maximum Receive Unit size.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
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
GT_STATUS cpssDxCh3PolicerMruSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               mruSize
);

/**
* @internal cpssDxCh3PolicerMruGet function
* @endinternal
*
* @brief   Gets the Policer Maximum Receive Unit size.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMruGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *mruSizePtr
);

/**
* @internal cpssDxCh3PolicerErrorGet function
* @endinternal
*
* @brief   Gets address and type of Policer Entry that had an ECC error.
*         This information available if error is happened and not read till now.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerErrorGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT   *entryTypePtr,
    OUT GT_U32                              *entryAddrPtr
);

/**
* @internal cpssDxCh3PolicerErrorCounterGet function
* @endinternal
*
* @brief   Gets the value of the Policer ECC Error Counter.
*         The Error Counter is a free-running non-sticky 8-bit read-only
*         counter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] cntrValuePtr             - pointer to the Policer ECC Error counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerErrorCounterGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *cntrValuePtr
);

/**
* @internal cpssDxCh3PolicerManagementCountersSet function
* @endinternal
*
* @brief   Sets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
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
*       and 42 bits are used for xCat and above devices.
*
*/
GT_STATUS cpssDxCh3PolicerManagementCountersSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT      mngCntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT     mngCntrType,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC    *mngCntrPtr
);

/**
* @internal cpssDxCh3PolicerManagementCountersGet function
* @endinternal
*
* @brief   Gets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
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
*       and 42 bits are used for xCat and above devices.
*
*/
GT_STATUS cpssDxCh3PolicerManagementCountersGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     mngCntrSet,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT    mngCntrType,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC   *mngCntrPtr
);

/**
* @internal cpssDxCh3PolicerMeteringEntrySet function
* @endinternal
*
* @brief   Sets Metering Policer Entry.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] entryPtr                 - pointer to the metering policer entry going to be set.
*
* @param[out] tbParamsPtr              - pointer to actual policer token bucket parameters.
*                                      The token bucket parameters are returned as output
*                                      values. This is due to the hardware rate resolution,
*                                      the exact rate or burst size requested may not be
*                                      honored. The returned value gives the user the
*                                      actual parameters configured in the hardware.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex or
*                                       entry parameters.
* @retval GT_OUT_OF_RANGE          - on Billing Entry Index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Should not be used to update member of MEF 10.3 Envelope.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringEntrySet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbParamsPtr
);

/**
* @internal cpssDxCh3PolicerMeteringEntryGet function
* @endinternal
*
* @brief   Gets Metering Policer Entry configuration.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @param[out] entryPtr                 - pointer to the requested metering policer entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
GT_STATUS cpssDxCh3PolicerMeteringEntryGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr
);

/**
* @internal cpssDxCh3PolicerEntryMeterParamsCalculate function
* @endinternal
*
* @brief   Calculates Token Bucket parameters in the Application format without
*         HW update.
*         The token bucket parameters are returned as output values. This is
*         due to the hardware rate resolution, the exact rate or burst size
*         requested may not be honored. The returned value gives the user the
*         actual parameters configured in the hardware.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] tbInParamsPtr            - pointer to Token bucket input parameters.
* @param[in] meterMode                - Meter mode (SrTCM or TrTCM).
*
* @param[out] tbOutParamsPtr           - pointer to Token bucket output paramters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong devNum or meterMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerEntryMeterParamsCalculate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  CPSS_DXCH3_POLICER_METER_MODE_ENT       meterMode,
    IN  CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbInParamsPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbOutParamsPtr
);

/**
* @internal cpssDxCh3PolicerBillingEntrySet function
* @endinternal
*
* @brief   Sets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
* @param[in] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out upon counter reset by
*                                       indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*       In order to set Billing entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxChIpfixEntrySet".
*
*/
GT_STATUS cpssDxCh3PolicerBillingEntrySet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
);

/**
* @internal cpssDxCh3PolicerBillingEntryGet function
* @endinternal
*
* @brief   Gets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*/
GT_STATUS cpssDxCh3PolicerBillingEntryGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  GT_BOOL                                 reset,
    OUT CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
);

/**
* @internal cpssDxCh3PolicerQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colors.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Bobcat3, Aldrin, AC3X Qos Remarking Entry contains three indexes of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
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
*                                      the Yellow color.
*                                      (APPLICABLE RANGES: DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin, Aldrin2, Falcon, AC3X 0..1023).
* @param[in] redQosTableRemarkIndex   - QoS profile (index in the Qos Table)
*                                      assigned to Out-Of-Profile packets with
*                                      the Red color.
*                                      (APPLICABLE RANGES: DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
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
GT_STATUS cpssDxCh3PolicerQosRemarkingEntrySet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               qosProfileIndex,
    IN GT_U32                               greenQosTableRemarkIndex,
    IN GT_U32                               yellowQosTableRemarkIndex,
    IN GT_U32                               redQosTableRemarkIndex
);

/**
* @internal cpssDxCh3PolicerQosRemarkingEntryGet function
* @endinternal
*
* @brief   Gets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colours.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Bobcat3, Aldrin, AC3X Qos Remarking Entry contains three indexes of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
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
GT_STATUS cpssDxCh3PolicerQosRemarkingEntryGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               qosProfileIndex,
    OUT GT_U32                              *greenQosTableRemarkIndexPtr,
    OUT GT_U32                              *yellowQosTableRemarkIndexPtr,
    OUT GT_U32                              *redQosTableRemarkIndexPtr
);

/**
* @internal cpssDxChPolicerEgressQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Egress Policer Re-Marking table Entry.
*         Re-marking is the process of assigning new QoS parameters to the
*         packet, at the end of the metering process, based on the
*         Conformance Level the packet is assigned by metering process.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] remarkTableType          - Remark table type: DSCP, EXP or TC/UP.
* @param[in] remarkParamValue         - QoS parameter value.
* @param[in] confLevel                - Conformance Level: Green, Yellow, Red.
* @param[in] qosParamPtr              - pointer to the Re-Marking Entry going to be set.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkTableType,
*                                       remarkParamValue or confLevel.
*                                       -------------------
*                                       |QoS param | Range |
* @retval |                        ------------------
*                                       | DSCP  | [0:63] |
*                                       | TC/UP  | [0:7] |
*                                       | EXP   | [0:7] |
*                                       -------------------
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on QoS parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEgressQosRemarkingEntrySet
(
    IN GT_U8                                        devNum,
    IN  CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT     remarkTableType,
    IN GT_U32                                       remarkParamValue,
    IN CPSS_DP_LEVEL_ENT                            confLevel,
    IN CPSS_DXCH_POLICER_QOS_PARAM_STC              *qosParamPtr
);

/**
* @internal cpssDxChPolicerEgressQosRemarkingEntryGet function
* @endinternal
*
* @brief   Gets Egress Policer Re-Marking table Entry.
*         Re-marking is the process of assigning new QoS parameters to the
*         packet, at the end of the metering process, based on the
*         Conformance Level the packet is assigned by metering process.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
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
*                                       -------------------
*                                       |QoS param | Range |
* @retval |                        ------------------
*                                       | DSCP  | [0:63] |
*                                       | TC/UP  | [0:7] |
*                                       | EXP   | [0:7] |
*                                       -------------------
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
GT_STATUS cpssDxChPolicerEgressQosRemarkingEntryGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT     remarkTableType,
    IN  GT_U32                                      remarkParamValue,
    IN  CPSS_DP_LEVEL_ENT                           confLevel,
    OUT CPSS_DXCH_POLICER_QOS_PARAM_STC             *qosParamPtr
);

/**
* @internal cpssDxChPolicerStageMeterModeSet function
* @endinternal
*
* @brief   Sets Policer Global stage mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - Policer meter mode: FLOW or PORT.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In FLOW mode both the Policy and the port-trigger commands are
*       considered and in case of contention, the Policy command takes
*       precedence.
*
*/
GT_STATUS cpssDxChPolicerStageMeterModeSet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT      mode
);

/**
* @internal cpssDxChPolicerStageMeterModeGet function
* @endinternal
*
* @brief   Gets Policer Global stage mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In FLOW mode both the Policy and the port-trigger commands are
*       considered and in case of contention, the Policy command takes
*       precedence.
*
*/
GT_STATUS cpssDxChPolicerStageMeterModeGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT      *modePtr
);

/**
* @internal cpssDxChPolicerMeteringOnTrappedPktsEnableSet function
* @endinternal
*
* @brief   Enable/Disable Ingress metering for Trapped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The Ingress Policer considers data traffic as any packet that is not
*       trapped or dropped.
*
*/
GT_STATUS cpssDxChPolicerMeteringOnTrappedPktsEnableSet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN GT_BOOL                                  enable
);

/**
* @internal cpssDxChPolicerMeteringOnTrappedPktsEnableGet function
* @endinternal
*
* @brief   Get Ingress metering status (Enabled/Disabled) for Trapped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The Ingress Policer considers data traffic as any packet that is not
*       trapped or dropped.
*
*/
GT_STATUS cpssDxChPolicerMeteringOnTrappedPktsEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChPolicerPortStormTypeIndexSet function
* @endinternal
*
* @brief   Associates policing profile with source/target port and storm rate type.
*         There are up to four policers per port and every storm type can be
*         flexibly associated with any of the four policers.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] stormType                - storm type
* @param[in] index                    - policer  (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, stormType ot port.
* @retval GT_OUT_OF_RANGE          - on out of range of index.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortStormTypeIndexSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN CPSS_DXCH_POLICER_STORM_TYPE_ENT     stormType,
    IN GT_U32                               index
);


/**
* @internal cpssDxChPolicerPortStormTypeIndexGet function
* @endinternal
*
* @brief   Get policing profile for given source/target port and storm rate type.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortStormTypeIndexGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_POLICER_STORM_TYPE_ENT    stormType,
    OUT GT_U32                              *indexPtr
);

/**
* @internal cpssDxChPolicerEgressL2RemarkModelSet function
* @endinternal
*
* @brief   Sets Egress Policer L2 packets remarking model.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] model                    - L2 packets remarking model. It defines QoS parameter
*                                      which used as index in the Qos table:
*                                      User Priority or Traffic Class.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEgressL2RemarkModelSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT    model
);

/**
* @internal cpssDxChPolicerEgressL2RemarkModelGet function
* @endinternal
*
* @brief   Gets Egress Policer L2 packets remarking model.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] modelPtr                 - L2 packets remarking model. It defines QoS parameter,
*                                      which used as index in the Qos table:
*                                      User Priority or Traffic Class.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEgressL2RemarkModelGet
(
    IN  GT_U8                                    devNum,
    OUT CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT    *modelPtr
);

/**
* @internal cpssDxChPolicerEgressQosUpdateEnableSet function
* @endinternal
*
* @brief   The function enables or disables QoS remarking of conforming packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - Enable/disable Qos update for conforming packets:
*                                      GT_TRUE  - Remark Qos parameters of conforming packets.
*                                      GT_FALSE - Keep incoming Qos parameters of conforming packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEgressQosUpdateEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPolicerEgressQosUpdateEnableGet function
* @endinternal
*
* @brief   The function get QoS remarking status of conforming packets
*         (enable/disable).
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - Enable/disable Qos update for conforming packets:
*                                      GT_TRUE  - Remark Qos parameters of conforming packets.
*                                      GT_FALSE - Keep incoming Qos parameters of conforming
*                                      packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEgressQosUpdateEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPolicerVlanCntrSet function
* @endinternal
*
* @brief   Sets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
*         When working in VLAN Counting Mode each counter counts the number of
*         packets or bytes according to the VLAN counters mode set
*         by cpssDxChPolicerVlanCountingModeSet(). Index of counter is VLAN ID.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Bobcat3; Aldrin2; Falcon.
*
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The VLAN counters are free-running no-sticky counters.
*       In order to set Vlan counting entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
GT_STATUS cpssDxChPolicerVlanCntrSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U16                              vid,
    IN  GT_U32                              cntrValue
);

/**
* @internal cpssDxChPolicerVlanCntrGet function
* @endinternal
*
* @brief   Gets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
*         When working in VLAN Counting Mode each counter counts the number of
*         packets or bytes according to the VLAN counters mode set
*         by cpssDxChPolicerVlanCountingModeSet(). Index of counter is VLAN ID.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Bobcat3; Aldrin2; Falcon.
*
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The VLAN counters are free-running no-sticky counters.
*
*/
GT_STATUS cpssDxChPolicerVlanCntrGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U16                              vid,
    OUT GT_U32                              *cntrValuePtr
);

/**
* @internal cpssDxChPolicerVlanCountingPacketCmdTriggerSet function
* @endinternal
*
* @brief   Enables or disables VLAN counting triggering according to the
*         specified packet command.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] cmdTrigger               - The packet command trigger.
*                                      The following packet command are supported:
*                                      Forward (CPSS_PACKET_CMD_FORWARD_E),
*                                      Hard Drop (CPSS_PACKET_CMD_DROP_HARD_E),
*                                      Soft Drop (CPSS_PACKET_CMD_DROP_SOFT_E),
*                                      Trap (CPSS_PACKET_CMD_TRAP_TO_CPU_E),
*                                      Mirror to CPU (CPSS_PACKET_CMD_MIRROR_TO_CPU_E).
* @param[in] enable                   - Enable/Disable VLAN Counting according to the packet
*                                      command trigger:
*                                      GT_TRUE  - Enable VLAN counting on packet with
*                                      packet command trigger.
*                                      GT_FALSE - Disable VLAN counting on packet with
*                                      packet command trigger.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported egress direction
*
* @note All packet commands are enabled in the HW by default.
*       For egress policer the default trigger values shouldn't be changed.
*
*/
GT_STATUS cpssDxChPolicerVlanCountingPacketCmdTriggerSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_PACKET_CMD_ENT                  cmdTrigger,
    IN GT_BOOL                              enable
);

/**
* @internal cpssDxChPolicerVlanCountingPacketCmdTriggerGet function
* @endinternal
*
* @brief   Gets VLAN counting triggering status (Enable/Disable) according to the
*         specified packet command.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] cmdTrigger               - The packet command trigger.
*                                      The following packetcommand are supported:
*                                      Forward (CPSS_PACKET_CMD_FORWARD_E),
*                                      Hard Drop (CPSS_PACKET_CMD_DROP_HARD_E),
*                                      Soft Drop (CPSS_PACKET_CMD_DROP_SOFT_E),
*                                      Trap (CPSS_PACKET_CMD_TRAP_TO_CPU_E),
*                                      Mirror to CPU (CPSS_PACKET_CMD_MIRROR_TO_CPU_E).
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All packet commands are enabled in the HW by default.
*
*/
GT_STATUS cpssDxChPolicerVlanCountingPacketCmdTriggerGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  CPSS_PACKET_CMD_ENT                 cmdTrigger,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChPolicerVlanCountingModeSet function
* @endinternal
*
* @brief   Sets the Policer VLAN counters mode as byte or packet based.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - VLAN counters mode: Byte or Packet based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerVlanCountingModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT     mode
);

/**
* @internal cpssDxChPolicerVlanCountingModeGet function
* @endinternal
*
* @brief   Gets the Policer VLAN counters mode as byte or packet based.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerVlanCountingModeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT     *modePtr
);

/**
* @internal cpssDxChPolicerPolicyCntrSet function
* @endinternal
*
* @brief   Sets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
* @param[in] cntrValue                - packets counter.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index (index range is limited by
*                                       max number of Policer Policy counters).
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*       In order to set Policy counting entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
GT_STATUS cpssDxChPolicerPolicyCntrSet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    IN  GT_U32                              cntrValue
);

/**
* @internal cpssDxChPolicerPolicyCntrGet function
* @endinternal
*
* @brief   Gets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
*
* @param[out] cntrValuePtr             - Pointer to the packet counter.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*
*/
GT_STATUS cpssDxChPolicerPolicyCntrGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    OUT GT_U32                              *cntrValuePtr
);

/**
* @internal cpssDxChPolicerMemorySizeModeSet function
* @endinternal
*
* @brief   Sets internal table sizes and the way they are shared between the Ingress
*         policers.
*         In Bobcat3 the 'flex mode' allow to control partitions also with the egress
*         policer.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     - The  in which internal tables are shared.
* @param[in] numEntriesIngressStage0  - The number of metering/billing counters for
*                                      ingress policer stage 0
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*                                      (APPLICABLE DEVICES Bobcat3)
* @param[in] numEntriesIngressStage1  - The number of metering/billing counters for
*                                      ingress policer stage 1
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon)
*                                      NOTE: the egress policer gets the 'rest' of the metering/billing counters
*                                      the that device support.
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mode change should not be done when meter or counter entries are in use.
*
*/
GT_STATUS cpssDxChPolicerMemorySizeModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT      mode,
    IN  GT_U32                                      numEntriesIngressStage0,
    IN  GT_U32                                      numEntriesIngressStage1
);

/**
* @internal cpssDxChPolicerMemorySizeModeGet function
* @endinternal
*
* @brief   Gets internal table sizes and the way they are shared between the Ingress
*         policers.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerMemorySizeModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT     *modePtr,
    OUT GT_U32                                     *numEntriesIngressStage0Ptr,
    OUT GT_U32                                     *numEntriesIngressStage1Ptr,
    OUT GT_U32                                     *numEntriesEgressStagePtr
);

/**
* @internal cpssDxChPolicerCountingWriteBackCacheFlush function
* @endinternal
*
* @brief   Flush internal Write Back Cache (WBC) of counting entries.
*         The Policer implements internal Write Back Cache for frequent and rapid
*         update of counting entries.
*         Since the WBC holds the policer entries' data, the CPU must clear its
*         content when performing direct write access of the counting entries by
*         one of following functions:
*         - cpssDxCh3PolicerBillingEntrySet
*         - cpssDxChPolicerVlanCntrSet
*         - cpssDxChPolicerPolicyCntrSet
*         The policer counting should be disabled before flush of WBC if direct
*         write access is performed under traffic. And policer counting should be
*         enabled again after finish of write access.
*         The cpssDxChPolicerCountingModeSet may be used to disable/enable
*         the policer counting.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To use the listed above APIs (cpssDxCh3PolicerBillingEntrySet,
*       cpssDxChPolicerVlanCntrSet and cpssDxChPolicerPolicyCntrSet) under
*       traffic, perform the following algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
GT_STATUS cpssDxChPolicerCountingWriteBackCacheFlush
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage
);

/**
* @internal cpssDxChPolicerMeteringCalcMethodSet function
* @endinternal
*
* @brief   Sets Metering Policer parameters calculation orientation method.
*         Due to the fact that selecting algorithm has effect on how HW metering
*         parameters are set, this API influence on
*         cpssDxCh3PolicerMeteringEntrySet and
*         cpssDxCh3PolicerEntryMeterParamsCalculate APIs.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] meteringCalcMethod       - selecting calculation of HW metering
*                                      parameters between CIR\PIR or CBS\PBS
*                                      orientation.
* @param[in] cirPirAllowedDeviation   - the allowed deviation in percentage from the
*                                      requested CIR\PIR. Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
* @param[in] cbsPbsCalcOnFail         - GT_TRUE: If CBS\PBS constraints cannot be
*                                      matched return to CIR\PIR oriented
*                                      calculation.
*                                      GT_FALSE: If CBS\PBS constraints cannot be
*                                      matched return error. Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or method.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E - The default
*       algorithm in which metering parameters are configured in such a way
*       that the selected CIR\PIR is the one which is the nearest possible
*       (under the granularity constraint) to the requested CIR\PIR.
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E - In this
*       algorithm a deviation from requested CIR\PIR is defined. At first stage
*       the algorithm tries to find parameters that will complay with both the
*       CBS\PBS requested and the range allowed for the CIR\PIR. If no matching
*       solution is achieved there is an option to perform another stage. At
*       the second (optional) stage CIR\PIR is selected in the allowed range
*       and under this selection CBS\PBS is configured to be as near as
*       possible to the requested one. If second stage is needed and not
*       selected failure (error) indication is reported.
*
*/
GT_STATUS cpssDxChPolicerMeteringCalcMethodSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT   meteringCalcMethod,
    IN GT_U32                                       cirPirAllowedDeviation,
    IN GT_BOOL                                      cbsPbsCalcOnFail
);

/**
* @internal cpssDxChPolicerMeteringCalcMethodGet function
* @endinternal
*
* @brief   Gets Metering Policer parameters calculation orientation method.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
*
* @param[out] meteringCalcMethodPtr    - (pointer to) selected calculation of HW
*                                      metering parameters between CIR\PIR or
*                                      CBS\PBS orientation.
* @param[out] cirPirAllowedDeviationPtr - (pointer to) the allowed deviation in
*                                      percentage from the requested CIR\PIR.
*                                      Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
* @param[out] cbsPbsCalcOnFailPtr      - (pointer to)
*                                      GT_TRUE: If CBS\PBS constraints cannot be
*                                      matched return to CIR\PIR oriented
*                                      calculation.
*                                      GT_FALSE: If CBS\PBS constraints cannot
*                                      be matched return error.
*                                      Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - wrong devNum or method.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E - The default
*       algorithm in which metering parameters are configured in such a way
*       that the selected CIR\PIR is the one which is the nearest possible
*       (under the granularity constraint) to the requested CIR\PIR.
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E - In this
*       algorithm a deviation from requested CIR\PIR is defined. At first stage
*       the algorithm tries to find parameters that will complay with both the
*       CBS\PBS requested and the range allowed for the CIR\PIR. If no matching
*       solution is achieved there is an option to perform another stage. At
*       the second (optional) stage CIR\PIR is selected in the allowed range
*       and under this selection CBS\PBS is configured to be as near as
*       possible to the requested one. If second stage is needed and not
*       selected failure (error) indication is reported.
*
*/
GT_STATUS cpssDxChPolicerMeteringCalcMethodGet
(
    IN GT_U8                                        devNum,
    OUT CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT  *meteringCalcMethodPtr,
    OUT GT_U32                                      *cirPirAllowedDeviationPtr,
    OUT GT_BOOL                                     *cbsPbsCalcOnFailPtr
);

/**
* @internal cpssDxChPolicerCountingTriggerByPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Counting when port based metering is triggered for
*         the packet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - GT_TRUE - If counting mode is Billing, counting is performed
*                                      for every packet subject to port based metering.
*                                      If counting mode is not Billing, counting is
*                                      performed for these packets only if the Policy
*                                      engine enabled for Counting.
*                                      GT_FALSE - Counting is performed for these packets only
*                                      if the Policy engine enabled for Counting.
*                                      .
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerCountingTriggerByPortEnableSet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN GT_BOOL                                  enable
);

/**
* @internal cpssDxChPolicerCountingTriggerByPortEnableGet function
* @endinternal
*
* @brief   Get Counting trigger by port status.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
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
GT_STATUS cpssDxChPolicerCountingTriggerByPortEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    OUT GT_BOOL                                 *enablePtr
);


/**
* @internal cpssDxChPolicerPortModeAddressSelectSet function
* @endinternal
*
* @brief   Configure Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] type                     - Address select type: Full or Compressed.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or type.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortModeAddressSelectSet
(
    IN  GT_U8                                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT                     stage,
    IN CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT     type
);


/**
* @internal cpssDxChPolicerPortModeAddressSelectGet function
* @endinternal
*
* @brief   Get Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
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
GT_STATUS cpssDxChPolicerPortModeAddressSelectGet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT                    stage,
    OUT CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT    *typePtr
);


/**
* @internal cpssDxChPolicerTriggerEntrySet function
* @endinternal
*
* @brief   Set Policer trigger entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..8191).
* @param[in] entryPtr                 - (pointer to) Policer port trigger entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerTriggerEntrySet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
);

/**
* @internal cpssDxChPolicerTriggerEntryGet function
* @endinternal
*
* @brief   Get Policer trigger entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..8191).
*
* @param[out] entryPtr                 - (pointer to) Policer port trigger entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerTriggerEntryGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
);

/******************************************************************************/
/******************************************************************************/
/******* start of functions with portGroupsBmp parameter **********************/
/******************************************************************************/
/******************************************************************************/


/**
* @internal cpssDxChPolicerPortGroupMeteringEntryRefresh function
* @endinternal
*
* @brief   Refresh the Policer Metering Entry.
*         Meter Bucket's state parameters refreshing are needed in order to
*         prevent mis-behavior due to wrap around of timers.
*         The wrap around problem can occur when there are long periods of
*         'silence' on the flow and the bucket's state parameters do not get
*         refreshed (meter is in the idle state). This causes a problem once the
*         flow traffic is re-started since the timers might have already wrapped
*         around which can cause a situation in which the bucket's state is
*         mis-interpreted and the incoming packet is marked as non-conforming
*         even though the bucket was actually supposed to be full.
*         To prevent this from happening the CPU needs to trigger a meter
*         refresh transaction at least once every 10 minutes per meter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, entryIndex or
*                                       portGroupsBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Once the CPU triggers Refresh transaction the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
GT_STATUS cpssDxChPolicerPortGroupMeteringEntryRefresh
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex
);

/**
* @internal cpssDxChPolicerPortGroupErrorGet function
* @endinternal
*
* @brief   Gets address and type of Policer Entry that had an ECC error.
*         This information available if error is happened and not read till now.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] entryTypePtr             - pointer to the Type of Entry (Metering or Counting)
*                                      that had an error.
* @param[out] entryAddrPtr             - pointer to the Policer Entry that had an error.
*
* @retval GT_OK                    - on success.
* @retval GT_EMPTY                 - on missing error information.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortGroupErrorGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT   *entryTypePtr,
    OUT GT_U32                              *entryAddrPtr
);

/**
* @internal cpssDxChPolicerPortGroupErrorCounterGet function
* @endinternal
*
* @brief   Gets the value of the Policer ECC Error Counter.
*         The Error Counter is a free-running non-sticky 8-bit read-only
*         counter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] cntrValuePtr             - pointer to the Policer ECC Error counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortGroupErrorCounterGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *cntrValuePtr
);

/**
* @internal cpssDxChPolicerPortGroupManagementCountersSet function
* @endinternal
*
* @brief   Sets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
* @param[in] mngCntrPtr               - pointer to the Management Counters Entry must be set.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage or mngCntrType,
*                                       Management Counters Set number or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*
*/
GT_STATUS cpssDxChPolicerPortGroupManagementCountersSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT      mngCntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT     mngCntrType,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC    *mngCntrPtr
);

/**
* @internal cpssDxChPolicerPortGroupManagementCountersGet function
* @endinternal
*
* @brief   Gets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
*
* @param[out] mngCntrPtr               - pointer to the requested Management Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, mngCntrType,
*                                       Management Counters Set number or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*
*/
GT_STATUS cpssDxChPolicerPortGroupManagementCountersGet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     mngCntrSet,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT    mngCntrType,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC   *mngCntrPtr
);

/**
* @internal cpssDxChPolicerPortGroupMeteringEntrySet function
* @endinternal
*
* @brief   Sets Metering Policer Entry.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] entryPtr                 - pointer to the metering policer entry going to be set.
*
* @param[out] tbParamsPtr              - pointer to actual policer token bucket parameters.
*                                      The token bucket parameters are returned as output
*                                      values. This is due to the hardware rate resolution,
*                                      the exact rate or burst size requested may not be
*                                      honored. The returned value gives the user the
*                                      actual parameters configured in the hardware.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage, entryIndex,
*                                       entry parameters or portGroupsBmp.
* @retval GT_OUT_OF_RANGE          - on Billing Entry Index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortGroupMeteringEntrySet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbParamsPtr
);


/**
* @internal cpssDxChPolicerPortGroupMeteringEntryGet function
* @endinternal
*
* @brief   Gets Metering Policer Entry configuration.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @param[out] entryPtr                 - pointer to the requested metering policer entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, entryIndex or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
GT_STATUS cpssDxChPolicerPortGroupMeteringEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr
);

/**
* @internal cpssDxChPolicerPortGroupBillingEntrySet function
* @endinternal
*
* @brief   Sets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
* @param[in] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out upon counter reset by
*                                       indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, entryIndex or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*       In order to set Billing entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxChIpfixEntrySet".
*
*/
GT_STATUS cpssDxChPolicerPortGroupBillingEntrySet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
);

/**
* @internal cpssDxChPolicerPortGroupBillingEntryGet function
* @endinternal
*
* @brief   Gets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
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
* @retval GT_BAD_PARAM             - on wrong devNum, stage, entryIndex or
*                                       portGroupsBmp.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*/
GT_STATUS cpssDxChPolicerPortGroupBillingEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  GT_BOOL                                 reset,
    OUT CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
);

/**
* @internal cpssDxChPolicerPortGroupPolicyCntrSet function
* @endinternal
*
* @brief   Sets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
* @param[in] cntrValue                - packets counter.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index (index range is limited by
*                                       max number of Policer Policy counters) or portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*       In order to set Policy counting entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
GT_STATUS cpssDxChPolicerPortGroupPolicyCntrSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    IN  GT_U32                              cntrValue
);

/**
* @internal cpssDxChPolicerPortGroupPolicyCntrGet function
* @endinternal
*
* @brief   Gets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
*
* @param[out] cntrValuePtr             - Pointer to the packet counter.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, index or portGroupsBmp.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*
*/
GT_STATUS cpssDxChPolicerPortGroupPolicyCntrGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    OUT GT_U32                              *cntrValuePtr
);

/**
* @internal cpssDxChPolicerTrappedPacketsBillingEnableSet function
* @endinternal
*
* @brief   Enables the billing algorithm for Trapped packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - enable/disable billing for Trapped packets.
*                                      GT_TRUE  - enable billing for Trapped packets.
*                                      GT_FALSE - disable billing for Trapped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerTrappedPacketsBillingEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_BOOL                              enable
);

/**
* @internal cpssDxChPolicerTrappedPacketsBillingEnableGet function
* @endinternal
*
* @brief   Get state of the billing algorithm for Trapped packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - (pointer to) enable/disable billing for Trapped packets.
*                                      GT_TRUE  - enable billing for Trapped packets.
*                                      GT_FALSE - disable billing for Trapped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerTrappedPacketsBillingEnableGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChPolicerLossMeasurementCounterCaptureEnableSet function
* @endinternal
*
* @brief   Enables or disables Loss Measurement capture.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - enable/disable Loss Measurement capture.
*                                      GT_TRUE  - enable capture of Loss Measurement packets.
*                                      GT_FALSE - disable capture of Loss Measurement packets.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChPolicerLossMeasurementCounterCaptureEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_BOOL                              enable
);

/**
* @internal cpssDxChPolicerLossMeasurementCounterCaptureEnableGet function
* @endinternal
*
* @brief   Get Loss Measurement capture state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - (pointer to)enable/disable Loss Measurement capture.
*                                      GT_TRUE  - enable capture of Loss Measurement packets.
*                                      GT_FALSE - disable capture of Loss Measurement packets.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChPolicerLossMeasurementCounterCaptureEnableGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChPolicerEAttributesMeteringModeSet function
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
GT_STATUS cpssDxChPolicerEAttributesMeteringModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT  mode
);

/**
* @internal cpssDxChPolicerEAttributesMeteringModeGet function
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
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
GT_STATUS cpssDxChPolicerEAttributesMeteringModeGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT *modePtr
);


/**
* @internal cpssDxChPolicerFlowIdCountingCfgSet function
* @endinternal
*
* @brief   Set Flow Id based counting configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
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
GT_STATUS cpssDxChPolicerFlowIdCountingCfgSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC  *billingIndexCfgPtr
);

/**
* @internal cpssDxChPolicerFlowIdCountingCfgGet function
* @endinternal
*
* @brief   Get Flow Id based counting configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] billingIndexCfgPtr       - (pointer to) billing index configuration structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChPolicerFlowIdCountingCfgGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC  *billingIndexCfgPtr
);

/**
* @internal cpssDxChPolicerHierarchicalTableEntrySet function
* @endinternal
*
* @brief   Set Hierarchical Table Entry configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
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
GT_STATUS cpssDxChPolicerHierarchicalTableEntrySet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          index,
    IN  CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC  *entryPtr
);

/**
* @internal cpssDxChPolicerHierarchicalTableEntryGet function
* @endinternal
*
* @brief   Get Hierarchical Table Entry configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Hierarchical Table Entry index.
*
* @param[out] entryPtr                 - (pointer to) Hierarchical Table Entry structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or index
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Applicable only to Ingress #0 Policer Stage.
*
*/
GT_STATUS cpssDxChPolicerHierarchicalTableEntryGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          index,
    OUT CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC  *entryPtr
);

/**
* @internal cpssDxChPolicerSecondStageIndexMaskSet function
* @endinternal
*
* @brief   Sets second stage policer index mask.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] plrIndexMask             - mask for policer index
*                                      (APPLICABLE RANGES: Lion 0..0xFFF, Lion2 0..0xFFFF).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on illegal plrIndexMask
*
* @note Note: Lion device applicable starting from revision B2
*       Lion2 device applicable starting from revision B1
*
*/
GT_STATUS cpssDxChPolicerSecondStageIndexMaskSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  plrIndexMask
);

/**
* @internal cpssDxChPolicerSecondStageIndexMaskGet function
* @endinternal
*
* @brief   Gets second stage policer index mask.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] plrIndexMaskPtr          - (pointer to) mask for policer index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Note: Lion device applicable starting from revision B2
*       Lion2 device applicable starting from revision B1
*
*/
GT_STATUS cpssDxChPolicerSecondStageIndexMaskGet
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *plrIndexMaskPtr
);

/**
* @internal cpssDxChPolicerPortStormTypeInFlowModeEnableSet function
* @endinternal
*
* @brief   Enable port-based metering triggered in Flow mode to work in
*         (storm-pkt-type, port) based, otherwise (port) based is used.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - Enable\Disable Port metering according to storm type
*                                      when working in flow mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerPortStormTypeInFlowModeEnableSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssDxChPolicerPortStormTypeInFlowModeEnableGet function
* @endinternal
*
* @brief   Get if port-based metering triggered in Flow mode is enabled for
*         (storm-pkt-type, port) based (otherwise (port) based is used).
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - (pointer to) Enable status of Port metering according to storm type
*                                      when working in flow mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerPortStormTypeInFlowModeEnableGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChPolicerPacketSizeModeForTimestampTagSet function
* @endinternal
*
* @brief   Determines whether the timestamp tag should be included in the byte count
*         computation or not.
*         The configuration is used for IPFIX, VLAN and Management counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] timestampTagMode         - packet size counting mode for timestamp tag.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or timestampTagMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPacketSizeModeForTimestampTagSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT                stage,
    IN  CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT     timestampTagMode
);

/**
* @internal cpssDxChPolicerPacketSizeModeForTimestampTagGet function
* @endinternal
*
* @brief   Retrieves whether the timestamp tag should be included in the byte count
*         computation or not.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] timestampTagModePtr      - (pointer to) packet size counting mode for timestamp tag.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPacketSizeModeForTimestampTagGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT                stage,
    OUT CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT    *timestampTagModePtr
);

/**
* @internal cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_BOOL                       enable
);

/**
* @internal cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_BOOL                       enable
);

/**
* @internal cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal cpssDxChPolicerMeteringEntryEnvelopeSet function
* @endinternal
*
* @brief   Sets Envelope of Metering Policer Entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startEntryIndex          - index of Start Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] envelopeSize             - amount of entries in the envelope
* @param[in] couplingFlag0            - in MEF 10.3, the CIR bucket of the bottom rank in the
*                                      MEF 10.3 envelope may either spill the overflow to
*                                      the EIR bucket of the highest rank or not, according
*                                      to coupling flag 0.
*                                      GT_FALSE = DONT SPILL: The extra tokens of the CIR bucket
*                                      are not spilled to the EIR bucket.
*                                      GT_TRUE = SPILL: The extra tokens of the CIR bucket
*                                      are spilled to the EIR bucket.
* @param[in] entryArr[]               - array of the metering policer entries going to be set.
*
* @param[out] tbParamsArr[]            - array of actual policer token bucket parameters.
*                                      The token bucket parameters are returned as output
*                                      values. This is due to the hardware rate resolution,
*                                      the exact rate or burst size requested may not be
*                                      honored. The returned value gives the user the
*                                      actual parameters configured in the hardware.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex or
*                                       entry parameters.
* @retval GT_OUT_OF_RANGE          - on some parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerMeteringEntryEnvelopeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN  GT_U32                                      startEntryIndex,
    IN  GT_U32                                      envelopeSize,
    IN  GT_BOOL                                     couplingFlag0,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC       entryArr[],
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT      tbParamsArr[]
);

/**
* @internal cpssDxChPolicerMeteringEntryEnvelopeGet function
* @endinternal
*
* @brief   Gets Envelope of Metering Policer Entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startEntryIndex          - index of Start Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] maxEnvelopeSize          - amount of entries in output arrays
*
* @param[out] envelopeSizePtr          - (pointer to) amount of entries in the envelope
* @param[out] couplingFlag0Ptr         - (pointer to)   in MEF 10.3, the CIR bucket of the bottom
*                                      rank in the MEF 10.3 envelope may either spill the overflow
*                                      to the EIR bucket of the highest rank or not, according
*                                      to coupling flag 0.
*                                      GT_FALSE = DONT SPILL: The extra tokens of the CIR bucket
*                                      are not spilled to the EIR bucket.
*                                      GT_TRUE = SPILL: The extra tokens of the CIR bucket
*                                      are spilled to the EIR bucket.
* @param[out] entryArr[]               - array of the metering policer entries going to be get.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex or
*                                       entry parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerMeteringEntryEnvelopeGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      startEntryIndex,
    IN   GT_U32                                      maxEnvelopeSize,
    OUT  GT_U32                                      *envelopeSizePtr,
    OUT  GT_BOOL                                     *couplingFlag0Ptr,
    OUT  CPSS_DXCH3_POLICER_METERING_ENTRY_STC       entryArr[]
);

/**
* @internal cpssDxChPolicerMeterTableFlowBasedIndexConfigSet function
* @endinternal
*
* @brief   Set Policy Pointer threshold and maximal envelope size for using the
*         Flow based algorithm. For Policy Pointer values greater or equal
*         than threshold the Priority based algorithm used.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] threshold                - upper  of Policy Pointer for Flow Based algorithm.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
* @param[in] maxSize                  - maximal envelope size and alignment.
*                                      the index of start entry of each envelope
*                                      accessed by Flow Based Algorithm should be
*                                      aligned to maxSize.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerMeterTableFlowBasedIndexConfigSet
(
    IN  GT_U8                                          devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT               stage,
    IN  GT_U32                                         threshold,
    IN  CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT        maxSize
);

/**
* @internal cpssDxChPolicerMeterTableFlowBasedIndexConfigGet function
* @endinternal
*
* @brief   Get Policy Pointer threshold and maximal envelope size for using the
*         Flow based algorithm. For Policy Pointer values greater or equal
*         than threshold the Priority based algorithm used.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] thresholdPtr             - (poiner to) upper threshold
*                                      of Policy Pointer for Flow Based algorithm.
* @param[out] maxSizePtr               - (poiner to) maximal envelope size and alignment.
*                                      the index of start entry of each envelope
*                                      accessed by Flow Based Algorithm should be
*                                      aligned to maxSize.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_STATE             - on wrong HW data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerMeterTableFlowBasedIndexConfigGet
(
    IN   GT_U8                                            devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT                 stage,
    OUT  GT_U32                                           *thresholdPtr,
    OUT  CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT          *maxSizePtr
);

/**
* @internal cpssDxChPolicerTokenBucketMaxRateSet function
* @endinternal
*
* @brief   Set the value of the Maximal Rate in the Maximal Rate table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - Bucket Max Rate table index.
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] bucketIndex              - Bucket index.
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] meterEntryRate           - CIR or EIR of meter entry that use this
*                                      maximal rate in Kbps (1Kbps = 1000 bits per second)
* @param[in] meterEntryRate           - CBS or EBS of meter entry that use this maximal rate in bytes.
* @param[in] maxRate                  - maximal Information Rate in Kbps (1Kbps = 1000 bits per second)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See MEF10.3 examples in FS to know relation between bucket0/1 and Commited/Exess
*
*/
GT_STATUS cpssDxChPolicerTokenBucketMaxRateSet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      entryIndex,
    IN   GT_U32                                      bucketIndex,
    IN   GT_U32                                      meterEntryRate,
    IN   GT_U32                                      meterEntryBurstSize,
    IN   GT_U32                                      maxRate
);

/**
* @internal cpssDxChPolicerTokenBucketMaxRateGet function
* @endinternal
*
* @brief   Get the value of the Maximal Rate in the Maximal Rate table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - Bucket Max Rate table index.
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] bucketIndex              - Bucket index.
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] meterEntryRate           - CIR or EIR of meter entry that use this
*                                      maximal rate in Kbps (1Kbps = 1000 bits per second)
* @param[in] meterEntryRate           - CBS or EBS of meter entry that use this maximal rate in bytes.
*
* @param[out] maxRatePtr               - (pointer to) maximal Information Rate in Kbps (1Kbps = 1000 bits per second)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerTokenBucketMaxRateGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      entryIndex,
    IN   GT_U32                                      bucketIndex,
    IN   GT_U32                                      meterEntryRate,
    IN   GT_U32                                      meterEntryBurstSize,
    OUT  GT_U32                                      *maxRatePtr
);

/**
* @internal cpssDxChPolicerQosProfileToPriorityMapSet function
* @endinternal
*
* @brief   Set entry of Qos Profile Mapping to Priority table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] qosProfileIndex          - Qos Profile index.
*                                      (APPLICABLE RANGES: 0..1023)
* @param[in] priority                 - priority.
*                                      (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerQosProfileToPriorityMapSet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      qosProfileIndex,
    IN   GT_U32                                      priority
);

/**
* @internal cpssDxChPolicerQosProfileToPriorityMapGet function
* @endinternal
*
* @brief   Get entry of Qos Profile Mapping to Priority table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] qosProfileIndex          - Qos Profile index.
*                                      (APPLICABLE RANGES: 0..1023)
*
* @param[out] priorityPtr              - (pointer to) priority.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerQosProfileToPriorityMapGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      qosProfileIndex,
    OUT  GT_U32                                      *priorityPtr
);

/**
* @internal cpssDxChPolicerPortGroupBucketsCurrentStateGet function
* @endinternal
*
* @brief   Get current state of metering entry buckets.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
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
*                                      Data retieved from the first poert group of bitmap.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] policerIndex             - index of metering entry
*
* @param[out] bucketsStatePtr          - (pointer to) Buckets State structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerPortGroupBucketsCurrentStateGet
(
    IN   GT_U8                                       devNum,
    IN   GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      policerIndex,
    OUT  CPSS_DXCH_POLICER_BUCKETS_STATE_STC        *bucketsStatePtr
);

/**
* @internal cpssDxChPolicerBucketsCurrentStateGet function
* @endinternal
*
* @brief   Get current state of metering entry buckets.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] policerIndex             - index of metering entry
*
* @param[out] bucketsStatePtr          - (pointer to) Buckets State structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerBucketsCurrentStateGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      policerIndex,
    OUT  CPSS_DXCH_POLICER_BUCKETS_STATE_STC        *bucketsStatePtr
);

/**
* @internal cpssDxChPolicerMeteringAnalyzerIndexSet function
* @endinternal
*
* @brief  Set analyzer index per color of the packet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] color                    - possible values are: green, yellow or red.
* @param[in] enable                   - enable/disable mirroring.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index or color.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPolicerMeteringAnalyzerIndexSet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN CPSS_DP_LEVEL_ENT                   color,
    IN GT_BOOL                             enable,
    IN GT_U32                              index
);

/**
* @internal cpssDxChPolicerMeteringAnalyzerIndexGet function
* @endinternal
*
* @brief  Get analyzer index per color of the packet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] color                    - possible values are: green, yellow or red.
* @param[out] enablePtr               - (pointer to) enable/disable mirroring.
* @param[out] indexPtr                - (pointer to) analyzer interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or color.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPolicerMeteringAnalyzerIndexGet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN CPSS_DP_LEVEL_ENT                   color,
    OUT GT_BOOL                            *enablePtr,      
    OUT GT_U32                             *indexPtr  
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxCh3Policerh */

