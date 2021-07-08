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
* @file cpssGenBrgGen.h
*
* @brief CPSS Bridge Generic APIs.
*
*
* @version   2
********************************************************************************
*/
#ifndef __cpssGenBrgGenh
#define __cpssGenBrgGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_IGMP_SNOOP_MODE_ENT
 *
 * @brief Enumeration of IGMP snooping trapping/mirroring modes.
*/
typedef enum{

    /** All IGMP packets are trapped to CPU. */
    CPSS_IGMP_ALL_TRAP_MODE_E = 0,

    /** @brief Query messages are mirrored, non-query are
     *  trapped.
     */
    CPSS_IGMP_SNOOP_TRAP_MODE_E,

    /** All IGMP packets are mirrored to CPU. */
    CPSS_IGMP_ROUTER_MIRROR_MODE_E,

} CPSS_IGMP_SNOOP_MODE_ENT;

/**
* @enum CPSS_RATE_LIMIT_MODE_ENT
 *
 * @brief Enumeration of broadcast rate limiting modes.
*/
typedef enum{

    /** count packets */
    CPSS_RATE_LIMIT_PCKT_BASED_E = 1,

    /** count bytes */
    CPSS_RATE_LIMIT_BYTE_BASED_E

} CPSS_RATE_LIMIT_MODE_ENT;

/**
* @enum CPSS_BRIDGE_INGR_CNTR_MODES_ENT
 *
 * @brief Enumeration of four a CPU-configured packet "stream" modes for
 * bridge ingress counters.
*/
typedef enum{

    /** @brief counter set acts on all packets received by
     *  local ports.
     */
    CPSS_BRG_CNT_MODE_0_E = 0,

    /** @brief counter set acts on all packets received on
     *  the specified local source port.
     */
    CPSS_BRG_CNT_MODE_1_E,

    /** @brief counter set acts on all packets received by
     *  local ports, which have a specified VLAN
     *  classification.
     */
    CPSS_BRG_CNT_MODE_2_E,

    /** @brief counter set acts on all packets received by a
     *  counterset specific local port and VLAN
     *  classification.
     */
    CPSS_BRG_CNT_MODE_3_E

} CPSS_BRIDGE_INGR_CNTR_MODES_ENT;

/*
 * typedef: struct CPSS_BRIDGE_INGRESS_CNTR_STC
 *
 * Description: Structure of ingress bridge counters.
 *
 * Enumerations:
 *   gtBrgInFrames          - number of packets receives according to the
 *                            specified Mode criteria. Depending on the Mode
 *                            selected, this counter can be used such as
 *                            dot1dTpPortInFrames (mode 1),
 *                            dot1qTpVlanPortInFrames (mode 3).
 *
 *   gtBrgVlanIngFilterDisc - number of packets discarded due to VLAN
 *                            Ingress Filtering. This counter can be
 *                            used such as dot1qTpVlanPortInDiscard
 *                            (mode 1).
 *
 *   gtBrgSecFilterDisc     - number of packets discarded due to
 *                            Security Filtering measures:
 *                            MAC SA/DA filtering, Locked port, MAC
 *                            Range Filtering, Invalid SA.
 *
 *   gtBrgLocalPropDisc     - number of packets discarded due to reasons other 
 *                            than VLAN ingress and Security filtering:
 *                              - Rate Limiting drop
 *                              - Local port drop (Bridge Local Switching)
 *                              - Spanning Tree state drop
 *                              - IP and Non-IP Multicast filtering
 *                              - Per VLAN Unregistered/Unknown 
 *                                Multicast/Broadcast filtering
 *                            This counter can be used such as
 *                            - dot1dTpPortInDiscards (mode 1)
 *                            - dot1qTpVlanPortInDiscard (mode 3).
 */
typedef struct
{
    GT_U32 gtBrgInFrames;
    GT_U32 gtBrgVlanIngFilterDisc;
    GT_U32 gtBrgSecFilterDisc;
    GT_U32 gtBrgLocalPropDisc;
} CPSS_BRIDGE_INGRESS_CNTR_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenBrgGenh */



