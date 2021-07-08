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
* @file tgfCommonMplsUT.c
*
* @brief Enhanced UTs for CPSS MPLS
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <common/tgfCommon.h>
#include <common/tgfIpGen.h>
#include <mpls/prvTgfMpls.h>

/*----------------------------------------------------------------------------*/
/* AUTODOC: Basic MPLS traffic:
    Send MPLS packet with Label 16;
    expect it to be routed to port 3, vlan tag is added,
    the rest of the packet is unchanged.
*/
UTF_TEST_CASE_MAC(tgfMplsBasic)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E| UTF_LION_E| UTF_XCAT2_E| UTF_LION2_E| UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfMplsBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfMplsRouteConfigurationSet();

    /* Set Mpls configuration */
    prvTgfMplsConfigurationSet();

    /* Set Mpls basic NHLFE configuration */
    prvTgfMplsNhlfeBasicConfigurationSet();

    /* Generate traffic */
    prvTgfMplsBasicTrafficGenerate();

    /* Restore configuration */
    prvTgfMplsConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: SWAP MPLS label:
    Send MPLS packet with Label 16;
    verify tagged traffic to be routed to port 3 with
    updateded DA(according to arpPtr), SA (according to vlan tag), MPLS label.
*/
UTF_TEST_CASE_MAC(tgfMplsSwapLabel)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E| UTF_LION_E| UTF_XCAT2_E| UTF_LION2_E| UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfMplsBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfMplsRouteConfigurationSet();

    /* Set Mpls configuration */
    prvTgfMplsConfigurationSet();

    /* Set Mpls basic NHLFE configuration */
    prvTgfMplsNhlfeBasicConfigurationSet();

    /* Set Mpls NHLFE SWAP configuration */
    prvTgfMplsNhlfeSwapConfigurationSet();

    /* Generate traffic */
    prvTgfMplsSwapTrafficGenerate();

    /* Restore configuration */
    prvTgfMplsConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: PUSH MPLS label:
    Send MPLS packet with Label 17;
    verify tagged traffic to be routed to port 3 with
    updateded DA(according to arpPtr), SA (according to vlan tag), MPLS label.
    MPLS label is changed to be 17 then 21.
*/
UTF_TEST_CASE_MAC(tgfMplsPushLabel)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E| UTF_LION_E| UTF_XCAT2_E| UTF_LION2_E| UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfMplsBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfMplsRouteConfigurationSet();

    /* Set Mpls configuration */
    prvTgfMplsConfigurationSet();

    /* Set Mpls basic NHLFE configuration */
    prvTgfMplsNhlfeBasicConfigurationSet();

    /* Set Mpls NHLFE PUSH configuration */
    prvTgfMplsNhlfePushConfigurationSet();

    /* Generate traffic */
    prvTgfMplsPushTrafficGenerate();

    /* Restore configuration */
    prvTgfMplsConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: PHP (Penultimate Hop Pop) MPLS label:
    Send MPLS packet with Label 16 then label 10;
    forwarding decision is based on popped label - label 16;
    verify tagged traffic to be routed to port 3 with
    updateded DA(according to arpPtr), SA (according to vlan tag), MPLS label;
    MPLS label is now 10 (second) ttl=0xA(copied from the entry).
*/
UTF_TEST_CASE_MAC(tgfMplsPhpLabel)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E| UTF_LION_E| UTF_XCAT2_E| UTF_LION2_E| UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfMplsBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfMplsRouteConfigurationSet();

    /* Set Mpls configuration */
    prvTgfMplsConfigurationSet();

    /* Set Mpls basic NHLFE configuration */
    prvTgfMplsNhlfeBasicConfigurationSet();

    /* Set Mpls NHLFE PHP configuration */
    prvTgfMplsNhlfePhpConfigurationSet();

    /* Generate traffic */
    prvTgfMplsPhpTrafficGenerate();

    /* Restore configuration */
    prvTgfMplsConfigurationRestore();
}

/*
 * Configuration of tgfMpls suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfMpls)
    UTF_SUIT_DECLARE_TEST_MAC(tgfMplsBasic)
    UTF_SUIT_DECLARE_TEST_MAC(tgfMplsSwapLabel)
    UTF_SUIT_DECLARE_TEST_MAC(tgfMplsPushLabel)
    UTF_SUIT_DECLARE_TEST_MAC(tgfMplsPhpLabel)

UTF_SUIT_END_TESTS_MAC(tgfMpls)


