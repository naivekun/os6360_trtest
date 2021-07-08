/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssDiagPacketGenerator.h
*
* @brief Common Diag Packet Generator definitions for DxCh and Px.
*
* @version   1
********************************************************************************
*/

#ifndef  __cpssDiagPacketGenerator_h__
#define  __cpssDiagPacketGenerator_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @enum CPSS_DIAG_PG_IF_SIZE_ENT
 *
 * @brief interface size (what is the word width to the port, instead of the TxDMA).
*/
typedef enum{

    /** 8 bits */
    CPSS_DIAG_PG_IF_SIZE_1_BYTE_E    = 0x00,

    /** 64 bits */
    CPSS_DIAG_PG_IF_SIZE_8_BYTES_E   = 0x01,

    /** 128 bits */
    CPSS_DIAG_PG_IF_SIZE_16_BYTES_E  = 0x02,

    /** 256 bits */
    CPSS_DIAG_PG_IF_SIZE_32_BYTES_E  = 0x03,

    /** 512 bits */
    CPSS_DIAG_PG_IF_SIZE_64_BYTES_E  = 0x04,

    /** @brief determine one of CPSS_DIAG_PG_IF_SIZE_1_BYTE_E : CPSS_DIAG_PG_IF_SIZE_64_BYTES_E
     *  based on port mac type and speed
     */
    CPSS_DIAG_PG_IF_SIZE_DEFAULT_E   = 0xFF,

    /** - the feature is not supported at this processor (used at Get function) */
    CPSS_DIAG_PG_IF_SIZE_NOT_APPLICABLE_E  = 0xFFFF

} CPSS_DIAG_PG_IF_SIZE_ENT;

/**
* @enum CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT
 *
 * @brief Enumeration of packet's payload types.
*/
typedef enum{

    /** The packet's payload is cyclic pattern */
    CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E = 0,

    /** The packet's payload is random */
    CPSS_DIAG_PG_PACKET_PAYLOAD_RANDOM_E,

    /** The packet's payload is constant */
    CPSS_DIAG_PG_PACKET_PAYLOAD_CONSTANT_E

} CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT;

/**
* @enum CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT
 *
 * @brief Enumeration of packet's length types.
*/
typedef enum{

    /** The packet's length is constant */
    CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E = 0,

    /** The packet's length is random */
    CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E

} CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT;

/**
* @enum CPSS_DIAG_PG_TRANSMIT_MODE_ENT
 *
 * @brief Enumeration of packet generator transmit modes.
*/
typedef enum{

    /** continues transmit */
    CPSS_DIAG_PG_TRANSMIT_CONTINUES_E = 0,

    /** single burst transmit */
    CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E

} CPSS_DIAG_PG_TRANSMIT_MODE_ENT;

/**
* @enum CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT
 *
 * @brief Enumeration of packet count multiplier.
*/
typedef enum{

    /** multiply by 1 */
    CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1_E = 0,

    /** multiply by 256 */
    CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_256_E,

    /** multiply by 512 */
    CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_512_E,

    /** multiply by 1K (1024) */
    CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1K_E,

    /** multiply by 4K (4096) */
    CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_4K_E,

    /** multiply by 64K (65536) */
    CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_64K_E,

    /** multiply by 1M (1048576) */
    CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1M_E,

    /** multiply by 16M (16777216) */
    CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_16M_E

} CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDiagPacketGenerator_h__ */

