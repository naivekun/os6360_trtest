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
* @file cpssTmNodesUpdate.h
*
* @brief TM nodes update APIs
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmNodesUpdateh
#define __cpssTmNodesUpdateh

#include <cpss/generic/tm/cpssTmPublicDefs.h>


/**
* @internal cpssTmQueueUpdate function
* @endinternal
*
* @brief   Update queue parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Queue index.
* @param[in] paramsPtr                - Queue parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. When error occurs, the entry is considered inconsistent.
*       2. 'dropProfileInd' parameter will be updated in any case,
*       set it's value to be the same as in DB if you don't want to change it.
*
*/
GT_STATUS cpssTmQueueUpdate
(
    IN GT_U8                        devNum,
    IN GT_U32                       index,
    IN CPSS_TM_QUEUE_PARAMS_STC     *paramsPtr
);


/**
* @internal cpssTmAnodeUpdate function
* @endinternal
*
* @brief   Update A-node parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
* @param[in] paramsPtr                - Node parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. When error occurs, the entry is considered inconsistent.
*       2. 'numOfChildren' can't be updated.
*       3. 'dropProfileInd' parameter will be updated in any case,
*       set it's value to be the same as in DB if you don't want to change it.
*
*/
GT_STATUS cpssTmAnodeUpdate
(
    IN GT_U8                        devNum,
    IN GT_U32                       index,
    IN CPSS_TM_A_NODE_PARAMS_STC    *paramsPtr
);


/**
* @internal cpssTmBnodeUpdate function
* @endinternal
*
* @brief   Update B-node parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
* @param[in] paramsPtr                - Node parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. When error occurs, the entry is considered inconsistent.
*       2. 'numOfChildren' can't be updated.
*
*/
GT_STATUS cpssTmBnodeUpdate
(
    IN GT_U8                        devNum,
    IN GT_U32                       index,
    IN CPSS_TM_B_NODE_PARAMS_STC    *paramsPtr
);


/**
* @internal cpssTmCnodeUpdate function
* @endinternal
*
* @brief   Update C-node parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
* @param[in] paramsPtr                - Node parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. When error occurs, the entry is considered inconsistent.
*       2. 'numOfChildren' can't be updated.
*
*/
GT_STATUS cpssTmCnodeUpdate
(
    IN GT_U8                         devNum,
    IN GT_U32                        index,
    IN CPSS_TM_C_NODE_PARAMS_STC     *paramsPtr
);


/**
* @internal cpssTmPortShapingUpdate function
* @endinternal
*
* @brief   Update Port Shaping parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Port index.
* @param[in] cirBw                    - Port CIR bandwidth.
* @param[in] eirBw                    - Port EIR bandwidth.
* @param[in] cbs                      - Port CIR burst size.
* @param[in] ebs                      - Port EIR burst size.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_BAD_SIZE              - on Port's min/max token too large.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note When error occurs, the entry is considered inconsistent.
*
*/
GT_STATUS cpssTmPortShapingUpdate
(
    IN GT_U8    devNum,
    IN GT_U32   index,
    IN GT_U32   cirBw,
    IN GT_U32   eirBw,
    IN GT_U32   cbs,
    IN GT_U32   ebs
);


/**
* @internal cpssTmPortSchedulingUpdate function
* @endinternal
*
* @brief   Update Port Scheduling parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Port index.
* @param[in] eligPrioFuncId           - Eligible Priority Function pointer.
* @param[in] quantumArrPtr[8]         - Port quantum 8 cell array.
* @param[in] schdModeArr[8]           - Port RR/DWRR priority pointer for C-level.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note When error occurs, the entry is considered inconsistent.
*
*/
GT_STATUS cpssTmPortSchedulingUpdate
(
    IN GT_U8    devNum,
    IN GT_U32   index,
    IN CPSS_TM_ELIG_FUNC_NODE_ENT   eligPrioFuncId,
    IN GT_U32   quantumArrPtr[8], /* 8 cells array */
    IN CPSS_TM_SCHD_MODE_ENT  schdModeArr[8]
);


/**
* @internal cpssTmPortDropUpdate function
* @endinternal
*
* @brief   Update Port Drop parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Port index.
* @param[in] wredProfileRef           - Port Drop Profile reference.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note When error occurs, the entry is considered inconsistent.
*
*/
GT_STATUS cpssTmPortDropUpdate
(
    IN GT_U8    devNum,
    IN GT_U32   index,
    IN GT_U32   wredProfileRef
);


/**
* @internal cpssTmPortDropCosUpdate function
* @endinternal
*
* @brief   Update Port Drop per Cos parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Port index.
* @param[in] paramsPtr                - Port Drop per Cos parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note When error occurs, the entry is considered inconsistent.
*
*/
GT_STATUS cpssTmPortDropCosUpdate
(
    IN GT_U8    devNum,
    IN GT_U32   index,
    IN CPSS_TM_PORT_DROP_PER_COS_STC   *paramsPtr
);


#endif 	    /* __cpssTmNodesUpdateh */


