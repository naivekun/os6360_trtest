
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
* @file cpssDxChBrgL2Ecmp.c
*
* @brief L2 ECMP facility CPSS DxCh implementation.
*
*
* @version   14
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Ecmp.h>
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunk.h>

/* check that the index not over it's boundaries */
/* Note that the l2ecmp get 'only' lower part of the 'Hosting table'
  the other part is for the trunk usage */
#define ECMP_MEMBER_INDEX_CHECK_MAC(_devNum,_index)    \
    if((_index) >= ((PRV_CPSS_DXCH_PP_MAC(_devNum)->bridge.l2EcmpNumMembers - \
                     PRV_CPSS_DXCH_PP_MAC(_devNum)->bridge.l2EcmpNumMembers_usedByTrunk)))   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/**
* @internal internal_cpssDxChBrgL2EcmpEnableSet function
* @endinternal
*
* @brief   Globally enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - L2 ECMP status:
*                                      GT_TRUE: enable L2 ECMP
*                                      GT_FALSE: disable L2 ECMP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2EcmpEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_U32      regAddr;
    GT_U32      hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).ePort.ePortGlobalConfig;

    hwData = BOOL2BIT_MAC(enable);
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, hwData);
}

/**
* @internal cpssDxChBrgL2EcmpEnableSet function
* @endinternal
*
* @brief   Globally enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - L2 ECMP status:
*                                      GT_TRUE: enable L2 ECMP
*                                      GT_FALSE: disable L2 ECMP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2EcmpEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgL2EcmpEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgL2EcmpEnableGet function
* @endinternal
*
* @brief   Get the global enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) L2 ECMP  status:
*                                      GT_TRUE : L2 ECMP is enabled
*                                      GT_FALSE: L2 ECMP is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2EcmpEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      regAddr;
    GT_U32      hwData;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).ePort.ePortGlobalConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &hwData);
    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgL2EcmpEnableGet function
* @endinternal
*
* @brief   Get the global enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) L2 ECMP  status:
*                                      GT_TRUE : L2 ECMP is enabled
*                                      GT_FALSE: L2 ECMP is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2EcmpEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgL2EcmpEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgL2EcmpIndexBaseEportSet function
* @endinternal
*
* @brief   Defines the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ecmpIndexBaseEport       - the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2EcmpIndexBaseEportSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      ecmpIndexBaseEport
)
{
    GT_STATUS rc;
    GT_U32    regAddr;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    if(ecmpIndexBaseEport > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPLTTIndexBaseEPort;
    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        rc = prvCpssHwPpSetRegField(devNum,regAddr ,0 ,15 ,ecmpIndexBaseEport);
    }
    else
    {
        rc = prvCpssHwPpSetRegField(devNum,regAddr ,0 ,13 ,ecmpIndexBaseEport);
    }

    return rc;
}

/**
* @internal cpssDxChBrgL2EcmpIndexBaseEportSet function
* @endinternal
*
* @brief   Defines the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ecmpIndexBaseEport       - the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpIndexBaseEportSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      ecmpIndexBaseEport
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2EcmpIndexBaseEportSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ecmpIndexBaseEport));

    rc = internal_cpssDxChBrgL2EcmpIndexBaseEportSet(devNum, ecmpIndexBaseEport);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ecmpIndexBaseEport));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgL2EcmpIndexBaseEportGet function
* @endinternal
*
* @brief   Return the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] ecmpIndexBaseEportPtr    - (pointer to) the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2EcmpIndexBaseEportGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *ecmpIndexBaseEportPtr
)
{
    GT_STATUS   rc;
    GT_U32      hwData;
    GT_U32      regAddr;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(ecmpIndexBaseEportPtr);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPLTTIndexBaseEPort;
    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        rc = prvCpssHwPpGetRegField(devNum,regAddr ,0 ,15 ,&hwData);
    }
    else
    {
        rc = prvCpssHwPpGetRegField(devNum,regAddr ,0 ,13 ,&hwData);
    }

    *ecmpIndexBaseEportPtr = hwData;

    return rc;
}

/**
* @internal cpssDxChBrgL2EcmpIndexBaseEportGet function
* @endinternal
*
* @brief   Return the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] ecmpIndexBaseEportPtr    - (pointer to) the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpIndexBaseEportGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *ecmpIndexBaseEportPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2EcmpIndexBaseEportGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ecmpIndexBaseEportPtr));

    rc = internal_cpssDxChBrgL2EcmpIndexBaseEportGet(devNum, ecmpIndexBaseEportPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ecmpIndexBaseEportPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgL2EcmpLttTableSet function
* @endinternal
*
* @brief   Set L2 ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index
* @param[in] ecmpLttInfoPtr           - (pointer to) L2 ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range ecmpStartIndex or ecmpNumOfPaths
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The index to the LTT is the result of Target ePort - Base ePort.
*       Base ePort may be configured by cpssDxChBrgL2EcmpIndexBaseEportSet.
*
*/
static GT_STATUS internal_cpssDxChBrgL2EcmpLttTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
)
{
    GT_STATUS rc;           /* return code */
    GT_U32    hwFormat = 0; /* data of LTT entry in HW format */
    GT_U32          numBits_L2_ECMP_Start_Index;/* number of bits in <L2_ECMP_Start_Index> */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(ecmpLttInfoPtr);

    if(
       (ecmpLttInfoPtr->ecmpNumOfPaths >= (BIT_12 + 1)) ||
       (ecmpLttInfoPtr->ecmpNumOfPaths == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* check that combination of start index and 'num path' not exceed the members table */
    ECMP_MEMBER_INDEX_CHECK_MAC(devNum,
        (ecmpLttInfoPtr->ecmpStartIndex + ecmpLttInfoPtr->ecmpNumOfPaths));

    numBits_L2_ECMP_Start_Index =
        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 14 : 13;

    U32_SET_FIELD_MAC(hwFormat,2,numBits_L2_ECMP_Start_Index,ecmpLttInfoPtr->ecmpStartIndex);
    /* The number of paths in the ECMP block.
       0x0 = 1 path.
       0x1 = 2 paths.*/
    U32_SET_FIELD_MAC(hwFormat,2+numBits_L2_ECMP_Start_Index,12,(ecmpLttInfoPtr->ecmpNumOfPaths - 1));
    U32_SET_FIELD_MAC(hwFormat,0,1,ecmpLttInfoPtr->ecmpEnable);
    U32_SET_FIELD_MAC(hwFormat,1,1,BOOL2BIT_MAC(ecmpLttInfoPtr->ecmpRandomPathEnable));

    if((PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).ePort != 0) &&
       (index >= PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).ePort))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "tableType[%d] index[%d] is out of range [0..%d]",
            CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_LTT_E, index,
            PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).ePort-1);
    }
    rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                             CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_LTT_E,
                                             index,
                                             &hwFormat);
    return rc;
}

/**
* @internal cpssDxChBrgL2EcmpLttTableSet function
* @endinternal
*
* @brief   Set L2 ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index
* @param[in] ecmpLttInfoPtr           - (pointer to) L2 ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range ecmpStartIndex or ecmpNumOfPaths
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The index to the LTT is the result of Target ePort - Base ePort.
*       Base ePort may be configured by cpssDxChBrgL2EcmpIndexBaseEportSet.
*
*/
GT_STATUS cpssDxChBrgL2EcmpLttTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2EcmpLttTableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, ecmpLttInfoPtr));

    rc = internal_cpssDxChBrgL2EcmpLttTableSet(devNum, index, ecmpLttInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, ecmpLttInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgL2EcmpLttTableGet function
* @endinternal
*
* @brief   Get L2 ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index
*
* @param[out] ecmpLttInfoPtr           - (pointer to) L2 ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The index to the LTT is the result of Target ePort - Base ePort.
*       Base ePort may be configured by cpssDxChBrgL2EcmpIndexBaseEportSet.
*
*/
static GT_STATUS internal_cpssDxChBrgL2EcmpLttTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
)
{
    GT_STATUS rc;               /* return code          */
    GT_U32    hwFormat;         /* data of LTT entry in HW format */
    GT_U32    numBits_L2_ECMP_Start_Index;/* number of bits in <L2_ECMP_Start_Index> */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ecmpLttInfoPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    if((PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).ePort != 0) &&
       (index >= PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).ePort))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "tableType[%d] index[%d] is out of range [0..%d]",
            CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_LTT_E, index,
            PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).ePort-1);
    }
    rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                         CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                         CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_LTT_E,
                                         index,
                                         &hwFormat);

    numBits_L2_ECMP_Start_Index =
        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 14 : 13;
    ecmpLttInfoPtr->ecmpStartIndex = U32_GET_FIELD_MAC(hwFormat,2,numBits_L2_ECMP_Start_Index);
     /* The number of paths in the ECMP block.
       0x0 = 1 path.
       0x1 = 2 paths.*/
    ecmpLttInfoPtr->ecmpNumOfPaths = U32_GET_FIELD_MAC(hwFormat,2+numBits_L2_ECMP_Start_Index,12) + 1;
    ecmpLttInfoPtr->ecmpEnable     = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormat,0,1));
    ecmpLttInfoPtr->ecmpRandomPathEnable     = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwFormat,1,1));

    return rc;
}

/**
* @internal cpssDxChBrgL2EcmpLttTableGet function
* @endinternal
*
* @brief   Get L2 ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index
*
* @param[out] ecmpLttInfoPtr           - (pointer to) L2 ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The index to the LTT is the result of Target ePort - Base ePort.
*       Base ePort may be configured by cpssDxChBrgL2EcmpIndexBaseEportSet.
*
*/
GT_STATUS cpssDxChBrgL2EcmpLttTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2EcmpLttTableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, ecmpLttInfoPtr));

    rc = internal_cpssDxChBrgL2EcmpLttTableGet(devNum, index, ecmpLttInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, ecmpLttInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgL2EcmpTableSet function
* @endinternal
*
* @brief   Set L2 ECMP entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the ECMP entry index
* @param[in] ecmpEntryPtr             - (pointer to) L2 ECMP entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Index according to index from L2 ECMP LTT
*
*/
static GT_STATUS internal_cpssDxChBrgL2EcmpTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC     *ecmpEntryPtr
)
{
    GT_STATUS rc;
    GT_U32    hwFormat=0;
    GT_U32    numBits_target_eport_phy_port; /*number of bits in <Target_ePort_phy_port> */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(ecmpEntryPtr);

    /* check that index not exceed the table */
    ECMP_MEMBER_INDEX_CHECK_MAC(devNum,index);

    if((ecmpEntryPtr->targetHwDevice > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum)) ||
       (ecmpEntryPtr->targetEport  > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    numBits_target_eport_phy_port =
        PRV_CPSS_SIP_6_CHECK_MAC(devNum)    ? 13 :
        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 14 : 13;

    U32_SET_FIELD_MAC(hwFormat,0 ,10,ecmpEntryPtr->targetHwDevice);
    U32_SET_FIELD_MAC(hwFormat,10,numBits_target_eport_phy_port,ecmpEntryPtr->targetEport);

    rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                             CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E,
                                             index,
                                             &hwFormat);
    return rc;
}

/**
* @internal cpssDxChBrgL2EcmpTableSet function
* @endinternal
*
* @brief   Set L2 ECMP entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the ECMP entry index
* @param[in] ecmpEntryPtr             - (pointer to) L2 ECMP entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Index according to index from L2 ECMP LTT
*
*/
GT_STATUS cpssDxChBrgL2EcmpTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC     *ecmpEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2EcmpTableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, ecmpEntryPtr));

    rc = internal_cpssDxChBrgL2EcmpTableSet(devNum, index, ecmpEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, ecmpEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgL2EcmpTableGet function
* @endinternal
*
* @brief   Get L2 ECMP entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the ECMP entry index
*
* @param[out] ecmpEntryPtr             - (pointer to) L2 ECMP entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Index according to index from L2 ECMP LTT
*
*/
static GT_STATUS internal_cpssDxChBrgL2EcmpTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC     *ecmpEntryPtr
)
{
    GT_STATUS rc;               /* return code          */
    GT_U32    hwFormat=0;
    GT_U32    numBits_target_eport_phy_port; /*number of bits in <Target_ePort_phy_port> */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ecmpEntryPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check that index not exceed the table */
    ECMP_MEMBER_INDEX_CHECK_MAC(devNum,index);

    rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                         CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                         CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E,
                                         index,
                                         &hwFormat);
    if(rc != GT_OK)
        return rc;

    numBits_target_eport_phy_port =
        PRV_CPSS_SIP_6_CHECK_MAC(devNum)    ? 13 :
        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 14 : 13;

    ecmpEntryPtr->targetHwDevice = U32_GET_FIELD_MAC(hwFormat,0,10);
    ecmpEntryPtr->targetEport  = U32_GET_FIELD_MAC(hwFormat,10,numBits_target_eport_phy_port);

    return rc;
}

/**
* @internal cpssDxChBrgL2EcmpTableGet function
* @endinternal
*
* @brief   Get L2 ECMP entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the ECMP entry index
*
* @param[out] ecmpEntryPtr             - (pointer to) L2 ECMP entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Index according to index from L2 ECMP LTT
*
*/
GT_STATUS cpssDxChBrgL2EcmpTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC     *ecmpEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2EcmpTableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, ecmpEntryPtr));

    rc = internal_cpssDxChBrgL2EcmpTableGet(devNum, index, ecmpEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, ecmpEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgL2EcmpMemberSelectionModeSet function
* @endinternal
*
* @brief   Set mode used to calculate the Index of the secondary ePort (L2 ECMP member)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] selectionMode            - member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The selection mode configuration may be changed by:
*       - cpssDxChTrunkHashNumBitsSet.
*       - cpssDxChTrunkHashGlobalModeSet.
*       - cpssDxChTrunkInit.
*       - cpssDxChTrunkHashCrcParametersSet.
*
*/
static GT_STATUS internal_cpssDxChBrgL2EcmpMemberSelectionModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_MEMBER_SELECTION_MODE_ENT      selectionMode
)
{
    GT_U32    startBit;
    GT_U32    numOfBits;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    switch(selectionMode)
    {
        case CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E:
            startBit = 0;
            numOfBits = 6;
            break;
        case CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E:
            startBit  = 6;
            numOfBits = 6;
            break;
        case CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E:
            startBit  = 0;
            numOfBits = 12;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return cpssDxChTrunkHashNumBitsSet(devNum,CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E,startBit,numOfBits);
}

/**
* @internal cpssDxChBrgL2EcmpMemberSelectionModeSet function
* @endinternal
*
* @brief   Set mode used to calculate the Index of the secondary ePort (L2 ECMP member)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] selectionMode            - member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The selection mode configuration may be changed by:
*       - cpssDxChTrunkHashNumBitsSet.
*       - cpssDxChTrunkHashGlobalModeSet.
*       - cpssDxChTrunkInit.
*       - cpssDxChTrunkHashCrcParametersSet.
*
*/
GT_STATUS cpssDxChBrgL2EcmpMemberSelectionModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_MEMBER_SELECTION_MODE_ENT      selectionMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2EcmpMemberSelectionModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, selectionMode));

    rc = internal_cpssDxChBrgL2EcmpMemberSelectionModeSet(devNum, selectionMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, selectionMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgL2EcmpMemberSelectionModeGet function
* @endinternal
*
* @brief   Get mode used to calculate the Index of the secondary ePort (L2 ECMP member)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] selectionModePtr         - (pointer to) member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2EcmpMemberSelectionModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_MEMBER_SELECTION_MODE_ENT     *selectionModePtr
)
{
    GT_STATUS   rc;
    GT_U32    startBit;
    GT_U32    numOfBits;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(selectionModePtr);

    rc = prvCpssDxChTrunkHashBitsSelectionGet(devNum,CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E,&startBit,&numOfBits);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(startBit == 0)
    {
        if(numOfBits == 6)
        {
            *selectionModePtr = CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E;
        }
        else if((numOfBits == 12))
        {
            *selectionModePtr = CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    if(startBit == 6 && numOfBits == 6)
    {
        *selectionModePtr = CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgL2EcmpMemberSelectionModeGet function
* @endinternal
*
* @brief   Get mode used to calculate the Index of the secondary ePort (L2 ECMP member)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] selectionModePtr         - (pointer to) member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpMemberSelectionModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_MEMBER_SELECTION_MODE_ENT     *selectionModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2EcmpMemberSelectionModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, selectionModePtr));

    rc = internal_cpssDxChBrgL2EcmpMemberSelectionModeGet(devNum, selectionModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, selectionModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

