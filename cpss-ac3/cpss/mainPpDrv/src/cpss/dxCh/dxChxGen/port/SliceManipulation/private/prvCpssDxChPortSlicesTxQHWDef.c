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
* @file prvCpssDxChPortSlicesTxQHWDef.c
*
* @brief TxQ Unit Slices HW definion
*
* @version   5
********************************************************************************
*/
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesTxQHWDef.h>

/**
* @internal prvLion2PortPizzaArbiterTxQDrvSlicesNumSet function
* @endinternal
*
* @brief   The function sets slice number used in DRV of TxQ unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] slicesNum2Configure      - slices number to be set in pizza arbiter
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_OUT_OF_RANGE          - number of slice to be set is greater than available
*/
GT_STATUS prvLion2PortPizzaArbiterTxQDrvSlicesNumSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 slicesNum2Configure
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      regAddr;            /* register's address */
    GT_U32      fieldLen;           /* register's field length */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (slicesNum2Configure > TxQ_TOTAL_AVAIBLE_SLICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.scheduler.portArbiter.pizzaArbiterConfig;
    fieldLen = 8;

    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                            0, fieldLen, slicesNum2Configure);
    if (GT_OK != rc)
    {
        return rc;
    }
    return GT_OK;

}

/**
* @internal prvLion2PortPizzaArbiterTxQDrvSlicesNumGet function
* @endinternal
*
* @brief   The function get slice number used in Pizza Arbiter and strict priority state bit
*         for BM unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
*
* @param[out] slicesNumPtr             - place to store slices number used in pizza arbiter
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_BAD_PTR               - bad slicesNumPtr, enableStrictPriority4CPUPtr
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterTxQDrvSlicesNumGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    OUT GT_U32  *slicesNumPtr
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      regAddr;            /* register's address */
    GT_U32      txQSlicesNum;
    GT_U32      fieldLen;           /* register's field length */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (NULL == slicesNumPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.scheduler.portArbiter.pizzaArbiterConfig;
    fieldLen = 8;

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                            0, fieldLen, &txQSlicesNum);
    if (GT_OK != rc)
    {
        return rc;
    }
    *slicesNumPtr =  txQSlicesNum;
    return GT_OK;
}



/**
* @internal prvLion2PortPizzaArbiterTxQDrvSliceStateSet function
* @endinternal
*
* @brief   The function occupy the slice (i.e. set the spicific slice
*         be asigned to specific port for BM unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceId                  - slice to be set
* @param[in] isEnabled                - whether the slice shall be enebled or diabled
* @param[in] portId                   - local port id (inside portGroup) to which slice is assigned
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceId or portId are greater than available
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterTxQDrvSliceStateSet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroupId,
    IN GT_U32  sliceId,
    IN GT_BOOL              isEnabled,
    IN GT_PHYSICAL_PORT_NUM portId
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 fieldData;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (sliceId >= TxQ_TOTAL_AVAIBLE_SLICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* field is 4 bit length */
    if (portId >= PRV_CPSS_GEN_PP_MAX_PORTS_IN_PORT_GROUP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.scheduler.
                                            portArbiter.pizzaArbiterMap[sliceId/6];

    fieldData = portId; /* map current slot to this port */
    if (GT_TRUE == isEnabled)
    {
        fieldData |= TxQ_SLICE_ENABLE_BIT_CNS; /* (1<<4);   *//* enable slot mapping by arbiter */
    }
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                            (sliceId%6)*5, 5, fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


/**
* @internal prvLion2PortPizzaArbiterTxQDrvSliceStateGet function
* @endinternal
*
* @brief   The function get the slice state(i.e. set the spicific slice
*         be asigned to specific port for TxQ unit and whether it enabled
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceId                  - slice to be occupied
*                                      OUTPUTS:
* @param[in] isEnabledPtr             - place to store whether slice is occupied
* @param[in] portIdPtr                - plavce to store to which port slice is assigned
*
* @param[out] isEnabledPtr             - place to store whether slice is occupied
* @param[out] portIdPtr                - plavce to store to which port slice is assigned
*                                      OUTPUTS:
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceId or portId are greater than available
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterTxQDrvSliceStateGet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroupId,
    IN GT_U32  sliceId,
    OUT GT_BOOL              *isEnabledPtr,
    OUT GT_PHYSICAL_PORT_NUM *portIdPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 fieldData;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (NULL == isEnabledPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if (NULL == portIdPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    *isEnabledPtr = GT_FALSE;
    *portIdPtr    = 0xFF;

    if (sliceId >= TxQ_TOTAL_AVAIBLE_SLICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.scheduler.
                                            portArbiter.pizzaArbiterMap[sliceId/6];

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                            (sliceId%6)*5, 5, &fieldData);
    if(rc != GT_OK)
    {
        return rc;
    }
    *isEnabledPtr = (GT_BOOL)((fieldData & TxQ_SLICE_ENABLE_BIT_CNS) == TxQ_SLICE_ENABLE_BIT_CNS);
    *portIdPtr    = fieldData  & TxQ_SLICE_PORT_BITS_CNS;
    return GT_OK;
}


/**
* @internal prvLion2PortPizzaArbiterTxQDrvInit function
* @endinternal
*
* @brief   Init BM unit (sets number of slice to use , strict priority to CPU bit
*         and disables all slices of the device.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceNum2Init            - number of slices to be used
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceNum2Init is greater than available
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterTxQDrvInit
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 sliceNum2Init
)
{
    GT_STATUS rc;
    GT_U32    sliceId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    rc = prvLion2PortPizzaArbiterTxQDrvSlicesNumSet(/*IN*/devNum,/*IN*/portGroupId,
                                                    /*IN*/sliceNum2Init);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(sliceId = 0; sliceId < TxQ_TOTAL_AVAIBLE_SLICES_CNS; sliceId++)
    {
        rc = prvLion2PortPizzaArbiterTxQDrvSliceStateSet(devNum,portGroupId,sliceId,GT_FALSE,0);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

