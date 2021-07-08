/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChHwInit.c
*
* DESCRIPTION:
*       Includes CPSS DXCH level basic Hw initialization functions.
*
* FILE REVISION NUMBER:
*       $Revision: 1$
*
*******************************************************************************/
/* use dxCh_prvCpssPpConfig[dev]  : for debugging MACRO PRV_CPSS_DXCH_PP_MAC(dev)*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitGlobal2localDma.h>

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
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    /* if both zero ---> error, otherwise caller is interesting just in one parameter */
    if (txqDqIndexPtr == NULL && localTxqNumPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if (globalTxqNum != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
        {
            /* convert global to dq unit index */
            if (txqDqIndexPtr != NULL)
            {
                *txqDqIndexPtr = SIP_5_20_GLOBAL_TXQ_DQ_PORT_TO_DQ_UNIT_INDEX_MAC(devNum,globalTxqNum);
            }
            /* convert global to local */
            if (localTxqNumPtr != NULL)
            {
                *localTxqNumPtr  = SIP_5_20_GLOBAL_TXQ_DQ_PORT_TO_LOCAL_TXQ_DQ_PORT_MAC(devNum,globalTxqNum);
            }
        }
        else
        {
            if (txqDqIndexPtr != NULL)
            {
                *txqDqIndexPtr = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
            }
            /* convert global to local */
            if (localTxqNumPtr != NULL)
            {
                *localTxqNumPtr  = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
            }
        }
    }
    else
    {
        /* convert global to dq unit index */
        if (txqDqIndexPtr != NULL)
        {
            *txqDqIndexPtr = 0;
        }
        /* convert global to local */
        if (localTxqNumPtr != NULL)
        {
            *localTxqNumPtr  = globalTxqNum;
        }
    }
    return GT_OK;
}







