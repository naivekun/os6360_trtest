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
* @file tgfCommonPolicerUT.c
*
* @brief Enhanced UTs for CPSS Policer
*
* @version   25
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <policer/prvTgfPolicerQosRemarking.h>
#include <policer/prvTgfPolicerVlanCounting.h>
#include <policer/prvTgfPolicerPortModeAddressSelect.h>
#include <policer/prvTgfPolicerEportEvlan.h>
#include <policer/prvTgfPolicerEarchMeterEntry.h>
#include <policer/prvTgfPolicerEgressUpModify.h>
#include <policer/prvTgfPolicerDsaTagCountingMode.h>
#include <policer/prvTgfPolicerEgressDscpModify.h>
#include <policer/prvTgfPolicerEgressExpModify.h>
#include <policer/prvTgfPolicerECNMarkingCheck.h>
#include <policer/prvTgfPolicerMirroring.h>
#include <common/tgfPolicerStageGen.h>

/* traffic generation sending port */
#define ING_PORT_IDX_CNS    0

/* target port */
#define EGR_PORT_IDX_CNS    1

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ingress0 policer QoS remarking by entry:
    configure VLAN, FDB entries;
    configure policer entry with Remark By Entry cmd;
    set Cos profile entry with RED dp;
    send L2 traffic and verify expected traffic with updated UP;
    check Management Counters - verify updated YELLOW counters;
    set Cos profile entry with GREEN dp;
    send L2 traffic and verify expected traffic with updated UP;
    check Management Counters - verify updated RED counters.
*/
UTF_TEST_CASE_MAC(tgfPolicerIngress0QosRemarkingByEntry)
{
/*
    1. Set VLAN configuration
    2. Generate traffic for INGRESS_0 STAGE
    3. Restore VLAN configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    prvTgfPolicerQosRemarkingVlanConfigurationSet();

    prvTgfPolicerIngressQosRemarkingByEntryTrafficGenerate(
            PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    prvTgfPolicerQosRemarkingConfigurationRestore(PRV_TGF_POLICER_STAGE_INGRESS_0_E);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ingress1 policer QoS remarking by entry:
    configure VLAN, FDB entries;
    configure policer entry with Remark By Entry cmd;
    set Cos profile entry with RED dp;
    send L2 traffic and verify expected traffic with updated UP;
    check Management Counters - verify updated YELLOW counters;
    set Cos profile entry with GREEN dp;
    send L2 traffic and verify expected traffic with updated UP;
    check Management Counters - verify updated RED counters.
*/
UTF_TEST_CASE_MAC(tgfPolicerIngress1QosRemarkingByEntry)
{
/*
    1. Set VLAN configuration
    2. Generate traffic for INGRESS_1 STAGE
    3. Restore VLAN configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        return ;
    }

    prvTgfPolicerQosRemarkingVlanConfigurationSet();

    prvTgfPolicerIngressQosRemarkingByEntryTrafficGenerate(
            PRV_TGF_POLICER_STAGE_INGRESS_1_E);

    prvTgfPolicerQosRemarkingConfigurationRestore(PRV_TGF_POLICER_STAGE_INGRESS_1_E);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ingress0 policer QoS remarking by table:
    configure VLAN, FDB entries;
    configure policer entry with Remark By Table cmd;
    configure Cos profile for GREEN, RED dp;
    set policer relative QoS remarking entry for configured Cos profiles;
    send L2 traffic and verify expected traffic with updated UP;
    check Management Counters - verify updated YELLOW counters.
*/
UTF_TEST_CASE_MAC(tgfPolicerIngress0QosRemarkingByTable)
{
/*
    1. Set VLAN configuration
    2. Generate traffic for INGRESS_0 STAGE
    3. Restore VLAN configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    prvTgfPolicerQosRemarkingVlanConfigurationSet();

    prvTgfPolicerIngressQosRemarkingByTableTrafficGenerate(
            PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    prvTgfPolicerQosRemarkingConfigurationRestore(PRV_TGF_POLICER_STAGE_INGRESS_0_E);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ingress1 policer QoS remarking by table:
    configure VLAN, FDB entries;
    configure policer entry with Remark By Table cmd;
    configure Cos profile for GREEN, RED dp;
    set policer relative QoS remarking entry for configured Cos profiles;
    send L2 traffic and verify expected traffic with updated UP;
    check Management Counters - verify updated YELLOW counters.
*/
UTF_TEST_CASE_MAC(tgfPolicerIngress1QosRemarkingByTable)
{
/*
    1. Set VLAN configuration
    2. Generate traffic for INGRESS_0 STAGE
    3. Restore VLAN configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        return ;
    }

    prvTgfPolicerQosRemarkingVlanConfigurationSet();

    prvTgfPolicerIngressQosRemarkingByTableTrafficGenerate(
            PRV_TGF_POLICER_STAGE_INGRESS_1_E);

    prvTgfPolicerQosRemarkingConfigurationRestore(PRV_TGF_POLICER_STAGE_INGRESS_1_E);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test egress policer QoS remarking:
    configure VLAN, FDB entries;
    configure QoS remarking YELLOW packet to RED;
    send L2 traffic and verify expected traffic with updated UP;
    check Management Counters - verify updated RED counters;
    configure QoS remarking RED packet to GREEN;
    send L2 traffic and verify expected traffic with updated UP;
    check Management Counters - verify updated GREEN counters;
    configure QoS remarking GREEN packet to RED;
    send L2 traffic and verify expected traffic with updated UP;
    check Management Counters - verify updated RED counters.
*/
UTF_TEST_CASE_MAC(tgfPolicerEgressQosRemarking)
{
/*
    1. Set VLAN configuration
    2. Generate traffic for EGRESS STAGE
    3. Restore VLAN configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        return ;
    }

    prvTgfPolicerQosRemarkingVlanConfigurationSet();

    prvTgfPolicerEgressQosRemarkingTrafficGenerate();

    prvTgfPolicerQosRemarkingConfigurationRestore(PRV_TGF_POLICER_STAGE_EGRESS_E);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ingress policer QoS remarking by table for Green Packet:
    configure VLAN, FDB entries;
    configure QoS remarking GREEN packet by QoS profile relative table;
    send L2 traffic and verify expected traffic with updated UP;
    check Management Counters - verify updated GREEN counters;
*/
UTF_TEST_CASE_MAC(tgfPolicerIngressQosRemarkingByTableGreen)
{
/*
    1. Set VLAN configuration
    2. Generate traffic for INGRESS0 STAGE
    3. Restore VLAN configuration
 */
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPolicerQosRemarkingVlanConfigurationSet();

    prvTgfPolicerIngressQosRemarkingByTableGreenTrafficGenerate(PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    prvTgfPolicerQosRemarkingConfigurationRestore(PRV_TGF_POLICER_STAGE_INGRESS_0_E);
}

/*----------------------------------------------------------------------------*/
/*
    3.2.4 tgfPolicerVlanCounting
*/
UTF_TEST_CASE_MAC(tgfPolicerVlanCounting)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT2_E |
        UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* NOTE: bobcat3 (sip5_20) not support vlan counting in the PLR */

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set default Policer VLAN configuration */
    prvTgfDefPolicerVlanConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfDefPolicerVlanStageConfigurationSet(stage);

        prvTgPolicerVlanStagefTrafficGenerate(stage);

        prvTgfPolicerVlanStageConfigurationRestore(stage);
    }

    /* Restore configuration */
    prvTgfPolicerVlanConfigurationRestore();
}



/*----------------------------------------------------------------------------*/
/*
    3.2.5 prvTgfPolicerPortModeAddressSelectFullRedCmdDrop
*/
UTF_TEST_CASE_MAC(prvTgfPolicerPortModeAddressSelectFullRedCmdDrop)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E| UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerPortModeAddressSelectConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerPortModeAddressSelectFullRedCmdDropTrafficGenerate(stage);

        prvTgfPolicerPortModeAddressSelectConfigurationRestore(stage);
    }

    prvTgfPolicerPortModeAddressSelectGlobalConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/*
    3.2.6 prvTgfPolicerPortModeAddressSelectFullRedCmdFrwd
*/
UTF_TEST_CASE_MAC(prvTgfPolicerPortModeAddressSelectFullRedCmdFrwd)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerPortModeAddressSelectConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerPortModeAddressSelectFullRedCmdFrwdTrafficGenerate(stage);

        prvTgfPolicerPortModeAddressSelectConfigurationRestore(stage);
    }

    prvTgfPolicerPortModeAddressSelectGlobalConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/*
    3.2.7 prvTgfPolicerPortModeAddressSelectCompresYellowCmdDrop
*/
UTF_TEST_CASE_MAC(prvTgfPolicerPortModeAddressSelectCompresYellowCmdDrop)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerPortModeAddressSelectConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_INGRESS_1_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerPortModeAddressSelectCompresYellowCmdDropTrafficGenerate(stage);

        prvTgfPolicerPortModeAddressSelectConfigurationRestore(stage);
    }

    prvTgfPolicerPortModeAddressSelectGlobalConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    3.2.8 prvTgfPolicerPortModeAddressSelectCompresYellowCmdFrwd
*/
UTF_TEST_CASE_MAC(prvTgfPolicerPortModeAddressSelectCompresYellowCmdFrwd)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerPortModeAddressSelectConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_INGRESS_1_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerPortModeAddressSelectCompresYellowCmdFrwdTrafficGenerate(stage);

        prvTgfPolicerPortModeAddressSelectConfigurationRestore(stage);
    }

    prvTgfPolicerPortModeAddressSelectGlobalConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/*
    3.2.9 prvTgfPolicerPortModeAddressSelectCompresYellowCmdFrwd
*/
UTF_TEST_CASE_MAC(prvTgfPolicerPortModeAddressSelectFullGreen)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerPortModeAddressSelectConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerPortModeAddressSelectFullGreenTrafficGenerate(stage);

        prvTgfPolicerPortModeAddressSelectConfigurationRestore(stage);
    }

    prvTgfPolicerPortModeAddressSelectGlobalConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    3.2.9 prvTgfPolicerPortModeAddressSelectCompresGreen
*/
UTF_TEST_CASE_MAC(prvTgfPolicerPortModeAddressSelectCompresGreen)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerPortModeAddressSelectConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_EGRESS_E;
          stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerPortModeAddressSelectCompresGreenTrafficGenerate(stage);

        prvTgfPolicerPortModeAddressSelectConfigurationRestore(stage);
    }

    prvTgfPolicerPortModeAddressSelectGlobalConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/*
    3.2.9 prvTgfPolicerEportEvlanFlowModeEvlanDisabled
*/
UTF_TEST_CASE_MAC(prvTgfPolicerEportEvlanFlowModeEvlanDisabled)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerEportEvlanConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_INGRESS_1_E/*PRV_TGF_POLICER_STAGE_EGRESS_E*/; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerEportEvlanFlowModeEvlanDisabledTrafficGenerate(stage);

        prvTgfPolicerEportEvlanConfigurationRestore(stage);
    }

    prvTgfPolicerEportEvlanGlobalConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    3.2.9 prvTgfPolicerEportEvlanFlowModeEvlanEnabled
*/
UTF_TEST_CASE_MAC(prvTgfPolicerEportEvlanFlowModeEvlanEnabled)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerEportEvlanConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_INGRESS_0_E/*PRV_TGF_POLICER_STAGE_EGRESS_E*/; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerEportEvlanFlowModeEvlanEnabledTrafficGenerate(stage);

        prvTgfPolicerEportEvlanConfigurationRestore(stage);
    }

    prvTgfPolicerEportEvlanGlobalConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    3.2.10 prvTgfPolicerEportEvlanFlowModeTtiActionAndEvlanEnabled
*/
UTF_TEST_CASE_MAC(prvTgfPolicerEportEvlanFlowModeTtiActionAndEvlanEnabled)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerEportEvlanConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_INGRESS_1_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerEportEvlanFlowModeTtiActionAndEvlanEnabledTrafficGenerate(stage);

        prvTgfPolicerEportEvlanConfigurationRestore(stage);
    }

    prvTgfPolicerEportEvlanGlobalConfigurationRestore();

}


/*----------------------------------------------------------------------------*/
/*
    3.2.11 prvTgfPolicerEportEvlanPortModeEportEnabled
*/
UTF_TEST_CASE_MAC(prvTgfPolicerEportEvlanPortModeEportEnabled)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerEportEvlanConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerEportEvlanPortModeEportEnabledTrafficGenerate(stage);

        prvTgfPolicerEportEvlanConfigurationRestore(stage);
    }

    prvTgfPolicerEportEvlanGlobalConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/*
    3.2.12 prvTgfPolicerEportEvlanPortModeEportDisable
*/
UTF_TEST_CASE_MAC(prvTgfPolicerEportEvlanPortModeEportDisable)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerEportEvlanConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerEportEvlanPortModeEportDisabledTrafficGenerate(stage);

        prvTgfPolicerEportEvlanConfigurationRestore(stage);
    }

    prvTgfPolicerEportEvlanGlobalConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    3.2.13 prvTgfPolicerEportEvlanFlowModeEvlanAllPktTypes
*/
UTF_TEST_CASE_MAC(prvTgfPolicerEportEvlanFlowModeEvlanAllPktTypes)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT      stage;
    GT_U32                              i;
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerEportEvlanConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        for (i = PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E;
              i <= PRV_TGF_POLICER_STORM_TYPE_BC_E/*PRV_TGF_POLICER_STORM_TYPE_TCP_SYN_E*/; i++)
        {
            prvTgfPolicerEportEvlanFlowModeEvlanAllPktTypesTrafficGenerate(stage, i);

            prvTgfPolicerEportEvlanConfigurationRestore(stage);

        }
    }

    prvTgfPolicerEportEvlanGlobalConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - New (earch) meter entry related fields:
    This test verify the usage of several new metering entry fields due to the
    eArch format update. The features examined in this test are the bytes or
    packet metering mode, bytes metering counting mode (L1/L2/L3) and green
    packet command for ingress stages.
*/
UTF_TEST_CASE_MAC(prvTgfPolicerBasicEarchMeterEntry)
{
    PRV_TGF_POLICER_STAGE_TYPE_ENT      stage;
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPolicerEarchMeterEntryGlobalConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        if(GT_FALSE == prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerBasicEarchMeterEntryTest(stage);
    }

    prvTgfPolicerEarchMeterEntryGlobalConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Egress QoS UP updates:
    The test modify UP value for outer tag1 and inner tago due to
    EGRESS policer.
*/
UTF_TEST_CASE_MAC(prvTgfPolicerEgressUpModify)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        return ;
    }

    prvTgfPolicerEgressUpModifyTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Explicit Congestion Notification (ECN) marking for IPv4/IPv6 packets:
*/
UTF_TEST_CASE_MAC(prvTgfPolicerECNMarkingCheck)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        return ;
    }

    /* AUTODOC: Test configurations */
    prvTgfPolicerECNMarkingConfigurationSet();

    /* AUTODOC: Test traffic and checks */
    prvTgfPolicerECNMarkingGenerateTraffic();

    /* AUTODOC: Restore configurations */
    prvTgfPolicerECNMarkingRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: DSA Tag Counting Mode test.
*/
UTF_TEST_CASE_MAC(prvTgfPolicerDsaTagCountingMode)
{
/*
    1. Set  configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   ING_PORT_IDX_CNS,
                                   EGR_PORT_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    prvTgfPolicerDsaTagCountingModeGlobalConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        if(GT_FALSE == prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerDsaTagCountingModeTest(stage);
    }

    prvTgfPolicerDsaTagCountingModeGlobalConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Egress QoS DSCP updates:
    The test modify DSCP value due to EGRESS policer.
*/
UTF_TEST_CASE_MAC(prvTgfPolicerEgressDscpModify)
{
    GT_U32 notAppFamilyBmp;
    GT_U32      prvUtfVrfId;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        return ;
    }

    /* NOTE: Test is wrong for PBR mode!!!
             can't do tunnel start and routing though PCL in PBR mode,
             in "IPCL Action as Next Hop Entry" there is no support for tunnel start */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    prvTgfPolicerEgressDscpModifyTest(prvUtfVrfId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Egress QoS EXP updates:
    The test modify EXP value due to EGRESS policer.
*/
UTF_TEST_CASE_MAC(prvTgfPolicerEgressExpModify)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        return ;
    }

    prvTgfPolicerEgressExpModifyTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - checks TTI action configurations related to billing counters.

*/
UTF_TEST_CASE_MAC(prvTgfPolicerEportEvlanFlowModeTtiActionBilling)
{
/*
    1. Set Vlan configuration
    2. Set Policer Vlan configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Policer Port Mode Address Select Configuration */
    prvTgfPolicerEportEvlanConfigurationSet();

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
          stage <= PRV_TGF_POLICER_STAGE_INGRESS_1_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        prvTgfPolicerEportEvlanFlowModeTtiActionBillingCountersTrafficGenerate(stage);

        prvTgfPolicerEportEvlanConfigurationRestore(stage);
    }

    prvTgfPolicerEportEvlanGlobalConfigurationRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - check rate on egress port according to configerd CIR
*/
UTF_TEST_CASE_MAC(prvTgfPolicerWSRateRatioRateType)
{
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT3_E);

    if(GT_FALSE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        return ;
    }

    prvTgfPolicerWSRateRatioRateTypeTest();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Test prvTgfPolicerMirroring
    configure VLAN, FDB entries;
    configure Policer, configure mirroring ;
    send traffic ;
    verify traffic is sent correctly from analyzer.
*/

UTF_TEST_CASE_MAC(prvTgfPolicerMirroring)
{
    /*
        1. Set configuration
        2. Set Policer configuration
        3. Generate traffic
        4. Restore configuration
    */

    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))    

    /* Per stage configurations and traffic */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
        stage <= PRV_TGF_POLICER_STAGE_EGRESS_E; stage++)
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(stage))
        {
            continue ;
        }

        /* Set Policer, FDB and mirroring configuration */
        prvTgfPolicerMirroringCfgSet(stage);

        /* Generate traffic */
        prvTgfPolicerMirroringTrafficGenerate(stage);

        /* Restore configuration */
        prvTgfPolicerMirroringCfgRestore(stage);
    }        
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of tgfPolicer suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfPolicer)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPolicerIngress0QosRemarkingByEntry)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPolicerIngress1QosRemarkingByEntry)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPolicerIngress0QosRemarkingByTable)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPolicerIngress1QosRemarkingByTable)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPolicerEgressQosRemarking)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPolicerIngressQosRemarkingByTableGreen)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPolicerVlanCounting)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerPortModeAddressSelectFullRedCmdDrop)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerPortModeAddressSelectFullRedCmdFrwd)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerPortModeAddressSelectCompresYellowCmdDrop)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerPortModeAddressSelectCompresYellowCmdFrwd)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerPortModeAddressSelectFullGreen)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerPortModeAddressSelectCompresGreen)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerEportEvlanFlowModeEvlanDisabled)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerEportEvlanFlowModeEvlanEnabled)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerEportEvlanFlowModeTtiActionAndEvlanEnabled)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerEportEvlanFlowModeTtiActionBilling)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerEportEvlanPortModeEportEnabled)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerEportEvlanPortModeEportDisable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerEportEvlanFlowModeEvlanAllPktTypes)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerBasicEarchMeterEntry)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerEgressUpModify)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerECNMarkingCheck)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerDsaTagCountingMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerEgressDscpModify)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerEgressExpModify)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerWSRateRatioRateType)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPolicerMirroring)
UTF_SUIT_END_TESTS_MAC(tgfPolicer)


