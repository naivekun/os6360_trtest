/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfMirrorGen.h
*
* DESCRIPTION:
*       Generic APIs Mirroring.
*
* FILE REVISION NUMBER:
*       $Revision: 11 $
*
*******************************************************************************/
#ifndef __tgfMirrorGenh
#define __tgfMirrorGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/monitor/cpssExMxPmMirror.h>
#endif /* EXMXPM_FAMILY */

#include <extUtils/trafficEngine/tgfTrafficGenerator.h>

/**
* @enum PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT
 *
 * @brief This enum defines mode of forwarding To Analyzer packets.
*/
typedef enum{

    PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E,

    PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E,

    /** @brief The Src/Trg
     *  fields are separate fields than the target analyzer.
     *  This mode is possible only in eDSA format.
     */
    PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E

} PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT;


/**
* @internal prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_BOOL   enable,
    IN GT_U32    index
);

/**
* @internal prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
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
GT_STATUS prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
);

/**
* @internal prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for ingress
*         mirroring for Port-Based hop-by-hop mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_BOOL   enable,
    IN GT_U32    index
);

/**
* @internal prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for ingress
*         mirroring for Port-Based hop-by-hop mode.
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
GT_STATUS prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
);

/**
* @internal prvTgfMirrorAnalyzerInterfaceTypeSet function
* @endinternal
*
* @brief   This function sets analyzer interface type. relevant only to ExMxPm.
*
* @param[in] isTxMirror               - is Tx mirror ?
*                                      GT_TRUE : TX mirror
*                                      GT_FALSE : RX mirror
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvTgfMirrorAnalyzerInterfaceTypeSet
(
    IN GT_BOOL  isTxMirror
);

/**
* @internal prvTgfMirrorAnalyzerInterfaceSet function
* @endinternal
*
* @brief   This function sets analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index, interface type.
* @retval GT_OUT_OF_RANGE          - on wrong port or device number in interfacePtr.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorAnalyzerInterfaceSet
(
    IN GT_U32    index,
    IN PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
);

/**
* @internal prvTgfMirrorAnalyzerInterfaceGet function
* @endinternal
*
* @brief   This function gets analyzer interface.
*
* @param[in] devNum                   - device number.
* @param[in] index                    -  of analyzer interface. (0 - 6)
*
* @param[out] interfacePtr             - Pointer to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorAnalyzerInterfaceGet
(
    IN  GT_U8     devNum,
    IN GT_U32     index,
    OUT PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
);

/**
* @internal prvTgfMirrorToAnalyzerForwardingModeSet function
* @endinternal
*
* @brief   Set Forwarding mode to Analyzer for egress/ingress mirroring.
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
GT_STATUS prvTgfMirrorToAnalyzerForwardingModeSet
(
    IN PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   mode
);

/**
* @internal prvTgfMirrorToAnalyzerForwardingModeGet function
* @endinternal
*
* @brief   Get Forwarding mode to Analyzer for egress/ingress mirroring.
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
GT_STATUS prvTgfMirrorToAnalyzerForwardingModeGet
(
    IN  GT_U8     devNum,
    OUT PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   *modePtr
);

/**
* @internal prvTgfMirrorRxPortSet function
* @endinternal
*
* @brief   Sets a specific port to be Rx mirrored port.
*         NOTE: this is 'eport' mirroring for eArch devices.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorRxPortSet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  enable,
    IN  GT_U32   index
);

/**
* @internal prvTgfMirrorRxPortGet function
* @endinternal
*
* @brief   Gets status of Rx mirroring (enabled or disabled) of specific port
*         NOTE: this is 'eport' mirroring for eArch devices.
*
* @param[out] enablePtr                - (pointer to) Rx mirror mode
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                      received on a mirrPort are
*                                      mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index. (0 - 6)
*                                      Supported for DxChXcat and above device.
*                                      Used only if forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorRxPortGet
(
    IN  GT_PORT_NUM    mirrPort,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *indexPtr
);

/**
* @internal prvTgfMirrorTxPortSet function
* @endinternal
*
* @brief   Sets a specific port to be Tx mirrored port.
*         NOTE: this is 'physical port' mirroring for eArch devices.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorTxPortSet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  enable,
    IN  GT_U32   index
);

/**
* @internal prvTgfMirrorTxPortGet function
* @endinternal
*
* @brief   Get status (enabled/disabled) of Tx mirroring per port .
*         NOTE: this is 'physical port' mirroring for eArch devices.
*
* @param[out] enablePtr                - (pointer to) Tx mirror mode on this port
*                                      GT_TRUE - Tx mirroring enabled, packets
*                                      transmitted from a mirrPort are
*                                      mirrored to Tx analyzer.
*                                      GT_FALSE - Tx mirroring disabled.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index. (0 - 6)
*                                      Supported for DxChXcat and above device.
*                                      Used only if forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorTxPortGet
(
    IN  GT_PORT_NUM    mirrPort,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *indexPtr
);


/**
* @internal prvTgfMirrorRxAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a specific port to be an analyzer port of Rx mirrored
*         ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_HW_ERROR              - on writing to HW error
*/
GT_STATUS prvTgfMirrorRxAnalyzerPortSet
(
    IN  GT_PORT_NUM    analyzerPort,
    IN  GT_U8    analyzerdev
);

/**
* @internal prvTgfMirrorRxAnalyzerPortGet function
* @endinternal
*
* @brief   Gets the analyzer port of Rx mirrored ports
*
* @param[in] devNum                   - the device number
*
* @param[out] analyzerPortPtr          - (pointer to) port number of analyzer port
* @param[out] analyzerDevPtr           - (pointer to) pp Device number of analyzer port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_HW_ERROR              - on writing to HW error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfMirrorRxAnalyzerPortGet
(
    IN  GT_U8                         devNum,
    OUT GT_U8                        *analyzerPortPtr,
    OUT GT_U8                        *analyzerDevPtr
);

/**
* @internal prvTgfMirrorPclAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a specific port to be an analyzer port of Rx mirrored
*         ports
* @param[in] analyzerIndex            - Analyzer index 1..7
* @param[in] analyzerPort             - port number of analyzer port
* @param[in] analyzerdev              - pp Device number of analyzer port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_HW_ERROR              - on writing to HW error
*/
GT_STATUS prvTgfMirrorPclAnalyzerPortSet
(
    IN  GT_U32         analyzerIndex,
    IN  GT_PORT_NUM    analyzerPort,
    IN  GT_U8          analyzerdev
);

/**
* @internal prvTgfMirrorAnalyzerVlanTagRemoveEnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN tag removal of mirrored traffic.
*         When VLAN tag removal is enabled for a specific analyzer port, all
*         packets that are mirrored to this port are sent without any VLAN tags.
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorAnalyzerVlanTagRemoveEnableSet
(
    IN  GT_PORT_NUM    portNum,
    IN GT_BOOL   enable
);

/**
* @internal prvTgfMirrorPortEgressAnalyzerIndexSet function
* @endinternal
*
* @brief   Set the analyzer index for the egress mirrored traffic per egress port.
*         The packet will be either mirrored due to Vlan egress mirroring or
*         mirroring to CPU or due to STC mirroring or egress mirrored traffic
*         per egress port according to highest value of the analyzer index.
* @param[in] portNum                  - port number
* @param[in] index                    - analyzer  for the egress port.
*                                      (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number, port number
* @retval GT_OUT_OF_RANGE          - wrong index
* @retval GT_HW_ERROR              - on writing to HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorPortEgressAnalyzerIndexSet
(
    IN GT_U32       portNum,
    IN GT_U32      index
);

/**
* @internal prvTgfMirrorEgressAnalyzerIndexSet function
* @endinternal
*
* @brief   Set the analyzer index for the egress mirrored traffic.
*         The packet will be either mirrored due to Vlan egress mirroring or
*         mirroring to CPU or due to STC mirroring or egress mirrored traffic
*         per egress port according to highest value of the analyzer index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_OUT_OF_RANGE          - wrong vlanAnalyzerIndex,
*                                       mirrorToCpuAnalyzerIndex, stcAnalyzerIndex
* @retval GT_HW_ERROR              - on writing to HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorEgressAnalyzerIndexSet
(
    IN GT_U32      vlanAnalyzerIndex,
    IN GT_U32      mirrorToCpuAnalyzerIndex,
    IN GT_U32      stcAnalyzerIndex
);

/**
* @internal prvTgfMirrorEgressAnalyzerIndexGet function
* @endinternal
*
* @brief   Get the analyzer index for the egress mirrored traffic.
*         The packet will be either mirrored due to Vlan egress mirroring or
*         mirroring to CPU or due to STC mirroring or egress mirrored traffic
*         per egress port according to highest value of the analyzer index.
* @param[in] devNum                   - device number.
*
* @param[out] vlanAnalyzerIndexPtr     - pointer to the analyzer index attached
*                                      to packets which need to be duplicated
*                                      due to Vlan egress mirroring.
* @param[out] mirrorToCpuAnalyzerIndexPtr - The analyzer index attached to packets
*                                      which need to be duplicated due
*                                      to mirroring to CPU.
* @param[out] stcAnalyzerIndexPtr      - pointer to analyzer index
*                                      for the STC mirrored traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on writing to HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorEgressAnalyzerIndexGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *vlanAnalyzerIndexPtr,
    OUT GT_U32      *mirrorToCpuAnalyzerIndexPtr,
    OUT GT_U32      *stcAnalyzerIndexPtr
);

/**
* @internal prvTgfMirrorRxPortTypeSet function
* @endinternal
*
* @brief   Sets a specific port/eport to be Rx mirrored port.
*         INPUTS:
*         mirrPort - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*                                      Ignored by other devices, mirrPort is always physical one.
* @param[in] enable                   - enable/disable Rx mirror on this port
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                      received on a mirrPort are
*                                      mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (0 - 6)- 6)
*                                      Supported for DxChXcat and above device.
*                                      Used only if forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorRxPortTypeSet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  isPhysicalPort,
    IN  GT_BOOL  enable,
    IN  GT_U32   index
);

/**
* @internal prvTgfMirrorRxPortTypeGet function
* @endinternal
*
* @brief   Gets status of Rx mirroring (enabled or disabled) of specific port/eport
*         INPUTS:
*         mirrPort - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*                                      Ignored by other devices, mirrPort is always physical one.
*
* @param[out] enablePtr                - (pointer to) Rx mirror mode
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                      received on a mirrPort are
*                                      mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index. (0 - 6)
*                                      Supported for DxChXcat and above device.
*                                      Used only if forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorRxPortTypeGet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  isPhysicalPort,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *indexPtr
);

/**
* @internal prvTgfMirrorTxPortTypeSet function
* @endinternal
*
* @brief   Sets a specific port/eport to be Tx mirrored port.
*         INPUTS:
*         mirrPort - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*                                      Ignored by other devices, mirrPort is always physical one.
* @param[in] enable                   - enable/disable Tx mirror on this port
*                                      GT_TRUE - Tx mirroring enabled, packets
*                                      transmitted from a mirrPort are
*                                      mirrored to Tx analyzer.
*                                      GT_FALSE - Tx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (0 - 6)- 6)
*                                      Supported for DxChXcat and above device.
*                                      Used only if forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorTxPortTypeSet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  isPhysicalPort,
    IN  GT_BOOL  enable,
    IN  GT_U32   index
);

/**
* @internal prvTgfMirrorTxPortTypeGet function
* @endinternal
*
* @brief   Get status (enabled/disabled) of Tx mirroring per port/eport .
*         INPUTS:
*         mirrPort - port number.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
* @param[in] mirrPort                 - port number.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*                                      Ignored by other devices, mirrPort is always physical one.
*
* @param[out] enablePtr                - (pointer to) Tx mirror mode on this port
*                                      GT_TRUE - Tx mirroring enabled, packets
*                                      transmitted from a mirrPort are
*                                      mirrored to Tx analyzer.
*                                      GT_FALSE - Tx mirroring disabled.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index. (0 - 6)
*                                      Supported for DxChXcat and above device.
*                                      Used only if forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorTxPortTypeGet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  isPhysicalPort,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *indexPtr
);


/**
* @internal prvTgfMirrorTxPortVlanEnableSet function
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
GT_STATUS prvTgfMirrorTxPortVlanEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfMirrorGenh */


