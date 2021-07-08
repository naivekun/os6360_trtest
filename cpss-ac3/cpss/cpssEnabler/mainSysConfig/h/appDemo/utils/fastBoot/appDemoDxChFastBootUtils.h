/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoDxChFastBootUtils.h
*
* @brief App demo DxCh appdemo FastBoot API header.
*
* @version   1
********************************************************************************
*/

#ifndef __appDemoDxChFastBootUtilsh
#define __appDemoDxChFastBootUtilsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal appDemoDxChMacCounterGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Gets Ethernet MAC counter / MAC Captured counter for a
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number,
*                                      CPU port if getFromCapture is GT_FALSE
* @param[in] cntrName                 - specific counter name
*                                      getFromCapture -  GT_TRUE -  Gets the captured Ethernet MAC counter
*                                      GT_FALSE - Gets the Ethernet MAC counter
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The 10G MAC MIB counters are 64-bit wide.
*       Not supported counters: CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E,
*       CPSS_BadFC_RCV_E, CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E.
*       This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChMacCounterGet
*
*/
GT_STATUS appDemoDxChMacCounterGet_HardCodedRegAddr
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
);

/**
* @internal appDemoDxChCfgIngressDropCntrGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Get the Ingress Drop Counter value.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
*
* @param[out] counterPtr               - (pointer to) Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChCfgIngressDropCntrGet
*
*/
GT_STATUS appDemoDxChCfgIngressDropCntrGet_HardCodedRegAddr
(
    IN  GT_U8       devNum,
    OUT GT_U32      *counterPtr
);

/**
* @internal appDemoDxChMirrorRxPortSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Sets a specific port to be Rx mirrored port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] enable                   - enable/disable Rx mirror on this port
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                      received on a mirrPort are
*                                      mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
*                                      Supported for xCat and above device.
*                                      Used only if forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChMirrorRxPortSet
*
*/
GT_STATUS appDemoDxChMirrorRxPortSet_HardCodedRegAddr
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     mirrPort,
    IN  GT_BOOL         enable,
    IN  GT_U32          index
);

/**
* @internal setOwnDevice_HardCodedRegAddr function
* @endinternal
*
* @brief   Sets the device Device_ID within a Prestera chipset.
*
* @param[in] devNum                   - the device number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*
*/
GT_STATUS setOwnDevice_HardCodedRegAddr
(
    IN  GT_U8           devNum
);

/**
* @internal appDemoDxChMirrorAnalyzerInterfaceSet_HardCodedRegAddr function
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
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChMirrorAnalyzerInterfaceSet
*
*/
GT_STATUS appDemoDxChMirrorAnalyzerInterfaceSet_HardCodedRegAddr
(
    IN GT_U8     devNum,
    IN GT_U32    index,
    IN CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
);

/**
* @internal appDemoDxChMirrorRxGlobalAnalyzerInterfaceIndexSet_HardCodedRegAddr function
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
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet
*
*/
GT_STATUS appDemoDxChMirrorRxGlobalAnalyzerInterfaceIndexSet_HardCodedRegAddr
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable,
    IN GT_U32    index
);

/**
* @internal appDemoDxChIngressCscdPortSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Set ingress port as cascaded/non-cascade .
*         (effect all traffic on ingress pipe only)
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The physical port number.
*                                      NOTE: this port is not covered and is NOT checked for 'mapping'
* @param[in] portRxDmaNum             - The RXDMA number for cascading.
* @param[in] portType                 - cascade  type regular/extended DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChIngressCscdPortSet
*
*/
GT_STATUS appDemoDxChIngressCscdPortSet_HardCodedRegAddr
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_U32                       portRxDmaNum,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
);

/**
* @internal appDemoDxChPortEgressCntrsGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number : 0, 1
*
* @param[out] egrCntrPtr               - (pointer to) structure of egress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChPortEgressCntrsGet
*
*/
GT_STATUS appDemoDxChPortEgressCntrsGet_HardCodedRegAddr
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       cntrSetNum,
    OUT CPSS_PORT_EGRESS_CNTR_STC   *egrCntrPtr
);

/**
* @internal appDemoDxChSdmaTxPacketSend_HardCodedRegAddr function
* @endinternal
*
* @brief   This function sends a single packet.
*         The packet is sent through interface port type to port given by
*         dstPortNum with VLAN vid.
* @param[in] isPacketWithVlanTag      - indication that the packetDataPtr[12..15] hold vlan tag.
* @param[in] vid                      - VLAN ID. (used when isPacketWithVlanTag ==GT_FALSE,
*                                      otherwise taken from packetDataPtr[14..15])
* @param[in] packetPtr                - pointer to the packet data and length in bytes.
* @param[in] dstPortNum               - Destination port number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*
*/
GT_STATUS appDemoDxChSdmaTxPacketSend_HardCodedRegAddr
(
    IN GT_U8    devNum,
    IN GT_BOOL  isPacketWithVlanTag,
    IN GT_U8    vid,
    IN TGF_PACKET_PAYLOAD_STC  *packetPtr,
    IN GT_U32   dstPortNum
);

/**
* @internal appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChPortForceLinkPassEnableSet
*
*/
GT_STATUS appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
);

/**
* @internal appDemoDxChPortLoopbackModeEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Set port in 'loopback' mode
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] enable                   -  / disable (loopback/no loopback)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       based on cpssDxChPortInternalLoopbackEnableSet
*
*/
GT_STATUS appDemoDxChPortLoopbackModeEnableSet_HardCodedRegAddr
(
    IN  GT_U8                   devNum,
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN GT_BOOL                   enable
);

/**
* @internal appDemoDxChPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure TxQ port 2 TxDMA mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] txqNum                   - txq port
* @param[in] txDmaNum                 - txDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortMappingTxQPort2TxDMAMapSet
*
*/
GT_STATUS appDemoDxChPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 txqNum,
    IN GT_U32 txDmaNum
);

/**
* @internal appDemoDxChPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure BMA local physical port 2 RxDMA port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] physPort                 - physical port
*                                      rxDmaNum   - rxDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet
*
*/
GT_STATUS appDemoDxChPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 physPort,
    IN GT_U32 rxDMAPort
);

/**
* @internal appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure RxDMA 2 phys port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] rxDmaNum                 - rxDMA port
* @param[in] physPort                 - physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChPortMappingRxDMA2LocalPhysSet
*
*/
GT_STATUS appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 rxDmaNum,
    IN GT_U32 physPort
);

/**
* @internal appDemoDxChBobcat2PortMappingConfigSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Bobcat2 Port mapping initial configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The Pp's device number.
* @param[in] physPort                 - physical port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on prvCpssDxChBobcat2PortMappingConfigSet
*
*/
GT_STATUS appDemoDxChBobcat2PortMappingConfigSet_HardCodedRegAddr
(
    IN  GT_U8  devNum,
    IN  GT_U32 physPort
);

/**
* @internal appDemoDxChBrgEportToPhysicalPortTargetMappingTableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Set physical info for the given ePort in the E2PHY mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*       This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssDxChBrgEportToPhysicalPortTargetMappingTableSet
*
*/
GT_STATUS appDemoDxChBrgEportToPhysicalPortTargetMappingTableSet_HardCodedRegAddr
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_INTERFACE_INFO_STC          *physicalInfoPtr
);

/**
* @internal appDemoDxChPortFastInit function
* @endinternal
*
* @brief   Fast ports configuration
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2: xCat3; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDxChPortFastInit
(
    IN  GT_U8 devNum
);

/**
* @internal appDemoDxChPortModeSpeedSet function
* @endinternal
*
* @brief   Fast ports configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDxChPortModeSpeedSet
(
    IN  GT_U8 devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoDxChFastBootUtilsh */


