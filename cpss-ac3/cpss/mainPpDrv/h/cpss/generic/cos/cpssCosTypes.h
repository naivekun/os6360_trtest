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
* @file mainPpDrv/h/cpss/generic/cos/cpssCosTypes.h
*
* @brief Includes structures definition of COS module.
*
*
* @version   4
********************************************************************************
*/

#ifndef __cpssCosTypesh
#define __cpssCosTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cos/cpssCosTypes.h>
#include <cpss/generic/cpssCommonDefs.h>

/* Traffic class range limit */
#define CPSS_4_TC_RANGE_CNS   4

/* Extended Traffic class range limit */
#define CPSS_16_TC_RANGE_CNS   16

/* Drop Precedence range limit */
#define CPSS_DP_RANGE_CNS 8

/* Drop Precedence range limit */
#define CPSS_4_DP_RANGE_CNS 4

/**
* @enum CPSS_COS_TABLE_TYPE_ENT
*/
typedef enum{

    /** @brief The COS table used for initial marking of the
     *  packet COS.
     */
    CPSS_COS_TABLE_MARKING_E = 0,

    /** @brief The COS table used for re-marking of the
     *  packet COS by the Traffic Conditioner.
     */
    CPSS_COS_TABLE_REMARKING_E

} CPSS_COS_TABLE_TYPE_ENT;


/**
* @struct CPSS_COS_STC
 *
 * @brief The Prestera Class-of-Service parameters.
*/
typedef struct{

    /** Drop precedence (color), range 0 */
    CPSS_DP_LEVEL_ENT dropPrecedence;

    /** IEEEE 802.1p User Priority, range 0 */
    GT_U8 userPriority;

    /** Egress tx traffic class queue, range 0 */
    GT_U8 trafficClass;

} CPSS_COS_STC;


/**
* @struct CPSS_COS_MAP_STC
 *
 * @brief Structure for Cos Mapping entry
*/
typedef struct{

    /** DiffServ Code point */
    GT_U8 dscp;

    /** Class of service parameters. */
    CPSS_COS_STC cosParam;

    /** @brief Experimental field in MPLS Shim header, used for DiffServ
     *  support of MPLS
     */
    GT_U8 expField;

    /** @brief User defined 15 bits value passed to the CIB for external
     *  processing
     */
    GT_U16 userDef;

} CPSS_COS_MAP_STC;


/**
* @enum CPSS_COS_USER_PRIO_MODE_ENT
 *
 * @brief untagged packet VPT assign mode.
*/
typedef enum{

    /** assign ser priority (VPT) according to ingress packet port. */
    CPSS_COS_USER_PRIO_MODE_USER_PRIORITY_PER_PORT_E = 0,

    /** assign user priority (VPT) according to TC. */
    CPSS_COS_USER_PRIO_MODE_TC_PER_PORT_E,

    /** the MAX value (to use for array sizes) */
    CPSS_COS_USER_PRIO_MODE_MAX_E

} CPSS_COS_USER_PRIO_MODE_ENT;


/**
* @enum CPSS_COS_MAC_ADDR_TYPE_ENT
 *
 * @brief Enumeration for the MAC TC assignment settings.
*/
typedef enum{

    /** No override of TC */
    CPSS_COS_MAC_ADDR_TYPE_NONE_E,

    /** @brief Set TC if match mac entry of both
     *  SA,DA mac entries
     */
    CPSS_COS_MAC_ADDR_TYPE_ALL_E,

    /** @brief Set TC if match mac entry by
     *  SA mac entry
     */
    CPSS_COS_MAC_ADDR_TYPE_SA_ONLY_E,

    /** @brief Set TC if match mac entry by
     *  DA mac entry
     */
    CPSS_COS_MAC_ADDR_TYPE_DA_ONLY_E

} CPSS_COS_MAC_ADDR_TYPE_ENT;


/**
* @enum CPSS_COS_PARAM_ENT
 *
 * @brief enumeration for the Cos parameters
*/
typedef enum{

    /** DSCP parameter */
    CPSS_COS_PARAM_DSCP_E,

    /** EXP parameter */
    CPSS_COS_PARAM_EXP_E,

    /** TC parameter */
    CPSS_COS_PARAM_TC_E,

    /** UP parameter */
    CPSS_COS_PARAM_UP_E,

    /** DP parameter */
    CPSS_COS_PARAM_DP_E

} CPSS_COS_PARAM_ENT;


/**********************************************************************
 * typedef: struct CPSS_QOS_ENTRY_STC
 *
 * Description:
 *      Port QoS Configuration.
 *
 * Enumerations:
 *      qosProfileId     -  The port's QoSProfile. This is the port's default
 *                          QoSProfile and may be assigned to the packet as part
 *                          of the QoS assignment algorithm.
 *      assignPrecedence -  port marking of the QoS Precedence.
 *      enableModifyUp   - mac modification mode of the up filed
 *      enableModifyDscp - mac modification mode of the dscp filed (or EXP for MPLS)
 * COMMENTS:
 ***********************************************************************/
typedef struct
{
    GT_U32                                       qosProfileId;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  assignPrecedence;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT        enableModifyUp;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT        enableModifyDscp;
}CPSS_QOS_ENTRY_STC;


/**
* @enum CPSS_QOS_PORT_TRUST_MODE_ENT
 *
 * @brief Trust modes for the port.
*/
typedef enum{

    /** @brief untrust: packet QosProfile is assigned as the
     *  port's default PortQosProfile.
     */
    CPSS_QOS_PORT_NO_TRUST_E = 0,

    /** @brief if the packet is tagged the Qos Profile is
     *  assigned according to User Priority field, if the packet isn't
     *  tagged the QoS Profile is assigned by default Port's QoS Profile.
     */
    CPSS_QOS_PORT_TRUST_L2_E,

    /** @brief if the packet is IPv4/IPv6 the QoS profile is
     *  assigned to the Dscp field or remapped Dscp, else according to
     *  Port's Default QoS Profile.
     */
    CPSS_QOS_PORT_TRUST_L3_E,

    /** @brief if the packet is IPv4/IPv6 the packet is
     *  assigned like Trust L3, else if the packet is tagged like Trust
     *  L2, else the packet is assigned Port's Default QoS profile.
     */
    CPSS_QOS_PORT_TRUST_L2_L3_E

} CPSS_QOS_PORT_TRUST_MODE_ENT;


/**
* @enum CPSS_MAC_QOS_RESOLVE_ENT
 *
 * @brief MAC QoS Marking Conflict Resolution mode.
*/
typedef enum{

    /** get the QoS attribute index from DA. */
    CPSS_MAC_QOS_GET_FROM_DA_E =0,

    /** get the QoS attribute index from SA */
    CPSS_MAC_QOS_GET_FROM_SA_E

} CPSS_MAC_QOS_RESOLVE_ENT;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssExMxCosTypesh */


