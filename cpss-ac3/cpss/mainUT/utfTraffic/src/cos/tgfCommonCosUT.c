/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfCommonVntUT.c
*
* DESCRIPTION:
*       Enhanced UTs for CPSS Vnt
*
* FILE REVISION NUMBER:
*       $Revision: 15 $
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <cos/prvTgfCos.h>
#include <common/tgfCosGen.h>
#include <common/tgfCommon.h>
#include <cos/tgfCosEgressQoSdscpUpRemapping.h>
#include <cos/tgfCosSettingEgressCfi.h>
#include <cos/tgfCosTcDpRemapping.h>
#include <cos/tgfCosTrustQosMappingTableSelect.h>


/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

void prvTgfCaptureTpidWithZeroBmpDisable(void);
void prvTgfCaptureTpidWithZeroBmpRestore(void);



/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Cos Trust EXP mode:
    configure VLAN, FDB entries;
    configure 2 Cos profiles for specific TC;
    map EXP to Cos profiles;
    enable\disable trust EXP mode for MPLS packets;
    send MPLS traffic;
    verify egress counters.
*/
UTF_TEST_CASE_MAC(tgfCosTrustExpModeSet)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfCosTestCommonConfigure();

    /* Generate traffic */
    prvTgfCosExpTestTrafficGenerateAndCheck();

    /* Restore configuration */
    prvTgfCosTestCommonReset();
}

UTF_TEST_CASE_MAC(tgfCosTrustExpModeSet_qosMappingTableIterator)
{
    /* test with qos mapping table iterator  */
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfCosTrustExpModeSet));
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Cos Trust EXP mode:
    configure VLAN, FDB entries;
    configure 2 Cos profiles for specific TC;
    configure Qos L2 Trust Mode;
    select VLAN TAG0\TAG1 type for Trust L2 Qos mode;
    send Ethernet traffic for each modes;
    verify egress counters.
*/
UTF_TEST_CASE_MAC(tgfCosL2TrustModeVlanTagSelectSet)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfCosTestCommonConfigure();

    /* Generate traffic */
    prvTgfCosVlanTagSelectTestTrafficGenerateAndCheck();

    /* Restore configuration */
    prvTgfCosTestCommonReset();
}

UTF_TEST_CASE_MAC(tgfCosL2TrustModeVlanTagSelectSet_qosMappingTableIterator)
{
    /* test with qos mapping table iterator  */
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfCosL2TrustModeVlanTagSelectSet));
}

UTF_TEST_CASE_MAC(tgfCosMappingTableIndex)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                    UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E );
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfCosMappingTableIndexTest();
}

/*
    test the 'port protocol' qos parameters assignment.
*/
UTF_TEST_CASE_MAC(tgfCosPortProtocol)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfCosPortProtocolTest();
}


static void egressInterfaceBasedQosRemapping(IN GT_U32 testNumber)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration
    4. Generate traffic
    5. Restore configuration
*/

    /* Build Packet */
    prvTgfCosEgressQoSdscpUpRemappingIpv4BuildPacket();
    /* Set Bridge configuration*/
    prvTgfCosEgressQoSdscpUpRemappingBridgeConfigSet();
    /* Set QoS configuration */
    prvTgfCosEgressQoSdscpUpRemappingEgrRemarkingConfigurationSet(testNumber);

    /* Generate traffic */
    prvTgfCosEgressQoSdscpUpRemappingTrafficGenerate(testNumber);

    /* Restore configuration */
    prvTgfCosEgressQoSdscpUpRemappingConfigurationRestore(testNumber);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Egress QoS Interface based remapping
    configure VLAN with 4 ports;
    configure default eport settings:
    trustUp=1,trustL2QoS = Tag1, trustQoSMappingTableSelect = 1,
    UpToQoSProfileMappingTable1 maps PRI 5 to QoS Profile 1, modifyUp = 1, modifyDscp = 1;
    configure QoS profile 1 with TC3;
    configure egress eports attributes:
    enableTcDpMapping =1, QoSTableMappingIndex =1,EnableEgresUpMapping = 1,(if Up marking)
    enableEgressDscpMapping = 1 (if Dscp marking)
    configure egress remarking:
    QoS Mapping Table 1 maps (TC=3,DP =1) to Up = 4, DSCP = 10.
    verify tunneled traffic on nextHop port.
*/

UTF_TEST_CASE_MAC(tgfCosEgressQosTcDpRemapping)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    egressInterfaceBasedQosRemapping(1);/* test number 1 : tcdp remapping */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Egress QoS Interface based remapping
    configure VLAN with 4 ports;
    configure default eport settings:
    trustUp=1,trustL2QoS = Tag1, trustQoSMappingTableSelect = 1,
    UpToQoSProfileMappingTable1 maps PRI 5 to QoS Profile 1, modifyUp = 1, modifyDscp = 1;
    configure QoS profile 1 with TC3;
    configure egress eports attributes:
    enableTcDpMapping =1, QoSTableMappingIndex =1,EnableEgresUpMapping = 1,(if Up marking)
    enableEgressDscpMapping = 1 (if Dscp marking)
    configure egress remarking:
    QoS Mapping Table 1 maps (TC=3,DP =1) to Up = 4, DSCP = 10.
    verify tunneled traffic on nextHop port.
*/

UTF_TEST_CASE_MAC(tgfCosEgressQosUpDscpRemapping)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    egressInterfaceBasedQosRemapping(2);/* test number 2 : up->up, dsp->dscp remapping */
}


static void egressTagCfiBitSet(IN GT_U32 testNumber, IN GT_U32 vlanTagType)
{
/*
    1. Set Base configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_BOOL tunnelIsTagged = GT_FALSE;
    /* Build Packet */
    prvTgfCosSettingEgressTagCfiFieldBuildPacket(testNumber);
    /* Set Bridge configuration*/
    prvTgfCosSettingEgressTagCfiFieldBridgeConfigSet(testNumber,vlanTagType);

    if ((testNumber >= 3) &&
        (testNumber != 5))
    {
        if (testNumber == 3)
        {
            tunnelIsTagged = GT_TRUE;
        }
        else
        {
            tunnelIsTagged = GT_FALSE;
        }
        /* Set Tunnel configuration */
        prvTgfCosSettingEgressTagCfiFieldTunnelConfigSet(testNumber,tunnelIsTagged);

        /* set PCL configuration */
         prvTgfCosSettingEgressTagCfiFieldPclConfigSet();

    }
    if ((testNumber == 2) || (testNumber == 3))
    {
        prvTgfCosSettingEgressTagCfiFieldDpToCfiMappingSet();
    }
    /* Generate traffic */
    prvTgfCosSettingEgressTagCfiFieldTrafficGenerate(testNumber,vlanTagType);
    /* Restore configuration */
    prvTgfCosSettingEgressTagCfiFieldConfigurationRestore(testNumber,vlanTagType);
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Setting Egress Packet Tag0 CFI Field
    configure VLAN 5 with 4 ports;
    TPID is set to recognise packet as TAG0 one;
    CFI relay bit is enabled;
    build ipv4 packet to send
    Generate traffic and verify that cfi bit in output
    packet is the same as in sent packet.
    Disable CFI relay bit.
    Generate traffic (CFI bit is 1) and verify that cfi bit in output
    packet is not same as in sent packet.
*/

UTF_TEST_CASE_MAC(tgfCosSettingEgressTag0CfiField)
{
/*
    1. Set Base configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    egressTagCfiBitSet(1,0);/* test number 1 : cfi in tag0, no dp to cfi mapping */
}

/* AUTODOC: Test Setting Egress Packet Tag0 CFI Field
            configure VLAN 5 with 4 ports;
            TPID is set to recognise packet as TAG0 one;
            build ipv4 packet to send;
            enable dp to cfi configuration for egress eport;
            set dp to cfi mapping;
            Generate traffic (cfi is 0) and verify that cfi bit in output
            packet is 1 in accordance with mapping.
*/
UTF_TEST_CASE_MAC(tgfCosSettingEgressTag0DpCfiMapping)
{
/*
    1. Set Base configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    egressTagCfiBitSet(2,0);/* test number 2 : dp to cfi mapping is enable*/
}

/* AUTODOC: Test Setting Egress Packet Tag1 CFI Field
            configure VLAN 5 with 4 ports;
            Set TPID to recognise packet as TAG1 one;
            build ipv4 packet to send
            Generate traffic (CFI bit is 1 and verify that cfi bit in output
            packet is the same as in sent packet.
            Generate traffic (CFI bit is 0) and verify that cfi bit in output
            packet is the same as in sent packet.
*/

UTF_TEST_CASE_MAC(tgfCosSettingEgressTag1CfiField)
{
/*
    1. Set Base configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    prvTgfCaptureTpidWithZeroBmpDisable();
    egressTagCfiBitSet(1,1);/* test number 1 : cfi in tag0, no dp to cfi mapping */
    prvTgfCaptureTpidWithZeroBmpRestore();
}

/* AUTODOC: Test Setting Egress Packet Tag1 CFI Field
            configure VLAN 5 with 4 ports;
            Set TPID to recognise packet as TAG1 one;
            build ipv4 packet to send;
            enable dp to cfi configuration for egress eport;
            set dp to cfi mapping;
            Generate traffic (cfi is 0) and verify that cfi bit in output
            packet is 1 in accordance with mapping.
*/

UTF_TEST_CASE_MAC(tgfCosSettingEgressTag1DpCfiMapping)
{
/*
    1. Set Base configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    prvTgfCaptureTpidWithZeroBmpDisable();
    egressTagCfiBitSet(2,1);/* test number 2 : dp to cfi mapping is enable*/
    prvTgfCaptureTpidWithZeroBmpRestore();
}
/* AUTODOC: 1. Set Ethenet packet to send
            2. Set Bridge configuration
            3. Set Tunnel configuration (ehernet over ipv4, tunnel header with Vlan tag)
            4. Set PCL configuration (mac DA rule and tunnel start action)
            5. Set DP to CFI configuration
            6. Generate traffic
            7. Verify that cfi was changed on outer vlan in TS header
               and tunnel L2 is it was configured in TS
            8. Restore configuration

*/

UTF_TEST_CASE_MAC(tgfCosSettingEgressOuterTagDpCfiMapping)
{
/*
    1. Set Ethenet packet to send
    2. Set Bridge configuration
    3. Set Tunnel configuration (ehernet over ipv4, tunnel header with Vlan tag)
    4. Set PCL configuration (mac DA rule and tunnel start action)
    5. Set DP to CFI configuration
    6. Generate traffic
    7. Verify that cfi was changed on outer vlan in TS header
       and tunnel L2 is it was configured in TS
    8. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    egressTagCfiBitSet(3,0);/* test number 3 : dp to cfi mapping is enable*/
}

/* AUTODOC: 1. Set Ethenet packet to send
            2. Set Bridge configuration
            3. Set Tunnel configuration (ehernet over ipv4, tunnel header is untagged)
            4. Set PCL configuration (mac DA rule and tunnel start action)
            5. Set DP to CFI configuration
            6. Generate traffic
            7. Verify that cfi was not changed any passenger vlan
               and tunnel L2 is it was configured in TS
            8. Restore configuration

*/

UTF_TEST_CASE_MAC(tgfCosSettingEgressOuterUntagDpCfiMapping)
{
/*
    1. Set Ethenet packet to send
    2. Set Bridge configuration
    3. Set Tunnel configuration (ehernet over ipv4, tunnel header is untagged)
    4. Set PCL configuration (mac DA rule and tunnel start action)
    5. Set DP to CFI configuration
    6. Generate traffic
    7. Verify that cfi was not changed any passenger vlan
       and tunnel L2 is it was configured in TS
    8. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    egressTagCfiBitSet(4,0);/* test number 4 : dp to cfi mapping is enable*/
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Setting Egress Double tagged Packet Tag0 DEI Field
    configure VLAN 5 with 4 ports;
    TPID is set to recognise packet as TAG0 one;
    DEI relay bit is enabled;
    build ipv4 packet to send
    Generate traffic and verify that DEI bit in output
    packet is the same as in sent packet.
    Disable DEI relay bit.
    Generate traffic (DEI bit is 1) and verify that DEI bit in output
    packet is not same as in sent packet.
*/

UTF_TEST_CASE_MAC(tgfCosSettingEgressTag0DeiField)
{
/*
    1. Set Base configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    egressTagCfiBitSet(5,0);/* test number 5: DEI in tag0, tag1 present, no dp to cfi mapping */
}



static void tcDpRemappingOnStackPort(IN GT_U32 testNumber)
{
/*
    1. Set Base configuration
    4. Generate traffic
    5. Restore configuration
*/
    if (testNumber == 1)
    {
        /* Build Packet */
        prvTgfTcDpRemappingOnStackPortBuildPacket();
    }
    /* Set Bridge configuration*/
    prvTgfCosTcDpRemappingOnStackPortBridgeConfigSet();
    /* Set QoS configuration*/
    prvTgfCosTcDpRemappingOnStackPortQoSConfigSet(testNumber);
    /* Set Cascade configuration*/
    prvTgfCosTcDpRemappingOnStackPortCscdConfigSet(testNumber);
    if (testNumber == 2)
    {
        prvTgfCosTcDpRemappingOnStackPortDsaTagSet();
    }
    /* Set tc and dp remapping configuration */
    prvTgfCosTcDpRemappingOnStackPortTcDpRemappingConfigSet(testNumber);
    /* Generate traffic */
    prvTgfCosTcDpRemappingOnStackPortTrafficGenerate(testNumber);
    /* Restore configuration */
    prvTgfCosTcDpRemappingOnStackPortConfigurationRestore(testNumber);
}




/* AUTODOC: 1. Configure vlan5 with ports 0,18,36,54
            2. Set QoS trust mode in No_Trust. Set qos profile 7 for ingress port.
               Set qos profile entry with tc =3, dp =0.
            3. Set a physical port 54 to be a cascade port for egress direction.
            4. Enable physical port 54 for tc and dp remapping.
               Remap {tc = 3, dp =0}->{tc =6, dp =0}
            5. Generate traffic
            7. Verify that outgoing traffic used priority queue 6
            8. Restore configuration

*/
UTF_TEST_CASE_MAC(tgfCosTcDpRemappingOnStackPorts)
{
/*
    1. Set Ethenet packet to send
    2. Set Bridge configuration
    3. Set QoS configuration
    4. Set cascade configuration
    5. Set TC and DP remapping configuration
    6. Generate traffic
    7. Verify that counter tuned on TC6 is counted 1 packet
    8. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(prvTgfDevNum) &&
        (GT_TRUE == prvUtfIsTrafficManagerUsed()))
    {
        /* test use egress port managed by TM.
           Test is not relevant for such systems. */
        SKIP_TEST_MAC
    }

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   PRV_TGF_EGR_PORT_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    tcDpRemappingOnStackPort(1);/* test number 1 : tcdp remapping for network port*/
}

/* AUTODOC: 1. Configure vlan5 with ports 0,18,36,54
            2. Set QoS trust mode in No_Trust. Set trust DSA tag QoS mode on send port.
            3. Set a physical port 54 to be a cascade port for egress direction.
               Set a physical port 0 as cascade one for ingress direction.
               Set extended DSA tag qos mode for cascade port 0 - retrieve TC and DP from eDsa tag.
            4. Configure incoming eDsa tag. Set lookup mode for accessing the Device Map table.
               Set the device map table to know where to send traffic.
            4. Enable physical egress port 54 for tc and dp remapping (set as stack port).
               Enable physical ingress port 0 for tc and dp remapping (set as stack port):
               ingress stack port whose traffic is going to be remapped on egress stack port 54.
               Remap {tc = 3, dp =0}->{tc =6, dp =0}
            5. Generate traffic
            7. Verify that outgoing traffic used priority queue 6
            8. Restore configuration

*/
UTF_TEST_CASE_MAC(tgfCosTcDpRemappingOfStackPortTrafficOnStackPorts)
{
/*
    1. Set Ethenet packet to send
    2. Set Bridge configuration
    3. Set QoS configuration
    4. Set cascade configuration
    5. Set TC and DP remapping configuration
    6. Generate traffic
    7. Verify that counter tuned on TC6 is counted 1 packet
    8. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(prvTgfDevNum) &&
        (GT_TRUE == prvUtfIsTrafficManagerUsed()))
    {
        /* test use egress port managed by TM.
           Test is not relevant for such systems. */
        SKIP_TEST_MAC
    }

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   PRV_TGF_SEND_PORT_IDX_CNS,
                                   PRV_TGF_EGR_PORT_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    tcDpRemappingOnStackPort(2);/* test number 2 : tcdp remapping for cascade port*/
}

/* AUTODOC: 1. Build ipv4 packet with vid=5, up=6
            2. Configure VLAN 5 with ports [0, 8], VLAN 6 with ports [18, 23]

            First Test: Select table with mappingTableIndex
            3. Configure default eport settings:
               trustL2QoS = Tag0, useUpAsIndex=FALSE
               map UP 6 and MappingTable 8 to QoS Profile 10
            4. Configure QoS profile 10 with modifyUp disabled
            5. Configure TTI Action with: trustUp=TRUE, MappingTable 8,
               useUpAsTableIndex disabled

            Second Test: Use UP to select a table
            3. Configure default eport settings:
               trustL2QoS = Tag0, useUpAsIndex=TRUE
               map UP 6 and MappingTable 6 to QoS Profile 10
            4. Configure QoS profile 10 with:
               modifyUp enabled, UP = 7
            5. Configure TTI Action with: trustUp = TRUE, MappingTable 8,
               useUpAsTableIndex enabled, keepPreviousQos disabled

            6. Verify tunneled traffic on egress port.
*/
UTF_TEST_CASE_MAC(tgfCosTrustQosMappingTableSelect)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration
    4. Generate traffic
    5. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Build ipv4 packet with vid=5, up=6 */
    tgfCosTrustQosMappingTableSelectBuildPacket();

    /* Configure VLAN 5 with ports [0, 8], VLAN 6 with ports [18, 23] */
    tgfCosTrustQosMappingTableSelectBridgeConfigSet();

    /* Set TTI Action, QoS configuration */
    tgfCosTrustQosMappingTableSelectCommonConfigurationSet();

    /* Set configuration for test one */
    tgfCosTrustQosMappingTableSelectCaseConfigurationSet(0);

    /* Generate traffic */
    tgfCosTrustQosMappingTableSelectTrafficGenerate();

    /* Set configuration for test two */
    tgfCosTrustQosMappingTableSelectCaseConfigurationSet(1);

    /* Generate traffic */
    tgfCosTrustQosMappingTableSelectTrafficGenerate();

    /* Restore configuration */
    tgfCosTrustQosMappingTableSelectConfigurationRestore();
}

/*
 * Configuration of tgfCos suit
*/
UTF_SUIT_BEGIN_TESTS_MAC(tgfCos)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosTrustExpModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosL2TrustModeVlanTagSelectSet)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosTrustExpModeSet_qosMappingTableIterator)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosL2TrustModeVlanTagSelectSet_qosMappingTableIterator)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosMappingTableIndex)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosPortProtocol)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosEgressQosTcDpRemapping)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosEgressQosUpDscpRemapping)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosSettingEgressTag0CfiField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosSettingEgressTag0DpCfiMapping)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosSettingEgressTag1CfiField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosSettingEgressTag1DpCfiMapping)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosSettingEgressOuterTagDpCfiMapping)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosSettingEgressOuterUntagDpCfiMapping)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosSettingEgressTag0DeiField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosTcDpRemappingOnStackPorts)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosTcDpRemappingOfStackPortTrafficOnStackPorts)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCosTrustQosMappingTableSelect)
UTF_SUIT_END_TESTS_MAC(tgfCos)
