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
* @file cpssIpTypes.h
*
* @brief Common IP facility types definitions
*
* @version   3
********************************************************************************
*/

#ifndef __cpssIpTypesh
#define __cpssIpTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_IP_SITE_ID_ENT
 *
 * @brief This enum defines IPv6 Site ID (Used by Router for Ipv6 scope checking)
*/
typedef enum{

    /** Internal */
    CPSS_IP_SITE_ID_INTERNAL_E,

    /** External */
    CPSS_IP_SITE_ID_EXTERNAL_E

} CPSS_IP_SITE_ID_ENT;

/**
* @enum CPSS_IP_CNT_SET_ENT
 *
 * @brief Each UC/MC Route Entry result can be linked with one the
 * below IP counter sets.
*/
typedef enum{

    /** counter set 0 */
    CPSS_IP_CNT_SET0_E   = 0,

    /** counter set 1 */
    CPSS_IP_CNT_SET1_E   = 1,

    /** counter set 2 */
    CPSS_IP_CNT_SET2_E   = 2,

    /** counter set 3 */
    CPSS_IP_CNT_SET3_E   = 3,

    /** do not link route entry with a counter set */
    CPSS_IP_CNT_NO_SET_E = 4

} CPSS_IP_CNT_SET_ENT;

/**
* @enum CPSS_IPV6_PREFIX_SCOPE_ENT
 *
 * @brief Address scope for IPv6 addresses
*/
typedef enum{

    /** linked local scope */
    CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E   = 0,

    /** site local scope */
    CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E   = 1,

    /** unique local scope */
    CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E = 2,

    /** unique global scope */
    CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E       = 3

} CPSS_IPV6_PREFIX_SCOPE_ENT;

/**
* @enum CPSS_IPV6_MLL_SELECTION_RULE_ENT
 *
 * @brief Rule for choosing MLL for IPv6 multicast propogation
*/
typedef enum{

    /** use local MLL only */
    CPSS_IPV6_MLL_SELECTION_RULE_LOCAL_E   = 0,

    /** use whole MLL */
    CPSS_IPV6_MLL_SELECTION_RULE_GLOBAL_E  = 1

} CPSS_IPV6_MLL_SELECTION_RULE_ENT;

/**
* @enum CPSS_IP_UNICAST_MULTICAST_ENT
 *
 * @brief Ip Unicast / Multicast selection
*/
typedef enum{

    /** ip Unicast */
    CPSS_IP_UNICAST_E    = 0,

    /** ip Multicast */
    CPSS_IP_MULTICAST_E  = 1

} CPSS_IP_UNICAST_MULTICAST_ENT;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssIpTypesh */


