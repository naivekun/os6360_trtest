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
* @file prvCpssDxChHwInitGlobal2localDma.h
*
* @brief Global ==> local DMA / dpIndex conversion
* structures.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChHwInitGlobal2localDma_H
#define __prvCpssDxChHwInitGlobal2localDma_H

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/port/cpssPortCtrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
* @internal prvCpssDxChPpResourcesTxqGlobal2LocalConvert function
* @endinternal
*
* @brief   convert the global TXQ number in device to txq-dq index and local txq number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - The PP's device number.
*                                      globalDmaNum        - the DMA global number.
*
* @param[out] txqDqIndexPtr            - (pointer to) the Data Path (DP) Index
* @param[out] localTxqNumPtr           - (pointer to) the DMA local number
*                                       GT_OK on success
*
* @note if both dpIndexPtr & localDmaNumPtr are NULL ---> error,
*       otherwise caller is interesting just in one parameter
*
*/
GT_STATUS prvCpssDxChPpResourcesTxqGlobal2LocalConvert
(
    IN  GT_U8    devNum,
    IN  GT_U32   globalTxqNum,
    OUT GT_U32  *txqDqIndexPtr,
    OUT GT_U32  *localTxqNumPtr
);


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif


