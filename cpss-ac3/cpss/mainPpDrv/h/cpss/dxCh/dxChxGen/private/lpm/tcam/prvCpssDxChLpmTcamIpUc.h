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
* @file prvCpssDxChLpmTcamIpUc.h
*
* @brief This file includes functions declarations for controlling the IP (v6/v4)
* UC tables and structures, and structures definitions for shadow
* management.
*
* @version   12
********************************************************************************
*/
#ifndef __prvCpssDxChLpmTcamIpUch
#define __prvCpssDxChLpmTcamIpUch

#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssDxChLpmTcamAddIpUcEntry function
* @endinternal
*
* @brief   creates a new or override an exist route entry in the IP routing
*         structures.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipDestPtr                - The destination IP address of this route.
* @param[in] ipPrefix                 - The number of bits that are actual valid in,
*                                      the ipDestPtr.
* @param[in] nextHopInfoPtr           - the next hop info pointer to associate with this prefix
* @param[in] ruleIdxPtr               - if not null then this is the internal rule index for this
*                                      prefix (Hsu use)
* @param[in] override                 -  an existing entry for this mask if it already
*                                      exists, or don't override and return error.
* @param[in] isBulkInsert             - is this insert is part of a big bulk insertion which
*                                      means we do all the operation apart from writing to the HW
* @param[in] defragEnable             - whether to defrag entries in case there is no
*                                      free place for this entry cause of entries
*                                      fragmentation; relevant only if the TCAM entries
*                                      handler was created with partitionEnable = GT_FALSE
* @param[in] protocolStack            - types of IP we're dealing with.
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NOT_FOUND             - If the given nextHopId is not valid, or
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamAddIpUcEntry
(
    IN GT_U32                                 vrId,
    IN GT_U8                                  *ipDestPtr,
    IN GT_U32                                 ipPrefix,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    IN GT_U32                                 *ruleIdxPtr,
    IN GT_BOOL                                override,
    IN GT_BOOL                                isBulkInsert,
    IN GT_BOOL                                defragEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT             protocolStack,
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     *ipTcamShadowPtr
);

/**
* @internal prvCpssDxChLpmTcamDeleteIpUcEntry function
* @endinternal
*
* @brief   Deletes on ip/prefix unicast address from a practicular virtual private
*         network table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipDestPtr                - The destination IP address of this route.
* @param[in] ipPrefix                 - The number of bits that are actual valid in,
*                                      the ipDestPtr. (CIDR like)
* @param[in] protocolStack            - types of IP we're dealing with.
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - if the required vrId is not valid, or
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_SUPPORTED         - the request is not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamDeleteIpUcEntry
(
    IN GT_U32                             vrId,
    IN GT_U8                              *ipDestPtr,
    IN GT_U32                             ipPrefix,
    IN CPSS_IP_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC  *ipTcamShadowPtr
);


/**
* @internal prvCpssDxChLpmTcamFlushIpUcEntries function
* @endinternal
*
* @brief   flushes the unicast IP Routing table and stays with the default entry
*         only.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] protocolStack            - types of IP we're dealing with.
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
* @param[in] flushDefault             - whether to flush the defualt
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - the request is not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamFlushIpUcEntries
(
    IN GT_U32                             vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC  *ipTcamShadowPtr,
    IN GT_BOOL                            flushDefault
);

/**
* @internal prvCpssDxChLpmTcamSearchIpUcEntry function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the next
*         hop/s associated with it.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in] ipAddrPtr                - The ip address to search for.
* @param[in] prefix                   - Prefix length of ipAddrPtr.
* @param[in] protocolStack            - types of IP we're dealing with.
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
*
* @param[out] nextHopInfoPtr           - The next hop info to which the given prefix is bound.
* @param[out] internalRuleIdxPtr       - if not null then it is filled with internal mangment
*                                      pool rule index (Hsu use)
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_SUPPORTED         - the request is not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamSearchIpUcEntry
(
    IN  GT_U32                                      vrId,
    IN  GT_U8                                       *ipAddrPtr,
    IN  GT_U32                                      prefix,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT      *nextHopInfoPtr,
    OUT GT_U32                                      *internalRuleIdxPtr,
    IN  CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack,
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC           *ipTcamShadowPtr
);



/**
* @internal prvCpssDxChLpmTcamUcRuleOffsetGet function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the
*         Ip Tcam unicast rule (tcam row and tcam column) associated with it.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in] ipAddrPtr                - The ip address to search for.
* @param[in] prefixLength             - Prefix length of ipAddrPtr.
* @param[in] protocolStack            - types of IP we're dealing with.
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
*
* @param[out] tcamRowIndexPtr          - pointer to TCAM row index.
* @param[out] tcamColumnIndexPtr       - pointer to TCAM column index.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_NOT_SUPPORTED         - the request is not supported.
* @retval GT_BAD_VALUE             - on bad value
*/

GT_STATUS prvCpssDxChLpmTcamUcRuleOffsetGet
(
    IN  GT_U32                                      vrId,
    IN  GT_U8                                       *ipAddrPtr,
    IN  GT_U32                                      prefixLength,
    IN  CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack,
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC           *ipTcamShadowPtr,
    OUT GT_U32                                      *tcamRowIndexPtr,
    OUT GT_U32                                      *tcamColumnIndexPtr
);

/**
* @internal prvCpssDxChLpmTcamGetIpUcEntry function
* @endinternal
*
* @brief   This function returns an IP-Unicast entry with larger (ip,prefix) than
*         the given one.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] ipAddrPtr                - The ip address to start the search from.
* @param[in,out] prefixPtr                - Prefix length of ipAddrPtr.
* @param[in] protocolStack            - types of IP we're dealing with.
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
* @param[in,out] ipAddrPtr                - The ip address of the found entry.
* @param[in,out] prefixPtr                - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - The next hop info to which the found prefix is bound.
* @param[out] internalRuleIdxPtr       - if not null then it is filled with internal mangment
*                                      pool rule index (Hsu use)
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_NOT_FOUND             - if no more entries are left in the IP table.
* @retval GT_NOT_SUPPORTED         - the request is not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamGetIpUcEntry
(
    IN GT_U32                                   vrId,
    INOUT GT_U8                                 *ipAddrPtr,
    INOUT GT_U32                                *prefixPtr,
    IN CPSS_IP_PROTOCOL_STACK_ENT               protocolStack,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT GT_U32                                  *internalRuleIdxPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC       *ipTcamShadowPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmTcamIpUch */


