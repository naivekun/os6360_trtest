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
* @file prvTgfIpv4UcRoutingAddManyIpLpmMng.h
*
* @brief IPV4 UC Routing when filling the Lpm using cpssDxChIpLpmIpv4UcPrefixAddManyByOctet.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfpv4UcRoutingAddManyIpLpmMngh
#define __prvTgfpv4UcRoutingAddManyIpLpmMngh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <common/tgfIpGen.h>
/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

#define PRV_TGF_IP_ADD_MANY_DEFAULT_PBR_SIZE_CNS _8K
#define PRV_TGF_IP_ADD_MANY_DEFAULT_LPM_RAM_MEMORY_MODE PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E
#define PRV_TGF_IP_ADD_MANY_DEFAULT_LPM_RAM_BLOCKS_ALLOCATION_METHOD  PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E


/**
* @internal prvTgfIpLpmMngIpv4UcRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpLpmMngIpv4UcRoutingAddManyConfigurationAndTrafficGenerate(GT_VOID);

/**
* @internal prvTgfIpLpmMngIpv4UcRoutingAddManyConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpLpmMngIpv4UcRoutingAddManyConfigurationRestore(GT_VOID);

/**
* @internal prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig function
* @endinternal
*
* @brief   This function cleans CPSS appDemo default configurations -
*         LPM DB id 0 with VR id 0
*         changes the LPM RAM blocks allocation method to DYNAMIC
*         then restores the CPSS appDemo default configurations
* @param[in] blocksAllocationMethod   - the method of blocks allocation
*                                       GT_OK on success, or
*
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_VOID prvTgfIpLpmMngRoutingAddManyConfigurationChangeLpmDbConfig
(
    IN PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT  blocksAllocationMethod
);

/**
* @internal prvTgfIpLpmMngRoutingAddManyRestoreToDefault function
* @endinternal
*
* @brief   This function restores the CPSS appDemo to default configurations.
*
* @param[in] currentBlockAllocationMethod - the method of blocks allocation currently used at the system
* @param[in] currentLpmMemoryMode     - current lpm memory mode that is used at the system
* @param[in] currentPbrSize           - current size of PBR that is used at the system
*                                       None
*
* @note In case PBR size or LPM memory mode is different then default ,then system reset is triggered.
*       In case only the method of blocks allocationis different then default , then no system reset is triggered
*
*/

GT_VOID prvTgfIpLpmMngRoutingAddManyRestoreToDefault
(
    IN PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT    currentBlockAllocationMethod,
    IN PRV_TGF_LPM_RAM_MEM_MODE_ENT                    currentLpmMemoryMode,
    IN GT_U32                                          currentPbrSize
);


/**
* @internal prvTgfIpLpmMngRoutingAddManyPrepareIteration function
* @endinternal
*
* @brief   This function configure the CPSS appDemo to configurations required by the current test step.
*
* @param[in,out] currentBlockAllocationMethodPtr - the method of blocks allocation currently used at the system
*                                      currentLpmMemoryModePrt - current lpm memory mode that is used at the system
* @param[in,out] currentPbrSizePtr        - current size of PBR that is used at the system
* @param[in,out] currentBlockAllocationMethodPtr - the method of blocks allocation currently used at the system
*                                      currentLpmMemoryModePrt - current lpm memory mode that is used at the system
* @param[in,out] currentPbrSizePtr        - current size of PBR that is used at the system
*
* @param[out] newBlockAllocationMethod - the method of blocks allocation required by the current test step
* @param[out] newLpmMemoryMode         - the lpm memory mode required by the current test step
* @param[out] newPbrSize               - the size of PBR required by the current test step
* @param[out] iterationValidPtr        - GT_FALSE if current configuration is not supported by CPSS, GT_TRUE otherwise
*                                       None
*/

GT_VOID prvTgfIpLpmMngRoutingAddManyPrepareIteration
(
    INOUT PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT    *currentBlockAllocationMethodPtr,
    INOUT PRV_TGF_LPM_RAM_MEM_MODE_ENT                    *currentLpmMemoryModePtr,
    INOUT GT_U32                                          *currentPbrSizePtr,
    IN PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT       newBlockAllocationMethod,
    IN PRV_TGF_LPM_RAM_MEM_MODE_ENT                       newLpmMemoryMode,
    IN GT_U32                                             newPbrSize,
    OUT GT_BOOL                                           *iterationValidPtr
);



#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __prvTgfpv4UcRoutingAddManyIpLpmMngh */


