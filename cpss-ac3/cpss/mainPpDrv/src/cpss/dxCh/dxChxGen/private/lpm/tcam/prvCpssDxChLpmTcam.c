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
* @file prvCpssDxChLpmTcam.c
*
* @brief the CPSS DXCH LPM Hierarchy manager .
*
* @version   56
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamMgm.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamPatTrie.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamIpUc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamIpMc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamEntriesHandler.h>
#include <cpss/dxCh/dxChxGen/resourceManager/cpssDxChTcamManager.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamDispatcher.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/private/lpm/prvCpssDxChLpmUtils.h>

/*******************************************************************************
 *  extern vars                                                                 *
 ******************************************************************************/
/* Pointers to dummy trie nodes to support policy based routing MC defaults */
GT_U32                                    pbrDefaultMcTrieNodePtrArraySize = 0;
PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC  **pbrDefaultMcTrieNodePtrArray = NULL;

/* number of rows in the TCAM */
extern GT_U32  tcamRows;

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
)
{
    return prvCpssDxChLpmTcamInit(shadowType, &(memoryCfgPtr->indexesRange),
                                  memoryCfgPtr->partitionEnable,
                                  &(memoryCfgPtr->tcamLpmManagerCapcityCfg),
                                  protocolStack,
                                  memoryCfgPtr->tcamManagerHandlerPtr, lpmDbPtrPtr);
}

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
)
{
    /* LPM DB can be deleted only if no VR present in the LPM DB */
    if (prvCpssSlIsEmpty(lpmDbPtr->vrSl) == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

    /* delete the IP TCAM shadow struct (also free LPM DB memory) */
    return prvCpssDxChLpmTcamDeleteShadowStruct(lpmDbPtr);
}

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
)
{
    if (lpmDbPtr->isIpVerIsInitialized[CPSS_IP_PROTOCOL_IPV4_E] == GT_TRUE)
        *protocolStackPtr = (lpmDbPtr->isIpVerIsInitialized[CPSS_IP_PROTOCOL_IPV6_E] == GT_TRUE) ?
            CPSS_IP_PROTOCOL_IPV4V6_E : CPSS_IP_PROTOCOL_IPV4_E;
    else
        *protocolStackPtr = CPSS_IP_PROTOCOL_IPV6_E;

    memoryCfgPtr->indexesRange.firstIndex = lpmDbPtr->tcamIndexRange.firstIndex;
    memoryCfgPtr->indexesRange.lastIndex  = lpmDbPtr->tcamIndexRange.lastIndex;

    memoryCfgPtr->partitionEnable = lpmDbPtr->tcamPartitionEnable;

    memoryCfgPtr->tcamLpmManagerCapcityCfg.numOfIpv4Prefixes = lpmDbPtr->numOfIpPrefixes[CPSS_IP_PROTOCOL_IPV4_E];
    memoryCfgPtr->tcamLpmManagerCapcityCfg.numOfIpv6Prefixes = lpmDbPtr->numOfIpPrefixes[CPSS_IP_PROTOCOL_IPV6_E];
    memoryCfgPtr->tcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = lpmDbPtr->numOfIpMcSrcAddr;

    if (lpmDbPtr->isTcamManagerInternal)
    {
        memoryCfgPtr->tcamManagerHandlerPtr = NULL;
    }
    else
    {
        memoryCfgPtr->tcamManagerHandlerPtr = lpmDbPtr->tcamManagerHandlerPtr;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS                                           retVal;
    CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT reservationType;
    GT_BOOL                                             partitionEnable;
    CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC             reservedEntriesArray[4];
    CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC             reservedEntriesArrayDummy[4];
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC                    reservationRange;
    GT_U32                                              totalLines;
    GT_U32                                              totalSingleEntries;
    GT_U32                                              totalQuadEntries;
    GT_U32                                              sizeOfArray = 4;
    GT_U32                                              sizeOfDummyArray = 4;
    GT_U32                                              numberOfUnusedTcamEntries = 0;

    if (lpmDbPtr->tcamManagementMode != PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E)
    {
        /*this api is relevant only when working with tcam manager*/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* get reservation type */
    retVal = cpssDxChTcamManagerEntriesReservationGet(lpmDbPtr->tcamManagerHandlerPtr,
                                                      lpmDbPtr->clientId,
                                                      &reservationType,
                                                      reservedEntriesArrayDummy,
                                                      &sizeOfDummyArray,
                                                      &reservationRange);

    if (retVal != GT_OK)
        return retVal;

    if (reservationType == CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E )
    {
        partitionEnable = GT_TRUE;
    }
    else
    {
        partitionEnable = GT_FALSE;
    }

    retVal = prvCpssDxChLpmTcamCheckCapacityForShadow(lpmDbPtr->shadowType,
                                                      indexesRangePtr,
                                                      partitionEnable,
                                                      tcamLpmManagerCapcityCfgPtr,
                                                      &totalLines,
                                                      &totalSingleEntries,
                                                      &totalQuadEntries);

    if (retVal != GT_OK)
        return retVal;

    retVal = prvCpssDxChLpmTcamPrepareReservation(lpmDbPtr->shadowType,
                                                  partitionEnable,
                                                  tcamLpmManagerCapcityCfgPtr,
                                                  totalSingleEntries + totalQuadEntries * 4,
                                                  totalLines,
                                                  &reservationType,
                                                  reservedEntriesArray,
                                                  &sizeOfArray,
                                                  &numberOfUnusedTcamEntries);

    if (retVal != GT_OK)
        return retVal;

    reservationRange.firstLine = indexesRangePtr->firstIndex;
    reservationRange.lastLine = indexesRangePtr->lastIndex;

    /* Reserve entries for a client in the TCAM manager*/
    retVal =  cpssDxChTcamManagerEntriesReservationSet(lpmDbPtr->tcamManagerHandlerPtr,
                                                       lpmDbPtr->clientId,
                                                       reservationType,
                                                       reservedEntriesArray,
                                                       sizeOfArray,
                                                       &reservationRange);
    if (retVal == GT_OK)
    {
        /* update TCAM shadow */
        lpmDbPtr->tcamIndexRange.firstIndex = indexesRangePtr->firstIndex;
        lpmDbPtr->tcamIndexRange.lastIndex = indexesRangePtr->lastIndex;
        lpmDbPtr->numOfIpMcSrcAddr =
            tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes;
        lpmDbPtr->numOfIpPrefixes[CPSS_IP_PROTOCOL_IPV4_E] =
            tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes;
        if ( (lpmDbPtr->isIpVerIsInitialized[CPSS_IP_PROTOCOL_IPV4_E] == GT_TRUE) &&
             (lpmDbPtr->tcamPartitionEnable == GT_TRUE) )
        {
            tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes += numberOfUnusedTcamEntries;
        }
        lpmDbPtr->numOfIpPrefixes[CPSS_IP_PROTOCOL_IPV6_E] =
            tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes;
    }

    return retVal;
}

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
)
{
    GT_STATUS retVal;
    CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT reservationType;
    CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC             reservedEntriesArray[2];
    CPSS_DXCH_TCAM_MANAGER_RANGE_STC                    reservationRange;
    GT_U32 sizeOfArray = 2;

    if (lpmDbPtr->tcamManagementMode != PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E)
    {
        /*this api is relevant only when working with tcam manager*/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    retVal = cpssDxChTcamManagerEntriesReservationGet(lpmDbPtr->tcamManagerHandlerPtr,
                                                      lpmDbPtr->clientId,
                                                      &reservationType,
                                                      reservedEntriesArray,
                                                      &sizeOfArray,
                                                      &reservationRange);

    if (retVal != GT_OK)
        return retVal;

    if (reservationType == CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E)
    {
        *partitionEnablePtr = GT_TRUE;
    }
    else
    {
        *partitionEnablePtr = GT_FALSE;
    }

    indexesRangePtr->firstIndex = lpmDbPtr->tcamIndexRange.firstIndex;
    indexesRangePtr->lastIndex = lpmDbPtr->tcamIndexRange.lastIndex;

    tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes = lpmDbPtr->numOfIpMcSrcAddr;
    tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes = lpmDbPtr->numOfIpPrefixes[CPSS_IP_PROTOCOL_IPV4_E];
    tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes = lpmDbPtr->numOfIpPrefixes[CPSS_IP_PROTOCOL_IPV6_E];

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STC addedDevList;

    addedDevList.ipShareDevNum = numOfDevs;
    addedDevList.ipShareDevs = devListArray;
    return prvCpssDxChLpmTcamAddDevToShadow(&addedDevList,lpmDbPtr);
}

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
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STC addedDevList;

    addedDevList.ipShareDevNum = numOfDevs;
    addedDevList.ipShareDevs = devListArray;
    return prvCpssDxChLpmTcamRemoveDevsFromShadow(&addedDevList,lpmDbPtr);
}

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
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
)
{
    GT_STATUS                                               rc = GT_OK;
    CPSS_IP_PROTOCOL_STACK_ENT                              protocolStack;
    PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT   entryType;
    GT_PTR                                                  tcamRulePtr = NULL;
    GT_U32                                                  arrayIndex;
    GT_U32                                                  i;
    GT_U32                                                  prefixNumber = 1;
    CPSS_IP_UNICAST_MULTICAST_ENT                           ipMode = CPSS_IP_UNICAST_E;
    PRV_CPSS_DXCH_LPM_TCAM_IP_MC_ENTRY_TYPE_ENT             mcEntryType = PRV_CPSS_DXCH_LPM_TCAM_IP_MC_GROUP_E;

    if ((pclIdArrayLen == 0) || (pclIdArrayLen > 1048575))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* default MC can not be set more than once */
    if (lpmDbPtr->usePbrDefaultMc == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }
    /* the LPM DB must be empty when using this API (can't
       contain any prefixes or virtual routers) */
    if (prvCpssSlIsEmpty(lpmDbPtr->vrSl) != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    /* the LPM DB must be configured to policy based routing mode */
    switch (lpmDbPtr->shadowType)
    {
    case PRV_CPSS_DXCH_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* allocate memory for the default MC rule pointers: 2 rules per PCL ID
       that may be used be the LPM DB (one per protocol stack) */
    pbrDefaultMcTrieNodePtrArraySize = 2 * pclIdArrayLen;
    pbrDefaultMcTrieNodePtrArray = cpssOsMalloc(pbrDefaultMcTrieNodePtrArraySize *
                                                sizeof(PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC*));
    if (pbrDefaultMcTrieNodePtrArray == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    /* init all pointers to NULL */
    cpssOsMemSet(pbrDefaultMcTrieNodePtrArray,0,pbrDefaultMcTrieNodePtrArraySize *
                 sizeof(PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC*));

    /* for each PCL ID add one MC default per protocol stack supported by the LPM DB */
    for (i = 0 ; i < pclIdArrayLen ; i++)
    {
        /* for each supported protocol stack, set rule for default MC */
        for (protocolStack = CPSS_IP_PROTOCOL_IPV4_E ; protocolStack <= CPSS_IP_PROTOCOL_IPV6_E ; protocolStack++)
        {
            arrayIndex = 2 * i + protocolStack;
            /* verify the LPM DB supports the protocol stack */
            if (lpmDbPtr->isIpVerIsInitialized[protocolStack] == GT_FALSE)
                continue;
            /* The new TCAM allocation is on the way -- check by means of prefix
               counters if it is possible to do */
            rc = prvCpssDxChLpmTcamFreeSpaceCountersCheck(prefixNumber,ipMode,mcEntryType,
                                                          protocolStack,lpmDbPtr);
            if (rc != GT_OK)
            {
                cpssOsFree(pbrDefaultMcTrieNodePtrArray);
                return rc;
            }
            /* create dummy trie node for policy based routing MC defaults */
            rc = prvCpssDxChLpmTcamPatTrieInit(lpmDbPtr,&pbrDefaultMcTrieNodePtrArray[arrayIndex]);
            if (rc != GT_OK)
                return rc;

            /* to support pbr mc defaults in Tcam Manager Mode allocate memory for
               token info attached to dummy pbrDefaultMcTrieNode */
            PRV_CPSS_DXCH_LPM_TCAM_ALLOCATE_TOKEN_NODE_MAC(lpmDbPtr->tcamManagementMode,
                                                           pbrDefaultMcTrieNodePtrArray[arrayIndex]);

            if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
                entryType = PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_IPV4_UC_ENTRY_E;
            else /* protocolStack == CPSS_IP_PROTOCOL_IPV6_E */
                entryType = PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_IPV6_UC_ENTRY_E;

            /* allocate minimal rule for the default MC from the UC pool */
            rc =  prvCpssDxChLpmTcamDispatchAllocate(lpmDbPtr,
                                                     entryType,
                                                     NULL,
                                                     NULL,
                                                     PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_MIN_E,
                                                     GT_FALSE,
                                                     pbrDefaultMcTrieNodePtrArray[arrayIndex],
                                                     &tcamRulePtr);
            if (rc != GT_OK)
                return rc;
            lpmDbPtr->allocatedIpPrefixCounters.numOfAllocatedIpPrefixes[protocolStack]++;
            /* update allocated rule rule */
            rc = prvCpssDxChLpmTcamDispatcherRuleInfoUpdate(PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_ALLOCATED_RULE_UPDATE_E,
                                                            lpmDbPtr->tcamManagementMode,
                                                            NULL,
                                                            pbrDefaultMcTrieNodePtrArray[arrayIndex],
                                                            tcamRulePtr);
            if (rc != GT_OK)
                return rc;

            /* update TCAM rule data */
            rc = prvCpssDxChLpmTcamDispatcherRuleInfoUpdate(PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_RULE_UPDATE_TRIE_E,
                                                            lpmDbPtr->tcamManagementMode,
                                                            pbrDefaultMcTrieNodePtrArray[arrayIndex],
                                                            pbrDefaultMcTrieNodePtrArray[arrayIndex],
                                                            NULL);
            if (rc != GT_OK)
                return rc;

            /* store the PCL ID in the trie node vrId field */
            pbrDefaultMcTrieNodePtrArray[arrayIndex]->vrId = pclIdArray[i];
        }
    }

    /* set the default MC rule in the PCL */
    rc = prvCpssDxChLpmTcamPbrDefaultMcRuleSet(lpmDbPtr,
                                               lpmDbPtr->workDevListPtr);
    if (rc != GT_OK)
        return rc;

    /* mark that this LPM DB uses policy based routing default MC */
    lpmDbPtr->usePbrDefaultMc = GT_TRUE;

    return rc;
}

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
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
)
{
    GT_STATUS rc = GT_OK;
    GT_PTR tcamRulePtr = NULL;
    GT_U32 offset;
    GT_U32 i;
    GT_U32 ipv4ArrayCurrentIndex = 0;
    GT_U32 ipv6ArrayCurrentIndex = 0;
    GT_U32 pclIdArrayCurrentindex = 0;

    *defaultMcUsedPtr = lpmDbPtr->usePbrDefaultMc;

    if (*defaultMcUsedPtr == GT_TRUE)
    {
        /* determine the protocol stack the LPM DB supports */
        if (lpmDbPtr->isIpVerIsInitialized[CPSS_IP_PROTOCOL_IPV4_E] == GT_TRUE)
        {
            if (lpmDbPtr->isIpVerIsInitialized[CPSS_IP_PROTOCOL_IPV6_E] == GT_TRUE)
                *protocolStackPtr = CPSS_IP_PROTOCOL_IPV4V6_E;
            else
                *protocolStackPtr = CPSS_IP_PROTOCOL_IPV4_E;
        }
        else
        {
            if (lpmDbPtr->isIpVerIsInitialized[CPSS_IP_PROTOCOL_IPV6_E] == GT_TRUE)
                *protocolStackPtr = CPSS_IP_PROTOCOL_IPV6_E;
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);     /* should never happen */
        }

        /* go over default MC rules allocated and fill output arrays */
        for (i = 0 ; i < pbrDefaultMcTrieNodePtrArraySize ; i++)
        {
            if (pbrDefaultMcTrieNodePtrArray[i] == NULL)
                continue;

            rc = prvCpssDxChLpmTcamDispatcherRuleSet(pbrDefaultMcTrieNodePtrArray[i],
                                                     lpmDbPtr,&tcamRulePtr);
            if (rc != GT_OK)
                return rc;

            rc = prvCpssDxChLpmTcamGetTcamOffset(lpmDbPtr,tcamRulePtr,GT_TRUE,&offset);
            if (rc != GT_OK)
                return rc;

            /* even entries are used to IPv4 while odd one for IPv6 */
            if ((i % 2) == 0)
            {
                if ((ipv4ArrayCurrentIndex + 1) > *defaultIpv4RuleIndexArrayLenPtr)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
                defaultIpv4RuleIndexArray[ipv4ArrayCurrentIndex] = offset;
                ipv4ArrayCurrentIndex++;

                /* if IPv6 is not supported, update the PCL ID from the ipv4 entry */
                if (*protocolStackPtr != CPSS_IP_PROTOCOL_IPV6_E)
                {
                    if ((pclIdArrayCurrentindex + 1) > *pclIdArrayLenPtr)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
                    pclIdArray[pclIdArrayCurrentindex] = pbrDefaultMcTrieNodePtrArray[i]->vrId;
                    pclIdArrayCurrentindex++;
                }
            }
            else /* odd (ipv6) entry */
            {
                if (ipv6ArrayCurrentIndex + 1 > *defaultIpv6RuleIndexArrayLenPtr)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
                defaultIpv6RuleIndexArray[ipv6ArrayCurrentIndex] = offset;
                ipv6ArrayCurrentIndex++;

                /* if only IPv6 is supported, update the PCL ID from the ipv6 entry */
                if (*protocolStackPtr == CPSS_IP_PROTOCOL_IPV6_E)
                {
                    if ((pclIdArrayCurrentindex + 1) > *pclIdArrayLenPtr)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
                    pclIdArray[pclIdArrayCurrentindex] = pbrDefaultMcTrieNodePtrArray[i]->vrId;
                    pclIdArrayCurrentindex++;
                }
            }
        }
    }

    *defaultIpv4RuleIndexArrayLenPtr = ipv4ArrayCurrentIndex;
    *defaultIpv4RuleIndexArrayLenPtr = ipv6ArrayCurrentIndex;
    *pclIdArrayLenPtr                = pclIdArrayCurrentindex;

    return rc;
}

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
)
{
    GT_STATUS                                 retVal;
    PRV_CPSS_DXCH_LPM_TCAM_VR_TBL_BLOCK_STC   *sharedVrPtr,tmpVrEntry;
    CPSS_IP_PROTOCOL_STACK_ENT                protocolStack;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT    *defIpv4UcNextHopInfoPtr = NULL;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT    *defIpv6UcNextHopInfoPtr = NULL;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *defIpv4McRouteLttEntryPtr = NULL;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *defIpv6McRouteLttEntryPtr = NULL;

    cpssOsMemSet(&tmpVrEntry,0,sizeof(tmpVrEntry));

    /* init pointers */
    if (vrConfigPtr->supportIpv4Uc == GT_TRUE)
    {
        defIpv4UcNextHopInfoPtr = &vrConfigPtr->defIpv4UcNextHopInfo;
    }
    if (vrConfigPtr->supportIpv6Uc == GT_TRUE)
    {
        defIpv6UcNextHopInfoPtr = &vrConfigPtr->defIpv6UcNextHopInfo;
    }
    if (vrConfigPtr->supportIpv4Mc == GT_TRUE)
    {
        defIpv4McRouteLttEntryPtr = &vrConfigPtr->defIpv4McRouteLttEntry;
    }
    if (vrConfigPtr->supportIpv6Mc == GT_TRUE)
    {
        defIpv6McRouteLttEntryPtr = &vrConfigPtr->defIpv6McRouteLttEntry;
    }

    /* determine the supported protocols */
    if ((vrConfigPtr->supportIpv6Uc == GT_TRUE) || (vrConfigPtr->supportIpv6Mc == GT_TRUE))
    {
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
    }
    else
    {
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    }

    /* if protocol stack was determined to be ipv6, we will check if it ipv4v6 */
    if ((protocolStack == CPSS_IP_PROTOCOL_IPV6_E) &&
        ((vrConfigPtr->supportIpv4Uc == GT_TRUE) || (vrConfigPtr->supportIpv4Mc == GT_TRUE)))
    {
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
    }

    /* policy based shadow types do not support MC */
    if ((lpmDbPtr->shadowType == PRV_CPSS_DXCH_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E))
    {
        if ( (vrConfigPtr->supportIpv4Mc == GT_TRUE) || (vrConfigPtr->supportIpv6Mc == GT_TRUE) )
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* if shared virtual router exists, make sure this added virtual router
       does not share prefixes with the shared virtual router */
    if (lpmDbPtr->sharedVrExist == GT_TRUE)
    {
        /* retrieve the shared virtual router */
        tmpVrEntry.vrId = lpmDbPtr->sharedVrId;
        sharedVrPtr = prvCpssSlSearch(lpmDbPtr->vrSl,&tmpVrEntry);
        if (sharedVrPtr == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* should never happen */

        /* check if UC entries exist in the shared virtual router */
        if ((vrConfigPtr->supportIpv4Uc == GT_TRUE) || (vrConfigPtr->supportIpv6Uc == GT_TRUE))
        {
            if ((protocolStack == CPSS_IP_PROTOCOL_IPV4_E) || (protocolStack == CPSS_IP_PROTOCOL_IPV4V6_E))
                if (sharedVrPtr->ipUcTrie[0] != NULL)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            if ((protocolStack == CPSS_IP_PROTOCOL_IPV6_E) || (protocolStack == CPSS_IP_PROTOCOL_IPV4V6_E))
                if (sharedVrPtr->ipUcTrie[1] != NULL)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* check if MC entries exist in the shared virtual router */
        if ((vrConfigPtr->supportIpv4Mc == GT_TRUE) || (vrConfigPtr->supportIpv6Mc == GT_TRUE))
        {
            if ((protocolStack == CPSS_IP_PROTOCOL_IPV4_E) || (protocolStack == CPSS_IP_PROTOCOL_IPV4V6_E))
                if (sharedVrPtr->ipMcTrie[0] != NULL)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            if ((protocolStack == CPSS_IP_PROTOCOL_IPV6_E) || (protocolStack == CPSS_IP_PROTOCOL_IPV4V6_E))
                if (sharedVrPtr->ipMcTrie[1] != NULL)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* check validity of the route entry method */
    if ((lpmDbPtr->shadowType != PRV_CPSS_DXCH_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E))
    {
        if ((vrConfigPtr->defIpv4UcNextHopInfo.routeEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E) &&
            (vrConfigPtr->defIpv4UcNextHopInfo.routeEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E) &&
            (vrConfigPtr->defIpv4UcNextHopInfo.routeEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if ((vrConfigPtr->defIpv6UcNextHopInfo.routeEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E) &&
            (vrConfigPtr->defIpv6UcNextHopInfo.routeEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E) &&
            (vrConfigPtr->defIpv6UcNextHopInfo.routeEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if ((vrConfigPtr->defIpv4McRouteLttEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E) &&
            (vrConfigPtr->defIpv4McRouteLttEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E) &&
            (vrConfigPtr->defIpv4McRouteLttEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if ((vrConfigPtr->defIpv6McRouteLttEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E) &&
            (vrConfigPtr->defIpv6McRouteLttEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E) &&
            (vrConfigPtr->defIpv6McRouteLttEntry.routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    retVal = prvCpssDxChLpmTcamAddVirtualRouter(vrId,
                                                defIpv4UcNextHopInfoPtr,
                                                defIpv6UcNextHopInfoPtr,
                                                NULL,
                                                defIpv4McRouteLttEntryPtr,
                                                defIpv6McRouteLttEntryPtr,
                                                NULL,
                                                protocolStack,
                                                lpmDbPtr);
    return retVal;
}

/**
* @internal prvCpssDxChLpmTcamVirtualRouterSharedAdd function
* @endinternal
*
* @brief   This function adds a shared virtual router in system for specific LPM DB.
*         Prefixes that reside within shared virtual router will participate in the
*         lookups of all virtual routers.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
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
)
{
    GT_STATUS                                 retVal;
    GT_STATUS                                 retVal2;
    PRV_CPSS_DXCH_LPM_TCAM_VR_TBL_BLOCK_STC   *vrEntryPtr;
    GT_UINTPTR                                iter;
    GT_U32                                    internalVrId;
    GT_IPADDR                                 nonMcIpv4Addr[4];
    GT_U32                                    nonMcIpv4PrefixLen[4];
    GT_IPV6ADDR                               nonMcIpv6Addr[8];
    GT_U32                                    nonMcIpv6PrefixLen[8];
    GT_U32                                    i;
    GT_BOOL                                   defragEnable; /* whether LPM DB supports dynamic partition */
    CPSS_IP_PROTOCOL_STACK_ENT                protocolStack;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT    *defIpv4UcNextHopInfoPtr = NULL;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT    *defIpv6UcNextHopInfoPtr = NULL;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *defIpv4McRouteLttEntryPtr = NULL;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *defIpv6McRouteLttEntryPtr = NULL;

    /* init pointers */
    if (vrConfigPtr->supportIpv4Uc == GT_TRUE)
    {
        defIpv4UcNextHopInfoPtr = &vrConfigPtr->defIpv4UcNextHopInfo;
    }
    if (vrConfigPtr->supportIpv6Uc == GT_TRUE)
    {
        defIpv6UcNextHopInfoPtr = &vrConfigPtr->defIpv6UcNextHopInfo;
    }
    if (vrConfigPtr->supportIpv4Mc == GT_TRUE)
    {
        defIpv4McRouteLttEntryPtr = &vrConfigPtr->defIpv4McRouteLttEntry;
    }
    if (vrConfigPtr->supportIpv6Mc == GT_TRUE)
    {
        defIpv6McRouteLttEntryPtr = &vrConfigPtr->defIpv6McRouteLttEntry;
    }

    /* determine the supported protocols */
    if ((vrConfigPtr->supportIpv6Uc == GT_TRUE) || (vrConfigPtr->supportIpv6Mc == GT_TRUE))
    {
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
    }
    else
    {
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    }

    /* if protocol stack was determined to be ipv6, we will check if it ipv4v6 */
    if ((protocolStack == CPSS_IP_PROTOCOL_IPV6_E) &&
        ((vrConfigPtr->supportIpv4Uc == GT_TRUE) || (vrConfigPtr->supportIpv4Mc == GT_TRUE)))
    {
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
    }

    /* Currently, shared MC prefixes are not supported */
    if ( (vrConfigPtr->supportIpv4Mc == GT_TRUE) || (vrConfigPtr->supportIpv6Mc == GT_TRUE) )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* only one shared virtual router can exists within LPM DB */
    if (lpmDbPtr->sharedVrExist == GT_TRUE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);

    /* Only xcat shadow type and above are supported. Policy based routing
       shadow types are not supported */
    if (lpmDbPtr->shadowType != PRV_CPSS_DXCH_LPM_TCAM_XCAT_SHADOW_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* scan all existing non-shared virtual routers and verify that the shared
       virtual router will not share prefixes with them */
    iter = 0;
    while ((vrEntryPtr = prvCpssSlGetNext(lpmDbPtr->vrSl,&iter)) != NULL)
    {
        /* check if UC entries exist in non-shared virtual router */
        if ((vrConfigPtr->supportIpv4Uc == GT_TRUE) || (vrConfigPtr->supportIpv6Uc == GT_TRUE))
        {
            if ((protocolStack == CPSS_IP_PROTOCOL_IPV4_E) || (protocolStack == CPSS_IP_PROTOCOL_IPV4V6_E))
                if (vrEntryPtr->ipUcTrie[0] != NULL)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            if ((protocolStack == CPSS_IP_PROTOCOL_IPV6_E) || (protocolStack == CPSS_IP_PROTOCOL_IPV4V6_E))
                if (vrEntryPtr->ipUcTrie[1] != NULL)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* check if MC entries exist in non-shared virtual router */
        if ( (vrConfigPtr->supportIpv4Mc == GT_TRUE) || (vrConfigPtr->supportIpv6Mc == GT_TRUE) )
        {
            if ((protocolStack == CPSS_IP_PROTOCOL_IPV4_E) || (protocolStack == CPSS_IP_PROTOCOL_IPV4V6_E))
                if (vrEntryPtr->ipMcTrie[0] != NULL)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            if ((protocolStack == CPSS_IP_PROTOCOL_IPV6_E) || (protocolStack == CPSS_IP_PROTOCOL_IPV4V6_E))
                if (vrEntryPtr->ipMcTrie[1] != NULL)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* internally, bit 15 in vrId indicates shared virtual router */
    internalVrId = vrId | BIT_15;

    /* add the virtual router */
    retVal = prvCpssDxChLpmTcamAddVirtualRouter(internalVrId,
                                                defIpv4UcNextHopInfoPtr,
                                                defIpv6UcNextHopInfoPtr,
                                                NULL,
                                                defIpv4McRouteLttEntryPtr,
                                                defIpv6McRouteLttEntryPtr,
                                                NULL,
                                                protocolStack,lpmDbPtr);
    if (retVal != GT_OK)
        return retVal;

    /* the virtual router was added successfully */
    lpmDbPtr->sharedVrExist = GT_TRUE;
    lpmDbPtr->sharedVrId = vrId;

    /* For shared virtual router that supports shared IPv4 UC and/or IPv6 UC,
       the <match all> TCAM entry that is used as UC default is not written to
       the TCAM. Instead add prefixes that represent all non-MC prefixes */
    if ((vrConfigPtr->supportIpv4Uc == GT_TRUE) || (vrConfigPtr->supportIpv6Uc == GT_TRUE))
    {
        /* add non-MC IPv4 prefixes */
        if ((protocolStack == CPSS_IP_PROTOCOL_IPV4_E) || (protocolStack == CPSS_IP_PROTOCOL_IPV4V6_E))
        {
            /* IPv4 MC prefixes are 0xE0/4. Therefore non IPv4 MC prefixes
               are: 0x00/1, 0x80/2, 0xC0/3, 0xF0/4 */
            cpssOsMemSet(nonMcIpv4Addr,0,sizeof(nonMcIpv4Addr));
            nonMcIpv4Addr[0].arIP[0] = 0;   nonMcIpv4PrefixLen[0] = 1;
            nonMcIpv4Addr[1].arIP[0] = 128; nonMcIpv4PrefixLen[1] = 2;
            nonMcIpv4Addr[2].arIP[0] = 192; nonMcIpv4PrefixLen[2] = 3;
            nonMcIpv4Addr[3].arIP[0] = 240; nonMcIpv4PrefixLen[3] = 4;

            for (i = 0 ; i < 4 ; i++)
            {
                retVal = prvCpssDxChLpmTcamIpv4UcPrefixAdd(lpmDbPtr,vrId,
                                                           nonMcIpv4Addr[i],
                                                           nonMcIpv4PrefixLen[i],
                                                           defIpv4UcNextHopInfoPtr,
                                                           GT_FALSE);
                if (retVal != GT_OK)
                {
                    retVal2 = prvCpssDxChLpmTcamVirtualRouterDel(lpmDbPtr,vrId);
                    return retVal2 == GT_OK ? retVal : retVal2 ;
                }
            }
        }

        /* add non-MC IPv6 prefixes */
        if ((protocolStack == CPSS_IP_PROTOCOL_IPV6_E) || (protocolStack == CPSS_IP_PROTOCOL_IPV4V6_E))
        {
            /* IPv6 MC prefixes are 0xFF/8. Therefore non IPv6 MC prefixes
               are: 0x00/1, 0x80/2, 0xC0/3, 0xE0/4, 0xF0/5, 0xF8/6, 0xFC/7, 0xFE/8 */
            cpssOsMemSet(nonMcIpv6Addr,0,sizeof(nonMcIpv6Addr));
            nonMcIpv6Addr[0].arIP[0] = 0x00;    nonMcIpv6PrefixLen[0] = 1;
            nonMcIpv6Addr[1].arIP[0] = 0x80;    nonMcIpv6PrefixLen[1] = 2;
            nonMcIpv6Addr[2].arIP[0] = 0xC0;    nonMcIpv6PrefixLen[2] = 3;
            nonMcIpv6Addr[3].arIP[0] = 0xE0;    nonMcIpv6PrefixLen[3] = 4;
            nonMcIpv6Addr[4].arIP[0] = 0xF0;    nonMcIpv6PrefixLen[4] = 5;
            nonMcIpv6Addr[5].arIP[0] = 0xF8;    nonMcIpv6PrefixLen[5] = 6;
            nonMcIpv6Addr[6].arIP[0] = 0xFC;    nonMcIpv6PrefixLen[6] = 7;
            nonMcIpv6Addr[7].arIP[0] = 0xFE;    nonMcIpv6PrefixLen[7] = 8;

            if (lpmDbPtr->tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E)
            {
                CPSS_TBD_BOOKMARK
                defragEnable = GT_FALSE;
            }
            else
            {
                if (lpmDbPtr->tcamEntriesHandler->operationMode == PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_PARTITION_MODE_E)
                    defragEnable = GT_FALSE;
                else if (lpmDbPtr->tcamEntriesHandler->operationMode == PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_NO_PARTITION_MODE_E)
                    defragEnable = GT_TRUE;
                else
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            for (i = 0 ; i < 8 ; i++)
            {
                retVal = prvCpssDxChLpmTcamIpv6UcPrefixAdd(lpmDbPtr,vrId,
                                                           nonMcIpv6Addr[i],
                                                           nonMcIpv6PrefixLen[i],
                                                           defIpv6UcNextHopInfoPtr,
                                                           GT_FALSE,defragEnable);
                if (retVal != GT_OK)
                {
                    retVal2 = prvCpssDxChLpmTcamVirtualRouterDel(lpmDbPtr,vrId);
                    return retVal;
                }
            }
        }
    } /* end adding non-MC prefixes */

    return retVal;
}

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
)
{
    return prvCpssDxChLpmTcamGetVirtualRouter(vrId,
                                              lpmDbPtr,
                                              &vrConfigPtr->supportIpv4Uc,
                                              &vrConfigPtr->defIpv4UcNextHopInfo,
                                              &vrConfigPtr->supportIpv6Uc,
                                              &vrConfigPtr->defIpv6UcNextHopInfo,
                                              NULL,
                                              &vrConfigPtr->supportIpv4Mc,
                                              &vrConfigPtr->defIpv4McRouteLttEntry,
                                              &vrConfigPtr->supportIpv6Mc,
                                              &vrConfigPtr->defIpv6McRouteLttEntry,
                                              NULL);
}

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
)
{
    GT_STATUS retVal;

    retVal = prvCpssDxChLpmTcamDeleteVirtualRouter(vrId,lpmDbPtr);

    /* if shared virtual router was deleted, mark that in the LPM DB */
    if ((lpmDbPtr->sharedVrExist == GT_TRUE) && (vrId == lpmDbPtr->sharedVrId))
    {
        lpmDbPtr->sharedVrExist = GT_FALSE;
        lpmDbPtr->sharedVrId = 0;
    }

    return retVal;
}

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
)
{
    return prvCpssDxChLpmTcamAddIpUcEntry(vrId,ipAddr.arIP,prefixLen,
                                          nextHopInfoPtr,NULL,override,GT_FALSE,
                                          GT_FALSE /* defragEnable */,
                                          CPSS_IP_PROTOCOL_IPV4_E,lpmDbPtr);
}

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
)
{
    GT_STATUS retVal,retVal2;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));

    retVal = GT_OK;

    for (i = 0; i < ipv4PrefixArrayLen; i++)
    {
        if (ipv4PrefixArrayPtr[i].prefixLen > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if ((ipv4PrefixArrayPtr[i].ipAddr.arIP[0] >= 224) && (ipv4PrefixArrayPtr[i].ipAddr.arIP[0] <= 239))
        {
            /* Multicast range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(lpmDbPtr->shadowType,
                                                         &(ipv4PrefixArrayPtr[i].nextHopInfo),
                                                         &lpmRouteEntry);
        ipv4PrefixArrayPtr[i].returnStatus =
            prvCpssDxChLpmTcamAddIpUcEntry(ipv4PrefixArrayPtr[i].vrId,
                                           ipv4PrefixArrayPtr[i].ipAddr.arIP,
                                           ipv4PrefixArrayPtr[i].prefixLen,
                                           &lpmRouteEntry,
                                           NULL,ipv4PrefixArrayPtr[i].override,GT_TRUE,
                                           GT_FALSE /* defragEnable */,
                                           CPSS_IP_PROTOCOL_IPV4_E,lpmDbPtr);
        if (ipv4PrefixArrayPtr[i].returnStatus != GT_OK)
        {
            retVal = GT_FAIL;
        }
    }

    /* now update the hw */
    retVal2 = prvCpssDxChLpmTcamUpdateHw(lpmDbPtr,
                                         PRV_CPSS_DXCH_LPM_TCAM_UPDATE_UC_E,
                                         CPSS_IP_PROTOCOL_IPV4_E);
    if (retVal == GT_OK)
        retVal = retVal2;


    return retVal;
}

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
)
{
    return prvCpssDxChLpmTcamDeleteIpUcEntry(vrId,ipAddr.arIP,prefixLen,
                                             CPSS_IP_PROTOCOL_IPV4_E,lpmDbPtr);
}

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
)
{
    GT_STATUS retVal = GT_OK;
    GT_U32 i;

    for (i = 0; i < ipv4PrefixArrayLen; i++)
    {
        if (ipv4PrefixArrayPtr[i].prefixLen > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if ((ipv4PrefixArrayPtr[i].ipAddr.arIP[0] >= 224) && (ipv4PrefixArrayPtr[i].ipAddr.arIP[0] <= 239))
        {
            /* Multicast range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        ipv4PrefixArrayPtr[i].returnStatus =
            prvCpssDxChLpmTcamDeleteIpUcEntry(ipv4PrefixArrayPtr[i].vrId,
                                              ipv4PrefixArrayPtr[i].ipAddr.arIP,
                                              ipv4PrefixArrayPtr[i].prefixLen,
                                              CPSS_IP_PROTOCOL_IPV4_E,lpmDbPtr);
        if (ipv4PrefixArrayPtr[i].returnStatus != GT_OK)
            retVal = GT_FAIL;
    }

    return retVal;
}

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
)
{
    return prvCpssDxChLpmTcamFlushIpUcEntries(vrId,CPSS_IP_PROTOCOL_IPV4_E,
                                              lpmDbPtr,GT_FALSE);
}

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
)
{
    GT_STATUS retVal;
    GT_U32    *internalRuleIdxPtr = NULL;

    retVal = prvCpssDxChLpmTcamSearchIpUcEntry(vrId,ipAddr.arIP,prefixLen,
                                               nextHopInfoPtr,internalRuleIdxPtr,
                                               CPSS_IP_PROTOCOL_IPV4_E,lpmDbPtr);
    if(retVal != GT_OK)
        return retVal;

    retVal = prvCpssDxChLpmTcamUcRuleOffsetGet(vrId,ipAddr.arIP,
                                               prefixLen,
                                               CPSS_IP_PROTOCOL_IPV4_E,
                                               lpmDbPtr,
                                               tcamRowIndexPtr,
                                               tcamColumnIndexPtr);
    return retVal;
}

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
)
{
    GT_STATUS retVal;
    GT_U32                            *internalRuleIdxPtr = NULL;

    GT_U8  tempIpAddr[4];
    GT_U32 ipAddrNumOfOctets;
    GT_U32 ipAddrNumOfBitsToMask;

    retVal = prvCpssDxChLpmTcamGetIpUcEntry(vrId,
                                            ipAddrPtr->arIP,
                                            prefixLenPtr,
                                            CPSS_IP_PROTOCOL_IPV4_E,
                                            nextHopInfoPtr,
                                            internalRuleIdxPtr,
                                            lpmDbPtr);
    if(retVal != GT_OK)
        return retVal;

    retVal = prvCpssDxChLpmTcamUcRuleOffsetGet(vrId,
                                               ipAddrPtr->arIP,
                                               *prefixLenPtr,
                                               CPSS_IP_PROTOCOL_IPV4_E,
                                               lpmDbPtr,
                                               tcamRowIndexPtr,
                                               tcamColumnIndexPtr);
     if(retVal != GT_OK)
        return retVal;

    /* reset non relevant bit according to prefix length */

    cpssOsMemSet(tempIpAddr,0,sizeof(tempIpAddr));
    ipAddrNumOfOctets = *prefixLenPtr / 8;
    ipAddrNumOfBitsToMask = *prefixLenPtr % 8;

    if(ipAddrNumOfBitsToMask == 0)
    {
        cpssOsMemCpy(tempIpAddr,&ipAddrPtr->arIP,sizeof(GT_U8)*ipAddrNumOfOctets);
    }
    else
    {
        cpssOsMemCpy(tempIpAddr,&ipAddrPtr->arIP,sizeof(GT_U8)*(ipAddrNumOfOctets+1));
    }

    cpssOsMemCpy(&ipAddrPtr->arIP,tempIpAddr,sizeof(ipAddrPtr->arIP));

    return retVal;
}

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
)
{
    return prvCpssDxChLpmTcamAddIpMcEntry(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                          ipSrc.arIP,ipSrcPrefixLen,
                                          mcRouteEntryPtr,NULL,override,
                                          defragmentationEnable,
                                          CPSS_IP_PROTOCOL_IPV4_E,lpmDbPtr);
}

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
)
{
    return prvCpssDxChLpmTcamDeleteIpMcEntry(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                             ipSrc.arIP,ipSrcPrefixLen,
                                             CPSS_IP_PROTOCOL_IPV4_E,lpmDbPtr);
}

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
)
{
    return prvCpssDxChLpmTcamFlushIpMcEntries(vrId,CPSS_IP_PROTOCOL_IPV4_E,
                                              lpmDbPtr,GT_FALSE);
}

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
)
{
    GT_STATUS retVal;
    GT_U8  tempIpAddr[4];
    GT_U32 ipAddrNumOfOctets;
    GT_U32 ipAddrNumOfBitsToMask;

    retVal = prvCpssDxChLpmTcamGetMcEntry(vrId,ipGroupPtr->arIP,ipGroupPrefixLenPtr,
                                          ipSrcPtr->arIP,ipSrcPrefixLenPtr,
                                          mcRouteLttEntryPtr,NULL,
                                          CPSS_IP_PROTOCOL_IPV4_E,lpmDbPtr);

    if (retVal != GT_OK)
    {
        return retVal;
    }

    retVal = prvCpssDxChLpmTcamMcRuleOffsetGet(vrId,
                                               ipGroupPtr->arIP,
                                               *ipGroupPrefixLenPtr,
                                               ipSrcPtr->arIP,
                                               *ipSrcPrefixLenPtr,
                                               CPSS_IP_PROTOCOL_IPV4_E,
                                               lpmDbPtr,
                                               tcamGroupRowIndexPtr,
                                               tcamGroupColumnIndexPtr,
                                               tcamSrcRowIndexPtr,
                                               tcamSrcColumnIndexPtr);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    /* reset non relevant bit according to prefix length */

    /* IP Group */
    cpssOsMemSet(tempIpAddr,0,sizeof(tempIpAddr));
    ipAddrNumOfOctets = *ipGroupPrefixLenPtr / 8;
    ipAddrNumOfBitsToMask = *ipGroupPrefixLenPtr % 8;

    if(ipAddrNumOfBitsToMask == 0)
    {
        cpssOsMemCpy(tempIpAddr,&ipGroupPtr->arIP,sizeof(GT_U8)*ipAddrNumOfOctets);
    }
    else
    {
        cpssOsMemCpy(tempIpAddr,&ipGroupPtr->arIP,sizeof(GT_U8)*(ipAddrNumOfOctets+1));
    }

    cpssOsMemCpy(&ipGroupPtr->arIP,tempIpAddr,sizeof(ipGroupPtr->arIP));

    /* IP Source */
    cpssOsMemSet(tempIpAddr,0,sizeof(tempIpAddr));
    ipAddrNumOfOctets = *ipSrcPrefixLenPtr / 8;
    ipAddrNumOfBitsToMask = *ipSrcPrefixLenPtr % 8;

    if(ipAddrNumOfBitsToMask == 0)
    {
        cpssOsMemCpy(tempIpAddr,&ipSrcPtr->arIP,sizeof(GT_U8)*ipAddrNumOfOctets);
    }
    else
    {
        cpssOsMemCpy(tempIpAddr,&ipSrcPtr->arIP,sizeof(GT_U8)*(ipAddrNumOfOctets+1));
    }

    cpssOsMemCpy(&ipSrcPtr->arIP,tempIpAddr,sizeof(ipSrcPtr->arIP));

    return retVal;
}


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
)
{
    GT_STATUS retVal;
    GT_U32                            *internalRuleIdxPtr = NULL;

    retVal = prvCpssDxChLpmTcamSearchMcEntry(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                             ipSrc.arIP,ipSrcPrefixLen,
                                             mcRouteLttEntryPtr,internalRuleIdxPtr,
                                             CPSS_IP_PROTOCOL_IPV4_E,lpmDbPtr);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    retVal = prvCpssDxChLpmTcamMcRuleOffsetGet(vrId,
                                               ipGroup.arIP,
                                               ipGroupPrefixLen,
                                               ipSrc.arIP,
                                               ipSrcPrefixLen,
                                               CPSS_IP_PROTOCOL_IPV4_E,
                                               lpmDbPtr,
                                               tcamGroupRowIndexPtr,
                                               tcamGroupColumnIndexPtr,
                                               tcamSrcRowIndexPtr,
                                               tcamSrcColumnIndexPtr);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    return retVal;
}

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
)
{
    return prvCpssDxChLpmTcamAddIpUcEntry(vrId,ipAddr.arIP,prefixLen,
                                          nextHopInfoPtr,NULL,override,
                                          GT_FALSE,defragmentationEnable,
                                          CPSS_IP_PROTOCOL_IPV6_E,lpmDbPtr);
}

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
)
{
    GT_STATUS retVal,retVal2;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));

    retVal = GT_OK;

    for (i = 0; i < ipv6PrefixArrayLen; i++)
    {
        if (ipv6PrefixArrayPtr[i].prefixLen > 128)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if (ipv6PrefixArrayPtr[i].ipAddr.arIP[0] == 255)
        {
            /* Multicast range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(lpmDbPtr->shadowType,
                                                         &(ipv6PrefixArrayPtr[i].nextHopInfo),
                                                         &lpmRouteEntry);
        ipv6PrefixArrayPtr[i].returnStatus =
            prvCpssDxChLpmTcamAddIpUcEntry(ipv6PrefixArrayPtr[i].vrId,
                                           ipv6PrefixArrayPtr[i].ipAddr.arIP,
                                           ipv6PrefixArrayPtr[i].prefixLen,
                                           &lpmRouteEntry,
                                           NULL,ipv6PrefixArrayPtr[i].override,
                                           GT_TRUE,defragmentationEnable,
                                           CPSS_IP_PROTOCOL_IPV6_E,lpmDbPtr);
        if (ipv6PrefixArrayPtr[i].returnStatus != GT_OK)
        {
            retVal = GT_FAIL;
        }
    }

    /* now update the hw */
    retVal2 = prvCpssDxChLpmTcamUpdateHw(lpmDbPtr,
                                         PRV_CPSS_DXCH_LPM_TCAM_UPDATE_UC_E,
                                         CPSS_IP_PROTOCOL_IPV6_E);
    if (retVal == GT_OK)
        retVal = retVal2;

    return retVal;
}

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
)
{
    return prvCpssDxChLpmTcamDeleteIpUcEntry(vrId,ipAddr.arIP,prefixLen,
                                             CPSS_IP_PROTOCOL_IPV6_E,lpmDbPtr);
}

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
)
{
    GT_STATUS retVal = GT_OK;
    GT_U32 i;

    for (i = 0; i < ipv6PrefixArrayLen; i++)
    {
        if (ipv6PrefixArrayPtr[i].prefixLen > 128)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if (ipv6PrefixArrayPtr[i].ipAddr.arIP[0] == 255)
        {
            /* Multicast range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        ipv6PrefixArrayPtr[i].returnStatus =
            prvCpssDxChLpmTcamDeleteIpUcEntry(ipv6PrefixArrayPtr[i].vrId,
                                              ipv6PrefixArrayPtr[i].ipAddr.arIP,
                                              ipv6PrefixArrayPtr[i].prefixLen,
                                              CPSS_IP_PROTOCOL_IPV6_E,lpmDbPtr);
        if (ipv6PrefixArrayPtr[i].returnStatus != GT_OK)
            retVal = GT_FAIL;
    }

    return retVal;
}


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
)
{
    return prvCpssDxChLpmTcamFlushIpUcEntries(vrId,CPSS_IP_PROTOCOL_IPV6_E,
                                              lpmDbPtr,GT_FALSE);
}

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
)
{
    GT_STATUS                        retVal;
    GT_U32                           *internalRuleIdxPtr = NULL;
    GT_U32                           tcamRowIndex;
    GT_U32                           tcamColumnIndex;

    retVal = prvCpssDxChLpmTcamSearchIpUcEntry(vrId,
                                               ipAddr.arIP,
                                               prefixLen,
                                               nextHopInfoPtr,
                                               internalRuleIdxPtr,
                                               CPSS_IP_PROTOCOL_IPV6_E,
                                               lpmDbPtr);
    if(retVal != GT_OK)
        return retVal;

    retVal = prvCpssDxChLpmTcamUcRuleOffsetGet(vrId,
                                               ipAddr.arIP,
                                               prefixLen,
                                               CPSS_IP_PROTOCOL_IPV6_E,
                                               lpmDbPtr,
                                               &tcamRowIndex,
                                               &tcamColumnIndex);
    if(retVal != GT_OK)
        return retVal;

    *tcamRowIndexPtr = tcamRowIndex;
    *tcamColumnIndexPtr = tcamColumnIndex;

    return retVal;
}

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
)
{
    GT_STATUS                        retVal;
    GT_U32                           *internalRuleIdxPtr = NULL;
    GT_U32                           tcamRowIndex;
    GT_U32                           tcamColumnIndex;

    GT_U8  tempIpAddr[16];
    GT_U32 ipAddrNumOfOctets;
    GT_U32 ipAddrNumOfBitsToMask;

    retVal = prvCpssDxChLpmTcamGetIpUcEntry(vrId,
                                            ipAddrPtr->arIP,
                                            prefixLenPtr,
                                            CPSS_IP_PROTOCOL_IPV6_E,
                                            nextHopInfoPtr,
                                            internalRuleIdxPtr,
                                            lpmDbPtr);
    if(retVal != GT_OK)
        return retVal;

    retVal = prvCpssDxChLpmTcamUcRuleOffsetGet(vrId,
                                               ipAddrPtr->arIP,
                                               *prefixLenPtr,
                                               CPSS_IP_PROTOCOL_IPV6_E,
                                               lpmDbPtr,
                                               &tcamRowIndex,
                                               &tcamColumnIndex);
     if(retVal != GT_OK)
        return retVal;

     *tcamRowIndexPtr    = tcamRowIndex;
     *tcamColumnIndexPtr = tcamColumnIndex;


    /* reset non relevant bit according to prefix length */

    cpssOsMemSet(tempIpAddr,0,sizeof(tempIpAddr));
    ipAddrNumOfOctets = *prefixLenPtr / 8;
    ipAddrNumOfBitsToMask = *prefixLenPtr % 8;

    if(ipAddrNumOfBitsToMask == 0)
    {
        cpssOsMemCpy(tempIpAddr,&ipAddrPtr->arIP,sizeof(GT_U8)*ipAddrNumOfOctets);
    }
    else
    {
        cpssOsMemCpy(tempIpAddr,&ipAddrPtr->arIP,sizeof(GT_U8)*(ipAddrNumOfOctets+1));
    }

    cpssOsMemCpy(&ipAddrPtr->arIP,tempIpAddr,sizeof(ipAddrPtr->arIP));

    return retVal;
}

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
)
{
    return prvCpssDxChLpmTcamAddIpMcEntry(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                          ipSrc.arIP,ipSrcPrefixLen,
                                          mcRouteEntryPtr,NULL,override,
                                          defragmentationEnable,
                                          CPSS_IP_PROTOCOL_IPV6_E,lpmDbPtr);
}

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
)
{
    return prvCpssDxChLpmTcamDeleteIpMcEntry(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                             ipSrc.arIP,ipSrcPrefixLen,
                                             CPSS_IP_PROTOCOL_IPV6_E,lpmDbPtr);
}

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
)
{
    return prvCpssDxChLpmTcamFlushIpMcEntries(vrId,CPSS_IP_PROTOCOL_IPV6_E,
                                              lpmDbPtr,GT_FALSE);
}

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
)
{
    GT_STATUS retVal;
    GT_U8  tempIpAddr[16];
    GT_U32 ipAddrNumOfOctets;
    GT_U32 ipAddrNumOfBitsToMask;
    GT_U32 tcamGroupColumnIndex,tcamSrcColumnIndex;

    retVal = prvCpssDxChLpmTcamGetMcEntry(vrId,ipGroupPtr->arIP,ipGroupPrefixLenPtr,
                                          ipSrcPtr->arIP,ipSrcPrefixLenPtr,
                                          mcRouteEntryPtr,NULL,
                                          CPSS_IP_PROTOCOL_IPV6_E,lpmDbPtr);
    if (retVal != GT_OK)
    {
        return retVal;
    }
    retVal = prvCpssDxChLpmTcamMcRuleOffsetGet(vrId,ipGroupPtr->arIP,*ipGroupPrefixLenPtr,
                                               ipSrcPtr->arIP,*ipSrcPrefixLenPtr,
                                               CPSS_IP_PROTOCOL_IPV6_E,lpmDbPtr,
                                               tcamGroupRowIndexPtr,
                                               &tcamGroupColumnIndex,
                                               tcamSrcRowIndexPtr,
                                               &tcamSrcColumnIndex);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    /* reset non relevant bit according to prefix length */

    /* IP Group */
    cpssOsMemSet(tempIpAddr,0,sizeof(tempIpAddr));
    ipAddrNumOfOctets = *ipGroupPrefixLenPtr / 8;
    ipAddrNumOfBitsToMask = *ipGroupPrefixLenPtr % 8;

    if(ipAddrNumOfBitsToMask == 0)
    {
        cpssOsMemCpy(tempIpAddr,&ipGroupPtr->arIP,sizeof(GT_U8)*ipAddrNumOfOctets);
    }
    else
    {
        cpssOsMemCpy(tempIpAddr,&ipGroupPtr->arIP,sizeof(GT_U8)*(ipAddrNumOfOctets+1));
    }

    cpssOsMemCpy(&ipGroupPtr->arIP,tempIpAddr,sizeof(ipGroupPtr->arIP));

    /* IP Source */
    cpssOsMemSet(tempIpAddr,0,sizeof(tempIpAddr));
    ipAddrNumOfOctets = *ipSrcPrefixLenPtr / 8;
    ipAddrNumOfBitsToMask = *ipSrcPrefixLenPtr % 8;

    if(ipAddrNumOfBitsToMask == 0)
    {
        cpssOsMemCpy(tempIpAddr,&ipSrcPtr->arIP,sizeof(GT_U8)*ipAddrNumOfOctets);
    }
    else
    {
        cpssOsMemCpy(tempIpAddr,&ipSrcPtr->arIP,sizeof(GT_U8)*(ipAddrNumOfOctets+1));
    }

    cpssOsMemCpy(&ipSrcPtr->arIP,tempIpAddr,sizeof(ipSrcPtr->arIP));

    return retVal;
}

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
)
{
    GT_STATUS                        retVal;
    GT_U32                           *internalRuleIdxPtr = NULL;
    GT_U32                           tcamGroupColumnIndex;
    GT_U32                           tcamSrcColumnIndex;

    retVal = prvCpssDxChLpmTcamSearchMcEntry(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                             ipSrc.arIP,ipSrcPrefixLen,
                                             mcRouteEntryPtr,internalRuleIdxPtr,
                                             CPSS_IP_PROTOCOL_IPV6_E,lpmDbPtr);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    retVal = prvCpssDxChLpmTcamMcRuleOffsetGet(vrId,
                                               ipGroup.arIP,
                                               ipGroupPrefixLen,
                                               ipSrc.arIP,
                                               ipSrcPrefixLen,
                                               CPSS_IP_PROTOCOL_IPV6_E,
                                               lpmDbPtr,
                                               tcamGroupRowIndexPtr,
                                               &tcamGroupColumnIndex,
                                               tcamSrcRowIndexPtr,
                                               &tcamSrcColumnIndex);
    return retVal;
}

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
)
{
    tcamRows = 0;
}

