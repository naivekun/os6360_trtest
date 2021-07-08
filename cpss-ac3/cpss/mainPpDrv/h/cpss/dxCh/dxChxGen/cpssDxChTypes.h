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
* @file cpssDxChTypes.h
*
* @brief CPSS DXCH Generic types.
*
* @version   8
********************************************************************************
*/

#ifndef __cpssDxChTypesh
#define __cpssDxChTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>

/**
* @struct CPSS_DXCH_OUTPUT_INTERFACE_STC
 *
 * @brief Defines the interface info
*/

typedef struct
{
    GT_BOOL                 isTunnelStart;

    struct{
        CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT passengerPacketType;
        GT_U32                              ptr;
    }tunnelStartInfo;

    CPSS_INTERFACE_INFO_STC physicalInterface;
} CPSS_DXCH_OUTPUT_INTERFACE_STC;

/**
* @enum CPSS_DXCH_MEMBER_SELECTION_MODE_ENT
 *
 * @brief enumerator that hold values for the type of how many bits
 * are used in a member selection function.
 * Used for trunk member selection and by L2 ECMP member selection.
*/
typedef enum{

    /** @brief Use the entire 12 bit hash in the member selection function.
     *  ((Hash[11:0] #members)/4096)
     */
    CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E = 0,

    /** @brief Use only the 6 least significant bits in the member selection.
     *  ((Hash[5:0] #members)/64)
     */
    CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E,

    /** @brief Use only the 6 most significant bits in the member selection.
     *  ((Hash[11:6] #members)/64)
     */
    CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E

} CPSS_DXCH_MEMBER_SELECTION_MODE_ENT;

/**
* @enum CPSS_DXCH_FDB_LEARN_PRIORITY_ENT
 *
 * @brief Enumeration of FDB Learn Priority
*/
typedef enum{

    /** low priority */
    CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E,

    /** high priority */
    CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E

} CPSS_DXCH_FDB_LEARN_PRIORITY_ENT;

/**
* @enum CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT
 *
 * @brief the possible forms to write a pair of entries
*/
typedef enum{

    /** @brief reads/writes just the
     *  first entry of the pair.
     */
    CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E                = 0,

    /** @brief reads/writes
     *  just the second entry of the pair and the next pointer
     */
    CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E  = 1,

    /** reads/writes the whole pair. */
    CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E                     = 2

} CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT;


/**
* @enum CPSS_DXCH_MLL_ACCESS_CHECK_MIN_MAX_ENT
 *
 * @brief whether a limit is used as upper limit or lower limit
*/
typedef enum{

    /** MLL pointer limit is used as a lower limit */
    CPSS_DXCH_MLL_ACCESS_CHECK_MIN_ENT = 0,

    /** MLL pointer limit is used as a upper limit */
    CPSS_DXCH_MLL_ACCESS_CHECK_MAX_ENT = 1

} CPSS_DXCH_MLL_ACCESS_CHECK_MIN_MAX_ENT;

/**
* @enum CPSS_DXCH_ETHERTYPE_TABLE_ENT
 *
 * @brief This enumeration defines etherType table selection
*/
typedef enum{

    /** Ingress TPID table */
    CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E = CPSS_DIRECTION_INGRESS_E,

    /** Egress TPID table */
    CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E = CPSS_DIRECTION_EGRESS_E,

    /** Egress Logical Port TPID table */
    CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E,

    /** Egress tunnel start TPID table */
    CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_TS_E

} CPSS_DXCH_ETHERTYPE_TABLE_ENT;

/**
* @enum CPSS_DXCH_PORT_PROFILE_ENT
 *
 * @brief Type of the source/target port profile.
*/
typedef enum{

    /** network port profile used in index calculation. */
    CPSS_DXCH_PORT_PROFILE_NETWORK_E = GT_FALSE,

    /** cascade port profile used in index calculation. */
    CPSS_DXCH_PORT_PROFILE_CSCD_E = GT_TRUE,

    /** fabric port profile used in index calculation. */
    CPSS_DXCH_PORT_PROFILE_FABRIC_E,

    /** lopback port profile used in index calculation. */
    CPSS_DXCH_PORT_PROFILE_LOOPBACK_E

} CPSS_DXCH_PORT_PROFILE_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTypesh */

