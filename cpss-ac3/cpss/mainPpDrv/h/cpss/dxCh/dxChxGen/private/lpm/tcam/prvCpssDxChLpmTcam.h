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
* @file prvCpssDxChLpmTcam.h
*
* @brief This file includes internal typedefs declarations for controlling the
* IP tables and structures, and structures definitions for shadow
* management.
*
* @version   37
********************************************************************************
*/
#ifndef __prvCpssDxChLpmTcamh
#define __prvCpssDxChLpmTcamh

#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>
#include <cpss/dxCh/dxChxGen/resourceManager/cpssDxChTcamManagerTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Pointers to dummy trie nodes to support policy based routing MC defaults */
extern GT_U32                              pbrDefaultMcTrieNodePtrArraySize;
extern PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC  **pbrDefaultMcTrieNodePtrArray;

/**
* @internal prvCpssDxChLpmTcamMoveToLocation function
* @endinternal
*
* @brief   Copy single entry from one TCAM location to another TCAM location
*         and to invalidate the entry in the original location
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] fromPtr                  - points to the original hardware TCAM location
* @param[in] toPtr                    - points to the new hardware TCAM location
* @param[in] clientCookiePtr          - points to the client cookie associated with the entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChLpmTcamMoveToLocation
(
    IN  CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC    *fromPtr,
    IN  CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC    *toPtr,
    IN  GT_VOID                                     *clientCookiePtr
);

/**
* @internal prvCpssDxChLpmTcamMoveToAnywhere function
* @endinternal
*
* @brief   Move single entry from TCAM location and to invalidate the entry in
*         the original location.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] clientCookiePtr          - points to the client cookie associated with the entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChLpmTcamMoveToAnywhere
(
    IN  GT_VOID         *clientCookiePtr
);

/**
* @internal prvCpssDxChLpmTcamCheckIfDefaultLocation function
* @endinternal
*
* @brief   Return if an entry from TCAM is a default entry
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] clientCookiePtr          - points to the client cookie associated with the entry
*
* @param[out] isDefaultEntryLocationPtr - (pointer to)
*                                      GT_TRUE: clientCookiePtr is a default entry
* @param[out] clientCookiePtr          is not a default entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChLpmTcamCheckIfDefaultLocation
(
    IN  GT_VOID         *clientCookiePtr,
    OUT GT_BOOL         *isDefaultEntryLocationPtr
);

/**
* @internal prvCpssDxChLpmTcamCheckCapacityForShadow function
* @endinternal
*
* @brief   This function checks the capacity given in lpb db create or capacity update.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] shadowType               - the type of shadow we're building
* @param[in] indexesRangePtr          - the range of TCAM indexes availble for this
*                                      LPM DB (see explanation in
*                                      PRV_CPSS_DXCH_LPM_TCAM_MANGER_INDEX_RANGE_STC)
* @param[in] partitionEnable          - whether to enable partitioning of the TCAM
*                                      according to the tcamLpmManagerCapcaityCfgPtr
* @param[in] capacityCfgPtr           - The capcity configuration struct.
*
* @param[out] totalLinesPtr            - total lines needed
* @param[out] totalSingleEntriesPtr    - total single entries needed
* @param[out] totalQuadEntriesPtr      - total quad entries needed
*                                       GT_OK on success, or
*                                       GT_BAD_PARAM on bad capacityCfg.
*/
GT_STATUS prvCpssDxChLpmTcamCheckCapacityForShadow
(
    IN PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT                shadowType,
    IN PRV_CPSS_DXCH_LPM_TCAM_MANGER_INDEX_RANGE_STC    *indexesRangePtr,
    IN GT_BOOL                                          partitionEnable,
    IN  PRV_CPSS_DXCH_LPM_TCAM_MANGER_CAPCITY_CFG_STC   *capacityCfgPtr,
    OUT GT_U32                                          *totalLinesPtr,
    OUT GT_U32                                          *totalSingleEntriesPtr,
    OUT GT_U32                                          *totalQuadEntriesPtr
);

/**
* @internal prvCpssDxChLpmTcamPrepareReservation function
* @endinternal
*
* @brief   This function returns a pointer to a new IP shadow struct.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] shadowType               - the type of shadow we're building (cheetah+ or cheetah2)
* @param[in] partitionEnable          - whether to enable partitioning of the TCAM
*                                      according to tcamLpmManagerCapacityCfgPtr
* @param[in] capacityCfgPtr           - The capcity configuration struct.
* @param[in] totalPlacesNeeded        - total places needed
* @param[in] totalCapacity            - total number of lines possible
* @param[in,out] sizeOfArrayPtr           - points to allocated size of the
* @param[in] requestedEntriesArrayPtr array
*
* @param[out] reservationTypePtr       - type of reservation needed
* @param[out] requestedEntriesArrayPtr - array of amounts needed per entry type
* @param[in,out] sizeOfArrayPtr           - number of elements filled in the
* @param[out] requestedEntriesArrayPtr array
* @param[out] numberOfUnusedTcamEntriesPtr - points to unused Tcam entires
*                                       GT_OK on success, or
*                                       GT_BAD_PARAM on bad capacityCfg.
*/
GT_STATUS prvCpssDxChLpmTcamPrepareReservation
(
    IN    PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT                   shadowType,
    IN    GT_BOOL                                             partitionEnable,
    IN    PRV_CPSS_DXCH_LPM_TCAM_MANGER_CAPCITY_CFG_STC       *capacityCfgPtr,
    IN    GT_U32                                              totalPlacesNeeded,
    IN    GT_U32                                              totalCapacity,
    OUT   CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT *reservationTypePtr,
    OUT   CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC             *requestedEntriesArrayPtr,
    INOUT GT_U32                                              *sizeOfArrayPtr,
    OUT   GT_U32                                              *numberOfUnusedTcamEntriesPtr
);

/**
* @internal prvCpssDxChLpmTcamFreeSpaceCountersCheck function
* @endinternal
*
* @brief   This function check whether TCAM has enough free space for
*         added IPv4/Ipv6 prefixes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] numberOfPrefixes         - number of added prefixes
* @param[in] ipMode                   - ip uc/mc selection
* @param[in] mcEntryType              - mc entry type: relevant only if ipMode is mc.
* @param[in] protocolStack            - the protocol stack to support
* @param[in] ipTcamShadowPtr          - ip shadow we are working on.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong number.
* @retval GT_FULL                  - if TCAM is full.
*/
GT_STATUS prvCpssDxChLpmTcamFreeSpaceCountersCheck
(
    IN GT_U32                                          numberOfPrefixes,
    IN CPSS_IP_UNICAST_MULTICAST_ENT                   ipMode,
    IN PRV_CPSS_DXCH_LPM_TCAM_IP_MC_ENTRY_TYPE_ENT     mcEntryType,
    IN CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC               *ipTcamShadowPtr
);

/**
* @internal prvCpssDxChLpmTcamLttWrite function
* @endinternal
*
* @brief   Writes a LookUp Translation Table Entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number.
* @param[in] lttTtiRow                - the entry's row index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
* @param[in] lttTtiColumn             - the entry's column index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
*                                      lttEntryPtr   - the lookup translation table entry
*
* @retval GT_OK                    - if succeeded
* @retval GT_BAD_PARAM             - on devNum not active or
*                                       invalid ipv6MCGroupScopeLevel value.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Converts an entry of type PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC to
*       CPSS_DXCH_IP_LTT_ENTRY_STC and calls cpssDxChIpLttWrite.
*
*/
GT_STATUS prvCpssDxChLpmTcamLttWrite
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         lttTtiRow,
    IN  GT_U32                                         lttTtiColumn,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC      *routeEntryPtr
);

/**
* @internal prvCpssDxChLpmTcamLttRead function
* @endinternal
*
* @brief   Reads a LookUp Translation Table Entry.
*         DESCRIPTION:
*         Reads a LookUp Translation Table Entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number.
* @param[in] lttTtiRow                - the entry's row index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
* @param[in] lttTtiColumn             - the entry's column index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
*
* @retval GT_OK                    - if succeeded
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Calls cpssDxChIpLttRead and converts an entry of type
*       PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC to CPSS_DXCH_IP_LTT_ENTRY_STC.
*
*/
GT_STATUS prvCpssDxChLpmTcamLttRead
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         lttTtiRow,
    IN  GT_U32                                         lttTtiColumn,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC      *routeEntryPtr
);

/**
* @internal prvCpssDxChLpmTcamInit function
* @endinternal
*
* @brief   Init the Ip unit
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] shadowType               - the type of shadow we're building (cheetah+ or
*                                      cheetah2)
* @param[in] rangeIndexsPtr           - the range of TCAM indexes availble for this
*                                      LPM DB (see explanation in
*                                      PRV_CPSS_DXCH_LPM_TCAM_MANGER_INDEX_RANGE_STC)
* @param[in] partitionEnable          - whether to enable partitioning of the TCAM
*                                      according to the tcamLpmManagercapacityCfgPtr
* @param[in] capacityCfgPtr           - The capcity configuration struct.
* @param[in] protocolStack            - the protocol stack to support
* @param[in] tcamManagerHandlerPtr    - the TCAM manager handler
*
* @param[out] ipTcamShadowPtrPtr       - the ip shadow we created.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamInit
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT                shadowType,
    IN  PRV_CPSS_DXCH_LPM_TCAM_MANGER_INDEX_RANGE_STC    *rangeIndexsPtr,
    IN  GT_BOOL                                          partitionEnable,
    IN  PRV_CPSS_DXCH_LPM_TCAM_MANGER_CAPCITY_CFG_STC    *capacityCfgPtr,
    IN  CPSS_IP_PROTOCOL_STACK_ENT                       protocolStack,
    IN  GT_VOID                                          *tcamManagerHandlerPtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                **ipTcamShadowPtrPtr
);

/**
* @internal prvCpssDxChLpmTcamDeleteShadowStruct function
* @endinternal
*
* @brief   This function deletes IP TCAM shadow struct.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] ipTcamShadowPtr          - A pointer to the ip shadow struct.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChLpmTcamDeleteShadowStruct
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC   *ipTcamShadowPtr
);


/**
* @internal prvCpssDxChLpmTcamAddVirtualRouter function
* @endinternal
*
* @brief   This function adds a new virtual router to the valid virtual routers
*         in system for specific shared shadow devices.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] vrId                     - The virtual's router ID (in cheetah+ this
*                                      will be the pcl id).
* @param[in] defIpv4UcNextHopInfoPtr  - The next hop info representing the default IPv4-Uc
*                                      route entry.
* @param[in] defIpv6UcNextHopInfoPtr  - The next hop info representing the default IPv6-Uc
*                                      route entry.
* @param[in] defUcNextHopRuleIdxArr[] - if not NULL, then this is the internal rule idx
*                                      of the defualt NextHop prefix. (array of 2
*                                      for each of the protocol Stacks)
* @param[in] defIpv4McRouteLttEntryPtr - The default IPv4_Mc LTT entry
* @param[in] defIpv6McRouteLttEntryPtr - The default IPv6_Mc LTT entry
* @param[in] defMcRouteRuleIdxArr[]   - if not null then this is the internal Rule idx
*                                      of the default mc route (prefix) (array of 2
*                                      for each of the protocol Stacks)
* @param[in] protocolStack            - types of IP stack used in this virtual router.
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
*
* @retval GT_OK                    - if success
* @retval GT_NOT_FOUND             - if the given next hop Id is not valid.
* @retval GT_ALREADY_EXIST         - if the virtual router already exists
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamAddVirtualRouter
(
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT       *defIpv4UcNextHopInfoPtr,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT       *defIpv6UcNextHopInfoPtr,
    IN GT_U32                                       defUcNextHopRuleIdxArr[],
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defIpv4McRouteLttEntryPtr,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defIpv6McRouteLttEntryPtr,
    IN GT_U32                                       defMcRouteRuleIdxArr[],
    IN CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC            *ipTcamShadowPtr
);

/**
* @internal prvCpssDxChLpmTcamGetVirtualRouter function
* @endinternal
*
* @brief   This function gets the virtual router configuration
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] vrId                     - The virtual's router ID (in cheetah+ this
*                                      will be the pcl id).
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
*
* @param[out] supportIpv4UcPtr         - Whether the virtual router supports IPv4 UC
* @param[out] defIpv4UcNextHopInfoPtr  - The next hop info representing the default Ipv4 Uc
*                                      route entry.
*                                      Relevant only when <supportIpv4UcPtr> == GT_TRUE
* @param[out] supportIpv6UcPtr         - Whether the virtual router supports IPv6 UC
* @param[out] defIpv6UcNextHopInfoPtr  - The next hop info representing the default Ipv6 Uc
*                                      route entry.
*                                      Relevant only when <supportIpv6UcPtr> == GT_TRUE
* @param[out] defUcNextHopRuleIdxArr[] - if not NULL, then this is the internal rule idx
*                                      of the defualt NextHop prefix. (array of 2
*                                      for each of the protocol Stacks)
*                                      Relevant only when <supportIpv4UcPtr> == GT_TRUE
*                                      or <supportIpv6UcPtr> == GT_TRUE
* @param[out] supportIpv4McPtr         - Whether the virtual router supports Ipv4 MC
* @param[out] defIpv4McRouteLttEntryPtr - The default Ipv4 MC LTT entry
*                                      Relevant only when <supportIpv4McPtr> == GT_TRUE
* @param[out] supportIpv6McPtr         - Whether the virtual router supports Ipv6 MC
* @param[out] defIpv6McRouteLttEntryPtr - The default Ipv6 MC LTT entry
*                                      Relevant only when <supportIpv6McPtr> == GT_TRUE
* @param[out] defMcRouteRuleIdxArr[]   - if not null then this is the internal Rule idx
*                                      of the default mc route (prefix) (array of 2
*                                      for each of the protocol Stacks)
*                                      Relevant only when <supportIpv4McPtr> == GT_TRUE
*                                      or <supportIpv6McPtr> == GT_TRUE
*                                       GT_OK on success, or
*
* @retval GT_NOT_FOUND             - if the given next hop Id is not valid.
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamGetVirtualRouter
(
    IN  GT_U32                                       vrId,
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC            *ipTcamShadowPtr,
    OUT GT_BOOL                                      *supportIpv4UcPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT       *defIpv4UcNextHopInfoPtr,
    OUT GT_BOOL                                      *supportIpv6UcPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT       *defIpv6UcNextHopInfoPtr,
    OUT GT_U32                                       defUcNextHopRuleIdxArr[],
    OUT GT_BOOL                                      *supportIpv4McPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defIpv4McRouteLttEntryPtr,
    OUT GT_BOOL                                      *supportIpv6McPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defIpv6McRouteLttEntryPtr,
    OUT GT_U32                                       defMcRouteRuleIdxArr[]
);

/**
* @internal prvCpssDxChLpmTcamDeleteVirtualRouter function
* @endinternal
*
* @brief   This function deletes an existing virtual router from the vr table, The
*         IP tables (Uc & Mc) are flushed as a result of deletion.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] vrId                     - The virtual's router ID to be deleted.
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
*                                       GT_OK on success, or
*                                       GT_FAIL if the given vrId does not exist.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamDeleteVirtualRouter
(
    IN GT_U32                              vrId,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC   *ipTcamShadowPtr
);

/**
* @internal prvCpssDxChLpmTcamAddDevToShadow function
* @endinternal
*
* @brief   add the device to the ip shadow
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devListPtr               - The device list to add.
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamAddDevToShadow
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STC *devListPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC      *ipTcamShadowPtr
);

/**
* @internal prvCpssDxChLpmTcamRemoveDevsFromShadow function
* @endinternal
*
* @brief   removes the devices from belonging to the shadow.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devListPtr               - The device list to remove
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamRemoveDevsFromShadow
(
    IN PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STC *devListPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC      *ipTcamShadowPtr
);

/**
* @internal prvCpssDxChLpmTcamGetDevsFromShadow function
* @endinternal
*
* @brief   Get the devices from belonging to the shadow.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on.
* @param[in,out] numOfDevsPtr             - points to the size of devListArray
* @param[in,out] numOfDevsPtr             - points to the number of devices retrieved
*
* @param[out] devListArray[]           - array of device ids in the LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to device list
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The application should allocate memory for the device list array.
*       If the function returns GT_OK, then number of devices holds
*       the number of devices the function filled.
*       If the function returns GT_BAD_SIZE, then the memory allocated by the
*       application to the device list is not enough. In this case the
*       number of devices will hold the size of array needed.
*
*/
GT_STATUS prvCpssDxChLpmTcamGetDevsFromShadow
(
    IN    PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC          *ipTcamShadowPtr,
    OUT   GT_U8                                      devListArray[],
    INOUT GT_U32                                     *numOfDevsPtr
);

/**
* @internal prvCpssDxChLpmTcamTrieSetPrefixIpCheetah2 function
* @endinternal
*
* @brief   utility function to Add IP UC prefix.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] vrId                     - The virtual's router ID.
* @param[in] ipAddrPtr                - IP Uc address
* @param[in] prefixLen                - Ip Uc prefix length
* @param[in] ecmpRouteNum             - the ecmp route number to write
* @param[in] isMcSrc                  - if this entry is MC source
* @param[in] gIndex                   - the G-index to write (relevant if isMcSrc == GT_TRUE)
* @param[in] tcamRulePtr              - the tcam rule to set it in
* @param[in] ipTcamShadowPtr          - ip shadow we are working on.
* @param[in] pData                    - the data to attach to this prefix.
* @param[in] protocolStack            - the type of the prefix.
* @param[in] devListPtr               - pointer to the device list to preform the actions on.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. In order to add a default IP group, use prefixLen = 0.
*       2. Bit 15 in the vrId indicates whether this is shared virtual router
*       or not (if the bit is 1 this is shared virtual router).
*       For shared virtual router:
*       - <match all> entries (meaning perfix len is 0) are not written to
*       the hardware
*       - all other prefixes are written with <don't care> masking for
*       the virtual router field.
*
*/
GT_STATUS prvCpssDxChLpmTcamTrieSetPrefixIpCheetah2
(
    IN  GT_U32                                    vrId,
    IN  GT_U8                                     *ipAddrPtr,
    IN  GT_U32                                    prefixLen,
    IN  GT_U8                                     ecmpRouteNum,
    IN  GT_BOOL                                   isMcSrc,
    IN  GT_U16                                    gIndex,
    IN  GT_PTR                                    tcamRulePtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC         *ipTcamShadowPtr,
    IN  GT_PTR                                    pData,
    IN  CPSS_IP_PROTOCOL_STACK_ENT                protocolStack,
    IN  PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STC    *devListPtr
);

/**
* @internal prvCpssDxChLpmTcamTrieDelPrefixCheetah2 function
* @endinternal
*
* @brief   utility function to Delete IP prefix rule from IP table.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] vrId                     - the virtual router id.
* @param[in] tcamRulePtr              - the tcam rule to delete it from.
* @param[in] ipTcamShadowPtr          - ip shadow we are working on.
* @param[in] pData                    - the data entry that was attached to this prefix.
* @param[in] protocolStack            - the protocol stack of the prefix.
* @param[in] devListPtr               - pointer to the device list to preform the actions on.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamTrieDelPrefixCheetah2
(
    IN  GT_U32                                        vrId,
    IN  GT_PTR                                        tcamRulePtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC             *ipTcamShadowPtr,
    IN  GT_PTR                                        pData,
    IN  CPSS_IP_PROTOCOL_STACK_ENT                    protocolStack,
    IN  PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STC        *devListPtr
);

/**
* @internal prvCpssDxChLpmTcamTrieDelIpv4PrefixCheetah2 function
* @endinternal
*
* @brief   utility function to Delete IP prefix rule from IP table.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] vrId                     - the virtual router id.
* @param[in] tcamRulePtr              - the tcam rule to delete it from.
* @param[in] ipTcamShadowPtr          - ip shadow we are working on.
* @param[in] pData                    - the data entry that was attached to this prefix.
* @param[in] devListPtr               - pointer to the device list to preform the actions on.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamTrieDelIpv4PrefixCheetah2
(
    IN  GT_U32                                        vrId,
    IN  GT_PTR                                        tcamRulePtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC             *ipTcamShadowPtr,
    IN  GT_PTR                                        pData,
    IN  PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STC        *devListPtr
);

/**
* @internal prvCpssDxChLpmTcamTrieDelIpv6PrefixCheetah2 function
* @endinternal
*
* @brief   utility function to Delete IP prefix rule from IP table.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] vrId                     - the virtual router id.
* @param[in] tcamRulePtr              - the tcam rule to delete it from.
* @param[in] ipTcamShadowPtr          - ip shadow we are working on.
* @param[in] pData                    - the data entry that was attached to this prefix.
* @param[in] devListPtr               - pointer to the device list to preform the actions on.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamTrieDelIpv6PrefixCheetah2
(
    IN  GT_U32                                        vrId,
    IN  GT_PTR                                        tcamRulePtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC             *ipTcamShadowPtr,
    IN  GT_PTR                                        pData,
    IN  PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STC        *devListPtr
);

/**
* @internal prvCpssDxChLpmTcamUpdateHw function
* @endinternal
*
* @brief   Perform the actual HW update at the end of a hot sync operation
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] ipTcamShadowPtr          - the shadow to write the HW.
* @param[in] updateMode               - the update mode.
* @param[in] protocolStack            - which protocol stack to update.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamUpdateHw
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC         *ipTcamShadowPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_UPDATE_TYPE_ENT    updateMode,
    IN CPSS_IP_PROTOCOL_STACK_ENT                protocolStack
);

/**
* @internal prvCpssDxChLpmTcamPbrDefaultMcRuleSet function
* @endinternal
*
* @brief   Sets "do-nothing" PCL rule to capture all MC traffic for specific
*         protocol stack.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] ipTcamShadowPtr          - the ip shadow we are working on
* @param[in] devListPtr               - list of devices to set the rule at
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
*/
GT_STATUS prvCpssDxChLpmTcamPbrDefaultMcRuleSet
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC       *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STC  *devListPtr
);

/**
* @internal prvCpssDxChLpmTcamGetTcamOffset function
* @endinternal
*
* @brief   Get IP TCAM hardware offset
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] ipTcamShadowPtr          - ip shadow we are working on.
* @param[in] tcamRulePtr              - pointer to tcam rule
* @param[in] isToken                  - whether the tcamRulePtr represent TCAM location
*                                      or token (relevant only in TCAM manager mode)
*
* @param[out] offsetPtr                - pointer to TCAM rule hardware offset.
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on FAIL
*/
GT_STATUS prvCpssDxChLpmTcamGetTcamOffset
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC          *ipTcamShadowPtr,
    IN  GT_PTR                                    tcamRulePtr,
    IN  GT_BOOL                                   isToken,
    OUT GT_U32                                    *offsetPtr
);

/**
* @internal prvCpssDxChLpmTcamGetTcamRowColumn function
* @endinternal
*
* @brief   Get IP TCAM hardware location :row and column
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] ipTcamShadowPtr          - ip shadow we are working on.
* @param[in] tcamRulePtr              - pointer to tcam rule
*
* @param[out] tcamRowPtr               - pointer to TCAM row.
* @param[out] tcamColumnPtr            - pointer to TCAM column.
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvCpssDxChLpmTcamGetTcamRowColumn
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC          *ipTcamShadowPtr,
    IN  GT_PTR                                    tcamRulePtr,
    OUT GT_U32                                    *tcamRowPtr,
    OUT GT_U32                                    *tcamColumnPtr
);

/**
* @internal prvCpssDxChLpmTcamDbCreate function
* @endinternal
*
* @brief   This function creates an LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtrPtr              - the LPM DB information
* @param[in] shadowType               - the type of shadow to maintain
* @param[in] protocolStack            - the type of protocol stack this LPM DB support
* @param[in] memoryCfgPtr             - the memory configuration of this LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning
*                                       is disabled.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamDbCreate
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     **lpmDbPtrPtr,
    IN PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT     shadowType,
    IN CPSS_IP_PROTOCOL_STACK_ENT            protocolStack,
    IN PRV_CPSS_DXCH_LPM_TCAM_CONFIG_STC     *memoryCfgPtr
);

/**
* @internal prvCpssDxChLpmTcamDbDelete function
* @endinternal
*
* @brief   This function deletes LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_STATE             - the LPM DB is not empty
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Deleting LPM DB is allowed only if no VR present in the LPM DB (since
*       prefixes reside within VR it means that no prefixes present as well).
*
*/
GT_STATUS prvCpssDxChLpmTcamDbDelete
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     *lpmDbPtr
);

/**
* @internal prvCpssDxChLpmTcamDbConfigGet function
* @endinternal
*
* @brief   This function retrieves configuration of the LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmTcamDbConfigGet
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC    *lpmDbPtr,
    OUT CPSS_IP_PROTOCOL_STACK_ENT           *protocolStackPtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_CONFIG_STC    *memoryCfgPtr
);

/**
* @internal prvCpssDxChLpmTcamDbCapacityUpdate function
* @endinternal
*
* @brief   This function updates the initial LPM DB allocation.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] indexesRangePtr          - the range of TCAM indexes availble for this
*                                      LPM DB (see explanation in
*                                      PRV_CPSS_DXCH_LPM_TCAM_MANGER_INDEX_RANGE_STC).
*                                      this field is relevant when partitionEnable
*                                      in cpssDxChIpLpmDBCreate was GT_TRUE.
* @param[in] tcamLpmManagerCapcityCfgPtr - the new capacity configuration. when
*                                      partitionEnable in cpssDxChIpLpmDBCreate
*                                      was set to GT_TRUE this means new prefixes
*                                      partition, when this was set to GT_FALSE
*                                      this means the new prefixes guaranteed
*                                      allocation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NO_RESOURCE           - failed to allocate resources needed to the
*                                       new configuration
*/
GT_STATUS prvCpssDxChLpmTcamDbCapacityUpdate
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                *lpmDbPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_MANGER_INDEX_RANGE_STC    *indexesRangePtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_MANGER_CAPCITY_CFG_STC    *tcamLpmManagerCapcityCfgPtr
);

/**
* @internal prvCpssDxChLpmTcamDbCapacityGet function
* @endinternal
*
* @brief   This function gets the current LPM DB allocation.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
*
* @param[out] indexesRangePtr          - the range of TCAM indexes availble for this
*                                      LPM DB (see explanation in
*                                      PRV_CPSS_DXCH_LPM_TCAM_MANGER_INDEX_RANGE_STC).
*                                      this field is relevant when partitionEnable
*                                      in cpssDxChIpLpmDBCreate was GT_TRUE.
* @param[out] partitionEnablePtr       - GT_TRUE:  the TCAM is partitioned according
*                                      to the capacityCfgPtr, any unused TCAM entries
*                                      were allocated to IPv4 UC entries.
*                                      GT_FALSE: TCAM entries are allocated on demand
*                                      while entries are guaranteed as specified
*                                      in capacityCfgPtr.
* @param[out] tcamLpmManagerCapcityCfgPtr - the current capacity configuration. when
*                                      partitionEnable in cpssDxChIpLpmDBCreate
*                                      was set to GT_TRUE this means current
*                                      prefixes partition, when this was set to
*                                      GT_FALSE this means the current guaranteed
*                                      prefixes allocation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_RESOURCE           - failed to allocate resources needed to the
*                                       new configuration.
*/
GT_STATUS prvCpssDxChLpmTcamDbCapacityGet
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                *lpmDbPtr,
    OUT GT_BOOL                                          *partitionEnablePtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_MANGER_INDEX_RANGE_STC    *indexesRangePtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_MANGER_CAPCITY_CFG_STC    *tcamLpmManagerCapcityCfgPtr
);

/**
* @internal prvCpssDxChLpmTcamDbDevListAdd function
* @endinternal
*
* @brief   This function adds devices to an existing LPM DB. this addition will
*         invoke a hot sync of the newly added devices.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
*                                      devListArr     - the array of device ids to add to the LPM DB.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChLpmTcamDbDevListAdd
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     *lpmDbPtr,
    IN GT_U8                                 devListArray[],
    IN GT_U32                                numOfDevs
);

/**
* @internal prvCpssDxChLpmTcamDbDevListRemove function
* @endinternal
*
* @brief   This function removes devices from an existing LPM DB. this remove will
*         invoke a hot sync removal of the devices.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
*                                      devListArr     - the array of device ids to remove from the
*                                      LPM DB.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChLpmTcamDbDevListRemove
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     *lpmDbPtr,
    IN GT_U8                                 devListArray[],
    IN GT_U32                                numOfDevs
);

/**
* @internal prvCpssDxChLpmTcamPolicyBasedRoutingDefaultMcSet function
* @endinternal
*
* @brief   The function uses available TCAM rules at the beginning of the TCAM range
*         allocated to the LPM DB and place there "do nothing" rules that will make
*         sure that Multicast traffic is matched by these rules and not by the
*         Unicast default rule.
*         When policy based routing Unicast traffic coexists with IP based Multicast
*         (S,G,V) bridging (used in IGMP and MLD protocols), there is a need to add
*         default Multicast rules in order to make sure that the Multicast traffic
*         will not be matched by the Unicast default rule.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] pclIdArray[]             - size of the PCL ID array (range 1..1048575)
* @param[in] pclIdArray[]             - array of PCL ID that may be in used by the LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if wrong value in any of the parameters
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_STATE             - if the existing LPM DB is not empty
* @retval GT_ALREADY_EXIST         - if default MC already set
* @retval GT_NOT_SUPPORTED         - if the LPM DB doesn't configured to
*                                       operate in policy based routing mode
* @retval GT_FULL                  - if TCAM is full
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The following conditions must be matched:
*       - the LPM DB must be empty (no prefixes and no virtual routers)
*       - the LPM DB must be configured to use policy based routing
*       One default Multicast rule will be set for each protocol stack supported
*       by the LPM DB and for each PCL ID that may be used in the LPM DB (each
*       virtaul router uses unique PCL ID).
*       For each protocol stack, the Multicast default rules will use the
*       prefixes allocated for Unicast prefixes for this protocol. For example
*       if the application allocates 10 IPv4 Unicast prefixes, then after setting
*       one default Multicast prefix, only 9 IPv4 Unicast prefixes will be
*       available.
*       The default Multicast rules will capture all IPv4/IPv6 Multicast
*       traffic with the same assigned PCL ID. As a result all rules configured
*       to match IP Multicast traffic with same assigned PCL ID that reside
*       after the Multicast defaults will not be matched.
*       The default Multicast rules can not be deleted after set.
*       The default Multicast rules can be set no more than once.
*
*/
GT_STATUS prvCpssDxChLpmTcamPolicyBasedRoutingDefaultMcSet
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC               *lpmDbPtr,
    IN  GT_U32                                          pclIdArrayLen,
    IN  GT_U32                                          pclIdArray[]
);

/**
* @internal prvCpssDxChLpmTcamPolicyBasedRoutingDefaultMcGet function
* @endinternal
*
* @brief   The function gets whether the LPM DB is configured to support default MC
*         rules in policy based routing mode and the rule indexes and PCL ID of those
*         default rules.
*         Refer to cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet for more details.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in,out] defaultIpv4RuleIndexArrayLenPtr - points to the allocated size of the array
* @param[in,out] defaultIpv6RuleIndexArrayLenPtr - points to the allocated size of the array
* @param[in,out] pclIdArrayLenPtr         - points to the allocated size of the array
*
* @param[out] defaultMcUsedPtr         - whether default MC is used for this LPM DB
* @param[out] protocolStackPtr         - protocol stack supported by this LPM DB
*                                      relevant only if defaultMcUsedPtr == GT_TRUE
* @param[out] defaultIpv4RuleIndexArray[] - rule indexes of the default IPv4 MC (in case
*                                      the LPM DB support IPv4)
*                                      relevant only if defaultMcUsedPtr == GT_TRUE
* @param[in,out] defaultIpv4RuleIndexArrayLenPtr - points to number of elements filled in the array
* @param[out] defaultIpv6RuleIndexArray[] - rule indexes of the default IPv6 MC (in case
*                                      the LPM DB support IPv6)
*                                      relevant only if defaultMcUsedPtr == GT_TRUE
* @param[in,out] defaultIpv6RuleIndexArrayLenPtr - points to number of elements filled in the array
* @param[out] pclIdArray[]             - array of PCL ID that may be in used by the LPM DB
*                                      relevant only if defaultMcUsedPtr == GT_TRUE
* @param[in,out] pclIdArrayLenPtr         - points to number of elements filled in the array
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_FULL                  - if any of the arrays is not big enough
* @retval GT_FAIL                  - on failure
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The rule indexes are according to explanations in cpssDxChPclRuleSet.
*
*/
GT_STATUS prvCpssDxChLpmTcamPolicyBasedRoutingDefaultMcGet
(
    IN    PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC  *lpmDbPtr,
    OUT   GT_BOOL                            *defaultMcUsedPtr,
    OUT   CPSS_IP_PROTOCOL_STACK_ENT         *protocolStackPtr,
    OUT   GT_U32                             defaultIpv4RuleIndexArray[],
    INOUT GT_U32                             *defaultIpv4RuleIndexArrayLenPtr,
    OUT   GT_U32                             defaultIpv6RuleIndexArray[],
    INOUT GT_U32                             *defaultIpv6RuleIndexArrayLenPtr,
    OUT   GT_U32                             pclIdArray[],
    INOUT GT_U32                             *pclIdArrayLenPtr
);

/**
* @internal prvCpssDxChLpmTcamVirtualRouterAdd function
* @endinternal
*
* @brief   This function adds a virtual router in system for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual's router ID.
* @param[in] vrConfigPtr              - Virtual router configuration.
*
* @retval GT_OK                    - if success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_PARAM             - if wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - if the existing VR is not empty.
* @retval GT_BAD_PTR               - if illegal pointer value
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
*
* @note Refer to cpssDxChIpLpmVirtualRouterSharedAdd for limitation when shared
*       virtual router is used.
*
*/
GT_STATUS prvCpssDxChLpmTcamVirtualRouterAdd
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     *lpmDbPtr,
    IN GT_U32                                vrId,
    IN PRV_CPSS_DXCH_LPM_TCAM_VR_CONFIG_STC  *vrConfigPtr
);

/**
* @internal prvCpssDxChLpmTcamVirtualRouterSharedAdd function
* @endinternal
*
* @brief   This function adds a shared virtual router in system for specific LPM DB.
*         Prefixes that reside within shared virtual router will participate in the
*         lookups of all virtual routers.
*
* @note   APPLICABLE DEVICES:      xCat3;  Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router ID.
* @param[in] vrConfigPtr              - Virtual router configuration.
*
* @retval GT_OK                    - if success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate TCAM memory.
* @retval GT_BAD_PTR               - if illegal pointer value
* @retval GT_BAD_STATE             - if the existing VR is not empty.
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
*
* @note 1. Only one shared virtual router can exists at any time within a
*       given LPM DB.
*       2. Virtual router ID that is used for non-shared virtual router can't
*       be used for the shared virtual router and via versa.
*       3. Shared prefixes can't overlap non-shared prefixes. If the shared
*       virtual router supports shared prefixes type then adding non-shared
*       virtual router that supports the same prefixes type will fail.
*       Also, if a non-shared virtual router that supports prefixes type
*       exists, then adding a shared virtual router that supports the same
*       prefixes type will fail.
*       4. When the shared virtual router supports IPv4 UC prefixes, then the
*       the following will apply:
*       - The <match all> default TCAM entry will not be written to TCAM
*       (however TCAM entry will still be allocated to this entry)
*       - The following prefixes will be added to match all non-MC traffic:
*       0x00/1, 0x80/2, 0xC0/3, 0xF0/4
*       Same applies when the shared virtual router supports IPv6 UC
*       prefixes. The prefixes added to match all non-MC traffic are:
*       0x00/1, 0x80/2, 0xC0/3, 0xE0/4, 0xF0/5, 0xF8/6, 0xFC/7, 0xFE/8
*       5. The application should not delete the non-MC prefixes (when added).
*       6. Manipulation of the default UC will not be applied on the non-MC
*       prefixes. The application should manipulate those non-MC entries
*       directly when needed.
*
*/
GT_STATUS prvCpssDxChLpmTcamVirtualRouterSharedAdd
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     *lpmDbPtr,
    IN GT_U32                                vrId,
    IN PRV_CPSS_DXCH_LPM_TCAM_VR_CONFIG_STC  *vrConfigPtr
);

/**
* @internal prvCpssDxChLpmTcamVirtualRouterGet function
* @endinternal
*
* @brief   This function gets the virtual router in system for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual's router ID.
*
* @param[out] vrConfigPtr              - Virtual router configuration
*
* @retval GT_OK                    - if success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate TCAM memory.
* @retval GT_BAD_PTR               - if illegal pointer value
* @retval GT_BAD_STATE             - if the existing VR is not empty.
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmTcamVirtualRouterGet
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC    *lpmDbPtr,
    IN  GT_U32                               vrId,
    OUT PRV_CPSS_DXCH_LPM_TCAM_VR_CONFIG_STC *vrConfigPtr
);

/**
* @internal prvCpssDxChLpmTcamVirtualRouterDel function
* @endinternal
*
* @brief   This function removes a virtual router in system for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual's router ID.
*                                       GT_OK on success, or
*
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmTcamVirtualRouterDel
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     *lpmDbPtr,
    IN GT_U32                                vrId
);

/**
* @internal prvCpssDxChLpmTcamIpv4UcPrefixAdd function
* @endinternal
*
* @brief   Creates a new or override an existing Ipv4 prefix in a Virtual Router for
*         the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix.
* @param[in] override                 -  the existing entry if it already exists
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate TCAM memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv4UcPrefixAdd
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC         *lpmDbPtr,
    IN GT_U32                                    vrId,
    IN GT_IPADDR                                 ipAddr,
    IN GT_U32                                    prefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT    *nextHopInfoPtr,
    IN GT_BOOL                                   override
);

/**
* @internal prvCpssDxChLpmTcamIpv4UcPrefixBulkAdd function
* @endinternal
*
* @brief   Creates a new or override an existing bulk of Ipv4 prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] ipv4PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv4PrefixArrayPtr       - The UC prefix array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on one of prefixes' lengths is too big
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv4UcPrefixBulkAdd
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                        *lpmDbPtr,
    IN GT_U32                                                   ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC                      *ipv4PrefixArrayPtr
);

/**
* @internal prvCpssDxChLpmTcamIpv4UcPrefixDel function
* @endinternal
*
* @brief   Deletes an existing Ipv4 prefix in a Virtual Router for the specified LPM
*         DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NO_SUCH               - If the given prefix doesn't exitst in the VR, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note the default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv4UcPrefixDel
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     *lpmDbPtr,
    IN GT_U32                                vrId,
    IN GT_IPADDR                             ipAddr,
    IN GT_U32                                prefixLen
);

/**
* @internal prvCpssDxChLpmTcamIpv4UcPrefixBulkDel function
* @endinternal
*
* @brief   Deletes an existing bulk of Ipv4 prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] ipv4PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv4PrefixArrayPtr       - The UC prefix array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on one of the prefixes' lengths is too big
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv4UcPrefixBulkDel
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                        *lpmDbPtr,
    IN GT_U32                                                   ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC                      *ipv4PrefixArrayPtr
);

/**
* @internal prvCpssDxChLpmTcamIpv4UcPrefixesFlush function
* @endinternal
*
* @brief   Flushes the unicast IPv4 Routing table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamIpv4UcPrefixesFlush
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     *lpmDbPtr,
    IN GT_U32                                vrId
);

/**
* @internal prvCpssDxChLpmTcamIpv4UcPrefixGet function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the
*         information associated with it.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - if found, this is the route entry info.
*                                      accosiated with this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChLpmTcamIpv4UcPrefixGet
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC       *lpmDbPtr,
    IN  GT_U32                                  vrId,
    IN  GT_IPADDR                               ipAddr,
    IN  GT_U32                                  prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT GT_U32                                  *tcamRowIndexPtr,
    OUT GT_U32                                  *tcamColumnIndexPtr
);

/**
* @internal prvCpssDxChLpmTcamIpv4UcPrefixGetNext function
* @endinternal
*
* @brief   This function returns an IP-Unicast prefix with larger (ip,prefix) than
*         the given one and prefix Tcam index; it used for iterating over the existing
*         prefixes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] ipAddrPtr                - The ip address to start the search from.
* @param[in,out] prefixLenPtr             - Prefix length of ipAddr.
* @param[in,out] ipAddrPtr                - The ip address of the found entry.
* @param[in,out] prefixLenPtr             - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - If found, this is the route entry info accosiated
*                                      with this UC prefix.
* @param[out] tcamRowIndexPtr          - TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if no more entries are left in the IP table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChLpmTcamIpv4UcPrefixGetNext
(
    IN    PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC         *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_IPADDR                                 *ipAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT    *nextHopInfoPtr,
    OUT   GT_U32                                    *tcamRowIndexPtr,
    OUT   GT_U32                                    *tcamColumnIndexPtr
);

/**
* @internal prvCpssDxChLpmTcamIpv4McEntryAdd function
* @endinternal
*
* @brief   Add IP multicast route for a particular/all source and a particular
*         group address. this is done for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree
*                                      protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*                                      mcRouteLttEntryPtr  - the LTT entry pointing to the MC route entry
*                                      associated with this MC route.
* @param[in] override                 - whether to  an mc Route pointer for the
*                                      given prefix
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - if one of prefixes' lengths is too big.
* @retval GT_ERROR                 - if the virtual router does not exist.
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory.
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate TCAM memory.
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*
* @note to override the default mc route use ipGroup = 0.
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv4McEntryAdd
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC            *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPADDR                                    ipGroup,
    IN GT_U32                                       ipGroupPrefixLen,
    IN GT_IPADDR                                    ipSrc,
    IN GT_U32                                       ipSrcPrefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *mcRouteEntryPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
);

/**
* @internal prvCpssDxChLpmTcamIpv4McEntryDel function
* @endinternal
*
* @brief   To delete a particular mc route entry for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Inorder to delete the multicast entry and all the src ip addresses
*       associated with it, call this function with ipSrc = ipSrcPrefix = 0.
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv4McEntryDel
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC    *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPADDR                            ipGroup,
    IN GT_U32                               ipGroupPrefixLen,
    IN GT_IPADDR                            ipSrc,
    IN GT_U32                               ipSrcPrefixLen
);

/**
* @internal prvCpssDxChLpmTcamIpv4McEntriesFlush function
* @endinternal
*
* @brief   Flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamIpv4McEntriesFlush
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC    *lpmDbPtr,
    IN GT_U32                               vrId
);

/**
* @internal prvCpssDxChLpmTcamIpv4McEntryGetNext function
* @endinternal
*
* @brief   This function returns the next muticast (ipSrc,ipGroup) entry, used
*         to iterate over the existing multicast addresses for a specific LPM DB,
*         and ipSrc,ipGroup TCAM indexes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router Id.
* @param[in,out] ipGroupPtr               - The ip Group address to get the next entry for.
* @param[in,out] ipGroupPrefixLenPtr      - the ipGroup prefix Length.
* @param[in,out] ipSrcPtr                 - The ip Source address to get the next entry for.
* @param[in,out] ipSrcPrefixLenPtr        - ipSrc prefix length.
* @param[in,out] ipGroupPtr               - The next ip Group address.
* @param[in,out] ipGroupPrefixLenPtr      - the ipGroup prefix Length.
* @param[in,out] ipSrcPtr                 - The next ip Source address.
* @param[in,out] ipSrcPrefixLenPtr        - ipSrc prefix length.
*
* @param[out] mcRouteLttEntryPtr       - the LTT entry pointer pointing to the MC route.
*                                      entry associated with this MC route.
* @param[out] tcamGroupRowIndexPtr     - pointer to TCAM group row  index.
* @param[out] tcamGroupColumnIndexPtr  - pointer to TCAM group column  index.
* @param[out] tcamSrcRowIndexPtr       - pointer to TCAM source row  index.
* @param[out] tcamSrcColumnIndexPtr    - pointer to TCAM source column  index.
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if one of prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The value of ipGroupPtr must be a valid value, it
*       means that it exists in the IP-Mc Table, unless this is the first
*       call to this function, then it's value is 0.
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv4McEntryGetNext
(
    IN    PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC          *lpmDbPtr,
    IN    GT_U32                                     vrId,
    INOUT GT_IPADDR                                  *ipGroupPtr,
    INOUT GT_U32                                     *ipGroupPrefixLenPtr,
    INOUT GT_IPADDR                                  *ipSrcPtr,
    INOUT GT_U32                                     *ipSrcPrefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  *mcRouteLttEntryPtr,
    OUT   GT_U32                                     *tcamGroupRowIndexPtr,
    OUT   GT_U32                                     *tcamGroupColumnIndexPtr,
    OUT   GT_U32                                     *tcamSrcRowIndexPtr,
    OUT   GT_U32                                     *tcamSrcColumnIndexPtr
);

/**
* @internal prvCpssDxChLpmTcamIpv4McEntryGet function
* @endinternal
*
* @brief   This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry, and ipSrc,ipGroup TCAM indexes
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefixLen         - The ip Group prefix len.
* @param[in] ipSrc                    - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteLttEntryPtr       - the LTT entry pointer pointing to the MC route
*                                      entry associated with this MC route.
* @param[out] tcamGroupRowIndexPtr     - pointer to TCAM group row  index.
* @param[out] tcamGroupColumnIndexPtr  - pointer to TCAM group column  index.
* @param[out] tcamSrcRowIndexPtr       - pointer to TCAM source row  index.
* @param[out] tcamSrcColumnIndexPtr    - pointer to TCAM source column  index.
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChLpmTcamIpv4McEntryGet
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC           *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPADDR                                   ipGroup,
    IN  GT_U32                                      ipGroupPrefixLen,
    IN  GT_IPADDR                                   ipSrc,
    IN  GT_U32                                      ipSrcPrefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *mcRouteLttEntryPtr,
    OUT GT_U32                                      *tcamGroupRowIndexPtr,
    OUT GT_U32                                      *tcamGroupColumnIndexPtr,
    OUT GT_U32                                      *tcamSrcRowIndexPtr,
    OUT GT_U32                                      *tcamSrcColumnIndexPtr
);

/**
* @internal prvCpssDxChLpmTcamIpv6UcPrefixAdd function
* @endinternal
*
* @brief   Creates a new or override an existing Ipv6 prefix in a Virtual Router
*         for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - wether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - If prefix length is too big.
* @retval GT_ERROR                 - If the vrId was not created yet.
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory.
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate TCAM memory.
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv6UcPrefixAdd
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC      *lpmDbPtr,
    IN GT_U32                                 vrId,
    IN GT_IPV6ADDR                            ipAddr,
    IN GT_U32                                 prefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    IN GT_BOOL                                override,
    IN GT_BOOL                                defragmentationEnable
);

/**
* @internal prvCpssDxChLpmTcamIpv6UcPrefixBulkAdd function
* @endinternal
*
* @brief   Creates a new or override an existing bulk of Ipv6 prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] ipv6PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv6PrefixArrayPtr       - The UC prefix array.
* @param[in] defragmentationEnable    - wether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - on one of prefixes' lengths is too big.
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv6UcPrefixBulkAdd
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                         *lpmDbPtr,
    IN GT_U32                                                    ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC                       *ipv6PrefixArrayPtr,
    IN GT_BOOL                                                   defragmentationEnable
);

/**
* @internal prvCpssDxChLpmTcamIpv6UcPrefixDel function
* @endinternal
*
* @brief   Deletes an existing Ipv6 prefix in a Virtual Router for the specified LPM
*         DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - The LPM DB information
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NO_SUCH               - If the given prefix doesn't exitst in the VR, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note the default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv6UcPrefixDel
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC    *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPV6ADDR                          ipAddr,
    IN GT_U32                               prefixLen
);

/**
* @internal prvCpssDxChLpmTcamIpv6UcPrefixBulkDel function
* @endinternal
*
* @brief   Deletes an existing bulk of Ipv6 prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] ipv6PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv6PrefixArrayPtr       - The UC prefix array.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on one of prefixes' lengths is too big, or
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv6UcPrefixBulkDel
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                        *lpmDbPtr,
    IN GT_U32                                                   ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC                      *ipv6PrefixArrayPtr
);

/**
* @internal prvCpssDxChLpmTcamIpv6UcPrefixesFlush function
* @endinternal
*
* @brief   Flushes the unicast IPv6 Routing table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamIpv6UcPrefixesFlush
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC    *lpmDbPtr,
    IN GT_U32                               vrId
);

/**
* @internal prvCpssDxChLpmTcamIpv6UcPrefixGet function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the next
*         hop pointer associated with it and TCAM prefix index.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - If  found, the route entry info accosiated with
*                                      this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChLpmTcamIpv6UcPrefixGet
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC        *lpmDbPtr,
    IN  GT_U32                                  vrId,
    IN  GT_IPV6ADDR                             ipAddr,
    IN  GT_U32                                  prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT GT_U32                                  *tcamRowIndexPtr,
    OUT GT_U32                                  *tcamColumnIndexPtr
);

/**
* @internal prvCpssDxChLpmTcamIpv6UcPrefixGetNext function
* @endinternal
*
* @brief   This function returns an IP-Unicast prefix with larger (ip,prefix) than
*         the given one and prefix Tcam index; it used for iterating over the existing
*         prefixes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] ipAddrPtr                - The ip address to start the search from.
* @param[in,out] prefixLenPtr             - Prefix length of ipAddr.
* @param[in,out] ipAddrPtr                - The ip address of the found entry.
* @param[in,out] prefixLenPtr             - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - If found, the route entry info accosiated with
*                                      this UC prefix.
* @param[out] tcamRowIndexPtr          - If found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - If found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if no more entries are left in the IP table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChLpmTcamIpv6UcPrefixGetNext
(
    IN    PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC       *lpmDbPtr,
    IN    GT_U32                                  vrId,
    INOUT GT_IPV6ADDR                             *ipAddrPtr,
    INOUT GT_U32                                  *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT   GT_U32                                  *tcamRowIndexPtr,
    OUT   GT_U32                                  *tcamColumnIndexPtr
);

/**
* @internal prvCpssDxChLpmTcamIpv6McEntryAdd function
* @endinternal
*
* @brief   To add the multicast routing information for IP datagrams from a particular
*         source and addressed to a particular IP multicast group address for a
*         specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*                                      mcRouteLttEntryPtr - the LTT entry pointing to the MC route entry
*                                      associated with this MC route.
* @param[in] override                 - weather to  the mcRoutePointerPtr for the
*                                      given prefix
* @param[in] defragmentationEnable    - wether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big.
* @retval GT_ERROR                 - if the virtual router does not exist.
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory.
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented .
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*
* @note to override the default mc route use ipGroup = ipGroupPrefixLen = 0.
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv6McEntryAdd
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC          *lpmDbPtr,
    IN GT_U32                                     vrId,
    IN GT_IPV6ADDR                                ipGroup,
    IN GT_U32                                     ipGroupPrefixLen,
    IN GT_IPV6ADDR                                ipSrc,
    IN GT_U32                                     ipSrcPrefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  *mcRouteEntryPtr,
    IN GT_BOOL                                    override,
    IN GT_BOOL                                    defragmentationEnable
);

/**
* @internal prvCpssDxChLpmTcamIpv6McEntryDel function
* @endinternal
*
* @brief   To delete a particular mc route entry for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. In order to delete the multicast entry and all the src ip addresses
*       associated with it, call this function with ipSrc = ipSrcPrefix = 0.
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv6McEntryDel
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC    *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPV6ADDR                          ipGroup,
    IN GT_U32                               ipGroupPrefixLen,
    IN GT_IPV6ADDR                          ipSrc,
    IN GT_U32                               ipSrcPrefixLen
);

/**
* @internal prvCpssDxChLpmTcamIpv6McEntriesFlush function
* @endinternal
*
* @brief   Flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamIpv6McEntriesFlush
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC    *lpmDbPtr,
    IN GT_U32                               vrId
);

/**
* @internal prvCpssDxChLpmTcamIpv6McEntryGetNext function
* @endinternal
*
* @brief   This function returns the next muticast (ipSrc,ipGroup) entry, used
*         to iterate over the existing multicast addresses for a specific LPM DB,
*         and ipSrc,ipGroup TCAM indexes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router Id.
* @param[in,out] ipGroupPtr               - The ip Group address to get the next entry for.
* @param[in,out] ipGroupPrefixLenPtr      - ipGroup prefix length.
* @param[in,out] ipSrcPtr                 - The ip Source address to get the next entry for.
* @param[in,out] ipSrcPrefixLenPtr        - ipSrc prefix length.
* @param[in,out] ipGroupPtr               - The next ip Group address.
* @param[in,out] ipGroupPrefixLenPtr      - ipGroup prefix length.
* @param[in,out] ipSrcPtr                 - The next ip Source address.
* @param[in,out] ipSrcPrefixLenPtr        - ipSrc prefix length.
*                                      mcRouteLttEntryPtr   - the LTT entry pointer pointing to the MC route
*                                      entry associated with this MC route.
*
* @param[out] tcamGroupRowIndexPtr     - pointer to TCAM group row  index.
* @param[out] tcamSrcRowIndexPtr       - pointer to TCAM source row  index.
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if one of prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The values of (ipGroupPtr,ipGroupPrefixLenPtr) must be a valid
*       values, it means that they exist in the IP-Mc Table, unless this is
*       the first call to this function, then the value of (ipGroupPtr,
*       ipSrcPtr) is (0,0).
*
*/
GT_STATUS prvCpssDxChLpmTcamIpv6McEntryGetNext
(
    IN    PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC         *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_IPV6ADDR                               *ipGroupPtr,
    INOUT GT_U32                                    *ipGroupPrefixLenPtr,
    INOUT GT_IPV6ADDR                               *ipSrcPtr,
    INOUT GT_U32                                    *ipSrcPrefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcRouteEntryPtr,
    OUT   GT_U32                                    *tcamGroupRowIndexPtr,
    OUT   GT_U32                                    *tcamSrcRowIndexPtr
);

/**
* @internal prvCpssDxChLpmTcamIpv6McEntryGet function
* @endinternal
*
* @brief   This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry, and ipSrc,ipGroup TCAM indexes
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrc                    - The ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if one of prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChLpmTcamIpv6McEntryGet
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC         *lpmDbPtr,
    IN  GT_U32                                    vrId,
    IN  GT_IPV6ADDR                               ipGroup,
    IN  GT_U32                                    ipGroupPrefixLen,
    IN  GT_IPV6ADDR                               ipSrc,
    IN  GT_U32                                    ipSrcPrefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcRouteEntryPtr,
    OUT GT_U32                                    *tcamGroupRowIndexPtr,
    OUT GT_U32                                    *tcamSrcRowIndexPtr
);

/**
* @internal prvCpssDxChLpmTcamRowsClear function
* @endinternal
*
* @brief   restore tcamRows to 'pre-init' state
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*                                       None
*/
GT_VOID prvCpssDxChLpmTcamRowsClear
(
    GT_VOID
);

/**
* @internal prvCpssDxChLpmTcamConvertIpLttEntryToLpmRouteEntry function
* @endinternal
*
* @brief   Convert IP LTT entry structure to private LPM route entry structure
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] ipLttEntryPtr            - the IP LTT entry
*
* @param[out] routeEntryPtr            - the LPM route entry
*                                       None
*/
GT_VOID prvCpssDxChLpmTcamConvertIpLttEntryToLpmRouteEntry
(
    IN  CPSS_DXCH_IP_LTT_ENTRY_STC                      *ipLttEntryPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *routeEntryPtr
);

/**
* @internal prvCpssDxChLpmTcamConvertLpmRouteEntryToIpLttEntry function
* @endinternal
*
* @brief   Convert private LPM route entry structure to IP LTT entry structure
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] routeEntryPtr            - the LPM route entry
*
* @param[out] ipLttEntryPtr            - the IP LTT entry
*                                       None
*/
GT_VOID prvCpssDxChLpmTcamConvertLpmRouteEntryToIpLttEntry
(
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *routeEntryPtr,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC                      *ipLttEntryPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmTcamh */

