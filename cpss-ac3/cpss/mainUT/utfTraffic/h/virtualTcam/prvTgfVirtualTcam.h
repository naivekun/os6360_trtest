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
* @file prvTgfVirtualTcam.h
*
* @brief VirtualTcam test header file
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfVirtualTcam
#define __prvTgfVirtualTcam

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfVirtualTcamPriorityMoveRuleTest function
* @endinternal
*
* @brief   Test basic moved rule between priorities (use traffic)-
*         priority mode
*         PURPOSE:
*         1. Check that the moved rule between priorities changed actual priority
*         AUTODOC:
*         1. Creat vTcam
*         2. Set rules with diffirent priorites
*         3. Send packet to hit the rules
*         4. Move rules
*         5. Send and check packets to garantee that moving are correct
* @param[in] useTti                   - GT_TRUE: test TTI TCAM, GT_FALSE: test PCL TCAM
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
*                                       None
*/

GT_VOID prvTgfVirtualTcamPriorityMoveRuleTest
(
    GT_BOOL                                         useTti,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize
);

/**
* @internal prvTgfVirtualTcamPriorityMoveRuleUnderTrafficTest function
* @endinternal
*
* @brief   Test basic moved rule between priorities (use traffic)-
*         priority mode
*         PURPOSE:
*         1. Check that the moved rule between priorities changed actual priority
*         AUTODOC:
*         1. Creat vTcam
*         2. Set rules with diffirent priorites
*         3. Generate traffic to hit the rules
*         4. Move rules under traffic
*         5. Stop traffic
*         6. Check the CNC counters
*/

GT_VOID prvTgfVirtualTcamPriorityMoveRuleUnderTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration function
*         PURPOSE:
*         Restore configuration after prvTgfVirtualTcamPriorityMoveRuleTest
*         AUTODOC:
*         1. Deleting rules
*         2. Deleting vTcam
*         3. Deleting vTcam manager
*         4. restore vLan
* @param[in] cncUsed                  - to include CNC for cheking in
*                                      prvTgfVirtualTcamMoveRulePriorityUnderTrafficTest
*                                       None
*/
GT_VOID prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore
(
    GT_BOOL cncUsed
);


/**
* @internal prvTgfVirtualTcamUpdateActionTest function
* @endinternal
*
* @brief   Test action update under traffic (use traffic)-
*         priority mode
*         PURPOSE:
*         1. Check that update action of rules under traffic not losing packets.
*         AUTODOC:
*         1. Create tcam manager with vtcam with 10 rules
*         2. Make rule #8 with action to count to CNC index 0 (redirect to single egress port)
*         3. Make rule #9 Make SAME content of rule like #8 with action (bypass bridge + flood)
*         4.
*         5. Send continues WS traffic that hit this rule
*         6. Updated the action of rule 8 to count to CNC index 1 (cpssDxChVirtualTcamRuleActionUpdate)
*         a. For simulation do sleep 50 millisec (to allow traffic hit the entry)
*         7. Updated the action of rule 8 to count to CNC index 2
*         8. …
*         9. Updated the action of rule 8 to count to CNC index 511
*         10. Stop the traffic
*         11. Check that the CNC summary of indexes 0..511 is equal to number of packets that egress the device.
*         12. Check that there was no flooding to other ports.
* @param[in] useTti                   - GT_TRUE: test TTI TCAM, GT_FALSE: test PCL TCAM
*                                       None
*/

GT_VOID prvTgfVirtualTcamUpdateActionTest
(
    GT_BOOL                                         useTti
);

/**
* @internal prvTgfVirtualTcamUpdateActionConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration function
*         PURPOSE:
*         Restore configuration after prvTgfVirtualTcamUpdateActionTest
*         AUTODOC:
*         1. Deleting rules
*         2. Deleting vTcam
*         3. Deleting vTcam manager
*         4. restore vLan
*/
GT_VOID prvTgfVirtualTcamUpdateActionConfigurationRestore
(
    GT_VOID
);


/**
* @internal prvTgfVirtualTcamResizeTest function
* @endinternal
*
* @brief   Check that resize under traffic not losing packets.-
*         logical index mode
*         PURPOSE:
*         1. Check that resize under traffic not losing packets.
*         AUTODOC:
*         1. Create vtcam , guaranteed 10 entries (30B rules)
*         2. Write rule#9 as redirect to egresss port A
*         3. Send WS traffic to hit this entry
*         4. Resize vtcam adding 1000 entries from index 0
*         5. Sleep 100 millisec
*         6. Resize vtcam adding 1000 entries from index 0
*         a. Sleep 100 millisec
*         7. Resize vtcam removing 500 entries from index 0
*         a. Sleep 100 millisec
*         8. Repeat 7. three more times (total 2000 removed)
*         9. Check no flooding in the vlan (all egress port A)
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
* @param[in] ruleAdditionMethod       - Either priority or logical index method
*                                       None
*/

GT_VOID prvTgfVirtualTcamResizeTest
(
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT ruleAdditionMethod
);

/**
* @internal prvTgfVirtualTcamResizeConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration function
*         PURPOSE:
*         Restore configuration after prvTgfVirtualTcamResizeTest
*         AUTODOC:
*         1. Deleting rules
*         2. Deleting vTcam
*         3. Deleting vTcam manager
*         4. restore vLan
*/
GT_VOID prvTgfVirtualTcamResizeConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamDraggingTest function
* @endinternal
*
*/
GT_VOID prvTgfVirtualTcamDraggingTest
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamDraggingConfigurationRestore function
* @endinternal
*
* @brief   Virtual Tcam Dragging configurations restore
*/
GT_VOID prvTgfVirtualTcamDraggingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamRulePriorityUpdateTrafficTest function
* @endinternal
*
* @brief   Test checks changing priorities of existing rules (use traffic)-
*         1. Create vTcam, Configure PCL and CNC
*         2. Set 5 rules with priorities 101-105 counting macthes in different CNC counters
*         3. Clean CNC counters
*         4. Generate traffic to hit the rules
*         5. Check and clean CNC counters
*         6. Change priority of each rule to 150 aligning to first.
*         7. Generate traffic to hit the rules
*         8. Check and clean CNC counters
*         9. Change priority of each rule to 50 aligning to last.
*        10. Generate traffic to hit the rules
*        11. Check and clean CNC counters
*/
GT_VOID prvTgfVirtualTcamRulePriorityUpdateTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamRulePriorityUpdateConfigurationRestore function
* @endinternal
*
* @brief   Virtual Tcam Update Rules Priority Test configurations restore
*
*/
GT_VOID prvTgfVirtualTcamRulePriorityUpdateConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfVirtualTcam */


