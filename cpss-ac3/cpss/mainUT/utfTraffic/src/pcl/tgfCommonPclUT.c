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
* @file tgfCommonPclUT.c
*
* @brief Enhanced UTs for CPSS PCL
*
* @version   108
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <pcl/prvTgfIngressPclKeyStdL2.h>
#include <pcl/prvTgfPclValidInvalidRulesBasicExtended.h>
#include <pcl/prvTgfPclValidInvalidRulesActionUpdate.h>
#include <pcl/prvTgfPclSecondLookupMatch.h>
#include <pcl/prvTgfPclBothUserDefinedBytes.h>
#include <pcl/prvTgfPclUdb.h>
#include <pcl/prvTgfPclUdbFlds.h>
#include <pcl/prvTgfPclBypass.h>
#include <pcl/prvTgfPclTcpRstFinMirroring.h>
#include <pcl/prvTgfPclPolicer.h>
#include <pcl/prvTgfPclPortGroupPolicer.h>
#include <pcl/prvTgfPclFullLookupControl.h>
#include <pcl/prvTgfPclMacModify.h>
#include <pcl/prvTgfPclTrunkHashVal.h>
#include <pcl/prvTgfPclUdePacketTypes.h>
#include <pcl/prvTgfPclEgrOrgVid.h>
#include <pcl/prvTgfPclRedirectToLTT.h>
#include <pcl/prvTgfPclLookup0ForRoutedPackets.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfBridgeGen.h>
#include <pcl/prvTgfPclMiscellanous.h>
#include <pcl/prvTgfPclLion2.h>
#include <common/tgfCosGen.h>
#include <pcl/prvTgfPclBc2.h>
#include <pcl/prvTgfPclQuadLookup.h>
#include <pcl/prvTgfPclEgrQuadLookup.h>
#include <pcl/prvTgfPclUdbMetadataSrcEPort.h>
#include <pcl/prvTgfPclNonMplsTransit.h>
#include <pcl/prvTgfPclMplsTransit.h>
#include <pcl/prvTgfPclSourceIdMask.h>
#include <pcl/prvTgfPclSourcePortSelect.h>
#include <pcl/prvTgfPclTargetPortSelect.h>
#include <pcl/prvTgfPclPortListGrouping.h>
#include <pcl/prvTgfPclUdbOnlyKeys.h>
#include <pcl/prvTgfPclUdb10KeyCustomer.h>
#include <pcl/prvTgfPclUdbL3Minus2.h>
#include <pcl/prvTgfPclEVlanBindingMode.h>
#include <pcl/prvTgfPclEPortBindingMode.h>
#include <pcl/prvTgfPclPhyPortBindingMode.h>
#include <pcl/prvTgfEgrPclEvlanBindingMode.h>
#include <pcl/prvTgfEgrPclEportBindingMode.h>
#include <pcl/prvTgfPclMetadataPacketType.h>
#include <pcl/prvTgfPclEgressTrapMirror.h>
#include <pcl/prvTgfPclEgressMirroring.h>

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - ingress PCL rule for non IP traffic:
    configure VLAN, FDB entries;
    configure PCL rule;
    send traffic to match and not match PCL rule;
    verify traffic is dropped (PCL rule) and forwarded (FDB).
*/
UTF_TEST_CASE_MAC(tgfIngressPclKeyStdL2)
{
/*
    1. Set configuration
    2. Set PCL configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfConfigurationSet();

    /* Set PCL configuration */
    prvTgfPclConfigurationSet();

    /* Generate traffic */
    prvTgfTrafficGenerate();

    /* Restore configuration */
    prvTgfConfigurationRestore();

    prvTgfPclRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - basic valid\ivalid PCL extended rules:
    configure VLAN, FDB entries;
    configure 2 PCL rules - to trap and forward not IPv4\IPv6 traffic;
    send traffic with enabled\disabled rules;
    verify traffic is trapped (PCL rule) and forwarded (FDB) according to rules validity.
*/
UTF_TEST_CASE_MAC(tgfPclValidInvalidRulesBasicExtended)
{
/*
    1. Set configuration
    2. Set PCL configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfPclValidInvalidRulesBasicExtendedConfigurationSet();

    /* Set PCL configuration */
    prvTgfPclValidInvalidRulesBasicExtendedPclConfigurationSet();

    /* Generate traffic */
    prvTgfPclValidInvalidRulesBasicExtendedTrafficGenerate();

    /* Restore configuration */
    prvTgfPclValidInvalidRulesBasicExtendedConfigurationRestore();

    prvTgfPclRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL rule action update:
    configure VLAN, FDB entries;
    configure 2 PCL rules - to trap and redirect not IPv4\IPv6 traffic;
    send traffic with updated rules;
    verify traffic is trapped\forwarded according to updated rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclValidInvalidRulesActionUpdate)
{
/*
    1. Set configuration
    2. Set PCL configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfPclValidInvalidRulesActionUpdateConfigurationSet();

    /* Set PCL configuration */
    prvTgfPclValidInvalidRulesActionUpdatePclConfigurationSet();

    /* Generate traffic */
    prvTgfPclValidInvalidRulesActionUpdateTrafficGenerate();

    /* Restore configuration */
    prvTgfPclValidInvalidRulesActionUpdateConfigurationRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL second lookup match:
    configure VLAN, FDB entries;
    configure 2 PCL rules with second lookup;
    send traffic to match PCL rules;
    verify traffic is forwarded\dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclSecondLookupMatch)
{
/*
    1. Set configuration
    2. Set PCL configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfPclSecondLookupMatchConfigurationSet();

    /* Set PCL configuration */
    prvTgfPclSecondLookupMatchPclConfigurationSet();

    /* Generate traffic */
    prvTgfPclSecondLookupMatchTrafficGenerate();

    /* Restore configuration */
    prvTgfPclSecondLookupMatchConfigurationRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL 2 user defined bytes:
    configure VLAN, FDB entries;
    configure PCL rule and 2 UDBs;
    send traffic to match PCL rules and UDBs;
    verify traffic is dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclBothUserDefinedBytes)
{
/*
    1. Set configuration
    2. Set PCL configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfPclBothUserDefinedBytesConfigurationSet();

    /* Set PCL configuration */
    prvTgfPclBothUserDefinedBytesPclConfigurationSet();

    /* Generate traffic */
    prvTgfPclBothUserDefinedBytesTrafficGenerate();

    /* Restore configuration */
    prvTgfPclBothUserDefinedBytesConfigurationRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs;
    send traffic to match PCL rules and UDBs;
    verify traffic is dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclUserDefinedBytes)
{
/*
    1. Set configuration
    2. Set PCL configuration
    3. Generate traffic
    4. Restore configuration
*/

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set bridge configuration */
    prvTgfPclBothUserDefinedBytesConfigurationSet();

    /* 1. Set PCL configuration to check IPV4 packet:
     * OFFSET_L2 0-8 bytes � Lion-B0 simulation
     * Generate traffic
     */
    prvTgfPclUserDefinedBytes_L2_0_8_ConfigurationSet();
    prvTgfPclBothUserDefinedBytesTrafficGenerate();

    /* 2. Set PCL configuration to check IPV4 packet:
     * OFFSET_L4 0-15 bytes � Lion-B0 simulation
     * Generate traffic
     */
    prvTgfPclUserDefinedBytes_L4_0_15_ConfigurationSet();
    prvTgfPclBothUserDefinedBytesTrafficGenerate();

    /* 3. Set PCL configuration to check ARP packet:
     * OFFSET_L2 0-11 bytes � Lion-B0 simulation
     * Generate traffic
     */
    prvTgfPclUserDefinedBytes_L2_0_11_ConfigurationSet();
    prvTgfPclBothUserDefinedBytesTrafficGenerate();

    /* Restore configuration */
    prvTgfPclBothUserDefinedBytesConfigurationRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes for TCP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclIpv4TcpUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Set configuration */
    prvTgfPclIpv4TcpConfigSet();

    /* Generate traffic */
    prvTgfPclIpv4TcpTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclIpv4TcpAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclIpv4TcpAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclIpv4TcpConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes for UDP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclIpv4UdpUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Set configuration */
    prvTgfPclIpv4UdpConfigSet();

    /* Generate traffic */
    prvTgfPclIpv4UdpTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclIpv4UdpAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclIpv4UdpAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclIpv4UdpConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes for IPv4 (DF) packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclIpv4FragmentUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Set configuration */
    prvTgfPclIpv4FragmentConfigSet();

    /* Generate traffic */
    prvTgfPclIpv4FragmentTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclIpv4FragmentAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclIpv4FragmentAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclIpv4FragmentConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes for other IP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclIpv4OtherUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Set configuration */
    prvTgfPclIpv4OtherConfigSet();

    /* Generate traffic */
    prvTgfPclIpv4OtherTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclIpv4OtherAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclIpv4OtherAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclIpv4OtherConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes for other Ethernet packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclEthernetOtherUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Set configuration */
    prvTgfPclEthernetOtherConfigSet();

    /* Generate traffic */
    prvTgfPclEthernetOtherTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclEthernetOtherAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclEthernetOtherAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclEthernetOtherConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes for MPLS packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclMplsUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Set configuration */
    prvTgfPclMplsConfigSet();

    /* Generate traffic */
    prvTgfPclMplsTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclMplsAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclMplsAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclMplsConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes for UDE packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclUdeUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Set configuration */
    prvTgfPclUdeConfigSet();

    /* Generate traffic */
    prvTgfPclUdeTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclUdeAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclUdeAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclUdeConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes for IPv6 packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclIpv6Udb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Set configuration */
    prvTgfPclIpv6ConfigSet();

    /* Generate traffic */
    prvTgfPclIpv6TrafficGenerate();

    /* Set additional configuration */
    prvTgfPclIpv6AdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclIpv6AdditionalTrafficGenerate();

    if(prvTgfResetModeGet() == GT_TRUE)
    {
        prvTgfResetAndInitSystem();
    }
    else
    {
        /* Restore configuration */
        prvTgfPclIpv6ConfigRestore();

        prvTgfPclRestore();
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes for IPv6 TCP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclIpv6TcpUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Set configuration */
    prvTgfPclIpv6TcpConfigSet();

    /* Generate traffic */
    prvTgfPclIpv6TcpTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclIpv6TcpAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclIpv6TcpAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclIpv6TcpConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes for Ethernet packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    override User Defined Bytes with by predefined key;
    override VRF Id related UDB values;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclEthernetOtherVridUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT2_E | UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        return;
    }

    /* Set configuration */
    prvTgfPclEthernetOtherVridConfigSet();

    /* Generate traffic */
    prvTgfPclEthernetOtherVridTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclEthernetOtherVridAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclEthernetOtherVridAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclEthernetOtherVridConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL user defined bytes for IPv4 UDP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    override User Defined Bytes with by predefined key;
    set Qos Trust Mode for ingess port;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclIpv4UdpQosUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_PUMA_E);

    /* Set configuration */
    prvTgfPclIpv4UdpQosConfigSet();

    /* Generate traffic */
    prvTgfPclIpv4UdpQosTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclIpv4UdpQosAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclIpv4UdpQosAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclIpv4UdpQosConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfPclUdbFldsIsIp
*/

UTF_TEST_CASE_MAC(tgfPclUdbFldsIsIp)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IS_IP_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IS_IP_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfPclUdbFldsPacketDepend
*/

UTF_TEST_CASE_MAC(tgfPclUdbFldsPacketDepend)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* common tests note 1 */
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_VID_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_VID_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_UP_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_UP_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_DSCP_OR_EXP_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_DSCP_OR_EXP_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_MAC_DA_TYPE_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_MAC_DA_TYPE_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_L4_PROTOCOL_TYPE_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_L4_PROTOCOL_TYPE_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_PKT_TAGGING_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_PKT_TAGGING_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

#if 0
    /* not debugged cases */
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_PKT_TYPE_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_PKT_TYPE_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IS_UDB_VALID_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IS_UDB_VALID_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_L3_OFFSET_INVALIDATE_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_L3_OFFSET_INVALIDATE_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IP_HEADER_OK_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IP_HEADER_OK_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_L4_OFFSET_INVALID_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_L4_OFFSET_INVALID_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_L2_ENCAPSULATION_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_L2_ENCAPSULATION_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IS_IPV6_EH_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IS_IPV6_EH_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IS_IPV6_HOP_BY_HOP_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IS_IPV6_HOP_BY_HOP_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);
#endif

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfPclUdbFldsNotPacketDepend
*/

UTF_TEST_CASE_MAC(tgfPclUdbFldsNotPacketDepend)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* common tests note 2 */
    prvTgfPclUdbFldsTwoPorts(PRV_TGF_PCL_RULE_FORMAT_PCL_ID_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFldsTwoPorts(PRV_TGF_PCL_RULE_FORMAT_PCL_ID_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclUdbFldsTwoPorts(PRV_TGF_PCL_RULE_FORMAT_SOURCE_PORT_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFldsTwoPorts(PRV_TGF_PCL_RULE_FORMAT_SOURCE_PORT_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfPclUdbFldsMacToMe
*/

UTF_TEST_CASE_MAC(tgfPclUdbFldsMacToMe)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* common tests note 3 */
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_MAC_TO_ME_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_MAC_TO_ME_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfPclUdbFldsComplicated
*/

UTF_TEST_CASE_MAC(tgfPclUdbFldsComplicated)
{
    /* relevant for Puma2 but yet not adapted */
    CPSS_TBD_BOOKMARK

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E
        | UTF_CH2_E | UTF_CH3_E | UTF_XCAT2_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        return;
    }

    /* common tests note 3 */
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IS_L2_VALID_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IS_L2_VALID_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfPclUdbFldsStd
*/

UTF_TEST_CASE_MAC(tgfPclUdbFldsStd)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* std tests only */
    prvTgfPclUdbFlds(
        PRV_TGF_PCL_RULE_FORMAT_IS_IPV4_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E);
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfPclUdbFldsExt
*/

UTF_TEST_CASE_MAC(tgfPclUdbFldsExt)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* ext tests only */
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IS_IPV6_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_IP_PROTOCOL_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_SIP_BITS_31_0_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_SIP_BITS_79_32_OR_MAC_SA_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_SIP_BITS_127_80_OR_MAC_DA_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_DIP_BITS_127_112_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);
    prvTgfPclUdbFlds(PRV_TGF_PCL_RULE_FORMAT_DIP_BITS_31_0_E, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E);
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfPclBypassBridge
*/

UTF_TEST_CASE_MAC(tgfPclBypassBridge)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_PUMA_E);

    /* Test of Bypass bridge feature */
    prvTgfPclBypass(PRV_TGF_PCL_BYPASS_BRIDGE_E);
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfPclBypassIngressPipe
*/

UTF_TEST_CASE_MAC(tgfPclBypassIngressPipe)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_PUMA_E);

    /* Test of Bypass ingress pipe feature */
    prvTgfPclBypass(PRV_TGF_PCL_BYPASS_INGRESS_PIPE_E);
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - action TCP RST/FIN mirroring feature:
    configure VLAN, FDB entries;
    configure PCL rule with action TCP RST/FIN mirror for all packets;
    send TCP RST\FIN Matched\Not Matched traffic;
    verify traffic on CPU and FDB ports.
*/
UTF_TEST_CASE_MAC(tgfPclTcpRstFinMirroring)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_PUMA_E);

    /* Test of action TCP RST/FIN mirroring feature */
    prvTgfPclTcpRstFinMirroring();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL action policer counting feature:
    configure VLAN, FDB entries;
    configure default DP to be green;
    configure policer entry that remarks DP to be RED any case (minimal burst);
    configure Rule with action bound to this policer entry counting only;
    send 2 matched packets and 3 unmatched packets;
    read billing counters and verify that the 2 matched packets counted;
    read management counters and verify 2 GREEN counter, 0 YELLOW and RED.
*/
UTF_TEST_CASE_MAC(tgfPclPolicerCounting)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E
        | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    /* Test of action Policer Counting only */
    prvTgfPclPolicerCounting();
    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfPclPolicerCounting_qosMappingTableIterator)
{
    /* test with qos mapping table iterator  */
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfPclPolicerCounting));
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL action policer metering feature:
    configure VLAN, FDB entries;
    configure default DP to be green;
    configure policer entry that remarks DP to be RED any case (minimal burst);
    configure Rule with action bound to this policer entry metering only;
    send 2 matched packets and 3 unmatched packets;
    read billing counters and verifythat 2 matched packets not counted;
    read management counters and verify GREEN and YELLOW 0 and RED 2.
*/

UTF_TEST_CASE_MAC(tgfPclPolicerMetering)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    /* Test of action Policer Metering only */
    prvTgfPclPolicerMetering();
    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfPclPolicerMetering_qosMappingTableIterator)
{
    /* test with qos mapping table iterator  */
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfPclPolicerMetering));
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL action policer flow id based counting feature:
    configure VLAN, FDB entries;
    configure default DP to be green;
    configure Rule with action bound to billing entry only;
    configure Flow Id based to other billing entry.
    send 2 matched packets and 3 unmatched packets;
    read billing counters and verify that the 2 matched packets counted;
*/
UTF_TEST_CASE_MAC(prvTgfPclPolicerCountingFlowIdBased)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    /* Test of action Policer Counting only */
    prvTgfPclPolicerCountingFlowIdBased();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL action policer hierarchical counting feature:
    configure VLAN, FDB entries;
    Clear billing entries entry0_0 and entry0_1 in stage0 and entry1 in stage1.
    Remap index0_0 and index0_1 to index1 (in stge0 for stage1)
    Bind all packets to entry0_0 by PCL rule (Set Rule)
    send 2 packets;
    Bind all packets to entry0_1 by PCL rule (Update Action).
    send 1 packet;
    read billing counters and verify that conted packets:
    entry0_0 - 2, entry0_1 - 1, entry1 - 3.
*/
UTF_TEST_CASE_MAC(prvTgfPclPolicerCountingHierarchical)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    /* Test of action Policer Counting only */
    prvTgfPclPolicerCountingHierarchicalTest();
    prvTgfPclPolicerCountingHierarchicalReset();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - DP to Egress CFI Mapping feature:
    Configure VLAN, FDB entries;
    Enable mapping DP to CFI,
    Set map: Green => 0, Yellow => 1, Red => 0.
    Bind all packets to Egress Meter Entry that changes DP to Yellow
    by Egress PCL rule (Set Rule)
    Send 1 packet with CFI==0;
    Verify captured egress packet CFY==1
*/
UTF_TEST_CASE_MAC(prvTgfPclPolicerDp2CfiMap)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    /* Test of action Policer Counting only */
    prvTgfPclPolicerDp2CfiMapTest();
    prvTgfPclPolicerDp2CfiMapRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL action policer counting feature - billing counters (for multi-port groups devices):
    configure VLAN, FDB entries;
    configure QoS profile and set default DP to be green;
    configure policer entry that remarks DP to be RED any case (minimal burst);
    configure Rule with action bound to this policer entry counting only;
    send 2 matched packets and 3 unmatched packets;
    read billing counters and verify that the 2 matched packets counted;
    read management counters and verify 2 GREEN counter, 0 YELLOW and RED.
*/
UTF_TEST_CASE_MAC(tgfPclPortGroupPolicerCounting)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_XCAT_E | UTF_XCAT3_E | UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Test of action Policer Counting only */
    prvTgfPclPortGroupPolicerCounting();
    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfPclPortGroupPolicerCounting_qosMappingTableIterator)
{
    /* skip single port group device from this long test */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* test with qos mapping table iterator  */
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfPclPortGroupPolicerCounting));
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL action policer metering feature - billing and management counters (for multi-port groups devices):
    configure VLAN, FDB entries;
    configure QoS profile and set default DP to be green;
    configure policer entry that remarks DP to be RED any case (minimal burst);
    configure Rule with action bound to this policer entry metering only;
    send 2 matched packets and 3 unmatched packets;
    read billing counters and verifythat 2 matched packets not counted;
    read management counters and verify GREEN and YELLOW 0 and RED 2.
*/
UTF_TEST_CASE_MAC(tgfPclPortGroupPolicerMetering)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_XCAT_E | UTF_XCAT3_E | UTF_PUMA_E);

    /* Test of action Policer Metering only */
    prvTgfPclPortGroupPolicerMetering();
    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfPclPortGroupPolicerMetering_qosMappingTableIterator)
{
    /* skip single port group device from this long test */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* test with qos mapping table iterator  */
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfPclPortGroupPolicerMetering));
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL action full lookup control:
    configure VLAN, FDB entries;
    enable full lookup option for lookup0_1;
    configure 3 PCL entries (0,0x1FF,0x3FF): 1 for ingress port, 2 in vlan area;
    create 3 rules checking PCL ID only for PCL ID values 0,0x1FF and 0x3FF;
    actions in these rules must modify VID (rule0), UP(rule1), DSCP(rule2);
    send tagged IPV4 packet;
    verify that egressed packet has modified VID, UP, DSCP;
    update Cfg Table to Generate keys with PCL Ids 10,11,12 for lookups 0_0, 0_1 and 1;
    add 3 new Rules modifying VID (rule0), UP(rule1), DSCP(rule2);
    send the same packet and check that the new remarking done.
*/
UTF_TEST_CASE_MAC(tgfPclFullLookupControl)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Test of action Full lookup control */
    prvTgfPclFullLookupControl();
    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfPclFullLookupControl_qosMappingTableIterator)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* test with qos mapping table iterator  */
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfPclFullLookupControl));
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL redirection with MAC DA modification:
    configure ARP entry with specific MAC;
    configure PCL rule to redirect traffic with DA modification;
    send ethernet traffic;
    verify captured traffic has same MAC DA as configured in ARP entry.
*/
UTF_TEST_CASE_MAC(tgfPclRedirectWithMacDaModify)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_PUMA_E);

    /* xCat2 do not support feature because of erratum */

    /* Test of redirection with MAC_DA modification */
    prvTgfPclRedirectMacDaModify();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL redirection with MAC SA modification:
    enable MAC SA modification;
    configure 40 MSBs and 8 LSB of SA base addr;
    configure PCL rule to redirect traffic with SA modification;
    send ethernet traffic;
    verify captured traffic has same MAC SA as configured.
*/
UTF_TEST_CASE_MAC(tgfPclRedirectWithMacSaModify)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_XCAT_E | UTF_XCAT3_E | UTF_PUMA_E);

    /* Test of redirection with MAC_SA modification */
    prvTgfPclRedirectMacSaModify();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL UDB override by Trunk Hash Value use:
    configure 2 VLAN entries;
    configure PCL and UDB entries;
    enable UDB override by Trunk Hash value for VID;
    send Ethernet traffic;
    verify that egressed packet has modified VID;
    disable UDB override by Trunk Hash value for VID;
    send Ethernet traffic;
    verify that egressed packet has same VID.
*/
UTF_TEST_CASE_MAC(prvTgfPclOverrideUserDefinedBytesByTrunkHashSet)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_XCAT_E | UTF_XCAT3_E | UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Test of Trunk Hash Value use */
    prvTgfPclTrunkHashTrafficTest();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL UDB override by UDE Packet types use:
    configure VLAN entries;
    configure PCL and UDB entries;
    configure specific UDE EtherTypes;
    send traffic with configured EtherTypes;
    verify that egressed packet has modified VID.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdeEtherTypeSet)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_XCAT_E | UTF_XCAT3_E | UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Test of UDE Packet types use */
    prvTgfPclUdePktTypesTrafficTest();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL egress key VID modification:
    configure VLAN entries;
    configure PCL and add PCL rules to modify outer VLAN tag;
    send double tagged traffic;
    verify that egressed packet has modified outer TAG VID.
*/
UTF_TEST_CASE_MAC(prvTgfPclEgressKeyFieldsVidUpModeSet)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_XCAT_E | UTF_XCAT3_E | UTF_CPSS_PP_E_ARCH_CNS | UTF_PUMA_E);

    /* Test of UDE Packet types use */
    prvTgfPclEgrOrgVidTrafficTest();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Egress PASS/DROP test on SIP and DIP of key : Egress Ext Ipv4 Racl/Vacl:
    configure VLAN entry;
    configure PCL and add PCL rules to PASS/DROP packet (once on PASS once on DROP);
    send IPv4 to match the rule traffic;
    verify that egress packet PASS/DROP .
*/
UTF_TEST_CASE_MAC(prvTgfPclEgrExtIpv4RaclVaclSipDipTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum,
            UTF_DXCH_E - (UTF_LION2_E | UTF_XCAT3_E | UTF_CPSS_PP_E_ARCH_CNS));

    prvTgfPclEgrExtIpv4RaclVaclSipDipTest();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Egress PASS/DROP test on SIP and DIP of key : Egress Ultra Ipv6 Racl/Vacl:
    configure VLAN entry;
    configure PCL and add PCL rules to PASS/DROP packet (once on PASS once on DROP);
    send IPv6 to match the rule traffic;
    verify that egress packet PASS/DROP .
*/
UTF_TEST_CASE_MAC(prvTgfPclEgrUltraIpv6RaclVaclSipDipTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum,
            UTF_DXCH_E - (UTF_LION2_E | UTF_XCAT3_E | UTF_CPSS_PP_E_ARCH_CNS));

    prvTgfPclEgrUltraIpv6RaclVaclSipDipTest();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL action lookup 00 and lookup01 control:
    configure VLAN, FDB entries;
    enable full lookup option for lookup00 and lookup01;
    configure PCL entries and create PCL rules;
    send Ethernet traffic to match\not match lookup00 and lookup01;
    verify that egressed packet has modified VID or UP.
*/
UTF_TEST_CASE_MAC(prvTgfPclLookup00And01)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_PUMA_E);

    /* Test of xCat2 lookup0_0 and Lookup0_1 different PCL keys */
    prvTgfPclLookup00And01();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL port list mode:
    configure VLAN, FDB entries;
    enable full lookup option for lookup0 and lookup1;
    enable using port-list in search keys for each lookup;
    configure PCL entries and create PCL rules;
    send Ethernet traffic to match\not match lookup00 and lookup01;
    verify that egressed packet has modified VID or UP.
*/
UTF_TEST_CASE_MAC(prvTgfPclPortListMode)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_PUMA_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Test of xCat2 port list mode */
    prvTgfPclPortListMode();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL invalid rule write:
    configure VLAN, FDB entries;
    enable full lookup option for lookup0 and lookup1;
    configure PCL entries and create PCL rules in invalid state;
    send Ethernet traffic to match both lookup;
    verify that egressed packet has no modified VID or UP;
    validate PCL rules;
    send Ethernet traffic to match both lookup;
    verify that egressed packet has modified VID or UP.
*/
UTF_TEST_CASE_MAC(prvTgfPclWriteRuleInvalid)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Test of invalid rule writing */
    prvTgfPclWriteRuleInvalid();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL four lookups packet matching:
    configure VLAN, FDB entries;
    enable full lookup option for 4 lookups;
    configure PCL entries and create PCL rules;
    send Ethernet traffic to match each lookup;
    verify that egressed packet has modified VID or UP.
*/
UTF_TEST_CASE_MAC(prvTgfPclFourLookups)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_PUMA_E);

    /* Test of four lookups packet matching */
    prvTgfPclFourLookups();
    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfPclLion2VidAndExt
*/
UTF_TEST_CASE_MAC(prvTgfPclLion2VidAndExt)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Test of VID0 and VID1 igress and egress modification */
    prvTgfPclLion2VidAndExtTrafficTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL Ingress list mode:
    configure VLAN, FDB entries;
    configure PCL table and add PCL rules for all ports and lookup0;
    enable using port-list in search keys for lookup0;
    send 4 Ethernet packets on each port to match each PCL rule;
    verify that 4 egressed packets has modified VID.
*/
UTF_TEST_CASE_MAC(prvTgfPclPortListCrossIpclTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_PUMA_E);

    /* Test of xCat2 and above Ingress PCL port list mode */
    prvTgfPclPortListCrossIpclTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL Egress list mode:
    configure VLAN, FDB entries;
    configure egress PCL table and add PCL rules for all ports and lookup0;
    enable egress using port-list in search keys for lookup0;
    send 4 Ethernet packets on each port to match each PCL rule;
    verify that 4 egressed packets has modified VID.
*/
UTF_TEST_CASE_MAC(prvTgfPclPortListCrossEpclTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_PUMA_E);

    /* Test of xCat2 and above Ingress PCL port list mode */
    prvTgfPclPortListCrossEpclTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - ingress PCL extension for ePorts:
    configure VLAN, FDB entries;
    configure PCL entries and create extended PCL rules for ePort;
    send Ethernet traffic to match rule;
    verify egressed packet has double tag with modified VID.
*/
UTF_TEST_CASE_MAC(prvTgfPclEportEvlanIpclTest)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclEportEvlanIpclTrafficTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - egress PCL extension for ePorts:
    configure VLAN, FDB entries;
    configure PCL entries and create extended egress PCL rules for ePort;
    send Ethernet traffic to match rule;
    verify egressed packet has double tag with modified VID.
*/
UTF_TEST_CASE_MAC(prvTgfPclEportEvlanEpclTest)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclEportEvlanEpclTrafficTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - ingress PCL extension for eVlan:
    configure VLAN, FDB entries;
    configure PCL entries and create extended PCL rules for eVlan;
    send Ethernet traffic to match rule;
    verify egressed packet has double tag with modified VIDs.
*/
UTF_TEST_CASE_MAC(prvTgfPclEvlanMatchIpclTest)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclEvlanMatchIpclTrafficTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - egress PCL extension for eVlan:
    configure VLAN, FDB entries;
    configure PCL entries and create egress extended PCL rules for eVlan;
    send Ethernet traffic to match rule;
    verify egressed packet has double tag with modified VIDs.
*/
UTF_TEST_CASE_MAC(prvTgfPclEvlanMatchEpclTest)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclEvlanMatchEpclTrafficTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL redirection to ePorts:
    configure VLAN, FDB entries;
    map ePorts to physical port;
    configure PCL entries and create PCL rules to redirect to ePort;
    send Ethernet traffic to match rule;
    verify packet redirected to ePort.
*/
UTF_TEST_CASE_MAC(prvTgfPclEArchRedirectEPort)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclEArchRedirectIpclEPortTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL redirection to eVids:
    configure VLAN, FDB entries;
    map eVids to physical port;
    configure PCL entries and create PCL rules to flood to Vlan;
    send Ethernet traffic to match rule;
    verify packet redirected to Vlan.
*/
UTF_TEST_CASE_MAC(prvTgfPclEArchRedirectEVid)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclEArchRedirectIpclEVidTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL redirection to eVidx:
    configure VLAN, FDB, LTT entries;
    map eVidxs to physical port;
    configure PCL entries and create PCL rules to redirect to eVidx;
    send Ethernet traffic to match rule;
    verify packet redirected to configured port.
*/
UTF_TEST_CASE_MAC(prvTgfPclEArchRedirectEVidx)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclEArchRedirectIpclEVidxTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - ingress PCL TAG1 match:
    configure VLAN, FDB entries;
    configure PCL entries and create PCL rules for TAG1;
    send Ethernet traffic to match rule;
    verify egressed packet has double tag with modified VID.
*/
UTF_TEST_CASE_MAC(prvTgfPclTag1MatchIpclTest)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclTag1MatchIpclTrafficTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - egress PCL TAG1 match:
    configure VLAN, FDB entries;
    configure PCL entries and create egress PCL rules for TAG1;
    send Ethernet traffic to match rule;
    verify packet has double tag with modified VID.
*/
UTF_TEST_CASE_MAC(prvTgfPclTag1MatchEpclTest)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclTag1MatchEpclTrafficTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - ingress PCL source Trunk ID match:
    configure VLAN, FDB, TRUNK entries;
    configure PCL entries and create PCL rules for TRUNK;
    send Ethernet traffic to match rule;
    verify egressed packet has double tag with modified VID.
*/
UTF_TEST_CASE_MAC(prvTgfPclSrcTrunkIdMatchIpcl)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclSrcTrunkIdMatchIpclTrafficTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL rule redirect to LTT entry:
    configure VLAN, FDB entries;
    configure UC Routing entry and LTT entry;
    configure PCL entries and create PCL rules to redirect to LTT entry;
    send IPv4 traffic to match rule;
    verify egressed packet redirected to configured interface.
*/
UTF_TEST_CASE_MAC(prvTgfPclRedirectToLTT)
{
    /* relevant for Puma2 but yet not adapted */
    CPSS_TBD_BOOKMARK

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_XCAT2_E);

    if (GT_TRUE == prvUtfIsPbrModeUsed())
    {
        return;
    }

    /* Test of redirection to LTT */
    prvTgfPclRedirectToLTT();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - ingress PCL UDB override by QoS profile:
    configure VLAN, FDB, QoS entries;
    configure PCL entries and create 2 PCL rules to modify Vlan;
    disable overriding UDB content by QoS profile;
    send Ethernet traffic to match rules;
    verify egressed packet has modified TAG0 by Rule 1;
    enable overriding UDB content by QoS profile;
    send Ethernet traffic to match rules;
    verify egressed packet has modified TAG0 by Rule 2.
*/
UTF_TEST_CASE_MAC(prvTgfPclLion2UdbOverrideIpclQosProfile)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclLion2UdbOverrideIpclQosProfileTrafficTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - ingress PCL UDB override by TAG1 info:
    configure VLAN, FDB, QoS entries;
    configure PCL entries and create 2 PCL rules to modify Vlan;
    disable overriding UDB content by TAG1 info;
    send Ethernet traffic to match rules;
    verify egressed packet has modified TAG0 by Rule 1;
    enable overriding UDB content by TAG1 info;
    send Ethernet traffic to match rules;
    verify egressed packet has modified TAG0 by Rule 2.
*/
UTF_TEST_CASE_MAC(prvTgfPclLion2UdbOverrideIpclTag1)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclLion2UdbOverrideIpclTag1TrafficTest();
}

/* FWS test */

/*----------------------------------------------------------------------------*/
/*
    prvTgfPclFwsCnc
*/

UTF_TEST_CASE_MAC(prvTgfPclFwsCnc)
{
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_PUMA2_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;


#ifndef ASIC_SIMULATION
    prvTgfPclFWSCncTest();
    prvTgfPclFWSCncRestore(GT_FALSE);
#endif

}

/*
    prvTgfPclFwsMove
*/

UTF_TEST_CASE_MAC(prvTgfPclFwsMove)
{
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_PUMA2_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfPclFWSMoveTest(GT_FALSE, GT_FALSE, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E);
    prvTgfPclFWSCncRestore(GT_FALSE);
#endif

}

/*
    prvTgfPclFwsMove_virtTcam
*/

UTF_TEST_CASE_MAC(prvTgfPclFwsMove_virtTcam)
{
    GT_U32 notAppFamilyBmp;

    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~UTF_XCAT3_E) & (~UTF_LION2_E)) ;
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfPclFWSMoveTest(GT_TRUE, GT_FALSE, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E);
    prvTgfPclFWSCncRestore(GT_TRUE);
#endif

}


/*
    prvTgfPclFwsMove_virtTcam_80B
*/

UTF_TEST_CASE_MAC(prvTgfPclFwsMove_virtTcam_80B)
{
    GT_U32 notAppFamilyBmp;

    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~UTF_XCAT3_E) & (~UTF_LION2_E)) ;
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfPclFWSMoveTest(GT_TRUE, GT_FALSE, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E);
    prvTgfPclFWSCncRestore(GT_TRUE);
#endif

}


/*
    prvTgfPclFwsMove_virtTcamPriority
*/

UTF_TEST_CASE_MAC(prvTgfPclFwsMove_virtTcamPriority)
{
    GT_U32 notAppFamilyBmp;

    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~UTF_XCAT3_E) & (~UTF_LION2_E)) ;
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfPclFWSMoveTest(GT_TRUE, GT_TRUE, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E);
    prvTgfPclFWSCncRestore(GT_TRUE);
#endif

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL UDB and TCP/UDP port comparators:
    configure VLAN entries;
    configure PCL entries for send port;
    configure UDP comparator entry for src port type;
    configure 2 UDBs for IPv4 and L2 offset;
    add PCL rule for UDB pattern to modify DSCP;
    send UDP traffic to match rules;
    verify egressed packet has modified DSCP field;
    send UDP traffic to not match rules;
    verify egressed packet has the same DSCP field.
*/
UTF_TEST_CASE_MAC(prvTgfPclPumaPortGroupTestUnaware)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);


    /* Test of Puma UDB and TCP/UDP port comparators */
    prvTgfPclPumaPortGroupTestUnaware();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL UDB and TCP/UDP port comparators for port groups:
    configure VLAN entries;
    configure PCL entries for send port;
    configure UDP comparator entry for src port type;
    configure 2 UDBs for IPv4 and L2 offset;
    add PCL rule for UDB pattern to modify DSCP;
    send UDP traffic to match rules;
    verify egressed packet has modified DSCP field;
    send UDP traffic to not match rules;
    verify egressed packet has the same DSCP field.
*/
UTF_TEST_CASE_MAC(prvTgfPclPumaPortGroupTestAware)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);


    if (0 == PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(prvTgfDevNum))
    {
        return;
    }

    /* Test of Puma UDB and TCP/UDP port comparators */
    prvTgfPclPumaPortGroupTestAware();
}

/*
    prvTgfPclBc2SetMac2me
*/

UTF_TEST_CASE_MAC(prvTgfPclBc2SetMac2me)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclBc2Mac2meTest();
    prvTgfPclBc2Mac2meTestRestore();
}

/*
    prvTgfPclBc2SetMacSa
*/

UTF_TEST_CASE_MAC(prvTgfPclBc2SetMacSa)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclBc2SetMacSaTest();
    prvTgfPclBc2SetMacSaTestRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Enable/disable PCL lookup 0 for routed packets:
    configure VLAN entries;
    configure PCL rules on both lookups - forward for lookup 0, drop for lookup 1;
    Disable PCL lookup0 for routed packets;
    send UDP traffic to match rules - check that the packet is forwarded;
    Enable PCL lookup0 for routed packets;
    send UDP traffic to match rules - check that the packet is dropped;
*/
UTF_TEST_CASE_MAC(prvTgfPclLookup0ForRoutedPackets)
{
/*
    1. Set Base configuration
    2. Set PCL configuration
    3. Disable PCL lookup0 for routed packets
    4. Generate traffic - expect no traffic
    5. Enable PCL lookup0 for routed packets
    6. Generate traffic - expect traffic
    7. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfPclLookup0ForRoutedPacketsConfigurationSet();

    /* Set PCL configuration */
    prvTgfPclLookup0ForRoutedPacketsMatchConfigurationSet();

    /* Disable PCL lookup0 for routed packets */
    prvTgfPclLookup0ForRoutedPacketsConfigurationEnableSet(GT_FALSE);

    /* Generate traffic - expect no traffic */
    prvTgfPclLookup0ForRoutedPacketsTrafficGenerate(GT_TRUE);

    /* Enable PCL lookup0 for routed packets */
    prvTgfPclLookup0ForRoutedPacketsConfigurationEnableSet(GT_TRUE);

    /* Generate traffic - expect traffic */
    prvTgfPclLookup0ForRoutedPacketsTrafficGenerate(GT_FALSE);

    /* Restore configuration */
    prvTgfPclLookup0ForRoutedPacketsConfigurationRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - PCL sourse ePort assignment:
    configure VLAN, FDB entries;
    map ePorts to physical port;
    configure PCL entries and create PCL rules to assign source ePort;
    enable forwarding of NA to CPU on the new src eport;
    send Ethernet traffic to match rule;
    verify that packet received on all ports of VLAN;
    send other packet with MAC DA == (MAC SA of learned MAC on new port);
    check traffic on assigned ePort and other ports received 0 packets.
*/
UTF_TEST_CASE_MAC(prvTgfPclEArchSourcePort)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclEArchSourcePortTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Ingress PCL Quad lookup:
    Test the various TCAM segment mode (logical division) as set in the IPCL
    Configuration Table, while all lookups can return a match.
*/
UTF_TEST_CASE_MAC(prvTgfPclIngressQuadLookup)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclIngressQuadLookupTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Ingress PCL Quad lookup with vTCAMs:
    Test the various vTCAMs segment (logical division) as set in the IPCL
    Configuration Table, while all lookups can return a match.
*/
UTF_TEST_CASE_MAC(prvTgfPclIngressQuadLookup_virtTcam)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclIngressQuadLookupTest_virtTcam();
}

/* AUTODOC: Test - Ingress PCL Quad lookup with vTCAMs add rule by priority:
    Test the various vTCAMs segment (logical division) as set in the IPCL
    Configuration Table, while all lookups can return a match.
*/
UTF_TEST_CASE_MAC(prvTgfPclIngressQuadLookup_virtTcamPriority)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclIngressQuadLookupTest_virtTcamPriority();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Ingress PCL Quad lookup with unmatched lookups:
    Test the various TCAM segment mode (logical division) as set in the IPCL
    Configuration Table, while some lookups do not return a match.
*/
UTF_TEST_CASE_MAC(prvTgfPclIngressQuadLookupWithInvalid)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfPclIngressQuadLookupTestWithInvalid();
}

/*
    prvTgfPclBc2MapMplsChannelToOamOpcode
*/
/* AUTODOC: Test - Egress PCL actions causes treat MPLS packet as
OAM packet with mapping MPLS Channel Type to OAM opcode instead of
extracting OAM Code from packet according to UDB configuration.
The same packet sent twice with different states of Opcode-cmdProfile
Command table. For first send an appropriate cell of the table contains
DROP command, the second time - FORWARD.
The result checked using the egress port MAC Counters.
*/

UTF_TEST_CASE_MAC(prvTgfPclBc2MapMplsChannelToOamOpcode)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclBc2MapMplsChannelToOamOpcodeTest();
    prvTgfPclBc2MapMplsChannelToOamOpcodeReset();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Egress PCL Quad lookup:
    Test the various TCAM segment mode (logical division) as set in the EPCL
    Configuration Table, while all lookups can return a match.
*/
UTF_TEST_CASE_MAC(prvTgfPclEgressQuadLookup)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclEgressQuadLookupTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Egress PCL Quad lookup with unmatched lookups:
    Test the various TCAM segment mode (logical division) as set in the EPCL
    Configuration Table, while some lookups do not return a match.
*/
UTF_TEST_CASE_MAC(prvTgfPclEgressQuadLookupWithInvalid)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfPclEgressQuadLookupTestWithInvalid();
}

/* AUTODOC: Test - PCL user defined bytes for other IP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleStdNotIpUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= ~UTF_XCAT3_E;
    notAppFamilyBmp &= ~UTF_LION2_E;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleStdNotIpUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleStdNotIpUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleStdNotIpUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleStdNotIpUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleStdNotIpUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for UDP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleStdIpL2QosUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= ~UTF_XCAT3_E;
    notAppFamilyBmp &= ~UTF_LION2_E;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleStdIpL2QosUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleStdIpL2QosUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleStdIpL2QosUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleStdIpL2QosUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleStdIpL2QosUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for UDP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleStdIpv4L4Udb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleStdIpv4L4UdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleStdIpv4L4UdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleStdIpv4L4UdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleStdIpv4L4UdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleStdIpv4L4UdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for IPV6 TCP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleStdIpv6DipUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleStdIpv6DipUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleStdIpv6DipUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleStdIpv6DipUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleStdIpv6DipUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleStdIpv6DipUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for other IP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleExtNotIpv6Udb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleExtNotIpv6UdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleExtNotIpv6UdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleExtNotIpv6UdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleExtNotIpv6UdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleExtNotIpv6UdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for IPV6 TCP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleExtIpv6L2Udb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleExtIpv6L2UdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleExtIpv6L2UdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleExtIpv6L2UdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleExtIpv6L2UdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleExtIpv6L2UdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for IPV6 TCP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleExtIpv6L4Udb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleExtIpv6L4UdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleExtIpv6L4UdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleExtIpv6L4UdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleExtIpv6L4UdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleExtIpv6L4UdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for UDP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleIngrStdUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleIngrStdUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleIngrStdUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleIngrStdUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleIngrStdUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleIngrStdUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for UDP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleIngrExtUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleIngrExtUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleIngrExtUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleIngrExtUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleIngrExtUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleIngrExtUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for UDP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleStdIpv4RoutedAclQosUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleStdIpv4RoutedAclQosUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleStdIpv4RoutedAclQosUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleStdIpv4RoutedAclQosUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleStdIpv4RoutedAclQosUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleStdIpv4RoutedAclQosUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for UDP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleExtIpv4PortVlanQosUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleExtIpv4PortVlanQosUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleExtIpv4PortVlanQosUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleExtIpv4PortVlanQosUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleExtIpv4PortVlanQosUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleExtIpv4PortVlanQosUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for IPV6 TCP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleUltraIpv6PortVlanQosUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= ~UTF_XCAT3_E;
    notAppFamilyBmp &= ~UTF_LION2_E;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleUltraIpv6PortVlanQosUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleUltraIpv6PortVlanQosUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleUltraIpv6PortVlanQosUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleUltraIpv6PortVlanQosUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleUltraIpv6PortVlanQosUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for IPV6 TCP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleUltraIpv6RoutedAclQosUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleUltraIpv6RoutedAclQosUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleUltraIpv6RoutedAclQosUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleUltraIpv6RoutedAclQosUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleUltraIpv6RoutedAclQosUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleUltraIpv6RoutedAclQosUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - IPCL source ePort match by metadata:
    IPCL matching source ePort by UDBs configuration based on metadata ancor.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbMetadataSrcEport)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Test flow - configuration, traffic & restore */
    prvTgfPclUdbMetadataSrcEportTest();

    prvTgfPclRestore();
}

/* AUTODOC: Test - Non-MPLS Transit Tunnel:
    Passenger parsing of Non-MPLS Transit Tunnel pacekts. IPCL fixed key fields
    (non-UDB) are based on tunnel header, while IPCL UDB key fields can be
    relative to the passenger header or the tunnel header.
*/
UTF_TEST_CASE_MAC(prvTgfPclNonMplsTransit)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Test flow - configuration, traffic & restore */
    prvTgfPclNonMplsTransitTest();

    prvTgfPclRestore();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngressMetadata checks Ingress Metadata[207:192] - Ethernet type
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngressMetadata)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngressMetadataTest();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysEgressMetadata checks Egress Metadata[207:196] - Original VID
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysEgressMetadata)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysEgressMetadataTest();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngressTunnelL2 checks Tunnel Header MAC_SA of
    Tunnel terminated packet.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngressTunnelL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngressTunnelL2Test();
}

/* AUTODOC: Test - MPLS Transit Tunnel:
    Passenger parsing of MPLS Transit Tunnel pacekts. IPCL fixed key fields
    (non-UDB) are based on tunnel header, while IPCL UDB key fields can be
    relative to the passenger header or the tunnel header.
*/
UTF_TEST_CASE_MAC(prvTgfPclMplsTransit)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Test flow - configuration, traffic & restore */
    prvTgfPclMplsTransitTest();

    prvTgfPclRestore();
}

/* AUTODOC: Test - Source ID mask:
    IPCL can assign source ID based on action content. this assignment can be
    masked which will cause only selected bits in the source ID will be changed.
*/
UTF_TEST_CASE_MAC(prvTgfPclSourceIdMask)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Test flow - configuration, traffic & restore */
    prvTgfPclSourceIdMaskTest();
}

/* AUTODOC: Test - EPCL Source Port Selection test:
    The EPCL keys may contain the source port field based on original or local
    value.
*/
UTF_TEST_CASE_MAC(prvTgfPclSourcePortSelect)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Test flow - configuration, traffic & restore */
    prvTgfPclSourcePortSelectTest();
}

/* AUTODOC: Test - EPCL Target Port Selection test:
    The EPCL keys may contain the source port field based on final or local
    value.
*/
UTF_TEST_CASE_MAC(prvTgfPclTargetPortSelect)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Test flow - configuration, traffic & restore */
    prvTgfPclTargetPortSelectTest();
}

/* AUTODOC: Test - Port List and group mode test:
    Port List approach enables sharing of rules between different interfaces,
    without the need for rules duplication. Here we examine as well the Port
    Grouping mode.
*/
UTF_TEST_CASE_MAC(prvTgfPclPortListGrouping)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Test flow - configuration, traffic & restore */
    prvTgfPclPortListGroupingTest();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngressTunnelL3 checks Tunnel Header DIP[15:0] of
    Tunnel terminated packet.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngressTunnelL3)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngressTunnelL3Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysEgressTunnelL2 checks Tunnel Header MAC_SA[15:0] of
    Tunnel started packet.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysEgressTunnelL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysEgressTunnelL2Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2 checks Tunnel Header SIP[31:0] of
    Tunnel started packet.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2Test();
}

/* AUTODOC: Test - PCL user defined bytes for other IP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleEgrStdNotIpUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleEgrStdNotIpUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrStdNotIpUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleEgrStdNotIpUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrStdNotIpUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleEgrStdNotIpUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for UDP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleEgrStdIpL2QosUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleEgrStdIpL2QosUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrStdIpL2QosUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleEgrStdIpL2QosUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrStdIpL2QosUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleEgrStdIpL2QosUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for UDP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleEgrStdIpv4L4Udb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleEgrStdIpv4L4UdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrStdIpv4L4UdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleEgrStdIpv4L4UdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrStdIpv4L4UdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleEgrStdIpv4L4UdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for other IP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleEgrExtNotIpv6Udb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleEgrExtNotIpv6UdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrExtNotIpv6UdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleEgrExtNotIpv6UdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrExtNotIpv6UdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleEgrExtNotIpv6UdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for IPV6 TCP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleEgrExtIpv6L2Udb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleEgrExtIpv6L2UdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrExtIpv6L2UdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleEgrExtIpv6L2UdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrExtIpv6L2UdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleEgrExtIpv6L2UdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test - PCL user defined bytes for IPV6 TCP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleEgrExtIpv6L4Udb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleEgrExtIpv6L4UdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrExtIpv6L4UdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleEgrExtIpv6L4UdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrExtIpv6L4UdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleEgrExtIpv6L4UdbConfigRestore();

    prvTgfPclRestore();
}
/* AUTODOC: Test - PCL user defined bytes for UDP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleEgrExtIpv4RaclVaclUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleEgrExtIpv4RaclVaclUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrExtIpv4RaclVaclUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleEgrExtIpv4RaclVaclUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrExtIpv4RaclVaclUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleEgrExtIpv4RaclVaclUdbConfigRestore();

    prvTgfPclRestore();
}
/* AUTODOC: Test - PCL user defined bytes for IPV6 TCP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleEgrUltraIpv6RaclVaclUdb)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Set additional configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfPclRuleEgrUltraIpv6RaclVaclUdbConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrUltraIpv6RaclVaclUdbTrafficGenerate();

    /* Set additional configuration */
    prvTgfPclRuleEgrUltraIpv6RaclVaclUdbAdditionalConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrUltraIpv6RaclVaclUdbAdditionalTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleEgrUltraIpv6RaclVaclUdbConfigRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test dedicated to check simulation
    It does sents two packets - correct (to be matched)
    and incorrect, that differs from correct only by not 6 value
    in version field of IPV6 header (to be not matced).
    Test - PCL user defined bytes for IPV6 TCP packets:
    configure VLAN, FDB entries;
    configure PCL rule and different UDBs with valid\ivalid offsets;
    send traffic to match\not match PCL rules and UDBs;
    verify traffic is dropped\not dropped according to rule actions.
*/
UTF_TEST_CASE_MAC(tgfPclRuleEgrExtIpv6L2UdbWrongIpVer)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* save execution time only */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(0xFFFFFFFF/*skipFamilyBmp*/);

    /* Set configuration */
    prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerConfigSet();

    /* Generate traffic */
    prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerTrafficGenerate();

    /* Restore configuration */
    prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerConfigRestore();

    prvTgfPclRestore();
}

/*
    AUTODOC: prvTgfPclUdb10KeyCustomer checks several fields
    using Ingress UDB10 key.
    Fields:
    pclId  - replacedFld.pclId   - muxed with UDB0-1
    Evlan  - replacedFld.vid     - muxed with UDB2-3
    Eport  - replacedFld.srcPort - muxed with UDB4-5
    VID1   - replacedFld.vid1    - muxed with UDB6-7
    UP1    - replacedFld.up1     - muxed with UDB7
    UP0    - UDB8 L2 + 14,  mask 0xE0, patten - (up0 << 5)
    DSCP   - UDB9 L3Minus2 + 3,  mask 0xFC, patten - (dscp << 2)
    UDB mapping 30-1 => 8-9
*/
UTF_TEST_CASE_MAC(prvTgfPclUdb10KeyCustomer)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdb10KeyCustomerTest();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4
    Test on Ingress PCL UDB10 L4 offset type on IPV4 TCP packet.
    Fields: 10 first bytes from TCP header beginning.
    UDB mapping 40-49 => L4+0 - L4+9
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4
    Test on Ingress PCL UDB10 L4 offset type on IPV4 UDP packet.
    Fields: 10 first bytes from UDP header beginning.
    UDB mapping 40-49 => L4+0 - L4+9
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4
    Test on Ingress PCL UDB20 L4 offset type on IPV6 UDP packet.
    Fields: 20 first bytes from TCP header beginning.
    UDB mapping 39-20 => L4+0 - L4+19
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2
    Test on Ingress PCL UDB30 MplsMinus2 offset type on Ethernet over MPLS packet.
    Fields: 20 first bytes from MPLS Ethernet Type beginning.
    UDB mapping 20-39 => MplsMinus2+0 - MplsMinus2+19
    MPLS offset supported by HW only up to 20 bytes
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2
    Test on Ingress PCL UDB40 L2 offset type on Ethernet Other packet.
    Fields: 40 first bytes from L2 header beginning.
    UDB mapping 10-49 => L2+0 - L2+39
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2
    Test on Ingress PCL UDB50 L3Minus2 offset type on IPV6 Other packet.
    Fields: 50 first bytes from IPV6 Ethernet Type beginning.
    UDB mapping 49-0 => L3Minus2+0 - L3Minus2+49
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2
    Test on Ingress PCL UDB60 L3Minus2 offset type on IPV4 Other packet.
    Fields: 50 first bytes from IPV4 Ethernet Type beginning.
    UDB mapping 0-49 => L3Minus2+0 - L3Minus2+49
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2
    Test on Ingress PCL UDB60 L3Minus2 offset type on IPV4 Other packet.
    Fields: 60 first bytes from IPV4 Ethernet Type beginning.
    UDB mapping 0-59 => L3Minus2+0 - L3Minus2+59
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2Test();
}


/*
    AUTODOC: prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4
    Test on Egress PCL UDB10 L4 offset type on IPV4 TCP packet.
    Fields: 10 first bytes from TCP header beginning.
    UDB mapping 40-49 => L4+0 - L4+9
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4
    Test on Egress PCL UDB20 L4 offset type on IPV6 UDP packet.
    Fields: 20 first bytes from UDP header beginning.
    UDB mapping 39-20 => L4+0 - L4+19
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2
    Test on Egress PCL UDB30 MplsMinus2 offset type on ETH over MPLS packet.
    Fields: 20 first bytes from MPLS Ethernet Type beginning.
    UDB mapping 20-39 => MplsMinus2+0 - MplsMinus2+19
    MPLS offset supported by HW only up to 20 bytes
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2
    Test on Egress PCL UDB20 L2 offset type on ETH Other packet.
    Fields: 40 first bytes from L2 Header beginning.
    UDB mapping 10-49 => L2+0 - L2+39
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysEgrEgrUdb50Ipv6OtherL3Minus2
    Test on Egress PCL UDB50 L3Minus2 offset type on IPV6 Other packet.
    Fields: 50 first bytes from IPV3 Ethernet Type beginning.
    UDB mapping 49-0 => L3Minus2+0 - L3Minus2+49
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysEgrUdb50Ipv6OtherL3Minus2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysEgrUdb50Ipv6OtherL3Minus2Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2
    Test on Egress PCL UDB60 L3Minus2 offset type on IPV4 Other packet.
    Fields: 50 first bytes from IPV4 Ethernet Type beginning.
    UDB mapping 0-49 => L3Minus2+0 - L3Minus2+49
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2
    Test on Ingress PCL UDB30 L3Minus2 offset type on Ethernet over MPLS packet.
    Fields: 30 first bytes from MPLS Ethernet Type beginning.
    UDB mapping 20-49 => L3Minus2+0 - L3Minus2+29
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2Test();
}

/*
    AUTODOC: prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2
    Test on Egress PCL UDB30 L3Minus2 offset type on ETH over MPLS packet.
    Fields: 30 first bytes from MPLS Ethernet Type beginning.
    UDB mapping 20-49 => L3Minus2+0 - L3Minus2+29
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2Test();
}

/*
    AUTODOC: prvTgfPclTrunkEportRedirectCustomer
    Test replacement of trunk id and srcPort in FDB table
    Create PCL rule to change trunk id and srcPort.
    Send traffic and after learning check replaced values
    Invalidate PCL rule
    Send traffic and after learning check values are not replaced
*/
UTF_TEST_CASE_MAC(prvTgfPclTrunkEportRedirectCustomer)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclTrunkEportRedirectCustomerTest();
}

/*
    AUTODOC: prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4WithL2
    Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Ipv4 With Passenger L2
    UDBs for anchor L3Minus2 offset 0-3.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4WithL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4WithL2Test();
}

/*
    AUTODOC: prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4NoL2
    Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Ipv4 Without Passenger L2
    UDBs for anchor L3Minus2 offset 0-3.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4NoL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4NoL2Test();
}

/*
    AUTODOC: prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsWithL2
    Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Mpls With Passenger L2
    UDBs for anchor L3Minus2 offset 0-3.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsWithL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsWithL2Test();
}

/*
    AUTODOC: prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsNoL2
    Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Mpls Without Passenger L2
    UDBs for anchor L3Minus2 offset 0-3.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsNoL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsNoL2Test();
}

/*
    AUTODOC: prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4WithL2
    Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Ipv4 With Passenger L2
    UDBs for anchor L3Minus2 offset 0-3.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4WithL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4WithL2Test();
}

/*
    AUTODOC: prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4NoL2
    Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Ipv4 Without Passenger L2
    UDBs for anchor L3Minus2 offset 0-3.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4NoL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4NoL2Test();
}

/*
    AUTODOC: prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsWithL2
    Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Mpls With Passenger L2
    UDBs for anchor L3Minus2 offset 0-3.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsWithL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsWithL2Test();
}

/*
    AUTODOC: prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsNoL2
    Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Mpls Without Passenger L2
    UDBs for anchor L3Minus2 offset 0-3.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsNoL2)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsNoL2Test();
}

/*----------------------------------------------------------------------------*/
/*  AUTODOC: prvTgfPclUdbL3Minus2IngressLLCNonSnap
    Test on IPCL L3Minus2 anchor for LLC Non SNAP packets
    UDBs for anchor L3Minus2 offset 0-3.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbL3Minus2IngressLlcNonSnap)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbL3Minus2IngressLlcNonSnapTest();
}

/*----------------------------------------------------------------------------*/
/*  AUTODOC: prvTgfPclUdbL3Minus2EgressLlcNonSnap
    Test on EPCL L3Minus2 anchor for LLC Non SNAP packets
    UDBs for anchor L3Minus2 offset 0-3.
*/
UTF_TEST_CASE_MAC(prvTgfPclUdbL3Minus2EgressLlcNonSnap)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclUdbOnlyKeysEgrUdb10LlcNonSnapTest();
}

/*----------------------------------------------------------------------------*/
/*  AUTODOC: prvTgfPclEVlanBindingMode
    Test on IPCL eVLAN based binding mode
    for Lookup 0,1,2.
*/
UTF_TEST_CASE_MAC(prvTgfPclEVlanBindingMode)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclEVlanBindingModeTest();
}

/*----------------------------------------------------------------------------*/
/*  AUTODOC: prvTgfPclEPortBindingMode
    Test on IPCL ePort based binding mode
    for Lookup 0,1,2.
*/
UTF_TEST_CASE_MAC(prvTgfPclEPortBindingMode)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclEPortBindingModeTest();
}

/*----------------------------------------------------------------------------*/
/*  AUTODOC: prvTgfPclPhyPortBindingMode
    Test on IPCL physical port based binding mode
    eVLAN updated by first lookup, then eVLAN used for second lookup.
*/
UTF_TEST_CASE_MAC(prvTgfPclPhyPortBindingMode)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclPhyPortBindingModeTest();
}

/*  AUTODOC: prvTgfEgrPclEvlanBindingModeTest
    Test on eVLAN-based binding mode for Egress PCL Configuration Table
    Entry Selection.*/
UTF_TEST_CASE_MAC(prvTgfEgrPclEvlanBindingMode)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfEgrPclEvlanBindingModeTest();
}

/*  AUTODOC: prvTgfEgrPclPortBindingModeTest
    Test on ePort-based binding mode for Egress PCL Configuration Table
    Entry Selection.*/
UTF_TEST_CASE_MAC(prvTgfEgrPclEportBindingMode)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfEgrPclEportBindingModeTest();
}

/*  AUTODOC: prvTgfPclMetadataPacketTypeTest
    Test IPCL Metadata fields Ingress UDB Packet Type and
    Applicable Flow Sub-template for different packet types.*/
UTF_TEST_CASE_MAC(prvTgfPclMetadataPacketType)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    PRV_TGF_SKIP_PASS_OK_LONG_TEST_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);

    prvTgfPclMetadataPacketTypeTest();
}

/*  AUTODOC: prvTgfPclMetadataPacketTypeIpOverMplsTest
    Test IPCL Metadata fields Ingress UDB Packet Type and
    Applicable Flow Sub-template for IP Over MPLS packet types.
    The L3 L4 parsing is enabled. */
UTF_TEST_CASE_MAC(prvTgfPclMetadataPacketTypeIpOverMpls)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfPclMetadataPacketTypeIpOverMplsTest();
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfPclVplsVirtSrcPortAssign
*/

UTF_TEST_CASE_MAC(prvTgfPclVplsVirtSrcPortAssign)
{
    if (GT_FALSE == prvUtfIsVplsModeUsed(prvTgfDevNum))
    {
        /* set skip flag and update skip tests num */                          \
        prvUtfSkipTestsSet();                                                  \
        return;
    }

    /* Test of virtual source port assignment */
    prvTgfPclVplsVirtSrcPortAssign();
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfPclVplsUnknownSaCmd
*/

UTF_TEST_CASE_MAC(prvTgfPclVplsUnknownSaCmd)
{
    if (GT_FALSE == prvUtfIsVplsModeUsed(prvTgfDevNum))
    {
        /* set skip flag and update skip tests num */                          \
        prvUtfSkipTestsSet();                                                  \
        return;
    }

    /* Test of virtual source port assignment */
    prvTgfPclVplsUnknownSaCmd();
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfPclVplsVirtSrcPortAssignAndAcEnable
*/

UTF_TEST_CASE_MAC(prvTgfPclVplsVirtSrcPortAssignAndAcEnable)
{
    if (GT_FALSE == prvUtfIsVplsModeUsed(prvTgfDevNum))
    {
        /* set skip flag and update skip tests num */                          \
        prvUtfSkipTestsSet();                                                  \
        return;
    }

    /* Test of virtual source port assignment */
    prvTgfPclVplsVirtSrcPortAssignAndAcEnable();
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfPclVplsMeteringToFloodedOnlyEnable
*/

UTF_TEST_CASE_MAC(prvTgfPclVplsMeteringToFloodedOnlyEnable)
{
    if (GT_FALSE == prvUtfIsVplsModeUsed(prvTgfDevNum))
    {
        /* set skip flag and update skip tests num */                          \
        prvUtfSkipTestsSet();                                                  \
        return;
    }

    /* Test of virtual source port assignment */
    prvTgfPclVplsMeteringToFloodedOnlyEnable();
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfPclVplsBillingToFloodedOnlyEnable
*/

UTF_TEST_CASE_MAC(prvTgfPclVplsBillingToFloodedOnlyEnable)
{
    if (GT_FALSE == prvUtfIsVplsModeUsed(prvTgfDevNum))
    {
        /* set skip flag and update skip tests num */                          \
        prvUtfSkipTestsSet();                                                  \
        return;
    }

    /* Test of virtual source port assignment */
    prvTgfPclVplsBillingToFloodedOnlyEnable();
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfPclVplsVirtualSrcPortMeshIdAssign
*/

UTF_TEST_CASE_MAC(prvTgfPclVplsVirtualSrcPortMeshIdAssign)
{
    if (GT_FALSE == prvUtfIsVplsModeUsed(prvTgfDevNum))
    {
        /* set skip flag and update skip tests num */                          \
        prvUtfSkipTestsSet();                                                  \
        return;
    }

    /* Test of virtual source port assignment */
    prvTgfPclVplsVirtualSrcPortMeshIdAssign();
}

/*----------------------------------------------------------------------------*/
/*
    tgfPclEgressTrapMirror
    configure VLAN, FDB entries;
    configure PCL rules - to trap ;
    send traffic ;
    verify traffic is trapped\forwarded.
*/

UTF_TEST_CASE_MAC(tgfPclEgressTrapMirror)
{
    /*
        1. Set configuration
        2. Set PCL configuration
        3. Generate traffic
        4. Restore configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* Set PCl configuration */
    prvTgfPclEgressTrapMirrorCfgSet();

    /* Generate traffic */
    prvTgfPclEgressTrapMirrorTrafficGenerate();

    /* Restore configuration */
    prvTgfPclEgressTrapMirrorCfgRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Test tgfPclEgressMirroring
    configure VLAN, FDB entries;
    configure PCL rules - to forward, configure Egress mirroring ;
    send traffic ;
    verify traffic is sent from analyzer.
*/

UTF_TEST_CASE_MAC(tgfPclEgressMirroring)
{
    /*
        1. Set configuration
        2. Set PCL configuration
        3. Generate traffic
        4. Restore configuration
    */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* Set PCl configuration */
    prvTgfPclEgressMirroringCfgSet();

    /* Generate traffic */
    prvTgfPclEgressMirroringTrafficGenerate();

    /* Restore configuration */
    prvTgfPclEgressMirroringCfgRestore();
}

/*
 * Configuration of tgfPcl suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(tgfPcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIngressPclKeyStdL2)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclValidInvalidRulesBasicExtended)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclValidInvalidRulesActionUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclSecondLookupMatch)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclBothUserDefinedBytes)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclUserDefinedBytes)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclIpv4TcpUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclIpv4UdpUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclIpv4FragmentUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclIpv4OtherUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclEthernetOtherUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclMplsUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclUdeUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclIpv6Udb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclIpv6TcpUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclEthernetOtherVridUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclIpv4UdpQosUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclUdbFldsIsIp)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclUdbFldsPacketDepend)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclUdbFldsNotPacketDepend)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclUdbFldsMacToMe)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclUdbFldsComplicated)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclUdbFldsStd)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclUdbFldsExt)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclBypassBridge)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclBypassIngressPipe)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclTcpRstFinMirroring)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclPolicerCounting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclPolicerMetering)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclPortGroupPolicerCounting)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclPortGroupPolicerMetering)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclFullLookupControl)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPclFullLookupControl_qosMappingTableIterator)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclPolicerCounting_qosMappingTableIterator)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclPolicerMetering_qosMappingTableIterator)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclPolicerCountingFlowIdBased)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclPolicerCountingHierarchical)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclPolicerDp2CfiMap)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclPortGroupPolicerCounting_qosMappingTableIterator)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclPortGroupPolicerMetering_qosMappingTableIterator)


    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRedirectWithMacDaModify)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRedirectWithMacSaModify)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclOverrideUserDefinedBytesByTrunkHashSet)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdeEtherTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEgressKeyFieldsVidUpModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEgrExtIpv4RaclVaclSipDipTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEgrUltraIpv6RaclVaclSipDipTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclLookup00And01)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclPortListMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclWriteRuleInvalid)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclFourLookups)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclRedirectToLTT)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclLion2VidAndExt)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclPortListCrossIpclTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclPortListCrossEpclTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEportEvlanIpclTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEportEvlanEpclTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEvlanMatchIpclTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEvlanMatchEpclTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEArchRedirectEPort)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEArchRedirectEVid)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEArchRedirectEVidx)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclTag1MatchIpclTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclTag1MatchEpclTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclSrcTrunkIdMatchIpcl)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclLion2UdbOverrideIpclQosProfile)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclLion2UdbOverrideIpclTag1)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclFwsCnc)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclFwsMove)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclFwsMove_virtTcam)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclFwsMove_virtTcam_80B)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclFwsMove_virtTcamPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclPumaPortGroupTestUnaware)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclPumaPortGroupTestAware)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclBc2SetMac2me)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclBc2SetMacSa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclLookup0ForRoutedPackets)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEArchSourcePort)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclIngressQuadLookup)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclIngressQuadLookup_virtTcam)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclIngressQuadLookup_virtTcamPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclIngressQuadLookupWithInvalid)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclBc2MapMplsChannelToOamOpcode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEgressQuadLookup)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEgressQuadLookupWithInvalid)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleStdNotIpUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleStdIpL2QosUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleStdIpv4L4Udb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleStdIpv6DipUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleExtNotIpv6Udb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleExtIpv6L2Udb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleExtIpv6L4Udb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleIngrStdUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleIngrExtUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleStdIpv4RoutedAclQosUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleExtIpv4PortVlanQosUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleUltraIpv6PortVlanQosUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleUltraIpv6RoutedAclQosUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleEgrStdNotIpUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleEgrStdIpL2QosUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleEgrStdIpv4L4Udb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleEgrExtNotIpv6Udb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleEgrExtIpv6L2Udb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleEgrExtIpv6L4Udb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleEgrExtIpv4RaclVaclUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleEgrUltraIpv6RaclVaclUdb)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclRuleEgrExtIpv6L2UdbWrongIpVer)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbMetadataSrcEport)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclNonMplsTransit)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclMplsTransit)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclSourceIdMask)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclSourcePortSelect)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclTargetPortSelect)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclPortListGrouping)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngressMetadata)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysEgressMetadata)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngressTunnelL2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngressTunnelL3)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysEgressTunnelL2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdb10KeyCustomer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysEgrUdb50Ipv6OtherL3Minus2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclTrunkEportRedirectCustomer)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4WithL2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4NoL2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsWithL2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsNoL2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4WithL2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4NoL2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsWithL2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsNoL2)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbL3Minus2IngressLlcNonSnap)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclUdbL3Minus2EgressLlcNonSnap)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEVlanBindingMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclEPortBindingMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclPhyPortBindingMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgrPclEvlanBindingMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgrPclEportBindingMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclMetadataPacketType)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclMetadataPacketTypeIpOverMpls)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclVplsVirtSrcPortAssign)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclVplsUnknownSaCmd)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclVplsVirtSrcPortAssignAndAcEnable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclVplsMeteringToFloodedOnlyEnable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclVplsBillingToFloodedOnlyEnable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPclVplsVirtualSrcPortMeshIdAssign)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPclEgressTrapMirror)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPclEgressMirroring)
UTF_SUIT_END_TESTS_MAC(tgfPcl)


