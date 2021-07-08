/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfCommonDitUT.c
*
* DESCRIPTION:
*       Enhanced UTs for CPSS FCOE
*
* FILE REVISION NUMBER:
*       $Revision: 6 $
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <fcoe/prvTgfFcoe.h>
#include <common/tgfDitGen.h>

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - FCOE using PCL user defined bytes:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid offsets;
    send traffic to match PCL rules and UDBs;
    verify traffic is not dropped according to rule actions;
    and the MAC_DA and MAC_SA is changed according to DIT configuration.
*/

UTF_TEST_CASE_MAC(tgfFcoeBaseForwarding)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* Set Base configuration */
    prvTgfFcoeBaseConfigurationSet();

    /* Check IPCL based FCOE forwarding */
    prvTgfFcoePclConfigurationSet();
    prvTgfTtiFcoeForwardingConfigurationSet();
    prvTgfFcoeTrafficGenerate();
    prvTgfTtiFcoeForwardingConfigurationRestore();
    prvTgfFcoePclConfigurationRestore();

    /* Restore Base configuration */
    prvTgfFcoeBaseConfigurationRestore();
}

UTF_TEST_CASE_MAC(tgfFcoeBasicLpmForwarding)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_FALCON_E , "JIRA:7430");

    /* Set Base configuration */
    prvTgfFcoeBaseConfigurationSet();

    prvTgfTtiFcoeForwardingConfigurationSet();
    prvTgfBasicFcoeLpmForwardingConfigurationSet();
    prvTgfFcoeTrafficGenerate();
    prvTgfBasicFcoeLpmForwardingConfigurationRestore();
    prvTgfTtiFcoeForwardingConfigurationRestore();

    /* Restore Base configuration */
    prvTgfFcoeBaseConfigurationRestore();
}

UTF_TEST_CASE_MAC(tgfFcoeVRForwarding)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* Set Base configuration */
    prvTgfFcoeBaseConfigurationSet();

    /* Check VF-ID=> VRF-ID FCOE forwarding */
    prvTgfTtiFcoeForwardingConfigurationSet();
    prvTgfIpPortFcoeForwardingConfigurationSet();
    prvTgfVlanFcoeForwardingConfigurationSet();
    prvTgfFcoeTrafficGenerate();
    prvTgfVlanFcoeForwardingConfigurationRestore();
    prvTgfIpPortFcoeForwardingConfigurationRestore();
    prvTgfTtiFcoeForwardingConfigurationRestore();

    /* Restore Base configuration */
    prvTgfFcoeBaseConfigurationRestore();
}

/*
 * Configuration of tgfFcoe suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfFcoe)
    UTF_SUIT_DECLARE_TEST_MAC(tgfFcoeBaseForwarding)
    UTF_SUIT_DECLARE_TEST_MAC(tgfFcoeBasicLpmForwarding)
    UTF_SUIT_DECLARE_TEST_MAC(tgfFcoeVRForwarding)
UTF_SUIT_END_TESTS_MAC(tgfFcoe)
