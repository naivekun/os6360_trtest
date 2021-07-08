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
* @file cpssDxChPortBufMg.c
*
* @brief CPSS implementation for Port configuration and control facility -DXCH
*
* @version   45
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortBufMg.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>

/* check the flow control profile set range */
#define PRV_CPSS_DXCH_FC_PROFILE_CHECK_MAC(_devNum,profileSet) \
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)                \
    {                                                                                   \
        if ((GT_U32)(profileSet) > (GT_U32)CPSS_PORT_RX_FC_PROFILE_8_E)                 \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                        \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        if ((GT_U32)(profileSet) > (GT_U32)CPSS_PORT_RX_FC_PROFILE_4_E)                 \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                        \
    }


/* check the global Xon/Xoff flow control limit value range */
#define PRV_CPSS_DXCH_GLOBAL_XON_XOFF_CHECK_MAC(_devNum,xonXoff)                        \
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)                \
    {                                                                                   \
        if (xonXoff >= BIT_16)                                                          \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                        \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        if (xonXoff >= BIT_11)                                                          \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                        \
    }

/* check the rx buffer limit value range */
#define PRV_CPSS_DXCH_RX_BUFFER_LIMIT_CHECK_MAC(_devNum,rxBufLimit)                     \
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)                \
    {                                                                                   \
        if (rxBufLimit >= BIT_16)                                                       \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                        \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        if (rxBufLimit >= BIT_10)                                                       \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                        \
    }

/* check the FIFO type range */
#define PRV_CPSS_DXCH_FIFO_TYPE_CHECK_MAC(fifoType) \
        if ((GT_U32)(fifoType) > (GT_U32)CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_CPU_E) \
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/**
* @internal internal_cpssDxChGlobalXonLimitSet function
* @endinternal
*
* @brief   Sets the Global Xon limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xon limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xonLimit), all ports that support 802.3x Flow Control send an Xon
*         frame.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] xonLimit                 - X-ON limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or xonLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The Global Xon limit must be not be greater than Global Xoff Limit.
*       2. To disable Global Xon, the 2xonLimit should be set greater-or-equal
*       of 8<Max Buffer Limit> (maximal number of buffers allocated for
*       all ports).
*       3. Since in CH3 and above the HW resolution is 8,
*       in case the xonLimit input parameter is not a multiple of 8 we
*       round it UP to the nearest one.
*
*/
static GT_STATUS internal_cpssDxChGlobalXonLimitSet
(
    IN GT_U8    devNum,
    IN GT_U32   xonLimit
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      fieldOffset; /* field offset     */
    GT_U32      fieldLength; /* field length     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_BM_MAC(devNum).BMGlobalConfigs.BMPauseConfig;
        fieldOffset = 0;
        fieldLength = 16;
        xonLimit = xonLimit*2;
    }
    else
    {
        /* For DXCH and DXCH2 devices the field resolution is of 2 buffers */
        /* For DXCH3 and above, the field resolution is of 8 buffers,*/
        /* which we are rounding up */
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            >= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            if( xonLimit != 0 )
            {
                xonLimit = ((xonLimit - 1) >> 2) + 1;
            }
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngGlobalLimit;
        fieldOffset = 0;
        fieldLength = 11;
    }

    PRV_CPSS_DXCH_GLOBAL_XON_XOFF_CHECK_MAC(devNum,xonLimit);

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, xonLimit);
}

/**
* @internal cpssDxChGlobalXonLimitSet function
* @endinternal
*
* @brief   Sets the Global Xon limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xon limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xonLimit), all ports that support 802.3x Flow Control send an Xon
*         frame.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] xonLimit                 - X-ON limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or xonLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The Global Xon limit must be not be greater than Global Xoff Limit.
*       2. To disable Global Xon, the 2xonLimit should be set greater-or-equal
*       of 8<Max Buffer Limit> (maximal number of buffers allocated for
*       all ports).
*       3. Since in CH3 and above the HW resolution is 8,
*       in case the xonLimit input parameter is not a multiple of 8 we
*       round it UP to the nearest one.
*
*/
GT_STATUS cpssDxChGlobalXonLimitSet
(
    IN GT_U8    devNum,
    IN GT_U32   xonLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChGlobalXonLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xonLimit));

    rc = internal_cpssDxChGlobalXonLimitSet(devNum, xonLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xonLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChGlobalXonLimitGet function
* @endinternal
*
* @brief   Gets the Global Xon limit value.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChGlobalXonLimitGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *xonLimitPtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      fieldOffset; /* field offset     */
    GT_U32      fieldLength; /* field length     */

    GT_STATUS   rc;          /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(xonLimitPtr);
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_BM_MAC(devNum).BMGlobalConfigs.BMPauseConfig;
        fieldOffset = 0;
        fieldLength = 16;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngGlobalLimit;
        fieldOffset = 0;
        fieldLength = 11;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, xonLimitPtr);
    if( rc != GT_OK )
        return rc;
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* For DXCH3 and above, the field resolution is 8 buffers instead of 2 */
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            >= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            *xonLimitPtr = (*xonLimitPtr) << 2;
        }
    }
    else
    {
        *xonLimitPtr = *xonLimitPtr/2;
    }

    return GT_OK;
}

/**
* @internal cpssDxChGlobalXonLimitGet function
* @endinternal
*
* @brief   Gets the Global Xon limit value.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChGlobalXonLimitGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *xonLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChGlobalXonLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xonLimitPtr));

    rc = internal_cpssDxChGlobalXonLimitGet(devNum, xonLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xonLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChGlobalXoffLimitSet function
* @endinternal
*
* @brief   Sets the Global Xoff limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xoff limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xoffLimit), all ports that support 802.3x Flow Control send a pause
*         frame.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] xoffLimit                - X-OFF limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or xoffLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To disable Global Xoff, the 2xoffLimit should be set
*       greater-or-equal of 8<Max Buffer Limit> (maximal number of buffers
*       allocated for all ports).
*       2. Since in CH3 and above the HW resolution is 8,
*       in case the xoffLimit input parameter is not a multiple of 8 we
*       round it UP to the nearest one.
*
*/
static GT_STATUS internal_cpssDxChGlobalXoffLimitSet
(
    IN GT_U8    devNum,
    IN GT_U32   xoffLimit
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      fieldOffset; /* field offset     */
    GT_U32      fieldLength; /* field length     */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_BM_MAC(devNum).BMGlobalConfigs.BMPauseConfig;
        fieldOffset = 16;
        fieldLength = 16;
        xoffLimit = xoffLimit*2;
    }
    else
    {
        /* For DXCH and DXCH2 devices the field resolution is of 2 buffers */
        /* For DXCH3 and above, the field resolution is of 8 buffers,*/
        /* which we are rounding up */
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            >= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            if( xoffLimit != 0 )
            {
                xoffLimit = ((xoffLimit - 1) >> 2) + 1;
            }
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngGlobalLimit;
        fieldOffset = 11;
        fieldLength = 11;
    }

    PRV_CPSS_DXCH_GLOBAL_XON_XOFF_CHECK_MAC(devNum,xoffLimit);

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, xoffLimit);
}

/**
* @internal cpssDxChGlobalXoffLimitSet function
* @endinternal
*
* @brief   Sets the Global Xoff limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xoff limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xoffLimit), all ports that support 802.3x Flow Control send a pause
*         frame.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] xoffLimit                - X-OFF limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or xoffLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To disable Global Xoff, the 2xoffLimit should be set
*       greater-or-equal of 8<Max Buffer Limit> (maximal number of buffers
*       allocated for all ports).
*       2. Since in CH3 and above the HW resolution is 8,
*       in case the xoffLimit input parameter is not a multiple of 8 we
*       round it UP to the nearest one.
*
*/
GT_STATUS cpssDxChGlobalXoffLimitSet
(
    IN GT_U8    devNum,
    IN GT_U32   xoffLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChGlobalXoffLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xoffLimit));

    rc = internal_cpssDxChGlobalXoffLimitSet(devNum, xoffLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xoffLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChGlobalXoffLimitGet function
* @endinternal
*
* @brief   Gets the Global Xoff limit value.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChGlobalXoffLimitGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *xoffLimitPtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      fieldOffset; /* field offset     */
    GT_U32      fieldLength; /* field length     */

    GT_STATUS   rc;          /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(xoffLimitPtr);
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_BM_MAC(devNum).BMGlobalConfigs.BMPauseConfig;
        fieldOffset = 16;
        fieldLength = 16;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngGlobalLimit;
        fieldOffset = 11;
        fieldLength = 11;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, xoffLimitPtr);
    if( rc != GT_OK )
        return rc;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* For DXCH3 and above, the field resolution is 8 buffers instead of 2 */
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            >= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            *xoffLimitPtr = (*xoffLimitPtr) << 2;
        }
    }
    else
    {
        *xoffLimitPtr = *xoffLimitPtr/2;
    }
    return GT_OK;
}

/**
* @internal cpssDxChGlobalXoffLimitGet function
* @endinternal
*
* @brief   Gets the Global Xoff limit value.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChGlobalXoffLimitGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *xoffLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChGlobalXoffLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xoffLimitPtr));

    rc = internal_cpssDxChGlobalXoffLimitGet(devNum, xoffLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xoffLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRxFcProfileSet function
* @endinternal
*
* @brief   Bind a port to a flow control profile.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortRxFcProfileSet
(
    IN GT_U8                            devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  fieldOffset;    /* the start bit number in the register         */
    GT_U32  fieldLength;    /* needed field length */
    GT_U32  index;
    GT_U32  portGroupId;  /*the port group Id - support multi-port-groups device */
    GT_U32   localPort;    /* local port - support multi-port-groups device */
    GT_U32  portRxdmaNum; /* rxdma port number  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(devNum,portNum,portRxdmaNum);
    PRV_CPSS_DXCH_FC_PROFILE_CHECK_MAC(devNum,profileSet);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxdmaNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portRxdmaNum);
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig0[localPort];
        fieldOffset = 9;
        fieldLength = 3;
    }
    else
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        index = (localPort % 32);/* support also CPU port */

        fieldOffset =  (index % 16) * 2;
        fieldLength = 2;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
                    bufLimitsProfile.associationReg0 + 4 * (index / 16);
    }

    return prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,
                                     regAddr,
                                     fieldOffset,
                                     fieldLength,
                                     (GT_U32)profileSet);
}

/**
* @internal cpssDxChPortRxFcProfileSet function
* @endinternal
*
* @brief   Bind a port to a flow control profile.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxFcProfileSet
(
    IN GT_U8                            devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRxFcProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileSet));

    rc = internal_cpssDxChPortRxFcProfileSet(devNum, portNum, profileSet);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileSet));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRxFcProfileGet function
* @endinternal
*
* @brief   Get the port's flow control profile.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] profileSetPtr            - (pointer to) the Profile Set in which the Flow Control
*                                      Parameters are associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortRxFcProfileGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    OUT CPSS_PORT_RX_FC_PROFILE_SET_ENT  *profileSetPtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  fieldOffset;    /* the start bit number in the register         */
    GT_U32  fieldLength;    /* needed field length */
    GT_U32  index;
    GT_U32  portGroupId;  /*the port group Id - support multi-port-groups device */
    GT_U32   localPort;    /* local port - support multi-port-groups device */
    GT_U32  portRxdmaNum; /* rxdma port number  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(devNum,portNum,portRxdmaNum);
    CPSS_NULL_PTR_CHECK_MAC(profileSetPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxdmaNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portRxdmaNum);
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig0[localPort];
        fieldOffset = 9;
        fieldLength = 3;
    }
    else
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        index = (localPort % 32);/* support also CPU port */

        fieldOffset =  (index % 16) * 2;
        fieldLength = 2;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
                    bufLimitsProfile.associationReg0 + 4 * (index / 16);
    }

    return prvCpssHwPpPortGroupGetRegField(devNum,portGroupId,
                                     regAddr,
                                     fieldOffset,
                                     fieldLength,
                                     (GT_U32*)profileSetPtr);
}

/**
* @internal cpssDxChPortRxFcProfileGet function
* @endinternal
*
* @brief   Get the port's flow control profile.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] profileSetPtr            - (pointer to) the Profile Set in which the Flow Control
*                                      Parameters are associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxFcProfileGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    OUT CPSS_PORT_RX_FC_PROFILE_SET_ENT  *profileSetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRxFcProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileSetPtr));

    rc = internal_cpssDxChPortRxFcProfileGet(devNum, portNum, profileSetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileSetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortXonLimitSet function
* @endinternal
*
* @brief   Sets X-ON port limit. When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xonLimit                 - X-ON limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 the HW resolution is 8, in case the xonLimit input
*       parameter is not a multiple of 8 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/
static GT_STATUS internal_cpssDxChPortXonLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xonLimit
)
{
    GT_U32  regAddr;        /* register address                             */
    GT_U32  fieldOffset;    /* the start bit number in the register         */
    GT_U32  fieldLength;    /* the number of bits to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_FC_PROFILE_CHECK_MAC(devNum,profileSet);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileXONXOFFConfig[profileSet];
        fieldOffset = 0;
        fieldLength = 16;
        xonLimit = xonLimit << 1;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
                  bufLimitProfileConfig[profileSet];

        /* For DXCH and DXCH2 devices the field resolution is of 2 buffers */
        /* For DXCH3 the field resolution is of 8 buffers, which we are rounding up */
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            >= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            if( xonLimit != 0 )
            {
                xonLimit = ((xonLimit - 1) >> 2) + 1;
            }
        }

        fieldOffset= 0;
        fieldLength= 11;

    }

    PRV_CPSS_DXCH_GLOBAL_XON_XOFF_CHECK_MAC(devNum,xonLimit);

    return  prvCpssHwPpSetRegField(devNum,
                                      regAddr,
                                      fieldOffset,
                                      fieldLength,
                                      xonLimit);
}

/**
* @internal cpssDxChPortXonLimitSet function
* @endinternal
*
* @brief   Sets X-ON port limit. When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xonLimit                 - X-ON limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 the HW resolution is 8, in case the xonLimit input
*       parameter is not a multiple of 8 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortXonLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xonLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortXonLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, xonLimit));

    rc = internal_cpssDxChPortXonLimitSet(devNum, profileSet, xonLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, xonLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortXonLimitGet function
* @endinternal
*
* @brief   Gets the X-ON port limit. When Flow Control is enabled, X-ON packet
*         sent when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters of
*                                      are associated.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2 buffer resolution in HW is 1.
*
*/
static GT_STATUS internal_cpssDxChPortXonLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xonLimitPtr
)
{
    GT_U32  regAddr;        /* register address                             */
    GT_U32  fieldOffset;    /* the start bit number in the register         */
    GT_U32  fieldLength;    /* the number of bits to be written to register */
    GT_STATUS   rc;         /* return code                    */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_FC_PROFILE_CHECK_MAC(devNum,profileSet);
    CPSS_NULL_PTR_CHECK_MAC(xonLimitPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileXONXOFFConfig[profileSet];
        fieldOffset = 0;
        fieldLength = 16;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
                  bufLimitProfileConfig[profileSet];
        fieldOffset= 0;
        fieldLength= 11;
    }

    rc = prvCpssHwPpGetRegField(devNum,
                                   regAddr,
                                   fieldOffset,
                                   fieldLength,
                                   xonLimitPtr);
    if( rc != GT_OK )
        return rc;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* For DXCH3 the field resolution is 8 buffers instead of 2 */
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            >= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            *xonLimitPtr = (*xonLimitPtr) << 2;
        }
    }
    else
    {
        *xonLimitPtr = (*xonLimitPtr) >> 1;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortXonLimitGet function
* @endinternal
*
* @brief   Gets the X-ON port limit. When Flow Control is enabled, X-ON packet
*         sent when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters of
*                                      are associated.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortXonLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xonLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortXonLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, xonLimitPtr));

    rc = internal_cpssDxChPortXonLimitGet(devNum, profileSet, xonLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, xonLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortXoffLimitSet function
* @endinternal
*
* @brief   Sets X-OFF port limit. When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xoffLimit                - X-OFF limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 the HW resolution is 8, in case the xoffLimit input
*       parameter is not a multiple of 8 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/
static GT_STATUS internal_cpssDxChPortXoffLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xoffLimit
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  fieldOffset;    /* the start bit number in the register         */
    GT_U32  fieldLength;    /* the number of bits to be written to register */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_FC_PROFILE_CHECK_MAC(devNum,profileSet);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileXONXOFFConfig[profileSet];
        fieldOffset = 16;
        fieldLength = 16;
        xoffLimit = xoffLimit << 1;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
                  bufLimitProfileConfig[profileSet];

        /* For DXCH and DXCH2 devices the field resolution is of 2 buffers */
        /* For DXCH3 the field resolution is of 8 buffers, which we are rounding up */
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            >= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            if( xoffLimit != 0 )
            {
                xoffLimit = ((xoffLimit - 1) >> 2) + 1;
            }
        }
        fieldOffset= 11;
        fieldLength= 11;
    }

    PRV_CPSS_DXCH_GLOBAL_XON_XOFF_CHECK_MAC(devNum,xoffLimit);

    return  prvCpssHwPpSetRegField(devNum,
                                      regAddr,
                                      fieldOffset,
                                      fieldLength,
                                      xoffLimit);
}

/**
* @internal cpssDxChPortXoffLimitSet function
* @endinternal
*
* @brief   Sets X-OFF port limit. When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xoffLimit                - X-OFF limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 the HW resolution is 8, in case the xoffLimit input
*       parameter is not a multiple of 8 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortXoffLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xoffLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortXoffLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, xoffLimit));

    rc = internal_cpssDxChPortXoffLimitSet(devNum, profileSet, xoffLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, xoffLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortXoffLimitGet function
* @endinternal
*
* @brief   Gets the X-OFF port limit. When Flow Control is enabled, X-OFF packet
*         sent when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2 buffer resolution in HW is 1.
*
*/
static GT_STATUS internal_cpssDxChPortXoffLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xoffLimitPtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  fieldOffset;    /* the start bit number in the register         */
    GT_U32  fieldLength;    /* the number of bits to be written to register */
    GT_STATUS   rc;         /* return code                    */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_FC_PROFILE_CHECK_MAC(devNum,profileSet);
    CPSS_NULL_PTR_CHECK_MAC(xoffLimitPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileXONXOFFConfig[profileSet];
        fieldOffset = 16;
        fieldLength = 16;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
                  bufLimitProfileConfig[profileSet];
        fieldOffset= 11;
        fieldLength= 11;
    }

    rc = prvCpssHwPpGetRegField(devNum,
                                   regAddr,
                                   fieldOffset,
                                   fieldLength,
                                   xoffLimitPtr);

    if( rc != GT_OK )
        return rc;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* For DXCH3 the field resolution is 8 buffers instead of 2 */
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            >= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            *xoffLimitPtr = (*xoffLimitPtr) << 2;
        }
    }
    else
    {
        *xoffLimitPtr = (*xoffLimitPtr) >> 1;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortXoffLimitGet function
* @endinternal
*
* @brief   Gets the X-OFF port limit. When Flow Control is enabled, X-OFF packet
*         sent when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortXoffLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xoffLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortXoffLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, xoffLimitPtr));

    rc = internal_cpssDxChPortXoffLimitGet(devNum, profileSet, xoffLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, xoffLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] rxBufLimit               - buffer limit threshold in resolution of 4 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 and above, the HW resolution is 16, in case the rxBufLimit
*       input parameter is not a multiple of 16 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/
static GT_STATUS internal_cpssDxChPortRxBufLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           rxBufLimit
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  fieldOffset;    /* the start bit number in the register         */
    GT_U32  fieldLength;    /* the number of bits to be written to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_FC_PROFILE_CHECK_MAC(devNum,profileSet);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileRxlimitConfig[profileSet];
        fieldOffset = 0;
        fieldLength = 16;
        rxBufLimit = rxBufLimit << 2;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
                  bufLimitProfileConfig[profileSet];

        /* For DXCH and DXCH2 devices the field resolution is of 4 buffers */
        /* For DXCH3 the field resolution is of 16 buffers, which we are rounding up */
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            >= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            if( rxBufLimit != 0 )
            {
                rxBufLimit = ((rxBufLimit - 1) >> 2) + 1;
            }
        }
        fieldOffset= 22;
        fieldLength= 10;
    }

    PRV_CPSS_DXCH_RX_BUFFER_LIMIT_CHECK_MAC(devNum,rxBufLimit);

    return prvCpssHwPpSetRegField(devNum,
                                     regAddr,
                                     fieldOffset,
                                     fieldLength,
                                     rxBufLimit);
}

/**
* @internal cpssDxChPortRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] rxBufLimit               - buffer limit threshold in resolution of 4 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 and above, the HW resolution is 16, in case the rxBufLimit
*       input parameter is not a multiple of 16 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortRxBufLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           rxBufLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRxBufLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, rxBufLimit));

    rc = internal_cpssDxChPortRxBufLimitSet(devNum, profileSet, rxBufLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, rxBufLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRxBufLimitGet function
* @endinternal
*
* @brief   Gets the receive buffer limit threshold for the specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] rxBufLimitPtr            - (pointer to) the rx buffer limit value in resolution
*                                      of 4 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2 buffer resolution in HW is 1.
*
*/
static GT_STATUS internal_cpssDxChPortRxBufLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *rxBufLimitPtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  fieldOffset;    /* the start bit number in the register         */
    GT_U32  fieldLength;    /* the number of bits to be written to register */
    GT_STATUS   rc;         /* return code                    */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_FC_PROFILE_CHECK_MAC(devNum,profileSet);
    CPSS_NULL_PTR_CHECK_MAC(rxBufLimitPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileRxlimitConfig[profileSet];
        fieldOffset = 0;
        fieldLength = 16;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
                  bufLimitProfileConfig[profileSet];
        fieldOffset= 22;
        fieldLength= 10;
    }

    rc = prvCpssHwPpGetRegField(devNum,
                                   regAddr,
                                   fieldOffset,
                                   fieldLength,
                                   rxBufLimitPtr);
    if( rc != GT_OK )
        return rc;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* For DXCH3 the field resolution is 8 buffers instead of 2 */
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
            >= CPSS_PP_FAMILY_CHEETAH3_E))
        {
            *rxBufLimitPtr = (*rxBufLimitPtr) << 2;
        }
    }
    else
    {
        *rxBufLimitPtr = (*rxBufLimitPtr) >> 2;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortRxBufLimitGet function
* @endinternal
*
* @brief   Gets the receive buffer limit threshold for the specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] rxBufLimitPtr            - (pointer to) the rx buffer limit value in resolution
*                                      of 4 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortRxBufLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *rxBufLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRxBufLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, rxBufLimitPtr));

    rc = internal_cpssDxChPortRxBufLimitGet(devNum, profileSet, rxBufLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, rxBufLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCpuRxBufCountGet function
* @endinternal
*
* @brief   Gets receive buffer count for the CPU on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - The device to get the Cpu Rx buffer count for.
*
* @param[out] cpuRxBufCntPtr           - (Pointer to) number of buffers allocated for the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCpuRxBufCountGet
(
    IN  GT_U8   devNum,
    OUT GT_U16  *cpuRxBufCntPtr
)
{
    GT_U32 data;
    GT_U32 mask;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(cpuRxBufCntPtr);

    mask= 0x0FFF;

    if (prvCpssHwPpReadRegBitMask(devNum,
                           PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
                           cpuRxBufCount,
                           mask,
                           &data) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *cpuRxBufCntPtr = (GT_U16) (data & 0xFFFF);
    return GT_OK;
}

/**
* @internal cpssDxChPortCpuRxBufCountGet function
* @endinternal
*
* @brief   Gets receive buffer count for the CPU on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - The device to get the Cpu Rx buffer count for.
*
* @param[out] cpuRxBufCntPtr           - (Pointer to) number of buffers allocated for the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCpuRxBufCountGet
(
    IN  GT_U8   devNum,
    OUT GT_U16  *cpuRxBufCntPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCpuRxBufCountGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuRxBufCntPtr));

    rc = internal_cpssDxChPortCpuRxBufCountGet(devNum, cpuRxBufCntPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuRxBufCntPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortFcHolSysModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode on the specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] modeFcHol                - GT_FC_E  : set Flow Control mode
*                                      GT_HOL_E : set HOL system mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortFcHolSysModeSet
(
    IN  GT_U8                     devNum,
    IN  CPSS_DXCH_PORT_HOL_FC_ENT modeFcHol
)
{
    GT_U32      tailDropDis;
    GT_U32      bufsTailDropEn;
    GT_U32      descLim;  /* value to write into register   */
    GT_STATUS   rc;       /* return code                    */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    descLim  = 0x10;

    switch (modeFcHol)
    {
        case CPSS_DXCH_PORT_FC_E:
            tailDropDis    = 1;
            bufsTailDropEn = 0;

            /* When setting FC, first buffer's mode must be set and then */
            /* disable the Tail Drop. */
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
            {
                /* symmetric FC - set buffer's mode to divided */
                rc = cpssDxChPortBuffersModeSet(devNum,
                                           CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E);
                if (rc != GT_OK)
                    return rc;
            }
            break;

        case CPSS_DXCH_PORT_HOL_E:
            tailDropDis    = 0;
            bufsTailDropEn = 1;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    /* enable / disable Tail Drop */
    /* Disable Tail Drop for all Unicast packets. */
    rc = cpssDxChPortTxTailDropUcEnableSet(devNum,
        (tailDropDis == GT_TRUE ? GT_FALSE : GT_TRUE));

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* set a descriptor limit for Multicast packets*/
        rc = cpssDxChPortTxMcastPcktDescrLimitSet(devNum,descLim);
        if (rc != GT_OK)
            return rc;

        /* enable / disable Tail Drop according to the number of buffers */
        /* in the queues. */
        rc = cpssDxChPortTxBufferTailDropEnableSet(devNum,bufsTailDropEn);
        if (rc != GT_OK)
            return rc;
    }

    /* When setting HOL, first enable the Tail Drop (already done) and then */
    /* set buffer's mode. */
    if( CPSS_DXCH_PORT_HOL_E == modeFcHol )
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
        {
            /* tail drop - set buffer's mode to shared */
            rc = cpssDxChPortBuffersModeSet(devNum, CPSS_DXCH_PORT_BUFFERS_MODE_SHARED_E);
        }
    }

    return rc;
}

/**
* @internal cpssDxChPortFcHolSysModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode on the specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] modeFcHol                - GT_FC_E  : set Flow Control mode
*                                      GT_HOL_E : set HOL system mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortFcHolSysModeSet
(
    IN  GT_U8                     devNum,
    IN  CPSS_DXCH_PORT_HOL_FC_ENT modeFcHol
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFcHolSysModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modeFcHol));

    rc = internal_cpssDxChPortFcHolSysModeSet(devNum, modeFcHol);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modeFcHol));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortBuffersModeSet function
* @endinternal
*
* @brief   The function sets buffers mode either shared or divided.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] bufferMode               - buffers mode, divided or shared.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on invalid input parameters value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortBuffersModeSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_PORT_BUFFERS_MODE_ENT  bufferMode
)
{

    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    switch (bufferMode)
    {
        case CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E:
            value = 1;
            break;
        case CPSS_DXCH_PORT_BUFFERS_MODE_SHARED_E:
            value = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngGlobalLimit;

    return  prvCpssHwPpSetRegField(devNum, regAddr, 31, 1,value);
}

/**
* @internal cpssDxChPortBuffersModeSet function
* @endinternal
*
* @brief   The function sets buffers mode either shared or divided.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] bufferMode               - buffers mode, divided or shared.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on invalid input parameters value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortBuffersModeSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_PORT_BUFFERS_MODE_ENT  bufferMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortBuffersModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bufferMode));

    rc = internal_cpssDxChPortBuffersModeSet(devNum, bufferMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bufferMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortBuffersModeGet function
* @endinternal
*
* @brief   The function gets buffers mode either shared or divided.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] bufferModePtr            - pointer to the buffer mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on invalid input parameters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortBuffersModeGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_PORT_BUFFERS_MODE_ENT *bufferModePtr
)
{
    GT_U32    regAddr;     /* register address */
    GT_U32    value;       /* value to be read from the register */
    GT_STATUS rc;          /* function return value            */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(bufferModePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngGlobalLimit;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 31, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *bufferModePtr = (value == 1) ?
            CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E :
            CPSS_DXCH_PORT_BUFFERS_MODE_SHARED_E;
    return GT_OK;

}

/**
* @internal cpssDxChPortBuffersModeGet function
* @endinternal
*
* @brief   The function gets buffers mode either shared or divided.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] bufferModePtr            - pointer to the buffer mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on invalid input parameters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortBuffersModeGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_PORT_BUFFERS_MODE_ENT *bufferModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortBuffersModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bufferModePtr));

    rc = internal_cpssDxChPortBuffersModeGet(devNum, bufferModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bufferModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortGroupXonLimitSet function
* @endinternal
*
* @brief   The function sets the port-group X-ON threshold.
*         When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
* @param[in] xonLimit                 -  X-ON limit in buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of xonLimit field in hardware:
*       - DXCH and DXCH2 devices: 2 buffers.
*       - DXCH3 and above devices: 8 buffers.
*       The value is rounded down before write to HW.
*
*/
static GT_STATUS internal_cpssDxChPortGroupXonLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    xonLimit
)
{
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    switch(portGroup)
    {
        case CPSS_DXCH_PORT_GROUP_GIGA_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngGigaPortGroupLimit;
            break;

        case CPSS_DXCH_PORT_GROUP_HGS_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngHyperGStackPortGroupLimit;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* For DXCH and DXCH2 devices the field resolution is of 2 buffers */
    /* For DXCH3 and above, the field resolution is of 8 buffers,*/
    /* which we are rounding down */
    if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        >= CPSS_PP_FAMILY_CHEETAH3_E))
    {
        xonLimit = xonLimit/8;
    }
    else
    {
        xonLimit = xonLimit/2;
    }

    PRV_CPSS_DXCH_GLOBAL_XON_XOFF_CHECK_MAC(devNum,xonLimit);

    return  prvCpssHwPpSetRegField(devNum, regAddr, 0, 11, xonLimit);

}

/**
* @internal cpssDxChPortGroupXonLimitSet function
* @endinternal
*
* @brief   The function sets the port-group X-ON threshold.
*         When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
* @param[in] xonLimit                 -  X-ON limit in buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of xonLimit field in hardware:
*       - DXCH and DXCH2 devices: 2 buffers.
*       - DXCH3 and above devices: 8 buffers.
*       The value is rounded down before write to HW.
*
*/
GT_STATUS cpssDxChPortGroupXonLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    xonLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortGroupXonLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroup, xonLimit));

    rc = internal_cpssDxChPortGroupXonLimitSet(devNum, portGroup, xonLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroup, xonLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortGroupXonLimitGet function
* @endinternal
*
* @brief   The function gets the port-group X-ON threshold.
*         When Flow Control is enabled, X-ON packet
*         sent when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] xonLimitPtr              - Pointer to the  X-ON limit in buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortGroupXonLimitGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT portGroup,
    OUT GT_U32                   *xonLimitPtr
)
{
    GT_U32    regAddr;         /* register address */
    GT_STATUS rc;              /* return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(xonLimitPtr);

    switch(portGroup)
    {
        case CPSS_DXCH_PORT_GROUP_GIGA_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngGigaPortGroupLimit;
            break;

        case CPSS_DXCH_PORT_GROUP_HGS_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngHyperGStackPortGroupLimit;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 11, xonLimitPtr);
    if(rc != GT_OK)
        return rc;

    /* For DXCH and DXCH2 devices the field resolution is of 2 buffers */
    /* For DXCH3 and above, the field resolution is of 8 buffers */
    if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        >= CPSS_PP_FAMILY_CHEETAH3_E))
    {
        *xonLimitPtr = (*xonLimitPtr) * 8;
    }
    else
    {
        *xonLimitPtr = (*xonLimitPtr) * 2;
    }

    return GT_OK;


}

/**
* @internal cpssDxChPortGroupXonLimitGet function
* @endinternal
*
* @brief   The function gets the port-group X-ON threshold.
*         When Flow Control is enabled, X-ON packet
*         sent when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] xonLimitPtr              - Pointer to the  X-ON limit in buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGroupXonLimitGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT portGroup,
    OUT GT_U32                   *xonLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortGroupXonLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroup, xonLimitPtr));

    rc = internal_cpssDxChPortGroupXonLimitGet(devNum, portGroup, xonLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroup, xonLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortGroupXoffLimitSet function
* @endinternal
*
* @brief   The function sets the port-group X-OFF threshold.
*         When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
* @param[in] xoffLimit                - X-OFF limit in buffers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of xoffLimit field in hardware:
*       - DXCH and DXCH2 devices: 2 buffers.
*       - DXCH3 and above devices: 8 buffers.
*       The value is rounded down before write to HW.
*
*/
static GT_STATUS internal_cpssDxChPortGroupXoffLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    xoffLimit
)
{
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    switch(portGroup)
    {
        case CPSS_DXCH_PORT_GROUP_GIGA_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngGigaPortGroupLimit;
            break;

        case CPSS_DXCH_PORT_GROUP_HGS_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngHyperGStackPortGroupLimit;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* For DXCH and DXCH2 devices the field resolution is of 2 buffers */
    /* For DXCH3 and above, the field resolution is of 8 buffers,*/
    /* which we are rounding down */
    if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        >= CPSS_PP_FAMILY_CHEETAH3_E))
    {
        xoffLimit = xoffLimit/8;
    }
    else
    {
        xoffLimit = xoffLimit/2;
    }

    PRV_CPSS_DXCH_GLOBAL_XON_XOFF_CHECK_MAC(devNum,xoffLimit);

    return  prvCpssHwPpSetRegField(devNum, regAddr, 11, 11, xoffLimit);

}

/**
* @internal cpssDxChPortGroupXoffLimitSet function
* @endinternal
*
* @brief   The function sets the port-group X-OFF threshold.
*         When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
* @param[in] xoffLimit                - X-OFF limit in buffers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of xoffLimit field in hardware:
*       - DXCH and DXCH2 devices: 2 buffers.
*       - DXCH3 and above devices: 8 buffers.
*       The value is rounded down before write to HW.
*
*/
GT_STATUS cpssDxChPortGroupXoffLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    xoffLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortGroupXoffLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroup, xoffLimit));

    rc = internal_cpssDxChPortGroupXoffLimitSet(devNum, portGroup, xoffLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroup, xoffLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortGroupXoffLimitGet function
* @endinternal
*
* @brief   The function gets the port-group X-OFF threshold.
*         When Flow Control is enabled, X-OFF packet
*         sent when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] xoffLimitPtr             - Pointer to the X-OFF limit in buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortGroupXoffLimitGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT portGroup,
    OUT GT_U32                   *xoffLimitPtr
)
{
    GT_U32    regAddr;         /* register address */
    GT_STATUS rc;              /* return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(xoffLimitPtr);

    switch(portGroup)
    {
        case CPSS_DXCH_PORT_GROUP_GIGA_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngGigaPortGroupLimit;
            break;

        case CPSS_DXCH_PORT_GROUP_HGS_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngHyperGStackPortGroupLimit;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssHwPpGetRegField(devNum, regAddr, 11, 11, xoffLimitPtr);
    if(rc != GT_OK)
        return rc;

    /* For DXCH and DXCH2 devices the field resolution is of 2 buffers */
    /* For DXCH3 and above, the field resolution is of 8 buffers */
    if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        >= CPSS_PP_FAMILY_CHEETAH3_E))
    {
        *xoffLimitPtr = (*xoffLimitPtr) * 8;
    }
    else
    {
        *xoffLimitPtr = (*xoffLimitPtr) * 2;
    }

    return GT_OK;

}

/**
* @internal cpssDxChPortGroupXoffLimitGet function
* @endinternal
*
* @brief   The function gets the port-group X-OFF threshold.
*         When Flow Control is enabled, X-OFF packet
*         sent when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] xoffLimitPtr             - Pointer to the X-OFF limit in buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGroupXoffLimitGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT portGroup,
    OUT GT_U32                   *xoffLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortGroupXoffLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroup, xoffLimitPtr));

    rc = internal_cpssDxChPortGroupXoffLimitGet(devNum, portGroup, xoffLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroup, xoffLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortGroupRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified port-group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
* @param[in] rxBufLimit               - Maximal number of buffers that may be allocated for the
*                                      ports belong to a group
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of rxBufLimit field in hardware:
*       - DXCH and DXCH2 devices: 4 buffers.
*       - DXCH3 and above devices: 16 buffers.
*       The value is rounded down before write to HW.
*
*/
static GT_STATUS internal_cpssDxChPortGroupRxBufLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    rxBufLimit
)
{
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    switch(portGroup)
    {
        case CPSS_DXCH_PORT_GROUP_GIGA_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngGigaPortGroupLimit;
            break;

        case CPSS_DXCH_PORT_GROUP_HGS_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngHyperGStackPortGroupLimit;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* For DXCH and DXCH2 devices the field resolution is of 4 buffers */
    /* For DXCH3 and above, the field resolution is of 16 buffers,*/
    /* which we are rounding down */
    if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        >= CPSS_PP_FAMILY_CHEETAH3_E))
    {
        rxBufLimit = rxBufLimit/16;
    }
    else
    {
        rxBufLimit = rxBufLimit/4;
    }

    if(rxBufLimit >= BIT_10)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return  prvCpssHwPpSetRegField(devNum, regAddr, 22, 10, rxBufLimit);

}

/**
* @internal cpssDxChPortGroupRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified port-group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
* @param[in] rxBufLimit               - Maximal number of buffers that may be allocated for the
*                                      ports belong to a group
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of rxBufLimit field in hardware:
*       - DXCH and DXCH2 devices: 4 buffers.
*       - DXCH3 and above devices: 16 buffers.
*       The value is rounded down before write to HW.
*
*/
GT_STATUS cpssDxChPortGroupRxBufLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    rxBufLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortGroupRxBufLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroup, rxBufLimit));

    rc = internal_cpssDxChPortGroupRxBufLimitSet(devNum, portGroup, rxBufLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroup, rxBufLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortGroupRxBufLimitGet function
* @endinternal
*
* @brief   Gets the receive buffer limit threshold for the specified port-group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] rxBufLimitPtr            - Pointer to value of maximal number of buffers that
*                                      may be allocated for the ports belong to a group.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortGroupRxBufLimitGet
(
    IN  GT_U8                     devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    OUT GT_U32                    *rxBufLimitPtr
)
{
    GT_U32    regAddr;         /* register address */
    GT_U32    value;           /* register value */
    GT_STATUS rc;              /* return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(rxBufLimitPtr);

    switch(portGroup)
    {
        case CPSS_DXCH_PORT_GROUP_GIGA_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngGigaPortGroupLimit;
            break;

        case CPSS_DXCH_PORT_GROUP_HGS_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngHyperGStackPortGroupLimit;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssHwPpGetRegField(devNum, regAddr, 22, 10, &value);
    if( rc != GT_OK )
        return rc;

    /* For DXCH and DXCH2 devices the field resolution is of 4 buffers */
    /* For DXCH3 and above, the field resolution is of 16 buffers */
    if ((PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily
        >= CPSS_PP_FAMILY_CHEETAH3_E))
    {
        *rxBufLimitPtr = value * 16;
    }
    else
    {
        *rxBufLimitPtr = value * 4;
    }

    return GT_OK;

}

/**
* @internal cpssDxChPortGroupRxBufLimitGet function
* @endinternal
*
* @brief   Gets the receive buffer limit threshold for the specified port-group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] rxBufLimitPtr            - Pointer to value of maximal number of buffers that
*                                      may be allocated for the ports belong to a group.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGroupRxBufLimitGet
(
    IN  GT_U8                     devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    OUT GT_U32                    *rxBufLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortGroupRxBufLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroup, rxBufLimitPtr));

    rc = internal_cpssDxChPortGroupRxBufLimitGet(devNum, portGroup, rxBufLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroup, rxBufLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCrossChipFcPacketRelayEnableSet function
* @endinternal
*
* @brief   Enable/Disable SOHO Cross Chip Flow control packet relay.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE:  Enable SOHO Cross Chip Flow control packet relay.
*                                      GT_FALSE: Disable SOHO Cross Chip Flow control packet relay.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCrossChipFcPacketRelayEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    value = (enable == GT_TRUE) ? 1 : 0;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ctrlReg0;

    return prvCpssHwPpSetRegField(devNum, regAddr, 22, 1, value);
}

/**
* @internal cpssDxChPortCrossChipFcPacketRelayEnableSet function
* @endinternal
*
* @brief   Enable/Disable SOHO Cross Chip Flow control packet relay.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE:  Enable SOHO Cross Chip Flow control packet relay.
*                                      GT_FALSE: Disable SOHO Cross Chip Flow control packet relay.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCrossChipFcPacketRelayEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCrossChipFcPacketRelayEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortCrossChipFcPacketRelayEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortCrossChipFcPacketRelayEnableGet function
* @endinternal
*
* @brief   Gets the current status of SOHO Cross Chip Flow control packet relay.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to SOHO Cross Chip Flow control packet relay staus:
*                                      GT_TRUE  -  Enable SOHO Cross Chip Flow control packet relay.
*                                      GT_FALSE - Disable SOHO Cross Chip Flow control packet relay.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortCrossChipFcPacketRelayEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32     value;     /* register value */
    GT_U32     regAddr;   /* register address */
    GT_STATUS  rc;        /* function return value */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ctrlReg0;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 22, 1, &value);

    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;

}

/**
* @internal cpssDxChPortCrossChipFcPacketRelayEnableGet function
* @endinternal
*
* @brief   Gets the current status of SOHO Cross Chip Flow control packet relay.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to SOHO Cross Chip Flow control packet relay staus:
*                                      GT_TRUE  -  Enable SOHO Cross Chip Flow control packet relay.
*                                      GT_FALSE - Disable SOHO Cross Chip Flow control packet relay.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCrossChipFcPacketRelayEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortCrossChipFcPacketRelayEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortCrossChipFcPacketRelayEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortClearByBufferEnableSet function
* @endinternal
*
* @brief   Set port to work in Clear By Buffer mode.
*
* @note   APPLICABLE DEVICES:      DxCh1.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] enable                   - GT_TRUE: Clear By Buffer mode.
*                                      GT_FALSE: Regular Clear mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortClearByBufferEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
)
{
    GT_U32  index;      /* index of port bit in the register */
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data */
    GT_STATUS rc;       /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E |
                                          CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

    if(portNum == CPSS_CPU_PORT_NUM_CNS)
    {
        index = 27;
    }
    else /* physical port - not CPU port */
    {
        index = portNum;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->addrSpace.buffMemClearType;

    if(enable == GT_TRUE)
    {
        /* set port bit in buffer memory clear type register */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, index, 1, 1);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* RM: enable Clear By Buffer in Buffer Management Metal Fix Register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngMetalFix;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 4, 1, 1);
    }
    else
    {
        /* clear port bit in buffer memory clear type register */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, index, 1, 0);
        if(rc != GT_OK)
        {
            return rc;
        }

        regData = 0;
        rc = prvCpssHwPpReadRegBitMask(devNum, regAddr, 0x0FFFFFFF, &regData);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* check if all port bits are cleared. */
        /* In case no port is working in Clear By Buffer mode, */
        /* disable Clear By Buffer in Buffer Management Metal Fix Register */
        if( regData == 0x0 )
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngMetalFix;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 4, 1, 0);
        }
    }

    return rc;
}

/**
* @internal cpssDxChPortClearByBufferEnableSet function
* @endinternal
*
* @brief   Set port to work in Clear By Buffer mode.
*
* @note   APPLICABLE DEVICES:      DxCh1.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] enable                   - GT_TRUE: Clear By Buffer mode.
*                                      GT_FALSE: Regular Clear mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortClearByBufferEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortClearByBufferEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortClearByBufferEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortClearByBufferEnableGet function
* @endinternal
*
* @brief   Get port Buffer Memory Clear type.
*
* @note   APPLICABLE DEVICES:      DxCh1.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] enablePtr                - (pointer to) GT_TRUE: Clear By Buffer mode.
*                                      GT_FALSE: Regular Clear mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortClearByBufferEnableGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32  index;      /* index of port bit in the register */
    GT_U32  regAddr;    /* register address */
    GT_U32  value;      /* register bit value */
    GT_STATUS rc;       /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E |
                                          CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(portNum == CPSS_CPU_PORT_NUM_CNS)
    {
        index = 27;
    }
    else /* physical port - not CPU port */
    {
        index = portNum;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->addrSpace.buffMemClearType;

    /* set port bit in buffer memory clear type register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, index, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChPortClearByBufferEnableGet function
* @endinternal
*
* @brief   Get port Buffer Memory Clear type.
*
* @note   APPLICABLE DEVICES:      DxCh1.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] enablePtr                - (pointer to) GT_TRUE: Clear By Buffer mode.
*                                      GT_FALSE: Regular Clear mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortClearByBufferEnableGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortClearByBufferEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortClearByBufferEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortGlobalRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortGlobalRxBufNumberGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *numOfBuffersPtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      fieldOffset; /* field offset     */
    GT_U32      fieldLength; /* field length     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfBuffersPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_BM_MAC(devNum).BMCntrs.BMGlobalBufferCntr;
        fieldOffset = 0;
        fieldLength = 16;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.totalBufAllocCntr;
        fieldOffset = 0;
        fieldLength = 13;
    }
    /* Get counters value */
    return prvCpssPortGroupsCounterSummary(devNum, regAddr,fieldOffset,fieldLength, numOfBuffersPtr,NULL);
}

/**
* @internal cpssDxChPortGlobalRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGlobalRxBufNumberGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *numOfBuffersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortGlobalRxBufNumberGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfBuffersPtr));

    rc = internal_cpssDxChPortGlobalRxBufNumberGet(devNum, numOfBuffersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfBuffersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortGroupRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated for the specified
*         port group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or port group
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortGroupRxBufNumberGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    OUT GT_U32  *numOfBuffersPtr
)
{
    GT_U32      regAddr;     /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(numOfBuffersPtr);

    switch (portGroup)
    {
        case CPSS_DXCH_PORT_GROUP_GIGA_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.netPortGroupAllocCntr;
            break;
        case CPSS_DXCH_PORT_GROUP_HGS_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.stackPortGroupAllocCntr;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get counters value */
    return prvCpssPortGroupsCounterSummary(devNum, regAddr,0,13, numOfBuffersPtr,NULL);
}

/**
* @internal cpssDxChPortGroupRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated for the specified
*         port group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or port group
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGroupRxBufNumberGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    OUT GT_U32  *numOfBuffersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortGroupRxBufNumberGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroup, numOfBuffersPtr));

    rc = internal_cpssDxChPortGroupRxBufNumberGet(devNum, portGroup, numOfBuffersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroup, numOfBuffersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated for the specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortRxBufNumberGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *numOfBuffersPtr
)
{
    GT_U32      regAddr;      /* register address */
    GT_U32      portRxdmaNum; /* port rxdma number */
    GT_U32      fieldOffset; /* field offset     */
    GT_U32      fieldLength; /* field length     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_FALCON_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(devNum,portNum,portRxdmaNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfBuffersPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).scdmaDebug.portBuffAllocCnt[portRxdmaNum];
        fieldOffset = 0;
        fieldLength = 20;
    }
    else
    {
        if(portNum == CPSS_CPU_PORT_NUM_CNS)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.cpuBufMngPerPort.portBufAllocCounter;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bufferMng.bufMngPerPort[portNum].portBufAllocCounter;
        }
        fieldOffset = 0;
        fieldLength = 13;
    }


    return prvCpssHwPpPortGroupGetRegField(devNum, PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxdmaNum),
            regAddr, fieldOffset, fieldLength, numOfBuffersPtr);
}

/**
* @internal cpssDxChPortRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated for the specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxBufNumberGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *numOfBuffersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRxBufNumberGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, numOfBuffersPtr));

    rc = internal_cpssDxChPortRxBufNumberGet(devNum, portNum, numOfBuffersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, numOfBuffersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortGlobalPacketNumberGet function
* @endinternal
*
* @brief   Gets total number of unique packets currently in the system.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numOfPacketsPtr          - (pointer to) number of packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortGlobalPacketNumberGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *numOfPacketsPtr
)
{
    GT_U32      regAddr;     /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E |
                                          CPSS_CH1_DIAMOND_E | CPSS_CH2_E);
    CPSS_NULL_PTR_CHECK_MAC(numOfPacketsPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    else
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).debug.countersAndCountersStatuses.currentNumOfPacketsStoredInMppm;

        /* Get counters value */
        return prvCpssPortGroupsCounterSummary(devNum, regAddr, 0, 32, numOfPacketsPtr, NULL);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngPacketCntr;

        /* Get counters value */
        return prvCpssPortGroupsCounterSummary(devNum, regAddr,0,13, numOfPacketsPtr,NULL);
    }
}

/**
* @internal cpssDxChPortGlobalPacketNumberGet function
* @endinternal
*
* @brief   Gets total number of unique packets currently in the system.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numOfPacketsPtr          - (pointer to) number of packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGlobalPacketNumberGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *numOfPacketsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortGlobalPacketNumberGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfPacketsPtr));

    rc = internal_cpssDxChPortGlobalPacketNumberGet(devNum, numOfPacketsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfPacketsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRxNumOfAgedBuffersGet function
* @endinternal
*
* @brief   Gets number of buffers cleared by aging.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] agedBuffersPtr           - (pointer to) Number of aged buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortRxNumOfAgedBuffersGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *agedBuffersPtr
)
{
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(agedBuffersPtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngAgedBuffersCntr;

    return prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32,
                                               agedBuffersPtr, NULL);
}

/**
* @internal cpssDxChPortRxNumOfAgedBuffersGet function
* @endinternal
*
* @brief   Gets number of buffers cleared by aging.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] agedBuffersPtr           - (pointer to) Number of aged buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxNumOfAgedBuffersGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *agedBuffersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRxNumOfAgedBuffersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, agedBuffersPtr));

    rc = internal_cpssDxChPortRxNumOfAgedBuffersGet(devNum, agedBuffersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, agedBuffersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortRxMcCntrGet function
* @endinternal
*
* @brief   Gets multicast counter of a certain buffer.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrIdx                  - counter index
*                                      (APPLICABLE RANGES for Lion, Lion2: 0..16383)
*                                      (APPLICABLE RANGES for xCat2: 0..4095)
*                                      (APPLICABLE RANGES for Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X: 0..8191)
*
* @param[out] mcCntrPtr                - (pointer to) multicast counter of a certain buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or bufIdx
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortRxMcCntrGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   cntrIdx,
    OUT GT_U32   *mcCntrPtr
)
{
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E);
    CPSS_NULL_PTR_CHECK_MAC(mcCntrPtr);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if (cntrIdx >= _8K)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        return prvCpssDxChReadTableEntry(devNum,
                                         CPSS_DXCH_SIP5_TABLE_BMA_MULTICAST_COUNTERS_E,
                                         cntrIdx, mcCntrPtr);
    }
    else
    {
        /* update implementation for Lion2, see Bugs00127999 */
        CPSS_TBD_BOOKMARK_LION2

        /* Check index validity */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
        {
            /* xCat2 */
            if (cntrIdx >= _4K)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            /* Lion */
            if (cntrIdx >= _16K)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.mcCntrsMemBaseAddr +
                                                                        (cntrIdx * 0x4);

        /* For Lion, this counter valid only in port group 0 */
        return prvCpssHwPpPortGroupGetRegField(devNum, 0, regAddr, 0, 11, mcCntrPtr);
    }
}

/**
* @internal cpssDxChPortRxMcCntrGet function
* @endinternal
*
* @brief   Gets multicast counter of a certain buffer.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrIdx                  - counter index
*                                      (APPLICABLE RANGES for Lion, Lion2: 0..16383)
*                                      (APPLICABLE RANGES for xCat2: 0..4095)
*                                      (APPLICABLE RANGES for Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X: 0..8191)
*
* @param[out] mcCntrPtr                - (pointer to) multicast counter of a certain buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or bufIdx
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxMcCntrGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   cntrIdx,
    OUT GT_U32   *mcCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRxMcCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrIdx, mcCntrPtr));

    rc = internal_cpssDxChPortRxMcCntrGet(devNum, cntrIdx, mcCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrIdx, mcCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortBufMemFifosThresholdSet function
* @endinternal
*
* @brief   Sets buffer memory FIFOs Threshold.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] fifoType                 - FIFO type
* @param[in] descFifoThreshold        - number of descriptors per port threshold mode.
* @param[in] txFifoThreshold          - number of lines per port threshold mode in the Tx FIFO.
* @param[in] minXFifoReadThreshold    - minimal number of descriptors to read per port threshold mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Improper configuration of values may affect entire system behavior.
*
*/
static GT_STATUS internal_cpssDxChPortBufMemFifosThresholdSet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT fifoType,
    IN  GT_U32   descFifoThreshold,
    IN  GT_U32   txFifoThreshold,
    IN  GT_U32   minXFifoReadThreshold
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      hwData;         /* data to read from hw */
    GT_U32      hwDataSize;     /* size of HW data in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E |
                                          CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_FIFO_TYPE_CHECK_MAC(fifoType);

    if ((txFifoThreshold >= BIT_6) ||
        (minXFifoReadThreshold >= BIT_6))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        if (descFifoThreshold >= BIT_4)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        hwData = (descFifoThreshold | (txFifoThreshold << 4) |
                  (minXFifoReadThreshold << 10));

        hwDataSize = 16;
    }
    else /* Lion2, Bobcat2, Caelum, Bobcat3, Aldrin, AC3X */
    {
        if (descFifoThreshold >= BIT_6)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        hwData = (descFifoThreshold | (txFifoThreshold << 6) |
                  (minXFifoReadThreshold << 12));

        hwDataSize = 18;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaBufMemFifosThreshold[fifoType];

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, hwDataSize, hwData);
}

/**
* @internal cpssDxChPortBufMemFifosThresholdSet function
* @endinternal
*
* @brief   Sets buffer memory FIFOs Threshold.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] fifoType                 - FIFO type
* @param[in] descFifoThreshold        - number of descriptors per port threshold mode.
* @param[in] txFifoThreshold          - number of lines per port threshold mode in the Tx FIFO.
* @param[in] minXFifoReadThreshold    - minimal number of descriptors to read per port threshold mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Improper configuration of values may affect entire system behavior.
*
*/
GT_STATUS cpssDxChPortBufMemFifosThresholdSet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT fifoType,
    IN  GT_U32   descFifoThreshold,
    IN  GT_U32   txFifoThreshold,
    IN  GT_U32   minXFifoReadThreshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortBufMemFifosThresholdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fifoType, descFifoThreshold, txFifoThreshold, minXFifoReadThreshold));

    rc = internal_cpssDxChPortBufMemFifosThresholdSet(devNum, fifoType, descFifoThreshold, txFifoThreshold, minXFifoReadThreshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fifoType, descFifoThreshold, txFifoThreshold, minXFifoReadThreshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortBufMemFifosThresholdGet function
* @endinternal
*
* @brief   Gets buffer memory FIFOs Thresholds.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] fifoType                 - FIFO type
*
* @param[out] descFifoThresholdPtr     - pointer to number of descriptors per port per port threshold mode.
* @param[out] txFifoThresholdPtr       - pointer to number of lines per port per port threshold mode in the Tx FIFO.
* @param[out] minXFifoReadThresholdPtr - pointer to minimal number of descriptors to read per port per port threshold mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortBufMemFifosThresholdGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT fifoType,
    OUT GT_U32   *descFifoThresholdPtr,
    OUT GT_U32   *txFifoThresholdPtr,
    OUT GT_U32   *minXFifoReadThresholdPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      hwData;     /* data to read from hw */
    GT_STATUS   rc;         /* return code      */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E |
                                          CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_FIFO_TYPE_CHECK_MAC(fifoType);

    CPSS_NULL_PTR_CHECK_MAC(descFifoThresholdPtr);
    CPSS_NULL_PTR_CHECK_MAC(txFifoThresholdPtr);
    CPSS_NULL_PTR_CHECK_MAC(minXFifoReadThresholdPtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaBufMemFifosThreshold[fifoType];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 18, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        *descFifoThresholdPtr = (hwData & 0xF);
        *txFifoThresholdPtr = ((hwData >> 4) & 0x3F);
        *minXFifoReadThresholdPtr = ((hwData >> 10) & 0x3F);
    }
    else
    {
        *descFifoThresholdPtr = (hwData & 0x3F);
        *txFifoThresholdPtr = ((hwData >> 6) & 0x3F);
        *minXFifoReadThresholdPtr = ((hwData >> 12) & 0x3F);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortBufMemFifosThresholdGet function
* @endinternal
*
* @brief   Gets buffer memory FIFOs Thresholds.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] fifoType                 - FIFO type
*
* @param[out] descFifoThresholdPtr     - pointer to number of descriptors per port per port threshold mode.
* @param[out] txFifoThresholdPtr       - pointer to number of lines per port per port threshold mode in the Tx FIFO.
* @param[out] minXFifoReadThresholdPtr - pointer to minimal number of descriptors to read per port per port threshold mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortBufMemFifosThresholdGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT fifoType,
    OUT GT_U32   *descFifoThresholdPtr,
    OUT GT_U32   *txFifoThresholdPtr,
    OUT GT_U32   *minXFifoReadThresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortBufMemFifosThresholdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fifoType, descFifoThresholdPtr, txFifoThresholdPtr, minXFifoReadThresholdPtr));

    rc = internal_cpssDxChPortBufMemFifosThresholdGet(devNum, fifoType, descFifoThresholdPtr, txFifoThresholdPtr, minXFifoReadThresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fifoType, descFifoThresholdPtr, txFifoThresholdPtr, minXFifoReadThresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortTxdmaBurstLimitEnableSet function
* @endinternal
*
* @brief   For a given port Enable/Disable TXDMA burst limit thresholds use.
*         When enabled: instruct the TxDMA to throttle the Transmit Queue Scheduler
*         as a function of the TxDMA FIFOs fill level measured in bytes and descriptor.
*         When disabled: the TxDMA throttles the Transmit Queue Scheduler
*         based on FIFOs fill level measured in descriptors only.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] enable                   - GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortTxdmaBurstLimitEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
)
{
    GT_U32  bitNum;
    GT_U32  regAddr;
    GT_U32  value;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_U32  portRxdmaNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXDMA_NUM_GET_MAC(devNum,portNum,portRxdmaNum);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).txDMAPerSCDMAConfigs.SCDMAConfigs[portRxdmaNum];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).txDMAPerSCDMAConfigs.burstLimiterSCDMA[portRxdmaNum];
        }
        bitNum = 30;
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxdmaNum);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaBurstLimitGlobalConfig;
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
        bitNum = (localPort == CPSS_CPU_PORT_NUM_CNS) ? 12 : localPort;
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    }
    value = BOOL2BIT_MAC(enable);

    return prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,bitNum,1,value);
}

/**
* @internal cpssDxChPortTxdmaBurstLimitEnableSet function
* @endinternal
*
* @brief   For a given port Enable/Disable TXDMA burst limit thresholds use.
*         When enabled: instruct the TxDMA to throttle the Transmit Queue Scheduler
*         as a function of the TxDMA FIFOs fill level measured in bytes and descriptor.
*         When disabled: the TxDMA throttles the Transmit Queue Scheduler
*         based on FIFOs fill level measured in descriptors only.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] enable                   - GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxdmaBurstLimitEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxdmaBurstLimitEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortTxdmaBurstLimitEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxdmaBurstLimitEnableGet function
* @endinternal
*
* @brief   Gets the current status for a given port of TXDMA burst limit thresholds use.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortTxdmaBurstLimitEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32  bitNum;
    GT_U32  regAddr;
    GT_U32  value;
    GT_STATUS   rc;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_U32  portRxdmaNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXDMA_NUM_GET_MAC(devNum,portNum,portRxdmaNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).txDMAPerSCDMAConfigs.SCDMAConfigs[portRxdmaNum];
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).txDMAPerSCDMAConfigs.burstLimiterSCDMA[portRxdmaNum];
        }
        bitNum = 30;
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxdmaNum);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaBurstLimitGlobalConfig;
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
        bitNum = (localPort == CPSS_CPU_PORT_NUM_CNS) ? 12 : localPort;
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    }

    rc =  prvCpssHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,bitNum,1,&value);

    if(rc != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPortTxdmaBurstLimitEnableGet function
* @endinternal
*
* @brief   Gets the current status for a given port of TXDMA burst limit thresholds use.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxdmaBurstLimitEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxdmaBurstLimitEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortTxdmaBurstLimitEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxdmaBurstLimitThresholdsSet function
* @endinternal
*
* @brief   Sets the TXDMA burst limit thresholds for a given port.
*         The TxDMA supports three throttling levels: Normal, Slow and OFF.
*         The levels are controlled using two thresholds (almostFullThreshold and
*         fullThreshold) measuring the FIFOs fill level.
*         Normal - the Transmit Queue scheduler is not throttled.
*         Slow - the Transmit Queue scheduler is throttled.
*         OFF - the Transmit Queue scheduler is paused.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] almostFullThreshold      - almost full threshold in Bytes (0..0x3FFC0).
* @param[in] fullThreshold            - full threshold in Bytes (0..0x3FFC0).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of almostFullThreshold and fullThreshold fields in hardware:
*       - Lion and Lion2 devices: 64 Bytes.
*       - Bobcat2, Caelum, Bobcat3, Aldrin, AC3X devices: 128 Bytes.
*       The value is rounded down before write to HW.
*
*/
static GT_STATUS internal_cpssDxChPortTxdmaBurstLimitThresholdsSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32                 almostFullThreshold,
    IN GT_U32                 fullThreshold
)
{
    GT_U32  regAddr;
    GT_U32  value;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32  maxThreshold;
    GT_U32  thresholdResolution;
    GT_U32  thresholdFieldLength;
    GT_U32  portRxdmaNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXDMA_NUM_GET_MAC(devNum,portNum,portRxdmaNum);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        maxThreshold = 0x3FFF80;
        thresholdResolution = 7;
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            thresholdFieldLength = 16;
        }
        else
        {
            thresholdFieldLength = 15;
        }
        regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).txDMAPerSCDMAConfigs.burstLimiterSCDMA[portRxdmaNum];
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxdmaNum);
    }
    else
    {
        maxThreshold = 0x3FFC0;
        thresholdResolution = 6;
        thresholdFieldLength = 12;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaPortBurstLimitThresholds[portNum];
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    }

    if ((almostFullThreshold > maxThreshold) || (fullThreshold > maxThreshold))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    value = (fullThreshold >> thresholdResolution) |
        ((almostFullThreshold >> thresholdResolution) << thresholdFieldLength);

    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0,
                                              2 * thresholdFieldLength, value);
}

/**
* @internal cpssDxChPortTxdmaBurstLimitThresholdsSet function
* @endinternal
*
* @brief   Sets the TXDMA burst limit thresholds for a given port.
*         The TxDMA supports three throttling levels: Normal, Slow and OFF.
*         The levels are controlled using two thresholds (almostFullThreshold and
*         fullThreshold) measuring the FIFOs fill level.
*         Normal - the Transmit Queue scheduler is not throttled.
*         Slow - the Transmit Queue scheduler is throttled.
*         OFF - the Transmit Queue scheduler is paused.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] almostFullThreshold      - almost full threshold in Bytes (0..0x3FFC0).
* @param[in] fullThreshold            - full threshold in Bytes (0..0x3FFC0).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of almostFullThreshold and fullThreshold fields in hardware:
*       - Lion and Lion2 devices: 64 Bytes.
*       - Bobcat2, Caelum, Bobcat3, Aldrin, AC3X devices: 128 Bytes.
*       The value is rounded down before write to HW.
*
*/
GT_STATUS cpssDxChPortTxdmaBurstLimitThresholdsSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32                 almostFullThreshold,
    IN GT_U32                 fullThreshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxdmaBurstLimitThresholdsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, almostFullThreshold, fullThreshold));

    rc = internal_cpssDxChPortTxdmaBurstLimitThresholdsSet(devNum, portNum, almostFullThreshold, fullThreshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, almostFullThreshold, fullThreshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxdmaBurstLimitThresholdsGet function
* @endinternal
*
* @brief   Gets the TXDMA burst limit thresholds for a given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] almostFullThresholdPtr   - (pointer to) almost full threshold in Bytes.
* @param[out] fullThresholdPtr         - (pointer to) full threshold in Bytes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortTxdmaBurstLimitThresholdsGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32                 *almostFullThresholdPtr,
    OUT GT_U32                 *fullThresholdPtr
)
{
    GT_U32  regAddr;
    GT_U32  value;
    GT_STATUS   rc;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32  thresholdResolution;
    GT_U32  thresholdFieldLength;
    GT_U32  thresholdFieldMask;
    GT_U32  portRxdmaNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXDMA_NUM_GET_MAC(devNum,portNum,portRxdmaNum);
    CPSS_NULL_PTR_CHECK_MAC(almostFullThresholdPtr);
    CPSS_NULL_PTR_CHECK_MAC(fullThresholdPtr);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        thresholdResolution = 7;
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            thresholdFieldLength = 16;
        }
        else
        {
            thresholdFieldLength = 15;
        }
        thresholdFieldMask = 0x7FFF;
        regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).txDMAPerSCDMAConfigs.burstLimiterSCDMA[portRxdmaNum];
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxdmaNum);
    }
    else
    {
        thresholdResolution = 6;
        thresholdFieldLength = 12;
        thresholdFieldMask = 0xFFF;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaPortBurstLimitThresholds[portNum];
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    }

    rc =  prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 0,
                                             2 * thresholdFieldLength, &value);
    if(rc != GT_OK)
        return rc;

    *fullThresholdPtr = (value & thresholdFieldMask) << thresholdResolution;
    *almostFullThresholdPtr = ((value >> thresholdFieldLength) & thresholdFieldMask) << thresholdResolution;

    return GT_OK;
}

/**
* @internal cpssDxChPortTxdmaBurstLimitThresholdsGet function
* @endinternal
*
* @brief   Gets the TXDMA burst limit thresholds for a given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] almostFullThresholdPtr   - (pointer to) almost full threshold in Bytes.
* @param[out] fullThresholdPtr         - (pointer to) full threshold in Bytes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxdmaBurstLimitThresholdsGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32                 *almostFullThresholdPtr,
    OUT GT_U32                 *fullThresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxdmaBurstLimitThresholdsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, almostFullThresholdPtr, fullThresholdPtr));

    rc = internal_cpssDxChPortTxdmaBurstLimitThresholdsGet(devNum, portNum, almostFullThresholdPtr, fullThresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, almostFullThresholdPtr, fullThresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

