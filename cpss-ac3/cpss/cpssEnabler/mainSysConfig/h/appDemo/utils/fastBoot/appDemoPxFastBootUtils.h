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
* @file appDemoPxFastBootUtils.h
*
* @brief App demo Px appdemo FastBoot API header.
*
* @version   1
********************************************************************************
*/

#ifndef __appDemoPxFastBootUtilsh
#define __appDemoPxFastBootUtilsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal appDemoPxMacCounterGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Gets Ethernet MAC counter / MAC Captured counter for a
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
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
*       Based on prvCpssPxPortMacCounterGet
*
*/
GT_STATUS appDemoPxMacCounterGet_HardCodedRegAddr
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
);

/**
* @internal appDemoPxCfgIngressDropCntrGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Get the Ingress Drop Counter value.
*
* @note   APPLICABLE DEVICES:      Pipe.
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
*       Based on cpssPxCfgCntrGet
*
*/
GT_STATUS appDemoPxCfgIngressDropCntrGet_HardCodedRegAddr
(
    IN  GT_U8       devNum,
    OUT GT_U32      *counterPtr
);

/**
* @internal appDemoPxIngressPortTargetEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Enable/disable forwarding to the target port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] enable                   -  or disable forwarding to the target port.
*                                      GT_TRUE - Enables packet forwarding to the target port,
*                                      GT_FALSE - Filters all traffic to the target port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssPxIngressPortTargetEnableSet
*
*/
GT_STATUS appDemoPxIngressPortTargetEnableSet_HardCodedRegAddr
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_BOOL                      enable
);

/**
* @internal appDemoPxIngressPortMapEntrySet_HardCodedRegAddr function
* @endinternal
*
* @brief   Set Source/Destination port map table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] entryIndex               - port map entry index.
*                                      Index calculated by packet type source/destination format.
*                                      Destination table -
*                                      (APPLICABLE RANGES: 0..8191)
*                                      Source table -
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] portsBmp                 - the bitmap of ports.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Source, Destination port map tables, LAG designated table and Port Enable
*       map table are used in calculation of egress ports
*       to which the unicast or multicast packet is forwarded.
*       This function is written for pexOnly mode --> these is no register initialization
*       All register addresses are hard coded
*       Based on cpssPxIngressPortMapEntrySet
*
*/
GT_STATUS appDemoPxIngressPortMapEntrySet_HardCodedRegAddr
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_TABLE_ENT       tableType,
    IN  GT_U32                  entryIndex,
    IN  CPSS_PX_PORTS_BMP       portsBmp
);

/**
* @internal appDemoPxPortEgressCntrsGet_HardCodedRegAddr function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @note   APPLICABLE DEVICES:      Pipe.
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
*       Based on cpssPxPortEgressCntrsGet
*
*/
GT_STATUS appDemoPxPortEgressCntrsGet_HardCodedRegAddr
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           cntrSetNum,
    OUT CPSS_PX_PORT_EGRESS_CNTR_STC   *egrCntrPtr
);

/**
* @internal appDemoPxSdmaTxPacketSend_HardCodedRegAddr function
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
GT_STATUS appDemoPxSdmaTxPacketSend_HardCodedRegAddr
(
    IN GT_U8    devNum,
    IN GT_BOOL  isPacketWithVlanTag,
    IN GT_U8    vid,
    IN TGF_PACKET_PAYLOAD_STC  *packetPtr,
    IN GT_U32   dstPortNum
);

/**
* @internal appDemoPxPortForceLinkPassEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
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
*       Based on cpssPxPortForceLinkPassEnableSet
*
*/
GT_STATUS appDemoPxPortForceLinkPassEnableSet_HardCodedRegAddr
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
);

/**
* @internal appDemoPxPortLoopbackModeEnableSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Set port in 'loopback' mode
*
* @note   APPLICABLE DEVICES:      Pipe.
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
*       based on prvCpssPxPortInternalLoopbackEnableSet
*
*/
GT_STATUS appDemoPxPortLoopbackModeEnableSet_HardCodedRegAddr
(
    IN  GT_U8                   devNum,
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN GT_BOOL                   enable
);

/**
* @internal appDemoPxPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure TxQ port 2 TxDMA mapping
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
*       Based on TxQPort2TxDMAMapSet
*
*/
GT_STATUS appDemoPxPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 txqNum,
    IN GT_U32 txDmaNum
);

/**
* @internal appDemoPxPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure BMA local physical port 2 RxDMA port mapping
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
*       Based on BMAMapOfLocalPhys2RxDMASet
*
*/
GT_STATUS appDemoPxPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 physPort,
    IN GT_U32 rxDMAPort
);

/**
* @internal appDemoPxPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr function
* @endinternal
*
* @brief   configure RxDMA 2 phys port mapping
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
*       Based on rxDMA2LocalPhysSet
*
*/
GT_STATUS appDemoPxPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr
(
    IN GT_U8  devNum,
    IN GT_U32 rxDmaNum,
    IN GT_U32 physPort
);

/**
* @internal appDemoPxPipePortMappingConfigSet_HardCodedRegAddr function
* @endinternal
*
* @brief   Pipe Port mapping initial configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
*       Based on prvCpssPxPipePortMappingConfigSet
*
*/
GT_STATUS appDemoPxPipePortMappingConfigSet_HardCodedRegAddr
(
    IN  GT_U8  devNum,
    IN  GT_U32 physPort
);

/**
* @internal appDemoPxPortMappingRestore function
* @endinternal
*
* @brief   Restore port mapping configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/

GT_STATUS appDemoPxPortMappingRestore
(
    IN  GT_U8 devNum
);

/**
* @internal appDemoPxPortFastInit function
* @endinternal
*
* @brief   Fast ports configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoPxPortFastInit
(
    IN  GT_U8 devNum
);

/**
* @internal appDemoPxPortModeSpeedSet function
* @endinternal
*
* @brief   Fast ports configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoPxPortModeSpeedSet
(
    IN  GT_U8 devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoPxFastBootUtilsh */


