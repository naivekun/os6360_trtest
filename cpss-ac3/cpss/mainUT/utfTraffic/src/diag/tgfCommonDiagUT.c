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
* @file tgfCommonDiagUT.c
*
* @brief Enhanced UTs for CPSS Diag
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <diag/prvTgfDiagDescriptor.h>
#include <diag/prvTgfDiagDataIntegrityErrorInjection.h>
#include <diag/prvTgfDiagMppmEccProtection.h>

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Diag descriptor dump for Bridge:
    send traffic with different MAC SA on each port;
    verify that got traffic on each port;
    verify dynamically created FDB entries;
    send traffic with learned MACs;
    verify traffic forwarded according to dynamically created FDB entries;
    get HW\SW descriptor information and compare with descriptor dump results.
*/
UTF_TEST_CASE_MAC(prvTgfDiagDescriptorBridge)
{
/********************************************************************
    Test 1 - Descriptor Dump - Bridge

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, "JIRA:6788");

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                        UTF_CH2_E | UTF_CH3_E | UTF_PUMA_E |
                        /* need to use the iDebug for this feature */
                        UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* Set configuration */
    prvTgfDiagDescriptorBridgeConfigurationSet();

    /* Generate traffic */
    prvTgfDiagDescriptorBridgeTrafficGenerate();

    /* Restore configuration */
    prvTgfDiagDescriptorBridgeRestore();
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Diag descriptor dump for Policer:
    configure VLAN, FDB entries with different cmd;
    configure Vlan counting mode default values;
    send traffic with different DAs;
    verify traffic and verify Policer Vlan counters;
    get HW\SW descriptor information and compare with descriptor dump results.
*/
UTF_TEST_CASE_MAC(prvTgfDiagDescriptorPolicer)
{
/********************************************************************
    Test 2 - Descriptor Dump - Policer

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, "JIRA:6790");

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                        UTF_CH2_E | UTF_CH3_E | UTF_PUMA_E |
                        /* need to use the iDebug for this feature */
                        UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Set configuration */
    prvTgfDiagDescriptorPolicerConfigurationSet();

    /* Generate traffic */
    prvTgfDiagDescriptorPolicerTrafficGenerate();

    /* Restore configuration */
    prvTgfDiagDescriptorPolicerRestore();
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Diag descriptor dump for Tunnel:
    configure VLAN, FDB entries;
    configure TTI rule with Ether key;
    send tunneled traffic to match TTI rule;
    verify to get no traffic -- dropped;
        get HW\SW descriptor information and compare with descriptor dump results.
*/
UTF_TEST_CASE_MAC(prvTgfDiagDescriptorTunnelTerm)
{
/********************************************************************
    Test 2 - Descriptor Dump - TunnelTerm

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, "JIRA:6791");

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                         UTF_CH2_E | UTF_CH3_E | UTF_PUMA_E |
                        /* need to use the iDebug for this feature */
                        UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Set configuration */
    prvTgfDiagDescriptorTunnelTermConfigurationSet();

    /* Generate traffic */
    prvTgfDiagDescriptorTunnelTermTrafficGenerate();

    /* Restore configuration */
    prvTgfDiagDescriptorTunnelTermRestore();
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Diag descriptor dump for PCL:
    configure VLAN, FDB entries;
    configure 2 PCL rules - to trap and redirect not IPv4\IPv6 traffic;
    send traffic with updated rules;
    verify traffic is trapped\forwarded according to updated rule actions;
    get HW\SW descriptor information and compare with descriptor dump results.
*/
UTF_TEST_CASE_MAC(prvTgfDiagDescriptorPcl)
{
/********************************************************************
    Test 2 - Descriptor Dump - PCL

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E, "JIRA:6789");

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                       UTF_CH2_E | UTF_CH3_E | UTF_PUMA_E |
                        /* need to use the iDebug for this feature */
                        UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Set configuration */
    prvTgfDiagDescriptorPclConfigurationSet();

    /* Generate traffic */
    prvTgfDiagDescriptorPclTrafficGenerate();

    /* Restore configuration */
    prvTgfDiagDescriptorPclRestore();
#endif /* ASIC_SIMULATION */
}

/* AUTODOC: Test Data Integrity Events for different RAMs:
    configure VLAN with all ports are members;
    enable RAM error injection;
    unmask RAM error interrupts;
    send flooding traffic;
    verify traffic is forwarded and events with proper information;
*/
UTF_TEST_CASE_MAC(prvTgfDiagDataIntegrityErrorInjection)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, ~(UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E |
                        UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E));

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfDiagDataIntegrityErrorInjectionConfigSet();
    prvTgfDiagDataIntegrityErrorInjectionTrafficGenerate();
    prvTgfDiagDataIntegrityErrorInjectionRestore();
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test for buffer memory (MPPM) ECC protection check
    - configure UC bridging;
    - enable jumbo frames;
    - unmask data integrity interrupt;
    - send packets
    -     enable error injection for MPPM
    -     send packet
    -     check error event
    - restore
*/
UTF_TEST_CASE_MAC(prvTgfDiagMppmEccProtection)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, ~(UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E |
                        UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E));

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#ifndef ASIC_SIMULATION
    prvTgfDiagMppmEccProtectionConfigurationSet();
    prvTgfDiagMppmEccProtectionTrafficGenerate();
    prvTgfDiagMppmEccProtectionRestore();
#endif /* ASIC_SIMULATION */
}

/*
 * Configuration of tgfDiag suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfDiag)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfDiagDescriptorBridge)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfDiagDescriptorPolicer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfDiagDescriptorTunnelTerm)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfDiagDescriptorPcl)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfDiagDataIntegrityErrorInjection)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfDiagMppmEccProtection)
UTF_SUIT_END_TESTS_MAC(tgfDiag)
