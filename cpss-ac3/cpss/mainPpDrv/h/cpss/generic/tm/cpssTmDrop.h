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
* @file cpssTmDrop.h
*
* @brief TM Drop Unit configuration, including WRED curves and Drop Profiles.
*
* @version   2
********************************************************************************
*/

#ifndef __cpssTmDroph
#define __cpssTmDroph

#include <cpss/generic/tm/cpssTmPublicDefs.h>
#include <cpss/generic/cos/cpssCosTypes.h>


#define     CPSS_TM_AGING_PROFILES_CNS  4


/**
* @internal cpssTmDropWredCurveCreate function
* @endinternal
*
* @brief   Create a WRED curve for a TM level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - A nodes  the WRED curve is created for (Port/C/B/A/Q).
* @param[in] cos                      - CoS of RED Curve (APPLICABLE RANGES: 0..7).
* @param[in] probabilityArr[CPSS_TM_DROP_PROB_ARR_SIZE_CNS] - Array of 32 probability points in % (APPLICABLE RANGES: 0..100).
*
* @param[out] curveIndexPtr            - (pointer to) The created WRED curve index (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL curveIndPtr.
* @retval GT_FULL                  - on full allocation of WRED Curves table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       2. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else curve will be created for CoS mode.
*
*/
GT_STATUS cpssTmDropWredCurveCreate
(
    IN  GT_U8               devNum,
    IN  CPSS_TM_LEVEL_ENT   level,
    IN  GT_U32              cos,
    IN  GT_U32              probabilityArr[CPSS_TM_DROP_PROB_ARR_SIZE_CNS],
    OUT GT_U32              *curveIndexPtr
);


/**
* @internal cpssTmDropWredTraditionalCurveCreate function
* @endinternal
*
* @brief   Create a WRED traditional curve for a level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - A nodes  the WRED curve is created for (Port/C/B/A/Q).
* @param[in] cos                      - CoS of RED Curve (APPLICABLE RANGES: 0..7).
* @param[in] maxProbability           - Max probability in percents (APPLICABLE RANGES: 1..100).
*
* @param[out] curveIndexPtr            - (pointer to) The created WRED curve index (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_FULL                  - on full allocation of WRED Curves table.
* @retval GT_BAD_PTR               - on NULL curveIndPtr.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       2. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else curve will be created for CoS mode.
*
*/
GT_STATUS cpssTmDropWredTraditionalCurveCreate
(
    IN  GT_U8               devNum,
    IN  CPSS_TM_LEVEL_ENT   level,
    IN  GT_U32              cos,
    IN  GT_U32              maxProbability,
    OUT GT_U32              *curveIndexPtr
);


/**
* @internal cpssTmDropProfileCreate function
* @endinternal
*
* @brief   Create Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - A nodes  the Drop profile is created for (Port/C/B/A/Q).
* @param[in] cos                      - CoS of Drop profile (APPLICABLE RANGES: 0..7).
* @param[in] profileStcPtr            - (pointer to) Drop Profile configuration structure.
*
* @param[out] profileIndexPtr          - (pointer to) The created Drop profile index
*                                      (APPLICABLE RANGES: 0..2047  for Queues,
*                                      0..255   for Anodes,
*                                      0..63    for Bnodes,Cnodes,
*                                      0..number of ports for Ports).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL profileIndPtr or profileStcPtr.
* @retval GT_FULL                  - on full allocation of level's Drop profile table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. In case of Color Blind TD disabled set cbTdThresholdBytes=CPSS_TM_MAX_DROP_THRESHOLD_CNS
*       2. Cos of Drop Profile matches Cos of given curve.
*       3. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       4. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be created for CoS mode.
*
*/
GT_STATUS cpssTmDropProfileCreate
(
    IN  GT_U8                           devNum,
    IN  CPSS_TM_LEVEL_ENT               level,
    IN  GT_U32                          cos,
    IN  CPSS_TM_DROP_PROFILE_PARAMS_STC *profileStcPtr,
    OUT GT_U32                          *profileIndexPtr
);

/**
* @internal cpssTmDropProfileUpdate function
* @endinternal
*
* @brief   Update Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - A nodes  the Drop profile is created for (Port/C/B/A/Q).
* @param[in] cos                      - CoS of Drop profile (APPLICABLE RANGES: 0..7).
* @param[in] profileIndex             - Drop profile index to update
*                                      (APPLICABLE RANGES: 0..2047  for Queues,
*                                      0..255   for Anodes,
*                                      0..63    for Bnodes,Cnodes,
*                                      0..number of ports for Ports).
* @param[in] profileStcPtr            - (pointer to) Drop Profile configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL profileStcPtr.
* @retval GT_FULL                  - on full allocation of level's Drop profile table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. In case of Color Blind TD disabled set cbTdThresholdBytes=CPSS_TM_MAX_DROP_THRESHOLD_CNS
*       2. Cos of Drop Profile matches Cos of given curve.
*       3. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       4. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be created for CoS mode.
*       5. For Q/A/B/C levels the user is aware to pause traffic during update process
*       6. For P level update is possible only if profile is not in use.
*
*/
GT_STATUS cpssTmDropProfileUpdate
(
    IN  GT_U8                           devNum,
    IN  CPSS_TM_LEVEL_ENT               level,
    IN  GT_U32                          cos,
    IN  GT_U32                          profileIndex,
    IN  CPSS_TM_DROP_PROFILE_PARAMS_STC *profileStcPtr
);


/**
* @internal cpssTmDropConvertBytesToBW function
* @endinternal
*
* @brief   Utility function :   Converts Tail drop threshold in bytes to  kBits/sec .
*
* @note none
*
*/
GT_U32 cpssTmDropConvertBytesToBW
(
    IN  GT_U32                          cbTdBytes
);


/**
* @internal cpssTmDropConvertBWToBytes function
* @endinternal
*
* @brief   Utility function :   Converts Tail drop in kkBits/sec units to Bytes.
*
* @note none
*
*/
GT_U32 cpssTmDropConvertBWToBytes
(
    IN  GT_U32                          cbTdkBitsPerSec
);



/**
* @internal cpssTmDropProfileDelete function
* @endinternal
*
* @brief   Delete Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - A nodes  the Drop profile is deleted for (Port/C/B/A/Q).
* @param[in] cos                      - CoS of Drop profile (APPLICABLE RANGES: 0..7).
* @param[in] profile                  - An index of the Drop profile.
*                                      (APPLICABLE RANGES: 0..2047  for Queues,
*                                      0..255   for Anodes,
*                                      0..63    for Bnodes,Cnodes,
*                                      0..number of ports for Ports).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       2. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be deleted for CoS mode.
*
*/
GT_STATUS cpssTmDropProfileDelete
(
    IN GT_U8                            devNum,
    IN CPSS_TM_LEVEL_ENT                level,
    IN GT_U32                           cos,
    IN GT_U32                   		profile
);


/**
* @internal cpssTmDropProfileRead function
* @endinternal
*
* @brief   Read Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - A nodes  the Drop profile is read for (Port/C/B/A/Q).
* @param[in] cos                      - CoS of Drop profile (APPLICABLE RANGES: 0..7).
* @param[in] profileInd               - An index of the Drop profile.
*                                      (APPLICABLE RANGES: 0..2047  for Queues,
*                                      0..255   for Anodes,
*                                      0..63    for Bnodes,Cnodes,
*                                      0..number of ports for Ports).
*
* @param[out] profileStcPtr            - (pointer to) Drop profile configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL profileStcPtr.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The cbTdThresholdRatio is calculated aproximately from the register's values.
*       2. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       3. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be read for CoS mode.
*
*/
GT_STATUS cpssTmDropProfileRead
(
    IN GT_U8                            devNum,
    IN CPSS_TM_LEVEL_ENT                level,
    IN GT_U32                           cos,
    IN GT_U32                           profileInd,
    OUT CPSS_TM_DROP_PROFILE_PARAMS_STC *profileStcPtr
);


/**
* @internal cpssTmDropColorNumSet function
* @endinternal
*
* @brief   Set Drop (Egress) Colors number per level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - A nodes  to set colors number for (Port/C/B/A/Q).
* @param[in] number                   - Colors amount.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. This API should be called before all the rest Drop APIs (if need).
*       2. By default there are two colors per each level.
*
*/
GT_STATUS cpssTmDropColorNumSet
(
    IN GT_U8                    devNum,
    IN CPSS_TM_LEVEL_ENT        level,
    IN CPSS_TM_COLOR_NUM_ENT    number
);

/**
* @internal cpssTmDropColorNumResolutionGet function
* @endinternal
*
* @brief   Retrieves Drop (Egress) Colors number and curve resolution birs count per level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - A nodes  to set colors number for (Port/C/B/A/Q).
*                                      colorNumPtr  - (pointer to) variable  where color number will be assigned.
*                                      resolutionPtr   - (pointer to) variable  where  curve resolution  will be assigned.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL colorNumPtr or resolutionPtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/

GT_STATUS cpssTmDropColorNumResolutionGet
(
    IN GT_U8                    devNum,
    IN CPSS_TM_LEVEL_ENT        level,
    OUT GT_U8 *					colorNumPtr,
    OUT GT_U8 *					resolutionPtr
);

/**
* @internal cpssTmAgingChangeStatus function
* @endinternal
*
* @brief   Change the aging status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] status                   - Aging status.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmAgingChangeStatus
(
    IN GT_U8         devNum,
    IN GT_U32        status
);

/**
* @internal cpssTmAgingStatusGet function
* @endinternal
*
* @brief   Retrieve the aging status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @param[out] statusPtr                - (pointer to) Aging status.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL statusPtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmAgingStatusGet
(
    IN  GT_U8         devNum,
    OUT GT_U32        *statusPtr
);


/**
* @internal cpssTmDpSourceSet function
* @endinternal
*
* @brief   Change Drop Probability (DP) source.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - A nodes  to set dp source for (APPLICABLE RANGES: CPSS_TM_LEVEL_Q_E..CPSS_TM_LEVEL_P_E).
* @param[in] color                    - A  to set dp source for (APPLICABLE RANGES: 0..2).
* @param[in] source                   - Dp  (APPLICABLE RANGES: CPSS_TM_DP_SOURCE_AQL_E..CPSS_TM_DP_SOURCE_QL_E).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmDpSourceSet
(
    IN GT_U8                   devNum,
    IN CPSS_TM_LEVEL_ENT       level,
    IN CPSS_DP_LEVEL_ENT       color,
    IN CPSS_TM_DP_SOURCE_ENT   source
);

/**
* @internal cpssTmDpSourceGet function
* @endinternal
*
* @brief   Change Drop Probability (DP) source.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - A nodes  to set dp source for (APPLICABLE RANGES: CPSS_TM_LEVEL_Q_E..CPSS_TM_LEVEL_P_E).
* @param[in] color                    - A  to set dp source for (APPLICABLE RANGES: 0..2).
*
* @param[out] sourcePtr                - (pointer to)Dp source.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL sourcePtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmDpSourceGet
(
    IN GT_U8                   devNum,
    IN CPSS_TM_LEVEL_ENT       level,
    IN CPSS_DP_LEVEL_ENT       color,
    OUT CPSS_TM_DP_SOURCE_ENT  *sourcePtr
);

/**
* @internal cpssTmDropQueryResponceSet function
* @endinternal
*
* @brief   Drop Query Response Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portDp                   - 0 - Global, 1 - CoS.
* @param[in] level                    - Local response  (APPLICABLE RANGES: CPSS_TM_LEVEL_Q_E..CPSS_TM_LEVEL_P_E).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmDropQueryResponceSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portDp,
    IN CPSS_TM_LEVEL_ENT       level
);

/**
* @internal cpssTmDropQueryResponceGet function
* @endinternal
*
* @brief   Retrieve Drop Query Response state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @param[out] portDpPtr                - (pointer to) PortDp.
* @param[out] levelPtr                 - (pointer to) Local response level.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL portDpPtr or levelPtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmDropQueryResponceGet
(
    IN GT_U8                     devNum,
    OUT GT_U32                  *portDpPtr,
    OUT CPSS_TM_LEVEL_ENT       *levelPtr
);

/**
* @internal cpssTmDropQueueCosSet function
* @endinternal
*
* @brief   Drop Queue Cos Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Queue  (APPLICABLE RANGES: 0..16383).
* @param[in] cos                      - Cos (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmDropQueueCosSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  index,
    IN GT_U32                  cos
);

/**
* @internal cpssTmDropQueueCosGet function
* @endinternal
*
* @brief   Retrieve Drop Queue Cos.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Queue  (APPLICABLE RANGES: 0..16383).
*
* @param[out] cosPtr                   - (pointer to)CoS.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmDropQueueCosGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  index,
    OUT GT_U32                 *cosPtr
);

/**
* @internal cpssTmDropProfileAgingBlockCreate function
* @endinternal
*
* @brief   Create Aging drop profile for CPSS_TM_LEVEL_Q_E.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] profileStcArr[CPSS_TM_AGING_PROFILES_CNS] - array of 4 Aging Profile configuration structure pointer.
*
* @param[out] profileIndexPtr          - (pointer to) The first created Aging profile index (APPLICABLE RANGES: 0..2044).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL profileIndexPtr.
* @retval GT_FULL                  - on full allocation of level's Drop profile table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. In case of Color Blind TD disabled set cbTdThresholdBytes=CPSS_TM_MAX_DROP_THRESHOLD_CNS
*       2. Cos of Drop Profile matches Cos of given curve.
*       3. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       4. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be created for CoS mode.
*
*/
GT_STATUS cpssTmDropProfileAgingBlockCreate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_TM_DROP_PROFILE_PARAMS_STC         profileStcArr[CPSS_TM_AGING_PROFILES_CNS],
    OUT GT_U32                                  *profileIndexPtr
);

/**
* @internal cpssTmDropProfileAgingUpdate function
* @endinternal
*
* @brief   Update Aging Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] profileIndex             - The first created Aging profile index (APPLICABLE RANGES: 0..2044).
* @param[in] profileOffset            - The drop profile index to be update (APPLICABLE RANGES: 0..CPSS_TM_AGING_BLOCK_SIZE).
* @param[in] profileParamsPtr         - (pointer to)Drop Profile configurations structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL profileParamsPtr.
* @retval GT_FULL                  - on full allocation of level's Drop profile table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmDropProfileAgingUpdate
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  profileIndex,
    IN  GT_U32                                  profileOffset,
    IN  CPSS_TM_DROP_PROFILE_PARAMS_STC         *profileParamsPtr
);


/**
* @internal cpssTmDropProfileAgingBlockDelete function
* @endinternal
*
* @brief   Delete Aging profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] profileIndex             - The index of the first Aging profile (APPLICABLE RANGES: 0..2044).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmDropProfileAgingBlockDelete
(
    IN GT_U8                    devNum,
    IN GT_U32                   profileIndex
);

/**
* @internal cpssTmNodeDropProfileIndexRead function
* @endinternal
*
* @brief   As a result of aging process the queue drop profile index can be changed dynamically
*         internally by TM. This function allows to monitor these changes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Scheduling level: Queue/A/B/C-node/Port.-node/Port.
* @param[in] cos                      -   of drop profile (APPLICABLE RANGES: 0..7 for port/C-level, otherwise ignored ).
* @param[in] nodeIndex                - The index of queue to read its drop profile.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL profileIndexPtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The CoS parameter is relevant for C and P level (Global mode) only, else set CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmNodeDropProfileIndexRead
(
    IN GT_U8                    devNum,
    IN CPSS_TM_LEVEL_ENT        level,
    IN GT_U32                   cos,
    IN GT_U32                   nodeIndex,
    OUT GT_U32                  *profileIndexPtr
);

#endif 	    /* __cpssTmDroph */


