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
* @file cpssDxChPortDynamicPAUnitBW.c
*
* @brief bobcat2 and higher dynamic (algorithmic) pizza arbiter
*
* @version   3
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitDrv.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAPortSpeedDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiterWS.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>


static GT_STATUS prvCpssDxChPortPizzaArbiterSpeedGreaterCheck
(
    IN GT_U8     devNum,
    IN GT_U32    pipeBandwithInGbps,
    IN GT_U32    minSpeedMbps,
    OUT GT_BOOL *isGreaterPtr
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM               portNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_BOOL                            isInitilized;
    GT_U32                             speedInMBit;
    GT_U32                             accBw;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(isGreaterPtr);

    accBw = 0;
    *isGreaterPtr = GT_TRUE;
    for (portNum = 0 ; portNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; portNum++)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E)
        {
            rc = prvCpssDxChPortDynamicPAPortSpeedDBGet(devNum,portNum,/*OUT*/&isInitilized,&speedInMBit);
            if (isInitilized == GT_TRUE)
            {
                if (speedInMBit < minSpeedMbps)
                {
                    *isGreaterPtr = GT_FALSE;
                    cpssOsPrintf("\nport %d speed = %5d Mpbs < resolution (%5d Mbps)",portNum,speedInMBit,minSpeedMbps);
                }
                accBw = accBw + speedInMBit;
            }
        }
    }
    if (accBw > pipeBandwithInGbps*1000)
    {
        cpssOsPrintf("\nport configured BW = %d greater than requested %d",accBw,pipeBandwithInGbps);
        *isGreaterPtr = GT_FALSE;
    }
    return GT_OK;
}

typedef struct
{
    CPSS_DXCH_MIN_SPEED_ENT minSpeedMbpsEnt;
    GT_U32                  minSpeedMbps;
}CPSS_DXCH_MIN_SPEED_CON_STC;
static CPSS_DXCH_MIN_SPEED_CON_STC arr[] =
{
     { CPSS_DXCH_MIN_SPEED_500_Mbps_E   ,    500 }
    ,{ CPSS_DXCH_MIN_SPEED_1000_Mbps_E  ,   1000 }
    ,{ CPSS_DXCH_MIN_SPEED_2000_Mbps_E  ,   2000 }
    ,{ CPSS_DXCH_MIN_SPEED_5000_Mbps_E  ,   5000 }
    ,{ CPSS_DXCH_MIN_SPEED_10000_Mbps_E ,  10000 }
};


GT_U32 prvCpssDxChPortPizzaArbiterSpeedResolutionByEnmConv
(
    CPSS_DXCH_MIN_SPEED_ENT minSpeedMbpsEnt
)
{
    GT_U32 i;
    for (i = 0 ; i < sizeof(arr)/sizeof(arr[0]); i++)
    {
        if (arr[i].minSpeedMbpsEnt == minSpeedMbpsEnt)
        {
            return arr[i].minSpeedMbps;
        }
    }
    return 0;
}

CPSS_DXCH_MIN_SPEED_ENT prvCpssDxChPortPizzaArbiterSpeedResolutionBySpeedConv
(
    GT_U32 minSpeedMbps
)
{
    GT_U32 i;
    for (i = 0 ; i < sizeof(arr)/sizeof(arr[0]); i++)
    {
        if (arr[i].minSpeedMbps == minSpeedMbps)
        {
            return arr[i].minSpeedMbpsEnt;
        }
    }
    return CPSS_DXCH_MIN_SPEED_INVALID_E;
}


/**
* @internal internal_cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet function
* @endinternal
*
* @brief   set pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, Eth-Tx-FIFO and slice resolution for TxQ
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] pipeBandwithInGbps       - pipe bandwidth in GBits
* @param[in] minimalPortSpeedMBps     - min port speed resolution in MBps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, combination of BW and slice resolution
* @retval GT_FAIL                  - on error
*
* @note a. API does not change system BW, it only informs pizza arbiter about requested BW.
*       b. API returns GT_BAD_PARAM, if number of available slices resolution is smaller than requested BW,
*       ex: Bobcat2 has 340 slices, default resolution is 0.5G , max BW that can be supported is 340[slices] 0.5[Gbps/slice] = 170 [Gpbs]
*       ex: if one like to support BW 240Gpbs, than ceil(240[Gpbs]/340[slices]) = 1[Gpbs/slice] = 1000 [Mpbs/slice]
*       c. API checks whether there are ports with speed smaller than resolution. If any is present, API returns with error (GT_FAIL).
*       d. API cpssDxChPortModeSpeedSet() will return error, if requested speed is smaller than speed resolution.
*
*/
static GT_STATUS internal_cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet
(
    IN GT_U8                     devNum,
    IN GT_U32                    pipeBandwithInGbps,
    IN CPSS_DXCH_MIN_SPEED_ENT   minimalPortSpeedMBps
)
{
    GT_STATUS rc;
    CPSS_DXCH_PA_UNIT_ENT unitType;
    GT_U32                    bwEstInGBpsByMinSliceRes;
    GT_BOOL                   isGreater;
    GT_U32                    minPortSpeedMbps;
    GT_U32                    unitIdx;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr;
    GT_U32                    maxSlicesNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    if (pipeBandwithInGbps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    minPortSpeedMbps = prvCpssDxChPortPizzaArbiterSpeedResolutionByEnmConv(minimalPortSpeedMBps);
    if (minPortSpeedMbps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    maxSlicesNum = PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum) ? PRV_CPSS_DXCH_Aldrin_PA_TXQ_MAP_TOTAL_FLDS_REG_CNS :
                                                            PRV_CPSS_DXCH_BC2_PA_MAP_TOTAL_FLDS_REG_CNS;

    bwEstInGBpsByMinSliceRes = maxSlicesNum * minPortSpeedMbps;
    if (bwEstInGBpsByMinSliceRes <  pipeBandwithInGbps*1000)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPizzaArbiterSpeedGreaterCheck(devNum,pipeBandwithInGbps,minPortSpeedMbps,/*OUT*/&isGreater);
    if (GT_OK != rc)
    {
        return rc;
    }
    if (isGreater == GT_FALSE) /* there are ports having speed smaller than minSpeed */
    {
        cpssOsPrintf("\ncpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet() failed");
        cpssOsPrintf("\n    there are ports smaller than resolution or     ");
        cpssOsPrintf("\n    configured BW greater is greater than requested");
        cpssOsPrintf("\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (unitIdx = 0 ; paWsPtr->prv_DeviceUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWsPtr->prv_DeviceUnitListPtr[unitIdx];
        if (paWsPtr->prv_unitListConfigByPipeBwSetBmp[unitType] == GT_TRUE) /* configure !!! */
        {
            rc = prvCpssDxChPortDynamicPizzaArbiterIfUnitBWSet(devNum, unitType, pipeBandwithInGbps);
            if (GT_OK != rc)
            {
                return rc;
            }
        }
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paMinSliceResolutionMbps = minPortSpeedMbps;
    return GT_OK;
}

/**
* @internal cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet function
* @endinternal
*
* @brief   set pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, Eth-Tx-FIFO and slice resolution for TxQ
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] pipeBandwithInGbps       - pipe bandwidth in GBits
* @param[in] minimalPortSpeedMBps     - min port speed resolution in MBps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, combination of BW and slice resolution
* @retval GT_FAIL                  - on error
*
* @note a. API does not change system BW, it only informs pizza arbiter about requested BW.
*       b. API returns GT_BAD_PARAM, if number of available slices resolution is smaller than requested BW,
*       ex: Bobcat2 has 340 slices, default resolution is 0.5G , max BW that can be supported is 340[slices] 0.5[Gbps/slice] = 170 [Gpbs]
*       ex: if one like to support BW 240Gpbs, than ceil(240[Gpbs]/340[slices]) = 1[Gpbs/slice] = 1000 [Mpbs/slice]
*       c. API checks whether there are ports with speed smaller than resolution. If any is present, API returns with error (GT_FAIL).
*       d. API cpssDxChPortModeSpeedSet() will return error, if requested speed is smaller than speed resolution.
*
*/
GT_STATUS cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet
(
    IN GT_U8                     devNum,
    IN GT_U32                    pipeBandwithInGbps,
    IN CPSS_DXCH_MIN_SPEED_ENT   minimalPortSpeedMBps
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pipeBandwithInGbps, minimalPortSpeedMBps));

    rc = internal_cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(devNum, pipeBandwithInGbps, minimalPortSpeedMBps);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pipeBandwithInGbps, minimalPortSpeedMBps));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet function
* @endinternal
*
* @brief   get pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, Eth-Tx-FIFO and slice resolution for TxQ
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] pipeBandwithInGbpsPtr    - pipe bandwidth in GBits
*                                      minimalSliceResolutionInMBpsPtr - min port speed resolution in MBps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - wrong devNum, combination of BW and slice resolution
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet
(
    IN  GT_U8                     devNum,
    OUT GT_U32                   *pipeBandwithInGbpsPtr,
    OUT CPSS_DXCH_MIN_SPEED_ENT  *minimalPortSpeedResolutionInMBpsPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC    *paWsPtr;
    CPSS_DXCH_PA_UNIT_ENT unitType;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (pipeBandwithInGbpsPtr == NULL && minimalPortSpeedResolutionInMBpsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if (pipeBandwithInGbpsPtr != NULL)
    {
        rc = prvCpssDxChPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        unitType = paWsPtr->prv_unitListConfigByPipeBwSetPtr[0];
        rc = prvCpssDxChPortDynamicPizzaArbiterIfUnitBWGet(devNum, unitType, /*OUT*/pipeBandwithInGbpsPtr);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    if (minimalPortSpeedResolutionInMBpsPtr != NULL)
    {
        *minimalPortSpeedResolutionInMBpsPtr = prvCpssDxChPortPizzaArbiterSpeedResolutionBySpeedConv(PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paMinSliceResolutionMbps);
    }
    return GT_OK;
}

/**
* @internal cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet function
* @endinternal
*
* @brief   get pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, Eth-Tx-FIFO and slice resolution for TxQ
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] pipeBandwithInGbpsPtr    - pipe bandwidth in GBits
*                                      minimalSliceResolutionInMBpsPtr - min port speed resolution in MBps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - wrong devNum, combination of BW and slice resolution
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet
(
    IN  GT_U8                     devNum,
    OUT GT_U32                   *pipeBandwithInGbpsPtr,
    OUT CPSS_DXCH_MIN_SPEED_ENT  *minimalPortSpeedResolutionInMBpsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pipeBandwithInGbpsPtr, minimalPortSpeedResolutionInMBpsPtr));

    rc = internal_cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet(devNum, pipeBandwithInGbpsPtr, minimalPortSpeedResolutionInMBpsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pipeBandwithInGbpsPtr, minimalPortSpeedResolutionInMBpsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal intertal_cpssDxChPortPizzaArbiterBWModeSet function
* @endinternal
*
* @brief   Set Bandwidth mode of TX_FIFO Unit Pizza Arbiter Channel for specified Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*                                       Remote ports and SDMAs port not supported.
* @param[in] portMode                 - Pizza Arbiter Bandwidth port mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter or not enough Unit BW for ports summary BW
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPortPizzaArbiterBWModeSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_PA_BW_MODE_ENT       portMode
)
{
    GT_STATUS                         rc;                                                                  \
    GT_U32                            speedFactor;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
        | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
        | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);

    rc = prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorGet(
        portMode, &speedFactor);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChPortPizzaArbiterTxFifoChannelSpeedFactorSet(
        devNum, portNum, speedFactor);
}

/**
* @internal cpssDxChPortPizzaArbiterBWModeSet function
* @endinternal
*
* @brief   Set Bandwidth mode of TX_FIFO Unit Pizza Arbiter Channel for specified Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*                                       Remote ports and SDMAs port not supported.
* @param[in] portMode                 - Pizza Arbiter Bandwidth port mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter or not enough Unit BW for ports summary BW
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPizzaArbiterBWModeSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_PA_BW_MODE_ENT       portMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPizzaArbiterBWModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portMode));

    rc = internal_cpssDxChPortPizzaArbiterBWModeSet(
        devNum, portNum, portMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPizzaArbiterBWModeGet function
* @endinternal
*
* @brief   Get Bandwidth mode of TX_FIFO Unit Pizza Arbiter Channel for specified Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*                                       Remote ports and SDMAs port not supported.
* @param[out] portModePtr             - (pointrer to)Pizza Arbiter Bandwidth port mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter or not enough Unit BW for ports summary BW
* @retval GT_BAD_PTR               - on null-pointer parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPortPizzaArbiterBWModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_PA_BW_MODE_ENT       *portModePtr
)
{
    GT_STATUS                         rc;                                                                  \
    GT_U32                            speedFactor;
    GT_U32                            speedFactorPattern;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
        | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
        | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(portModePtr);

    rc = prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBGet(
        devNum, portNum, &speedFactor);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    for (*portModePtr = 0; (*portModePtr < CPSS_PORT_PA_BW_MODE_LAST_E); (*portModePtr)++)
    {
        rc = prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorGet(
            *portModePtr, &speedFactorPattern);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (speedFactorPattern == speedFactor)
        {
            /* speedFactor value found */
            return GT_OK;
        }
    }

    /* speedFactor value not found */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChPortPizzaArbiterBWModeGet function
* @endinternal
*
* @brief   Get Bandwidth mode of TX_FIFO Unit Pizza Arbiter Channel for specified Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*                                       Remote ports and SDMAs port not supported.
* @param[out] portModePtr             - (pointrer to)Pizza Arbiter Bandwidth port mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter or not enough Unit BW for ports summary BW
* @retval GT_BAD_PTR               - on null-pointer parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPizzaArbiterBWModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_PA_BW_MODE_ENT       *portModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPizzaArbiterBWModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portModePtr));

    rc = internal_cpssDxChPortPizzaArbiterBWModeGet(
        devNum, portNum, portModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


