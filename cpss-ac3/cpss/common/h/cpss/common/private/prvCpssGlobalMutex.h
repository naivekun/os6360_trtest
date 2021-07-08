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
* @file prvCpssGlobalMutex.h
*
* @brief This file provides lock/unlock with CPSS global mutex
*
* @version   3
********************************************************************************
*/

#ifndef __prvCpssGlobalMutexh
#define __prvCpssGlobalMutexh

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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
GT_STATUS cpssGlobalMtxInit(GT_VOID);

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
GT_VOID cpssGlobalMtxLock(GT_VOID);

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
GT_VOID cpssGlobalMtxUnlock(GT_VOID);

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
GT_STATUS cpssGlobalMtxDelete(GT_VOID);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGlobalMutexh */

