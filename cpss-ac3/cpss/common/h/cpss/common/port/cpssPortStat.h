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
* @file cpssPortStat.h
*
* @brief CPSS definitions for port statistics and egress counters.
*
* @version   9
********************************************************************************
*/

#ifndef __cpssPortStath
#define __cpssPortStath

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>


/**
* @enum CPSS_PORT_INTERLAKEN_COUNTER_ID_ENT
 *
 * @brief Enumeration of Interlaken counter IDs
*/
typedef enum{

    /** number of received bytes (64bit value) */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_BYTE_E,

    /** number of received wrong packets (64bit value) */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_BAD_PKT_E,

    /** number of received packets (64bit value) */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_PKT_E,

    /** number of received bursts with CRC error (64bit value) */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_CRC_ERR_E,

    /** number of transmitted bytes (64bit value) */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_TX_BYTE_E,

    /** number of transmitted wrong packets (64bit value) */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_TX_BAD_PKT_E,

    /** number of transmitted packets (64bit value) */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_TX_PKT_E,

    /** @brief Per lane counter (0..23)
     *  Number of words received with Control Word framing ('x10')
     *  that don't match one of the defined Control Words
     */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_BLK_TYPE_ERR_E,

    /** @brief Per lane counter (0..23)
     *  Number of errors in the lane CRC
     */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_DIAG_CRC_ERR_E,

    /** @brief Per lane counter (0..23)
     *  Number of times a lane lost word boundary synchronization
     */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_WORD_SYNC_ERR_E,

    /** @brief Number of alignment sequences received in error
     *  (i.e., those that violate the current alignment)
     *  (16-bit value)
     */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_ALIGNMENT_ERR_E,

    /** @brief Number of times alignment was lost
     *  (after four consecutive RX alignment errors)(16-bit value)
     */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_ALIGNMENT_FAIL_E,

    /** Number of received bursts with incorrect size (44bit value) */
    CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_BURST_SIZE_ERR_E

} CPSS_PORT_INTERLAKEN_COUNTER_ID_ENT;


/**
* @enum CPSS_PORT_MAC_COUNTERS_ENT
 *
 * @brief Enumeration of the ethernet MAC Counters.
*/
typedef enum{

    /** @brief Sum of lengths of all good ethernet frames
     *  received that are neither bad ethernet frames nor
     *  MAC Control pkts. This does not include 802.3x
     *  pause messages, but it does include Bridge Control
     *  packets (LCAP, BPDU)
     */
    CPSS_GOOD_OCTETS_RCV_E = 0,

    /** Sum of lengths of all bad ethernet frames received */
    CPSS_BAD_OCTETS_RCV_E,

    /** @brief Number of frames not transmitted correctly or
     *  dropped due to internal MAC Tx error
     *  Used for TX FIFO underrun and TX CRC
     *  errors MAC MIB Counter.
     */
    CPSS_MAC_TRANSMIT_ERR_E,

    /** @brief Number of ethernet frames received that are
     *  neither bad ethernet frames nor MAC Control pkts.
     *  This includes Bridge Control packets (LCAP, BPDU)
     *  Not Supported for DxCh and ExMxPm.
     */
    CPSS_GOOD_PKTS_RCV_E,

    /** @brief Number of bad ethernet frames received
     *  Not Supported for DxCh and ExMxPm.
     */
    CPSS_BAD_PKTS_RCV_E,

    /** @brief Total number of good packets received that were
     *  directed to the broadcast address
     */
    CPSS_BRDC_PKTS_RCV_E,

    /** @brief Total number of good packets received that were
     *  directed to a multicast address
     */
    CPSS_MC_PKTS_RCV_E,

    /** @brief The number of received and transmitted good and
     *  bad frames which are 64 bytes in size.
     *  This does not include MAC Control Frames.
     */
    CPSS_PKTS_64_OCTETS_E,

    /** @brief The number of received and transmitted good and
     *  and bad frames which are 65 to 127 bytes in size.
     *  This does not include MAC Control Frames.
     */
    CPSS_PKTS_65TO127_OCTETS_E,

    /** @brief The number of received and transmitted good and
     *  and bad frames which are 128 to 255 bytes in size.
     *  This does not include MAC Control Frames.
     */
    CPSS_PKTS_128TO255_OCTETS_E,

    /** @brief The number of received and transmitted good and
     *  and bad frames which are 256 to 511 bytes in size.
     *  This does not include MAC Control Frames.
     */
    CPSS_PKTS_256TO511_OCTETS_E,

    /** @brief The number of received and transmitted good and
     *  and Bad frames which are 512 to 1023 bytes in size.
     *  This does not include MAC Control Frames.
     */
    CPSS_PKTS_512TO1023_OCTETS_E,

    /** @brief The number of received and transmitted good and
     *  and bad frames which are more than 1024 bytes in size.
     *  This does not include MAC Control Frames.
     *  (xCat2: the behavior of this counter is determined by
     *  oversized packets counter mode.
     *  See: cpssDxChMacOversizedPacketsCounterModeSet.)
     */
    CPSS_PKTS_1024TOMAX_OCTETS_E,

    /** @brief Sum of lengths of all good ethernet frames sent
     *  from this MAC.
     *  This does not include 802.3 Flow Control packets,
     *  packets dropped due to excessive collision or
     *  packets with a Tx Error.
     */
    CPSS_GOOD_OCTETS_SENT_E,

    /** @brief Number of ethernet frames sent from this MAC.
     *  This does not include 802.3 Flow Control packets,
     *  packets dropped due to excessive collision or
     *  packets with a Tx Error.
     *  Not Supported for DxCh and ExMxPm.
     */
    CPSS_GOOD_PKTS_SENT_E,

    /** @brief Number of frames dropped in the transmit MAC
     *  due to excessive collisions. This is applicable
     *  for Half-Duplex mode only.
     */
    CPSS_EXCESSIVE_COLLISIONS_E,

    /** @brief Total number of good packets sent that have a
     *  multicast destination MAC address.
     *  This does not include 802.3 Flow Control packets,
     *  packets dropped due to excessive collision or
     *  packets with a Tx Error.
     */
    CPSS_MC_PKTS_SENT_E,

    /** @brief Total number of good packets sent that have a
     *  broadcast destination MAC address.
     *  This does not include 802.3 Flow Control packets,
     *  packets dropped due to excessive collision or
     *  packets with a Tx Error.
     */
    CPSS_BRDC_PKTS_SENT_E,

    /** @brief number of received MAC Control frames that
     *  have an opcode different from 0001.
     *  Not Supported for DxCh and ExMxPm.
     */
    CPSS_UNRECOG_MAC_CNTR_RCV_E,

    /** number of Flow Control frames sent. */
    CPSS_FC_SENT_E,

    /** number of good Flow Control frames received. */
    CPSS_GOOD_FC_RCV_E,

    /** @brief Number of instances that the port was unable to
     *  receive packets due to insufficient bandwidth to
     *  one of the PP internal resources, such as the DRAM
     *  or buffer allocation.
     */
    CPSS_DROP_EVENTS_E,

    /** number of undersize packets received. */
    CPSS_UNDERSIZE_PKTS_E,

    /** number of fragments received. */
    CPSS_FRAGMENTS_PKTS_E,

    /** number of oversize packets received. */
    CPSS_OVERSIZE_PKTS_E,

    /** number of jabber packets received. */
    CPSS_JABBER_PKTS_E,

    /** @brief Number of Rx Error events seen by the receive side
     *  of the MAC
     */
    CPSS_MAC_RCV_ERROR_E,

    /** Number of CRC error events */
    CPSS_BAD_CRC_E,

    /** Total number of collisions seen by the MAC */
    CPSS_COLLISIONS_E,

    /** @brief Total number of late collisions seen by the MAC
     *  (xCat2: also counts unknown MAC control frames)
     */
    CPSS_LATE_COLLISIONS_E,

    /** @brief number of bad Flow Control frames received.
     *  Not Supported for DxCh, and ExMxPm.
     */
    CPSS_BadFC_RCV_E,

    /** @brief Number of Ethernet Unicast frames received that
     *  are not bad Ethernet frames or MAC Control packets.
     *  Note that this number includes Bridge Control
     *  packets such as LCAP and BPDU.
     *  Supported for DxCh and ExMxPm only.
     */
    CPSS_GOOD_UC_PKTS_RCV_E,

    /** @brief Number of good frames sent that had a Unicast
     *  destination MAC Address.
     *  Supported for DxCh and ExMxPm only.
     */
    CPSS_GOOD_UC_PKTS_SENT_E,

    /** @brief Valid Frame transmitted on halfduplex link that
     *  encountered more then one collision.
     *  Byte count and cast are valid.
     *  Supported for DxCh and ExMxPm only.
     */
    CPSS_MULTIPLE_PKTS_SENT_E,

    /** @brief Valid frame transmitted on halfduplex link with
     *  no collisions, but where the frame transmission
     *  was delayed due to media being busy. Byte count
     *  and cast are valid.
     *  Supported for DxCh and ExMxPm only.
     *  (xCat2: the behavior of this counter is determined by
     *  oversized packets counter mode.
     *  See: cpssDxChMacOversizedPacketsCounterModeSet.)
     */
    CPSS_DEFERRED_PKTS_SENT_E,

    /** @brief The number of received and transmitted, good and bad frames that are
     *  1024 to 1518 bytes in size.
     *  Relevant only for Interlaken channel MIB counter
     *  (APPLICABLE DEVICES: Bobcat2 and above)
     */
    CPSS_PKTS_1024TO1518_OCTETS_E,

    /** @brief The number of received and transmitted, good and bad frames that are
     *  more than 1519 bytes in size.
     *  Relevant only for Interlaken channel MIB counter
     *  (APPLICABLE DEVICES: Bobcat2 and above)
     */
    CPSS_PKTS_1519TOMAX_OCTETS_E,

    /** represent last number in enum */
    CPSS_LAST_MAC_COUNTER_NUM_E

} CPSS_PORT_MAC_COUNTERS_ENT;

/**
* @struct CPSS_PORT_MAC_COUNTER_SET_STC
 *
 * @brief Counters for BULK API of the ethernet MAC Counters.
 * The actual counter size (32 or 64 bits counters) is according
 * the hardware device, refer to data sheet for more info.
*/
typedef struct{

    /** @brief Number of ethernet frames received that are
     *  not bad ethernet frames or MAC Control pkts.
     *  This includes Bridge Control packets (LCAP, BPDU)
     */
    GT_U64 goodOctetsRcv;

    /** Sum of lengths of all bad ethernet frames received */
    GT_U64 badOctetsRcv;

    /** @brief Number of frames not transmitted correctly or
     *  dropped due to internal MAC Tx error
     */
    GT_U64 macTransmitErr;

    /** @brief Number of ethernet frames received that are
     *  neither bad ethernet frames nor MAC Control pkts.
     *  This includes Bridge Control packets (LCAP, BPDU)
     *  Not Supported for DxCh and ExMxPm.
     */
    GT_U64 goodPktsRcv;

    /** @brief Number of corrupted ethernet frames received
     *  Not Supported for DxCh and ExMxPm.
     */
    GT_U64 badPktsRcv;

    /** @brief Total number of undamaged packets received that
     *  were directed to the broadcast address
     */
    GT_U64 brdcPktsRcv;

    /** @brief Total number of undamaged packets received that
     *  were directed to a multicast address
     */
    GT_U64 mcPktsRcv;

    /** @brief Total number of received and transmitted undamaged
     *  and damaged frames which are 64 bytes in size.
     *  This does not include MAC Control Frames.
     *  In CG unit this counter counts only received frames.
     */
    GT_U64 pkts64Octets;

    /** @brief Total number of received and transmitted undamaged
     *  and damaged frames which are 65 to 127 bytes in
     *  size. This does not include MAC Control Frames.
     *  In CG unit this counter counts only received frames.
     */
    GT_U64 pkts65to127Octets;

    /** @brief Total number of received and transmitted undamaged
     *  and damaged frames which are 128 to 255 bytes in
     *  size. This does not include MAC Control Frames.
     *  In CG unit this counter counts only received frames.
     */
    GT_U64 pkts128to255Octets;

    /** @brief Total number of received and transmitted undamaged
     *  and damaged frames which are 256 to 511 bytes in
     *  size. This does not include MAC Control Frames.
     *  In CG unit this counter counts only received frames.
     */
    GT_U64 pkts256to511Octets;

    /** @brief Total number of received and transmitted undamaged
     *  and damaged frames which are 512 to 1023 bytes in
     *  size. This does not include MAC Control Frames.
     *  In CG unit this counter counts only received frames.
     */
    GT_U64 pkts512to1023Octets;

    /** @brief Total number of received and transmitted undamaged
     *  and damaged frames which are more than 1024 bytes
     *  in size. This does not include MAC Control Frames.
     *  (xCat2: the behavior of this counter is determined by
     *  oversized packets counter mode.
     *  See: cpssDxChMacOversizedPacketsCounterModeSet.)
     *  In CG unit this counter counts only received frames.
     */
    GT_U64 pkts1024tomaxOoctets;

    /** @brief Sum of lengths of all good ethernet frames sent
     *  from this MAC.
     *  This does not include 802.3 Flow Control packets,
     *  packets dropped due to excessive collision or
     *  packets with a Tx Error.
     */
    GT_U64 goodOctetsSent;

    /** @brief Number of ethernet frames sent from this MAC.
     *  This does not include 802.3 Flow Control packets,
     *  packets dropped due to excessive collision or
     *  packets with a Tx Error.
     *  Not Supported for DxCh and ExMxPm.
     */
    GT_U64 goodPktsSent;

    /** @brief Number of frames dropped in the transmit MAC due
     *  to excessive collisions. This is an applicable for
     *  Half-Duplex mode only.
     */
    GT_U64 excessiveCollisions;

    /** @brief Total number of good packets sent that have a
     *  multicast destination MAC address.
     *  This does not include 802.3 Flow Control packets,
     *  packets dropped due to excessive collision or
     *  packets with a Tx Error.
     */
    GT_U64 mcPktsSent;

    /** @brief Total number of good packets sent that have a
     *  broadcast destination MAC address.
     *  This does not include 802.3 Flow Control packets,
     *  packets dropped due to excessive collision or
     *  packets with a Tx Error.
     */
    GT_U64 brdcPktsSent;

    /** @brief Number of received MAC Control frames that have an
     *  opcode different from 00-01.
     *  Not Supported for DxCh and ExMxPm.
     */
    GT_U64 unrecogMacCntrRcv;

    /** Number of Flow Control frames sent. */
    GT_U64 fcSent;

    /** Number of good Flow Control frames received */
    GT_U64 goodFcRcv;

    /** @brief Number of instances that the port was unable to
     *  receive packets due to insufficient bandwidth to
     *  one of the PP internal resources, such as the DRAM
     *  or buffer allocation.
     */
    GT_U64 dropEvents;

    /** Number of undersize packets received. */
    GT_U64 undersizePkts;

    /** Number of fragments received. */
    GT_U64 fragmentsPkts;

    /** Number of oversize packets received. */
    GT_U64 oversizePkts;

    /** Number of jabber packets received. */
    GT_U64 jabberPkts;

    /** @brief Number of Rx Error events seen by the receive side
     *  of the MAC
     */
    GT_U64 macRcvError;

    /** Number of CRC error events. */
    GT_U64 badCrc;

    /** @brief Total number of collisions seen by the MAC.
     *  Not supported in CG unit.
     */
    GT_U64 collisions;

    /** @brief Total number of late collisions seen by the MAC
     *  (xCat2: also counts unknown MAC control frames)
     *  Not supported in CG unit.
     */
    GT_U64 lateCollisions;

    /** @brief Number of bad Flow Control frames received.
     *  Not Supported for DxCh and ExMxPm.
     */
    GT_U64 badFcRcv;

    /** @brief Number of Ethernet Unicast frames received that
     *  are not bad Ethernet frames or MAC Control packets.
     *  Note that this number includes Bridge Control
     *  packets such as LCAP and BPDU.
     *  Supported for DxCh and ExMxPm only.
     */
    GT_U64 ucPktsRcv;

    /** @brief Number of good frames sent that had a Unicast
     *  destination MAC Address.
     *  Supported for DxCh and ExMxPm only.
     */
    GT_U64 ucPktsSent;

    /** @brief Valid Frame transmitted on half
     *  encountered more then one collision.
     *  Byte count and cast are valid.
     *  Supported for DxCh and ExMxPm only.
     */
    GT_U64 multiplePktsSent;

    /** @brief Valid frame transmitted on half
     *  no collisions, but where the frame transmission
     *  was delayed due to media being busy. Byte count
     *  and cast are valid.
     *  Supported for DxCh and ExMxPm only.
     *  (xCat2: the behavior of this counter is determined by
     *  oversized packets counter mode.
     *  See: cpssDxChMacOversizedPacketsCounterModeSet.)
     */
    GT_U64 deferredPktsSent;

    /** @brief the number of received good and bad frames that
     *  are 1024 to 1518 bytes in size
     *  Relevant only for Interlaken channel MIB counter
     *  (APPLICABLE DEVICES: Bobcat2)
     */
    GT_U64 pkts1024to1518Octets;

    /** @brief the number of received good and bad frames that
     *  are more than 1519 bytes in size
     *  Relevant only for Interlaken channel MIB counter
     *  (APPLICABLE DEVICES: Bobcat2)
     */
    GT_U64 pkts1519toMaxOctets;

} CPSS_PORT_MAC_COUNTER_SET_STC;

/**
* @struct CPSS_PORT_MAC_CG_COUNTER_SET_STC
 *
 * @brief A struct represent the CG unit MAC counters.
 * The struct includes fields by the names of MIB counters, each
 * field represents the respective MIB counter.
*/
typedef struct{

    /** @brief All octets transmitted except preamble
     *  (i.e. Header, Payload, Padding and FCS)
     *  for all valid frames and valid pause frames transmitted.
     */
    GT_U64 ifOutOctets;

    /** @brief Incremented with each frame written to the FIFO interface
     *  and bit 0 of the destination address set to '1' but not
     *  the broadcast address (all bits set to '1'). (good only)
     */
    GT_U64 ifOutMulticastPkts;

    /** @brief Number of frames transmitted with error:
     *  FIFO Overflow Error (if FIFO has overflow protection)
     *  FIFO Underflow Error
     *  User application defined error (ff_tx_err asserted together with ff_tx_eop)
     */
    GT_U64 ifOutErrors;

    /** @brief Incremented with each frame written to the FIFO interface
     *  and bit 0 of the destination address set to '0'. (good only)
     */
    GT_U64 ifOutUcastPkts;

    /** @brief Incremented with each frame written to the FIFO interface
     *  and all bits of the destination address set to '1'. (good only)
     */
    GT_U64 ifOutBroadcastPkts;

    /** @brief All octets received except preamble (i.e. Header, Payload, Padding and FCS)
     *  for all valid frames and valid pause frames received.
     */
    GT_U64 ifInOctets;

    /** @brief Number of frames received with error:
     *  FIFO Overflow Error
     *  CRC Error
     *  Frame Too Long Error
     *  Alignment Error
     *  The dedicated Error Code (0xfe, not a code error) was received
     */
    GT_U64 ifInErrors;

    /** @brief Incremented with each valid frame received on the Receive FIFO
     *  interface and all bits of the destination address were set to '1'.
     */
    GT_U64 ifInBroadcastPkts;

    /** @brief Incremented with each valid frame received on the Receive FIFO
     *  interface and bit 0 of the destination address was '1' but not
     *  the broadcast address (all bits set to '1'). Pause frames are not counted.
     */
    GT_U64 ifInMulticastPkts;

    /** @brief Incremented with each valid frame received on the Receive FIFO
     *  interface and bit 0 of the destination address was '0'.
     */
    GT_U64 ifInUcastPkts;

    /** Frame transmitted without error (includes generated pause and PFC frames). */
    GT_U64 aFramesTransmittedOK;

    /** Frame received without error (including pause frames). */
    GT_U64 aFramesReceivedOK;

    /** CRC */
    GT_U64 aFrameCheckSequenceErrors;

    /** @brief Valid control frame transmitted. Increments for application frames
     *  with type 0x8808 and either an opcode other than 0x0001 when pfc_mode = 0,
     *  or an opcode other than 0x0101 when pfc_mode = 1.
     *  The counter does not increment for internally generated Pause and CBFC frames.
     */
    GT_U64 aMACControlFramesTransmitted;

    /** @brief Valid control frame received (type 0x8808, and either opcode != 0001
     *  when pfc_mode = 0, or opcode != 0101 when pfc_mode = 1).
     */
    GT_U64 aMACControlFramesReceived;

    /** Valid pause/PFC frame transmitted (only internally generated frames). */
    GT_U64 aPAUSEMACCtrlFramesTransmitted;

    /** Valid pause frame received. */
    GT_U64 aPAUSEMACCtrlFramesReceived;

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 0. */
    GT_U64 aCBFCPAUSEFramesTransmitted_0;

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 1. */
    GT_U64 aCBFCPAUSEFramesTransmitted_1;

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 2. */
    GT_U64 aCBFCPAUSEFramesTransmitted_2;

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 3. */
    GT_U64 aCBFCPAUSEFramesTransmitted_3;

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 4. */
    GT_U64 aCBFCPAUSEFramesTransmitted_4;

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 5. */
    GT_U64 aCBFCPAUSEFramesTransmitted_5;

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 6. */
    GT_U64 aCBFCPAUSEFramesTransmitted_6;

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 7. */
    GT_U64 aCBFCPAUSEFramesTransmitted_7;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 0. */
    GT_U64 aCBFCPAUSEFramesReceived_0;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 1. */
    GT_U64 aCBFCPAUSEFramesReceived_1;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 2. */
    GT_U64 aCBFCPAUSEFramesReceived_2;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 3. */
    GT_U64 aCBFCPAUSEFramesReceived_3;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 4. */
    GT_U64 aCBFCPAUSEFramesReceived_4;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 5. */
    GT_U64 aCBFCPAUSEFramesReceived_5;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 6. */
    GT_U64 aCBFCPAUSEFramesReceived_6;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 7. */
    GT_U64 aCBFCPAUSEFramesReceived_7;

    /** Total number of octets received. Good and bad packets. */
    GT_U64 etherStatsOctets;

    /** @brief Total number of packets that were less than 64 octets long with a good CRC.
     *  Note: Undersize packets are not delivered to the FIFO interface.
     */
    GT_U64 etherStatsUndersizePkts;

    /** @brief Total number of packets longer than the valid maximum length programmed
     *  in register FRM_LENGTH (excluding framing bits, but including FCS octets),
     *  and with a good Frame Check Sequence.
     */
    GT_U64 etherStatsOversizePkts;

    /** @brief Total number of packets longer than the valid maximum length programmed
     *  in register FRM_LENGTH (excluding framing bits, but including FCS octets),
     *  and with a bad Frame Check Sequence.
     */
    GT_U64 etherStatsJabbers;

    /** @brief Counts the number of dropped packets due to internal errors of the MAC
     *  Client. Occurs when a Receive FIFO overflow condition persists.
     */
    GT_U64 etherStatsDropEvents;

    /** @brief Total number of packets that were less than 64 octets long with a wrong CRC.
     *  Note: Fragments are not delivered to the FIFO interface.
     */
    GT_U64 etherStatsFragments;

    /** Total number of packets received. Good and bad packets. */
    GT_U64 etherStatsPkts;

    /** @brief Incremented when a packet of 64 octets length is received
     *  (good and bad frames are counted).
     */
    GT_U64 etherStatsPkts64Octets;

    /** Frames (good and bad) with 65 to 127 octets. */
    GT_U64 etherStatsPkts65to127Octets;

    /** Frames (good and bad) with 128 to 255 octets. */
    GT_U64 etherStatsPkts128to255Octets;

    /** Frames (good and bad) with 256 to 511 octets. */
    GT_U64 etherStatsPkts256to511Octets;

    /** Frames (good and bad) with 512 to 1023 octets. */
    GT_U64 etherStatsPkts512to1023Octets;

    /** Frames (good and bad) with 1024 to 1518 octets. */
    GT_U64 etherStatsPkts1024to1518Octets;

    /** @brief Proprietary RMON extension counter that counts the number
     *  of frames with 1519 bytes to the maximum length programmed
     *  in register FRM_LENGTH.
     */
    GT_U64 etherStatsPkts1519toMaxOctets;

    /** Frame received with an alignment error. */
    GT_U64 aAlignmentErrors;

    /** Frame received exceeded the maximum length programmed in register FRM_LENGTH. */
    GT_U64 aFrameTooLongErrors;

    /** @brief A count of frames with a length/type field value between
     *  46 and less than 0x0600, that does not match the number of
     *  payload data octets received.
     */
    GT_U64 aInRangeLengthErrors;

    /** VLAN frame transmitted without error. */
    GT_U64 VLANTransmittedOK;

    /** VLAN frame received without error. */
    GT_U64 VLANRecievedOK;

} CPSS_PORT_MAC_CG_COUNTER_SET_STC;

/**
* @enum CPSS_PORT_MAC_CG_COUNTER_ENT
 *
 * @brief Enumeration of the ethernet CG MAC Counters.
*/
typedef enum{

    /** @brief All octets transmitted except preamble
     *  (i.e. Header, Payload, Padding and FCS)
     *  for all valid frames and valid pause frames transmitted.
     */
    CPSS_CG_IF_OUT_OCTETS_E,

    /** @brief Incremented with each frame written to the FIFO interface
     *  and bit 0 of the destination address set to '1' but not
     *  the broadcast address (all bits set to '1'). (good only)
     */
    CPSS_CG_IF_OUT_MC_PKTS_E,

    /** @brief Number of frames transmitted with error:
     *  FIFO Overflow Error (if FIFO has overflow protection)
     *  FIFO Underflow Error
     *  User application defined error (ff_tx_err asserted together with ff_tx_eop)
     */
    CPSS_CG_IF_OUT_ERRORS_E,

    /** @brief Incremented with each frame written to the FIFO interface
     *  and bit 0 of the destination address set to '0'. (good only)
     */
    CPSS_CG_IF_OUT_UC_PKTS_E,

    /** @brief Incremented with each frame written to the FIFO interface
     *  and all bits of the destination address set to '1'. (good only)
     */
    CPSS_CG_IF_OUT_BC_PKTS_E,

    /** @brief All octets received except preamble (i.e. Header, Payload, Padding and FCS)
     *  for all valid frames and valid pause frames received.
     */
    CPSS_CG_IF_IN_OCTETS_E,

    /** @brief Number of frames received with error:
     *  FIFO Overflow Error
     *  CRC Error
     *  Frame Too Long Error
     *  Alignment Error
     *  The dedicated Error Code (0xfe, not a code error) was received
     */
    CPSS_CG_IF_IN_ERRORS_E,

    /** @brief Incremented with each valid frame received on the Receive FIFO
     *  interface and all bits of the destination address were set to '1'.
     */
    CPSS_CG_IF_IN_BC_PKTS_E,

    /** @brief Incremented with each valid frame received on the Receive FIFO
     *  interface and bit 0 of the destination address was '1' but not
     *  the broadcast address (all bits set to '1'). Pause frames are not counted.
     */
    CPSS_CG_IF_IN_MC_PKTS_E,

    /** @brief Incremented with each valid frame received on the Receive FIFO
     *  interface and bit 0 of the destination address was '0'.
     */
    CPSS_CG_IF_IN_UC_PKTS_E,

    /** Frame transmitted without error (includes generated pause and PFC frames). */
    CPSS_CG_A_FRAMES_TRANSMITTED_OK_E,

    /** Frame received without error (including pause frames). */
    CPSS_CG_A_FRAMES_RECIEVED_OK_E,

    /** CRC-32 Error is detected but the frame is otherwise of correct length. */
    CPSS_CG_A_FRAME_CHECK_SEQUENCE_ERRORS_E,

    /** @brief Valid control frame transmitted. Increments for application frames
     *  with type 0x8808 and either an opcode other than 0x0001 when pfc_mode = 0,
     *  or an opcode other than 0x0101 when pfc_mode = 1.
     *  The counter does not increment for internally generated Pause and CBFC frames.
     */
    CPSS_CG_A_MAC_CONTROL_FRAMES_TRANSMITTED_E,

    /** @brief Valid control frame received (type 0x8808, and either opcode != 0001
     *  when pfc_mode = 0, or opcode != 0101 when pfc_mode = 1).
     */
    CPSS_CG_A_MAC_CONTROL_FRAMES_RECIEVED_E,

    /** Valid pause/PFC frame transmitted (only internally generated frames). */
    CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E,

    /** Valid pause frame received. */
    CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E,

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 0. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_0_E,

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 1. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_1_E,

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 2. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_2_E,

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 3. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_3_E,

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 4. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_4_E,

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 5. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_5_E,

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 6. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_6_E,

    /** Number of CBFC (Class Based Flow Control) pause frames transmitted for class 7. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_7_E,

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 0. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_0_E,

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 1. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_1_E,

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 2. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_2_E,

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 3. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_3_E,

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 4. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_4_E,

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 5. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_5_E,

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 6. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_6_E,

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 7. */
    CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_7_E,

    /** Total number of octets received. Good and bad packets. */
    CPSS_CG_ETHER_STATS_OCTETS_E,

    /** @brief Total number of packets that were less than 64 octets long with a good CRC.
     *  Note: Undersize packets are not delivered to the FIFO interface.
     */
    CPSS_CG_ETHER_STATS_UNDERSIZE_PKTS_E,

    /** @brief Total number of packets longer than the valid maximum length programmed
     *  in register FRM_LENGTH (excluding framing bits, but including FCS octets),
     *  and with a good Frame Check Sequence.
     */
    CPSS_CG_ETHER_STATS_OVERSIZE_PKTS_E,

    /** @brief Total number of packets longer than the valid maximum length programmed
     *  in register FRM_LENGTH (excluding framing bits, but including FCS octets),
     *  and with a bad Frame Check Sequence.
     */
    CPSS_CG_ETHER_STATS_JABBERS_E,

    /** @brief Counts the number of dropped packets due to internal errors of the MAC
     *  Client. Occurs when a Receive FIFO overflow condition persists.
     */
    CPSS_CG_ETHER_STATS_DROP_EVENTS_E,

    /** @brief Total number of packets that were less than 64 octets long with a wrong CRC.
     *  Note: Fragments are not delivered to the FIFO interface.
     */
    CPSS_CG_ETHER_STATS_FRAGMENTS_E,

    /** Total number of packets received. Good and bad packets. */
    CPSS_CG_ETHER_STATS_PKTS_E,

    /** @brief Incremented when a packet of 64 octets length is received
     *  (good and bad frames are counted).
     */
    CPSS_CG_ETHER_STATS_PTKS_64OCTETS_E,

    /** Frames (good and bad) with 65 to 127 octets. */
    CPSS_CG_ETHER_STATS_PTKS_65TO127_OCTETS_E,

    /** Frames (good and bad) with 128 to 255 octets. */
    CPSS_CG_ETHER_STATS_PTKS_128TO255_OCTETS_E,

    /** Frames (good and bad) with 256 to 511 octets. */
    CPSS_CG_ETHER_STATS_PTKS_256TO511_OCTETS_E,

    /** Frames (good and bad) with 512 to 1023 octets. */
    CPSS_CG_ETHER_STATS_PTKS_512TO1023_OCTETS_E,

    /** Frames (good and bad) with 1024 to 1518 octets. */
    CPSS_CG_ETHER_STATS_PTKS_1024TO1518_OCTETS_E,

    /** @brief Proprietary RMON extension counter that counts the number
     *  of frames with 1519 bytes to the maximum length programmed
     *  in register FRM_LENGTH.
     */
    CPSS_CG_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E,

    /** Frame received with an alignment error. */
    CPSS_CG_A_ALIGNMENT_ERRORS_E,

    /** Frame received exceeded the maximum length programmed in register FRM_LENGTH. */
    CPSS_CG_A_FRAME_TOO_LONG_ERRORS_E,

    /** @brief A count of frames with a length/type field value between
     *  46 and less than 0x0600, that does not match the number of
     *  payload data octets received.
     */
    CPSS_CG_A_IN_RANGE_LENGTH_ERRORS_E,

    /** VLAN frame transmitted without error. */
    CPSS_CG_VLAN_TRANSMITED_OK_E,

    /** VLAN frame received without error. */
    CPSS_CG_VLAN_RECIEVED_OK_E,

    CPSS_CG_LAST_E

} CPSS_PORT_MAC_CG_COUNTER_ENT;

/**
* @struct CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC
 *
 * @brief A struct represent the CG unit MAC counters.
 * The struct includes fields by the names of MIB counters, each
 * field represents the respective MIB counter.
*/
typedef struct{

    /** @brief Total number of octets received. Good and bad packets. */
    GT_U64 etherStatsOctets;

    /** @brief All octets received except preamble (i.e. Header, Payload, Padding and FCS)
     *  for all valid frames and valid pause frames received.
     */
    GT_U64 ifInOctets;

    /** @brief Frame received with an alignment error. */
    GT_U64 aAlignmentErrors;

    /** @brief Valid control frame received (type 0x8808, and either opcode != 0001
     *  when pfc_mode = 0, or opcode != 0101 when pfc_mode = 1).
     */
    GT_U64 aMACControlFramesReceived;

    /** @brief Frame received exceeded the maximum length programmed in register FRM_LENGTH. */
    GT_U64 aFrameTooLongErrors;

    /** @brief A count of frames with a length/type field value between
     *  46 and less than 0x0600, that does not match the number of
     *  payload data octets received.
     */
    GT_U64 aInRangeLengthErrors;

    /** @brief Frame received without error (including pause frames). */
    GT_U64 aFramesReceivedOK;

    /** @brief CRC */
    GT_U64 aFrameCheckSequenceErrors;

    /** @brief VLAN frame received without error. */
    GT_U64 VLANRecievedOK;

    /** @brief Number of frames received with error:
     *  FIFO Overflow Error
     *  CRC Error
     *  Frame Too Long Error
     *  Alignment Error
     *  The dedicated Error Code (0xfe, not a code error) was received
     */
    GT_U64 ifInErrors;

    /** @brief Incremented with each valid frame received on the Receive FIFO
     *  interface and bit 0 of the destination address was '0'.
     */
    GT_U64 ifInUcastPkts;

    /** @brief Incremented with each valid frame received on the Receive FIFO
     *  interface and bit 0 of the destination address was '1' but not
     *  the broadcast address (all bits set to '1'). Pause frames are not counted.
     */
    GT_U64 ifInMulticastPkts;

    /** @brief Incremented with each valid frame received on the Receive FIFO
     *  interface and all bits of the destination address were set to '1'.
     */
    GT_U64 ifInBroadcastPkts;

    /** @brief Counts the number of dropped packets due to internal errors of the MAC
     *  Client. Occurs when a Receive FIFO overflow condition persists.
     */
    GT_U64 etherStatsDropEvents;

    /** @brief Total number of packets received. Good and bad packets. */
    GT_U64 etherStatsPkts;

    /** @brief Total number of packets that were less than 64 octets long with a good CRC.
     *  Note: Undersize packets are not delivered to the FIFO interface.
     */
    GT_U64 etherStatsUndersizePkts;

    /** @brief Incremented when a packet of 64 octets length is received
     *  (good and bad frames are counted).
     */
    GT_U64 etherStatsPkts64Octets;

    /** @brief Frames (good and bad) with 65 to 127 octets. */
    GT_U64 etherStatsPkts65to127Octets;

    /** @brief Frames (good and bad) with 128 to 255 octets. */
    GT_U64 etherStatsPkts128to255Octets;

    /** @brief Frames (good and bad) with 256 to 511 octets. */
    GT_U64 etherStatsPkts256to511Octets;

    /** @brief Frames (good and bad) with 512 to 1023 octets. */
    GT_U64 etherStatsPkts512to1023Octets;

    /** @brief Frames (good and bad) with 1024 to 1518 octets. */
    GT_U64 etherStatsPkts1024to1518Octets;

    /** @brief Proprietary RMON extension counter that counts the number
     *  of frames with 1519 bytes to the maximum length programmed
     *  in register FRM_LENGTH.
     */
    GT_U64 etherStatsPkts1519toMaxOctets;

    /** @brief Total number of packets longer than the valid maximum length programmed
     *  in register FRM_LENGTH (excluding framing bits, but including FCS octets),
     *  and with a good Frame Check Sequence.
     */
    GT_U64 etherStatsOversizePkts;

    /** @brief Total number of packets longer than the valid maximum length programmed
     *  in register FRM_LENGTH (excluding framing bits, but including FCS octets),
     *  and with a bad Frame Check Sequence.
     */
    GT_U64 etherStatsJabbers;


    /** @brief Total number of packets that were less than 64 octets long with a wrong CRC.
     *  Note: Fragments are not delivered to the FIFO interface.
     */
    GT_U64 etherStatsFragments;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 0. */
    GT_U64 aCBFCPAUSEFramesReceived_0;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 1. */
    GT_U64 aCBFCPAUSEFramesReceived_1;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 2. */
    GT_U64 aCBFCPAUSEFramesReceived_2;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 3. */
    GT_U64 aCBFCPAUSEFramesReceived_3;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 4. */
    GT_U64 aCBFCPAUSEFramesReceived_4;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 5. */
    GT_U64 aCBFCPAUSEFramesReceived_5;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 6. */
    GT_U64 aCBFCPAUSEFramesReceived_6;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 7. */
    GT_U64 aCBFCPAUSEFramesReceived_7;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 8. */
    GT_U64 aCBFCPAUSEFramesReceived_8;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 9. */
    GT_U64 aCBFCPAUSEFramesReceived_9;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 10. */
    GT_U64 aCBFCPAUSEFramesReceived_10;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 11. */
    GT_U64 aCBFCPAUSEFramesReceived_11;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 12. */
    GT_U64 aCBFCPAUSEFramesReceived_12;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 13. */
    GT_U64 aCBFCPAUSEFramesReceived_13;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 14. */
    GT_U64 aCBFCPAUSEFramesReceived_14;

    /** Number of CBFC (Class Based Flow Control) pause frames received for class 15. */
    GT_U64 aCBFCPAUSEFramesReceived_15;

    /** @brief Valid pause frame received. */
    GT_U64 aPAUSEMACCtrlFramesReceived;

} CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC;

/**
* @struct CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC
 *
 * @brief A struct represent the MTI TX MAC counters.
 * The struct includes fields by the names of MIB counters, each
 * field represents the respective MIB counter.
*/
typedef struct{

    /** @brief All octets transmitted except preamble
     *  (i.e. Header, Payload, Padding and FCS)
     *  for all valid frames and valid pause frames transmitted.
     */
    GT_U64 ifOutOctets;

    /** @brief All good octets transmitted
     */
    GT_U64 octetsTransmittedOk;

    /** @brief Valid pause/PFC frame transmitted (only internally generated frames). */
    GT_U64 aPAUSEMACCtrlFramesTransmitted;

    /** @brief Frame transmitted without error (includes generated pause and PFC frames). */
    GT_U64 aFramesTransmittedOK;

    /** @brief VLAN frame transmitted without error. */
    GT_U64 VLANTransmittedOK;

    /** @brief Number of frames transmitted with error:
     *  FIFO Overflow Error (if FIFO has overflow protection)
     *  FIFO Underflow Error
     *  User application defined error (ff_tx_err asserted together with ff_tx_eop)
     */
    GT_U64 ifOutErrors;

    /** @brief Incremented with each frame written to the FIFO interface
     *  and bit 0 of the destination address set to '0'. (good only)
     */
    GT_U64 ifOutUcastPkts;

    /** @brief Incremented with each frame written to the FIFO interface
     *  and bit 0 of the destination address set to '1' but not
     *  the broadcast address (all bits set to '1'). (good only)
     */
    GT_U64 ifOutMulticastPkts;

    /** @brief Incremented with each frame written to the FIFO interface
     *  and all bits of the destination address set to '1'. (good only)
     */
    GT_U64 ifOutBroadcastPkts;

    /** @brief Incremented when a packet of 64 octets length is transmitted
     *  (good and bad frames are counted).
     */
    GT_U64 etherStatsPkts64Octets;

    /** @brief Frames (good and bad) with 65 to 127 octets. */
    GT_U64 etherStatsPkts65to127Octets;

    /** @brief Frames (good and bad) with 128 to 255 octets. */
    GT_U64 etherStatsPkts128to255Octets;

    /** @brief Frames (good and bad) with 256 to 511 octets. */
    GT_U64 etherStatsPkts256to511Octets;

    /** @brief Frames (good and bad) with 512 to 1023 octets. */
    GT_U64 etherStatsPkts512to1023Octets;

    /** @brief Frames (good and bad) with 1024 to 1518 octets. */
    GT_U64 etherStatsPkts1024to1518Octets;

    /** @brief Proprietary RMON extension counter that counts the number
     *  of frames with 1519 bytes to the maximum length programmed
     *  in register FRM_LENGTH.
     */
    GT_U64 etherStatsPkts1519toMaxOctets;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 0. */
    GT_U64 aCBFCPAUSEFramesTransmitted_0;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 1. */
    GT_U64 aCBFCPAUSEFramesTransmitted_1;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 2. */
    GT_U64 aCBFCPAUSEFramesTransmitted_2;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 3. */
    GT_U64 aCBFCPAUSEFramesTransmitted_3;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 4. */
    GT_U64 aCBFCPAUSEFramesTransmitted_4;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 5. */
    GT_U64 aCBFCPAUSEFramesTransmitted_5;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 6. */
    GT_U64 aCBFCPAUSEFramesTransmitted_6;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 7. */
    GT_U64 aCBFCPAUSEFramesTransmitted_7;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 8. */
    GT_U64 aCBFCPAUSEFramesTransmitted_8;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 9. */
    GT_U64 aCBFCPAUSEFramesTransmitted_9;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 10. */
    GT_U64 aCBFCPAUSEFramesTransmitted_10;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 11. */
    GT_U64 aCBFCPAUSEFramesTransmitted_11;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 12. */
    GT_U64 aCBFCPAUSEFramesTransmitted_12;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 13. */
    GT_U64 aCBFCPAUSEFramesTransmitted_13;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 14. */
    GT_U64 aCBFCPAUSEFramesTransmitted_14;

    /** @brief Number of CBFC (Class Based Flow Control) pause frames transmitted for class 15. */
    GT_U64 aCBFCPAUSEFramesTransmitted_15;

    /** @brief Valid control frame transmitted (type 0x8808, and either opcode != 0001
     *  when pfc_mode = 0, or opcode != 0101 when pfc_mode = 1).
     */
    GT_U64 aMACControlFramesTransmitted;

    /** @brief Total number of packets transmitted. Good and bad packets. */
    GT_U64 etherStatsPkts;

} CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC;

/**
* @struct CPSS_PORT_MAC_MTI_COUNTER_SET_STC
 *
 * @brief A struct represent the MTI unit MAC counters.
 * The struct includes fields by the names of MIB counters, each
 * field represents the respective MIB counter.
*/
typedef struct{

    /** @brief MTI RX MIB structure
     */
    CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC rxCounterStc;

    /** @brief MTI TX MIB structure
     */
    CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC txCounterStc;

} CPSS_PORT_MAC_MTI_COUNTER_SET_STC;


/**
* @enum CPSS_PORT_MAC_MTI_RX_COUNTER_ENT
 *
 * @brief Enumeration of the ethernet MTI MAC RX Counters.
*/
typedef enum{

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_OCTETS_E = 0,

    /** @brief
     */
    CPSS_MTI_RX_OCTETS_RECEIVED_OK_E = 1,

    /** @brief
     */
    CPSS_MTI_RX_A_ALIGNMENT_ERRORS_E = 2,

    /** @brief
     */
    CPSS_MTI_RX_A_MAC_CONTROL_FRAMES_RECIEVED_E = 3,

    /** @brief
     */
    CPSS_MTI_RX_A_FRAME_TOO_LONG_ERRORS_E = 4,

    /** @brief
     */
    CPSS_MTI_RX_A_IN_RANGE_LENGTH_ERRORS_E = 5,

    /** @brief
     */
    CPSS_MTI_RX_A_FRAMES_RECIEVED_OK_E = 6,

    /** @brief
     */
    CPSS_MTI_RX_A_FRAME_CHECK_SEQUENCE_ERRORS_E = 7,

    /** @brief
     */
    CPSS_MTI_RX_VLAN_RECIEVED_OK_E = 8,

    /** @brief
     */
    CPSS_MTI_RX_IF_IN_ERRORS_E = 9,

    /** @brief
     */
    CPSS_MTI_RX_IF_IN_UC_PKTS_E = 10,

    /** @brief
     */
    CPSS_MTI_RX_IF_IN_MC_PKTS_E = 11,

    /** @brief
     */
    CPSS_MTI_RX_IF_IN_BC_PKTS_E = 12,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_DROP_EVENTS_E = 13,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_PKTS_E = 14,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_UNDERSIZE_PKTS_E = 15,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_PTKS_64OCTETS_E = 16,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_PTKS_65TO127_OCTETS_E = 17,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_PTKS_128TO255_OCTETS_E = 18,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_PTKS_256TO511_OCTETS_E = 19,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_PTKS_512TO1023_OCTETS_E = 20,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_PTKS_1024TO1518_OCTETS_E = 21,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E = 22,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_OVERSIZE_PKTS_E = 23,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_JABBERS_E = 24,

    /** @brief
     */
    CPSS_MTI_RX_ETHER_STATS_FRAGMENTS_E = 25,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_0_E = 26,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_1_E = 27,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_2_E = 28,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_3_E = 29,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_4_E = 30,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_5_E = 31,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_6_E = 32,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_7_E = 33,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_8_E = 34,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_9_E = 35,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_10_E = 36,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_11_E = 37,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_12_E = 38,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_13_E = 39,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_14_E = 40,

    /** @brief
     */
    CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_15_E = 41,

    /** @brief
     */
    CPSS_MTI_RX_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E = 42,

    CPSS_MTI_RX_LAST_E

} CPSS_PORT_MAC_MTI_RX_COUNTER_ENT;


/**
* @enum CPSS_PORT_MAC_MTI_TX_COUNTER_ENT
 *
 * @brief Enumeration of the ethernet MTI MAC TX Counters.
*/
typedef enum{

    /** @brief
     */
    CPSS_MTI_TX_ETHER_STATS_OCTETS_E = 0,

    /** @brief
     */
    CPSS_MTI_TX_OCTETS_TRANSMITTED_OK_E = 1,

    /** @brief
     */
    CPSS_MTI_TX_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E = 2,

    /** @brief
     */
    CPSS_MTI_TX_A_FRAMES_TRANSMITTED_OK_E = 3,

    /** @brief
     */
    CPSS_MTI_TX_VLAN_RECIEVED_OK_E = 4,

    /** @brief
     */
    CPSS_MTI_TX_IF_OUT_ERRORS_E = 5,

    /** @brief
     */
    CPSS_MTI_TX_IF_OUT_UC_PKTS_E = 6,

    /** @brief
     */
    CPSS_MTI_TX_IF_OUT_MC_PKTS_E = 7,

    /** @brief
     */
    CPSS_MTI_TX_IF_OUT_BC_PKTS_E = 8,

    /** @brief
     */
    CPSS_MTI_TX_ETHER_STATS_PTKS_64OCTETS_E = 9,

    /** @brief
     */
    CPSS_MTI_TX_ETHER_STATS_PTKS_65TO127_OCTETS_E = 10,

    /** @brief
     */
    CPSS_MTI_TX_ETHER_STATS_PTKS_128TO255_OCTETS_E = 11,

    /** @brief
     */
    CPSS_MTI_TX_ETHER_STATS_PTKS_256TO511_OCTETS_E = 12,

    /** @brief
     */
    CPSS_MTI_TX_ETHER_STATS_PTKS_512TO1023_OCTETS_E = 13,

    /** @brief
     */
    CPSS_MTI_TX_ETHER_STATS_PTKS_1024TO1518_OCTETS_E = 14,

    /** @brief
     */
    CPSS_MTI_TX_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E = 15,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_0_E = 16,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_1_E = 17,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_2_E = 18,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_3_E = 19,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_4_E = 20,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_5_E = 21,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_6_E = 22,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_7_E = 23,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_8_E = 24,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_9_E = 25,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_10_E = 26,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_11_E = 27,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_12_E = 28,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_13_E = 29,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_14_E = 30,

    /** @brief
     */
    CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_15_E = 31,

    /** @brief
     */
    CPSS_MTI_TX_A_MAC_CONTROL_FRAMES_TRANSMITTED_E = 32,

    /** @brief
     */
    CPSS_MTI_TX_ETHER_STATS_PKTS_E = 33,

    CPSS_MTI_TX_LAST_E

} CPSS_PORT_MAC_MTI_TX_COUNTER_ENT;


/**
* @enum CPSS_PORT_EGRESS_CNT_MODE_ENT
 *
 * @brief Enumeration for egress counters mode bitmap
*/
typedef enum{

    /** @brief Port bit,
     *  If clear all ports
     *  If set - specific port
     */
    CPSS_EGRESS_CNT_PORT_E = 1,

    /** @brief VLAN bit
     *  If clear all vlans
     *  If set - specific vlan
     */
    CPSS_EGRESS_CNT_VLAN_E = 2,

    /** @brief Traffic class bit
     *  If clear all priorities
     *  If set - specific user priority
     */
    CPSS_EGRESS_CNT_TC_E   = 4,

    /** @brief Drop precedence bit,
     *  If clear all DP levels,
     *  If set - specific DP level
     */
    CPSS_EGRESS_CNT_DP_E   = 8

} CPSS_PORT_EGRESS_CNT_MODE_ENT;

/**
* @struct CPSS_PORT_EGRESS_CNTR_STC
 *
 * @brief Structure of egress counters.
*/
typedef struct{

    /** Number of unicast packets transmitted. */
    GT_U32 outUcFrames;

    /** @brief Number of multicast packets transmitted. This includes
     *  registered multicasts, unregistered multicasts and
     *  unknown unicast packets.
     */
    GT_U32 outMcFrames;

    /** Number of broadcast packets transmitted. */
    GT_U32 outBcFrames;

    /** @brief Number of IN packets that were Bridge Egress filtered.
     *  Counts regardless of port, priority or DP configured in
     *  counter mode.
     */
    GT_U32 brgEgrFilterDisc;

    /** @brief Number of IN packets that were filtered due to TxQ
     *  congestion.
     */
    GT_U32 txqFilterDisc;

    /** @brief ExMxPm and DxCh devices:
     *  Number of out control packets (to cpu, from cpu and to
     *  analyzer).
     */
    GT_U32 outCtrlFrames;

    /** @brief CH2 and above: Number of packets dropped due to
     *  egress forwarding restrictions.
     */
    GT_U32 egrFrwDropFrames;

    /** @brief counts number of packets dropped due to full TXQ
     *  multicast FIFO 3..0. Multicast packets discarded due to
     *  Congestion in the L2 Replication engine. The discarded
     *  descriptors are counted before L2 replications
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2)
     */
    GT_U32 mcFifo3_0DropPkts;

    /** @brief counts number of packets dropped due to full TXQ
     *  multicast FIFO 7..4. Multicast packets discarded due to
     *  Congestion in the L2 Replication engine. The discarded
     *  descriptors are counted before L2 replications
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2)
     */
    GT_U32 mcFifo7_4DropPkts;

    /** @brief counts number of packets dropped due to full any TXQ
     *  multicast FIFO. Multicast packets discarded due to
     *  Congestion in the EFT_QAG Replication engine. The discarded
     *  descriptors are counted before EFT_QAG replications.
     *  (APPLICABLE DEVICES: Falcon)
     */
    GT_U32 mcFifoDropPkts;

    /** @brief Counts the number of packets dropped due to Multicast Filtering.
     *  (APPLICABLE DEVICES: Falcon)
     */
    GT_U32 mcFilterDropPkts;

} CPSS_PORT_EGRESS_CNTR_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPortStath */


