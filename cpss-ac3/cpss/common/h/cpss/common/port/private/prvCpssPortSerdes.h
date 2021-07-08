/********************************************************************************
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
* @file prvCpssPortSerdes.h
*
* @brief CPSS implementation for port serdes.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssPortSerdes__
#define __prvCpssPortSerdes__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal prvCpssPortSerdesAutoTunePhase function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
* @param[in,out] trainCookie              - current phase of training
* @param[in,out] trainCookie              - current phase of training to be updated if needed
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssPortSerdesAutoTunePhase
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode,
    INOUT PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_COOKIE_STC   *trainCookie
);

/**
* @internal prvCpssPortSerdesEnhancedAutoTuneByPhase function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process, by phases.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2,xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @param[out] phaseFinishedPtr         - (pointer to) whether or not a phase was finished.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS prvCpssPortSerdesEnhancedAutoTuneByPhase
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U8                  min_LF,
    IN  GT_U8                  max_LF,
    IN  GT_U32                 phase,
    IN  GT_BOOL                *phaseFinishedPtr
);

/**
* @internal prvCpssPortSerdesEnhancedAutoTuneByPhase function
* @endinternal
*
* @brief   Set Rx training process and starts the
*         auto tune process, by phases.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2,xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] phase                   - current phase
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @param[out] phaseFinishedPtr         - (pointer to) whether or not a phase was finished.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS prvCpssPortSerdesLunchOneShotDfe
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  phase,
    IN  GT_BOOL                *phaseFinishedPtr
);

/**
* @internal prvCpssPortSerdesEdgeDetectByPhase function
* @endinternal
*
* @brief   Perform edge detection algorithm in order to detect TX change of peer,
*         than adjust rx according to it.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] phase                    -  to perform
*
* @param[out] phaseFinishedPtr         - (pointer to) whether or not a phase was finished.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS prvCpssPortSerdesEdgeDetectByPhase
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32                 phase,
    IN  GT_BOOL                *phaseFinishedPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPortSerdes__ */

