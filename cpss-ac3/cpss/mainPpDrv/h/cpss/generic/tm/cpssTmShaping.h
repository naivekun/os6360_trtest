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
* @file cpssTmShaping.h
*
* @brief TM Shaping APIs.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmShapingh
#define __cpssTmShapingh

#include <cpss/generic/tm/cpssTmPublicDefs.h>


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
);


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
);


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
);


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
);



/* */

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
*                                      profileStrPtr   - Shaping profile configuration struct pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_INITIALIZED       - on not configured periodic update rate for the given level
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on too large min/max token or thy are to different to cofigure them together
* @retval GT_BAD_SIZE              - on cps/ebs value is too small for required bandwidth
*                                       in this case  cbs/ebs values are updated to minimum possible value.
*/
GT_STATUS cpssTmShapingProfileValidate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_LEVEL_ENT                       level,
    IN  CPSS_TM_SHAPING_PROFILE_PARAMS_STC      *profileStcPtr
);

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
* @param[in,out] cbsPtr                   - (pointer to)Port CIR burst size.
* @param[in,out] ebsPtr                   - (pointer to)Port EIR burst size.
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
    INOUT GT_U32 *         cbsPtr,
    INOUT GT_U32 *         ebsPtr
);






#endif 	    /* __cpssTmShapingh */


