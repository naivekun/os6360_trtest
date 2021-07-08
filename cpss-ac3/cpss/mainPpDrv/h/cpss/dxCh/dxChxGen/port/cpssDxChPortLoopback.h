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
* @file cpssDxChPortLoopback.h
*
* @brief Forwarding to loopback/service port APIs.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPortLoopbackh
#define __cpssDxChPortLoopbackh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>

/* loopback profiles number (0..3)*/
#define CPSS_DXCH_PORT_LB_PROFILES_NUM_CNS 4

/**
* @internal cpssDxChPortLoopbackEnableSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port for specified combination of
*         source/target loopback profiles and traffic class.
*         NOTE: bobcat2 A0 device enables/disables the feature for all
*         traffic classes regardless 'tc' input parameter value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcProfile               - source loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] trgProfile               - target loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] tc                       - traffic class.
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] enable                   - GT_TRUE  - enable forwarding to loopback
*                                      GT_FALSE - disable forwarding to loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is general trigger.
*       You need to enable forwarding to loopback per desirable packet types
*       additionally. See
*       cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet
*       cpssDxChPortLoopbackPktTypeFromCpuEnableSet
*       cpssDxChPortLoopbackPktTypeToCpuSet
*       cpssDxChPortLoopbackPktTypeToAnalyzerSet
*
*/
GT_STATUS cpssDxChPortLoopbackEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  srcProfile,
    IN GT_U32  trgProfile,
    IN GT_U32  tc,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChPortLoopbackEnableGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status for specified combination of
*         source/target loopback profiles and traffic class.
*         bobcat2 a0 is not able enable forwarding-to-loopback per particular
*         traffic class so it ignores input parameter 'traffic class'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcProfile               - source loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] trgProfile               - target loopback profile number
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] tc                       - traffic class
*                                      (APPLICABLE RANGES: 0..7)
*
* @param[out] enablePtr                - GT_TRUE  - forwarding to loopback is enabled
*                                        GT_FALSE - forwarding to loopback is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  srcProfile,
    IN  GT_U32  trgProfile,
    IN  GT_U32  tc,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port for FORWARD and FROM_CPU packet type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] fromCpuEn                - GT_TRUE  - enable forwarding to loopback
*                                      FORWARD, FROM_CPU traffic
*                                      GT_FALSE - enable forwarding to loopback
*                                      FORWARD traffic only
* @param[in] singleTargetEn           - GT_TRUE  - enable forwarding to loopback for
*                                      single-target traffic
*                                      GT_FALSE - disable forwarding to loopback for
*                                      single target traffic
* @param[in] multiTargetEn            - GT_TRUE  - enable forwarding to loopback for
*                                      multi-target traffic
*                                      GT_FALSE - disable forwarding to loopback for
*                                      multi-target traffic
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet
(
    IN GT_U8                devNum,
    IN GT_BOOL              fromCpuEn,
    IN GT_BOOL              singleTargetEn,
    IN GT_BOOL              multiTargetEn
);

/**
* @internal cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet function
* @endinternal
*
* @brief   Get status of loopback for FORWARD and FROM_CPU packet type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] fromCpuEnPtr             - if forwarding-to-loopback is enabled for FROM_CPU
*                                      packet type
*                                      Can be NULL
* @param[out] singleTargetEnPtr        - if single-target traffic is forwarded to loopback
*                                      Can be NULL
* @param[out] multiTargetEnPtr         - if multi-target traffic is forwarded to loopback
*                                      Can be NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                        packet type.
*                                        Can be NULL
*                                        Can be NULL
*                                        Can be NULL
*/
GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet
(
    IN  GT_U8                devNum,
    OUT GT_BOOL              *fromCpuEnPtr,
    OUT GT_BOOL              *singleTargetEnPtr,
    OUT GT_BOOL              *multiTargetEnPtr
);

/**
* @internal cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet function
* @endinternal
*
* @brief   Specify loopback port for physical port number. This loopback port is
*         used for FORWARD and FROM_CPU packets targeted to this physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - target physical port number
* @param[in] loopbackPortNum          - loopback port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The port loopbackPortNum is only designated as a loopback port
*       of the portNum but its loopback state is not configured.
*       cpssDxChPortInternalLoopbackEnableSet can be used to configure it.
*
*/
GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_PHYSICAL_PORT_NUM   loopbackPortNum
);

/**
* @internal cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet function
* @endinternal
*
* @brief   Get loopback port assigned to physical port number. The loopback port is
*         used for by FORWARD and FROM_CPU packet targeted to this physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] loopbackPortNumPtr       - loopback port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *loopbackPortNumPtr
);


/**
* @internal cpssDxChPortLoopbackPktTypeToCpuSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port and assign a loopback port for
*         TO_CPU packet type with specified CPU code
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - CPU code. Can be CPSS_NET_ALL_CPU_OPCODES_E
* @param[in] loopbackPortNum          - loopback port number
* @param[in] enable                   - enable forwarding to loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The port loopbackPortNum is only designated as a loopback port
*       of the portNum but its loopback state is not configured.
*       You can use cpssDxChPortInternalLoopbackEnableSet to congifure it.
*
*/
GT_STATUS cpssDxChPortLoopbackPktTypeToCpuSet
(
    IN GT_U8                    devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    IN GT_PHYSICAL_PORT_NUM     loopbackPortNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChPortLoopbackPktTypeToCpuGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status and a loopback port number for
*         TO_CPU packet type with specified CPU code
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - CPU code
*
* @param[out] loopbackPortNumPtr       - loopback port number
* @param[out] enablePtr                - if forwarding to loopback is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackPktTypeToCpuGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    OUT GT_PHYSICAL_PORT_NUM     *loopbackPortNumPtr,
    OUT GT_BOOL                  *enablePtr
);

/**
* @internal cpssDxChPortLoopbackPktTypeToAnalyzerSet function
* @endinternal
*
* @brief   Enable forwarding-to-loopback-port and assign a loopback port for
*         TO_ANALYZER packet mirrored to specified analyzer index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] analyzerIndex            - analyzer index
*                                      (APPLICABLE RANGE: 0..6)
* @param[in] loopbackPortNum          - loopback port number
* @param[in] enable                   - enable forwarding to loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The port loopbackPortNum is only designated as a loopback port
*       of the portNum but its loopback state is not configured.
*       You can use cpssDxChPortInternalLoopbackEnableSet to congifure it.
*
*/
GT_STATUS cpssDxChPortLoopbackPktTypeToAnalyzerSet
(
    IN GT_U8                devNum,
    IN GT_U32               analyzerIndex,
    IN GT_PHYSICAL_PORT_NUM loopbackPortNum,
    IN GT_BOOL              enable
);

/**
* @internal cpssDxChPortLoopbackPktTypeToAnalyzerGet function
* @endinternal
*
* @brief   Get a forwarding-to-loopback-port status and a loopback port number for
*         TO_ANALYZER packet mirrored to specified analyzer index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] analyzerIndex            - analyzer index
*                                      (APPLICABLE RANGE: 0..6)
*
* @param[out] loopbackPortNumPtr       - loopback port number
* @param[out] enablePtr                - if forwarding to loopback is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackPktTypeToAnalyzerGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               analyzerIndex,
    OUT GT_PHYSICAL_PORT_NUM *loopbackPortNumPtr,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssDxChPortLoopbackProfileSet function
* @endinternal
*
* @brief   Bind source or target loopback profiles to specified physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - specifies which one loopback profile to configure
*                                      CPSS_DIRECTION_INGRESS_E - source loopback profile
*                                      CPSS_DIRECTION_EGRESS_E  - target loopback profile
*                                      CPSS_DIRECTION_BOTH_E    - both source/target
*                                      loopback profiles
* @param[in] profile                  - loopback profile
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DIRECTION_ENT   direction,
    IN  GT_U32               profile
);

/**
* @internal cpssDxChPortLoopbackProfileGet function
* @endinternal
*
* @brief   Get source or target loopback profile of specified physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - specifies which one loopback profile get
*                                      CPSS_DIRECTION_INGRESS_E - source loopback profile
*                                      CPSS_DIRECTION_EGRESS_E  - target loopback profile
*
* @param[out] profilePtr              - loopback profile (0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DIRECTION_ENT   direction,
    OUT GT_U32               *profilePtr
);

/**
* @internal cpssDxChPortLoopbackEvidxMappingSet function
* @endinternal
*
* @brief   Enable eVIDX mapping for multi-target traffic forwarded to loopback and
*         specify Loopback eVIDX offset.
*         If enabled multi-target packet forwarded to loopback (this is done after
*         the replication to the port distribution list) will be assigned with
*         new eVIDX=<Loopback eVIDX offset> + <target port>.
*         The plan is to allow you configure every eVIDX in the all target ports
*         range as physical-port-distribution-list that includes only the
*         corresponding physical port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vidxOffset               - a Loopback eVIDX offset.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] enable                   -  eVIDX mapping.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is usually used in conjunction with
*       cpssDxChPortLoopbackEnableEgressMirroringSet
*
*/
GT_STATUS cpssDxChPortLoopbackEvidxMappingSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               vidxOffset,
    IN  GT_BOOL              enable
);

/**
* @internal cpssDxChPortLoopbackEvidxMappingGet function
* @endinternal
*
* @brief   Get status of VIDX mapping for multi-target traffic forwarded to loopback
*         and get a Loopback eVIDX offset value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] vidxOffsetPtr            - a Loopback eVIDX offset.
* @param[out] enablePtr                - enable eVIDX mapping.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortLoopbackEvidxMappingGet
(
    IN  GT_U8                devNum,
    OUT GT_U32               *vidxOffsetPtr,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssDxChPortLoopbackEnableEgressMirroringSet function
* @endinternal
*
* @brief   Enables egress mirroring for packets forwarded to loopback.
*          If 'egress mirroring' and 'forwarding to looback' are both enabled
*          on target port a packet destined for this port will go either to
*          loopback port (if this feature is disabled) or to
*          loopback port and analyzer port (if the feature is enabled) instead
*          of target port.
*          NOTE: the feature is relevant for ePort/eVlan egress mirroring.
*          If case of 'Physical Port Egress Mirroring' packet which didn't egress
*          via target port will not be mirrored in any case.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - enable egress mirroring
*                                      GT_FALSE - disable egress mirroring
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is usually used in conjunction with
*       cpssDxChPortLoopbackEvidxMappingSet
*
*/
GT_STATUS cpssDxChPortLoopbackEnableEgressMirroringSet
(
    IN  GT_U8                devNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssDxChPortLoopbackEnableEgressMirroringGet function
* @endinternal
*
* @brief   Get status of egress mirroring for packets forwarded to loopback.
*          If 'egress mirroring' and 'forwarding to looback' are both enabled
*          on target port a packet destined for this port will go either to
*          loopback port (if this feature is disabled) or to
*          loopback port and analyzer port (if the feature is enabled) instead
*          of target port.
*          NOTE: the feature is relevant for ePort/eVlan egress mirroring.
*          If case of 'Physical Port Egress Mirroring' packet which didn't egress
*          via target port will not be mirrored in any case.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE  - egress mirroring is enabled
*                                      GT_FALSE - egress mirroring is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                        GT_FALSE - egress mirroring is disabled
*/
GT_STATUS cpssDxChPortLoopbackEnableEgressMirroringGet
(
    IN  GT_U8                devNum,
    OUT GT_BOOL              *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortLoopbackh */
