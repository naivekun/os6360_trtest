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
* @file prvCpssDxChLpmRam.c
*
* @brief the CPSS LPM Engine support.
*
* @version   26
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTrie.h>
#include <cpssCommon/private/prvCpssDevMemManager.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamUc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMc.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamMng.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hsu/private/prvCpssDxChHsu.h>
#include <cpss/dxCh/dxChxGen/private/lpm/prvCpssDxChLpmUtils.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>

#ifdef CPSS_DXCH_LPM_DEBUG_MEM
GT_U32 prvCpssDxChLpmRamMemSize = 0;
#endif

void * cpssOsLpmMalloc
(
    IN GT_U32 size
)
{
#ifdef CPSS_DXCH_LPM_DEBUG_MEM
    static GT_U32 k = 0;
    prvCpssDxChLpmRamMemSize += size;
    k++;
#endif

    GT_VOID_PTR ptr = cpssOsMalloc(size);

    if(ptr)
    {
        cpssOsMemSet(ptr,0,size);
    }

    return ptr;
}

void cpssOsLpmFree
(
    IN void* const memblock
)
{
#ifdef CPSS_DXCH_LPM_DEBUG_MEM
    static GT_U32 k = 0;
    GT_U32 size1,size2,size3,size4;

    size1 = size2 = size3 = 0;
    printMemDelta(&size1,&size2,&size3);
    size4 = prvCpssDxChLpmRamMemSize;
#endif
    cpssOsFree(memblock);
#ifdef CPSS_DXCH_LPM_DEBUG_MEM
    size1 = size2 = size3 = 0;
    printMemDelta(&prvCpssDxChLpmRamMemSize,&size2,&size3);
    size4 = size4 - prvCpssDxChLpmRamMemSize;
    k++;
#endif
}

/*******************************************************************************
**                               local defines                                **
*******************************************************************************/
#if 0
#define PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_FREE_LINK_LIST_END_CNS 0xfffffffd
#define PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_PARTITION_END_CNS      0xfffffffe
#endif

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_TYPE_ENT
 *
 * @brief Indicates how the LPM search structures should be updated
*/
typedef enum{

    /** don't allocate memory in the HW. */
    PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E = 0,

    /** allocate memory in the HW. */
    PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E,

    /** overwrite and allocate memory if needed. */
    PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E

} PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_TYPE_ENT;

#if 0
/**
* @enum IP_HSU_STAGE_ENT
 *
 * @brief A enum representing an ip coreExMxPm shadow iterartor stage
 * values:
 * IP_HSU_VR_STAGE_E    - VR router stage
 * IP_HSU_UC_PREFIX_STAGE_E - ip uc prefix stage
 * IP_HSU_UC_LPM_STAGE_E  - ip uc lpm stage
 * IP_HSU_MC_PREFIX_STAGE_E - ip mc prefix stage
 * IP_HSU_MC_LPM_STAGE_E  - ip mc lpm stage
 * IP_HSU_DMM_FREE_LIST_E  - dmm free lists stage
*/
typedef enum{

    IP_HSU_VR_STAGE_E = 0,

    IP_HSU_UC_PREFIX_STAGE_E ,

    IP_HSU_UC_LPM_STAGE_E,

    IP_HSU_MC_PREFIX_STAGE_E,

    IP_HSU_MC_LPM_STAGE_E,

    IP_HSU_DMM_FREE_LIST_E,

    IP_HSU_IP_LAST_E

} IP_HSU_STAGE_ENT;

/*
 * Typedef: struct IP_HSU_ITERATOR_STC
 *
 * Description: A struct that holds an ip coreExMxPm shadow iterartor
 *
 * Fields:
 *      magic        - iterator magic number.
 *      currMemPtr   - the current memory address we're working on.
 *      currMemSize  - the current accumulated memory size.
 *      currIter - the current stage iterator.
 *      currStage - the current stage.
 *      currProtocolStack - the current Protocol stack.
 *      currVrId - the current vrId.
 *      currShadowIdx - shadow index.
 */
typedef struct IP_HSU_ITERATOR_STCT
{
    GT_U32                          magic;
    GT_VOID                         *currMemPtr;
    GT_U32                          currMemSize;
    GT_UINTPTR                      currIter;
    IP_HSU_STAGE_ENT                currStage;
    CPSS_IP_PROTOCOL_STACK_ENT      currProtocolStack;
    GT_U32                          currVrId;
    GT_U32                          currShadowIdx;
}IP_HSU_ITERATOR_STC;

/*
 * Typedef: struct IPV4_HSU_MC_ROUTE_ENTRY_STC
 *
 * Description: A structure to hold multicast route entry info. Used for HSU
 *              Pp synchronization.
 *
 * Fields:
 *      vrId                 - The virtual router identifier.
 *      mcGroup              - MC group IP address
 *      ipSrc                - MC source IP address
 *      ipSrcPrefixlength    - ip source prefix length
 *      mcRoutePointer       - Multicast Route Entry pointer
 *      last                 - weather this is the last entry
 */
typedef struct IPV4_HSU_MC_ROUTE_ENTRY_STCT
{
    GT_U32                                      vrId;
    GT_IPADDR                                   mcGroup;
    GT_IPADDR                                   ipSrc;
    GT_U32                                      ipSrcPrefixlength;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   mcRoutePointer;
    GT_BOOL                                     last;
}IPV4_HSU_MC_ROUTE_ENTRY_STC;

/*
 * Typedef: struct IPV6_HSU_MC_ROUTE_ENTRY_STC
 *
 * Description: A structure to hold multicast route entry info. Used for HSU
 *              Pp synchronization.
 *
 * Fields:
 *      vrId              - The virtual router identifier.
 *      mcGroup           - MC group IP address
 *      ipSrc             - MC source IP address
 *      ipSrcPrefixlength - ip source prefix length
 *      mcRoutePointer    - Multicast Route Entry pointer
 *      last              - weather this is the last entry
 *      groupScope        - the group scope.
 */
typedef struct IPV6_HSU_MC_ROUTE_ENTRY_STCT
{
    GT_U32                                      vrId;
    GT_IPV6ADDR                                 mcGroup;
    GT_IPV6ADDR                                 ipSrc;
    GT_U32                                      ipSrcPrefixlength;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   mcRoutePointer;
    GT_BOOL                                     last;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  groupScope;
}IPV6_HSU_MC_ROUTE_ENTRY_STC;

/*
 * Typedef: struct DMM_IP_HSU_ENTRY_STC
 *
 * Description: A structure to hold hsu DMM entry info. Used for HSU
 *              Pp synchronization.
 *
 * Fields:
 *      partitionIndex            - dmm partition index.
 *      currentFreeLinkList       - it shows for which memory size this list is intended.
 *      currentDmmBlockAddrInList - current DMM block in the list.
 *      endOfList                 - list was processed but end of list signature was not done.
 *      endOfPartition            - partition was processed but end of partition signature was
 *                                  not done
 *
 */
typedef struct DMM_IP_HSU_ENTRY_STCT
{
    GT_U32                                  partitionIndex;
    GT_U32                                  currentFreeLinkList;
    GT_DMM_BLOCK                            *currentDmmBlockAddrInList;
    GT_BOOL                                 endOfList;
    GT_BOOL                                 endOfPartition;
    GT_DMM_PARTITION                        *partitionArray[8];
}DMM_IP_HSU_ENTRY_STC;
#endif

/*******************************************************************************
**                             forward declarations                           **
*******************************************************************************/

/* Check validity of values of route entry pointer */
static GT_STATUS prvCpssDxChLpmRouteEntryPointerCheck
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT          shadowType,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  *routeEntryPointerPtr
);

/*******************************************************************************
**                               local functions                              **
*******************************************************************************/

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
* @retval GT_OUT_OF_RANGE          - on out of range parameter 
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamSip6FillCfg
(
    IN    CPSS_DXCH_LPM_RAM_CONFIG_STC      *memoryCfgPtr,
    OUT   PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC  *ramMemoryPtr
)
{
    GT_U32 numberOfPbrLines;
    GT_U32 bigBanksNumber;
    GT_U32 smallBanksNumber;
    GT_U32 smallBankSize = 640;
    GT_U32 numOfBigPbrBlocks = 0;
    GT_U32 numOfSmallPbrBlocks = 0;
    GT_U32 smallBankStartOffset = 10;
    GT_U32 i;
    CPSS_NULL_PTR_CHECK_MAC(memoryCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(ramMemoryPtr);
    numberOfPbrLines = memoryCfgPtr->maxNumOfPbrEntries/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
    if (memoryCfgPtr->maxNumOfPbrEntries%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS != 0) 
    {
        numberOfPbrLines++;
    }
    if (numberOfPbrLines > PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_PBR_SIZE_IN_LPM_LINES_CNS) 
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "bad number of PBR");
    }

    /* calculated PBR */
    switch (memoryCfgPtr->sharedMemCnfg) 
    {
    case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E:
       /* ramMemoryPtr->sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E;*/
        bigBanksNumber = ramMemoryPtr->bigBanksNumber = 28;
        smallBanksNumber = 2;
        smallBankStartOffset = 28;
        ramMemoryPtr->bigBankSize = 14*_1K;
        ramMemoryPtr->numOfBlocks = 30;
        break;
    case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E:
       /* ramMemoryPtr->sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E;*/
        bigBanksNumber = ramMemoryPtr->bigBanksNumber = 0;
        ramMemoryPtr->bigBankSize = 0;
        smallBanksNumber = 20;
        smallBankStartOffset = 10;
        ramMemoryPtr->numOfBlocks = 20;
        if (numberOfPbrLines > PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_PBR_SIZE_MIN_LPM_MODE_IN_LPM_LINES_CNS) 
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "bad number of PBR");
        }
        break;
    case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:
    case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:
       /* ramMemoryPtr->sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E;*/
        bigBanksNumber = ramMemoryPtr->bigBanksNumber = 24;
        smallBanksNumber = 6;
        ramMemoryPtr->bigBankSize = 7*_1K;
        smallBankStartOffset = 24;
        ramMemoryPtr->numOfBlocks = 30;
        break;
    default:
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bad share table mode");
    }
   /* ramMemoryPtr->maxNumberOfPbrEntries = memoryCfgPtr->maxNumOfPbrEntries;*/
    if (numberOfPbrLines > 0) 
    {
        if (numberOfPbrLines <= smallBankSize) 
        {
            if (smallBanksNumber > 0) 
            {
                /* take 1 bank from the end : it is only for PBR*/
                smallBanksNumber--;
                ramMemoryPtr->numOfBlocks--;
                numOfSmallPbrBlocks++;
            }
            else
            {
                /* num of small banks is 0*/
                ramMemoryPtr->numOfBlocks--;
                ramMemoryPtr->bigBanksNumber--;
                numOfBigPbrBlocks++;
            }
        }
        else
        {
            if (bigBanksNumber > 0) 
            {
                /* The max pbr value in lines is 32K. There is a differnce in max PBR lines value
                   for shared memory configuration modes. It is limited by pbr bank size register*/
                /* pbr banks size reg config must be done correspondingly for each device */
                if (numberOfPbrLines > 28*_1K) 
                {
                    /* in order to support more than 28K pbr we need use max number of big pbr blocks */
                    if ( (memoryCfgPtr->sharedMemCnfg == CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E) ||
                         (memoryCfgPtr->sharedMemCnfg == CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E) ) 
                    {
                        /* 7k size blocks. Pbr bank size configured to 4K */
                        numOfBigPbrBlocks = 8;
                    }
                    else /* CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E - max lpm configuration */
                    {
                        /* 14k size blocks. Pbr bank size configured to 8k */
                        numOfBigPbrBlocks = 4;
                    }
                }
                else
                {
                    /* calculate number of big banks needed for PBR */
                    numOfBigPbrBlocks = (numberOfPbrLines +(ramMemoryPtr->bigBankSize-1))/
                                      (ramMemoryPtr->bigBankSize);
                    if (numOfBigPbrBlocks >= bigBanksNumber) 
                    {
                        cpssOsPrintf("wrong configuration: numOfBigPbrBlocks =%d, bigBanksNumber =%d\n",
                                     numOfBigPbrBlocks,bigBanksNumber);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bad lpm-pbr configuration");
                    }
                    /* in case of max lpm configuration we need maximum 2 big banks : pbr bank size 16K*/
                    /* in case of medium lpm configuration we need 4 big banks : pbr bank size 16K*/
                }

                ramMemoryPtr->bigBanksNumber = ramMemoryPtr->bigBanksNumber - numOfBigPbrBlocks; 
                ramMemoryPtr->numOfBlocks = ramMemoryPtr->numOfBlocks - numOfBigPbrBlocks;

            }
            else
            {
                /* we have only small banks */
                /* calculate number of small banks needed for PBR */
                numOfSmallPbrBlocks = (numberOfPbrLines +(smallBankSize-1))/
                                  (smallBankSize);
                if (numOfSmallPbrBlocks >= smallBanksNumber) 
                {
                    cpssOsPrintf("wrong configuration: numOfSmallPbrBlocks =%d, smallBanksNumber =%d\n",
                                 numOfSmallPbrBlocks,smallBanksNumber);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bad lpm-pbr configuration");
                }
                smallBanksNumber = smallBanksNumber - numOfSmallPbrBlocks; 
                ramMemoryPtr->numOfBlocks = ramMemoryPtr->numOfBlocks - numOfSmallPbrBlocks;
                /* in case of min lpm configuration we need max 8 small banks : pbr bank size 4K*/
            }
        }
    }
    ramMemoryPtr->numberOfBigPbrBanks = numOfBigPbrBlocks;

    /* fill big banks data: banks used for PBR filled with 0 size */
    for (i = numOfBigPbrBlocks; i < bigBanksNumber; i++) 
    {
        ramMemoryPtr->blocksSizeArray[i] = ramMemoryPtr->bigBankSize;
    }
    /* if pbr resided in small block, it would be last */
    /* handle small blocks */
    for (i = 0; i < smallBanksNumber; i++) 
    {
        ramMemoryPtr->blocksSizeArray[i+smallBankStartOffset] = smallBankSize;
    }

    return GT_OK;
}




/**
* @internal deleteShadowStruct function
* @endinternal
*
* @brief   This function deletes existing shadow struct.
*
* @param[in] shadowPtr                - points to the shadow to delete
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS deleteShadowStruct
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr
)
{
    GT_STATUS       rc;
    GT_U32          i,octet;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol;
    GT_U32          numOfOctetsPerProtocol[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS] = {PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS,
                                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS,
                                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS};
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC startOfOctetList;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *tempNextMemInfoPtr;

    /* first make sure all virtual routers are deleted */
    for (i = 0 ; i < shadowPtr->vrfTblSize ; i++)
    {
        if (shadowPtr->vrRootBucketArray[i].valid == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    /* now free resources used by the shadow struct */

    /* free memory set to vr table array */
    cpssOsLpmFree(shadowPtr->vrRootBucketArray);

    /* free all memory pools allocated */
    for (i = 0 ; i < shadowPtr->numOfLpmMemories ; i++)
    {
        if(shadowPtr->lpmRamStructsMemPoolPtr[i]!=0)
        {
            rc = prvCpssDmmClosePartition(shadowPtr->lpmRamStructsMemPoolPtr[i]);
            if (rc != GT_OK)
            {
                 /* free alloctated arrays used for RAM configuration */
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);

                return  rc;
            }
        }
    }

    /* free alloctated arrays used for RAM configuration */
    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);

    /* go over all linked list per protocol for all octets and free it */
    for (protocol = 0; protocol < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; protocol++)
    {
        for (octet = 0; octet < numOfOctetsPerProtocol[protocol]; octet++)
        {
            startOfOctetList = shadowPtr->lpmMemInfoArray[protocol][octet];
            while(startOfOctetList.nextMemInfoPtr != NULL)
            {
                /* keep a temp pointer to the element in the list we need to free */
                tempNextMemInfoPtr = startOfOctetList.nextMemInfoPtr;
                /* assign a new next element for the startOfOctetList */
                startOfOctetList.nextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                /* free the memory kept in the temp pointer */
                cpssOsFree(tempNextMemInfoPtr);
            }
        }
    }

    /* free memory allocated to swap */
    if (shadowPtr->swapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->swapMemoryAddr);
    }

    /* free memory allocated to second swap */
    if (shadowPtr->secondSwapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->secondSwapMemoryAddr);
    }

    /* remove add devices from device list */
    if (shadowPtr->shadowDevList.shareDevs != NULL)
    {
        cpssOsLpmFree(shadowPtr->shadowDevList.shareDevs);
    }

    return GT_OK;
}

/**
* @internal deleteSip6ShadowStruct function
* @endinternal
*
* @brief   This function deletes existing shadow struct.
*
* @param[in] shadowPtr                - points to the shadow to delete
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS deleteSip6ShadowStruct
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr
)
{
    GT_STATUS       rc;
    GT_U32          i,octet;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol;
    GT_U32          numOfOctetsPerProtocol[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS] = {PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS,
                                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS,
                                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS};
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC startOfOctetList;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *tempNextMemInfoPtr;

    /* first make sure all virtual routers are deleted */
    for (i = 0 ; i < shadowPtr->vrfTblSize ; i++)
    {
        if (shadowPtr->vrRootBucketArray[i].valid == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    /* now free resources used by the shadow struct */

    /* free memory set to vr table array */
    cpssOsLpmFree(shadowPtr->vrRootBucketArray);

    /* free all memory pools allocated */
    for (i = 0 ; i < shadowPtr->numOfLpmMemories ; i++)
    {
        if(shadowPtr->lpmRamStructsMemPoolPtr[i]!=0)
        {
            rc = prvCpssDmmClosePartition(shadowPtr->lpmRamStructsMemPoolPtr[i]);
            if (rc != GT_OK)
            {
                 /* free alloctated arrays used for RAM configuration */
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);

                return  rc;
            }
        }
    }

    /* free alloctated arrays used for RAM configuration */
    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);

    /* go over all linked list per protocol for all octets and free it */
    for (protocol = 0; protocol < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; protocol++)
    {
        for (octet = 0; octet < numOfOctetsPerProtocol[protocol]; octet++)
        {
            startOfOctetList = shadowPtr->lpmMemInfoArray[protocol][octet];
            while(startOfOctetList.nextMemInfoPtr != NULL)
            {
                /* keep a temp pointer to the element in the list we need to free */
                tempNextMemInfoPtr = startOfOctetList.nextMemInfoPtr;
                /* assign a new next element for the startOfOctetList */
                startOfOctetList.nextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                /* free the memory kept in the temp pointer */
                cpssOsFree(tempNextMemInfoPtr);
            }
        }
    }

    /* free memory allocated to swap */
    if (shadowPtr->swapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->swapMemoryAddr);
    }

    /* free memory allocated to second swap */
    if (shadowPtr->secondSwapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->secondSwapMemoryAddr);
    }

    /* remove add devices from device list */
    if (shadowPtr->shadowDevList.shareDevs != NULL)
    {
        cpssOsLpmFree(shadowPtr->shadowDevList.shareDevs);
    }

    return GT_OK;
}


/**
* @internal createSip6ShadowStruct function
* @endinternal
*
* @brief   This function returns a pointer to a new shadow struct.
*
* @param[in] shadowType               - the type of shadow to maintain
* @param[in] protocolBitmap           - the protocols this shodow supports
* @param[in] memoryCfgPtr             - (pointer to) the memory configuration of this LPM DB
*
* @param[out] shadowPtr                - points to the created shadow struct
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS createSip6ShadowStruct
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT   shadowType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_BMP      protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC     *memoryCfgPtr,
    OUT PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr
)
{
    GT_U8 numOfDevs;        /* Number of devices in system.                                     */
    GT_STATUS retVal = GT_OK;
    GT_U32 memSize;         /* size of the memory block to be inserted to                       */
                            /* the memory management unit.                                      */
    GT_U32 memSizeBetweenBlocksInBytes; /* the size between each memory block management unit   */
    GT_DMM_BLOCK  *dmmBlockPtr;
    GT_DMM_BLOCK  *secondDmmBlockPtr;

    GT_U32      i,j,k = 0;
    /* GT_U32      maxSizeOfBucketInLpmLines;*/
    GT_U32      maxAllocSizeInLpmLines;
    GT_U32      sizeLpmEntryInBytes;
    GT_U32      minAllocationSizeInBytes;
    /* when regular node is going to be converted to compressed all 6 GONs must be reunited*/
    /* to one big one. maxGonSizeUnderDeleteForSwap is the maximal possible allocation place for it*/
    GT_U32      maxGonSizeUnderDeleteForSwap;
    GT_BOOL     isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E];
    GT_U32      smallBankFirstPossiblePosition = 10;
    GT_U32      startOffsetCompensation = 0;
    /* number of memories in falcon is not constant, must be taken from configuration. */
    GT_UINTPTR  structsMemPool[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    GT_U32      numOfOctetsPerProtocol[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E] = {PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS,
                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS,
                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS};
    /* zero out the shadow */
    cpssOsMemSet(shadowPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC));

    numOfDevs = PRV_CPSS_MAX_PP_DEVICES_CNS;
    maxAllocSizeInLpmLines = NUMBER_OF_RANGES_IN_SUBNODE*PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS;
    sizeLpmEntryInBytes = PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS;
    minAllocationSizeInBytes = DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS;
    maxGonSizeUnderDeleteForSwap = maxAllocSizeInLpmLines;

    shadowPtr->shadowType = shadowType;
    shadowPtr->lpmRamTotalBlocksSizeIncludingGap = PRV_CPSS_DXCH_LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_FALCON_CNS;

    /* update the protocols that the shadow needs to support */
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap);
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap);
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap);

    /* reset the to be freed memory list */
    shadowPtr->freeMemListDuringUpdate = NULL;
    shadowPtr->freeMemListEndOfUpdate = NULL;

    /* we keep the numOfBlocks value as the numOfLpmMemories.
       Block and Memory has the same meaning.
       We use numOfLpmMemories due to legacy code */
    shadowPtr->numOfLpmMemories = memoryCfgPtr->numOfBlocks;
    shadowPtr->bigBanksNumber = memoryCfgPtr->bigBanksNumber;
    shadowPtr->bigBankSize = memoryCfgPtr->bigBankSize;
    shadowPtr->smallBanksNumber = memoryCfgPtr->numOfBlocks - memoryCfgPtr->bigBanksNumber;

    /* allocate needed RAM configuration arrays */
    shadowPtr->lpmRamBlocksSizeArrayPtr = cpssOsMalloc(sizeof(GT_U32)*shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamBlocksSizeArrayPtr == NULL)
    {
        /* fail to allocate space */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    shadowPtr->lpmRamStructsMemPoolPtr = cpssOsMalloc(sizeof(GT_UINTPTR) * shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamStructsMemPoolPtr == NULL)
    {
        /* fail to allocate space */
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    shadowPtr->lpmRamOctetsToBlockMappingPtr = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamOctetsToBlockMappingPtr == NULL)
    {
        /* fail to allocate space */
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    /* reset needed RAM configuration arrays */
    cpssOsMemSet(shadowPtr->lpmRamBlocksSizeArrayPtr,0,sizeof(GT_U32)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->lpmRamStructsMemPoolPtr,0,sizeof(GT_UINTPTR)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->lpmRamOctetsToBlockMappingPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

    shadowPtr->lpmMemMode = memoryCfgPtr->lpmMemMode;

    shadowPtr->lpmRamBlocksAllocationMethod = memoryCfgPtr->blocksAllocationMethod; 
    if ( (memoryCfgPtr->bigBanksNumber + memoryCfgPtr->numberOfBigPbrBanks) < smallBankFirstPossiblePosition) 
    {
        startOffsetCompensation = smallBankFirstPossiblePosition - memoryCfgPtr->bigBanksNumber;
    }
    startOffsetCompensation = startOffsetCompensation + memoryCfgPtr->numberOfBigPbrBanks;
    shadowPtr->memoryOffsetValue = startOffsetCompensation;
    /* init the search memory pools */
    for (i = 0; i < shadowPtr->numOfLpmMemories + startOffsetCompensation; i++)
    {
        if (memoryCfgPtr->blocksSizeArray[i] == 0) 
        {
            /* it is used for PBR or we have hole when number of big banks less than 10 */
            continue;
        }
        shadowPtr->lpmRamBlocksSizeArrayPtr[k] = memoryCfgPtr->blocksSizeArray[i]; 
        /* Swap area is in the first block. In order to leave space for it, we
           deduct the size of a max LPM bucket */
        if (k == 0) 
        {
             /* in the first block there must be space for two swap areas */
            if (shadowPtr->lpmRamBlocksSizeArrayPtr[0] < maxGonSizeUnderDeleteForSwap)
            {
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }

            memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - maxGonSizeUnderDeleteForSwap;
            /* record the swap memory address (at the end of the structs memory)*/
            dmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
            if (dmmBlockPtr == NULL)
            {
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            SET_OFFSET_IN_WORDS(dmmBlockPtr, memSize);

            /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
    #if __WORDSIZE == 64
            dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
    #else
            dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
    #endif
            /* set first swap area */
            shadowPtr->swapMemoryAddr = (GT_UINTPTR)dmmBlockPtr;

            memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - (2*maxGonSizeUnderDeleteForSwap);
            /* record the swap memory address (at the end of the structs memory)*/
            secondDmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
            if (secondDmmBlockPtr == NULL)
            {
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                cpssOsLpmFree(dmmBlockPtr);/* free first swap area allocated */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            SET_OFFSET_IN_WORDS(secondDmmBlockPtr, memSize);

            /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
    #if __WORDSIZE == 64
            secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
    #else
            secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
    #endif

            /* set second swap area */
            shadowPtr->secondSwapMemoryAddr = (GT_UINTPTR)secondDmmBlockPtr;
        }
        else
        {
            memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[k];
        }

        memSizeBetweenBlocksInBytes  = shadowPtr->lpmRamTotalBlocksSizeIncludingGap * sizeLpmEntryInBytes;

        if (prvCpssDmmCreatePartition(minAllocationSizeInBytes * memSize,
                                      i * memSizeBetweenBlocksInBytes,
                                      minAllocationSizeInBytes,
                                      minAllocationSizeInBytes * maxAllocSizeInLpmLines,
                                      &structsMemPool[k]) != GT_OK)
        {

            cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
            cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
            cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* keep all memory pool Id's for future use, when binding octets to the blocks */
        shadowPtr->lpmRamStructsMemPoolPtr[k] = structsMemPool[k];
        k++;
    }
    if (k != shadowPtr->numOfLpmMemories) 
    {
        cpssOsPrintf(" wrong banks number!!!!!!!!!!! %d\n", shadowPtr->numOfLpmMemories);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* block 0 is used by default entries */
    shadowPtr->lpmRamOctetsToBlockMappingPtr[0].isBlockUsed=GT_TRUE;
    /* block 1 is used by vrf group of nodes */
    shadowPtr->lpmRamOctetsToBlockMappingPtr[1].isBlockUsed=GT_TRUE;

    for (i = 0; i < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; i++)
    {
        /* for all the protocols octet 0 is mapped to block 0 */
        shadowPtr->lpmMemInfoArray[i][0].structsMemPool = shadowPtr->lpmRamStructsMemPoolPtr[0];
        shadowPtr->lpmMemInfoArray[i][0].ramIndex = 0;
        shadowPtr->lpmMemInfoArray[i][0].structsBase = 0;
        shadowPtr->lpmMemInfoArray[i][0].nextMemInfoPtr = NULL;

        /* for all the protocols octet 1 is mapped to block 1 */
        shadowPtr->lpmMemInfoArray[i][1].structsMemPool = shadowPtr->lpmRamStructsMemPoolPtr[1];
        shadowPtr->lpmMemInfoArray[i][1].ramIndex = 1;
        shadowPtr->lpmMemInfoArray[i][1].structsBase = 0;
        shadowPtr->lpmMemInfoArray[i][1].nextMemInfoPtr = NULL;

        for (j = 2; j < numOfOctetsPerProtocol[i]; j++)
        {
            /* all blocks except 0 are not mapped */
            shadowPtr->lpmMemInfoArray[i][j].structsMemPool = 0;
            shadowPtr->lpmMemInfoArray[i][j].ramIndex = 0;
            shadowPtr->lpmMemInfoArray[i][j].structsBase = 0;
            shadowPtr->lpmMemInfoArray[i][j].nextMemInfoPtr = NULL;
        }

        /* block 0 is used by all the protocols  - mark bit 0 as 0x1*/
        PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,i,0,0);
        /* block 1 is used by all the protocols  - mark bit 1 as 0x1*/
        PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,i,1,1);
    }

    /* IPv4 init the lpm level's memory usage */
    /* meaning of i - octet in IP address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];

        /* for the mc grp search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];

        /* for the mc src search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i+PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];
    }

     /* IPv6 init the lpm level's memory usage */
     /* meaning of i - octet in IP address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];

        /* for the mc grp search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];

        /* for the mc src search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i+PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];
    }

    /* FCoE init the lpm level's memory usage */
    /* meaning of i - octet in FCoE address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E][i];
    }

    /* Allocate the device list */
    shadowPtr->shadowDevList.shareDevs = (GT_U8*)cpssOsLpmMalloc(sizeof(GT_U8) * numOfDevs);

    if (shadowPtr->shadowDevList.shareDevs == NULL)
    {
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* set the working devlist (which is empty curretly)*/
    shadowPtr->workDevListPtr = &shadowPtr->shadowDevList;

    cpssOsMemCpy(shadowPtr->isProtocolInitialized, isProtocolInitialized, PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E * sizeof(GT_BOOL));

    shadowPtr->vrfTblSize = PRV_CPSS_DXCH_LPM_RAM_NUM_OF_VIRTUAL_ROUTERS_CNS;
    shadowPtr->vrRootBucketArray =
        cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_STC) * shadowPtr->vrfTblSize);
    if (shadowPtr->vrRootBucketArray == NULL)
    {
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    for (i = 0; i < shadowPtr->vrfTblSize; i++)
    {
        shadowPtr->vrRootBucketArray[i].valid = GT_FALSE;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = NULL;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = NULL;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] = NULL;
    }

    return retVal;
}



/**
* @internal createShadowStruct function
* @endinternal
*
* @brief   This function returns a pointer to a new shadow struct.
*
* @param[in] shadowType               - the type of shadow to maintain
* @param[in] protocolBitmap           - the protocols this shodow supports
* @param[in] memoryCfgPtr             - (pointer to) the memory configuration of this LPM DB
*
* @param[out] shadowPtr                - points to the created shadow struct
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS createShadowStruct
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT   shadowType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_BMP      protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC     *memoryCfgPtr,
    OUT PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr
)
{
    GT_U8 numOfDevs;        /* Number of devices in system.                                     */
    GT_STATUS retVal = GT_OK;
    GT_U32 memSize;         /* size of the memory block to be inserted to                       */
                            /* the memory management unit.                                      */
    GT_U32 memSizeBetweenBlocksInBytes; /* the size between each memory block management unit   */
    GT_DMM_BLOCK  *dmmBlockPtr;
    GT_DMM_BLOCK  *secondDmmBlockPtr;

    GT_U32      i,j;
    GT_BOOL     isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E];
    GT_UINTPTR  structsMemPool[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    GT_U32      numOfOctetsPerProtocol[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E] = {PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS,
                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS,
                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS};

    numOfDevs = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* zero out the shadow */
    cpssOsMemSet(shadowPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC));
    shadowPtr->shadowType = shadowType;

    /* update the protocols that the shadow needs to support */
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap);
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap);
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap);

    /* reset the to be freed memory list */
    shadowPtr->freeMemListDuringUpdate = NULL;
    shadowPtr->freeMemListEndOfUpdate = NULL;

    /* we keep the numOfBlocks value as the numOfLpmMemories.
       Block and Memory has the same meaning.
       We use numOfLpmMemories due to legacy code */
    shadowPtr->numOfLpmMemories = memoryCfgPtr->numOfBlocks;

    /* allocate needed RAM configuration arrays */
    shadowPtr->lpmRamBlocksSizeArrayPtr = cpssOsMalloc(sizeof(GT_U32)*shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamBlocksSizeArrayPtr == NULL)
    {
        /* fail to allocate space */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    shadowPtr->lpmRamStructsMemPoolPtr = cpssOsMalloc(sizeof(GT_UINTPTR) * shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamStructsMemPoolPtr == NULL)
    {
        /* fail to allocate space */
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    shadowPtr->lpmRamOctetsToBlockMappingPtr = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamOctetsToBlockMappingPtr == NULL)
    {
        /* fail to allocate space */
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    /* reset needed RAM configuration arrays */
    cpssOsMemSet(shadowPtr->lpmRamBlocksSizeArrayPtr,0,sizeof(GT_U32)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->lpmRamStructsMemPoolPtr,0,sizeof(GT_UINTPTR)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->lpmRamOctetsToBlockMappingPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

    shadowPtr->lpmMemMode = memoryCfgPtr->lpmMemMode;

    for (i=0;i<shadowPtr->numOfLpmMemories;i++)
    {
         shadowPtr->lpmRamBlocksSizeArrayPtr[i] = memoryCfgPtr->blocksSizeArray[i];

         if(shadowPtr->lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
         {
            shadowPtr->lpmRamBlocksSizeArrayPtr[i]/=2;
         }
    }


    shadowPtr->lpmRamTotalBlocksSizeIncludingGap = PRV_CPSS_DXCH_LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_CNS;
    shadowPtr->lpmRamBlocksAllocationMethod = memoryCfgPtr->blocksAllocationMethod;



    /* init the search memory pools */
    for (i = 0; i < shadowPtr->numOfLpmMemories; i++)
    {
        /* Swap area is in the first block. In order to leave space for it, we
           deduct the size of a max LPM bucket */
        if (i == 0)
        {
             /* in the first block there must be space for two swap areas */
            if (shadowPtr->lpmRamBlocksSizeArrayPtr[0] < PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * 2)
            {
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }

            memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS;
            /* record the swap memory address (at the end of the structs memory)*/
            dmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
            if (dmmBlockPtr == NULL)
            {
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            SET_OFFSET_IN_WORDS(dmmBlockPtr, memSize);

            /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
    #if __WORDSIZE == 64
            dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
    #else
            dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
    #endif
            /* set first swap area */
            shadowPtr->swapMemoryAddr = (GT_UINTPTR)dmmBlockPtr;

            memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - (2*PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS);
            /* record the swap memory address (at the end of the structs memory)*/
            secondDmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
            if (secondDmmBlockPtr == NULL)
            {
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                cpssOsLpmFree(dmmBlockPtr);/* free first swap area allocated */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            SET_OFFSET_IN_WORDS(secondDmmBlockPtr, memSize);

            /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
    #if __WORDSIZE == 64
            secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
    #else
            secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
    #endif

            /* set second swap area */
            shadowPtr->secondSwapMemoryAddr = (GT_UINTPTR)secondDmmBlockPtr;
        }
        else
        {
            memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[i];
        }

        memSizeBetweenBlocksInBytes  = shadowPtr->lpmRamTotalBlocksSizeIncludingGap * PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS;

        if (prvCpssDmmCreatePartition(DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * memSize,
                                      i * memSizeBetweenBlocksInBytes,
                                      DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS,
                                      DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS,
                                      &structsMemPool[i]) != GT_OK)
        {

            cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
            cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
            cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* keep all memory pool Id's for future use, when binding octets to the blocks */
        shadowPtr->lpmRamStructsMemPoolPtr[i] = structsMemPool[i];
    }

    /* block 0 is used by default entries */
    shadowPtr->lpmRamOctetsToBlockMappingPtr[0].isBlockUsed=GT_TRUE;

    for (i = 0; i < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; i++)
    {
        /* for all the protocols octet 0 is mapped to block 0 */
        shadowPtr->lpmMemInfoArray[i][0].structsMemPool = shadowPtr->lpmRamStructsMemPoolPtr[0];
        shadowPtr->lpmMemInfoArray[i][0].ramIndex = 0;
        shadowPtr->lpmMemInfoArray[i][0].structsBase = 0;
        shadowPtr->lpmMemInfoArray[i][0].nextMemInfoPtr = NULL;

        for (j = 1; j < numOfOctetsPerProtocol[i]; j++)
        {
            /* all blocks except 0 are not mapped */
            shadowPtr->lpmMemInfoArray[i][j].structsMemPool = 0;
            shadowPtr->lpmMemInfoArray[i][j].ramIndex = 0;
            shadowPtr->lpmMemInfoArray[i][j].structsBase = 0;
            shadowPtr->lpmMemInfoArray[i][j].nextMemInfoPtr = NULL;
        }

        /* block 0 is used by all the protocols  - mark bit 0 as 0x1*/
        PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,i,0,0);
    }

    /* IPv4 init the lpm level's memory usage */
    /* meaning of i - octet in IP address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];

        /* for the mc grp search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];

        /* for the mc src search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i+PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];
    }

     /* IPv6 init the lpm level's memory usage */
     /* meaning of i - octet in IP address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];

        /* for the mc grp search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];

        /* for the mc src search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i+PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];
    }

    /* FCoE init the lpm level's memory usage */
    /* meaning of i - octet in FCoE address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E][i];
    }

    /* Allocate the device list */
    shadowPtr->shadowDevList.shareDevs = (GT_U8*)cpssOsLpmMalloc(sizeof(GT_U8) * numOfDevs);

    if (shadowPtr->shadowDevList.shareDevs == NULL)
    {
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* set the working devlist (which is empty curretly)*/
    shadowPtr->workDevListPtr = &shadowPtr->shadowDevList;

    cpssOsMemCpy(shadowPtr->isProtocolInitialized, isProtocolInitialized, PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E * sizeof(GT_BOOL));

    shadowPtr->vrfTblSize = PRV_CPSS_DXCH_LPM_RAM_NUM_OF_VIRTUAL_ROUTERS_CNS;
    shadowPtr->vrRootBucketArray =
        cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_STC) * shadowPtr->vrfTblSize);
    if (shadowPtr->vrRootBucketArray == NULL)
    {
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    for (i = 0; i < shadowPtr->vrfTblSize; i++)
    {
        shadowPtr->vrRootBucketArray[i].valid = GT_FALSE;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = NULL;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = NULL;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] = NULL;
    }

    return retVal;
}

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
)
{
    GT_STATUS                            retVal = GT_OK;
    GT_U32                               shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr;

    /* right now only one shadow configuration can be attached to LPM DB and
       only PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E is supported */
    GT_U32                              numOfShadowCfg = 1;

    *lpmDbPtrPtr = (PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)
                    cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC));
    if (*lpmDbPtrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    lpmDbPtr = *lpmDbPtrPtr;

    /* allocate memory */
    lpmDbPtr->shadowArray =
        cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC) * numOfShadowCfg);
    if (lpmDbPtr->shadowArray == NULL)
    {
        cpssOsLpmFree(lpmDbPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /*copy out the lpm db info */
    lpmDbPtr->protocolBitmap = protocolBitmap;
    lpmDbPtr->numOfShadowCfg = numOfShadowCfg;

    /* now initlize the shadows */
    for (shadowIdx = 0; (shadowIdx < numOfShadowCfg) && (retVal == GT_OK);
          shadowIdx++)
    {
        if (shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = createSip6ShadowStruct(shadowType,
                                            protocolBitmap,
                                            memoryCfgPtr,
                                            &lpmDbPtr->shadowArray[shadowIdx]);
        }
        else
        {
            retVal = createShadowStruct(shadowType,
                                            protocolBitmap,
                                            memoryCfgPtr,
                                            &lpmDbPtr->shadowArray[shadowIdx]);
        }
    }

    return retVal;
}

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
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;AC3X;
*                                  Bobcat3; Aldrin2; Falcon.
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
)
{
    GT_STATUS                           rc;         /* function return code */
    GT_U32                              i;

    /* make sure the lpm db id exists */
    /* delete all shadows */
    for (i = 0 ; i < lpmDbPtr->numOfShadowCfg ; i++)
    {
        if (shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            rc = deleteSip6ShadowStruct(&lpmDbPtr->shadowArray[i]);
        }
        else
        {
            rc = deleteShadowStruct(&lpmDbPtr->shadowArray[i]);

        }
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* free resources used by the lpm db */
    if (lpmDbPtr->shadowArray != NULL)
    {
        cpssOsLpmFree(lpmDbPtr->shadowArray);
    }

    cpssOsLpmFree(lpmDbPtr);

    return GT_OK;
}

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
)
{
    GT_U32 i=0;

    if (protocolBitmapPtr!=NULL)
    {
        *protocolBitmapPtr = lpmDbPtr->protocolBitmap;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if (memoryCfgPtr!=NULL)
    {
        memoryCfgPtr->numOfBlocks = lpmDbPtr->shadowArray[0].numOfLpmMemories;
        for (i=0;i<memoryCfgPtr->numOfBlocks;i++)
        {
            memoryCfgPtr->blocksSizeArray[i] = lpmDbPtr->shadowArray[0].lpmRamBlocksSizeArrayPtr[i];
        }
        memoryCfgPtr->blocksAllocationMethod = lpmDbPtr->shadowArray[0].lpmRamBlocksAllocationMethod;
        memoryCfgPtr->lpmMemMode = lpmDbPtr->shadowArray[0].lpmMemMode;
        memoryCfgPtr->bigBankSize = lpmDbPtr->shadowArray[0].bigBankSize;
        memoryCfgPtr->bigBanksNumber = lpmDbPtr->shadowArray[0].bigBanksNumber;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal isProtocolSetInBitmap function
* @endinternal
*
* @brief   Check whether the protocol is set in the protocols bitmap
*
* @param[in] protocol                 - bitmap of protocols
* @param[in] protocol                 - the protocol
*
* @retval GT_TRUE                  - the protocol bit is set in the bitmap, or
* @retval GT_FALSE                 - the protocol bit is not set in the bitmap
*/
static GT_BOOL isProtocolSetInBitmap
(
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP          protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol
)
{
    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        return PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap);
    }
    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
    {
        return PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap);
    }
    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        return PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap);
    }
    return GT_FALSE;
}

/**
* @internal updateHwSearchStc function
* @endinternal
*
* @brief   updates the search structures accroding to the parameters passed
*
* @param[in] updateType               - indicates which and how entries will be updated
* @param[in] protocolBitmap           - bitmap of protocols to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS updateHwSearchStc
(
    IN PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_TYPE_ENT updateType,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP                   protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                 *shadowPtr
)
{
    GT_STATUS                                       retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          tmpRootRange;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_TYPE_ENT      lpmTrieUpdateType;
    GT_U32                                          vrId;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            currProtocolStack;

    for (currProtocolStack = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
          currProtocolStack < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
          currProtocolStack++)
    {
        if ((isProtocolSetInBitmap(protocolBitmap, currProtocolStack) == GT_FALSE) ||
            (shadowPtr->isProtocolInitialized[currProtocolStack] == GT_FALSE))
        {
            continue;
        }
        for (vrId = 0 ; vrId < shadowPtr->vrfTblSize; vrId++)
        {
            /* go over all valid VR , if it's initilized for this protocol */
            if ((shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE) ||
                (shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack] == NULL))
            {
                continue;
            }

            /* if we are in update mode and the vr doesn't need an update , don't
               update */
            if ((updateType != PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E) &&
                (shadowPtr->vrRootBucketArray[vrId].needsHwUpdate == GT_FALSE))
            {
                continue;
            }

            retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                                     &shadowPtr->freeMemListDuringUpdate,shadowPtr);
            if (retVal != GT_OK)
                return retVal;

            if (shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack])
            {
                switch (updateType)
                {
                case PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E:
                    lpmTrieUpdateType = PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_ONLY_E;
                    break;
                case PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E:
                    lpmTrieUpdateType = PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_AND_ALLOC_MEM_E;
                    break;
                case PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E:
                    lpmTrieUpdateType = PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E;
                    break;
                CPSS_COVERITY_NON_ISSUE_BOOKMARK
                /* coverity[dead_error_begin] */
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                /* write the changes in the tree */
                tmpRootRange.lowerLpmPtr.nextBucket =
                    shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack];
                tmpRootRange.pointerType =
                    (GT_U8)(shadowPtr->vrRootBucketArray[vrId].rootBucketType[currProtocolStack]);
                tmpRootRange.next = NULL;
                retVal =
                    prvCpssDxChLpmRamMngBucketTreeWrite(&tmpRootRange,
                                                        shadowPtr->ucSearchMemArrayPtr[currProtocolStack],
                                                        shadowPtr,lpmTrieUpdateType,
                                                        currProtocolStack,vrId);
                if (retVal == GT_OK)
                {
                    /* now write the vr table */
                    retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId, currProtocolStack, shadowPtr);
                    if (retVal == GT_OK)
                    {
                        /* indicate the update was done */
                        shadowPtr->vrRootBucketArray[vrId].needsHwUpdate = GT_FALSE;

                    }
                }
            }

            /* now it's possible to free all the "end of update to be freed
               memory" memory , now that the HW is updated (and the "during
               update to be freed" memory if needed ) */
            retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                     &shadowPtr->freeMemListDuringUpdate,shadowPtr);

            if (retVal != GT_OK)
            {
                return retVal;
            }

            retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                     &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

            if (retVal != GT_OK)
            {
                return retVal;
            }
        }
    }

    return (retVal);
}

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
)
{
    GT_STATUS                                   retVal = GT_OK;
    GT_U32                                      shadowIdx, devIdx, i;
    static GT_U8                                shadowDevList[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                                      numOfShadowDevs=0;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC  addedDevList;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC  *shadowDevListPtr;
    GT_U32                                      totalLpmSize=0,lpmRamNumOfLines=0;
    GT_BOOL                                     memoryModeFits = GT_TRUE;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC             *moduleCfgPtr;



    /* first check the devices we got fit the lpm DB shadows */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        /*Lock the access to per device data base in order to avoid corruption*/
        CPSS_API_LOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        PRV_CPSS_DXCH_DEV_CHECK_MAC(devListArray[devIdx]);
        moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devListArray[devIdx])->moduleCfg);
        for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
        {
            /* check if shadow fit to devices*/
            if ( (lpmDbPtr->shadowArray[shadowIdx].shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E) &&
                 (!PRV_CPSS_SIP_5_CHECK_MAC(devListArray[devIdx])))
            {
                /*Unlock the access to per device data base*/
                CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if ( (lpmDbPtr->shadowArray[shadowIdx].shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E) &&
                 (!PRV_CPSS_SIP_6_CHECK_MAC(devListArray[devIdx])))
            {
                /*Unlock the access to per device data base*/
                CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            totalLpmSize = 0;
            /* calculate the size of the RAM in this LPM DB */
            for (i=0;i<lpmDbPtr->shadowArray[shadowIdx].numOfLpmMemories;i++)
            {
                totalLpmSize += lpmDbPtr->shadowArray[shadowIdx].lpmRamBlocksSizeArrayPtr[i];
            }
            /* check if the device has enough RAM space to contain the LPM DB configuration */
            if (lpmDbPtr->shadowArray[shadowIdx].shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E) 
            {
                 lpmRamNumOfLines = PRV_CPSS_DXCH_SIP6_LPM_RAM_GET_NUM_OF_LINES_MAC(PRV_CPSS_DXCH_PP_MAC(devListArray[devIdx])->fineTuning.tableSize.lpmRam);
            }
            else
            {
                lpmRamNumOfLines = PRV_CPSS_DXCH_LPM_RAM_GET_NUM_OF_LINES_MAC(PRV_CPSS_DXCH_PP_MAC(devListArray[devIdx])->fineTuning.tableSize.lpmRam); 
            }
            
            /*Check if memory mode fits*/
            if(lpmDbPtr->shadowArray[shadowIdx].shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E)
            {
                if(lpmDbPtr->shadowArray[shadowIdx].lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
                {
                    if(PRV_CPSS_DXCH_PP_MAC(devListArray[devIdx])->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E)
                    {
                        memoryModeFits = GT_FALSE;
                    }
                    else
                    /*if we are working in half memory mode then we have half of the lines*/
                    {
                        lpmRamNumOfLines/=2;
                    }
                }
                else
                {
                    if(PRV_CPSS_DXCH_PP_MAC(devListArray[devIdx])->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
                    {
                        memoryModeFits = GT_FALSE;
                    }
                }

                if(memoryModeFits == GT_FALSE)
                {
                    /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    /* not compatible memory mode */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL , LOG_ERROR_NO_MSG);
                }

            }
            if(lpmDbPtr->shadowArray[shadowIdx].shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
            {
                if( (lpmRamNumOfLines - moduleCfgPtr->ip.maxNumOfPbrEntries) < totalLpmSize)
                {
                    /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    /* not enough memory in the device */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }
            }
            else
            {    /* SIP6 devices*/
                if(!((moduleCfgPtr->ip.numOfBigBanks >= lpmDbPtr->shadowArray[shadowIdx].bigBanksNumber) &&
                     (moduleCfgPtr->ip.numOfSmallBanks >= lpmDbPtr->shadowArray[shadowIdx].smallBanksNumber)&&
                     (moduleCfgPtr->ip.lpmBankSize >= lpmDbPtr->shadowArray[shadowIdx].bigBankSize) &&
                     (moduleCfgPtr->ip.numOfBigBanks+moduleCfgPtr->ip.numOfSmallBanks >= lpmDbPtr->shadowArray[shadowIdx].numOfLpmMemories) ))
                {
                    /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    /* not enough memory in the device */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }
                if( (lpmRamNumOfLines - (moduleCfgPtr->ip.maxNumOfPbrEntries/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS)) < totalLpmSize)
                {
                    /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    /* not enough memory in the device */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }


            }

            shadowDevListPtr =
                &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            /* ok the device fits, see that it doesn't exist already */
            for (i = 0 ; i < shadowDevListPtr->shareDevNum;i++)
            {
                if (shadowDevListPtr->shareDevs[i] == devListArray[devIdx])
                {
                   /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                   /* the device exists, can't re-add it */
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
                }
            }
        }

        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    }

    /* if we reached here, all the added devices are ok, add them */
    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        numOfShadowDevs = 0;
        for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
        {
            shadowDevList[numOfShadowDevs] = devListArray[devIdx];
            numOfShadowDevs++;
        }
        /* check if we have added device for the shadow , and if there are add
           them */
        if (numOfShadowDevs > 0)
        {
            addedDevList.shareDevNum = numOfShadowDevs;
            addedDevList.shareDevs   = shadowDevList;

            /* change the working devlist*/
            lpmDbPtr->shadowArray[shadowIdx].workDevListPtr = &addedDevList;

            /* write the lpm data to the added devices */
            retVal = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E,
                                       lpmDbPtr->protocolBitmap,
                                       &lpmDbPtr->shadowArray[shadowIdx]);
            if (retVal != GT_OK)
            {
                return (retVal);
            }

            /* change back the working devlist*/
            lpmDbPtr->shadowArray[shadowIdx].workDevListPtr =
                &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            /* now register them */
            shadowDevListPtr = &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;
            for (i = 0; i < numOfShadowDevs; i++)
            {
                shadowDevListPtr->shareDevs[shadowDevListPtr->shareDevNum] =
                    shadowDevList[i];
                shadowDevListPtr->shareDevNum++;
            }
        }
    }
    return GT_OK;
}

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
)
{
    GT_U32                                      shadowIdx, devIdx, i, j;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC  *shadowDevListPtr;
    GT_BOOL                                     devExists;

    /* first check the devices we got are ok */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        /*Lock the access to per device data base in order to avoid corruption*/
        CPSS_API_LOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        PRV_CPSS_DXCH_DEV_CHECK_MAC(devListArray[devIdx]);

        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    /* first check that the devices exist in the shadows */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
        {
            shadowDevListPtr = &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            devExists = GT_FALSE;
            for (i = 0 ; i < shadowDevListPtr->shareDevNum;i++)
            {
                if (shadowDevListPtr->shareDevs[i] == devListArray[devIdx])
                {
                    /* found it */
                    devExists = GT_TRUE;
                    break;
                }
            }
            if (devExists == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* now go and remove them from their shadow */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
        {
            shadowDevListPtr = &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            /* ok the device fits, see that it doesn't exist already */
            for (i = 0 ; i < shadowDevListPtr->shareDevNum;i++)
            {
                if (shadowDevListPtr->shareDevs[i] == devListArray[devIdx])
                {
                    /* found it - remove it*/
                    for (j = i ; j < shadowDevListPtr->shareDevNum - 1; j++)
                    {
                        shadowDevListPtr->shareDevs[j] =
                            shadowDevListPtr->shareDevs[j+1];
                    }
                    shadowDevListPtr->shareDevNum--;
                }
            }
        }
    }
    return GT_OK;
}

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
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC *shadowDevListPtr;

    shadowDevListPtr = &lpmDbPtr->shadowArray[0].shadowDevList;

    if (*numOfDevsPtr < shadowDevListPtr->shareDevNum)
    {
        *numOfDevsPtr = shadowDevListPtr->shareDevNum;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
    }

    for (i = 0 ; i < shadowDevListPtr->shareDevNum ; i++)
    {
        devListArray[i] = shadowDevListPtr->shareDevs[i];
    }
    *numOfDevsPtr = shadowDevListPtr->shareDevNum;

    return GT_OK;
}



/**
* @internal removeSupportedProtocolsFromVirtualRouter function
* @endinternal
*
* @brief   This function removes all supported protocols from virtual router for specific shadow.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] vrId                     - The virtual's router ID.
* @param[in] shadowPtr                - the shadow to work on.
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_BAD_STATE             - if the existing VR is not empty.
*
* @note The VR must be empty from prefixes in order to be Removed!
*
*/
static GT_STATUS removeSupportedProtocolsFromVirtualRouter
(
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr
)
{
    GT_STATUS                                 retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protStk;
    GT_UINTPTR                                handle;
    GT_U32                                    blockIndex;
    GT_U8                                     i;

    for (protStk = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E; protStk < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; protStk++)
    {
        if (shadowPtr->vrRootBucketArray[vrId].rootBucket[protStk] == NULL)
        {
            continue;
        }
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
            {
                handle = shadowPtr->vrRootBucketArray[vrId].rootBucket[protStk]->hwGroupOffsetHandle[i];

                /*need to free memory*/
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(handle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                /* set pending flag for future need */
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates += PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(handle);

                prvCpssDmmFree(handle);
            }
        }
        handle = shadowPtr->vrRootBucketArray[vrId].rootBucket[protStk]->hwBucketOffsetHandle;
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            /*need to free memory*/
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(handle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
            /* set pending flag for future need */
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates += PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(handle);
        }
        else
        {
            /*need to free memory*/
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(handle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            /* set pending flag for future need */
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates += PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(handle);
        }
        prvCpssDmmFree(handle);
        retVal = prvCpssDxChLpmRamMngRootBucketDelete(shadowPtr, vrId, protStk);
        if (retVal != GT_OK)
        {
            /* we fail in allocation, reset pending array */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            return retVal;
        }

        shadowPtr->vrRootBucketArray[vrId].rootBucket[protStk] = NULL;
        /* update counters for UC allocation */
        retVal = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                           shadowPtr->pendingBlockToUpdateArr,
                                                           shadowPtr->protocolCountersPerBlockArr,
                                                           shadowPtr->pendingBlockToUpdateArr,
                                                           protStk,
                                                           shadowPtr->numOfLpmMemories);
        if (retVal!=GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }
    }
    return retVal;
}

/**
* @internal lpmVirtualRouterSet function
* @endinternal
*
* @brief   This function sets a virtual router in system for specific shadow.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] vrId                     - The virtual's router ID.
* @param[in] defIpv4UcNextHopPointer  - the pointer info of the default IPv4 UC next
*                                      hop for this virtual router.
* @param[in] defIpv6UcNextHopPointer  - the pointer info of the default IPv6 UC next
*                                      hop for this virtual router.
* @param[in] defFcoeNextHopPointer    - the pointer info of the default FCoE next
*                                      hop for this virtual router.
* @param[in] defIpv4McRoutePointer    - the pointer info of the default IPv4 MC route
*                                      for this virtual router.
* @param[in] defIpv6McRoutePointer    - the pointer info of the default IPv6 MC route
*                                      for this virtual router.
* @param[in] protocolBitmap           - types of protocol stack used in this virtual router.
* @param[in] ucSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS] - Boolean array stating UC support for every protocol stack
* @param[in] mcSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS] - Boolean array stating MC support for every protocol stack
* @param[in] shadowPtr                - the shadow to work on.
* @param[in] updateHw                 - GT_TRUE : update the VR in the HW
*                                      GT_FALSE : do not update the VR in the HW
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_ALREADY_EXIST         - if the vr id is already used
*/
static GT_STATUS lpmVirtualRouterSet
(
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv4UcNextHopPointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv6UcNextHopPointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defFcoeNextHopPointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv4McRoutePointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv6McRoutePointer,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP               protocolBitmap,
    IN GT_BOOL                                      ucSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS],
    IN GT_BOOL                                      mcSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS],
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_BOOL                                      updateHw
)
{
    GT_STATUS                                       retVal;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            protStk;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defUcNextHopEntry;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defReservedNextHopEntry;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defMcNextHopEntry;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       defUcNextHopPointer = {0};
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       defMcNextHopPointer = {0};
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       defReservedNextHopPointer = {0};

    /* make compiler silent */
    defUcNextHopPointer.routeEntryBaseMemAddr = defUcNextHopPointer.routeEntryMethod = 0;

    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    for (protStk = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
         protStk < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
         protStk++)
    {
        defUcNextHopEntry = NULL;
        defReservedNextHopEntry = NULL;
        defMcNextHopEntry = NULL;

        if (isProtocolSetInBitmap(protocolBitmap, protStk) == GT_FALSE)
        {
            if (protStk < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS)
            {
                shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protStk] = GT_FALSE;
            }
            if (protStk < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS)
            {
                shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protStk] = GT_FALSE;
            }
            continue;
        }

        if (protStk <= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            if ((ucSupportArr[protStk] == GT_FALSE) && (mcSupportArr[protStk] == GT_FALSE))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            if (ucSupportArr[protStk] == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        /* init UC structures */
        if (ucSupportArr[protStk] == GT_TRUE)
        {
            shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protStk] = GT_TRUE;
            if (protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                defUcNextHopPointer = defIpv4UcNextHopPointer;
                defReservedNextHopPointer = defIpv4UcNextHopPointer;
            }
            else if (protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
            {
                defUcNextHopPointer = defIpv6UcNextHopPointer;
            }
            else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E */
            {
                defUcNextHopPointer = defFcoeNextHopPointer;
            }

            /* allocate space for the new default */
            defUcNextHopEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            if (defUcNextHopEntry == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
            cpssOsMemCpy(defUcNextHopEntry, &defUcNextHopPointer,
                         sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            if (protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                defReservedNextHopEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                if (defReservedNextHopEntry == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
                cpssOsMemCpy(defReservedNextHopEntry, &defReservedNextHopPointer,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            }

        }
        else
        {
            shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protStk] = GT_FALSE;
        }

        /* now init mc stuctures */
        if (protStk <= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            if (mcSupportArr[protStk] == GT_TRUE)
            {
                shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protStk] = GT_TRUE;
                if (protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
                {
                    defMcNextHopPointer = defIpv4McRoutePointer;
                }
                else /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
                {
                    defMcNextHopPointer = defIpv6McRoutePointer;
                }

                /* In the following cases we need to create a new nexthop pointer
                   info structure:
                   1. UC is not supported for this protocol so we didn't create it yet
                   2. UC is supported but the defaults of the UC and MC are different */
                if ((ucSupportArr[protStk] == GT_FALSE) ||
                    (defUcNextHopPointer.routeEntryBaseMemAddr != defMcNextHopPointer.routeEntryBaseMemAddr) ||
                    (defUcNextHopPointer.routeEntryMethod != defMcNextHopPointer.routeEntryMethod))
                {
                    defMcNextHopEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                    if (defMcNextHopEntry == NULL)
                    {
                        if (ucSupportArr[protStk] == GT_TRUE)
                        {
                             cpssOsLpmFree(defUcNextHopEntry);
                        }
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }

                    cpssOsMemCpy(defMcNextHopEntry, &defMcNextHopPointer,
                                 sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                }
                else
                {
                    /* both UC and MC are supported, with the same default NH */
                    defMcNextHopEntry = defUcNextHopEntry;
                }
            }
            else    /* mcSupportArr[protStk] == GT_FALSE */
            {
                shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protStk] = GT_FALSE;
            }
        }

        /* Create the root bucket */
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamMngRootBucketCreate(shadowPtr, vrId, protStk,
                                                              defUcNextHopEntry,
                                                              defReservedNextHopEntry,
                                                              defMcNextHopEntry,
                                                              updateHw);
        }
        else
        {
            retVal = prvCpssDxChLpmRamMngRootBucketCreate(shadowPtr, vrId, protStk,
                                                          defUcNextHopEntry,
                                                          defReservedNextHopEntry,
                                                          defMcNextHopEntry,
                                                          updateHw);
        }
        if (retVal != GT_OK)
        {
            if (ucSupportArr[protStk] == GT_TRUE)
            {
                cpssOsLpmFree(defUcNextHopEntry);
            }
            if ((protStk <= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) && (mcSupportArr[protStk] == GT_TRUE))
            {
                if ((ucSupportArr[protStk] == GT_FALSE) ||
                    (defUcNextHopPointer.routeEntryBaseMemAddr != defMcNextHopPointer.routeEntryBaseMemAddr) ||
                    (defUcNextHopPointer.routeEntryMethod != defMcNextHopPointer.routeEntryMethod))
                {
                    cpssOsLpmFree(defMcNextHopEntry);
                }
            }
            return retVal;
        }

        /* Don't touch the hw in a HSU process */
        if(updateHw == GT_TRUE)
        {
            /* write the mc and uc vr router table */
            retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId, protStk, shadowPtr);

            if (retVal != GT_OK)
            {
                if (ucSupportArr[protStk] == GT_TRUE)
                {
                    cpssOsLpmFree(defUcNextHopEntry);
                }
                if (mcSupportArr[protStk] == GT_TRUE)
                {
                    if ((ucSupportArr[protStk] == GT_FALSE) ||
                        (defUcNextHopPointer.routeEntryBaseMemAddr != defMcNextHopPointer.routeEntryBaseMemAddr) ||
                        (defUcNextHopPointer.routeEntryMethod != defMcNextHopPointer.routeEntryMethod))
                    {
                        cpssOsLpmFree(defMcNextHopEntry);
                    }
                }
                return retVal;
            }
        }
    }

    shadowPtr->vrRootBucketArray[vrId].valid = GT_TRUE;

    /* the data was written successfully to HW, we can reset the information regarding the new memoryPool allocations done.
    next call will set this array with new values of allocated/bound blocks */
    cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));

    cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

    return GT_OK;
}

/**
* @internal lpmVirtualRouterRemove function
* @endinternal
*
* @brief   This function removes a virtual router in system for specific shadow.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] vrId                     - The virtual's router ID.
* @param[in] shadowPtr                - the shadow to work on.
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_BAD_STATE             - if the existing VR is not empty.
*
* @note The VR must be empty from prefixes in order to be Removed!
*
*/
static GT_STATUS lpmVirtualRouterRemove
(
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr
)
{
    GT_STATUS                                 retVal = GT_OK;

    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    /* this is an existing Vr - delete the allocated structures */
    retVal = removeSupportedProtocolsFromVirtualRouter(vrId,shadowPtr);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    shadowPtr->vrRootBucketArray[vrId].valid = GT_FALSE;

    return retVal;
}



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
)
{
    GT_STATUS                              retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC       *shadowPtr;
    GT_U32                                 shadowIdx;

    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP         protocolBitmap = 0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP         protocolDifference;
    GT_BOOL                                vrUcSupport[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS];
    GT_BOOL                                vrMcSupport[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS];


    vrUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = (vrConfigPtr->supportUcIpv4) ? GT_TRUE : GT_FALSE;
    vrUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = (vrConfigPtr->supportUcIpv6) ? GT_TRUE : GT_FALSE;
    vrUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] = (vrConfigPtr->supportFcoe)   ? GT_TRUE : GT_FALSE;
    vrMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = (vrConfigPtr->supportMcIpv4) ? GT_TRUE : GT_FALSE;
    vrMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = (vrConfigPtr->supportMcIpv6) ? GT_TRUE : GT_FALSE;

    /* determine the supported protocols */
    if ((vrConfigPtr->supportUcIpv4 == GT_TRUE) || (vrConfigPtr->supportMcIpv4 == GT_TRUE))
    {
        protocolBitmap |= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_BIT_E;
    }
    if ((vrConfigPtr->supportUcIpv6 == GT_TRUE) || (vrConfigPtr->supportMcIpv6 == GT_TRUE))
    {
        protocolBitmap |= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_BIT_E;
    }
    if (vrConfigPtr->supportFcoe == GT_TRUE)
    {
        protocolBitmap |= PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_BIT_E;
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    protocolDifference = protocolBitmap ^ lpmDbPtr->protocolBitmap;
    if (protocolDifference)
    {
        if (protocolDifference & protocolBitmap)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
        }
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (vrConfigPtr->supportUcIpv4 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,
                                                          &vrConfigPtr->defaultUcIpv4RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportMcIpv4 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,
                                                          &vrConfigPtr->defaultMcIpv4RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportUcIpv6 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,
                                                          &vrConfigPtr->defaultUcIpv6RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportMcIpv6 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,
                                                          &vrConfigPtr->defaultMcIpv6RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportFcoe == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,
                                                          &vrConfigPtr->defaultFcoeForwardingEntry);
            if (retVal != GT_OK)
                return retVal;
        }


        if (vrId >= shadowPtr->vrfTblSize)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        retVal = lpmVirtualRouterSet(vrId,
                                     vrConfigPtr->defaultUcIpv4RouteEntry,
                                     vrConfigPtr->defaultUcIpv6RouteEntry,
                                     vrConfigPtr->defaultFcoeForwardingEntry,
                                     vrConfigPtr->defaultMcIpv4RouteEntry,
                                     vrConfigPtr->defaultMcIpv6RouteEntry,
                                     protocolBitmap,
                                     vrUcSupport,
                                     vrMcSupport,
                                     shadowPtr,
                                     GT_TRUE);
        if (retVal != GT_OK)
        {
            if (retVal != GT_ALREADY_EXIST)
            {
                /* free all allocated resources  */
                removeSupportedProtocolsFromVirtualRouter(vrId,shadowPtr);
            }
            return(retVal);
        }
    }

    return retVal;
}

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
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;
    GT_U32                              shadowIdx;

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* delete all the IPv4 unicast prefixes for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E])
        {
                retVal = prvCpssDxChLpmRamIpv4UcPrefixesFlush(lpmDbPtr,vrId);
                if (retVal != GT_OK)
                {
                    return (retVal);
                }
        }

        /* delete all the IPv4 multicast entries for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E])
        {
            retVal = prvCpssDxChLpmRamIpv4McEntriesFlush(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        /* delete all the IPv6 unicast prefixes for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E])
        {
            retVal = prvCpssDxChLpmRamIpv6UcPrefixesFlush(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        /* delete all the IPv6 multicast entries for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E])
        {
            retVal = prvCpssDxChLpmRamIpv6McEntriesFlush(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        /* delete all the FCoE prefixes for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E])
        {
            retVal = prvCpssDxChLpmRamFcoePrefixesFlush(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        retVal = lpmVirtualRouterRemove(vrId, shadowPtr);
        if (retVal != GT_OK)
        {
            return (retVal);
        }
    }

    return retVal;
}

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
)
{
    GT_STATUS                                   retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *defRoutePointerPtr = NULL;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   routeEntry;
    GT_IPV6ADDR                                 ipv6Addr;
    GT_IPADDR                                   ipv4Addr;
    GT_FCID                                     fcoeAddr;

    shadowPtr = &lpmDbPtr->shadowArray[0];

    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* IPv4 MC */
    if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] == GT_TRUE)
    {
        retVal = prvCpssDxChLpmRamMcDefRouteGet(vrId ,&defRoutePointerPtr,
                                                shadowPtr ,PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((defRoutePointerPtr == NULL) || (retVal == GT_NOT_FOUND) || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportMcIpv4 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultMcIpv4RouteEntry = *defRoutePointerPtr;
            vrConfigPtr->supportMcIpv4 = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportMcIpv4 = GT_FALSE;
    }

    /* IPv4 UC */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] == GT_TRUE)
    {
        cpssOsMemSet(&ipv4Addr, 0, sizeof(ipv4Addr));
        retVal = prvCpssDxChLpmRamIpv4UcPrefixSearch(lpmDbPtr,vrId,ipv4Addr,0,&routeEntry);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((retVal == GT_NOT_FOUND)  || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportUcIpv4 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultUcIpv4RouteEntry = routeEntry;
            vrConfigPtr->supportUcIpv4 = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportUcIpv4 = GT_FALSE;
    }

    /* IPv6 MC */
    if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] == GT_TRUE)
    {
        retVal = prvCpssDxChLpmRamMcDefRouteGet(vrId, &defRoutePointerPtr,
                                                shadowPtr, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((defRoutePointerPtr == NULL) || (retVal == GT_NOT_FOUND) || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportMcIpv6 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultMcIpv6RouteEntry = *defRoutePointerPtr;
            vrConfigPtr->supportMcIpv6 = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportMcIpv6 = GT_FALSE;
    }

    /* IPv6 UC */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] == GT_TRUE)
    {
        cpssOsMemSet(&ipv6Addr, 0, sizeof(ipv6Addr));
        retVal = prvCpssDxChLpmRamIpv6UcPrefixSearch(lpmDbPtr,vrId,ipv6Addr,0,&routeEntry);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((retVal == GT_NOT_FOUND)  || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportUcIpv6 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultUcIpv6RouteEntry = routeEntry;
            vrConfigPtr->supportUcIpv6 = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportUcIpv6 = GT_FALSE;
    }

    /* FCoE */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] == GT_TRUE)
    {
        cpssOsMemSet(&fcoeAddr, 0, sizeof(fcoeAddr));
        retVal = prvCpssDxChLpmRamFcoePrefixSearch(lpmDbPtr,vrId,fcoeAddr,0,&routeEntry);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((retVal == GT_NOT_FOUND)  || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportFcoe = GT_FALSE;
        else
        {
            vrConfigPtr->defaultFcoeForwardingEntry = routeEntry;
            vrConfigPtr->supportFcoe = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportFcoe = GT_FALSE;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;


    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,nextHopInfoPtr);
        if (retVal != GT_OK)
            return retVal;

        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamUcEntryAdd(vrId,
                                                     ipAddr.arIP,
                                                     prefixLen,
                                                     nextHopInfoPtr,
                                                     PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                     override,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                     shadowPtr);

        }
        else
        {
            retVal = prvCpssDxChLpmRamUcEntryAdd(vrId,
                                                 ipAddr.arIP,
                                                 prefixLen,
                                                 nextHopInfoPtr,
                                                 PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                 override,
                                                  PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr,
                                                 defragmentationEnable);
        }
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

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
)
{
    GT_U32 blockIndex=0;

    for (blockIndex = 0 ; blockIndex < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS; blockIndex++)
    {   /* we only keep the values were update=GT_TRUE, since this is the sum of all updates done */
        if(pendingBlockToUpdateArr[blockIndex].updateInc==GT_TRUE)
        {
            totalPendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
            totalPendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                                        pendingBlockToUpdateArr[blockIndex].numOfIncUpdates;
          /* reset pending array for next element */
            pendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
            pendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;
        }
        if(pendingBlockToUpdateArr[blockIndex].updateDec==GT_TRUE)
        {
            totalPendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            totalPendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                            pendingBlockToUpdateArr[blockIndex].numOfDecUpdates;
            /* reset pending array for next element */
            pendingBlockToUpdateArr[blockIndex].updateDec=GT_FALSE;
            pendingBlockToUpdateArr[blockIndex].numOfDecUpdates=0;
        }
    }
    return GT_OK;
}

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
* @param[in] numOfLpmMemories         -  number of LPM memories
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
)
{
    GT_U32 blockIndex=0; /* calculated according to the memory offset devided by block size including gap */
    GT_U32 numOfIncUpdatesToCounter=0;/* counter to be used for incrementing "protocol to block" usage*/
    GT_U32 numOfDecUpdatesToCounter=0;/* counter to be used for decrementing "protocol to block" usage*/
    GT_U32 numOfLinesInBlock;

    /* update the block usage counters */
    for (blockIndex = 0; blockIndex < numOfLpmMemories; blockIndex++)
    {
        numOfLinesInBlock  = lpmRamBlocksSizeArrayPtr[blockIndex];

        if((totalPendingBlockToUpdateArr[blockIndex].updateInc==GT_TRUE)||
           (totalPendingBlockToUpdateArr[blockIndex].updateDec==GT_TRUE))
        {
            numOfIncUpdatesToCounter = totalPendingBlockToUpdateArr[blockIndex].numOfIncUpdates;
            numOfDecUpdatesToCounter = totalPendingBlockToUpdateArr[blockIndex].numOfDecUpdates;
            switch (protocol)
            {
                case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
                if (((protocolCountersPerBlockArr[blockIndex].sumOfIpv4Counters +
                         numOfIncUpdatesToCounter)<numOfDecUpdatesToCounter) ||
                    (((protocolCountersPerBlockArr[blockIndex].sumOfIpv4Counters +
                         numOfIncUpdatesToCounter)-numOfDecUpdatesToCounter)>numOfLinesInBlock))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                protocolCountersPerBlockArr[blockIndex].sumOfIpv4Counters +=
                        numOfIncUpdatesToCounter - numOfDecUpdatesToCounter ;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
                if (((protocolCountersPerBlockArr[blockIndex].sumOfIpv6Counters +
                         numOfIncUpdatesToCounter)<numOfDecUpdatesToCounter) ||
                    (((protocolCountersPerBlockArr[blockIndex].sumOfIpv6Counters +
                         numOfIncUpdatesToCounter)-numOfDecUpdatesToCounter)>numOfLinesInBlock))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                protocolCountersPerBlockArr[blockIndex].sumOfIpv6Counters +=
                        numOfIncUpdatesToCounter - numOfDecUpdatesToCounter ;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
                if (((protocolCountersPerBlockArr[blockIndex].sumOfFcoeCounters +
                         numOfIncUpdatesToCounter)<numOfDecUpdatesToCounter) ||
                    (((protocolCountersPerBlockArr[blockIndex].sumOfFcoeCounters +
                         numOfIncUpdatesToCounter)-numOfDecUpdatesToCounter)>numOfLinesInBlock))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                protocolCountersPerBlockArr[blockIndex].sumOfFcoeCounters +=
                        numOfIncUpdatesToCounter - numOfDecUpdatesToCounter ;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

           /* reset pending for future use */
           resetPendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
           resetPendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;
           resetPendingBlockToUpdateArr[blockIndex].updateDec=GT_FALSE;
           resetPendingBlockToUpdateArr[blockIndex].numOfDecUpdates=0;
        }
    }
    return GT_OK;
}

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
)
{
    GT_STATUS           retVal = GT_OK,retVal1=GT_OK,retVal2=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));
    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    for (i = 0 ; i < ipv4PrefixArrayLen; i++)
    {
        if (ipv4PrefixArrayPtr[i].prefixLen > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if ((ipv4PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS) &&
            (ipv4PrefixArrayPtr[i].ipAddr.arIP[0] <= PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS))
        {
            /* Multicast range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));


        /* first go over the prefixes and update the shadow */
        for (i = 0 ; i < ipv4PrefixArrayLen ; i++)
        {
            prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(shadowPtr->shadowType,
                                                             &(ipv4PrefixArrayPtr[i].nextHopInfo),
                                                             &lpmRouteEntry);
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,&(lpmRouteEntry.routeEntry));
            if (retVal != GT_OK)
            {
                return retVal;
            }

            ipv4PrefixArrayPtr[i].returnStatus =
                prvCpssDxChLpmRamUcEntryAdd(ipv4PrefixArrayPtr[i].vrId,
                                            ipv4PrefixArrayPtr[i].ipAddr.arIP,
                                            ipv4PrefixArrayPtr[i].prefixLen,
                                            &lpmRouteEntry.routeEntry,
                                            PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                            ipv4PrefixArrayPtr[i].override,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                            shadowPtr,
                                            defragmentationEnable);

            if (ipv4PrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                retVal2 = GT_FAIL;
            }
            else
            {
                retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    return retVal1;
                }
            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E, &protocolBitmap);
#if 0
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E)
        {
            /* for sip5 the assumption is that if ipv4Uc is supported the FCoE is supported */
            PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_SET_MAC(protocolBitmap);
        }
#endif
        /* now update the HW */
        retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            /* reset pending array for next use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            return (retVal1);
        }
        else
        {
            retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
            if (retVal1!=GT_OK)
            {
                return retVal1;
            }

            retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                tempPendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal1!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                return retVal1;
            }
        }
    }
    return (retVal2);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmRamSip6UcEntryDel(vrId,
                                                     ipAddr.arIP,
                                                     prefixLen,
                                                     GT_TRUE,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                     shadowPtr,
                                                     GT_FALSE);
        }
        else
        {
            retVal = prvCpssDxChLpmRamUcEntryDel(vrId,
                                                 ipAddr.arIP,
                                                 prefixLen,
                                                 GT_TRUE,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr,
                                                 GT_FALSE);
        }

        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

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
)
{
    GT_STATUS           retVal = GT_OK,retVal1;
    GT_U32      i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,
                     sizeof(shadowPtr->pendingBlockToUpdateArr));

        for (i = 0 ; i < ipv4PrefixArrayLen ; i++)
        {
            if (ipv4PrefixArrayPtr[i].prefixLen > 32)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            if ((ipv4PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS) &&
                (ipv4PrefixArrayPtr[i].ipAddr.arIP[0] <= PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS))
            {
                /* Multicast range */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            ipv4PrefixArrayPtr[i].returnStatus =
                prvCpssDxChLpmRamUcEntryDel(ipv4PrefixArrayPtr[i].vrId,
                                            ipv4PrefixArrayPtr[i].ipAddr.arIP,
                                            ipv4PrefixArrayPtr[i].prefixLen,
                                            GT_FALSE,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                            shadowPtr,
                                            GT_FALSE);

            if (ipv4PrefixArrayPtr[i].returnStatus != GT_OK)
            {
               /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                retVal = GT_FAIL;
            }
            else
            {
                retVal1=prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    return retVal1;
                }
            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E, &protocolBitmap);

        /* now update the HW */
        retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            /* reset pending array for next use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            return (retVal1);
        }
        else
        {
            retVal1=prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
            if (retVal1!=GT_OK)
            {
                return retVal1;
            }
            retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                tempPendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal1!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                return retVal1;
            }
        }
    }
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamUcEntriesFlush(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntrySearch(vrId,ipAddr.arIP,prefixLen,
                                            nextHopInfoPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                            shadowPtr);

    return (retVal);
}

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
)
{
    GT_STATUS                                   retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *tmpNextHopInfoPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryGet(vrId,ipAddrPtr->arIP,prefixLenPtr,
                                         &tmpNextHopInfoPtr,
                                         PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                         shadowPtr);
    if (retVal == GT_OK)
    {
        cpssOsMemCpy(nextHopInfoPtr,tmpNextHopInfoPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    }

    return retVal;
}

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
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
       return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryLpmSearch(vrId,ipAddr.arIP,prefixLenPtr,
                                               nextHopPointerPtr,
                                               PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                               shadowPtr);
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    if ((ipGroupPrefixLen > 4) && (ipGroupPrefixLen < 32))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,mcRouteEntryPtr);
        if (retVal != GT_OK)
            return retVal;

        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamMcEntryAdd(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                     ipSrc.arIP,ipSrcPrefixLen,
                                                     mcRouteEntryPtr,
                                                     override,
                                                     PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                     shadowPtr,
                                                     defragmentationEnable);

        }
        else
        {
            retVal = prvCpssDxChLpmRamMcEntryAdd(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                 ipSrc.arIP,ipSrcPrefixLen,
                                                 mcRouteEntryPtr,
                                                 override,
                                                 PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr,
                                                 defragmentationEnable);
        }
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E) 
        {
            retVal = prvCpssDxChLpmSip6RamMcEntryDelete(vrId, ipGroup.arIP,ipGroupPrefixLen,
                                                        ipSrc.arIP,ipSrcPrefixLen,
                                                        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                        shadowPtr,GT_FALSE,NULL);
        }
        else
        {
            retVal = prvCpssDxChLpmRamMcEntryDelete(vrId, ipGroup.arIP,ipGroupPrefixLen,
                                                    ipSrc.arIP,ipSrcPrefixLen,
                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                    shadowPtr,GT_FALSE,NULL);
        }
        if (retVal != GT_OK)
        {
            break;
        }

    }
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamMcEntriesFlush(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntrySearch(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                            ipSrc.arIP,ipSrcPrefixLen,
                                            mcRouteEntryPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                            shadowPtr);
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntryGetNext(vrId,ipGroupPtr->arIP,ipGroupPrefixLenPtr,
                                             ipSrcPtr->arIP,
                                             ipSrcPrefixLenPtr,
                                             mcRouteEntryPtr,
                                             PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                             shadowPtr);
    /* group prefix 0 means this is the default entry (*,*) */
    if (*ipGroupPrefixLenPtr == 0)
    {
        ipGroupPtr->u32Ip = 0;
    }
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;


    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,nextHopInfoPtr);
        if (retVal != GT_OK)
            return retVal;

        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamUcEntryAdd(vrId,
                                                     ipAddr.arIP,
                                                     prefixLen,
                                                     nextHopInfoPtr,
                                                     PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                     override,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                     shadowPtr);
        }
        else
        {
            retVal = prvCpssDxChLpmRamUcEntryAdd(vrId,
                                             ipAddr.arIP,
                                             prefixLen,
                                             nextHopInfoPtr,
                                             PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                             override,
                                             PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                             shadowPtr,
                                             defragmentationEnable);
        }

        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

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
)
{
    GT_STATUS           retVal = GT_OK,retVal1=GT_OK,retVal2=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));

    for (i = 0 ; i < ipv6PrefixArrayLen ; i++)
    {
        if (ipv6PrefixArrayPtr[i].prefixLen > 128)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if (ipv6PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS)
        {
            /* Multicast range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,
                     sizeof(shadowPtr->pendingBlockToUpdateArr));

        /* first go over the prefixes and update the shadow */
        for (i = 0 ; i < ipv6PrefixArrayLen ; i++)
        {
            prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(shadowPtr->shadowType,
                                                             &(ipv6PrefixArrayPtr[i].nextHopInfo),
                                                             &lpmRouteEntry);
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,&(lpmRouteEntry.routeEntry));
            if (retVal != GT_OK)
            {
                return retVal;
            }
            ipv6PrefixArrayPtr[i].returnStatus =
                prvCpssDxChLpmRamUcEntryAdd(ipv6PrefixArrayPtr[i].vrId,
                                            ipv6PrefixArrayPtr[i].ipAddr.arIP,
                                            ipv6PrefixArrayPtr[i].prefixLen,
                                            &lpmRouteEntry.routeEntry,
                                            PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                            ipv6PrefixArrayPtr[i].override,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                            shadowPtr,
                                            defragmentationEnable);

            if (ipv6PrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                retVal2 = GT_FAIL;
            }
            else
            {
                retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,
                                                                                            shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    return retVal1;
                }

            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E, &protocolBitmap);
        /* now update the HW */
        retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            /* reset pending array for next use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            return (retVal1);
        }
        else
        {
            retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,
                                                                                        shadowPtr->pendingBlockToUpdateArr);
            if (retVal1!=GT_OK)
            {
                return retVal1;
            }
            retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                tempPendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal1!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                return retVal1;
            }
        }
    }
    return (retVal2);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmRamSip6UcEntryDel(vrId,
                                                     ipAddr.arIP,
                                                     prefixLen,
                                                     GT_TRUE,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                     shadowPtr,
                                                     GT_FALSE);
        }
        else
        {
            retVal = prvCpssDxChLpmRamUcEntryDel(vrId,
                                                 ipAddr.arIP,
                                                 prefixLen,
                                                 GT_TRUE,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr,
                                                 GT_FALSE);
        }
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

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
)
{
    GT_STATUS           retVal = GT_OK,retVal1;
    GT_U32      i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    if(ipv6PrefixArrayLen == 0)
        return GT_OK;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,
                     sizeof(tempPendingBlockToUpdateArr));

        for (i = 0 ; i < ipv6PrefixArrayLen ; i++)
        {
            if (ipv6PrefixArrayPtr[i].prefixLen > 128)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            if (ipv6PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS)
            {
                /* Multicast range */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            ipv6PrefixArrayPtr[i].returnStatus =
                prvCpssDxChLpmRamUcEntryDel(ipv6PrefixArrayPtr[i].vrId,
                                            ipv6PrefixArrayPtr[i].ipAddr.arIP,
                                            ipv6PrefixArrayPtr[i].prefixLen,
                                            GT_FALSE,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                            shadowPtr,
                                            GT_FALSE);

            if (ipv6PrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                retVal = GT_FAIL;
            }
            else
            {
                retVal1=prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,
                                                                                          shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    return retVal1;
                }
            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E, &protocolBitmap);
        /* now update the HW */
        retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            return (retVal1);
        }
        else
        {
            retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,
                                                                                        shadowPtr->pendingBlockToUpdateArr);
            if (retVal1!=GT_OK)
            {
                return retVal1;
            }

            retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                tempPendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal1!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                return retVal1;
            }
        }
    }
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamUcEntriesFlush(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamUcEntrySearch(vrId,ipAddr.arIP,prefixLen,
                                            nextHopInfoPtr,
                                          PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                            shadowPtr);
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *tmpNextHopInfoPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryGet(vrId,ipAddrPtr->arIP,prefixLenPtr,
                                         &tmpNextHopInfoPtr,
                                         PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                         shadowPtr);
    if (retVal == GT_OK)
    {
        cpssOsMemCpy(nextHopInfoPtr,tmpNextHopInfoPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    }

    return retVal;
}

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
)
{
    GT_STATUS                       retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC   *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
       return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamUcEntryLpmSearch(vrId,ipAddr.arIP,prefixLenPtr,
                                               nextHopInfoPtr,
                                               PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                               shadowPtr);
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    if ((ipGroupPrefixLen > 8) && (ipGroupPrefixLen < 128))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,mcRouteEntryPtr);
        if (retVal != GT_OK)
            return retVal;
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamMcEntryAdd(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                     ipSrc.arIP,ipSrcPrefixLen,
                                                     mcRouteEntryPtr,
                                                     override,
                                                     PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                     shadowPtr,
                                                     defragmentationEnable);

        }
        else
        {
            retVal = prvCpssDxChLpmRamMcEntryAdd(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                 ipSrc.arIP,ipSrcPrefixLen,
                                                 mcRouteEntryPtr,
                                                 override,
                                                 PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr,
                                                 defragmentationEnable);

        }
    }
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E) 
        {
            retVal = prvCpssDxChLpmSip6RamMcEntryDelete(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                        ipSrc.arIP,ipSrcPrefixLen,
                                                        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                        shadowPtr,GT_FALSE,NULL);
        }
        else
        {
            retVal = prvCpssDxChLpmRamMcEntryDelete(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                    ipSrc.arIP,ipSrcPrefixLen,
                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                    shadowPtr,GT_FALSE,NULL);
        }

        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        retVal = prvCpssDxChLpmRamMcEntriesFlush(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

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
)
{
    GT_STATUS                               retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntrySearch(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                            ipSrc.arIP,ipSrcPrefixLen,
                                            mcRouteEntryPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                            shadowPtr);
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntryGetNext(vrId,ipGroupPtr->arIP,ipGroupPrefixLenPtr,
                                             ipSrcPtr->arIP,
                                             ipSrcPrefixLenPtr,
                                             mcRouteEntryPtr,
                                             PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                             shadowPtr);
    /* group prefix 0 means this is the default entry (*,*) */
    if (*ipGroupPrefixLenPtr == 0)
    {
        cpssOsMemSet(ipGroupPtr->u32Ip,0,sizeof(GT_U32)*4);
    }
    return (retVal);
}


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
* @param[in] fcoeAddr                 - The FC_ID of this prefix.
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
    IN  GT_FCID                                     fcoeAddr,
    IN  GT_U32                                      prefixLen,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr,
    IN  GT_BOOL                                     override,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;
    GT_U32                              shadowIdx;


    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,nextHopInfoPtr);
        if (retVal != GT_OK)
            return retVal;

        retVal = prvCpssDxChLpmRamUcEntryAdd(vrId,
                                             fcoeAddr.fcid,
                                             prefixLen,
                                             nextHopInfoPtr,
                                             PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                             override,
                                             PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                             shadowPtr,
                                             defragmentationEnable);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

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
* @param[in] fcoeAddr                 - The destination FC_ID address of the prefix.
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
    IN  GT_FCID                                 fcoeAddr,
    IN  GT_U32                                  prefixLen
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        retVal = prvCpssDxChLpmRamUcEntryDel(vrId,
                                             fcoeAddr.fcid,
                                             prefixLen,
                                             GT_TRUE,
                                             PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                             shadowPtr,
                                             GT_FALSE);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

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
* @param[in] fcoeAddrPrefixArrayLen   - Length of FC_ID prefix array.
* @param[in] fcoeAddrPrefixArrayPtr   - The FC_ID prefix array.
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
    IN  GT_U32                                          fcoeAddrPrefixArrayLen,
    IN  CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    *fcoeAddrPrefixArrayPtr,
    IN  GT_BOOL                                         defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK,retVal1=GT_OK,retVal2=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));

    for (i = 0 ; i < fcoeAddrPrefixArrayLen; i++)
    {
        if (fcoeAddrPrefixArrayPtr[i].prefixLen > 24)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        /* first go over the prefixes and update the shadow */
        for (i = 0 ; i < fcoeAddrPrefixArrayLen ; i++)
        {
            prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(shadowPtr->shadowType,
                                                             &(fcoeAddrPrefixArrayPtr[i].nextHopInfo),
                                                             &lpmRouteEntry);
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,&(lpmRouteEntry.routeEntry));
            if (retVal != GT_OK)
            {
                return retVal;
            }
            fcoeAddrPrefixArrayPtr[i].returnStatus =
                prvCpssDxChLpmRamUcEntryAdd(fcoeAddrPrefixArrayPtr[i].vrId,
                                            fcoeAddrPrefixArrayPtr[i].fcoeAddr.fcid,
                                            fcoeAddrPrefixArrayPtr[i].prefixLen,
                                            &lpmRouteEntry.routeEntry,
                                            PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                            fcoeAddrPrefixArrayPtr[i].override,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                            shadowPtr,
                                            defragmentationEnable);

            if (fcoeAddrPrefixArrayPtr[i].returnStatus != GT_OK)
            {
                retVal2 = GT_FAIL;
            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(CPSS_IP_PROTOCOL_FCOE_E, &protocolBitmap);
        /* now update the HW */
        retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            return (retVal1);
        }
    }
    return (retVal2);
}

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
* @param[in] fcoeAddrPrefixArrayLen   - Length of FC_ID prefix array.
* @param[in] fcoeAddrPrefixArrayPtr   - The FC_ID prefix array.
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
    IN  GT_U32                                          fcoeAddrPrefixArrayLen,
    IN  CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    *fcoeAddrPrefixArrayPtr
)
{
    GT_STATUS           retVal = GT_OK,retVal1;
    GT_U32      i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        for (i = 0 ; i < fcoeAddrPrefixArrayLen ; i++)
        {
            if (fcoeAddrPrefixArrayPtr[i].prefixLen > 24)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            fcoeAddrPrefixArrayPtr[i].returnStatus =
                prvCpssDxChLpmRamUcEntryDel(fcoeAddrPrefixArrayPtr[i].vrId,
                                            fcoeAddrPrefixArrayPtr[i].fcoeAddr.fcid,
                                            fcoeAddrPrefixArrayPtr[i].prefixLen,
                                            GT_FALSE,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                            shadowPtr,
                                            GT_FALSE);

            if (fcoeAddrPrefixArrayPtr[i].returnStatus != GT_OK)
            {
                retVal = GT_FAIL;
            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E, &protocolBitmap);
        /* now update the HW */
        retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            return (retVal1);
        }
    }
    return (retVal);
}

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
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamUcEntriesFlush(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

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
* @param[in] fcoeAddr                 - The FC_ID to look for.
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
    IN  GT_FCID                                     fcoeAddr,
    IN  GT_U32                                      prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntrySearch(vrId,fcoeAddr.fcid,prefixLen,
                                            nextHopInfoPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                            shadowPtr);

    return (retVal);
}

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
* @param[in,out] fcoeAddrPtr              - The FC_ID to start the search from.
* @param[in,out] prefixLenPtr             - The number of bits that are actual valid in the
*                                      FC_ID.
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
GT_STATUS prvCpssDxChLpmRamFcoePrefixGetNext
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_FCID                                   *fcoeAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopInfoPtr
)
{
    GT_STATUS                                   retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *tmpNextHopInfoPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryGet(vrId,fcoeAddrPtr->fcid,prefixLenPtr,
                                         &tmpNextHopInfoPtr,
                                         PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                         shadowPtr);
    if (retVal == GT_OK)
    {
        cpssOsMemCpy(nextHopInfoPtr,tmpNextHopInfoPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    }

    return retVal;
}

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
* @param[in] fcoeAddr                 - The FC_ID to look for.
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
    IN  GT_FCID                                     fcoeAddr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
       return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryLpmSearch(vrId,fcoeAddr.fcid,prefixLenPtr,
                                               nextHopInfoPtr,
                                               PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                               shadowPtr);
    return (retVal);
}

#if 0
/**
* @internal ipLpmDbGetL3Vr function
* @endinternal
*
* @brief   Retrives the valid virtual routers
*
* @param[in] ipShadowPtr              - points to the shadow to retrieve from
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - the table size info block
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3Vr
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *ipShadowPtr,
    OUT    GT_U32                               *tableSizePtr,
    OUT    GT_VOID                              **tablePtrPtr,
    INOUT  GT_UINTPTR                           *iterPtr,
    INOUT  GT_U32                               *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    GT_U8 ipAddr[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32 *vrId;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC currNh;
    PRV_CPSS_EXMXPM_IPV6MC_PCL_ENTRY_STC ipv6McPclEntry,*ipv6McPclEntryPtr;

    PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC *vrEntry;
    GT_U32 numOfEntriesLeft = 0;

    GT_U32 numOfEntries = 0;

    if (*iterPtr == 0)
    {
        /* this is the first call , so intilize */
        *iterPtr = (GT_UINTPTR)cpssOsLpmMalloc(sizeof (GT_U32));
        if (*iterPtr == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        vrId = (GT_U32*)(*iterPtr);
        *vrId = 0;
    }
    else
    {
        vrId = (GT_U32*)(*iterPtr);

    }

    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* set the pointer */
    vrEntry = (PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC*)(*tablePtrPtr);
    for(;(numOfEntries < numOfEntriesLeft) &&
          (*vrId < ipShadowPtr->vrfTblSize); (*vrId)++)
    {

        if (ipShadowPtr->vrRootBucketArray[*vrId].valid == GT_FALSE)
        {
            continue;
        }

        if (tableSizePtr != NULL)
        {
            *tableSizePtr += sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC);
        }
        if (*tablePtrPtr != NULL)
        {
            vrEntry->vrId = *vrId;
            vrEntry->isLast = GT_FALSE;

            /* check the protocol stack */
            if ((ipShadowPtr->vrRootBucketArray[*vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != NULL) &&
                (ipShadowPtr->vrRootBucketArray[*vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] != NULL))
            {
                vrEntry->protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
            }
            else if (ipShadowPtr->vrRootBucketArray[*vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != NULL)
            {
                vrEntry->protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
            }
            else
            {
                vrEntry->protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
            }

            /* check IPv4 uc */
            if (ipShadowPtr->vrRootBucketArray[*vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != NULL)
            {
                retVal = prvCpssDxChLpmRamUcEntrySearch(*vrId,ipAddr,0,&currNh,
                                                        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                        ipShadowPtr);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                cpssOsMemCpy(&vrEntry->defUcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E],
                             &currNh,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                vrEntry->vrIpUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = GT_TRUE;
            }
            else
            {
                 vrEntry->vrIpUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = GT_FALSE;
            }
            /* check ipv4 mc */
            retVal = prvCpssDxChLpmRamMcEntrySearch(*vrId,ipAddr,0,NULL
                                                    ipAddr,0,&currNh,
                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                    ipShadowPtr);
            if ((retVal != GT_OK) && (retVal !=GT_NOT_FOUND))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            if (retVal == GT_NOT_FOUND)
            {
                /* ipv4 mc is disable */
                vrEntry->vrIpMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = GT_FALSE;
                 cpssOsMemSet(&vrEntry->defMcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E],
                         0,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            }
            else
            {
                vrEntry->vrIpMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = GT_TRUE;
                 cpssOsMemCpy(&vrEntry->defMcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E],
                         &currNh,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            }

            /* check IPv6 uc */
            if (ipShadowPtr->vrRootBucketArray[*vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] != NULL)
            {
                retVal = prvCpssDxChLpmRamUcEntrySearch(*vrId,ipAddr,0,&currNh,
                                                        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                        ipShadowPtr);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                cpssOsMemCpy(&vrEntry->defUcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E],
                             &currNh,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                vrEntry->vrIpUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = GT_TRUE;
            }
            else
            {
                vrEntry->vrIpUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = GT_FALSE;
            }
            /* check ipv6 mc */

            ipv6McPclEntry.ipGroupPrefix = 0;
            ipv6McPclEntryPtr = prvCpssSlSearch(ipShadowPtr->vrRootBucketArray[*vrId].coreIPv6McGroup2PceIdDb,
                              &ipv6McPclEntry);

            if (ipv6McPclEntryPtr == NULL)
            {
                vrEntry->vrIpMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = GT_FALSE;
                cpssOsMemSet(&vrEntry->defMcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E],
                             0,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            }
            else
            {
                vrEntry->vrIpMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = GT_TRUE;
                cpssOsMemCpy(&vrEntry->defMcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E],
                             ipv6McPclEntryPtr->lpmBucketPtr->rangeList->lowerLpmPtr.pNextHopEntry,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

                vrEntry->defIpv6McRuleIndex = ipv6McPclEntryPtr->pceId;

                vrEntry->vrIpv6McPclId =
                    ipShadowPtr->vrRootBucketArray[*vrId].vrIpv6McPclId;
            }
            /* advance the pointer */
            vrEntry++;
        }
        numOfEntries++;
    }

    /* check if we finished iterating*/
    if ( (*vrId == ipShadowPtr->vrfTblSize) && (numOfEntriesLeft > numOfEntries))
    {
        if (*tablePtrPtr != NULL)
        {
            vrEntry->isLast = GT_TRUE;
            *tablePtrPtr = (GT_VOID *)(vrEntry + 1);
        }
        else
        {
            /* take in account "last" virtual router entry: */
            *tableSizePtr += sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC);
        }
        numOfEntries++;
        /* free & zero the iterator */
        cpssOsLpmFree((GT_PTR)*iterPtr);
        *iterPtr = 0;

        retVal = GT_OK;
    }
    else
    {
        /* we didn't finish record the ptr */
        *tablePtrPtr = (GT_VOID *)vrEntry;
    }

    /* update the left num of entries */
    numOfEntriesLeft -= numOfEntries;


    *dataSizePtr = *dataSizePtr - numOfEntries *(sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC));

    return (retVal);
}

/**
* @internal ipLpmDbGetL3UcPrefix function
* @endinternal
*
* @brief   Retrives a specifc device's core ip uc prefix memory Size needed and info
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3UcPrefix
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT    GT_U32                           *tableSizePtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    GT_U8 ipAddr[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32 ipPrefix;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC currNh;
    GT_U32 numOfEntries = 0;
    GT_U32 numOfEntriesLeft = 0;
    PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC *ipv6Prefix;
    PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC *ipv4Prefix;
    GT_PTR  dummy;

    cpssOsMemSet(&currNh,0,sizeof(currNh));

    if (*iterPtr == 0)
    {
        /* this is the first call , so intilize */
        *iterPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E) ?
            (GT_UINTPTR)cpssOsLpmMalloc(sizeof (PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC)):
            (GT_UINTPTR)cpssOsLpmMalloc(sizeof (PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC));
        ipPrefix = 0;
    }
    else
    {
        /* get the current */
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            ipv4Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC*)(*iterPtr);
            cpssOsMemCpy(ipAddr,ipv4Prefix->ipAddr.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
            ipPrefix = ipv4Prefix->prefixLen;

        }
        else
        {
            ipv6Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC*)(*iterPtr);
            cpssOsMemCpy(ipAddr,ipv6Prefix->ipAddr.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
            ipPrefix = ipv6Prefix->prefixLen ;
        }
    }


     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC));
     else
         numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* set the pointer */
    ipv4Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC*)(*tablePtrPtr);
    ipv6Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC*)(*tablePtrPtr);

    retVal =
        prvCpssDxChLpmRamEntryGet(ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack],
                                  ipAddr,&ipPrefix,&dummy);
    if(retVal == GT_OK)
        currNh = *((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)dummy);

    while (retVal == GT_OK)
    {
        if (tableSizePtr != NULL)
        {
            *tableSizePtr += (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
                          sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC):
                sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC);
        }
        if (*tablePtrPtr != NULL)
        {
            /* copy the prefix info, start with ip address*/
            if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
            {
                cpssOsMemCpy(ipv4Prefix->ipAddr.arIP,ipAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
                cpssOsMemCpy(&ipv4Prefix->nextHopPointer,&currNh,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                ipv4Prefix->prefixLen = ipPrefix;
                ipv4Prefix->vrId = vrId;

                /* advance the pointer */
                ipv4Prefix = ipv4Prefix + 1;
            }
            else
            {
                cpssOsMemCpy(ipv6Prefix->ipAddr.arIP,ipAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
                cpssOsMemCpy(&ipv6Prefix->nextHopPointer,&currNh,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                ipv6Prefix->prefixLen = ipPrefix;
                ipv6Prefix->vrId = vrId;

                /* advance the pointer */
                ipv6Prefix = ipv6Prefix + 1;
            }
        }
        numOfEntries++;
        if (numOfEntries >= numOfEntriesLeft)
            /* we run out of entries , we'll conitune later */
            break;

        retVal =
            prvCpssDxChLpmRamEntryGet(ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack],
                                      ipAddr,&ipPrefix,&dummy);

        if(retVal == GT_OK)
            currNh = *((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)dummy);
    }

    /* check if we finished iterating*/
    if (retVal != GT_OK)
    {
        if (tableSizePtr != NULL)
        {
            /* make space for that the last one with invalid prefix of 255 */
            *tableSizePtr += (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
                sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC):
                sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC);

        }
        if (*tablePtrPtr != NULL)
        {
            /* indicate the last one with invalid prefix of 255 */
            if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
            {
                ipv4Prefix->prefixLen = 0xFF;
                /* advance the pointer */
                *tablePtrPtr = (GT_VOID *)(ipv4Prefix + 1);
            }
            else
            {
                ipv6Prefix->prefixLen = 0xFF;
                /* advance the pointer */
                *tablePtrPtr = (GT_VOID *)(ipv6Prefix + 1);
            }
        }
        numOfEntries++;
        /* free & zero the iterator */
        cpssOsLpmFree((GT_PTR)*iterPtr);
        *iterPtr = 0;

        retVal = GT_OK;
    }
    else
    {
        /* we didn't finish , record the currnet prefix. */
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC *tmpIpv4Prefix =
                (PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC*)*iterPtr;
            cpssOsMemCpy(tmpIpv4Prefix->ipAddr.arIP,ipAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
            tmpIpv4Prefix->prefixLen = ipPrefix;

            /* record the ptr */
            *tablePtrPtr = (GT_VOID *)ipv4Prefix;

        }
        else
        {
            PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC *tmpIpv6Prefix =
                (PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC*)*iterPtr;
            cpssOsMemCpy(tmpIpv6Prefix->ipAddr.arIP,ipAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
            tmpIpv6Prefix->prefixLen = ipPrefix;

            /* record the ptr */
            *tablePtrPtr = (GT_VOID *)ipv6Prefix ;
        }
    }

    /* update the left num of entries */
    numOfEntriesLeft -= numOfEntries;

    if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC));
    else
        *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC));

    return(retVal);
}

/**
* @internal ipLpmDbGetL3UcLpm function
* @endinternal
*
* @brief   Retrives a specifc device's core ip uc prefix memory Size needed and info
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3UcLpm
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT    GT_U32                           *tableSizePtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC tmpRootRange;

    GT_U32 memAllocIndex = 0;
    GT_U32 numOfEntriesLeft = 0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_OP_ENT traOp;
    PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC *memAllocInfo = NULL;

    if (ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL)
    {
        /* no uc prefixes */
        return GT_OK;
    }
    if (*tablePtrPtr != NULL)
    {
        /* set the params for lpm traverse */
        traOp = PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_REC_WITH_PCL_E;

        memAllocInfo = (PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC*)*tablePtrPtr;
    }
    else
    {
        traOp = PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_COUNT_WITH_PCL_E;
    }


    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));
    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    tmpRootRange.lowerLpmPtr.nextBucket =
        ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];
    tmpRootRange.pointerType =
        (GT_U8)(ipShadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack]);
    tmpRootRange.next = NULL;

    retVal = prvCpssDxChLpmRamMemTraverse(traOp,
                                          memAllocInfo,
                                          &memAllocIndex,
                                          numOfEntriesLeft,
                                          &tmpRootRange,
                                          GT_FALSE,
                                          ipShadowPtr->ipUcSearchMemArrayPtr[protocolStack],
                                          iterPtr);

    /* update the left num of entries */
    numOfEntriesLeft = numOfEntriesLeft - memAllocIndex;

    *dataSizePtr = *dataSizePtr - memAllocIndex * (sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));
    if (tableSizePtr != NULL)
    {
        /* calc the lpm memory need */
        *tableSizePtr += memAllocIndex * sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC);
    }
    if (*tablePtrPtr != NULL)
    {
        /* advance the pointer */
        *tablePtrPtr = (GT_VOID*)(&memAllocInfo[memAllocIndex]);
    }
    return (retVal);
}

/**
* @internal ipLpmDbGetL3McRoutes function
* @endinternal
*
* @brief   Retrives a specifc device's core ip mc routes memory Size needed and info
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3McRoutes
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT    GT_U32                           *tableSizePtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    GT_U8 groupIp[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS],srcAddr[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32 groupPrefix,srcPrefix;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC mcEntry;
    GT_U32 numOfEntries = 0;
    GT_U32 numOfEntriesLeft = 0;
    IPV6_HSU_MC_ROUTE_ENTRY_STC *ipv6McRoute;
    IPV4_HSU_MC_ROUTE_ENTRY_STC *ipv4McRoute;

    cpssOsMemSet(&mcEntry,0,sizeof(mcEntry));

    if (*iterPtr == 0)
    {
        /* this is the first call , so intilize */
        *iterPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E) ?
               (GT_UINTPTR)cpssOsLpmMalloc(sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC)):
               (GT_UINTPTR)cpssOsLpmMalloc(sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC));
        groupPrefix = 0;
    }
    else
    {
        /* get the current */
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            ipv4McRoute = (IPV4_HSU_MC_ROUTE_ENTRY_STC*)(*iterPtr);
            cpssOsMemCpy(groupIp,ipv4McRoute->mcGroup.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
            cpssOsMemCpy(srcAddr,ipv4McRoute->ipSrc.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));

            groupPrefix = (ipv4McRoute->mcGroup.u32Ip == 0)?0:32;
        }
        else
        {
            ipv6McRoute = (IPV6_HSU_MC_ROUTE_ENTRY_STC*)(*iterPtr);
            cpssOsMemCpy(groupIp,ipv6McRoute->mcGroup.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
            cpssOsMemCpy(srcAddr,ipv6McRoute->ipSrc.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));

            groupPrefix = ((ipv6McRoute->mcGroup.u32Ip[0] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[1] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[2] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[3] == 0))?0:128;
        }
    }


     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         numOfEntriesLeft = *dataSizePtr/(sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC));
     else
         numOfEntriesLeft = *dataSizePtr/(sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* set the ptrs */
    ipv4McRoute = (IPV4_HSU_MC_ROUTE_ENTRY_STC*)*tablePtrPtr;
    ipv6McRoute = (IPV6_HSU_MC_ROUTE_ENTRY_STC*)*tablePtrPtr;

    retVal = prvCpssDxChLpmRamMcEntryGetNext(vrId,groupIp,&groupPrefix,
                                             NULL,srcAddr,&srcPrefix,&mcEntry,
                                             protocolStack,ipShadowPtr);

    while (retVal == GT_OK)
    {
        if (tableSizePtr != NULL)
        {
            *tableSizePtr  +=
                (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
                sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC):
                sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC);
        }
        if (*tablePtrPtr != NULL)
        {
            if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
            {
                cpssOsMemCpy(ipv4McRoute->mcGroup.arIP,groupIp,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
                cpssOsMemCpy(ipv4McRoute->ipSrc.arIP,srcAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
                ipv4McRoute->ipSrcPrefixlength = srcPrefix;
                ipv4McRoute->vrId = vrId;
                cpssOsMemCpy(&ipv4McRoute->mcRoutePointer,&mcEntry,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                ipv4McRoute->last = GT_FALSE;

                ipv4McRoute++;
            }
            else
            {
                cpssOsMemCpy(ipv6McRoute->mcGroup.arIP,groupIp,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
                cpssOsMemCpy(ipv6McRoute->ipSrc.arIP,srcAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
                ipv6McRoute->vrId = vrId;
                ipv6McRoute->ipSrcPrefixlength = srcPrefix;
                cpssOsMemCpy(&ipv6McRoute->mcRoutePointer,&mcEntry,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                ipv6McRoute->last = GT_FALSE;
                ipv6McRoute++;
            }
        }

        numOfEntries++;

        if (numOfEntries >= numOfEntriesLeft)
            /* we run out of entries , we'll continue later */
            break;

        retVal = prvCpssDxChLpmRamMcEntryGetNext(vrId,groupIp,&groupPrefix,NULL,
                                                 srcAddr,&srcPrefix,&mcEntry,
                                                 protocolStack,ipShadowPtr);
    }

    /* check if we finished iterating*/
    if (retVal != GT_OK)
    {
        if (tableSizePtr != NULL)
        {
            /* make space for that the last one with invalid prefix of 255 */
            *tableSizePtr  += (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
                sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC):
                sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC);
        }

        if (*tablePtrPtr != NULL)
        {
            /* indicate last for mc*/
            if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
            {
                ipv4McRoute->last = GT_TRUE;
                *tablePtrPtr = (GT_VOID*)(ipv4McRoute + 1);
            }
            else
            {
                ipv6McRoute->last = GT_TRUE;
                *tablePtrPtr = (GT_VOID*)(ipv6McRoute + 1);
            }
        }
        numOfEntries++;
        /* free & zero the iterator */
        cpssOsLpmFree((GT_PTR)*iterPtr);
        *iterPtr = 0;

        retVal = GT_OK;
    }
    else
    {
        /* we didn't finish , record the currnet prefix. */
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            IPV4_HSU_MC_ROUTE_ENTRY_STC* tmpIpv4McRoute =
                (IPV4_HSU_MC_ROUTE_ENTRY_STC*)*iterPtr;
            cpssOsMemCpy(tmpIpv4McRoute->mcGroup.arIP,groupIp,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
            cpssOsMemCpy(tmpIpv4McRoute->ipSrc.arIP,srcAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
            /* record the ptr */
            *tablePtrPtr = (GT_VOID *)ipv4McRoute;
        }
        else
        {
            IPV6_HSU_MC_ROUTE_ENTRY_STC* tmpIpv6McRoute =
                (IPV6_HSU_MC_ROUTE_ENTRY_STC*)*iterPtr;
            cpssOsMemCpy(tmpIpv6McRoute->mcGroup.arIP,groupIp,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
            cpssOsMemCpy(tmpIpv6McRoute->ipSrc.arIP,srcAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
            /* record the ptr */
            *tablePtrPtr = (GT_VOID *)ipv6McRoute;
        }
    }

    /* update the left num of entries */
    numOfEntriesLeft -= numOfEntries;

     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC));
     else
         *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC));
    return (retVal);
}

/**
* @internal ipLpmDbGetL3McLpm function
* @endinternal
*
* @brief   Retrives a specifc device's core ip mc routes memory Size needed and info
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration step
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3McLpm
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT    GT_U32                           *tableSizePtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_EXMXPM_IPV6MC_PCL_ENTRY_STC ipv6McPclEntry,*ipv6McPclEntryPtr;
    GT_U32 memAllocIndex = 0;
    GT_U32 numOfEntriesLeft = 0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_OP_ENT traOp;
    PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC *memAllocInfo = NULL;

    /* check if mc prefixes are exist */
    if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E )
    {
        if(ipShadowPtr->vrRootBucketArray[vrId].mcRootBucket[CPSS_IP_PROTOCOL_IPV4_E] == NULL)
        {
            /* no ipv4 mc prefixes */
            return GT_OK;
        }
    }
    if (protocolStack == CPSS_IP_PROTOCOL_IPV6_E )
    {
        if(ipShadowPtr->vrRootBucketArray[vrId].mcRootBucket[CPSS_IP_PROTOCOL_IPV6_E] == NULL)
        {
            /* no ipv4 mc prefixes */
            return GT_OK;
        }
    }
    if (*tablePtrPtr != NULL)
    {
        /* set the params for lpm traverse */
        traOp = PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_REC_WITH_PCL_E;

        memAllocInfo = (PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC*)*tablePtrPtr;
    }
    else
    {
        traOp = PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_COUNT_WITH_PCL_E;
    }
    memAllocIndex = 0;


    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* move on to mc LPM*/
    retVal = prvCpssDxChLpmRamMcTraverse(traOp,memAllocInfo,
                                         &memAllocIndex,
                                         numOfEntriesLeft,
                                         vrId,ipShadowPtr,iterPtr);
    /* update the left num of entries */
    numOfEntriesLeft = numOfEntriesLeft - memAllocIndex;


    *dataSizePtr = *dataSizePtr - memAllocIndex * (sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    if (tableSizePtr != NULL)
    {
        /* calc the mc lpm memory need */
        *tableSizePtr  += memAllocIndex * sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC);
    }
    if (*tablePtrPtr != NULL)
    {
        /* advance the pointer */
        *tablePtrPtr = (GT_VOID*)(&memAllocInfo[memAllocIndex]);
    }
    return (retVal);
}

/**
* @internal ipLpmDbGetL3DmmFreeLists function
* @endinternal
*
* @brief   Retrives needed dmm free lists memory size and info.
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration step
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3DmmFreeLists
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    OUT    GT_U32                           *tableSizePtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_U32 numberInternalEntiresLeft = 0;
    GT_U32 numberInternalEntriesUsed = 0;
    DMM_IP_HSU_ENTRY_STC *dmmIterEntry;
    GT_U32 i = 0;
    GT_U32 j = 0;
    GT_U32 *hsuMemPtr;
    GT_DMM_BLOCK *tempPtr;
    if (*iterPtr == 0)
    {
        /* this is the first call , so intilize */
        *iterPtr = (GT_UINTPTR)cpssOsLpmMalloc(sizeof(DMM_IP_HSU_ENTRY_STC));
        if (*iterPtr == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        dmmIterEntry = (DMM_IP_HSU_ENTRY_STC*)(*iterPtr);
        dmmIterEntry->currentDmmBlockAddrInList = NULL;
        dmmIterEntry->currentFreeLinkList = 0;
        dmmIterEntry->partitionIndex = 0;
        dmmIterEntry->endOfList = 0;
        dmmIterEntry->endOfPartition = 0;
        cpssOsMemSet(dmmIterEntry->partitionArray,0,sizeof(dmmIterEntry->partitionArray));

        /* need to change logic, since for bobcat2 and above we support multi blocks mapping per octet*/
        CPSS_TBD_BOOKMARK

        /*this code need to be checked and the next code should be deleted*/

        for (j = 0, i= 0; (j <8) && (i < ipShadowPtr->numOfLpmMemories); i++, j++;)
        {
           dmmIterEntry->partitionArray[j] = (GT_DMM_PARTITION*)ipShadowPtr->lpmRamStructsMemPoolPtr[i];
        }

        /* init dmm partition array. Partition array should contain only different partition Ids */
        /*for (j = 0, i= 0; (j <8) && (i < ipShadowPtr->numOfLpmMemories); i++)
        {
           if (ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool ==
               ipShadowPtr->lpmMemInfoArray[1][i].structsMemPool)
           {
               dmmIterEntry->partitionArray[j] = (GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool;
               j++;
           }
           else
           {
               dmmIterEntry->partitionArray[j] = ((GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool);
               dmmIterEntry->partitionArray[j+1] = (GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[1][i].structsMemPool;
               j = j+2;
           }
        }
        */
    }
    else
    {
        dmmIterEntry = (DMM_IP_HSU_ENTRY_STC*)(*iterPtr);
    }

    numberInternalEntiresLeft = *dataSizePtr/(sizeof(GT_U32));

    if (numberInternalEntiresLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* set the pointer */
    hsuMemPtr = (GT_U32*)(*tablePtrPtr);
    i = dmmIterEntry->currentFreeLinkList;
    j = dmmIterEntry->partitionIndex;
    for (; j < 8; j ++)
    {
        if (dmmIterEntry->partitionArray[j] == NULL)
        {
            continue;
        }
        dmmIterEntry->partitionIndex = j;
        if (dmmIterEntry->endOfPartition == GT_FALSE)
        {
            for(;i < DMM_MAXIMAL_BLOCK_SIZE_ALLOWED+1;i++)
            {
                dmmIterEntry->currentFreeLinkList = i;
                if (i == 0)
                {
                    /* first link list in partition is examined. Write partition index */
                    if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                    {
                        if (tableSizePtr != NULL)
                        {
                            *tableSizePtr+= sizeof(GT_U32);
                        }
                        if (*tablePtrPtr != NULL)
                        {
                            *hsuMemPtr = j;
                            hsuMemPtr++;
                        }
                        numberInternalEntriesUsed++;
                    }
                    else
                    {
                        *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                        return GT_OK;
                    }
                }
                if (dmmIterEntry->currentDmmBlockAddrInList == NULL)
                {
                    /* new free list is in process */
                    if (dmmIterEntry->partitionArray[j]->tableOfSizePointers[i] == NULL)
                    {
                        /* no export action for empty free list */
                        continue;
                    }
                    else
                    {
                        /* write link list number-name */
                        if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                        {
                            if (tableSizePtr != NULL)
                            {
                                *tableSizePtr+= sizeof(GT_U32);
                            }
                            if (*tablePtrPtr != NULL)
                            {
                                *hsuMemPtr = i;
                                hsuMemPtr++;
                            }
                            numberInternalEntriesUsed++;
                        }
                        else
                        {
                            *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                            return GT_OK;
                        }
                        tempPtr = dmmIterEntry->partitionArray[j]->tableOfSizePointers[i];
                    }
                }
                else
                {
                    /*  free list from previuos iteration is not finished yet. Write
                        hw offset of remains link list members */
                    tempPtr = dmmIterEntry->currentDmmBlockAddrInList;
                }
                /* write hw offset of  link list members */
                if (dmmIterEntry->endOfList == GT_FALSE)
                {
                    do
                    {
                        dmmIterEntry->currentDmmBlockAddrInList = tempPtr;
                        if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                        {
                            if (tableSizePtr != NULL)
                            {
                                *tableSizePtr+= sizeof(GT_U32);
                            }
                            if (*tablePtrPtr != NULL)
                            {
                                *hsuMemPtr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempPtr);
                                hsuMemPtr++;
                            }
                            numberInternalEntriesUsed++;
                        }
                        else
                        {
                            *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                            return GT_OK;
                        }
                        tempPtr = tempPtr->nextBySizeOrPartitionPtr.nextBySize;
                    }while(tempPtr != NULL);
                }

                /* write end of the list */
                if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                {
                    if (tableSizePtr != NULL)
                    {
                        *tableSizePtr+= sizeof(GT_U32);
                    }
                    if (*tablePtrPtr != NULL)
                    {
                        *hsuMemPtr = PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_FREE_LINK_LIST_END_CNS;
                        hsuMemPtr++;
                    }
                    numberInternalEntriesUsed++;
                    dmmIterEntry->endOfList = GT_FALSE;
                }
                else
                {
                    dmmIterEntry->endOfList = GT_TRUE;
                    *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                    return GT_OK;
                }
                dmmIterEntry->currentDmmBlockAddrInList = NULL;
            }
            /* small blocks export was done */
            if (dmmIterEntry->currentFreeLinkList <= DMM_MAXIMAL_BLOCK_SIZE_ALLOWED)
            {
                dmmIterEntry->currentFreeLinkList = DMM_MAXIMAL_BLOCK_SIZE_ALLOWED+1;
            }
            /* now handle big blocks list */

            /* check if big block list is exist */
            if (dmmIterEntry->partitionArray[j]->bigBlocksList != NULL)
            {
                tempPtr = dmmIterEntry->partitionArray[j]->bigBlocksList;
                if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                {
                    if (tableSizePtr != NULL)
                    {
                        *tableSizePtr+= sizeof(GT_U32);
                    }
                    if (*tablePtrPtr != NULL)
                    {
                        *hsuMemPtr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempPtr);
                        hsuMemPtr++;
                    }
                    dmmIterEntry->currentFreeLinkList = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempPtr);
                    numberInternalEntriesUsed++;
                }
                else
                {
                    *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                    return GT_OK;
                }
                if (dmmIterEntry->currentDmmBlockAddrInList == NULL)
                {
                    /* first offset in big blocks list */
                    dmmIterEntry->currentDmmBlockAddrInList = dmmIterEntry->partitionArray[j]->bigBlocksList;
                }
                tempPtr = dmmIterEntry->currentDmmBlockAddrInList;
                do
                {
                    dmmIterEntry->currentDmmBlockAddrInList = tempPtr;
                    if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                    {
                        if (tableSizePtr != NULL)
                        {
                            *tableSizePtr+= sizeof(GT_U32);
                        }
                        if (*tablePtrPtr != NULL)
                        {
                            *hsuMemPtr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempPtr);
                            hsuMemPtr++;
                        }
                        numberInternalEntriesUsed++;
                    }
                    else
                    {
                        *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                        return GT_OK;
                    }
                    tempPtr = tempPtr->nextBySizeOrPartitionPtr.nextBySize;
                }while(tempPtr != NULL);
            }
            /* prepare free link list indexes for new partition */
            dmmIterEntry->currentDmmBlockAddrInList = NULL;
            i = 0;
            dmmIterEntry->currentFreeLinkList = 0;
        }
        /* partition is done : partition end signature is assigned */
        if (numberInternalEntiresLeft > numberInternalEntriesUsed )
        {
            if (tableSizePtr != NULL)
            {
                *tableSizePtr+= sizeof(GT_U32);
            }
            if (*tablePtrPtr != NULL)
            {
                *hsuMemPtr = PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_PARTITION_END_CNS;
                hsuMemPtr++;
            }
            numberInternalEntriesUsed++;
            dmmIterEntry->endOfPartition = GT_FALSE;
        }
        else
        {
            dmmIterEntry->endOfPartition = GT_TRUE;
            *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
            return GT_OK;
        }
    }
    *tablePtrPtr = (GT_VOID*)hsuMemPtr;
    /* in case all data was processed */
    /* update the left num of entries */

    numberInternalEntiresLeft -= numberInternalEntriesUsed;

    *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
    /* free & zero the iterator */
    cpssOsLpmFree((GT_PTR)*iterPtr);
    *iterPtr = 0;

    return GT_OK;
}

/**
* @internal ipLpmDbGetL3 function
* @endinternal
*
* @brief   Retrives a specifc shadow's ip Table memory Size needed and info
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in,out] iterationSizePtr         - points to data size in bytes supposed to be processed
*                                      in current iteration
* @param[in,out] iterPtr                  - points to the iterator, to start - set to 0.
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes).
* @param[out] tablePtr                 - points to the table size info block.
* @param[in,out] iterationSizePtr         - points to data size in bytes left after iteration .
* @param[in,out] iterPtr                  - points to the iterator, if = 0 then the operation is done.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad iterPtr
*/
GT_STATUS ipLpmDbGetL3
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    OUT   GT_U32                                *tableSizePtr,
    OUT   GT_VOID                               *tablePtr,
    INOUT GT_U32                                *iterationSizePtr,
    INOUT GT_UINTPTR                            *iterPtr
)
{
    GT_STATUS                       retVal = GT_OK;
    IP_HSU_ITERATOR_STC             *currentIterPtr;

    /* if HSU process called this function currDataAmount is size in bytes.
       otherwise it is number of entries */
    GT_U32                              *currDataSizePtr = iterationSizePtr;
    GT_U32                              curentDataSizeBeforeStage;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *ipShadowPtr;

    currentIterPtr = (IP_HSU_ITERATOR_STC*)*iterPtr;

    if (currentIterPtr == NULL)
    {
        /* we need to allocate the iterator */
        currentIterPtr =
            (IP_HSU_ITERATOR_STC*)cpssOsLpmMalloc(sizeof(IP_HSU_ITERATOR_STC));
        if (currentIterPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        /*reset it */
        cpssOsMemSet(currentIterPtr,0,sizeof(IP_HSU_ITERATOR_STC));

        /* set the starting stage */
        currentIterPtr->currStage = IP_HSU_VR_STAGE_E;
        currentIterPtr->magic = PRV_CPSS_EXMXPM_HSU_ITERATOR_MAGIC_NUMBER_CNC;
        *iterPtr = (GT_UINTPTR)currentIterPtr;

        /* init the table size */
        if (tableSizePtr != NULL)
        {
            *tableSizePtr = 0;
        }
    }
    else
    {
        if(currentIterPtr->magic != PRV_CPSS_EXMXPM_HSU_ITERATOR_MAGIC_NUMBER_CNC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    /* set the memory position */
    currentIterPtr->currMemPtr = tablePtr;

    for (;currentIterPtr->currShadowIdx < lpmDbPtr->numOfShadowCfg;
          currentIterPtr->currShadowIdx++)
    {
        ipShadowPtr = &lpmDbPtr->shadowArray[currentIterPtr->currShadowIdx];

        if (currentIterPtr->currStage == IP_HSU_VR_STAGE_E)
        {
            curentDataSizeBeforeStage = *currDataSizePtr;
            retVal = ipLpmDbGetL3Vr(ipShadowPtr,tableSizePtr,
                                    &currentIterPtr->currMemPtr,
                                    &currentIterPtr->currIter,
                                    currDataSizePtr);
            if ((retVal == GT_NO_RESOURCE)&&
                (*currDataSizePtr == curentDataSizeBeforeStage))
            {
                /* the stage is not done: the data size was smaller than entry in shadow */
                return GT_OK;
            }
            /* check if we finished with the phase */
            if (currentIterPtr->currIter == 0)
            {
                /* set the next stage */
                currentIterPtr->currStage = IP_HSU_UC_PREFIX_STAGE_E;
            }

            if ((retVal != GT_OK) || (*currDataSizePtr == 0))
            {
                return(retVal);
            }
        }

        if (currentIterPtr->currStage != IP_HSU_DMM_FREE_LIST_E)
        {
            for (;currentIterPtr->currProtocolStack < 2; currentIterPtr->currProtocolStack++)
            {
                if (ipShadowPtr->isIpVerIsInitialized[currentIterPtr->currProtocolStack] == GT_FALSE)
                {
                    continue;
                }

                while (currentIterPtr->currVrId < ipShadowPtr->vrfTblSize)
                {
                    /* go over all valid VR */
                    if ((ipShadowPtr->vrRootBucketArray[currentIterPtr->currVrId].valid == GT_FALSE) ||
                        (ipShadowPtr->vrRootBucketArray[currentIterPtr->currVrId].rootBucket[currentIterPtr->currProtocolStack] == NULL))
                    {
                        currentIterPtr->currVrId++;
                        continue;
                    }

                    if (currentIterPtr->currStage == IP_HSU_UC_PREFIX_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbGetL3UcPrefix(ipShadowPtr,
                                                      currentIterPtr->currVrId,
                                                      currentIterPtr->currProtocolStack,
                                                      tableSizePtr,
                                                      &currentIterPtr->currMemPtr,
                                                      &currentIterPtr->currIter,
                                                      currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }

                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_UC_LPM_STAGE_E;
                        }

                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return(retVal);
                        }
                    }

                    if (currentIterPtr->currStage == IP_HSU_UC_LPM_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbGetL3UcLpm(ipShadowPtr,
                                                   currentIterPtr->currVrId,
                                                   currentIterPtr->currProtocolStack,
                                                   tableSizePtr,
                                                   &currentIterPtr->currMemPtr,
                                                   &currentIterPtr->currIter,
                                                   currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_MC_PREFIX_STAGE_E;
                        }
                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return(retVal);
                        }
                    }

                    if (currentIterPtr->currStage == IP_HSU_MC_PREFIX_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbGetL3McRoutes(ipShadowPtr,
                                                      currentIterPtr->currVrId,
                                                      currentIterPtr->currProtocolStack,
                                                      tableSizePtr,
                                                      &currentIterPtr->currMemPtr,
                                                      &currentIterPtr->currIter,
                                                      currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_MC_LPM_STAGE_E;
                        }

                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return(retVal);
                        }
                    }

                    if (currentIterPtr->currStage == IP_HSU_MC_LPM_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbGetL3McLpm(ipShadowPtr,
                                                   currentIterPtr->currVrId,
                                                   currentIterPtr->currProtocolStack,
                                                   tableSizePtr,
                                                   &currentIterPtr->currMemPtr,
                                                   &currentIterPtr->currIter,
                                                   currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_UC_PREFIX_STAGE_E;
                            /* this is the end for this vrid */
                            currentIterPtr->currVrId++;
                        }
                        if (retVal != GT_OK)
                        {
                            return(retVal);
                        }
                   }
                }
                /* we finished with this protocol stack zero the vrid */
                currentIterPtr->currVrId = 0;
            }
        }

        /* the last stage is IP_HSU_DMM_FREE_LIST_E for this shadow*/
        currentIterPtr->currStage = IP_HSU_DMM_FREE_LIST_E;
        if (currentIterPtr->currStage == IP_HSU_DMM_FREE_LIST_E)
        {
            curentDataSizeBeforeStage = *currDataSizePtr;
            retVal = ipLpmDbGetL3DmmFreeLists(ipShadowPtr,
                                              tableSizePtr,
                                              &currentIterPtr->currMemPtr,
                                              &currentIterPtr->currIter,
                                              currDataSizePtr);
            if ((retVal == GT_NO_RESOURCE)&&
                (*currDataSizePtr == curentDataSizeBeforeStage))
            {
                /* the stage is not done: the data size was smaller than entry in shadow */
                return GT_OK;
            }
            /* check if we finished with the last phase */
            if (currentIterPtr->currIter == 0)
            {
                /* set the start stage */
                currentIterPtr->currStage = IP_HSU_VR_STAGE_E;
            }

            if ((retVal != GT_OK)||(currentIterPtr->currIter != 0))
            {
                return(retVal);
            }
        }
        /* we finished with the shadow , set the starting stage */
        currentIterPtr->currStage = IP_HSU_VR_STAGE_E;
    }

    /* we reached the end free the iterator */
    cpssOsLpmFree(currentIterPtr);
    *iterPtr = 0;

    return (GT_OK);
}

/**
* @internal ipLpmDbSetL3Vr function
* @endinternal
*
* @brief   Retrives the valid virtual routers
*
* @param[in] ipShadowPtr              - points to the shadow to retrieve from
* @param[in] tablePtrPtr              - the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3Vr
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *ipShadowPtr,
    IN     GT_VOID                              **tablePtrPtr,
    INOUT  GT_UINTPTR                           *iterPtr,
    INOUT  GT_U32                               *dataSizePtr
)
{
    GT_STATUS   retVal = GT_OK;
    GT_U32      numOfEntries = 0;
    GT_U32      numOfEntriesLeft = 0;

    PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC *vrEntry;

    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* set the pointer */
    vrEntry = (PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC*)(*tablePtrPtr);

    while ((vrEntry->isLast == GT_FALSE) && (numOfEntries < numOfEntriesLeft))
    {
        /* setting the VR automatically set the default UC and MC prefixes in the lpm DB */
        retVal = lpmVirtualRouterSet(vrEntry->vrId,
                                     vrEntry->defUcRouteEntrey[CPSS_IP_PROTOCOL_IPV4_E],
                                     vrEntry->defUcRouteEntrey[CPSS_IP_PROTOCOL_IPV6_E],
                                     vrEntry->defMcRouteEntrey[CPSS_IP_PROTOCOL_IPV4_E],
                                     vrEntry->defMcRouteEntrey[CPSS_IP_PROTOCOL_IPV6_E],
                                     vrEntry->protocolStack,
                                     vrEntry->vrIpUcSupport,
                                     vrEntry->vrIpMcSupport,
                                     vrEntry->defIpv6McRuleIndex,
                                     vrEntry->vrIpv6McPclId,
                                     ipShadowPtr,
                                     GT_FALSE);
        if (retVal != GT_OK)
        {
            return (retVal);
        }

        /* advance the pointer */
        vrEntry++;

        numOfEntries++;
    }

    /* check if we finished iterating*/
    if ( (vrEntry->isLast == GT_TRUE)&& (numOfEntriesLeft > numOfEntries))
    {
        /* zero the iterator */
        *iterPtr = 0;

        /* set the ptr */
        *tablePtrPtr = (GT_VOID *)(vrEntry + 1);
         numOfEntries++;
    }
    else
    {
         /* just indicate we didn't finish using a fake iterator */
        *iterPtr = 0xff;
        *tablePtrPtr = (GT_VOID *)vrEntry;
    }

    /* update the left num of entries */
    numOfEntriesLeft -= numOfEntries;

    *dataSizePtr = *dataSizePtr - numOfEntries *(sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC));
    return (retVal);
}

/**
* @internal ipLpmDbSetL3UcPrefix function
* @endinternal
*
* @brief   Set the data needed for core uc prefix shadow reconstruction used
*         after HSU.
* @param[in] ipShadowPtr              - points to the shadow to set in
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3UcPrefix
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    IN     GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopPointerPtr;
    GT_STATUS retVal = GT_OK;
    GT_U8 *ipAddr;
    GT_U32 ipPrefix;
    GT_U32 numOfEntries = 0;
    GT_U32 numOfEntriesLeft = 0;
    PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC *ipv6Prefix;
    PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC *ipv4Prefix;
    GT_BOOL isLast;

    /* set the pointer */
    ipv4Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC*)(*tablePtrPtr);
    ipv6Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC*)(*tablePtrPtr);

    /* start with uc prefixes */
    isLast = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
             (ipv4Prefix->prefixLen == 0xFF):(ipv6Prefix->prefixLen == 0xFF);


     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC));
     else
         numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }


    while ((!isLast) && (numOfEntries < numOfEntriesLeft))
    {
        /* copy the prefix info, start with ip address*/
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            ipAddr = ipv4Prefix->ipAddr.arIP;
            ipPrefix = ipv4Prefix->prefixLen;
            nextHopPointerPtr = &ipv4Prefix->nextHopPointer;
            vrId = ipv4Prefix->vrId;

            /* advance the pointer */
            ipv4Prefix = ipv4Prefix + 1;

            isLast = (ipv4Prefix->prefixLen == 0xFF);

        }
        else
        {
            ipAddr = ipv6Prefix->ipAddr.arIP;
            ipPrefix = (GT_U8)ipv6Prefix->prefixLen;
            nextHopPointerPtr = &ipv6Prefix->nextHopPointer;
            vrId = ipv6Prefix->vrId;

            /* advance the pointer */
            ipv6Prefix = ipv6Prefix + 1;

            isLast = (ipv6Prefix->prefixLen == 0xFF);

        }

        /* The default UC was already defined in the call to ipLpmDbSetL3Vr */
        if (ipPrefix != 0)
        {
            retVal = prvCpssDxChLpmRamUcEntryAdd(vrId,ipAddr,ipPrefix,nextHopPointerPtr,
                                                 PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E,
                                                 GT_FALSE,protocolStack,ipShadowPtr);
        }

        if (retVal != GT_OK)
        {
            return (retVal);
        }

        numOfEntries++;

    }

    /* check if we finished */
    if ( (isLast == GT_TRUE) && (numOfEntries < numOfEntriesLeft))
    {
        /* advance the pointer */
        *tablePtrPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
            (GT_VOID *)(ipv4Prefix + 1) : (GT_VOID *)(ipv6Prefix + 1);
        numOfEntries++;
        /* just indicate we finished using a fake iterator */
        *iterPtr = 0;
    }
    else
    {
        /* just indicate we didn't finish using a fake iterator */
        *iterPtr = 0xff;

        /* and record the pointer */
        *tablePtrPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
            (GT_VOID *)ipv4Prefix : (GT_VOID *)ipv6Prefix;
    }

    /* update the num of entries left */
    numOfEntriesLeft -= numOfEntries;

    if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC));
    else
        *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC));
    return (retVal);
}

/**
* @internal ipLpmDbSetL3UcLpm function
* @endinternal
*
* @brief   Set the data needed for core uc LPM shadow reconstruction used
*         after HSU.
* @param[in] ipShadowPtr              - points to the shadow to set in
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3UcLpm
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    IN     GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC tmpRootRange;

    GT_U32 memAllocIndex = 0;
    GT_U32 numOfEntriesLeft = 0;

    PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC *memAllocInfo = NULL;

    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* check uc prefixes */
    if (ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL)
    {
        /* no uc prefixes */
        return GT_OK;
    }
    /* set the pointer for lpm traverse */
    memAllocInfo = (PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC*)*tablePtrPtr;

    memAllocIndex = 0;

    tmpRootRange.lowerLpmPtr.nextBucket =
    ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];
    tmpRootRange.pointerType =
    (GT_U8)(ipShadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack]);
    tmpRootRange.next = NULL;
    retVal = prvCpssDxChLpmRamMemTraverse(PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_SET_WITH_PCL_E,
                                          memAllocInfo,
                                          &memAllocIndex,
                                          numOfEntriesLeft,
                                          &tmpRootRange,
                                          GT_FALSE,
                                          ipShadowPtr->ipUcSearchMemArrayPtr[protocolStack],
                                          iterPtr);

    /* update the left num of entries */
    numOfEntriesLeft = numOfEntriesLeft - memAllocIndex;

    *dataSizePtr = *dataSizePtr - memAllocIndex * (sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    /* advance the pointer */
    *tablePtrPtr = (GT_VOID*)(&memAllocInfo[memAllocIndex]);

    return (retVal);
}

/**
* @internal ipLpmDbSetL3McRoutes function
* @endinternal
*
* @brief   Set the data needed for core shadow reconstruction used after HSU.
*
* @param[in] ipShadowPtr              - points to the shadow to set in
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3McRoutes
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *ipShadowPtr,
    IN     GT_U32                               vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN     GT_VOID                              **tablePtrPtr,
    INOUT  GT_UINTPTR                           *iterPtr,
    INOUT  GT_U32                               *dataSizePtr
)
{
    GT_STATUS                               retVal = GT_OK;
    GT_U8                                   *groupIp = NULL;
    GT_U8                                   *srcAddr = NULL;
    GT_U32                                  groupPrefix,srcPrefix;

    /* regarding group scope, here we give a false one, it will
       updated as part of the traverse function on the ipv6 groups */
    CPSS_IPV6_PREFIX_SCOPE_ENT          groupScope =
                                            CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    CPSS_EXMXPM_IP_MC_ADDRESS_TYPE_ENT  addressType =
                                            CPSS_EXMXPM_IP_MC_REGULAR_ADDRESS_TYPE_E;

    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcEntry;

    GT_U32 numOfEntries = 0;
    GT_U32 numOfEntriesLeft = 0;

    IPV6_HSU_MC_ROUTE_ENTRY_STC *ipv6McRoute;
    IPV4_HSU_MC_ROUTE_ENTRY_STC *ipv4McRoute;

    GT_BOOL isLast;

    ipv4McRoute = (IPV4_HSU_MC_ROUTE_ENTRY_STC*)*tablePtrPtr;
    ipv6McRoute = (IPV6_HSU_MC_ROUTE_ENTRY_STC*)*tablePtrPtr;

    isLast = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
             ipv4McRoute->last:ipv6McRoute->last;

     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         numOfEntriesLeft = *dataSizePtr/(sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC));
     else
         numOfEntriesLeft = *dataSizePtr/(sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
    {
        groupIp =  (GT_U8 *)cpssOsMalloc(sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS);
        srcAddr =  (GT_U8 *)cpssOsMalloc(sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS);
    }
    else
    {
        groupIp =  (GT_U8 *)cpssOsMalloc(sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS);
        srcAddr =  (GT_U8 *)cpssOsMalloc(sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS);
    }
     cpssOsMemSet(groupIp, 0, sizeof(groupIp));
     cpssOsMemSet(srcAddr, 0, sizeof(srcAddr));


    while ((isLast == GT_FALSE) && (numOfEntries < numOfEntriesLeft))
    {
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            cpssOsMemCpy(groupIp,ipv4McRoute->mcGroup.arIP,sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS);
            groupPrefix = (ipv4McRoute->mcGroup.u32Ip == 0)?0:32;

            cpssOsMemCpy(srcAddr,ipv4McRoute->ipSrc.arIP,sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS);
            srcPrefix = ipv4McRoute->ipSrcPrefixlength;

            vrId = ipv4McRoute->vrId;
            mcEntry = &ipv4McRoute->mcRoutePointer;

            ipv4McRoute++;

            isLast = ipv4McRoute->last;
        }
        else
        {
            cpssOsMemCpy(groupIp,ipv6McRoute->mcGroup.arIP,sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS);
            groupPrefix = ((ipv6McRoute->mcGroup.u32Ip[0] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[1] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[2] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[3] == 0))?0:128;

            cpssOsMemCpy(srcAddr,ipv6McRoute->ipSrc.arIP,sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS);
            srcPrefix = ipv6McRoute->ipSrcPrefixlength;

            vrId = ipv6McRoute->vrId;
            mcEntry = &ipv6McRoute->mcRoutePointer;

            ipv6McRoute++;

            isLast = ipv6McRoute->last;
        }

        /* we are overriding an entry that exists in core*/
        retVal = prvCpssDxChLpmRamMcEntryAdd(vrId,groupIp,groupPrefix,addressType,
                                             0,srcAddr,srcPrefix,
                                             mcEntry,GT_FALSE,
                                             PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E,
                                             protocolStack,ipShadowPtr);
        if (retVal != GT_OK)
        {
            cpssOsFree(groupIp);
            cpssOsFree(srcAddr);
            return (retVal);
        }

        numOfEntries++;

    }

    /* check if we finished */
    if ( (isLast == GT_TRUE) && (numOfEntries < numOfEntriesLeft) )
    {
        /* just indicate we finished using a fake iterator */
        *iterPtr = 0;

        /* advance the pointer */
        *tablePtrPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
            (GT_VOID *)(ipv4McRoute + 1) : (GT_VOID *)(ipv6McRoute + 1);
        numOfEntries++;
    }
    else
    {
        /* just indicate we didn't finish using a fake iterator */
        *iterPtr = 0xff;

        /* and record the pointer */
        *tablePtrPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
            (GT_VOID *)ipv4McRoute : (GT_VOID *)ipv6McRoute;
    }

    /* update the num of entries left */
    numOfEntriesLeft -= numOfEntries;

     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC));
     else
         *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC));
    cpssOsFree(groupIp);
    cpssOsFree(srcAddr);
    return (retVal);
}

/**
* @internal ipLpmDbSetL3McLpm function
* @endinternal
*
* @brief   Set the data needed for core shadow reconstruction used after HSU.
*
* @param[in] ipShadowPtr              - points to the shadow to set in
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3McLpm
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *ipShadowPtr,
    IN     GT_U32                               vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN     GT_VOID                              **tablePtrPtr,
    INOUT  GT_UINTPTR                           *iterPtr,
    INOUT  GT_U32                               *dataSizePtr
)
{
    GT_STATUS                                   retVal = GT_OK;
    PRV_CPSS_EXMXPM_IPV6MC_PCL_ENTRY_STC        ipv6McPclEntry,*ipv6McPclEntryPtr;
    GT_U32                                      memAllocIndex = 0;
    GT_U32                                      numOfEntriesLeft = 0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC    *memAllocInfo = NULL;
    /* set the pointer for lpm traverse */

    /* check if mc prefixes are exist */
    if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E )
    {
        if(ipShadowPtr->vrRootBucketArray[vrId].mcRootBucket[CPSS_IP_PROTOCOL_IPV4_E] == NULL)
        {
            /* no ipv4 mc prefixes */
            return GT_OK;
        }
    }
    if (protocolStack == CPSS_IP_PROTOCOL_IPV6_E )
    {
        if(ipShadowPtr->vrRootBucketArray[vrId].mcRootBucket[CPSS_IP_PROTOCOL_IPV6_E] == NULL)
        {
            /* no ipv4 mc prefixes */
            return GT_OK;
        }
    }
    memAllocInfo = (PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC*)*tablePtrPtr;
    memAllocIndex = 0;

    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* move on to mc LPM*/
    retVal = prvCpssDxChLpmRamMcTraverse(PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_SET_WITH_PCL_E,
                                         memAllocInfo,&memAllocIndex,
                                         numOfEntriesLeft,vrId,ipShadowPtr,
                                         iterPtr);
    /* advance the pointer */
    *tablePtrPtr = (GT_VOID*)(&memAllocInfo[memAllocIndex]);

    /* update the left num of entries */
    numOfEntriesLeft = numOfEntriesLeft - memAllocIndex;

    *dataSizePtr = *dataSizePtr - memAllocIndex * (sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    return (retVal);
}

/**
* @internal ipLpmDbSetL3DmmFreeLists function
* @endinternal
*
* @brief   Set needed dmm free lists in dmm manager .
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration step
*
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3DmmFreeLists
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_U32 numberInternalEntiresLeft = 0;
    GT_U32 numberInternalEntriesUsed = 0;
    DMM_IP_HSU_ENTRY_STC *dmmIterEntry;
    GT_DMM_BLOCK *tempDmmPtr = NULL;
    GT_DMM_BLOCK *foundDmmBlock = NULL;
    GT_DMM_BLOCK *tempNextBySize = NULL;
    GT_DMM_BLOCK *tempPrevBySize = NULL;
    GT_DMM_BLOCK *tempCurrentNextBySize = NULL;
    GT_BOOL updateSmallFreeListArray = GT_FALSE;
    GT_BOOL updateBigFreeList = GT_FALSE;
    GT_U32 tempListName = 0;
    GT_U32 i = 0;
    GT_U32 j = 0;
    GT_U32 *hsuMemPtr;
    GT_U32 data;

    if (*iterPtr == 0)
    {
        /* this is the first call , so intilize */
        *iterPtr = (GT_UINTPTR)cpssOsLpmMalloc(sizeof(DMM_IP_HSU_ENTRY_STC));
        if (*iterPtr == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        dmmIterEntry = (DMM_IP_HSU_ENTRY_STC*)(*iterPtr);
        dmmIterEntry->currentDmmBlockAddrInList = NULL;
        dmmIterEntry->currentFreeLinkList = 0xffffffff;
        dmmIterEntry->partitionIndex = 0;
        dmmIterEntry->endOfList = 0;
        dmmIterEntry->endOfPartition = GT_TRUE;
        cpssOsMemSet(dmmIterEntry->partitionArray,0,sizeof(dmmIterEntry->partitionArray));

        /* need to change logic, since for bobcat2 and above we support multi blocks mapping per octet*/
        CPSS_TBD_BOOKMARK

        /*this code need to be checked and the next code should be deleted*/

        for (j = 0, i= 0; (j <8) && (i < ipShadowPtr->numOfLpmMemories); i++, j++;)
        {
           dmmIterEntry->partitionArray[j] = (GT_DMM_PARTITION*)ipShadowPtr->lpmRamStructsMemPoolPtr[i];
        }

        /* init dmm partition array. Partition array should contain only different partition Ids */
        /*for (j = 0, i= 0; (j <8) && (i < ipShadowPtr->numOfLpmMemories); i++)
        {
           if (ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool ==
               ipShadowPtr->lpmMemInfoArray[1][i].structsMemPool)
           {
               dmmIterEntry->partitionArray[j] = (GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool;
               j++;
           }
           else
           {
               dmmIterEntry->partitionArray[j] = (GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool;
               dmmIterEntry->partitionArray[j+1] = (GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[1][i].structsMemPool;
               j = j+2;
           }
        }
        */
    }
    else
    {
        dmmIterEntry = (DMM_IP_HSU_ENTRY_STC*)(*iterPtr);
    }

    numberInternalEntiresLeft = *dataSizePtr/(sizeof(GT_U32));

    if (numberInternalEntiresLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* set the pointer */

    hsuMemPtr = (GT_U32*)(*tablePtrPtr);
    j = dmmIterEntry->partitionIndex;
    for (; j < 8; j ++)
    {
        if (dmmIterEntry->partitionArray[j] == NULL)
        {
            continue;
        }

        /* partition index in the begining of each partition data */
        /* first reading of new partition block */
        if (dmmIterEntry->endOfPartition == GT_TRUE)
        {
            dmmIterEntry->partitionIndex = j;
            if (numberInternalEntiresLeft > numberInternalEntriesUsed )
            {
                data = *hsuMemPtr;
                hsuMemPtr++;
                numberInternalEntriesUsed++;
                dmmIterEntry->endOfPartition = GT_FALSE;
            }
            else
            {
                *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                return GT_OK;
            }
            if (dmmIterEntry->partitionIndex != data)
            {
                /* export and import are not synchronized */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        while (1)
        {
            if (numberInternalEntiresLeft > numberInternalEntriesUsed )
            {
                data = *hsuMemPtr;
                hsuMemPtr++;
                numberInternalEntriesUsed++;
            }
            else
            {
                *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                return GT_OK;
            }
            if (data == PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_PARTITION_END_CNS)
            {
                /* partition is done */
                dmmIterEntry->endOfPartition = GT_TRUE;
                dmmIterEntry->currentFreeLinkList = 0xffffffff;
                break;
            }

            if (data == PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_FREE_LINK_LIST_END_CNS)
            {
                /* this list was finished */
                dmmIterEntry->currentFreeLinkList = 0xffffffff;
                continue;
            }
            /* if iteration mode is activated it is importent to understand when
               link list name is read and when hw offset is read */
            if (dmmIterEntry->currentFreeLinkList == 0xffffffff)
            {
                dmmIterEntry->currentFreeLinkList = data;
                dmmIterEntry->currentDmmBlockAddrInList = NULL;
            }
            else
            {
                tempListName = dmmIterEntry->currentFreeLinkList;
                /* hw offset was read -- set it in existing list */
                if (dmmIterEntry->currentDmmBlockAddrInList == NULL)
                {
                    /* first hw offset in this list */
                    if (tempListName <= DMM_MAXIMAL_BLOCK_SIZE_ALLOWED)
                    {
                        updateSmallFreeListArray = GT_TRUE;
                        dmmIterEntry->currentDmmBlockAddrInList = dmmIterEntry->partitionArray[dmmIterEntry->partitionIndex]->tableOfSizePointers[tempListName];
                    }
                    else
                    {
                        /* big blocks */
                        updateBigFreeList = GT_TRUE;
                        dmmIterEntry->currentDmmBlockAddrInList = dmmIterEntry->partitionArray[dmmIterEntry->partitionIndex]->bigBlocksList;
                    }
                }
                /* find the element with received hw offset in current list */

                if (dmmIterEntry->currentDmmBlockAddrInList == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
                /* except of case when  first hw offset in the list was read
                  currentDmmBlockAddrInList is already processed so we start
                  from the next. If next dmm block == NULL, it is bad state:
                  list is done but end of list still wasn't arrived  */

                if ( (updateSmallFreeListArray == GT_TRUE) || (updateBigFreeList == GT_TRUE) )
                {
                    tempDmmPtr = dmmIterEntry->currentDmmBlockAddrInList;
                }
                else
                {
                    tempDmmPtr = dmmIterEntry->currentDmmBlockAddrInList->nextBySizeOrPartitionPtr.nextBySize;
                }
                if (tempDmmPtr == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
                foundDmmBlock = NULL;
                while (tempDmmPtr != NULL)
                {
                    if (PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempDmmPtr) == data)
                    {
                        foundDmmBlock = tempDmmPtr;
                        break;
                    }
                    tempDmmPtr = tempDmmPtr->nextBySizeOrPartitionPtr.nextBySize;
                }
                if (foundDmmBlock == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                if (foundDmmBlock->prevBySize == NULL)
                {
                    /* the found block with correct hw offset is first in the list */
                    /* it is the right place - no move is needed */
                    updateSmallFreeListArray = GT_FALSE;
                    updateBigFreeList = GT_FALSE;
                    dmmIterEntry->currentDmmBlockAddrInList = foundDmmBlock;
                    continue;
                }
                if ( (updateSmallFreeListArray == GT_FALSE) && (updateBigFreeList == GT_FALSE) )
                {
                    /* after first iteration in list is done and first currentDmmBlockAddrInList
                       moved on its constant place */
                    if (foundDmmBlock->prevBySize == dmmIterEntry->currentDmmBlockAddrInList)
                    {
                        /* the foundDmmBlock block is not first in the list. This is the
                           case when founded block should be located just after previously
                           found block. So if this block points by prevBySize on the
                           currentDmmBlockAddrInList --  it is the right place - no move is needed */
                        dmmIterEntry->currentDmmBlockAddrInList = foundDmmBlock;
                        continue;
                    }
                }
                /* the block with the same hw offset is found */
                tempDmmPtr = dmmIterEntry->currentDmmBlockAddrInList;
                tempCurrentNextBySize = tempDmmPtr->nextBySizeOrPartitionPtr.nextBySize;
                tempNextBySize = foundDmmBlock->nextBySizeOrPartitionPtr.nextBySize;
                tempPrevBySize = foundDmmBlock->prevBySize;

                if ( (updateSmallFreeListArray == GT_TRUE) || (updateBigFreeList == GT_TRUE) )
                {
                    /* set this block as first in the list */
                    if (updateSmallFreeListArray == GT_TRUE)
                    {
                        dmmIterEntry->partitionArray[j]->tableOfSizePointers[tempListName] = foundDmmBlock;
                    }
                    else
                    {
                        dmmIterEntry->partitionArray[j]->bigBlocksList = foundDmmBlock;
                    }
                    foundDmmBlock->prevBySize = NULL;
                    foundDmmBlock->nextBySizeOrPartitionPtr.nextBySize = tempDmmPtr;
                    tempDmmPtr->prevBySize = foundDmmBlock;
                    updateSmallFreeListArray = GT_FALSE;
                    updateBigFreeList = GT_FALSE;
                }
                else
                {
                    /* set this block after current block in the list */
                    foundDmmBlock->prevBySize = tempDmmPtr;
                    foundDmmBlock->nextBySizeOrPartitionPtr.nextBySize = tempDmmPtr->nextBySizeOrPartitionPtr.nextBySize;
                    tempDmmPtr->nextBySizeOrPartitionPtr.nextBySize = foundDmmBlock;
                    tempCurrentNextBySize->prevBySize = foundDmmBlock;
                }
                if (tempNextBySize != NULL)
                {
                    /* last element in the list */
                    tempNextBySize->prevBySize = tempPrevBySize;
                }
                tempPrevBySize->nextBySizeOrPartitionPtr.nextBySize = tempNextBySize;
                dmmIterEntry->currentDmmBlockAddrInList = foundDmmBlock;
            }
        }
    }
    /* in case all data was processed */
    /* update the left num of entries */

    numberInternalEntiresLeft -= numberInternalEntriesUsed;

    *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
    /* free & zero the iterator */
    cpssOsLpmFree((GT_PTR)*iterPtr);
    *iterPtr = 0;

    return GT_OK;
}

/**
* @internal ipLpmDbSetL3 function
* @endinternal
*
* @brief   Set the data needed for core IP shadow reconstruction used after HSU
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] tablePtr                 - points to the table size info block
* @param[in,out] iterationSizePtr         - points to data size in bytes supposed to be processed
*                                      in current iteration
* @param[in,out] iterPtr                  - points to the iterator, to start - set to 0.
* @param[in,out] iterationSizePtr         - points to data size in bytes left after iteration step.
* @param[in,out] iterPtr                  - points to the iterator, if = 0 then the operation is done.
*
* @retval GT_OK                    - on success
*/
GT_STATUS ipLpmDbSetL3
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN    GT_VOID                               *tablePtr,
    INOUT GT_U32                                *iterationSizePtr,
    INOUT GT_UINTPTR                            *iterPtr
)
{
    GT_STATUS retVal = GT_OK;
    IP_HSU_ITERATOR_STC *currentIterPtr;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *ipShadowPtr;
    GT_U32                              *currDataSizePtr = iterationSizePtr;
    GT_U32                              curentDataSizeBeforeStage;

    currentIterPtr = (IP_HSU_ITERATOR_STC*)*iterPtr;

    if (currentIterPtr == NULL)
    {
        /* we need to allocate the iterator */
        currentIterPtr =
        (IP_HSU_ITERATOR_STC*)cpssOsLpmMalloc(sizeof(IP_HSU_ITERATOR_STC));
        if (currentIterPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        /*reset it */
        cpssOsMemSet(currentIterPtr,0,sizeof(IP_HSU_ITERATOR_STC));
        /* set the starting stage */
        currentIterPtr->currStage = IP_HSU_VR_STAGE_E;
       *iterPtr = (GT_UINTPTR)currentIterPtr;
    }
     /* set the memory position */
        currentIterPtr->currMemPtr = tablePtr;
    for (; currentIterPtr->currShadowIdx < lpmDbPtr->numOfShadowCfg;
        currentIterPtr->currShadowIdx++)
    {
        ipShadowPtr = &lpmDbPtr->shadowArray[currentIterPtr->currShadowIdx];

        if (currentIterPtr->currStage == IP_HSU_VR_STAGE_E)
        {
            curentDataSizeBeforeStage = *currDataSizePtr;
            retVal = ipLpmDbSetL3Vr(ipShadowPtr,
                                    &currentIterPtr->currMemPtr,
                                    &currentIterPtr->currIter,
                                    currDataSizePtr);
            if ((retVal == GT_NO_RESOURCE)&&
                (*currDataSizePtr == curentDataSizeBeforeStage))
            {
                /* the stage is not done: the data size was smaller than entry in shadow */
                return GT_OK;
            }
            /* check if we finished with the phase */
            if (currentIterPtr->currIter == 0)
            {
                /* set the next stage */
                currentIterPtr->currStage = IP_HSU_UC_PREFIX_STAGE_E;
            }

            if ((retVal != GT_OK) || (*currDataSizePtr == 0))
            {
                return (retVal);
            }
        }
        if (currentIterPtr->currStage != IP_HSU_DMM_FREE_LIST_E)
        {
            for (; currentIterPtr->currProtocolStack < 2; currentIterPtr->currProtocolStack++)
            {
                if (ipShadowPtr->isIpVerIsInitialized[currentIterPtr->currProtocolStack] == GT_FALSE)
                {
                    continue;
                }

                while (currentIterPtr->currVrId < ipShadowPtr->vrfTblSize)
                {
                    /* go over all valid VR */
                    if ((ipShadowPtr->vrRootBucketArray[currentIterPtr->currVrId].valid == GT_FALSE) ||
                        (ipShadowPtr->vrRootBucketArray[currentIterPtr->currVrId].rootBucket == NULL))
                    {
                        currentIterPtr->currVrId++;
                        continue;
                    }

                    if (currentIterPtr->currStage == IP_HSU_UC_PREFIX_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbSetL3UcPrefix(ipShadowPtr,
                                                      currentIterPtr->currVrId,
                                                      currentIterPtr->currProtocolStack,
                                                      &currentIterPtr->currMemPtr,
                                                      &currentIterPtr->currIter,
                                                      currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_UC_LPM_STAGE_E;
                        }

                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return (retVal);
                        }
                    }

                    if (currentIterPtr->currStage == IP_HSU_UC_LPM_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbSetL3UcLpm(ipShadowPtr,
                                                   currentIterPtr->currVrId,
                                                   currentIterPtr->currProtocolStack,
                                                   &currentIterPtr->currMemPtr,
                                                   &currentIterPtr->currIter,
                                                   currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }

                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_MC_PREFIX_STAGE_E;
                        }

                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return (retVal);
                        }

                    }

                    if (currentIterPtr->currStage == IP_HSU_MC_PREFIX_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbSetL3McRoutes(ipShadowPtr,
                                                      currentIterPtr->currVrId,
                                                      currentIterPtr->currProtocolStack,
                                                      &currentIterPtr->currMemPtr,
                                                      &currentIterPtr->currIter,
                                                      currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_MC_LPM_STAGE_E;
                        }

                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return (retVal);
                        }
                    }
                    if (currentIterPtr->currStage == IP_HSU_MC_LPM_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbSetL3McLpm(ipShadowPtr,
                                                   currentIterPtr->currVrId,
                                                   currentIterPtr->currProtocolStack,
                                                   &currentIterPtr->currMemPtr,
                                                   &currentIterPtr->currIter,
                                                   currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_UC_PREFIX_STAGE_E;

                            /* we finished with this vrid go to the next */
                            currentIterPtr->currVrId++;
                        }
                        if (retVal != GT_OK)
                        {
                            return (retVal);
                        }
                    }
                }
                /* we finished with this protocol stack zero the vrid */
                currentIterPtr->currVrId = 0;
            }

        }

        /* the last stage is IP_HSU_DMM_FREE_LIST_E */
        currentIterPtr->currStage = IP_HSU_DMM_FREE_LIST_E;
        if (currentIterPtr->currStage == IP_HSU_DMM_FREE_LIST_E)
        {
            curentDataSizeBeforeStage = *currDataSizePtr;
            retVal = ipLpmDbSetL3DmmFreeLists(ipShadowPtr,
                                              &currentIterPtr->currMemPtr,
                                              &currentIterPtr->currIter,
                                              currDataSizePtr);
            if ((retVal == GT_NO_RESOURCE)&&
                (*currDataSizePtr == curentDataSizeBeforeStage))
            {
                /* the stage is not done: the data size was smaller than entry in shadow */
                return GT_OK;
            }
            /* check if we finished with the phase */
            if (currentIterPtr->currIter == 0)
            {
                /* set the next stage */
                currentIterPtr->currStage = IP_HSU_UC_PREFIX_STAGE_E;
            }
            if ((retVal != GT_OK) || ( currentIterPtr->currIter != 0))
            {
                /*osSemSignal(ipShadowPtr->ipUnitSem);*/
                return(retVal);
            }
        }
        /* we finished with the shadow , set the starting stage */
        currentIterPtr->currStage = IP_HSU_VR_STAGE_E;
    }
    /* we reached the end free the iterator */
    cpssOsLpmFree(currentIterPtr);
    *iterPtr = 0;

    return (GT_OK);
}

/**
* @internal cpssExMxPmIpLpmDbMemSizeGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function calculates the memory size needed to export the Lpm DB
*         (used for HSU and unit Hotsync)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
*
* @param[out] lpmDbSizePtr             - pointer to lpmDb size calculated in bytes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssExMxPmIpLpmDbMemSizeGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC    *lpmDbPtr,
    OUT GT_U32                                  *lpmDbSizePtr
)
{
    GT_UINTPTR iterPtr = 0;
    GT_U32 iterationSise = 0xffffffff;/* perform size calculating in one iteration*/
    return ipLpmDbGetL3(lpmDbPtr,lpmDbSizePtr,NULL,&iterationSise,&iterPtr);
}

/**
* @internal cpssExMxPmIpLpmDbExport function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function exports the Lpm DB into the preallocated memory,
*         (used for HSU and unit Hotsync)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in,out] lpmDbMemBlockSizePtr     - pointer to block data size supposed to be exported
*                                      in current iteration.
* @param[in] lpmDbMemBlockPtr         - pointer to allocated for lpm DB memory area
* @param[in,out] iterPtr                  - the iterator, to start - set to 0.
* @param[in,out] lpmDbMemBlockSizePtr     - pointer to block data size that was not used
*                                      in current iteration.
* @param[in,out] iterPtr                  - the iterator, if = 0 then the operation is done.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Change behaving: from this version treatment of lpmDbMemBlockPtr was changed.
*       Now assumption is that it points on concrete block allocated only for
*       this iteration and not on big contiguous memory block for whole export/import
*       operation. User working with contiguous memory should give exact pointer
*       for next iteration: previous pointer + calculated iteration size.
*
*/
GT_STATUS cpssExMxPmIpLpmDbExport
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    INOUT  GT_U32                               *lpmDbMemBlockSizePtr,
    IN     GT_U32                               *lpmDbMemBlockPtr,
    INOUT  GT_UINTPTR                           *iterPtr
)
{
    return ipLpmDbGetL3(lpmDbPtr,NULL,lpmDbMemBlockPtr,lpmDbMemBlockSizePtr,iterPtr);
}

/**
* @internal cpssExMxPmIpLpmDbImport function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function imports the Lpm DB recived and reconstruct it,
*         (used for HSU and unit Hotsync)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in,out] lpmDbMemBlockSizePtr     - pointer to block data size supposed to be imported
*                                      in current iteration.
* @param[in] lpmDbMemBlockPtr         - pointer to allocated for lpm DB memory area
* @param[in,out] iterPtr                  - the iterator, to start - set to 0.
* @param[in,out] lpmDbMemBlockSizePtr     - pointer to block data size that was not used
*                                      in current iteration.
* @param[in,out] iterPtr                  - the iterator, if = 0 then the operation is done.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Change behaving: from this version treatment of lpmDbMemBlockPtr was changed.
*       Now assumption is that it points on concrete block allocated only for
*       this iteration and not on big contiguous memory block for whole export/import
*       operation. User working with contiguous memory should give exact pointer
*       for next iteration: previous pointer + calculated iteration size.
*
*/
GT_STATUS cpssExMxPmIpLpmDbImport
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    INOUT GT_U32                                *lpmDbMemBlockSizePtr,
    IN    GT_U32                                *lpmDbMemBlockPtr,
    INOUT GT_UINTPTR                            *iterPtr
)
{
    return ipLpmDbSetL3(lpmDbPtr,lpmDbMemBlockPtr,lpmDbMemBlockSizePtr,iterPtr);
}
#endif

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC  *ppMemEntry,*tmpPtr; /* Pp memory list entry.    */
    GT_STATUS                   retVal = GT_OK;     /* Functions return value.      */
    GT_U32                      blockIndex=0;/* calculated according to the ppMemAddr*/
    GT_U32                      sizeOfMemoryBlockInlines=0;/* calculated according to the memory handle */
    GT_BOOL                     oldUpdateDec=GT_FALSE;/* for reconstruct */
    GT_U32                      oldNumOfDecUpdates=0;/* for reconstruct */
    switch(operation)
    {
    case (PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E):
        /* make sure it an empty list */
        if (*memBlockListPtrPtr != NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

        break;

    case (PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E):
        ppMemEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC));
        if (ppMemEntry == NULL)
        {
            retVal = GT_OUT_OF_CPU_MEM;
        }
        else
        {
            ppMemEntry->memAddr  = ppMemAddr;
            ppMemEntry->next = *memBlockListPtrPtr;
            *memBlockListPtrPtr = ppMemEntry;

        }
        break;

    case (PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E):
        ppMemEntry = *memBlockListPtrPtr;
        while (ppMemEntry != NULL)
        {
            /* create a pending list of blocks that will be used to
               update protocolCountersPerBlockArr */
            if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
            {
                /* the block index updated out of 20 blocks*/
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(ppMemEntry->memAddr)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                sizeOfMemoryBlockInlines = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(ppMemEntry->memAddr);
            }
            else
            {
                /* the block index updated out of 20 blocks*/
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(ppMemEntry->memAddr)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                sizeOfMemoryBlockInlines = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(ppMemEntry->memAddr);
            }

            /* keep values incase reconstruct is needed */
            oldUpdateDec = shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec;
            oldNumOfDecUpdates = shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates;

            /* set pending flag for future need */
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates += sizeOfMemoryBlockInlines;

            if (prvCpssDmmFree(ppMemEntry->memAddr) == 0)
            {
                /* reconstruct */
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec = oldUpdateDec;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates = oldNumOfDecUpdates;

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            tmpPtr = ppMemEntry;
            ppMemEntry = ppMemEntry->next;

            cpssOsLpmFree(tmpPtr);
        }
        *memBlockListPtrPtr = NULL;
        break;
    }

    return retVal;
}

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC *params;

    params = (PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC*)data;

    return prvCpssDxChLpmRamMngVrfEntryUpdate(params->vrId, params->protocol,
                                              params->shadowPtr);
}

/**
* @internal prvCpssDxChLpmRouteEntryPointerCheck function
* @endinternal
*
* @brief   Check validity of values of route entry pointer
*
* @param[in] routeEntryPointerPtr     - route entry pointer to check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChLpmRouteEntryPointerCheck
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT          shadowType,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  *routeEntryPointerPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(routeEntryPointerPtr);

    if ((routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E) &&
        (routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E) &&
        (routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_MULTIPATH_E) &&
        (routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "routeEntryMethod: not supported type ");
    }

    if (shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        if ((routeEntryPointerPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E) ||
            (routeEntryPointerPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "routeEntryMethod : Sip6 not supports types : 'ECMP' , 'QOS' ");
        }
    }
    /* validate the ipv6 MC group scope level */
    switch (routeEntryPointerPtr->ipv6McGroupScopeLevel)
    {
        case CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E:
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "groupScopeLevel : not supported type ");

    }
    if (shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        switch (routeEntryPointerPtr->priority) 
        {
            case PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E:
            case PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "priority : not supported type ");
        }
    }
    /* note that since LPM DB can exist without any devices added to it, there
       is no way to know how many route entries or ECMP/QoS entries are
       available in specific device; therefore neither the route entries base
       address nor the size of the ECMP/QoS block value can't be checked */

    return GT_OK;
}

#if 0
/**
* @internal prvCpssExMxPmLpmDbIdGetNext function
* @endinternal
*
* @brief   This function retrieve next LPM DB ID from LPM DBs Skip List
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in,out] slIteratorPtr            - The iterator Id that was returned from the last call to
*                                      this function.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_MORE               - on absence of elements in skip list
*
* @note none.
*
*/
GT_STATUS prvCpssExMxPmLpmDbIdGetNext
(
    OUT     GT_U32      *lpmDbIdPtr,
    INOUT   GT_UINTPTR  *slIteratorPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_LPM_DB_STC *lpmDbPtr = NULL;
    CPSS_NULL_PTR_CHECK_MAC(slIteratorPtr);
    CPSS_NULL_PTR_CHECK_MAC(lpmDbIdPtr);

    lpmDbPtr = (PRV_CPSS_DXCH_LPM_RAM_LPM_DB_STC *)prvCpssSlGetNext(lpmDbSL,slIteratorPtr);
    if (lpmDbPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
    }
    *lpmDbIdPtr = lpmDbPtr->lpmDBId;
    return GT_OK;
}
#endif

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
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChIpLpmRamUcPrefixActivityStatusGet(
                vrId,
                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                ipPtr,
                prefixLen,
                clearActivity,
                shadowPtr,
                activityStatusPtr);
}

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
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChIpLpmRamUcPrefixActivityStatusGet(
                vrId,
                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                ipPtr,
                prefixLen,
                clearActivity,
                shadowPtr,
                activityStatusPtr);
}

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
* @param[in] ipSrcPtr                 - Pointer to the ip Source address to get the entry for.
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
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChIpLpmRamMcEntryActivityStatusGet(
                vrId,
                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                ipGroupPtr,
                ipGroupPrefixLen,
                ipSrcPtr,
                ipSrcPrefixLen,
                clearActivity,
                shadowPtr,
                activityStatusPtr);
}

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
* @param[in] ipGroupPtr               - Pointer to the ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrcPtr                 - Pointer to the ip Source address to get the entry for.
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
GT_STATUS prvCpssDxChIpLpmRamIpv6McEntryActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipGroupPtr,
    IN  GT_U32                              ipGroupPrefixLen,
    IN  GT_U8                               *ipSrcPtr,
    IN  GT_U32                              ipSrcPrefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChIpLpmRamMcEntryActivityStatusGet(
                vrId,
                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                ipGroupPtr,
                ipGroupPrefixLen,
                ipSrcPtr,
                ipSrcPrefixLen,
                clearActivity,
                shadowPtr,
                activityStatusPtr);
}

