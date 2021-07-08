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
* @file cpssDxChPortDynamicPizzaArbiterWorkConserving.h
*
* @brief CPSS implementation for SerDes configuration and control facility.
*
* @version   9
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_PORT_DYNAMIC_PIZZA_ARBITER_WORK_CONSERVING_H
#define __PRV_CPSS_DXCH_PORT_DYNAMIC_PIZZA_ARBITER_WORK_CONSERVING_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>


/**
* @internal cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet function
* @endinternal
*
* @brief   Configure work conserving mode at one or all units
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] unit                     - RxDMA/TxDMA/TXQ/Tx-FIFO/Eth-Tx-FIFO/Ilkn-Tx-FIFO or CPSS_DXCH_PA_UNIT_UNDEFINED_E (meaning all !!)
* @param[in] status                   - enable (1) disable (0)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet
(
    IN GT_U8                 devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unit,
    IN GT_BOOL               status
);

/**
* @internal cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet function
* @endinternal
*
* @brief   get work conserving mode of one units
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] unit                     - RxDMA/TxDMA/TXQ/Tx-FIFO/Eth-Tx-FIFO/Ilkn-Tx-FIFO
*
* @param[out] statusPtr                - pointer to status.: enable (1) disable (0)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet
(
    IN GT_U8                 devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unit,
    OUT GT_BOOL              *statusPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

