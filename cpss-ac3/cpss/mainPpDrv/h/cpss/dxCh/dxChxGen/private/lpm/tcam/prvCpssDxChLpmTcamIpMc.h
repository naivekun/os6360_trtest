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
* @file prvCpssDxChLpmTcamIpMc.h
*
* @brief This file includes functions declarations for controlling the IP (v6/v4)
* MC tables and structures, and structures definitions for shadow
* management.
*
* @version   16
********************************************************************************
*/
#ifndef __prvCpssDxChLpmTcamIpMch
#define __prvCpssDxChLpmTcamIpMch

#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssDxChLpmTcamAddIpMcEntry function
* @endinternal
*
* @brief   To add the multicast routing information for IP datagrams from
*         a particular source and addressed to a particular IP multicast
*         group address.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupPtr               - (pointer to) The IP multicast group address.
* @param[in] ipGroupPrefix            - The number of bits that are actual valid in,
*                                      the ipGroup. (CIDR like)
* @param[in] ipSrcPtr                 - (pointer to) the root address for source base multi tree protocol.
* @param[in] ipSrcPrefix              - The number of bits that are actual valid in,
*                                      the ipSrc. (CIDR like)
* @param[in] ipLttEntryPtr            - The Ltt entry pointer to write for this mc route
* @param[in] ruleIdxArr[]             - if not null then this is the internal rule indexes
*                                      (G,S) for this prefix (Hsu use)
* @param[in] overwrite                - update or override an existing entry, or create a new one.
* @param[in] defragEnable             - whether to defrag entries in case there is no
*                                      free place for this entry cause of entries
*                                      fragmentation; relevant only if the TCAM entries
*                                      handler was created with partitionEnable = GT_FALSE
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] ipTcamShadowPtr          - the shadow pointer we are working on
*
* @retval GT_OK                    - on success.
* @retval GT_ERROR                 - if the virtual router does not exist, or ipGroupPrefix == 0.
* @retval GT_FULL                  - if IP TCAM is full.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_SUPPORTED         - the request is not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note If tries to override a non existing entry - return error.
*
*/
GT_STATUS prvCpssDxChLpmTcamAddIpMcEntry
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *ipGroupPtr,
    IN GT_U32                                       ipGroupPrefix,
    IN GT_U8                                        *ipSrcPtr,
    IN GT_U32                                       ipSrcPrefix,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *ipLttEntryPtr,
    IN GT_U32                                       ruleIdxArr[],
    IN GT_BOOL                                      overwrite,
    IN GT_BOOL                                      defragEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC            *ipTcamShadowPtr
);

/**
* @internal prvCpssDxChLpmTcamDeleteIpMcEntry function
* @endinternal
*
* @brief   To delete a particular mc route entry and all its MLLs.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupPtr               - (pointer to)The IP multicast group address.
* @param[in] ipGroupPrefix            - The number of bits that are actual valid in,
*                                      the ipGroup. (CIDR like)
* @param[in] ipSrcPtr                 - (pointer to)the root address for source base multi tree protocol.
* @param[in] ipSrcPrefix              - The number of bits that are actual valid in,
*                                      the ipSrc. (CIDR like)
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] ipTcamShadowPtr          - the shadow pointer we are working on
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
*                                       GT_OUT_OF_CPU_MEM if failed to allocate CPU memory, or
*                                       GT_OUT_OF_PP_MEM if failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_SUPPORTED         - the request is not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamDeleteIpMcEntry
(
    IN GT_U32                             vrId,
    IN GT_U8                              *ipGroupPtr,
    IN GT_U32                             ipGroupPrefix,
    IN GT_U8                              *ipSrcPtr,
    IN GT_U32                             ipSrcPrefix,
    IN CPSS_IP_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC  *ipTcamShadowPtr
);

/**
* @internal prvCpssDxChLpmTcamFlushIpMcEntries function
* @endinternal
*
* @brief   flushes the multicast IP Routing table and stays with the default entry
*         only.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] ipTcamShadowPtr          - the shadow pointer we are working on.
* @param[in] flushDefault             - whether to flush the default as well
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - the request is not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamFlushIpMcEntries
(
    IN GT_U32                             vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC  *ipTcamShadowPtr,
    IN GT_BOOL                            flushDefault
);


/**
* @internal prvCpssDxChLpmTcamIpMcEntriesWriteToHw function
* @endinternal
*
* @brief   writes to the HW the multicast IP Routing table
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] vrEntryPtr               - The virtual router entry pointer
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] ipTcamShadowPtr          - the shadow pointer we are working on
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamIpMcEntriesWriteToHw
(
    IN PRV_CPSS_DXCH_LPM_TCAM_VR_TBL_BLOCK_STC *vrEntryPtr,
    IN CPSS_IP_PROTOCOL_STACK_ENT              protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC       *ipTcamShadowPtr
);


/**
* @internal prvCpssDxChLpmTcamSearchMcEntry function
* @endinternal
*
* @brief   This function searches for the route entries associates with a given
*         (S,G), prefixes.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupArr[]             - The ip multicast addr. associated with the given mll node.
* @param[in] ipGroupPrefix            - ipGroup prefix.
* @param[in] ipSrcArr[]               - The source ip address. associated with the given mll node.
* @param[in] srcPrefix                - ipSrc prefix.
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] ipTcamShadowPtr          - the shadow pointer we are working on
*
* @param[out] lttEntryPtr              - pointer to ltt entry struct
* @param[out] internalRuleIdxPtr       - if not null then it is filled with internal mangment
*                                      pool rule index (Hsu use)
*                                       GT_OK if successful, or
*                                       GT_NOT_FOUND if the given prefix was not found.
*
* @retval GT_NOT_SUPPORTED         - the request is not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Calling this function with ipGroupPrefix = 0, will return the
*       default Mc route entries.
*
*/
GT_STATUS prvCpssDxChLpmTcamSearchMcEntry
(
    IN GT_U32                                            vrId,
    IN GT_U8                                             ipGroupArr[],
    IN GT_U32                                            ipGroupPrefix,
    IN GT_U8                                             ipSrcArr[],
    IN GT_U32                                            srcPrefix,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC        *lttEntryPtr,
    OUT GT_U32                                           *internalRuleIdxPtr,
    IN  CPSS_IP_PROTOCOL_STACK_ENT                       protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                 *ipTcamShadowPtr
);

/**
* @internal prvCpssDxChLpmTcamMcRuleOffsetGet function
* @endinternal
*
* @brief   This function searches for the given (S,G) prefixes and returns the
*         Ip Tcam multicast rules offsets associated with it.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupPtr               - (pointer to)The ip multicast addr. associated with the given mll node.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrcPtr                 - (pointer to)The source ip address. associated with the given mll node.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] ipTcamShadowPtr          - the shadow pointer we are working on
*
* @param[out] tcamGroupRowIndexPtr     - pointer to group TCAM row index
* @param[out] tcamGroupColumnIndexPtr  - pointer to group TCAM column index
* @param[out] tcamSrcRowIndexPtr       - pointer to src TCAM row index
* @param[out] tcamSrcColumnIndexPtr    - pointer to src TCAM column index
*
* @retval GT_OK                    - if successful, or
* @retval GT_NOT_FOUND             - if the given prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported.
* @retval GT_BAD_VALUE             - on bad value
*/
GT_STATUS prvCpssDxChLpmTcamMcRuleOffsetGet
(
    IN GT_U32                             vrId,
    IN GT_U8                              *ipGroupPtr,
    IN GT_U32                             ipGroupPrefixLen,
    IN GT_U8                              *ipSrcPtr,
    IN GT_U32                             ipSrcPrefixLen,
    IN  CPSS_IP_PROTOCOL_STACK_ENT        protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC  *ipTcamShadowPtr,
    OUT GT_U32                            *tcamGroupRowIndexPtr,
    OUT GT_U32                            *tcamGroupColumnIndexPtr,
    OUT GT_U32                            *tcamSrcRowIndexPtr,
    OUT GT_U32                            *tcamSrcColumnIndexPtr
);

/**
* @internal prvCpssDxChLpmTcamGetMcEntry function
* @endinternal
*
* @brief   This function returns the next muticast (ipSrc,ipGroup) entry, used
*         to iterate over the existing multicast addresses for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] vrId                     - The virtual router Id.
* @param[in,out] ipGroupArr[]             - The ip Group address to get the next entry for.
* @param[in,out] groupPrefixPtr           - the ipGroup prefix Length.
* @param[in,out] ipSrcArr[]               - The ip Source address to get the next entry for.
* @param[in,out] srcPrefixPtr             - ipSrc prefix length.
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] ipTcamShadowPtr          - the shadow pointer we are working on
* @param[in,out] ipGroupArr[]             - The next ip Group address.
* @param[in,out] groupPrefixPtr           - the ipGroup prefix Length.
* @param[in,out] ipSrcArr[]               - The next ip Source address.
* @param[in,out] srcPrefixPtr             - ipSrc prefix length.
*
* @param[out] lttEntryPtr              - the LTT entry pointing to the MC route entry
*                                      associated with this MC route.
* @param[out] internalRuleIdxArray[]   - if not null then it is array of 2 internal rule
*                                      indexes (group and source); HSU use
*                                       GT_OK if found, or
*
* @retval GT_NOT_FOUND             - If the given address is the last one on the IP-Mc table.
* @retval GT_NOT_SUPPORTED         - the request is not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The value of ipGroupPtr must be a valid value, it
*       means that it exists in the IP-Mc Table, unless this is the first
*       call to this function, then it's value is 0/0
*
*/
GT_STATUS prvCpssDxChLpmTcamGetMcEntry
(
    IN    GT_U32                                         vrId,
    INOUT GT_U8                                          ipGroupArr[],
    INOUT GT_U32                                         *groupPrefixPtr,
    INOUT GT_U8                                          ipSrcArr[],
    INOUT GT_U32                                         *srcPrefixPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC      *lttEntryPtr,
    OUT   GT_U32                                         internalRuleIdxArray[],
    IN    CPSS_IP_PROTOCOL_STACK_ENT                     protocolStack,
    IN    PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC              *ipTcamShadowPtr
);

/**
* @internal prvCpssDxChLpmTcamRegisterIpMcFuncs function
* @endinternal
*
* @brief   register the relevant ip functions for that ip protocol
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] shadowType               - the shadow type (cheetah+ or cheetah2)
* @param[in] protocolStack            - types of IP stack used in the future virtual routers.
* @param[in] ipTcamShadowPtr          - the shadow pointer we are working on
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamRegisterIpMcFuncs
(
    IN PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT       shadowType,
    IN CPSS_IP_PROTOCOL_STACK_ENT              protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC       *ipTcamShadowPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmTcamIpMch */


