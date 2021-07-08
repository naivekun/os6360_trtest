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
* @file prvCpssDxChPortDynamicPAUnitBW.c
*
* @brief bobcat2 and higher dynamic (algorithmic) pizza arbiter
*
* @version   4
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAPortSpeedDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiterWS.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>

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
)
{
    GT_U32 * unitBWPtr;
    GT_32   unitN;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);
    unitN = sizeof(PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr)/sizeof(PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr[0]);
    if (0 <= unit && unit < unitN )
    {
        unitBWPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr[0];
        unitBWPtr[unit] = bwInGBit;
        return GT_OK;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}


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
)
{
    GT_U32 * unitBWPtr;
    GT_32 unitN;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(bwInGBitPtr);

    unitN = sizeof(PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr)/sizeof(PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr[0]);
    if (0 <= unit && unit < unitN)
    {
        unitBWPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr[0];
        * bwInGBitPtr = unitBWPtr[unit];
        return GT_OK;
    }
    else
    {
        * bwInGBitPtr = 0;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}




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
)
{
    GT_STATUS rc;
    GT_U32 unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_MAX_E];
    CPSS_DXCH_PA_UNIT_ENT unitType;
    GT_U32                unitIdx;
    GT_U32 unitBwInGbps;
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC *paWsPtr;
    PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC *devBwPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* copy pipe BW capacity from fine tuning params */
    unitBwInGbps = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.pipeBWCapacityInGbps;

    if (unitBwInGbps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    /* Init BW */
    for (unitType = (CPSS_DXCH_PA_UNIT_ENT)0; unitType < CPSS_DXCH_PA_UNIT_MAX_E; unitType++)
    {
        unitBwInGbpsArr[unitType] = 0;
    }
    /*--------------------------------------*
     * fill units controlled by pipeBWSet() *
     *--------------------------------------*/
    for (unitIdx = 0 ; paWsPtr->prv_unitListConfigByPipeBwSetPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWsPtr->prv_unitListConfigByPipeBwSetPtr[unitIdx];
        unitBwInGbpsArr[unitType] = unitBwInGbps;
    }
    /*----------------------------------------------------*
     * fill units that are not contorlled by  pipeBWSet() *
     *----------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPizzaArbiterWSdevBwGet(devNum,/*OUT*/&devBwPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortDynamicPizzaArbiterWSdevBwGet() : ");
    }
    if (devBwPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                                            "prvCpssDxChPortDynamicPizzaArbiterWSdevBwGet() : BW definition not found : rev = %d coreClock = %d",
                                               PRV_CPSS_PP_MAC(devNum)->revision,
                                               PRV_CPSS_PP_MAC(devNum)->coreClock);
    }


    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    switch (pDev->genInfo.devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            switch (pDev->genInfo.devSubFamily)
            {
                case CPSS_PP_SUB_FAMILY_NONE_E:
                break;
                case CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E:
#ifndef GM_USED
                        switch (pDev->genInfo.devType)
                        {
                            case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_RXDMA_1_E      ] = devBwPtr->coreBWGbps[1];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TXDMA_1_E      ] = devBwPtr->coreBWGbps[1];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TX_FIFO_1_E    ] = devBwPtr->coreBWGbps[1];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E] = devBwPtr->coreBWGbps[1];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_MPPM_E         ] = (GT_U32)(~0);
                            break;
                            case CPSS_BOBK_CAELUM_DEVICES_CASES_MAC:
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_RXDMA_0_E      ] = devBwPtr->coreBWGbps[0];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TXDMA_0_E      ] = devBwPtr->coreBWGbps[0];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TX_FIFO_0_E    ] = devBwPtr->coreBWGbps[0];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E] = devBwPtr->coreBWGbps[0];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_RXDMA_1_E      ] = devBwPtr->coreBWGbps[1];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TXDMA_1_E      ] = devBwPtr->coreBWGbps[1];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TX_FIFO_1_E    ] = devBwPtr->coreBWGbps[1];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E] = devBwPtr->coreBWGbps[1];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E   ] = devBwPtr->coreBWGbps[0] + devBwPtr->coreBWGbps[1];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E   ] = devBwPtr->coreBWGbps[0] + devBwPtr->coreBWGbps[1];
                                unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_MPPM_E         ] = (GT_U32)(~0);
                            break;
                            default:
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                            }
                        }
#endif
                break;
                default:
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
            }
        break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        {
            typedef struct
            {
                CPSS_DXCH_PA_UNIT_ENT rxDma [6];
                CPSS_DXCH_PA_UNIT_ENT txDma [6];
                CPSS_DXCH_PA_UNIT_ENT txFifo[6];
                CPSS_DXCH_PA_UNIT_ENT txQ   [6];
            }DP_2_DMA_UNIT_STC;

            static DP_2_DMA_UNIT_STC dp2dmaUnit=
            {
                 {  CPSS_DXCH_PA_UNIT_RXDMA_0_E,   CPSS_DXCH_PA_UNIT_RXDMA_1_E,    CPSS_DXCH_PA_UNIT_RXDMA_2_E,   CPSS_DXCH_PA_UNIT_RXDMA_3_E,   CPSS_DXCH_PA_UNIT_RXDMA_4_E,    CPSS_DXCH_PA_UNIT_RXDMA_5_E    }
                ,{  CPSS_DXCH_PA_UNIT_TXDMA_0_E,   CPSS_DXCH_PA_UNIT_TXDMA_1_E,    CPSS_DXCH_PA_UNIT_TXDMA_2_E,   CPSS_DXCH_PA_UNIT_TXDMA_3_E,   CPSS_DXCH_PA_UNIT_TXDMA_4_E,    CPSS_DXCH_PA_UNIT_TXDMA_5_E    }
                ,{  CPSS_DXCH_PA_UNIT_TX_FIFO_0_E, CPSS_DXCH_PA_UNIT_TX_FIFO_1_E,  CPSS_DXCH_PA_UNIT_TX_FIFO_2_E, CPSS_DXCH_PA_UNIT_TX_FIFO_3_E, CPSS_DXCH_PA_UNIT_TX_FIFO_4_E,  CPSS_DXCH_PA_UNIT_TX_FIFO_5_E  }
                ,{  CPSS_DXCH_PA_UNIT_TXQ_0_E,     CPSS_DXCH_PA_UNIT_TXQ_1_E,      CPSS_DXCH_PA_UNIT_TXQ_2_E,     CPSS_DXCH_PA_UNIT_TXQ_3_E,     CPSS_DXCH_PA_UNIT_TXQ_4_E,      CPSS_DXCH_PA_UNIT_TXQ_5_E      }
            };
            GT_U32 i;
            GT_U32  iMax = (pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ? 6 :
                            4;/*Aldrin2 */
            if(pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                /* currently Falcon as BC3 */
                CPSS_TBD_BOOKMARK_FALCON
                iMax = 6;
            }

            for (i  = 0 ; i < iMax; i++)
            {
                unitBwInGbpsArr[dp2dmaUnit.rxDma [i]] = devBwPtr->coreBWGbps[i];
                unitBwInGbpsArr[dp2dmaUnit.txDma [i]] = devBwPtr->coreBWGbps[i];
                unitBwInGbpsArr[dp2dmaUnit.txFifo[i]] = devBwPtr->coreBWGbps[i];
                unitBwInGbpsArr[dp2dmaUnit.txQ   [i]] = devBwPtr->txqDqBWGbps[i];
            }
        }
        break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_RXDMA_0_E   ] = devBwPtr->coreBWGbps[0];
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TXDMA_0_E   ] = devBwPtr->coreBWGbps[0];
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TX_FIFO_0_E ] = devBwPtr->coreBWGbps[0];
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_RXDMA_1_E   ] = devBwPtr->coreBWGbps[1];
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TXDMA_1_E   ] = devBwPtr->coreBWGbps[1];
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TX_FIFO_1_E ] = devBwPtr->coreBWGbps[1];
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_RXDMA_2_E   ] = devBwPtr->coreBWGbps[2];
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TXDMA_2_E   ] = devBwPtr->coreBWGbps[2];
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TX_FIFO_2_E ] = devBwPtr->coreBWGbps[2];
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TXQ_0_E     ] = devBwPtr->txqDqBWGbps[0];
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E] = devBwPtr->coreBWGbps[0] + devBwPtr->coreBWGbps[1] + devBwPtr->coreBWGbps[2];
            unitBwInGbpsArr[CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E] = devBwPtr->coreBWGbps[0] + devBwPtr->coreBWGbps[1] + devBwPtr->coreBWGbps[2];
        break;
        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        break;
    }

    for (unitType = (CPSS_DXCH_PA_UNIT_ENT)0; unitType < CPSS_DXCH_PA_UNIT_MAX_E; unitType++)
    {
        rc = prvCpssDxChPortDynamicPizzaArbiterIfUnitBWSet(devNum, unitType, unitBwInGbpsArr[unitType]);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paMinSliceResolutionMbps = 1000;  /* 1G */
    return GT_OK;
}

static GT_U32 prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorArr[
    CPSS_PORT_PA_BW_MODE_LAST_E] =
{
     /*CPSS_PORT_PA_BW_MODE_REGULAR_E*/            100
    ,/*CPSS_PORT_PA_BW_EXT_MODE_1_E*/              147
    ,/*CPSS_PORT_PA_BW_EXT_MODE_2_E*/              108
    ,/*CPSS_PORT_PA_BW_EXT_MODE_3_E*/               83
    ,/*CPSS_PORT_PA_BW_EXT_MODE_4_E*/                1
    ,/*CPSS_PORT_PA_BW_EXT_MODE_5_E*/       0xFFFFFFFF
    ,/*CPSS_PORT_PA_BW_EXT_MODE_6_E*/       0xFFFFFFFF
    ,/*CPSS_PORT_PA_BW_EXT_MODE_7_E*/       0xFFFFFFFF
    ,/*CPSS_PORT_PA_BW_EXT_MODE_8_E*/       0xFFFFFFFF
};

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
)
{
    if (txFifoPortMode >= CPSS_PORT_PA_BW_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorArr[txFifoPortMode] = factorInPercent;
    return GT_OK;
}

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
)
{
    if (txFifoPortMode >= CPSS_PORT_PA_BW_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(factorInPercentPtr);

    *factorInPercentPtr = prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorArr[txFifoPortMode];
    return GT_OK;
}

