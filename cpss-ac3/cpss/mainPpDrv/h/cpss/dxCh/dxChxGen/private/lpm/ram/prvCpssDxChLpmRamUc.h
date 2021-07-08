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
* @file prvCpssDxChLpmRamUc.h
*
* @brief This file includes functions declarations for controlling the LPM UC
* tables and structures, and structures definitions for shadow management.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChLpmRamUch
#define __prvCpssDxChLpmRamUch

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvCpssDxChLpmSip6RamUcEntryAdd function
* @endinternal
*
* @brief   creates a new or override an exist route entry in the routing
*         structures.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] destPtr                  - The destination address of this route.
* @param[in] prefix                   - The number of bits that are actual valid in,
*                                      the destPtr.
* @param[in] nextHopPointerPtr        - The next hop pointer to be associated with the given
* @param[in] prefix
* @param[in] insertMode               - how to insert the prefix
*                                      (see PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT)
* @param[in] override                 - whether this is an  or a new insert.
* @param[in] protocolStack            - type of protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - If the vrId was not created yet, or
* @retval GT_NOT_FOUND             - If the given nextHopId is not valid, or
* @retval GT_ALREADY_EXIST         - If overide is false and entry already exist, or
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmSip6RamUcEntryAdd
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *destPtr,
    IN GT_U32                                       prefix,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *nextHopPointerPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN GT_BOOL                                      override,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr
);

/**
* @internal prvCpssDxChLpmRamUcEntryAdd function
* @endinternal
*
* @brief   creates a new or override an exist route entry in the routing
*         structures.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] destPtr                  - The destination address of this route.
* @param[in] prefix                   - The number of bits that are actual valid in,
*                                      the destPtr.
* @param[in] nextHopPointerPtr        - The next hop pointer to be associated with the given
* @param[in] prefix
* @param[in] insertMode               - how to insert the prefix
*                                      (see PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT)
* @param[in] override                 - whether this is an  or a new insert.
* @param[in] protocolStack            - type of protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NOT_FOUND             - If the given nextHopId is not valid, or
* @retval GT_ALREADY_EXIST         - If overide is false and entry already exist, or
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntryAdd
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *destPtr,
    IN GT_U32                                       prefix,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *nextHopPointerPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN GT_BOOL                                      override,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_BOOL                                      defragmentationEnable
);

/**
* @internal prvCpssDxChLpmRamSip6UcEntryDel function
* @endinternal
*
* @brief   Delete address/prefix unicast address from a practicular virtual router
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] destPtr                  - The destination address of this route.
* @param[in] prefix                   - The number of bits that are actual valid in destPtr
* @param[in] updateHwAndMem           - whether an HW update and memory alloc should take place
* @param[in] protocolStack            - type of protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] roolBack                 - GT_TRUE: rollback is taking place.
*                                      GT_FALSE: otherwise.
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - if the required vrId is not valid, or
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamSip6UcEntryDel
(
    IN GT_U32                               vrId,
    IN GT_U8                                *destPtr,
    IN GT_U32                               prefix,
    IN GT_BOOL                              updateHwAndMem,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    IN GT_BOOL                              roolBack
);



/**
* @internal prvCpssDxChLpmRamUcEntryDel function
* @endinternal
*
* @brief   Delete address/prefix unicast address from a practicular virtual router
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] destPtr                  - The destination address of this route.
* @param[in] prefix                   - The number of bits that are actual valid in destPtr
* @param[in] updateHwAndMem           - whether an HW update and memory alloc should take place
* @param[in] protocolStack            - type of protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] roolBack                 - GT_TRUE: rollback is taking place.
*                                      GT_FALSE: otherwise.
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - if the required vrId is not valid, or
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntryDel
(
    IN GT_U32                               vrId,
    IN GT_U8                                *destPtr,
    IN GT_U32                               prefix,
    IN GT_BOOL                              updateHwAndMem,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    IN GT_BOOL                              roolBack
);


/**
* @internal prvCpssDxChLpmRamUcEntriesFlush function
* @endinternal
*
* @brief   flushes the unicast Routing table and stays with the default entry
*         only.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] protocolStack            - the protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntriesFlush
(
    IN GT_U32                               vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr
);

/**
* @internal prvCpssDxChLpmRamUcEntrySearch function
* @endinternal
*
* @brief   This function searches for a given uc address, and returns the next
*         hop pointer associated with it.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in] addrPtr                  - The address to search for.
* @param[in] prefix                   - Prefix length of the address.
* @param[in] protocolStack            - protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] nextHopPointerPtr        - The next hop pointer to which the given prefix is
*                                      bound.
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_NOT_FOUND             - if the given prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntrySearch
(
    IN  GT_U32                                      vrId,
    IN  GT_U8                                       *addrPtr,
    IN  GT_U32                                      prefix,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopPointerPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocolStack,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr
);

/**
* @internal prvCpssDxChLpmRamUcEntryLpmSearch function
* @endinternal
*
* @brief   This function searches for a given uc address, and returns the next
*         hop pointer associated with it. This is a LPM search - the exact prefix
*         doesn't have to exist in DB.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in] addrPtr                  - The address to search for.
* @param[in] protocolStack            - type of protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopPointerPtr        - The next hop pointer to which the given prefix is
*                                      bound.
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_NOT_FOUND             - if the given prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntryLpmSearch
(
    IN  GT_U32                                      vrId,
    IN  GT_U8                                       *addrPtr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopPointerPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocolStack,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr
);

/**
* @internal prvCpssDxChLpmRamUcEntryGet function
* @endinternal
*
* @brief   This function returns an Unicast entry with larger (address,prefix) than
*         the given one.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] addrPtr                  - The address to start the search from.
* @param[in,out] prefixPtr                - Prefix length of addrPtr.
* @param[in] protocolStack            - type of protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in,out] addrPtr                  - The address of the found entry.
* @param[in,out] prefixPtr                - The prefix length of the found entry.
*
* @param[out] nextHopPointerPtrPtr     - A pointer to the next hop pointer
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_NOT_FOUND             - if no more entries are left in the LPM shadow
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntryGet
(
    IN    GT_U32                                    vrId,
    INOUT GT_U8                                     *addrPtr,
    INOUT GT_U32                                    *prefixPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC **nextHopPointerPtrPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocolStack,
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr
);

/**
* @internal prvCpssDxChIpLpmRamUcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific UC prefix
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] protocol                 - type of  stack to work on.
* @param[in] ipPtr                    - Pointer to the ip address to look for.
* @param[in] prefixLen                - ipAddr prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChIpLpmRamUcPrefixActivityStatusGet
(
    IN  GT_U32                               vrId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT           protocol,
    IN  GT_U8                                *ipPtr,
    IN  GT_U32                               prefixLen,
    IN  GT_BOOL                              clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    OUT GT_BOOL                              *activityStatusPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmRamUch */


