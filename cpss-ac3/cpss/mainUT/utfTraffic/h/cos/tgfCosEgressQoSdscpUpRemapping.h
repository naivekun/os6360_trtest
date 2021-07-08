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
* @file tgfCosEgressQoSdscpUpRemapping.h
*
* @brief QoS Egress Interface Base Remapping
*
* @version   2
********************************************************************************
*/


/**
* @internal prvTgfCosEgressQoSdscpUpRemappingBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfCosEgressQoSdscpUpRemappingBridgeConfigSet
(
     GT_VOID
);

/**
* @internal prvTgfCosEgressQoSdscpUpRemappingEgrRemarkingConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] testNumber               -  test number
*                                       None
*/
GT_VOID prvTgfCosEgressQoSdscpUpRemappingEgrRemarkingConfigurationSet
(
    IN GT_U32 testNumber
);

/**
* @internal prvTgfCosEgressQoSdscpUpRemappingIpv4BuildPacket function
* @endinternal
*
* @brief   Build packet
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
GT_VOID prvTgfCosEgressQoSdscpUpRemappingIpv4BuildPacket
(
   GT_VOID
);

/**
* @internal prvTgfCosEgressQoSdscpUpRemappingCheckCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test.
*
* @param[in] capturedPort             - port where output packet is captured.
* @param[in] testNumber               - test number
*                                       None
*/
void prvTgfCosEgressQoSdscpUpRemappingCheckCaptureEgressTrafficOnPort
(
    GT_U32 capturedPort,
    GT_U32 testNumber
);

/**
* @internal prvTgfCosEgressQoSdscpUpRemappingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosEgressQoSdscpUpRemappingTrafficGenerate
(
    IN GT_U32 testNumber
);


/**
* @internal prvTgfCosEgressQoSdscpUpRemappingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosEgressQoSdscpUpRemappingConfigurationRestore
(
    IN GT_U32 testNumber
);











