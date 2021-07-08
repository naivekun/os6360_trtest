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
* @file tgfCosTcDpRemapping.h
*
* @brief Tc and Dp remapping on stack ports.
*
* @version   1
********************************************************************************
*/


/**
* @internal prvTgfCosTcDpRemappingOnStackPortBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortBridgeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfCosTcDpRemappingOnStackPortQoSConfigSet function
* @endinternal
*
* @brief   Set QoS Configuration
*
* @param[in] testNumber               - number of test
*                                       None
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortQoSConfigSet
(
    IN GT_U32 testNumber
);


/**
* @internal prvTgfCosTcDpRemappingOnStackPortCscdConfigSet function
* @endinternal
*
* @brief   Set Cascade Configuration
*
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortCscdConfigSet
(
    IN GT_U32 testNumber
);


/**
* @internal prvTgfCosTcDpRemappingOnStackPortTcDpRemappingConfigSet function
* @endinternal
*
* @brief   Set Tc and DP Remapping Configuration
*
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortTcDpRemappingConfigSet
(
    IN GT_U32 testNumber
);


/**
* @internal prvTgfTcDpRemappingOnStackPortBuildPacket function
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
GT_VOID prvTgfTcDpRemappingOnStackPortBuildPacket
(
   GT_VOID
);


/**
* @internal prvTgfCosTcDpRemappingOnStackPortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortTrafficGenerate
(
    IN GT_U32 testNumber
);

/**
* @internal prvTgfCosTcDpRemappingOnStackPortConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortConfigurationRestore
(
    IN GT_U32 testNumber
);


/**
* @internal prvTgfCosTcDpRemappingOnStackPortDsaTagSet function
* @endinternal
*
* @brief   1. enable running the test in DSA tag mode.
*         3. set DSA tag type
*/
GT_VOID prvTgfCosTcDpRemappingOnStackPortDsaTagSet
(
    GT_VOID
);

