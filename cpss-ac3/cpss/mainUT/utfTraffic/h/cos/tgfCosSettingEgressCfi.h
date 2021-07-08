/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfCosEgressQoSdscpUpRemapping.h
*
* DESCRIPTION:
*       Egress CFI settings.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/


/**
* @internal prvTgfCosSettingEgressTagCfiFieldBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*
* @param[in] testNumber               - number of test
* @param[in] vlanTagType              - vlan tag0 or vlan tag1
*                                       None
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldBridgeConfigSet
(
     GT_U32 testNumber,
     GT_U32 vlanTagType
);

/**
* @internal prvTgfCosSettingEgressTagCfiFieldBuildPacket function
* @endinternal
*
* @brief   Build packet
*
* @param[in] testNumber               - number of test
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Packet description:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldBuildPacket
(
   GT_U32 testNumber
);

/**
* @internal prvTgfCosSettingEgressTagCfiFieldCheckCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test.
*
* @param[in] capturedPort             - port where output packet is captured.
* @param[in] testNumber               - test number
* @param[in] cfiValue                 - CFI bit
*                                       None
*/
void prvTgfCosSettingEgressTagCfiFieldCheckCaptureEgressTrafficOnPort
(
    GT_U32 capturedPort,
    GT_U32 testNumber,
    GT_U32 cfiValue
);


/**
* @internal prvTgfCosSettingEgressTagCfiFieldTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
* @param[in] testNumber               - test number
* @param[in] vlanTagType              - vlan tag type
*                                       None
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldTrafficGenerate
(
    IN GT_U32 testNumber,
    IN GT_U32 vlanTagType
);

/**
* @internal prvTgfCosSettingEgressTagCfiFieldConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] testNumber               - test number
* @param[in] vlanTagType              - vlan tag type
*                                       None
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldConfigurationRestore
(
    IN GT_U32 testNumber,
    IN GT_U32 vlanTagType
);



/**
* @internal prvTgfCosSettingEgressTagCfiFieldDpToCfiMappingSet function
* @endinternal
*
* @brief   Set DP to CFI mapping configuration
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldDpToCfiMappingSet
(
     GT_VOID
);


/**
* @internal prvTgfCosSettingEgressTagCfiFieldPclConfigSet function
* @endinternal
*
* @brief   Set PCL configuration
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldPclConfigSet
(
     GT_VOID
);



/**
* @internal prvTgfCosSettingEgressTagCfiFieldTunnelConfigSet function
* @endinternal
*
* @brief   Build packet
*
* @param[in] testNumber               - number of test
* @param[in] tunneVlanTagState        - tunnel vlan tag state
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Packet description:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldTunnelConfigSet
(
    GT_U32 testNumber,
    GT_BOOL tunneVlanTagState

);

