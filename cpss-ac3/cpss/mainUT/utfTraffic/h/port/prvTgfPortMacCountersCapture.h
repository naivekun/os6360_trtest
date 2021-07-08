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
* @file prvTgfPortMacCountersCapture.h
*
* @brief Specific Port MAC Counters features testing
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfPortMacCountersCaptureh
#define __prvTgfPortMacCountersCaptureh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfPortMacCountersVlanConfigurationSet function
* @endinternal
*
* @brief   Set Default Vlan Port MAC Counters configuration.
*/
GT_VOID prvTgfPortMacCountersVlanConfigurationSet
(
    GT_VOID    
);

/**
* @internal prvTgfPortMacCountersToCpuVlanConfigurationSet function
* @endinternal
*
* @brief   Set Vlan Port MAC Counters configuration for traffic to CPU test.
*/
GT_VOID prvTgfPortMacCountersToCpuVlanConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPortToCpuTraffic function
* @endinternal
*
* @brief   Generate and test traffic
*
* @param[in] cpuPortEnabled           - GT_TRUE - cpu port enabled - traffic expected
*                                      GT_FALSE - otherwise
*                                       None
*/
GT_VOID prvTgfPortToCpuTraffic
(
    GT_BOOL cpuPortEnabled
);

/**
* @internal prvTgfPortMacCountersConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfPortMacCountersConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPortMacCountersRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPortMacCountersRestore
(
    GT_VOID
);

/**
* @internal prvTgPortMacCounterCaptureBroadcastTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0packets:
*         macDa = ff:ff:ff:ff:ff:ff,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         MAC MIB Captured counters:
*         1 Broadcast Packet sent from port 0.
*         1 Broadcast Packets received on ports 8, 18 and 23.
*/
GT_VOID prvTgPortMacCounterCaptureBroadcastTrafficGenerate
(
    void
);

/**
* @internal prvTgPortMacCounterCaptureUnicastTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0packets:
*         macDa = 00:00:00:00:11:11,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         MAC MIB Captured counters:
*         1 Unicast Packet sent from port 18.
*         1 Unicast Packets received on ports 0, 8 and 23.
*/
GT_VOID prvTgPortMacCounterCaptureUnicastTrafficGenerate
(
    void
);

/**
* @internal prvTgPortMacCounterCaptureMulticastTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0packets:
*         macDa = 01:02:03:04:05:06,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         MAC MIB Captured counters:
*         1 Multicast Packet sent from port 0.
*         1 Multicast Packets received on ports 8, 18 and 23.
*/
GT_VOID prvTgPortMacCounterCaptureMulticastTrafficGenerate
(
    void
);

/**
* @internal prvTgPortMacCounterCaptureOversizeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0 packets:
*         macDa = 00:00:00:00:11:11,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         MAC MIB Captured counters:
*         1 Unicast Packet sent from port 0.
*         1 Oversize Packets received on ports 8, 18 and 23.
*/
GT_VOID prvTgPortMacCounterCaptureOversizeTrafficGenerate
(
    void
);

/**
* @internal prvTgPortMacCounterCaptureHistogramTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0packets:
*         macDa = 01:02:03:04:05:06,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         Histogram Captured counters:
*         Check, that when the feature is disabled, the counters are not updated.
*         Check, that when the feature is enabled, the counters are updated.
*         Check for different packet sizes:
*         packetSize = 64.
*         64<packetSize<128
*         127<packetSize<256
*         255<packetSize<512
*         511<packetSize<1024
*         1023<packetSize
*/
GT_VOID prvTgPortMacCounterCaptureHistogramTrafficGenerate
(
    void
);

/**
* @internal prvTgPortMacCounterClearOnReadTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0 packets:
*         macDa = 00:00:00:00:11:11,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         MAC MIB counters:
*         Check that when the feature enabled, counters are cleared on read.
*         Check that when the feature disabled, counters are not cleared on read.
*/
GT_VOID prvTgPortMacCounterClearOnReadTrafficGenerate
(
    void
);

/**
* @internal prvTgPortMacCounterCaptureCpuPortDisable function
* @endinternal
*
* @brief   Disable CPU port and send traffic
*
* @note Only MII CPU port connection disabled
*
*/
GT_VOID prvTgPortMacCounterCaptureCpuPortDisable
(
    void
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortMacCountersCapture */



