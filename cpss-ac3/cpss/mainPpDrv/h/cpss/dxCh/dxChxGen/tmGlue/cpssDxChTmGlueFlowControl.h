/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssDxChTmGlueFlowControl.h
*
* @brief Traffic Manager Glue - Flow Control API declaration.
*
* @version   2
********************************************************************************
*/

#ifndef __cpssDxChTmGlueFlowControlh
#define __cpssDxChTmGlueFlowControlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

/**
* @internal cpssDxChTmGlueFlowControlEnableSet function
* @endinternal
*
* @brief   Enable/disable Flow Control for TM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] egressEnable             - GT_TRUE / GT_FALSE - enable / disable egress Flow Control for TM.
* @param[in] ingressEnable            GT_TRUE / GT_FALSE - enable / disable ingress Flow Control for TM,
*                                      relevant for Caelum only.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note egressEnable must be enabled whenever the TM is used
*       to response back-pressure from physical ports to TM Ports,
*       used along with cpssTmSchedPortExternalBpSet.
*       Caelum global ingressEnable is used along with
*       Caelum port cpssDxChTmGluePfcResponseModeSet.
*
*/
GT_STATUS cpssDxChTmGlueFlowControlEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  egressEnable,
    IN GT_BOOL                  ingressEnable
);

/**
* @internal cpssDxChTmGlueFlowControlEnableGet function
* @endinternal
*
* @brief   Get state of Flow Control for TM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] egressEnablePtr          - (pointer to) egress flow control state.
* @param[out] ingressEnablePtr         - (pointer to) ingress flow control state.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChTmGlueFlowControlEnableGet
(
    IN GT_U8                    devNum,
    OUT GT_BOOL                *egressEnablePtr,
    OUT GT_BOOL                *ingressEnablePtr
);

/**
* @internal cpssDxChTmGlueFlowControlEgressCounterSet function
* @endinternal
*
* @brief   Set value of the TM Egress Flow Control counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tmPortInd                - TM port index.
*                                      (APPLICABLE RANGES: 0..191).
* @param[in] value                    -  of the TM egress flow control counter.
*                                      (APPLICABLE RANGES: 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, TM port index.
* @retval GT_OUT_OF_RANGE          - on wrong counter value.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChTmGlueFlowControlEgressCounterSet
(
    IN GT_U8 devNum,
    IN GT_U32 tmPortInd,
    IN GT_U32 value
);

/**
* @internal cpssDxChTmGlueFlowControlEgressCounterGet function
* @endinternal
*
* @brief   Get value of the TM Egress Flow Control counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tmPortInd                - TM port index.
*                                      (APPLICABLE RANGES: 0..191).
*
* @param[out] valuePtr                 - (poiter to) value of the TM egress flow control counter.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or TM port index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChTmGlueFlowControlEgressCounterGet
(
    IN GT_U8 devNum,
    IN GT_U32 tmPortInd,
    OUT GT_U32 *valuePtr
);

/**
* @internal cpssDxChTmGlueFlowControlEgressThresholdsSet function
* @endinternal
*
* @brief   Set XON/XOFF TM Egress Flow Control thresholds values.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tmPortInd                - TM port index.
*                                      (APPLICABLE RANGES: 0..191).
* @param[in] xOffThreshold            - TM FC Queue XOFF threshold.
*                                      (APPLICABLE RANGES: 0..1023).
* @param[in] xOnThreshold             - TM FC Queue XON threshold.
*                                      (APPLICABLE RANGES: 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, TM port index.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_OUT_OF_RANGE          - on wrong XOFF/XON threshold.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChTmGlueFlowControlEgressThresholdsSet
(
    IN GT_U8 devNum,
    IN GT_U32 tmPortInd,
    IN GT_U32 xOffThreshold,
    IN GT_U32 xOnThreshold
);

/**
* @internal cpssDxChTmGlueFlowControlEgressThresholdsGet function
* @endinternal
*
* @brief   Get XON/XOFF TM Egress Flow Control thresholds values.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tmPortInd                - TM port index.
*                                      (APPLICABLE RANGES: 0..191).
*
* @param[out] xOffThresholdPtr         - (pointer to) TM FC Queue XOFF threshold.
* @param[out] xOnThresholdPtr          - (pointer to) TM FC Queue XON threshold.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or TM port index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChTmGlueFlowControlEgressThresholdsGet
(
    IN GT_U8 devNum,
    IN GT_U32 tmPortInd,
    OUT GT_U32 *xOffThresholdPtr,
    OUT GT_U32 *xOnThresholdPtr
);

/**
* @internal cpssDxChTmGlueFlowControlPortSpeedSet function
* @endinternal
*
* @brief   Set PFC calibration value by port speed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This API should be called on changing of TM Enabled physical port speed
*       mapped to the PFC port, as a result of:
*       1) port speed has changed. (cpssDxChPortModeSpeedSet)
*       2) PFC port mapping has changed (cpssDxChTmGluePfcPortMappingSet).
*       This table is the calibration value to be multiplied to the value in the PFC header
*       to be alligned to the number of cycles according to the port speed.
*
*/
GT_STATUS cpssDxChTmGlueFlowControlPortSpeedSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_SPEED_ENT      speed
);

/**
* @internal cpssDxChTmGlueFlowControlPortSpeedGet function
* @endinternal
*
* @brief   Get port speed from its PFC calibration value set by
*         cpssDxChTmGlueFlowControlPortSpeedSet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] speedPtr                 - (pointer of) CPSS_PORT_SPEED_ENT.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChTmGlueFlowControlPortSpeedGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_SPEED_ENT  *speedPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTmGlueFlowControlh */



