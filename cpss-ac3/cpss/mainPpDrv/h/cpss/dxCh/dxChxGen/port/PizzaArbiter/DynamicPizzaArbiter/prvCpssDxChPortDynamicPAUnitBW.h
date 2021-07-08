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
* @file prvCpssDxChPortDynamicPAUnitBW.h
*
* @brief bobcat2 and higher dynamic (algorithmic) pizza arbiter
*
* @version   3
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_PORT_DYNAMIC_PA_UNIT_BW_H
#define __PRV_CPSS_DXCH_PORT_DYNAMIC_PA_UNIT_BW_H

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPATypeDef.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPAUnitBW.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfUnitBWSet function
* @endinternal
*
* @brief   set pipe BW for specific unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] unit                     -  type
* @param[in] bwInGBit                 - bandwidth in GBits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note I forced to place this function here, because it needs number of port
*       group where CPU port is connected and there is just no more suitable
*       place.
*
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfUnitBWSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PA_UNIT_ENT     unit,
    IN GT_U32                    bwInGBit
);


/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfUnitBWGet function
* @endinternal
*
* @brief   Get pipe BW for specific unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] unit                     -  type
*
* @param[out] bwInGBitPtr              - bandwidth in GBits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on wrong ptr
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfUnitBWGet
(
    IN  GT_U8                     devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT     unit,
    OUT GT_U32                   *bwInGBitPtr
);
/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfUnitBWInit function
* @endinternal
*
* @brief   Inti pipe BW for all units depending on device id
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfUnitBWInit
(
    GT_U8 devNum
);

/**
* @internal prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorSet function
* @endinternal
*
* @brief   Set TX_FIFO Pizza Arbiter Channel Bandwidth Mode Factors.
*          This function sets global value and inpacts only configurations made later.
*          Function dedicated to debugging purposes.
*
* @param[in] txFifoPortMode           - mode of TX_FIFO Pizza Arbiter Channel
* @param[in] factorInPercent          - factor in percent relative to port speed bandwidth allocation.
*                                       Regular port speed allocation is considered 100%
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorSet
(
    IN  CPSS_PORT_PA_BW_MODE_ENT     txFifoPortMode,
    IN  GT_U32                       factorInPercent
);

/**
* @internal prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorGet function
* @endinternal
*
* @brief   Get TX_FIFO Pizza Arbiter Channel Bandwidth Mode Factors.
*
* @param[in]  txFifoPortMode           - mode of TX_FIFO Pizza Arbiter Channel
* @param[out] factorInPercentPtr       - (pointer to)factor in percent to port speed bandwidth allocation.
*                                        Regular port speed allocation is considered 100%
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorGet
(
    IN  CPSS_PORT_PA_BW_MODE_ENT     txFifoPortMode,
    OUT GT_U32                       *factorInPercentPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif


