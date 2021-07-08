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
* @file tgfVirtualTcamUT.c
*
* @brief Enhanced UTs for Virtual Tcam Init.
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <virtualTcam/prvTgfVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

#include <common/tgfBridgeGen.h>

UTF_TEST_CASE_MAC(prvTgfVirtualTcamPriorityMoveRule)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~UTF_XCAT3_E) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamPriorityMoveRuleTest(GT_FALSE, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E);
    prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore(GT_FALSE);

}

UTF_TEST_CASE_MAC(prvTgfVirtualTcamPriorityMoveRule_80B)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E);
    notAppFamilyBmp &= (~UTF_LION2_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfVirtualTcamPriorityMoveRuleTest(GT_FALSE, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E);
    prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore(GT_FALSE);

}

UTF_TEST_CASE_MAC(prvTgfVirtualTcamPriorityMoveRule_tti)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~UTF_XCAT3_E) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamPriorityMoveRuleTest(GT_TRUE, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E);
    prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore(GT_FALSE);

}

/* AUTODOC: Test - action update under traffic:
    1.  Create tcam manager with vtcam with 10 rules
    2.  Make rule #8 with action to count to CNC index 0 (redirect to single egress port)
    3.  Make rule #9 Make SAME content of rule like #8 with action (bypass bridge + flood)
    4.
    5.  Send continous WS traffic that hit this rule
    6.  Updated the action of rule 8  to count to CNC index 1 (cpssDxChVirtualTcamRuleActionUpdate)
        a.  For simulation do sleep 50 millisec (to allow traffic hit the entry)
    7.  Updated the action of rule 8 to count to CNC index 2
    8.  ..
    9.  Updated the action of rule 8 to count to CNC index 511
    10. Stop the traffic
    11. Check that the CNC summary of indexes 0..511 is equal to number of packets that egress the device.
    12. Check that there was no flooding to other ports.
*/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamUpdateAction)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~UTF_XCAT3_E) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamUpdateActionTest(GT_FALSE);
    prvTgfVirtualTcamUpdateActionConfigurationRestore();

}

/* AUTODOC: Test - action update under traffic:
    1.  Create tcam manager with vtcam with 10 rules
    2.  Make rule #8 with action to count to CNC index 0 (redirect to single egress port)
    3.  Make rule #9 Make SAME content of rule like #8 with action (bypass bridge + flood)
    4.
    5.  Send continous WS traffic that hit this rule
    6.  Updated the action of rule 8  to count to CNC index 1 (cpssDxChVirtualTcamRuleActionUpdate)
        a.  For simulation do sleep 50 millisec (to allow traffic hit the entry)
    7.  Updated the action of rule 8 to count to CNC index 2
    8.  ..
    9.  Updated the action of rule 8 to count to CNC index 511
    10. Stop the traffic
    11. Check that the CNC summary of indexes 0..511 is equal to number of packets that egress the device.
    12. Check that there was no flooding to other ports.
*/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamUpdateAction_tti)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~UTF_XCAT3_E) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamUpdateActionTest(GT_TRUE);
    prvTgfVirtualTcamUpdateActionConfigurationRestore();

}

/* AUTODOC: Test - resize under traffic - logical index mode:
    1.  Create vtcam , guaranteed 10 entries (30B rules)
    2.  Write rule#9 as redirect to egresss port A
    3.  Send WS traffic to hit this entry
    4.  Resize vtcam adding 1000 entries from index 0
    5.  Sleep 100 millisec
    6.  Resize vtcam adding 1000 entries from index 0
        a.  Sleep 100 millisec
    7.  Resize vtcam removing 500 entries from index 0
        a.  Sleep 100 millisec
    8.  Repeat 7. three more times (total 2000 removed)
    9.  Check no flooding in the vlan (all egress port A)
*/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamResize)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~UTF_XCAT3_E) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamResizeTest(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E,
                                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E);
    prvTgfVirtualTcamResizeConfigurationRestore();
}

/* AUTODOC: Test - resize under traffic - 80B rules - logical index mode:
    1.  Create vtcam , guaranteed 10 entries (80B rules)
    2.  Write rule#9 as redirect to egresss port A
    3.  Send WS traffic to hit this entry
    4.  Resize vtcam adding 1000 entries from index 0
    5.  Sleep 100 millisec
    6.  Resize vtcam adding 1000 entries from index 0
        a.  Sleep 100 millisec
    7.  Resize vtcam removing 500 entries from index 0
        a.  Sleep 100 millisec
    8.  Repeat 7. three more times (total 2000 removed)
    9.  Check no flooding in the vlan (all egress port A)
*/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamResize_80B)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E);
    notAppFamilyBmp &= (~UTF_LION2_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfVirtualTcamResizeTest(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E,
                                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E);
    prvTgfVirtualTcamResizeConfigurationRestore();
}

/* AUTODOC: Test - resize under traffic - priority mode:
    1.  Create vtcam , guaranteed 10 entries (30B rules)
    2.  Write rule#9 as redirect to egresss port A
    3.  Send WS traffic to hit this entry
    4.  Resize vtcam adding 1000 entries from index 0
    5.  Sleep 100 millisec
    6.  Resize vtcam adding 1000 entries from index 0
        a.  Sleep 100 millisec
    7.  Resize vtcam removing 500 entries from index 0
        a.  Sleep 100 millisec
    8.  Repeat 7. three more times (total 2000 removed)
    9.  Check no flooding in the vlan (all egress port A)
*/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamResize_PriorityMode)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~UTF_XCAT3_E) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamResizeTest(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E,
                                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E);
    prvTgfVirtualTcamResizeConfigurationRestore();
}

/*******************************************************************************
* prvTgfVirtualTcamPriorityMoveRuleUnderTraffic
*
* DESCRIPTION:
*       Test basic moved rule between priorities (use traffic)-
*       priority mode
* PURPOSE:
*       1. Check that the moved rule between priorities changed actual priority
* AUTODOC:
*       1. Create vTcam
*       2. Set rules with different priorities
*       3. Generate traffic to hit the rules
*       4. Move rules under traffic
*       5. Stop traffic
*       6. Check the CNC counters
*
*******************************************************************************/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamPriorityMoveRuleUnderTraffic)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~UTF_XCAT3_E) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamPriorityMoveRuleUnderTrafficTest();
    prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore(GT_TRUE);
}

/*******************************************************************************
* prvTgfVirtualTcamDragging
*
* DESCRIPTION:
*       Test dragging vTcam under traffic
*       priority mode
* PURPOSE:
*       1. Check that we can create new vTcam after successfull dragging
*          under the traffic
* AUTODOC:
*       1. Create vTcam #100 to fill all Tcam except 2 Blocks
*       2. Create vTcams #1 #2 #3 #4 #10 #11
*       3. Delete vTcams #1 #2 #3 #4.
*       2. Set rules with different priorities in vTcams #10 #11 that would be dragged
*       3. Generate traffic to hit this rules
*       4. create vTcam #20 after dragging vTcam #10 #11
*       5. Stop traffic
*       6. Check the CNC counters
*
*******************************************************************************/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamDragging)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

     /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfVirtualTcamDraggingTest();
    prvTgfVirtualTcamDraggingConfigurationRestore();
#endif
}

/*******************************************************************************
* prvTgfVirtualTcamRuleUpdatePriority
*
* DESCRIPTION:
*       Test checks changing priorities of existing rules
* PURPOSE:
*       1. Check that the existing rule priority updated correctly.
* AUTODOC:
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
*
*******************************************************************************/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamRuleUpdatePriority)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~UTF_XCAT3_E) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamRulePriorityUpdateTrafficTest();
    prvTgfVirtualTcamRulePriorityUpdateConfigurationRestore();
}

/*
 * Configuration of tgfVirtualTcam suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfVirtualTcam)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamPriorityMoveRule)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamPriorityMoveRule_80B)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamPriorityMoveRule_tti)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamPriorityMoveRuleUnderTraffic)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamUpdateAction)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamUpdateAction_tti)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamResize)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamResize_80B)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamResize_PriorityMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamDragging)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamRuleUpdatePriority)

UTF_SUIT_END_TESTS_MAC(tgfVirtualTcam)


