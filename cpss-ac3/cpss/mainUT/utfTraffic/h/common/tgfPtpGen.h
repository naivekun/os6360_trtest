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
* @file tgfPtpGen.h
*
* @brief Generic API for Presision Time Protocol (PTP) API.
*
* @version   3
********************************************************************************
*/
#ifndef __tgfPtpGenh
#define __tgfPtpGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_TGF_PTP_TAI_INSTANCE_ENT
 *
 * @brief Instance of TAI type (contains 2 TAI Units).
*/
typedef enum{

    /** Global Instance Only */
    PRV_TGF_PTP_TAI_INSTANCE_GLOBAL_E,

    /** @brief Instance identified by port
     *  There is one instance per several ports typically for 4 ports.
     */
    PRV_TGF_PTP_TAI_INSTANCE_PORT_E,

    /** All Instances */
    PRV_TGF_PTP_TAI_INSTANCE_ALL_E

} PRV_TGF_PTP_TAI_INSTANCE_ENT;

/**
* @enum PRV_TGF_PTP_TAI_NUMBER_ENT
 *
 * @brief Number of TAI in Instance.
*/
typedef enum{

    /** TAI0 */
    PRV_TGF_PTP_TAI_NUMBER_0_E,

    /** TAI1 */
    PRV_TGF_PTP_TAI_NUMBER_1_E,

    /** All TAIs */
    PRV_TGF_PTP_TAI_NUMBER_ALL_E

} PRV_TGF_PTP_TAI_NUMBER_ENT;

/**
* @struct PRV_TGF_PTP_TAI_ID_STC
 *
 * @brief Structure for TAI Identification.
*/
typedef struct{

    /** TAI Instance */
    PRV_TGF_PTP_TAI_INSTANCE_ENT taiInstance;

    /** TAI Number */
    PRV_TGF_PTP_TAI_NUMBER_ENT taiNumber;

    /** Physical Port Number */
    GT_PHYSICAL_PORT_NUM portNum;

} PRV_TGF_PTP_TAI_ID_STC;

/**
* @enum PRV_TGF_PTP_TOD_COUNTER_FUNC_ENT
 *
 * @brief This enum defines possible TOD (Time Of Day) counter functions.
*/
typedef enum{

    /** @brief Copy the value from the TOD
     *  counter shadow to the TOD counter register
     */
    PRV_TGF_PTP_TOD_COUNTER_FUNC_UPDATE_E,

    /** @brief Add the value of the TOD
     *  counter shadow to the TOD counter register
     */
    PRV_TGF_PTP_TOD_COUNTER_FUNC_INCREMENT_E,

    /** @brief Copy the value of the TOD
     *  counter to the TOD counter shadow register
     */
    PRV_TGF_PTP_TOD_COUNTER_FUNC_CAPTURE_E,

    /** @brief Generate a pulse on the
     *  external interface at the configured time,
     *  determined by the TOD counter shadow
     */
    PRV_TGF_PTP_TOD_COUNTER_FUNC_GENERATE_E,

    /** @brief update the value of the TOD Counter
     *  by subtracting a preconfigured offset.
     */
    PRV_TGF_PTP_TOD_COUNTER_FUNC_DECREMENT_E,

    /** @brief as INCREMENT, but the TOD Counter
     *  modified by a preconfigured offset over a given period of time.
     */
    PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E,

    /** @brief as DECREMENT, but the TOD Counter
     *  modified by a preconfigured offset over a given period of time.
     */
    PRV_TGF_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E,

    /** @brief Copy the value from the TOD
     *  counter shadow to the fractional nanosecond drift register
     */
    PRV_TGF_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E,

    /** No Operation. */
    PRV_TGF_PTP_TOD_COUNTER_FUNC_NOP_E

} PRV_TGF_PTP_TOD_COUNTER_FUNC_ENT;

/**
* @enum PRV_TGF_PTP_TAI_CLOCK_MODE_ENT
 *
 * @brief This enum defines the clock mode.
*/
typedef enum{

    /** Clock connection disabled */
    PRV_TGF_PTP_TAI_CLOCK_MODE_DISABLED_E,

    /** @brief Generate external clock
     *  relevant to Global TAI Instance only.
     */
    PRV_TGF_PTP_TAI_CLOCK_MODE_OUTPUT_E,

    /** Receive clock and update TOD */
    PRV_TGF_PTP_TAI_CLOCK_MODE_INPUT_E,

    /** @brief Receive clock and update TOD
     *  and adjust frequency
     */
    PRV_TGF_PTP_TAI_CLOCK_MODE_INPUT_ADJUST_E

} PRV_TGF_PTP_TAI_CLOCK_MODE_ENT;

/**
* @enum PRV_TGF_PTP_TAI_TOD_TYPE_ENT
 *
 * @brief This enum defines type of value of TOD (Time Of Day) counter.
*/
typedef enum{

    /** @brief The value of the TOD
     *  when the trigger generated for external device.
     */
    PRV_TGF_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,

    /** Generate function Mask. */
    PRV_TGF_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,

    /** @brief The value to update the TOD
     *  by loading it instead the current TOD
     *  or adding it to the current TOD
     *  or subtracting it from the current TOD.
     */
    PRV_TGF_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,

    /** @brief Copy of the value of the TOD
     *  made by Capture command.
     */
    PRV_TGF_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E,

    /** @brief Copy of the value of the TOD
     *  made by Capture command.
     */
    PRV_TGF_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E

} PRV_TGF_PTP_TAI_TOD_TYPE_ENT;

/**
* @struct PRV_TGF_PTP_TOD_COUNT_STC
 *
 * @brief Structure for TOD Counter.
*/
typedef struct{

    /** nanosecond value of the time stamping TOD counter */
    GT_U32 nanoSeconds;

    /** @brief second value of the time stamping TOD counter
     *  48 LSBits are significant.
     */
    GT_U64 seconds;

    /** @brief fractional nanosecond part of the value.
     *  30 MSBits are significant.
     */
    GT_U32 fracNanoSeconds;

} PRV_TGF_PTP_TOD_COUNT_STC;

/**
* @struct PRV_TGF_PTP_TAI_TOD_STEP_STC
 *
 * @brief Structure for value of TOD Step.
*/
typedef struct{

    /** @brief whole nanosecond part of the value
     *  all 32 bits are significant.
     */
    GT_U32 nanoSeconds;

    /** @brief fractional nanosecond part of the value.
     *  Value measured in (1/(2^32)) units.
     */
    GT_U32 fracNanoSeconds;

} PRV_TGF_PTP_TAI_TOD_STEP_STC;

/**
* @struct PRV_TGF_PTP_TSU_CONTROL_STC
 *
 * @brief Structure for TSU (Time Stamping Unit) Control.
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - the unit disabled.
     */
    GT_BOOL unitEnable;

    /** @brief select TAI for ingress time stamping.
     *  (APPLICABLE RANGES: 0..1.)
     */
    GT_U32 rxTaiSelect;

    /** @brief GT_TRUE
     *  the oldest entry in the queue is overwritten,
     *  and an interrupt is sent to the CPU.
     *  GT_FALSE - if a timestamp needs to be written to a full queue,
     *  the timestamp is not written.
     */
    GT_BOOL tsQueOverrideEnable;

} PRV_TGF_PTP_TSU_CONTROL_STC;

/**
* @struct PRV_TGF_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC
 *
 * @brief Structure for TSU (Time Stamping Unit) TX Timestamp Queue Entry.
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - Queue is empty.
     */
    GT_BOOL entryValid;

    /** @brief This field is used to match between the egress timestamp
     *  queue entry and the per-port timestamp entry.
     *  The same <Queue Entry ID> is also stored in the
     *  per-port timestamp queue.
     */
    GT_U32 entryId;

    /** selected TAI. (APPLICABLE RANGES: 0..1.) */
    GT_U32 taiSelect;

    /** @brief The least significant bit of the <Time Update Counter>
     *  in the TAI, at the time that <Timestamp> is captured.
     */
    GT_U32 todUpdateFlag;

    /** timestamp */
    GT_U32 timestamp;

} PRV_TGF_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC;

/**
* @enum PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_ENT
 *
 * @brief This enum defines the packet counter type.
*/
typedef enum{

    /** @brief Counts the total number
     *  of packets that are subject to any kind of time stamping action in
     *  the egress time stamping unit (TSU).
     */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E,

    /** Counts PTP V1 packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E,

    /** Counts PTP V2 packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E,

    /** Counts Y1731 packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_Y1731_E,

    /** Counts NTP Timestamp packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TS_E,

    /** Counts NTP received packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_NTP_RX_E,

    /** Counts NTP transmitted packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TX_E,

    /** Counts WAMP packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_WAMP_E,

    /** Counts None Action packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_NONE_ACTION_E,

    /** Counts Forwarded packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_FORWARD_E,

    /** Counts Dropped packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_DROP_E,

    /** Counts Captured packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_E,

    /** Counts Add Time Action packets */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_ADD_TIME_E,

    /** @brief Counts Add
     *  Corrected Time Action packets
     */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_ADD_CORR_TIME_E,

    /** @brief Counts Captured
     *  Add Time Action packets
     */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_TIME_E,

    /** @brief Counts Captured
     *  Add Corrected Time Action packets
     */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_CORR_TIME_E,

    /** @brief Counts
     *  Add Ingress Time Action packets
     */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_ADD_INGRESS_TIME_E,

    /** @brief Counts
     *  Captured Add Ingress Time Action packets
     */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_INGRESS_TIME_E,

    /** @brief Counts
     *  Captured Ingress Time Action packets
     */
    PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_INGRESS_TIME_E

} PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_ENT;

/**
* @struct PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC
 *
 * @brief Structure Timestamp Tag Global Configuration.
*/
typedef struct{

    /** @brief GT_TRUE
     *  as timestamp tagged with Extended or Non-Extended Tag.
     *  GT_FALSE - no packets are identified
     *  as timestamp tagged with Extended or Non-Extended Tag.
     */
    GT_BOOL tsTagParseEnable;

    /** @brief GT_TRUE
     *  are identified as timestamp tagged with Hybrid Tag.
     *  GT_FALSE - no packets
     *  are identified as timestamp tagged with Hybrid Tag.
     */
    GT_BOOL hybridTsTagParseEnable;

    /** @brief Ethertype of Extended and Non Extended TS Tags.
     *  (APPLICABLE RANGES: 0..0xFFFF.)
     */
    GT_U32 tsTagEtherType;

    /** @brief Ethertype of Hybrid TS Tags.
     *  (APPLICABLE RANGES: 0..0xFFFF.)
     */
    GT_U32 hybridTsTagEtherType;

} PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC;

/**
* @enum PRV_TGF_PTP_TS_TAG_MODE_ENT
 *
 * @brief This enum defines the Timestamp Tag mode.
*/
typedef enum{

    /** Timestamps not added to any packets. */
    PRV_TGF_PTP_TS_TAG_MODE_NONE_E,

    /** @brief Timestamps (TS Tags) added
     *  to all packets in nonextended format.
     */
    PRV_TGF_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E,

    /** @brief Timestamps (TS Tags) added
     *  to all packets in extended format.
     */
    PRV_TGF_PTP_TS_TAG_MODE_ALL_EXTENDED_E,

    /** @brief Timestamps added
     *  only to time protocol packets (PTP, DM) as nonextended TS Tags.
     */
    PRV_TGF_PTP_TS_TAG_MODE_NON_EXTENDED_E,

    /** @brief Timestamps added
     *  only to time protocol packets (PTP, DM) as extended TS Tags.
     */
    PRV_TGF_PTP_TS_TAG_MODE_EXTENDED_E,

    /** @brief Timestamps added
     *  only to PTP packets, using the reserved field in the PTP header.
     *  Valid only for PTP header of version v2.
     */
    PRV_TGF_PTP_TS_TAG_MODE_PIGGYBACK_E,

    /** @brief Timestamps added
     *  only to PTP packets, using the hybrid TS tag.
     *  Valid only for PTP header of version v2.
     */
    PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E

} PRV_TGF_PTP_TS_TAG_MODE_ENT;

/**
* @struct PRV_TGF_PTP_TS_TAG_PORT_CFG_STC
 *
 * @brief Structure Timestamp Port Configuration.
*/
typedef struct{

    /** @brief Time for ingress timestamp based on :
     *  GT_TRUE - the timestamp tag.
     *  GT_FALSE - the reception time in the current device.
     */
    GT_BOOL tsReceptionEnable;

    /** @brief ingress PTP V2 packets header reserved field Timestamp use:
     *  GT_TRUE - contains Piggy Back Timestamp.
     *  GT_FALSE - contains data not supported by the unit.
     */
    GT_BOOL tsPiggyBackEnable;

    /** Mode of time stamping the egress packets. */
    PRV_TGF_PTP_TS_TAG_MODE_ENT tsTagMode;

} PRV_TGF_PTP_TS_TAG_PORT_CFG_STC;

/**
* @enum PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_ENT
 *
 * @brief This enum defines the TAI selection mode.
 * Used for selection TAI0 or TAI1 for PTP Header Correction Field.
*/
typedef enum{

    /** @brief use the same TAI as for ingress
     *  Used when the ingress time received by TAI (of current or remote device).
     */
    PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E,

    /** @brief TAI selected from
     *  pertargetport configuration.
     *  Used when the ingress time received by Timestamp TAG from PHY.
     */
    PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E

} PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_ENT;

/**
* @enum PRV_TGF_PTP_TS_TIMESTAMPING_MODE_ENT
 *
 * @brief This enum defines Timestamping Mode.
*/
typedef enum{

    /** Do Action */
    PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E,

    /** Modify Timestamp Tag */
    PRV_TGF_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E

} PRV_TGF_PTP_TS_TIMESTAMPING_MODE_ENT;

/**
* @enum PRV_TGF_PTP_TS_ACTION_ENT
 *
 * @brief This enum defines the PTP Timestamp Action.
*/
typedef enum{

    /** no operation (forward) */
    PRV_TGF_PTP_TS_ACTION_NONE_E,

    /** drop */
    PRV_TGF_PTP_TS_ACTION_DROP_E,

    /** capture */
    PRV_TGF_PTP_TS_ACTION_CAPTURE_E,

    /** add time */
    PRV_TGF_PTP_TS_ACTION_ADD_TIME_E,

    /** add corrected time */
    PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E,

    /** capture and add time */
    PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_TIME_E,

    /** capture and add corrected time */
    PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E,

    /** add ingress time */
    PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_TIME_E,

    /** capture ingress time */
    PRV_TGF_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E,

    /** capture and add ingress time */
    PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E,

    /** @brief Add ingress & egress time
     *  (APPLICABLE DEVICES: Falcon.)
     */
    PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E


} PRV_TGF_PTP_TS_ACTION_ENT;

/**
* @enum PRV_TGF_PTP_TS_PACKET_TYPE_ENT
 *
 * @brief This enum defines the packet type.
*/
typedef enum{

    /** PTP V1 packets */
    PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V1_E,

    /** PTP V2 packets */
    PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E,

    /** Y1731 packets */
    PRV_TGF_PTP_TS_PACKET_TYPE_Y1731_E,

    /** NTP packets timestaming */
    PRV_TGF_PTP_TS_PACKET_TYPE_NTP_TS_E,

    /** NTP received packets */
    PRV_TGF_PTP_TS_PACKET_TYPE_NTP_RX_E,

    /** NTP transmitted packets */
    PRV_TGF_PTP_TS_PACKET_TYPE_NTP_TX_E,

    /** WAMP packets */
    PRV_TGF_PTP_TS_PACKET_TYPE_WAMP_E,

    PRV_TGF_PTP_TS_PACKET_TYPE_RESERVED_E

} PRV_TGF_PTP_TS_PACKET_TYPE_ENT;

/**
* @enum PRV_TGF_PTP_TRANSPORT_TYPE_ENT
 *
 * @brief This enum defines PTP Transport.
*/
typedef enum{

    /** Ethernet */
    PRV_TGF_PTP_TRANSPORT_TYPE_ETHERNET_E,

    /** UDP over IPV4 */
    PRV_TGF_PTP_TRANSPORT_TYPE_UDP_IPV4_E,

    /** UDP over IPV6 */
    PRV_TGF_PTP_TRANSPORT_TYPE_UDP_IPV6_E

} PRV_TGF_PTP_TRANSPORT_TYPE_ENT;

/**
* @struct PRV_TGF_PTP_TS_CFG_ENTRY_STC
 *
 * @brief Structure Timestamp Configuration Table Entry.
*/
typedef struct{

    /** Timestamping mode. */
    PRV_TGF_PTP_TS_TIMESTAMPING_MODE_ENT tsMode;

    /** @brief Offset Profile.
     *  relevant if tsMode==PRV_TGF_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E
     *  field to upate in Extended and Non-Extended TS Tag
     *  (APPLICABLE RANGES: 0..127.)
     */
    GT_U32 offsetProfile;

    /** @brief Offset Enable.
     *  relevant if tsMode==PRV_TGF_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E
     *  field to upate in Extended and Non-Extended TS Tag
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL OE;

    /** @brief Timestamping Action.
     *  relevant if tsMode==PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     */
    PRV_TGF_PTP_TS_ACTION_ENT tsAction;

    /** @brief Packet Format.
     *  relevant if tsMode==PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     *  or packets egressed without Extended and Non-Extended TS Tag
     */
    PRV_TGF_PTP_TS_PACKET_TYPE_ENT packetFormat;

    /** @brief PTP Transport.
     *  relevant if tsMode==PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     *  or packets egressed without Extended and Non-Extended TS Tag
     */
    PRV_TGF_PTP_TRANSPORT_TYPE_ENT ptpTransport;

    /** @brief The offset relative to the beginning of the L3 header.
     *  The offset is measured in bytes.
     *  relevant if tsMode==PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     *  or packets egressed without Extended and Non-Extended TS Tag
     *  (APPLICABLE RANGES: 0..127.)
     */
    GT_U32 offset;

    /** @brief The PTP message type. Affects the <Ingress Link Delay En>,
     *  and the <Message Type> in the timestamp queues
     *  relevant if tsMode==PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     *  and packetFormat is
     *  PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V1_E or
     *  PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E.
     *  (APPLICABLE RANGES: 0..15.)
     */
    GT_U32 ptpMessageType;

    /** @brief PTP message domain index.
     *  relevant if tsMode==PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     *  and packetFormat is
     *  PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V1_E or
     *  PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E.
     *  (APPLICABLE RANGES: 0..4.)
     */
    GT_U32 domain;

    /** @brief Ingress Link Delay Enable.
     *  Ingress link delay is considered in the ingress timestamp computation.
     *  GT_TRUE - enable, GT_FALSE - disable.
     *  This attribute is implicitly enabled for non-PTP packets.
     */
    GT_BOOL ingrLinkDelayEnable;

    /** @brief Packet Dispatching Enable.
     *  Typically enabled for 1-step timestamps.
     *  When enabled, the packet is scheduled to be transmitted at
     *  the precise time that was embedded in the packet.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL packetDispatchingEnable;

} PRV_TGF_PTP_TS_CFG_ENTRY_STC;

/**
* @struct PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC
 *
 * @brief Structure Timestamp Local Action Table Entry.
*/
typedef struct{

    /** @brief Timestamping Action.
     *  Supported values are:
     *  PRV_TGF_PTP_TS_ACTION_NONE_E
     *  PRV_TGF_PTP_TS_ACTION_DROP_E
     *  PRV_TGF_PTP_TS_ACTION_CAPTURE_E
     *  PRV_TGF_PTP_TS_ACTION_ADD_TIME_E
     *  PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E
     *  PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_TIME_E
     *  PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E
     */
    PRV_TGF_PTP_TS_ACTION_ENT tsAction;

    /** @brief Ingress Link Delay Enable.
     *  Ingress link delay is considered in the ingress timestamp computation.
     *  GT_TRUE - enable, GT_FALSE - disable.
     *  This attribute is implicitly enabled for non-PTP packets.
     */
    GT_BOOL ingrLinkDelayEnable;

    /** @brief Packet Dispatching Enable.
     *  Typically enabled for 1-step timestamps.
     *  When enabled, the packet is scheduled to be transmitted at
     *  the precise time that was embedded in the packet.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL packetDispatchingEnable;

} PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC;

/**
* @enum PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT
 *
 * @brief This enum defines UDP checksum update modes.
 * Related to updated timestamps inside UDP payload.
*/
typedef enum{

    /** clear UDP Checksum */
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E,

    /** recalculate UDP Checksum */
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E,

    /** keep original UDP Checksum */
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E

} PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT;

/**
* @struct PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC
 *
 * @brief Structure Timestamp UDP Checksum Update Modes.
*/
typedef struct{

    /** UDP Checksum Update Mode for PTP over IPV4. */
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT ptpIpv4Mode;

    /** UDP Checksum Update Mode for PTP over IPV6. */
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT ptpIpv6Mode;

    /** UDP Checksum Update Mode for NTP over IPV4. */
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT ntpIpv4Mode;

    /** UDP Checksum Update Mode for NTP over IPV6. */
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT ntpIpv6Mode;

    /** UDP Checksum Update Mode for WAMP over IPV4. */
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT wampIpv4Mode;

    /** UDP Checksum Update Mode for WAMP over IPV6. */
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT wampIpv6Mode;

} PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC;

/**
* @enum PRV_TGF_PTP_INGRESS_DOMAIN_MODE_ENT
 *
 * @brief This enum defines PTP domain mode.
*/
typedef enum{

    /** Disable */
    PRV_TGF_PTP_INGRESS_DOMAIN_MODE_DISABLE_E,

    /** PTP version 1 */
    PRV_TGF_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E,

    /** PTP version 2 */
    PRV_TGF_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E

} PRV_TGF_PTP_INGRESS_DOMAIN_MODE_ENT;

/**
* @struct PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC
 *
 * @brief Structure for Egress Domain Table Entry.
*/
typedef struct{

    /** @brief PTP Over Ehernet Timestamping Enable.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL ptpOverEhernetTsEnable;

    /** @brief PTP Over UDP IPV4 Timestamping Enable.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL ptpOverUdpIpv4TsEnable;

    /** @brief PTP Over UDP IPV6 Timestamping Enable.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL ptpOverUdpIpv6TsEnable;

    /** @brief PTP message Timestamping Enable.
     *  Bitmap of 16 bits indexed by PTP Message Type.
     *  1 - enable, 0 - disable.
     *  (APPLICABLE RANGES: 0..0xFFFF.)
     */
    GT_U32 messageTypeTsEnableBmp;

    /** @brief PTP message Timestamping Enable.
     *  Bitmap of 16 bits indexed by 4-bit Transport Specific value.
     *  1 - enable, 0 - disable.
     *  (APPLICABLE RANGES: 0..0xFFFF.)
     */
    GT_U32 transportSpecificTsEnableBmp;

} PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC;

/**
* @struct PRV_TGF_PTP_INGRESS_EXCEPTION_CFG_STC
 *
 * @brief Structure for Ingress Exception Configuration.
*/
typedef struct{

    /** @brief enable PTP command assignment when an exception occurs.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL ptpExceptionCommandEnable;

    /** @brief packet command in case of a PTP exception.
     *  valid values
     *  CPSS_PACKET_CMD_FORWARD_E
     *  CPSS_PACKET_CMD_MIRROR_TO_CPU_E
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     *  CPSS_PACKET_CMD_DROP_HARD_E
     *  CPSS_PACKET_CMD_DROP_SOFT_E
     */
    CPSS_PACKET_CMD_ENT ptpExceptionCommand;

    /** @brief the CPU/Drop Code when the PTP exception packet command
     *  is not FORWARD.
     */
    CPSS_NET_RX_CPU_CODE_ENT ptpExceptionCpuCode;

    /** @brief enable PTP version checking.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL ptpVersionCheckEnable;

} PRV_TGF_PTP_INGRESS_EXCEPTION_CFG_STC;

/**
* @enum PRV_TGF_PTP_INGRESS_CHECKING_MODE_ENT
 *
 * @brief This enum defines ingress PTP packet checking mode.
*/
typedef enum{

    /** @brief The Basic Mode, checks that:
     *  The <versionPTP> field is either 1 or 2.
     *  The packet is long enough to include the version,
     *  message type, and domain fields in the PTP header.
     */
    PRV_TGF_PTP_INGRESS_CHECKING_MODE_BASIC_E,

    /** @brief The Transparent Clock Mode
     *  Additional to the Basic Mode checks it checks that,
     *  the Correction Field
     *  is within the packet length, and within the first 128B.
     */
    PRV_TGF_PTP_INGRESS_CHECKING_MODE_TRANS_CLK_E,

    /** @brief The Piggy Back Mode
     *  Additional to the Basic Mode checks it checks that,
     *  the 32-bit Reserved Field
     *  is within the packet length, and within the first 128B.
     */
    PRV_TGF_PTP_INGRESS_CHECKING_MODE_PIGGY_BACK_E,

    /** @brief The Boundary Clock Mode
     *  Additional to the Basic Mode checks it checks that,
     *  the Origin Timestamp Field
     *  is within the packet length, and within the first 128B.
     */
    PRV_TGF_PTP_INGRESS_CHECKING_MODE_BOUNDRY_CLK_E

} PRV_TGF_PTP_INGRESS_CHECKING_MODE_ENT;

/**
* @struct PRV_TGF_PTP_EGRESS_EXCEPTION_CFG_STC
 *
 * @brief Structure for Egress Exception Configuration.
*/
typedef struct{

    /** @brief invalid PTP Packet Command.
     *  valid values DROP and FORWARD only.
     */
    CPSS_PACKET_CMD_ENT invalidPtpPktCmd;

    /** @brief invalid Outgoing Piggyback Packet Command.
     *  valid values DROP and FORWARD only.
     */
    CPSS_PACKET_CMD_ENT invalidOutPiggybackPktCmd;

    /** @brief invalid Ingoing Piggyback Packet Command.
     *  valid values DROP and FORWARD only.
     */
    CPSS_PACKET_CMD_ENT invalidInPiggybackPktCmd;

    /** @brief invalid Timestamp Packet Command.
     *  valid values DROP and FORWARD only.
     */
    CPSS_PACKET_CMD_ENT invalidTsPktCmd;

} PRV_TGF_PTP_EGRESS_EXCEPTION_CFG_STC;

/**
* @struct PRV_TGF_PTP_EGRESS_EXCEPTION_COUNTERS_STC
 *
 * @brief Structure for Egress Exception Counters.
*/
typedef struct{

    /** invalid PTP Packet Counter. */
    GT_U32 invalidPtpPktCnt;

    /** invalid Outgoing Piggyback Packet Counter. */
    GT_U32 invalidOutPiggybackPktCnt;

    /** invalid Ingoing Piggyback Packet Counter. */
    GT_U32 invalidInPiggybackPktCnt;

    /** invalid Timestamp Packet Counter. */
    GT_U32 invalidTsPktCnt;

} PRV_TGF_PTP_EGRESS_EXCEPTION_COUNTERS_STC;

/**
* @struct PRV_TGF_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC
 *
 * @brief Structure for Ingress Timestamp Queue Entry.
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - Queue is empty.
     */
    GT_BOOL entryValid;

    /** @brief GT_TRUE
     *  GT_FALSE - no PTP exception was triggered
     */
    GT_BOOL isPtpExeption;

    /** Packet Format */
    PRV_TGF_PTP_TS_PACKET_TYPE_ENT packetFormat;

    /** used TAI: 0 for TAI0, 1 for TAI1 */
    GT_U32 taiSelect;

    /** @brief The least significant bit of the Time Update Counter
     *  in the TAI, at the time that Timestamp is captured
     */
    GT_U32 todUpdateFlag;

    /** @brief Message Type from PTP header.
     *  0 for not PTP packets.
     */
    GT_U32 messageType;

    /** @brief Domain Number from PTP header.
     *  0 for not PTP packets.
     */
    GT_U32 domainNum;

    /** @brief The sequenceID field from the PTP packet header.
     *  For non-PTP packets it is packet's Flow-ID.
     */
    GT_U32 sequenceId;

    /** @brief The value of the TOD counter at the times of transmission
     *  or reception of the packet. This is a
     *  32 bit nanosecond timestamp in the range -2^30 to 2^30.
     */
    GT_32 timestamp;

    /** The local */
    GT_PHYSICAL_PORT_NUM portNum;

} PRV_TGF_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC;

/**
* @struct PRV_TGF_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC
 *
 * @brief Structure for Egress Timestamp Queue Entry.
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - Queue is empty.
     */
    GT_BOOL entryValid;

    /** @brief GT_TRUE
     *  GT_FALSE - no PTP exception was triggered
     */
    GT_BOOL isPtpExeption;

    /** Packet Format */
    PRV_TGF_PTP_TS_PACKET_TYPE_ENT packetFormat;

    /** @brief Message Type from PTP header.
     *  0 for not PTP packets.
     */
    GT_U32 messageType;

    /** @brief Domain Number from PTP header.
     *  0 for not PTP packets.
     */
    GT_U32 domainNum;

    /** @brief The sequenceID field from the PTP packet header.
     *  For non-PTP packets it is packet's Flow-ID.
     */
    GT_U32 sequenceId;

    /** @brief This field is used to match between the egress timestamp
     *  queue entry and the per-port timestamp entry.
     *  The same <Queue Entry ID> is also stored
     *  in the per-port timestamp queue.
     *  See prvTgfPtpTsuTxTimestampQueueRead.
     */
    GT_U32 queueEntryId;

    /** The local */
    GT_PHYSICAL_PORT_NUM portNum;

} PRV_TGF_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC;

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfPtpEtherTypeSet function
* @endinternal
*
* @brief   Configure ethertype0/ethertype1 of PTP over Ethernet packets.
*         A packet is identified as PTP over Ethernet if its EtherType matches
*         one of the configured values.
* @param[in] etherType                - PTP EtherType index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] etherType                - PTP EtherType0 or EtherType1 according to the index.
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_OUT_OF_RANGE          - on wrong etherType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEtherTypeSet
(
    IN GT_U32   etherTypeIndex,
    IN GT_U32   etherType
);

/**
* @internal prvTgfPtpEtherTypeGet function
* @endinternal
*
* @brief   Get ethertypes of PTP over Ethernet packets.
*
* @param[out] etherTypePtr             - (pointer to) PTP EtherType0 or EtherType1 according
*                                      to the index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEtherTypeGet
(
    IN  GT_U32   etherTypeIndex,
    OUT GT_U32   *etherTypePtr
);

/**
* @internal prvTgfPtpUdpDestPortsSet function
* @endinternal
*
* @brief   Configure UDP destination port0/port1 of PTP over UDP packets.
*         A packet is identified as PTP over UDP if it is a UDP packet, whose
*         destination port matches one of the configured ports.
* @param[in] udpPortIndex             - UDP port index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] udpPortNum               - UDP port1/port0 number according to the index,
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_OUT_OF_RANGE          - on wrong udpPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpUdpDestPortsSet
(
    IN GT_U32   udpPortIndex,
    IN GT_U32   udpPortNum
);

/**
* @internal prvTgfPtpUdpDestPortsGet function
* @endinternal
*
* @brief   Get UDP destination port0/port1 of PTP over UDP packets.
*
* @param[in] udpPortIndex             - UDP port index
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] udpPortNumPtr            - (pointer to) UDP port0/port1 number, according
*                                      to the index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpUdpDestPortsGet
(
    IN  GT_U32   udpPortIndex,
    OUT GT_U32   *udpPortNumPtr
);

/**
* @internal prvTgfPtpMessageTypeCmdSet function
* @endinternal
*
* @brief   Configure packet command per PTP message type.
*         The message type is extracted from the PTP header.
* @param[in] portNum                  - port number.
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] domainNum                - domain number
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] command                  - assigned  to the packet.
*                                      Acceptable values: FORWARD, MIRROR, TRAP, HARD_DROP and
*                                      SOFT_DROP.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpMessageTypeCmdSet
(
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   domainNum,
    IN GT_U32                   messageType,
    IN CPSS_PACKET_CMD_ENT      command
);

/**
* @internal prvTgfPtpMessageTypeCmdGet function
* @endinternal
*
* @brief   Get packet command per PTP message type.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] commandPtr               - (pointer to) assigned command to the packet.
*                                      Acceptable values: FORWARD, MIRROR, TRAP, HARD_DROP and
*                                      SOFT_DROP.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpMessageTypeCmdGet
(
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   domainNum,
    IN  GT_U32                   messageType,
    OUT CPSS_PACKET_CMD_ENT      *commandPtr
);

/**
* @internal prvTgfPtpCpuCodeBaseSet function
* @endinternal
*
* @brief   Set CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
* @param[in] cpuCode                  - The base of CPU code assigned to PTP packets mirrored
*                                      or trapped to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpCpuCodeBaseSet
(
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode
);

/**
* @internal prvTgfPtpCpuCodeBaseGet function
* @endinternal
*
* @brief   Get CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
*
* @param[out] cpuCodePtr               - (pointer to) The base of CPU code assigned to PTP
*                                      packets mirrored or trapped to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpCpuCodeBaseGet
(
    OUT CPSS_NET_RX_CPU_CODE_ENT     *cpuCodePtr
);

/**
* @internal prvTgfPtpTodCounterFunctionSet function
* @endinternal
*
* @brief   The function configures type of TOD counter action that will be
*         performed once triggered by prvTgfPtpTodCounterFunctionTriggerSet()
*         API or by pulse from an external interface.
* @param[in] direction                - Tx, Rx or both directions
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] tgfFunction              - One of the four possible TOD counter functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTodCounterFunctionSet
(
    IN CPSS_PORT_DIRECTION_ENT                  direction,
    IN PRV_TGF_PTP_TAI_ID_STC                   *tgfTaiIdPtr,
    IN PRV_TGF_PTP_TOD_COUNTER_FUNC_ENT         tgfFunction
);

/**
* @internal prvTgfPtpTodCounterFunctionGet function
* @endinternal
*
* @brief   Get type of TOD counter action that will be performed once triggered by
*         prvTgfPtpTodCounterFunctionTriggerSet() API or by pulse from an
*         external interface.
* @param[in] direction                - Tx, Rx or both directions
*                                      taiIdPtr        - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTodCounterFunctionGet
(
    IN  CPSS_PORT_DIRECTION_ENT                 direction,
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT PRV_TGF_PTP_TOD_COUNTER_FUNC_ENT        *tgfFunctionPtr
);

/**
* @internal prvTgfPtpTodCounterFunctionTriggerSet function
* @endinternal
*
* @brief   Trigger TOD (Time of Day) counter function accorging to the function set
*         by prvTgfPtpTodCounterFunctionSet().
* @param[in] direction                - Tx, Rx or both directions
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTodCounterFunctionTriggerSet
(
    IN CPSS_PORT_DIRECTION_ENT     direction,
    IN PRV_TGF_PTP_TAI_ID_STC      *tgfTaiIdPtr
);

/**
* @internal prvTgfPtpTaiTodCounterFunctionAllTriggerSet function
* @endinternal
*
* @brief   Triggeres TOD update operation for all TAI units.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodCounterFunctionAllTriggerSet
(
    IN GT_VOID
);

/**
* @internal prvTgfPtpTodCounterFunctionTriggerGet function
* @endinternal
*
* @brief   Get status of trigger TOD (Time of Day) counter function.
*
* @param[in] direction                - Tx, Rx
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - The trigger is on, the TOD
*                                      function is not finished.
*                                      GT_FALSE - The trigger is off, the TOD function is
*                                      finished.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTodCounterFunctionTriggerGet
(
    IN CPSS_PORT_DIRECTION_ENT     direction,
    IN  PRV_TGF_PTP_TAI_ID_STC     *tgfTaiIdPtr,
    OUT GT_BOOL                    *enablePtr
);

/**
* @internal prvTgfPtpTaiClockModeSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Mode.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] tgfClockMode             - clock interface using mode.
*                                      Output mode relevant to Global TAI Instance only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockModeSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  PRV_TGF_PTP_TAI_CLOCK_MODE_ENT          tgfClockMode
);

/**
* @internal prvTgfPtpTaiClockModeGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Mode.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] tgfClockModePtr          - (pointer to) clock interface using mode.
*                                      Output mode relevant to Global TAI Instance only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockModeGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT PRV_TGF_PTP_TAI_CLOCK_MODE_ENT          *tgfClockModePtr
);

/**
* @internal prvTgfPtpTaiInternalClockGenerateEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Internal Clock Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   - Internal Clock Generate Enable.
*                                      GT_TRUE - the internal clock generator
*                                      generates a clock signal
*                                      GT_FALSE - the internal clock not generated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiInternalClockGenerateEnableSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  GT_BOOL                                 enable
);

/**
* @internal prvTgfPtpTaiInternalClockGenerateEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Internal Clock Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] enablePtr                - (pointer to) Internal Clock Generate Enable.
*                                      GT_TRUE - the internal clock generator
*                                      generates a clock
*                                      GT_FALSE - the internal clock not generated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiInternalClockGenerateEnableGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal prvTgfPtpTaiPtpPClockDriftAdjustEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   - PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPtpPClockDriftAdjustEnableSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  GT_BOOL                                 enable
);

/**
* @internal prvTgfPtpTaiPtpPClockDriftAdjustEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] enablePtr                - (pointer to) PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPtpPClockDriftAdjustEnableGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal prvTgfPtpTaiCaptureOverrideEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Capture Override Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   - Capture Override Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiCaptureOverrideEnableSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  GT_BOOL                                 enable
);

/**
* @internal prvTgfPtpTaiCaptureOverrideEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Capture Override Enable
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] enablePtr                - (pointer to) Capture Override Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiCaptureOverrideEnableGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal prvTgfPtpTaiInputTriggersCountEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Count Input Triggers Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   - Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see tgfPrvPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiInputTriggersCountEnableSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  GT_BOOL                                 enable
);

/**
* @internal prvTgfPtpTaiInputTriggersCountEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Count Input Triggers Enable
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] enablePtr                - (pointer to) Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see tgfPrvPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiInputTriggersCountEnableGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal prvTgfPtpTaiExternalPulseWidthSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Pulse Width.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] extPulseWidth            - External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on bad extPulseWidth
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiExternalPulseWidthSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    IN  GT_U32                                  extPulseWidth
);

/**
* @internal prvTgfPtpTaiExternalPulseWidthGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Pulse Width
*
* @param[out] extPulseWidthPtr         -  (pointer to) External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiExternalPulseWidthGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC                  *tgfTaiIdPtr,
    OUT GT_U32                                  *extPulseWidthPtr
);

/**
* @internal prvTgfPtpTaiTodSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD values.
*         The TOD will be updated by triggering an appropriate function.
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] tgfTodValueType          - type of TOD value.
*                                      Valid types are
*                                      PRV_TGF_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,
*                                      PRV_TGF_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,
*                                      PRV_TGF_PTP_TAI_TOD_TYPE_LOAD_VALUE_E.
* @param[in] tgfTodValuePtr           - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API does not activate any triggers, other APIs does it
*
*/
GT_STATUS prvTgfPtpTaiTodSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    IN  PRV_TGF_PTP_TAI_TOD_TYPE_ENT        tgfTodValueType,
    IN  PRV_TGF_PTP_TOD_COUNT_STC           *tgfTodValuePtr
);

/**
* @internal prvTgfPtpTaiTodGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD values.
*         The TOD was captured by triggering an appropriate function.
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
* @param[in] tgfTodValueType          - type of TOD value.
*
* @param[out] tgfTodValuePtr           - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    IN  PRV_TGF_PTP_TAI_TOD_TYPE_ENT        tgfTodValueType,
    OUT PRV_TGF_PTP_TOD_COUNT_STC           *tgfTodValuePtr
);

/**
* @internal prvTgfPtpTaiOutputTriggerEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   -   output trigger generation.
*                                      GT_TRUE  - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note use prvTgfPtpTaiTodSet with
*       PRV_TGF_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E and
*       the time to trigger generation.
*
*/
GT_STATUS prvTgfPtpTaiOutputTriggerEnableSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    IN  GT_BOOL                             enable
);

/**
* @internal prvTgfPtpTaiOutputTriggerEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] enablePtr                - (pointer to) enable output trigger generation.
*                                      GT_TRUE  - enable GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiOutputTriggerEnableGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal prvTgfPtpTaiTodStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Step.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] tgfTodStepPtr            - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodStepSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  PRV_TGF_PTP_TAI_TOD_STEP_STC    *tgfTodStepPtr
);

/**
* @internal prvTgfPtpTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] tgfTodStepPtr            - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodStepGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT PRV_TGF_PTP_TAI_TOD_STEP_STC    *tgfTodStepPtr
);

/**
* @internal prvTgfPtpTaiPulseDelaySet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Trigger Propagation Delay.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] nanoSeconds              - nano seconds delay.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPulseDelaySet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          nanoSeconds
);

/**
* @internal prvTgfPtpTaiPulseDelayGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Trigger Propagation Delay.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds delay.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPulseDelayGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *nanoSecondsPtr
);

/**
* @internal prvTgfPtpTaiClockDelaySet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Clock Propagation Delay.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] nanoSeconds              - nano seconds delay.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockDelaySet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          nanoSeconds
);

/**
* @internal prvTgfPtpTaiClockDelayGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Clock Propagation Delay.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds delay.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockDelayGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *nanoSecondsPtr
);

/**
* @internal prvTgfPtpTaiPClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PClock Cycle.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] nanoSeconds              - nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPClockCycleSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          nanoSeconds
);

/**
* @internal prvTgfPtpTaiPClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PClock Cycle.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiPClockCycleGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *nanoSecondsPtr
);

/**
* @internal prvTgfPtpTaiClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Cycle.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] seconds                  -  part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] nanoSeconds              - nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockCycleSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          seconds,
    IN  GT_U32                          nanoSeconds
);

/**
* @internal prvTgfPtpTaiClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Cycle.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*                                      Single unit must be specified.
*
* @param[out] secondsPtr               - (pointer to) seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiClockCycleGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *secondsPtr,
    OUT GT_U32                          *nanoSecondsPtr
);

/**
* @internal prvTgfPtpTaiTodCaptureStatusSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Capture Status.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] captureIndex             - Capture Index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] valid                    - TOD Capture is valid.
*                                      GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodCaptureStatusSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          captureIndex,
    IN  GT_BOOL                         valid
);

/**
* @internal prvTgfPtpTaiTodCaptureStatusGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Capture Status.
*
* @param[out] validPtr                 - (pointer to)Capture is valid.
*                                      GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiTodCaptureStatusGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          captureIndex,
    OUT GT_BOOL                         *validPtr
);

/**
* @internal prvTgfPtpTaiTodUpdateCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Update Counter.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] valuePtr                 - (pointer to)TOD Update Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counter is Clear On Read. Value incremented at each
*       TOD Update/Increment/Decrement function.
*
*/
GT_STATUS prvTgfPtpTaiTodUpdateCounterGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *valuePtr
);

/**
* @internal prvTgfPtpTaiIncomingTriggerCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Trigger Counter.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] value                    -  to set.
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiIncomingTriggerCounterSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          value
);

/**
* @internal prvTgfPtpTaiIncomingClockCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Clock Counter.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] enable                   -  counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
* @param[in] value                    -  to set.
*                                      (APPLICABLE RANGES: 0..2^30-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiIncomingClockCounterSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_BOOL                         enable,
    IN  GT_U32                          value
);

/**
* @internal prvTgfPtpTaiIncomingClockCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Clock Counter.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] enablePtr                - (pointer to)enable counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiIncomingClockCounterGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_BOOL                         *enablePtr,
    OUT GT_U32                          *valuePtr
);

/**
* @internal prvTgfPtpTaiFrequencyDriftThesholdsSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Frequency Drift Thesholds.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] interruptThreshold       - the drift value threshold causing interrupt.
*                                      Notifies about the clock frequency out of sync.
*                                      (APPLICABLE RANGES: 0..2^24-1.)
* @param[in] adjustThreshold          - the drift value threshold that triggers
*                                      the adjustment logic. Prevents minor jitters
*                                      from affecting the drift adjustment.
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiFrequencyDriftThesholdsSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    IN  GT_U32                          interruptThreshold,
    IN  GT_U32                          adjustThreshold
);

/**
* @internal prvTgfPtpTaiFrequencyDriftThesholdsGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Frequency Drift Thesholds.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] interruptThresholdPtr    - (pointer to)the drift value threshold causing interrupt.
*                                      Notifies about the clock frequency out of sync.
*                                      (APPLICABLE RANGES: 0..2^24-1.)
* @param[out] adjustThresholdPtr       - (pointer to)the drift value threshold that triggers
*                                      the adjustment logic. Prevents minor jitters
*                                      from affecting the drift adjustment.
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTaiFrequencyDriftThesholdsGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC          *tgfTaiIdPtr,
    OUT GT_U32                          *interruptThresholdPtr,
    OUT GT_U32                          *adjustThresholdPtr
);

/**
* @internal prvTgfPtpTsuControlSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) Control.
*
* @param[in] portNum                  - port number.
* @param[in] tgfControlPtr            - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuControlSet
(
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  PRV_TGF_PTP_TSU_CONTROL_STC     *tgfControlPtr
);

/**
* @internal prvTgfPtpTsuControlGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) Control.
*
* @param[in] portNum                  - port number.
*
* @param[out] tgfControlPtr            - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuControlGet
(
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT PRV_TGF_PTP_TSU_CONTROL_STC       *tgfControlPtr
);

/**
* @internal prvTgfPtpTsuTxTimestampQueueRead function
* @endinternal
*
* @brief   Read TSU (Time Stamp Unit) TX Timestamp Queue Entry.
*
* @param[in] portNum                  - port number.
* @param[in] queueNum                 - queue number.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Entry deleted from the Queue by reading it.
*
*/
GT_STATUS prvTgfPtpTsuTxTimestampQueueRead
(
    IN  GT_PHYSICAL_PORT_NUM                               portNum,
    IN  GT_U32                                             queueNum,
    OUT PRV_TGF_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC       *tgfEntryPtr
);

/**
* @internal prvTgfPtpTsuCountersClear function
* @endinternal
*
* @brief   Clear All TSU (Time Stamp Unit) Packet counters.
*
* @param[in] portNum                  - port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuCountersClear
(
    IN  GT_PHYSICAL_PORT_NUM              portNum
);

/**
* @internal prvTgfPtpTsuPacketCouterGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) packet counter.
*
* @param[in] portNum                  - port number.
* @param[in] tgfCounterType           - counter type (see enum)
*
* @param[out] valuePtr                 - (pointer to) counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuPacketCouterGet
(
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  PRV_TGF_PTP_TSU_PACKET_COUNTER_TYPE_ENT   tgfCounterType,
    OUT GT_U32                                    *valuePtr
);

/**
* @internal prvTgfPtpTsuNtpTimeOffsetSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) NTP Time Offset.
*
* @param[in] portNum                  - port number.
* @param[in] ntpTimeOffset            - NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuNtpTimeOffsetSet
(
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  GT_U32                            ntpTimeOffset
);

/**
* @internal prvTgfPtpTsuNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) NTP Time Offset.
*
* @param[in] portNum                  - port number.
*
* @param[out] ntpTimeOffsetPtr         - (pointer to) NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsuNtpTimeOffsetGet
(
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT GT_U32                            *ntpTimeOffsetPtr
);

/**
* @internal prvTgfPtpTsTagGlobalCfgSet function
* @endinternal
*
* @brief   Set Timestamp Global Configuration.
*
* @param[in] tgfTsTagGlobalCfgPtr     - (pointer to) TS Tag Global Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsTagGlobalCfgSet
(
    IN  PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   *tgfTsTagGlobalCfgPtr
);

/**
* @internal prvTgfPtpTsTagGlobalCfgGet function
* @endinternal
*
* @brief   Get Timestamp Global Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsTagGlobalCfgGet
(
    OUT PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   *tgfTsTagGlobalCfgPtr
);

/**
* @internal prvTgfPtpTsTagPortCfgSet function
* @endinternal
*
* @brief   Set Timestamp Port Configuration.
*
* @param[in] portNum                  - port number.
* @param[in] tgfTsTagPortCfgPtr       - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsTagPortCfgSet
(
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     *tgfTsTagPortCfgPtr
);

/**
* @internal prvTgfPtpTsTagPortCfgGet function
* @endinternal
*
* @brief   Get Timestamp Port Configuration.
*
* @param[in] portNum                  - port number.
*
* @param[out] tgfTsTagPortCfgPtr       - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsTagPortCfgGet
(
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     *tgfTsTagPortCfgPtr
);

/**
* @internal prvTgfPtpTsDelayIngressPortDelaySet function
* @endinternal
*
* @brief   Set Ingress Port Delay Correction.
*
* @param[in] portNum                  - port number.
* @param[in] ingressDelayCorr         - Ingress Port Delay Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayIngressPortDelaySet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_32                   ingressDelayCorr
);

/**
* @internal prvTgfPtpTsDelayIngressPortDelayGet function
* @endinternal
*
* @brief   Get Port Delay Correction Configuration.
*
* @param[in] portNum                  - port number.
*
* @param[out] ingressDelayCorrPtr      - (pointer to) Ingress Port Delay Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayIngressPortDelayGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_32                   *ingressDelayCorrPtr
);

/**
* @internal prvTgfPtpTsDelayEgressPipeDelaySet function
* @endinternal
*
* @brief   Set Egress Pipe Delay Correction.
*
* @param[in] portNum                  - port number.
* @param[in] egrPipeDelayCorr         - Egress Pipe Delay Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressPipeDelaySet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_32                   egrPipeDelayCorr
);

/**
* @internal prvTgfPtpTsDelayEgressPipeDelayGet function
* @endinternal
*
* @brief   Get Egress Pipe Delay Correction.
*
* @param[in] portNum                  - port number.
*
* @param[out] egrPipeDelayCorrPtr      - (pointer to) Egress Pipe Delay Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressPipeDelayGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_32                   *egrPipeDelayCorrPtr
);

/**
* @internal prvTgfPtpTsDelayEgressAsymmetryCorrectionSet function
* @endinternal
*
* @brief   Set Egress Asymmetry Correction.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] egrAsymmetryCorr         - Egress Asymmetry Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressAsymmetryCorrectionSet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainNum,
    IN  GT_32                   egrAsymmetryCorr
);

/**
* @internal prvTgfPtpTsDelayEgressAsymmetryCorrectionGet function
* @endinternal
*
* @brief   Get Egress Asymmetry Correction.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..8.)
*
* @param[out] egrAsymmetryCorrPtr      - (pointer to) Egress Asymmetry Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressAsymmetryCorrectionGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainNum,
    OUT GT_32                   *egrAsymmetryCorrPtr
);

/**
* @internal prvTgfPtpTsDelayIngressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Ingress Correction Field Piggyback Enable.
*
* @param[in] portNum                  - port number.
* @param[in] ingrCorrFldPBEnable      - Ingress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayIngressCorrFieldPiggybackEnableSet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 ingrCorrFldPBEnable
);

/**
* @internal prvTgfPtpTsDelayIngressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Ingress Correction Field Piggyback Enable.
*
* @param[in] portNum                  - port number.
*
* @param[out] ingrCorrFldPBEnablePtr   - (pointer to) Ingress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayIngressCorrFieldPiggybackEnableGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *ingrCorrFldPBEnablePtr
);

/**
* @internal prvTgfPtpTsDelayEgressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Egress Correction Field Piggyback Enable.
*
* @param[in] portNum                  - port number.
* @param[in] egrCorrFldPBEnable       - Egress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressCorrFieldPiggybackEnableSet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 egrCorrFldPBEnable
);

/**
* @internal prvTgfPtpTsDelayEgressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Egress Correction Field Piggyback Enable.
*
* @param[in] portNum                  - port number.
*
* @param[out] egrCorrFldPBEnablePtr    - (pointer to) Egress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressCorrFieldPiggybackEnableGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *egrCorrFldPBEnablePtr
);

/**
* @internal prvTgfPtpTsDelayEgressTimeCorrTaiSelectModeSet function
* @endinternal
*
* @brief   Set Egress Time Correction TAI Select Mode.
*
* @param[in] portNum                  - port number.
* @param[in] tgfEgrTimeCorrTaiSelMode - Egress Time Correction TAI Select Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressTimeCorrTaiSelectModeSet
(
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_ENT     tgfEgrTimeCorrTaiSelMode
);

/**
* @internal prvTgfPtpTsDelayEgressTimeCorrTaiSelectModeGet function
* @endinternal
*
* @brief   Get Egress Time Correction TAI Select Mode.
*
* @param[in] portNum                  - port number.
*
* @param[out] tgfEgrTimeCorrTaiSelModePtr - (pointer to)Egress Time Correction TAI Select Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressTimeCorrTaiSelectModeGet
(
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    OUT PRV_TGF_PTP_TS_DELAY_TAI_SEL_MODE_ENT     *tgfEgrTimeCorrTaiSelModePtr
);

/**
* @internal prvTgfPtpTsDelayEgressTimestampTaiSelectSet function
* @endinternal
*
* @brief   Set Egress Timestamp TAI Select.
*
* @param[in] portNum                  - port number.
* @param[in] egrTsTaiNum              - Egress Timestamp TAI Number.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressTimestampTaiSelectSet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  egrTsTaiNum
);

/**
* @internal prvTgfPtpTsDelayEgressTimestampTaiSelectGet function
* @endinternal
*
* @brief   Get Egress Timestamp TAI Select.
*
* @param[in] portNum                  - port number.
*
* @param[out] egrTsTaiNumPtr           - (pointer to)Egress Timestamp TAI Number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDelayEgressTimestampTaiSelectGet
(
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *egrTsTaiNumPtr
);

/**
* @internal prvTgfPtpTsCfgTableSet function
* @endinternal
*
* @brief   Set Timestamp Configuration Table Entry.
*
* @param[in] entryIndex               - entry index.
*                                      (APPLICABLE RANGES: 0..127.)
* @param[in] tgfEntryPtr              - (pointer to)Timestamp Configuration Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsCfgTableSet
(
    IN  GT_U32                         entryIndex,
    IN  PRV_TGF_PTP_TS_CFG_ENTRY_STC   *tgfEntryPtr
);

/**
* @internal prvTgfPtpTsCfgTableGet function
* @endinternal
*
* @brief   Get Timestamp Configuration Table Entry.
*
* @param[in] entryIndex               - entry index.
*                                      (APPLICABLE RANGES: 0..127.)
*
* @param[out] tgfEntryPtr              - (pointer to)Timestamp Configuration Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsCfgTableGet
(
    IN  GT_U32                         entryIndex,
    OUT PRV_TGF_PTP_TS_CFG_ENTRY_STC   *tgfEntryPtr
);

/**
* @internal prvTgfPtpTsLocalActionTableSet function
* @endinternal
*
* @brief   Set Timestamp Local Action Table Entry.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] tgfEntryPtr              - (pointer to)Timestamp Local Action Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsLocalActionTableSet
(
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainNum,
    IN  GT_U32                                  messageType,
    IN  PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC   *tgfEntryPtr
);

/**
* @internal prvTgfPtpTsLocalActionTableGet function
* @endinternal
*
* @brief   Get Timestamp Local Action Table Entry.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] tgfEntryPtr              - (pointer to)Timestamp Local Action Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsLocalActionTableGet
(
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainNum,
    IN  GT_U32                                  messageType,
    OUT PRV_TGF_PTP_TS_LOCAL_ACTION_ENTRY_STC   *tgfEntryPtr
);

/**
* @internal prvTgfPtpTsUdpChecksumUpdateModeSet function
* @endinternal
*
* @brief   Set Timestamp UDP Checksum Update Mode.
*
* @param[in] tgfUdpCsUpdModePtr       - (pointer to)Timestamp UDP Checksum Update Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsUdpChecksumUpdateModeSet
(
    IN  PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC *tgfUdpCsUpdModePtr
);

/**
* @internal prvTgfPtpTsUdpChecksumUpdateModeGet function
* @endinternal
*
* @brief   Get Timestamp UDP Checksum Update Mode.
*
* @param[out] tgfUdpCsUpdModePtr       - (pointer to)Timestamp UDP Checksum Update Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsUdpChecksumUpdateModeGet
(
    OUT PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC *tgfUdpCsUpdModePtr
);

/**
* @internal prvTgfPtpOverEthernetEnableSet function
* @endinternal
*
* @brief   Set enable PTP over Ethernet packet.
*
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpOverEthernetEnableSet
(
    IN  GT_BOOL   enable
);

/**
* @internal prvTgfPtpOverEthernetEnableGet function
* @endinternal
*
* @brief   Get enable PTP over Ethernet packet.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpOverEthernetEnableGet
(
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfPtpOverUdpEnableSet function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpOverUdpEnableSet
(
    IN  GT_BOOL   enable
);

/**
* @internal prvTgfPtpOverUdpEnableGet function
* @endinternal
*
* @brief   Get enable PTP over UDP packet.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpOverUdpEnableGet
(
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfPtpDomainModeSet function
* @endinternal
*
* @brief   Set PTP domain mode.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] tgfDomainMode            - domain mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpDomainModeSet
(
    IN  GT_U32                                 domainNum,
    IN  PRV_TGF_PTP_INGRESS_DOMAIN_MODE_ENT    tgfDomainMode
);

/**
* @internal prvTgfPtpDomainModeGet function
* @endinternal
*
* @brief   Get PTP domain mode.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] tgfDomainModePtr         - (pointer to) domain mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpDomainModeGet
(
    IN  GT_U32                                 domainNum,
    OUT PRV_TGF_PTP_INGRESS_DOMAIN_MODE_ENT    *tgfDomainModePtr
);

/**
* @internal prvTgfPtpDomainV1IdSet function
* @endinternal
*
* @brief   Set PTP V1 domain Id.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainIdArr[4]           - domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS prvTgfPtpDomainV1IdSet
(
    IN  GT_U32       domainNum,
    IN  GT_U32       domainIdArr[4]
);

/**
* @internal prvTgfPtpDomainV1IdGet function
* @endinternal
*
* @brief   Get PTP V1 domain Id.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainIdArr[4]           - domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS prvTgfPtpDomainV1IdGet
(
    IN  GT_U32       domainNum,
    OUT GT_U32       domainIdArr[4]
);

/**
* @internal prvTgfPtpDomainV2IdSet function
* @endinternal
*
* @brief   Set PTP V2 domain Id.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainId                 - domain Id
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS prvTgfPtpDomainV2IdSet
(
    IN  GT_U32       domainNum,
    IN  GT_U32       domainId
);

/**
* @internal prvTgfPtpDomainV2IdGet function
* @endinternal
*
* @brief   Get PTP V2 domain Id.
*
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainIdPtr              - (pointer to) domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS prvTgfPtpDomainV2IdGet
(
    IN  GT_U32       domainNum,
    OUT GT_U32       *domainIdPtr
);

/**
* @internal prvTgfPtpEgressDomainTableSet function
* @endinternal
*
* @brief   Set Egress Domain Table Entry.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] tgfEntryPtr              - (pointer to) Domain Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEgressDomainTableSet
(
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainNum,
    IN  PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC     *tgfEntryPtr
);

/**
* @internal prvTgfPtpEgressDomainTableGet function
* @endinternal
*
* @brief   Get Egress Domain Table Entry.
*
* @param[in] portNum                  - port number.
* @param[in] domainNum                - domain number.
*                                      (APPLICABLE RANGES: 0..4.)
*
* @param[out] tgfEntryPtr              - (pointer to) Egress Domain Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEgressDomainTableGet
(
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainNum,
    OUT PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC     *tgfEntryPtr
);

/**
* @internal prvTgfPtpIngressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Exception Configuration.
*
* @param[in] tgfIngrExceptionCfgPtr   - (pointer to) PTP packet Ingress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpIngressExceptionCfgSet
(
    IN  PRV_TGF_PTP_INGRESS_EXCEPTION_CFG_STC   *tgfIngrExceptionCfgPtr
);

/**
* @internal prvTgfPtpIngressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Ingress Exception Configuration.
*
* @param[out] tgfIngrExceptionCfgPtr   - (pointer to) PTP packet Ingress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpIngressExceptionCfgGet
(
    OUT PRV_TGF_PTP_INGRESS_EXCEPTION_CFG_STC   *tgfIngrExceptionCfgPtr
);

/**
* @internal prvTgfPtpIngressPacketCheckingModeSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Checking Mode.
*
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] domainNum                - domain number
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] tgfCheckingMode          - PTP packet ingress checking mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpIngressPacketCheckingModeSet
(
    IN  GT_U32                                   domainNum,
    IN  GT_U32                                   messageType,
    IN  PRV_TGF_PTP_INGRESS_CHECKING_MODE_ENT    tgfCheckingMode
);

/**
* @internal prvTgfPtpIngressPacketCheckingModeGet function
* @endinternal
*
* @brief   Get PTP packet Ingress Checking Mode.
*
* @param[in] domainNum                - domain number
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] tgfCheckingModePtr       - (pointer to)PTP packet ingress checking mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpIngressPacketCheckingModeGet
(
    IN  GT_U32                                   domainNum,
    IN  GT_U32                                   messageType,
    OUT PRV_TGF_PTP_INGRESS_CHECKING_MODE_ENT    *tgfCheckingModePtr
);

/**
* @internal prvTgfPtpIngressExceptionCounterGet function
* @endinternal
*
* @brief   Get PTP Ingress Exception packet Counter.
*
* @param[out] counterPtr               - (pointer to) PTP Ingress Exception packet Counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is Clear On Read.
*       Returns the summary from all devices.
*
*/
GT_STATUS prvTgfPtpIngressExceptionCounterGet
(
    OUT GT_U32   *counterPtr
);

/**
* @internal prvTgfPtpEgressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Egress Exception Configuration.
*
* @param[in] tgfEgrExceptionCfgPtr    - (pointer to) PTP packet Egress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEgressExceptionCfgSet
(
    IN  PRV_TGF_PTP_EGRESS_EXCEPTION_CFG_STC      *tgfEgrExceptionCfgPtr
);

/**
* @internal prvTgfPtpEgressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Configuration.
*
* @param[out] tgfEgrExceptionCfgPtr    - (pointer to) PTP packet Egress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpEgressExceptionCfgGet
(
    OUT PRV_TGF_PTP_EGRESS_EXCEPTION_CFG_STC     *tgfEgrExceptionCfgPtr
);

/**
* @internal prvTgfPtpEgressExceptionCountersGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Counters.
*
* @param[out] tgfEgrExceptionCntPtr    - (pointer to) PTP packet Egress Exception Counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are Clear On Read.
*
*/
GT_STATUS prvTgfPtpEgressExceptionCountersGet
(
    OUT PRV_TGF_PTP_EGRESS_EXCEPTION_COUNTERS_STC     *tgfEgrExceptionCntPtr
);

/**
* @internal prvTgfPtpTsIngressTimestampQueueEntryRead function
* @endinternal
*
* @brief   Read Ingress Timestamp Queue Entry.
*
* @param[in] queueNum                 - Queue Number
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] tgfTsQueueEntryPtr       - (pointer to) Ingress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
GT_STATUS prvTgfPtpTsIngressTimestampQueueEntryRead
(
    IN  GT_U32                                             queueNum,
    OUT PRV_TGF_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   *tgfTsQueueEntryPtr
);

/**
* @internal prvTgfPtpTsEgressTimestampQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry.
*
* @param[in] queueNum                 - Queue Number
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] tgfTsQueueEntryPtr       - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
GT_STATUS prvTgfPtpTsEgressTimestampQueueEntryRead
(
    IN  GT_U32                                             queueNum,
    OUT PRV_TGF_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    *tgfTsQueueEntryPtr
);

/**
* @internal prvTgfPtpTsMessageTypeToQueueIdMapSet function
* @endinternal
*
* @brief   Set PTP Message Type To Queue Id Map.
*
* @param[in] idMapBmp                 - PTP Message Type To Queue Id Map.
*                                      16 bits, bit per message type
*                                      0 - queue0, 1 - queue1
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsMessageTypeToQueueIdMapSet
(
    IN  GT_U32       idMapBmp
);

/**
* @internal prvTgfPtpTsMessageTypeToQueueIdMapGet function
* @endinternal
*
* @brief   Get PTP Message Type To Queue Id Map.
*
* @param[out] idMapBmpPtr              - (pointer to) PTP Message Type To Queue Id Map.
*                                      16 bits, bit per message type
*                                      0 - queue0, 1 - queue1
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsMessageTypeToQueueIdMapGet
(
    OUT GT_U32       *idMapBmpPtr
);

/**
* @internal prvTgfPtpTsQueuesEntryOverrideEnableSet function
* @endinternal
*
* @brief   Set Timestamp Queues Override Enable.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
* @param[in] enable                   - when the TS Queue is full the new timestamp
*                                      GT_TRUE  - overrides an oldest Queue entry.
*                                      GT_FALSE - not queued.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note prvTgfPtpTsuControlSet should configure the same value per port
*
*/
GT_STATUS prvTgfPtpTsQueuesEntryOverrideEnableSet
(
    IN  GT_BOOL     enable
);

/**
* @internal prvTgfPtpTsQueuesEntryOverrideEnableGet function
* @endinternal
*
* @brief   Get Timestamp Queues Override Enable.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @param[out] enablePtr                - (pointer to)when the TS Queue is full the new timestamp
*                                      GT_TRUE  - overrides an oldest Queue entry.
*                                      GT_FALSE - not queued.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsQueuesEntryOverrideEnableGet
(
    OUT GT_BOOL     *enablePtr
);

/**
* @internal prvTgfPtpTsQueuesSizeSet function
* @endinternal
*
* @brief   Set Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
* @param[in] queueSize                - The size of each PTP Queue.
*                                      (APPLICABLE RANGES: 0..256.)
*                                      If Timestamp Queues Override is Enable,
*                                      the size should be (maximal - 2) == 254.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsQueuesSizeSet
(
    IN  GT_U32      queueSize
);

/**
* @internal prvTgfPtpTsQueuesSizeGet function
* @endinternal
*
* @brief   Get Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @param[out] queueSizePtr             - (pointer to)The size of each PTP Queue.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsQueuesSizeGet
(
    OUT GT_U32      *queueSizePtr
);

/**
* @internal prvTgfPtpTsDebugQueuesEntryIdsClear function
* @endinternal
*
* @brief   Set Timestamp Queues Current Entry Ids Clear.
*         For Debug only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDebugQueuesEntryIdsClear
(
    GT_VOID
);

/**
* @internal prvTgfPtpTsDebugQueuesEntryIdsGet function
* @endinternal
*
* @brief   Get Timestamp Queues Current Entry IDs.
*         For Debug only.
*
* @param[out] queueEntryId0Ptr         - (pointer to)The PTP Queue0 current entry Id.
* @param[out] queueEntryId1Ptr         - (pointer to)The PTP Queue1 current entry Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsDebugQueuesEntryIdsGet
(
    OUT GT_U32      *queueEntryId0Ptr,
    OUT GT_U32      *queueEntryId1Ptr
);

/**
* @internal prvTgfPtpTsNtpTimeOffsetSet function
* @endinternal
*
* @brief   Set TS NTP Time Offset.
*
* @param[in] ntpTimeOffset            - NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The same value should be configured per port.
*       See prvTgfPtpTsuNtpTimeOffsetSet.
*
*/
GT_STATUS prvTgfPtpTsNtpTimeOffsetSet
(
    IN  GT_U32                            ntpTimeOffset
);

/**
* @internal prvTgfPtpTsNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TS NTP Time Offset.
*
* @param[out] ntpTimeOffsetPtr         - (pointer to) NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPtpTsNtpTimeOffsetGet
(
    OUT GT_U32                            *ntpTimeOffsetPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfPtpGenh */


