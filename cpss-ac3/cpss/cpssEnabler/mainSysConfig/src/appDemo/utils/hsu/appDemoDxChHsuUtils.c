/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoDxChHsuUtils.c
*
* @brief App demo DxCh HSU API.
*
* @version   8
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hsu/cpssDxChHsu.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hsu/private/prvCpssDxChHsu.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/utils/hsu/appDemoDxChHsuUtils.h>
#include <appDemo/userExit/userEventHandler.h>

#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <appDemo/sysHwConfig/appDemoDb.h>

static GT_U8  *hsuBlockMemPtr;

/**
* @internal appDemoDxChHsuOldImagePreUpdatePreparation function
* @endinternal
*
* @brief   This function perform following steps of HSU process:
*         1. Disable interrupts
*         2. Set systemRecoveryMode
*         3. export of all hsu datatypes
*         4. warm restart
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] isMultipleIteration      - GT_TRUE - multiIteration HSU operation
*                                      GT_FALSE - single iteration HSU operation
* @param[in] origIterationSize        - hsu iteration size
* @param[in] systemRecoveryMode       - hsu mode defines if application want to process
*                                      messages arrived during HSU
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_RESOURCE           - no place to save the request
*/
GT_STATUS appDemoDxChHsuOldImagePreUpdatePreparation
(
  IN GT_BOOL                               isMultipleIteration,
  IN GT_U32                                origIterationSize,
  IN CPSS_SYSTEM_RECOVERY_MODE_STC         systemRecoveryMode
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i;
    GT_U32 iterationSize;
    GT_U32 hsuDataSize;
    GT_U32 tempIterSize;
    CPSS_DXCH_HSU_DATA_TYPE_ENT dataType = CPSS_DXCH_HSU_DATA_TYPE_ALL_E;
    GT_BOOL exportComplete;
    GT_U32 iterationNumber;
    GT_U32 remainedSize = 0;
    CPSS_SYSTEM_RECOVERY_INFO_STC hsuInputInfo;
#ifndef ASIC_SIMULATION
    GT_U32 *hsuPtr;
#endif

    GT_UINTPTR iter = 0;

    /* disable interrupts */
    rc = cpssPpInterruptsDisable(0);
    if (rc != GT_OK)
    {
        return rc;
    }
    hsuInputInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_PREPARATION_STATE_E;
    hsuInputInfo.systemRecoveryMode  = systemRecoveryMode;
    hsuInputInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E;
    rc =  cpssSystemRecoveryStateSet(&hsuInputInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* perform full export */

    /* get the size of all exported data */
    rc =  cpssDxChHsuBlockSizeGet(dataType, &hsuDataSize);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check with maximum size 8M RAM (max HSU memory size in our system)*/
    if (hsuDataSize > _8M)
    {
        return GT_NO_RESOURCE;
    }
    if (isMultipleIteration == GT_FALSE)
    {
        iterationNumber = 1;
        iterationSize = hsuDataSize;
    }
    else
    {
        iterationNumber = hsuDataSize/origIterationSize;
        remainedSize = hsuDataSize%origIterationSize;
        iterationSize  = origIterationSize;
    }

#ifdef ASIC_SIMULATION
    hsuBlockMemPtr = osMalloc(hsuDataSize*sizeof(GT_U8));
    osMemSet(hsuBlockMemPtr, 0, hsuDataSize*sizeof(GT_U8));
#else
    rc = cpssExtDrvHsuMemBaseAddrGet(&hsuPtr);
    hsuBlockMemPtr = (GT_U8*)hsuPtr;
    if (rc != GT_OK)
    {
        return rc;
    }
#endif
    tempIterSize = origIterationSize;
    /* call cpss api function */
    for( i = 0; i < iterationNumber; i++)
    {
        if (i == iterationNumber -1)
        {
            iterationSize += remainedSize;
        }
        rc = cpssDxChHsuExport(dataType,
                                     &iter,
                                     &iterationSize,
                                     hsuBlockMemPtr,
                                     &exportComplete);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (i != iterationNumber -1 )
        {
            hsuBlockMemPtr = (GT_U8*)(hsuBlockMemPtr + (tempIterSize - iterationSize));
            tempIterSize = origIterationSize + iterationSize;
            iterationSize = tempIterSize;
        }
    }

    /* this delay currently solve timing problem. The root cause
       should be revealed */
    osTimerWkAfter(100);
#ifndef ASIC_SIMULATION
    cpssExtDrvHsuWarmRestart();
#endif
    return GT_OK;
}


/**
* @internal appDemoDxChHsuNewImageUpdateAndSynch function
* @endinternal
*
* @brief   This function perform following steps of HSU process:
*         1. import of all hsu datatypes
*         2. setting HSU state in "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E"
*         4. open pp interrupts
*         3. enable interrupts
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] isMultipleIteration      - GT_TRUE - multiIteration HSU operation
*                                      GT_FALSE - single iteration HSU operation
* @param[in] origIterationSize        - hsu iteration size
* @param[in] hsuDataSize              - hsu import data size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_RESOURCE           - no place to save the request
*
* @note When appDemoDxChHsuOldImagePreUpdatePreparation is finished and new image
*       have arisen after warm restart, user should perform
*       cpssSystemRecoveryStateSet(CPSS_SYSTEM_RECOVERY_INIT_STATE_E), cpssInitSystem and only
*       after that call appDemoDxChHsuNewImageUpdateAndSynch.
*
*/
GT_STATUS appDemoDxChHsuNewImageUpdateAndSynch
(
  IN GT_BOOL                               isMultipleIteration,
  IN GT_U32                                origIterationSize,
  IN GT_U32                                hsuDataSize
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i;
    GT_U32 iterationSize;
    GT_U32 tempIterSize;
    CPSS_DXCH_HSU_DATA_TYPE_ENT dataType;
    GT_BOOL importComplete;
    GT_U32 iterationNumber;
    GT_STATUS stat;
    GT_U32 remainedSize = 0;
    GT_U32 value        = 0;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

#ifndef ASIC_SIMULATION
    GT_U32 *hsuPtr;
#endif
    GT_UINTPTR iter = 0;

    dataType = CPSS_DXCH_HSU_DATA_TYPE_ALL_E;

    /* perform full import */

    if (isMultipleIteration == GT_FALSE)
    {
        iterationNumber = 1;
        iterationSize = hsuDataSize;
    }
    else
    {
        iterationNumber = hsuDataSize/origIterationSize;
        remainedSize = hsuDataSize%origIterationSize;
        iterationSize  = origIterationSize;
    }
#ifndef ASIC_SIMULATION
    rc = cpssExtDrvHsuMemBaseAddrGet(&hsuPtr);
    hsuBlockMemPtr = (GT_U8*)hsuPtr;
    if (rc != GT_OK)
    {
        return rc;
    }
#endif
    tempIterSize = origIterationSize;
    /* call cpss api function */
    for( i = 0; i < iterationNumber; i++)
    {
        if (i == iterationNumber -1)
        {
            iterationSize += remainedSize;
        }

        rc = cpssDxChHsuImport(dataType,
                               &iter,
                               &iterationSize,
                               hsuBlockMemPtr,
                               &importComplete);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (i != iterationNumber - 1)
        {
            hsuBlockMemPtr = (GT_U8*)(hsuBlockMemPtr + (tempIterSize - iterationSize));
            tempIterSize = origIterationSize + iterationSize;
            iterationSize = tempIterSize;
        }
    }
    /* set HSU state in "HSU_COMPLETE" */
    cpssOsMemSet(&system_recovery,0,sizeof(system_recovery));
    system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E;
    rc =  cpssSystemRecoveryStateSet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* spawn the user event handler processes and open interrupts */
    stat = appDemoDbEntryGet("initSystemWithoutInterrupts", &value);
    if (stat == GT_NO_SUCH ||(stat == GT_OK && value == 0))
    {
        rc = appDemoEventRequestDrvnModeInit(); 
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}



