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
* @file tgfCommonCscdUT.c
*
* @brief Enhanced UTs for CPSS Cascading
*
* @version   30
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>

#include <cscd/prvTgfCscd.h>
#include <cscd/prvTgfCscdPortStackAggregation.h>
#include <cscd/tgfCscdRemotePortRemapping.h>
#include <cscd/tgfCscdEPortMode.h>
#include <cscd/prvTgfCscdCentralizedChassisRemotePhysicalPortMap.h>
#include <cscd/prvTgfCscdCentralizedChassisRemoteSourcePhysicalPortMap.h>
#include <cscd/prvTgfCscdDsaFrwdSrcId.h>
#include <cscd/prvTgfCscdDsaToAnalyserVlanTag.h>
#include <cscd/prvTgfCscdDsaEgrFilterRegister.h>
#include <cscd/prvTgfCscdDsaRxMirrSrcFlds.h>
#include <cscd/prvTgfCscdDsaTxMirrSrcFlds.h>
#include <cscd/prvTgfCscdDsaSrcIsTrunk.h>
#include <cscd/prvTgfCscdDsaTxMirrVlanTag.h>
#include <cscd/prvTgfCscdDsaToAnalyzerVidx.h>
#include <cscd/prvTgfCscdDsaTxMirrVidx.h>
#include <cscd/prvTgfCscdDsaToCpuTrgFlds.h>
#include <cscd/prvTgfCscdDsaRxMirrAnInfo.h>
#include <cscd/prvTgfCscdDsaTxMirrAnInfo.h>
#include <cscd/prvTgfCscdDsaEgrToCpuTrgFlds.h>
#include <cscd/prvTgfCscdDsaEportSizeCheck.h>
#include <cscd/prvTgfCscdFdbSaLookup.h>
#include <cscd/prvTgfCscdSkipSaLearning.h>
#include <cscd/prvTgfCscdDsaCfiId.h>


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test packet sending according to device map table for not local devices:
    configure VLAN, FDB entries;
    configure both Device and Port lookup mode for accessing Device Map table;
    configure CSCD map table;
        configure FDB entries with different devices and ports;
    send Ethernet traffic for different FDB entries;
    verify traffic on FDB ports.
*/
UTF_TEST_CASE_MAC(prvTgfCscdSingleTargetDestination)
{
/********************************************************************
    Test 1.1 - Packet sending according to device map table for not local devices.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_NONE_FAMILY_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfCscdSingleTargetDestinationConfigurationSet();

    /* Generate traffic */
    prvTgfCscdSingleTargetDestinationTrafficGenerate();

    /* Restore configuration */
    prvTgfCscdSingleTargetDestinationConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Packet sending according to device map table for local devices:
    configure VLAN, FDB entries;
    configure lookup mode to port for accessing Device Map table;
    configure CSCD map table;
        enable local target port for device map lookup for local device;
    send Ethernet traffic and verify traffic on egress ports.
*/
UTF_TEST_CASE_MAC(prvTgfCscdSingleTargetDestinationLocalDevice)
{
/********************************************************************
    Test 1.2 - Packet sending according to device map table for local devices.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfCscdSingleTargetDestinationLocalDeviceConfigurationSet();

    /* Generate traffic */
    prvTgfCscdSingleTargetDestinationLocalDeviceTrafficGenerate();

    /* Restore configuration */
    prvTgfCscdSingleTargetDestinationLocalDeviceConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Packet sending according to device map table to remote device:
    configure VLAN, FDB entries;
    configure lookup mode to port for accessing Device Map table;
    configure CSCD map table;

    send Ethernet traffic and verify traffic on egress ports.
*/
UTF_TEST_CASE_MAC(prvTgfCscdCscdPortStackAggregation)
{
/********************************************************************
    Test 1.3 - Test port stack aggregation functionality.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(prvTgfCscdStackAggregationCheck() == GT_FALSE)
    {
        return;
    }

    /* Set configuration */
    prvTgfCscdPortStackAggregationConfigurationSet();

    /* Generate traffic */
    prvTgfCscdPortStackAggregationTrafficGenerate();

    /* Restore configuration */
    prvTgfCscdPortStackAggregationConfigurationRestore();

}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Packet sending according to device map table to remote device:
    configure VLAN;
    configure lookup mode to port for accessing Device Map table;
    configure CSCD map table;

    send Ethernet traffic and verify traffic on egress ports.

    Check: TTI action and HA when packet come from port interface.
*/
UTF_TEST_CASE_MAC(prvTgfCscdCscdPortStackAggregationFullPath)
{
/********************************************************************
    Test 1.4 - Test port stack aggregation functionality.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(prvTgfCscdStackAggregationCheck() == GT_FALSE)
    {
        return;
    }

    /* Set configuration */
    prvTgfCscdPortStackAggregationFullPathConfigurationSet();

    /* Generate traffic */
    prvTgfCscdPortStackAggregationFullPathTrafficGenerate();

    /* Restore configuration */
    prvTgfCscdPortStackAggregationFullPathConfigurationRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Check mapping of a target physical port to a remote physical port
    that resides over a DSA-tagged interface
*/
UTF_TEST_CASE_MAC(prvTgfCscdCentralizedChassisRemotePhysicalPortMap)
{
    GT_U32 notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set configuration */
    prvTgfCscdCentralizedChassisRemotePhysicalPortMapConfigurationSet();

    /* Generate traffic */
    prvTgfCscdCentralizedChassisRemotePhysicalPortMapTrafficGenerate();

    /* Restore configuration */
    prvTgfCscdCentralizedChassisRemotePhysicalPortMapConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Check mapping of a source physical port to a remote physical port
    that resides over a DSA-tagged interface
*/
UTF_TEST_CASE_MAC(prvTgfDSARemotePhysicalPortMap)
{

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    if ( 1024 != prvTgfPortModeGet())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set configuration */
    prvTgfDSARemotePhysicalPortMapConfigurationSet();

    /* Generate traffic */
    prvTgfDSARemotePhysicalPortMapTrafficGenerate();

    /* Restore configuration */
    prvTgfDSARemotePhysicalPortMapConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Check mapping of a source physical port to a remote physical port
    that resides over a vlan-tagged interface
*/
UTF_TEST_CASE_MAC(prvTgfVlanTagRemotePhysicalPortMap)
{

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    if ( 1024 != prvTgfPortModeGet())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set configuration */
    prvTgfVlanRemotePhysicalPortMapConfigurationSet();

    /* Generate traffic */
    prvTgfVlanRemotePhysicalPortMapTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanRemotePhysicalPortMapConfigurationRestore();
}



/*----------------------------------------------------------------------------*/
/* AUTODOC: Check mapping of a source physical port to a remote physical port
    that resides over a E-tagged interface
*/
UTF_TEST_CASE_MAC(prvTgfEtagRemotePhysicalPortMap)
{

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    if ( 1024 != prvTgfPortModeGet())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set configuration */
    prvTgfEtagRemotePhysicalPortMapConfigurationSet();

    /* Generate traffic */
    prvTgfEtagRemotePhysicalPortMapTrafficGenerate();

    /* Restore configuration */
    prvTgfEtagRemotePhysicalPortMapConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Packet sending according to port remapping to remote device:

    Set test configuration:
        configure Port 0 as DSA port,
        set <Number Of DSA Source Port Bits>   = 5;
            <Number Of DSA Source Device Bits> = 0;
            <Physical Port Base> = 10;

    Generate traffic:
        Send to device's port[0] packet:
            macDa = FF:FF:FF:FF:FF:FF,
            macSa = 00:00:00:00:00:02,
        Success Criteria:
             fdb learning return match on port 13

    Restore test configuration
*/
UTF_TEST_CASE_MAC(tgfCscdRemotePortRemapping)
{
/********************************************************************
    Test 1.5 - Test remote port remapping functionality.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;


    /* Set configuration */
    tgfCscdRemotePortRemappingConfigurationSet();

    /* Generate traffic */
    tgfCscdRemotePortRemappingTrafficGenerate();

    /* Restore configuration */
    tgfCscdRemotePortRemappingConfigurationRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Packet sending according to ePort of remote device

    Set test configuration:
        configure Port 0 as DSA port,

        configure port with:
            port base ePort
            trunk base ePort
            src trunk lsb amount
            src port lsb amount
            src dev lsb amount

    Generate traffic:
        Send to device's port[0] packet:
            macDa = FF:FF:FF:FF:FF:FF,
            macSa = 00:00:00:00:00:02,
        Success Criteria:
             fdb learning return match on port 18

    Restore test configuration
*/
UTF_TEST_CASE_MAC(tgfCscdEPortMode)
{
/********************************************************************
    Test 1.6 - Test remote ePort remapping functionality.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    tgfCscdEPortModeConfigurationSet();

    /* Generate traffic */
    tgfCscdEPortModeTrafficGenerate();

    /* Restore configuration */
    tgfCscdEPortModeConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Check DSA for Packet sending according to port remapping to remote device:

*       test relevant only to E_ARCH devices
*
*       1. set fdb entry on eport (TARGET_EPORT_NUM_CNS) on local device
*       2. map by the e2Phy this eport to remote {dev,port} physical interface.
*           a. remote dev = targetDevNumArr[2] , remote port = targetPortNumArr[2]
*       3. Set the egress port as 'egress cascade port' with DSA of 2 words
*           (but as ingress 'not cascade port')
*       4. send the traffic
*       5. trap the traffic that egress the port
*       6. make sure that trgPort in the DSA is targetPortNumArr[2] (and not eport)
*/
UTF_TEST_CASE_MAC(tgfCscdLocalEPortMapToRemotePhyPortAndDsaTagCheck)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* run the test */
    localEPortMapToRemotePhyPortAndDsaTagCheck();
}

/* AUTODOC:
   Test a source-ID in the eDsa tag (FORWARD command). The packet is sent:
   - from a network port to a cascade port
   - from a cascade port to a cascade port
   - from a cascade port to a network port
   Tx port is checked: it should or shouldn't filter the packet
   depending on the port is added to appropriate source-id group. */
UTF_TEST_CASE_MAC(prvTgfCscdDsaFrwdSrcId)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaFrwdSrcIdTest();

    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        prvTgfCscdDsaFrwdSrcIdTest_xCat3x_Extra();
    }
}

/* AUTODOC:
   Test a Vlan Tag Related Fields in the eDsa tag (TO ANALYSER command). The packet is sent:
   - from a network port to a cascaded port
   Analyser output checked */
UTF_TEST_CASE_MAC(prvTgfCscdDsaToAnalyzerVlanTagNetworkToCascaded)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaToAnalyzerVlanTagNetworkToCascaded();
}

/* AUTODOC:
   Test a Vlan Tag Related Fields in the eDsa tag (TO ANALYSER command). The packet is sent:
   - from a cascaded port to a network port
   Analyser output checked */
UTF_TEST_CASE_MAC(prvTgfCscdDsaToAnalyzerVlanTagCascadedToNetwork)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaToAnalyzerVlanTagCascadedToNetwork();
}

/* AUTODOC:
   Test a Vlan Tag Related Fields in the eDsa tag (TO ANALYSER command). The packet is sent:
   - from a cascaded port to a cascaded port
   Analyser output checked */
UTF_TEST_CASE_MAC(prvTgfCscdDsaToAnalyzerVlanTagCascadedToCascaded)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaToAnalyzerVlanTagCascadedToCascaded();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test for Egress Filter Registered field in Forward eDsa tag:
    CASE 1: packet send from Network port to Cascade port
    CASE 2: packet send from Cascade port to Cascade port
    CASE 3: packet send from Cascade port to Network port
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaEgrFilterRegister)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfCscdDsaEgrFilterRegisterTest();
}

/* AUTODOC:
   Check correctness of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
   <SrcTrg Tagged> inside a packet rx-mirrored to a remote port. */
UTF_TEST_CASE_MAC(prvTgfCscdDsaRxMirrSrcFlds)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaRxMirrSrcFldsTest();
}

/* AUTODOC:
   Check correctness of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
   <SrcTrg Tagged> inside a packet tx-mirrored to a remote port. */
UTF_TEST_CASE_MAC(prvTgfCscdDsaTxMirrSrcFlds)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaTxMirrSrcFldsTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    Test for Source is trunk; Source port/trunk; MC filter enable fields in
    Forward eDsa tag:
    CASE 1: packet send from Network port to Cascade port
    CASE 2: packet send from Cascade port to Cascade port
    CASE 3: packet send from Cascade port to Network port
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaFrwdSrcIsTrunk)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfCscdDsaFrwdSrcIsTrunkTest();
}

/* AUTODOC:
   Check correctness of eDSA fields <eVLAN>, <CFI>, <UP>, <TPID> of
   of a tx-mirrored packet. There are 3 cases. */
UTF_TEST_CASE_MAC(prvTgfCscdDsaTxMirrVlanTag)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaTxMirrVlanTagTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    Test for Analyzer Use eVIDX, Analyzer eVidx fields in To Analyzer eDsa tag:
    CASE 1: packet send from Network port to Cascade port
    CASE 2: packet send from Cascade port to Cascade port
    CASE 3: packet send from Cascade port to Network port
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaToAnalyzerVidx)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfCscdDsaToAnalyzerVidxTest();
}

/* AUTODOC:
   Check correctress of eDSA fields  <Analyzer Use eVIDX>, <Analyzer eVIDX>
    inside a tx-mirrored packet */
UTF_TEST_CASE_MAC(prvTgfCscdDsaTxMirrVidx)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaTxMirrVidxTest();
}

/* AUTODOC:
   Test checks target info for TO CPU by Ingress Pipe for Original is Trunk.
   The packet is sent:
   - from a network port to a cascade port
   Following eDSA tag fields checked:
       Source / Target packet data
       Source Tagged / Untagged
       Source Device
       Source physical port
       Orig is Trunk - should be 1
       Source ePort
       Source Trunk
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaToCpuNetworkToCascade)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaToCpuNetworkToCascade();
}

/* AUTODOC:
   Test checks target info for TO CPU by Ingress Pipe for Original is Trunk.
   The packet is sent:
   - from a cascade port to a cascade port
   Following eDSA tag fields checked:
       Source / Target packet data
       Source Tagged / Untagged
       Source Device
       Source physical port
       Orig is Trunk - should be 1
       Source ePort
       Source Trunk
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaToCpuCascadeToCascade)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaToCpuCascadeToCascade();
}

/* AUTODOC:
   Test checks target info for TO CPU by Ingress Pipe for Original is Trunk.
   The packet is sent:
   - from a cascade port to a CPU port
   Following eDSA tag fields checked:
       Source / Target packet data
       Source Tagged / Untagged
       Source Device
       Source physical port
       Orig is Trunk - should be 1
       Source ePort
       Source Trunk
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaToCpuCascadeToCpu)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaToCpuCascadeToCpu();
}

/* AUTODOC:
   Test checks target info for FORWARD that change TO CPU by Ingress Pipe for Original is Trunk = 0.
   The packet is sent:
   - from a cascade port to a CPU port
   Following eDSA tag fields checked:
       Source / Target packet data
       Source Tagged / Untagged
       Source Device
       Source physical port
       Orig is Trunk - should be 1
       Source ePort
       Source Trunk
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaForwardCascadeToCpu)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaForwardCascadeToCpu();
}


/* AUTODOC:
   Check correctress of eDSA (TO_ANALYZER) fields
           - <Analyzer phy port is valid>
           - <Analyzer target device>
           - <Analyzer target physical port>
           - <Analyzer target ePort>
   inside an rx-mirrored packet */
UTF_TEST_CASE_MAC(prvTgfCscdDsaRxMirrAnInfo)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaRxMirrAnInfoTest();
}


/* AUTODOC:
   Check correctress of eDSA (TO_ANALYZER) fields
           - <Analyzer phy port is valid>
           - <Analyzer target device>
           - <Analyzer target physical port>
           - <Analyzer target ePort>
   inside a tx-mirrored packet */
UTF_TEST_CASE_MAC(prvTgfCscdDsaTxMirrAnInfo)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaTxMirrAnInfoTest();
}


/* AUTODOC:
   Test checks target info for TO_CPU by Egress Pipe.
   Following eDSA tag fields checked:
           - Target Tagged / Untagged
           - Target Device
           - Target physical port
           - Target ePort
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaEgrToCpuTrgFlds)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCscdDsaEgrToCpuTrgFldsTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test for Eport size in Forward eDsa tag:
            packet send from Network port to Cascade port
            Send a packet with target ePort beyond maximum range
            Verify the number should be with-in the range.
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaEportSizeCheck)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    prvTgfCscdDsaEportSizeCheck();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test for Skip FDB SA lookup bit in Forward eDsa tag:
 *          Send 2 packets before and after setting SA Skip bit
 *          Verify the packets egressed from cascade port.
 *          The packet before the configuration should have 0(SA skip bit not set)
 *          The packet After the configuration should have 1(SA skip bit set)
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaSkipFdbSaLookup)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    prvTgfCscdDsaSkipFdbSaLookupTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test for Skip FDB SA lookup bit in Forward eDsa tag:
 *          Send 2 packets with different SA MAC with SET/RESET of "AcceptFdbSaLookupSkipEnable" bit
 *          Verify the FDB table MAC belongs to the "AcceptFdbSaLookupSkipEnable" SET packet should be learnt,
 *          MAC belongs to RESET "AcceptFdbSaLookupSkipEnable" should not be present in FDB table.
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaSkipFdbSaLearning)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    prvTgfCscdDsaSkipFdbSaLearningTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test for Checking CFI bit in egress DSA tag/
 *          Send Tagged/Untagged packet with CFI bit enable/disable
 *          Verify the egress DSA tag hae the CFI bit according to dpToCFI bit,
*/
UTF_TEST_CASE_MAC(prvTgfCscdDsaCfiId)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));
    prvTgfCscdDsaCfiIdTest();
}

/*
 * Configuration of tgfCscd suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfCscd)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdSingleTargetDestination)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdSingleTargetDestinationLocalDevice)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdCscdPortStackAggregation)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdCscdPortStackAggregationFullPath)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdCentralizedChassisRemotePhysicalPortMap)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCscdRemotePortRemapping)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCscdEPortMode)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCscdLocalEPortMapToRemotePhyPortAndDsaTagCheck)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaFrwdSrcId)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaToAnalyzerVlanTagNetworkToCascaded)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaToAnalyzerVlanTagCascadedToNetwork)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaToAnalyzerVlanTagCascadedToCascaded)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaEgrFilterRegister)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaRxMirrSrcFlds)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaTxMirrSrcFlds)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaFrwdSrcIsTrunk)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaTxMirrVlanTag)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaToAnalyzerVidx)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaTxMirrVidx)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaToCpuNetworkToCascade)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaToCpuCascadeToCascade)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaToCpuCascadeToCpu)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaForwardCascadeToCpu)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaRxMirrAnInfo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaTxMirrAnInfo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaEgrToCpuTrgFlds)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaEportSizeCheck)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaSkipFdbSaLookup)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaSkipFdbSaLearning)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCscdDsaCfiId)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfDSARemotePhysicalPortMap)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanTagRemotePhysicalPortMap)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEtagRemotePhysicalPortMap)
UTF_SUIT_END_TESTS_MAC(tgfCscd)


