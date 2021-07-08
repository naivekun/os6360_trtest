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
* @file cpssGenBrgVlanTypes.h
* @version   5
********************************************************************************
*/

#ifndef __cpssGenBrgVlanTypesh
#define __cpssGenBrgVlanTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>


/**
* @enum CPSS_UNREG_MC_EGR_FILTER_CMD_ENT
 *
 * @brief Enumeration of egress filtering modes for
 * Unknown unicast and multicast packets
*/
typedef enum{

    /** forward according to vlan port membership */
    CPSS_UNREG_MC_VLAN_FRWD_E,

    /** @brief forward according to vlan port membership and to CPU
     *  (even if CPU is NOT a member of a VLAN)
     */
    CPSS_UNREG_MC_VLAN_CPU_FRWD_E,

    /** @brief forward to CPU only
     *  (even if CPU is NOT a member of a VLAN)
     */
    CPSS_UNREG_MC_CPU_FRWD_E,

    /** @brief forward according to vlan port membership and NOT to
     *  CPU (even if CPU is a member of a VLAN)
     */
    CPSS_UNREG_MC_NOT_CPU_FRWD_E

} CPSS_UNREG_MC_EGR_FILTER_CMD_ENT;

/**
* @enum CPSS_VLAN_TAG_TYPE_ENT
 *
 * @brief Enumeration of packet Vlan Tags
*/
typedef enum{

    /** 0-th Vlan Tag */
    CPSS_VLAN_TAG0_E,

    /** 1-th Vlan Tag */
    CPSS_VLAN_TAG1_E

} CPSS_VLAN_TAG_TYPE_ENT;


/**
* @struct CPSS_VLAN_INFO_STC
 *
 * @brief A structure to hold generic VLAN entry info.
*/
typedef struct{

    /** @brief GT_FALSE, if VLAN does not have any local member
     *  ports on this device;
     */
    GT_BOOL hasLocals;

    /** @brief GT_FALSE, if VLAN does not have port members
     *  residing on remote device;
     */
    GT_BOOL hasUplinks;

    /** GT_TRUE, if CPU is a member of this VLAN */
    GT_BOOL isCpuMember;

    /** GT_TRUE, if enabled leraning of MAC SA for this VLAN */
    GT_BOOL learnEnable;

    /** @brief CPSS_EXMX_UNREG_MC_VLAN_FRWD_E   0x0
     *  CPSS_EXMX_UNREG_MC_VLAN_CPU_FRWD_E 0x1
     *  CPSS_EXMX_UNREG_MC_CPU_FRWD_E   0x2
     *  CPSS_EXMX_UNREG_MC_NOT_CPU_FRWD_E 0x3
     */
    CPSS_UNREG_MC_EGR_FILTER_CMD_ENT unregMcFilterCmd;

    /** TRUE if SA MAC per VLAN is in use */
    GT_BOOL perVlanSaMacEnabled;

    /** @brief SA MAC suffix in bit 10:5 of VLAN entry.
     *  Relevant only if isSaMacEnabled == TRUE.
     */
    GT_U8 saMacSuffix;

    /** STP group, range 0 */
    GT_U32 stpId;

} CPSS_VLAN_INFO_STC;

/**
* @enum CPSS_ETHER_MODE_ENT
 *
 * @brief Enumeration of VLAN etherType modes
*/
typedef enum{

    /** @brief vlan0 used for EtherType associated with
     *  standard VLAN Tagged Packets
     */
    CPSS_VLAN_ETHERTYPE0_E = 1,

    /** @brief vlan1 used for Nested VLAN configuration and
     *  BackEnd configuration
     */
    CPSS_VLAN_ETHERTYPE1_E

} CPSS_ETHER_MODE_ENT;


/*
 * typedef: struct CPSS_PROT_CLASS_ENCAP_STC
 *
 * Description: Structure of encapsulation formats for Protocol based
 *              VLAN classification
 *
 * Enumerations:
 *    ethV2      -  EthernetV2 encapsulation
 *    nonLlcSnap -  non-SNAP LLC encapsulation
 *    llcSnap    -  LLC/SNAP encapsulation
 *
 * COMMENT: REMOVE
 *          This struct is defined localy but should be removed after it will
 *          be defined in the cpssExMxBrgClass.h
 *
 */
typedef struct
{
    GT_BOOL ethV2;
    GT_BOOL nonLlcSnap;
    GT_BOOL llcSnap;
}CPSS_PROT_CLASS_ENCAP_STC;

/**
* @enum CPSS_STP_STATE_ENT
 *
 * @brief Enumeration of STP Port state
*/
typedef enum{

    /** STP disabled on this port. */
    CPSS_STP_DISABLED_E = 0,

    /** blocking/Listening. */
    CPSS_STP_BLCK_LSTN_E,

    /** learning. */
    CPSS_STP_LRN_E,

    /** forwarding. */
    CPSS_STP_FRWRD_E

} CPSS_STP_STATE_ENT;

/**
* @enum CPSS_PORT_ACCEPT_FRAME_TYPE_ENT
 *
 * @brief Enumeration of port frames discard modes
*/
typedef enum{

    /** @brief All Untagged/Priority Tagged packets received on this
     *  port are discarded. Only Tagged accepted.
     */
    CPSS_PORT_ACCEPT_FRAME_TAGGED_E = 0,

    /** @brief Both Tagged and Untagged packets are accepted on the
     *  port.
     */
    CPSS_PORT_ACCEPT_FRAME_ALL_E,

    /** @brief All Tagged packets received on this port are
     *  discarded. Only Untagged/Priority Tagged packets
     *  accepted.
     */
    CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E,

    /** @brief Both Tagged and Untagged packets are discarded on the
     *  port.
     */
    CPSS_PORT_ACCEPT_FRAME_NONE_E

} CPSS_PORT_ACCEPT_FRAME_TYPE_ENT;

/**
* @enum CPSS_BRG_MODE_ENT
 *
 * @brief This enum defines bridging mode
*/
typedef enum{

    /** 802.1Q Bridge, VLAN-aware mode */
    CPSS_BRG_MODE_802_1Q_E,

    /** 802.1Q Bridge, VLAN-unaware mode */
    CPSS_BRG_MODE_802_1D_E

} CPSS_BRG_MODE_ENT;

/**
* @enum CPSS_BRG_TPID_SIZE_TYPE_ENT
 *
 * @brief Enumeration of packet TPID size type
*/
typedef enum{

    /** TPID size is 4 bytes */
    CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E,

    /** TPID size is 8 bytes */
    CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E,

    /** TPID size is 6 bytes */
    CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E

} CPSS_BRG_TPID_SIZE_TYPE_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __cpssGenBrgVlanTypesh */



