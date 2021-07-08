/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfCommonBridgeUT.c
*
* DESCRIPTION:
*       Enhanced UTs for CPSS Bridge
*
* FILE REVISION NUMBER:
*       $Revision: 8 $
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <nst/prvTgfNstPortIsolation.h>

/* skip test for device with tested port numbers >= 256 .
   because port isolation not supports such ports.
*/
#define PORT_ISOLATION_SKIP_TESTED_PORTS_HIGHER_THAN_256_MAC    \
    if(prvTgfPortsArray[0] >= 256)                              \
        SKIP_TEST_MAC



/*----------------------------------------------------------------------------*/
/* AUTODOC: Test 6.1 - Port Isolation for Layer2 Packets:
    configure VLAN, FDB entries;
    configure Port Isolation entry for L2;
    send traffic and verify that traffic doesn't egress from the isolated ports;
    disable Port Isolation feature;
    send traffic and verify that traffic egress from all ports.
*/
UTF_TEST_CASE_MAC(tgfNstL2PortIsolation)
{
/********************************************************************
    Test 6.1 - Port Isolation for Layer2 Packets

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E );
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PORT_ISOLATION_SKIP_TESTED_PORTS_HIGHER_THAN_256_MAC;
    /* Set configuration */
    prvTgfNstL2PortIsolationConfigurationSet();

    /* Generate traffic */
    prvTgfNstL2PortIsolationTrafficGenerate();

    /* Restore configuration */
    prvTgfNstL2PortIsolationConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test 6.2 - Port Isolation for Layer3 Packets:
    configure VLAN, FDB entries;
    configure Port Isolation entry for L3;
    configure IPv4 UC Routing;
    send L3 traffic and verify that traffic doesn't egress from the isolated ports;
    disable Port Isolation feature;
    send traffic and verify that traffic egress from all ports.
*/
UTF_TEST_CASE_MAC(tgfNstL3PortIsolation)
{
/********************************************************************
    Test 6.2 - Port Isolation for Layer3 Packets

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PORT_ISOLATION_SKIP_TESTED_PORTS_HIGHER_THAN_256_MAC;

    /* Set configuration */
    prvTgfNstL3PortIsolationConfigurationSet();

    /* Generate traffic */
    prvTgfNstL3PortIsolationTrafficGenerate();

    /* Restore configuration */
    prvTgfNstL3PortIsolationConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test 6.3 - Port Isolation for Layer3 and Layer3 Packets
    configure VLAN, FDB entries;
    configure Port Isolation entry for L2_L3 traffic;
    configure IPv4 UC Routing;
    send L2 and L3 traffic and verify that traffic doesn't egress from isolated ports;
    disable Port Isolation feature in VLAN;
    send L2 and L3 traffic and verify that traffic egress from all ports.
*/
UTF_TEST_CASE_MAC(tgfNstL2L3PortIsolation)
{
/********************************************************************
    Test 6.3 - Port Isolation for Layer3 and Layer3 Packets

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PORT_ISOLATION_SKIP_TESTED_PORTS_HIGHER_THAN_256_MAC;

    /* Set configuration */
    prvTgfNstL2L3PortIsolationConfigurationSet();

    /* Generate traffic */
    prvTgfNstL2L3PortIsolationTrafficGenerate();

    /* Restore configuration */
    prvTgfNstL2L3PortIsolationConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test 6.4 Port Isolation for Layer2 Packets with Trunk members
    configure VLAN, FDB, TRUNK entries;
    configure Port Isolation entry for L2 with Trunk interface;
    send traffic to port trunk member;
    verify that traffic doesn't egress from the isolated ports;
    disable Port Isolation feature;
    send traffic to port trunk member;
    send traffic and verify that traffic egress from all ports.
*/
UTF_TEST_CASE_MAC(tgfNstL2TrunkPortIsolation)
{
/********************************************************************
    Test 6.4 Port Isolation for Layer2 Packets - Port 8 is trunk member

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E );
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PORT_ISOLATION_SKIP_TESTED_PORTS_HIGHER_THAN_256_MAC;

    /* Set configuration */
    prvTgfNstL2TrunkPortIsolationConfigurationSet();

    /* Generate traffic */
    prvTgfNstL2TrunkPortIsolationTrafficGenerate();

    /* Restore configuration */
    prvTgfNstL2TrunkPortIsolationConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test 6.5 Port Isolation for Layer3 Packets with Trunk members
    configure VLAN, FDB, TRUNK entries;
    configure Port Isolation entry for L3 with Trunk interface;
    configure IPv4 UC Routing;
    send traffic to port trunk member;
    verify that traffic doesn't egress from the isolated ports;
    disable Port Isolation feature;
    send traffic to port trunk member;
    send traffic and verify that traffic egress from all ports.
*/
UTF_TEST_CASE_MAC(tgfNstL3TrunkPortIsolation)
{
/********************************************************************
    Test 6.5 Port Isolation for Layer3 Packets - Port 8 is trunk member

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    PORT_ISOLATION_SKIP_TESTED_PORTS_HIGHER_THAN_256_MAC;

    /* Set configuration */
    prvTgfNstL3TrunkPortIsolationConfigurationSet();

    /* Generate traffic */
    prvTgfNstL3TrunkPortIsolationTrafficGenerate();

    /* Restore configuration */
    prvTgfNstL3TrunkPortIsolationConfigurationRestore();
}

/*
 * Configuration of tgfNst suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfNst)
    UTF_SUIT_DECLARE_TEST_MAC(tgfNstL2PortIsolation)
    UTF_SUIT_DECLARE_TEST_MAC(tgfNstL3PortIsolation)
    UTF_SUIT_DECLARE_TEST_MAC(tgfNstL2L3PortIsolation)
    UTF_SUIT_DECLARE_TEST_MAC(tgfNstL2TrunkPortIsolation)
    UTF_SUIT_DECLARE_TEST_MAC(tgfNstL3TrunkPortIsolation)
UTF_SUIT_END_TESTS_MAC(tgfNst)

