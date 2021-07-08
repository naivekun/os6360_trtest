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
* @file cpssDxChMirror.h
*
* @brief CPSS DxCh Mirror APIs.
*
*
* @version   22
********************************************************************************
*/
#ifndef __cpssDxChMirrorh
#define __cpssDxChMirrorh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>

/**
* @struct CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC
 *
 * @brief used to describe Analyzer Vlan Tag
*/
typedef struct{

    /** @brief The Vlan ether type of the VLAN tag added to packet
     *  forwarded to Analyzer
     */
    GT_U16 etherType;

    /** The VPT of the VLAN tag added to packet forwarded to Analyzer (APPLICABLE RANGES: 0..7) */
    GT_U8 vpt;

    /** @brief The CFI of the VLAN tag added to packet forwarded to
     *  Analyzer
     */
    GT_U8 cfi;

    /** The VLAN ID of the VLAN tag added to packet forwarded to Analyzer */
    GT_U16 vid;

} CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC;

/* deprecated legacy names that should not be used ,
    and replaced by CPSS_CSCD_PORT_DSA_MODE_1_WORD_E ,
    and CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E and
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E */
enum
{
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E = 1
};


/**
* @enum CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT
 *
 * @brief This enum defines mode of forwarding To Analyzer packets.
*/
typedef enum{

    /** @brief DSA tag holds
     *  the Device/Port trigerring parameters. The forwarding decision of the
     *  To Analyzer packet is performed at each hop according to the local
     *  configuration of the ingress/egress analyzer.
     *  CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E - DSA tag holds
     *  the mirroring Device/Port destination. The destination analyzer is
     *  determined at the triggered mirroring device
     */
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E,

    /** @brief Backward compatible mode. The Src/Trg fields in the DSA tag are
     *  overwritten with the target analyzer interface.
     */
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E,

    /** @brief The Src/Trg
     *  fields are separate fields than the target analyzer.
     *  This mode is possible only in eDSA format.
     */
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E

} CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT;

/**
* @enum CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT
 *
 * @brief This enum defines mode of Enhanced Mirroring Priority.
*/
typedef enum{

    /** @brief TDM arbiter is
     *  selected (no client receives strict priority over the others).
     */
    CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_TDM_E,

    /** @brief Ingress
     *  and Egress mirroring receive strict priority over other
     *  clients (regular received, not mirrored, traffic).
     */
    CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_SP_INGRESS_EGRESS_MIRROR_E

} CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT;

/**
* @enum CPSS_DXCH_MIRROR_EGRESS_MODE_ENT
 *
 * @brief This enum defines mode of Egress Mirroring.
*/
typedef enum{

    /** @brief Egress Mirror packets
     *  that are not DROPPED
     */
    CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E,

    /** @brief Otherwise there are three options in case that packets are dropped:
     *  Egress Mirror packets
     *  on TAIL-DROP
     */
    CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E,

    /** @brief Egress Mirror packets
     *  on CONGESTION
     */
    CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E,

    /** @brief Egress Mirror packets
     *  on CONGESTION or TAIL-DROP
     */
    CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E

} CPSS_DXCH_MIRROR_EGRESS_MODE_ENT;

/* maximal index of analyzer */
#define  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS 6

/**
* @struct CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC
 *
 * @brief used to describe Analyzer interface.
*/
typedef struct{

    /** @brief Analyzer interface.
     *  Only CPSS_INTERFACE_PORT_E interface type is supported.
     */
    CPSS_INTERFACE_INFO_STC interface;

} CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC;

/**
 * @enum CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT
 *
 * @brief This enum defines mode of congestion limits used
 * during mirror on congestion.
*/
typedef enum{

    /** @brief QCN limits used for mirror on congestion.
     */
    CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E,

    /** @brief ECN limits used for mirror on congestion.
     */
    CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_ECN_E

} CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT;

/**
* @internal cpssDxChMirrorAnalyzerVlanTagEnable function
* @endinternal
*
* @brief   Enable/Disable Analyzer port adding additional VLAN Tag to mirrored
*         packets. The content of this VLAN tag is configured
*         by cpssDxChMirrorTxAnalyzerVlanTagConfig.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ports number to be set
* @param[in] enable                   - GT_TRUE - additional VLAN tag inserted to mirrored packets
*                                      GT_FALSE - no additional VLAN tag inserted to mirrored packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerVlanTagEnable
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChMirrorAnalyzerVlanTagEnableGet function
* @endinternal
*
* @brief   Get Analyzer port VLAN Tag to mirrored packets mode.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ports number to be set
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - additional VLAN tag inserted to mirrored packets
*                                      GT_FALSE - no additional VLAN tag inserted to mirrored packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerVlanTagEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChMirrorTxAnalyzerVlanTagConfig function
* @endinternal
*
* @brief   Set global Tx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxAnalyzerVlanTagConfig
(
    IN GT_U8                                       devNum,
    IN CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
);

/**
* @internal cpssDxChMirrorTxAnalyzerVlanTagConfigGet function
* @endinternal
*
* @brief   Get global Tx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxAnalyzerVlanTagConfigGet
(
    IN GT_U8                                       devNum,
    OUT CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
);

/**
* @internal cpssDxChMirrorRxAnalyzerVlanTagConfig function
* @endinternal
*
* @brief   Set global Rx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxAnalyzerVlanTagConfig
(
    IN GT_U8                                       devNum,
    IN CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
);

/**
* @internal cpssDxChMirrorRxAnalyzerVlanTagConfigGet function
* @endinternal
*
* @brief   Get global Rx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerVlanTagConfigPtr - Pointer to analyzer Vlan tag configuration:
*                                      EtherType, VPT, CFI and VID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxAnalyzerVlanTagConfigGet
(
    IN GT_U8                                       devNum,
    OUT CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *analyzerVlanTagConfigPtr
);

/**
* @internal cpssDxChMirrorTxAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a port to be an analyzer port of tx mirrored
*         ports on all system pp's.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] analyzerPort             - port number of analyzer port
* @param[in] analyzerHwDev            - pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxAnalyzerPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     analyzerPort,
    IN  GT_HW_DEV_NUM   analyzerHwDev
);

/**
* @internal cpssDxChMirrorTxAnalyzerPortGet function
* @endinternal
*
* @brief   Gets the analyzer port of tx mirrored ports on all system pp's.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] analyzerPortPtr          - (pointer to)port number of analyzer port
* @param[out] analyzerHwDevPtr         - (pointer to)pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxAnalyzerPortGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *analyzerPortPtr,
    OUT GT_HW_DEV_NUM   *analyzerHwDevPtr
);

/**
* @internal cpssDxChMirrorRxAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a specific port to be an analyzer port of Rx mirrored
*         ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] analyzerPort             - port number of analyzer port
* @param[in] analyzerHwDev            - pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxAnalyzerPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     analyzerPort,
    IN  GT_HW_DEV_NUM   analyzerHwDev
);

/**
* @internal cpssDxChMirrorRxAnalyzerPortGet function
* @endinternal
*
* @brief   Gets the analyzer port of Rx mirrored ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] analyzerPortPtr          - (pointer to)port number of analyzer port
* @param[out] analyzerHwDevPtr         - (pointer to)pp HW Device number of analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxAnalyzerPortGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *analyzerPortPtr,
    OUT GT_HW_DEV_NUM   *analyzerHwDevPtr
);

/**
* @internal cpssDxChMirrorTxCascadeMonitorEnable function
* @endinternal
*
* @brief   One global bit that is set to 1 when performing egress mirroring or
*         egress STC of any of the cascading ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] enable                   - enable/disable Egress Monitoring on cascading ports.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxCascadeMonitorEnable
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
);

/**
* @internal cpssDxChMirrorTxCascadeMonitorEnableGet function
* @endinternal
*
* @brief   Get One global bit that indicate performing egress mirroring or
*         egress STC of any of the cascading ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] enablePtr                - enable/disable Egress Monitoring on cascading ports.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxCascadeMonitorEnableGet
(
    IN   GT_U8   devNum,
    OUT  GT_BOOL *enablePtr
);

/**
* @internal cpssDxChMirrorRxPortSet function
* @endinternal
*
* @brief   Sets a specific port to be Rx mirrored port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                  - the device number
* @param[in] mirrPort                - port number, CPU port supported.
* @param[in] isPhysicalPort          - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
* @param[in] enable                  - enable/disable Rx mirror on this port
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                      received on a mirrPort are
*                                      mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[in] index                   - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*                                      Supported for xCat and above device.
*                                      In xCat3 the parameter is used only if
*                                      forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         isPhysicalPort,
    IN  GT_BOOL         enable,
    IN  GT_U32          index
);

/**
* @internal cpssDxChMirrorRxPortGet function
* @endinternal
*
* @brief   Gets status of Rx mirroring (enabled or disabled) of specific port
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
*
* @param[out] enablePtr              - (pointer to) Rx mirror mode
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                                received on a mirrPort are
*                                                mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[out] indexPtr               - (pointer to) Analyzer destination interface index.
*                                      Supported for xCat and above device.
*                                      In xCat3 the parameter is used only if
*                                      forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxPortGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         isPhysicalPort,
    OUT GT_BOOL         *enablePtr,
    OUT GT_U32          *indexPtr
);

/**
* @internal cpssDxChMirrorTxPortSet function
* @endinternal
*
* @brief   Enable or disable Tx mirroring per port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                  - the device number
* @param[in] mirrPort                - port number.
* @param[in] isPhysicalPort          - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
* @param[in] enable                  - enable/disable Tx mirror on this port
*                                      GT_TRUE - Tx mirroring enabled, packets
*                                      transmitted from a mirrPort are
*                                      mirrored to Tx analyzer.
*                                      GT_FALSE - Tx mirroring disabled.
* @param[in] index                   - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*                                      Supported for xCat and above device.
*                                      In xCat3 the parameter is used only if
*                                      forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         isPhysicalPort,
    IN  GT_BOOL         enable,
    IN  GT_U32          index
);

/**
* @internal cpssDxChMirrorTxPortGet function
* @endinternal
*
* @brief   Get status (enabled/disabled) of Tx mirroring per port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - the device number
*         mirrPort    - port number.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                  - the device number
* @param[in] mirrPort                - port number.
* @param[in] isPhysicalPort          - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      Ignored by other devices, mirrPort is always physical one.
*
* @param[out] enablePtr              - (pointer to) Tx mirror mode on this port
*                                      GT_TRUE - Tx mirroring enabled, packets
*                                                transmitted from a mirrPort are
*                                                mirrored to Tx analyzer.
*                                      GT_FALSE - Tx mirroring disabled.
* @param[out] indexPtr               - (pointer to) Analyzer destination interface index.
*                                      Supported for xCat and above device.
*                                      In xCat3 the parameter is used only if
*                                      forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         isPhysicalPort,
    OUT GT_BOOL         *enablePtr,
    OUT GT_U32          *indexPtr
);

/**
* @internal cpssDxChMirrorTxPortModeSet function
* @endinternal
*
* @brief  Set Egress Mirroring mode to given port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                  - the device number
* @param[in] mirrPort                - port number.
* @param[in] isPhysicalPort          - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
* @param[in] egressMirroringMode     - one of 4 options of the mode enum.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_NUM                     mirrPort,
    IN  GT_BOOL                         isPhysicalPort,
    IN CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode
);

/**
* @internal cpssDxChMirrorTxPortModeGet function
* @endinternal
*
* @brief  Get Egress Mirroring mode to given port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                  - the device number
* @param[in] mirrPort                - port number.
* @param[in] isPhysicalPort          - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
* @param[out] egressMirroringModePtr – (pointer to) Egress mirroring mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortModeGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      mirrPort,
    IN  GT_BOOL                          isPhysicalPort,
    OUT CPSS_DXCH_MIRROR_EGRESS_MODE_ENT *egressMirroringModePtr
);

/**
* @internal cpssDxChMirrRxStatMirroringToAnalyzerRatioSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Rx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] ratio                    - Indicates the  of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Rx mirroring
*       ratio 1 -> all Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       ratio 10 -> 1 in 10 Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssDxChMirrRxStatMirroringToAnalyzerRatioSet
(
    IN GT_U8    devNum,
    IN GT_U32   ratio
);

/**
* @internal cpssDxChMirrRxStatMirroringToAnalyzerRatioGet function
* @endinternal
*
* @brief   Get the statistical mirroring rate in the Rx Analyzer port
*         Get Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] ratioPtr                 - (pointer to) Indicates the ratio of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Rx mirroring
*       ratio 1 -> all Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       ratio 10 -> 1 in 10 Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssDxChMirrRxStatMirroringToAnalyzerRatioGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *ratioPtr
);

/**
* @internal cpssDxChMirrRxStatMirrorToAnalyzerEnable function
* @endinternal
*
* @brief   Enable Ingress Statistical Mirroring to the Ingress Analyzer Port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - enable\disable Ingress Statistical Mirroring.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrRxStatMirrorToAnalyzerEnable
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable
);

/**
* @internal cpssDxChMirrRxStatMirrorToAnalyzerEnableGet function
* @endinternal
*
* @brief   Get Ingress Statistical Mirroring to the Ingress Analyzer Port Mode
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) Ingress Statistical Mirroring mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrRxStatMirrorToAnalyzerEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet function
* @endinternal
*
* @brief  Set Ingress statistical Mirroring
*         to the Ingress Analyzer Port including ratio per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[in] enable                   - enable/disable statistical mirroring.
* @param[in] ratio                    - Indicates the  ratio of  mirrored traffic to analyzer port.
*                                       ratio 0 -> no Rx mirroring.
*                                       ratio 1 -> all Rx mirrored packets are forwarded out the Rx Analyzer port.
*                                       ratio 31 -> 1 in 31 Rx mirrored packets are forwarded out the Rx  Analyzer port.
*                                                                                     32
*                                                         The maximum ratio value is 2    - 1
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet
(
    IN GT_U8    devNum,
    IN GT_U32   index,
    IN GT_BOOL  enable,
    IN GT_U32   ratio
);

/**
* @internal cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet function
* @endinternal
*
* @brief  Get Ingress statistical Mirroring
*         to the Ingress Analyzer Port including ratio per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[out] enablePtr               - (pointer to) enable/disable statistical mirroring.
* @param[out] ratio                   - (pointer to) Indicates the  ratio of  mirrored traffic to analyzer port
*                                       ratio 0 -> no Rx mirroring.
*                                       ratio 1 -> all Rx mirrored packets are forwarded out the Rx Analyzer port.
*                                       ratio 31 -> 1 in 31 Rx mirrored packets are forwarded out the Rx  Analyzer port.
*                                                                                     32
*                                                         The maximum ratio value is 2    - 1
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet
(
    IN GT_U8       devNum,
    IN GT_U32      index,
    OUT GT_BOOL    *enablePtr,
    OUT GT_U32     *ratioPtr
);

/**
* @internal cpssDxChMirrorToAnalyzerTruncateSet function
* @endinternal
*
* @brief  Set if to truncate TO_ANALYZER packets per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[in] truncate                 - Indicates if truncate TO_ANALYZER packates to 128B
*                                       GT_TRUE – truncate, GT_FALSE – do not truncate.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChMirrorToAnalyzerTruncateSet
(
    IN GT_U8      devNum,
    IN GT_U32     index,
    IN GT_BOOL    truncate
);

/**
* @internal cpssDxChMirrorToAnalyzerTruncateGet function
* @endinternal
*
* @brief  Get truncate field per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[out] trucnatePtr             - (pointer to) Indicates the  truncate field in TO_ANALYZER packets
*                                       GT_TRUE – truncated, GT_FALSE – not truncated.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChMirrorToAnalyzerTruncateGet
(
    IN  GT_U8      devNum,
    IN  GT_U32     index,
    OUT GT_BOOL    *truncatePtr
);
/**
* @internal cpssDxChMirrRxAnalyzerDpTcSet function
* @endinternal
*
* @brief   The TC/DP assigned to the packet forwarded to the ingress analyzer port due
*         to ingress mirroring to the analyzer port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerDp               - the Drop Precedence to be set
* @param[in] analyzerTc               - traffic class on analyzer port (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrRxAnalyzerDpTcSet
(
    IN GT_U8             devNum,
    IN CPSS_DP_LEVEL_ENT analyzerDp,
    IN GT_U8             analyzerTc
);

/**
* @internal cpssDxChMirrRxAnalyzerDpTcGet function
* @endinternal
*
* @brief   Get TC/DP assigned to the packet forwarded to the ingress analyzer port due
*         to ingress mirroring to the analyzer port cofiguration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerDpPtr            - pointer to the Drop Precedence.
* @param[out] analyzerTcPtr            - pointer to traffic class on analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_BAD_PTR               - wrong pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrRxAnalyzerDpTcGet
(
    IN GT_U8              devNum,
    OUT CPSS_DP_LEVEL_ENT *analyzerDpPtr,
    OUT GT_U8             *analyzerTcPtr
);

/**
* @internal cpssDxChMirrTxAnalyzerDpTcSet function
* @endinternal
*
* @brief   The TC/DP assigned to the packet forwarded to the egress analyzer port due
*         to egress mirroring to the analyzer port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerDp               - the Drop Precedence to be set
* @param[in] analyzerTc               - traffic class on analyzer port (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrTxAnalyzerDpTcSet
(
    IN GT_U8              devNum,
    IN CPSS_DP_LEVEL_ENT  analyzerDp,
    IN GT_U8              analyzerTc
);

/**
* @internal cpssDxChMirrTxAnalyzerDpTcGet function
* @endinternal
*
* @brief   Get TC/DP assigned to the packet forwarded to the egress analyzer port due
*         to egress mirroring to the analyzer port cofiguration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] analyzerDpPtr            - pointer to the Drop Precedence.
* @param[out] analyzerTcPtr            - pointer to traffic class on analyzer port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_BAD_PTR               - wrong pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrTxAnalyzerDpTcGet
(
    IN GT_U8              devNum,
    OUT CPSS_DP_LEVEL_ENT *analyzerDpPtr,
    OUT GT_U8             *analyzerTcPtr
);

/**
* @internal cpssDxChMirrTxStatMirroringToAnalyzerRatioSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Tx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] ratio                    - Indicates the  of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
*
* @note Examples:
*       ratio 0 -> no Tx mirroring
*       ratio 1 -> all Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       ratio 10 -> 1 in 10 Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssDxChMirrTxStatMirroringToAnalyzerRatioSet
(
    IN GT_U8    devNum,
    IN GT_U32   ratio
);

/**
* @internal cpssDxChMirrTxStatMirroringToAnalyzerRatioGet function
* @endinternal
*
* @brief   Get the statistical mirroring rate in the Tx Analyzer port
*         Get Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] ratioPtr                 - (pointer to)Indicates the ratio of egress mirrored to analyzer
*                                      port packets forwarded to the analyzer port. 1 of every 'ratio'
*                                      packets are forwarded to the analyzer port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Tx mirroring
*       ratio 1 -> all Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       ratio 10 -> 1 in 10 Tx mirrored packets are forwarded out the Tx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssDxChMirrTxStatMirroringToAnalyzerRatioGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *ratioPtr
);

/**
* @internal cpssDxChMirrTxStatMirrorToAnalyzerEnable function
* @endinternal
*
* @brief   Enable Egress Statistical Mirroring to the Egress Analyzer Port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - enable\disable Egress Statistical Mirroring.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrTxStatMirrorToAnalyzerEnable
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable
);

/**
* @internal cpssDxChMirrTxStatMirrorToAnalyzerEnableGet function
* @endinternal
*
* @brief   Get mode of Egress Statistical Mirroring to the Egress Analyzer Port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)  Egress Statistical Mirroring.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrTxStatMirrorToAnalyzerEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChMirrorToAnalyzerForwardingModeSet function
* @endinternal
*
* @brief   Set Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     -  of forwarding To Analyzer packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change Forwarding mode, applicaton should
*       disable Rx/Tx mirrorred ports.
*
*/
GT_STATUS cpssDxChMirrorToAnalyzerForwardingModeSet
(
    IN GT_U8     devNum,
    IN CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   mode
);

/**
* @internal cpssDxChMirrorToAnalyzerForwardingModeGet function
* @endinternal
*
* @brief   Get Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - pointer to mode of forwarding To Analyzer packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorToAnalyzerForwardingModeGet
(
    IN  GT_U8     devNum,
    OUT CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChMirrorAnalyzerInterfaceSet function
* @endinternal
*
* @brief   This function sets analyzer interface.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    -  of analyzer interface. (APPLICABLE RANGES: 0..6)
* @param[in] interfacePtr             - Pointer to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index, interface type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on wrong port or device number in interfacePtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerInterfaceSet
(
    IN GT_U8     devNum,
    IN GT_U32    index,
    IN CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
);

/**
* @internal cpssDxChMirrorAnalyzerInterfaceGet function
* @endinternal
*
* @brief   This function gets analyzer interface.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    -  of analyzer interface. (APPLICABLE RANGES: 0..6)
*
* @param[out] interfacePtr             - Pointer to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerInterfaceGet
(
    IN  GT_U8     devNum,
    IN GT_U32     index,
    OUT CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
);

/**
* @internal cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for ingress
*         mirroring from all engines except
*         port-mirroring source-based-forwarding mode.
*         (Port-Based hop-by-hop mode, Policy-Based, VLAN-Based,
*         FDB-Based, Router-Based).
*         If a packet is mirrored by both the port-based ingress mirroring,
*         and one of the other ingress mirroring, the selected analyzer is
*         the one with the higher index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode, Policy-Based,
*                                      VLAN-Based, FDB-Based, Router-Based.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable,
    IN GT_U32    index
);

/**
* @internal cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for ingress mirroring
*         from all engines except port-mirroring source-based-forwarding mode.
*         (Port-Based hop-by-hop mode, Policy-Based, VLAN-Based,
*         FDB-Based, Router-Based).
*         If a packet is mirrored by both the port-based ingress mirroring,
*         and one of the other ingress mirroring, the selected analyzer is
*         the one with the higher index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - Pointer to global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode, Policy-Based,
*                                      VLAN-Based, FDB-Based, Router-Based.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[out] indexPtr                 - pointer to analyzer destination interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
);

/**
* @internal cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable,
    IN GT_U32    index
);

/**
* @internal cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - Pointer to global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[out] indexPtr                 - pointer to analyzer destination interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
);

/**
* @internal cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for
*          egress mirroring for tail dropped/congestion frames.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] egressMirroringMode      - can be one of the following 2 values: tail_drop or congestion
* @param[in] enable                   - global enable/disable egress mirroring
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index or egressMirroringMode is not tail drop or congestion.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode,
    IN GT_BOOL                          enable,
    IN GT_U32                           index
);

/**
* @internal cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for
*          egress mirroring for tail dropped/congestion frames.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] egressMirroringMode      - can be one of the following 2 values: tail_drop or congestion
* @param[out] enablePtr               - pointer to global enable/disable egress mirroring.
* @param[out] indexPtr                - pointer to analyzer interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or egressMirroringMode is not tail drop or congestion.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode,
    OUT GT_BOOL                         *enablePtr,
    OUT GT_U32                          *indexPtr
);

/**
* @internal cpssDxChMirrorAnalyzerMirrorOnDropEnableSet function
* @endinternal
*
* @brief   Enable / Disable mirroring of dropped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
* @param[in] enable                   - GT_TRUE  - dropped packets are mirrored to analyzer interface.
*                                      GT_FALSE - dropped packets are not mirrored to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerMirrorOnDropEnableSet
(
    IN GT_U8     devNum,
    IN GT_U32    index,
    IN GT_BOOL   enable
);

/**
* @internal cpssDxChMirrorAnalyzerMirrorOnDropEnableGet function
* @endinternal
*
* @brief   Get mirroring status of dropped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*
* @param[out] enablePtr                - Pointer to mirroring status of dropped packets.
*                                      - GT_TRUE  - dropped packets are mirrored to
*                                      analyzer interface.
*                                      - GT_FALSE - dropped packets are not mirrored to
*                                      analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerMirrorOnDropEnableGet
(
    IN  GT_U8     devNum,
    IN GT_U32     index,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN tag removal of mirrored traffic.
*         When VLAN tag removal is enabled for a specific analyzer port, all
*         packets that are mirrored to this port are sent without any VLAN tags.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  - VLAN tag is removed from mirrored traffic.
*                                      GT_FALSE - VLAN tag isn't removed from mirrored traffic.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);


/**
* @internal cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet function
* @endinternal
*
* @brief   Get status of enabling/disabling VLAN tag removal of mirrored traffic.
*         When VLAN tag removal is enabled for a specific analyzer port, all
*         packets that are mirrored to this port are sent without any VLAN tags.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - pointer to status of VLAN tag removal of mirrored traffic.
*                                      - GT_TRUE  - VLAN tag is removed from mirrored traffic.
*                                      GT_FALSE - VLAN tag isn't removed from mirrored traffic.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChMirrorEnhancedMirroringPriorityModeSet function
* @endinternal
*
* @brief   Setting Enhanced Mirroring Priority mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Enhanced Mirroring Priority selected working mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorEnhancedMirroringPriorityModeSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT  mode
);

/**
* @internal cpssDxChMirrorEnhancedMirroringPriorityModeGet function
* @endinternal
*
* @brief   Getting Enhanced Mirroring Priority mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) Enhanced Mirroring Priority selected working mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorEnhancedMirroringPriorityModeGet
(
    IN GT_U8                                         devNum,
    OUT CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT  *modePtr
);

/**
* @internal cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet function
* @endinternal
*
* @brief   Set the drop code to be used for Hop-By-Hop mirroring mode, when a packet
*         is duplicated to a target analyzer, and the global Ingress Analyzer Index
*         or Egress Analyzer Index is DISABLED.
*         In this case the packet is hard dropped with this drop code.
*         NOTE: not relevant to the device that generates the duplication but only
*         to device that will get 'TO_ANALYZER' DSA tag (in HOP-BY_HOP mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] dropCode                 - the drop code. (values like 'cpu code')
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device , bad dropCode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode
);

/**
* @internal cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet function
* @endinternal
*
* @brief   Get the drop code to be used for Hop-By-Hop mirroring mode, when a packet
*         is duplicated to a target analyzer, and the global Ingress Analyzer Index
*         or Egress Analyzer Index is DISABLED.
*         In this case the packet is hard dropped with this drop code.
*         NOTE: not relevant to the device that generates the duplication but only
*         to device that will get 'TO_ANALYZER' DSA tag (in HOP-BY_HOP mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] dropCodePtr              - (pointer to) the drop code. (values like 'cpu code')
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr
);

/**
* @internal cpssDxChMirrorTxPortVlanEnableSet function
* @endinternal
*
* @brief   Enable or disable Tx vlan mirroring from specific physical port.
*         (define if the physical port allow/deny egress vlan mirroring)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - enable/disable Tx vlan mirroring from this physical port
*                                      GT_TRUE  - Tx vlan mirroring enabled  for this physical port.
*                                      GT_FALSE - Tx vlan mirroring disabled for this physical port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device , bad portNum.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortVlanEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssDxChMirrorTxPortVlanEnableGet function
* @endinternal
*
* @brief   Get is the Tx vlan mirroring from specific physical port enabled/disabled.
*         (define if the physical port allow/deny egress vlan mirroring)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to)enable/disable Tx vlan mirroring from this physical port
*                                      GT_TRUE  - Tx vlan mirroring enabled  for this physical port.
*                                      GT_FALSE - Tx vlan mirroring disabled for this physical port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorTxPortVlanEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChMirrorOnCongestionModeSet function
* @endinternal
*
* @brief   Defines which congestion limits are used for
*          mirror-on-congestion(QCN/ECN).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - the device number
* @param[in] congestionMode           - congestion mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device, bad congestionMode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorOnCongestionModeSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT  congestionMode
);

/**
* @internal cpssDxChMirrorOnCongestionModeGet function
* @endinternal
*
* @brief   Get the congestion limits for mirror
*          on congestion(QCN/ECN).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - the device number
* @param[in] congestionModePtr        - (pointer to)congestion mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMirrorOnCongestionModeGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT  *congestionModePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChMirrorh */
