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
* @file cpssDxChTmGlueAgingAndDelay.c
*
* @brief The CPSS DXCH Aging and Delay Measurements APIs
*
* @version   5
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGlueLog.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueAgingAndDelay.h>

/* the Queue ID can't exceed 16383 */
#define TM_GLUE_AGING_AND_DELAY_QUEUE_ID_CHECK_MAC(_queueId)                \
    if((_queueId) >  16383)                                                 \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);                                                \
    }

/* the Profile ID can't exceed 15 */
#define TM_GLUE_AGING_AND_DELAY_PROFILE_ID_CHECK_MAC(_profileId)            \
    if((_profileId) >  15)                                                  \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);                                                \
    }

/* the Threshold can't exceed 0xFFFFF */
#define TM_GLUE_AGING_AND_DELAY_THRESHOLD_VAL_CHECK_MAC(_thresholdVal)      \
    if((_thresholdVal) >  0xFFFFF)                                          \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);                                                \
    }

/* the Statistical Aging set can't exceed 99 */
#define TM_GLUE_AGING_AND_DELAY_STATISTICAL_AGING_SET_VAL_CHECK_MAC(_statisticalAgingSet)      \
    if((_statisticalAgingSet) >  99)                                        \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                \
    }

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet function
* @endinternal
*
* @brief   Enable dropping of aged packets.
*         Sets if packets that are stored longer than highest aging threhold should
*         be dropped or queue Drop Profile shuld be changed to highest threshold DP.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  Drop
*                                      GT_FALSE: No Drop - change DP
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regData = BOOL2BIT_MAC(enable);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).globalConfig;

    return prvCpssHwPpSetRegField(devNum, regAddr, 1, 1, regData);
}

/**
* @internal cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet function
* @endinternal
*
* @brief   Enable dropping of aged packets.
*         Sets if packets that are stored longer than highest aging threhold should
*         be dropped or queue Drop Profile shuld be changed to highest threshold DP.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  Drop
*                                      GT_FALSE: No Drop - change DP
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet function
* @endinternal
*
* @brief   Get dropping aged packets status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  Drop
*                                      GT_FALSE: No Drop - change DP
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *enablePtr
)
{
    GT_STATUS rc;       /* return code      */
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).globalConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 1, 1, &regData);

    if( rc != GT_OK )
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regData);

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet function
* @endinternal
*
* @brief   Get dropping aged packets status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  Drop
*                                      GT_FALSE: No Drop - change DP
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet function
* @endinternal
*
* @brief   Enable statistics on packets dropped due to aging in
*         TM queue aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  Count
*                                      GT_FALSE: Do not count
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regData = BOOL2BIT_MAC(enable);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).globalConfig;

    return prvCpssHwPpSetRegField(devNum, regAddr, 5, 1, regData);
}

/**
* @internal cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet function
* @endinternal
*
* @brief   Enable statistics on packets dropped due to aging in
*         TM queue aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  Count
*                                      GT_FALSE: Do not count
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet function
* @endinternal
*
* @brief   Get status of enable statistics on packets dropped due to aging in
*         TM queue aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  Count
*                                      GT_FALSE: Do not count
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *enablePtr
)
{
    GT_STATUS rc;       /* return code      */
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).globalConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 5, 1, &regData);

    if( rc != GT_OK )
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regData);

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet function
* @endinternal
*
* @brief   Get status of enable statistics on packets dropped due to aging in
*         TM queue aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  Count
*                                      GT_FALSE: Do not count
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet function
* @endinternal
*
* @brief   Enable Drop of packets transmitted through TM-Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmPort                   - TM port
*                                      (APPLICABLE RANGES: 0..191)
* @param[in] enable                   - if enabled all packet transmitted through
*                                      TM-Port are dropped.
*                                      GT_TRUE  = Dropped Packets
*                                      GT_FALSE = Regular functionality
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tmPort,
    IN  GT_BOOL          enable
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(tmPort>191)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    regData = BOOL2BIT_MAC(enable);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).dropPortAllOutgoingPkts[tmPort];

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, regData);
}

/**
* @internal cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet function
* @endinternal
*
* @brief   Enable Drop of packets transmitted through TM-Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmPort                   - TM port
*                                      (APPLICABLE RANGES: 0..191)
* @param[in] enable                   - if enabled all packet transmitted through
*                                      TM-Port are dropped.
*                                      GT_TRUE  = Dropped Packets
*                                      GT_FALSE = Regular functionality
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tmPort,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tmPort, enable));

    rc = internal_cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet(devNum, tmPort, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tmPort, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet function
* @endinternal
*
* @brief   Return drop status of packets transmitted through TM-Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmPort                   - TM port
*                                      (APPLICABLE RANGES: 0..191)
*
* @param[out] enablePtr                - (pointer to) drop status of all packet
*                                      transmitted through TM-Port
*                                      GT_TRUE  = Dropped Packets
*                                      GT_FALSE = Regular functionality
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tmPort,
    IN  GT_BOOL          *enablePtr
)
{
    GT_STATUS rc;       /* return code      */
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(tmPort>191)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).dropPortAllOutgoingPkts[tmPort];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &regData);

    if( rc != GT_OK )
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regData);

    return rc;
}

/**
* @internal cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet function
* @endinternal
*
* @brief   Return drop status of packets transmitted through TM-Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmPort                   - TM port
*                                      (APPLICABLE RANGES: 0..191)
*
* @param[out] enablePtr                - (pointer to) drop status of all packet
*                                      transmitted through TM-Port
*                                      GT_TRUE  = Dropped Packets
*                                      GT_FALSE = Regular functionality
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tmPort,
    IN  GT_BOOL          *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tmPort, enablePtr));

    rc = internal_cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet(devNum, tmPort, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tmPort, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet function
* @endinternal
*
* @brief   Set Configuration for Dropped Aged Packet Counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] agedPacketCouterQueue    - This is the mask on queue number for
*                                      triggering the aging drop counter
*                                      (APPLICABLE RANGES: 0..16383)
* @param[in] agedPacketCouterQueue    - This is the queue number for triggering
*                                      the aging drop counter
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is increment if the following logic apply:
*       (agedPacketCouterQueueMask & PacketQueue) = agedPacketCouterQueue
*
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           agedPacketCouterQueueMask,
    IN  GT_U32           agedPacketCouterQueue
)
{
    GT_STATUS rc;         /* return code      */
    GT_U32    regAddr;    /* register address */
    GT_U32    regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    TM_GLUE_AGING_AND_DELAY_QUEUE_ID_CHECK_MAC(agedPacketCouterQueueMask);
    TM_GLUE_AGING_AND_DELAY_QUEUE_ID_CHECK_MAC(agedPacketCouterQueue);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regData =  agedPacketCouterQueue;/*bits 0..13*/
    U32_SET_FIELD_MAC(regData, 14, 14 ,agedPacketCouterQueueMask);/*bits 14..27*/

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).dropAgedPktCntrConfig;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 28, regData);

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet function
* @endinternal
*
* @brief   Set Configuration for Dropped Aged Packet Counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] agedPacketCouterQueue    - This is the mask on queue number for
*                                      triggering the aging drop counter
*                                      (APPLICABLE RANGES: 0..16383)
* @param[in] agedPacketCouterQueue    - This is the queue number for triggering
*                                      the aging drop counter
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is increment if the following logic apply:
*       (agedPacketCouterQueueMask & PacketQueue) = agedPacketCouterQueue
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           agedPacketCouterQueueMask,
    IN  GT_U32           agedPacketCouterQueue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, agedPacketCouterQueueMask, agedPacketCouterQueue));

    rc = internal_cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet(devNum, agedPacketCouterQueueMask, agedPacketCouterQueue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, agedPacketCouterQueueMask, agedPacketCouterQueue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet function
* @endinternal
*
* @brief   Get Configuration for Dropped Aged Packet Counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] agedPacketCouterQueueMaskPtr - (pointer to) the mask on queue number
*                                      for triggering the aging drop counter
* @param[out] agedPacketCouterQueuePtr - (pointer to) the queue number for
*                                      triggering the aging drop counter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is increment if the following logic apply:
*       (agedPacketCouterQueueMask & PacketQueue) = agedPacketCouterQueue
*
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet
(
    IN  GT_U8            devNum,
    OUT GT_U32           *agedPacketCouterQueueMaskPtr,
    OUT GT_U32           *agedPacketCouterQueuePtr
)
{
    GT_STATUS rc;         /* return code      */
    GT_U32    regAddr;    /* register address */
    GT_U32    regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(agedPacketCouterQueueMaskPtr);
    CPSS_NULL_PTR_CHECK_MAC(agedPacketCouterQueuePtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).dropAgedPktCntrConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 28, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    *agedPacketCouterQueuePtr = U32_GET_FIELD_MAC(regData,0,14);
    *agedPacketCouterQueueMaskPtr = U32_GET_FIELD_MAC(regData,14,14);

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet function
* @endinternal
*
* @brief   Get Configuration for Dropped Aged Packet Counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] agedPacketCouterQueueMaskPtr - (pointer to) the mask on queue number
*                                      for triggering the aging drop counter
* @param[out] agedPacketCouterQueuePtr - (pointer to) the queue number for
*                                      triggering the aging drop counter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is increment if the following logic apply:
*       (agedPacketCouterQueueMask & PacketQueue) = agedPacketCouterQueue
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet
(
    IN  GT_U8            devNum,
    OUT GT_U32           *agedPacketCouterQueueMaskPtr,
    OUT GT_U32           *agedPacketCouterQueuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, agedPacketCouterQueueMaskPtr, agedPacketCouterQueuePtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet(devNum, agedPacketCouterQueueMaskPtr, agedPacketCouterQueuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, agedPacketCouterQueueMaskPtr, agedPacketCouterQueuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet function
* @endinternal
*
* @brief   Read the counter that counts the number of aged packets that were dropped
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] agedPacketCounterPtr     - (pointer to) the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*       The counter is increment if the following logic apply:
*       (agedPacketCouterQueueMask & PacketQueue) = agedPacketCouterQueue
*
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet
(
    IN  GT_U8            devNum,
    OUT GT_U32           *agedPacketCounterPtr
)
{
    GT_STATUS rc;         /* return code      */
    GT_U32    regAddr;    /* register address */
    GT_U32    regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(agedPacketCounterPtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).dropAgedPktCntr;

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    *agedPacketCounterPtr = regData;

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet function
* @endinternal
*
* @brief   Read the counter that counts the number of aged packets that were dropped
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] agedPacketCounterPtr     - (pointer to) the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*       The counter is increment if the following logic apply:
*       (agedPacketCouterQueueMask & PacketQueue) = agedPacketCouterQueue
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet
(
    IN  GT_U8            devNum,
    OUT GT_U32           *agedPacketCounterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, agedPacketCounterPtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet(devNum, agedPacketCounterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, agedPacketCounterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayTimerResolutionSet function
* @endinternal
*
* @brief   Set aging's timer units resolution.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*                                      timerResolution     - timer resolution in nanosec
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayTimerResolutionSet
(
    IN    GT_U8                    devNum,
    INOUT GT_U32                   *timerResolutionPtr
)
{
    GT_STATUS rc;               /* return code      */
    GT_U32    regAddr;          /* register address */
    GT_U32    initialValue;     /* units resolution in coreClock */
    GT_U32    maxInitialValue;  /* max value that is legal to set to the register */
    double    fCoreClockPeriod;  /* core Clock Period in nanosec */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* "calculate" Core Clock period in nanoseconds */

    fCoreClockPeriod = 1000.0 / PRV_CPSS_PP_MAC(devNum)->coreClock;

    /* calculate Initial Value */
    initialValue = (GT_U32)(*timerResolutionPtr / fCoreClockPeriod +0.5);

    maxInitialValue = 0x1FFFF;

    if(initialValue > maxInitialValue)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    *timerResolutionPtr = (GT_U32)(initialValue * fCoreClockPeriod +0.5);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).queueTimerConfig;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 17, initialValue);

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayTimerResolutionSet function
* @endinternal
*
* @brief   Set aging's timer units resolution.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*                                      timerResolution     - timer resolution in nanosec
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayTimerResolutionSet
(
    IN GT_U8                    devNum,
    INOUT GT_U32                *timerResolutionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayTimerResolutionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, timerResolutionPtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayTimerResolutionSet(devNum, timerResolutionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, timerResolutionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayTimerResolutionGet function
* @endinternal
*
* @brief   Get aging's timer units resolution.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] timerResolutionPtr       - (pointer to) timer resolution in nanosec
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayTimerResolutionGet
(
    IN  GT_U8                    devNum,
    OUT GT_U32                   *timerResolutionPtr
)
{
    GT_STATUS rc;               /* return code      */
    GT_U32    regAddr;          /* register address */
    GT_U32    initialValue;     /* units resolution in coreClock */
    double    fCoreClockPeriod;  /* core Clock Period in nanosec */


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(timerResolutionPtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).queueTimerConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 17, &initialValue);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* "calculate" Core Clock period in nanoseconds */
    fCoreClockPeriod = 1000.0 / (PRV_CPSS_PP_MAC(devNum)->coreClock);

    /* calculate timerResolution */
    *timerResolutionPtr = (GT_U32)(initialValue * fCoreClockPeriod + 0.5);

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayTimerResolutionGet function
* @endinternal
*
* @brief   Get aging's timer units resolution.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] timerResolutionPtr       - (pointer to) timer resolution in nanosec
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayTimerResolutionGet
(
    IN  GT_U8                    devNum,
    OUT GT_U32                   *timerResolutionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayTimerResolutionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, timerResolutionPtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayTimerResolutionGet(devNum, timerResolutionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, timerResolutionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayTimerGet function
* @endinternal
*
* @brief   The aging and delay timer is the reference clock used for calculating
*         the time each packet was stored.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] timerPtr                 - (pointer to) the aging and delay timer
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The timer is incremented by 1 each time the free running counter
*       reach "0".
*       see cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayTimerGet
(
    IN  GT_U8                    devNum,
    OUT GT_U32                   *timerPtr
)
{
    GT_STATUS rc;         /* return code      */
    GT_U32    regAddr;    /* register address */
    GT_U32    regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(timerPtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).queueTimer;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 20, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    *timerPtr = regData;

    return rc;

}

/**
* @internal cpssDxChTmGlueAgingAndDelayTimerGet function
* @endinternal
*
* @brief   The aging and delay timer is the reference clock used for calculating
*         the time each packet was stored.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] timerPtr                 - (pointer to) the aging and delay timer
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The timer is incremented by 1 each time the free running counter
*       reach "0".
*       see cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayTimerGet
(
    IN  GT_U8                    devNum,
    OUT GT_U32                   *timerPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayTimerGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, timerPtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayTimerGet(devNum, timerPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, timerPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet function
* @endinternal
*
* @brief   Configure the Aging Profile Thresholds Table
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - profileId
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] thresholdsPtr            - (pointer to) thresholds values.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The thresholds units are set in cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               profileId,
    IN CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC  *thresholdsPtr
)
{
    GT_U32    hwValue[2];    /* hw data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    TM_GLUE_AGING_AND_DELAY_PROFILE_ID_CHECK_MAC(profileId);
    CPSS_NULL_PTR_CHECK_MAC(thresholdsPtr);
    TM_GLUE_AGING_AND_DELAY_THRESHOLD_VAL_CHECK_MAC(thresholdsPtr->threshold0);
    TM_GLUE_AGING_AND_DELAY_THRESHOLD_VAL_CHECK_MAC(thresholdsPtr->threshold1);
    TM_GLUE_AGING_AND_DELAY_THRESHOLD_VAL_CHECK_MAC(thresholdsPtr->threshold2);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    hwValue[0]=0;
    hwValue[1]=0;

    U32_SET_FIELD_IN_ENTRY_MAC(hwValue,0,20,thresholdsPtr->threshold0);  /*bits 0..19*/
    U32_SET_FIELD_IN_ENTRY_MAC(hwValue,20,20,thresholdsPtr->threshold1); /*bits 20..39*/
    U32_SET_FIELD_IN_ENTRY_MAC(hwValue,40,20,thresholdsPtr->threshold2); /*bits 40..59*/

    return prvCpssDxChWriteTableEntry(devNum,
                                      CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_PROFILE_THESHOLDS_E,
                                      profileId,
                                      hwValue);
}

/**
* @internal cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet function
* @endinternal
*
* @brief   Configure the Aging Profile Thresholds Table
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - profileId
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] thresholdsPtr            - (pointer to) thresholds values.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The thresholds units are set in cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               profileId,
    IN CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC  *thresholdsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileId, thresholdsPtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(devNum, profileId, thresholdsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileId, thresholdsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet function
* @endinternal
*
* @brief   Get the Aging Profile Thresholds Table configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - profileId
*                                      (APPLICABLE RANGES: 0..15)
*
* @param[out] thresholdsPtr            - (pointer to) thresholds values.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The thresholds units are set in cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              profileId,
    OUT CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC *thresholdsPtr
)
{
    GT_STATUS rc;        /* return code */
    GT_U32    hwValue[2]; /* hw data     */
    GT_U32    tempVal;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    TM_GLUE_AGING_AND_DELAY_PROFILE_ID_CHECK_MAC(profileId);
    CPSS_NULL_PTR_CHECK_MAC(thresholdsPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_PROFILE_THESHOLDS_E,
                                   profileId,
                                   hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    U32_GET_FIELD_IN_ENTRY_MAC(hwValue,0,20,tempVal);     /*bits 0..19*/
    thresholdsPtr->threshold0 = tempVal;
    U32_GET_FIELD_IN_ENTRY_MAC(hwValue,20,20,tempVal);     /*bits 20..39*/
    thresholdsPtr->threshold1 = tempVal;
    U32_GET_FIELD_IN_ENTRY_MAC(hwValue,40,20,tempVal);     /*bits 40..59*/
    thresholdsPtr->threshold2 = tempVal;

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet function
* @endinternal
*
* @brief   Get the Aging Profile Thresholds Table configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - profileId
*                                      (APPLICABLE RANGES: 0..15)
*
* @param[out] thresholdsPtr            - (pointer to) thresholds values.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The thresholds units are set in cpssDxChTmGlueAgingAndDelayTimerResolutionSet
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              profileId,
    OUT CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC *thresholdsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileId, thresholdsPtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet(devNum, profileId, thresholdsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileId, thresholdsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet function
* @endinternal
*
* @brief   Configure Queue Aging Profile Table that Maps a Traffic Manager Queue ID
*         to an aging profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - Queue Id to configure
*                                      (APPLICABLE RANGES: 0..16383)
* @param[in] profileId                - Profile Id value to set for the given queueId
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       queueId,
    IN GT_U32       profileId
)
{
    GT_U32    entryIndex;
    GT_U32    fieldOffset;
    GT_U32    fieldWordNum=0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    TM_GLUE_AGING_AND_DELAY_QUEUE_ID_CHECK_MAC(queueId);
    TM_GLUE_AGING_AND_DELAY_PROFILE_ID_CHECK_MAC(profileId);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    entryIndex = queueId/16;
    fieldOffset = queueId%16*4;
    if(fieldOffset > 31)
    {
        fieldWordNum = 1;
        fieldOffset = (fieldOffset-32);
    }

    return prvCpssDxChWriteTableEntryField(devNum,
                                          CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_QUEUE_PROFILE_E,
                                          entryIndex,
                                          fieldWordNum,
                                          fieldOffset,
                                          4,
                                          profileId);
}

/**
* @internal cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet function
* @endinternal
*
* @brief   Configure Queue Aging Profile Table that Maps a Traffic Manager Queue ID
*         to an aging profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - Queue Id to configure
*                                      (APPLICABLE RANGES: 0..16383)
* @param[in] profileId                - Profile Id value to set for the given queueId
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       queueId,
    IN GT_U32       profileId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueId, profileId));

    rc = internal_cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet(devNum, queueId, profileId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueId, profileId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet function
* @endinternal
*
* @brief   Get Queue Aging Profile Table configuration that Maps a Traffic Manager
*         Queue ID to an aging profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - Queue Id to configure
*                                      (APPLICABLE RANGES: 0..16383)
*
* @param[out] profileIdPtr             - (pointer to)
*                                      Profile Id value to set for the given queueId
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       queueId,
    OUT GT_U32       *profileIdPtr
)
{
    GT_STATUS rc;        /* return code */
    GT_U32    entryIndex;
    GT_U32    fieldOffset;
    GT_U32    fieldWordNum;
    GT_U32    hwValue;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    TM_GLUE_AGING_AND_DELAY_QUEUE_ID_CHECK_MAC(queueId);
    CPSS_NULL_PTR_CHECK_MAC(profileIdPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    entryIndex = queueId/16;
    fieldOffset = queueId%16*4;
    fieldWordNum = 0;
    if(fieldOffset > 31)
    {
        fieldWordNum = 1;
        fieldOffset = (fieldOffset-32);
    }

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_QUEUE_PROFILE_E,
                                        entryIndex,
                                        fieldWordNum,
                                        fieldOffset,
                                        4,
                                        &hwValue);
    if(rc != GT_OK)
        return rc;

    *profileIdPtr = hwValue;
    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet function
* @endinternal
*
* @brief   Get Queue Aging Profile Table configuration that Maps a Traffic Manager
*         Queue ID to an aging profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - Queue Id to configure
*                                      (APPLICABLE RANGES: 0..16383)
*
* @param[out] profileIdPtr             - (pointer to)
*                                      Profile Id value to set for the given queueId
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       queueId,
    OUT GT_U32       *profileIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueId, profileIdPtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet(devNum, queueId, profileIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueId, profileIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayCounterQueueIdSet function
* @endinternal
*
* @brief   Map one of aging and delay counter sets to a queuedId number.
*         The counters in the set perform delay and aging statistics on the packets
*         queued to the configured Queue-ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - Aging and delay counter set index
*                                      (APPLICABLE RANGES: 0..99)
* @param[in] queueId                  - queue id
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayCounterQueueIdSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   counterSetIndex,
    IN GT_U32                   queueId
)
{
    GT_STATUS rc;         /* return code      */
    GT_U32    regAddr;    /* register address */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    TM_GLUE_AGING_AND_DELAY_STATISTICAL_AGING_SET_VAL_CHECK_MAC(counterSetIndex);
    TM_GLUE_AGING_AND_DELAY_QUEUE_ID_CHECK_MAC(queueId);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).statisticalAgingQueueID[counterSetIndex];

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 14, queueId);

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayCounterQueueIdSet function
* @endinternal
*
* @brief   Map one of aging and delay counter sets to a queuedId number.
*         The counters in the set perform delay and aging statistics on the packets
*         queued to the configured Queue-ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - Aging and delay counter set index
*                                      (APPLICABLE RANGES: 0..99)
* @param[in] queueId                  - queue id
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayCounterQueueIdSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   counterSetIndex,
    IN GT_U32                   queueId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayCounterQueueIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterSetIndex, queueId));

    rc = internal_cpssDxChTmGlueAgingAndDelayCounterQueueIdSet(devNum, counterSetIndex, queueId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterSetIndex, queueId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayCounterQueueIdGet function
* @endinternal
*
* @brief   Get Mapping of aging and delay counter set to a queuedId number.
*         The counters in the set perform delay and aging statistics on the packets
*         queued to the configured Queue-ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - Aging and delay counter set index
*                                      (APPLICABLE RANGES: 0..99)
*
* @param[out] queueIdPtr               - (pointer to) queue id
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayCounterQueueIdGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   counterSetIndex,
    OUT GT_U32                   *queueIdPtr
)
{
    GT_STATUS rc;         /* return code      */
    GT_U32    regAddr;    /* register address */
    GT_U32    regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    TM_GLUE_AGING_AND_DELAY_STATISTICAL_AGING_SET_VAL_CHECK_MAC(counterSetIndex);
    CPSS_NULL_PTR_CHECK_MAC(queueIdPtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).statisticalAgingQueueID[counterSetIndex];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 14, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    *queueIdPtr = regData;

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayCounterQueueIdGet function
* @endinternal
*
* @brief   Get Mapping of aging and delay counter set to a queuedId number.
*         The counters in the set perform delay and aging statistics on the packets
*         queued to the configured Queue-ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - Aging and delay counter set index
*                                      (APPLICABLE RANGES: 0..99)
*
* @param[out] queueIdPtr               - (pointer to) queue id
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayCounterQueueIdGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   counterSetIndex,
    OUT GT_U32                   *queueIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayCounterQueueIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterSetIndex, queueIdPtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayCounterQueueIdGet(devNum, counterSetIndex, queueIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterSetIndex, queueIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChTmGlueAgingAndDelayCountersGet function
* @endinternal
*
* @brief   Read all the counters of aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - aging and delay counter set
*                                      (APPLICABLE RANGES: 0..99)
*
* @param[out] agingCountersPtr         - (pointer to) aging and delay counters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayCountersGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      counterSetIndex,
    OUT CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC *agingCountersPtr
)
{
    GT_STATUS rc;         /* return code      */
    GT_U32    regAddr;    /* register address */
    GT_U32    regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    TM_GLUE_AGING_AND_DELAY_STATISTICAL_AGING_SET_VAL_CHECK_MAC(counterSetIndex);
    CPSS_NULL_PTR_CHECK_MAC(agingCountersPtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).statisticalAgingPktCntr[counterSetIndex];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 24, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    agingCountersPtr->packetCounter = regData;

    /* LSB Aging Octet Counter */
    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).statisticalAgingOctetCntrsLsbShadow;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 32, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    agingCountersPtr->octetCounter.l[0] = regData;

    /* MSB Aging Octet Counter */
    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).statisticalAgingOctetCntrsMsbShadow;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 8, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    agingCountersPtr->octetCounter.l[1] = regData;

    /* LSB Aging Timer Counter */
    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).statisticalAgingTimeCntrsLsbShadow;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 32, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    agingCountersPtr->latencyTimeCounter.l[0] = regData;

    /* MSB Aging Timer Counter */
    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).statisticalAgingTimeCntrsMsbShadow;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 12, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    agingCountersPtr->latencyTimeCounter.l[1] = regData;

    /* Aging Aged Out Pkt Counter */
    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).statisticalAgingAgedOutPktCntrsShadow;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 24, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    agingCountersPtr->agedPacketCounter = regData;

    /* Max Age Time Counter */
    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).statisticalMaxAgeTimeShadow;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 20, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    agingCountersPtr->maxLatencyTime = regData;

    /* Min Age Time Counter */
    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).statisticalMinAgeTimeShadow;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 20, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    agingCountersPtr->minLatencyTime = regData;

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayCountersGet function
* @endinternal
*
* @brief   Read all the counters of aging and delay counter set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - aging and delay counter set
*                                      (APPLICABLE RANGES: 0..99)
*
* @param[out] agingCountersPtr         - (pointer to) aging and delay counters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayCountersGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      counterSetIndex,
    OUT CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC *agingCountersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterSetIndex, agingCountersPtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayCountersGet(devNum, counterSetIndex, agingCountersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterSetIndex, agingCountersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet function
* @endinternal
*
* @brief   Read the counter that counts the number of aged out packets
*         that should have been transmiited from tm-port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmPort                   - TM port
*                                      (APPLICABLE RANGES: 0..191)
*
* @param[out] agedOutPacketCounterPtr  - (pointer to) Aged Out Packet Counter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*
*/
static GT_STATUS internal_cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tmPort,
    OUT GT_U32           *agedOutPacketCounterPtr
)
{
    GT_STATUS rc;         /* return code      */
    GT_U32    regAddr;    /* register address */
    GT_U32    regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(agedOutPacketCounterPtr);

    if(tmPort>191)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(devNum).perPortAgedOutPktCntr[tmPort];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 24, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    *agedOutPacketCounterPtr = regData;

    return rc;
}

/**
* @internal cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet function
* @endinternal
*
* @brief   Read the counter that counts the number of aged out packets
*         that should have been transmiited from tm-port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmPort                   - TM port
*                                      (APPLICABLE RANGES: 0..191)
*
* @param[out] agedOutPacketCounterPtr  - (pointer to) Aged Out Packet Counter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear On Read Counter
*
*/
GT_STATUS cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tmPort,
    OUT GT_U32           *agedOutPacketCounterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tmPort, agedOutPacketCounterPtr));

    rc = internal_cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet(devNum, tmPort, agedOutPacketCounterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tmPort, agedOutPacketCounterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


