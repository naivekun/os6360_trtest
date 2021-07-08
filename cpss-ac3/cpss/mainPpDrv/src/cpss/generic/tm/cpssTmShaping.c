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
* @file cpssTmShaping.c
*
* @brief TM Shaping APIs
*
* @version   2
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <tm_shaping.h>
#include <tm_nodes_update.h>



/**
* @internal internal_cpssTmShapingProfileCreate function
* @endinternal
*
* @brief   Create a Shaping Profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Level to configure the profile for.
* @param[in] profileStrPtr            - Shaping profile configuration struct pointer.
*
* @param[out] profileIndPtr            - (pointer to) The created Shaping profile index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_FULL                  - on full allocation of Shaping profile table.
* @retval GT_NOT_INITIALIZED       - on not configured periodic update rate for the given level
* @retval GT_BAD_SIZE              - on too large min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmShapingProfileCreate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr,
    OUT GT_U32                                  *profileIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    struct tm_shaping_profile_params prf;
    uint32_t prof_index;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );
    CPSS_NULL_PTR_CHECK_MAC(profileStrPtr);
    CPSS_NULL_PTR_CHECK_MAC(profileIndPtr);

    prf.cir_bw = profileStrPtr->cirBw;
    prf.cbs = profileStrPtr->cbs;
    prf.eir_bw = profileStrPtr->eirBw;
    prf.ebs = profileStrPtr->ebs;
    ret = tm_create_shaping_profile(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    level,
                                    &prf,
                                    &prof_index);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *profileIndPtr = prof_index;
    return rc;
}

/**
* @internal cpssTmShapingProfileCreate function
* @endinternal
*
* @brief   Create a Shaping Profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Level to configure the profile for.
* @param[in] profileStrPtr            - Shaping profile configuration struct pointer.
*
* @param[out] profileIndPtr            - (pointer to) The created Shaping profile index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_FULL                  - on full allocation of Shaping profile table.
* @retval GT_NOT_INITIALIZED       - on not configured periodic update rate for the given level
* @retval GT_BAD_SIZE              - on too large min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmShapingProfileCreate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr,
    OUT GT_U32                                  *profileIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmShapingProfileCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, level, profileStrPtr, profileIndPtr));

    rc = internal_cpssTmShapingProfileCreate(devNum, level, profileStrPtr, profileIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, level, profileStrPtr, profileIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmShapingProfileDelete function
* @endinternal
*
* @brief   Delete a Shaping Profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] profileInd               - An index of the Shaping profile.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmShapingProfileDelete
(
    IN  GT_U8   devNum,
    IN  GT_U32  profileInd
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );
    ret = tm_delete_shaping_profile(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, profileInd);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

/**
* @internal cpssTmShapingProfileDelete function
* @endinternal
*
* @brief   Delete a Shaping Profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] profileInd               - An index of the Shaping profile.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmShapingProfileDelete
(
    IN  GT_U8   devNum,
    IN  GT_U32  profileInd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmShapingProfileDelete);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileInd));

    rc = internal_cpssTmShapingProfileDelete(devNum, profileInd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileInd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmShapingProfileRead function
* @endinternal
*
* @brief   Read Shaping profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Level to read the profile for.
* @param[in] profileInd               - An index of the Shaping profile.
*
* @param[out] profileStrPtr            - (pointer to) Shaping profile configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The cirBw and eirBw may deviate from the originally configured
*       by cpssTmShapingProfileCreate or cpssTmShapingProfileUpdate by the
*       bw accuracy parameter provisioned in cpssTmSchedPeriodicSchemeConfig API.
*
*/
static GT_STATUS internal_cpssTmShapingProfileRead
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  GT_U32                                  profileInd,
    OUT CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    struct tm_shaping_profile_params prf;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );
    CPSS_NULL_PTR_CHECK_MAC(profileStrPtr);

    ret = tm_read_shaping_profile(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                  level,
                                  profileInd,
                                  &prf);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    profileStrPtr->cirBw = prf.cir_bw;
    profileStrPtr->cbs = prf.cbs;
    profileStrPtr->eirBw = prf.eir_bw;
    profileStrPtr->ebs = prf.ebs;
    return rc;
}

/**
* @internal cpssTmShapingProfileRead function
* @endinternal
*
* @brief   Read Shaping profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Level to read the profile for.
* @param[in] profileInd               - An index of the Shaping profile.
*
* @param[out] profileStrPtr            - (pointer to) Shaping profile configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The cirBw and eirBw may deviate from the originally configured
*       by cpssTmShapingProfileCreate or cpssTmShapingProfileUpdate by the
*       bw accuracy parameter provisioned in cpssTmSchedPeriodicSchemeConfig API.
*
*/
GT_STATUS cpssTmShapingProfileRead
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  GT_U32                                  profileInd,
    OUT CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmShapingProfileRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, level, profileInd, profileStrPtr));

    rc = internal_cpssTmShapingProfileRead(devNum, level, profileInd, profileStrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, level, profileInd, profileStrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmShapingProfileUpdate function
* @endinternal
*
* @brief   Update a Shaping Profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Level to update the profile for.
* @param[in] profileInd               - An index of the Shaping profile.
* @param[in] profileStrPtr            - Shaping profile configuration struct pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_INITIALIZED       - on not configured periodic update rate for the given level
* @retval GT_BAD_SIZE              - on too large min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmShapingProfileUpdate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  GT_U32                                  profileInd,
    IN  CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

   struct tm_shaping_profile_params prf;

   PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );
   CPSS_NULL_PTR_CHECK_MAC(profileStrPtr);

    cpssOsMemCpy(&prf, profileStrPtr, sizeof(struct tm_shaping_profile_params));
    ret = tm_update_shaping_profile(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                  level,
                                  profileInd,
                                  &prf);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

/**
* @internal cpssTmShapingProfileUpdate function
* @endinternal
*
* @brief   Update a Shaping Profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Level to update the profile for.
* @param[in] profileInd               - An index of the Shaping profile.
* @param[in] profileStrPtr            - Shaping profile configuration struct pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_INITIALIZED       - on not configured periodic update rate for the given level
* @retval GT_BAD_SIZE              - on too large min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmShapingProfileUpdate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  GT_U32                                  profileInd,
    IN  CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmShapingProfileUpdate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, level, profileInd, profileStrPtr));

    rc = internal_cpssTmShapingProfileUpdate(devNum, level, profileInd, profileStrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, level, profileInd, profileStrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}




/**
* @internal internal_cpssTmShapingProfileValidate function
* @endinternal
*
* @brief   Validates if shaping profile can be configured and returns updated bursts values if possible
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Level to configure the profile for.
* @param[in] profileStrPtr            - Shaping profile configuration struct pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_INITIALIZED       - on not configured periodic update rate for the given level
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on too large min/max token or thyare to different to cofigure them togther
* @retval GT_BAD_SIZE              - on cps/ebs value is to small for required bandwidth
*                                       in this case  cbs/ebs values are updated to minimum possible value.
*/
static GT_STATUS internal_cpssTmShapingProfileValidate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

   struct tm_shaping_profile_params prf;

   PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );
   CPSS_NULL_PTR_CHECK_MAC(profileStrPtr);

    cpssOsMemCpy(&prf, profileStrPtr, sizeof(struct tm_shaping_profile_params));
    ret = tm_verify_shaping_profile(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                  level,
                                  &prf);
    /* return updated values of cbs/ebs (if changed ) */
    cpssOsMemCpy(profileStrPtr, &prf, sizeof(struct tm_shaping_profile_params));
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    return rc;
}


/**
* @internal cpssTmShapingProfileValidate function
* @endinternal
*
* @brief   Validates if shaping profile can be configured and returns updated bursts values if possible
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Level to configure the profile for.
* @param[in] profileStrPtr            - Shaping profile configuration struct pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_INITIALIZED       - on not configured periodic update rate for the given level
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on too large min/max token or thyare to different to cofigure them togther
* @retval GT_BAD_SIZE              - on cps/ebs value is to small for required bandwidth
*                                       in this case  cbs/ebs values are updated to minimum possible value.
*/
GT_STATUS cpssTmShapingProfileValidate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmShapingProfileValidate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, level, profileStrPtr));

    rc = internal_cpssTmShapingProfileValidate(devNum, level, profileStrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, level,profileStrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmPortShapingValidate function
* @endinternal
*
* @brief   Validate Port Shaping parameters.and returns updated bursts values if possible
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] cirBw                    - Port CIR bandwidth.
* @param[in] eirBw                    - Port EIR bandwidth.
*                                      INPUT/OUTPUT
* @param[in] pcbs                     - (pointer to)Port CIR burst size.
* @param[in] pebs                     - (pointer to)Port EIR burst size.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on too large min/max token or thy are to different to cofigure them together
* @retval GT_BAD_SIZE              - on cps/ebs value is too small for required bandwidth
*                                       in this case  pcbs/pebs values are updated to minimum possible value.
*
* @note none
*
*/
static GT_STATUS internal_cpssTmPortShapingValidate
(
    IN GT_U8               devNum,
    IN GT_U32              cirBw,
    IN GT_U32              eirBw,
    IN GT_U32 *            pcbs,
    IN GT_U32 *            pebs
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    uint32_t    cbs;
    uint32_t    ebs;

    cbs = (uint32_t)*pcbs;
    ebs = (uint32_t)*pebs;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    ret = tm_verify_port_shaping_configuration(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, cirBw, eirBw, &cbs, &ebs);

    *pcbs = (GT_U32)cbs;
    *pebs = (GT_U32)ebs;

    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

/**
* @internal cpssTmPortShapingValidate function
* @endinternal
*
* @brief   Validate Port Shaping parameters.and returns updated bursts values if possible
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] cirBw                    - Port CIR bandwidth.
* @param[in] eirBw                    - Port EIR bandwidth.
*                                      INPUT/OUTPUT
*                                      cbsPtr          - (pointer to)Port CIR burst size.
*                                      ebsPtr          - (pointer to)Port EIR burst size.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on too large min/max token or thy are to different to cofigure them together
* @retval GT_BAD_SIZE              - on cps/ebs value is too small for required bandwidth
*                                       in this case cbs/ebs values are updated to minimum possible value.
*
* @note none
*
*/
GT_STATUS cpssTmPortShapingValidate
(
    IN GT_U8               devNum,
    IN GT_U32              cirBw,
    IN GT_U32              eirBw,
    IN GT_U32 *            pcbs,
    IN GT_U32 *            pebs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmPortShapingValidate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum,  cirBw, eirBw, pcbs, pebs));
    CPSS_NULL_PTR_CHECK_MAC(pcbs);
    CPSS_NULL_PTR_CHECK_MAC(pebs);

    rc = internal_cpssTmPortShapingValidate(devNum, cirBw, eirBw, pcbs, pebs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, cirBw, eirBw, pcbs, pebs));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

