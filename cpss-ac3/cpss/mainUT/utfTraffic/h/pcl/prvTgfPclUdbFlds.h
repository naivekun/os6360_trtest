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
* @file prvTgfPclUdbFlds.h
*
* @brief Second Lookup match
*
* @version   6
********************************************************************************
*/
#ifndef __prvTgfPclUdbFldsh
#define __prvTgfPclUdbFldsh

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_TGF_PCL_RULE_FORMAT_ENT
 *
 * @brief Type of all standard and extended IP key formats
 * according to CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC
 * and CPSS_DXCH_PCL_RULE_FORMAT_UNT
*/
typedef enum
{
    PRV_TGF_PCL_RULE_FORMAT_PCL_ID_E,
    PRV_TGF_PCL_RULE_FORMAT_MAC_TO_ME_E,
    PRV_TGF_PCL_RULE_FORMAT_SOURCE_PORT_E,
    PRV_TGF_PCL_RULE_FORMAT_VID_E,
    PRV_TGF_PCL_RULE_FORMAT_UP_E,
    PRV_TGF_PCL_RULE_FORMAT_IS_IP_E,
    PRV_TGF_PCL_RULE_FORMAT_DSCP_OR_EXP_E,
    PRV_TGF_PCL_RULE_FORMAT_IS_L2_VALID_E,
    PRV_TGF_PCL_RULE_FORMAT_IS_UDB_VALID_E,
    PRV_TGF_PCL_RULE_FORMAT_PKT_TAGGING_E,
    PRV_TGF_PCL_RULE_FORMAT_L3_OFFSET_INVALIDATE_E,
    PRV_TGF_PCL_RULE_FORMAT_L4_PROTOCOL_TYPE_E,
    PRV_TGF_PCL_RULE_FORMAT_PKT_TYPE_E,
    PRV_TGF_PCL_RULE_FORMAT_IP_HEADER_OK_E,
    PRV_TGF_PCL_RULE_FORMAT_MAC_DA_TYPE_E,
    PRV_TGF_PCL_RULE_FORMAT_L4_OFFSET_INVALID_E,
    PRV_TGF_PCL_RULE_FORMAT_L2_ENCAPSULATION_E,
    PRV_TGF_PCL_RULE_FORMAT_IS_IPV6_EH_E,
    PRV_TGF_PCL_RULE_FORMAT_IS_IPV6_HOP_BY_HOP_E,
    PRV_TGF_PCL_RULE_FORMAT_IS_IPV4_E,
    PRV_TGF_PCL_RULE_FORMAT_IS_IPV6_E,
    PRV_TGF_PCL_RULE_FORMAT_IP_PROTOCOL_E,
    PRV_TGF_PCL_RULE_FORMAT_SIP_BITS_31_0_E,             /* 4 Bytes */
    PRV_TGF_PCL_RULE_FORMAT_SIP_BITS_79_32_OR_MAC_SA_E,  /* 6 Bytes */
    PRV_TGF_PCL_RULE_FORMAT_SIP_BITS_127_80_OR_MAC_DA_E, /* 6 Bytes */
    PRV_TGF_PCL_RULE_FORMAT_DIP_BITS_127_112_E,          /* 2 Bytes */
    PRV_TGF_PCL_RULE_FORMAT_DIP_BITS_31_0_E,             /* 4 Bytes */

    PRV_TGF_PCL_RULE_FORMAT_MAX_NUMBER_E
} PRV_TGF_PCL_RULE_FORMAT_ENT;


/**
* @internal prvTgfPclUdbFlds function
* @endinternal
*
* @brief   Set test configuration
*
* @param[in] fieldIndex               -
* @param[in] ipKey                    -
*                                       None
*/
GT_VOID prvTgfPclUdbFlds
(
    IN PRV_TGF_PCL_RULE_FORMAT_ENT      fieldIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipKey
);

/**
* @internal prvTgfPclUdbFldsTwoPorts function
* @endinternal
*
* @brief   Set test configuration sent packets from different ports
*
* @param[in] fieldIndex               -
* @param[in] ipKey                    -
*                                       None
*/
GT_VOID prvTgfPclUdbFldsTwoPorts
(
    IN PRV_TGF_PCL_RULE_FORMAT_ENT      fieldIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipKey
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclUdbFldsh */

