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
* @file prvCpssDxChLpmRam.h
*
* @brief Private CPSS DXCH LPM RAM functions
*
* @version   5
********************************************************************************
*/
#ifndef __prvCpssDxChLpmRamh
#define __prvCpssDxChLpmRamh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h>

/* max number of virtual routers */
#define PRV_CPSS_DXCH_LPM_RAM_NUM_OF_VIRTUAL_ROUTERS_CNS     4096

/* set bit in the octet_to_block bitmap */
#define PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadow,protocol,octet,block)\
    (shadowPtr->lpmRamOctetsToBlockMappingPtr[block].octetsToBlockMappingBitmap[protocol] |= (1<<octet))

/* clear bit in the octet_to_block bitmap */
#define PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadow,protocol,octet,block)\
    (shadowPtr->lpmRamOctetsToBlockMappingPtr[block].octetsToBlockMappingBitmap[protocol] &= ~(1<<octet))

/* clear bit in the temp_octet_to_block bitmap used for reconstruct */
#define PRV_CPSS_DXCH_TEMP_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadow,protocol,octet,block)\
    (shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr[block].octetsToBlockMappingBitmap[protocol] &= ~(1<<octet))

/* get whether the block is used for the given protocol by a specific octet */
#define PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_OCTET_IN_PROTOCOL_GET_MAC(shadow,protocol,octet,block)\
    (((shadowPtr->lpmRamOctetsToBlockMappingPtr[block].octetsToBlockMappingBitmap[protocol]&(1<<octet))!=0)?GT_TRUE:GT_FALSE)

/* get whether the block is used for the given protocol by any octet */
#define PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadow,protocol,block)\
      ((shadowPtr->lpmRamOctetsToBlockMappingPtr[block].octetsToBlockMappingBitmap[protocol]==0)?GT_FALSE:GT_TRUE)

/* the LPM RAM total blocks size including the gap */
/*#define PRV_CPSS_DXCH_LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_CNS _16KB*/


/**
* @internal prvCpssDxChLpmRamMemFreeListMng function
* @endinternal
*
* @brief   This function is used to collect all Pp Narrow Sram memory free
*         operations inorder to be freed at the end of these operations.
* @param[in] ppMemAddr                - the address in the HW (the device memory pool) to record.
* @param[in] operation                - the  (see
*                                      PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
*                                      PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
*                                      PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E
* @param[in] memBlockListPtrPtr       - (pointer to) the memory block list to act upon.
* @param[in] shadowPtr                - (pointer to) the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMemFreeListMng
(
    IN  GT_UINTPTR                                  ppMemAddr,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_OP_ENT       operation,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC    **memBlockListPtrPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr
);

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC
 *
 * Description: A struct that holds the parameters for update of the VRF table
 *
 * Fields:
 *      vrId      - the virtual router/fabric id
 *      protocol  - the protocol to update
 *      shadowPtr - the LPM shadow to update
 */
typedef struct _writeVRFTable2DeviceRAMFuncParams
{
    IN GT_U32                                       vrId;
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol;
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr;
}PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC
 *
 * Description: A struct that holds the parameters for update node ranges and pointer
 *
 * Fields:
 *      pRange     - the range pointed to the shadow of going to be fixed node
 *      bucketPtr  - bucket of previous level 
 *      shadowPtr  - the LPM shadow to update
 *      isRootNode - if this is root node
 */
typedef struct 
{
     PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC       *pRange;
     PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC      *bucketPtr;
     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr;
     GT_BOOL                                      isRootNode;
}PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC;

/**
* @internal prvCpssDxChLpmRamDbCreate function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         LPM DB is a database that holds LPM shadow information that is shared
*         to one PP or more. The LPM DB manage adding, deleting and searching
*         This function creates LPM DB for a shared LPM managment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] shadowType               - the type of shadow to maintain
* @param[in] lpmDbPtrPtr              - the LPM DB
* @param[in] protocolBitmap           - the protocols that are used in this LPM DB
* @param[in] memoryCfgPtr             - (pointer to) the memory configuration of this LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_OUT_OF_CPU_MEM        - on failure to allocate memory
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChLpmRamDbCreate
(
    IN PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT     shadowType,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  **lpmDbPtrPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP        protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC      *memoryCfgPtr

);

/**
* @internal prvCpssDxChLpmRamDbDelete function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         LPM DB is a database that holds LPM shadow information that is shared
*         to one PP or more. The LPM DB manage adding, deleting and searching
*         IPv4/6 UC/MC and FCoE prefixes.
*         This function deletes LPM DB for a shared LPM managment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] shadowType               - the shadow type 
*  
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_OUT_OF_CPU_MEM        - on failure to allocate memory
* @retval GT_FAIL                  - on error
*
* @note Deleting LPM DB is allowed only if no VR present in the LPM DB (since
*       prefixes reside within VR it means that no prefixes present as well).
*
*/
GT_STATUS prvCpssDxChLpmRamDbDelete
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT     shadowType
);

/**
* @internal prvCpssDxChLpmRamDbConfigGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         LPM DB is a database that holds LPM shadow information that is shared
*         to one PP or more. The LPM DB manages adding, deleting and searching
*         IPv4/6 UC/MC and FCoE prefixes.
*         This function retrieves configuration of LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - (pointer to) the LPM DB
*
* @param[out] protocolBitmapPtr        - (pointer to) the protocols that are used in this LPM DB
* @param[out] memoryCfgPtr             - (pointer to) the memory configuration of this LPM DB
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamDbConfigGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC    *lpmDbPtr,
    OUT PRV_CPSS_DXCH_LPM_PROTOCOL_BMP          *protocolBitmapPtr,
    OUT PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC        *memoryCfgPtr
);

/**
* @internal prvCpssDxChLpmRamDbDevListAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds list of devices to an existing LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] devListArray[]           - array of device ids to add to the LPM DB
* @param[in] numOfDevs                - the number of device ids in the array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbDevListAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U8                                 devListArray[],
    IN GT_U32                                numOfDevs
);

/**
* @internal prvCpssDxChLpmRamDbDevListRemove function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function removes devices from an existing LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] devListArray[]           - array of device ids to remove from the LPM DB
* @param[in] numOfDevs                - the number of device ids in the array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChLpmRamDbDevListRemove
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U8                                 devListArray[],
    IN GT_U32                                numOfDevs
);

/**
* @internal prvCpssDxChLpmRamDbDevListGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function retrieves the list of devices in an existing LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - the LPM DB
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
GT_STATUS prvCpssDxChLpmRamDbDevListGet
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    INOUT GT_U32                                *numOfDevsPtr,
    OUT   GT_U8                                 devListArray[]
);

/**
* @internal prvCpssDxChLpmRamVirtualRouterAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds a virtual router for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] vrId                     - The virtual's router ID.
* @param[in] vrConfigPtr              - Configuration of the virtual router
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_ALREADY_EXIST         - if the vr id is already used
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamVirtualRouterAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId,
    IN PRV_CPSS_DXCH_LPM_RAM_VR_CONFIG_STC   *vrConfigPtr
);

/**
* @internal prvCpssDxChLpmRamVirtualRouterDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function deletes a virtual router for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] vrId                     - The virtual's router ID.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_FOUND             - if the LPM DB id or vr id does not found
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_BAD_STATE             - if the existing VR is not empty.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All prefixes must be previously deleted.
*
*/
GT_STATUS prvCpssDxChLpmRamVirtualRouterDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId
);

/**
* @internal prvCpssDxChLpmRamVirtualRouterGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets configuration of a virtual router for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - the LPM DB.
* @param[in] vrId                     - The virtual's router ID.
*
* @param[out] vrConfigPtr              - Configuration of the virtual router
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_FOUND             - if the LPM DB id or vr id does not found
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - if the existing VR is not empty.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamVirtualRouterGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN  GT_U32                               vrId,
    OUT PRV_CPSS_DXCH_LPM_RAM_VR_CONFIG_STC  *vrConfigPtr
);

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds a new IPv4 prefix to a Virtual Router in a
*         specific LPM DB or overrides an existing existing IPv4 prefix.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
* @param[in] nextHopInfoPtr           - (points to) The next hop pointer to set for this prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
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
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPADDR                                    ipAddr,
    IN GT_U32                                       prefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *nextHopInfoPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
);

/**
* @internal prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList function
* @endinternal
*
* @brief   Update a summary list of all pending block values to be used at
*         the end of bulk operation
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] totalPendingBlockToUpdateArr[] - array holding all updates done until now
* @param[in] pendingBlockToUpdateArr[] - array of new updates to be kept in totalPendingBlockToUpdateArr
*
* @retval GT_OK                    - on succes
*/
GT_STATUS prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList
(
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC totalPendingBlockToUpdateArr[],
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC pendingBlockToUpdateArr[]
);

/**
* @internal prvCpssDxChLpmRamUpdateBlockUsageCounters function
* @endinternal
*
* @brief   Update block usage counters according to pending block values
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmRamBlocksSizeArrayPtr - used for finding lpm lines per block
* @param[in] totalPendingBlockToUpdateArr[] - array holding all updates done until now
* @param[in] protocol                 - counters should be updated for given protocol
* @param[in] resetPendingBlockToUpdateArr[] - array need to be reset
* @param[in] protocol                 - counters should be updated for given protocol
* @param[in] numOfLpmMemories         - number of LPM memories
*
* @retval GT_OK                    - on succes
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvCpssDxChLpmRamUpdateBlockUsageCounters
(
    IN GT_U32                                            *lpmRamBlocksSizeArrayPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC totalPendingBlockToUpdateArr[],
    IN PRV_CPSS_DXCH_LPM_ADDRESS_COUNTERS_INFO_STC       protocolCountersPerBlockArr[],
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC resetPendingBlockToUpdateArr[],
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT              protocol,
    IN GT_U32                                            numOfLpmMemories
);

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixBulkAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Creates a new or override an existing bulk of IPv4 prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv4PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv4PrefixArrayPtr       - The UC prefix array.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixBulkAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArrayPtr,
    IN GT_BOOL                              defragmentationEnable
);

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Deletes an existing IPv4 prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB.
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
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId,
    IN GT_IPADDR                             ipAddr,
    IN GT_U32                                prefixLen
);

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixBulkDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Deletes an existing bulk of Ipv4 prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv4PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv4PrefixArrayPtr       - The UC prefix array.
*
* @retval GT_OK                    - if all prefixes were successfully deleted
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_FAIL                  - if deleting one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixBulkDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArrayPtr
);

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixesFlush function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Flushes the unicast IPv4 Routing table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixesFlush
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId
);

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixSearch function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function searches for a given ip-uc address, and returns the next
*         hop pointer information bound to it if found.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - If found this is The next hop pointer to for this
*                                      prefix.
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixSearch
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPADDR                                   ipAddr,
    IN  GT_U32                                      prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
);

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixGetNext function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns an IP-Unicast prefix with larger (ipAddrPtr,prefixLenPtr)
*         than the given one, it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] ipAddrPtr                - The ip address to start the search from.
* @param[in,out] prefixLenPtr             - Prefix length of ipAddr.
* @param[in,out] ipAddrPtr                - The ip address of the found entry.
* @param[in,out] prefixLenPtr             - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - the next hop pointer associated with the found
*                                      ipAddr.
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if no more entries are left in the IP table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The values of (ipAddrPtr,prefixLenPtr) must be a valid values, it
*       means that they exist in the IP-UC Table, unless this is the first
*       call to this function, then the value of (ipAddrPtr,prefixLenPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv4
*       UC prefix get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixGetNext
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC          *lpmDbPtr,
    IN    GT_U32                                        vrId,
    INOUT GT_IPADDR                                     *ipAddrPtr,
    INOUT GT_U32                                        *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC     *nextHopInfoPtr
);

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets a given ip address, and returns the next
*         hop pointer information bounded to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopPointerPtr        - The next hop pointer bounded to the longest match
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPADDR                                   ipAddr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopPointerPtr
);

/**
* @internal prvCpssDxChLpmRamIpv4McEntryAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To add the multicast routing information for IP datagrams from
*         a particular source and addressed to a particular IP multicast
*         group address for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
* @param[in] mcRouteEntryPtr          - the mc Route pointer to set for the mc entry.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_ERROR                 - if the virtual router does not exist.
* @retval GT_NOT_FOUND             - prefix was not found when override is GT_TRUE
* @retval GT_ALREADY_EXIST         - prefix already exist when override is GT_FALSE
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To override the default mc route use ipGroup = 0.
*       2. If (S,G) MC group is added when (,G) doesn't exists then implicit
*       (,G) is added pointing to (,) default. Application added (,G)
*       will override the implicit (,G).
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
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
* @internal prvCpssDxChLpmRamIpv4McEntryDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To delete a particular mc route entry for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. In order to delete the multicast entry and all the src ip addresses
*       associated with it, call this function with ipSrc = ipSrcPrefixLen = 0.
*       2. If no (,G) was added but (S,G) were added, then implicit (,G)
*       that points to (,) is added. If (,G) is added later, it will
*       replace the implicit (,G).
*       When deleting (,G), then if there are still (S,G), an implicit (,G)
*       pointing to (,) will be added.
*       When deleting last (S,G) and the (,G) was implicitly added, then
*       the (,G) will be deleted as well.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPADDR                            ipGroup,
    IN GT_U32                               ipGroupPrefixLen,
    IN GT_IPADDR                            ipSrc,
    IN GT_U32                               ipSrcPrefixLen
);

/**
* @internal prvCpssDxChLpmRamIpv4McEntriesFlush function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntriesFlush
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId
);

/**
* @internal prvCpssDxChLpmRamIpv4McEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrc                    - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteEntryPtr          - the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device.
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPADDR                                   ipGroup,
    IN  GT_U32                                      ipGroupPrefixLen,
    IN  GT_IPADDR                                   ipSrc,
    IN  GT_U32                                      ipSrcPrefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *mcRouteEntryPtr
);

/**
* @internal prvCpssDxChLpmRamIpv4McEntryGetNext function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the next muticast (ipSrcPtr,ipGroupPtr) entry, used
*         to iterate over the existing multicast addresses for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in,out] ipGroupPtr               - The ip Group address to get the next entry for.
* @param[in,out] ipGroupPrefixLenPtr      - ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - The ip Source address to get the next entry for.
* @param[in,out] ipSrcPrefixLenPtr        - ipSrcPtr prefix length.
* @param[in,out] ipGroupPtr               - The next ip Group address.
* @param[in,out] ipGroupPrefixLenPtr      - ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - The next ip Source address.
* @param[in,out] ipSrcPrefixLenPtr        - ipSrc prefix length.
*
* @param[out] mcRouteEntryPtr          - the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device.
*
* @note 1. The values of (ipSrcPtr,ipGroupPtr) must be a valid values, it
*       means that they exist in the IP-Mc Table, unless this is the first
*       call to this function, then the value of (ipSrcPtr,ipGroupPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv4
*       MC get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryGetNext
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_IPADDR                                 *ipGroupPtr,
    INOUT GT_U32                                    *ipGroupPrefixLenPtr,
    INOUT GT_IPADDR                                 *ipSrcPtr,
    INOUT GT_U32                                    *ipSrcPrefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcRouteEntryPtr
);

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         creates a new or override an existing Ipv6 prefix in a Virtual Router
*         for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - Points to the destination IP address of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
* @param[in] nextHopInfoPtr           - Points to the next hop pointer to set for this prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the vrId was not created yet, or
* @retval GT_ALREADY_EXIST         - if prefix already exist when override is GT_FALSE, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPV6ADDR                                  ipAddr,
    IN GT_U32                                       prefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *nextHopInfoPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
);

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixBulkAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         creates a new or override an existing bulk of Ipv6 prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv6PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv6PrefixArrayPtr       - The UC prefix array.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixBulkAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC      *ipv6PrefixArrayPtr,
    IN GT_BOOL                                  defragmentationEnable
);

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         deletes an existing Ipv6 prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - Points to the destination IP address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the vrId was not created yet, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NO_SUCH               - if the given prefix doesn't exitst in the VR, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note The default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPV6ADDR                          ipAddr,
    IN GT_U32                               prefixLen
);

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixBulkDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         deletes an existing bulk of Ipv6 prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv6PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv6PrefixArrayPtr       - The UC prefix array.
*
* @retval GT_OK                    - if all prefixes were successfully deleted
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if deleting one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixBulkDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC      *ipv6PrefixArrayPtr
);

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixesFlush function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         flushes the unicast IPv6 Routing table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixesFlush
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId
);

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixSearch function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function searches for a given ip-uc address, and returns the next
*         hop pointer information bound to it if found.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - Points to the destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - If found this is The next hop pointer to for this
*                                      prefix.
*
* @retval GT_OK                    - if the required entry was found
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixSearch
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPV6ADDR                                 ipAddr,
    IN  GT_U32                                      prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
);

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixGetNext function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns an IP-Unicast prefix with larger (ipAddrPtr,prefixLenPtr)
*         than the given one, it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] ipAddrPtr                - The ip address to start the search from.
* @param[in,out] prefixLenPtr             - Prefix length of ipAddr.
* @param[in,out] ipAddrPtr                - The ip address of the found entry.
* @param[in,out] prefixLenPtr             - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - the next hop pointer associated with the found
*                                      ipAddr.
*
* @retval GT_OK                    - if the required entry was found
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if no more entries are left in the IP table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The values of (ipAddrPtr,prefixLenPtr) must be a valid values, it
*       means that they exist in the IP-UC Table, unless this is the first
*       call to this function, then the value of (ipAddrPtr,prefixLenPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv6
*       UC prefix get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixGetNext
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_IPV6ADDR                               *ipAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopInfoPtr
);

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets a given ip address, and returns the next
*         hop pointer information bounded to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopInfoPtr           - The next hop pointer bounded to the longest match
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPV6ADDR                                 ipAddr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
);

/**
* @internal prvCpssDxChLpmRamIpv6McEntryAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To add the multicast routing information for IP datagrams from
*         a particular source and addressed to a particular IP multicast
*         group address for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
* @param[in] mcRouteEntryPtr          - (pointer to) the mc Route pointer to set for the mc entry.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if prefix was not found when override is GT_TRUE, or
* @retval GT_ALREADY_EXIST         - if prefix already exist when override is GT_FALSE, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note 1. To override the default mc route use ipGroup = 0.
*       2. If (S,G) MC group is added when (,G) doesn't exists then implicit
*       (,G) is added pointing to (,) default. Application added (,G)
*       will override the implicit (,G).
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPV6ADDR                                  ipGroup,
    IN GT_U32                                       ipGroupPrefixLen,
    IN GT_IPV6ADDR                                  ipSrc,
    IN GT_U32                                       ipSrcPrefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *mcRouteEntryPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
);

/**
* @internal prvCpssDxChLpmRamIpv6McEntryDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To delete a particular mc route entry for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note 1. Inorder to delete the multicast entry and all the src ip addresses
*       associated with it, call this function with ipSrc = ipSrcPrefixLen = 0.
*       2. If no (,G) was added but (S,G) were added, then implicit (,G)
*       that points to (,) is added. If (,G) is added later, it will
*       replace the implicit (,G).
*       When deleting (,G), then if there are still (S,G), an implicit (,G)
*       pointing to (,) will be added.
*       When deleting last (S,G) and the (,G) was implicitly added, then
*       the (,G) will be deleted as well.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPV6ADDR                          ipGroup,
    IN GT_U32                               ipGroupPrefixLen,
    IN GT_IPV6ADDR                          ipSrc,
    IN GT_U32                               ipSrcPrefixLen
);

/**
* @internal prvCpssDxChLpmRamIpv6McEntriesFlush function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntriesFlush
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId
);

/**
* @internal prvCpssDxChLpmRamIpv6McEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrc                    - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteEntryPtr          - (pointer to) the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device.
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPV6ADDR                                 ipGroup,
    IN  GT_U32                                      ipGroupPrefixLen,
    IN  GT_IPV6ADDR                                 ipSrc,
    IN  GT_U32                                      ipSrcPrefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *mcRouteEntryPtr
);

/**
* @internal prvCpssDxChLpmRamIpv6McEntryGetNext function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the next muticast (ipSrcPtr,ipGroupPtr) entry, used
*         to iterate over the existing multicast addresses for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in,out] ipGroupPtr               - (pointer to) The ip Group address to get the next entry for.
* @param[in,out] ipGroupPrefixLenPtr      - (pointer to) the ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - (pointer to) The ip Source address to get the next entry for.
* @param[in,out] ipSrcPrefixLenPtr        - (pointer to) ipSrcPtr prefix length.
* @param[in,out] ipGroupPtr               - (pointer to) The next ip Group address.
* @param[in,out] ipGroupPrefixLenPtr      - (pointer to) the ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - (pointer to) The next ip Source address.
* @param[in,out] ipSrcPrefixLenPtr        - (pointer to) ipSrcPtr prefix length.
*
* @param[out] mcRouteEntryPtr          - (pointer to) the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
*
* @note 1. The values of (ipSrcPtr,ipGroupPtr) must be a valid values, it
*       means that they exist in the IP-Mc Table, unless this is the first
*       call to this function, then the value of (ipSrcPtr,ipGroupPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv6
*       MC get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryGetNext
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_IPV6ADDR                               *ipGroupPtr,
    INOUT GT_U32                                    *ipGroupPrefixLenPtr,
    INOUT GT_IPV6ADDR                               *ipSrcPtr,
    INOUT GT_U32                                    *ipSrcPrefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcRouteEntryPtr
);

/**
* @internal prvCpssDxChLpmRamFcoePrefixAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds a new FCoE prefix to a Virtual Router in a
*         specific LPM DB or overrides an existing existing FCoE prefix.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] fcid                     - The FC_ID of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the FC_ID.
* @param[in] nextHopInfoPtr           - (points to) The next hop pointer to set for this prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
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
GT_STATUS prvCpssDxChLpmRamFcoePrefixAdd
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcid,
    IN  GT_U32                                      prefixLen,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr,
    IN  GT_BOOL                                     override,
    IN  GT_BOOL                                     defragmentationEnable
);

/**
* @internal prvCpssDxChLpmRamFcoePrefixDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Deletes an existing FCoE prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB.
* @param[in] vrId                     - The virtual router id.
* @param[in] fcid                     - The destination FC_ID address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the FC_ID.
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
GT_STATUS prvCpssDxChLpmRamFcoePrefixDel
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC    *lpmDbPtr,
    IN  GT_U32                                  vrId,
    IN  GT_FCID                                 fcid,
    IN  GT_U32                                  prefixLen
);

/**
* @internal prvCpssDxChLpmRamFcoePrefixAddBulk function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Creates a new or override an existing bulk of FCoE prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] fcidPrefixArrayLen       - Length of FC_ID prefix array.
* @param[in] fcidPrefixArrayPtr       - The FC_ID prefix array.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixAddBulk
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC            *lpmDbPtr,
    IN  GT_U32                                          fcidPrefixArrayLen,
    IN  CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    *fcidPrefixArrayPtr,
    IN  GT_BOOL                                         defragmentationEnable
);

/**
* @internal prvCpssDxChLpmRamFcoePrefixDelBulk function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Deletes an existing bulk of FCoE prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] fcidPrefixArrayLen       - Length of FC_ID prefix array.
* @param[in] fcidPrefixArrayPtr       - The FC_ID prefix array.
*
* @retval GT_OK                    - if all prefixes were successfully deleted
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_FAIL                  - if deleting one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixDelBulk
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC            *lpmDbPtr,
    IN  GT_U32                                          fcidPrefixArrayLen,
    IN  CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    *fcidPrefixArrayPtr
);

/**
* @internal prvCpssDxChLpmRamFcoePrefixesFlush function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Flushes the FCoE forwarding table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixesFlush
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId
);

/**
* @internal prvCpssDxChLpmRamFcoePrefixSearch function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function searches for a given FC_ID, and returns the next
*         hop pointer information bound to it if found.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] fcid                     - The FC_ID to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
*                                      FC_ID.
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
GT_STATUS prvCpssDxChLpmRamFcoePrefixSearch
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcid,
    IN  GT_U32                                      prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
);

/**
* @internal prvCpssDxChLpmRamFcoePrefixGetNext function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns an FCoE prefix with larger (FC_ID,prefix)
*         than the given one, it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] fcidPtr                  - The FC_ID to start the search from.
* @param[in,out] prefixLenPtr             - The number of bits that are actual valid in the
*                                      FC_ID.
* @param[in,out] fcidPtr                  - The FC_ID of the found entry.
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
GT_STATUS prvCpssDxChLpmRamFcoePrefixGetNext
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_FCID                                   *fcidPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopInfoPtr
);

/**
* @internal prvCpssDxChLpmRamFcoePrefixGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets a given FC_ID address, and returns the next
*         hop pointer information bounded to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] fcid                     - The FC_ID to look for.
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
GT_STATUS prvCpssDxChLpmRamFcoePrefixGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcid,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
);

/**
* @internal prvCpssDxChLpmRamUpdateVrTableFuncWrapper function
* @endinternal
*
* @brief   This function is a wrapper to PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PTR
*
* @param[in] data                     - the parmeters for PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PTR
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUpdateVrTableFuncWrapper
(
    IN  GT_PTR                  data
);

/**
* @internal prvCpssDxChIpLpmRamIpv4UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 UC prefix for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
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
GT_STATUS prvCpssDxChIpLpmRamIpv4UcPrefixActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipPtr,
    IN  GT_U32                              prefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
);

/**
* @internal prvCpssDxChIpLpmRamIpv6UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 UC prefix for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
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
GT_STATUS prvCpssDxChIpLpmRamIpv6UcPrefixActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipPtr,
    IN  GT_U32                              prefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
);

/**
* @internal prvCpssDxChIpLpmRamIpv4McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv4 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipGroupPtr               - Pointer to the ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrcPtr                 - Pointer to the ip Group address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
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
GT_STATUS prvCpssDxChIpLpmRamIpv4McEntryActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipGroupPtr,
    IN  GT_U32                              ipGroupPrefixLen,
    IN  GT_U8                               *ipSrcPtr,
    IN  GT_U32                              ipSrcPrefixLen,
    IN  GT_BOOL                             clearActivity,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
);

/**
* @internal prvCpssDxChIpLpmRamIpv6McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
*                                      srcIpAddr           - The ip Source address to get the entry for.
* @param[in] srcPrefixLen             - srcIpAddr prefix length.
*                                      destIpAddr          - The ip Group address to get the entry for.
* @param[in] destPrefixLen            - destIpAddr prefix length.
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
GT_STATUS prvCpssDxChIpLpmRamIpv6McEntryActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *srcIpPtr,
    IN  GT_U32                              srcPrefixLen,
    IN  GT_U8                               *destIpPtr,
    IN  GT_U32                              destPrefixLen,
    IN  GT_BOOL                             clearActivity,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
);

/**
* @internal prvCpssDxChLpmRamSip6FillCfg function
* @endinternal
*
* @brief   This function retrieve next LPM DB ID from LPM DBs Skip List
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2. 
*
* @param[in] memoryCfgPtr        - provisioned LPM RAM configuration 
*
* @param[out] ramMemoryPtr       - calculated in lines memory ram configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamSip6FillCfg
(
    IN    CPSS_DXCH_LPM_RAM_CONFIG_STC      *memoryCfgPtr,
    OUT   PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC  *ramMemoryPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmRamh */


