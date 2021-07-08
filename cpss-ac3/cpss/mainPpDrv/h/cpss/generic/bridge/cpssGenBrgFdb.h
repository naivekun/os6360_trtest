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
* @file cpssGenBrgFdb.h
*
* @brief definitions for Multiple Forwarding Databases for 802.1Q Transparent
* devices.
*
* @version   23
********************************************************************************
*/

#ifndef __cpssGenBrgFdbh
#define __cpssGenBrgFdbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/generic/cpssCommonDefs.h>

/**
* @enum CPSS_FDB_AGE_TRUNK_MODE_ENT
 *
 * @brief Enumeration of FDB aging mode for trunk entries.
*/
typedef enum{

    /** @brief the PP when doing trigger/auto aging
     *  can age trunk entry regardless to the associated device
     *  number.
     */
    CPSS_FDB_AGE_TRUNK_ENTRIES_ALL_E,

    /** @brief the PP when doing trigger/auto aging can age trunk entry ,
     *  only if the associated device number is "own devNum"
     */
    CPSS_FDB_AGE_TRUNK_ENTRIES_ASSOCIATE_ONLY_WITH_OWN_DEVNUM_E

} CPSS_FDB_AGE_TRUNK_MODE_ENT;

/**
* @enum CPSS_MAC_TABLE_CMD_ENT
 *
 * @brief Enumeration of MAC Table entry actions taken when a packet's
 * MAC address (DA/SA) matches this entry
*/
typedef enum{

    /** forward (if address is automatically learned) */
    CPSS_MAC_TABLE_FRWRD_E = 0,

    /** drop (filtering on destination/source address) */
    CPSS_MAC_TABLE_DROP_E,

    /** @brief intervention to CPU (may be dropped by other
     *  mechanisms)
     */
    CPSS_MAC_TABLE_INTERV_E,

    /** control (unconditionally trap to CPU) */
    CPSS_MAC_TABLE_CNTL_E,

    /** @brief mirror to the CPU (in addition to sending
     *  the packet to its destination) not
     *  supported in ExMx device
     */
    CPSS_MAC_TABLE_MIRROR_TO_CPU_E,

    /** @brief soft drop (does not prevent the packet from
     *  being sent to the CPU) supported in
     *  DxCh devices
     */
    CPSS_MAC_TABLE_SOFT_DROP_E

} CPSS_MAC_TABLE_CMD_ENT;


/**
* @struct CPSS_FDB_HASH_OPTION_STC
 *
 * @brief FDB Hash Options parameters
*/
typedef struct{

    /** @brief the MAC lookup mask indicates the MAC address
     *  bits that are used in the MAC lookup table.
     *  Default PP HW value of macLookupMasc is 0xFFFFFFFFFFFF
     *  (all bits are used).
     *  Note: The Application SW should support MAC address
     *  Network byte-order (big-endian format). Therefore,
     *  the element with index [0] will contain the most
     *  significant byte of MAC lookup mask, and the element
     *  with index [5] will contain the least significant
     *  byte of MAC lookup mask.
     */
    GT_ETHERADDR macLookupMask;

    /** @brief the VID lookup mask indicates the VLAN ID bits that are
     *  used in the MAC lookup table.
     *  Default PP HW value of vidLookupMask is 0xFFF
     *  (all bits are used).
     */
    GT_U16 vidLookupMask;

    /** @brief indicates the cyclic shift left done on the MAC address
     *  to determine which bits are to be the most valuable in
     *  lookup and learning (in the bytes).
     *  Due to the 6 bytes size of MAC address, the range of this
     *  parameter is [0:5].
     *  Default PP HW value of macShiftLeft is 0 bytes.
     */
    GT_U32 macShiftLeft;

    /** @brief indicates the cyclic shift left done on the VID to
     *  determine which bits are to be the most valuable in
     *  lookup and learning (in the nibbles).
     *  Due to the 3 nibbles size of VLAN ID, the range of this
     *  parameter is [0:2].
     *  Default PP HW value of vidShiftLeft is 0 nibbles.
     */
    GT_U32 vidShiftLeft;

} CPSS_FDB_HASH_OPTION_STC;

/**
* @enum CPSS_MAC_VL_ENT
 *
 * @brief Enumeration of VLAN Learning modes
*/
typedef enum{

    /** Independent VLAN Learning */
    CPSS_IVL_E = 1,

    /** Shared VLAN Learning */
    CPSS_SVL_E

} CPSS_MAC_VL_ENT;

/**
* @enum CPSS_ADDR_LOOKUP_MODE_ENT
 *
 * @brief Enumeration of Address lookup modes
*/
typedef enum{

    /** @brief Optimized for sequential MAC addresses and
     *  sequential VLAN id's.
     */
    CPSS_MAC_SQN_VLAN_SQN_E = 0,

    /** @brief Optimized for random MAC addresses and
     *  sequential VLAN id's.
     */
    CPSS_MAC_RND_VLAN_SQN_E,

    /** @brief Optimized for sequential MAC addresses and
     *  random VLAN id's. Reserved.
     */
    CPSS_MAC_SQN_VLAN_RND_E,

    /** @brief Optimized for random MAC addresses and
     *  random VLAN id's. Reserved.
     */
    CPSS_MAC_RND_VLAN_RND_E

} CPSS_ADDR_LOOKUP_MODE_ENT;

/**
* @enum CPSS_MAC_ACTION_MODE_ENT
 *
 * @brief Enumeration of MAC address table action mode
*/
typedef enum{

    /** Action is done automatically */
    CPSS_ACT_AUTO_E = 0,

    /** Action is done via trigger from CPU */
    CPSS_ACT_TRIG_E

} CPSS_MAC_ACTION_MODE_ENT;


/**
* @enum CPSS_MAC_AU2CPU_TYPE_ENT
 *
 * @brief Enumeration of MAC address update message sent to the CPU.
*/
typedef enum{

    /** @brief Security address messages due to VLAN
     *  learning disabled.
     */
    CPSS_MAC_AU_MESSAGE_SA_LEARNING_E = 0,

    /** Other security address messages. */
    CPSS_MAC_AU_MESSAGE_SA_SECURITY_E,

    /** Transplanting and aged address messages. */
    CPSS_MAC_AU_MESSAGE_TA_AA_E,

    /** Enable Query address to CPU */
    CPSS_MAC_AU_MESSAGE_QA_E,

    /** All types of TA, AA, QA , and SA messages. */
    CPSS_MAC_AU_MESSAGE_ALL_E,

    /** @brief All types of TA, AA, and SA messages
     *  exclude QA and NA - DXSAL Only
     */
    CPSS_MAC_AU_MESSAGE_ALL_EXCLUDE_QA_NA_E,

    /** @brief When enabled EXMX device is send QA to the
     *  other EXMX devices (via Uplink) , and
     *  optionally to the CPU on every packet with
     *  unknown DA !
     *  To disable the CPU from getting those
     *  messages , disable this feature or the
     *  CPSS_MAC_AU_MESSAGE_QA_E.
     *  NOTE:
     *  By default - feature is enabled.
     */
    CPSS_MAC_AU_MESSAGE_QA_FROM_DEVICE_TO_OTHER_DEVICES_AND_TO_CPU_E

} CPSS_MAC_AU2CPU_TYPE_ENT;

/**
* @enum CPSS_MAC_FLT_CMD_ENT
 *
 * @brief Enumeration of MAC Filter Commands taken when a packet's
 * MAC address (DA/SA) matches specified range
*/
typedef enum{

    /** Forward */
    CPSS_MAC_FLT_FRWRD_E = 1,

    /** Drop */
    CPSS_MAC_FLT_DROP_E,

    /** Control (unconditionally trap to CPU) */
    CPSS_MAC_FLT_CNTL_E,

    /** Forward and mirror to the CPU */
    CPSS_MAC_FLT_FRWRD_MIRROR_TO_CPU_E

} CPSS_MAC_FLT_CMD_ENT;


/**
* @enum CPSS_MAC_ENTRY_STATUS_ENT
 *
 * @brief Enumeration of MAC Table entry status
*/
typedef enum{

    /** The value of the corresponding instance was learned */
    CPSS_LRND_E = 0,

    /** @brief The value of the corresponding instance was created by
     *  CPU like static.
     */
    CPSS_MGMT_E,

    /** @brief The value of the corresponding instance was created by
     *  CPU like NOT static.
     */
    CPSS_NOT_STATIC_E

} CPSS_MAC_ENTRY_STATUS_ENT;

/**
* @enum CPSS_PORT_LOCK_CMD_ENT
 *
 * @brief Enum for Locked port forward commands
*/
typedef enum{

    /** Do not drop (forward as usual). */
    CPSS_LOCK_FRWRD_E = 1,

    /** Drop (hard drop). */
    CPSS_LOCK_DROP_E,

    /** Trap to CPU (Intervention). */
    CPSS_LOCK_TRAP_E,

    /** Forward and Mirror to CPU. Applicable to Dx devices */
    CPSS_LOCK_MIRROR_E,

    /** @brief Soft Drop(can still get the CPU).
     *  Applicable to DxCh devices.
     */
    CPSS_LOCK_SOFT_DROP_E

} CPSS_PORT_LOCK_CMD_ENT;

/**
* @enum CPSS_LEARNING_MODE_ENT
 *
 * @brief Enum for Learning modes
*/
typedef enum{

    /** Automatic. */
    CPSS_AUTOMATIC_E        = 0,

    /** Using message default */
    CPSS_CONTROLED_MSSG_E   = 1,

    /** Using DSA tag . Applicable to Dx devices. */
    CPSS_CONTROLED_TAG_E    = 2

} CPSS_LEARNING_MODE_ENT;

/**
* @struct CPSS_MAC_ENTRY_STC
 *
 * @brief MAC Address Entry
*/
typedef struct{

    /** MAC address */
    GT_ETHERADDR macAddr;

    /** Destination interface : port/trunk/vidx */
    CPSS_INTERFACE_INFO_STC dstInterface;

    /** static/dynamic entry */
    GT_BOOL isStatic;

    /** Vlan Id */
    GT_U16 vlanId;

    /** Traffic class assigned to a packet with this source MAC */
    GT_U8 srcTc;

    /** Traffic class assigned to a packet with this destination MAC */
    GT_U8 dstTc;

    /** Action taken when a packet's DA matches this entry */
    CPSS_MAC_TABLE_CMD_ENT daCommand;

    /** Action taken when a packet's SA matches this entry */
    CPSS_MAC_TABLE_CMD_ENT saCommand;

    /** @brief GT_TRUE and packet's SA matches this entry,
     *  send packet to the Multi-Field Classifier
     */
    GT_BOOL saClass;

    /** @brief GT_TRUE, and packet's DA matches this entry,
     *  send packet to the Multi-Field Classifier
     */
    GT_BOOL daClass;

    /** @brief GT_TRUE, and packet's SA matches this entry,
     *  send packet to Customer Interface Bus
     */
    GT_BOOL saCib;

    /** @brief GT_TRUE, and packet's DA matches this entry,
     *  send packet to Customer Interface Bus
     */
    GT_BOOL daCib;

    /** @brief GT_TRUE, and packet's DA matches this entry,
     *  send packet to the IPv4 or MPLS
     */
    GT_BOOL daRoute;

    /** @brief If the packet SA or DA lookup matches the FDB entry,
     *  and this field is set to GT_TRUE, the packet is assigned COS
     *  parameters according to assignment made by the CoS library.
     *  Applicable to DxSal devices
     */
    GT_BOOL cosIpv4En;

    /** @brief GT_TRUE, and packet's DA matches this entry,
     *  mirror packet to a configurable analyzer port.
     */
    GT_BOOL mirrorToRxAnalyzerPortEn;

} CPSS_MAC_ENTRY_STC;


/**
* @enum CPSS_UPD_MSG_TYPE_ENT
 *
 * @brief Enumeration of MAC Table update messages that can be sent
 * between Prestera and CPU.
*/
typedef enum{

    /** @brief new address (NA) message.
     *  message from PP (to CPU)
     *  in auto learn : meaning that the PP learned new source
     *  address(or 'Station movement').
     *  in controlled learn : meaning the PP notify
     *  the CPU about a new source address(or 'Station movement').
     *  NOTE: for ExMx devices , in controlled learning the device send
     *  SA and not NA messages
     *  message from CPU (to PP) - CPU 'ask' PP to add/update FDB entry.
     *  this message can be used to 'skip' entry.
     *  NOTE:
     *  1. operation may fail due to bucket full.
     *  2. in ExMxPm devices operation will fail if bucket and TCAM
     *  are full , or when working in external FDB and the LUT
     *  (lookup table) entry is full.
     */
    CPSS_NA_E = 0,

    /** @brief query address (QA) message
     *  message from CPU (to PP) when the CPU wants to get full FDB entry
     *  info about an entry identified by the 'Mac+vid' (key of entry).
     *  NOTE: the PP will reply with the info in QR (Query reply) message.
     */
    CPSS_QA_E,

    /** @brief query reply (QR)
     *  message from PP (to CPU) the PP sends info about a query send
     *  from CPU. the query was one of : QA / QI
     *  reply to QA - the PP will return the full FDB entry , and indication
     *  weather entry was found.
     *  reply to QI - the PP will return the Index of the FDB entry, and
     *  indication weather entry was found.
     *  NOTE:
     *  1. in DxCh,ExMxPm devices the Index is offset in the bucket
     *  and not index in the FDB table. (CPU needs to calculate
     *  the index of the start of bucket)
     *  2. in DxCh,ExMxPm devices the QR always holds all the information:
     *  full FDB entry and the index (offset in the bucket)
     */
    CPSS_QR_E,

    /** @brief aged address (AA) message
     *  message from PP (to CPU)
     *  in age with removal : meaning the PP aged out an entry.
     *  in age without removal : meaning the PP notify the CPU about
     *  entry that should be aged out.
     *  NOTE: the PP will send those messages also in the process of
     *  'FDB flush' , or 'FDB triggered aging' for entries that need
     *  to be aged out.
     *  message from CPU (to PP) - CPU force the PP to invalidate an FDB entry
     */
    CPSS_AA_E,

    /** @brief transplanted address (TA) message.
     *  message from PP (to CPU) the PP in the process of 'FDB transplanting'
     *  notify the CPU that an entry transplanted to associate with
     *  new destination.
     */
    CPSS_TA_E,

    /** @brief security address message
     *  message from PP (to CPU)
     *  in controlled learn : meaning the PP notify
     *  the CPU about a new source address(or 'Station movement').
     *  NOTE:
     *  1. In auto learning the device send NA and not SA messages
     *  2. applicable devices : All ExMx devices.
     */
    CPSS_SA_E,

    /** @brief query address index message
     *  message from CPU (to PP) When the CPU wants to get index of FDB
     *  entry , info about an entry identified by the 'Mac+vid'
     *  (key of entry).
     *  NOTE:
     *  1. the PP will reply with the info in QR (Query reply) message.
     *  2. applicable devices : All ExMx devices.
     */
    CPSS_QI_E,

    /** @brief FDB Upload (FU) message.
     *  message from PP (to CPU) the PP notify the CPU about valid and
     *  non skipped FDB entries.
     *  NOTE:
     *  1. this done in the process of 'FDB upload triggering' .
     *  2. applicable devices : DxCh2 and above devices.
     *  applicable devices : All ExMxPm devices.
     */
    CPSS_FU_E,

    /** @brief Hash Request (HR) message.
     *  message that is sent from the CPU to the PP , when FDB hash results
     *  that are calculated by the PP needed for the 'search key'.
     *  see function cpssDxChBrgFdbHashRequestSend.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  CPSS_NA_MOVED_E - CPU to PP message to move 'valid' entry from <origFdbIndex>
     *  in the FDB table to 'new index'.
     *  see function cpssDxChBrgFdbMacEntryMove.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_HR_E

} CPSS_UPD_MSG_TYPE_ENT;



/**
* @struct CPSS_MAC_UPDATE_MSG_STC
 *
 * @brief MAC Table Update message exchanged between the Prestera
 * device and CPU.
*/
typedef struct{

    /** update message type (NA, QA, QR, AA, TA, SA, QI) */
    CPSS_UPD_MSG_TYPE_ENT updType;

    /** entry found or not flag (relevant only ot QR/QI msg to CPU) */
    GT_BOOL entryWasFound;

    /** @brief the entry index in the MAC table in which the MAC address
     *  resides
     *  relevant only for QI msg & entryFound = 1)
     */
    GT_U32 macEntryIndex;

    /** FDB entry format */
    CPSS_MAC_ENTRY_STC macEntry;

    /** @brief the skip bit
     *  GT_FALSE - entry used by HW
     */
    GT_BOOL skip;

    /** @brief the age bit
     *  aging cycle since last traffic on the entry)
     *  GT_FALSE - the entry has "aged" by 1 aging cycle
     */
    GT_BOOL aging;

    /** @brief the device num that entry associated with
     *  NOTE: this value is valid for all type of entries:
     *  unicast on port / unicast on trunk /using Vidx
     */
    GT_U8 associatedDevNum;

    /** @brief The Device
     *  Relevant ONLY for QA/QR messages between Prestera Packet
     *  Processors.
     */
    GT_U8 queryDevNum;

    /** @brief GT_FALSE
     *  GT_TRUE - Message is from/to CPU.
     *  NOTE: CPU must set this bit to '1'.
     */
    GT_BOOL msgCpu;

} CPSS_MAC_UPDATE_MSG_STC;

/**
* @enum CPSS_BRG_IPM_MODE_ENT
 *
 * @brief This enum defines IPM Bridging mode
*/
typedef enum{

    /** @brief This mode is used for Source-Specific
     *  Multicast (SSM) snooping. The FDB lookup
     *  is based on the packet source IP (SIP),
     *  group destination IP (DIP), and VLAN-ID
     */
    CPSS_BRG_IPM_SGV_E,

    /** @brief This mode is used for Any-Source Multicast
     *  (ASM) snooping. The FDB lookup is based on
     *  the packet group destination IP (DIP),
     *  and VLAN-ID
     */
    CPSS_BRG_IPM_GV_E

} CPSS_BRG_IPM_MODE_ENT;

/**
* @enum CPSS_MAC_ENTRY_EXT_TYPE_ENT
 *
 * @brief This enum defines the type of the FDB Entry
*/
typedef enum{

    /** @brief The entry is MAC Address
     *  entry (hushed by Mac Address and vlan ID).
     */
    CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E = 0,

    /** @brief The entry is IPv4 Multicast
     *  entry (IGMP Snooping).
     */
    CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E,

    /** @brief The entry is IPv6 Multicast
     *  entry (MLD Snooping).
     */
    CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E,

    /** @brief The entry is IPv4 Unicast entry.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E,

    /** @brief The entry is IPv6 Unicast address entry
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E,

    /** @brief The entry is IPv6 Unicast data entry
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E,

    /** @brief The entry is MAC Address entry
     *  (hashed by Mac Address, FID and VID1).
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E,

    /** @brief The entry is IPv4 Multicast
     *  entry. Hashed by SIP, DIP, FID, and VID1.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E,

    /** @brief The entry is IPv6 Multicast
     *  entry. Hashed by SIP, DIP, FID, and VID1.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E

} CPSS_MAC_ENTRY_EXT_TYPE_ENT;


/**
* @struct CPSS_MAC_ENTRY_EXT_KEY_STC
 *
 * @brief Key parameters of the MAC Address Entry
*/
typedef struct
{
    CPSS_MAC_ENTRY_EXT_TYPE_ENT         entryType;
    GT_U32                              vid1;
    union {
       struct{
           GT_ETHERADDR     macAddr;
           GT_U16           vlanId;
       }macVlan;

       struct{
           GT_U8            sip[4];
           GT_U8            dip[4];
           GT_U16           vlanId;
       }ipMcast;

       struct{
           GT_U8            dip[4];
           GT_U32           vrfId;
       }ipv4Unicast;

       struct{
           GT_U8            dip[16];
           GT_U32           vrfId;
       }ipv6Unicast;

    } key;

}CPSS_MAC_ENTRY_EXT_KEY_STC;

/**
* @enum CPSS_FDB_UC_ROUTING_TYPE_ENT
*
* @brief This enum defines the location on the Next Hop or
*        Multipath (ECMP/QOS) information
*   (APPLICABLE DEVICES: Falcon)
*/
typedef enum{

    /** @brief The Next Hop information is located in the FDB entry in
     *  the Router
     */
    CPSS_FDB_UC_ROUTING_TYPE_NH_FDB_E,

    /** @brief The Multipath (ECMP or QOS) information is located in
     * ECMP/QOS Table in the Router
     */
    CPSS_FDB_UC_ROUTING_TYPE_MULTIPATH_ROUTER_E

} CPSS_FDB_UC_ROUTING_TYPE_ENT;

/**
* @struct CPSS_FDB_UC_ROUTING_INFO_STC
 *
 * @brief Routing Address Entry
*/
typedef struct{

    /** @brief Enable TTL/Hop Limit Decrement */
    GT_BOOL ttlHopLimitDecEnable;

    /** @brief Enable TTL/HopLimit Decrement and option/Extention
     *         check bypass.
     */
    GT_BOOL ttlHopLimDecOptionsExtChkByPass;

    /** @brief mirror to ingress analyzer.
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL ingressMirror;

    /** @brief One of seven possible analyzers. Relevant
     *  when ingressMirror is GT_TRUE.
     *  If a previous engine in the pipe assigned a different
     *  analyzer index, the higher index wins.
     *  (APPLICABLE RANGES: 0..6)
     *  qosProfileMarkingEnable- Enable Qos profile assignment.
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    GT_U32 ingressMirrorToAnalyzerIndex;

    /** @brief Enable the remarking of the QoS Profile assigned
     *  to the packet.
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL qosProfileMarkingEnable;

    /** @brief The qos profile index.
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    GT_U32 qosProfileIndex;

    /** @brief whether this packet Qos parameters can be overridden
     *  after this assingment.
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;

    /** @brief whether to change the packets UP and how.
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyUp;

    /** @brief whether to change the packets DSCP and how.
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyDscp;

    /** @brief he counter set this route entry is linked to */
    CPSS_IP_CNT_SET_ENT countSet;

    /** @brief enable Trap/Mirror ARP Broadcasts with DIP matching
     *  this entry
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL trapMirrorArpBcEnable;

    /** @brief The security level associated with the DIP.
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    GT_U32 dipAccessLevel;

    /** @brief Enable performing ICMP Redirect Exception
     *         Mirroring. */
    GT_BOOL ICMPRedirectEnable;

    /** @brief One of the eight global configurable MTU sizes
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 mtuProfileIndex;

    /** @brief weather this nexthop is tunnel start enrty, in which
     *  case the outInteface & mac are irrlevant and the tunnel
     *  id is used.
     *  only if isTunnelStart = GT_FALSE then dstInterface in
     *  CPSS_MAC_ENTRY_EXT_STC is relevant
     *  isTunnelStart = GT_TRUE
     */
    GT_BOOL isTunnelStart;

    /** @brief the output vlan id (used also for SIP RPF check, and
     *  ICMP check)
     */
    GT_U16 nextHopVlanId;

    /** @brief The ARP Pointer indicating the routed packet MAC DA,
     *  relevant only if the isTunnelStart = GT_FALSE
     */
    GT_U32 nextHopARPPointer;

    /** @brief the tunnel pointer in case this is a tunnel start */
    GT_U32 nextHopTunnelPointer;

    /** @brief The bank number of the associated IPv6
     *  NOTE: Relevant only for entries of type CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E
     * (NOT APPLICABLE DEVICES: Falcon)
     */
    GT_U32 nextHopDataBankNumber;

    /** @brief Enable IPv6 Scope Checking. */
    GT_BOOL scopeCheckingEnable;

    /** @brief The site id of this route entry. */
    CPSS_IP_SITE_ID_ENT siteId;

    /** @brief The ECMP/QOS or NextHop Routing Type
     *  (APPLICABLE DEVICES: Falcon)
     *  only if routingType==CPSS_FDB_UC_ROUTING_TYPE_NH_FDB_E then
     *  dstInterface in CPSS_MAC_ENTRY_EXT_STC is relevant.
     *  If routingType==CPSS_FDB_UC_ROUTING_TYPE_MULTIPATH_ROUTER_E
     *  then all above fields are not relevant.
     */
    CPSS_FDB_UC_ROUTING_TYPE_ENT routingType;

    /** @brief The multipathPointer is the index of the ECMP or QOS Entry in
     *  the ECMP/QOS Table in case routingType==CPSS_FDB_UC_ROUTING_TYPE_MULTIPATH_ROUTER_E
     *  Otherwise - multipathPointer is ignored
     *  (APPLICABLE DEVICES: Falcon)
     */ 
    GT_U32 multipathPointer;

} CPSS_FDB_UC_ROUTING_INFO_STC;

/**
* @struct CPSS_MAC_ENTRY_EXT_STC
 *
 * @brief Extension to MAC Address Entry
*/
typedef struct{

    /** @brief key data, depends on the type of the MAC entry
     *  MAC entry -> mac address + vlan ID
     *  IP Multicast entry -> srcIP + dstIP + vlan ID
     *  IPv4 UC entry -> dstIP + prefixLen + VRF_ID
     *  IPv6 UC Full entry -> dstIP + prefixLen + VRF_ID
     */
    CPSS_MAC_ENTRY_EXT_KEY_STC key;

    /** @brief destination interface : port/trunk/vidx
     *  For IPv4 and IPv6 multicast entries only VLAN or
     *  VIDX is used.
     *  Relevant for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST/IPV4_UC/IPV6_UC
     *  for key of type IPV4_UC/IPV6_UC this field is the next hop interface
     */
    CPSS_INTERFACE_INFO_STC dstInterface;

    /** @brief Age flag that is used for the two
     *  GT_FALSE - The entry will be aged out in the next pass.
     *  GT_TRUE - The entry will be aged-out in two age-passes.
     *  Notes: used only for DxCh devices.
     *  IP UC entries are not aged out, but only marked as such
     *  Relevant for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST/IPV4_UC/IPV6_UC
     */
    GT_BOOL age;

    /** @brief static/dynamic entry
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    GT_BOOL isStatic;

    /** @brief action taken when a packet's DA matches this entry
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    CPSS_MAC_TABLE_CMD_ENT daCommand;

    /** @brief action taken when a packet's SA matches this entry.
     *  For IPv4 and IPv6 multicast entries these field is not used,
     *  it is ignored.
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    CPSS_MAC_TABLE_CMD_ENT saCommand;

    /** @brief GT_TRUE, and packet's DA matches this entry,
     *  send packet to the IPv4 or MPLS
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    GT_BOOL daRoute;

    /** @brief GT_TRUE, and packet's DA matches this entry,
     *  mirror packet to a configurable analyzer port.
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    GT_BOOL mirrorToRxAnalyzerPortEn;

    /** @brief Source ID. For IPv4 and IPv6 multicast entries these field
     *  is not used, it is ignored.
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    GT_U32 sourceID;

    /** @brief user defined field [0..0xF].
     *  for Lion2 devices: if the use of <MyCoreId> Field In Fdb
     *  Entry is enabled then udb0 is used for <myCoreId> and the
     *  number of bits of udb field is decreased by 1
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    GT_U32 userDefined;

    /** @brief Qos attribute set applied to the packet if there is a
     *  destination lookup mode. If both FDB lookups find a matching
     *  entry, whose attribute index is not NULL, a global
     *  FDB QOS Marking Conflict Resolution command selects one
     *  of the Qos attributes source or destination [0..0x7].
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    GT_U32 daQosIndex;

    /** @brief Qos attribute set applied to the packet if there is a
     *  source lookup mode. If both FDB lookups find a matching
     *  entry, whose attribute index is not NULL, a global
     *  FDB QOS Marking Conflict Resolution command selects one
     *  of the Qos attributes source or destination [0..0x7].
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     *  Cheetah2 fields:
     */
    GT_U32 saQosIndex;

    /** @brief security level assigned to the MAC DA that matches
     *  this entry [0..0x7].
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    GT_U32 daSecurityLevel;

    /** @brief security level assigned to the MAC SA that matches
     *  this entry [0..0x7].
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    GT_U32 saSecurityLevel;

    /** @brief If set, and the entry <DA command> is TRAP or MIRROR
     *  then the CPU code may be overwritten according
     *  to the Application Specific CPU Code assignment
     *  mechanism.
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    GT_BOOL appSpecificCpuCode;

    /** @brief Relevant when auto
     *  New Address (NA) storm prevention is enabled
     *  GT_FALSE - Regular Entry;
     *  GT_TRUE - Storm Prevention Entry;
     *  This is a storm prevention entry indicating
     *  that a NA message has been sent to the CPU
     *  but the CPU has not yet learned this MAC Address
     *  on its current location. The device does not send
     *  further NA messages to the CPU for this source
     *  MAC Address. Should a MAC DA lookup match this entry,
     *  it is treated as an unknown Unicast packet.
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     */
    GT_BOOL spUnknown;

    /** @brief GT_TRUE, and packet's SA matches this entry,
     *  mirror packet to a configurable analyzer port.
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL saMirrorToRxAnalyzerPortEn;

    /** @brief GT_TRUE, and packet's DA matches this entry,
     *  mirror packet to a configurable analyzer port.
     *  Relevant only for key of type MAC_ADDR/IPV4_MCAST/IPV6_MCAST
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL daMirrorToRxAnalyzerPortEn;

    /** @brief All information needed for FDB Routing
     *  Relevant only for key of type IPV4_UC/IPV6_UC
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_FDB_UC_ROUTING_INFO_STC fdbRoutingInfo;

} CPSS_MAC_ENTRY_EXT_STC;


/**
* @struct CPSS_MAC_UPDATE_MSG_EXT_STC
 *
 * @brief MAC Table Update message exchanged between the Prestera
 * device and CPU.
*/
typedef struct{

    /** update message type (NA, QA, QR, AA, TA, SA, QI) */
    CPSS_UPD_MSG_TYPE_ENT updType;

    /** entry found or not flag (relevant only to QR/QI msg to CPU) */
    GT_BOOL entryWasFound;

    /** @brief the entry index in the MAC table in which the MAC address
     *  resides
     *  relevant only for QI msg & entryFound = 1)
     */
    GT_U32 macEntryIndex;

    /** FDB entry format for MAC Address and IP Multicast entries */
    CPSS_MAC_ENTRY_EXT_STC macEntry;

    /** @brief the skip bit
     *  GT_FALSE - entry used by HW
     */
    GT_BOOL skip;

    /** @brief the age bit
     *  aging cycle since last traffic on the entry)
     *  GT_FALSE - the entry has "aged" by 1 aging cycle
     */
    GT_BOOL aging;

    /** @brief the HW device num that entry associated with
     *  NOTE: this value is valid for all type of entries:
     *  unicast on port / unicast on trunk /using Vidx
     */
    GT_HW_DEV_NUM associatedHwDevNum;

    /** @brief The HW Device
     *  Relevant ONLY for QA/QR messages between Prestera Packet
     *  Processors.
     */
    GT_HW_DEV_NUM queryHwDevNum;

    /** @brief indicates that NA message contains a MAC Address
     *  that cannot be retained because the chain is too long.
     *  GT_TRUE - the length of the FDB chain is too long
     *  GT_FALSE - the length of the FDB chain is not too long
     */
    GT_BOOL naChainIsTooLong;

    /** @brief The FDB entry offset relative to the hash index for
     *  this address. This is valid in the following cases:
     *  1. NA messages, when naChainIsTooLong = 0.
     *  2. QR messages, when entryFound = 1.
     */
    GT_U32 entryOffset;

    /** @brief port group Id.indicating the port group from which the AU/FU message
     *  came.
     *  relevant ONLY to multi-port groups device.
     */
    GT_U32 portGroupId;

    /** @brief Relevant when message type is NA.
     *  VLAN Id of Tag1 [0..0xFFF], assigned by the device's
     *  ingress pipe VLAN assignment mechanisms.
     *  APPLICABLE DEVICES: xCat2
     */
    GT_U16 vid1;

    /** @brief Relevant when message type is NA.
     *  Tag0 User Priority value [0..7], assigned by the device's
     *  ingress pipe QOS assignment mechanisms.
     *  APPLICABLE DEVICES: xCat2
     */
    GT_U32 up0;

    /** @brief Relevant when message type is NA.
     *  GT_TRUE: this address is a moved one (the MAC address
     *  is associated with new device/port or trunk).
     *  GT_FALSE: MAC address is new one or already exists in FDB
     *  and it's source is not moved.
     *  APPLICABLE DEVICES: xCat2
     */
    GT_BOOL isMoved;

    /** @brief Relevant when isMoved = GT_TRUE.
     *  Source Id of the old address [0..0x1F].
     *  APPLICABLE DEVICES: xCat2
     */
    GT_U32 oldSrcId;

    /** @brief Relevant when isMoved = GT_TRUE.
     *  The associated information with old entry.
     *  Valid interface types: port or trunk.
     *  APPLICABLE DEVICES: xCat2
     */
    CPSS_INTERFACE_INFO_STC oldDstInterface;

    /** @brief Relevant when isMoved = GT_TRUE.
     *  The old HW device num that entry associated with.
     *  APPLICABLE DEVICES: xCat2
     */
    GT_HW_DEV_NUM oldAssociatedHwDevNum;

} CPSS_MAC_UPDATE_MSG_EXT_STC;

/**
* @enum CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT
 *
 * @brief Enum for Secure Automatic Learning modes.
*/
typedef enum{

    /** Disable Secure Automatic learning. */
    CPSS_MAC_SECURE_AUTO_LEARN_DISABLED_E,

    /** @brief enable secure auto learning, trap
     *  to CPU packets with unknown MAC DA Trap to CPU.
     */
    CPSS_MAC_SECURE_AUTO_LEARN_UNK_TRAP_E,

    /** @brief enable secure auto learning,
     *  soft drop packets with unknown MAC DA.
     */
    CPSS_MAC_SECURE_AUTO_LEARN_UNK_SOFT_DROP_E,

    /** @brief enable secure auto learning,
     *  hard drop packets with unknown MAC DA.
     */
    CPSS_MAC_SECURE_AUTO_LEARN_UNK_HARD_DROP_E,

    /** @brief enable secure auto learning,
     *  packet forwarded without Automatic learning.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3)
     */
    CPSS_MAC_SECURE_AUTO_LEARN_UNK_FORWARD_E,

    /** @brief enable secure auto learning,
     *  packet forwarded and mirrored to CPU without
     *  Automatic learning.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_MAC_SECURE_AUTO_LEARN_UNK_MIRROR_TO_CPU_E

} CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT;


/**
* @enum CPSS_MAC_HASH_FUNC_MODE_ENT
 *
 * @brief Enum for FDB Hash function modes.
*/
typedef enum{

    /** @brief XOR based hash function mode,
     *  provides optimal hash index distribution
     *  for controlled testing scenarios, where
     *  sequential addresses and vlans are often used.
     *  (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
     */
    CPSS_MAC_HASH_FUNC_XOR_E,

    /** @brief CRC based hash function mode,
     *  provides the best hash index distribution
     *  for random addresses and vlans.
     *  (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
     */
    CPSS_MAC_HASH_FUNC_CRC_E,

    /** @brief Different CRC hash functions per FDB bank.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E

} CPSS_MAC_HASH_FUNC_MODE_ENT;

/**
* @enum CPSS_FDB_ACTION_MODE_ENT
 *
 * @brief Enum for FDB Action modes.
*/
typedef enum{

    /** @brief Automatic aging with removal of
     *  aged out entries.
     */
    CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E,

    /** @brief Automatic or triggered aging
     *  without removal of aged out
     *  entries or triggered FDB Upload.
     */
    CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E,

    /** Triggered Address deleting */
    CPSS_FDB_ACTION_DELETING_E,

    /** Triggered Address Transplanting */
    CPSS_FDB_ACTION_TRANSPLANTING_E

} CPSS_FDB_ACTION_MODE_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenBrgFdbh */


