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
* @file cpssGenBrgSecurityBreachTypes.h
* @version   4
********************************************************************************
*/

#ifndef __cpssGenBrgSecurityBreachTypesh
#define __cpssGenBrgSecurityBreachTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_BRG_SECUR_BREACH_EVENTS_ENT
 *
 * @brief Enumeration identifies the type of security breach reported
*/
typedef enum{

    /** @brief FDB entry command Soft or
     *  Hard Drop
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_FDB_ENTRY_E = 1,

    /** @brief port new address is a
     *  security breach
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NA_E,

    /** invalid MAC source address */
    CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,

    /** vlan not valid */
    CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,

    /** port not member in vlan */
    CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,

    /** vlan range drop */
    CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E,

    /** @brief moved static address is a
     *  security breach
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,

    /** @brief ARP header MAC SA and L2
     *  header MAC SA Mismatch
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_ARP_SA_MISMATCH_E,

    /** TCP SYN packets with data */
    CPSS_BRG_SECUR_BREACH_EVENTS_TCP_SYN_E,

    /** @brief TCP over Multicast/Broadcast
     *  packets
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_TCP_OVER_MC_E,

    /** @brief Bridge Access Matrix table
     *  drop configuration
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_BRIDGE_ACCESS_MATRIX_E,

    /** @brief the source MAC address is
     *  NOT found in the FDB
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_SECURE_AUTO_LEARN_E,

    /** @brief Packet that does not match
     *  the Acceptable Frame Type
     *  configuration
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_FRAME_TYPE,

    /** IPv4 fragmented packet */
    CPSS_BRG_SECUR_BREACH_EVENTS_FRAGMENT_ICMP_TYPE_E,

    /** @brief TCP packet with the flags
     *  all set to zero
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_TCP_FLAG_ZERO_E,

    /** @brief TCP packets with the TCP
     *  FIN, URG, and PSH flags
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_TCP_FIN_URG_PSH_E,

    /** @brief TCP packet with the TCP SYN
     *  and FIN flags
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_TCP_SYN_FIN_E,

    /** @brief TCP packet with the TCP SYN
     *  and RST flags
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_TCP_SYN_RST_E,

    /** @brief TCP/UDP packet with
     *  a zero source or destination port
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_TCP_UDP_SRC_DST_PORT_ZERO,

    /** @brief vlan entry new address is a
     *  security breach
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_NA_E,

    /** @brief MAC SA is found
     *  in FDB, but changed it's location
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_AUTO_LEARN_NO_RELEARN_E,

    /** @brief MAC Spoof protection
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,

    /** @brief MAC SA is found in FDB
     *  but changed it's location
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_MOVED_E,

    /** @brief MAC SA is DA
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,

    /** @brief SIP is DIP
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_SIP_IS_DIP_E,

    /** @brief TCP/UDP Source port is destination port
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_TCP_UDP_SPORT_IS_DPORT_E,

    /** @brief TCP flags has FIN but no ACK
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_TCP_FIN_WITHOUT_ACK_E,

    /** @brief TCP has no full header
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_TCP_WITHOUT_FULL_HEADER_E,

    /** @brief Source MAC is 0
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  The CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E is used in the
     *  Security Breach Code for the exception.
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E

} CPSS_BRG_SECUR_BREACH_EVENTS_ENT;

/**
* @struct CPSS_BRG_SECUR_BREACH_MSG_STC
 *
 * @brief Security Breach Message
*/
typedef struct{

    /** Source MAC Address of the breaching packet */
    GT_ETHERADDR macSa;

    /** The VID assigned to the breaching packet */
    GT_U16 vlan;

    /** Source port on which the security breach packet was received */
    GT_PORT_NUM port;

    /** @brief security breach code, that the security breach event was
     *  reported with
     */
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT code;

} CPSS_BRG_SECUR_BREACH_MSG_STC;

/**
* @enum CPSS_BRG_SECUR_BREACH_DROP_COUNT_MODE_ENT
 *
 * @brief Enumeration of security breach drop port/vlan counter mode
*/
typedef enum{

    /** @brief Port counter - counts all
     *  packets received on SecurityBreach DropCntPort
     *  and dropped due to security breach.
     */
    CPSS_BRG_SECUR_BREACH_DROP_COUNT_PORT_E,

    /** @brief Vlan counter - counts all
     *  packets assigned with VID from SecurityBreach
     *  DropCntVID and dropped due to security breach.
     */
    CPSS_BRG_SECUR_BREACH_DROP_COUNT_VLAN_E

} CPSS_BRG_SECUR_BREACH_DROP_COUNT_MODE_ENT;

/**
* @struct CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC
 *
 * @brief Security Breach Port/Vlan counter configuration entry
*/
typedef struct{

    /** security breach port/vlan drop counter port or vlan mode */
    CPSS_BRG_SECUR_BREACH_DROP_COUNT_MODE_ENT dropCntMode;

    /** @brief port number
     *  If the drop count mode is Port, then all packets received
     *  on this port and dropped due to security breach are
     *  counted by the Security Breach Drop counter.
     */
    GT_PHYSICAL_PORT_NUM port;

    /** @brief vlan ID. If the drop counter mode is VID, then all packets
     *  assigned with this VID and dropped due to security breach
     *  are counted by the Security Breach Drop counter.
     */
    GT_U16 vlan;

} CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenBrgSecurityBreachTypesh */



