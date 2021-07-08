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
* @file prvTgfConfigInit.h
*
* @brief CPSS Config.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfConfigInit
#define __prvTgfConfigInit

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfCfgIngressDropCounterSet function
* @endinternal
*
* @brief   Set test configuration:
*         Set configuration to check ingress drop counter.
*/
GT_VOID prvTgfCfgIngressDropCounterSet
(
    GT_VOID
);

/**
* @internal prvTgfCfgIngressDropCounterTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 1.
*         - Set Ingress drop counter mode to be port mode and
*         set port number to 0.
*         Set Ingress drop counter to 0.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 0.
*         - Set set port number to 8.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 1.
*         - Set Ingress drop counter mode to be port mode and
*         set port number to 0.
*         Set Ingress drop counter to 0.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 0.
*         - Set Ingress drop counter mode to be vlan mode and
*         set vlan number to 0.
*         Set Ingress drop counter to 0.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 0.
*         - Set Ingress drop counter mode to be vlan mode and
*         set vlan number to 5.
*         Set Ingress drop counter to 0.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 1.
*         - Set Ingress drop counter mode to be ALL.
*         Set Ingress drop counter to 0.
*         Send to device's port 8 packet:
*         macDa = 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packet was received.
*         Ingress drop counter is 1.
*/
GT_VOID prvTgfCfgIngressDropCounterTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCfgIngressDropCounterRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCfgIngressDropCounterRestore
(
    GT_VOID
);

/**
* @internal prvTgfCfgProbePacketBasicConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfCfgProbePacketBasicConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfCfgProbePacketPclConfigSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfCfgProbePacketPclConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfCfgProbePacketTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfCfgProbePacketTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCfgProbePacketConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCfgProbePacketConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfCfgProbePacketSourceIdUpdate function
* @endinternal
*
* @brief  Source-Id probe packet bit location update.
*/
GT_VOID prvTgfCfgProbePacketSourceIdUpdate
(
    GT_VOID
);

/**
* @internal prvTgfCfgProbePacketEgressPclConfigSet function
* @endinternal
*
* @brief  Configure Egress PCL Rule
*/

GT_VOID prvTgfCfgProbePacketEgressPclConfigSet
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfConfigInith */

