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
* @file cpssDxChPortTxDba.h
*
* @brief CPSS definitions for configuring, gathering info and statistics
* for the DBA (Dynamic Buffer Allocation) feature
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChPortTxDbah
#define __cpssDxChPortTxDbah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortTx.h>


/**
* @internal cpssDxChPortTxDbaEnableSet function
* @endinternal
*
* @brief   Enabling DBA feature
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - GT_TRUE  - enable DBA feature
*                                       GT_FALSE - disable DBA feature
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxDbaEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
);

/**
* @internal cpssDxChPortTxDbaEnableGet function
* @endinternal
*
* @brief   Indicates if DBA feature is enabled
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE  - DBA feature enabled
*                                        GT_FALSE - no DBA feature support (or disabled)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - enabledPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxDbaEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortTxDbaDebugControl function
* @endinternal
*
* @brief   Start or stop DBA operation and\or gathering statistics
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] startStopDba             - GT_TRUE  - start DBA operation, i.e. profile thresholds updates.
*                                      GT_FALSE - stop DBA operation, i.e. no profile thresholds updates.
* @param[in] startStopStatistics      - GT_TRUE  - start statistics gathering
*                                      GT_FALSE - stop statistics gathering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*/
GT_STATUS cpssDxChPortTxDbaDebugControl
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  startStopDba,
    IN  GT_BOOL  startStopStatistics
);

/**
* @internal cpssDxChPortTxDbaAvailBuffSet function
* @endinternal
*
* @brief   Set maximal available buffers for allocation.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] maxBuff                  - Maximal available buffers for allocation.
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on out of range number of buffers
*                                      (APPLICABLE RANGES: Aldrin2; Falcon: 0..0xFFFFF)
*/
GT_STATUS cpssDxChPortTxDbaAvailBuffSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  maxBuff
);

/**
* @internal cpssDxChPortTxDbaAvailBuffGet function
* @endinternal
*
* @brief   Get maximal available buffers for allocation.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] maxBuffPtr               - (pointer to) Maximal available buffers for allocation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*/
GT_STATUS cpssDxChPortTxDbaAvailBuffGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *maxBuffPtr
);

/**
* @internal cpssDxChPortTxDbaDebugClearGlobalCounters function
* @endinternal
*
* @brief   Clear the statistics collected related to number of free buffers
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*/
GT_STATUS cpssDxChPortTxDbaDebugClearGlobalCounters
(
    IN  GT_U8   devNum
);

/**
* @internal cpssDxChPortTxDbaDebugGlobalCountersGet function
* @endinternal
*
* @brief   Get Min and Max values of free buffers during DBA run & threshold
*         updates from last values clearing.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] minFreeBuffersPtr        - (pointer to) Minimum value of free buffers.
* @param[out] maxFreeBuffersPtr        - (pointer to) Maximum value of free buffers.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*
* @note Previous values clearing is done using API
*       cpssDxChPortTxDbaDebugClearGlobalCounters.
*
*/
GT_STATUS cpssDxChPortTxDbaDebugGlobalCountersGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *minFreeBuffersPtr,
    OUT GT_U32  *maxFreeBuffersPtr
);

/**
* @internal cpssDxChPortTxDbaDebugProfileCountersEnable function
* @endinternal
*
* @brief   Enable collecting statistical values of <profile,dp,queue> triplet
*         threshold during DBA run & threshold updates, the enable operation
*         clears previous data.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] profileSet               - the Profile Set to which the Traffic
*                                      Class Drop Parameters are associated.
* @param[in] dpLevel                  - Drop Precedence level
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters.
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on out of range tc
*/
GT_STATUS cpssDxChPortTxDbaDebugProfileCountersEnable
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  CPSS_DP_LEVEL_ENT                   dpLevel,
    IN  GT_U8                               trafficClass
);

/**
* @internal cpssDxChPortTxDbaDebugProfileCountersGet function
* @endinternal
*
* @brief   Get Min and Max values of <profile,dp,queue> triplet threshold during
*         DBA run & threshold updates from last values clearing.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] minThresholdPtr          - (pointer to) Minimum value of threshold.
* @param[out] maxThresholdPtr          - (pointer to) Maximum value of threshold.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*
* @note Previous values clearing and triplet selection is done using API
*       cpssDxChPortTxDbaDebugProfileCountersEnable.
*
*/
GT_STATUS cpssDxChPortTxDbaDebugProfileCountersGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *minThresholdPtr,
    OUT GT_U32  *maxThresholdPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortDbah */

