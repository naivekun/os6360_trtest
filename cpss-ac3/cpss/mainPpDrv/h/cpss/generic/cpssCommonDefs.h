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
* @file cpssCommonDefs.h
*
* @brief Includes common definitions for all Prestera Sw layers.
*
*
* @version   8
********************************************************************************
*/
#ifndef __cpssCommonDefsh
#define __cpssCommonDefsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @enum CPSS_L4_PROTOCOL_ENT
 *
 * @brief general L4 protocol
 * Enumerators:
 * CPSS_L4_PROTOCOL_TCP_E - TCP
 * CPSS_L4_PROTOCOL_UDP_E - UDP
*/
typedef enum{

    CPSS_L4_PROTOCOL_TCP_E = 6,

    CPSS_L4_PROTOCOL_UDP_E = 17

} CPSS_L4_PROTOCOL_ENT;

/**
* @enum CPSS_L4_PROTOCOL_PORT_TYPE_ENT
 *
 * @brief type of TCP/UDP port
 * Enumerators:
 * CPSS_L4_PROTOCOL_PORT_SRC_E - source port
 * CPSS_L4_PROTOCOL_PORT_DST_E - destination port
*/
typedef enum{

    CPSS_L4_PROTOCOL_PORT_SRC_E,

    CPSS_L4_PROTOCOL_PORT_DST_E

} CPSS_L4_PROTOCOL_PORT_TYPE_ENT;

/**
* @enum CPSS_COMPARE_OPERATOR_ENT
 *
 * @brief compare operator
 * Enumerators:
 * CPSS_COMPARE_OPERATOR_INVALID_E - always generates result "0"
 * CPSS_COMPARE_OPERATOR_LTE   - less or equal
 * CPSS_COMPARE_OPERATOR_GTE   - greater or equal
 * CPSS_COMPARE_OPERATOR_NEQ   - not equal
*/
typedef enum{

    CPSS_COMPARE_OPERATOR_INVALID_E,

    CPSS_COMPARE_OPERATOR_LTE,

    CPSS_COMPARE_OPERATOR_GTE,

    CPSS_COMPARE_OPERATOR_NEQ

} CPSS_COMPARE_OPERATOR_ENT;


/**
* @enum CPSS_PACKET_CMD_ENT
 *
 * @brief This enum defines the packet command.
*/
typedef enum{

    /** forward packet */
    CPSS_PACKET_CMD_FORWARD_E              ,

    /** mirror packet to CPU */
    CPSS_PACKET_CMD_MIRROR_TO_CPU_E        ,

    /** trap packet to CPU */
    CPSS_PACKET_CMD_TRAP_TO_CPU_E          ,

    /** hard drop packet */
    CPSS_PACKET_CMD_DROP_HARD_E            ,

    /** soft drop packet */
    CPSS_PACKET_CMD_DROP_SOFT_E            ,

    /** IP Forward the packets */
    CPSS_PACKET_CMD_ROUTE_E                ,

    /** @brief Packet is routed and mirrored to
     *  the CPU.
     */
    CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E     ,

    /** Bridge and Mirror to CPU. */
    CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E    ,

    /** Bridge only */
    CPSS_PACKET_CMD_BRIDGE_E               ,

    /** Do nothing. (disable) */
    CPSS_PACKET_CMD_NONE_E                 ,

    /** loopback packet is send back to originator */
    CPSS_PACKET_CMD_LOOPBACK_E             ,

    /** same as CPSS_PACKET_CMD_ROUTE_E but packet can be failed by loose uRPF. */
    CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E

} CPSS_PACKET_CMD_ENT;



/**
* @enum CPSS_DROP_MODE_TYPE_ENT
 *
 * @brief Enumeration for drop mode for red packets
*/
typedef enum{

    /** drop mode is Soft drop */
    CPSS_DROP_MODE_SOFT_E = 0,

    /** drop mode is hard drop */
    CPSS_DROP_MODE_HARD_E

} CPSS_DROP_MODE_TYPE_ENT;

/**
* @enum CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT
 *
 * @brief Enumerator for modification of packet's attribute
 * like User Priority and DSCP.
*/
typedef enum{

    /** @brief Keep
     *  previous packet's attribute modification command.
     *  CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,    - disable
     *  modification of the packet's attribute.
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E = 0,

    CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,

    /** @brief enable
     *  modification of the packet's attribute.
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E

} CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT;

/**
* @enum CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT
 *
 * @brief Enumerator for the packet's attribute assignment precedence
 * for the subsequent assignment mechanism.
*/
typedef enum{

    /** @brief Soft precedence:
     *  The packet's attribute assignment can be overridden
     *  by the subsequent assignment mechanism
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E = 0,

    /** @brief Hard precedence:
     *  The packet's attribute assignment is locked
     *  to the last value of attribute assigned to the packet
     *  and cannot be overridden.
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E

} CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT;

/**
* @enum CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT
 *
 * @brief Enumerator for the packet's attribute assignment command.
*/
typedef enum{

    /** @brief packet's attribute assignment
     *  disabled
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,

    /** @brief packet's attribute assignment
     *  only if the packet is VLAN tagged.
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E,

    /** @brief packet's attribute assignment
     *  only if the packet is untagged or Prioritytagged.
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E,

    /** @brief packet's attribute assignment
     *  regardless of packet tagging state.
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E

} CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssCommonDefsh */


