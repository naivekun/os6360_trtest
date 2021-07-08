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
* @file cpssGenPolicerTypes.h
*
* @brief Generic definitions for Traffic Policer.
*
* @version   2
********************************************************************************
*/

#ifndef __cpssGenPolicerTypesh
#define __cpssGenPolicerTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/******************************************************************************/
/**********************   Policer API Enumeration Types  **********************/
/******************************************************************************/

/**
* @enum CPSS_POLICER_PACKET_SIZE_MODE_ENT
 *
 * @brief Enumeration of metering and counting packet size.
*/
typedef enum{

    /** @brief the policing and counting is
     *  done according to Tunnel Passenger
     *  packet size:
     *  For IP-Over-x packets, the counting
     *  includes the passenger packet's
     *  Byte Count, excluding the tunnel
     *  header and the packets CRC
     *  For, Ethernet-Over-x packets, the
     *  counting includes the passenger
     *  packet's Byte Count, excluding the
     *  tunnel header and if packet includes
     *  Two CRC patterns (one for the inner
     *  packet and one for the outer packets)
     *  also excluding the outer CRC.
     */
    CPSS_POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E,

    /** @brief the policing and counting is
     *  done according to L3 datagram
     *  size only.
     */
    CPSS_POLICER_PACKET_SIZE_L3_ONLY_E,

    /** @brief the policing and counting is
     *  done according to the packet
     *  length including L2.
     */
    CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E,

    /** @brief the policing and counting is
     *  done according to the packet
     *  length including L1
     *  (include preamble + IFG + FCS).
     */
    CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E

} CPSS_POLICER_PACKET_SIZE_MODE_ENT;

/**
* @enum CPSS_POLICER_TB_MODE_ENT
 *
 * @brief Policer Token Bucket mode
*/
typedef enum{

    /** @brief If Token Bucket Size Count > Packet's Byte
     *  Count then packet is comforming, else it is out of profile
     */
    CPSS_POLICER_TB_STRICT_E,

    /** @brief If Token Bucket Size Count > Policer MRU
     *  then packet is comforming, else it is out of profile
     */
    CPSS_POLICER_TB_LOOSE_E

} CPSS_POLICER_TB_MODE_ENT;


/**
* @enum CPSS_POLICER_MRU_ENT
 *
 * @brief Policer MRU
*/
typedef enum{

    /** 1.5 KB, */
    CPSS_POLICER_MRU_1536_E,

    /** 2 KB, */
    CPSS_POLICER_MRU_2K_E,

    /** 10 KB */
    CPSS_POLICER_MRU_10K_E

} CPSS_POLICER_MRU_ENT;

/**
* @enum CPSS_POLICER_COLOR_MODE_ENT
 *
 * @brief Enumeration of Meter color modes
*/
typedef enum{

    /** color blind mode. */
    CPSS_POLICER_COLOR_BLIND_E,

    /** @brief color aware mode.
     *  Notes:
     *  This applies to both Meter types:
     *  single rate three color marking (srTCM)
     *  two rate single color marking (trTCM)
     */
    CPSS_POLICER_COLOR_AWARE_E

} CPSS_POLICER_COLOR_MODE_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenPolicerTypesh */


