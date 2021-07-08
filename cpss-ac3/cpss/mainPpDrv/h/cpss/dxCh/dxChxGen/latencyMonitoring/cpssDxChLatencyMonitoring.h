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
* @file cpssDxChLatencyMonitoring.h
*
* @brief CPSS DxCh Latency monitoring API.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChLatencyMonitoringh
#define __cpssDxChLatencyMonitoringh

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Defines BMP for LMU selection. Each LMU# has own bit offset
   starting from LSB.
   Every LMU corresponds certain MAC range of 8 ports.
   MAC# can be converted to LMU# by following formula LMU#=MAC#>>3
   Value CPSS_LMU_UNAWARE_MODE_CNS means that all LMUs configured
*/
typedef GT_U32 GT_LATENCY_MONITORING_UNIT_BMP;

/* Constant that selects all LMUs in the device. */
#define CPSS_LMU_UNAWARE_MODE_CNS 0Xffffffff

/**
 * @struct CPSS_DXCH_LATENCY_MONITORING_STAT_STC
 *
 * @brief A structure that contains latency monitoring statistics
*/
typedef struct{

    /** @brief Minimal latency (ns) */
    GT_U32 minLatency;

    /** @brief Maximal latency (ns) */
    GT_U32 maxLatency;

    /** @brief Average latency (ns) */
    GT_U32 avgLatency;

    /** @brief number of packets in range */
    GT_U64 packetsInRange;

    /** @brief number of packets out of range */
    GT_U64 packetsOutOfRange;

} CPSS_DXCH_LATENCY_MONITORING_STAT_STC;

/**
 * @struct CPSS_DXCH_LATENCY_MONITORING_CFG_STC
 *
 * @brief A structure that contains latency monitoring configuration
 */
typedef struct{

    /** @brief Range upper latency limit, ns (APPLICABLE RANGES: 0..0x3FFFFFFF) */
    GT_U32 rangeMax;

    /** @brief Range lower latency limit, ns (APPLICABLE RANGES: 0..0x3FFFFFFF) */
    GT_U32 rangeMin;

    /** @brief Notfication threshold, ns (APPLICABLE RANGES: 0..0x3FFFFFFF) */
    GT_U32 notificationThresh;

} CPSS_DXCH_LATENCY_MONITORING_CFG_STC;

/**
 * @struct CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT
 *
 * @brief Latency monitoring packet sampling mode in EPCL
*/
typedef enum{

    /** @brief Every 2^samplingThreshold packet sampled */
    CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E,

    /** @brief Packet sampled if samplingThreshold < 32-bit random value */
    CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_RANDOM_E
} CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT;

/**
* @internal cpssDxChLatencyMonitoringPortCfgSet function
* @endinternal
*
* @brief  Set index and profile for physical port for port latency monitoring.
*         Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portType              - determine type of port: ingress, egress or both.
* @param[in] index                 - index (APPLICABLE RANGES: 0..255)
* @param[in] profile               - latency profile (APPLICABLE RANGES: 0..511)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - index and profile is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringPortCfgSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN CPSS_DIRECTION_ENT                   portType,
    IN GT_U32                               index,
    IN GT_U32                               profile
);

/**
* @internal cpssDxChLatencyMonitoringPortCfgGet function
* @endinternal
*
* @brief  Get index and profile for physical port for port latency monitoring.
*         Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portType              - determine type of port: ingress or egress
*
* @param[out] indexPtr             - (pointer to) index
* @param[out] profilePtr           - (pointer to) latency profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringPortCfgGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DIRECTION_ENT                   portType,
    OUT GT_U32                               *indexPtr,
    OUT GT_U32                               *profilePtr
);

/**
* @internal cpssDxChLatencyMonitoringEnableSet function
* @endinternal
*
* @brief  Set latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] srcPortNum            - source port number
* @param[in] trgPortNum            - target port number
* @param[in] enabled               - latency monitoring enabled state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM srcPortNum,
    IN GT_PHYSICAL_PORT_NUM trgPortNum,
    IN GT_BOOL              enabled
);

/**
* @internal cpssDxChLatencyMonitoringEnableGet function
* @endinternal
*
* @brief  Get latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] srcPortNum            - source port number
* @param[in] trgPortNum            - target port number
*
* @param[out] enabledPtr           - (pointer to) latency monitoring enabled state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringEnableGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM srcPortNum,
    IN GT_PHYSICAL_PORT_NUM trgPortNum,
    OUT GT_BOOL             *enabledPtr
);

/**
* @internal cpssDxChLatencyMonitoringSamplingProfileSet function
* @endinternal
*
* @brief  Set latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingProfileSet
(
    IN GT_U8        devNum,
    IN GT_U32       latencyProfile,
    IN GT_U32       samplingProfile
);

/**
* @internal cpssDxChLatencyMonitoringSamplingProfileGet function
* @endinternal
*
* @brief  Get latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] samplingProfilePtr   - (pointer to) sampling profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingProfileGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       latencyProfile,
    OUT GT_U32       *samplingProfilePtr
);

/**
* @internal cpssDxChLatencyMonitoringSamplingConfigurationSet function
* @endinternal
*
* @brief  Set latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
* @param[in] samplingMode          - sampling mode.
* @param[in] samplingThreshold     - sampling threshold. Value 0 - every packet sampled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingConfigurationSet
(
    IN GT_U8                                          devNum,
    IN GT_U32                                         samplingProfile,
    IN CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT samplingMode,
    IN GT_U32                                         samplingThreshold
);

/**
* @internal cpssDxChLatencyMonitoringSamplingConfigurationGet function
* @endinternal
*
* @brief  Get latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
*
* @param[out] samplingModePtr      - (pointer to)sampling mode.
* @param[out] samplingThresholdPtr - (pointer to)sampling threshold
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingConfigurationGet
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         samplingProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT *samplingModePtr,
    OUT GT_U32                                         *samplingThresholdPtr
);

/**
* @internal cpssDxChLatencyMonitoringStatGet function
* @endinternal
*
* @brief  Get latency monitoring statistics.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3. 
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[out] statisticsPtr        - (pointer to) latency monitoring statistics
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note In case if multiple LMUs selected, packet counts are summed, average
* latency calculated as average of average latency values for all selected LMUs.
* Minimal latency is the lowest value of minimal latency values for all
* selected LMUs. Maximal latency is the highest value of maximal latency values
* for all selected LMUs.
* 
*/
GT_STATUS cpssDxChLatencyMonitoringStatGet
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_STAT_STC  *statisticsPtr
);

/**
* @internal cpssDxChLatencyMonitoringCfgSet function
* @endinternal
*
* @brief  Set latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3. 
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[in] lmuConfigPtr          - (pointer to)latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringCfgSet
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile,
    IN  CPSS_DXCH_LATENCY_MONITORING_CFG_STC   *lmuConfigPtr
);

/**
* @internal cpssDxChLatencyMonitoringCfgGet function
* @endinternal
*
* @brief  Get latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3. Only one LMU
*                                    can be selected.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] lmuConfigPtr         - (pointer to) latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringCfgGet
(
    IN  GT_U8                                 devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP        lmuBmp,
    IN  GT_U32                                latencyProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_CFG_STC  *lmuConfigPtr
);

/**
* @internal cpssDxChLatencyMonitoringPortEnableSet function
* @endinternal
*
* @brief  Enable/Disable egress port for latency monitoring.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] enable                - latency monitoring enabled state
*                                    GT_TRUE - enable
*                                    GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS cpssDxChLatencyMonitoringPortEnableSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  GT_BOOL                                  enable
);

/**
* @internal cpssDxChLatencyMonitoringPortEnableGet function
* @endinternal
*
* @brief  Get enabled state of latency monitoring for port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] enablePtr            - (pointer to)latency monitoring enabled state
*                                    GT_TRUE  - enabled
*                                    GT_FALSE - disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS cpssDxChLatencyMonitoringPortEnableGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT GT_BOOL                                 *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChLatencyMonitoringh */
