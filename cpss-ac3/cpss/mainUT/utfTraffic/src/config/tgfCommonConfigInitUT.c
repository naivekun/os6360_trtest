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
* @file tgfCommonConfigInitUT.c
*
* @brief Enhanced UTs for Config Init.
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <config/prvTgfConfigInit.h>

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ingress drop counters:
    configure VLAN, FDB entries;
    set different ingress drop counter modes;
    send Ethernet traffic for each mode;
    verify drop counters are correct.
*/
UTF_TEST_CASE_MAC(prvTgfCfgIngressDropCounter)
{
/********************************************************************
    Test 1 - Ingress drop counter.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfCfgIngressDropCounterSet();

    /* Generate traffic */
    prvTgfCfgIngressDropCounterTrafficGenerate();

    /* Restore configuration */
    prvTgfCfgIngressDropCounterRestore();
}

UTF_TEST_CASE_MAC(prvTgfCfgProbePacketDropCode)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    prvTgfCfgProbePacketBasicConfigSet();
    prvTgfCfgProbePacketPclConfigSet();
    prvTgfCfgProbePacketTrafficGenerate();
    prvTgfCfgProbePacketSourceIdUpdate();
    prvTgfCfgProbePacketTrafficGenerate();
    prvTgfCfgProbePacketConfigRestore();
}

UTF_TEST_CASE_MAC(prvTgfCfgProbePacketEgressDropCodeTest)
{

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    prvTgfCfgProbePacketBasicConfigSet();
    prvTgfCfgProbePacketEgressPclConfigSet();
    prvTgfCfgProbePacketTrafficGenerate();
    prvTgfCfgProbePacketConfigRestore();
}

/*
 * Configuration of tgfConfig suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfConfig)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgIngressDropCounter)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgProbePacketDropCode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgProbePacketEgressDropCodeTest)
UTF_SUIT_END_TESTS_MAC(tgfConfig)


