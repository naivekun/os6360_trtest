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
* @file cpssDxChIpLpm.h
*
* @brief the CPSS DXCH LPM Hierarchy manager .
*
* @version   43
********************************************************************************
*/

#ifndef __cpssDxChIpLpmh
#define __cpssDxChIpLpmh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h>


/**
* @internal cpssDxChIpLpmDBCreate function
* @endinternal
*
* @brief   This function creates an LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] lpmDBId                  - the LPM DB id
* @param[in] shadowType               - the type of shadow to maintain
* @param[in] protocolStack            - the type of protocol stack this LPM DB support
* @param[in] memoryCfgPtr             - (pointer to)the memory configuration for this LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning
*                                       is disabled.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDBCreate
(
    IN GT_U32                                       lpmDBId,
    IN CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType,
    IN CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack,
    IN CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           *memoryCfgPtr
);

/**
* @internal cpssDxChIpLpmDBDelete function
* @endinternal
*
* @brief   This function deletes LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id (range 32 bits)
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
GT_STATUS cpssDxChIpLpmDBDelete
(
    IN  GT_U32      lpmDbId
);

/**
* @internal cpssDxChIpLpmDBConfigGet function
* @endinternal
*
* @brief   This function retrieves configuration of the LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - the LPM DB id.
*
* @param[out] shadowTypePtr            - (pointer to)the type of shadow to maintain
* @param[out] protocolStackPtr         - (pointer to)the type of protocol stack this LPM DB support
* @param[out] memoryCfgPtr             - (pointer to)the memory configuration for this LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_PTR               - NULL pointer.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDBConfigGet
(
    IN  GT_U32                             lpmDBId,
    OUT CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT   *shadowTypePtr,
    OUT CPSS_IP_PROTOCOL_STACK_ENT         *protocolStackPtr,
    OUT CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT *memoryCfgPtr
);

/**
* @internal cpssDxChIpLpmDBCapacityUpdate function
* @endinternal
*
* @brief   This function updates the initial LPM DB allocation.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDBId                  - the LPM DB id.
* @param[in] indexesRangePtr          - (pointer to)the range of TCAM indexes availble
*                                      for this LPM DB (see explanation in
*                                      CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC).
*                                      this field is relevant when partitionEnable
*                                      in cpssDxChIpLpmDBCreate was GT_TRUE.
* @param[in] tcamLpmManagerCapcityCfgPtr - (pointer to)the new capacity configuration.
*                                      When partitionEnable in cpssDxChIpLpmDBCreate
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
*
* @note This API is relevant only when using TCAM Manager. This API is used for
*       updating only the capacity configuration of the LPM. for updating the
*       lines reservation for the TCAM Manger use cpssDxChTcamManagerRangeUpdate.
*
*/
GT_STATUS cpssDxChIpLpmDBCapacityUpdate
(
    IN GT_U32                                       lpmDBId,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
);


/**
* @internal cpssDxChIpLpmDBCapacityGet function
* @endinternal
*
* @brief   This function gets the current LPM DB allocation.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDBId                  - the LPM DB id.
*
* @param[out] indexesRangePtr          - (pointer to) the range of TCAM indexes
*                                      available for this LPM DB (see explanation in
*                                      CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC).
*                                      this field is relevant when partitionEnable
*                                      in cpssDxChIpLpmDBCreate was GT_TRUE.
* @param[out] partitionEnablePtr       - (pointer to) the partitioning enabling status
*                                      GT_TRUE: the TCAM is partitioned according
*                                      to the capacityCfgPtr, any unused TCAM entries
*                                      were allocated to IPv4 UC entries.
*                                      GT_FALSE: TCAM entries are allocated on demand
*                                      while entries are guaranteed as specified
*                                      in capacityCfgPtr.
* @param[out] tcamLpmManagerCapcityCfgPtr - (pointer to) the current capacity
*                                      configuration. when partitionEnable in
*                                      cpssDxChIpLpmDBCreate was set to GT_TRUE
*                                      this means current prefixes partition,
*                                      when this was set to GT_FALSE this means
*                                      that the current guaranteed prefixes
*                                      allocation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_RESOURCE           - failed to allocate resources needed to the
*                                       new configuration.
*/
GT_STATUS cpssDxChIpLpmDBCapacityGet
(
    IN  GT_U32                                          lpmDBId,
    OUT GT_BOOL                                         *partitionEnablePtr,
    OUT CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    *indexesRangePtr,
    OUT CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    *tcamLpmManagerCapcityCfgPtr
);

/**
* @internal cpssDxChIpLpmDBDevListAdd function
* @endinternal
*
* @brief   This function adds devices to an existing LPM DB. this addition will
*         invoke a hot sync of the newly added devices.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - the LPM DB id.
* @param[in] devListArr[]             - the array of device ids to add to the LPM DB.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
*
* @note For LPM shadowType CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E, device should
*       be added only to 1 lpmDBId
*
*/
GT_STATUS cpssDxChIpLpmDBDevListAdd
(
    IN GT_U32   lpmDBId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
);

/**
* @internal cpssDxChIpLpmDBDevsListRemove function
* @endinternal
*
* @brief   This function removes devices from an existing LPM DB. this remove will
*         invoke a hot sync removal of the devices.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - the LPM DB id.
* @param[in] devListArr[]             - the array of device ids to remove from the
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
GT_STATUS cpssDxChIpLpmDBDevsListRemove
(
    IN GT_U32   lpmDBId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
);

/**
* @internal cpssDxChIpLpmDBDevListGet function
* @endinternal
*
* @brief   This function retrieves the list of devices in an existing LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id.
* @param[in,out] numOfDevsPtr             - (pointer to) the size of devListArray
* @param[in,out] numOfDevsPtr             - (pointer to) the number of devices retreived
*
* @param[out] devListArray[]           - array of device ids in the LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to
*                                       the device list
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
GT_STATUS cpssDxChIpLpmDBDevListGet
(
    IN    GT_U32                        lpmDbId,
    INOUT GT_U32                        *numOfDevsPtr,
    OUT   GT_U8                         devListArray[]
);

/**
* @internal cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet function
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
* @param[in] lpmDBId                  - the LPM DB id.
* @param[in] pclIdArray[]             - size of the PCL ID array (APPLICABLE RANGES: 1..1048575)
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
GT_STATUS cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet
(
    IN  GT_U32  lpmDBId,
    IN  GT_U32  pclIdArrayLen,
    IN  GT_U32  pclIdArray[]
);

/**
* @internal cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet function
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
* @param[in] lpmDBId                  - the LPM DB id.
* @param[in,out] defaultIpv4RuleIndexArrayLenPtr - (pointer to) the allocated size of the array
* @param[in,out] defaultIpv6RuleIndexArrayLenPtr - (pointer to) the allocated size of the array
* @param[in,out] pclIdArrayLenPtr         - (pointer to) the allocated size of the array
*
* @param[out] defaultMcUsedPtr         - (pointer to) whether default MC is used for this LPM DB
* @param[out] protocolStackPtr         - (pointer to) protocol stack supported by this LPM DB
*                                      relevant only if defaultMcUsedPtr == GT_TRUE
* @param[out] defaultIpv4RuleIndexArray[] - rule indexes of the default IPv4 MC (in case
*                                      the LPM DB support IPv4)
*                                      relevant only if defaultMcUsedPtr == GT_TRUE
* @param[in,out] defaultIpv4RuleIndexArrayLenPtr - (pointer to) number of elements filled in the array
* @param[out] defaultIpv6RuleIndexArray[] - rule indexes of the default IPv6 MC (in case
*                                      the LPM DB support IPv6)
*                                      relevant only if defaultMcUsedPtr == GT_TRUE
* @param[in,out] defaultIpv6RuleIndexArrayLenPtr - (pointer to) number of elements filled in the array
* @param[out] pclIdArray[]             - array of PCL ID that may be in used by the LPM DB
*                                      relevant only if defaultMcUsedPtr == GT_TRUE
* @param[in,out] pclIdArrayLenPtr         - (pointer to) number of elements filled in the array
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
GT_STATUS cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet
(
    IN      GT_U32                          lpmDBId,
    OUT     GT_BOOL                         *defaultMcUsedPtr,
    OUT     CPSS_IP_PROTOCOL_STACK_ENT      *protocolStackPtr,
    OUT     GT_U32                          defaultIpv4RuleIndexArray[],
    INOUT   GT_U32                          *defaultIpv4RuleIndexArrayLenPtr,
    OUT     GT_U32                          defaultIpv6RuleIndexArray[],
    INOUT   GT_U32                          *defaultIpv6RuleIndexArrayLenPtr,
    OUT     GT_U32                          pclIdArray[],
    INOUT   GT_U32                          *pclIdArrayLenPtr
);

/**
* @internal cpssDxChIpLpmVirtualRouterAdd function
* @endinternal
*
* @brief   This function adds a virtual router in system for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual's router ID.(APPLICABLE RANGES: 0..4095)
* @param[in] vrConfigPtr              - (pointer to) Virtual router configuration.
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
GT_STATUS cpssDxChIpLpmVirtualRouterAdd
(
    IN GT_U32                                 lpmDBId,
    IN GT_U32                                 vrId,
    IN  CPSS_DXCH_IP_LPM_VR_CONFIG_STC        *vrConfigPtr
);


/**
* @internal cpssDxChIpLpmVirtualRouterSharedAdd function
* @endinternal
*
* @brief   This function adds a shared virtual router in system for specific LPM DB.
*         Prefixes that reside within shared virtual router will participate in the
*         lookups of all virtual routers.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual's router ID.(APPLICABLE RANGES: 0..4095)
* @param[in] vrConfigPtr              - (pointer to) Virtual router configuration.
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
GT_STATUS cpssDxChIpLpmVirtualRouterSharedAdd
(
    IN GT_U32                                 lpmDBId,
    IN GT_U32                                 vrId,
    IN  CPSS_DXCH_IP_LPM_VR_CONFIG_STC        *vrConfigPtr
);

/**
* @internal cpssDxChIpLpmVirtualRouterGet function
* @endinternal
*
* @brief   This function gets the virtual router in system for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual's router ID.(APPLICABLE RANGES: 0..4095)
*
* @param[out] vrConfigPtr              - (pointer to) Virtual router configuration.
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
GT_STATUS cpssDxChIpLpmVirtualRouterGet
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    OUT CPSS_DXCH_IP_LPM_VR_CONFIG_STC          *vrConfigPtr
);

/**
* @internal cpssDxChIpLpmVirtualRouterDel function
* @endinternal
*
* @brief   This function removes a virtual router in system for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - the LPM DB id.
* @param[in] vrId                     - The virtual's router ID.(APPLICABLE RANGES: 0..4095)
*                                       GT_OK on success, or
*
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChIpLpmVirtualRouterDel
(
    IN GT_U32                                 lpmDBId,
    IN GT_U32                                 vrId
);

/**
* @internal cpssDxChIpLpmIpv4UcPrefixAdd function
* @endinternal
*
* @brief   Creates a new or override an existing Ipv4 prefix in a Virtual Router for
*         the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddrPtr                - (pointer to) the destination IP address of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
* @param[in] nextHopInfoPtr           - (pointer to) the route entry info accosiated with this
*                                      UC prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
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
GT_STATUS cpssDxChIpLpmIpv4UcPrefixAdd
(
    IN GT_U32                                   lpmDBId,
    IN GT_U32                                   vrId,
    IN GT_IPADDR                                *ipAddrPtr,
    IN GT_U32                                   prefixLen,
    IN CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT   *nextHopInfoPtr,
    IN GT_BOOL                                  override,
    IN GT_BOOL                                  defragmentationEnable
);

/**
* @internal cpssDxChIpLpmIpv4UcPrefixAddBulk function
* @endinternal
*
* @brief   Creates a new or override an existing bulk of Ipv4 prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - the LPM DB id
* @param[in] ipv4PrefixArrayLen       - length of UC prefix array
* @param[in] ipv4PrefixArrayPtr       - (pointer to) the UC prefix array
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on one of prefixes' lengths is too big
*
* @note none.
*
*/
GT_STATUS cpssDxChIpLpmIpv4UcPrefixAddBulk
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArrayPtr
);


/**
* @internal cpssDxChIpLpmIpv4UcPrefixDel function
* @endinternal
*
* @brief   Deletes an existing Ipv4 prefix in a Virtual Router for the specified LPM
*         DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddrPtr                - (pointer to) the destination IP address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NO_SUCH               - If the given prefix doesn't exist in the VR, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note the default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS cpssDxChIpLpmIpv4UcPrefixDel
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               vrId,
    IN GT_IPADDR                            *ipAddrPtr,
    IN GT_U32                               prefixLen
);

/**
* @internal cpssDxChIpLpmIpv4UcPrefixDelBulk function
* @endinternal
*
* @brief   Deletes an existing bulk of Ipv4 prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - the LPM DB id
* @param[in] ipv4PrefixArrayLen       - length of UC prefix array
* @param[in] ipv4PrefixArrayPtr       - (pointer to) the UC prefix array
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
GT_STATUS cpssDxChIpLpmIpv4UcPrefixDelBulk
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArrayPtr
);


/**
* @internal cpssDxChIpLpmIpv4UcPrefixesFlush function
* @endinternal
*
* @brief   Flushes the unicast IPv4 Routing table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.(APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmIpv4UcPrefixesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
);

/**
* @internal cpssDxChIpLpmIpv4UcPrefixSearch function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the
*         information associated with it.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddrPtr                - (pointer to) the destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
*                                      ipAddr.
*
* @param[out] nextHopInfoPtr           - (pointer to) the route entry info associated with
*                                      this UC prefix, if found.
* @param[out] tcamRowIndexPtr          - (pointer to) the TCAM row index of this UC
*                                      prefix, if found.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamColumnIndexPtr       - (pointer to) the TCAM column index of this UC
*                                      prefix, if found.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChIpLpmIpv4UcPrefixSearch
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_IPADDR                               *ipAddrPtr,
    IN  GT_U32                                  prefixLen,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT GT_U32                                  *tcamRowIndexPtr,
    OUT GT_U32                                  *tcamColumnIndexPtr
);



/**
* @internal cpssDxChIpLpmIpv4UcPrefixGetNext function
* @endinternal
*
* @brief   This function returns an IP-Unicast prefix with larger (ip,prefix) than
*         the given one; it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id to get the entry from.(APPLICABLE RANGES: 0..4095)
* @param[in,out] ipAddrPtr                - The ip address to start the search from.
* @param[in,out] prefixLenPtr             - Prefix length of ipAddr.
* @param[in,out] ipAddrPtr                - (pointer to) the ip address of the found entry
* @param[in,out] prefixLenPtr             - (pointer to) the prefix length of the found entry
*
* @param[out] nextHopInfoPtr           - (pointer to) the route entry info associated
*                                      with this UC prefix, if found.
* @param[out] tcamRowIndexPtr          - (pointer to) the TCAM row index of this uc prefix
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamColumnIndexPtr       - (pointer to) the TCAM column index of this uc
*                                      prefix.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if no more entries are left in the IP table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChIpLpmIpv4UcPrefixGetNext
(
    IN    GT_U32                                    lpmDBId,
    IN    GT_U32                                    vrId,
    INOUT GT_IPADDR                                 *ipAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *nextHopInfoPtr,
    OUT GT_U32                                      *tcamRowIndexPtr,
    OUT GT_U32                                      *tcamColumnIndexPtr

);

/**
* @internal cpssDxChIpLpmIpv4UcPrefixGet function
* @endinternal
*
* @brief   This function gets a given ip address, find LPM match in the trie and
*         returns the prefix length and pointer to the next hop information bound
*         to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddrPtr                - (pointer to) the destination IP address to look for.
*
* @param[out] prefixLenPtr             - (pointer to) the prefix length of the found entry
* @param[out] nextHopInfoPtr           - (pointer to) the route entry info associated with
*                                      this UC prefix, if found.
* @param[out] tcamRowIndexPtr          - (pointer to) the TCAM row index of this UC
*                                      prefix, if found.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamColumnIndexPtr       - (pointer to) the TCAM column index of this UC
*                                      prefix, if found.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChIpLpmIpv4UcPrefixGet
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_IPADDR                               *ipAddrPtr,
    OUT GT_U32                                  *prefixLenPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT GT_U32                                  *tcamRowIndexPtr,
    OUT GT_U32                                  *tcamColumnIndexPtr
);

/**
* @internal cpssDxChIpLpmIpv4McEntryAdd function
* @endinternal
*
* @brief   Add IP multicast route for a particular/all source and a particular
*         group address. this is done for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual private network identifier.(APPLICABLE RANGES: 0..4095)
* @param[in] ipGroupPtr               - (pointer to) the IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrcPtr                 - (pointer to) the root address for source base multi tree
*                                      protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
* @param[in] mcRouteLttEntryPtr       - (pointer to) the LTT entry pointing to the MC
*                                      route entry associated with this MC route.
* @param[in] override                 - whether to  an mc Route pointer for the
*                                      given prefix
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
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
GT_STATUS cpssDxChIpLpmIpv4McEntryAdd
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPADDR                    *ipGroupPtr,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPADDR                    *ipSrcPtr,
    IN GT_U32                       ipSrcPrefixLen,
    IN CPSS_DXCH_IP_LTT_ENTRY_STC   *mcRouteLttEntryPtr,
    IN GT_BOOL                      override,
    IN GT_BOOL                      defragmentationEnable
);

/**
* @internal cpssDxChIpLpmIpv4McEntryDel function
* @endinternal
*
* @brief   To delete a particular mc route entry for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.(APPLICABLE RANGES: 0..4095)
* @param[in] ipGroupPtr               - (pointer to) the IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrcPtr                 - (pointer to) the root address for source base multi tree protocol.
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
GT_STATUS cpssDxChIpLpmIpv4McEntryDel
(
    IN GT_U32       lpmDBId,
    IN GT_U32       vrId,
    IN GT_IPADDR    *ipGroupPtr,
    IN GT_U32       ipGroupPrefixLen,
    IN GT_IPADDR    *ipSrcPtr,
    IN GT_U32       ipSrcPrefixLen
);

/**
* @internal cpssDxChIpLpmIpv4McEntriesFlush function
* @endinternal
*
* @brief   Flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.(APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmIpv4McEntriesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
);

/**
* @internal cpssDxChIpLpmIpv4McEntryGetNext function
* @endinternal
*
* @brief   This function returns the next multicast (ipSrc, ipGroup) entry, used
*         to iterate over the existing multicast addresses for a specific LPM DB,
*         and ipSrc + ipGroup.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - the LPM DB id.
* @param[in] vrId                     - the virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in,out] ipGroupPtr               - (pointer to) the ip Group address to get the next entry for.
* @param[in,out] ipGroupPrefixLenPtr      - (pointer to) the ipGroup prefix Length.
* @param[in,out] ipSrcPtr                 - (pointer to) the ip Source address to get the next
*                                      entry for.
* @param[in,out] ipSrcPrefixLenPtr        - (pointer to) ipSrc prefix length.
* @param[in,out] ipGroupPtr               - (pointer to) the next ip Group address
* @param[in,out] ipGroupPrefixLenPtr      - (pointer to) the ipGroup prefix Length
* @param[in,out] ipSrcPtr                 - (pointer to) the next ip Source address
* @param[in,out] ipSrcPrefixLenPtr        - (pointer to) ipSrc prefix length
*
* @param[out] mcRouteLttEntryPtr       - (pointer to) the LTT entry pointer pointing
*                                      to the MC route.
*                                      entry associated with this MC route.
* @param[out] tcamGroupRowIndexPtr     - (pointer to) TCAM group row  index.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamGroupColumnIndexPtr  - (pointer to) TCAM group column  index.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamSrcRowIndexPtr       - (pointer to) TCAM source row  index.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamSrcColumnIndexPtr    - (pointer to) TCAM source column  index.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if one of prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The value of ipGroupPtr must be a valid value, it
*       means that it exists in the IP-Mc Table, unless this is the first
*       call to this function, then it's value is 0.
*
*/
GT_STATUS cpssDxChIpLpmIpv4McEntryGetNext
(
    IN    GT_U32                        lpmDBId,
    IN    GT_U32                        vrId,
    INOUT GT_IPADDR                     *ipGroupPtr,
    INOUT GT_U32                        *ipGroupPrefixLenPtr,
    INOUT GT_IPADDR                     *ipSrcPtr,
    INOUT GT_U32                        *ipSrcPrefixLenPtr,
    OUT   CPSS_DXCH_IP_LTT_ENTRY_STC    *mcRouteLttEntryPtr,
    OUT GT_U32                          *tcamGroupRowIndexPtr,
    OUT GT_U32                          *tcamGroupColumnIndexPtr,
    OUT GT_U32                          *tcamSrcRowIndexPtr,
    OUT GT_U32                          *tcamSrcColumnIndexPtr
);


/**
* @internal cpssDxChIpLpmIpv4McEntrySearch function
* @endinternal
*
* @brief   This function returns the multicast (ipSrc, ipGroup) entry, used
*         to find specific multicast adrress entry, and ipSrc + ipGroup
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipGroupPtr               - (pointer to) the ip Group address to get the next entry for.
* @param[in] ipGroupPrefixLen         - The ip Group prefix len.
* @param[in] ipSrcPtr                 - (pointer to) the ip Source address to get the next entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteLttEntryPtr       - (pointer to) the LTT entry pointer pointing
*                                      to the MC route entry associated with this
*                                      MC route.
* @param[out] tcamGroupRowIndexPtr     - (pointer to) TCAM group row  index.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamGroupColumnIndexPtr  - (pointer to) TCAM group column  index.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamSrcRowIndexPtr       - (pointer to) TCAM source row  index.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamSrcColumnIndexPtr    - (pointer to) TCAM source column  index.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChIpLpmIpv4McEntrySearch
(
    IN  GT_U32                      lpmDBId,
    IN  GT_U32                      vrId,
    IN  GT_IPADDR                   *ipGroupPtr,
    IN  GT_U32                      ipGroupPrefixLen,
    IN  GT_IPADDR                   *ipSrcPtr,
    IN  GT_U32                      ipSrcPrefixLen,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC  *mcRouteLttEntryPtr,
    OUT GT_U32                      *tcamGroupRowIndexPtr,
    OUT GT_U32                      *tcamGroupColumnIndexPtr,
    OUT GT_U32                      *tcamSrcRowIndexPtr,
    OUT GT_U32                      *tcamSrcColumnIndexPtr
);

/**
* @internal cpssDxChIpLpmIpv6UcPrefixAdd function
* @endinternal
*
* @brief   Creates a new or override an existing Ipv6 prefix in a Virtual Router
*         for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddrPtr                - (pointer to) the destination IP address of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
* @param[in] nextHopInfoPtr           - (pointer to) the route entry info accosiated with this
*                                      UC prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - wether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
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
GT_STATUS cpssDxChIpLpmIpv6UcPrefixAdd
(
    IN GT_U32                                   lpmDBId,
    IN GT_U32                                   vrId,
    IN GT_IPV6ADDR                              *ipAddrPtr,
    IN GT_U32                                   prefixLen,
    IN CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT   *nextHopInfoPtr,
    IN GT_BOOL                                  override,
    IN GT_BOOL                                  defragmentationEnable
);

/**
* @internal cpssDxChIpLpmIpv6UcPrefixAddBulk function
* @endinternal
*
* @brief   Creates a new or override an existing bulk of Ipv6 prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - the LPM DB id
* @param[in] ipv6PrefixArrayLen       - length of UC prefix array
* @param[in] ipv6PrefixArrayPtr       - (pointer to) the UC prefix array
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - on one of prefixes' lengths is too big.
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*/
GT_STATUS cpssDxChIpLpmIpv6UcPrefixAddBulk
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC  *ipv6PrefixArrayPtr,
    IN GT_BOOL                              defragmentationEnable
);


/**
* @internal cpssDxChIpLpmIpv6UcPrefixDel function
* @endinternal
*
* @brief   Deletes an existing Ipv6 prefix in a Virtual Router for the specified LPM
*         DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddrPtr                - (pointer to) the destination IP address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NO_SUCH               - If the given prefix doesn't exist in the VR, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note the default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS cpssDxChIpLpmIpv6UcPrefixDel
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               vrId,
    IN GT_IPV6ADDR                          *ipAddrPtr,
    IN GT_U32                               prefixLen
);

/**
* @internal cpssDxChIpLpmIpv6UcPrefixDelBulk function
* @endinternal
*
* @brief   Deletes an existing bulk of Ipv6 prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - the LPM DB id
* @param[in] ipv6PrefixArrayLen       - length of UC prefix array
* @param[in] ipv6PrefixArrayPtr       - (pointer to) the UC prefix array
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
GT_STATUS cpssDxChIpLpmIpv6UcPrefixDelBulk
(
    IN GT_U32                               lpmDBId,
    IN GT_U32                               ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC  *ipv6PrefixArrayPtr
);


/**
* @internal cpssDxChIpLpmIpv6UcPrefixesFlush function
* @endinternal
*
* @brief   Flushes the unicast IPv6 Routing table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.(APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmIpv6UcPrefixesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
);


/**
* @internal cpssDxChIpLpmIpv6UcPrefixSearch function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the next
*         hop pointer associated with it and TCAM prefix index.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddrPtr                - (pointer to) the destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
*                                      ipAddr.
*
* @param[out] nextHopInfoPtr           - (pointer to) the route entry info associated with
*                                      this UC prefix, if found.
* @param[out] tcamRowIndexPtr          - (pointer to) the TCAM row index of this uc prefix, if
*                                      found
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamColumnIndexPtr       - (pointer to) the TCAM column index of this uc
*                                      prefix, if found.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChIpLpmIpv6UcPrefixSearch
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_IPV6ADDR                             *ipAddrPtr,
    IN  GT_U32                                  prefixLen,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT GT_U32                                  *tcamRowIndexPtr,
    OUT GT_U32                                  *tcamColumnIndexPtr
);

/**
* @internal cpssDxChIpLpmIpv6UcPrefixGetNext function
* @endinternal
*
* @brief   This function returns an IP-Unicast prefix with larger (ip,prefix) than
*         the given one; it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id to get the entry from.(APPLICABLE RANGES: 0..4095)
* @param[in,out] ipAddrPtr                - The ip address to start the search from.
* @param[in,out] prefixLenPtr             - Prefix length of ipAddr.
* @param[in,out] ipAddrPtr                - (pointer to) the ip address of the found entry
* @param[in,out] prefixLenPtr             - (pointer to) the prefix length of the found entry
*
* @param[out] nextHopInfoPtr           - (pointer to) the route entry info associated with
*                                      this UC prefix, if found.
* @param[out] tcamRowIndexPtr          - (pointer to) the TCAM row index of this uc prefix. if
*                                      found.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamColumnIndexPtr       - (pointer to) the TCAM column index of this uc
*                                      prefix, if found.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if no more entries are left in the IP table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChIpLpmIpv6UcPrefixGetNext
(
    IN    GT_U32                                    lpmDBId,
    IN    GT_U32                                    vrId,
    INOUT GT_IPV6ADDR                               *ipAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *nextHopInfoPtr,
    OUT GT_U32                                      *tcamRowIndexPtr,
    OUT GT_U32                                      *tcamColumnIndexPtr
);

/**
* @internal cpssDxChIpLpmIpv6UcPrefixGet function
* @endinternal
*
* @brief   This function gets a given ip address, find LPM match in the trie and
*         returns the prefix length and pointer to the next hop information bound
*         to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddrPtr                - (pointer to) the destination IP address to look for.
*
* @param[out] prefixLenPtr             - (pointer to) the prefix length of the found entry
* @param[out] nextHopInfoPtr           - (pointer to) the route entry info associated with
*                                      this UC prefix, if found
* @param[out] tcamRowIndexPtr          - (pointer to) the TCAM row index of this uc prefix, if
*                                      found
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamColumnIndexPtr       - (pointer to) the TCAM column index of this uc
*                                      prefix, if found
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChIpLpmIpv6UcPrefixGet
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_IPV6ADDR                             *ipAddrPtr,
    OUT GT_U32                                  *prefixLenPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    OUT GT_U32                                  *tcamRowIndexPtr,
    OUT GT_U32                                  *tcamColumnIndexPtr
);

/**
* @internal cpssDxChIpLpmIpv6McEntryAdd function
* @endinternal
*
* @brief   To add the multicast routing information for IP datagrams from a particular
*         source and addressed to a particular IP multicast group address for a
*         specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual private network identifier.(APPLICABLE RANGES: 0..4095)
* @param[in] ipGroupPtr               - (pointer to) the IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrcPtr                 - (pointer to) the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
* @param[in] mcRouteLttEntryPtr       - (pointer to) the LTT entry pointing to the MC route
*                                      entry associated with this MC route.
* @param[in] override                 - whether to  the mcRoutePointerPtr for the
*                                      given prefix
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
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
GT_STATUS cpssDxChIpLpmIpv6McEntryAdd
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPV6ADDR                  *ipGroupPtr,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPV6ADDR                  *ipSrcPtr,
    IN GT_U32                       ipSrcPrefixLen,
    IN CPSS_DXCH_IP_LTT_ENTRY_STC   *mcRouteLttEntryPtr,
    IN GT_BOOL                      override,
    IN GT_BOOL                      defragmentationEnable
);

/**
* @internal cpssDxChIpLpmIpv6McEntryDel function
* @endinternal
*
* @brief   To delete a particular mc route entry for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.(APPLICABLE RANGES: 0..4095)
* @param[in] ipGroupPtr               - (pointer to) the IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrcPtr                 - (pointer to) the root address for source base multi tree protocol.
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
GT_STATUS cpssDxChIpLpmIpv6McEntryDel
(
    IN GT_U32       lpmDBId,
    IN GT_U32       vrId,
    IN GT_IPV6ADDR  *ipGroupPtr,
    IN GT_U32       ipGroupPrefixLen,
    IN GT_IPV6ADDR  *ipSrcPtr,
    IN GT_U32       ipSrcPrefixLen
);

/**
* @internal cpssDxChIpLpmIpv6McEntriesFlush function
* @endinternal
*
* @brief   Flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.(APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmIpv6McEntriesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
);

/**
* @internal cpssDxChIpLpmIpv6McEntryGetNext function
* @endinternal
*
* @brief   This function returns the next multicast (ipSrc, ipGroup) entry, used
*         to iterate over the existing multicast addresses for a specific LPM DB,
*         and ipSrc + ipGroup.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - the LPM DB id
* @param[in] vrId                     - the virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in,out] ipGroupPtr               - (pointer to) the ip Group address to get the next
*                                      entry for
* @param[in,out] ipGroupPrefixLenPtr      - (pointer to) the ipGroup prefix length
* @param[in,out] ipSrcPtr                 - (pointer to) the ip Source address to get the next entry for
* @param[in,out] ipSrcPrefixLenPtr        - (pointer to) the ipSrc prefix length
* @param[in,out] ipGroupPtr               - (pointer to) the next ip Group address.
* @param[in,out] ipGroupPrefixLenPtr      - (pointer to) ipGroup prefix length.
* @param[in,out] ipSrcPtr                 - (pointer to) the next ip Source address.
* @param[in,out] ipSrcPrefixLenPtr        - (pointer to) ipSrc prefix length.
*
* @param[out] mcRouteLttEntryPtr       - (pointer to) the LTT entry pointer pointing to
*                                      the MC route entry associated with this MC route
* @param[out] tcamGroupRowIndexPtr     - (pointer to) TCAM group row  index
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamSrcRowIndexPtr       - (pointer to) TCAM source row  index.
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if one of prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The values of (ipGroupPtr,ipGroupPrefixLenPtr) must be a valid
*       values, it means that they exist in the IP-Mc Table, unless this is
*       the first call to this function, then the value of (ipGroupPtr,
*       ipSrcPtr) is (0,0).
*
*/
GT_STATUS cpssDxChIpLpmIpv6McEntryGetNext
(
    IN    GT_U32                      lpmDBId,
    IN    GT_U32                      vrId,
    INOUT GT_IPV6ADDR                 *ipGroupPtr,
    INOUT GT_U32                      *ipGroupPrefixLenPtr,
    INOUT GT_IPV6ADDR                 *ipSrcPtr,
    INOUT GT_U32                      *ipSrcPrefixLenPtr,
    OUT   CPSS_DXCH_IP_LTT_ENTRY_STC  *mcRouteLttEntryPtr,
    OUT GT_U32                        *tcamGroupRowIndexPtr,
    OUT GT_U32                        *tcamSrcRowIndexPtr
);

/**
* @internal cpssDxChIpLpmIpv6McEntrySearch function
* @endinternal
*
* @brief   This function returns the multicast (ipSrc, ipGroup) entry, used to find
*         specific multicast adrress entry, and ipSrc + ipGroup.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipGroupPtr               - (pointer to) the ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrcPtr                 - (pointer to) the ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteLttEntryPtr       - (pointer to) the LTT entry pointer pointing to
*                                      the MC route entry associated with this MC route
* @param[out] tcamGroupRowIndexPtr     - (pointer to) TCAM group row  index
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
* @param[out] tcamSrcRowIndexPtr       - (pointer to) TCAM source row  index
*                                      (APPLICABLE DEVICES: xCat3; Lion2)
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if one of prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChIpLpmIpv6McEntrySearch
(
    IN  GT_U32                      lpmDBId,
    IN  GT_U32                      vrId,
    IN  GT_IPV6ADDR                 *ipGroupPtr,
    IN  GT_U32                      ipGroupPrefixLen,
    IN  GT_IPV6ADDR                 *ipSrcPtr,
    IN  GT_U32                      ipSrcPrefixLen,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC  *mcRouteLttEntryPtr,
    OUT GT_U32                      *tcamGroupRowIndexPtr,
    OUT GT_U32                      *tcamSrcRowIndexPtr
);


/**
* @internal cpssDxChIpLpmFcoePrefixAdd function
* @endinternal
*
* @brief   This function adds a new FCoE prefix to a Virtual Router in a
*         specific LPM DB or overrides an existing FCoE prefix.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_ALREADY_EXIST         - prefix already exist when override is GT_FALSE
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS cpssDxChIpLpmFcoePrefixAdd
(
    IN  GT_U32                                      lpmDBId,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     *fcoeAddrPtr,
    IN  GT_U32                                      prefixLen,
    IN CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT       *nextHopInfoPtr,
    IN  GT_BOOL                                     override,
    IN GT_BOOL                                      defragmentationEnable
);

/**
* @internal cpssDxChIpLpmFcoePrefixDel function
* @endinternal
*
* @brief   Deletes an existing FCoE prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
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
GT_STATUS cpssDxChIpLpmFcoePrefixDel
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_FCID                                 *fcoeAddrPtr,
    IN  GT_U32                                  prefixLen
);

/**
* @internal cpssDxChIpLpmFcoePrefixAddBulk function
* @endinternal
*
* @brief   Creates a new or override an existing bulk of FCoE prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmFcoePrefixAddBulk
(
    IN  GT_U32                                          lpmDBId,
    IN  GT_U32                                          fcoeAddrPrefixArrayLen,
    IN  CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    *fcoeAddrPrefixArrayPtr
);

/**
* @internal cpssDxChIpLpmFcoePrefixDelBulk function
* @endinternal
*
* @brief   Deletes an existing bulk of FCoE prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
*
* @retval GT_OK                    - if all prefixes were successfully deleted
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_FAIL                  - if deleting one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmFcoePrefixDelBulk
(
    IN  GT_U32                                          lpmDBId,
    IN  GT_U32                                          fcoeAddrPrefixArrayLen,
    IN  CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    *fcoeAddrPrefixArrayPtr
);

/**
* @internal cpssDxChIpLpmFcoePrefixesFlush function
* @endinternal
*
* @brief   Flushes the FCoE forwarding table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmFcoePrefixesFlush
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                vrId
);
/**
* @internal cpssDxChIpLpmFcoePrefixSearch function
* @endinternal
*
* @brief   This function searches for a given FC_ID, and returns the next
*         hop pointer information bound to it if found.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
*
* @param[out] nextHopInfoPtr           - If found this is The next hop pointer to for this
*                                      prefix.
*
* @retval GT_OK                    - the required entry was found, or
* @retval GT_OUT_OF_RANGE          - the prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - the given prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmFcoePrefixSearch
(
    IN  GT_U32                                      lpmDBId,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                    *fcoeAddrPtr,
    IN  GT_U32                                      prefixLen,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT     *nextHopInfoPtr
);

/**
* @internal cpssDxChIpLpmFcoePrefixGet function
* @endinternal
*
* @brief   This function gets a given FC_ID address, and returns the next
*         hop pointer information bounded to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopInfoPtr           - The next hop pointer bound to the longest match
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmFcoePrefixGet
(
    IN  GT_U32                                      lpmDBId,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     *fcoeAddrPtr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT      *nextHopInfoPtr
);

/**
* @internal cpssDxChIpLpmFcoePrefixGetNext function
* @endinternal
*
* @brief   This function returns FCoE prefix with larger (FC_ID,prefix)
*         than the given one, it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in,out] fcoeAddrPtr              - The FC_ID of the found entry.
* @param[in,out] prefixLenPtr             - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - the next hop pointer associated with the found FC_ID
*
* @retval GT_OK                    - the required entry was found, or
* @retval GT_OUT_OF_RANGE          - the prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - no more entries are left in the FC_ID table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The values of (FC_ID,prefix) must be a valid values, it
*       means that they exist in the forwarding Table, unless this is the
*       first call to this function, then the value of (FC_ID,prefix) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the FC_ID
*       prefix get function.
*
*/
GT_STATUS cpssDxChIpLpmFcoePrefixGetNext
(
    IN    GT_U32                                    lpmDBId,
    IN    GT_U32                                    vrId,
    INOUT GT_FCID                                   *fcoeAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *nextHopInfoPtr
);

/**
* @internal cpssDxChIpLpmDBMemSizeGet function
* @endinternal
*
* @brief   This function gets the memory size needed to export the Lpm DB,
*         (used for HSU and unit Hotsync)
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
*
* @param[out] lpmDbSizePtr             - (pointer to) the table size calculated (in bytes)
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS cpssDxChIpLpmDBMemSizeGet
(
    IN    GT_U32     lpmDBId,
    OUT   GT_U32     *lpmDbSizePtr
);

/**
* @internal cpssDxChIpLpmDBExport function
* @endinternal
*
* @brief   This function exports the Lpm DB into the preallocated memory,
*         (used for HSU and unit Hotsync)
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in,out] lpmDbMemBlockSizePtr     - (pointer to) block data size supposed to be
*                                      exported.
*                                      in current iteration.
* @param[in,out] iterPtr                  - the iterator, to start - set to 0.
* @param[in,out] lpmDbMemBlockSizePtr     - (pointer to) block data size that was not used.
*                                      in current iteration.
*
* @param[out] lpmDbMemBlockPtr         - (pointer to) allocated for lpm DB memory area.
* @param[in,out] iterPtr                  - (pointer to) the iterator, if = 0 then the
*                                      operation is done.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS cpssDxChIpLpmDBExport
(
    IN    GT_U32     lpmDBId,
    OUT   GT_U32     *lpmDbMemBlockPtr,
    INOUT GT_U32     *lpmDbMemBlockSizePtr,
    INOUT GT_UINTPTR *iterPtr
);

/**
* @internal cpssDxChIpLpmDBImport function
* @endinternal
*
* @brief   This function imports the Lpm DB recived and reconstruct it,
*         (used for HSU and unit Hotsync)
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in,out] lpmDbMemBlockSizePtr     - (pointer to) block data size supposed to be
*                                      imported.
*                                      in current iteration.
* @param[in] lpmDbMemBlockPtr         - (pointer to) allocated for lpm DB memory area.
* @param[in,out] iterPtr                  - (pointer to) the iterator, to start - set to 0.
* @param[in,out] lpmDbMemBlockSizePtr     - (pointer to) block data size that was not used.
*                                      in current iteration.
* @param[in,out] iterPtr                  - (pointer to) the iterator, if = 0 then the
*                                      operation is done.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS cpssDxChIpLpmDBImport
(
    IN    GT_U32     lpmDBId,
    IN    GT_U32     *lpmDbMemBlockPtr,
    INOUT GT_U32     *lpmDbMemBlockSizePtr,
    INOUT GT_UINTPTR *iterPtr
);

/**
* @internal cpssDxChIpLpmIpv4UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 UC prefix for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
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
* @retval GT_FAIL                  - otherwise.
*
* @note Only 32 bit prefixes supported.
*
*/
GT_STATUS cpssDxChIpLpmIpv4UcPrefixActivityStatusGet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *ipAddrPtr,
    IN  GT_U32      prefixLen,
    IN  GT_BOOL     clearActivity,
    OUT GT_BOOL     *activityStatusPtr
);

/**
* @internal cpssDxChIpLpmIpv4McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv4 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
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
* @retval GT_FAIL                  - otherwise.
*
* @note Only 32 bit prefixes supported.
*
*/
GT_STATUS cpssDxChIpLpmIpv4McEntryActivityStatusGet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   *ipGroupPtr,
    IN  GT_U32      ipGroupPrefixLen,
    IN  GT_IPADDR   *ipSrcPtr,
    IN  GT_U32      ipSrcPrefixLen,
    IN  GT_BOOL     clearActivity,
    OUT GT_BOOL     *activityStatusPtr
);

/**
* @internal cpssDxChIpLpmIpv6UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 UC prefix for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
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
* @retval GT_FAIL                  - otherwise.
*
* @note Only 128 bit prefixes supported.
*
*/
GT_STATUS cpssDxChIpLpmIpv6UcPrefixActivityStatusGet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *ipAddrPtr,
    IN  GT_U32      prefixLen,
    IN  GT_BOOL     clearActivity,
    OUT GT_BOOL     *activityStatusPtr
);

/**
* @internal cpssDxChIpLpmIpv6McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
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
* @retval GT_FAIL                  - otherwise.
*
* @note Only 128 bit prefixes supported.
*
*/
GT_STATUS cpssDxChIpLpmIpv6McEntryActivityStatusGet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR *ipGroupPtr,
    IN  GT_U32      ipGroupPrefixLen,
    IN  GT_IPV6ADDR *ipSrcPtr,
    IN  GT_U32      ipSrcPrefixLen,
    IN  GT_BOOL     clearActivity,
    OUT GT_BOOL     *activityStatusPtr
);

/**
* @internal cpssDxChIpLpmActivityBitEnableGet function
* @endinternal
*
* @brief   Get status of the aging/refresh mechanism of trie leaf entries in the
*         LPM memory.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - The device number.
*
* @param[out] activityBitPtr           - (pointer to) the aging/refresh mechanism of trie
*                                      leaf entries in the LPM memory:
*                                      GT_TRUE  - enabled.
*                                      GT_FALSE - disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChIpLpmActivityBitEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *activityBitPtr
);

/**
* @internal cpssDxChIpLpmActivityBitEnableSet function
* @endinternal
*
* @brief   Set status of the aging/refresh mechanism of trie leaf entries in the
*         LPM memory.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - The device number.
* @param[in] activityBit              - the aging/refresh mechanism of trie leaf entries in the
*                                      LPM memory:
*                                      GT_TRUE  - enabled.
*                                      GT_FALSE - disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChIpLpmActivityBitEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     activityBit
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpLpmh */

