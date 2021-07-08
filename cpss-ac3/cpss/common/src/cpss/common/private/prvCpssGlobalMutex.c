/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
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
* @file prvCpssGlobalMutex.c
*
* @brief This file provides lock/unlock with CPSS global mutex
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/prvCpssGlobalMutex.h>

static CPSS_OS_MUTEX prvCpssGlobalMtx = (CPSS_OS_MUTEX)0;
CPSS_OS_MUTEX prvCpssIntrScanMtx = (CPSS_OS_MUTEX)0;

/**
* @internal cpssGlobalMtxInit function
* @endinternal
*
* @brief   Initialize CPSS global mutex object
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssGlobalMtxInit(GT_VOID)
{
    GT_STATUS rc;
    if (prvCpssGlobalMtx != (CPSS_OS_MUTEX)0)
    {
         /* Already initialized */
         return GT_OK;
    }
    rc = cpssOsMutexCreate("prvCpssGlobalMtx",&prvCpssGlobalMtx);
    rc = cpssOsMutexCreate("prvCpssIntScanMtx",&prvCpssIntrScanMtx);
    return rc;
}

/**
* @internal cpssGlobalMtxLock function
* @endinternal
*
* @brief   Lock CPSS global mutex
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       None.
*/
GT_VOID cpssGlobalMtxLock(GT_VOID)
{
    cpssOsMutexLock(prvCpssGlobalMtx);
}

/**
* @internal cpssGlobalMtxUnlock function
* @endinternal
*
* @brief   Unlock CPSS global mutex
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       None.
*/
GT_VOID cpssGlobalMtxUnlock(GT_VOID)
{
    cpssOsMutexUnlock(prvCpssGlobalMtx);
}

/**
* @internal cpssGlobalMtxDelete function
* @endinternal
*
* @brief   Destroy CPSS global mutex object
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssGlobalMtxDelete(GT_VOID)
{
    cpssOsMutexDelete(prvCpssGlobalMtx);
    prvCpssGlobalMtx = 0;

    return GT_OK;
}




