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
* @file cpssTmSched.h
*
* @brief TM Scheduler APIs.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmSchedh
#define __cpssTmSchedh

#include <cpss/generic/tm/cpssTmPublicDefs.h>


/**
* @internal cpssTmSchedPortExternalBpSet function
* @endinternal
*
* @brief   Enable/Disable Port External Backpressure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portExtBp                - Enable/Disable port external BP.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*                                       COMMENTS:
*
* @note COMMENTS:
*       to enable Backpressure response from TM Ports to physical ports,
*       egressEnable at cpssDxChTmGlueFlowControlEnableSet must also be enabled.
*
*/
GT_STATUS cpssTmSchedPortExternalBpSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   portExtBp
);


/**
* @internal cpssTmSchedPeriodicSchemeConfig function
* @endinternal
*
* @brief   Configure Periodic Scheme.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] paramsPtr                - (pointer to) scheduling parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_VALUE             - on not found parameter or parameter's value can't be obtained.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The API may be invoked once in a system lifetime.
*       2. To indicate the shaping is disabled for a level the
*       periodicState field must be set to GT_FALSE.
*       3 paramsPtr is handled as 5 elements array including also a
*       port level scheduling configuration.
*
*/
GT_STATUS cpssTmSchedPeriodicSchemeConfig
(
    IN GT_U8                                devNum,
    IN CPSS_TM_LEVEL_PERIODIC_PARAMS_STC    *paramsPtr
);


/**
* @internal cpssTmShapingPeriodicUpdateEnableSet function
* @endinternal
*
* @brief   Enable/Disable periodic update for a given level: Q, A, B, C or Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Level to enable/disable shaping for.
* @param[in] status                   - Enable/Disable shaping.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note If periodic update will be disabled for some level, traffic through nodes with shaping-enabled eligible functions will be stopped !
*
*/
GT_STATUS cpssTmShapingPeriodicUpdateEnableSet
(
    IN GT_U8                    devNum,
    IN CPSS_TM_LEVEL_ENT        level,
    IN GT_BOOL                  status
);

/**
* @internal cpssTmShapingPeriodicUpdateStatusGet function
* @endinternal
*
* @brief   Read periodc update status for all TM levels.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] statusPtr                - (pointer to) scheduling parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmShapingPeriodicUpdateStatusGet
(
    IN GT_U8								devNum,
    IN CPSS_TM_LEVEL_PERIODIC_PARAMS_STC    *statusPtr
);



/**
* @internal cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet function
* @endinternal
*
* @brief   Set the number of DWRR bytes per burst limit for all ports.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] bytes                    - Number of  per burst limit.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet
(
    IN GT_U8    devNum,
    IN GT_U32   bytes
);

/**
* @internal cpssTmPortQuantumLimitsGet function
* @endinternal
*
* @brief   get maximum and minimum possible quantum values and quantum value granularity for port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Since set/update port parameters APIs are note in bytes resolution but Quantums units,
*       applicable values are minQuantum/resolution, maxQuantum/resolution.
*
*/
GT_STATUS cpssTmPortQuantumLimitsGet
(
    IN GT_U8						 devNum,
    OUT CPSS_TM_QUANTUM_LIMITS_STC  *quantumLimitsPtr
);

/**
* @internal cpssTmNodeQuantumLimitsGet function
* @endinternal
*
* @brief   get maximum and minimum possible quantum values and quantum value granularity for nodes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @param[out] quantumLimitsPtr         - (pointer to) quantum limits structure.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Since set/update node parameters APIs are note in bytes resolution but Quantums units,
*       applicable values are minQuantum/resolution, maxQuantum/resolution.
*
*/
GT_STATUS cpssTmNodeQuantumLimitsGet
(
    IN GT_U8						 devNum,
    OUT CPSS_TM_QUANTUM_LIMITS_STC  *quantumLimitsPtr
);


#endif 	    /* __cpssTmSchedh */


