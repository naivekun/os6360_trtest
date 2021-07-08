/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfCommonMirrorUT.c
*
* DESCRIPTION:
*       Enhanced UTs for CPSS Cut-Through
*
* FILE REVISION NUMBER:
*       $Revision: 8 $
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <cutThrough/prvTgfCutThrough.h>
#include <common/tgfCutThrough.h>

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test enable\disable bypassing Router and Ingress Policer engines:
    configure VLAN, FDB entries;
    configure CutThrough;
    configure UC Routing entries;
    add IPv4 UC prefix to LPM DB with exact packet's DIP match;
    send IPv4 UC traffic and verify expected routed traffic on nextHop port;
    enable bypassing Router and Ingress Policer engines;
    send IPv4 UC traffic and verify expected traffic on FDB port.
*/
UTF_TEST_CASE_MAC(prvTgfCutThroughBypassRouterAndPolicer)
{
/********************************************************************
    Test 1.1 - Enabling / disabling bypassing the Router and Ingress Policer engines.

    1. Set base configuration
    2. Set Routing configuration
    3. Generate traffic
    4. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set base configuration */
    prvTgfCutThroughBypassRouterAndPolicerBaseConfigurationSet();

    /* Set Routing configuration */
    prvTgfCutThroughBypassRouterAndPolicerRouteConfigurationSet();

    /* Generate traffic */
    prvTgfCutThroughBypassRouterAndPolicerTrafficGenerate();

    /* Restore configuration */
    prvTgfCutThroughBypassRouterAndPolicerConfigurationRestore();
}

UTF_TEST_CASE_MAC(prvTgfCutThroughBypassPcl)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |  UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);
    /* Set configuration */
    prvTgfCutThroughBypassPclConfig();

    /* Generate traffic */
    prvTgfCutThroughBypassPclTrafficGenerate();

    /* Restore configuration */
    prvTgfCutThroughBypassPclRestore();
}

UTF_TEST_CASE_MAC(prvTgfCutThroughBypassRouter)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |  UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);
    if (prvTgfCutThroughGlobalEnableGet() == GT_FALSE)
    {
        SKIP_TEST_MAC
    }

    UTF_SIP6_GM_NOT_READY_SKIP_MAC("SIP6 Cut Through yet not aligned to GM");

    /* Set configuration */
    prvTgfCutThroughBypassRouterConfig();

    /* Generate traffic */
    prvTgfCutThroughBypassRouterTrafficGenerate();

    /* Restore configuration */
    prvTgfCutThroughBypassRouterRestore();
}

UTF_TEST_CASE_MAC(prvTgfCutThroughBypassMll)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |  UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);
    /* Set configuration */
    prvTgfCutThroughBypassMllConfig();

    /* Generate traffic */
    prvTgfCutThroughBypassMllTrafficGenerate();

    /* Restore configuration */
    prvTgfCutThroughBypassMllRestore();
}

UTF_TEST_CASE_MAC(prvTgfCutThroughBypassPolicer)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |  UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);
    /* Set configuration */
    prvTgfCutThroughBypassPolicerConfig();

    /* Generate traffic */
    prvTgfCutThroughBypassPolicerTrafficGenerate();

    /* Restore configuration */
    prvTgfCutThroughBypassPolicerRestore();
}

UTF_TEST_CASE_MAC(prvTgfCutThroughBypassOam)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |  UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Set configuration */
    prvTgfCutThroughBypassOamConfig();

    /* Generate traffic */
    prvTgfCutThroughBypassOamTrafficGenerate();

    /* Restore configuration */
    prvTgfCutThroughBypassOamRestore();
}

UTF_TEST_CASE_MAC(prvTgfCutThroughCheckByteCount)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    UTF_SIP6_GM_NOT_READY_SKIP_MAC("SIP6 Cut Through yet not aligned to GM");

    prvTgfCutThroughByteCountTest();
}

UTF_TEST_CASE_MAC(prvTgfCutThroughCheckByteCountIpv4OverMpls)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    UTF_SIP6_GM_NOT_READY_SKIP_MAC("SIP6 Cut Through yet not aligned to GM");

    prvTgfCutThroughByteCountIpv4OverMplsTest(GT_FALSE /*transitOrTunnelTerm*/, GT_TRUE  /*passengerWithL2*/);
    prvTgfCutThroughByteCountIpv4OverMplsTest(GT_TRUE  /*transitOrTunnelTerm*/, GT_FALSE /*passengerWithL2*/);
    prvTgfCutThroughByteCountIpv4OverMplsTest(GT_TRUE  /*transitOrTunnelTerm*/, GT_TRUE  /*passengerWithL2*/);
}

UTF_TEST_CASE_MAC(prvTgfCutThroughByteCountUdeCtTermination_defByteCount)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    UTF_SIP6_GM_NOT_READY_SKIP_MAC("SIP6 Cut Through yet not aligned to GM");

    prvTgfCutThroughByteCountUdeCtTerminationTest_defByteCount();
    /* return; test yet not debugged */
}

UTF_TEST_CASE_MAC(prvTgfCutThroughIpv6CtTermination_EPCL)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    UTF_SIP6_GM_NOT_READY_SKIP_MAC("SIP6 Cut Through yet not aligned to GM");

    prvTgfCutThroughIpv6CtTerminationTest_EPCL();
}

UTF_TEST_CASE_MAC(prvTgfCutThroughIpv4CtTermination_SlowToFast)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    UTF_SIP6_GM_NOT_READY_SKIP_MAC("SIP6 Cut Through yet not aligned to GM");

    /* this test yet cannot be executed                  */
    /* Only 50G speed supported                          */
    /* whed 10G or 25G will be supported be continued    */
    SKIP_TEST_MAC

    /*prvTgfCutThroughIpv4CtTerminationTest_SlowToFast();*/
}

/*
 * Configuration of tgfCutThrough suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfCutThrough)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCutThroughBypassRouterAndPolicer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCutThroughBypassPcl)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCutThroughBypassRouter)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCutThroughBypassMll)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCutThroughBypassPolicer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCutThroughBypassOam)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCutThroughCheckByteCount)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCutThroughCheckByteCountIpv4OverMpls)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCutThroughByteCountUdeCtTermination_defByteCount)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCutThroughIpv6CtTermination_EPCL)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCutThroughIpv4CtTermination_SlowToFast)
UTF_SUIT_END_TESTS_MAC(tgfCutThrough)
