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
* @file prvTgfMirror.h
*
* @brief CPSS Mirror
*
* @version   9
********************************************************************************
*/
#ifndef __prvTgfMirror
#define __prvTgfMirror

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfMirror.h>

/**
* @internal prvTgfIngressMirrorHopByHopConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set analyzer interface for index 4.
*         Device number 0, port number 18, port interface.
*         -    Set analyzer interface for index 0.
*         Device number 0, port number 23, port interface.
*         -    Set analyzer interface index to 4 and enable Rx mirroring.
*         - Enable Rx mirroring on port 8.
*/
GT_VOID prvTgfIngressMirrorHopByHopConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorHopByHopTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Disable Rx mirroring.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*         Set analyzer interface index to 0 and enable Rx mirroring.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         Disable Rx mirroring on port 8.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*/
GT_VOID prvTgfIngressMirrorHopByHopTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorHopByHopConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIngressMirrorHopByHopConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorSourceBasedConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set source-based forwarding mode.
*         - Set analyzer interface for index 6.
*         Device number 0, port number 18, port interface.
*         - Set analyzer interface for index 0.
*         Device number 0, port number 23, port interface.
*         - Enable Rx mirroring on port 8 and set analyzer
*         interface index for the port to 6.
*         - Enable Rx mirroring on port 0 and set analyzer interface
*         index for the port to 0.
*/
GT_VOID prvTgfIngressMirrorSourceBasedConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorSourceBasedTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         Disable Rx mirroring on port 8.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*         Disable Rx mirroring on port 0.
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 8, 18, 23.
*/
GT_VOID prvTgfIngressMirrorSourceBasedTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorSourceBasedConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIngressMirrorSourceBasedConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorHighestIndexSelectionConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set source-based forwarding mode.
*         - Set analyzer interface for index 3.
*         Device number 0, port number 18, port interface.
*         - Set analyzer interface for index 1.
*         Device number 0, port number 23, port interface.
*         - Enable Rx mirroring on port 8 and set analyzer
*         interface index for the port to 1.
*         - Set FDB entry with MAC address 00:00:00:00:00:01
*         and mirrorToRxAnalyzerPortEn = GT_TRUE.
*         -    Set analyzer interface index to 3 and enable Rx mirroring.
*/
GT_VOID prvTgfIngressMirrorHighestIndexSelectionConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorHighestIndexSelectionTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:01,
*         Success Criteria:
*         1 packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         - Set analyzer interface index to 0 and enable Rx mirroring.
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*/
GT_VOID prvTgfIngressMirrorHighestIndexSelectionTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorHighestIndexSelectionConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIngressMirrorHighestIndexSelectionConfigurationRestore
(
    GT_VOID
);
/**
* @internal prvTgfEgressMirrorHopByHopConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set analyzer interface for index 2.
*         Device number 0, port number 18, port interface.
*         -    Set analyzer interface for index 0.
*         Device number 0, port number 23, port interface.
*         - Set analyzer interface index to 2 and enable Tx mirroring.
*         - Enable Tx mirroring on port 0.
*/
GT_VOID prvTgfEgressMirrorHopByHopConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorHopByHopTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Disable Tx mirroring.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*         Set analyzer interface index to 0 and enable Tx mirroring.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         Disable Rx mirroring on port 0.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*/
GT_VOID prvTgfEgressMirrorHopByHopTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorHopByHopConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEgressMirrorHopByHopConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorSourceBasedConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set source-based forwarding mode.
*         -    Set analyzer interface for index 5.
*         Device number 0, port number 17, port interface.
*         - Set analyzer interface for index 0.
*         Device number 0, port number 0, port interface.
*         -    Enable Tx mirroring on port 23 and
*         set analyzer interface index for the port to 5.
*         -    Enable Tx mirroring on port 18 and
*         set analyzer interface index for the port to 0.
*/
GT_VOID prvTgfEgressMirrorSourceBasedConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorSourceBasedTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Learn MACs UC1 and UC2 on Tx mirrored ports 18 and 23
*         Send to device's port 8 two packets:
*         macDa = UC1,macDa = UC2,
*         Success Criteria:
*         1 packet is captured o n ports Tx mirrored 23.
*         1 Packets are captured on port Tx mirrored 18.
*         1 Packets are captured on analyzer port 0.
*         1 Packets are captured on analyzer port 17.
*         Disable Tx mirroring on port 18.
*         Send to device's port 8 two packets:
*         macDa = UC1,macDa = UC2,
*         Success Criteria:
*         1 packet is captured o n ports Tx mirrored 23.
*         1 Packets are captured on port Tx mirrored 18.
*         0 Packets are captured on analyzer port 0.
*         1 Packets are captured on analyzer port 17.
*         Disable Tx mirroring on port 23.
*         Send to device's port 8 two packets:
*         macDa = UC1,macDa = UC2,
*         Success Criteria:
*         1 packet is captured o n ports Tx mirrored 23.
*         1 Packets are captured on port Tx mirrored 18.
*         0 Packets are captured on analyzer port 0.
*         0 Packets are captured on analyzer port 17.
*/
GT_VOID prvTgfEgressMirrorSourceBasedTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorSourceBasedConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEgressMirrorSourceBasedConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorForwardingModeChangeConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set analyzer interface for index 3.
*         Device number 0, port number 18, port interface.
*         -    Set analyzer interface for index 0.
*         Device number 0, port number 23, port interface.
*         -    Set analyzer interface index to 3 and enable Rx mirroring.
*         - Enable Rx mirroring on port 8.
*/
GT_VOID prvTgfIngressMirrorForwardingModeChangeConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorForwardingModeChangeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Disable Rx mirroring on port 8.
*         Set source-based forwarding mode.
*         Enable Rx mirroring on port 8 and set
*         analyzer interface index for the port to 0.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         Disable Rx mirroring on port 8.
*         Set hop-byhop forwarding mode.
*         Enable Rx mirroring on port 8.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*/
GT_VOID prvTgfIngressMirrorForwardingModeChangeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorForwardingModeChangeConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIngressMirrorForwardingModeChangeConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorForwardingModeChangeConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set analyzer interface for index 3.
*         Device number 0, port number 18, port interface.
*         -    Set analyzer interface for index 0.
*         Device number 0, port number 23, port interface.
*         -    Set analyzer interface index to 3 and enable Tx mirroring.
*         - Enable Tx mirroring on port 0.
*/
GT_VOID prvTgfEgressMirrorForwardingModeChangeConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorForwardingModeChangeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Disable Tx mirroring on port 0.
*         Set source-based forwarding mode.
*         Enable Tx mirroring on port 0 and set
*         analyzer interface index for the port to 0.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         Disable Tx mirroring on port 0.
*         Set hop-byhop forwarding mode.
*         Enable Tx mirroring on port 0.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*/
GT_VOID prvTgfEgressMirrorForwardingModeChangeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorForwardingModeChangeConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEgressMirrorForwardingModeChangeConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorVlanTagRemovalConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Create VLAN 5 with all ports.
*         - Remove port 18 from VLAN 5.
*         - Set analyzer interface for index 3.
*         Device number 0, port number 18, port interface.
*         - Enable Rx mirroring on port 8.
*         -    Set analyzer interface index to 3 and enable Rx mirroring.
*         - Enable VLAN tag removal of mirrored traffic.
*/
GT_VOID prvTgfIngressMirrorVlanTagRemovalConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorVlanTagRemovalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         Success Criteria:
*         Untagged packet is captured on port 18.
*         Disable VLAN tag removal of mirrored traffic.
*         Send to device's port 8 packet:
*         Success Criteria:
*         Tagged packet is captured on port 18.
*/
GT_VOID prvTgfIngressMirrorVlanTagRemovalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorVlanTagRemovalConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIngressMirrorVlanTagRemovalConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorVlanTagRemovalConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Create VLAN 5 with all ports.
*         - Remove port 18 from VLAN 5.
*         - Set analyzer interface for index 3.
*         Device number 0, port number 18, port interface.
*         - Enable Rx mirroring on port 8.
*         -    Set analyzer interface index to 3 and enable Rx mirroring.
*         - Enable VLAN tag removal of mirrored traffic.
*/
GT_VOID prvTgfEgressMirrorVlanTagRemovalConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorVlanTagRemovalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         Success Criteria:
*         Untagged packet is captured on port 18.
*         Disable VLAN tag removal of mirrored traffic.
*         Send to device's port 8 packet:
*         Success Criteria:
*         Tagged packet is captured on port 18.
*/
GT_VOID prvTgfEgressMirrorVlanTagRemovalTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorVlanTagRemovalConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEgressMirrorVlanTagRemovalConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorSingleAnalyzerConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set Rx analyzer interface : Device 0, port 18.
*         - Enable Rx mirroring on port 8.
*/
GT_VOID prvTgfIngressMirrorSingleAnalyzerConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorSingleAnalyzerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Disable Rx mirroring.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*/
GT_VOID prvTgfIngressMirrorSingleAnalyzerTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorSingleAnalyzerConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIngressMirrorSingleAnalyzerConfigurationRestore
(
    GT_VOID
);


/**
* @internal prvTgfEgressMirrorSingleAnalyzerConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set Tx analyzer interface : Device 0, port 18.
*         - Enable Tx mirroring on port 0.
*/
GT_VOID prvTgfEgressMirrorSingleAnalyzerConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorSingleAnalyzerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Disable Tx mirroring.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*/
GT_VOID prvTgfEgressMirrorSingleAnalyzerTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorSingleAnalyzerConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEgressMirrorSingleAnalyzerConfigurationRestore
(
    GT_VOID
);


/**
* @internal prvTgfCombinedMirrorSingleAnalyzerConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set Rx analyzer interface : Device 0, port 22.
*         - Set Tx analyzer interface : Device 0, port 18.
*         - remove port 22 from vlan.
*         - Enable Rx mirroring on port 8.
*         - Enable Tx mirroring on port 0.
* @param[in] settingOrder             - setting order:
*                                      0 -
*                                      Set Rx analyzer interface : Device 0, port 22.
*                                      Set Tx analyzer interface : Device 0, port 18.
*                                      1 -
*                                      Set Rx analyzer interface : Device 0, port 18.
*                                      Set Tx analyzer interface : Device 0, port 22.
*                                       None
*/
GT_VOID prvTgfCombinedMirrorSingleAnalyzerConfigurationSet
(
    IN GT_U32   settingOrder
);

/**
* @internal prvTgfCombinedMirrorSingleAnalyzerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         1 Packets are captured on port 22.(port 22 not in vlan)
*         Disable Rx mirroring. (Tx is still ON)
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         0 Packets are captured on port 22.(port 22 not in vlan)
*         Disable Tx mirroring. (Rx already disabled)
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         0 Packets are captured on port 22.(port 22 not in vlan)
*/
GT_VOID prvTgfCombinedMirrorSingleAnalyzerTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCombinedMirrorSingleAnalyzerConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCombinedMirrorSingleAnalyzerConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfMirrorConfigParamsSet function
* @endinternal
*
* @brief   1. enable/disable running the test in DSA tag mode.
*         2. set DSA tag type
*         3. set mirroring mode
* @param[in] dsaTagEnable             - enable/disable running the test in DSA tag mode.
* @param[in] dsaType                  - DSA type.
* @param[in] mirrorMode               - mirroring mode.
*
* @retval GT_TRUE                  - configuration is supported
* @retval GT_FALSE                 - configuration is NOT supported
*/
GT_BOOL prvTgfMirrorConfigParamsSet
(
    IN GT_BOOL                  dsaTagEnable,
    IN TGF_DSA_TYPE_ENT         dsaType,
    IN PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   mirrorMode
);


/**
* @internal prvTgfEgressMirrorAnalyzerIndexConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set Tx analyzer interface : Device 0, port 18.
*         - Enable Tx mirroring on port 0.
*/
GT_VOID prvTgfEgressMirrorAnalyzerIndexConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorAnalyzerIndexTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 8, 23.
*         2 Packets are captured on port 18.
*         Disable Tx mirroring.
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 8, 18, 23.
*/
GT_VOID prvTgfEgressMirrorAnalyzerIndexTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorAnalyzerIndexConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEgressMirrorAnalyzerIndexConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorAnalyzerIndexToCpuConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set Tx analyzer interface : Device 0, port 18.
*         - Enable Tx mirroring on port 0.
*/
GT_VOID prvTgfEgressMirrorAnalyzerIndexToCpuConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorAnalyzerIndexToCpuTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfEgressMirrorAnalyzerIndexToCpuTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorAnalyzerIndexToCpuConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfEgressMirrorAnalyzerIndexToCpuConfigurationRestore
(
    GT_VOID
);


/**
* @internal prvTgfIngressMirrorEportVsPhysicalPortTest function
* @endinternal
*
* @brief   configure and test next (and restore) :
*         Test EPort Vs Physical port ingress mirroring.
*         set eport e1 , ePort e2 both associated with physical port pA.
*         use tti to classify eVlan = 0x10 from port pA as e1
*         use tti to classify eVlan = 0x20 from port pA as e2
*         (all others keep the default eport and not get new assignment)
*         case 1:
*         set pA with rx mirror (analyzer 3)
*         set e1 with rx mirror (analyzer 4)
*         set e2 with rx mirror (analyzer 5)
*         --> check:
*         traffic from e1 generate mirroring --> to analyzer 4.
*         traffic from e2 generate mirroring --> to analyzer 5.
*         other traffic from pA generate mirroring --> to analyzer 3.
*         case 2:
*         unset e1 from rx mirror
*         --> check:
*         traffic from e1 generate mirroring --> to analyzer 3(due to physical).
*         traffic from e2 generate mirroring --> to analyzer 5.
*         other traffic from pA generate mirroring --> to analyzer 3.
*         case 3:
*         unset e2 from rx mirror
*         --> check:
*         traffic from e1 generate mirroring --> to analyzer 3(due to physical).
*         traffic from e2 generate mirroring --> to analyzer 3(due to physical).
*         other traffic from pA generate mirroring --> to analyzer 3.
*         case 4:
*         set pA with rx mirror (analyzer 6)
*         set e1 with rx mirror (analyzer 4)
*         set e2 with rx mirror (analyzer 5)
*         --> check:
*         traffic from e1 generate mirroring --> to analyzer 6(due to physical).
*         traffic from e2 generate mirroring --> to analyzer 6(due to physical).
*         other traffic from pA generate mirroring --> to analyzer 3.
*         case 5:
*         unset pA from rx mirror
*         set e1 with rx mirror (analyzer 4)
*         set e2 with rx mirror (analyzer 5)
*         --> check:
*         traffic from e1 generate mirroring --> to analyzer 4.
*         traffic from e2 generate mirroring --> to analyzer 5.
*         other traffic from pA NOT generate mirroring.
*         case 6:
*         unset e1 from rx mirror
*         --> check:
*         traffic from e1 NOT generate mirroring.
*         traffic from e2 generate mirroring --> to analyzer 5.
*         other traffic from pA NOT generate mirroring.
*         case 7:
*         unset e2 from rx mirror
*         --> check:
*         traffic from e1 NOT generate mirroring.
*         traffic from e2 NOT generate mirroring.
*         other traffic from pA NOT generate mirroring.
*/
GT_VOID prvTgfIngressMirrorEportVsPhysicalPortTest
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorEportVsPhysicalPortTest function
* @endinternal
*
* @brief   configure and test next (and restore) :
*         Test EPort Vs Physical port egress mirroring.
*         Test similar to prvTgfIngressMirrorEportVsPhysicalPort , but for 'egress mirror'
*         and not for 'ingress mirror'
*/
GT_VOID prvTgfEgressMirrorEportVsPhysicalPortTest
(
    GT_VOID
);

/**
* @internal prvTgfEgressVlanMirroringTest function
* @endinternal
*
* @brief   Test eVlan based egress mirroring
*         two cases:
*         1. UC traffic
*         2. Multi-destination traffic e.g. flooding (several ports send mirror replications)
*/
GT_VOID prvTgfEgressVlanMirroringTest
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorAnalyzerIsEportIsVidxTest function
* @endinternal
*
* @brief   Test ingress analyzer that is EPort that is mapped by the E2Phy to physical VIDX.
*         With analyzer (egress ports) vidx members in STP blocking/forward
*/
GT_VOID prvTgfIngressMirrorAnalyzerIsEportIsVidxTest
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorAnalyzerIsEportIsVidxTest function
* @endinternal
*
* @brief   Test egress analyzer that is EPort that is mapped by the E2Phy to physical VIDX.
*         With analyzer (egress ports) vidx members in STP blocking/forward
*/
GT_VOID prvTgfEgressMirrorAnalyzerIsEportIsVidxTest
(
    GT_VOID
);

/**
* @internal prvTgfIngressVlanMirroringTest function
* @endinternal
*
* @brief   Main function of ingress VLAN mirroring test.
*/
GT_VOID prvTgfIngressVlanMirroringTest
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirroringToAnalyzerCounterTest function
* @endinternal
*
* @brief Test to check the counters for mirror to analyzer packet.
*/
GT_VOID prvTgfEgressMirrorToAnalyzerCounterTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfMirror */


