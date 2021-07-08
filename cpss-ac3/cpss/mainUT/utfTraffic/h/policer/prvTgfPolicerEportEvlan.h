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
* @file prvTgfPolicerEportEvlan.h
*
* @brief Specific Policer Eport and Evlan features testing
*
* @version   1.1
********************************************************************************
*/
#ifndef __prvTgfPolicerEportEvlan
#define __prvTgfPolicerEportEvlan

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerStageGen.h>


/**
* @internal prvTgfPolicerEportEvlanConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*/
GT_VOID prvTgfPolicerEportEvlanConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPolicerEportEvlanFlowModeEvlanDisabledTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to FLOW .
*         2. Enable Billing.
*         3. Disable eVlan and ePort metering/counting.
*         4. Configure eVlan Trigger entry for vlan 5:
*         Enable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 10.
*         enable unicast
*         Configure unicast offset to 1 -> Metering Entry [201]
*         5. Enable port index 1 for metering.
*         6. Configure index 3 for Unicast traffic for port 8 -> the index in
*         the Meter Table should be 200 (643 + portArr[portIndex]).
*         7. Configure Meter entry matching to eVlan: cir = 1, cbs = 1 and ebs = 1 and
*         red command no_change.
*         8. Configure Meter entry matching to physical port: cir = 1, cbs = 1 and ebs = 10304.
*         9. Send one unicast packet from port index 1 with vlan 5.
*         10. Check management counters: one packet should be yellow.
*         11. Check Billing counters on index matching to physical port: 64 bytes should be counted
*         for red packets and 0 bytes for other.
*         12. Check Billing counters matching to physical port: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanFlowModeEvlanDisabledTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerEportEvlanFlowModeEvlanEnabledTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to FLOW.
*         2. Enable Billing.
*         3. Enable eVlan and disable ePort metering/counting.
*         4. Configure eVlan Trigger entry for vlan 500:
*         Enable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 10
*         enable unknown unicast
*         Configure unknown unicast offset to 1 -> maxPolicerIdx[stage] - 9
*         5. Enable port index 1 for metering.
*         6. Configure index 3 for Unicast traffic for port 8 -> the index in
*         the Meter Table should be 200 (643 + portArr[portIndex]).
*         7. Configure Meter entry matching to eVlan : cir = 1, cbs = 1 and ebs = 1 and
*         red command no_change.
*         8. Configure Meter entry matching to physical port: cir = 1, cbs = 1 and ebs = 10304.
*         9. Send one unknown unicast packet from port index 1 with vlan 5.
*         10. Check management counters: one packet should be red.
*         11. Check Billing counters on entry matching to eVlan: 64 bytes should be counted
*         for red packets and 0 bytes for other.
*         12. Check Billing counters on matching to physical port: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanFlowModeEvlanEnabledTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerEportEvlanFlowModeTtiActionAndEvlanEnabledTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to FLOW.
*         2. Enable Billing.
*         3. Enable eVlan and disable ePort metering/counting.
*         4. Configure eVlan Trigger entry for vlan 5:
*         Enable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 10
*         enable unknown unicast
*         Configure unknown unicast offset to 1 -> maxPolicerIdx[stage] - 9
*         5. Configure TTI action.
*         6. Configure TTI policer index to 50.
*         7. Configure Meter entry matching to eVlan : cir = 1, cbs = 1 and ebs = 1 and
*         red command no_change.
*         8. Configure Meter entry matching to TTI action: cir = 1, cbs = 10368 and ebs = 1.
*         9. Send two unknown unicast packets from port index 3 with vlan 5.
*         10. Check management counters: two packets should be green.
*         11. Check Billing counters on entry matching to TTI action: 128 bytes should be counted
*         for green packets and 128 bytes for other.
*         12. Check Billing counters on matching to eVlan: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanFlowModeTtiActionAndEvlanEnabledTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);


/**
* @internal prvTgfPolicerEportEvlanPortModeEportEnabledTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to PORT .
*         2. Enable Billing.
*         3. Disable eVlan metering/counting.
*         4. Enable ePort metering/counting.
*         4. Configure ePort Trigger entry for ePort 300:
*         Enable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 40.
*         enable registered multicast
*         Configure registered multicast offset to 4.
*         5. Enable port index 2 for metering.
*         6. Configure index 3 for registered Multicast traffic for port index 2 -> the index in
*         the Meter Table should be (643 + portArr[portIndex]).
*         7. Configure Meter entry matching to ePort: cir = 1, cbs = 1 and ebs = 10368 and
*         red command no_change.
*         8. Configure Meter entry matching to physical port: cir = 1, cbs = 10368 and ebs.
*         9. For Ingress, Configure TTI rule with ePort assignment: port index 2 to ePort 300.
*         10. For Egress, configure E2Phy table: 300 -> port index 0 (recieve port).
*         11. Send two registered multicast packets from port port index 2.
*         12. Check management counters: two packets should be yellow.
*         13. Check Billing counters on index matching to ePort: 128 bytes should be counted
*         for yellow packets and 0 bytes for other.
*         14. Check Billing counters matching to physical port: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanPortModeEportEnabledTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);


/**
* @internal prvTgfPolicerEportEvlanPortModeEportDisabledTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to PORT .
*         2. Enable Billing.
*         3. Disable eVlan metering/counting.
*         4. Disable ePort metering/counting.
*         4. Configure ePort Trigger entry for ePort 300:
*         Enable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 40.
*         enable registered multicast
*         Configure registered multicast offset to 4.
*         5. Enable port index 2 for metering.
*         6. Configure index 3 for registered Multicast traffic for port index 2 -> the index in
*         the Meter Table should be (643 + portArr[portIndex]).
*         7. Configure Meter entry matching to ePort: cir = 1, cbs = 1 and ebs = 10368 and
*         red command no_change.
*         8. Configure Meter entry matching to physical port: cir = 1, cbs = 10368 and ebs = 1.
*         9. For Ingress, Configure TTI rule with ePort assignment: port index 2 to ePort 300.
*         10. For Egress, configure E2Phy table: 300 -> port index 0 (recieve port).
*         11. Send two registered multicast packets from port port index 2.
*         12. Check management counters: two packets should be green.
*         13. Check Billing counters on index matching physical port: 128 bytes should be counted
*         for green packets and 0 bytes for other.
*         14. Check Billing counters matching ePort: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanPortModeEportDisabledTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerEportEvlanFlowModeEvlanAllPktTypesTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to FLOW .
*         2. Enable Billing.
*         3. Enable eVlan metering/counting.
*         4. Disable ePort metering/counting.
*         4. Configure eVlan Trigger entry for eVlan 500:
*         Enable metering
*         Enable counting
*         Configure police Index.
*         Note: Each time all offsets are configured (they all differ),
*         so each one points to another metering entry.
*         5. Configure 6 metering entries, matching each offset, so that the
*         one matching givn packet type should color one packet red, where
*         others should color all packets green.
*         6. Send one specific packet type from port port index 3.
*         7. Check management counters: one packets should be red.
*         8. Check Billing counters on index matching policer index + given offset: 64 bytes should be counted
*         for red packets and 0 bytes for other.
*         9. Check Billing counters matching other metering entries: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
* @param[in] pktTypeToSend            - packet type to send
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanFlowModeEvlanAllPktTypesTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN PRV_TGF_POLICER_STORM_TYPE_ENT pktTypeToSend
);

/**
* @internal prvTgfPolicerEportEvlanConfigurationRestore function
* @endinternal
*
* @brief   Restore Policer per stage test configuration
*
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanConfigurationRestore
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerEportEvlanGlobalConfigurationRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
GT_VOID prvTgfPolicerEportEvlanGlobalConfigurationRestore
(
    void
);

/**
* @internal prvTgfPolicerEportEvlanFlowModeTtiActionBillingCountersTrafficGenerate function
* @endinternal
*
* @brief   1. Configure Policer mode to FLOW.
*         2. Enable Billing.
*         3. Enable eVlan and disable ePort metering/counting.
*         4. Configure eVlan Trigger entry for vlan 5:
*         Disable metering
*         Enable counting
*         police Index = maxPolicerIdx[stage] - 10
*         enable unknown unicast
*         Configure unknown unicast offset to 1 -> maxPolicerIdx[stage] - 9
*         5. Configure TTI action: meter disable, counter enable
*         6. Configure TTI policer index to 50.
*         7. Configure Meter entry matching to TTI action: cir = 1, cbs = 10368 and ebs = 1.
*         8. Send one unknown unicast packet from port index 3 with vlan 5.
*         9. Check management counters: no packets.
*         10. Check Billing counters on entry matching to TTI action: 64 bytes should be counted
*         for green packets and 0 bytes for other.
*         11. Check Billing counters on matching to eVlan: 0 bytes should be counted
*         for all packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerEportEvlanFlowModeTtiActionBillingCountersTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPolicerEportEvlanh */



