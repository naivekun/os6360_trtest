/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfIpLpmIpUcPrefixBulkAdd.c
*
* DESCRIPTION:
*       IP UC Prefix Bulk Add for enhanced form of bulk test
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfIpLpmIpUcPrefixBulkAdd
#define __prvTgfIpLpmIpUcPrefixBulkAdd

/* maximum number of prefixes to add during test */
#define PRV_TGF_MAX_PREFIXES_TO_ADD_CNS     5

/*
 * struct PRV_TGF_IP_TCAM_LPM_PREFIXES_ADD_STC
 *
 * Description:
 *      This struct hold parameters set describing inputs for prefixes
 *      add bulk functions.
 *
 * Fields:
 *    lpmDBId           - The LPM DB id.
 *    vrId              - The virtual router identifier.
 *    ipv4Addr          - The destination IPv4 address of this prefix.
 *    ipv6Addr          - The destination IPv6 address of this prefix.
 *    prefixLen         - The prefix length of ipAddr
 *    nextHopInfo       - the route entry info associated with this UC prefix.
 *                        This union holds the two possible associations a
 *                          prefix can have:
 *                          for Cheetha+ devices it will be a PCL action.
 *                          for Cheetha2 devices it will be a LTT entry.
 *    override          - GT_TRUE, override an existing prefix.
 *                        GT_FALSE, don't override an existing prefix,and return
 *                        an error.
 *
 */
typedef struct PRV_TGF_IPVX_LPM_PREFIXES_BULK_STORE_STCT
{
    GT_U32                                lpmDBId;
    GT_U32                                vrId[PRV_TGF_MAX_PREFIXES_TO_ADD_CNS];
    GT_IPADDR                             ipv4Addr[PRV_TGF_MAX_PREFIXES_TO_ADD_CNS];
    GT_IPV6ADDR                           ipv6Addr[PRV_TGF_MAX_PREFIXES_TO_ADD_CNS];
    GT_U32                                ipv4PL[PRV_TGF_MAX_PREFIXES_TO_ADD_CNS];
    GT_U32                                ipv6PL[PRV_TGF_MAX_PREFIXES_TO_ADD_CNS];
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfoPtr[PRV_TGF_MAX_PREFIXES_TO_ADD_CNS];
    GT_BOOL                               override[PRV_TGF_MAX_PREFIXES_TO_ADD_CNS];
    GT_U32                                ipvxAPQty;
}PRV_TGF_IPVX_LPM_PREFIXES_BULK_STORE_STC;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfIpLpmIpUcPrefixBulkAddBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpLpmIpUcPrefixBulkAddBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpLpmIpUcPrefixBulkAddRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] qtyOfRouteEntries        - number of route entries to add during configuration
*                                       None
*/
GT_VOID prvTgfIpLpmIpUcPrefixBulkAddRouteConfigurationSet
(
    GT_U32 qtyOfRouteEntries
);

/**
* @internal prvTgfIpLpmIpvxUcPrefixBulkTest function
* @endinternal
*
* @brief   Run LPM bulk prefixes add/delete test with complex scenario.
*         The goal is to add/delete IP prefixes to LPM table bucket and to
*         transform it through all hierarchy levels (compressed 1, compressed 2,
*         regular) while checking routing capabilities of LPM.
* @param[in] ipvxProtVer              - protocol to run test for [ipv4,ipv6]
*                                       None
*/
GT_VOID prvTgfIpLpmIpvxUcPrefixBulkTest
(
    CPSS_IP_PROTOCOL_STACK_ENT  ipvxProtVer
);

/**
* @internal prvTgfIpLpmIpUcPrefixBulkAddTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] ipvxProtVer              - protocol to generate traffic for [ipv4,ipv6].
* @param[in] ipvxPrfxsQty             - number of prefixes to send packets for.
* @param[in] bulkTestCntrlStc         - pointer to a test running control structure
*                                       None
*/
GT_VOID prvTgfIpLpmIpUcPrefixBulkAddTrafficGenerate
(
    CPSS_IP_PROTOCOL_STACK_ENT                  ipvxProtVer,
    GT_U32                                      ipvxPrfxsQty,
    PRV_TGF_IPVX_LPM_PREFIXES_BULK_STORE_STC    *bulkTestCntrlStc
);

/**
* @internal prvTgfIpLpmIpUcPrefixBulkAddConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 1. Restore Route Configuration
*       2. Restore Base Configuration
*
*/
GT_VOID prvTgfIpLpmIpUcPrefixBulkAddConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpLpmIpUcPrefixBulkAdd */


